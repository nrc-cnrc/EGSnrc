/*
###############################################################################
#
#  EGSnrc egs++ source testing utility
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


#include "egs_input.h"
#include "egs_object_factory.h"
#include "egs_base_source.h"
#include "egs_rndm.h"

int main(int argc, char **argv) {

    if (argc < 2) {
        egsFatal("Usage: %s input_file\n",argv[0]);
    }

    EGS_Input input;
    input.setContentFromFile(argv[1]);
    EGS_ObjectFactory *factory = new EGS_ObjectFactory("egs++/dso/gcc");

    EGS_Object *o = factory->createObjects(&input,"source input",
                                           "source","main source","createSource");

    if (!o) {
        egsFatal("Got null object\n");
    }

    EGS_BaseSource *s = dynamic_cast<EGS_BaseSource *>(o);
    if (!s) {
        egsFatal("This is not a source object\n");
    }

    egsInformation("Source description: %s\n",s->getSourceDescription());

    EGS_RandomGenerator *rndm = EGS_RandomGenerator::defaultRNG();
    int q, latch;
    EGS_Float E, wt;
    EGS_Vector x,u;
    for (int j=0; j<10000; j++) {
        s->getNextParticle(rndm,q,latch,E,wt,x,u);
        egsWarning("%g %g %g\n",x.x,x.y,x.z);
    }

    delete factory;

    return 0;

}
