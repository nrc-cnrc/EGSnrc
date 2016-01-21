/*
###############################################################################
#
#  EGSnrc egs++ geometry viewer lighting headers
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


#ifndef EGS_LIGHT_
#define EGS_LIGHT_

// for now, we just use the diffuse component of a GL type light.

class EGS_Light {

public:

    EGS_Light(const EGS_Vector &pos, const EGS_Vector &color) :
        xl(pos), diffuse_c(color) {};

    // get the light reflection at vertex x with notmal n that has
    // the color d.
    EGS_Vector getColor(const EGS_Vector &x, const EGS_Vector &n,
                        const EGS_Vector &d_color) {
        EGS_Vector L(xl-x);
        EGS_Float Lxn = L*n;
        if (Lxn > 0) {
            Lxn /= L.length();
            return diffuse_c.getScaled(d_color)*Lxn;
        }
        return EGS_Vector();
    };

protected:

    EGS_Vector     xl;           // light position.
    EGS_Vector     diffuse_c;    // the r,g,b components of the difuse light.

};

// for now, the ambient color of a material is the same as the diffuse
// color and there is no specular component.

class EGS_MaterialColor {

public:

    EGS_MaterialColor(EGS_Float Alpha=1) : d(EGS_Vector()), alpha(Alpha) {};
    EGS_MaterialColor(const EGS_Vector &d_color, EGS_Float Alpha=1) :
        d(d_color), alpha(Alpha) {};

    EGS_Vector d;        // material color as a r,g,b triplet, should be
    // clamped between 0 and 1.
    EGS_Float  alpha;

};

#endif
