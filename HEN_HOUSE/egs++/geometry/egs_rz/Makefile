
###############################################################################
#
#  EGSnrc egs++ makefile to build rz geometry
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


include $(EGS_CONFIG)
include $(SPEC_DIR)egspp.spec
include $(SPEC_DIR)egspp_$(my_machine).conf

DEFS = $(DEF1) -Wall

library = egs_rz
lib_files = egs_rz
link2_libs = egspp egs_planes egs_cylinders egs_ndgeometry

.PHONY: ndgeom cyls planes clean

include $(SPEC_DIR)egspp_libs.spec

$(make_depend)

$(DSO2)$(library).$(obje): ndgeom cyls planes

$(ABS_DSO)$(libpre)$(library)$(libext): ndgeom cyls planes

ndgeom:
	cd ..$(DSEP)egs_nd_geometry && make

cyls:
	cd ..$(DSEP)egs_cylinders && make

planes:
	cd ..$(DSEP)egs_planes && make

clean:
	rm -f $(ABS_DSO)$(libpre)$(library)$(libext)
	rm -f $(DSO2)$(library).$(obje)
