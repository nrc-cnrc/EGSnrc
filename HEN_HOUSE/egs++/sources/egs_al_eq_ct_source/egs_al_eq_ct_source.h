/*
###############################################################################
#
#  EGSnrc egs++ egs_al_eq_ct_source headers
#
###############################################################################
#
#  Author:          Marie-Luise Kuhlmann, 2021
#
#
###############################################################################
*/


/*! \file egs_al_eq_ct_source.h
 *  \brief A aluminium equivalent ct source.
 */

#ifndef EGS_AL_EQ_CT_SOURCE_
#define EGS_AL_EQ_CT_SOURCE_

#include "egs_vector.h"
#include "egs_base_source.h"
#include "egs_rndm.h"
#include "egs_shapes.h"
#include "egs_functions.h"
#include "egs_alias_table.h"
#include "egs_math.h"
#include <map>
#include "locate_in_array.h"
// Interpolators
#include "egs_interpolator.h"


#ifdef WIN32

    #ifdef BUILD_COLLIMATED_SOURCE_DLL
        #define EGS_AL_EQ_CT_SOURCE_EXPORT __declspec(dllexport)
    #else
        #define EGS_AL_EQ_CT_SOURCE_EXPORT __declspec(dllimport)
    #endif
    #define EGS_AL_EQ_CT_SOURCE_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_AL_EQ_CT_SOURCE_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_AL_EQ_CT_SOURCE_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_AL_EQ_CT_SOURCE_EXPORT
        #define EGS_AL_EQ_CT_SOURCE_LOCAL
    #endif

#endif

/*! \brief A aluminium equivalent ct source

\ingroup Sources

A aluminium equivalent ct source is a source that models the photon fluence of a CT scanner. The source emits photons in a collimated fan shape of a given width to model the photon fluence output of an arbitrary CT scanner defined by given input data. The source class also calculates the attenuation and filtration characteristics of a bowtie filter modeled from aluminium.

For a detailed description see the following paper:
Marie-Luise Kuhlmann and Stefan Pojtinger 2024 Phys. Med. Biol. 69 095021
DOI 10.1088/1361-6560/ad3886

This source is defined as follows:
\verbatim
:start source:
    library = egs_al_eq_ct_source
    name = some_name

    distance = the distance between the x-ray source and the middle of the CT gantry in cm
    collimation = the collimation of the CT fan beam in the center of the gantry along the patient axis in cm
    bowtie distribution = filename of a file containing the bowtie angular distribution
    al equivalent bowtie = filename of a file containing the bowtie shape in terms of aluminum equivalent thickness

    :start spectrum:
        definition of the spectrum
    :stop spectrum:
:stop source:
\endverbatim

The <tt>bowtie distribution</tt> file is a list of angles in radians and normalized probability (similar to a spectrum file). The first line is a title, the second contains the number of points, minimum value, and mode (set to 0 for histogram counts/bin, or 1 for counts/radian). For example:

\verbatim
100kV_body GE Optima CT 660
177, 0.0, 0
0.0000000000000    0.367292274143348
0.0010908308125    0.367292274143348
...
\endverbatim

The <tt>al equivalent bowtie</tt> file describes the aluminum equivalent thickness. The file has a header, stating the material, the density used to calculate the Al-equivalent thickness, the angular increment, and the number of values. Each line of the thickness array contains the thickness in mm at the specific fan angle in degrees, starting in the middle of the bowtiefilter, going to the rim and assuming a symetric filter. For the following example, at 0° and at 0.125°, the bowtiefilter has a thickness of 10.88 mm:

\verbatim
Filter
#Name of the material
Al
# Density of the material (g/cm^3)
2.7
# Angular increment (°)
0.125
# Num values
177
# Thickness array (mm)
10.88
10.88
...
\endverbatim

A simple example follows. Find the referenced data files in HEN_HOUSE/sources/egs_al_eq_ct_source/example/.

\verbatim
:start source:
    library = egs_al_eq_ct_source
    name = my_al_equivalent_ct_source

    distance = 54.1
    collimation = 4
    bowtie distribution = 201110_attenuationData_100kV_body.dat # Bowtie distribution
    al equivalent bowtie = 201110_equivBowtie_100kV_body.dat # Al equivalent Bowtie

    :start spectrum:
        type = tabulated spectrum
        spectrum file = 201110_HVLDyn_spec_100kV_body.spectrum # base spectrum
        charge = 0 # photons
    :stop spectrum:
:stop source:
\endverbatim

*/

class EGS_AL_EQ_CT_SOURCE_EXPORT EGS_AlEqCtSource :
    public EGS_BaseSimpleSource {

    bool valid;         //!< Is the object a valid source?

public:

    /*! Constructor

    Construct a aluminium equivalent ct source with charge \a Q, base spectrum \a Spec,
    source to center distance \a d and collimation \a c.
    */
    EGS_AlEqCtSource(int Q, EGS_BaseSpectrum *Spec, const EGS_Vector &Xo,
                     const string &Name="", EGS_ObjectFactory *f=0) :
        EGS_BaseSimpleSource(Q,Spec,Name,f), ctry(0), dist(1), collimation(1), valid(true), table(0) {
        setUp();
    };

    /*! Constructor

    Construct a aluminium equivalent ct source from the information pointed to by \a inp.
    */
    EGS_AlEqCtSource(EGS_Input *, EGS_ObjectFactory *f=0);
    ~EGS_AlEqCtSource() {};

    EGS_I64 getNextParticle(EGS_RandomGenerator *rndm,
                            int &Q, int &latch, EGS_Float &E, EGS_Float &wt,
                            EGS_Vector &x, EGS_Vector &u) {

        getPositionDirection(rndm,x,u,wt);
        Q = 0;

        EGS_Float x1 = 0;
        EGS_Float x2 = 0.4286965093125;
        EGS_Float tol = 0.00001;
        EGS_Float rand_max = 0.9886;
        EGS_Float rand_value = (2*rndm->getUniform()-1) * rand_max;
        EGS_Float beta;
        beta = table->sample(rndm);
        if (rand_value < 0) {
            beta = - beta;
        }
        //std::cout << beta << std::endl;
        EGS_Float cosb = cos(beta);
        EGS_Float sinb = sqrt(1-cosb*cosb);
        u.z = 1;
        if (beta < 0) {
            u.x = -sinb/cosb;
        }
        else {
            u.x = sinb/cosb;
        }

        EGS_Float alpha = (2*rndm->getUniform()-1) * atan((collimation/2)/std::abs(dist));
        EGS_Float cosa = cos(alpha);
        EGS_Float sina = sqrt(1-cosa*cosa);
        if (alpha < 0) {
            u.y = -sina/cosa;
        }
        else {
            u.y = sina/cosa;
        }
        EGS_Float len = sqrt(u.x*u.x + u.y*u.y + u.z*u.z);
        u.x = u.x / len;
        u.y = u.y / len;
        u.z = u.z / len;
        EGS_AliasTable *current_spec;
        EGS_Float beta_near = table->getX()[locate(table->getX(), table->get_length(), abs(beta))];
        current_spec = spectra_dict[std::abs(beta_near)];
        //std::cout << current_spec << std::endl;
        E = current_spec->sample(rndm);
        setLatch(latch);
        wt = 1;
        return ++count;
    };

    void getPositionDirection(EGS_RandomGenerator *rndm,
                              EGS_Vector &x, EGS_Vector &u, EGS_Float &wt) {

        x = EGS_Vector(0.0, 0.0, dist*(-1));
        int ntry = 0;
        ctry += ntry;
    };


    EGS_Float getFluence() const {
        return count;
    };

    bool storeFluenceState(ostream &data) const {
        return egsStoreI64(data,ctry);
    };

    bool setFluenceState(istream &data) {
        return egsGetI64(data,ctry);
    };

    bool addFluenceData(istream &data) {
        EGS_I64 tmp;
        bool ok = egsGetI64(data,tmp);
        if (!ok) {
            return false;
        }
        ctry += tmp;
        return true;
    };

    bool isValid() const {
        return (valid && s != 0);
    };

    void resetFluenceCounter() {
        ctry = 0;
    };

protected:

    EGS_Float     collimation;    //!<
    EGS_I64       ctry;           //!< number of attempts to sample a particle
    EGS_Float     dist;           //!< source-target shape min. distance

    std::map<EGS_Float, EGS_AliasTable *> spectra_dict;
    EGS_AliasTable *table;
    EGS_Interpolator *E_Muen_Rho;

    const EGS_Float AL_DENSITY = 2.702; //g/cm^3

    void setUp();

    EGS_AliasTable *read_distribution_file(string distribution_file, bool print, bool smooth);
    EGS_AliasTable *calculate_filtered_spectrum(EGS_AliasTable *basepec, EGS_Float al_thickness, int counter, EGS_Interpolator *mu_table);
    EGS_Interpolator *read_mu_table(string muen_data);
    std::map<EGS_Float, EGS_AliasTable *> get_spectrum_dict(EGS_AliasTable *basepec, EGS_Float *thickness_list, EGS_AliasTable *distribution, EGS_Interpolator *mu_table);
    EGS_Float *read_filterthickness_from_file(string filter_thickness_file);
    EGS_AliasTable *read_spectrum(string spec_file, int err);



#endif

};
