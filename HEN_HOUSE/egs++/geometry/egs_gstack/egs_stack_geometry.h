/*
###############################################################################
#
#  EGSnrc egs++ stack geometry headers
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
#  Contributors:    Frederic Tessier
#
###############################################################################
*/


/*! \file egs_stack_geometry.h
 *  \brief A geometry stack: header
 *  \IK
 */

#ifndef EGS_STACK_GEOMETRY_
#define EGS_STACK_GEOMETRY_

#ifdef WIN32

    #ifdef BUILD_STACKG_DLL
        #define EGS_STACKG_EXPORT __declspec(dllexport)
    #else
        #define EGS_STACKG_EXPORT __declspec(dllimport)
    #endif
    #define EGS_STACKG_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_STACKG_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_STACKG_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_STACKG_EXPORT
        #define EGS_STACKG_LOCAL
    #endif

#endif


#include "egs_base_geometry.h"
#include "egs_functions.h"

#include<vector>
using std::vector;

/*! \brief A stack of geometries

\ingroup Geometry
\ingroup CompositeG

A stack of geometries is conceptually similar to a
\link EGS_UnionGeometry geometry union \endlink except
that additional restrictions are made, which permit a more efficient
implementation of the various geometry methods:
- Geometries in a stack should not overlap. This way there is no need
  for a priority and all related checks.
- A particle outside of a geometry stack can only enter the stack
  from the first or last geometry in the list of geometries.
- When a particle exits the \f$j\f$'th geometry in a stack, it can only enter
  the \f$j+1\f$'st or \f$j-1\f$'st geometry.

Due to these limitations, a geometry stack is less general and therefore
presumably less useful than a geometry union. However, if the geometry
under consideration is such that it can be represented as a stack of
geometries, the simulation may be substantially more efficient compared
to a union, particularly when there are many geometries in the stack.
A typical example of a geometry stack is the geometry package of the
BEAMnrc code, which is built from "component modules" (corresponding
to the geometry objects in this implementation), put together
one after the other. The EGS_StackGeometry class is of course
much more versatile than the BEAMnrc geometry as it permits to have
an arbitrary number of geometries of arbitrary type, dynamically
determined at run time from the user input, instead of a fixed
set of component modules determined when the BEAMnrc accelerator
is built and compiled.

A geometry stack is defined in the input file using the following keys:
\verbatim
library = egs_gstack
geometries = list of names of previously defined geometries
tolerance = small floating number
\endverbatim
The tolerance key defines a small floating point number \f$\epsilon\f$.
Each time a particle exits a geometry, its position is moved by
\f$\epsilon\f$ along its direction of motion. This is needed to avoid
numericall roundoff problems that may result in the particle not having
entered the next geometry in the stack and therefore being discarded.
*/
class EGS_StackGeometry : public EGS_BaseGeometry {

public:

    /*! \brief Construct a geometry stack from the vector of geometries
    \a geom.
    */
    EGS_StackGeometry(const vector<EGS_BaseGeometry *> &geoms,
                      EGS_Float Tol=1e-4, const string &Name = "");

    ~EGS_StackGeometry();

    bool isRealRegion(int ireg) const {
        if (ireg < 0 || ireg >= nreg) {
            return false;
        }
        int jg = ireg/nmax;
        int jl = ireg - jg*nmax;
        return g[jg]->isRealRegion(jl);
    };

    bool isInside(const EGS_Vector &x) {
        for (int j=0; j<ng; j++) if (g[j]->isInside(x)) {
                return true;
            }
        return false;
    };

    int isWhere(const EGS_Vector &x) {
        for (int j=0; j<ng; j++) {
            int i = g[j]->isWhere(x);
            if (i >= 0) {
                return nmax*j + i;
            }
        }
        return -1;
    };

    int inside(const EGS_Vector &x) {
        return isWhere(x);
    };

    int medium(int ireg) const {
        int j = ireg/nmax;
        return g[j]->medium(ireg-j*nmax);
    };

    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t, int *newmed=0, EGS_Vector *normal=0) {
        if (ireg >= 0) {
            // inside the geometry.
            int jg = ireg/nmax;
            int jl = ireg - jg*nmax;
            // jg is the stack index, jl the local region index in jg.
            // see if we hit a boundary in jg.
            int inew = g[jg]->howfar(jl,x,u,t,newmed,normal);
            // inew>=0 implies that we either stay in the same local region
            // or enter a new region still inside of jg => simply calculate
            // the new gloabal region and return
            if (inew >= 0) {
                return jg*nmax + inew;
            }
            // inew < 0 implies that we have exited jg.
            // to prevent roundoff problems, we add eps to the path-length
            // to the boundary of jg.
            t += eps;
            if (jg > 0) {
                // check if we enter the jg-1'th geometry in the stack.
                inew = g[jg-1]->howfar(-1,x,u,t,newmed,normal);
                if (inew >= 0) {
                    return (jg-1)*nmax + inew;    // yes, we do.
                }
            }
            if (jg < ng-1) {
                // check if we enter the jg+1'th geometry in the stack.
                inew = g[jg+1]->howfar(-1,x,u,t,newmed,normal);
                if (inew >= 0) {
                    return (jg+1)*nmax + inew;    // yes, we do
                }
            }
            // if here, we don't enter either of the "stack neighbours"
            // => we exit the geometry.
            t -= eps;
            return -1;
        }
        int i1 = g[0]->howfar(-1,x,u,t,newmed,normal);
        int i2 = g[ng-1]->howfar(-1,x,u,t,newmed,normal);
        if (i2 >= 0) {
            return (ng-1)*nmax + i2;
        }
        if (i1 >= 0) {
            return i1;
        }
        return -1;
    };

    EGS_Float hownear(int ireg, const EGS_Vector &x) {
        if (ireg >= 0) {
            int jg = ireg/nmax;
            return g[jg]->hownear(ireg-jg*nmax,x);
        }
        EGS_Float t1 = g[0]->hownear(-1,x);
        EGS_Float t2 = g[ng-1]->hownear(-1,x);
        if (t2 < t1) {
            return t2;
        }
        return t1;
    };

    bool hasBooleanProperty(int ireg, EGS_BPType prop) const {
        if (ireg >= 0 && ireg < nreg) {
            int jg = ireg/nmax;
            return g[jg]->hasBooleanProperty(ireg-jg*nmax,prop);
        }
        return false;
    };
    void setBooleanProperty(EGS_BPType) {
        setPropertError("setBooleanProperty()");
    };
    void addBooleanProperty(int) {
        setPropertError("addBooleanProperty()");
    };
    void setBooleanProperty(EGS_BPType,int,int,int step=1) {
        setPropertError("setBooleanProperty()");
    };
    void addBooleanProperty(int,int,int,int step=1) {
        setPropertError("addBooleanProperty()");
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

    int ng;
    int nmax;
    EGS_BaseGeometry **g;
    EGS_Float  eps;
    static string    type;

    void setMedia(EGS_Input *,int,const int *);

    vector<label> stack_labels;

private:

    void setPropertError(const char *funcname) {
        egsFatal("EGS_StackGeometry::%s: don't use this method\n  Define "
                 "properties in the constituent geometries instead\n");
    };


};

#endif
