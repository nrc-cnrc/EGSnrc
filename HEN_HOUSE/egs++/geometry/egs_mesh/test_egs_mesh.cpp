// TODO add test for intersection point right on element node

#include "egs_mesh.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace {

const EGS_Float eps = 1e-8;

static inline bool approx_eq(double a, double b, double e = eps) {
    return (std::abs(a - b) <= e * (std::abs(a) + std::abs(b) + 1.0));
}

std::string to_string_with_precision(double d, const int n = 17)
{
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << d;
    return out.str();
}

} // anonymous namespace

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

// Corner case test: behaviour of particle travelling parallel and very near
// boundary surface:
//
//          /
//         /  a
//    *->  ------
//         \  b
//          \
//
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
    std::cout << "before howfar\n";
    for (std::size_t i = 0; i < xs.size(); i++) {
        EGS_Float dist = 1e30;
        reg = mesh.howfar(reg, xs[i], us[i], dist, nullptr);
        //if (reg == -1) { throw std::runtime_error("clipped to outside"); }
        std::cout << "isWhere(x) = " << mesh.isWhere(xs[i]) << "\n";
        std::cout << "reg = " << reg << "\n";
        std::cout << "dist = " << dist << "\n";
    }
    std::cout << "after howfar\n";
    std::ofstream msh("howfar-fail.msh");
    msh << std::setprecision(std::numeric_limits<double>::max_digits10);
    msh << "$MeshFormat\n";
    msh << "2.2 0 8\n";
    msh << "$EndMeshFormat\n";
    msh << "$Nodes\n";
    msh << "10\n";
    msh << 1 << " " << n1.x << " " << n1.y << " " << n1.z << "\n";
    msh << 2 << " " << n2.x << " " << n2.y << " " << n2.z << "\n";
    msh << 3 << " " << n3.x << " " << n3.y << " " << n3.z << "\n";
    msh << 4 << " " << n4.x << " " << n4.y << " " << n4.z << "\n";
    msh << 5 << " " << n5.x << " " << n5.y << " " << n5.z << "\n";
    msh << 6 << " " << xs[0].x << " " << xs[0].y << " " << xs[0].z << "\n";
    msh << 7 << " " << xs[1].x << " " << xs[1].y << " " << xs[1].z << "\n";
    msh << 8 << " " << xs[2].x << " " << xs[2].y << " " << xs[2].z << "\n";
    msh << 9 << " " << xs[3].x << " " << xs[3].y << " " << xs[3].z << "\n";
    msh << 10 << " " << xs[4].x << " " << xs[4].y << " " << xs[4].z << "\n";
    msh << "$EndNodes\n";
    msh << "$Elements\n";
    msh << "6\n";
    msh << "1 4 2 0 0 1 2 3 4\n";
    msh << "2 4 2 0 0 2 3 4 5\n";
    msh << "3 1 2 0 0 6 7\n";
    msh << "4 1 2 0 0 7 8\n";
    msh << "5 1 2 0 0 8 9\n";
    msh << "6 1 2 0 0 9 10\n";
    msh << "$EndElements\n";
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
    int newreg = mesh.howfar(0, x, u, dist, &newmed);
    std::cout << "dist: " << dist << "\n";
    if (dist < 0.0) {
        throw std::runtime_error("test failed, got negative intersection");
    }

}

static void test_howfar_interior() {
    // Create a simple two-element mesh and test the three howfar_interior cases
    //
    //            e1      e2
    //                b,c
    //                /|\
    //   y           / | \
    //   ^          /  |  \
    //   |         a   |   e
    //   +--> x     \  |  /
    //               \ | /
    //                \|/
    //                 d
    //
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
    EGS_Mesh mesh(std::move(spec));

    try {
        test_howfar_interior_regular(mesh);
        std::cout << "test_howfar_interior_regular PASSED\n";
    } catch (const std::runtime_error &e) {
        throw std::runtime_error(std::string(
            "test_howfar_interior_regular failed: ") + e.what());
    }
    try {
        test_howfar_interior_outside_thick_plane(mesh);
        std::cout << "test_howfar_interior_outside_thick_plane PASSED\n";
    } catch (const std::runtime_error &e) {
        throw std::runtime_error(std::string(
            "test_howfar_interior_outside_thick_plane failed: ") + e.what());
    }
    try {
        test_howfar_interior_lost_particle(mesh);
        std::cout << "test_howfar_interior_lost_particle PASSED\n";
    } catch (const std::runtime_error &e) {
        throw std::runtime_error(std::string(
            "test_howfar_interior_lost_particle failed: ") + e.what());
    }
    try {
        test_howfar_interior_tolerance(mesh);
        std::cout << "test_howfar_interior_tolerance PASSED\n";
    } catch (const std::runtime_error &e) {
        throw std::runtime_error(std::string(
            "test_howfar_interior_tolerance failed: ") + e.what());
    }
    try {
        test_howfar_interior_reentry();
        std::cout << "test_howfar_interior_reentry PASSED\n";
    } catch (const std::runtime_error &e) {
        throw std::runtime_error(std::string(
            "test_howfar_interior_reentry failed: ") + e.what());
    }
    try {
        test_howfar_interior_boundary_straddle();
        std::cout << "test_howfar_interior_boundary_straddle PASSED\n";
    } catch (const std::runtime_error &e) {
        throw std::runtime_error(std::string(
            "test_howfar_interior_boundary_straddle failed: ") + e.what());
    }
    try {
        test_howfar_interior_stuck_on_boundary();
        std::cout << "test_howfar_interior_stuck_on_boundary PASSED\n";
    } catch (const std::runtime_error &e) {
        throw std::runtime_error(std::string(
            "test_howfar_interior_stuck_on_boundary failed: ") + e.what());
    }
    try {
        test_howfar_interior_thick_plane_negative_intersection();
        std::cout << "test_howfar_interior_thick_plane_negative_intersection PASSED\n";
    } catch (const std::runtime_error &e) {
        throw std::runtime_error(std::string(
            "test_howfar_interior_thick_plane_negative_intersection failed: ") + e.what());
    }
}

int main() {
    test_howfar_interior();
    std::cout << "OK\n";
    return 0;
}
