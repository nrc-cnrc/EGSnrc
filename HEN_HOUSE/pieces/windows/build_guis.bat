@echo off
rem ###############################################################################
rem #
rem #  EGSnrc bat file to build GUIs on Windows
rem #  Copyright (C) 2015 National Research Council Canada
rem #
rem #  This file is part of EGSnrc.
rem #
rem #  EGSnrc is free software: you can redistribute it and/or modify it under
rem #  the terms of the GNU Affero General Public License as published by the
rem #  Free Software Foundation, either version 3 of the License, or (at your
rem #  option) any later version.
rem #
rem #  EGSnrc is distributed in the hope that it will be useful, but WITHOUT ANY
rem #  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
rem #  FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for
rem #  more details.
rem #
rem #  You should have received a copy of the GNU Affero General Public License
rem #  along with EGSnrc. If not, see <http://www.gnu.org/licenses/>.
rem #
rem ###############################################################################
rem #
rem #  Author:          Iwan Kawrakow, 2003
rem #
rem #  Contributors:    Ernesto Mainegra-Hing
rem #
rem ###############################################################################
rem #
rem #  Build GUIs statically linked to Qt library for distribution. If no static
rem #  Qt library available, one can use dynamically linked Qt as long as EGSnrc
rem #  is installed on an identical system with the same Qt version.
rem #
rem #  Pre-requisite:
rem #  the_conf must point to an existing configuration file. At NRC we use
rem #  win-static.conf but the user can use an arbitrary name.
rem #
rem #  BEWARE:
rem #  Change variable QTDIR below to point to the desired Qt library. The current
rem #  implementation is motivated by the fact that normally at NRC we use a
rem #  dynamically built Qt library. However, for distribution we want to use a
rem #  static version.
rem #
rem #  NOTE:
rem #  If QTDIR is set to the desired Qt library version in the user's environment,
rem #  one could comment out the line below that sets QTDIR.
rem #
rem ###############################################################################


set QTDIR=C:\Qt\4.8.6-mingw482-static
set the_config=win-static
echo .
echo Checking if GUIs and libraries need to be rebuilt
echo .
rem ###########################################################################
cd ..\..\gui\egs_gui
echo Working in egs_gui...
echo Making Makefile_%the_config%...
%QTDIR%\bin\qmake my_machine=%the_config% MAKEFILE=Makefile_%the_config%
echo Running GNU make...
mingw32-make -f Makefile_%the_config%
rem
rem ###########################################################################
cd ..\egs_inprz
echo Working in egs_inprz...
echo Making Makefile_%the_config%...
%QTDIR%\bin\qmake my_machine=%the_config% MAKEFILE=Makefile_%the_config%
echo Running GNU make...
mingw32-make -f Makefile_%the_config%
rem
rem ###########################################################################
cd ..\egs_configure
echo Working in egs_configure...
echo Making Makefile_%the_config%...
%QTDIR%\bin\qmake my_machine=%the_config% MAKEFILE=Makefile_%the_config% the_dir=..
echo Running GNU make...
mingw32-make -f Makefile_%the_config%
rem
rem ###########################################################################
cd ..\..\egs++
echo Working in egspp...
echo Running GNU make...
mingw32-make EGS_CONFIG=%HEN_HOUSE%\specs\%the_config%.conf glibs
mingw32-make EGS_CONFIG=%HEN_HOUSE%\specs\%the_config%.conf shapes
mingw32-make EGS_CONFIG=%HEN_HOUSE%\specs\%the_config%.conf aobjects
rem
rem ###########################################################################
cd view
echo Working in egs_view...
echo Making Makefile_%the_config%...
%QTDIR%\bin\qmake my_machine=%the_config% MAKEFILE=Makefile_%the_config%
echo Running GNU make...
mingw32-make -f Makefile_%the_config%
rem
rem ###########################################################################
cd ..\..\pieces\windows
