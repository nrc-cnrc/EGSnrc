/*
###############################################################################
#
#  EGSnrc egs++ object factory
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
#                   Hubert Ho
#                   Max Orok
#                   Reid Townson
#
###############################################################################
*/


/*! \file egs_object_factory.cpp
 *  \brief EGS_Object and EGS_ObjectFactory implementations
 *  \IK
*/

#include <cstdio>
#include <cstdlib>

#include "egs_object_factory.h"
#include "egs_library.h"
#include "egs_input.h"

static unsigned int object_count = 0;

EGS_Object::EGS_Object(const string &Name, EGS_ObjectFactory *f) :
    name(Name), otype("EGS_Object"), nref(0), factory(f) {
    object_count++;
    if (!name.size()) {
        name = getUniqueName(this);
    }
    //if( factory ) factory->addObject(this);
}

EGS_Object::EGS_Object(EGS_Input *input, EGS_ObjectFactory *f) :
    name(""), otype("EGS_Object"), nref(0), factory(f) {
    object_count++;
    setName(input);
    //if( factory ) factory->addObject(this);
}

EGS_Object::~EGS_Object() {
    //egsWarning("Destruncting object of type %s with name %s\n",
    //     otype.c_str(),name.c_str());
    if (factory) {
        factory->removeObject(this);
    }
}

void EGS_Object::setFactory(EGS_ObjectFactory *f) {
    if (f && f != factory) {
        if (factory) {
            factory->removeObject(this);
        }
        factory = f;
        factory->addObject(this);
    }
}

string EGS_Object::getUniqueName(const EGS_Object *o) {
    char buf[256];
    if (o) {
        sprintf(buf,"%s_%d",o->getObjectType().c_str(),object_count);
    }
    else {
        sprintf(buf,"object_%d",object_count);
    }
    string result(buf);
    return result;
}

void EGS_Object::setName(EGS_Input *input) {
    int err = 1;
    if (input) {
        err = input->getInput("name",name);
    }
    if (err) {
        name = getUniqueName(this);
    }
}

EGS_ObjectFactory::EGS_ObjectFactory(const string &dsoPath, int where) {
    //egsWarning("Creating object factory at 0x%x\n",this);
    if (egsIsAbsolutePath(dsoPath)) {
        dso_path = dsoPath;
    }
    else {
        static const char *locations[] = {"HEN_HOUSE","EGS_HOME"};
        int i = !where ? 0 : 1;
        char *loc = getenv(locations[i]);
        if (!loc)
            egsFatal("EGS_ObjectFactory: the environment variable "
                     "%s must be defined\n",locations[i]);
        dso_path = egsJoinPath(loc,dsoPath);
    }
}

EGS_ObjectFactory::~EGS_ObjectFactory() {
    unsigned int j;
    //egsWarning("Destructing object factory at 0x%x\n",this);
    //egsWarning("  - destructing known objects\n");
    for (j=0; j<known_objects.size(); j++) {
        EGS_Object::deleteObject(known_objects[j]);
    }
    //egsWarning("  - destructing added objects, objects size is %d\n",
    //        objects.size());
    for (j=0; j<objects.size(); j++) {
        EGS_Object *o = objects[j];
        //egsWarning("Deleting object at 0x%x of type %s with name %s\n",
        //        o,o->getObjectType().c_str(),o->getObjectName().c_str());
        EGS_Object::deleteObject(o);
    }
    //egsWarning("  - unloading libraries\n");
    for (j=0; j<libs.size(); j++) {
        delete libs[j];
    }
}

void EGS_ObjectFactory::removeObject(EGS_Object *o) {
    for (vector<EGS_Object *>::iterator i = objects.begin();
            i != objects.end(); i++) {
        if (o == *i) {
            // why not calling o->deref() here ?
            objects.erase(i);
            break;
        }
    }
}

typedef EGS_Object *(*EGS_ObjectCreationFunction)(EGS_Input *,
        EGS_ObjectFactory *);

EGS_Object *EGS_ObjectFactory::createObjects(EGS_Input *i,
        const string &section_delimeter, const string &object_delimeter,
        const string &select_key, const char *funcname, bool unique) {
    if (!i) {
        egsWarning("EGS_ObjectFactory::createObjects(): null input?\n");
        return 0;
    }
    EGS_Input *input = i;
    if (!i->isA(section_delimeter)) {
        input = i->takeInputItem(section_delimeter);
        if (!input) {
            egsWarning("EGS_ObjectFactory::createObjects(): the input is"
                       " not of type %s and also does not have items of this type\n",
                       section_delimeter.c_str());
            return 0;
        }
    }
    EGS_Input *ij;
    int errors = 0;
    while ((ij = input->takeInputItem(object_delimeter)) != 0) {
        EGS_Object *o = createSingleObject(ij,funcname,unique);
        if (!o) {
            errors++;
        }
        delete ij;
    }
    if (errors) egsWarning("EGS_ObjectFactory::createObjects(): %d errors"
                               " occured while creating objects\n",errors);
    string sought_object;
    EGS_Object *o = 0;
    if (objects.size() > 0) {
        o = objects[objects.size()-1];
    }
    int err = input->getInput(select_key,sought_object);
    if (!err) {
        o = getObject(sought_object);
        if (!o) egsWarning("EGS_ObjectFactory::createObjects(): an object "
                               "with the name %s does not exist\n",sought_object.c_str());
    }
    delete input;
    return o;
}

EGS_Object *EGS_ObjectFactory::createSingleObject(EGS_Input *i,
        const char *funcname, bool unique) {
    if (!i) {
        egsWarning("EGS_ObjectFactory::createSingleObject(): null input?\n");
        return 0;
    }
    string type;
    int err = i->getInput("type",type);
    if (!err) {
        for (unsigned int j=0; j<known_objects.size(); j++) {
            if (i->compare(type,known_objects[j]->getObjectType())) {
                EGS_Object *o = known_objects[j]->createObject(i);
                if (addObject(o,unique)) {
                    return o;
                }
                EGS_Object::deleteObject(o);
                return 0;
            }
        }
    }
    string libname;
    int error = i->getInput("library",libname);
    if (error) {
        if (err) egsWarning("EGS_ObjectFactory::createObject(): \n"
                                "  input item %s does not define an object type or an object "
                                "library\n",i->name());
        else egsWarning("EGS_ObjectFactory::createObject(): input item %s\n"
                            "  don't know anything about object type %s and no object"
                            "library defined\n",i->name(),type.c_str());
        return 0;
    }
    EGS_Library *lib = 0;
    for (unsigned int j=0; j<libs.size(); j++) {
        if (libname == libs[j]->libraryName()) {
            lib = libs[j];
            break;
        }
    }
    if (!lib) {
        lib = new EGS_Library(libname.c_str(),dso_path.c_str());
        lib->load();
        if (!lib->isLoaded()) {
            egsWarning("EGS_ObjectFactory::createObject(): "
                       "failed to load the library %s from %s\n",
                       libname.c_str(),dso_path.c_str());
            return 0;
        }
        libs.push_back(lib);
    }
    EGS_ObjectCreationFunction create;
    const char *fname = funcname ? funcname : "createObject";
    create = (EGS_ObjectCreationFunction) lib->resolve(fname);
    if (!create) {
        egsWarning("EGS_ObjectFactory::createObject():\n"
                   "  failed to resolve the '%s' function in the library %s\n",
                   fname,lib->libraryName());
        return 0;
    }
    EGS_Object *o = create(i,this);
    if (addObject(o,unique)) {
        return o;
    }
    EGS_Object::deleteObject(o);
    return 0;
}

bool EGS_ObjectFactory::addObject(EGS_Object *o, bool unique) {
    if (!o) {
        egsWarning("EGS_ObjectFactory::addObject(): attempt to add a null"
                   " object\n");
        return false;
    }
    for (unsigned int j=0; j<objects.size(); j++) {
        if (objects[j] == o) {
            return true;
        }
    }
    if (unique) {
        for (unsigned int j=0; j<objects.size(); j++) {
            if (o->getObjectName() == objects[j]->getObjectName()) {
                egsWarning("EGS_ObjectFactory::addObject(): an object with "
                           "the name %s already exists\n",o->getObjectName().c_str());
                //if( o->deref() == -1 ) delete o;
                return false;
            }
        }
    }
    //egsWarning("adding an object with name '%s' of type '%s' at 0x%x\n",
    //        o->getObjectName().c_str(),o->getObjectType().c_str(),o);
    objects.push_back(o);
    o->setFactory(this);
    return true;
}

bool EGS_ObjectFactory::haveObject(const EGS_Object *o) const {
    for (unsigned int j=0; j<objects.size(); j++) {
        if (objects[j] == o) {
            return true;
        }
    }
    return false;
}

EGS_Object *EGS_ObjectFactory::getObject(const string &name) {
    for (unsigned int j=0; j<objects.size(); j++) {
        if (name == objects[j]->getObjectName()) {
            return objects[j];
        }
    }
    return 0;
}

EGS_Object *EGS_ObjectFactory::takeObject(const string &name) {
    for (vector<EGS_Object *>::iterator i = objects.begin();
            i != objects.end(); i++) {
        if ((*i)->getObjectName() == name) {
            (*i)->deref();
            objects.erase(i);
            return *i;
        }
    }
    return 0;
}

void EGS_ObjectFactory::addKnownTypeId(const char *typeid_name) {
    if (!typeid_name) {
        return;
    }
    for (int j=0; j<known_typeids.size(); j++)
        if (known_typeids[j] == typeid_name) {
            return;
        }
    known_typeids.push_back(typeid_name);
}
