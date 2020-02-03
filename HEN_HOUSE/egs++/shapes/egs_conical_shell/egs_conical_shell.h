/*
###############################################################################
#
#  EGSnrc egs++ conical shell shape headers
#  Copyright (C) 2016 Randle E. P. Taylor, Rowan M. Thomson,
#  Marc J. P. Chamberland, D. W. O. Rogers
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
#  Author:          Randle Taylor, 2016
#
#  Contributors:    Marc Chamberland
#                   Rowan Thomson
#                   Dave Rogers
#
###############################################################################
#
#  egs_conical_shell was developed for the Carleton Laboratory for
#  Radiotherapy Physics.
#
###############################################################################
*/


/*! \file egs_conical_shell.h
    \brief a conical stack shell shape
    \author Randle Taylor (randle.taylor@gmail.com)
*/

#ifndef EGS_CONICAL_SHELL_
#define EGS_CONICAL_SHELL_

#include "egs_shapes.h"
#include "egs_rndm.h"
#include "egs_math.h"
#include "egs_alias_table.h"
#include <fstream>

#ifdef WIN32

    #ifdef BUILD_CONICAL_SHELL_DLL
        #define EGS_CONICAL_SHELL_EXPORT __declspec(dllexport)
    #else
        #define EGS_CONICAL_SHELL_EXPORT __declspec(dllimport)
    #endif
    #define EGS_CONICAL_SHELL_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_CONICAL_SHELL_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_CONICAL_SHELL_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_CONICAL_SHELL_EXPORT
        #define EGS_CONICAL_SHELL_LOCAL
    #endif

#endif

/*! \brief A conical shell shape.

Samples random points within a stack of conical shells.

The input format is identical to the EGS_ConeStack geometry with the exception
that the `top radii` and `bottom radii` inputs must have 1 (outer radius only,
inner radius assumed to be 0) or 2 values (inner and outer radius).

Like the EGS_ConeStack, the `top radii` input can be omitted for layers
other than the first shell and the `top radii` will be assumed to
be equal to the `bottom radii` of the previous layer.

An example shape specification looks like:

\verbatim

:start shape:

    library = egs_conical_shell

    midpoint =  0  0 -1

    :start layer:
        thickness    = 0.5
        top radii    = 0 1
        bottom radii = 0.5 2
    :stop layer:

    :start layer:
        thickness    = 0.5
        bottom radii = 0.25 1
    :stop layer:

    :start layer:
        thickness    = 0.5
        bottom radii = 0.5
    :stop layer:

    :start layer:
        thickness    = 0.5
        bottom radii = 2
    :stop layer:

:stop shape:

\endverbatim

 * The above definition would create a shape equivalent to the `active` media
 * region of the geometry defined below:

 \verbatim

 :start geometry definition:

     :start geometry:

         library = egs_cones
         type = EGS_ConeStack
         name = stack

         axis =  0  0 -1 0 0 1

         :start layer:
             thickness    = 0.5
             top radii    = 0 1
             bottom radii = 0.5 2

             media = empty active
         :stop layer:

         :start layer:
             thickness    = 0.5
             bottom radii = 0.25 1
             media = empty active
         :stop layer:

         :start layer:
             thickness    = 0.5
             bottom radii = 0 0.5
             media = empty active
         :stop layer:

         :start layer:
             thickness    = 0.5
             bottom radii = 0 2
             media = empty active
         :stop layer:

     :stop geometry:

     simulation geometry = stack

 :stop geometry definition:

 \endverbatim


*/


struct CSSSLayer {

public:

    EGS_Float thick;
    EGS_Float ri_top, ro_top;
    EGS_Float ri_bot, ro_bot;
    EGS_Float zo;
    EGS_Float o_slope, i_slope;
    EGS_Float volume, vout, vcyl, vin;
    bool const_width;

    EGS_Float ri_min, ri_max, ro_min, ro_max;


    CSSSLayer(EGS_Float t, EGS_Float rit, EGS_Float rot, EGS_Float rib, EGS_Float rob, EGS_Float z);

    EGS_Vector getPoint(EGS_RandomGenerator *rndm);

    void getRZEqualWidth(EGS_RandomGenerator *rndm, EGS_Float &r, EGS_Float &z);

    void getRZRejection(EGS_RandomGenerator *rndm, EGS_Float &r, EGS_Float &z);

    EGS_Vector getPointInCircleAtZ(EGS_RandomGenerator *rndm, EGS_Float r, EGS_Float z);

    EGS_Float getRoAtZ(EGS_Float z);

    EGS_Float getRiAtZ(EGS_Float z);

};



class EGS_CONICAL_SHELL_EXPORT EGS_ConicalShellStackShape : public EGS_BaseShape {


public:

    /*! \brief Construct a sphere of radius \a r with midpoint \a Xo */
    EGS_ConicalShellStackShape(const EGS_Vector &Xo, const string &Name = "", EGS_ObjectFactory *f=0);

    ~EGS_ConicalShellStackShape() {
        for (size_t i = 0; i < layers.size(); i++) {
            delete layers[i];
        }
    };

    /*! \brief Returns a random point within the conical shell. */
    EGS_Vector getPoint(EGS_RandomGenerator *rndm);

    bool supportsDirectionMethod() const {
        return false;
    };

    EGS_Float area() const {
        return surface_area;
    };

    void addLayer(EGS_Float thick, EGS_Float ri_top, EGS_Float ro_top, EGS_Float ri_bot,EGS_Float ro_bot);
    void addLayer(EGS_Float thick, EGS_Float ri_bot,EGS_Float ro_bot);

protected:

    void setLayerSampler();

    vector<CSSSLayer *> layers; // inner radii of shell stack

    vector<EGS_Float> volumes;  // volume of each section of shell
    vector<EGS_Float> cum_volumes;  // cumulative volume of each section of shell (normed to 1)
    EGS_SimpleAliasTable *layer_sampler;  // cumulative volume of each section of shell (normed to 1)

    EGS_Vector xo;    // The midpoint
    EGS_Float surface_area;    // calculated total surface area of stack
    EGS_Float total_thick;    // calculated total surface area of stack

};


#endif
