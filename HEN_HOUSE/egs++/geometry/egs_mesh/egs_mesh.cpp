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
#include "egs_input.h"
#include "egs_mesh.h"
#include "egs_vector.h"

#include "mesh_neighbours.h"
#include "msh_parser.h"

#include <cassert>
#include <chrono>
#include <deque>
#include <limits>
#include <unordered_map>

// anonymous namespace
namespace {

const EGS_Float eps = 1e-8;

inline bool approx_eq(double a, double b, double e = eps) {
    return (std::abs(a - b) <= e * (std::abs(a) + std::abs(b) + 1.0));
}

inline bool is_zero(const EGS_Vector &v) {
    return approx_eq(0.0, v.length(), eps);
}

inline EGS_Float min3(EGS_Float a, EGS_Float b, EGS_Float c) {
    return std::min(std::min(a, b), c);
}

inline EGS_Float max3(EGS_Float a, EGS_Float b, EGS_Float c) {
    return std::max(std::max(a, b), c);
}

void print_egsvec(const EGS_Vector& v, std::ostream& out = std::cout) {
    out << std::setprecision(std::numeric_limits<double>::max_digits10) <<
    "{\n  x: " << v.x << "\n  y: " << v.y << "\n  z: " << v.z << "\n}\n";
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

// Returns true if the point is on the outside of the plane defined by ABC using
// reference point D, i.e. if D and P are on opposite sides of the plane of ABC.
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
    if (dist < eps) {
        return 0;
    }
    return 1;
}

EGS_Vector centroid(const EGS_Vector& a, const EGS_Vector& b, const EGS_Vector& c,
    const EGS_Vector& d)
{
    return EGS_Vector(
        (a.x + b.x + c.x + d.x) / 4.0,
        (a.y + b.y + c.y + d.y) / 4.0,
        (a.z + b.z + c.z + d.z) / 4.0
    );
}

/// Parse the body of a msh4.1 file into an EGS_Mesh using the msh_parser API.
///
/// Throws a std::runtime_error if parsing fails.
EGS_Mesh* parse_msh41_body(std::istream& input) {
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
            elt.tag, element_groups[i], elt.a, elt.b, elt.c, elt.d
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
    return new EGS_Mesh(mesh_elts, mesh_nodes, media);
}
} // anonymous namespace

class EGS_Mesh_Octree {
public:
    struct Tet {
        Tet(int offset, EGS_Vector a, EGS_Vector b, EGS_Vector c, EGS_Vector d)
            : offset(offset), a(a), b(b), c(c), d(d) {}
        int offset = -1;
        EGS_Vector a;
        EGS_Vector b;
        EGS_Vector c;
        EGS_Vector d;
        EGS_Vector centroid() const {
            return ::centroid(a, b, c, d);
        }
    };
private:
    static double tet_min_x(const Tet &t) {
        return std::min(t.a.x, std::min(t.b.x, std::min(t.c.x, t.d.x)));
    }
    static double tet_max_x(const Tet &t) {
        return std::max(t.a.x, std::max(t.b.x, std::max(t.c.x, t.d.x)));
    }
    static double tet_min_y(const Tet &t) {
        return std::min(t.a.y, std::min(t.b.y, std::min(t.c.y, t.d.y)));
    }
    static double tet_max_y(const Tet &t) {
        return std::max(t.a.y, std::max(t.b.y, std::max(t.c.y, t.d.y)));
    }
    static double tet_min_z(const Tet &t) {
        return std::min(t.a.z, std::min(t.b.z, std::min(t.c.z, t.d.z)));
    }
    static double tet_max_z(const Tet &t) {
        return std::max(t.a.z, std::max(t.b.z, std::max(t.c.z, t.d.z)));
    }
    static double tet_longest_edge2(const Tet &t) {
        return std::max(distance2(t.a, t.b), std::max(distance2(t.a, t.c),
                   std::max(distance2(t.a, t.d), std::max(distance2(t.b, t.c),
                       std::max(distance2(t.b, t.d), distance2(t.c, t.d))))));
    }

    // An axis-aligned bounding box.
    struct BoundingBox {
        double min_x = 0.0;
        double max_x = 0.0;
        double min_y = 0.0;
        double max_y = 0.0;
        double min_z = 0.0;
        double max_z = 0.0;
        BoundingBox() = default;
        BoundingBox(double min_x, double max_x, double min_y, double max_y,
            double min_z, double max_z) : min_x(min_x), max_x(max_x),
                min_y(min_y), max_y(max_y), min_z(min_z), max_z(max_z) {}
        double mid_x() const {
            return (min_x + max_x) / 2.0;
        }
        double mid_y() const {
            return (min_y + max_y) / 2.0;
        }
        double mid_z() const {
            return (min_z + max_z) / 2.0;
        }
        void expand(double delta) {
            min_x -= delta;
            min_y -= delta;
            min_z -= delta;
            max_x += delta;
            max_y += delta;
            max_z += delta;
        }
        void print(std::ostream& out = std::cout) const {
            std::cout <<
                std::setprecision(std::numeric_limits<double>::max_digits10) <<
                    "min_x: " << min_x << "\n";
            std::cout <<
                std::setprecision(std::numeric_limits<double>::max_digits10) <<
                    "max_x: " << max_x << "\n";
            std::cout <<
                std::setprecision(std::numeric_limits<double>::max_digits10) <<
                    "min_y: " << min_y << "\n";
            std::cout <<
                std::setprecision(std::numeric_limits<double>::max_digits10) <<
                    "max_y: " << max_y << "\n";
            std::cout <<
                std::setprecision(std::numeric_limits<double>::max_digits10) <<
                    "min_z: " << min_z << "\n";
            std::cout <<
                std::setprecision(std::numeric_limits<double>::max_digits10) <<
                    "max_z: " << max_z << "\n";
        }

        // Adapted from Ericson section 5.2.9 "Testing AABB Against Triangle".
        // Uses a separating axis approach, as originally presented in Akenine-
        // Möller's "Fast 3D Triangle-Box Overlap Testing" with 13 axes checked
        // in total. There are three axis categories, and it is suggested the
        // fastest way to check is 3, 1, 2.
        //
        // We use a more straightforward but less optimized formulation of the
        // separating axis test than Ericson presents, because this test is
        // intended to be done as part of the octree setup but not during the
        // actual simulation.
        //
        // This routine should be robust for ray edges parallel with bounding
        // box edges (category 3) but does not attempt to be robust for the case
        // of degenerate triangle face normals (category 2). See Ericson 5.2.1.1
        //
        // The non-robustness of some cases should not be an issue for the most
        // part as these will likely be false positives (harmless extra checks)
        // instead of false negatives (missed intersections, a huge problem if
        // present).
        bool intersects_triangle(const EGS_Vector& a, const EGS_Vector& b,
            const EGS_Vector& c) const
        {
            if (min3(a.x, b.x, c.x) >= max_x ||
                min3(a.y, b.y, c.y) >= max_y ||
                min3(a.z, b.z, c.z) >= max_z ||
                max3(a.x, b.x, c.x) <= min_x ||
                max3(a.y, b.y, c.y) <= min_y ||
                max3(a.z, b.z, c.z) <= min_z)
            {
                return false;
            }

            EGS_Vector centre(mid_x(), mid_y(), mid_z());
            // extents
            EGS_Float ex = (max_x - min_x) / 2.0;
            EGS_Float ey = (max_y - min_y) / 2.0;
            EGS_Float ez = (max_z - min_z) / 2.0;
            //std::cout << "extents : " << ex << " " << ey << " " << ez << "\n";

            // move triangle to bounding box origin
            EGS_Vector v0 = a - centre;
            EGS_Vector v1 = b - centre;
            EGS_Vector v2 = c - centre;

            // find triangle edge vectors
            const std::array<EGS_Vector, 3> edge_vecs { v1-v0, v2-v1, v0-v2 };

            // Test the 9 category 3 axes (cross products between axis-aligned
            // bounding box unit vectors and triangle edge vectors)
            const EGS_Vector ux {1, 0 ,0}, uy {0, 1, 0}, uz {0, 0, 1};
            const std::array<EGS_Vector, 3> unit_vecs { ux, uy, uz};
            for (const EGS_Vector& u : unit_vecs) {
                for (const EGS_Vector& f : edge_vecs) {
                    const EGS_Vector a = cross(u, f);
                    if (is_zero(a)) {
                        //std::cout << "warning a near zero\n";
                        // Ignore testing this axis, likely won't be a separating
                        // axis. This may lead to false positives, but not false
                        // negatives.
                        continue;
                    }
                    // find box projection radius
                    const EGS_Float r = ex * std::abs(dot(ux, a)) +
                        ey * std::abs(dot(uy, a)) + ez * std::abs(dot(uz, a));
                    // find three projections onto axis a
                    const EGS_Float p0 = dot(v0, a);
                    const EGS_Float p1 = dot(v1, a);
                    const EGS_Float p2 = dot(v2, a);
                    if (std::max(-max3(p0, p1, p2), min3(p0, p1, p2)) + eps > r) {
                        //std::cout << "found separating axis\n";
             //           print_egsvec(a);
                        return false;
                    }
                }
            }
            //std::cout << "passed 9 edge-edge axis tests\n";
            // category 1 - test overlap with AABB face normals
            if (max3(v0.x, v1.x, v2.x) <= -ex || min3(v0.x, v1.x, v2.x) >= ex ||
                max3(v0.y, v1.y, v2.y) <= -ey || min3(v0.y, v1.y, v2.y) >= ey ||
                max3(v0.z, v1.z, v2.z) <= -ez || min3(v0.z, v1.z, v2.z) >= ez)
            {
                return false;
            }
            //std::cout << "passed 3 overlap tests\n";

            // category 2 - test overlap with triangle face normal using AABB
            // plane test (5.2.3)

            // Cross product robustness issues are ignored here (assume
            // non-degenerate and non-oversize triangles)
            const EGS_Vector n = cross(edge_vecs[0], edge_vecs[1]);
            //std::cout << "plane normal: \n";
            //print_egsvec(n);
            if (is_zero(n)) {
                std::cout << "n near zero!\n";
            }
            // projection radius
            const EGS_Float r = ex * std::abs(n.x) + ey * std::abs(n.y) +
                ez * std::abs(n.z);
            // distance from box centre to plane
            //
            // We have to use `a` here and not `v0` as in my printing since the
            // bounding box was not translated to the origin. This is a known
            // erratum, see http://realtimecollisiondetection.net/books/rtcd/errata/
            const EGS_Float s = dot(n, centre) - dot(n, a);
            // intersection if s falls within projection radius
            return std::abs(s) <= r;
        }

        bool intersects_tetrahedron(const EGS_Mesh_Octree::Tet& tet) const {
            return intersects_triangle(tet.a, tet.b, tet.c) ||
                   intersects_triangle(tet.a, tet.c, tet.d) ||
                   intersects_triangle(tet.a, tet.b, tet.d) ||
                   intersects_triangle(tet.b, tet.c, tet.d);
        }

        // Adapted from Ericson section 5.3.3 "Intersecting Ray or Segment
        // Against Box".
        //
        // Returns 1 if there is an intersection and 0 if not. If there is an
        // intersection, the out parameter dist will be the distance along v to
        // the intersection point q.
        int ray_intersection(const EGS_Vector &p, const EGS_Vector &v,
            EGS_Float& dist, EGS_Vector &q) const
        {
            // check intersection of ray with three bounding box slabs
            EGS_Float tmin = 0.0;
            EGS_Float tmax = std::numeric_limits<EGS_Float>::max();
            std::array<EGS_Float, 3> p_vec {p.x, p.y, p.z};
            std::array<EGS_Float, 3> v_vec {v.x, v.y, v.z};
            std::array<EGS_Float, 3> mins {min_x, min_y, min_z};
            std::array<EGS_Float, 3> maxs {max_x, max_y, max_z};
            for (std::size_t i = 0; i < 3; i++) {
                // Parallel to slab. Point must be within slab bounds to hit
                // the bounding box
                if (std::abs(v_vec[i]) < eps) {
                    // Outside slab bounds
                    if (p_vec[i] < mins[i] || p_vec[i] > maxs[i]) { return 0; }
                } else {
                    // intersect ray with slab planes
                    EGS_Float inv_vel = 1.0 / v_vec[i];
                    EGS_Float t1 = (mins[i] - p_vec[i]) * inv_vel;
                    EGS_Float t2 = (maxs[i] - p_vec[i]) * inv_vel;
                    // convention is t1 is near plane, t2 is far plane
                    if (t1 > t2) { std::swap(t1, t2); }
                    tmin = std::max(tmin, t1);
                    tmax = std::min(tmax, t2);
                    if (tmin > tmax) { return 0; }
                }
            }
            q = p + v * tmin;
            dist = tmin;
            return 1;
        }

        // Returns the closest point on the bounding box to the given point.
        // If the given point is inside the bounding box, it is considered the
        // closest point. This method is intended for use by hownear, to decide
        // where to search first.
        //
        // See section 5.1.3 Ericson.
        EGS_Vector closest_point(const EGS_Vector& point) const {
            std::array<EGS_Float, 3> p = {point.x, point.y, point.z};
            std::array<EGS_Float, 3> mins = {min_x, min_y, min_z};
            std::array<EGS_Float, 3> maxs = {max_x, max_y, max_z};
            // set q to p, then clamp it to min/max bounds as needed
            std::array<EGS_Float, 3> q = p;
            for (int i = 0; i < 3; i++) {
                if (p[i] < mins[i]) {
                    q[i] = mins[i];
                }
                if (p[i] > maxs[i]) {
                    q[i] = maxs[i];
                }
            }
            return EGS_Vector(q[0], q[1], q[2]);
        }

        bool contains(const EGS_Vector& point) const {
            // Inclusive at the lower bound, non-inclusive at the upper bound,
            // so points on the interface between two bounding boxes only belong
            // to one of them:
            //
            //  +---+---+
            //  |   x   |
            //  +---+---+
            //        ^ belongs here
            //
            return point.x >= min_x && point.x < max_x &&
                   point.y >= min_y && point.y < max_y &&
                   point.z >= min_z && point.z < max_z;
        }

        bool contains(const Tet& tet) const {
            // following Furuta et al, "Implementation of tetrahedral-mesh
            // geometry in Monte Carlo radiation transport code PHITS",
            // we consider a tetrahedron inside a bounding box if any of the
            // points or the centre of mass are inside
            // TODO: maybe consider midpoint of edge as well?
            return contains(tet.a) || contains(tet.b) || contains(tet.c) ||
                   contains(tet.d) || contains(tet.centroid());
        }

        bool is_indivisible() const {
            // check if we're running up against precision limits
            return approx_eq(min_x, mid_x()) ||
                approx_eq(max_x, mid_x()) ||
                approx_eq(min_y, mid_y()) ||
                approx_eq(max_y, mid_y()) ||
                approx_eq(min_z, mid_z()) ||
                approx_eq(max_z, mid_z());

        }

        // Split into 8 equal octants. Octant numbering follows an S, i.e:
        //
        //        -z         +z
        //     +---+---+  +---+---+
        //     | 2 | 3 |  | 6 | 7 |
        //  y  +---+---+  +---+---+
        //  ^  | 0 | 1 |  | 4 | 5 |
        //  |  +---+---+  +---+---+
        //  + -- > x
        //
        std::array<BoundingBox, 8> divide8() const {
            return {
                BoundingBox (
                    min_x, mid_x(),
                    min_y, mid_y(),
                    min_z, mid_z()
                ),
                BoundingBox(
                    mid_x(), max_x,
                    min_y, mid_y(),
                    min_z, mid_z()
                ),
                BoundingBox(
                    min_x, mid_x(),
                    mid_y(), max_y,
                    min_z, mid_z()
                ),
                BoundingBox(
                    mid_x(), max_x,
                    mid_y(), max_y,
                    min_z, mid_z()
                ),
                BoundingBox(
                    min_x, mid_x(),
                    min_y, mid_y(),
                    mid_z(), max_z
                ),
                BoundingBox(
                    mid_x(), max_x,
                    min_y, mid_y(),
                    mid_z(), max_z
                ),
                BoundingBox(
                    min_x, mid_x(),
                    mid_y(), max_y,
                    mid_z(), max_z
                ),
                BoundingBox(
                    mid_x(), max_x,
                    mid_y(), max_y,
                    mid_z(), max_z
                )
            };
        }
    };
    struct Node {
        std::vector<int> elts_;
        std::vector<Node> children_;
        BoundingBox bbox_;
        // Knowing the longest edge gives us a good stopping criteria when a
        // point in an overlapping element without nodes in the octree, e.g.
        //
        //     +---+---+
        //     |   |   | /|
        //     +---+---+/ |
        //     |   |   /  |
        //     +---+--/+  |
        //           /    |
        //           ------
        // If the lookup fails, we will keep trying until we have searched a
        // bounding box of at least "longest_edge" around the query point.
        static EGS_Float longest_edge;
        static BoundingBox global_bounds;

        Node() = default;
        // TODO: think about passing in EGS_Mesh as parameter
        Node(const std::vector<Tet> &elts, const BoundingBox& bbox,
            std::size_t n_max) : bbox_(bbox)
        {
            // TODO: max level and precision warning
            if (bbox_.is_indivisible() || elts.size() < n_max) {
                elts_.reserve(elts.size());
                for (const auto &e : elts) {
                    elts_.push_back(e.offset);
                }
                return;
            }

            std::array<std::vector<Tet>, 8> octants;
            std::array<BoundingBox, 8> bbs = bbox_.divide8();

            // elements may be in more than one bounding box
            for (const auto &e : elts) {
                for (int i = 0; i < 8; i++) {
                    if (bbs[i].intersects_tetrahedron(e)) {
                        octants[i].push_back(e);
                    }
                }
            }
            for (int i = 0; i < 8; i++) {
                children_.push_back(Node(
                    std::move(octants[i]), std::move(bbs[i]), n_max
                ));
            }
        }

        bool isLeaf() const {
            return children_.empty();
        }

        void print(std::ostream& out, int level) const {
            out << "Level " << level << "\n";
            bbox_.print(out);
            if (children_.empty()) {
            out << "num_elts: " << elts_.size() << "\n";
                for (const auto& e: elts_) {
                    out << e << " ";
                }
                out << "\n";
                return;
            }
            for (int i = 0; i < 8; i++) {
                children_.at(i).print(out, level + 1);
            }
        }


        // Check if this node's bounding box is guaranteed to hold the largest
        // possible tetrahedron in any orientation.
        //
        // TODO: test if this method actually works
        bool containsLargestTetrahedron(const EGS_Vector& p) const {
            // Float comparison safety: global bounds should be equal to bounding
            // boxes on the edge as they are assigned directly.
            //
            // Min bounds: local min is larger than global min and there
            // could be an edge that extends past the local min
            if ((bbox_.min_x > Node::global_bounds.min_x &&
                    bbox_.min_x > (p.x - Node::longest_edge)) ||
                (bbox_.min_y > Node::global_bounds.min_y &&
                    bbox_.min_y > (p.y - Node::longest_edge)) ||
                (bbox_.min_z > Node::global_bounds.min_z &&
                    bbox_.min_z > (p.z - Node::longest_edge)) ||
                // Max bounds: local max is smaller than global max and there
                // could be an edge that extends past the local max
                (bbox_.max_x < Node::global_bounds.max_x &&
                    bbox_.max_x < (p.x + Node::longest_edge)) ||
                (bbox_.max_y < Node::global_bounds.max_y &&
                    bbox_.max_y < (p.y + Node::longest_edge)) ||
                (bbox_.max_z < Node::global_bounds.max_z &&
                    bbox_.max_z < (p.z + Node::longest_edge)))
            {
                return false;
            }
            return true;
        }

        int findOctant(const EGS_Vector &p) const {
            // Our choice of octant ordering (see BoundingBox.divide8) means we
            // can determine the correct octant with three checks. E.g. octant 0
            // is (-x, -y, -z), octant 1 is (+x, -y, -z), octant 4 is (-x, -y, +z)
            // octant 7 is (+x, +y, +z), etc.
            std::size_t octant = 0;
            if (p.x >= bbox_.mid_x()) { octant += 1; };
            if (p.y >= bbox_.mid_y()) { octant += 2; };
            if (p.z >= bbox_.mid_z()) { octant += 4; };
            return octant;
        }

        // Octants are returned ordered by minimum intersection distance
        std::vector<int> findOtherIntersectedOctants(const EGS_Vector& p,
                const EGS_Vector& v, int exclude_octant) const
        {
            if (isLeaf()) {
                throw std::runtime_error(
                    "findOtherIntersectedOctants called on leaf node");
            }
            std::vector<std::pair<EGS_Float, int>> intersections;
            for (int i = 0; i < 8; i++) {
                if (i == exclude_octant) {
                    continue;
                }
                EGS_Vector intersection;
                EGS_Float dist;
                if (children_[i].bbox_.ray_intersection(p, v, dist, intersection)) {
                    intersections.push_back({dist, i});
                }
            }
            std::sort(intersections.begin(), intersections.end());
            std::vector<int> octants;
            for (const auto& i : intersections) {
                octants.push_back(i.second);
            }
            return octants;
        }

        // Given the most likely octant, return the remaining octants ordered by
        // minimum distance to the query point.
        std::array<int, 7> nextClosestOctants(const EGS_Vector& p,
            int exclude_octant) const
        {
            if (isLeaf()) {
                throw std::runtime_error(
                    "nextClosestOctants called on leaf node");
            }
            std::vector<std::pair<EGS_Float, int>> octant_distances;
            for (int i = 0; i < 8; i++) {
                if (i == exclude_octant) {
                    continue;
                }
                auto closest_point = children_[i].bbox_.closest_point(p);
                octant_distances.push_back({distance2(p, closest_point), i});
            }
            std::sort(octant_distances.begin(), octant_distances.end());
            std::array<int, 7> octants;
            for (int i = 0; i < 7; i++) {
                octants[i] = octant_distances[i].second;
            }
            return octants;
        }

        struct HownearResult {
            int elt = -1;
            // store squared distance to avoid square roots until the end
            EGS_Float sq_dist = veryFar;
        };

        // Leaf node: search all bounded elements, updating dist with the
        // closest distance.
        HownearResult hownear_leaf_search(const EGS_Vector& p,
            const EGS_Float& best_sq_dist, EGS_Mesh& mesh) const
        {
            HownearResult res;
            res.sq_dist = best_sq_dist;
            for (const auto &e: elts_) {
                const auto& n = mesh.element_nodes(e);
                auto dist = distance2(p,
                    closest_point_tetrahedron(p, n.A, n.B, n.C, n.D));
                if (dist < res.sq_dist) {
                    res.elt = e;
                    res.sq_dist = dist;
                }
            }
            return res;
        }

        HownearResult hownear_exterior(const EGS_Vector& p,
            const EGS_Float& best_sq_dist, EGS_Mesh& mesh) const
        {
            // Determine the closest point on this bounding box. If this is a
            // parent node, this will determine where the search starts.
            auto closest_point = bbox_.closest_point(p);
            // Stop early: if the closest point on this octant's bounding box is
            // farther away than the best distance so far, we can skip searching
            // this octant.
            if (distance2(p, closest_point) > best_sq_dist) {
                return {};
            }
            // Leaf node: search all bounded elements, updating dist with the
            // closest distance
            if (isLeaf()) {
                return hownear_leaf_search(p, best_sq_dist, mesh);
            }
            // Parent node: decide which octant to search first and begin there
            auto octant = findOctant(closest_point);
            // Once we find a result, we still have to keep searching until we
            // know this must be the closest element
            auto res = children_[octant].hownear_exterior(p, best_sq_dist, mesh);
            for (const auto& o : nextClosestOctants(p, octant)) {
                // skip search if we know beforehand we can't beat the best
                // distance
                if (distance2(p, children_[o].bbox_.closest_point(p))
                       > res.sq_dist)
                {
                    continue;
                }
                auto other = children_[o].hownear_exterior(p, res.sq_dist, mesh);
                if (other.sq_dist < res.sq_dist) {
                    res = other;
                }
            }
            return res;
        }

        // TODO remove containsLargestTetrahedron check
        // Does not mutate the EGS_Mesh.
        int isWhere(const EGS_Vector &p, /*const*/ EGS_Mesh &mesh) const {
            // Leaf node: search all bounded elements, returning -1 if the
            // element wasn't found.
            if (isLeaf()) {
                for (const auto &e: elts_) {
                    if (mesh.insideElement(e, p)) {
                        return e;
                    }
                }
                return -1;
            }

            // Parent node: decide which octant to search and descend the tree
            auto octant = findOctant(p);
            auto elt = children_[octant].isWhere(p, mesh);
            // If we find a valid element, we can conclude it is the correct
            // solution since elements are assumed not to overlap.
            if (elt != -1) {
                return elt;
            }

            // If the element wasn't found, it is possible the point is in an
            // overlapping element in a nearby octant. Give up if we can
            // conclude that knowing the longest possible edge, we've searched
            // a suitably sized bounding box around the query point.
            //
            // Otherwise begin a fallback search through the siblings.
            if (children_[octant].containsLargestTetrahedron(p)) {
                return -1;
            }
            for (std::size_t i = 0; i < 8; i++) {
                if (i == octant) { continue; }
                auto elt = children_[i].isWhere(p, mesh);
                if (elt != -1) {
                    return elt;
                }
            }
            return -1;
        }

        // TODO split into two functions
        int howfar_exterior(const EGS_Vector &p, const EGS_Vector &v,
            const EGS_Float &max_dist, EGS_Float &t, /* const */ EGS_Mesh& mesh)
            const
        {
            // Leaf node: check for intersection with any boundary elements
            EGS_Float min_dist = std::numeric_limits<EGS_Float>::max();
            int min_elt = -1;
            if (isLeaf()) {
                for (const auto &e: elts_) {
                    if (!mesh.is_boundary(e)) {
                        continue;
                    }
                    // closest_boundary_face only counts intersections where the
                    // point is on the outside of the face, when it's possible
                    // to intersect the boundary face directly
                    auto intersection = mesh.closest_boundary_face(e, p, v);
                    if (intersection.dist < min_dist) {
                        min_elt = e;
                        min_dist = intersection.dist;
                    }
                }
                t = min_dist;
                return min_elt; // min_elt may be -1 if there is no intersection
            }
            // Parent node: decide which octant to search and descend the tree
            EGS_Vector intersection;
            EGS_Float dist;
            auto hit = bbox_.ray_intersection(p, v, dist, intersection);
            // case 1: there's no intersection with this bounding box, return
            if (!hit) {
                return -1;
            }
            // case 2: we have a hit. Descend into the most likely intersecting
            // child octant's bounding box to find any intersecting elements
            auto octant = findOctant(intersection);
            auto elt = children_[octant].howfar_exterior(
                p, v, max_dist, t, mesh
            );
            // If we find a valid element, return it
            if (elt != -1) {
                return elt;
            }
            // Otherwise, if there was no intersection in the most likely
            // octant, examine the other octants that are intersected by
            // the ray:
            for (const auto& o : findOtherIntersectedOctants(p, v, octant)) {
                auto elt = children_[o].howfar_exterior(
                    p, v, max_dist, t, mesh
                );
                // If we find a valid element, return it
                if (elt != -1) {
                    return elt;
                }
            }
            return -1;
        }
    };

    Node root_;
public:

    // Must be called before creating and using any EGS_Mesh_Octrees
    // TODO: maybe a nicer way to do this? Check core guidlines?
    static void initStaticMembers(const std::vector<Tet> &elts) {
        if (elts.empty()) {
            throw std::runtime_error("EGS_Mesh_Octree: empty elements vector");
        }
        const EGS_Float INF = std::numeric_limits<EGS_Float>::infinity();
        BoundingBox b(INF, -INF, INF, -INF, INF, -INF);
        EGS_Float longest_edge2 = -INF;
        for (const auto& e : elts) {
            b.min_x = std::min(b.min_x, tet_min_x(e));
            b.max_x = std::max(b.max_x, tet_max_x(e));
            b.min_y = std::min(b.min_y, tet_min_y(e));
            b.max_y = std::max(b.max_y, tet_max_y(e));
            b.min_z = std::min(b.min_z, tet_min_z(e));
            b.max_z = std::max(b.max_z, tet_max_z(e));
            longest_edge2 = std::max(longest_edge2, tet_longest_edge2(e));
        }
        // Add a small delta around the bounding box to avoid numerical problems
        // at the boundary
        b.expand(1e-8);
        Node::longest_edge = std::sqrt(longest_edge2);
        Node::global_bounds = b;
    }

    EGS_Mesh_Octree() = default;
    EGS_Mesh_Octree(const std::vector<Tet> &elts, std::size_t n_max) {
        if (elts.empty()) {
            throw std::runtime_error("EGS_Mesh_Octree: empty elements vector");
        }
        std::cout << "making octree of " << elts.size() << " elements\n";
        if (elts.size() > std::numeric_limits<int>::max()) {
            throw std::runtime_error("EGS_Mesh_Octree: num elts must fit into an int");
        }
        // TODO: add check that initStaticMembers was called?
        root_ = Node(elts, Node::global_bounds, n_max);
    }

    int isWhere(const EGS_Vector& p, /*const*/ EGS_Mesh& mesh) const {
        if (!root_.bbox_.contains(p)) {
            return -1;
        }
        return root_.isWhere(p, mesh);
    }

    void print(std::ostream& out) const {
		root_.print(out, 0);
    }

    int howfar_exterior(const EGS_Vector &p, const EGS_Vector &v,
        const EGS_Float &max_dist, EGS_Float &t, EGS_Mesh& mesh) const
    {
        EGS_Vector intersection;
        EGS_Float dist;
        auto hit = root_.bbox_.ray_intersection(p, v, dist, intersection);
        if (!hit || dist > max_dist) {
            return -1;
        }
        return root_.howfar_exterior(p, v, max_dist, t, mesh);
    }

    // Returns the closest exterior element to the given point and updates the
    // distance in the out parameter dist.
    int hownear_exterior(const EGS_Vector& p, EGS_Float& dist, EGS_Mesh& mesh)
        const
    {
        EGS_Float best_dist = std::numeric_limits<EGS_Float>::max();
        Node::HownearResult res = root_.hownear_exterior(p, best_dist, mesh);
        dist = std::sqrt(res.sq_dist);
        return res.elt;
    }
};

EGS_Float EGS_Mesh_Octree::Node::longest_edge = 0.0;
EGS_Mesh_Octree::BoundingBox EGS_Mesh_Octree::Node::global_bounds = {};

// msh4.1 parsing
//
// TODO parse into MeshSpec struct instead of EGS_Mesh directly
// to better delineate errors
EGS_Mesh* EGS_Mesh::parse_msh_file(std::istream& input) {
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
    std::vector<EGS_Mesh::Node> nodes, std::vector<EGS_Mesh::Medium> materials)
        : EGS_BaseGeometry("EGS_Mesh")
{
    std::size_t max_elts = std::numeric_limits<int>::max();
    if (elements.size() >= max_elts) {
        throw std::runtime_error("maximum number of elements (" +
            std::to_string(max_elts) + ") exceeded (" + std::to_string(elements.size()) + ")");
    }
    EGS_BaseGeometry::nreg = elements.size();

    _elt_tags.reserve(elements.size());
    _elt_points.reserve(elements.size() * 4);

    std::unordered_map<int, EGS_Mesh::Node> node_map;
    node_map.reserve(nodes.size());
    for (const auto& n : nodes) {
        node_map.insert({n.tag, n});
    }
    if (node_map.size() != nodes.size()) {
        throw std::runtime_error("duplicate nodes in node list");
    }
    // Find the matching nodes for every tetrahedron
    auto find_node = [&](int node_tag) -> EGS_Mesh::Node {
        auto node_it = node_map.find(node_tag);
        if (node_it == node_map.end()) {
            throw std::runtime_error("No mesh node with tag: " + std::to_string(node_tag));
        }
        return node_it->second;
    };
    for (int i = 0; i < static_cast<int>(elements.size()); i++) {
        const auto& e = elements[i];
        _elt_tags.push_back(e.tag);
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

    _boundary_faces.reserve(elements.size() * 4);
    for (const auto& ns: _neighbours) {
        for (const auto& n: ns) {
            _boundary_faces.push_back(n == mesh_neighbours::NONE);
        }
    }

    // TODO figure out materials setup (override setMedia?) with egsinp

    // map from medium tags to offsets
    std::unordered_map<int, int> medium_offsets;
    for (std::size_t i = 0; i < materials.size(); i++) {
        // TODO use EGS_BaseGeometry tracker
        // auto med = EGS_BaseGeometry::addMedium(m.medium_name);
        _medium_names.push_back(materials[i].medium_name);
        auto material_tag = materials[i].tag;
        bool inserted = medium_offsets.insert({material_tag, i}).second;
        if (!inserted) {
            throw std::runtime_error("duplicate medium tag: " + std::to_string(material_tag));
        }
    }

    _medium_indices.reserve(elements.size());
    for (const auto& e: elements) {
        // TODO handle vacuum tag (-1)?
        _medium_indices.push_back(medium_offsets.at(e.medium_tag));
    }

    initOctrees();
}

void EGS_Mesh::initOctrees() {
    std::vector<EGS_Mesh_Octree::Tet> elts;
    std::vector<EGS_Mesh_Octree::Tet> boundary_elts;
    elts.reserve(num_elements());
    for (int i = 0; i < num_elements(); i++) {
        EGS_Mesh_Octree::Tet elt(
             i,
            _elt_points.at(4*i),
            _elt_points.at(4*i+1),
            _elt_points.at(4*i+2),
            _elt_points.at(4*i+3)
        );
        elts.push_back(elt);
        if (is_boundary(i)) {
            boundary_elts.push_back(elt);
        }
    }
    std::cout << "before making octree\n";
    EGS_Mesh_Octree::initStaticMembers(elts);
    // Max element sizes from Furuta et al section 2.1.1
    std::size_t n_vol = 200;
    // TODO: pass in EGS_Mesh?
    _volume_tree = std::unique_ptr<EGS_Mesh_Octree>(
        new EGS_Mesh_Octree(elts, n_vol)
    );
    std::size_t n_surf = 100;
    _surface_tree = std::unique_ptr<EGS_Mesh_Octree>(
        new EGS_Mesh_Octree(boundary_elts, n_surf)
    );
    std::cout << "after making octree\n";
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

bool EGS_Mesh::insideElement(int i, const EGS_Vector &x) /* const */ {
    const auto& n = element_nodes(i);
    if (point_outside_of_plane(x, n.A, n.B, n.C, n.D)) {
        return false;
    }
    if (point_outside_of_plane(x, n.A, n.C, n.D, n.B)) {
        return false;
    }
    if (point_outside_of_plane(x, n.A, n.B, n.D, n.C)) {
        return false;
    }
    if (point_outside_of_plane(x, n.B, n.C, n.D, n.A)) {
        return false;
    }
    return true;
}

std::vector<int> EGS_Mesh::findNeighbourhood(int elt) {
    auto sz = 128;
    std::vector<int> hood;
    hood.reserve(sz);
    std::deque<int> to_search;
    to_search.push_back(elt);
    while (!to_search.empty() && hood.size() <= 128) {
        int elt = to_search.front();
        to_search.pop_front();
        std::array<int, 4> neighbours = _neighbours[elt];
        for (auto n : neighbours) {
            if (n == mesh_neighbours::NONE) {
                continue;
            }
            if (std::find(begin(hood), end(hood), n) == hood.end()) {
                hood.push_back(n);
                to_search.push_back(n);
            }
        }
    }
    return hood;
}

int EGS_Mesh::isWhere(const EGS_Vector &x) {
    static int num_calls = 0;
    static int num_hits = 0;

    if (!_volume_tree) {
        std::cerr << "lookup tree is null!\n";
    }

    auto octree_elt = _volume_tree->isWhere(x, *this);
    return octree_elt;

    int elt = -1;
    for (auto i = 0; i < num_elements(); i++) {
        if (insideElement(i, x)) {
            elt = i;
        }
    }

    if (octree_elt == elt) {
        num_hits++;
    } else {
        std::cout << "octree: " << octree_elt << "\n";
        std::cout << "brute:  " << elt << "\n";
        printElement(elt);
        std::cout << "false negative for point:\n";
        print_egsvec(x);
        exit(1);
    }

    num_calls++;
    if (num_calls && num_calls % 1000 == 0) {
        egsInformation("isWhere: \n%d of %d\n", num_hits, num_calls);
    }

    return elt;

    /*
    std::vector<EGS_Vector> centroids;
    centroids.reserve(num_elements());
    for (int i = 0; i < num_elements(); i++) {
        auto n = element_nodes(i);
        centroids.push_back(centroid(n.A, n.B, n.C, n.D));
    }

    std::cout << "\n";
    int elt = -1;
    for (int i = 0; i < num_elements(); i++) {
        const auto& c = centroids[i];
        //for (auto j = 0; j < num_elements(); j++) {
        //    if (insideElement(j, c)) {
        //        elt = j;
        //    }
        //}
        auto octree_elt = _volume_tree->isWhere(c, *this);
        if (i != octree_elt) {
            throw std::runtime_error("octree and brute don't match for element "
                + std::to_string(i) + " " + std::to_string(octree_elt));
        }
        //std::cout << "Brute search  " << elt << "\n";
        //std::cout << "Octree search " << octree_elt << "\n";
    }
    exit(1);
    */

    //int elt = -1;
    //{
    //auto start = std::chrono::steady_clock::now();
    //for (auto i = 0; i < num_elements(); i++) {
    //    if (insideElement(i, x)) {
    //        elt = i;
    //    }
    //}
    //auto end = std::chrono::steady_clock::now();
    //std::cout << "Brute search: "
    //    << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
    //    << " us\n";
    //}
    //std::cout << "elt should be " << elt << "\n";

    //return _volume_tree->findTetrahedron(x, *this);
    //auto start = std::chrono::steady_clock::now();
    //auto closest_elt = _volume_tree->findTetrahedron(x, *this);
    //auto end = std::chrono::steady_clock::now();
    //std::cout << "Octree search: "
    //    << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
    //    << " us\n";
    //std::cout << "found elt " << closest_elt << "\n";
    //if (closest_elt != -1) {
    //    //num_hits++;
    //    return closest_elt;
    //}

    //std::cout << "got closest elt " << closest_elt << "\n";
    //auto neighbourhood = findNeighbourhood(closest_elt);
    //std::array<int, 5> neighbourhood { closest_elt,
    //    _neighbours[closest_elt][0], _neighbours[closest_elt][1],
    //    _neighbours[closest_elt][2], _neighbours[closest_elt][3]
    //};
    //for (auto n : neighbourhood) {
    //    if (n != mesh_neighbours::NONE && insideElement(n, x)) {
    ////        num_hits++;
    //        return n;
    //    }
    //}
    for (auto i = 0; i < num_elements(); i++) {
        if (insideElement(i, x)) {
            return i;
        }
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
    return min_exterior_face_dist(x);
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

    const auto& n = element_nodes(ireg);
    maybe_update_min(n.A, n.B, n.C);
    maybe_update_min(n.A, n.C, n.D);
    maybe_update_min(n.A, n.B, n.D);
    maybe_update_min(n.B, n.C, n.D);

    return std::sqrt(min2);
}

EGS_Float EGS_Mesh::min_exterior_face_dist(const EGS_Vector& x) {
    EGS_Float dist = std::numeric_limits<EGS_Float>::max();
    _surface_tree->hownear_exterior(x, dist, *this);
    return dist;
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
            EGS_Vector n = cross(ab, ac);
            // egs++ convention is normal pointing opposite view ray
            if (dot(n, u) > 0) {
                n = -1.0 * n;
            }
            n.normalize();
            *normal = n;
        }
    };

    EGS_Float dist = 1e30;
    const auto& n = element_nodes(ireg);
    if (triangle_ray_intersection(x, u_norm, n.A, n.B, n.C, dist)) {
        // too far away to intersect
        if (dist > t) {
            return ireg;
        }
        t = dist;
        // index 3 = excluding last point D = face ABC
        auto new_reg = _neighbours[ireg][3];
        update_media_and_normal(n.A, n.B, n.C, new_reg);
        return new_reg;
    }
    if (triangle_ray_intersection(x, u_norm, n.A, n.C, n.D, dist)) {
        // too far away to intersect
        if (dist > t) {
            return ireg;
        }
        t = dist;
        // index 1 = excluding point B = face ACD
        auto new_reg = _neighbours[ireg][1];
        update_media_and_normal(n.A, n.C, n.D, new_reg);
        return new_reg;
    }
    if (triangle_ray_intersection(x, u_norm, n.A, n.B, n.D, dist)) {
        // too far away to intersect
        if (dist > t) {
            return ireg;
        }
        t = dist;
        // index 2 = excluding point C = face ABD
        auto new_reg = _neighbours[ireg][2];
        update_media_and_normal(n.A, n.B, n.D, new_reg);
        return new_reg;
    }
    if (triangle_ray_intersection(x, u_norm, n.B, n.C, n.D, dist)) {
        if (dist > t) {
            return ireg;
        }
        t = dist;
        // index 0 = excluding point A = face BCD
        auto new_reg = _neighbours[ireg][0];
        update_media_and_normal(n.B, n.C, n.D, new_reg);
        return new_reg;
    }

    return ireg;
}

EGS_Mesh::Intersection EGS_Mesh::closest_boundary_face(int ireg, const EGS_Vector &x,
    const EGS_Vector &u)
{
    assert(is_boundary(ireg));
    EGS_Float min_dist = std::numeric_limits<EGS_Float>::max();

    auto dist = min_dist;
    auto closest_face = -1;

    auto check_face_intersection = [&](int face, const EGS_Vector& A, const EGS_Vector& B,
            const EGS_Vector& C, const EGS_Vector& D)
    {
        if (_boundary_faces[4*ireg + face] &&
            // check if the point is on the outside looking in (rather than just
            // clipping the edge of a boundary face)
            point_outside_of_plane(x, A, B, C, D) &&
            triangle_ray_intersection(x, u, A, B, C, dist) && dist < min_dist)
        {
            min_dist = dist;
            closest_face = face;
        }
    };


    const auto& n = element_nodes(ireg);
    // face 0 (BCD), face 1 (ACD) etc.
    check_face_intersection(0, n.B, n.C, n.D, n.A);
    check_face_intersection(1, n.A, n.C, n.D, n.B);
    check_face_intersection(2, n.A, n.B, n.D, n.C);
    check_face_intersection(3, n.A, n.B, n.C, n.D);

    return EGS_Mesh::Intersection(min_dist, closest_face);
}

int EGS_Mesh::howfar_exterior(int ireg, const EGS_Vector &x, const EGS_Vector &u,
    EGS_Float &t, int *newmed, EGS_Vector *normal)
{
    EGS_Float min_dist = 1e30;
    auto min_reg = _surface_tree->howfar_exterior(x, u, t, min_dist, *this);

    // no intersection
    if (min_dist > t || min_reg == -1) {
        return -1;
    }

    // intersection found, update out parameters
    t = min_dist;
    if (newmed) {
        *newmed = medium(min_reg);
    }
    if (normal) {
        EGS_Vector tmp_normal;
        const auto& n = element_nodes(min_reg);
        auto intersection = closest_boundary_face(min_reg, x, u);
        switch(intersection.face_index) {
            case 0: tmp_normal = cross(n.C - n.B, n.D - n.B); break;
            case 1: tmp_normal = cross(n.C - n.A, n.D - n.A); break;
            case 2: tmp_normal = cross(n.B - n.A, n.D - n.A); break;
            case 3: tmp_normal = cross(n.B - n.A, n.C - n.A); break;
            default: throw std::runtime_error("Bad intersection, got face index: " +
                std::to_string(intersection.face_index));
        }
        // egs++ convention is normal pointing opposite view ray
        if (dot(tmp_normal, u) > 0) {
            tmp_normal = -1.0 * tmp_normal;
        }
        tmp_normal.normalize();
        *normal = tmp_normal;
    }
    return min_reg;
}

// TODO deduplicate
static char EGS_MESH_LOCAL geom_class_msg[] = "createGeometry(EGS_Mesh): %s\n";
const std::string EGS_Mesh::type = "EGS_Mesh";

void EGS_Mesh::printInfo() const {
    EGS_BaseGeometry::printInfo();
    std::ostringstream oss;
    printElement(0, oss);
    egsInformation(oss.str().c_str());
}

extern "C" {
    EGS_MESH_EXPORT EGS_BaseGeometry *createGeometry(EGS_Input *input) {
        if (!input) {
            egsWarning("createGeometry(EGS_Mesh): null input\n");
            return nullptr;
        }
        std::string mesh_file;
        int err = input->getInput("file", mesh_file);
        if (err) {
            egsWarning("createGeometry(EGS_Mesh): no mesh file key `file` in input\n");
            return nullptr;
        }
        if (!(mesh_file.length() >= 4 && mesh_file.rfind(".msh") == mesh_file.length() - 4)) {
            egsWarning("createGeometry(EGS_Mesh): unknown file extension for file `%s`,"
                "only `.msh` is allowed\n", mesh_file.c_str());
            return nullptr;
        }
        std::ifstream input_file(mesh_file);
        if (!input_file) {
            egsWarning("createGeometry(EGS_Mesh): unable to open file: `%s`\n"
                "\thelp => try using the absolute path to the mesh file",
                mesh_file.c_str());
            return nullptr;
        }
        EGS_Mesh* mesh = EGS_Mesh::parse_msh_file(input_file);
        if (!mesh) {
            egsWarning("createGeometry(EGS_Mesh): Gmsh msh file parsing failed\n");
            return nullptr;
        }
        mesh->setFilename(mesh_file);
        mesh->setBoundaryTolerance(input);
        mesh->setName(input);
        mesh->setLabels(input);
        for (const auto& medium: mesh->medium_names()) {
            mesh->addMedium(medium);
        }
        return mesh;
    }
}

/*
void EGS_Mesh::reorderMesh(const EGS_Vector &x) {
    std::vector<std::pair<int, EGS_Vector>> elt_centroids;
    elt_centroids.reserve(num_elements());
    for (int i = 0; i < num_elements(); i++) {
        auto n = element_nodes(i);
        elt_centroids.push_back(std::make_pair(i, centroid(n.A, n.B, n.C, n.D)));
    }
    std::sort(begin(elt_centroids), end(elt_centroids),
        [&](const std::pair<int, EGS_Vector>& A, const std::pair<int, EGS_Vector>& B) {
        return (A.second - x).length2() < (B.second - x).length2();
    });
    std::vector<int> reordered_tags;
    reordered_tags.reserve(num_elements());
    for (const auto& pair: elt_centroids) {
        reordered_tags.push_back(pair.first);
    }
    //for (int i = 0; i < 5; i++) {
    //    std::cout << "tet " << elt_centroids[i].first << "\nwith centroid:\n";
    //    print_egsvec(elt_centroids[i].second);
    //}
    renumberMesh(reordered_tags);
}

void EGS_Mesh::renumberMesh(const std::vector<int>& reordered_tags) {
    if (reordered_tags.size() != _elt_tags.size()) {
        throw std::runtime_error("renumberMesh: tag vector length mismatch");
    }

    // map from old numbering to new numbering
    std::unordered_map<int, int> renum;
    renum.reserve(num_elements());
    for (int i = 0; i < num_elements(); i++) {
        renum.insert({reordered_tags[i], i});
    }

    auto old_tags = _elt_tags;
    auto old_points = _elt_points;
    auto old_b_faces = _boundary_faces;
    auto old_b_elts = _boundary_elts;
    auto old_media = _medium_indices;
    auto old_neighbours = _neighbours;

    for (std::size_t i = 0; i < reordered_tags.size(); i++) {
        auto old = reordered_tags.at(i);
        _elt_tags.at(i) = old;
        for (int j = 0; j < 4; j++) {
            _elt_points.at(4*i+j) = old_points.at(4*old+j);
            _boundary_faces.at(4*i+j) = old_b_faces.at(4*old+j);
        }
        _boundary_elts.at(i) = old_b_elts.at(old);
        _medium_indices.at(i) = old_media.at(old);
        for (int j = 0; j < 4; j++) {
            auto old_n = old_neighbours.at(old).at(j);
            if (old_n == -1) {
                _neighbours.at(i).at(j) = -1;
            } else {
                _neighbours.at(i).at(j) = renum.at(old_n);
            }
        }
    }
}
*/
