/*
###############################################################################
#
#  EGSnrc egs++ object factory headers
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
#  Contributors:    Reid Townson
#
###############################################################################
*/


/*! \file egs_object_factory.h
 *  \brief EGS_Object and EGS_ObjectFactory class header file
 *  \IK
 */

#ifndef EGS_OBJECT_FACTORY_
#define EGS_OBJECT_FACTORY_

#include "egs_libconfig.h"
#include "egs_functions.h"

#include <string>
#include <vector>
#include <typeinfo>
using namespace std;

class EGS_Input;
class EGS_ObjectFactory;

/*! \brief Base egspp object.

  \ingroup egspp_main

  The purpose of the EGS_Object class, together with
  the EGS_ObjectFactory class, is to provide the main
  functionality related to dynamically producing objects based
  on user input by loading the specified shared library and
  calling the object creation function provided by the library.
  This functionality is needed by \link Sources particle sources, \endlink
  \link Shapes shapes \endlink and
  \link Geometry geometries. \endlink Whereas particle sources and shapes
  are derived from the EGS_Object class, geometries are not. This is
  due to the fact that geometries were implemented first, before the
  plan emerged to develop a more complete class library for the EGSnrc
  system. This inconsistency may be removed in future versions of the
  egspp package.

  EGS_Object objects have a name that can be obtained or set
  using getObjectName() or setObjectName() and a type that is
  obtained with getObjectType(). They can set their name from  the
  information provided by an EGS_Input object and they also can
  create instances of their respective class based on such information
  with the createObject() function. EGS_Object instances can be safely
  shared between several objects by increasing and decreasing the
  reference count to the EGS_Object instance using ref() and deref().
  Lists of EGS_Object objects are typically maintained by
  \link EGS_ObjectFactory object factories \endlink.

*/
class EGS_EXPORT EGS_Object {

public:

    /*! \brief Create an EGS_Object named \a Name belonging to the object
      factory \a f.

    The object type is set to "EGS_Object" and the reference count is
    initialized to zero. If \a Name is empty, the object name is det
    to a unique name obtained from getUniqueName().
    */
    EGS_Object(const string &Name = "", EGS_ObjectFactory *f = 0);

    /*! \brief Create an EGS_Object from the information pointed to
      by \a inp that belongs to object factory \a f.

      The only difference to the previous constructor is that now
      the object name is set using setName().
    */
    EGS_Object(EGS_Input *inp, EGS_ObjectFactory *f = 0);
    virtual ~EGS_Object();

    /*! \brief Get the object name. */
    const string &getObjectName() const {
        return name;
    };

    /*! \brief Set the object name to \a Name */
    void setObjectName(const string &Name) {
        name = Name;
    };

    /*! \brief Get the object type */
    const string &getObjectType() const {
        return otype;
    };

    /*! \brief Create an object from the infromation pointed to by \a inp

      This virtual function must be re-implemented in derived classes to
      create an instance of the respective class from the information
      provided by \a inp, if this information is valid and sufficient,
      and to return a pointer to it. Otherwise the return value should be
      \a null (which is the default implementation)
    */
    virtual EGS_Object *createObject(EGS_Input *inp) {
        return 0;
    };

    /*! \brief Create and return a unique object name

      If \a o is not \c null, the resulting name will be "%s_%d"
      where the %s is filled with o->getObjectType() and %d with
      the number of objects created so far. If \a o is \c null,
      the resulting name will be "object_%d", with %d again
      filled with the number of objects created so far.
    */
    static string getUniqueName(const EGS_Object *o = 0);

    /*! \brief Set the name of the object from the information provided by
      \a inp.

    If \a inp has a \c name key, the name is set to the value of this key.
    Otherwise the name is set using getUniqueName().
    */
    void setName(EGS_Input *inp);

    /*! \brief Increase the reference count to this object */
    inline int ref() {
        return ++nref;
    };
    /*! \brief Decrease the reference count to this object */
    inline int deref() {
        return --nref;
    };
    /*! \brief Set the factory to which the object belongs

      If the object already belongs to a different factory, it is first
      removed from this factory and then added to \a f.
    */
    void setFactory(EGS_ObjectFactory *f);

    /*! \brief Delete an object.

      This function decreases the reference count of the object and
      deletes it, if the reference count is zero.
    */
    static void deleteObject(EGS_Object *o) {
        if (!o) {
            return;
        }
        if (!o->deref()) {
            delete o;
        }
    };

protected:

    string  name;    //!< The object name
    string  otype;   //!< The object type
    int     nref;    //!< Number of references to the object
    EGS_ObjectFactory *factory; //!< The factory this object belongs to.

};

class EGS_Library;

/*! \brief An object factory

  \ingroup egspp_main

  An object factory can produce objects derived from
  EGS_Object from information stored in an instance of the
  EGS_Input class by either using the EGS_Object::createObject()
  function of "known" object types added to the factory with
  addKnownObject(), or by dynamically loading DSOs (a.k.a. DLLs)
  that provide an object creation function. This functionality
  is provided by the createSingleObject() and createObjects()
  functions. Object factories maintain lists of objects created
  so far and provide functionality for retrieving these objects
  by name. The main egspp library has static object factories
  for \link EGS_BaseSource particle sources \endlink and
  \link Shapes shapes \endlink.

*/
class EGS_EXPORT EGS_ObjectFactory {

public:

    /*! \brief Create an object factory that will load shared libraries
      from the directory \a dsoPath.

      If \a dsoPath is an absolute path name, the object factory will
      ignore \a where and look for DSOs (a.k.a. DLLs) in the directory
      \a dsoPath. If \a dsoPath is a relative path name, then the
      the path in which the factory will look for DSOs is constructed
      from \c \$HEN_HOUSE and \a dsoPath when \a where=0, else from
      \c \$EGS_HOME and \a dsoPath. In such cases, a fatal error occurs
      if the environment variable \c HEN_HOUSE or \c EGS_HOME is not set.
    */
    EGS_ObjectFactory(const string &dsoPath, int where=0);

    /*! \brief Destructor

      The destructor calls EGS_Object::deleteObject() for all known
      objects added using addKnownObject() and added to the factory
      via addObject() or createSingleObject(). It also unloads all
      DSOs that may have been loaded in order to create objects
      by deleting their EGS_Library instances.
    */
    virtual ~EGS_ObjectFactory();

    /*! Add a "known" object to the factorty.

      The factory maintains a list of "known" objects and this function
      adds \a o to the list. The list of known object is queried
      when constructing objects with createSingleObject().
    */
    virtual void addKnownObject(EGS_Object *o) {
        if (o) {
            o->ref();
            known_objects.push_back(o);
        }
    };

    /*! \brief Create a single object from the information pointed to
      by \a inp.

      If \a inp has a key \c type, the factory checks if any of the
      known objects added with addKnownObject() is of the same type
      and if yes, uses the EGS_Object::createObject() function of
      this object to create a new object of this type from the information
      provided by \a inp. If no \c type key exists or if no known
      object is of the same type, this function looks for a \c library
      key. If such a key is present, it attempts to load a DSO with name given
      by the \c library value (the \c library value must not contain
      platform specific prefixes and extensions) from the directory
      specified by the factory \link EGS_ObjectFactory::EGS_ObjectFactory
      constructor. \endlink If this is successfull, the address of
      the object creation function of the DSO is resolved using \a funcname
      (or "createObject", if \a funcname is \c null) and this function
      is called with \a inp as argument to create the new object.
      If one of the two methods succeed, the newly created object is added
      to the list of objects using addObject() with \a unique as
      argument. If this also suceeds, this function returns a pointer
      to the newly created object. In all other cases \c null is returned.
    */
    virtual EGS_Object *createSingleObject(EGS_Input *inp,
                                           const char *funcname = 0, bool unique = true);

    /*! \brief Create all objects specified by the information \a inp.

      This function creates all objects specified by the information
      pointed to by \a inp using createSingleObject().
      The implementation works as follows:
       - If \a inp is a \a section_delimeter property, it is used directly,
         otherwise the property \a section_delimeter is taken from
         \a inp and used as the input. If \a inp is neither a
         \a section_delimeter property itself nor does it have a
         \a section_delimeter property, \c null is returned
         immediately.
       - For all properties named \a object_delimeter contained in the input,
         this function calls createSingleObject() with this property
         as argument.
       - If the input has a key named \a select_key, the return value is
         the object with name specified by the \a select_key value.
         Otherwise the return value is a pointer to the last object created.

      Note that in all of the above this function uses the
      \link EGS_Input::takeInputItem() takeInputItem() \endlink function
      of the input object and therefore all \a object_delimeter properties
      and the first \a section_delimeter property are removed from \a inp
      after a call to this function.

      The meaning of the \a funcname and \a unique parameters is the same
      as in createSingleObject().
    */
    EGS_Object *createObjects(EGS_Input *inp, const string &section_delimeter,
                              const string &object_delimeter, const string &select_key,
                              const char *funcname = 0, bool unique = true);

    /*! \brief Does the factory own the object pointed to by \a o?

      This function returns \c true, if the factory owns the object
      pointed to by \a o (\em i.e. the object is in the factory's list
      of objects), \c null otherwise.
    */
    bool haveObject(const EGS_Object *o) const;

    /*! \brief Get the object named \a Name.

     This function returns a pointer to the object named \a Name, if
     such object exists, \c null otherwise. Ownership remains with
     the factory.

     \sa takeObject()
     */
    EGS_Object *getObject(const string &Name);

    /*! \brief Take the object named \a Name from the list of objects.

      If an object named \a Name exists in the list of objects,
      this function removes it from the list, decreases its reference
      count and returns a pointer to the object. This implies that
      ownership is transfered to the caller, who is reposnsible
      for deleting the object when not needed. The return value is \c null,
      if no object with name \a Name exists in the list.

      \sa getObject()
    */
    EGS_Object *takeObject(const string &Name);

    /*! \brief Remove \a o from the list of objects */
    void removeObject(EGS_Object *o);

    /*! \brief Add the object \a o to the factory's list of objects.

      This function returns \c true, if the object was successfully
      added to the list. An object can be added to the list, if \a o is
      not \c null and \a unique is \c false or \a unique is \c true \em and
      an object with the same name does not already exist in the list.
    */
    virtual bool addObject(EGS_Object *o, bool unique = true);

    /*! \brief Add a known typeid to this factory.

    */
    void addKnownTypeId(const char *typeid_name);

    /*! \brief Get the number of objects this factory has created so far */
    int  nObjects() const {
        return objects.size();
    };

    /*! \brief Get the \a j'th object */
    EGS_Object *getObject(int j) {
        return (j>=0 && j<objects.size()) ? objects[j] : 0;
    };

protected:

    vector<EGS_Library *> libs;          //!< DSOs loaded so far
    vector<EGS_Object *>  known_objects; //!< known Objects
    vector<EGS_Object *>  objects;       //!< Created objects
    vector<string>        known_typeids; //!< Known typeid's
    string dso_path;                     //!< The path to look for DSOs

};

/*! \brief A typed object factory

  \ingroup egspp_main

  Typed object factories can only create/add objects of a given
  type. This is useful for checking that \em e.g. the object
  created based on the user input actually is a
  \link EGS_BaseSource particle source \endlink
  and not some other type of EGS_Object.
*/
template <class T>
class EGS_EXPORT EGS_TypedObjectFactory : public EGS_ObjectFactory {

public:

    EGS_TypedObjectFactory(const string &dsoPath, const string &type,
                           int where=0) :
        EGS_ObjectFactory(dsoPath,where), otype(type) {};
    ~EGS_TypedObjectFactory() {};

    bool isKnownTypeId(EGS_Object *o) const {
        for (int j=0; j<known_typeids.size(); j++) {
            if (known_typeids[j] == typeid(*o).name()) {
                return true;
            }
        }
        return false;
    };

    bool isMyObjectType(EGS_Object *o, const char *func) {
        if (!o) {
            return false;
        }
        T *t = dynamic_cast<T *>(o);
        bool res;
        if (t) {
            res = true;
        }
        else {
            res = isKnownTypeId(o);
        }
        if (!res && func) egsWarning("EGS_TypedObjectFactory::%s:\n"
                                         "  dynamic_cast to %s fails for object of type %s\n"
                                         "  This object's typeid is also not in the list of know typeids\n",
                                         func,otype.c_str(),o->getObjectType().c_str());
        return res;
    };

    void addKnownObject(EGS_Object *o) {
        if (isMyObjectType(o,"addKnownObject()")) {
            EGS_ObjectFactory::addKnownObject(o);
        }
        EGS_ObjectFactory::addKnownObject(o);
    };

    EGS_Object *createSingleObject(EGS_Input *i,
                                   const char *fname = 0, bool u = true) {
        EGS_Object *o = EGS_ObjectFactory::createSingleObject(i,fname,u);
        if (o) {
            if (!isMyObjectType(o,"createSingleObject()")) {
                delete o;
                o = 0;
            }
        }
        return o;
    };

    bool addObject(EGS_Object *o, bool unique = true) {
        if (!isMyObjectType(o,"addObject()")) {
            return false;
        }
        return EGS_ObjectFactory::addObject(o,unique);
    };

private:

    string   otype;

};

#endif
