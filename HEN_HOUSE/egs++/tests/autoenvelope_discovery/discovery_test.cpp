/*
###############################################################################
#
#  EGSnrc egs++ test: region discovery in egs_autoenvelope
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

/*! \file discovery_test.cpp
 *  \brief Check that an EGS_AEnvelope found every base region its inscribed
 *         geometries occupy.
 *
 * A base region that region discovery did not register is never tested against
 * the inscribed geometries, so a point that is inside an inscribed geometry but in
 * such a region is reported by EGS_AEnvelope::isWhere() as being in the base
 * region. This test samples points and compares isWhere() of the envelope with a
 * reference built from copies of the inscribed geometry placed at the same
 * locations (egs_gtransformed geometries listed as 'layers'):
 *
 *  - missed: inside a layer, but the envelope reports a base region
 *  - false:  inside no layer, but the envelope reports an inscribed region
 *
 * Points are drawn uniformly in each of the 'probe' boxes. A probe can be made a
 * thin slab around a sliver of the inscribed geometry that random region-discovery
 * points are unlikely to hit.
 *
 * Input: a normal ":start geometry definition:" block, whose simulation geometry is
 * the EGS_AEnvelope, plus
 *
 *   :start test:
 *       base geometry   = phantom
 *       layers          = copy1 copy2        # inscribed geometry at each location
 *       probe           = x0 y0 z0 x1 y1 z1  # one or more boxes (repeat the key)
 *       n points        = 200000             # per probe
 *       seed            = 1
 *       expect missed   = no                 # yes: the case must miss (a control)
 *       expect leak warning = no             # yes: the bounding shape must be reported
 *                                            #      as not enclosing the geometry
 *   :stop test:
 *
 * Exit status 0 if every expectation is met.
 */

#include "egs_base_geometry.h"
#include "egs_input.h"
#include "egs_functions.h"
#include "egs_vector.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <random>
#include <string>
#include <vector>

using namespace std;

static int leak_warnings = 0;
static string superset_line;

static void onWarning(const char *fmt, ...) {
    if (strstr(fmt, "does not enclose")) {
        leak_warnings++;
    }
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

static void onInformation(const char *fmt, ...) {
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    if (strstr(buf, "superset region discovery")) {
        superset_line = buf;
        while (!superset_line.empty() && superset_line.back() == '\n') {
            superset_line.pop_back();
        }
    }
}

static bool yes(EGS_Input *in, const char *key) {
    vector<string> choices;
    choices.push_back("no");
    choices.push_back("yes");
    return in->getInput(key, choices, 0) == 1;
}

int main(int argc, char **argv) {

    if (argc < 2) {
        egsFatal("Usage: %s input_file\n", argv[0]);
    }
    egsSetInfoFunction(Warning, onWarning);
    egsSetInfoFunction(Information, onInformation);

    EGS_Input input;
    input.setContentFromFile(argv[1]);
    EGS_BaseGeometry *env = EGS_BaseGeometry::createGeometry(&input);
    if (!env) {
        egsFatal("Got a null geometry. Check the input file.\n");
    }

    EGS_Input *ti = input.takeInputItem("test");
    if (!ti) {
        egsFatal("Missing ':start test:' block.\n");
    }
    string base_name;
    if (ti->getInput("base geometry", base_name)) {
        egsFatal("Missing 'base geometry' input.\n");
    }
    EGS_BaseGeometry *base = EGS_BaseGeometry::getGeometry(base_name);
    if (!base) {
        egsFatal("No geometry named '%s'.\n", base_name.c_str());
    }
    int nregbase = base->regions();

    vector<string> names;
    if (ti->getInput("layers", names) || names.empty()) {
        egsFatal("Missing 'layers' input.\n");
    }
    vector<EGS_BaseGeometry *> layers;
    for (size_t i = 0; i < names.size(); ++i) {
        EGS_BaseGeometry *g = EGS_BaseGeometry::getGeometry(names[i]);
        if (!g) {
            egsFatal("No geometry named '%s'.\n", names[i].c_str());
        }
        layers.push_back(g);
    }

    vector< vector<EGS_Float> > probes;
    EGS_Input *pi;
    while ((pi = ti->takeInputItem("probe"))) {
        vector<EGS_Float> p;
        if (pi->getInput("probe", p) || p.size() != 6) {
            egsFatal("A 'probe' needs 6 numbers: x0 y0 z0 x1 y1 z1.\n");
        }
        probes.push_back(p);
        delete pi;
    }
    if (probes.empty()) {
        egsFatal("Missing 'probe' input.\n");
    }

    EGS_I64 npoints = 200000;
    ti->getInput("n points", npoints);
    int seed = 1;
    ti->getInput("seed", seed);
    bool expect_missed = yes(ti, "expect missed");
    bool expect_leak = yes(ti, "expect leak warning");

    mt19937_64 rng(seed);
    uniform_real_distribution<EGS_Float> uni(0., 1.);
    long long n_inside = 0, n_missed = 0, n_false = 0;
    for (size_t k = 0; k < probes.size(); ++k) {
        const vector<EGS_Float> &p = probes[k];
        for (EGS_I64 n = 0; n < npoints; ++n) {
            EGS_Vector x(p[0] + (p[3]-p[0])*uni(rng), p[1] + (p[4]-p[1])*uni(rng),
                         p[2] + (p[5]-p[2])*uni(rng));
            bool inside = false;
            for (size_t l = 0; l < layers.size() && !inside; ++l) {
                inside = layers[l]->isInside(x);
            }
            int ir = env->isWhere(x);
            bool in_inscribed = ir >= nregbase;
            if (inside) {
                n_inside++;
                if (!in_inscribed) {
                    n_missed++;
                }
            }
            else if (in_inscribed) {
                n_false++;
            }
        }
    }

    bool ok = (n_missed > 0) == expect_missed && n_false == 0 && (leak_warnings > 0) == expect_leak &&
              n_inside > 0;
    printf("%-58s %s\n", argv[1], ok ? "PASS" : "FAIL");
    printf("    %s\n", superset_line.empty() ? "(no superset report)" : superset_line.c_str());
    printf("    points inside the inscribed geometry: %lld; missed: %lld (expected %s); "
           "false: %lld; leak warnings: %d (expected %s)\n",
           n_inside, n_missed, expect_missed ? "some" : "none", n_false, leak_warnings,
           expect_leak ? "some" : "none");
    return ok ? 0 : 1;
}
