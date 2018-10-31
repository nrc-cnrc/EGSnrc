#*****************************************************************************
#
#  $Id: Makefile,v 1.2 2005/06/27 17:24:30 iwan Exp $
#
#*****************************************************************************

include $(EGS_CONFIG)
include $(SPEC_DIR)egspp.spec
include $(SPEC_DIR)egspp_$(my_machine).conf

DEFS = $(DEF1) -DBUILD_CUBIC_LATTICE_DLL

library = egs_cubic_lattice
lib_files = egs_cubic_lattice
my_deps = egs_transformations.h geometry/egs_gtransformed/egs_gtransformed.h
extra_dep = $(addprefix $(DSOLIBS), $(my_deps))

include $(SPEC_DIR)egspp_libs.spec

$(make_depend)
