/*
###############################################################################
#
#  EGSnrc egs++ box geometry headers
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
#  Contributors:
#
###############################################################################
*/


/*! \file egs_box.h
 *  \brief A box geometry: header
 *  \IK
 */

#ifndef EGS_BOX_
#define EGS_BOX_

#include "egs_base_geometry.h"
#include "egs_transformations.h"


#ifdef WIN32

    #ifdef BUILD_BOX_DLL
        #define EGS_BOX_EXPORT __declspec(dllexport)
    #else
        #define EGS_BOX_EXPORT __declspec(dllimport)
    #endif
    #define EGS_BOX_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_BOX_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_BOX_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_BOX_EXPORT
        #define EGS_BOX_LOCAL
    #endif

#endif

/*! \brief A box geometry

  \ingroup Geometry
  \ingroup ElementaryG

The EGS_Box class implements a box geometry centered about the
origin that consists of a single region.
Strictly speaking a box is not a primitive geometry because
its methods could be implemented using the methods of sets of planes.
However, due to its frequent use, slightly faster geometry methods
using a direct implementation, and simpler definition, it is provided
as a separate elementary geometry. A box is defined using the following
key:
\verbatim
library  = egs_box
box size = 1 or 3 numbers defining the box size
\endverbatim
If a single number is found, the box is assumed to be a cube.
Note that a box at any location (not just about the origin) and/or
a rotated box can be obtained using a \link EGS_TransformedGeometry
transformed geometry. \endlink

*/
class EGS_BOX_EXPORT EGS_Box : public EGS_BaseGeometry {

    EGS_Float           ax, ay, az;
    EGS_AffineTransform *T;
    static string       type;

public:

    EGS_Box(EGS_Float a, const EGS_AffineTransform *t = 0,
            const string &Name = "") : EGS_BaseGeometry(Name),
        ax(a), ay(a), az(a), T(0)  {
        if (t) {
            T = new EGS_AffineTransform(*t);
        }
        nreg = 1;
    };

    EGS_Box(EGS_Float Ax, EGS_Float Ay, EGS_Float Az,
            const EGS_AffineTransform *t = 0,
            const string &Name = "") : EGS_BaseGeometry(Name),
        ax(Ax), ay(Ay), az(Az), T(0)  {
        if (t) {
            T = new EGS_AffineTransform(*t);
        }
        nreg = 1;
    };

    ~EGS_Box() {
        if (T) {
            delete T;
        }
    };

    bool isInside(const EGS_Vector &x) {
        EGS_Vector xp = T ? x*(*T) : x;
        if (2*xp.x + ax < 0 || 2*xp.x - ax > 0) {
            return false;
        }
        if (2*xp.y + ay < 0 || 2*xp.y - ay > 0) {
            return false;
        }
        if (2*xp.z + az < 0 || 2*xp.z - az > 0) {
            return false;
        }
        return true;
    };

    int isWhere(const EGS_Vector &x) {
        return isInside(x) ? 0 : -1;
    };

    int inside(const EGS_Vector &x) {
        return isWhere(x);
    };

    EGS_Float howfarToOutside(int ireg, const EGS_Vector &x,
                              const EGS_Vector &u) {
        EGS_Float t = 1e30;
        howfar(ireg,x,u,t);
        return t;
    };

    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t, int *newmed=0, EGS_Vector *normal=0) {
        EGS_Vector xp(x), up(u);
        if (T) {
            xp = x*(*T);
            up = u*T->getRotation();
        }
        if (ireg == 0) {
            EGS_Float t1 = 1e30;
            int inew = 0;
            EGS_Vector n;
            if (up.x > 0) {
                t1 = (ax - 2*xp.x)/(2*up.x);
                n.x = -1;
            }
            else if (up.x < 0) {
                t1 = -(ax + 2*xp.x)/(2*up.x);
                n.x = 1;
            }
            if (t1 < t) {
                t = t1;
                inew = -1;
            }
            t1 = 1e30;
            if (up.y > 0) {
                t1 = (ay - 2*xp.y)/(2*up.y);
            }
            else if (up.y < 0) {
                t1 = -(ay + 2*xp.y)/(2*up.y);
            }
            if (t1 < t) {
                n.x = 0;
                n.y = up.y > 0 ? -1 : 1;
                t = t1;
                inew = -1;
            }
            t1 = 1e30;
            if (up.z > 0) {
                t1 = (az - 2*xp.z)/(2*up.z);
            }
            else if (up.z < 0) {
                t1 = -(az + 2*xp.z)/(2*up.z);
            }
            if (t1 < t) {
                t = t1;
                inew = -1;
                n.x = n.y = 0;
                n.z = up.z > 0 ? -1 : 1;
            }
            if (inew < 0) {
                if (newmed) {
                    *newmed = -1;
                }
                if (normal) {
                    if (T) {
                        *normal = T->getRotation()*n;
                    }
                    else {
                        *normal = n;
                    }
                }
            }
            return inew;
        }
        EGS_Float t1 = 1e30;
        if (2*xp.x + ax < 0 && up.x > 0) {
            t1 = -(2*xp.x + ax)/(2*up.x);
        }
        else if (2*xp.x - ax > 0 && up.x < 0) {
            t1 = -(2*xp.x - ax)/(2*up.x);
        }
        if (t1 < t) {
            EGS_Float y1 = xp.y + up.y*t1, z1 = xp.z + up.z*t1;
            if (2*y1 + ay >= 0 && 2*y1 - ay <= 0 &&
                    2*z1 + az >= 0 && 2*z1 - az <= 0) {
                t = t1;
                if (newmed) {
                    *newmed = med;
                }
                if (normal) {
                    if (T) {
                        const EGS_RotationMatrix &R = T->getRotation();
                        if (up.x > 0) {
                            *normal = R*EGS_Vector(-1,0,0);
                        }
                        else {
                            *normal = R*EGS_Vector(1,0,0);
                        }
                    }
                    else {
                        if (up.x > 0) {
                            *normal = EGS_Vector(-1,0,0);
                        }
                        else {
                            *normal = EGS_Vector(1,0,0);
                        }
                    }
                }
                return 0;
            }
        }
        t1 = 1e30;
        if (2*xp.y + ay < 0 && up.y > 0) {
            t1 = -(2*xp.y + ay)/(2*up.y);
        }
        else if (2*xp.y - ay > 0 && up.y < 0) {
            t1 = -(2*xp.y - ay)/(2*up.y);
        }
        if (t1 < t) {
            EGS_Float x1 = xp.x + up.x*t1, z1 = xp.z + up.z*t1;
            if (2*x1 + ax >= 0 && 2*x1 - ax <= 0 &&
                    2*z1 + az >= 0 && 2*z1 - az <= 0) {
                t = t1;
                if (newmed) {
                    *newmed = med;
                }
                if (normal) {
                    if (T) {
                        const EGS_RotationMatrix &R = T->getRotation();
                        if (up.y > 0) {
                            *normal = R*EGS_Vector(0,-1,0);
                        }
                        else {
                            *normal = R*EGS_Vector(0,1,0);
                        }
                    }
                    else {
                        if (up.y > 0) {
                            *normal = EGS_Vector(0,-1,0);
                        }
                        else {
                            *normal = EGS_Vector(0,1,0);
                        }
                    }
                }
                return 0;
            }
        }
        t1 = 1e30;
        if (2*xp.z + az < 0 && up.z > 0) {
            t1 = -(2*xp.z + az)/(2*up.z);
        }
        else if (2*xp.z - az > 0 && up.z < 0) {
            t1 = -(2*xp.z - az)/(2*up.z);
        }
        if (t1 < t) {
            EGS_Float x1 = xp.x + up.x*t1, y1 = xp.y + up.y*t1;
            if (2*x1 + ax >= 0 && 2*x1 - ax <= 0 &&
                    2*y1 + ay >= 0 && 2*y1 - ay <= 0) {
                t = t1;
                if (newmed) {
                    *newmed = med;
                }
                if (normal) {
                    if (T) {
                        const EGS_RotationMatrix &R = T->getRotation();
                        if (up.z > 0) {
                            *normal = R*EGS_Vector(0,0,-1);
                        }
                        else {
                            *normal = R*EGS_Vector(0,0,1);
                        }
                    }
                    else {
                        if (up.z > 0) {
                            *normal = EGS_Vector(0,0,-1);
                        }
                        else {
                            *normal = EGS_Vector(0,0,1);
                        }
                    }
                }
                return 0;
            }
        }
        return -1;
    };

    EGS_Float hownear(int ireg, const EGS_Vector &x) {
        EGS_Vector xp = T ? x*(*T) : x;
        if (ireg >= 0) {
            EGS_Float tmin;
            EGS_Float t1 = xp.x + 0.5*ax, t2 = 0.5*ax - xp.x;
            if (t1 < t2) {
                tmin = t1;
            }
            else {
                tmin = t2;
            }
            t1 = xp.y + 0.5*ay;
            if (t1 < tmin) {
                tmin = t1;
            }
            t1 = 0.5*ay - xp.y;
            if (t1 < tmin) {
                tmin = t1;
            }
            t1 = xp.z + 0.5*az;
            if (t1 < tmin) {
                tmin = t1;
            }
            t1 = 0.5*az - xp.z;
            if (t1 < tmin) {
                tmin = t1;
            }
            return tmin;
        }
        EGS_Float s1=0, s2=0;
        int nout = 0;
        if (2*xp.x + ax < 0) {
            EGS_Float t = -0.5*ax - xp.x;
            s1 += t;
            s2 += t*t;
            nout++;
        }
        else if (2*xp.x - ax > 0) {
            EGS_Float t = xp.x - 0.5*ax;
            s1 += t;
            s2 += t*t;
            nout++;
        }
        if (2*xp.y + ay < 0) {
            EGS_Float t = -0.5*ay - xp.y;
            s1 += t;
            s2 += t*t;
            nout++;
        }
        else if (2*xp.y - ay > 0) {
            EGS_Float t = xp.y - 0.5*ay;
            s1 += t;
            s2 += t*t;
            nout++;
        }
        if (2*xp.z + az < 0) {
            EGS_Float t = -0.5*az - xp.z;
            s1 += t;
            s2 += t*t;
            nout++;
        }
        else if (2*xp.z - az > 0) {
            EGS_Float t = xp.z - 0.5*az;
            s1 += t;
            s2 += t*t;
            nout++;
        }
        if (nout < 2) {
            return s1;
        }
        return sqrt(s2);
    };

    const string &getType() const {
        return type;
    };

    void printInfo() const;

};

#endif
