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

#include <cstdlib>

using std::vector;
using std::string;

//All EGS++ user codes extend the EGS_AdvancedApplication class.
class APP_EXPORT Mevegs_Application : public EGS_AdvancedApplication {

 Mesh* pmesh;

 //result vectors built up by aggregateResults
 vector<double> allDoses, allUncerts;

    // data variables
    EGS_ScoringArray *score;    // scoring array with energies deposited
    int              nreg;      // number of regions in the geometry
    double           Etot;      // total energy that has entered the geometry
    int              rr_flag;   // used for RR and radiative splitting
    EGS_Float        current_weight; // the weight of the initial particle that
                                     // is currently being simulated
    static string revision;    // revision number

public:

    /*! Constructor
     The command line arguments are passed to the EGS_AdvancedApplication
     contructor, which determines the input file, the pegs file, if the
     simulation is a parallel run, etc.
    */
    Mevegs_Application(int argc, char **argv):
        EGS_AdvancedApplication(argc,argv),
        score(0), nreg(0), Etot(0), rr_flag(0),
        current_weight(1) {

        std::cout << "Successfully constructed MevEGS Application" << std::endl;
    };

    void setMeshPtr(Mesh* _pmesh){pmesh = _pmesh;};

    //returns whether this job is the last one of a parallel run using a batch script or similar
    bool isLastJob(){
      return EGS_AdvancedApplication::final_job;
    }

    /*! Destructor.
     Deallocate memory
     */
    ~Mevegs_Application() override {
        if( score ) delete score;
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

    //send out our result vectors into the world to possibly be put in a gmsh pos or mesh file
    void getResultVectors(vector<double>& _doses,
                          vector<double>& _uncerts) const {
        _doses   = allDoses;
        _uncerts = allUncerts;
    }

    const string getInputFileName() const {
      return EGS_Application::input_file;
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

    //MXO -> save results to internal result vectors
    inline void aggregateResults();
};

string Mevegs_Application::revision = "1.0";

extern "C" void F77_OBJ_(egs_scale_xcc,EGS_SCALE_XCC)(const int *,const EGS_Float *);
extern "C" void F77_OBJ_(egs_scale_bc,EGS_SCALE_BC)(const int *,const EGS_Float *);

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

    //try setting the WATCH
    //"this is for the watch"
    //F77_OBJ_(watch, WATCH)(-99, 1);

  the_egsvr->i_do_rr = 1; //Nigel's bugfix re breaking brems with ausgab

  //JBT: Code to read in some variance reduction parameters from egsinp.
  EGS_Input *vr = input->takeInputItem("variance reduction");
  if( vr ) {

    egsInformation("Variance reduction options\n==========================\n");

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
  }
  return 0;
}


//initializes the tet collection geometry for the simulation, either from a
//mesh class or from a tet file
int Mevegs_Application::initGeometry(){

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
  vector<double> _doses, _uncerts; //result vectors
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
        score->setHistory(current_case);
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
  using std::make_pair;

  dosemath::namedResults allRes;

  vector<double> energyFrac, uncertRes;
  getResultVectors(energyFrac, uncertRes);

  allRes.emplace_back(make_pair("Energy Fraction", energyFrac));
  allRes.emplace_back(make_pair("Absolute uncertainty", uncertRes));

  //then find quantites used for other quantites up front
  vector<double> tetVols = dosemath::getTetVols(mesh.getCoords());
  allRes.emplace_back(make_pair("Tet Volumes [cm^3]", tetVols));

  return allRes;
}

// MevEGS main function
int main(int argc, char** argv) {

    // check for mesh file
    string meshFilePath = "";
    for (int i = 0; i < argc; i++) {
      auto strArg = string(argv[i]);
      if ((strArg.size() > 3) &&
          (strArg.compare(strArg.size()-4, 4, ".msh")) == 0) {
              meshFilePath = strArg;
      }
    }

    if (meshFilePath == "") {
        std::cerr << "no msh file given, exiting\n";
        exit(1);
    }

    // make a mesh or die trying
    Mesh mesh = gmsh_manip::createMesh(meshFilePath);

    Mevegs_Application app(argc, argv);
    app.setMeshPtr(&mesh);

    int initErr = app.initSimulation();
    if (initErr)
        return initErr;

    // FIXME (do in constructor)
    // set mesh relative densities
    for (std::size_t i = 0; i < mesh.rhor.size(); i++) {
      app.getGeometry()->setRelativeRho(i, mesh.rhor[i]);
    }

    //3/4: run simulation if there weren't any initErrs
    int runErr = app.runSimulation();
    if (runErr < 0)
        return runErr;

    //4/4: finish simulation and return error value
    int finishErr = app.finishSimulation();

    // if serial run, or last job of a parallel run, save to output file
    if (app.getNparallel() == 0 || app.isLastJob()){
      dosemath::namedResults allRes = app.calculateResults(mesh);
      gmsh_manip::saveMeshOutput(mesh, allRes, app.getInputFileName());
    }

    return finishErr;
}
