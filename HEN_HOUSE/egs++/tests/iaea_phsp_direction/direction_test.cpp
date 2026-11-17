/*
###############################################################################
#
#  EGSnrc egs++ test: directions delivered by iaea_phsp_source
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

/*! \file direction_test.cpp
 *  \brief Check that iaea_phsp_source delivers unit direction vectors.
 *
 * Writes a small IAEA phase space with random directions (stored, as always, in
 * single precision), reads it back through iaea_phsp_source and reports the
 * largest deviation of |u| from 1, for the directions as stored and as delivered.
 * egs++ geometries assume unit directions; a direction off unit length by ~1e-7
 * makes intersection distances wrong by the same fraction, which causes geometry
 * errors where surfaces of different geometries coincide.
 *
 * Usage: direction_test [work directory]   (default: current directory)
 * Exit status 0 if every delivered direction has | |u| - 1 | < 1e-12.
 */

#include "egs_base_source.h"
#include "egs_input.h"
#include "egs_functions.h"
#include "egs_rndm.h"
#include "egs_vector.h"
#include "iaea_phsp.h"

#include <cmath>
#include <cstdio>
#include <random>
#include <string>

using namespace std;

int main(int argc, char **argv) {

    string dir = argc > 1 ? argv[1] : ".";
    string name = dir + "/iaea_direction_test";
    const IAEA_I32 nparticles = 100000;

    // write the phase space
    IAEA_I32 id, result, access = 2;
    iaea_new_source(&id, (char *)name.c_str(), &access, &result, (int)name.size());
    if (result < 0) {
        egsFatal("Could not create the phase space %s (IAEA error %d)\n", name.c_str(), result);
    }
    IAEA_I32 n_extra_float = 0, n_extra_int = 0;
    iaea_set_extra_numbers(&id, &n_extra_float, &n_extra_int);

    mt19937_64 rng(1);
    uniform_real_distribution<double> uni(0., 1.);
    double max_stored = 0;
    for (IAEA_I32 i = 0; i < nparticles; ++i) {
        double cost = 2*uni(rng) - 1, sint = sqrt(1 - cost*cost), phi = 2*M_PI*uni(rng);
        IAEA_I32 n_stat = 1, type = 1, extra_int = 0;
        IAEA_Float E = 0.03f, wt = 1.f, extra_float = 0.f;
        IAEA_Float x = (IAEA_Float)(0.4*uni(rng) - 0.2), y = (IAEA_Float)(0.4*uni(rng) - 0.2),
                   z = (IAEA_Float)(0.4*uni(rng) - 0.2);
        IAEA_Float u = (IAEA_Float)(sint*cos(phi)), v = (IAEA_Float)(sint*sin(phi)), w = (IAEA_Float)cost;
        double dev = fabs(sqrt((double)u*u + (double)v*v + (double)w*w) - 1);
        if (dev > max_stored) {
            max_stored = dev;
        }
        iaea_write_particle(&id, &n_stat, &type, &E, &wt, &x, &y, &z, &u, &v, &w, &extra_float, &extra_int);
    }
    IAEA_I64 n_orig = nparticles;
    iaea_set_total_original_particles(&id, &n_orig);
    iaea_destroy_source(&id, &result);

    // read it back through iaea_phsp_source
    string spec = ":start source definition:\n"
                  "    :start source:\n"
                  "        library = iaea_phsp_source\n"
                  "        name = phsp\n"
                  "        iaea phase space file = " + name + "\n"
                  "        particle type = all\n"
                  "    :stop source:\n"
                  "    simulation source = phsp\n"
                  ":stop source definition:\n";
    EGS_Input input;
    input.setContentFromString(spec);
    EGS_BaseSource *src = EGS_BaseSource::createSource(&input);
    if (!src) {
        egsFatal("Could not create the iaea_phsp_source.\n");
    }
    EGS_RandomGenerator *r = EGS_RandomGenerator::defaultRNG();
    double max_delivered = 0;
    for (IAEA_I32 i = 0; i < nparticles; ++i) {
        int q, latch;
        EGS_Float E, wt;
        EGS_Vector x, u;
        src->getNextParticle(r, q, latch, E, wt, x, u);
        double dev = fabs(sqrt(u.length2()) - 1);
        if (dev > max_delivered) {
            max_delivered = dev;
        }
    }

    bool ok = max_delivered < 1e-12;
    printf("%s: %d particles; largest | |u| - 1 |: as stored %.3g, as delivered %.3g\n",
           ok ? "PASS" : "FAIL", nparticles, max_stored, max_delivered);
    remove((name + ".IAEAphsp").c_str());
    remove((name + ".IAEAheader").c_str());
    return ok ? 0 : 1;
}
