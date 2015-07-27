
###############################################################################
#
#  EGSnrc makefile for the AAPM to Pinnacle format converter
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
#  Author:          Julie Zachman, 1996
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


aapm2pinnacle:    aapm2pinnacle.o removeCtrlM.o TokenValue.o SortTokenValue.o NewImage.o CT_Scan.o utils.o Z_list.o
	cc -g -o aapm2pinnacle aapm2pinnacle.o removeCtrlM.o TokenValue.o SortTokenValue.o NewImage.o CT_Scan.o utils.o Z_list.o

aapm2pinnacle.o: aapm2pinnacle.c aapm2pinnacle.h userdefs.h
	cc -g -c aapm2pinnacle.c

removeCtrlM.o: removeCtrlM.c
	cc -g -c removeCtrlM.c

SortTokenValue.o: SortTokenValue.c NewImage.o CT_Scan.o Z_list.o aapm2pinnacle.h userdefs.h
	cc -g -c SortTokenValue.c

TokenValue.o: TokenValue.c bitmask.h
	cc -g -c TokenValue.c

NewImage.o: NewImage.c userdefs.h Z_list.h
	cc -g -c NewImage.c

CT_Scan.o:    CT_Scan.c userdefs.h
	cc -g -c CT_Scan.c

Z_list.o:    Z_list.c userdefs.h
	cc -g -c Z_list.c

utils.o: utils.c Z_list.h byteorder.h
	cc -g -c utils.c

