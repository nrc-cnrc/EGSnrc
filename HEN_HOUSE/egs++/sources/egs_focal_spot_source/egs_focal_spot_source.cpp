/*
###############################################################################
#
#  EGSnrc egs++ source with gaussian distribution in XY and UV
#  Copyright (C) 2025 Marvin Apel
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


/*! \file egs_focal_spot_source.cpp
 *  \brief A source with gaussian distribution for XY and UV that
 *  is an expanded version of BEAMnrc's ISOURC19
 *  \author MA
 */

#include "egs_focal_spot_source.h"
#include "egs_input.h"

static bool EGS_FOCAL_SPOT_LOCAL inputSet = false;

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
    else if (sigma_x_space <= 0) {
        egsWarning("EGS_FocalSpot: 'spatial spread x' must be positive (got %g)\n",
                   sigma_x_space);
        valid = false;
    }

    if (input->getInput("spatial spread y",sigma_y_space)) {
        egsWarning("EGS_FocalSpot: missing input for parameter 'spatial spread y'\n");
        valid = false;
    }
    else if (sigma_y_space <= 0) {
        egsWarning("EGS_FocalSpot: 'spatial spread y' must be positive (got %g)\n",
                   sigma_y_space);
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

    // guard against negative or zero cutoff values
    if (space_cutoff_x <= 0 || space_cutoff_y <= 0) {
        egsFatal("EGS_FocalSpot: spatial cutoff values must be positive "
                 "(got cutoff_x=%g, cutoff_y=%g)\n",
                 space_cutoff_x, space_cutoff_y);
    }

    angle_mode = 0;
    if ((sigma_x_angle != 0) || (sigma_y_angle != 0)) {
        is_deviating = true;
        // Set angle_mode for sampling of direction of motion (default is u.x=u.y=0, u.z=1)
        if ((sigma_x_angle != 0) && (sigma_y_angle != 0)) {
            // 1 - deviation from z-axis in both directions !
            angle_mode = 1;
        }
        else if (sigma_x_angle != 0) {
            // 2 - only deviation along x-axis
            angle_mode = 2;
        }
        else if (sigma_y_angle != 0) {
            // 3 - only deviation along y-axis
            angle_mode = 3;
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
    if ((!err_x || !err_y) && !is_rotated) {
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
            if (sigma_x_angle) {
                description += " - " + to_string(sigma_x_angle) + " degree standard deviation along x \n";
            }
            if (sigma_y_angle) {
                description += " - " + to_string(sigma_y_angle) + " degree standard deviation along y \n";
            }
            // Convert Units after reporting to log
            sigma_x_angle = sigma_x_angle*DEGREE_TO_RAD; //CONVERT UNITS
            sigma_y_angle = sigma_y_angle*DEGREE_TO_RAD; //CONVERT UNITS
        }
        else {
            description += "   The initial direction of motion is constant and pointing along the z-axis.\n";
        }
        // Output Information on Rotation of focal spot
        if (is_rotated) {
            // EGSnrc convention: positive angle = clockwise viewed from positive end of axis
            EGS_RotationMatrix R = EGS_RotationMatrix::rotX(x_rotation) * EGS_RotationMatrix::rotY(y_rotation);
            EGS_Vector pivot(0, 0, z_point_of_rotation);
            EGS_Vector t = pivot - R*pivot;
            rotation = EGS_AffineTransform(R, t);
        }
    }
}


extern "C" {

    static void setInputs() {
        inputSet = true;

        setBaseSourceInputs();

        srcBlockInput->getSingleInput("library")->setValues({"egs_focal_spot_source"});

        // Format: name, isRequired, description, vector string of allowed values
        srcBlockInput->addSingleInput("z position", true, "The z position of the source");
        srcBlockInput->addSingleInput("spatial spread x", true, "The standard deviation of the source along x");
        srcBlockInput->addSingleInput("spatial spread y", true, "The standard deviation of the source along y");
        srcBlockInput->addSingleInput("spatial cutoff x", false, "Particles will not be generated outside [x0-cutoff, x0+cutoff]");
        srcBlockInput->addSingleInput("spatial cutoff y", false, "Particles will not be generated outside [y0-cutoff, y0+cutoff]");
        srcBlockInput->addSingleInput("angular spread x", false, "The standard deviation in degrees from the z-axis toward x");
        srcBlockInput->addSingleInput("angular spread y", false, "The standard deviation in degrees from the z-axis toward y");
        srcBlockInput->addSingleInput("x translation", false, "An offset from the origin along x");
        srcBlockInput->addSingleInput("y translation", false, "An offset from the origin along y");
        srcBlockInput->addSingleInput("z of rotation", false, "The z position for the point of rotation");
        srcBlockInput->addSingleInput("x rotation", false, "A rotation clockwise when viewed from the +x axis, in degrees");
        srcBlockInput->addSingleInput("y rotation", false, "A rotation clockwise when viewed from the +y axis, in degrees");
    }

    EGS_FOCAL_SPOT_EXPORT string getExample() {
        string example;
        example = {
            R"(
    # Example of egs_focal_spot_source
    #:start source:
        library              = egs_focal_spot_source
        name                 = focal_spot_test
        z position           = 1        # cm
        spatial spread x     = 0.2      # cm standard deviation always never FWHM
        spatial spread y     = 0.2      # cm standard deviation always never FWHM
        spatial cutoff x     = 0.3      # cm particles will not be generated outside [x0-cutoff, x0+cutoff] (optional)
        spatial cutoff y     = 0.3      # cm particles will not be generated outside [y0-cutoff, y0+cutoff] (optional)
        angular spread x     = 1.5      # degrees (optional) (standard deviation from z-axis)
        angular spread y     = 0.9      # degrees (optional) (standard deviation from z-axis)
        x translation        = 0        # cm (optional)
        y translation        = 0        # cm (optional)
        z of rotation        = 0        # cm (optional)
        x rotation           = 1        # degrees, clockwise when viewed from +x axis
        y rotation           = 0        # degrees, clockwise when viewed from +y axis
        :start spectrum:
            definition of the spectrum
        :stop spectrum:
        charge = -1 or 0 or 1 for electrons or photons or positrons
    :stop source:
)"};
        return example;
    }

    EGS_FOCAL_SPOT_EXPORT shared_ptr<EGS_BlockInput> getInputs() {
        if(!inputSet) {
            setInputs();
        }
        return srcBlockInput;
    }

    EGS_FOCAL_SPOT_EXPORT EGS_BaseSource *createSource(EGS_Input *input,
            EGS_ObjectFactory *f) {
        return createSourceTemplate<EGS_FocalSpot>(input,f,"focal spot");
    }

}
