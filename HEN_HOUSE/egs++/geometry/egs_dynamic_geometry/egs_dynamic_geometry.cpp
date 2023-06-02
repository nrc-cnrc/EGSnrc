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
    
    if (dyninp) {
	EGS_DynamicGeometry *result =new EGS_DynamicGeometry(g, dyninp);
	result->setName(input);
	result->setBoundaryTolerance(input);
	result->setLabels(input);
	return result;
    }
    else {
	egsWarning("EGS_DynamicGeometry: no control points input.\n");
	//valid = false;
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
	valid = true;
	stringstream itos;
	ncpts=0;
	vector<EGS_Float> point;
	EGS_ControlPointGeom cpt;
	int err;
	int icpts=1;
	itos << icpts;
	string sstring = "control point " + itos.str();
	int rotsize=0;
	while (!(err = dyninp->getInput(sstring,point))) {
		if (point.size()!=6 && point.size()!=7 && point.size()!=13) {
			egsWarning("EGS_DynamicGeometry: control point %i must specify either 6, 7 or 13 values.\n",icpts);
			valid = false;
		}
		else {
			if(ncpts == 0){
				rotsize=point.size();
			}
			if (ncpts>0 && point[0] < cpts[ncpts-1].mu) {
				egsWarning("EGS_DynamicGeometry: mu index of control point %i < mu index of control point %i\n",icpts,ncpts);
				valid = false;
			}
			else if (point[0] < 0.) {
				egsWarning("EGS_DynamicGeometry: mu index of control point %i < 0.0\n",icpts);
				valid = false;
			}
			else if(ncpts> 0 && point.size() != rotsize){
				egsWarning("EGS_DynamicGeometry: Rotation definition inconsistent \n");
				valid = false;
			}
			else {
				ncpts++;
				if (ncpts ==1 && point[0] > 0.0) {
				egsWarning("EGS_DynamicGeometry: mu index of control point 1 > 0.0.  This will generate many warning messages.\n");
				} 
				vector<EGS_Float> T_vect;
				vector<EGS_Float> R_vect;
				T_vect.push_back(point[1]);
				T_vect.push_back(point[2]);
				T_vect.push_back(point[3]);
				//cpt.trnsl = vector<EGS_Float>(point[1],point[2],point[3]); //[1] is x, [2] is y and [3] is z translation respectively
				if (point.size()>=6){
					R_vect.push_back(point[4]);
					R_vect.push_back(point[5]);
				}//2 number case
				if(point.size()>=7){
					R_vect.push_back(point[6]);
				}//3 number case
				if(point.size()==13){
					R_vect.push_back(point[7]);
					R_vect.push_back(point[8]);
					R_vect.push_back(point[9]);
					R_vect.push_back(point[10]);
					R_vect.push_back(point[11]);
					R_vect.push_back(point[12]);
				}//9 number case
				//add it to the vector of control points
				cpt.mu = point[0];
				cpt.trnsl = T_vect;
				cpt.rot = R_vect;
				cpts.push_back(cpt);
				icpts++;
				itos.str("");
				itos << icpts;
				sstring = "control point " + itos.str();
			}
		}
	}
	if (ncpts<=1) {
		egsWarning("EGS_DynamicGeometry: not enough or missing control points.\n");
		valid = false;
		//return 0;
	}
	if (cpts[ncpts-1].mu == 0.0) {
		egsWarning("EGS_DynamicGeometry: mu index of last control point = 0.  Something's wrong.\n");
		valid = false;
		//return 0;
	}
	else {
		//normalize mu index to max. value
		for (int i=0; i<ncpts-1; i++) {
			cpts[i].mu /= cpts[ncpts-1].mu;
		}
	}
	
	EGS_AffineTransform *init_transform = new EGS_AffineTransform(EGS_RotationMatrix(45,45,0),EGS_Vector(5,5,5));
	//setTransformation(*init_transform);
	
};
    
int EGS_DynamicGeometry::getCoordGeom(EGS_Float rand, EGS_ControlPointGeom &gipt) {
    valid = false;
    //egsWarning("getting geometry coordinates with mu %f \n",rand);
    int iindex=0;
    int i;
    for (i=0; i<ncpts; i++) {
	if (rand < cpts[i].mu) {
	iindex =i;
	break;
	}
    }
    
    if (i==ncpts) {
	egsWarning("EGS_DynamicSource: could not locate control point.\n");
	return 1;
    }
    vector<EGS_Float> translation_LB=cpts[iindex-1].trnsl;
    vector<EGS_Float> translation_UB=cpts[iindex].trnsl;
    vector<EGS_Float> translation_samp;
    EGS_Float factor = (rand-cpts[iindex-1].mu)/(cpts[iindex].mu-cpts[iindex-1].mu);
    
    //set translations
    translation_samp.push_back(translation_LB[0]+(translation_UB[0]-translation_LB[0])*factor);
    translation_samp.push_back(translation_LB[1]+(translation_UB[1]-translation_LB[1])*factor);
    translation_samp.push_back(translation_LB[2]+(translation_UB[2]-translation_LB[2])*factor);
    gipt.trnsl=translation_samp;
    
    
    
    vector<EGS_Float> rotation_LB=cpts[iindex-1].rot;
    vector<EGS_Float> rotation_UB=cpts[iindex].rot;
    vector<EGS_Float> rotation_samp;
    //now set rotations. These must be done case by case. for now the 9 input case is ignored
    //*(M_PI/180)
    if(cpts[iindex].rot.size()==2){
	rotation_samp.push_back((rotation_LB[0]+(rotation_UB[0]-rotation_LB[0])*factor));
	rotation_samp.push_back((rotation_LB[1]+(rotation_UB[1]-rotation_LB[1])*factor));
    }
    else if(cpts[iindex].rot.size()==3){
	rotation_samp.push_back((rotation_LB[0]+(rotation_UB[0]-rotation_LB[0])*factor));
	rotation_samp.push_back((rotation_LB[1]+(rotation_UB[1]-rotation_LB[1])*factor));
	rotation_samp.push_back((rotation_LB[2]+(rotation_UB[2]-rotation_LB[2])*factor));
    }
    else{
	egsWarning("EGS_DynamicSource: Functionality for rotation matrix definition using 9 values has not yet been enabled.\n");//for now would be best to have it crash here. Should I use egsFatal??
    }
    gipt.rot=rotation_samp;
    return 0;
};

}
