
###############################################################################
#
#  EGSnrc makefile for the ctcreate utility
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
#  Contributors:    Blake Walters
#
###############################################################################
#
#  If you have have a working C compiler that manages to compile all DICOM
#  related C files, use 'make' to build ctcreate. If you don't have a C
#  compiler or you can not make your C compiler compile the DICOM related
#  C files, use 'make nodicom' to build ctcreate.
#
#  Note: some fortran compilers (e.g., IBM's XLF compiler) pass definitions to
#  the C-preprocessor differently (e.g. XLF needs -Wp,-DMACRO instead of just
#  -DMACRO). If you are blessed with such a compiler, you need to adjust the
#  variable define_dicom accordingly.
#
###############################################################################


include $(EGS_CONFIG)

OBJE = $(FOBJE)
OBJD = .obj$(DSEP)$(my_machine)
OBJDD = $(OBJD)$(DSEP)

defines =

define_dicom = -DDICOM

includes = -I$(HEN_HOUSE)lib$(DSEP)$(my_machine)

CFLAGS = $(C_FLAGS) $(defines) $(includes)

DOSXYZ_HOME = $(HEN_HOUSE)user_codes$(DSEP)dosxyznrc$(DSEP)

SOURCES = $(MACHINE_MACROS) $(DOSXYZ_HOME)dosxyznrc_user_macros.mortran \
          ctcreate.mortran $(EGS_SOURCEDIR)lnblnk1.mortran

#**************************************************************************

all: $(EGS_BINDIR)ctcreate$(EXE)

$(EGS_BINDIR)ctcreate$(EXE): ctcreate_$(my_machine).F ReadCT_DICOM.$(OBJE)
	$(F77) $(FCFLAGS) $(FOPT) $(FOUT)$@ ctcreate_$(my_machine).F ReadCT_DICOM.$(OBJE)

ctcreate_$(my_machine).F: $(SOURCES)
	$(MORTRAN_EXE) -d $(MORTRAN_DATA) -f $(SOURCES) -o7 $@ -o8 ctcreate_$(my_machine).mortlst

ReadCT_DICOM.$(OBJE): ReadCT_DICOM.c tags_ct.h
	$(CC) -c $(CFLAGS) $(COUT)$@ ReadCT_DICOM.c

