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
rem #  Author:          Ernesto Mainegra-Hing, 2011
rem #
rem #  Contributors:
rem #
rem ###############################################################################

echo Checking if GUIs and libraries need to be cleaned
echo .

cd ..\..\egs++
mingw32-make realclean

cd view
echo Working in egs++\view...
     if exist Makefile_win-static (
          echo Running make clean
          mingw32-make realclean
     )

cd ..\..\gui\egs_configure
echo Working in egs_configure...
    if exist Makefile_win-static (
        echo Running mingw32-make clean
        mingw32-make -f Makefile_win-static clean
    )
    echo Deleting Makefile_win-static
    del Makefile_win-static*

cd ..\egs_gui
echo Working in egs_gui...
    if exist Makefile_win-static (
        echo Running mingw32-make clean
        mingw32-make -f Makefile_win-static clean
    )
    echo Deleting Makefile_win-static
    del Makefile_win-static*

cd ..\egs_inprz
echo Working in egs_inprz...
    if exist Makefile_win-static (
        echo Running mingw32-make clean
        mingw32-make -f Makefile_win-static clean
    )
    echo Deleting Makefile_win-static
    del Makefile_win-static*

cd ..\..\pieces\windows
del *.exe
