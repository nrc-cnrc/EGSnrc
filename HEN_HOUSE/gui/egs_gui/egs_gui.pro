
###############################################################################
#
#  EGSnrc Qt project file for the egs_gui graphical user interface
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
#                   Frederic Tessier
#                   Cody Crewson
#                   Reid Townson
#
###############################################################################


SOURCES += main.cpp \
        main_widget.cpp \
        egs_compile_page.cpp \
        egs_run_page.cpp \
        pegs_page.cpp \
        pegs_runoutput.cpp \
        egs_gui_widget.cpp \
        egs_configuration_page.cpp \
        egs_config_reader.cpp

HEADERS += main_widget.h \
        egs_compile_page.h \
        egs_run_page.h \
        pegs_page.h \
        pegs_runoutput.h \
        egs_gui_widget.h \
        egs_configuration_page.h \
        egs_config_reader.h

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

MOC_DIR =       .moc/$$my_machine
OBJECTS_DIR =   .obj/$$my_machine
DESTDIR  = ../../bin/$$my_machine

win32 {
    DEFINES += WIN32
    CONFIG  += qt thread warn_off release windows
    RC_FILE = egs_gui.rc
}

unix {
    CONFIG  += qt thread warn_on debug_and_release $$my_build
    contains( CONFIG, shared ):message( "Dynamic build..." )
    contains( CONFIG, static ){
        message( "Static build ..." )
        DEFINES += STATICGUI
        #INCLUDEPATH += ../egs_install/include
        DESTDIR = ../../pieces/linux
        UNAME = $$system(getconf LONG_BIT)
        contains( UNAME, 64 ){
           message( "-> 64 bit ($$SNAME)" )
           TARGET = egs_gui_64
        }
        contains( UNAME, 32 ){
           message( "-> 32 bit ($$SNAME)" )
           TARGET = egs_gui_32
        }
        QMAKE_POST_LINK = strip $(TARGET)
    }
}

FORMS   = pegs_page.ui \
          pegs_runoutput.ui

RESOURCES = egs_gui.qrc # resource collection file to store images in the application executable.
TEMPLATE  = app
LANGUAGE  = C++
