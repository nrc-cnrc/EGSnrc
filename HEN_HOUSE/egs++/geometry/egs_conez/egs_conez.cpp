/*
###############################################################################
#
#  EGSnrc egs++ conez geometry
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
#  Contributors:    Marc Chamberland
#                   Frederic Tessier
#
###############################################################################
*/


#include "egs_conez.h"
#include "egs_input.h"

string XProjector::type = "EGS_Xconez";
string YProjector::type = "EGS_Yconez";
string ZProjector::type = "EGS_Zconez";
string Projector::type = "EGS_conez";

extern "C" {

    EGS_CONEZ_EXPORT EGS_BaseGeometry *createGeometry(EGS_Input *input) {

        // valid input
        if (!input) {
            egsWarning("createGeometry(conez): null input?\n");
            return 0;
        }
        string type;
        int err = input ->getInput("type",type);
        if (err) {
            egsWarning("createGeometry(conez): missing type key\n");
            return 0;
        }

        // apex
        EGS_Vector xo;
        vector<EGS_Float> tmp;
        err = input->getInput("apex",tmp);
        if (!err && tmp.size() == 3) {
            xo=EGS_Vector(tmp[0],tmp[1],tmp[2]);
        }
        else {
            egsWarning("createGeometry(conez): invalid apex\n");
            return 0;
        }

        // opening angles
        vector<EGS_Float> angles;
        err = input->getInput("opening angles",angles);
        if (err) {
            angles.clear();
            err = input->getInput("opening angles in radian",angles);
            if (err) {
                egsWarning("createGeometry(conez): no 'opening angles' or "
                           "'opening angles in radian' input\n");
                return 0;
            }
        }

        // check valid angles
        int nc = angles.size();
        for (int j=0; j<nc; j++) {
            if (angles[j] <= 0) {
                egsWarning("createGeometry(conez): opening angles must be"
                           " positive\n");
                return 0;
            }
            if (angles[j] >= 90) {
                egsWarning("createGeometry(conez): opening angles should be "
                           "less than 90\n");
                return 0;
            }
            if (j > 0) {
                if (angles[j] <= angles[j-1]) {
                    egsWarning("createGeometry(conez): opening angles must be"
                               " in increasing order\n");
                    return 0;
                }
            }
        }
        EGS_Float *t=new EGS_Float [angles.size()];
        for (int i=0; i<nc; i++) {
            t[i]=angles[i]*M_PI/180;
        }

        // select geometry
        EGS_BaseGeometry *g;
        if (type == "EGS_Xconez") {
            g = new EGS_ConezX(nc,t,xo,"",XProjector());
        }
        else if (type == "EGS_Yconez") {
            g = new EGS_ConezY(nc,t,xo,"",YProjector());
        }
        else if (type == "EGS_Yconez") {
            g = new EGS_ConezZ(nc,t,xo,"",ZProjector());
        }
        else {
            vector<EGS_Float> a;
            err=input->getInput("axis",a);
            if (err || a.size() !=3) {
                egsWarning("createGeometry(conez): missing/wrong input\n");
                return 0;
            }
            egsWarning("got axis (%g,%g,%g)\n",a[0],a[1],a[2]);

            g = new EGS_Conez(nc,t,xo,"",
                              Projector(EGS_Vector(a[0],a[1],a[2])));
        }

        g->setName(input);
        g->setBoundaryTolerance(input);
        g->setMedia(input);
        return g;
    }
}
