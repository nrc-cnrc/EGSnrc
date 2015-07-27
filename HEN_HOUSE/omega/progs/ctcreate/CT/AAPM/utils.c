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


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Z_list.h"
#include "byteorder.h"

/* Graceful fopen */
FILE *gfopen (char *file_name, char *mode)
{
   FILE *fp;

   if ((fp = fopen (file_name, mode)) == NULL)
      {
      fprintf (stderr, "Cannot open %s - bye!\n", file_name);
      exit(1);
      }
   return fp;
}


/*
 * Input: *d
 * Output: *n
 *
 * Input:  "./bowers"
 * Output: "bowers"
 *
 * Input:  "/home/local/data/zachman.x/omega/Workshop96/bowers"
 * Output: "bowers"
 *
 * Input: "bowers"
 * Output: "bowers"
 */
void
DirName(char *d,char *n)
{

   char dirname[BUFSIZ];
   char *h, *junk;

      /* Parse the full directory name */
      junk = strtok(d,"/");
      if( junk ) {
          fprintf(stderr,"first token: %s\n",junk);
          sprintf (n, "%s",junk);
      }
      else fprintf(stderr,"first token is null\n");
      while ( (h = strtok(NULL,"/")) != NULL)
         {
         strcpy(n,h);
         fprintf (stderr, " \"%s\"",h);
         }

      /* while ( (h = strtok(NULL,"/")) != NULL) */
/*
      while (h != "")
          sprintf (h, "%s",strtok(NULL,"/"));
      while ( (h = strtok(NULL,"/")) != NULL)
         {
         strcpy(n,h);
         fprintf (stderr, " \"%s\"",h);
         }
*/
}


/* Allocate space for the structure that is an element */
/* typedef float Z_ListElement; */
/* This is way over-general, but I am following the
 * CS 367 (Data Structures) model
 */
Z_ListElement *CreateZ_Element()
{
    Z_ListElement *CreatedElement;

    CreatedElement = (Z_ListElement *)malloc(sizeof(Z_ListElement));
    if (CreatedElement == (Z_ListElement *)0)
        {
        fprintf (stderr, "Error malloc'ing Z_ListElement\n");
        exit(1);
        }

    return (CreatedElement);
}

/* Allocate space for the structure that is an element */
CT_ListElement *CreateCT_Element()
{
    CT_ListElement *CreatedElement;
    char  *Image_type;
    char  *Patient_name;
    char  *Scan_type;
    char  *Number_representation;

    CreatedElement = (CT_ListElement *)malloc(sizeof(CT_ListElement));
    if (CreatedElement == (CT_ListElement *)0)
        {
        fprintf (stderr, "Error malloc'ing CT_ListElement\n");
        exit(1);
        }

    Image_type = (char *)malloc(sizeof(char)*MAX_CHAR_BUF);
    if (Image_type == (char *)0)
        {
        fprintf (stderr,"Error malloc'ing Image_type\n");
        exit(1);
        }

    Patient_name = (char *)malloc(sizeof(char)*MAX_CHAR_BUF);
    if (Patient_name == (char *)0)
        {
        fprintf (stderr, "Error malloc'ing Patient_name\n");
        exit(1);
        }

    Scan_type = (char *)malloc(sizeof(char)*MAX_CHAR_BUF);
    if (Scan_type == (char *)0)
        {
        fprintf (stderr, "Error malloc'ing Scan_type\n");
        exit(1);
        }

    Number_representation = (char *)malloc(sizeof(char)*MAX_CHAR_BUF);
    if (Number_representation == (char *)0)
        {
        fprintf (stderr, "Error malloc'ing Number_representation\n");
        exit(1);
        }

    CreatedElement->image_type   = Image_type;
    CreatedElement->patient_name = Patient_name;
    CreatedElement->scan_type    = Scan_type;
    CreatedElement->number_representation = Number_representation;

    return (CreatedElement);
}

static char smallstarbuf[] = {"**************************************"};

/* Print the items in a list by name, item or cost */
void PrintList (L,s)
Z_LIST *L;
char *s;
{
    int N,i;
    Z_ListElement *el;

    N = Length (L);

    fprintf (stderr, "%s\n",smallstarbuf);
    if (strcmp (s,"all") == 0)
        for (i=1; i<=N; i++)
            {
            el = Retrieve (L,i);
            fprintf (stderr, "%5d\t",  el->image_no);
            fprintf (stderr, "%8.2f\n", el->z_value);
            }
    fprintf (stderr, "%s\n\n",smallstarbuf);
}

int byteorder()
{
    u.Long = 1;
    /* LittleEndian */
    if (u.Char[0] == 1)
        {
        /* printf ("Addressing is right-to-left\n"); */
        return 0;
        }
    /* BigEndian */
    else if (u.Char[sizeof(long)-1] == 1)
        {
         /* printf ("Addressing is left-to_right\n"); */
        return 1;
        }
    else fprintf (stderr, "Addressing is strange\n");
    return -1;
}
