/*
###############################################################################
#
#  EGSnrc egs++ lattice geometry headers
#  Copyright (C) 2019 Rowan Thomson and Martin Martinov
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
#  Author:          Martin Martinov, 2019
#
#  Contributors:    Frederic Tessier
#                   Ernesto Mainegra-Hing
#
###############################################################################
#
#  When egs_lattice is used for publications, please cite the following paper:
#
#  Martinov, Martin P., and Rowan M. Thomson. Taking EGSnrc to new lows:
#  Development of egs++ lattice geometry and testing with microscopic
#  geometries. Medical Physics 47, 3225-3232 (2020).
#
###############################################################################
*/


/*! \file egs_lattice.h
 *  \brief Lattice geometries: header
*/

#include "egs_base_geometry.h"
#include "../egs_gtransformed/egs_gtransformed.h"

#ifndef EGS_LATTICE_GEOMETRY_
#define EGS_LATTICE_GEOMETRY_

#ifdef WIN32

    #ifdef BUILD_LATTICE_DLL
        #define EGS_LATTICE_EXPORT __declspec(dllexport)
    #else
        #define EGS_LATTICE_EXPORT __declspec(dllimport)
    #endif
    #define EGS_LATTICE_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_LATTICE_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_LATTICE_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_LATTICE_EXPORT
        #define EGS_LATTICE_LOCAL
    #endif

#endif

/*! \brief A Bravais, cubic, and hexagonal lattice geometry
  \ingroup Geometry
  \ingroup CompositeG

A geometry which embeds a lattice of one geometry (named subgeometry below)
into one region of a second geometry (named base geometry). This geometry
effectively recurses the subgeometry at every position defined by a Bravais,
cubic, or hexagonal lattice. As such, you can model an infinite amount of
subgeometries (e.g., region 0 of egs_space) and the only slow down to your
simulation would depend on how many subgeometries you would expect over a
particle track.

As this geometry only stores a single subgeometry in memory, it can only score
in ALL subgeometries (or rather, the single subgeometry at all lattice
positions) at once. Therefore, dose to the subgeometry at different locations
cannot be discerned. Final geometry regions are numbered as all base geometry
regions first, then enumerating all subgeometry regions afterwards. A more
complete description can be found in the paper referenced in the header.

All egs_lattice geometries require a base geometry, a subgeometry, and a
subgeometry index. The base geometry is the larger geometry into which the
lattice will be embedded. The subgeometry is the geometry which will be placed
at each lattice position. The subgeometry index will be the base geometry
region in which the lattice is placed. Any geometries at the boundary will be
partially modelled (only the fraction in the selected region). The spacing and
type of geometry can be defined in one of three ways:

    1) Bravais Lattice: Define spacing in x, y, and z using the three inputs
    for spacing, respectively.

    2) Cubic Lattice: Define the same spacing in x, y, and z using a single
    input for spacing.

    3) Hexagonal Lattice: Define the type as hexagonal, then spacing defines
    the hexagonal close-packed lattice nearest neighbour distance.

\verbatim
    #Example Bravais lattice with spacings 1, 2 and 3
    :start geometry:
        library           = egs_lattice
        name              = phantom_w_microcavity
        base geometry     = phantom
        subgeometry       = microcavity
        subgeometry index = 0
        spacing           = 1 2 3
    :stop geometry:
\endverbatim

\verbatim
    #Example cubic lattice with spacings 1
    :start geometry:
        library           = egs_lattice
        name              = phantom_w_microcavity
        base geometry     = phantom
        subgeometry       = microcavity
        subgeometry index = 0
        spacing           = 1
    :stop geometry:
\endverbatim

\verbatim
    #Example hexagonal lattice with nearest neighbour distance 1
    :start geometry:
        library           = egs_lattice
        type              = hexagonal
        name              = phantom_w_microcavity
        base geometry     = phantom
        subgeometry       = microcavity
        subgeometry index = 0
        spacing           = 1
    :stop geometry:
\endverbatim
*/

class EGS_LATTICE_EXPORT EGS_Lattice : public EGS_BaseGeometry {
protected:

    EGS_BaseGeometry         *base;    //!< The geometry within which the sub geometry appears
    EGS_TransformedGeometry  *sub;     //!< The sub geometry that could appear within base
    int                       ind;     //!< The region in base geom where we could encounter sub geom
    int                       maxStep; //!< The maximum number of steps
    EGS_Float                 a, b, c; //!< The center-to-center distance along x, y, and z
    string                    type;    //!< The geometry type

public:

    EGS_Lattice(EGS_BaseGeometry *B, EGS_BaseGeometry *S, int i, EGS_Float x,
                EGS_Float y, EGS_Float z, const string &Name = "");
    ~EGS_Lattice();

    EGS_Vector closestPoint(const EGS_Vector &x) {
        return EGS_Vector(int(round(x.x/a))*a,
                          int(round(x.y/b))*b,
                          int(round(x.z/c))*c);
    };

    int computeIntersections(int ireg, int n, const EGS_Vector &x,
                             const EGS_Vector &u, EGS_GeometryIntersections *isections) {
        return base->computeIntersections(ireg,n,x,u,isections);
    };

    bool isRealRegion(int ireg) const {
        if (ireg < base->regions()) { // If ireg is less than base regions
            return base->isRealRegion(ireg);    // check against base regions
        }
        return sub->isRealRegion(ireg - base->regions()); // then check sub regions
    };

    bool isInside(const EGS_Vector &x) {
        return base->isInside(x);
    };

    int isWhere(const EGS_Vector &x) {
        int temp = base->isWhere(x);
        if (temp == ind) {// Are we in the subgeom?
            sub->setTransformation(closestPoint(x));
            if (sub->isInside(x)) {
                return sub->isWhere(x) + base->regions();
            }
        }
        return temp; // otherwise base geom
    };

    int inside(const EGS_Vector &x) {
        return isWhere(x);
    };

    int medium(int ireg) const {
        if (ireg >= base->regions()) { // If ireg is greater than base regions
            return sub->medium(ireg-base->regions());
        }
        return base->medium(ireg); // If in base region
    };

    EGS_Float howfarToOutside(int ireg, const EGS_Vector &x, const EGS_Vector &u) {
        if (ireg < 0) { // Error catch, not inside
            return 0;
        }

        // Are we in the subgeom
        if (ireg >= base->regions()) {
            return base->howfarToOutside(ind,x,u);
        }
        return base->howfarToOutside(ireg,x,u);
    };

    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t,int *newmed=0, EGS_Vector *normal=0) {
        // Catch t=0 exception, which sometimes causes issues when ind is the region neighboring -1
        if (t < epsilon) {
            t = 0.0;
            return ireg;
        }

        if (ireg >= base->regions()) { // Are we in the subgeom?
            sub->setTransformation(closestPoint(x));

            // Do the howfar call
            EGS_Float tempT = t;
            int tempReg = sub->howfar(ireg-base->regions(),x,u,tempT,newmed,normal);

            // If we do leave the subgeom, we want to return the index
            // of the region of the base geometry we would be entering,
            // which is not necessarily ind if subgeom is at a boundary
            EGS_Vector newX = u;
            newX.normalize();
            newX = x + (newX * tempT);

            if (base->isWhere(newX) != ind) {// If we leave ind region of base
                t = tempT;

                tempReg = base->howfar(ind,x,u,t,0,normal);
                if (newmed && tempReg >= 0) {
                    *newmed = base->medium(tempReg);
                }

                //if (t<0) {
                //  egsWarning("Returning negative t from subgeom of %s\n",getName().c_str());
                //  egsWarning("howfar(%7d,[%e,%e,%e],[%e,%e,%e],%e)\n",ireg, x.x, x.y, x.z, u.x, u.y, u.z, t);
                //}
                return tempReg;
            }

            if (!(tempReg+1)) {// If we leave sub geom back into ind
                if (newmed) {
                    *newmed = base->medium(ind);
                }
                t = tempT;
                return ind;
            }

            // else we stay in sub geom
            t = tempT;
            //if (t<0) {
            //  egsWarning("Returning negative t from subgeom of %s\n",getName().c_str());
            //  egsWarning("howfar(%7d,[%e,%e,%e],[%e,%e,%e],%e)\n",ireg, x.x, x.y, x.z, u.x, u.y, u.z, t);
            //}
            return tempReg+base->regions();
        }
        else if (ireg == ind) {// If we are in the region that could contain subgeoms
            // Determine the path travelled ------------------------------------------------ //
            EGS_Float tempT = t;
            base->howfar(ireg,x,u,tempT); // Get how far it is in temp
            EGS_Float max = tempT;

            // Iterate through the lattice ------------------------------------------------- //
            EGS_Float min = max, minX = max, minY = max, minZ = max; // Distance to closest subgeom for three different cases
            EGS_Vector unit = u;
            unit.normalize();
            EGS_Vector xInt = unit*(a/4.0/fabs(unit.x)), yInt = unit*(b/4.0/fabs(unit.y)), zInt = unit*(c/4.0/fabs(unit.z));
            EGS_Vector tempP; // Temporarily hold indices of subgeom we are testing against
            EGS_Vector finalP, finalX, finalY, finalZ; // Current closest intersecting subgeom

            EGS_Vector x0;
            EGS_Float max2 = max*max;

            x0 = x;
            while ((x-x0).length2() < max2) {// Quit as soon as our testing position is beyond current closest intersection (which could be tempT)
                tempT = minX;
                tempP = closestPoint(x0);
                sub->setTransformation(tempP);
                if (sub->howfar(-1,x,u,tempT)+1) // Intersection!
                    if (tempT < max && tempT > 0) {
                        finalX = tempP;
                        minX = tempT;
                        break;
                    }
                x0 += xInt;
            }

            x0 = x;
            while ((x-x0).length2() < max2) {// Quit as soon as our testing position is beyond current closest intersection (which could be tempT)
                tempT = minY;
                tempP = closestPoint(x0);
                sub->setTransformation(tempP);
                if (sub->howfar(-1,x,u,tempT)+1) // Intersection!
                    if (tempT < max && tempT > 0) {
                        finalY = tempP;
                        minY = tempT;
                        break;
                    }
                x0 += yInt;
            }

            x0 = x;
            while ((x-x0).length2() < max2) {// Quit as soon as our testing position is beyond current closest intersection (which could be tempT)
                tempT = minZ;
                tempP = closestPoint(x0);
                sub->setTransformation(tempP);
                if (sub->howfar(-1,x,u,tempT)+1) // Intersection!
                    if (tempT < max && tempT > 0) {
                        finalZ = tempP;
                        minZ = tempT;
                        break;
                    }
                x0 += zInt;
            }

            finalP = finalX;
            min = minX;
            if (min > minY) {
                min = minY;
                finalP = finalY;
            }
            if (min > minZ) {
                min = minZ;
                finalP = finalZ;
            }

            // Check last point
            tempT = max;
            sub->setTransformation(closestPoint(x+unit*tempT));
            sub->howfar(-1,x,u,tempT);
            if (tempT < min  && tempT > 0) {
                min = tempT;
                finalP = closestPoint(x+unit*tempT);
            }

            // We did intersect subgeom
            if (min < max) {
                tempT = t;
                sub->setTransformation(finalP);
                int tempReg = sub->howfar(-1,x,u,tempT,newmed,normal);
                if (tempReg+1) {
                    if (newmed && tempReg >= 0) {
                        *newmed = sub->medium(tempReg);
                    }
                    t = tempT;

                    //if (t<0) {
                    //  egsWarning("Returning negative t from region %d in base geom of %s\n", ind, getName().c_str());
                    //  egsWarning("howfar(%7d,[%e,%e,%e],[%e,%e,%e],%e)\n",ireg, x.x, x.y, x.z, u.x, u.y, u.z, t);
                    //}
                    return tempReg+base->regions();
                }
            }

            // We didn't intersect subgeom
            int tempReg = base->howfar(ireg,x,u,t,newmed,normal);
            //if (t<0) {
            //  egsWarning("Returning negative t from region %d in base geom of %s\n", ind, getName().c_str());
            //  egsWarning("howfar(%7d,[%e,%e,%e],[%e,%e,%e],%e)\n",ireg, x.x, x.y, x.z, u.x, u.y, u.z, t);
            //}
            if (newmed && tempReg >= 0) {
                *newmed = base->medium(tempReg);
            }

            return tempReg;
        }
        else {// Not in region containing subgeoms, then it is quite easy
            // unless we enter directly into a subgeom when entering
            // region ind
            int tempReg = base->howfar(ireg,x,u,t,newmed,normal);

            // If we enter region ind
            if (tempReg == ind) {
                // newX is the point of entrance
                EGS_Vector newX = u;
                newX.normalize();
                newX = x + (newX * t);

                sub->setTransformation(closestPoint(newX));
                int newReg = sub->isWhere(newX);
                if (newReg+1) {                         // see what region we are in
                    if (newmed && newReg >= 0) {        // in subgeom, if its not -1
                        *newmed = sub->medium(newReg);    // then return the proper
                    }
                    //if (t<0) {                          // media and region
                    //  egsWarning("Returning negative t from region %d (not ind) in base geom of %s\n", ireg, getName().c_str());
                    //  egsWarning("howfar(%7d,[%e,%e,%e],[%e,%e,%e],%e)\n",ireg, x.x, x.y, x.z, u.x, u.y, u.z, t);
                    //}
                    return newReg + base->regions();
                }
            }

            if (newmed && tempReg >= 0) {
                *newmed = base->medium(tempReg);
            }
            //if (t<0) {
            //  egsWarning("Returning negative t from region %d (not ind) in base geom of %s\n", ireg, getName().c_str());
            //  egsWarning("howfar(%7d,[%e,%e,%e],[%e,%e,%e],%e)\n",ireg, x.x, x.y, x.z, u.x, u.y, u.z, t);
            //}
            return tempReg;
        }
    };

    EGS_Float hownear(int ireg, const EGS_Vector &x) {
        EGS_Float temp, dist = base->hownear(ind,x);
        if (ireg >= base->regions()) {
            sub->setTransformation(closestPoint(x));
            temp = sub->hownear(ireg-base->regions(),x);
            dist=(temp<dist)?temp:dist;
        }
        else if (ireg == ind) {
            // Check all nearby geometries
            EGS_Float xa = floor(x.x/a)*a, yb = floor(x.y/b)*b, zc = floor(x.z/c)*c;
            EGS_Vector x0[8] = {EGS_Vector(xa,  yb,  zc),
                                EGS_Vector(xa+a,yb,  zc),
                                EGS_Vector(xa,  yb+b,zc),
                                EGS_Vector(xa+a,yb+b,zc),
                                EGS_Vector(xa,  yb,  zc+c),
                                EGS_Vector(xa+a,yb,  zc+c),
                                EGS_Vector(xa,  yb+b,zc+c),
                                EGS_Vector(xa+a,yb+b,zc+c)
                               };
            for (int i = 0; i < 8; i++) {
                sub->setTransformation(x0[i]);
                temp = sub->hownear(-1,x);
                if (temp < dist) {
                    dist = temp;
                }
            }
        }
        else {
            dist = base->hownear(ireg,x);
        }
        return dist;
    };

    int regions() {
        return base->regions() + sub->regions();
    }

    int getMaxStep() const {
        return maxStep;
    };

    const string &getType() const {
        return type;
    };

    void printInfo() const;

    // I have no idea what this boolean stuff is for
    bool hasBooleanProperty(int ireg, EGS_BPType prop) const {
        if (ireg < base->regions()) { // If ireg is less than base regions
            return base->hasBooleanProperty(ireg, prop);    // check against base regions
        }
        return sub->hasBooleanProperty(ireg - base->regions(), prop); // then check sub regions
    };
    void setBooleanProperty(EGS_BPType prop) {
        base->setBooleanProperty(prop);
    };
    void addBooleanProperty(int bit) {
        base->addBooleanProperty(bit);
    };
    void setBooleanProperty(EGS_BPType prop, int start, int end, int step=1) {
        base->setBooleanProperty(prop,start,end,step);
    };
    void addBooleanProperty(int bit, int start, int end, int step=1) {
        base->addBooleanProperty(bit,start,end,step);
    };

    EGS_Float getRelativeRho(int ireg) const {
        if (ireg < base->regions()) { // If ireg is less than base regions
            return base->getRelativeRho(ireg);    // check against base regions
        }
        return sub->getRelativeRho(ireg - base->regions()); // then check sub regions
    };
    void setRelativeRho(int start, int end, EGS_Float rho);
    void setRelativeRho(EGS_Input *);

    void  setBScaling(int start, int end, EGS_Float rho);
    void  setBScaling(EGS_Input *);
    EGS_Float getBScaling(int ireg) const {
        if (ireg < base->regions()) { // If ireg is less than base regions
            return base->getBScaling(ireg);    // check against base regions
        }
        return sub->getBScaling(ireg - base->regions()); // then check sub regions
    };

    virtual void getLabelRegions(const string &str, vector<int> &regs);

protected:
    void setMedia(EGS_Input *inp,int,const int *);
};

class EGS_LATTICE_EXPORT EGS_Hexagonal_Lattice : public EGS_BaseGeometry {
protected:

    EGS_BaseGeometry        *base;    //!< The geometry within which the sub geometry appears
    EGS_TransformedGeometry *sub;     //!< The sub geometry that could appear within base
    int                      ind;     //!< The region in base geom where we could encounter sub geom
    int                      maxStep; //!< The maximum number of steps
    EGS_Float                a;       //!< The center-to-center distance to the nearest 12 neighbours
    vector<EGS_Float>        d;       //!< Don't redefine 4 dists in closestPoint each invocation
    string                   type;    //!< The geometry type
    EGS_Float                gap;     //!< Translating the hexagonal lattice to xyz coordinate,
    //   y and z have a shorter recurrence scale than a
public:

    EGS_Hexagonal_Lattice(EGS_BaseGeometry *B, EGS_BaseGeometry *S, int i, EGS_Float x, const string &Name = "");
    ~EGS_Hexagonal_Lattice();

    // Based off the closestPoint() function in the egs_lattice definition, here we check against the four
    // Bravais lattices that make up the hexagonal lattice to find the closest point
    EGS_Vector closestPoint(const EGS_Vector &x) {
        EGS_Float i1,j1,k1,i2,j2,j3,k3,j4;
        EGS_Float r3a = 2.0*gap;

        i1 = a*(round(x.x/a));
        j1 = r3a*(round(x.y/r3a));
        k1 = r3a*(round(x.z/r3a));

        i2 = a*(0.5+round(x.x/a-0.5));
        j2 = r3a*(0.5+round(x.y/r3a-0.5));

        j3 = r3a*(0.25+round(x.y/r3a-0.25));
        k3 = r3a*(0.5+round(x.z/r3a-0.5));

        j4 = r3a*(-0.25+round(x.y/r3a+0.25));

        EGS_Vector p1(i1,j1,k1);
        EGS_Vector p2(i2,j2,k1);
        EGS_Vector p3(i1,j3,k3);
        EGS_Vector p4(i2,j4,k3);

        d[0] = (x-p1).length2();
        d[1] = (x-p2).length2();
        d[2] = (x-p3).length2();
        d[3] = (x-p4).length2();

        if (d[0] < d[1] && d[0] < d[2] && d[0] < d[3]) {
            return p1;
        }
        if (d[1] < d[2] && d[1] < d[3]) {
            return p2;
        }
        if (d[2] < d[3]) {
            return p3;
        }
        return p4;
    };

    int computeIntersections(int ireg, int n, const EGS_Vector &x,
                             const EGS_Vector &u, EGS_GeometryIntersections *isections) {
        return base->computeIntersections(ireg,n,x,u,isections);
    };

    bool isRealRegion(int ireg) const {
        if (ireg < base->regions()) { // If ireg is less than base regions
            return base->isRealRegion(ireg);    // check against base regions
        }
        return sub->isRealRegion(ireg - base->regions()); // then check sub regions
    };

    bool isInside(const EGS_Vector &x) {
        return base->isInside(x);
    };

    int isWhere(const EGS_Vector &x) {
        int temp = base->isWhere(x);
        if (temp == ind) {// Are we in the subgeom?
            sub->setTransformation(closestPoint(x));
            if (sub->isInside(x)) {
                return sub->isWhere(x) + base->regions();
            }
        }
        return temp; // otherwise base geom
    };

    int inside(const EGS_Vector &x) {
        return isWhere(x);
    };

    int medium(int ireg) const {
        if (ireg >= base->regions()) { // If ireg is greater than base regions
            return sub->medium(ireg-base->regions());
        }
        return base->medium(ireg); // If in base region
    };

    EGS_Float howfarToOutside(int ireg, const EGS_Vector &x, const EGS_Vector &u) {
        if (ireg < 0) { // Error catch, not inside
            return 0;
        }

        // Are we in the subgeom
        if (ireg >= base->regions()) {
            return base->howfarToOutside(ind,x,u);
        }
        return base->howfarToOutside(ireg,x,u);
    };

    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t,int *newmed=0, EGS_Vector *normal=0) {
        // Catch t=0 exception, which sometimes causes issues when ind is the region neighboring -1
        if (t < epsilon) {
            t = 0.0;
            return ireg;
        }

        if (ireg >= base->regions()) { // Are we in the subgeom?
            sub->setTransformation(closestPoint(x));

            // Do the howfar call
            EGS_Float tempT = t;
            int tempReg = sub->howfar(ireg-base->regions(),x,u,tempT,newmed,normal);

            // If we do leave the subgeom, we want to return the index
            // of the region of the base geometry we would be entering,
            // which is not necessarily ind if subgeom is at a boundary
            EGS_Vector newX = u;
            newX.normalize();
            newX = x + (newX * tempT);

            if (base->isWhere(newX) != ind) {// If we leave ind region of base
                t = tempT;

                tempReg = base->howfar(ind,x,u,t,0,normal);
                if (newmed && tempReg >= 0) {
                    *newmed = base->medium(tempReg);
                }

                //if (t<0) {
                //  egsWarning("Returning negative t from subgeom of %s\n",getName().c_str());
                //  egsWarning("howfar(%7d,[%e,%e,%e],[%e,%e,%e],%e)\n",ireg, x.x, x.y, x.z, u.x, u.y, u.z, t);
                //}
                return tempReg;
            }

            if (!(tempReg+1)) {// If we leave sub geom back into ind
                if (newmed) {
                    *newmed = base->medium(ind);
                }
                t = tempT;
                return ind;
            }

            // else we stay in sub geom
            t = tempT;
            //if (t<0) {
            //  egsWarning("Returning negative t from subgeom of %s\n",getName().c_str());
            //  egsWarning("howfar(%7d,[%e,%e,%e],[%e,%e,%e],%e)\n",ireg, x.x, x.y, x.z, u.x, u.y, u.z, t);
            //}
            return tempReg+base->regions();
        }
        else if (ireg == ind) {// If we are in the region that could contain subgeoms
            // Determine the path travelled ------------------------------------------------ //
            EGS_Float tempT = t;
            base->howfar(ireg,x,u,tempT); // Get how far it is in temp
            EGS_Float max = tempT;

            // Iterate through the lattice ------------------------------------------------- //
            EGS_Float min = max, minX = max, minY = max, minZ = max; // Distance to closest subgeom for three different cases
            EGS_Vector unit = u;
            unit.normalize();
            EGS_Vector xInt = unit*(a/8.0/fabs(unit.x)), yInt = unit*(gap/8.0/fabs(unit.y)), zInt = unit*(gap/8.0/fabs(unit.z));
            EGS_Vector tempP; // Temporarily hold indices of subgeom we are testing against
            EGS_Vector finalP, finalX, finalY, finalZ; // Current closest intersecting subgeom

            EGS_Vector x0;
            EGS_Float max2 = max*max;

            x0 = x;
            while ((x-x0).length2() < max2) {// Quit as soon as our testing position is beyond current closest intersection (which could be tempT)
                tempT = minX;
                tempP = closestPoint(x0);
                sub->setTransformation(tempP);
                if (sub->howfar(-1,x,u,tempT)+1) // Intersection!
                    if (tempT < max && tempT > 0) {
                        finalX = tempP;
                        minX = tempT;
                        break;
                    }
                x0 += xInt;
            }

            x0 = x;
            while ((x-x0).length2() < max2) {// Quit as soon as our testing position is beyond current closest intersection (which could be tempT)
                tempT = minY;
                tempP = closestPoint(x0);
                sub->setTransformation(tempP);
                if (sub->howfar(-1,x,u,tempT)+1) // Intersection!
                    if (tempT < max && tempT > 0) {
                        finalY = tempP;
                        minY = tempT;
                        break;
                    }
                x0 += yInt;
            }

            x0 = x;
            while ((x-x0).length2() < max2) {// Quit as soon as our testing position is beyond current closest intersection (which could be tempT)
                tempT = minZ;
                tempP = closestPoint(x0);
                sub->setTransformation(tempP);
                if (sub->howfar(-1,x,u,tempT)+1) // Intersection!
                    if (tempT < max && tempT > 0) {
                        finalZ = tempP;
                        minZ = tempT;
                        break;
                    }
                x0 += zInt;
            }

            finalP = finalX;
            min = minX;
            if (min > minY) {
                min = minY;
                finalP = finalY;
            }
            if (min > minZ) {
                min = minZ;
                finalP = finalZ;
            }

            // Check last point
            tempT = max;
            sub->setTransformation(closestPoint(x+unit*tempT));
            sub->howfar(-1,x,u,tempT);
            if (tempT < min  && tempT > 0) {
                min = tempT;
                finalP = closestPoint(x+unit*tempT);
            }

            // We did intersect subgeom
            if (min < max) {
                tempT = t;
                sub->setTransformation(finalP);
                int tempReg = sub->howfar(-1,x,u,tempT,newmed,normal);
                if (tempReg+1) {
                    if (newmed && tempReg >= 0) {
                        *newmed = sub->medium(tempReg);
                    }
                    t = tempT;

                    //if (t<0) {
                    //  egsWarning("Returning negative t from region %d in base geom of %s\n", ind, getName().c_str());
                    //  egsWarning("howfar(%7d,[%e,%e,%e],[%e,%e,%e],%e)\n",ireg, x.x, x.y, x.z, u.x, u.y, u.z, t);
                    //}
                    return tempReg+base->regions();
                }
            }

            // We didn't intersect subgeom
            int tempReg = base->howfar(ireg,x,u,t,newmed,normal);
            //if (t<0) {
            //  egsWarning("Returning negative t from region %d in base geom of %s\n", ind, getName().c_str());
            //  egsWarning("howfar(%7d,[%e,%e,%e],[%e,%e,%e],%e)\n",ireg, x.x, x.y, x.z, u.x, u.y, u.z, t);
            //}
            if (newmed && tempReg >= 0) {
                *newmed = base->medium(tempReg);
            }

            return tempReg;
        }
        else {// Not in region containing subgeoms, then it is quite easy
            // unless we enter directly into a subgeom when entering
            // region ind
            int tempReg = base->howfar(ireg,x,u,t,newmed,normal);

            // If we enter region ind
            if (tempReg == ind) {
                // newX is the point of entrance
                EGS_Vector newX = u;
                newX.normalize();
                newX = x + (newX * t);

                sub->setTransformation(closestPoint(newX));
                int newReg = sub->isWhere(newX);
                if (newReg+1) {                        // see what region we are in
                    if (newmed && newReg >= 0) {       // in subgeom, if its not -1
                        *newmed = sub->medium(newReg);    // then return the proper
                    }
                    //if (t<0) {                         // media and region
                    //  egsWarning("Returning negative t from region %d (not ind) in base geom of %s\n", ireg, getName().c_str());
                    //  egsWarning("howfar(%7d,[%e,%e,%e],[%e,%e,%e],%e)\n", ireg, x.x, x.y, x.z, u.x, u.y, u.z, t);
                    //}
                    return newReg + base->regions();
                }
            }

            if (newmed && tempReg >= 0) {
                *newmed = base->medium(tempReg);
            }
            //if (t<0) {
            //  egsWarning("Returning negative t from region %d (not ind) in base geom of %s\n", ireg, getName().c_str());
            //  egsWarning("howfar(%7d,[%e,%e,%e],[%e,%e,%e],%e)\n", ireg, x.x, x.y, x.z, u.x, u.y, u.z, t);
            //}
            return tempReg;
        }
    };

    EGS_Float hownear(int ireg, const EGS_Vector &x) {
        EGS_Float temp, dist = base->hownear(ireg>=base->regions()?ind:ireg,x);
        if (ireg >= base->regions()) {
            sub->setTransformation(closestPoint(x));
            temp = sub->hownear(ireg-base->regions(),x);
            dist=(temp<dist)?temp:dist;
        }
        else if (ireg == ind) {
            // Check all nearby geometries, making use of the 4 Bravais lattice substructure of the geometry
            EGS_Float i,j,k;
            EGS_Float r3a = 2.0*gap, hgap = gap/2.0, ha = a/2.0;
            EGS_Vector x0[8];

            // Calculate the edge of an oblique rhombic prism formed around x
            i = a*(floor(x.x/a));
            j = r3a*(round(x.y/r3a));
            k = r3a*(round(x.z/r3a));

            // One of the following 4 trigonal trapezohedrons must enclose x, they all share an edge defined
            // as (i,j,k) to (i+a,j,k) and can recurse through all space
            if (x.y<=j && x.z<=k)
                EGS_Vector x0[8] = {EGS_Vector(i,j,k),
                                    EGS_Vector(i-ha,j-gap,k),
                                    EGS_Vector(i,j-gap-hgap,k-gap),
                                    EGS_Vector(i+ha,j-hgap,k-gap),
                                    EGS_Vector(i+a,j,k),
                                    EGS_Vector(i+ha,j-gap,k),
                                    EGS_Vector(i+a,j-gap-hgap,k-gap),
                                    EGS_Vector(i+a+ha,j-hgap,k-gap)
                                   };
            else if (x.y>j && x.z<=k)
                EGS_Vector x0[8] = {EGS_Vector(i,j,k),
                                    EGS_Vector(i+ha,j+gap,k),
                                    EGS_Vector(i+a,j+hgap,k-gap),
                                    EGS_Vector(i+ha,j-hgap,k-gap),
                                    EGS_Vector(i+a,j,k),
                                    EGS_Vector(i+a+ha,j+gap,k),
                                    EGS_Vector(i+a+a,j+hgap,k-gap),
                                    EGS_Vector(i+a+ha,j-hgap,k-gap)
                                   };
            else if (x.y<j && x.z>k)
                EGS_Vector x0[8] = {EGS_Vector(i,j,k),
                                    EGS_Vector(i+ha,j-gap,k),
                                    EGS_Vector(i,j-gap-hgap,k+gap),
                                    EGS_Vector(i-ha,j-hgap,k+gap),
                                    EGS_Vector(i+a,j,k),
                                    EGS_Vector(i+a+ha,j-gap,k),
                                    EGS_Vector(i+a,j-gap-hgap,k+gap),
                                    EGS_Vector(i+ha,j-hgap,k+gap)
                                   };
            else //(x.y>j && x.z>k)
                EGS_Vector x0[8] = {EGS_Vector(i,j,k),
                                    EGS_Vector(i-ha,j+gap,k),
                                    EGS_Vector(i-a,j+hgap,k+gap),
                                    EGS_Vector(i-ha,j-hgap,k+gap),
                                    EGS_Vector(i+a,j,k),
                                    EGS_Vector(i+ha,j+gap,k),
                                    EGS_Vector(i,j+hgap,k+gap),
                                    EGS_Vector(i+ha,j-hgap,k+gap)
                                   };
            for (int i = 0; i < 8; i++) {
                sub->setTransformation(x0[i]);
                temp = sub->hownear(-1,x);
                if (temp < dist) {
                    dist = temp;
                }
            }
        }
        return dist;
    };

    int regions() {
        return base->regions() + sub->regions();
    }

    int getMaxStep() const {
        return maxStep;
    };

    const string &getType() const {
        return type;
    };

    void printInfo() const;

    // I have no idea what this boolean stuff is for
    bool hasBooleanProperty(int ireg, EGS_BPType prop) const {
        if (ireg < base->regions()) { // If ireg is less than base regions
            return base->hasBooleanProperty(ireg, prop);    // check against base regions
        }
        return sub->hasBooleanProperty(ireg - base->regions(), prop); // then check sub regions
    };
    void setBooleanProperty(EGS_BPType prop) {
        base->setBooleanProperty(prop);
    };
    void addBooleanProperty(int bit) {
        base->addBooleanProperty(bit);
    };
    void setBooleanProperty(EGS_BPType prop, int start, int end, int step=1) {
        base->setBooleanProperty(prop,start,end,step);
    };
    void addBooleanProperty(int bit, int start, int end, int step=1) {
        base->addBooleanProperty(bit,start,end,step);
    };

    EGS_Float getRelativeRho(int ireg) const {
        if (ireg < base->regions()) { // If ireg is less than base regions
            return base->getRelativeRho(ireg);    // check against base regions
        }
        return sub->getRelativeRho(ireg - base->regions()); // then check sub regions
    };
    void setRelativeRho(int start, int end, EGS_Float rho);
    void setRelativeRho(EGS_Input *);

    void  setBScaling(int start, int end, EGS_Float rho);
    void  setBScaling(EGS_Input *);
    EGS_Float getBScaling(int ireg) const {
        if (ireg < base->regions()) { // If ireg is less than base regions
            return base->getBScaling(ireg);    // check against base regions
        }
        return sub->getBScaling(ireg - base->regions()); // then check sub regions
    };

    virtual void getLabelRegions(const string &str, vector<int> &regs);

protected:
    void setMedia(EGS_Input *inp,int,const int *);
};

#endif
