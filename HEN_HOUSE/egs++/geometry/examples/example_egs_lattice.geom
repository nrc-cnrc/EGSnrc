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