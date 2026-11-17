/*
###############################################################################
#
#  EGSnrc egs++ test: egs_autoenvelope volume correction with many points
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
*/

/*! \file volcor_counts_test.cpp
 *  \brief Check the volumes an EGS_AEnvelope computes by Monte Carlo against
 *         their analytic values.
 *
 * Builds the geometry of the input file, whose simulation geometry is an
 * EGS_AEnvelope using 'discover and correct volume', and compares
 *   - the inscribed geometry volume printed in the volume correction summary, and
 *   - the corrected volume of base region 0 (EGS_AEnvelope::getVolume(0))
 * with the values given in the test block:
 *
 *   :start test:
 *       inscribed volume = 4.18879     # cm^3
 *       corrected volume = 519.410     # cm^3, of base region 0
 *       tolerance        = 1e-3        # relative
 *   :stop test:
 *
 * With more than 2^31 - 1 random points (or hits in one region) the counts used to
 * be truncated to int, which gave volumes that are wrong by large factors.
 *
 * Exit status 0 if both volumes agree within the tolerance.
 */

#include "egs_base_geometry.h"
#include "egs_input.h"
#include "egs_functions.h"

#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>

using namespace std;

static double inscribed_volume = -1;
static double npoints = -1;

static void onInformation(const char *fmt, ...) {
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    const char *p;
    if ((p = strstr(buf, "Inscribed geometry volume")) && (p = strchr(p, '='))) {
        inscribed_volume = atof(p + 1);
    }
    if ((p = strstr(buf, "Number of points used")) && (p = strchr(p, '='))) {
        npoints = atof(p + 1);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        egsFatal("Usage: %s input_file\n", argv[0]);
    }
    egsSetInfoFunction(Information, onInformation);

    EGS_Input input;
    input.setContentFromFile(argv[1]);
    EGS_BaseGeometry *g = EGS_BaseGeometry::createGeometry(&input);
    if (!g) {
        egsFatal("Got a null geometry. Check the input file.\n");
    }
    EGS_BaseGeometry::describeGeometries();   // prints the volume correction summary

    EGS_Input *ti = input.takeInputItem("test");
    if (!ti) {
        egsFatal("Missing ':start test:' block.\n");
    }
    EGS_Float v_ins, v_cor, tol = 1e-3;
    if (ti->getInput("inscribed volume", v_ins) || ti->getInput("corrected volume", v_cor)) {
        egsFatal("The test block needs 'inscribed volume' and 'corrected volume'.\n");
    }
    ti->getInput("tolerance", tol);

    double got_cor = g->getVolume(0);
    bool ok_ins = fabs(inscribed_volume - v_ins) <= tol*v_ins;
    bool ok_cor = fabs(got_cor - v_cor) <= tol*v_cor;
    printf("%-50s %s\n", argv[1], ok_ins && ok_cor ? "PASS" : "FAIL");
    printf("    points: %.4g (2^31 - 1 = 2.147e9)\n", npoints);
    printf("    inscribed volume: %.6g cm^3 (expected %.6g)\n", inscribed_volume, v_ins);
    printf("    corrected volume of base region 0: %.6g cm^3 (expected %.6g)\n", got_cor, v_cor);
    return ok_ins && ok_cor ? 0 : 1;
}
