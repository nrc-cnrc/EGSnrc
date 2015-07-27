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


/* Called by SortTokenValue */
/* ToDo:  Move image_no_expect here */

#include <stdio.h>
#include "userdefs.h"
#include "Z_list.h"

int
NewImage(char *t, char *v, Z_LIST **Z_list )
{
   int iret;
   int iTokenValuePair(char *t, char *v, char *s);
   static int img_expect;

      /*  Returns atoi(v) if t is IMAGE # */
      iret = iTokenValuePair(t,v,"IMAGE #");
      if (iret == -1)
         {
         /*
         fprintf (stderr, "%s\n",
                          "iTokenValuePair choked on IMAGE #");
         */
         return 0;
         }

      /* img_expect is the expected value.  */
      /* The first time through, image_no is 0.  The first
       * image does not have to be #1
       * Create the z_value linked list here
       */
      else if (iret != img_expect)
         {
         if (img_expect == 0)
             {
              img_expect = iret + 1;
             /* printf ("%s\t%4d\n","img_expect:",img_expect); */
             first_CT_scan = iret;
             Z_List = CreateList();
             return(iret);
             }
         else
           {
           fprintf (stderr,"%s%d%s\n", "IMAGE # is not ",iret,
                                       ", as expected");
           /* Do not return the image number if it's wrong. */
           return 0;
           }
         }

      /* Everything was OK */
      else
         {
          img_expect += 1;
        /*      printf ("%s\t%4d\n","img_expect:",img_expect); */
         return (iret);
         }
}
