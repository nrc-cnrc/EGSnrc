
###############################################################################
#
#  EGSnrc egs++ sample linear accelerator treatment head geometry
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
#  Contributors:
#
###############################################################################
#
#  An example geometry input file for the egs++ geometry package.
#
#  This input file defines an egs++ geometry for the treatment head of a
#  medical linear accelerator in photon mode. It is based on the EX16MVp
#  example geometry that is included in BEAMnrc (except that the monitor
#  chamber is not modeled for the sake of simplicity).
#
#  The accelerator is modeled as a CD geometry (EGS_CDGeometry) where the
#  mirror and the two pairs of jaws are put into the appropriate regions of
#  the base linac geometry, which is a cone stack (EGS_ConeStack). The cone
#  stack defines most of the accelerator geometry (bremsstrahlung target,
#  primary collimator and flattening filter) and provides placeholder regions
#  for the mirror and the two pairs of jaws.
#
###############################################################################


:start geometry definition:

    :start geometry:

        library = egs_cones      # this is the geometry library
        type    = EGS_ConeStack  # this is the object type that we need
                                 # from the geometry library egs_cones
        axis    = 0 0 0   0 0 1  # we use the z-axis as the axis of
                                 # the cone stack
        name    = linac          # and give it the very original name 'linac'

        ######################### the target
        :start layer:
            thickness    = 0.5
            top radii    = 2.5  9.11111
            bottom radii = 2.5  9.11111
            media        = W700ICRU AIR700ICRU
        :stop layer:
        #  ends at z = 0.5   regions 0...2

        ######################### air between target and primary collimator
        :start layer:
            thickness    = 0.4
            bottom radii = 2.5  9.11111
            media        = AIR700ICRU AIR700ICRU
        :stop layer:
        #  ends at z = 0.9   regions 3...5

        ######################### the primary collimator
        :start layer:
            thickness    = 5
            top radii    = 0.5  9.11111
            bottom radii = 3.0  9.11111
            media        = AIR700ICRU W700ICRU
        :stop layer:
        #  ends at z = 5.9   regions 6...8

        ######################### air between primary collimator and
        #                         flattening filter
        :start layer:
            thickness    = 2.1
            top radii    = 9.11111
            bottom radii = 9.11111
            media        = AIR700ICRU
        :stop layer:
        #  ends at z = 8   regions 9...11

        ######################## start of flattening filter (ff)
        #
        #----------------------- layer 1
        :start layer:
            thickness    = 0.3
            top radii    = 0    9.11111
            bottom radii = 0.5  9.11111
            media        = FE700ICRU AIR700ICRU
        :stop layer:
        #  ends at z = 8.3   regions 12...14
        #----------------------- layer 2
        :start layer:
            thickness    = 0.7
            bottom radii = 0.8  9.11111
            media        = FE700ICRU AIR700ICRU
        :stop layer:
        #  ends at z = 9.0   regions 15...17
        #----------------------- layer 3
        :start layer:
            thickness    = 1.0
            top radii    = 0    0.8  9.11111
            bottom radii = 0.4  1.7  9.11111
            media        = W700ICRU FE700ICRU AIR700ICRU
        :stop layer:
        #  ends at z = 9.11111   regions 18...20
        #----------------------- layer 4
        :start layer:
            thickness    = 1.2
            bottom radii = 1.2  3.0  9.11111
            media        = W700ICRU FE700ICRU AIR700ICRU
        :stop layer:
        #  ends at z = 11.2   regions 21...23
        #
        ############################ end of flattening filter


        ##################### we skip the monitor chamber
        #                     (too lazy to input so many layers)
        #
        ##################### air between ff and mirror
        :start layer:
            thickness    = 9.8
            top radii    = 9.11111
            bottom radii = 9.11111
            media        = AIR700ICRU
        :stop layer:
        #  ends at z = 21.0   regions 24...26
        ##################### placeholder for the mirror
        #                     it is left empty for now, it will be
        #                     divided into regions later on
        #                     when this geometry becomes the base geometry
        #                     of a CD geometry used to model the final linac
        #
        :start layer:
            thickness    = 2
            bottom radii = 9.11111
            media        = AIR700ICRU
        :stop layer:
        #  ends at z = 23.0   regions 27...29

        ##################### air between mirror and jaws
        :start layer:
            thickness    = 6
            bottom radii = 9.11111
            media        = AIR700ICRU
        :stop layer:
        #  ends at z = 29.0   regions 30...32

        ##################### placeholder for the x-jaws
        #                     same comments as for the mirror layer.
        #
        :start layer:
            thickness    = 9
            bottom radii = 9.11111
            media        = W700ICRU
        :stop layer:
        #  ends at z = 38.0   regions 33...35

        ##################### air between x- and y-jaws
        :start layer:
            thickness    = 0.5
            bottom radii = 9.11111
            media        = AIR700ICRU
        :stop layer:
        #  ends at z = 38.5   regions 36...38

        ##################### placeholder for the y-jaws
        #                     same comments as for the mirror layer.
        #
        :start layer:
            thickness    = 9
            bottom radii = 9.11111
            media        = W700ICRU
        :stop layer:
        #  ends at z = 47.5   regions 39...41

    :stop geometry:
    ################################### end of the cone stack named 'linac'

    ################################### the mirror planes
    #                                   we make them extend to infinity
    #                                   but they will be cut to the desired
    #                                   size when put into the linac
    :start geometry:
        library = egs_planes
        type    = EGS_Planes
        name    = mirror_planes
        normal  =  0.316227766017 0 0.948683298051
        #positions = -100 20.8709325571 20.8710325571 20.8775325571 120
        #
        #    Let's use a single, slightly ticker layer for better
        #    visibility
        #
        positions = -100 20.8 21 120
        :start media input:
            media = AIR700ICRU AL700ICRU PMMA700ICRU
            set medium = 1 1
        :stop media input:
    :stop geometry:

    ################################### the x-jaws planes
    #                 We use a plane collection which will be cut
    #                 to the proper size in the z-direction when
    #                 put into the x-jaws placeholder in the linac geometry
    #                 We make the left- and right-most regions very large
    #                 These regions will be cut by the bounding cylinder of
    #                 the base linac geometry
    #
    :start geometry:
        library = egs_planes
        type    = EGS_PlaneCollection
        name    = x_jaws
        normals = 1,0,0    0.99875,0,0.049938,  0.99875,0,-0.049938, 1,0,0
        positions = -50  0  0   50
        :start media input:
            media = W700ICRU AIR700ICRU
            set medium = 0 2 0
            set medium = 1 1
        :stop media input:
    :stop geometry:

    ################################### the y-jaws planes
    #                 We use a plane collection which will be cut
    #                 to the proper size in the z-direction when
    #                 put into the x-jaws placeholder in the linac geometry
    #                 We make the left- and right-most regions very large
    #                 These regions will be cut by the bounding cylinder of
    #                 the base linac geometry
    #
    :start geometry:
        library = egs_planes
        type    = EGS_PlaneCollection
        name    = y_jaws
        normals = 0,1,0    0,0.99875,0.049938,  0,0.99875,-0.049938, 0,1,0
        positions = -50  0  0   50
        :start media input:
            media = W700ICRU AIR700ICRU
            set medium = 0 2 0
            set medium = 1 1
        :stop media input:
    :stop geometry:


    ################################## the final accelerator
    :start geometry:
        library = egs_cdgeometry
        name    = final_linac
        base geometry = linac
        set geometry = 27 mirror_planes   # define the mirror
        set geometry = 33 x_jaws          # define the x-jaws
        set geometry = 39 y_jaws          # define the y-jaws
    :stop geometry:

    simulation geometry = final_linac
    #simulation geometry = linac

:stop geometry definition:
