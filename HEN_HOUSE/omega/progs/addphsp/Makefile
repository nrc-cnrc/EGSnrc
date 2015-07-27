
###############################################################################
#
#  EGSnrc makefile for addphsp utility
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
#  Author:          Blake Walters, 2004
#
#  Contributors:    Iwan Kawrakow
#                   Frederic Tessier
#
###############################################################################


include $(EGS_CONFIG)
include $(SPEC_DIR)beamnrc.spec

SOURCES = $(MACHINE_MACROS) $(EGS_SOURCEDIR)egsnrc.macros \
          $(EGS_UTILS)phsp_macros.mortran $(IAEA_PHSP_MACROS)\
          addphsp.mortran \
          $(EGS_SOURCEDIR)lnblnk1.mortran

$(EGS_BINDIR)addphsp$(EXE): addphsp_$(my_machine).f
	@echo "Fortran compiling addphsp_$(my_machine).f using flags $(FOPT) $(FCFLAGS)"
	@$(F77) $(FOPT) $(FCFLAGS) $(FOUT)$@ addphsp_$(my_machine).f $(BEAM_EXTRA_OBJECTS)

addphsp_$(my_machine).f: $(SOURCES)
	@echo "Mortran compiling addphsp sources ..."
	@$(MORTRAN_EXE) -d $(MORTRAN_DATA) -f $(SOURCES) -o7 $@ \
	    -o8 addphsp_$(my_machine).mortlst
