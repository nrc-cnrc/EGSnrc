/*
###############################################################################
#
#  EGSnrc egs++ base source
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


/*! \file egs_base_source.cpp
 *  \brief Base source implementation
 *  \IK
 *
 *  Also constructs a static typed object factory used to manage particle
 *  sources created so far.
 */

#include "egs_base_source.h"
#include "egs_input.h"

EGS_BaseSimpleSource::EGS_BaseSimpleSource(EGS_Input *input,
        EGS_ObjectFactory *f) : EGS_BaseSource(input,f), q(0), s(0), count(0) {
    int Q;
    int err = input->getInput("charge",Q);
    if (!err) {
        q = Q;
    }
    s = EGS_BaseSpectrum::createSpectrum(input);
    if (!s) egsWarning("EGS_BaseSimpleSource::EGS_BaseSimpleSource:\n"
                           "    no spectrum was defined\n");
}

static EGS_LOCAL EGS_TypedObjectFactory<EGS_BaseSource>
source_creator(string("egs++/dso/")+CONFIG_NAME,"EGS_BaseSource");

EGS_BaseSource *EGS_BaseSource::createSource(EGS_Input *i) {
    //EGS_Object *o = source_creator.createSingleObject(i,"createSource",true);
    EGS_Object *o = source_creator.createObjects(i,"source definition",
                    "source","simulation source","createSource",true);
    return dynamic_cast<EGS_BaseSource *>(o);
}

EGS_BaseSource *EGS_BaseSource::getSource(const string &Name) {
    EGS_Object *o = source_creator.getObject(Name);
    if (!o) {
        return 0;
    }
    EGS_BaseSource *s = dynamic_cast<EGS_BaseSource *>(o);
    if (s) {
        return s;
    }
    egsWarning("EGS_BaseSource::getSource(): dynamic cast failed?\n"
               "  Object named %s is of type %s. Trying simple cast\n",
               Name.c_str(),o->getObjectType().c_str());
    return (EGS_BaseSource *)o;
}

void EGS_BaseSource::addKnownSource(EGS_BaseSource *o) {
    if (o) egsInformation("Adding known source of type %s\n",
                              o->getObjectType().c_str());
    source_creator.addKnownObject(o);
}

void EGS_BaseSource::addKnownTypeId(const char *tid) {
    source_creator.addKnownTypeId(tid);
}
