/*
###############################################################################
#
#  EGSnrc egs++ spherical shell shape headers
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
#  egs_spherical_shell was developed for the Carleton Laboratory for
#  Radiotherapy Physics.
#
###############################################################################
*/


/*! \file egs_spherical_shell.h
    \brief a spherical shell shape
    \author Randle Taylor (randle.taylor@gmail.com)
*/

#ifndef EGS_SPHERICAL_SHELL_
#define EGS_SPHERICAL_SHELL_

#include "egs_shapes.h"
#include "egs_rndm.h"
#include "egs_math.h"
#include <fstream>

#ifdef WIN32

    #ifdef BUILD_SPHERICAL_SHELL_DLL
        #define EGS_SPHERICAL_SHELL_EXPORT __declspec(dllexport)
    #else
        #define EGS_SPHERICAL_SHELL_EXPORT __declspec(dllimport)
    #endif
    #define EGS_SPHERICAL_SHELL_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_SPHERICAL_SHELL_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_SPHERICAL_SHELL_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_SPHERICAL_SHELL_EXPORT
        #define EGS_SPHERICAL_SHELL_LOCAL
    #endif

#endif


/*! \brief A spherical shell shape.

Samples random points within a spherical shell.

A sample input block is given below

\verbatim
:start shape:
    library = egs_spherical_shell
    midpoint = 0, 0, 0
    inner radius = 0.5
    outer radius = 1
    hemisphere = 1  # optional
    half angle = 35 # optional
:stop shape:
\endverbatim

If `hemisphere` is 1 or -1 the shell will be truncated at the z = 0 plane with
points being sampled with positive z if `hemisphere` is 1 and negative z if
`hemisphere` is -1;

If `half angle` is specified (in degrees) the random points will be sampled within a
spherical shell truncated by a conical section with the half angle specified.
If `half angle` is negative the points will sampled with negative z coordinates.

 */
class EGS_SPHERICAL_SHELL_EXPORT EGS_SphericalShellShape : public EGS_BaseShape {

public:

    /*! \brief Construct a sphere of radius \a r with midpoint \a Xo */
    EGS_SphericalShellShape(EGS_Float ri, EGS_Float ro, int hemisph = 0, EGS_Float halfangle=0, const EGS_Vector &Xo = EGS_Vector(0,0,0),
                            const string &Name="",EGS_ObjectFactory *f=0);

    ~EGS_SphericalShellShape() { };

    /*! \brief Returns a random point within the spherical shell. */
    EGS_Vector getPoint(EGS_RandomGenerator *rndm);

    /*! \brief Returns \c true. (It is easy to implement the
     * getPointSourceDirection() method for a sphere.)
     */
    bool supportsDirectionMethod() const {
        return true;
    };

    /*! \brief Sets the direction \a u by picking a random point uniformely
     * on the sphere surface.
     * \sa EGS_BaseShape::getPointSourceDirection()
     */
    void getPointSourceDirection(const EGS_Vector &Xo,
                                 EGS_RandomGenerator *rndm, EGS_Vector &u, EGS_Float &wt);

    /*! \brief Returns the sphere surface area.*/
    EGS_Float area() const;

protected:

    EGS_Float  r_inner, r_outer, sgn;     //!< The sphere radius
    int hemisphere;
    EGS_Float  half_angle;     //!< Half angle of conical section
    EGS_Vector xo;    //!< The sphere midpoint

};


#endif
