#ifndef NEIGHBOUR
#define NEIGHBOUR

#include <algorithm>
#include <array>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace mesh_neighbours {

// Magic number for no neighbour.
constexpr int NONE = -1;

class Tetrahedron {
public:
    using Face = std::array<int, 3>;

    // Make a tetrahedron from four nodes.
    //
    // Throws a std::invalid_argument exception if:
    // * negative node tags are passed in or,
    // * duplicate node tags are passed in.
    Tetrahedron(int a, int b, int c, int d) {
        if (a < 0) { throw std::invalid_argument("negative node " + std::to_string(a)); }
        if (b < 0) { throw std::invalid_argument("negative node " + std::to_string(b)); }
        if (c < 0) { throw std::invalid_argument("negative node " + std::to_string(c)); }
        if (d < 0) { throw std::invalid_argument("negative node " + std::to_string(d)); }
        if (a == b || a == c || a == d) {
            throw std::invalid_argument("duplicate node " + std::to_string(a));
        }
        if (b == c || b == d) {
            throw std::invalid_argument("duplicate node " + std::to_string(b));
        }
        if (c == d) {
            throw std::invalid_argument("duplicate node " + std::to_string(c));
        }
        std::vector<int> sorted {a, b, c, d};
        std::sort(sorted.begin(), sorted.end());
        _a = sorted[0];
        _b = sorted[1];
        _c = sorted[2];
        _d = sorted[3];
    }
    std::array<int, 4> nodes() const {
        return std::array<int, 4> {_a, _b, _c, _d};
    }
    int max_node() const {
        return _d;
    }
    std::array<Face, 4> faces() const {
        return {
            std::array<int, 3>{_b, _c, _d},
            std::array<int, 3>{_a, _c, _d},
            std::array<int, 3>{_a, _b, _d},
            std::array<int, 3>{_a, _b, _c}
        };
    }

private:
    int _a = -1;
    int _b = -1;
    int _c = -1;
    int _d = -1;
};

/// The mesh_neighbours::internal namespace is for internal API functions and is not
/// part of the public API. Functions and types may change without warning.
namespace internal {

class SharedNodes {
public:
    SharedNodes(std::vector<std::vector<int>> shared_nodes) :
        shared_nodes(std::move(shared_nodes)) {}
    const std::vector<int>& elements_around_node(int node) const {
        return shared_nodes.at(node - 1);
    }
private:
    std::vector<std::vector<int>> shared_nodes;
};

// Find the elements around each node.
//
// Requires dense node numbering starting at 1.
SharedNodes elements_around_nodes(const std::vector<mesh_neighbours::Tetrahedron>& elements) {
    // the number of unique nodes is equal to the maximum node number
    // because the nodes are continuously numbered from 1..=max_node
    int max_node = -1;
    for (const auto& elt: elements) {
        if (elt.max_node() > max_node) {
            max_node = elt.max_node();
        }
    }

    std::vector<std::vector<int>> shared_nodes(max_node);
    for (std::size_t i = 0; i < elements.size(); i++) {
        for (auto node: elements[i].nodes()) {
            shared_nodes.at(node - 1).push_back(i);
        }
    }
    return SharedNodes(shared_nodes);
}

} // namespace internal

// Given a list of tetrahedrons, returns the indices of neighbouring tetrahedrons.
//
// Requires dense node numbering starting at 1.
std::vector<std::array<int,4>> tetrahedron_neighbours(
        const std::vector<mesh_neighbours::Tetrahedron>& elements)
{
    const int NUM_FACES = 4;
    const auto shared_nodes = mesh_neighbours::internal::elements_around_nodes(elements);

    // initialize neighbour element index vector with "no neighbour" constant
    std::vector<std::array<int, 4>> neighbours(elements.size(), {NONE, NONE, NONE, NONE});

    for (int i = 0; i < static_cast<int>(elements.size()); i++) {
        auto elt_faces = elements[i].faces();
        for (int f = 0; f < NUM_FACES; f++) {
            // if this face's neighbour was already found, skip it
            if (neighbours[i][f] != NONE) {
                continue;
            }
            auto face = elt_faces[f];
            // select a face node and loop through the other elements that share it
            const auto& elts_sharing_node = shared_nodes.elements_around_node(face[0]);
            for (auto j: elts_sharing_node) {
                if (j == i) {
                    // elt can't be a neighbour of itself, skip it
                    continue;
                }
                auto other_elt_faces = elements[j].faces();
                for (int jf = 0; jf < NUM_FACES; jf++) {
                    if (face == other_elt_faces[jf]) {
                        neighbours[i][f] = j;
                        neighbours[j][jf] = i;
                        break;
                    }
                }
            }
        }
    }
    return neighbours;
};

} // namespace mesh_neighbours
#endif
