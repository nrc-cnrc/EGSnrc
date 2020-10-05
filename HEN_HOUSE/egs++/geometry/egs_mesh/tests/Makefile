include $(EGS_CONFIG)
include $(SPEC_DIR)egspp.spec
include $(SPEC_DIR)egspp_$(my_machine).conf

bad-egs_mesh-inputs.exe: bad-egs_mesh-inputs.cpp
		$(CXX) $(INCL) -I../ $(DEF1) $(opt) $(lib_link1) bad-egs_mesh-inputs.cpp $(EOUT)$@ $(lib_link2) -legs_mesh

include $(HEN_HOUSE)makefiles$(DSEP)cpp_makefile
