
###############################################################################
#
#  EGSnrc egs++ makefile to build auto envelope geometry
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
#                   Reid Townson
#
###############################################################################


include $(EGS_CONFIG)
include $(SPEC_DIR)egspp.spec
include $(SPEC_DIR)egspp_$(my_machine).conf

SOBOL =
SOBOL_DEF =
SOBOL_H =
ifneq ("$(wildcard egs_sobol.cpp)","")
SOBOL = sobol egs_sobol
SOBOL_DEF = -DBUILD_SOBOL_DLL -DBUILD_AE_SOBOL -DHAS_SOBOL
SOBOL_H = sobol.h egs_sobol.h
endif

GZSTREAM =
GZSTREAM_DEF =
GZSTREAM_H =
GZSTREAM_LIB =
ifneq ("$(wildcard gzstream.cpp)","")
GZSTREAM = gzstream
GZSTREAM_DEF = -DBUILD_GZSTREAM -DHAS_GZSTREAM
GZSTREAM_H = gzstream.h
GZSTREAM_LIB = z
endif

DEFS = $(DEF1) -DBUILD_AENVELOPE_DLL $(SOBOL_DEF) $(GZSTREAM_DEF)


library = egs_autoenvelope
lib_files = egs_autoenvelope $(SOBOL) $(GZSTREAM)

link2_libs = egs_gtransformed egspp $(GZSTREAM_LIB)


include $(SPEC_DIR)egspp_libs.spec

$(make_depend)


$(DSO2)autoenvelope.$(obje): volcor.h $(GZSTREAM_H) \
                          ..$(DSEP)egs_gtransformed$(DSEP)egs_gtransformed.h

$(ABS_DSO)$(libpre)egs_autoenvelope$(libext): volcor.h $(GZSTREAM_H) \
						..$(DSEP)egs_gtransformed$(DSEP)egs_gtransformed.h

clean:
	$(REMOVE) $(ABS_DSO)$(libpre)egs_autoenvelope$(libext) $(ABS_DSO)$(libpre)egs_sobol$(libext) $(ABS_DSO)$(libpre)sobol$(libext) \
		$(ABS_DSO)$(libpre)gzstream$(libext) $(ABS_DSO)$(libpre)gzstream$(libext)
	$(REMOVE) $(DSO2)autoenvelope.$(obje) $(DSO2)sobol.$(obje) $(DSO2)egs_sobol.$(obje) \
		$(DSO2)gzstream.$(obje) $(DSO2)gzstream.$(obje) \

touchup:
	touch egs_autoenvelope.cpp

fresh: clean touchup $(lib_objects)


.PHONY: clean touchup fresh
