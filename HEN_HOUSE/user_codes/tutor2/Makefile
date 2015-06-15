
###############################################################################
#
#  EGSnrc makefile to build the tutor2 application
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
#  Author:          Iwan Kawrakow, 2003
#
#  Contributors:
#
###############################################################################
#
#  A generic Makefile for a EGSnrc user code that uses mortran source files
#  only. To use it, copy it to your user code directory and:
#
#  - set the name of your user code in the line USER_CODE = ...
#
#  - create a file named something.make where 'something' is the name of your
#    user code. In the simplest possible case this file is empty. You can use
#    it to specify an alternative set of sources, select the random number
#    generator (ranlux vs ranmar), etc. See the .make files in tutor1-tutor7
#    and the NRC RZ codes for examples.
#
###############################################################################


# First include the active EGSnrc configuration. EGS_CONFIG is a environment
# variable that must be set to point to the EGSnrc config file. This file
# defines the compiler name, compiler options, various directories, the
# standard set of Mortran sources needed to build a user code, etc.
#
# The configuration file is created during the EGSnrc installations and can be
# found in $HEN_HOUSE/specs.

include $(EGS_CONFIG)

# Then define the user code name
#
USER_CODE = tutor2

# User code specific definitions for Make
#
include $(USER_CODE).make

# That's all we need. Now we can use the standard EGSnrc Makefile to
# build the code with optimization, no optimization, debug and to also
# clean up. We have the following targets defined:
#
# make                   Build the user code executable with optimization
# make opt               turned on.
#
# make noopt             Build the user code executable with optimization with
#                        optimization turned off.
#
# make debug             Build a user code executable for debugging
#
# make fortran           Mortran step only.
#
# make clean             Remove the fortran file, mortjob file, mortlst file
#                        and the various executables.
#
include $(EGS_MAKEFILE)
