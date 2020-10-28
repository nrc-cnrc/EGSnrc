include $(EGS_CONFIG)
include $(SPEC_DIR)egspp.spec
include $(SPEC_DIR)egspp_$(my_machine).conf

msh-parser-tests.exe: msh-parser-tests.cpp ../msh_parser.h
		$(CXX) -Wall -Wextra -I../ msh-parser-tests.cpp $(EOUT)$@

bad-egs_mesh-inputs.exe: bad-egs_mesh-inputs.cpp
		$(CXX) $(INCL) -Wall -Wextra -I../ $(DEF1) $(opt) $(lib_link1) bad-egs_mesh-inputs.cpp $(EOUT)$@ $(lib_link2) -legs_mesh

include $(HEN_HOUSE)makefiles$(DSEP)cpp_makefile
