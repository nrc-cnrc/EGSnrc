#include "egs_mesh.h"
#include "egs_vector.h"

#include <cassert>

#define RUN_TEST(test_fn) \
    std::cerr << "starting test " << #test_fn << std::endl; \
    err = test_fn; \
    num_total++; \
    if (err) { \
        std::cerr << "test FAILED" << std::endl; \
        num_failed++; \
    } else { \
        std::cerr << "test passed" << std::endl; \
    }

int test_isInside() {
    EGS_Vector o(0, 0, 0);

    return 0;
}

int main() {
    int num_failed = 0;
    int num_total = 0;
    int err = 0;

    RUN_TEST(test_isInside());

    std::cerr << num_total - num_failed << " out of " << num_total << " tests passed\n";
    return num_failed;
}
