/*****************************************************************************
 *
 *   Define in this file the maximum number of media (MXMED) and the
 *   maximum number of particles on the stack (MXSTACK).
 *   This file gets included by the egsnrc fortran subroutines
 *   (egsnrc_$my_machine.F), the base application (egs_simple_application.cpp
 *   or egs_advanced_application.cpp in $HEN_HOUSE/egs++), and possible by
 *   the user code, if use is made of the particle stack or one of the
 *   structures that depends on the maximum number of media.
 *
 *****************************************************************************/

#ifndef ARRAY_SIZES_
#define ARRAY_SIZES_
#define MXMED 100
#define MXSTACK 10000
#endif
