/*
###############################################################################
#
#  EGSnrc egs++ ausgab object
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
#  Author:          Iwan Kawrakow, 2009
#
#  Contributors:    Reid Townson
#
###############################################################################
*/


/*! \file egs_ausgab_object.cpp
 *  \brief EGS_AusgabObject base ausgab object implementation
 *  \IK
 *
 *  Also constructs a static typed object factory used to manage ausgab objects
 *  created so far.
 */

#include "egs_ausgab_object.h"
#include "egs_input.h"

static EGS_LOCAL EGS_TypedObjectFactory<EGS_AusgabObject>
ausgab_object_creator(string("egs++/dso/")+CONFIG_NAME,"EGS_AusgabObject");

void EGS_AusgabObject::createAusgabObjects(EGS_Input *i) {
    ausgab_object_creator.createObjects(i,"ausgab object definition",
                                        "ausgab object","__no__key__","createAusgabObject",true);
}

EGS_AusgabObject *EGS_AusgabObject::getAusgabObject(const string &Name) {
    EGS_Object *o = ausgab_object_creator.getObject(Name);
    if (!o) {
        return 0;
    }
    EGS_AusgabObject *s = dynamic_cast<EGS_AusgabObject *>(o);
    if (s) {
        return s;
    }
    egsWarning("EGS_AusgabObject::getAusgabObject(): dynamic cast failed?\n"
               "  Object named %s is of type %s. Trying simple cast\n",
               Name.c_str(),o->getObjectType().c_str());
    return (EGS_AusgabObject *)o;
}

void EGS_AusgabObject::addKnownAusgabObject(EGS_AusgabObject *o) {
    if (o) egsInformation("Adding known ausgab object of type %s\n",
                              o->getObjectType().c_str());
    ausgab_object_creator.addKnownObject(o);
}

void EGS_AusgabObject::addKnownTypeId(const char *tid) {
    ausgab_object_creator.addKnownTypeId(tid);
}

int EGS_AusgabObject::nObjects() {
    return ausgab_object_creator.nObjects();
}

EGS_AusgabObject *EGS_AusgabObject::getObject(int j) {
    return (EGS_AusgabObject *) ausgab_object_creator.getObject(j);
}
