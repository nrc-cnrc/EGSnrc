/*
###############################################################################
#
#  EGSnrc mevegs application mesh element neighbours.
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
#  Fast nearest-neighbours algorithm for tetrahedral meshes.
#  Adapted from Löhner's Applied CFD Techniques 2e.
#  O(nlog(n))
#
###############################################################################
*/

#ifndef NEIGHBOUR
#define NEIGHBOUR

#include <algorithm>
#include <chrono>

using std::chrono::steady_clock;

// c.f. section 2.2.1 Löhner
// can only handle tets! will throw exception if not the case
// eltList is a list of all elts (with duplicates)
// indices of eltList are entries of indices
// how it works is for a given node, e.g. node n, the elts containing node n
// are contained in eltList[indices[n]] to eltList[indices[n+1]]
// n.b. that if indices[n] = indices [n+1] there are no elts containing a given node
// shouldn't occur in regular runs but may be useful for debugging
std::pair<std::vector<int>, std::vector<int>> // <eltList, indices>
elements_around_points(std::size_t nodes_per_element, const std::vector<int>& nodes) {

  // algorithm is general for mesh elements, but was only tested with tetrahedrons.
  assert(nodes_per_element == 4);
  assert(nodes.size() % nodes_per_element == 0);

  auto begin = steady_clock::now();

  // the number of unique nodes is equal to the maximum node number
  auto num_unique_nodes = *std::max_element(nodes.begin(), nodes.end());

  // two arrays needed (naming deviates from Löhner):
  // 1: esup1 becomes the eltlist vector
  //    for elements numbers (with duplicates) -> non trivial size rqts, found in first pass
  // 2: esup2 becomes the indices vector
  //    for indices to element sublists in array 1 -> sized as num points + 1
  // first pass -> find storage requirements
  // second pass -> fill list

  std::vector<int> indices(num_unique_nodes + 1, 0);
  auto num_elts = nodes.size() / nodes_per_element;

  //counts up the number of times a number idx node is used
  for (std::size_t i = 0; i < num_elts; ++i){
    for (std::size_t j = 0; j < nodes_per_element; ++j){
        // skips first element, always set to zero later
        auto idx = nodes[i * nodes_per_element + j];
        ++indices[idx];
    }
  }

  //reshuffle pass 1
  //adds up storage rqmts to align with a bounding box
  for (std::size_t i = 1; i < indices.size(); ++i){
    indices[i] = indices[i] + indices[i-1];
  }

  ///////
  //second pass - store in new array
  //we know size of array is last elt of indices array
  std::vector<int> eltList(indices.back());

  for (std::size_t i = 0; i < num_elts; ++i){
    for (std::size_t j = 0; j < nodes_per_element; ++j){
        // node idx for list of indices
        auto idx = nodes[i * nodes_per_element + j] - 1;
        // index to store at in the eltList (accessed by idx)
        // increment the index list value each time
        auto istor = indices[idx]++;
        eltList[istor] = i;   //i is base element number, +1 to align to gmsh numbering
    }
  }

  // 2nd reshuffle for indices
  for (std::size_t i = indices.size()-1; i > 0; --i){
    indices[i] = indices[i-1];
  }
  //set first entry to 0
  indices[0] = 0;

  std::cout << "Found elements around points in "
      << std::chrono::duration_cast<std::chrono::milliseconds>(steady_clock::now() - begin).count()
      << " milliseconds\n";

  return std::make_pair(eltList, indices);
}

//find all neighbouring elements, element value -1 by EGS convention if there is no neighbour
//indices is the numbering of those elts
//c.f. section 2.2.3 Löhner
std::vector<int>
tetrahedron_neighbours(const std::vector<int>& nodes) {

  auto begin = steady_clock::now();

  // get elts around each point -> now stored in eltList and indices
  // indices[node_num-1] is start of that node's elts in eltList
  // indices[node_num] is the end (non-inclusive)
  std::vector<int> eltList;
  std::vector<int> indices;
  constexpr int nodes_per_element = 4;
  std::tie(eltList, indices) = elements_around_points(nodes_per_element, nodes);

  // number of unique nodes (indices is one bigger than the number of unique nodes)
  int num_unique_nodes = indices.size() - 1;
  // hard coded geometry values for tetrahedrons
  int num_elts = nodes.size() / nodes_per_element;
  int num_faces = 4;
  int nodes_per_face = 3;
  // initialize vector to -1 -> egs uses this value as a boundary
  // if an elt has no neighbours on a face, that face is a boundary
  std::vector<int> neighbours(num_faces * num_elts, -1);

  ////////////////
  //obtain neighbouring elts
  //helper arrays
  std::array<int, 3> face_points = {0, 0, 0};
  std::vector<int> pointsOfFaceFlag(num_unique_nodes, 0);

// std::vector<bool> pointsOfFaceFlagBool(num_unique_nodes, false); //cleaner edit to Löhner's code

  // loop over unique elts
  for (int e = 0; e < num_elts; ++e){
    // loop over faces of the elt
    for (int f = 0; f < num_faces; ++f){
      if(neighbours[e * num_faces + f] == -1) {
      // store the nodes of a face in the threeFacePoints vector
      // we refer to the four nodes of a tet as 1 2 3 4
      // face 1 is made of nodes 2 3 4
      int ctr = 0;
      for (int nd = 0; nd < nodes_per_element; ++nd){
        // if the nd number isn't equal to the face number
        if (nd != f){
          // assign the tag of that node to the threeFacePoints
          // indexing math: [element num * 4 npe + node num = gmsh node tag]
          face_points[ctr++] = nodes[e * nodes_per_element + nd];
          }
        }

        //mark points in the point list help array
        for (int pt: face_points){
          pointsOfFaceFlag[pt-1] = 1;
        }

      //select one point of the face
      int ipoin = face_points.front(); // first point just as good as others

      //loop over the elts surrounding this point
      for (int istor = indices[ipoin-1]; istor < indices[ipoin]; ++istor){
        // indexed element number
        int jelt = eltList[istor];
        // if this gets to 3 (tets), jelt is a neighbour of elts[e]
        int icount = 0;
          if (jelt != e){
            for (int jface = 0; jface < num_faces; ++jface){
              icount = 0;
              //count number of equal points
              for (int jj = 0; jj < nodes_per_element; ++jj){
                if (jj != jface){
                  int jpoin = nodes[jelt * nodes_per_element + jj];
                  icount += pointsOfFaceFlag[jpoin-1];
                }
              }
              if (icount == nodes_per_face){
                neighbours[e * num_faces + f] = jelt; // mark jelt as neighbour of e
                neighbours[jelt * num_faces + jface] = e; // mark e as neighbour of jelt (obverse)
              }
            } // loop over faces of jelt
          }
        } // loop over surrounding elts of e (jelt values)
      //reset pointsOfFaceFlag to zero
      std::fill(pointsOfFaceFlag.begin(), pointsOfFaceFlag.end(), 0);
      } // check for non-boundary tets
    } // loop over faces of e
  } // loop over all elements


  std::cout << "Found element neighbours in "
      << std::chrono::duration_cast<std::chrono::milliseconds>(steady_clock::now() - begin).count()
      << " milliseconds\n";

  return neighbours;
}

#endif
