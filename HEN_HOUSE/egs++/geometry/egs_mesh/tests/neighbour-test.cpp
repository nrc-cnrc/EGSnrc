#include "neighbour.h"
#include <cassert>

using mesh_neighbours::NONE;

int test_node_renumbering() {
    std::vector<int> nodes {1000, 2, 3, 6, 1, 2, 5};
    auto renumbering = mesh_neighbours::renumber_sparse_nodes(nodes);
    assert(renumbering.size() == 6);
    assert(renumbering.at(1) == 1);
    assert(renumbering.at(2) == 2);
    assert(renumbering.at(3) == 3);
    assert(renumbering.at(5) == 4);
    assert(renumbering.at(6) == 5);
    assert(renumbering.at(1000) == 6);
    return 0;
}

int test_tetrahedron_neighbours() {
    using mesh_neighbours::Tetrahedron;
    std::vector<Tetrahedron> disjoint_tets {
        Tetrahedron(1, 2, 3, 4),
        Tetrahedron(5, 6, 7, 8)
    };
    assert((mesh_neighbours::tetrahedron_neighbours(disjoint_tets) ==
        std::vector<int>{
            NONE, NONE, NONE, NONE,
            NONE, NONE, NONE, NONE
        }
    ));
    std::vector<Tetrahedron> linked_tets {
        Tetrahedron(1, 2, 3, 4),
        Tetrahedron(1, 2, 3, 5)
    };
    assert((mesh_neighbours::tetrahedron_neighbours(linked_tets) ==
        std::vector<int>{
            NONE, NONE, NONE, 1,
            NONE, NONE, NONE, 0
        }
    ));
    return 0;
}

int main() {
    test_node_renumbering();
    test_tetrahedron_neighbours();
}
