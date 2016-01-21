/*
###############################################################################
#
#  EGSnrc egs++ spectrum testing utility
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

#include "egs_base_source.h"
#include "egs_rndm.h"
#include "egs_input.h"
#include "egs_timer.h"

int main(int argc, char **argv) {

    if (argc < 2) {
        egsFatal("Usage: %s input_file\n",argv[0]);
    }

    EGS_Input input;
    input.setContentFromFile(argv[1]);
    EGS_BaseSpectrum *spec = EGS_BaseSpectrum::createSpectrum(&input);
    if (!spec) {
        egsFatal("Got null source\n");
    }
    egsInformation("Got spectrum of type %s\n",spec->getType().c_str());

    EGS_RandomGenerator *rndm = EGS_RandomGenerator::defaultRNG();

    int ncase;
    int err = input.getInput("ncase",ncase);
    if (err) {
        ncase = 1000000;
    }

    EGS_Timer t;
    for (int j=0; j<ncase; j++) {
        spec->sampleEnergy(rndm);
    }
    EGS_Float cpu = t.time();
    egsInformation("CPU time: %g\n",cpu);
    spec->reportAverageEnergy();

    delete spec;
    delete rndm;
    return 0;

}

