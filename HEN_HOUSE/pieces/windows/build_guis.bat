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


set QTDIR=C:\Qt\4.8.6-mingw482-static
echo .
echo Checking if GUIs need to be rebuilt
echo .
rem make
cd ..\..\gui\egs_gui
echo Working in egs_gui...
echo Making Makefile_win-static...
%QTDIR%\bin\qmake my_machine=win-static MAKEFILE=Makefile_win-static
echo Running nmake...
mingw32-make -f Makefile_win-static
rem
rem
cd ..\egs_inprz
echo Working in egs_inprz...
echo Making Makefile_win-static...
%QTDIR%\bin\qmake my_machine=win-static MAKEFILE=Makefile_win-static
echo Running nmake...
mingw32-make -f Makefile_win-static
rem
cd ..\egs_beam_install
echo Working in egs_beam_install...
echo Making Makefile_win-static...
%QTDIR%\bin\qmake my_machine=win-static MAKEFILE=Makefile_win-static the_dir=..
echo Running nmake...
mingw32-make -f Makefile_win-static

cd ..\..\pieces\windows
