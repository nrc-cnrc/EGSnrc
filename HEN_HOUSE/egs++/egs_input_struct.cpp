/*
###############################################################################
#
#  EGSnrc egs++ input struct
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
###############################################################################
*/


/*! \file egs_input_struct.cpp
 *  \brief The input struct cpp file
 *  \RT
 *
 */

#include "egs_functions.h"
#include "egs_input_struct.h"

EGS_InputStruct::EGS_InputStruct() {}

EGS_InputStruct::~EGS_InputStruct() {}

EGS_BlockInput::EGS_BlockInput() {}

EGS_BlockInput::EGS_BlockInput(string blockTit, bool isReq, shared_ptr<EGS_BlockInput> par) {
    blockTitle = blockTit;
    isRequired = isReq;
    parent = par;
}

EGS_BlockInput::~EGS_BlockInput() {}

void EGS_BlockInput::setTitle(string blockTit) {
    blockTitle = blockTit;
}

string EGS_BlockInput::getTitle() {
    return blockTitle;
}

void EGS_BlockInput::addSingleInput(string attr, bool isReq, const string desc, const vector<string> vals) {
    singleInputs.push_back(EGS_SingleInput(attr, isReq, desc, vals));
}

shared_ptr<EGS_BlockInput> EGS_BlockInput::addBlockInput(string blockTit, bool isReq) {
    egsInformation("addBlockInput\n");
    blockInputs.push_back(make_shared<EGS_BlockInput>(blockTit, isReq, shared_from_this()));
    egsInformation("addBlockInput2\n");

    return blockInputs.back();
}

vector<EGS_SingleInput> EGS_BlockInput::getSingleInputs() {
    return singleInputs;
}

vector<shared_ptr<EGS_BlockInput>> EGS_BlockInput::getBlockInputs() {
    return blockInputs;
}

EGS_SingleInput EGS_BlockInput::getSingleInput(string attr) {
    for(auto& inp : singleInputs) {
        // TODO: this assumes unique attr
        if(inp.getAttribute() == attr) {
            return inp;
        }
    }

    return EGS_SingleInput();
}

void EGS_BlockInput::setParent(shared_ptr<EGS_BlockInput> par) {
    parent = par;
}

shared_ptr<EGS_BlockInput> EGS_BlockInput::getParent() {
    return parent;
}

EGS_SingleInput::EGS_SingleInput() {}

EGS_SingleInput::EGS_SingleInput(string attr, bool isReq, const string desc, const vector<string> vals) {
    attribute = attr;
    isRequired = isReq;
    description = desc;
    values = vals;
}

EGS_SingleInput::~EGS_SingleInput() {}

void EGS_SingleInput::addRequirement(string attr, string val) {

}

vector<EGS_SingleInput> EGS_SingleInput::getDependents() {

}

string EGS_SingleInput::getAttribute() {
    return attribute;
}

bool EGS_SingleInput::getRequired() {
    return isRequired;
}

const vector<string> EGS_SingleInput::getValues() {
    return values;
}





