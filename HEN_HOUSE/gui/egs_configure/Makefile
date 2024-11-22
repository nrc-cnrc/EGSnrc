
###############################################################################
#
#  EGSnrc makefile to build the configuration GUI
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
#  Author:          Ernesto Mainegra-Hing, 2015
#
#  Contributors:
#
###############################################################################
#
#  A Makefile for building the configuration GUIs. Once EGS_CONFIG is set, one
#  can just issue 'make' in this directory instead of using:
#
#  $(QTDIR)/bin/qmake my_machine=$(my_machine) MAKEFILE=Makefile_$(my_machine)
#  make -f Makefile_$(my_machine)
#
###############################################################################


include $(EGS_CONFIG)

all: Makefile_$(my_machine)
	$(gui_make) -f Makefile_$(my_machine) my_machine=$(my_machine)

Makefile_$(my_machine): egs_beam_install.pro
	$(QTDIR)/bin/qmake my_machine=$(my_machine) MAKEFILE=Makefile_$(my_machine) the_dir=.

clean: Makefile_$(my_machine)
	$(gui_make) -f Makefile_$(my_machine) my_machine=$(my_machine) clean

realclean: clean
	$(REMOVE) Makefile_$(my_machine)*

.PHONY: all clean
