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

/*
class point {
   EGS_Vector p_;
public:
    point(EGS_Vector p) : p_(p) {}
    double get(size_t index) const {
        if (index == 0) { return p_.x; }
        if (index == 1) { return p_.y; }
        if (index == 2) { return p_.z; }
        throw std::runtime_error("index out of bounds");
    }
    // Returns distance squared to another point
    double distance(const point& pt) const {
        auto diff = p_ - pt.p_;
        return diff.length2();
    }
};

std::ostream& operator<<(std::ostream& out, const point& pt) {
    out << '(';
    for (size_t i = 0; i < 3; ++i) {
        if (i > 0)
            out << ", ";
        out << pt.get(i);
    }
    out << ')';
    return out;
}

class kdtree {
private:
    struct node {
        node(const point& pt) : point_(pt), left_(nullptr), right_(nullptr) {}
        double get(size_t index) const {
            return point_.get(index);
        }
        double distance(const point& pt) const {
            return point_.distance(pt);
        }
        point point_;
        node* left_;
        node* right_;
        // to lookup which tetrahedron this centroid belongs to
        int offset_ = -1;
    };
    node* root_ = nullptr;
    node* best_ = nullptr;
    double best_dist_ = 0;
    size_t visited_ = 0;
    std::vector<node> nodes_;

    struct node_cmp {
        node_cmp(size_t index) : index_(index) {}
        bool operator()(const node& n1, const node& n2) const {
            return n1.point_.get(index_) < n2.point_.get(index_);
        }
        size_t index_;
    };

    node* make_tree(size_t begin, size_t end, size_t index) {
        if (end <= begin)
            return nullptr;
        size_t n = begin + (end - begin)/2;
        auto i = nodes_.begin();
        std::nth_element(i + begin, i + n, i + end, node_cmp(index));
        index = (index + 1) % 3;
        nodes_[n].left_ = make_tree(begin, n, index);
        nodes_[n].right_ = make_tree(n + 1, end, index);
        return &nodes_[n];
    }

    void nearest(node* root, const point& point, size_t index) {
        if (root == nullptr)
            return;
        ++visited_;
        double d = root->distance(point);
        if (best_ == nullptr || d < best_dist_) {
            best_dist_ = d;
            best_ = root;
        }
        if (best_dist_ == 0)
            return;
        double dx = root->get(index) - point.get(index);
        index = (index + 1) % 3;
        nearest(dx > 0 ? root->left_ : root->right_, point, index);
        if (dx * dx >= best_dist_)
            return;
        nearest(dx > 0 ? root->right_ : root->left_, point, index);
    }
public:
    kdtree() = default;
    template<typename iterator>
    kdtree(iterator begin, iterator end) : nodes_(begin, end) {
        for (std::size_t i = 0; i < nodes_.size(); i++) {
            nodes_[i].offset_ = i;
        }
        root_ = make_tree(0, nodes_.size(), 0);
    }

    bool empty() const { return nodes_.empty(); }
    size_t visited() const { return visited_; }
    double distance() const { return std::sqrt(best_dist_); }
    const node& nearest(const point& pt) {
        if (root_ == nullptr)
            throw std::logic_error("tree is empty");
        best_ = nullptr;
        visited_ = 0;
        best_dist_ = 0;
        nearest(root_, pt, 0);
        return *best_;
    }
};
*/

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

/*! \brief A tetrahedral mesh geometry
  \ingroup Geometry
  \ingroup ElementaryG
*/

class EGS_Mesh_Octree;

class EGS_MESH_EXPORT EGS_Mesh : public EGS_BaseGeometry {
public:
    /// A single tetrahedral mesh element
    struct Tetrahedron {
        Tetrahedron(int medium_tag, int a, int b, int c, int d) :
            medium_tag(medium_tag), a(a), b(b), c(c), d(d) {}
        int medium_tag = -1;
        // nodes
        int a = -1;
        int b = -1;
        int c = -1;
        int d = -1;
    };

    /// A single 3D point
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

    // TODO: refactor to MeshSpec struct
    EGS_Mesh(std::vector<EGS_Mesh::Tetrahedron> elements,
        std::vector<EGS_Mesh::Node> nodes, std::vector<EGS_Mesh::Medium> materials);

    ~EGS_Mesh() = default;

    /// Parse a msh file into an owned EGS_Mesh allocated using new.
    ///
    /// Throws a std::runtime_error if parsing fails.
    static EGS_Mesh* parse_msh_file(std::istream& input);

    int num_elements() const {
        // todo return nreg
        return _elt_points.size() / 4;
    }
    const std::vector<std::string>& medium_names() const {
        return _medium_names;
    }
    const std::vector<EGS_Mesh::Tetrahedron>& elements() const {
        return _elements;
    }
    const std::vector<EGS_Mesh::Node>& nodes() const {
        return _nodes;
    }
    const std::vector<EGS_Mesh::Medium>& materials() const {
        return _materials;
    }
    const std::vector<std::array<int, 4>>& neighbours() const {
        return _neighbours;
    }
    const std::vector<EGS_Vector>& points() const {
        return _elt_points;
    }
    std::vector<EGS_Float> volumes() const {
        std::vector<EGS_Float> volumes;
        volumes.reserve(num_elements());
        for (int i = 0; i < num_elements(); i++) {
            volumes.push_back(std::abs(
                (_elt_points[4*i] - _elt_points[4*i+3]) *
                    ((_elt_points[4*i+1] - _elt_points[4*i+3])
                        % (_elt_points[4*i+2] - _elt_points[4*i+3]))) / 6.0);
        }
        return volumes;
    }
    // Return element densities [g/cm3].
    std::vector<EGS_Float> densities() const {
        std::vector<EGS_Float> densities;
        densities.reserve(num_elements());
        for (int i = 0; i < num_elements(); i++) {
            densities.push_back(getMediumRho(_medium_indices[i]));
        }
        return densities;
    }
    std::vector<int> tags() const {
        return _elt_tags;
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
      elt_info << " Tetrahedron " << i << ":\n"
          << " \tNode coordinates (cm):\n"
          << " \t0: " << _elt_points[4*i].x << " " << _elt_points[4*i].y << " " << _elt_points[4*i].z << "\n"
          << " \t1: " << _elt_points[4*i+1].x << " " << _elt_points[4*i+1].y << " " << _elt_points[4*i+1].z << "\n"
          << " \t2: " << _elt_points[4*i+2].x << " " << _elt_points[4*i+2].y << " " << _elt_points[4*i+2].z << "\n"
          << " \t3: " << _elt_points[4*i+3].x << " " << _elt_points[4*i+3].y << " " << _elt_points[4*i+3].z << "\n"
          << " \tNeighbour elements:\n"
          << " \t\tOn face 0: " << _neighbours[i][0] << "\n"
          << " \t\tOn face 1: " << _neighbours[i][1] << "\n"
          << " \t\tOn face 2: " << _neighbours[i][2] << "\n"
          << " \t\tOn face 3: " << _neighbours[i][3] << "\n"
          << std::boolalpha
          << " \tBoundary element: " << is_boundary(i) << "\n"
          << " \tMedia index: "<< _medium_indices[i] << "\n";
    }

    // Order the mesh elements by their distance to x to improve performance.
    void reorderMesh(const EGS_Vector &x);

    // reorderMesh helper: renumber all the internal data vectors.
    void renumberMesh(const std::vector<int>& reordered_tags);

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
        return Nodes {
            _elt_points.at(4*element),
            _elt_points.at(4*element + 1),
            _elt_points.at(4*element + 2),
            _elt_points.at(4*element + 3),
        };
    }

private:
    // `hownear` helper method
    // Given a tetrahedron ireg, find the minimum distance to a face in any direction.
    EGS_Float min_interior_face_dist(int ireg, const EGS_Vector& x);

    // `hownear` helper method
    // Outside the mesh, find the minimum distance to the mesh in any direction (ireg = -1)
    EGS_Float min_exterior_face_dist(int ireg, const EGS_Vector& x);

    // `howfar` helper method inside a given tetrahedron
    int howfar_interior(int ireg, const EGS_Vector &x, const EGS_Vector &u,
        EGS_Float &t, int *newmed, EGS_Vector *normal);

    // `howfar` helper method outside the mesh
    int howfar_exterior(int ireg, const EGS_Vector &x, const EGS_Vector &u,
        EGS_Float &t, int *newmed, EGS_Vector *normal);

    std::vector<int> findNeighbourhood(int elt);

    // Initialize the two octrees used to accelerate transport
    void initOctrees();

    std::vector<int> _elt_tags;
    std::vector<EGS_Vector> _elt_points;
    // 4 * num_elts of which faces are boundaries
    // TODO: try vec<array<bool, 4>>
    std::vector<bool> _boundary_faces;
    std::vector<int> _medium_indices;
    std::vector<std::string> _medium_names;
    std::string _filename;

    std::unique_ptr<EGS_Mesh_Octree> _volume_tree;
    std::unique_ptr<EGS_Mesh_Octree> _surface_tree;

    // TODO: check and remove
    std::vector<EGS_Mesh::Tetrahedron> _elements;
    std::vector<EGS_Mesh::Node> _nodes;
    std::vector<EGS_Mesh::Medium> _materials;

    std::vector<std::array<int, 4>> _neighbours;
    static const std::string type;
};

#endif // EGS_MESH
