/*
###############################################################################
#
#  EGSnrc simple C interface v1
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
#  Author:          Iwan Kawrakow, 2003
#
#  Contributors:    Ernesto Mainegra-Hing
#
###############################################################################
#
#  To use it:
#
#  - compile EGSnrc using the set of macros egs_c_interface2.macros
#  - implement a main, howfar, hownear and ausgab
#  - link EGSnrc, this file and your implementation together
#
###############################################################################
*/


#include "egs_interface2.h"

static int n_arg = 0;
static char **args = 0;

extern __extc__ struct EGS_Stack F77_OBJ(stack,STACK);
extern __extc__ struct EGS_Bounds F77_OBJ(bounds,BOUNDS);
extern __extc__ struct EGS_Thresh F77_OBJ(thresh,THRESH);
extern __extc__ struct EGS_Epcont F77_OBJ(epcont,EPCONT);
extern __extc__ struct EGS_EtControl F77_OBJ_(et_control,ET_CONTROL);
extern __extc__ struct EGS_Media F77_OBJ(media,MEDIA);
extern __extc__ struct EGS_Useful F77_OBJ(useful,USEFUL);
extern __extc__ struct EGS_XOptions F77_OBJ_(xsection_options,XSECTION_OPTIONS);
extern __extc__ struct EGS_IO F77_OBJ_(egs_io,EGS_IO);
extern __extc__ struct EGS_VarianceReduction F77_OBJ_(egs_vr,EGS_VR);
extern __extc__ struct EGS_Rayleigh F77_OBJ_(rayleigh_inputs,RAYLEIGH_INPUTS);

struct EGS_Stack        *the_stack     = & F77_OBJ(stack,STACK);
struct EGS_Bounds       *the_bounds    = & F77_OBJ(bounds,BOUNDS);
struct EGS_Thresh       *the_thresh    = & F77_OBJ(thresh,THRESH);
struct EGS_Epcont       *the_epcont    = & F77_OBJ(epcont,EPCONT);
struct EGS_EtControl    *the_etcontrol = & F77_OBJ_(et_control,ET_CONTROL);
struct EGS_Media        *the_media     = & F77_OBJ(media,MEDIA);
struct EGS_Useful       *the_useful    = & F77_OBJ(useful,USEFUL);
struct EGS_XOptions     *the_xoptions  = & F77_OBJ_(xsection_options,XSECTION_OPTIONS);
struct EGS_IO           *the_egsio     = & F77_OBJ_(egs_io,EGS_IO);
struct EGS_VarianceReduction *the_egsvr = & F77_OBJ_(egs_vr,EGS_VR);
struct EGS_Rayleigh     *the_rayleigh   = & F77_OBJ_(rayleigh_inputs,RAYLEIGH_INPUTS);

extern __extc__ void F77_OBJ_(egs_init_f,EGS_INIT_F)();
extern __extc__ void F77_OBJ(electr,ELECTR)(int *);
extern __extc__ void F77_OBJ(photon,PHOTON)(int *);

void egsInit(int argc, char **argv) {
  n_arg = argc; args = argv;
  F77_OBJ_(egs_init_f,EGS_INIT_F)();
}

extern __extc__ void F77_OBJ_(egs_iargc,EGS_IARGC)(int *n) { *n = n_arg; }

extern __extc__ void F77_OBJ_(egs_getarg,EGS_GETARG)(int *ii, char *arg, int len) {
  int j; int i = *ii;
  for(j=0; j<len; j++) arg[j] = ' ';
  if( i >= n_arg ) return;
  for(j=0; j<len; j++) {
    if( !args[i][j] ) break;
    arg[j] = args[i][j];
  }
}

extern __extc__ void egsShower() {
  int np = the_stack->np-1, ircode;
  while (np >= 0 ) {
    if( the_stack->iq[np] ) F77_OBJ(electr,ELECTR)(&ircode);
    else                    F77_OBJ(photon,PHOTON)(&ircode);
    np = the_stack->np-1;
  }
}
