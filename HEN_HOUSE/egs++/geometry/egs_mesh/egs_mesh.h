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

// TODO
// #include "egs_base_geometry.h"
#include "egs_vector.h"

// #ifdef WIN32
//
//     #ifdef BUILD_EGS_MESH_DLL
//         #define EGS_MESH_EXPORT __declspec(dllexport)
//     #else
//         #define EGS_MESH_EXPORT __declspec(dllimport)
//     #endif
//     #define EGS_MESH_LOCAL
//
// #else
//
//     #ifdef HAVE_VISIBILITY
//         #define EGS_MESH_EXPORT __attribute__ ((visibility ("default")))
//         #define EGS_MESH_LOCAL  __attribute__ ((visibility ("hidden")))
//     #else
//         #define EGS_MESH_EXPORT
//         #define EGS_MESH_LOCAL
//     #endif
//
// #endif

/*! \brief A tetrahedral mesh geometry
  \ingroup Geometry
  \ingroup ElementaryG
*/

// TODO
// class EGS_MESH_EXPORT EGS_Mesh : public EGS_BaseGeometry {
class EGS_Mesh {
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
        std::vector<EGS_Mesh::Node> nodes, std::vector<EGS_Mesh::Medium> materials) :
        /* EGS_BaseGeometry("EGS_Mesh"), */ _elements(std::move(elements)), _nodes(std::move(nodes)),
        _materials(std::move(materials))
    {
        // TODO find neighbours, construct value arrays
    }

    const std::vector<EGS_Mesh::Tetrahedron>& elements() {
        return _elements;
    }
    const std::vector<EGS_Mesh::Node>& nodes() {
        return _nodes;
    }
    const std::vector<EGS_Mesh::Medium>& materials() {
        return _materials;
    }

    // EGS_BaseGeometry interface
    const std::string& getType() const { return type; }
    bool isInside(const EGS_Vector &x);
    int inside(const EGS_Vector &x);
    int isWhere(const EGS_Vector &x) { throw std::runtime_error("unimplemented!"); }
    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
        EGS_Float &t, int *newmed=0, EGS_Vector *normal=0) {
        throw std::runtime_error("unimplemented!");
    }
    EGS_Float hownear(int ireg, const EGS_Vector &x) {
        throw std::runtime_error("unimplemented!");
    }

private:
    std::vector<EGS_Mesh::Tetrahedron> _elements;
    std::vector<EGS_Mesh::Node> _nodes;
    std::vector<EGS_Mesh::Medium> _materials;
    const std::string type = "EGS_Mesh";
};

#endif // EGS_MESH
