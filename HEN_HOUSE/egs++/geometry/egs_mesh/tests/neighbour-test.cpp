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

int test_triangle_neighbours() {
    std::vector<int> disjoint_triangles {1, 2, 3, 4, 5, 6};
    assert((mesh_neighbours::triangle_neighbours(disjoint_triangles) ==
        std::vector<int>{
            NONE, NONE, NONE,
            NONE, NONE, NONE
        }
    ));

    std::vector<int> two_triangles {1, 2, 3, 1, 2, 4};
    assert((mesh_neighbours::triangle_neighbours(two_triangles) ==
        std::vector<int>{
            NONE, NONE, 1,
            NONE, NONE, 0,
        }
    ));
    return 0;
}

int test_tetrahedron_neighbours() {
    std::vector<int> disjoint_tets {1, 2, 3, 4, 5, 6, 7, 8};
    assert((mesh_neighbours::tetrahedron_neighbours(disjoint_tets) ==
        std::vector<int>{
            NONE, NONE, NONE, NONE,
            NONE, NONE, NONE, NONE
        }
    ));
    std::vector<int> linked_tets {1, 2, 3, 4, 1, 2, 3, 5};
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
    test_triangle_neighbours();
}
