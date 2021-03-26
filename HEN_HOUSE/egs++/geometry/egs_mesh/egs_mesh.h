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

#include <iostream>
#include <iomanip>
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

/*! \brief A tetrahedral mesh geometry
  \ingroup Geometry
  \ingroup ElementaryG
*/

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

    bool is_boundary(int reg) const;

    const std::string& filename() const {
        return _filename;
    }
    void setFilename(std::string filename) {
        _filename = filename;
    }
    void printElement(int i, std::ostream& elt_info = std::cout) const {
      elt_info << "Tetrahedron " << i << ":\n"
          << "\tNode coordinates (cm):\n"
          << "\t0: " << _elt_points[4*i].x << " " << _elt_points[4*i].y << " " << _elt_points[4*i].z << "\n"
          << "\t1: " << _elt_points[4*i+1].x << " " << _elt_points[4*i+1].y << " " << _elt_points[4*i+1].z << "\n"
          << "\t2: " << _elt_points[4*i+2].x << " " << _elt_points[4*i+2].y << " " << _elt_points[4*i+2].z << "\n"
          << "\t3: " << _elt_points[4*i+3].x << " " << _elt_points[4*i+3].y << " " << _elt_points[4*i+3].z << "\n"
          << "\tNeighbour elements:\n"
          << "\t\tOn face 0: " << _neighbours[i][0] << "\n"
          << "\t\tOn face 1: " << _neighbours[i][1] << "\n"
          << "\t\tOn face 2: " << _neighbours[i][2] << "\n"
          << "\t\tOn face 3: " << _neighbours[i][3] << "\n"
          << std::boolalpha
          << "\tBoundary element: " << is_boundary(i) << "\n"
          << "\tMedia index: "<< _medium_indices[i] << "\n";
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

    std::vector<EGS_Vector> _elt_points;
    std::vector<bool> _boundary_faces;
    std::vector<int> _medium_indices;
    std::vector<std::string> _medium_names;
    std::string _filename;

    std::vector<EGS_Mesh::Tetrahedron> _elements;
    std::vector<EGS_Mesh::Node> _nodes;
    std::vector<EGS_Mesh::Medium> _materials;

    std::vector<std::array<int, 4>> _neighbours;
    const std::string type = "EGS_Mesh";
};

#endif // EGS_MESH
