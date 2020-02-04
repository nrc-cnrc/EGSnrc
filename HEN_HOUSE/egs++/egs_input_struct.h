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

class EGS_EXPORT EGS_SingleInput {

public:
    EGS_SingleInput();
    EGS_SingleInput(string attr, bool isReq, const string desc, const vector<string> vals);
    string getAttribute();
    bool getRequired();
    ~EGS_SingleInput();
    const vector<string> getValues();
    string getDescription();

protected:

    void addRequirement(string attr, string value="");
    vector<EGS_SingleInput> getDependents();

private:

    vector<EGS_SingleInput> dependents;
    vector<string> requirements;
    string attribute;
    bool isRequired;
    string description;
    vector<string> values;
};

class EGS_EXPORT EGS_BlockInput
    : public std::enable_shared_from_this<EGS_BlockInput> {
public:
    EGS_BlockInput();
    EGS_BlockInput(string blockTit, bool isReq = false, shared_ptr<EGS_BlockInput> par = nullptr);
    ~EGS_BlockInput();

    void setTitle(string blockTit);
    string getTitle();
    void addSingleInput(string attr, bool isReq, const string desc, const vector<string> vals = vector<string>());
    shared_ptr<EGS_BlockInput> addBlockInput(string blockTit, bool isReq = false);
    vector<shared_ptr<EGS_SingleInput>> getSingleInputs();
    vector<shared_ptr<EGS_BlockInput>> getBlockInputs();
    shared_ptr<EGS_SingleInput> getSingleInput(string attr);
    void setParent(shared_ptr<EGS_BlockInput> par);
    shared_ptr<EGS_BlockInput> getParent();
    shared_ptr<EGS_BlockInput> getLibraryBlock(string blockTitle, string libraryName);
    bool contains(string inputTag);


private:

    vector<shared_ptr<EGS_SingleInput>> singleInputs;
    vector<shared_ptr<EGS_BlockInput>> blockInputs;
    shared_ptr<EGS_BlockInput> parent;
    string blockTitle;
    bool isRequired;
    const string desc;
};

class EGS_EXPORT EGS_InputStruct {
public:
    EGS_InputStruct();
    ~EGS_InputStruct();

    void addBlockInput(shared_ptr<EGS_BlockInput> block);
    //void addBlockInput(string blockTit, bool isReq);
    void addBlockInputs(vector<shared_ptr<EGS_BlockInput>> blocks);
    vector<shared_ptr<EGS_BlockInput>> getBlockInputs();
    shared_ptr<EGS_BlockInput> getLibraryBlock(string blockTitle, string libraryName);
    vector<string> getLibraryOptions(string blockTitle);

private:

    vector<shared_ptr<EGS_BlockInput>> blockInputs;
};


#endif


