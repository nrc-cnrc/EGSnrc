/*
###############################################################################
#
#  EGSnrc egs++ fortran geometry interface
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
#  Contributors:    Reid Townson
#
###############################################################################
*/

/*! \file     egs_fortran_geometry.cpp
 *  \brief    F77_OBJ_ fortran interface implementation
 *  \IK
 ***************************************************************************/

#include "egs_fortran_geometry.h"
#include "egs_base_geometry.h"
#include "egs_simple_container.h"
#include "egs_input.h"
#include "egs_functions.h"

#include <cctype>
#include <cstring>
using namespace std;

static EGS_SimpleContainer<EGS_BaseGeometry *> egspp_f_geoms;

const static char *egspp_f_error_message = "%s: Invalid geometry index %d\n";

extern __extc__ void EGS_EXPORT F77_OBJ_(egspp_init_geometry,EGSPP_INIT_GEOMETRY)(
    EGS_I32 *igeom, const char *file_name, int flength) {
    int n = flength-1;
    while (isspace(file_name[n]) && n > 0) {
        --n;
    }
    char *fname = new char [n+1];
    for (int j=0; j<n; ++j) {
        fname[j] = file_name[j];
    }
    fname[n] = '\0';
    EGS_Input input;
    input.setContentFromFile(fname);
    delete [] fname;
    EGS_BaseGeometry *g = EGS_BaseGeometry::createGeometry(&input);
    if (g) {
        *igeom = egspp_f_geoms.size();
        g->ref();
        egspp_f_geoms.add(g);
    }
    else {
        egsWarning("egspp_init_geometry: failed to create a geometry from "
                   "the input in file %s\n",fname);
        *igeom = -1;
    }
}

extern __extc__ void EGS_EXPORT F77_OBJ_(egspp_howfar,EGSPP_HOWFAR)(
    const EGS_I32 *igeom, const EGS_I32 *ireg,
    const EGS_Float *x, const EGS_Float *y, const EGS_Float *z,
    const EGS_Float *u, const EGS_Float *v, const EGS_Float *w,
    EGS_I32 *inew, EGS_I32 *newmed, EGS_Float *ustep) {
    int ig = *igeom;
    if (ig < 0 || ig >= egspp_f_geoms.size()) {
        egsFatal(egspp_f_error_message,"egspp_howfar",ig);
    }
    EGS_Vector xx(*x,*y,*z), uu(*u,*v,*w);
    *inew = egspp_f_geoms[ig]->howfar(*ireg,xx,uu,*ustep,newmed);
}

extern __extc__ void EGS_EXPORT F77_OBJ_(egspp_hownear,EGSPP_HOWNEAR)(
    const EGS_I32 *igeom, const EGS_I32 *ireg,
    const EGS_Float *x, const EGS_Float *y, const EGS_Float *z,
    EGS_Float *tperp) {
    int ig = *igeom;
    if (ig < 0 || ig >= egspp_f_geoms.size()) {
        egsFatal(egspp_f_error_message,"egspp_hownear",ig);
    }
    EGS_Vector xx(*x,*y,*z);
    *tperp = egspp_f_geoms[ig]->hownear(*ireg,xx);
}

extern __extc__ void EGS_EXPORT F77_OBJ_(egspp_is_where,EGSPP_IS_WHERE)(
    const EGS_I32 *igeom,
    const EGS_Float *x, const EGS_Float *y, const EGS_Float *z,
    EGS_I32 *ireg) {
    int ig = *igeom;
    if (ig < 0 || ig >= egspp_f_geoms.size()) {
        egsFatal(egspp_f_error_message,"egspp_is_where",ig);
    }

    EGS_Vector xx(*x,*y,*z);
    *ireg = egspp_f_geoms[ig]->isWhere(xx);
}

extern __extc__ void EGS_EXPORT F77_OBJ_(egspp_n_regions,EGSPP_N_REGIONS)(
    const EGS_I32 *igeom, EGS_I32 *nreg) {
    int ig = *igeom;
    if (ig < 0 || ig >= egspp_f_geoms.size()) {
        egsFatal(egspp_f_error_message,"egspp_n_regions",ig);
    }
    *nreg = egspp_f_geoms[ig]->regions();
}


extern __extc__ void EGS_EXPORT F77_OBJ_(egspp_describe_geometry,EGSPP_DESCRIBE_GEOMETRY)(
    const EGS_I32 *igeom) {
    int ig = *igeom;
    if (ig < 0 || ig >= egspp_f_geoms.size()) {
        egsWarning(egspp_f_error_message,"egspp_describe_geometry",ig);
        return;
    }
    egspp_f_geoms[ig]->printInfo();
}

extern __extc__ void EGS_EXPORT F77_OBJ_(egspp_n_media,EGSPP_N_MEDIA)(EGS_I32 *nmed) {
    *nmed = EGS_BaseGeometry::nMedia();
}

extern __extc__ void EGS_EXPORT F77_OBJ_(egspp_get_medium_name,EGSPP_GET_MEDIUM_NAME)(
    const EGS_I32 *imed, char *medname, int mlength) {
    const static char *vacuum = "VACUUM";
    const char *the_medium;
    int im = *imed - 1;
    if (im >= 0 && im < EGS_BaseGeometry::nMedia()) {
        the_medium = EGS_BaseGeometry::getMediumName(im);
    }
    else {
        the_medium = vacuum;
    }
    int n = strlen(the_medium);
    if (n > mlength) {
        n = mlength;
    }
    int j;
    for (j=0; j<n; ++j) {
        medname[j] = the_medium[j];
    }
    for (j=n; j<mlength; ++j) {
        medname[j] = ' ';
    }
}
