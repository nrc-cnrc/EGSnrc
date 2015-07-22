
###############################################################################
#
#  EGSnrc makefile for the beam_build tool
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

beam_build_sources = $(EGS_SOURCEDIR)egsnrc.macros \
	              $(BEAM_HOME)beamnrc_user_macros.mortran \
                     $(MACHINE_MACROS) $(EGS_SOURCEDIR)lnblnk1.mortran \
                     beam_build.mortran $(MACHINE_MORTRAN)

$(BEAM_BUILD): beam_build_$(my_machine).f
	@echo "Compiling beam_build_$(my_machine).f using flags $(FOPT) $(FCFLAGS)"
	@$(F77) $(FOPT) $(FCFLAGS) -o $@ beam_build_$(my_machine).f

beam_build_$(my_machine).f: $(beam_build_sources)
	@echo "Mortran compiling beam_build"
	@$(MORTRAN_EXE) -d $(MORTRAN_DATA) -s -f $(beam_build_sources) -o7 $@ -o8 beam_build.lst
