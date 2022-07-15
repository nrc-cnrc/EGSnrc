
###############################################################################
#
#  EGSnrc configuration GUI project file
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
#  Contributors:    Cody Crewson
#                   Reid Townson
#                   Ernesto Mainegra-Hing
#
###############################################################################


SOURCES += main.cpp \
           egs_wizard.cpp \
           egs_install.cpp \
           egs_install_conf.cpp \
           egs_install_env.cpp \
           egs_install_beam.cpp \
           egs_tools.cpp \
           egs_archive.cpp \
        ../egs_gui/egs_config_reader.cpp

HEADERS += egs_wizard.h \
           egs_location.h \
           egs_compilers.h \
           egs_licence.h \
           egs_install.h \
           egs_tools.h \
           egs_archive.h \
        ../egs_gui/egs_config_reader.h

MOC_DIR =       .moc/$$my_machine
OBJECTS_DIR =   .obj/$$my_machine
TARGET = egs_configuration

win32 {
    DEFINES += WIN32
    ####################################################
    # Needed by shortcut creation function to compile.
    ####################################################
    DEFINES -= UNICODE
    QMAKE_CXXFLAGS += -fpermissive
    ####################################################
    CONFIG  += qt thread warn_off release windows
    greaterThan(QT_MAJOR_VERSION, 4): LIBS += -lz -lole32 -luuid
    DESTDIR = ../../pieces/windows
    RC_FILE = egs_beam_install.rc
}

unix {
    CONFIG  += qt thread warn_on debug_and_release $$my_build
    LIBS    += -lz  #Needed if not using Qt own zlib, in QT5 using Qtzlibh is not recommended according to documentation
    #message("CONFIG = $$CONFIG")
    contains( CONFIG, shared ):message( "Dynamic build..." )
    contains( CONFIG, static ){
        message( "Static build ..." )
        DESTDIR = ../../pieces/linux
        UNAME = $$system(getconf LONG_BIT)
        contains( UNAME, 64 ){
           message( "-> 64 bit ($$UNAME)" )
           TARGET = egs_configuration_64
        }
        contains( UNAME, 32 ){
           message( "-> 32 bit ($$UNAME)" )
           TARGET = egs_configuration_32
        }
        QMAKE_POST_LINK = strip $(TARGET)
    }
}

INCLUDEPATH += ../egs_gui

RESOURCES = egs_beam_install.qrc # resource collection file to store images in the application executable.
TEMPLATE  = app
LANGUAGE  = C++
QT += xml
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
