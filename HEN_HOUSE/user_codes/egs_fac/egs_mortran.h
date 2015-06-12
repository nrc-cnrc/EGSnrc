/*
###############################################################################
#
#  EGSnrc egs++ egs_fac mortran interface headers
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
#  Author:          Iwan Kawrakow, 2008
#
#  Contributors:
#
###############################################################################
*/


#ifndef EGS_MORTRAN_
#define EGS_MORTRAN_

#include "egs_libconfig.h"
#include "egs_interface2.h"
#include "array_sizes.h"
#include "egs_functions.h"

struct EGS_ExtraStack {
    EGS_Float expmfp[MXSTACK];
    int       is_fat[MXSTACK];
    EGS_Float expmfpI;
    int       is_fatI;
};

extern struct EGS_ExtraStack *the_extra_stack;

#define calculatePhotonMFP F77_OBJ_(calculate_photon_mfp,CALCULATE_PHOTON_MFP)
extern __extc__ void calculatePhotonMFP(EGS_Float *,EGS_Float *);

#define doRayleigh F77_OBJ_(do_rayleigh,DO_RAYLEIGH)
extern __extc__ void doRayleigh();

#define calculatePhotonBranching F77_OBJ_(calculate_photon_branching,CALCULATE_PHOTON_BRANCHING)
extern __extc__ void calculatePhotonBranching(EGS_Float *gbr1,EGS_Float *gbr2);

#define doPair F77_OBJ(pair,PAIR)
extern __extc__ void doPair();

#define doCompton F77_OBJ(compt,COMPT)
extern __extc__ void doCompton();

#define doPhoto F77_OBJ(photo,PHOTO)
extern __extc__ void doPhoto();

extern __extc__  void
F77_OBJ_(select_photon_mfp,SELECT_PHOTON_MFP)(EGS_Float *dpmfp);

extern __extc__ void F77_OBJ_(range_discard,RANGE_DISCARD)(
        const EGS_Float *tperp, const EGS_Float *range);

extern __extc__ void F77_OBJ_(egs_scale_xcc,EGS_SCALE_XCC)(const int *,
                                  const EGS_Float *);

#define egsScaleXsection F77_OBJ_(egs_scale_photon_xsection,EGS_SCALE_PHOTON_XSECTION)
extern __extc__ void egsScaleXsection(const int *imed, const EGS_Float *fac,
                                      const int *which);

#define computeRange F77_OBJ_(compute_range,COMPUTE_RANGE)
extern __extc__ void computeRange(const EGS_Float *eke, const EGS_Float *elke,
        const EGS_I32 *medium, const EGS_I32 *lelec, EGS_Float *range);
#endif
