###############################################################################
#
#  EGSnrc egs++ lattice geometry
#  Copyright (C) 2019 Rowan Thomson and Martin Martinov
#
#  This file is part of EGSnrc.
#
#  EGSnrc is free software: you can redistribute it and/or modify it under
#  the terms of the GNU Affero General Public License as published by the Free
#  Software Foundation, either version 3 of the License, or (at your option)
#  any later version.
#
#  EGSnrc is distributed in the hope that it will be useful, but WITHOUT
#  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
#  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License
#  for more details.
#
#  To see the GNU Affero General Public License at:
# <http://www.gnu.org/licenses/>.
#
################################################################################
#
#  When egs_lattice is used for publications, please cite the following paper:
#  Manuscript submission underway, to be replaced with title after publication
#
###############################################################################
#
#  Author:          Martin Martinov, 2019
#
#  Contributors:
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