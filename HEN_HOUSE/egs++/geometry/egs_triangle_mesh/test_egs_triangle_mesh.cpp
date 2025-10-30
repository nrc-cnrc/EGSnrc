/*
###############################################################################
#
#  EGSnrc egs_triangle_mesh test suite
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

#include "egs_input.h"
#include "egs_triangle_mesh.h"
#include "stl_parser.h"

#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <sstream>

// Test runner globals
int num_total = 0;
int num_failed = 0;

#define RUN_TEST(test_fn) \
    std::cerr << "test " << #test_fn << "... "; \
    num_total++; \
    try { \
        test_fn; \
        std::cerr << "ok\n"; \
    } catch (const std::runtime_error& err) { \
        num_failed++; \
        std::cerr << "FAILED: " << err.what() << "\n"; \
    }

#define EXPECT_ERROR(stmt, err_msg) \
    try { \
        stmt; \
        std::ostringstream oss; \
        oss << "expected exception with message: \"" << err_msg << "\""; \
        throw std::runtime_error(oss.str()); \
    } catch (const std::exception& err) { \
        if (err.what() != std::string(err_msg)) { \
            std::ostringstream oss; \
            oss << "got error message: \"" \
                << err.what() << "\" but expected: \"" << err_msg << "\""; \
            throw std::runtime_error(oss.str()); \
        } \
    }


// RAII class for a temporary file
class TempFile {
public:
    TempFile(const std::string &filename, const std::string &contents)
        : filename_(filename) {
        {
            std::ofstream out(filename_);
            out << contents;
        }
    }

    ~TempFile() {
        std::remove(this->filename_.c_str());
    }

    std::string filename() const {
        return filename_;
    }
private:
    std::string filename_;
};

static bool egsvec_eq(EGS_Vector a, EGS_Vector b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

static bool approx_eq(double a, double b, double e = 1e-6) {
    return (std::abs(a - b) <= e * (std::abs(a) + std::abs(b) + 1.0));
}

static bool egsvec_approx_eq(EGS_Vector a, EGS_Vector b) {
    return approx_eq(a.x, b.x) && approx_eq(a.y, b.y) && approx_eq(a.z, b.z);
}

// Custom egsInfoFunction that throws error messages as exceptions for testing
void egsInfoThrowing(const char *msg, ...) {
    char buf[8192];
    va_list ap;
    va_start(ap, msg);
    vsprintf(buf, msg, ap);
    va_end(ap);
    throw std::runtime_error(buf); // buf copied by constructor
}

namespace stl_parser {

static void missing_file() {
    EXPECT_ERROR(stl_parser::parse_stl_file("temp_missing.stl"), "STL file "
                 "`temp_missing.stl` does not exist or is not readable");
}

static void empty_file() {
    TempFile empty("temp_empty.stl", "");
    EXPECT_ERROR(stl_parser::parse_stl_file(empty.filename()),
                 "failed to parse STL file `temp_empty.stl`");
}

static void parse_ascii_file() {
    TempFile ascii("temp_ascii.stl", R"(solid
facet normal 0.6229 0.35962 0.694744
  outer loop
    vertex 43.062 20.491 -149.441
    vertex 41.768 19.197 -147.611
    vertex 43.946 19.607 -149.776
  endloop
endfacet
facet normal 0.730297 0.632387 0.258365
  outer loop
    vertex 43.536 20.965 -151.941
    vertex 43.062 20.491 -149.441
    vertex 43.946 19.607 -149.776
  endloop
endfacet
facet normal 0.81508 0.547608 0.189131
  outer loop
    vertex 43.536 20.965 -151.941
    vertex 43.946 19.607 -149.776
    vertex 44.593 18.96 -150.691
  endloop
endfacet
endsolid)");

    auto mesh = stl_parser::parse_stl_file(ascii.filename());
    if (mesh.elements.size() != 3) {
        throw std::runtime_error("expected 3 triangles, got "
                                 + std::to_string(mesh.elements.size()));
    }
    if (!egsvec_approx_eq(mesh.elements[0].n, EGS_Vector(0.6229f, 0.35962f, 0.694744f)) ||
            !egsvec_approx_eq(mesh.elements[0].a, EGS_Vector(43.062f, 20.491f, -149.441f)) ||
            !egsvec_approx_eq(mesh.elements[0].b, EGS_Vector(41.768f, 19.197f, -147.611f)) ||
            !egsvec_approx_eq(mesh.elements[0].c, EGS_Vector(43.946f, 19.607f, -149.776f))) {
        throw std::runtime_error("element 0 parsing failed");
    }
    if (!egsvec_approx_eq(mesh.elements[1].n, EGS_Vector(0.730297f, 0.632387f, 0.258365f)) ||
            !egsvec_approx_eq(mesh.elements[1].a, EGS_Vector(43.536f, 20.965f, -151.941f)) ||
            !egsvec_approx_eq(mesh.elements[1].b, EGS_Vector(43.062f, 20.491f, -149.441f)) ||
            !egsvec_approx_eq(mesh.elements[1].c, EGS_Vector(43.946f, 19.607f, -149.776f))) {
        throw std::runtime_error("element 1 parsing failed");
    }
    if (!egsvec_approx_eq(mesh.elements[2].n, EGS_Vector(0.81508f, 0.547608f, 0.189131f)) ||
            !egsvec_approx_eq(mesh.elements[2].a, EGS_Vector(43.536f, 20.965f, -151.941f)) ||
            !egsvec_approx_eq(mesh.elements[2].b, EGS_Vector(43.946f, 19.607f, -149.776f)) ||
            !egsvec_approx_eq(mesh.elements[2].c, EGS_Vector(44.593f, 18.96f, -150.691f))) {
        throw std::runtime_error("element 2 parsing failed");
    }
}

static void catch_zero_triangles() {
    // write 84 zero bytes (80 byte comment + 4 n_tri)
    std::string header(84, 0);
    TempFile binfile("temp_zero_tris.stl", header);
    EXPECT_ERROR(stl_parser::parse_stl_file(binfile.filename()),
                 "STL file `temp_zero_tris.stl` has 0 triangles");
}

static void truncated_file() {
    // write binary file header
    // 1. 80 byte comment
    std::string header(80, 'A');
    // 2. Number of triangles (LE bytes for 100,000)
    for (auto b : {
                0xA0, 0x86, 0x1, 0
            }) {
        header.push_back(b);
    }
    TempFile binfile("temp_truncated_file.stl", header);
    std::ifstream input(binfile.filename(), std::ios::binary);
    // Missing triangle data
    EXPECT_ERROR(stl_parser::parse_stl_file(binfile.filename()),
                 "failed to parse STL file `temp_truncated_file.stl`");
}

static void parse_binary_file() {
    // Test parser with known binary STL file
    EGS_TriangleMeshSpec mesh = stl_parser::parse_stl_file("sample_bin.stl");

    if (mesh.elements.size() != 3) {
        throw std::runtime_error("expected 3 triangles, got "
                                 + std::to_string(mesh.elements.size()));
    }

    if (!egsvec_approx_eq(mesh.elements[0].n, EGS_Vector(0.6229f, 0.35962f, 0.694744f)) ||
            !egsvec_approx_eq(mesh.elements[0].a, EGS_Vector(43.062f, 20.491f, -149.441f)) ||
            !egsvec_approx_eq(mesh.elements[0].b, EGS_Vector(41.768f, 19.197f, -147.611f)) ||
            !egsvec_approx_eq(mesh.elements[0].c, EGS_Vector(43.946f, 19.607f, -149.776f))) {
        throw std::runtime_error("element 0 parsing failed");
    }
    if (!egsvec_approx_eq(mesh.elements[1].n, EGS_Vector(0.730297f, 0.632387f, 0.258365f)) ||
            !egsvec_approx_eq(mesh.elements[1].a, EGS_Vector(43.536f, 20.965f, -151.941f)) ||
            !egsvec_approx_eq(mesh.elements[1].b, EGS_Vector(43.062f, 20.491f, -149.441f)) ||
            !egsvec_approx_eq(mesh.elements[1].c, EGS_Vector(43.946f, 19.607f, -149.776f))) {
        throw std::runtime_error("element 1 parsing failed");
    }
    if (!egsvec_approx_eq(mesh.elements[2].n, EGS_Vector(0.81508f, 0.547608f, 0.189131f)) ||
            !egsvec_approx_eq(mesh.elements[2].a, EGS_Vector(43.536f, 20.965f, -151.941f)) ||
            !egsvec_approx_eq(mesh.elements[2].b, EGS_Vector(43.946f, 19.607f, -149.776f)) ||
            !egsvec_approx_eq(mesh.elements[2].c, EGS_Vector(44.593f, 18.96f, -150.691f))) {
        throw std::runtime_error("element 2 parsing failed");
    }
}

} // namespace stl_parser

namespace egs_triangle_mesh {

static void input_errors() {
    egsSetInfoFunction(Warning, egsInfoThrowing);
    egsSetInfoFunction(Fatal, egsInfoThrowing);

    // Missing `file` key fails
    {
        EGS_Input egsinp;
        std::string bad_stl_file(
            ":start geometry definition:\n"
            "    :start geometry:\n"
            "       name = my_mesh\n"
            "       library = egs_triangle_mesh\n"
            "       #file = bad_input.stl\n" // commented out filename
            "    :stop geometry:\n"
            "    simulation geometry = my_mesh\n"
            ":stop geometry definition:\n"
        );
        egsinp.setContentFromString(bad_stl_file);
        EXPECT_ERROR(EGS_TriangleMesh::createGeometry(&egsinp),
                     "createGeometry(EGS_TriangleMesh): no mesh file key `file` in input\n");
    }

    // Non-existent STL file fails
    {
        EGS_Input egsinp;
        std::string bad_stl_file(
            ":start geometry definition:\n"
            "    :start geometry:\n"
            "       name = my_mesh\n"
            "       library = egs_triangle_mesh\n"
            "       file = bad_input.stl\n" // bad filename
            "    :stop geometry:\n"
            "    simulation geometry = my_mesh\n"
            ":stop geometry definition:\n"
        );
        egsinp.setContentFromString(bad_stl_file);
        EXPECT_ERROR(EGS_TriangleMesh::createGeometry(&egsinp),
                     "\ncreateGeometry(EGS_TriangleMesh): STL file `bad_input.stl` does not exist or is not readable\n");
    }

    // bad scale value fails
    {
        EGS_Input egsinp;
        std::string bad_stl_scale(
            ":start geometry definition:\n"
            "    :start geometry:\n"
            "       name = my_mesh\n"
            "       library = egs_triangle_mesh\n"
            "       file = sample_bin.stl\n"
            "       scale = -1.0\n"
            "    :stop geometry:\n"
            "    simulation geometry = my_mesh\n"
            ":stop geometry definition:\n"
        );
        egsinp.setContentFromString(bad_stl_scale);
        EXPECT_ERROR(EGS_TriangleMesh::createGeometry(&egsinp),
                     "createGeometry(EGS_TriangleMesh): invalid scale value (-1), expected a positive number\n");
    }

    // Reset egs info functions
    egsSetDefaultIOFunctions();
}

static void zero_elements() {
    EGS_TriangleMeshSpec spec; // empty mesh spec
    EXPECT_ERROR(EGS_TriangleMesh(std::move(spec)), "empty triangles vector in EGS_TriangleMesh constructor");

}

static void check_scale() {

    EGS_TriangleMeshSpec mesh = stl_parser::parse_stl_file("sample_bin.stl");
    EGS_TriangleMeshSpec scaled_mesh = stl_parser::parse_stl_file("sample_bin.stl");
    scaled_mesh.scale(0.1);

    for (int i = 0; i < mesh.elements.size(); i++) {
        if (!egsvec_eq(mesh.elements[i].n, scaled_mesh.elements[i].n) ||
                !egsvec_eq(0.1 * mesh.elements[i].a, scaled_mesh.elements[i].a) ||
                !egsvec_eq(0.1 * mesh.elements[i].b, scaled_mesh.elements[i].b) ||
                !egsvec_eq(0.1 * mesh.elements[i].c, scaled_mesh.elements[i].c)) {
            throw std::runtime_error("STL mesh scaling failed");
        }
    }
}

// TODO: is there a way to test that `set media` block exists and only has
// one medium? Maybe not, looking at the EGS_BaseGeometry::setMedia impl.

} // namespace egs_triangle_mesh

int main() {

    RUN_TEST(stl_parser::empty_file());
    RUN_TEST(stl_parser::missing_file());
    RUN_TEST(stl_parser::catch_zero_triangles());
    RUN_TEST(stl_parser::truncated_file());
    RUN_TEST(stl_parser::parse_binary_file());
    RUN_TEST(stl_parser::parse_ascii_file());

    RUN_TEST(egs_triangle_mesh::input_errors());
    RUN_TEST(egs_triangle_mesh::zero_elements());
    RUN_TEST(egs_triangle_mesh::check_scale());

    std::cerr << "\ntest result: " << num_total - num_failed << " out of " <<
              num_total << " tests passed\n";

    return num_failed;
}

#undef RUN_TEST
#undef EXPECT_ERROR
