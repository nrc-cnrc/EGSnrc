////////////////////////////////////////////////////////////////////////////////
// MevEGS - (C) 2018 Mevex Corp.
//
// Code for MevEGS's interactions with the gmsh API.
// The design idea here is for this file to do *all* of the interacting with
// gmsh, and for any and all other files to only interact with this file.
// This should keep messiness with dependencies localized.
////////////////////////////////////////////////////////////////////////////////

#ifndef GMSH_MANIP
#define GMSH_MANIP

#include <map>
#include <chrono>

#include "gmsh.h"
#include "dosemath.h"
#include "../../egs++/geometry/egs_mesh/Mesh.h"
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

//generates a gmsh options file from a contents string and a file name
inline void makeMeshOptFile(const std::string& outputMeshFile, const std::string& contents){
  constexpr auto fileExt = ".opt";
  std::ofstream ofs(outputMeshFile + fileExt);
  if (ofs){
    ofs << contents;
    ofs.close();
  }
}

// assume only called inside initialize -> finalize section
std::string turnOffViewString(int viewNum){
  const int notVisible = 0;
  return "View[" + std::to_string(viewNum) + "].Visible = " + std::to_string(notVisible) + ";\n";
}

//tet centroid calculator
//assume xyz is 12 long, x y z x y z x y z x y z
inline std::vector<double> tetCentroid(std::vector<double> xyz){
  assert(xyz.size() == 12);
  std::vector<double> centroid;
  for (std::size_t i; i < 3; ++i){
    centroid.push_back(0.25*(xyz[i] + xyz[i+3] + xyz[i+6] + xyz[i+9]));
  }
  return centroid;
}

//paste relevant egslog data into mesh file using $EGSInfo / $EndEGSInfo delimiters
//NB can be called anything...
//gmsh ignores any $X $EndX pairs that it doesn't know
//we assume here that the commentBlob is formatted without an extra line break at the end
void addCommentSection(const std::string& meshFileName, const std::string& commentBlob){
  constexpr auto startComment = "$EGSInfo";
  constexpr auto endComment = "$EndEGSInfo";

  //start building up our new file contents
  std::stringstream fileBuffer;
  //add comments section
  fileBuffer << startComment << std::endl;
  fileBuffer << commentBlob << std::endl;
  fileBuffer << endComment << std::endl;

  std::ofstream meshFileOut(meshFileName, std::ios::out | std::ios::app);
  //if open worked...
  if (meshFileOut){
      meshFileOut << fileBuffer.str();
      meshFileOut.close();
  }
  else {
    throw std::runtime_error("couldn't open mesh file to add comments");
  }
}

//save a mesh object to a pos or .msh file
//return 0 if good
int saveMeshOutput(const Mesh& mesh,
                   const dosemath::namedResults& allResults,
                   const std::string& egsinpFileN,
                   const std::string& comments){

  auto begin = std::chrono::steady_clock::now();
  auto fName = mesh.getFileName();

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
  for(auto const & group : mesh.getMediaMap()) {
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


  std::string optFileContents; //for turning off all views except dose per C
  constexpr auto dpcName = "Dose per Coulomb [kGy/C]";

  //loop over result vector, adding to mesh file
  auto dataIt = allResults.cbegin();
  for (const auto & vt : viewTags){
    //add view to gmsh
    //@format_data -> returns format needed by gmsh
    auto dataVec = dataIt->second;
    gmsh::view::addModelData(vt, frame, modelName,
                             eltDataToken,
                             mesh.getDataByMedia(mesh.getElements(), mediaNames),
                             format_data(mesh.getDataByMedia(dataVec, mediaNames)),
                             time_step);

    //for every thing other than the first view, append to the file
    if (dataIt != allResults.begin()){
      appendToResults = true;
    }

    //for the first, clear the old results out, overwriteResults = true
    gmsh::view::write(vt, outputFile, appendToResults);

    if (dataIt->first != dpcName){
      optFileContents += turnOffViewString(vt);
    }
    //move to next data
    dataIt++;
  }

  ////DEBUG add k means data

  //int ktag = gmsh::view::add("KMEANS");
  //std::vector<int> clusters;
  //std::vector<double> means;
  //kmeans::mesh_kmeans(clusters, means, mesh);
  //std::cout << "gmsh_manip: adding kmeans data" << std::endl;
  //std::cout << "gmsh_manip: mesh.getElements() size: " << mesh.getDataByMedia(mesh.getElements(), mediaNames).size() << std::endl;
  //gmsh::view::addModelData(ktag, 0, modelName, eltDataToken,
  //  mesh.getDataByMedia(mesh.getElements(), mediaNames),
  //  format_data(std::vector<double>(clusters.begin(), clusters.end())),
  //  time_step);
  //std::cout << "gmsh_manip: writing kmeans data" << std::endl;
  //gmsh::view::write(ktag, outputFile, true);

  //\DEBUG

  //add egs run data to mesh file in $EGSinfo section
  // addCommentSection(outputFile, comments);

  //create .opt file so only view 5 is visible
  makeMeshOptFile(outputFile, optFileContents);

  auto end = std::chrono::steady_clock::now();
  std::cout << "Output data time (milliseconds): " <<
  std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << std::endl;

	//kill gmsh instance
	gmsh::finalize();
  return 0;
}

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
    std::cout << "mesh file doesn't exist" << std::endl;
    return Mesh();
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
      std::vector<std::vector<int>> dummyElTags, dummyNdTags;
      gmsh::model::mesh::getElements(tet_type, dummyElTags, dummyNdTags, 3, ceTag);
      //always want the first vector of vectors
      elTags = dummyElTags.front();
      ndTags = dummyNdTags.front();

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
	std::vector<int> unique_nodes;
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
		int nd = unique_nodes[ni];
		coordmap[nd] = std::make_tuple(	coords[3*ni+0],   // x
								                    coords[3*ni+1],   // y
								                    coords[3*ni+2]);  // z
  }

	// dumb init -> changed within neighbours.h
	std::vector<int> eltNeighbours(1);
	//smart init - roughly this size - TODO get right size from the start
	std::vector<int> indices(num_unique_nodes + 1);

	std::map<int, int> adjustment = renumberNodes(nodesOfSortedElts);
	std::vector<int> nodesNoSkip(nodesOfSortedElts);

	for (auto & nns : nodesNoSkip){
		nns = nns - adjustment[nns];
	}

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

	Mesh mesh(nodesOfSortedElts, coordmap, sortedElts,
		eltNeighbours, mediaOfSortedElts, mediamap, rhorOfSortedElts);

  //DEBUG
  std::cout << "MEDIA MAP:" << std::endl;
  for(auto key_str : mediamap) {
    std::cout << key_str.first << ": " << key_str.second << std::endl;
  }

  //set name of the mesh TODO put inside?
  mesh.setFileName(fileName);

	//end gmsh run
	gmsh::finalize();

	return mesh;
  }
} // gmsh_manip

#endif
