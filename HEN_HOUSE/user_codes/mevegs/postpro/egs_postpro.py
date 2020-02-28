###############################################################################
#
#  EGSnrc mevegs application post-processing script.
#
#  Copyright (C) 2020 Mevex Corporation
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
#  Authors:          Max Orok
#
#  Contributors:     Dave Macrillo,
#                    Matt Ronan,
#                    Nigel Vezeau,
#                    Lou Thompson,
#
###############################################################################
#
#  Calculate various quantities using EGSnrc output data and the Gmsh API.
#
###############################################################################

import gmsh
import sys
import gmsh_io

# fixed views from mevegs.cpp
ENERGY_FRACTION_VIEW = 0
ENERGY_VIEW = 1
UNCERTAINTY_VIEW = 2
VOLUME_VIEW = 3
DENSITY_VIEW = 4

# physics constants
JOULES_PER_MEV = 1.602e-13
ELECTRONS_PER_COULOMB = 6.24150e18

# -----------------------------------------------------------------------------

# Element doses in gray [Gy]
def element_doses(energies, masses):
    return [energy * JOULES_PER_MEV / mass for (energy, mass) in zip(energies, masses)]

# Element masses in kg
def element_masses(volumes, densities):
    return [vol * dens / 1000.0 for (vol, dens) in zip(volumes, densities)]

# Get the average source particle energy [MeV] based on deposited energy and energy fraction.
def average_source_energy(energies, e_fractions):
    eps = 1e-6
    for (energy, frac) in zip(energies, e_fractions):
        if frac > eps:
            return energy / frac

# Get the dose to an element for a Coulomb's worth of source electronvolts [Gy/C]
def element_doses_per_coulomb(energies, masses):
    return [dose * ELECTRONS_PER_COULOMB for dose in element_doses(energies, masses)]

# -----------------------------------------------------------------------------

# main
def analyze(filename):
    gmsh.initialize()
    gmsh.open(filename)

    elements, energies = gmsh_io.get_scalar_data(ENERGY_VIEW)
    _, e_fractions = gmsh_io.get_scalar_data(ENERGY_FRACTION_VIEW)
    _, uncerts = gmsh_io.get_scalar_data(UNCERTAINTY_VIEW)
    _, volumes = gmsh_io.get_scalar_data(VOLUME_VIEW)
    _, densities = gmsh_io.get_scalar_data(DENSITY_VIEW)

    masses = element_masses(volumes, densities)
    doses = element_doses(energies, masses)

    # add/comment out views here
    gmsh_io.append_to_file("Element mass [kg]", filename, elements, masses)
    gmsh_io.append_to_file("Element dose [Gy]", filename, elements, doses)
    gmsh_io.append_to_file("Element dose per Coulomb [Gy/C]", filename, elements, element_doses_per_coulomb(energies, masses))

    # add total energy in a volume
    gmsh_io.append_to_file("Total volume energy [MeV]", filename, *gmsh_io.get_volume_totals(ENERGY_VIEW))

    gmsh.finalize()

# cli
if __name__ == "__main__":
    len(sys.argv) == 2 or gmsh_io.abort("usage: python3 " + __file__ + " output.results.msh")
    analyze(sys.argv[1])
