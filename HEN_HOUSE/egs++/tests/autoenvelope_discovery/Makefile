###############################################################################
#
#  EGSnrc egs++ makefile for the egs_autoenvelope region-discovery test
#
#  Usage (EGS_CONFIG and HEN_HOUSE set, egs++ and egs_autoenvelope built):
#      make            build discovery_test
#      make check      run every case in cases/
#
###############################################################################

include $(EGS_CONFIG)
include $(SPEC_DIR)egspp.spec
include $(SPEC_DIR)egspp_$(my_machine).conf

INCL = -I$(HEN_HOUSE)egs++ -I$(HEN_HOUSE)lib$(DSEP)$(my_machine)
CASES = $(sort $(wildcard cases/*.egsinp))

all: discovery_test$(EXE)

discovery_test$(EXE): discovery_test.cpp $(ABS_DSO)$(libpre)egspp$(libext)
	$(CXX) $(INCL) $(DEF1) $(opt) $(lib_link1) discovery_test.cpp $(EOUT)$@ $(link2_prefix)egspp$(link2_suffix)

check: discovery_test$(EXE)
	@status=0; for c in $(CASES); do ./discovery_test$(EXE) $$c 2>/dev/null || status=1; done; exit $$status

clean:
	$(REMOVE) discovery_test$(EXE)

.PHONY: all check clean
