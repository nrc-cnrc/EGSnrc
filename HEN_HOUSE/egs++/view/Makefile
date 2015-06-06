
###############################################################################
#
#  EGSnrc egs++ makefile to build geometry viewer
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
#  Author:          Iwan Kawrakow, 2005
#
#  Contributors:    Ernesto Mainegra-Hing
#
###############################################################################


include $(EGS_CONFIG)

all: Makefile_$(my_machine)
	$(gui_make) -f Makefile_$(my_machine) my_machine=$(my_machine) hhouse=$(HEN_HOUSE)

static: Makefile_static_$(my_machine)
	$(gui_make) -f Makefile_static_$(my_machine) my_machine=$(my_machine)

Makefile_$(my_machine): view.pro
	$(QTDIR)/bin/qmake my_machine=$(my_machine) MAKEFILE=Makefile_$(my_machine) hhouse=$(HEN_HOUSE)

Makefile_static_$(my_machine): view.pro
	$(QTDIR)/bin/qmake my_machine=$(my_machine) my_build=static MAKEFILE=Makefile_static_$(my_machine)  hhouse=$(HEN_HOUSE)

clean: Makefile_$(my_machine)
	$(gui_make) -f Makefile_$(my_machine) my_machine=$(my_machine) clean hhouse=$(HEN_HOUSE)

realclean: clean
	rm Makefile_*$(my_machine)

.PHONY: all clean


