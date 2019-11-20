/*
###############################################################################
#
#  EGSnrc egs++ mevegs application
#  Copyright (C) 2019 Mevex Corporation
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
#  Authors:          Dave Macrillo,
#                    Matt Ronan,
#                    Nigel Vezeau,
#                    Lou Thompson,
#                    Max Orok
#
###############################################################################

           ███▄ ▄███▓▓█████ ██▒   █▓▓█████   ▄████   ██████
          ▓██▒▀█▀ ██▒▓█   ▀▓██░   █▒▓█   ▀  ██▒ ▀█▒▒██    ▒
          ▓██    ▓██░▒███   ▓██  █▒░▒███   ▒██░▄▄▄░░ ▓██▄
          ▒██    ▒██ ▒▓█  ▄  ▒██ █░░▒▓█  ▄ ░▓█  ██▓  ▒   ██▒
          ▒██▒   ░██▒░▒████▒  ▒▀█░  ░▒████▒░▒▓███▀▒▒██████▒▒
          ░ ▒░   ░  ░░░ ▒░ ░  ░ ▐░  ░░ ▒░ ░ ░▒   ▒ ▒ ▒▓▒ ▒ ░
          ░  ░      ░ ░ ░  ░  ░ ░░   ░ ░  ░  ░   ░ ░ ░▒  ░ ░
          ░      ░      ░       ░░     ░   ░ ░   ░ ░  ░  ░
                 ░      ░  ░     ░     ░  ░      ░       ░
                                ░
*/

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <functional>
#include <memory>

// functions for mevegs setup from gmsh
#include "gmsh_manip.h" 	  // functions for mevegs setup from gmsh MXO
// EGSnrc or IAEA phase space file output
#include "phsp_manip.h"
// output quantities calculation
#include "dosemath.h"
// Use the mesh geometry
#include "egs_mesh.h"

// NRC included files
//! We derive from EGS_AdvancedApplication => need the header file.
#include "egs_advanced_application.h"
//! We use scoring objects provided by egspp => need the header file.
#include "egs_scoring.h"
//! Every C++ EGSnrc application needs this header file
#include "egs_interface2.h"
//! We use egsInformation() => need the egs_functions.h header file.
#include "egs_functions.h"
//! We use the EGS_Input class
#include "egs_input.h"
//! To get the maximum source energy
#include "egs_base_source.h"
#include "egs_rndm.h"

//We need to make our own ausgab object for phase space output
#include "egs_ausgab_object.h"

// Interpolators for photon splitting
#include "egs_interpolator.h"

#include <cstdlib>

// #define doRayleigh F77_OBJ_(do_rayleigh,DO_RAYLEIGH)

extern __extc__ void F77_OBJ_(do_rayleigh,DO_RAYLEIGH)();
extern __extc__ void F77_OBJ(pair,PAIR)();
extern __extc__ void F77_OBJ(compt,COMPT)();
extern __extc__ void F77_OBJ(photo,PHOTO)();

//All EGS++ user codes extend the EGS_AdvancedApplication class.
class APP_EXPORT Mevegs_Application : public EGS_AdvancedApplication {

 //mesh object if mesh is read in
 //if usingMeshObj is false, don't use overloaded initGeometry fn
 Mesh* pmesh;
 bool usingMeshObj = false;

 std::string commentBlob; //comment blob to save inside mesh file after the run is done
                          //this information was typically printed to the console

 //result vectors built up by aggregateResults
 std::vector<double> allDoses, allUncerts;

    // data variables
    EGS_ScoringArray *score;    // scoring array with energies deposited
    EGS_ScoringArray *eflu;     // scoring array for electron fluence at back of geometry
    EGS_ScoringArray *gflu;     // scoring array for photon fluence at back of geometry
    EGS_ScoringArray **pheight; // pulse height distributions.
    EGS_Float        *ph_de;    // bin widths if the pulse height distributions.
    int              *ph_regions; // region indices of the ph-distributions
    int              nreg;      // number of regions in the geometry
    int              nph;       // number of pulse height objects.
    double           Etot;      // total energy that has entered the geometry
    int              rr_flag;   // used for RR and radiative splitting
    EGS_Float        current_weight; // the weight of the initial particle that
                                     // is currently being simulated
    bool  deflect_brems;
    static string revision;    // revision number

    EGS_Float       fsplit;       // photon splitting number
    EGS_Float       fspliti;      // inverse fsplit
    std::vector<int> split_media; // which media indices to split inside

    /*! Range interpolators for photon splitting in different mediums*/
    std::vector<EGS_Interpolator> rr_erange;
    std::vector<EGS_Interpolator> rr_prange;



public:

    /*! Constructor
     The command line arguments are passed to the EGS_AdvancedApplication
     contructor, which determines the input file, the pegs file, if the
     simulation is a parallel run, etc.
    */
    Mevegs_Application(int argc, char **argv, bool _usingMeshObj):
    // std::unique_ptr<onelab::remoteNetworkClient> _client) :
    // onelab::remoteNetworkClient* _client) :
        EGS_AdvancedApplication(argc,argv), usingMeshObj(_usingMeshObj),
        score(0), eflu(0), gflu(0), pheight(0), nreg(0), nph(0), Etot(0), rr_flag(0),
        current_weight(1), deflect_brems(false), fsplit(1), fspliti(1) {

        std::cout << "Successfully constructed MevEGS Application" << std::endl;
    };

    void setMeshPtr(Mesh* _pmesh){pmesh = _pmesh;};

    //returns whether this job is the last one of a parallel run using a batch script or similar
    bool isLastJob(){
      return EGS_AdvancedApplication::final_job;
    }

    //default move and copy cstors and operator=
    Mevegs_Application(const Mevegs_Application&) = default;
    Mevegs_Application(Mevegs_Application&&) = default;
    Mevegs_Application& operator=(const Mevegs_Application&) = default;
    Mevegs_Application& operator=(Mevegs_Application&&) = default;

    /*! Destructor.
     Deallocate memory
     */
    ~Mevegs_Application() override {
        if( score ) delete score;
        if( eflu ) delete eflu;
        if( gflu ) delete gflu;
        if( nph > 0 ) {
            for(int j=0; j<nph; j++) delete pheight[j];
            delete [] pheight; delete [] ph_regions; delete [] ph_de;
        }

    };

    /*! Describe the application.
     This function is called from within the initSimulation() function
     so that applications derived from EGS_AdvancedApplication can print a
     header at the beginning of the output file.
    */
    void describeUserCode() const override;

    /*! Initialize scoring.
     This function is called from within initSimulation() after the
     geometry and the particle source have been initialized.
     In our case we simple construct a scoring array with nreg+2 regions
     to collect the deposited energy in the nreg geometry regions and
     the reflected and transmitted energies, and if the user has
     requested it, initialize scoring array objects for pulse height
     distributions in the requested regions.
    */
    int initScoring() override;

    // Initialize geometry.
    // Matches vrtl fn inside egs_application.h
    // Called within initSimulation() and reimplemented here to handle Mesh
    // objects used for making tet_collection geometries w/o reading them
    // directly from an input file - MXO

    int initGeometry() override;

    /*! Accumulate quantities of interest at run time
     This function is called from within the electron and photon transport
     routines at 28 different occasions specified by iarg (see PIRS-701
     for details). Here we are only interested in energy deposition =>
     only in iarg<=4 ausgab calls and simply use the score method of
     the scoring array object to accumulate the deposited energy.
    */
    int ausgab(int iarg) override;

    /*! Output intermediate results to the .egsdat file.
     This function is called at the end of each batch. We must store
     the results in the file so that simulations can be restarted and results
     of parallel runs recombined.
     */
    int outputData() override;

    /*! Read results from a .egsdat file.
     This function is used to read simulation results in restarted
     calculations.
     */
    int readData() override;

    /*! Reset the variables used for accumulating results
     This function is called at the beginning of the combineResults()
     function, which is used to combine the results of parallel runs.
    */
    void resetCounter() override;

    /*! Add simulation results
     This function is called from within combineResults() in the loop
     over parallel jobs. data is a reference to the currently opened
     data stream (basically the j'th .egsdat file).
     */
    int addState(istream &data) override;

    /*! Output the results of a simulation. */
    void outputResults() override;

    /*! Get the current simulation result.
     This function is called from the run control object in parallel runs
     in order to obtain a combined result for all parallel jobs.
     A single result is requested (and so, in simulations that calculate
     many quantites such as a 3D dose distribution, it is up to the user
     code to decide which single result to return). If this function is
     not reimplemented in a derived class, the run control object will simply
     not store information about the combined result in the JCF and not print
     this info in the log file. In our case we arbitrarily decide to return the
     reflected energy fraction as the single result of the simulation.
    */
    void getCurrentResult(double &sum, double &sum2, double &norm,
            double &count);

    /*! Select photon mean-free-path
     *  used to implement photon splitting variance reduction
     */
    void selectPhotonMFP(EGS_Float &dpmfp);

    //send out our result vectors into the world to possibly be put in a gmsh pos or mesh file
    void getResultVectors(std::vector<double>& _doses,
                          std::vector<double>& _uncerts) const {
        _doses   = allDoses;
        _uncerts = allUncerts;
    }

    const std::string getInputFileName() const {
      return EGS_Application::input_file;
    }

    //overloaded information functions from EGS_Application, c.f. lines 1016-1038
    void appInformation(const char *msg) override {
      //put into comment blob for saving out to files
      Mevegs_Application::commentBlob += std::string(msg);
      //also print to screen
      EGS_Application::appInformation(msg);
    }

    const std::string& getRunComments() const {
      return Mevegs_Application::commentBlob;
    }

    //returns the total energy of the simulation.
    const double getETot() const{
      return Etot;
    }

    //returns the current number of particles in the problem.
    //Calling after the simulation is done will be Equivalent
    //to the total number of particles in the simulation.
    const long long getNParticles() const{
      return static_cast<long long>(current_case);
    }

    //returns the number of regions in the current problem
    const int getNReg() const{
      return nreg;
    }

    //returns the tet collection object representing the problem geometry for
    //this simulation
    //by itself doesn't change geometry but cannot be returned as const -> edited later
    EGS_Mevex_tet_collection* getGeometry() const{
       return dynamic_cast<EGS_Mevex_tet_collection*>(geometry);
    }

    //get an array for the internal egs "media map" of indices to densities
    const double* getDensities() const{
      return the_media->rho;
    }

    //get all the results for the simulation with a given mesh object
    dosemath::namedResults calculateResults(const Mesh& mesh);

protected: // called by the base class

    /*! Start a new shower.
     This function is called from within the shower loop just before the
     actual simulation of the particle starts. The particle parameters are
     available via the protected base class variable p which is of type
     EGS_Particle (see egs_application.h).
    */
    int startNewShower();

private:

    // prepare interpolators for photon splitting
    void prepare_rr_interpolators();

    //MXO -> save results to internal result vectors
    inline void aggregateResults();
};

// mevegs revision 0.1
string Mevegs_Application::revision = "1.0";

extern "C" void F77_OBJ_(egs_scale_xcc,EGS_SCALE_XCC)(const int *,const EGS_Float *);
extern "C" void F77_OBJ_(egs_scale_bc,EGS_SCALE_BC)(const int *,const EGS_Float *);

// add photon splitting (adapted from egs_chamber)
extern __extc__ void
F77_OBJ_(select_photon_mfp, SELECT_PHOTON_MFP)(EGS_Float *dpmfp) {

    EGS_Application *a = EGS_Application::activeApplication();
    Mevegs_Application *app = dynamic_cast<Mevegs_Application *>(a);
    if (!app) {
       egsFatal("select_photon_mfp called with active application "
              "not of type Mevegs_Application!\n");
    }
    app->selectPhotonMFP(*dpmfp);

}

// Find the photon mean-free-path
// -- adapted from egs_chamber.cpp
//
// If photon splitting is turned on, the entire photon transport
// chain happens here.
//
// After photon splitting, @arg dummy_mfp is set to -1 and flags an early exit
// for PHOTON (since transport was already done here)
//
// If photon splitting wasn't requested, @dummy_mfp returns a typical mean free
// path value and PHOTON continues as normal.

void Mevegs_Application::selectPhotonMFP(EGS_Float &dummy_mfp) {

    // go from Fortran 1-based indexing to 0-based indexing
    int np = the_stack->np-1;

    // error checks before splitting

    // fail if not called with a photon -> iq is the integer particle charge
    if (the_stack->iq[np]) {
        egsFatal("selectPhotonMFP called with a "
                 "particle of charge %d\n", the_stack->iq[np]);
    }

    // no photon splitting, return a typical mean-free-path
    if (this->fsplit <= 1) {
        dummy_mfp = -log(1 - rndm->getUniform());
        return;
    }

    // check whether splitting was requested in this medium
    int ireg = the_stack->ir[np]-2;    // current region
    int imed = geometry->medium(ireg); // media of region

    // this media wasn't requested for splitting, return a typical mfp
    if (this->split_media.end() == std::find(this->split_media.begin(), this->split_media.end(), imed)) {
        dummy_mfp = -log(1 - rndm->getUniform());
        return;
    }

    // otherwise, get ready for photon splitting

    EGS_Float wt_o = the_stack->wt[np];
    EGS_Float E    = the_stack->E[np];
    int latch      = the_stack->latch[np];
    int latch1     = latch;

    EGS_Float split_num, inv_split_num;
    // why is latch the determinant here?
    if (latch < 2) {
        split_num  = this->fsplit;
        inv_split_num = this->fspliti;
    }
    else { // use range rejection heuristic
        split_num  = rr_flag;
        inv_split_num = 1/split_num;
        latch1   = latch - rr_flag;
        the_stack->latch[np] = latch1;
    }
    // adjust statistical weight by the number of photons split
    the_stack->wt[np] = wt_o * inv_split_num;


    // relative gamma mean free path (like rhor = relative rho)
    EGS_Float gmfpr = 1e15;
    // coherent scattering factor
    EGS_Float cohfac = 1;
    // natural log of photon energy
    EGS_Float gle = the_epcont->gle;

    if (imed >= 0) { // if it's not in vacuum
        gmfpr = EGS_AdvancedApplication::i_gmfp[imed].interpolateFast(gle);
        // iraylr is a flag for turning on coherent (Rayleigh) scattering
        if (the_xoptions->iraylr) {
            cohfac = EGS_AdvancedApplication::i_cohe[imed].interpolateFast(gle);
            // adjust relative gamma MFP by coherent scattering factor
            gmfpr *= cohfac;
        }
    }

    // get position
    EGS_Vector x(the_stack->x[np], the_stack->y[np], the_stack->z[np]);
    // get velocity
    EGS_Vector u(the_stack->u[np], the_stack->v[np], the_stack->w[np]);
    // get relative density
    EGS_Float rhor = the_useful->rhor;

    // calculate gamma mean free path
    EGS_Float gmfp = gmfpr/rhor;

    // how many photons survive (0 to split_num)
    // this means sometimes no photons are split
    int i_survive = static_cast<int> (split_num * rndm->getUniform());

    // loop variables
    EGS_Float mfp_old = 0;
    EGS_Float eta_prime = 1 + rndm->getUniform() * inv_split_num;
    dummy_mfp = -1;
    int isplit = 0;

    while(1) {
        // eta_prime is some sort of energy store ?
        // guaranteed to pass this check at least once since eta_prime initialized to
        // a larger number.
        eta_prime -= inv_split_num;
        if( eta_prime <= 0 ) {
            --the_stack->np;
            // early exit
            return;
        }

        // find the mean free path of this iteration's photon
        EGS_Float mfp = -log(eta_prime) - mfp_old;
        mfp_old = mfp_old + mfp;

        // EGS_Float xp , up, t;
        // double ttot = 0;

        while(1) {

            EGS_Float tstep = mfp*gmfp; // mfp * gmfp : [time] ???
            int newmed;
            int inew = geometry->howfar(ireg,x,u,tstep,&newmed);
            if( inew < 0 ) {
               --the_stack->np;
               return;
            }

            // distance += velocity * time
            x += u * tstep;
            if( inew == ireg ) {
                break;
            }

            mfp -= tstep/gmfp;

            if( geometry->hasRhoScaling() ) {
                rhor = geometry->getRelativeRho(inew);
            }
            else {
                rhor = 1;
            }
            // go to new region
            ireg = inew;
            // change material data if the new region is a different medium
            //
            // Note: if the particle reaches a new medium, split there even if
            // splitting wasn't requested. This simplifies splitting setup in initScoring
            if( newmed != imed ) {
                imed = newmed;
                the_useful->medium = imed+1;
                if( imed >= 0 ) {
                    gmfpr = EGS_AdvancedApplication::i_gmfp[imed].interpolateFast(gle);
                    if( the_xoptions->iraylr ) {
                        cohfac = EGS_AdvancedApplication::i_cohe[imed].interpolateFast(gle);
                        gmfpr *= cohfac;
                    }
                }
                else {
                    gmfpr=1e15, cohfac=1;
                }
            }
            gmfp = gmfpr/rhor;
        }

        // update stack particle data after step
        the_stack->x[np]=x.x;
        the_stack->y[np]=x.y;
        the_stack->z[np]=x.z;
        the_stack->ir[np] = ireg+2;
        // optional distance to next boundary
        the_stack->dnear[np] = 0;

        // finally ready to interact

        // Rayleigh interaction?
        bool is_rayleigh = false;
        if( the_xoptions->iraylr ) {
            if( rndm->getUniform() < 1 - cohfac ) {
                is_rayleigh = true;
                if( isplit != i_survive ) {
                    --np;
                    --the_stack->np;
                }
                else {
                 the_stack->wt[np] = wt_o;
                 F77_OBJ_(do_rayleigh,DO_RAYLEIGH)();
                 the_stack->latch[np] = latch < 2 ?
                        1 : (rr_flag+1);
                }
            }
        }

        // not Rayleigh, check the other interaction types
        if( !is_rayleigh ) {

            EGS_Float gbr1, gbr2;
            gbr1 = EGS_AdvancedApplication::i_gbr1[imed].interpolateFast(gle);
            gbr2 = EGS_AdvancedApplication::i_gbr2[imed].interpolateFast(gle);
            EGS_Float eta = rndm->getUniform();

            // pair production
            if( E > the_thresh->rmt2 && eta < gbr1 ) {
                F77_OBJ(pair,PAIR)();
            }

            // Compton scattering
            else if( eta < gbr2 ) {
                F77_OBJ(compt,COMPT)();
            }

            // photo-electric absorption
            else {
                F77_OBJ(photo,PHOTO)();
            }

            np = the_stack->np-1;
            int ip = the_stack->npold-1;

            // old way checked against a list of cavity geometries
            //
            bool do_rr = (rr_flag > 0); // && !is_cavity[ig][ireg]);
            // if( do_rr && cgeoms[ig] ) {
            //    if( !cgeoms[ig]->isInside(x) ) cperp = cgeoms[ig]->hownear(-1,x);
            //    else do_rr = false;
            // } else do_rr = false;

            do {
                if( !the_stack->iq[ip] ) {
                    if( isplit == i_survive ) {
                      the_stack->wt[ip] = wt_o;
                      the_stack->latch[ip++] = latch < 2 ?
                            1 : (rr_flag+1);
                    }
                    else {
                    if( ip < np ) {
                            the_stack->E[ip] = the_stack->E[np];
                            the_stack->iq[ip] = the_stack->iq[np];
                            the_stack->latch[ip] = the_stack->latch[np];
                            the_stack->u[ip] = the_stack->u[np];
                            the_stack->v[ip] = the_stack->v[np];
                            the_stack->w[ip] = the_stack->w[np];
                    }
                    --np; --the_stack->np;
                    }
                }
                else {
                    bool keep = true;
                    if( do_rr ) {
                        EGS_Float crange = 0;

                        // check the particle's energy
                        EGS_Float e = the_stack->E[ip] - the_useful->rm;
                        if( e > 0 ) {
                            // elke is misleading: can also refer to a photon's energy
                            EGS_Float elke=log(e); // ln of electron kinetic energy
                            crange = (the_stack->iq[ip] == -1)
                                ? rr_erange[imed].interpolate(elke) // electron
                                : rr_prange[imed].interpolate(elke);  // photon
                        }

                        EGS_Float cperp=1e30;
                        if( crange < cperp ) {
                            if( rr_flag == 1 ) {
                                keep = false;
                            }
                            else {
                                if( rndm->getUniform()*rr_flag < 1 ) {
                                    the_stack->wt[ip] *= rr_flag;
                                    the_stack->latch[ip] += rr_flag;
                                } else keep = false;
                            }
                        }
                    }
                    if( keep ) ++ip;
                    else {
                        if( ip < np ) {
                            the_stack->E[ip] = the_stack->E[np];
                            the_stack->iq[ip] = the_stack->iq[np];
                            the_stack->latch[ip] = the_stack->latch[np];
                            the_stack->u[ip] = the_stack->u[np];
                            the_stack->v[ip] = the_stack->v[np];
                            the_stack->w[ip] = the_stack->w[np];
                        }
                        --np; --the_stack->np;
                    }
                }
            } while (ip <= np);
        }

        // split a new photon
        ++isplit;
        ++np;            // increment particle number
        ++the_stack->np; // increment stack particle number
        // initialize the new particle
        the_stack->E[np] = E;
        the_stack->wt[np] = wt_o*inv_split_num;
        the_stack->iq[np] = 0;
        the_stack->latch[np] = latch1;
        the_stack->ir[np] = ireg+2;
        the_stack->u[np]=u.x; the_stack->v[np]=u.y; the_stack->w[np]=u.z;
        the_stack->x[np]=x.x; the_stack->y[np]=x.y; the_stack->z[np]=x.z;
        the_stack->dnear[np] = 0;
    }
}

//MXO
//try to expose WATCH subroutine in nrcaux.mortran to our code
//experimental
extern "C" void F77_OBJ_(watch, WATCH)(int iarg, int iwatch);

//Prints some text before the simulation. Gets called by EGS.
void Mevegs_Application::describeUserCode() const {
   egsInformation("\nMEVEGS\n");
   egsInformation("This is MEVEGS %s based on\n"
     "      Tutor7ppApplication %s\n\n",
     egsSimplifyCVSKey(revision).c_str(),
     egsSimplifyCVSKey(base_revision).c_str());
}

//Initialize scoring quantities for EGS, as well as some other things
//that are convenient to initialize at the same time, such as
//variance reduction.
//Originally from tutor7pp, lots of additions.
int Mevegs_Application::initScoring() {

  // Get the number of regions in the geometry.
  this->nreg = geometry->regions();

  this->score = new EGS_ScoringArray(nreg+2);
   //i.e. we always score energy fractions
  this->eflu = new EGS_ScoringArray(200); this->gflu = new EGS_ScoringArray(200);

    //try setting the WATCH
    //"this is for the watch"
    //F77_OBJ_(watch, WATCH)(-99, 1);

  the_egsvr->i_do_rr = 1; //Nigel's bugfix re breaking brems with ausgab

  //JBT: Code to read in some variance reduction parameters from egsinp.
  EGS_Input *vr = input->takeInputItem("variance reduction");
  if( vr ) {

    egsInformation("Variance reduction options\n==========================\n");

    // Photon splitting
    EGS_Float tmp = 1;

    if ((! vr->getInput("photon splitting", tmp) && tmp > 1)) {
        egsInformation("Photon splitting:");

        if (! this->usingMeshObj) {
            egsFatal("\nphoton splitting in Mevegs requires a mesh object, aborting");
        }

        egsWarning("\nWarning: photon splitting support is experimental");
        this->fsplit = tmp;
        this->fspliti = 1 / tmp;
        egsInformation("\n => initScoring: splitting photons %g times", this->fsplit);

        // check which media we want to split with
        std::map<int, string> mesh_media = this->pmesh->getMediaMap();
        // flip map to initialize the right indices based on medium strings
        std::map<string, int> rev_mesh_media;
        for (auto media: mesh_media) {
            rev_mesh_media.insert(std::make_pair(media.second, media.first));
        }

        std::vector<string> splitting_media;
        if (! vr->getInput("splitting media", splitting_media)) {
            egsInformation("\n => initScoring: splitting media specified in input file");
            // check input media names against the mesh names
            for (auto media_name: splitting_media) {
                auto media_iter = rev_mesh_media.find(media_name);
                // quit for unknown media names
                if (media_iter == rev_mesh_media.end()) {
                    egsFatal("\n => initScoring: unknown splitting media name: %s\n aborting\n", media_name.c_str());
                }
                // found it
                int media_num = rev_mesh_media.at(media_name);
                this->split_media.push_back(media_num);
            }
        }

        // no splitting media set, default to all media
        else {
            egsInformation("\n => initScoring: no splitting media specified, defaulting to all media");
            for (auto& media_pair : rev_mesh_media) {
                // add media index to split list
                this->split_media.push_back(media_pair.second);
            }
        }

        // initialize interpolators for photon splitting loop
        this->prepare_rr_interpolators();

        // print in order
        std::sort(split_media.begin(), split_media.end());
        // print media we're splitting inside
        for (auto split_media_num: split_media) {
           egsInformation("\n => initScoring: splitting in %s, medium number %d", mesh_media.at(split_media_num).c_str(), split_media_num);
        }

    }
    else {
        egsInformation("\n => initScoring: photon splitting off");
    }

    //Radiative Splitting (brems)
    int csplit=1;
    if( !vr->getInput("radiative splitting", csplit) && csplit > 1) {
       egsInformation("\nRadiative (brems) splitting:");
       egsInformation("\n => initScoring: splitting radiative events %d times", csplit);
       the_egsvr->nbr_split = csplit;
    }

   //Russian Roulette
   int n_rr;
   if( !vr->getInput("russian roulette",n_rr) && n_rr > 1 ) {
       egsInformation("\nRussian roulette:");
       the_egsvr->i_do_rr = n_rr;
       setAusgabCall(BeforeBrems,true);
       setAusgabCall(AfterBrems,true);
       setAusgabCall(BeforeAnnihFlight,true);
       setAusgabCall(AfterAnnihFlight,true);
       setAusgabCall(BeforeAnnihRest,true);
       setAusgabCall(AfterAnnihRest,true);
       //setAusgabCall(FluorescentEvent,true);
       egsInformation("\n => initScoring: russian roulette with survival probability 1/%d\n\n",n_rr);
     }
   delete vr;
  }

  EGS_Input *options = input->takeInputItem("scoring options");
  if( options ) {

      EGS_Input *scale;
      while( (scale = options->takeInputItem("scale xcc")) ) {
          vector<string> tmp;
          int err = scale->getInput("scale xcc",tmp);
          //egsInformation("Found 'scale xcc', err=%d tmp.size()=%d\n",err,tmp.size());
          if( !err && tmp.size() == 2 ) {
              int imed = EGS_BaseGeometry::getMediumIndex(tmp[0]) + 1;
              if( imed > 0 ) {
                  EGS_Float fac = atof(tmp[1].c_str());
                  egsInformation("\n ***** Scaling xcc of medium %d with %g\n",imed,fac);
                  F77_OBJ_(egs_scale_xcc,EGS_SCALE_XCC)(&imed,&fac);
              }
          }
          delete scale;
      }
      while( (scale = options->takeInputItem("scale bc")) ) {
          vector<string> tmp;
          int err = scale->getInput("scale bc",tmp);
          //egsInformation("Found 'scale xcc', err=%d tmp.size()=%d\n",err,tmp.size());
          if( !err && tmp.size() == 2 ) {
              int imed = EGS_BaseGeometry::getMediumIndex(tmp[0]) + 1;
              if( imed > 0 ) {
                  EGS_Float fac = atof(tmp[1].c_str());
                  egsInformation("\n ***** Scaling bc of medium %d with %g\n",imed,fac);
                  F77_OBJ_(egs_scale_bc,EGS_SCALE_BC)(&imed,&fac);
              }
          }
          delete scale;
      }

      std::vector<string> choices; choices.push_back("no"); choices.push_back("yes");
      deflect_brems = options->getInput("deflect electron after brems",choices,0);
      if( deflect_brems ) {
          egsInformation("\n *** Using electron deflection in brems events\n\n");
          setAusgabCall(AfterBrems,true);
      }

      int n_rr;
      if( !options->getInput("Russian Roulette",n_rr) && n_rr > 1 ) {
          the_egsvr->i_do_rr = n_rr;
          setAusgabCall(BeforeBrems,true);
          setAusgabCall(AfterBrems,true);
          setAusgabCall(BeforeAnnihFlight,true);
          setAusgabCall(AfterAnnihFlight,true);
          setAusgabCall(BeforeAnnihRest,true);
          setAusgabCall(AfterAnnihRest,true);
          //setAusgabCall(FluorescentEvent,true);
          egsInformation("\nUsing Russian Roulette with survival probability 1/%d\n",n_rr);
      }

      // The user has provided scoring options input.
      // See where she/he wants to score a pulse height distribution
      // and how many bins to use for each pulse height distribution
      vector<int> regions;
      int err = options->getInput("pulse height regions",regions);
      vector<int> nbins;
      int err1 = options->getInput("pulse height bins",nbins);
      if( !err && !err1 ) {
          if( regions.size() != nbins.size() && nbins.size() != 1 )
                  egsWarning("initScoring(): you must input the same "
              "number of 'regions' and 'bins' inputs or a single 'bins'"
              " input\n");
          else {
              EGS_ScoringArray **tmp = new EGS_ScoringArray* [nreg+2];
              for(int i=0; i<nreg+2; i++) tmp[i] = 0;

              for(std::size_t j=0; j<regions.size(); j++) {
                  int nb = nbins.size() == 1 ? nbins[0] : nbins[j];
                  if( nb < 1 )
                      egsWarning("zero bins for region %d?\n",regions[j]);
                  if( regions[j] < 0 || regions[j] > nreg+1 )
                      egsWarning("invalid region index %d\n",regions[j]);
                  if( nb > 0 && regions[j] >= 0 && regions[j] < nreg+2 ){
                      int ij = regions[j];
                      if( tmp[ij] ) egsInformation("There is already a "
                "PHD object in region %d => ignoring it\n",ij);
                      else { tmp[ij] = new EGS_ScoringArray(nb); ++nph; }
                  }
              }
              if( nph > 0 ) {
                  pheight = new EGS_ScoringArray* [nph];
                  ph_regions = new int [nph];
                  ph_de = new EGS_Float [nph];
                  EGS_Float Emax = source->getEmax();
                  int iph = 0;
                  for(int j=0; j<nreg+2; j++) {
                      if( tmp[j] ) {
                          pheight[iph] = tmp[j];
                          ph_regions[iph] = j;
                          int nbin = pheight[iph]->bins();
                          ph_de[iph++] = Emax/nbin;
                      }
                  }
              }
              delete [] tmp;
          }
      } else egsWarning("initScoring(): you must provide both, 'regions'"
                 " and 'bins' input\n");
      delete options;
    }

  // add phase space options here
     // MXO 5-Aug-2018
     // the user wants a phsp space output (dear god they're insatiable)
     // find out where the scoring plane is (only handle x,y,z planes rn)
     EGS_Input *phsp = input->takeInputItem("phase space definition");
     if( phsp ) {
         //phase space geometry options
         constexpr auto phspKey = "phase space geometry";
         constexpr auto posKey  = "position";

         const auto geometry_opt = phsp_manip::geo_opt_strs;

         std::string phspGeometryVal;
         double phspPos;
         if( !phsp->getInput(phspKey, phspGeometryVal)){
            if(phsp->getInput(posKey, phspPos)) {
              egsWarning("\n => You must specify a position with your phase space plane...");
            }
            else {
              egsInformation("\n=> found phase space geometry definition %s ", phspGeometryVal.c_str());

              std::string phspOpt; // for printing out options on failure
              bool allowableOpt = false;
              for (const auto & opt : geometry_opt){
                if (phspGeometryVal == opt){
                  egsInformation("\n  => creating slice using %s geometry at position %f \n", phspGeometryVal.c_str(), phspPos);
                  allowableOpt = true;
                  break;
                }
                else {
                  phspOpt += opt + "\n";
                }
              }
              //fail if the option is bad
              if (!allowableOpt){
                egsWarning("\n => That geometry is not an option, please change it to one of the following: \n\n%s ", phspOpt.c_str());
              }
              //success! make a fake input file for the ausgab definition
              else {

                //default format to egsnrc
                std::string phsp_format_str = "EGSnrc";
                egsInformation("default phsp output format is %s\n", phsp_format_str.c_str());

                constexpr auto phsp_format_key  = "format";
                //check for IAEA format
                std::string format_temp;
                if(!phsp->getInput(phsp_format_key, format_temp)){
                  if (format_temp == "IAEA") {
                    phsp_format_str = format_temp;
                  }
                }

                egsInformation("using: %s output format\n", phsp_format_str.c_str());

                auto phsp_type = make_pair(phspGeometryVal, phspPos);
                egsInformation("\nphsp geometry type: %s", phspGeometryVal.c_str());
                egsInformation("\nphsp position: %f", phspPos);

                //default to EGSnrc for now
                //make internal egsinp to avoid geometry problems in egsinp
                EGS_Input phsp_input = phsp_manip::get_egsinp(phsp_format_str, phsp_type);

                EGS_BaseGeometry::describeGeometries();
                //register the scoring plane to the internal list of active geometries
                EGS_BaseGeometry::createGeometry(&phsp_input);
                //make a phase space scoring object manually
                EGS_AusgabObject::createAusgabObjects(&phsp_input);
              }
            }
         }
          else {
            egsWarning("\n Warning: no geometry type found for phase space file output...");
          }
         delete phsp;
        }
         // notify if phase space definition wasn't found
         else {
           egsInformation("\nNo phase space input was found for this file\n");
         }
  return 0;
}


// Prepare interpolators for the photon splitting loop
// we handle every medium to simplify setup and implementation
//
// Interpolator setup code is taken from egs_chamber.cpp's initScoring method
void Mevegs_Application::prepare_rr_interpolators() {

    int nmed = this->geometry->nMedia();
    // reserve vector space (and initialize vector)
    this->rr_erange.reserve(nmed);
    this->rr_prange.reserve(nmed);

    // some magic number of bins
    int nbin = 512;

    // i is media index, used to access media data
    for (int i = 0; i < nmed; ++i) {

        EGS_Float log_emin = this->i_ededx[i].getXmin();
        EGS_Float log_emax = this->i_ededx[i].getXmax();

        // derivative of energy logarithm
        EGS_Float dloge = (log_emax - log_emin) / nbin;

        // new range lookup tables (maybe a memory leak if not deleted in interpolator)
        EGS_Float *erange = new EGS_Float [nbin];
        EGS_Float *prange = new EGS_Float [nbin];
        // start at zero
        erange[0] = 0;
        prange[0] = 0;

        // loop energy variables
        EGS_Float ededx_old = EGS_AdvancedApplication::i_ededx[i].interpolate(log_emin);
        EGS_Float pdedx_old = EGS_AdvancedApplication::i_pdedx[i].interpolate(log_emin);
        EGS_Float Eold = exp(log_emin);
        EGS_Float efak = exp(dloge);

        // populate lookup tables
        for (int j = 1; j < nbin; ++j) {
            EGS_Float elke = log_emin + dloge * j; // ln of electron's kinetic energy
            EGS_Float E = Eold * efak;

            EGS_Float ededx = i_ededx[i].interpolate(elke);
            EGS_Float pdedx = i_pdedx[i].interpolate(elke);

            EGS_Float E_del = E - Eold;

            // 1.02???
            if (ededx < ededx_old) {
                erange[j] = erange[j-1] + 1.02 * E_del / ededx;
            } else {
                erange[j] = erange[j-1] + 1.02 * E_del / ededx_old;
            }

            if (pdedx < pdedx_old) {
                prange[j] = prange[j-1] + 1.02 * E_del / pdedx;
            } else {
                prange[j] = prange[j-1] + 1.02 * E_del / pdedx_old;
            }

            Eold = E;
            ededx_old = ededx;
            pdedx_old = pdedx;
        }

       // add to internal list of interpolators
       this->rr_erange.push_back(EGS_Interpolator(nbin, log_emin, log_emax, erange));
       this->rr_prange.push_back(EGS_Interpolator(nbin, log_emin, log_emax, prange));
    }
    return;
}


//initializes the tet collection geometry for the simulation, either from a
//mesh class or from a tet file
int Mevegs_Application::initGeometry(){

  if (Mevegs_Application::usingMeshObj){

    EGS_BaseGeometry::setActiveGeometryList(app_index);

    //Read in scaling factor
    double scaling = 10;
    EGS_Input *mevegs = input->takeInputItem("mevegs");
    if( mevegs ) {
      if(!mevegs->getInput("scaling", scaling) && scaling != 0) {
        egsInformation("\n => initScoring: scaling by a factor of %lf", scaling);
      }
      delete mevegs;
    }

    if(scaling != scaling) {//somehow got NaN scaling (:
      egsInformation("\nError scaling, got NaN! Using default of 10.");
      scaling = 10;
    }

    geometry = createMeshGeometry(input, scaling, *pmesh);

    egsInformation("\nMesh class\n");
    egsInformation("nregions: %d \n", geometry->regions());
    egsInformation("nmedia: %d \n", geometry->nMedia());
    egsInformation("label count: %d \n", geometry->getLabelCount());
    EGS_BaseGeometry::describeGeometries();

      if (!geometry) {
        return 1;
      }
      geometry->ref();
      return 0;

  }

  else {

  int initErr = EGS_Application::initGeometry();

  egsInformation("\nTET DATA FILE\n");
  egsInformation("nregions: %d \n", geometry->regions());
  egsInformation("nmedia: %d \n", geometry->nMedia());
  egsInformation("label count: %d \n", geometry->getLabelCount());
  EGS_BaseGeometry::describeGeometries();

  return initErr;
  }
}

//output scoring quantities to an ausgab object
//From tutor7pp.
int Mevegs_Application::ausgab(int iarg) {
    if( iarg <= 4 ) {
        int np = the_stack->np - 1; int ir = the_stack->ir[np]-1;
        if( ir == 0 && the_stack->w[np] > 0 ) ir = nreg+1;

        EGS_Float aux = the_epcont->edep*the_stack->wt[np];
        if(aux > 0) {
            score->score(ir,aux);
        }

        // if( the_stack->iq[np] ) score->score(ir,the_epcont->edep*the_stack->wt[np]);
        if( ir == nreg+1 ) {
            EGS_ScoringArray *flu = the_stack->iq[np] ? eflu : gflu;
            EGS_Float r2 = the_stack->x[np]*the_stack->x[np] + the_stack->y[np]*the_stack->y[np];
            if( r2 < 400 ) {
                int bin = (int) (sqrt(r2)*10.);

                aux = the_stack->wt[np]/the_stack->w[np];
                if(aux > 0) {
                    flu->score(bin,aux);
                }
            }
        }
        return 0;
    }
    int np = the_stack->np-1;
    if( iarg == BeforeBrems || iarg == BeforeAnnihRest || (iarg == BeforeAnnihFlight &&
        the_stack->latch[np] > 0 )) {
        the_stack->latch[np] = 0; rr_flag = 1;
        //NB this should be okay, but if we get weird results uncomment this line or set
        //russian roulette and brems splitting to the same constant
        //the_egsvr->nbr_split = the_egsvr->i_do_rr;
        return 0;
    }
    if( iarg == AfterBrems && deflect_brems ) {
        EGS_Vector u(the_stack->u[np-1],the_stack->v[np-1],the_stack->w[np-1]);
        EGS_Float tau = the_stack->E[np-1]/the_useful->rm - 1;
        EGS_Float beta = sqrt(tau*(tau+2))/(tau+1);
        EGS_Float eta = 2*rndm->getUniform()-1;
        EGS_Float cost = (beta + eta)/(1 + beta*eta);
        EGS_Float sint = 1 - cost*cost;
        if( sint > 0 ) {
            sint = sqrt(sint); EGS_Float cphi, sphi;
            rndm->getAzimuth(cphi,sphi); u.rotate(cost,sint,cphi,sphi);
            the_stack->u[np-1] = u.x;
            the_stack->v[np-1] = u.y;
            the_stack->w[np-1] = u.z;
        }
    }

    if( iarg == AfterBrems || iarg == AfterAnnihRest || iarg == AfterAnnihFlight ) {
        the_egsvr->nbr_split = 1;
        if( iarg == AfterBrems && rr_flag ) {
            the_stack->latch[the_stack->npold-1] = 1;
        }
        rr_flag = 0; return 0;
    }

    return 0;
}

//Outputs the current data of the simulation to a file.
//This is used internally by EGS for restarting and combining runs.
//From tutor7pp.
int Mevegs_Application::outputData() {
    int err = EGS_AdvancedApplication::outputData();
    if( err ) return err;
    // We first call the outputData() function of our base class.
    // This takes care of saving data related to the source, the random
    // number generator, CPU time used, number of histories, etc.
    // We then write our own data to the data stream. data_out is
    // a pointer to a data stream that has been opened for writing
    // in the base class.
    (*data_out) << " " << Etot << endl;
    if( !score->storeState(*data_out) ) return 101;
    for(int j=0; j<nph; j++) {
        if( !pheight[j]->storeState(*data_out) ) return 102+j;
    }
    if( !eflu->storeState(*data_out) ) return 301;
    if( !gflu->storeState(*data_out) ) return 302;
    return 0;
}

//This function gets called when you restart a run.
//It sets the state of EGS to the final state of the previous simulation
//whose data this function reads.
//From tutor7pp.
int Mevegs_Application::readData() {
    // We first call the readData() function of our base class.
    // This takes care of reading data related to the source, the random
    // number generator, CPU time used, number of histories, etc.
    // (everything that was stored by the base class outputData() method).
    int err = EGS_AdvancedApplication::readData();
    if( err ) return err;
    // We then read our own data from the data stream.
    // data_in is a pointer to an input stream that has been opened
    // by the base class.
    (*data_in) >> Etot;
    if( !score->setState(*data_in) ) return 101;
    for(int j=0; j<nph; j++) {
        if( !pheight[j]->setState(*data_in) ) return 102+j;
    }
    if( !eflu->setState(*data_in) ) return 301;
    if( !gflu->setState(*data_in) ) return 302;
    return 0;
}

//Resets all aspects of the simulation to empty/0/nothing.
//Used right before combining results from parallel simulations.
//From tutor7pp.
void Mevegs_Application::resetCounter() {
    // Reset everything in the base class
    EGS_AdvancedApplication::resetCounter();
    // Reset our own data to zero.
    score->reset(); Etot = 0;
    for(int j=0; j<nph; j++) pheight[j]->reset();
    eflu->reset(); gflu->reset();
}

//this code gets called from combineResults() when combining parallel jobs.
//From tutor7pp.
int Mevegs_Application::addState(istream &data) {
    // Call first the base class addState() function to read and add
    // all data related to source, RNG, CPU time, etc.
    int err = EGS_AdvancedApplication::addState(data);
    if( err ) return err;
    // Then read our own data to temporary variables and add to
    // our results.
    double etot_tmp; data >> etot_tmp; Etot += etot_tmp;
    EGS_ScoringArray tmp(nreg+2);
    if( !tmp.setState(data) ) return 101;
    (*score) += tmp;
    for(int j=0; j<nph; j++) {
        EGS_ScoringArray tmpj(pheight[j]->bins());
        if( !tmpj.setState(data) ) return 102 + j;
        (*pheight[j]) += tmpj;
    }
    EGS_ScoringArray tmp1(200);
    if( !tmp1.setState(data) ) return 301;
    (*eflu) += tmp1;
    if( !tmp1.setState(data) ) return 302;
    (*gflu) += tmp1;
    return 0;
}

//save all results to result vectors.
//this is where the internal EGS EFrac and Uncert values get calculated
void Mevegs_Application::aggregateResults(){

  egsInformation("aggregating results for job %d\n", getIparallel());

  //normalize the results
  double norm = (static_cast<double>(current_case))/Etot;
  auto currScore = Mevegs_Application::score;

  // go over score array and load up dose and uncertainty vectors
  // the score array is size nreg + 2, first elt is reflected, last is transmitted
  // start at 1 to skip energy reflected, stop 1 before end to skip transmitted
  std::vector<double> _doses, _uncerts; //result vectors
  for (int i = 1; i < Mevegs_Application::nreg + 1; ++i){
    double _dose, _uncert;
    currScore->currentResult(i, _dose, _uncert);

    _doses.emplace_back(_dose*norm);
    _uncerts.emplace_back(_uncert*norm);
  }

  allDoses   = _doses;
  allUncerts = _uncerts;

  char reflectBuffer[100];
  char transmitBuffer[100];
  //comment reflected and transmitted fractions to mesh file
  double dosebuff, uncertbuff;
  currScore->currentResult(0, dosebuff, uncertbuff);
  sprintf(reflectBuffer, "Reflected (-Z) Fraction: %lf +/- %lf\n", dosebuff*norm, uncertbuff*norm);
  currScore->currentResult(nreg + 1, dosebuff, uncertbuff);
  sprintf(transmitBuffer, "Transmitted (+Z) Fraction: %lf +/- %lf\n", dosebuff*norm, uncertbuff*norm);
  appInformation(reflectBuffer);
  appInformation(transmitBuffer);
}

//called after the simulation is over, actually outputs the calculated quantities.
//if you want to print something to the console after the simulation is done,
//maybe do it here! Or do whatever you want, I guess. Don't let me tell you
//what to do.
void Mevegs_Application::outputResults() {

    //save results to internal result vectors in Application class
    aggregateResults();

    // double norm = (static_cast<double>(current_case))/Etot;
    egsInformation("\n\n last case = %d Etot = %g\n",
    static_cast<int>(current_case),Etot);

    //NB -> we commented this stuff out to avoid printing info of > 100000 tets
    //for every run, would take a whole minute to do.

    // score->reportResults(norm,
    //         "Reflected/deposited/transmitted energy fraction",false,
    //         "  %d  %12.6e +/- %12.6e %c\n");
    // if( nph > 0 ) {
    //     if( nph > 1 ) egsInformation("\n\n Pulse height distributions\n"
    //             "==========================\n\n");
    //     else egsInformation("\n\n Pulse height distribution in region %d\n"
    //             "==========================================\n\n",
    //             ph_regions[0]);
    //     for(int j=0; j<nph; j++) {
    //         if( nph > 1 ) egsInformation("Region %d\n"
    //                 "----------------\n\n",ph_regions[j]);
    //         double f,df;
    //         for(int i=0; i<pheight[j]->bins(); i++) {
    //             pheight[j]->currentResult(i,f,df);
    //             egsInformation("%g   %g   %g\n",ph_de[j]*(0.5+i),
    //                     f/ph_de[j],df/ph_de[j]);
    //         }
    //     }
    // }
    // EGS_Float Rmax = 20; EGS_Float dr = Rmax/200;
}

//Reports the current result,
//used for reporting intermediate results during simulation.
//From tutor7pp
void Mevegs_Application::getCurrentResult(double &sum, double &sum2,
        double &norm, double &count) {
    count = current_case;
    norm = Etot > 0 ? count/Etot : 0;
    score->currentScore(0,sum,sum2);
}

//called just before the shower() function. If this function does not return 0,
//shower() will not get called.
//From tutor7pp
int Mevegs_Application::startNewShower() {
    Etot += p.E*p.wt;
    int res = EGS_Application::startNewShower();
    if( res ) return res;
    if( current_case != last_case ) {
        if( nph > 0 ) {
            for(int j=0; j<nph; j++) {
                 pheight[j]->setHistory(current_case);
                int ireg = ph_regions[j];
                EGS_Float edep = score->currentScore(ireg);
                if( edep > 0 ) {
                    int ibin = min( (int)(edep/(current_weight*ph_de[j])), pheight[j]->bins()-1 );
                    if( ibin >= 0 && ibin < pheight[j]->bins() )
                        pheight[j]->score(ibin,1);

                }
            }
        }
        score->setHistory(current_case);
        eflu->setHistory(current_case);
        gflu->setHistory(current_case);
        last_case = current_case;
    }
    current_weight = p.wt;
    return 0;
}

//bundles together dosemath calculations
//namedResults is alias for result type in dosemath
//natural place is of course in dosemath but need the Mevegs_Application
//ptr to app and don't want to make a header just to include in the dosemath file
dosemath::namedResults Mevegs_Application::calculateResults(const Mesh& mesh){

  dosemath::namedResults allRes;

  const double Ea = dosemath::getEA(getNParticles(), getETot());

  //NB dosePerC is actual dose, not this number
  std::vector<double> energyFrac, uncertRes;
  //get independent results from the simulation
  getResultVectors(energyFrac, uncertRes);

  //put independent results into result vector first
  allRes.emplace_back(std::make_pair("Energy Fraction", energyFrac));
  allRes.emplace_back(std::make_pair("Absolute uncertainty", uncertRes));

  //then find quantites used for other quantites up front
  std::vector<double> tetVols = dosemath::getTetVols(mesh.getCoords());
  std::vector<double> tetDensities = dosemath::getTetDensities(mesh.getRhor(), getDensities(),
                                                               getGeometry()->getMediaIndices(), getNReg());
  std::vector<double> tetMasses     = dosemath::getTetMasses(tetVols, tetDensities);

  //put results into namedResults vector
  //take coords right from the mesh
  allRes.emplace_back(std::make_pair("Tet Volumes [cm^3]", tetVols));
  allRes.emplace_back(std::make_pair("Tet Densities [g/cm^3]", tetDensities));

  allRes.emplace_back(std::make_pair("Energy Density per Coulomb [J/cm^3 C]",
                                      dosemath::getEDensities(energyFrac, Ea, tetVols)));

  allRes.emplace_back(std::make_pair("Dose per Coulomb [kGy/C]", dosemath::getDoses(energyFrac, Ea, tetMasses)));
  allRes.emplace_back(std::make_pair("Uncertainty Percentage [%]", dosemath::getUncertaintyPercentages(uncertRes, energyFrac)));

  return allRes;
}

//this is the main logic of the entire MevEGS system.
int main (int argc, char** argv){

  //flag for using the Mesh class internally to make the geometry
  bool mesh_flag = false;
  std::string meshFilePath = "";

  //if mesh is specified on cmd line, want to pass everything but to application cstor
  //NB WILL BREAK if more than one .msh is fed in

  int new_argc = argc;
  char** new_argv = argv;
  int dotMeshPos;

  /////////////////////
  //CHECK FOR MESH FILE
  //loop over cmd line args to see if .msh file is in there
  for(int i = 0; i < argc; i++){
    auto strArg = std::string(argv[i]);
    if((strArg.size() > 3) && (strArg.compare(strArg.size()-4, 4, ".msh")) == 0){
      //try and make Mesh object from input file
      //can return empty Mesh or full one, check using isEmpty()
      meshFilePath = strArg;
      // Mesh inputMesh = gmsh_manip::generateMeshObject(strArg);
      mesh_flag = true; //yes being called
      dotMeshPos = i;
    }
  }

  //if using mesh, loop over args and get rid of the one that was .msh filename
  // we flag it with dotMeshPos
  if (mesh_flag){
    int ni = 0;
    for(int i = 0; i < argc; i
      ++){
      if(i == dotMeshPos){
        new_argc--; // subtract from new_argc
      }
      //only increment if not the dotMeshPos
      else {
        new_argv[ni++] = argv[i];
      }
    }
  }

  // will be OK if strArg is bad
  Mesh mesh = gmsh_manip::createMesh(meshFilePath);
  mesh_flag = !mesh.isEmpty();

  ////////////////////////
  //RUN SIMULATION
  ///////////////////////

  //make an application
  Mevegs_Application app(new_argc, new_argv, mesh_flag);
  //make a movie from the application as well

  if(mesh_flag){
    app.setMeshPtr(&mesh);
  }

  //2/4: initialize simulation
    int initErr = app.initSimulation();
    if( initErr ) return initErr;

    if(mesh_flag){
      //set tet relative densities
      std::vector<double> rhor = mesh.getRhor();
      for(std::size_t i = 0; i < rhor.size(); i++) {
        //std::cout << "Index: " << i << " RhoR: " << rhor[i] << std::endl;
        app.getGeometry()->setRelativeRho(i, rhor[i]);
        //std::cout << app.getGeometry()->getRelativeRho(i) << std::endl;
    }
  }
  //3/4: run simulation if there weren't any initErrs
    int runErr = app.runSimulation();
    if( runErr < 0) return runErr;

  //4/4: finish simulation and return error value
    int finishErr = app.finishSimulation();

    //if not parallel run, save to output file
    //OR if it's the last job of a parallel run
    if (app.getNparallel() == 0 || app.isLastJob()){
      //calculate all results using the dosemath methods as a member function of the app
      dosemath::namedResults allRes = app.calculateResults(mesh);
      gmsh_manip::saveMeshOutput(mesh, allRes, app.getInputFileName(), app.getRunComments());
    }
    return finishErr;
  }
