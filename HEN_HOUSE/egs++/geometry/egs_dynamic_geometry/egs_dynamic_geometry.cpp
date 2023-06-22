/*
###############################################################################
#
#  EGSnrc egs++ dynamic_geometry geometry
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
#  Author:          Alex Demelo, 2023
#
#  Contributors:
#
###############################################################################
*/


/*! \file egs_dynamic_geometry.cpp
 *  \brief A dynamic geometry
 *  \IK
 */

#include "egs_dynamic_geometry.h"
#include "egs_input.h"
#include "egs_functions.h"


void EGS_DynamicGeometry::setMedia(EGS_Input *,int,const int *) {
    egsWarning("EGS_DynamicGeometry::setMedia: don't use this method. Use the\n"
               " setMedia() methods of the geometry objects that make up this geometry\n");
}

void EGS_DynamicGeometry::setRelativeRho(int start, int end, EGS_Float rho) {
    setRelativeRho(0);
}

void EGS_DynamicGeometry::setRelativeRho(EGS_Input *) {
    egsWarning("EGS_DynamicGeometry::setRelativeRho(): don't use this "
               "method. Use the \n setRelativeRho() methods of the underlying "
               "geometry\n");
}

void EGS_DynamicGeometry::setBScaling(int start, int end, EGS_Float rho) {
    setBScaling(0);
}

void EGS_DynamicGeometry::setBScaling(EGS_Input *) {
    egsWarning("EGS_DynamicGeometry::setBScaling(): don't use this "
               "method. Use the \n setBScaling() methods of the underlying "
               "geometry\n");
}



extern "C" {

EGS_DYNAMIC_GEOMETRY_EXPORT EGS_BaseGeometry *createGeometry(EGS_Input *input) {
    EGS_BaseGeometry *g = 0;
    EGS_Input *ij = input->takeInputItem("geometry",false);
    if (ij) {
	g = EGS_BaseGeometry::createSingleGeometry(ij);
	delete ij;
	if (!g) {
		egsWarning("createGeometry(dynamic_geometry): got a null pointer as a geometry?\n");
		return 0;
	}
    }
    if (!g) {
	string gname;
	int err = input->getInput("my geometry",gname);
	if (err) {
		egsWarning("createGeometry(dynamic_geometry): my geometry must be defined\n either inline or using 'my geometry = some_name'\n");
		return 0;
	}
	g = EGS_BaseGeometry::getGeometry(gname);
	if (!g) {
		egsWarning("createGeometry(dynamic_geometry): no geometry named %s is defined\n",gname.c_str());
		return 0;
	}
    }
    g->ref();

    //now getting motion information for dynamic component
    EGS_Input *dyninp = input->takeInputItem("motion");
    //input file dynamic geometry definition looks something like this:
    /*:start geometry:
        name        = ...
        library     = egs_dynamic_geometry
        my geometry = predefined geometry being made to move
        :start motion:
            control point 1 = timeindex translation_X translation_Y translation_Z rotation_X rotation_Y rotation_Z
            control point 2 = ...
            ...
        :stop motion:
    :stop geometry:
    angles are in degrees and translations in cm

    note if one was using the 2 angular parameter format, the control point instead should be:
    control point 1 = timeindex translation_X translation_Y translation_Z rotation_Z rotation_X (polar and azimuthal angles)
    */

    if (dyninp) {
	EGS_DynamicGeometry *result =new EGS_DynamicGeometry(g, dyninp);
	result->setName(input);
	result->setBoundaryTolerance(input);
	result->setLabels(input);
	return result;
    }
    else {
	egsWarning("EGS_DynamicGeometry: no control points input.\n");
	return 0;
    }

}



void EGS_DynamicGeometry::getLabelRegions(const string &str, vector<int> &regs) {

    // label defined in the geometry being transformed
    g->getLabelRegions(str, regs);

    // label defined in self (transformation input block)
    EGS_BaseGeometry::getLabelRegions(str, regs);

}

void EGS_DynamicGeometry::buildDynamicGeometry(EGS_BaseGeometry *g, EGS_Input *dyninp) {
	stringstream itos;
	ncpts=0;
	vector<EGS_Float> point;
	EGS_ControlPointGeom cpt;
	int err;
	int icpts=1;
	itos << icpts;
	string sstring = "control point " + itos.str();
	int rotsize=0;
	while (!(err = dyninp->getInput(sstring,point))) {//control points read one by one from motion block in dynamic geometry definition, and saved as a vector to points
		if (point.size()!=6 && point.size()!=7) {//checking the size to make sure it is a valid control point input
			egsWarning("EGS_DynamicGeometry: control point %i must specify either 6 or 7 values.\n",icpts);
		}
		else {
			if(ncpts == 0){
				rotsize=point.size();//variable to make sure all control point definitons have consistent formats
			}
			if (ncpts>0 && point[0] < cpts[ncpts-1].time) {//make sure each time index is larger than the last
				egsWarning("EGS_DynamicGeometry: time index of control point %i < time index of control point %i\n",icpts,ncpts);
			}
			else if (point[0] < 0.) {//checks that time index is valid (larger than zero)
				egsWarning("EGS_DynamicGeometry: time index of control point %i < 0.0\n",icpts);
			}
			else if(ncpts> 0 && point.size() != rotsize){//checks that control point formats follow the first
				egsWarning("EGS_DynamicGeometry: Rotation definition inconsistent \n");
			}
			else {
				ncpts++;
				if (ncpts ==1 && point[0] > 0.0) {
				egsWarning("EGS_DynamicGeometry: time index of control point 1 > 0.0.  This will generate many warning messages.\n");
				}
				vector<EGS_Float> T_vect;//vector storing translation information
				vector<EGS_Float> R_vect;//vector storing rotation information
				//add translation coordinates to the translaton vector
				T_vect.push_back(point[1]);
				T_vect.push_back(point[2]);
				T_vect.push_back(point[3]);
                                //add rotation coordinates to rotation vector (cases differentiate cpt formats, 6 is 2 rotation parameters, 7 is 3)
                                //in each case vector order is determined by format of EGS_rotationMatrix constructor (EGS_Float arguments)
				if (point.size()==6){
					R_vect.push_back(point[6]);//rotation about z
					R_vect.push_back(point[4]);//rotation about x
				}//2 number case
				if(point.size()==7){
                                        R_vect.push_back(point[4]);//rotation about x
                                        R_vect.push_back(point[5]);//rotation about y
					R_vect.push_back(point[6]);//rotation about z
				}//3 number case
				//add it to the vector of control points
				cpt.time = point[0];
				cpt.trnsl = T_vect;
				cpt.rot = R_vect;
				cpts.push_back(cpt);//add created control point to list of control points
				icpts++;
				itos.str("");
				itos << icpts;
				sstring = "control point " + itos.str();//define next control point i string for getInput in while condition
			}
		}
	}
	if (ncpts<=1) {
		egsWarning("EGS_DynamicGeometry: not enough or missing control points.\n");
	}
	if (cpts[ncpts-1].time == 0.0) {
		egsWarning("EGS_DynamicGeometry: time index of last control point = 0.  Something's wrong.\n");
	}
	else {
		//normalize time index to max. value
		for (int i=0; i<=ncpts-1; i++) {//I changed the normalization here (and in dynamic source) to have <= instead of <. Otherwise last cpt never gets normalized, which is not an issue for final cpt time>1 but is an issue if final cpt time<1.
			cpts[i].time /= cpts[ncpts-1].time;
		}
	}

};

int EGS_DynamicGeometry::getCoordGeom(EGS_Float rand, EGS_ControlPointGeom &gipt) {//gipt is a control point struct used to track the current state of the geometry (coordinates and time)
    int iindex=0;
    int i;
    //the following loop determines which 2 control points the current time index falls between
    for (i=0; i<ncpts; i++) {
	if (rand < cpts[i].time-epsilon) { //if the control point i time index is larger than the current time index, we know this is the upper bound control point (represented by iindex)
            iindex =i;
            break;
	}
    }

    if (i==ncpts) {
	egsWarning("EGS_DynamicGeometry: could not locate control point.\n");
	return 1;
    }

    //below 3 vectors are defined, a vector containing the lower bound translation coordinates, a vector containing the upper bound translation coordinates, and a vector for the sampled translation coordinates
    vector<EGS_Float> translation_LB=cpts[iindex-1].trnsl;
    vector<EGS_Float> translation_UB=cpts[iindex].trnsl;
    vector<EGS_Float> translation_samp;

    //the following is a factor (between 0 and 1) used for sampling. Essentially, it represents the fractional position within the interval
    EGS_Float factor = (rand-cpts[iindex-1].time)/(cpts[iindex].time-cpts[iindex-1].time);

    //translations are given as the lower bound plus the length of the interval multiplied by the fractional position factor. So its essentially lower bound + n% of the interval length
    translation_samp.push_back(translation_LB[0]+(translation_UB[0]-translation_LB[0])*factor);
    translation_samp.push_back(translation_LB[1]+(translation_UB[1]-translation_LB[1])*factor);
    translation_samp.push_back(translation_LB[2]+(translation_UB[2]-translation_LB[2])*factor);
    //update the translation coordinates in the current state control point object
    gipt.trnsl=translation_samp;


    //again 3 vectors defined, lower bound, upper bound, and sampled, this time for rotation coordinates
    vector<EGS_Float> rotation_LB=cpts[iindex-1].rot;
    vector<EGS_Float> rotation_UB=cpts[iindex].rot;
    vector<EGS_Float> rotation_samp;
    //now set rotations. Current coordinates computed as before (lowerbound+n% of interval length), but now we also convert degrees to radians
    //These must be done case by case (since order of arguments matters)
    if(cpts[iindex].rot.size()==2){
	rotation_samp.push_back((rotation_LB[0]+(rotation_UB[0]-rotation_LB[0])*factor)*(M_PI/180));
	rotation_samp.push_back((rotation_LB[1]+(rotation_UB[1]-rotation_LB[1])*factor)*(M_PI/180));
    }
    else if(cpts[iindex].rot.size()==3){
	rotation_samp.push_back((rotation_LB[0]+(rotation_UB[0]-rotation_LB[0])*factor)*(M_PI/180));
	rotation_samp.push_back((rotation_LB[1]+(rotation_UB[1]-rotation_LB[1])*factor)*(M_PI/180));
	rotation_samp.push_back((rotation_LB[2]+(rotation_UB[2]-rotation_LB[2])*factor)*(M_PI/180));
    }
    else{
	egsWarning("EGS_DynamicGeometry: Invalid number of rotation parameters\n");
    }
    gipt.rot=rotation_samp;
    //update the rotation coordinates in the current state control point object
    return 0;
};

}
