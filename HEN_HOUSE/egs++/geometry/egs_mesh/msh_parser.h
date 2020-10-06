#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <unordered_set>

// todo namespace private

// trim function from https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

enum class MshVersion { v22, Failure };

MshVersion parse_msh_version(std::istream& input, std::string& err_msg) {
    if (!input) {
        err_msg = "bad input to parse_msh";
        return MshVersion::Failure;
    }
    std::string format_line;
    std::getline(input, format_line);
    if (input.bad()) {
        err_msg = "IO error during reading";
        return MshVersion::Failure;
    }
    if (input.eof()) {
        err_msg = "unexpected end of input";
        return MshVersion::Failure;
    }
    rtrim(format_line);
    if (format_line != "$MeshFormat") {
        err_msg = "expected $MeshFormat, got " + format_line;
        return MshVersion::Failure;
    }

    std::string version;
    int binary_flag = -1;
    int sizet = -1;
    input >> version;
    input >> binary_flag;
    input >> sizet;

    if (input.fail()) {
        err_msg = "failed to parse msh version";
        return MshVersion::Failure;
    }
    if (version != "2.2") {
        err_msg = "unhandled msh version `" + version + "`, the only known version is 2.2";
        return MshVersion::Failure;
    }
    if (binary_flag != 0) {
        if (binary_flag == 1) {
            err_msg = "binary msh files are unsupported, please convert this file to ascii and try again";
            return MshVersion::Failure;
        }
        err_msg = "failed to parse msh version";
        return MshVersion::Failure;
    }
    if (sizet != 8) {
        err_msg = "msh file size_t must be 8";
        return MshVersion::Failure;
    }
    // eat newline
    std::getline(input, format_line);

    std::getline(input, format_line);
    rtrim(format_line);
    if (format_line != "$EndMeshFormat") {
        err_msg = "expected $EndMeshFormat, got `" + format_line + "`";
        return MshVersion::Failure;
    }

    if (version == "4.1") {
        return MshVersion::v41;
    }

    return MshVersion::Failure;
}

// A model volume (e.g. cube, cylinder, complex shape constructed by boolean operations).
struct MeshVolume {
    int tag = -1;
    int group = -1;
};

std::vector<MeshVolume> parse_msh4_entities(std::istream& input, std::string& err_msg) {
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
           || num_0d < 0 || num_1d < 0 || num_2d < 0 || num_3d < 0)
        {
            err_msg = "$Entities parsing failed";
            return std::vector<MeshVolume>{};
        }
        if (num_3d == 0) {
            err_msg = "$Entities parsing failed, no volumes found";
            return std::vector<MeshVolume>{};
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
            err_msg = "$Entities parsing failed, 3d volume parsing failed";
            return std::vector<MeshVolume>{};
        }
        if (num_groups == 0) {
            err_msg = "$Entities parsing failed, volume " + std::to_string(tag) + " was not assigned a physical group";
            return std::vector<MeshVolume>{};
        }
        if (num_groups != 1) {
            err_msg = "$Entities parsing failed, volume " + std::to_string(tag) + " has more than one physical group";
            return std::vector<MeshVolume>{};
        }
        volumes.push_back( MeshVolume { tag, group } );
    }
    if (volumes.size() != num_3d) {
        err_msg = "$Entities parsing failed, expected " + std::to_string(num_3d) + " volumes but got " + std::to_string(volumes.size());
        return std::vector<MeshVolume>{};
    }
    // ensure the volumes have unique tags
    std::unordered_set<int> vol_tags;
    vol_tags.reserve(num_3d);
    for (const auto& v: volumes) {
        auto insert_res = vol_tags.insert(v.tag);
        if (insert_res.second == false) {
            err_msg = "$Entities parsing failed, found duplicate volume tag " + std::to_string(v.tag);
            return std::vector<MeshVolume>{};
        }
    }
    return volumes;
}

struct Node {
    int idx;
    double x;
    double y;
    double z;
};

int get_int_line(std::istream& input, std::string& err_msg) {
    std::string line;
    std::getline(input, line);
    rtrim(line);
    std::istringstream line_stream(line);
    int target;
    line_stream >> target;
    if (line_stream.fail()) {
        err_msg = "integer parsing failed";
        return -1;
    }
    // check for trailing data
    std::string trailing;
    line_stream >> trailing;
    if (!trailing.empty()) {
        err_msg = "unexpected trailing data";
    }
    return target;
}

std::vector<Node> parse_msh2_nodes(std::istream& input, std::string& err_msg) {
    std::vector<Node> nodes;
    int num_nodes = get_int_line(input, err_msg);
    if (!err_msg.empty()) {
        // todo add context to error message:
        // -- failed to parse num_nodes
        return std::vector<Node>{};
    }
    nodes.reserve(num_nodes);

    int node_num = -1;
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    while (input >> node_num >> x >> y >> z) {
        nodes.push_back(Node { node_num, x, y, z });
    }

    if (nodes.size() != num_nodes) {
        err_msg = "expected " + std::to_string(num_nodes) + " nodes, but read "
            + std::to_string(nodes.size());
        return std::vector<Node>{};
    }

    // clear error state to continue parsing and check if we hit $EndNodes
    input.clear();
    std::string end_nodes;
    input >> end_nodes;
    if (input.bad()) {
        err_msg = "IO error during reading";
        return std::vector<Node>{};
    }
    if (input.eof()) {
        err_msg = "expected $EndNodes, got EOF";
        return std::vector<Node>{};
    }
    rtrim(end_nodes);
    if (end_nodes != "$EndNodes") {
        err_msg = "expected $EndNodes, got " + end_nodes;
        return std::vector<Node>{};
    }

    return nodes;
}

// 3D Gmsh physical group
struct PhysicalGroup {
    int tag;
    std::string name;
};

std::vector<PhysicalGroup> parse_msh2_groups(std::istream& input, std::string& err_msg) {
    std::vector<PhysicalGroup> groups;
    // this is the total number of physical groups, not just the number of 3D groups
    int num_groups = get_int_line(input, err_msg);
    if (!err_msg.empty()) {
        // todo add context to error message:
        // -- failed to parse num_groups
        return groups;
    }
    groups.reserve(num_groups);

    int dim = -1;
    int tag = -1;
    std::string line;
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
            err_msg = "unexpected end of file, expected $EndPhysicalNames";
            return std::vector<PhysicalGroup>{};
        }
        if (line_stream.fail()) {
            err_msg = "physical group parsing failed: " + line;
            return std::vector<PhysicalGroup>{};
        }
        // only save 3D physical groups
        if (dim != 3) {
            continue;
        }
        // find quoted group name
        auto name_start = line.find_first_of('"');
        if (name_start == std::string::npos) {
            err_msg = "physical group names must be quoted: " + line;
            return std::vector<PhysicalGroup>{};
        }
        auto name_end = line.find_last_of('"');
        if (name_end == name_start) {
            err_msg = "couldn't find closing quote for physical group: " + line;
            return std::vector<PhysicalGroup>{};
        }
        if (name_end - name_start == 1) {
            err_msg = "empty physical group name: " + line;
            return std::vector<PhysicalGroup>{};
        }
        auto name_len = name_end - name_start - 1; // -1 to exclude closing quote
        groups.push_back(PhysicalGroup { tag, line.substr(name_start + 1, name_len) });
    }
    return groups;
}

// A tetrahedron composed of four nodes
struct Tetrahedron {
    int tag = -1;
    int group = -1;
    int a = -1;
    int b = -1;
    int c = -1;
    int d = -1;
};

std::vector<Tetrahedron> parse_msh2_elements(std::istream& input, std::string& err_msg) {
    std::vector<Tetrahedron> elts;
    // total number of elements, not just tetrahedrons
    int num_elts = get_int_line(input, err_msg);
    if (!err_msg.empty()) {
        // todo add context to error message:
        // -- failed to parse num_elts
        return elts;
    }
    elts.reserve(num_elts);

    std::string line;
    while (input) {
        std::getline(input, line);
        rtrim(line);
        if (line == "$EndElements") {
            break;
        }
        std::istringstream line_stream(line);

        int tag = -1;
        int elt_type = -1;
        int num_following = -1;
        line_stream >> tag;
        line_stream >> elt_type;
        line_stream >> num_following;
        if (line_stream.eof()) {
            err_msg = "unexpected end of file, expected $EndElements";
            return std::vector<Tetrahedron>{};
        }
        if (line_stream.fail()) {
            err_msg = "element parsing failed:\n " + line;
            return std::vector<Tetrahedron>{};
        }
        if (elt_type != 4 /* tetrahedron type */) {
            continue;
        }
        if (num_following == 0) {
            err_msg = "got num_following = 0 which means no physical group. All elements must belong to a physical group";
            return std::vector<Tetrahedron>{};
        }
        // parse physical group, skip the rest of num_following
        int group = -1;
        line_stream >> group;
        int dummy = -1;
        for (int i = 0; i < num_following - 1; ++i) {
            line_stream >> dummy;
        }
        // parse the 4 nodes of this tetrahedron
        int a = -1;
        int b = -1;
        int c = -1;
        int d = -1;
        line_stream >> a >> b >> c >> d;
        if (line_stream.fail()) {
            err_msg = "element parsing failed:\n " + line;
            return std::vector<Tetrahedron>{};
        }
        if (group == 0) {
            err_msg = "got physical group id of 0, all elements must have a nonzero physical group:\n " + line;
            return std::vector<Tetrahedron>{};
        }
        elts.push_back(Tetrahedron { tag, group, a, b, c, d });
    }
    return elts;
}

void parse_msh2_body(std::istream& input, std::string& err_msg) {
    std::vector<Node> nodes;
    std::vector<PhysicalGroup> groups;

    std::string input_line;
    while (std::getline(input, input_line)) {
        rtrim(input_line);
        // stop reading if we hit another mesh file
        if (input_line == "$MeshFormat") {
            break;
        }
        if (input_line == "$Nodes") {
            nodes = parse_msh2_nodes(input, err_msg);
        }
        else if (input_line == "$PhysicalNames") {
            parse_msh2_groups(input, err_msg);
        } /* else if (input_line == "$Elements") {
            parse_msh2_elements(input, err_msg);
        } */
    }

    err_msg = "unimplemented";
}

void parse_msh_file(std::istream& input, std::string& err_msg) {
    auto version = parse_msh_version(input, err_msg);
    // TODO auto mesh_data;
    switch(version) {
        case MshVersion::v22: parse_msh2_body(input, err_msg); break;
        default: break; // TODO couldn't parse msh file
    }
}
