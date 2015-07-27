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
#include <stdlib.h>
#include "Z_list.h"

#define DEBUG 0
#define DEBUG_SERIOUS 0


/* RESULTS: creates an empty ordered list L  */

Z_LIST *CreateList()
{
    Z_LIST *temp1,*temp2;

    temp1 = (Z_LIST *)malloc(sizeof(Z_LIST));
    if (temp1 == (Z_LIST *)0)
        {
        printf ("Error malloc'ing list\n");
        exit(1);
        }

    temp2 = (Z_LIST *)malloc(sizeof(Z_LIST));
    if (temp2 == (Z_LIST *)0)
        {
        printf ("Error malloc'ing list\n");
        exit(1);
        }

    temp1->next=temp2;
    temp2->next=(Z_LIST *)0;

    return (temp1);
}


/* REQUIRES:  L must already be created
   RESULTS:   returns the number of items in the ordered list L
*/
int Length(Z_LIST *L)
{
    int i=-1;

    while (L->next != (Z_LIST *)0)
        {
        L = L->next;
        i++;
        }

    return (i);
}


/* REQUIRES: L must already be created.  The value of i must be
    within the range [ 1 .. Length(L)+1 ].
  RESULTS: inserts newitem at position i of ordered list L.
    If i<= Length(L) items are shifted so that the item at
    position i becomes the item at position i+1, and so on.
*/
void Insert( Z_LIST *L, int i, Z_ListElement *newelement )
{
    Z_LIST *node, *newnode;
    Z_LIST *MakeNode(), *GetNode(Z_LIST *L, int i);

    node          = GetNode (L,i-1);

    newnode       = MakeNode();
    newnode->e       = newelement;
    newnode->next = node->next;

    node->next       = newnode;
}



/* REQUIRES: L must already be created.  The value of i must be
    in the range [ 1 .. Length(L) ].
  RESULTS:  deletes item at position i of ordered list L.  Items
    are shifted so that the element at position i+1 becomes
    the item at position i, and so on.
*/
void Delete( Z_LIST *L, int i )
{
    Z_LIST *node, *prevnode;
    Z_LIST *GetNode (Z_LIST *L, int i);

    prevnode = GetNode(L,i-1);
    node = prevnode->next;
    prevnode->next = node->next;

    free(node->e);
    free(node);
}

/* REQUIRES: L must already be created.  The value of i must be in the
    range [ 1 .. Length(L) ].
  RESULTS: The value of the element at position i of the ordered list L is
    returned.  The list is left unchanged.
*/
Z_ListElement *Retrieve (Z_LIST *L, int i)
{
    Z_LIST *Retrieved;
    Z_LIST *GetNode (Z_LIST *L, int i);

    Retrieved = GetNode(L,i);

    return (Retrieved->e);
}

/* RESULTS: Creates Z_LIST node and returns a pointer to it */
Z_LIST *MakeNode()
{
    Z_LIST *node;

    node = (Z_LIST *)malloc(sizeof(Z_LIST));

    if (node == (Z_LIST *)0)
        {
        printf ("Error malloc'ing Z_LIST node in MakeNode\n");
        exit(1);
        }

    return (node);
}


/* REQUIRES: L must already be created.  The value of i must be in the
    range [ 1 .. Length(L) ].
  RESULTS: A pointer to the ith node of the ordered list L is returned.
  The list is left unchanged.
*/
Z_LIST *GetNode( Z_LIST *L, int i)
{
    int j;

    for (j=0; j<i; j++)
        L = L->next;

    return (L);
}
