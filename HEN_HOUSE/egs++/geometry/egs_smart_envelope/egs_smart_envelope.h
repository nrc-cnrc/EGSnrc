/*
###############################################################################
#
#  EGSnrc egs++ smart envelope geometry headers
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
#  Author:          Iwan Kawrakow, 2008
#
#  Contributors:    Frederic Tessier
#
###############################################################################
#
#  A "smart" envelope geometry. The smartness is due to the fact that there
#  can be only zero or one inscribed geometry per base geometry region, which
#  makes many of the checks faster.
#
#  In addition, unlike the regular envelope geometry where all inscribed
#  geometries always must completely fit inside the base geometry, here
#  geometries can be inscribed using logic 0 or 1.
#
#  Logic 0 is as before, i.e., geometry must completely fit into the region
#  where it is being inscribed.
#
#  Logic 1 means that the inscribed geometry extends beyond the region and
#  therefore the smart envelope also checks the base geometry in this case.
#  This is very similar to a CD geometry except that now the space outside
#  the inscribed geometry is still part of the geometry.
#
#  Warning: not completely tested, so don't use for production runs yet.
#
###############################################################################
*/


/*! \file egs_smart_envelope.h
 *  \brief A smart envelope geometry: header
 *  \IK
 */

#ifndef EGS_SMART_ENVELOPE_
#define EGS_SMART_ENVELOPE_

#ifdef WIN32

    #ifdef BUILD_SMART_ENVELOPE_DLL
        #define EGS_SMART_ENVELOPE_EXPORT __declspec(dllexport)
    #else
        #define EGS_SMART_ENVELOPE_EXPORT __declspec(dllimport)
    #endif
    #define EGS_SMART_ENVELOPE_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_SMART_ENVELOPE_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_SMART_ENVELOPE_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_SMART_ENVELOPE_EXPORT
        #define EGS_SMART_ENVELOPE_LOCAL
    #endif

#endif


#include "egs_base_geometry.h"
#include "egs_functions.h"

#include<vector>
using std::vector;

/*! \brief A "smart" envelope geometry class.

\ingroup Geometry
\ingroup CompositeG

A "smart" envelope geometry is a geometry consisting of a base geometry \f$G_B\f$
(the envelope) with 0 or 1 geometry inscribed into the \f$n_B\f$ regions of the
envelope. There are two possible ways to inscribe a geometry into a given region
of the \f$G_B\f$
  - Using logic type 0: in this case, the smart envelope geometry assumes that
    the inscribed geometry completely fits into the region of \f$G_B\f$
    where it is being inscribed.
  - Using logic type 1: in this case, the inscribed geometry may extend beyond
    the \f$G_B\f$ region, but it will be cut to the portion within the region.
    This is very similar to the CD geometry (see the EGS_CDGeometry class), except
    that now the space outside of the inscribed geometry is still part of the geometry.
Note that logic type 0 is a subset of logic type 1. The only reason for having
logic type 0 is speed: if the position is inside an inscribed geometry and it
is known that this geometry completely fits into the corresponding region of the
envelope, then one does not need to check for intersections with the envelope.
Any geometry can be used as the envelope and any geometry can be inscribed.
A smart envelope geometry is defined as follows:
\verbatim
library = egs_smart_envelope
base geometry = name of a previously defined geometry
inscribe geometry = geometry_name1 region1 [logic]
inscribe geometry = geometry_name2 region2 [logic]
...
\endverbatim
The "inscribe geometry" input can be repeated as often as needed.
<code>logic</code> is 0 or 1 and may be missing (0 is assumed if missing).

The convention for region indexing in a smart envelope is as follows:
The \f$n_B\f$ regions of \f$G_B\f$ are regions \f$0...n_B-1\f$ of the
smart envelope geometry. Regions \f$n_B+j,j=0,...,n_1-1\f$ are the \f$n_1\f$
regions of the first inscribed geometry, regions \f$n_B+n_1+j,j=0,...,n_2-1\f$
are the \f$n_2\f$ regions of the second inscribed geometry, etc.
Note that inscribed geometries are processed in the order they are
defined by the user and not in increasing order of base geometry index.
For instance, if the user first inscribed the geometry \f$G_1\f$
into region 5 of \f$G_B\f$ and then geometry \f$G_2\f$ into region 1, then
the regions of \f$G_1\f$ will come before the regions of \f$G_2\f$
in the list of smart envelope regions.
*/

//#include "egs_functions.h"

struct SmartEnvelopeAux;

class EGS_SMART_ENVELOPE_EXPORT EGS_SmartEnvelope : public EGS_BaseGeometry {

public:

    EGS_SmartEnvelope(EGS_BaseGeometry *G,
                      const vector<SmartEnvelopeAux *> &fgeoms, const string &Name = "");

    ~EGS_SmartEnvelope();

    bool isInside(const EGS_Vector &x) {
        return g->isInside(x);
    };

    int isWhere(const EGS_Vector &x) {
        int ibase = g->isWhere(x);
        if (ibase < 0) {
            return ibase;
        }
        int j = gindex[ibase];
        if (j >= 0) {
            int i = geometries[j]->isWhere(x);
            if (i >= 0) {
                ibase = local_start[j] + i;
            }
        }
        return ibase;
    };

    int inside(const EGS_Vector &x) {
        return isWhere(x);
    };

    bool isRealRegion(int ireg) const {
        if (ireg < nbase) {
            return g->isRealRegion(ireg);
        }
        int i = ireg - nbase;
        int j = reg_to_inscr[i];
        return geometries[j]->isRealRegion(ireg-local_start[j]);
    };

    int medium(int ireg) const {
        if (ireg < nbase) {
            return g->medium(ireg);
        }
        int i = ireg - nbase;
        int j = reg_to_inscr[i];
        return geometries[j]->medium(ireg-local_start[j]);
    };

    EGS_Float howfarToOutside(int ireg, const EGS_Vector &x,
                              const EGS_Vector &u) {
        if (ireg < 0) {
            return 0;
        }
        int ibase = ireg < nbase ? ireg : reg_to_base[ireg-nbase];
        return g->howfarToOutside(ibase,x,u);
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
                int j = gindex[ireg];
                if (j >= 0) {
                    // there is an inscribed geometry in this region
                    // => check if we will enter
                    int inscr = geometries[j]->howfar(-1,x,u,t,newmed,normal);
                    if (inscr >= 0)
                        // yes, we do.
                    {
                        return local_start[j] + inscr;
                    }
                }
                if (ibase == ireg || ibase < 0) {
                    return ibase;
                }
                // if here, we enter a new base geometry region.
                j = gindex[ibase];
                if (j >= 0 && itype[j]) {
                    // geometry inscribed in the new base region
                    // using inscription type 1 => check if already
                    // inside the inscribed
                    int inscr = geometries[j]->isWhere(x+u*t);
                    if (inscr >= 0) {
                        if (newmed) {
                            *newmed = geometries[j]->medium(inscr);
                        }
                        return local_start[j] + inscr;
                    }
                }
                return ibase;
            }
            // if here, we are in an inscribed geometry.
            int i = ireg-nbase;
            int ibase = reg_to_base[i];
            int j = reg_to_inscr[i];
            int ilocal = ireg-local_start[j];
            int inew = geometries[j]->howfar(ilocal,x,u,t,newmed,normal);
            if (itype[j]) {
                int ibase_new = g->howfar(ibase,x,u,t,newmed,normal);
                if (ibase_new != ibase) {
                    if (ibase_new < 0) {
                        return ibase_new;
                    }
                    j = gindex[ibase_new];
                    if (j >= 0 && itype[j]) {
                        int inscr = geometries[j]->isWhere(x+u*t);
                        if (inscr >= 0) {
                            if (newmed) {
                                *newmed = geometries[j]->medium(inscr);
                            }
                            return local_start[j] + inscr;
                        }
                    }
                    return ibase_new;
                }
            }
            if (inew < 0) {
                if (newmed) {
                    *newmed = g->medium(ibase);
                }
                return ibase;
            }
            return local_start[j] + inew;
        }
        // if here, we are outside the base geometry.
        // check to see if we will enter.
        int ibase = g->howfar(ireg,x,u,t,newmed,normal);
        if (ibase >= 0) {
            int j = gindex[ibase];
            if (j >= 0 && itype[j]) {
                int inscr = geometries[j]->isWhere(x+u*t);
                if (inscr >= 0) {
                    if (newmed) {
                        *newmed = geometries[j]->medium(inscr);
                    }
                    return local_start[j] + inscr;
                }
            }
        }
        return ibase;
    };

    EGS_Float hownear(int ireg, const EGS_Vector &x) {
        if (ireg >= 0) {
            EGS_Float tmin;
            if (ireg < nbase) {  // in one of the regions of the base geom.
                tmin = g->hownear(ireg,x);
                if (tmin <= 0) {
                    return tmin;
                }
                int j = gindex[ireg];
                if (j >= 0) {
                    EGS_Float ti = geometries[j]->hownear(-1,x);
                    if (ti < tmin) {
                        tmin = ti;
                    }
                }
                return tmin;
            }
            int i = ireg-nbase;
            int j = reg_to_inscr[i];
            int ilocal = ireg-local_start[j];
            tmin = geometries[j]->hownear(ilocal,x);
            if (itype[j]) {
                int ibase = reg_to_base[i];
                EGS_Float tbase = g->hownear(ibase,x);
                if (tbase < tmin) {
                    tmin = tbase;
                }
            }
            return tmin;
        }
        return g->hownear(ireg,x);
    };

    int getMaxStep() const {
        int nstep = g->getMaxStep() + n_in;
        for (int j=0; j<n_in; ++j) {
            nstep += geometries[j]->getMaxStep();
        }
        return nstep;
    };

    bool hasBooleanProperty(int ireg, EGS_BPType prop) const {
        if (ireg >= 0 && ireg < nreg) {
            if (ireg < nbase) {
                return g->hasBooleanProperty(ireg,prop);
            }
            int i = ireg-nbase, j = reg_to_inscr[i];
            return geometries[j]->hasBooleanProperty(ireg-local_start[j],prop);
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
        int i = ireg-nbase;
        int j = reg_to_inscr[i];
        return geometries[j]->getRelativeRho(ireg-local_start[j]);
    };

    virtual void getLabelRegions(const string &str, vector<int> &regs);


protected:

    EGS_BaseGeometry *g;           //!< The envelope geometry
    EGS_BaseGeometry **geometries; //!< The inscribed geometries
    int              *gindex;      //!< Index of inscribed geometries
    int *reg_to_inscr;        //!< Region to inscribed geometry conversion
    int *reg_to_base;         //!< Region to base region conversion
    int *local_start;         //!< First region for each inscribed geometry
    char             *itype;
    int              n_in;         //!< Number of inscribed geometries
    int              nbase;   //!< Number of regions in the base geometry

    static string    type;    //!< Geometry type

    /*! \brief Don't set media for an envelope geometry

    This function is re-implemented to warn the user to not set media
    in the envelope geometry. Instead, media should be set for the envelope
    and in the inscribed geometries.
    */
    void setMedia(EGS_Input *,int,const int *);

private:

    void setPropertyError(const char *funcname) {
        egsFatal("EGS_SmartEnvelope::%s: don't use this method\n  Define "
                 "properties in the constituent geometries instead\n",
                 funcname);
    };


};

#endif
