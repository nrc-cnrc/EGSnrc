/*
###############################################################################
#
#  EGSnrc egs++ envelope geometry headers
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
#  volcor was developed for the Carleton Laboratory for Radiotherapy Physics.
#
###############################################################################
*/


/*! \file volcor.h
 *
 *  \brief Region discovery/volume correction functionality for EGS_AEnvelope geometries
 *  \author Randle Taylor (randle.taylor@gmail.com)
 *  \version 1.0
 *  \date 2015/03/09
 */

#ifndef VC_VOLCOR
#define VC_VOLCOR

#include <map>
#include <set>
#include <cstdlib>
#include <fstream>

#include "egs_base_geometry.h"
#include "egs_functions.h"
#include "egs_input.h"
#include "egs_timer.h"
#include "egs_rndm.h"
#include "egs_shapes.h"

#ifdef HAS_SOBOL
    #include "egs_sobol.h"
#endif

#ifdef HAS_GZSTREAM
    #include "gzstream.h"
#endif

/*! \brief Region discovery/volume correction for auto envelope geometries
 *
 *
 * The *volcor* namespace contains methods for correction of geometry region
 * volumes which have other geometries overlapping them.  Volume corrections
 * are done using a simple Monte Carlo method.
*/
namespace volcor {


/*! Available volume correction modes */
enum VolCorMode {
    DISCOVERY_ONLY,  /*!< Region discovery only. No volume correction applied */
    ZERO_VOL,       /*!< Set base geometry region volume to zero in regions with inscribed geometries */
    CORRECT_VOLUME  /*!< Subtract inscribed geometry volume from base geometry region volume */
};

/*! Type for specifying a region (GeomRegPairT.second) within a given geometry (GeomRegPairT.first) */
typedef pair<EGS_BaseGeometry *, int> GeomRegPairT;


/*! Counter for counting number of times that a base geometry region (HitCounterT.first)
 * gets hit (HitCounterT.second) by random points */
typedef std::map<int, EGS_I64> HitCounterT;


/*! A map from base geomtry region number to the set of geometries inscribed in that region */
typedef map<int, set<EGS_BaseGeometry *> > RegionGeomSetT;

/*! \brief RegVolumeT is a pair of the form (RegionNumber, Volume) */
typedef pair<int, EGS_Float> RegVolume;


/*! getShapeVolume takes an EGS_Input for a shape and
 * returns the volume of the shape.  Currently the volume will
 * be calculated automatically for cylinders, spheres and
 * box shapes. Other shapes must specify a volume using
 * the *shape volume* input key. For example:
\verbatim

:start shape:
    type = my_new_shape
    input key 1 = 1234
    input key 2= 5678
    shape volume = 123456
:stop shape:

\endverbatim
 *
 * If *shape volume* is present for a cylinder, sphere or box shape that value
 * will be used and the automatic calculation will be ignored
 * */
EGS_Float getShapeVolume(EGS_Input *shape_inp) {

    if (!shape_inp) {
        return -1;
    }

    EGS_Float volume;
    int err = shape_inp->getInput("shape volume", volume);
    if (!err) {
        return volume;
    }

    string shape_type;
    shape_inp->getInput("type", shape_type);

    if (shape_type == "cylinder") {
        EGS_Float radius, height;
        int err = shape_inp->getInput("radius", radius);
        if (err) {
            egsFatal("getShapeVolume :: missing 'radius' input for shape\n");
        }

        err = shape_inp->getInput("height", height);
        if (err) {
            egsFatal("getShapeVolume :: missing 'height' input for shape\n");
        }

        return M_PI*radius*radius*height;

    }
    else if (shape_type == "sphere") {
        EGS_Float radius;

        int err = shape_inp->getInput("radius", radius);
        if (err) {
            egsFatal("getShapeVolume :: missing 'radius' input for shape\n");
        }

        return 4./3.*M_PI*radius*radius*radius;

    }
    else if (shape_type == "spherical shell" || shape_type == "egsSphericalShell") {
        EGS_Float ri, ro;
        int hemi;
        int err = shape_inp->getInput("inner radius", ri);
        if (err) {
            egsFatal("getShapeVolume :: missing 'inner radius' input for shape\n");
        }

        err = shape_inp->getInput("outer radius", ro);
        if (err) {
            egsFatal("getShapeVolume :: missing 'outer radius' input for shape\n");
        }

        err = shape_inp->getInput("hemisphere", hemi);
        if (err) {
            hemi = 0;
        }
        EGS_Float vol = 4./3.*M_PI*(ro*ro*ro - ri*ri*ri);
        if (hemi != 0) {
            return vol/2;
        }

        return vol;

    }
    else if (shape_type == "box") {
        vector<EGS_Float> box_size;
        int err = shape_inp->getInput("box size", box_size);
        if (err) {
            egsFatal("getShapeVolume :: missing 'box size' input for shape\n");
        }
        if (box_size.size() == 3) {
            return box_size[0]*box_size[1]*box_size[2];
        }
        else {
            return box_size[0]*box_size[0]*box_size[0];
        }
    }

    if (shape_type == "") {
        egsWarning("Either include a `type` or `shape volume` input key.");
    }
    else {
        egsWarning(
            "The volume (in cm^3) for shape type '%s' must be specified using a `shape volume` input key.",
            shape_type.c_str()
        );
    }

    return -1;

}

/*! \brief Volume correction initialization helper class
 *
 * VCOptions is a small helper class for parsing a volume correction input item
 * A sample input would look like this:
 * \verbatim
 :start region discovery:

    action = discovery # discover, discover and correct volume, discover and zero volume
    density of random points (cm^-3) = 1E6 # Defaults to 1E8

    :start shape:
        type = cylinder
        radius = 0.04
        height = 0.45
        # volume = 123456 # use volume key for shapes other than cylinder, sphere, or box
    :stop shape:

 :stop region discovery:
\endverbatim

 *
 * You may also include a standard egs++ random number generator input if
 * you want to use an RNG different from the EGS Default
 *
 * */
class VCOptions {

public:

    const static unsigned long DEFAULT_RAND_POINT_DENSITY = 100000000;

    /*! VCOptions constructor. Initializes volume correction options from given input */
    VCOptions(EGS_Input *inp):
        rng(NULL), vc_file(""), input(inp), bounds(NULL), sobolAllowed(false) {

        valid=true;

        if (!inp) {
            valid = false;
            return;
        }

        setMode();

        // set external file to load volume corrections from
        setVCFile();

        if (vc_file == "") {
            // no external file found. Run a MC volume correction

            int err = setBoundsShape();
            if (err) {
                valid = false;
                return;
            }

            setDensity();
            setRNG();
        }

    }

    /*! VCOptions destructor. */
    ~VCOptions() {
        if (rng) {
            delete rng;
        }
        if (bounds) {
            delete bounds;
        }
    }

    /*! Return a random point within the boundary shape. */
    EGS_Vector getRandomPoint() {
        return bounds->getRandomPoint(rng);
    }

    bool valid; /*!< was the object initialized completely? */

    double bounds_volume; /*!< Volume of bounding shape in cm^3 */

    EGS_Float density; /*!< Density of points (cm^-3) used for MC volume calculation */
    EGS_Float npoints; /*!< total number of points used for VC (density*bounds_volume) */

    VolCorMode mode;  /* mode requested by user (defaults to DISCOVERY_ONLY) */

    EGS_RandomGenerator *rng;
    string vc_file;

protected:

    EGS_Input *input;

    EGS_BaseShape *bounds;
    bool sobolAllowed;


    /*! get requested mode from input. Default to DISCOVERY_ONLY */
    void setMode() {

        vector<string> choices;
        choices.push_back("discover");
        choices.push_back("discover and zero volume");
        choices.push_back("discover and correct volume");

        mode = (VolCorMode)input->getInput("action", choices, (int)DISCOVERY_ONLY);
    }

    /*! get external volume correction file */
    void setVCFile() {
        int err = input->getInput("volume correction file", vc_file);
        if (err) {
            vc_file = "";
        }
    }

    /*! create bounding shape from the shape input and calculate its volume */
    int setBoundsShape() {

        EGS_Input *shape_inp = input->takeInputItem("shape");
        if (!shape_inp) {
            egsWarning("VolCor::VCOptions::setBoundsShape - no `shape` input found.\n");
            return 1;
        }

        bounds_volume = getShapeVolume(shape_inp);
        if (bounds_volume < 0) {
            egsWarning("VolCor::VCOptions::setBoundsShape - unable to calculate volume.\n");
            delete shape_inp;
            return 1;
        }

        bounds = EGS_BaseShape::createShape(shape_inp);
        if (!bounds) {
            egsWarning("VolCor::VCOptions::setBoundsShape - `shape` input not valid.\n");
            delete shape_inp;
            return 1;
        }

        sobolAllowed = bounds->getObjectType() == "box";

        delete shape_inp;
        return 0;

    }

    /*! set user requested density or default to DEFAULT_RAND_POINT_DENSITY */
    void setDensity() {

        int err = input->getInput("density of random points (cm^-3)", density);
        if (err) {
            egsWarning("The volume correction 'density of random points (cm^-3)' input was not found\n");
            density = (EGS_Float)DEFAULT_RAND_POINT_DENSITY;
        }

        npoints = max(1., density*bounds_volume);
    }

    /*! set user requested RNG or default to EGS_RandomGenerator::defaultRNG */
    void setRNG() {

        EGS_Input *rng_input = input->getInputItem("rng definition");

        if (rng_input) {
            string type;
            int err = rng_input->getInput("type", type);
            if (!err && rng_input->compare(type, "sobol")) {
                if (!sobolAllowed) {
                    egsWarning(
                        "Sobol QRNG are not allowed for non rectilinear shapes. "
                        "Using default Ranmar instead.\n"
                    );
                    rng = EGS_RandomGenerator::defaultRNG();
                }
                else {
#ifdef HAS_SOBOL
                    rng = new EGS_Sobol(rng_input);
#else
                    egsWarning("Sobol RNG requested but not compiled with Sobol support\n");
#endif
                }
            }
            else {
                rng = EGS_RandomGenerator::createRNG(rng_input);
            }

            if (!rng) {
                egsFatal("VolCor::setRNG Invalid 'rng definition'.\n");
            }

        }
        else {
#ifdef HAS_SOBOL
            if (sobolAllowed) {
                rng = new EGS_Sobol();
            }
            else {
#endif
                rng = EGS_RandomGenerator::defaultRNG();
#ifdef HAS_SOBOL
            }
#endif
        }
    }

};

/*! \brief Struct used to collect and output results about a volume correction run.*/
struct VCResults {

public:

    bool success;  /*!< did the volume correction succeeed? */
    EGS_Float time; /*!< how long (s) did the volume correction take */
    double density; /*!< what was the density of points used for the VC */
    double npoints; /*!< what was the total number of points used for the VC */
    double ninscribed; /*!< what was the total number of inscribed geometries */
    double bounds_volume; /*!< what was the volume of the bounding shape */
    double inscribed_volume; /*!< what was the estimated volume of the inscribed geometry */

    string vc_file;

    RegionGeomSetT regions_with_inscribed; /*!< which base regions have inscribed geometries in them */
    vector<EGS_Float> uncorrected_volumes; /*!< uncorrected volume of all base regions */
    vector<EGS_Float> corrected_volumes; /*!< corrected (value of corrected vol depends on mode)volume of all base regions */

    VCOptions *options;

    VCResults():
        success(false),
        time(0),
        density(0),
        npoints(0),
        ninscribed(0),
        bounds_volume(0),
        inscribed_volume(0),
        vc_file(""),
        options(0) {};

    VCResults(string vcfile):
        success(false),
        time(0),
        density(0),
        npoints(0),
        ninscribed(0),
        bounds_volume(0),
        inscribed_volume(0),
        vc_file(vcfile),
        options(0) {};


    VCResults(VCOptions *opts): success(false), time(0), inscribed_volume(0), options(opts) {
        vc_file = opts->vc_file;
        density = opts->density;
        npoints =  opts->npoints;
        bounds_volume = opts->bounds_volume;
    }

    /*! print information about the volume correction run */
    void outputResults() const {

        egsInformation(" --------- Volume Correction Results -----------\n");
        if (vc_file == "") {
            egsInformation(" Time taken                  = %.4f s (%.3E s/point) \n", time, time/npoints);
            egsInformation(" Density of points used      = %.3E points/cm^-3\n", density);
            egsInformation(" Number of points used       = %G\n", npoints);
            egsInformation(" Bounding shape volume       = %.5E cm^3\n", bounds_volume);
            egsInformation(" Inscribed geometry volume   = %.5E cm^3\n", inscribed_volume);
            options->rng->describeRNG();
        }
        else {
            egsInformation(" Time taken                  = %.4f s\n", time);
            egsInformation(" Volume correction file      = %s\n", vc_file.c_str());
        }

        egsInformation(" -----------------------------------------------\n");

    }

};


/*! \brief Apply volume corrections to base regions.
 *
 * After the MC volume run is complete, use the tallied hits and mode requested
 * to set the correct volume of the base regions. */
vector<EGS_Float> applyVolumeCorrections(VCOptions *opts, HitCounterT hit_counter, vector<EGS_Float> uncorrected) {

    bool zero = opts->mode == ZERO_VOL;
    double bounds_vol = opts->bounds_volume;
    double npoints =  opts->npoints;
    vector<EGS_Float> corrected_vols(uncorrected);

    for (HitCounterT::iterator hi = hit_counter.begin(); hi != hit_counter.end(); hi++) {

        int base_reg = hi->first;
        if (base_reg < 0) {
            continue;
        }

        int hits = hi->second;
        EGS_Float corrected = uncorrected[base_reg] - bounds_vol*double(hits)/npoints;
        corrected_vols[base_reg] = zero ? 0 : corrected;
    }

    return corrected_vols;
}

/*! \brief Apply file volume corrections to base regions.
 *
 * After volumes are loaded from file use the volumes and mode requested
 * to set the correct volume of the base regions. */
vector<EGS_Float> applyFileVolumeCorrections(VCOptions *opts, vector<RegVolume> &reg_volumes, vector<EGS_Float> uncorrected) {

    bool zero = opts->mode == ZERO_VOL;
    vector<EGS_Float> corrected_vols(uncorrected);

    for (size_t rvc=0; rvc < reg_volumes.size(); rvc++) {
        int base_reg = reg_volumes[rvc].first;
        EGS_Float vol =  reg_volumes[rvc].second;
        corrected_vols[base_reg] = zero ? 0 : vol;
    }

    return corrected_vols;
}

vector<EGS_Float> getUncorrectedVolumes(EGS_BaseGeometry *base) {
    vector<EGS_Float> uncorrected;
    for (int ir=0; ir < base->regions(); ir++) {
        EGS_Float vol = base->getVolume(ir);
        uncorrected.push_back(vol);
    }
    return uncorrected;

}

/*! \brief Run the MC simulation.
 *
 * Finds regions with inscribed geometries and corrects
 * the volume of those regions. The algorithm is described
 * on main page of the Auto Envelope documentation. */
VCResults findRegionsWithInscribed(VCOptions *opts, EGS_BaseGeometry *base,
                                   vector<EGS_BaseGeometry *> inscribed, vector<EGS_AffineTransform *> transforms) {

    EGS_Timer timer;
    timer.start();

    VCResults results(opts);

    results.uncorrected_volumes =  getUncorrectedVolumes(base);

    EGS_Vector point;

    EGS_I64 n_in_inscribed = 0;

    //use first inscribed object to test if point is in inscribed geom
    EGS_BaseGeometry *first_inscribed = inscribed[0];
    EGS_AffineTransform *first_transform = transforms[0];

    HitCounterT hit_counter;

    for (EGS_I64 i=0; i < opts->npoints; i++) {

        point = opts->getRandomPoint();

        EGS_Vector inscribed_point(point);
        first_transform->transform(inscribed_point);

        if (!first_inscribed->isInside(inscribed_point)) {
            continue;
        }
        n_in_inscribed += 1;

        // transform to every location and check which regions in the base it is
        for (size_t sidx = 0; sidx < transforms.size();  sidx++) {

            EGS_Vector transformed(point);
            transforms[sidx]->transform(transformed);
            int base_reg = base->isWhere(transformed);
            bool in_base = base_reg >= 0;

            if (in_base) {
                results.regions_with_inscribed[base_reg].insert(inscribed[sidx]);
                hit_counter[base_reg] += 1;
            }

        }
    }

    results.inscribed_volume = opts->bounds_volume*(double)n_in_inscribed/(int)opts->npoints;

    if (opts->mode != DISCOVERY_ONLY) {
        results.corrected_volumes = applyVolumeCorrections(opts, hit_counter, results.uncorrected_volumes);
    }
    else {
        results.corrected_volumes = results.uncorrected_volumes;
    }

    results.success = true;

    results.time = timer.time();

    return results;

}


bool isGZip(istream &vfile) {
    return (vfile.get() == 0x1f && vfile.get() == 0x8b);
}


void readVolumes(istream &vfile, vector<RegVolume> &reg_volumes, RegionGeomSetT &reg_with_inscribed, vector<EGS_BaseGeometry *> inscribed) {
    int nrecords;
    vfile >> nrecords;
    for (int rec = 0; rec < nrecords; rec++) {
        int reg, ninscribed;
        EGS_Float vol;
        vfile >> reg >> vol >> ninscribed;
        egsInformation("loaded %d/%d %f %d\n", reg, nrecords, vol, ninscribed);
        reg_volumes.push_back(RegVolume(reg, vol));
        for (int i=0; i < ninscribed; i++) {
            int gidx;
            vfile >> gidx;
            reg_with_inscribed[reg].insert(inscribed[gidx]);
        }
    }

}

int loadVolumes(string fname, vector<RegVolume> &reg_volumes, RegionGeomSetT &reg_with_inscribed, vector<EGS_BaseGeometry *> inscribed) {
    ifstream vfile(fname.c_str(), ios::binary);
    if (!vfile.is_open()) {
        return 1;
    }

    if (isGZip(vfile)) {
        vfile.close();
#ifdef HAS_GZSTREAM
        igzstream gzf(fname.c_str());
        readVolumes(gzf, reg_volumes, reg_with_inscribed, inscribed);
        gzf.close();
#else
        egsWarning("Tried to load gzip volume correction file but not compiled with gzstream.\n");
        return 1;
#endif
    }
    else {
        vfile.close();
        ifstream vfile2(fname.c_str(), ios::in);
        readVolumes(vfile2, reg_volumes, reg_with_inscribed, inscribed);
        vfile2.close();
    }

    return 0;

}


/*! \brief Load volume corrections from external file
 * */
VCResults loadFileResults(VCOptions *opts, EGS_BaseGeometry *base,
                          vector<EGS_BaseGeometry *> inscribed, vector<EGS_AffineTransform *> transforms) {

    EGS_Timer timer;
    timer.start();

    VCResults results(opts->vc_file);
    results.uncorrected_volumes =  getUncorrectedVolumes(base);

    vector<RegVolume> reg_volumes;
    int error = loadVolumes(opts->vc_file, reg_volumes, results.regions_with_inscribed, inscribed);

    if (error) {
        egsFatal(
            "loadFileVolumeCorrections: failed to read "
            "volumes from file '%s'\n", opts->vc_file.c_str()
        );
    }
    if (opts->mode != DISCOVERY_ONLY) {
        results.corrected_volumes = applyFileVolumeCorrections(opts, reg_volumes, results.uncorrected_volumes);
    }
    else {
        results.corrected_volumes = results.uncorrected_volumes;
    }
    results.success = true;

    results.time = timer.time();

    return results;

}

}

#endif
