/*
###############################################################################
#
#  EGSnrc egs++ egs_fac range rejection
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
#  Author:          Iwan Kawrakow, 2008
#
#  Contributors:
#
###############################################################################
*/


#include "egs_range_rejection.h"
#include "egs_interface2.h"
#include "egs_functions.h"
#include "egs_input.h"
#include "egs_rndm.h"
#include "egs_base_geometry.h"
#include "egs_mortran.h"

EGS_RangeRejection* EGS_RangeRejection::getRangeRejection(EGS_Input *input,
        EGS_Interpolator *i_ededx, EGS_Interpolator *i_pdedx) {
    EGS_Input *rr; bool delete_it;
    if( input->isA("range rejection") ) {
        rr = input; delete_it = false;
    }
    else {
        rr = input->takeInputItem("range rejection");
        if( !rr ) return 0;
        delete_it = true;
    }
    int iaux; int err = rr->getInput("rejection",iaux);
    EGS_RangeRejection *result = 0;
    if( !err && iaux > 0 ) {
        result = new EGS_RangeRejection;
        if( iaux == 1 )
            result->type = RangeDiscard;
        else {
            result->type = RussianRoulette;
            result->probi = iaux;
        }
        EGS_Float aux; err = rr->getInput("Esave",aux);
        if( !err && aux >= 0 ) result->Esave = aux;
        string cavity_geometry;
        err = rr->getInput("cavity geometry",cavity_geometry);
        if( !err ) {
            result->cgeom = EGS_BaseGeometry::getGeometry(cavity_geometry);
            if( !result->cgeom ) egsWarning("\n\n********** no geometry named"
                       " %s exists => using region-by-region rejection only\n");
        }
        if( result->Esave <= 0.511 && result->type == RangeDiscard ) {
            egsWarning("\n\n********* rr_flag = 1 but Esave = 0 =>"
                         " not using range rejection\n\n");
            delete result; result = 0;
        }
        if( result && result->type != None && result->cgeom ) {
            string rej_medium; int irej_medium = -1;
            err = rr->getInput("rejection range medium",rej_medium);
            if( !err ) {
                int imed = EGS_BaseGeometry::getMediumIndex(rej_medium);
                if( imed < 0 ) egsWarning("\n\n*********** no medium"
                    " with name %s initialized => using region-by-region rejection only\n",
                    rej_medium.c_str());
                else irej_medium = imed;
            }
            if( irej_medium < 0 ) { result->cgeom = 0; result->type = RangeDiscard; }
            else {
                //
                // *** prepare an interpolator for the electron range
                //     in the range rejection medium
                //
                int i = irej_medium; // save some typing
                EGS_Float log_emin = i_ededx[i].getXmin();
                EGS_Float log_emax = i_ededx[i].getXmax();
                int nbin = 512;
                EGS_Float dloge = (log_emax - log_emin)/nbin;
                EGS_Float *erange = new EGS_Float [nbin];
                EGS_Float *prange = new EGS_Float [nbin];
                erange[0] = 0; prange[0] = 0;
                EGS_Float ededx_old = i_ededx[i].interpolate(log_emin);
                EGS_Float pdedx_old = i_pdedx[i].interpolate(log_emin);
                EGS_Float Eold = exp(log_emin);
                EGS_Float efak = exp(dloge);
                for(int j=1; j<nbin; j++) {
                    EGS_Float elke = log_emin + dloge*j;
                    EGS_Float E = Eold*efak;
                    EGS_Float ededx = i_ededx[i].interpolate(elke);
                    EGS_Float pdedx = i_pdedx[i].interpolate(elke);
                    if( ededx < ededx_old )
                    erange[j] = erange[j-1]+1.02*(E-Eold)/ededx;
                    else
                    erange[j] = erange[j-1]+1.02*(E-Eold)/ededx_old;
                    if( pdedx < pdedx_old )
                    prange[j] = prange[j-1]+1.02*(E-Eold)/pdedx;
                    else
                    prange[j] = prange[j-1]+1.02*(E-Eold)/pdedx_old;
                    Eold = E; ededx_old = ededx; pdedx_old = pdedx;
                }
                result->erange.initialize(nbin,log_emin,log_emax,erange);
                result->prange.initialize(nbin,log_emin,log_emax,prange);
            }
        }
    }
    if( delete_it ) delete rr;
    return result;
}

EGS_RangeRejection::RejectionAction
EGS_RangeRejection::rangeDiscard(int np, EGS_Stack *stack,
                    EGS_Float tperp, EGS_Float range, bool is_cav,
                    EGS_Float logE, EGS_RandomGenerator *rndm) const {
    if( type == None ) return NoAction;
    if( (type == RangeDiscard || is_cav) && stack->E[np] > Esave ) return NoAction;
    // i.e., don't take action if type is RangeDiscard or type is RussianRoulette but
    // we are in the cavity and the energy is greater than Esave
    int q = stack->iq[np];
    RejectionAction retval = q == -1 ? DiscardE : DiscardP;
    // if here: type = RangeDiscard && E < Esave
    //  or      type = RussianRoulette && ((in cavity but E<Esave) || not in cavity)
    bool do_RR = false;
    if( range < tperp ) { // can not escape current region
        if( type == RangeDiscard || is_cav ) return retval;
        do_RR = true;
    }
    else { // can escape current region
        if( is_cav || !cgeom ) return NoAction;
        EGS_Vector x(stack->x[np],stack->y[np],stack->z[np]);
        int ireg = cgeom->isWhere(x);
        if( !cgeom->isInside(x) ) {
            EGS_Float cperp = cgeom->hownear(-1,x);
            EGS_Float crange = q == -1 ?
                erange.interpolateFast(logE) :
                prange.interpolateFast(logE);
            if( crange < cperp ) {
                if( type == RangeDiscard ) return retval;
                do_RR = true;
            }
        }
    }
    if( !do_RR ) return NoAction;
    if( rndm->getUniform()*probi < 1 ) {
        // particle survives.
        stack->wt[np] *= probi; return Survive;
    }
    return Kill; // i.e. particle is killed and must be discarded immediately.
}

EGS_RangeRejection::RejectionAction
EGS_RangeRejection::rangeDiscard(int q,
                    const EGS_Vector &x, EGS_Float E, EGS_Float logE,
                    EGS_Float tperp, EGS_Float range, bool is_cav,
                    EGS_RandomGenerator *rndm, EGS_Float &wt) const {
    if( type == None ) return NoAction;
    if( (type == RangeDiscard || is_cav) && E > Esave ) return NoAction;
    // i.e., don't take action if type is RangeDiscard or type is RussianRoulette but
    // we are in the cavity and the energy is greater than Esave
    RejectionAction retval = q == -1 ? DiscardE : DiscardP;
    // if here: type = RangeDiscard && E < Esave
    //  or      type = RussianRoulette && ((in cavity but E<Esave) || not in cavity)
    bool do_RR = false;
    if( range < tperp ) { // can not escape current region
        if( type == RangeDiscard || is_cav ) return retval;
        do_RR = true;
    }
    else { // can escape current region
        if( is_cav || !cgeom ) return NoAction;
        int ireg = cgeom->isWhere(x);
        if( !cgeom->isInside(x) ) {
            EGS_Float cperp = cgeom->hownear(-1,x);
            EGS_Float crange = q == -1 ?
                erange.interpolateFast(logE) :
                prange.interpolateFast(logE);
            if( crange < cperp ) {
                if( type == RangeDiscard ) return retval;
                do_RR = true;
            }
        }
    }
    if( !do_RR ) return NoAction;
    if( rndm->getUniform()*probi < 1 ) {
        // particle survives.
        wt *= probi; return Survive;
    }
    return Kill; // i.e. particle is killed and must be discarded immediately.
}
