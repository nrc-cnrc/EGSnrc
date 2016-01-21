/*
###############################################################################
#
#  EGSnrc egs++ octree geometry
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
#  Author:          Frederic Tessier, 2008
#
#  Contributors:    Iwan Kawrakow
#
###############################################################################
*/


/*
===============================================================================
egs_octree: an octree implementation to the egs++ geometry collection.
===============================================================================

An octree is a way to partition space in a tree-like fashion, and it is a direct
extension of binary trees to 3D. The idea is to start with a cuboid and divide
it in half along each of the three cartesian axes to yield 8 children cuboids
(hence the name "octree"), and so on recursively until some desired limit. One
can restrict this subdivision scheme for any branch of the octree to obtain
leaf cells of varying size: the resolution of the octree can vary spatially.

The main motivation for coding an egs++ octree geometry is to increase the
performance of large voxelized geometries such as a the human phantom. If we
imagine individual voxels in the phantom to be the leaves of the octree, then we
can merge voxels of same medium into larger cells and significantly reduce the
overall number of regions, thus reducing the number of howfar calls.

The geometry can handle multiple bounding boxes in order to model different part
of the geometry at different resolutions (see example below). This can be used
to model micromatrices, for example.

Any egs++ geometry can be "octreefied". One defines a child geometry, a bounding
box and the desired resolution, and the geometry will grow and prune the octree
accordingly; the child geometry is simply used as an oracle that returns the
local medium index for a given point. This approach allows one to use any
existing input file and wrap an octree around it, so in effect in can also serve
as a generic voxelizer (there is an option to disable pruning).

More documentation will follow in the code, but you should be able to try it
right away with your own geometries, following the examples I will commit to
the egs++/geometry/examples. Please take it out for a spin on your own favorite
geometry and let me know if it works well.

Enjoy.


===============================================================================
EXAMPLE:
===============================================================================

Here is a simple example that defines a sphere and then models it with an octree
in which the positive octant features a higher resolution:

:start geometry definition:

    :start geometry:
        library         = egs_spheres
        name            = my_sphere
        :start media input:
            media       = my_medium
        :stop media input:
        midpoint        = 0 0 0
        radii           = 0.9
    :stop geometry:

    :start geometry:
        library         = egs_octree
        name            = my_octree
        :start octree box:
            box min     = -1 -1 -1
            box max     = 1 1 1
            resolution  = 32 32 32
        :stop octree box:
        :start octree box:
            box min     = 0 0 0
            box max     = 1 1 1
            resolution  = 128 128 128
        :stop octree box:
        child geometry  = my_sphere
    :stop geometry:

     simulation geometry = my_octree

:stop geometry definition:


===============================================================================
PERFORMANCE:
===============================================================================

I tested the octree geometry on the fax06 human phantom to check what kind of
gains can be achieved. Using the howfar time test from the geometry tester, I
found the following:

fax06 human phantom:
-------------------------------------------------------------------------------
model        # cells    avg cell size    avg # steps     time / sample (on tux)
-------------------------------------------------------------------------------
voxels     141716520             0.12            284             14 us
octree       9045359             0.30             46             20 us
-------------------------------------------------------------------------------
GAIN            94 %                            84 %             32 %
-------------------------------------------------------------------------------

There is a major gain in the number of cells and the number of howfar calls,
but the performance increase is mitigated by the more intricate logic for
navigating in the octree (finding neighbors etc.). So the performance increase
is only 32%, as opposed to the 84% that could be expected solely from the
decrease of the number of steps. Of course, all these numbers are model
specific, and the gain will be better for geometries in which there are large
volumes of uniform medium.


===============================================================================
MEMORY:
===============================================================================

Each node in the final octree (the number of reported regions) requires 46
bytes of memory (on 64-bit machines), that is, 38 bytes for each instance of
the node EGS_OCtree_node class and 8 bytes for a pointer to the node in the
region list. So for example the fax06 phantom requires about 400 MB of memory
(but it shows up as using about 600 MB, so maybe there is a leak?).


===============================================================================
TODO:
===============================================================================

1. Add an option in the VHP geometry to represent the model with an octree, and
perhaps implement a bottom-up octree growth algorithm if it can be faster than
the current recursive method.

2. Add the option to merge voxels when a certain proportion of the children
nodes have the same medium.

3. Add the option to set the resolution depending on the medium or other
criteria than bounding boxes?

=============================================================================
*/


/*! \file egs_octree.cpp
 *  \brief An octree geometry: implementation
 *  \FT
 */

#include "egs_octree.h"
#include "egs_input.h"

void EGS_Octree::printInfo() const {
    EGS_BaseGeometry::printInfo();
    egsInformation(" bounding box minimum     = %g %g %g\n", bbxmin, bbymin, bbzmin);
    egsInformation(" bounding box maximum     = %g %g %g\n", bbxmax, bbymax, bbzmax);
    egsInformation(" bounding box resolution  = %d %d %d\n", nx, ny, nz);
    egsInformation(" octree leaf size         = %g %g %g\n", dx, dy, dz);
    egsInformation(" octree cells (no medium) = %d\n", nreg-nLeaf);
    egsInformation(" octree cells (medium)    = %d\n", nLeaf);
    egsInformation(" octree average cell size = %.2f\n", nLeafMax/(float)nLeaf);
    char percent = '%';
    egsInformation(" octree cell savings      = %.1f%c\n", 100*(1-(float)nLeaf/nLeafMax),percent);
    egsInformation("=======================================================\n");
}

string EGS_Octree::type("EGS_Octree");

static char EGS_OCTREE_LOCAL eoctree_message1[]  = "createGeometry(octree): %s\n";
static char EGS_OCTREE_LOCAL eoctree_message2[]  = "null input?";
static char EGS_OCTREE_LOCAL eoctree_message3[]  = "wrong/missing 'octree size' input?";
static char EGS_OCTREE_LOCAL eoctree_message4[]  = "expecting 1 or 3 float inputs for 'octree size'";
static char EGS_OCTREE_LOCAL eoctree_message5[]  = "wrong/missing 'min' or input?";
static char EGS_OCTREE_LOCAL eoctree_message6[]  = "expecting 3 float inputs for 'box min' input";
static char EGS_OCTREE_LOCAL eoctree_message7[]  = "wrong/missing 'max' or input?";
static char EGS_OCTREE_LOCAL eoctree_message8[]  = "expecting 3 float inputs for 'max max' input";
static char EGS_OCTREE_LOCAL eoctree_message9[]  = "wrong or missing 'resolution' input?";
static char EGS_OCTREE_LOCAL eoctree_message10[] = "expecting 3 integer inputs for 'resolution' input";
static char EGS_OCTREE_LOCAL eoctree_message11[] = "wrong or missing 'discard child' input?";
static char EGS_OCTREE_LOCAL eoctree_message12[] = "expecting 'yes' or 'no' for 'discard child' input";
static char EGS_OCTREE_LOCAL eoctree_message13[] = "missing or wrong 'child geometry' input";
static char EGS_OCTREE_LOCAL eoctree_message14[] = "undefined child geometry";
static char EGS_OCTREE_LOCAL eoctree_message15[] = "you need to define at least one octree box";
static char EGS_OCTREE_LOCAL eoctree_message16[] = "wrong 'prune tree' input?";
static char EGS_OCTREE_LOCAL eoctree_message17[] = "expecting 'yes' or 'no' for 'prune tree' input?";
static char EGS_OCTREE_LOCAL eoctree_key0[] = "octree box";
static char EGS_OCTREE_LOCAL eoctree_key1[] = "box min";
static char EGS_OCTREE_LOCAL eoctree_key2[] = "box max";
static char EGS_OCTREE_LOCAL eoctree_key3[] = "resolution";
static char EGS_OCTREE_LOCAL eoctree_key4[] = "child geometry";
static char EGS_OCTREE_LOCAL eoctree_key5[] = "discard child";
static char EGS_OCTREE_LOCAL eoctree_key6[] = "prune tree";

extern "C" {

    EGS_OCTREE_EXPORT EGS_BaseGeometry *createGeometry(EGS_Input *input) {

        EGS_Input *i;

        // check that we have an input
        if (!input) {
            egsWarning(eoctree_message1,eoctree_message2);
            return 0;
        }

        // read bounding boxes
        vector<EGS_Octree_bbox> vBox;
        while (i = input->takeInputItem(eoctree_key0)) {

            // read the bounding box minimum
            vector<EGS_Float> v;
            int err = i->getInput(eoctree_key1, v);
            if (err) {
                egsWarning(eoctree_message1, eoctree_message5);
                return 0;
            }
            if (v.size() != 3) {
                egsWarning(eoctree_message1, eoctree_message6);
                return 0;
            }
            EGS_Vector bboxMin(v[0],v[1],v[2]);

            // read the bounding box maximum
            err = i->getInput(eoctree_key2, v);
            if (err) {
                egsWarning(eoctree_message1, eoctree_message7);
                return 0;
            }
            if (v.size() != 3) {
                egsWarning(eoctree_message1, eoctree_message8);
                return 0;
            }
            EGS_Vector bboxMax(v[0],v[1],v[2]);

            // read the bounding box resolution
            vector<int> bboxRes;
            err = i->getInput(eoctree_key3, bboxRes);
            if (err) {
                egsWarning(eoctree_message1, eoctree_message9);
                return 0;
            }
            if (bboxRes.size() != 3) {
                egsWarning(eoctree_message1, eoctree_message10);
                return 0;
            }

            EGS_Octree_bbox box = EGS_Octree_bbox(bboxMin, bboxMax, bboxRes);
            vBox.push_back(box);
        }
        if (vBox.size() < 1) {
            egsWarning(eoctree_message1, eoctree_message15);
            return 0;
        }

        // read discard child option
        bool discardChild = true;
        string discard;
        if (input->getInputItem(eoctree_key5)) {
            int err = input->getInput(eoctree_key5, discard);
            if (err) {
                egsWarning(eoctree_message1, eoctree_message11);
                return 0;
            }
            if (discard.find("yes")==string::npos && discard.find("no")==string::npos) {
                egsWarning(eoctree_message1, eoctree_message12);
                return 0;
            }
            if (discard.find("no")!=string::npos) {
                discardChild = false;
            }
        }

        // read prune tree option
        bool pruneTree = true;
        string prune;
        if (input->getInputItem(eoctree_key6)) {
            int err = input->getInput(eoctree_key6, prune);
            if (err) {
                egsWarning(eoctree_message1, eoctree_message16);
                return 0;
            }
            if (prune.find("yes")==string::npos && prune.find("no")==string::npos) {
                egsWarning(eoctree_message1, eoctree_message17);
                return 0;
            }
            if (prune.find("no")!=string::npos) {
                pruneTree = false;
            }
        }

        // read and load the child geometry
        string gname;
        {
            int err = input->getInput(eoctree_key4, gname);
            if (err) {
                egsWarning(eoctree_message1, eoctree_message13);
                return 0;
            }
        }
        EGS_BaseGeometry *g = EGS_BaseGeometry::getGeometry(gname);
        if (!g) {
            egsWarning(eoctree_message1, eoctree_message14);
            return 0;
        }

        // create the octree geometry
        EGS_Octree *octree = new EGS_Octree(vBox, pruneTree, g);
        octree->setName(input);
        octree->setLabels(input);
        octree->printInfo();

        if (discardChild) {
            delete g;
        }
        return octree;
    }
}
