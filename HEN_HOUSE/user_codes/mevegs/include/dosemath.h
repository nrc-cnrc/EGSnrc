/*
###############################################################################
#
#  EGSnrc mevegs application post-processing.
#
#  Copyright (C) 2019 Mevex Corporation
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
#  Authors:          Dave Macrillo,
#                    Matt Ronan,
#                    Nigel Vezeau,
#                    Lou Thompson,
#                    Max Orok
#
###############################################################################
#
#  Equations for output quantities.
#
###############################################################################
*/

#ifndef DOSEMATH
#define DOSEMATH

#include "../../egs++/egs_vector.h"
#include <vector>
#include <cassert>
#include <stdlib.h>

//contains pretty much everything related to calculating MevEGS's output quantities.
namespace dosemath {

  // a nice type for interacting with our data
  typedef std::vector<std::pair<std::string, std::vector<double>>> namedResults;

  //calculate volume of a tet defined by four vectors.
  inline double getTetVolume(EGS_Vector a, EGS_Vector b, EGS_Vector c, EGS_Vector d) {
    auto vol = std::abs(((a - d) * ((b - d) % (c - d))) / 6.0);
    //assert(std::abs(((a - d) * ((b - d) % (c - d))) / 6) > 0);
    //assert (vol > 0);
    return vol;
  }

  //Given all the tets for a mesh, calculates the volume of each tet.
  std::vector<double> getTetVols(const std::vector<double>& coords) {
      auto numTets = coords.size() / 12;
      std::vector<double> volumes;
      volumes.reserve(numTets);

      //coords is xyz xyz xyz xyz
      for(std::size_t i = 0; i < coords.size(); i+=12) {
        volumes.emplace_back(getTetVolume(
          EGS_Vector(coords[i], coords[i+1], coords[i+2]),
          EGS_Vector(coords[i+3], coords[i+4], coords[i+5]),
          EGS_Vector(coords[i+6], coords[i+7], coords[i+8]),
          EGS_Vector(coords[i+9], coords[i+10], coords[i+11])
        ));

        if(volumes.back() == 0) std::cout << "[DOSEMATH] Tet " << i/12 << " has a volume of 0" << std::endl;
      }
      return volumes;
  }
} // namespace dosemath

#endif
