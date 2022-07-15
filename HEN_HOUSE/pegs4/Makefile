
###############################################################################
#
#  EGSnrc makefile to build pegs4
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
#  Author:          Iwan Kawrakow
#
#  Contributors:    Frederic Tessier
#                   Ernesto Mainegra-Hing
#
###############################################################################


include $(EGS_CONFIG)

FORTRAN_FILE = pegs4_$(my_machine).$(FEXT)

sources = $(MACHINE_MACROS) pegs4.mortran $(MACHINE_MORTRAN)

$(PEGS4_EXE): $(FORTRAN_FILE)
	@echo "Compiling $(FORTRAN_FILE) with flags '$(FCFLAGS) $(FOPT)'"
	@$(F77) $(FCFLAGS) $(FOPT) $(FOUT)$@ $(FORTRAN_FILE) $(FLIBS)

$(FORTRAN_FILE): $(sources)
	@echo "Generating $(FORTRAN_FILE) from pegs4.mortran"
	@$(MORTRAN_EXE) -d $(MORTRAN_DATA) -f $(sources) -o7 $@ -o8 pegs4_$(my_machine).mortlst

clean:
	$(REMOVE) $(FORTRAN_FILE) $(HEN_HOUSE)/bin/$(my_machine)/pegs4.exe
