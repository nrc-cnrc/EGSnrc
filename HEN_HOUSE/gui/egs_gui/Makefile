
###############################################################################
#
#  EGSnrc makefile to build the egs_gui graphical user interface
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
#  Contributors:    Ernesto Mainegra-Hing
#                   Dave Rogers
#
###############################################################################
#
#  A Makefile for building the egs_gui GUI. Once EGS_CONFIG is set, one
#  can just issue 'make' in this directory instead of using:
#
#  qmake my_machine=$(my_machine) MAKEFILE=Makefile_$(my_machine)
#  make -f Makefile_$(my_machine)
#
###############################################################################


include $(EGS_CONFIG)

#all: clean Makefile_$(my_machine)
all: Makefile_$(my_machine)
	$(gui_make) -f Makefile_$(my_machine) my_machine=$(my_machine)

static: Makefile_static_$(my_machine)
	$(gui_make) -f Makefile_static_$(my_machine) my_machine=$(my_machine)

Makefile_$(my_machine): egs_gui.pro
	$(QTDIR)/bin/qmake my_machine=$(my_machine) my_build=shared MAKEFILE=Makefile_$(my_machine)

Makefile_static_$(my_machine): egs_gui.pro
	$(QTDIR)/bin/qmake my_machine=$(my_machine) my_build=static MAKEFILE=Makefile_static_$(my_machine)

clean: Makefile_$(my_machine)
	$(gui_make) -f Makefile_$(my_machine) my_machine=$(my_machine) clean

realclean: clean
	rm Makefile_*$(my_machine)

.PHONY: all clean
