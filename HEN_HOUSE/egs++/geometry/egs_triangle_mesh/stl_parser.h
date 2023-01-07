/*
###############################################################################
#
#  EGS_TriangleMesh STL file parser
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

#ifndef EGS_TRIANGLE_MESH_STL_PARSER_
#define EGS_TRIANGLE_MESH_STL_PARSER_

#include "egs_triangle_mesh.h" // EGS_TriangleMeshSpec

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
//#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>

namespace stl_parser {

/// Top-level STL file parser. Only binary STL files are currently supported.
///
/// Throws a std::runtime_error if parsing fails.
EGS_TriangleMeshSpec parse_stl_file(const std::string &filename,
                                    EGS_InfoFunction info = nullptr);

/// The stl_parser::internal namespace is for internal API functions and is
/// not part of the public API. Functions and types may change without warning.
namespace internal {

static inline void trim(std::string &s) {
    // ltrim
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    // rtrim
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

EGS_TriangleMeshSpec::Triangle parse_ascii_stl_triangle(std::string facet_line,
                                                        std::istream& input,
                                                        const std::string& filename) {
    EGS_TriangleMeshSpec::Triangle tri;
    // parse triangle normal
    {
        std::istringstream line_stream(facet_line);
        std::string facet;
        std::string normal;
        line_stream >> facet >> normal;
        if (facet != "facet" || normal != "normal") {
            throw std::runtime_error("failed to parse STL file `" + filename + "`, expected `facet normal`");
        }
        float n_x = 0.0;
        float n_y = 0.0;
        float n_z = 0.0;
        line_stream >> n_x >> n_y >> n_z;
        if (line_stream.fail()) {
            throw std::runtime_error("failed to parse STL file `" + filename + "`, normal parsing failed");
        }
        tri.n = EGS_Vector(n_x, n_y, n_z);
    }
    // parse vertices
    {
        std::string line;
        std::getline(input, line);
        stl_parser::internal::trim(line);
        if (line != "outer loop") {
            throw std::runtime_error("failed to parse STL file `" + filename + "`, expected `outer loop`");
        }
        auto parse_vertex = [&]() -> EGS_Vector {
            std::getline(input, line);
            stl_parser::internal::trim(line);
            std::istringstream line_stream(line);
            std::string vertex;
            float x = 0.0;
            float y = 0.0;
            float z = 0.0;
            line_stream >> vertex >> x >> y >> z;
            if (line_stream.fail() || vertex != "vertex") {
                throw std::runtime_error("failed to parse STL file `" + filename + "` vertex data");
            }
            return EGS_Vector(x, y, z);
        };
        tri.a = parse_vertex();
        tri.b = parse_vertex();
        tri.c = parse_vertex();

        std::getline(input, line);
        stl_parser::internal::trim(line);
        if (line != "endloop") {
            throw std::runtime_error("failed to parse STL file `" + filename + "`, expected `endloop`");
        }
        std::getline(input, line);
        stl_parser::internal::trim(line);
        if (line != "endfacet") {
            throw std::runtime_error("failed to parse STL file `" + filename + "`, expected `endfacet`");
        }
    }
    return tri;
}

// Parse the body of an ascii STL file into an EGS_TriangleMeshSpec. Throws a
// std::runtime_error if parsing fails.
EGS_TriangleMeshSpec parse_ascii_stl_file(std::istream& input,
                                          const std::string& filename) {
    // discard any remaining characters after `solid`
    std::string line;
    std::getline(input, line);

    std::vector<EGS_TriangleMeshSpec::Triangle> raw_triangles;

    while (std::getline(input, line)) {
        stl_parser::internal::trim(line);
        if (line.rfind("endsolid", 0) == 0) {
            break;
        }
        else if (line.rfind("facet", 0) == 0) {
            const auto tri = parse_ascii_stl_triangle(line, input, filename);
            raw_triangles.push_back(tri);
        }
        else {
            throw std::runtime_error("failed to parse STL file `" + filename + "`, expected `facet` or `endsolid`");
        }
    }

    return EGS_TriangleMeshSpec(std::move(raw_triangles));
}

// Parse the body of a binary STL file into an EGS_TriangleMeshSpec. Throws a
// std::runtime_error if parsing fails.
EGS_TriangleMeshSpec parse_binary_stl_file(std::istream& input,
                                           const std::string& filename) {
    // parse the number of triangles
    std::uint32_t n_tri = 0;
    input.read(reinterpret_cast<char*>(&n_tri), 4);
    if (!input.good()) {
        throw std::runtime_error("failed to parse the number of triangles");
    }
    if (n_tri == 0) {
        throw std::runtime_error(std::string("STL file `") + filename
                                 +  "` has 0 triangles");
    }

    // working variables for coordinate parsing
    float xf;
    float yf;
    float zf;

    // potential cast from float to EGS_Float (which is usually double)
    auto fill_egsvec = [&](EGS_Vector& v) {
        v.x = xf;
        v.y = yf;
        v.z = zf;
    };

    std::vector<EGS_TriangleMeshSpec::Triangle> raw_triangles;
    raw_triangles.resize(n_tri);
    for (std::uint32_t i = 0; i < n_tri; i++) {
        // assumed to be an outward-facing unit normal as per STL spec
        input.read(reinterpret_cast<char*>(&xf), sizeof(float));
        input.read(reinterpret_cast<char*>(&yf), sizeof(float));
        input.read(reinterpret_cast<char*>(&zf), sizeof(float));
        fill_egsvec(raw_triangles[i].n);

        // nodes assumed to be in CCW order as viewed from outside
        input.read(reinterpret_cast<char*>(&xf), sizeof(float));
        input.read(reinterpret_cast<char*>(&yf), sizeof(float));
        input.read(reinterpret_cast<char*>(&zf), sizeof(float));
        fill_egsvec(raw_triangles[i].a);

        input.read(reinterpret_cast<char*>(&xf), sizeof(float));
        input.read(reinterpret_cast<char*>(&yf), sizeof(float));
        input.read(reinterpret_cast<char*>(&zf), sizeof(float));
        fill_egsvec(raw_triangles[i].b);

        input.read(reinterpret_cast<char*>(&xf), sizeof(float));
        input.read(reinterpret_cast<char*>(&yf), sizeof(float));
        input.read(reinterpret_cast<char*>(&zf), sizeof(float));
        fill_egsvec(raw_triangles[i].c);

        // Skip any attribute bytes. These are usually zero but just in case.
        std::uint16_t attr_bytes = 0;
        input.read(reinterpret_cast<char*>(&attr_bytes), 2);
        input.ignore(attr_bytes);

        if (!input.good()) {
            throw std::runtime_error("failed to parse STL file `" + filename + "`");
        }
    }

    return EGS_TriangleMeshSpec(std::move(raw_triangles));
}

} // namespace stl_parser::internal

EGS_TriangleMeshSpec parse_stl_file(const std::string &filename,
                                    EGS_InfoFunction info /* = nullptr */) {

    std::ifstream stl_file(filename, std::ios::binary);
    if (!stl_file) {
        throw std::runtime_error("STL file `" + filename
                                 +  "` does not exist or is not readable");
    }

    // check if this is an ascii file
    std::string header;
    header.resize(5);
    stl_file.read(&header[0], 5);
    if (stl_file.fail()) {
        throw std::runtime_error("failed to parse STL file `" + filename + "`");
    }

    if (header == "solid") {
        return stl_parser::internal::parse_ascii_stl_file(stl_file, filename);
    }

    // otherwise, assume this is a binary file

    // ignore the next 75 bytes of the 80 byte binary STL header
    stl_file.ignore(75);
    return stl_parser::internal::parse_binary_stl_file(stl_file, filename);
}

} // namespace stl_parser

#endif // EGS_TRIANGLE_MESH_STL_PARSER_

/// @endcond
