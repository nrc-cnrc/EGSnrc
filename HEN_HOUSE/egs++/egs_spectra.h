/*
###############################################################################
#
#  EGSnrc egs++ spectra headers
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
#  Contributors:    Frederic Tessier
#                   Reid Townson
#                   Ernesto Mainegra-Hing
#                   Marie-Luise Kuhlmann
#
###############################################################################
*/


/*! \file egs_spectra.h
 *  \brief Declarations of concrete spectra that other libraries need to use
 *  directly
 */

#ifndef EGS_SPECTRA_
#define EGS_SPECTRA_

#include "egs_base_source.h"
#include "egs_alias_table.h"
#include "egs_rndm.h"

/*! \brief A tabulated spectrum.
 *
 * \ingroup egspp_main
 *
 * A tabulated spectrum can be used to simulate any type of energy
 * distribution. Three different types of a tabulated spectrum are
 * available:
 *  - A line spectrum, \em i.e. the spectrum consists of a series
 *    of discrete energies \f$E_i\f$ with probabilities \f$p_i\f$.
 *    This type of spectrum is useful for instance for the simulation of
 *    some radioactive sources.
 *  - A histogram spectrum. In this case the spectrum consists of a series
 *    of energy bins \f$E_i ... E_{i+1}\f$ with probabilities \f$p_i\f$, where
 *    the distribution within a bin is uniform. This is the type of spectrum
 *    implemented by \c ensrc.mortran and used in the standard set of
 *    EGSnrc mortran user codes.
 *  - An 'interpolated' spectrum. This is similar to a histogram spectrum,
 *    however, the probabilities are considered to be at the bin edges and
 *    a linear interpolation is used between the bin edges. As a result,
 *    an interpolated spectrum with \f$N\f$ bins requires \f$N+1\f$ energies and
 *    \f$N+1\f$ probabilities, whereas a histogram spectrum with \f$N\f$ bins
 *    requires \f$N+1\f$ energies but \f$N\f$ probabilities.
 *
 * Internally the tabulated spectrum object uses
 * an \link EGS_AliasTable alias table object\endlink to
 * sample energies efficiently.

There are two ways for defining a tabulated spectrum by specifying the
name of a file containing a spectrum definition, or inline. In the former
case the keys needed to define a spectrum are
\verbatim
:start spectrum:
    type = tabulated spectrum
    spectrum file = the name of a spectrum file
:stop spectrum:
\endverbatim
The syntax of the spectrum file is the same as for the standard set
of EGSnrc user codes (see the example spectra distributed with the
system in $HEN_HOUSE/spectra), except that now the mode of the file can also
be 2 (line spectrum) or 3 (interpolated spectrum).
A spectrum is defined inline as follows:
\verbatim
:start spectrum:
    type = tabulated spectrum
    energies = list of discrete energies or bin edges
    probabilities = list of probabilities
    spectrum mode = 0 or 1 or 2 or 3
:stop spectrum:
\endverbatim
where the meaning of the spectrum mode is the same as the mode of a
spectrum file.
 */
class EGS_EXPORT EGS_TabulatedSpectrum : public EGS_BaseSpectrum {

public:

    /*! \brief Construct a tabulated spectrum from the energies \a x and
     *  the probabilities \a f.
     *
     * The interpretation of \a x and \a f depends on the spectrum type
     * specified by \a Type. For \a Type=0,1 it is considered to be a
     * histogram spectrum with N-1 bins defined by the N edge energies \a x.
     * (the difference between a \a Type=0 and a \a Type=1 spectrum is that
     * in the one case the probabilities \a f are considered to be counts
     * per bin whereas in the latter case they are counts per MeV). For
     * \a Type=2 the spectrum is considered to be a line spectrum with
     * \a N discrete energies and probabilities given by \a x and \a f.
     * For \a Type=3 the spectrum is considered to be an interpolated
     * spectrum.
     */
    EGS_TabulatedSpectrum(int N, const EGS_Float *x, const EGS_Float *f,
                          int Type = 1, const char *fname = 0) : EGS_BaseSpectrum(),
        table(new EGS_AliasTable(N,x,f,Type)) {
        setType(Type,fname);
    };

    ~EGS_TabulatedSpectrum() {
        delete table;
    };
    EGS_Float maxEnergy() const {
        return table->getMaximum();
    };
    EGS_Float expectedAverage() const {
        return table->getAverage();
    };

    EGS_Float * getF() {
        return table->getF();
    };

    EGS_Float * getX() {
        return table->getX();
    };

    int get_length() {
        return table->get_length();
    };

protected:

    EGS_AliasTable *table; //!< The alias table object used to sample energies.
    void setType(int Type,const char *fname) {
        if (Type == 0) {
            type = "tabulated line spectrum";
        }
        else if (Type == 1) {
            type = "tabulated histogram spectrum";
        }
        else {
            type = "tabulated spectrum";
        }
        if (fname) {
            type += " defined in ";
            type += fname;
        }
        else {
            type += " defined inline";
        }
    };

    EGS_Float sample(EGS_RandomGenerator *rndm) {
        return table->sample(rndm);
    };

};

#endif
