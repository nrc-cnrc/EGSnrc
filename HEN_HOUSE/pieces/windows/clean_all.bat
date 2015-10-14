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


echo Checking if object files need to be deleted
echo Argument %1
echo .
echo Deleting objects
     del *.obj
echo .
echo .
echo Checking if GUIs need to be cleaned
echo .
cd ..\..\egs++\view
echo Working in egs++\view...
     if exist Makefile_win2k-cl (
         echo Running nmake clean
         nmake -nologo -f Makefile_win2k-cl clean
     )
     echo Deleting Makefile_win2k-cl
     del Makefile_win2k-cl

cd ..\..\gui\egs_configure
echo Working in egs_configure...
    if exist Makefile_win2k (
        echo Running nmake clean
        nmake -nologo -f Makefile_win2k clean
    )
    echo Deleting Makefile_win2k
    del Makefile_win2k

cd ..\egs_gui
echo Working in egs_gui...
    if exist Makefile_win2k (
        echo Running nmake clean
        nmake -nologo -f Makefile_win2k clean
    )
    echo Deleting Makefile_win2k
    del Makefile_win2k

cd ..\egs_inprz
echo Working in egs_inprz...
    if exist Makefile_win2k (
        echo Running nmake clean
        nmake -nologo -f Makefile_win2k clean
    )
    echo Deleting Makefile_win2k
    del Makefile_win2k

cd ..\egs_install\static
echo Working in egs_install...
    if exist Makefile_win2k (
        echo Running nmake clean
        nmake -nologo -f Makefile_win2k clean
    )
    echo Deleting Makefile_win2k
    del Makefile_win2k

cd ..\..\beam_install
echo Working in beam_install...
    if exist Makefile_win2k (
        echo Running nmake clean
        nmake -nologo -f Makefile_win2k clean
    )
    echo Deleting Makefile_win2k
    del Makefile_win2k

cd ..\..\pieces\windows
