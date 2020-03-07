/*
###############################################################################
#
#  EGSnrc egs++ radiative splitting object
#  Copyright (C) 2018 National Research Council Canada
#
#  This file is part of EGSnrc.
#
#  EGSnrc is free software: you can redistribute it and/or modify it under
#  the terms of the GNU Affero General Public License as published by the
#  Free Software Foundation, either version 3 of the License, or (at your
#  option) any later version.
#
#  EGSnrc is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
#  FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for
#  more details.
#
#  You should have received a copy of the GNU Affero General Public License
#  along with EGSnrc. If not, see <http://www.gnu.org/licenses/>.
#
###############################################################################
#
#  Author:          Ernesto Mainegra-Hing, 2018
#
#  Contributors:
#
###############################################################################
#
#  A general radiative splitting tool.
#
#  TODO:
#
#  - Add directional radiative splitting (DRS)
#
###############################################################################
*/


/*! \file egs_radiative_splitting.cpp
 *  \brief A radiative splitting ausgab object: implementation
 *  \EM
 */

#include <fstream>
#include <string>
#include <cstdlib>

#include "egs_radiative_splitting.h"
#include "egs_input.h"
#include "egs_functions.h"
#include "array_sizes.h"

extern "C" void F77_OBJ_(kill_the_photons,KILL_THE_PHOTONS)(const EGS_Float *fs, const EGS_Float *ssd,
                  const int *nbrspl,const int *nstart,const int *do_electrons){;}

extern "C" void F77_OBJ_(brems,BREMS)(){;}
extern __extc__ void F77_OBJ_(egs_fill_rndm_array,EGS_GET_RNDM_ARRAY)(const EGS_I32 *n, EGS_Float *rarray);

struct EGS_Stack {
  double    E[MXSTACK];
  EGS_Float x[MXSTACK];
  EGS_Float y[MXSTACK];
  EGS_Float z[MXSTACK];
  EGS_Float u[MXSTACK];
  EGS_Float v[MXSTACK];
  EGS_Float w[MXSTACK];
  EGS_Float dnear[MXSTACK];
  EGS_Float wt[MXSTACK];
  EGS_I32   iq[MXSTACK];
  EGS_I32   ir[MXSTACK];
  EGS_I32   latch[MXSTACK];
  EGS_I32   latchi;
  EGS_I32   np;
  EGS_I32   npold;
};

struct EGS_XOptions {
  EGS_I32   ibrdst;
  EGS_I32   iprdst;
  EGS_I32   ibr_nist;
  EGS_I32   spin_effects;
  EGS_I32   ibcmp;
  EGS_I32 iraylr;
  EGS_I32   iedgfl;
  EGS_I32   iphter;
  EGS_I32   pair_nrc;
  EGS_I32   itriplet;
  EGS_I32   radc_flag;
  EGS_I32   eii_flag;
  EGS_I32 iphotonuc;
  EGS_I32 eadl_relax;
  EGS_I32 mcdf_pe_xsections;
};

struct EGS_Media {
    EGS_Float rho[MXMED];
    char      pxsec[16];
    char      eiixsec[16];
    char      compxsec[16];
    char      photonucxsec[16];
    EGS_I32   nmed;
};

extern __extc__ struct EGS_Stack F77_OBJ_(stack,STACK);
struct EGS_Stack *the_stack = & F77_OBJ_(stack,STACK);

extern __extc__ struct EGS_XOptions F77_OBJ_(xsection_options,XSECTION_OPTIONS);
struct EGS_XOptions  *the_xoptions = & F77_OBJ_(xsection_options,XSECTION_OPTIONS);

extern __extc__ struct EGS_Media F77_OBJ_(media,MEDIA);
struct EGS_Media *the_media = & F77_OBJ_(media,MEDIA);

EGS_RadiativeSplitting::EGS_RadiativeSplitting(const string &Name,
        EGS_ObjectFactory *f) :
    nsplit(1) {
    otype = "EGS_RadiativeSplitting";
}

EGS_RadiativeSplitting::~EGS_RadiativeSplitting() {
}

void EGS_RadiativeSplitting::setApplication(EGS_Application *App) {
    EGS_AusgabObject::setApplication(App);
    if (!app) {
        return;
    }

    char buf[32];

    // Set EGSnrc internal radiative splitting number .
    app->setRadiativeSplitting(nsplit);

    description = "\n===========================================\n";
    description +=  "Radiative splitting Object (";
    description += name;
    description += ")\n";
    description += "===========================================\n";
    if (nsplit > 1) {
        description +="\n - Splitting radiative events in ";
        sprintf(buf,"%d\n\n",nsplit);
        description += buf;
    }
    else if (nsplit == 1) {
        description +="\n - NO radiative splitting";
    }
    else {
        description +="\n - BEWARE: Turning OFF radiative events !!!";
    }
    description += "\n===========================================\n\n";
}

void EGS_RadiativeSplitting::initDBS(const float &field_rad, const float &field_ssd, const vector<int> &splitreg, const int &irad, const float &zrr) {
        fs = field_rad;
        ssd = field_ssd;
        ireg_esplit = splitreg;
        irad_esplit = irad;
        zrr_esplit = zrr;

        y2_KM = new EGS_Float [nsplit];
        f_KM_a = new EGS_Interpolator* [the_media->nmed];
        f_KM_b = new EGS_Interpolator* [the_media->nmed];
        zbr_KM = new EGS_Float [the_media->nmed];
}

//at least try to get brems working with this
int EGS_RadiativeSplitting::doInteractions(int iarg, EGS_RandomGenerator *rndm, int &killed)
{
    killed = 0;

    if( iarg > EGS_Application::AfterTransport && app->top_p.x.z > ssd ) {
        //particle is past ssd, no splitting
        app->setRadiativeSplitting(1); return 0;
    }

    int np = app->Np; int np1 = np;

    int latch = app->top_p.latch;

    //use bit 31 to mark phat particles
    //seems like a temporary solution
    int is_fat = (latch & (1 << 31));

    if( iarg == EGS_Application::BeforeBrems ) {
        double E = app->top_p.E;
        EGS_Float wt = app->top_p.wt;
        if( is_fat ) {
            //clear bit 31
            latch = latch & ~(1 << 31);
            app->setLatch(latch);
            //is the next line necessary?
            app->setRadiativeSplitting(nsplit);
            int res = doSmartBrems(rndm);
            if( res ) {
                F77_OBJ(brems,BREMS)();
                int nstart = np+2, aux=0;
                F77_OBJ_(kill_the_photons,KILL_THE_PHOTONS)(&fs,&ssd,&nsplit,&nstart,&aux);
            }
        }
        else {
            app->setRadiativeSplitting(1);
            F77_OBJ(brems,BREMS)();
            int nstart = np+2, aux=0;
            F77_OBJ_(kill_the_photons,KILL_THE_PHOTONS)(&fs,&ssd,&nsplit,&nstart,&aux);
        }
    }

    return 1;
}


int EGS_RadiativeSplitting::doSmartBrems(EGS_RandomGenerator *rndm) {
    int np = app->Np;
    int iwt = nsplit;
    int nbrspl = iwt;

    double E = app->top_p.E;
    EGS_Float ener = E - app->getRM();
    EGS_Float tau = ener/app->getRM();
    EGS_Float beta = sqrt(tau*(tau+2))/(tau+1);
    EGS_Float beta2 = beta*beta;
    EGS_Vector x = app->top_p.x;
    EGS_Vector u = app->top_p.u;

    /*
    egsInformation("smartBrems: E=%g be=%g x=(%g,%g,%g) wt=%g nspl=%d\n",ener,
            be_factor,x.x,x.y,x.z,the_stack->wt[np],nbrspl);
    */

    EGS_Float ct_min,ct_max,ro;
    getCostMinMax(x,u,ro,ct_min,ct_max);
    int imed = app->getMedium(app->isWhere(x));
    EGS_Float f_max_KM = 1, q_KM, p_KM; int j_KM;
    /* figure out what to do with ibrdst == 1 later
    if(the_xoptions->ibrdst == 1) {
        q_KM = a_KM[imed]*the_epcont->elke + b_KM[imed];
        j_KM = (int) q_KM; q_KM -= j_KM; p_KM = 1 - q_KM;
        f_max_KM = f_KM_max[imed].interpolateFast(j_KM,the_epcont->elke);
    }
    */

    EGS_Float w1, cmin, cmax;
    if( the_xoptions->ibrdst == 1 ) {
        cmin = sqrt(1-beta*ct_min); cmax = sqrt(1-beta*ct_max);
        w1 = (cmin - cmax)*sqrt(1+beta)/(cmax*cmin*((1+beta)*(1+tau)-1));
    }
    else w1 = (ct_max - ct_min)/((1-beta*ct_max)*(1-beta*ct_min)*2*
                                     (tau+1)*(tau+1));
    w1 *= f_max_KM;
    EGS_Float d = ssd - x.z;
    EGS_Float dmin = ro <= fs ? d : sqrt(d*d + (ro-fs)*(ro-fs));
    EGS_Float w2 = fs*fs*d/(2*dmin*dmin*dmin);
    bool will_rotate = use_cyl_sym && x.z < zcyls;
    //if( will_rotate ) w2 *= rsamp->getAeff();
    EGS_Float aux;
    if( the_xoptions->ibrdst == 1 ) {
        w2 *= beta*sqrt(1+beta)/(2*(1-beta*ct_max)*sqrt(1-beta*ct_max)*
                ((1+beta)*(tau+1)-1));
    }
    else {
       aux = (tau+1)*(1-beta*ct_max);
       w2 /= (2*aux*aux);
    }
    w2 *= f_max_KM;
    EGS_Float wprob = min(w1,w2);
    if( wprob >= 1 && the_xoptions->ibrdst == 1 ) return 1;
    int nsample;
    if( wprob > 1 ) { ct_min = -1; ct_max = 1; wprob = 1; }
    //EGS_Float asample = wprob*nbrspl/be_factor;
    EGS_Float asample = wprob*nbrspl;
    nsample = (int) asample; asample -= nsample;
    if( rndm->getUniform() < asample ) ++nsample;

    EGS_Float aux1 = ct_max - ct_min, aux2 = 1 - beta*ct_max;
    EGS_Float wt = app->top_p.wt/nbrspl;
    EGS_Float sinpsi, sindel, cosdel; bool need_rotation;
    sinpsi = u.x*u.x + u.y*u.y;
    if( sinpsi > 1e-20 ) {
        sinpsi = sqrt(sinpsi); need_rotation = true;
        cosdel = u.x/sinpsi;
        sindel = u.y/sinpsi;
    } else need_rotation = false;
    int ip = np;
    int irl=app->top_p.ir;
    int latch=app->top_p.latch;
    EGS_Float dnear = the_stack->dnear[np+1]; int ib = 0;

    if( w1 < w2 ) {
        for(int j=0; j<nsample; j++) {
            EGS_Float rnno = rndm->getUniform(), cost;
            if( the_xoptions->ibrdst == 1 ) {
                EGS_Float tmp = cmin*cmax/(rnno*cmin+(1-rnno)*cmax);
                cost = (1 - tmp*tmp)/beta;
            }
            else {
                rnno *= aux1;
                cost = (ct_min*aux2+rnno)/(aux2+beta*rnno);
            }
            EGS_Float sint = 1 - cost*cost;
            sint = sint > 0 ? sqrt(sint) : 0;
            EGS_Float cphi,sphi; rndm->getAzimuth(cphi,sphi);
            EGS_Float un,vn,wn;
            if( need_rotation ) {
                EGS_Float us = sint*cphi, vs = sint*sphi;
                un = u.z*cosdel*us - sindel*vs + u.x*cost;
                vn = u.z*sindel*us + cosdel*vs + u.y*cost;
                wn = u.z*cost - sinpsi*us;
            } else { un = sint*cphi; vn = sint*sphi; wn = u.z*cost; }
            int ns = 0;
            if( wn > 0 ) {
                EGS_Float aux = (ssd - x.z)/wn;
                EGS_Float x1 = x.x + un*aux, y1 = x.y + vn*aux;
                if( x1*x1 + y1*y1 < fs*fs ) ns = 1;
            }
            if( !ns ) {
                if( rndm->getUniform()*nbrspl < 1 ) ns = nbrspl;
            }
            if( ns > 0 ) {
                if( ++ip >= MXSTACK ) egsFatal(dbs_err_msg,
                        "smartBrems",MXSTACK);
                if( the_xoptions->ibrdst == 1 )
                    y2_KM[ib++] = beta*(1+beta)*(tau+1)*(tau+1)*(1-cost);
                the_stack->x[ip]=x.x; the_stack->y[ip]=x.y;
                the_stack->z[ip]=x.z;
                the_stack->u[ip]=un;  the_stack->v[ip]=vn;  the_stack->w[ip]=wn;
                the_stack->ir[ip]=irl; the_stack->dnear[ip]=dnear;
                the_stack->wt[ip]=wt*ns; the_stack->latch[ip]=latch;
                the_stack->iq[ip]=0;
                /*
                the_extra_stack->icreate[ip] = the_extra_stack->int_num;
                the_extra_stack->pid[ip] = ++the_extra_stack->pidI;
                the_extra_stack->iweight[ip] = ns;
                 */
            }
        }
    }
    else {
        for(int j=0; j<nsample; j++) {
            EGS_Float x1, y1; int iw;
            //if( will_rotate ) rsamp->getPoint(rndm,fs,x1,y1,iw);
            //else {
                do {
                    x1 = 2*rndm->getUniform()-1; y1 = 2*rndm->getUniform()-1;
                } while ( x1*x1 + y1*y1 > 1 );
                x1 *= fs; y1 *= fs; iw = 1;
            //}
            EGS_Float un = x1 - x.x, vn = y1 - x.y, wn = d;
            EGS_Float dist = sqrt(un*un + vn*vn + wn*wn);
            EGS_Float disti = 1/dist;
            un *= disti; vn *= disti; wn *= disti;
            EGS_Float cost = u.x*un + u.y*vn + u.z*wn;
            EGS_Float aux = dmin*disti;
            EGS_Float rejf;
            if( the_xoptions->ibrdst == 1 ) {
                rejf = aux2/(1-beta*cost); rejf *= sqrt(rejf)*aux*aux*aux;
            }
            else {
                rejf = aux2/(1-beta*cost)*aux;
                rejf *= rejf*aux;
            }
            if( rndm->getUniform() < rejf ) {
                if( ++ip >= MXSTACK ) egsFatal(dbs_err_msg,
                        "smartBrems",MXSTACK);
                if( the_xoptions->ibrdst == 1 ) {
                    EGS_Float y2 = beta*(1+beta)*(tau+1)*(tau+1)*(1-cost);
                    if( y2 < 0 ) y2 = 0;
                    y2_KM[ib++] = y2;
                }
                the_stack->x[ip]=x.x; the_stack->y[ip]=x.y;
                the_stack->z[ip]=x.z;
                the_stack->u[ip]=un;  the_stack->v[ip]=vn;  the_stack->w[ip]=wn;
                the_stack->ir[ip]=irl; the_stack->dnear[ip]=dnear;
                the_stack->wt[ip]=wt*iw; the_stack->latch[ip]=latch;
                the_stack->iq[ip]=0;
                /*
                the_extra_stack->icreate[ip] = the_extra_stack->int_num;
                the_extra_stack->pid[ip] = ++the_extra_stack->pidI;
                the_extra_stack->iweight[ip] = iw;
                 */
            }
        }
    }
    bool real_brems = true;
    //if( be_factor > 1 ) real_brems = be_factor*rndm->getUniform() < 1;
    int ireal = -1;
    if( real_brems ) {
        bool take_it = true; EGS_Float un=0,vn=0,wn=1;
        if( the_xoptions->ibrdst != 1 ) {
            aux1 = 2; aux2 = 1 - beta; EGS_Float rnno = rndm->getUniform()*aux1;
            EGS_Float cost = (rnno-aux2)/(aux2+beta*rnno);
            if( w2 <= w1 || ((cost < ct_min || cost > ct_max) && w1 < w2) ) {
                EGS_Float sint = 1 - cost*cost;
                sint = sint > 0 ? sqrt(sint) : 0;
                EGS_Float cphi,sphi; rndm->getAzimuth(cphi,sphi);
                if( need_rotation ) {
                    EGS_Float us = sint*cphi, vs = sint*sphi;
                    un = u.z*cosdel*us - sindel*vs + u.x*cost;
                    vn = u.z*sindel*us + cosdel*vs + u.y*cost;
                    wn = u.z*cost - sinpsi*us;
                } else { un = sint*cphi; vn = sint*sphi; wn = u.z*cost; }
                if( w2 <= w1 && wn > 0) {
                    EGS_Float t = (ssd-x.z)/wn;
                    EGS_Float x1 = x.x + un*t, y1 = x.y + vn*t;
                    if( x1*x1 + y1*y1 <= fs*fs ) take_it = false;
                }
            } else take_it = false;
        }
        if( take_it ) {
            if( ++ip >= MXSTACK ) egsFatal(dbs_err_msg,
                    "smartBrems",MXSTACK);
            ireal = ip;
            the_stack->x[ip]=x.x;the_stack->y[ip]=x.y;the_stack->z[ip]=x.z;
            the_stack->u[ip]=un; the_stack->v[ip]=vn; the_stack->w[ip]=wn;
            the_stack->ir[ip]=irl; the_stack->dnear[ip]=dnear;
            the_stack->wt[ip]=wt*nbrspl; the_stack->latch[ip]=latch;
            the_stack->iq[ip]=0;
          /*
            the_extra_stack->icreate[ip] = the_extra_stack->int_num;
            the_extra_stack->pid[ip] = ++the_extra_stack->pidI;
            the_extra_stack->iweight[ip] = nbrspl;
          */
        }
    }

    the_stack->np = ip+1;
    if( ip > np || real_brems ) {
        //getBremsEnergies();
        if( !real_brems ) the_stack->E[np] = E;
        if( ip > np && the_xoptions->ibrdst == 1 ) {
            ib = 0; EGS_Float E = tau+1; EGS_Float unorm = log(0.5*(E+1));
            for(int j=np+1; j<=ip; j++) {
                EGS_Float k = the_stack->E[j]/app->getRM();
                EGS_Float Ep = E - k;
                EGS_Float r = Ep/E; EGS_Float delta = k/(2*E*Ep);
                delta *= delta;
                EGS_Float arg1 = 1 + r*r, arg2 = arg1 + 2*r;
                if( j != ireal ) {
                    EGS_Float y2 = y2_KM[ib++];
                    EGS_Float x = k/tau;
                    EGS_Float f1,f2;
                    if( x <= 0.5 ) {
                        f1 = f_KM_a[imed][j_KM].interpolate(x);
                        f2 = f_KM_a[imed][j_KM+1].interpolate(x);
                    } else {
                        EGS_Float u = log(E-k)/unorm;
                        f1 = f_KM_b[imed][j_KM].interpolate(u);
                        f2 = f_KM_b[imed][j_KM+1].interpolate(u);
                    }
                    EGS_Float f = f1*p_KM + f2*q_KM;
                    EGS_Float a = y2 + 1, a2 = a*a;
                    EGS_Float rejf = 16*y2*r/a2 - arg2 -
                       (arg1 - 4*y2/a2*r)*log(delta+zbr_KM[imed]/a2);
                    rejf /= (f*sqrt(a)*f_max_KM);
                    if( rejf > 1 ) egsInformation("rejf=%g for y2=%g\n",
                            rejf,y2);
                    if( rndm->getUniform() > rejf ) {
                        the_stack->wt[j] = 0; the_stack->E[j] = 0;
                    }
                }
                else {
                    // now that we know the photon energy, we must sample
                    // a direction for this photon from 2BS
                    EGS_Float arg3 = -log(delta+zbr_KM[imed]);
                    EGS_Float rejmax = arg1*arg3-arg2;
                    EGS_Float y2max = 2*beta*(1+beta)*E*E;
                    EGS_Float z2maxi = sqrt(1+y2max);
                    EGS_Float rejf,y2,rnno;
                    do {
                        rnno = rndm->getUniform();
                        y2 = rndm->getUniform();
                        EGS_Float aux3 = z2maxi/(y2+(1-y2)*z2maxi);
                        y2 = aux3*aux3-1;
                        rnno *= rejmax*aux3;
                        EGS_Float aux3_4 = aux3*aux3; aux3_4 *= aux3_4;
                        EGS_Float y2tst1 = r*y2/aux3_4;
                        EGS_Float aux4 = 16*y2tst1-arg2,
                                  aux5 = arg1-4*y2tst1;
                        if( rnno < aux4 + aux5*arg3 ) break;
                        EGS_Float aux2=log(aux3_4/(delta*aux3_4+zbr_KM[imed]));
                        rejf = aux4+aux5*aux2;
                    } while (rejf < rnno);
                    EGS_Float cost = 1 - 2*y2/y2max; bool take_it;
                    EGS_Float un,vn,wn;
                    if( w2<=w1 || ((cost<ct_min || cost>ct_max) && w1<w2) ) {
                        take_it = true;
                        EGS_Float sint = 1 - cost*cost;
                        sint = sint > 0 ? sqrt(sint) : 0;
                        EGS_Float cphi,sphi; rndm->getAzimuth(cphi,sphi);
                        if( need_rotation ) {
                            EGS_Float us = sint*cphi, vs = sint*sphi;
                            un = u.z*cosdel*us - sindel*vs + u.x*cost;
                            vn = u.z*sindel*us + cosdel*vs + u.y*cost;
                            wn = u.z*cost - sinpsi*us;
                        } else {
                            un = sint*cphi; vn = sint*sphi; wn = u.z*cost;
                        }
                        if( w2 <= w1 && wn > 0) {
                            EGS_Float t = (ssd-x.z)/wn;
                            EGS_Float x1 = x.x + un*t, y1 = x.y + vn*t;
                            if( x1*x1 + y1*y1 <= fs*fs ) take_it = false;
                        }
                    } else take_it = false;
                    if( take_it ) {
                        the_stack->u[j] = un; the_stack->v[j] = vn;
                        the_stack->w[j] = wn;
                    }
                    else {
                        the_stack->wt[j] = 0; the_stack->E[j] = 0;
                    }
                }
            }
        }
    }

    /*
    egsInformation("Particles:\n");
    for(int j=np; j<the_stack->np; j++) egsInformation("%d E=%g q=%d wt=%g"
           " iwt=%d u=(%g,%g,%g)\n",j,the_stack->E[j],the_stack->iq[j],
           the_stack->wt[j],the_extra_stack->iweight[j],
           the_stack->u[j],the_stack->v[j],the_stack->w[j]);
     */

    return 0;
}


void EGS_RadiativeSplitting::getCostMinMax(const EGS_Vector &xx, const EGS_Vector &uu,
                        EGS_Float &ro, EGS_Float &ct_min, EGS_Float &ct_max) {
    double d = ssd - xx.z, d2 = d*d;
    double xo = xx.x, yo = xx.y;
    double u = uu.x, v = uu.y, w = uu.z;
    double ro2 = xo*xo + yo*yo; ro = sqrt(ro2);
    double r = fs; double r2 = r*r; double d2r2 = d2+r2;
    double st2 = u*u + v*v;
    // handle odd cases st=0 and/or ro=0
    if( st2 < 1e-10 ) {
        double dmin = sqrt(d2 + (r-ro)*(r-ro)),
               dmax = sqrt(d2 + (r+ro)*(r+ro));
        if( w > 0 ) { ct_max = ro <= r ? 1 : d*w/dmin; ct_min = d*w/dmax; }
        else        { ct_max = d*w/dmax; ct_min = ro <= r ? -1 : d*w/dmin; }
        return;
    }
    double st = sqrt(st2);
    if( ro2 < 1e-8 ) {
        double aux = 1/sqrt(d2 + r*r);
        ct_max = (d*w + r*st)*aux; ct_min = (d*w - r*st)*aux;
        if( w ) {
            double x1 = xo + u*d/w, y1 = yo + v*d/w;
            if( x1*x1 + y1*y1 <= r2 ) {
                if( w > 0 ) ct_max = 1; else ct_min = -1;
            }
        }
        return;
    }

    EGS_Float dmin = ro <= fs ? d : sqrt(d2 + (fs-ro)*(fs-ro));
    EGS_Float dmax = sqrt(d2 + (fs+ro)*(fs+ro));
    EGS_Float aux = w*d - u*xo - v*yo;
    ct_max = aux + fs*st;
    if( ct_max > 0 ) ct_max /= dmin; else ct_max /= dmax;
    if( ct_max > 1 ) ct_max = 1;
    ct_min = aux - fs*st;
    if( ct_min > 0 ) ct_min /= dmax; else ct_min /= dmin;
    if( ct_min < -1 ) ct_min = -1;
}

//*********************************************************************
// Process input for this ausgab object
//
//**********************************************************************
extern "C" {

    EGS_RADIATIVE_SPLITTING_EXPORT EGS_AusgabObject *createAusgabObject(EGS_Input *input,
            EGS_ObjectFactory *f) {
        const static char *func = "createAusgabObject(radiative_splitting)";
        if (!input) {
            egsWarning("%s: null input?\n",func);
            return 0;
        }

        //get type of splitting
        vector<string> allowed_split_type;
        allowed_split_type.push_back("uniform");
        allowed_split_type.push_back("directional");
        allowed_split_type.push_back("BEAMnrc directional");
        int split_type;
        string str;
        if(input->getInput("splitting type",str) < 0)
	{
		egsInformation("\nEGS_RadiativeSplitting: No input for splitting type.  Will default to uniform.\n");
		split_type = EGS_RadiativeSplitting::URS;
	}
	else
	{
		split_type = input->getInput("splitting type",allowed_split_type,-1);
        	if (split_type < EGS_RadiativeSplitting::URS || split_type > EGS_RadiativeSplitting::DRSf)
        	{
    			egsFatal("\nEGS_RadiativeSplitting: Invalid splitting type.\n");
		}
	}
        EGS_Float nsplit = 1.0;
        EGS_Float fs,ssd,zrr;
        int irad = 0;
        vector<int> esplit_reg;
        vector <string> allowed_irad;
        int err = input->getInput("splitting",nsplit);
        if (err)
	{
		egsWarning("\nEGS_RadiativeSplitting: Invalid splitting no.  Radiative splitting will be turned off.\n");
	}
        if (split_type == EGS_RadiativeSplitting::DRS || split_type == EGS_RadiativeSplitting::DRSf)
	{
		allowed_irad.push_back("no");
		allowed_irad.push_back("yes");
		int err01 = input->getInput("field size",fs);
		if (err01)
		{
			egsFatal("\nEGS_RadiativeSplitting: Missing/invalid input for splitting field radius.\n");
		}
		int err02 = input->getInput("ssd",ssd);
                if (err02)
                {
                        egsFatal("\nEGS_RadiativeSplitting: Missing/invalid input for splitting field ssd.\n");
                }
		int err03 = input->getInput("e-/e+ split region",esplit_reg);
                if (err03)
		{
			egsWarning("\nEGS_RadiativeSplitting: Missing/invalid input for e+/e- splitting region.\nCharged particles will not be split.\n");
		}
		else
		{
			if(!input->getInput("radially redistribute e-/e+",str))
			{
				vector <string> allowed_irad;
                		allowed_irad.push_back("no");
                		allowed_irad.push_back("yes");
				irad = input->getInput("radially redistribute e-/e+",allowed_irad,-1);
                           	if (irad < 0)
				{
					egsWarning("\nEGS_RadiativeSplitting: Invalid input for e-/e+ radial redistribution.  Will not radially redistribute split charged particles.\n");
					irad = 0;
				}
			}
			int err04 = input->getInput("Z of russian roulette plane",zrr);
                        if (err04)
			{
				egsWarning("\nEGS_RadiativeSplitting: Missing/invalid input for Z of russian roulette plane.  Will always subject split charged particles to russian roulette.\n");
			}
		}
	}

        //=================================================
        /* Setup radiative splitting object with input parameters */
        EGS_RadiativeSplitting *result = new EGS_RadiativeSplitting("",f);
        result->setSplitType(split_type);
        result->setSplitting(nsplit);
        if (split_type == EGS_RadiativeSplitting::DRS || split_type == EGS_RadiativeSplitting::DRSf)
	{
		result->initDBS(fs,ssd,esplit_reg,irad,zrr);
	}
        result->setName(input);
        return result;
    }
}
