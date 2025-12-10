/*
###############################################################################
#
#  EGSnrc egs++ source with gaussian distribution in XY and UV
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
#   A PARTICULAR PURPOSE.  See the GNU Affero General Public License for
#  more details.
#
#  You should have received a copy of the GNU Affero General Public License
#  along with EGSnrc. If not, see <http://www.gnu.org/licenses/>.
#
###############################################################################
#
#  Author:          Marvin Apel, 2025
#
#  Contributors:
#
###############################################################################
*/


/*! \file egs_egs_focal_spot_source.cpp
 *  \brief A source with gaussian distribution for XY and UV that
 *  is an expanded version of BEAMnrc's ISOURC19
 *  \MA
 */

#include "egs_focal_spot_source.h"
#include "egs_input.h"

EGS_FocalSpot::EGS_FocalSpot(EGS_Input *input, EGS_ObjectFactory *f) :
    EGS_BaseSimpleSource(input,f), valid(true) {    
    // read required inputs
    if (input->getInput("z position",z_pos)) {
        egsWarning("EGS_FocalSpot: missing input for parameter 'z position'\n");
        valid = false;
    }
    
    if (input->getInput("spatial spread x",sigma_x_space)) {
        egsWarning("EGS_FocalSpot: missing input for parameter 'spatial spread x\n");
        valid = false;
    }

    if (input->getInput("spatial spread y",sigma_y_space)) {
        egsWarning("EGS_FocalSpot: missing input for parameter 'spatial spread y'\n");
        valid = false;
    }

    //read optional inputs for cutoff
    int err_cut_x = input->getInput("spatial cutoff x",space_cutoff_x); 
    int err_cut_y = input->getInput("spatial cutoff y",space_cutoff_y); 
    input->getInput("x translation",x_translation); 
    input->getInput("y translation",y_translation); 
    input->getInput("angular spread x",sigma_x_angle);
    input->getInput("angular spread y",sigma_y_angle);

    // if no input for spatial cutoff is available set it to 5*standard deviation
    space_cutoff_x = (err_cut_x) ? 5*sigma_x_space : space_cutoff_x;
    space_cutoff_y = (err_cut_y) ? 5*sigma_y_space : space_cutoff_y;

    if ((sigma_x_angle != 0 ) || (sigma_y_angle != 0 )) {
        is_deviating = true;
        // Set ANGLE_MODE for sampling of direction of motion (default is u.x=u.y=0, u.z=1)
        if ((sigma_x_angle != 0 ) && (sigma_y_angle != 0 )) {
            // 1 - deviation from z-axis in both directions !
            ANGLE_MODE = 1;
            }
        else if (sigma_x_angle != 0 ){
            // 2 - only deviation along x-axis
            ANGLE_MODE = 2;
        } 
        else if (sigma_y_angle != 0 ) {
            // 3 - only deviation along y-axis
            ANGLE_MODE = 3;
        }
    }

    // If z of rotation is read in test if the input aligns with the requirements ! 
    if (!input->getInput("z of rotation",z_point_of_rotation)) {
        is_rotated = true;
        if (z_point_of_rotation >= z_pos) {
            egsWarning("EGS_FocalSpot: wrong input for variable 'z of rotation'\n \
              The requirement z of rotation < z position needs to be fullfilled !\n");
            valid = false;
        }
    }

    // Test if focal spot will be rotated and if input is valid
    bool err_x = abs(input->getInput("x rotation",x_rotation));
    bool err_y = abs(input->getInput("y rotation",y_rotation));

    if ((x_rotation<=-90) || (x_rotation>=90)) {
        egsWarning("EGS_FocalSpot: wrong input for variable 'x rotation' the value should be within -90 degrees < x rotation < 90 degrees\n");
        valid = false; 
    }

    if ((y_rotation<=-90) || (y_rotation>=90)) {
        egsWarning("EGS_FocalSpot: wrong input for variable 'y rotation' the value should be within -90 degrees < y rotation < 90 degrees\n");
        valid = false; 
    }
    
    // If one of the rotations is turned on test if the z position of the rotation exists or not !
    if((!err_x || !err_y) && !is_rotated) {
        egsWarning("EGS_FocalSpot: missing input for variable 'z of rotation'\n");
        valid = false; 
    }

    setUp();
}

void EGS_FocalSpot::setUp() {
    otype = "EGS_FocalSpot";
    if (!isValid()) {
        description = "Invalid focal spot source";
    }
    else {
        description = "A focal spot of ";
        description += s->getType();
        if (q == -1) {
            description += " electrons";
        }
        else if (q == 0) {
            description += " photons";
        }
        else if (q == 1) {
            description += " positrons";
        }
        else {
            description += " an unknown particle type";
        }
        // Output Spatial Distribution Information
        description += " at a constant z  "+ to_string(z_pos) +" cm\n";
        //
        description += "   that has a spatial distribution described by a 2D Gaussian with\n";
        description += " - " + to_string(sigma_x_space) + " cm standard deviation in x  (Cutoff at: "+ to_string(space_cutoff_x) +" cm)\n";
        description += " - " + to_string(sigma_y_space) + " cm standard deviation in y  (Cutoff at: "+ to_string(space_cutoff_y) +" cm)\n";
        description += "   Around the point ("+to_string(x_translation)+"cm,"+to_string(y_translation)+"cm)\n";
        // Output Angular Distribution Information
        if (is_deviating) {
            description += "   The sampling of the direction vector determines the azimuth depending on the polar arc with \n";
            if (sigma_x_angle) description += " - " + to_string(sigma_x_angle) + " degree standard deviation along x \n";
            if (sigma_y_angle) description += " - " + to_string(sigma_y_angle) + " degree standard deviation along y \n";
            // Convert Units after reporting to log
            sigma_x_angle = sigma_x_angle*DEGREE_TO_RAD; //CONVERT UNITS
            sigma_y_angle = sigma_y_angle*DEGREE_TO_RAD; //CONVERT UNITS
        }
        else {
            description += "   The initial direction of motion is constant and pointing along the z-axis.\n";
        }
        // Output Information on Rotation of focal spot
        if (is_rotated) {
            description += "   Additionally the focal spot is rotated around the point (X,Y,Z) = (";
            description += to_string(x_translation)+","+to_string(y_translation)+","+to_string(z_point_of_rotation)+")\n";
            if (x_rotation) description += " - " + to_string(x_rotation) + " degree rotation around x \n";
            if (y_rotation) description += " - " + to_string(y_rotation) + " degree rotation around y \n";
            description += "   afterwards the particles are shifted along their new direction of motion to the z specified.\n";
            // Convert Units after reporting to log
            x_rotation = x_rotation*DEGREE_TO_RAD; //CONVERT UNITS
            y_rotation = y_rotation*DEGREE_TO_RAD; //CONVERT UNITS
            //Store Auxilarry variables to safe computational time 
            CALPHA = cos(x_rotation); 
            CBETA  = cos(y_rotation);
            SALPHA = sin(x_rotation);
            SBETA  = sin(y_rotation);
            DIST   = z_pos - z_point_of_rotation;
       }
    }
}


extern "C" {

    EGS_FOCAL_SPOT_EXPORT EGS_BaseSource *createSource(EGS_Input *input,
            EGS_ObjectFactory *f) {
        return createSourceTemplate<EGS_FocalSpot>(input,f,"focal spot");
    }

}

