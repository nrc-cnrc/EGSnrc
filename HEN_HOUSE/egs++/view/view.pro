
###############################################################################
#
#  EGSnrc egs++ geometry viewer Qt project file
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

TEMPLATE	= app
LANGUAGE	= C++

INCLUDEPATH	+= . .. ../../lib/$$my_machine

HEADERS	+= egs_visualizer.h image_window.h egs_light.h \
                 clippingplanes.h viewcontrol.h geometryview.ui.h \
                 saveimage.h egs_user_color.h egs_track_view.h \
                 renderworker.h

SOURCES	+= main.cpp egs_visualizer.cpp egs_track_view.cpp \
                 saveimage.cpp clippingplanes.cpp viewcontrol.cpp \
                 renderworker.cpp image_window.cpp

FORMS           = saveimage.ui clippingplanes.ui viewcontrol.ui 

win32 {
    CONFIG	+= qt warn_off release windows exceptions_off thread
    DEFINES += WIN32
    DEFINES += VDEBUG
    RC_FILE = egs_view.rc
    LIBS	+= ../dso/$$my_machine/egspp.lib
    DESTDIR = ../dso/$$my_machine
    TARGET = egs_view
}

unix {
    CONFIG    += qt warn_on release $$my_build
    macx {
        LIBS  += -L../dso/$$my_machine -legspp
        TARGET = ../../bin/$$my_machine/egs_view
    }
    !macx {
        DESTDIR = ../../bin/$$my_machine/
       !contains( CONFIG, static ){
         message( "Dynamic build..." )
         TARGET = egs_view
         LIBS += -L../dso/$$my_machine -Wl,-rpath,$$hhouse/egs++/dso/$$my_machine -legspp
        }
        contains( CONFIG, static ){
            message( "Static build ..." )
            DESTDIR = ../../pieces/linux
            #LIBS += -L../dso/$$my_machine -Wl,-rpath,$$hhouse/egs++/dso/$$my_machine -legspp # Fixes path to library
            LIBS += -L$$hhouse/egs++/dso/$$my_machine -legspp                                 # Relies on LD_LIBRARY_PATH
            UNAME = $$system(getconf LONG_BIT)
            contains( UNAME, 64 ){
               message( "-> 64 bit ($$SNAME)" )
               TARGET = egs_view_64
            }
            contains( UNAME, 32 ){
               message( "-> 32 bit ($$SNAME)" )
               TARGET = egs_view_32
            }
            QMAKE_POST_LINK = strip $(TARGET)
        }
    }
}

# Debug options
#DEFINES += VIEW_DEBUG
#QMAKE_CXXFLAGS+="-fsanitize=address -fno-omit-frame-pointer"
#QMAKE_CXXFLAGS+="-ggdb3"
#QMAKE_LFLAGS+="-fsanitize=address"

UI_DIR = .ui/$$my_machine
MOC_DIR = .moc/$$my_machine
OBJECTS_DIR = .obj/$$my_machine


