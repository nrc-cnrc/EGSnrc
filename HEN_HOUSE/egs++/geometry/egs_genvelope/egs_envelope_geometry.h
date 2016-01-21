/*
###############################################################################
#
#  EGSnrc egs++ envelope geometry headers
#  Copyright (C) 2015 National Research Council Canada
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
#  Author:          Iwan Kawrakow, 2005
#
#  Contributors:    Ernesto Mainegra-Hing
#                   Frederic Tessier
#
###############################################################################
*/


/*! \file egs_envelope_geometry.h
 *  \brief An envelope geometry: header
 *  \IK
 */

#ifndef EGS_ENVELOPE_GEOMETRY_
#define EGS_ENVELOPE_GEOMETRY_

#ifdef WIN32

    #ifdef BUILD_ENVELOPEG_DLL
        #define EGS_ENVELOPEG_EXPORT __declspec(dllexport)
    #else
        #define EGS_ENVELOPEG_EXPORT __declspec(dllimport)
    #endif
    #define EGS_ENVELOPEG_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_ENVELOPEG_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_ENVELOPEG_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_ENVELOPEG_EXPORT
        #define EGS_ENVELOPEG_LOCAL
    #endif

#endif


#include "egs_base_geometry.h"
#include "egs_functions.h"

#include<vector>
using std::vector;

/*! \brief An envelope geometry class.

\ingroup Geometry
\ingroup CompositeG

An envelope geometry is a geometry consisting of a base geometry \f$G_B\f$
(the envelope) and one or more inscribed geometries \f$G_i, i=1,...,N\f$.
As a very simple example consider a box (the envelope)
and \em e.g. a sphere (the inscribed geometry) inside the box.
There can be much more complex geometries modeled as envelope
geometries and more examples will be given after the details
of the implementation of the various geometry methods:
- A position is considered to be inside an envelope geometry if it
  inside the envelope \f$G_B\f$.
- If the envelope consists of \f$n_B\f$ regions and the inscribed
  geometries have \f$n_1, n_2, ..., n_N\f$ regions with
  \f$n_{\rm max}\f$ denoting the maximum of \f$n_1, n_2, ..., n_N\f$,
  then the region index of a point inside the envelope geometry is
  determined as follows:
    - If the point is outside of all inscribed geometries, the region index
      is given by the region index in the envelope.
    - If the point is inside of the \f$j\f$'th inscribed geometry and the
      local index in this geometry is \f$i_j\f$, then the
      region index is given by \f$n_B + n_{\rm max} (j-1) + i_j\f$. Note that
      this implies that the envelope has \f$n_B + n_{\rm max} N\f$ regions
      instead of \f$n_B + n_1 + \cdots n_N\f$ as one would expect. This
      approach is used because during the simulation only a single global
      region index is kept and in order to determine if the position is
      in the envelope geometry or one of the inscribed geometries it is much
      faster to use such a region indexing scheme
      (if index is less than \f$n_B\f$, then
      the particle is in the envelope, otherwise subtract \f$n_B\f$ and divide
      by \f$n_{\rm max}\f$ to determine the index of the inscribed geometry).
      Such an indexing scheme is not a problem in practice as particles will
      never enter non-existing regions.

- If the position is outside, the howfar() method is simple: just use
  the howfar() method of the envelope. If the position is inside,
  then
   - If the position is in one of the inscribed geometries,
     use the howfar()
     method of this geometry only. If the particle will remain in the inscribed
     geometry, the new region index is determined from the index of the
     inscribed geometry in the list of inscribed geometries and the new region
     index.
     If the particles exits the inscribed geometry, the new region index
     becomes the region index in the envelope.
   - If the particle is not in any
     of the inscribed geometries, then the distance to a boundary is given by
     the minimum of the distances to a boundary of the envelope or any of the
     inscribed geometries. The new region index is either the new region
     in the envelope or determined by the entry region index of the inscribed
     geometry being entered, depending on which distance is smallest.
- The hownear() is simple if the position is outside: just use
  the hownear() method of the envelope. If the position is inside,
  then
    - If the position is inside one of the inscribed geometries,
      \f$t_\perp\f$ is
      given by the hownear() method of this geometry
    - If the position is outside of all inscribed geometries, \f$t_\perp\f$ is
      given by the minimum of the minimum perpendicular distances to the
      envelope and all inscribed geometries.

The above explanation makes it clear that any type of a geometry can be
used as the envelope or an inscribed geometry as long as the logic applies.
The conditions for the applicability of the above logic are:
- Inscribed geometries can not overlap. Undefined behavior will result
  if they do overlap (\em e.g. depending on whether the particle entered
  two overlapping geometries from the first or second geometry,
  the regions and media
  of that geometry will determine the transport as long as the particle
  remains in the geometry)
- The envelope must completely enclose the inscribed geometries.
  If this is not the case and a particle enters an inscribed geometry
  that extends outside of the envelope, the particle will be considered
  inside even if it is outside the envelope for as long as the particle
  remains in the inscribed geometry.

Apart for these two restrictions any geometries can be used. In
particular, neither the envelope nor the inscribed geometries need to
be elementary geometries. More complex examples of the use of this type
of a composite geometry are:
- An XYZ-geometry inscribed in a box of air useful for \em e.g. external
  beam radiotherapy calculations
- A complex geometry such as a brachytherapy seed inscribed in an XYZ geometry.
  One would use this type of geometry if one wanted to calculate the
  complete 3D dose distribution in a given voxel grid for one or
  more radioactive sources.
- A complex geometry representing a detailed model of an ionization chamber
  inscribed in a box of water. This is very useful for dosimetry standards
  simulations.
- Sets of non-concentric cylinders or spheres.

One should keep in mind that an envelope geometry becomes slower with
increasing number of inscribed objects. In situations with many such
objects it may be advantageous to combine several objects into
some other composite geometry (\em e.g. a geometry union, a stack, another
envelope, etc.) before inscribing into the envelope.

An envelope geometry can be defined using the following keys:
\verbatim
library = egs_genvelope
base geometry = name of a previously defined geometry
inscribed geometries = list of names of previously defined geometries
\endverbatim
The above should be self explanatory. Example geometry files making
use of an envelope geometry are <code>car.geom, chambers_in_box.geom,
rz1.geom, seeds_in_xyz.geom</code> and \c seeds_in_xyz1.geom.

*/
class EGS_ENVELOPEG_EXPORT EGS_EnvelopeGeometry : public EGS_BaseGeometry {

public:

    EGS_EnvelopeGeometry(EGS_BaseGeometry *G,
                         const vector<EGS_BaseGeometry *> &geoms, const string &Name = "",
                         bool newindexing=false);

    ~EGS_EnvelopeGeometry();

    bool isRealRegion(int ireg) const {
        if (ireg < 0 || ireg >= nreg) {
            return false;
        }
        if (ireg < nbase) {
            return g->isRealRegion(ireg);
        }
        int jg, ilocal;
        if (new_indexing) {
            jg = reg_to_inscr[ireg-nbase];
            ilocal = ireg - local_start[jg];
        }
        else {
            jg = (ireg - nbase)/nmax;
            ilocal = ireg - nbase - jg*nmax;
        }
        return geometries[jg]->isRealRegion(ilocal);
    };

    bool isInside(const EGS_Vector &x) {
        return g->isInside(x);
    };

    int isWhere(const EGS_Vector &x) {
        int ireg = g->isWhere(x);
        if (ireg < 0) {
            return ireg;
        }
        for (int j=0; j<n_in; j++) {
            int i = geometries[j]->isWhere(x);
            if (i >= 0) return new_indexing ? local_start[j] + i :
                                   nbase + nmax*j + i;
        }
        return ireg;
    };

    int inside(const EGS_Vector &x) {
        return isWhere(x);
    };

    int medium(int ireg) const {
        if (ireg < nbase) {
            return g->medium(ireg);
        }
        int jg, ilocal;
        if (new_indexing) {
            jg = reg_to_inscr[ireg-nbase];
            ilocal = ireg - local_start[jg];
        }
        else {
            jg = (ireg - nbase)/nmax;
            ilocal = ireg - nbase - jg*nmax;
        }
        return geometries[jg]->medium(ilocal);
    };

    int computeIntersections(int ireg, int n, const EGS_Vector &X,
                             const EGS_Vector &u, EGS_GeometryIntersections *isections) {
        if (n < 1) {
            return -1;
        }
        int ifirst = 0;
        EGS_Float t, ttot = 0;
        EGS_Vector x(X);
        int imed;
        if (ireg < 0) {
            t = 1e30;
            ireg = howfar(ireg,x,u,t,&imed);
            if (ireg < 0) {
                return 0;
            }
            isections[0].t = t;
            isections[0].rhof = 1;
            isections[0].ireg = -1;
            isections[0].imed = -1;
            ttot = t;
            ++ifirst;
            x += u*t;
        }
        else {
            imed = medium(ireg);
        }


        int j = ifirst;
        int ij = -1, ig;
        while (1) {
            if (ireg >= nbase) {
                if (new_indexing) {
                    ig = reg_to_inscr[ireg-nbase];
                    ij = ireg - local_start[ig];
                }
                else {
                    ig = (ireg - nbase)/nmax;
                    ij = ireg - nbase - ig*nmax;
                }
            }
            isections[j].imed = imed;
            isections[j].ireg = ireg;
            isections[j].rhof = getRelativeRho(ireg);
            if (ireg < nbase) { // in one of the regions of the base geometry
                t = 1e30;
                int ibase = g->howfar(ireg,x,u,t,&imed);
                ij = -1, ig;
                for (int i=0; i<n_in; i++) {
                    int ireg_i = geometries[i]->howfar(-1,x,u,t,&imed);
                    if (ireg_i >= 0) {
                        ij = ireg_i;
                        ig = i;
                    }
                }
                ttot += t;
                isections[j++].t = ttot;
                if (ij < 0) {
                    ireg = ibase;
                }
                else ireg = new_indexing ? local_start[ig] + ij :
                                nbase + ig*nmax + ij;
                if (ireg < 0) {
                    return j;
                }
                if (j >= n) {
                    return -1;
                }
                x += u*t;
            }
            else {
                int iadd = new_indexing ? local_start[ig] : nbase + ig*nmax;
                int nsec = geometries[ig]->computeIntersections(ij,n-j,
                           x,u,&isections[j]);
                int nm = nsec >= 0 ? nsec+j : n;
                for (int i=j; i<nm; i++) {
                    isections[i].ireg += iadd;
                    isections[i].t += ttot;
                }
                if (nsec < 0) {
                    return nsec;
                }
                j += nsec;
                if (j >= n) {
                    return -1;
                }
                t = isections[j-1].t - ttot;
                x += u*t;
                ttot = isections[j-1].t;
                ireg = g->isWhere(x);
                if (ireg < 0) {
                    return j;
                }
                imed = g->medium(ireg);
            }
        }
        return -1;
    }

    EGS_Float howfarToOutside(int ireg, const EGS_Vector &x,
                              const EGS_Vector &u) {
        if (ireg < 0) {
            return 0;
        }
        EGS_Float d;
        if (ireg < nbase) {
            d = g->howfarToOutside(ireg,x,u);
        }
        else if (g->regions() == 1) {
            d = g->howfarToOutside(0,x,u);
        }
        else {
            int ir = g->isWhere(x);
            d = g->howfarToOutside(ir,x,u);
        }
        return d;
    };

    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t, int *newmed = 0, EGS_Vector *normal = 0) {
        if (ireg >= 0) {
            // inside.
            if (ireg < nbase) {
                // in one of the regions of the base geometry
                // check if we hit a boundary in the base geometry.
                // if we do, newmed and normal get set accordingly.
                int ibase = g->howfar(ireg,x,u,t,newmed,normal);
                int ij = -1, jg;
                // check if we will enter any of the inscribed geometries
                // before entering a new region in the base geometry.
                for (int j=0; j<n_in; j++) {
                    int ireg_j =
                        geometries[j]->howfar(-1,x,u,t,newmed,normal);
                    if (ireg_j >= 0) {
                        // we do. remember the inscribed geometry index
                        // and local region
                        ij = ireg_j;
                        jg = j;
                    }
                }
                if (ij < 0) {
                    return ibase;
                }
                // ij<0 implies that we have not hit any of the
                // inscribed geometries => return the base geometry index.
                // ij>=0 implies that we entered inscribed geometry
                // jg in its local region ij.
                return new_indexing ? local_start[jg] + ij :
                       nbase + jg*nmax + ij;
            }
            // if here, we are in an inscribed geometry.
            // calculate its index (jg) and its local region (ilocal).
            int jg, ilocal;
            if (new_indexing) {
                jg = reg_to_inscr[ireg-nbase];
                ilocal = ireg-local_start[jg];
            }
            else {
                jg = (ireg - nbase)/nmax;
                ilocal = ireg - nbase - jg*nmax;
            }
            // and then check if we will hit a boundary in this geometry.
            int inew = geometries[jg]->howfar(ilocal,x,u,t,newmed,normal);
            if (inew >= 0) return new_indexing ? local_start[jg] + inew :
                                      nbase + jg*nmax + inew;
            // inew >= 0 implies that we either stay in the same
            // region (inew=ilocal) or we entered a new region
            // (inew!=ilocal), which is still inside the inscribed geometry
            // inew<0 implies that we have exited the inscribed geometry
            // => check to see in which base geometry region we are.
            inew = g->isWhere(x+u*t);
            if (inew >= 0 && newmed) {
                *newmed = g->medium(inew);
            }
            return inew;
        }
        // if here, we are outside the base geometry.
        // check to see if we will enter.
        int ienter = g->howfar(ireg,x,u,t,newmed,normal);
        if (ienter >= 0) {
            // yes, we do. see if we are already inside of one of the
            // inscribed geometries.
            for (int j=0; j<n_in; j++) {
                int i = geometries[j]->isWhere(x+u*t);
                if (i >= 0) {
                    // yes, we are.
                    if (newmed) {
                        *newmed = geometries[j]->medium(i);
                    }
                    return new_indexing ? local_start[j] + i :
                           nbase + nmax*j + i;
                }
            }
        }
        return ienter;
    };

    EGS_Float hownear(int ireg, const EGS_Vector &x) {
        if (ireg >= 0) {
            EGS_Float tmin;
            if (ireg < nbase) {  // in one of the regions of the base geom.
                tmin = g->hownear(ireg,x);
                for (int j=0; j<n_in; j++) {
                    EGS_Float tj = geometries[j]->hownear(-1,x);
                    if (tj < tmin) {
                        tmin = tj;
                        if (tmin <= 0) {
                            return tmin;
                        }
                    }
                }
                return tmin;
            }
            int jg, ilocal;
            if (new_indexing) {
                jg = reg_to_inscr[ireg-nbase];
                ilocal = ireg-local_start[jg];
            }
            else {
                jg = (ireg - nbase)/nmax;
                ilocal = ireg - nbase - jg*nmax;
            }
            return geometries[jg]->hownear(ilocal,x);
        }
        return g->hownear(ireg,x);
    };

    int getMaxStep() const {
        int nstep = g->getMaxStep();
        for (int j=0; j<n_in; ++j) {
            nstep += geometries[j]->getMaxStep();
        }
        return nstep+n_in;
    };

    bool hasBooleanProperty(int ireg, EGS_BPType prop) const {
        if (ireg >= 0 && ireg < nreg) {
            if (ireg < nbase) {
                return g->hasBooleanProperty(ireg,prop);
            }
            int jg = (ireg - nbase)/nmax;
            int ilocal = ireg - nbase - jg*nmax;
            return geometries[jg]->hasBooleanProperty(ilocal,prop);
        }
        return false;
    };
    void setBooleanProperty(EGS_BPType) {
        setPropertyError("setBooleanProperty()");
    };
    void addBooleanProperty(int) {
        setPropertyError("addBooleanProperty()");
    };
    void setBooleanProperty(EGS_BPType,int,int,int step=1) {
        setPropertyError("setBooleanProperty()");
    };
    void addBooleanProperty(int,int,int,int step=1) {
        setPropertyError("addBooleanProperty()");
    };

    const string &getType() const {
        return type;
    };

    void printInfo() const;

    void setRelativeRho(int start, int end, EGS_Float rho);
    void setRelativeRho(EGS_Input *);
    EGS_Float getRelativeRho(int ireg) const {
        if (ireg < 0 || ireg >= nreg) {
            return 1;
        }
        if (ireg < nbase) {
            return g->getRelativeRho(ireg);
        }
        int jg = (ireg - nbase)/nmax;
        return geometries[jg]->getRelativeRho(ireg - nbase - jg*nmax);
    };

    virtual void getLabelRegions(const string &str, vector<int> &regs);


protected:

    EGS_BaseGeometry *g;           //!< The envelope geometry
    EGS_BaseGeometry **geometries; //!< The inscribed geometries
    int              n_in;         //!< Number of inscribed geometries
    int              nbase,   //!< Number of regions in the base geometry
                     nmax;    /*!< Max. number of regions in any inscribed
                                     geometry */
    static string    type;    //!< Geometry type

    bool new_indexing;        //!< If true, use new indexing style
    int *reg_to_inscr;        //!< Region to inscribed geometry conversion
    int *local_start;         //!< First region for each inscribed geometry

    /*! \brief Don't set media for an envelope geometry

    This function is re-implemented to warn the user to not set media
    in the envelope geometry. Instead, media should be set for the envelope
    and in the inscribed geometries.
    */
    void setMedia(EGS_Input *,int,const int *);

private:

    void setPropertyError(const char *funcname) {
        egsFatal("EGS_EnvelopeGeometry::%s: don't use this method\n  Define "
                 "properties in the constituent geometries instead\n",
                 funcname);
    };

};


struct EnvelopeAux;

/*! \brief An envelope geometry class.

\ingroup Geometry
\ingroup CompositeG

This class needs to be documented.

*/
class EGS_ENVELOPEG_EXPORT EGS_FastEnvelope : public EGS_BaseGeometry {

public:

    EGS_FastEnvelope(EGS_BaseGeometry *G,
                     const vector<EnvelopeAux *> &fgeoms, const string &Name = "",
                     int newindexing=false);

    ~EGS_FastEnvelope();

    bool isRealRegion(int ireg) const {
        if (ireg < 0 || ireg >= nreg) {
            return false;
        }
        if (ireg < nbase) {
            return g->isRealRegion(ireg);
        }
        int jg, ilocal;
        if (new_indexing) {
            jg = reg_to_inscr[ireg-nbase];
            ilocal = ireg - local_start[jg];
        }
        else {
            jg = (ireg - nbase)/nmax;
            ilocal = ireg - nbase - jg*nmax;
        }
        return geometries[jg]->isRealRegion(ilocal);
    };

    bool isInside(const EGS_Vector &x) {
        return g->isInside(x);
    };

    int isWhere(const EGS_Vector &x) {
        int ireg = g->isWhere(x);
        if (ireg < 0 || n_start[ireg] < 0) {
            return ireg;
        }
        for (int jj=n_start[ireg]; jj<n_start[ireg+1]; jj++) {
            int j = glist[jj];
            int i = geometries[j]->isWhere(x);
            if (i >= 0) {
                return nbase + nmax*j + i;
            }
        }
        return ireg;
    };

    int inside(const EGS_Vector &x) {
        return isWhere(x);
    };

    int medium(int ireg) const {
        if (ireg < nbase) {
            return g->medium(ireg);
        }
        int jg = (ireg - nbase)/nmax;
        int ilocal = ireg - nbase - jg*nmax;
        return geometries[jg]->medium(ilocal);
    };

    int computeIntersections(int ireg, int n, const EGS_Vector &X,
                             const EGS_Vector &u, EGS_GeometryIntersections *isections) {
        if (n < 1) {
            return -1;
        }
        int ifirst = 0;
        EGS_Float t, ttot = 0;
        EGS_Vector x(X);
        int imed;
        //egsInformation("computeIntersections: ireg=%d x=(%g,%g,%g) "
        //     "u=(%g,%g,%g)\n",ireg,x.x,x.y,x.z,u.x,u.y,u.z);
        if (ireg < 0) {
            t = 1e30;
            ireg = howfar(ireg,x,u,t,&imed);
            if (ireg < 0) {
                return 0;
            }
            isections[0].t = t;
            isections[0].rhof = 1;
            isections[0].ireg = -1;
            isections[0].imed = -1;
            ttot = t;
            ++ifirst;
            x += u*t;
            //egsInformation("entered after t=%g in ireg=%d at x=(%g,%g,%g)\n",
            //        t,ireg,x.x,x.y,x.z);
        }
        else {
            imed = medium(ireg);
        }


        int j = ifirst;
        int ij = -1, ig;
        while (1) {
            //egsInformation("in loop: j=%d ireg=%d imed=%d x=(%g,%g,%g)\n",
            //        j,ireg,imed,x.x,x.y,x.z);
            if (ireg >= nbase) {
                ig = (ireg - nbase)/nmax;
                ij = ireg - nbase - ig*nmax;
            }
            isections[j].imed = imed;
            isections[j].ireg = ireg;
            isections[j].rhof = getRelativeRho(ireg);
            if (ireg < nbase) { // in one of the regions of the base geometry
                t = 1e30;
                int ibase = g->howfar(ireg,x,u,t,&imed);
                //egsInformation("In base geometry: t=%g inew=%d\n",t,ibase);
                ij = -1, ig;
                for (int ii=n_start[ireg]; ii<n_start[ireg+1]; ii++) {
                    int i = glist[ii];
                    int ireg_i = geometries[i]->howfar(-1,x,u,t,&imed);
                    if (ireg_i >= 0) {
                        ij = ireg_i;
                        ig = i;
                    }
                }
                ttot += t;
                isections[j++].t = ttot;
                //egsInformation("after inscribed loop: t=%g ij=%d ig=%d"
                //      " ttot=%g\n",t,ij,ig,ttot);
                if (ij < 0) {
                    ireg = ibase;
                }
                else {
                    ireg = nbase + ig*nmax + ij;
                }
                if (ireg < 0) {
                    return j;
                }
                if (j >= n) {
                    return -1;
                }
                x += u*t;
            }
            else {
                int iadd = nbase + ig*nmax;
                int nsec = geometries[ig]->computeIntersections(ij,n-j,
                           x,u,&isections[j]);
                //egsInformation("In inscribed %d: got %d intersections\n",ig,
                //        nsec);
                int nm = nsec >= 0 ? nsec+j : n;
                for (int i=j; i<nm; i++) {
                    isections[i].ireg += iadd;
                    isections[i].t += ttot;
                }
                //egsInformation("last intersection: %g\n",isections[nm-1].t);
                if (nsec < 0) {
                    return nsec;
                }
                j += nsec;
                if (j >= n) {
                    return -1;
                }
                t = isections[j-1].t - ttot;
                x += u*t;
                ttot = isections[j-1].t;
                ireg = g->isWhere(x);
                //egsInformation("new region: %d\n",ireg);
                if (ireg < 0) {
                    return j;
                }
                imed = g->medium(ireg);
            }
        }
        return -1;
    }

    EGS_Float howfarToOutside(int ireg, const EGS_Vector &x,
                              const EGS_Vector &u) {
        if (ireg < 0) {
            return 0;
        }
        EGS_Float d;
        if (ireg < nbase) {
            d = g->howfarToOutside(ireg,x,u);
        }
        else if (g->regions() == 1) {
            d = g->howfarToOutside(0,x,u);
        }
        else {
            int ir = g->isWhere(x);
            d = g->howfarToOutside(ir,x,u);
        }
        return d;
    };

    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t, int *newmed = 0, EGS_Vector *normal = 0) {
        if (ireg >= 0) {
            // inside.
            if (ireg < nbase) {
                // in one of the regions of the base geometry
                // check if we hit a boundary in the base geometry.
                // if we do, newmed and normal get set accordingly.
                int ibase = g->howfar(ireg,x,u,t,newmed,normal);
                int ij = -1, jg;
                // check if we will enter any of the inscribed geometries
                // before entering a new region in the base geometry.
                for (int jj=n_start[ireg]; jj<n_start[ireg+1]; jj++) {
                    int j = glist[jj];
                    int ireg_j =
                        geometries[j]->howfar(-1,x,u,t,newmed,normal);
                    if (ireg_j >= 0) {
                        // we do. remember the inscribed geometry index
                        // and local region
                        ij = ireg_j;
                        jg = j;
                    }
                }
                if (ij < 0) {
                    return ibase;
                }
                // ij<0 implies that we have not hit any of the
                // inscribed geometries => return the base geometry index.
                // ij>=0 implies that we entered inscribed geometry
                // jg in its local region ij.
                return nbase + jg*nmax + ij;
            }
            // if here, we are in an inscribed geometry.
            // calculate its index (jg) and its local region (ilocal).
            int jg = (ireg - nbase)/nmax;
            int ilocal = ireg - nbase - jg*nmax;
            // and then check if we will hit a boundary in this geometry.
            int inew = geometries[jg]->howfar(ilocal,x,u,t,newmed,normal);
            if (inew >= 0) {
                return nbase + jg*nmax + inew;
            }
            // inew >= 0 implies that we either stay in the same
            // region (inew=ilocal) or we entered a new region
            // (inew!=ilocal), which is still inside the inscribed geometry
            // inew<0 implies that we have exited the inscribed geometry
            // => check to see in which base geometry region we are.
            inew = g->isWhere(x+u*t);
            if (inew >= 0 && newmed) {
                *newmed = g->medium(inew);
            }
            return inew;
        }
        // if here, we are outside the base geometry.
        // check to see if we will enter.
        int ienter = g->howfar(ireg,x,u,t,newmed,normal);
        if (ienter >= 0) {
            // yes, we do. see if we are already inside of one of the
            // inscribed geometries.
            //if( n_start[ienter] < 0 ) return ienter;
            for (int jj=n_start[ienter]; jj<n_start[ienter+1]; jj++) {
                int j = glist[jj];
                int i = geometries[j]->isWhere(x+u*t);
                if (i >= 0) {
                    // yes, we are.
                    if (newmed) {
                        *newmed = geometries[j]->medium(i);
                    }
                    return nbase + nmax*j + i;
                }
            }
        }
        return ienter;
    };

    EGS_Float hownear(int ireg, const EGS_Vector &x) {
        if (ireg >= 0) {
            EGS_Float tmin;
            if (ireg < nbase) {  // in one of the regions of the base geom.
                tmin = g->hownear(ireg,x);
                if (tmin <= 0) {
                    return tmin;
                }
                for (int jj=n_start[ireg]; jj<n_start[ireg+1]; jj++) {
                    int j = glist[jj];
                    EGS_Float tj = geometries[j]->hownear(-1,x);
                    if (tj < tmin) {
                        tmin = tj;
                        if (tmin <= 0) {
                            return tmin;
                        }
                    }
                }
                return tmin;
            }
            int jg = (ireg - nbase)/nmax;
            int ilocal = ireg - nbase - jg*nmax;
            return geometries[jg]->hownear(ilocal,x);
        }
        return g->hownear(ireg,x);
    };

    bool hasBooleanProperty(int ireg, EGS_BPType prop) const {
        if (ireg >= 0 && ireg < nreg) {
            if (ireg < nbase) {
                return g->hasBooleanProperty(ireg,prop);
            }
            int jg = (ireg - nbase)/nmax;
            int ilocal = ireg - nbase - jg*nmax;
            return geometries[jg]->hasBooleanProperty(ilocal,prop);
        }
        return false;
    };
    void setBooleanProperty(EGS_BPType) {
        setPropertyError("setBooleanProperty()");
    };
    void addBooleanProperty(int) {
        setPropertyError("addBooleanProperty()");
    };
    void setBooleanProperty(EGS_BPType,int,int,int step=1) {
        setPropertyError("setBooleanProperty()");
    };
    void addBooleanProperty(int,int,int,int step=1) {
        setPropertyError("addBooleanProperty()");
    };

    int getMaxStep() const {
        int nstep = g->getMaxStep();
        for (int j=0; j<n_in; ++j) {
            nstep += geometries[j]->getMaxStep();
        }
        return nstep+n_in;
    };

    const string &getType() const {
        return type;
    };

    void printInfo() const;

    void setRelativeRho(int start, int end, EGS_Float rho);
    void setRelativeRho(EGS_Input *);
    EGS_Float getRelativeRho(int ireg) const {
        if (ireg < 0 || ireg >= nreg) {
            return 1;
        }
        if (ireg < nbase) {
            return g->getRelativeRho(ireg);
        }
        int jg = (ireg - nbase)/nmax;
        return geometries[jg]->getRelativeRho(ireg - nbase - jg*nmax);
    };

    virtual void getLabelRegions(const string &str, vector<int> &regs);


protected:

    EGS_BaseGeometry *g;           //!< The envelope geometry
    EGS_BaseGeometry **geometries; //!< The inscribed geometries
    int              n_in;         //!< Number of inscribed geometries
    int              nbase,   //!< Number of regions in the base geometry
                     nmax;    /*!< Max. number of regions in any inscribed
                                     geometry */
    int              *glist;
    int              *n_start;
    static string    type;    //!< Geometry type

    bool new_indexing;        //!< If true, use new indexing style
    int *reg_to_inscr;        //!< Region to inscribed geometry conversion
    int *local_start;         //!< First region for each inscribed geometry

    /*! \brief Don't set media for an envelope geometry

    This function is re-implemented to warn the user to not set media
    in the envelope geometry. Instead, media should be set for the envelope
    and in the inscribed geometries.
    */
    void setMedia(EGS_Input *,int,const int *);

private:

    void setPropertyError(const char *funcname) {
        egsFatal("EGS_FastEnvelope::%s: don't use this method\n  Define "
                 "properties in the constituent geometries instead\n",
                 funcname);
    };


};

#endif
