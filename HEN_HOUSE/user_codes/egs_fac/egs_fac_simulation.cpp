/*
###############################################################################
#
#  EGSnrc egs++ egs_fac simulation
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


#include "egs_fac_simulation.h"

#include "egs_functions.h"
#include "egs_input.h"
#include "egs_transformations.h"
#include "egs_base_geometry.h"

EGS_FACSimulation::~EGS_FACSimulation() {
    if( transform ) delete transform;
    if( properties ) delete [] properties;
}

EGS_FACSimulation::EGS_FACSimulation(EGS_BaseGeometry *g,
    EGS_AffineTransform *t) : geometry(g), rr(0), transform(t), cmass(1),
    z_pom(0), R_pom(0.5), R2_pom(0.25), L(0), h(0),
    fsplit(1), fspliti(1), med_cv(0), had_edep(false) {
    int nreg = geometry->regions();
    properties = new unsigned char [nreg];
    int j;
    for(j=0; j<nreg; ++j) properties[j] = 0;
    resetCounter();
}

void EGS_FACSimulation::getRatio(double r1, double dr1, double r2, double dr2,
                  double dcor, EGS_I64 ncase,
                  double &A, double &dA) {
    A = 1; dA = 100;
    if( !r1 || !r2 ) return;
    r1 /= ncase; dr1 /= ncase; dr1 -= r1*r1;
    r2 /= ncase; dr2 /= ncase; dr2 -= r2*r2;
    dr1 = dr1/(r1*r1); dr2 = dr2/(r2*r2);
    dcor = dcor/(r1*r2*ncase) - 1;
    A = r1/r2;
    dA = (dr1 + dr2 - 2*dcor)/(ncase - 1);
    if( dA > 0 ) dA = 100*sqrt(dA);
    //dA = 100*sqrt((dr1 + dr2 - 2*dcor)/(ncase - 1));
}

const char* EGS_FACSimulation::Dnames[] = {
    "Eg  = ",
    "E1  = ",
    "E2  = ",
    "E3  = ",
    "E4  = ",
    "E4a = ",
    "E5  = ",
    "E6  = "
};

const char* EGS_FACSimulation::Anames[] = {
    "Eg/E1  = Aap    = ",
    "E1/E2  = Ascat  = ",
    "E2/E3  = Aeloss = ",
    "E3/E4  = Acpe   = ",
    "E4/E4a = Acheck = ",
    "E4a/E5 = Ag     = ",
    "E5/E6  = Aatt   = ",
    "Aap*Ascat*Aeloss*Acpe = ",
    "Atotal                = "
};

bool EGS_FACSimulation::outputData(ostream &data) {
    int j;
    for(j=0; j<N_FAC_DOSE; ++j) data << dose[j] << " ";
    data << endl;
    for(j=0; j<N_FAC_DOSE; ++j) data << dose2[j] << " ";
    data << endl;
    for(j=0; j<N_FAC_DOSE; ++j) data << dosec[j] << " ";
    data << endl;
    for(j=0; j<N_FAC_CORR; ++j) data << extra[j] << " ";
    data << endl;
    return data.fail();
}

bool EGS_FACSimulation::readData(istream &data) {
    int j;
    for(j=0; j<N_FAC_DOSE; ++j) data >> dose[j];
    for(j=0; j<N_FAC_DOSE; ++j) data >> dose2[j];
    for(j=0; j<N_FAC_DOSE; ++j) data >> dosec[j];
    for(j=0; j<N_FAC_CORR; ++j) data >> extra[j];
    return data.fail();
}

void EGS_FACSimulation::resetCounter() {
    int j;
    for(j=0; j<N_FAC_DOSE; ++j) {
        dose[j]=0; dose2[j]=0; dosec[j]=0; dtmp[j]=0;
    }
    for(j=0; j<N_FAC_CORR; ++j) extra[j] = 0;
}

bool EGS_FACSimulation::addData(istream &data) {
    int j; double tmp;
    for(j=0; j<N_FAC_DOSE; ++j) { data >> tmp; dose[j] += tmp; }
    for(j=0; j<N_FAC_DOSE; ++j) { data >> tmp; dose2[j] += tmp; }
    for(j=0; j<N_FAC_DOSE; ++j) { data >> tmp; dosec[j] += tmp; }
    for(j=0; j<N_FAC_CORR; ++j) { data >> tmp; extra[j] += tmp; }
    return data.fail();
}

void EGS_FACSimulation::describeSimulation(){
     egsInformation("%-40s ",geometry->getName().c_str());
     if (include_scatter)
      egsInformation("=> Include all non-aperture scatter\n");
     else
      egsInformation("=> Exclude scatter past POM\n");
}

void EGS_FACSimulation::reportResults(double flu, EGS_I64 ncase) {
    finishHistory();
    egsInformation(
"\n===============================================================================\n"
"  Simulation geometry: %s %s\n"
"===============================================================================\n"
"  POM position      : (0,0,%g)\n"
"  POM radius        : %g\n"
"  POM to CV distance: %g\n"
"  CV height         : %g\n",
    geometry->getName().c_str(),
    include_scatter ? "(Include all scatter)":"(Exclude scatter past POM)",
    z_pom,R_pom,L,h);
    egsInformation("  Cavity regions    :");
    int nout=0; int j;
    for(j=0; j<geometry->regions(); ++j) {
        if( isCavity(j) ) {
            if( nout == 20 ) {
                egsInformation("\n                     ");
                nout = 0;
            }
            egsInformation(" %d",j); ++nout;
        }
    }
    if( nout > 0 ) egsInformation("\n");
    egsInformation("  Aperture regions  :");
    nout=0;
    for(j=0; j<geometry->regions(); ++j) {
        if( isAperture(j) ) {
            if( nout == 20 ) {
                egsInformation("\n                     ");
                nout = 0;
            }
            egsInformation(" %d",j); ++nout;
        }
    }
    if( nout > 0 ) egsInformation("\n");
    egsInformation("  Front/back regions:");
    nout=0;
    for(j=0; j<geometry->regions(); ++j) {
        if( isFrontBack(j) ) {
            if( nout == 20 ) {
                egsInformation("\n                     ");
                nout = 0;
            }
            egsInformation(" %d",j); ++nout;
        }
    }
    if( nout > 0 ) egsInformation("\n");
    egsInformation(
"-------------------------------------------------------------------------------\n");
    char c='%';
    for(j=0; j<N_FAC_DOSE; ++j) {
        double f = dose[j]/ncase, f2 = dose2[j]/ncase;
        if( f ) {
            f2 -= f*f; if( f2 > 0 ) f2 = sqrt(f2/(ncase-1));
            f2 *= 100./fabs(f);
            f *= ncase; f *= 1.6022e-10/(flu*cmass);
            egsInformation("%s%g +/- %g%c\n",Dnames[j],f,f2,c);
        } else egsInformation("%sZero dose\n",Dnames[j]);
    }
    egsInformation(
"-------------------------------------------------------------------------------\n");
    double A, dA; double Atot=1, dAtot=0;
    for(j=0; j<N_FAC_DOSE-1; ++j) {
        getRatio(dose[j],dose2[j],dose[j+1],dose2[j+1],dosec[j],ncase,A,dA);
        egsInformation("%s%10.6f +/- %8.4f%c\n",Anames[j],A,dA,c);
        if( j != 4 ) { Atot *= A; dAtot += dA*dA; };
    }
    egsInformation(
"-------------------------------------------------------------------------------\n");
    getRatio(dose[0],dose2[0],dose[4],dose2[4],dosec[7],ncase,A,dA);
    egsInformation("%s%10.6f +/- %8.4f%c\n",Anames[7],A,dA,c);
    egsInformation("%s%10.6f +/- %8.4f%c\n",Anames[8],Atot,sqrt(dAtot),c);
    egsInformation(
"===============================================================================\n");
}

void EGS_FACSimulation::getAtotal(EGS_I64 ncase, double &A, double &dA) {
    A = 1; dA = 100;
    if( !dose[4] || !dose[7] ) return;
    if( !dose[0] || !dose[5] ) { A = 0; return; }
    double Eg = dose[0]/ncase, dEg = dose2[0]/ncase; dEg = dEg/(Eg*Eg)-1;
    double E4 = dose[4]/ncase, dE4 = dose2[4]/ncase; dE4 = dE4/(E4*E4)-1;
    double E5 = dose[5]/ncase, dE5 = dose2[5]/ncase; dE5 = dE5/(E5*E5)-1;
    double E7 = dose[7]/ncase, dE7 = dose2[7]/ncase; dE7 = dE7/(E7*E7)-1;
    egsInformation("getAtotal: dEg=%g dE4=%g dE5=%g dE7=%g\n",dEg,dE4,dE5,dE7);
    egsInformation("cov(Eg,E4)=%g\n",extra[0]/(Eg*E4*ncase)-1);
    egsInformation("cov(Eg,E5)=%g\n",extra[1]/(Eg*E5*ncase)-1);
    egsInformation("cov(Eg,E7)=%g\n",extra[2]/(Eg*E7*ncase)-1);
    egsInformation("cov(E4,E5)=%g\n",extra[3]/(E4*E5*ncase)-1);
    egsInformation("cov(E4,E7)=%g\n",extra[4]/(E4*E7*ncase)-1);
    egsInformation("cov(E5,E7)=%g\n",extra[5]/(E5*E7*ncase)-1);
    double cov = -(extra[0]/(Eg*E4*ncase)-1) + (extra[1]/(Eg*E5*ncase)-1)
                 -(extra[2]/(Eg*E7*ncase)-1) - (extra[3]/(E4*E5*ncase)-1)
                 +(extra[4]/(E4*E7*ncase)-1) - (extra[5]/(E5*E7*ncase)-1);
    egsInformation("total cov=%g\n",cov);
    A = Eg*E5/(E4*E7);
    dA = (dEg + dE4 + dE5 + dE7 + 2*cov)/(ncase-1);
    if( dA > 0 ) dA = 100*sqrt(dA);
}

EGS_FACSimulation* EGS_FACSimulation::getFACSimulation(EGS_Input *aux) {
    const static char *func = "EGS_FACSimulation::getFACSimulation:";
    const static char *err_msg = "%s wrong/missing '%s' input\n";
    if( !aux ) return 0;
    string gname;
    int err = aux->getInput("geometry name",gname);
    vector<int> cav;
    int err1 = aux->getInput("cavity regions",cav);
    vector<int> apert;
    int err2 = aux->getInput("aperture regions",apert);
    vector<int> ends;
    int err3 = aux->getInput("front and back regions",ends);
    EGS_Float cmass;
    int err4 = aux->getInput("cavity mass",cmass);
    vector<EGS_Float> pom;
    int err5 = aux->getInput("POM",pom);
    EGS_Float fspl;
    int err7 = aux->getInput("photon splitting",fspl);
    vector<string> scat; scat.push_back("no"); scat.push_back("yes");
    int tmp_scatter = aux->getInput("include scatter",scat,0);
    if( pom.size() != 2 ) err5 = 1;
    if( err  ) egsWarning(err_msg,func,"geometry name");
    if( err1 ) egsWarning(err_msg,func,"cavity regions");
    if( err3 ) egsWarning(err_msg,func,"front and back regions");
    if( err5 ) egsWarning(err_msg,func,"POM");
    if( err || err1 || err3 || err5 ) return 0;
    EGS_BaseGeometry *g = EGS_BaseGeometry::getGeometry(gname);
    if( !g ) {
        egsWarning("%s a geometry with name %s does not exist\n",func,gname.c_str());
        return 0;
    }
    vector<int> splitting_on, splitting_off;
    int err8 = aux->getInput("splitting on in regions",splitting_on);
    int err9 = aux->getInput("splitting off in regions",splitting_off);
    EGS_AffineTransform *t = EGS_AffineTransform::getTransformation(aux);
    EGS_FACSimulation *result = new EGS_FACSimulation(g,t);
    aux->getInput("cavity mass",result->cmass);
    if (tmp_scatter) result->include_scatter = true;
    else             result->include_scatter = false;
    int nreg = g->regions(); int j; bool ok = true;
    bool found_cv = false;
    for(j=0; j<cav.size(); ++j) {
        if( cav[j] >= 0 && cav[j] < nreg ) {
            result->setCavity(cav[j]);
            int imed = result->geometry->medium(cav[j]);
            if( !found_cv ) { result->med_cv = imed; found_cv = true; }
            else {
                if( imed != result->med_cv ) {
                    egsWarning("\n*** CV medium %d found in region %d is different"
                       " from the CV medium %d in another CV region?\n",
                       imed,cav[j],result->med_cv);
                    ok = false;
                }
            }
        }
        else {
            egsWarning("%s invalid cavity region %d\n",func,cav[j]);
            ok = false;
        }
    }
    for(j=0; j<ends.size(); ++j) {
        if( ends[j] >= 0 && ends[j] < nreg ) result->setFrontBack(ends[j]);
        else {
            egsWarning("%s invalid front/back region %d\n",func,ends[j]);
            ok = false;
        }
    }
    if( !ok ) {
        egsWarning("%s errors for simulation geometry %s -> ignored\n",
                func,gname.c_str());
        delete result; return 0;
    }
    if( !err7 && fspl > 1 ) {
        result->fsplit = fspl;
        result->fspliti = 1/fspl;
    }
    for(j=0; j<apert.size(); ++j) {
        if( apert[j] >= 0 && apert[j] < nreg ) result->setAperture(apert[j]);
        else egsWarning("%s invalid aperture region %d\n",func,apert[j]);
    }
    if( result->setPOM(pom[0],pom[1]) ) {
        egsWarning("%s failed to set POM\n",func);
        delete result; result = 0;
    }
    if( result ) {
        if( err8 && err9 ) {
            // turn on everywhere
            for(j=0; j<nreg; ++j) result->setSplittingOn(j);
        }
        else if( !err8 ) {
            // turn on in selected regions
            for(j=0; j<splitting_on.size(); ++j) {
                int ireg = splitting_on[j];
                if( ireg >= 0 && ireg < nreg ) result->setSplittingOn(ireg);
            }
        }
        else if( !err9 ) {
            // turn on everywhere and the turn off in selected regions
            for(j=0; j<nreg; ++j) result->setSplittingOn(j);
            for(j=0; j<splitting_on.size(); ++j) {
                int ireg = splitting_on[j];
                if( ireg >= 0 && ireg < nreg ) result->setSplittingOff(ireg);
            }
        }
        else {
            egsFatal("You can not use splitting on and off simultaneously\n"
                    "because this does not make any sense\n");
        }
    }
    return result;
}

int EGS_FACSimulation::setPOM(EGS_Float z, EGS_Float r) {
    if( r <= 0 ) {
        egsWarning("EGS_FACSimulation::setPOM: negative radius %g?\n",r);
        return 1;
    }
    z_pom = z; R_pom = r; R2_pom = r*r;
    EGS_Vector x(0,0,z_pom+1e-6), u(0,0,1);
    int ireg = geometry->isWhere(x);
    if( ireg < 0 ) {
        egsWarning("Position (0,0,%g) is outside of the geometry?\n",z_pom+1e-6);
        return 2;
    }
    bool found_CV = false, is_CV = false;
    if( isCavity(ireg) ) { found_CV = true; is_CV = true; }
    EGS_Float L_tot = 0, h_tot = 0;
    while(1) {
        EGS_Float t=1e30; int inew = geometry->howfar(ireg,x,u,t);
        if( is_CV ) h_tot += t; else if( !found_CV ) L_tot += t;
        if( inew < 0 ) break;
        ireg = inew; x += u*t;
        bool is_CV_next = isCavity(ireg);
        if( is_CV_next && !is_CV && found_CV ) {
            egsWarning("EGS_FACSimulation::setPOM: CV appears to consist of disjoint regions\n"
                "  This is not supported\n"); return 3;
        }
        is_CV = is_CV_next; if( is_CV ) found_CV = true;
    }
    L = L_tot; h = h_tot;
    egsInformation("EGS_FACSimulation::setPOM:\n");
    egsInformation("  distance from POM to CV front face: %g\n",L);
    egsInformation("  CV heigh                          : %g\n",h);
    return 0;
}

EGS_FACCorrelation::EGS_FACCorrelation(EGS_FACSimulation *sim1,
        EGS_FACSimulation *sim2) : s1(sim1), s2(sim2) {
    resetCounter();
}

EGS_FACCorrelation::~EGS_FACCorrelation() {}

void EGS_FACCorrelation::resetCounter() {
    int j;
    for(j=0; j<N_FAC_DOSE; ++j) corr[j] = 0;
    for(j=0; j<N_RATIO_COV; ++j) extra[j] = 0;
}

bool EGS_FACCorrelation::outputData(ostream &data) {
    int j;
    for(j=0; j<N_FAC_DOSE; ++j) data << corr[j] << " ";
    for(j=0; j<N_RATIO_COV; ++j) data << extra[j] << " ";
    data << endl;
    return data.fail();
}

bool EGS_FACCorrelation::readData(istream &data) {
    int j;
    for(j=0; j<N_FAC_DOSE; ++j) data >> corr[j];
    for(j=0; j<N_RATIO_COV; ++j) data >> extra[j];
    return data.fail();
}

bool EGS_FACCorrelation::addData(istream &data) {
    double tmp; int j;
    for(int j=0; j<N_FAC_DOSE; ++j) {
        data >> tmp; corr[j] += tmp;
    }
    for(int j=0; j<N_RATIO_COV; ++j) {
        data >> tmp; extra[j] += tmp;
    }
    return data.fail();
}

const char* EGS_FACCorrelation::Dnames[] = {
    "Eg  ratio = ",
    "E1  ratio = ",
    "E2  ratio = ",
    "E3  ratio = ",
    "E4  ratio = ",
    "E4a ratio = ",
    "E5  ratio = ",
    "E6  ratio = ",
    "Eg' ratio = "
};


void EGS_FACCorrelation::reportResults(double flu, EGS_I64 ncase) {
    egsInformation("\n==============================================================================\n");
    egsInformation("geometry1: %s,",s1->geometry->getName().c_str());
    int ns = s1->geometry->getName().size();
    int nskip = ns < 25 ? 25 - ns : 1;
    int j;
    for(j=0; j<nskip; ++j) egsInformation(" ");
    egsInformation("geometry2: %s\n",s2->geometry->getName().c_str());
    egsInformation("------------------------------------------------------------------------------\n");
    char c='%'; double r, dr;
    for(j=0; j<N_FAC_DOSE; ++j) {
        if( s1->dose[j] && s2->dose[j] ) {
            s1->getRatio(s1->dose[j],s1->dose2[j],s2->dose[j],s2->dose2[j],
                    corr[j],ncase,r,dr);
            egsInformation("%s%10.6f +/- %8.4f%c\n",Dnames[j],r,dr,c);
        }
        else egsInformation("%sZero dose\n",Dnames[j]);
    }
    /*
    s1->getRatio(extra[0],extra[1],extra[2],extra[3],extra[4],ncase,r,dr);
    egsInformation("%s%10.6f +/- %8.4f%c\n",Dnames[8],r,dr,c);
    egsInformation("Eg' ratio: %10.6f ncase=%lld\n",
            s1->dose[0]*s1->dose[5]*s2->dose[4]/
           (s2->dose[0]*s2->dose[5]*s1->dose[4]),ncase);
    */
    double v[6], dv[6]; double nci = 1./ncase;

    v[0] = s1->dose[0]*nci; dv[0] = s1->dose2[0]*nci/(v[0]*v[0])-1;
    v[1] = s2->dose[4]*nci; dv[1] = s2->dose2[4]*nci/(v[1]*v[1])-1;
    v[2] = s1->dose[5]*nci; dv[2] = s1->dose2[5]*nci/(v[2]*v[2])-1;

    v[3] = s2->dose[0]*nci; dv[3] = s2->dose2[0]*nci/(v[3]*v[3])-1;
    v[4] = s1->dose[4]*nci; dv[4] = s1->dose2[4]*nci/(v[4]*v[4])-1;
    v[5] = s2->dose[5]*nci; dv[5] = s2->dose2[5]*nci/(v[5]*v[5])-1;

    double dA = dv[0] + dv[1] + dv[2] + dv[3] + dv[4] + dv[5];
    double A = v[0]*v[1]*v[2]/(v[3]*v[4]*v[5]);
    int ij=0;
    for(int i=0; i<5; ++i) {
        double sign1 = i < 3 ? 1 : -1;
        for(int j=i+1; j<6; ++j) {
            double sign2 = j < 3 ? 1 : -1;
            double cov = extra[ij++]/(v[i]*v[j]*ncase)-1;
            dA += 2*sign1*sign2*cov;
        }
    }
    if( dA > 0 ) dA = sqrt(dA/(ncase-1))*100;
    egsInformation("%s%10.6f +/- %8.4f%c\n",Dnames[8],A,dA,c);
    egsInformation("==============================================================================\n");
}

void EGS_AxCalculator::reportResults(EGS_I64 ncase) {
    double nci = 1./ncase; double A, dA = 0; char c='%';
    v[0] = s0->dose[6]*nci; dA += s0->dose2[6]*nci/(v[0]*v[0])-1;
    v[1] = s2->dose[0]*nci; dA += s2->dose2[0]*nci/(v[1]*v[1])-1;
    v[2] = s2->dose[5]*nci; dA += s2->dose2[5]*nci/(v[2]*v[2])-1;
    v[3] = s1->dose[4]*nci; dA += s1->dose2[4]*nci/(v[3]*v[3])-1;
    v[4] = s0->dose[7]*nci; dA += s0->dose2[7]*nci/(v[4]*v[4])-1;
    v[5] = s1->dose[0]*nci; dA += s1->dose2[0]*nci/(v[5]*v[5])-1;
    v[6] = s1->dose[5]*nci; dA += s1->dose2[5]*nci/(v[6]*v[6])-1;
    v[7] = s2->dose[4]*nci; dA += s2->dose2[4]*nci/(v[7]*v[7])-1;
    A = v[0]*v[1]*v[2]*v[3]/(v[4]*v[5]*v[6]*v[7]);
    int ij=0;
    for(int i=0; i<7; ++i) {
        double sign1 = i < 4 ? 1 : -1;
        for(int j=i+1; j<8; ++j) {
            double sign2 = j < 4 ? 1 : -1;
            double cov = cov_matrix[ij++]/(v[i]*v[j]*ncase)-1;
            dA += 2*sign1*sign2*cov;
        }
    }
    if( dA > 0 ) dA = sqrt(dA/(ncase-1))*100;
    egsInformation("\n==============================================================================\n");
    egsInformation("Ax computed from the following geometries:\n");
    egsInformation("   Aatt calculated in %s\n",s0->geometry->getName().c_str());
    egsInformation("   experimental Aatt from Eg ratio in %s and %s\n",
            s1->geometry->getName().c_str(),s2->geometry->getName().c_str());
    egsInformation("------------------------------------------------------------------------------\n");
    egsInformation("Ax = %10.6f +/- %8.4f%c\n",A,dA,c);
    egsInformation("==============================================================================\n");
}

EGS_AxCalculator::EGS_AxCalculator(EGS_FACSimulation *sim0,
    EGS_FACSimulation *sim1, EGS_FACSimulation *sim2) :
    s0(sim0), s1(sim1), s2(sim2) {
    resetCounter();
}

EGS_AxCalculator::~EGS_AxCalculator() {}

bool EGS_AxCalculator::outputData(ostream &data) {
    for(int j=0; j<N_AX_COV; ++j) data << cov_matrix[j] << " ";
    data << endl; return data.fail();
}

bool EGS_AxCalculator::readData(istream &data) {
    for(int j=0; j<N_AX_COV; ++j) data >> cov_matrix[j];
    return data.fail();
}

bool EGS_AxCalculator::addData(istream &data) {
    for(int j=0; j<N_AX_COV; ++j) {
        double tmp; data >> tmp; cov_matrix[j] += tmp;
    }
    return data.fail();
}

void EGS_AxCalculator::resetCounter() {
    for(int j=0; j<N_AX_COV; ++j) cov_matrix[j] = 0;
}
