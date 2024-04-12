/*
###############################################################################
#
#  EGSnrc egs++ dynamic shape
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
#  Author:          Reid Townson
#
#  Contributors:
#
###############################################################################
*/


/*! \file egs_dynamic_shape.cpp
 *  \brief Implementation of a dynamic shape
 *  \RT
 */

#include "egs_dynamic_shape.h"
#include "egs_input.h"
#include "egs_functions.h"
#include <sstream>

extern "C" {

    /*!
     * \brief Factory function to create an instance of EGS_DynamicShape
     * \param input Input specifying the dynamic shape
     * \param f EGS_ObjectFactory pointer
     * \return Pointer to the created EGS_DynamicShape instance
     */
    EGS_DYNAMIC_SHAPE_EXPORT EGS_BaseShape *createShape(EGS_Input *input,
            EGS_ObjectFactory *f) {
        if (!input) {
            egsWarning("createShape(dynamic shape): null input?\n");
            return 0;
        }
        EGS_Input *ishape = input->takeInputItem("shape",false);
        EGS_BaseShape *shape=0;;
        if (ishape) {
            shape = EGS_BaseShape::createShape(ishape);
            delete ishape;
        }
        if (!shape) {
            string shape_name;
            int err = input->getInput("shape name",shape_name);
            if (err) {
                egsWarning("createShape(dynamic shape): no inline shape definition"
                           " and no 'shape name' keyword\n");
                return 0;
            }
            shape = EGS_BaseShape::getShape(shape_name);
            if (!shape) {
                egsWarning("createShape(dynamic shape): no shape named %s "
                           "exists\n",shape_name.c_str());
                return 0;
            }
        }

        // Now getting motion information for dynamic component
        EGS_Input *dyninp = input->takeInputItem("motion");

        if (dyninp) {
            EGS_DynamicShape *s = new EGS_DynamicShape(shape, dyninp, "", f);
            s->setName(input);
            return s;
        }
        else {
            egsWarning("EGS_DynamicShape: no control points input.\n");
            return 0;
        }
    }

    /*!
     * \brief Builds the dynamic shape using input specifications
     * \param dyninp Input containing dynamic shape specifications
     */
    void EGS_DynamicShape::buildDynamicShape(EGS_Input *dyninp) {
        stringstream itos;
        ncpts=0;
        vector<EGS_Float> point;
        EGS_ControlPoint cpt;
        int err;
        int icpts=1;
        itos << icpts;
        string sstring = "control point " + itos.str();
        int rotsize=0;

        // Control points read one by one from motion block in dynamic geometry definition, and saved as a vector to points
        while (!(err = dyninp->getInput(sstring,point))) {

            // Checking the size to make sure it is a valid control point input
            if (point.size()!=6 && point.size()!=7) {
                egsWarning("EGS_DynamicShape: Control point %i must specify either 6 or 7 values.\n",icpts);
            }
            else {
                if (ncpts == 0) {
                    rotsize=point.size();// Variable to make sure all control point definitions have consistent formats
                }
                if (ncpts>0 && point[0] < cpts[ncpts-1].time) {// Make sure each time index is larger than the last
                    egsWarning("EGS_DynamicShape: Time index of control point %i < time index of control point %i\n",icpts,ncpts);
                }
                else if (point[0] < 0.) {// Checks that time index is valid (larger than zero)
                    egsWarning("EGS_DynamicShape: Time index of control point %i < 0.0\n",icpts);
                }
                else if (ncpts> 0 && point.size() != rotsize) { // Checks that control point formats follow the first
                    egsWarning("EGS_DynamicShape: Rotation definition inconsistent \n");
                }
                else {
                    ncpts++;

                    if (ncpts ==1 && point[0] > 0.0) {
                        egsWarning("EGS_DynamicShape: Time index of control point 1 > 0.0.  This will generate many warning messages.\n");
                    }

                    vector<EGS_Float> T_vect;// Vector storing translation information
                    vector<EGS_Float> R_vect;// Vector storing rotation information
                    // Add translation coordinates to the translation vector
                    T_vect.push_back(point[1]);
                    T_vect.push_back(point[2]);
                    T_vect.push_back(point[3]);
                    // Add rotation coordinates to rotation vector (cases differentiate cpt formats, 6 is 2 rotation parameters, 7 is 3)
                    // In each case vector order is determined by format of EGS_rotationMatrix constructor (EGS_Float arguments)
                    if (point.size()==6) {
                        R_vect.push_back(point[6]);//rotation about z
                        R_vect.push_back(point[4]);//rotation about x
                    }//2 number case
                    if (point.size()==7) {
                        R_vect.push_back(point[4]);//rotation about x
                        R_vect.push_back(point[5]);//rotation about y
                        R_vect.push_back(point[6]);//rotation about z
                    }//3 number case

                    // Add it to the vector of control points
                    cpt.time = point[0];
                    cpt.trnsl = T_vect;
                    cpt.rot = R_vect;
                    cpts.push_back(cpt);// Add created control point to list of control points
                    icpts++;
                    itos.str("");
                    itos << icpts;
                    sstring = "control point " + itos.str();// Define next control point i string for getInput in while condition
                }
            }
        }
        if (ncpts<=1) {
            egsWarning("EGS_DynamicShape: not enough or missing control points.\n");
        }
        if (cpts[ncpts-1].time == 0.0) {
            egsWarning("EGS_DynamicShape: time index of last control point = 0.  Something's wrong.\n");
        }
        else {
            // Normalize time index to max. value
            for (int i=0; i<=ncpts-1; i++) {// I changed the normalization here (and in dynamic source) to have <= instead of <. Otherwise last cpt never gets normalized, which is not an issue for final cpt time>1 but is an issue if final cpt time<1.
                cpts[i].time /= cpts[ncpts-1].time;
            }
        }
        updatePosition(0); // Sets position to initial time in egs_view upon opening
    };

    /*!
     * \brief Get the next state of the dynamic shape
     * \param rndm Random number generator
     */
    void EGS_DynamicShape::getNextShapePosition(EGS_RandomGenerator *rndm) {
        int errg = 1;
        EGS_ControlPoint gipt;

        // Here get source from activeapplication in order to extract time
        EGS_Application *app = EGS_Application::activeApplication();
        while (errg) {
            // Gets time if it's already set (otherwise gives -1).
            ptime = app->getTimeIndex();

            if (ptime<0) {
                // If no time is given by the source the shape will randomly sample from 0 to 1.
                ptime = rndm->getUniform();

                // Set randomly sampled time index for all objects in the simulation
                app->setTimeIndex(ptime);
            }

            // Now run the get coord method that will sample the cpt given to find the transformation that will be applied for the current history
            errg = getCoord(ptime,gipt);
        }

        // Create and set the current shape transformation using the sampled coordinates from getCoord. This is where overloaded EGS_AffineTransform is used
        EGS_AffineTransform *tDG = EGS_AffineTransform::getTransformation(gipt.trnsl, gipt.rot);
        shape->setTransformation(tDG);
        delete tDG;

        // Call getNextShapePosition on base shape in case there are lower level dynamic shapes
        shape->getNextShapePosition(rndm);
    };

    /*!
     * \brief Extract coordinates for the next dynamic shape position
     * \param rand Random number for time sampling
     * \param gipt EGS_ControlPoint structure to store the coordinates
     * \return 0 if successful, otherwise 1
     */
    int EGS_DynamicShape::getCoord(EGS_Float rand, EGS_ControlPoint &gipt) {
        int iindex=0;
        int i;

        // The following loop determines which 2 control points the current time index falls between
        for (i=0; i<ncpts; i++) {
            if (rand < cpts[i].time-epsilon) { // If the control point i time index is larger than the current time index, we know this is the upper bound control point (represented by iindex)
                iindex = i;
                break;
            }
        }

        if (i==ncpts) {
            egsWarning("EGS_DynamicShape: could not locate control point.\n");
            return 1;
        }

        // Below 3 vectors are defined, a vector containing the lower bound translation coordinates, a vector containing the upper bound translation coordinates, and a vector for the sampled translation coordinates
        vector<EGS_Float> translation_LB=cpts[iindex-1].trnsl;
        vector<EGS_Float> translation_UB=cpts[iindex].trnsl;
        vector<EGS_Float> translation_samp;

        // The following is a factor (between 0 and 1) used for sampling. Essentially, it represents the fractional position within the interval
        EGS_Float factor = (rand-cpts[iindex-1].time)/(cpts[iindex].time-cpts[iindex-1].time);

        // Translations are given as the lower bound plus the length of the interval multiplied by the fractional position factor. So its essentially lower bound + n% of the interval length
        translation_samp.push_back(translation_LB[0]+(translation_UB[0]-translation_LB[0])*factor);
        translation_samp.push_back(translation_LB[1]+(translation_UB[1]-translation_LB[1])*factor);
        translation_samp.push_back(translation_LB[2]+(translation_UB[2]-translation_LB[2])*factor);
        // Update the translation coordinates in the current state control point object
        gipt.trnsl=translation_samp;

        // Again 3 vectors defined, lower bound, upper bound, and sampled, this time for rotation coordinates
        vector<EGS_Float> rotation_LB=cpts[iindex-1].rot;
        vector<EGS_Float> rotation_UB=cpts[iindex].rot;
        vector<EGS_Float> rotation_samp;

        // Now set rotations. Current coordinates computed as before (lowerbound+n% of interval length), but now we also convert degrees to radians
        // These must be done case by case (since order of arguments matters)
        if (cpts[iindex].rot.size()==2) {
            rotation_samp.push_back((rotation_LB[0]+(rotation_UB[0]-rotation_LB[0])*factor)*(M_PI/180));
            rotation_samp.push_back((rotation_LB[1]+(rotation_UB[1]-rotation_LB[1])*factor)*(M_PI/180));
        }
        else if (cpts[iindex].rot.size()==3) {
            rotation_samp.push_back((rotation_LB[0]+(rotation_UB[0]-rotation_LB[0])*factor)*(M_PI/180));
            rotation_samp.push_back((rotation_LB[1]+(rotation_UB[1]-rotation_LB[1])*factor)*(M_PI/180));
            rotation_samp.push_back((rotation_LB[2]+(rotation_UB[2]-rotation_LB[2])*factor)*(M_PI/180));
        }
        else {
            egsWarning("EGS_DynamicShape: Invalid number of rotation parameters\n");
        }
        gipt.rot=rotation_samp;

        // Update the rotation coordinates in the current state control point object
        return 0;
    };

}  // extern "C"

