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
#  Contributors:    Blake Walters, 2021
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

//local structures required
struct DBS_Aux {
    int       np;
    EGS_Float E;
    EGS_Float Z13;
    EGS_Float le;
};

//local functions
EGS_Float dbs_func_KM(EGS_Float x, void *data) {
    DBS_Aux *the_aux = (DBS_Aux *)data; EGS_Float Z13 = the_aux->Z13;
    int np = the_aux->np;
    EGS_Float E = the_aux->E;
    EGS_Float k = (E-1)*x;
    EGS_Float Ep = E - k;
    EGS_Float r = Ep/E; EGS_Float delta = k/(2*E*Ep); delta *= delta;
    EGS_Float r2p1 = 1+r*r, rp12 = r2p1 + 2*r;
    EGS_Float beta = sqrt((E-1)*(E+1))/E;
    EGS_Float y2min = 0, y2max = 2*beta*(1+beta)*E*E;
    EGS_Float y2maxi = 1/y2max;
    EGS_Float a = sqrt(1+y2max);
    EGS_Float deta = 1./np;
    double sum = 0;
    for(int j=0; j<np; j++) {
        EGS_Float eta = deta*(0.5+j);
        EGS_Float tmp = a/(eta+(1-eta)*a);
        EGS_Float y2 = tmp*tmp-1;
        EGS_Float aux = y2 + 1; EGS_Float aux2 = aux*aux;
        EGS_Float M = delta + Z13/aux2;
        EGS_Float f = (16*y2*r/aux2 - rp12 - (r2p1 - 4*y2/aux2*r)*log(M))/tmp;
        sum += f;
    }
    return sum*deta;
}

EGS_Float dbs_func_KM1(EGS_Float u, void *data) {
    DBS_Aux *the_aux = (DBS_Aux *)data; EGS_Float Z13 = the_aux->Z13;
    int np = the_aux->np;
    EGS_Float E = the_aux->E;
    EGS_Float k = E - exp(u*the_aux->le);
    EGS_Float Ep = E - k;
    EGS_Float r = Ep/E; EGS_Float delta = k/(2*E*Ep); delta *= delta;
    EGS_Float r2p1 = 1+r*r, rp12 = r2p1 + 2*r;
    EGS_Float beta = sqrt((E-1)*(E+1))/E;
    EGS_Float y2min = 0, y2max = 2*beta*(1+beta)*E*E;
    EGS_Float y2maxi = 1/y2max;
    EGS_Float a = sqrt(1+y2max);
    EGS_Float deta = 1./np;
    double sum = 0;
    for(int j=0; j<np; j++) {
        EGS_Float eta = deta*(0.5+j);
        EGS_Float tmp = a/(eta+(1-eta)*a);
        EGS_Float y2 = tmp*tmp-1;
        EGS_Float aux = y2 + 1; EGS_Float aux2 = aux*aux;
        EGS_Float M = delta + Z13/aux2;
        EGS_Float f = (16*y2*r/aux2 - rp12 - (r2p1 - 4*y2/aux2*r)*log(M))/tmp;
        sum += f;
    }
    return sum*deta;
}

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

    if (( split_type == DRS || split_type == DRSf ) && app->getIbrdst())
    {
        initSmartKM(app->getEmax()*1.05);
    }

    description = "\n===========================================\n";
    description +=  "Radiative splitting Object (";
    description += name;
    description += ")\n";
    description += "===========================================\n";
    if (nsplit > 1) {
        description +="\n - Splitting radiative events in ";
        sprintf(buf,"%d",nsplit);
        description += buf;
        if      ( split_type == URS ){
                description +=" using URS\n\n";
        }
        else if ( split_type == DRS ) {
            description +=" using DRS\n\n";
        }
        else if ( split_type == DRSf ){
            description +=" using BEAMnrc-style DRS\n\n";
        }
        else {
            description +=" using an unknown splitting algorithm? :-(";
        }
    }
    if (nsplit == 1) {
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
}

//at least try to get brems working with this
int EGS_RadiativeSplitting::doInteractions(int iarg, int &killed)
{

    int check = 1; //set to 1 to return to shower

    killed = 0;

    if( iarg > EGS_Application::AfterTransport && app->top_p.x.z > ssd ) {
        //particle is past ssd, no splitting
        app->setRadiativeSplitting(1); return 0;
    }

    int np = app->getNp();
    EGS_Particle pi = app->getParticleFromStack(np);
    int latch = pi.latch;

    //use bit 0 to mark phat particles
    //seems like a temporary solution
    int is_fat = (latch & (1 << 0));

    if(pi.wt < 1 && is_fat) exit(1);

    EGS_Float tmp_dist = (ssd - app->top_p.x.z)/app->top_p.u.z;
    EGS_Float tmp_r = sqrt((app->top_p.x.x+tmp_dist*app->top_p.u.x)*(app->top_p.x.x+tmp_dist*app->top_p.u.x) +
                      (app->top_p.x.y+tmp_dist*app->top_p.u.y)*(app->top_p.x.y+tmp_dist*app->top_p.u.y));
    if(np==846 || np==610 )
    {
    egsInformation("np=%d iarg=%d iq=%d E=%g wt=%g ir=%d is_fat=%d x=%g y=%g z=%g u=%g v=%g w=%g\n",np,iarg,pi.q,pi.E,pi.wt,pi.ir,is_fat,pi.x.x,pi.x.y,pi.x.z,pi.u.x,pi.u.y,pi.u.z);
    }

    if( iarg == EGS_Application::BeforeTransport)
    {
        return 0;
    }

    if( iarg == EGS_Application::BeforeBrems ) {
        double E = pi.E;
        EGS_Float wt = pi.wt;
        if( is_fat ) {
            //clear bit 0
            latch = latch & ~(1 << 0);
            //reset latch value of top particle
            app->setLatch(latch);
            app->top_p.latch=latch; //have to set this because it is used in doSmartBrems
            //is the next line necessary?
            app->setRadiativeSplitting(nsplit);
            int npold = np;
            int res = doSmartBrems();
            if( res ) {
                app->callBrems();
                int nstart = np+1, aux=0;
                killThePhotons(fs,ssd,nsplit,nstart,aux);
            }
            for (int i=npold; i<=app->getNp(); i++)
            {
                 EGS_Particle ptest = app->getParticleFromStack(i);
                 EGS_Float tmp_dist = (0.01001 - ptest.x.z)/ptest.u.z;
                 EGS_Float tmp_dist1 = (ssd - ptest.x.z)/ptest.u.z;
                 EGS_Float tmp_r = sqrt((ptest.x.x+tmp_dist*ptest.u.x)*(ptest.x.x+tmp_dist*ptest.u.x) +
                      (ptest.x.y+tmp_dist*ptest.u.y)*(ptest.x.y+tmp_dist*ptest.u.y));
                 EGS_Float tmp_r1 = sqrt((ptest.x.x+tmp_dist1*ptest.u.x)*(ptest.x.x+tmp_dist1*ptest.u.x) +
                      (ptest.x.y+tmp_dist1*ptest.u.y)*(ptest.x.y+tmp_dist1*ptest.u.y));
                 //egsInformation("post brems: i=%d iq=%d E=%g wt=%g r=%g r1=%g\n",i,ptest.q,ptest.E,ptest.wt,tmp_r,tmp_r1);
            }
            //we need to relable the interacting e- as fat
            //at this point np holds the e-
            EGS_Particle p = app->getParticleFromStack(np);
            latch = latch | (1 << 0);
            p.latch = latch;
            EGS_Float dnear = app->getDnear(np);
            app->updateParticleOnStack(np,p,dnear);
        }
        else {
            app->setRadiativeSplitting(1);
            app->callBrems();
            int nstart = np+1, aux=0;
            killThePhotons(fs,ssd,nsplit,nstart,aux);
        }
        check = 1;
    }
    else if( iarg == EGS_Application::BeforeAnnihFlight ) {
        if( is_fat ) {
            //figure out what to do with the extra stack
            //the_extra_stack->iweight[np]=1;
            //set interacting particle to nonphat so this is
            //passed on to resultant photons
            //TODO: figure out a better way to do this that does not use latch
            latch = latch & ~(1 << 0);
            app->setLatch(latch);
            app->setRadiativeSplitting(nsplit);
        }
        else app->setRadiativeSplitting(1);
        app->callAnnih();
        int nstart=np+1,aux=0;
        killThePhotons(fs,ssd,nsplit,nstart,aux);
        check = 1;
    }
    else if( iarg == EGS_Application::BeforeAnnihRest ) {
        if( is_fat ) {
            //app->setNpold(np);not sure why this is necessary--unless it is intended to ensure
                              //overwriting of the annihilated positron, in which case, we should
                              //actually be setting npold=np-1
            app->setRadiativeSplitting(nsplit);//this also seems like a formality
            int nsamp = 2*nsplit;
            //label photons as nonphat
            latch = latch & ~(1 << 0);
            app->setLatch(latch);
            uniformPhotons(nsamp,nsplit,fs,ssd,app->getRM());
        }
        else {
            app->setRadiativeSplitting(1);
            app->callAnnihAtRest();
            int nstart=np+1,aux=0;
            killThePhotons(fs,ssd,nsplit,nstart,aux);
        }
        check = 1;
    }
    if( iarg == EGS_Application::BeforeCompton ||
             iarg == EGS_Application::BeforePair ||
             iarg == EGS_Application::BeforePhoto ||
             iarg == EGS_Application::BeforeRayleigh )
    {

        //currently only phat photons undergo interactions
        //unless it is bound compton
        if(!is_fat && (iarg != EGS_Application::BeforeCompton ||
            (iarg == EGS_Application::BeforeCompton && !app->getIbcmp())))
        {
               EGS_Particle p = app->getParticleFromStack(app->getNp());
               EGS_Float dnear = app->getDnear(app->getNp());
               if (app->getRngUniform()*nsplit > 1)
               {
                    //send interacting particle back with zero wt
                    p.wt = 0;
                    app->updateParticleOnStack(app->getNp(),p,dnear);
                    return 0;
               }
               else
               {
                    //keep the photon, increase weight and label as phat
                    p.wt = p.wt*nsplit;
                    p.latch = p.latch | (1 << 0);
                    app->updateParticleOnStack(app->getNp(),p,dnear);
                    is_fat = 1;
               }
           if(np==610) {
             egsInformation("about to: iarg=%d getNp=%d iq=%d E=%g wt=%g u=%g v=%g w=%g x=%g y=%g z=%g\n",iarg,app->getNp(),p.q,p.E,p.wt,p.u.x,p.u.y,p.u.z,p.x.x,p.x.y,p.x.z);
           }
        }

        int nint = is_fat ? nsplit : 1;

        if(iarg == EGS_Application::BeforePair)
        {
            app->callPair();
        }
        if(iarg == EGS_Application::BeforePhoto)
        {
            app->callPhoto();
        }
        else if(iarg == EGS_Application::BeforeCompton)
        {
            if(is_fat && !app->getIbcmp())
            {
                //label as nonphat to be passed on to descendents
                latch = latch & ~(1 << 0);
                EGS_Particle p = app->getParticleFromStack(np);
                p.latch = latch;
                EGS_Float dnear = app->getDnear(np);
                app->updateParticleOnStack(np,p,dnear);
            //    EGS_Particle p_tmp = app->getParticleFromStack(app->getNp());
     // egsInformation("about to doSC: np=%d, iq=%d, E=%g, wt=%g\n",app->getNp(),p_tmp.q,p_tmp.E,p_tmp.wt);
                doSmartCompton(nint);
          //      p_tmp = app->getParticleFromStack(app->getNp());
      //egsInformation("after doSC: np=%d, iq=%d, E=%g, wt=%g\n",app->getNp(),p_tmp.q,p_tmp.E,p_tmp.wt);
            }
            else //straight-up compton
            {
                //label interacting photon as non-phat and reduce weight (if phat)
                latch = latch & ~(1 << 0);
                EGS_Particle p = app->getParticleFromStack(np);
                p.latch = latch;
                p.wt = p.wt/nint;
                EGS_Float dnear = app->getDnear(np);
                app->updateParticleOnStack(np,p,dnear);
                for (int i=0; i<nint; i++)
                {
                    if (i>0)
                    {
                       // put a copy of the interacting photon on top of the stack
                       app->addParticleToStack(p,dnear);
                    }
                    app->callCompt();

                }
                // kill photons not aimed into the field and any electrons
                int nstart = np, aux=1;
                killThePhotons(fs,ssd,nsplit,nstart,aux);
            }
        }
        else if (iarg == EGS_Application::BeforeRayleigh)
        {
            //TODO: Put all of this in a doRayleigh function
            if (is_fat) {
                    //this should always be true
                    latch = latch & ~(1 << 0);
                    app->setLatch(latch);
            }
            EGS_Particle pi = app->getParticleFromStack(app->getNp());
            int npstart = app->getNp();
            EGS_Float dnear = app->getDnear(app->getNp());
            EGS_Float gle = app->getGle();
            int imed = app->getMedium(pi.ir);
            int lgle = app->getLgle(gle,imed);
            EGS_Float costhe, sinthe;
            //delete the particle from the top of the stack because we are going to replace it
            app->deleteParticleFromStack(app->getNp());
            for (int i=0; i<nint; i++)
            {
                EGS_Particle p = pi;
                p.wt = pi.wt/nint;
                //probably don't need to set latch here
                p.latch = latch;
                //call EGS rayleigh sampling routine to get scatter angles cost, sint
                app->callEgsRayleighSampling(imed,p.E,gle,lgle,costhe,sinthe);
                //adjust scatter angles and apply to particle
                doUphi21(sinthe,costhe,p.u);
                //add the particle to the stack
       //egsInformation("rayleigh: np=%d i=%d iq=%d E=%g wt=%g ir=%d sinthe=%g costhe=%g u=%g v=%g w=%g\n",np,i,p.q,p.E,p.wt,p.ir,sinthe,costhe,p.u.x,p.u.y,p.u.z);
                app->addParticleToStack(p,dnear);
       if (app->getNp()+i==846) {
             egsInformation("rayleigh: npstart=%d np=%d i=%d iq=%d E=%g wt=%g ir=%d sinthe=%g costhe=%g u=%g v=%g w=%g x=%g y=%g z=%g wti=%g nint=%d\n",npstart,app->getNp()+i,i,p.q,p.E,p.wt,p.ir,sinthe,costhe,p.u.x,p.u.y,p.u.z,p.x.x,p.x.y,p.x.z,pi.wt,nint);
       }
                //now potentially kill it -- seems like we should kill the particle before adding to the stack
                int nstart = app->getNp(), aux=0;
                killThePhotons(fs,ssd,nsplit,nstart,aux);
            }
        }
        check = 1;
    }
    /*
    else if(iarg == EGS_Application::FluorescentEvent )
    {
        if( is_fat ) {
            EGS_Float ener = app->top_p.E;
            //label photons as nonphat
            latch = latch & ~(1 << 0);
            app->setLatch(latch);
            uniformPhotons(nsplit,nsplit,fs,ssd,ener);
        }
        else {
            int nstart=np, aux=0;
            killThePhotons(fs,ssd,nsplit,nstart,aux);
        }
        // Do not need to return to shower
        check = 2;
    }
    */

    if( check ) {
        if( check == 1 ) {
            if( app->getNp() >= app->getMxstack() ) egsFatal("Stack size exceeded "
              "in EGS_RadiativeSplitting::doInteractions()\n");
            //set weight of particle on top of stack to 0
            EGS_Particle p = app->getParticleFromStack(app->getNp());
            EGS_Float dnear = app->getDnear(app->getNp());
            p.wt = 0.;
            app->addParticleToStack(p,dnear);
        }
        return 0;
    }

    return 1;
}

int EGS_RadiativeSplitting::doSmartBrems() {
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
    int ir = app->top_p.ir;

    EGS_Float ct_min,ct_max,ro;
    getCostMinMax(x,u,ro,ct_min,ct_max);
    imed = app->getMedium(ir);
    EGS_Float f_max_KM = 1, q_KM, p_KM; int j_KM;

    if(app->getIbrdst() == 1) {
        q_KM = a_KM[imed]*log(ener) + b_KM[imed];
        j_KM = (int) q_KM; q_KM -= j_KM; p_KM = 1 - q_KM;
        f_max_KM = f_KM_max[imed].interpolateFast(j_KM,log(ener));
    }

    EGS_Float w1, cmin, cmax;
    if( app->getIbrdst() == 1 ) {
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
    if( app->getIbrdst() == 1 ) {
        w2 *= beta*sqrt(1+beta)/(2*(1-beta*ct_max)*sqrt(1-beta*ct_max)*
                ((1+beta)*(tau+1)-1));
    }
    else {
       aux = (tau+1)*(1-beta*ct_max);
       w2 /= (2*aux*aux);
    }
    w2 *= f_max_KM;
    EGS_Float wprob = min(w1,w2);
    if( wprob >= 1 && app->getIbrdst() == 1 ) return 1;
    int nsample;
    if( wprob > 1 ) { ct_min = -1; ct_max = 1; wprob = 1; }
    EGS_Float asample = wprob*nbrspl;
    nsample = (int) asample; asample -= nsample;
    if( app->getRngUniform() < asample ) ++nsample;

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
    EGS_Float dnear = app->getDnear(np); int ib = 0;

    if( w1 < w2 ) {
        for(int j=0; j<nsample; j++) {
            EGS_Float rnno = app->getRngUniform(), cost;
            if( app->getIbrdst() == 1 ) {
                EGS_Float tmp = cmin*cmax/(rnno*cmin+(1-rnno)*cmax);
                cost = (1 - tmp*tmp)/beta;
            }
            else {
                rnno *= aux1;
                cost = (ct_min*aux2+rnno)/(aux2+beta*rnno);
            }
            EGS_Float sint = 1 - cost*cost;
            sint = sint > 0 ? sqrt(sint) : 0;
            EGS_Float cphi,sphi; app->getRngAzimuth(cphi,sphi);
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
                if( app->getRngUniform()*nbrspl < 1 ) ns = nbrspl;
            }
            if( ns > 0 ) {
                if( ++ip >= app->getMxstack() ) egsFatal(dbs_err_msg,
                        "smartBrems",app->getMxstack());
                if( app->getIbrdst() == 1 )
                    y2_KM[ib++] = beta*(1+beta)*(tau+1)*(tau+1)*(1-cost);

                EGS_Particle p;
                p.x = x;
                p.u = EGS_Vector(un,vn,wn);
                p.ir = irl;
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
            //if( will_rotate ) rsamp->getPoint(fs,x1,y1,iw);
            //else {
                do {
                    x1 = 2*app->getRngUniform()-1; y1 = 2*app->getRngUniform()-1;
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
            if( app->getIbrdst() == 1 ) {
                rejf = aux2/(1-beta*cost); rejf *= sqrt(rejf)*aux*aux*aux;
            }
            else {
                rejf = aux2/(1-beta*cost)*aux;
                rejf *= rejf*aux;
            }

            if( app->getRngUniform() < rejf ) {
                if( ++ip >= app->getMxstack() ) egsFatal(dbs_err_msg,
                        "smartBrems",app->getMxstack());
                if( app->getIbrdst() == 1 ) {
                    EGS_Float y2 = beta*(1+beta)*(tau+1)*(tau+1)*(1-cost);
                    if( y2 < 0 ) y2 = 0;
                    y2_KM[ib++] = y2;
                }

                EGS_Particle p;
                p.x = x;
                p.u = EGS_Vector(un,vn,wn);
                p.ir = irl;
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
    //if( be_factor > 1 ) real_brems = be_factor*app->getRngUniform() < 1;
    int ireal = -1;
    if( real_brems ) {
        bool take_it = true; EGS_Float un=0,vn=0,wn=1;
        if( app->getIbrdst() != 1 ) {
            aux1 = 2; aux2 = 1 - beta; EGS_Float rnno = app->getRngUniform()*aux1;
            EGS_Float cost = (rnno-aux2)/(aux2+beta*rnno);
            if( w2 <= w1 || ((cost < ct_min || cost > ct_max) && w1 < w2) ) {
                EGS_Float sint = 1 - cost*cost;
                sint = sint > 0 ? sqrt(sint) : 0;
                EGS_Float cphi,sphi; app->getRngAzimuth(cphi,sphi);
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
            //on the stack if IBRDST!=1
            //otherwise, put a high weight photon on the stack with
            //u=0, v=0, w=1 (i.e. directed into the splitting field) as in beampp_dbs!
            //TODO: Figure out if this is correct or if this should go inside the
            //condition, if ibrdst!=1
            if( ++ip >= app->getMxstack() ) egsFatal(dbs_err_msg,
                    "smartBrems",app->getMxstack());
            ireal = ip-np-1; //index of this high weight particle in the local particle_stack

            EGS_Particle p;
            p.x = x;
            p.u = EGS_Vector(un,vn,wn);
            p.ir = irl;
            p.wt = wt*nbrspl;
            //label the particle as phat
            p.latch = latch | (1 << 0);
            p.q = 0;

            particle_stack.emplace_back(p);
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
        if( !real_brems ) particle_stack[ip].E = E;
        if( ip > np && app->getIbrdst() == 1 ) {
            ib = 0; EGS_Float E = tau+1; EGS_Float unorm = log(0.5*(E+1));
            for(int j=0; j<ip-np; j++) {
                //note that this loop potentially includes high-weight particle put on stack
                //above. Should reassign direction if ibrdst=1, right?
                EGS_Float k = particle_stack[j].E/app->getRM();
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
                    if( app->getRngUniform() > rejf ) {
                        particle_stack[j].wt = 0; particle_stack[j].E = 0;
                    }
                }
                else {
                    // this is the single high-weight photon
                    // now that we know the photon energy, we must sample
                    // a direction for this photon from 2BS
                    EGS_Float arg3 = -log(delta+zbr_KM[imed]);
                    EGS_Float rejmax = arg1*arg3-arg2;
                    EGS_Float y2max = 2*beta*(1+beta)*E*E;
                    EGS_Float z2maxi = sqrt(1+y2max);
                    EGS_Float rejf,y2,rnno;
                    do {
                        rnno = app->getRngUniform();
                        y2 = app->getRngUniform();
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
                        EGS_Float cphi,sphi; app->getRngAzimuth(cphi,sphi);
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
                        particle_stack[j].u = EGS_Vector(un,vn,wn);
                    }
                    else {
                        particle_stack[j].wt = 0; particle_stack[j].E = 0;
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
    double st = sqrt(st2);

    // handle odd cases st=0 and/or ro=0
    if( st2 < 1e-10 ) {
        double dmin = sqrt(d2 + (r-ro)*(r-ro)),
               dmax = sqrt(d2 + (r+ro)*(r+ro));
        if( w > 0 ) { ct_max = ro <= r ? 1 : d*w/dmin; ct_min = d*w/dmax; }
        else        { ct_max = d*w/dmax; ct_min = ro <= r ? -1 : d*w/dmin; }
        return;
    }
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
    int nsample,ip,j,jj,l,l1, imed;
    EGS_Float peie,pese,pesg;

    EGS_Vector x = app->top_p.x;
    int irl = app->top_p.ir;
    imed = app->getMedium(irl);

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
    ekin = peie - app->getRM(); brmin = app->getAp(imed)/ekin;
    waux = log(ekin) - log(app->getAp(imed));

    if (app->getIbrnist() == 1)
    {
        ajj = 1 + (waux + log(app->getAp(imed)) - app->getNbLemin(imed))*app->getNbDlei(imed);
        jj = ajj; ajj = ajj - jj;
        if ( jj > app->getMxbres()) {
           jj = app->getMxbres();
           ajj = -1;
        }
    }
    ip = 0;
    for (int ibr = 0; ibr < nsample; ibr++)
    {
        if (app->getIbrnist() == 1)
        {
            if (ekin > app->getNbEmin(imed))
            {
              r1 = app->getRngUniform();
              if (r1 < ajj) {
                     j = jj+1;
              }
              else
              {
                     j = jj;
              }
              //maybe should use egs++ alias_sample function here
              int mxbrxs = app->getMxbrxs();
              EGS_Float* f1 = app->getNbXdata(j,imed);
              EGS_Float* f2 = app->getNbFdata(j,imed);
              EGS_Float* f3 = app->getNbWdata(j,imed);
              int* f4 = app->getNbIdata(j,imed);
              br = app->callAliasSample1(mxbrxs,f1,f2,f3,f4);
            }
            else
            {
              br = app->getRngUniform();
            }
            esg = app->getAp(imed)*exp(br*waux);
            pesg = esg;
            pese = peie - pesg;
            ese = pese;
        }
        else
        {
          do
          {
           // use BH cross-sections
           rnno06 = app->getRngUniform();
           rnno07 = app->getRngUniform();
           br = brmin*exp(rnno06*waux);
           esg = ekin*br;
           pesg = esg;
           pese = peie - pesg;
           ese = pese;
           delta = esg/eie/ese*app->getDelcm(imed);
           aux = ese/eie;
           if (delta < 1)
           {
               phi1 = app->getDl1(l,imed)+delta*
                      (app->getDl2(l,imed)+app->getDl3(l,imed));
               phi2 = app->getDl1(l1,imed)+delta*
                      (app->getDl2(l1,imed)+app->getDl3(l1,imed));
           }
           else
           {
               phi1 = app->getDl4(l,imed)+app->getDl5(l,imed)*
                      log(delta + app->getDl6(l,imed));
               phi2 = phi1;
           }
           rejf = (1+aux*aux)*phi1 - 2*aux*phi2/3;
         } while (rnno07 >= rejf);
       }
       if (ip+npold <= np)
       {
          particle_stack[ip].E = pesg;
       }
       ip++; //why use this counter in particular
   }
   //now set energy of initiating e-
   EGS_Particle p = app->top_p;
   p.E = pese;
   EGS_Float dnear = app->getDnear(app->Np);
   app->updateParticleOnStack(npold,p,dnear);
}

void EGS_RadiativeSplitting::killThePhotons(EGS_Float fs, EGS_Float ssd, int n_split, int npstart, int kill_electrons) {
   //an adaptation of the Mortran subroutine kill_the_photons found in beamnrc.mortran, beampp.mortran
   if (npstart > app->getNp()) return;
   int i_playrr = 0;
   int idbs = npstart;
   EGS_Float dnear = app->getDnear(idbs);
   do {
      EGS_Particle p = app->getParticleFromStack(idbs);
      if (p.q == 0)
      {
         i_playrr = 0;
         if (p.u.z < 0)
         {
               i_playrr = 1;
         }
         else
         {
               EGS_Float dist = (ssd - p.x.z)/p.u.z;
               EGS_Float r2 = (p.x.x+dist*p.u.x)*(p.x.x+dist*p.u.x) + (p.x.y+dist*p.u.y)*(p.x.y+dist*p.u.y);
               if (r2 > fs*fs) i_playrr = 1;
         }
         if (i_playrr)
         {
            if (app->getRngUniform()*n_split > 1)
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
                //set bit 0 of latch to mark as phat--temporary
                p1.latch = p1.latch | (1 << 0);
                app->updateParticleOnStack(idbs,p1,dnear);
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
            if (app->getRngUniform()*n_split > 1)
            {
                //kill it
                app->deleteParticleFromStack(idbs);
            }
            else
            {
                //keep the particle, increase weight and label as phat
                EGS_Particle p2 = app->getParticleFromStack(idbs);
                p2.wt = p2.wt*n_split;
                p2.latch = p2.latch | (1 << 0);
                app->updateParticleOnStack(idbs,p2,dnear);
                idbs++;
            }
         }
      }
   } while (idbs <= app->getNp());

   return;
}

void EGS_RadiativeSplitting::selectAzimuthalAngle(EGS_Float &cphi, EGS_Float &sphi)
{
   //function to randomly select an azimuthal angle evenly distributed on 4pi
   //returns cos and sin of angle
   //C++ version of $SELECT-AZIMUTHAL-ANGLE macro in egsnrc.macros
   EGS_Float xphi,yphi,xphi2,yphi2,rhophi2;
   do {
      xphi = app->getRngUniform();
      xphi = 2*xphi - 1;
      xphi2 = xphi*xphi;
      yphi = app->getRngUniform();
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

   //get properties of interacting particle (annihilating positron or radiative photon being split)
   EGS_Particle p_ip = app->getParticleFromStack(app->getNp());
   EGS_Float x = p_ip.x.x;
   EGS_Float y = p_ip.x.y;
   EGS_Float z = p_ip.x.z;
   EGS_Float dnear = app->getDnear(app->Np);
   EGS_Float weight = p_ip.wt/n_split;

   //calculate min/max polar angles subtended by the splitting field
   EGS_Float ro = sqrt(x*x+y*y);
   EGS_Float d = ssd - z;
   EGS_Float aux = (ro + fs)/d;
   EGS_Float ct_min = 1/sqrt(1+aux*aux);
   EGS_Float ct_max;
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
   if (app->getRngUniform() < an_split) num_split += 1;

   //now create the num_split photons
   //note: not all photons will go towards the circle and we will have to play russian roulette with these
   EGS_Float cost, sint, cphi, sphi;
   int ns,ip;
   //delete interacting particle on stack so that we overwrite it
   app->deleteParticleFromStack(app->getNp());
   //and update np
   int np = app->getNp();
   for (int i=0; i< num_split; i++)
   {
       EGS_Particle p;
       cost = ct_min + app->getRngUniform()*(ct_max-ct_min);
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
       EGS_Float x_tmp = x + aux*cphi;
       EGS_Float y_tmp = y + aux*sphi;
       ns = 0;
       if (x_tmp*x_tmp + y_tmp*y_tmp < fs*fs)
       {
           ns = 1;
       }
       else
       {
          //this is directed outside the field, play RR in next if block
          ns = nsplit;
       }
       if (ns > 1)
       {
          if(app->getRngUniform()*ns>1)
          {
             ns=0;
          }
       }
       if (ns > 0)
       {
           np++;
           p.latch = p_ip.latch;
           if (ns > 1)
           {
              // going outside field, label as phat
              p.latch = p.latch | (1 << 0);
           }
           p.ir = p_ip.ir;
           p.x = p_ip.x;
           //stuff that we do not inherit
           p.q = 0;
           p.E = energy;
           p.wt = weight*ns;
           p.u = EGS_Vector(sint*cphi, sint*sphi, cost);

           EGS_Float dist = (ssd - p.x.z)/p.u.z;
           EGS_Float r = sqrt((p.x.x+dist*p.u.x)*(p.x.x+dist*p.u.x) + (p.x.y+dist*p.u.y)*(p.x.y+dist*p.u.y));
           app->addParticleToStack(p,dnear);
       }
   }

   //now generate (1-(ct_max-ct_min)/2*nsample photons directed outside the field
   //model this by generating nsample/num_split photons over all polar angles and
   //rejecting those with angles between ct_min and ct_max
   num_split = nsample/n_split;
   for (int i= 0; i<num_split; i++)
   {
       cost = 2*app->getRngUniform()-1;
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
           EGS_Particle p;
           //label particle as phat
           p.latch = p_ip.latch | (1 << 0);
           p.ir = p_ip.ir;
           p.x = p_ip.x;
           //stuff that we do not inherit
           p.q = 0;
           p.E = energy;
           p.wt = weight*n_split;
           p.u = EGS_Vector(sint*cphi, sint*sphi, cost);
           app->addParticleToStack(p,dnear);
       }
   }
   return;
}

void EGS_RadiativeSplitting::doSmartCompton(int nint)
{

   //This is based on the beampp implementation

   //get properties of interacting particle
   int np = app->getNp();
   EGS_Particle p_init = app->getParticleFromStack(np);
   EGS_Float E = p_init.E;
   EGS_Vector x = p_init.x;
   EGS_Vector u = p_init.u;
   int irl=p_init.ir, latch=p_init.latch;
   imed = app->getMedium(irl);
   EGS_Float AP = app->getAp(imed);
   EGS_Float dnear = app->getDnear(np);
   //reduce weight of split particles
   EGS_Float wt = p_init.wt/nint;

   EGS_Float ct_min,ct_max,ro; EGS_Float d = ssd - x.z;
   getCostMinMax(x,u,ro,ct_min,ct_max);

   EGS_Float Ko = E/app->getRM();
   EGS_Float broi = 1+2*Ko, Ko2 = Ko*Ko;

   //
   // calculate probability for method 1: picking points within
   // the circle and rejecting with probability sigma(direction)/sigma_max
   //
   EGS_Float dmin = ro <= fs ? d : sqrt(d*d + (ro-fs)*(ro-fs));
   EGS_Float wnew = fs*fs*d/(2*dmin*dmin*dmin);
   EGS_Float alpha1_t = log(broi);
   EGS_Float eps1_t = 1/broi, eps2_t = 1;
   EGS_Float w2 = alpha1_t*(Ko2-2*Ko-2)+(eps2_t-eps1_t)*
       (1./eps1_t/eps2_t + broi + Ko2*(eps1_t+eps2_t)/2);
   EGS_Float fnorm = w2/(Ko2*Ko);
   EGS_Float a = 1 + Ko*(1-ct_max); EGS_Float a2 = a*a;
   EGS_Float f1 = (1/a + a - 1 + ct_max*ct_max)/a2;
   a = 1 + Ko*(1-ct_min); a2 = a*a;
   EGS_Float f2 = (1/a + a - 1 + ct_min*ct_min)/a2;
   EGS_Float fmax = f1 > f2 ? f1 : f2;
   wnew *= fmax/fnorm;

   //
   // calculate probability for method 2: picking directions
   // between ct_min and ct_max and rejecting those not going towards
   // the circle
   //
   EGS_Float eps1 = 1/(1+Ko*(1-ct_min)), eps2 = 1/(1+Ko*(1-ct_max));
   EGS_Float alpha1 = log(eps2/eps1);
   EGS_Float w1 = alpha1*(Ko2-2*Ko-2)+(eps2-eps1)*(1./eps1/eps2 + broi
                + Ko2*(eps1+eps2)/2);
   EGS_Float wc = w1/w2;

   //
   // number of interactions to sample
   //
   bool method1; EGS_Float wprob;
   if( wnew <= wc ) { method1 = true; wprob = wnew; }
   else { method1 = false; wprob = wc; }
   EGS_Float asample = wprob*nint; int nsample = (int) asample;
   asample -= nsample; if( app->getRngUniform() < asample ) ++nsample;


   // prepare rotations--not totally sure why this is needed
   EGS_Float sinpsi, sindel, cosdel; bool need_rotation;
   sinpsi = u.x*u.x + u.y*u.y;
   if( sinpsi > 1e-20 ) {
       sinpsi = sqrt(sinpsi); need_rotation = true;
       cosdel = u.x/sinpsi;
       sindel = u.y/sinpsi;
   } else need_rotation = false;

   //
   // sample interactions towards circle
   //
   app->deleteParticleFromStack(np); //delete interacting particle
   if( method1 ) {
        for(int j=0; j<nsample; j++) {
            EGS_Float x1, y1; int iw;
            do {
                    x1 = 2*app->getRngUniform()-1; y1 = 2*app->getRngUniform()-1;
            } while ( x1*x1 + y1*y1 > 1 );
            x1 *= fs; y1 *= fs; iw = 1;
            EGS_Float un = x1 - x.x, vn = y1 - x.y, wn = d;
            EGS_Float dist = sqrt(un*un + vn*vn + wn*wn);
            EGS_Float disti = 1/dist;
            un *= disti; vn *= disti; wn *= disti;
            EGS_Float cost = u.x*un + u.y*vn + u.z*wn;
            EGS_Float aux = dmin*disti;
            a = 1/(1 + Ko*(1-cost));
            if( E*a > AP ) {
                EGS_Float frej = (1/a+a-1+cost*cost)*a*a*aux*aux*aux;
                if( app->getRngUniform()*fmax < frej ) {
                    EGS_Particle p;
                    p.x=x;
                    p.u = EGS_Vector(un,vn,wn);
                    p.ir=irl;
                    p.wt=wt;
                    p.latch=latch;
                    p.q=0;
                    p.E=E*a;
                    app->addParticleToStack(p,dnear);
                }
            }
        }
    }
    else {
        EGS_Float eps12 = eps1*eps1, alpha2 = (eps2*eps2-eps12);
        EGS_Float alpha = alpha1/(alpha1+alpha2/2);
        EGS_Float rej1 = 1-(1-eps1)*(broi*eps1-1)/(Ko*Ko*eps1*(1+eps1*eps1));
        EGS_Float rej2 = 1-(1-eps2)*(broi*eps2-1)/(Ko*Ko*eps2*(1+eps2*eps2));
        EGS_Float rejmax = max(rej1,rej2);
        for(int j=0; j<nsample; j++) {
            EGS_Float br,temp, cost, sint, rejf;
            do {
                if( app->getRngUniform() < alpha )
                    br = eps1*exp(alpha1*app->getRngUniform());
                else
                    br = sqrt(eps12 + app->getRngUniform()*alpha2);
                temp = (1-br)/(Ko*br); sint = temp*(2-temp);
                rejf = 1 - br*sint/(1+br*br);
            } while ( app->getRngUniform()*rejmax > rejf || sint < 0 );
            if( E*br > AP ) {
                cost = 1 - temp; sint = sqrt(sint);
                EGS_Float cphi,sphi;
                app->getRngAzimuth(cphi,sphi);
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
                if( ns > 0 ) {
                    EGS_Particle p;
                    p.x=x;
                    p.u = EGS_Vector(un,vn,wn);
                    p.ir=irl;
                    p.wt=wt;
                    p.latch=latch;
                    p.q=0;
                    p.E=E*a;
                    app->addParticleToStack(p,dnear);
                }
            }
        }
    }

    // get potential high weight photon directed away from the field and compton electron.

    eps1 = eps1_t; eps2 = eps2_t; alpha1 = alpha1_t;
    EGS_Float eps12 = eps1*eps1; EGS_Float alpha2 = (eps2*eps2-eps12);
    EGS_Float alpha = alpha1/(alpha1+alpha2/2);
    EGS_Float br,temp, cost, sint, rejf;
    do {
            if( app->getRngUniform() < alpha )
                br = eps1*exp(alpha1*app->getRngUniform());
            else
                br = sqrt(eps12 + app->getRngUniform()*alpha2);
            temp = (1-br)/(Ko*br); sint = temp*(2-temp);
            rejf = 1 - br*sint/(1+br*br);
    } while ( app->getRngUniform() > rejf || sint < 0 );
    cost = 1 - temp; sint = sqrt(sint);
    EGS_Float cphi,sphi; app->getRngAzimuth(cphi,sphi);
    if( E*br > AP) {
            //this is a potential high wt photon directed away from the field
            EGS_Float un,vn,wn;
            if( need_rotation ) {
                EGS_Float us = sint*cphi, vs = sint*sphi;
                un = u.z*cosdel*us - sindel*vs + u.x*cost;
                vn = u.z*sindel*us + cosdel*vs + u.y*cost;
                wn = u.z*cost - sinpsi*us;
            } else { un = sint*cphi; vn = sint*sphi; wn = u.z*cost; }
            bool take_it = true;
            if( wn > 0) {
                EGS_Float t = (ssd-x.z)/wn;
                EGS_Float x1 = x.x + un*t, y1 = x.y + vn*t;
                if( x1*x1 + y1*y1 <= fs*fs ) take_it = false;
            }
            if( take_it ) {
                EGS_Particle p;
                p.x=x;
                p.u=EGS_Vector(un,vn,wn);
                p.wt=wt*nint;
                p.latch=latch | (1 << 0);
                p.ir=irl;
                p.q=0; p.E = E*br;
                app->addParticleToStack(p,dnear);
            }
   }
   //now the electron--Note: br, cost will have the values set for the phat photon
   EGS_Float Eelec = E*(1-br);
   EGS_Float aux = 1 + br*br - 2*br*cost;
   EGS_Float un=0,vn=0,wn=1;
   if( aux > 1e-8 ) {
            cost = (1-br*cost)/sqrt(aux); sint = 1-cost*cost;
            if( sint > 0 ) sint = -sqrt(sint); else sint = 0;
            EGS_Float us = sint*cphi, vs = sint*sphi;
            un = u.z*cosdel*us - sindel*vs + u.x*cost;
            vn = u.z*sindel*us + cosdel*vs + u.y*cost;
            wn = u.z*cost - sinpsi*us;
   }
   EGS_Particle p;
   p.x = x;
   p.u = EGS_Vector(un,vn,wn);
   p.ir = irl;
   p.wt = wt*nint;
   p.latch = latch | (1 << 0);
   p.q = -1;
   p.E = Eelec + app->getRM();
   app->addParticleToStack(p,dnear);
}

void EGS_RadiativeSplitting::initSmartKM(EGS_Float Emax) {

      int nmed = app->getNmed();
      if( nmed < 1 ) return;
      EGS_Float logEmax = log(Emax);

      y2_KM = new EGS_Float [nsplit];
      f_KM_a = new EGS_Interpolator* [nmed];
      f_KM_b = new EGS_Interpolator* [nmed];
      f_KM_max = new EGS_Interpolator [nmed];
      zbr_KM = new EGS_Float [nmed];
      a_KM = new EGS_Float [nmed]; b_KM = new EGS_Float [nmed];

      DBS_Aux the_aux; the_aux.np = 128; EGS_Float b = 0.5;
      int nx = 257; EGS_Float ddx = 0.00390625;
      for(int imed=0; imed<nmed; imed++) {
        EGS_Float AP = app->getAp(imed);
        EGS_Float logAP = log(AP);
        int ne = 1 + (int) ((logEmax-logAP)/0.12);
        a_KM[imed] = ((EGS_Float)ne-1)/(logEmax-logAP);
        b_KM[imed] = -a_KM[imed]*logAP;
        f_KM_a[imed] = new EGS_Interpolator [ne];
        f_KM_b[imed] = new EGS_Interpolator [ne];
        EGS_Float *tmp_fmax = new EGS_Float [ne];
        EGS_Float dle = (logEmax-logAP)/(ne-1);
        int imedm = imed+1;
        zbr_KM[imed] = app->getZbrang(imed);
        the_aux.Z13 = zbr_KM[imed];
        for(int ie=0; ie<ne; ie++) {
            EGS_Float eke = AP*exp(dle*ie);
            EGS_Float E = eke/app->getRM() + 1;
            the_aux.E = E; the_aux.le = log(E - b*(E-1));
            f_KM_a[imed][ie].initialize(0,b,dbs_func_KM,&the_aux,1025,9e-4);
            f_KM_b[imed][ie].initialize(0,1,dbs_func_KM1,&the_aux,1025,9e-4);
            EGS_Float fmax = 0;
            for(int j=0; j<nx; j++) {
                EGS_Float x = ddx*j;
                EGS_Float k = x*(E-1);
                EGS_Float fnorm;
                if( x <= b ) fnorm = f_KM_a[imed][ie].interpolate(x);
                else {
                    EGS_Float u = log(E-x*(E-1))/the_aux.le;
                    fnorm = f_KM_b[imed][ie].interpolate(u);
                }
                EGS_Float r = 1-k/E; EGS_Float delta = (1-r)/(2*r*E);
                EGS_Float f = (-(1+r)*(1+r)-
                        (1+r*r)*log(delta*delta+zbr_KM[imed]))/fnorm;
                if( f > fmax ) fmax = f;
            }
            tmp_fmax[ie] = fmax;
        }
        f_KM_max[imed].initialize(ne,logAP,logEmax,tmp_fmax);
        //egsWarning("---deleting in initSmartKM()\n");
        delete [] tmp_fmax;
    }
    nmed_KM = nmed;
    if( nsplit > 0 ) y2_KM = new EGS_Float [nsplit];
}

void EGS_RadiativeSplitting::doUphi21(EGS_Float sinthe, EGS_Float costhe, EGS_Vector &u)
{
    //This is equivalent to calling the Mortran routine UPHI(2,1) and is used
    //to adjust particle angles after a Rayleigh event
    EGS_Float cosphi,sinphi;
    selectAzimuthalAngle(cosphi,sinphi);
    EGS_Float a=u.x; EGS_Float b=u.y; EGS_Float c=u.z;
    EGS_Float sinps2 = a*a+b*b;
    if (sinps2 < 1e-20)
    {
        //small polar angle case
        u.x = sinthe*cosphi;
        u.y = sinthe*sinphi;
        u.z = c*costhe;
    }
    else
    {
        EGS_Float sinpsi = sqrt(sinps2);
        EGS_Float us = sinthe*cosphi;
        EGS_Float vs = sinthe*sinphi;
        EGS_Float sindel = b/sinpsi;
        EGS_Float cosdel = a/sinpsi;
        u.x = c*cosdel*us - sindel*vs + a*costhe;
        u.y = c*sindel*us + cosdel*vs + b*costhe;
        u.z = -sinpsi*us + c*costhe;
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
        //disable BEAMnrc-style DBS until we either implement/eliminate it
        //allowed_split_type.push_back("BEAMnrc directional");
        int split_type;
        string str;
        if(input->getInput("splitting type",str) < 0)
	    {
	    	split_type = EGS_RadiativeSplitting::URS;
	    }
	    else
	    {
	    	split_type = input->getInput("splitting type",allowed_split_type,-1);
           	if ( split_type < EGS_RadiativeSplitting::URS ||
                 split_type > EGS_RadiativeSplitting::DRSf )
           	{
     		   egsFatal("\nEGS_RadiativeSplitting: Invalid splitting type.\n");
	    	}
	    }
        EGS_Float nsplit = 1.0;
        EGS_Float fs,ssd,zrr;
        int irad = 0;
        vector<int> esplit_reg;
        int err = input->getInput("splitting",nsplit);
        if (err)
	    {
	    	egsWarning("\nEGS_RadiativeSplitting: Invalid splitting no.  Radiative splitting will be turned off.\n");
	    }
        if ( split_type == EGS_RadiativeSplitting::DRS ||
             split_type == EGS_RadiativeSplitting::DRSf )
	    {
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

        printf("\nAbout to set up\n");

        //=================================================
        /* Setup radiative splitting object with input parameters */
        EGS_RadiativeSplitting *result = new EGS_RadiativeSplitting("",f);
        result->setSplitType(split_type);
        result->setSplitting(nsplit);
        if (split_type == EGS_RadiativeSplitting::DRS ||
            split_type == EGS_RadiativeSplitting::DRSf){
	    	result->initDBS(fs,ssd,esplit_reg,irad,zrr);
	    }
        result->setName(input);
        return result;
    }
}
