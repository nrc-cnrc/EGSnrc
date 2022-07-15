/*
###############################################################################
#
#  EGSnrc egs++ egs_chamber application
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
#  Authors:         Iwan Kawrakow, 2007
#                   Joerg Wulff, 2007
#
#  Contributors:    Ernesto Mainegra-Hing
#                   Hugo Bouchard
#                   Frederic Tessier
#                   Reid Townson
#                   Blake Walters
#
###############################################################################
#
#  This code was originally adapter from the egs++ application cavity by
#  Joerg Wulff in 2007.
#
###############################################################################
#
#  Efficient in-phantom ion chamber calculations.
#
#  Hugo Bouchard, 2009: Added the possibility to calculate positioning-induced
#  dose uncertainty. Also re-factored the code to have most function
#  implementations outside of the class declaration.
#
#  Iwan Kawrakow, 2007: Committing Joerg Wulff's modifications to cavity as a
#  separate user code. Joerg threw out everything related to FAC and HVL and
#  the code is now usable for ion chamber correction factors only. So, I
#  decided to rename it to egs_chamber and have it as a separate user code.
#  He had a hack to store the state of the random number generator (with
#  modifications of the EGS_RandomGenerator class). For now I commented this
#  out but clearly it will be useful to add the functionality of random number
#  generators storing and restoring their state.
#
#  Joerg Wulff, 2007: modifications made for eff-improvements for in-phantom
#  calculations: a) photon cross section enhancement in defined regions;
#  b) temporary phase-space scoring and correlated sampling.
#
###############################################################################
*/


#include <cstdlib>
// We derive from EGS_AdvancedApplication => need the header file.
#include "egs_advanced_application.h"
// We use scoring objects provided by egspp => need the header file.
#include "egs_scoring.h"
// Every C++ EGSnrc application needs this header file
#include "egs_interface2.h"
// We use egsInformation() => need the egs_functions.h header file.
#include "egs_functions.h"
// We use the EGS_Input class
#include "egs_input.h"
// To get the maximum source energy
#include "egs_base_source.h"
// The random number generator
#include "egs_rndm.h"
// Transformations
#include "egs_transformations.h"
// Interpolators
#include "egs_interpolator.h"
#include "egs_run_control.h"

#include "egs_rndm.h"
#define getRNGPointers F77_OBJ_(egs_get_rng_pointers,EGS_GET_RNG_POINTERS)
extern __extc__ void getRNGPointers(EGS_I32 *, EGS_I32 *);
#define getRNGArray F77_OBJ_(egs_get_rng_array,EGS_GET_RNG_ARRAY)
extern __extc__ void getRNGArray(EGS_Float *);
#define setRNGState F77_OBJ_(egs_set_rng_state,EGS_SET_RNG_STATE)
extern __extc__ void setRNGState(const EGS_I32 *, const EGS_Float *);

#define doRayleigh F77_OBJ_(do_rayleigh,DO_RAYLEIGH)
extern __extc__ void doRayleigh();
extern __extc__ void F77_OBJ(pair,PAIR)();
extern __extc__ void F77_OBJ(compt,COMPT)();
extern __extc__ void F77_OBJ(photo,PHOTO)();

// copying particles
#define COPY_PARTICLE(np,np1) \
                the_stack->x[np1] = the_stack->x[np]; \
                the_stack->y[np1] = the_stack->y[np]; \
                the_stack->z[np1] = the_stack->z[np]; \
                the_stack->u[np1] = the_stack->u[np]; \
                the_stack->v[np1] = the_stack->v[np]; \
                the_stack->w[np1] = the_stack->w[np]; \
                the_stack->iq[np1] = the_stack->iq[np]; \
                the_stack->E[np1] = the_stack->E[np]; \
                the_stack->wt[np1] = the_stack->wt[np]; \
                the_stack->latch[np1] = the_stack->latch[np]; \
                the_stack->ir[np1] = the_stack->ir[np]; \
                the_stack->dnear[np1] = the_stack->dnear[np]; \
                the_extra_stack->nbr_splitting[np1] = the_extra_stack->nbr_splitting[np]; \

// Extra stack to hold variance reduction data per particle
struct EGS_ExtraStack {
    int nbr_splitting[MXSTACK];
};

extern __extc__ struct EGS_ExtraStack F77_OBJ_(extra_stack,EXTRA_STACK);
static struct EGS_ExtraStack *the_extra_stack =
                 &F77_OBJ_(extra_stack,EXTRA_STACK);

/*! a class for storing the phase-space temporarilly */

#define MAXPHSP 100000
class TmpPhsp {
public:
    // constructor
    TmpPhsp() : np(0), ntot(4), p(new EGS_Particle[4]), nbr_split(new int[4]) {};
    ~TmpPhsp() { delete [] p; delete [] nbr_split; };
    void grow() {
        int ntot_new = 2*ntot;
        if( ntot_new > MAXPHSP ) egsFatal("TmpPhsp::grow(): exceeded maximum size %d\n",MAXPHSP);
        EGS_Particle *pnew = new EGS_Particle [ntot_new];
        for(int j=0; j<np; j++) pnew[j] = p[j];
        delete [] p;
        p = pnew; ntot = ntot_new;

        int *nbr_split_new = new int [ntot_new];
        for(int j=0; j<np; j++) nbr_split_new[j] = nbr_split[j];
        delete [] nbr_split;
        nbr_split = nbr_split_new;
    };

    void set() {
        if( np > ntot-1 ) grow();
        int ip = the_stack->np-1;
        p[np].q     = the_stack->iq[ip];
        p[np].latch = the_stack->latch[ip];
        p[np].ir    = the_stack->ir[ip];
        p[np].E     = the_stack->E[ip];
        p[np].wt    = the_stack->wt[ip];
        p[np].x     = EGS_Vector(the_stack->x[ip],the_stack->y[ip],the_stack->z[ip]);
        p[np].u   = EGS_Vector(the_stack->u[ip],the_stack->v[ip],the_stack->w[ip]);

        nbr_split[np++] = the_extra_stack->nbr_splitting[ip];
    };
    void set(const EGS_Particle &particle) {
        if( np > ntot-1 ) grow();
        p[np++] = particle;
    };
    void get(){
        int ip = the_stack->np-1; --np;
        the_stack->x[ip]  = p[np].x.x;
        the_stack->y[ip]  = p[np].x.y;
        the_stack->z[ip]  = p[np].x.z;
        the_stack->u[ip]  = p[np].u.x;
        the_stack->v[ip]  = p[np].u.y;
        the_stack->w[ip]  = p[np].u.z;
        the_stack->E[ip]  = p[np].E;
        the_stack->iq[ip] = p[np].q;
        the_stack->latch[ip] = p[np].latch;
        the_stack->wt[ip] = p[np].wt;

        the_extra_stack->nbr_splitting[ip] = nbr_split[np];
    };

    void clean() { np = 0; };

    void setPointer(int ip) { np = ip; };

    int  size() const { return np; };

private:

    int          ntot, np;
    EGS_Particle *p;
    int *nbr_split;
};

//*HB_start************************

/*a class in which you put infos and generate new positions*/
class APP_EXPORT EGS_PosUncertDistributor {

public:
        /*Constructor*/
        EGS_PosUncertDistributor(): transl(), sigma_transl(), max_transl(), theta(), sigma_theta(), max_theta(), typeTransl(0), typeRot(0) {}

        /*Destructor*/
        ~EGS_PosUncertDistributor() {};

        /*initialization*/
        void initTranslation(EGS_Vector sigmaval, EGS_Vector maxval) {
            transl.x = 0;
            transl.y = 0;
            transl.z = 0;
            sigma_transl = sigmaval;
            max_transl = maxval;
        };
        /*initialization*/
        void initRotation(EGS_Vector sigmaval, EGS_Vector maxval) {
            theta.x = 0;
            theta.y = 0;
            theta.z = 0;
            sigma_theta = sigmaval;
            max_theta = maxval;
        };
        /*initialization*/
        int initTypeTransl(int dist) { typeTransl = dist; };
        int initTypeRot(int dist) { typeRot = dist; };
        /*validation*/
        bool validateInput() {
            bool retval = true;
            /*make sure values are not negative*/
            if((sigma_transl.x<0)||(sigma_transl.y<0)||(sigma_transl.z<0)||(max_transl.x<0)||(max_transl.y<0)||(max_transl.z<0))
                retval = false;
            if((sigma_theta.x<0)||(sigma_theta.y<0)||(sigma_theta.z<0)||(max_theta.x<0)||(max_theta.y<0)||(max_theta.z<0))
                retval = false;
            /*make sure max is at least equal to sigma*/
            if((max_transl.x<sigma_transl.x)||(max_transl.y<sigma_transl.y)||(max_transl.z<sigma_transl.z))
                retval = false;
            if((max_theta.x<sigma_theta.x)  ||(max_theta.y<sigma_theta.y)  ||(max_theta.z<sigma_theta.z))
                retval = false;
            /*make sure if sigma is 0, max is 0 also*/
            if((sigma_transl.x==0&&max_transl.x!=0)||(sigma_transl.y==0&&max_transl.y!=0)||(sigma_transl.z==0&&max_transl.z!=0))
                retval = false;
            if((sigma_theta.x==0&&max_theta.x!=0)||(sigma_theta.y==0&&max_theta.y!=0)||(sigma_theta.z==0&&max_theta.z!=0))
                retval = false;
            /*make sure distribution types are valid*/
            if((typeTransl!=0)&&(typeTransl!=1))
                retval = false;
            if((typeRot!=0)&&(typeRot!=1))
                retval = false;
            /*return value*/
            return retval;
        };
        /*information*/
        int returnTypeTransl() { return typeTransl; };
        int returnTypeRot() { return typeRot; };
        EGS_Vector getTranslation()      { return transl; };
        EGS_Vector getSigmaTranslation() { return sigma_transl; };
        EGS_Vector getMaxTranslation()   { return max_transl; };
        EGS_Vector getRotation()         { return theta; };
        EGS_Vector getSigmaRotation()    { return sigma_theta; };
        EGS_Vector getMaxRotation()      { return max_theta; };
        /*set new shifts*/
        void setNewShifts(EGS_RandomGenerator *rndm){
            transl.x = rndmShift(sigma_transl.x,max_transl.x,typeTransl,rndm);
            transl.y = rndmShift(sigma_transl.y,max_transl.y,typeTransl,rndm);
            transl.z = rndmShift(sigma_transl.z,max_transl.z,typeTransl,rndm);
            theta.x = rndmShift(sigma_theta.x,max_theta.x,typeRot,rndm);
            theta.y = rndmShift(sigma_theta.y,max_theta.y,typeRot,rndm);
            theta.z = rndmShift(sigma_theta.z,max_theta.z,typeRot,rndm);
        };
        /*generate rndm shift*/
        EGS_Float rndmShift(EGS_Float sigmaval, EGS_Float maxval, int type,EGS_RandomGenerator *rndm) {
            EGS_Float val = 2*maxval;
            if(sigmaval*maxval != 0) {
                if(type==0) {
                    while((val > maxval)||(val < -maxval))
                        val = sigmaval*rndm->getGaussian();
                }
                else if(type==1)  val = maxval*(rndm->getUniform()-0.5)*2;
            }
            return val; //positive sign because the fact that we move particle instead of cavity is taken care of
        };

private:

        int  typeTransl, typeRot; //cavity positioning uncertainty typeibution types

        EGS_Vector   transl; //// cavity translation during simulation

        EGS_Vector   sigma_transl;   // cavity positioning uncertainty translation std values

        EGS_Vector   max_transl;   // cavity positioning uncertainty maximum transllation values during rdm gen

        EGS_Vector   theta; //// cavity rotation during simulation

        EGS_Vector   sigma_theta;   // cavity positioning uncertainty rotation std values

        EGS_Vector   max_theta;   // cavity positioning uncertainty maximum rotlation values during rdm gen

};
/* a class for correlated positioning uncertainty estimator*/
class APP_EXPORT EGS_PosUncertEstimator {

public:
        /*Constructor*/
        EGS_PosUncertEstimator() : K(0), N(0), M(0), previous_score_case(0), correlation(0),
                                       McasePerPos(0), NposPerSample(0),sum1(0), sum2(0),
                                       sumdij(0), sumdij2(0), sumDi(0), sumDi2(0), sumvarDi(0),
                                       sumeij(0), sumeij2(0), sumEi(0), sumEi2(0), sumvarEi(0),
                                                  sumdijeij(0), sumDiEi(0),sumcovarDiEi(0){};

        /*Destructor*/
        ~EGS_PosUncertEstimator() {};

        /*Basic functions*/
        EGS_I64 returnM() { return M;};
        EGS_I64 returnN() { return N;};
        EGS_I64 returnK() { return K;};
        EGS_I64 returnMcasePerPos() { return McasePerPos;};
        EGS_I64 returnNposPerSample() { return NposPerSample;};
        EGS_I64 calcIcase(EGS_I64 K, EGS_I64 N, EGS_I64 M) {
                EGS_I64 icase;
                icase = K*NposPerSample*McasePerPos + N*McasePerPos + M;
                return icase;
        };
        void invCalcIcase(EGS_I64 icase, EGS_I64 &k, EGS_I64 &n, EGS_I64 &m) {
            m = (EGS_I64)(icase % McasePerPos);
            n = (EGS_I64)(((double)(icase - m))/McasePerPos);
            n = (EGS_I64)(n % NposPerSample);
            k = (EGS_I64)( ( (double)(((double)(icase - m))/McasePerPos - n) )/NposPerSample);
        };
        /*set McasePerPos*/
        int initMcasePerPos(EGS_I64 m) {
            int retval = -1;
            if(McasePerPos==0) {
                if(m>1)
                    McasePerPos = m;
                else
                    McasePerPos = 2;
                retval = 0;
            }
            return retval;
        };
        /*set NposPerSample*/
        int initNposPerSample(EGS_I64 n) {
            int retval = -1;
            if(NposPerSample ==0) {
                if(n>1)
                    NposPerSample = n;
                else
                    NposPerSample = 2;
                retval = 0;
            }
            return retval;
        };
        void initCorrelation(bool flag) {correlation = flag;};
        /*reset counters after one position*/
        int resetPos() {
            int retval = -1;
            M = 0;
            sumdij = 0;
            sumdij2 = 0;
            if(correlation) {
                sumeij = 0;
                sumeij2 = 0;
                sumdijeij = 0;
            }
            retval = 0;
            return retval;
        };
        /*reset counters after one sample*/
        int resetSample() {
            int retval = -2;
            N = 0;
            sumDi = 0;
            sumDi2 = 0;
            sumvarDi = 0;
            if(correlation) {
                sumEi = 0;
                sumEi2 = 0;
                sumvarEi = 0;
                sumDiEi = 0;
                sumcovarDiEi = 0;
            }
            retval = resetPos();
            return retval;
        };
        /*instruction to move cavity or particle*/
        bool shiftManager(EGS_I64 next_score_case) {
            EGS_I64 nextM = M + next_score_case - previous_score_case;
            bool setNewPos = false;
            if( nextM > McasePerPos ) {
                setNewPos = true;
                EGS_I64 nextK, nextN;
                invCalcIcase(next_score_case, nextK, nextN, nextM);
                if(returnK()==nextK) {
                    int err = finishPos(nextN - returnN());
                    previous_score_case = calcIcase(nextK,nextN,0);
                }
                else {
                    int err = finishPos(NposPerSample - returnN());
                    err = finishSample(nextK - returnK());
                    err = finishPos(nextN);
                    previous_score_case = calcIcase(nextK,nextN,0);
                }
            }
            return setNewPos;
        };
        /*score after one history*/
        int scoreHist(EGS_I64 score_case, EGS_Float dose_val) {
            int retval = -3;
            EGS_I64 case_increment = score_case - previous_score_case;
            previous_score_case = score_case;
            if(case_increment != 0) retval = 0;
            /*scoring at this very position was predicted during last score*/
            M += case_increment;
            scorePos(dose_val);
            EGS_I64 k,n,m;
            invCalcIcase(score_case,k,n,m);
            return retval;
        };
        /*score while still*/
        void scorePos(EGS_Float dose_val) {
            sumdij += dose_val;
            sumdij2 += dose_val*dose_val;
        };
        /*score after one history*/
        int scoreHist(EGS_I64 score_case, EGS_Float dose_val1, EGS_Float dose_val2) {
            int retval = -3;
            EGS_I64 case_increment = score_case - previous_score_case;
            previous_score_case = score_case;
            if(case_increment != 0) retval = 0;
            /*scoring at this very position was predicted during last score*/
            M += case_increment;
            scorePos(dose_val1,dose_val2);
            EGS_I64 k,n,m;
            invCalcIcase(score_case,k,n,m);
            return retval;
        };
        /*score while still*/
        void scorePos(EGS_Float dose_val1, EGS_Float dose_val2) {
            sumdij += dose_val1;
            sumdij2 += dose_val1*dose_val1;
            sumeij += dose_val2;
            sumeij2 += dose_val2*dose_val2;
            sumdijeij += dose_val1*dose_val2;
        };
        /*score after one position*/
        int finishPos(EGS_I64 pos_increment) {
            int retval = -4;
            sumdij = sumdij/McasePerPos; //becomes Di
            sumDi += sumdij;
            sumDi2 += sumdij*sumdij;
            sumvarDi += (sumdij2/McasePerPos - sumdij*sumdij)/(McasePerPos-1);
            if(correlation) {
                sumeij = sumeij/McasePerPos; //becomes Ei
                sumEi += sumeij;
                sumEi2 += sumeij*sumeij;
                sumDiEi += sumdij*sumeij;
                sumvarEi += (sumeij2/McasePerPos - sumeij*sumeij)/(McasePerPos-1);
                sumcovarDiEi += (sumdijeij/McasePerPos - sumdij*sumeij)/(McasePerPos-1);
            }
            N += pos_increment;
            retval = resetPos();
            return retval;
        };
        /*score after one sample*/
        int finishSample(EGS_I64 sample_increment) {
            int retval = -5;
            int flag_good = true;
            EGS_Float pu_var_est = sumDi2/(NposPerSample-1) - sumDi*sumDi/NposPerSample/(NposPerSample-1) - sumvarDi/NposPerSample;
            if(correlation) {
                EGS_Float pu_var2_est = sumEi2/(NposPerSample-1) - sumEi*sumEi/NposPerSample/(NposPerSample-1) - sumvarEi/NposPerSample;
                EGS_Float pu_covar_est = sumDiEi/(NposPerSample-1) - sumDi*sumEi/NposPerSample/(NposPerSample-1) - sumcovarDiEi/NposPerSample;
                EGS_Float tmp = 0;
                if(sumDi!=0 && sumEi!=0) {
                    tmp +=  pu_var_est/(sumDi/NposPerSample)/(sumDi/NposPerSample);
                    tmp +=  pu_var2_est/(sumEi/NposPerSample)/(sumEi/NposPerSample);
                    tmp +=  -2*pu_covar_est/(sumDi/NposPerSample)/(sumEi/NposPerSample);
                }
                else
                    flag_good = false;
                //tmp = tmp*(sumDi/sumEi)*(sumDi/sumEi); //remember that is it OF = D/E;
                //sum1 += pu_covar_est;
                //sum2 += pu_covar_est*pu_covar_est;
                sum1 += tmp;
                sum2 += tmp*tmp;
            }
            else {
                sum1 += pu_var_est;
                sum2 += pu_var_est*pu_var_est;
            }
            if(flag_good)
                K += sample_increment;
            retval = resetSample();
            return retval;
        };
        /*get results and delete last sample*/
        int getResult(EGS_Float &val1, EGS_Float &val2, EGS_Float &val3){
            int retval = -6;
            EGS_Float v1 = 0, v2 = 0;
            if(K>1) {
                v1 = sum1/K;
                v2 = (sum2 - sum1*sum1/K)/(K-1)/K; //variance of average
                if(v2<0) v2 = 0;
                if(v1>=0) {
                    val1 = sqrt(v1);
                    val2 = sqrt(v2/v1/4);
                    val3 = sqrt(v2);
                }
                else  {
                    val1 = v1;
                    val2 = sqrt(v2);
                    val3 = sqrt(v2);
                }
                retval = resetSample();
            }
            else {
                val1 = v1;
                val2 = v2;
                val3 = v2;
                retval = resetSample();
                egsWarning("Error in getResult(): not enough samples for positioning uncertainty estimation\n");
            }
            return retval;
        };
        /*get current sampling score*/
        int getScore(EGS_I64 &k, EGS_Float &val1, EGS_Float &val2){
            int retval = -7;
            k = K;
            val1 = sum1;
            val2 = sum2;
            retval = 0;
            return retval;
        };
        /*set current sampling score*/
        int setScore(EGS_I64 k, EGS_Float val1, EGS_Float val2){
            int retval = -8;
            K = k;
            sum1 = val1;
            sum2 = val2;
            retval = resetSample();
            return retval;
        };

private:
        EGS_I64 N, M, K; //integers used during summation: number of positions for a sample, number of cases for a position, number of samples

        EGS_I64 previous_score_case; //case counter

        EGS_I64 McasePerPos, NposPerSample; //integer set during efficiency optimization

        bool correlation; //flag for correlation on or off

        EGS_Float sum1, sum2; //variables to sum positioning uncertainty estimator in each batch

        EGS_Float sumdij, sumdij2, sumDi, sumDi2, sumvarDi; //sum variables

        EGS_Float sumeij, sumeij2, sumEi, sumEi2, sumvarEi; //sum variables

        EGS_Float sumdijeij, sumDiEi, sumcovarDiEi;// cum variables
};
//*HB_end**************************

class APP_EXPORT EGS_ChamberApplication : public EGS_AdvancedApplication {

public:

    /*! Constructor */
    EGS_ChamberApplication(int argc, char **argv) :
        EGS_AdvancedApplication(argc,argv), ngeom(0), dose(0),
        fsplit(1), fspliti(1), rr_flag(0), Esave(0), rho_rr(1),
	cgeom(0), nsmall_step(0), ncg(0), do_cse(0), do_TmpPhsp(0),
	cgeoms(0), nsubgeoms(0), check_for_subreg(0), is_subgeomreg(0), subgeoms(0),
	container(0), container2(0), container3(0), save_dose(0), silent(0) ,
        iso_pu_flag(0), cav_pu_flag(0), iso_pu_do_shift(0), cav_pu_do_shift(0),pu_flag(0),
        McasePerPos(0), NposPerSample(0), onegeom(0), csplit(1) {  };

    /*! Destructor.  */
    ~EGS_ChamberApplication() {
        if( dose )  delete dose;
        if( ngeom > 0 ) {
            delete [] geoms; delete [] mass; int j;
            for(j=0; j<ngeom; j++) if( transforms[j] ) delete transforms[j];
            delete [] transforms;
            for(j=0; j<ngeom; j++) delete [] is_cavity[j];
            delete [] is_cavity;
            for(j=0; j<ngeom; j++) delete [] cs_enhance[j];
            delete [] cs_enhance;
            for(j=0; j<ngeom; j++) delete [] subgeoms[j];
            delete [] nsubgeoms;
            for(j=0; j<ngeom; j++) delete [] is_subgeomreg[j];
            delete [] is_subgeomreg;
        }
        if( ncg > 0 ) {
            delete [] gind1; delete [] gind2; delete [] scg;
        }
    };

    void startNewParticle();

    /*! Describe the application.  */
    void describeUserCode() const;

    void do_cs_enhancement(EGS_Float &GMFP);

    /*! Describe the simulation */
    void describeSimulation();

    /*! Initialize scoring.  */
    int initScoring();

    /*! Accumulate quantities of interest at run time */
    int ausgab(int iarg);

    /*  Re-implementation of runSimulation in order to extract information on the
        number of histories run
     */
    int runSimulation();

    /* For eventual implementation*/
    //int combineResults();

    /*! Simulate a single shower.
        We need to do special things and therefore reimplement this method.
     */
    int simulateSingleShower();

    /*! Output intermediate results to the .egsdat file. */
    int outputData();

    /*! Read intermediate results to the .egsdat file. */
    int readData();

    /*! Reset the variables used for accumulating results */
    void resetCounter();

    /*! Add simulation results */
    int addState(istream &data);

    /*! Output the results of a simulation. */
    void outputResults();

    /*! Get the current simulation result.  */
    void getCurrentResult(double &sum, double &sum2, double &norm,
            double &count);

    /*! simulate a shower */
    int shower();

    /* Select photon mean-free-path */
    void selectPhotonMFP(EGS_Float &dpmfp);

    int rangeDiscard(EGS_Float tperp, EGS_Float range) const ;

protected:

    /*! Start a new shower.  */
    int startNewShower();

private:

    EGS_I64          ncase;

    int              ngeom;     // number of geometries to calculate
                                // quantities of interest
    int              ig;        // current geometry index

    int              ncg;       // number of correlated geometry pairs.
    int              *gind1,
                     *gind2;    // indeces of correlated geometries
    double           *scg;      // sum(dose(gind1[j])*dose(gind2[j]);

    EGS_BaseGeometry **geoms;   // geometries for which to calculate the
                                // quantites of interest.
    EGS_AffineTransform **transforms;
                                // transformations to apply before transporting
                                // for each geometry
    bool             **is_cavity; // array of flags for each region in each
                                // geometry, which is true if the region
                                // belongs to the cavity and false otherwise
    EGS_ScoringArray *dose;     // scoring array for dose scoring in each of
                                // the calculation geometries.
    EGS_Float        *mass;     // mass of the material in the cavity.

    EGS_Float        fsplit;    // photon splitting number
    EGS_Float        fspliti;   // inverse photon splitting number
    int              csplit;    // radiative splitting number

    /*! Range rejection flag
      If set to 0, no range rejection is used
      If set to 1, charged particles that can not enter the cavity are
      immediately discarded.
      If > 1, Russian Roulette (RR) with survival probability 1/rr_flag
      is played with charged particles that can not enter the cavity.
     */
    int              rr_flag;   // range rejection flag:
    /*! Save energy for range rejection
      For rr_flag = 1, electrons are range-discarded if E<Esave
      For rr_flag > 1, electrons in the cavity are range-discarded if
      E<Esave, electrons outside of the cavity are always rouletted,
      no matter what their energy.
     */
    EGS_Float        Esave;
    /*! Mass density of the range rejection medium */
    EGS_Float        rho_rr;
    /*! Cavity bounding geometry.
      If no cavity bounding geometry is defined, range-rejection of RR
      is used only on a region-by-region basis. If a cavity bounding geometry
      is defined, then tperp to that geometry is also checked and if greater
      than the electron range, range-rejection or RR is done.
     */
    EGS_BaseGeometry *cgeom;
    /*! Range interpolators */
    EGS_Interpolator rr_erange;
    EGS_Interpolator rr_prange;
    int nsmall_step;

    static string revision;

    int **cs_enhance;
    bool do_cse;
    int do_TmpPhsp;
    TmpPhsp *container;
    TmpPhsp *container2;
    TmpPhsp *container3;
    bool do_mcav;
    EGS_BaseGeometry **cgeoms;
    //EGS_I32 ip, np;		// pointers in mortran arrays
    vector<string *> subgeoms;
    int *nsubgeoms;
    bool **is_subgeomreg;
    bool check_for_subreg;
    bool do_sub;
    int basereg;			// region of the "base" geometry where particles where scored
    EGS_Float save_dose;
    bool *has_sub;
    int silent;


    EGS_Float *rECUT;	// region based ECUT
    bool	*do_rECUT;	// flag for doing region-based ECUT
    bool    **is_rECUT;

    bool onegeom;

    //*HB_start************************

    vector<string> correlgnames;        // identifier of correlated set of geometries

    bool             iso_pu_flag, cav_pu_flag; // on-off flag for positioning uncertainty

    bool             iso_pu_do_shift, cav_pu_do_shift; //directive to do positioning shift during simulation

    vector<EGS_PosUncertDistributor*>  pu_distributor; //shift info and manager

    int              pu_flag;      // number of flag for positioning uncertainty (0, 1 or 2)

    EGS_I64          McasePerPos, NposPerSample; //positioning uncertainty parameters

    vector<EGS_PosUncertEstimator*>   pu_estimator, pu_estimator_corr;      //positioning uncertainty estimator

    //*HB_end**************************

};

const static char __egs_app_msg_my3[] = "EGS_ChamberApplication::runSimulation():";

string EGS_ChamberApplication::revision = " ";

extern __extc__  void
F77_OBJ_(select_photon_mfp,SELECT_PHOTON_MFP)(EGS_Float *dpmfp) {
    EGS_Application *a = EGS_Application::activeApplication();
    EGS_ChamberApplication *app = dynamic_cast<EGS_ChamberApplication *>(a);
    if( !app ) egsFatal("select_photon_mfp called with active application "
            " not being of type EGS_ChamberApplication!\n");
    app->selectPhotonMFP(*dpmfp);
}

extern __extc__ void F77_OBJ_(range_discard,RANGE_DISCARD)(
        const EGS_Float *tperp, const EGS_Float *range) {
    EGS_ChamberApplication *app = dynamic_cast<EGS_ChamberApplication *>(
            EGS_Application::activeApplication());
    the_epcont->idisc = app->rangeDiscard(*tperp,*range);
}
extern __extc__ void F77_OBJ_(egs_scale_xcc,EGS_SCALE_XCC)(const int *,
                                  const EGS_Float *);
#define egsScaleXsection F77_OBJ_(egs_scale_photon_xsection,EGS_SCALE_PHOTON_XSECTION)
extern __extc__ void egsScaleXsection(const int *imed, const EGS_Float *fac,
                                      const int *which);

extern __extc__ void F77_OBJ_(do_cs_enhancement,DO_CS_ENHANCEMENT)(EGS_Float *gmfp){
     EGS_Application *a = EGS_Application::activeApplication();
     EGS_ChamberApplication *app = dynamic_cast<EGS_ChamberApplication *>(a);
     if( !app ) egsFatal("do_cs_enhancement called with active application "
            " not being of type EGS_ChamberApplication!\n");
     app->do_cs_enhancement(*gmfp);
}

void EGS_ChamberApplication::startNewParticle() {
    EGS_AdvancedApplication::startNewParticle();
    nsmall_step = 0;
};

void EGS_ChamberApplication::do_cs_enhancement(EGS_Float &gmfp) {
	// rayleigh-correction-function is called during 'normal' egs-photon routine
	// (not selectPhotonMFP in here for photon-splitting)
	// it is misused here to enhance photon cross-sections (adopted from dosrznrc.mortran)
	// GMFP is passed from egs-photon routine
	int np = the_stack->np-1;
	int ir = the_stack->ir[np]-2;
	if(cs_enhance[ig][ir] > 1 )
		gmfp /= cs_enhance[ig][ir];
	return;
}

/*! Describe the application.  */
void EGS_ChamberApplication::describeUserCode() const {
    egsInformation(
      "\n               *************************************************"
      "\n               *                                               *"
      "\n               *               egs_chamber CSE)                *"
      "\n               *                                               *"
      "\n               *************************************************"
      "\n\n");
    egsInformation("This is EGS_ChamberApplication %s based on\n"
      "      EGS_AdvancedApplication %s\n\n",
      egsSimplifyCVSKey(revision).c_str(),
      egsSimplifyCVSKey(base_revision).c_str());

};

void EGS_ChamberApplication::describeSimulation() {
    EGS_AdvancedApplication::describeSimulation();
    egsInformation("Variance reduction\n"
            "====================================================\n");
    egsInformation("Photon splitting = ");
    if( fsplit > 1 ) egsInformation("%g\n",fsplit);
    else egsInformation("off\n");
    egsInformation("Range rejection = ");
    if( rr_flag == 0 ) egsInformation("off\n");
    else if( rr_flag == 1 ) egsInformation("on for E < %g\n",Esave);
    else {
        egsInformation("Russian Roullette (RR)\n");
        egsInformation("    rejection in cavity for E < %g\n",Esave);
        egsInformation("    else RR with survival probability %g\n",
                1./rr_flag);
        if (do_mcav) {
            egsInformation("	multiple cavity option is turned ON\n");
        	egsInformation("	following cavity-geometries are used:\n");
        	for (int i=0; i<ngeom; i++)
        		egsInformation("	%i) %s -> %s\n", i+1, geoms[i]->getName().c_str(),  cgeoms[i]->getName().c_str() );
        }
        else {
            if (cgeom)
                egsInformation("    rejection geometry is %s\n",
                cgeom->getName().c_str());
            else
                egsInformation("    on a region-by-region basis\n");
        }
    }
    if (do_TmpPhsp){
	egsInformation("\nscoring phase-space at cavity of '%s'",geoms[0]->getName().c_str());
    	if(do_TmpPhsp>1)egsInformation("\n recycling %i times",do_TmpPhsp);
	if(do_sub &! silent){ egsInformation("\n\n subgeometries:");
		for(int j=0; j<ngeom; j++) {
			if(nsubgeoms[j]>0 && has_sub[j]){
				egsInformation("\n %i) %s", j+1, geoms[j]->getName().c_str());
				for(int i=0; i<nsubgeoms[j];i++){
					egsInformation("\n	- %s",subgeoms[j][i].c_str());
				}
				egsInformation("\n 	(regions:");
				for(int k=0; k<geoms[j]->regions(); k++)
					if(is_subgeomreg[j][k]) egsInformation(" %i",k);
				egsInformation(")\n");
		}
		}
	}
    }
    egsInformation("\n");
    egsInformation("\nphoton cross-section enhancement");
    if(do_cse) {
        egsInformation(" = On");
        if (!silent) {
            for(int j=0; j<ngeom; j++) {
                egsInformation("\n %i) %s", j+1, geoms[j]->getName().c_str());
                egsInformation("\n    regions    : ");
                for (int k=0; k<geoms[j]->regions(); k++){
                    if(cs_enhance[j][k] > 1)egsInformation(" %i",k);
                }
                egsInformation("\n    enhancement: ");
                for (int k=0; k<geoms[j]->regions(); k++){
                    if(cs_enhance[j][k] > 1)egsInformation(" %i",cs_enhance[j][k]);
                }
                egsInformation("\n");
            }
        }
    }
    else egsInformation(" = Off\n");
    egsInformation("\n");

    egsInformation("\nregion by region ECUT");
    bool recut = false;
    for(int j=0; j<ngeom; j++)
        if(do_rECUT[j])
            recut = true;


    if(recut){
        for(int j=0; j<ngeom; j++) {
           if(do_rECUT[j]){
               egsInformation("\n %i) %s", j+1, geoms[j]->getName().c_str());
               egsInformation("\n    with ECUT of %g MeV",rECUT[j]);
               egsInformation("\n    regions    : ");
	           for (int k=0; k<geoms[j]->regions(); k++)
		           if(is_rECUT[j][k])egsInformation(" %i",k);
               egsInformation("\n");
		   }
        }

    }
    else egsInformation(" = Off\n");


    if(onegeom)
        egsInformation("\n\n->assuming all geometries to be identical except defined cavity regions!!!<-\n");


    egsInformation("\n=============================================\n");
}

int EGS_ChamberApplication::initScoring() {

    //
    // **** variance reduction
    //
    EGS_Input *vr = input->takeInputItem("variance reduction");
    if( vr ) {
        //
        // ******* phase-space scoring
        //
        int tmps;
        int err815 = vr->getInput("TmpPhsp",tmps);
        if(!err815){
            do_TmpPhsp = tmps;
            container = new TmpPhsp;
            container3 = new TmpPhsp;
        }
        //
        // ******* cs enhancement
        //
        EGS_Float tmp2; int err2 = vr->getInput("cs enhancement",tmp2);
        if( err2 || tmp2 == 0 ) {
            do_cse = false;
        }
        else do_cse = true;

        //
        // ******** photon splitting
        //
        EGS_Float tmp; int err = vr->getInput("photon splitting",tmp);
        if( !err && tmp > 1 &! do_cse &! do_TmpPhsp) {
            fsplit = tmp; fspliti = 1/tmp;
        }

        //
        // ******** radiative event splitting
        //
        if( !vr->getInput("radiative splitting", csplit) && csplit > 1) {
            egsInformation("\n => initScoring: splitting radiative events %d times ...\n", csplit);
           the_egsvr->nbr_split = csplit;
        }

        //
        // ******* range rejection
        //
        EGS_Input *rr = vr->takeInputItem("range rejection");
        if( rr ) {
            int iaux; err = rr->getInput("rejection",iaux);
            if( !err && iaux >= 0 ) rr_flag = iaux;
            if( rr_flag ) {
                EGS_Float aux; err = rr->getInput("Esave",aux);
                if( !err && aux >= 0 ) Esave = aux;
                string cavity_geometry;
                err = rr->getInput("cavity geometry",cavity_geometry);
                if( !err ) {
                    EGS_BaseGeometry::setActiveGeometryList(app_index);
                    cgeom = EGS_BaseGeometry::getGeometry(cavity_geometry);
                }
                if( !cgeom || err) egsWarning("\n\n********** no geometry named"
                            " %s exists => using region-by-region rejection only\n",cavity_geometry.c_str());
                if( !Esave && rr_flag == 1 ) {
                    egsWarning("\n\n********* rr_flag = 1 but Esave = 0 =>"
                            " not using range rejection\n\n");
                    rr_flag = 0;
                }
                if( rr_flag && cgeom ) {
                    string rej_medium; int irej_medium = -1;
                    err = rr->getInput("rejection range medium",rej_medium);
                    if( !err ) {
                        EGS_BaseGeometry::setActiveGeometryList(app_index);
                        int nmed = cgeom->nMedia();
                        int imed = cgeom->addMedium(rej_medium);
                        if( imed >= nmed ) egsWarning(
                                "\n\n*********** no medium"
                                " with name %s initialized => "
                                "using region-by-region rejection only\n",
                                rej_medium.c_str());
                        else irej_medium = imed;
                    }
                    if( irej_medium < 0 ) { cgeom = 0; rr_flag = 1; }
                    else {
                        //
                        // *** prepare an interpolator for the electron range
                        //     in the range rejection medium
                        //
                        int i = irej_medium; // save some typing
                        rho_rr = the_media->rho[i];
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
                        rr_erange.initialize(nbin,log_emin,log_emax,erange);
                        rr_prange.initialize(nbin,log_emin,log_emax,prange);
                    }
                }
            }
            delete rr;
        }
        delete vr;
    }
    the_egsvr->i_do_rr = rr_flag;

    EGS_Input *options = input->takeInputItem("scoring options");

    if( options ) {

        options->getInput("silent",silent);

        //
        // *********** photon cross section scaling
        //
        EGS_Input *scaling;
        EGS_BaseGeometry::setActiveGeometryList(app_index);
        while( (scaling = options->takeInputItem("scale photon x-sections")) ) {
            EGS_Float factor; string medname; int what;
            int err1 = scaling->getInput("factor",factor);
            int err2 = scaling->getInput("medium",medname);
            vector<string> allowed;
            allowed.push_back("all"); allowed.push_back("Rayleigh");
            allowed.push_back("Compton"); allowed.push_back("Pair");
            allowed.push_back("Photo");
            what = scaling->getInput("cross section",allowed,0);
            if( !err1 && !err2 ) {
                int imed;
                if( medname == "ALL" || medname == "all" || medname == "All" )
                    imed = 0;
                else {
                    EGS_BaseGeometry::setActiveGeometryList(app_index);
                    imed = EGS_BaseGeometry::addMedium(medname); ++imed;
                    if( imed > the_media->nmed ) {
                        egsInformation("Scaling requested for medium %s,"
                                " but such medium does not exist\n",medname.c_str());
                        imed = -1;
                    }
                }
                if( imed >= 0 )
                    egsScaleXsection(&imed,&factor,&what);
            }
            delete scaling; scaling = 0;

        }

        //
        // *** onegeom ***
        //
        int tmp_onegeom=0;
        options->getInput("onegeom", tmp_onegeom);
        if(tmp_onegeom == 1)
            onegeom = true;
        else
        onegeom = false;

        //
        // ********* scale elastic scattering
        //
        EGS_Input *scale;
        while( (scale = options->takeInputItem("scale xcc")) ) {
            vector<EGS_Float> tmp;
            int err = scale->getInput("scale xcc",tmp);
            if( !err ) {
                int im = (int) tmp[0]; ++im;
                egsInformation("Scaling xcc of medium %d with %g\n",im,tmp[1]);
                F77_OBJ_(egs_scale_xcc,EGS_SCALE_XCC)(&im,&tmp[1]);
            }
            delete scale;
        }

        //
        // *********** calculation geometries
        //
        vector<EGS_BaseGeometry *> geometries;
        vector<int *>  cavity_regions;
        vector<int>  n_cavity_regions;
        vector<int *>  enhance_regions;
        vector<int>  n_enhance_regions;
        vector<int *>  enhance_fac;
        vector<EGS_Float> cavity_masses;
        vector<EGS_AffineTransform *> transformations;
        EGS_Input *aux;
        EGS_BaseGeometry::setActiveGeometryList(app_index);
        vector<EGS_BaseGeometry *> cav_geoms;
        vector<int>  n_subgeometries;
        vector<int *>  subgeometry_regions;
        vector<int>  n_subgeometry_regions;

        vector<int *> ecut_regions;
        vector<int> ecut_regions_nr;
        vector<EGS_Float> ecut_val;

        do_mcav = true;
        while( (aux = options->takeInputItem("calculation geometry")) ) {
            string gname;
            int err = aux->getInput("geometry name",gname);

            string cavString;
            vector<int> cav;
            int err1 = aux->getInput("cavity regions",cavString);

            string ecut_rString;
            vector<int> ecut_r;
            EGS_Float ecut_v;
            int err5 = aux->getInput("ECUT regions",ecut_rString);	// jwu
            int err6 = aux->getInput("ECUT",ecut_v);

            string cav_gname;
            int err999 = aux->getInput("cavity geometry", cav_gname);
            if (err999){
                egsWarning("\ninitScoring: missing/wrong 'cavity geometry' "
                        " for geometry '%s' \n", gname.c_str());
                do_mcav = false;
            }
            int err11, err12;
            string cs_regString;
            vector<int> cs_reg;
            vector<int> cs_fac;
            if( do_cse ) {
                err11 = aux->getInput("enhance regions",cs_regString);
                err12 = aux->getInput("enhancement",cs_fac);
            }
            else{ err11 = 1; err12 = 1; }

            EGS_Float cmass;
            int err2 = aux->getInput("cavity mass",cmass);
            if( err ) egsWarning("initScoring: missing/wrong 'geometry name' "
                    "input\n");
            if( err1 ) egsWarning("initScoring: missing/wrong 'cavity regions' "
                    "input\n");
            if( err2 ) {
                egsWarning("initScoring: missing/wrong 'cavity mass' "
                        "input\n"); cmass = -1;
            }
            if( err11 ) egsWarning("initScoring: missing/wrong 'enhance regions' "
                    "input\n");
            if( err12 ) egsWarning("initScoring: missing/wrong 'enhancement' "
                    "input\n");
            int err13 = 0;

            if( err || err1 ) egsWarning("  --> input ignored\n");
            else {
                EGS_BaseGeometry::setActiveGeometryList(app_index);

                EGS_BaseGeometry *cg = EGS_BaseGeometry::getGeometry(cav_gname);
                if( !cg ){
                    egsWarning("initScoring: no 'cavity geometry' named %s -->"
                            " input ignored\n",cav_gname.c_str());
                    cg = 0;
                }

                EGS_BaseGeometry *g = EGS_BaseGeometry::getGeometry(gname);
                if( !g ) {
                    egsWarning("initScoring: no geometry named %s -->"
                        " input ignored\n",gname.c_str());
                } else {
                    g->getNumberRegions(cavString, cav);
                    g->getLabelRegions(cavString, cav);
                    g->getNumberRegions(cs_regString, cs_reg);
                    g->getLabelRegions(cs_regString, cs_reg);
                    g->getNumberRegions(ecut_rString, ecut_r);
                    g->getLabelRegions(ecut_rString, ecut_r);
                }

                if (do_cse && cs_reg[0]<0 && cs_reg[1]<0 && cs_reg.size()==2) {
                    int start = -cs_reg[0];
                    int end   = -cs_reg[1];
                    cs_reg.clear();
                    for (int i=start; i<=end; i++) {
                        cs_reg.push_back(i);
                    }
                }
                if (do_cse && cs_fac[0]<0 && cs_fac.size()==1) {
                    int tmp = -cs_fac[0];
                    cs_fac.clear();
                    for (int i=0; i<cs_reg.size(); i++) {
                        cs_fac.push_back(tmp);
                    }
                }
                if( !err11 && !err12 && (cs_reg.size() != cs_fac.size() )){
                    egsWarning("initScoring: number of 'enhance regions' must match 'enhancement'\n");
                    err13 = 1;
                }

                if( g ) {

                    int nreg = g->regions();
                    int *regs = new int [cav.size()];
                    int ncav = 0;
                    for(int j=0; j<cav.size(); j++) {
                        if( cav[j] < 0 || cav[j] >= nreg )
                            egsWarning("initScoring: region %d is not within"
                                    " the allowed range of 0...%d -> input"
                                    " ignored\n",cav[j],nreg-1);
                        else regs[ncav++] = cav[j];
                    }
                    if( !ncav ) {
                        egsWarning("initScoring: no cavity regions "
                                "specified for geometry %s --> input ignored\n",
                                gname.c_str());
                        delete [] regs;
                    }
                    else {
                        geometries.push_back(g);
                        cav_geoms.push_back(cg);
                        n_cavity_regions.push_back(ncav);
                        cavity_regions.push_back(regs);
                        cavity_masses.push_back(cmass);
                        transformations.push_back(
                                EGS_AffineTransform::getTransformation(aux));


                        if( !err5 && !err6 && ecut_r.size() > 0 ) {
                            int *r = new int [ecut_r.size()]; int nr=0;
                            for(int j=0; j<ecut_r.size(); j++) {
                                if( ecut_r[j] >= 0 && ecut_r[j] < nreg ) {
                                    r[nr++] = ecut_r[j];
                                }
                            }
                            if( nr ) {
                                ecut_regions.push_back(r);
                                ecut_regions_nr.push_back(nr);
                                ecut_val.push_back(ecut_v);
                            }
                            else {
                                delete [] r;
                                ecut_regions.push_back(0);
                                ecut_regions_nr.push_back(0);
                                ecut_val.push_back(0);
                            }
                        }
                        else {
                            ecut_regions.push_back(0);
                            ecut_regions_nr.push_back(0);
                            ecut_val.push_back(0);
                        }


                        if( !err12 && !err11 && !err13 && cs_reg.size() > 0 ) {
                            int *r = new int [cs_reg.size()]; int nr=0;
                            int *cse = new int [cs_reg.size()];
                            for(int j=0; j<cs_reg.size(); j++) {
                                if( cs_reg[j] < nreg ) {
                                    r[nr] = cs_reg[j];
                                    cse[nr] = cs_fac[j];
                                    nr++;
                                }
                            }
                            if( nr ) {
                                enhance_regions.push_back(r);
                                n_enhance_regions.push_back(nr);
                                enhance_fac.push_back(cse);
                            }
                            else {
                                delete [] r;
                                enhance_regions.push_back(0);
                                n_enhance_regions.push_back(0);
                                enhance_fac.push_back(0);
                            }
                        }
                        else {
                            enhance_regions.push_back(0);
                            n_enhance_regions.push_back(0);
                            enhance_fac.push_back(0);
                        }
                    }

                    // get the defined subgeometries
                    if ( do_TmpPhsp ){
                        vector<string> subgeom_name;
                        int err777 = aux->getInput("sub geometries",subgeom_name);
                        string subgeom_regsString;
                        vector<int> subgeom_regs;
                        int err888 = aux->getInput("subgeom regions",subgeom_regsString);
                        cg->getNumberRegions(subgeom_regsString, subgeom_regs);
                        cg->getLabelRegions(subgeom_regsString, subgeom_regs);
                        if( err777 ){
                            egsWarning("initScoring: missing/wrong 'sub geometries' "
                                    " for geometry '%s'\n", gname.c_str());
                            n_subgeometries.push_back(0);
                            subgeoms.push_back(0);
                        }
                        else if(!err888){
                            int nr = 0;
                            string *gnam = new string[subgeom_name.size()];
                            for(int i=0; i<subgeom_name.size(); i++){
                                EGS_BaseGeometry *g = EGS_BaseGeometry::getGeometry(subgeom_name[i]);
                                if(g) {
                                    nr++;
                                    gnam[i] = subgeom_name[i];
                                }
                                else egsFatal("\n geometry %s does not exist!\n\n", subgeom_name[i].c_str());
                            }
                            if( nr ){
                                n_subgeometries.push_back(nr);
                                subgeoms.push_back(gnam);
                            }
                            else {
                                delete [] gnam;
                                n_subgeometries.push_back(0);
                                subgeoms.push_back(0);
                            }
                        }
                        if( !err777 && !err888 && subgeom_regs.size() > 0 ) {
                            int *r = new int [subgeom_regs.size()]; int nr=0;
                            for(int j=0; j<subgeom_regs.size(); j++) {
                                if( subgeom_regs[j] >= -1 && subgeom_regs[j] < nreg )
                                    r[nr++] = subgeom_regs[j];
                            }
                            if( nr ) {
                                subgeometry_regions.push_back(r);
                                n_subgeometry_regions.push_back(nr);
                            }
                            else {
                                delete [] r;
                                subgeometry_regions.push_back(0);
                                n_subgeometry_regions.push_back(0);
                            }
                        }
                        else {
                            subgeometry_regions.push_back(0);
                            n_subgeometry_regions.push_back(0);
                        }
                    }
                    else {
                        subgeoms.push_back(0);
                        n_subgeometries.push_back(0);
                        subgeometry_regions.push_back(0);
                        n_subgeometry_regions.push_back(0);
                    }
                }
            }
            delete aux;
        }
        ngeom = geometries.size();
        if( !ngeom ) {
            egsWarning("initScoring: no calculation geometries defined\n");
            return 1;
        }
        is_rECUT = new bool* [ngeom];
		do_rECUT = new bool [ngeom];
	    rECUT = new EGS_Float [ngeom];

        cgeoms = new EGS_BaseGeometry* [ngeom];
        for(int j=0; j<ngeom; j++)
            cgeoms[j] = do_mcav ? cav_geoms[j] : cgeom;

        nsubgeoms = new int[ngeom];
        is_subgeomreg = new bool* [ngeom];
        geoms = new EGS_BaseGeometry* [ngeom];
        is_cavity   = new bool* [ngeom];
        cs_enhance = new int* [ngeom];
        mass = new EGS_Float [ngeom];
        dose = new EGS_ScoringArray(ngeom);
        transforms = new EGS_AffineTransform* [ngeom];
        do_cse = false;
        has_sub = new bool [ngeom];
        //if(do_TmpPhsp && n_cavity_regions[0] > 1)
        //    egsFatal("\nTmpPhsp: the first geometry can only have one cavity-region!\n\n");

        for(int j=0; j<ngeom; j++) {
			do_rECUT[j] = false;
            geoms[j] = geometries[j]; //geoms[j]->ref();
            mass[j] = cavity_masses[j];
            transforms[j] = transformations[j];
            int nreg = geoms[j]->regions();
            is_cavity[j]  = new bool [nreg];
            cs_enhance[j] = new int[nreg];
            is_subgeomreg[j]  = new bool [nreg];
            int i;
            for(i=0; i<nreg; i++){
                is_cavity[j][i]     = false;
                cs_enhance[j][i]    = 1;
                is_subgeomreg[j][i] = false;
            }
            for(i=0; i<n_subgeometry_regions[j]; i++) {
                int ireg = subgeometry_regions[j][i];
                if (ireg == -1)
                    has_sub[j] = false;
                else{
                    has_sub[j] = true;
                    is_subgeomreg[j][ireg] = true;
                }
            }
            nsubgeoms[j] = n_subgeometries[j];
            int imed = -999;
            for(i=0; i<n_cavity_regions[j]; i++) {
                int ireg = cavity_regions[j][i];
                is_cavity[j][ireg] = true;
                if( imed == -999 ) imed = geoms[j]->medium(ireg);
                else {
                    int imed1 = geoms[j]->medium(ireg);
                    if( imed1 != imed ) egsWarning("initScoring: different "
                            "medium %d in region %d compared to medium %d in "
                            "region %d for geometry %s.\nHope you know what you are doing\n",
                            imed1,ireg,imed,cavity_regions[j][0],geoms[j]->getName().c_str());
                }
            }
            if ( n_enhance_regions[j] > 0 ){
                if( enhance_regions[j][0] >= 0 )
                    for(i=0; i<n_enhance_regions[j]; i++) {
                        int ireg = enhance_regions[j][i];
                        cs_enhance[j][ireg] = enhance_fac[j][i];
                        do_cse = true;
                    }
                else
                    //*HB_start************************
                    for(i=-enhance_regions[j][0]; i<nreg; i++) {
                        cs_enhance[j][i] = enhance_fac[j][0];
                        do_cse = true;
                    }
                    //*HB_end**************************
            }


			if( ecut_regions_nr[j] > 0 ) {
			    do_rECUT[j] = true;
			    is_rECUT[j] = new bool [nreg];
			    int i; for(i=0; i<nreg; i++) is_rECUT[j][i] = false;
			    for(i=0; i<ecut_regions_nr[j]; i++){
			        is_rECUT[j][ecut_regions[j][i]] = true;
			    }
			    delete [] ecut_regions[j];
			    rECUT[j] = ecut_val[j];
            }

            delete [] cavity_regions[j];
            delete [] enhance_regions[j];
        }

        EGS_Input *aux2;
        vector<int> cor1, cor2;
        while( (aux2 = options->takeInputItem("correlated geometries")) ) {
            vector<string> cgnames;
            int err = aux2->getInput("correlated geometries",cgnames);
            //*HB_start************************
            if( !err && (cgnames.size() == 2) ||(cgnames.size() == 3) ) {
                int j1, j2;
                for(j1=0; j1<ngeom; j1++)
                    if( cgnames[0] == geoms[j1]->getName() ) break;
                for(j2=0; j2<ngeom; j2++)
                    if( cgnames[1] == geoms[j2]->getName() ) break;
                if( j1 < ngeom && j2 < ngeom ) {
                    cor1.push_back(j1); cor2.push_back(j2);
		    if(cgnames.size() == 3) //If size of gnames is 3, the 3rd string is the identifier
		    	correlgnames.push_back(cgnames[2]);
		    else correlgnames.push_back("-");
                }
            }
            //*HB_end**************************
        }
        if( cor1.size() > 0 ) {
            ncg = cor1.size();
            gind1 = new int [ncg]; gind2 = new int [ncg];
            scg = new double [ncg];
            for(int j=0; j<ncg; j++) {
                scg[j] = 0; gind1[j] = cor1[j]; gind2[j] = cor2[j];
            }
        }
        delete aux2;

	//*HB_start************************
	//
        // **** positioning uncertainty
        //
        EGS_Input *ipu, *ipu_transl, *ipu_rot;
        //isocenter positioning uncertainty
        if( (ipu = options->takeInputItem("isocenter positioning uncertainty")) )
        {
            EGS_PosUncertDistributor iso_pu_distributor;
            if(do_TmpPhsp)
            {
                egsInformation("\nReading isocenter positioning uncertainty input...\n");
                int dmb;
                EGS_I64 tmp;
                vector<EGS_Float> tmp1;
                string stmp;
                int  typeTransl = 0, typeRot = 0;
                EGS_Vector   sigma_transl = EGS_Vector(), max_transl = EGS_Vector(), sigma_rot = EGS_Vector(), max_rot = EGS_Vector();
                dmb = ipu->getInput("ncase per position",tmp);
                if(dmb) McasePerPos = 10;
                else McasePerPos = tmp;
                if (McasePerPos < 2) McasePerPos = 2;
                dmb = ipu->getInput("positions per sample",tmp);
                if(dmb) NposPerSample = 10;
                else NposPerSample = tmp;
                if (NposPerSample < 2) NposPerSample = 2;
                if( (ipu_transl = ipu->takeInputItem("translation")) ) {
                    dmb = ipu_transl->getInput("distribution",stmp);
                    if (!dmb) {
                        if(!stmp.compare("gaussian"))     typeTransl = 0;
                        else if(!stmp.compare("uniform")) typeTransl = 1;
                        else
                            egsWarning("Wrong/absent values for 'distribution' in the isocenter positioning uncertainty input 'translation', will set to 'gaussian'.\n");
                    }
                    else
                        egsWarning("Wrong/absent values for 'distribution' in the isocenter positioning uncertainty input 'translation', will set to 'gaussian'.\n");
                    dmb = ipu_transl->getInput("max shift",tmp1);
                    if(dmb||tmp1.size()!=3)
                        egsWarning("Wrong/absent values for 'max shift' in the isocenter positioning uncertainty input 'translation', will set to 0.\n");
                    else {
                        max_transl.x = tmp1[0];
                        max_transl.y = tmp1[1];
                        max_transl.z = tmp1[2];
                    }
                    if(typeTransl==0) {
                        dmb = ipu_transl->getInput("sigma",tmp1);
                        if(dmb||tmp1.size()!=3)
                            egsWarning("Wrong/absent values for 'sigma' in the isocenter positioning uncertainty input 'translation', will set to 0.\n");
                        else {
                            sigma_transl.x = tmp1[0];
                            sigma_transl.y = tmp1[1];
                            sigma_transl.z = tmp1[2];
                        }
                    }
                    else sigma_transl = max_transl;
                }
                iso_pu_distributor.initTranslation(sigma_transl,max_transl);
                iso_pu_distributor.initTypeTransl(typeTransl);

                if( (ipu_rot = ipu->takeInputItem("rotation")) ) {
                    dmb = ipu_rot->getInput("distribution",stmp);
                    if (!dmb) {
                        if(!stmp.compare("gaussian"))     typeRot = 0;
                        else if(!stmp.compare("uniform")) typeRot = 1;
                        else
                            egsWarning("Wrong/absent values for 'distribution' in the isocenter positioning uncertainty input 'rotation', will set to 'gaussian'.\n");
                    }
                    else
                        egsWarning("Wrong/absent values for 'distribution' in the isocenter positioning uncertainty input 'rotation', will set to 'gaussian'.\n");
                    dmb = ipu_rot->getInput("max shift",tmp1);
                    if(dmb||tmp1.size()!=3)
                        egsWarning("Wrong/absent values for 'max shift' in the isocenter positioning uncertainty input 'rotation', will set to 0.\n");
                    else {
                        max_rot.x = tmp1[0];
                        max_rot.y = tmp1[1];
                        max_rot.z = tmp1[2];
                    }
                    if(typeRot==0) {
                        dmb = ipu_rot->getInput("sigma",tmp1);
                        if(dmb||tmp1.size()!=3)
                            egsWarning("Wrong/absent values for 'sigma' in the isocenter positioning uncertainty input 'rotation', will set to 0.\n");
                        else {
                            sigma_rot.x = tmp1[0];
                            sigma_rot.y = tmp1[1];
                            sigma_rot.z = tmp1[2];
                        }
                    }
                    else sigma_rot = max_rot;
                }
                iso_pu_distributor.initRotation(sigma_rot,max_rot);
                iso_pu_distributor.initTypeRot(typeRot);
                iso_pu_flag = iso_pu_distributor.validateInput();

                if(iso_pu_flag) {
                    EGS_PosUncertDistributor *tmpdstr1 = new EGS_PosUncertDistributor();
                    *tmpdstr1 = iso_pu_distributor;
                    pu_distributor.push_back(tmpdstr1);
                }
            }
            else
                egsWarning("!!!Must have do_TmpPhsp option ON to calculate positioning uncertainty!!!\n");
        }
        else
            iso_pu_flag = false;

        //cavity positioning uncertainty
        if( (ipu = options->takeInputItem("cavity positioning uncertainty")) )
        {
            EGS_PosUncertDistributor cav_pu_distributor;
            if(do_TmpPhsp)
            {
                egsInformation("\nReading cavity positioning uncertainty input...\n");
                int dmb;
                EGS_I64 tmp;
                vector<EGS_Float> tmp1;
                string stmp;
                int  typeTransl = 0, typeRot = 0;
                EGS_Vector   sigma_transl = EGS_Vector(), max_transl = EGS_Vector(), sigma_rot = EGS_Vector(), max_rot = EGS_Vector();
                dmb = ipu->getInput("ncase per position",tmp);
                if(dmb) {if(McasePerPos==0) McasePerPos = 10;}
                else McasePerPos = tmp;
                if (McasePerPos < 2) McasePerPos = 2;
                dmb = ipu->getInput("positions per sample",tmp);
                if(dmb) {if(NposPerSample==0) NposPerSample = 10;}
                else NposPerSample = tmp;
                if (NposPerSample < 2) NposPerSample = 2;

                if( (ipu_transl = ipu->takeInputItem("translation")) ) {
                    dmb = ipu_transl->getInput("distribution",stmp);
                    if (!dmb) {
                        if(!stmp.compare("gaussian"))     typeTransl = 0;
                        else if(!stmp.compare("uniform")) typeTransl = 1;
                        else
                            egsWarning("Wrong/absent values for 'distribution' in the cavity positioning uncertainty input 'translation', will set to 'gaussian'.\n");
                    }
                    else
                        egsWarning("Wrong/absent values for 'distribution' in the cavity positioning uncertainty input 'translation', will set to 'gaussian'.\n");
                    dmb = ipu_transl->getInput("max shift",tmp1);
                    if(dmb||tmp1.size()!=3)
                        egsWarning("Wrong/absent values for 'max shift' in the cavity positioning uncertainty input 'translation', will set to 0.\n");
                    else {
                        max_transl.x = tmp1[0];
                        max_transl.y = tmp1[1];
                        max_transl.z = tmp1[2];
                    }
                    if(typeTransl==0) {
                        dmb = ipu_transl->getInput("sigma",tmp1);
                        if(dmb||tmp1.size()!=3)
                            egsWarning("Wrong/absent values for 'sigma' in the cavity positioning uncertainty input 'translation', will set to 0.\n");
                        else {
                            sigma_transl.x = tmp1[0];
                            sigma_transl.y = tmp1[1];
                            sigma_transl.z = tmp1[2];
                        }
                    }
                    else sigma_transl = max_transl;
                }
                cav_pu_distributor.initTranslation(sigma_transl,max_transl);
                cav_pu_distributor.initTypeTransl(typeTransl);

                if( (ipu_rot = ipu->takeInputItem("rotation")) ) {
                    dmb = ipu_rot->getInput("distribution",stmp);
                    if (!dmb) {
                        if(!stmp.compare("gaussian"))     typeRot = 0;
                        else if(!stmp.compare("uniform")) typeRot = 1;
                        else
                            egsWarning("Wrong/absent values for 'distribution' in the cavity positioning uncertainty input 'rotation', will set to 'gaussian'.\n");
                    }
                    else
                        egsWarning("Wrong/absent values for 'distribution' in the cavity positioning uncertainty input 'rotation', will set to 'gaussian'.\n");
                    dmb = ipu_rot->getInput("max shift",tmp1);
                    if(dmb||tmp1.size()!=3)
                        egsWarning("Wrong/absent values for 'max shift' in the cavity positioning uncertainty input 'rotation', will set to 0.\n");
                    else {
                        max_rot.x = tmp1[0];
                        max_rot.y = tmp1[1];
                        max_rot.z = tmp1[2];
                    }
                    if(typeRot==0) {
                        dmb = ipu_rot->getInput("sigma",tmp1);
                        if(dmb||tmp1.size()!=3)
                            egsWarning("Wrong/absent values for 'sigma' in the cavity positioning uncertainty input 'rotation', will set to 0.\n");
                        else {
                            sigma_rot.x = tmp1[0];
                            sigma_rot.y = tmp1[1];
                            sigma_rot.z = tmp1[2];
                        }
                    }
                    else sigma_rot = max_rot;
                }
                cav_pu_distributor.initRotation(sigma_rot,max_rot);
                cav_pu_distributor.initTypeRot(typeRot);
                cav_pu_flag = cav_pu_distributor.validateInput();

                if(cav_pu_flag) {
                    EGS_PosUncertDistributor *tmpdstr2 = new EGS_PosUncertDistributor();
                    *tmpdstr2 = cav_pu_distributor;
                    pu_distributor.push_back(tmpdstr2);
                }
            }
            else
                egsWarning("!!!Must have do_TmpPhsp option ON to calculate positioning uncertainty!!!\n");
        }
        else
            cav_pu_flag = false;

        //Geometry validation

        /*Positioning uncertainty: output information and initialization of variables*/
        egsInformation("\n=========================positioning uncertainty=========================");
        pu_flag = 0;
        int jpu = 0;
        if(iso_pu_flag) {
            iso_pu_do_shift = true;
            egsInformation("\nIsocenter positioning uncertainty option is set ON.\n");
            egsInformation("\nTranslation:\n");
            if(pu_distributor[jpu]->returnTypeTransl()==0)
                egsInformation("\tdistribution = gaussian\n");
            else if(pu_distributor[jpu]->returnTypeTransl()==1)
                egsInformation("\tdistribution = uniform\n");
            EGS_Vector tmp = pu_distributor[jpu]->getSigmaTranslation();
            egsInformation("\tsigma     (x,y,z) = %f, %f, %f\n",tmp.x,tmp.y,tmp.z);
            tmp = pu_distributor[jpu]->getMaxTranslation();
            egsInformation("\tmax shift (x,y,z) = %f, %f, %f\n",tmp.x,tmp.y,tmp.z);

            egsInformation("Rotation:\n");
            if(pu_distributor[jpu]->returnTypeRot()==0)
                egsInformation("\tdistribution = gaussian\n");
            else if(pu_distributor[jpu]->returnTypeRot()==1)
                egsInformation("\tdistribution = uniform\n");
            tmp = pu_distributor[jpu]->getSigmaRotation();
            egsInformation("\tsigma     (x,y,z) = %f, %f, %f\n",tmp.x,tmp.y,tmp.z);
            tmp = pu_distributor[jpu]->getMaxRotation();
            egsInformation("\tmax shift (x,y,z) = %f, %f, %f\n",tmp.x,tmp.y,tmp.z);
            iso_pu_do_shift = true;
            pu_flag++;
            jpu++;
        }
        else {
            egsInformation("\nIsocenter positioning uncertainty option is set OFF.\n");
            iso_pu_do_shift = false;
        }
        if(cav_pu_flag) {
            cav_pu_do_shift = true;
            egsInformation("\nCavity positioning uncertainty option is set ON.\n");
            egsInformation("\nTranslation:\n");
            if(pu_distributor[jpu]->returnTypeTransl()==0)
                egsInformation("\tdistribution = gaussian\n");
            else if(pu_distributor[jpu]->returnTypeTransl()==1)
                egsInformation("\tdistribution = uniform\n");
            EGS_Vector tmp = pu_distributor[jpu]->getSigmaTranslation();
            egsInformation("\tsigma     (x,y,z) = %f, %f, %f\n",tmp.x,tmp.y,tmp.z);
            tmp = pu_distributor[jpu]->getMaxTranslation();
            egsInformation("\tmax shift (x,y,z) = %f, %f, %f\n",tmp.x,tmp.y,tmp.z);

            egsInformation("Rotation:\n");
            if(pu_distributor[jpu]->returnTypeRot()==0)
                egsInformation("\tdistribution = gaussian\n");
            else if(pu_distributor[jpu]->returnTypeRot()==1)
                egsInformation("\tdistribution = uniform\n");
            tmp = pu_distributor[jpu]->getSigmaRotation();
            egsInformation("\tsigma     (x,y,z) = %f, %f, %f\n",tmp.x,tmp.y,tmp.z);
            tmp = pu_distributor[jpu]->getMaxRotation();
            egsInformation("\tmax shift (x,y,z) = %f, %f, %f\n",tmp.x,tmp.y,tmp.z);
            cav_pu_do_shift = true;
            pu_flag++;
        }
        else {
            egsInformation("\nCavity positioning uncertainty option is set OFF.\n");
            cav_pu_do_shift = false;
        }
        if(pu_flag){
            for(int i=1;i<ngeom;i++) {
                EGS_PosUncertEstimator *putmp = new EGS_PosUncertEstimator();
                pu_estimator.push_back(putmp);
                pu_estimator[i-1]->initMcasePerPos(McasePerPos);
                pu_estimator[i-1]->initNposPerSample(NposPerSample);
                pu_estimator[i-1]->initCorrelation(false);
            }
            if(ncg) {
                egsInformation("\nCorrelated positioning uncertainty estimators for the following geometries:\n");
                for(int j=0; j<ncg; j++) {
                    EGS_PosUncertEstimator *putmp = new EGS_PosUncertEstimator();
                    pu_estimator_corr.push_back(putmp);
                    pu_estimator_corr[j]->initMcasePerPos(McasePerPos);
                    pu_estimator_corr[j]->initNposPerSample(NposPerSample);
                    pu_estimator_corr[j]->initCorrelation(true);
                    egsInformation("\tGeometry #%d / geometry #%d\n",gind1[j],gind2[j]);
                }
            }
        }
        if(pu_estimator.size()) {
            egsInformation("\nOptimizable parameters set to:\n");
            egsInformation("McasePerPos = %d\n",McasePerPos);
            egsInformation("NposPerSample = %d\n",NposPerSample);
        }
        else {
            McasePerPos = 0;
            NposPerSample = 0;
        }
        egsInformation("\nNumber of uncorrelated estimators is %d among %d geometries.\n",pu_estimator.size(),ngeom);
        egsInformation("Number of correlated estimators is %d among %d geometries.\n",pu_estimator_corr.size(),ngeom);
        egsInformation("\n=========================================================================");

        //
        // **** End positioning uncertainty
        //
        //*HB_end**************************

        delete options;

        /********************************************/

    } //end if(options)
    else {
        egsWarning("\n\n*********** no 'scoring options' input *********\n\n");
        return 2;
    }

    if( do_TmpPhsp ){
        do_sub = false;
        if(ngeom == 1)
            egsFatal("\nTmpPhsp: define more than one geometry!\n\n");
        for(int i=1; i<ngeom; i++)
            if(nsubgeoms[i] != 0){
                do_sub = true;
                break;
            }
        if(do_sub){
            //for(int i=1; i<ngeom; i++)
            //	if(nsubgeoms[i] == 0){
            //		egsFatal("\nsubgeometries");
            //}
            container2 = new TmpPhsp;
        }
    }
    //
    // **** set up ausgab calls
    //
    int call;
    for(call=BeforeTransport; call<=ExtraEnergy; ++call)
        setAusgabCall((AusgabCall)call,true);
    for(call=AfterTransport; call<UnknownCall; ++call)
        setAusgabCall((AusgabCall)call,false);

    if( fsplit > 1 ) {
        // If fsplit > 1 we also want to
        // Russian Roulette such photons. We don't need to call ausgab after
        // photon interactions as the entire photon transport is done in
        // selectPhotonMFP() and we discard/mark scattered photons there.
        //
        setAusgabCall(AfterBrems,true);
        if( the_xoptions->eii_flag ) {
            // with EII on, we may get fluorescent events after Moller/Bhabha
            setAusgabCall(AfterMoller,true);
            setAusgabCall(AfterBhabha,true);
        }
        setAusgabCall(AfterAnnihFlight,true);
        setAusgabCall(AfterAnnihRest,true);
    }

    if ( do_TmpPhsp )
        setAusgabCall(AfterTransport, true);
    if ( do_cse ){
        setAusgabCall(BeforePair, true);
        setAusgabCall(BeforeCompton, true);
        setAusgabCall(BeforePhoto, true);
        setAusgabCall(BeforeRayleigh, true);
        setAusgabCall(BeforePhotoNuc, true);
        setAusgabCall(AfterCompton, true);
        setAusgabCall(AfterPhoto, true);
        setAusgabCall(AfterRayleigh, true);
        setAusgabCall(AfterPhotoNuc, true);
        setAusgabCall(AfterPair, true);
        setAusgabCall(AfterTransport, true);
        setAusgabCall(AfterBrems,true);
        if( the_xoptions->eii_flag ) {
            // with EII on, we may get fluorescent events after Moller/Bhabha
            setAusgabCall(AfterMoller,true);
            setAusgabCall(AfterBhabha,true);
        }
        setAusgabCall(AfterAnnihFlight,true);
        setAusgabCall(AfterAnnihRest,true);
        if( rr_flag > 1){
            setAusgabCall(BeforeBrems,true);
//             setAusgabCall(BeforeMoller,true);
//             setAusgabCall(BeforeBhabha,true);
            setAusgabCall(BeforeAnnihFlight,true);
            setAusgabCall(BeforeAnnihRest,true);
        }
    }
    return 0;
}


/*! Accumulate quantities of interest at run time */
int EGS_ChamberApplication::ausgab(int iarg) {
    int np = the_stack->np-1;
    int ir = the_stack->ir[np]-2;

    // jwu: check if we are above ECUT
    if( do_rECUT[ig] && (the_stack->iq[np] == -1 || the_stack->iq[np] == 1) )
        if( is_rECUT[ig][ir] ){
            if( the_stack->E[np] < rECUT[ig] ){
            // before discarding the particle
            // deposit all of its energy locally
            if( ir >= 0 && is_cavity[ig][ir] ) {
                EGS_Float aux = (the_stack->E[np]-the_useful->rm)*the_stack->wt[np];
                if(aux > 0){
                    dose->score(ig,aux);
                    if(check_for_subreg && nsubgeoms[ig] != 0){
                        save_dose += aux;	//get the current dose deposition
                    }
                }
            }
        the_stack->wt[np] = 0;
        }
    }

    //
    //  **** temporarily scoring phase-space
    //
    // saving the phase-space at the 'base' geometry
    if( do_TmpPhsp && ig == 0 && iarg == AfterTransport && is_cavity[ig][ir] ){
        container->set();
        the_stack->wt[np] = 0;
        basereg = ir;
        return 0;
    }
    // saving the phase-space at the defined region of geometries with subgeometries
    if( check_for_subreg && iarg == AfterTransport && is_subgeomreg[ig][ir] ){
        container2->set();
        the_stack->wt[np] = 0;
        return 0;
    }
    //
    //  **** cross-section enhancement
    //
    // (mainly adopted from dosrznrc.mortran)
    // if inside an cs_enhance_region
    if( cs_enhance[ig][ir] > 1  ){
        // devide photon in interacting and non-interacting portions
        if( iarg == BeforePair  || iarg == BeforeCompton ||
            iarg == BeforePhoto || iarg == BeforeRayleigh || iarg == BeforePhotoNuc) {
            // increase stack
            ++the_stack->np;
            if (the_stack->np > MXSTACK)
                egsFatal("\ncs enhancement: unable to add to stack"
                        "\nincrease MXSTACK in arraysizes.h!");
            // copy the photon to top of stack
            COPY_PARTICLE(np,np+1);
            //adjust the weight of interacting photon-portion
            the_stack->wt[np+1] /= cs_enhance[ig][ir];
            return 0;
        }
        if( iarg == AfterCompton || iarg == AfterPhoto || iarg == AfterRayleigh || iarg == AfterPair || iarg == AfterPhotoNuc) {
            if( rndm->getUniform()*cs_enhance[ig][ir] < 1 ){
                // remove non-interacting portion
                the_stack->wt[the_stack->npold-2] = 0;
                // keep all scattered and regain weight
                for( int ip=the_stack->npold-1; ip<=np; ip++)
                    if( the_stack->iq[ip] == 0 ) {
                        the_stack->wt[ip] *= cs_enhance[ig][ir];
                    }
            }
            else {
                // keep the interacting portion
                // throw all scattered away
                for( int ip=the_stack->npold-1; ip<=np; ip++)
                    if( the_stack->iq[ip] == 0 ) the_stack->wt[ip] = 0;
            }
            return 0;
        }
    }

    // electron splitting or RR
    // whenever the cse-factor changes from region to region the electrons
    // are split or RR'ed so a uniform weight distribution results
    // and electrons not likely to contribute to dose in region of
    // interest are reduced in number
    // however dont do this with fat electrons that survived rangerejection-RR (nbr_split > 1)
    if( iarg == AfterTransport && the_stack->iq[np] && the_extra_stack->nbr_splitting[np] < 1 ){
        // region change occured for e-/e+
        // split since cse is enhanced in this region
        if( ir >= 0 && cs_enhance[ig][the_epcont->irold-2] < cs_enhance[ig][ir] ){
            int n_esplit = cs_enhance[ig][ir]/cs_enhance[ig][the_epcont->irold-2];
            the_stack->wt[np] = the_stack->wt[np]/(EGS_Float)n_esplit;
            the_stack->np += n_esplit-1;
            if (the_stack->np > MXSTACK)
                egsFatal("\ncs enhancement: unable to add to stack"
                        "\nincrease MXSTACK in arraysizes.h");
            for(int i=np+1; i<(np+n_esplit); i++){
                COPY_PARTICLE(np,i);
            }
        }
        //play RR since cse is decreased in this reg
        if( ir >= 0 && cs_enhance[ig][the_epcont->irold-2] > cs_enhance[ig][ir] ){
            int RRprob = cs_enhance[ig][the_epcont->irold-2]/cs_enhance[ig][ir];
            //
            // ******* IK: why is nbr_split not set here?
            //
            if( rndm->getUniform()*RRprob < 1 ) {
                the_stack->wt[np] *= RRprob;
            }
            else the_stack->wt[np] = 0;
        }
        return 0;
    }

    // split up fat electrons' radiative events
    // i.e. electrons that survived range-rejection RR
    if( (the_extra_stack->nbr_splitting[np] != 0) &&
        (iarg == BeforeBrems || iarg == BeforeAnnihFlight || iarg == BeforeAnnihRest) ) {
        the_egsvr->nbr_split = the_extra_stack->nbr_splitting[np];	// split photons up
        return 0;
    }

    // RR all (low weight) radiative photons
    // that can be the descendants of cse-split electrons
    // if these are photons of brems-split (see above) reset splitting#
    if(  (iarg == AfterBrems || iarg == AfterMoller ||
          iarg == AfterBhabha || iarg == AfterAnnihFlight ||
          iarg == AfterAnnihRest) && do_cse ) {
        for(int ip=the_stack->npold-1; ip<=np; ip++)
            if( !the_stack->iq[ip] )
                if(cs_enhance[ig][ir] > 1 && the_extra_stack->nbr_splitting[ip] == 0)
                    // play RR with photons of non-fat electron radiative event
                    if( rndm->getUniform() * (EGS_Float)cs_enhance[ig][ir] < 1 ){
                        the_stack->wt[ip] *= cs_enhance[ig][ir];
                    }
                    else the_stack->wt[ip] = 0;
                else the_extra_stack->nbr_splitting[ip] = 0;	// split-photon of fat electron
        // or electron outside cse-region
        the_egsvr->nbr_split = 1;
        return 0;
    }

    //
    //  **** energy deposition
    //
    if( iarg <= ExtraEnergy ) {
        if( the_epcont->ustep < 1e-5 ) {
            if( ++nsmall_step > 10000 ) {
                egsWarning("Too many small steps: ir=%d x=(%g,%g,%g)\n",
                        ir,the_stack->x[np],the_stack->y[np],
                        the_stack->z[np]);
                the_stack->wt[np] = 0;
                nsmall_step = 0;
            }
        }
        else nsmall_step = 0;
        if(!onegeom){
            if( ir >= 0 && is_cavity[ig][ir] ) {
                EGS_Float aux = the_epcont->edep*the_stack->wt[np];
                if(aux > 0){
                    /*
                    if( fabs(the_stack->wt[np]*cs_enhance[ig][ir]/p.wt-1) > 0.01 ) {
                        egsInformation("Fat particle scoring in cavity:\n");
                        egsInformation("  np=%d q=%d E=%g wt=%g latch=%d\n",np+1,the_stack->iq[np],
                                the_stack->E[np],the_stack->wt[np],the_stack->latch[np]);
                        egsInformation("  Initial particle was:\n");
                        egsInformation("  q=%d E=%g wt=%g latch=%d\n",p.q,p.E,p.wt,p.latch);
                        egsInformation("  current case = %lld\n",current_case);
                        egsFatal("quitting now\n");
                    }
                    */
                    dose->score(ig,aux);
                    if(check_for_subreg && nsubgeoms[ig] != 0){
                        save_dose += aux;	//get the current dose deposition
                    }
                }
            }
	    }
	    else{
		    // we use the onegeom option which means we have identical geometries
			// except the region numbers differ
            // we simulate only the first geometry so see if the actual region with
            // energy-deposition belongs to any geometry
            if( ir >= 0 )
                for(int i=0;i<ngeom;i++){
                    if( is_cavity[i][ir] ) {
                        EGS_Float aux = the_epcont->edep*the_stack->wt[np];
                        if( aux > 0 ) {
                            dose->score(i,aux);
                        }
                    }
                }
		}

        return 0;
    }


    //
    //  **** mark or throw away scattered photons
    //
    if( iarg == AfterBrems     || iarg == AfterMoller ||
        iarg == AfterBhabha    || iarg == AfterAnnihFlight ||
        iarg == AfterAnnihRest ) {
        for(int ip=the_stack->npold-1; ip<=np; ip++) {
            if( !the_stack->iq[ip] ) {
                if( fsplit > 1 && the_extra_stack->nbr_splitting[ip] < 2 ) {
                    if( rndm->getUniform()*fsplit < 1 )
                        the_stack->wt[ip] *= fsplit;
                    else
                        the_stack->wt[ip] = 0;
                }
                if( !do_cse && (the_extra_stack->nbr_splitting[ip] == 0 ||
                            (rr_flag > 1 && the_extra_stack->nbr_splitting[ip] == rr_flag) ) ){
                    the_extra_stack->nbr_splitting[ip] += 1;
                }
            }
        }
    }
    return 0;
};
int EGS_ChamberApplication::runSimulation() {
    bool ok = true;
    if( !geometry ) {
        egsWarning("%s no geometry\n",__egs_app_msg_my3); ok = false;
    }
    if( !source ) {
        egsWarning("%s no source\n",__egs_app_msg_my3); ok = false;
    }
    if( !rndm ) {
        egsWarning("%s no RNG\n",__egs_app_msg_my3); ok = false;
    }
    if( !run ) {
        egsWarning("%s no run control object\n",__egs_app_msg_my3); ok = false;
    }
    if( !ok ) return 1;

    int start_status = run->startSimulation();
    if( start_status ) {
        if( start_status < 0 )
            egsWarning("\n%s failed to start the simulation\n\n",__egs_app_msg_my3);
        return start_status;
    }

    bool next_chunk = true;

    while( next_chunk && (ncase = run->getNextChunk()) > 0 ) {

        egsInformation("\nRunning %lld histories\n",ncase);
        double f,df;
        if( run->getCombinedResult(f,df) ) {
            char c = '%';
            egsInformation("    combined result from this and other parallel"
                    " runs: %lg +/- %7.3lf%c\n\n",f,df,c);
        }
        else egsInformation("\n");
        int nbatch = run->getNbatch();
        EGS_I64 ncase_per_batch = ncase/nbatch;
        if( !ncase_per_batch ) {
            ncase_per_batch = 1; nbatch = ncase;
        }
        for(int ibatch=0; ibatch<nbatch; ibatch++) {
            if( !run->startBatch(ibatch,ncase_per_batch) ) {
                egsInformation("  startBatch() loop termination\n");
                next_chunk = false; break;
            }
            for(EGS_I64 icase=0; icase<ncase_per_batch; icase++) {
                if( simulateSingleShower() ) {
                    egsInformation("  simulateSingleShower() "
                            "loop termination\n");
                    next_chunk = false; break;
                }
            }
            if( !next_chunk ) break;
            if( !run->finishBatch() ) {
                egsInformation("  finishBatch() loop termination\n");
                next_chunk = false; break;
            }
        }
    }
    // call this from within finishSimulation()
    //run->finishSimulation();
    return 0;
}


/* If one wants to reimplementinteractive outputs...
bool EGS_RunControl::finishBatch() {
    cpu_time = timer.time();
    int out = app->outputData();
    if( out ) egsWarning("\n\noutputData() returned error code %d ?\n",out);
    double sum, sum2, norm, count;
    app->getCurrentResult(sum,sum2,norm,count);
    double f, df;
    if( sum > 0 && sum2 > 0 && norm > 0 && count > 1 ) {
        f = sum*norm/count;
        /initdf = count*sum2/(sum*sum)-1;
        if( df > 0 ) df = 100*sqrt(df/(count-1)); else df = 100;
    } else { f = 0; df = 100; }
    egsInformation("xxx        %12.2f %14g %14.2f\n",cpu_time,f,df);
    if( df < 100 && accu > 0 && df < accu ) {
        char c = '%';
        egsWarning("\n\n*** Reached the requested uncertainty of %g%c\n"
                       "    => terminating simulation.\n\n",accu,c);
        return false;
    }
    return true;
}
*/


/*! Simulate a single shower.
    We need to do special things and therefore reimplement this method.
 */
int EGS_ChamberApplication::simulateSingleShower() {

    // for the onegeom option we need only one actual simulation geometry
    int stop_geom;
    if(!onegeom)
        stop_geom = ngeom;
    else
        stop_geom = 1;

    last_case = current_case;
    EGS_Vector x,u;
    the_egsvr->nbr_split = csplit;
    current_case = source->getNextParticle(rndm,p.q,p.latch,p.E,p.wt,x,u);
    //egsInformation("Got particle: q=%d E=%g wt=%g latch=%d x=(%g,%g,%g) u=(%g,%g,%g)\n",p.q,p.E,p.wt,p.latch,x.x,x.y,x.z,u.x,u.y,u.z);
    the_extra_stack->nbr_splitting[0] = 0;
    int err = startNewShower(); if( err ) return err;
    //*HB_start************************
    //isocenter positioning uncertainty
    int jpu = 0;
    if(iso_pu_flag) {
        if(iso_pu_do_shift) {
            pu_distributor[jpu]->setNewShifts(rndm);
            iso_pu_do_shift = false;
        }
        EGS_Vector tmp1 = pu_distributor[jpu]->getRotation();
        EGS_RotationMatrix Rtmp = EGS_RotationMatrix(-tmp1.x,-tmp1.y,-tmp1.z);
        //egsInformation("(%f,%f,%f)-->",x.x,x.y,x.z);
        x = Rtmp*x;
        u = Rtmp*u;
        //egsInformation("(%f,%f,%f)\n",x.x,x.y,x.z);
        EGS_Vector tmp2 = pu_distributor[jpu]->getTranslation();
        x.x -= tmp2.x;
        x.y -= tmp2.y;
        x.z -= tmp2.z;
        jpu++;
        //egsInformation("Got rotation (%f,%f,%f) and translation (%f%f,%f)\n",tmp1.x,tmp1.y,tmp1.z,tmp2.x,tmp2.y,tmp2.z);
    }
/*
    //cavity positioning uncertainty (implem here to avoid delay in shift if particle do not reach TmpPhsp)
    if(cav_pu_do_shift) {
        for(int j=1;j<ngeom; j++) {
            if(j==1) pu_distributor[jpu]->setNewShifts(rndm);
            EGS_Vector tmp1 = pu_distributor[jpu]->getTranslation();
            EGS_Vector tmp2 = pu_distributor[jpu]->getRotation();
            EGS_RotationMatrix Rtmp = EGS_RotationMatrix(tmp2.x,tmp2.y,tmp2.z);
            if( !transforms[j] ) transforms[j] = new EGS_AffineTransform();
            (*transforms[j]) = EGS_AffineTransform(Rtmp,tmp1);
        }
        cav_pu_do_shift = false;
    }
*/
    //*HB_end**************************
    EGS_BaseGeometry *save_geometry = geometry;
    if(!do_TmpPhsp) {
        for(ig=0; ig<stop_geom; ig++) {
            geometry = geoms[ig]; p.x = x; p.u = u;
            if( transforms[ig] ) {
                transforms[ig]->transform(p.x); transforms[ig]->rotate(p.u);
            }
            int ireg = geometry->isWhere(p.x);
            if( ireg < 0 ) {
                EGS_Float t = 1e30; ireg = geometry->howfar(ireg,p.x,p.u,t);
                if( ireg >= 0 ) p.x += p.u*t;
            }
            if( ireg >= 0 ) {
                p.ir = ireg;
                nsmall_step = 0;
                err = shower(); if( err ) return err;
            }
        }
    }
    else {
        // start with the first geometry
        for(ig=0; ig<ngeom; ig++){
            if(ig == 0){
                container->clean();
                geometry = geoms[ig]; p.x = x; p.u = u;
                if( transforms[ig] ) {
                    transforms[ig]->transform(p.x); transforms[ig]->rotate(p.u);
                }
                int ireg = geometry->isWhere(p.x);
                if( ireg < 0 ) {
                    EGS_Float t = 1e30; ireg = geometry->howfar(ireg,p.x,p.u,t);
                    if( ireg >= 0 ) p.x += p.u*t;
                }
                if( ireg >= 0 ) {
                    p.ir = ireg;
                    nsmall_step = 0;
                    setAusgabCall(AfterTransport, true);
                    if( is_cavity[ig][ireg] ) { // a particle already inside the phase space scoring geometry
                        p.E = (p.q) ? p.E + the_useful->rm : p.E;   // JW 2011
                        container->set(p);
                        basereg = ireg;
                        continue;
                    }
                    // during shower the phasespace will be scored
                    // at the 'base' geometry (i.e. the first defined)
                    err = shower(); if( err ) return err;
                }
                /*
                else {
                    if( container->size() > 0 )
                        egsInformation("Hah: %d x=(%g,%g,%g) u=(%g,%g,%g)\n",container->size(),
                                p.x.x,p.x.y,p.x.z,p.u.x,p.u.y,p.u.z);
                }
                */
            }
            // when particles were scored reuse them for all other geometries
            else if(container->size() > 0){
                if( (nsubgeoms[ig] != 0) || !do_sub ){
                    // i.e. has subgeometries or no subgeom option
                    geometry = geoms[ig];
                    int pc;
                    int tmppc = container->size(); // save the number of particles in container
                    if(do_sub)container2->clean();
                    if(has_sub[ig])check_for_subreg = true;
                    save_dose = 0;
                    // get tmpPhsp1 and use as a source
                    while(container->size() > 0){
                        int tmppc2 = container->size();
                        //recycle particles
                        for( int i=0; i< do_TmpPhsp; i++){
                            container->setPointer(tmppc2);
                            nsmall_step = 0;
                            the_stack->np = 1;
                            container->get();
                            EGS_Vector xt(the_stack->x[0], the_stack->y[0], the_stack->z[0]);
                            if( transforms[ig] ) {
                                transforms[ig]->transform(xt);
                                EGS_Vector ut(the_stack->u[0], the_stack->v[0], the_stack->w[0]);
                                transforms[ig]->rotate(ut);
                                the_stack->u[0] = ut.x; the_stack->v[0] = ut.y; the_stack->w[0] = ut.z;
                                the_stack->x[0] = xt.x; the_stack->y[0] = xt.y; the_stack->z[0] = xt.z;
                            }
                            //HB Nov. 2009: Could implement motion of paricle here for positioning uncertainty
                            //*HB_start************************
                            //cavity positioning uncertainty (implem here to avoid delay in shift if particle do not reach TmpPhsp)
                            if(cav_pu_flag) {
                                if(cav_pu_do_shift) {
                                    pu_distributor[jpu]->setNewShifts(rndm);
                                    cav_pu_do_shift = false;
                                }
                                EGS_Vector tmp1 = pu_distributor[jpu]->getRotation();
                                EGS_RotationMatrix Rtmp = EGS_RotationMatrix(-tmp1.x,-tmp1.y,-tmp1.z);
                                EGS_Vector ut(the_stack->u[0], the_stack->v[0], the_stack->w[0]);
                                //EGS_Vector xt(the_stack->x[0], the_stack->y[0], the_stack->z[0]);
                                xt = Rtmp*xt;
                                ut = Rtmp*ut;
                                EGS_Vector tmp2 = pu_distributor[jpu]->getTranslation();
                                xt.x -= tmp2.x;
                                xt.y -= tmp2.y;
                                xt.z -= tmp2.z;
                                the_stack->u[0] = ut.x; the_stack->v[0] = ut.y; the_stack->w[0] = ut.z;
                                the_stack->x[0] = xt.x; the_stack->y[0] = xt.y; the_stack->z[0] = xt.z;
                            }
                            //*HB_end**************************

                            the_stack->ir[0] = geometry->isWhere(xt) + 2;
                            if( the_stack->ir[0] < 2 ) continue;
                            the_stack->wt[0] /= (EGS_Float)do_TmpPhsp;// adjust weight due to splitting
                            the_extra_stack->nbr_splitting[0] /= do_TmpPhsp; // nbr_split is only set for fat electrons ph
                            the_stack->dnear[0] = 0;
                            // adjust the number/weight of electrons
                            // I assume that the cse_enhance in the cavity of the TmpPhsp object
                            // is related to the actual weight of the electron
                            // when I start in the geometry I compare with the new and old cse
                            if( the_stack->iq[0] ){
                                int ir = the_stack->ir[0]-2;
                                if( cs_enhance[0][basereg] < cs_enhance[ig][ir] ){
                                    // split since CSE increased in this reg
                                    int n_esplit = cs_enhance[ig][ir]/cs_enhance[0][basereg];
                                    the_stack->wt[0] = the_stack->wt[0]/(EGS_Float)n_esplit;
                                    for(int i=0; i<n_esplit; i++){
                                        container3->set();
                                    }
                                    while(container3->size() > 0){
                                        nsmall_step = 0;
                                        the_stack->dnear[0] = 0;
                                        the_stack->np = 1;
                                        container3->get();
                                        EGS_Vector xt2(the_stack->x[0], the_stack->y[0], the_stack->z[0]);
                                        the_stack->ir[0] = geometry->isWhere(xt2) + 2;
                                        egsShower();
                                    }
                                }
                                //play RR since cse is decreased in this reg
                                else if( ir >= 0 && cs_enhance[0][basereg] > cs_enhance[ig][ir] ){
                                    int RRprob = cs_enhance[0][basereg]/cs_enhance[ig][ir];
                                    if( rndm->getUniform()*RRprob < 1 ){
                                        the_stack->wt[0] *= RRprob;
                                        egsShower();
                                    }
                                }
                                else{
                                    // CSE stayed the same
                                    egsShower(); // shortcut to mortran-backend
                                }
                            }
                            else{
                                egsShower();
                            }
                        }
                    }
                    container->setPointer(tmppc);	// for next geometry
                    if(do_sub){				// when subgeometries are defined

                        if(container2->size() > 0 || save_dose > 0){
                            if(container2->size() > 0){
                                saveRNGState();
                                pc = container2->size();
                            }else pc = 0;
                            check_for_subreg = false;
                            for(int j=0; j<nsubgeoms[ig]; j++){	// use phasespace for each subgeometry
                                for(int sub_ig = 1; sub_ig < ngeom; sub_ig++){
                                    if(geoms[sub_ig]->getName().c_str() == subgeoms[ig][j]){
                                        container2->setPointer(pc);
                                        int save_ig = ig;
                                        ig = sub_ig;
                                        // copy dose results of first geometry since
                                        // dose may have been deposited by now
                                        if ( (ig != save_ig) && (save_dose > 0) )
                                            dose->score(ig,save_dose);
                                        geometry = geoms[ig];
                                        while(container2->size() > 0){
                                            nsmall_step = 0;
                                            the_stack->np = 1;
                                            container2->get();
                                            EGS_Vector xt2(the_stack->x[0], the_stack->y[0], the_stack->z[0]);
                                            the_stack->ir[0] = geometry->isWhere(xt2) + 2;
                                            the_stack->dnear[0] = 0;
                                            resetRNGState();
                                            egsShower();
                                        }
                                        ig = save_ig;
                                    }
                                }
                            }
                        }
                    }
                    check_for_subreg = true;
                }
            }
        }
    }
    err = finishShower();
    geometry = save_geometry;
    return err;
};


/*! Output intermediate results to the .egsdat file. */
int EGS_ChamberApplication::outputData() {
    int err = EGS_AdvancedApplication::outputData();
    if( err ) return err;
    if( !dose->storeState(*data_out) ) return 101;
    if( ncg > 0 ) {
        for(int j=0; j<ncg; j++) {
            double aux = dose->thisHistoryScore(gind1[j])*
                         dose->thisHistoryScore(gind2[j]);
            (*data_out) << scg[j]+aux << "  ";
        }
        (*data_out) << endl;
        if( !data_out->good() ) return 104;
    }
    //*HB_start************************
    if(pu_flag) {
        bool flgfirst = true;
        for(int j=1; j<ngeom; j++) {
            EGS_Float v1,v2;
            EGS_I64 k;
            int errpu = pu_estimator[j-1]->getScore(k,v1,v2);
            if(errpu) egsWarning("Error %d in outputData() for k = %d, v1 = %e and v2 = %e\n",errpu,k,v1,v2);
            if(flgfirst) flgfirst = false;
            else (*data_out) << "\n" ;
            (*data_out) << k << " " << v1 << " " << v2 ;
        }
        (*data_out) << endl;
        flgfirst = true;
        for(int j=0; j<ncg; j++) {
            EGS_Float v1,v2;
            EGS_I64 k;
            int errpu = pu_estimator_corr[j]->getScore(k,v1,v2);
            if(errpu) egsWarning("Error %d in outputData() for k = %d, v1 = %e and v2 = %e\n",errpu,k,v1,v2);
            if(flgfirst) flgfirst = false;
            else (*data_out) << "\n" ;
            (*data_out) << k << " " << v1 << " " << v2 ;
        }
        (*data_out) << endl;
        if( !data_out->good() ) return 104;
    }
    //*HB_end**************************
    data_out->flush();
    delete data_out; data_out = 0;
    return 0;
};

/*! Read results from a .egsdat file. */
int EGS_ChamberApplication::readData() {
    int err = EGS_AdvancedApplication::readData();
    if( err ) return err;
    if( !dose->setState(*data_in) ) return 101;
    if( ncg > 0 ) {
        for(int j=0; j<ncg; j++) (*data_in) >> scg[j];
        if( !data_in->good() ) return 104;
    }
    //*HB_start************************
    if(pu_flag) {
        for(int j=1; j<ngeom; j++) {
            EGS_Float v1,v2;
            EGS_I64 k;
            (*data_in) >> k >> v1 >> v2;
            int errpu = pu_estimator[j-1]->setScore(k,v1,v2);
            if(errpu) egsWarning("Error %d in readData() for k = %d, v1 = %e and v2 = %e\n",errpu,k,v1,v2);
        }
        for(int j=0; j<ncg; j++) {
            EGS_Float v1,v2;
            EGS_I64 k;
            (*data_in) >> k >> v1 >> v2;
            int errpu = pu_estimator_corr[j]->setScore(k,v1,v2);
            if(errpu) egsWarning("Error %d in readData() for k = %d, v1 = %e and v2 = %e\n",errpu,k,v1,v2);
        }
        if( !data_in->good() ) return 104;
    }
    //*HB_end**************************
    return 0;
};

/*! Reset the variables used for accumulating results */
void EGS_ChamberApplication::resetCounter() {
    EGS_AdvancedApplication::resetCounter();
    dose->reset();
    if( ncg > 0 ) {
        for(int j=0; j<ncg; j++) scg[j] = 0;
    }
    //*HB_start************************
    if(pu_flag) {
        for(int j=1; j<ngeom; j++) {
            int errpu = pu_estimator[j-1]->setScore(0,0,0);
            if(errpu) egsWarning("Error %d in resetCounter()\n",errpu);
            errpu += pu_estimator[j-1]->resetSample();
        }
        for(int j=0; j<ncg; j++) {
            int errpu = pu_estimator_corr[j]->setScore(0,0,0);
            if(errpu) egsWarning("Error %d in resetCounter()\n",errpu);
            errpu += pu_estimator_corr[j]->resetSample();
        }
    }
    //*HB_end**************************
};

/*! Add simulation results */
int EGS_ChamberApplication::addState(istream &data) {
    int err = EGS_AdvancedApplication::addState(data);
    if( err ) return err;
    EGS_ScoringArray tmp(ngeom);
    if( !tmp.setState(data) ) return 101;
    (*dose) += tmp;
    if( ncg > 0 ) {
        for(int j=0; j<ncg; j++) {
            double tmp; data >> tmp;
            if( !data.good() ) return 104;
            scg[j] += tmp;
        }
    }
    //*HB_start************************
    if(pu_flag) {
        for(int j=1; j<ngeom; j++) {
            EGS_I64 rk;
            EGS_Float rv1,rv2;
            data >> rk >> rv1 >> rv2;
            if( !data.good() ) return 104;
            EGS_I64 k;
            EGS_Float v1,v2;
            int errpu = pu_estimator[j-1]->getScore(k,v1,v2);
            if(errpu) egsWarning("Error %d in addState() for k = %d, v1 = %e and v2 = %e\n",errpu,k,v1,v2);
            k += rk;
            v1 += rv1;
            v2 += rv2;
            errpu += pu_estimator[j-1]->setScore(k,v1,v2);
            errpu += pu_estimator[j-1]->resetSample();
        }
        for(int j=0; j<ncg; j++) {
            EGS_I64 rk;
            EGS_Float rv1,rv2;
            data >> rk >> rv1 >> rv2;
            if( !data.good() ) return 104;
            EGS_I64 k;
            EGS_Float v1,v2;
            int errpu = pu_estimator_corr[j]->getScore(k,v1,v2);
            if(errpu) egsWarning("Error %d in addState() for k = %d, v1 = %e and v2 = %e\n",errpu,k,v1,v2);
            k += rk;
            v1 += rv1;
            v2 += rv2;
            errpu += pu_estimator_corr[j]->setScore(k,v1,v2);
            errpu += pu_estimator_corr[j]->resetSample();
        }
    }
    //*HB_end**************************
    return 0;
};

/*! Output the results of a simulation. */
void EGS_ChamberApplication::outputResults() {
    egsInformation("\n\n last case = %lld fluence = %g\n\n",
            current_case,source->getFluence());
    //*HB_start************************
    if(pu_flag)
        egsInformation("%-25s       Cavity dose      \tPositioning uncertainty","Geometry");
    else
        egsInformation("%-25s       Cavity dose      ","Geometry");
    //*HB_end**************************

    egsInformation("\n"
                    "-----------------------------------------------\n");
    char c = '%';
    for(int j=0; j<ngeom; j++) {
        double r,dr; dose->currentResult(j,r,dr);
        if( r > 0 ) dr = 100*dr/r; else dr = 100;
        EGS_Float norm = 1.602e-10*current_case/source->getFluence();
        norm /= mass[j];
	//*HB_start************************
        if(pu_flag&&j) {
            EGS_Float dr1,ddr1,dvr1;
            EGS_I64 m = pu_estimator[j-1]->returnMcasePerPos();
            EGS_I64 n = pu_estimator[j-1]->returnNposPerSample();
            EGS_I64 k = pu_estimator[j-1]->returnK();
            int errpu = pu_estimator[j-1]->getResult(dr1,ddr1,dvr1);
            EGS_Float eff = 1/dvr1/dvr1/(n*m*k);
            if(errpu)
                    egsWarning("\nError %d in getResult() of pu option for geometry index %d. (K,N,M) = (%d,%d,%d)\n", \
                                errpu,j,pu_estimator[j-1]->returnK(), \
                                pu_estimator[j-1]->returnN(),pu_estimator[j-1]->returnM());
            if( r > 0 ) {dr1 = 100*dr1/r ; ddr1 = 100*ddr1/r;}
            else {dr1 = 100; ddr1 = 100;}
            if (dr1 > 0) {
                egsInformation("%-25s %10.4le +/- %-7.3lf%c \t+/- (%-7.3lf +/- %-7.3lf)%c (efficiency indicator: %10.4le)",
                    geoms[j]->getName().c_str(),
                    r*norm,dr,c,dr1,ddr1,c,eff);
            }
            else {
                ddr1 = r*ddr1/100.0;
                egsInformation("%-25s %10.4le +/- %-7.3lf%c \t+/- Negative estimator! (efficiency indicator: %10.4le)",
                    geoms[j]->getName().c_str(),
                    r*norm,dr,c,eff);

            }
        }
        else {
            egsInformation("%-25s %10.4le +/- %-7.3lf%c ",
                    geoms[j]->getName().c_str(),
                    r*norm,dr,c);
        }
        //*HB_end**************************
        if(do_TmpPhsp && j==0)egsInformation("(this MUST be zero!)");
        egsInformation("\n");
    }
    egsInformation("\n\n");
    if( ncg > 0 ) {
	//*HB_start************************
        if(pu_flag)
            egsInformation("%-20s %-20s %-20s     Dose ratio\t\t\tPositioning uncertainty\n","Geometry 1",
                "Geometry 2","Identifier");
        else
            egsInformation("%-20s %-20s %-20s     Dose ratio\n","Geometry 1",
            "Geometry 2","Identifier");
        //*HB_end**************************
        vector<double> ratio, dratio;
        for(int j=0; j<ncg; j++) {
            double r1,dr1,r2,dr2;
            dose->currentResult(gind1[j],r1,dr1);
            dose->currentResult(gind2[j],r2,dr2);
            if( r1 > 0 && r2 > 0 ) {
                double rc=(scg[j]/(r1*r2*current_case)-1)/(current_case-1);
                dr1 /= r1; dr2 /= r2;
                double dr = dr1*dr1 + dr2*dr2 - 2*rc;
                if( dr > 0 ) dr = sqrt(dr);
                double r = r1*mass[gind2[j]]/(r2*mass[gind1[j]]);
                //*HB_start************************
                if (pu_flag) {
                    double tmp1, tmp2, tmp3;
                    int errpu = pu_estimator_corr[j]->getResult(tmp1,tmp2,tmp3);
                    if(tmp1>0)
                        egsInformation("%-20s %-20s %-20s     %-8.5lf +/- %-7.5lf \t+/- (%-7.5lf +/- %-7.5lf)\n",
                               geoms[gind1[j]]->getName().c_str(),
                               geoms[gind2[j]]->getName().c_str(),correlgnames[j].c_str(),r,r*dr,tmp1*r,tmp2*r);
                    else
                        egsInformation("%-20s %-20s %-20s     %-8.5lf +/- %-7.5lf \t+/- Negative estimator!\n",
                               geoms[gind1[j]]->getName().c_str(),
                               geoms[gind2[j]]->getName().c_str(),correlgnames[j].c_str(),r,r*dr);
                }
                else
                    egsInformation("%-20s %-20s %-20s     %-8.5lf +/- %-7.5lf\n",
                               geoms[gind1[j]]->getName().c_str(),
                               geoms[gind2[j]]->getName().c_str(),correlgnames[j].c_str(),r,r*dr);
                //*HB_end**************************
                ratio.push_back(r); dratio.push_back(r*dr);
            }
            else egsInformation("zero dose\n");
        }

    }
};

/*! Get the current simulation result.  */
void EGS_ChamberApplication::getCurrentResult(double &sum, double &sum2, double &norm,
        double &count) {
    count = current_case; double flu = source->getFluence();
    int geom_count = do_TmpPhsp ? 1 : 0;
    norm = flu > 0 ? 1.602e-10*count/(flu*mass[geom_count]) : 0;
    dose->currentScore(geom_count,sum,sum2);
};

/*! simulate a shower */
int EGS_ChamberApplication::shower() {
    return EGS_AdvancedApplication::shower();
};

/* Select photon mean-free-path */
void EGS_ChamberApplication::selectPhotonMFP(EGS_Float &dpmfp) {
    int np = the_stack->np-1;
    if( fsplit <= 1 ) {
        dpmfp = -log(1 - rndm->getUniform()); return;
    }
    if( the_stack->iq[np] ) egsFatal("selectPhotonMFP called with a"
        " particle of charge %d\n",the_stack->iq[np]);
    EGS_Float wt_o = the_stack->wt[np];
    EGS_Float E = the_stack->E[np];
    int ireg   = the_stack->ir[np]-2, nbr_split = the_extra_stack->nbr_splitting[np];
    int nbr_split1 = nbr_split;
    int latch1 = the_stack->latch[np];
    EGS_Float f_split, f_spliti;
    if( nbr_split < 2 ) { f_split = fsplit; f_spliti = fspliti; }
    else {
        f_split = rr_flag; f_spliti = 1/f_split;
        nbr_split1 = nbr_split - rr_flag; the_extra_stack->nbr_splitting[np] = nbr_split1;
    }
    the_stack->wt[np] = wt_o*f_spliti;
    int imed = geometry->medium(ireg);
    EGS_Float gmfpr=1e15, cohfac=1;
    EGS_Float gle = the_epcont->gle;
    if( imed >= 0 ) {
        gmfpr = i_gmfp[imed].interpolateFast(gle);
        if( the_xoptions->iraylr ) {
            cohfac = i_cohe[imed].interpolateFast(gle);
            gmfpr *= cohfac;
        }
    }
    EGS_Vector x(the_stack->x[np],the_stack->y[np],the_stack->z[np]);
    EGS_Vector u(the_stack->u[np],the_stack->v[np],the_stack->w[np]);
    EGS_Float rhor = the_useful->rhor;
    EGS_Float gmfp = gmfpr/rhor;

    int i_survive = (int) ( f_split*rndm->getUniform() );
    EGS_Float mfp_old = 0,
              eta_prime = 1 + rndm->getUniform()*f_spliti;
    dpmfp = -1; int isplit = 0;
    while(1) {
        eta_prime -= f_spliti;
        if( eta_prime <= 0 ) { --the_stack->np; return; }
        EGS_Float mfp = -log(eta_prime) - mfp_old;
        EGS_Float xp , up, t;
        double ttot = 0;
        mfp_old = mfp_old + mfp;
        while(1) {
            EGS_Float tstep = mfp*gmfp; int newmed;
            int inew = geometry->howfar(ireg,x,u,tstep,&newmed);
            if( inew < 0 ) { --the_stack->np; return; }
            x += u*tstep;
            if( inew == ireg ) break;
            mfp -= tstep/gmfp;
            if( geometry->hasRhoScaling() )
                rhor = geometry->getRelativeRho(inew);
            else rhor = 1;
            ireg = inew;
            if( newmed != imed ) {
                imed = newmed; the_useful->medium = imed+1;
                if( imed >= 0 ) {
                    gmfpr = i_gmfp[imed].interpolateFast(gle);
                    if( the_xoptions->iraylr ) {
                        cohfac = i_cohe[imed].interpolateFast(gle);
                        gmfpr *= cohfac;
                    }
                }
                else { gmfpr=1e15, cohfac=1; }
            }
            gmfp = gmfpr/rhor;
        }
        the_stack->x[np]=x.x; the_stack->y[np]=x.y; the_stack->z[np]=x.z;
        the_stack->ir[np] = ireg+2;
        the_stack->dnear[np] = 0;
        bool is_rayleigh = false;
        if( the_xoptions->iraylr ) {
            if( rndm->getUniform() < 1 - cohfac ) { // ******** rayleigh
                is_rayleigh = true;
                if( isplit != i_survive ) { --np; --the_stack->np; }
                else {
                 the_stack->wt[np] = wt_o;
                 doRayleigh();
                 the_extra_stack->nbr_splitting[np] = nbr_split < 2 ?
                        1 : (rr_flag+1);
                }
            }
        }
        if( !is_rayleigh ) {
            EGS_Float gbr1, gbr2;
            gbr1 = i_gbr1[imed].interpolateFast(gle);
            gbr2 = i_gbr2[imed].interpolateFast(gle);
            EGS_Float eta = rndm->getUniform();
            if( E > the_thresh->rmt2 && eta < gbr1 ) { // ********* pair
                F77_OBJ(pair,PAIR)();
            }
            else if( eta < gbr2 ) {                    // ********* compton
                F77_OBJ(compt,COMPT)();
            }
            else {                                     // ********* photo
                F77_OBJ(photo,PHOTO)();
            }
            np = the_stack->np-1; int ip = the_stack->npold-1;
            int ipo = ip, npo = np;
            bool do_rr=(rr_flag>0 && !is_cavity[ig][ireg]);
            EGS_Float cperp=1e30;
            if( do_rr && cgeoms[ig] ) {
                if( !cgeoms[ig]->isInside(x) ) cperp = cgeoms[ig]->hownear(-1,x);
                else do_rr = false;
            } else do_rr = false;

            do {
                if( !the_stack->iq[ip] ) {
                    if( isplit == i_survive ) {
                      the_stack->wt[ip] = wt_o;
                      the_extra_stack->nbr_splitting[ip++] = nbr_split < 2 ?
                            1 : (rr_flag+1);
                    }
                    else {
                    if( ip < np ) {
                            the_stack->E[ip] = the_stack->E[np];
                            the_stack->iq[ip] = the_stack->iq[np];
                            the_stack->latch[ip] = the_stack->latch[np];
                            the_stack->u[ip] = the_stack->u[np];
                            the_stack->v[ip] = the_stack->v[np];
                            the_stack->w[ip] = the_stack->w[np];
                            the_extra_stack->nbr_splitting[ip] = the_extra_stack->nbr_splitting[np];
                    }
                    --np; --the_stack->np;
                    }
                }
                else {
                    bool keep = true;
                    if( do_rr ) {
                        EGS_Float crange = 0;
                        EGS_Float e = the_stack->E[ip]-the_useful->rm;
                        if( e > 0 ) {
                            EGS_Float elke=log(e);
                            crange = the_stack->iq[ip] == -1 ?
                                rr_erange.interpolate(elke) :
                                rr_prange.interpolate(elke);
                        }
                        if( crange < cperp ) {
                            if( rr_flag == 1 ) keep = false;
                            else {
                                if( rndm->getUniform()*rr_flag < 1 ) {
                                    the_stack->wt[ip] *= rr_flag;
                                    the_extra_stack->nbr_splitting[ip] += rr_flag;
                                } else keep = false;
                            }
                        }
                    }
                    if( keep ) ++ip;
                    else {
                        if( ip < np ) {
                            the_stack->E[ip] = the_stack->E[np];
                            the_stack->iq[ip] = the_stack->iq[np];
                            the_stack->latch[ip] = the_stack->latch[np];
                            the_stack->u[ip] = the_stack->u[np];
                            the_stack->v[ip] = the_stack->v[np];
                            the_stack->w[ip] = the_stack->w[np];
                            the_extra_stack->nbr_splitting[ip] = the_extra_stack->nbr_splitting[np];
                        }
                        --np; --the_stack->np;
                    }
                }
            } while (ip <= np);
        }
        ++isplit;
        ++np; ++the_stack->np;
        the_stack->E[np] = E; the_stack->wt[np] = wt_o*f_spliti;
        the_stack->iq[np] = 0;
        the_stack->latch[np] = latch1;
        the_extra_stack->nbr_splitting[np] = nbr_split1;
        the_stack->ir[np] = ireg+2;
        the_stack->u[np]=u.x; the_stack->v[np]=u.y; the_stack->w[np]=u.z;
        the_stack->x[np]=x.x; the_stack->y[np]=x.y; the_stack->z[np]=x.z;
        the_stack->dnear[np] = 0;
    }
};

int EGS_ChamberApplication::rangeDiscard(EGS_Float tperp, EGS_Float range) const {
    // we can be sure that when this function is called
    // range rejection/RR is on.
    //
    // If rr_flag = 1 & E<Esave, we immediately discard the particle if it
    // can not reach the cavity or escape the current region
    // If rr_flag > 1, we play RR with the particle with survival
    // probability of 1/rr_flag, if it can not reach the cavity or
    // discard it if it is in the cavity and can not escape and E<Esave
    // However, we only play RR if that was not done before.
    // This is indicated by the value of nbr_split:
    //   nbr_split=0,1 indicates a primary/secondary electron that has
    //             not been previosly subjected to RR.
    //   nbr_split=x,x+1 (with x>1) indicates a primary/secondary electron
    //             that has already been range-RR'ed.
    //
    int np = the_stack->np-1;
    if( abs(the_extra_stack->nbr_splitting[np]) > 1 ) return 0;
    bool is_cav = is_cavity[ig][the_stack->ir[np]-2];

    // if transport is done only in one geometry
    // check if current region is cavity in the others ones
    if(onegeom &! is_cav){
            int gcount = 0;
            is_cav = false;
            while(gcount < ngeom){
                if(is_cavity[gcount][the_stack->ir[np]-2]){
                    is_cav = true;
                    break;
                }
                gcount++;
            }
    }
    // remember to set a huge cavity geometry which encompasses all
	// cavity geometries, so that the range calc is valid... (below)


    if( (rr_flag == 1 || is_cav) && the_stack->E[np] > Esave ) return 0;
      // i.e., if rr_flag is 1 or rr_flag > 1 but we are in the cavity and
      // the energy is greater than Esave, don't discard the particle
    int retval = the_stack->iq[np] == -1 ? 1 : 99;
      // if here: rr_flag = 1 && E < Esave
      //  or      rr_flag > 1 && (in cavity but E<Esave) || not in cavity
    bool do_RR = false;
    if( range < tperp ) { // can not escape current region
        if( rr_flag == 1 || is_cav ) return retval;
        do_RR = true;
    }
    else { // can escape current region
        if( is_cav || !cgeoms[ig] ) return 0;
        EGS_Float rho = the_media->rho[the_useful->medium-1];
        if( rho < 0.95*rho_rr ) return 0;
        EGS_Vector x(the_stack->x[np],the_stack->y[np],the_stack->z[np]);
        int ireg = cgeoms[ig]->isWhere(x);
        if( ireg < 0 ) {
            EGS_Float cperp = cgeoms[ig]->hownear(ireg,x);
            EGS_Float crange = the_stack->iq[np] == -1 ?
                rr_erange.interpolateFast(the_epcont->elke) :
                rr_prange.interpolateFast(the_epcont->elke);
            //egsInformation("E=%g elke=%g crange=%g x=(%g,%g,%g) cperp=%g\n",
            //        the_stack->E[np],the_epcont->elke,crange,
            //        x.x,x.y,x.z,cperp);
            if( crange < cperp ) {
                if( rr_flag == 1 ) return retval;
                do_RR = true;
            }
        }
    }
    if( !do_RR ) return 0;
    if( rndm->getUniform()*rr_flag < 1 ) {
        // particle survives.
        the_stack->wt[np] *= rr_flag;
        // mark where this electron as RR and if it is already low weight due to cse
        the_extra_stack->nbr_splitting[np] = rr_flag/cs_enhance[ig][the_stack->ir[np]-2];
        return 0;
    }
    //egsInformation("Killing particle: E=%g x=(%g,%g,%g) tperp=%g"
    //      " g=%s\n",the_stack->E[np],the_stack->x[np],the_stack->y[np],
    //      the_stack->z[np],tperp,geometry->getName().c_str());
    return -1; // i.e. particle is killed and must be discarded immediately.
};


/*! Start a new shower.  */
int EGS_ChamberApplication::startNewShower() {
  int res = EGS_Application::startNewShower();
  if( res ) return res;
  if( current_case != last_case ) {
      if( ncg > 0 ) {
          for(int j=0; j<ncg; j++)
              scg[j] += dose->thisHistoryScore(gind1[j])*
                        dose->thisHistoryScore(gind2[j]);
      }
      //*HB_start************************
      if(pu_flag) {
            for (int j = 1; j<ngeom ; j++) {
                int errpu = pu_estimator[j-1]->scoreHist(last_case,dose->thisHistoryScore(j));
                bool instrct = pu_estimator[j-1]->shiftManager(current_case);
                //this will be redundant for j>1
                if(iso_pu_flag)
                    if(!iso_pu_do_shift)
                        iso_pu_do_shift = instrct;
                if(cav_pu_flag)
                    if(!cav_pu_do_shift)
                        cav_pu_do_shift = instrct;
            }
            for(int j=0; j<ncg; j++) {
                int errpu = pu_estimator_corr[j]->scoreHist(last_case,dose->thisHistoryScore(gind1[j]),dose->thisHistoryScore(gind2[j]));
                bool instrct = pu_estimator_corr[j]->shiftManager(current_case);
            }
      }
      //*HB_end**************************
      dose->setHistory(current_case);
      last_case = current_case;
  }
  return 0;
};

#ifdef BUILD_APP_LIB
APP_LIB(EGS_ChamberApplication);
#else
APP_MAIN(EGS_ChamberApplication);
#endif
