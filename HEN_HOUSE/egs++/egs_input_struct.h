/*
###############################################################################
#
#  EGSnrc egs++ input struct headers
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
#  Author:          Reid Townson, 2019
#
#  Contributors:
#
#  An application has one global scope input struct object. Input blocks are
#  then added to contain geometries or sources. Input blocks may contain single
#  inputs.
#
###############################################################################
*/


/*! \file egs_input_struct.h
 *  \brief The input struct header file
 *  \RT
 *
 */

#ifndef EGS_INPUT_STRUCT_
#define EGS_INPUT_STRUCT_

#include <vector>
#include <string>
#include "egs_libconfig.h"
#include <memory>

using namespace std;

class EGS_BlockInput;

/*! \brief A class to represent an egsinp single line input.

    \ingroup egspp_main

    The EGS_SingleInput class is used to represent an egsinp input parameter. The class contains the ability to link whether or not this input is required, depending on a different single input parameter or input block. If there are only a few possible options, they can be set so that input values can be validated. A description of the input can also be provided.
*/
class EGS_EXPORT EGS_SingleInput {

public:
    EGS_SingleInput();

    /*! \brief Construct a single input \a inputTag.

    The input by the name \a inputTag is optional when \a isReq is false. For cases where the input is only required depending on a different input, leave \a isReq false and then use \a addDependency(). A description can be provided in \a desc. A list of valid values for the input can be provided in \a vals.
    */
    EGS_SingleInput(string inputTag, bool isReq, const string desc, const vector<string> vals);
    ~EGS_SingleInput();

    /*! \brief Get the name of the input. */
    string getTag();

    /*! \brief Get whether or not this input is required. */
    bool getRequired();

    /*! \brief Add a dependency for this input, by the name of \a inp.

    Optionally, the current input is only required if the dependency input has a value of \a val. If \a isAntiDependency is true, then the current input is required only if \a inp is not set.
    */
    void addDependency(shared_ptr<EGS_SingleInput> inp, string val = "", bool isAntiDependency = false);

    /*! \brief Add a dependency on an input block (there can only be one). */
    void addDependency(shared_ptr<EGS_BlockInput> block, bool isAntiDependency = false);

    /*! \brief Get a list of all the dependencies for this input. */
    vector<shared_ptr<EGS_SingleInput>> getDependencyInp();

    /*! \brief Get a list of the required dependency values. */
    vector<string> getDependencyVal();

    /*! \brief Get a list of whether or not these are anti-dependencies. */
    vector<bool> getDependencyAnti();

    /*! \brief Get the dependency block (can be only one). */
    shared_ptr<EGS_BlockInput> getDependencyBlock();

    /*! \brief Get whether or not the block dependency is an anti-dependency. */
    bool getDependencyBlockAnti();

    /*! \brief Get the list of possible values for this input. */
    const vector<string> getValues();

    /*! \brief Set the list of possible values for this input. */
    void setValues(const vector<string> vals);

    /*! \brief Get the description for this input. */
    string getDescription();

private:

    vector<string> requirements;
    string tag;
    bool isRequired;
    string description;
    vector<string> values;
    vector<shared_ptr<EGS_SingleInput>> dependencyInp;
    vector<string> dependencyVal;
    vector<bool> dependencyAnti;
    shared_ptr<EGS_BlockInput> dependencyBlock;
    bool dependencyBlockAnti;
};

/*! \brief A class to represent an egsinp input block.

    \ingroup egspp_main

    The EGS_BlockInput class is used to represent an egsinp input block. An input block has a title, may have a parent that it is nested within, and may contain child single inputs and input blocks.
*/
class EGS_EXPORT EGS_BlockInput
    : public std::enable_shared_from_this<EGS_BlockInput> {
public:
    EGS_BlockInput();

    /*! \brief Construct a block input \a blockTit.

    The input by the name \a blockTit is optional when \a isReq is false. If this block is always nested, specify the parent with \a par.
    */
    EGS_BlockInput(string blockTit, bool isReq = false, shared_ptr<EGS_BlockInput> par = nullptr);
    ~EGS_BlockInput();

    /*! \brief Set the title of the block. */
    void setTitle(string blockTit);

    /*! \brief Get the title of the block. */
    string getTitle();

    /*! \brief Add a single input. */
    shared_ptr<EGS_SingleInput> addSingleInput(string inputTag, bool isReq, const string desc, const vector<string> vals = vector<string>());

    /*! \brief Add an input block to be nested inside this one. */
    shared_ptr<EGS_BlockInput> addBlockInput(string blockTit, bool isReq = false);

    /*! \brief Add an already defined input block to be nested inside this one. */
    shared_ptr<EGS_BlockInput> addBlockInput(shared_ptr<EGS_BlockInput> block);

    /*! \brief Get a list of the inputs for this input block. */
    vector<shared_ptr<EGS_SingleInput>> getSingleInputs();

    /*! \brief Get a list of the inputs for the nested input block \a title. */
    vector<shared_ptr<EGS_SingleInput>> getSingleInputs(string title);

    /*! \brief Get a list of the nested input blocks for this input block. */
    vector<shared_ptr<EGS_BlockInput>> getBlockInputs();

    /*! \brief Get a list of the nested input blocks inside \a title. */
    vector<shared_ptr<EGS_BlockInput>> getBlockInputs(string title);

    /*! \brief Get the input named \a inputTag. */
    shared_ptr<EGS_SingleInput> getSingleInput(string inputTag);

    /*! \brief Get the input named \a inputTag from the input block \a title. */
    shared_ptr<EGS_SingleInput> getSingleInput(string inputTag, string title);

    /*! \brief Get the input block \a title. */
    shared_ptr<EGS_BlockInput> getBlockInput(string title);

    /*! \brief Set the parent to be \a par. */
    void setParent(shared_ptr<EGS_BlockInput> par);

    /*! \brief Get the parent input block. */
    shared_ptr<EGS_BlockInput> getParent();

    /*! \brief Get the input block containing the library tag matching \a libraryName. */
    shared_ptr<EGS_BlockInput> getLibraryBlock(string blockTitle, string libraryName);

    /*! \brief Check if this input block contains the input \a inputTag. */
    bool contains(string inputTag);

    /*! \brief Add a dependency of this input block on input \a inp being value \a val. */
    void addDependency(shared_ptr<EGS_SingleInput> inp, string val="");

    /*! \brief Get the input dependency. */
    shared_ptr<EGS_SingleInput> getDependencyInp();

    /*! \brief Get the input dependency required value. */
    string getDependencyVal();


private:

    vector<shared_ptr<EGS_SingleInput>> singleInputs;
    vector<shared_ptr<EGS_BlockInput>> blockInputs;
    shared_ptr<EGS_BlockInput> parent;
    string blockTitle;
    bool isRequired;
    const string desc;
    shared_ptr<EGS_SingleInput> dependencyInp;
    string dependencyVal;
};

/*! \brief A class to represent an egsinp input structure.

    \ingroup egspp_main

    The EGS_InputStruct class is used to represent the top level of the egsinp file structure.

    Example from egs_cylinders.cpp:

    \verbatim
    static void setInputs() {
        inputSet = true;

        // Get the inputs common to all geometries (name, library, media)
        // Builds on the global geometry input block geomBlockInput
        setBaseGeometryInputs();

        // Get the 'library' input, and set the valid values to be only 'EGS_Cylinders'
        geomBlockInput->getSingleInput("library")->setValues({"EGS_Cylinders"});

        // Add the 'type' input, required, and set the valid options for it.
        // Note that we save a reference to this input 'typePtr'.
        auto typePtr = geomBlockInput->addSingleInput("type", true, "The type of cylinder.", {"EGS_XCylinders", "EGS_YCylinders", "EGS_ZCylinders", "EGS_Cylinders"});

        // Add other inputs, 'radii' and 'midpoint'
        geomBlockInput->addSingleInput("radii", true, "A list of cylinder radii, must be in increasing order");
        geomBlockInput->addSingleInput("midpoint", false, "The position of the midpoint of the cylinder (x, y, z)");

        // Add the 'axis' input, and save a reference to it 'inpPtr'
        auto inpPtr = geomBlockInput->addSingleInput("axis", true, "The unit vector defining the axis along the length of the cylinder.");

        // Set a dependency for 'inpPtr' on 'typePrt' being set to the value 'EGS_Cylinders'. This makes it so that 'axis' is only required if 'type = EGS_Cylinders'.
        inpPtr->addDependency(typePtr, "EGS_Cylinders");
    }
    \endverbatim
*/
class EGS_EXPORT EGS_InputStruct {
public:
    EGS_InputStruct();
    ~EGS_InputStruct();

    /*! \brief Add an input block named \a blockTit. */
    shared_ptr<EGS_BlockInput> addBlockInput(string blockTit, bool isReq = false);

    /*! \brief Add an input block that is already defined as \a block. */
    shared_ptr<EGS_BlockInput> addBlockInput(shared_ptr<EGS_BlockInput> block);

    /*! \brief Add a list of input blocks that are already defined as \a blocks */
    void addBlockInputs(vector<shared_ptr<EGS_BlockInput>> blocks);

    /*! \brief Get the list of input blocks. */
    vector<shared_ptr<EGS_BlockInput>> getBlockInputs();

    /*! \brief Get the input block named \a title. */
    shared_ptr<EGS_BlockInput> getBlockInput(string title);

    /*! \brief Get the input block \a blockTitle that contains the library \a libraryName. */
    shared_ptr<EGS_BlockInput> getLibraryBlock(string blockTitle, string libraryName);

    /*! \brief Get the possible values for the library tag for the block \a blockTitle. */
    vector<string> getLibraryOptions(string blockTitle);

private:

    vector<shared_ptr<EGS_BlockInput>> blockInputs;
    vector<shared_ptr<EGS_BlockInput>> generalBlocks;
};


#endif


