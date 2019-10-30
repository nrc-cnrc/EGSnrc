/*
###############################################################################
#
# EGSnrc egs++ auto envelope geometry
# Copyright (C) 2019 Martin Martinov
#
# This file is part of EGSnrc.
#
###############################################################################
*/

#include "egs_hexagonal_lattice.h"
#include "egs_input.h"
#include "egs_functions.h"

void EGS_DummyGeometry::setMedia(EGS_Input *,int,const int *) {
egsWarning("EGS_TransformedGeometry::setMedia: don't use this method. Use the\n"
" setMedia() methods of the geometry objects that make up this geometry\n");
}

void EGS_DummyGeometry::setRelativeRho(int start, int end, EGS_Float rho){
    setRelativeRho(0);
}

void EGS_DummyGeometry::setRelativeRho(EGS_Input *) {
    egsWarning("EGS_TransformedGeometry::setRelativeRho(): don't use this "
      "method. Use the \n setRelativeRho() methods of the underlying "
      "geometry\n");
}

void EGS_SubGeometry::setMedia(EGS_Input *,int,const int *)
{
	egsWarning("EGS_SubGeometry::setMedia: don't use this method. Use the\n"
	" setMedia() methods of the geometry objects that make up this geometry\n");
}

void EGS_SubGeometry::setRelativeRho(int start, int end, EGS_Float rho)
{
    setRelativeRho(0);
}

void EGS_SubGeometry::setRelativeRho(EGS_Input *)
{
    egsWarning("EGS_SubGeometry::setRelativeRho(): don't use this "
    "method. Use the \n setRelativeRho() methods of the underlying "
    "geometry\n");
}

extern "C" {

EGS_HEXAGONAL_LATTICE_EXPORT EGS_BaseGeometry* createGeometry(EGS_Input *input)
{
	// Honestly, this whole section is monkey see monkey do
    int error = 0;
	
	// Get base geometry
	EGS_Input *i = input->getInputItem("base geometry");
    if( !i )
	{
        egsWarning("createGeometry(hexagonal_lattice): base geometry must be defined\n" 
                   " using 'base geometry = some_geom'?\n"); return 0;
    }
    EGS_Input *ig = i->getInputItem("geometry");
    EGS_BaseGeometry *b;
    if( ig )
	{
		b = EGS_BaseGeometry::createSingleGeometry(ig);
        delete ig;
		if (!b)
		{
			egsWarning("createGeometry(hexagonal_lattice): incorrect base geometry definition\n");
			delete i;  return 0;
		}
    }
    else
	{
        string bgname;
        int err = i->getInput("base geometry",bgname);
        delete i;
        if( err )
		{
            egsWarning("createGeometry(hexagonal_lattice): missing/incorrect 'base geometry' input\n"); return 0;
        }
        b = EGS_BaseGeometry::getGeometry(bgname);
        if( !b )
		{
            egsWarning("createGeometry(envelope geometry): no geometry with name %s defined\n",
						bgname.c_str());
			return 0;
        }
    }
	
	// Get subgeometry
	i = input->getInputItem("subgeometry");
    if( !i )
	{
        egsWarning("createGeometry(hexagonal_lattice): subgeometry must be defined\n" 
                   " using 'subgeometry = some_geom'?\n"); return 0;
    }
    ig = i->getInputItem("geometry");
    EGS_BaseGeometry *s;
    if( ig )
	{
		s = EGS_BaseGeometry::createSingleGeometry(ig);
        delete ig;
		if (!s)
		{
			egsWarning("createGeometry(hexagonal_lattice): incorrect subgeometry definition\n");
			delete i;  return 0;
		}
    }
    else
	{
        string bgname;
        int err = i->getInput("subgeometry",bgname);
        delete i;
        if( err )
		{
            egsWarning("createGeometry(hexagonal_lattice): missing/incorrect 'subgeometry' input\n"); return 0;
        }
        s = EGS_BaseGeometry::getGeometry(bgname);
        if( !s )
		{
            egsWarning("createGeometry(envelope geometry): no geometry with name %s defined\n",
						bgname.c_str());
			return 0;
        }
    }
	
	// Non-geometry parameters (ie, lattice part starts here)
	int ind = -1;
	i = input->getInputItem("subgeometry index");
    if( !i )
	{
        egsWarning(
               "createGeometry(hexagonal_lattice): subgeometry index must be defined\n"
               "  using 'subgeometry index = some_index'\n");
        return 0;
	}
	else
	{
		int err = i->getInput("subgeometry index",ind);
        delete i;
        if( err )
		{
            egsWarning("createGeometry(hexagonal_lattice): missing/incorrect 'subgeometry index' input\n"); return 0;
        }
        else if (ind < 0 || ind >= b->regions()) // Not a real index
		{
            egsWarning("createGeometry(hexagonal_lattice): subgeometry index %d"
                       " is not valid, must be a region in base geometry\n",ind);
			return 0;
		}
    }
	
	EGS_Float vd = -1;
	i = input->getInputItem("volumetric density");
    if( !i )
	{
        egsWarning(
               "createGeometry(hexagonal_lattice): density must be defined\n"
               "  using 'volumetric density = some_vol_den_in_cm3'\n");
        return 0;
	}
	else
	{
		int err = i->getInput("volumetric density",vd);
        delete i;
        if( err )
		{
            egsWarning("createGeometry(hexagonal_lattice): missing/incorrect 'volumetric density' input\n"); return 0;
        }
        else if (vd <= 0) // Not a real volume density
		{
            egsWarning("createGeometry(hexagonal_lattice): volumetric density"
                       " is not valid, must be greater than zero\n");
			return 0;
		}
    }
	
	EGS_Float width = -1; // largest spacing, an arbitrary variable for egs_view purposes for too many howfar calls
	i = input->getInputItem("largest spacing");
    if( !i )
	{
        egsWarning(
               "createGeometry(hexagonal_lattice): largest spacing must be defined\n"
               "  using 'largest spacing = some_distance'\n");
        return 0;
	}
	else
	{
		int err = i->getInput("largest spacing",width);
        delete i;
        if( err )
		{
            egsWarning("createGeometry(hexagonal_lattice): missing/incorrect 'largest spacing' input\n"); return 0;
        }
        else if (width <= 0) // Not a real distance
		{
            egsWarning("createGeometry(hexagonal_lattice): largest spacing"
                       " is not valid, must be greater than zero\n");
			return 0;
		}
	}
	
	// Final build
    EGS_BaseGeometry *result;
    result = new EGS_SubGeometry(b, s, ind, /*envelope,*/ vd, width);
    result->setName(input);
    return result;
}
}
