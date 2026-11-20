/*
###############################################################################
#
#  EGSnrc estar
#  Copyright (C) 2026 National Research Council Canada
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
#  Author:          Sehmimul Hoque, 2022
#
#  Contributors:    Martin J. Berger
#                   Johnathan S. Coursey
#                   Reid Townson
#                   Ernesto Mainegra-Hing
#
#  Based on the original ESTAR code by Martin J. Berger,
#  National Institute of Standards and Technology (NIST). Including
#  modifications by Johnathan S. Coursey.
#
###############################################################################
*/

#pragma once

#include <string>
#include <map>

extern const double energy_grid[113];
extern const double atb[100];
extern const double poth[100];
extern const double potgas[9];
extern const double potcon[9];
extern const std::map<std::string, int> atomic_number;

extern const double elementData[14532];

