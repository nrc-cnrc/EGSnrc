/*
###############################################################################
#
#  EGSnrc egs++ geometry testing utility
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
#  Contributors:
#
###############################################################################
*/


/*! \file test_geometry.cpp
 *  \brief Main program for a simple geometry testing utility
 *  \IK
 *
 *  This program uses an EGS_GeometryTester object to perform
 *  tests on a geometry. Both, the geometry and the tests to be
 *  performed, are specified in an input file.
 */

#include "egs_base_geometry.h"
#include "egs_geometry_tester.h"
#include "egs_input.h"
#include "egs_functions.h"

#include <iostream>
using namespace std;

int main(int argc, char **argv) {

    if (argc < 2) {
        egsFatal("Usage: %s input_file\n",argv[0]);
    }

    EGS_Input input;
    input.setContentFromFile(argv[1]);
    //input.print(0,cout);
    EGS_BaseGeometry *g = EGS_BaseGeometry::createGeometry(&input);
    if (!g) {
        egsFatal("\nGot a null geometry? Check your input file\n\n");
    }
    EGS_BaseGeometry::describeGeometries();

    EGS_GeometryTester *tester = EGS_GeometryTester::getGeometryTester(&input);

    tester->testInside(g);
    tester->testInsideTime(g);
    tester->testHownear(20,g);
    tester->testHownearTime(g);
    tester->testHowfar(g);
    tester->testHowfarTime(g);

    delete tester;

    return 0;

}
