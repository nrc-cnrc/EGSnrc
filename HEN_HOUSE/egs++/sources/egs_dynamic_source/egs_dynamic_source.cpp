/*
###############################################################################
#
#  EGSnrc egs++ dynamic source
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
#  Author:         Blake Walters, 2017
#
#  Contributors:    Alexandre Demelo
#
###############################################################################
*/


/*! \file egs_dynamic_source.cpp
 *  \brief A dynamic source
 *  \BW
 */

#include "egs_dynamic_source.h"
#include "egs_input.h"

EGS_DynamicSource::EGS_DynamicSource(EGS_Input *input,
                                     EGS_ObjectFactory *f) : EGS_BaseSource(input,f), source(0), valid(true) {
    EGS_Input *isource = input->takeInputItem("source",false);
    if (isource) {
        source = EGS_BaseSource::createSource(isource);
        delete isource;
    }
    if (!source) {
        string sname;
        int err = input->getInput("source name",sname);
        if (err)
            egsWarning("EGS_DynamicSource: missing/wrong inline source "
                       "definition and missing wrong 'source name' input\n");
        else {
            source = EGS_BaseSource::getSource(sname);
            if (!source) egsWarning("EGS_DynamicSource: a source named %s"
                                        " does not exist\n",sname.c_str());
        }
    }
    //now read inputs relevant to dynamic source
    //see if user wants to synchronize source with time read from
    //iaea phsp or beam simulation source
    vector<string> sync_options;
    sync_options.push_back("no");
    sync_options.push_back("yes");
    sync = input->getInput("synchronize motion",sync_options,0);
    if (sync && source->getObjectType()!="IAEA_PhspSource" &&
            source->getObjectType()!="EGS_BeamSource") {
        egsWarning("EGS_DynamicSource: source motion can only be synchronized with a source of type iaea_phsp_source or egs_beam_source.\n  Will not synchronize.\n");
        sync = false;
    }

    //get control points
    EGS_Input *dyninp = input->takeInputItem("motion");
    if (dyninp) {
        //get control points
        ncpts=0;
        vector<EGS_Float> point;
        EGS_ControlPoint cpt;
        stringstream itos;
        int err;
        int icpts=1;
        itos << icpts;

        string inputTag = "control point";
        string inputTag_backCompat = "control point " + itos.str();
        EGS_Input *currentInput;

        while (true) {
            currentInput = dyninp->takeInputItem(inputTag);
            if (!currentInput || currentInput->getInput(inputTag, point)) {
                currentInput = dyninp->takeInputItem(inputTag_backCompat);
                if (!currentInput || currentInput->getInput(inputTag_backCompat, point)) {
                    delete currentInput;
                    break;
                }
            }
            delete currentInput;

            if (point.size()!=8) {
                egsWarning("EGS_DynamicSource: control point %i does not specify 8 values.\n",icpts);
                valid = false;
            }
            else {
                if (ncpts>0 && point[7] < cpts[ncpts-1].time) {
                    egsWarning("EGS_DynamicSource: time index of control point %i < time index of control point %i\n",icpts,ncpts);
                    valid = false;
                }
                else if (point[7] < 0.) {
                    egsWarning("EGS_DynamicSource: time index of control point %i < 0.0\n",icpts);
                    valid = false;
                }
                else {
                    ncpts++;
                    if (ncpts ==1 && point[7] > 0.0) {
                        egsWarning("EGS_DynamicSource: time index of control point 1 > 0.0.  This will generate many warning messages.\n");
                    }
                    //set cpt values
                    cpt.iso = EGS_Vector(point[0],point[1],point[2]);
                    cpt.dsource = point[3];
                    cpt.theta = point[4];
                    cpt.phi = point[5];
                    cpt.phicol = point[6];
                    cpt.time = point[7];
                    //add it to the vector of control points
                    cpts.push_back(cpt);
                    icpts++;
                    itos.str("");
                    itos << icpts;
                    inputTag_backCompat = "control point " + itos.str();
                }
            }
        }
        if (ncpts<=1) {
            egsWarning("EGS_DynamicSource: not enough or missing control points.\n");
            valid = false;
        }
        if (cpts[ncpts-1].time == 0.0) {
            egsWarning("EGS_DynamicSource: time index of last control point = 0.  Something's wrong.\n");
            valid = false;
        }
        else {
            //normalize time index to max. value
            for (int i=0; i<=ncpts-1; i++) {
                cpts[i].time /= cpts[ncpts-1].time;
            }
        }
    }
    else {
        egsWarning("EGS_DynamicSource: no control points input.\n");
        valid = false;
    }
    setUp();
}

void EGS_DynamicSource::setUp() {
    //most setup done in constructor
    otype="EGS_DynamicSource";
    if (!isValid()) {
        description = "Invalid dynamic source";
    }
    else {
        description = "Dynamic source based on\n";
        description += source->getSourceDescription();
        if (sync) {
            description += "\n Source will be synched with time values read in (if available).";
        }
    }
}

//actually select the rotation coordinates for the incident particle
int EGS_DynamicSource::getCoord(EGS_Float rand, EGS_ControlPoint &ipt) {
    int iindex=0;
    int i;
    for (i=0; i<ncpts; i++) {
        if (rand < cpts[i].time-epsilon) {
            iindex =i;
            break;
        }
    }
    if (i==ncpts) {
        egsWarning("EGS_DynamicSource: could not locate control point.\n");
        return 1;
    }
    EGS_Float factor = (rand-cpts[iindex-1].time)/(cpts[iindex].time-cpts[iindex-1].time);
    ipt.iso.x=cpts[iindex-1].iso.x+ (cpts[iindex].iso.x-cpts[iindex-1].iso.x)*factor;
    ipt.iso.y=cpts[iindex-1].iso.y+ (cpts[iindex].iso.y-cpts[iindex-1].iso.y)*factor;
    ipt.iso.z=cpts[iindex-1].iso.z+ (cpts[iindex].iso.z-cpts[iindex-1].iso.z)*factor;
    ipt.dsource=cpts[iindex-1].dsource+ (cpts[iindex].dsource-cpts[iindex-1].dsource)*factor;
    ipt.theta=cpts[iindex-1].theta+ (cpts[iindex].theta-cpts[iindex-1].theta)*factor;
    ipt.phi=cpts[iindex-1].phi+ (cpts[iindex].phi-cpts[iindex-1].phi)*factor;
    ipt.phicol=cpts[iindex-1].phicol+ (cpts[iindex].phicol-cpts[iindex-1].phicol)*factor;
    return 0;
};

/**
* @brief Check if the simulation source contains time indices.
*
* @param hasdynamic Boolean flag to indicate if time indices are included in particles returned by the source.
*/
void EGS_DynamicSource::containsDynamic(bool &hasdynamic) {
    hasdynamic = true;
}

extern "C" {

    EGS_DYNAMIC_SOURCE_EXPORT EGS_BaseSource *createSource(EGS_Input *input,
            EGS_ObjectFactory *f) {
        return
            createSourceTemplate<EGS_DynamicSource>(input,f,"dynamic source");
    }

}
