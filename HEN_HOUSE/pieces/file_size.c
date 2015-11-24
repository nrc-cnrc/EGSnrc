/*
###############################################################################
#
#  EGSnrc file size functions
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
#  Contributors:    Iwan Kawrakow
#
###############################################################################
#
#  Print the size of a file. Used for determining the record length of
#  unformatted I/O in fortran.
#
###############################################################################
*/


#include <stdio.h>
#include <stdlib.h>

#ifdef STDCALL
#define C_CONVENTION __stdcall
#else
#define C_CONVENTION
#endif

int fsize(const char* fname) {
  FILE* fp;
  long the_size;
  if((fp=fopen(fname, "rb")) == NULL ){
     printf("Error opening %s.\n", fname);
  }
  do {
   getc(fp);
  }while(!feof(fp));
  the_size = ftell(fp);
  fclose(fp);
  /*printf("%i\n", the_size);*/
  return the_size;
}
int C_CONVENTION file_size(  const char* fname, int len) {return fsize(fname);}
int C_CONVENTION file_size_( const char* fname, int len) {return fsize(fname);}
int C_CONVENTION file_size__(const char* fname, int len) {return fsize(fname);}
int C_CONVENTION FILE_SIZE(  const char* fname, int len) {return fsize(fname);}
int C_CONVENTION FILE_SIZE_( const char* fname, int len) {return fsize(fname);}
int C_CONVENTION FILE_SIZE__(const char* fname, int len) {return fsize(fname);}

