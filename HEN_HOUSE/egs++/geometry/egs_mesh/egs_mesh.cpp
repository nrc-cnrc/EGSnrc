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

#include <cassert>
#include <limits>
#include <unordered_map>

// anonymous namespace
namespace {

void print_egsvec(const EGS_Vector& v) {
    std::cout << "{\n  x: " << v.x << "\n  y: " << v.y << "\n  z: " << v.z << "\n}\n";
}

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

// Inputs:
// * particle position p,
// * normalized velocity v_norm
// * triangle points A, B, C (any ordering)
//
// Returns 1 if there is an intersection and 0 if not. If there is an intersection,
// the out parameter dist will be the distance along v_norm to the intersection point.
//
// Implementation of double-sided Möller-Trumbore ray-triangle intersection
// <http://www.graphics.cornell.edu/pubs/1997/MT97.pdf>
int triangle_ray_intersection(const EGS_Vector &p, const EGS_Vector &v_norm,
    const EGS_Vector& a, const EGS_Vector& b, const EGS_Vector& c, EGS_Float& dist)
{
    const EGS_Float eps = 1e-10;
    EGS_Vector ab = b - a;
    EGS_Vector ac = c - a;

    EGS_Vector pvec = cross(v_norm, ac);
    EGS_Float det = dot(ab, pvec);

    if (det > -eps && det < eps) {
        return 0;
    }
    EGS_Float inv_det = 1.0 / det;
    EGS_Vector tvec = p - a;
    EGS_Float u = dot(tvec, pvec) * inv_det;
    if (u < 0.0 || u > 1.0) {
        return 0;
    }
    EGS_Vector qvec = cross(tvec, ab);
    EGS_Float v = dot(v_norm, qvec) * inv_det;
    if (v < 0.0 || u + v > 1.0) {
        return 0;
    }
    // intersection found
    dist = dot(ac, qvec) * inv_det;
    return 1;
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
        throw std::runtime_error("No volumes were parsed from $Entities section");
    }
    if (nodes.empty()) {
        throw std::runtime_error("No nodes were parsed, missing $Nodes section");
    }
    if (groups.empty()) {
        throw std::runtime_error("No groups were parsed from $PhysicalNames section");
    }
    if (elements.empty()) {
        throw std::runtime_error("No tetrahedrons were parsed from $Elements section");
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
    std::size_t max_elts = std::numeric_limits<int>::max();
    if (_elements.size() >= max_elts) {
        throw std::runtime_error("maximum number of elements (" +
            std::to_string(max_elts) + ") exceeded (" + std::to_string(_elements.size()) + ")");
    }
    // TODO EGS_BaseGeometry::nreg = _elements.size();

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
    neighbour_elts.reserve(_elements.size());
    for (const auto& e: _elements) {
        neighbour_elts.emplace_back(mesh_neighbours::Tetrahedron(e.a, e.b, e.c, e.d));
    }
    this->_neighbours = mesh_neighbours::tetrahedron_neighbours(neighbour_elts);

    _is_boundary.reserve(_elements.size());
    for (const auto& ns: _neighbours) {
        _is_boundary.push_back(std::any_of(ns.begin(), ns.end(),
            [](std::size_t n) { return n == mesh_neighbours::NONE; })
        );
    }

    // TODO figure out materials setup (override setMedia?) with egsinp

    // map from medium tags to offsets
    std::unordered_map<int, int> medium_offsets;
    medium_offsets.reserve(_materials.size());
    for (std::size_t i = 0; i < _materials.size(); i++) {
        // TODO use EGS_BaseGeometry tracker
        // auto med = EGS_BaseGeometry::addMedium(m.medium_name);
        auto material_tag = _materials[i].tag;
        bool inserted = medium_offsets.insert({material_tag, i}).second;
        if (!inserted) {
            throw std::runtime_error("duplicate medium tag: " + std::to_string(material_tag));
        }
    }

    _medium_indices.reserve(_elements.size());
    for (const auto& e: _elements) {
        // TODO handle vacuum tag (-1)?
        _medium_indices.push_back(medium_offsets.at(e.medium_tag));
    }
}

bool EGS_Mesh::isInside(const EGS_Vector &x) {
    return isWhere(x) != -1;
}

int EGS_Mesh::inside(const EGS_Vector &x) {
    return isInside(x) ? 0 : -1;
}

int EGS_Mesh::medium(int ireg) const {
    return _medium_indices.at(ireg);
}

int EGS_Mesh::isWhere(const EGS_Vector &x) {
    for (auto i = 0; i < num_elements(); i++) {
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

EGS_Float EGS_Mesh::hownear(int ireg, const EGS_Vector& x) {
    if (ireg > 0 && ireg > num_elements() - 1) {
        throw std::runtime_error("ireg " + std::to_string(ireg) + " out of bounds for mesh with " + std::to_string(num_elements()) + " regions");
    }
    // inside
    if (ireg >= 0) {
        return min_interior_face_dist(ireg, x);
    }
    // outside
    return min_exterior_face_dist(ireg, x);
}

EGS_Float EGS_Mesh::min_interior_face_dist(int ireg, const EGS_Vector& x) {
    assert(ireg >= 0);
    EGS_Float min2 = std::numeric_limits<EGS_Float>::max();

    auto maybe_update_min = [&](const EGS_Vector& A, const EGS_Vector& B, const EGS_Vector& C) {
        EGS_Float dis = distance2(closest_point_triangle(x, A, B, C), x);
        if (dis < min2) {
            min2 = dis;
        }
    };

    const auto& A = _elt_points.at(4*ireg);
    const auto& B = _elt_points.at(4*ireg + 1);
    const auto& C = _elt_points.at(4*ireg + 2);
    const auto& D = _elt_points.at(4*ireg + 3);

    maybe_update_min(A, B, C);
    maybe_update_min(A, C, D);
    maybe_update_min(A, B, D);
    maybe_update_min(B, D, C);

    return std::sqrt(min2);
}

EGS_Float EGS_Mesh::min_exterior_face_dist(int ireg, const EGS_Vector& x) {
    assert(ireg < 0);
    // loop over all boundary tetrahedrons and find the closest point to the tetrahedron
    EGS_Float min2 = std::numeric_limits<EGS_Float>::max();
    for (auto i = 0; i < num_elements(); i++) {
        if (!_is_boundary[i]) {
            continue;
        }
        const auto& A = _elt_points.at(4*i);
        const auto& B = _elt_points.at(4*i + 1);
        const auto& C = _elt_points.at(4*i + 2);
        const auto& D = _elt_points.at(4*i + 3);
        EGS_Float dis = distance2(x, closest_point_tetrahedron(x, A, B, C, D));
        if (dis < min2) {
            min2 = dis;
        }
    }
    return std::sqrt(min2);
}

int EGS_Mesh::howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
    EGS_Float &t, int *newmed /* =0 */, EGS_Vector *normal /* =0 */)
{
    if (ireg < 0) {
        return howfar_exterior(ireg, x, u, t, newmed, normal);
    }
    return howfar_interior(ireg, x, u, t, newmed, normal);
}

int EGS_Mesh::howfar_interior(int ireg, const EGS_Vector &x, const EGS_Vector &u,
    EGS_Float &t, int *newmed, EGS_Vector *normal)
{
    assert(ireg >= 0 && ireg < num_elements());
    EGS_Vector u_norm = u;
    u_norm.normalize();

    const auto& A = _elt_points.at(4*ireg);
    const auto& B = _elt_points.at(4*ireg + 1);
    const auto& C = _elt_points.at(4*ireg + 2);
    const auto& D = _elt_points.at(4*ireg + 3);

    std::cout << "A "; print_egsvec(A);
    std::cout << "B "; print_egsvec(B);
    std::cout << "C "; print_egsvec(C);
    std::cout << "D "; print_egsvec(D);

    std::cout << "neighbour 0: " << _neighbours[ireg][0] << "\n";
    std::cout << "neighbour 1: " << _neighbours[ireg][1] << "\n";
    std::cout << "neighbour 2: " << _neighbours[ireg][2] << "\n";
    std::cout << "neighbour 3: " << _neighbours[ireg][3] << "\n";

    auto update_media_and_normal = [&](const EGS_Vector &A, const EGS_Vector &B,
        const EGS_Vector &C, int new_reg)
    {
        if (newmed) {
            if (new_reg == -1) {
                *newmed = -1; // vacuum
            } else {
                *newmed = medium(new_reg);
            }
        }
        if (normal) {
            EGS_Vector ab = B - A;
            EGS_Vector ac = C - A;
            *normal = cross(ab, ac);
        }
    };

    EGS_Float dist = 1e30;
    if (triangle_ray_intersection(x, u_norm, A, B, C, dist)) {
        // too far away to intersect
        if (dist > t) {
            return ireg;
        }
        t = dist;
        // index 3 = excluding last point D = face ABC
        auto new_reg = _neighbours[ireg][3];
        update_media_and_normal(A, B, C, new_reg);
        return new_reg;
    }
    if (triangle_ray_intersection(x, u_norm, A, C, D, dist)) {
        // too far away to intersect
        if (dist > t) {
            return ireg;
        }
        t = dist;
        // index 1 = excluding point B = face ACD
        auto new_reg = _neighbours[ireg][1];
        update_media_and_normal(A, C, D, new_reg);
        return new_reg;
    }
    if (triangle_ray_intersection(x, u_norm, A, B, D, dist)) {
        // too far away to intersect
        if (dist > t) {
            return ireg;
        }
        t = dist;
        // index 2 = excluding point C = face ABD
        auto new_reg = _neighbours[ireg][2];
        update_media_and_normal(A, B, D, new_reg);
        return new_reg;
    }
    if (triangle_ray_intersection(x, u_norm, B, C, D, dist)) {
        if (dist > t) {
            return ireg;
        }
        t = dist;
        // index 0 = excluding point A = face ACD
        auto new_reg = _neighbours[ireg][0];
        update_media_and_normal(B, C, D, new_reg);
        return new_reg;
    }

    return ireg;
}

int EGS_Mesh::howfar_exterior(int ireg, const EGS_Vector &x, const EGS_Vector &u,
    EGS_Float &t, int *newmed, EGS_Vector *normal)
{
    throw std::runtime_error("unimplemented!");
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
