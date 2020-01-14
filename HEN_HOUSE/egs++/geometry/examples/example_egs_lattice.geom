/*
###############################################################################
#
#  EGSnrc egs++ lattice geometry
#  Copyright (C) 2019 Rowan Thomson and Martin Martinov
#
#  This file is part of EGSnrc.
#
#  EGSnrc is free software: you can redistribute it and/or modify it under
#  the terms of the GNU Affero General Public License as published by the Free
#  Software Foundation, either version 3 of the License, or (at your option)
#  any later version.
#
#  EGSnrc is distributed in the hope that it will be useful, but WITHOUT
#  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
#  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License
#  for more details.
#
#  To see the GNU Affero General Public License at:
# <http://www.gnu.org/licenses/>.
#
################################################################################
#
#  When egs_lattice is used for publications, please cite the following paper:
#  Manuscript submission underway, to be replaced with title after publication
#
###############################################################################
#
#  Author:          Martin Martinov, 2019
#
#  Contributors:
#
###############################################################################
#
#  An example geometry of three different lattices (Bravais lattice of lead
#  spheres, cubic lattice of gold spheres, and hexagonal lattice of silver
#  spheres) set up in three different regions in a large cube of water.
#
###############################################################################
*/

:start geometry definition:		
	:start geometry:
		library  = egs_spheres
		type     = EGS_cSpheres
		name     = subgeom1
		midpoint = 0 0 0
		radii    = 0.1
		:start media input:
			media = LEAD
		:stop media input:
	:stop geometry:
	:start geometry:
		library  = egs_spheres
		type     = EGS_cSpheres
		name     = subgeom2
		midpoint = 0 0 0
		radii    = 0.1
		:start media input:
			media = GOLD
		:stop media input:
	:stop geometry:
	:start geometry:
		library  = egs_spheres
		type     = EGS_cSpheres
		name     = subgeom3
		midpoint = 0 0 0
		radii    = 0.1
		:start media input:
			media = SILVER
		:stop media input:
	:stop geometry:

	:start geometry:
		library  = egs_ndgeometry
		type     = EGS_XYZgeometry
		name     = phantom
		x-planes = -10 -9 -3 3 9 10
		y-planes = -10 10
		z-planes = -10 10
		:start media input:
			media = WATER
		:stop media input:
	:stop geometry:
	
	:start geometry:
		library           = egs_lattice
		name              = phantom_w_Bravais
		base geometry     = phantom
		subgeometry       = subgeom1
		subgeometry index = 1
		spacing           = 1 2 3
	:stop geometry:
	:start geometry:
		library           = egs_lattice
		name              = phantom_w_Bravais_Cubic
		base geometry     = phantom_w_Bravais
		subgeometry       = subgeom2
		subgeometry index = 2
		spacing           = 1
	:stop geometry:
	:start geometry:
		library           = egs_lattice
		type              = hexagonal
		name              = phantom_w_Bravais_Cubic_Hex
		base geometry     = phantom_w_Bravais_Cubic
		subgeometry       = subgeom3
		subgeometry index = 3
		spacing           = 1
	:stop geometry:
	
	simulation geometry = phantom_w_Bravais_Cubic_Hex
:stop geometry definition:
