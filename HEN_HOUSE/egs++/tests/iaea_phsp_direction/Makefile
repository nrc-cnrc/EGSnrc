###############################################################################
#
#  EGSnrc egs++ makefile for the iaea_phsp_source direction test
#
#  Usage (EGS_CONFIG and HEN_HOUSE set, egs++, iaea_phsp and iaea_phsp_source built):
#      make            build direction_test
#      make check      run it
#
###############################################################################

include $(EGS_CONFIG)
include $(SPEC_DIR)egspp.spec
include $(SPEC_DIR)egspp_$(my_machine).conf

INCL = -I$(HEN_HOUSE)egs++ -I$(HEN_HOUSE)lib$(DSEP)$(my_machine) -I$(HEN_HOUSE)iaea_phsp

all: direction_test$(EXE)

direction_test$(EXE): direction_test.cpp $(ABS_DSO)$(libpre)egspp$(libext)
	$(CXX) $(INCL) $(DEF1) $(opt) $(lib_link1) direction_test.cpp $(EOUT)$@ \
	    $(link2_prefix)egspp$(link2_suffix) $(link2_prefix)iaea_phsp$(link2_suffix)

# libiaea_phsp is built with a relative install name on macOS, so point the loader at the dso
check: direction_test$(EXE)
	DYLD_LIBRARY_PATH=$(ABS_DSO) LD_LIBRARY_PATH=$(ABS_DSO) ./direction_test$(EXE) .

clean:
	$(REMOVE) direction_test$(EXE)

.PHONY: all check clean
