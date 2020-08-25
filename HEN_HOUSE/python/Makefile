###############################################################################
#
#  Copyright (C) 2020 Simon Biggs
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
#  Contributors:    Frederic Tessier, Simon Biggs
#
###############################################################################

include $(EGS_CONFIG)

export PYTHON_BUILD = cpython-37m-x86_64-linux-gnu

all: pyegsnrc/egsnrc.$(PYTHON_BUILD).so

SOURCES = \
    $(EGS_SOURCEDIR)egsnrc.macros \
	$(MACHINE_MACROS) \
	$(RANDOM).macros \
	./user-stubs.mortran \
	$(RANDOM).mortran \
	$(EGS_SOURCEDIR)nrcaux.mortran \
	$(MACHINE_MORTRAN) \
	$(EGS_SOURCEDIR)egs_utilities.mortran \
	$(EGS_SOURCEDIR)egsnrc.mortran

pyegsnrc/egsnrc.%.so :
	$(MORTRAN_EXE) -s -d $(MORTRAN_DATA) -f $(SOURCES) $< -o7 ./pyegsnrc/egsnrc.f -o8 egsnrc.mortlst
	poetry run python -m numpy.f2py -c ./pyegsnrc/egsnrc.f -m egsnrc
	mv egsnrc.$(PYTHON_BUILD).so $@
