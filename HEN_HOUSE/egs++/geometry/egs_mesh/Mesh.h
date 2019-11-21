/*
###############################################################################
#
#  EGSnrc egs++ mesh geometry library Mesh object.
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
#  Contains all information about a mesh as set in Gmsh.
#  Actually built up using the gmsh_manip.h functions.
#
###############################################################################
*/

//TODO think about whether find neighbour fns belong in cstor...

#ifndef MESH
#define MESH

#include <algorithm>
#include <vector>
#include <tuple>
#include <map>
#include <iostream>
#include <stdio.h>
#include <string>
#include <fstream>
#include <sstream>
#include <cassert>

class Mesh {

public:

    // read only data
    const std::string fileName;
    const std::vector<int> nodes;
    const std::map<int, std::tuple<double, double, double>> coordMap;
    const std::vector<int> elts;
    const std::vector<int> eltNeighbours;
    const std::vector<int> media;
    const std::map<int, std::string> mediaMap;
    const std::vector<int> boundaryTet;
    const std::vector<double> rhor;

private:

  // only handles tetrahedrons for now
  const int neighbours_per_elt = 4;
  const int coords_per_elt = 12;
  const int nodes_per_elt = 4;


  //TODO watch this -> could have errors for 64 bit ints
  std::pair<std::vector<std::string>, std::vector<std::vector<std::vector<double>>>> eltData;


public:

    Mesh() = delete; // there's only one way to make a Mesh

  //NB coords is 3 times length of unique number of nodes, need to match coords before calling MEVEGS
  //nodes are matched to elts i.e. there are duplicates in nodes
  Mesh(const std::string _fileName,
     const std::vector<int>& _nodes,
     const std::map<int, std::tuple<double, double, double>>& _coordMap,
     const std::vector<int>& _elts,
     const std::vector<int>& _neighbours,
     const std::vector<int>& _media,
     const std::map<int, std::string> _mediaMap,
     const std::vector<double>& _rhor):
   fileName(_fileName), nodes(_nodes), coordMap(_coordMap), elts(_elts), eltNeighbours(_neighbours),
   media(_media), mediaMap(_mediaMap), rhor(_rhor) {

	  //find neighbours TODO actually implement it here instead of in gmsh_manip
    findNeighbours();

    //inititalize boundary tet vector with information from neighbours
    std::vector<int> tempBoundary;
    tempBoundary.reserve(elts.size());
    // boundaryTet.reserve(elts.size());
    bool boundaryFlag = false; //zero for non boundary, 1 for boundary
    for (std::size_t i = 0; i < elts.size(); ++i){
      boundaryFlag =  (eltNeighbours[neighbours_per_elt*i]   == -1) ||
                      (eltNeighbours[neighbours_per_elt*i+1] == -1) ||
                      (eltNeighbours[neighbours_per_elt*i+2] == -1) ||
                      (eltNeighbours[neighbours_per_elt*i+3] == -1); // if any neighbours are -1
      tempBoundary.emplace_back(boundaryFlag); // implicit conversion to int here
      }

      const_cast <std::vector<int>&> (this->boundaryTet) = std::move(tempBoundary);

      //check that input data is well formed
      assert(boundaryTet.size() == elts.size());
	 }


  //TODO merge mvx_test1 neighbours calls into this fn
  void findNeighbours(){

  }

  // deprecated method used for "tet" files, which used to be read in by
  //@arg fileName: the desired file to write to, e.g. "data.txt"
  // if this file does not already exist, it will be created - be careful!
  // if it already exists, will overwrite it
  //TODO consider need for append flag in arg list
  void writeToEGSINP(const std::string& fileName) const{
  // out, trunc not needed but spells out intent
  std::ofstream outf(fileName, std::ofstream::out | std::ofstream::trunc);
  //check if we actually opened
  if (outf){
    //print media names on the top line
    for (auto const & medName : mediaMap){
      outf << medName.second << " ";
    }
    outf << std::endl; //newline and loop over all elts

    //intitalize idx helper vars and loop over all elts
    for (std::size_t i = 0; i < elts.size(); ++i){
      //write 12 coords of tet
      for (std::size_t ic = 0; ic < nodes_per_elt; ++ic){
        // unpack tuple to x y z
        double x,y,z;
        std::tie(x,y,z) = coordMap.at(nodes[i*nodes_per_elt+ic]);
        outf << x << " " << y << " " << z << " ";
      }
      //write 4 neighbours of tet
      for (std::size_t in = 0; in < neighbours_per_elt; ++in){
        int neighbour = eltNeighbours[i*neighbours_per_elt+in];
        if (neighbour == -1){
          outf << neighbour << " ";
        }
        else { // need (num - 1) for correct indexing in egs
          outf << (neighbour - 1) << " ";
        }
      }
      //write whether boundary tet or not (1, 0 respectively)
      outf << boundaryTet[i] << " ";
      // write media index set in gmsh session
      outf << media[i] << std::endl;
    }
  }
  outf.close();
  }

  //expand the map of unique coords to a vector of all coords used by egs
  const std::vector<double> getCoords() const{
	  std::vector<double> coordList;
  coordList.reserve(elts.size()*coords_per_elt);

  for (std::size_t i = 0; i < elts.size(); ++i){
    //TODO add list of media strings for the top line - use gmsh info/
    //write 12 coords of tet
    for (std::size_t ic = 0; ic < nodes_per_elt; ++ic){
      // unpack tuple to x y z
      double x,y,z;
      // std::cout << "cm idx" << nodes[i*nodes_per_elt+ic] << std::endl;
      std::tie(x,y,z) = coordMap.at(nodes[i*nodes_per_elt+ic]);
      coordList.emplace_back(x);
      coordList.emplace_back(y);
      coordList.emplace_back(z);
    }
  }

  return coordList;
  }

  // convert signed element tags to unsigned size tags like gmsh expects
  std::vector<std::size_t> getUnsignedElements() const {
      std::vector<std::size_t> unsignedElts;
      std::transform(elts.cbegin(), elts.cend(),
                     std::back_inserter(unsignedElts),
                     [](int tag) -> std::size_t { return static_cast<std::size_t>(tag);});
      return unsignedElts;
  }

  template<typename T>
  std::vector<T> getDataByMedia(std::vector<T> input,
                                std::vector<std::string> names) const {

    std::vector<T> result;
    int i = 0;

    for(const auto inp : input) {
        for(const auto name : names) {
            if(!mediaMap.empty() && !media.empty() && name.compare(mediaMap.at(media.at(i))) == 0) {
              result.emplace_back(inp);
            }
        }
        ++i;
    }
    return result;
  }

};

#endif
