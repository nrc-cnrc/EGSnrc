###############################################################################
#
#  EGSnrc egs++ lattice geometry
#  Copyright (C) 2019 Rowan Thomson and Martin Martinov
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
#  Author:          Martin Martinov, 2019
#
#  Contributors:
#
###############################################################################
#
#  When egs_lattice is used for publications, please cite the following paper:
#
#  Martinov, Martin P., and Rowan M. Thomson. Taking EGSnrc to new lows:
#  Development of egs++ lattice geometry and testing with microscopic
#  geometries. Medical Physics 47, 3225-3232 (2020).
#
###############################################################################


include $(EGS_CONFIG)
include $(SPEC_DIR)egspp.spec
include $(SPEC_DIR)egspp_$(my_machine).conf

DEFS = $(DEF1) -DBUILD_LATTICE_DLL

library = egs_lattice
lib_files = egs_lattice

link2_libs = egspp egs_gtransformed

include $(SPEC_DIR)egspp_libs.spec

$(make_depend)

$(DSO2)egs_lattice.$(obje): ..$(DSEP)egs_gtransformed$(DSEP)egs_gtransformed.h