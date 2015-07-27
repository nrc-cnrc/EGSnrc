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


#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include "bitmask.h"
#include "aapm2pinnacle.h"
#include "userdefs.h"

#define HBITS 3
#define BITS 4

extern AAPM_HEADER AAPM_Header;

void ParseTokenValuePair(char *line_buf, char *nice_token, char *nice_value)
{
   char  token[BUFSIZ];
   char  value[BUFSIZ];
   char *w;

      /* Parse the first ("token") half of the line */
      sprintf (token, "%s",strtok(line_buf,":="));
      /* This is the value half */
      sprintf (value, "%s",strtok(NULL,":="));

      /* token and value are strings padded with spaces and newlines */
      /* TOKEN */
      sprintf (nice_token, "%s",strtok(token," "));
      while ( (w = strtok(NULL," ")) != NULL)
         {
         if (w[0] != ' ')
            strcat (strcat(nice_token," "), w);
         }
      /* VALUE */
      sprintf (nice_value, "%s",strtok(&value[1],"\n"));
}

int HeaderTokenValuePair(char *t, char *v, AAPM_HEADER *A)
{
   static header_flag;
   int i,is;
   static short bheader[HBITS];   /* Can't be a char, must be a short */
   static short standard;      /* Calculate what this should be if all
                         *  headers have been set, and compare
                         *  the current header bitmask to it
                         *  in order to determine if we shoule
                         *  bother with the header
                         */

   /*
    * See bitmask.c for test code.
    */
   if (standard == 0)
       for (i=0; i<5; i++)
        BITSET(&standard, i);

   /* Header has not been filled */
    if (bheader[0] != standard)
      {
      if ( (is = strcmp(t,"TAPE STANDARD #")) == 0)
         {
         BITSET(bheader,0);
          A->tape_standard_no = atof(v);
         }
      else if ( (is = strcmp(t,"INTERCOMPARISON STANDARD #")) == 0)
         {
         BITSET(bheader,1);
         A->intercomparison_standard_no = atof(v);
         }
      else if ( (is = strcmp(t,"INSTITUTION")) == 0)
         {
         BITSET(bheader,2);
         strcpy (A->institution, v);
         }
      else if ( (is = strcmp(t,"DATE CREATED")) == 0)
         {
         BITSET(bheader,3);
         strcpy (A->date_created, v);
         }
      else if ( (is = strcmp(t,"WRITER")) == 0)
         {
         BITSET(bheader,4);
         strcpy (A->writer, v);
         }
      if (bheader[0] == standard)
         {
         for (i = 0; i<HBITS; i++)
             bheader[i] = 0;
         return (1);                        /* Header is full */
         }
      else return (0);                      /* Header is not filled */
      }
   else                                     /* Header is full */
      return (1);
}

int CT_TokenValuePair(char *t, char *v,CT_ListElement *CT_el)
{
    int i,is;
    static unsigned short ct_data[BITS];
    static unsigned short standard[BITS];
    /* Calculate what this should be if all ? */

   /*
    * See bitmask.c for test code.
    */
   if (standard[0] == 0)
       for (i=0; i<16; i++)
        BITSET(standard, i);

    if ( (ct_data[0] != standard[0]) || (ct_data[1] != standard[1]) )
      {
      if ( (is = strcmp(t,"CASE #")) == 0)
         {
         BITSET(ct_data,0);
         CT_el->case_no = atoi(v);
         }
      else if ( (is = strcmp(t,"PATIENT NAME")) == 0)
         {
         BITSET(ct_data,1);
         strcpy (CT_el->patient_name, v);
         }
      else if ( (is = strcmp(t,"SCAN TYPE")) == 0)
         {
         BITSET(ct_data,2);
         strcpy (CT_el->scan_type, v);
         }
      else if ( (is = strcmp(t,"CT OFFSET")) == 0)
         {
         BITSET(ct_data,3);
         CT_el->ct_offset = atol(v);
         }
      else if ( (is = strcmp(t,"GRID 1 UNITS")) == 0)
         {
         BITSET(ct_data,4);
         CT_el->grid_1_units = atof(v);
         }
      else if ( (is = strcmp(t,"GRID 2 UNITS")) == 0)
         {
         BITSET(ct_data,5);
         CT_el->grid_2_units = atof(v);
         }
      else if ( (is = strcmp(t,"NUMBER REPRESENTATION")) == 0)
         {
         BITSET(ct_data,6);
         strcpy (CT_el->number_representation, v);
         }
      else if ( (is = strcmp(t,"BYTES PER PIXEL")) == 0)
         {
         BITSET(ct_data,7);
         CT_el->bytes_per_pixel = (short)atoi(v);
         }
      else if ( (is = strcmp(t,"NUMBER OF DIMENSIONS")) == 0)
         {
         BITSET(ct_data,8);
         CT_el->number_of_dimensions = (short)atoi(v);
         }
      else if ( (is = strcmp(t,"SIZE OF DIMENSION 1")) == 0)
         {
         BITSET(ct_data,9);
         CT_el->size_of_dimension_1 = atoi(v);
         }
      else if ( (is = strcmp(t,"SIZE OF DIMENSION 2")) == 0)
         {
         BITSET(ct_data,10);
         CT_el->size_of_dimension_2 = atoi(v);
         }
      else if ( (is = strcmp(t,"Z VALUE")) == 0)
         {
         BITSET(ct_data,11);
         CT_el->z_value = atof(v);
         }
      else if ( (is = strcmp(t,"X OFFSET")) == 0)
         {
         BITSET(ct_data,12);
         CT_el->x_offset = atof(v);
         }
      else if ( (is = strcmp(t,"Y OFFSET")) == 0)
         {
         BITSET(ct_data,13);
         CT_el->y_offset = atof(v);
         }
      else if ( (is = strcmp(t,"CT-AIR")) == 0)
         {
         BITSET(ct_data,14);
         CT_el->ct_air = atoi(v);
         }
      else if ( (is = strcmp(t,"CT-WATER")) == 0)
         {
         BITSET(ct_data,15);
         CT_el->ct_water = atoi(v);
         }
      if ( (ct_data[0] == standard[0]) && (ct_data[1] == standard[1]) )
         {
         for (i = 0; i<BITS; i++)
             ct_data[i] = 0;
         return (1);                        /* CT Data is full */
         }
      else return (0);                      /* CT Data is not filled */
      }
   else                                     /* CT Data is full */
      return (1);
}

int iTokenValuePair(char *t, char *v, char *s)
{
    int is;

    if ( (is = strcmp(t,s)) == 0)
        return (atoi(v));
    else
        return (-1);
}

char *sTokenValuePair(char *t, char *v, char *s)
{
    int is;

    if ( (is = strcmp(t,s)) == 0)
        return (v);
    else
        return (NULL);
}
