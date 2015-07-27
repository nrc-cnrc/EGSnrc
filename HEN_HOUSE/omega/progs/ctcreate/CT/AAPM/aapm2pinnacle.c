/*
###############################################################################
#
#  EGSnrc AAPM to Pinnacle format converter source code
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


/* This files reads in each line of an ascii file (no ^M's or NULLS!)
 * and parses the lines using := as the delimiter
 *  BUFSIZE is in one of the includes.
 */

#include <stdio.h>
#include <stdlib.h>
#include "userdefs.h"    /* For CT_ListElement and other globals */
#include "Z_list.h"
#include "aapm2pinnacle.h"

/* Global Definitions */
int first_CT_scan = 0;
int last_CT_scan = 10000;
Z_LIST *Z_List;
AAPM_HEADER AAPM_Header;
char pinbuf[5012];          /* Pinnacle headers are generall ~820 bytes */

main (int argc, char **argv)
{
   FILE  *fp, *removeCtrlM(char *a, char *b), *fp_out;
   int    SortTokenValue (char *t, char *v, CT_ListElement **CT_el_ret);
   void   ParseTokenValuePair(char *line_buf, char *nice_token, char * nice_value);
   char line_buf[BUFSIZ];
   char nice_token[BUFSIZ];
   char nice_value[BUFSIZ];
   char AAPMdir[BUFSIZ];
   char temp[BUFSIZ];
   char Pinndir[BUFSIZ];
   char basename[BUFSIZ];
   char dirfile[BUFSIZ];
   int  cur_image, ret;
   CT_ListElement *CT_el_ret;
   static CT_ListElement *CT_save;
   char pheader[5012];
   void PinnacleHeader(CT_ListElement *CT_save);
   void DirName(char *a,char *d);
   char im_number[5];
   int i,N;
   Z_ListElement *el;
   FILE  *gfopen (char *file_name, char *mode);
   short *CT_raw;
   int x_by_y_size;

   if (argc == 1)
      {
      fprintf (stderr, "\n%s%s%s\n\n%s%s%s\n\n",
          "Usage: ", argv[0], " AAPM-directory-name [Pinnacle-directory-name]",
          "If no Pinnacle directory is given, the *.header and *.img files \n",
          "are written to the current directory with a basename of the\n",
          "AAPM directory name given.");
      exit(1);
      }
   if (argc > 1)
      {
      sscanf(argv[1],"%s",AAPMdir);
      strcpy (temp,AAPMdir);
      DirName(temp,basename);        /* basename is, e.g., "bowers" */
      }
   if (argc > 2)
      {
      sscanf(argv[2],"%s",Pinndir);
      if ( Pinndir[strlen(Pinndir)-1] != '/')
         strcat (Pinndir, "/");
      }
   /* No pinnacle_dir given */
   else
      strcat (Pinndir,"./");

   /* fp will point to a temporary file which is identical to
    * the AAPM header file, but with no nasty ^M's and NULL's in it.
    */
   fp = removeCtrlM(AAPMdir,basename);

   /* Gets one line, up to the newline */
   while ( fgets (line_buf, BUFSIZ, fp ) != NULL)
      {
      /* Read and clean up the records */
       ParseTokenValuePair(line_buf, nice_token, nice_value);
      /* printf ("%s%s%s\n",nice_token,"********",nice_value); */

      /* Load up the CT_ListElement data structure */
      cur_image = SortTokenValue(nice_token,nice_value, &CT_el_ret);
      if (CT_el_ret != NULL)
        CT_save = CT_el_ret;
      /*
      if (cur_image == -1)
         fprintf (stderr, "%s\n","SortTokenValue done.");
      */
      }
   fclose(fp);
   fprintf (stderr, "\n%s%s.\n", "Done with ",argv[1]);

   /* Write out the Pinnacle header. */
   strcpy (temp,Pinndir);
   strcat (temp,basename);
   strcat (temp,".header");
   PinnacleHeader(CT_save);             /* fills the global "pinbuf" */
   fp = gfopen (temp,"w+");
   fprintf (fp,"%s",pinbuf);
   /* printf ("%s",pinbuf); */
   fclose(fp);

   strcat (Pinndir,basename);
   strcat (Pinndir,".img");

   fp_out = gfopen (Pinndir,"w+");

   N = Length (Z_List);
   fprintf (stderr, "N IS %d\n\n",N);
   strcat(AAPMdir,"/");
   x_by_y_size = CT_save->size_of_dimension_1 * CT_save->size_of_dimension_2;

   for (i=1; i<=N; i++)
    {
    /* Initialize each time through loop */
    strcpy (dirfile,AAPMdir);
    strcpy (temp,basename);

    el = Retrieve(Z_List,i);
    sprintf (im_number,"%04d",el->image_no);

    strcat (temp, im_number);
    strcat (dirfile, temp);
    fprintf (stderr,"%s\n",dirfile);    /* e.g. "./bowers/bowers0009" */

    CT_raw = (short *)malloc(sizeof(short)*x_by_y_size);
    fp = gfopen (dirfile, "r+");
    ret = fread (CT_raw, sizeof(short), x_by_y_size, fp);
    if ( ret != x_by_y_size)
        {
        fprintf (stderr,"Error reading %s!\n", dirfile);
        exit(1);
        }
    fwrite(CT_raw, sizeof (short), x_by_y_size, fp_out);
    fclose(fp);
    }
}

/* I have screwed up the globality of CT_el somehow */
/* Define a CT_ListElement, NOT *CT_ListElement */
void
PinnacleHeader(CT_ListElement *CT)
{
    char temp[256];
    int first_scan, last_scan;
    int endian;
    Z_ListElement *first_el, *last_el, *second_el;
    int byteorder();
    float z_pixdim;

    first_el = Retrieve (Z_List, 1);
    last_el  = Retrieve(Z_List,Length(Z_List) );
    second_el = Retrieve(Z_List, 2);

    first_scan = first_el->image_no;
    last_scan = last_el->image_no;
    z_pixdim = second_el->z_value - first_el->z_value;

    /* 0 = little_endian (DEC), 1 = big_endian (sparc) */
    if ( (endian = byteorder()) != -1)
       sprintf (temp,"\t%s = %d;\n", "byte_order",endian);
    else
       exit(1);
    strcpy (pinbuf,temp);

    sprintf (temp,"\t%s\n\t%s\n\t%s\n",
                  "read_conversion = \"\";",
                  "write_conversion = \"\";",
                  "t_dim = 0;");
    strcat (pinbuf,temp);

    sprintf (temp, "\t%s %d;\n\t%s %d;\n\t%s %d;\n",
                   "x_dim = ", CT->size_of_dimension_1,
                   "y_dim = ", CT->size_of_dimension_2,
                   "z_dim = ", last_scan - first_scan + 1);
    strcat (pinbuf,temp);

    sprintf (temp, "\t%s\n\t%s\n\t%s\n\t%s\n\t%s\n\t%s\n",
                   "datatype = 0;",
                   "bitpix = 16;",
                   "bytes_pix = 2;",
                   "vol_max = 4095.000000;",
                   "vol_min = 0.000000;",
                   "t_pixdim = 0.000000;");
    strcat (pinbuf,temp);


    sprintf (temp, "\t%s %7.6f;\n\t%s %7.6f;\n\t%s %7.6f;\n",
                   "x_pixdim = ", CT->grid_1_units,
                   "y_pixdim = ", CT->grid_2_units,
                   "z_pixdim = ", z_pixdim);
    strcat (pinbuf,temp);

    sprintf (temp, "\t%s\n\t%s %7.6f;\n\t%s %7.6f;\n\t%s %7.6f;\n",
                   "t_start = 0.000000;",
                   "x_start = ", CT->x_offset,
                   "y_start = ", CT->y_offset,
                   "z_start = ", first_el->z_value);
    strcat (pinbuf,temp);

    sprintf (temp, "\t%s\n\t%s\n\t%s\n\t%s\n\t%s\n\t%s\n",
                   "z_time = 0.000000;",
                   "dim_units : cm",
                   "voxel_type : ",
                   "id = 0;",
                   "data_type : short",
                   "vol_type : Hounsfield");
    strcat (pinbuf,temp);

    sprintf (temp, "\t%s%s\n\t%s%s\n\t%s%s\n\t%s%s\n",
                   "db_name : ", CT->patient_name,
                   "medical_record : ", AAPM_Header.writer,
                   "originator : ", AAPM_Header.institution,
                   "date : ", AAPM_Header.date_created);
    strcat (pinbuf,temp);


    sprintf (temp, "\t%s\n\t%s\n\t%s\n\t%s\n\t%s\n\t%s\n\t%s\n\t%s\n",
                   "scanner_id :",
                   "patient_position : ",
                   "orientation = 0;",
                   "comment : ",
                   "fname_format : ",
                   "fname_index_start = 0;",
                   "fname_index_delta = 0;",
                   "binary_header_size = 0;");
    strcat (pinbuf,temp);

}

