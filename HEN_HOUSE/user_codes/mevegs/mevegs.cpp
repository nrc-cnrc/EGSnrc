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
#include <sstream>
#include <string>
#include <vector>

#include "egs_advanced_application.h"
#include "egs_scoring.h"
#include "egs_interface2.h"
#include "egs_functions.h"
#include "egs_input.h"

#include "egs_mesh.h"
#include "gmsh_manip.h"

class APP_EXPORT Mevegs_Application : public EGS_AdvancedApplication {

 const TetrahedralMesh &mesh;

 // result vectors
 std::vector<double> e_deps, uncerts;
 // info blob to log
 std::string run_info;

 // scoring array with deposited energies
 EGS_ScoringArray *score = nullptr;
 // number of mesh elements
 int nreg = 0;
 // total energy
 double Etot = 0.0;
 // used for russian roulette and radiative splitting
 int rr_flag = 0;
 EGS_Float current_weight = 1.0;
 static std::string revision;

public:

    Mevegs_Application(int argc, char **argv, const TetrahedralMesh &_mesh):
        EGS_AdvancedApplication(argc,argv),
        mesh(_mesh) {
        std::cout << "Successfully constructed MevEGS Application" << std::endl;
    };

    ~Mevegs_Application() override {
        if( score ) delete score;
    };

    // Number of mesh elements
    int num_elements() const {
        return nreg;
    }

    // Average energy per source particle in MeV
    EGS_Float average_energy() const {
        return this->Etot /  static_cast<EGS_Float>(this->current_case);
    }

    // EGS_AdvancedApplication overrides
    void describeUserCode() const override;
    int initScoring() override;
    int initGeometry() override;
    int ausgab(int iarg) override;
    int outputData() override;
    int readData() override;
    void resetCounter() override;
    int addState(istream &data) override;
    void outputResults() override;
    void getCurrentResult(double &sum, double &sum2, double &norm,
            double &count);

    EGS_Mesh* getGeometry() const {
       return dynamic_cast<EGS_Mesh*>(geometry);
    }

protected: // called by the base class
    int startNewShower();

private:

    // save results to internal result vectors
    void aggregateResults();

    // write simulation results to a Gmsh msh file.
    void write_gmsh();
    void append_gmsh_data(std::ostream& out_file, std::string title,
            const std::vector<double>& data);
};

string Mevegs_Application::revision = "1.1";

void Mevegs_Application::describeUserCode() const {
   egsInformation("\nMEVEGS\n");
   egsInformation("This is MEVEGS %s based on\n"
     "      Tutor7ppApplication %s\n\n",
     egsSimplifyCVSKey(revision).c_str(),
     egsSimplifyCVSKey(base_revision).c_str());
}

// Based on tutor7pp.
int Mevegs_Application::initScoring() {

  // Get the number of regions in the geometry.
  this->nreg = geometry->regions();
  this->score = new EGS_ScoringArray(nreg+2);
  the_egsvr->i_do_rr = 1;

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
       egsInformation("\n => initScoring: russian roulette with survival probability 1/%d\n\n",n_rr);
     }
   delete vr;
  }
  return 0;
}

// initializes the mesh geometry for the simulation
int Mevegs_Application::initGeometry(){

    EGS_BaseGeometry::setActiveGeometryList(app_index);

    //Read in scaling factor
    double default_scaling = 1.0;
    double scaling = 1.0;
    EGS_Input *mevegs = input->takeInputItem("mevegs");
    if( mevegs ) {
      if(!mevegs->getInput("scaling", scaling) && scaling != 0) {
        egsInformation("\n => initScoring: scaling by a factor of %lf", scaling);
      }
      delete mevegs;
    }

    if(scaling != scaling) {
      egsInformation("\nError scaling, got NaN! Using default of %lf.", default_scaling);
      scaling = default_scaling;
    }

    geometry = createMeshGeometry(input, this, scaling, this->mesh);

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

// output scoring quantities to an ausgab object
// from tutor7pp.
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

// from tutor7pp.
int Mevegs_Application::outputData() {
    int err = EGS_AdvancedApplication::outputData();
    if( err ) return err;
    (*data_out) << " " << Etot << endl;
    if( !score->storeState(*data_out) ) return 101;
    return 0;
}

// from tutor7pp.
int Mevegs_Application::readData() {
    int err = EGS_AdvancedApplication::readData();
    if( err ) return err;
    (*data_in) >> Etot;
    if( !score->setState(*data_in) ) return 101;
    return 0;
}

// Resets all aspects of the simulation.
// Used right before combining results from parallel simulations.
// from tutor7pp.
void Mevegs_Application::resetCounter() {
    EGS_AdvancedApplication::resetCounter();
    score->reset(); Etot = 0;
}

int Mevegs_Application::addState(istream &data) {
    int err = EGS_AdvancedApplication::addState(data);
    if( err ) return err;
    double etot_tmp; data >> etot_tmp; Etot += etot_tmp;
    EGS_ScoringArray tmp(nreg+2);
    if( !tmp.setState(data) ) return 101;
    (*score) += tmp;
    return 0;
}

void Mevegs_Application::aggregateResults() {

  egsInformation("aggregating results for job %d\n\n", getIparallel());

  // call score by a more descriptive name
  auto edep_score = this->score;

  // the score array is size nreg + 2, first elt is reflected, last is transmitted
  // start at 1 to skip energy reflected, stop 1 before end to skip transmitted
  this->e_deps.reserve(num_elements());
  this->uncerts.reserve(num_elements());

  // convert absolute uncertainty to percent uncertainty
  auto abs_to_percent = [](EGS_Float val, EGS_Float uncert) -> EGS_Float {
        if (val > 0) {
            return uncert / val * 100.0;
        }
        return 100.0;
  };

  for (int i = 1; i < num_elements() + 1; ++i){
    double e_dep, uncert;
    // don't normalize the results, we can calculate fractions, etc. after the simulation
    edep_score->currentResult(i, e_dep, uncert);
    e_deps.push_back(e_dep);
    uncerts.push_back(abs_to_percent(e_dep, uncert));
  }

    // report reflected and transmitted quantities
  std::ostringstream oss;
  oss << "****Results given for a 66% confidence interval****\n";

  EGS_Float dep;
  EGS_Float uncert;

  // 0th region is reflected energy
  edep_score->currentResult(0, dep, uncert);

  // normalize to the source's average particle energy
  auto norm = 1.0 / average_energy();

  oss << "Reflected (-Z) fraction: " << dep * norm << " +/- " << abs_to_percent(dep * norm, uncert * norm) << "%\n";
  oss << "Reflected (-Z) energy per particle [MeV]: " << dep << " +/- " << abs_to_percent(dep, uncert) << "%\n";

  // one past last region is transmitted energy
  edep_score->currentResult(num_elements() + 1, dep, uncert);
  oss << "Transmitted (+Z) fraction: " << dep * norm << " +/- " << abs_to_percent(dep * norm, uncert * norm) << "%\n";
  oss << "Transmitted (+Z) energy per particle [MeV]: " << dep << " +/- " << abs_to_percent(dep, uncert) << "%\n";

  appInformation(oss.str().c_str());
  this->run_info += oss.str();
}

void Mevegs_Application::outputResults() {
    aggregateResults();

    egsInformation("\n\n last case = %d Etot = %g\n",
        static_cast<int>(current_case),Etot);

    // save the results as a mesh data file
    write_gmsh();
}

// Save simulation results to a Gmsh msh file.
//
// Should work for either msh2 or msh4 because the ElementData section format
// is the same for both.
//
// FIXME: fstream error checking
void Mevegs_Application::write_gmsh() {

    // calculate energy fractions
    std::vector<EGS_Float> fractions;
    fractions.reserve(this->e_deps.size());

    for (auto e: e_deps) {
        fractions.push_back(e / average_energy());
    }

    auto result_file = std::ofstream(this->mesh.output_filename(), std::ios::app);

    append_gmsh_data(result_file, "Energy fraction per particle", fractions);
    append_gmsh_data(result_file, "Energy deposition per particle [MeV]", this->e_deps);
    append_gmsh_data(result_file, "Energy uncertainty [%]", this->uncerts);
    append_gmsh_data(result_file, "Volume [cm^3]", this->getGeometry()->element_volumes());
    append_gmsh_data(result_file, "Density [g/cm^3]", this->getGeometry()->element_densities());

    auto extra_info = std::ofstream(this->mesh.output_filename() + ".egsinfo", std::ios::app);
    // extra run information
    extra_info << "$EGSInfo\n" << this->run_info << "$EndEGSInfo\n";
}

// append data to a mesh (same ElementData format for msh v2 and v4.1)
void Mevegs_Application::append_gmsh_data(std::ostream& out_file, std::string title, const std::vector<double>& data)
{
    auto elt_tags = this->mesh.element_tags();

    out_file << "$ElementData\n"; // header
    out_file << "1\n" << "\"" << title << "\"\n"; // one string, the view title
    out_file << "1\n0.0\n"; // one real number, the time (0.0)

    // three ints, timestep 0, 1 value per elt, number of elts
    out_file << "3\n0\n1\n" << data.size() << "\n";

    for (std::size_t i = 0; i < data.size(); i++) {
        out_file << elt_tags[i] << " " << data[i] << "\n";
    }
    out_file << "$EndElementData\n"; // footer
}

// from tutor7pp
void Mevegs_Application::getCurrentResult(double &sum, double &sum2,
        double &norm, double &count) {
    count = current_case;
    norm = Etot > 0 ? count/Etot : 0;
    score->currentScore(0,sum,sum2);
}

// from tutor7pp
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

int main(int argc, char** argv) {

    // check for mesh file
    std::string meshFilePath = "";
    for (int i = 0; i < argc; i++) {
      auto strArg = std::string(argv[i]);
      if ((strArg.size() > 3) &&
          (strArg.compare(strArg.size()-4, 4, ".msh")) == 0) {
              meshFilePath = strArg;
      }
    }

    if (meshFilePath == "") {
        std::cerr << "mevegs: no msh file given, exiting\n";
        exit(1);
    }

    // make a mesh or die trying
    auto mesh = gmsh_manip::mesh_from_gmsh(meshFilePath);
    Mevegs_Application app(argc, argv, mesh);

    int initErr = app.initSimulation();
    if (initErr)
        return initErr;

    int runErr = app.runSimulation();
    if (runErr < 0)
        return runErr;

    return app.finishSimulation();
}
