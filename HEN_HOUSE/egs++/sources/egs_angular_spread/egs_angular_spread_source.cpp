/*
###############################################################################
#
#  EGSnrc egs++ angular spread source
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
#  Contributors:    Hannah Gallop
#
###############################################################################
*/


/*! \file  egs_angular_spread_source.cpp
 *  \brief Adds angular spread to the directions of particles from some other source
 *  \IK
 */

#include "egs_angular_spread_source.h"
#include "egs_input.h"
#include "egs_math.h"

static bool EGS_ANGULAR_SPREAD_SOURCE_LOCAL inputSet = false;

// 1 / (2*sqrt(2*ln(2)))
static const EGS_Float FWHM_TO_SIGMA = 0.4246609001440095285;

EGS_AngularSpreadSource::EGS_AngularSpreadSource(EGS_Input *input,
        EGS_ObjectFactory *f) : EGS_BaseSource(input,f), source(0), sigma(0) {
    EGS_Input *isource = input->takeInputItem("source",false);
    if (isource) {
        source = EGS_BaseSource::createSource(isource);
        delete isource;
    }
    if (!source) {
        string sname;
        int err = input->getInput("source name",sname);
        if (err)
            egsWarning("EGS_AngularSpreadSource: missing/wrong inline source "
                       "definition and missing wrong 'source name' input\n");
        else {
            source = EGS_BaseSource::getSource(sname);
            if (!source) egsWarning("EGS_AngularSpreadSource: a source named %s"
                                        " does not exist\n", sname.c_str());
        }
    }
    // Check for the new 'fwhm' input key first
    EGS_Float fwhm = 0;
    int err_fwhm = input->getInput("fwhm",fwhm);
    int err_sigma = input->getInput("sigma",sigma);

    if (!err_fwhm && !err_sigma) {
        egsFatal("EGS_AngularSpreadSource: both 'sigma' and 'fwhm' inputs "
                 "provided. Please use only one.\n");
    }
    else if (!err_fwhm) {
        if (fwhm < 0) {
            egsFatal("EGS_AngularSpreadSource: 'fwhm' must be positive, "
                     "got %g\n", fwhm);
        }
        // Convert FWHM (degrees) to sigma (radians), then store as sigma^2 (rad^2)
        sigma = FWHM_TO_SIGMA * fwhm;
        sigma *= M_PI/180;
        sigma *= sigma;
    }
    else if (!err_sigma) {
        if (sigma < 0) {
            egsFatal("EGS_AngularSpreadSource: negative 'sigma' input is no "
                     "longer supported. To specify the angular spread as FWHM, "
                     "use the 'fwhm' input key instead.\n");
        }
        // Convert sigma (degrees) to sigma^2 (rad^2)
        sigma *= M_PI/180;
        sigma *= sigma;
    }
    setUp();
}

void EGS_AngularSpreadSource::setUp() {
    otype = "EGS_AngularSpreadSource";
    if (!isValid()) {
        description = "Invalid angular spread source";
        return;
    }
    description = "Angular spread added to a ";
    description += source->getSourceDescription();
    source->ref();
}

extern "C" {

    static void setInputs() {
        inputSet = true;

        setBaseSourceInputs(false, false);

        srcBlockInput->getSingleInput("library")->setValues({"egs_angular_spread_source"});

        // Format: name, isRequired, description, vector string of allowed values
        srcBlockInput->addSingleInput("source name", true, "The name of a previously defined source.");
        srcBlockInput->addSingleInput("sigma", false, "If positive, it is the sigma of the Gaussian distribution in degrees. If negative, it is the FWHM of the distribution.");
    }

    EGS_ANGULAR_SPREAD_SOURCE_EXPORT string getExample() {
        string example;
        example = {
            R"(
    # Example of egs_angular_spread_source
    #:start source:
        library = egs_angular_spread_source
        name = my_source
        sigma = 10
        source name = my_parallel_source
        #create source called my_parallel_source
    :stop source:
)"};
        return example;
    }

    EGS_ANGULAR_SPREAD_SOURCE_EXPORT shared_ptr<EGS_BlockInput> getInputs() {
        if(!inputSet) {
            setInputs();
        }
        return srcBlockInput;
    }

    EGS_ANGULAR_SPREAD_SOURCE_EXPORT EGS_BaseSource *createSource(EGS_Input *input,
            EGS_ObjectFactory *f) {
        return
            createSourceTemplate<EGS_AngularSpreadSource>(input,f,"angular spread source");
    }

}
