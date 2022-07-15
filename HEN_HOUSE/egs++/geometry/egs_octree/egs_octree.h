/*
###############################################################################
#
#  EGSnrc egs++ octree geometry headers
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
#                   Hubert Ho
#                   Reid Townson
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


/*! \file egs_octree.h
 *  \brief An octree geometry: header
 *  \FT
 */

#ifndef EGS_OCTREE_
#define EGS_OCTREE_

#include "egs_functions.h"
#include "egs_base_geometry.h"
#include <vector>
#include <cstring>
#include <cmath>


#ifdef WIN32

    #ifdef BUILD_OCTREE_DLL
        #define EGS_OCTREE_EXPORT __declspec(dllexport)
    #else
        #define EGS_OCTREE_EXPORT __declspec(dllimport)
    #endif
    #define EGS_OCTREE_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_OCTREE_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_OCTREE_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_OCTREE_EXPORT
        #define EGS_OCTREE_LOCAL
    #endif

#endif


class EGS_Octree_bbox {
public:
    EGS_Vector  vmin, vmax;
    int         level, maxlevel;
    int         ixmin, iymin, izmin;
    int         ixmax, iymax, izmax;
    int         nx, ny, nz;

    // constructor
    EGS_Octree_bbox(const EGS_Vector &boxMin, const EGS_Vector &boxMax, vector<int> &bboxRes) :
        vmin(boxMin), vmax(boxMax), nx(bboxRes[0]), ny(bboxRes[1]), nz(bboxRes[2]) {
        maxlevel = 0;
        ixmin = iymin = izmin = 0;
        ixmax = iymax = izmax = 0;
    }

    // addition
    EGS_Octree_bbox operator+ (const EGS_Octree_bbox &b2) const {

        EGS_Octree_bbox b1(*this);

        // set the b1 box limits to enclose the two boxes
        if (b2.vmin.x < b1.vmin.x) {
            b1.vmin.x = b2.vmin.x;
        }
        if (b2.vmin.y < b1.vmin.y) {
            b1.vmin.y = b2.vmin.y;
        }
        if (b2.vmin.z < b1.vmin.z) {
            b1.vmin.z = b2.vmin.z;
        }
        if (b2.vmax.x > b1.vmax.x) {
            b1.vmax.x = b2.vmax.x;
        }
        if (b2.vmax.y > b1.vmax.y) {
            b1.vmax.y = b2.vmax.y;
        }
        if (b2.vmax.z > b1.vmax.z) {
            b1.vmax.z = b2.vmax.z;
        }

        // calculate voxel sizes for the two boxes
        EGS_Float dx1 = (b1.vmax.x-b1.vmin.x)/b1.nx;
        EGS_Float dy1 = (b1.vmax.y-b1.vmin.y)/b1.ny;
        EGS_Float dz1 = (b1.vmax.z-b1.vmin.z)/b1.nz;
        EGS_Float dx2 = (b2.vmax.x-b2.vmin.x)/b2.nx;
        EGS_Float dy2 = (b2.vmax.y-b2.vmin.y)/b2.ny;
        EGS_Float dz2 = (b2.vmax.z-b2.vmin.z)/b2.nz;

        // calculate the number of voxels in the overall box b1 (using the highest resolution of b1 and b2)
        if (dx2 < dx1) {
            b1.nx = (int)((b1.vmax.x - b1.vmin.x + 0.5*dx2) / dx2);
        }
        else {
            b1.nx = (int)((b1.vmax.x - b1.vmin.x +0.5*dx1) / dx1);
        }
        if (dy2 < dy1) {
            b1.ny = (int)((b1.vmax.y - b1.vmin.y + 0.5*dy2) / dy2);
        }
        else {
            b1.ny = (int)((b1.vmax.y - b1.vmin.y +0.5*dy1) / dy1);
        }
        if (dz2 < dz1) {
            b1.nz = (int)((b1.vmax.z - b1.vmin.z + 0.5*dz2) / dz2);
        }
        else {
            b1.nz = (int)((b1.vmax.z - b1.vmin.z +0.5*dz1) / dz1);
        }

        // return the overall box b1
        return b1;
    }

    EGS_Octree_bbox &operator+= (const EGS_Octree_bbox &b2) {

        // set the b1 box limits to enclose the two boxes
        if (b2.vmin.x < vmin.x) {
            vmin.x = b2.vmin.x;
        }
        if (b2.vmin.y < vmin.y) {
            vmin.y = b2.vmin.y;
        }
        if (b2.vmin.z < vmin.z) {
            vmin.z = b2.vmin.z;
        }
        if (b2.vmax.x > vmax.x) {
            vmax.x = b2.vmax.x;
        }
        if (b2.vmax.y > vmax.y) {
            vmax.y = b2.vmax.y;
        }
        if (b2.vmax.z > vmax.z) {
            vmax.z = b2.vmax.z;
        }

        // calculate voxel sizes for the two boxes
        EGS_Float dx1 = (vmax.x-vmin.x)/nx;
        EGS_Float dy1 = (vmax.y-vmin.y)/ny;
        EGS_Float dz1 = (vmax.z-vmin.z)/nz;
        EGS_Float dx2 = (b2.vmax.x-b2.vmin.x)/b2.nx;
        EGS_Float dy2 = (b2.vmax.y-b2.vmin.y)/b2.ny;
        EGS_Float dz2 = (b2.vmax.z-b2.vmin.z)/b2.nz;

        // calculate the number of voxels in the overall box b1 (using the highest resolution of b1 and b2)
        if (dx2 < dx1) {
            nx = (int)((vmax.x - vmin.x + 0.5*dx2) / dx2);
        }
        else {
            nx = (int)((vmax.x - vmin.x +0.5*dx1) / dx1);
        }
        if (dy2 < dy1) {
            ny = (int)((vmax.y - vmin.y + 0.5*dy2) / dy2);
        }
        else {
            ny = (int)((vmax.y - vmin.y +0.5*dy1) / dy1);
        }
        if (dz2 < dz1) {
            nz = (int)((vmax.z - vmin.z + 0.5*dz2) / dz2);
        }
        else {
            nz = (int)((vmax.z - vmin.z +0.5*dz1) / dz1);
        }

        // return the overall box
        return *this;
    }
};


class EGS_Octree_node {
public:
    int             ix, iy, iz;                                 ///< octree indices, representing the binary location code of the node in x, y, z;
    int             region;                                     ///< region number of the node
    short           medium;                                     ///< medium index for the node
    unsigned short  level;                                      ///< depth of the node (root node is level 0)
    EGS_Octree_node *child;                                     ///< pointer to children nodes, NULL is there are no children
    EGS_Octree_node *parent;                                    ///< pointer to the parent node (only root node can have parent set to NULL)

    // constructor
    EGS_Octree_node() {
        medium = -1;                                            // set the medium to -1 by default
        region = -1;                                            // set region to -1 by default
        level  = 0;                                             // set level to 0 (root) by default
        ix = iy = iz = 0;                                       // set octree indices to 0 by default
        child = NULL;                                           // the node has no children initially
        parent = NULL;                                          // the node has no parent initially
    }

    // create children nodes
    void createChildren() {                                     // create children to the this node
        if (!child) {                                           // ensure there are no children already
            child = new EGS_Octree_node [8];                    // allocate memory for 8 new children nodes
            if (!child) {
                egsFatal("EGS_Octree_node::createChildren(): Memory allocation error");
            }
            for (int i=0; i<8; i++) {                           // loop over all 8 newly created children nodes
                child[i].level = level+1;                       // increase level by 1 compared to current level
                child[i].ix = (ix << 1) | (i>>0 & 0x1);         // shift up ix by one, and set new bit to that of the child index's bit 0 (x position)
                child[i].iy = (iy << 1) | (i>>1 & 0x1);         // shift up iy by one, and set new bit to that of the child index's bit 1 (y position)
                child[i].iz = (iz << 1) | (i>>2 & 0x1);         // shift up iz by one, and set new bit to that of the child index's bit 2 (z position)
                child[i].parent = this;                         // set the parent pointer of every child to the current node
            }
        }
    }

    // delete children (recursively)
    void deleteChildren() {                                     // delete children of this node
        if (child) {                                            // if this node has children, delete them
            for (int i=0; i<8; i++) {
                child[i].deleteChildren();    // recursive calls to delete all children branches
            }
            delete [] child;                                    // free the memory allocated for the 8 children
            child = NULL;                                       // set children pointer to NULL to indicate that there are no children now
        }
    }

    // collapse node
    int collapseChildren() {                                    // collapse children in this node if they are all the same medium
        if (child) {                                            // check that we indeed have children
            for (int i=0; i<8; i++) {                           // loop over each of the 8 children of this node
                if (child[i].child) {
                    return 0;    // bail out if there are nodes below the children (this could be made recursive)
                }
                if (child[i].medium!=child[0].medium) {
                    return 0;    // bail out as soon as one children has a different medium
                }
            }
            medium = child[0].medium;                           // set the medium of this node to that of the children
            deleteChildren();                                   // delete the children of this node
            return 1;                                           // return 1 to indicate that the children were collapsed
        }
        return 0;
    }

    // insideBBox
    bool insideBBox(EGS_Octree_bbox &bbox) {
        int shift = bbox.maxlevel - level;
        int ii;
        ii = (ix<<shift);
        if (ii < bbox.ixmin) {
            return false;
        }
        ii = ~(~ix<<shift);
        if (ii > bbox.ixmax) {
            return false;
        }
        ii = (iy<<shift);
        if (ii < bbox.iymin) {
            return false;
        }
        ii = ~(~iy<<shift);
        if (ii > bbox.iymax) {
            return false;
        }
        ii = iz<<shift;
        if (ii < bbox.izmin) {
            return false;
        }
        ii = ~(~iz<<shift);
        if (ii > bbox.izmax) {
            return false;
        }
        return true;
    }

    // instersectBBox
    bool intersectBBox(EGS_Octree_bbox &bbox) {

        int shift = bbox.maxlevel - level;
        int iimin, iimax;

        iimin = (ix<<shift);
        if (bbox.ixmin > iimin) {
            iimin = bbox.ixmin;
        }
        iimax = ~(~ix<<shift);
        if (bbox.ixmax < iimax) {
            iimax = bbox.ixmax;
        }
        if (iimax < iimin) {
            return false;
        }

        iimin = (iy<<shift);
        if (bbox.iymin > iimin) {
            iimin = bbox.iymin;
        }
        iimax = ~(~iy<<shift);
        if (bbox.iymax < iimax) {
            iimax = bbox.iymax;
        }
        if (iimax < iimin) {
            return false;
        }

        iimin = (iz<<shift);
        if (bbox.izmin > iimin) {
            iimin = bbox.izmin;
        }
        iimax = ~(~iz<<shift);
        if (bbox.izmax < iimax) {
            iimax = bbox.izmax;
        }
        if (iimax < iimin) {
            return false;
        }

        return true;
    }
};


/*! \brief An octree geometry

\ingroup Geometry

The EGS_Octree class implements an octree geometry. An octree is a partitioning
scheme for a 3D volume where cells are subdivided into 8 children cells (corresponding
to the 8 octants making up the parent cell). Octrees are useful to partition space
in multi-resolution fashion: portion of space requiring more details are further
subdivided as needed.

This class partitions an existing geometry into an octree such that leaf nodes
are maximally collapsed according to the medium index, that is, adjacent children nodes
with the same medium index are collapsed into their parent node (which then becomes a leaf),
as much as possible. This means that portions of the child geometry with a uniform medium are
represented as large cells rather than many smaller voxels.

\note The actual octree will usually spill out of the bounding box, because internally the number of cells
must be the same along each axis and must be a power of 2. But the dimensions of the octree are adjusted such
that the specified resolution will fit in the specified bounding box. Points outside the bounding box are considered
to be outside the geometry. This means that some region numbers will be attributed to octree cells
lying outside the bounding box, although in practice these region numbers will never be returned by the
geometry.

An octree is defined as follows
\verbatim

library = egs_octree
:start octree box:
    box min = Px Py Pz
    box max = Px Py Pz
    resolution = Nx Ny Nz
:stop octree box:
child geometry = g_name
discard child = yes or no
prune tree = yes or no

\endverbatim

\todo Instead of taking the medium index at the midpoint position of the cells when building the
octree, take the mode of the medium indices at a number of locations (possibly random) inside the cell.

*/

class EGS_OCTREE_EXPORT EGS_Octree : public EGS_BaseGeometry {

    EGS_Octree_node         *root;                              ///< pointer to the octree's root node
    EGS_Octree_node         **nodeReg;                          ///< holding pointers to all leaf nodes, indexed by region number
    EGS_BaseGeometry        *geom;                              ///< pointer to child geometry
    EGS_Float               bbxmin, bbymin, bbzmin;             ///< min of the bounding box
    EGS_Float               bbxmax, bbymax, bbzmax;             ///< max of the bounding box
    EGS_Float               xmin, ymin, zmin;                   ///< min of the octree
    EGS_Float               xmax, ymax, zmax;                   ///< max of the octree
    EGS_Float               dx, dy, dz, dxi, dyi, dzi;          ///< size of the cells (at maximum depth) and inverses
    int                     ixmin, iymin, izmin;                ///< minimum value of the cell index
    int                     ixmax, iymax, izmax;                ///< maximum value of the cell index
    int                     maxlevel, n;                        ///< depth of the octree, and number of cells along one axis
    int                     nx, ny, nz;                         ///< maximum number of leaves along each axis
    static string           type;                               ///< geometry type string
    long int                nLeaf, nLeafMax;                    ///< statistics on leaf nodes
    vector<EGS_Octree_node *> tmp;                              ///< tmp vector to build list of node pointers

public:

    EGS_Octree(vector<EGS_Octree_bbox> &vBox, bool pruneTree, EGS_BaseGeometry *g) : EGS_BaseGeometry(""), geom(g) {

        // combine bounding boxes to get the overall bounding box
        EGS_Octree_bbox bbox(vBox[0]);
        for (int i=1; i<vBox.size(); i++) {
            bbox += vBox[i];
        }
        vBox.push_back(bbox);

        // save overall resolution
        nx = bbox.nx;
        ny = bbox.ny;
        nz = bbox.nz;

        // set size of cells at maxlevel
        dx = (bbox.vmax.x - bbox.vmin.x) / bbox.nx;
        dy = (bbox.vmax.y - bbox.vmin.y) / bbox.ny;
        dz = (bbox.vmax.z - bbox.vmin.z) / bbox.nz;

        // pre-compute cell size inverses to save time
        dxi = 1.0/dx;
        dyi = 1.0/dy;
        dzi = 1.0/dz;

        // set octree depth (maxlevel)
        int res = nx;
        if (ny > res) {
            res = ny;
        }
        if (nz > res) {
            res = nz;
        }
        maxlevel = (int) ceil(log((EGS_Float)res)/0.6931471805599452862);

        // set octree cell count at maxlevel;
        n = 1<<maxlevel;

        // set octree bounds in space
        xmin = bbox.vmin.x;
        ymin = bbox.vmin.y;
        zmin = bbox.vmin.z;
        xmax = bbox.vmin.x + dx*n;
        ymax = bbox.vmin.y + dy*n;
        zmax = bbox.vmin.z + dz*n;

        // set cell indices range for each box;
        {
            for (int i=0; i<vBox.size(); i++) {
                EGS_Octree_bbox *box = &vBox[i];
                box->ixmin = (int)((box->vmin.x - xmin + 0.5*dx) * dxi);
                box->iymin = (int)((box->vmin.y - ymin + 0.5*dy) * dyi);
                box->izmin = (int)((box->vmin.z - zmin + 0.5*dz) * dzi);
                box->nx    = (int)((box->vmax.x - box->vmin.x + 0.5*dx) * dxi);
                box->ny    = (int)((box->vmax.y - box->vmin.y + 0.5*dy) * dyi);
                box->nz    = (int)((box->vmax.z - box->vmin.z + 0.5*dz) * dzi);
                box->ixmax = box->ixmin + box->nx - 1;
                box->iymax = box->iymin + box->ny - 1;
                box->izmax = box->izmin + box->nz - 1;
                if (box->ixmin<0) {
                    box->ixmin = 0;
                }
                if (box->ixmax>=n) {
                    box->ixmax = n-1;
                }
                if (box->iymin<0) {
                    box->iymin = 0;
                }
                if (box->iymax>=n) {
                    box->iymax = n-1;
                }
                if (box->izmin<0) {
                    box->izmin = 0;
                }
                if (box->izmax>=n) {
                    box->izmax = n-1;
                }
                box->nx = box->ixmax - box->ixmin + 1;
                box->ny = box->iymax - box->iymin + 1;
                box->nz = box->izmax - box->izmin + 1;
            }
        }


        // determine maxlevel in each defined bounding box
//         {for (int i=0; i<vBox.size(); i++) {
//             EGS_Octree_bbox *box = &vBox[i];
//             EGS_Float scale = box->nxtree/(EGS_Float)box->nx;
//             if (box->nytree/box->ny > scale) scale = box->nytree/(EGS_Float)box->ny;
//             if (box->nztree/box->nz > scale) scale = box->nztree/(EGS_Float)box->nz;
//             int difflevel = (int) ceil (log(scale)/0.6931471805599452862);
//             box->maxlevel = maxlevel;
//             box->level    = maxlevel-difflevel;
//         }}

        // avoid indirections for overall bounding box parameters
        EGS_Octree_bbox *box = &(vBox.back());
        bbxmin = box->vmin.x;
        bbxmax = box->vmax.x;
        bbymin = box->vmin.y;
        bbymax = box->vmax.y;
        bbzmin = box->vmin.z;
        bbzmax = box->vmax.z;
        ixmin  = box->ixmin;
        ixmax  = box->ixmax;
        iymin  = box->iymin;
        iymax  = box->iymax;
        izmin  = box->izmin;
        izmax  = box->izmax;

        // build octree
        root = new EGS_Octree_node();
        growOctree(root, vBox, pruneTree);

        // allocate memory to hold the region-indexed node pointers in a simple array, and free up the tmp vector
        nreg = tmp.size();
        nodeReg = new EGS_Octree_node* [nreg];
        {
            for (int i=0; i<nreg; i++) {
                nodeReg[i] = tmp[i];
            }
        }
        tmp.erase(tmp.begin(),tmp.end());

        // calculate leaf node statistics
        nLeaf = 0;
        nLeafMax = 0;
        statOctree(root, vBox);
    }


    // destructor
    ~EGS_Octree() {
        if (root) {
            root->deleteChildren();
            delete root;
            delete [] nodeReg;
        }
    }


    // statOctree
    void statOctree(EGS_Octree_node *node, vector<EGS_Octree_bbox> &vBox) {
        if (node->child) {
            for (int i=0; i<8; i++) {
                statOctree(node->child+i, vBox);
            }
        }
        else if (node->insideBBox(vBox[vBox.size()-1]) && node->medium>=0) {
            int shift = maxlevel - node->level;
            nLeaf++;
            nLeafMax += 1<<(3*shift);
        }
    }


    // growOctree (recursive)
    void growOctree(EGS_Octree_node *node, vector<EGS_Octree_bbox> &vBox, bool prune) {

        // assume node needs no refinement
        bool refineNode = false;

        // check if we need to refine this node
        if (node->level < maxlevel) {

            // check all bounding boxes to determine local refinement level (priority to boxes defined later)
            int level = 0;
            {
                for (int i=0; i<vBox.size()-1; i++) {
                    int boxlevel = vBox[i].level;
                    if (node->insideBBox(vBox[i])) {
                        level = boxlevel;
                    }
                }
            }

            // node may still need refinement if it intersects a higher resolution bounding box
            {
                for (int i=0; i<vBox.size()-1; i++) {
                    int boxlevel = vBox[i].level;
                    if (!node->insideBBox(vBox[i]) && node->intersectBBox(vBox[i])) {
                        if (boxlevel > level) {
                            level = boxlevel;
                        }
                    }
                }
            }

            // if the level of this node is not sufficient, set it for refinement
            if (node->level < level) {
                refineNode = true;
            }
        }

        // if this node needs refinement, grow children branches (recursively), and then try to collapse children
        if (refineNode) {
            node->createChildren();
            for (int i=0; i<8; i++) {
                growOctree(node->child+i, vBox, prune);
            }
            if (prune) {
                if (node->collapseChildren()) {
                    tmp.resize(tmp.size()-8);
                    node->region = tmp.size();
                    tmp.push_back(node);
                }
            }
        }
        // otherwise this node is a leaf: set the node medium and add it to the region list
        else {
            bool insideSomeBox = false;
            for (int i=0; i<vBox.size()-1; i++) {
                if (node->insideBBox(vBox[i])) {
                    insideSomeBox = true;
                    break;
                }
            }
            node->medium = -1;
            if (insideSomeBox) {
                int shift = maxlevel - node->level;
                int ix = node->ix << shift;
                int iy = node->iy << shift;
                int iz = node->iz << shift;
                int ireg = geom->isWhere(EGS_Vector(xmin+(ix+0.5)*dx, ymin+(iy+0.5)*dy, zmin+(iz+0.5)*dz));
                if (ireg>=0) {
                    node->medium = geom->medium(ireg);
                }
            }
            node->region = tmp.size();
            tmp.push_back(node);
        }
    }


    // getNeighborNodeX
    EGS_Octree_node *getNeighborNodeX(EGS_Octree_node *node, int ixn, int iyn, int izn) {

        // check if neighbor index is in range
        if (ixn < ixmin || ixn > ixmax) {
            return NULL;
        }

        // constrain neighbor indices in y and z
        int shift = maxlevel - node->level;
        if (iyn < node->iy<<shift) {
            iyn = node->iy<<shift;
        }
        else if (iyn > ~(~node->iy<<shift)) {
            iyn = ~(~node->iy<<shift);
        }
        if (izn < node->iz<<shift) {
            izn = node->iz<<shift;
        }
        else if (izn > ~(~node->iz<<shift)) {
            izn = ~(~node->iz<<shift);
        }

        // walk up and down the octree to new cell
        int diff  = node->ix ^ (ixn>>shift);
        shift = 0;
        while (diff & (1<<shift)) {
            node = node->parent;
            shift++;
        }
        shift = maxlevel - node->level;
        while (node->child) {
            shift--;
            int childIndex;
            childIndex  = (ixn>>shift & 0x1);
            childIndex |= (iyn>>shift & 0x1) << 1;
            childIndex |= (izn>>shift & 0x1) << 2;
            node = node->child + childIndex;
        }
        shift = maxlevel - node->level;

        return node;
    }


    // getNeighborNodeY
    EGS_Octree_node *getNeighborNodeY(EGS_Octree_node *node, int ixn, int iyn, int izn) {

        // check if neighbor index is in range
        if (iyn < iymin || iyn > iymax) {
            return NULL;
        }

        // constrain neighbor indices in x and z
        int shift = maxlevel - node->level;
        if (ixn < node->ix<<shift) {
            ixn = node->ix<<shift;
        }
        else if (ixn > ~(~node->ix<<shift)) {
            ixn = ~(~node->ix<<shift);
        }
        if (izn < node->iz<<shift) {
            izn = node->iz<<shift;
        }
        else if (izn > ~(~node->iz<<shift)) {
            izn = ~(~node->iz<<shift);
        }

        // walk up and down the octree to new cell
        int diff  = node->iy ^ (iyn>>shift);
        shift = 0;
        while (diff & (1<<shift)) {
            node = node->parent;
            shift++;
        }
        shift = maxlevel - node->level;
        while (node->child) {
            shift--;
            int childIndex;
            childIndex  = (ixn>>shift & 0x1);
            childIndex |= (iyn>>shift & 0x1) << 1;
            childIndex |= (izn>>shift & 0x1) << 2;
            node = node->child + childIndex;
        }
        return node;
    }


    // getNeighborNodeZ
    EGS_Octree_node *getNeighborNodeZ(EGS_Octree_node *node, int ixn, int iyn, int izn) {

        // check if neighbor index is in range
        if (izn < izmin || izn > izmax) {
            return NULL;
        }

        // constrain neighbor indices in x and y
        int shift = maxlevel - node->level;
        if (ixn < node->ix<<shift) {
            ixn = node->ix<<shift;
        }
        else if (ixn > ~(~node->ix<<shift)) {
            ixn = ~(~node->ix<<shift);
        }
        if (iyn < node->iy<<shift) {
            iyn = node->iy<<shift;
        }
        else if (iyn > ~(~node->iy<<shift)) {
            iyn = ~(~node->iy<<shift);
        }

        // walk up and down the octree to new cell
        int diff  = node->iz ^ (izn>>shift);
        shift = 0;
        while (diff & (1<<shift)) {
            node = node->parent;
            shift++;
        }
        shift = maxlevel - node->level;
        while (node->child) {
            shift--;
            int childIndex;
            childIndex  = (ixn>>shift & 0x1);
            childIndex |= (iyn>>shift & 0x1) << 1;
            childIndex |= (izn>>shift & 0x1) << 2;
            node = node->child + childIndex;
        }
        return node;
    }


    // getNode
    EGS_Octree_node *getNode(int ix, int iy, int iz) {
        if ((ix<ixmin) || (ix>ixmax) || (iy<iymin) || (iy>iymax) || (iz<izmin) || (iz>izmax)) {
            return NULL;
        }
        EGS_Octree_node *node = root;
        int shift = maxlevel;
        while (node->child) {
            shift--;
            int childIndex;
            childIndex  = (ix>>shift & 0x1);
            childIndex |= (iy>>shift & 0x1) << 1;
            childIndex |= (iz>>shift & 0x1) << 2;
            node = node->child + childIndex;
        }
        return node;
    }


    // setIndices
    void setIndices(const EGS_Vector &r, int &ix, int &iy, int &iz) {
        ix = (int)((r.x-xmin)*dxi);
        iy = (int)((r.y-ymin)*dyi);
        iz = (int)((r.z-zmin)*dzi);
        if (ix<ixmin) {
            ix=ixmin;
        }
        if (ix>ixmax) {
            ix=ixmax;
        }
        if (iy<iymin) {
            iy=iymin;
        }
        if (iy>iymax) {
            iy=iymax;
        }
        if (iz<izmin) {
            iz=izmin;
        }
        if (iz>izmax) {
            iz=izmax;
        }
    }


    // isInside
    bool isInside(const EGS_Vector &r) {
        if (r.x >= bbxmin && r.x <= bbxmax &&
                r.y >= bbymin && r.y <= bbymax &&
                r.z >= bbzmin && r.z <= bbzmax) {
            return true;
        }
        return false;
    }


    // isWhere
    int isWhere(const EGS_Vector &r) {
        if (!isInside(r)) {
            return -1;
        }
        int ix, iy, iz;
        setIndices(r,ix,iy,iz);
        return getNode(ix,iy,iz)->region;
    }


    // inside (deprecated)
    int inside(const EGS_Vector &r) {
        return isWhere(r);
    }


    // isWhereFast
    int isWhereFast(const EGS_Vector &r) {
        if (!isInside(r)) {
            return -1;
        }
        int ix, iy, iz;
        setIndices(r, ix, iy, iz);
        return getNode(ix,iy,iz)->region;
    }


    // medium
    int medium(int ireg) const {
        return nodeReg[ireg]->medium;
    }


    // howfarIn
    int howfarIn(EGS_Octree_node *node, const EGS_Vector &r, const EGS_Vector &u, EGS_Float &t, EGS_Vector *normal=0) {

        int ix, iy, iz, tmp;
        int crossed = -1;

        if (!node) {
            return -1;
        }

        // set shift and local cell indices
        int shift = maxlevel - node->level;                     // how many levels missing between current level and full depth
        ix = node->ix;
        iy = node->iy;
        iz = node->iz;

        // x direction
        if (u.x > 0) {
            ix = ~(~ix<<shift);                                 // fill lower level bits with 1's to always consider +x below current node
            EGS_Float xBound;
            if (ix>=ixmax) {
                xBound = bbxmax;
            }
            else {
                xBound = xmin + (ix+1)*dx;
            }
            EGS_Float d = (xBound-r.x) / u.x;
            if (d <= t) {
                t = d;
                crossed = 0;
                ix++;
                if (normal) {
                    *normal = EGS_Vector(-1,0,0);
                }
            }
        }
        else if (u.x < 0) {
            ix = ix<<shift;                                     // fill lower level bits with 0's to always consider -x below current node
            EGS_Float xBound;
            if (ix<=ixmin) {
                xBound = bbxmin;
            }
            else {
                xBound = xmin + ix*dx;
            }
            EGS_Float d = (xBound-r.x) / u.x;
            if (d <= t) {
                t = d;
                crossed = 0;
                ix--;
                if (normal) {
                    *normal = EGS_Vector(1,0,0);
                }
            }
        }


        // y direction
        if (u.y > 0) {
            iy = ~(~iy<<shift);                                 // fill lower level bits with 1's to always consider +y below current node
            EGS_Float yBound;
            if (iy>=iymax) {
                yBound = bbymax;
            }
            else {
                yBound = ymin + (iy+1)*dy;
            }
            EGS_Float d = (yBound - r.y) / u.y;
            if (d <= t) {
                t = d;
                crossed = 1;
                iy++;
                if (normal) {
                    *normal = EGS_Vector(0,-1,0);
                }
            }
        }
        else if (u.y < 0) {
            iy = iy<<shift;                                     // fill lower level bits with 0's to always consider -y below current node
            EGS_Float yBound;
            if (iy<=iymin) {
                yBound = bbymin;
            }
            else {
                yBound = ymin + iy*dy;
            }
            EGS_Float d = (yBound-r.y) / u.y;
            if (d <= t) {
                t = d;
                crossed = 1;
                iy--;
                if (normal) {
                    *normal = EGS_Vector(0,1,0);
                }
            }
        }

        // z direction
        if (u.z > 0) {
            iz = ~(~iz<<shift);                                 // fill lower level bits with 1's to always consider +z below current node
            EGS_Float zBound;
            if (iz>=izmax) {
                zBound = bbzmax;
            }
            else {
                zBound = zmin + (iz+1)*dz;
            }
            EGS_Float d = (zBound-r.z) / u.z;
            if (d <= t) {
                t = d;
                crossed = 2;
                iz++;
                if (normal) {
                    *normal = EGS_Vector(0,0,-1);
                }
            }
        }
        else if (u.z < 0) {
            iz = iz<<shift;                                     // fill lower level bits with 0's to always consider -z below current node
            EGS_Float zBound;
            if (iz<=izmin) {
                zBound = bbzmin;
            }
            else {
                zBound = zmin + iz*dz;
            }
            EGS_Float d = (zBound-r.z) / u.z;
            if (d <= t) {
                t = d;
                crossed = 2;
                iz--;
                if (normal) {
                    *normal = EGS_Vector(0,0,1);
                }
            }
        }

        // get the new region index for the neighbor cell:
        // 1) find the new position in the plane perpendicular to the crossing direction
        // 2) get the indices for the neighbor cell at maximum depth, corresponding to that position
        // 3) call an axis specific function to get the neighbor node
        if (crossed==0) {
            EGS_Vector ryz(r.x, r.y+t*u.y, r.z+t*u.z);
            setIndices(ryz, tmp, iy, iz);
            node = getNeighborNodeX(node, ix, iy, iz);
        }
        else if (crossed==1) {
            EGS_Vector rxz(r.x+t*u.x, r.y, r.z+t*u.z);
            setIndices(rxz, ix, tmp, iz);
            node = getNeighborNodeY(node, ix, iy, iz);
        }
        else if (crossed==2) {
            EGS_Vector rxy(r.x+t*u.x, r.y+t*u.y, r.z);
            setIndices(rxy, ix, iy, tmp);
            node = getNeighborNodeZ(node, ix, iy, iz);
        }

        if (node) {
            return node->region;
        }
        else {
            return -1;
        }
    }


    // howfarOut
    int howfarOut(const EGS_Vector &r, const EGS_Vector &u, EGS_Float &t, EGS_Vector *normal=0) {

        int tmp;
        int ix=0, iy=0, iz=0;
        EGS_Float d, tlong = 2*t;
        EGS_Octree_node *node = NULL;

        // x axis
        if (r.x <= bbxmin && u.x > 0) {
            ix = ixmin;
            d = (bbxmin-r.x) / u.x;
        }
        else if (r.x >= bbxmax && u.x < 0) {
            ix = ixmax;
            d = (bbxmax-r.x) / u.x;
        }
        else {
            d = tlong;
        }
        if (d <= t) {
            EGS_Float yy = r.y + u.y*d;
            EGS_Float zz = r.z + u.z*d;
            if (yy >= bbymin && yy <= bbymax && zz >= bbzmin && zz <= bbzmax) {
                EGS_Vector rr(0,yy,zz);
                setIndices(rr, tmp, iy, iz);
                t = d;
                if (normal) {
                    *normal = (ix == ixmin) ? EGS_Vector(-1,0,0) : EGS_Vector(1,0,0);
                }
                node = getNode(ix,iy,iz);
                return node->region;
            }
        }

        // y axis
        if (r.y <= bbymin && u.y > 0) {
            iy = iymin;
            d = (bbymin-r.y) / u.y;
        }
        else if (r.y >= bbymax && u.y < 0) {
            iy = iymax;
            d = (bbymax-r.y) / u.y;
        }
        else {
            d = tlong;
        }
        if (d <= t) {
            EGS_Float xx = r.x + u.x*d;
            EGS_Float zz = r.z + u.z*d;
            if (xx >= bbxmin && xx <= bbxmax && zz >= bbzmin && zz <= bbzmax) {
                EGS_Vector rr(xx,0,zz);
                setIndices(rr, ix, tmp, iz);
                t = d;
                if (normal) {
                    *normal = (iy == iymin) ? EGS_Vector(0,-1,0) : EGS_Vector(0,1,0);
                }
                node = getNode(ix,iy,iz);
                return node->region;
            }
        }

        // z axis
        if (r.z <= bbzmin && u.z > 0) {
            iz = izmin;
            d = (bbzmin-r.z) / u.z;
        }
        else if (r.z >= bbzmax && u.z < 0) {
            iz = izmax;
            d = (bbzmax-r.z) / u.z;
        }
        else {
            d = tlong;
        }
        if (d <= t) {
            EGS_Float xx = r.x + u.x*d;
            EGS_Float yy = r.y + u.y*d;
            if (xx >= bbxmin && xx <= bbxmax && yy >= bbymin && yy <= bbymax) {
                EGS_Vector rr(xx,yy,0);
                setIndices(rr, ix, iy, tmp);
                t = d;
                if (normal) {
                    *normal = (iz == izmin) ? EGS_Vector(0,0,-1) : EGS_Vector(0,0,1);
                }
                node = getNode(ix,iy,iz);
                return node->region;
            }
        }

        return -1;
    }


    // howfar
    int howfar(int ireg, const EGS_Vector &r, const EGS_Vector &u, EGS_Float &t, int *newmed, EGS_Vector *normal=0) {

        int inew = ireg;

        // get new region number
        if (ireg==-1) {
            inew = howfarOut(r, u, t, normal);
        }
        else {
            inew = howfarIn(nodeReg[ireg], r, u, t, normal);
        }

        // set new medium
        if (inew>=0 && newmed) {
            *newmed = nodeReg[inew]->medium;
        }
        return inew;
    }


    // hownearIn
    EGS_Float hownearIn(int ireg, const EGS_Vector &r) {
        EGS_Octree_node *node = nodeReg[ireg];
        int shift = maxlevel - node->level;
        EGS_Float t1, t2, tx, ty, tz;
        int imin, imax;

        // x
        imin = node->ix << shift;
        imax = ~(~node->ix << shift);
        t1 = (r.x-xmin)-dx*imin;
        t2 = dx*(imax-imin+1)-t1;
        tx = t1 < t2 ? t1 : t2;

        // y
        imin = node->iy << shift;
        imax = ~(~node->iy << shift);
        t1 = (r.y-ymin)-dy*imin;
        t2 = dy*(imax-imin+1)-t1;
        ty = t1 < t2 ? t1 : t2;

        // z
        imin = node->iz << shift;
        imax = ~(~node->iz << shift);
        t1 = (r.z-zmin)-dz*imin;
        t2 = dz*(imax-imin+1)-t1;
        tz = t1 < t2 ? t1 : t2;

        return tx<ty && tx<tz ? tx : ty<tz ? ty : tz;
    }


    // hownear
    EGS_Float hownear(int ireg, const EGS_Vector &r) {
        if (ireg>=0) {
            return hownearIn(ireg, r);
        }
        int nc=0;
        EGS_Float s1=0, s2=0;
        if (r.x < bbxmin || r.x > bbxmax) {
            EGS_Float t = r.x < bbxmin ? bbxmin-r.x : r.x-bbxmax;
            nc++;
            s1 += t;
            s2 += t*t;
        }
        if (r.y < bbymin || r.y > bbymax) {
            EGS_Float t = r.y < bbymin ? bbymin-r.y : r.y-bbymax;
            nc++;
            s1 += t;
            s2 += t*t;
        }
        if (r.z < bbzmin || r.z > bbzmax) {
            EGS_Float t = r.z < bbzmin ? bbzmin-r.z : r.z-bbzmax;
            nc++;
            s1 += t;
            s2 += t*t;
        }
        return nc == 1 ? s1 : sqrt(s2);
    }


    // getType
    const string &getType() const {
        return type;
    }

    // printInfo
    void printInfo() const;
};

#endif
