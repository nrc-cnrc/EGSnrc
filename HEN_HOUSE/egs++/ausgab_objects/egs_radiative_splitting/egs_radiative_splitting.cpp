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

//subroutines and functions we need from egsnrc.mortran and other src codes
extern "C" void F77_OBJ_(kill_the_photons,KILL_THE_PHOTONS)(const EGS_Float *fs, const EGS_Float *ssd,
                  const int *nbrspl,const int *nstart,const int *do_electrons){;}

extern "C" void F77_OBJ_(brems,BREMS)(){;}
extern "C" void F77_OBJ_(annih,ANNIH)(){;}
extern "C" void F77_OBJ_(egs_fill_rndm_array,EGS_GET_RNDM_ARRAY)(const EGS_I32 *n, EGS_Float *rarray){;}
extern "C" void F77_OBJ_(alias_sample1,ALIAS_SAMPLE1)(const EGS_I32 *mxbrxs, EGS_Float *nb_xdata[3], EGS_Float *nb_fdata[3], EGS_Float *nb_wdata[3], EGS_Float *nb_idata[3]){;}

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
        f_KM_a = new EGS_Interpolator* [app->the_media->nmed];
        f_KM_b = new EGS_Interpolator* [app->the_media->nmed];
        zbr_KM = new EGS_Float [app->the_media->nmed];

        rndm = app->getRNG();
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
                killThePhotons(fs,ssd,nsplit,nstart,aux);
            }
        }
        else {
            app->setRadiativeSplitting(1);
            F77_OBJ(brems,BREMS)();
            int nstart = np+2, aux=0;
            F77_OBJ_(kill_the_photons,KILL_THE_PHOTONS)(&fs,&ssd,&nsplit,&nstart,&aux);
        }
    }
    else if( iarg == EGS_Application::BeforeAnnihFlight ) {
        if( is_fat ) {
            //figure out what to do with the extra stack
            //the_extra_stack->iweight[np]=1;
            app->setRadiativeSplitting(nsplit);
        }
        else app->setRadiativeSplitting(1);
        F77_OBJ_(annih,ANNIH)();
        int nstart=np+1,aux=0;
        F77_OBJ_(kill_the_photons,KILL_THE_PHOTONS)(&fs,&ssd,&nsplit,&nstart,&aux);
        check = 1;
    }
    else if( iarg == EGS_Application::BeforeAnnihRest ) {
        if( is_fat ) {
            app->setNpold(np);
            app->setRadiativeSplitting(nsplit);
            int nsamp = 2*nsplit;
            uniformPhotons(&nsamp,&iwt,&fs,&ssd,&the_useful->rm);
            //uniformPhotons(rndm,nsamp,2,the_useful->rm);
        }
        else {
            the_egsvr->nbr_split = 1;
            F77_OBJ_(annih_at_rest,ANNIH_AT_REST)();
            int nstart=np+1,aux=0;
            killThePhotons(&fs,&ssd,&nbrspl,&nstart,&aux);
        }
        check = 1;
    }
    return 1;
}


int EGS_RadiativeSplitting::doSmartBrems(EGS_RandomGenerator *rndm) {
    int np = app->Np;
    int iwt = nsplit;
    int nbrspl = iwt;

    //clear stack of brems particles
    particle_stack.clear();
    dnear_stack.clear();

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
    imed = app->getMedium(app->isWhere(x));
    EGS_Float f_max_KM = 1, q_KM, p_KM; int j_KM;
    if(app->the_xoptions->ibrdst == 1) {
        q_KM = a_KM[imed]*app->the_epcont->elke + b_KM[imed];
        j_KM = (int) q_KM; q_KM -= j_KM; p_KM = 1 - q_KM;
        f_max_KM = f_KM_max[imed].interpolateFast(j_KM,app->the_epcont->elke);
    }

    EGS_Float w1, cmin, cmax;
    if( app->the_xoptions->ibrdst == 1 ) {
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
    if( app->the_xoptions->ibrdst == 1 ) {
        w2 *= beta*sqrt(1+beta)/(2*(1-beta*ct_max)*sqrt(1-beta*ct_max)*
                ((1+beta)*(tau+1)-1));
    }
    else {
       aux = (tau+1)*(1-beta*ct_max);
       w2 /= (2*aux*aux);
    }
    w2 *= f_max_KM;
    EGS_Float wprob = min(w1,w2);
    if( wprob >= 1 && app->the_xoptions->ibrdst == 1 ) return 1;
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
    EGS_Float dnear = app->getDnear[np]; int ib = 0;

    if( w1 < w2 ) {
        for(int j=0; j<nsample; j++) {
            EGS_Float rnno = rndm->getUniform(), cost;
            if( app->the_xoptions->ibrdst == 1 ) {
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
                if( app->the_xoptions->ibrdst == 1 )
                    y2_KM[ib++] = beta*(1+beta)*(tau+1)*(tau+1)*(1-cost);

                EGS_Particle p;
                p.x = x;
                p.u = EGS_Vector(un,vn,wn);
                p.irl = irl;
                p.wt = wt*ns;
                p.latch = latch;
                p.q = 0;

                particle_stack.emplace_back(p);
                dnear_stack.emplace_back(dnear);

                /* need to figure this out
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
            if( app->the_xoptions->ibrdst == 1 ) {
                rejf = aux2/(1-beta*cost); rejf *= sqrt(rejf)*aux*aux*aux;
            }
            else {
                rejf = aux2/(1-beta*cost)*aux;
                rejf *= rejf*aux;
            }
            if( rndm->getUniform() < rejf ) {
                if( ++ip >= MXSTACK ) egsFatal(dbs_err_msg,
                        "smartBrems",MXSTACK);
                if( app->the_xoptions->ibrdst == 1 ) {
                    EGS_Float y2 = beta*(1+beta)*(tau+1)*(tau+1)*(1-cost);
                    if( y2 < 0 ) y2 = 0;
                    y2_KM[ib++] = y2;
                }

                EGS_Particle p;
                p.x = x;
                p.u = EGS_Vector(un,vn,wn);
                p.irl = irl;
                p.wt = wt*iw;
                p.latch = latch;
                p.q = 0;

                particle_stack.emplace_back(p);
                dnear_stack.emplace_back(dnear);

                //have to figure out what to do below
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
        if( app->the_xoptions->ibrdst != 1 ) {
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
            //put a high-weight particle directed away from the splitting field
            //on the stack?
            if( ++ip >= MXSTACK ) egsFatal(dbs_err_msg,
                    "smartBrems",MXSTACK);
            ireal = ip;

            EGS_Particle p;
            p.x = x;
            p.u = EGS_Vector(un,vn,wn);
            p.irl = irl;
            p.wt = wt*ns;
            p.latch = latch;
            p.q = 0;

            particle_stack.emplace_back(p)
            dnear_stack.emplace_back(dnear);

            //need to figure out what to do here
          /*
            the_extra_stack->icreate[ip] = the_extra_stack->int_num;
            the_extra_stack->pid[ip] = ++the_extra_stack->pidI;
            the_extra_stack->iweight[ip] = nbrspl;
          */
        }
    }

    //at this point ip has kept count of the total no. of particles to add to the stack
    //not sure what to do with real_brems yet--only relevant to BCSE
    if( ip > np || real_brems ) {
        getBremsEnergies(ip,np);
        if( !real_brems ) particle_stack.E[ip] = E;
        if( ip > np && app->the_xoptions->ibrdst == 1 ) {
            ib = 0; EGS_Float E = tau+1; EGS_Float unorm = log(0.5*(E+1));
            for(int j=0; j<ip-np; j++) {
                //note that this loop potentially includes high-weight particle put on stack
                //above.  Why?
                EGS_Float k = particle_stack.E[j]/app->getRM();
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
                        particle_stack.wt[j] = 0; particle_stack.E[j] = 0;
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
                        particle_stack.u[j] = EGS_Vector(un,vn,wn);
                    }
                    else {
                        particle_stack.wt[j] = 0; particle_stack.E[j] = 0;
                    }
                }
            }
        }
        //now add particles to the stack
        for (int i=0; i<ip-np; i++)
        {
            app->addParticleToStack(particle_stack[i],dnear_stack[i]);
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

void EGS_RadiativeSplitting::getBremsEnergies(int np, int npold) {
//a rough copy of getBremsEnergies from beampp.mortran
    EGS_Float eie, ekin, ese, esg, brmin, waux, r1,ajj,br,rnno06,rnno07,delta,phi1,phi2,rejf,x1,y1,aux;
    int nsample,ip,j,jj,l,l1, medium;
    EGS_Float peie,pese,pesg;

    nsample = np-npold;
    if (nsample < 1) nsample = 1;
    peie = app->top_p.E; //i.e. we have not added particles to the stack yet
    eie = peie;
    if (eie < 50.0) {
        l=1;
    }
    else
    {
        l=3;
    }
    l1 = l+1;
    ekin = peie - app->getRM(); brmin = app->the_thresh->ap[imed]/ekin;
    waux = app->the_epcont->elke - app->the_nist_brems->log_ap[imed];

    if (app->ibr_nist == 1)
    {
        ajj = 1 + (waux + app->the_nist_brems->log_ap[imed] - app->the_nist_brems->nb_lemin[imed])*app->the_nist_brems->nb_dlei[imed];
        jj = ajj; ajj = ajj - jj;
        if ( jj > MXBRES) {
           jj = MXBRES;
           ajj = -1;
        }
    }
    ip = npold;
    for (int ibr = 0; ibr < nsample; ibr++)
    {
        ip++;
        if (app->ibr_nist == 1)
        {
            if (ekin > app->the_nist_brems->nb_emin[imed])
            {
              r1 = rndm->getUniform();
              if (r1 < ajj) {
                     j = jj+1;
              }
              else
              {
                     j = jj;
              }
              //maybe should use egs++ alias_sample function here
              br = F77_OBJ(alias_sample1)(MXBRXS,app->the_nist_brems->nb_xdata[0,j,imed],
                                          app->the_nist_brems->nb_fdata[0,j,imed],
                                          app->the_nist_brems->nb_wdata[1,j,imed],
                                          app->the_nist_brems->nb_idata[i,j,imed]);
            }
            else
            {
              br = rndm->getUniform();
            }
            esg = app->the_epcont->ap[imed]*exp(br*waux);
            pesg = esg;
            pese = peie - pesg;
            ese = pese;
        }
        else
        {
          do
          {
           // use BH cross-sections
           rnno006 = rndm->getUniform();
           rnno007 = rndm->getUniform();
           br = brmin*exp(rnno06*waux);
           esg = ekin*br;
           pesg = esg;
           delta = esg/eie/ese*app->the_brempr->delcm[imed];
           aux = ese/eie;
           if (delta < 1)
           {
               phi1 = app->the_brempr->dl1[l,imed]+delta*
                      (app->the_brempr->dl2[l,imed]+app->the_brempr->dl3[l,imed]);
               phi2 = app->the_brempr->dl1[l1,imed]+delta*
                      (app->the_brempr->dl2[l1,imed]+app->the_brempr->dl3[l1,imed]);
           }
           else
           {
               phi1 = app->the_brempr->dl4[l,imed]+app->the_brempr->dl5[l,imed]*
                      log(delta + app->the_brempr->dl6[l,imed]);
               phi2 = phi1;
           }
           rejf = (1+aux*aux)*phi1 - 2*aux*phi2/3;
         } while (rnno07 >= rejf);
       }
       if (ip <= np)
       {
          particle_stack.E[ip] = pesg;
       }
   }
   particle_stack.E[npold] = pese;
}

void EGS_RadiativeSplitting::killThePhotons(EGS_Float fs, EGS_Float ssd, int n_split, int npstart, int kill_electrons) {
   //an adaptation of the Mortran subroutine kill_the_photons found in beamnrc.mortran, beampp.mortran
   if (npstart > app->Np) return;
   int i_playrr = 0;
   int idbs = npstart;
   do {
      EGS_Particle p = app->getParticleFromStack(idbs);
      //below is temporary until we figure out how to pass iweight
      int is_fat = (p.latch & (1 << 31));
      if (p.q == 0)
      {
         i_playrr = 0;
         if (!is_fat)
         {
            if (p.w < 0)
            {
               i_playrr = 1;
            }
            else
            {
               EGS_Float dist = (ssd - p.z)/p.w;
               EGS_Float r2 = (p.x+dist*p.u)*(p.x+dist*p.u) + (p.y+dist*p.v)*(p.y+dist*p.v);
               if (r2 > fs*fs) i_playrr = 1;
            }
         }
         if (rndm->getUniform()*i_playrr)
         {
            if (rndm->getUniform()*n_split > 1)
            {
                //kill the particle
                app->deleteParticleFromStack(idbs);
            }
            else
            {
                //keep the particle and increase weight
                //do this by saving the particle info, deleting the particle, changing the weight and adding it
                //back to the stack
                EGS_Particle p1 = app->getParticleFromStack(idbs);
                p1.wt = p1.wt*n_split;
                //set bit 31 of latch to mark as phat--temporary
                p1.latch = p1.latch | (1 << 31);
                app->deleteParticleFromStack(idbs);
                app->addParticleToStack(p1);
                idbs++;
            }
         }
         else
         {
            //keep the particle
            idbs++;
         }
     }
     else
     {
         //this is a charged particle
         if (kill_electrons == 0)
         {
            //keep it
            idbs++;
         }
         else
         {
            if (rndm->getUniform()*kill_electrons > 1)
            {
                //kill it
                //Note: this will never happen for now because kill_electrons = 0 or 1
                app->deleteParticleFromStack(idbs);
            }
            else
            {
                //keep the particle and increase weight
                EGS_Particle p2 = app->getParticleFromStack(idbs);
                p2.wt = p2.wt*kill_electrons;
                app->deleteParticleFromStack(idbs);
                app->addParticleToStack(p2);
            }
         }
      }
   } while (idbs <= app->Np);

   return;
}

void EGS_RadiativeSplitting::selectAzimuthalAngle(EGS_Float &cphi, EGS_FLOAT &sphi)
{
   //function to randomly select an azimuthal angle evenly distributed on 4pi
   //returns cos and sin of angle
   //C++ version of $SELECT-AZIMUTHAL-ANGLE macro in egsnrc.macros
   EGS_Float xphi,yphi,xphi2,yphi2,rhophi2;
   do {
      xphi = rndm->getUniform();
      xphi = 2*xphi - 1;
      xphi2 = xphi*xphi;
      yphi = rndm->getUniform();
      yphi2 = yphi*yphi;
      rhophi2 = xphi2 + yphi2;
   } while (rhophi2 > 1);
   rhophi2 = 1/rhophi2;
   cphi = (xphi2 - yphi2)*rhophi2;
   sphi = 2*xphi*yphi*rhophi2;
   return;
}

void EGS_RadiativeSplitting::uniformPhotons(int nsample, int n_split, EGS_Float fs, EGS_Float ssd, EGS_Float energy)
{
   //function to generate photons radiating isotropically from a point that are directed into the splitting
   //field defined by fs, ssd
   //modeled after the Mortran routine used in beamnrc, beampp

   //calculate min/max polar angles subtended by the splitting field
   EGS_Float x = app->top_p.x.x;
   EGS_Float y = app->top_p.x.y;
   EGS_Float ro = sqrt(x*x+y*y);
   EGS_Float d = ssd - app->top_p.x.z;
   EGS_Float aux = (ro + fs)/d;
   EGS_Float ct_min = 1/sqrt(1+aux*aux);
   if (ro <= fs)
   {
      ct_max = 1;
   }
   else
   {
      aux = (fs - ro)/d;
      ct_max = 1/sqrt(1+aux*aux);
   }

   //calculate fraction of nsample photons that will have polar angles between ct_min/ct_max
   EGS_Float an_split = 0.5*(ct_max - ct_min)*nsample;
   int num_split = an_split;
   an_split = an_split - num_split;
   if (rndm->getUniform() < an_split) num_split += 1;

   //now create the num_split photons
   //note: not all photons will go towards the circle and we will have to play russian roulette with these
   EGS_Particle p_old = app->getParticleFromStack(app->getNpold());
   EGS_Float weight = p_old.wt/n_split;
   EGS_Float cost, sint, cphi, sphi;
   int ns,ip;
   int np = app->Np-1;
   for (int i=0; i< num_split; i++)
   {
       EGS_particle p;
       cost = ct_min + rndm->getUniform()*(ct_max-ct_min);
       sint = 1-cost*cost;
       if (sint > 0)
       {
           sint = sqrt(sint);
       }
       else
       {
           sint = 0;
       }
       selectAzimuthalAngle(cphi,sphi);
       aux = d/cost*sint;
       x = p_old.x.x + aux*cphi;
       y = p_old.x.y + aux*sphi;
       ns = 0;
       if (x*x + y*y < fs*fs)
       {
           ns = 1;
       }
       else if (rndm->getUniform()*n_split < 1)
       {
           ns = n_split;
       }
       if (ns > 0)
       {
           np++;
           if (np > app->getNpold())
           {
              ip = np-1;
           }
           else
           {
              ip = app->getNpold();
           }
           EGS_Particle p_ip = app->getParticleFromStack(ip);
           p.latch = p_ip.latch;
           p.ir = p_ip.ir;
           p.x = p_ip.x;
           //stuff that we do not inherit
           p.q = 0;
           p.E = energy;
           p.wt = weight*ns;
           p.u = EGS_Vector(sint*cphi, sint*sphi, cost);

           app->addParticleToStack(p);
       }
   }

   //now generate (1-(ct_max-ct_min)/2*nsample photons directed outside the field
   //model this by generating nsample/num_split photons over all polar angles and
   //rejecting those with angles between ct_min and ct_max
   num_split = nsample/n_split;
   for (int i= 0; i<num_split; i++)
   {
       cost = 2*rndm->getUniform()-1;
       if (cost < ct_min || cost > ct_max)
       {
           sint = 1 - cost*cost;
           if (sint > 0 )
           {
               sint = sqrt(sint);
           }
           else
           {
               sint = 0;
           }
           selectAzimuthalAngle(cphi,sphi);
           np++;
           if (np > app->getNpold())
           {
              ip = np-1;
           }
           else
           {
              ip = app->getNpold();
           }
           EGS_Particle p_ip = app->getParticleFromStack(ip);
           p.latch = p_ip.latch;
           p.ir = p_ip.ir;
           p.x = p_ip.x;
           //stuff that we do not inherit
           p.q = 0;
           p.E = energy;
           p.wt = weight*n_split;
           p.u = EGS_Vector(sint*cphi, sint*sphi, cost);

           app->addParticleToStack(p);
       }
   }
   return;
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
