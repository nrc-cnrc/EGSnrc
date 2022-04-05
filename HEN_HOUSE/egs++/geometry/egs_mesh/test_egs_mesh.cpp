/*
###############################################################################
#
#  EGSnrc egs_mesh test suite
#
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
#  Author:          Max Orok, 2022
#
###############################################################################
*/

// The test suite has four main parts:
//
// * egs_mesh tests
// * mesh_neighbours
// * msh_parser
// * tetgen_parser
//
// TODO add test for intersection point right on element node

#include "egs_input.h"
#include "egs_mesh.h"
#include "mesh_neighbours.h"
#include "msh_parser.h"
#include "tetgen_parser.h"

#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

// test runner globals
int num_total = 0;
int num_failed = 0;

#define RUN_TEST(test_fn) \
    std::cerr << "test " << #test_fn << "... "; \
    num_total++; \
    try { \
        test_fn; \
        std::cerr << "ok\n"; \
    } catch (const std::runtime_error& err) { \
        num_failed++; \
        std::cerr << "FAILED: " << err.what() << "\n"; \
    }

#define EXPECT_ERROR(stmt, err_msg) \
    try { \
        stmt; \
        std::ostringstream oss; \
        oss << "expected exception with message: \"" << err_msg << "\""; \
        throw std::runtime_error(oss.str()); \
    } catch (const std::exception& err) { \
        if (err.what() != std::string(err_msg)) { \
            std::ostringstream oss; \
            oss << "got error message: \"" \
                << err.what() << "\" but expected: \"" << err_msg << "\""; \
            throw std::runtime_error(oss.str()); \
        } \
    }

static bool approx_eq(double a, double b, double e = 1e-8) {
    return (std::abs(a - b) <= e * (std::abs(a) + std::abs(b) + 1.0));
}

// Floating point values must match exactly, no approximate equality is used.
static bool egsvec_eq(EGS_Vector x, EGS_Vector y) {
    return x.x == y.x && x.y == y.y && x.z == y.z;
}

static std::string to_string_with_precision(double d, const int n = 17)
{
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << d;
    return out.str();
}

// RAII class for a temporary file used by some tests
class TempFile {
public:
    TempFile(const std::string& filename, const std::string& contents)
        : filename_(filename)
    {
        {
            std::ofstream out(filename_);
            out << contents;
        }
    }

    ~TempFile() {
        std::remove(this->filename_.c_str());
    }

private:
    std::string filename_;
};

// String for a five element Gmsh 4.1 msh file
static std::string five_elt_mesh_str =
R"($MeshFormat
4.1 0 8
$EndMeshFormat
$Entities
0 0 0 1
1 0 0 0 0 0 0 1 1 0
$EndEntities
$PhysicalNames
1
3 1 "H2O"
$EndPhysicalNames
$Nodes
1 8 1 8
3 1 0 8
1
2
3
4
5
6
7
8
0 0 0
1 0 0
0 1 0
0 0 1
0 -1 0
-1 0 0
0 0 -1
1 1 1
$EndNodes
$Elements
1 5 1 5
3 1 4 5
1 1 2 3 4
2 1 2 4 5
3 1 3 4 6
4 1 2 3 7
5 2 3 4 8
$EndElements)";

// We'll use a simple five-element mesh for smoke testing
//
// Not declared const because of EGS_BaseGeometry method requirements but
// isn't mutated at any point.
static EGS_Mesh test_mesh = [](){
    std::stringstream input(five_elt_mesh_str);
    return EGS_Mesh(msh_parser::parse_msh_file(input));
}();

// egs_mesh tests

static void test_unknown_node() {
    std::vector<EGS_MeshSpec::Tetrahedron> elt { EGS_MeshSpec::Tetrahedron(1, 0, 0, 1, 2, 100) };
    // no node 100 in nodes vector
    std::vector<EGS_MeshSpec::Node> nodes {
        EGS_MeshSpec::Node(0, 1.0, 1.0, -1.0),
        EGS_MeshSpec::Node(1, -1.0, 1.0, -1.0),
        EGS_MeshSpec::Node(2, 0.0, -1.0, -1.0),
        EGS_MeshSpec::Node(3, 0.0, 0.0, 1.0)
    };
    std::vector<EGS_MeshSpec::Medium> media { EGS_MeshSpec::Medium(1, "") };
    EXPECT_ERROR(EGS_Mesh(EGS_MeshSpec(elt, nodes, media)), "No mesh node with tag: 100");
}

static void test_boundary() {
    // element 0 is surrounded by the other four elements
    if (test_mesh.is_boundary(0)) {
        throw std::runtime_error("expected region 0 not to be a surface element");
    }
    for (auto i = 1; i < test_mesh.num_elements(); i++) {
        if (!test_mesh.is_boundary(i)) {
            throw std::runtime_error("expected region " + std::to_string(i) +
                " to be a surface element");
        }
    }
}

static void test_neighbours() {
    // element 0 is neighbours with the other four elements
    auto n0 = test_mesh.element_neighbours(0);
    for (int i = 1; i <= 4; i++) {
        if (std::count(n0.begin(), n0.end(), i) != 1) {
            throw std::runtime_error("expected region " + std::to_string(i) +
                " to be a neighbour of region 0");
        }
    }

    for (int i = 1; i <= 4; i++) {
        auto ns = test_mesh.element_neighbours(i);
        if (std::count(ns.begin(), ns.end(), 0) != 1) {
            throw std::runtime_error("expected region " + std::to_string(i) +
                " to be a neighbour of region 0");
        }
        if (std::count(ns.begin(), ns.end(), -1) != 3) {
            throw std::runtime_error("expected region " + std::to_string(i) +
                " to have three surface faces");
        }
    }
}

class Tet {
public:
    Tet(EGS_Vector a, EGS_Vector b, EGS_Vector c, EGS_Vector d)
        : a(a), b(b), c(c), d(d) {}
    EGS_Vector centroid() const {
        return EGS_Vector (
            (a.x + b.x + c.x + d.x) / 4.0,
            (a.y + b.y + c.y + d.y) / 4.0,
            (a.z + b.z + c.z + d.z) / 4.0
        );
    }
private:
    EGS_Vector a;
    EGS_Vector b;
    EGS_Vector c;
    EGS_Vector d;
};

static std::vector<Tet> get_tetrahedrons(const EGS_Mesh& mesh) {
    std::vector<Tet> elts;
    elts.reserve(mesh.num_elements());
    for (auto i = 0; i < mesh.num_elements(); i++) {
        const auto elt_nodes = mesh.element_nodes(i);
        elts.emplace_back(Tet(elt_nodes.A, elt_nodes.B, elt_nodes.C, elt_nodes.D));
    }
    return elts;
}

static void test_isWhere() {
    // test the centroid of each tetrahedron is inside the tetrahedron
    auto elts = get_tetrahedrons(test_mesh);
    for (int i = 0; i < (int)elts.size(); i++) {
        auto c = elts.at(i).centroid();
        auto in_tet = test_mesh.isWhere(c);
        if (in_tet != i) {
            throw std::runtime_error("expected point in tetrahedron " +
                std::to_string(i) + " got: " + std::to_string(in_tet));
        }
    }
    EGS_Vector out(1e10, 0, 0);
    if (test_mesh.isWhere(out) != -1) {
        throw std::runtime_error("expected point to be outside (-1), got: " +
            std::to_string(test_mesh.isWhere(out)));
    }
}

static void test_hownear_interior() {
    auto elts = get_tetrahedrons(test_mesh);
    for (int i = 0; i < (int)elts.size(); i++) {
        auto c = elts.at(i).centroid();
        auto dist = test_mesh.hownear(i, c);
        if (i < 4 && !approx_eq(dist, 0.144338, 1e-6)) {
            throw std::runtime_error(
                "expected min distance to be 0.144338, got: " +
                    std::to_string(dist));
        } else if (i == 4 && !approx_eq(dist, 0.288675, 1e-6)) {
            throw std::runtime_error(
                "expected min distance to be 0.288675, got: " +
                    std::to_string(dist));
        } else if (i > 5) {
            // test specific to five-tet.msh
            throw std::runtime_error(
                "unknown mesh file for test_hownear_interior");
        }
    }
}

static void test_hownear_exterior() {
    // known point 1.0 away from tetrahedron 1 with point (0.0, -1.0, 0.0)
    {
        EGS_Vector x(0.0, -2.0, 0.0);
        auto dist = test_mesh.hownear(-1, x);
        if (!approx_eq(1.0, dist)) {
            throw std::runtime_error("expected min distance to be 1.0, got: "
                + std::to_string(dist));
        }
    }
    // known point 10.0 away from tetrahedron 3 with point (0.0, 0.0, -1.0)
    {
        EGS_Vector x(0.0, 0.0, -11.0);
        auto dist = test_mesh.hownear(-1, x);
        if (!approx_eq(10.0, dist)) {
            throw std::runtime_error("expected min distance to be 10.0, got: "
                + std::to_string(dist));
        }
    }
}

static void test_medium() {
    for (auto i = 0; i < test_mesh.num_elements(); i++) {
        if (0 != test_mesh.medium(i)) {
            throw std::runtime_error("expected medium index to be 0, got: "
                + std::to_string(test_mesh.medium(i)));
        }
    }
}

static void test_howfar_interior_basic() {
    {
    // Element 3 (ireg = 3-1 = 2) of the test mesh has a boundary face at x = 0
    // For a point inside element 3, the distance along the x-axis to a boundary face is -p.x
    // and the new region index is 0 (element 1)
        auto reg = 2;
        const EGS_Vector p(-0.1, 0.1, 0.1);
        const EGS_Vector u(1.0, 0.0, 0.0);
        auto dist = 1e20;
        int newmed = -100;
        auto new_reg = test_mesh.howfar(reg, p, u, dist, &newmed);
        if (new_reg != 0) {
            throw std::runtime_error("expected new region index to be 0, got: "
                + std::to_string(new_reg));
        }
        if (!approx_eq(dist, 0.1)) {
            throw std::runtime_error("expected distance to be 0.1, got: " +
                std::to_string(dist));
        }
        if (newmed != 0) {
            throw std::runtime_error("expected medium index to be 0, got: " +
                std::to_string(newmed));
        }
    }
    {
    // Element 3 (ireg = 3-1 = 2) of the test mesh has an outside boundary face at y = 0
    // For a point inside element 3, the distance along the y-axis to a boundary face is -p.y
    // and the new region index is -1 (vacuum)
        auto reg = 2;
        const EGS_Vector p(-0.1, 0.1, 0.1);
        const EGS_Vector u(0.0, -1.0, 0.0);
        auto dist = 1e20;
        int newmed = -100;
        auto new_reg = test_mesh.howfar(reg, p, u, dist, &newmed);
        if (new_reg != -1) {
            throw std::runtime_error("expected new region index to be -1, got: "
                + std::to_string(new_reg));
        }
        if (!approx_eq(dist, 0.1)) {
            throw std::runtime_error("expected distance to be 0.1, got: " +
                std::to_string(dist));
        }
        if (newmed != -1) {
            throw std::runtime_error("expected medium index to be -1, got: " +
                std::to_string(newmed));
        }
    }
    {
    // Element 1 (ireg = 0) of the test mesh has a boundary face at z = 0 with elt 4
        auto reg = 0;
        const EGS_Vector p(0.1, 0.1, 0.3);
        const EGS_Vector u(0.0, 0.0, -1.0);
        auto dist = 1e20;
        int newmed = -100;
        auto new_reg = test_mesh.howfar(reg, p, u, dist, &newmed);
        if (new_reg != 3) {
            throw std::runtime_error("expected new region index to be 3, got: "
                + std::to_string(new_reg));
        }
        if (!approx_eq(dist, 0.3)) {
            throw std::runtime_error("expected distance to be 0.3, got: " +
                std::to_string(dist));
        }
        if (newmed != 0) {
            throw std::runtime_error("expected medium index to be 0, got: " +
                std::to_string(newmed));
        }
    }
}

static void test_howfar_exterior() {
    {
    // Element 3 (ireg = 2) has an exterior boundary with a point at x = -1
        auto reg = -1; // outside the mesh
        const EGS_Vector p(-2.0, 0.0, 0.0);
        const EGS_Vector u(1.0, 0.0, 0.0);
        auto dist = 1e20;
        int newmed = -100;
        auto new_reg = test_mesh.howfar(reg, p, u, dist, &newmed);
        if (new_reg != 2) {
            throw std::runtime_error("expected new region index to be 2, got: "
                + std::to_string(new_reg));
        }
        if (!approx_eq(dist, 1.0)) {
            throw std::runtime_error("expected distance to be 1.0, got: " +
                std::to_string(dist));
        }
        if (newmed != 0) {
            throw std::runtime_error("expected medium index to be 0, got: " +
                std::to_string(newmed));
        }
    }
}

// Test the egsinp `scale` key.
static void test_mesh_scaling() {
    std::string filename = "tmp_test_mesh_scaling.msh";
    TempFile tmp(filename, five_elt_mesh_str);

    EGS_Input egsinp;
    std::string egsinp_str(
        ":start geometry definition:\n"
        "    :start geometry:\n"
        "       name = my_mesh\n"
        "       library = egs_mesh\n"
        "       file = " + filename + "\n"
        "       scale = 0.1\n" // mm to cm
        "    :stop geometry:\n"
        "    simulation geometry = my_mesh\n"
        ":stop geometry definition:\n"
    );
    egsinp.setContentFromString(egsinp_str);
    EGS_BaseGeometry *geo = EGS_Mesh::createGeometry(&egsinp);
    EGS_Mesh *scaled_mesh = dynamic_cast<EGS_Mesh*>(geo);
    if (!scaled_mesh) {
        throw std::runtime_error("dynamic_cast<EGS_Mesh*> failed!");
    }

    auto mesh_volume = [](const EGS_Mesh& mesh) -> EGS_Float {
        EGS_Float vol = 0.0;
        for (int i = 0; i < mesh.num_elements(); i++) {
            vol += mesh.element_volume(i);
        }
        return vol;
    };

    auto scaled_vol = mesh_volume(*scaled_mesh);
    auto vol = mesh_volume(test_mesh) / 1000.0; // 10^3
    if (!approx_eq(scaled_vol, vol)) {
        throw std::runtime_error("scaled volume " + std::to_string(scaled_vol) +
            ") != expected (" + std::to_string(vol) + ")");
    }
    delete geo;
}

// Custom egsInfoFunction that throws error messages as exceptions for testing
void egsInfoThrowing(const char *msg, ...) {
    char buf[8192];
    va_list ap;
    va_start(ap, msg);
    vsprintf(buf, msg, ap);
    va_end(ap);
    throw std::runtime_error(buf); // buf copied by constructor
}

// Test egsinp `scale` key errors.
static void test_mesh_scale_key_errors() {
    egsSetInfoFunction(Fatal, egsInfoThrowing);

    std::string filename = "tmp_test_mesh_scaling.msh";
    TempFile tmp(filename, five_elt_mesh_str);

    EGS_Input egsinp;
    std::string egsinp_str(
        ":start geometry definition:\n"
        "    :start geometry:\n"
        "       name = my_mesh\n"
        "       library = egs_mesh\n"
        "       file = " + filename + "\n"
        "       scale = -0.1\n" // invalid negative scale value
        "    :stop geometry:\n"
        "    simulation geometry = my_mesh\n"
        ":stop geometry definition:\n"
    );
    egsinp.setContentFromString(egsinp_str);
    EXPECT_ERROR(EGS_Mesh::createGeometry(&egsinp),
        "createGeometry(EGS_Mesh): invalid scale value (-0.1), "
        "expected a positive number\n");

    // Reset egsFatal
    egsSetDefaultIOFunctions();
}

// Test the basic howfar_interior implementation
//        __________
//       /\        /
//      /  \      /
//     /    \    /
//    / * -> X  /
//   /________\/
//
static void test_howfar_interior_regular(EGS_Mesh& mesh) {
    EGS_Vector x(0.5, 0.5, 0.0);
    EGS_Vector u(1.0, 0.0, 0.0);
    EGS_Float dist = veryFar;
    int newmed = -1;
    auto newreg = mesh.howfar(0, x, u, dist, &newmed);
    if (newreg != 1) {
        throw std::runtime_error(std::string("expected newreg = 1, got ") +
            std::to_string(newreg));
    }
    if (!approx_eq(dist, 0.5)) {
        throw std::runtime_error(std::string("expected dist = 0.5, got ") +
            to_string_with_precision(dist));
    }
    if (newmed != 0) {
        throw std::runtime_error(std::string("expected newmed = 0, got ") +
            std::to_string(newmed));
    }
}

// Test the howfar_interior implementation for particles too far away to be in
// a thick plane. The mesh should detect this case and return the actual
// region.
//           __________
//          /\        /
//         /  \      /
//        /    \    /
// * ->  /      X  /
//      /________\/
//
// /---/
//   d = distance along plane normal to plane >> EGS_Mesh::thick_plane_tolerance
//
static void test_howfar_interior_outside_thick_plane(EGS_Mesh& mesh) {
    EGS_Vector x(-0.5, 0.5, 0.0);
    EGS_Vector u(1.0, 0.0, 0.0);
    EGS_Float dist = veryFar;
    int newmed = -1;
    auto newreg = mesh.howfar(0, x, u, dist, &newmed);
    if (newreg != -1) {
        throw std::runtime_error(std::string("expected newreg = -1, got ") +
            std::to_string(newreg));
    }
    if (!approx_eq(dist, EGS_Mesh::get_min_step_size())) {
        throw std::runtime_error(std::string("expected dist = 1e-10, got ") +
            to_string_with_precision(dist));
    }
    if (newmed != -1) {
        throw std::runtime_error(std::string("expected newmed = 0, got ") +
            std::to_string(newmed));
    }
}

// Test how the howfar_interior implementation deals with a "lost" particle
//           __________
//          /\        /
//         /  \      /
//        /    \    /
// <- *  /      \  /
//      /________\/
//
static void test_howfar_interior_lost_particle(EGS_Mesh &mesh) {
    EGS_Vector x(-0.5, 0.5, 0.0);
    EGS_Vector u(-1.0, 0.0, 0.0);
    EGS_Float dist = veryFar;
    int newmed = -1;
    auto newreg = mesh.howfar(0, x, u, dist, &newmed);
    if (newreg != -1) {
        throw std::runtime_error(std::string("expected newreg = -1, got ") +
            std::to_string(newreg));
    }
    if (dist != EGS_Mesh::get_min_step_size()) {
        throw std::runtime_error(std::string("expected dist = 1e-10, got ") +
            to_string_with_precision(dist));
    }
    if (newmed != -1) {
        throw std::runtime_error(std::string("expected newmed = -1, got ") +
            std::to_string(newmed));
    }
}

// Test for a problematic case: particle exiting the geometry, but on the next
// step considered to reenter the geometry along a very small step.
static void test_howfar_interior_reentry() {
    EGS_MeshSpec spec;
    spec.elements = {EGS_MeshSpec::Tetrahedron(1, 0, 1, 2, 3, 4)};
    spec.nodes = {EGS_MeshSpec::Node(1, -12.664085999999999, 10.155149, -60.478188000000003),
            EGS_MeshSpec::Node(2, -12.849235999999999, 10.501016999999999, -59.192646000000003),
            EGS_MeshSpec::Node(3, -12.232048000000001, 10.934556000000001, -60.241214999999997),
            EGS_MeshSpec::Node(4, -12.853522999999999, 10.503458, -59.193728999999998)
    };
    spec.media = {EGS_MeshSpec::Medium(0, "H2O")};
    EGS_Mesh mesh(std::move(spec));
    EGS_Vector x(-12.73449210566031,10.364445524278567,-59.830664698600309);
    EGS_Vector u(-0.91563184997597713, -0.043079362143260982, -0.39970299456834102);
    // particle starts inside mesh and travels to boundary
    EGS_Float dist = 1e30;
    auto newreg = mesh.howfar(0, x, u, dist, nullptr);
    if (newreg != -1) {
        throw std::runtime_error(std::string("expected newreg = -1, got ") +
            std::to_string(newreg));
    }
    // move particle `dist` along velocity vector
    EGS_Vector x2(x.x + u.x * dist, x.y + u.y * dist, x.z + u.z * dist);
    dist = 1e30;
    auto boundary = mesh.howfar(-1, x2, u, dist, nullptr);
    if (boundary != -1) {
        throw std::runtime_error(std::string("expected boundary = -1, got ") +
            std::to_string(boundary));
    }
}

// Corner case test: EGS_Mesh enforces a minimum step size. Particles supposed
// to travel steps shorter than the minimum step size should be bumped up to the
// minimum = EGS_Mesh::get_min_step_size().
//
//      a     b
//         |
//    *->  |        =>  howfar = EGS_Mesh::get_min_step_size(), newreg = b
//         |
//    |----|
//    d << 1
static void test_howfar_interior_tolerance(/* const */ EGS_Mesh& mesh) {
    EGS_Vector x(1.0 - 1e-15, 0.0, 0.0);
    EGS_Vector u(1.0, 0.0, 0.0);
    EGS_Float dist = veryFar;
    int newmed = -1;
    auto newreg = mesh.howfar(0, x, u, dist, &newmed);
    if (newreg != 1) {
        throw std::runtime_error(std::string("expected newreg = 1, got ") +
            std::to_string(newreg));
    }
    // check we get a minimum step
    if (dist != EGS_Mesh::get_min_step_size()) {
        throw std::runtime_error(std::string("expected dist = 1e-10, got ") +
            to_string_with_precision(dist));
    }
}

/* Corner case test: behaviour of particle travelling parallel and very near
   boundary surface:

          /
         /  a
    *->  ------
         \  b
          \
*/
static void test_howfar_interior_boundary_straddle() {
    EGS_MeshSpec::Tetrahedron a(1, 0, 1, 2, 3, 4);
    EGS_MeshSpec::Tetrahedron b(2, 0, 2, 3, 4, 5);
    EGS_MeshSpec::Node n1(1, 7.0657957133719744, 2.9344907960112301, 5.0000104685638238);
    EGS_MeshSpec::Node n2(2, 7.3463458702386566, 3.2150409528779131, 5.0000104685638238);
    EGS_MeshSpec::Node n3(3, 7.0345368195412803, 3.2463714740544072, 5.0000086824684313);
    EGS_MeshSpec::Node n4(4, 7.2318114804210998, 3.32957534269547, 4.7698948239873156);
    EGS_MeshSpec::Node n5(5, 7.3150869764079633, 3.5269216309210898, 5.0000086824684313);
    EGS_MeshSpec spec;
    spec.elements = {a, b};
    spec.nodes = {n1, n2, n3, n4, n5};
    spec.media = {EGS_MeshSpec::Medium(0, "H2O")};
    EGS_Mesh mesh(std::move(spec));
    std::vector<EGS_Vector> xs {
        EGS_Vector(7.2785250525479777,3.2806997454426994,4.8659208444750019),
        EGS_Vector(7.2784383802325188,3.2807421180203833,4.8659209976387903),
        EGS_Vector(7.2323212358066646,3.27632101005501,4.8885263860313106),
        EGS_Vector(7.232250310905779,3.2763456480491859,4.8885780658269145),
        EGS_Vector(7.2321865766360034,3.2763687101303014,4.8886269534653843)
    };
    std::vector<EGS_Vector> us {
        EGS_Vector(-0.89838549587000849,0.43920494119202419,0.0015875902833647274),
        EGS_Vector(-0.90129953316251854,0.43319633941521896,0.00028816363187951824),
        EGS_Vector(-0.77867313157333951,0.26853705234531422,0.56705908482578615),
        EGS_Vector(-0.76264679745978736,0.27596177640776914,0.58499141898509754),
        EGS_Vector(-0.75904947178523308,0.30259180325315083,0.57643915549393798)
    };
    int reg = 0;
    for (std::size_t i = 0; i < xs.size(); i++) {
        EGS_Float dist = 1e30;
        reg = mesh.howfar(reg, xs[i], us[i], dist, nullptr);
        if (reg == -1) { throw std::runtime_error("clipped to outside"); }
    }
}

void test_howfar_interior_stuck_on_boundary() {
    EGS_MeshSpec::Tetrahedron e(1, 0, 1, 2, 3, 4);
    EGS_MeshSpec::Node a(1, 3.694681593168294, 0.2039078707572552, 9.025968029424174);
    EGS_MeshSpec::Node b(2, 3.666485868947344, 0, 9.025968029424174);
    EGS_MeshSpec::Node c(3, 3.691449709403482, 0, 8.831161635309009);
    EGS_MeshSpec::Node d(4, 3.874819202280677, 0, 9.025968029424174);
    EGS_MeshSpec::Medium medium(0, "H2O");

    EGS_MeshSpec spec;
    spec.elements = {e};
    spec.nodes = {a, b, c, d};
    spec.media = {medium};
    EGS_Mesh mesh(std::move(spec));

    // particle right on boundary plane travelling parallel to plane
    EGS_Vector x(3.7514090538024902, 0.0, 8.9338606917473982);
    EGS_Vector u(0.0, 0.0, 1.0);
    EGS_Float dist = veryFar;
    int newmed = -1;
    int newreg = mesh.howfar(0, x, u, dist, &newmed);
    if (newreg == 0) {
        throw std::runtime_error("test failed, stuck on boundary plane");
    }
}

// Corner case test: particle is in front of the plane and eligible for
// transport in the thick plane but rounding error makes the calculated distance
// a small negative number (1/1M chance for one 5M-element test mesh). Occurs
// near corner nodes and edges.
//
//                /
//            *->/      =>    t < 0, |t| << 1
//         ------
//           e /
//
void test_howfar_interior_thick_plane_negative_intersection() {
    EGS_MeshSpec::Tetrahedron e(1, 0, 1, 2, 3, 4);
    EGS_MeshSpec::Node a(1, 9.4000000000000004, 5.0, 0.40000000000000002);
    EGS_MeshSpec::Node b(2, 9.5, 5.0, 0.5);
    EGS_MeshSpec::Node c(3, 9.5, 5.0999999999999996, 0.40000000000000002);
    EGS_MeshSpec::Node d(4, 9.4000000000000004, 5.0999999999999996, 0.5);
    EGS_MeshSpec::Medium medium(0, "H2O");

    EGS_MeshSpec spec;
    spec.elements = {e};
    spec.nodes = {a, b, c, d};
    spec.media = {medium};
    EGS_Mesh mesh(std::move(spec));

    // particle in a thick plane, and very near an edge.
    EGS_Vector x(9.4293150393675766,5,0.42931503936757603);
    EGS_Vector u(0.87678438068197007,0,0.48088371753692627);
    EGS_Float dist = veryFar;
    int newmed = -1;
    mesh.howfar(0, x, u, dist, &newmed);
    if (dist < 0.0) {
        throw std::runtime_error("test failed, got negative intersection");
    }
}

static void test_howfar_interior() {
    /* Create a simple two-element mesh and test the three howfar_interior cases

                e1      e2
                    b,c
                    /|\
       y           / | \
       ^          /  |  \
       |         a   |   e
       +--> x     \  |  /
                   \ | /
                    \|/
                     d
    */
    EGS_MeshSpec::Tetrahedron e1(1, 0, 1, 2, 3, 4);
    EGS_MeshSpec::Tetrahedron e2(2, 0, 2, 3, 4, 5);
    EGS_MeshSpec::Node a(1, 0.0, 0.0, 0.0);
    EGS_MeshSpec::Node b(2, 1.0, 1.0, 0.5);
    EGS_MeshSpec::Node c(3, 1.0, 1.0, -0.5);
    EGS_MeshSpec::Node d(4, 1.0, -1.0, 0.0);
    EGS_MeshSpec::Node e(5, 2.0, 0.0, 0.0);
    EGS_MeshSpec::Medium medium(0, "H2O");

    EGS_MeshSpec spec;
    spec.elements = {e1, e2};
    spec.nodes = {a, b, c, d, e};
    spec.media = {medium};
    EGS_Mesh two_elt_mesh(std::move(spec));

    RUN_TEST(test_howfar_interior_regular(two_elt_mesh));
    RUN_TEST(test_howfar_interior_outside_thick_plane(two_elt_mesh));
    RUN_TEST(test_howfar_interior_lost_particle(two_elt_mesh));
    RUN_TEST(test_howfar_interior_tolerance(two_elt_mesh));
    RUN_TEST(test_howfar_interior_reentry());
    RUN_TEST(test_howfar_interior_boundary_straddle());
    RUN_TEST(test_howfar_interior_stuck_on_boundary());
    RUN_TEST(test_howfar_interior_thick_plane_negative_intersection());
}

// neighbour-finding tests

static void test_tetrahedron_face_eq() {
    // tetrahedron faces with the same nodes will compare equal
    {
        mesh_neighbours::Tetrahedron a(1, 2, 3, 4);
        mesh_neighbours::Tetrahedron b(4, 2, 3, 1);
        auto a_faces = a.faces();
        auto b_faces = b.faces();
        if (a_faces[0] != b_faces[3]) {
            throw std::runtime_error("a_faces[0] should equal b_faces[3]");
        }
        if (a_faces[1] != b_faces[1]) {
            throw std::runtime_error("a_faces[1] should equal b_faces[1]");
        }
        if (a_faces[2] != b_faces[2]) {
            throw std::runtime_error("a_faces[2] should equal b_faces[2]");
        }
        if (a_faces[3] != b_faces[0]) {
            throw std::runtime_error("a_faces[3] should equal b_faces[0]");
        }
    }
    // tetrahedrons with three shared nodes have 1 face in common
    {
        mesh_neighbours::Tetrahedron a(1, 2, 3, 4);
        mesh_neighbours::Tetrahedron b(5, 2, 3, 1);
        auto a_faces = a.faces();
        auto b_faces = b.faces();
        if (a_faces[0] == b_faces[3]) {
            throw std::runtime_error("a_faces[0] shouldn't equal b_faces[3]");
        }
        if (a_faces[1] == b_faces[1]) {
            throw std::runtime_error("a_faces[1] shouldn't equal b_faces[1]");
        }
        if (a_faces[2] == b_faces[2]) {
            throw std::runtime_error("a_faces[2] shouldn't equal b_faces[2]");
        }
        if (a_faces[3] != b_faces[0]) {
            throw std::runtime_error("a_faces[3] should equal b_faces[0]");
        }
    }
}

static void test_tetrahedron_errors() {
    // duplicate tetrahedron nodes are caught
    EXPECT_ERROR(mesh_neighbours::Tetrahedron(1, 1, 2, 3), "duplicate node 1");
    EXPECT_ERROR(mesh_neighbours::Tetrahedron(1, 2, 2, 3), "duplicate node 2");
}

static void test_tetrahedron_neighbours() {
    using mesh_neighbours::NONE;
    egs_mesh::internal::PercentCounter null_logger(nullptr, "");

    std::vector<mesh_neighbours::Tetrahedron> disjoint_tets {
        mesh_neighbours::Tetrahedron(1, 2, 3, 4),
        mesh_neighbours::Tetrahedron(5, 6, 7, 8)
    };

    if (mesh_neighbours::tetrahedron_neighbours(disjoint_tets, null_logger) !=
        std::vector<std::array<int, 4>>{
            std::array<int, 4>{NONE, NONE, NONE, NONE},
            std::array<int, 4>{NONE, NONE, NONE, NONE}
        })
    {
        throw std::runtime_error("disjoint_tets should have no neighbours");
    }
    std::vector<mesh_neighbours::Tetrahedron> linked_tets {
        mesh_neighbours::Tetrahedron(1, 2, 3, 4),
        mesh_neighbours::Tetrahedron(1, 2, 3, 5)
    };
    if (mesh_neighbours::tetrahedron_neighbours(linked_tets, null_logger) !=
        std::vector<std::array<int, 4>>{
            std::array<int, 4>{NONE, NONE, NONE, 1},
            std::array<int, 4>{NONE, NONE, NONE, 0}
        })
    {
        throw std::runtime_error("bad neighbours for linked_tets");
    }
    // 0 nodes are OK
    std::vector<mesh_neighbours::Tetrahedron> tets_with_0 {
        mesh_neighbours::Tetrahedron(0, 2, 3, 4),
        mesh_neighbours::Tetrahedron(1, 2, 3, 4)
    };
    if (mesh_neighbours::tetrahedron_neighbours(tets_with_0, null_logger) !=
        std::vector<std::array<int,4>>{
            std::array<int, 4>{1, NONE, NONE, NONE},
            std::array<int, 4>{0, NONE, NONE, NONE}
        })
    {
        throw std::runtime_error("bad neighbours for tets_with_0");
    }
}

// msh_parser tests

static void test_parse_msh_version() {
    using namespace msh_parser::internal;
    // catch empty inputs
    {
        std::istringstream input("");
        EXPECT_ERROR(parse_msh_version(input), "unexpected end of input");
    }
    // bad format header
    {
        std::istringstream input("$MshFmt\n");
        EXPECT_ERROR(parse_msh_version(input), "expected $MeshFormat, got $MshFmt");
    }
    // bad msh version line
    {
         std::istringstream input(
            "$MeshFormat\n"
            "0\n"
            "$EndMeshFormat\n"
        );
        EXPECT_ERROR(parse_msh_version(input), "failed to parse msh version");
    }
    // unknown msh version
    {
        std::istringstream input(
            "$MeshFormat\n"
            "100.2 0 8\n"
            "$EndMeshFormat\n"
        );
        EXPECT_ERROR(parse_msh_version(input), "unsupported msh version `100.2`, the only supported version is 4.1");
    }
    // binary files are unsupported
    {
        std::istringstream input(
            "$MeshFormat\n"
            "4.1 1 8\n"
            "$EndMeshFormat\n"
        );
        EXPECT_ERROR(parse_msh_version(input), "binary msh files are unsupported, please convert this file to ascii and try again");
    }
    // size_t != 8 is unsupported
    {
        std::istringstream input(
            "$MeshFormat\n"
            "4.1 0 4\n"
            "$EndMeshFormat\n"
        );
        EXPECT_ERROR(parse_msh_version(input), "msh file size_t must be 8");
    }
    // eof after version line fails
    {
        std::istringstream input(
            "$MeshFormat\n"
            "4.1 0 8\n"
        );
        EXPECT_ERROR(parse_msh_version(input), "expected $EndMeshFormat, got ``");
    }
    // parse msh v4.1 successfully
    {
        std::istringstream input(
            "$MeshFormat\n"
            "4.1 0 8\n"
            "$EndMeshFormat\n"
        );
        // might throw
        MshVersion vers = parse_msh_version(input);
        if (vers != MshVersion::v41) {
            throw std::runtime_error("expected version = v41");
        }
    }
    // windows line-endings are OK
    {
        std::istringstream input(
            "$MeshFormat\r\n"
            "4.1 0 8\r\n"
            "$EndMeshFormat\r\n"
        );
        // might throw
        MshVersion vers = parse_msh_version(input);
        if (vers != MshVersion::v41) {
            throw std::runtime_error("expected version = v41");
        }
    }
}

// all cases assume $PhysicalNames header has already been parsed
static void test_parse_msh41_groups() {
    using namespace msh_parser::internal;
    // empty section is OK
    {
        std::istringstream input(
            "0\n"
            "$EndPhysicalNames\n"
        );
        std::vector<msh41::PhysicalGroup> groups = msh41::parse_groups(input);
        if (groups.size() != 0) {
            throw std::runtime_error("expected 0 groups at line " +
                std::to_string(__LINE__));
        }
    }
    // missing $EndPhysicalNames tag fails
    {
        std::istringstream input(
            "3\n"
            "1 1 \"a line\"\n"
            "2 2 \"a surface\"\n"
            "3 3 \"a volume\"\n"
        );
        EXPECT_ERROR(msh41::parse_groups(input),
            "unexpected end of file, expected $EndPhysicalNames");
    }
    // bad physical group line fails
    {
        std::istringstream input(
            "1\n"
            "1 \"a line\"\n" // missing tag
            "$EndPhysicalNames\n"
        );
        EXPECT_ERROR(msh41::parse_groups(input),
            "physical group parsing failed: 1 \"a line\"");
    }
    // catch invalid physical group names
    {
        std::istringstream input(
            "1\n"
            "3 1 \"\"\n"
            "$EndPhysicalNames\n"
        );
        EXPECT_ERROR(msh41::parse_groups(input),
            "empty physical group name: 3 1 \"\"");
    }
    // physical group names are quoted
    {
        std::istringstream input(
            "1\n"
            "3 1 Steel\n"
            "$EndPhysicalNames\n"
        );
        EXPECT_ERROR(msh41::parse_groups(input),
            "physical group names must be quoted: 3 1 Steel");
    }
    // closing name quote is required
    {
        std::istringstream input(
            "1\n"
            "3 1 \"Steel\n"
            "$EndPhysicalNames\n"
        );
        EXPECT_ERROR(msh41::parse_groups(input),
            "couldn't find closing quote for physical group: 3 1 \"Steel");
    }
    // only 3D groups are returned
    {
         std::istringstream input(
            "3\n"
            "1 1 \"line\"\n"
            "2 2 \"surface\"\n"
            "3 3 \"volume\"\n"
            "$EndPhysicalNames\n"
        );
        std::vector<msh41::PhysicalGroup> groups = msh41::parse_groups(input);
        if (groups.size() != 1) {
            throw std::runtime_error("expected 1 group at line" +
                std::to_string(__LINE__));
        }
        std::string expected_name = "volume";
        if (groups.at(0).name != expected_name) {
            throw std::runtime_error("bad physical name parse, expected: " +
                expected_name + "but got: " + groups.at(0).name);
        }
    }
    // duplicate 3D group tags are caught
    {
         std::istringstream input(
            "4\n"
            "1 1 \"line\"\n"
            "2 2 \"surface\"\n"
            "3 3 \"volume\"\n"
            "3 4 \"volume2\"\n"
            "3 4 \"other volume\"\n" // tag 4 repeated
            "$EndPhysicalNames\n"
        );
        EXPECT_ERROR(msh41::parse_groups(input),
            "$PhysicalNames section parsing failed, found duplicate tag 4");
    }
    // spaces in names are OK
    {
         std::istringstream input(
            "3\n"
            "1 1 \"a line\"\n"
            "2 2 \"a surface\"\n"
            "3 3 \"a volume\"\n"
            "$EndPhysicalNames\n"
        );
        std::vector<msh41::PhysicalGroup> groups = msh41::parse_groups(input);
        if (groups.size() != 1) {
            throw std::runtime_error("expected 1 group at line" +
                std::to_string(__LINE__));
        }
        std::string expected_name = "a volume";
        if (groups.at(0).name != expected_name) {
            throw std::runtime_error("bad physical name parse, expected: " +
                expected_name + "but got: " + groups.at(0).name);
        }
    }
    // single letter names are OK
    {
         std::istringstream input(
            "3\n"
            "1 1 \"a line\"\n"
            "2 2 \"a surface\"\n"
            "3 3 \"a\"\n"
            "$EndPhysicalNames\n"
        );
        std::vector<msh41::PhysicalGroup> groups = msh41::parse_groups(input);
        if (groups.size() != 1) {
            throw std::runtime_error("expected 1 group at line" +
                std::to_string(__LINE__));
        }
        std::string expected_name = "a";
        if (groups.at(0).name != expected_name) {
            throw std::runtime_error("bad physical name parse, expected: " +
                expected_name + "but got: " + groups.at(0).name);
        }
    }
    // successfully parse a valid physical groups section
    {
         std::istringstream input(
            "5\n"
            "1 1 \"a line\"\n"
            "2 2 \"a surface\"\n"
            "3 3 \"Steel\"\n"
            "3 4 \"Air\"\n"
            "3 5 \"Water\"\n"
            "$EndPhysicalNames\n"
        );
        std::vector<msh41::PhysicalGroup> groups = msh41::parse_groups(input);
        if (groups.size() != 3) {
            throw std::runtime_error("expected 3 groups at line" +
                std::to_string(__LINE__));
        }
        if (! (groups.at(0).name == "Steel" && groups.at(0).tag == 3) &&
              (groups.at(1).name == "Air" && groups.at(1).tag == 4) &&
              (groups.at(2).name == "Water" && groups.at(2).tag == 5))
        {
            throw std::runtime_error(
                "parsed physical groups didn't match reference values");
        }
    }
}

static void test_parse_msh41_node_bloc() {
    using namespace msh_parser::internal;
    // missing bloc metadata fails
    {
        std::istringstream input(
        //    "1 100 0 1\n"
            "1\n"
            "1 0 0\n"
        );
        EXPECT_ERROR(msh41::parse_node_bloc(input), "Node bloc parsing failed");
    }
    // bad dimension value fails
    {
        std::istringstream input(
            "4 100 0 1\n" // 4d entity?
            "1\n"
            "1 0 0\n"
        );
        EXPECT_ERROR(msh41::parse_node_bloc(input),
            "Node bloc parsing failed for entity 100, got dimension 4,"
            " expected 0, 1, 2, or 3");
    }
    // wrong number of nodes fails
    {
        std::istringstream input(
            "1 100 0 2\n" // 2 nodes given, only 1 present
            "1\n"
            "1 0 0\n"
        );
        EXPECT_ERROR(msh41::parse_node_bloc(input),
            "Node bloc parsing failed during node coordinate section of entity 100");
    }
    // successfully parse a single node bloc
    {
        std::istringstream input(
            "1 1 0 3\n"
            "1\n"
            "2\n"
            "3\n"
            "1 0 0\n"
            "0 1 0\n"
            "0 0 1\n"
        );
        std::string err_msg;
        std::vector<msh41::Node> nodes = msh41::parse_node_bloc(input);
        if (nodes.size() != 3) {
            throw std::runtime_error("expected 3 nodes, got " +
                std::to_string(nodes.size()));
        }
        auto n0 = nodes.at(0);
        auto n1 = nodes.at(1);
        auto n2 = nodes.at(2);
        if (!(n0.tag == 1 && n0.x == 1.0 && n0.y == 0.0 && n0.z == 0.0 &&
              n1.tag == 2 && n1.x == 0.0 && n1.y == 1.0 && n1.z == 0.0 &&
              n2.tag == 3 && n2.x == 0.0 && n2.y == 0.0 && n2.z == 1.0))
        {
            throw std::runtime_error(
                "parsed nodes didn't match reference value");
        }
    }
}

static void test_parse_msh41_nodes() {
    using namespace msh_parser::internal;
    // bad input stream fails
    {
        std::ifstream input("bad-file");
        EXPECT_ERROR(msh41::parse_nodes(input, nullptr),
            "$Nodes section parsing failed, missing metadata");
    }
    // missing section metadata fails eventually (is parsed as the first bloc metadata)
    {
        std::istringstream input(
            // "1 1 1 1\n"
            "1 1 0 1\n"
            "1\n"
            "1 0 0\n"
        );
        EXPECT_ERROR(msh41::parse_nodes(input, nullptr),
            "$Nodes section parsing failed\nNode bloc parsing failed");
    }
    // wrong num_blocs fails
    {
        std::istringstream input(
            "2 1 1 2\n" // num_blocs = 2 but is really 1
            "1 1 0 1\n"
            "1\n"
            "1 0 0\n"
        );
        EXPECT_ERROR(msh41::parse_nodes(input, nullptr),
            "$Nodes section parsing failed\nNode bloc parsing failed");
    }
    // node tags must fit into an int
    {
        std::size_t too_large = std::size_t(std::numeric_limits<int>::max());
        too_large += 1;
        std::istringstream input(
            "2 1 1 " + std::to_string(too_large) + "\n"
            "1 1 0 1\n"
            "1\n"
            "1 0 0\n"
        );
        EXPECT_ERROR(msh41::parse_nodes(input, nullptr),
            "Max node tag is too large (2147483648), limit is 2147483647");
    }
    // wrong num_nodes fails
    {
        std::istringstream input(
            "1 100 1 2\n" // 100 nodes
            "1 1 0 1\n"
            "1\n"
            "1 0 0\n"
        );
        EXPECT_ERROR(msh41::parse_nodes(input, nullptr),
            "$Nodes section parsing failed, expected 100 nodes but read 1");
    }
    // missing $EndNodes fails
    {
         std::istringstream input(
            "1 1 1 1\n"
            "1 1 0 1\n"
            "1\n"
            "1 0 0\n"
            // "$EndNodes\n"
        );
        EXPECT_ERROR(msh41::parse_nodes(input, nullptr),
            "$Nodes section parsing failed, expected $EndNodes");
    }
    // duplicate node tags are caught
    {
         std::istringstream input(
            "2 2 1 2\n"
            "1 1 0 1\n"
            "1\n"
            "1 0 0\n"
            "1 2 0 1\n"
            "1\n" // node tag 1 repeated
            "1 0 0\n"
            "$EndNodes\n"
        );
        EXPECT_ERROR(msh41::parse_nodes(input, nullptr),
            "$Nodes section parsing failed, found duplicate node tag 1");
    }
    // parse multiple blocs successfully
    {
        std::istringstream input(
            "3 7 1 7\n"
            "1 1 0 1\n"
            "1\n"
            "1 0 0\n"
            "1 2 0 2\n"
            "2\n"
            "3\n"
            "0 1 0\n"
            "0 1 0\n"
            "3 2 0 4\n"
            "4\n"
            "5\n"
            "6\n"
            "7\n"
            "0 0 1\n"
            "0 0 1\n"
            "0 0 1\n"
            "0 0 1\n"
            "$EndNodes\n"
        );
        std::vector<msh41::Node> nodes = msh41::parse_nodes(input, nullptr);
        if (nodes.size() != 7) {
            throw std::runtime_error("expected 7 nodes at line" +
                std::to_string(__LINE__));
        }
        auto n0 = nodes.at(0);
        auto n1 = nodes.at(1);
        auto n6 = nodes.at(6);
        if (!(n0.tag == 1 && n0.x == 1.0 && n0.y == 0.0 && n0.z == 0.0 &&
              n1.tag == 2 && n1.x == 0.0 && n1.y == 1.0 && n1.z == 0.0 &&
              n6.tag == 7 && n6.x == 0.0 && n6.y == 0.0 && n6.z == 1.0))
        {
            throw std::runtime_error(
                "parsed nodes didn't match reference value");
        }
    }
}

static void test_parse_msh41_entities() {
    using namespace msh_parser::internal;
    // bad input stream fails
    {
        std::ifstream input("bad-file");
        EXPECT_ERROR(msh41::parse_entities(input), "$Entities parsing failed");
    }
    // no 3d entities fails
    {
        std::istringstream input(
            "2 1 1 0\n"
            "$EndEntities\n"
        );
        EXPECT_ERROR(msh41::parse_entities(input), "$Entities parsing failed, no volumes found");
    }
    // 3d entity without a physical group fails
    {
        std::istringstream input(
            "0 0 0 1\n"
            "1 0.0 0.0 0.0 1.0 1.0 1.0 0 1\n"
            //                         ^-- num physical groups = 0
            "$EndEntities\n"
        );
        EXPECT_ERROR(msh41::parse_entities(input),
            "$Entities parsing failed, volume 1 was not assigned a physical group");
    }
    // 3d entity with more than one physical group fails
    {
        std::istringstream input(
            "0 0 0 1\n"
            "2 0.0 0.0 0.0 1.0 1.0 1.0 2 1 2\n"
            //                         ^-- num physical groups = 2
            "$EndEntities\n"
        );
        EXPECT_ERROR(msh41::parse_entities(input),
            "$Entities parsing failed, volume 2 has more than one physical group");
    }
    // repeated 3d entity tags fails
    {
        std::istringstream input(
            "0 0 0 2\n"
            "1 0.0 0.0 0.0 1.0 1.0 1.0 1 1\n"
            "1 0.0 0.0 0.0 1.0 1.0 1.0 1 1\n"
        //   ^-- volume tag 1 appears twice
            "$EndEntities\n"
        );
        EXPECT_ERROR(msh41::parse_entities(input),
            "$Entities section parsing failed, found duplicate volume tag 1");
    }
    // num entities mismatch fails
    {
        std::istringstream input(
            "0 0 0 2\n"
            "2 0.0 0.0 0.0 1.0 1.0 1.0 1 1\n"
            "$EndEntities\n"
        );
        EXPECT_ERROR(msh41::parse_entities(input),
            "$Entities parsing failed, expected 2 volumes but got 1");
    }
    // catch duplicate volume tags
    {
        std::istringstream input(
            "0 0 0 2\n"
            "2 0.0 0.0 0.0 1.0 1.0 1.0 1 1\n"
            "2 0.0 0.0 0.0 1.0 1.0 1.0 1 1\n"
            "$EndEntities\n"
        );
        EXPECT_ERROR(msh41::parse_entities(input),
            "$Entities section parsing failed, found duplicate volume tag 2");
    }
    // successfully parse volumes, skipping 0, 1, 2d entities
    {
        std::istringstream input(
            "1 2 3 2\n"
            // 1 0d entity
            "1 0 0 1 0\n"
            // 2 1d entities
            "1 -1e-007 -1e-007 -9.999999994736442e-008 1e-007 1e-007 1.0000001 0 2 2 -1\n"
            "2 -1e-007 -9.999999994736442e-008 0.9999999000000001 1e-007 1.0000001 1.0000001 0 2 1 -3\n"
            // 3 2d entities
            "1 -1e-007 -9.999999994736442e-008 -9.999999994736442e-008 1e-007 1.0000001 1.0000001 0 4 1 2 -3 -4\n"
            "2 0.9999999000000001 -9.999999994736442e-008 -9.999999994736442e-008 1.0000001 1.0000001 1.0000001 0 4 5 6 -7 -8\n"
            "3 -9.999999994736442e-008 -1e-007 -9.999999994736442e-008 1.0000001 1e-007 1.0000001 0 4 9 5 -10 -1\n"
            // 2 3d entities
        //   |-- tag                                           |-- physical group
            "1 -9.99e-008 -9.99e-008 -9.99e-008 1.0 1.0 1.0 1 100 6 1 2 3 4 5 6\n"
            "2 -9.99e-008 -9.99e-008 -9.99e-008 1.0 1.0 1.0 1 200 6 1 2 3 4 5 6\n"
            "$EndEntities\n"
        );
        std::vector<msh41::MeshVolume> vols = msh41::parse_entities(input);
        if (vols.size() != 2) {
            throw std::runtime_error("expected 2 volumes at line" +
                std::to_string(__LINE__));
        }
        if (!(vols.at(0).tag == 1 && vols.at(0).group == 100 &&
              vols.at(1).tag == 2 && vols.at(1).group == 200))
        {
            throw std::runtime_error(
                "parsed volumes didn't match reference value");
        }
    }
}

static void test_parse_msh41_element_bloc() {
    using namespace msh_parser::internal;
    // bad input stream fails
    {
        std::ifstream input("bad-file");
        EXPECT_ERROR(msh41::parse_element_bloc(input), "Element bloc parsing failed");
    }
    // skip lower-dimension elements
    {
        std::istringstream input(
        //  v-- 2d shape
            "2 1 3 2\n"
            "1 1 2 3 4\n"
            "2 2 5 6 3\n"
        );
        std::vector<msh41::Tetrahedron> elts = msh41::parse_element_bloc(input);
        if (elts.size() != 0) {
            throw std::runtime_error("expected 0 elements at line" +
                std::to_string(__LINE__));
        }
    }
    // non-tetrahedral 3d elements fails
    {
         std::istringstream input(
            "3 1 5 1\n"
            //   ^-- 5 is code for hexahedron
            "1 1 2 3 4 5 6\n"
        );
        EXPECT_ERROR(msh41::parse_element_bloc(input),
            "Element bloc parsing failed for entity 1" ", got non-tetrahedral mesh element type 5");
    }
    // missing tetrahedron data fails
    {
         std::istringstream input(
            "3 2 4 3\n"
            "1 1 2 3\n" // only 3/4 nodes given
            "10 10 20 30 40\n"
            "11 5 6 7 8\n"
        );
        EXPECT_ERROR(msh41::parse_element_bloc(input),
            "Element bloc parsing failed for entity 2");
    }
    // successfully parse a tetrahedron element bloc
    {
         std::istringstream input(
            "3 1 4 3\n"
            // ^ ^ ^-- 3 elements
            // | |---- 4 => tetrahedron
            // |------ volume id 1
            "1 1 2 3 4\n"
            "10 10 20 30 40\n"
            "11 5 6 7 8\n"
        );
        std::vector<msh41::Tetrahedron> elts = msh41::parse_element_bloc(input);
        if (elts.size() != 3) {
            throw std::runtime_error("expected 3 elements at line" +
                std::to_string(__LINE__));
        }
        auto e0 = elts.at(0);
        auto e1 = elts.at(1);
        auto e2 = elts.at(2);
        if (!(e0.tag == 1 && e0.volume == 1 &&
               e0.a == 1 && e0.b == 2 && e0.c == 3 && e0.d == 4 &&
              e1.tag == 10 && e1.volume == 1 &&
               e1.a == 10 && e1.b == 20 && e1.c == 30 && e1.d == 40 &&
              e2.tag == 11 && e2.volume == 1 &&
               e2.a == 5 && e2.b == 6 && e2.c == 7 && e2.d == 8))
        {
            throw std::runtime_error(
                "parsed elements didn't match reference value");
        }
    }
}

static void test_parse_msh41_elements() {
    using namespace msh_parser::internal;
    // bad input stream fails
    {
        std::ifstream input("bad-file");
        EXPECT_ERROR(msh41::parse_elements(input, nullptr), "$Elements section parsing failed, missing metadata");
    }
    // no elements fails
    {
         std::istringstream input(
            "0 0 0 0\n"
            "$EndElements\n"
         );
        EXPECT_ERROR(msh41::parse_elements(input, nullptr), "$Elements section parsing failed, no tetrahedral elements were read");
    }
    // no tetrahedral elements fails
    {
        std::istringstream input(
            "1 1 1 2\n"
            "1 10 1 2\n"
            "1 1 2\n"
            "2 2 3\n"
            "$EndElements\n"
        );
        EXPECT_ERROR(msh41::parse_elements(input, nullptr), "$Elements section parsing failed, no tetrahedral elements were read");
    }
    // missing $EndElements fails
    {
         std::istringstream input(
            "1 2 1 2\n" // 1 bloc, 2 elements, min = 1, max = 2
            "3 1 4 2\n"
            "1 1 2 3 4\n"
            "2 5 6 7 8\n"
            // "$EndElements\n"
        );
        EXPECT_ERROR(msh41::parse_elements(input, nullptr), "$Elements section parsing failed, expected $EndElements");
    }
    // skip lower-dimension elements
    {
         std::istringstream input(
            "2 4 1 4\n" // 2 blocs, 4 elts, min = 1, max = 4
            "1 10 1 2\n" // 1d element bloc should be skipped
            "1 1 2\n"
            "2 2 3\n"
            "3 50 4 2\n" // tetrahedron bloc should be parsed
            "1 1 2 3 4\n"
            "2 5 6 7 8\n"
            "$EndElements\n"
        );
        std::vector<msh41::Tetrahedron> elts = msh41::parse_elements(input,
            nullptr);
        if (elts.size() != 2) {
            throw std::runtime_error("expected 2 elements at line" +
                std::to_string(__LINE__));
        }
        auto e0 = elts.at(0);
        auto e1 = elts.at(1);
        if (!(e0.tag == 1 && e0.volume == 50 &&
               e0.a == 1 && e0.b == 2 && e0.c == 3 && e0.d == 4 &&
              e1.tag == 2 && e1.volume == 50 &&
               e1.a == 5 && e1.b == 6 && e1.c == 7 && e1.d == 8))
        {
            throw std::runtime_error(
                "parsed elements didn't match reference value");
        }
    }
    // duplicate tetrahedron tags are caught
    {
         std::istringstream input(
            "2 4 1 4\n"
            "3 1 4 2\n"
            "1 1 2 3 4\n"
            "2 5 6 7 8\n"
            "3 2 4 2\n"
            "1 1 2 3 4\n" // tag 1 again
            "4 5 6 7 8\n"
            "$EndElements\n"
        );
        EXPECT_ERROR(msh41::parse_elements(input, nullptr),
            "$Elements section parsing failed, found duplicate tetrahedron tag 1");
    }
    // successfully parses multiple element blocs
    {
         std::istringstream input(
            "3 6 1 6\n"
            "1 10 1 2\n"
            "1 1 2\n"
            "2 2 3\n"
            "3 1 4 2\n"   // tetrahedron bloc 1
            "3 1 2 3 4\n"
            "4 5 6 7 8\n"
            "3 2 4 2\n"   // tetrahedron bloc 2
            "5 1 2 3 5\n"
            "6 5 6 7 1\n"
            "$EndElements\n"
        );
        std::vector<msh41::Tetrahedron> elts = msh41::parse_elements(input,
            nullptr);
        if (elts.size() != 4) {
            throw std::runtime_error("expected 4 elements at line" +
                std::to_string(__LINE__));
        }
        auto e0 = elts.at(0);
        auto e1 = elts.at(1);
        auto e2 = elts.at(2);
        auto e3 = elts.at(3);
        if (!(e0.tag == 3 && e0.volume == 1 &&
               e0.a == 1 && e0.b == 2 && e0.c == 3 && e0.d == 4 &&
              e1.tag == 4 && e1.volume == 1 &&
               e1.a == 5 && e1.b == 6 && e1.c == 7 && e1.d == 8 &&
              e2.tag == 5 && e2.volume == 2 &&
               e2.a == 1 && e2.b == 2 && e2.c == 3 && e2.d == 5 &&
              e3.tag == 6 && e3.volume == 2 &&
               e3.a == 5 && e3.b == 6 && e3.c == 7 && e3.d == 1))
        {
            throw std::runtime_error(
                "parsed elements didn't match reference value");
        }
    }
}

// example mesh file for file tests
struct MeshFile {
    const std::string header =
        "$MeshFormat\n"
        "4.1 0 8\n"
        "$EndMeshFormat\n";

    const std::string entities =
        "$Entities\n"
        "0 0 0 2\n"
        "1 0 0 0 1.0 1.0 1.0 1 1 6 1 2 3 4 5 6\n"
        "2 0 0 0 1.0 1.0 1.0 1 2 6 1 2 3 4 5 6\n"
        "$EndEntities\n";

    const std::string pgroups =
        "$PhysicalNames\n"
        "2\n"
        "3 1 \"Steel\"\n"
        "3 2 \"Water\"\n"
        "$EndPhysicalNames\n";

    const std::string nodes =
        "$Nodes\n"
        "2 5 1 5\n"
        "1 1 0 2\n"
        "1\n"
        "2\n"
        "0 0 0\n"
        "0 1 0\n"
        "1 2 0 3\n"
        "3\n"
        "4\n"
        "5\n"
        "1 0 0\n"
        "1 1 0\n"
        "1 1 1\n"
        "$EndNodes\n";

    const std::string elts =
        "$Elements\n"
         "2 4 1 4\n"
         "3 1 4 2\n"
         "1 1 2 3 4\n"
         "2 1 2 3 5\n"
         "3 2 4 2\n"
         "3 1 2 4 5\n"
         "4 2 3 4 5\n"
         "$EndElements\n";
};

static void test_parse_msh41_file_errors(MeshFile file) {
    using namespace msh_parser::internal;
    // Unknown physical group tags assigned to entities are caught
    {
        std::istringstream input(
            file.header + file.nodes + file.elts +
            "$Entities\n"
            "0 0 0 1\n"
            "1 0.0 0.0 0.0 1.0 1.0 1.0 1 100\n"
            //                           ^
            // physical group tag 100 is not part of $PhysicalNames
            "$EndEntities\n"
            "$PhysicalNames\n"
            "1\n"
            "3 1 \"Steel\"\n"
            // ^ expecting tag == 1
            "$EndPhysicalNames\n"
        );
        EXPECT_ERROR(EGS_Mesh(msh_parser::parse_msh_file(input)),
            "msh 4.1 parsing failed\nvolume 1 had unknown physical group tag 100");
    }

    // Unknown volume (entity) tags assigned to elements are caught
    {
        std::istringstream input(
            file.header + file.nodes + file.pgroups +
            "$Entities\n"
            "0 0 0 1\n"
            "1 0.0 0.0 0.0 1.0 1.0 1.0 1 1\n"
            "$EndEntities\n"
            "$Elements\n"
            "1 1 1 1\n"
            "3 100 4 1\n"
            // ^ entity tag 100 is not present in $Entities
            "1 1 2 3 4\n"
            "$EndElements\n"
        );
        EXPECT_ERROR(EGS_Mesh(msh_parser::parse_msh_file(input)),
            "msh 4.1 parsing failed\ntetrahedron 1 had unknown volume tag 100");
    }
}

static void test_parse_msh41_file(MeshFile file) {
    using namespace msh_parser::internal;
    // section errors bubble up
    {
        std::istringstream input(
            "$MeshFormat\n"
            "4.1 0 8\n"
            "$EndMeshFormat\n"
            "$PhysicalNames\n" // missing PhysicalNames content
            "$EndPhysicalNames\n"
        );
        EXPECT_ERROR(EGS_Mesh(msh_parser::parse_msh_file(input)),
            "msh 4.1 parsing failed\n$PhysicalNames parsing failed");
    }
    // minimum complete mesh file for EGSnrc
    {
        std::istringstream input(file.header + file.entities + file.pgroups
            + file.nodes + file.elts);

        EGS_Mesh mesh(msh_parser::parse_msh_file(input));
        auto n_elts = mesh.num_elements();
        if (n_elts != 4) {
            throw std::runtime_error("expected 4 elements at line " +
                std::to_string(__LINE__));
        }

        // media
        if (mesh.getMediumName(mesh.medium(0)) != std::string("Steel")) {
            throw std::runtime_error("element 0 should be Steel");
        }
        if (mesh.getMediumName(mesh.medium(1)) != std::string("Steel")) {
            std::cout << mesh.medium(1) << "\n";
            throw std::runtime_error("element 1 should be Steel");
        }
        if (mesh.getMediumName(mesh.medium(2)) != std::string("Water")) {
            throw std::runtime_error("element 2 should be Water");
        }
        if (mesh.getMediumName(mesh.medium(3)) != std::string("Water")) {
            throw std::runtime_error("element 3 should be Water");
        }

        // element nodes
        auto node_offsets = mesh.element_node_offsets(0);
        // offsets are node_tag - 1
        if (!(node_offsets[0] == 0 && node_offsets[1] == 1 &&
            node_offsets[2] == 2 && node_offsets[3] == 3))
        {
            throw std::runtime_error("bad node offsets for element 0");
        }
        node_offsets = mesh.element_node_offsets(1);
        if (!(node_offsets[0] == 0 && node_offsets[1] == 1 &&
            node_offsets[2] == 2 && node_offsets[3] == 4))
        {
            throw std::runtime_error("bad node offsets for element 1");
        }
        node_offsets = mesh.element_node_offsets(2);
        if (!(node_offsets[0] == 0 && node_offsets[1] == 1 &&
            node_offsets[2] == 3 && node_offsets[3] == 4))
        {
            throw std::runtime_error("bad node offsets for element 2");
        }
        node_offsets = mesh.element_node_offsets(3);
        if (!(node_offsets[0] == 1 && node_offsets[1] == 2 &&
            node_offsets[2] == 3 && node_offsets[3] == 4))
        {
            throw std::runtime_error("bad node offsets for element 3");
        }

        if (mesh.num_nodes() != 5) {
            throw std::runtime_error("expected 5 nodes at line " +
                std::to_string(__LINE__));
        }

        auto pos = mesh.node_coordinates(0);
        if (!(pos.x == 0.0 && pos.y == 0.0 && pos.z == 0.0)) {
            throw std::runtime_error("bad node coordinates for node 0");
        }
        pos = mesh.node_coordinates(1);
        if (!(pos.x == 0.0 && pos.y == 1.0 && pos.z == 0.0)) {
            throw std::runtime_error("bad node coordinates for node 1");
        }
        pos = mesh.node_coordinates(2);
        if (!(pos.x == 1.0 && pos.y == 0.0 && pos.z == 0.0)) {
            throw std::runtime_error("bad node coordinates for node 2");
        }
        pos = mesh.node_coordinates(3);
        if (!(pos.x == 1.0 && pos.y == 1.0 && pos.z == 0.0)) {
            throw std::runtime_error("bad node coordinates for node 3");
        }
        pos = mesh.node_coordinates(4);
        if (!(pos.x == 1.0 && pos.y == 1.0 && pos.z == 1.0)) {
            throw std::runtime_error("bad node coordinates for node 4");
        }
    }
}

// tetgen_parser tests

static void test_parse_tetgen_nodes() {
    // blank file fails
    {
        std::istringstream input("");
        EXPECT_ERROR(tetgen_parser::internal::parse_tetgen_node_file(input, nullptr),
            "failed to parse TetGen node file header");
    }
    // file has to start with the header line
    {
        std::istringstream input(
            "# some comment\n"
            "4 3 0 0\n" // header line
            "0 0 0 0\n"
            "1 1.0 2.0 3.0\n"
            "2 2.0 4.0 6.0\n"
            "3 3.0 6.0 9.0\n"
        );
        EXPECT_ERROR(tetgen_parser::internal::parse_tetgen_node_file(input, nullptr),
            "failed to parse TetGen node file header");
    }
    // num_coords must be 3
    {
        std::istringstream input(
            "4 2 0 0\n" // header line
        );
        EXPECT_ERROR(tetgen_parser::internal::parse_tetgen_node_file(input, nullptr),
            "TetGen node file parsing failed, expected num_coords = 3");
    }
    // Helper to check correctness of parsed nodes
    auto check_parsed_nodes = [](const std::vector<EGS_MeshSpec::Node>& nodes) -> bool {
        if (nodes.size() != 4) { return false; }
        if (nodes[0].tag != 0 || nodes[0].x != 0.0 || nodes[0].y != 0.0 || nodes[0].z != 0.0) { return false; }
        if (nodes[1].tag != 1 || nodes[1].x != 1.0 || nodes[1].y != 2.0 || nodes[1].z != 3.0) { return false; }
        if (nodes[2].tag != 2 || nodes[2].x != 2.0 || nodes[2].y != 4.0 || nodes[2].z != 6.0) { return false; }
        if (nodes[3].tag != 3 || nodes[3].x != 3.0 || nodes[3].y != 6.0 || nodes[3].z != 9.0) { return false; }
        return true;
    };
    // node body is parsed correctly
    {
        std::istringstream input(
            "4 3 0 0\n" // header line
            "0 0 0 0\n"
            "1 1.0 2.0 3.0\n"
            "2 2.0 4.0 6.0\n"
            "3 3.0 6.0 9.0\n"
        );
        if (!check_parsed_nodes(tetgen_parser::internal::parse_tetgen_node_file(input, nullptr))) {
            throw std::runtime_error("TetGen node parsing failed");
        }
    }
    // attribute and boundary data are ignored
    {
        std::istringstream input(
            "4 3 1 1\n" // header line
            "0 0 0 0 1.0 1\n"
            "1 1.0 2.0 3.0 1.0 0 \n"
            "2 2.0 4.0 6.0 2.0 0 \n"
            "3 3.0 6.0 9.0 3.0 0 \n"
        );
        if (!check_parsed_nodes(tetgen_parser::internal::parse_tetgen_node_file(input, nullptr))) {
            throw std::runtime_error("TetGen node file attributes and boundary data aren't skipped");
        }
    }
    // comment lines starting with # in the body are skipped
    {
        std::istringstream input(
            "4 3 0 0\n" // header line
            "# comment\n"
            "0 0 0 0\n"
            "1 1.0 2.0 3.0\n"
            "2 2.0 4.0 6.0\n"
            "3 3.0 6.0 9.0\n"
        );
        if (!check_parsed_nodes(tetgen_parser::internal::parse_tetgen_node_file(input, nullptr))) {
            throw std::runtime_error("TetGen node file comments aren't skipped");
        }
    }
}

static void test_parse_tetgen_elements() {
    // blank file fails
    {
        std::istringstream input("");
        EXPECT_ERROR(tetgen_parser::internal::parse_tetgen_ele_file(input, nullptr),
            "failed to parse TetGen ele file header");
    }
    // file has to start with the header line
    {
        std::istringstream input(
            "# some comment\n"
            "4 4 1\n" // header line
            "0 0 1 2 3 1\n"
            "1 1 2 3 4 1\n"
            "2 2 3 4 5 2\n"
            "3 3 4 5 6 2\n"
        );
        EXPECT_ERROR(tetgen_parser::internal::parse_tetgen_ele_file(input, nullptr),
            "failed to parse TetGen ele file header");
    }
    // num_nodes must be 4
    {
        std::istringstream input(
            "4 10 0\n" // header line
        );
        EXPECT_ERROR(tetgen_parser::internal::parse_tetgen_ele_file(input, nullptr),
            "TetGen ele file parsing failed, expected 4 nodes per tetrahedron");
    }
    // num_attr must be 1 (medium)
    {
        std::istringstream input(
            "4 4 0\n" // header line
        );
        EXPECT_ERROR(tetgen_parser::internal::parse_tetgen_ele_file(input, nullptr),
            "TetGen ele file parsing failed, expected each element to only have"
            " one attribute (EGSnrc medium)");
    }
    // Helper to check correctness of parsed elts
    auto check_parsed_elts = [](const std::vector<EGS_MeshSpec::Tetrahedron>& elts) -> bool {
        if (elts.size() != 4) { return false; }
        if (elts[0].tag != 0 || elts[0].a != 0 || elts[0].b != 1 || elts[0].c != 2 || elts[0].d != 3 || elts[0].medium_tag != 1) { return false; }
        if (elts[1].tag != 1 || elts[1].a != 1 || elts[1].b != 2 || elts[1].c != 3 || elts[1].d != 4 || elts[1].medium_tag != 1) { return false; }
        if (elts[2].tag != 2 || elts[2].a != 2 || elts[2].b != 3 || elts[2].c != 4 || elts[2].d != 5 || elts[2].medium_tag != 2) { return false; }
        if (elts[3].tag != 3 || elts[3].a != 3 || elts[3].b != 4 || elts[3].c != 5 || elts[3].d != 6 || elts[3].medium_tag != 2) { return false; }
        return true;
    };

    // ele file parsed successfully
    {
        std::istringstream input(
            "4 4 1\n" // header line
            "0 0 1 2 3 1\n"
            "1 1 2 3 4 1\n"
            "2 2 3 4 5 2\n"
            "3 3 4 5 6 2\n"
        );
        if (!check_parsed_elts(tetgen_parser::internal::parse_tetgen_ele_file(input, nullptr))) {
            throw std::runtime_error("TetGen ele file parsing failed");
        }
    }
    // comment lines starting with # in the body are skipped
    {
        std::istringstream input(
            "4 4 1\n" // header line
            "# some comment\n"
            "0 0 1 2 3 1\n"
            "1 1 2 3 4 1\n"
            "2 2 3 4 5 2\n"
            "3 3 4 5 6 2\n"
        );
        if (!check_parsed_elts(tetgen_parser::internal::parse_tetgen_ele_file(input, nullptr))) {
            throw std::runtime_error("TetGen ele file comments aren't skipped");
        }
    }
}

static void test_tetgen_elt_media() {
    // expecting two media:
    // EGS_MeshSpec::Medium(tag: 1, medium_name: "1")
    // EGS_MeshSpec::Medium(tag: 1, medium_name: "2")
    std::istringstream input(
            "4 4 1\n" // header line
            "# some comment\n"
            "0 0 1 2 3 1\n"
            "1 1 2 3 4 1\n"
            "2 2 3 4 5 2\n"
            "3 3 4 5 6 2\n"
        );
    auto elts = tetgen_parser::internal::parse_tetgen_ele_file(input, nullptr);
    auto media = tetgen_parser::internal::find_tetgen_elt_media(elts);
    if (media.size() != 2) {
        throw std::runtime_error("parsed wrong number of media, expected 2");
    }
    // Implementation uses std::set<int> so tags will be in numerical order. If
    // that changes, these checks could fail.
    if (media[0].tag != 1 || media[0].medium_name != "1") {
        throw std::runtime_error("TetGen media finding failed");
    }
    if (media[1].tag != 2 || media[1].medium_name != "2") {
        throw std::runtime_error("TetGen media finding failed");
    }
}

// Five elts (1-5), 8 nodes (1-8), 2 media (1, 2)
static std::string tetgen_elt_str =
R"(5 4 1
1 1 2 3 4 1
2 1 2 4 5 1
3 1 3 4 6 2
4 1 2 3 7 2
5 2 3 4 8 2)";

static std::string tetgen_node_str =
R"(8 3 0 0
1 0 0 0
2 1 0 0
3 0 1 0
4 0 0 1
5 0 -1 0
6 -1 0 0
7 0 0 -1
8 1 1 1)";

static void test_parse_tetgen_file_errors() {
    // This test uses two files: model.ele and model.node
    // Various possible file errors are tested for proper error-handling.
    egsSetInfoFunction(Warning, egsInfoThrowing);
    std::string tetgen_ele_egsinp(
        ":start geometry definition:\n"
        "    :start geometry:\n"
        "       name = my_mesh\n"
        "       library = egs_mesh\n"
        "       file = tmp_model.node\n"   // model.node not written out yet
        "    :stop geometry:\n"
        "    simulation geometry = my_mesh\n"
        ":stop geometry definition:\n");

    // Non-existent TetGen file fails (model.node not written out yet)
    {
        EGS_Input egsinp;
        egsinp.setContentFromString(tetgen_ele_egsinp);
        EXPECT_ERROR(EGS_Mesh::createGeometry(&egsinp),
            "\ncreateGeometry(EGS_Mesh): Tetgen node file `tmp_model.node` does not exist or is not readable\n");
    }
    // Write the node file to disk
    std::string filename = "tmp_model.node";
    TempFile tmp(filename, tetgen_node_str);

    // Missing ele file fails even if the node file exists
    {
        EGS_Input egsinp;
        egsinp.setContentFromString(tetgen_ele_egsinp);
        EXPECT_ERROR(EGS_Mesh::createGeometry(&egsinp),
            "\ncreateGeometry(EGS_Mesh): Tetgen ele file `tmp_model.ele` does not exist or is not readable\n");
    }
    // 3. Malformed TetGen files are caught
    {
        EGS_Input egsinp;
        egsinp.setContentFromString(tetgen_ele_egsinp);
        // Write the malformed ele file to disk
        std::string filename = "tmp_model.ele";
        // Use node file body for ele file to check parsing errors
        TempFile tmp(filename, tetgen_node_str);
        EXPECT_ERROR(EGS_Mesh::createGeometry(&egsinp),
            "\ncreateGeometry(EGS_Mesh): TetGen ele file parsing failed, expected 4 nodes per tetrahedron\n");
    }

    // Reset egsWarning
    egsSetDefaultIOFunctions();
}

static void test_parse_tetgen_file() {
    TempFile node_file("mesh.node", tetgen_node_str);
    TempFile ele_file("mesh.ele", tetgen_elt_str);

    EGS_Input egsinp;
    std::string egsinp_str(
        ":start geometry definition:\n"
        "    :start geometry:\n"
        "       name = my_mesh\n"
        "       library = egs_mesh\n"
        "       file = mesh.node\n"
        "    :stop geometry:\n"
        "    simulation geometry = my_mesh\n"
        ":stop geometry definition:\n"
    );
    egsinp.setContentFromString(egsinp_str);
    EGS_BaseGeometry *geo = EGS_Mesh::createGeometry(&egsinp);
    EGS_Mesh *mesh = dynamic_cast<EGS_Mesh*>(geo);
    if (!mesh) {
        throw std::runtime_error("dynamic_cast<EGS_Mesh*> failed!");
    }
    if (mesh->num_elements() != 5) {
        throw std::runtime_error("TetGen parser: expected 5 elements");
    }
    if (mesh->num_nodes() != 8) {
        throw std::runtime_error("TetGen parser: expected 8 nodes");
    }
    if (test_mesh.is_boundary(0)) {
        throw std::runtime_error("TetGen parser: expected region 0 not to be a boundary element");
    }
    for (auto i = 1; i < test_mesh.num_elements(); i++) {
        if (!test_mesh.is_boundary(i)) {
            throw std::runtime_error("TetGen parser: expected region " + std::to_string(i) +
                " to be a boundary element");
        }
    }
    // nodes
    if (!egsvec_eq(mesh->node_coordinates(0), EGS_Vector(0, 0, 0))) {
        throw std::runtime_error("bad coordinates for node 0");
    }
    if (!egsvec_eq(mesh->node_coordinates(1), EGS_Vector(1, 0, 0))) {
        throw std::runtime_error("bad coordinates for node 1");
    }
    if (!egsvec_eq(mesh->node_coordinates(2), EGS_Vector(0, 1, 0))) {
        throw std::runtime_error("bad coordinates for node 2");
    }
    if (!egsvec_eq(mesh->node_coordinates(3), EGS_Vector(0, 0, 1))) {
        throw std::runtime_error("bad coordinates for node 3");
    }
    if (!egsvec_eq(mesh->node_coordinates(4), EGS_Vector(0, -1, 0))) {
        throw std::runtime_error("bad coordinates for node 4");
    }
    if (!egsvec_eq(mesh->node_coordinates(5), EGS_Vector(-1, 0, 0))) {
        throw std::runtime_error("bad coordinates for node 5");
    }
    if (!egsvec_eq(mesh->node_coordinates(6), EGS_Vector(0, 0, -1))) {
        throw std::runtime_error("bad coordinates for node 6");
    }
    if (!egsvec_eq(mesh->node_coordinates(7), EGS_Vector(1, 1, 1))) {
        throw std::runtime_error("bad coordinates for node 7");
    }
    // elements
    if (mesh->element_node_offsets(0) != std::array<int, 4>{0, 1, 2, 3} || mesh->element_tag(0) != 1) {
        throw std::runtime_error("bad region 0");
    }
    if (mesh->element_node_offsets(1) != std::array<int, 4>{0, 1, 3, 4} || mesh->element_tag(1) != 2) {
        throw std::runtime_error("bad region 1");
    }
    if (mesh->element_node_offsets(2) != std::array<int, 4>{0, 2, 3, 5} || mesh->element_tag(2) != 3) {
        throw std::runtime_error("bad region 2");
    }
    if (mesh->element_node_offsets(3) != std::array<int, 4>{0, 1, 2, 6} || mesh->element_tag(3) != 4) {
        throw std::runtime_error("bad region 3");
    }
    if (mesh->element_node_offsets(4) != std::array<int, 4>{1, 2, 3, 7} || mesh->element_tag(4) != 5) {
        throw std::runtime_error("bad region 4");
    }
    // media
    for (auto i = 0; i < mesh->num_elements(); i++) {
        if (i < 2 && mesh->getMediumName(mesh->medium(i)) != std::string("1")) {
            std::cout << "i: `" << mesh->getMediumName(mesh->medium(i)) << "`n";
            throw std::runtime_error("expected elements 1, 2 to have media 1");
        }
        if (i >= 2 && mesh->getMediumName(mesh->medium(i)) != std::string("2")) {
            std::cout << "i: " << mesh->getMediumName(mesh->medium(i)) << "\n";
            throw std::runtime_error("expected elements 3, 4, 5 to have media 2");
        }
    }
}

static void test_unknown_mesh_file_extension() {
    egsSetInfoFunction(Warning, egsInfoThrowing);

    EGS_Input egsinp;
    std::string egsinp_str(
        ":start geometry definition:\n"
        "    :start geometry:\n"
        "       name = my_mesh\n"
        "       library = egs_mesh\n"
        "       file = mesh.bad_extension\n"
        "    :stop geometry:\n"
        "    simulation geometry = my_mesh\n"
        ":stop geometry definition:\n"
    );
    egsinp.setContentFromString(egsinp_str);
    EXPECT_ERROR(EGS_Mesh::createGeometry(&egsinp), "\ncreateGeometry(EGS_Mesh)"
        ": unknown extension for mesh file `mesh.bad_extension`, supported "
        "extensions are msh, ele, node\n");

    // Reset egsWarning
    egsSetDefaultIOFunctions();
}

int main() {

    test_howfar_interior();

    RUN_TEST(test_unknown_node());
    RUN_TEST(test_isWhere());
    RUN_TEST(test_medium());
    RUN_TEST(test_boundary());
    RUN_TEST(test_neighbours());
    RUN_TEST(test_hownear_interior());
    RUN_TEST(test_hownear_exterior());
    RUN_TEST(test_howfar_interior_basic());
    RUN_TEST(test_howfar_exterior());
    RUN_TEST(test_mesh_scaling());
    RUN_TEST(test_mesh_scale_key_errors());

    RUN_TEST(test_tetrahedron_face_eq());
    RUN_TEST(test_tetrahedron_errors());
    RUN_TEST(test_tetrahedron_neighbours());

    RUN_TEST(test_parse_msh_version());
    RUN_TEST(test_parse_msh41_groups());
    RUN_TEST(test_parse_msh41_node_bloc());
    RUN_TEST(test_parse_msh41_nodes());
    RUN_TEST(test_parse_msh41_entities());
    RUN_TEST(test_parse_msh41_element_bloc());
    RUN_TEST(test_parse_msh41_elements());

    MeshFile mesh_file;
    RUN_TEST(test_parse_msh41_file_errors(mesh_file));
    RUN_TEST(test_parse_msh41_file(mesh_file));

    RUN_TEST(test_parse_tetgen_nodes());
    RUN_TEST(test_parse_tetgen_elements());
    RUN_TEST(test_tetgen_elt_media());
    RUN_TEST(test_parse_tetgen_file_errors());
    RUN_TEST(test_parse_tetgen_file());

    RUN_TEST(test_unknown_mesh_file_extension());

    std::cerr << "\ntest result: " << num_total - num_failed << " out of " <<
        num_total << " tests passed\n";
    return num_failed;
}

#undef RUN_TEST
#undef EXPECT_ERROR
