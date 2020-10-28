#include "msh_parser.h"
#include <cassert>

int test_parse_msh_version() {
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
        std::string expected = "unsupported msh version `100.2`, the only supported version is 4.1";
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
            "4.1 1 8\n"
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
            "4.1 0 4\n"
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
            "4.1 0 8\n"
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
    // parse msh v4.1 successfully
    {
        std::istringstream input(
            "$MeshFormat\n"
            "4.1 0 8\n"
            "$EndMeshFormat\n"
        );
        std::string err_msg;
        auto vers = parse_msh_version(input, err_msg);
        if (err_msg != "") {
            std::cerr << "got error message: \"" << err_msg << "\"\n";
            return 1;
        }
        if (vers != MshVersion::v41) {
            std::cerr << "failed to parse mesh version 4.1\n";
            return 1;
        }
    }
    return 0;
}

// all test cases assume $Nodes header has already been parsed
int test_parse_msh2_nodes() {
    // empty section
    {
        std::istringstream input(
            "0\n"
            "$EndNodes\n"
        );
        std::string err_msg;
        auto nodes = parse_msh2_nodes(input, err_msg);
        if (!err_msg.empty()) {
            std::cerr << "got error message: \"" << err_msg << "\"\n";
            return 1;
        }
        if (nodes.size() != 0) {
            std::cerr << "expected 0 nodes, got " << nodes.size() << "\n";
            return 1;
        }
    }
    // missing number of nodes fails
    {
        std::istringstream input(
            "1 0 0 1\n"
            "2 0 0 0\n"
            "3 0 1 1\n"
            "$EndNodes\n"
        );
        std::string err_msg;
        auto nodes = parse_msh2_nodes(input, err_msg);
        std::string expected = "unexpected trailing data";
        if (err_msg != expected) {
            std::cerr << "got error message: \""
                << err_msg << "\"\nbut expected: \"" << expected << "\"\n";
            return 1;
        }
    }
    // num_nodes vs actual number of nodes mismatch
    {
        std::istringstream input(
            "4\n"
            "1 0 0 1\n"
            "2 0 0 0\n"
            "3 0 1 1\n"
        );
        std::string err_msg;
        auto nodes = parse_msh2_nodes(input, err_msg);
        std::string expected = "expected 4 nodes, but read 3";
        if (err_msg != expected) {
            std::cerr << "got error message: \""
                << err_msg << "\"\nbut expected: \"" << expected << "\"\n";
            return 1;
        }
    }
    // catch missing $EndNodes
    {
        std::istringstream input(
            "4\n"
            "1 0 0 1\n"
            "2 0 0 0\n"
            "3 0 1 1\n"
            "4 0 1 0\n"
        );
        std::string err_msg;
        auto nodes = parse_msh2_nodes(input, err_msg);
        std::string expected = "expected $EndNodes, got EOF";
        if (err_msg != expected) {
            std::cerr << "got error message: \""
                << err_msg << "\"\nbut expected: \"" << expected << "\"\n";
            return 1;
        }
    }
    // bad section end tag
    {
        std::istringstream input(
            "4\n"
            "1 0 0 1\n"
            "2 0 0 0\n"
            "3 0 1 1\n"
            "4 0 1 0\n"
            "$End\n"
        );
        std::string err_msg;
        auto nodes = parse_msh2_nodes(input, err_msg);
        std::string expected = "expected $EndNodes, got $End";
        if (err_msg != expected) {
            std::cerr << "got error message: \""
                << err_msg << "\"\nbut expected: \"" << expected << "\"\n";
            return 1;
        }
    }

    // successfully parse a nodes section
    {
        std::istringstream input(
            "4\n"
            "1 0 0 1\n"
            "2 0 0 0\n"
            "3 0 1 1\n"
            "4 0 1 0\n"
            "$EndNodes\n"
        );
        std::string err_msg;
        auto nodes = parse_msh2_nodes(input, err_msg);
        if (!err_msg.empty()) {
                std::cerr << "got error message: \"" << err_msg << "\"\n";
                return 1;
        }
        if (nodes.size() != 4) {
            std::cerr << "expected 4 nodes, got " << nodes.size() << "\n";
            return 1;
        }
        if (nodes[3].tag != 4 ||
            nodes[3].x != 0.0 ||
            nodes[3].y != 1.0 ||
            nodes[3].z != 0.0)
        {
            std::cerr << "parsed node didn't match reference value\n";
            return 1;
        }
    }
    return 0;
}

// all test cases assume $PhysicalNames header has already been parsed
int test_parse_msh4_groups() {
    // empty section
    {
        std::istringstream input(
            "0\n"
            "$EndPhysicalNames\n"
        );
        std::string err_msg;
        auto groups = parse_msh4_groups(input, err_msg);
        if (!err_msg.empty()) {
            std::cerr << "got error message: \"" << err_msg << "\"\n";
            return 1;
        }
        if (groups.size() != 0) {
            std::cerr << "expected 0 nodes, got " << groups.size() << "\n";
            return 1;
        }
    }
    // missing number of groups fails
    {
        std::istringstream input(
            "1 1 \"a line\"\n"
            "2 2 \"a surface\"\n"
            "3 3 \"a volume\"\n"
            "$EndPhysicalNames\n"
        );
        std::string err_msg;
        auto groups = parse_msh4_groups(input, err_msg);
        std::string expected = "unexpected trailing data";
        if (err_msg != expected) {
            std::cerr << "got error message: \""
                << err_msg << "\"\nbut expected: \"" << expected << "\"\n";
            return 1;
        }
    }
    // missing $EndPhysicalNames tag fails
    {
        std::istringstream input(
            "3\n"
            "1 1 \"a line\"\n"
            "2 2 \"a surface\"\n"
            "3 3 \"a volume\"\n"
        );
        std::string err_msg;
        auto groups = parse_msh4_groups(input, err_msg);
        std::string expected = "unexpected end of file, expected $EndPhysicalNames";
        if (err_msg != expected) {
            std::cerr << "got error message: \""
                << err_msg << "\"\nbut expected: \"" << expected << "\"\n";
            return 1;
        }
    }
    // bad physical group line fails
    {
        std::istringstream input(
            "1\n"
            "1 \"a line\"\n" // missing tag
            "$EndPhysicalNames\n"
        );
        std::string err_msg;
        auto groups = parse_msh4_groups(input, err_msg);
        std::string expected = "physical group parsing failed: 1 \"a line\"";
        if (err_msg != expected) {
            std::cerr << "got error message: \""
                << err_msg << "\"\nbut expected: \"" << expected << "\"\n";
            return 1;
        }
    }
    // catch invalid physical group names
    {
        std::istringstream input(
            "1\n"
            "3 1 \"\"\n"
            "$EndPhysicalNames\n"
        );
        std::string err_msg;
        auto groups = parse_msh4_groups(input, err_msg);
        std::string expected = "empty physical group name: 3 1 \"\"";
        if (err_msg != expected) {
            std::cerr << "got error message: \""
                << err_msg << "\"\nbut expected: \"" << expected << "\"\n";
            return 1;
        }
    }
    // physical group names are quoted
    {
        std::istringstream input(
            "1\n"
            "3 1 Steel\n"
            "$EndPhysicalNames\n"
        );
        std::string err_msg;
        auto groups = parse_msh4_groups(input, err_msg);
        std::string expected = "physical group names must be quoted: 3 1 Steel";
        if (err_msg != expected) {
            std::cerr << "got error message: \""
                << err_msg << "\"\nbut expected: \"" << expected << "\"\n";
            return 1;
        }
    }
    // closing name quote is required
    {
        std::istringstream input(
            "1\n"
            "3 1 \"Steel\n"
            "$EndPhysicalNames\n"
        );
        std::string err_msg;
        auto groups = parse_msh4_groups(input, err_msg);
        std::string expected = "couldn't find closing quote for physical group: 3 1 \"Steel";
        if (err_msg != expected) {
            std::cerr << "got error message: \""
                << err_msg << "\"\nbut expected: \"" << expected << "\"\n";
            return 1;
        }
    }
    // only 3D groups are returned
    {
         std::istringstream input(
            "3\n"
            "1 1 \"line\"\n"
            "2 2 \"surface\"\n"
            "3 3 \"volume\"\n"
            "$EndPhysicalNames\n"
        );
        std::string err_msg;
        auto groups = parse_msh4_groups(input, err_msg);
        if (!err_msg.empty()) {
            std::cerr << "got error message: \"" << err_msg << "\"\n";
            return 1;
        }
        std::string expected_name = "volume";
        if (groups.at(0).name != expected_name) {
            std::cerr << "bad physical name parse, expected: " << expected_name
                << "but got: " << groups.at(0).name << "\n";
            return 1;
        }
    }
    // spaces in names are OK
    {
         std::istringstream input(
            "3\n"
            "1 1 \"a line\"\n"
            "2 2 \"a surface\"\n"
            "3 3 \"a volume\"\n"
            "$EndPhysicalNames\n"
        );
        std::string err_msg;
        auto groups = parse_msh4_groups(input, err_msg);
        if (!err_msg.empty()) {
            std::cerr << "got error message: \"" << err_msg << "\"\n";
            return 1;
        }
        std::string expected_name = "a volume";
        if (groups.at(0).name != expected_name) {
            std::cerr << "bad physical name parse, expected: " << expected_name
                << "but got: " << groups.at(0).name << "\n";
            return 1;
        }
    }
    // single letter names are OK
    {
         std::istringstream input(
            "3\n"
            "1 1 \"a line\"\n"
            "2 2 \"a surface\"\n"
            "3 3 \"a\"\n"
            "$EndPhysicalNames\n"
        );
        std::string err_msg;
        auto groups = parse_msh4_groups(input, err_msg);
        if (!err_msg.empty()) {
            std::cerr << "got error message: \"" << err_msg << "\"\n";
            return 1;
        }
        std::string expected_name = "a";
        if (groups.at(0).name != expected_name) {
            std::cerr << "bad physical name parse, expected: " << expected_name
                << "but got: " << groups.at(0).name << "\n";
            return 1;
        }
    }
    // successfully parse a valid physical groups section
    {
         std::istringstream input(
            "5\n"
            "1 1 \"a line\"\n"
            "2 2 \"a surface\"\n"
            "3 3 \"Steel\"\n"
            "3 4 \"Air\"\n"
            "3 5 \"Water\"\n"
            "$EndPhysicalNames\n"
        );
        std::string err_msg;
        auto groups = parse_msh4_groups(input, err_msg);
        if (!err_msg.empty()) {
            std::cerr << "got error message: \"" << err_msg << "\"\n";
            return 1;
        }
        if (groups.size() != 3) {
            std::cerr << "expected 3 groups, got " << groups.size() << "\n";
            return 1;
        }
        if (! (groups.at(0).name == "Steel" && groups.at(0).tag == 3) &&
              (groups.at(1).name == "Air" && groups.at(1).tag == 4) &&
              (groups.at(2).name == "Water" && groups.at(2).tag == 5))
        {
            std::cerr << "parsed physical groups didn't match reference values\n";
            return 1;
        }
    }
    return 0;
}

int test_parse_msh4_nodes() {
    // missing bloc metadata fails
    {
        std::istringstream input(
        //    "1 100 0 1\n"
            "1\n"
            "1 0 0\n"
        );
        std::string err_msg;
        auto nodes = parse_msh4_node_bloc(input, err_msg);
        assert(nodes.size() == 0);
        std::string expected = "Node bloc parsing failed";
        if (err_msg != expected) {
            std::cerr << "got error message: \""
                << err_msg << "\"\nbut expected: \"" << expected << "\"\n";
            return 1;
        }
    }
    // bad dimension value fails
    {
        std::istringstream input(
            "4 100 0 1\n" // 4d entity?
            "1\n"
            "1 0 0\n"
        );
        std::string err_msg;
        auto nodes = parse_msh4_node_bloc(input, err_msg);
        assert(nodes.size() == 0);
        std::string expected = "Node bloc parsing failed for entity 100, got dimension 4,"
            " expected 0, 1, 2, or 3";
        if (err_msg != expected) {
            std::cerr << "got error message: \""
                << err_msg << "\"\nbut expected: \"" << expected << "\"\n";
            return 1;
        }
    }
    // wrong number of nodes fails
    {
        std::istringstream input(
            "1 100 0 2\n" // 2 nodes given, only 1 present
            "1\n"
            "1 0 0\n"
        );
        std::string err_msg;
        auto nodes = parse_msh4_node_bloc(input, err_msg);
        assert(nodes.size() == 0);
        std::string expected = "Node bloc parsing failed during node coordinate section of entity 100";
        if (err_msg != expected) {
            std::cerr << "got error message: \""
                << err_msg << "\"\nbut expected: \"" << expected << "\"\n";
            return 1;
        }
    }
    // successfully parse a single node bloc
    {
        std::istringstream input(
            "1 1 0 3\n"
            "1\n"
            "2\n"
            "3\n"
            "1 0 0\n"
            "0 1 0\n"
            "0 0 1\n"
        );
        std::string err_msg;
        auto nodes = parse_msh4_node_bloc(input, err_msg);
        if (!err_msg.empty()) {
            std::cerr << "got error message: \"" << err_msg << "\"\n";
            return 1;
        }
        if (nodes.size() != 3) {
            std::cerr << "expected 3 nodes, got " << nodes.size() << "\n";
            return 1;
        }
        auto n0 = nodes.at(0);
        auto n1 = nodes.at(1);
        auto n2 = nodes.at(2);
        if (!(n0.tag == 1 && n0.x == 1.0 && n0.y == 0.0 && n0.z == 0.0 &&
              n1.tag == 2 && n1.x == 0.0 && n1.y == 1.0 && n1.z == 0.0 &&
              n2.tag == 3 && n2.x == 0.0 && n2.y == 0.0 && n2.z == 1.0))
        {
            std::cerr << "parsed entities didn't match reference value\n";
            return 1;
        }
    }
    return 0;
}

int test_parse_msh4_entities() {
    // bad input stream fails
    {
        std::ifstream input("bad-file");
        std::string err_msg;
        auto elts = parse_msh4_entities(input, err_msg);
        std::string expected = "$Entities parsing failed";
        if (err_msg != expected) {
            std::cerr << "got error message: \""
                << err_msg << "\"\nbut expected: \"" << expected << "\"\n";
            return 1;
        }
    }
    // no 3d entities fails
    {
        std::istringstream input(
            "2 1 1 0\n"
            "$EndEntities\n"
        );
        std::string err_msg;
        auto vols = parse_msh4_entities(input, err_msg);
        std::string expected = "$Entities parsing failed, no volumes found";
        if (err_msg != expected) {
            std::cerr << "got error message: \""
                << err_msg << "\"\nbut expected: \"" << expected << "\"\n";
            return 1;
        }
    }
    // 3d entity without a physical group fails
    {
        std::istringstream input(
            "0 0 0 1\n"
            "1 0.0 0.0 0.0 1.0 1.0 1.0 0 1\n"
            //                         ^-- num physical groups = 0
            "$EndEntities\n"
        );
        std::string err_msg;
        auto vols = parse_msh4_entities(input, err_msg);
        std::string expected = "$Entities parsing failed, volume 1 was not assigned a physical group";
        if (err_msg != expected) {
            std::cerr << "got error message: \""
                << err_msg << "\"\nbut expected: \"" << expected << "\"\n";
            return 1;
        }
    }
    // 3d entity with more than one physical group fails
    {
        std::istringstream input(
            "0 0 0 1\n"
            "2 0.0 0.0 0.0 1.0 1.0 1.0 2 1 2\n"
            //                         ^-- num physical groups = 2
            "$EndEntities\n"
        );
        std::string err_msg;
        auto vols = parse_msh4_entities(input, err_msg);
        std::string expected = "$Entities parsing failed, volume 2 has more than one physical group";
        if (err_msg != expected) {
            std::cerr << "got error message: \""
                << err_msg << "\"\nbut expected: \"" << expected << "\"\n";
            return 1;
        }
    }
    // num entities mismatch fails
    {
        std::istringstream input(
            "0 0 0 2\n"
            "2 0.0 0.0 0.0 1.0 1.0 1.0 1 1\n"
            "$EndEntities\n"
        );
        std::string err_msg;
        auto vols = parse_msh4_entities(input, err_msg);
        std::string expected = "$Entities parsing failed, expected 2 volumes but got 1";
        if (err_msg != expected) {
            std::cerr << "got error message: \""
                << err_msg << "\"\nbut expected: \"" << expected << "\"\n";
            return 1;
        }
    }
    // catch duplicate volume tags
    {
        std::istringstream input(
            "0 0 0 2\n"
            "2 0.0 0.0 0.0 1.0 1.0 1.0 1 1\n"
            "2 0.0 0.0 0.0 1.0 1.0 1.0 1 1\n"
            "$EndEntities\n"
        );
        std::string err_msg;
        auto vols = parse_msh4_entities(input, err_msg);
        std::string expected = "$Entities parsing failed, found duplicate volume tag 2";
        if (err_msg != expected) {
            std::cerr << "got error message: \""
                << err_msg << "\"\nbut expected: \"" << expected << "\"\n";
            return 1;
        }
    }
    // successfully parse volumes, skipping 0, 1, 2d entities
    {
        std::istringstream input(
            "1 2 3 2\n"
            // 1 0d entity
            "1 0 0 1 0\n"
            // 2 1d entities
            "1 -1e-007 -1e-007 -9.999999994736442e-008 1e-007 1e-007 1.0000001 0 2 2 -1\n"
            "2 -1e-007 -9.999999994736442e-008 0.9999999000000001 1e-007 1.0000001 1.0000001 0 2 1 -3\n"
            // 3 2d entities
            "1 -1e-007 -9.999999994736442e-008 -9.999999994736442e-008 1e-007 1.0000001 1.0000001 0 4 1 2 -3 -4\n"
            "2 0.9999999000000001 -9.999999994736442e-008 -9.999999994736442e-008 1.0000001 1.0000001 1.0000001 0 4 5 6 -7 -8\n"
            "3 -9.999999994736442e-008 -1e-007 -9.999999994736442e-008 1.0000001 1e-007 1.0000001 0 4 9 5 -10 -1\n"
            // 2 3d entities
        //   |-- tag                                           |-- physical group
            "1 -9.99e-008 -9.99e-008 -9.99e-008 1.0 1.0 1.0 1 100 6 1 2 3 4 5 6\n"
            "2 -9.99e-008 -9.99e-008 -9.99e-008 1.0 1.0 1.0 1 200 6 1 2 3 4 5 6\n"
            "$EndEntities\n"
        );
        std::string err_msg;
        auto vols = parse_msh4_entities(input, err_msg);
        if (!err_msg.empty()) {
            std::cerr << "got error message: \"" << err_msg << "\"\n";
            return 1;
        }
        if (vols.size() != 2) {
            std::cerr << "expected 2 volumes, got " << vols.size() << "\n";
            return 1;
        }
        if (!(vols.at(0).tag == 1 && vols.at(0).group == 100 &&
              vols.at(1).tag == 2 && vols.at(1).group == 200))
        {
            std::cerr << "parsed entities didn't match reference value\n";
            return 1;
        }
    }
    return 0;
}

// all test cases assume $Elements header has already been parsed
int test_parse_msh2_elements() {
    // empty section
    {
        std::istringstream input(
            "0\n"
            "$EndElements\n"
        );
        std::string err_msg;
        auto elts = parse_msh2_elements(input, err_msg);
        if (!err_msg.empty()) {
            std::cerr << "got error message: \"" << err_msg << "\"\n";
            return 1;
        }
        if (elts.size() != 0) {
            std::cerr << "expected 0 elements, got " << elts.size() << "\n";
            return 1;
        }
    }
    // skips non-tetrahedral elements
    {
         std::istringstream input(
            "3\n"
            "1 1 2 1 1 1 2\n" // line
            "2 3 2 1 1 1 2 3 4\n" // quad
            "3 4 2 1 1 1 2 3 4\n" // tetrahedron
            "$EndElements\n"
        );
        std::string err_msg;
        auto elts = parse_msh2_elements(input, err_msg);
        if (!err_msg.empty()) {
            std::cerr << "got error message: \"" << err_msg << "\"\n";
            return 1;
        }
        if (elts.size() != 1) {
            std::cerr << "expected 1 element, got " << elts.size() << "\n";
            return 1;
        }
        auto elt = elts.at(0);
        if (! (elt.tag == 3 &&
               elt.group == 1 &&
               elt.a == 1 &&
               elt.b == 2 &&
               elt.c == 3 &&
               elt.d == 4))
        {
            std::cerr << "parsed element didn't match reference value\n";
            return 1;
        }
    }
    // skips num_following OK
    {
         std::istringstream input(
            "2\n"
            "1 4 3 100 200 300 1 2 3 4\n"
            "2 4 3 400 200 300 1 2 3 4\n"
            "$EndElements\n"
        );
        std::string err_msg;
        auto elts = parse_msh2_elements(input, err_msg);
        if (!err_msg.empty()) {
            std::cerr << "got error message: \"" << err_msg << "\"\n";
            return 1;
        }
        if (elts.size() != 2) {
            std::cerr << "expected 1 element, got " << elts.size() << "\n";
            return 1;
        }
        if (! (elts[0].tag == 1 && elts[0].group == 100 &&
               elts[0].a == 1 && elts[0].b == 2 && elts[0].c == 3 && elts[0].d == 4 &&
               elts[1].tag == 2 && elts[1].group == 400 &&
               elts[1].a == 1 && elts[1].b == 2 && elts[1].c == 3 && elts[1].d == 4))
        {
            std::cerr << "parsed element didn't match reference value\n";
            return 1;
        }
    }
    // num_following = 0 fails
    {
         std::istringstream input(
            "1\n"
            "1 4 0 1 2 3 4\n"
            "$EndElements\n"
        );
        std::string err_msg;
        auto elts = parse_msh2_elements(input, err_msg);
        std::string expected = "got num_following = 0 which means no physical group. All elements must belong to a physical group";
        if (err_msg != expected) {
            std::cerr << "got error message: \""
                << err_msg << "\"\nbut expected: \"" << expected << "\"\n";
            return 1;
        }
    }
    // physical group = 0 fails
    {
         std::istringstream input(
            "1\n"
            "1 4 2 0 1 1 2 3 4\n"
            "$EndElements\n"
        );
        std::string err_msg;
        auto elts = parse_msh2_elements(input, err_msg);
        std::string expected =
            "got physical group id of 0, all elements must have a nonzero physical group:\n "
            "1 4 2 0 1 1 2 3 4";
        if (err_msg != expected) {
            std::cerr << "got error message: \""
                << err_msg << "\"\nbut expected: \"" << expected << "\"\n";
            return 1;
        }
    }
    // not enough nodes for a tetrahedron fails
    {
         std::istringstream input(
            "1\n"
            "1 4 2 1 1 1 2 3\n" // only 3 nodes
            "$EndElements\n"
        );
        std::string err_msg;
        auto elts = parse_msh2_elements(input, err_msg);
        std::string expected =
            "element parsing failed:\n 1 4 2 1 1 1 2 3";
        if (err_msg != expected) {
            std::cerr << "got error message: \""
                << err_msg << "\"\nbut expected: \"" << expected << "\"\n";
            return 1;
        }
    }
    return 0;
}

int test_parse_msh_file() {
    std::string header =
        "$MeshFormat\n"
        "4.1 0 8\n"
        "$EndMeshFormat\n";

    std::string pgroups =
        "$PhysicalNames\n"
        "1\n"
        "3 1 \"Steel\"\n"
        "$EndPhysicalNames\n";

    std::string nodes =
        "$Nodes\n"
        "4\n"
        "1 0 0 1\n"
        "2 0 0 0\n"
        "3 0 1 1\n"
        "4 0 1 0\n"
        "$EndNodes\n";

    std::string elts =
        "$Elements\n"
        "1160\n"
        "1 4 2 1 1 1 2 3 4\n"
        "$EndElements\n";


    // minimum complete mesh file for EGSnrc
    {
        std::istringstream input(header + pgroups + nodes + elts);
        std::string err_msg;
        parse_msh_file(input, err_msg);
        if (err_msg != "") {
            std::cerr << "got error message: \"" << err_msg << "\"\n";
            return 1;
        }
    }
    return 0;
}

int main() {
    int num_failed = 0;

    std::cerr << "starting test parse_msh_version" << std::endl;
    int err = test_parse_msh_version();
    if (err) {
        std::cerr << "test FAILED" << std::endl;
        num_failed++;
    } else {
        std::cerr << "test PASSED" << std::endl;
    }

    std::cerr << "starting test parse_msh4_entities" << std::endl;
    err = test_parse_msh4_entities();
    if (err) {
        std::cerr << "test FAILED" << std::endl;
        num_failed++;
    } else {
        std::cerr << "test PASSED" << std::endl;
    }

    std::cerr << "starting test parse_msh4_nodes" << std::endl;
    err = test_parse_msh4_nodes();
    if (err) {
        std::cerr << "test FAILED" << std::endl;
        num_failed++;
    } else {
        std::cerr << "test PASSED" << std::endl;
    }

    std::cerr << "starting test parse_msh4_groups" << std::endl;
    err = test_parse_msh4_groups();
    if (err) {
        std::cerr << "test FAILED" << std::endl;
        num_failed++;
    } else {
        std::cerr << "test PASSED" << std::endl;
    }

    std::cerr << "starting test parse_msh2_elements" << std::endl;
    err = test_parse_msh2_elements();
    if (err) {
        std::cerr << "test FAILED" << std::endl;
        num_failed++;
    } else {
        std::cerr << "test PASSED" << std::endl;
    }

    /*
    std::cerr << "starting test parse_msh_file" << std::endl;
    err = test_parse_msh_file();
    if (err) {
        std::cerr << "test FAILED" << std::endl;
        num_failed++;
    } else {
        std::cerr << "test PASSED" << std::endl;
    }
    */
    return num_failed;
}
