/*
###############################################################################
#
#  EGSnrc egs++ mesh geometry library headers.
#
#  Copyright (C) 2020 Mevex Corporation
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
#  Authors:          Dave Macrillo,
#                    Matt Ronan,
#                    Nigel Vezeau,
#                    Lou Thompson,
#                    Max Orok
#
###############################################################################
*/

/*! \file egs_mesh.h
 *  \brief Tetrahedral mesh geometry: header
 */

#ifndef EGS_MESH
#define EGS_MESH

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <vector>

#include "egs_base_geometry.h"
#include "egs_vector.h"

#ifdef WIN32

    #ifdef BUILD_EGS_MESH_DLL
        #define EGS_MESH_EXPORT __declspec(dllexport)
    #else
        #define EGS_MESH_EXPORT __declspec(dllimport)
    #endif
    #define EGS_MESH_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_MESH_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_MESH_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_MESH_EXPORT
        #define EGS_MESH_LOCAL
    #endif

#endif

class EGS_Mesh_Octree;

/// A container for raw unstructured tetrahedral mesh data.
class EGS_MESH_EXPORT EGS_MeshSpec {
public:
    EGS_MeshSpec() = default;
    EGS_MeshSpec(const EGS_MeshSpec&) = delete;
    EGS_MeshSpec& operator=(const EGS_MeshSpec&) = delete;
    // EGS_MeshSpec is move-only
    EGS_MeshSpec(EGS_MeshSpec&&) = default;
    EGS_MeshSpec& operator=(EGS_MeshSpec&&) = default;
    ~EGS_MeshSpec() = default;

    /// A tetrahedral mesh element
    struct Tetrahedron {
        Tetrahedron(int tag, int medium_tag, int a, int b, int c, int d) :
            tag(tag), medium_tag(medium_tag), a(a), b(b), c(c), d(d) {}
        int tag = -1;
        int medium_tag = -1;
        // nodes
        int a = -1;
        int b = -1;
        int c = -1;
        int d = -1;
    };

    /// A 3D point
    struct Node {
        Node(int tag, double x, double y, double z) :
            tag(tag), x(x), y(y), z(z) {}
        int tag = -1;
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
    };

    /// A physical medium
    struct Medium {
        Medium(int tag, std::string medium_name) :
            tag(tag), medium_name(medium_name) {}
        int tag = -1;
        std::string medium_name;
    };

    /// Throws std::runtime_error if an EGS_Mesh can't be properly initialized
    /// using this mesh data.
    void checkValid() const;

    // Public members

    // Unique mesh elements
    std::vector<EGS_MeshSpec::Tetrahedron> elements;
    // Unique nodes
    std::vector<EGS_MeshSpec::Node> nodes;
    // Unique medium information
    std::vector<EGS_MeshSpec::Medium> media;
};

/*! \brief A tetrahedral mesh geometry
  \ingroup Geometry
  \ingroup ElementaryG
*/

class EGS_MESH_EXPORT EGS_Mesh : public EGS_BaseGeometry {
public:
    /// Throws std::runtime_error if construction fails.
    EGS_Mesh(EGS_MeshSpec spec);

    // EGS_Mesh is move-only
    EGS_Mesh(const EGS_Mesh&) = delete;
    EGS_Mesh& operator=(const EGS_Mesh&) = delete;
    EGS_Mesh(EGS_Mesh&&) = default;
    EGS_Mesh& operator=(EGS_Mesh&&) = default;

    // Just declare destructor without defining it. We can't define it yet
    // because of the unique_ptr to forward declared EGS_Mesh_Octree members.
    ~EGS_Mesh();

    int num_elements() const {
        return _elt_tags.size();
    }

    const std::vector<std::string>& medium_names() const {
        return _medium_names;
    }

    const std::vector<std::array<int, 4>>& neighbours() const {
        return _neighbours;
    }

    // Return element volume [cm3].
    EGS_Float element_volume(int i) const {
        const auto& n = element_nodes(i);
        return std::abs((n.A - n.D) * ((n.B - n.D) % (n.C - n.D))) / 6.0;
    }

    // Return element density [g/cm3].
    EGS_Float element_density(int i) const {
        return EGS_BaseGeometry::getMediumRho(_medium_indices.at(i));
    }

    // Return element tag.
    int element_tag(int i) const {
        return _elt_tags.at(i);
    }

    inline bool is_boundary(int reg) const {
        return _boundary_faces[4*reg] || _boundary_faces[4*reg + 1] ||
               _boundary_faces[4*reg + 2] || _boundary_faces[4*reg + 3];
    }

    const std::string& filename() const {
        return _filename;
    }
    void setFilename(std::string filename) {
        _filename = filename;
    }
    void printElement(int i, std::ostream& elt_info = std::cout) const {
        const auto& n = element_nodes(i);
      elt_info << " Tetrahedron " << i << ":\n"
          << " \tNode coordinates (cm):\n"
          << " \t0: " << n.A.x << " " << n.A.y << " " << n.A.z << "\n"
          << " \t1: " << n.B.x << " " << n.B.y << " " << n.B.z << "\n"
          << " \t2: " << n.C.x << " " << n.C.y << " " << n.C.z << "\n"
          << " \t3: " << n.D.x << " " << n.C.y << " " << n.C.z << "\n"
          << " \tNeighbour elements:\n"
          << " \t\tOn face 0: " << _neighbours[i][0] << "\n"
          << " \t\tOn face 1: " << _neighbours[i][1] << "\n"
          << " \t\tOn face 2: " << _neighbours[i][2] << "\n"
          << " \t\tOn face 3: " << _neighbours[i][3] << "\n"
          << std::boolalpha
          << " \tBoundary element: " << is_boundary(i) << "\n"
          << " \tMedia index: "<< _medium_indices[i] << "\n";
    }

    // EGS_BaseGeometry interface
    const std::string& getType() const override { return type; }
    bool isInside(const EGS_Vector &x) override;
    int inside(const EGS_Vector &x) override; // TODO figure out setMedia() situation
    int medium(int ireg) const override;
    int isWhere(const EGS_Vector &x) override;
    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
        EGS_Float &t, int *newmed=0, EGS_Vector *normal=0) override;
    EGS_Float hownear(int ireg, const EGS_Vector &x) override;
    void printInfo() const override;

    // Check if a point x is inside element i.
    bool insideElement(int i, const EGS_Vector &x);

    struct Intersection {
        Intersection(EGS_Float dist, int face_index)
            : dist(dist), face_index(face_index) {}
        /// Intersection distance
        EGS_Float dist;
        /// Face index
        int face_index;
    };

    // `howfar` helper: Determine the closest boundary face intersection
    Intersection closest_boundary_face(int ireg, const EGS_Vector& x, const EGS_Vector& u);

    struct Nodes {
        const EGS_Vector& A;
        const EGS_Vector& B;
        const EGS_Vector& C;
        const EGS_Vector& D;
    };

    Nodes element_nodes(int element) const {
        const auto& node_indices = _elt_node_indices.at(element);
        return Nodes {
            _nodes.at(node_indices[0]),
            _nodes.at(node_indices[1]),
            _nodes.at(node_indices[2]),
            _nodes.at(node_indices[3])
        };
    }

private:
    // `hownear` helper method
    // Given a tetrahedron ireg, find the minimum distance to a face in any direction.
    EGS_Float min_interior_face_dist(int ireg, const EGS_Vector& x);

    // `hownear` helper method
    // Outside the mesh, find the minimum distance to the mesh in any direction (ireg = -1)
    EGS_Float min_exterior_face_dist(const EGS_Vector& x);

    // `howfar` helper method inside a given tetrahedron
    int howfar_interior(int ireg, const EGS_Vector &x, const EGS_Vector &u,
        EGS_Float &t, int *newmed, EGS_Vector *normal);

    // Find the region where lost particles in `howfar_interior` are.
    int howfar_interior_find_lost_particle(int ireg, const EGS_Vector &x,
        const EGS_Vector &u);

    // `howfar` helper method outside the mesh
    int howfar_exterior(int ireg, const EGS_Vector &x, const EGS_Vector &u,
        EGS_Float &t, int *newmed, EGS_Vector *normal);

    inline void update_medium(int newreg, int *newmed) const {
        if (!newmed) {
            return;
        }
        if (newreg == -1) {
            // vacuum
            *newmed = -1;
        } else {
            *newmed = medium(newreg);
        }
    }

    inline void update_normal(const EGS_Vector& face_normal,
        const EGS_Vector& u_norm, EGS_Vector *normal) const
    {
        if (!normal) {
            return;
        }
        // egs++ convention is normal pointing opposite view ray
        if (face_normal * u_norm > 0.0) {
            *normal = -1.0 * face_normal;
        } else {
            *normal = face_normal;
        }
    }

    // Constructor helper methods:
    //
    // Each logical piece is a separate function to help reduce peak memory
    // usage. For large meshes, having temporaries around until the end of the
    // constructor, including during intense operations like constructing the
    // octrees, etc. can raise the peak memory usage far above the steady state
    // requirements.

    // Initialize the mesh element information in the EGS_Mesh constructor.
    // Must be called before any other initialize functions.  After this method
    // is called, the following member data is initialized:
    // * EGS_Mesh::_nodes
    // * EGS_Mesh::_elt_tags
    // * EGS_Mesh::_elt_node_indices
    // * EGS_BaseGeometry::nreg
    // and member functions that depend on this data, like num_elements().
    void initializeElements(std::vector<EGS_MeshSpec::Tetrahedron> elements,
        std::vector<EGS_MeshSpec::Node> nodes,
        std::vector<EGS_MeshSpec::Medium> materials);

    // Initialize neigbhour and boundary information. Must be called after
    // initializeElements. Responsible for initializing:
    // * EGS_Mesh::_neighbours
    // * EGS_Mesh::_boundary_faces
    void initializeNeighbours();

    // Initialize the two octrees used to accelerate transport. Both
    // initializeElements and initializeNeighbours must be called to properly
    // set up the octrees. Responsible for initializing:
    // * EGS_Mesh::_volume_tree
    // * EGS_Mesh::_surface_tree
    void initializeOctrees();

    // Initialize the tetrahedron face normals. Must be called after
    // initializeElements. Responsible for initializing EGS_Mesh::_face_normals.
    void initializeNormals();

    std::vector<EGS_Vector> _nodes;
    std::vector<int> _elt_tags;
    std::vector<std::array<int, 4>> _elt_node_indices;
    // 4 * num_elts of which faces are boundaries
    // TODO: try vec<array<bool, 4>>
    std::vector<bool> _boundary_faces;
    // TODO if memory is an issue, could try storing tets as sets of faces,
    // faces as sets of edges, etc.
    std::vector<std::array<EGS_Vector, 4>> _face_normals;
    std::vector<int> _medium_indices;
    std::vector<std::string> _medium_names;
    std::string _filename;

    std::unique_ptr<EGS_Mesh_Octree> _volume_tree;
    std::unique_ptr<EGS_Mesh_Octree> _surface_tree;

    std::vector<std::array<int, 4>> _neighbours;
    static const std::string type;
};

#endif // EGS_MESH
