#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

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

    if (version == "2.2") {
        return MshVersion::v22;
    }

    return MshVersion::Failure;
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
        return nodes;
    }

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
        return nodes;
    }

    // clear error state to continue parsing and check if we hit $EndNodes
    input.clear();
    std::string end_nodes;
    input >> end_nodes;
    if (input.bad()) {
        err_msg = "IO error during reading";
        return nodes;
    }
    if (input.eof()) {
        err_msg = "expected $EndNodes, got EOF";
        return nodes;
    }
    rtrim(end_nodes);
    if (end_nodes != "$EndNodes") {
        err_msg = "expected $EndNodes, got " + end_nodes;
        return nodes;
    }

    return nodes;
}

void parse_msh2_body(std::istream& input, std::string& err_msg) {
    std::vector<Node> nodes;

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
        /* else if (input_line == "$PhysicalNames") {
            parse_msh2_physical_groups(input, err_msg);
        } else if (input_line == "$Elements") {
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
