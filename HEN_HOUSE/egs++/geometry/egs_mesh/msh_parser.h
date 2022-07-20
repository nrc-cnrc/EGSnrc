/*
###############################################################################
#
#  EGSnrc Gmsh msh file parser
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
#  Author:          Max Orok, 2020
#
#  Contributors:
#
###############################################################################
*/

// exclude from doxygen
/// @cond

#ifndef MSH_PARSER_
#define MSH_PARSER_

#include "egs_mesh.h" // for EGS_MeshSpec

#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace msh_parser {

// Top-level Gmsh msh file parser.
EGS_MeshSpec parse_msh_file(std::istream &input, EGS_InfoFunction info = nullptr);

/// The msh_parser::internal namespace is for internal API functions and is not
/// part of the public API. Functions and types may change without warning.
namespace internal {

// trim function from https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

enum class MshVersion { v41 };
constexpr std::size_t SIZET_MAX = std::numeric_limits<std::size_t>::max();

/// Parse a msh file header.
///
/// Throws a std::runtime_error if parsing fails.
/// Only version 4.1 ascii is supported, any other version will throw.
static MshVersion parse_msh_version(std::istream &input) {
    if (!input) {
        throw std::runtime_error("bad input to parse_msh_version");
    }
    std::string format_line;
    std::getline(input, format_line);
    if (input.bad()) {
        throw std::runtime_error("IO error during reading");
    }
    if (input.eof()) {
        throw std::runtime_error("unexpected end of input");
    }
    rtrim(format_line);
    if (format_line != "$MeshFormat") {
        throw std::runtime_error("expected $MeshFormat, got " + format_line);
    }

    std::string version;
    int binary_flag = -1;
    int sizet = -1;
    input >> version;
    input >> binary_flag;
    input >> sizet;

    if (input.fail()) {
        throw std::runtime_error("failed to parse msh version");
    }
    if (version != "4.1") {
        throw std::runtime_error("unsupported msh version `" + version + "`, the only supported version is 4.1");
    }
    if (binary_flag != 0) {
        if (binary_flag == 1) {
            throw std::runtime_error("binary msh files are unsupported, please convert this file to ascii and try again");
        }
        throw std::runtime_error("failed to parse msh version");
    }
    if (sizet != 8) {
        throw std::runtime_error("msh file size_t must be 8");
    }
    // eat newline
    std::getline(input, format_line);

    std::getline(input, format_line);
    rtrim(format_line);
    if (format_line != "$EndMeshFormat") {
        throw std::runtime_error("expected $EndMeshFormat, got `" + format_line + "`");
    }

    return MshVersion::v41;
}

/// Types and functions specific to parsing msh4.1 files. This namespace is part
/// of the private API and may change without warning.
namespace msh41 {

// A model volume (e.g. cube, cylinder, complex shape constructed by boolean operations).
struct MeshVolume {
    MeshVolume(int tag, int group) : tag(tag), group(group) {}
    int tag = -1;
    int group = -1;
};

// A point in 3D space
struct Node {
    Node(int tag, double x, double y, double z) : tag(tag), x(x), y(y), z(z) {}
    int tag = -1; // TODO size_t?
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

// A tetrahedron composed of four nodes
struct Tetrahedron {
    Tetrahedron(int tag, int volume, int a, int b, int c, int d) :
        tag(tag), volume(volume), a(a), b(b), c(c), d(d) {}
    int tag = -1;
    int volume = -1;
    int a = -1;
    int b = -1;
    int c = -1;
    int d = -1;
};

// 3D Gmsh physical group
struct PhysicalGroup {
    PhysicalGroup(int tag, std::string name) : tag(tag), name(name) {}
    int tag = -1;
    std::string name;
};

// Checks whether a list of structs with "tag" members have unique tags.
// Returns (false, <duplicate_tag>) if a duplicate was found, and returns
// (true, 0) otherwise.
template <typename T>
std::pair<bool, int> check_unique_tags(const std::vector<T> &values) {
    std::unordered_set<int> tags;
    tags.reserve(values.size());
    for (const auto &v: values) {
        auto insert_res = tags.insert(v.tag);
        if (insert_res.second == false) {
            return std::make_pair(false, v.tag);
        }
    }
    return std::make_pair(true, 0);
}

/// Returns a list of volumes. Volume tags are unique.
///
/// Throws a std::runtime_error if parsing fails.
static std::vector<MeshVolume> parse_entities(std::istream &input) {
    std::vector<MeshVolume> volumes;
    int num_3d = -1;
    // parse number of entities
    {
        std::string line;
        std::getline(input, line);
        std::istringstream line_stream(line);
        int num_0d = -1;
        int num_1d = -1;
        int num_2d = -1;
        line_stream >> num_0d >> num_1d >> num_2d >> num_3d;
        if (input.fail()
                || num_0d < 0 || num_1d < 0 || num_2d < 0 || num_3d < 0) {
            throw std::runtime_error("$Entities parsing failed");
        }
        if (num_3d == 0) {
            throw std::runtime_error("$Entities parsing failed, no volumes found");
        }
        // skip to 3d entities
        for (int i = 0; i < (num_0d + num_1d + num_2d); ++i) {
            std::getline(input, line);
        }
    }

    // parse 3d entities
    volumes.reserve(num_3d);
    std::string line;
    while (std::getline(input, line)) {
        rtrim(line);
        if (line == "$EndEntities") {
            break;
        }
        std::istringstream line_stream(line);
        int tag = -1;
        // unused
        double min_x = 0.0;
        double min_y = 0.0;
        double min_z = 0.0;
        double max_x = 0.0;
        double max_y = 0.0;
        double max_z = 0.0;
        // ...unused
        std::size_t num_groups = 0;
        int group = -1;
        line_stream >> tag >>
                    min_x >> min_y >> min_z >>
                    max_x >> max_y >> max_z >>
                    num_groups >> group;
        if (line_stream.fail()) {
            throw std::runtime_error("$Entities parsing failed, 3d volume parsing failed");
        }
        if (num_groups == 0) {
            throw std::runtime_error("$Entities parsing failed, volume " + std::to_string(tag) + " was not assigned a physical group");
        }
        if (num_groups != 1) {
            throw std::runtime_error("$Entities parsing failed, volume " + std::to_string(tag) + " has more than one physical group");
        }
        volumes.push_back(MeshVolume(tag, group));
    }
    if (volumes.size() != static_cast<std::size_t>(num_3d)) {
        throw std::runtime_error("$Entities parsing failed, expected " + std::to_string(num_3d) + " volumes but got " + std::to_string(volumes.size()));
    }
    // ensure volume tags are unique
    auto unique_res = check_unique_tags(volumes);
    if (!unique_res.first) {
        throw std::runtime_error("$Entities section parsing failed, found duplicate volume tag "
                                 + std::to_string(unique_res.second));
    }
    return volumes;
}

/// Parse a single entity bloc of nodes.
///
/// Throws a std::runtime_error if parsing fails.
static std::vector<Node> parse_node_bloc(std::istream &input) {
    std::vector<Node> nodes;
    std::size_t num_nodes = SIZET_MAX;
    int entity = -1;
    std::string line;
    {
        std::getline(input, line);
        std::istringstream line_stream(line);
        int dim = -1;
        int parametric = -1;
        line_stream >> dim >> entity >> parametric >> num_nodes;
        if (line_stream.fail() || dim == -1 || entity == -1 || parametric == -1
                || num_nodes == SIZET_MAX) {
            throw std::runtime_error("Node bloc parsing failed");
        }
        if (dim < 0 || dim > 3) {
            throw std::runtime_error("Node bloc parsing failed for entity " + std::to_string(entity) + ", got dimension " + std::to_string(dim) + ", expected 0, 1, 2, or 3");
        }
    }
    nodes.reserve(num_nodes);
    // initialize node tags
    for (std::size_t i = 0; i < num_nodes; ++i) {
        std::getline(input, line);
        std::istringstream line_stream(line);
        std::size_t tag = SIZET_MAX;
        line_stream >> tag;
        if (line_stream.fail() || tag == SIZET_MAX) {
            throw std::runtime_error("Node bloc parsing failed during node tag section of entity " + std::to_string(entity));
        }
        Node n(-1, 0.0, 0.0, 0.0);
        n.tag = tag;
        nodes.push_back(n);
    }
    // fill in coordinates
    for (std::size_t i = 0; i < num_nodes; ++i) {
        std::getline(input, line);
        std::istringstream line_stream(line);
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
        line_stream >> x >> y >> z;
        if (line_stream.fail()) {
            throw std::runtime_error("Node bloc parsing failed during node coordinate section of entity " + std::to_string(entity));
        }
        nodes.at(i).x = x;
        nodes.at(i).y = y;
        nodes.at(i).z = z;
    }
    if (nodes.size() != num_nodes) {
        throw std::runtime_error("Node bloc parsing failed, expected " + std::to_string(num_nodes) + " nodes but read "
                                 + std::to_string(nodes.size()) + " for entity " + std::to_string(entity));
    }
    return nodes;
}

/// Parse the entire $Nodes section and returns a list of Nodes. Node tags are unique.
///
/// Throws a std::runtime_error if parsing fails.
static std::vector<Node> parse_nodes(std::istream &input,
                                     EGS_InfoFunction info) {
    std::vector<Node> nodes;
    std::size_t num_blocs = SIZET_MAX;
    std::size_t num_nodes = SIZET_MAX;
    std::string line;
    {
        std::getline(input, line);
        std::istringstream line_stream(line);
        std::size_t min_tag = SIZET_MAX;
        std::size_t max_tag = SIZET_MAX;
        line_stream >> num_blocs >> num_nodes >> min_tag >> max_tag;
        if (line_stream.fail() || num_blocs == SIZET_MAX || num_nodes == SIZET_MAX ||
                min_tag == SIZET_MAX || max_tag == SIZET_MAX) {
            throw std::runtime_error("$Nodes section parsing failed, missing metadata");
        }
        if (max_tag > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
            throw std::runtime_error("Max node tag is too large (" + std::to_string(max_tag) + "), limit is "
                                     + std::to_string(std::numeric_limits<int>::max()));
        }
    }

    if (num_nodes < 50000) {
        info = nullptr; // don't log for small meshes
    }

    egs_mesh::internal::PercentCounter progress(info, "EGS_Mesh: reading " +
            std::to_string(num_nodes) + " nodes");
    progress.start(num_nodes);

    nodes.reserve(num_nodes);
    for (std::size_t i = 0; i < num_blocs; ++i) {
        std::vector<Node> bloc_nodes;
        try {
            bloc_nodes = parse_node_bloc(input);
        }
        catch (const std::runtime_error &err) {
            throw std::runtime_error("$Nodes section parsing failed\n" + std::string(err.what()));
        }
        nodes.insert(nodes.end(), bloc_nodes.begin(), bloc_nodes.end());
        progress.step(bloc_nodes.size());
    }
    if (nodes.size() != num_nodes) {
        throw std::runtime_error("$Nodes section parsing failed, expected " + std::to_string(num_nodes) + " nodes but read "
                                 + std::to_string(nodes.size()));
    }
    std::getline(input, line);
    rtrim(line);
    if (line != "$EndNodes") {
        throw std::runtime_error("$Nodes section parsing failed, expected $EndNodes");
    }
    // ensure node tags are unique
    auto unique_res = check_unique_tags(nodes);
    if (!unique_res.first) {
        throw std::runtime_error("$Nodes section parsing failed, found duplicate node tag "
                                 + std::to_string(unique_res.second));
    }

    progress.finish("EGS_Mesh: read " + std::to_string(num_nodes) + " nodes");
    return nodes;
}

/// Returns a list of PhysicalGroups. PhysicalGroup tags are unique.
///
/// Throws a std::runtime_error if parsing fails.
static std::vector<PhysicalGroup> parse_groups(std::istream &input) {
    std::vector<PhysicalGroup> groups;
    // this is the total number of groups, not just 3D groups
    int num_groups = -1;
    std::string line;
    {
        std::getline(input, line);
        std::istringstream line_stream(line);
        line_stream >> num_groups;
        if (line_stream.fail() || num_groups == -1) {
            throw std::runtime_error("$PhysicalNames parsing failed");
        }
    }
    groups.reserve(num_groups);

    int dim = -1;
    int tag = -1;
    while (input) {
        std::getline(input, line);
        rtrim(line);
        if (line == "$EndPhysicalNames") {
            break;
        }
        std::istringstream line_stream(line);
        line_stream >> dim;
        line_stream >> tag;
        if (line_stream.eof()) {
            throw std::runtime_error("unexpected end of file, expected $EndPhysicalNames");
        }
        if (line_stream.fail()) {
            throw std::runtime_error("physical group parsing failed: " + line);
        }
        // only save 3D physical groups
        if (dim != 3) {
            continue;
        }
        // find quoted group name
        auto name_start = line.find_first_of('"');
        if (name_start == std::string::npos) {
            throw std::runtime_error("physical group names must be quoted: " + line);
        }
        auto name_end = line.find_last_of('"');
        if (name_end == name_start) {
            throw std::runtime_error("couldn't find closing quote for physical group: " + line);
        }
        if (name_end - name_start == 1) {
            throw std::runtime_error("empty physical group name: " + line);
        }
        auto name_len = name_end - name_start - 1; // -1 to exclude closing quote
        groups.push_back(PhysicalGroup(tag, line.substr(name_start + 1, name_len)));
    }
    // ensure group tags are unique
    auto unique_res = check_unique_tags(groups);
    if (!unique_res.first) {
        throw std::runtime_error("$PhysicalNames section parsing failed, found duplicate tag "
                                 + std::to_string(unique_res.second));
    }
    return groups;
}

/// Parse a single msh4 element bloc.
///
/// Throws a std::runtime_error if parsing fails.
static std::vector<Tetrahedron> parse_element_bloc(std::istream &input) {
    std::vector<Tetrahedron> elts;
    std::size_t num_elts = SIZET_MAX;
    int entity = -1;
    std::string line;
    {
        std::getline(input, line);
        std::istringstream line_stream(line);
        int dim = -1;
        int element_type = -1;
        line_stream >> dim >> entity >> element_type >> num_elts;
        if (line_stream.fail() || dim == -1 || entity == -1 || element_type == -1
                || num_elts == SIZET_MAX) {
            throw std::runtime_error("Element bloc parsing failed");
        }
        if (dim < 0 || dim > 3) {
            throw std::runtime_error("Element bloc parsing failed for entity " + std::to_string(entity)
                                     + ", got dimension " + std::to_string(dim) + ", expected 0, 1, 2, or 3");
        }
        // skip 0, 1, 2d element blocs
        if (dim != 3) {
            for (std::size_t i = 0; i < num_elts; ++i) {
                std::getline(input, line);
            }
            return std::vector<Tetrahedron> {};
        }
        // If a mesh with 3d non-tetrahedral elements is provided, exit.
        // The mesh may have some volumes that are supposed to be simulated but
        // not represented by tetrahedrons, so they will be missing from the
        // EGSnrc representation of the mesh.
        const int TETRAHEDRON_TYPE = 4;
        if (element_type != TETRAHEDRON_TYPE) {
            throw std::runtime_error("Element bloc parsing failed for entity " + std::to_string(entity) +
                                     ", got non-tetrahedral mesh element type " + std::to_string(element_type));
        }
    }
    elts.reserve(num_elts);

    for (std::size_t i = 0; i < num_elts; ++i) {
        std::getline(input, line);
        std::istringstream line_stream(line);
        int tag = -1;
        int a = -1;
        int b = -1;
        int c = -1;
        int d = -1;
        line_stream >> tag >> a >> b >> c >> d;
        if (line_stream.fail() || tag == -1 || a == -1 || b == -1 ||
                c == -1 || d == -1) {
            throw std::runtime_error("Element bloc parsing failed for entity " + std::to_string(entity));
        }
        elts.push_back(Tetrahedron(tag, entity, a, b, c, d));
    }
    return elts;
}

/// Returns a list of tetrahedral elements. Element tags are unique.
///
/// Throws a std::runtime_error if parsing fails.
static std::vector<Tetrahedron> parse_elements(std::istream &input,
        EGS_InfoFunction info) {
    std::vector<Tetrahedron> elts;
    std::size_t num_blocs = SIZET_MAX;
    std::size_t num_elts = SIZET_MAX;
    std::string line;
    {
        std::getline(input, line);
        std::istringstream line_stream(line);
        std::size_t min_tag = SIZET_MAX;
        std::size_t max_tag = SIZET_MAX;
        line_stream >> num_blocs >> num_elts >> min_tag >> max_tag;
        if (line_stream.fail() || num_blocs == SIZET_MAX || num_elts == SIZET_MAX ||
                min_tag == SIZET_MAX || max_tag == SIZET_MAX) {
            throw std::runtime_error("$Elements section parsing failed, missing metadata");
        }
    }

    if (num_elts < 50000) {
        info = nullptr; // don't log for small meshes
    }

    egs_mesh::internal::PercentCounter progress(info, "EGS_Mesh: reading " +
            std::to_string(num_elts) + " tetrahedrons");
    progress.start(num_elts);

    elts.reserve(num_elts);
    for (std::size_t i = 0; i < num_blocs; ++i) {
        std::vector<Tetrahedron> bloc_elts;
        try {
            bloc_elts = parse_element_bloc(input);
        }
        catch (const std::runtime_error &err) {
            throw std::runtime_error("$Elements section parsing failed\n" + std::string(err.what()));
        }
        elts.insert(elts.end(), bloc_elts.begin(), bloc_elts.end());
        progress.step(bloc_elts.size());
    }
    // can't check against num_elts because it counts all elements
    std::getline(input, line);
    rtrim(line);
    if (line != "$EndElements") {
        throw std::runtime_error("$Elements section parsing failed, expected $EndElements");
    }
    if (elts.size() == 0) {
        throw std::runtime_error("$Elements section parsing failed, no tetrahedral elements were read");
    }
    // ensure element tags are unique
    auto unique_res = check_unique_tags(elts);
    if (!unique_res.first) {
        throw std::runtime_error("$Elements section parsing failed, found duplicate tetrahedron tag "
                                 + std::to_string(unique_res.second));
    }
    progress.finish("EGS_Mesh: read " + std::to_string(num_elts)
                    + " tetrahedrons");
    // TODO check against min and max tag values
    return elts;
}

/// Parse the body of a msh4.1 file into an EGS_MeshSpec.
///
/// Throws a std::runtime_error if parsing fails.
EGS_MeshSpec parse_msh41_body(std::istream &input, EGS_InfoFunction info) {
    std::vector<msh_parser::internal::msh41::Node> nodes;
    std::vector<msh_parser::internal::msh41::MeshVolume> volumes;
    std::vector<msh_parser::internal::msh41::PhysicalGroup> groups;
    std::vector<msh_parser::internal::msh41::Tetrahedron> elements;

    std::string parse_err;
    std::string input_line;
    while (std::getline(input, input_line)) {
        msh_parser::internal::rtrim(input_line);
        // stop reading if we hit another mesh file
        if (input_line == "$MeshFormat") {
            break;
        }
        if (input_line == "$Entities") {
            volumes = msh_parser::internal::msh41::parse_entities(input);
        }
        else if (input_line == "$PhysicalNames") {
            groups = msh_parser::internal::msh41::parse_groups(input);
        }
        else if (input_line == "$Nodes") {
            nodes = msh_parser::internal::msh41::parse_nodes(input, info);
        }
        else if (input_line == "$Elements") {
            elements = msh_parser::internal::msh41::parse_elements(input, info);
        }
    }
    if (volumes.empty()) {
        throw std::runtime_error("No volumes were parsed from $Entities section");
    }
    if (nodes.empty()) {
        throw std::runtime_error("No nodes were parsed, missing $Nodes section");
    }
    if (groups.empty()) {
        throw std::runtime_error("No groups were parsed from $PhysicalNames section");
    }
    if (elements.empty()) {
        throw std::runtime_error("No tetrahedrons were parsed from $Elements section");
    }

    // ensure each entity has a valid group
    std::unordered_set<int> group_tags;
    group_tags.reserve(groups.size());
    for (auto g: groups) {
        group_tags.insert(g.tag);
    }
    std::unordered_map<int, int> volume_groups;
    volume_groups.reserve(volumes.size());
    for (auto v: volumes) {
        if (group_tags.find(v.group) == group_tags.end()) {
            throw std::runtime_error("volume " + std::to_string(v.tag) + " had unknown physical group tag " + std::to_string(v.group));
        }
        volume_groups.insert({ v.tag, v.group });
    }

    // ensure each element has a valid entity and therefore a valid physical group
    std::vector<int> element_groups;
    element_groups.reserve(elements.size());
    for (auto e: elements) {
        auto elt_group = volume_groups.find(e.volume);
        if (elt_group == volume_groups.end()) {
            throw std::runtime_error("tetrahedron " + std::to_string(e.tag) + " had unknown volume tag " + std::to_string(e.volume));
        }
        element_groups.push_back(elt_group->second);
    }

    std::vector<EGS_MeshSpec::Tetrahedron> mesh_elts;
    mesh_elts.reserve(elements.size());
    for (std::size_t i = 0; i < elements.size(); ++i) {
        const auto &elt = elements[i];
        mesh_elts.push_back(EGS_MeshSpec::Tetrahedron(
                                elt.tag, element_groups[i], elt.a, elt.b, elt.c, elt.d
                            ));
    }

    std::vector<EGS_MeshSpec::Node> mesh_nodes;
    mesh_nodes.reserve(nodes.size());
    for (const auto &n: nodes) {
        mesh_nodes.push_back(EGS_MeshSpec::Node(
                                 n.tag, n.x, n.y, n.z
                             ));
    }

    std::vector<EGS_MeshSpec::Medium> media;
    media.reserve(groups.size());
    for (const auto &g: groups) {
        media.push_back(EGS_MeshSpec::Medium(g.tag, g.name));
    }

    // TODO: check all 3d physical groups were used by elements
    // TODO: ensure all element node tags are valid
    return EGS_MeshSpec(std::move(mesh_elts), std::move(mesh_nodes),
                        std::move(media));
}

} // namespace msh_parser::internal::msh41
} // namespace msh_parser::internal

EGS_MeshSpec parse_msh_file(std::istream &input, EGS_InfoFunction info /*default=nullptr*/) {
    auto version = msh_parser::internal::parse_msh_version(input);
    switch (version) {
    case msh_parser::internal::MshVersion::v41:
        try {
            return msh_parser::internal::msh41::parse_msh41_body(
                       input, info);
        }
        catch (const std::runtime_error &err) {
            throw std::runtime_error("msh 4.1 parsing failed\n" + std::string(err.what()));
        }
        break;
    }
    throw std::runtime_error("couldn't parse msh file");
}

} // namespace msh_parser

#endif // MSH_PARSER_

/// @endcond
