/*
###############################################################################
#
#  EGSnrc egs++ egs_cbct mortran interface
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
#  Author:          Ernesto Mainegra-Hing, 2008
#
#  Contributors:
#
###############################################################################
*/


#include "egs_mortran.h"
#include "egs_cbct.h"

extern __extc__  void
F77_OBJ_(select_photon_mfp,SELECT_PHOTON_MFP)(EGS_Float *dpmfp) {
    EGS_Application *a = EGS_Application::activeApplication();
    EGS_CBCT *app = dynamic_cast<EGS_CBCT *>(a);
    if( !app ) egsFatal("select_photon_mfp called with active application "
            " not being of type EGS_CBCT!\n");
    app->selectPhotonMFP(*dpmfp);
}

extern __extc__ void F77_OBJ_(range_discard,RANGE_DISCARD)(
        const EGS_Float *tperp, const EGS_Float *range) {
    EGS_CBCT *app = dynamic_cast<EGS_CBCT *>(
            EGS_Application::activeApplication());
    the_epcont->idisc = 0;
}