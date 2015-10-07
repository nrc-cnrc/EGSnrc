
###############################################################################
#
#  EGSnrc makefile to build the dosxyznrc application
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
#  Author:          Iwan Kawrakow, 2004
#
#  Contributors:    Blake Walters
#                   Frederic Tessier
#
###############################################################################
#
#  A Makefile for dosxyznrc.
#
#  This version does not use beam model sources. If you want to use the beam
#  model macros and routines, first copy beammodel_macros.mortran and
#  beammodel_routines.mortran from $HEN_HOUSE/omega/progs/beamdp to your local
#  dosxyznrc area. Then below, add  beammodel_macros.mortran before
#  $(USER_CODE).mortran, and beammodel_routines.mortran somewhere after it.
#  (This is done for you in Makefile.MS which you could copy to your area and
#  rename 'Makefile'.)
#
#  This Makefile uses the standard EGSnrc Makefile to do the actual mortran
#  and fortran compilation.
#
###############################################################################


include $(EGS_CONFIG)
include $(SPEC_DIR)beamnrc.spec
include $(SPEC_DIR)dosxyznrc_$(my_machine).spec

RANDOM = $(EGS_SOURCEDIR)ranmar

USER_CODE = dosxyznrc

EGS_EXTRA_OBJECTS = $(CUTIL_OBJECTS) $(PARDOSE_OBJECTS) $(BEAMLIB_OBJECTS) $(BEAM_OBJECTS) $(VCULIB_OBJECTS)
EGS_EXTRA_LIBS = $(FLIBS) $(BEAMLIB_EXTRA_LIBS)

SOURCES = $(EGS_SOURCEDIR)egsnrc.macros \
          $(EGS_UTILS)timing.macros \
          $(MACHINE_MACROS) $(RANDOM).macros \
          dosxyznrc_user_macros.mortran srcxyznrc.macros \
          $(EGS_SOURCEDIR)transportp.macros \
          $(EGS_SOURCEDIR)pegs4_macros.mortran \
          $(EGS_UTILS)phsp_macros.mortran $(IAEA_PHSP_MACROS)\
          $(USER_CODE).mortran \
          $(EGS_SOURCEDIR)get_inputs.mortran \
          $(EGS_SOURCEDIR)get_media_inputs.mortran $(MACHINE_MORTRAN) \
          srcxyznrc.mortran $(RANDOM).mortran \
          $(EGS_UTILS)nrcaux.mortran \
          $(EGS_SOURCEDIR)egs_utilities.mortran \
          $(EGS_SOURCEDIR)egs_parallel.mortran \
          $(EGS_SOURCEDIR)pegs4_routines.mortran \
          $(EGS_SOURCEDIR)egsnrc.mortran

include $(EGS_MAKEFILE)

pardose: $(EGS_LIBDIR)read_write_pardose.$(FOBJE)

$(EGS_LIBDIR)read_write_pardose.$(FOBJE): $(DOSXYZ_HOME)read_write_pardose.c
	$(CC) $(C_FLAGS) -c -I. -I$(EGS_LIBDIR) $(COUT)$(EGS_LIBDIR)read_write_pardose.$(FOBJE) $(DOSXYZ_HOME)read_write_pardose.c
