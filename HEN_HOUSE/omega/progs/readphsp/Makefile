
###############################################################################
#
#  EGSnrc makefile for the readphsp utility
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
#  Contributors:    Frederic Tessier
#
###############################################################################


include $(EGS_CONFIG)
include $(SPEC_DIR)beamnrc.spec

SOURCES = $(MACHINE_MACROS) $(EGS_UTILS)phsp_macros.mortran readphsp.mortran \
          $(EGS_SOURCEDIR)lnblnk1.mortran

# If you don't have the PAW library, you will have to use the
# file dummy.f to provide dummy functions and then not have the
# PAW functionality. If you have the PAW library, then set DUMMY_FILE
# to empty and set LIB_PATH to where your paw library is and
# LIBS to -lpawlib -lpacklib.
#
DUMMY_FILE = dummy.f
LIB_PATH =
LIBS =
#DUMMY_FILE =
#LIB_PATH = -L readphsp_Linux
#LIBS = -lpawlib -lpacklib

$(EGS_BINDIR)readphsp$(EXE): readphsp_$(my_machine).f $(DUMMY_FILE)
	@echo "Fortran compiling readphsp_$(my_machine).f using flags $(FOPT) $(FCFLAGS) "
	@$(F77) $(FOPT) $(FCFLAGS) $(LIB_PATH) $(FOUT)$@ readphsp_$(my_machine).f $(DUMMY_FILE) $(LIBS)

readphsp_$(my_machine).f: $(SOURCES)
	@echo "Mortran compiling readphsp sources ..."
	@$(MORTRAN_EXE) -d $(MORTRAN_DATA) -f $(SOURCES) -o7 $@ \
	      -o8 readphsp_$(my_machine).mortlst
