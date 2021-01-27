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

#include "mesh_neighbours.h"
#include "msh_parser.h"

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

/// Parse the body of a msh4.1 file into an EGS_Mesh using the msh_parser API.
///
/// Throws a std::runtime_error if parsing fails.
EGS_Mesh parse_msh41_body(std::istream& input) {
    std::vector<msh_parser::internal::msh41::Node> nodes;
    std::vector<msh_parser::internal::msh41::MeshVolume> volumes;
    std::vector<msh_parser::internal::msh41::PhysicalGroup> groups;
    std::vector<msh_parser::internal::msh41::Tetrahedron> elements;

    std::string parse_err;
    std::string input_line;
    while (std::getline(input, input_line)) {
        msh_parser::internal::rtrim(input_line);
        // stop reading if we hit another mesh file
        if (input_line == "$MeshFormat") {
            break;
        }
        if (input_line == "$Entities") {
           volumes = msh_parser::internal::msh41::parse_entities(input);
        } else if (input_line == "$PhysicalNames") {
            groups = msh_parser::internal::msh41::parse_groups(input);
        } else if (input_line == "$Nodes") {
            nodes = msh_parser::internal::msh41::parse_nodes(input);
        } else if (input_line == "$Elements") {
            elements = msh_parser::internal::msh41::parse_elements(input);
        }
    }
    if (volumes.empty()) {
        throw std::runtime_error("No volumes were parsed");
    }
    if (nodes.empty()) {
        throw std::runtime_error("No nodes were parsed");
    }
    if (groups.empty()) {
        throw std::runtime_error("No groups were parsed");
    }
    if (elements.empty()) {
        throw std::runtime_error("No tetrahedrons were parsed");
    }

    // ensure each entity has a valid group
    std::unordered_set<int> group_tags;
    group_tags.reserve(groups.size());
    for (auto g: groups) {
        group_tags.insert(g.tag);
    }
    std::unordered_map<int, int> volume_groups;
    volume_groups.reserve(volumes.size());
    for (auto v: volumes) {
        if (group_tags.find(v.group) == group_tags.end()) {
            throw std::runtime_error("volume " + std::to_string(v.tag) + " had unknown physical group tag " + std::to_string(v.group));
        }
        volume_groups.insert({ v.tag, v.group });
    }

    // ensure each element has a valid entity and therefore a valid physical group
    std::vector<int> element_groups;
    element_groups.reserve(elements.size());
    for (auto e: elements) {
        auto elt_group = volume_groups.find(e.volume);
        if (elt_group == volume_groups.end()) {
            throw std::runtime_error("tetrahedron " + std::to_string(e.tag) + " had unknown volume tag " + std::to_string(e.volume));
        }
        element_groups.push_back(elt_group->second);
    }

    std::vector<EGS_Mesh::Tetrahedron> mesh_elts;
    mesh_elts.reserve(elements.size());
    for (std::size_t i = 0; i < elements.size(); ++i) {
        const auto& elt = elements[i];
        mesh_elts.push_back(EGS_Mesh::Tetrahedron(
            element_groups[i], elt.a, elt.b, elt.c, elt.d
        ));
    }

    std::vector<EGS_Mesh::Node> mesh_nodes;
    mesh_nodes.reserve(nodes.size());
    for (const auto& n: nodes) {
        mesh_nodes.push_back(EGS_Mesh::Node(
            n.tag, n.x, n.y, n.z
        ));
    }

    std::vector<EGS_Mesh::Medium> media;
    media.reserve(groups.size());
    for (const auto& g: groups) {
        media.push_back(EGS_Mesh::Medium(g.tag, g.name));
    }

    // TODO: check all 3d physical groups were used by elements
    // TODO: ensure all element node tags are valid
    return EGS_Mesh(mesh_elts, mesh_nodes, media);
}
} // anonymous namespace

// msh4.1 parsing
EGS_Mesh EGS_Mesh::parse_msh_file(std::istream& input) {
    auto version = msh_parser::internal::parse_msh_version(input);
    // TODO auto mesh_data;
    switch(version) {
        case msh_parser::internal::MshVersion::v41:
            try {
                return parse_msh41_body(input);
            } catch (const std::runtime_error& err) {
                throw std::runtime_error("msh 4.1 parsing failed\n" + std::string(err.what()));
            }
            break;
    }
    throw std::runtime_error("couldn't parse msh file");
}

EGS_Mesh::EGS_Mesh(std::vector<EGS_Mesh::Tetrahedron> elements,
    std::vector<EGS_Mesh::Node> nodes, std::vector<EGS_Mesh::Medium> materials) :
    _elements(std::move(elements)), _nodes(std::move(nodes)), _materials(std::move(materials))
{
    _elt_points.reserve(_elements.size() * 4);
    // Find the matching nodes for every tetrahedron
    auto find_node = [&](int node_tag) -> EGS_Mesh::Node {
        auto node_it = std::find_if(_nodes.begin(), _nodes.end(),
            [&](const EGS_Mesh::Node& n) { return n.tag == node_tag; });
        if (node_it == _nodes.end()) {
            throw std::runtime_error("No mesh node with tag: " + std::to_string(node_tag));
        }
        return *node_it;
    };
    for (const auto& e: _elements) {
        auto a = find_node(e.a);
        auto b = find_node(e.b);
        auto c = find_node(e.c);
        auto d = find_node(e.d);
        _elt_points.emplace_back(EGS_Vector(a.x, a.y, a.z));
        _elt_points.emplace_back(EGS_Vector(b.x, b.y, b.z));
        _elt_points.emplace_back(EGS_Vector(c.x, c.y, c.z));
        _elt_points.emplace_back(EGS_Vector(d.x, d.y, d.z));
    }

    std::vector<mesh_neighbours::Tetrahedron> neighbour_elts;
    neighbour_elts.reserve(elements.size());
    for (const auto& e: elements) {
        neighbour_elts.emplace_back(mesh_neighbours::Tetrahedron(e.a, e.b, e.c, e.d));
    }
    this->_neighbours = mesh_neighbours::tetrahedron_neighbours(neighbour_elts);

    // TODO figure out materials
    // TODO set EGS_BaseGeometry::nreg;
}

bool EGS_Mesh::isInside(const EGS_Vector &x) {
    return isWhere(x) != -1;
}

int EGS_Mesh::inside(const EGS_Vector &x) {
    return isInside(x) ? 0 : -1;
}

int EGS_Mesh::isWhere(const EGS_Vector &x) {
    for (std::size_t i = 0; i < num_elements(); i++) {
        const auto& A = _elt_points.at(4*i);
        const auto& B = _elt_points.at(4*i + 1);
        const auto& C = _elt_points.at(4*i + 2);
        const auto& D = _elt_points.at(4*i + 3);
        if (point_outside_of_plane(x, A, B, C, D)) {
            continue;
        }
        if (point_outside_of_plane(x, A, C, D, B)) {
            continue;
        }
        if (point_outside_of_plane(x, A, B, D, C)) {
            continue;
        }
        if (point_outside_of_plane(x, B, D, C, A)) {
            continue;
        }
        return i;
    }
    return -1;
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
