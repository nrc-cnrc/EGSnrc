/*
###############################################################################
#
#  EGSnrc egs++ egs_fac range rejection headers
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


#ifndef EGS_RANGE_REJECTION_
#define EGS_RANGE_REJECTION_

#include "egs_interpolator.h"
#include "egs_base_geometry.h"

class EGS_Vector;
//class EGS_BaseGeometry;
class EGS_RandomGenerator;
class EGS_Stack;
class EGS_Input;

class EGS_RangeRejection {

public:

    /*! Rejection types */
    enum RejectionType {
        None = 0,           //<! i.e., no rejection/RR
        RangeDiscard = 1,   //<! simple range discard
        RussianRoulette = 2 //<! RR and/or range discard
    };

    /*! The possible outcomes of a call to rangeDiscard(); */
    enum RejectionAction {
        NoAction = 0,    //!< do nothing
        Kill     = -1,   //!< kill particle
        Survive  = 2,    //!< survived RR
        DiscardE = 1,    //!< discard electron
        DiscardP = 99,   //!< discard positron
    };

    EGS_RangeRejection() : cgeom(0), probi(1), Esave(0), type(None) {};

    /*! \brief Perform range discard or Russian Roulette on e+/e-
    */
    RejectionAction rangeDiscard(int q,
            const EGS_Vector &x, EGS_Float E, EGS_Float logE,
            EGS_Float tperp, EGS_Float range, bool is_cav,
            EGS_RandomGenerator *rndm, EGS_Float &wt) const;

    RejectionAction rangeDiscard(int np, EGS_Stack *stack,
            EGS_Float tperp, EGS_Float range, bool is_cav,
            EGS_Float logE, EGS_RandomGenerator *rndm) const;

    EGS_Float getRange(int q, EGS_Float elke) {
        return q == -1 ? erange.interpolateFast(elke) :
                         prange.interpolateFast(elke);
    };

    bool canEnterCavity(int q, EGS_Float elke, const EGS_Vector &x) {
        EGS_Float range = getRange(q,elke);
        return canEnterCavity(range,x);
    };

    bool canEnterCavity(EGS_Float range, const EGS_Vector &x) {
        return range >= cgeom->hownear(-1,x);
    };

    EGS_Float hownear(const EGS_Vector &x) { return cgeom->hownear(-1,x); };

    static EGS_RangeRejection* getRangeRejection(EGS_Input *inp,
            EGS_Interpolator *i_ededx, EGS_Interpolator *i_pdedx);

    RejectionType getType() const { return type; };
    EGS_Float     getProbi() const { return probi; };
    EGS_Float     getEsave() const { return Esave; };
    EGS_BaseGeometry *getCavityGeometry() { return cgeom; };

protected:

    EGS_BaseGeometry *cgeom; //<! Cavity geometry
    EGS_Float         probi; //<! Inverse survival probability
    EGS_Float         Esave; //<! "Safe" energy
    RejectionType     type;  //<! Rejection type
    EGS_Interpolator  erange;//<! electron range interpolator
    EGS_Interpolator  prange;//<! positron range interpolator

};

#endif
