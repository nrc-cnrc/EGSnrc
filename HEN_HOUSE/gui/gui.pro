
###############################################################################
#
#  EGSnrc Qt project file to build all graphical user interfaces
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
#  Author:          Ernesto Mainegra-Hing, 2003
#
#  Contributors:
#
###############################################################################
#
#  Project file to build the whole EGSnrc GUI suite of codes. It uses the
#  template 'subdirs' to have qmake generating Makefiles in the subdirectories
#  for the existing project files a call make on it.
#
###############################################################################


TEMPLATE    =	subdirs
SUBDIRS	=	egs_configure \
		egs_gui \
		egs_inprz
