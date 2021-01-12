include $(EGS_CONFIG)
include $(SPEC_DIR)egspp.spec
include $(SPEC_DIR)egspp_$(my_machine).conf

TEST_CXX      = g++
TEST_CXXFLAGS = -Wall -Wextra -O2 -std=c++11 -g

all: egs-mesh-tests neighbour-test msh-parser-tests

egs-mesh-tests: egs-mesh-tests.cpp ../egs_mesh.h ../egs_mesh.cpp ../egs_vector.h
		$(TEST_CXX) $(TEST_CXXFLAGS) -I../ egs-mesh-tests.cpp ../egs_mesh.cpp -o egs-mesh-tests

msh-parser-tests: msh-parser-tests.cpp ../msh_parser.h ../egs_mesh.h ../egs_mesh.cpp
		$(TEST_CXX) $(TEST_CXXFLAGS) -I../ msh-parser-tests.cpp ../egs_mesh.cpp $(EOUT)$@

neighbour-test: neighbour-test.cpp ../mesh_neighbours.h
	$(TEST_CXX) $(TEST_CXXFLAGS) neighbour-test.cpp -I../ -o neighbour-test
