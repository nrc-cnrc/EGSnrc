
###############################################################################
#
#  EGSnrc egs++ makefile to build the egs++ library
#  Copyright (C) 2015 National Research Council Canada
#
#  This file is part of EGSnrc.
#
#  EGSnrc is free software: you can redistribute it and/or modify it under
#  the terms of the GNU Affero General Public License as published by the
#  Free Software Foundation, either version 3 of the License, or (at your
#  option) any later version.
#
#  EGSnrc is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
#  FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for
#  more details.
#
#  You should have received a copy of the GNU Affero General Public License
#  along with EGSnrc. If not, see <http://www.gnu.org/licenses/>.
#
###############################################################################
#
#  Author:          Iwan Kawrakow, 2005
#
#  Contributors:    Ernesto Mainegra-Hing
#                   Frederic Tessier
#                   Manuel Stoeckl
#
###############################################################################


include $(EGS_CONFIG)
include $(SPEC_DIR)egspp.spec
include $(SPEC_DIR)egspp_$(my_machine).conf

DEFS = $(DEF1) -DBUILD_DLL

egspp_files = egs_input egs_base_geometry egs_library egs_transformations \
             egs_rndm egs_shapes egs_geometry_tester egs_timer egs_polygon \
             egs_projectors egs_alias_table egs_object_factory egs_spectra \
             egs_base_source egs_functions egs_application egs_run_control \
             egs_scoring egs_interpolator egs_atomic_relaxations \
             egs_ausgab_object egs_particle_track egs_fortran_geometry \
             egs_ensdf

egspp_objects = $(addprefix $(DSO1), $(addsuffix .$(obje), $(egspp_files)))
config1h = $(IEGS1)$(DSEP)egs_config1.h egs_libconfig.h egs_functions.h

geometry_libs = egs_planes egs_cd_geometry egs_gtransformed egs_nd_geometry \
               egs_box egs_genvelope egs_spheres egs_cylinders egs_iplanes \
               egs_cones egs_gstack egs_prism egs_union egs_pyramid egs_conez\
               egs_space egs_elliptic_cylinders egs_smart_envelope \
               egs_vhp_geometry egs_octree egs_roundrect_cylinders

source_libs = egs_collimated_source egs_isotropic_source egs_parallel_beam \
              egs_point_source egs_source_collection egs_transformed_source \
              egs_beam_source egs_phsp_source egs_angular_spread \
              iaea_phsp_source egs_radionuclide_source

shape_libs = egs_circle egs_ellipse egs_extended_shape egs_gaussian_shape \
             egs_line_shape egs_polygon_shape egs_rectangle egs_shape_collection \
             egs_voxelized_shape

aobject_libs = egs_track_scoring egs_dose_scoring

all_libs = $(geometry_libs) $(source_libs) $(shape_libs) $(aobject_libs)

#******************************************************************************

all: $(EGS_BINDIR)egspp$(EXE) $(ABS_DSO)$(libpre)egspp$(libext) glibs slibs shapes aobjects gtest

$(EGS_BINDIR)egspp$(EXE): $(dso) $(DSO1)egspp.$(obje) $(ABS_DSO)$(libpre)egspp$(libext)
	$(CXX) $(INC1) $(DEF1) $(opt) $(EOUT)$@ $(DSO1)egspp.$(obje) $(lib_link1) $(link2_prefix)egspp$(link2_suffix)

$(ABS_DSO)$(libpre)egspp$(libext): $(dso) $(egspp_objects)
	$(CXX) $(INC1) $(DEFS) $(opt) $(shared) $(egspp_objects) $(extra)

$(DSO1)egspp.$(obje): egspp.cpp egs_application.h egs_base_geometry.h egs_vector.h \
	               egs_math.h egs_library.h $(config1h)
	$(CXX) $(INC1) $(DEF1) $(opt) -c $(COUT)$@ $(notdir $(basename $@)).cpp

$(DSO1)egs_interpolator.$(obje): egs_interpolator.cpp egs_interpolator.h \
                                 $(config1h)

$(DSO1)egs_scoring.$(obje): egs_scoring.cpp egs_scoring.h $(config1h) egs_math.h

$(DSO1)egs_input.$(obje): egs_input.cpp egs_input.h $(config1h)

$(DSO1)egs_base_geometry.$(obje): egs_base_geometry.cpp egs_base_geometry.h \
	egs_vector.h egs_library.h egs_input.h $(config1h)

$(DSO1)egs_library.$(obje): egs_library.cpp egs_library.h $(config1h)

$(DSO1)egs_transformations.$(obje): egs_transformations.cpp \
    egs_transformations.h egs_vector.h egs_input.h egs_math.h $(config1h)

$(DSO1)egs_rndm.$(obje): egs_rndm.cpp egs_rndm.h egs_input.h egs_math.h \
    $(config1h)

$(DSO1)egs_shapes.$(obje): egs_shapes.cpp egs_shapes.h egs_vector.h egs_math.h \
    egs_input.h egs_library.h egs_object_factory.h egs_rndm.h $(config1h) \
    egs_transformations.h

$(DSO1)egs_timer.$(obje): egs_timer.cpp egs_timer.h $(config1h)

$(DSO1)egs_projectors.$(obje): egs_projectors.cpp egs_projectors.h \
    egs_vector.h $(config1h)

$(DSO1)egs_polygon.$(obje):  egs_polygon.cpp egs_polygon.h $(config1h) \
    egs_projectors.h egs_math.h egs_vector.h

$(DSO1)egs_alias_table.$(obje): egs_alias_table.cpp egs_alias_table.h \
    $(config1h) egs_rndm.h egs_math.h

$(DSO1)egs_object_factory.$(obje): egs_object_factory.cpp egs_object_factory.h \
    egs_library.h egs_input.h $(config1h)

$(DSO1)egs_spectra.$(obje): egs_spectra.cpp egs_base_source.h $(config1h) \
    egs_rndm.h egs_object_factory.h egs_alias_table.h egs_vector.h \
    egs_input.h egs_math.h

$(DSO1)egs_base_source.$(obje): egs_base_source.cpp egs_base_source.h \
    egs_vector.h $(config1h) egs_object_factory.h egs_input.h

$(DSO1)egs_geometry_tester.$(obje): egs_geometry_tester.cpp \
    egs_geometry_tester.h egs_input.h egs_vector.h egs_base_geometry.h \
    egs_shapes.h egs_rndm.h egs_transformations.h egs_timer.h egs_math.h \
    egs_object_factory.h $(config1h)

$(DSO1)egs_functions.$(obje): egs_functions.cpp $(config1h)

$(DSO1)egs_application.$(obje): egs_application.cpp egs_application.h

$(DSO1)egs_run_control.$(obje): egs_run_control.cpp egs_run_control.h

$(DSO1)egs_atomic_relaxations.$(obje): egs_atomic_relaxations.cpp egs_atomic_relaxations.h \
    $(config1h) egs_simple_container.h egs_rndm.h egs_math.h egs_application.h \
    egs_base_geometry.h egs_vector.h

$(DSO1)egs_ausgab_object.$(obje): egs_ausgab_object.cpp egs_ausgab_object.h \
    $(config1h) egs_application.h egs_base_geometry.h egs_vector.h egs_math.h \
    egs_object_factory.h egs_input.h

$(DSO1)egs_particle_track.$(obje): egs_particle_track.cpp egs_particle_track.h $(config1h)

$(DSO1)egs_fortran_geometry.$(obje): egs_fortran_geometry.cpp egs_fortran_geometry.h \
    $(config1h) egs_base_geometry.h egs_vector.h egs_math.h egs_simple_container.h \
    egs_input.h

#----------------------------------------------------------------------------------------------

$(egspp_objects):
	$(obj_rule1)

$(dso): dso
	@$(CHECK_DIR) $(MKDIR)

dso:
	@$(CHECK_DIR) $(MKDIR)

gtest: $(DSO1)gtest$(EXE)

$(DSO1)gtest$(EXE): test_geometry.cpp egs_base_geometry.h egs_vector.h \
                 egs_geometry_tester.h egs_input.h $(config1h) $(ABS_DSO)$(libpre)egspp$(libext)
	$(CXX) $(INC1) $(DEF1) $(opt) $(lib_link1) test_geometry.cpp $(EOUT)$@ $(lib_link2)

test_source: $(DSO1)test_source.exe;

$(DSO1)test_source.exe: test_source.cpp egs_input.h $(config1h) \
	                 egs_object_factory.h egs_base_source.h egs_vector.h \
                        egs_rndm.h egs_math.h
	$(CXX) $(INC1) $(DEF1) $(opt) $(lib_link1) test_source.cpp $(EOUT)$@ $(lib_link2)

glibs: $(geometry_libs)

$(geometry_libs): $(ABS_DSO)$(libpre)egspp$(libext)
	cd geometry$(DSEP)$@ && $(MAKE)

slibs: $(source_libs)

$(source_libs): $(ABS_DSO)$(libpre)egspp$(libext)
	cd sources$(DSEP)$@ && $(MAKE)

shapes: $(shape_libs)

$(shape_libs): $(ABS_DSO)$(libpre)egspp$(libext)
	cd shapes$(DSEP)$@ && $(MAKE)

aobjects: $(aobject_libs)

$(aobject_libs): $(ABS_DSO)$(libpre)egspp$(libext)
	cd ausgab_objects$(DSEP)$@ && $(MAKE)

check:
	@echo "targets  : $(EGS_BINDIR)egspp$(EXE) $(ABS_DSO)$(libpre)egspp$(libext) glibs slibs shapes gtest"
	@echo "obj_rule1: $(obj_rule1)"
	@echo "obj_rule2: $(obj_rule2)"

clean:
	$(REMOVE) $(egspp_objects) $(DSO1)test.exe


.PHONY: libs $(all_libs) clean
