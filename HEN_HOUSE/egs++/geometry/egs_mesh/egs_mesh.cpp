/*
###############################################################################
#
#  EGSnrc egs++ mesh geometry library implementation.
#
#  Copyright (C) 2022 Mevex Corporation
#
#  This file is part of EGSnrc.
#
#  Parts of this file, namely, the closest_point_triangle and
#  closest_point_tetrahedron functions, are adapted from Chapter 5 of
#  "Real-Time Collision Detection" by Christer Ericson with the permission
#  of the author and the publisher.
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
#  Revision:         Max Orok
#
###############################################################################
*/

#include "egs_input.h"
#include "egs_mesh.h"
#include "egs_vector.h"

#include "mesh_neighbours.h"
#include "msh_parser.h"

#include <cassert>
#include <chrono>
#include <deque>
#include <limits>
#include <stdexcept>
#include <unordered_map>

#ifndef WIN32
    #include <unistd.h> // isatty
#else
    #include <io.h>     // _isatty
#endif

// Have to define the move constructor, move assignment operator and destructor
// here instead of in egs_mesh.h because of the unique_ptr to forward declared
// EGS_Mesh_Octree members.
EGS_Mesh::~EGS_Mesh() = default;
EGS_Mesh::EGS_Mesh(EGS_Mesh&&) = default;
EGS_Mesh& EGS_Mesh::operator=(EGS_Mesh&&) = default;

void EGS_MeshSpec::checkValid() const {
    std::size_t n_max = std::numeric_limits<int>::max();
    if (this->elements.size() >= n_max) {
        throw std::runtime_error("maximum number of elements (" +
            std::to_string(n_max) + ") exceeded (" +
                std::to_string(this->elements.size()) + ")");
    }
    if (this->nodes.size() >= n_max) {
        throw std::runtime_error("maximum number of nodes (" +
            std::to_string(n_max) + ") exceeded (" +
                std::to_string(this->nodes.size()) + ")");
    }
}

namespace egs_mesh {
namespace internal {

PercentCounter::PercentCounter(EGS_InfoFunction info, const std::string& msg)
    : info_(info), msg_(msg) {}

void PercentCounter::start(EGS_Float goal) {
    goal_ = goal;
    t_start_ = std::chrono::system_clock::now();
#ifndef WIN32
    interactive_ = isatty(STDOUT_FILENO);
#else
    interactive_ = _isatty(STDOUT_FILENO);
#endif
    if (!info_ || !interactive_) {
        return;
    }
    info_("\r%s (0%%)", msg_.c_str());
}

// Assumes delta is positive
void PercentCounter::step(EGS_Float delta) {
    if (!info_ || !interactive_) {
        return;
    }
    progress_ += delta;
    const int percent = static_cast<int>((progress_ / goal_) * 100.0);
    if (percent > old_percent_) {
        info_("\r%s (%d%%)", msg_.c_str(), percent);
        old_percent_ = percent;
    }
}

void PercentCounter::finish(const std::string& end_msg) {
    if (!info_) {
        return;
    }
    const auto t_end = std::chrono::system_clock::now();
    const std::chrono::duration<float> elapsed = t_end - t_start_;
    if (!interactive_) {
        info_("%s in %0.3fs\n", end_msg.c_str(), elapsed.count());
        return;
    }
    // overwrite any remaining text
    info_("\r                                                                ");
    info_("\r%s in %0.3fs\n", end_msg.c_str(), elapsed.count());
}

} // namespace internal
} // namespace egs_mesh

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

inline EGS_Float dot(const EGS_Vector &x, const EGS_Vector &y) {
    return x * y;
}

inline EGS_Vector cross(const EGS_Vector &x, const EGS_Vector &y) {
    return x.times(y);
}

inline EGS_Float distance2(const EGS_Vector &x, const EGS_Vector &y) {
    return (x - y).length2();
}

inline EGS_Float distance(const EGS_Vector &x, const EGS_Vector &y) {
    return std::sqrt(distance2(x, y));
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

/// TODO: replace with Woop 2013 watertight algorithm.
///
/// Triangle-ray intersection algorithm for finding intersections with a ray
/// outside the tetrahedron.
/// Inputs:
/// * particle position p,
/// * normalized velocity v_norm
/// * triangle points A, B, C (any ordering)
///
/// Returns 1 if there is an intersection and 0 if not. If there is an intersection,
/// the out parameter dist will be the distance along v_norm to the intersection point.
///
/// Implementation of double-sided Möller-Trumbore ray-triangle intersection
/// <http://www.graphics.cornell.edu/pubs/1997/MT97.pdf>
int exterior_triangle_ray_intersection(const EGS_Vector &p,
    const EGS_Vector &v_norm, const EGS_Vector& a, const EGS_Vector& b,
    const EGS_Vector& c, EGS_Float& dist)
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
    if (dist < 0.0) {
        return 0;
    }
    return 1;
}

} // anonymous namespace

class EGS_Mesh_Octree {
private:
    static double tet_min_x(const EGS_Mesh::Nodes& n) {
        return std::min(n.A.x, std::min(n.B.x, std::min(n.C.x, n.D.x)));
    }
    static double tet_max_x(const EGS_Mesh::Nodes& n) {
        return std::max(n.A.x, std::max(n.B.x, std::max(n.C.x, n.D.x)));
    }
    static double tet_min_y(const EGS_Mesh::Nodes& n) {
        return std::min(n.A.y, std::min(n.B.y, std::min(n.C.y, n.D.y)));
    }
    static double tet_max_y(const EGS_Mesh::Nodes& n) {
        return std::max(n.A.y, std::max(n.B.y, std::max(n.C.y, n.D.y)));
    }
    static double tet_min_z(const EGS_Mesh::Nodes& n) {
        return std::min(n.A.z, std::min(n.B.z, std::min(n.C.z, n.D.z)));
    }
    static double tet_max_z(const EGS_Mesh::Nodes& n) {
        return std::max(n.A.z, std::max(n.B.z, std::max(n.C.z, n.D.z)));
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
        double volume() const {
            return (max_x - min_x) * (max_y - min_y) * (max_z - min_z);
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
            out <<
                std::setprecision(std::numeric_limits<double>::max_digits10) <<
                    "min_x: " << min_x << "\n";
            out <<
                std::setprecision(std::numeric_limits<double>::max_digits10) <<
                    "max_x: " << max_x << "\n";
            out <<
                std::setprecision(std::numeric_limits<double>::max_digits10) <<
                    "min_y: " << min_y << "\n";
            out <<
                std::setprecision(std::numeric_limits<double>::max_digits10) <<
                    "max_y: " << max_y << "\n";
            out <<
                std::setprecision(std::numeric_limits<double>::max_digits10) <<
                    "min_z: " << min_z << "\n";
            out <<
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
                        return false;
                    }
                }
            }
            // category 1 - test overlap with AABB face normals
            if (max3(v0.x, v1.x, v2.x) <= -ex || min3(v0.x, v1.x, v2.x) >= ex ||
                max3(v0.y, v1.y, v2.y) <= -ey || min3(v0.y, v1.y, v2.y) >= ey ||
                max3(v0.z, v1.z, v2.z) <= -ez || min3(v0.z, v1.z, v2.z) >= ez)
            {
                return false;
            }

            // category 2 - test overlap with triangle face normal using AABB
            // plane test (5.2.3)

            // Cross product robustness issues are ignored here (assume
            // non-degenerate and non-oversize triangles)
            const EGS_Vector n = cross(edge_vecs[0], edge_vecs[1]);
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

        bool intersects_tetrahedron(const EGS_Mesh::Nodes& tet) const {
            return intersects_triangle(tet.A, tet.B, tet.C) ||
                   intersects_triangle(tet.A, tet.C, tet.D) ||
                   intersects_triangle(tet.A, tet.B, tet.D) ||
                   intersects_triangle(tet.B, tet.C, tet.D);
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

        // Given an interior point, return the minimum distance to a boundary.
        // This is a helper method for hownear, as the minimum of the boundary
        // distance and tetrahedron distance will be hownear's result.
        //
        // TODO maybe need to clamp to 0.0 here for results slightly under zero?
        EGS_Float min_interior_distance(const EGS_Vector& point) const {
            return std::min(point.x - min_x, std::min(point.y - min_y,
                   std::min(point.z - min_z, std::min(max_x - point.x,
                   std::min(max_y - point.y, max_z - point.z)))));
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

        Node() = default;
        Node(const std::vector<int> &elts, const BoundingBox& bbox,
            std::size_t n_max, const EGS_Mesh& mesh,
            egs_mesh::internal::PercentCounter& progress) : bbox_(bbox)
        {
            // TODO: max level and precision warning
            if (bbox_.is_indivisible() || elts.size() < n_max) {
                elts_ = elts;
                progress.step(bbox_.volume());
                return;
            }

            std::array<std::vector<int>, 8> octants;
            std::array<BoundingBox, 8> bbs = bbox_.divide8();

            // elements may be in more than one bounding box
            for (const auto &e : elts) {
                for (int i = 0; i < 8; i++) {
                    if (bbs[i].intersects_tetrahedron(mesh.element_nodes(e))) {
                        octants[i].push_back(e);
                    }
                }
            }
            for (int i = 0; i < 8; i++) {
                children_.push_back(Node(
                    std::move(octants[i]), bbs[i], n_max, mesh, progress
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
            // Perf note: this function was changed to use std::array, but there
            // wasn't any observed performance change during benchmarking. Since
            // the std::array logic was more complicated, the std::vector impl
            // was kept.
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

        // Leaf node: search all bounded elements, returning the minimum
        // distance to a boundary tetrahedron or a bounding box surface.
        EGS_Float hownear_leaf_search(const EGS_Vector& p, EGS_Mesh& mesh) const
        {
            const EGS_Float best_dist = bbox_.min_interior_distance(p);
            // Use squared distance to avoid computing square roots in the
            // loop. This has the added bonus of ridding ourselves of any
            // negatives from near-zero floating-point issues
            EGS_Float best_dist2 = best_dist * best_dist;
            for (const auto &e: elts_) {
                const auto& n = mesh.element_nodes(e);
                best_dist2 = std::min(best_dist2, distance2(p,
                    closest_point_tetrahedron(p, n.A, n.B, n.C, n.D)));
            }
            return std::sqrt(best_dist2);
        }

        EGS_Float hownear_exterior(const EGS_Vector& p, EGS_Mesh& mesh) const
        {
            // Leaf node: find a lower bound on the mesh exterior distance
            // closest distance
            if (isLeaf()) {
                return hownear_leaf_search(p, mesh);
            }
            // Parent node: decide which octant to search and descend the tree
            const auto octant = findOctant(p);
            return children_[octant].hownear_exterior(p, mesh);
        }

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
            return children_[findOctant(p)].isWhere(p, mesh);
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
    EGS_Mesh_Octree() = default;
    EGS_Mesh_Octree(const std::vector<int> &elts, std::size_t n_max,
        const EGS_Mesh& mesh, egs_mesh::internal::PercentCounter& progress)
    {
        if (elts.empty()) {
            throw std::runtime_error("EGS_Mesh_Octree: empty elements vector");
        }
        if (elts.size() > std::numeric_limits<int>::max()) {
            throw std::runtime_error("EGS_Mesh_Octree: num elts must fit into an int");
        }

        const EGS_Float INF = std::numeric_limits<EGS_Float>::infinity();
        BoundingBox g_bounds(INF, -INF, INF, -INF, INF, -INF);
        for (const auto& e : elts) {
            const auto& nodes = mesh.element_nodes(e);
            g_bounds.min_x = std::min(g_bounds.min_x, tet_min_x(nodes));
            g_bounds.max_x = std::max(g_bounds.max_x, tet_max_x(nodes));
            g_bounds.min_y = std::min(g_bounds.min_y, tet_min_y(nodes));
            g_bounds.max_y = std::max(g_bounds.max_y, tet_max_y(nodes));
            g_bounds.min_z = std::min(g_bounds.min_z, tet_min_z(nodes));
            g_bounds.max_z = std::max(g_bounds.max_z, tet_max_z(nodes));
        }
        // Add a small delta around the bounding box to avoid numerical problems
        // at the boundary
        g_bounds.expand(1e-8);

        // Track progress using how much volume has been covered
        progress.start(g_bounds.volume());
        root_ = Node(elts, g_bounds, n_max, mesh, progress);
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

    // Returns a lower bound on the distance to the mesh exterior boundary.
    // The actual distance to the mesh may be larger, i.e. a distance to an
    // axis-aligned bounding box might be returned instead. This is allowed by
    // the HOWNEAR spec, PIRS-701 section 3.6, "Specifications for HOWNEAR":
    //
    // > In complex geometries, the mathematics of HOWNEAR can become difficult
    // and sometimes almost impossible! If it is easier for the user to
    // compute some lower bound to the nearest distance, this could be used...
    EGS_Float hownear_exterior(const EGS_Vector& p, EGS_Mesh& mesh) const {
        // If the point is outside the octree bounding box, return the distance
        // to the bounding box.
        if (!root_.bbox_.contains(p)) {
            return distance(root_.bbox_.closest_point(p), p);
        }
        // Otherwise, descend the octree
        return root_.hownear_exterior(p, mesh);
    }
};

EGS_Mesh::EGS_Mesh(EGS_MeshSpec spec) :
    EGS_BaseGeometry(EGS_BaseGeometry::getUniqueName())
{
    spec.checkValid();
    initializeElements(std::move(spec.elements), std::move(spec.nodes),
        std::move(spec.media));
    initializeNeighbours();
    initializeOctrees();
    initializeNormals();
}

void EGS_Mesh::initializeElements(
    std::vector<EGS_MeshSpec::Tetrahedron> elements,
    std::vector<EGS_MeshSpec::Node> nodes,
    std::vector<EGS_MeshSpec::Medium> materials)
{
    EGS_BaseGeometry::nreg = elements.size();

    elt_tags_.reserve(elements.size());
    elt_node_indices_.reserve(elements.size());
    nodes_.reserve(nodes.size());

    std::unordered_map<int, int> node_map;
    node_map.reserve(nodes.size());
    for (int i = 0; i < static_cast<int>(nodes.size()); i++) {
        const auto& n = nodes[i];
        node_map.insert({n.tag, i});
        nodes_.push_back(EGS_Vector(n.x, n.y, n.z));
    }
    if (node_map.size() != nodes.size()) {
        throw std::runtime_error("duplicate nodes in node list");
    }
    // Find the matching node indices for every tetrahedron
    auto find_node = [&](int node_tag) -> int {
        auto node_it = node_map.find(node_tag);
        if (node_it == node_map.end()) {
            throw std::runtime_error("No mesh node with tag: " + std::to_string(node_tag));
        }
        return node_it->second;
    };
    for (int i = 0; i < static_cast<int>(elements.size()); i++) {
        const auto& e = elements[i];
        elt_tags_.push_back(e.tag);
        elt_node_indices_.push_back({
            find_node(e.a), find_node(e.b), find_node(e.c), find_node(e.d)
        });
    }

    initializeMedia(std::move(elements), std::move(materials));
}

void EGS_Mesh::initializeMedia(std::vector<EGS_MeshSpec::Tetrahedron> elements,
    std::vector<EGS_MeshSpec::Medium> materials)
{
    std::unordered_map<int, int> medium_offsets;
    for (const auto& m : materials) {
        // If the medium was already registered, returns its offset. For new
        // media, addMedium adds them to the list and returns the new offset.
        const int media_offset = EGS_BaseGeometry::addMedium(m.medium_name);
        bool inserted = medium_offsets.insert({m.tag, media_offset}).second;
        if (!inserted) {
            throw std::runtime_error("duplicate medium tag: "
                + std::to_string(m.tag));
        }
    }

    medium_indices_.reserve(elements.size());
    for (const auto& e: elements) {
        medium_indices_.push_back(medium_offsets.at(e.medium_tag));
    }
}

void EGS_Mesh::initializeNeighbours() {
    std::vector<mesh_neighbours::Tetrahedron> neighbour_elts;
    neighbour_elts.reserve(num_elements());
    for (const auto& e: elt_node_indices_) {
        neighbour_elts.emplace_back(mesh_neighbours::Tetrahedron(e[0], e[1], e[2], e[3]));
    }

    egs_mesh::internal::PercentCounter progress(get_logger(),
        "EGS_Mesh: finding element neighbours");

    neighbours_ = mesh_neighbours::tetrahedron_neighbours(
            neighbour_elts, progress);

    progress.finish("EGS_Mesh: found element neighbours");

    boundary_faces_.reserve(num_elements() * 4);
    for (const auto& ns: neighbours_) {
        for (const auto& n: ns) {
            boundary_faces_.push_back(n == mesh_neighbours::NONE);
        }
    }
}

void EGS_Mesh::initializeNormals() {
    face_normals_.reserve(num_elements());
    for (int i = 0; i < static_cast<int>(num_elements()); i++) {
        auto get_normal = [](const EGS_Vector& a, const EGS_Vector& b,
            const EGS_Vector& c, const EGS_Vector& d) -> EGS_Vector
        {
            EGS_Vector normal = cross(b - a, c - a);
            normal.normalize();
            if (dot(normal, d-a) < 0) {
                normal *= -1.0;
            }
            return normal;
        };
        const auto& n = element_nodes(i);
        face_normals_.push_back({
            get_normal(n.B, n.C, n.D, n.A),
            get_normal(n.A, n.C, n.D, n.B),
            get_normal(n.A, n.B, n.D, n.C),
            get_normal(n.A, n.B, n.C, n.D)
        });
    }
}

void EGS_Mesh::initializeOctrees() {
    std::vector<int> elts;
    std::vector<int> boundary_elts;
    elts.reserve(num_elements());
    for (int i = 0; i < num_elements(); i++) {
        elts.push_back(i);
        if (is_boundary(i)) {
            boundary_elts.push_back(i);
        }
    }
    // Max element sizes from Furuta et al section 2.1.1
    std::size_t n_vol = 200;
    egs_mesh::internal::PercentCounter vol_progress(get_logger(),
        "EGS_Mesh: building volume octree");
    volume_tree_ = std::unique_ptr<EGS_Mesh_Octree>(
        new EGS_Mesh_Octree(elts, n_vol, *this, vol_progress)
    );
    vol_progress.finish("EGS_Mesh: built volume octree");

    std::size_t n_surf = 100;
    egs_mesh::internal::PercentCounter surf_progress(get_logger(),
        "EGS_Mesh: building surface octree");
    surface_tree_ = std::unique_ptr<EGS_Mesh_Octree>(
        new EGS_Mesh_Octree(boundary_elts, n_surf, *this, surf_progress)
    );
    surf_progress.finish("EGS_Mesh: built surface octree");
}

bool EGS_Mesh::isInside(const EGS_Vector &x) {
    return isWhere(x) != -1;
}

int EGS_Mesh::inside(const EGS_Vector &x) {
    return isWhere(x);
}

int EGS_Mesh::medium(int ireg) const {
    return medium_indices_.at(ireg);
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

int EGS_Mesh::isWhere(const EGS_Vector &x) {
    return volume_tree_->isWhere(x, *this);
}

EGS_Float EGS_Mesh::hownear(int ireg, const EGS_Vector& x) {
    // inside
    if (ireg >= 0) {
        return min_interior_face_dist(ireg, x);
    }
    // outside
    return min_exterior_face_dist(x);
}

// Assumes the input normal is normalized. Returns the absolute value of the
// distance.
EGS_Float distance_to_plane(const EGS_Vector &x,
    const EGS_Vector& unit_plane_normal, const EGS_Vector& plane_point)
{
    return std::abs(dot(unit_plane_normal, x - plane_point));
}

EGS_Float EGS_Mesh::min_interior_face_dist(int ireg, const EGS_Vector& x) {
    const auto& n = element_nodes(ireg);

    // First face is BCD, second is ACD, third is ABD, fourth is ABC
    EGS_Float min_dist = distance_to_plane(x, face_normals_[ireg][0], n.B);
    min_dist = std::min(min_dist,
        distance_to_plane(x, face_normals_[ireg][1], n.A));
    min_dist = std::min(min_dist,
        distance_to_plane(x, face_normals_[ireg][2], n.A));
    min_dist = std::min(min_dist,
        distance_to_plane(x, face_normals_[ireg][3], n.A));

    return min_dist;
}

EGS_Float EGS_Mesh::min_exterior_face_dist(const EGS_Vector& x) {
    return surface_tree_->hownear_exterior(x, *this);
}

int EGS_Mesh::howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
    EGS_Float &t, int *newmed /* =0 */, EGS_Vector *normal /* =0 */)
{
    if (ireg < 0) {
        return howfar_exterior(x, u, t, newmed, normal);
    }
    // Find the minimum distance to an element boundary. If this distance is
    // smaller than the intended step, adjust the step length and return the
    // neighbouring element. If the step length is larger than the distance to a
    // boundary, don't adjust it upwards! This will break particle transport.
    EGS_Float distance_to_boundary = veryFar;
    auto new_reg = howfar_interior(
        ireg, x, u, distance_to_boundary, newmed, normal);

    if (distance_to_boundary < t) {
        t = distance_to_boundary;
        return new_reg;
    }
    // Otherwise, return the current region and don't change the value of t.
    return ireg;
}

// howfar_interior is the most complicated EGS_BaseGeometry method. Apart from
// the intersection logic, there are exceptional cases that must be carefully
// handled. In particular, the region number and the position `x` may not agree.
// For example, the region may be 1, but the position x may be slightly outside
// of region 1 because of numerical undershoot. The region number takes priority
// because it is where EGS thinks the particle should be based on the simulation
// so far, and we assume steps like boundary crossing calculations have already
// taken place. So we have to do our best to calculate intersections as if the
// position really is inside the given tetrahedron.
int EGS_Mesh::howfar_interior(int ireg, const EGS_Vector &x, const EGS_Vector &u,
    EGS_Float &t, int *newmed, EGS_Vector *normal)
{
    // General idea is to use ray-plane intersection because it's watertight and
    // uses less flops than ray-triangle intersection. To my understanding, this
    // approach is also used by Geant, PHITS and MCNP.
    //
    // Because planes are infinite, intersections can be found far away from
    // the actual tetrahedron bounds if the particle is travelling away from the
    // element. So we limit valid intersections with planes by element face
    // distances and normal angles.
    //
    //                          OK, if d < eps, and theta > -angle_eps
    //
    //        |<-d->|               |    /
    //              |               | i /
    //              |  n          * |  /
    //        * ->  | -->         | | /
    //              |             v |/
    //              |               /
    //              |              /
    //                            x
    //
    // There is a chance that floating-point rounding may cause computed
    // quantities to be inconsistent. The most important thing is the simulation
    // should try to continue by always at least taking a small step. Returning
    // 0.0 opens the door to hanging the transport routine entirely and for
    // enough histories, it will almost certainly hang. Even returning a small
    // step does not ensure forward progress for all possible input meshes,
    // since if the step is too small, it may be wiped out by rounding error
    // after being added to a large number. But if the step is too large, small
    // tetrahedrons may be skipped entirely.

    // TODO add test for transport after intersection point lands right on a
    // corner node.

    const auto& n = element_nodes(ireg);
    // Pick an arbitrary face point to do plane math with. Face 0 is BCD, Face 1
    // is ACD, etc...
    std::array<EGS_Vector, 4> face_points {n.B, n.A, n.A, n.A};
    std::array<PointLocation, 4> intersect_tests {};
    for (int i = 0; i < 4; ++i) {
        intersect_tests[i] = find_point_location(
                x, u, face_points[i], face_normals_[ireg][i]);
    }
    // If the particle is not strictly inside the element, try transporting
    // along a thick plane.
    if (intersect_tests[0].signed_distance < 0.0 ||
        intersect_tests[1].signed_distance < 0.0 ||
        intersect_tests[2].signed_distance < 0.0 ||
        intersect_tests[3].signed_distance < 0.0)
    {
        return howfar_interior_thick_plane(intersect_tests, ireg, x, u, t,
            newmed, normal);
    }

    // Otherwise, if points are inside the element, calculate the minimum
    // distance to a plane.
    auto ix = find_interior_intersection(intersect_tests);
    if (ix.dist < 0.0 || ix.face_index == -1) {
        egsWarning("\nEGS_Mesh warning: bad interior intersection t = %.17g, face_index = %d in region %d: "
                   "x=(%.17g,%.17g,%.17g) u=(%.17g,%.17g,%.17g)\n", ix.dist, ix.face_index,
                   ireg, x.x, x.y, x.z, u.x, u.y, u.z);
        EGS_BaseGeometry::error_flag = 1;
        return ireg;
    }
    // Very small distances might get swallowed up by rounding error, so enforce
    // a minimum step size.
    if (ix.dist < EGS_Mesh::min_step_size) {
        ix.dist = EGS_Mesh::min_step_size;
    }
    t = ix.dist;
    int new_reg = neighbours_[ireg].at(ix.face_index);
    update_medium(new_reg, newmed);
    update_normal(face_normals_[ireg].at(ix.face_index), u, normal);
    return new_reg;
}

// Returns the position of the point relative to the face.
EGS_Mesh::PointLocation EGS_Mesh::find_point_location(const EGS_Vector& x,
    const EGS_Vector& u, const EGS_Vector& plane_point,
    const EGS_Vector& plane_normal)
{
    // TODO handle degenerate triangles, by not finding any intersections.

    // Face normals point inwards, to the tetrahedron centroid. So if
    // dot(u, n) < 0, the particle is travelling towards the plane and will
    // intersect it at some point. The intersection point might be outside the
    // face's triangular bounds though.
    EGS_Float direction_dot_normal = dot(u, plane_normal);

    // Find the signed distance to the plane, to see if it lies in the thick
    // plane and so might be a candidate for transport even if it's not strictly
    // inside the tetrahedron. This is the distance along the plane normal, not
    // the distance along the velocity vector.
    //
    //  (-)       (+)
    //        |
    //  *     |-> n
    //        |
    //  |-----+
    //     d
    //
    EGS_Float signed_distance = dot(plane_normal, x - plane_point);
    return PointLocation(direction_dot_normal, signed_distance);
}

// Assuming the point is inside the element, find the plane intersection point.
//
// Caller is responsible for checking that each intersections signed_distance is
// >= 0.
EGS_Mesh::Intersection EGS_Mesh::find_interior_intersection(
    const std::array<PointLocation, 4>& ixs)
{
    EGS_Float t_min = veryFar;
    int min_face_index = -1;
    for (int i = 0; i < 4; ++i) {
        // If the particle is travelling away from the face, it will not
        // intersect it.
        if (ixs[i].direction_dot_normal >= 0.0) {
            continue;
        }
        EGS_Float t_i =  -ixs[i].signed_distance / ixs[i].direction_dot_normal;
        if (t_i < t_min) {
            t_min   = t_i;
            min_face_index = i;
        }
    }
    // Is there a risk of returning min_face_index = -1? Could t_min also be negative?
    return Intersection(t_min, min_face_index);
}

// Try and transport the particle using thick plane intersections.
int EGS_Mesh::howfar_interior_thick_plane(const std::array<PointLocation, 4>&
    intersect_tests, int ireg, const EGS_Vector &x, const EGS_Vector &u,
    EGS_Float &t, int *newmed, EGS_Vector *normal)
{
    // The particle isn't inside the element, but it might be a candidate for
    // transport along a thick plane, as long as it is parallel or travelling
    // towards the element.
    //
    //        OK              not OK            not OK
    //
    //                           *                 ^
    //    -    * ->            - |               - |
    //  d |    |             d | v             d | *
    //    -----v---            ------            ------
    //

    // Find the largest negative distance to a face plane. If it is bigger
    // than the thick plane tolerance, the particle isn't close enough to be
    // considered part of the element.
    EGS_Float max_neg_dist = veryFar;
    int face_index = -1;
    for (int i = 0; i < 4; ++i) {
        if (intersect_tests[i].signed_distance < max_neg_dist) {
            max_neg_dist = intersect_tests[i].signed_distance;
            face_index = i;
        }
    }
    if (face_index == -1) {
        egsWarning("\nEGS_Mesh warning: howfar_interior_thick_plane face_index %d in region %d: "
                   "x=(%.17g,%.17g,%.17g) u=(%.17g,%.17g,%.17g)\n",
                   face_index, ireg, x.x, x.y, x.z, u.x, u.y, u.z);
        EGS_BaseGeometry::error_flag = 1;
        return ireg;
    }

    // If the perpendicular distance to the plane is too big to be a thick
    // plane, or if the particle is travelling away from the plane, push the
    // particle by a small step and return the region the particle is in.

    // Some small epsilon, TODO maybe look into gamma bounds for this?
    // Or EGS_BaseGeometry::BoundaryTolerance?
    constexpr EGS_Float thick_plane_bounds = EGS_Mesh::min_step_size;
    if (max_neg_dist < -thick_plane_bounds ||
        intersect_tests.at(face_index).direction_dot_normal < -thick_plane_bounds)
    {
        return howfar_interior_recover_lost_particle(ireg, x, u, t, newmed);
    }

    // If the particle is inside the thick plane and travelling towards a face
    // plane, find the intersection point. This might be outside the strict
    // bounds of the tetrahedron, but necessary for "wall-riding" particles to
    // be transported without tanking simulation efficiency.
    //
    //       \            /
    //  out   \    * ->  x
    //         \--------/
    //  in      \      /
    //
    EGS_Float t_min = veryFar;
    int min_face_index = -1;
    for (int i = 0; i < 4; ++i) {
        // If the particle is travelling away from the face, it will not
        // intersect it.
        if (intersect_tests[i].direction_dot_normal >= 0.0) {
            continue;
        }

        EGS_Float t_i = -intersect_tests[i].signed_distance
                        / intersect_tests[i].direction_dot_normal;

        if (t_i < t_min) {
            t_min = t_i;
            min_face_index = i;
        }
    }

    // Rarely, rounding error near an edge or corner can cause t_min to be
    // negative or a very small positive number. For this case, try to recover
    // as if the particle is lost.
    if (t_min < EGS_Mesh::min_step_size) {
        return howfar_interior_recover_lost_particle(ireg, x, u, t, newmed);
    }
    if (min_face_index == -1) {
        egsWarning("\nEGS_Mesh warning: face_index %d in region %d: "
                   "x=(%.17g,%.17g,%.17g) u=(%.17g,%.17g,%.17g)\n",
                   min_face_index, ireg, x.x, x.y, x.z, u.x, u.y, u.z);
        EGS_BaseGeometry::error_flag = 1;
        return ireg;
    }
    t = t_min;
    int new_reg = neighbours_[ireg].at(min_face_index);
    update_medium(new_reg, newmed);
    update_normal(face_normals_[ireg].at(min_face_index), u, normal);
    return new_reg;
}

/* No valid intersections were found for this particle. Push it along the
   momentum vector by a small step and try to recover.

           /\
     <- * /  \
         /____\

    ^^^^ i.e., which region is this particle actually in?

   There are many cases where this could happen, so this routine can't make
   too many assumptions. Some possibiltites:

   * The particle is right on an edge and a negative distance was calculated
   * The particle is inside the thick plane but travelling away from ireg
   * The particle is outside the thick plane but travelling away or towards ireg
*/
int EGS_Mesh::howfar_interior_recover_lost_particle(int ireg,
    const EGS_Vector &x, const EGS_Vector &u, EGS_Float &t, int *newmed)
{
    t = EGS_Mesh::min_step_size;
    // This may hang if min_step_size is too small for the mesh.
    EGS_Vector new_pos = x + u * t;
    // Fast path: particle is in a neighbouring element.
    for (int i = 0; i < 4; ++i) {
        const auto neighbour = neighbours_[ireg][i];
        if (neighbour == -1) {
            continue;
        }
        if (insideElement(neighbour, new_pos)) {
            update_medium(neighbour, newmed);
            return neighbour;
        }
    }
    // Slow path: particle isn't in a neighbouring element, initiate a full
    // octree search. This can also happen if the particle is leaving the
    // mesh.
    auto actual_elt = isWhere(new_pos);
    update_medium(actual_elt, newmed);
    // We can't determine which normal to display (which is only for
    // egs_view in any case), so we don't update the normal for this
    // exceptional case.
    return actual_elt;
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
        if (boundary_faces_[4*ireg + face] &&
            // check if the point is on the outside looking in (rather than just
            // clipping the edge of a boundary face)
            point_outside_of_plane(x, A, B, C, D) &&
            dot(face_normals_[ireg][face], u) > 0.0 && // point might be in a thick plane
            exterior_triangle_ray_intersection(x, u, A, B, C, dist) &&
            dist < min_dist)
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

int EGS_Mesh::howfar_exterior(const EGS_Vector &x, const EGS_Vector &u,
    EGS_Float &t, int *newmed, EGS_Vector *normal)
{
    EGS_Float min_dist = 1e30;
    auto min_reg = surface_tree_->howfar_exterior(x, u, t, min_dist, *this);

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
            egsWarning("createGeometry(EGS_Mesh): mesh file: `%s` does "
                       "not exist or is not readable\n", mesh_file.c_str());
            return nullptr;
        }

        EGS_MeshSpec mesh_spec;
        try {
            mesh_spec = msh_parser::parse_msh_file(input_file, egsInformation);
        }
        catch (const std::runtime_error& e) {
            std::string error_msg = std::string("createGeometry(EGS_Mesh): ") +
                "Gmsh msh file parsing failed\nerror: " + e.what() + "\n";
            egsWarning("\n%s", error_msg.c_str());
            return nullptr;
        }

        EGS_Mesh* mesh = nullptr;
        try {
            mesh = new EGS_Mesh(std::move(mesh_spec));
        } catch (const std::runtime_error& e) {
            std::string error_msg = std::string("createGeometry(EGS_Mesh): ") +
                "bad input to EGS_Mesh\nerror: " + e.what() + "\n";
            egsWarning("\n%s", error_msg.c_str());
            return nullptr;
        }

        mesh->setBoundaryTolerance(input);
        mesh->setName(input);
        mesh->setLabels(input);
        return mesh;
    }
}

