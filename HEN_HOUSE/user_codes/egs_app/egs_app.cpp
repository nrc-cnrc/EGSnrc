/*
###############################################################################
#
#  EGSnrc egs++ egs_app template application
#  Copyright (C) 2018 National Research Council Canada
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
#  Authors:         Frederic Tessier, 2018
#
#  Contributors:
#
###############################################################################
#
#  The simplest possible EGSnrc application, which relies on the implementation
#  of EGS_AdvancedApplication, to run a simulation described in an egs++ input
#  file (the provided slab.egsinp, for example).
#
#  The code commented below APP_MAIN can serve as a template for deriving an
#  application class from EGS_AdvancedApplication, and re-implementing the
#  initScoring method to select ausgab call triggers, and the ausgab method
#  to handle those calls.
#
###############################################################################
*/

#include "egs_advanced_application.h"
#include "egs_interface2.h"

// main
APP_MAIN (EGS_AdvancedApplication);




/*
class APP_EXPORT myapp : public EGS_AdvancedApplication {
    public:
        myapp(int argc, char **argv) :
            EGS_AdvancedApplication(argc,argv) {}
        int initScoring();
        int ausgab(int iarg);
};


// initScoring
int myapp::initScoring() {

    // call ausgab for all energy deposition events
    for (int call=BeforeTransport; call<=ExtraEnergy; ++call) {
        setAusgabCall((AusgabCall)call, true);
    }

    // don't call ausgab for other events
    for (int call=AfterTransport; call<UnknownCall; ++call) {
        setAusgabCall((AusgabCall)call, false);
    }

    // activate individual ausgab triggers (full list in myapplication.h). e.g.,
    setAusgabCall((AusgabCall) BeforeBrems, true);
    setAusgabCall((AusgabCall) AfterBrems,  true);

    return 0;
}


// ausgab
int myapp::ausgab(int iarg) {

    // stack index variables
    int np = the_stack->np - 1;
    int npold  = the_stack->npold - 1;
    int region = the_stack->ir[np]-2;

    // stack phase space variables
    double E = the_stack->E[np];
    double x = the_stack->x[np];
    double y = the_stack->y[np];
    double z = the_stack->z[np];
    double u = the_stack->u[np];
    double v = the_stack->v[np];
    double w = the_stack->w[np];
    double weight = the_stack->wt[np];
    int iq = the_stack->iq[np];

    // latch stack variable
    int latch  = the_stack->latch[np];

    // handle the ausgab call

    return 0;
}
*/
