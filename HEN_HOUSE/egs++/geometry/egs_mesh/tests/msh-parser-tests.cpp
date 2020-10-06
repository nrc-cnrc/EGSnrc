#include "msh_parser.h"

int test_parse_msh_version() {
    std::cerr << "starting test parse_msh_version" << std::endl;
    // catch empty inputs
    {
        std::istringstream input("");
        std::string err_msg;
        parse_msh_version(input, err_msg);
        std::string expected = "unexpected end of input";
        if (err_msg != expected) {
            std::cerr << "got error message: \""
                << err_msg << "\"\nbut expected: \"" << expected << "\"\n";
            return 1;
        }
    }
    // bad format header
    {
        std::istringstream input("$MshFmt\n");
        std::string err_msg;
        parse_msh_version(input, err_msg);
        std::string expected = "expected $MeshFormat, got $MshFmt";
        if (err_msg != expected) {
            std::cerr << "got error message: \""
                << err_msg << "\"\nbut expected: \"" << expected << "\"\n";
            return 1;
        }
    }
    // windows line-endings
    {
        std::istringstream input("$MeshFormat\r\n");
        std::string err_msg;
        parse_msh_version(input, err_msg);
        std::string expected = "failed to parse msh version";
        if (err_msg != expected) {
            std::cerr << "got error message: \""
                << err_msg << "\"\nbut expected: \"" << expected << "\"\n";
            return 1;
        }
    }
    // bad msh version line
    {
         std::istringstream input(
            "$MeshFormat\n"
            "0\n"
            "$EndMeshFormat\n"
        );
        std::string err_msg;
        parse_msh_version(input, err_msg);
        std::string expected = "failed to parse msh version";
        if (err_msg != expected) {
            std::cerr << "got error message: \""
                << err_msg << "\"\nbut expected: \"" << expected << "\"\n";
            return 1;
        }
    }
    // unknown msh version
    {
        std::istringstream input(
            "$MeshFormat\n"
            "100.2 0 8\n"
            "$EndMeshFormat\n"
        );
        std::string err_msg;
        parse_msh_version(input, err_msg);
        std::string expected = "unhandled msh version `100.2`, the only known version is 2.2";
        if (err_msg != expected) {
            std::cerr << "got error message: \""
                << err_msg << "\"\nbut expected: \"" << expected << "\"\n";
            return 1;
        }
    }
    // binary files are unsupported
    {
        std::istringstream input(
            "$MeshFormat\n"
            "2.2 1 8\n"
            "$EndMeshFormat\n"
        );
        std::string err_msg;
        parse_msh_version(input, err_msg);
        std::string expected = "binary msh files are unsupported, please convert this file to ascii and try again";
        if (err_msg != expected) {
            std::cerr << "got error message: \""
                << err_msg << "\"\nbut expected: \"" << expected << "\"\n";
            return 1;
        }
    }
    // size_t != 8 is unsupported
    {
        std::istringstream input(
            "$MeshFormat\n"
            "2.2 0 4\n"
            "$EndMeshFormat\n"
        );
        std::string err_msg;
        parse_msh_version(input, err_msg);
        std::string expected = "msh file size_t must be 8";
        if (err_msg != expected) {
            std::cerr << "got error message: \""
                << err_msg << "\"\nbut expected: \"" << expected << "\"\n";
            return 1;
        }

    }
    // eof after version line fails
    {
        std::istringstream input(
            "$MeshFormat\n"
            "2.2 0 8\n"
        );
        std::string err_msg;
        auto vers = parse_msh_version(input, err_msg);
        std::string expected = "expected $EndMeshFormat, got ``";
        if (err_msg != expected) {
            std::cerr << "got error message: \""
                << err_msg << "\"\nbut expected: \"" << expected << "\"\n";
            return 1;
        }
    }
    // parse msh v2.2 successfully
    {
        std::istringstream input(
            "$MeshFormat\n"
            "2.2 0 8\n"
            "$EndMeshFormat\n"
        );
        std::string err_msg;
        auto vers = parse_msh_version(input, err_msg);
        if (err_msg != "") {
            std::cerr << "got error message: \"" << err_msg << "\"\n";
            return 1;
        }
        if (vers != MshVersion::v22) {
            std::cerr << "failed to parse mesh version 2.2\n";
            return 1;
        }
    }
    return 0;
}

int main() {
    int err = test_parse_msh_version();
    if (err) {
        std::cerr << "test FAILED" << std::endl;
        return 1;
    }
    std::cerr << "test PASSED" << std::endl;
}
