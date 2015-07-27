/*
###############################################################################
#
#  EGSnrc DICOM format reader
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
#  Author:          Nick Rynaert, 2004
#
#  Contributors:    Blake Walters
#                   Iwan Kawrakow
#
###############################################################################
#
#  The contributors named above are only those who could be identified from
#  this file's revision history.
#
#  This code is part of the BEAMnrc code system for Monte Carlo simulation of
#  radiotherapy treatments units. BEAM was originally developed at the
#  National Research Council of Canada as part of the OMEGA collaborative
#  research project with the University of Wisconsin, and was originally
#  described in:
#
#  BEAM: A Monte Carlo code to simulate radiotherapy treatment units,
#  DWO Rogers, BA Faddegon, GX Ding, C-M Ma, J Wei and TR Mackie,
#  Medical Physics 22, 503-524 (1995).
#
#  BEAM User Manual
#  DWO Rogers, C-M Ma, B Walters, GX Ding, D Sheikh-Bagheri and G Zhang,
#  NRC Report PIRS-509A (rev D)
#
#  As well as the authors of this paper and report, Joanne Treurniet of NRC
#  made significant contributions to the code system, in particular the GUIs
#  and EGS_Windows. Mark Holmes, Brian Geiser and Paul Reckwerdt of Wisconsin
#  played important roles in the overall OMEGA project within which the BEAM
#  code system was developed.
#
#  There have been major upgrades in the BEAM code starting in 2000 which
#  have been heavily supported by Iwan Kawrakow, most notably: the port to
#  EGSnrc, the inclusion of history-by-history statistics and the development
#  of the directional bremsstrahlung splitting variance reduction technique.
#
###############################################################################
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "egs_config1.h"
#include "tags_ct.h"
#define MAX_SLICES 300

/* below defines endianness of machine that DICOM files
were collected on, 1 = little endian (the usual case).  If
you collected data on a big endian machine, change this to 0 */
#define DICOM_ENDIAN 1

/*****************************************************************\
|*                                                               *|
|*  Types.                                                       *|
|*                                                               *|
\*****************************************************************/

/*`````````````````````````````````````````````````\
|  Integer types of specified byte length.         |
\.................................................*/

typedef unsigned short UINT2;
typedef short          INT2;
typedef unsigned int   UINT4;
typedef int            INT4;

/*`````````````````````````````````````````````````\
|   Dicom Element header :                         |
|   for each entry in a dicom file, an element     |
|   header is used.                                |
\.................................................*/

typedef struct
{
  UINT4 tag;
  UINT4 len;
} Elemh;

typedef union {
  UINT4   i4;
  char    c4[4];
  UINT2   i2;
  char    c2[2];
} SwapHelper;

void swap_bytes_4(SwapHelper *s) {
  char c;
  c = s->c4[0]; s->c4[0] = s->c4[3]; s->c4[3] = c;
  c = s->c4[1]; s->c4[1] = s->c4[2]; s->c4[2] = c;
}

void swap_bytes_2(SwapHelper *s) {
  char c;
  c = s->c2[0]; s->c2[0] = s->c2[1]; s->c2[1] = c;
}


/*****************************************************************\
|*                                                               *|
|*  Global variables for reading dicom file.                     *|
|*  This buffer (buffle) is used to read element values.         *|
|*                                                               *|
\*****************************************************************/

char *buffle=NULL;
short *buffle2=NULL;
int hwm_buffle=0;   /* available memory of buffle storage. */
double doubles[600000];
int ktel=0;
int endian, swap;

/* minimum number of bytes by which the buffer will be expanded. */
#define MADDLEN_BUFFEL  (1000)

int read_elem (Elemh *h, FILE *fi)
{
  UINT2 gtag,etag;

  /* read tag (group tag and element tag) and element value length. */
  if( fread((void*)&gtag,sizeof(UINT2),1,fi)   != 1  ||
      fread((void*)&etag,sizeof(UINT2),1,fi)   != 1  ||
      fread((void*)&h->len,sizeof(UINT4),1,fi) != 1
     )
    return -1;
  /* join group & element tags. */
  h->tag = (gtag << 16 ) | etag;
  printf(" h->len,tag here %i %x \n",h->len,h->tag);
  return 0;
}

int read_elem_vr (Elemh *h, FILE *fi)
{
  UINT2 gtag,etag,vlen2;
  UINT4 vlen4;
  SwapHelper sh;
  long int vrposn;
  char *vr;

  vr=NULL;

  /* read tag (group tag and element tag) and element value length. */
  if( fread((void*)&gtag,sizeof(UINT2),1,fi)   != 1  ||
      fread((void*)&etag,sizeof(UINT2),1,fi)   != 1
    )
    return -1;

  if(swap==1)
  {
     sh.i2 = gtag; swap_bytes_2(&sh); gtag = sh.i2;
     sh.i2 = etag; swap_bytes_2(&sh); etag = sh.i2;
  }

  /* successfully read group & element tags */
  h->tag = (gtag << 16 ) | etag;

  /* try reading value representation (vr) code */
  /* but first save this file position in case vr is implicit */
  vrposn=ftell(fi);

  if( fread(&vr,1,2,fi) != 2 )
    return -1;

  /* successfully read in what we think is vr, check this */
  /* will have to change possible values of vr as DICOM evolves */
  if(strcmp((char*)&vr,"AE") != 0 && strcmp((char*)&vr,"AS") != 0 &&
     strcmp((char*)&vr,"AT") != 0 && strcmp((char*)&vr,"CS") != 0 &&
     strcmp((char*)&vr,"DA") != 0 && strcmp((char*)&vr,"DS") != 0 &&
     strcmp((char*)&vr,"DT") != 0 && strcmp((char*)&vr,"FL") != 0 &&
     strcmp((char*)&vr,"FD") != 0 && strcmp((char*)&vr,"IS") != 0 &&
     strcmp((char*)&vr,"LO") != 0 && strcmp((char*)&vr,"LT") != 0 &&
     strcmp((char*)&vr,"OB") != 0 && strcmp((char*)&vr,"OW") != 0 &&
     strcmp((char*)&vr,"PN") != 0 && strcmp((char*)&vr,"SH") != 0 &&
     strcmp((char*)&vr,"SL") != 0 && strcmp((char*)&vr,"SQ") != 0 &&
     strcmp((char*)&vr,"SS") != 0 && strcmp((char*)&vr,"ST") != 0 &&
     strcmp((char*)&vr,"TM") != 0 && strcmp((char*)&vr,"UI") != 0 &&
     strcmp((char*)&vr,"UL") != 0 && strcmp((char*)&vr,"UN") != 0 &&
     strcmp((char*)&vr,"US") != 0 && strcmp((char*)&vr,"UT") != 0 ) {

     /* not a valid VR value, VR must be implicit, step back in file */
     if (fseek(fi,vrposn,0) != 0) {
         printf(" Error setting file position back in read_elem_vr\n");
         return -1;
     }

     /* now read the value length */
     if(fread((void*)&vlen4,sizeof(UINT4),1,fi) != 1)
       return -1;
     if(swap==1)
     {
        sh.i4 = vlen4; swap_bytes_4(&sh); vlen4 = sh.i4;
     }
     h->len=vlen4;
  }
  else if(strcmp((char*)&vr,"OB") == 0 || strcmp((char*)&vr,"OW") == 0 ||
          strcmp((char*)&vr,"SQ") == 0 || strcmp((char*)&vr,"UN") == 0 ) {
     if(fread((void*)&vlen2,sizeof(UINT2),1,fi) !=1 ||
        fread((void*)&vlen4,sizeof(UINT4),1,fi) !=1 )
       return -1;
     if(swap==1)
     {
        sh.i4 = vlen4; swap_bytes_4(&sh); vlen4 = sh.i4;
     }
     h->len=vlen4;
  }
  else {
     /* these store length in 2 byte mode */
     if(fread((void*)&vlen2,sizeof(UINT2),1,fi) !=1)
       return -1;
     if(swap==1)
     {
        sh.i2 = vlen2; swap_bytes_2(&sh); vlen2 = sh.i2;
     }
     h->len=vlen2;
  }

 /* printf(" tag,vr,length here %x %s %i\n",h->tag,&vr,h->len); */

  return 0;
}


/*****************************************************************\
|*                                                               *|
|*  Read element value of specified length in buffle.            *|
|*                                                               *|
\*****************************************************************/

int read_val (FILE *fi, UINT4 len, UINT4 tag)
{
  /* memory expansion. */
  if(len>=hwm_buffle)
    {
      if(len-hwm_buffle < MADDLEN_BUFFEL)
        hwm_buffle += MADDLEN_BUFFEL;
      else
        hwm_buffle = len+1;
      if(buffle==NULL)
        {
	  if( (buffle = (void*)malloc(hwm_buffle)) == NULL )
	    {
	      fputs(" Cannot alloc a sufficient amount of memory.\n",stderr);
	      exit(1);
	      return -1;
	    }
	}
      else
	{
	  if( (buffle = (void*)realloc(buffle,hwm_buffle)) == NULL )
	    {
	      fputs(" Cannot alloc a sufficient amount of memory.\n",stderr);
	      exit(1);
	      return -1;
	    }
	}
    }
  /* read data. */
  if(fread((char*)buffle,1,len,fi) != len)
   {
     fputs(" Error reading dicom file.",stderr);
     return -1;
   }
   /* add extra nul-char to delimit string. */
   ((char*)buffle)[len]=0;
  if((tag == columns_tag) || (tag == rows_tag) || (tag == pixeldata_tag))
  {
   buffle2=(short*)buffle;
  }
  return 0;
}


/*****************************************************************\
|*                                                               *|
|*  Get doubles separated by a backslash from a string.          *|
|*                                                               *|
\*****************************************************************/

int string_to_doubles (const char *s,int num_doub)
{
  const char *c=s;
  char *endptr;
  /* find backslash. */
  do if(*c == 0) return -1; while( *c++ != '\\' );
  /* found it. */
  if(--c==s) return -1;
  /* get first double. */
  doubles[ktel] = strtod(s,&endptr);
  if(endptr != c) return -1;
  ktel++;
  if(num_doub == 2)
  {
   /* get last double */
   doubles[ktel] = strtod(++c,&endptr);
   /*if(*endptr != 0) return -1;*/
   return 0;
  }
  else
  {
   string_to_doubles(++c,num_doub-1);
  }
  return 0;
}


F77_OBJ_(readct_dicom,READCT_DICOM)(char *ct_filename, int ct_arraysize[3],
		                      short *ctdata, float ct_offset[3],
	                      	float ct_voxelsize[3],int *ct_errorcode)
{
  int nrows,ncolumns,jump=0,prev,itel;
  int ij=0, iresol_half=0, i=0, j=0, num_slices, slice, count, num_overlap=0;
  int slice_thick_def=0, slice_loc_def=0, ct_negative=0, zerocount;
  int offset=0;
  long jtel=-1, mtel;
  float slice_location[MAX_SLICES],ct_offset_tmp[3];
  float rescale_slope,rescale_intercept,diff;
  int ct_orient[10];
  FILE *file_ptr,*fi;
  int next_slice[MAX_SLICES],first_slice,nxt;
  Elemh h;
  char
    *slicename[MAX_SLICES],
    filelist[MAX_SLICES*80] = "",
    ch;
  float z_final;
  int nl = 0x12345678;
  unsigned char *p = (unsigned char *)(&nl);
  SwapHelper sh;

  if (DICOM_ENDIAN==1)
  {
    printf("\n DICOM data is little endian.");
  }
  else
  {
    printf("\n DICOM data is big endian.");
  }

  /* determine the endianness of the machine ctcreate is running on*/
  if ( p[0] == 0x12 && p[1] == 0x34 && p[2] == 0x56 && p[3] == 0x78 )
  {
    endian = 0;
    printf("\n Machine is big endian...");
  }
  else {
    if ( p[0] == 0x78 && p[1] == 0x56 && p[2] == 0x34 && p[3] == 0x12 )
    {
       endian = 1;
       printf("\n Machine is little endian...");
    }
    else {
      printf("\n Machine has unsupported endiannes\n");
      exit(1);
    }
  }

  if (DICOM_ENDIAN==endian)
  {
    swap=0;
    printf("bytes will not be swapped\n\n");
  }
  else
  {
    swap=1;
    printf("bytes will be swapped\n\n");
  }

  /* add NULL at end */
  zerocount=0;
  first_slice=0;
  i=0;
  *ct_errorcode=0;
  while (ct_filename[i] != ' ')
    {
      i++;
    }
  ct_filename[i] = '\0';

  /* open file containing CT filenames */
  file_ptr = fopen(ct_filename, "r");
  if (file_ptr == NULL)
    {
      printf("Failed to open file %s in ReadCT_DICOM\n",ct_filename);
      *ct_errorcode=1;
      exit(1);
    }

  ch = fgetc(file_ptr);

  /* read all filenames into filelist */
  i = 0;
  while (!feof(file_ptr))
    {
      filelist[i] = ch;
      i++;
      ch = fgetc(file_ptr);
    }
  filelist[i] = '\0';
  fclose(file_ptr);

  /* find individual files */
  i = 0;
  j = 0;
  slicename[j] = &filelist[i];
  j++;
  while (filelist[i] != '\0')
    {
      if (filelist[i] == '\n')
	{
	  slicename[j] = &filelist[i+1];
	  filelist[i] = '\0';
	  j++;
	}
      i++;
    }
  num_slices = j-1;
  count=-1;
 /* first read all files and sort them */
  printf(" Image slices being ordered in increasing Z\n");
  for (slice = 0; slice < num_slices; slice++)
  {
    count++;
    if( (fi=fopen(slicename[count],"rb")) == NULL )
    {
         fprintf(stderr,"Error opening file %s.\n",slicename[count]);
    }

    while( (jump == 0) && (read_elem_vr(&h,fi) == 0) )
    {
       if( (h.len == 0) && (h.tag == 0))
          zerocount++;  /* could be part of 128 byte header */
       if( zerocount==16 )  /* read the characters DICM */
       {
           if( read_val(fi,4,0) < 0 )
           {
              fprintf(stderr," Error reading dicom file.");
              break;
           }
           zerocount=0;
       }
       else if( (h.len > 0) && (h.len < 0xFFFFFFFF))
       {
          if( read_val(fi,h.len,h.tag) < 0 )
          {
              fprintf(stderr," Error reading dicom file.");
              break;
          }
          switch( h.tag )
          {
            case image_sequence_tag:
              if(strstr(buffle,"LOCALIZER") != NULL)
              {
                 slice-=1;
                 num_slices-=1;
                 printf(" slice %s is a localizer, counter reset to %d\n",
                            slicename[count],slice);
                 jump=1;
                 continue;
              }
              break;

            case image_position_tag:
              ktel=0;
              if( string_to_doubles(buffle,3 ) < 0)
              {
                 fprintf(stderr," Invalid image_position for slice %i: %s\n",slice,buffle);
                 *ct_errorcode=1;
                 exit(1);
              }
              else
              {
                 slice_loc_def=1;
                 //define Z position of slice
                 slice_location[count]=doubles[2]/10.0;
                 if(slice==0)
                 { //first time through loop, define first slice
                   first_slice=count;
                   next_slice[count]=-2;
                 }
                 else
                 {
                   i=first_slice;
                   while(slice_location[i] < slice_location[count])
                   {
                     if(next_slice[i] == -2) { break;}
                     else
                     {
                       prev=i;
                       i=next_slice[i];
                     }
                   }
                   if((i == first_slice) && (slice_location[i] > slice_location[count]))

                   { //slice becomes first slice
                     first_slice=count;
                     next_slice[count]=i;
                   }
                   else if(next_slice[i] == -2 && slice_location[i] < slice_location[count])
                   { //add to the end of the list
                     if(slice_location[count] == slice_location[i])/*total overlap*/
                     {
                       num_overlap+=1;
                       printf("slice %d removed due to overlap\n",count);
                     }
                     else
                     {
                       next_slice[i]=count;
                       next_slice[count]=-2;
                     }
                   }
                   else
                   { //add between two slices
                     if(slice_location[count] == slice_location[i])/*total overlap*/
                     {
                       num_overlap+=1;
                       printf("slice %d removed due to overlap\n",count);
                     }
                     else
                     {
                       next_slice[count]=next_slice[prev];
                       next_slice[prev]=count;
                     }
                   }
                 }

              }
              jump=1; //tells us to close the file, we have what we
                      //came in for
              break;

            default:
              break;
          }
       }
    }
    if(jump==1)
    {
      jump=0; //prepare for reading next slice
      if( fclose(fi) == EOF )
      {
       fprintf(stderr,"Error closing file %s.\n",slicename[count]);
      }
    }
  }

  //can now calculate Z voxel size using first 2 slices
  ct_voxelsize[2] = slice_location[next_slice[first_slice]]-slice_location[first_slice];
  printf(" -----------------------------------------------\n");
  printf(" From image positions of first two slices, slice thickness = %f\n",ct_voxelsize[2]);

  num_slices-=num_overlap;
  ct_arraysize[2]=num_slices;
  slice=first_slice;
  do
  {
      printf("-------------------------------\n");
      printf("\nWorking on file   : %s\n", slicename[slice]);

      if( (fi=fopen(slicename[slice],"rb")) == NULL )
      {
         fprintf(stderr,"Error opening file %s.\n",slicename[slice]);
      }

      /* read elements until we don't find any further elements. */
      /* first read element header. */
      while(read_elem_vr(&h,fi) == 0)
      {
       if( (h.len == 0) && (h.tag == 0))
          zerocount++;  /* could be part of 128 byte header */
       if( zerocount==16 )  /* read the characters DICM */
       {
           if( read_val(fi,4,0) < 0 )
            {
              fprintf(stderr," Error reading dicom file.");
              break;
            }
           zerocount=0;
       }
       else if( (h.len > 0) && (h.len < 0xFFFFFFFF))
 	{ /* element value (unread so far) has more than 0 bytes. */
 	  if( read_val(fi,h.len,h.tag) < 0 )
 	    {
 	      fprintf(stderr," Error reading dicom file.");
 	      break;
 	    }
 	}
       /* handle different tags. */
       if(slice == first_slice)
       {
        switch( h.tag )
	{
	case image_orientation_tag:
	  ktel=0;
	  if( string_to_doubles(buffle,6 ) < 0)
	  {
	   fprintf(stderr,"Invalid image orientation: %s\n",buffle);
	   *ct_errorcode=1;
	   exit(1);
	  }
	  else
          {
           for(ij=0;ij<6;ij++) { ct_orient[ij]=(int)doubles[ij];}
	   ct_orient[8]=ct_orient[0]*ct_orient[4];
	   printf(" Image orientation: %i %i %i\n",ct_orient[0],ct_orient[4],
			   ct_orient[8]);
	  }
	  break;

	case rows_tag:
	  nrows = *buffle2;
          if(swap==1)
          {
            sh.i2 = nrows; swap_bytes_2(&sh); nrows = sh.i2;
          }
	  /* if(nrows>300)  do not know what this is here for
          {
	    nrows=nrows/2;
	    iresol_half=1;
	  }
	  else
          {
	   iresol_half=0;
	  } */
	  ct_arraysize[0]=nrows;
	  printf(" nrows : %i\n",nrows);
	  break;

	case columns_tag:
	  ncolumns = *buffle2;
          if(swap==1)
          {
            sh.i2 = ncolumns; swap_bytes_2(&sh); ncolumns = sh.i2;
          }
	  if(iresol_half==1)
	  {
	   ncolumns=ncolumns/2;
	  }
	  ct_arraysize[1]=ncolumns;
	  printf(" ncols : %i \n",ncolumns);
	  break;

	case image_position_tag:
	  ktel=0;
	  if( string_to_doubles(buffle,3 ) < 0)
	  {
	   fprintf(stderr," Invalid image_position: %s\n",buffle);
	   *ct_errorcode=1;
	   exit(1);
	  }
	  else
          {
           ct_offset[0]=doubles[0]/10.0;
           ct_offset[1]=doubles[1]/10.0;
           ct_offset[2]=doubles[2]/10.0;
	   z_final=ct_offset[2]+ct_voxelsize[2]/2.0;
	   printf("\n Image position: %f %f %f\n",ct_offset[0],ct_offset[1],
			   ct_offset[2]);
          }
	  break;

	case pixel_spacing_tag:
	  ktel=0;
	  if( string_to_doubles(buffle,2 ) < 0)
	  {
	   fprintf(stderr,"Invalid voxel sizes: %s\n",buffle);
	   *ct_errorcode=1;
	   exit(1);
	  }
	  else
          {
	   if(iresol_half==1)
	   {
	    ct_voxelsize[0]=doubles[0]/5.0;  /* mm to cm *2 */
	    ct_voxelsize[1]=doubles[1]/5.0;
	    ct_offset[0]+=ct_voxelsize[0]/4.0;
	    ct_offset[1]+=ct_voxelsize[1]/4.0;
            //Note: above is a potential problem, since it is possible that we have not
            //defined ct_offset yet, depending on order of tags
            //luckily, coding which can set iresol_half=1 has been commented out
	   }
	   else
	   {
	    ct_voxelsize[0]=doubles[0]/10.0;
	    ct_voxelsize[1]=doubles[1]/10.0;
            //ct_voxelsize[2] already defined when sorting slices above
	   }
	   printf(" voxelsizes : %f %f\n",ct_voxelsize[0],ct_voxelsize[1]);
	  }
	  break;

        case rescale_intercept_tag:
          rescale_intercept=atof(buffle);
          printf(" intercept for data conversion: %f\n",rescale_intercept);
          break;

        case rescale_slope_tag:
          rescale_slope=atof(buffle);
          printf(" slope for data conversion: %f\n",rescale_slope);
          break;

	case pixeldata_tag:
	   if(iresol_half==1)
           {
	    for(i=0;i<(nrows*2);i++)
	    {
             for(j=0;j<(ncolumns*2);j++)
             {
	      ij=i*ncolumns*2+j;
	      jtel++;
	      ctdata[jtel]=(int)(0.25*(buffle2[ij]+buffle2[ij+1]+
	      buffle2[ij+ncolumns*2]+buffle2[ij+ncolumns*2+1]));
              if(swap==1)
              {
                /* swap all the CT data */
                sh.i2 = ctdata[jtel]; swap_bytes_2(&sh); ctdata[jtel] = sh.i2;
              }
              if(ctdata[jtel]<ct_negative) ct_negative=ctdata[jtel];
	      j++;
	     }
	     i++;
	    }
	   }
	   else
           {
            for(i=0;i<nrows;i++)
            {
             for(j=0;j<ncolumns;j++)
             {
	      jtel++;
	      ij=i*ncolumns+j;
	      ctdata[jtel]=buffle2[ij];
              if(swap==1)
              {
                /* swap all the CT data */
                sh.i2 = ctdata[jtel]; swap_bytes_2(&sh); ctdata[jtel] = sh.i2;
              }
              if(ctdata[jtel]<ct_negative) ct_negative=ctdata[jtel];
	     }
	    }
	   }
	  break;

	default:
	  /* found an unrecognized element. No problem, skip. */
	  /* printf("Unrecognized tag : %X\n",h.tag); */
	  break;
	}
       }
       else
       {
       switch( h.tag )
	{

        case image_position_tag:
        /* this is only here to determine whether the difference between
           the Z-position of this slice and the previous one is different
           than the previously-defined slice thickness */
          ktel=0;
          if( string_to_doubles(buffle,3 ) < 0)
          {
           fprintf(stderr," Invalid image_position: %s\n",buffle);
           *ct_errorcode=1;
           exit(1);
          }
          else
          {
           ct_offset_tmp[0]=doubles[0]/10.0;
           ct_offset_tmp[1]=doubles[1]/10.0;
           ct_offset_tmp[2]=doubles[2]/10.0;
           z_final+=ct_voxelsize[2];
           printf("\n Image position: %f %f %f\n",ct_offset[0],ct_offset[1],
                           z_final-ct_voxelsize[2]/2.);
           diff=z_final-ct_voxelsize[2]/2.-ct_offset_tmp[2];
           if(diff > 0.0001 || diff < -0.0001){
             printf(" Warning in slice %i:\n",slice);
             printf("   Z from image position = %f\n",ct_offset_tmp[2]);
             printf("   Z relative to first slice = %f\n",z_final-ct_voxelsize[2]/2.);
             printf(" Will use latter value since it is derived based on constant Z-voxel dimension.\n");
           }
          }
          break;

	case pixeldata_tag:
	   if(iresol_half==1)
           {
	    for(i=0;i<(nrows*2);i++)
	    {
             for(j=0;j<(ncolumns*2);j++)
             {
	      ij=i*ncolumns*2+j;
	      jtel++;
	      ctdata[jtel]=(int)(0.25*(buffle2[ij]+buffle2[ij+1]+
	      buffle2[ij+ncolumns*2]+buffle2[ij+ncolumns*2+1]));
              if(swap==1)
              {
                /* swap all the CT data */
                sh.i2 = ctdata[jtel]; swap_bytes_2(&sh); ctdata[jtel] = sh.i2;
              }
              if(ctdata[jtel]<ct_negative) ct_negative=ctdata[jtel];
	      j++;
	     }
	     i++;
	    }
	   }
	   else
           {
	    for(i=0;i<nrows;i++)
	    {
             for(j=0;j<ncolumns;j++)
             {
	      ij=i*ncolumns+j;
              jtel++;
	      ctdata[jtel]=buffle2[ij];
              if(swap==1)
              {
                /* swap all the CT data */
                sh.i2 = ctdata[jtel]; swap_bytes_2(&sh); ctdata[jtel] = sh.i2;
              }
              if(ctdata[jtel]<ct_negative) ct_negative=ctdata[jtel];
	     }
	    }
            /*printf("ctdata: %d %d %d\n",i,buffle2[i],ctdata[jtel+i]);*/
	   }
	  break;

	default:
	  /* found an unrecognized element. No problem, skip. */
	  /* printf("Unrecognized tag : %X\n",h.tag); */
	  break;
	}
       }

    }
    /* close input file. */
    printf(" Finished reading file %s.\n",slicename[slice]);
    if( fclose(fi) == EOF )
    {
      fprintf(stderr,"Error closing file %s.\n",slicename[slice]);
    }
    prev=slice;
    slice=next_slice[slice];
 } while (next_slice[prev] != -2);

/*
 if(ct_negative<-1000) offset=1500;
 else if(ct_negative<0) offset=1000;
 offset=2000;
 if(offset>0){
   printf("\n CT data starts at %i.\n",-1*offset);
   printf(" Will offset by %i to make it all +ve.\n\n",offset);
   for(mtel=0;mtel<=jtel;mtel++) ctdata[mtel]+=offset;
 }
*/

 if(rescale_slope != 0 & rescale_intercept !=0){
    printf("\n CT data will be scaled by %f and shifted by %f\n",rescale_slope,
          rescale_intercept);
    for(mtel=0;mtel<=jtel;mtel++) ctdata[mtel]=ctdata[mtel]*rescale_slope+rescale_intercept;
 }

 ij=first_slice;
 itel=0;

 //subtract half a voxel to get lower X,Y,Z edge of data
 ct_offset[0]-=ct_voxelsize[0]/2.0;
 ct_offset[1]-=ct_voxelsize[1]/2.0;
 ct_offset[2]-=ct_voxelsize[2]/2.0;

 printf("-------------------------------\n");

 printf("\n Finished reading all files from: %s \n",ct_filename);

 printf("Summary of DICOM CT data as read in :\n \n");
 printf("X range : %f - %f cm\n",ct_offset[0],
           ct_offset[0]+ct_arraysize[0]*ct_voxelsize[0]);
 printf("Y range : %f - %f cm\n",ct_offset[1],
            ct_offset[1]+ct_arraysize[1]*ct_voxelsize[1]);
 printf("Z range : %f - %f cm\n\n",ct_offset[2],
            z_final);
 printf("X dimension = %d \n",ct_arraysize[0]);
 printf("Y dimension = %d \n",ct_arraysize[1]);
 printf("Z dimension = %d \n \n",ct_arraysize[2]);
 printf("X voxel size = %f cm\n",ct_voxelsize[0]);
 printf("Y voxel size = %f cm\n",ct_voxelsize[1]);
 printf("Z voxel size = %f cm\n",ct_voxelsize[2]);

 printf("-------------------------------------\n\n");

}
