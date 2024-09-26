/*
###############################################################################
#
#  EGSnrc egs++ egs_quality application
#  Copyright (C) 2017 National Research Council Canada
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
#  Author:          Reid Townson, 2017
#
#  Contributors:
#
###############################################################################
#
#  An application for testing the quality of an egs++ build
#
###############################################################################
*/

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <algorithm>

// We derive from EGS_AdvancedApplication => need the header file.
#include "egs_advanced_application.h"
// We use scoring objects provided by egspp => need the header file.
#include "egs_scoring.h"
// Every C++ EGSnrc application needs this header file
#include "egs_interface2.h"
// We use egsInformation() => need the egs_functions.h header file.
#include "egs_functions.h"
// We use the EGS_Input class
#include "egs_input.h"
// To get the maximum source energy
#include "egs_base_source.h"
// The random number generator
#include "egs_rndm.h"
// Transformations
#include "egs_transformations.h"
// Interpolators
#include "egs_interpolator.h"
#include "egs_run_control.h"
#include "egs_quality.h"

#include "egs_rndm.h"
#define getRNGPointers F77_OBJ_(egs_get_rng_pointers,EGS_GET_RNG_POINTERS)
extern __extc__ void getRNGPointers(EGS_I32 *, EGS_I32 *);
#define getRNGArray F77_OBJ_(egs_get_rng_array,EGS_GET_RNG_ARRAY)
extern __extc__ void getRNGArray(EGS_Float *);
#define setRNGState F77_OBJ_(egs_set_rng_state,EGS_SET_RNG_STATE)
extern __extc__ void setRNGState(const EGS_I32 *, const EGS_Float *);

#define doRayleigh F77_OBJ_(do_rayleigh,DO_RAYLEIGH)
extern __extc__ void doRayleigh();
extern __extc__ void F77_OBJ(pair,PAIR)();
extern __extc__ void F77_OBJ(compt,COMPT)();
extern __extc__ void F77_OBJ(photo,PHOTO)();

/*! Constructor */
EGS_QualityApplication::EGS_QualityApplication(int argc, char **argv) :
        EGS_AdvancedApplication(argc,argv) {

    input_orig = new EGS_Input();
    input_orig->addInputItem(*input);
};

/*! Destructor.  */
EGS_QualityApplication::~EGS_QualityApplication() {
    delete input_orig;
};

const static char __egs_app_msg3[] = "EGS_QualityApplication::runSimulation():";
const static char __egs_app_msg2[] = "EGS_QualityApplication::initBackEnd():";

int EGS_QualityApplication::initSimulation() {
    egsInformation("In EGS_QualityApplication::initSimulation()\n");
    int err;
    bool ok = true;
    describeUserCode();

    return 0;
};

int EGS_QualityApplication::initBackEnd() {
    egsInformation("In EGS_QualityApplication::initBackEnd()\n");
    int err;
    bool ok = true;

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
    return 0;
};

/*! Describe the application.  */
void EGS_QualityApplication::describeUserCode() const {
    egsInformation(
      "\n               *************************************************"
      "\n               *                                               *"
      "\n               *                  egs_quality                  *"
      "\n               *                                               *"
      "\n               *************************************************"
      "\n\n");
};

int EGS_QualityApplication::initScoring() {
    return 0;
}


/*! Accumulate quantities of interest at run time */
int EGS_QualityApplication::ausgab(int iarg) {

    return 0;
};

int EGS_QualityApplication::runSimulation() {
    bool ok = true;
    string inputDir = egsJoinPath(app_dir, "inputs");
    string geometryDir = egsJoinPath(inputDir, "geometries");
    string sourceDir = egsJoinPath(inputDir, "sources");
    string appInputDir = egsJoinPath(inputDir, "apps");
    string outputDir = egsJoinPath(app_dir, "outputs");
    string currentOutputDir = egsJoinPath(outputDir, "latest");
    string benchmarkDir = egsJoinPath(outputDir, "benchmark");

    // Open an output file to write a report to
    string reportFilename = EGS_AdvancedApplication::getFinalOutputFile();
    string reportFile = egsJoinPath(currentOutputDir, reportFilename + "_report.txt");

    egsInformation("Results will be output to:\n %s\n\n",reportFile.c_str());

    reportStream.open(reportFile.c_str());
    if(reportStream.is_open()) {
        reportStream << "#   EGSnrc quality report" << endl;

        // Current date/time based on current system
        time_t now = time(0);

        // Convert to string form
        char* dt = ctime(&now);

        reportStream << "### Local time: " << dt << endl;

        reportStream << "-------------------------------------------------------------------------------" << endl << endl;

        reportStream << "This file contains a quality report for EGSnrc. It is generated automatically by the `egs_quality` application and can be used to test for consistency of results following changes to the EGSnrc codebase." << endl << endl;

        reportStream << "-------------------------------------------------------------------------------" << endl << endl;
    } else {
        egsWarning("%s Error: Failed to open output report file for writing.\n",__egs_app_msg3);
        reportStream.close();
        return 1;
    }

    string outputFilename = EGS_AdvancedApplication::getFinalOutputFile();
    string outputFile = egsJoinPath(app_dir, outputFilename + ".egslog");

    // *************************************
    // Quality test applications
    // *************************************
    EGS_Input *qualityTestApps = input->takeInputItem("quality test applications");
    if(qualityTestApps) {

        reportStream << "## Application quality tests" << endl;
        reportStream << "These tests check the output of various applications." << endl;

        EGS_Input *aux;
        while((aux = qualityTestApps->takeInputItem("test"))) {

            // Get the application name
            string app;
            int err = aux->getInput("application",app);
            if(err) {
                egsWarning("EGS_QualityApplication::runSimulation: Application quality test is missing require input: 'application'\n");
                continue;
            }

            // Optional inputs
            vector<string> inputs, pegs;
            err = aux->getInput("inputs",inputs);
            err = aux->getInput("pegs",pegs);

            if(!inputs.size()) {
                if(!pegs.size()) {
                    string cmd = app;

                    // Run the application
                    reportStream << endl << "### `" << cmd << "`" << endl;
                    string result = runCommand(cmd);

                    string newOutputFile = egsJoinPath(currentOutputDir, app + ".egslog");
                    string benchmarkFile = egsJoinPath(benchmarkDir, app + ".egslog");

                    // If the file already exists remove it
                    remove(newOutputFile.c_str());

                    // Write the results to a file
                    ofstream out(newOutputFile.c_str());
                    out << result;
                    out.close();

                    // Compare against the benchmark
                    compareAppOutput(app, newOutputFile, benchmarkFile);
                } else {

                    // Run the application for each pegs file
                    for(int i=0; i<pegs.size(); ++i) {

                        string cmd = app;
                        cmd += " -p " + pegs[i];

                        // Run the application
                        reportStream << endl << "### `" << cmd << "`" << endl;
                        string result = runCommand(cmd);

                        string newOutputFile = egsJoinPath(currentOutputDir, app + "_" + pegs[i] + ".egslog");
                        string benchmarkFile = egsJoinPath(benchmarkDir, app + "_" + pegs[i] + ".egslog");

                        // If the file already exists remove it
                        remove(newOutputFile.c_str());

                        // Write the results to a file
                        ofstream out(newOutputFile.c_str());
                        out << result;
                        out.close();

                        // Compare against the benchmark
                        compareAppOutput(app, newOutputFile, benchmarkFile);
                    }
                }
            } else {

                // Run the application for each input file
                for(int i=0; i<inputs.size(); ++i) {

                    // Move the input file to the app directory
                    std::ifstream  src((appInputDir + "/" + app + "/" + inputs[i] + ".egsinp").c_str());
                    std::ofstream  dst((egs_home + "/" + app + "/" + inputs[i] + ".egsinp").c_str());
                    dst << src.rdbuf();

                    string cmd = app;
                    cmd += " -i " + inputs[i];
                    if(!pegs.size() || pegs.size()-1 < i) {
                        // pegsless
                    } else {
                        cmd += " -p " + pegs[i];
                    }

                    // Run the application, save stdout to the result string
                    reportStream << endl << "### " << cmd << "`" << endl;
                    string result = runCommand(cmd);

                    string newOutputFile, benchmarkFile;
                    if(!pegs.size() || pegs.size()-1 < i) {
                        newOutputFile = egsJoinPath(currentOutputDir, app + "_" + inputs[i] + ".egslog");
                        benchmarkFile = egsJoinPath(benchmarkDir, app + "_" + inputs[i] + ".egslog");
                    } else {
                        newOutputFile = egsJoinPath(currentOutputDir, app + "_" + inputs[i] + "_" + pegs[i] + ".egslog");
                        benchmarkFile = egsJoinPath(benchmarkDir, app + "_" + inputs[i] + "_" + pegs[i] + ".egslog");
                    }

                    // If the file already exists remove it
                    remove(newOutputFile.c_str());

                    if(app == "dosrznrc" || app == "flurznrc" || app == "cavrznrc" || app == "cavsphnrc" || app == "sprrznrc" || app == "BEAM_EX10MeVe" || app == "dosxyznrc") {
                        // Move the egslog file into the output directory
                        // instead of using the stdout
                        rename((egs_home + "/" + app + "/" + inputs[i] + ".egslst").c_str(), newOutputFile.c_str());
                    } else {
                        // Write the stdout results to a file
                        ofstream out(newOutputFile.c_str());
                        out << result;
                        out.close();
                    }

                    // Compare against the benchmark
                    compareAppOutput(app, newOutputFile, benchmarkFile);
                }
            }
        }

        reportStream << "\n-------------------------------------------------------------------------------" << endl << endl;

        delete qualityTestApps;
    }


    // *************************************
    // Quality test geometries
    // *************************************

    EGS_Input *qualityTestGeometries = input->takeInputItem("quality test geometries");
    if(qualityTestGeometries) {

        reportStream << "## Geometry quality tests" << endl;
        reportStream << "These tests run full simulations using each geometry, optionally repeated with multiple sources." << endl;

        // Get the geometry list
        vector<string> geometries;
        int err = qualityTestGeometries->getInput("geometry inputs",geometries);
        if (err || !geometries.size()) {
            egsWarning("%s Error: Missing geometry inputs\n",__egs_app_msg3);
        }
        else {
            // For each geometry
            for (vector<string>::iterator it = geometries.begin();
                    it!=geometries.end(); ++it) {

                // Get the source list
                vector<string> sources;
                err = qualityTestGeometries->getInput("source inputs",sources);
                if (err || !sources.size()) {
                    egsWarning("%s Error: Missing source inputs\n",__egs_app_msg3);
                }
                else {
                    // For each source
                    for (vector<string>::iterator it2 = sources.begin();
                            it2!=sources.end(); ++it2) {

                        // Reset stdout to go to the egslog file
                        freopen(outputFile.c_str(),"w",stdout);

                        reportStream << endl << "### `" << *it << "` / `" << *it2 << "`" << endl;

                        // Reset to the original input data
                        delete input;
                        input = new EGS_Input();
                        input->addInputItem(*input_orig);

                        // Add the geometry
                        string geomFile = egsJoinPath(geometryDir, *it + ".egsinp");
                        egsInformation("%s Testing geometry: %s\n", __egs_app_msg3, geomFile.c_str());
                        egsWarning("%s Testing geometry: %s\n", __egs_app_msg3, geomFile.c_str());
                        err = input->addContentFromFile(geomFile.c_str());
                        if(!err) {
                            if(initGeometry()) {
                                egsWarning("%s Error: Failed to initialize geometry\n",__egs_app_msg3);
                            }
                        } else {
                            egsWarning("%s Error: Geometry input file not found\n",__egs_app_msg3);

                            reportStream << "**FAIL** geometry input file not found" << endl;
                            reportStream.close();
                            delete qualityTestGeometries;
                            return 1;
                        }

                        // Add the source
                        string sourceFile = egsJoinPath(sourceDir, *it2 + ".egsinp");
                        egsInformation("%s Testing source: %s\n", __egs_app_msg3, sourceFile.c_str());
                        err = input->addContentFromFile(sourceFile.c_str());
                        if(!err) {
                            if(initSource()) {
                                egsWarning("%s Error: Failed to initialize source\n",__egs_app_msg3);
                            }
                        } else {
                            egsWarning("%s Error: Source input file not found\n",__egs_app_msg3);

                            reportStream << "**FAIL** source input file not found" << endl;
                            reportStream.close();
                            delete qualityTestGeometries;
                            return 1;
                        }

                        // Complete the application initialization
                        err = initBackEnd();
                        if(err) {
                            reportStream.close();
                            delete qualityTestGeometries;
                            return 1;
                        }

                        // Run the simulation
                        err = runMiniSim();
                        if(err) {
                            reportStream.close();
                            delete qualityTestGeometries;
                            return 1;
                        }

                        // Finish the simulation
                        err = EGS_AdvancedApplication::finishSimulation();
                        if(err < 0) {
                            reportStream << "**FAIL** finishSimulation() returned " << err << endl;
                            reportStream.close();
                            delete qualityTestGeometries;
                            return err;
                        }

                        // Move the output file to a directory for later analysis
                        string newOutputFile = egsJoinPath(currentOutputDir, outputFilename + "_" + *it + "_" + *it2 + ".egslog");

                        // If the file already exists remove it
                        remove(newOutputFile.c_str());

                        egsInformation("\nRenaming %s to %s\n",outputFile.c_str(), newOutputFile.c_str());
                        if(rename(outputFile.c_str(),newOutputFile.c_str())) {
                            egsWarning("%s Error: Failed to move output file to output directory. Make sure to run in parallel in order to produce an egslog file.\n",__egs_app_msg3);
                        } else {

                            // The benchmark file we will compare with
                            string benchmarkFile = egsJoinPath(benchmarkDir, outputFilename + "_" + *it + "_" + *it2 + ".egslog");

                            egsInformation("\nComparing %s with %s\n\n",newOutputFile.c_str(), benchmarkFile.c_str());

                            // Compare the egslog files
                            compareEgsppAusgab(newOutputFile, benchmarkFile);
                        }

                        // Reset the application objects for the next run
                        resetCounter();
                        //EGS_Object::deleteObject(source);
                        delete source;
                        delete geometry;
                        geometry = NULL;
                        EGS_BaseGeometry::clearGeometries();
                        a_objects_list.clear();
                        if (a_objects) {
                            delete [] a_objects;
                            a_objects = NULL;
                        }
                        delete rndm;
                        rndm = NULL;
                        //delete run;
                    }
                }
            }
        }
        delete qualityTestGeometries;

        reportStream << endl << "-------------------------------------------------------------------------------" << endl << endl;
    }

    // *************************************
    // Quality test sources
    // *************************************

    const unsigned int numTestParticles = 10;
    EGS_Input *qualityTestSources = input->takeInputItem("quality test sources");

    if(qualityTestSources) {

        reportStream << "## Source quality tests" << endl;
        reportStream << "These tests check for an *exact* particle match of each source, compared against benchmark data. Only expect success when the benchmark was calculated with the same machine, code version, compiler and random number seeds." << endl;

        // Get the source list
        vector<string> sources;
        int err = qualityTestSources->getInput("source inputs",sources);
        if (err || !sources.size()) {
            egsWarning("%s Error: Missing source inputs\n",__egs_app_msg3);
        }
        else {
            // For each source
            for (vector<string>::iterator it = sources.begin();
                    it!=sources.end(); ++it) {

                // Reset stdout to go to the egslog file
                freopen(outputFile.c_str(),"w",stdout);

                reportStream << endl << "### `" << *it << "`" << endl;

                // Reset to the original input data
                delete input;
                input = new EGS_Input();
                input->addInputItem(*input_orig);

                // Add a geometry that will not be used
                // This just needs to be there for source generation to work
                string geomFile = egsJoinPath(geometryDir, "egs_box.egsinp");
                err = input->addContentFromFile(geomFile.c_str());
                if(!err) {
                    if(initGeometry()) {
                        egsWarning("%s Error: Failed to initialize geometry\n",__egs_app_msg3);
                    }
                }

                bool fail = false;
                unsigned int failCount = 0;

                // Add the source
                string sourceFile = egsJoinPath(sourceDir, *it + ".egsinp");
                egsInformation("%s Testing source: %s\n", __egs_app_msg3, sourceFile.c_str());
                egsWarning("%s Testing source: %s\n", __egs_app_msg3, sourceFile.c_str());
                err = input->addContentFromFile(sourceFile.c_str());
                if(!err) {
                    if(initSource()) {
                        egsWarning("%s Error: Failed to initialize source\n",__egs_app_msg3);
                    }
                }

                // Complete the application initialization
                err = initBackEnd();
                if(err) {
                    delete qualityTestSources;
                    reportStream.close();
                    return 1;
                }
                int start_status = run->startSimulation();

                // Open the benchmark file to compare against
                string benchmarkFile = egsJoinPath(benchmarkDir, outputFilename + "_" + *it + ".egslog");
                ifstream benchmarkStream (benchmarkFile.c_str());

                // Skip the first line in the benchmark (number of particles)
                string line;
                if(benchmarkStream.is_open()) {
                    getline(benchmarkStream, line);
                } else {
                    reportStream << "**FAIL** benchmark does not exist" << endl;
                    fail = true;
                }

                // Open an output file to write to
                string outputFilename = EGS_AdvancedApplication::getFinalOutputFile();
                string outputFile = egsJoinPath(currentOutputDir, outputFilename + "_" + *it + ".egslog");
                ofstream outputStream (outputFile.c_str());

                // Write the output and compare with the benchmark
                if(outputStream.is_open()) {

                    // First line is the number of particles
                    outputStream << numTestParticles << endl;
                    for(int i=0; i<numTestParticles; ++i) {

                        // Get the next particle
                        source->getNextParticle(rndm,p.q,p.latch,p.E,p.wt,p.x,p.u);

                        // Write the data to output
                        stringstream buffer;
                        buffer << p.q << " " << p.E << " " << p.wt;
                        buffer << " " << p.latch;
                        buffer << " " << p.q << " " << p.E << " " << p.wt;
                        buffer << " " << p.x.x << " " << p.x.y << " " << p.x.z;
                        buffer << " " << p.u.x << " " << p.u.y << " " << p.u.z;
                        outputStream << buffer.str() << endl;

                        // Compare with the benchmark
                        // This just tests to see if the lines are an exact match
                        if(benchmarkStream.is_open()) {
                            if(getline(benchmarkStream, line)) {
                                if(line.compare(buffer.str()) != 0) {
                                    fail = true;
                                    failCount++;
                                }
                            }
                        }
                    }
                    outputStream.close();

                    if (benchmarkStream.is_open()) {
                        benchmarkStream.close();
                    }
                }

                // Finish the simulation
                err = EGS_AdvancedApplication::finishSimulation();
                if(err < 0) {
                    reportStream << "**FAIL** finishSimulation() returned " << err << endl;
                    reportStream.close();
                    delete qualityTestSources;
                    return err;
                }

                if(fail) {
                    reportStream << "**FAIL** on " << failCount << " of " << numTestParticles << " particles" << endl;
                } else {
                    reportStream << "**PASS** with exact match" << endl;
                }

                // Reset the application objects for the next run
                if(it < sources.end()-1) {
                    resetCounter();
                    //EGS_Object::deleteObject(source);
                    delete source;
                    delete geometry;
                    geometry = NULL;
                    EGS_BaseGeometry::clearGeometries();
                    a_objects_list.clear();
                    if (a_objects) {
                        delete [] a_objects;
                        a_objects = NULL;
                    }
                    delete rndm;
                    rndm = NULL;
                    //delete run;
                }
            }
        }

        delete qualityTestSources;

        reportStream << endl << "-------------------------------------------------------------------------------" << endl << endl;
    }

    reportStream.close();
    return 0;
}

int EGS_QualityApplication::runMiniSim() {
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
    return 0;
}

/*! Execute on the command line */
string EGS_QualityApplication::runCommand(string cmd) {
    char buffer[128];
    string result = "";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (pipe) {
        while (!feof(pipe)) {
            if (fgets(buffer, 128, pipe) != NULL) {
                result += buffer;
            }
        }
        pclose(pipe);
    }

    return result;
}

/*! Compare application outputs */
int EGS_QualityApplication::compareAppOutput(string app, string newFile, string benchmarkFile) {
    egsWarning("EGS_QualityApplication::compareAppOutput: %s\n",app.c_str());

    ifstream newStream (newFile.c_str());
    ifstream benchmarkStream (benchmarkFile.c_str());

    if(!newStream.is_open()) {
        egsWarning("\n%s:compareEgsppAusgab: Error: Failed to open latest egslog file: %s\n\n",__egs_app_msg3, newFile.c_str());
        reportStream << "**FAIL** missing latest egslog file for comparison" << endl;
        return 1;
    }

    if(!benchmarkStream.is_open()) {
        egsWarning("\n%s:compareEgsppAusgab: Error: Failed to open benchmark egslog file: %s\n\n",__egs_app_msg3, benchmarkFile.c_str());
        reportStream << "**FAIL** missing benchmark egslog file for comparison" << endl;
        return 1;
    }

    int numMismatches = 0;

    // Loop through the new output and compare it with the benchmark
    if(app == "tutor1") {
            string startLine = "Kinetic Energy(MeV)  charge  angle w.r.t.Z axis-degrees";
            string stopLine = "=========================================";

            // Check for an exact match of the result lines
            numMismatches = compareLinesExact(startLine, stopLine, newStream, benchmarkStream);

            if(numMismatches) {
                reportStream << "**FAIL** " << numMismatches << " result lines did not match" << endl;
            } else {
                reportStream << "**PASS** with exact match" << endl;
            }
    } else if(app == "tutor7pp") {
        // First set of results is between these lines
        string startLine = "ir+1 | Reflected, deposited, or transmitted energy fraction";
        string stopLine = "Pulse height distribution";

        // Get the lines between the flags
        vector<string> newResult, benchmarkResult;
        getLinesBetween(startLine, stopLine, newStream, benchmarkStream, newResult, benchmarkResult);

        // Check if there is agreement within uncertainty
        compareLinesStat("%s val %s uncAbs", newResult, benchmarkResult);

        // Second set of results is between these lines
        startLine = "=======================";
        stopLine = "=======================";
        newResult.clear();
        benchmarkResult.clear();
        getLinesBetween(startLine, stopLine, newStream, benchmarkStream, newResult, benchmarkResult);
        compareLinesStat("%s val uncAbs", newResult, benchmarkResult);

    } else if(app == "egs_chamber") {
        // First set of results is between these lines
        string startLine = "Geometry                        Cavity dose";
        string stopLine = "=======================";
        vector<string> newResult, benchmarkResult;
        getLinesBetween(startLine, stopLine, newStream, benchmarkStream, newResult, benchmarkResult);
        compareLinesStat("%s val %s uncRel %s", newResult, benchmarkResult);

    } else if(app == "BEAM_EX10MeVe") {
        string startLine = "---- -----------  ------------------  ------------------";
        string stopLine = "CONTAMINANT DOSE/INCIDENT";
        vector<string> newResult, benchmarkResult;
        getLinesBetween(startLine, stopLine, newStream, benchmarkStream, newResult, benchmarkResult);
        compareLinesStat("BEAM_EX10MeVe", newResult, benchmarkResult);

    } else if(app == "dosxyznrc") {
        string startLine = "DOSXYZnrc Dose outputs";
        string stopLine = "The dose values in";
        vector<string> newResult, benchmarkResult;
        getLinesBetween(startLine, stopLine, newStream, benchmarkStream, newResult, benchmarkResult);
        compareLinesStat("dosxyznrc", newResult, benchmarkResult);

    } else if(app == "dosrznrc") {
        string startLine = "|T  :";
        string stopLine = "1.0000-------------";
        vector<string> newResult, benchmarkResult;
        getLinesBetween(startLine, stopLine, newStream, benchmarkStream, newResult, benchmarkResult);
        compareLinesStat("dosrznrc", newResult, benchmarkResult);

    } else if(app == "cavrznrc") {
        string startLine = "TOTAL DOSE/Awall:";
        string stopLine = "END OF RUN";
        vector<string> newResult, benchmarkResult;
        getLinesBetween(startLine, stopLine, newStream, benchmarkStream, newResult, benchmarkResult);
        compareLinesStat("cavrznrc", newResult, benchmarkResult);

    } else if(app == "cavsphnrc") {
        string startLine = "TOTAL DOSE (GRAYS/INCIDENT FLUENCE):";
        string stopLine = "===============";
        vector<string> newResult, benchmarkResult;
        getLinesBetween(startLine, stopLine, newStream, benchmarkStream, newResult, benchmarkResult);
        compareLinesStat("cavsphnrc", newResult, benchmarkResult);

    } else if(app == "flurznrc") {
        string startLine = "|Te-:";
        string stopLine = "|Te+:";
        vector<string> newResult, benchmarkResult;
        getLinesBetween(startLine, stopLine, newStream, benchmarkStream, newResult, benchmarkResult);
        compareLinesStat("flurznrc", newResult, benchmarkResult);

    } else if(app == "sprrznrc") {
        string startLine = "Region           depth (cm)";
        string stopLine = "***** spr without track ends";
        vector<string> newResult, benchmarkResult;
        getLinesBetween(startLine, stopLine, newStream, benchmarkStream, newResult, benchmarkResult);
        compareLinesStat("sprrznrc", newResult, benchmarkResult);

    } else if(app == "edknrc") {
        string startLine = "histories run in";
        string stopLine = "energy fraction Frad";
        vector<string> newResult, benchmarkResult;
        getLinesBetween(startLine, stopLine, newStream, benchmarkStream, newResult, benchmarkResult);
        compareLinesStat("edknrc", newResult, benchmarkResult);

    } else if(app == "g") {
        string startLine = " Final:";
        string stopLine = "g                 =";
        vector<string> newResult, benchmarkResult;
        getLinesBetween(startLine, stopLine, newStream, benchmarkStream, newResult, benchmarkResult);
        compareLinesStat("g", newResult, benchmarkResult);

    } else {
        egsWarning("EGS_QualityApplication::compareAppOutput: Application is not supported: %s\n",app.c_str());
        reportStream << "**FAIL** application is not supported" << endl;
        return 1;
    }

    newStream.close();
    benchmarkStream.close();

    return 0;
}

void EGS_QualityApplication::getLinesBetween(string startLine, string stopLine, ifstream &newStream, ifstream &benchmarkStream, vector<string> &newResult, vector<string> &benchmarkResult) {

    bool foundResultLine = false, foundBenchmarkLine = false;
    string newLine, benchmarkLine;
    while(getline(newStream, newLine)) {

        // Watch for this line, but don't include it
        if(!foundResultLine && newLine.find(startLine) != std::string::npos) {
            foundResultLine = true;
            continue;
        }

        if(foundResultLine) {
            // If we're not at the stop line yet, save the line
            // The stop line is not included
            if(newLine.find(stopLine) == std::string::npos) {
                newResult.push_back(newLine);
            } else {
                break;
            }
        }
    }

    while(getline(benchmarkStream, benchmarkLine)) {

        // Watch for this line, but don't include it
        if(!foundBenchmarkLine && benchmarkLine.find(startLine) != std::string::npos) {
            foundBenchmarkLine = true;
            continue;
        }

        if(foundBenchmarkLine) {
            // If we're not at the stop line yet, save the line
            // The stop line is not included
            if(benchmarkLine.find(stopLine) == std::string::npos) {
                benchmarkResult.push_back(benchmarkLine);
            } else {
                break;
            }
        }
    }
}

/*! Check if values agree within uncertainty */
void EGS_QualityApplication::compareLinesStat(string format, vector<string> &newResult, vector<string> &benchmarkResult) {
    bool exactMatch = true;
    int numFails = 0;

    // Do a statistical comparison
    unsigned int i = 0;
    for (vector<string>::iterator it = benchmarkResult.begin();
            it!=benchmarkResult.end(); ++it) {

        // Break if we run out of lines
        if(i >= newResult.size()) {
            ++numFails;
            break;
        }

        // If the strings match we can skip this line
        if((*it).compare(newResult[i]) == 0) {
            ++i;
            continue;
        }
        exactMatch = false;

        istringstream bStream(*it), nStream(newResult[i]);
        EGS_Float benchmarkValue, bUncAbs, newValue, nUncAbs;
        string tmp;

        if(format == "%s val %s uncAbs") {
            bStream >> std::skipws >> tmp >> benchmarkValue >> tmp >> bUncAbs;
            nStream >> std::skipws >> tmp >> newValue >> tmp >> nUncAbs;
        } else if(format == "%s val uncAbs") {
            bStream >> std::skipws >> tmp >> benchmarkValue >> bUncAbs;
            nStream >> std::skipws >> tmp >> newValue >> nUncAbs;
        } else if(format == "%s val %s uncRel %s") {
            EGS_Float bUncRel, nUncRel;
            bStream >> std::skipws >> tmp >> benchmarkValue >> bUncRel >> tmp;
            nStream >> std::skipws >> tmp >> newValue >> nUncRel >> tmp;
            bUncAbs = benchmarkValue * bUncRel / 100;
            nUncAbs = benchmarkValue * nUncRel / 100;
        } else if(format == "BEAM_EX10MeVe") {
            EGS_Float bUncRel, nUncRel;

            benchmarkValue = atof((*it).substr(23,9).c_str());
            newValue = atof(newResult[i].substr(23,9).c_str());

            bUncRel = atof((*it).substr(35,4).c_str());
            nUncRel = atof(newResult[i].substr(35,4).c_str());

            bUncAbs = benchmarkValue * bUncRel / 100;
            nUncAbs = benchmarkValue * nUncRel / 100;
        } else if(format == "dosxyznrc") {
            EGS_Float bUncRel, nUncRel;

            benchmarkValue = atof((*it).substr(14,9).c_str());
            newValue = atof(newResult[i].substr(14,9).c_str());

            bUncRel = atof((*it).substr(24,4).c_str());
            nUncRel = atof(newResult[i].substr(24,4).c_str());

            bUncAbs = benchmarkValue * bUncRel / 100;
            nUncAbs = benchmarkValue * nUncRel / 100;
        } else if(format == "flurznrc" || format == "dosrznrc") {
            EGS_Float bUncRel, nUncRel;

            benchmarkValue = atof((*it).substr(16,9).c_str());
            newValue = atof(newResult[i].substr(16,9).c_str());

            bUncRel = atof((*it).substr(27,4).c_str());
            nUncRel = atof(newResult[i].substr(27,4).c_str());

            bUncAbs = benchmarkValue * bUncRel / 100;
            nUncAbs = benchmarkValue * nUncRel / 100;

        } else if(format == "cavrznrc") {
            EGS_Float bUncRel, nUncRel;

            benchmarkValue = atof((*it).substr(50,10).c_str());
            newValue = atof(newResult[i].substr(50,10).c_str());

            bUncRel = atof((*it).substr(66,4).c_str());
            nUncRel = atof(newResult[i].substr(66,4).c_str());

            bUncAbs = benchmarkValue * bUncRel / 100;
            nUncAbs = benchmarkValue * nUncRel / 100;

        } else if(format == "cavsphnrc") {
            EGS_Float bUncRel, nUncRel;

            benchmarkValue = atof((*it).substr(50,10).c_str());
            newValue = atof(newResult[i].substr(50,10).c_str());

            bUncRel = atof((*it).substr(66,6).c_str());
            nUncRel = atof(newResult[i].substr(66,6).c_str());

            bUncAbs = benchmarkValue * bUncRel / 100;
            nUncAbs = benchmarkValue * nUncRel / 100;

        } else if(format == "sprrznrc") {
            EGS_Float bUncRel, nUncRel;

            benchmarkValue = atof((*it).substr(35,7).c_str());
            newValue = atof(newResult[i].substr(35,7).c_str());

            bUncAbs = atof((*it).substr(48,7).c_str());
            nUncAbs = atof(newResult[i].substr(48,7).c_str());

        } else if(format == "edknrc") {
            EGS_Float bUncRel, nUncRel;

            benchmarkValue = atof((*it).substr(27,8).c_str());
            newValue = atof(newResult[i].substr(27,8).c_str());

            bUncAbs = atof((*it).substr(42,8).c_str());
            nUncAbs = atof(newResult[i].substr(42,8).c_str());

//             egsWarning("TEST %s %s %.10e %.10e\n",(*it).substr(27,8).c_str(),(*it).substr(42,8).c_str(), benchmarkValue, bUncAbs);

        } else if(format == "g") {
            EGS_Float bUncRel, nUncRel;

            benchmarkValue = atof((*it).substr(30,18).c_str());
            newValue = atof(newResult[i].substr(30,18).c_str());

            bUncRel = atof((*it).substr(77,24).c_str());
            nUncRel = atof(newResult[i].substr(77,24).c_str());

            benchmarkValue *= pow(10,-12);
            newValue *= pow(10,-12);

            bUncAbs = benchmarkValue * bUncRel / 100;
            nUncAbs = benchmarkValue * nUncRel / 100;
        }

        // Adjust to 2 sigma
        bUncAbs *= 2;
        nUncAbs *= 2;

        bool lineFailed = false;
        if(benchmarkValue + bUncAbs < newValue - nUncAbs || benchmarkValue - bUncAbs > newValue + nUncAbs) {
            reportStream << "**FAIL** result beyond statistical uncertainty (2 sigma)" << endl;
            reportStream << benchmarkValue + bUncAbs << " < " << newValue - nUncAbs << " || " << benchmarkValue - bUncAbs << " > " << newValue + nUncAbs << endl;
            lineFailed = true;
        }

        if(lineFailed) {
            ++numFails;
            reportStream << "See the following result disagreement:" << endl;
            reportStream << "```" << endl;
            reportStream << "Latest: " << newResult[i] << endl;
            reportStream << "Benchm: " << *it << endl;
            reportStream << "```" << endl;
        }

        ++i;
    }

    if(exactMatch) {
        reportStream << "**PASS** exact result match" << endl;
    } else {
        reportStream << "**WARN** not exact result match" << endl;
        if(!numFails) {
            reportStream << "**PASS** all results agree within statistical uncertainty (2 sigma)" << endl;
        } else {
            reportStream << "**FAIL** " << numFails << " results fail to agree within statistical uncertainty (2 sigma)" << endl;
        }
    }
}

/*! Compare a set of lines from two files */
int EGS_QualityApplication::compareLinesExact(string startLine, string stopLine, ifstream &newStream, ifstream &benchmarkStream) {

    vector<string> newResult, benchmarkResult;
    int numMismatches = 0;

    getLinesBetween(startLine, stopLine, newStream, benchmarkStream, newResult, benchmarkResult);

    for(int i=0; i < benchmarkResult.size(); ++i) {
        if(i < newResult.size()) {
            if(benchmarkResult[i] != newResult[i]) {
                numMismatches++;
            }
        } else {
            numMismatches++;
        }
    }
    if(benchmarkResult.size() < newResult.size()) {
        numMismatches += newResult.size() - benchmarkResult.size();
    }

    return numMismatches;
}

/*! Compare the results of a simulation with a benchmark */
int EGS_QualityApplication::compareEgsppAusgab(string newFile, string benchmarkFile) {

    ifstream newStream (newFile.c_str());
    ifstream benchmarkStream (benchmarkFile.c_str());

    if(!newStream.is_open()) {
        egsWarning("\n%s:compareEgsppAusgab: Error: Failed to open latest egslog file: %s\n\n",__egs_app_msg3, newFile.c_str());
        reportStream << "**FAIL** missing latest egslog file for comparison" << endl;
        return 1;
    }

    if(!benchmarkStream.is_open()) {
        egsWarning("\n%s:compareEgsppAusgab: Error: Failed to open benchmark egslog file: %s\n\n",__egs_app_msg3, benchmarkFile.c_str());
        reportStream << "**FAIL** missing benchmark egslog file for comparison" << endl;
        return 1;
    }

    int numMismatches = 0;
    bool foundNewDosimetry = false, foundBenchmarkDosimetry = false;
    vector<string> newDosimetry, benchmarkDosimetry;

    // Loop through the new output and compare it with the benchmark
    string newLine, benchmarkLine;
    while(getline(newStream, newLine)) {

        // Watch for this line, just before ausgab output
        if(!foundNewDosimetry && newLine.find("Summary of region dosimetry") != std::string::npos) {
            foundNewDosimetry = true;

            // Skip two lines
            getline(newStream, newLine);
            getline(newStream, newLine);
            getline(newStream, newLine);
        }

        if(foundNewDosimetry) {
            if(newLine.find("--------------------") == std::string::npos) {
                newDosimetry.push_back(newLine);
            } else {
                foundNewDosimetry = false;
            }
        }

        if(getline(benchmarkStream, benchmarkLine)) {
            if(newLine.compare(benchmarkLine) != 0) {
                numMismatches++;
            }

            // Watch for this line, just before ausgab output
            if(!foundBenchmarkDosimetry && benchmarkLine.find("Summary of region dosimetry") != std::string::npos) {
                foundBenchmarkDosimetry = true;

                // Skip two lines
                getline(benchmarkStream, benchmarkLine);
                getline(benchmarkStream, benchmarkLine);
                getline(benchmarkStream, benchmarkLine);
            }

            if(foundBenchmarkDosimetry) {
                if(benchmarkLine.find("--------------------") == std::string::npos) {
                    benchmarkDosimetry.push_back(benchmarkLine);
                } else {
                    foundBenchmarkDosimetry = false;
                }
            }
        } else {
            numMismatches++;
        }
    }

    // If there are still more lines in the benchmark keep searching for the dosimetry
    while(getline(benchmarkStream, benchmarkLine)) {

        // Watch for this line, just before ausgab output
        if(!foundBenchmarkDosimetry && benchmarkLine.find("Summary of region dosimetry") != std::string::npos) {
            foundBenchmarkDosimetry = true;

            // Skip two lines
            getline(benchmarkStream, benchmarkLine);
            getline(benchmarkStream, benchmarkLine);
            getline(benchmarkStream, benchmarkLine);
        }

        if(!foundBenchmarkDosimetry) {
            numMismatches++;
        }

        if(foundBenchmarkDosimetry) {
            if(benchmarkLine.find("--------------------") == std::string::npos) {
                benchmarkDosimetry.push_back(benchmarkLine);
            } else {
                foundBenchmarkDosimetry = false;
            }
        }
    }

    newStream.close();
    benchmarkStream.close();

    if(numMismatches) {
        reportStream << "**NOTE** " << numMismatches << " lines did not match" << endl << endl;

        bool exactDoseMatch = true;
        int numDoseFails = 0;

        // Compare the actual dosimetry since we don't have an exact match
        unsigned int i = 0;
        for (vector<string>::iterator it = benchmarkDosimetry.begin();
                it!=benchmarkDosimetry.end(); ++it) {

            // Break if we run out of lines
            if(i >= newDosimetry.size()) {
                ++numDoseFails;
                break;
            }

            // If the strings match we can skip this line
            if((*it).compare(newDosimetry[i]) == 0) {
                ++i;
                continue;
            }
            exactDoseMatch = false;

            (*it).erase(std::remove((*it).begin(), (*it).end(), '%'), (*it).end());
            (newDosimetry[i]).erase(std::remove((newDosimetry[i]).begin(), (newDosimetry[i]).end(), '%'), (newDosimetry[i]).end());

            istringstream bStream(*it), nStream(newDosimetry[i]);

            int bRegion, nRegion;
            string bMedium, nMedium, tmp;
            double bDensity, bVolume, bEdep, bEdepUnc, bEdepUncAbs, bDose, bDoseUnc, bDoseUncAbs;
            double nDensity, nVolume, nEdep, nEdepUnc, nEdepUncAbs, nDose, nDoseUnc, nDoseUncAbs;

            // Parse the strings assuming a particular format...
            bStream >> std::skipws >> bRegion >> bMedium >> bDensity >> bVolume >> bEdep >> tmp >> bEdepUnc >> tmp >> bDose >> tmp >> bDoseUnc >> tmp;

            nStream >> std::skipws >> nRegion >> nMedium >> nDensity >> nVolume >> nEdep >> tmp >> nEdepUnc >> tmp >> nDose >> tmp >> nDoseUnc >> tmp;

            bEdepUncAbs = bEdep * 2*bEdepUnc / 100;
            nEdepUncAbs = nEdep * 2*nEdepUnc / 100;
            bDoseUncAbs = bDose * 2*bDoseUnc / 100;
            nDoseUncAbs = nDose * 2*nDoseUnc / 100;

            bool lineFailed = false;
            if(bRegion != nRegion) {
                reportStream << "**FAIL** dosimetry match due to region difference" << endl;
                lineFailed = true;
            } else if(bMedium.compare(nMedium) != 0) {
                reportStream << "**FAIL** dosimetry match due to medium difference" << endl;
                lineFailed = true;
            } else if(bDensity != nDensity) {
                reportStream << "**FAIL** dosimetry match due to density difference" << endl;
                lineFailed = true;
            } else if(bVolume != nVolume) {
                reportStream << "**FAIL** dosimetry match due to volume difference" << endl;
                lineFailed = true;
            } else if(bEdep + bEdepUncAbs < nEdep - nEdepUncAbs || bEdep - bEdepUncAbs > nEdep + nEdepUncAbs) {
                reportStream << "**FAIL** dosimetry match due to deposited energy difference beyond statistical uncertainty (2 sigma)" << endl;
                lineFailed = true;
            } else if(bDose + bDoseUncAbs < nDose - nDoseUncAbs || bDose - bDoseUncAbs > nDose + nDoseUncAbs) {
                reportStream << "**FAIL** dosimetry match due to dose difference beyond statistical uncertainty (2 sigma)" << endl;
                lineFailed = true;
            }

            if(lineFailed) {
                ++numDoseFails;
                reportStream << "See the following dosimetry region for disagreement:" << endl;
                reportStream << "```" << endl;
                reportStream << "Latest: " << newDosimetry[i] << endl;
                reportStream << "Benchm: " << *it << endl;
                reportStream << "```" << endl;
            }

            ++i;
        }

        if(exactDoseMatch) {
            reportStream << "**PASS** exact dosimetry match" << endl;
        } else {
            reportStream << "**FAIL** exact dosimetry match" << endl;
            if(!numDoseFails) {
                reportStream << "**PASS** " << benchmarkDosimetry.size() << " of " << benchmarkDosimetry.size() << " dosimetry regions agree within statistical uncertainty" << endl;
            } else {
                reportStream << "**FAIL** " << benchmarkDosimetry.size() - numDoseFails << " of " << benchmarkDosimetry.size() << " dosimetry regions agree within statistical uncertainty" << endl;
            }
        }

    } else {
        reportStream << "**PASS** exact output file match" << endl;
        reportStream << "**PASS** exact dosimetry match" << endl;
    }

    return numMismatches;
};

#ifdef BUILD_APP_LIB
APP_LIB(EGS_QualityApplication);
#else
APP_MAIN(EGS_QualityApplication);
#endif
