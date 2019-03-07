
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
#
#  Build GUIs statically linked to Qt library for distribution. If no static
#  Qt library available, one can use dynamically linked Qt as long as EGSnrc
#  is installed on an identical system with the same Qt version.
#
#  BEWARE:
#  Change variable qt4 below to point to the desired Qt library. The current
#  implementation is motivated by the fact that normally at NRC we use a
#  dynamically built Qt library. However, for distribution we want to use a
#  static version.
#
#  NOTE:
#  If QTDIR is set to the desired Qt library version in the user's environment,
#  one could get rid of the qt4 variable or set it to $QTDIR.
#
################################################################################


#machine=$(shell echo linux-static`getconf LONG_BIT`)
machine=linux-static
egs_config=$(HEN_HOUSE)/specs/$(machine).conf

######################################################################
# qt4 environment needed for egs_gui, egs_inprz and egs_beam_install #
######################################################################
qt4=$(QTDIR)
qmake4=$(qt4)/mkspecs/linux-g++

all: egs_gui egs_inprz egs_view egs_configure

egs_gui:
	@echo "Building egs_gui statically ..."
	cd ../../gui/egs_gui && export QTDIR=$(qt4) && export QMAKESPEC=$(qmake4) && export EGS_CONFIG=$(egs_config) && make static

egs_inprz:
	@echo "Building egs_inprz statically ..."
	cd ../../gui/egs_inprz && export QTDIR=$(qt4) && export QMAKESPEC=$(qmake4) && export EGS_CONFIG=$(egs_config) && make static

egs_view: egspp
	@echo "Building egs_view statically ..."
	@cd ../../egs++/view && export QTDIR=$(qt4)  && export QMAKESPEC=$(qmake4) && export EGS_CONFIG=$(egs_config) && make static

egspp:
	@echo "Building egspp ..."
	cd ../../egs++ && export EGS_CONFIG=$(egs_config) && make glibs && make aobjects && make shapes

viewclean:
	cd ../../egs++/view && export QTDIR=$(qt4) && export EGS_CONFIG=$(egs_config) && make realclean

egs_configure:
	cd ../../gui/egs_configure && export QTDIR=$(qt4) && export QMAKESPEC=$(qmake4) && export EGS_CONFIG=$(egs_config) && make

clean: viewclean
	cd ../../egs++ && export EGS_CONFIG=$(egs_config) && make realclean
	cd ../../gui/egs_inprz && export QTDIR=$(qt4) && export QMAKESPEC=$(qmake4) && export EGS_CONFIG=$(egs_config) && make realclean
	cd ../../gui/egs_gui && export QTDIR=$(qt4) && export QMAKESPEC=$(qmake4) && export EGS_CONFIG=$(egs_config) && make realclean
	cd ../../gui/egs_configure && export QTDIR=$(qt4) && export QMAKESPEC=$(qmake4) && export EGS_CONFIG=$(egs_config) && make realclean

.PHONY: all clean

