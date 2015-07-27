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


#include "aapm2pinnacle.h"
#include "userdefs.h"
#include "Z_list.h"

extern AAPM_HEADER AAPM_Header;

/* Put those values into a nicey-nice data structure */
int
SortTokenValue(char *t, char *v, CT_ListElement **CT_el_ret)
{
   int HeaderTokenValuePair(char *t, char *v, AAPM_HEADER *a);
   int NewImage(char *t, char *v, Z_LIST *Z_list);
   CT_ListElement *CT_Scan(char *t,char *v,int cur_image);
   int CT_TokenValuePair(char *t, char *v, CT_ListElement *CT_el);
   Z_ListElement *el;

   static int header_done;
   static int CT_done;
   static int check_for_new_image = 1;
   static int is_CT_scan = 0;
   static int cur_image;               /* this is initialized to 0 */
   int iret;
   static CT_ListElement *CT_el;

   /* NOTE:  NO ERROR CHECKING ON HeaderTokenValuePair (Yet) */
   /* HEADER */
   if (header_done != 1)
      {
       header_done =  HeaderTokenValuePair(t,v,&AAPM_Header);

      /* If header_done status changes, don't need to continue */
       cur_image = header_done;
       return(cur_image);
      }
   /* IMAGE #:  NewImage() */
   else if ( check_for_new_image == 1 || cur_image >=  last_CT_scan )
      {
      /* This routine is the one that fills first_CT_scan */
      /* When the first CT scan is detected, the Z_List is created */
      iret = NewImage(t,v,Z_List);

      /* Success! */
      if ( iret != 0 && Z_List != NULL )
         {
         cur_image = iret;
         /* Got it, don't look more for now */
         check_for_new_image = 0;
         /* We don't know yet if it's CT data */
         is_CT_scan = 0;
         }

      /* Done with one CT image stack, looking through rest of file
       * for new images that might be anothe CT image stack
       */
      else if (iret == 0 )
         check_for_new_image = 1;
      return (cur_image);
      }
   /* IMAGE TYPE */
   else if (is_CT_scan == 0 && check_for_new_image == 0)
      {
      /* Returns NULL if IMAGE TYPE is not CT SCAN */
      CT_el = CT_Scan(t,v,cur_image);
	  *CT_el_ret = CT_el;
      if (CT_el == NULL)
         {
         /* See if it's NULL because we're at the end of the CT images */
         if (first_CT_scan != 0 )
            {
            last_CT_scan = cur_image - 1;
            if (last_CT_scan < 10000)
               {
               PrintList(Z_List, "all");
               last_CT_scan = 10000;
               }
            first_CT_scan = 0;
            }
         check_for_new_image = 1;
         return (-1);
         }
      /* Success! */
      else
         {
         is_CT_scan = 1;
         check_for_new_image = 0;
         return (CT_el->image_no);
         }
      }
   /* CT DATA: CT_TokenValuePair */
   else if (is_CT_scan)
      {
      /* Get the rest of the CT header data */
      CT_done = CT_TokenValuePair(t,v,CT_el);

      /* If the token-value pair filled up a CT data array, set flag
       * to get the next image
       */
      if (CT_done == 1)
         {
         check_for_new_image = 1;
         is_CT_scan = 0;
         el = CreateZ_Element();
         el->z_value = CT_el->z_value;
         el->image_no = CT_el->image_no;
         Insert(Z_List, Length(Z_List)+1, el);
         }
      return (cur_image);
      }
}
