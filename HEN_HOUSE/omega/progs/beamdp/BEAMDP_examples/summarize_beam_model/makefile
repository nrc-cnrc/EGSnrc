
###############################################################################
#
#  EGSnrc makefile for summarize beam model
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
#  Author:          Zdenko Sego, 2005
#
#  Contributors:    Iwan Kawrakow
#                   Blake Walters
#
###############################################################################


.SUFFIXES:
.SUFFIXES:	.o .cpp

CC	= g++
OBJS	= msmodel.o summarize_beam_model.o sources.o
HDRS	= msmodel.h sources.h abuf.h
CLEAN	= $(OBJS)

CFLAGS	= -o
LDFLAGS	=

summarize_beam_model: $(OBJS) makefile
	$(CC) -o $@ $(LDFLAGS) $(OBJS)

$(OBJS): $(HDRS) makefile

clean:
	rm -f $(CLEAN)
