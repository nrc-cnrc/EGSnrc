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

class APP_EXPORT EGS_QualityApplication : public EGS_AdvancedApplication {

public:

    /*! Constructor */
    EGS_QualityApplication(int argc, char **argv);

    /*! Destructor.  */
    ~EGS_QualityApplication();

    /*! Describe the application  */
    void describeUserCode() const;

    /*! Initialize the application */
    int initSimulation();

    /*! Initialize the back-end aspects of the application */
    int initBackEnd();

    /*! Initialize scoring  */
    int initScoring();

    /*! Accumulate quantities of interest at run time */
    int ausgab(int iarg);

    /*!  Re-implementation of runSimulation */
    int runSimulation();
    int runMiniSim();

    /*! Execute on the command line */
    string runCommand(string cmd);

    /*! Compare egs++ ausgab results */
    int compareEgsppAusgab(string newFile, string benchmarkFile);

    /*! Compare application outputs */
    int compareAppOutput(string app, string newFile, string benchmarkFile);

    /*! Get lines between two search keys */
    void getLinesBetween(string startLine, string stopLine, ifstream &newStream, ifstream &benchmarkStream, vector<string> &newResult, vector<string> &benchmarkResult);

    /*! Check if values agree within uncertainty */
    void compareLinesStat(string format, vector<string> &newResult, vector<string> &benchmarkResult);

    /*! Compare a set of lines from two files */
    int compareLinesExact(string startLine, string stopLine, ifstream &newStream, ifstream &benchmarkStream);

private:

    EGS_I64         ncase;
    EGS_Input       *input_orig;
    ofstream        reportStream;

};