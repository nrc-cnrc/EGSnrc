/*
###############################################################################
#
#  EGSnrc tetrahedral mesh nlogn nearest-neighbour algorithm
#  Copyright (C) 2020 Mevex Corporation
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
#  Authors:          Dave Macrillo,
#                    Matt Ronan,
#                    Nigel Vezeau,
#                    Lou Thompson,
#                    Max Orok
#
###############################################################################
*/

#ifndef MESH_NEIGHBOURS_
#define MESH_NEIGHBOURS_

#include <algorithm>
#include <array>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace mesh_neighbours {

// Magic number for no neighbour.
constexpr std::size_t NONE = -1;

class Tetrahedron {
public:
    using Face = std::array<std::size_t, 3>;

    // Make a tetrahedron from four nodes.
    //
    // Throws a std::invalid_argument exception if duplicate node tags are passed in.
    Tetrahedron(std::size_t a, std::size_t b, std::size_t c, std::size_t d) {
        if (a == b || a == c || a == d) {
            throw std::invalid_argument("duplicate node " + std::to_string(a));
        }
        if (b == c || b == d) {
            throw std::invalid_argument("duplicate node " + std::to_string(b));
        }
        if (c == d) {
            throw std::invalid_argument("duplicate node " + std::to_string(c));
        }
        std::vector<std::size_t> sorted {a, b, c, d};
        std::sort(sorted.begin(), sorted.end());
        _a = sorted[0];
        _b = sorted[1];
        _c = sorted[2];
        _d = sorted[3];
    }
    std::array<std::size_t, 4> nodes() const {
        return std::array<std::size_t, 4> {_a, _b, _c, _d};
    }
    std::size_t max_node() const {
        return _d;
    }
    std::array<Face, 4> faces() const {
        return {
            std::array<std::size_t, 3>{_b, _c, _d},
            std::array<std::size_t, 3>{_a, _c, _d},
            std::array<std::size_t, 3>{_a, _b, _d},
            std::array<std::size_t, 3>{_a, _b, _c}
        };
    }

private:
    std::size_t _a;
    std::size_t _b;
    std::size_t _c;
    std::size_t _d;
};

/// The mesh_neighbours::internal namespace is for internal API functions and is not
/// part of the public API. Functions and types may change without warning.
namespace internal {

class SharedNodes {
public:
    SharedNodes(std::vector<std::vector<std::size_t>> shared_nodes) :
        shared_nodes(std::move(shared_nodes)) {}
    const std::vector<std::size_t>& elements_around_node(std::size_t node) const {
        return shared_nodes.at(node);
    }
private:
    std::vector<std::vector<std::size_t>> shared_nodes;
};

// Find the elements around each node.
SharedNodes elements_around_nodes(const std::vector<mesh_neighbours::Tetrahedron>& elements) {
    std::size_t max_node = 0;
    for (const auto& elt: elements) {
        if (elt.max_node() > max_node) {
            max_node = elt.max_node();
        }
    }

    // the number of unique nodes is equal to the maximum node number + 1
    // because the nodes are numbered from 0..=max_node
    std::vector<std::vector<std::size_t>> shared_nodes(max_node + 1);
    for (std::size_t i = 0; i < elements.size(); i++) {
        for (auto node: elements[i].nodes()) {
            shared_nodes.at(node).push_back(i);
        }
    }
    return SharedNodes(shared_nodes);
}

} // namespace internal

// Given a list of tetrahedrons, returns the indices of neighbouring tetrahedrons.
std::vector<std::array<std::size_t,4>> tetrahedron_neighbours(
        const std::vector<mesh_neighbours::Tetrahedron>& elements)
{
    const std::size_t NUM_FACES = 4;
    const auto shared_nodes = mesh_neighbours::internal::elements_around_nodes(elements);

    // initialize neighbour element index vector with "no neighbour" constant
    std::vector<std::array<std::size_t, 4>> neighbours(elements.size(), {NONE, NONE, NONE, NONE});

    for (std::size_t i = 0; i < elements.size(); i++) {
        auto elt_faces = elements[i].faces();
        for (std::size_t f = 0; f < NUM_FACES; f++) {
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
                for (std::size_t jf = 0; jf < NUM_FACES; jf++) {
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
#endif // MESH_NEIGHBOURS_
