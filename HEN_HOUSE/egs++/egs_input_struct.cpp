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

void EGS_InputStruct::addBlockInput(shared_ptr<EGS_BlockInput> block) {
    blockInputs.push_back(block);
}

void EGS_InputStruct::addBlockInputs(vector<shared_ptr<EGS_BlockInput>> blocks) {
    egsInformation("testA EGS_InputStruct::addBlockInputs\n");
    blockInputs.insert(blockInputs.end(), blocks.begin(), blocks.end());
}

vector<shared_ptr<EGS_BlockInput>> EGS_InputStruct::getBlockInputs() {
    return blockInputs;
}

shared_ptr<EGS_BlockInput> EGS_InputStruct::getLibraryBlock(string blockTitle, string libraryName) {
    // Loop through each input block in the structure to find the library with
    // the matching name
    auto libraryBlock = make_shared<EGS_BlockInput>();
    for(auto& block : blockInputs) {
        libraryBlock = block->getLibraryBlock(blockTitle, libraryName);
        if(libraryBlock) {
            break;
        }
    }
    return libraryBlock;
}

vector<string> EGS_InputStruct::getLibraryOptions(string blockTitle) {
    // Loop through each input block in the structure to find all the possible
    // library options that match the input block type
    // E.g. find all the geometry libraries
    vector<string> libOptions;
    for(auto& block : blockInputs) {
        if(block && block->getTitle() == blockTitle) {
            string lib = block->getSingleInput("library")->getValues().front();
            if(lib.size() > 0) {
                libOptions.push_back(lib);
            }
        }
    }
    return libOptions;
}

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
    singleInputs.push_back(make_shared<EGS_SingleInput>(attr, isReq, desc, vals));
}

shared_ptr<EGS_BlockInput> EGS_BlockInput::addBlockInput(string blockTit, bool isReq) {
    egsInformation("addBlockInput\n");
    blockInputs.push_back(make_shared<EGS_BlockInput>(blockTit, isReq, shared_from_this()));

    return blockInputs.back();
}

vector<shared_ptr<EGS_SingleInput>> EGS_BlockInput::getSingleInputs() {
    return singleInputs;
}

vector<shared_ptr<EGS_BlockInput>> EGS_BlockInput::getBlockInputs() {
    return blockInputs;
}

shared_ptr<EGS_SingleInput> EGS_BlockInput::getSingleInput(string attr) {
    for(auto& inp : singleInputs) {
        // TODO: this assumes unique attr
        if(inp && inp->getAttribute() == attr) {
            return inp;
        }
    }

    return nullptr;
}

void EGS_BlockInput::setParent(shared_ptr<EGS_BlockInput> par) {
    parent = par;
}

shared_ptr<EGS_BlockInput> EGS_BlockInput::getParent() {
    return parent;
}

shared_ptr<EGS_BlockInput> EGS_BlockInput::getLibraryBlock(string blockTitle, string libraryName) {

    // First search the singleInputs for the library name
    // only if the block title matches (e.g. it's a geometry, or a source)
    if(this->getTitle() == blockTitle) {
        for(auto &inp: singleInputs) {
            if(!inp) {
                continue;
            }
            if(egsEquivStr(inp->getAttribute(), "library")) {
                if(inp->getValues().size() && egsEquivStr(inp->getValues().front(), libraryName)) {
                    return shared_from_this();
                } else {
                    break;
                }
            }
        }
    }

    // If not found, go through input blocks
    for(auto &block: blockInputs) {
        auto libraryBlock = block->getLibraryBlock(blockTitle, libraryName);
        if(libraryBlock) {
            return libraryBlock;
        }
    }
    return nullptr;
}

bool EGS_BlockInput::contains(string inputTag) {
    for(auto &inp: singleInputs) {
        if(!inp) {
            continue;
        }
        if(egsEquivStr(inp->getAttribute(), inputTag)) {
            return true;
        }
    }
    return false;
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

string EGS_SingleInput::getDescription() {
    return description;
}





