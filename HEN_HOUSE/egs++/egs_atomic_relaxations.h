/*
###############################################################################
#
#  EGSnrc egs++ atomic relaxation headers
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
#  Contributors:    Reid Townson
#
###############################################################################
*/


/*! \file     egs_atomic_relaxations.h
 *  \brief    EGS_AtomicRelaxations class header file
 *  \IK
 ***************************************************************************/

#ifndef EGS_ATOMIC_RELAXATIONS_

#define EGS_ATOMIC_RELAXATIONS_

#include "egs_simple_container.h"

struct EGS_RelaxationParticle {
    int        q;  //!< charge (0-photon, -1=electron)
    EGS_Float  E;  //!< energy in MeV
    EGS_RelaxationParticle(int Q, EGS_Float e) : q(Q), E(e) {};
    EGS_RelaxationParticle() {};
};

class EGS_RelaxImplementation;
class EGS_RandomGenerator;

/*! A class for handling atomic relaxations

 Note: this class is used internally at the NRC for some applications, but is
 not yet used in the distributed user codes or in EGSnrc itself. The difference
 to the EGSnrc built-in mortran implementation of atomic relaxations is that
 all shells and relaxations according to data from the EADL library are taken
 into account.
 */
class EGS_EXPORT EGS_AtomicRelaxations {

public:

    /*! Constructor

        If \a data_path is not null, the data file (relax.data) will be
        searched for in this path. Otherwise the data file will be
        assumed to be in the \$HEN_HOUSE/data folder
     */
    EGS_AtomicRelaxations(const char *data_path = 0);

    ~EGS_AtomicRelaxations();

    /*! Loads shell transitions for element Z

        Returns 0 on success and a non-zero error code on failure.
     */
    int loadData(int Z);

    /*! Loads shell transitions for all elements

        Returns 0 on success and a non-zero error code on failure.
     */
    int loadAllData();

    /*! Loads shell transitions for the \a nz elements in the array \a Zarray

    Returns 0 on success and a non-zero error code on failure.
     */
    int loadData(int nz, const int *Zarray);

    /*! Perform the relaxation cascade for shell \a sh from element \a Z.

        The resulting relaxation particles are stored in the container
        \a particles. Relaxation photons/electrons with energies below
        \a pcut / \a ecut will not be produced and their energy will be
        added to \a edep. The binding enerty of the outermost shell
        and shells for which there is no data will also be added to
        \a edep.
     */
    void relax(int Z, int sh, EGS_Float ecut, EGS_Float pcut,
               EGS_RandomGenerator *rndm, double &edep,
               EGS_SimpleContainer<EGS_RelaxationParticle> &particles);

    /*! Returns the binding energy of shell \a shell in element \a Z */
    EGS_Float getBindingEnergy(int Z, int shell);

    /*! Returns the number of shells in element \a Z */
    int      getNShell(int Z);

    /*! Set the binding energy of shell \a shell in element \a Z to
        \a new_be. This method can be used to overwrite the default
        data loaded with the relaxation probabilities when using
        a photon cross section tabulation other than EPFL97
     */
    void      setBindingEnergy(int Z, int shell, EGS_Float new_be);

    /*! Returns the maximum energy of characteristic X-rays created when
        filling a vacancy in shell \a shell of element \a Z.
     */
    EGS_Float getMaxGammaEnergy(int Z, int shell);

    /*! Returns the maximum energy of Auger electrons created when
        filling a vacancy in shell \a shell of element \a Z.
     */
    EGS_Float getMaxElectronEnergy(int Z, int shell);

protected:

    EGS_RelaxImplementation *p;

};

#endif
