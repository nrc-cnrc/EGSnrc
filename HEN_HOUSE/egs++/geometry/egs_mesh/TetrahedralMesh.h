/*
###############################################################################
#
#  EGSnrc egs++ mesh geometry library TetrahedralMesh object.
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
#  Authors:          Dave Macrillo,
#                    Matt Ronan,
#                    Nigel Vezeau,
#                    Lou Thompson,
#                    Max Orok
#
###############################################################################
#
#  A finite-element mesh with tetrahedral elements.
#
###############################################################################
*/

#ifndef MESH
#define MESH

#include <algorithm>
#include <vector>
#include <tuple>
#include <map>
#include <iostream>
#include <stdio.h>
#include <string>
#include <fstream>
#include <sstream>
#include <cassert>

#include "neighbour.h"

// Per-element simulation settings
struct EGSnrcProperties {
    int medium = -1;
    double relative_rho = 1.0;

    EGSnrcProperties() = default;

    explicit EGSnrcProperties(int _medium) {
        medium = _medium;
    }

    std::string print() const {
        std::stringstream ss;
        ss << "medium = " << this->medium << ", relative_rho = " << this->relative_rho;
        return ss.str();
    }

};

// A single tetrahedral mesh element
struct Tetrahedron {

    struct Point {
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;

        Point() = default;
        Point(double _x, double _y, double _z): x(_x), y(_y), z(_z) {}
    };

    EGSnrcProperties properties;
    std::array<std::size_t, 4> nodes;
    // tag for external mesh IO (e.g. plots for each element)
    int tag = -1;

    explicit Tetrahedron(int _tag, std::array<std::size_t, 4> _nodes,
        EGSnrcProperties _properties):
            properties(_properties), nodes(_nodes), tag(_tag) {}

    int medium() const {
        return this->properties.medium;
    }

    double relative_density() const {
        return this->properties.relative_rho;
    }

    std::string print() const {
        std::stringstream ss;
        ss << "Tetrahedron { nodes: {"
           << nodes[0] << ", "
           << nodes[1] << ", "
           << nodes[2] << ", "
           << nodes[3] << "} , properties: { " << this->properties.print() << "} }";
        return ss.str();
    }
};

class TetrahedralMesh {

private:
    const std::string input_mesh_file;
    const std::string output_file;
    const std::vector<Tetrahedron> elements;
    const std::map<int, Tetrahedron::Point> coord_map;
    const std::map<int, std::string> media_map;

    std::vector<int> _neighbours;

    bool is_boundary_element(std::size_t elt_index) const {
         return _neighbours[4 * elt_index] == -1
                || _neighbours[4 * elt_index + 1] == -1
                || _neighbours[4 * elt_index + 2] == -1
                || _neighbours[4 * elt_index + 3] == -1;
    }

public:

  TetrahedralMesh() = delete;

  TetrahedralMesh(std::string _input_mesh, const std::vector<Tetrahedron>& _elements,
       const std::map<int, Tetrahedron::Point>& _coord_map,
       const std::map<int, std::string> _media_map, std::string _output_file)
       : input_mesh_file(_input_mesh),
         output_file(_output_file),
         elements(_elements),
         coord_map(_coord_map),
         media_map(_media_map)
  {
       this->_neighbours = tetrahedron_neighbours(this->nodes());
  }

  std::string input_mesh_filename() const {
    return input_mesh_file;
  }

  std::string output_filename() const {
    return output_file;
  }

  std::size_t size() const {
      return elements.size();
  }

  std::map<int, std::string> mesh_media_map() const {
    return media_map;
  }

  std::vector<int> nodes() const {
      std::vector<int> nodes;
      nodes.reserve(4 * this->size());

      for (auto elt: elements) {
          for (int i = 0; i < 4; i++) {
              nodes.push_back(elt.nodes[i]);
          }
      }

      assert(nodes.size() == 4 * elements.size());
      return nodes;
  }

  std::vector<int> boundaries() const {
    std::vector<int> boundaries;
    boundaries.reserve(this->size());

    for (std::size_t i = 0; i < this->size(); i++) {
        boundaries.push_back(this->is_boundary_element(i));
    }

    return boundaries;
  }

  std::vector<int> neighbours() const {
        return this->_neighbours;
  }

  std::vector<int> media() const {
    std::vector<int> media;
    media.reserve(this->size());

    for (auto elt: elements) {
        media.push_back(elt.medium());
    }

    return media;
  }

  std::vector<double> relative_densities() const {
    std::vector<double> relative_densities;
    relative_densities.reserve(this->size());

    for (auto elt: elements) {
        relative_densities.push_back(elt.relative_density());
    }

    return relative_densities;
  }

  std::vector<double> coords(double scale = 1.0) const {
	  std::vector<double> coords;
      // an element has 4 nodes with 3 coordinates each
      coords.reserve(this->size() * 4 * 3);

      for (auto nd: nodes()) {
        auto point = coord_map.at(nd);
        coords.push_back(point.x * scale);
        coords.push_back(point.y * scale);
        coords.push_back(point.z * scale);
      }

      return coords;
  }

  // Get all the element tags. Useful for external programs (e.g. plotting).
  std::vector<int> element_tags() const {
     std::vector<int> tags;
     tags.reserve(this->size());

     for (auto elt: elements) {
        tags.push_back(elt.tag);
     }

     return tags;
  }
};

#endif
