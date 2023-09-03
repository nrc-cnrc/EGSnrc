/*
###############################################################################
#
#  EGSnrc egs++ auto envelope geometry
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


/*! \file egs_autoenvelope.cpp
 *  \brief A fast envelope geometry (based on EGS_FastEnvelope) with automatic region detection
 *  \author Randle Taylor (randle.taylor@gmail.com)
 */

#include "egs_input.h"
#include "egs_autoenvelope.h"
#include "../egs_gtransformed/egs_gtransformed.h"

#include <cstdlib>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <fstream>

#ifdef HAS_SOBOL
    #include "sobol.h"
#endif


string EGS_AENVELOPE_LOCAL EGS_AEnvelope::type = "EGS_AEnvelope";
string EGS_AENVELOPE_LOCAL EGS_ASwitchedEnvelope::type = "EGS_ASwitchedEnvelope";
/* only geometries that support getting regional volume can be used as phantoms */
const string EGS_AENVELOPE_LOCAL EGS_AEnvelope::allowed_base_geom_types[] = {"EGS_cSpheres", "EGS_cSphericalShell", "EGS_XYZGeometry", "EGS_RZ"};

static char EGS_AENVELOPE_LOCAL geom_class_msg[] = "createGeometry(AEnvelope): %s\n";
static char EGS_AENVELOPE_LOCAL base_geom_keyword[] = "base geometry";
static char EGS_AENVELOPE_LOCAL inscribed_geom_keyword[] = "inscribed geometry";
static char EGS_AENVELOPE_LOCAL inscribed_geom_name_keyword[] = "inscribed geometry name";
static char EGS_AENVELOPE_LOCAL transformations_keyword[] = "transformations";
static char EGS_AENVELOPE_LOCAL type_keyword[] = "type";
static char EGS_AENVELOPE_LOCAL transformation_keyword[] = "transformation";


EGS_AEnvelope::EGS_AEnvelope(EGS_BaseGeometry *base_geom,
                             const vector<AEnvelopeAux> inscribed, const string &Name, bool debug, string output_vc_file) :
    EGS_BaseGeometry(Name), base_geom(base_geom), debug_info(debug), output_vc(output_vc_file) {

    bool volcor_available = allowedBaseGeomType(base_geom->getType());
    bool volcor_requested = inscribed.size() > 0 && inscribed[0].vcopts->mode == CORRECT_VOLUME;

    if (!volcor_available && volcor_requested) {

        string msg(
            "EGS_AEnvelope:: Volume correction is not available for geometry type '%s (%s)'. "
            "Geometry types must implement getVolume.  Valid choices are:\n\t"
        );

        int end = (int)(sizeof(allowed_base_geom_types)/sizeof(string));
        for (int i=0; i < end; i++) {
            msg += allowed_base_geom_types[i] + " ";
        }
        msg += "\nPlease set `action = discover -or- correct and zero volume` or use a different base geometry type.\n";
        egsFatal(msg.c_str(), base_geom->getType().c_str(), base_geom->getName().c_str());
    }


    base_geom->ref();
    nregbase = base_geom->regions();
    is_convex = base_geom->isConvex();
    has_rho_scaling = getHasRhoScaling();

    ninscribed = inscribed.size();
    if (ninscribed == 0) {
        egsFatal("EGS_AEnvelope: no inscribed geometries!\n");
    }

    // nreg is total regions in geometry = nregbase + sum(nreginscribed_j)
    nreg = nregbase;

    // create a transformed copy of the inscribed geometry
    int global = nreg;
    for (int geom_idx=0; geom_idx < ninscribed; geom_idx++) {

        EGS_BaseGeometry *inscribed_geom = inscribed[geom_idx].geom;
        EGS_AffineTransform *transform = inscribed[geom_idx].transform;
        VCOptions *vcopt = inscribed[geom_idx].vcopts;
        EGS_TransformedGeometry *transformed = new EGS_TransformedGeometry(inscribed_geom, *transform);

        inscribed_geoms.push_back(transformed);
        transforms.push_back(transform);
        opts.push_back(vcopt);

        int ninscribed_reg= transformed->regions();
        nreg += ninscribed_reg;

        for (int lreg = 0; lreg < ninscribed_reg; lreg++) {
            GeomRegPairT local(transformed, lreg);
            local_to_global_reg[local] = global;
            global_reg_to_local[global] = local;
            global++;
        }
    }

    geoms_in_region = new vector<EGS_BaseGeometry *>[nregbase];

    if (inscribed[0].vcopts->vc_file != "") {
        vc_results = loadFileResults(inscribed[0].vcopts, base_geom, inscribed_geoms, transforms);
        egsInformation("loaded from %s\n", inscribed[0].vcopts->vc_file.c_str());
    }
    else {
        // now run volume correction and figure out which regions have inscribed geometries
        vc_results = findRegionsWithInscribed(inscribed[0].vcopts, base_geom, inscribed_geoms, transforms);
    }


    nreg_with_inscribed = 0;
    for (volcor::RegionGeomSetT::iterator it=vc_results.regions_with_inscribed.begin();
            it!=vc_results.regions_with_inscribed.end(); it++) {
        nreg_with_inscribed++;
        copy(it->second.begin(), it->second.end(), back_inserter(geoms_in_region[it->first]));
    }

    if (getNRegWithInscribed() == 0) {
        egsFatal("EGS_AEnvelope:: Failed to find any regions with inscribed geometries\n");
    }

    if (output_vc=="yes" || output_vc=="text" || output_vc=="gzip") {
        writeVolumeCorrection();
    }

    if (debug_info) {
        printInfo();
    }

}


EGS_AEnvelope::~EGS_AEnvelope() {
    if (!base_geom->deref()) {
        delete base_geom;
    }
}

bool EGS_AEnvelope::allowedBaseGeomType(const string &geom_type) {
    // Check if the input geometry is one that aenvelope can handle

    int end = (int)(sizeof(allowed_base_geom_types)/sizeof(string));

    for (int i=0; i<end; i++) {
        if (allowed_base_geom_types[i] == geom_type) {
            return true;
        }
    }

    return false;
}

int EGS_AEnvelope::getGlobalRegFromLocalReg(EGS_BaseGeometry *g, int local_reg) {
    return getGlobalRegFromLocal(volcor::GeomRegPairT(g, local_reg));
}

int EGS_AEnvelope::getGlobalRegFromLocal(const volcor::GeomRegPairT local) const {

    map<volcor::GeomRegPairT, int>::const_iterator glob_it = local_to_global_reg.find(local);
    if (glob_it != local_to_global_reg.end()) {
        return (*glob_it).second;
    }
    return -1;
}

volcor::GeomRegPairT EGS_AEnvelope::getLocalFromGlobalReg(int ireg) const {

    map<int, volcor::GeomRegPairT>::const_iterator glob_it = global_reg_to_local.find(ireg);
    if (glob_it != global_reg_to_local.end()) {
        return (*glob_it).second;
    }
    return volcor::GeomRegPairT();
}

int EGS_AEnvelope::getNRegWithInscribed() const {
    return nreg_with_inscribed;
}

bool EGS_AEnvelope::isRealRegion(int ireg) const {

    bool is_outside =  ireg < 0 || ireg >= nreg;
    if (is_outside) {
        return false;
    }

    bool in_base_geom = ireg < nregbase;
    if (in_base_geom) {
        return base_geom->isRealRegion(ireg);
    }

    volcor::GeomRegPairT local;
    local = getLocalFromGlobalReg(ireg);
    return local.first->isRealRegion(local.second);

};


bool EGS_AEnvelope::isInside(const EGS_Vector &x) {
    return base_geom->isInside(x);
};


int EGS_AEnvelope::isWhere(const EGS_Vector &x) {

    int base_reg = base_geom->isWhere(x);

    // which inscribed geometries are in this region
    vector<EGS_BaseGeometry *> geoms_in_reg = getGeomsInRegion(base_reg);

    if (base_reg < 0 || geoms_in_reg.size()==0) {
        return base_reg;
    }

    // loop over all inscribed geometries in this region and check
    // if position is inside any of them
    for (vector<EGS_BaseGeometry *>::iterator geom=geoms_in_reg.begin(); geom!= geoms_in_reg.end(); ++geom) {
        int inscribed_reg = (*geom)->isWhere(x);
        if (inscribed_reg >= 0) {
            return getGlobalRegFromLocalReg(*geom, inscribed_reg);
        }
    }

    // not in any of the inscribed geometries so must be in envelope region
    return base_reg;
};

int EGS_AEnvelope::inside(const EGS_Vector &x) {
    return isWhere(x);
};

int EGS_AEnvelope::medium(int ireg) const {

    if (ireg < nregbase) {
        return base_geom->medium(ireg);
    }

    volcor::GeomRegPairT local = getLocalFromGlobalReg(ireg);

    return local.first->medium(local.second);
};

vector<EGS_BaseGeometry *> EGS_AEnvelope::getGeomsInRegion(int ireg) {
    if (ireg < 0 || ireg >= nregbase) {
        vector<EGS_BaseGeometry *> empty;
        return empty;
    }
    return geoms_in_region[ireg];
};

int EGS_AEnvelope::computeIntersections(int ireg, int n, const EGS_Vector &X,
                                        const EGS_Vector &u, EGS_GeometryIntersections *isections) {

    // For a given position x, direction u and region number ireg, this
    // method returns the number of intersections with the geometry and
    // distances, medium indeces and relative mass densities for all
    // intersections, or, if this number is larger than the size n of
    // isections, -1 (but the n intersections are still put in isections).
    // If the position is outside, the method checks if the trajectory
    // intersects the geometry and if yes, puts in the first element of
    // isections the distance to the entry point and then finds all other
    // intersections as in the case of x inside.

    if (n < 1) {
        return -1;
    }

    int isec_idx = 0; // current intersection index
    EGS_Float t;        // distance to next boundary
    EGS_Float ttot = 0; // total distance along path
    EGS_Vector x(X); // current position

    int imed;

    bool outside_envelope = ireg < 0;
    bool in_base_geom = ireg >= 0 && ireg < nregbase;

    if (outside_envelope) {
        t = 1e30;

        // region we would hit
        ireg = howfar(ireg,x,u,t,&imed);
        bool would_not_intersect = ireg < 0;

        if (would_not_intersect) {
            return 0;
        }

        isections[0].t = t; // distance to entry point
        isections[0].rhof = 1;
        isections[0].ireg = -1;
        isections[0].imed = -1;
        ttot = t;

        x += u*t;

        isec_idx = 1;
    }
    else {
        // we're already inside the envelope
        imed = medium(ireg);
    }

    // keep looping until we exit the geometry!
    while (1) {
        //egsInformation("in loop: j=%d ireg=%d imed=%d x=(%g,%g,%g)\n",
        //        j,ireg,imed,x.x,x.y,x.z);
        //

        // why are we setting these here?
        // on first pass if we were outside envelope then:
        //   ireg  is set to entry region
        //   imed  is set to med of entry region (set in howfar above)
        // else
        //   ireg is region of input ireg (current region?)
        //   imed is medium of input ireg (current region?)
        isections[isec_idx].imed = imed; //
        isections[isec_idx].ireg = ireg;
        isections[isec_idx].rhof = getRelativeRho(ireg);

        if (in_base_geom) { // in one of the regions of the base geometry

            t = 1e30;

            // next region will be in base geom by default
            ireg = base_geom->howfar(ireg,x,u,t,&imed);

            // if there's inscribed geoms in this region see if we will be intersecting one
            vector<EGS_BaseGeometry *> geoms_in_reg = getGeomsInRegion(ireg);
            for (vector<EGS_BaseGeometry *>::iterator geom=geoms_in_reg.begin(); geom!= geoms_in_reg.end(); ++geom) {

                int inscribed_reg = (*geom)->howfar(-1, x, u, t, &imed);

                bool hits_inscribed = inscribed_reg >= 0;

                if (hits_inscribed) {
                    ireg = getGlobalRegFromLocalReg(*geom, inscribed_reg);
                }
            }

            ttot += t;
            isections[isec_idx++].t = ttot;

            // left the geometry (implies that no inscribed geoms hit
            if (ireg < 0) {
                return isec_idx;
            }


            // over limit of number of intersections
            if (isec_idx >= n) {
                return -1;
            }

            // move to next boundary
            x += u*t;

        }
        else {

            // in inscribed geometry
            volcor::GeomRegPairT local = getLocalFromGlobalReg(ireg);

            int max_isec = n - isec_idx;

            int ninter_sec = local.first->computeIntersections(local.second, max_isec,
                             x, u, &isections[isec_idx]);

            // convert intersection region indexes to global indexex
            int nmax = ninter_sec >= 0 ? ninter_sec + isec_idx : n;
            for (int i=isec_idx; i < nmax; i++) {
                isections[i].ireg = getGlobalRegFromLocalReg(local.first, isections[i].ireg);
                isections[i].t += ttot;
            }

            //egsInformation("last intersection: %g\n",isections[nm-1].t);
            if (ninter_sec < 0) {
                return ninter_sec;
            }

            isec_idx += ninter_sec;

            if (isec_idx >= n) {
                return -1;
            }

            t = isections[isec_idx-1].t - ttot;
            x += u*t;
            ttot = isections[isec_idx-1].t;
            ireg = base_geom->isWhere(x);
            //egsInformation("new region: %d\n",ireg);

            if (ireg < 0) {
                return isec_idx;
            }

            imed = base_geom->medium(ireg);
        }
    }
    return -1;
}

EGS_Float EGS_AEnvelope::howfarToOutside(int ireg, const EGS_Vector &x, const EGS_Vector &u) {

    if (ireg < 0) {
        return 0;
    }

    EGS_Float d;

    bool in_base_geom = ireg < nregbase;
    if (in_base_geom) {
        d = base_geom->howfarToOutside(ireg, x, u);
    }
    else if (base_geom->regions() == 1) {
        d = base_geom->howfarToOutside(0, x, u);
    }
    else {
        int ir = base_geom->isWhere(x);
        d = base_geom->howfarToOutside(ir,x,u);
    }
    return d;
};


int EGS_AEnvelope::howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
                          EGS_Float &t, int *newmed, EGS_Vector *normal) {

    bool inside_geom = ireg >= 0;
    if (inside_geom) {

        bool inside_base_geom = ireg < nregbase;

        if (inside_base_geom) {

            int base_reg = base_geom->howfar(ireg,x,u,t,newmed,normal);
            vector<EGS_BaseGeometry *> geoms_in_reg = getGeomsInRegion(ireg);

            bool no_inscribed_in_reg = geoms_in_reg.size() == 0;
            if (no_inscribed_in_reg) {
                return base_reg;
            }

            int inscribed_global = -1;
            for (vector<EGS_BaseGeometry *>::iterator geom=geoms_in_reg.begin(); geom!= geoms_in_reg.end(); ++geom) {
                int local_reg = (*geom)->howfar(-1, x, u, t, newmed, normal);
                bool hits_inscribed = local_reg >= 0;
                if (hits_inscribed) {
                    inscribed_global = getGlobalRegFromLocalReg(*geom, local_reg);
                }
            }

            bool hit_inscribed_first = inscribed_global >= 0;

            if (hit_inscribed_first) {
                return inscribed_global;
            }

            return base_reg;
        }
        else {

            // if here, we are in an inscribed geometry.
            volcor::GeomRegPairT local = getLocalFromGlobalReg(ireg);

            // and then check if we will hit a boundary in this geometry.
            int new_local = local.first->howfar(local.second, x, u, t, newmed, normal);
            bool hit_boundary_in_inscribed = new_local >=0;
            if (hit_boundary_in_inscribed) {
                return getGlobalRegFromLocalReg(local.first, new_local);
            }

            // new_local < 0 implies that we have exited the inscribed geometry
            // => check to see in which base geometry region we are.
            int inew = base_geom->isWhere(x + u*t);
            if (inew >= 0 && newmed) {
                *newmed = base_geom->medium(inew);
            }
            return inew;
        }
    }

    // if here, we are outside the base geometry.
    // check to see if we will enter.
    int new_base_reg = base_geom->howfar(ireg,x,u,t,newmed,normal);
    vector<EGS_BaseGeometry *> geoms_in_reg = getGeomsInRegion(new_base_reg);
    bool has_geoms_in_reg = geoms_in_reg.size() > 0;

    //cout << "new reg "<<new_base_reg << " has geoms " << has_geoms_in_reg << " t "<<t<<endl;
    if (new_base_reg >= 0 && has_geoms_in_reg) {

        // check if we will be inside an inscribed geometry when we enter
        for (vector<EGS_BaseGeometry *>::iterator geom=geoms_in_reg.begin(); geom!= geoms_in_reg.end(); ++geom) {
            int new_local_reg = (*geom)->isWhere(x+u*t);
            bool landed_in_inscribed = new_local_reg >= 0;
            if (landed_in_inscribed) {
                if (newmed) {
                    *newmed = (*geom)->medium(new_local_reg);
                }
                return getGlobalRegFromLocalReg(*geom, new_local_reg);
            }

        }
    }

    return new_base_reg;
};


EGS_Float EGS_AEnvelope::hownear(int ireg, const EGS_Vector &x) {
    bool inside_envelope = ireg >= 0;

    if (!inside_envelope) {
        return base_geom->hownear(ireg, x);
    }

    bool in_base_geom =  ireg < nregbase ;
    if (!in_base_geom) {
        volcor::GeomRegPairT local = getLocalFromGlobalReg(ireg);
        return local.first->hownear(local.second, x);
    }

    EGS_Float tmin = base_geom->hownear(ireg, x);
    if (tmin <= 0) {
        return tmin;
    }
    vector<EGS_BaseGeometry *> geoms_in_reg = getGeomsInRegion(ireg);
    for (vector<EGS_BaseGeometry *>::iterator geom=geoms_in_reg.begin(); geom!= geoms_in_reg.end(); ++geom) {
        EGS_Float local_t = (*geom)->hownear(-1, x);
        if (local_t < tmin) {
            tmin = local_t;
            if (tmin < 0) {
                return tmin;
            }
        }
    }
    return tmin;
};

bool EGS_AEnvelope::hasBooleanProperty(int ireg, EGS_BPType prop) const {
    if (ireg >= 0 && ireg < nreg) {
        if (ireg < nregbase) {
            return base_geom->hasBooleanProperty(ireg,prop);
        }
        volcor::GeomRegPairT local = getLocalFromGlobalReg(ireg);
        return local.first->hasBooleanProperty(local.second, prop);
    }
    return false;
};

void EGS_AEnvelope::setBooleanProperty(EGS_BPType prop) {
    setPropertyError("setBooleanProperty()");
};

void EGS_AEnvelope::addBooleanProperty(int bit) {
    setPropertyError("addBooleanProperty()");
};

void EGS_AEnvelope::setBooleanProperty(EGS_BPType prop, int start, int end,int step) {
    setPropertyError("setBooleanProperty()");
};

void EGS_AEnvelope::addBooleanProperty(int bit,int start,int end,int step) {
    setPropertyError("addBooleanProperty()");
};

int EGS_AEnvelope::getMaxStep() const {
    int nstep = base_geom->getMaxStep();
    for (size_t j=0; j< inscribed_geoms.size(); ++j) {
        nstep += inscribed_geoms[j]->getMaxStep();
    }
    return nstep + inscribed_geoms.size();
};

EGS_Float EGS_AEnvelope::getVolume(int ireg) {

    if (ireg < 0) {
        return -1;
    }
    else if (ireg < nregbase) {
        return vc_results.corrected_volumes[ireg];
    }

    volcor::GeomRegPairT local = global_reg_to_local[ireg];

    return local.first->getVolume(local.second);
};

EGS_Float EGS_AEnvelope::getCorrectionRatio(int ireg) {

    if (0 <= ireg && ireg < nregbase) {
        return vc_results.corrected_volumes[ireg]/vc_results.uncorrected_volumes[ireg];
    }

    return 1;
};


/* Print information about the geometry. If `print debug info = yes` is present
 * in the geometry, extra information about which regions of the base geometry
 * contain inscribed geometries and how their volumes were corrected.
*/
void EGS_AEnvelope::printInfo() const {

    EGS_BaseGeometry::printInfo();

    if (debug_info) {

        for (int ir=0; ir < nregbase; ir++) {
            if (fabs(vc_results.uncorrected_volumes[ir] - vc_results.corrected_volumes[ir]) > 1E-8) {
                egsInformation("    volume of region %d was corrected from %.5E g to %.5E g\n",
                               ir, vc_results.uncorrected_volumes[ir], vc_results.corrected_volumes[ir]);
            }
        }

        for (int ir=0; ir < nregbase; ir++) {
            if (geoms_in_region[ir].size() > 0) {
                egsInformation("    region %d has %d inscribed geometries\n", ir, geoms_in_region[ir].size());
            }
        }
    }

    vc_results.outputResults();

    egsInformation(" base geometry = %s (type %s) nreg=%d\n",base_geom->getName().c_str(),
                   base_geom->getType().c_str(), nregbase, base_geom->regions());
    egsInformation(" # of inscribed geometries= %d in %d regions\n",inscribed_geoms.size(), getNRegWithInscribed());


    egsInformation("=======================================================\n");
}

void EGS_AEnvelope::writeVCToFile(ostream &out) {

    vector<int> to_write;

    for (int i=0; i < base_geom->regions(); i++) {
        EGS_Float cor =  vc_results.corrected_volumes[i];
        EGS_Float uncor =  vc_results.uncorrected_volumes[i];
        bool has_correction = fabs(cor-uncor) > 1E-8;
        if (has_correction) {
            to_write.push_back(i);
        }
    }

    size_t nrecords = to_write.size();
    out << nrecords << endl;

    for (size_t i=0; i<to_write.size(); i++) {
        int ir = to_write[i];
        EGS_Float cor =  vc_results.corrected_volumes[ir];

        // find out the indexes of geometries inscribed in this region (if any)
        vector<EGS_BaseGeometry *> geoms_in_reg = getGeomsInRegion(ir);
        vector<int> geom_idxs_in_reg;
        for (size_t i=0; i<geoms_in_reg.size(); i++) {
            size_t pos = find(inscribed_geoms.begin(), inscribed_geoms.end(), geoms_in_reg[i]) - inscribed_geoms.begin();
            if (pos < inscribed_geoms.size()) {
                geom_idxs_in_reg.push_back(pos);
            }
        }

        int ninscribed = (int)geom_idxs_in_reg.size();

        out << ir << " " << cor;
        // write number of and indexes of geometries inscribed in this region
        out << " " << ninscribed;
        for (size_t gidx = 0; gidx < geom_idxs_in_reg.size(); gidx++) {
            out << " " <<geom_idxs_in_reg[gidx];
        }
        out << endl;
    }

}

void EGS_AEnvelope::writeVolumeCorrection() {

    egsInformation("\nVolume Correction Output File \n%s\n", string(80,'=').c_str());

    string gname = base_geom->getName();
    string fname = gname+".autoenv.volcor";
    fname += (output_vc == "gzip" ?  ".gz" : "");
    string mode  = (output_vc == "gzip" ?  "gzip" : "text");

    egsInformation(
        "Writing volume correction file for %s to %s file %s\n",
        gname.c_str(), mode.c_str(), fname.c_str());

    if (output_vc == "gzip") {
#ifdef HAS_GZSTREAM
        ogzstream outgz(fname.c_str());
        writeVCToFile(outgz);
        outgz.close();
#endif
    }
    else {
        ofstream out(fname.c_str());
        writeVCToFile(out);
        out.close();
    }

}


/* check if the base or any of inscribed geometries have density scaling on */
bool EGS_AEnvelope::getHasRhoScaling() {

    has_rho_scaling = base_geom->hasRhoScaling();
    if (has_rho_scaling) {
        return true;
    }

    for (size_t geom_idx=0; geom_idx < inscribed_geoms.size(); geom_idx++) {
        if (inscribed_geoms[geom_idx]->hasRhoScaling()) {
            return true;
        }
    }
    return false;

}


void EGS_AEnvelope::setMedia(EGS_Input *,int,const int *) {
    egsWarning("EGS_AEnvelope::setMedia: don't use this method. Use the\n"
               " setMedia() methods of the geometry objects that make up this geometry\n");
}


void EGS_AEnvelope::setRelativeRho(int start, int end, EGS_Float rho) {
    setRelativeRho(0);
}


void EGS_AEnvelope::setRelativeRho(EGS_Input *) {
    egsWarning("EGS_AEnvelope::setRelativeRho(): don't use this method."
               " Use the\n setRelativeRho methods of the geometry objects that make up"
               " this geometry\n");
}


EGS_Float EGS_AEnvelope::getRelativeRho(int ireg) const {
    if (ireg < 0 || ireg >= nreg) {
        return 1;
    }
    if (ireg < nregbase) {
        EGS_Float v = base_geom->getRelativeRho(ireg);
        return v;
    }
    volcor::GeomRegPairT local = getLocalFromGlobalReg(ireg);
    if (local.first) {
        return local.first->getRelativeRho(local.second);
    }
    return 1;
};


/*************************************************************************/
/* Switched envelope *****************************************************/
/*************************************************************************/

EGS_ASwitchedEnvelope::EGS_ASwitchedEnvelope(EGS_BaseGeometry *base_geom,
        const vector<AEnvelopeAux> inscribed, const string &Name, bool debug, string output_vc_file):
    EGS_AEnvelope(base_geom, inscribed, Name, debug, output_vc_file) {

    // activate a single geometry
    cur_ptr = 0;
    active_inscribed.push_back(inscribed_geoms[cur_ptr]);

};


//TODO: this gets called a lot and is probably quite slow.  Instead fo doing a
//set intersection on every call we can probably do it once when activated
//geometries change and cache it
vector<EGS_BaseGeometry *> EGS_ASwitchedEnvelope::getGeomsInRegion(int ireg) {
    // find which geometries are present AND active in this region
    vector<EGS_BaseGeometry *> active_in_reg;
    if ((0 <= ireg) && (ireg < nregbase)) {
        set_intersection(
            geoms_in_region[ireg].begin(), geoms_in_region[ireg].end(),
            active_inscribed.begin(), active_inscribed.end(),
            back_inserter(active_in_reg)
        );
    }

    return active_in_reg;

}


void EGS_ASwitchedEnvelope::setActiveGeometries(vector<EGS_BaseGeometry *> geoms) {
    active_inscribed.clear();
    active_inscribed = geoms;
    cur_ptr = -1;
    if (geoms.size() <= 0) {
        return;
    }

    for (size_t i=0; i < inscribed_geoms.size(); i++) {
        if (inscribed_geoms[i] == geoms[0]) {
            cur_ptr = i;
            return;
        }
    }
}


void EGS_ASwitchedEnvelope::setActiveGeometries(vector<int> geom_indexes) {

    active_inscribed.clear();

    for (size_t i = 0; i < geom_indexes.size(); i++) {
        int idx =  geom_indexes[i];
        if (idx < 0 || idx >= ninscribed) {
            egsFatal("EGS_ASwitchedEnvelope:: %d is not a valid geometry index\n", idx);
        }
        active_inscribed.push_back(inscribed_geoms[i]);
    }

    cur_ptr = geom_indexes[0];

}


bool EGS_ASwitchedEnvelope::hasActiveGeom(int ireg) {

    if ((0 <= ireg) && (ireg < nregbase)) {
        size_t nactive = getGeomsInRegion(ireg).size();
        return nactive > 0;
    }

    return false;
}


bool EGS_ASwitchedEnvelope::hasInactiveGeom(int ireg) {

    if ((0 <= ireg) && (ireg < nregbase)) {
        size_t nactive = getGeomsInRegion(ireg).size();
        size_t ntotal = EGS_AEnvelope::getGeomsInRegion(ireg).size();
        return nactive < ntotal;
    }

    return false;

}

void EGS_ASwitchedEnvelope::setActiveByIndex(int inscribed_index) {
    vector<EGS_BaseGeometry *> to_activate;
    to_activate.push_back(inscribed_geoms[inscribed_index]);
    setActiveGeometries(to_activate);
    cur_ptr = inscribed_index;
}

void EGS_ASwitchedEnvelope::activateByIndex(int inscribed_index) {
    active_inscribed.push_back(inscribed_geoms[inscribed_index]);
}

void EGS_ASwitchedEnvelope::deactivateByIndex(int inscribed_index) {
    vector<EGS_BaseGeometry *>::iterator loc = find(
                active_inscribed.begin(), active_inscribed.end(),
                inscribed_geoms[inscribed_index]
            );
    if (loc != active_inscribed.end()) {
        active_inscribed.erase(loc);
    }
}

void EGS_ASwitchedEnvelope::cycleActive() {
    cur_ptr = cur_ptr == ninscribed -1 ? 0 : cur_ptr + 1;
    vector<EGS_BaseGeometry *> to_activate;
    to_activate.push_back(inscribed_geoms[cur_ptr]);
    setActiveGeometries(to_activate);
}


vector<EGS_AffineTransform *> EGS_AEnvelope::createTransforms(EGS_Input *input) {

    vector<EGS_AffineTransform *> transforms;
    if (input) {
        EGS_Input *i;

        while ((i = input->takeInputItem(transformation_keyword))) {
            EGS_AffineTransform *transform = EGS_AffineTransform::getTransformation(i);
            if (!transform) {
                egsWarning("Invalid transform input given\n");

            }
            transforms.push_back(transform);
            delete i;

        }
    }

    return transforms;

}


extern "C" {

    EGS_AENVELOPE_EXPORT EGS_BaseGeometry *createGeometry(EGS_Input *input) {

        if (!input) {
            egsWarning(geom_class_msg, "null input");
            return 0;
        }

        bool debug;
        vector<string> debug_choices;
        debug_choices.push_back("no");
        debug_choices.push_back("yes");
        debug = input->getInput("print debug info", debug_choices, 0);

        int output_vc_file_choice;
        vector<string> vc_file_choices;
        vc_file_choices.push_back("no");
        vc_file_choices.push_back("yes");
        vc_file_choices.push_back("text");
        vc_file_choices.push_back("gzip");
        output_vc_file_choice = input->getInput("output volume correction file", vc_file_choices, 0);
        string output_vc_file = vc_file_choices[output_vc_file_choice];

#ifndef HAS_GZSTREAM
        if (output_vc_file == "gzip") {
            egsWarning(
                "GZip file output requested but not compiled with gzstream.\n"
                "Please recompile with gzstream support.\n"
            );
            return 0;
        }
#endif



        string base_geom_name;
        int err = input->getInput(base_geom_keyword, base_geom_name);
        if (err) {
            egsWarning(geom_class_msg, ("'"+string(base_geom_keyword)+"' input not found").c_str());
            return 0;
        }

        string type;
        err = input->getInput(type_keyword, type);
        if (err) {
            type = "AEnvelope";
        }

        EGS_BaseGeometry *base_geom = EGS_BaseGeometry::getGeometry(base_geom_name);
        if (!base_geom) {
            egsWarning(geom_class_msg, ("Unable to find geometry '"+base_geom_name+"'").c_str());
            return 0;
        }

        EGS_Input *inscribed_input = input->takeInputItem(inscribed_geom_keyword);
        if (!inscribed_input) {
            egsWarning(geom_class_msg, ("Missing '"+string(inscribed_geom_keyword)+"' input item").c_str());
            return 0;
        }

        string inscribed_geom_name;
        err = inscribed_input->getInput(inscribed_geom_name_keyword, inscribed_geom_name);
        if (err) {
            egsWarning(geom_class_msg, ("'"+string(inscribed_geom_name_keyword)+"' input not found").c_str());
            return 0;
        }

        EGS_BaseGeometry *inscribed_geom = EGS_BaseGeometry::getGeometry(inscribed_geom_name);
        if (!inscribed_geom) {
            egsWarning(geom_class_msg, ("Unable to find geometry '"+inscribed_geom_name+"'").c_str());
            return 0;
        }

        vector<EGS_AffineTransform *> transforms;
        EGS_Input *trans_input = inscribed_input->takeInputItem(transformations_keyword);
        if (trans_input) {
            transforms = EGS_AEnvelope::createTransforms(trans_input);
        }

        delete trans_input;

        EGS_Input *volcor_input = inscribed_input->takeInputItem("region discovery");
        VCOptions *vcopts = new VCOptions(volcor_input);
        if (!vcopts->valid) {
            egsWarning(geom_class_msg, "Missing or invalid 'region discovery' input item");
            return 0;
        }

        delete inscribed_input;


        vector<AEnvelopeAux> inscribed;
        if (transforms.size()>0) {
            for (size_t i=0; i < transforms.size(); i++) {
                inscribed.push_back(AEnvelopeAux(inscribed_geom, transforms[i], vcopts));
            }
        }
        else {
            EGS_AffineTransform *unityt = new EGS_AffineTransform();
            inscribed.push_back(AEnvelopeAux(inscribed_geom, unityt, vcopts));
        }

        EGS_BaseGeometry *result;
        if (type == "EGS_ASwitchedEnvelope") {
            result = new EGS_ASwitchedEnvelope(base_geom, inscribed, "", (bool)debug, output_vc_file);
        }
        else {
            result = new EGS_AEnvelope(base_geom, inscribed, "", (bool)debug, output_vc_file);
        }

        result->setName(input);
        return result;
    }

}
