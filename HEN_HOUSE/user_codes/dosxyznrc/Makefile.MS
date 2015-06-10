
###############################################################################
#
#  EGSnrc makefile to build the dosxyznrc application with beam models
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
#                   Ernesto Mainegra-Hing
#
###############################################################################
#
#  A Makefile for dosxyznrc.
#
#  This file is set to use the beam models. The files beammodel_macros.mortran
#  and  beammodel_routines.mortran MUST BE COPIED to your local area from
#  $HEN_HOUSE/omega/progs/beamdp.
#
#  If you do not need beam models, copy Makefile.NOMS to your local dosxyznrc
#  area rename it 'Makefile'.
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

EGS_EXTRA_OBJECTS = $(CUTIL_OBJECTS) $(PARDOSE_OBJECTS) $(BEAMLIB_OBJECTS) $(VCULIB_OBJECTS)
EGS_EXTRA_LIBS = $(FLIBS) $(BEAMLIB_EXTRA_LIBS)

SOURCES = $(EGS_SOURCEDIR)egsnrc.macros \
          $(EGS_UTILS)timing.macros \
          $(MACHINE_MACROS) $(RANDOM).macros \
          dosxyznrc_user_macros.mortran srcxyznrc.macros \
          $(EGS_SOURCEDIR)transportp.macros \
          $(EGS_UTILS)phsp_macros.mortran \
	  beammodel_macros.mortran \
          $(USER_CODE).mortran \
	  beammodel_routines.mortran \
          $(EGS_SOURCEDIR)get_inputs.mortran $(MACHINE_MORTRAN) \
          srcxyznrc.mortran $(RANDOM).mortran \
          $(EGS_UTILS)nrcaux.mortran \
          $(EGS_SOURCEDIR)egs_utilities.mortran \
          $(EGS_SOURCEDIR)egs_parallel.mortran \
          $(EGS_SOURCEDIR)egsnrc.mortran

include $(EGS_MAKEFILE)

pardose: $(EGS_LIBDIR)read_write_pardose.o

$(EGS_LIBDIR)read_write_pardose.o: $(DOSXYZ_HOME)read_write_pardose.c
	$(CC) $(C_FLAGS) -c -I. -I$(EGS_LIBDIR) $(COUT)$(EGS_LIBDIR)read_write_pardose.o $(DOSXYZ_HOME)read_write_pardose.c
