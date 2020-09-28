/*
###############################################################################
#
#  EGSnrc mevegs application Gmsh interface.
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
#  Functions for interacting with the Gmsh API.
#
###############################################################################
*/

#ifndef GMSH_MANIP
#define GMSH_MANIP

#include <algorithm>
#include <limits>
#include <map>
#include <chrono>
#include <set>

#include "gmsh.h"
#include "TetrahedralMesh.h"

namespace gmsh_manip {

    // Make a new TetrahedralMesh using a Gmsh mesh file.
    TetrahedralMesh mesh_from_gmsh(std::string fileName);

    // inner namespace to hide implemenation details
    namespace {
        // A Gmsh physical group. Materials with the same media may have different
        // physical groups depending on any material modifiers.
        //
        // For example, Water and Water;RHOR=1.2 are distinct physical groups, because
        // of the relative density modifier even though they share the Water medium number.
        class PhysicalGroup;

        // Find this model's physical groups.
        //
        // Assumes an open gmsh context.
        // Returns the physical groups and a map from medium number to medium name.
        std::pair<std::vector<PhysicalGroup>, std::map<int, std::string>>
        find_physical_groups();

        // Find each physical group's elements.
        //
        // Assumes an open gmsh context.
        std::vector<Tetrahedron>
        physical_group_elements(const std::vector<PhysicalGroup>& p_groups);

        // Find the map of node indices to cartesian coordinates.
        //
        // Assumes an open gmsh context. We used a map in an attempt to save space,
        // since each node can be shared by up to 64 tetrahedrons.
        std::map<int, Tetrahedron::Point> gmsh_node_coordinates();
    }

    TetrahedralMesh mesh_from_gmsh(std::string fileName) {

        gmsh::initialize();
        gmsh::open(fileName);

        // check if file actually opened
        std::vector<std::string> models;
        gmsh::model::list(models);
        assert(models.size() == 1);

        if (models[0] == std::string{""}) {
          std::cerr << "gmsh couldn't read mesh file, exiting\n";
          exit(1);
        }

        // Ensure element, node numbering is continuous
        gmsh::model::mesh::renumberNodes();
        gmsh::model::mesh::renumberElements();

        // save a copy to append our simulation results to after the simulation
        std::string output_file = fileName + ".results.msh";
        // overwrites any existing file
        gmsh::write(output_file);

        // find the model's physical groups
        std::vector<PhysicalGroup> physical_groups;
        std::map<int, std::string> media_map;
        std::tie(physical_groups, media_map) = find_physical_groups();

        auto elts = physical_group_elements(physical_groups);

        std::cout << "Total number of mesh elements is: " << elts.size() << "\n";

        auto coord_map = gmsh_node_coordinates();

        gmsh::finalize();

        // for (auto elt: elts) {
        //     std::cout << elt.print() << "\n";
        // }

        std::cout << "MEDIA MAP:" << std::endl;
        for (auto key_str : media_map) {
          std::cout << key_str.first << ": " << key_str.second << std::endl;
        }

        return TetrahedralMesh(fileName, elts, coord_map, media_map, output_file);

    }

    namespace {

    // split a string using a delimiter -- taken from FluentCpp
    // -- source: www.fluentcpp.com/2017/04/21/how-to-split-a-string-in-c/
    std::vector<std::string> string_split(const std::string& s, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
       return tokens;
    }

    // patch converter function after gmsh switched to size_t for tags
    auto checked_tag_convert = [](std::size_t tag) -> int {
        if (tag > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
            throw std::runtime_error("size_t tag too large for int");
        }
        return static_cast<int>(tag);
    };

    class PhysicalGroup {
    public:

        std::string name;
        int gmsh_tag;
        EGSnrcProperties properties;

        explicit PhysicalGroup(int _gmsh_tag, std::string _name, int _medium, std::vector<std::string> extra_properties) {
            gmsh_tag = _gmsh_tag;
            name = _name;
            properties = set_properties(_medium, extra_properties);
        }

        EGSnrcProperties set_properties(int medium, std::vector<std::string> tokens) const {
            auto props = EGSnrcProperties(medium);

            if (tokens.empty()) {
                return props;
            }

            std::cout << "Info:\tLoading additional physical group properties for physical group "
                      << this->gmsh_tag << " (" << this->name << ")" << std::endl;

            auto parse_segment = [](std::string segment) {
                auto key = segment.substr(0, segment.find("="));
                auto val = segment.substr(segment.find("=") + 1, segment.length() - 1);
                return std::pair<std::string, double> (key, std::stod(val));
            };

            // set prop values using remaining tokens
            for (auto segment: tokens) {
                std::string key;
                double val;

                std::tie(key, val) = parse_segment(segment);
                std::cout << "\t\t" << key << ": " << val << std::endl;

                // compare uppercase keys
                for (auto &c : key) { c = toupper(c); }

                if (key.compare(std::string("RHOR")) == 0) {
                    props.relative_rho = val;
                } else {
                    std::cerr << "Error:\tFound unsupported key: " << key << " \n"
                              << "Available keys are:\n"
                              << "RHOR\n"
                              << "Aborting\n";
                    exit(1);
                }
            }

            return props;
        }

        std::string print() const {
            std::stringstream ss;
            ss << "Physical Group { name: " << this->name << ", properties: " << properties.print() << "} \n";
            return ss.str();
        }
    };


    std::pair<std::vector<PhysicalGroup>, std::map<int, std::string>>
    find_physical_groups() {

        // get physical group tags and use them to get the physical group name
        gmsh::vectorpair physGroups; // first entry dim = 3, second is physical group tag
        gmsh::model::getPhysicalGroups(physGroups);
        std::cout << "physical groups length: " << physGroups.size() << std::endl;

        auto delimiter = ';';

        // find the unique set of material names
        std::set<std::string> media_names;
        for (auto pg : physGroups) {
            std::string p_name = "";
            gmsh::model::getPhysicalName(pg.first, pg.second, p_name);
            assert(pg.first == 3); // only want to process physical volumes
            // first element is always the material name
            media_names.insert(string_split(p_name, delimiter).front());
        }

        auto media_number = [&](std::string p_name) -> int {
            return std::distance(media_names.begin(), media_names.find(p_name));
        };

        // get the map from media numbers to media names
        std::map<int, std::string> media_map;
        for (auto name: media_names) {
            media_map[media_number(name)] = name;
        }

        // create physical groups
        std::vector<PhysicalGroup> p_groups;
        for (auto pg : physGroups) {
            std::string p_name = "";
            gmsh::model::getPhysicalName(pg.first, pg.second, p_name);

            auto properties = string_split(p_name, delimiter);
            auto name = properties.front(); // name is always the first property
            properties.erase(properties.begin());
            p_groups.push_back(PhysicalGroup(pg.second, name, media_number(name), properties));
        }

        return std::make_pair(p_groups, media_map);
    }

    std::vector<Tetrahedron>
    physical_group_elements(const std::vector<PhysicalGroup>& p_groups) {
        // return physical group constituent volumes
        auto get_volumes = [](int group_tag) -> std::vector<int> {
            std::vector<int> entities;
            gmsh::model::getEntitiesForPhysicalGroup(3, group_tag, entities);
            return entities;
        };

        // return a volume's tetrahedral mesh elements
        auto get_tetrahedrons = [](int entity_tag) -> std::pair<std::vector<std::size_t>, std::vector<std::size_t>> {
            std::vector<std::vector<std::size_t>> elts, nodes;
            auto tetrahedron = std::vector<int> {4};
            gmsh::model::mesh::getElements(tetrahedron, elts, nodes, 3, entity_tag);
            return std::make_pair(elts.front(), nodes.front());
        };

      std::vector<Tetrahedron> mesh;

      for (const auto &pg : p_groups) {
        // get 3D elements for this physical group
        auto volumes = get_volumes(pg.gmsh_tag);
        // get tetrahedrons for each volume
        for (auto vol: volumes) {
            std::vector<std::size_t> elts;
            std::vector<std::size_t> nodes;
            std::tie(elts, nodes) = get_tetrahedrons(vol);

            std::cout << "got " << elts.size() << " and " << nodes.size() << " nodes\n";

            for (std::size_t i = 0; i < elts.size(); ++i) {
                 mesh.emplace_back(Tetrahedron(
                     elts[i],
                     std::array<std::size_t, 4> {
                         nodes[i * 4], nodes[i * 4 + 1], nodes[i * 4 + 2], nodes[i * 4 + 3]
                     },
                     pg.properties // assign physical group properties to each mesh element
                 ));
            }
        }
      }

      return mesh;
    }

    std::map<int, Tetrahedron::Point>
    gmsh_node_coordinates() {
        std::vector<std::size_t> unique_nodes;
        std::vector<double> coords;
        std::vector<double> paraCoords;

        gmsh::model::mesh::getNodes(unique_nodes, coords, /*unused*/ paraCoords);

        std::map<int, Tetrahedron::Point> coord_map;
        for (std::size_t i = 0; i < unique_nodes.size(); ++i) {
            auto node = checked_tag_convert(unique_nodes[i]);
            coord_map[node] = Tetrahedron::Point(coords[3 * i], coords[3 * i + 1], coords[3 * i + 2]);
        }

        return coord_map;
    }

    } // inner namespace
} // gmsh_manip
#endif
