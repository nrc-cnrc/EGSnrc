/*
###############################################################################
#
#  EGSnrc egs++ ausgab object headers
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
#  Contributors:
#
###############################################################################
*/


/*! \file egs_ausgab_object.h
 *  \brief EGS_AusgabObject interface class header file
 *  \IK
 */

#ifndef EGS_AUSGAB_OBJECT_
#define EGS_AUSGAB_OBJECT_

#include "egs_application.h"
#include "egs_object_factory.h"

#include <string>
#include <iostream>
using namespace std;

/*! \brief An interface class for ausgab objects

  \ingroup egspp_main

  This class defines the interface for using "ausgab objects".
  The idea is that with time we will develop generic objects for
  scoring quantites of interest (e.g. dose, fluence) or for implementing
  variance reduction techniques. Such "ausgab objects" can be
  made into shared libraries (a.k.a. DLLs) and loaded by applications
  at user requests. Such "ausgab objects" will be registered with the
  application and will be automatically called when needed.

 */

class EGS_Input;
class EGS_Application;

class EGS_EXPORT EGS_AusgabObject : public EGS_Object {

public:

    /*! \brief Construct an ausgab object named \a Name. */
    EGS_AusgabObject(const string &Name="", EGS_ObjectFactory *f = 0) : EGS_Object(Name,f), app(0) {};

    /*! \brief Construct an ausgab object from the input pointed to by \a inp.

      The property tree pointed to by \a inp must contain at least
      the following key-value pairs:<br><br><code>
      name = name of this ausgab object <br>
      library = ausgab object library <br><br></code>
      plus additional information as needed by the ausgab object being created.

    */
    EGS_AusgabObject(EGS_Input *input, EGS_ObjectFactory *f = 0) : EGS_Object(input,f), app(0) {};

    virtual ~EGS_AusgabObject() {};

    /*! \brief Process an ausgab call for event \a iarg
     *
     * Derived classes should implement scoring of quantities of interest or
     * manipulations of the particle stack in this function.
     *
     */
    virtual int processEvent(EGS_Application::AusgabCall iarg) = 0;
    virtual int processEvent(EGS_Application::AusgabCall iarg, int ir) {};

    /*! \brief Is the ausgab call \a iarg relevant for this object?
     *
     * Derived classes should re-implement this function to return \a true
     * for ausgab calls that are of interest to them.
     */
    virtual bool needsCall(EGS_Application::AusgabCall iarg) const {
        return false;
    };

    /*! \brief Set the application this object belongs to */
    virtual void setApplication(EGS_Application *App) {
        app = App;
    };

    /*! \brief Set the current event */
    virtual void setCurrentCase(EGS_I64 ncase) {};

    /*!  \brief Get a short description of this ausgab object.
     *
     *   Derived classes should set #description to a short
     *   string describing the ausgab object.
     */
    const char *getObjectDescription() const {
        return description.c_str();
    };

    /*!  \brief Store the source state into the stream \a data_out.
     *
     *   Every ausgab object should reimplement this method to store
     *   the data needed to set the state of the object to its current
     *   state into the stream \a data_out. This is used for restarted
     *   calculations. Should return \c true on success, \c false on failure.
     *   \sa setState(), addState(), resetCounter().
     */
    virtual bool storeState(ostream &data_out) const {
        return true;
    };

    /*!  \brief Set the ausgab object state based on data from the stream \a data_in.
     *
     *   Every ausgab object should reimplement this method to read from the stream
     *   \a data_in data previously stored using storeState() and to set its
     *   state according to this data. This is used for restarted calculations.
     *   Should return \c true on success, \c false on failure (\em e.g. I/O error).
     *
     *   \sa addState(), storeState(), resetCounter()
     */
    virtual bool setState(istream &data_in) {
        return true;
    };

    /*! \brief Add data from the stream \a data_in to the ausgab object state.
     *
     *  This method is required for combining the results of parallel runs.
     *  It should therefore be re-implemented in derived classes to update
     *  its own state with the data read from the input stream \a data_in.
     *
     *  \sa storeState(), setState(), resetCounter().
     */
    virtual bool addState(istream &data_in) {
        return true;
    };

    /*! \brief Reset the ausgab object state.
     *
     *  Derived ausgab objects should reimplement this method to reset all data
     *  describing their state to a "pristine" state.
     *  This is needed for combining the results of parallel
     *  runs where the generic implementation of
     *  EGS_Application::combineResults() uses this method to reset the ausgab object
     *  state and then uses the addState() function to add the ausgab object data from
     *  all other parallel jobs.
     *
     */
    virtual void resetCounter() {};

    /*! \brief Report results
     *
     * Derived classes should reimplement this function to report results
     * accumulated during the simulation.
     *
     */
    virtual void reportResults() {};

    /*! \brief Create ausgab objects from the information pointed to by \a input.
     *
     *  This static function creates all ausgab objects specified by the information
     *  stored in an EGS_Input object and pointed to by \a inp. It looks
     *  for a composite property <code>ausgab object definition</code> in the input
     *  tree. If such property exists, it looks for sub-properties
     *  <code>ausgab object</code> and for each such property that contains a \c library
     *  and \c name key-value pairs, loads the DSO specified by the \c
     *  library key, resolves the address of the \c createAusgabObject function
     *  that must be provided by the DSO and calls this function passing the
     *  <code>ausgab object</code> property to it. If the <code>ausgab object</code> property contains a valid
     *  information sufficient to create an ausgab object of the given type,
     *  the \c createAusgabObject function creates the ausgab object and returns a pointer to it.
     *  The process is continued until there are no further <code>ausgab object</code>
     *  properties in the <code>ausgab object definition</code> input.
     *  All ausgab objects created in this way are added to a global list
     *  list of ausgab objects and can be retrieved later by name using
     *  the getAusgabObject() static function.
     */
    static void createAusgabObjects(EGS_Input *);

    /*! \brief Get a pointer to the ausgab object named \a Name.
     *
     *  A static list of ausgab objects created so far is maintained internally
     *  and this method can be used to get an ausgab object with a given name.
     *  Returns a pointer to the ausgab object, if an ausgab object named \a Name exists,
     *  \c null otherwise.
     *
     */
    static EGS_AusgabObject *getAusgabObject(const string &Name);

    /*! \brief Add a known ausgab object to the ausgab object factory.
     *
     *  This function adds the object \a o to the list of known ausgab objects
     *  maintained internally by the static ausgab object factory. That way, an
     *  application can define its own ausgab objects (in addition to
     *  the ausgab objects provided by egspp) and use them.
     */
    static void addKnownAusgabObject(EGS_AusgabObject *o);

    /*! \brief Add a known ausgab object typeid to the ausgab object factory.
     *
     *  For whatever reason dynamic_cast to EGS_AusgabObject* from EGS_Object*
     *  fails when an application is made into a shared library and
     *  dynamically loads an ausgab object DSO. I'm therefore adding this method
     *  so that ausgab object classes can add their typeid to allow for an additional
     *  check in such cases.
     */
    static void addKnownTypeId(const char *name);

    /*! \brief Returns the number of ausgab objects in the internal list */
    static int nObjects();

    /*! \brief Returns the j'th ausgab object in the internal list */
    static EGS_AusgabObject *getObject(int j);

protected:

    /*! \brief A short ausgab object description.
     *
     *  Derived ausgab object classes should set this data member to a short
     *  descriptive string.
     */
    string description;

    /*! \brief The application this object belongs to */
    EGS_Application *app;

};

#endif
