#include "egs_functions.h"
#include "egs_input.h"
#include "egs_mesh.h"
#include <iostream>

int requires_mesh_file() {
    EGS_Input inp;
    EGS_BaseGeometry *geo;

    // fails if the file key is not present
    std::cerr << "\tno_file_key:" << std::endl;
    std::string no_file_key =
        ":start geometry definition:\n"
        "   :start geometry:\n"
        "       library = egs_mesh\n"
        "       # file = test.msh\n"
        "       name = mesh\n"
        "   :stop geometry:\n"
        "   simulation geometry = mesh\n"
        ":stop geometry definition:";
    inp.setContentFromString(no_file_key);
    geo = EGS_BaseGeometry::createGeometry(&inp);
    if (geo != nullptr) {
        std::cerr << "\tegs_mesh cannot be constructed without the `file` key" << std::endl;
        std::cerr << "\t...failed" << std::endl;
        return 1;
    }
    std::cerr << "\t...passed" << std::endl;

    std::cerr << "\tno_such_file:" << std::endl;
    // fails if given a non-existing file
    std::string no_such_file =
        ":start geometry definition:\n"
        "   :start geometry:\n"
        "       library = egs_mesh\n"
        "       file = bad-file.msh\n"
        "       name = mesh\n"
        "   :stop geometry:\n"
        "   simulation geometry = mesh\n"
        ":stop geometry definition:";
    inp.setContentFromString(no_such_file);
    geo = EGS_BaseGeometry::createGeometry(&inp);
    if (geo != nullptr) {
        std::cerr << "\tegs_mesh cannot be constructed without a valid file" << std::endl;
        std::cerr << "\t...failed" << std::endl;
        return 1;
    }
    std::cerr << "\t...passed" << std::endl;

    std::cerr << "\tok_file:" << std::endl;
    // succeeds if given a valid file
    std::string ok_file =
        ":start geometry definition:\n"
        "   :start geometry:\n"
        "       library = egs_mesh\n"
        "       file = test.msh\n"
        "       name = mesh\n"
        "   :stop geometry:\n"
        "   simulation geometry = mesh\n"
        ":stop geometry definition:";
    inp.setContentFromString(ok_file);
    geo = EGS_BaseGeometry::createGeometry(&inp);
    if (geo == nullptr) {
        std::cerr << "\texpected valid egs_mesh" << std::endl;
        std::cerr << "\t...failed" << std::endl;
        return 1;
    }
    std::cerr << "\t...passed" << std::endl;

    return 0;
}

void do_nothing(const char *,...) {}

int main() {
    // override egsFatal to avoid terminating the program for our expected failures
    egsSetInfoFunction(Fatal, do_nothing);
    // override egsWarning to avoid overwhelming the test runner output
    egsSetInfoFunction(Warning, do_nothing);

    std::string test_name = "requires_mesh_file";
    std::cerr << "starting test `" << test_name << "`" << std::endl;
    int err = requires_mesh_file();
    if (err) {
        std::cerr << "test `" << test_name << "` FAILED" << std::endl;
        return 1;
    }
    std::cerr << "test `" << test_name << "` PASSED" << std::endl;
}
