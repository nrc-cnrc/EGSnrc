/*
###############################################################################
#
#  EGSnrc egs++ mesh geometry library implementation.
#
#  Copyright (C) 2020 Mevex Corporation
#
#  This file is part of EGSnrc.
#
#  Parts of this file, namely, the closest_point_triangle and
#  closest_point_tetrahedron functions, are adapted from Chapter 5 of
#  "Real-Time Collision Detection" by Christer Ericson with the consent
#  of the author and of the publisher.
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

// TODO
// #include "egs_input.h"
#include "egs_mesh.h"
#include "egs_vector.h"

#include <limits>

// anonymous namespace
namespace {

inline EGS_Float dot(const EGS_Vector &x, const EGS_Vector &y) {
    return x * y;
}

inline EGS_Vector cross(const EGS_Vector &x, const EGS_Vector &y) {
    return x.times(y);
}

inline EGS_Float distance2(const EGS_Vector &x, const EGS_Vector &y) {
    return (x - y).length2();
}

EGS_Vector closest_point_triangle(const EGS_Vector &P, const EGS_Vector &A, const EGS_Vector& B, const EGS_Vector& C)
{
    // vertex region A
    EGS_Vector ab = B - A;
    EGS_Vector ac = C - A;
    EGS_Vector ao = P - A;

    EGS_Float d1 = dot(ab, ao);
    EGS_Float d2 = dot(ac, ao);
    if (d1 <= 0.0 && d2 <= 0.0)
        return A;

    // vertex region B
    EGS_Vector bo = P - B;
    EGS_Float d3 = dot(ab, bo);
    EGS_Float d4 = dot(ac, bo);
    if (d3 >= 0.0 && d4 <= d3)
        return B;

    // edge region AB
    EGS_Float vc = d1 * d4 - d3 * d2;
    if (vc <= 0.0 && d1 >= 0.0 && d3 <= 0.0) {
        EGS_Float v = d1 / (d1 - d3);
        return A + v * ab;
    }

    // vertex region C
    EGS_Vector co = P - C;
    EGS_Float d5 = dot(ab, co);
    EGS_Float d6 = dot(ac, co);
    if (d6 >= 0.0 && d5 <= d6)
        return C;

    // edge region AC
    EGS_Float vb = d5 * d2 - d1 * d6;
    if (vb <= 0.0 && d2 >= 0.0 && d6 <= 0.0) {
        EGS_Float w = d2 / (d2 - d6);
        return A + w * ac;
    }

    // edge region BC
    EGS_Float va = d3 * d6 - d5 * d4;
    if (va <= 0.0 && (d4 - d3) >= 0.0 && (d5 - d6) >= 0.0) {
        EGS_Float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        return B + w * (C - B);
    }

    // inside the face
    EGS_Float denom = 1.0 / (va + vb + vc);
    EGS_Float v = vb * denom;
    EGS_Float w = vc * denom;
    return A + v * ab + w * ac;
}

inline bool point_outside_of_plane(EGS_Vector P, EGS_Vector A, EGS_Vector B, EGS_Vector C, EGS_Vector D) {
    return dot(P - A, cross(B - A, C - A)) * dot(D - A, cross(B - A, C - A)) < 0.0;
}

EGS_Vector closest_point_tetrahedron(const EGS_Vector &P, const EGS_Vector &A, const EGS_Vector &B, const EGS_Vector &C, const EGS_Vector &D)
{
    EGS_Vector min_point = P;
    EGS_Float min = std::numeric_limits<EGS_Float>::max();

    auto maybe_update_min_point = [&](const EGS_Vector& A, const EGS_Vector& B, const EGS_Vector& C) {
        EGS_Vector q = closest_point_triangle(P, A, B, C);
        EGS_Float dis = distance2(q, P);
        if (dis < min) {
            min = dis;
            min_point = q;
        }
    };

    if (point_outside_of_plane(P, A, B, C, D)) {
        maybe_update_min_point(A, B, C);
    }

    if (point_outside_of_plane(P, A, C, D, B)) {
        maybe_update_min_point(A, C, D);
    }

    if (point_outside_of_plane(P, A, B, D, C)) {
        maybe_update_min_point(A, B, D);
    }
    if (point_outside_of_plane(P, B, D, C, A)) {
        maybe_update_min_point(B, D, C);
    }

    return min_point;
}

} // anonymous namespace

bool EGS_Mesh::isInside(const EGS_Vector &x) {
    return false;
}

int EGS_Mesh::inside(const EGS_Vector &x) {
    return isInside(x) ? 0 : -1;
}

// TODO
/*
static char EGS_MESH_LOCAL geom_class_msg[] = "createGeometry(Mesh): %s\n";

extern "C" {
    EGS_MESH_EXPORT EGS_BaseGeometry *createGeometry(EGS_Input *input) {
        if (!input) {
            egsWarning(geom_class_msg, "null input");
            return nullptr;
        }
        std::string mesh_file;
        int err = input->getInput("file", mesh_file);
        if (err) {
            egsWarning(geom_class_msg, "no mesh file specified in input");
            return nullptr;
        }
        if (mesh_file.length() >= 4 && mesh_file.rfind(".msh") == mesh_file.length() - 4)
        {
            // std::ifstream input(mesh_file);
            // EGS_Mesh *mesh = new parse_msh_file(input);
            EGS_Mesh *mesh = nullptr;
            if (!mesh) {
                egsWarning("EGS_Mesh::from_file: Gmsh msh file parsing failed\n");
                return nullptr;
            }
            return mesh;
        }
        egsWarning("EGS_Mesh::from_file: unknown file extension for file `%s`,"
            "only `.msh` is allowed\n", mesh_file.c_str());
        return nullptr;
    }
}
*/
