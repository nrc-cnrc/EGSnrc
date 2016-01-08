/*
###############################################################################
#
#  EGSnrc egs++ union geometry headers
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


/*! \file egs_union_geometry.h
 *  \brief A geometry union
 *  \IK
 */

#ifndef EGS_UNION_GEOMETRY_
#define EGS_UNION_GEOMETRY_

#ifdef WIN32

    #ifdef BUILD_UNIONG_DLL
        #define EGS_UNIONG_EXPORT __declspec(dllexport)
    #else
        #define EGS_UNIONG_EXPORT __declspec(dllimport)
    #endif
    #define EGS_UNIONG_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_UNIONG_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_UNIONG_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_UNIONG_EXPORT
        #define EGS_UNIONG_LOCAL
    #endif

#endif


#include "egs_base_geometry.h"

#include<vector>
using std::vector;

/*! \brief A geometry constructed as the union of other geometries.

  \ingroup Geometry
  \ingroup CompositeG

A geometry union is a geometry constructed from geometries
\f$G_1, G_2,..., G_N\f$ so that a point is inside the union if it is
inside any of the constituent geometries. If two or more of the geometries
in the union overlap, the regions and media of the geometry
with the highest priority win. As an example consider two intersecting
spheres forming an union. For the region occupied simultaneously by
both spheres the material in the sphere with the higher priority
will be used. A geometry union is defined as follows:
\verbatim
library = egs_gunion
geometries = list of names of previously defined geometries
priorities = list of integers defining the geometry priorities (optional)
\endverbatim
The \c priorities key is optional. If it is missing, the priority
decreases when moving from the front to the back of the list (\em i.e.
the priority of \f$G_i\f$ is higher than the priority of \f$G_j\f$,
if \f$i < j\f$).
If the priorities of the geometries are explicitely defined via
the \c priorities key and \f$G_i\f$ and \f$G_j\f$ have the same priority
defined, \f$G_i\f$ is still considered to have a higher priority if
\f$i < j\f$. A geometry union is used in the \c car.geom example
geometry file.

*/
class EGS_UNIONG_EXPORT EGS_UnionGeometry : public EGS_BaseGeometry {

public:

    /*! \brief Construct a geometry union from the vector of geometries
    \a geom.
    */
    EGS_UnionGeometry(const vector<EGS_BaseGeometry *> &geoms,
                      const int *priorities = 0, const string &Name = "");

    ~EGS_UnionGeometry();

    bool isRealRegion(int ireg) const {
        if (ireg < 0 || ireg >= nreg) {
            return false;
        }
        int j = ireg/nmax;
        return g[j]->isRealRegion(ireg-j*nmax);
    };

    bool isInside(const EGS_Vector &x) {
        for (int j=0; j<ng; j++) if (g[j]->isInside(x)) {
                return true;
            }
        return false;
    };

    int isWhere(const EGS_Vector &x) {
        for (int j=0; j<ng; j++) {
            int ij = g[j]->isWhere(x);
            if (ij >= 0) {
                return ij + j*nmax;
            }
        }
        return -1;
    };

    int inside(const EGS_Vector &x) {
        return isWhere(x);
    };

    int medium(int ireg) const {
        if (ireg < 0 || ireg >= nreg) {
            return -1;
        }
        int j = ireg/nmax;
        return g[j]->medium(ireg-j*nmax);
    };

    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t,int *newmed=0, EGS_Vector *normal=0) {
        if (ireg >= 0) {
            // we are inside, set current geometry
            int jg = ireg/nmax;
            // distance to boundary and new region in this geometry.
            int inew = g[jg]->howfar(ireg-jg*nmax,x,u,t,newmed,normal);
            // jgnew is the geometry after the step
            int jgnew = inew >= 0 ? jg : -1;
            // geometries are now ordered in decreasing priorities.
            // Then,
            //   - if the particle remains in the current geometry,
            //     we only need to check geometries up to jg-1
            //     For these geometries the particle must be outside,
            //     otherwise it would have been in one of them
            //   - if the particle exits the current geometry, then
            //     we must also check jg+1...ng-1
            for (int j=0; j<jg; j++) {
                int ii = g[j]->howfar(-1,x,u,t,newmed,normal);
                if (ii >= 0) {
                    jgnew = j;
                    inew = ii;
                }
            }
            if (inew < 0) {
                // the particle didn't enter any of the higher priority
                // geometries but exits the current one.
                // => we need to check if the particle is in one
                // of the lower priority geometries at the exit point.
                EGS_Vector xnew(x+u*t);
                for (int j=jg+1; j<ng; j++) {
                    int ii = g[j]->isWhere(xnew);
                    if (ii >= 0) {
                        // when exiting jg, particle is in region ii of geometry j
                        // we don't need to check other geometries because they
                        // have a lower priority.
                        jgnew = j;
                        inew = ii;
                        break;
                    }
                }
            }
            if (inew < 0) {
                return inew;
            }
            if (newmed) {
                *newmed = g[jgnew]->medium(inew);
            }
            return inew + jgnew*nmax;
        }
        // if here, we are currently outside of all geometries in the union.
        int jg, inew=-1;
        for (int j=0; j<ng; j++) {
            int ii = g[j]->howfar(-1,x,u,t,newmed,normal);
            if (ii >= 0) {
                jg = j;
                inew = ii;
            }
        }
        return inew < 0 ? -1 : inew + jg*nmax;
    };

    EGS_Float hownear(int ireg, const EGS_Vector &x) {
        if (ireg >= 0) {
            int jg = ireg/nmax;
            EGS_Float tmin = g[jg]->hownear(ireg-jg*nmax,x);
            if (tmin <= 0) {
                return 0;
            }
            // tmin is now the perpendicular distance to a boundary
            // in the current geometry.
            // we need to check all geometries with higher priorities
            // i.e., all geometries between 0 and jg-1.
            // as their priorities are higher, we know that we are
            // outside of such geometries.
            for (int j=jg-1; j>=0; --j) {
                EGS_Float t = g[j]->hownear(-1,x);
                if (t < tmin) {
                    tmin = t;
                    if (tmin <= 0) {
                        return 0;
                    }
                }
            }
            return tmin;
        }
        // if here, we are outside of all geomtries in the union.
        EGS_Float tmin = 1e30;
        for (int j=ng-1; j>=0; j--) {
            EGS_Float t = g[j]->hownear(-1,x);
            if (t < tmin) {
                tmin = t;
            }
            if (tmin <= 0) {
                return 0;
            }
        }
        return tmin;
    };

    int getMaxStep() const {
        int nstep = 1;
        for (int j=0; j<ng; ++j) {
            nstep += g[j]->getMaxStep();
        }
        return nstep;
    };

    const string &getType() const {
        return type;
    };

    void printInfo() const;

    EGS_Float getRelativeRho(int ireg) const {
        if (ireg < 0 || ireg >= nreg) {
            return 1;
        }
        int jg = ireg/nmax;
        return g[jg]->getRelativeRho(ireg-jg*nmax);
    };
    void setRelativeRho(int start, int end, EGS_Float rho);
    void setRelativeRho(EGS_Input *);

    virtual void getLabelRegions(const string &str, vector<int> &regs);

protected:

    EGS_BaseGeometry **g;     //!< the geometries that form the union.
    int              ng;      //!< number of geometries.
    int              nmax;    //!< max. number of regions in all of the geoms.
    static string    type;    //!< the geometry type

    /*! \brief Don't set media when defining the union.

    This function is re-implemented to warn the user that media should be
    defined for each individual geometry participating in the union, not
    in the union itself.
    */
    void setMedia(EGS_Input *,int,const int *);

};

#endif
