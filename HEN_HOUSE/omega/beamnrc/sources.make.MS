
###############################################################################
#
#  EGSnrc BEAMnrc definitions to enable multiple source models
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
#  Author:          Blake Walters, 2005
#
#  Contributors:
#
###############################################################################
#
#  This file is created by beam_build so if you redo beam_build you will lose
#  any edits. This version of the file is set to employ the multiple source
#  models. Note that the source files
#
#  beammodel_macros.mortran
#  beammodel_routines.mortran
#
#  must be copied to your beamnrc area prior to making the user code. They are
#  on $OMEGA_HOME/progs/beamdp.
#
#  You may use this file to change the source files used to build this
#  accelerator. To do so, just include a statement 'SOURCES = list_of_files'
#  below.
#
###############################################################################


SOURCES = $(EGS_SOURCEDIR)egsnrc.macros \
          $(EGS_UTILS)timing.macros \
          $(EGS_UTILS)phsp_macros.mortran \
          $(MACHINE_MACROS) $(RANDOM).macros \
          $(EGS_SOURCEDIR)transportp.macros \
          $(BEAM_HOME)beamnrc_user_macros.mortran \
          beammodel_macros.mortran \
          $(BEAM_CODE)_macros.mortran \
          $(EGS_SOURCEDIR)egs_utilities.mortran \
          $(BEAM_HOME)beam_main.mortran\
          $(BEAM_HOME)beamnrc.mortran \
          $(EGS_UTILS)xvgrplot.mortran \
          $(BEAM_CODE)_cm.mortran \
          $(EGS_SOURCEDIR)get_inputs.mortran \
          $(RANDOM).mortran \
          beammodel_routines.mortran \
          $(EGS_UTILS)nrcaux.mortran \
          $(MACHINE_MORTRAN) \
          $(EGS_SOURCEDIR)egs_parallel.mortran \
          $(EGS_SOURCEDIR)egsnrc.mortran

LIB_SOURCES = $(EGS_SOURCEDIR)egsnrc.macros \
          $(EGS_UTILS)timing.macros \
          $(MACHINE_MACROS) $(RANDOM).macros \
          $(EGS_SOURCEDIR)transportp.macros \
          $(BEAM_HOME)beamnrc_user_macros.mortran \
          beammodel_macros.mortran \
          $(EGS_UTILS)phsp_macros.mortran \
          $(BEAM_HOME)sbsnrc_macros.mortran \
          $(BEAM_CODE)_macros.mortran \
          $(EGS_SOURCEDIR)egs_utilities.mortran \
          $(BEAM_HOME)beam_lib.mortran\
          $(BEAM_HOME)beamnrc.mortran \
          $(EGS_UTILS)xvgrplot.mortran \
          $(BEAM_CODE)_cm.mortran \
          $(EGS_SOURCEDIR)get_inputs.mortran \
          $(RANDOM).mortran \
          beammodel_routines.mortran \
          $(EGS_UTILS)nrcaux.mortran \
          $(MACHINE_MORTRAN) \
          $(EGS_SOURCEDIR)egs_parallel.mortran \
          $(EGS_SOURCEDIR)egsnrc.mortran

