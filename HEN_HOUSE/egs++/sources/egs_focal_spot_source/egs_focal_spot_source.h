/*
###############################################################################
#
#  EGSnrc egs++ focal spot source headers
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
#  Author:          Marvin Apel, 2025
#
#  Contributors:    -
#                                     
#
###############################################################################
*/

/*! \file egs_egs_focal_spot_source.h
 *  \brief A source with gaussian distribution for XY and UV that
 *  is an expanded version of BEAMnrc's ISOURC19
 *  \MA
 */

#ifndef EGS_FOCAL_SPOT_
#define EGS_FOCAL_SPOT_

#include "egs_base_source.h"
#include "egs_vector.h"
#include "egs_rndm.h"


#ifdef WIN32

    #ifdef BUILD_FOCAL_SPOT_DLL
        #define EGS_FOCAL_SPOT_EXPORT __declspec(dllexport)
    #else
        #define EGS_FOCAL_SPOT_EXPORT __declspec(dllimport)
    #endif
    #define EGS_FOCAL_SPOT_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_FOCAL_SPOT_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_FOCAL_SPOT_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_FOCAL_SPOT_EXPORT
        #define EGS_FOCAL_SPOT_LOCAL
    #endif

#endif

/*! \brief A focal spot source

  \ingroup Sources

A focal spot source is a special case of BEAMnrc's
ISOURCE19 which is primarily used for LINAC focal spots.
\verbatim
A simple example:
:start source:
    library              = egs_focal_spot_source
    name                 = focal_spot_test
    z position           = 1   #cm
    spatial spread x     = 0.2 #cm standard deviation always never FWHM !
    spatial spread y     = 0.2 #cm standard deviation always never FWHM !
    spatial cutoff x     = 0.3 #cm particles will not be generated outside [x0-cutoff, x0+cutoff] (optional)
    spatial cutoff y     = 0.3 #cm particles will not be generated outside [y0-cutoff, y0+cutoff] (optional)
    angular spread x     = 1.5 #degrees (optional) (standard deviation from z-axis)
    angular spread y     = 0.9 #degrees (optional) (standard deviation from z-axis)
    x translation        = 0   #cm (optional) 
    y translation        = 0   #cm (optional) 
    Z of rotation        = 0   #cm (optional) 
    x rotation           = 1   #cm (optional) 
    y rotation           = 0   #cm (optional) 
    :start spectrum:
        definition of the spectrum
    :stop spectrum:
    charge = -1 or 0 or 1 for electrons or photons or positrons
:stop source:
\endverbatim
*/

class EGS_FOCAL_SPOT_EXPORT EGS_FocalSpot : public EGS_BaseSimpleSource {
    bool valid;               //!< Is the object a valid source?
    bool is_deviating=false;  //!< The particles have an angular spread
    bool is_rotated=false;    //!< The particles are supposed to be rotated
    // The beam is always pointing in the + z-direction 
    EGS_Float  z_pos;               //!< The z-value at which the focal spot will be positioned
    EGS_Float  sigma_x_space;       //!< The std of Gaussian describing the spatial distribution in x 
    EGS_Float  sigma_y_space;       //!< The std of Gaussian describing the spatial distribution in y
    EGS_Float  space_cutoff_x;      //!< The std of Gaussian describing the spatial distribution in x 
    EGS_Float  space_cutoff_y;      //!< The std of Gaussian describing the spatial distribution in x 
    EGS_Float  sigma_x_angle=0;     //!< (optional) The std of Gaussian describing the angular distribution in x
    EGS_Float  sigma_y_angle=0;     //!< (optional) The std of Gaussian describing the angular distribution in y 
    EGS_Float  x_translation=0;     //!< (optional) Offset in x-direction
    EGS_Float  y_translation=0;     //!< (optional) Offset in y-direction
    EGS_Float  x_rotation=0;        //!< (optional) Rotating around x-axis (reference point is z_point_of_rotation)
    EGS_Float  y_rotation=0;        //!< (optional) Rotating around y-axis (reference point is z_point_of_rotation)
    EGS_Float  z_point_of_rotation; //!< (optional) Point around which rotation is done 

public:

    /*! \brief Constructor
    // All Inputs are  read in from the input file non is 
    Construct a focal spot (corresponding to ISOURCE19 in BEAMnrc) with charge \a Q, spectrum \a Spec at
    z-position \a z_pos that was offset to x \a x_translation and y \a x_translation. The gaussian
    spatial distribution is described by the standard deviation in x \a sigma_x_space and \a sigma_y_space.
    The deviation of directions of motion from the z-axis is given by the angular spread in x by \a sigma_x_angle
    and y \a sigma_y_angle. The angle corresponds to azimuth theta in spherical coordinates. Finally the point of 
    rotation \a z_point_of_rotation governs the point of rotation (needs to be at behind z position !).
    .The source object takes ownership of the spectrum.
    */
    EGS_FocalSpot(EGS_Input *, EGS_ObjectFactory *f=0);
    ~EGS_FocalSpot() {};

    void getPositionDirection(EGS_RandomGenerator *rndm, EGS_Vector &x, EGS_Vector &u, EGS_Float &wt) {
        wt = 1;
        //1. Sample position 
        do {
            x.z = sqrt(-2*log(1-rndm->getUniform())); // store value characteristic for Gaussian Sampling 
            x.y = PI2*rndm->getUniform();             // store value for phi 
            x.x = sigma_x_space * x.z * cos(x.y);
            x.y = sigma_y_space * x.z * sin(x.y);
            // Only Accept points within cutoff limits
        } while ( pow(x.x/space_cutoff_x,2) + pow(x.y/space_cutoff_y,2) > 1);
        x.x += x_translation; // safe computation time by only applying translation after finding a valid point
        x.y += y_translation; // otherwise it will be done in the while condition above repeatedly
        x.z = z_pos;


        //2. Sample direction
        switch (ANGLE_MODE) {
        case 1: // Deviation in x and y
            x.z = PI2 * rndm->getUniform(); // store value of rotation arc phi !
            do {
                u.z = sqrt(-2*pow((sigma_x_angle*sigma_y_angle), 2)/ 
                            (pow(sigma_x_angle*sin(x.z),2) + pow(sigma_y_angle*cos(x.z),2)) * (log(1- rndm->getUniform())));
                u.z = cos(u.z);
            } while (u.z < 0);

            u.x = sqrt((1-u.z)*(1+u.z)); 
            u.y = u.x * sin(x.z);
            u.x = u.x * cos(x.z);                           

            x.z = z_pos;// substitute value that held phi to z again
            break;

        case 2: // Deviation only in x, deviation is planar only along x-direction ! (u.y = 0 !!!)
            do {
                u.z = cos( sigma_x_angle * sqrt(-2*log(rndm->getUniform())) );
            } while (u.z < 0);
            u.x = sqrt((1-u.z)*(1+u.z));
            u.x = (rndm->getUniform() < 0.5) ? u.x : -1*u.x; 
            u.y = 0;
            break;

        case 3: // Deviation only in y, deviation is planar only along y-direction ! (u.x = 0 !!!)
            do {
                u.z = cos( sigma_y_angle * sqrt(-2*log(rndm->getUniform())) );
            } while (u.z < 0);
            u.y = sqrt((1-u.z)*(1+u.z));
            u.y = (rndm->getUniform() < 0.5) ? u.y : -1*u.y; 
            u.x = 0;
            break;
        
        default: // No Deviation
            u.x = 0;
            u.y = 0;
            u.z = 1;
        }
    
    //3. Rotate X, Y, Z, U, V, W and interpolate them into the z-pos plane !
    if (is_rotated) {
        // rotate position and direction vectors
        x.z  =  z_pos - DIST - x.x*CALPHA*SBETA - x.y*SALPHA + DIST*CALPHA*CBETA;
        x.y  = -x.x*SALPHA*SBETA + x.y*CALPHA + DIST*SALPHA*CBETA;
        x.x  =  x.x*CBETA + DIST*SBETA;
        TEMP = u.z;
        u.z  = -u.x*CALPHA*SBETA - u.y*SALPHA + u.z*CALPHA*CBETA;
        u.y  = -u.x*SALPHA*SBETA + u.y*CALPHA + TEMP*SALPHA*CBETA;
        u.x  =  u.x*CBETA + TEMP*SBETA;

        // SHIFT Particles along direction of motion back into plane ! 
        TEMP = (z_pos - x.z)/u.z; //u.z will never be 0 ! [limitation of x/y rotation to (-90,90)]
        x.x += TEMP * u.x;
        x.y += TEMP * u.y;
        x.z = z_pos;
    }
    };

    EGS_Float getFluence() const {
        return count;
    };

    bool storeFluenceState(ostream &) const {
        return true;
    };

    bool setFluenceState(istream &) {
        return true;
    };

    bool isValid() const {
        return (valid && s != 0);
    };

protected:
    static constexpr double PI  = 3.141592653589793;
    static constexpr double PI2 = 6.283185307179586;
    static constexpr double DEGREE_TO_RAD = 0.017453292519943295;    
    //!< governs angular sampling 
    int ANGLE_MODE;   
    //!< auxilarry variables used when beam is rotated 
    EGS_Float CALPHA; 
    EGS_Float CBETA ; 
    EGS_Float SALPHA; 
    EGS_Float SBETA;   
    EGS_Float DIST;   
    EGS_Float TEMP;   
    /*! \brief Sets up the source type and description */
    void setUp();

};

#endif

