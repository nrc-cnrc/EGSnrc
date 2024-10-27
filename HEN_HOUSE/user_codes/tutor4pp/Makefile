###############################################################################
#
#  This is the Makefile of the tutor4pp application.
#
#  As long as your own application is derived from EGS_AdvancedApplication
#  and there is a single source file named $(USER_CODE).cpp, you can simply
#  use this Makefile by copying it into the user code directory and changing
#  the USER_CODE variable below to the name of your user code.
#
###############################################################################

include $(EGS_CONFIG)
include $(SPEC_DIR)egspp1.spec
include $(SPEC_DIR)egspp_$(my_machine).conf

# Specify the name of the user code.
# The name of the executable is determined from this variable.
#
USER_CODE = tutor4pp

# The following can be used to add user macros and mortran subroutines.
# The file(s) specified here are added after egsnrc.macros, machine.macros
# and egs_c_interface2.macros but before any files that have
# executable code.
#
EGSPP_USER_MACROS = tutor4pp.macros

# Specify from which base class this application is being derived.
# This has the effect of automatically compiling the base application
# class and including it into the list of object files.
#
EGS_BASE_APPLICATION = egs_advanced_application

# Specify the set of mortran sources used.
# Here we simply use the standard set defined in $HEN_HOUSE/specs/egspp1.spec
#
CPP_SOURCES = $(C_ADVANCED_SOURCES)

# Specify here other header files that your user code depends upon.
#
other_dep_user_code = $(ABS_EGSPP)egs_scoring.h

include $(HEN_HOUSE)makefiles$(DSEP)cpp_makefile
