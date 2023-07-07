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

    // Returns 1 if there is an intersection and 0 if not. If there is an
    // intersection, the out parameter dist will be the distance along v to
    // the intersection point q.
    //
    // Adapted from Ericson section 5.3.3 "Intersecting Ray or Segment
    // Against Box".
    int ray_intersection(const EGS_Vector &p, const EGS_Vector &v) const {
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
        //q = p + v * tmin;
        //dist = tmin;
        return 1;
    }

private:
    EGS_Float min_x = 0.0;
    EGS_Float max_x = 0.0;
    EGS_Float min_y = 0.0;
    EGS_Float max_y = 0.0;
    EGS_Float min_z = 0.0;
    EGS_Float max_z = 0.0;
};

class EGS_TriangleMeshNode {
public:

}

class EGS_TriangleMesh_Octree {
private:

};

// No checks are done on element validity, triangles are used as-is
EGS_TriangleMesh::EGS_TriangleMesh(EGS_TriangleMeshSpec spec) :
    n_tris(spec.elements.size()), EGS_BaseGeometry(EGS_BaseGeometry::getUniqueName()) {

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
}

EGS_TriangleMesh::~EGS_TriangleMesh() = default;
EGS_TriangleMesh::EGS_TriangleMesh(EGS_TriangleMesh &&) = default;
EGS_TriangleMesh &EGS_TriangleMesh::operator=(EGS_TriangleMesh &&) = default;

namespace { // anonymous namespace for low-level geometry routines

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

bool EGS_TriangleMesh::isInside(const EGS_Vector &x) {
    return isWhere(x) != -1;
}

int EGS_TriangleMesh::inside(const EGS_Vector &x) {
    return isWhere(x);
}

int EGS_TriangleMesh::isWhere(const EGS_Vector &x) {

    // Bounding box check to avoid isWhere mesh search
    if (!bbox->contains(x)) {
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

    for (int i = 0; i < num_triangles(); i++) {
        const auto& xs = triangle_xs(i);
        const auto& ys = triangle_ys(i);
        const auto& zs = triangle_zs(i);

        // test for intersection
        double dist = veryFar;
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
        } else {
            min_dist_interior = std::min(dist, min_dist_interior);
        }
    }
    // If there were no intersections found, we must have been outside of the
    // mesh (ignoring watertightness issues at mesh corners and edges if the
    // point is inside of the mesh).
    if (min_dist_interior == veryFar && min_dist_exterior == veryFar) {
        return -1;
    }
    // If the closest exterior face is closer than the closest interior face,
    // we are outside the mesh. Less-or-equal (<=) is needed as points outside
    // the mesh can be intersect both inner and outer faces at corners at the
    // same distance.
    if (min_dist_exterior <= min_dist_interior) {
        return -1;
    }
    // Otherwise, we must be inside the region bounded by the mesh.
    return 0;
}

EGS_Float EGS_TriangleMesh::hownear(int ireg, const EGS_Vector &x) {

    // Bounding box check to avoid full mesh search.
    //
    // If the point is outside the mesh bounding box, the HOWNEAR spec allows
    // for returning a lower bound, which in this case is the minimum distance
    // to the bounding box.
    if (ireg == -1 && !bbox->contains(x)) {
        // TODO test potential performance improvement by calculating the
        // distance explicitly without finding the closest point.
        return distance(bbox->closest_point(x), x);
    }

    // naive impl: loop over all elements, finding the global minimum distance

    EGS_Vector min_point = x;
    EGS_Float min_t = veryFar;

    auto maybe_update_min_point = [&](const EGS_Vector& A, const EGS_Vector& B, const EGS_Vector& C) {
        EGS_Vector q = closest_point_triangle(x, A, B, C);
        EGS_Float dis = distance(q, x);
        if (dis < min_t) {
            min_t = dis;
            min_point = q;
        }
    };

    for (int i = 0; i < num_triangles(); i++) {
        const auto& xs = triangle_xs(i);
        const auto& ys = triangle_ys(i);
        const auto& zs = triangle_zs(i);

        maybe_update_min_point(
            EGS_Vector(xs[0], ys[0], zs[0]),
            EGS_Vector(xs[1], ys[1], zs[1]),
            EGS_Vector(xs[2], ys[2], zs[2])
        );
    }

    return min_t;
}

int EGS_TriangleMesh::howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
                             EGS_Float &t, int *newmed, EGS_Vector *normal) {

    // If the particle doesn't intersect the mesh bounding box, it can't
    // intersect the mesh.
    if (!bbox->ray_intersection(x, u)) {
        return -1;
    }

    // Loop over all elements, testing for intersection. If the point is outside
    // the mesh, only outward facing triangles are tested. Otherwise if the
    // point is inside the mesh, only inward facing triangles are tested.
    double min_dist = veryFar;
    int min_tri = -1;
    const bool inside_mesh = ireg != -1;

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
        if (!triangle_ray_intersection(x, u, EGS_Vector(xs[0], ys[0], zs[0]),
            EGS_Vector(xs[1], ys[1], zs[1]), EGS_Vector(xs[2], ys[2], zs[2]), dist))
        {
            // no intersection
            continue;
        }
        // intersection, update min_dist if smaller
        if (dist > min_dist) {
            continue;
        }
        min_dist= dist+boundaryTolerance; //add the small boundaryTolerance here to avoid floating point bug

        /* bug was that outward_triangle would not differentiate between truly in front of the plane and on the plane itself. Because of this particles would exit geometry,
         * re-enter via the same triangle (min dist would be zero), and once "inside" undefined behaviour would follow causing the particle to continue moving "inside mesh" while
         * travelling outside of the mesh geometry due to particle direction. This little push makes it clear where the particle is relative to the surface mesh and prevents
         * false positive intersections which yield unphysical results */
        //could try to modify is_outside_of_triangle_plane method but this would be much more complex. Would have to check particle direction and triangle normal to decide what is happening

        min_tri = i;
    }

    if (min_dist >= t) {
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
        return -1; // new region is outside the mesh
    }
    // outside mesh, new region is inside the mesh
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

        EGS_TriangleMesh *result = nullptr;
        try {
            result = new EGS_TriangleMesh(std::move(mesh_spec));
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
        return result;
    }
}
