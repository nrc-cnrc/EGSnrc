#include "egs_mesh.h"
#include "egs_vector.h"

#include <algorithm>
#include <cassert>
#include <fstream>

#define RUN_TEST(test_fn) \
    std::cerr << "starting test " << #test_fn << std::endl; \
    err = test_fn; \
    num_total++; \
    if (err) { \
        std::cerr << "test FAILED" << std::endl; \
        num_failed++; \
    } else { \
        std::cerr << "test passed" << std::endl; \
    }

#define EXPECT_ERROR(stmt, err_msg) \
    try { \
        stmt; \
        std::cerr << "expected exception with message: \"" << err_msg << "\"\n"; \
        return 1; \
    } catch (const std::runtime_error& err) { \
        if (err.what() != std::string(err_msg)) { \
            std::cerr << "got error message: \"" \
                << err.what() << "\"\nbut expected: \"" << err_msg << "\"\n"; \
            return 1; \
        } \
    }

bool approx_eq(double a, double b, double eps = 1e-6) {
    return (std::abs(a - b) <= eps * (std::abs(a) + std::abs(b) + 1.0));
}

// we'll use a simple five-element mesh for smoke testing
//
// Not declared const because of EGS_BaseGeometry method requirements but
// isn't mutated at any point.
static EGS_Mesh test_mesh = [](){
    std::ifstream input("five-tet.msh");
    return EGS_Mesh::parse_msh_file(input);
}();

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

std::vector<Tet> get_tetrahedrons(const EGS_Mesh& mesh) {
    std::vector<Tet> elts;
    elts.reserve(mesh.num_elements());
    auto points = mesh.points();
    for (auto i = 0; i < mesh.num_elements(); i++) {
        elts.emplace_back(Tet(points[4*i], points[4*i+1], points[4*i+2], points[4*i+3]));
    }
    return elts;
}

void print_egsvec(const EGS_Vector& v) {
    std::cout << "{\n  x: " << v.x << "\n  y: " << v.y << "\n  z: " << v.z << "\n}\n";
}

int test_unknown_node() {
    std::vector<EGS_Mesh::Tetrahedron> elt { EGS_Mesh::Tetrahedron(0, 0, 1, 2, 100) };
    // no node 100 in nodes vector
    std::vector<EGS_Mesh::Node> nodes {
        EGS_Mesh::Node(0, 1.0, 1.0, -1.0),
        EGS_Mesh::Node(1, -1.0, 1.0, -1.0),
        EGS_Mesh::Node(2, 0.0, -1.0, -1.0),
        EGS_Mesh::Node(3, 0.0, 0.0, 1.0)
    };
    std::vector<EGS_Mesh::Medium> media { EGS_Mesh::Medium(1, "") };
    EXPECT_ERROR(EGS_Mesh mesh(elt, nodes, media), "No mesh node with tag: 100");
    return 0;
}

int test_boundary() {
    // element 0 is surrounded by the other four elements
    if (test_mesh.is_boundary() != std::vector<bool>{false, true, true, true, true}) {
        return 1;
    }
    return 0;
}

int test_neighbours() {
    // element 0 is neighbours with the other four elements
    auto neighbours = test_mesh.neighbours();
    auto n0 = neighbours.at(0);
    assert(std::count(n0.begin(), n0.end(), 1) == 1);
    assert(std::count(n0.begin(), n0.end(), 2) == 1);
    assert(std::count(n0.begin(), n0.end(), 3) == 1);
    assert(std::count(n0.begin(), n0.end(), 4) == 1);

    for (auto ns = neighbours.begin() + 1; ns != neighbours.end(); ns++) {
        assert(std::count(ns->begin(), ns->end(), 0) == 1);
        assert(std::count(ns->begin(), ns->end(), -1) == 3);
    }

    return 0;
}

int test_isWhere() {
    // test the centroid of each tetrahedron is inside the tetrahedron
    auto elts = get_tetrahedrons(test_mesh);
    for (int i = 0; i < (int)elts.size(); i++) {
        auto c = elts.at(i).centroid();
        auto in_tet = test_mesh.isWhere(c);
        if (in_tet != i) {
            std::cerr << "expected point to be in tetrahedron " << i << " got: " << in_tet << "\n";
            return 1;
        }
    }
    EGS_Vector out(10e10, 0, 0);
    if (test_mesh.isWhere(out) != -1) {
        std::cerr << "expected point to be outside (-1), got: " << test_mesh.isWhere(out) << "\n";
        return 1;
    }

    return 0;
}

int test_hownear_interior() {
    auto elts = get_tetrahedrons(test_mesh);
    for (int i = 0; i < (int)elts.size(); i++) {
        auto c = elts.at(i).centroid();
        auto dist = test_mesh.hownear(i, c);
        if (i < 4 && !approx_eq(dist, 0.144338)) {
            std::cerr << "expected min distance to be 0.144338, got: " << dist << "\n";
            return 1;
        } else if (i == 4 && !approx_eq(dist, 0.288675)) {
            std::cerr << "expected min distance to be 0.288675, got: " << dist << "\n";
            return 1;
        } else if (i > 5) {
            // test specific to five-tet.msh
            std::cerr << "unknown mesh file for hownear test\n";
            return 1;
        }
    }
    return 0;
}

int test_hownear_exterior() {
    // known point 1.0 away from tetrahedron 1 with point (0.0, -1.0, 0.0)
    {
        EGS_Vector x(0.0, -2.0, 0.0);
        auto dist = test_mesh.hownear(-1, x);
        if (!approx_eq(1.0, dist)) {
            std::cerr << "expected min distance to be 1.0, got: " << dist << "\n";
            return 1;
        }
    }
    // known point 10.0 away from tetrahedron 3 with point (0.0, 0.0, -1.0)
    {
        EGS_Vector x(0.0, 0.0, -11.0);
        auto dist = test_mesh.hownear(-1, x);
        if (!approx_eq(10.0, dist)) {
            std::cerr << "expected min distance to be 10.0, got: " << dist << "\n";
            return 1;
        }
    }
    return 0;
}

int test_medium() {
    for (auto i = 0; i < test_mesh.num_elements(); i++) {
        if (0 != test_mesh.medium(i)) {
            std::cerr << "expected medium index to be 0, got: " << test_mesh.medium(i) << "\n";
            return 1;
        }
    }
    return 0;
}

int main() {
    int num_failed = 0;
    int num_total = 0;
    int err = 0;

    RUN_TEST(test_unknown_node());
    RUN_TEST(test_isWhere());
    RUN_TEST(test_medium());
    RUN_TEST(test_boundary());
    RUN_TEST(test_neighbours());
    RUN_TEST(test_hownear_interior());
    RUN_TEST(test_hownear_exterior());

    std::cerr << num_total - num_failed << " out of " << num_total << " tests passed\n";
    return num_failed;
}
