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
        rng(NULL), vc_file(""), superset(true), superset_regions(0), sampled_only_regions(0), bounds_leaks(0),
        input(inp), bounds(NULL), sobolAllowed(false) {

        valid=true;

        if (!inp) {
            valid = false;
            return;
        }

        setMode();
        setSuperset();

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

    bool superset; /*!< register a guaranteed superset of base regions (see registerSuperset) */

    // What the superset did, for the report. Kept here and not in VCResults so that
    // the size of VCResults (a member of EGS_AEnvelope) does not change.
    string superset_method; /*!< how the superset of base regions was found ("" if not used) */
    int superset_regions; /*!< number of base regions registered by the superset */
    int sampled_only_regions; /*!< regions found by sampling that the superset did not have */
    EGS_I64 bounds_leaks; /*!< probe points just outside the bounding box that are inside the inscribed geometry */
    vector<EGS_Float> box_size; /*!< x,y,z size of the box around the bounding shape; empty if not known */
    EGS_Vector box_centre; /*!< centre of that box in the frame of the shape, before its transformation */

    /*! The bounding shape (null if volume corrections are read from a file) */
    const EGS_BaseShape *getBounds() const {
        return bounds;
    }

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

    /*! should a superset of base regions be registered? Defaults to yes. */
    void setSuperset() {
        vector<string> choices;
        choices.push_back("no");
        choices.push_back("yes");
        superset = input->getInput("superset", choices, 1);
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

        // The box used for the superset of base regions: the bounding shape itself
        // if it is a box, or the box around a cylinder or a sphere. EGS_CylinderShape
        // keeps its midpoint and axis in the transformation of the shape, which
        // placedBox() applies; EGS_SphereShape adds its midpoint itself, so it is
        // put in box_centre.
        string shape_type;
        box_size.clear();
        box_centre = EGS_Vector(0,0,0);
        if (!shape_inp->getInput("type", shape_type)) {
            vector<EGS_Float> mid;
            EGS_Float radius, height;
            if (shape_type == "box") {
                if (shape_inp->getInput("box size", box_size) || (box_size.size() != 3 && box_size.size() != 1)) {
                    box_size.clear();
                }
                else if (box_size.size() == 1) {
                    box_size.resize(3, box_size[0]);
                }
            }
            else if (shape_type == "cylinder") {
                if (!shape_inp->getInput("radius", radius) && !shape_inp->getInput("height", height)) {
                    box_size.push_back(2*radius);
                    box_size.push_back(2*radius);
                    box_size.push_back(height);
                }
            }
            else if (shape_type == "sphere") {
                if (!shape_inp->getInput("radius", radius)) {
                    box_size.resize(3, 2*radius);
                }
            }
            if (box_size.size() == 3 && shape_type == "sphere" &&
                    !shape_inp->getInput("midpoint", mid) && mid.size() == 3) {
                box_centre = EGS_Vector(mid[0], mid[1], mid[2]);
            }
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
            if (options->superset_method != "") {
                egsInformation(" Superset of base regions    = %s, %d regions\n",
                               options->superset_method.c_str(), options->superset_regions);
                egsInformation(" Regions found by sampling only = %d\n", options->sampled_only_regions);
                egsInformation(" Probes outside bounding box inside geometry = %lld\n",
                               (long long)options->bounds_leaks);
            }
            else {
                egsInformation(" Superset of base regions    = not used\n");
            }
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

/*! \brief Centre, unit axes and half sizes, in the frame of the base geometry, of the
 * bounding box placed at the location given by \a loc. */
void placedBox(const VCOptions *opts, const EGS_AffineTransform *loc,
               EGS_Vector &c, EGS_Vector e[3], EGS_Float h[3]) {
    const EGS_AffineTransform *ts = opts->getBounds()->getTransform();
    EGS_Vector pts[4];
    pts[0] = opts->box_centre;
    pts[1] = opts->box_centre + EGS_Vector(0.5*opts->box_size[0], 0, 0);
    pts[2] = opts->box_centre + EGS_Vector(0, 0.5*opts->box_size[1], 0);
    pts[3] = opts->box_centre + EGS_Vector(0, 0, 0.5*opts->box_size[2]);
    for (int i=0; i<4; i++) {
        if (ts) {
            ts->transform(pts[i]);
        }
        loc->transform(pts[i]);
    }
    c = pts[0];
    for (int i=0; i<3; i++) {
        e[i] = pts[i+1] - c;
        h[i] = e[i].length();
        e[i] *= 1/h[i];
    }
}

/*! Range of indices i in [0,n) of the intervals [b(i),b(i+1)] of direction \a dir of
 * \a base that overlap [lo,hi]. Returns false if there are none. */
bool overlappedRange(EGS_BaseGeometry *base, int dir, EGS_Float lo, EGS_Float hi, int &i0, int &i1) {
    int n = base->getNRegDir(dir);
    i0 = -1;
    i1 = -1;
    for (int i=0; i<n; i++) {
        if (base->getBound(dir,i+1) >= lo && base->getBound(dir,i) <= hi) {
            if (i0 < 0) {
                i0 = i;
            }
            i1 = i;
        }
    }
    return i0 >= 0;
}

/*! \brief Register, for every location, every base region that the bounding box of
 * the inscribed geometry could overlap.
 *
 * Random points only find a base region if one of them lands in the part of the
 * inscribed geometry that is in that region, so a region holding a thin sliver of
 * the inscribed geometry can be missed however many points are used. A particle in
 * a missed region is never tested against the inscribed geometry. This function
 * instead registers a superset: all base regions that the bounding box touches.
 * Registering a region that turns out not to contain the inscribed geometry only
 * costs one extra howfar call there.
 *
 * It needs the bounding shape to be a box, a cylinder or a sphere (the box
 * around the latter two is used), and it needs to enumerate the base
 * regions near a box, which it can do for EGS_XYZGeometry, EGS_RZ and EGS_cSpheres
 * (through getBound/getNRegDir) and for any base geometry with few regions (all of
 * them are registered). Otherwise \a method is left empty and nothing is registered.
 *
 * The bounding box must enclose the inscribed geometry: see boundsLeaks().
 */
int registerSuperset(VCOptions *opts, EGS_BaseGeometry *base, vector<EGS_BaseGeometry *> inscribed,
                     vector<EGS_AffineTransform *> transforms, RegionGeomSetT &regs, string &method) {

    const int SMALL_BASE = 64; // register everywhere if the base has at most this many regions
    const EGS_Float tol = 1e-8; // cm; widens every test so that touching counts as overlapping

    method = "";
    if (!opts->superset || opts->box_size.size() != 3) {
        return 0;
    }

    string type = base->getType();
    bool xyz = type == "EGS_XYZGeometry";
    bool rz = type == "EGS_RZ";
    bool sph = type == "EGS_cSpheres";
    bool small = base->regions() <= SMALL_BASE;
    if (small) {
        method = "all regions of a small base geometry";
    }
    else if (xyz) {
        method = "voxels touching the bounding box";
    }
    else if (rz) {
        method = "rings out to the bounding box";
    }
    else if (sph) {
        method = "shells out to the bounding box";
    }
    else {
        return 0;
    }

    set<int> all;
    for (size_t sidx = 0; sidx < transforms.size(); sidx++) {

        if (small) {
            for (int ir=0; ir<base->regions(); ir++) {
                regs[ir].insert(inscribed[sidx]);
                all.insert(ir);
            }
            continue;
        }

        EGS_Vector c, e[3];
        EGS_Float h[3];
        placedBox(opts, transforms[sidx], c, e, h);

        // the 8 corners and their extent along x, y, z and away from the z-axis
        EGS_Vector corner[8];
        EGS_Float lo[3], hi[3], rmax = 0;
        for (int k=0; k<8; k++) {
            corner[k] = c + e[0]*(k&1 ? h[0] : -h[0]) + e[1]*(k&2 ? h[1] : -h[1]) + e[2]*(k&4 ? h[2] : -h[2]);
            EGS_Float v[3] = {corner[k].x, corner[k].y, corner[k].z};
            for (int d=0; d<3; d++) {
                if (!k || v[d] < lo[d]) {
                    lo[d] = v[d];
                }
                if (!k || v[d] > hi[d]) {
                    hi[d] = v[d];
                }
            }
            EGS_Float r = sqrt(v[0]*v[0] + v[1]*v[1]);
            if (r > rmax) {
                rmax = r;
            }
        }

        if (xyz) {
            int i0[3], i1[3];
            bool any = true;
            for (int d=0; d<3; d++) {
                any = any && overlappedRange(base, d, lo[d]-tol, hi[d]+tol, i0[d], i1[d]);
            }
            if (!any) {
                continue;
            }
            for (int ix=i0[0]; ix<=i1[0]; ix++) {
                EGS_Float x0 = base->getBound(0,ix), x1 = base->getBound(0,ix+1);
                for (int iy=i0[1]; iy<=i1[1]; iy++) {
                    EGS_Float y0 = base->getBound(1,iy), y1 = base->getBound(1,iy+1);
                    for (int iz=i0[2]; iz<=i1[2]; iz++) {
                        EGS_Float z0 = base->getBound(2,iz), z1 = base->getBound(2,iz+1);
                        // The voxel lies within the sphere of radius d about its
                        // centre p. If that sphere does not reach the box, skip it:
                        // this trims the corners of the extent of a tilted box.
                        EGS_Vector p(0.5*(x0+x1), 0.5*(y0+y1), 0.5*(z0+z1));
                        EGS_Float d = 0.5*sqrt((x1-x0)*(x1-x0) + (y1-y0)*(y1-y0) + (z1-z0)*(z1-z0));
                        EGS_Vector q(p - c);
                        if (fabs(q*e[0]) > h[0] + d + tol || fabs(q*e[1]) > h[1] + d + tol ||
                                fabs(q*e[2]) > h[2] + d + tol) {
                            continue;
                        }
                        int ir = base->isWhere(p);
                        if (ir >= 0) {
                            regs[ir].insert(inscribed[sidx]);
                            all.insert(ir);
                        }
                    }
                }
            }
        }
        else if (rz) {
            // EGS_RZ: direction 0 is z, direction 1 is r, about the z-axis through
            // the origin. Every ring from the axis out to the farthest corner is
            // registered (the nearest point of the box to the axis is not looked for).
            int z0, z1, r0, r1;
            if (!overlappedRange(base, 0, lo[2]-tol, hi[2]+tol, z0, z1) ||
                    !overlappedRange(base, 1, 0, rmax+tol, r0, r1)) {
                continue;
            }
            for (int iz=z0; iz<=z1; iz++) {
                EGS_Float zm = 0.5*(base->getBound(0,iz) + base->getBound(0,iz+1));
                for (int irad=r0; irad<=r1; irad++) {
                    EGS_Float rm = 0.5*(base->getBound(1,irad) + base->getBound(1,irad+1));
                    int ir = base->isWhere(EGS_Vector(rm, 0, zm));
                    if (ir >= 0) {
                        regs[ir].insert(inscribed[sidx]);
                        all.insert(ir);
                    }
                }
            }
        }
        else {
            // concentric shells: the farthest point of a box from the centre is a
            // corner, so the outermost shell touched is that of a corner. A corner
            // outside the geometry means all shells.
            int last = 0;
            for (int k=0; k<8; k++) {
                int ir = base->isWhere(corner[k]);
                if (ir < 0) {
                    last = base->regions() - 1;
                    break;
                }
                if (ir > last) {
                    last = ir;
                }
            }
            for (int ir=0; ir<=last; ir++) {
                regs[ir].insert(inscribed[sidx]);
                all.insert(ir);
            }
        }
    }

    return all.size();
}

/*! \brief Check that the bounding box encloses the inscribed geometry.
 *
 * Probes a grid of points just outside each face of the bounding box and returns how
 * many of them are inside the inscribed geometry. A count above zero means the
 * inscribed geometry sticks out of the box through that face, so that neither random
 * points nor the superset can find the base regions it occupies out there.
 */
EGS_I64 boundsLeaks(VCOptions *opts, EGS_BaseGeometry *first_inscribed, EGS_AffineTransform *first_transform,
                    EGS_I64 per_face[6]) {
    const int NG = 64; // probe points per face: NG*NG
    const EGS_Float out = 1e-6; // cm outside the face
    EGS_I64 total = 0;
    for (int f=0; f<6; f++) {
        per_face[f] = 0;
    }
    if (opts->box_size.size() != 3) {
        return 0;
    }
    EGS_Vector c, e[3];
    EGS_Float h[3];
    placedBox(opts, first_transform, c, e, h);
    for (int f=0; f<6; f++) {
        int a = f/2, b = (a+1)%3, d = (a+2)%3;
        EGS_Float sgn = f%2 ? 1 : -1;
        for (int i=0; i<NG; i++) {
            for (int j=0; j<NG; j++) {
                EGS_Vector p = c + e[a]*(sgn*(h[a] + out))
                               + e[b]*(h[b]*(2*(i+0.5)/NG - 1)) + e[d]*(h[d]*(2*(j+0.5)/NG - 1));
                if (first_inscribed->isInside(p)) {
                    per_face[f]++;
                    total++;
                }
            }
        }
    }
    return total;
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

    // the inscribed geometry must not stick out of the bounding box
    EGS_I64 per_face[6];
    opts->bounds_leaks = boundsLeaks(opts, first_inscribed, first_transform, per_face);
    if (opts->bounds_leaks > 0) {
        egsWarning("\nVolCor: the bounding shape does not enclose the inscribed geometry '%s'.\n"
                   "  Probe points just outside the bounding box that are inside the geometry,\n"
                   "  by face (-x,+x,-y,+y,-z,+z of the box): %lld %lld %lld %lld %lld %lld\n"
                   "  Base regions holding the part outside the box will not be found.\n\n",
                   first_inscribed->getName().c_str(),
                   (long long)per_face[0], (long long)per_face[1], (long long)per_face[2],
                   (long long)per_face[3], (long long)per_face[4], (long long)per_face[5]);
    }

    // base regions that the bounding box could overlap, at every location
    RegionGeomSetT &regs = results.regions_with_inscribed;
    opts->superset_regions = registerSuperset(opts, base, inscribed, transforms, regs, opts->superset_method);
    size_t n_superset = results.regions_with_inscribed.size();
    if (opts->superset_method != "") {
        egsInformation("VolCor: superset region discovery (%s): %d base regions registered\n",
                       opts->superset_method.c_str(), opts->superset_regions);
    }
    else {
        egsInformation("VolCor: superset region discovery not used (%s)\n",
                       !opts->superset ? "switched off" :
                       opts->box_size.size() != 3 ? "the bounding shape is not a box, cylinder or sphere" :
                       "base geometry type not supported");
    }

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
    opts->sampled_only_regions = opts->superset_method == "" ? 0 :
                                 (int)(results.regions_with_inscribed.size() - n_superset);

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
