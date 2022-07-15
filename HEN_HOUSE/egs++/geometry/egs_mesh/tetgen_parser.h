/*
###############################################################################
#
#  EGS_Mesh TetGen node and ele file parser
#
#  Copyright (C) 2022 Max Orok
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
#  Author:          Max Orok, 2022
#
#  Contributors:
#
###############################################################################
*/

// exclude from doxygen
/// @cond

#ifndef EGS_MESH_TETGEN_PARSER_
#define EGS_MESH_TETGEN_PARSER_

#include "egs_mesh.h" // for EGS_MeshSpec

#include <algorithm>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace tetgen_parser {

/// Top-level TetGen node and ele file parser.
///
/// Either `file.node` or `file.ele` can be passed in as the filename, the other
/// file will be opened automatically.
///
/// Throws a std::runtime_error if parsing fails.
EGS_MeshSpec parse_tetgen_files(const std::string &filename,
                                EGS_InfoFunction info = nullptr);

enum class TetGenFile { Node, Ele };

/// The tetgen_parser::internal namespace is for internal API functions and is
/// not part of the public API. Functions and types may change without warning.
namespace internal {

/// Trim whitespace from the start of a string.
///
/// From https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

/// Parses a TetGen node file into a list of unique `EGS_Mesh::Node`s.
///
/// ```
/// num_nodes, num_coordinates (=3), num_attributes, num_boundary_markers
/// tag_0 x_0 y_0 z_0 [attr_0...] boundary_marker_0
/// tag_1 x_1 y_1 z_1 [attr_1...] boundary_marker_1
/// ...
/// ```
/// This function expects the node header to be on the first line of the file.
/// Lines starting with # in the body are ignored, along with attributes and
/// boundary marker node data.
///
/// Throws a std::runtime_error if parsing fails.
std::vector<EGS_MeshSpec::Node> parse_tetgen_node_file(std::istream &input,
        EGS_InfoFunction info) {
    std::vector<EGS_MeshSpec::Node> nodes;
    // Parse the header:
    // ```
    // num_nodes, num_coordinates (=3), num_attributes, num_boundary_markers
    // ```
    // Only num_nodes and num_coordinates (which must equal 3 or an exception is
    // thrown) are used.
    int num_nodes = -1;
    std::string line;
    {
        int num_coords = -1;
        int num_attr = -1;
        int num_boundary = -1;
        std::getline(input, line);
        std::istringstream line_stream(line);
        line_stream >> num_nodes >> num_coords >> num_attr >> num_boundary;
        if (line_stream.fail() || num_nodes == -1) {
            throw std::runtime_error("failed to parse TetGen node file header");
        }
        if (num_coords != 3) {
            throw std::runtime_error("TetGen node file parsing failed, expected"
                                     " num_coords = 3");
        }
    }

    if (num_nodes < 50000) {
        info = nullptr; // don't log for small meshes
    }

    egs_mesh::internal::PercentCounter progress(info, "EGS_Mesh: reading " +
            std::to_string(num_nodes) + " nodes");
    progress.start(num_nodes);

    nodes.reserve(num_nodes);

    // Parse node file body
    while (nodes.size() < static_cast<std::size_t>(num_nodes)) {
        std::getline(input, line);
        // Skip lines starting with #
        ltrim(line);
        if (line.rfind('#', 0) == 0) {
            continue;
        }
        std::istringstream line_stream(line);
        int tag = -1;
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
        line_stream >> tag >> x >> y >> z;
        if (line_stream.fail() || tag == -1) {
            throw std::runtime_error("TetGen node file parsing failed");
        }
        nodes.push_back(EGS_MeshSpec::Node(tag, x, y, z));
        progress.step(1);
    }
    progress.finish("EGS_Mesh: read " + std::to_string(num_nodes) + " nodes");
    return nodes;
}

/// Parses a TetGen ele file into a list of `EGS_Mesh::Tetrahedron`s.
///
/// ```
/// num_elts, num_nodes (=4), num_attributes (=1)
/// tag_0 n_00 n_01 n_02 n_03 attr_0
/// tag_1 n_10 n_11 n_12 n_13 attr_1
/// ...
/// ```
/// This function expects the header to be on the first line of the file.
/// Lines starting with # in the body are ignored. Each element is expected to
/// have one attribute, which is used as the EGSnrc media.
///
/// Throws a std::runtime_error if parsing fails.
std::vector<EGS_MeshSpec::Tetrahedron> parse_tetgen_ele_file(
    std::istream &input, EGS_InfoFunction info) {
    std::vector<EGS_MeshSpec::Tetrahedron> elts;
    // Parse the header:
    // ```
    // num_elts, num_nodes (=4), num_attributes (=1)
    // ```
    // num_nodes must be 4 and num_attributes must be 1, or an exception is thrown.
    int num_elts = -1;
    std::string line;
    {
        int num_nodes = -1;
        int num_attr = -1;
        std::getline(input, line);
        std::istringstream line_stream(line);
        line_stream >> num_elts >> num_nodes >> num_attr;
        if (line_stream.fail() || num_elts == -1) {
            throw std::runtime_error("failed to parse TetGen ele file header");
        }
        if (num_nodes != 4) {
            throw std::runtime_error("TetGen ele file parsing failed, expected"
                                     " 4 nodes per tetrahedron");
        }
        if (num_attr != 1) {
            throw std::runtime_error("TetGen ele file parsing failed, expected"
                                     " each element to only have one attribute (EGSnrc medium)");
        }
    }

    if (num_elts < 50000) {
        info = nullptr; // don't log for small meshes
    }

    egs_mesh::internal::PercentCounter progress(info, "EGS_Mesh: reading " +
            std::to_string(num_elts) + " tetrahedrons");
    progress.start(num_elts);

    elts.reserve(num_elts);
    // Parse ele file body
    while (elts.size() < static_cast<std::size_t>(num_elts)) {
        std::getline(input, line);
        // Skip lines starting with #
        ltrim(line);
        if (line.rfind('#', 0) == 0) {
            continue;
        }
        std::istringstream line_stream(line);
        int tag = -1;
        int n0 = -1;
        int n1 = -1;
        int n2 = -1;
        int n3 = -1;
        int media = -1;
        line_stream >> tag >> n0 >> n1 >> n2 >> n3 >> media;
        if (line_stream.fail() || tag == -1) {
            throw std::runtime_error("Tetgen ele file parsing failed");
        }
        elts.push_back(EGS_MeshSpec::Tetrahedron(tag, media, n0, n1, n2, n3));
        progress.step(1);
    }
    progress.finish("EGS_Mesh: read " + std::to_string(num_elts)
                    + " tetrahedrons");

    return elts;
}

// Extract the unique media from the list of all tetrahedrons.
std::vector<EGS_MeshSpec::Medium> find_tetgen_elt_media(
    const std::vector<EGS_MeshSpec::Tetrahedron> &elts) {
    // Find set of unique media tags
    std::set<int> media_tags;
    for (const auto &e: elts) {
        media_tags.insert(e.medium_tag);
    }
    std::vector<EGS_MeshSpec::Medium> media;
    media.reserve(media_tags.size());
    for (const auto &m : media_tags) {
        // TetGen files only store media tag numbers, so use a string version of
        // the media tag for the `medium_name` field instead.
        media.push_back(EGS_MeshSpec::Medium(m, std::to_string(m)));
    }
    return media;
}
} // namespace tetgen_parser::internal

EGS_MeshSpec parse_tetgen_files(const std::string &filename,
                                TetGenFile tetgen_file_kind, EGS_InfoFunction info /*default=nullptr*/) {
    std::string node_file;
    std::string ele_file;
    if (tetgen_file_kind == TetGenFile::Ele) {
        ele_file = filename;
        node_file = filename.substr(0, filename.size() - 4) + ".node";
    }
    else if (tetgen_file_kind == TetGenFile::Node) {
        ele_file = filename.substr(0, filename.size() - 5) + ".ele";
        node_file = filename;
    }
    else {
        throw std::runtime_error("Unhandled TetGen file type");
    }

    std::ifstream node_stream(node_file);
    if (!node_stream) {
        throw std::runtime_error(std::string("Tetgen node file `") + node_file
                                 +  "` does not exist or is not readable");
    }
    std::ifstream ele_stream(ele_file);
    if (!ele_stream) {
        throw std::runtime_error(std::string("Tetgen ele file `") + ele_file
                                 +  "` does not exist or is not readable");
    }

    auto nodes = internal::parse_tetgen_node_file(node_stream, info);
    auto elts = internal::parse_tetgen_ele_file(ele_stream, info);
    auto media = internal::find_tetgen_elt_media(elts);
    return EGS_MeshSpec(std::move(elts), std::move(nodes), std::move(media));
}

} // namespace tetgen_parser

#endif // EGS_MESH_TETGEN_PARSER_

/// @endcond
