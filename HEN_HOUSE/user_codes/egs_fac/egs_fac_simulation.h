/*
###############################################################################
#
#  EGSnrc egs++ egs_fac simulation headers
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
#  Contributors:    Ernesto Mainegra-Hing
#
###############################################################################
*/


#ifndef EGS_FAC_SIMULATION_
#define EGS_FAC_SIMULATION_

#include "egs_libconfig.h"
#include "egs_functions.h"
#include "egs_math.h"

#include <iostream>
using namespace std;

class EGS_BaseGeometry;
class EGS_RangeRejection;
class EGS_AffineTransform;
class EGS_Input;

#define N_FAC_DOSE 8
//#define N_FAC_CORR 11
#define N_FAC_CORR 6

/*! Defines a single FAC simulation  */
struct EGS_FACSimulation {

    /*! The simulation geometry */
    EGS_BaseGeometry     *geometry;

    /*! Range Rejection object */
    EGS_RangeRejection   *rr;

    /*! A transformation to be applied before transporting
        in this geoimetry */
    EGS_AffineTransform  *transform;

    /*! An array that identifies cavity, aperture and
        front/back regions */
    unsigned char        *properties;

    /*! The various doses needed to compute FAC correction factors:
        0:   The total dose deposited in the cavity (Eg)
        1:   The total dose excluding aperture dose (E1)
        2:   Primary dose (E2)
        3:   Primary dose with energy imbalance from the side faces
             removed (E3 = E2 + Eside)
        4:   Primary dose with energy imbalance from all faces
             removed (E2 + Eside + Efb)
        5:   Cavity kerma from primary photons (E4)
        6:   Cavity kerma from primary photons for ideal point source (E5)
        7:   POM kerma from primary photons (E6)
    */
    double                dose[N_FAC_DOSE];
    double                dose2[N_FAC_DOSE];
    double                dosec[N_FAC_DOSE];
    double                dtmp[N_FAC_DOSE];
    double                extra[N_FAC_CORR];

    /*! The cavity mass */
    EGS_Float            cmass;

    /*! Point of measurement */
    EGS_Float             z_pom, R_pom, R2_pom;

    /*! Distance from POM to CV front face */
    EGS_Float             L;

    /*! CV height */
    EGS_Float             h;

    /*! Photon splitting */
    EGS_Float             fsplit, fspliti;

    /*! CV medium */
    int                   med_cv;

    /*! Was there an energy deposition event ?*/
    bool                  had_edep;

    /*! Account for scatter ? Useful for Ab calculation*/
    bool                  include_scatter;


    EGS_FACSimulation(EGS_BaseGeometry *g, EGS_AffineTransform *t);

    ~EGS_FACSimulation();

    inline bool isCavity(int ireg) const { return (properties[ireg] & 1); };
    inline bool isAperture(int ireg) const { return (properties[ireg] & 2); };
    inline bool isFrontBack(int ireg) const { return (properties[ireg] & 4); };
    inline bool isSplitting(int ireg) const { return (properties[ireg] & 8); };
    void setCavity(int ireg) { properties[ireg] |= 1; };
    void setAperture(int ireg) { properties[ireg] |= 2; };
    void setFrontBack(int ireg) { properties[ireg] |= 4; };
    void setSplittingOn(int ireg) { properties[ireg] |= 8; };
    void setSplittingOff(int ireg) {
        if( isSplitting(ireg) ) properties[ireg] ^= 8;
    };
    void describeSimulation();
//     void describeSimulation(){
//          egsInformation("%s ",geometry->getName().c_str());
//          if (include_scatter) egsInformation("=> Include all scatter\n");
//          else egsInformation("=> Exclude scatter past POM\n");
//     };

    int  setPOM(EGS_Float z, EGS_Float r);

    inline void addEnergyDeposition(int ireg, int latch, EGS_Float edep) {
        if( isnan(edep) ) {
            egsInformation("\nattempt to add a NaN in addEnergySeposition\n");
            return;
        }
        if( !isCavity(ireg) || !edep ) return;
        had_edep = true;
        dtmp[0] += edep;
        if( latch >= 0 ) {
            dtmp[1] += edep;
            if( !latch ) dtmp[2] += edep;
        }
    };

    inline void addKerma(EGS_Float E4, EGS_Float E5, EGS_Float E6) {
        had_edep = true;
        dtmp[5] += E4; dtmp[6] += E5; dtmp[7] += E6;
    };

    /*! Computes the energy imbalance between electrons entering and leaving the CV.

      Energies of primary electrons entering the CV are deducted,
      energies of primary electrons leaving the CV are added.
      To reduce the uncertainty on the imbalance through the front/back faces,
      we deduct the unattenuated energy leaving through front/back and add
      the unattenuated energy entering through front/back faces. Because
      after unattenuation there is CPE along the beam direction, this operation
      corresponds to adding (or deducting) 0.
    */
    inline void addEnergyImbalance(int ireg, int inew, EGS_Float expmfp, EGS_Float edep) {
        if( isnan(edep) || isnan(expmfp) ) {
            egsInformation("\nattempt to add a NaN in addEnergyImbalance\n");
            return;
        }
        bool is_cv_old = isCavity(ireg), is_cv_new = isCavity(inew);
        if( ( !is_cv_old && !is_cv_new ) ||
            (  is_cv_old &&  is_cv_new ) ) return;
        had_edep = true;
        if( is_cv_old ) {
            // electron leaving the cavity => add its energy
            if( isFrontBack(inew) ) // leaving through front/back face
                //dtmp[4] += edep;
                dtmp[4] -= edep*(expmfp-1);
            else                    // leaving through side faces
                dtmp[3] += edep;
        }
        else {
            // electron entering the cavity => deduct its energy
            if( isFrontBack(ireg) ) // entering through front/back face
                //dtmp[4] -= edep;
                dtmp[4] += edep*(expmfp-1);
            else
                dtmp[3] -= edep;    // entering through side faces
        }
    };

    inline bool finishHistory() {
        bool ok = true;
        if( had_edep ) {
            dtmp[3] += dtmp[2];
            dtmp[4] += dtmp[3];
            int j;
            for(j=0; j<N_FAC_DOSE-1; ++j) dosec[j] += dtmp[j]*dtmp[j+1];
            dosec[N_FAC_DOSE-1] += dtmp[4]*dtmp[0];
            /*
            double aux1 = dtmp[0]*dtmp[5], aux2 = dtmp[4]*dtmp[7];
            extra[0] += aux1; extra[1] += aux1*aux1;
            extra[2] += aux2; extra[3] += aux2*aux2;
            extra[4] += aux1*aux2;
            */
            extra[0] += dtmp[0]*dtmp[4];
            extra[1] += dtmp[0]*dtmp[5];
            extra[2] += dtmp[0]*dtmp[7];
            extra[3] += dtmp[4]*dtmp[5];
            extra[4] += dtmp[4]*dtmp[7];
            extra[5] += dtmp[5]*dtmp[7];
            for(j=0; j<N_FAC_DOSE; ++j) {
                dose[j] += dtmp[j]; dose2[j] += dtmp[j]*dtmp[j]; dtmp[j] = 0;
            }
            had_edep = false;
        }
        return ok;
    };

    bool outputData(ostream &data);
    bool readData(istream &data);
    bool addData(istream &data);
    void resetCounter();

    void reportResults(double flu, EGS_I64 ncase);

    void getRatio(double r1, double dr1, double r2, double dr2,
                  double dcor, EGS_I64 ncase, double &A, double &dA);

    void getAtotal(EGS_I64 ncase, double &A, double &dA);

    static EGS_FACSimulation *getFACSimulation(EGS_Input *input);
    const static char* Dnames[];
    const static char* Anames[];
};

/* In addition to the ratios of the various Ei's, we also want to compute
   the ratios of Eg', where Eg' is Eg but computed using the scored kerma,
   i.e., Eg' = dose[0]/dose[4]*dose[5]. So, the ratio of Eg' in geometries
   is g1 and g2 is
     dose[0,g1]*dose[5,g1]*dose[4,g2]/(dose[0,g2]*dose[5,g2]*dose[4,g1])
   To compute the uncertainty on this quantity one needs the covariance
   matrix, which is scored in extra[N_RATIO_COV] (there are 6*5/2 pairs).
*/

#define N_RATIO_COV 15

class EGS_FACCorrelation {

public:

    EGS_FACCorrelation(EGS_FACSimulation *sim1, EGS_FACSimulation *sim2);

    ~EGS_FACCorrelation();

    inline void finishHistory() {
        corr[0] += s1->dtmp[0]*s2->dtmp[0];
        corr[1] += s1->dtmp[1]*s2->dtmp[1];
        corr[2] += s1->dtmp[2]*s2->dtmp[2];
        corr[5] += s1->dtmp[5]*s2->dtmp[5];
        corr[6] += s1->dtmp[6]*s2->dtmp[6];
        corr[7] += s1->dtmp[7]*s2->dtmp[7];
        double aux1, aux2;
        aux1 = s1->dtmp[2] + s1->dtmp[3];
        aux2 = s2->dtmp[2] + s2->dtmp[3];
        corr[3] += aux1*aux2;
        aux1 += s1->dtmp[4]; aux2 += s2->dtmp[4];
        corr[4] += aux1*aux2;
        double var[6];
        var[0] = s1->dtmp[0];
        var[1] = aux2;
        var[2] = s1->dtmp[5];
        var[3] = s2->dtmp[0];
        var[4] = aux1;
        var[5] = s2->dtmp[5];
        int ij=0;
        for(int i=0; i<5; ++i) for(int j=i+1; j<6; ++j)
            extra[ij++] += var[i]*var[j];
    };

    void reportResults(double flu, EGS_I64 ncase);

    bool outputData(ostream &data);
    bool readData(istream &data);
    bool addData(istream &data);
    void resetCounter();

    double             corr[N_FAC_DOSE];
    double             extra[N_RATIO_COV];
    EGS_FACSimulation *s1;
    EGS_FACSimulation *s2;

    const static char* Dnames[];

};

/*! Computes the Ax correction factor
 */

#define N_AX_COV 56

class EGS_AxCalculator {

public:

    /*! sim0 is the simulation geometry for the computed Aatt
        sim1 and sim2 are the two geometries used to determine
        Aatt experimentally with a vacuum tube technique.
     */
    EGS_AxCalculator(EGS_FACSimulation *sim0,
                     EGS_FACSimulation *sim1, EGS_FACSimulation *sim2);

    ~EGS_AxCalculator();

    inline void finishHistory() {
        v[0] = s0->dtmp[6];
        v[1] = s2->dtmp[0];
        v[2] = s2->dtmp[5];
        v[3] = s1->dtmp[2] + s1->dtmp[3] + s1->dtmp[4];
        v[4] = s0->dtmp[7];
        v[5] = s1->dtmp[0];
        v[6] = s1->dtmp[5];
        v[7] = s2->dtmp[2] + s2->dtmp[3] + s2->dtmp[4];
        int ij=0;
        for(int i=0; i<7; ++i) for(int j=i+1; j<8; ++j)
            cov_matrix[ij++] += v[i]*v[j];
    };

    bool outputData(ostream &data);
    bool readData(istream &data);
    bool addData(istream &data);
    void resetCounter();
    void reportResults(EGS_I64 ncase);

    EGS_FACSimulation *s0;
    EGS_FACSimulation *s1;
    EGS_FACSimulation *s2;

    double  cov_matrix[N_AX_COV];
    double  v[8];

};

#endif
