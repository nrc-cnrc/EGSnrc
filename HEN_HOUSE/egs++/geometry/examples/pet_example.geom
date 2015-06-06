
###############################################################################
#
#  EGSnrc egs++ sample pet scanner geometry
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
#  Author:          Iwan Kawrakow, 2008
#
#  Contributors:
#
###############################################################################
#
#  An example geometry for egspp modeling a PET scanner with a water box.
#  This example reproduces the examples/example_PET_Scanner/PET_Ecat_System.mac
#  example distributed with GATE (except that a water box instead of a
#  water cylinder is used as a phantom).
#
###############################################################################


:start geometry definition:

    #========================================================================
    #
    #  1. Define the simulation world.
    #
    #  We use a cone stack for this purpose so that the scanner
    #  shielding is defined together with a placeholder region
    #  for the scanner ring that will come later.
    #  The scanner+shielding is 28 cm long axially, so we
    #  start at -14-delta and end at 14+delta, where delta should
    #  be at least large enough for the phantom to fit in.
    #  (you can of course make delta larger, if you believe that
    #  scattering from the surrounding air is important). In this
    #  example I select delta=6 cm to have a world that is 40 cm long.
    #
    :start geometry:
        library = egs_cones
        type    = EGS_ConeStack
        axis    = 0 0 -20.   0 0 1
        name    = base_cylinder_stack
        # cone stack layer 1: extra space for a large phantom
        # for now just filled with air
        # This is region 0 in the cone stack
        :start layer:
            thickness    = 6
            top radii    = 44.3
            bottom radii = 44.3
            media        = Air
        :stop layer:
        # cone stack layer 2: needed to define shielding component
        # This layer defines regions 3,4,5 in the cone stack
        :start layer:
            thickness    = 1.
            top radii    = 39.7  40.7  44.3
            bottom radii = 39.7  40.7  44.3
            media        = Air Tungsten Air
        :stop layer:
        # cone stack layer 3: needed to define shielding component
        # This layer defines regions 6,7,8 in the cone stack
        :start layer:
            thickness    = 2
            top radii    = 39.7  43.7  44.3
            bottom radii = 39.7  40.7  44.3
            media        = Air Tungsten Air
        :stop layer:
        # cone stack layer 4: needed to define shielding component
        # This layer defines regions 9,10,11 in the cone stack
        :start layer:
            thickness    = 1
            top radii    = 29.0  40.7  44.3
            bottom radii = 29.0  40.7  44.3
            media        = Air Tungsten Air
        :stop layer:
        # cone stack layer 5: needed to define shielding component
        # This layer defines regions 12,13,14 in the cone stack
        :start layer:
            thickness    = 2
            top radii    = 29.0  30.0  44.3
            bottom radii = 29.0  30.0  44.3
            media        = Air Tungsten Air
        :stop layer:
        # cone stack layer 6: this is where we define 2 regions so
        # that later we can put the scanner detector ring in the outer
        # region between R=41.1 and 44.3. This layer defines regions
        # 15 and 16 in the cone stack
        :start layer:
            thickness    = 16
            top radii    = 41.1 44.3
            bottom radii = 41.1 44.3
            media        = Air Air
        :stop layer:
        # cone stack layer 7: needed to define shielding component
        # This layer defines regions 18,19,20 in the cone stack
        :start layer:
            thickness    = 2
            top radii    = 29.0  30.0  44.3
            bottom radii = 29.0  30.0  44.3
            media        = Air Tungsten Air
        :stop layer:
        # cone stack layer 8: needed to define shielding component
        # This layer defines regions 21,22,23 in the cone stack
        :start layer:
            thickness    = 1
            top radii    = 29.0  40.7  44.3
            bottom radii = 29.0  40.7  44.3
            media        = Air Tungsten Air
        :stop layer:
        # cone stack layer 9: needed to define shielding component
        # This layer defines regions 24,25,26 in the cone stack
        :start layer:
            thickness    = 2
            top radii    = 39.7  43.7  44.3
            bottom radii = 39.7  40.7  44.3
            media        = Air Tungsten Air
        :stop layer:
        # cone stack layer 10: needed to define shielding component
        # This layer defines regions 27,28,29 in the cone stack
        :start layer:
            thickness    = 1.
            top radii    = 39.7  40.7  44.3
            bottom radii = 39.7  40.7  44.3
            media        = Air Tungsten Air
        :stop layer:
        # cone stack layer 11: extra space for a large phantom
        # for now just filled with air
        # This is region 30 in the cone stack
        :start layer:
            thickness    = 6.
            top radii    = 44.3
            bottom radii = 44.3
            media        = Air
        :stop layer:
    :stop geometry:

    #=========================================================================
    #
    #  2. Define one segment of the scanner detector ring.
    #
    #  Note: 8 axial blocks are combined into a single XYZ geometry
    #        Reason: *way* more efficient at run time.
    #
    :start geometry:
        library = egs_ndgeometry
        type = EGS_XYZGeometry
        name = radial_block
        x-planes = 41.2 44.2

        #
        # Note: the correct crystal size and spaces between crystals are
        #       defined in the commented out y-planes and z-planes.
        #       I have increased the air gaps between the crystals so that
        #       one can better see what is going on in the viewer.
        #       Don't forget to uncomment the correct definition and
        #       comment out the wrong one for simulations!
        #
        #y-planes = -1.79297 -1.35297 -1.34355 -0.90355 -0.89413 -0.45413 \
        #           -0.44471 -0.00471 0.00471 0.44471 0.45413 0.89413 0.90355 \
        #           1.34355 1.35297 1.79297
        #z-planes = -7.755 -7.28 -7.27 -6.795 -6.785 -6.31 -6.3 -5.825 \
        #           -5.815 -5.34 -5.33 -4.855 -4.845 -4.37 -4.36 -3.885 -3.875\
        #           -3.4 -3.39 -2.915 -2.905 -2.43 -2.42 -1.945 -1.935 -1.46\
        #           -1.45 -0.975 -0.965 -0.49 -0.48 -0.005 0.005 0.48 0.49\
        #           0.965 0.975 1.45 1.46 1.935 1.945 2.42 2.43 2.905\
        #           2.915 3.39 3.4 3.875 3.885 4.36 4.37 4.845 4.855\
        #           5.33 5.34 5.815 5.825 6.3 6.31 6.785 6.795 7.27\
        #           7.28 7.755
        y-planes = -1.69297 -1.45297 -1.24355 -1.00355 -0.79413 -0.55413 \
                   -0.34471 -0.10471 0.10471 0.34471 0.55413 0.79413 1.00355 \
                   1.24355 1.45297 1.69297
        z-planes = -7.655 -7.38 -7.17 -6.895 -6.685 -6.41 -6.2 -5.925 \
                   -5.715 -5.44 -5.23 -4.955 -4.745 -4.47 -4.26 -3.985 -3.675\
                   -3.5 -3.29 -3.015 -2.805 -2.53 -2.32 -2.045 -1.835 -1.56\
                   -1.35 -1.075 -0.865 -0.59 -0.38 -0.105 0.105 0.38 0.59\
                   0.865 1.075 1.35 1.56 1.835 2.045 2.32 2.53 2.805\
                   3.015 3.29 3.5 3.775 3.985 4.26 4.47 4.745 4.955\
                   5.23 5.44 5.715 5.925 6.2 6.41 6.685 6.895 7.17 7.38 7.655
        :start media input:
            media = Air BGO
            # The above line filles all regions with Air
            # The line below fills avery second region in y- and z- with BGO
            set medium = 0 0 1  0 1000 2  0 1000 2     1
        :stop media input:

    :stop geometry:

    #=========================================================================
    #
    #  3. Construct the scanner ring from the radial block defined above
    #     using a radial repeater.
    #
    :start geometry:
        name = scanner_ring
        library = egs_iplanes
        type = EGS_RadialRepeater
        axis    = 0 0 0   0 0 1   #  The axis of rotation
        repeated geometry = radial_block
        number of repetitions = 72
        medium = Air # <- set the space outside of all repetitions to Air
    :stop geometry:

    #=========================================================================
    #
    #  4. Put the scanner ring into region 16 of the cone stack
    #     base_cylinder_stack. We use a a CD geometry for this
    #     puspose. We also utilize the new indexing style so that
    #     we don't get a huge number of virtual (non-existing) regions.
    #
    :start geometry:
        name = scanner
        library = egs_cdgeometry
        new indexing style = 1
        base geometry = base_cylinder_stack
        set geometry = 16 scanner_ring
    :stop geometry:

    #=========================================================================
    #
    #  5. Define a phantom
    #
    #  Any geometry constructable with egspp could have been used,
    #  we define a simple 30x30x30 cm water box.
    #
    :start geometry:
        library = egs_box
        name    = phantom
        box size = 30
        :start media input:
            media = Water
        :stop media input:
    :stop geometry:

    #========================================================================
    #
    #  6. Inscribe the phantom into the scanner.
    #
    #     Note that we use 'nscribe in regions' instead of
    #     'inscribed geometries'. The advantage of this technique is
    #     that geometry checks for intersections with the phantom will
    #     only be made when particles are in one of the regions listed,
    #     instead of always as would be the case with 'inscribed geometries'.
    #
    :start geometry:
        library = egs_genvelope
        name = final_scanner
        base geometry = scanner
        #inscribed geometries = phantom
        inscribe in regions = phantom 0 3 6 9 12 15 68058 68061 68064 68067\
                              68070
    :stop geometry:

    simulation geometry = scanner

:stop geometry definition:
