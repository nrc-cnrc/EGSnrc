/*
###############################################################################
#
#  EGSnrc functions to read and write partial dose files
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
#  Author:          Blake Walters, 1998
#
#  Contributors:    Iwan Kawrakow
#
###############################################################################
#
#  Blake Walters, 2004:
#
#  This file replaces files write_pardose.c and read_pardose.c. Since 2004 the
#  .pardose files are automatically recombined at the end of a parallel run
#  or when the user recombines them manually by running dosxyznrc with the
#  input flag IRESTART=4.
#
###############################################################################
*/


#include <stdio.h>
#include <stdlib.h>
#include "egs_config1.h"

#ifdef __cplusplus
extern "C" {
#endif

void F77_OBJ_(write_pardose,WRITE_PARDOSE)(
           double *temp2, int *imax, int *jmax, int *kmax,
           double *endep, double *endep2, const char *fname) {

    FILE *fp;
    int i,j,k;
    int ensize;
    float *sendep,*sendep2;

    if( !fname ) {
        printf("\n Null fname passed to write_pardose!\n");
        return;
    }
    ensize=(*imax)*(*jmax)*(*kmax);
    sendep = (float *) malloc(ensize*sizeof(float));
    sendep2 = (float *) malloc(ensize*sizeof(float));

    /*** open file for writing ***/

    if ((fp=fopen( fname,"wb"))==NULL) {
        printf("\n write_pardose: cannot open file %s\n",fname);
        return; /*exit(1);*/
    }

    if(fwrite((void *)temp2,
                sizeof(double),1,fp)!=1)
        printf("\n Error writing temp2. \n");

    if(fwrite((int *)imax,
                sizeof(int),1,fp)!=1)
        printf("\n Error writing imax. \n");

    if(fwrite((int *)jmax,
                sizeof(int),1,fp)!=1)
        printf("\n Error writing jmax. \n");

    if(fwrite((int *)kmax,
                sizeof(int),1,fp)!=1)
        printf("\n Error writing kmax. \n");

    for (i = 0; i < (*imax)*(*jmax)*(*kmax); i++) {
        sendep[i]=endep[i];
        sendep2[i]=endep2[i];
    }

    if(fwrite((float *)sendep,
                sizeof(float),ensize,fp)!=ensize)
        printf("\n Error writing endep. \n");

    if(fwrite((float *)sendep2,
                sizeof(float),ensize,fp)!=ensize)
        printf("\n Error writing endep2. \n");

    fclose(fp);
    free(sendep); free(sendep2);

    return;

}

void F77_OBJ_(read_pardose,READ_PARDOSE)(
           double *temp2, int *imax, int *jmax, int *kmax,
           float *endep, float *endep2, const char *fname) {

FILE *fp;
int i,j,k;
int ensize,xbsize,ybsize,zbsize;
float *sendep,*sendep2;

  /*** open file for reading ***/

  if ((fp=fopen(fname,"rb"))==NULL) {
    printf("\n Cannot open file \n");
    exit(1);
  }

  if(fread(temp2, sizeof(double),1,fp)!=1)
       printf("\n Error reading temp2. \n");

  if(fread((int *)imax,
      sizeof(int),1,fp)!=1)
       printf("\n Error reading imax. \n");

  if(fread((int *)jmax,
      sizeof(int),1,fp)!=1)
       printf("\n Error reading jmax. \n");

  if(fread((int *)kmax,
      sizeof(int),1,fp)!=1)
       printf("\n Error reading kmax. \n");

   /*** allocate memory for sendep,sendep2 ***/

  ensize=(*imax)*(*jmax)*(*kmax);
  sendep = (float *) malloc(ensize*sizeof(float));
  sendep2 = (float *) malloc(ensize*sizeof(float));

  if(fread((float *)sendep,
      sizeof(float),ensize,fp)!=ensize)
       printf("\n Error reading endep. \n");

  if(fread((float *)sendep2,
      sizeof(float),ensize,fp)!=ensize)
       printf("\n Error reading endep2. \n");

  for (i = 0; i < (*imax)*(*jmax)*(*kmax); i++) {
        endep[i]=sendep[i];
        endep2[i]=sendep2[i];
  }

  fclose(fp);
  free(sendep);free(sendep2);
  return;
}

#ifdef __cplusplus
}
#endif
