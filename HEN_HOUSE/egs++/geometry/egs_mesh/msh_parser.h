#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

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
        err_msg = "IO error during reading\n";
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
