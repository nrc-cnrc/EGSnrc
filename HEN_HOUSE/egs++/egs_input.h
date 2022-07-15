/*
###############################################################################
#
#  EGSnrc egs++ input headers
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


/*! \file egs_input.h
 *  \brief EGS_Input class header file
 *  \IK
 */

#ifndef EGS_INPUT_
#define EGS_INPUT_
#include "egs_libconfig.h"

#include <string>
#include <vector>
#include <iostream>
using namespace std;


class EGS_InputPrivate;

/*! \brief A class for storing information in a tree-like structure
 of key-value pairs. This class is used throughout the egspp class library
 for passing information to the various objects.

 \ingroup egspp_main

 An EGS_Input object contains a tree-like structure of elementary or
 composite properties, which are instances of EGS_Input.
 In the simplest possible scenario an EGS_Input object is simply
 the combination of a key and a corresponding value, \em i.e
 \verbatim
 key = value
 \endverbatim
 This is referred to
 as an elementary property. A composite property (or EGS_Input object)
 has several elementary or composite properties, \em i.e.
 \verbatim
  key
      key1 = value1
      key2 = value2
      key3
          key3a = value3a
          key3b = value3b
      key4 = value4
      ...
 \endverbatim
 The name of an EGS_Input object can be obtained using the name() function.
 A sub-property with name \a Name can be obtained using getInputItem() or
 takeInputItem(). Given an elementary property, one can use the various
 getInput() functions to parse the value as a single integer, floating point
 number or string or as an array of integers, floating point numbers or
 strings. The content of an EGS_Input object can be set from a file
 using setContentFromFile() or addContentFromFile() or from a string
 using setContentFromString() or addContentFromString(). These 4 methods
 use the following syntax rules to put the content of the file or string
 into the tree-like key-value structure described above:
 - Input following a \c \# character is ignored until the end of a line
 - The new line character delimits between subsequent key-value pairs
 - If a value is to be continued on the next line, a line continuation
   with a trailing comma is required
 - Elementary properties are of the form <br>
     key = value<br>
 - Composite properties are defined using
 \verbatim
  :start some_key:
      subkey1 = value1
      subkey2 = value2
      ...
  :stop some_key:
 \endverbatim
 where \c some_key vecomes the name (or key) of the  composite property
 - Composite properties can be nested, \em e.g.
 \verbatim
  :start key1:
      :start key2:
          subkey2 = value
          subkey3 = value
      :stop key2:
      key 3 = value
  :stop key1:
 \endverbatim
 Note: the indentation is not required, it is only used for better
 visibility

 Since the 2008 version of the C++ class library
 it is possible to include the content of some other file into the input file
 using
 \verbatim
 include file = some_file
 \endverbatim
 In this way one can define e.g. the geometry in a separate file and then
 include it into the input file for different runs. Note that the file
 is searched for in the current working directory, so for batch execution
 one has to add the absolute path to the file name.

 Also since the 2008 version of the C++ class library one has the
 ability to use so called input loops. Input loops are useful for
 saving typing for repetitive input where only relatively little changes.
 To do so, one encloses the to be repeated input between
 <code>:start input loop:</code> and <code>:stop input loop:</code> delimeters
 as follows:
 \verbatim
 :start input loop:
     loop count = N
     loop variable = 0 var1 v1min v1delta
     loop variable = 1 var2 v3min v3delta
     loop variable = 2 var3 list_of_N_string_values
     some other input
 :stop input loop:
 \endverbatim
 Then, everything in the input loop block except for
 the definition of loop count and loop variables will be repeated
 N times, replacing all occurences of <code>\$(var1)</code> with <code>v1min+v1delta*i</code>,
 of <code>\$(var2)</code> with <code>v2min+v2delta*i</code>, and of
 <code>\$(var3)</code> with the i'th
 string provided in the list of strings. The first integer in the
 definition of a loop variable specifies its type (0 = integer, 1 = float,
 2 = list of strings).
 Note that input loops can also be nested. To give a concrete example, consider
 \verbatim
 :start input loop:
     loop count = 3
     loop variable = 0 var1 1 1
     loop varaible = 2 var2 Blake Ernesto Iwan
     :start input loop:
         loop count = 2
         loop variable = 0 var3 10 5
         $(var2) = $(var1) $(var3)
     :stop input loop:
  :stop input loop:
 \endverbatim
 will get expanded to
 \verbatim
 Blake = 1 10
 Blake = 1 15
 Ernesto = 2 10
 Ernesto = 2 15
 Iwan = 3 10
 Iwan = 3 15
 \endverbatim

 \todo Add the possibility to replace values with previously defined
 values using \em e.g. make syntax:
 <code>media = \$(air_medium) \$(water_medium)</code>
 should initiate a search for a previous definition of \c air_medium
 and \c water_medium and should replace them, if such definitions exist.
 This would be useful for \em e.g. easier change of the media in a
 geometry definition if one wanted to use different PEGS data sets.

 \todo Extend the class to allow the definition of a required input
 by defining which keys are required or optional. This would need the
 possibility of grouping keys when alternative definitions are possible.
 */
class EGS_EXPORT EGS_Input {

    EGS_InputPrivate  *p; //!< Used for hiding the implementation details.

public:

    /*! \brief Create an empty property (no key and no value). */
    EGS_Input();

    /*! \brief Copy constructor
     *
     * Note: this uses a shallow copy so that changes made to \a o also
     * affect the new object and vice versa.
     */
    EGS_Input(const EGS_Input &o);

    /*! \brief Create an elementary property named \a name having a value
     * \a value.
     */
    EGS_Input(const string &name, const string &value="");

    /*! \brief Destructor. */
    ~EGS_Input();

    /*! \brief Get the name of this property. */
    const char *name() const;

    /*! \brief Set the property from the input file \a fname
     * (which is considered to be an absolute file name)
     *
     * Any previous content is discarded and then the input file is read
     * and parsed according to the rules described above.
     * \sa setContentFromString(), addContentFromFile(), addContentFromString()
     */
    int setContentFromFile(const char *fname);

    /*! \overload */
    int setContentFromString(string &input);

    /*! \brief Add the content of the file \a fname to this EGS_Input object.
     *
     * \sa setContentFromFile(), setContentFromString(), addContentFromString()
     */
    int addContentFromFile(const char *fname);

    /*! \overload */
    int addContentFromString(string &input);

    /*! \brief Get the property named \a key.
     *
     * The item is removed from the object and ownership is transfered to
     * the caller, who is responsible for deleting the item. If \a self is
     * \c true, it is first checked if the object itself is named \a key
     * and if yes, the object itself is returned and the current object becomes
     * an empty property. If \a self is \c false or if the object name is
     * not \a key, this function checks if the object contains a child property
     * named \a key and if yes, returns it. Otherwise \c null is returned.
     */
    EGS_Input *takeInputItem(const string &key, bool self=true);

    /*! \brief Same as the previous function but now ownership remains with the
      EGS_Input object.
     */
    EGS_Input *getInputItem(const string &key) const;

    /*! \brief Add the input \a i to this property */
    void addInputItem(const EGS_Input &i);

    /*! \brief Assign values to an array of strings from an input
        identified by key.

      Returns 0 on success or an error code on failure.
      The function fails if the object is not named \a key and
      does not have a child property named \a key (return value is -1),
      or if a property named \a key exists but an error occured while
      reading the value as an array of strings (returns 1 or 2).
     */
    int getInput(const string &key, vector<string> &values) const;

    /*! \brief Assign values to an array of floats from an input
        identified by key.

        \see getInput(const string &,vector<string> &)
     */
    int getInput(const string &key, vector<EGS_Float> &values) const;

    /*! \brief Assign values to an array of integers from an input
        identified by key.

        \see getInput(const string &,vector<string> &)
     */
    int getInput(const string &key, vector<int> &values) const;

    /*! \brief Assign values to a single string from an input
        identified by key.

        Returns 0 on success or an error code on failure.
        The function fails if the object is not named \a key and
        does not have a child property named \a key (return value is -1),
        or if a property named \a key exists but an error occured while
        reading the value as a string (returns 1 or 2).
     */
    int getInput(const string &key, string &value) const;

    /*! \brief Assign values to a float from an input
        identified by key.

        \see getInput(const string &,string &)
     */
    int getInput(const string &key, float &value) const;

    /*! \brief Assign values to an integer from an input
        identified by key.

        \see getInput(const string &,string &)
     */
    int getInput(const string &key, double &value) const;

    /*! \brief Assign values to a double from an input
        identified by key.

        \see getInput(const string &,string &)
     */
    int getInput(const string &key, int &value) const;

    /*! \brief Assign values to a 64 bit integer from an input
        identified by key.

        \see getInput(const string &,string &)
     */
    int getInput(const string &key, EGS_I64 &value) const;

    /*!  \brief Get input from a range of allowed values

      If the input was not found or some other error occured,
      this function returns the integer specified by \a def,
      otherwise it returns the index of the input in the array of
      allowed values \a allowed. If \a found is not null, it is
      set to \c true if everything worked ok and to \c false,
      if some error occured.
     */
    int getInput(const string &key, const vector<string> &allowed,
                 int def=0, bool *found=0) const;

    /*! Is this a property named \a key? */
    bool isA(const string &key) const;

    /*! Compare the strings \a s1 and \a s2.
     *
     * Comparison is case insensitive and also all white space is
     * removed from \a s1 and \a s2 before performing the comparison.
     */
    static bool compare(const string &s1, const string &s2);

    /*! \brief Used for debugging purposes */
    void print(int nind, ostream &);

};

#endif


