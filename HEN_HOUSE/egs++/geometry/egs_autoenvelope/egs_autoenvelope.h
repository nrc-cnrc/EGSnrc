/*
###############################################################################
#
#  EGSnrc egs++ auto envelope geometry headers
#  Copyright (C) 2016 Randle E. P. Taylor, Rowan M. Thomson,
#  Marc J. P. Chamberland, D. W. O. Rogers
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
#  Author:          Randle Taylor, 2016
#
#  Contributors:    Marc Chamberland
#                   Rowan Thomson
#                   Dave Rogers
#                   Martin Martinov
#
###############################################################################
#
#  egs_autoenvelope was developed for the Carleton Laboratory for
#  Radiotherapy Physics.
#
###############################################################################
*/


/*! \file egs_autoenvelope.h
 *  \brief An envelope geometry with automatic inscribed region detection (inspired by EGS_FastEnvelope)
 *  \author Randle Taylor
 */

#ifndef EGS_AENVELOPE_GEOMETRY_
#define EGS_AENVELOPE_GEOMETRY_

#ifdef WIN32

    #ifdef BUILD_AENVELOPE_DLL
        #define EGS_AENVELOPE_EXPORT __declspec(dllexport)
    #else
        #define EGS_AENVELOPE_EXPORT __declspec(dllimport)
    #endif
    #define EGS_AENVELOPE_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_AENVELOPE_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_AENVELOPE_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_AENVELOPE_EXPORT
        #define EGS_AENVELOPE_LOCAL
    #endif

#endif


#include "egs_base_geometry.h"
#include "egs_functions.h"
#include "egs_transformations.h"
#include "egs_shapes.h"
#include "volcor.h"

#include<vector>
#include<algorithm>
#include<map>
#include<set>


/*!

\ingroup Geometry
\ingroup CompositeG

EGS_AEnvelope is a fast envelope geometry used to inscribe one or more
copies of another geometry inside of a base geometry (for example a
brachytherapy seed inside a rectilinear phantom).

The implementation is very similar to the (currently undocumented)
EGS_FastEnvelope geometry (and indeed the implementation borrows heavily
from that class). Both of these geometry classes gain an efficiency
advantage over regular EGS_EnvelopeGeometry geometries by skipping the
boundary checks of inscribed geometries when the particle is in a region of
the base geometry without any other geometries inscribed in it.

With the EGS_FastEnvelope a user must manually specify which inscribed
geometries are present in a given region of the base geometry.  This process
would be tedious and error prone for base geometries with large numbers of
regions and many inscribed geometries in arbitrary orientations.  Instead of
this manual process, the Auto Envelope uses a Monte Carlo simulation to
determine which regions of the base geometry contain inscribed geometries.
This Monte Carlo process can also optionally be used to correct voxel volumes
of the base geometry.


As an example, consider a 5.1cm x 5.1cm x 5.1cm EGS_XYZGeometry phantom
subdivided into 0.2cm x 0.2cm x 0.2cm voxels (51^3=132 651 voxels), with 125
brachytherapy seeds inscribed in it.  With a regular EGS_EnvelopeGeometry,
the howfar routine has to check the boundaries of 126 geometry objects (1
base geometry + 125 seeds) at each step of the simulation.   With a fast
envelope, if a particle is in a voxel without any inscribed geometries
present the howfar routine only needs to check the boundaries of the base
phantom geometry and can skip the checks for the other 125 seed geometries.
This can make simulations with a fast envelope run an order of magnitude
faster than the same simulation using a plain EGS_EnvelopeGeometry.


Region Discovery / Volume Correction Algorithm
--------------------------------------------

(Note: Currently the auto envelope can only accomodate a single type of inscribed
geometry, although it could be adapted to handle an aribtrary number of
different inscribed geometries.)

With reference to the diagram below you can show that
if the bounding shape volume is \f$V_{BS}\f$, then the
inscribed geometry  volume, \f$V_{inscribed}\f$, is estimated as:

\f[

 V_{inscribed} =(total\_in\_inscribed / n\_total\_points) * V_{BS}

\f]

and the volume of the shape inside a give region \f$i\f$, \f$V_{inscribed}(i)\f$ is given by:

\f[
 V_{inscribed}(i) = (n\_inscribed(i)/n\_total\_points) * V_{BS}
\f]

So the corrected volume of region \f$i\f$ is just:

\f[
 V_{corrected}(i) = V_{uncorrected}(i) - (n\_inscribed(i)/n\_total\_points) * V_{BS}
\f]


\image html vc_diagram800.png "Volume correction diagram"


The way the algorithm works is roughly:

- Choose a random point in space within the bounding shape.
- Check whether the point lands within the inscribed geometry
- If the point is outside, stop and go to step 1
- If the point is inside increment total number of points falling within
  inscribe geometry
- Transform the point to all locations of the inscribed geometry
- find the region of the base geometry that the point falls within
- increment the number of hits in that region
- add the inscribed geometry to the set of all geometries in that region

which in pseudo-code would look something like:
\verbatim

total_in_inscribed = 0

for i = 1 to n_base_regions
    n_inscribed[i] =  0
    geoms_in_reg[i] = empty_set()

for step = 1 to n_total_points

    point  =  get_random_point_within_bounding_shape()

    if inscribed_geom.point_is_inside(point):

        total_in_inscribed += 1

        for inscribed_copy in all_inscribed_copies:
            transformed_point = transform_point_to_location(inscribed_copy.location, point)
            base_geom_reg =  base_geom.whichRegion(transformed_point)
            n_inscribed[base_geom_reg] += 1
            geoms_in_reg[base_geom_reg].add_to_set(inscribed_copy)


\endverbatim

Geometry Definition Format
--------------------------

There is an example geometry included in the examples/seeds_in_xyz_aenv.geom file.

\verbatim

:start geometry:

    library = egs_autoenvelope
    name = autoenvelope
    type = EGS_AEnvelope # optional: EGS_AEnvelope(default) or EGS_ASwitchedEnvelope
    print debug info = no  # optional: no(default), yes
    output volume correction file = gzip # optional no(default), yes(equivalent to text), text or gzip

    base geometry = name_of_base_geom # the name of a previously defined geometry

    :start inscribed geometry:

        inscribed geometry name = seed # the name of a previously defined geometry

        # optional: locations of inscribed geometries.  If no transformations
        # are included, a single inscribed geometry at the origin will be used
        :start transformations:
            :start transformation:
                translation = -2,-2,-2
            :stop transformation:

            :start transformation:
                translation = -2,-2,-1
            :stop transformation:

        :stop transformations:

        :start region discovery:

            action = discover # optional: discover (default), discover and correct volume, discover and zero volume

            volume correction file = phantom.autoenv.volcor # optional (if using omit below)

            density of random points (cm^-3) = 1E7 # optional random point sampling density defaults to 1E8

            # bounding shape definition. Note only volumetric shapes make sense here!
            :start shape:

                type = cylinder
                radius = 0.04
                height = 0.45

            :stop shape:

            # optional rng definition
            :start rng definition:
                type = sobol
                initial seed = 1234
            :stop rng definition:

            # -or-

            :start rng definition:
                type = ranmar
                initial seeds = 123, 456
            :stop rng definition:


        :stop region discovery:

    :stop inscribed geometry:

:stop geometry:

\endverbatim

Volume correction files
-----------------------

By including the `output volume correction file = text # text or gzip`
input key, you can output a volume correction file that will output a file
which includes all of the regions that were found to contain inscribed
geometries and their corrected volumes. (For the gzip functionality you
must have the egspp-geometry-lib-extras installed: see Optional Features
below).

The volume correction file can then be used in future runs by using a
volume correction block like the following for the inscribed geometry:

\verbatim

   :start region discovery:
       action = discover # optional: discover(default), discover and correct volume, discover and zero volume
       volume correction file = phantom.autoenv.volcor
   :stop region discovery:

\endverbatim

This allows you to e.g. run a lengthy volume correction a single time and
then reuse the volume correction file in future runs.

Points to consider
------------------

For performance reasons, the bounding shape should be made to just cover the
inscribed geometry (this minimizes the total number of random points used).

Due to the nature of region discovery algorithm, it is possible that not all
regions containing inscribed geometries will be discovered.  This would
generally happen if your bounding shape is too small to cover the entire
insribed geometry or you choose a density of points that is too low.

By default a Sobol quasi-random number generator (if available, see
    `Optional Features` below) is used for volume correction/region
discovery but that can be overridden by including an `rng definition`
block (see example input above)


EGS_ASwitchedEnvelope
------------------

There is a second geometry, a "switched" Auto Envelope type included in the
auto envelope library.  This geometry type allows you to activate and
deactivate inscribed geometries in custom egspp user codes.  As an example,
this geometry was developed to allow for investigating how interseed
attenuation affects dose distributions in brachytherapy implants.  By only
having one seed active at a time, you can simulate a superposition (TG43
style) type calculation.

The inputs are all exactly the same as the EGS_AEnvelope and only the
first inscribed geometry is activated upon initialization. Examples of using
this geometry type are available in the EGS_ASwitchedEnvelope class documentation.

Examples
--------

An example geometry file using an autoenvelope is provided in
seeds_in_xyz_aenv.geom.


Optional Features
-----------------

In order to use the gzip and Sobol functionality you must have the egspp-geometry-lib-extras
installed.  Due to NRC licensing requirements this code is distributed separately and
can be obtained from https://github.com/clrp-code/egspp-geometry-lib-extras/ .

*/

using std::vector;
using namespace volcor;


/*! \brief A helper class for initializing auto envelopes*/
struct EGS_AENVELOPE_LOCAL AEnvelopeAux {
    EGS_BaseGeometry *geom;
    EGS_AffineTransform *transform;
    volcor::VCOptions *vcopts;

    AEnvelopeAux(EGS_BaseGeometry *geom, EGS_AffineTransform *transform, volcor::VCOptions *vcopts):
        geom(geom), transform(transform), vcopts(vcopts) {};
};

/*! \brief A fast envelope geometry with automatic region detection */
class EGS_AENVELOPE_EXPORT EGS_AEnvelope : public EGS_BaseGeometry {


public:

    EGS_AEnvelope(EGS_BaseGeometry *base_geom,
                  const vector<AEnvelopeAux> inscribed, const string &Name = "", bool debug=false, string output_vc_file="no");

    ~EGS_AEnvelope();

    int getGlobalRegFromLocalReg(EGS_BaseGeometry *g, int local_reg);

    int getGlobalRegFromLocal(const volcor::GeomRegPairT local) const;

    volcor::GeomRegPairT getLocalFromGlobalReg(int ireg) const;

    int getNRegWithInscribed() const;

    bool isRealRegion(int ireg) const;

    bool isInside(const EGS_Vector &x);

    int isWhere(const EGS_Vector &x);

    int inside(const EGS_Vector &x);

    int medium(int ireg) const;

    virtual vector<EGS_BaseGeometry *> getGeomsInRegion(int ireg);

    int computeIntersections(int ireg, int n, const EGS_Vector &X,
                             const EGS_Vector &u, EGS_GeometryIntersections *isections);


    EGS_Float howfarToOutside(int ireg, const EGS_Vector &x, const EGS_Vector &u);

    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t, int *newmed = 0, EGS_Vector *normal = 0);

    EGS_Float hownear(int ireg, const EGS_Vector &x);

    bool hasBooleanProperty(int ireg, EGS_BPType prop) const;

    void setBooleanProperty(EGS_BPType);

    void addBooleanProperty(int);

    void setBooleanProperty(EGS_BPType,int,int,int step=1);

    void addBooleanProperty(int,int,int,int step=1);

    int getMaxStep() const;

    virtual EGS_Float getVolume(int ireg);

    virtual EGS_Float getCorrectionRatio(int ireg);

    virtual const string &getType() const {
        return type;
    };

    void printInfo() const;

    void setRelativeRho(int start, int end, EGS_Float rho);
    void setRelativeRho(EGS_Input *);

    EGS_Float getRelativeRho(int ireg) const;

    /*! \brief Take a block of transformations and return vector of EGS_AffineTransforms
    *
    * createTransforms takes an input with children of the form:
    *    :start transformation:
    *        translation = -2,-2,-2
    *    :stop transformation:
    *    :start transformation:
    *        translation = -2,-2,-1
    *    :stop transformation:
    *    :start transformation:
    *        translation = -2,-2,0
    *        rotation = ...
    *    :stop transformation:
    *
    * and returns a vector of pointers to EGS_AffineTransforms
    */
    static vector<EGS_AffineTransform *> createTransforms(EGS_Input *inpt);


    /*! \brief function for checking whether a given geometry type
     * is allowed to be used as a base geometry */
    static bool allowedBaseGeomType(const string &geom_type);


protected:

    EGS_BaseGeometry *base_geom;           //!< The envelope geometry
    vector<EGS_BaseGeometry *> inscribed_geoms; //!< The inscribed geometries
    vector<EGS_AffineTransform *> transforms; //!< The inscribed geometries
    vector<volcor::VCOptions *> opts; //!< The inscribed geometries
    int nregbase;   //!< Number of regions in the base geometry
    int ninscribed;   //!< Number of regions in the base geometry
    int nreg_with_inscribed;

    bool debug_info;
    string output_vc;
    volcor::VCResults vc_results;

    // conversions from inscribed to global region numbers and vice versa
    map<volcor::GeomRegPairT, int> local_to_global_reg;
    map<int, volcor::GeomRegPairT> global_reg_to_local;

    //keep track of which geometries are present in which base geometry regions
    vector<EGS_BaseGeometry *> *geoms_in_region;

    static string type;    //!< Geometry type

    const static string allowed_base_geom_types[];

    /*! \brief Don't set media for an envelope geometry

    This function is re-implemented to warn the user to not set media
    in the envelope geometry. Instead, media should be set for the envelope
    and in the inscribed geometries.
    */
    void setMedia(EGS_Input *,int,const int *);

    double findRegionsInscribedIn(vector<EGS_BaseGeometry *>, vector<EGS_AffineTransform *>, volcor::VCOptions *);
    double loadFileVolumeCorrections(vector<EGS_BaseGeometry *>, vector<EGS_AffineTransform *>, volcor::VCOptions *);

    void applyVolumeCorrections(volcor::VCOptions *opts, volcor::HitCounterT hit_counter);

    void writeVCToFile(ostream &);
    void writeVolumeCorrection();

private:

    void setPropertyError(const char *funcname) {
        egsFatal("EGS_AEnvelope::%s: don't use this method\n  Define "
                 "properties in the constituent geometries instead\n",
                 funcname);
    };

    bool getHasRhoScaling();

};

/*! \brief This geometry type allows you to activate and deactivate inscribed geometries in custom egspp user codes.
 *
 * EGS_ASwitchedEnvelope
* ------------------
*
*  This geometry type allows you to activate and
*  deactivate inscribed geometries in custom egspp user codes.  As an example,
*  this geometry was developed to allow for investigating how interseed
*  attenuation affects dose distributions in brachytherapy implants.  By only
*  having one seed active at a time, you can simulate a superposition (TG43
*   style) type calculation.
*
*  The inputs are all exactly the same as the EGS_AEnvelope and only the
*  first inscribed geometry is activated upon initialization.
*
\verbatim

   EGS_BaseGeometry *tmp_switch = EGS_BaseGeometry::getGeometry(your_geom_name);
   EGS_ASwitchedEnvelope *switched_geom = static_cast<EGS_ASwitchedEnvelope*>(tmp_switch);

   // activate only the ith geometry (deactivate the rest)
   switched_geom->setActiveByIndex(i);

   // add ith geometry to currently active list
   switched_geom->activateByIndex(i);

   // remove ith geometry from currently active list
   switched_geom->deactivateByIndex(i);

   // activate multipe geoms by index
   vector<int> to_activate;
   to_activate.push_back(i);
   to_activate.push_back(j);
   switched_geom->setActiveGeometries(to_activate);

   // activate multipe geoms by pointer
   vector<EGS_BaseGeometry* > to_activate;
   to_activate.push_back(pointer_to_inscribed_i);
   to_activate.push_back(pointer_to_inscribed_j);
   switched_geom->setActiveGeometries(to_activate);


   // cycle through activating one geometry at a time
   // and deactivating the rest
   switched_geom->cycleActive();
   switched_geom->cycleActive();
   switched_geom->cycleActive();


\endverbatim

*/
class EGS_AENVELOPE_EXPORT EGS_ASwitchedEnvelope : public EGS_AEnvelope {

private:

    vector<EGS_BaseGeometry *> active_inscribed;
    int cur_ptr;

protected:

    static string type;    //!< Geometry type

    vector<EGS_BaseGeometry *> getGeomsInRegion(int ireg);

public:

    EGS_ASwitchedEnvelope(EGS_BaseGeometry *base_geom,
                          const vector<AEnvelopeAux> inscribed, const string &Name = "", bool debug=false, string output_vc_file="no");

    const string &getType() const {
        return type;
    };

    /*! activate multiple geometries by passing pointers and set the current pointer
     to the first geometryin the input vector */
    void setActiveGeometries(vector<EGS_BaseGeometry *> geoms);

    /*! activate multiple geometries at input indexes and set the current pointer
     to the first index in the input vector */
    void setActiveGeometries(vector<int> geom_indexes);

    /*! check if a given region in the base geometry has an active geometry in it */
    bool hasActiveGeom(int ireg);

    /*! check if a given region in the base geometry has an inactive geometry in it */
    bool hasInactiveGeom(int ireg);

    /*! activate a single geometry and deactivate all the rest */
    void setActiveByIndex(int inscribed_index);

    /*! activate an inscribed geometry */
    void activateByIndex(int inscribed_index);

    /*! deactivate an inscribed geometry */
    void deactivateByIndex(int inscribed_index);

    /*! increment current pointer and activate the next geometry */
    void cycleActive();

};

#endif
