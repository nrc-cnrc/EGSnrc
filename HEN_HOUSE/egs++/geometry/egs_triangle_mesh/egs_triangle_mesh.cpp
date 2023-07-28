/*
###############################################################################
#
#  EGSnrc egs++ triangle mesh geometry library implementation.
#  Copyright (C) 2022 Max Orok
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
#  Authors:          Max Orok, 2022
#
#  Contributors:
#
###############################################################################
*/

#include "egs_input.h"
#include "egs_triangle_mesh.h"

#include "stl_parser.h"

#include <algorithm>
#include <array>
#include <stdexcept>

namespace { // anonymous namespace for low-level geometry routines
const EGS_Float eps = 1e-8;

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

inline EGS_Float min3(EGS_Float a, EGS_Float b, EGS_Float c) {
    return std::min(std::min(a, b), c);
}

inline EGS_Float max3(EGS_Float a, EGS_Float b, EGS_Float c) {
    return std::max(std::max(a, b), c);
}

inline bool approx_eq(double a, double b, double e = eps) {
    return (std::abs(a - b) <= e * (std::abs(a) + std::abs(b) + 1.0));
} //this is a helper function for the is_indivisible method

inline bool is_zero(const EGS_Vector &v) {
    return approx_eq(0.0, v.length(), eps);
}

// Test whether the point `p` is in front of the plane defined by the triangle's
// normal `n` and sample point `a` (i.e. in the plane's positive halfspace).
//
// This is a low-level method, callers must add additional logic to handle
// various edge cases if required (e.g. point on plane, etc.).
bool is_outside_of_triangle_plane(const EGS_Vector& p, const EGS_Vector& n,
    const EGS_Vector& a)
{
    return dot(n, (p - a)) >= 0.0;
}

// Find the closest point on triangle ABC to query point P.
EGS_Vector closest_point_triangle(const EGS_Vector &P, const EGS_Vector &A, const EGS_Vector &B, const EGS_Vector &C) {
    // vertex region A
    EGS_Vector ab = B - A;
    EGS_Vector ac = C - A;
    EGS_Vector ao = P - A;

    EGS_Float d1 = dot(ab, ao);
    EGS_Float d2 = dot(ac, ao);
    if (d1 <= 0.0 && d2 <= 0.0) {
        return A;
    }

    // vertex region B
    EGS_Vector bo = P - B;
    EGS_Float d3 = dot(ab, bo);
    EGS_Float d4 = dot(ac, bo);
    if (d3 >= 0.0 && d4 <= d3) {
        return B;
    }

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
    if (d6 >= 0.0 && d5 <= d6) {
        return C;
    }

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

// Möller-Trumbore triangle-ray intersection algorithm.
//
// Returns true if there is an intersection with the triangle abc and false
// otherwise. If there is an intersection, the out parameter `dist` is set to
// the intersection distance. Only positive values of `dist` are set. Particles
// travelling parallel to the triangle are considered to not intersect.
bool triangle_ray_intersection(const EGS_Vector &p,
                              const EGS_Vector &v_norm, const EGS_Vector &a, const EGS_Vector &b,
                              const EGS_Vector &c, EGS_Float &dist) {
    const EGS_Float eps = 1e-10;
    EGS_Vector ab = b - a;
    EGS_Vector ac = c - a;

    EGS_Vector pvec = cross(v_norm, ac);
    EGS_Float det = dot(ab, pvec);

    if (det > -eps && det < eps) {
        return false;
    }
    EGS_Float inv_det = 1.0 / det;
    EGS_Vector tvec = p - a;
    EGS_Float u = dot(tvec, pvec) * inv_det;
    if (u < 0.0 || u > 1.0) {
        return false;
    }
    EGS_Vector qvec = cross(tvec, ab);
    EGS_Float v = dot(v_norm, qvec) * inv_det;
    if (v < 0.0 || u + v > 1.0) {
        return false;
    }
    // intersection found
    dist = dot(ac, qvec) * inv_det;
    // along a negative direction of the ray
    if (dist < 0.0) {
        return false;
    }
    return true;
}

} // anonymous namespace

// exclude from doxygen
/// @cond
class EGS_TriangleMeshBbox {
public:
    EGS_TriangleMeshBbox() = default;
    EGS_TriangleMeshBbox(double min_x, double max_x, double min_y, double max_y,
                double min_z, double max_z) : min_x(min_x), max_x(max_x),
        min_y(min_y), max_y(max_y), min_z(min_z), max_z(max_z) {}

    void expand(double delta) {
        min_x -= delta;
        min_y -= delta;
        min_z -= delta;
        max_x += delta;
        max_y += delta;
        max_z += delta;
    }
    //get the midpoints of the bounding box along each axis to create the octets by dividing boxes into 8
    double mid_x() const {
        return (min_x + max_x) / 2.0;
    }
    double mid_y() const {
        return (min_y + max_y) / 2.0;
    }
    double mid_z() const {
        return (min_z + max_z) / 2.0;
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

    std::array<EGS_TriangleMeshBbox, 8> divide8() const {
            return {
                EGS_TriangleMeshBbox(
                    min_x, mid_x(),
                    min_y, mid_y(),
                    min_z, mid_z()
                ),
                EGS_TriangleMeshBbox(
                    mid_x(), max_x,
                    min_y, mid_y(),
                    min_z, mid_z()
                ),
                EGS_TriangleMeshBbox(
                    min_x, mid_x(),
                    mid_y(), max_y,
                    min_z, mid_z()
                ),
                EGS_TriangleMeshBbox(
                    mid_x(), max_x,
                    mid_y(), max_y,
                    min_z, mid_z()
                ),
                EGS_TriangleMeshBbox(
                    min_x, mid_x(),
                    min_y, mid_y(),
                    mid_z(), max_z
                ),
                EGS_TriangleMeshBbox(
                    mid_x(), max_x,
                    min_y, mid_y(),
                    mid_z(), max_z
                ),
                EGS_TriangleMeshBbox(
                    min_x, mid_x(),
                    mid_y(), max_y,
                    mid_z(), max_z
                ),
                EGS_TriangleMeshBbox(
                    mid_x(), max_x,
                    mid_y(), max_y,
                    mid_z(), max_z
                )
            };
        }

    bool contains(const EGS_Vector &point) const {
        // non-inclusive on the boundary
        // so points on the interface between two bounding boxes only belong
        // to one of them:
        //      +---+---+
        //      |   x   |
        //      +---+---+
        //            ^ belongs here
        return point.x > min_x && point.x < max_x &&
               point.y > min_y && point.y < max_y &&
               point.z > min_z && point.z < max_z;
    }

    // Returns the closest point on the bounding box to the given point.
    // If the given point is inside the bounding box, it is considered the
    // closest point (should only be called if the point is outside).
    //
    // See section 5.1.3 of Ericson's Real-Time Collision Detection.
    EGS_Vector closest_point(const EGS_Vector &point) const {
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

    EGS_Float min_interior_distance(const EGS_Vector &point) const {
            return std::min(point.x - min_x, std::min(point.y - min_y,
                            std::min(point.z - min_z, std::min(max_x - point.x,
                                     std::min(max_y - point.y, max_z - point.z)))));
        }

    // Returns 1 if there is an intersection and 0 if not. If there is an
    // intersection, the out parameter dist will be the distance along v to
    // the intersection point q.
    //
    // Adapted from Ericson section 5.3.3 "Intersecting Ray or Segment
    // Against Box".
    int ray_intersection(const EGS_Vector &p, const EGS_Vector &v, EGS_Float &dist, EGS_Vector &q) const {
        // check intersection of ray with three bounding box slabs
        EGS_Float tmin = 0.0;
        EGS_Float tmax = veryFar;
        std::array<EGS_Float, 3> p_vec {p.x, p.y, p.z};
        std::array<EGS_Float, 3> v_vec {v.x, v.y, v.z};
        std::array<EGS_Float, 3> mins {min_x, min_y, min_z};
        std::array<EGS_Float, 3> maxs {max_x, max_y, max_z};
        for (std::size_t i = 0; i < 3; i++) {
            // Parallel to slab. Point must be within slab bounds to hit
            // the bounding box
            if (std::abs(v_vec[i]) < 1e-10) {
                // Outside slab bounds
                if (p_vec[i] < mins[i] || p_vec[i] > maxs[i]) {
                    return 0;
                }
            }
            else {
                // intersect ray with slab planes
                EGS_Float inv_vel = 1.0 / v_vec[i];
                EGS_Float t1 = (mins[i] - p_vec[i]) * inv_vel;
                EGS_Float t2 = (maxs[i] - p_vec[i]) * inv_vel;
                // convention is t1 is near plane, t2 is far plane
                if (t1 > t2) {
                    std::swap(t1, t2);
                }
                tmin = std::max(tmin, t1);
                tmax = std::min(tmax, t2);
                if (tmin > tmax) {
                    return 0;
                }
            }
        }
        q = p + v * tmin;
        dist = tmin;
        return 1;
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
    bool intersects_triangle(const EGS_Vector &a, const EGS_Vector &b, const EGS_Vector &c, int e) const {
        const EGS_Float eps = 1e-30;
        if (min3(a.x, b.x, c.x) >= max_x+1e-10 ||
                min3(a.y, b.y, c.y) >= max_y+1e-10 ||
                min3(a.z, b.z, c.z) >= max_z+1e-10 ||
                max3(a.x, b.x, c.x) <= min_x-1e-10 ||
                max3(a.y, b.y, c.y) <= min_y-1e-10 ||
                max3(a.z, b.z, c.z) <= min_z-1e-10) {
            //cout<<"triangle "<<e<<" no intersect (1)::  ";
            //cout<<min_x<<" <x< "<<max_x<<"  "<<min_y<<" <y< "<<max_y<<"  "<<min_z<<" <z< "<<max_z<<endl;
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
        const EGS_Vector ux {1, 0, 0}, uy {0, 1, 0}, uz {0, 0, 1};
        const std::array<EGS_Vector, 3> unit_vecs { ux, uy, uz};
        for (const EGS_Vector &u : unit_vecs) {
            for (const EGS_Vector &f : edge_vecs) {
                const EGS_Vector a = cross(u, f);
                if (is_zero(a)) {
                    // Ignore testing this axis, likely won't be a separating
                    // axis. This may lead to false positives, but not false
                    // negatives.
                    continue;
                }
                // find box projection radius
                const EGS_Float r = ex * std::abs(dot(ux, a)) + ey * std::abs(dot(uy, a)) + ez * std::abs(dot(uz, a));
                // find three projections onto axis a
                const EGS_Float p0 = dot(v0, a);
                const EGS_Float p1 = dot(v1, a);
                const EGS_Float p2 = dot(v2, a);
                if (std::max(-max3(p0, p1, p2), min3(p0, p1, p2)) > r+ 1e-10) {
                    //cout<<"triangle "<<e<<" no intersect (2)::  ";
                    //cout<<min_x<<" <x< "<<max_x<<"  "<<min_y<<" <y< "<<max_y<<"  "<<min_z<<" <z< "<<max_z<<endl;
                    return false;
                }
            }
        }
        // category 1 - test overlap with AABB face normals
        if (max3(v0.x, v1.x, v2.x) <= -ex || min3(v0.x, v1.x, v2.x) >= ex ||
                max3(v0.y, v1.y, v2.y) <= -ey || min3(v0.y, v1.y, v2.y) >= ey ||
                max3(v0.z, v1.z, v2.z) <= -ez || min3(v0.z, v1.z, v2.z) >= ez) {
            //cout<<"triangle "<<e<<" no intersect (3)::  ";
            //cout<<min_x<<" <x< "<<max_x<<"  "<<min_y<<" <y< "<<max_y<<"  "<<min_z<<" <z< "<<max_z<<endl;
            return false;
        }

        // category 2 - test overlap with triangle face normal using AABB
        // plane test (5.2.3)

        // Cross product robustness issues are ignored here (assume
        // non-degenerate and non-oversize triangles)
        const EGS_Vector n = cross(edge_vecs[0], edge_vecs[1]);
        // projection radius
        const EGS_Float r = ex * std::abs(n.x) + ey * std::abs(n.y) + ez * std::abs(n.z);
        // distance from box centre to plane
        //
        // We have to use `a` here and not `v0` as in my printing since the
        // bounding box was not translated to the origin. This is a known
        // erratum, see http://realtimecollisiondetection.net/books/rtcd/errata/
        const EGS_Float s = dot(n, centre) - dot(n, a);
        // intersection if s falls within projection radius
        if(!(std::abs(s) <= r)){
            //cout<<"triangle "<<e<<" no intersect (4)::  ";
            //cout<<min_x<<" <x< "<<max_x<<"  "<<min_y<<" <y< "<<max_y<<"  "<<min_z<<" <z< "<<max_z<<endl;
        }
        //else cout<<min_x<<" <x< "<<max_x<<"  "<<min_y<<" <y< "<<max_y<<"  "<<min_z<<" <z< "<<max_z<<endl;

        return std::abs(s) <= r;
    }

private:
    EGS_Float min_x = 0.0;
    EGS_Float max_x = 0.0;
    EGS_Float min_y = 0.0;
    EGS_Float max_y = 0.0;
    EGS_Float min_z = 0.0;
    EGS_Float max_z = 0.0;
};

class TriNode {
public:
    std::vector<int> elts_; //list of elements within this node of the octree (i.e triangles intersecting bbox_)
    std::vector<TriNode> children_; //the octants that this node is divided into if it is not a leaf (this is empty if it is a leaf)
    EGS_TriangleMeshBbox bbox_;//region of the total bounding box represented by this octree node

    TriNode() = default;
    TriNode(const std::vector<int> &elts, const EGS_TriangleMeshBbox &bbox, std::size_t n_max, const EGS_TriangleMesh &mesh) : bbox_(bbox) {
        //cout<<"new node"<<endl;
        if (bbox_.is_indivisible() || elts.size() < n_max) {
                elts_ = elts;
                //this is then a leaf either because it cannot be further divided or has gotten its element number below the required maximum
                return;//return so no new children are produced (its children is empty is is_leaf() is true)
            }

        std::array<std::vector<int>, 8> octants;
        std::array<EGS_TriangleMeshBbox, 8> bbs = bbox_.divide8();

        // elements may be in more than one bounding box
        for (const auto &e : elts) {
            //get relevant information for triangle corresponding to element e
            const auto& xs = mesh.triangle_xs(e);
            const auto& ys = mesh.triangle_ys(e);
            const auto& zs = mesh.triangle_zs(e);
            int added = -1;
            //cout<<"new octant set (triangle "<<e<<")"<<endl;
            for (int i = 0; i < 8; i++) {
                //check if the triangle corresponding to element e intersects the bounding box of our current node
                if (bbs[i].intersects_triangle(EGS_Vector(xs[0], ys[0], zs[0]), EGS_Vector(xs[1], ys[1], zs[1]), EGS_Vector(xs[2], ys[2], zs[2]),e)) {
                    added++;
                    octants[i].push_back(e);
                }
            }
            if(added==-1){
                    cout<<"no octant intersection found for triangle "<<e<<" with points a= ("<<xs[0]<<", "<<ys[0]<<", "<<zs[0]<<")  b= ("<<xs[1]<<", "<<ys[1]<<", "<<zs[1]<<")  c= ("<<xs[2]<<", "<<ys[2]<<", "<<zs[2]<<")"<<endl;
            }
        }
        for (int i = 0; i < 8; i++) {
            children_.push_back(TriNode(std::move(octants[i]), bbs[i], n_max, mesh));
        }
    }

    bool isLeaf() const {
        return children_.empty();
    }

    int findOctant(const EGS_Vector &p) const {
        // Our choice of octant ordering (see BoundingBox.divide8) means we
        // can determine the correct octant with three checks. E.g. octant 0
        // is (-x, -y, -z), octant 1 is (+x, -y, -z), octant 4 is (-x, -y, +z)
        // octant 7 is (+x, +y, +z), etc.
        std::size_t octant = 0;
        if (p.x >= bbox_.mid_x()) {
            octant += 1;
        };
        if (p.y >= bbox_.mid_y()) {
            octant += 2;
        };
        if (p.z >= bbox_.mid_z()) {
            octant += 4;
        };
        return octant;
    }

    // Octants are returned ordered by minimum intersection distance
    std::vector<int> findOtherIntersectedOctants(const EGS_Vector &p, const EGS_Vector &v, int exclude_octant) const {
        if (isLeaf()) {
            throw std::runtime_error("findOtherIntersectedOctants called on leaf node");
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
        for (const auto &i : intersections) {
            octants.push_back(i.second);
        }
        return octants;
    }

    int howfar(const EGS_Vector &x, const EGS_Vector &u, double &min_dist, int &min_tri, const bool inside_mesh, EGS_TriangleMesh &trimesh) const {
        //leaf
        if (isLeaf()) {
            if(elts_.size()>0){
                for (const auto &i: elts_) {
                    const auto& xs = trimesh.triangle_xs(i);
                    const auto& ys = trimesh.triangle_ys(i);
                    const auto& zs = trimesh.triangle_zs(i);

                    bool outward_triangle = is_outside_of_triangle_plane(x, trimesh.triangle_normal(i), EGS_Vector(xs[0], ys[0], zs[0]));

                    // if the point is inside the mesh, skip testing all outward triangles
                    if (inside_mesh && outward_triangle) {
                        continue;
                    }
                    // if the point is outside the mesh, skip testing all inward triangles
                    if (!inside_mesh && !outward_triangle) {
                        continue;
                    }
                    // otherwise, test this triangle for intersection
                    double dist = veryFar;
                    trimesh.inctricheck_OHF();
                    if (!triangle_ray_intersection(x, u, EGS_Vector(xs[0], ys[0], zs[0]), EGS_Vector(xs[1], ys[1], zs[1]), EGS_Vector(xs[2], ys[2], zs[2]), dist)) {
                        // no intersection
                        continue;
                    }
                    // intersection, if intersection distance zero, particle is on the plane from its last step. Not true intersection so ignore it
                    if (dist>-1e-10 && dist<1e-10) { //0-1e-08 < dist && 0+1e-08 > dist
                        continue; //this fixes the floating point bug without modifying step size, as it prevents the particle from going back into the plane it just crossed. Will not count an intersection
                                //if the distance is zero as it means it previously intersected and is now sitting on the plane. Not a valid intersection.
                    }

                    // intersection, and distance is larger than zero so it is a true intersection
                    //update min_dist if smaller
                    if (dist > min_dist) {
                        continue;
                    }
                    min_dist= dist;
                    min_tri = i;
                }
            }
            return min_tri; //return min tri. Program knows there was no intersect in this octant if it is still -1
        }
        //parent
        EGS_Vector intersection;
        EGS_Float interdist;
        auto hit = bbox_.ray_intersection(x, u, interdist, intersection);
        // case 1: there's no intersection with this bounding box, return
        if (!hit) {
            return -1;
        }
        // case 2: we have a hit. Descend into the most likely intersecting
        // child octant's bounding box to find any intersecting elements
        auto octant = findOctant(intersection);
        auto elt = children_[octant].howfar(x, u, min_dist, min_tri, inside_mesh, trimesh);
            // If we find a valid element, return it
            if (elt != -1) {
                return elt;
            }
            // Otherwise, if there was no intersection in the most likely
            // octant, examine the other octants that are intersected by
            // the ray:
            for (const auto &o : findOtherIntersectedOctants(x, u, octant)) {
                auto elt = children_[o].howfar(x, u, min_dist, min_tri, inside_mesh, trimesh);
                // If we find a valid element, return it
                if (elt != -1) {
                    return elt;
                }
            }
            return -1;

    }
    //howfar end

    int isWhere(const EGS_Vector &x, const EGS_Vector &arbitrary_unit_velocity, double &min_dist_interior, double &min_dist_exterior, EGS_TriangleMesh &trimesh){
        //leaf
        if (isLeaf()) {
            int tri=-1;
            if(elts_.size()>0){
                for (const auto &i: elts_) {
                    const auto& xs = trimesh.triangle_xs(i);
                    const auto& ys = trimesh.triangle_ys(i);
                    const auto& zs = trimesh.triangle_zs(i);

                    // test for intersection
                    double dist = veryFar;
                    trimesh.inctricheck_OIW();
                    //iswhere bug has something to do with this ray intersection check
                    if (!triangle_ray_intersection(x, arbitrary_unit_velocity,
                            EGS_Vector(xs[0], ys[0], zs[0]), EGS_Vector(xs[1], ys[1], zs[1]),
                                EGS_Vector(xs[2], ys[2], zs[2]), dist))
                    {
                        // no intersection
                        continue;
                    }
                    tri=i;
                    // There's an intersection, check whether it is an inner or outer face.
                    //
                    // TODO check if adding epsilon check around 0.0 (parallel) is important here.
                    if (is_outside_of_triangle_plane(x, trimesh.triangle_normal(i), EGS_Vector(xs[0], ys[0], zs[0]))) {
                        min_dist_exterior = std::min(dist, min_dist_exterior);
                    }
                    else {
                        min_dist_interior = std::min(dist, min_dist_interior);
                    }
                }
            }
            else{
            }
            return tri;
        }
        //parent
        EGS_Vector intersection;
        EGS_Float interdist;
        auto hit = bbox_.ray_intersection(x, arbitrary_unit_velocity, interdist, intersection);
        // case 1: there's no intersection with this bounding box, return
        if (!hit) {
            return -1;
        }
        // case 2: we have a hit. Descend into the most likely intersecting
        // child octant's bounding box to find any intersecting elements
        auto octant = findOctant(intersection);
        auto elt = children_[octant].isWhere(x, arbitrary_unit_velocity, min_dist_interior, min_dist_exterior, trimesh);
            // If we find a valid element, return it
            if (elt != -1) {
                return elt;
            }
            // Otherwise, if there was no intersection in the most likely
            // octant, examine the other octants that are intersected by
            // the ray:
            for (const auto &o : findOtherIntersectedOctants(x, arbitrary_unit_velocity, octant)) {
                auto elt = children_[o].isWhere(x, arbitrary_unit_velocity, min_dist_interior, min_dist_exterior, trimesh);
                // If we find a valid element, return it
                if (elt != -1) {
                    return elt;
                }
            }
            return -1;
    }
    void hownear(const EGS_Vector &x, EGS_Float &min_t, EGS_Vector &min_point, EGS_TriangleMesh &trimesh){
        //leaf
        if (isLeaf()) {
            //min_t=distance(bbox_.closest_point(x), x);
            min_t=bbox_.min_interior_distance(x);
            EGS_Float min_t2=min_t*min_t;
            for (const auto &i: elts_) {
                const auto& xs = trimesh.triangle_xs(i);
                const auto& ys = trimesh.triangle_ys(i);
                const auto& zs = trimesh.triangle_zs(i);

                trimesh.inctricheck_OHN();
                EGS_Vector q = closest_point_triangle(x, EGS_Vector(xs[0], ys[0], zs[0]), EGS_Vector(xs[1], ys[1], zs[1]), EGS_Vector(xs[2], ys[2], zs[2]));
                EGS_Float dis2 = distance2(q, x);

                if (dis2 < min_t2) {
                    min_t2 = dis2;
                    min_point = q;
                }
            }
            min_t=std::sqrt(min_t2);
            //cout<<"octree min_t= "<<min_t<<endl;
            return;
        }
        //parent

        // Descend into the leaf octant containing the particle position
        auto octant = findOctant(x);
        children_[octant].hownear(x, min_t, min_point, trimesh);
            // If we find a valid element, return it
    }
};

class EGS_TriangleMesh_Octree {
private:
    TriNode root_;
public:
    EGS_TriangleMesh_Octree() = default;
    EGS_TriangleMesh_Octree(const std::vector<int> &elts, std::size_t n_max,
                    const EGS_TriangleMesh &mesh, EGS_TriangleMeshBbox &basebox) {
        if (elts.empty()) {
            throw std::runtime_error("EGS_Mesh_Octree: empty elements vector");
        }
        if (elts.size() > std::numeric_limits<int>::max()) {
            throw std::runtime_error("EGS_Mesh_Octree: num elts must fit into an int");
        }
        root_ = TriNode(elts, basebox, n_max, mesh);
    }

    int howfar(const EGS_Vector &x, const EGS_Vector &u, double &min_dist, const EGS_Float &max_dist, int &min_tri, const bool inside_mesh, EGS_TriangleMesh &trimesh) const {
        //cout<<"howfar"<<endl;
        EGS_Vector intersection;
        EGS_Float dist;
        auto hit = root_.bbox_.ray_intersection(x, u, dist, intersection);
        if (!hit || dist > max_dist) { //egsmesh also has maxdistance condition here ???
            return -1;
        }
        return root_.howfar(x, u, min_dist, min_tri, inside_mesh, trimesh);
    }

    int isWhere(const EGS_Vector &x, const EGS_Vector &arbitrary_unit_velocity, double &min_dist_interior, double &min_dist_exterior, EGS_TriangleMesh &trimesh){
        if (!root_.bbox_.contains(x)) {
            return -1;
        }
        return root_.isWhere(x, arbitrary_unit_velocity, min_dist_interior, min_dist_exterior, trimesh);
    }

    void hownear(const EGS_Vector &x, EGS_Float &min_t, EGS_Vector &min_point, EGS_TriangleMesh &trimesh){
        if (!root_.bbox_.contains(x)) {
            return;
        }
        root_.hownear(x, min_t, min_point, trimesh);
    }
};

// No checks are done on element validity, triangles are used as-is
EGS_TriangleMesh::EGS_TriangleMesh(EGS_TriangleMeshSpec spec, bool oct_set) :
    n_tris(spec.elements.size()), EGS_BaseGeometry(EGS_BaseGeometry::getUniqueName()),octree_acc_on(oct_set) {

    cout<<"mesh contains "<<n_tris<<" triangles"<<endl;
    // The volume bounded by the surface mesh is a single transport region.
    EGS_BaseGeometry::nreg = 1;

    // We could check for less than four elements here, since that's the
    // minimum required to make a closed 3D surface, but it seems pedantic.
    if (this->n_tris == 0) {
        throw std::runtime_error("empty triangles vector in EGS_TriangleMesh constructor");
    }

    xs.reserve(this->n_tris);
    ys.reserve(this->n_tris);
    zs.reserve(this->n_tris);
    ns.reserve(this->n_tris);

    EGS_Float bbox_min_x = veryFar;
    EGS_Float bbox_min_y = veryFar;
    EGS_Float bbox_min_z = veryFar;
    EGS_Float bbox_max_x = -veryFar;
    EGS_Float bbox_max_y = -veryFar;
    EGS_Float bbox_max_z = -veryFar;

    for (const auto& tri: spec.elements) {
        xs.push_back({tri.a.x, tri.b.x, tri.c.x});
        ys.push_back({tri.a.y, tri.b.y, tri.c.y});
        zs.push_back({tri.a.z, tri.b.z, tri.c.z});
        // TODO add check for degenerate triangle normals?
        ns.push_back(tri.n);

        bbox_min_x = std::min(bbox_min_x, std::min(tri.a.x, std::min(tri.b.x, tri.c.x)));
        bbox_min_y = std::min(bbox_min_y, std::min(tri.a.y, std::min(tri.b.y, tri.c.y)));
        bbox_min_z = std::min(bbox_min_z, std::min(tri.a.z, std::min(tri.b.z, tri.c.z)));

        bbox_max_x = std::max(bbox_max_x, std::max(tri.a.x, std::max(tri.b.x, tri.c.x)));
        bbox_max_y = std::max(bbox_max_y, std::max(tri.a.y, std::max(tri.b.y, tri.c.y)));
        bbox_max_z = std::max(bbox_max_z, std::max(tri.a.z, std::max(tri.b.z, tri.c.z)));
    }

    bbox = std::unique_ptr<EGS_TriangleMeshBbox>(new EGS_TriangleMeshBbox(
        bbox_min_x, bbox_max_x,
        bbox_min_y, bbox_max_y,
        bbox_min_z, bbox_max_z
    ));

    // expand bounding box by a small amount to avoid issues at the boundary
    bbox->expand(1e-8);
    //below here likely will be the starting point for all the octree initialization stuff
    //at this point, we have saved all the triangle vertices and normals, we have created and properly sized the bounding box, and the media has been "initialized' by the usual getinput
    //so we have essentially all we need to get started on creating the octrtee
    if(getOctBool()){
        cout<<"INITIALIZING OCTREE"<<endl;
        initializeOctree();
    }
    else{
        cout<<"SKIP OCTREE CREATION"<<endl;
    }
}

void EGS_TriangleMesh::initializeOctree(){
    std::vector<int> elts; //this tracks the indices of the triangles in the mesh for the octree to assign to octants
    //in this case there is no boundary list like the egs_mesh has as it is not relevant. There is only surface elements, no inner or outer elements

    elts.reserve(num_triangles());
    for (int i = 0; i < num_triangles(); i++) {
        elts.push_back(i);
    }
    std::size_t n_surf = 30; //the maximum number of elements allowed in a single octant (this may need to be fine tuned for best results)
    surface_tree_ = std::unique_ptr<EGS_TriangleMesh_Octree>(new EGS_TriangleMesh_Octree(elts, n_surf, *this, *bbox)); //creating the octree (in this case a surface octree i suppose but no point in differentiating)
    //note that surface tree is an attribute of the triangle mesh, hence how we will access the octree, which will access its root_, which then allows access to all of the other nodes in the tree
}

EGS_TriangleMesh::~EGS_TriangleMesh() = default;
EGS_TriangleMesh::EGS_TriangleMesh(EGS_TriangleMesh &&) = default;
EGS_TriangleMesh &EGS_TriangleMesh::operator=(EGS_TriangleMesh &&) = default;


bool EGS_TriangleMesh::isInside(const EGS_Vector &x) {
    return isWhere(x) != -1;
}

int EGS_TriangleMesh::inside(const EGS_Vector &x) {
    return isWhere(x);
}

int EGS_TriangleMesh::isWhere(const EGS_Vector &x) {
    n_hist++;
    *sortout<<"NEW HISTORY #"<<n_hist<<endl;
    *sortout<<"isWhere x=("<<x.x<<", "<<x.y<<", "<<x.z<<")::";
    // Bounding box check to avoid isWhere mesh search
    //cout<<"iswhere"<<endl;
    EGS_Float xo_dist=distance(x, EGS_Vector(0,0,0));
    //cout<<"trimesh iswhere:: dist="<<xo_dist;
    if (!bbox->contains(x)) {
        *sortout<<" Out bbox"<<endl;
        return -1;
    }

    // Pick an arbitrary direction vector, then loop over all elements, testing
    // for intersection. Compute the minimum distances to an interior face and
    // an exterior face. If the interior face is closer than the exterior face,
    // point `x` is inside the mesh (region 0), otherwise it is outside (-1).

    const double FRAC_1_SQRT_3 = 0.57735026919;
    const EGS_Vector arbitrary_unit_velocity(FRAC_1_SQRT_3, FRAC_1_SQRT_3, FRAC_1_SQRT_3);

    double min_dist_interior = veryFar;
    double min_dist_exterior = veryFar;
    if(octree_acc_on){
        surface_tree_->isWhere(x, arbitrary_unit_velocity, min_dist_interior, min_dist_exterior, *this);
    }
    else{
        for (int i = 0; i < num_triangles(); i++) {
            const auto& xs = triangle_xs(i);
            const auto& ys = triangle_ys(i);
            const auto& zs = triangle_zs(i);

            // test for intersection
            double dist = veryFar;
            inctricheck_NIW();
            if (!triangle_ray_intersection(x, arbitrary_unit_velocity,
                    EGS_Vector(xs[0], ys[0], zs[0]), EGS_Vector(xs[1], ys[1], zs[1]),
                        EGS_Vector(xs[2], ys[2], zs[2]), dist))
            {
                // no intersection
                continue;
            }

            // There's an intersection, check whether it is an inner or outer face.
            //
            // TODO check if adding epsilon check around 0.0 (parallel) is important here.
            if (is_outside_of_triangle_plane(x, triangle_normal(i), EGS_Vector(xs[0], ys[0], zs[0]))) {
                min_dist_exterior = std::min(dist, min_dist_exterior);
            }
            else {
                min_dist_interior = std::min(dist, min_dist_interior);
            }
        }
    }
    // If there were no intersections found, we must have been outside of the
    // mesh (ignoring watertightness issues at mesh corners and edges if the
    // point is inside of the mesh).
    if (min_dist_interior == veryFar && min_dist_exterior == veryFar) {
        *sortout<<" Outside"<<endl;
        return -1;
    }
    // If the closest exterior face is closer than the closest interior face,
    // we are outside the mesh. Less-or-equal (<=) is needed as points outside
    // the mesh can be intersect both inner and outer faces at corners at the
    // same distance.
    if (min_dist_exterior <= min_dist_interior) {
        //cout<<"closest triangle = "<<exmintri<<" (ext)"<<endl;
        //cout<<" point is outside mesh"<<endl;
        *sortout<<" Outside"<<endl;
        return -1;
    }
    // Otherwise, we must be inside the region bounded by the mesh.
    //cout<<"closest triangle = "<<inmintri<<" (int)"<<endl;
    //cout<<" point is inside mesh"<<endl;
    *sortout<<" Inside"<<endl;
    return 0;
}

EGS_Float EGS_TriangleMesh::hownear(int ireg, const EGS_Vector &x) {
    // Bounding box check to avoid full mesh search.
    *sortout<<"hownear ireg = "<<ireg<<" x=("<<x.x<<", "<<x.y<<", "<<x.z<<")::";
    //
    // If the point is outside the mesh bounding box, the HOWNEAR spec allows
    // for returning a lower bound, which in this case is the minimum distance
    // to the bounding box.
    if (ireg == -1 && !bbox->contains(x)) { //if (ireg == -1 && !bbox->contains(x))
        // TODO test potential performance improvement by calculating the
        // distance explicitly without finding the closest point.
        *sortout<<" boxmin= "<<distance(bbox->closest_point(x), x)<<endl;
        return distance(bbox->closest_point(x), x);
    }

    // naive impl: loop over all elements, finding the global minimum distance

    EGS_Vector min_point = x;
    EGS_Float min_t = veryFar;

    if(octree_acc_on){
        surface_tree_->hownear(x, min_t, min_point, *this);
    }
    else{
        for (int i = 0; i < num_triangles(); i++) {
            const auto& xs = triangle_xs(i);
            const auto& ys = triangle_ys(i);
            const auto& zs = triangle_zs(i);

             inctricheck_NHN();
             EGS_Vector q = closest_point_triangle(x, EGS_Vector(xs[0], ys[0], zs[0]), EGS_Vector(xs[1], ys[1], zs[1]), EGS_Vector(xs[2], ys[2], zs[2]));
             EGS_Float dis = distance(q, x);
             if (dis < min_t) {
                 min_t = dis;
                 min_point = q;
             }
        }
        //cout<<"naive min_t= "<<min_t<<endl;
    }
    *sortout<<" min_t= "<<min_t<<endl;
    return min_t;
}

int EGS_TriangleMesh::howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
                             EGS_Float &t, int *newmed, EGS_Vector *normal) {
    //cout<<"new howfar call (trimesh)"<<endl;
    // If the particle doesn't intersect the mesh bounding box, it can't intersect the mesh.
    *sortout<<"howfar ireg= "<<ireg<<" t= "<<t<<" x=("<<x.x<<", "<<x.y<<", "<<x.z<<")  u=("<<u.x<<", "<<u.y<<", "<<u.z<<") || ";
    EGS_Vector intersection;
    EGS_Float dist;
    if (!bbox->ray_intersection(x, u, dist, intersection)) {
        //cout<<"trimesh::debugging howfar (1): "<<-1<<endl;
        *sortout<<"(1) freg= "<<-1<<endl;
        return -1;
    }

    // The old implementation would Loop over all elements, testing for intersection.
    // In order to accelerate this process, the octree is used, first we determine
    // which octants the ray will intersect, and check only the triangles contained in these octants.

    //If the point is outside the mesh, only outward facing triangles are tested. Otherwise
    //if the point is inside the mesh, only inward facing triangles are tested.
    double min_dist = veryFar;
    int min_tri = -1;
    const bool inside_mesh = ireg != -1;
    if(inside_mesh) *sortout<<"inside mesh::";
    else *sortout<<"outside mesh::";

    if(octree_acc_on){
        surface_tree_->howfar(x, u, min_dist, t, min_tri, inside_mesh, *this);
    }
    else{
        for (int i = 0; i < num_triangles(); i++) {
            const auto& xs = triangle_xs(i);
            const auto& ys = triangle_ys(i);
            const auto& zs = triangle_zs(i);

            bool outward_triangle = is_outside_of_triangle_plane(x, triangle_normal(i), EGS_Vector(xs[0], ys[0], zs[0]));

            // if the point is inside the mesh, skip testing all outward triangles
            if (inside_mesh && outward_triangle) {
                continue;
            }
            // if the point is outside the mesh, skip testing all inward triangles
            if (!inside_mesh && !outward_triangle) {
                continue;
            }
            // otherwise, test this triangle for intersection
            double dist = veryFar;
            inctricheck_NHF();
            if (!triangle_ray_intersection(x, u, EGS_Vector(xs[0], ys[0], zs[0]),
                EGS_Vector(xs[1], ys[1], zs[1]), EGS_Vector(xs[2], ys[2], zs[2]), dist))
            {
                // no intersection
                continue;
            }
            // intersection, if intersection distance zero, particle is on the plane from its last step. Not true intersection so ignore it
            if (dist>-1e-10 && dist<1e-10) {
                continue; //this fixes the floating point bug without modifying step size, as it prevents the particle from going back into the plane it just crossed. Will not count an intersection
                        //if the distance is zero as it means it previously intersected and is now sitting on the plane. Not a valid intersection.
            }
            /* bug was that outward_triangle would not differentiate between truly in front of the plane and on the plane itself. Because of this particles would exit geometry,
            * re-enter via the same triangle (min dist would be zero), and once "inside" undefined behaviour would follow causing the particle to continue moving "inside mesh" while
            * travelling outside of the mesh geometry due to particle direction. Requiring the intersection distance be larger than zero makes it clear where the particle is relative
            * to the surface mesh and prevents invalid intersections which yield unphysical results */

            // intersection, and distance is larger than zero so it is a true intersection
            //update min_dist if smaller
            if (dist > min_dist) {
                continue;
            }
            min_dist= dist;
            min_tri = i;
        }
    }

    *sortout<<"Min dist= "<<min_dist<<" ";
    //cout<<"Min dist="<<min_dist<<" with triangle "<<min_tri<<endl;
    if (min_dist >= t) {
        //cout<<"trimesh::debugging howfar (2): "<<ireg<<endl;
        *sortout<<"(2) freg= "<<ireg<<endl;
        return ireg;
    }

    // otherwise, if min_dist is smaller than t, update out parameters

    if (newmed) {
        if (inside_mesh) {
            // new medium is outside the mesh (-1)
            *newmed = -1;
        } else /* outside_mesh */ {
            // new medium is the mesh medium
            *newmed = EGS_BaseGeometry::med;
        }
    }

    if (normal) {
        // egs++ convention is normal pointing opposite view ray
        if (triangle_normal(min_tri) * u > 0.0) {
            *normal = -1.0 * triangle_normal(min_tri);
        }
        else {
            *normal = triangle_normal(min_tri);
        }
    }

    t = min_dist;
    if (inside_mesh) {
        //cout<<"trimesh::debugging howfar (3): "<<-1<<endl;
        *sortout<<"(3) freg= "<<-1<<endl;
        return -1; // new region is outside the mesh
    }
    // outside mesh, new region is inside the mesh
    //cout<<"trimesh::debugging howfar (4): "<<0<<endl;
    *sortout<<"(4) freg= "<<0<<endl;
    return 0;
}

const std::string EGS_TriangleMesh::type = "EGS_TriangleMesh";

extern "C" {

    EGS_TRIANGLE_MESH_EXPORT EGS_BaseGeometry *createGeometry(EGS_Input *input) {
        if (!input) {
            egsWarning("createGeometry(EGS_TriangleMesh): null input?\n");
            return nullptr;
        }
        std::string mesh_file;
        int err = input->getInput("file", mesh_file);
        if (err) {
            egsWarning("createGeometry(EGS_TriangleMesh): no mesh file key `file` in input\n");
            return nullptr;
        }

        bool ends_with_stl = mesh_file.size() >= 3 &&
            mesh_file.compare(mesh_file.size() - 3, 3, "stl") == 0;
        if (!ends_with_stl) {
            throw std::runtime_error("unknown extension for triangle mesh file `"
                             + mesh_file + "`, only STL files are supported");
        }

        EGS_TriangleMeshSpec mesh_spec;
        try {
            mesh_spec = stl_parser::parse_stl_file(mesh_file);
        }
        catch (const std::runtime_error &e) {
            std::string error_msg = std::string("createGeometry(EGS_TriangleMesh): ") +
                                    e.what() + "\n";
            egsWarning("\n%s", error_msg.c_str());
            return nullptr;
        }

        EGS_Float scale = 0.0;
        err = input->getInput("scale", scale);
        if (!err) {
            if (scale > 0.0) {
                mesh_spec.scale(scale);
            }
            else {
                egsFatal("createGeometry(EGS_TriangleMesh): invalid scale value (%g), "
                         "expected a positive number\n", scale);
            }
        }

        vector<string> oct_options;
        oct_options.push_back("no");
        oct_options.push_back("yes");
        bool oct_set = input->getInput("octree accelerate",oct_options,false); //here false in argument makes time inclusion false by default
        if(oct_set){
            cout<<"code will be octree accelerated"<<endl;
        }
        else{
            cout<<"naive approach will be employed"<<endl;
        }

        EGS_TriangleMesh *result = nullptr;
        try {
            result = new EGS_TriangleMesh(std::move(mesh_spec),oct_set);
        }
        catch (const std::runtime_error &e) {
            std::string error_msg = std::string("createGeometry(EGS_TriangleMesh): ") +
                                    "bad input to EGS_TriangleMesh\nerror: " + e.what() + "\n";
            egsWarning("\n%s", error_msg.c_str());
            return nullptr;
        }

        result->setName(input);
        result->setMedia(input);
        result->setLabels(input);
        result->debugtool();
        return result;
    }
}
