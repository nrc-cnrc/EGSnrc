/*
###############################################################################
#
#  EGSnrc egs++ test: navigation through composite geometries whose
#  components have coincident surfaces
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

/*! \file coincident_test.cpp
 *  \brief Ray-trace a composite geometry with howfar() the way the transport
 *         loop does, and compare the path length per medium against a
 *         reference built from the component geometries traced on their own.
 *
 * The reference does not call isWhere() at a boundary of the composite. Each
 * component ("layer") is traced alone, from a start point that is not on any
 * surface, and the medium at a given distance along the ray is that of the
 * first layer (innermost first) the ray is inside of at that distance.
 *
 * Input: a normal ":start geometry definition:" block plus
 *
 *   :start test:
 *       layers       = seed catheter phantom   # innermost first
 *       n rays       = 1000000
 *       seed         = 1
 *       centre       = 0 0 0
 *       start radius = 1.0     # outside rays start on this sphere
 *       target box   = 0.08 0.08 0.5   # half-lengths around centre
 *       inside fraction = 0.5  # fraction of rays started inside the box
 *       max distance = 3.0
 *       tolerance    = 1e-7    # path-length mismatch that counts as wrong
 *       dump         = 5       # print this many failing rays
 *       ray          = x y z u v w     # optional: trace one ray verbosely
 *   :stop test:
 */

#include "egs_base_geometry.h"
#include "egs_input.h"
#include "egs_functions.h"
#include "egs_vector.h"

#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <map>
#include <random>
#include <string>
#include <vector>

using namespace std;

struct Segment {
    EGS_Float t0, t1;
    int ireg, med;
};

struct TraceResult {
    vector<Segment> segs;
    int n_flagged;     // howfar calls that set the geometry error flag
    int n_zero;        // steps of exactly zero length
    int max_zero_run;  // longest run of consecutive zero-length steps
    bool stuck;        // gave up after too many steps
    TraceResult() : n_flagged(0), n_zero(0), max_zero_run(0), stuck(false) {}
};

static map<string,long long> warning_count;
static bool quiet_warnings = true;

static void countWarning(const char *fmt, ...) {
    const char *p = strstr(fmt, "::howfar");
    if (p) {
        warning_count[string(fmt, p - fmt)]++;
    }
    if (!quiet_warnings) {
        va_list ap;
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
    }
}

// Follow one ray through g exactly as the transport loop would: ask howfar,
// move by the returned distance, take the returned region.
static TraceResult trace(EGS_BaseGeometry *g, const EGS_Vector &x0,
                         const EGS_Vector &u, EGS_Float tmax, bool verbose=false) {
    TraceResult r;
    EGS_Vector x(x0);
    int ireg = g->isWhere(x);
    EGS_Float ttot = 0;
    int zero_run = 0;
    const int max_steps = 100000;
    for (int step = 0; step < max_steps; ++step) {
        EGS_Float t = tmax - ttot;
        if (t <= 0) {
            return r;
        }
        int newmed = -1;
        EGS_BaseGeometry::resetErrorFlag();
        int inew = g->howfar(ireg, x, u, t, &newmed);
        bool flagged = EGS_BaseGeometry::getLastError();
        if (flagged) {
            r.n_flagged++;
        }
        if (verbose) {
            EGS_Float rho = sqrt(x.x*x.x + x.y*x.y);
            printf("  step %3d ttot=%.15g ireg=%d med=%d x=(%.15g,%.15g,%.15g) rho=%.17g -> t=%.17g inew=%d%s\n",
                   step, ttot, ireg, ireg >= 0 ? g->medium(ireg) : -2,
                   x.x, x.y, x.z, rho, t, inew, flagged ? "  ** ERROR FLAG" : "");
        }
        Segment s;
        s.t0 = ttot;
        s.t1 = ttot + t;
        s.ireg = ireg;
        s.med = ireg >= 0 ? g->medium(ireg) : -2;
        r.segs.push_back(s);
        if (t == 0) {
            r.n_zero++;
            if (++zero_run > r.max_zero_run) {
                r.max_zero_run = zero_run;
            }
        }
        else {
            zero_run = 0;
        }
        x += u*t;
        ttot += t;
        if (inew == ireg) {
            // no boundary within the remaining distance (or, from outside,
            // the ray never enters)
            return r;
        }
        ireg = inew;
    }
    r.stuck = true;
    return r;
}

// path length per medium; index 0 is "outside everything" (-2), 1 is
// vacuum (-1), medium m is at m+2
static void accumulate(const vector<Segment> &segs, vector<EGS_Float> &len) {
    for (size_t i = 0; i < segs.size(); ++i) {
        len[segs[i].med + 2] += segs[i].t1 - segs[i].t0;
    }
}

static void reference(const vector<EGS_BaseGeometry *> &layers, const EGS_Vector &x0,
                      const EGS_Vector &u, EGS_Float tmax, vector<EGS_Float> &len) {
    vector< vector<Segment> > tr(layers.size());
    vector<EGS_Float> brk;
    brk.push_back(0);
    brk.push_back(tmax);
    for (size_t l = 0; l < layers.size(); ++l) {
        tr[l] = trace(layers[l], x0, u, tmax).segs;
        for (size_t i = 0; i < tr[l].size(); ++i) {
            brk.push_back(tr[l][i].t0);
            brk.push_back(tr[l][i].t1);
        }
    }
    sort(brk.begin(), brk.end());
    for (size_t b = 0; b + 1 < brk.size(); ++b) {
        EGS_Float a = brk[b], c = brk[b+1];
        if (c <= a || a >= tmax) {
            continue;
        }
        if (c > tmax) {
            c = tmax;
        }
        EGS_Float mid = 0.5*(a + c);
        int med = -2;
        for (size_t l = 0; l < layers.size() && med == -2; ++l) {
            for (size_t i = 0; i < tr[l].size(); ++i) {
                if (tr[l][i].ireg >= 0 && mid > tr[l][i].t0 && mid < tr[l][i].t1) {
                    med = tr[l][i].med;
                    break;
                }
            }
        }
        len[med + 2] += c - a;
    }
}

static const char *medName(int idx) {
    if (idx == 0) {
        return "(outside)";
    }
    if (idx == 1) {
        return "(vacuum)";
    }
    return EGS_BaseGeometry::getMediumName(idx - 2);
}

int main(int argc, char **argv) {

    if (argc < 2) {
        egsFatal("Usage: %s input_file\n", argv[0]);
    }

    EGS_Input input;
    input.setContentFromFile(argv[1]);
    EGS_BaseGeometry *g = EGS_BaseGeometry::createGeometry(&input);
    if (!g) {
        egsFatal("Got a null geometry. Check the input file.\n");
    }

    EGS_Input *ti = input.takeInputItem("test");
    if (!ti) {
        egsFatal("Missing ':start test:' block.\n");
    }

    vector<string> layer_names;
    if (ti->getInput("layers", layer_names) || layer_names.empty()) {
        egsFatal("Missing 'layers' input.\n");
    }
    vector<EGS_BaseGeometry *> layers;
    for (size_t i = 0; i < layer_names.size(); ++i) {
        EGS_BaseGeometry *lg = EGS_BaseGeometry::getGeometry(layer_names[i]);
        if (!lg) {
            egsFatal("No geometry named '%s'.\n", layer_names[i].c_str());
        }
        layers.push_back(lg);
    }

    EGS_I64 nrays = 100000;
    ti->getInput("n rays", nrays);
    int seed = 1;
    ti->getInput("seed", seed);
    vector<EGS_Float> centre(3, 0.), box(3, 0.1);
    ti->getInput("centre", centre);
    ti->getInput("target box", box);
    EGS_Float rstart = 1, tmax = 3, tol = 1e-7, f_inside = 0.5;
    ti->getInput("start radius", rstart);
    ti->getInput("max distance", tmax);
    ti->getInput("tolerance", tol);
    ti->getInput("inside fraction", f_inside);
    int ndump = 5;
    ti->getInput("dump", ndump);
    vector<EGS_Float> one_ray;
    bool have_ray = !ti->getInput("ray", one_ray) && one_ray.size() == 6;

    int nmed = EGS_BaseGeometry::nMedia() + 2;

    if (have_ray) {
        quiet_warnings = false;
        EGS_Vector x(one_ray[0], one_ray[1], one_ray[2]);
        EGS_Vector u(one_ray[3], one_ray[4], one_ray[5]);
        printf("composite trace:\n");
        TraceResult r = trace(g, x, u, tmax, true);
        for (size_t l = 0; l < layers.size(); ++l) {
            printf("layer '%s' alone:\n", layer_names[l].c_str());
            trace(layers[l], x, u, tmax, true);
        }
        vector<EGS_Float> lc(nmed, 0.), lr(nmed, 0.);
        accumulate(r.segs, lc);
        reference(layers, x, u, tmax, lr);
        for (int m = 0; m < nmed; ++m) {
            printf("  %-20s composite=%.12g reference=%.12g diff=%.3g\n",
                   medName(m), lc[m], lr[m], lc[m] - lr[m]);
        }
        return 0;
    }

    egsSetInfoFunction(Warning, countWarning);

    mt19937_64 rng(seed);
    uniform_real_distribution<EGS_Float> uni(0., 1.);

    long long n_flag_rays = 0, n_flag_calls = 0, n_wrong = 0, n_wrong_silent = 0;
    long long n_zero_rays = 0, n_stuck = 0;
    int max_zero_run = 0;
    EGS_Float max_diff = 0, sum_abs_diff = 0;
    vector<EGS_Float> net_diff(nmed, 0.);
    int dumped = 0;

    for (EGS_I64 n = 0; n < nrays; ++n) {
        // a random point in the target box
        EGS_Vector p(centre[0] + box[0]*(2*uni(rng) - 1),
                     centre[1] + box[1]*(2*uni(rng) - 1),
                     centre[2] + box[2]*(2*uni(rng) - 1));
        // an isotropic direction
        EGS_Float cost = 2*uni(rng) - 1, sint = sqrt(1 - cost*cost);
        EGS_Float phi = 2*M_PI*uni(rng);
        EGS_Vector u(sint*cos(phi), sint*sin(phi), cost);
        EGS_Vector x;
        if (uni(rng) < f_inside) {
            x = p;                   // start in the box, like a decay
        }
        else {
            x = p - u*rstart;        // start outside, aimed through the box
        }

        TraceResult r = trace(g, x, u, tmax);
        vector<EGS_Float> lc(nmed, 0.), lr(nmed, 0.);
        accumulate(r.segs, lc);
        reference(layers, x, u, tmax, lr);

        EGS_Float worst = 0;
        for (int m = 0; m < nmed; ++m) {
            EGS_Float d = fabs(lc[m] - lr[m]);
            if (d > worst) {
                worst = d;
            }
        }
        bool wrong = worst > tol;
        if (r.n_flagged) {
            n_flag_rays++;
            n_flag_calls += r.n_flagged;
        }
        if (wrong) {
            n_wrong++;
            sum_abs_diff += worst;
            if (!r.n_flagged) {
                n_wrong_silent++;
            }
            if (worst > max_diff) {
                max_diff = worst;
            }
            for (int m = 0; m < nmed; ++m) {
                net_diff[m] += lc[m] - lr[m];
            }
        }
        if (r.n_zero) {
            n_zero_rays++;
        }
        if (r.max_zero_run > max_zero_run) {
            max_zero_run = r.max_zero_run;
        }
        if (r.stuck) {
            n_stuck++;
        }
        if ((wrong || r.stuck || r.n_flagged) && dumped < ndump) {
            ++dumped;
            printf("FAIL ray = %.17g %.17g %.17g %.17g %.17g %.17g  flagged=%d worst=%.3g%s\n",
                   x.x, x.y, x.z, u.x, u.y, u.z, r.n_flagged, worst, r.stuck ? " STUCK" : "");
        }
    }

    printf("\n=== %s ===\n", argv[1]);
    printf("rays                                  %lld\n", (long long) nrays);
    printf("rays with a flagged geometry error    %lld  (%.3g per ray)\n",
           n_flag_rays, (double) n_flag_rays/nrays);
    printf("flagged howfar calls                  %lld\n", n_flag_calls);
    printf("rays with wrong path per medium       %lld  (%.3g per ray)\n",
           n_wrong, (double) n_wrong/nrays);
    printf("  of which not flagged (silent)       %lld\n", n_wrong_silent);
    printf("  largest / mean wrong path (cm)      %.4g / %.4g\n",
           max_diff, n_wrong ? sum_abs_diff/n_wrong : 0.);
    printf("rays with a zero-length step          %lld  (longest run %d)\n",
           n_zero_rays, max_zero_run);
    printf("rays that never finished              %lld\n", n_stuck);
    printf("net path error by medium, composite - reference (cm, wrong rays only):\n");
    for (int m = 0; m < nmed; ++m) {
        if (net_diff[m] != 0) {
            printf("  %-20s %+.6g\n", medName(m), net_diff[m]);
        }
    }
    printf("warnings by class:\n");
    for (map<string,long long>::iterator it = warning_count.begin(); it != warning_count.end(); ++it) {
        printf("  %-20s %lld\n", it->first.c_str(), it->second);
    }

    // a non-zero exit status makes this usable as a regression test
    return (n_flag_rays || n_wrong || n_stuck) ? 1 : 0;
}
