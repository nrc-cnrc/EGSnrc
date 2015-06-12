/*
###############################################################################
#
#  EGSnrc egs++ egs_cbct mortran interface headers
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
#  Author:          Ernesto Mainegra-Hing, 2008
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

#ifdef DEBUG_WEIGHTS
struct EGS_ExtraStack {
    EGS_Float ddet[MXSTACK];
    EGS_I32   iphat[MXSTACK];
    EGS_I32   irdet[MXSTACK];
    EGS_I32   inter[MXSTACK];
    EGS_Float ddetI;
    EGS_I32   iphatI,irdetI,interI;
};
#else
/*
   EGS_ExtraStack MUST match COMIN STACK-EXTRA
   defined in egs_cbct.macros
*/
struct EGS_ExtraStack {
    EGS_Float katt[MXSTACK];
    EGS_Float ddet[MXSTACK];
    EGS_I32   iphat[MXSTACK];
    EGS_I32   irdet[MXSTACK];
    EGS_Float kattI, ddetI;
    EGS_I32   iphatI,irdetI;
};
#endif

#ifdef USTEP_DEBUG
#define MXSTEP 10000
struct EGS_GeometryDebug {
    long long icase;
    float x_debug[MXSTEP],y_debug[MXSTEP],z_debug[MXSTEP];
    float u_debug[MXSTEP],v_debug[MXSTEP],w_debug[MXSTEP];
    float ustep_debug[MXSTEP],dnear_debug[MXSTEP];
    int   ir_debug[MXSTEP],irnew_debug[MXSTEP],np_debug[MXSTEP];
    int   nstep;
};
extern __extc__ struct EGS_GeometryDebug
                 F77_OBJ_(geometry_debug,GEOMETRY_DEBUG);
struct EGS_GeometryDebug* the_geometry_debug =
                &F77_OBJ_(geometry_debug,GEOMETRY_DEBUG);
#endif

extern __extc__ void F77_OBJ_(range_discard,RANGE_DISCARD)(
        const EGS_Float *tperp, const EGS_Float *range);

#define calculatePhotonMFP F77_OBJ_(calculate_photon_mfp,CALCULATE_PHOTON_MFP)
extern __extc__ void calculatePhotonMFP(EGS_Float *,EGS_Float *);

#define doRayleigh F77_OBJ_(do_rayleigh,DO_RAYLEIGH)
extern __extc__ void doRayleigh();

#define calculatePhotonBranching F77_OBJ_(calculate_photon_branching,CALCULATE_PHOTON_BRANCHING)
extern __extc__ void calculatePhotonBranching(EGS_Float *gbr1,EGS_Float *gbr2);

extern __extc__ void F77_OBJ(pair,PAIR)();

extern __extc__ void F77_OBJ(compt,COMPT)();

extern __extc__ void F77_OBJ(photo,PHOTO)();

#define egsOpenUnits F77_OBJ_(egs_open_units,EGS_OPEN_UNITS)
extern __extc__ void egsOpenUnits(const EGS_I32 *);

extern __extc__ struct EGS_ExtraStack F77_OBJ_(extra_stack,EXTRA_STACK);
static struct EGS_ExtraStack *the_extra_stack = &F77_OBJ_(extra_stack,EXTRA_STACK);

extern __extc__  void
F77_OBJ_(select_photon_mfp,SELECT_PHOTON_MFP)(EGS_Float *dpmfp);

extern __extc__ void F77_OBJ_(egs_scale_xcc,EGS_SCALE_XCC)(const int *,
                                  const EGS_Float *);
#define egsScaleXsection F77_OBJ_(egs_scale_photon_xsection,EGS_SCALE_PHOTON_XSECTION)
extern __extc__ void egsScaleXsection(const int *imed, const EGS_Float *fac,
                                      const int *which);
#endif
