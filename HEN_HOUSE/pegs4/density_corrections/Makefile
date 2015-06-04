
###############################################################################
#
#  EGSnrc makefile to build split_del.exe
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
#
#  Makefile to build utility to split Berger and Seltzer's density correction
#  files into separate files.
#
###############################################################################


include $(EGS_CONFIG)

sources = $(MACHINE_MACROS) split_del.mortran $(MACHINE_MORTRAN)

data: time_stamp

time_stamp: split_del.exe elements.del compounds.del
	@./split_del.exe

split_del.exe: split_del.f
	@echo "Compiling split_del.f ..."
	@$(F77) $(FCFLAGS) $(FOPT) $(FOUT)$@ split_del.f $(FLIBS)

split_del.f: $(sources)
	@echo "Creating $@ from the mortran sources ..."
	@$(MORTRAN_EXE) -d $(MORTRAN_DATA) -f $(sources) -o7 $@ -o8 split_del.mortlst

force: ;

.PHONY: force
