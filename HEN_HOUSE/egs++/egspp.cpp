/*
###############################################################################
#
#  EGSnrc egs++ main program for egs++ applications
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
#  Author:          Iwan Kawrakow, 2009
#
#  Contributors:
#
###############################################################################
*/


/*! \file egspp.cpp
 *  \brief A main program for egspp applications.
 *  \IK
 */

#include "egs_application.h"
#include "egs_library.h"
#include "egs_functions.h"

#include <string>
#include <iostream>
using namespace std;

#ifdef WIN32
    const char fs = 92;
#else
    const char fs = '/';
#endif

typedef EGS_Application *(*createAppFunction)(int argc, char **argv);

/*! \brief A main program for egspp applications.

  \ingroup egspp_main

 */
int main(int argc, char **argv) {

    string app_name;
    if (!EGS_Application::getArgument(argc,argv,"-a","--application",app_name))
        egsFatal("\nUsage: %s -a application -p pegs_file [-i input_file] [-o output_file] "
                 "[-b] [-P number_of_parallel_jobs] [-j job_index]\n\n",argv[0]);

    string lib_dir;
    EGS_Application::checkEnvironmentVar(argc,argv,"-e","--egs-home","EGS_HOME",lib_dir);
    lib_dir += "bin";
    lib_dir += fs;
    lib_dir += CONFIG_NAME;
    lib_dir += fs;

    EGS_Library egs_lib(app_name.c_str(),lib_dir.c_str());
    if (!egs_lib.load()) egsFatal("\n%s: Failed to load the %s application library from %s\n\n",
                                      argv[0],app_name.c_str(),lib_dir.c_str());


    createAppFunction createApp = (createAppFunction) egs_lib.resolve("createApplication");
    if (!createApp) egsFatal("\n%s: Failed to resolve the address of the 'createApplication' function"
                                 " in the application library %s\n\n",argv[0],egs_lib.libraryFile());

    EGS_Application *app = createApp(argc,argv);
    if (!app) {
        egsFatal("\n%s: Failed to construct the application %s\n\n",argv[0],app_name.c_str());
    }

    int err = app->initSimulation();
    if (err) {
        return err;
    }
    err = app->runSimulation();
    if (err < 0) {
        return err;
    }
    err = app->finishSimulation();

    delete app;

    return err;

}


