/*
###############################################################################
#
#  EGSnrc egs++ mesh geometry library headers.
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
*/


/*! \file egs_Mevex_tet_collection.h
 *  \brief A tet collection geometry: header
 *  \MJR
 */

#ifndef EGS_MEVEX_TET_COLLECTION_
#define EGS_MEVEX_TET_COLLECTION_

#include <algorithm>
#include <iostream>
#include "egs_base_geometry.h"
#include "Mesh.h"

#ifdef WIN32

    #ifdef BUILD_MEVEX_TET_COLLECTION_DLL
        #define EGS_MEVEX_TET_COLLECTION_EXPORT __declspec(dllexport)
    #else
        #define EGS_MEVEX_TET_COLLECTION_EXPORT __declspec(dllimport)
    #endif
    #define EGS_MEVEX_TET_COLLECTION_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_MEVEX_TET_COLLECTION_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_MEVEX_TET_COLLECTION_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_MEVEX_TET_COLLECTION_EXPORT
        #define EGS_MEVEX_TET_COLLECTION_LOCAL
    #endif

#endif

/*! \brief A tet collection geometry

  \ingroup Geometry
  \ingroup ElementaryG

The EGS_Mevex_tet_collection class implements a tet collection geometry.
Strictly speaking a tet is not a primitive geometry because
its methods could be implemented using the methods of sets of planes.
However, due to its frequent use, slightly faster geometry methods
using a direct implementation, and simpler definition, it is provided
as a separate elementary geometry.

This implementation of a collection of tets makes use of a conventional EGS
input file to define the sources, run control, and other standard EGS input
parameters.  Tetrahedron geometry is initiated within the egsinp file but
refers to an external text files containing N lines of numbers to define N tets.
Materials for the tets are defined in the egsinp file as described below,
and the external text file contains a key on each tet line to describe which
material to apply to each tet.

A tet collection data file is defined using the following
descriptive key:

tet data file = 12 float numbers defining the tet nodes
				4 int numbers defining the neighbour tet number (or -1 for outside)
				1 boolean (0 or 1) number to flag if it is a boundary tet (ie contains
				  at least one outside face)
				1 int number to define the material
All arranged in a single line.  An example will be shown below.

The tet points are contained in an external file referenced by the "tet data file" key
in the egsinp file.

The tet data file path must be fully defined to use egs_view, or the data file must be in the
same directory as the egs_view executable, or the data file may not open properly.

The tet data file may be named as the user wishes providing the name and extension are
included in the geometry input file.

The tet data file format is strictly numbers, no headers, as follows:
\verbatim
0 0 0 10.0 0 0 0 10.0 0 0 5.0 10.0 -1 1 -1 -1 1 1
0 0 0 -10.0 0 0 0 10.0 0 0 5.0 10.0 -1 0 -1 -1 1 0
\endverbatim

The above input file describes two tetrahedrons per this descriptive text:
Tet 0 Nodes:
(0,0,0) (10.0,0,0) (0,10.0,0) (0,5.0,10.0) This defines the X, Y, Z coordinates of tets 1, 2, 3 and 4.
Tet 0 Neighbours:
(-1,1,-1,-1) This indicates faces 1, 3 and 4 are outside faces and face 2 is shared with
neighbour Tet 1.  By convention, face 1 is opposite node 1, face 2 is opposite node 2, etc.
Tet 0 Boundary:
(1) This tet has external faces.  Set to 0 if the tet is strictly internal to the tet collection.
This flag is used to optimize searching for intersection points when a particle is entering
the collection from the outside.
(1) This tet is composed of material 1 from a list of 0, 1, 2... n materials described in the
input file format below.

The above input file describes two tetrahedrons per this descriptive text:
Tet 1 Nodes:
(0,0,0) (-10.0,0,0) (0,10.0,0) (0,5.0,10.0) This defines the X, Y, Z coordinates of tets 1, 2, 3 and 4.
Tet 0 Neighbours:
(-1,0,-1,-1) This indicates faces 1, 3 and 4 are outside faces and face 2 is shared with
neighbour Tet 0.
Tet 0 Boundary:
(1) This tet has external faces.  Set to 0 if the tet is strictly internal to the tet collection.
This flag is used to optimize searching for intersection points when a particle is entering
the collection from the outside.
(0) This tet is composed of material 1 from a list of 0, 1, 2... n materials described in the
input file format below.


A simple example:
\verbatim
:start geometry definition:
    :start geometry:
        library = egs_Mevex_tet_collection
        nTets = 2 #total number of tetrahedrons in the collection
        tet data file = C:\EGS\HEN_HOUSE\egs++\geometry\egs_Mevex_tet_collection\tmp.txt #path to the
        							#external file containing the tet data lines
        name = myTets #User input
        :start media input:
            media = Water512 Air512 #Material names from pegs4dat file arranged by material 0, 1, .. n materials
        :stop media input:
    :stop geometry:
    simulation geometry = myTets
:stop geometry definition:
\endverbatim
*/




// MJR customized class definition
class EGS_MEVEX_TET_COLLECTION_EXPORT EGS_Mevex_tet_collection : public EGS_BaseGeometry {

    //EGS_Vector          *aN1, *aN2, *aN3, *aN4; // Node 1 (x,y,z), Node 2 (x,y,z), ...
    std::vector<EGS_Vector> aN1, aN2, aN3, aN4;
    //EGS_Vector          *aN1N2, *aN1N3, *aN1N4, *aN2N3, *aN2N4, *aN3N4; // Node 1 -> Node 2 vector, ...
    std::vector<EGS_Vector> aN1N2, aN1N3, aN1N4, aN2N3, aN2N4, aN3N4;
    //EGS_Vector          *aNorm1, *aNorm2, *aNorm3, *aNorm4; // Normal of plane without node 1, Normal of plane without node 2, ...
    std::vector<EGS_Vector> aNorm1, aNorm2, aNorm3, aNorm4;

    //int 				*n1, *n2, *n3, *n4; // Neighbours of the tet; n1 is the face 1 (missing N1) neighbour, etc.
    //bool				*BoundaryTet; //Indicates if the tet contains at least 1 face external to the collection.
    //int 	 			*mediaIndices; //Indicates which media from the input file to apply to the tet.

    std::vector<int> n1, n2, n3, n4;
    std::vector<bool> BoundaryTet;
    std::vector<int> mediaIndices;

    int 				areg; //current region of the particle

    string       type = "EGS_Mevex_tet_collection";

public:

  EGS_Mevex_tet_collection(const Mesh& mesh, const double scalingFactor = 10, const string &Name = "Mesh_tet_collection") : EGS_BaseGeometry(Name) {

    // call mesh methods to initialize class member data
    //get all of the x, y, z coordinate values for every point in the mesh object
    std::vector<double> coordinates;
    coordinates = mesh.getCoords();

    std::vector<std::size_t> neighbours;
    neighbours = mesh.getNeighbours();

    std::vector<std::size_t> mediaType;
    mediaType = mesh.getMedia();

    std::vector<int> isBoundaryTet;
    isBoundaryTet = mesh.getBoundaryTet();

    int nt = mediaType.size();

    EGS_BaseGeometry::nreg = nt; 							//set number of regions to equal number of tets nt passed in.

    //TODO change over to vectors
    // Create arrays for each attribute
    // aN1 = new EGS_Vector [nt]; 		//Node 1 Vector array, will contain aN1.x, aN1.y, aN1.z for each tet
    // aN2 = new EGS_Vector [nt];
    // aN3 = new EGS_Vector [nt];
    // aN4 = new EGS_Vector [nt];

    aN1.reserve(nt);
    aN2.reserve(nt);
    aN3.reserve(nt);
    aN4.reserve(nt);

    // aN1N3 = new EGS_Vector [nt];
    // aN1N2 = new EGS_Vector [nt]; 	//Vectors from N1 to N2, etc.
    // aN1N4 = new EGS_Vector [nt];
    // aN2N3 = new EGS_Vector [nt];
    // aN2N4 = new EGS_Vector [nt];
    // aN3N4 = new EGS_Vector [nt];

    aN1N2.reserve(nt);
    aN1N3.reserve(nt);
    aN1N4.reserve(nt);
    aN2N3.reserve(nt);
    aN2N4.reserve(nt);
    aN3N4.reserve(nt);


    // aNorm1 = new EGS_Vector [nt]; 	//Normal of face 1 (opposite Node 1)
    // aNorm2 = new EGS_Vector [nt];
    // aNorm3 = new EGS_Vector [nt];
    // aNorm4 = new EGS_Vector [nt];

    aNorm1.reserve(nt);
    aNorm2.reserve(nt);
    aNorm3.reserve(nt);
    aNorm4.reserve(nt);

    // n1 = new int [nt];  			//Neighbour of face 1 (opposite Node 1)
    // n2 = new int [nt];
    // n3 = new int [nt];
    // n4 = new int [nt];

    n1.reserve(nt);
    n2.reserve(nt);
    n3.reserve(nt);
    n4.reserve(nt);

    // BoundaryTet = new bool [nt]; 	//Variable to hold boundary tet flag.
    // mediaIndices = new int [nt]; 	//Variable to hold media index.

    BoundaryTet.reserve(nt);
    mediaIndices.reserve(nt);

    	//loop through each tet
  	for (int i = 0; i < nt; ++i){
      int coordIdx = i*12;

      aN1[i].x = coordinates[coordIdx+0] / scalingFactor;
      aN1[i].y = coordinates[coordIdx+1] / scalingFactor;
      aN1[i].z = coordinates[coordIdx+2] / scalingFactor;
      aN2[i].x = coordinates[coordIdx+3] / scalingFactor;
      aN2[i].y = coordinates[coordIdx+4] / scalingFactor;
      aN2[i].z = coordinates[coordIdx+5] / scalingFactor;
      aN3[i].x = coordinates[coordIdx+6] / scalingFactor;
      aN3[i].y = coordinates[coordIdx+7] / scalingFactor;
      aN3[i].z = coordinates[coordIdx+8] / scalingFactor;
      aN4[i].x = coordinates[coordIdx+9] / scalingFactor;
      aN4[i].y = coordinates[coordIdx+10] / scalingFactor;
      aN4[i].z = coordinates[coordIdx+11] / scalingFactor;

      int nIdx = i*4;

      (neighbours[nIdx]   == -1)?   n1[i] = neighbours[nIdx+0]   : n1[i] = neighbours[nIdx+0] - 1;
      (neighbours[nIdx+1] == -1)?   n2[i] = neighbours[nIdx+1]   : n2[i] = neighbours[nIdx+1] - 1;
      (neighbours[nIdx+2] == -1)?   n3[i] = neighbours[nIdx+2]   : n3[i] = neighbours[nIdx+2] - 1;
      (neighbours[nIdx+3] == -1)?   n4[i] = neighbours[nIdx+3]   : n4[i] = neighbours[nIdx+3] - 1;



      // n1[i] = neighbours[nIdx+0] - 1;
      // n2[i] = neighbours[nIdx+1] - 1;
      // n3[i] = neighbours[nIdx+2] - 1;
      // n4[i] = neighbours[nIdx+3] - 1;

      //same indices for these guys
      BoundaryTet[i]  = isBoundaryTet[i];
      mediaIndices[i] = mediaType[i];

    	// Calculate attribute values for vectors between nodes and normals
		// Use (b - a) to calculate vector from a to b
  		// aN1N2 - from Node 1 to Node 2, etc.
  		aN1N2[i] = aN2[i] - aN1[i];  
  		aN1N3[i] = aN3[i] - aN1[i];  
  		aN1N4[i] = aN4[i] - aN1[i]; 
  		aN2N3[i] = aN3[i] - aN2[i];
  		aN2N4[i] = aN4[i] - aN2[i]; 
  		aN3N4[i] = aN4[i] - aN3[i]; 

  		// Compute normals of each face of the tet
  		// Faces omit node with same number as face

  		aNorm1[i] = aN2N3[i].times(aN2N4[i]); //Calculate vector products of the vectors between nodes to determine the normals to the faces.
  		aNorm2[i] = aN1N3[i].times(aN1N4[i]);
  		aNorm3[i] = aN1N2[i].times(aN1N4[i]);
  		aNorm4[i] = aN1N2[i].times(aN1N3[i]);

  		// Check direction of normals is within 90 deg of vector to remaining node (inward)
  		// (dot product +ve otherwise multiply normal by -1)
  		// Face 1 - does not include node 1
  		// normal should point in same direction as vector from any node in the plane to node 1
  		if ((aNorm1[i] * aN1N2[i]) > 0) { // aN1N2 is from Node 1 to Node 2 - checking if the normal points in
  			// the direction of 1 to 2 would be an outward normal so change it
  			aNorm1[i].x = -1 * aNorm1[i].x;
  			aNorm1[i].y = -1 * aNorm1[i].y;
  			aNorm1[i].z = -1 * aNorm1[i].z;
  		}
  		// Face 2
  		if ((aNorm2[i] * aN1N2[i]) < 0) { // In this case aN1N2 is the direction we want the normal to point
  			// NB Face 1 had ">"; while for face 2 we have "<"
  			aNorm2[i].x = -1 * aNorm2[i].x;
  			aNorm2[i].y = -1 * aNorm2[i].y;
  			aNorm2[i].z = -1 * aNorm2[i].z;
  		}
  		// Face 3
  		if ((aNorm3[i] * aN1N3[i]) < 0) {
  			aNorm3[i].x = -1 * aNorm3[i].x;
  			aNorm3[i].y = -1 * aNorm3[i].y;
  			aNorm3[i].z = -1 * aNorm3[i].z;
  		}
  		// Face 4
  		if ((aNorm4[i] * aN1N4[i]) < 0) {
  			aNorm4[i].x = -1 * aNorm4[i].x;
  			aNorm4[i].y = -1 * aNorm4[i].y;
  			aNorm4[i].z = -1 * aNorm4[i].z;
  		}

  		// Unitize Normal vectors
  		(aNorm1[i]).normalize();
  		(aNorm2[i]).normalize();
  		(aNorm3[i]).normalize();
  		(aNorm4[i]).normalize();

    	}
    }

  // MJR customized
  // class constructor
  //EGS_Mevex_tet_collection(number of tets, Node1 x, y, z; Node2 x, y, z; Node3 x, y, z; Node4 x, y, z; Boundary tet; media index )
  EGS_Mevex_tet_collection(int nt, const EGS_Float *X1, const EGS_Float *Y1, const EGS_Float *Z1, const EGS_Float *X2, const EGS_Float *Y2, const EGS_Float *Z2, const EGS_Float *X3, const EGS_Float *Y3, const EGS_Float *Z3, const EGS_Float *X4, const EGS_Float *Y4, const EGS_Float *Z4,
    const int *N1, const int *N2, const int *N3, const int *N4, const bool *bT, const int *mediaIndexes,
    const string &Name = "") : EGS_BaseGeometry(Name) {


      if (nt > 0) { //If we have tets to build

        //TODO change over to vectors
        // Create arrays for each attribute
        // aN1 = new EGS_Vector [nt]; 		//Node 1 Vector array, will contain aN1.x, aN1.y, aN1.z for each tet
        // aN2 = new EGS_Vector [nt];
        // aN3 = new EGS_Vector [nt];
        // aN4 = new EGS_Vector [nt];

        aN1.reserve(nt);
        aN2.reserve(nt);
        aN3.reserve(nt);
        aN4.reserve(nt);


        // aN1N3 = new EGS_Vector [nt];
        // aN1N2 = new EGS_Vector [nt]; 	//Vectors from N1 to N2, etc.
        // aN1N4 = new EGS_Vector [nt];
        // aN2N3 = new EGS_Vector [nt];
        // aN2N4 = new EGS_Vector [nt];
        // aN3N4 = new EGS_Vector [nt];

        aN1N2.reserve(nt);
        aN1N3.reserve(nt);
        aN1N4.reserve(nt);
        aN2N3.reserve(nt);
        aN2N4.reserve(nt);
        aN3N4.reserve(nt);


        // aNorm1 = new EGS_Vector [nt]; 	//Normal of face 1 (opposite Node 1)
        // aNorm2 = new EGS_Vector [nt];
        // aNorm3 = new EGS_Vector [nt];
        // aNorm4 = new EGS_Vector [nt];

        aNorm1.reserve(nt);
        aNorm2.reserve(nt);
        aNorm3.reserve(nt);
        aNorm4.reserve(nt);

        // n1 = new int [nt];  			//Neighbour of face 1 (opposite Node 1)
        // n2 = new int [nt];
        // n3 = new int [nt];
        // n4 = new int [nt];

        n1.reserve(nt);
        n2.reserve(nt);
        n3.reserve(nt);
        n4.reserve(nt);

        // BoundaryTet = new bool [nt]; 	//Variable to hold boundary tet flag.
        // mediaIndices = new int [nt]; 	//Variable to hold media index.

        BoundaryTet.reserve(nt);
        mediaIndices.reserve(nt);
      }

      nreg = nt; 							//set number of regions to equal number of tets nt passed in.

      for (int i=0; i<nt; i++) {			// Initialize Arrays to values from input
        // egsInformation("\ncoords\n");
        aN1[i].x = X1[i];
        // egsInformation(" %f", X1[i]);
        aN1[i].y = Y1[i];
        // egsInformation(" %f", Y1[i]);
        aN1[i].z = Z1[i];
        // egsInformation(" %f", Z1[i]);
        aN2[i].x = X2[i];
        // egsInformation(" %f", X2[i]);
        aN2[i].y = Y2[i];
        // egsInformation(" %f", Y2[i]);
        aN2[i].z = Z2[i];
        // egsInformation(" %f", Z2[i]);
        aN3[i].x = X3[i];
        // egsInformation(" %f", X3[i]);
        aN3[i].y = Y3[i];
        // egsInformation(" %f", Y3[i]);
        aN3[i].z = Z3[i];
        // egsInformation(" %f", Z3[i]);
        aN4[i].x = X4[i];
        // egsInformation(" %f", X4[i]);
        aN4[i].y = Y4[i];
        // egsInformation(" %f", Y4[i]);
        aN4[i].z = Z4[i];
        // egsInformation(" %f", Z4[i]);

        // egsInformation("\nNeighbours\n");

        n1[i] = N1[i];
        // egsInformation(" %d", N1[i]);
        n2[i] = N2[i];
        // egsInformation(" %d", N2[i]);
        n3[i] = N3[i];
        // egsInformation(" %d", N3[i]);
        n4[i] = N4[i];
        // egsInformation(" %d", N4[i]);

        // egsInformation("\n");

        BoundaryTet[i] = bT[i];
        // egsInformation("%d", bT[i]) ;
        mediaIndices[i] = mediaIndexes[i];
        // egsInformation("%d", mediaIndexes[i]);

        // egsInformation("\n");
        // Calculate attribute values for vectors between nodes and normals
        // Use > to calculate vector from a to b
        // aN1N2 - from Node 1 to Node 2, etc.
  	aN1N2[i] = aN2[i] - aN1[i];  
  	aN1N3[i] = aN3[i] - aN1[i];  
  	aN1N4[i] = aN4[i] - aN1[i]; 
  	aN2N3[i] = aN3[i] - aN2[i];
  	aN2N4[i] = aN4[i] - aN2[i]; 
  	aN3N4[i] = aN4[i] - aN3[i]; 

        // Compute normals of each face of the tet
        // Faces omit node with same number as face
        aNorm1[i] = aN2N3[i].times(aN2N4[i]); //Calculate vector products of the vectors between nodes to determine the normals to the faces.
        aNorm2[i] = aN1N3[i].times(aN1N4[i]);
        aNorm3[i] = aN1N2[i].times(aN1N4[i]);
        aNorm4[i] = aN1N2[i].times(aN1N3[i]);

        // Check direction of normals is within 90 deg of vector to remaining node (inward)
        // (dot product +ve otherwise multiply normal by -1)
        // Face 1 - does not include node 1
        // normal should point in same direction as vector from any node in the plane to node 1
        if ((aNorm1[i]*aN1N2[i]) > 0) { // aN1N2 is from Node 1 to Node 2 - checking if the normal points in
          // the direction of 1 to 2 would be an outward normal so change it
          aNorm1[i].x = -1 * aNorm1[i].x;
          aNorm1[i].y = -1 * aNorm1[i].y;
          aNorm1[i].z = -1 * aNorm1[i].z;
        }
        // Face 2
        if ((aNorm2[i]*aN1N2[i]) < 0) { // In this case aN1N2 is the direction we want the normal to point
          // NB Face 1 had ">"; while for face 2 we have "<"
          aNorm2[i].x = -1 * aNorm2[i].x;
          aNorm2[i].y = -1 * aNorm2[i].y;
          aNorm2[i].z = -1 * aNorm2[i].z;
        }
        // Face 3
        if ((aNorm3[i]*aN1N3[i]) < 0) {
          aNorm3[i].x = -1 * aNorm3[i].x;
          aNorm3[i].y = -1 * aNorm3[i].y;
          aNorm3[i].z = -1 * aNorm3[i].z;
        }
        // Face 4
        if ((aNorm4[i]*aN1N4[i]) < 0) {
          aNorm4[i].x = -1 * aNorm4[i].x;
          aNorm4[i].y = -1 * aNorm4[i].y;
          aNorm4[i].z = -1 * aNorm4[i].z;
        }

        // Unitize Normal vectors
        (aNorm1[i]).normalize();
        (aNorm2[i]).normalize();
        (aNorm3[i]).normalize();
        (aNorm4[i]).normalize();
      }
    };

  ~EGS_Mevex_tet_collection() = default;

  //JBT
  //sets relative density for a specified tet
  void setRelativeRho(int ind, EGS_Float rho) {
      // egsInformation("# Regions: %d, Index: %d\n", nreg, ind);
      // if(ind >= nreg) egsInformation("^^^ ERROR ^^^\n");
      if (ind >= 0 && ind <= nreg) {
          if (!rhor) {
              rhor = new EGS_Float [nreg];
              for (int j=0; j<nreg; j++) {
                  rhor[j] = 1;
              }
          }
          rhor[ind] = rho;
          has_rho_scaling = true;
      }
  }

  const std::vector<int>& getMediaIndices() const {
    return mediaIndices;
  }

    // MJR written
	EGS_Float dist_to_edge(const EGS_Vector &p, const EGS_Vector &n,
                         const EGS_Vector &m, const EGS_Vector &nm) {
		EGS_Vector iP = p;
		EGS_Vector iN = n;
		EGS_Vector iM = m;
		EGS_Vector iNM = nm;
		EGS_Vector iMP;

		EGS_Vector iNP = iP - iN; //Vector from iN to iP

		EGS_Float w1;
		w1 = ((iNP*iNM) / (iNM*iNM));
		EGS_Vector POI;
		POI.x = iN.x + w1 * iNM.x;
		POI.y = iN.y + w1 * iNM.y;
		POI.z = iN.z + w1 * iNM.z;

		  EGS_Float iDist;
		if (w1 < 0) {
			// Projection is outside the endpoints of the edge with Node 3 as the closest
			iDist = iNP.length();
			return iDist;
		}
		if (w1 > 1) {
			// Projection is outside the endpoints of the edge with Node 4 as the closest
			iMP = iP - iM;
			iDist = iMP.length();

			return iDist;
		}
		// Projection is within the endpoints of the edge
		iDist = (POI - iP).length();

		return iDist;
	};

  	// MJR written
  	EGS_Float dist_to_face(const EGS_Vector &p,
                           const EGS_Vector &n,
                           const EGS_Vector &m,
                           const EGS_Vector &o,
                           const EGS_Vector &nm,
                           const EGS_Vector &no,
                           const EGS_Vector &mo,
                           const EGS_Vector &norm) {

      EGS_Vector iP = p;
  		EGS_Vector iN = n;
  		EGS_Vector iM = m;
  		EGS_Vector iO = o;
  		EGS_Vector iNM = nm;
  		EGS_Vector iNO = no;
  		EGS_Vector iMO = mo;
  		EGS_Vector iNorm = norm;
  		EGS_Vector iPN = iN - iP; //Vector from iP to iN
  		EGS_Float iDist;
  		EGS_Vector POI; //Vector to Point of Intersection
  		iDist = iPN*iNorm;
  		POI.x = iDist * iNorm.x + iP.x;
  		POI.y = iDist * iNorm.y + iP.y;
  		POI.z = iDist * iNorm.z + iP.z;

  		EGS_Vector NPOI = POI - iN; //Vector from iN to POI

  		EGS_Float w1;
  		EGS_Float w2;

  		EGS_Float invDenom = 1.0 / ((iNO*iNO) * (iNM*iNM) - (iNM*iNO) * (iNM*iNO));
  		w1 = ((iNM*iNM) * (iNO*NPOI) - (iNM*iNO) * (iNM*NPOI)) * invDenom;
  		w2 = ((iNO*iNO) * (iNM*NPOI) - (iNM*iNO) * (iNO*NPOI)) * invDenom;

  	    // Check if projection lies within triangle - if it does return distance
  	    if ((w1 >= 0) && (w2 >= 0) && ((w1 + w2) <= 1)) {
  	    	// the particle is within the triangle
  	    	return iDist;
  	    }
  	    // If it doesn't - project onto each edge
  	    // and find the shortest distance between the projection and the particle
  		EGS_Float iDist_edge1, iDist_edge2, iDist_edge3;
  		iDist_edge1 = dist_to_edge(iP, iN, iM, iNM);
  		iDist_edge2 = dist_to_edge(iP, iN, iO, iNO);
  		iDist_edge3 = dist_to_edge(iP, iM, iO, iMO);

  		iDist = 1e30;
  		if (iDist_edge1 < iDist) {
  			iDist = iDist_edge1;
  		}
  		if (iDist_edge2 < iDist) {
  			iDist = iDist_edge2;
  		}
  		if (iDist_edge3 < iDist) {
  			iDist = iDist_edge3;
  		}
  		return iDist;
  	};

    // MJR customized
     bool isInside(const EGS_Vector &x) override {

        EGS_Vector aP = x; // the position of the particle
        EGS_Vector aNP; // a vector from a node to the particle

        for (int i=0; i<nreg; i++) {

	        // Face 1:
	        aNP = aP - aN2[i]; //compute vector from node 2 to point

			    // If vector from plane to particle has the same general direction as the inward normal,
        	// the particle is inside the plane and MAY be inside the tet,
        	// otherwise the particle is outside the plane and therefore definitely outside the tet
        	// so skip the remaining checks of other faces to save time.
	        if ((aNorm1[i]*aNP) < 0) {
	        	 continue;
	        }
	        // Face 2:
	        aNP = aP - aN1[i];
	        if ((aNorm2[i]*aNP) < 0) {
	        	continue;
	        }
	        // Face 3:
	        aNP = aP - aN1[i];
	        if ((aNorm3[i]*aNP) < 0) {
	            continue;
	        }
	        // Face 4:
	        aNP = aP - aN1[i];
	        if ((aNorm4[i]*aNP) < 0) {
	            continue;
	        }
	        //If we made it this far the particle is inside this tet, so set class variable areg to our current tet number
	        //and return because we found which tet the particle is inside and don't need to look any further.
	        areg = i;
	        return true;
        }
        // If if we didn't return within the for loop, the particle isn't inside any of the tets in the tet collection.
        return false;

    };

    int isWhere(const EGS_Vector &x) override {
    	//Assume the particle is outside by setting areg to -1 and call isInside() to see
    	//if it is actually inside a tet.  Return class variable areg which will be either
    	//-1 if the particle isn't within a tet, or it will be the tet number if it is inside.
    	  areg = -1;
        isInside(x);
        return areg;
    };

    int inside(const EGS_Vector &x) override {
        return isWhere(x);
    };



    EGS_Float howfarToOutside(int ireg, const EGS_Vector &x,
                              const EGS_Vector &u) override {
    	// Returns the distance from the current position x to the nearest, outer boundary of the tetrahedron collection in the direction u.
    	// An outer boundary is defined as a boundary which has region "-1" as a neighbour.
    	// To determine the distance, howFar(ireg, x, u, t) is used to calculate the distance in the current tet to its boundary and to indicate what the neighbouring tet is.
        EGS_Float stepLength; // The the distance we are checking at a given step;
        // uninitialized so that it can be reset during each iteration
        int nextReg; // The region number of the region that neighbours the tet that was just checked
        // uninitialized because it represents the return value of howFar, which gets called within the loop; at this point any value would have no meaning
        int currentReg = ireg; // current region along the trajectory; when this flips to -1 the outer boundary has been found
        // This also allows the non-const argument to be untouched
        // initialized to ireg so that the calculation is started from the current index
        EGS_Vector currentPosition = x; // current position along the trajectory; needs to be current for each call to howfar
        // this also allows position to be updated despite the argument being a const
        // initialized to x so that the calculation is started from the current position
        EGS_Float distToOutside = 0; // running tally of the distance along the trajectory
        // initialized to 0 so that we calculate the distance from the current point
        // int count = 0; // a counter for the loop; if this value gets too large, stop counting as the outside is VERRRRRY far away...
        // initialized to 0 so we count the number of iterations
        // int countLimit = 1e6; // a limit to keep the number of iterations "low"
        // initialized to 1e6 as a dummy "Large" number
        // while((currentReg > -1) && (count < countLimit)) {
        while (currentReg > -1) {
        	// Keep checking until we either reach the outside (currentReg = -1) or until we have tried too many times (count = countLimit)
			stepLength = 1e30; // A very large trial distance, if the boundary is outside this it is considered to be at infinity
        	nextReg = howfar(currentReg,currentPosition,u,stepLength); // call howFar from the current values
        	// nextReg represents the neighbour on the boundary that is closest to the current location and in the direction of travel
        	// stepLength is expected to be reset to a smaller value if a boundary was found to be closer than the trial value
        	// currentReg, currentPosition, and u are expected to be unchanged
        	currentReg = nextReg; // update currentReg to the value returned from howFar to prepare for the next iteration
        	distToOutside = distToOutside + stepLength; // update distance to outside of tet collection as (previous distance from starting point to tet boundary) + (distance to next tet boundary)
        	currentPosition = currentPosition + stepLength * u; // update position to boundary of next tet
        	// count = count + 1; // increment loop counter
        }

        	return distToOutside;
    };



    //MJR customized
    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t, int *newmed=0, EGS_Vector *normal=0) override {
        EGS_Vector aP = x; // Position of particle
        EGS_Vector aDir = u; // particle direction
        EGS_Vector aDir_unitized; // unitized particle direction
        EGS_Vector aPN; // vector from a particle to node

        // Unitize direction
        aDir_unitized = aDir;
        aDir_unitized.normalize();

        EGS_Float dist = 1e30; // distance from particle to plane of tet face in direction of particle.
        EGS_Vector POI; // Point of intersection between particle trajectory and the plane of the tet face
        EGS_Vector NPOI; // vector from a node (local origin) to point of intersection between particle trajectory and plane of tet face
        EGS_Float w1, w2; // scalars that multiply the "basis vectors" to generate any point in the plane of the tet face

        int inew = ireg; // return value; returning ireg indicates region number will be unchanged, -1 means particle will leave the tet collection, any other integer is the number of the neighbour tet the particle moved into.
        EGS_Vector n; // Normal of the currently closest tet face in the direction of the particle trajectory


        // If the particle is inside the tet when we start, then do these checks.
        if (ireg >= 0) {

        	// If the direction is in the same general direction as the inward normal
        	// the particle will not cross the face,
        	// otherwise it will - calculate the distance between the particle and the face in the direction of travel

            // Face 1
            if ((aDir_unitized*aNorm1[ireg]) < 0) { // < 0 indicates that the trajectory loosely points at the plane
                aPN = aN2[ireg] - aP; //Computer vector from aP to aN2[]
                dist = (aPN*aNorm1[ireg]) / (aDir_unitized*aNorm1[ireg]); //calculated distance to the face.

                if (dist < t) { //If the calculated distance to the face is shorter than the proposed step length
                    t = dist; //Set the step length to the distance to the face.
                    n = aNorm1[ireg]; //Assign the normal of face 1 to n
                    inew = n1[ireg]; //Set the new tet number inew to the neighbour of face 1, n1.
                }
            }

            // Face 2
            dist = 1e30;
            if ((aDir_unitized*aNorm2[ireg]) < 0) {
                aPN = aN1[ireg]- aP;
                dist = (aPN*aNorm2[ireg]) / (aDir_unitized*aNorm2[ireg]);

                if (dist < t) {
                    t = dist;
                    n = aNorm2[ireg];
                    inew = n2[ireg];
                }
            }

            // Face 3
            dist = 1e30;
            if ((aDir_unitized*aNorm3[ireg]) < 0) {
                aPN = aN1[ireg]- aP;
                dist = (aPN*aNorm3[ireg]) / (aDir_unitized*aNorm3[ireg]);
                if (dist < t) {
                    t = dist;
                    n = aNorm3[ireg];
                    inew = n3[ireg];
                }
            }

            // Face 4
            dist = 1e30;
            if ((aDir_unitized*aNorm4[ireg]) < 0) {
                aPN = aN1[ireg]- aP;
                dist = (aPN*aNorm4[ireg]) / (aDir_unitized*aNorm4[ireg]);
                if (dist < t) {
                    t = dist;
                    n = aNorm4[ireg];
                    inew = n4[ireg];
                }
            }

            if (inew != ireg) { // if particle changes regions as a result of the previous code, update things
                if (newmed) { //check if newmed was passed in. (default value is 0, fails this check)
                	if (inew != -1) { //As long as the new region isn't outside,
                    	*newmed = region_media[inew]; //Set newmed to the new media for the new tet.
                    }
                    else {
                    	*newmed = -1; //We're outside, reset newmed to -1.
                    }
                }
                if (normal) { // Check if normal was passed in. (default value is 0, fails this check)
                	n.x = -1 * n.x; //EGS conventional direction for the normal is opposite from tets, so
                	n.y = -1 * n.y; //invert the tet normal to pass it back out.
                	n.z = -1 * n.z;
                    *normal = n;
                }
            }

            // Now that we figured out the region which could be a different region or the same depending
            //on the length of the step t,
            // and updated new med and normal, return the new region.
            return inew;
        }


        // If we skipped the previous IF statement, the particle starts outside tet so assign inew as -1 to start.
        inew = -1;


        //Proceed to check if the particle will intersect an outside face of boundary tets from the outside.
        // Face 1

        for (int i=0; i<nreg; i++) { //Cycle through all tets
        	if (!BoundaryTet[i]) { //If not a boundary tet, don't bother to check for intersection from the outside.
        		continue;
        	}
        	else { //It's a boundary tet, start looking at each face.
        		if (n1[i] == -1) { //Check if face 1 is an outside face (indicated by -1)
        			// check face 1
        			if ((aDir_unitized*aNorm1[i]) > 0) { // Plane faces same general direction as particle (particle has to cross inwards)
	            		aPN = aN2[i] - aP; // Vector from particle to node N2,
	            		dist = (aPN*aNorm1[i]) / (aDir_unitized*aNorm1[i]); // distance along up to plane from aP
		            	if ((dist < t) && (dist >= 0)) { // distance smaller than intended step
		                	POI.x = dist * aDir_unitized.x + aP.x; // Point of Intersection = dist * aDir_unitized + aP
		                	POI.y = dist * aDir_unitized.y + aP.y;
		                	POI.z = dist * aDir_unitized.z + aP.z;
			                // Check if POI is within tet face not just on the plane
			                NPOI = POI - aN2[i];
			                EGS_Float invDenom = 1.0 / ((aN2N4[i]*aN2N4[i]) * (aN2N3[i]*aN2N3[i]) - (aN2N3[i]*aN2N4[i]) * (aN2N3[i]*aN2N4[i]));
							w1 = ((aN2N3[i]*aN2N3[i]) * (aN2N4[i]*NPOI) - (aN2N3[i]*aN2N4[i]) * (aN2N3[i]*NPOI)) * invDenom;
							w2 = ((aN2N4[i]*aN2N4[i]) * (aN2N3[i]*NPOI) - (aN2N3[i]*aN2N4[i]) * (aN2N4[i]*NPOI)) * invDenom;

						    // Check if projection lies within triangle - if it does return distance
			                if ((w1 >= 0) && (w2 >= 0) && ((w1 + w2) <= 1)) {
			                    t = dist; //we've had a successful hit, so update t to dist.
			                    n = aNorm1[i]; //Update normal
			                    inew = i; // Update new region the particle will enter
			                }
			            }
	                }
	            }
        		if (n2[i] == -1) {
        			// check face 2
        			if ((aDir_unitized*aNorm2[i]) > 0) { // Plane faces same general direction as particle (particle has to cross inwards)
				    aPN = aN1[i] - aP; // Vector from particle to node N1,
			            dist = (aPN*aNorm2[i]) / (aDir_unitized*aNorm2[i]); // distance along up to plane from aP

			            if ((dist < t) && (dist >= 0)) { // distance smaller than intended step
			                POI.x = dist * aDir_unitized.x + aP.x; // Point of Intersection = dist * aDir_unitized + aP
			                POI.y = dist * aDir_unitized.y + aP.y;
			                POI.z = dist * aDir_unitized.z + aP.z;
			                // Check if POI is within tet face not just on the plane
			                NPOI = POI - aN1[i];
			                EGS_Float invDenom = 1.0 / ((aN1N4[i]*aN1N4[i]) * (aN1N3[i]*aN1N3[i]) - (aN1N3[i]*aN1N4[i]) * (aN1N3[i]*aN1N4[i]));
							w1 = ((aN1N3[i]*aN1N3[i]) * (aN1N4[i]*NPOI) - (aN1N3[i]*aN1N4[i]) * (aN1N3[i]*NPOI)) * invDenom;
							w2 = ((aN1N4[i]*aN1N4[i]) * (aN1N3[i]*NPOI) - (aN1N3[i]*aN1N4[i]) * (aN1N4[i]*NPOI)) * invDenom;

						    // Check if projection lies within triangle - if it does return distance
			                if ((w1 >= 0) && (w2 >= 0) && ((w1 + w2) <= 1)) {
			                    t = dist;
			                    n = aNorm2[i];
			                    inew = i;
			                }
			            }
			        }
        		}
        		if (n3[i] == -1) {
        			// check face 3
        			if ((aDir_unitized*aNorm3[i]) > 0) { // Plane faces same general direction as particle (particle has to cross inwards)
			            aPN = aN1[i] - aP; // Vector from particle to node
			            dist = (aPN*aNorm3[i]) / (aDir_unitized*aNorm3[i]); // distance along up to plane from aP
			            if ((dist < t) && (dist >= 0)) { // distance smaller than intended step
			                POI.x = dist * aDir_unitized.x + aP.x; // Point of Intersection = dist * aDir_unitized + aP
			                POI.y = dist * aDir_unitized.y + aP.y;
			                POI.z = dist * aDir_unitized.z + aP.z;
			                // Check if POI is within tet face not just on the plane
			                NPOI = POI - aN1[i];
			                EGS_Float invDenom = 1.0 / ((aN1N4[i]*aN1N4[i]) * (aN1N2[i]*aN1N2[i]) - (aN1N2[i]*aN1N4[i]) * (aN1N2[i]*aN1N4[i]));
							w1 = ((aN1N2[i]*aN1N2[i]) * (aN1N4[i]*NPOI) - (aN1N2[i]*aN1N4[i]) * (aN1N2[i]*NPOI)) * invDenom;
							w2 = ((aN1N4[i]*aN1N4[i]) * (aN1N2[i]*NPOI) - (aN1N2[i]*aN1N4[i]) * (aN1N4[i]*NPOI)) * invDenom;
						    // Check if projection lies within triangle - if it does return distance
			                if ((w1 >= 0) && (w2 >= 0) && ((w1 + w2) <= 1)) {
			                    t = dist;
			                    n = aNorm3[i];
			                    inew = i;
			                }
			            }
			        }
        		}
        		if (n4[i] == -1) {
        			// check face 4
        			if ((aDir_unitized*aNorm4[i]) > 0) { // Plane faces same general direction as particle (particle has to cross inwards)
			            aPN = aN1[i] - aP; // Vector from particle to node
			            dist = (aPN*aNorm4[i]) / (aDir_unitized*aNorm4[i]); // distance along up to plane from aP

			            if ((dist < t) && (dist >= 0)) { // distance smaller than intended step
			                POI.x = dist * aDir_unitized.x + aP.x; // Point of Intersection = dist * aDir_unitized + aP
			                POI.y = dist * aDir_unitized.y + aP.y;
			                POI.z = dist * aDir_unitized.z + aP.z;
			                // Check if POI is within tet face not just on the plane
			                NPOI = POI - aN1[i];
			                EGS_Float invDenom = 1.0 / ((aN1N3[i]*aN1N3[i]) * (aN1N2[i]*aN1N2[i]) - (aN1N2[i]*aN1N3[i]) * (aN1N2[i]*aN1N3[i]));
							w1 = ((aN1N2[i]*aN1N2[i]) * (aN1N3[i]*NPOI) - (aN1N2[i]*aN1N3[i]) * (aN1N2[i]*NPOI)) * invDenom;
							w2 = ((aN1N3[i]*aN1N3[i]) * (aN1N2[i]*NPOI) - (aN1N2[i]*aN1N3[i]) * (aN1N3[i]*NPOI)) * invDenom;

						    // Check if projection lies within triangle - if it does return distance
			                if ((w1 >= 0) && (w2 >= 0) && ((w1 + w2) <= 1)) {
			                    t = dist;
			                    n = aNorm4[i];
			                    inew = i;
			                }
			            }
	        		}
        		}
        	}
        }


        // If the previous block found a tet, inew would be updated to the new region.
        // Therefore particle enters tet, update stuff.
        if (inew >= 0) {
            if (newmed) {
                *newmed = region_media[inew]; //Set newmet to the media of region inew
            }
            if (normal) { //If a normal has been set, invert to comply with EGS convention.
                n.x = -1 * n.x;
                n.y = -1 * n.y;
                n.z = -1 * n.z;
                *normal = n;
            }
        }
        return inew; //return value of the howfar call is inew.
    };



    //MJR customized
    EGS_Float hownear(int ireg, const EGS_Vector &x) override {
        EGS_Vector aP = x; // Particle position

        EGS_Vector aNP;
        EGS_Vector aPN;
        EGS_Float dist=1e30;

        // If inside the particle is inside a tet, the shortest distance to a face is the shortest distance to a plane
        if (ireg >= 0) { //Check if we're inside. ireg = -1 when outside, and is >=0 if inside a tet.

        	EGS_Float tmin = 1e30;

        	//Check face 1 normal distance
            aNP = aP - aN2[ireg]; //Compute vector from node 2 to the current particle position
            dist = (aNP*aNorm1[ireg]); // Direction is normal -> denominator (dir dot norm) = 1 and is implied

            if (dist < tmin) { //if the distance to face 1 is less than tmin, update tmin.
                tmin = dist;
            }
            // Face 2
            aNP = aP - aN1[ireg];
            dist = (aNP*aNorm2[ireg]); // Direction is normal -> denominator (dir dot norm) = 1 and is implied

            if (dist < tmin) {
                tmin = dist;
            }
            // Face 3
            dist = (aNP*aNorm3[ireg]); // Direction is normal -> denominator (dir dot norm) = 1 and is implied

            if (dist < tmin) {
                tmin = dist;
            }
            // Face 4
            dist = (aNP*aNorm4[ireg]); // Direction is normal -> denominator (dir dot norm) = 1 and is implied

            if (dist < tmin) {
                tmin = dist;
            }

            //At this point all four faces have been checked and tmin has been updated to the distance along
            //the normal to the closest face. Return tmin.
            return tmin;
        }

        // If we skipped the previous block, it's because the particle is outside (ireg == -1)
        // Now we have to check all tets so we'll cycle from 0 to nreg-1 looking at faces.
        // If 1 boundary face is involved - distance from particle to tet is distance from particle to face
        // If 2 face is  involved - distance from particle to tet is distance from particle to edge
        // If 3 face is  involved - distance from particle to tet is distance from particle to node
        // This code checks if a plane is involved if (aNorm*aPN) is > 0
        // It also only checks boundary faces which might suggest the code wouldn't properly
        // evaluate how many faces have  a positive dot product, but the distance_to_face algorith
        // returns the normal distance to the face if the normal is inside the bounds of the face, or the distance
        // to the nearest edge or node if the the normal to the face is outside of the bounds of the face.

        // TODO Have a look at using Gmsh native algorithm for calculating the nearest tet rather than polling
        // through all tets to find the minimum distance.

        // TODO Consider reorganizing the code to call distanct_to_face directly within the first four
        // if(neighbour == -1) checks and continually updating a minimum distance rather than counting
        // faces and determining which set of calls to do later in the block.

        bool face1;
        bool face2;
        bool face3;
        bool face4;
        int nFaces;

        for (int i = 0; i<nreg; i++) {

        	if (BoundaryTet[i]) { //If the tet has outside faces, look at it.

        		face1 = false;
	        	face2 = false;
	        	face3 = false;
	        	face4 = false;
	        	nFaces = 0;
        		if (n1[i] == -1) { //if the neighbour of face 1 is -1, this face is an outside face.

        			// check face 1
        			aPN = aN2[i] - aP; //compute vector from particle position to a node on face 1.
			        if ((aNorm1[i]*aPN) > 0) { //If dot product of face normal and the vector is positive
			        	face1 = true; // then we can see face 1.  Update the flag
			        	nFaces = nFaces + 1; //increment the number of faces the particle can see.
			        }
        		}
        		if (n2[i] == -1) {

        			// check face 2
        			aPN = aN1[i] - aP;
			        if ((aNorm2[i]*aPN) > 0) {
			        	face2 = true;
			        	nFaces = nFaces + 1;
			        }
        		}
        		if (n3[i] == -1) {

        			// check face 3
        			if ((aNorm3[i]*aPN) > 0) {
			        	face3 = true;
			        	nFaces = nFaces + 1;
			        }
        		}
        		if (n4[i] == -1) {

        			// check face 4

        			if ((aNorm4[i]*aPN) > 0) {
			        	face4 = true;
			        	nFaces = nFaces + 1;

			        }
        		}
				// Rules
		        // 1 face involved => somewhere on that face within the triangle (including edges and nodes)
		        // Check if Projection along normal is within the triangle
		        // Either it is and the distance from the particle to the projection is the distance
		        // or check the projection onto each edge
		        // find the edge that corresponds to the shortest length
		        // if the projection is within the nodes that define the edge
		        // that is the point - calculate the distance to the particle
		        // otherwise take the node nearest the point of the shortest projection on the corresponding edge
		        // 2 faces involved => somewhere on the shared edge (including the nodes)
		        // project the particle onto the line and if the projection is within the endpoints defined by the
		        // nodes calculate the distance
		        // otherwise take the nearest node and calculate the distance
		        // 3 faces involved => at the node

		        // Simplest case - 3 faces => nearest point is a node
		        // dist = 1e30;
		        if (nFaces == 3) {

		        	EGS_Float dist1;
		            EGS_Float dist2;
		            EGS_Float dist3;
		            // node is nearest point - just figure out which one and calculate the distance
		        	if (face1 && face2 && face3) {
		        		dist1 = dist_to_face(aP, aN2[i], aN3[i], aN4[i], aN2N3[i], aN2N4[i], aN3N4[i], aNorm1[i]);
		                dist2 = dist_to_face(aP, aN1[i], aN3[i], aN4[i], aN1N3[i], aN1N4[i], aN3N4[i], aNorm2[i]);
		                dist3 = dist_to_face(aP, aN1[i], aN2[i], aN4[i], aN1N2[i], aN1N4[i], aN2N4[i], aNorm3[i]);
		                if (dist1 < dist) {
		                    dist = dist1;
		                }
		                if (dist2 < dist) {
		                    dist = dist2;
		                }
		                if (dist3 < dist) {
		                    dist = dist3;
		                }
		                continue;
		        	}
		        	if (face1 && face2 && face4) {
		        		dist1 = dist_to_face(aP, aN2[i], aN3[i], aN4[i], aN2N3[i], aN2N4[i], aN3N4[i], aNorm1[i]);
		                dist2 = dist_to_face(aP, aN1[i], aN3[i], aN4[i], aN1N3[i], aN1N4[i], aN3N4[i], aNorm2[i]);
		                dist3 = dist_to_face(aP, aN1[i], aN2[i], aN3[i], aN1N2[i], aN1N3[i], aN2N3[i], aNorm4[i]);
		                if (dist1 < dist) {
		                    dist = dist1;
		                }
		                if (dist2 < dist) {
		                    dist = dist2;
		                }
		                if (dist3 < dist) {
		                    dist = dist3;
		                }
		                continue;
		        	}
		        	if (face1 && face3 && face4) {
		        		dist1 = dist_to_face(aP, aN2[i], aN3[i], aN4[i], aN2N3[i], aN2N4[i], aN3N4[i], aNorm1[i]);
		                dist2 = dist_to_face(aP, aN1[i], aN2[i], aN4[i], aN1N2[i], aN1N4[i], aN2N4[i], aNorm3[i]);
		                dist3 = dist_to_face(aP, aN1[i], aN2[i], aN3[i], aN1N2[i], aN1N3[i], aN2N3[i], aNorm4[i]);
		                if (dist1 < dist) {
		                    dist = dist1;
		                }
		                if (dist2 < dist) {
		                    dist = dist2;
		                }
		                if (dist3 < dist) {
		                    dist = dist3;
		                }
		                continue;
		        	}
		        	if (face2 && face3 && face4) {
		        		dist1 = dist_to_face(aP, aN1[i], aN3[i], aN4[i], aN1N3[i], aN1N4[i], aN3N4[i], aNorm2[i]);
		                dist2 = dist_to_face(aP, aN1[i], aN2[i], aN4[i], aN1N2[i], aN1N4[i], aN2N4[i], aNorm3[i]);
		                dist3 = dist_to_face(aP, aN1[i], aN2[i], aN3[i], aN1N2[i], aN1N3[i], aN2N3[i], aNorm4[i]);
		                if (dist1 < dist) {
		                    dist = dist1;
		                }
		                if (dist2 < dist) {
		                    dist = dist2;
		                }
		                if (dist3 < dist) {
		                    dist = dist3;
		                }
		                continue;
		        	}
		        }
		        // Next simplest case - 2 faces => nearest point is on an edge
		        // determine which one and find the closest point
		        if (nFaces == 2) {

		        	EGS_Float dist1;
		        	EGS_Float dist2;
		        	if (face1 && face2) {
		        		// edge 3-4
		        		//dist = dist_to_edge(aP, aN3, aN4, aN3N4);
		        		dist1 = dist_to_face(aP, aN2[i], aN3[i], aN4[i], aN2N3[i], aN2N4[i], aN3N4[i], aNorm1[i]);
		        		dist2 = dist_to_face(aP, aN1[i], aN3[i], aN4[i], aN1N3[i], aN1N4[i], aN3N4[i], aNorm2[i]);
		        		if (dist1 < dist) {
		        			dist = dist1;
		        		}
		        		if (dist2 < dist) {
		        			dist = dist2;
		        		}
		        		continue;
		        	}
		        	if (face1 && face3) {
		        		// edge 2-4
		        		//dist = dist_to_edge(aP, aN2, aN4, aN2N4);
		        		dist1 = dist_to_face(aP, aN2[i], aN3[i], aN4[i], aN2N3[i], aN2N4[i], aN3N4[i], aNorm1[i]);
		        		dist2 = dist_to_face(aP, aN1[i], aN2[i], aN4[i], aN1N2[i], aN1N4[i], aN2N4[i], aNorm3[i]);
		        		if (dist1 < dist) {
		        			dist = dist1;
		        		}
		        		if (dist2 < dist) {
		        			dist = dist2;
		        		}
		        		continue;
		        	}
		        	if (face1 && face4) {
		        		// edge 2-3
		        		//dist = dist_to_edge(aP, aN2, aN3, aN2N3);
		        		dist1 = dist_to_face(aP, aN2[i], aN3[i], aN4[i], aN2N3[i], aN2N4[i], aN3N4[i], aNorm1[i]);
		        		dist2 = dist_to_face(aP, aN1[i], aN2[i], aN3[i], aN1N2[i], aN1N3[i], aN2N3[i], aNorm4[i]);
		        		if (dist1 < dist) {
		        			dist = dist1;
		        		}
		        		if (dist2 < dist) {
		        			dist = dist2;
		        		}
		        		continue;
		        	}
		        	if (face2 && face3) {
		        		// edge 1-4
		        		//dist = dist_to_edge(aP, aN1, aN4, aN1N4);
		        		dist1 = dist_to_face(aP, aN1[i], aN3[i], aN4[i], aN1N3[i], aN1N4[i], aN3N4[i], aNorm2[i]);
		        		dist2 = dist_to_face(aP, aN1[i], aN2[i], aN4[i], aN1N2[i], aN1N4[i], aN2N4[i], aNorm3[i]);
		        		if (dist1 < dist) {
		        			dist = dist1;
		        		}
		        		if (dist2 < dist) {
		        			dist = dist2;
		        		}
		        		continue;
		        	}
		        	if (face2 && face4) {
		        		// edge 1-3
		        		//dist = dist_to_edge(aP, aN1, aN3, aN1N3);
		        		dist1 = dist_to_face(aP, aN1[i], aN3[i], aN4[i], aN1N3[i], aN1N4[i], aN3N4[i], aNorm2[i]);
		        		dist2 = dist_to_face(aP, aN1[i], aN2[i], aN3[i], aN1N2[i], aN1N3[i], aN2N3[i], aNorm4[i]);
		        		if (dist1 < dist) {
		        			dist = dist1;
		        		}
		        		if (dist2 < dist) {
		        			dist = dist2;
		        		}
		        		continue;
		        	}
		        	if (face3 && face4) {
		        		// edge 1-2
		        		//dist = dist_to_edge(aP, aN1, aN2, aN1N2);
		        		dist1 = dist_to_face(aP, aN1[i], aN2[i], aN4[i], aN1N2[i], aN1N4[i], aN2N4[i], aNorm3[i]);
		        		dist2 = dist_to_face(aP, aN1[i], aN2[i], aN3[i], aN1N2[i], aN1N3[i], aN2N3[i], aNorm4[i]);
		        		if (dist1 < dist) {
		        			dist = dist1;
		        		}
		        		if (dist2 < dist) {
		        			dist = dist2;
		        		}
		        		continue;
		        	}
		        }
		        // Most complex case - 1 face => nearest point is on the face
		        // determine which face and find the closest point
		        if (nFaces == 1) {
		        	EGS_Float dist1;

		        	if (face1) {

		        		dist1 = dist_to_face(aP, aN2[i], aN3[i], aN4[i], aN2N3[i], aN2N4[i], aN3N4[i], aNorm1[i]);
		        		if (dist1 < dist) {
		        			dist = dist1;
		        		}
		        		continue;
		        	}
		        	if (face2) {

		        		dist1 = dist_to_face(aP, aN1[i], aN3[i], aN4[i], aN1N3[i], aN1N4[i], aN3N4[i], aNorm2[i]);
		        		if (dist1 < dist) {
		        			dist = dist1;
		        		}
		        		continue;
		        	}
		        	if (face3) {

		        		dist1 = dist_to_face(aP, aN1[i], aN2[i], aN4[i], aN1N2[i], aN1N4[i], aN2N4[i], aNorm3[i]);
		        		if (dist1 < dist) {
		        			dist = dist1;
		        		}
		        		continue;
		        	}
		        	if (face4) {

		        		dist1 = dist_to_face(aP, aN1[i], aN2[i], aN3[i], aN1N2[i], aN1N3[i], aN2N3[i], aNorm4[i]);

		        		if (dist1 < dist) {
		        			dist = dist1;
		        		}

		        		continue;
		        	}
		        }
		    }
    	}

	    return dist;
    };


    const string &getType() const {
        return type;
    };

    void printInfo() const override ;

    //create a mesh using a new mesh class
    //static fn mirrors similar EGS_BaseGeometry createGeometry fn
    //static
    // bool operator==(const EGS_Mevex_tet_collection& rhs) const {
    //   bool res;
    //   res = aN1 == rhs.aN1;
    //
    //   aN2, aN3, aN4;
    //   // //EGS_Vector          *aN1N2, *aN1N3, *aN1N4, *aN2N3, *aN2N4, *aN3N4; // Node 1 -> Node 2 vector, ...
    //   // std::vector<EGS_Vector> aN1N2, aN1N3, aN1N4, aN2N3, aN2N4, aN3N4;
    //   // //EGS_Vector          *aNorm1, *aNorm2, *aNorm3, *aNorm4; // Normal of plane without node 1, Normal of plane without node 2, ...
    //   // std::vector<EGS_Vector> aNorm1, aNorm2, aNorm3, aNorm4;
    //   //
    //   // //int 				*n1, *n2, *n3, *n4; // Neighbours of the tet; n1 is the face 1 (missing N1) neighbour, etc.
    //   // //bool				*BoundaryTet; //Indicates if the tet contains at least 1 face external to the collection.
    //   // //int 	 			*mediaIndices; //Indicates which media from the input file to apply to the tet.
    //   //
    //   // std::vector<int> n1, n2, n3, n4;
    //   // std::vector<bool> BoundaryTet;
    //   // std::vector<int> mediaIndices;
    //
    //   return res;
    // }
};


 EGS_BaseGeometry* createMeshGeometry(EGS_Input *input, const double scaling, const Mesh& m){

  egsInformation("It works!\n");

  EGS_Mevex_tet_collection* result = new EGS_Mevex_tet_collection(m, scaling, "Mesh_tet_collection");

  egsInformation("Makes tet collection!\n");

  result->setBoundaryTolerance(input);

  //overloaded for meshes
  result->setTetMedia(m);

  result->setLabels(input);

  return result;

 }

#endif
