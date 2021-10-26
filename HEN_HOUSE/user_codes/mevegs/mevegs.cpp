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
//! To get EGS_Mesh quantities
#include "egs_mesh.h"
//! To handle EGS_Mesh in an EGS_EnvelopeGeometry
#include "egs_envelope_geometry.h"

#include <algorithm>
#include <cstdlib>
#include <cassert>
#include <fstream>
using namespace std;

// anonymous namespace
namespace {
    // Whether the geometry is a plain mesh or inside an envelope
    enum class MevegsGeometry { Mesh, EnvelopedMesh };
} // anonymous namespace

class APP_EXPORT Mevegs_Application : public EGS_AdvancedApplication {

    EGS_ScoringArray *score;    // scoring array with energies deposited
    EGS_ScoringArray *eflu;     // scoring array for electron fluence at back of geometry
    EGS_ScoringArray *gflu;     // scoring array for photon fluence at back of geometry
    EGS_ScoringArray **pheight; // pulse height distributions.
    int              nreg;      // number of regions in the geometry
    int              nph;       // number of pulse height objects.
    double           Etot;      // total energy that has entered the geometry
    int              rr_flag;   // used for RR and radiative splitting
    EGS_Float        current_weight; // the weight of the initial particle that
    // is currently being simulated
    bool  deflect_brems;

    EGS_Float        *ph_de;    // bin widths if the pulse height distributions.
    int              *ph_regions; // region indeces of the ph-dsitributions
    static string revision;    // the CVS revision number

public:

    /*! Constructor
     The command line arguments are passed to the EGS_AdvancedApplication
     contructor, which determines the input file, the pegs file, if the
     simulation is a parallel run, etc.
    */
    Mevegs_Application(int argc, char **argv) :
        EGS_AdvancedApplication(argc,argv), score(0), eflu(0), gflu(0), pheight(0),
        nreg(0), nph(0), Etot(0), rr_flag(0), current_weight(1), deflect_brems(false) { };

    /*! Destructor.
     Deallocate memory
     */
    ~Mevegs_Application() {
        if (score) {
            delete score;
        }
        if (eflu) {
            delete eflu;
        }
        if (gflu) {
            delete gflu;
        }
        if (nph > 0) {
            for (int j=0; j<nph; j++) {
                delete pheight[j];
            }
            delete [] pheight;
            delete [] ph_regions;
            delete [] ph_de;
        }
    };

    /*! Describe the application.
     This function is called from within the initSimulation() function
     so that applications derived from EGS_AdvancedApplication can print a
     header at the beginning of the output file.
    */
    void describeUserCode() const;

    /*! Initialize scoring.
     This function is called from within initSimulation() after the
     geometry and the particle source have been initialized.
     In our case we simple construct a scoring array with nreg+2 regions
     to collect the deposited energy in the nreg geometry regions and
     the reflected and transmitted energies, and if the user has
     requested it, initialize scoring array objects for pulse height
     distributions in the requested regions.
    */
    int initScoring();

    /*! Accumulate quantities of interest at run time
     This function is called from within the electron and photon transport
     routines at 28 different occasions specified by iarg (see PIRS-701
     for details). Here we are only interested in energy deposition =>
     only in iarg<=4 ausgab calls and simply use the score method of
     the scoring array object to accumulate the deposited energy.
    */
    int ausgab(int iarg);

    /*! Output intermediate results to the .egsdat file.
     This function is called at the end of each batch. We must store
     the results in the file so that simulations can be restarted and results
     of parallel runs recombined.
     */
    int outputData();

    /*! Read results from a .egsdat file.
     This function is used to read simulation results in restarted
     calculations.
     */
    int readData();

    /*! Reset the variables used for accumulating results
     This function is called at the beginning of the combineResults()
     function, which is used to combine the results of parallel runs.
    */
    void resetCounter();

    /*! Add simulation results
     This function is called from within combineResults() in the loop
     over parallel jobs. data is a reference to the currently opened
     data stream (basically the j'th .egsdat file).
     */
    int addState(istream &data);

    /*! Output the results of a simulation. */
    void outputResults();

    /*! Write the results to mesh output files. */
    void writeMeshOutputFiles() const;

    /*! Write the results to Gmsh v4.1 ASCII msh file. */
    void writeGmsh(const EGS_Mesh& mesh, std::size_t offset) const;

    /*! Write the results to a VTK legacy ASCII file. */
    void writeVtk(const EGS_Mesh& mesh, std::size_t offset) const;

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

protected:

    /*! Start a new shower.
     This function is called from within the shower loop just before the
     actual simulation of the particle starts. The particle parameters are
     available via the protected base class variable p which is of type
     EGS_Particle (see egs_application.h).
     In our case we simply accumulate the total energy into Etot
     and, if the current history is different from the last, we call
     the startHistory() method of our scoring object to make known
     to the scoring object that a new history has started (needed for
     the history-by-history statistical analysis).
     If 1 or more pulse heoght distributions are being calculated (nph > 0),
     we get the energy deposited in each pulse height region from the
     energy scoring object and add a pulse to the pulse height scoring
     object of the region in the appropriate bin.
    */
    int startNewShower();


};

string Mevegs_Application::revision = "0.1";

extern "C" void F77_OBJ_(egs_scale_xcc,EGS_SCALE_XCC)(const int *,const EGS_Float *);
extern "C" void F77_OBJ_(egs_scale_bc,EGS_SCALE_BC)(const int *,const EGS_Float *);

void Mevegs_Application::describeUserCode() const {
    egsInformation(
        "\n               ***************************************************"
        "\n               *                                                 *"
        "\n               *                  Mevegs                         *"
        "\n               *                                                 *"
        "\n               ***************************************************"
        "\n\n");
    egsInformation("This is Mevegs_Application %s based on\n"
                   "      EGS_AdvancedApplication %s\n\n",
                   egsSimplifyCVSKey(revision).c_str(),
                   egsSimplifyCVSKey(base_revision).c_str());
}

int Mevegs_Application::initScoring() {
    // Get the numner of regions in the geometry.
    nreg = geometry->regions();
    score = new EGS_ScoringArray(nreg+2);
    //i.e. we always score energy fractions
    eflu = new EGS_ScoringArray(200);
    gflu = new EGS_ScoringArray(200);

    // Initialize with no russian roulette
    the_egsvr->i_do_rr = 1;

    EGS_Input *options = input->takeInputItem("scoring options");
    if (options) {

        EGS_Input *scale;
        while ((scale = options->takeInputItem("scale xcc"))) {
            vector<string> tmp;
            int err = scale->getInput("scale xcc",tmp);
            //egsInformation("Found 'scale xcc', err=%d tmp.size()=%d\n",err,tmp.size());
            if (!err && tmp.size() == 2) {
                int imed = EGS_BaseGeometry::getMediumIndex(tmp[0]) + 1;
                if (imed > 0) {
                    EGS_Float fac = atof(tmp[1].c_str());
                    egsInformation("\n ***** Scaling xcc of medium %d with %g\n",imed,fac);
                    F77_OBJ_(egs_scale_xcc,EGS_SCALE_XCC)(&imed,&fac);
                }
            }
            delete scale;
        }
        while ((scale = options->takeInputItem("scale bc"))) {
            vector<string> tmp;
            int err = scale->getInput("scale bc",tmp);
            //egsInformation("Found 'scale xcc', err=%d tmp.size()=%d\n",err,tmp.size());
            if (!err && tmp.size() == 2) {
                int imed = EGS_BaseGeometry::getMediumIndex(tmp[0]) + 1;
                if (imed > 0) {
                    EGS_Float fac = atof(tmp[1].c_str());
                    egsInformation("\n ***** Scaling bc of medium %d with %g\n",imed,fac);
                    F77_OBJ_(egs_scale_bc,EGS_SCALE_BC)(&imed,&fac);
                }
            }
            delete scale;
        }

        vector<string> choices;
        choices.push_back("no");
        choices.push_back("yes");
        deflect_brems = options->getInput("deflect electron after brems",choices,0);
        if (deflect_brems) {
            egsInformation("\n *** Using electron deflection in brems events\n\n");
            setAusgabCall(AfterBrems,true);
        }

        int n_rr;
        if (!options->getInput("Russian Roulette",n_rr) && n_rr > 1) {
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
        if (!err && !err1) {
            if (regions.size() != nbins.size() && nbins.size() != 1)
                egsWarning("initScoring(): you must input the same "
                           "number of 'regions' and 'bins' inputs or a single 'bins'"
                           " input\n");
            else {
                EGS_ScoringArray **tmp = new EGS_ScoringArray* [nreg+2];
                for (int i=0; i<nreg+2; i++) {
                    tmp[i] = 0;
                }
                for (int j=0; j<regions.size(); j++) {
                    int nb = nbins.size() == 1 ? nbins[0] : nbins[j];
                    if (nb < 1) {
                        egsWarning("zero bins for region %d?\n",regions[j]);
                    }
                    if (regions[j] < -1 || regions[j] > nreg) {
                        egsWarning("invalid region index %d\n",regions[j]);
                    }
                    if (nb > 0 && regions[j] >= 0 && regions[j] < nreg+2) {
                        int ij = regions[j];
                        if (tmp[ij]) egsInformation("There is already a "
                                                        "PHD object in region %d => ignoring it\n",ij);
                        else {
                            tmp[ij] = new EGS_ScoringArray(nb);
                            ++nph;
                        }
                    }
                }
                if (nph > 0) {
                    pheight = new EGS_ScoringArray* [nph];
                    ph_regions = new int [nph];
                    ph_de = new EGS_Float [nph];
                    EGS_Float Emax = source->getEmax();
                    int iph = 0;
                    for (int j=0; j<nreg+2; j++) {
                        if (tmp[j]) {
                            pheight[iph] = tmp[j];
                            ph_regions[iph] = j;
                            int nbin = pheight[iph]->bins();
                            ph_de[iph++] = Emax/nbin;
                        }
                    }
                }
                delete [] tmp;
            }
        }
        else egsWarning("initScoring(): you must provide both, 'regions'"
                            " and 'bins' input\n");
        delete options;
    }
    return 0;
}

int Mevegs_Application::ausgab(int iarg) {
    if (iarg <= 4) {
        int np = the_stack->np - 1;

        // Note: ir is the region number+1
        int ir = the_stack->ir[np]-1;

        // If the particle is outside the geometry and headed in the positive
        // z-direction, change the region to count it as "transmitted"
        // Note: This is only valid for certain source/geometry conditions!
        // If those conditions are not met, the reflected and transmitted
        // energy fractions will be wrong
        if (ir == 0 && the_stack->w[np] > 0) {
            ir = nreg+1;
        }

        EGS_Float aux = the_epcont->edep*the_stack->wt[np];
        if (aux > 0) {
            score->score(ir,aux);
        }

        // if( the_stack->iq[np] ) score->score(ir,the_epcont->edep*the_stack->wt[np]);
        if (ir == nreg+1) {
            EGS_ScoringArray *flu = the_stack->iq[np] ? eflu : gflu;
            EGS_Float r2 = the_stack->x[np]*the_stack->x[np] + the_stack->y[np]*the_stack->y[np];
            int bin = (int)(sqrt(r2)*10.);
            if (bin < 200) {

                aux = the_stack->wt[np]/the_stack->w[np];
                if (aux > 0) {
                    flu->score(bin,aux);
                }
            }
        }
        return 0;
    }
    int np = the_stack->np-1;
    if (iarg == BeforeBrems || iarg == BeforeAnnihRest || (iarg == BeforeAnnihFlight &&
            the_stack->latch[np] > 0)) {
        the_stack->latch[np] = 0;
        rr_flag = 1;
        the_egsvr->nbr_split = the_egsvr->i_do_rr;
        return 0;
    }
    if (iarg == AfterBrems && deflect_brems) {
        EGS_Vector u(the_stack->u[np-1],the_stack->v[np-1],the_stack->w[np-1]);
        EGS_Float tau = the_stack->E[np-1]/the_useful->rm - 1;
        EGS_Float beta = sqrt(tau*(tau+2))/(tau+1);
        EGS_Float eta = 2*rndm->getUniform()-1;
        EGS_Float cost = (beta + eta)/(1 + beta*eta);
        EGS_Float sint = 1 - cost*cost;
        if (sint > 0) {
            sint = sqrt(sint);
            EGS_Float cphi, sphi;
            rndm->getAzimuth(cphi,sphi);
            u.rotate(cost,sint,cphi,sphi);
            the_stack->u[np-1] = u.x;
            the_stack->v[np-1] = u.y;
            the_stack->w[np-1] = u.z;
        }
    }

    if (iarg == AfterBrems || iarg == AfterAnnihRest || iarg == AfterAnnihFlight) {
        the_egsvr->nbr_split = 1;
        if (iarg == AfterBrems && rr_flag) {
            the_stack->latch[the_stack->npold-1] = 1;
        }
        rr_flag = 0;
        return 0;
    }
    /*
    if( iarg == FluorescentEvent && the_stack->latch[np] > 0 ) {
        the_stack->latch[np] = 0; the_stack->wt[np] /= the_egsvr->i_do_rr;
        if( np+1+the_egsvr->i_do_rr > MXSTACK )
            egsFatal("Stack size exceeded while splitting dluorescent photon!\n");
        for(int j=1; j<the_egsvr->i_do_rr; j++) {
            EGS_Float cost = 2*rndm->getUniform()-1;
            EGS_Float sint = 1 - cost*cost; sint = sint > 0 ? sqrt(sint) : 0;
            EGS_Float cphi, sphi; rndm->getAzimuth(cphi,sphi);
            the_stack->E[np+j] = the_stack->E[np];
            the_stack->wt[np+j] = the_stack->wt[np];
            the_stack->iq[np+j] = the_stack->iq[np];
            the_stack->ir[np+j] = the_stack->ir[np];
            the_stack->dnear[np+j] = the_stack->dnear[np];
            the_stack->latch[np+j] = the_stack->latch[np];
            the_stack->x[np+j] = the_stack->x[np];
            the_stack->y[np+j] = the_stack->y[np];
            the_stack->z[np+j] = the_stack->z[np];
            the_stack->u[np+j] = sint*cphi;
            the_stack->v[np+j] = sint*sphi;
            the_stack->w[np+j] = cost;
        }
    }
    */


    return 0;
}

int Mevegs_Application::outputData() {
    // We first call the outputData() function of our base class.
    // This takes care of saving data related to the source, the random
    // number generator, CPU time used, number of histories, etc.
    int err = EGS_AdvancedApplication::outputData();
    if (err) {
        return err;
    }
    // We then write our own data to the data stream. data_out is
    // a pointer to a data stream that has been opened for writing
    // in the base class.
    (*data_out) << "  " << Etot << endl;
    if (!score->storeState(*data_out)) {
        return 101;
    }
    for (int j=0; j<nph; j++) {
        if (!pheight[j]->storeState(*data_out)) {
            return 102+j;
        }
    }
    if (!eflu->storeState(*data_out)) {
        return 301;
    }
    if (!gflu->storeState(*data_out)) {
        return 302;
    }
    return 0;
}

int Mevegs_Application::readData() {
    // We first call the readData() function of our base class.
    // This takes care of reading data related to the source, the random
    // number generator, CPU time used, number of histories, etc.
    // (everything that was stored by the base class outputData() method).
    int err = EGS_AdvancedApplication::readData();
    if (err) {
        return err;
    }
    // We then read our own data from the data stream.
    // data_in is a pointer to an input stream that has been opened
    // by the base class.
    (*data_in) >> Etot;
    if (!score->setState(*data_in)) {
        return 101;
    }
    for (int j=0; j<nph; j++) {
        if (!pheight[j]->setState(*data_in)) {
            return 102+j;
        }
    }
    if (!eflu->setState(*data_in)) {
        return 301;
    }
    if (!gflu->setState(*data_in)) {
        return 302;
    }
    return 0;
}

void Mevegs_Application::resetCounter() {
    // Reset everything in the base class
    EGS_AdvancedApplication::resetCounter();
    // Reset our own data to zero.
    score->reset();
    Etot = 0;
    for (int j=0; j<nph; j++) {
        pheight[j]->reset();
    }
    eflu->reset();
    gflu->reset();
}

int Mevegs_Application::addState(istream &data) {
    // Call first the base class addState() function to read and add
    // all data related to source, RNG, CPU time, etc.
    int err = EGS_AdvancedApplication::addState(data);
    if (err) {
        return err;
    }
    // Then read our own data to temporary variables and add to
    // our results.
    double etot_tmp;
    data >> etot_tmp;
    Etot += etot_tmp;
    EGS_ScoringArray tmp(nreg+2);
    if (!tmp.setState(data)) {
        return 101;
    }
    (*score) += tmp;
    for (int j=0; j<nph; j++) {
        EGS_ScoringArray tmpj(pheight[j]->bins());
        if (!tmpj.setState(data)) {
            return 102 + j;
        }
        (*pheight[j]) += tmpj;
    }
    EGS_ScoringArray tmp1(200);
    if (!tmp1.setState(data)) {
        return 301;
    }
    (*eflu) += tmp1;
    if (!tmp1.setState(data)) {
        return 302;
    }
    (*gflu) += tmp1;
    return 0;
}

void Mevegs_Application::outputResults() {
    egsInformation("\n\n last case = %d Etot = %g\n",
                   (int)current_case,Etot);
    writeMeshOutputFiles();
    if (nreg > 100) {
        return;
    }
    double norm = ((double)current_case)/Etot;

    egsInformation("\n\n======================================================\n");
    egsInformation(" Energy fractions\n");
    egsInformation("======================================================\n");
    egsInformation("The first and last items in the following list of energy fractions are the reflected and transmitted energy, respectively. These two values are only meaningful if the source is directed in the positive z-direction. The remaining values are the deposited energy fractions in the regions of the geometry, but notice that the identifying index is the region number offset by 1 (ir+1).");
    score->reportResults(norm,
                         "ir+1 | Reflected, deposited, or transmitted energy fraction",false,
                         "  %d  %12.6e +/- %12.6e %c\n");
    if (nph > 0) {
        if (nph > 1) {
            egsInformation("\n\n======================================================\n");
            egsInformation(" Pulse height distributions\n"
                           "======================================================\n\n");
        }
        else {
            egsInformation("\n\n Pulse height distribution in region %d\n"
                           "======================================================\n\n",
                           ph_regions[0]);
        }
        for (int j=0; j<nph; j++) {
            if (nph > 1) egsInformation("\nRegion %d\n"
                                            "----------------\n\n",ph_regions[j]);
            double f,df;
            for (int i=0; i<pheight[j]->bins(); i++) {
                pheight[j]->currentResult(i,f,df);
                egsInformation("%g   %g   %g\n",ph_de[j]*(0.5+i),
                               f/ph_de[j],df/ph_de[j]);
            }
        }
    }
}

EGS_Mesh * extractEGSMesh(EGS_EnvelopeGeometry *env) {
    if (!env) {
        return nullptr;
    }
    std::size_t nInscribed = 0;
    EGS_BaseGeometry **geometries = env->getInscribedGeometries(nInscribed);
    if (!geometries) {
        return nullptr;
    }
    // don't allow more than one inscribed geometry to simplify score array
    // indexing
    if (nInscribed != 1) {
        egsWarning("\nfound more than one inscribed geometry\n");
        return nullptr;
    }
    EGS_Mesh *mesh = dynamic_cast<EGS_Mesh*>(geometries[0]);
    if (!mesh) {
        return nullptr;
    }
    return mesh;
}

void Mevegs_Application::writeVtk(const EGS_Mesh& mesh,
    std::size_t score_offset) const
{
    if (EGS_Application::getIparallel()) {
        egsInformation("\n Mevegs_Application: This is one of a number of parallel jobs. Will only output VTK file on combining results.\n");
        return;
    }

    std::string vtk_out = getFinalOutputFile() + ".vtk";
    std::ofstream out(vtk_out);
    if (!out) {
        egsWarning("\n couldn't open \"%s\" for writing\n", vtk_out.c_str());
        return;
    }
    out << std::setprecision(std::numeric_limits<double>::max_digits10);
    // legacy header
    out << "# vtk DataFile Version 4.1\n"
           "EGS_Mesh results\n"
           "ASCII\n"
           "DATASET UNSTRUCTURED_GRID\n"
           "POINTS " << mesh.num_nodes() << " double\n";
    // point data
    for (int i = 0; i < mesh.num_nodes(); i++) {
        const EGS_Vector& node = mesh.node_coordinates(i);
        out << node.x << " " << node.y << " " << node.z << "\n";
    }
    // 5 numbers per line
    out << "CELLS " << mesh.num_elements() << " "
        << 5 * mesh.num_elements() << "\n";
    // unstructured grid
    for (int i = 0; i < mesh.num_elements(); i++) {
        const auto& node_offsets = mesh.element_node_offsets(i);
        // four nodes per tetrahedron
        out << "4 " << node_offsets[0] << " " << node_offsets[1] << " " <<
            node_offsets[2] << " " << node_offsets[3] << "\n";
    }
    out << "CELL_TYPES " << mesh.num_elements() << "\n";
    for (int i = 0; i < mesh.num_elements(); i++) {
        // vtk code for tetrahedron
        out << "10\n";
    }
    // doses
    out << "CELL_DATA " << mesh.num_elements() << "\n";
    // adjust number here vvvv if the number of fields written out changes
    out << "FIELD FieldData 2\n";
    // %20 url-encoded space, Paraview errors on space character
    out << "dose%20[Gy] 1 " << mesh.num_elements() << " double\n";
    const double JOULES_PER_MEV = 1.602e-13;
    for (int i = 0; i < mesh.num_elements(); i++) {
        double e_dep, uncert;
        score->currentResult(i + score_offset, e_dep, uncert);
        const auto mass_kg = mesh.element_density(i) * mesh.element_volume(i)
            / 1000.0;
        // TODO zero out doses with uncertainty over 50%?
        out << JOULES_PER_MEV * e_dep / mass_kg << "\n";
    }
    // uncertainties
    out << "uncertainty%20[%25] 1 " << mesh.num_elements() << " double\n";
    for (int i = 0; i < mesh.num_elements(); i++) {
        double e_dep, uncert;
        score->currentResult(i + score_offset, e_dep, uncert);
        // if edep is exactly zero, there is 100% uncertainty
        if (e_dep == 0.0) {
            out << 100.0 << "\n";
        } else {
            out << uncert / e_dep * 100.0 << "\n";
        }
    }
}

void Mevegs_Application::writeMeshOutputFiles() const {
    MevegsGeometry geo_type = MevegsGeometry::Mesh;
    EGS_Mesh *mesh = dynamic_cast<EGS_Mesh*>(geometry);
    if (!mesh) {
        EGS_Mesh *inscribed_mesh = extractEGSMesh(dynamic_cast<EGS_EnvelopeGeometry*>(geometry));
        if (!inscribed_mesh) {
            egsWarning("\n No mesh geometry found, skipping mesh output step\n");
            return;
        }
        mesh = inscribed_mesh;
        geo_type = MevegsGeometry::EnvelopedMesh;
    }

    // offset into score array
    std::size_t score_offset = 0;
    switch (geo_type) {
        // if it's a plain mesh being simulated, skip the first element (reflected energy)
        case MevegsGeometry::Mesh:
            assert(score->regions() == mesh->num_elements() + 2);
            score_offset = 1;
            break;
        // if it's a mesh in an envelope being simulated, skip two elements:
        // reflected energy and the envelope
        case MevegsGeometry::EnvelopedMesh:
            assert(score->regions() == mesh->num_elements() + 3);
            score_offset = 2;
            break;
        default:
            egsFatal("\nunhandled MevegsGeometry case\n");
    }

    //writeGmsh(*mesh, score_offset);
    writeVtk(*mesh, score_offset);
}

void Mevegs_Application::writeGmsh(const EGS_Mesh& mesh,
    std::size_t score_offset) const
{
    // make a copy of the input mesh file and append the results to it
    auto filename = mesh.filename();
    if (filename.empty()) {
        egsWarning("\n EGS_Mesh has no filename, skipping mesh output step\n");
        return;
    }
    std::string output_msh = EGS_Application::getFinalOutputFile() + "+" + filename;
    std::ofstream out(output_msh);
    if (!out) {
        egsWarning("\n couldn't open \"%s\" for writing EGS_Mesh results\n",
                   output_msh.c_str());
        return;
    }
    {
        std::ifstream in(filename);
        if (!in) {
            egsWarning("\n couldn't open EGS_Mesh input file \"%s\", skipping mesh output step\n",
                       filename.c_str());
            return;
        }
        out << in.rdbuf();
    }

    auto n_elts = mesh.num_elements();

    // write results to msh file

    // Energy deposition

    // Gmsh's ElementData section is the same for msh versions 2.2 and 4.1
    //
    // header
    //out << "$ElementData\n";
    //// one string, the view title
    //out << "1\n" << "\"Energy deposition per particle [MeV]\"\n";
    //// one float, the time (dummy 0.0)
    //out << "1\n0.0\n";
    //// three ints, timestep 0, 1 value per elt, number of elts
    //out << "3\n0\n1\n" << n_elts << "\n";
    //for (int i = 0; i < n_elts; i++) {
    //    double e_dep, uncert;
    //    score->currentResult(i + score_offset, e_dep, uncert);
    //    out << mesh.element_tag(i) << " " << e_dep << "\n";
    //}
    //// footer
    //out << "$EndElementData\n";

    //auto abs_to_percent = [](EGS_Float val, EGS_Float uncert) -> EGS_Float {
    //    if (val > 1e-6) {
    //        return uncert / val * 100.0;
    //    }
    //    return 100.0;
    //};

    //// Percent uncertainty
    //out << "$ElementData\n";
    //out << "1\n" << "\"Energy uncertainty [%]\"\n";
    //out << "1\n0.0\n";
    //out << "3\n0\n1\n" << n_elts << "\n";
    //for (int i = 0; i < n_elts; i++) {
    //    double e_dep, uncert;
    //    score->currentResult(i + score_offset, e_dep, uncert);
    //    out << mesh.element_tag(i) << " " << abs_to_percent(e_dep, uncert)
    //        << "\n";
    //}
    //out << "$EndElementData\n";

    //// Volumes
    //out << "$ElementData\n";
    //out << "1\n" << "\"Volume [cm^3]\"\n";
    //out << "1\n0.0\n";
    //out << "3\n0\n1\n" << n_elts << "\n";
    //for (int i = 0; i < n_elts; i++) {
    //    out << mesh.element_tag(i) << " " << mesh.element_volume(i) << "\n";
    //}
    //out << "$EndElementData\n";

    //// Densities
    //out << "$ElementData\n";
    //out << "1\n" << "\"Density [g/cm^3]\"\n";
    //out << "1\n0.0\n";
    //out << "3\n0\n1\n" << n_elts << "\n";
    //for (int i = 0; i < n_elts; i++) {
    //    out << mesh.element_tag(i) << " " << mesh.element_density(i) << "\n";
    //}
    //out << "$EndElementData\n";

    // Doses

    const double JOULES_PER_MEV = 1.602e-13;
    out << "$ElementData\n";
    out << "1\n" << "\"Dose [Gy]\"\n";
    out << "1\n0.0\n";
    out << "3\n0\n1\n" << n_elts << "\n";
    for (int i = 0; i < n_elts; i++) {
        double e_dep, uncert;
        score->currentResult(i + score_offset, e_dep, uncert);
        const auto mass_kg = mesh.element_density(i) * mesh.element_volume(i)
            / 1000.0;
        out << mesh.element_tag(i) << " " <<  JOULES_PER_MEV * e_dep / mass_kg
            << "\n";
    }
    out << "$EndElementData\n";
}

void Mevegs_Application::getCurrentResult(double &sum, double &sum2,
        double &norm, double &count) {
    count = current_case;
    norm = Etot > 0 ? count/Etot : 0;
    score->currentScore(0,sum,sum2);
}

int Mevegs_Application::startNewShower() {
    Etot += p.E*p.wt;
    int res = EGS_Application::startNewShower();
    if (res) {
        return res;
    }
    if (current_case != last_case) {
        if (nph > 0) {
            for (int j=0; j<nph; j++) {
                pheight[j]->setHistory(current_case);
                int ireg = ph_regions[j];

                // In ausgab the scoring array is offset by 1 to include
                // the reflected and transmitted as the first and last regions
                EGS_Float edep = score->currentScore(ireg+1);

                if (edep > 0) {
                    int ibin = min((int)(edep/(current_weight*ph_de[j])), pheight[j]->bins()-1);
                    if (ibin >= 0 && ibin < pheight[j]->bins()) {
                        pheight[j]->score(ibin,1);
                    }

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

#ifdef BUILD_APP_LIB
APP_LIB(Mevegs_Application);
#else
APP_MAIN(Mevegs_Application);
#endif
