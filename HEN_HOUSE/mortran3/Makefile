
###############################################################################
#
#  EGSnrc mortran3 makefile
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
#  Contributors:    Frederic Tessier
#
###############################################################################


include $(EGS_CONFIG)

all: $(MORTRAN_DATA) $(MORTRAN_EXE)

$(MORTRAN_DATA): mornew77.raw $(MORTRAN_EXE)
	@echo "Making mortran3.dat"
	@$(MORTRAN_EXE) -s -d mornew77.raw -o7 $@ -o8 mornew77.lst

sources = mortran3.f $(EGS_LIBDIR)machine.f

$(MORTRAN_EXE): $(sources)
	@echo "Compiling $(sources)"
	@$(F77) $(FCFLAGS) $(FOPT) $(FOUT)$@ $(sources) $(FLIBS)

check: check77_$(my_machine).exe
	@echo "Running check77 test program"
	@check77_$(my_machine).exe

check77_$(my_machine).exe: check77.mortran
	@echo "Mortran compiling check77 test program"
	@$(MORTRAN_EXE) -s -d $(MORTRAN_DATA) -f check77.mortran -o7 check77_$(my_machine).f -o8 check77_$(my_machine).mortlst
	@echo "Fortran compiling check77 test program"
	@$(F77) $(FCFLAGS) $(FOPT) -o $@ check77_$(my_machine).f $(FLIBS)
