###############################################################################
#
#  EGSnrc egs++ makefile for the coincident-boundary navigation test
#
#  Usage (EGS_CONFIG and HEN_HOUSE set, egs++ libraries built):
#      make            build coincident_test
#      make check      run every case in cases/
#
###############################################################################

include $(EGS_CONFIG)
include $(SPEC_DIR)egspp.spec
include $(SPEC_DIR)egspp_$(my_machine).conf

INCL = -I$(HEN_HOUSE)egs++ -I$(HEN_HOUSE)lib$(DSEP)$(my_machine)
CASES = $(sort $(wildcard cases/*.egsinp))

all: coincident_test$(EXE)

coincident_test$(EXE): coincident_test.cpp $(ABS_DSO)$(libpre)egspp$(libext)
	$(CXX) $(INCL) $(DEF1) $(opt) $(lib_link1) coincident_test.cpp $(EOUT)$@ $(link2_prefix)egspp$(link2_suffix)

check: coincident_test$(EXE)
	@status=0; for c in $(CASES); do ./coincident_test$(EXE) $$c || status=1; done; exit $$status

clean:
	$(REMOVE) coincident_test$(EXE)

.PHONY: all check clean
