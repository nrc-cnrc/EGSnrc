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
#include <chrono>
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

    EGS_MeshSpec() = default;
    EGS_MeshSpec(std::vector<Tetrahedron> elements, std::vector<Node> nodes,
        std::vector<Medium> media) : elements(std::move(elements)),
            nodes(std::move(nodes)), media(std::move(media)) {}
    EGS_MeshSpec(const EGS_MeshSpec&) = delete;
    EGS_MeshSpec& operator=(const EGS_MeshSpec&) = delete;
    // EGS_MeshSpec is move-only
    EGS_MeshSpec(EGS_MeshSpec&&) = default;
    EGS_MeshSpec& operator=(EGS_MeshSpec&&) = default;
    ~EGS_MeshSpec() = default;

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
    explicit EGS_Mesh(EGS_MeshSpec spec);

    // EGS_Mesh is move-only
    EGS_Mesh(const EGS_Mesh&) = delete;
    EGS_Mesh& operator=(const EGS_Mesh&) = delete;
    EGS_Mesh(EGS_Mesh&&);
    EGS_Mesh& operator=(EGS_Mesh&&);

    // Just declare destructor without defining it. We can't define it yet
    // because of the unique_ptr to forward declared EGS_Mesh_Octree members.
    ~EGS_Mesh();

    int num_elements() const {
        return _elt_tags.size();
    }

    int num_nodes() const {
        return _nodes.size();
    }

    const std::vector<std::string>& medium_names() const {
        return _medium_names;
    }

    const std::array<int, 4>& element_neighbours(int i) const {
        return _neighbours.at(i);
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
    int inside(const EGS_Vector &x) override;
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

    /// Given a node offset (from 0 to EGS_Mesh::num_nodes() - 1), returns the
    /// node coordinates.
    const EGS_Vector& node_coordinates(int node_offset) const {
        return _nodes.at(node_offset);
    }

    /// Given an element offset, return its four node offsets.
    const std::array<int, 4>& element_node_offsets(int element) const {
        return _elt_node_indices.at(element);
    }

    static EGS_Float get_min_step_size() {
        return EGS_Mesh::min_step_size;
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

    // `howfar_interior` helper methods

    struct PointLocation {
        PointLocation() = default;
        PointLocation(EGS_Float direction_dot_normal, EGS_Float signed_distance)
            : direction_dot_normal(direction_dot_normal), signed_distance(
                signed_distance) {}

        EGS_Float direction_dot_normal = 0.0;
        EGS_Float signed_distance = 0.0;
    };

    PointLocation find_point_location(const EGS_Vector& x, const
        EGS_Vector& u, const EGS_Vector& plane_point, const EGS_Vector&
        plane_normal);

    Intersection find_interior_intersection(
        const std::array<PointLocation, 4>& ixs);

    int howfar_interior_thick_plane(
        const std::array<PointLocation, 4>& intersect_tests, int ireg,
        const EGS_Vector &x, const EGS_Vector &u, EGS_Float &t, int *newmed,
        EGS_Vector *normal);

    // If the particle is lost, try to recover transport by shifting the
    // particle along a small step.
    int howfar_interior_recover_lost_particle(int ireg, const EGS_Vector &x,
        const EGS_Vector &u, EGS_Float &t, int *newmed);

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

    // Can only be called after initializeElements (for num_elements())
    EGS_InfoFunction get_logger() const {
        if (num_elements() < 50000) {
            // don't log for small meshes since loading is near instantaneous
            return nullptr;
        }
        return egsInformation;
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

    // Initialize mesh element medium offsets, adding any previously undefined
    // media to the EGS_BaseGeometry media list. Responsible for initializing:
    // * EGS_Mesh::_medium_indices
    // * EGS_Mesh::_medium_names
    void initializeMedia(std::vector<EGS_MeshSpec::Tetrahedron> elements,
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

    static constexpr EGS_Float min_step_size = 1e-10;
};

namespace egs_mesh {

// The egs_mesh::internal namespace is for internal API functions and may change
// without warning.
namespace internal {

/// Percent display counter for potentially long EGS_Mesh initialization steps.
class EGS_MESH_EXPORT PercentCounter {
public:
    // Create a new counter. This doesn't log anything or start the timer yet.
    // The counter must be activated later using `start`.
    PercentCounter(EGS_InfoFunction info, const std::string& msg);

    // Start the counter's progress toward the provided goal. The goal must be
    // a positive number and fit into an int. Logging and timing begin at this
    // point.
    void start(EGS_Float goal);

    // Advance the counter by a fraction of the goal value. Assumes delta is
    // positive.
    void step(EGS_Float delta);

    // After the task is finished, print a new message and the elapsed time.
    void finish(const std::string& end_msg);

private:
    // egsInformation-like function pointer.
    EGS_InfoFunction info_;
    // Progress message.
    std::string msg_;
    // Goal value.
    double goal_ = 0.0;
    // Progress value, percent complete = progress_ / goal_.
    double progress_ = 0.0;
    // The last progress percentage that was displayed, used to avoid redundant
    // I/O calls.
    int old_percent_ = 0;
    // The time when PercentCounter::start was called.
    std::chrono::time_point<std::chrono::system_clock> t_start_;
    // Whether to update the percentage in real time (true) or just report when
    // the task is finished (false).
    bool interactive_ = false;
};

} // namespace internal

} // namespace egs_mesh

#endif // EGS_MESH
