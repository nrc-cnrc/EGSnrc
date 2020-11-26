#include "neighbour.h"
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
        for (int i = 0; i < 4; i++) {
            assert(a_faces[i] == b_faces[i]);
        }
    }
    // tetrahedrons with three shared nodes have 1 face in common
    {
        Tetrahedron a(1, 2, 3, 4);
        Tetrahedron b(5, 2, 3, 1);
        auto a_faces = a.faces();
        auto b_faces = b.faces();
        Tetrahedron::Face shared_face;
        int num_shared = 0;
        for (int i = 0; i < 4; i++) {
            if (a_faces[i] == b_faces[i]) {
                num_shared += 1;
                shared_face = b_faces[i];
            }
        }
        assert(num_shared == 1);
        assert((shared_face == Tetrahedron::Face {1, 2, 3}));
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
        std::vector<std::array<std::size_t,4>>{
            std::array<std::size_t, 4>{NONE, NONE, NONE, NONE},
            std::array<std::size_t, 4>{NONE, NONE, NONE, NONE}
        }
    ));
    std::vector<Tetrahedron> linked_tets {
        Tetrahedron(1, 2, 3, 4),
        Tetrahedron(1, 2, 3, 5)
    };
    assert((mesh_neighbours::tetrahedron_neighbours(linked_tets) ==
        std::vector<std::array<std::size_t,4>>{
            std::array<std::size_t, 4>{NONE, NONE, NONE, 1},
            std::array<std::size_t, 4>{NONE, NONE, NONE, 0}
        }
    ));
    // 0 nodes are OK
    std::vector<Tetrahedron> tets_with_0 {
        Tetrahedron(0, 2, 3, 4),
        Tetrahedron(1, 2, 3, 4)
    };
    assert((mesh_neighbours::tetrahedron_neighbours(tets_with_0) ==
        std::vector<std::array<std::size_t,4>>{
            std::array<std::size_t, 4>{1, NONE, NONE, NONE},
            std::array<std::size_t, 4>{0, NONE, NONE, NONE}
        }
    ));

    return 0;
}

int main() {
    test_tetrahedron_face_eq();
    test_tetrahedron_errors();
    test_tetrahedron_neighbours();
}
