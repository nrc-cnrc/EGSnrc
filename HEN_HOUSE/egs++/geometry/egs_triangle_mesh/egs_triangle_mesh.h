/*
###############################################################################
#
#  EGSnrc egs++ triangle mesh geometry library headers.
#  Copyright (C) 2022 Max Orok
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
#  Author:          Max Orok, 2022
#
#  Contributors:
#
###############################################################################
*/


/*! \file egs_triangle_mesh.h
 *  \brief Triangle surface mesh geometry: header
 */

#ifndef EGS_TRIANGLE_MESH
#define EGS_TRIANGLE_MESH

#include "egs_base_geometry.h"
#include "egs_vector.h"

#include <array>
#include <memory>
#include <string>
#include <vector>

#ifdef WIN32

    #ifdef BUILD_EGS_TRIANGLE_MESH_DLL
        #define EGS_TRIANGLE_MESH_EXPORT __declspec(dllexport)
    #else
        #define EGS_TRIANGLE_MESH_EXPORT __declspec(dllimport)
    #endif
    #define EGS_TRIANGLE_MESH_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_TRIANGLE_MESH_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_TRIANGLE_MESH_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_TRIANGLE_MESH_EXPORT
        #define EGS_TRIANGLE_MESH_LOCAL
    #endif

#endif

/// A container for raw unstructured triangle surface mesh data.
class EGS_TRIANGLE_MESH_EXPORT EGS_TriangleMeshSpec {
public:

    // A triangle with a unit normal facing outward from the surface.
    struct Triangle {
        Triangle() = default;
        Triangle(EGS_Vector a, EGS_Vector b, EGS_Vector c, EGS_Vector n) :
            a(a), b(b), c(c), n(n) {}
        // nodes
        EGS_Vector a;
        EGS_Vector b;
        EGS_Vector c;
        // normal
        EGS_Vector n;
    };

    EGS_TriangleMeshSpec() = default;
    EGS_TriangleMeshSpec(std::vector<Triangle> elements) :
        elements(std::move(elements)) {}

    // EGS_TriangleMeshSpec is move-only
    EGS_TriangleMeshSpec(const EGS_TriangleMeshSpec &) = delete;
    EGS_TriangleMeshSpec &operator=(const EGS_TriangleMeshSpec &) = delete;
    EGS_TriangleMeshSpec(EGS_TriangleMeshSpec &&) = default;
    EGS_TriangleMeshSpec &operator=(EGS_TriangleMeshSpec &&) = default;
    ~EGS_TriangleMeshSpec() = default;

    /// Multiply all node coordinates by a constant factor.
    void scale(EGS_Float factor) {
        for (auto &e: elements) {
            e.a *= factor;
            e.b *= factor;
            e.c *= factor;
        }
    }

    std::size_t num_elements() const {
        return elements.size();
    }

    // Public members

    /// Unique elements
    std::vector<EGS_TriangleMeshSpec::Triangle> elements;
};

// exclude from doxygen
/// @cond
class EGS_TriangleMeshBbox;
/// @endcond

/*! \brief A triangular surface mesh geometry.

  \ingroup Geometry
  \ingroup ElementaryG

The EGS_TriangleMesh class implements an unstructured triangular surface mesh.
Input files should be closed surfaces made of a single medium. Simulations using
multiple media can be implemented using multiple triangle meshes. The volume
enclosed by the triangular mesh is treated as a single transport region.

The following file formats are supported:
    * ASCII STL,
    * Binary STL (little-endian)

\verbatim
:start geometry definition:
    :start geometry:
        name        = my_surface_mesh
        library     = egs_triangle_mesh
        file        = model.stl # your surface mesh here
        :start media input:
            media = water
        :stop media input:
        # scale = 0.1 # optional scaling factor. Mesh files are assumed to be in `cm`.
    :stop geometry:

    simulation geometry = my_surface_mesh
:stop geometry definition:
\endverbatim

Meshes should be enclosed in an envelope so particles that exit the mesh and
reenter (for example, if there's a cutout feature in the mesh) are simulated
correctly and not terminated immediately after initially exiting the mesh.

\verbatim

:start geometry definition:
    :start geometry:
        name = my_surface_mesh
        library = egs_triangle_mesh
        file = model.stl
        :start media input:
            media = water
        :stop media input:
    :stop geometry:

    # define a 50cm vacuum cube at the origin
    :start geometry:
        library = egs_box
        name = my_box
        box size = 50 50 50
        :start media input:
            media = vacuum
        :stop media input:
    :stop geometry:

    # embed the mesh in the vacuum box
    :start geometry:
        library = egs_genvelope
        name = my_envelope
        base geometry = my_box
        inscribed geometries = my_surface_mesh
    :stop geometry:

    simulation geometry = my_envelope
:stop geometry definition:
\endverbatim
*/

class EGS_TRIANGLE_MESH_EXPORT EGS_TriangleMesh : public EGS_BaseGeometry {
public:
    explicit EGS_TriangleMesh(EGS_TriangleMeshSpec);

    // EGS_TriangleMesh is move-only
    EGS_TriangleMesh(const EGS_TriangleMesh &) = delete;
    EGS_TriangleMesh &operator=(const EGS_TriangleMesh &) = delete;
    EGS_TriangleMesh(EGS_TriangleMesh &&);
    EGS_TriangleMesh &operator=(EGS_TriangleMesh &&);
    ~EGS_TriangleMesh();

    /// Returns the number of triangles in the surface mesh. Note the entire
    /// volume bounded by the mesh is treated as a single transport region.
    int num_triangles() const {
        return n_tris;
    }

    /// Returns the three triangle node x-coordinates.
    const std::array<EGS_Float, 3>& triangle_xs(int tri) const {
        return xs.at(tri);
    }

    /// Returns the three triangle node y-coordinates.
    const std::array<EGS_Float, 3>& triangle_ys(int tri) const {
        return ys.at(tri);
    }

    /// Returns the three triangle node z-coordinates.
    const std::array<EGS_Float, 3>& triangle_zs(int tri) const {
        return zs.at(tri);
    }

    /// Returns the outward-facing triangle unit normal.
    const EGS_Vector& triangle_normal(int tri) const {
        return ns.at(tri);
    }

    // EGS_BaseGeometry interface

    const std::string &getType() const override {
        return EGS_TriangleMesh::type;
    }

    bool isInside(const EGS_Vector &x) override;
    int inside(const EGS_Vector &x) override;
    int isWhere(const EGS_Vector &x) override;
    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t, int *newmed=0, EGS_Vector *normal=0) override;
    EGS_Float hownear(int ireg, const EGS_Vector &x) override;

    static const std::string type;

private:
    int n_tris = -1;
    // Vectors with len = n_tris
    std::vector<std::array<EGS_Float, 3>> xs;
    std::vector<std::array<EGS_Float, 3>> ys;
    std::vector<std::array<EGS_Float, 3>> zs;
    std::vector<EGS_Vector> ns;

    // Axis-aligned mesh bounding box used to accelerate geometry routines
    std::unique_ptr<EGS_TriangleMeshBbox> bbox;
};

#endif // EGS_TRIANGLE_MESH
