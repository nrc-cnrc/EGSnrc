#ifndef NEIGHBOUR
#define NEIGHBOUR

#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mesh_neighbours {

// class Tetrahedron {
// public:
//     Tetrahedron(int a, int b, int c, int d) : _a(a), _b(b), _c(c), _d(d) {
//         // if (_a < 0,
//     }
// private:
//     int _a = -1;
//     int _b = -1;
//     int _c = -1;
//     int _d = -1;
// };

constexpr int NONE = -1;

inline void print_vec(const std::vector<int>& vec) {
    for (auto v: vec) {
        std::cout << v << " ";
    }
    std::cout << "\n";
}

struct EltsAroundPoints {
    std::vector<int> elt_list;
    std::vector<int> list_indices;
};

/// Get the sparse-dense node renumbering map.
std::unordered_map<int, int> renumber_sparse_nodes(const std::vector<int>& nodes) {
    auto uniq_nodes = nodes;
    std::sort(uniq_nodes.begin(), uniq_nodes.end());
    auto last = std::unique(uniq_nodes.begin(), uniq_nodes.end());
    uniq_nodes.erase(last, uniq_nodes.end());

    std::unordered_map<int, int> node_renum;
    node_renum.reserve(uniq_nodes.size());
    for (std::size_t i = 0; i < uniq_nodes.size(); ++i) {
        // renumbered nodes start at 1
        node_renum.insert({uniq_nodes[i], i + 1});
    }
    return node_renum;
}

// Find the elements around each node.
// Returns a list of elements and a list of indices into the elements.
//
// Adapted from Applied CFD Techniques section 2.2.1
template<int NODES_PER_ELT>
EltsAroundPoints elements_around_points(const std::vector<int>& nodes) {
    assert(nodes.size() % NODES_PER_ELT == 0);
    // the number of unique nodes is equal to the maximum node number
    // because the nodes are continuously numbered from 1..=max_node
    int num_unique_nodes = *std::max_element(nodes.begin(), nodes.end());
    // fill indices vector with 0
	std::vector<int> indices(num_unique_nodes + 1, 0);

    int num_elts = nodes.size() / NODES_PER_ELT;
    std::cout << "num elts:" << num_elts << "\n";

    // counts up the number of times a number idx node is used
    for (int i = 0; i < num_elts; ++i){
      for (int j = 0; j < NODES_PER_ELT; ++j){
          auto idx = nodes.at(i * NODES_PER_ELT + j);
          indices.at(idx) += 1;
      }
    }

    for (std::size_t i = 1; i < indices.size(); ++i){
      indices.at(i) += indices.at(i-1);
    }
    print_vec(indices);

    std::vector<int> eltList(indices.back(), -1);
    for (int i = 0; i < num_elts; ++i){
        for (int j = 0; j < NODES_PER_ELT; ++j){
            auto node = nodes[i * NODES_PER_ELT + j] - 1;
            auto node_pos = indices.at(node);
            eltList[node_pos] = i;
            indices.at(node) += 1;
        }
        print_vec(eltList);
    }

    // shift indices vector right by 1
    for (std::size_t i = indices.size()-1; i > 0; --i){
        indices[i] = indices[i-1];
    }
    indices[0] = 0;

    print_vec(eltList);

    for (std::size_t i = 0; i < indices.size() - 1; ++i) {
        auto num_elts = indices[i+1] - indices[i];
        std::cout << "node " << i + 1 << " is part of " << num_elts << " elements\n\t";
        for (int j = indices[i]; j < indices[i+1]; ++j) {
            std::cout << eltList[j] << " ";
        }
        std::cout << "\n" ;
    }

	return EltsAroundPoints { eltList, indices };
}

// Given a list of node numbers starting from 1, returns the element neighbours.
//
// Adapted from Applied CFD Techniques section 2.2.3
template<int NODES_PER_ELT, int FACES_PER_ELT, int NODES_PER_FACE>
std::vector<int> element_neighbours(const std::vector<int>& element_nodes) {
    // this implementation requires one less node per face than the number of element nodes
    assert(NODES_PER_FACE == NODES_PER_ELT - 1);

    const auto elt_indices = elements_around_points<NODES_PER_ELT>(element_nodes);
    const auto& eltList = elt_indices.elt_list;
    const auto& indices = elt_indices.list_indices;
    const int num_unique_nodes = indices.size() - 1;

    const int num_elts = element_nodes.size() / NODES_PER_ELT;
    // initialize neighbour element index vector with "no neighbour" constant
    std::vector<int> neighbours(num_elts * FACES_PER_ELT, mesh_neighbours::NONE);
    std::array<int, NODES_PER_FACE> face;

    // During the neighbour search, elements at face point indices of this vector
    // are set to 1. Elements are added together and if the result is equal to the
    // number of nodes per face we've found a face neighbour.
    std::vector<int> face_point_flag(num_unique_nodes, 0);

    for (int i = 0; i < num_elts; i++) {
        for (int j = 0; j < FACES_PER_ELT; j++){
            // if this face's neighbour was already found, skip it
            if (neighbours[i * FACES_PER_ELT + j] != NONE) {
                continue;
            }
            int face_node_idx = 0;
            for (int k = 0; k < NODES_PER_ELT; k++) {
                // face 0 is made of nodes {1, 2, 3}, 1 is made of nodes {0, 2, 3}, etc.
                if (k != j) {
                    auto nd = element_nodes[i * NODES_PER_ELT + k];
                    face[face_node_idx] = nd;
                    face_node_idx += 1;
                    // set face_point_flag value for this node
                    face_point_flag[nd - 1] = 1;
                }
            }
            std::cout << "face " << j << " is made of points { ";
            for (auto n: face) {
                std::cout << n << " ";
            }
            std::cout << "}\n";
            // select a point of this face and loop over the other elements that have it
            // -- any point will work, since to be a neighbour other elements
            //    must have the same list of face points
            auto face_point = face[0];
            for (int elt_idx = indices[face_point - 1]; elt_idx < indices[face_point]; elt_idx++) {
                auto elt = eltList.at(elt_idx);
                if (elt == i) {
                    // elt can't be a neighbour of itself, skip it
                    continue;
                }
                // to match, all face nodes must match the reference face nodes
                for (int other_face_idx = 0; other_face_idx < FACES_PER_ELT; other_face_idx++) {
                    int num_shared = 0;
                    for (int other_face_point = 0; other_face_point < NODES_PER_ELT; other_face_point++) {
                        if (other_face_point == other_face_idx) {
                            continue;
                        }
                        auto other_face_node = element_nodes[elt * NODES_PER_ELT + other_face_point];
                        num_shared += face_point_flag[other_face_node - 1];
                    }
                    if (num_shared == NODES_PER_FACE) {
                        // mark neighbouring elements as neighbours of each other
                        neighbours[i * FACES_PER_ELT + j] = elt;
                        neighbours[elt * FACES_PER_ELT + other_face_idx] = i;
                    }
                }
            }
            // reset the face_point_flag elements
            for (int pt: face) {
                face_point_flag[pt-1] = 0;
            }
        }
    }
    return neighbours;
};

const auto triangle_neighbours = element_neighbours<
    3, // NODES_PER_ELT
    3, // FACES_PER_ELT
    2  // NODES_PER_FACE
>;

const auto tetrahedron_neighbours = element_neighbours<
    4, // NODES_PER_ELT
    4, // FACES_PER_ELT
    3  // NODES_PER_FACE
>;

} // namespace mesh_neighbours
#endif
