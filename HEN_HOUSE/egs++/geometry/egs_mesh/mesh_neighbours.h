/*
###############################################################################
#
#  EGSnrc tetrahedral mesh nearest-neighbour algorithm
#
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
#  Authors:          Dave Macrillo, 2020
#                    Matt Ronan,
#                    Nigel Vezeau,
#                    Lou Thompson,
#                    Max Orok
#
#  Contributors:
#
###############################################################################
*/

// exclude from doxygen
/// @cond

#ifndef MESH_NEIGHBOURS_
#define MESH_NEIGHBOURS_

#include "egs_mesh.h"

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
    class Face {
    public:
        Face() {}
        Face(int a, int b, int c) {
            std::array<int, 3> sorted {a, b, c};
            // sort to ease comparison between faces
            std::sort(sorted.begin(), sorted.end());
            nodes_ = sorted;
        }
        int node0() const {
            return nodes_[0];
        }
        friend bool operator==(const Face &a, const Face &b);
        friend bool operator!=(const Face &a, const Face &b);
    private:
        std::array<int, 3> nodes_;
    };

    // Make a tetrahedron from four nodes.
    //
    // Throws std::runtime_error if duplicate node tags are passed in.
    Tetrahedron(int a, int b, int c, int d)
        : nodes_({
        a, b, c, d
    }) {
        if (a == b || a == c || a == d) {
            throw std::runtime_error("duplicate node " + std::to_string(a));
        }
        if (b == c || b == d) {
            throw std::runtime_error("duplicate node " + std::to_string(b));
        }
        if (c == d) {
            throw std::runtime_error("duplicate node " + std::to_string(c));
        }
        // Node ordering is important here. Face 0 is missing node 1, Face 1
        // is missing node 2, etc. This will be used later in the particle
        // transport methods EGS_Mesh::howfar and EGS_Mesh::hownear.
        faces_ = {
            Face(b, c, d),
            Face(a, c, d),
            Face(a, b, d),
            Face(a, b, c)
        };
    }
    std::array<int, 4> nodes() const {
        return nodes_;
    }
    int max_node() const {
        return *std::max_element(nodes_.begin(), nodes_.end());
    }
    std::array<Face, 4> faces() const {
        return faces_;
    }

private:
    std::array<int, 4> nodes_;
    std::array<Face, 4> faces_;
};

bool operator==(const Tetrahedron::Face &a, const Tetrahedron::Face &b) {
    return a.nodes_ == b.nodes_;
}

bool operator!=(const Tetrahedron::Face &a, const Tetrahedron::Face &b) {
    return a.nodes_ != b.nodes_;
}

/// The mesh_neighbours::internal namespace is for internal API functions and is not
/// part of the public API. Functions and types may change without warning.
namespace internal {

class SharedNodes {
public:
    SharedNodes(std::vector<std::vector<int>> shared_nodes) :
        shared_nodes(std::move(shared_nodes)) {}
    const std::vector<int> &elements_around_node(int node) const {
        return shared_nodes.at(node);
    }
private:
    std::vector<std::vector<int>> shared_nodes;
};

// Find the elements around each node.
SharedNodes elements_around_nodes(const std::vector<mesh_neighbours::Tetrahedron> &elements) {
    int max_node = 0;
    for (const auto &elt: elements) {
        if (elt.max_node() > max_node) {
            max_node = elt.max_node();
        }
    }

    // the number of unique nodes is equal to the maximum node number + 1
    // because the nodes are numbered from 0..=max_node
    std::vector<std::vector<int>> shared_nodes(max_node + 1);
    for (std::size_t i = 0; i < elements.size(); i++) {
        for (auto node: elements[i].nodes()) {
            shared_nodes.at(node).push_back(i);
        }
    }
    return SharedNodes(shared_nodes);
}

} // namespace internal

// Given a list of tetrahedrons, returns the indices of neighbouring tetrahedrons.
std::vector<std::array<int, 4>> tetrahedron_neighbours(
                                 const std::vector<mesh_neighbours::Tetrahedron> &elements,
egs_mesh::internal::PercentCounter &progress) {
    progress.start(elements.size());
    const std::size_t NUM_FACES = 4;
    const auto shared_nodes = mesh_neighbours::internal::elements_around_nodes(elements);

    // initialize neighbour element index vector with "no neighbour" constant
    std::vector<std::array<int, 4>> neighbours(elements.size(), {NONE, NONE, NONE, NONE});

    for (std::size_t i = 0; i < elements.size(); i++) {
        auto elt_faces = elements[i].faces();
        for (std::size_t f = 0; f < NUM_FACES; f++) {
            // if this face's neighbour was already found, skip it
            if (neighbours[i][f] != NONE) {
                continue;
            }
            auto face = elt_faces[f];
            // select a face node and loop through the other elements that share it
            const auto &elts_sharing_node = shared_nodes.elements_around_node(face.node0());
            for (auto j: elts_sharing_node) {
                if (j == static_cast<int>(i)) {
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
        progress.step(1);
    }
    return neighbours;
};

} // namespace mesh_neighbours
#endif // MESH_NEIGHBOURS_

/// @endcond
