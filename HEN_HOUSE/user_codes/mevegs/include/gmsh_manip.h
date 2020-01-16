/*
###############################################################################
#
#  EGSnrc mevegs application Gmsh interface.
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
#  Functions for interacting with the Gmsh API.
#
###############################################################################
*/

#ifndef GMSH_MANIP
#define GMSH_MANIP

#include <chrono>

#include "gmsh.h"
#include "Mesh.h"
#include "neighbour.h"

namespace gmsh_manip{

//We have vectors of data. gmsh wants vectors of vectors of data.
//This function makes gmsh happy.
inline std::vector<std::vector<double>> format_data(std::vector<double> data){
  //vector of vectors to adhere to gmsh data format
  std::vector<std::vector<double>> vectorizedData;

  for (auto dataIt = data.cbegin(); dataIt != data.cend(); ++dataIt){
    vectorizedData.emplace_back(std::vector<double>(1, *dataIt));
  }
  return vectorizedData;
}

//save a mesh object to a pos or .msh file
//return 0 if good
int saveMeshOutput(const Mesh& mesh,
                   const std::vector<std::pair<std::string, std::vector<double>>>& allResults,
                   const std::string& egsinpFileN){

  auto begin = std::chrono::steady_clock::now();
  auto fName = mesh.fileName;

  std::cout << "input mesh file: " << fName << std::endl;

  //startup gmsh instance
  gmsh::initialize();

  // open mesh file used to make that mesh
  // to ensure data aligns with elt numbering etc
  gmsh::open(fName);

  gmsh::option::setNumber("Mesh.MshFileVersion", 2.2);

  //change element numbering and node numbering to continuous sequence
  //matches call in createMesh fn
  gmsh::model::mesh::renumberNodes();
  gmsh::model::mesh::renumberElements();

  // get names of models - only expecting 1
  std::vector<std::string> modelNames;
  gmsh::model::list(modelNames);

  assert(modelNames.size() == 1);
  const auto modelName = modelNames[0];

  // std::cout << "model name: " << modelName << std::endl;

  std::vector<int> viewTags;
  // add a view tag for each data vector of vectors in allData
  for (auto const & name_and_vec : allResults){
        // add returns the new view tag
      viewTags.push_back(gmsh::view::add(name_and_vec.first));
  }

  // std::cout << "viewTags len " << viewTags.size() << std::endl;
  assert(viewTags.size() > 1);

  //TODO replace hardcoded mediaNames with user input from gui/cli. Currently we just add all media names,
  //But by only adding some we can exclude certain media (e.g. Air) from the .pos file output
  std::vector<std::string> mediaNames;
  for(auto const & group : mesh.mediaMap) {
      mediaNames.emplace_back(group.second);
  }

  constexpr int frame          = 0;
  constexpr auto eltDataToken  = "ElementData";
  constexpr auto time_step     = 0.0;
  bool appendToResults         = false;
  const std::string fNameFolder = fName.substr(0, fName.find_last_of("/\\") + 1);
  const std::string fNameLocal = fName.substr(fNameFolder.length(), fName.length() - fNameFolder.length());
  const std::string outputFile = fNameFolder + egsinpFileN + "+" + fNameLocal + "results.msh";

  std::cout << "output file: " << outputFile << std::endl;

  assert(viewTags.size() == allResults.size());

  //dataIt->first is string, name of data
  //dataIt->second is a vector of doubles, actually the data

  //loop over result vector, adding to mesh file
  auto dataIt = allResults.cbegin();
  for (const auto & vt : viewTags){
    //add view to gmsh
    //@format_data -> returns format needed by gmsh
    auto dataVec = dataIt->second;
    gmsh::view::addModelData(vt, frame, modelName,
                             eltDataToken,
                             mesh.getDataByMedia(mesh.getUnsignedElements(), mediaNames),
                             format_data(mesh.getDataByMedia(dataVec, mediaNames)),
                             time_step);

    //for every thing other than the first view, append to the file
    if (dataIt != allResults.begin()){
      appendToResults = true;
    }

    //for the first, clear the old results out, overwriteResults = true
    gmsh::view::write(vt, outputFile, appendToResults);

    //move to next data
    dataIt++;
  }

  auto end = std::chrono::steady_clock::now();
  std::cout << "Output data time (milliseconds): " <<
  std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << std::endl;

	//kill gmsh instance
	gmsh::finalize();
  return 0;
}

// patch converter function after gmsh switched to size_t for tags
auto checked_tag_convert = [](std::size_t tag) -> int {
    if (tag > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        throw std::runtime_error("size_t tag too large for int");
    }
    return static_cast<int>(tag);
};

//given a path to a mesh file, this function loads it into program memory
//as a data structure and does the parsing of physical groups etc. into
//a Mesh object, which MevEGS can understand.
Mesh createMesh(std::string fileName){

	//initialize Gmsh
	gmsh::initialize();

	// open mesh file
	// gmsh::open("pGroupBug.msh");
	gmsh::open(fileName);

  //check if file actually opened
  std::vector<std::string> models;
  gmsh::model::list(models);
  // assert (models.size() == 1);

  if (models[0] == std::string{""}){
    std::cerr << "gmsh couldn't read mesh file, exiting\n";
    exit(1);
  }

  //change element numbering and node numbering to continuous sequence
  gmsh::model::mesh::renumberNodes();
  gmsh::model::mesh::renumberElements();

	// list of physical groups
	// the physical group tag can be used to get the name
	gmsh::vectorpair physGroups; // first entry dim = 3, second is physical group tag
	gmsh::model::getPhysicalGroups(physGroups);
	std::cout << "physical groups length: " << physGroups.size() << std::endl;

	std::vector<std::string> physNames;

	//for each physical group, store the std::string name in a dictionary
	std::map<int, std::string> mediamap;

  //map noncontinuous GMSH physical groups to continuous numbers so that EGS plays nice.
  std::map<int, int> medIndConvert;
  int currentInd = 0;

	// std::vectors to be filled with all nodes, elements, media, relative densities
	std::vector<int> allNodes, allElts, allMedia;
  std::vector<double> allRhor;

	//This loop sets up the media name std::map and also the elts, nodes, and media lists
	//TODO try and optimize with reserving space ^^ beforehand
	//careful with [] -> if doesn't exist will overwrite it

	//get elements by type only - only care about tets! -TET type = 4
	std::vector<int> tet_type(1, 4);

	std::string currPhysString;
	for (auto const & pg : physGroups){
    auto dimension = pg.first;
    auto tag = pg.second;
    auto zeroIdxTag = tag - 1;

		std::cout << "dimension " << dimension << std::endl;
		std::cout << "tag " << tag << std::endl;
		gmsh::model::getPhysicalName(dimension, tag, currPhysString);

    //JBT -> physical group name data parser
    //parsing the data from the physical group name into
    //the various input data that comes from it
    //might seem like overkill right now, but the idea
    //was to implement something robust so that if we want
    //to specify more in group names later (such as ECUT or PCUT)
    //then it'll be quite easy to just add a single line of code
    //to additionally parse that input. Should also handle garbage
    //input okay, just in case

    //just change this if we decide we hate semicolons
    std::string delimiter = ";";

    //loop variables
    size_t strpos = 0;
    std::string token;
    std::vector<std::string> segments;

    //split the input and populate a vector with it
    while((strpos = currPhysString.find(delimiter)) != std::string::npos) {

      token = currPhysString.substr(0,strpos);
      segments.emplace_back(token);
      currPhysString.erase(0, strpos + delimiter.length());
    }

    segments.emplace_back(currPhysString);
    //the physical group name should be the first thing
    std::string currPhysName = segments[0];

    const std::string equality = "=";
    float rhor = 1.0;

    if(segments.size() > 1) {
      std::cout << "\tLoading additional physical group properties:" << std::endl;
    }

    //parse the rest of the segments
    for(std::size_t i = 1; i < segments.size(); i++) {

      std::string key = segments[i].substr(0, segments[i].find(equality));
      std::string value = segments[i].substr(segments[i].find(equality) + equality.length(), segments[i].length()-1);
      double valdouble = std::stod(value);
      std::cout << "\t\t" << key << ": " << valdouble << std::endl;

      //make the key all uppercase
      for (auto & c: key) c = toupper(c);
      //if only c++ supported string switch statements :/
      if(key.compare(std::string("RHOR")) == 0) {
        rhor = valdouble;
      }
    }

    bool alreadyExists = false;
    int physOldIndex = -1;

    for(std::size_t i = 0; i < physNames.size(); i++) {
      if(currPhysName.compare(physNames[i]) == 0) {
        physOldIndex = i + 1;
      }
    }

    if(physOldIndex != -1) {
      alreadyExists = true;
      std::cout << currPhysName << " already exists at index " << medIndConvert[physOldIndex] << std::endl;
    }
    else {
      physNames.emplace_back(currPhysName);
      std::cout << "added " << currPhysName << " " << std::endl;
      std::cout << "physical name: " << currPhysName << std::endl;
      medIndConvert[zeroIdxTag] = currentInd;
      currentInd++;
    }

		if(!alreadyExists) mediamap[medIndConvert[zeroIdxTag]] = currPhysName;

		//add entities into allEntities vec
		std::vector<int> currEntityTags;
		gmsh::model::getEntitiesForPhysicalGroup(dimension, tag, currEntityTags);

		// loop over entities of this pgroup and get their elements
		// @arg 3 -> only care about 3d elts
		// tet_type is 4
		std::vector<int> elTags, ndTags; // reset each time
		for (auto const & ceTag : currEntityTags){
			std::cout << "Current Entity Tag: " << ceTag << std::endl;
      //port to new gmsh 4.0 API
      std::vector<std::vector<std::size_t>> dummyElTags, dummyNdTags;
      gmsh::model::mesh::getElements(tet_type, dummyElTags, dummyNdTags, 3, ceTag);


      //always want the first vector of vectors
      std::transform(dummyElTags.front().begin(),
                     dummyElTags.front().end(),
                     std::back_inserter(elTags),
                     checked_tag_convert);

      std::transform(dummyNdTags.front().begin(),
                     dummyNdTags.front().end(),
                     std::back_inserter(ndTags),
                     checked_tag_convert);


      // elTags = dummyElTags.front();
      // ndTags = dummyNdTags.front();

      std::cout << "num elts added" << elTags.size() << std::endl;
			//gmsh::model::mesh::getElementsByType(tet_type, elTags, ndTags, 3, ceTag);
			//initialize media to std::vector same size of elTags and to the value of the media
      std::vector<int> medTags(elTags.size(), 0);

      if(alreadyExists) {
        std::fill(medTags.begin(), medTags.end(), (medIndConvert[physOldIndex - 1]));
      }
      else {
        std::fill(medTags.begin(), medTags.end(), (medIndConvert[zeroIdxTag]));
      }

      //initialize relative density same size as elTags and to the value of rhor
      std::vector<double> rhorTags(elTags.size(), rhor);

      //add current entity's values to the vectors of all values
			allNodes.insert(allNodes.end(), ndTags.begin(), ndTags.end());
			allElts.insert(allElts.end(), elTags.begin(), elTags.end());
			allMedia.insert(allMedia.end(), medTags.begin(), medTags.end());
      allRhor.insert(allRhor.end(), rhorTags.begin(), rhorTags.end());
		}
	}

	std::cout << "num unique elts = " << allElts.size() << " and num nodes =" << allNodes.size() << std::endl;

  //basic element data extracted from gmsh
  struct baseElt {
    int media;
    double rhor;
    std::array<int, 4> fourTetNodes;
  };

  //map is self ordering :), each insertion results in ordered container
  //can get sorted by using range for on keys
  //key is elt number, value is the data of that elt
  std::map<int, baseElt> basicElts;

  //temporary array for each elt's nodes
  std::array<int, 4> nodesOfElt;

	for (size_t i = 0; i < allElts.size(); ++i){

    nodesOfElt[0] = allNodes[i * 4 + 0];
    nodesOfElt[1] = allNodes[i * 4 + 1];
    nodesOfElt[2] = allNodes[i * 4 + 2];
    nodesOfElt[3] = allNodes[i * 4 + 3];

    basicElts[allElts[i]] = baseElt{allMedia[i], allRhor[i], nodesOfElt};
	}

	std::vector<int> sortedElts, nodesOfSortedElts, mediaOfSortedElts;
  std::vector<double> rhorOfSortedElts;

	sortedElts.reserve(allElts.size());
  mediaOfSortedElts.reserve(allElts.size());
	nodesOfSortedElts.reserve(allNodes.size());
  rhorOfSortedElts.reserve(allElts.size());

  auto mask = basicElts.begin()->first - 1;
  for (const auto & mapped_elt : basicElts){
    auto currentEltNum = mapped_elt.first;
    auto currentElt = mapped_elt.second;

    sortedElts.emplace_back(currentEltNum - mask);
    mediaOfSortedElts.emplace_back(currentElt.media);
    rhorOfSortedElts.emplace_back(currentElt.rhor);
    //add four nodes of the current element at the end of the vector
    nodesOfSortedElts.insert(nodesOfSortedElts.end(),
      currentElt.fourTetNodes.begin(), currentElt.fourTetNodes.end());
  }

  //DEBUG
	assert(nodesOfSortedElts.size() == allNodes.size());
	assert(sortedElts.size() == allElts.size());

	//get number of unique nodes
	//@ arg paraCoord -> not used rn
	std::vector<std::size_t> unique_nodes;
	std::vector<double> coords, paraCoord; // matchedCoords matched coords is one to one std::vector for each
	gmsh::model::mesh::getNodes(unique_nodes, coords, paraCoord);
	auto num_unique_nodes = unique_nodes.size();

	// nodes is guaranteed to be w/o duplicates,
	// so we can safely use [] with coordmap
	std::map<int, std::tuple<double, double, double>> coordmap;

	//coords are ordered corresponding to nodes, themselves ordered by 1,2,3,....
	//   // therefore for a nodeTag at index i, the corresponding coordinate
	//   // values are at coords[3i], coords[3i+1], coords[3i+2], e.g.
	//   // [1,2]          //nodeTags
	//   // [0,1] 		     //indices of nodeTags
	//   // [x,y,z,x,y,z]  //coords
	//   // [0,1,2,3,4,5]  //coords array indices we want to calculate
	for (std::size_t ni = 0; ni < num_unique_nodes; ++ni){
		int nd = checked_tag_convert(unique_nodes[ni]);
		coordmap[nd] = std::make_tuple(	coords[3*ni+0],   // x
								                    coords[3*ni+1],   // y
								                    coords[3*ni+2]);  // z
  }

	// dumb init -> changed within neighbours.h
	std::vector<int> eltNeighbours(1);
	//smart init - roughly this size - TODO get right size from the start
	std::vector<int> indices(num_unique_nodes + 1);

	//eltMask is used because our neighbours algo needs elts w/o skips
	//and is expecting a std::vector
	// eltMask -> [1, 2, 3, 4, 5,  6]
	// elts    -> [2, 5, 6, 7, 29, 59]
	// the elt corresponding to eltMask is elts[i] for loop starting from i = 0
	std::vector<int> eltMask;
	eltMask.reserve(sortedElts.size());
	//kinda dumb - std::vector with value of idx + 1;
	//compiler can likely optimize out
	for (size_t j = 0; j < sortedElts.size(); ++j){
		eltMask.emplace_back(j + 1);
	}

	elts_around_elts(eltMask, nodesNoSkip, eltNeighbours, indices);

	Mesh mesh(fileName, nodesOfSortedElts, coordmap, sortedElts,
		eltNeighbours, mediaOfSortedElts, mediamap, rhorOfSortedElts);

  //DEBUG
  std::cout << "MEDIA MAP:" << std::endl;
  for(auto key_str : mediamap) {
    std::cout << key_str.first << ": " << key_str.second << std::endl;
  }

  //set name of the mesh TODO put inside?
  //mesh.setFileName(fileName);

	//end gmsh run
	gmsh::finalize();

	return mesh;
  }
} // gmsh_manip

#endif
