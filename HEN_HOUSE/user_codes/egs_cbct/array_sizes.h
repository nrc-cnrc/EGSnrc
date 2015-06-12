/*
###############################################################################
#
#  EGSnrc egs++ egs_cbct application array sizes headers
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
#  Author:          Ernesto Mainegra-Hing, 2007
#
#  Contributors:
#
###############################################################################
#
#  Defines he maximum number of media (MXMED) and the maximum number of
#  particles on the stack (MXSTACK). This file gets included by the egsnrc
#  fortran subroutines (egsnrc_$my_machine.F), the base application
#  (egs_simple_application.cpp or egs_advanced_application.cpp in
#  $HEN_HOUSE/egs++), and possibly the user code, if it uses the particle
#  stack or one of the structures that depends on the maximum number of media.
#
###############################################################################
*/


#ifndef ARRAY_SIZES_
#define ARRAY_SIZES_

#define MXMED   50
#define MXSTACK 2000000
#define MXGE 2000

#endif
