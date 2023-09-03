/*
###############################################################################
#
#  EGSnrc egs++ egs_fac application
#  Copyright (C) 2015 National Research Council Canada
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
#  Author:          Iwan Kawrakow, 2008
#
#  Contributors:    Ernesto Mainegra-Hing
#                   Blake Walters
#
###############################################################################
#
#  This code was originally extracted as a stand-alone application from the
#  egs++ application cavity by Iwan Kawrakow in 2008. Ernesto Mainegra-Hing
#  originally added FAC calculation capabilities to the cavity application
#  in 2007.
#
###############################################################################
#
#  An advanced C++ application optimized for the calculation of free-air
#  chamber (FAC) correction factors using a self-consistent approach as
#  described in Mainegra-Hing, Reynaert and Kawrakow, Med. Phys. 35 (8), 2008.
#
#  These corrections factors allow relating the dose deposited in the
#  collecting volume (CV) of the FAC to the air-kerma, free in air, at the
#  point of measurement (POM) located at the entrance diaphragm.
#
#  For details on the input syntax the user is referred to the user manual
#  for the C++ EGSnrc class library  (PIRS-898 report)
#
#
#  FAC corrections
#  ---------------
#
#  Free Air Chamber correction factors remove the effects of the following
#  processes:
#
#  Aap        Removes contribution from FAC aperture to cavity dose.
#
#  Ascat      Removes scatter contribution to cavity dose.
#
#  Aeloss     Electron loss correction by computing energy imbalance
#             through lateral cavity sides.
#
#  Aatt       Removes photon attenuation.
#
#  Ag         Geometry correction factor accounting for photons entering
#             the CV at an angle theta for which the path through the CV
#             d < h/cos(theta), with d the height of the CV. Mostly unity.
#
#  Acpe       Checks that CPE exists along the beam's and a homogeneous
#             electron field exists across the CV. Mostly unity.
#
#  Acheck     Checks that e- transport is self-consistent and artifact-free.
#             Mostly unity.
#
#  Ab         Removes contribution from scatter into POM when FAC removed
#
#  Ax         Corrects possible inconsistencies in experimental Aatt.
#             (Different from 1 for NRC's evacuated-tube technique)
#
#  For each defined calculation geometry the following corrections are
#  calculated: Aap, Ascat, Aeloss, Ag, Acpe and Acheck. Calculation of Ax
#  and Ab requires extra geometries. The input required for these
#  calculations is described below. Note that both types of calculations can
#  be performed with one input file.
#
#
#  Ab calculation
#  --------------
#
#  The correction for backscatter in air when no FAC is present, Ab, needs
#  an extra calculation of the air-kerma at the POM free in air. To this end,
#  one must define a geometry without the FAC and allow the scoring of the
#  contribution from scattered photons to the air-kerma at the POM. This is
#  accomplished by setting 'include scatter = yes' in the calculation geometry
#  definition block for the geometry without the chamber. The correction
#  factor Ab is then obtained as the ratio of the air-kerma at the POM for
#  both geometries, with and without the FAC. One should allow for enough air
#  behind the POM to avoid underestimating the backscatter.
#
#  :start scoring options:
#
#      :start calculation geometry:
#          geometry name  = sim_no_fac
#          cavity regions = list_of_cavity_region_indices
#          cavity mass    = cavity mass in g
#          aperture regions = regions_defining_aperture
#          front and back regions = f_reg b_reg
#          POM = 0.45 0.5  # first input is z-position, second is radius.
#          include scatter = yes # yes,no(default)
#      :stop calculation geometry:
#
#      :start calculation geometry:
#          geometry name  = sim_fac
#          cavity regions = list_of_cavity_region_indices
#          cavity mass    = cavity mass in g
#          aperture regions = regions_defining_aperture
#          front and back regions = f_reg b_reg
#          POM = 0.45 0.5  # first input is z-position, second is radius.
#      :stop calculation geometry:
#
#      (...)
#
#      correlated geometries = sim_fac sim_no_fac
#
#  :stop scoring options:
#
#  Note: 'front and back regions' and 'cavity regions' MUST be provided.
#  However, only air-kerma at the POM (E6) will be meaningful for a
#  calculation without the FAC.
#
#
#  Ax calculation
#  --------------
#
#  To correct for differences between the experimental and the MC calculated
#  attenuation correction Aatt, a correction factor, Ax, was introduced which
#  can be calculated by using the 'Ax calculation' input key. This input key
#  expects the name of three geometries, the realistic FAC geometry
#  'sim_no_tube', plus two geometries ('sim_vacuum_tube' and 'sim_air_tube')
#  to model the evacuated-tube technique for experimentally determining Aatt
#  at the NRC as the ratio of the FAC readings with and without an evacuated
#  tube. The names used here for these geometries constitute only examples.
#
#  :start scoring options:
#
#      :start calculation geometry:
#          geometry name  = sim_no_tube
#          cavity regions = list_of_cavity_region_indices
#          cavity mass    = cavity mass in g
#          aperture regions = regions_defining_aperture
#          front and back regions = f_reg b_reg
#          POM = 0.45 0.5  # first input is z-position, second is radius.
#      :stop calculation geometry:
#
#      :start calculation geometry:
#          geometry name  = sim_air_tube
#          (...)
#      :stop calculation geometry:
#
#      :start calculation geometry:
#          geometry name  = sim_vacuum_tube
#          (...)
#      :stop calculation geometry:
#
#      (...)
#
#      :start calculation geometry:
#          geometry name  = sim_n
#          (...)
#      :stop calculation geometry:
#
#      correlated geometries = sim_i sim_j
#
#      Ax calculation = sim_no_tube sim_air_tube sim_vacuum_tube
#
#      muen file = E*muen file name
#
#  :stop scoring options:
#
#  Note: Any other experimental procedure used to determine Aatt based on the
#  ratio of two cavity doses can be simulated with this user code.
#
###############################################################################
*/


#include "egs_fac.h"
#include "egs_application.h"
#include "egs_interface2.h"
#include "egs_functions.h"
#include "egs_input.h"
#include "egs_base_source.h"
#include "egs_rndm.h"
#include "egs_transformations.h"
#include "egs_interpolator.h"
#include "egs_base_geometry.h"
#include "egs_mortran.h"
#include "egs_range_rejection.h"
#include "egs_fac_simulation.h"
#include "egs_math.h"

#include <fstream>
using namespace std;

EGS_FACApplication::EGS_FACApplication(int argc, char **argv) :
        EGS_AdvancedApplication(argc,argv), ngeom(0), sim(0),
        ncor(0), corr(0), nax(0), Ax(0),
        rrej(0), fsplit(1), fspliti(1), gsections(0),
        increase_scatter(false) {}

EGS_FACApplication::~EGS_FACApplication() {
    if( ngeom > 0 ) {
        for(int j=0; j<ngeom; ++j) delete sim[j];
        delete [] sim;
    }
}

void EGS_FACApplication::describeUserCode() const {
    egsInformation(
        "\n               *************************************************"
        "\n               *                                               *"
        "\n               *                 egs_fac                       *"
        "\n               *                                               *"
        "\n               *************************************************"
        "\n\n");
    egsInformation("This is EGS_FACApplication %s based on\n"
            "      EGS_AdvancedApplication %s\n\n",
            egsSimplifyCVSKey(revision).c_str(),
            egsSimplifyCVSKey(base_revision).c_str());

}

int EGS_FACApplication::ausgab(int iarg) {
    static int nwarn = 0;
    int np = the_stack->np-1;
    if( iarg <= ExtraEnergy ) {
        int ir = the_stack->ir[np]-2;
        int latch = the_stack->latch[np];
        if( iarg == BeforeTransport ) {
            if( latch <= 0 && sim[ig]->isAperture(ir) )
                the_stack->latch[np] = -1;
            int irnew = the_epcont->irnew-2;
            if( the_stack->iq[np] && !the_stack->latch[np] && ir != irnew ) {
                // primary electron step that results in a change of regions
                // => collect energy imbalance of electrons entering/leaving
                //    CV. Note: addEnergyImbalance takes care of checks for
                //    regions being CV.
                EGS_Float dE = (the_stack->E[np]-the_epcont->edep-the_useful->rm)*
                                the_stack->wt[np];
                if( std::isnan(dE) || std::isnan(the_extra_stack->expmfp[np]) ||
                    std::isinf(dE) || std::isinf(the_extra_stack->expmfp[np]) )
                    egsInformation("\nAdding a NaN in imbalance: %g %g\n",dE,the_extra_stack->expmfp[np]);
                else sim[ig]->addEnergyImbalance(ir,irnew,the_extra_stack->expmfp[np],dE);
            }
        }
        if( the_epcont->edep > 0 ) {
            EGS_Float edep = the_stack->wt[np]*the_epcont->edep;
            if( sim[ig]->isCavity(ir) && the_stack->wt[np]*fsplit > 1.01 ) {
                if( ++nwarn < 20 ) {
                    egsWarning("Fat energy deposition in cavity:\n");
                    egsWarning("  iarg=%d edep=%g wt=%g q=%d latch=%d is_fat=%d\n",iarg,the_epcont->edep,
                            the_stack->wt[np],the_stack->iq[np],the_stack->latch[np],the_extra_stack->is_fat[np]);
                }
            }
            if( std::isnan(edep) || std::isinf(edep) )
                egsInformation("\nAdding a NaN in energy: %g %g\n",the_stack->wt[np],the_epcont->edep);
            else sim[ig]->addEnergyDeposition(ir,the_stack->latch[np],edep);
        }
        return 0;
    }
    if( iarg == AfterBrems || iarg == AfterMoller || iarg == AfterBhabha ||
        iarg == AfterAnnihFlight || iarg == AfterAnnihRest ) {
        //
        // if not fat, play RR with photons
        // if latch=0, set latch of photons to 1
        //
        int latch = the_stack->latch[np];
        int new_latch = !latch ? 1 : latch;
        bool play_RR = !the_extra_stack->is_fat[np];
        if( !play_RR && new_latch == latch ) return 0;
        for(int ip=the_stack->npold-1; ip<=np; ++ip) {
            if( !the_stack->iq[ip] ) {
                if( play_RR ) {
                    if( rndm->getUniform() > fspliti ) { // kill it
                        if( ip < np ) {
                            the_stack->E[ip] = the_stack->E[np];
                            the_stack->wt[ip] = the_stack->wt[np];
                            the_stack->iq[ip] = the_stack->iq[np];
                            the_stack->u[ip] = the_stack->u[np];
                            the_stack->v[ip] = the_stack->v[np];
                            the_stack->w[ip] = the_stack->w[np];
                        }
                        --ip; --np;
                    }
                    else { // survives
                        the_stack->wt[ip] *= fsplit;
                        the_stack->latch[ip] = new_latch;
                    }
                }
                else the_stack->latch[ip] = new_latch;
            }
        }
        the_stack->np = np+1;
        return 0;
    }
    return 0;
}

int EGS_FACApplication::simulateSingleShower() {
    last_case = current_case;
    EGS_Vector x,u;
    current_case = source->getNextParticle(rndm,p.q,p.latch,p.E,p.wt,x,u);
    if( p.q ) egsFatal("Got particle with q=%d.\n"
        "This application only works for photons\n",p.q);
    int err = startNewShower(); if( err ) return err;
    EGS_BaseGeometry *save_geometry = geometry;
    EGS_RangeRejection *save_rr = rrej;
    EGS_Float save_fspliti = fspliti;
    EGS_Float save_fsplit = fsplit;
    if( ngeom > 1 ) saveRNGState();
    for(ig=0; ig<ngeom; ig++) {
        if( ig > 0 ) resetRNGState();
        geometry = sim[ig]->geometry;
        med_cv = sim[ig]->med_cv;
        rrej = sim[ig]->rr ? sim[ig]->rr : save_rr;
        fsplit = sim[ig]->fsplit > 1 ? sim[ig]->fsplit : save_fsplit;
        fspliti = 1/fsplit;
        kerma_fac = sim[ig]->cmass/(M_PI*sim[ig]->R2_pom);
        //kerma_fac = sim[ig]->cmass;
        p.x = x; p.u = u;
        if( sim[ig]->transform ) {
            sim[ig]->transform->transform(p.x); sim[ig]->transform->rotate(p.u);
        }
        int ireg = geometry->isWhere(p.x);
        if( ireg < 0 ) {
            EGS_Float t = 1e30; ireg = geometry->howfar(ireg,p.x,p.u,t);
            if( ireg >= 0 ) p.x += p.u*t;
        }
        if( ireg >= 0 ) {
            p.ir = ireg;
            err = shower(); if( err ) break;
        }
    }
    if( !err ) err = finishShower();
    geometry = save_geometry;
    rrej = save_rr;
    fspliti = save_fspliti;
    return err;
}

int EGS_FACApplication::outputData() {
    int err = EGS_AdvancedApplication::outputData();
    if( err ) return err;
    int j;
    for(j=0; j<ngeom; ++j) {
        if( sim[j]->outputData(*data_out) ) return 100+j;
    }
    for(j=0; j<ncor; ++j) {
        if( corr[j]->outputData(*data_out) ) return 1000+j;
    }
    for(j=0; j<nax; ++j) {
        if( Ax[j]->outputData(*data_out) ) return 10000+j;
    }
    data_out->flush();
    delete data_out; data_out = 0;
    return 0;
}

int EGS_FACApplication::readData() {
    int err = EGS_AdvancedApplication::readData();
    if( err ) return err;
    int j;
    for(j=0; j<ngeom; ++j) {
        if( sim[j]->readData(*data_in) ) return 100+j;
    }
    for(j=0; j<ncor; ++j) {
        if( corr[j]->readData(*data_in) ) return 1000+j;
    }
    for(j=0; j<nax; ++j) {
        if( Ax[j]->readData(*data_in) ) return 10000+j;
    }
    return 0;
}

void EGS_FACApplication::resetCounter() {
    EGS_AdvancedApplication::resetCounter();
    int j;
    for(j=0; j<ngeom; ++j) sim[j]->resetCounter();
    for(j=0; j<ncor; ++j) corr[j]->resetCounter();
    for(j=0; j<nax;  ++j) Ax[j]->resetCounter();
}

int EGS_FACApplication::addState(istream &data) {
    int err = EGS_AdvancedApplication::addState(data);
    if( err ) return err;
    int j;
    for(j=0; j<ngeom; ++j) {
        if( sim[j]->addData(data) ) return 100+j;
    }
    for(j=0; j<ncor; ++j) {
        if( corr[j]->addData(data) ) return 1000+j;
    }
    for(j=0; j<nax; ++j) {
        if( Ax[j]->addData(data) ) return 10000+j;
    }
    return 0;
}

void EGS_FACApplication::outputResults() {
    egsInformation("\n\n last case = %lld fluence = %20.5f\n\n",
            current_case,source->getFluence());
    egsInformation("The energies listed below have the following definitions:\n\n"
"Eg  = Total energy deposited in the collecting volume (CV)\n"
"E1  = Total energy deposited in CV excluding energy from particles that have\n"
"    = visited an aperture region\n"
"E2  = Energy deposited in the CV from primary electrons\n"
"E3  = E2 corrected for energy loss/gain through the side faces of the CV\n"
"E4  = E3 corrected for energy loss/gain through the front/back CV faces\n"
"E4a = Same as E4 but computed from a kerma approximation for photons\n"
"    = passing through the CV\n"
"E5  = E4a for an ideal point source\n"
"E6  = E5 corrected for attenuation. E6 = collision kerma at POM\n");
    int j;
    for(j=0; j<ngeom; ++j)
        sim[j]->reportResults(source->getFluence(),current_case);
    for(j=0; j<ncor; ++j)
        corr[j]->reportResults(source->getFluence(),current_case);
    for(j=0; j<nax; ++j)
        Ax[j]->reportResults(current_case);
}

void EGS_FACApplication::getCurrentResult(double &sum, double &sum2, double &norm,
        double &count) {
    count = current_case; double flu = source->getFluence();
    norm = flu > 0 ? 1.6022e-10*count/(flu*sim[0]->cmass) : 0;
    sum = sim[0]->dose[2]; sum2 = sim[0]->dose2[2];
    if( std::isnan(sum) || std::isinf(sum) ) egsInformation("\nBad result? sum=%g sum2=%g\n",sum,sum2);
}

void EGS_FACApplication::selectPhotonMFP(EGS_Float &dpmfp) {
    static int nerror = 0;
    int np = the_stack->np-1;
    EGS_Vector x(the_stack->x[np],the_stack->y[np],the_stack->z[np]),
               u(the_stack->u[np],the_stack->v[np],the_stack->w[np]);
    EGS_Float E = the_stack->E[np];
    int ireg = the_stack->ir[np]-2;
    int latchi = the_stack->latch[np];
    EGS_Float gle = the_epcont->gle;
    if( fabs(log(E)-gle) > 1e-3 ) egsFatal("gle=%g log(E)=%g\n",gle,log(E));
    if( the_stack->iq[np] ) egsFatal("Electron in selectPhotonMFP? q=%d\n",the_stack->iq[np]);
    EGS_Float gmfp, cohfac, sigma, told = 0;
    //
    // *** get intersections with geometry
    //
    //egsInformation("\nSelectPhotonMFP: np=%d E=%g latch=%d wt=%g x=(%g,%g,%g) u=(%g,%g,%g)\n",
    //        np,E,latchi,the_stack->wt[np],x.x,x.y,x.z,u.x,u.y,u.z);
    int nsec = geometry->computeIntersections(ireg,ngsec,x,u,gsections);
    if( nsec < 0 ) {
        egsWarning("\nSize of gsections not large enough to hold "
           "all intersections with the geometry?\n"
           "x=(%16.10f,%16.10f,%16.10f)\n"
           "u=(%14.10f,%14.10f,%14.10f) ireg=%d ngsec=%d\n",
           x.x,x.y,x.z,u.x,u.y,u.z,ireg,ngsec);
        egsWarning("This indicates a geometry problem -> discarding photon\n");
        if( ++nerror > 100 )
            egsFatal("\nToo many geometry errors -> quitting\n");
        dpmfp = -1; --the_stack->np; return;
    }
    /*
    egsInformation("Got %d geometry intersections\n",nsec);
    for(int j=0; j<nsec; ++j) egsInformation("%d %d %d %g (%g,%g,%g)\n",
        j,gsections[j].ireg,gsections[j].imed,gsections[j].t,
        x.x+gsections[j].t*u.x,x.y+gsections[j].t*u.y,x.z+gsections[j].t*u.z);
    exit(1);
    */
    //
    // *** should we be checking for passing through the POM?
    //
    bool check_pom;
    if (sim[ig]->include_scatter)
       check_pom = (latchi >= 0 && ((x.z >= sim[ig]->z_pom && u.z < 0)||
                                    (x.z <= sim[ig]->z_pom && u.z > 0)
       ));
    else
       check_pom = (latchi == 0 && x.z <= sim[ig]->z_pom && u.z > 0);

    EGS_Float d_to_pom = check_pom ? (sim[ig]->z_pom-x.z)/u.z : 1e30;
    bool hits_pom_area = false;
    //
    // *** interaction counter and scattered photon to keep
    //
    //int ninter = 0; int keep = (int) (rndm->getUniform()*fsplit);
    int ninter = 0;
    EGS_Float keep_fac = increase_scatter && latchi == 0 ? 0.5 : 1;
    EGS_Float akeep = keep_fac*rndm->getUniform()*fsplit; int keep = (int) akeep;
    //
    // *** CV entry
    //
    bool enters_cv = false;
    //
    // *** MFPs
    //
    EGS_Float eta_prime = 1 - rndm->getUniform()*fspliti;
    EGS_Float lambda = -log(eta_prime), lambda_old = lambda;
    //egsInformation("lambda=%g eta_prime=%g fspliti=%g\n",lambda,eta_prime,fspliti);
    EGS_Float Lambda_to_pom = 0, Lambda_to_cv = 0, Lambda_in_cv = 0;
    //
    // *** hitting aperture
    //
    bool hits_ap = latchi == -1;
    if( hits_ap ) check_pom = false;
    EGS_Float rr_range = rrej ? rrej->getRange(-1,gle) : 1e30;
    int imed_old = -99; int j_hit_ap = 100000000;
    EGS_Float mu_cv = -1, Lambda_tot = 0;
    for(int j=0; j<nsec; ++j) {
        int imed = gsections[j].imed;
        ireg = gsections[j].ireg;
        //egsInformation("j=%d ireg=%d imed=%d t=%g\n",j,ireg,imed,gsections[j].t);
        if( !hits_ap && latchi == 0 ) {
            if( sim[ig]->isAperture(ireg) ) {
                //egsInformation("have hit aperture!\n");
                hits_ap = true; check_pom = false;
                j_hit_ap = j;
            }
        }
        if( imed != imed_old ) {
            if( imed >= 0 ) {
                gmfp = i_gmfp[imed].interpolateFast(gle);
                if( the_xoptions->iraylr ) {
                    cohfac = i_cohe[imed].interpolateFast(gle);
                    gmfp *= cohfac;
                }
                sigma = 1/gmfp;
                //egsInformation("computed cross section: sigma=%g cohfac=%g\n",sigma,cohfac);
            }
            else { sigma = 0; cohfac = 1; }
        }
        if( check_pom && gsections[j].t >= d_to_pom ) {
            check_pom = false;
            EGS_Float x1 = x.x + u.x*d_to_pom,
                      y1 = x.y + u.y*d_to_pom;
            //egsInformation("Reached POM plane at (%g,%g,%g)\n",x1,y1,x.z+u.z*d_to_pom);
            if( x1*x1 + y1*y1 <= sim[ig]->R2_pom ) {
                hits_pom_area = true;
                Lambda_to_pom += (d_to_pom-told)*sigma;
                //egsInformation("In POM circle, lambda=%g\n",Lambda_to_pom);
            }
        }
        EGS_Float t = gsections[j].t - told;
        EGS_Float lam = t*sigma;
        //egsInformation("t=%g lam=%g lambda=%g\n",t,lam,lambda);
        if( latchi == 0 ||(latchi >= 0 && sim[ig]->include_scatter) ) {
            if( sim[ig]->isCavity(ireg) ) {
                enters_cv = true;
                Lambda_in_cv += lam;
                mu_cv = sigma;
                //egsInformation("In CV: %g\n",Lambda_in_cv);
            }
            if( !enters_cv ) Lambda_to_cv += lam;
            if( check_pom ) Lambda_to_pom += lam;
            //egsInformation("Lambda_to_cv=%g\n",Lambda_to_cv);
        }
retry:
        if( lam > lambda ) {
            int ityp = -1;
            if( the_xoptions->iraylr ) {
                if( rndm->getUniform() < 1 - cohfac ) ityp = 0;
            }
            if( ityp < 0 ) {
                EGS_Float gbr1 = E > the_thresh->rmt2 ?
                    i_gbr1[imed].interpolateFast(gle) : 0;
                EGS_Float eta = rndm->getUniform();
                if( eta < gbr1 ) ityp = 1;
                else {
                    if( eta <= i_gbr2[imed].interpolateFast(gle) ) ityp = 2;
                    else ityp = 3;
                }
            }
            told += lambda*gmfp;
            Lambda_tot += lambda;
            //egsInformation("Interaction %d at distance=%g Ltot=%g\n",ityp,told,Lambda_tot);
            idist[ninter] = told; ilambda[ninter] = Lambda_tot;
            iindex[ninter] = j; itype[ninter++] = ityp;
            eta_prime -= fspliti;
            lambda = eta_prime > 0 ? -log(eta_prime) - lambda_old : 1e30;
            //egsInformation("Interacting: eta_prime=%g lambda=%g ityp=%d\n",eta_prime,lambda,ityp);
            lambda_old += lambda;
            t = gsections[j].t - told;
            lam = t*sigma;
            //egsInformation("new lambda=%g lam=%g t=%g\n",lambda,lam,t);
            goto retry;
        }
        else { lambda -= lam; Lambda_tot += lam; }
        told = gsections[j].t; imed_old = imed;
    }

    //
    // *** score kerma
    //
     if((latchi == 0||sim[ig]->include_scatter) && hits_pom_area && !hits_ap){
        EGS_Float mu_en = muen->interpolateFast(gle);
        //egsInformation("scoring kerma: mu_en=%g wt=%g\n",mu_en,the_stack->wt[np]);
        //egsInformation("Lambdas: to_pom=%g to_cv=%g in_cv=%g\n",Lambda_to_pom,Lambda_to_cv,Lambda_in_cv);
        mu_en *= the_stack->wt[np];
        mu_en *= kerma_fac; //EGS_Float uzi = 1/u.z;
        EGS_Float dE6 = mu_en*exp(-Lambda_to_pom)/fabs(u.z);
        /**********************************************
         If including scatter in Kerma @ POM,
         and u.z < 0 don't score dE4,dE5.
         In this case, latchi=0 and u.z can be negative
        ***********************************************/
        EGS_Float dE5 = 0, dE4 = 0;
        if( u.z > 0 ) {
           mu_en *= exp(-Lambda_to_cv);
           if( mu_cv < 0 ) {
               // means the photon didn't visit the CV
               // this is a case contributing to Ag
               gmfp = i_gmfp[med_cv].interpolateFast(gle);
               if( the_xoptions->iraylr ) {
                   cohfac = i_cohe[med_cv].interpolateFast(gle);
                   gmfp *= cohfac;
               }
               sigma = 1/gmfp; mu_cv = sigma;
           }
           EGS_Float lcv = sim[ig]->h*mu_cv/u.z;
           dE5 = mu_en*(1-exp(-lcv))/(sim[ig]->h*mu_cv);
           dE4 = enters_cv ? mu_en*(1-exp(-Lambda_in_cv))/(sim[ig]->h*mu_cv) : 0;
        }
        //egsInformation("scoring E4=%g E5=%g E6=%g\n",dE4,dE5,dE6);
        sim[ig]->addKerma(dE4,dE5,dE6);
    }

    //
    // *** do interactions
    //
    dpmfp = -1;
    if( ninter < 1 ) { --the_stack->np; return; }

    EGS_Float wt_o = the_stack->wt[np], wt = wt_o*fspliti;
    EGS_Float kappa = the_extra_stack->expmfp[np];
    int last_imed = -1; EGS_Float range;
    --np; --the_stack->np;
/*    egsInformation("Performing %d interactions, keeping scattered from %d\n",ninter,keep);*/
    for(int i=0; i<ninter; ++i) {
        int j = iindex[i];
        int do_interaction = 0;
        if( itype[i] > 0 ) {
            if( sim[ig]->isSplitting(gsections[j].ireg) ) do_interaction = 1;
            else if ( i == keep ) do_interaction = 2;
        }
        else if( i == keep ) do_interaction = 2;
        //if( gsections[j].imed == 0 ) egsInformation("Interaction in lead: typ=%d latch=%d do=%d\n",
        //        itype[i],latchi,do_interaction);
        if( do_interaction ) {
            EGS_Vector xi(x + u*idist[i]);
            //EGS_Float tperp = geometry->hownear(gsections[j].ireg,xi);
            EGS_Float tperp = 0; //EGS_Float tperp_cavity = rrej->hownear(xi);
            if( do_interaction == 1 && rrej && !sim[ig]->isCavity(gsections[j].ireg)) {
                if( !rrej->canEnterCavity(rr_range,xi) ) {
                //if( rr_range < tperp_cavity ) {
                    if( i != keep ) continue;
                    do_interaction = 2;
                }
                else {
                    if( gsections[j].imed != last_imed ) {
                        int med = gsections[j].imed + 1; int q=-1; last_imed = gsections[j].imed;
                        computeRange(&E,&gle,&med,&q,&range);
                        //egsInformation("E=%g imed=%d range=%g\n",E,gsections[j].imed,range);
                    }
                    tperp = geometry->hownear(gsections[j].ireg,xi);
                    if( range < tperp ) {
                        if( i != keep ) continue;
                        do_interaction = 2;
                    }
                }
            }
            ++np; ++the_stack->np;
            the_stack->u[np] = u.x; the_stack->v[np] = u.y; the_stack->w[np] = u.z;
            the_stack->x[np] = xi.x; the_stack->y[np] = xi.y; the_stack->z[np] = xi.z;
            the_stack->E[np] = E; the_stack->iq[np] = 0; the_stack->dnear[np] = tperp;
            the_stack->ir[np] = gsections[j].ireg + 2;
            if( do_interaction == 1 ) {
                the_stack->wt[np] = wt; the_extra_stack->is_fat[np] = 0;
            }
            else {
                //the_stack->wt[np] = wt_o; the_extra_stack->is_fat[np] = 1;
                the_stack->wt[np] = wt_o*keep_fac; the_extra_stack->is_fat[np] = 1;
            }
            //
            // *** set latch
            //     if latch=-1 (photon has already been in aperture) or
            //        latch=+1 (scattered photon),
            //     latch remains the same.
            //     if latch=0,
            //       - if the photon did not hit aperture before arriving
            //         at interaction site, then latch=0. But if we have
            //         Rayleigh, then there are no electrons, so we might
            //         as well set latch=1 before the interaction and save
            //         changing latch afterwards
            //       - else latch=-1
            //
            int latch_e = latchi; int latch_p = latchi;
            if( latchi == 0 ) {
                if( j >= j_hit_ap ) { latch_e = -1; latch_p = -1; }
                 else if( xi.z > sim[ig]->z_pom )    latch_p = 1;
            }
            if( itype[i] == 0 ) latch_e = latch_p;
            //if( gsections[j].imed == 0 ) egsInformation("z=%g latch_e=%d latch_p=%d keep=%d i=%d\n",
            //        xi.z,latch_e,latch_p,keep,i);
            the_stack->latch[np] = latch_e;
            the_extra_stack->expmfp[np] = latch_e == 0 ?
                kappa*exp(ilambda[i]-Lambda_to_pom) : kappa;
            //int latch = latchi;
            //if( latch == 0 ) {
            //    if( j >= j_hit_ap ) latch = -1;
            //    else if( itype[i] == 0 && xi.z > sim[ig]->z_pom ) latch=1;
            //}
            //the_extra_stack->expmfp[np] = latch == 0 ?
            //    kappa*exp(ilambda[i]-Lambda_to_pom) : kappa;
            the_useful->medium = gsections[j].imed + 1;
            //egsInformation("Interaction %d is %d, latch=%d expmfp=%g np=%d x=(%g,%g,%g)\n",
            //        i,itype[i],latch,the_extra_stack->expmfp[np],np,xi.x,xi.y,xi.z);
            if     ( itype[i] == 0 ) doRayleigh();
            else if( itype[i] == 1 ) F77_OBJ(pair,PAIR)();
            else if( itype[i] == 2 ) F77_OBJ(compt,COMPT)();
            else                     F77_OBJ(photo,PHOTO)();
            np = the_stack->np - 1;
            //egsInformation("after: np=%d\n",np);
            if( itype[i] > 0 ) { // nothing to be done for Rayleigh
                bool keep_photons = i == keep;
                //if( latch == 0 && xi.z > sim[ig]->z_pom ) latch = 1;
                //if( gsections[j].imed == 0 ) egsInformation("npold=%d np=%d E=%g edep=%g\n",
                //            the_stack->npold,the_stack->np,E,the_epcont->edep);
                for(int ip=the_stack->npold-1; ip<=np; ++ip) {
                    if( !the_stack->iq[ip] ) {
                        if( keep_photons ) {
                            the_stack->wt[ip] = wt_o*keep_fac;
                            //the_stack->wt[ip] = wt_o;
                            //the_stack->latch[ip] = latch;
                            the_stack->latch[ip] = latch_p;
                        }
                        else {
                            if( ip < np ) {
                                the_stack->E[ip] = the_stack->E[np];
                                the_stack->wt[ip] = the_stack->wt[np];
                                the_stack->iq[ip] = the_stack->iq[np];
                                the_stack->u[ip] = the_stack->u[np];
                                the_stack->v[ip] = the_stack->v[np];
                                the_stack->w[ip] = the_stack->w[np];
                                the_stack->latch[ip] = the_stack->latch[np];
                            }
                            --ip; --np;
                        }
                    }
                }
                the_stack->np = np+1;
            }
        }
        if( i == keep ) {
            akeep += keep_fac*fsplit; keep = (int) akeep;
        }
    }
    //egsInformation("Now on stack:\n");
    //for(int j=0; j<the_stack->np; ++j)
    //    egsInformation("%d q=%d E=%g wt=%g latch=%d ir=%d x=(%g,%g,%g)\n",
    //        j,the_stack->iq[j],the_stack->E[j],the_stack->wt[j],the_stack->latch[j],
    //        the_stack->ir[j],the_stack->x[j],the_stack->y[j],the_stack->z[j]);
}

int EGS_FACApplication::rangeDiscard(EGS_Float tperp, EGS_Float range) const {
    if( !rrej ) return 0;
    int np = the_stack->np-1;
    if( the_extra_stack->is_fat[np] > 0 ) return 0;
    int ir = the_stack->ir[np]-2;
    bool is_cav = sim[ig]->isCavity(ir);
    EGS_RangeRejection::RejectionAction action =
        rrej->rangeDiscard(np,the_stack,tperp,range,is_cav,
                the_epcont->elke,rndm);
    if( action == EGS_RangeRejection::Survive ) {
        the_extra_stack->is_fat[np] = 1;
        action = EGS_RangeRejection::NoAction;
    }
    return (int)action;
}

int EGS_FACApplication::startNewShower() {
    int res = EGS_Application::startNewShower();
    if( res ) return res;
    if( current_case != last_case ) {
        int j;
        for(j=0; j<ncor; ++j) corr[j]->finishHistory();
        for(j=0; j<nax; ++j) Ax[j]->finishHistory();
        for(j=0; j<ngeom; j++) {
            if( !sim[j]->finishHistory() ) egsInformation("Bad result in case %d\n",(int)current_case);
        }
        last_case = current_case;
    }
    return 0;
}

string EGS_FACApplication::revision = " ";

int EGS_FACApplication::initScoring() {
    EGS_Input *options = input->takeInputItem("scoring options");
    vector<EGS_FACSimulation *> sims; vector<EGS_Input *> sim_inputs;
    if( options ) {
        //
        // *********** photon cross section scaling
        //
        EGS_Input *scaling;
        EGS_BaseGeometry::setActiveGeometryList(app_index);
        while( (scaling = options->takeInputItem("scale photon x-sections")) ) {
            EGS_Float factor; string medname; int what;
            int err1 = scaling->getInput("factor",factor);
            int err2 = scaling->getInput("medium",medname);
            vector<string> allowed;
            allowed.push_back("all"); allowed.push_back("Rayleigh");
            allowed.push_back("Compton"); allowed.push_back("Pair");
            allowed.push_back("Photo");
            what = scaling->getInput("cross section",allowed);
            if( !err1 && !err2 ) {
                int imed;
                if( medname == "ALL" || medname == "all" || medname == "All" )
                    imed = 0;
                else {
                    EGS_BaseGeometry::setActiveGeometryList(app_index);
                    imed = EGS_BaseGeometry::addMedium(medname); ++imed;
                    if( imed > the_media->nmed ) {
                        egsInformation("Scaling requested for medium %s,"
                              " but such medium does not exist\n",medname.c_str());
                        imed = -1;
                    }
                }
                if( imed >= 0 )
                    egsScaleXsection(&imed,&factor,&what);
            }
            delete scaling; scaling = 0;

        }

        //
        // ********* scale elastic scattering
        //
        EGS_Input *scale;
        while( (scale = options->takeInputItem("scale xcc")) ) {
            vector<EGS_Float> tmp;
            int err = scale->getInput("scale xcc",tmp);
            if( !err ) {
                int im = (int) tmp[0]; ++im;
                egsInformation("Scaling xcc of medium %d with %g\n",im,tmp[1]);
                F77_OBJ_(egs_scale_xcc,EGS_SCALE_XCC)(&im,&tmp[1]);
            }
            delete scale;
        }

        //
        // *********** calculation geometries
        //
        EGS_Input *aux;
        EGS_BaseGeometry::setActiveGeometryList(app_index);
        while( (aux = options->takeInputItem("calculation geometry")) ) {
            EGS_FACSimulation *fsim = EGS_FACSimulation::getFACSimulation(aux);
            if( fsim ) {
                sims.push_back(fsim); sim_inputs.push_back(aux);
            }
            else delete aux;
        }
        ngeom = sims.size();
        if( !ngeom ) {
            egsWarning("initScoring: no calculation geometries defined\n");
            return 1;
        }
        sim = new EGS_FACSimulation* [ngeom];
        for(int j=0; j<ngeom; j++) sim[j] = sims[j];

        //
        // correlated geometries
        //
        vector<EGS_FACCorrelation *> corrleations;
        while( (aux = options->takeInputItem("correlated geometries")) ) {
            vector<string> geoms;
            int err = aux->getInput("correlated geometries",geoms);
            if( err || geoms.size() != 2 ) egsWarning("wrong 'correlated geometries' input -> ignored\n");
            else {
                EGS_BaseGeometry *g1 = EGS_BaseGeometry::getGeometry(geoms[0]),
                                 *g2 = EGS_BaseGeometry::getGeometry(geoms[1]);
                if( !g1 ) egsWarning("no geometry with name %s available\n",geoms[0].c_str());
                if( !g2 ) egsWarning("no geometry with name %s available\n",geoms[1].c_str());
                if( g1 && g2 ) {
                    EGS_FACSimulation *sim1=0, *sim2=0; int j;
                    for(j=0; j<ngeom; ++j) {
                        if( g1 == sim[j]->geometry ) { sim1 = sim[j]; break; }
                    }
                    for(j=0; j<ngeom; ++j) {
                        if( g2 == sim[j]->geometry ) { sim2 = sim[j]; break; }
                    }
                    if( !sim1 ) egsWarning("no simulation for geometry %s found\n",geoms[0].c_str());
                    if( !sim2 ) egsWarning("no simulation for geometry %s found\n",geoms[1].c_str());
                    if( sim1 && sim2 ) corrleations.push_back(new EGS_FACCorrelation(sim1,sim2));
                    else egsWarning("--> ignored\n");
                }
                else egsWarning("--> ignored\n");
            }
        }
        if( corrleations.size() > 0 ) {
            ncor = corrleations.size();
            corr = new EGS_FACCorrelation* [ncor];
            for(int j=0; j<ncor; ++j) corr[j] = corrleations[j];
        }

        //
        // Ax calculators
        //
        vector<EGS_AxCalculator *> ax_calculators;
        while( (aux = options->takeInputItem("Ax calculation")) ) {
            vector<string> geoms;
            int err = aux->getInput("Ax calculation",geoms);
            if( err || geoms.size() != 3 ) egsWarning("wrong 'Ax calculation' input -> ignored\n");
            else {
                EGS_BaseGeometry *g0 = EGS_BaseGeometry::getGeometry(geoms[0]),
                                 *g1 = EGS_BaseGeometry::getGeometry(geoms[1]),
                                 *g2 = EGS_BaseGeometry::getGeometry(geoms[2]);
                if( !g0 ) egsWarning("no geometry with name %s available\n",geoms[0].c_str());
                if( !g1 ) egsWarning("no geometry with name %s available\n",geoms[1].c_str());
                if( !g2 ) egsWarning("no geometry with name %s available\n",geoms[2].c_str());
                if( g0 && g1 && g2 ) {
                    EGS_FACSimulation *sim0=0, *sim1=0, *sim2=0; int j;
                    for(j=0; j<ngeom; ++j) {
                        if( g0 == sim[j]->geometry ) { sim0 = sim[j]; break; }
                    }
                    for(j=0; j<ngeom; ++j) {
                        if( g1 == sim[j]->geometry ) { sim1 = sim[j]; break; }
                    }
                    for(j=0; j<ngeom; ++j) {
                        if( g2 == sim[j]->geometry ) { sim2 = sim[j]; break; }
                    }
                    if( !sim0 ) egsWarning("no simulation for geometry %s found\n",geoms[0].c_str());
                    if( !sim1 ) egsWarning("no simulation for geometry %s found\n",geoms[1].c_str());
                    if( !sim2 ) egsWarning("no simulation for geometry %s found\n",geoms[2].c_str());
                    if( sim0 && sim1 && sim2 ) ax_calculators.push_back(new EGS_AxCalculator(sim0,sim1,sim2));
                    else egsWarning("--> ignored\n");
                }
                else egsWarning("--> ignored\n");
            }
        }
        if( ax_calculators.size() > 0 ) {
            nax = ax_calculators.size();
            Ax = new EGS_AxCalculator* [nax];
            for(int j=0; j<nax; ++j) Ax[j] = ax_calculators[j];
        }

        //
        // *** muen data file
        //
        string muen_file;
        int err3 = options->getInput("muen file",muen_file);
        if( err3 ) egsFatal("\n\n***  Wrong/missing 'muen file' input\n"
             "     This is a fatal error\n\n");
        muen_file = egsExpandPath(muen_file);
        ifstream muen_data(muen_file.c_str());
        if( !muen_data ) egsFatal("\n\n***  Failed to open muen file %s\n"
                 "     This is a fatal error\n",muen_file.c_str());
        else{
            egsInformation(
                "\n\n=============== Kerma Scoring ===============\n"
                    "E*muen/rho file: %s\n"
                    "=============================================\n\n",
                    muen_file.c_str());
        }
        int ndat; muen_data >> ndat;
        if( ndat < 2 || muen_data.fail() ) egsFatal(
                   "\n\n*** Failed to read muen dfata file\n");
        EGS_Float *xmuen = new EGS_Float [ndat];
        EGS_Float *fmuen = new EGS_Float [ndat];
        for(int j=0; j<ndat; j++) muen_data >> xmuen[j] >> fmuen[j];
        if( muen_data.fail() ) egsFatal(
                "\n\n*** Failed to read muen data file\n");
        muen = new EGS_Interpolator(ndat,log(xmuen[0]),
                   log(xmuen[ndat-1]),fmuen);
        delete [] xmuen; delete [] fmuen;

        delete options;
    }
    else {
        egsWarning("\n\n*********** no 'scoring options' input *********\n\n");
        return 2;
    }

    //
    // **** variance reduction
    //
    EGS_Input *vr = input->takeInputItem("variance reduction");
    if( vr ) {
        //
        // ******** photon splitting
        //
        EGS_Float tmp; int err = vr->getInput("photon splitting",tmp);
        if( !err && tmp > 1 ) {
            fsplit = tmp; fspliti = 1/tmp;
        }
        else { fsplit = 200; fspliti = 1/fsplit; }
        //
        // ******* range rejection
        //
        rrej = EGS_RangeRejection::getRangeRejection(vr,i_ededx,i_pdedx);

        //
        // ******* increase number of transported scattered photons
        //
        vector<string> incr; incr.push_back("no"); incr.push_back("yes");
        increase_scatter =  vr->getInput("increase scatter",incr,0);

        delete vr;

    }

    bool use_rr = rrej ? true : false;
    ngsec = 0;
    int nsplit = (int) (fsplit+0.5);
    for(int j=0; j<ngeom; ++j) {
        int ni = sim[j]->geometry->getMaxStep();
        if( ni > ngsec ) ngsec = ni;
        sim[j]->rr = EGS_RangeRejection::getRangeRejection(sim_inputs[j],i_ededx,i_pdedx);
        if( sim[j]->rr ) use_rr = true;
        int nspliti = (int) (sim[j]->fsplit+0.5);
        if( nspliti > nsplit ) nsplit = nspliti;
        delete sim_inputs[j];
    }
    gsections = new EGS_GeometryIntersections [ngsec];
    ++nsplit;
    idist = new EGS_Float [nsplit];
    ilambda = new EGS_Float [nsplit];
    iindex = new int [nsplit];
    itype = new int [nsplit];

    if( use_rr ) the_egsvr->i_do_rr = 1;

    //
    // **** set up the pointer to the expmfp array in extra_stack
    //
    expmfp = the_extra_stack->expmfp;
    is_fat = the_extra_stack->is_fat;

    //
    // **** set up ausgab calls
    //
    int call;
    // Turn on all energy deposition ausgab calls
    for(call=BeforeTransport; call<=ExtraEnergy; ++call)
        setAusgabCall((AusgabCall)call,true);
    // Turn off all other ausgab calls
    for(call=AfterTransport; call<UnknownCall; ++call)
        setAusgabCall((AusgabCall)call,false);

    // Turn on calls that may produce secondary photons
    setAusgabCall(AfterBrems,true);
    if( the_xoptions->eii_flag ) {
        // with EII on, we may get fluorescent events after Moller/Bhabha
        setAusgabCall(AfterMoller,true);
        setAusgabCall(AfterBhabha,true);
    }
    setAusgabCall(AfterAnnihFlight,true);
    setAusgabCall(AfterAnnihRest,true);

    //setAusgabCall(AfterCompton,true);
    //setAusgabCall(AfterRayleigh,true);
    //setAusgabCall(AfterPhoto,true);
    //setAusgabCall(FluorescentEvent,true);
    /*
    for(call=BeforeTransport; call<UnknownCall; ++call)
        setAusgabCall((AusgabCall)call,false);
    */

    return 0;
}

void EGS_FACApplication::describeSimulation() {
    EGS_AdvancedApplication::describeSimulation();
	egsInformation("=======================\n"
                       "   Simulation details  \n"
                       "=======================\n");
    for(int j=0; j<ngeom; j++){
        sim[j]->describeSimulation();
    }
}

#ifdef BUILD_APP_LIB
APP_LIB(EGS_FACApplication);
#else
APP_MAIN(EGS_FACApplication);
#endif
