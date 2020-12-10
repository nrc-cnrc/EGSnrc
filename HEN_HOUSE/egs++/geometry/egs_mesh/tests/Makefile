include $(EGS_CONFIG)
include $(SPEC_DIR)egspp.spec
include $(SPEC_DIR)egspp_$(my_machine).conf

include $(HEN_HOUSE)makefiles$(DSEP)cpp_makefile

msh-parser-tests.exe: msh-parser-tests.cpp ../msh_parser.h
		$(CXX) -std=c++11 -Wall -Wextra -I../ msh-parser-tests.cpp $(EOUT)$@

neighbour-test: neighbour-test.cpp ../mesh_neighbours.h
	$(CXX) $(CXXFLAGS) neighbour-test.cpp -I../ -o neighbour-test
