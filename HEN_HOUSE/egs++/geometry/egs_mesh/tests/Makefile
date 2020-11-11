include $(EGS_CONFIG)
include $(SPEC_DIR)egspp.spec
include $(SPEC_DIR)egspp_$(my_machine).conf

msh-parser-tests.exe: msh-parser-tests.cpp ../msh_parser.h
		$(CXX) -Wall -Wextra -I../ msh-parser-tests.cpp $(EOUT)$@

include $(HEN_HOUSE)makefiles$(DSEP)cpp_makefile
