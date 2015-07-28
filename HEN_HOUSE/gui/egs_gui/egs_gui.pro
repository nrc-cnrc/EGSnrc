
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
#
###############################################################################


SOURCES	+= main.cpp \
	main_widget.cpp \
	egs_compile_page.cpp \
	egs_run_page.cpp \
	egs_gui_widget.cpp \
	egs_configuration_page.cpp \
	my_pixmap.cpp \
	element_item.cpp \
	egs_config_reader.cpp

HEADERS	+= main_widget.h \
	egs_compile_page.h \
	egs_run_page.h \
	egs_gui_widget.h \
	egs_configuration_page.h \
	my_pixmap.h \
	element_item.h \
	egs_config_reader.h

MOC_DIR =       .moc/$$my_machine
OBJECTS_DIR =   .obj/$$my_machine
DESTDIR  = ../../bin/$$my_machine

win32 {
    DEFINES += WIN32
    CONFIG  += qt thread warn_off release windows
    RC_FILE = egs_gui.rc
}

unix {
    CONFIG  += qt thread warn_off release $$my_build
    contains( CONFIG, shared ):message( "Dynamic build..." )
    contains( CONFIG, static ){
        message( "Static build ..." )
        DEFINES += STATICGUI
        LIBS+=  ../../bin/$$my_machine/libegsconfig.a
        INCLUDEPATH += ../egs_install/include ../egs_configure
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

FORMS	= pegs_page.ui \
	pegs_runoutput.ui \
	pegs_viewerrors.ui \
	aboutform.ui
IMAGES	= images/file-manager.png \
	images/contents.png \
	images/launch.png \
	images/make_kdevelop.png \
	images/wizard-32.png \
	images/configure.png \
	images/rocket_egg_tr_f1.png \
        images/desktop_icon.png \
        images/rocket_egg_tr_f1_300.png

TEMPLATE	=app
LANGUAGE	= C++
