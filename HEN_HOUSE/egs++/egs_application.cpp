/*
###############################################################################
#
#  EGSnrc egs++ application
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
#  Author:          Iwan Kawrakow, 2005
#
#  Contributors:    Frederic Tessier
#                   Ernesto Mainegra-Hing
#                   Blake Walters
#
###############################################################################
*/


/*! \file egs_application.cpp
 *  \brief EGS_Application implementation
 *  \IK
 */

#include "egs_application.h"
#include "egs_functions.h"
#include "egs_input.h"
#include "egs_base_source.h"
#include "egs_rndm.h"
#include "egs_run_control.h"
#include "egs_base_source.h"
#include "egs_simple_container.h"
#include "egs_ausgab_object.h"

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

#ifdef WIN32
    const char fs = 92;
    #define F_OK 0
    #define W_OK 2
    #define R_OK 4
    #include <io.h>
    #define EGS_ACCESS ::_access
#else
    const char fs = '/';
    #include <unistd.h>
    #define EGS_ACCESS ::access
    #include <sys/statvfs.h>
#endif

static char __egs_app_msg1[] = "EGS_Application::EGS_Application(int,char**):";
static char __egs_app_msg2[] = "EGS_Application::initSimulation():";
static char __egs_app_msg3[] = "EGS_Application::runSimulation():";

static EGS_LOCAL bool __egs_find_pegsfile(const vector<string> &paths,
        const string &pegs_file, string &abs_pegs_file) {
    string pfile = pegs_file;
    if (pfile.find(".pegs4dat") == string::npos) {
        pfile += ".pegs4dat";
    }
    for (unsigned int j=0; j<paths.size(); j++) {
        string this_pegs =!paths[j].empty() ? egsJoinPath(paths[j],pfile) : pfile;
        if (!EGS_ACCESS(this_pegs.c_str(),R_OK)) {
            abs_pegs_file = this_pegs;
            return true;
        }
    }
    return false;
}

static EGS_LOCAL EGS_Application *active_egs_application = 0;

int EGS_Application::n_apps = 0;

EGS_EXPORT EGS_Application *EGS_Application::activeApplication() {
    return active_egs_application;
}

EGS_EXPORT void EGS_Application::setActiveApplication(EGS_Application *a) {
    active_egs_application = a;
};

int EGS_Application::userScoring(int iarg) {
    if (a_objects) {
        int early_return = 0;
        for (int j=0; j<a_objects[iarg].size(); ++j) {
            int res = a_objects[iarg][j]->processEvent((AusgabCall)iarg);
            if (res < 0) {
                return res;
            }
            if (res > 0) {
                early_return = res;
            }
        }
        if (early_return > 0) {
            return early_return;
        }
    }
    return ausgab(iarg);
}

void EGS_Application::checkEnvironmentVar(int &argc, char **argv,
        const char *n1, const char *n2, const char *env, string &var) {
    char *aux = getenv(env);
    if (aux) {
        var = aux;
    }
    else {
        var = "";
    }
    getArgument(argc,argv,n1,n2,var);
    if (!var.size()) egsFatal("%s\n  the environment variable %s is not set"
                                  " and it was not passed as argument\n",__egs_app_msg1,env);
    int n = var.size()-1;
    if (var[n] != '/' && var[n] != fs) {
        var += fs;
    }
}

class EGS_LOCAL EGS_GeometryHistory {
public:
    struct Step {
        EGS_Vector x, u;
        EGS_Float  twant, t;
        int        ireg, inew;
        Step() {};
        Step(int Ireg, int Inew, const EGS_Vector &X, const EGS_Vector &U,
             EGS_Float Twant, EGS_Float T) : x(X), u(U), twant(Twant), t(T),
            ireg(Ireg), inew(Inew) {};
        void show() const {
            egsWarning("old region=%d, position=(%g,%g,%g), "
                       "direction=(%g,%g,%g) intended step=%g, new region=%d, "
                       "step=%g\n",ireg,x.x,x.y,x.z,u.x,u.y,u.z,twant,inew,t);
        };
    };

    EGS_GeometryHistory(int N=2) : steps(new Step [N]), nsize(N), ns(0),
        wrap(false) {};
    ~EGS_GeometryHistory() {
        delete [] steps;
    };
    void  addStep(int ireg, int inew, const EGS_Vector &x,
                  const EGS_Vector &u, EGS_Float twant, EGS_Float t) {
        steps[ns++] = Step(ireg,inew,x,u,twant,t);
        if (ns >= nsize) {
            ns = 0;
            wrap = true;
        }
    };
    void reportHistory() const {
        int nhave = wrap ? nsize : ns;
        egsWarning("\n************** Last %d geometry steps:\n",nhave);
        if (wrap) {
            for (int j=ns; j<nsize; j++) {
                steps[j].show();
            }
        }
        for (int j=0; j<ns; j++) {
            steps[j].show();
        }
    };

    Step  *steps;
    int   nsize;
    int   ns;
    bool  wrap;
};

void EGS_Application::reportGeometryError() {
    ghistory->reportHistory();

    // check if we are over the tolerated limit for geometry errors
    run->geomErrorCount++;
    if (run->geomErrorCount > run->geomErrorMax) {
        egsFatal("\n\n******** Encountered %d geometry errors (maximum allowed is %d). \n \
        You can change this limit with the 'geometry error limit' run control input key. \n \
        Quitting now.\n", run->geomErrorCount, run->geomErrorMax);
    }
}

void EGS_Application::storeGeometryStep(int ireg, int inew,
                                        const EGS_Vector &x, const EGS_Vector &u, EGS_Float twant, EGS_Float t) {
    ghistory->addStep(ireg,inew,x,u,twant,t);
}

EGS_Application::EGS_Application(int argc, char **argv) : input(0), geometry(0),
    source(0), rndm(0), run(0), last_case(0), current_case(0),
    data_out(0), data_in(0), simple_run(false), a_objects(0),
    ghistory(new EGS_GeometryHistory) {

    app_index = n_apps++;

    if (!active_egs_application) {
        active_egs_application = this;
    }
    {
        for (int call=BeforeTransport; call<AfterTransport; call++) {
            ausgab_flag[call] = true;
        }
    }
    {
        for (int call=AfterTransport; call<UnknownCall; call++) {
            ausgab_flag[call] = false;
        }
    }

    //
    // *** get the name of this application
    //
    if (!getArgument(argc,argv,"-a","--application",app_name)) {
        app_name = egsStripPath(argv[0]);
    }
    if (!app_name.size()) egsFatal("%s\n   failed to determine application "
                                       "name from %s or command line arguments\n",__egs_app_msg1,argv[0]);
    //
    // *** make sure EGS_HOME is set.
    //
    checkEnvironmentVar(argc,argv,"-e","--egs-home","EGS_HOME",egs_home);
    app_dir = egsJoinPath(egs_home,app_name);
    //
    // *** make sure HEN_HOUSE is set.
    //
    checkEnvironmentVar(argc,argv,"-H","--hen-house","HEN_HOUSE",hen_house);
    //
    // *** check if there is an input file specified on the command line
    //
    getArgument(argc,argv,"-i","--input",input_file);
    //egsFatal("%s\n  no input file specified\n",__egs_app_msg1);
    //
    // *** create the name of the working directory
    //
    char buf[512];
    sprintf(buf,"egsrun_%d_",egsGetPid());
    run_dir = buf;
    if (input_file.size() > 0) {
        run_dir += input_file;
        run_dir += '_';
    }
    else {
        run_dir += "_noinput_";
    }
    run_dir += egsHostName();

    //
    // *** check if there is a PEGS file specified on the command line
    //
    is_pegsless= false;
    if (!getArgument(argc,argv,"-p","--pegs-file",pegs_file)) {
        is_pegsless= true;
    }
    //gsFatal("%s\n  no PEGS file specified\n",__egs_app_msg1);

    //
    // *** check if the pegs file exists.
    //
    if (pegs_file.size() > 0) {
        string pdirs = egsJoinPath("pegs4","data");
        vector<string> pegs_paths;
        pegs_paths.push_back("");
        pegs_paths.push_back(egsJoinPath(egs_home,pdirs));
        pegs_paths.push_back(egsJoinPath(hen_house,pdirs));
        if (!__egs_find_pegsfile(pegs_paths,pegs_file,abs_pegs_file))
            egsFatal("%s\n  the pegs file %s does not exist or is not "
                     "readable\n",__egs_app_msg1,pegs_file.c_str());
    }

    //
    // *** check if the input file exists.
    //
    string ifile;
    if (input_file.size() > 0) {
        ifile = egsJoinPath(app_dir,input_file);
        if (ifile.find(".egsinp") == string::npos) {
            ifile += ".egsinp";
        }
        if (EGS_ACCESS(ifile.c_str(),R_OK))
            egsFatal("%s\n  the input file %s does not exist or is not "
                     "readable\n",__egs_app_msg1,ifile.c_str());
    }

    //
    // *** set the output file
    //
    if (!getArgument(argc,argv,"-o","--output",output_file)) {
        output_file = input_file;
    }
    if (!output_file.size()) {
        output_file = "test";
    }
    //
    // *** see if a batch run.
    //
    batch_run = false;
    for (int j=1; j<argc; j++) {
        string tmp = argv[j];
        if (tmp == "-b" || tmp == "--batch") {
            batch_run = true;
            //for(int i=j; i<argc-1; i++) argv[i] = argv[i+1];
            //argc--;
            break;
        }
    }
    //
    // *** see if parallel run.
    //
    string npar, ipar, ifirst;
    n_parallel = i_parallel = 0;
    first_parallel = 1;
    bool have_np = getArgument(argc,argv,"-P","--parallel",npar);
    bool have_ip = getArgument(argc,argv,"-j","--job",ipar);
    bool have_first = getArgument(argc,argv,"-f","--first-job",ifirst);
    if (have_np && have_ip) {
        n_parallel = ::strtol(npar.c_str(),0,10);
        i_parallel = ::strtol(ipar.c_str(),0,10);
        if(have_first) {
            first_parallel = ::strtol(ifirst.c_str(),0,10);
            if(first_parallel < 0) {
                egsWarning("%s\n  invalid -f argument %d\n",__egs_app_msg1,
                       n_parallel);
                 n_parallel = 0;
                 i_parallel = 0;
            }
        }
        if (n_parallel < 0) {
            egsWarning("%s\n  invalid -P argument %d\n",__egs_app_msg1,
                       n_parallel);
            n_parallel = 0;
        }
        if (i_parallel < 0) {
            egsWarning("%s\n  invalid -j argument %d\n",__egs_app_msg1,
                       i_parallel);
            i_parallel = 0;
            n_parallel = 0;
        }
        if (i_parallel > n_parallel + first_parallel -1) {
            egsWarning("%s\n  job number (%d) can not be larger than number"
                       " of parallel jobs(%d). Turning off parallel option\n",
                       __egs_app_msg1,i_parallel,n_parallel+ first_parallel -1);
            n_parallel = 0;
            i_parallel = 0;
        }
    }
    else if (have_np || have_ip)
        egsWarning("%s\n  to specify a parallel run you need both,"
                   " the -P and -j command line options\n",__egs_app_msg1);

    //
    // *** see if user wants simple job control
    //
    {
        for (int j=1; j<argc; j++) {
            string tmp = argv[j];
            if (tmp == "-s" || tmp == "--simple-run") {
                simple_run = true;
                //for(int i=j; i<argc-1; i++) argv[i] = argv[i+1];
                //argc--;
                break;
            }
        }
    }

    final_output_file = output_file;
    if (i_parallel > 0 && n_parallel > 0) {
        batch_run = true;
        char buf[1024];
        sprintf(buf,"%s_w%d",output_file.c_str(),i_parallel);
        output_file = buf;
    }

    //
    // *** read the input
    //
    input = new EGS_Input;
    if (ifile.size() > 0) {
        if (input->setContentFromFile(ifile.c_str()))
            egsFatal("%s\n  error while reading input file %s\n",
                     __egs_app_msg1,ifile.c_str());
    }

}

bool EGS_Application::getArgument(int &argc, char **argv,
                                  const char *name1, const char *name2, string &arg) {
    string n1(name1), n2(name2);
    for (int j=1; j<argc-1; j++) {
        if (n1 == argv[j] || n2 == argv[j]) {
            arg = argv[j+1];
            //for(int i=j; i<argc-2; i++) argv[i] = argv[i+2];
            //argc -= 2;
            return true;
        }
    }
    return false;
}

string EGS_Application::constructIOFileName(const char *extension,
        bool with_run_dir) const {
    string iofile = with_run_dir ? egsJoinPath(app_dir,run_dir) : app_dir;
    iofile = egsJoinPath(iofile,output_file);
    if (extension) {
        iofile += extension;
    }
    return iofile;
}

int EGS_Application::outputData() {

    if (data_out) {
        delete data_out;
    }
    string ofile = constructIOFileName(".egsdat",true);
    /*
    string ofile = egsJoinPath(app_dir,run_dir);
    ofile = egsJoinPath(ofile,output_file);
    ofile += ".egsdat";
    */
    data_out = new ofstream(ofile.c_str());
    if (!(*data_out)) {
        egsWarning("EGS_Application::outputData: failed to open %s "
                   "for writing\n",ofile.c_str());
        return 1;
    }
    if (!run->storeState(*data_out)) {
        return 2;
    }
    if (!egsStoreI64(*data_out,current_case)) {
        return 3;
    }
    (*data_out) << endl;
    if (!rndm->storeState(*data_out)) {
        return 4;
    }
    if (!source->storeState(*data_out)) {
        return 5;
    }
    for (int j=0; j<a_objects_list.size(); ++j) {
        if (!a_objects_list[j]->storeState(*data_out)) {
            return 6;
        }
    }
    return 0;
}

int EGS_Application::readData() {
    if (data_in) {
        delete data_in;
    }
    string ifile = constructIOFileName(".egsdat",false);
    /*
    string ifile = egsJoinPath(app_dir,output_file);
    ifile += ".egsdat";
    */
    data_in = new ifstream(ifile.c_str());
    if (!(*data_in)) {
        egsWarning("EGS_Application::readData: failed to open %s "
                   "for reading\n",ifile.c_str());
        return 1;
    }
    if (!run->setState(*data_in)) {
        return 2;
    }
    if (!egsGetI64(*data_in,current_case)) {
        return 3;
    }
    last_case = current_case;
    if (!rndm->setState(*data_in)) {
        return 4;
    }
    if (!source->setState(*data_in)) {
        return 5;
    }
    for (int j=0; j<a_objects_list.size(); ++j) {
        if (!a_objects_list[j]->setState(*data_in)) {
            return 6;
        }
    }
    return 0;
}

void EGS_Application::resetCounter() {
    last_case = 0;
    current_case = 0;
    run->resetCounter();
    rndm->resetCounter();
    source->resetCounter();
    for (int j=0; j<a_objects_list.size(); ++j) {
        a_objects_list[j]->resetCounter();
    }

}

int EGS_Application::addState(istream &data) {
    if (!run->addState(data)) {
        return 1;
    }
    EGS_I64 tmp_case;
    if (!egsGetI64(data,tmp_case)) {
        return 2;
    }
    current_case += tmp_case;
    last_case = current_case;
    if (!rndm->addState(data)) {
        return 3;
    }
    if (!source->addState(data)) {
        return 4;
    }
    for (int j=0; j<a_objects_list.size(); ++j)
        if (!a_objects_list[j]->addState(data)) {
            return 5;
        }
    return 0;
}

int EGS_Application::combineResults() {
    egsInformation(
        "\n                      Suming the following .egsdat files:\n"
        "=======================================================================\n");
    char buf[512];
    resetCounter();
    EGS_Float last_cpu = 0;
    EGS_I64 last_ncase = 0;
    int ndat = 0;
    bool ok = true;
    for (int j=1; j<500; j++) {
        sprintf(buf,"%s_w%d.egsdat",output_file.c_str(),j);
        string dfile = egsJoinPath(app_dir,buf);
        ifstream data(dfile.c_str());
        if (data) {
            int err = addState(data);
            ++ndat;
            if (!err) {
                EGS_I64 ncase = run->getNdone();
                EGS_Float cpu = run->getCPUTime();
                egsInformation("%2d %-30s ncase=%-14lld cpu=%-11.2f\n",
                               ndat,buf,ncase-last_ncase,cpu-last_cpu);
                last_ncase = ncase;
                last_cpu = cpu;
            }
            else {
                ok = false;
                egsWarning("%2d %-30s error %d\n",ndat,buf,err);
            }
        }
    }
    if (ndat > 0) {
        egsInformation(
            "=======================================================================\n");
        egsInformation("%40s%-14lld cpu=%-11.2f\n\n","Total ncase=",last_ncase,
                       last_cpu);
    }
    if (ndat > 0) {
        return ok ? 0 : -1;
    }
    else {
        return 1;
    }
}

EGS_I64 EGS_Application::randomNumbersUsed() const {
    if (!rndm) {
        return 0;
    }
    return rndm->numbersUsed();
}

int EGS_Application::initGeometry() {
    if (!input) {
        return -1;
    }
    //if( geometry ) { delete geometry; geometry = 0; }
    EGS_BaseGeometry::setActiveGeometryList(app_index);
    geometry = EGS_BaseGeometry::createGeometry(input);
    if (!geometry) {
        return 1;
    }
    geometry->ref();
    return 0;
}

int EGS_Application::initSource() {
    if (!input) {
        return -1;
    }
    //if( source ) { delete source; source = 0; }
    source = EGS_BaseSource::createSource(input);
    if (!source) {
        return 1;
    }
    source->ref();
    return 0;
}

int EGS_Application::initRunControl() {
    if (run) {
        delete run;
    }
    if (simple_run) {
        run = new EGS_RunControl(this);
    }
    else {
        run = EGS_RunControl::getRunControlObject(this);
    }
    if (!run) {
        return 1;
    }
    return 0;
}

int EGS_Application::initRNG() {
    if (rndm) {
        delete rndm;
    }
    int sequence = 0;
    if (n_parallel > 0 && i_parallel > 0) {
        sequence = i_parallel - 1;
    }
    if (input) {
        rndm = EGS_RandomGenerator::createRNG(input,sequence);
    }
    if (!rndm) {
        egsWarning("EGS_Application::initRNG(): using default RNG\n");
        rndm = EGS_RandomGenerator::defaultRNG(sequence);
    }
    if (!rndm) {
        egsWarning("EGS_Application::initRNG(): got null RNG?\n");
        return 1;
    }
    return 0;
}

int EGS_Application::initSimulation() {
    //if( !input ) { egsWarning("%s no input\n",__egs_app_msg2); return -1; }
    egsInformation("In EGS_Application::initSimulation()\n");
    int err;
    bool ok = true;
    err = initGeometry();
    if (err) {
        egsWarning("\n\n%s geometry initialization failed\n",__egs_app_msg2);
        ok = false;
    }
    err = initSource();
    if (err) {
        egsWarning("\n\n%s source initialization failed\n",__egs_app_msg2);
        ok = false;
    }
    err = initRNG();
    if (err) {
        egsWarning("\n\n%s RNG initialization failed\n",__egs_app_msg2);
        ok = false;
    }
    err = initRunControl();
    if (err) {
        egsWarning("\n\n%s run control initialization failed\n",__egs_app_msg2);
        ok = false;
    }
    if (!ok) {
        return 1;
    }
    err = initEGSnrcBackEnd();
    if (err) {
        egsWarning("\n\n%s back-end initialization failed\n",__egs_app_msg2);
        return 2;
    }
    describeUserCode();
    err = initCrossSections();
    if (err) {
        egsWarning("\n\n%s cross section initialization failed\n",__egs_app_msg2);
        return 3;
    }
    err = initScoring();
    if (err) {
        egsWarning("\n\n%s scoring initialization failed with status %d\n",
                   __egs_app_msg2,err);
        return 4;
    }
    initAusgabObjects();
    //describeSimulation();
    return 0;
}

void EGS_Application::setSimulationChunk(EGS_I64 nstart, EGS_I64 nrun) {
    if (source) {
        source->setSimulationChunk(nstart,nrun);
    }
}

void EGS_Application::describeSimulation() {
    if (!geometry && !source) {
        return;
    }
    egsInformation("\n\n");
    if (geometry) {
        geometry->printInfo();
    }
    if (source) egsInformation("\n\nThe simulation uses the following source:"
                                   "\n========================================="
                                   "\n %s\n\n\n",source->getSourceDescription());
    if (rndm) {
        rndm->describeRNG();
        egsInformation("\n\n");
    }
    if (a_objects_list.size() > 0) {
        egsInformation("The following ausgab objects are included in the simulation\n");
        egsInformation("===========================================================\n\n");
        for (int j=0; j<a_objects_list.size(); ++j) {
            egsInformation("%s",a_objects_list[j]->getObjectDescription());
        }
        egsInformation("\n\n");
    }
}

int EGS_Application::runSimulation() {
    bool ok = true;
    if (!geometry) {
        egsWarning("%s no geometry\n",__egs_app_msg3);
        ok = false;
    }
    if (!source) {
        egsWarning("%s no source\n",__egs_app_msg3);
        ok = false;
    }
    if (!rndm) {
        egsWarning("%s no RNG\n",__egs_app_msg3);
        ok = false;
    }
    if (!run) {
        egsWarning("%s no run control object\n",__egs_app_msg3);
        ok = false;
    }
    if (!ok) {
        return 1;
    }

    int start_status = run->startSimulation();
    if (start_status) {
        if (start_status < 0) {
            egsWarning("\n%s failed to start the simulation\n\n",__egs_app_msg3);
        }
        return start_status;
    }

    EGS_I64 ncase;
    bool next_chunk = true;

    while (next_chunk && (ncase = run->getNextChunk()) > 0) {

        egsInformation("\nRunning %lld histories\n",ncase);
        double f,df;
        if (run->getCombinedResult(f,df)) {
            char c = '%';
            egsInformation("    combined result from this and other parallel"
                           " runs: %lg +/- %7.3lf%c\n\n",f,df,c);
        }
        else {
            egsInformation("\n");
        }
        int nbatch = run->getNbatch();
        EGS_I64 ncase_per_batch = ncase/nbatch;
        if (!ncase_per_batch) {
            ncase_per_batch = 1;
            nbatch = ncase;
        }
        for (int ibatch=0; ibatch<nbatch; ibatch++) {
            if (!run->startBatch(ibatch,ncase_per_batch)) {
                egsInformation("  startBatch() loop termination\n");
                next_chunk = false;
                break;
            }
            for (EGS_I64 icase=0; icase<ncase_per_batch; icase++) {
                if (simulateSingleShower()) {
                    egsInformation("  simulateSingleShower() "
                                   "loop termination\n");
                    next_chunk = false;
                    break;
                }
            }
            if (!next_chunk) {
                break;
            }
            if (!run->finishBatch()) {
                egsInformation("  finishBatch() loop termination\n");
                next_chunk = false;
                break;
            }
        }
    }
    // call this from within finishSimulation()
    //run->finishSimulation();
    return 0;
}

int EGS_Application::simulateSingleShower() {
    int ireg;
    int ntry = 0;
    last_case = current_case;
    do {
        ntry++;
        if (ntry > 100000) {
            egsWarning("EGS_Application::simulateSingleShower(): no particle"
                       " from the source has entered the geometry after 100000"
                       " attempts\n");
            return 1;
        }
        current_case =
            source->getNextParticle(rndm,p.q,p.latch,p.E,p.wt,p.x,p.u);
        ireg = geometry->isWhere(p.x);
        if (ireg < 0) {
            EGS_Float t = 1e30;
            ireg = geometry->howfar(ireg,p.x,p.u,t);
            if (ireg >= 0) {
                p.x += p.u*t;
            }
        }
    }
    while (ireg < 0);
    p.ir = ireg;
    int err = startNewShower();
    if (err) {
        return err;
    }
    err = shower();
    if (err) {
        return err;
    }
    return finishShower();
}

int EGS_Application::startNewShower() {
    if (current_case != last_case) {
        for (int j=0; j<a_objects_list.size(); ++j) {
            a_objects_list[j]->setCurrentCase(current_case);
        }
    }
    return 0;
}

EGS_Application::~EGS_Application() {
    if (rndm) {
        delete rndm;
    }
    if (run) {
        delete run;
    }
    if (input) {
        delete input;
    }
    EGS_Object::deleteObject(source);
    if (geometry) {
        if (!geometry->deref()) {
            EGS_Application *app = active_egs_application;
            active_egs_application = this;
            EGS_BaseGeometry::setActiveGeometryList(app_index);
            delete geometry;
            active_egs_application = app;
        }
    }
    /*
    if( a_objects ) {
        for(int j=(int)BeforeTransport; j<=(int)AugerEvent; ++j) {
            while( a_objects[j].size() ) EGS_Object::deleteObject(a_objects[j].pop());
        }
        delete [] a_objects;
    }
    */
    if (a_objects) {
        delete [] a_objects;
    }
    if (ghistory) {
        delete ghistory;
    }
    if (active_egs_application == this) {
        active_egs_application = 0;
    }
}

void EGS_Application::addAusgabObject(EGS_AusgabObject *o) {
    if (!o) {
        return;
    }
    o->setApplication(this);
    a_objects_list.add(o);
    //int ncall = 1 + (int)AugerEvent;
    int ncall = (int)UnknownCall;
    if (!a_objects) {
        a_objects = new EGS_SimpleContainer<EGS_AusgabObject *> [ncall];
    }
    for (int call=(int)BeforeTransport; call<ncall; ++call) {
        if (o->needsCall((AusgabCall)call)) {
            a_objects[call].add(o);
            setAusgabCall((AusgabCall)call,true);
        }
    }
}

void EGS_Application::initAusgabObjects() {
    if (!input) {
        return;
    }
    EGS_AusgabObject::createAusgabObjects(input);
    for (int i=0; i<EGS_AusgabObject::nObjects(); ++i) {
        addAusgabObject(EGS_AusgabObject::getObject(i));
    }
}

int EGS_Application::finishSimulation() {
    //I don't think we need a separate analyzeResults function.
    //analyzeResults();
    int err = 0;
    if (run) {
        err = run->finishSimulation();
        if (err < 0) {
            return err;
        }
    }

    outputResults();
    for (int j=0; j<a_objects_list.size(); ++j) {
        a_objects_list[j]->reportResults();
    }

    if (data_out) {
        delete data_out;
        data_out = 0;
        /*
        #ifdef WIN32
        string dfile = egsJoinPath(app_dir,run_dir);
        dfile = egsJoinPath(dfile,output_file);
        dfile += ".egsdat";
        string command = "move /Y ";
        command += dfile; command += "  "; command += app_dir;
        egsInformation("About to execute <%s>\n",command.c_str());
        int istat = system(command.c_str());
        if( istat ) egsWarning("Failed to move the .egsdat file from the"
               " working directory\n");
        #endif
        */
    }
    finishRun();
    run_dir = "";  // i.e. from now on all output will go to the user code
    // directory.
    return err;
}

void EGS_Application::fillRandomArray(int n, EGS_Float *rarray) {
    rndm->fillArray(n,rarray);
}

void EGS_Application::checkDeviceFull(FILE *stream) {
#ifndef WIN32
    // quit if space left on disk is less than 1 MB to mitigate disk full problems

    // stat the stream
    struct stat tmp;
    fstat(fileno(stream), &tmp);

    // check only if the stream is a regular file
    if (S_ISREG(tmp.st_mode)) {
        struct statvfs fstats;
        fstatvfs(fileno(stdout), &fstats);
        if (fstats.f_bavail*fstats.f_bsize < 1048576) {
            egsFatal("\n\n******** Space left on file system is below 1 MB. Quitting now.\n");
            exit(1);
        }
    }
#endif
}

void EGS_Application::appInformation(const char *msg) {
    if (msg) {
        fprintf(stdout,"%s",msg);
        fflush(stdout);
        checkDeviceFull(stdout);
    }
}

void EGS_Application::appWarning(const char *msg) {
    if (msg) {
        fprintf(stderr,"%s",msg);
        fflush(stderr);
        checkDeviceFull(stderr);
    }
}

void EGS_Application::appFatal(const char *msg) {
    if (msg) {
        fprintf(stderr,"%s",msg);
        fflush(stderr);
    }
    exit(1);
}
