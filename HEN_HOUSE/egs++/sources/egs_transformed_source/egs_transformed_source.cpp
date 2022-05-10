/*
###############################################################################
#
#  EGSnrc egs++ transformed source
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
#  Contributors:    Hannah Gallop
#
###############################################################################
*/


/*! \file egs_transformed_source.cpp
 *  \brief A transformed source
 *  \IK
 */

#include "egs_transformed_source.h"
#include "egs_input.h"

static bool EGS_TRANSFORMED_SOURCE_LOCAL inputSet = false;

EGS_TransformedSource::EGS_TransformedSource(EGS_Input *input,
        EGS_ObjectFactory *f) : EGS_BaseSource(input,f), source(0), T(0) {
    EGS_Input *isource = input->takeInputItem("source",false);
    if (isource) {
        source = EGS_BaseSource::createSource(isource);
        delete isource;
    }
    if (!source) {
        string sname;
        int err = input->getInput("source name",sname);
        if (err)
            egsWarning("EGS_TransformedSource: missing/wrong inline source "
                       "definition and missing wrong 'source name' input\n");
        else {
            source = EGS_BaseSource::getSource(sname);
            if (!source) egsWarning("EGS_TransformedSource: a source named %s"
                                        " does not exist\n");
        }
    }
    EGS_AffineTransform *t = EGS_AffineTransform::getTransformation(input);
    setUp(t);
    delete t;
}

void EGS_TransformedSource::setUp(EGS_AffineTransform *t) {
    setTransformation(t);
    otype = "EGS_TransformedSource";
    if (!isValid()) {
        description = "Invalid transformed source";
    }
    else {
        description = "Transformed ";
        description += source->getSourceDescription();
        source->ref();
    }
}

extern "C" {

    static void setInputs() {
        inputSet = true;

        setBaseSourceInputs(false, false);

        srcBlockInput->getSingleInput("library")->setValues({"EGS_Transformed_Source"});

        // Format: name, isRequired, description, vector striing of allowed values
        srcBlockInput->addSingleInput("source name", true, "The name of a previously defined source.");

        auto blockPtr = srcBlockInput->addBlockInput("transformation");
        blockPtr->addSingleInput("translation", false, "The translation for the geometry (x, y ,z)");
        auto rotPtr = blockPtr->addSingleInput("rotation", false, "2, 3, or 9 floating point numbers");
        auto vectPtr = blockPtr->addSingleInput("rotation vector", false, "3 floating point numbers");
        // Can either have "rotation" or "rotation vector"
        rotPtr->addDependency(vectPtr, "", true);
        vectPtr->addDependency(rotPtr, "", true);
    }

    EGS_TRANSFORMED_SOURCE_EXPORT string getExample() {
        string example;
        example = {
            R"(
    # Example of egs_transformed_source
    #:start source:
        library = egs_transformed_source
        name = my_source
        source_name = my_parallel_source
        #create source called my_parallel_source
        :start transformation:
            rotation vector = 0 -1 1
        :stop transformation:
    :stop source:
)"};
        return example;
    }

    EGS_TRANSFORMED_SOURCE_EXPORT shared_ptr<EGS_BlockInput> getInputs() {
        if(!inputSet) {
            setInputs();
        }
        return srcBlockInput;
    }

    EGS_TRANSFORMED_SOURCE_EXPORT EGS_BaseSource *createSource(EGS_Input *input,
            EGS_ObjectFactory *f) {
        return
            createSourceTemplate<EGS_TransformedSource>(input,f,"transformed source");
    }

}
