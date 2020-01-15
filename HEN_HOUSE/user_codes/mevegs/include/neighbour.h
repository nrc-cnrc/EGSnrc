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

//the neighbour-finding algo needs a continuous series of nodes, this function provides that
//deprecated now since the gmsh API supports it from v4.0 onward.
inline std::map<int,int> renumberNodes(std::vector<int> nodes){
  std::map<int, int> adjustment; // adjustment[idx] has number you need to
                                 // subtract from node with original num idx
  std::sort(nodes.begin(), nodes.end());

  int cumulativeSkip = nodes[0] - 1;
  adjustment[nodes[0]] = cumulativeSkip;
  //start at one to compare against elt before it
  for (std::size_t i = 1; i < nodes.size(); ++i){
    //only care if elts are different
    if (nodes[i] != nodes[i-1]){
      //only adds to the cumulativeSkip if difference is more than 1
      cumulativeSkip += nodes[i] - nodes[i-1] - 1;
    }
    adjustment[nodes[i]] = cumulativeSkip;
  }
  return adjustment;
}

//c.f. section 2.2.1 Lohner
// can only handle tets! will throw exception if not the case
// eltList is a list of all elts (with duplicates)
// indices of eltList are entries of indices
// how it works is for a given node, e.g. node n, the elts containing node n
// are contained in eltList[indices[n]] to eltList[indices[n+1]]
// n.b. that if indices[n] = indices [n+1] there are no elts containing a given node
// shouldn't occur in regular runs but may be useful for debugging
inline void elts_around_points(const std::vector<int>& elts,
                              const std::vector<int>& nodes,
                               std::vector<int>& eltList,
                               std::vector<int>& indices){

  auto begin = std::chrono::steady_clock::now();

  // two arrays needed (naming deviates from Lohner):
  // 1: Lohner's "esup1" becomes our "eltlist" vector
  // for elements numbers (with duplicates) -> non trivial size rqts, found in first pass
  // 2: Lohner's "esup2" becomes our "indices" vector
  // for indices to element sublists in array 1 -> sized as num points + 1
  // first pass -> count up storage rqmts
  // second pass -> fill list

  //get input vector sizes
  auto elts_size{elts.size()};
  auto nodes_size{nodes.size()};
  auto nodes_per_elt = nodes_size / elts_size;
  //primitive check that we're only using tets
  if (nodes_per_elt != 4) throw std::invalid_argument{"Only elements handled are tetrahedrons"};

  //counts up the number of times a number idx node is used
  for (std::size_t i = 0; i < elts_size; ++i){
    for (std::size_t j = 0; j < nodes_per_elt; ++j){
        //loop does not touch elt 0 of indices vector
        auto idx = nodes[i * nodes_per_elt + j];// + 1;
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
  //innerEltList is just mask var for eltList ref passed in the fn call
  std::vector<int> innerEltList(indices[indices.size()-1], 0);
  for (std::size_t i = 0; i < elts_size; ++i){
    for (std::size_t j = 0; j < nodes_per_elt; ++j){
        // node idx for list of indices
        auto idx = nodes[i * nodes_per_elt + j] - 1;
        // index to store at in the eltList (accessed by idx)
        // increment the index list value each time
        auto istor = indices[idx]++;
        innerEltList[istor] = i+1;   //i is base element number, +1 to align to gmsh numbering
    }
  }
  //@bug fix -> have to trick outside function into accepting reference to new vec
  eltList = innerEltList;

  // 2nd reshuffle for indices
  for (std::size_t i = indices.size()-1; i > 0; --i){
    indices[i] = indices[i-1];
  }
  //set first entry to 0
  indices[0] = 0;

  // profiling
  auto end = std::chrono::steady_clock::now();
  std::cout << "Elts around points [not wall clock] Total Time (milliseconds): " <<
  std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << std::endl;
}

//find all neighbouring elements, element value -1 by EGS convention if there is no neighbour
//indices is the numbering of those elts
//c.f. section 2.2.3 Lohner
inline void elts_around_elts(const std::vector<int>& elts,
                              const std::vector<int>& nodes,
                              std::vector<int>& eltNeighbours,
                              std::vector<int>& indices){
  // start timer
  auto begin = std::chrono::steady_clock::now();

  // declare and initialize as len 1 to avoid BUG with just declaring it
  std::vector<int> eltList(1);

  // get elts around each point -> now stored in eltList and indices
  // indices[node_num-1] is start of that node's elts in eltList
  // indices[node_num] is the end (non-inclusive)
  //
  elts_around_points(elts, nodes, eltList, indices);

  //number of unique nodes
  int num_unique_nodes = indices.size() - 1;
  //hard coded geometry values for tetrahedrons
  int num_elts = elts.size();
  int num_faces = 4;
  int nodes_per_elt = 4;
  int nodes_per_face = 3;
  //initialize vector to -1 -> egs uses this value as a boundary
  //if an elt has no neighbours on a face, that face is a boundary
  std::vector<int> eltNbrsTmp(num_faces*elts.size(), -1);


  ////////////////
  //obtain neighbouring elts
  //helper arrays
  std::vector<int> threeFacePoints(nodes_per_face, 0);
  std::vector<int> pointsOfFaceFlag(num_unique_nodes, 0);

//  std::vector<bool> pointsOfFaceFlagBool(num_unique_nodes, false); //cleaner edit to Lohner's code

  // loop over unique elts
  for (int e = 0; e < num_elts; ++e){
    // loop over faces of the elt
    for (int f = 0; f < num_faces; ++f){
      if(eltNbrsTmp[e * num_faces + f] == -1) {
      // store the nodes of a face in the threeFacePoints vector
      // we refer to the four nodes of a tet as 1 2 3 4
      // face 1 is made of nodes 2 3 4
      int ctr = 0;
      for (int nd = 0; nd < nodes_per_elt; ++nd){
        // if the nd number isn't equal to the face number
        if (nd != f){
          // assign the tag of that node to the threeFacePoints
          // indexing math: [element num * 4 npe + node num = gmsh node tag]
          threeFacePoints[ctr++] = nodes[e * nodes_per_elt + nd];
          }
        }

        //mark points in the point list help array
        for (int pt: threeFacePoints){
          pointsOfFaceFlag[pt-1] = 1;
        }

      //select one point of the face
      int ipoin = threeFacePoints[0]; // first point just as good as others

      //loop over the elts surrounding this point
      for (int istor = indices[ipoin-1]; istor < indices[ipoin]; ++istor){
        // indexed element number
        int jelt = eltList[istor];
        // if this gets to 3 (tets), jelt is a neighbour of elts[e]
        int icount = 0;
          if (jelt != elts[e]){
            for (int jface = 0; jface < num_faces; ++jface){
              icount = 0;
              //count number of equal points
              for (int jj = 0; jj < nodes_per_elt; ++jj){
                if (jj != jface){
                  int jpoin = nodes[(jelt-1) * nodes_per_elt + jj];
                  icount += pointsOfFaceFlag[jpoin-1];
                }
              }
              if (icount == nodes_per_face){
                eltNbrsTmp[e * num_faces + f] = jelt; // mark jelt as neighbour of e
                eltNbrsTmp[(jelt-1) * num_faces + jface] = elts[e]; // mark e as neighbour of jelt (obverse)
              }
            } // loop over faces of jelt
          }
        } // loop over surrounding elts of e (jelt values)
      //reset pointsOfFaceFlag to zero
      std::fill(pointsOfFaceFlag.begin(), pointsOfFaceFlag.end(), 0);
      } // check for non-boundary tets
    } // loop over faces of e
  } // loop over all elements

  //assign result to calling context
  eltNeighbours = eltNbrsTmp;

  // profiling
  auto end = std::chrono::steady_clock::now();
  std::cout << "Elts around elts [not wall clock] Total Time (milliseconds): " <<
  std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << std::endl;

}

#endif
