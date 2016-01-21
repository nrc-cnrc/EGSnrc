/*
###############################################################################
#
#  EGSnrc egs++ fortran geometry interface headers
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
#  Author:          Iwan Kawrakow, 2009
#
#  Contributors:
#
###############################################################################
*/


/* \file     egs_fortran_geometry.h
 *  \brief    A Fortran interface for using egspp geometries.
*/

#ifndef EGS_FORTRAN_GEOMETRY_
#define EGS_FORTRAN_GEOMETRY_

#include "egs_libconfig.h"

/* \brief A set of functions to use egspp geometries from Fortran (or C).

  \ingroup Geometry

*/

/* \brief Initialize an egspp geometry.

   This function initializes an egspp geometry from the definition provided
   in the file named \a file_name (string length \a flength). If the
   initialization is successful, \a igeom is set to a non-negative integer
   that later can be used to call the geometry methods of this geometry.
   If an error occures, \a igeom is set to a negative error code.
*/
extern __extc__ void EGS_EXPORT F77_OBJ_(egspp_init_geometry,EGSPP_INIT_GEOMETRY)(
    EGS_I32 *igeom, const char *file_name, int flength);

/* \brief The howfar function.

   Given position \a x, \a y, \a z, direction \a u, \a v, \a w,
   region index \a ireg, and intended step length \a ustep,
   this function checks for intersections with the geometry with index
   \a igeom. If an intersection is found, \a inew is set to the new
   region index, \a newmed to the new medium index, and \a ustep to the
   distance to the intersection. Else, \a inew is set to \a ireg.
*/
extern __extc__ void EGS_EXPORT F77_OBJ_(egspp_howfar,EGSPP_HOWFAR)(
    const EGS_I32 *igeom, const EGS_I32 *ireg,
    const EGS_Float *x, const EGS_Float *y, const EGS_Float *z,
    const EGS_Float *u, const EGS_Float *v, const EGS_Float *w,
    EGS_I32 *inew, EGS_I32 *newmed, EGS_Float *ustep);

/* \brief The hownear function.

   Given position \a x, \a y, \a z and a region index \a ireg,
   this function sets \a tperp to the minimum perpendicular
   distance to a boundary for the geometry with index \a igeom.
*/
extern __extc__ void EGS_EXPORT F77_OBJ_(egspp_hownear,EGSPP_HOWNEAR)(
    const EGS_I32 *igeom, const EGS_I32 *ireg,
    const EGS_Float *x, const EGS_Float *y, const EGS_Float *z,
    EGS_Float *tperp);

/* \brief The is_where function.

   Given position \a x, \a y, \a z, this function sets \ireg
   to the region number in geometry \a igeom (or -1 if outside).
*/
extern __extc__ void EGS_EXPORT F77_OBJ_(egspp_is_where,EGSPP_IS_WHERE)(
    const EGS_I32 *igeom,
    const EGS_Float *x, const EGS_Float *y, const EGS_Float *z,
    EGS_I32 *ireg);


/* \brief Get the number of regions in a geometry.

   Sets \a nreg to the number of regions in the geometry with index
   \a igeom.
*/
extern __extc__ void EGS_EXPORT F77_OBJ_(egspp_n_regions,EGSPP_N_REGIONS)(
    const EGS_I32 *igeom, EGS_I32 *nreg);

/* \brief Get a description of a geometry.

   This function prints information about the geometry with index
   \a igeom using egsInformation().
*/
extern __extc__ void EGS_EXPORT F77_OBJ_(egspp_describe_geometry,EGSPP_DESCRIBE_GEOMETRY)(
    const EGS_I32 *igeom);

/* \brief Get the number of media defined in all egspp geometries

   This function sets \a nmed to the total number of media definied
   in all egspp geometries constructed so far.
*/
extern __extc__ void EGS_EXPORT F77_OBJ_(egspp_n_media,EGSPP_N_MEDIA)(EGS_I32 *nmed);

/* \brief Get the name of medium \a imed

  This function puts the name of medium \a imed into the string pointed to by
  \a medname (length is \a mlength). Note that Fortran style medium indexing is
  used, i.e., first medium is 1 instead of 0.
*/
extern __extc__ void EGS_EXPORT F77_OBJ_(egspp_get_medium_name,EGSPP_GET_MEDIUM_NAME)(
    const EGS_I32 *imed, char *medname, int mlength);

#endif
