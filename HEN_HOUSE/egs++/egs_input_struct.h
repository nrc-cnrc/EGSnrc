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
    EGS_SingleInput(string inputTag, bool isReq, const string desc, const vector<string> vals);
    ~EGS_SingleInput();

    string getTag();
    bool getRequired();
    void addDependency(shared_ptr<EGS_SingleInput> inp, string val="", bool isAntiDependency = false);
    vector<shared_ptr<EGS_SingleInput>> getDependencyInp();
    vector<string> getDependencyVal();
    vector<bool> getDependencyAnti();
    const vector<string> getValues();
    void setValues(const vector<string> vals);
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
};

class EGS_EXPORT EGS_BlockInput
    : public std::enable_shared_from_this<EGS_BlockInput> {
public:
    EGS_BlockInput();
    EGS_BlockInput(string blockTit, bool isReq = false, shared_ptr<EGS_BlockInput> par = nullptr);
    ~EGS_BlockInput();

    void setTitle(string blockTit);
    string getTitle();
    shared_ptr<EGS_SingleInput> addSingleInput(string inputTag, bool isReq, const string desc, const vector<string> vals = vector<string>());
    shared_ptr<EGS_BlockInput> addBlockInput(string blockTit, bool isReq = false);
    shared_ptr<EGS_BlockInput> addBlockInput(shared_ptr<EGS_BlockInput> block);
    vector<shared_ptr<EGS_SingleInput>> getSingleInputs();
    vector<shared_ptr<EGS_SingleInput>> getSingleInputs(string title);
    vector<shared_ptr<EGS_BlockInput>> getBlockInputs();
    vector<shared_ptr<EGS_BlockInput>> getBlockInputs(string title);
    shared_ptr<EGS_SingleInput> getSingleInput(string inputTag);
    shared_ptr<EGS_SingleInput> getSingleInput(string inputTag, string title);
    shared_ptr<EGS_BlockInput> getBlockInput(string title);
    void setParent(shared_ptr<EGS_BlockInput> par);
    shared_ptr<EGS_BlockInput> getParent();
    shared_ptr<EGS_BlockInput> getLibraryBlock(string blockTitle, string libraryName);
    bool contains(string inputTag);
    void addDependency(shared_ptr<EGS_SingleInput> inp, string val="");
    shared_ptr<EGS_SingleInput> getDependencyInp();
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

class EGS_EXPORT EGS_InputStruct {
public:
    EGS_InputStruct();
    ~EGS_InputStruct();

    shared_ptr<EGS_BlockInput> addBlockInput(string blockTit, bool isReq = false);
    shared_ptr<EGS_BlockInput> addBlockInput(shared_ptr<EGS_BlockInput> block);
    void addBlockInputs(vector<shared_ptr<EGS_BlockInput>> blocks);
    vector<shared_ptr<EGS_BlockInput>> getBlockInputs();
    shared_ptr<EGS_BlockInput> getBlockInput(string title);
    shared_ptr<EGS_BlockInput> getLibraryBlock(string blockTitle, string libraryName);
    vector<string> getLibraryOptions(string blockTitle);

private:

    vector<shared_ptr<EGS_BlockInput>> blockInputs;
};


#endif


