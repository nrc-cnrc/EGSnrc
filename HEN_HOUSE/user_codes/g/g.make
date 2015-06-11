
###############################################################################
#
#  EGSnrc g application make definitions
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
#  Author:          Ernesto Mainegra-Hing, 2004
#
#  Contributors:
#
###############################################################################
#
#  The .make file MUST be present in the user code directory, even if it is
#  just an empty file.
#
#  The .make file can be used to
#
#  - change the standard set of files needed to build a user code by including
#    a "SOURCES = ..." statement.
#
#  - modify the compilation options, e.g.,
#    FCFLAS = -fno-automatic -finit-local-zero
#    to make all local variables static with G77.
#
#  - Pass C preprocessor directive on the command line, e.g.,
#    FCFLAGS = $(FCFLAGS) -Dmy_code
#
###############################################################################


# We use the .make file to remove the egs_c_utils.o file
# from the link step (because no parallel processing is being used).
#
EGS_EXTRA_OBJECTS =

# We prefer using the ranmar generator => we overwrite the
# default from $HEN_HOUSE/specs/all_common.spec
#
RANDOM = $(EGS_SOURCEDIR)ranmar

# tutor7 needs a different set of mortran files than tutor1-6.
# => We must overwrite the SOURCES variable.
# Note: order is important!
# Note: don;t forget the leading tabs on continuation lines!
#
SOURCES = \
       $(EGS_SOURCEDIR)egsnrc.macros \
       $(MACHINE_MACROS) \
       $(RANDOM).macros\
       $(EGS_SOURCEDIR)transportp.macros\
       $(USER_CODE).mortran \
       $(RANDOM).mortran \
       $(EGS_SOURCEDIR)get_inputs.mortran\
       $(EGS_UTILS)nrcaux.mortran \
       $(MACHINE_MORTRAN) \
       $(EGS_SOURCEDIR)egs_utilities.mortran \
       $(EGS_SOURCEDIR)egsnrc.mortran
