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
#include "userdefs.h"

CT_ListElement
*CT_Scan(char *t, char *v, int cur_image)
{
   char *sret;
   char *sTokenValuePair(char *t, char *v, char *s);
   CT_ListElement *newCT_el;
   CT_ListElement *CreateCT_Element();
   int is;

      sret = sTokenValuePair(t,v,"IMAGE TYPE");
      if (sret == NULL)
          {
          fprintf (stderr, "%s\n","sTokenValuePair: Not IMAGE TYPE = CT SCAN");
          return(NULL);
          }
      else if ( (is = strcmp(sret,"CT SCAN")) )
          {
          fprintf (stderr,"%5d.  %s\n",cur_image, "Not a CT SCAN image!");
          return(NULL);
          }
      else
          {
          /* You can't assign a value to something you passed in
           * -- like "newCT_el" (which is itself a pointer).
           * You must pass in a pointer to it, and assign it
           * by reference.
           */
          newCT_el = CreateCT_Element();
          newCT_el->image_no = cur_image;
          strcpy (newCT_el->image_type,"CT SCAN");
          return (newCT_el);
          }
}
