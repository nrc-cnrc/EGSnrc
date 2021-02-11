#include "mesh_neighbours.h"
#include <cassert>

using mesh_neighbours::NONE;
using mesh_neighbours::Tetrahedron;

int test_tetrahedron_face_eq() {
    // tetrahedron faces with the same nodes will compare equal
    {
        Tetrahedron a(1, 2, 3, 4);
        Tetrahedron b(4, 2, 3, 1);
        auto a_faces = a.faces();
        auto b_faces = b.faces();
        assert(a_faces[0] == b_faces[3]);
        assert(a_faces[1] == b_faces[1]);
        assert(a_faces[2] == b_faces[2]);
        assert(a_faces[3] == b_faces[0]);
    }
    // tetrahedrons with three shared nodes have 1 face in common
    {
        Tetrahedron a(1, 2, 3, 4);
        Tetrahedron b(5, 2, 3, 1);
        auto a_faces = a.faces();
        auto b_faces = b.faces();
        assert(a_faces[0] != b_faces[3]);
        assert(a_faces[1] != b_faces[1]);
        assert(a_faces[2] != b_faces[2]);
        assert(a_faces[3] == b_faces[0]);
    }
    return 0;
}

int test_tetrahedron_errors() {
    // duplicate tetrahedron nodes are caught
    {
        try {
            Tetrahedron(1, 1, 2, 3);
            std::cerr << "expected exception on duplicate nodes\n";
            return 1;
        } catch (const std::invalid_argument& err) {
            std::string expected = "duplicate node 1";
            if (err.what() != expected) {
                std::cerr << "expected \"" << expected << "\", got \"" << err.what() << "\"\n";
                return 1;
            }
        }
        try {
            Tetrahedron(1, 2, 2, 3);
            std::cerr << "expected exception on duplicate nodes\n";
            return 1;
        } catch (const std::invalid_argument& err) {
            std::string expected = "duplicate node 2";
            if (err.what() != expected) {
                std::cerr << "expected \"" << expected << "\", got \"" << err.what() << "\"\n";
                return 1;
            }
        }
    }
    return 0;
}

int test_tetrahedron_neighbours() {
    std::vector<Tetrahedron> disjoint_tets {
        Tetrahedron(1, 2, 3, 4),
        Tetrahedron(5, 6, 7, 8)
    };
    assert((mesh_neighbours::tetrahedron_neighbours(disjoint_tets) ==
        std::vector<std::array<int, 4>>{
            std::array<int, 4>{NONE, NONE, NONE, NONE},
            std::array<int, 4>{NONE, NONE, NONE, NONE}
        }
    ));
    std::vector<Tetrahedron> linked_tets {
        Tetrahedron(1, 2, 3, 4),
        Tetrahedron(1, 2, 3, 5)
    };
    assert((mesh_neighbours::tetrahedron_neighbours(linked_tets) ==
        std::vector<std::array<int, 4>>{
            std::array<int, 4>{NONE, NONE, NONE, 1},
            std::array<int, 4>{NONE, NONE, NONE, 0}
        }
    ));
    // 0 nodes are OK
    std::vector<Tetrahedron> tets_with_0 {
        Tetrahedron(0, 2, 3, 4),
        Tetrahedron(1, 2, 3, 4)
    };
    assert((mesh_neighbours::tetrahedron_neighbours(tets_with_0) ==
        std::vector<std::array<int,4>>{
            std::array<int, 4>{1, NONE, NONE, NONE},
            std::array<int, 4>{0, NONE, NONE, NONE}
        }
    ));

    return 0;
}

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

int main() {
    int num_failed = 0;
    int num_total = 0;
    int err = 0;

    RUN_TEST(test_tetrahedron_face_eq());
    RUN_TEST(test_tetrahedron_errors());
    RUN_TEST(test_tetrahedron_neighbours());

    std::cerr << num_total - num_failed << " out of " << num_total << " tests passed\n";
    return num_failed;
}
