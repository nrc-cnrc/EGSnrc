
###############################################################################
#
#  EGSnrc makefile for the dosxyz_show utility
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
#  Author:          Iwan Kawrakow, 2004
#
#  Contributors:
#
###############################################################################
#
#  Ideally one would use a proper configuration with autoconf, etc. but given
#  that this program is not really maintained, and is not easily portable to
#  non-unix systems (because of Motif dependencies, etc.) for the time being I
#  am leaving it up to the user to adjust include and library directories.
#
###############################################################################


include $(EGS_CONFIG)

executable = $(EGS_BINDIR)dosxyz_show$(EXE)

X11_include = -I/usr/X11R6/include
# On our system, Motif include files are in /usr/X11R6/include/Xm =>
# we don't need a separate -I for Motif. If this is different on your
# system, adjust accordingly.
Motif_include =

X11_libdir = -L/usr/X11R6/lib
# On our system, the Motif library is /usr/X11R6/lib =>
# we don't need a separate -L for Motif. If this is different on your
# system, adjust accordingly.
Motif_libdir =

libs = -lXm -lXt -lX11 -lm

# If you want to use a different Hounsfield number to gray scale conversion,
# a different way of defining colors, etc., you need to adjust the
# defines macro below (see dosxyz_show.c for possible -D flags).
defines =

$(executable): dosxyz_show.c
	$(CC) -O2 $(defines) $(X11_include) $(Motif_include) -o $@ $(X11_libdir) $(Motif_libdir) dosxyz_show.c $(libs)
