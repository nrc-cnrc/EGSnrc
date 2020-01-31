/*
###############################################################################
#
#  EGSnrc egs++ conical shell shape
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
#
###############################################################################
#
#  egs_conical_shell was developed for the Carleton Laboratory for
#  Radiotherapy Physics.
#
###############################################################################
*/


/*! \file egs_conical_shell.cpp
    \brief a conical stack shell shape
    \author Randle Taylor (randle.taylor@gmail.com)
*/

#include "egs_conical_shell.h"
#include "egs_input.h"
#include "egs_functions.h"
#include <iostream>
#include <fstream>


CSSSLayer::CSSSLayer(EGS_Float t, EGS_Float rit, EGS_Float rot, EGS_Float rib, EGS_Float rob, EGS_Float z):
    thick(t), ri_top(rit), ro_top(rot), ri_bot(rib), ro_bot(rob), zo(z) {

    ro_max = max(ro_top, ro_bot);
    ro_min = min(ro_top, ro_bot);

    ri_max = max(ri_top, ri_bot);
    ri_min = min(ri_top, ri_bot);

    o_slope = (ro_bot - ro_top)/thick;
    i_slope = (ri_bot - ri_top)/thick;

    const_width = ((ro_bot - ri_bot) - (ro_top - ri_top))  < 1E-5;

    vout = M_PI/3.*(3*ro_max+thick*fabs(o_slope))*thick*thick*fabs(o_slope);
    vout += M_PI*ro_min*ro_min*thick;

    vin  = M_PI/3.*(3*ri_max+thick*fabs(i_slope))*thick*thick*fabs(i_slope);
    vin += M_PI*ri_min*ri_min*thick;

    volume = vout - vin;

}

EGS_Vector CSSSLayer::getPoint(EGS_RandomGenerator *rndm) {

    EGS_Float r, z;

    if (const_width) {
        getRZEqualWidth(rndm, r, z);
    }
    else {
        getRZRejection(rndm, r, z);

    }

    EGS_Vector point = getPointInCircleAtZ(rndm, r, z);
    point.z += zo;
    return point;
}

void CSSSLayer::getRZEqualWidth(EGS_RandomGenerator *rndm, EGS_Float &r, EGS_Float &z) {

    z = thick*(rndm->getUniform());
    EGS_Float ri = getRiAtZ(z);
    EGS_Float ro = getRoAtZ(z);
    r = ri+(ro-ri)*sqrt(rndm->getUniform());
}

void CSSSLayer::getRZRejection(EGS_RandomGenerator *rndm, EGS_Float &r, EGS_Float &z) {

    int count = 0;

    while (1) {
        z = thick*(rndm->getUniform());
        r = ri_min+(ro_max-ri_min)*rndm->getUniform();

        if (r <= getRoAtZ(z) && r >= getRiAtZ(z)) {
            EGS_Vector point = getPointInCircleAtZ(rndm, r, z);
            point.z += zo;
            return;
        }

        if (count++ > 1000) {
            egsWarning("egs_conical_shell: Less than .1%% of random points are being accepted");
        }

    }

}


EGS_Vector CSSSLayer::getPointInCircleAtZ(EGS_RandomGenerator *rndm, EGS_Float r, EGS_Float z) {

    EGS_Float cphi, sphi;
    rndm->getAzimuth(cphi,sphi);
    return EGS_Vector(r*cphi, r*sphi, z);

};

EGS_Float CSSSLayer::getRoAtZ(EGS_Float z) {
    return o_slope*z+ro_top;
}

EGS_Float CSSSLayer::getRiAtZ(EGS_Float z) {
    return i_slope*z+ri_top;
}



/*! \brief Construct a concical shell with midpoint \a Xo */
EGS_ConicalShellStackShape::EGS_ConicalShellStackShape(const EGS_Vector &Xo, const string &Name, EGS_ObjectFactory *f):
    EGS_BaseShape(Name, f), layer_sampler(0), xo(Xo) {
    otype = "conicalShellStack";
    total_thick = 0;
};

/*! \brief Returns a random point within the conical shell. */
EGS_Vector EGS_ConicalShellStackShape::getPoint(EGS_RandomGenerator *rndm) {

    int lyr= layer_sampler->sample(rndm) ;
    CSSSLayer *layer = layers[lyr];
    EGS_Vector point = layer->getPoint(rndm);
    return xo + point;

};

void EGS_ConicalShellStackShape::addLayer(EGS_Float thick,
        EGS_Float ri_top, EGS_Float ro_top, EGS_Float ri_bot,EGS_Float ro_bot) {

    CSSSLayer *layer = new CSSSLayer(thick, ri_top, ro_top, ri_bot, ro_bot, total_thick);
    layers.push_back(layer);
    total_thick += thick;

    volumes.push_back(layer->volume);

    setLayerSampler();

}


void EGS_ConicalShellStackShape::addLayer(EGS_Float thick, EGS_Float ri_bot,EGS_Float ro_bot) {

    EGS_Float ri_top = layers[layers.size()-1]->ri_bot;
    EGS_Float ro_top = layers[layers.size()-1]->ro_bot;

    addLayer(thick, ri_top, ro_top, ri_bot, ro_bot);

}

void EGS_ConicalShellStackShape::setLayerSampler() {

    if (layer_sampler) {
        delete layer_sampler;
    }

    layer_sampler = new EGS_SimpleAliasTable(volumes.size(), &volumes[0]);
}


extern "C" {

    EGS_CONICAL_SHELL_EXPORT EGS_BaseShape *createShape(EGS_Input *input, EGS_ObjectFactory *f) {

        if (!input) {
            egsWarning("createShape(conicalShell): null input?\n");
            return 0;
        }


        vector<EGS_Float> xo;
        int err = input->getInput("midpoint", xo);
        if (err || xo.size() != 3) {
            xo.clear();
            xo.push_back(0);
            xo.push_back(0);
            xo.push_back(0);
        }

        EGS_ConicalShellStackShape *result = new EGS_ConicalShellStackShape(EGS_Vector(xo[0],xo[1],xo[2]));
        result->setName(input);

        EGS_Input *layer;
        int nl = 0;
        while ((layer = input->takeInputItem("layer"))) {
            vector<EGS_Float> rtop, rbot;
            EGS_Float thick;
            err = layer->getInput("thickness", thick);
            if (err) {
                egsWarning(
                    "createShape(EGS_ConicalShellStackShape): missing 'thickness'"
                    " input for layer %d\n  --> layer ignored\n", nl
                );
            }
            else {

                err = layer->getInput("top radii",rtop);
                if (err && nl==0) {
                    egsWarning("createGeometry(EGS_ConeStack): missing 'top radii' input for 1st layer\n");
                }
                else {
                    err=0;
                }

                int err1 = layer->getInput("bottom radii",rbot);
                if (err1) {
                    egsWarning("createGeometry(EGS_ConeStack): missing 'bottom radii' input for layer %d\n",nl);
                }
                if (err || err1) {
                    egsWarning("  --> layer ignored\n");
                }
                else {

                    EGS_Float rit, rot, rib, rob;
                    if (rbot.size() < 2) {
                        rib = 0;
                        rob = rbot[0];
                    }
                    else {
                        rib = rbot[0];
                        rob = rbot[1];
                    }

                    if (rtop.size() == 0) {
                        result->addLayer(thick, rib, rob);
                    }
                    else if (rtop.size() < 2) {
                        rit = 0;
                        rot = rtop[0];
                        result->addLayer(thick, rit, rot, rib, rob);
                    }
                    else {
                        rit = rtop[0];
                        rot = rtop[1];
                        result->addLayer(thick, rit, rot, rib, rob);
                    }
                }
            }
            delete layer;
            ++nl;
        }

        return result;
    }

}
