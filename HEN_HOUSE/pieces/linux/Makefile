
###############################################################################
#
#  EGSnrc makefile to build GUIs statically on linux
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
#  Author:          Ernesto Mainegra-Hing
#
#  Contributors:
#
###############################################################################


machine=$(shell echo linux`getconf LONG_BIT`)
egs_config=$(HEN_HOUSE)/specs/$(machine).conf

#######################################
# qt3 environment needed for egs_view #
#######################################
ifeq (${machine},linux64)
   qt_static=$(HOME)/qt3.3.8bstatic
else
   qt_static=$(HOME)/qt-static
endif
qmakespecs=$(qt_static)/mkspecs/linux-static

######################################################################
# qt4 environment needed for egs_gui, egs_inprz and egs_beam_install #
######################################################################
qt4=$(HOME)/qt-4.8.6-static
qmake4=$(qt4)/mkspecs/linux-g++

all: egs_gui egs_inprz egs_view egs_beam_install

egs_gui:
	@echo "Building egs_gui statically ..."
	cd ../../gui/egs_gui && export QTDIR=$(qt4) && export QMAKESPEC=$(qmake4) && export EGS_CONFIG=$(egs_config) && make static

egs_inprz:
	@echo "Building egs_inprz statically ..."
	cd ../../gui/egs_inprz && export QTDIR=$(qt4) && export QMAKESPEC=$(qmake4) && export EGS_CONFIG=$(egs_config) && make static

egs_view: egspp
	@echo "Building egs_view statically ..."
	@cd ../../egs++/view && export QTDIR=$(qt_static) && export EGS_CONFIG=$(egs_config) && make static

egspp:
	@echo "Building egspp ..."
	cd ../../egs++ && export EGS_CONFIG=$(egs_config) && make

viewclean:
	cd ../../egs++/view && export QTDIR=$(qt_static) && export EGS_CONFIG=$(egs_config) && make realclean

egs_beam_install:
	cd ../../gui/egs_beam_install && export QTDIR=$(qt4) && export QMAKESPEC=$(qmake4) && export EGS_CONFIG=$(egs_config) && make

clean: viewclean
	cd ../../egs++ && export EGS_CONFIG=$(egs_config) && make clean
	cd ../../gui/egs_inprz && export QTDIR=$(qt4) && export QMAKESPEC=$(qmake4) && export EGS_CONFIG=$(egs_config) && make realclean
	cd ../../gui/egs_gui && export QTDIR=$(qt4) && export QMAKESPEC=$(qmake4) && export EGS_CONFIG=$(egs_config) && make realclean

.PHONY: all clean

