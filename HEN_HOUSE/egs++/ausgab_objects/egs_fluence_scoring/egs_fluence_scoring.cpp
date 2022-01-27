/*
###############################################################################
#
#  EGSnrc egs++ fluence scoring object implementation
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
#  Author:          Ernesto Mainegra-Hing, 2022
#
#  Contributors:    
#
###############################################################################

/*! \file egs_fluence_scoring.cpp
 *  \brief A fluence scoring ausgab object: implementation
 *  
 */

#include <string>
#include <cstdlib>

#include "egs_fluence_scoring.h"
#include "egs_input.h"
#include "egs_functions.h"

EGS_PlanarFluence::EGS_PlanarFluence(const string &Name, EGS_ObjectFactory *f) :
    EGS_AusgabObject(Name,f), hits_field(false), 
    scoring_charge(photon), particle_name("photon"),
    flu_s(0), flu_nbin(128), flu_xmin(0.001), flu_xmax(1.0), 
    norm_u(1.0), Nx(1), Ny(1), flu(0), fluT(0), current_ncase(0), 
    m_primary(0.0), m_tot(0.0)
{
    otype = "EGS_PlanarFluence";
    m_midpoint = EGS_Vector(0,0,5);
    m_R  =  5;
    m_R2 = 25;
    field_type = circle; Area = M_PI * m_R2;
    m_normal = EGS_Vector(0,0,1);
    m_d = m_normal*m_midpoint;
}

/*! Destructor.  */
EGS_PlanarFluence::~EGS_PlanarFluence(){

   if( flu ) {
       for(int j=0; j<Nx*Ny; j++) {delete flu[j];}
       if(flu)  delete [] flu;
       if(fluT) delete fluT;
   }
}

void EGS_PlanarFluence::setApplication(EGS_Application *App) {

    EGS_AusgabObject::setApplication(App);

    if( !app ) return;

    /* Setup fluence scoring arrays, Nx*Ny = 1 for the circle */
    flu  = new EGS_ScoringArray* [Nx*Ny];
    fluT = new EGS_ScoringArray(Nx*Ny);
    for (int j = 0; j < Nx*Ny; j++) 
        flu[j] = new EGS_ScoringArray(flu_nbin);
    describeMe();
}

void EGS_PlanarFluence::initScoring(EGS_Input *inp) {
    if( !inp ) {
        egsWarning("AO type %s: null input?\n",otype.c_str()); return;
    }

    vector<string> name; int the_selection = 0;
    name.push_back("photon"); name.push_back("electron"); name.push_back("positron");
    the_selection = inp->getInput("scoring particle", name, 0);
    switch(the_selection){
        case 1:
          particle_name="electron";
          scoring_charge = electron;
          break;
        case 2:
          particle_name="positron";
          scoring_charge = positron;
          break;
        default:
          particle_name="photon";
          scoring_charge = photon;
    }    

    EGS_Float flu_Emin, flu_Emax, norma;
    int err_n    = inp->getInput("number of bins",flu_nbin);
    int err_i    = inp->getInput("minimum kinetic energy",flu_Emin);
    int err_f    = inp->getInput("maximum kinetic energy",flu_Emax);
    int err_norm = inp->getInput("normalization",norma);
    if (!err_norm) norm_u = norma;

    vector<string> scale;
    scale.push_back("linear"); scale.push_back("logarithmic");
    flu_s = inp->getInput("scale",scale,0);
    if( flu_s == 0 ) {
        flu_xmin = flu_Emin; flu_xmax = flu_Emax;
    }
    else {
        flu_xmin = log(flu_Emin); flu_xmax = log(flu_Emax);
    }
    flu_a = flu_nbin; flu_a /= (flu_xmax - flu_xmin);
    flu_b = -flu_xmin*flu_a;
    /****************************************************** 
       Algorithm assigns E in [Ei,Ei+1), one could add extra
       bin for E = Emax cases. Alternatively, push those
       events into last bin (bias?) during scoring.
       Which approach is correct?
     ******************************************************/
    //flu_nbin++;

    vector<string> output_spectra; 
    output_spectra.push_back("no");
    output_spectra.push_back("yes");
    verbose = inp->getInput("verbose",output_spectra,0);

    // Specific plane input
    vector<EGS_Float> tmp_field;
    int err2 = inp->getInput("scoring circle",tmp_field);
    if( err2 ) {
        int err3 =  inp->getInput("scoring rectangle",tmp_field);
        if( err3 || tmp_field.size() != 4) {
            egsWarning(
            "\n\n***  Wrong/missing 'scoring rectangle' input "
            "setting it to 10 cm X 10 cm field at origin!\n\n");
            m_midpoint = EGS_Vector();
            ax = 10; ay = 10;
            field_type = rectangle; Area = ax*ay;
        }
        else{
            EGS_Float xmin = tmp_field[0],xmax = tmp_field[1],
                      ymin = tmp_field[2],ymax = tmp_field[3];
            /* scoring plane location in space */
            m_midpoint = EGS_Vector((xmax+xmin)/2.,(ymax+ymin)/2.,0); // plane at origin by default
            /* scoring plane normal */
            m_normal = EGS_Vector(0,0,1); // default normal along positive z-axis
            /* define unit vectors on right-handed scoring plane */
            ux = EGS_Vector(1,0,0); uy = EGS_Vector(0,1,0);
            /* Request a scoring plane transformation for initial position and orientation */
            EGS_AffineTransform *t = EGS_AffineTransform::getTransformation(inp);
            if (t){
                t->rotate(m_normal) ; t->transform(m_midpoint);
                t->rotate(ux); t->rotate(uy);
                delete t;
            }
            
            /* get screen resolution */
            vector<int> screen;
            int err4 = inp->getInput("resolution",screen);
            if( err4 ) {
              Nx=1; Ny=1;
              egsWarning(
              "\n\n***  Missing/wrong 'resolution' input "
              "     Scoring in the whole field\n\n");
              
            }
            else if (screen.size()==1){ Nx = screen[0];Ny = screen[0];}
            else if (screen.size()==2){ Nx = screen[0];Ny = screen[1];}
            else if (screen.size()> 2){ Nx = screen[0];Ny = screen[1];
              egsWarning(
              "\n\n***  Too many 'resolution' inputs\n"
              "     Using first two entries\n\n");

            }

            ax = xmax - xmin; ay = ymax-ymin;
            vx = ax/Nx; vy = ay/Ny;
            field_type = rectangle; Area = vx*vy;
        }
    }
    else if( tmp_field.size() != 4 ) {
        egsWarning(
        "\n\n***  Wrong/missing 'scoring circle' input "
        "setting it to 10 cm diameter field at origin!\n\n");
        m_midpoint = EGS_Vector();
        m_R  =  5;
        m_R2 = 25;
        field_type = circle; Area = M_PI * m_R2;
    }
    else{
        vector<EGS_Float> tmp_normal;
        int err1 = inp->getInput("scoring plane normal",tmp_normal);
        if( err1 || tmp_normal.size() != 3 ) {
          egsWarning(
          "\n\n***  Wrong/missing 'scoring plane normal' input. "
          "Set to null vector\n\n");
          m_normal = EGS_Vector();// set to null vector
        }
        else{
          m_normal = EGS_Vector(tmp_normal[0],tmp_normal[1],
                                tmp_normal[2]);
          m_normal.normalize();
        }
        m_midpoint = EGS_Vector(tmp_field[0],tmp_field[1],
                                tmp_field[2]);
        m_R = tmp_field[3]; 
        m_R2 = tmp_field[3]*tmp_field[3];
        field_type = circle; Area = M_PI * m_R2;
    }

    m_d = m_normal*m_midpoint;
}

void EGS_PlanarFluence::describeMe(){
    char buf[128];
    sprintf(buf,"\nPlanar %s fluence\n",particle_name.c_str());
    description =  buf;
    //description  = "\nParticle Fluence Scoring\n";
    description += "========================\n";
    description += " - scoring field normal      = ";
    sprintf(buf,"(%g %g %g)\n",m_normal.x,m_normal.y,m_normal.z); 
    description += buf;
    description += " - scoring field center      = ";
    sprintf(buf,"(%g %g %g)\n",m_midpoint.x,m_midpoint.y,m_midpoint.z); 
    description += buf;
    if (field_type == circle){
       description += " - scoring field radius      = ";
       sprintf(buf,"%g cm\n",m_R); 
       description += buf;
    }
    else if (field_type == rectangle) {
       description += " - scoring field             = ";
       sprintf(buf,"%g cm X %g cm\n",ax,ay); 
       description += buf;
       description += " - scoring field resolution  = ";
       sprintf(buf,"%d X %d\n",Nx,Ny); 
       description += buf;
    }
    description += " - scoring field distance from origin = ";
    sprintf(buf,"%g cm\n",m_d); 
    description += buf;

    if (flu){
       //if (planar_fluence)
       //   description += " - scoring planar particle fluence between ";
       //else
          description += " - scoring planar particle fluence in the ";
       EGS_Float Emin = flu_s ? exp(flu_xmin):flu_xmin, 
                 Emax = flu_s ? exp(flu_xmax):flu_xmax;
       sprintf(buf,"%g MeV and %g MeV energy range",Emin,Emax);
       description += buf;
       if (flu_s)
          description += " on a logarithmic scale \n";
       else
          description += " on a linear scale \n";
    }
    
}

void EGS_PlanarFluence::score(const EGS_Particle& p, const int& ivoxel){
     EGS_Float up = p.u*m_normal, aup = fabs(up); 
     /********************************************************** 
      Prevent large weights from particles very close to plane.
     ***********************************************************/
     if( aup < 0.08 ) aup = 0.0871557;// Limit incident angle to 85 degrees
     EGS_Float e = p.q ? p.E - app->getRM() : p.E;
     if (flu){
       //EGS_Float fup = planar_fluence ? 1.0:aup;
       EGS_Float fup = aup;
       if( flu_s ) {e = log(e);} // log scale
       EGS_Float ae; int je;
       /* Score fluence in each voxel */
       if( e > flu_xmin && e <= flu_xmax) {
         /* Score total fluence in each voxel */
         fluT->score(ivoxel,p.wt/fup);
         /* Score differential fluence in each voxel */
         ae = flu_a*e + flu_b; 
        /****************************************************** 
           Algorithm assigns E in [Ei,Ei+1), hence push events
           with E = Emax into last bin (bias?) during scoring.
           Alternatively add extra bin for E = Emax cases.
           Which approach is correct?
        ******************************************************/
         je = min((int)ae,flu_nbin-1);//je = (int) ae;
        /******************************************************/
         if (ivoxel < 0 || ivoxel > Nx*Ny)
            egsFatal("\n-> Scoring out of bounds, ivoxel = %d\n",ivoxel);
         EGS_ScoringArray *aux = flu[ivoxel];
         if (je < 0 || je >= flu_nbin )
            egsFatal("\n-> Scoring out of bounds, ibin = %d ae = %g E = %g MeV\n",je,ae,e);
         aux->score(je,p.wt/fup);
       }
     }
}

int EGS_PlanarFluence::hitsField(const EGS_Particle& p, EGS_Float* dist){
     if ( field_type==circle ){
         EGS_Float xp = p.x*m_normal, up = p.u*m_normal;
         if( (up > 0 && m_d > xp ) || 
             (up < 0 && m_d < xp ) ){
             EGS_Float t = (m_d - xp)/up;
             EGS_Vector x1(p.x + p.u*t - m_midpoint);
             if( dist ) *dist = t;
             return x1.length2() < m_R*m_R ? 0:-1;
         }
         return -1;
     }
     else if ( field_type == rectangle ){
         EGS_Float xp = p.x*m_normal, up = p.u*m_normal;
         if( (up > 0 && m_d > xp) || (up < 0 && m_d < xp) ) {
             EGS_Float t = (m_d - xp)/up; // distance to plane along u
             EGS_Vector x1(p.x + p.u*t - m_midpoint);// vector on scoring plane
             EGS_Float xcomp = x1*ux;// x-direction component
             EGS_Float ycomp = x1*uy;// y-direction component
                       xcomp = 2*xcomp + ax;
                       ycomp = 2*ycomp + ay;
             if( xcomp > 0 && xcomp < 2*ax && 
                 ycomp > 0 && ycomp < 2*ay ){
                 int i = int(xcomp/(2*vx)),
                     j = int(ycomp/(2*vy)),
                     k = i + j*Nx;
                 if( dist ) *dist = t;
                 return k;
             }
         }
         return -1;
     }
     else return -1;
}

void EGS_PlanarFluence::ouputResults(){
  if (!flu) return;  

EGS_Float src_norm = 1.0,          // default to number of histories in this run
            Fsrc = app->getFluence();// Fluence or number of primary histories
  egsInformation("\n\n last case = %lld source particles or fluence = %g\n\n",
                  current_ncase, Fsrc);

  if (Fsrc) 
     src_norm = Fsrc/current_ncase;// fluence or primary histories per histories run

  string normLabel = src_norm == 1 ? "history" : "MeV-1 cm-2";
  string src_type = app->sourceType();
  if ( src_type == "EGS_BeamSource" ){ 
      normLabel = "primary history";
     egsInformation("\n\n %s normalization = %g (primary histories per particle)\n\n",
                  src_type.c_str(), src_norm);
  }
  else if ( src_type == "EGS_CollimatedSource" || 
           (src_type == "EGS_ParallelBeam" && src_norm != 1)){ 
     egsInformation("\n\n %s normalization = %g (fluence per particle)\n\n",
                  src_type.c_str(), src_norm);
  }
  else{
        egsInformation("\n\n %s normalization = %g (histories per particle)\n\n",
                  src_type.c_str(), src_norm);

  }

  double norm = 1.0/src_norm;  //per fluence or particle depending on source
         norm /= Area;         //per unit area
         norm *= flu_a;        //per unit bin width
         norm *= norm_u;       // times user-requested normalization
  
  if (verbose) 
     egsInformation("\nNormalization = Ncase/Fsrc/A/bw = %g\n",norm);

  string suffix = "_" + particle_name + ".agr";
  string spe_name = app->constructIOFileName(suffix.c_str(),true);
  ofstream spe_output(spe_name.c_str(),ios::out); 
  //spe_output.open(spe_name.c_str());
  if (!spe_output){
      egsFatal("\n EGS_PlanarFluence: Error: Failed to open file %s\n",spe_name.c_str());
      exit(1);
  }

  spe_output << "# " << particle_name.c_str() << " fluence \n";
  spe_output << "# \n";
  spe_output << "@    legend 0.2, 0.8\n";
  spe_output << "@    legend box linestyle 0\n";
  spe_output << "@    legend font 4\n";
  spe_output << "@    xaxis  label \"energy / MeV\"\n";
  spe_output << "@    xaxis  label char size 1.560000\n";
  spe_output << "@    xaxis  label font 4\n";
  spe_output << "@    xaxis  ticklabel font 4\n";
  spe_output << "@    yaxis  label \"fluence / MeV\\S-1\\Ncm\\S-2\"\n";
  spe_output << "@    yaxis  label char size 1.560000\n";
  spe_output << "@    yaxis  label font 4\n";
  spe_output << "@    yaxis  ticklabel font 4\n";
  spe_output << "@    title \""<< particle_name.c_str() << " fluence" <<"\"\n";
  spe_output << "@    title font 4\n";
  spe_output << "@    title size 1.500000\n";
  spe_output << "@    subtitle \"for each scoring region\"\n";
  spe_output << "@    subtitle font 4\n";
  spe_output << "@    subtitle size 1.000000\n";

 
  egsInformation("\n\n%s fluence scoring\n"
                     "=================================\n",particle_name.c_str());
  for(int j=0; j<Ny; j++) {     
    for(int i=0; i<Nx; i++) {     
        int k = i + j*Nx;
        egsInformation("\nVoxel # %d :",k);
        spe_output<<"@    s" << k <<" errorbar linestyle 0\n";
        spe_output<<"@    s" << k <<" legend \""<<
               "Voxel # " << k <<"\"\n";
        spe_output<<"@target G0.S"<<k<<"\n";
        spe_output<<"@type xydy\n";
        double fe,dfe;
        double fp,dfp,dfr;
        fluT->currentResult(k,fe,dfe);
        if( fe > 0 ) dfe = 100*dfe/fe; else dfe = 100;
        egsInformation(" total fluence = %10.4le +/- %-7.3lf\%\n",
              fe*norm/flu_a,dfe);
        if (verbose) egsInformation("\n   Emid/MeV    Flu/(MeV*cm2)   DFlu/(MeV*cm2)\n"
                     "---------------------------------------------\n");
        for(int l=0; l<flu_nbin; l++) {
            flu[k]->currentResult(l,fe,dfe);
            EGS_Float e = (l+0.5-flu_b)/flu_a;
            if( flu_s ) e = exp(e);
            spe_output<<e<<" "<<fe*norm<<" "<<dfe*norm<< "\n";
            if (verbose) egsInformation("%11.6f  %14.6e  %14.6e\n",
                         e,fe*norm,dfe*norm);
        }
        spe_output << "&\n";
    }
  }
  spe_output.close();
       
}

void EGS_PlanarFluence::reportResults() {
    egsInformation("\nFluence Scoring (%s)\n",name.c_str());
    //EGS_Float m_tot = m_fluor+ m_compt + m_ray + m_multiple; char per = '%';
    egsInformation("======================================================\n");
    egsInformation("   Total %ss reaching field:       %g\n",particle_name.c_str(),m_tot);
    egsInformation("   Primary %ss reaching field:     %g\n",particle_name.c_str(),m_primary);
    //egsInformation("   Non-primary photons reaching field: %g\n",m_tot);
    egsInformation("======================================================\n");

    ouputResults();
    
}

bool EGS_PlanarFluence::storeState(ostream &data) const {
  if( !egsStoreI64(data,current_ncase)) return false;
  data << endl;
  //data << m_primary << " " << m_ray << " " << m_compt << " " << m_fluor << " " << m_multiple;
  data << m_tot << " " << m_primary;
  data << endl;
  if (!data.good()) return false;
  if( flu ) {
      for(int j=0; j<Nx*Ny; j++) {
          if( !flu[j]->storeState(data) ) return false;
      }
      if( !fluT->storeState(data) ) return false;
  }
  return true;  
}

bool  EGS_PlanarFluence::setState(istream &data){
  if( !egsGetI64(data,current_ncase) ) return false;
  //data >> m_primary >> m_ray >> m_compt >> m_fluor >> m_multiple;
  data >> m_tot >> m_primary;
  if (!data.good()) return false;
  if( flu ) {
      for(int j=0; j<Nx*Ny; j++) {
          if( !flu[j]->setState(data) ) return false;
      }
      if( !fluT->setState(data) ) return false;
  }
  return true;
}

bool  EGS_PlanarFluence::addState(istream &data){
   EGS_I64 tmp_case; if( !egsGetI64(data,tmp_case) ) return false;
   current_ncase += tmp_case;
   /* individual contributions */
   //EGS_Float tmp_primary, tmp_fluor, tmp_compt, tmp_ray, tmp_multiple;
   //data >> tmp_primary >> tmp_ray >> tmp_compt >> tmp_fluor >> tmp_multiple;
   EGS_Float tmp_tot, tmp_primary;
   data >> tmp_tot >> tmp_primary;
   if (!data.good()) return false;
   m_primary += tmp_primary; 
   m_tot     += tmp_tot;
   //m_primary += tmp_primary;m_ray += tmp_ray;m_compt += tmp_compt;
   //m_fluor += tmp_fluor;m_multiple += tmp_multiple;
   /* fluence objects */
   if( flu ) {
       EGS_ScoringArray tg(flu_nbin);
       for(int j=0; j<Nx*Ny; j++) {
           if( !tg.setState(data) ) return false;
           (*flu[j]) += tg;
       }
       EGS_ScoringArray tgT(Nx*Ny);
       if( !tgT.setState(data) ) return false;
       (*fluT) += tgT;
   }

   return true;
}

/********************************************** 
 * Class EGS_VolumetricFluence Implementation *
***********************************************/

EGS_VolumetricFluence::EGS_VolumetricFluence(const string &Name, EGS_ObjectFactory *f) :
    EGS_AusgabObject(Name,f), scoring_charge(photon), particle_name("photon"),
    flu_s(0), flu_nbin(128), flu_xmin(0.001), flu_xmax(1.0), norm_u(1.0),
    flu(0), fluT(0), current_ncase(0), m_primary(0.0), m_tot(0.0), 
    n_scoring_regions(0), max_reg(-1), active_region(-1), verbose(false)
#ifdef DEBUG
    ,one_bin(0), multi_bin(0)
#endif    
{
    otype = "EGS_VolumetricFluence";
}

/*! Destructor.  */
EGS_VolumetricFluence::~EGS_VolumetricFluence(){

   if( flu ) {
       for(int j=0; j<n_scoring_regions; j++) {delete flu[j];}
       if(flu)  delete [] flu;
       if(fluT) delete fluT;
       //if(volume) delete [] volume;
       //if(is_sensitive) delete [] is_sensitive;
   }
}

void EGS_VolumetricFluence::setApplication(EGS_Application *App) {

    EGS_AusgabObject::setApplication(App);

    if( !app ) return;

    if ( f_regionsString.length() > 0 && !f_region.size() ) {
        getNumberRegions(f_regionsString, f_region);
        getLabelRegions(f_regionsString, f_region);
    }

    // Get the number of regions in the geometry.
    nreg = app->getnRegions();

    // Get the number of scoring regions. If too many, reset to nreg
    n_scoring_regions = f_region.size() < nreg ? f_region.size() : nreg;

    /* Initialize arrays with defaults */
    for (int j=0; j<nreg; j++) {
        is_sensitive.push_back(false);
        volume.push_back(vol_list[0]);// set to either 1.0 or first volume entered
    }
    
    /* Update arrays with user inputs */
    for (int i = 0; i < n_scoring_regions; i++) {
        is_sensitive[f_region[i]] = true;
        if ( i < vol_list.size() ) {
            volume[f_region[i]] = vol_list[i];
        }
        if ( f_region[i] > max_reg ) max_reg = f_region[i];
    }

    /* Setup fluence scoring arrays */
    flu  = new EGS_ScoringArray* [nreg];
    fluT = new EGS_ScoringArray(nreg);
    for (int j = 0; j < nreg; j++){ 
        if (is_sensitive[j])
           flu[j] = new EGS_ScoringArray(flu_nbin);
    }
#ifdef DEBUG 
    binDist = new EGS_ScoringArray(flu_nbin);
#endif    

    /* Initialize data required to score charged particle fluence */
    if ( scoring_charge ){
       EGS_Float flu_Emin = flu_s ? exp(flu_xmin) : flu_xmin, 
                 flu_Emax = flu_s ? exp(flu_xmax) : flu_xmax;
       EGS_Float bw = flu_s ?
                     (log(flu_Emax / flu_Emin))/flu_nbin :
                         (flu_Emax - flu_Emin) /flu_nbin;
       EGS_Float expbw;
       /* Pre-calculated values for faster evaluation on log scale */
       if (flu_s){
          expbw   = exp(bw); // => (Emax/Emin)^(1/nbin)
          r_const = 1/(expbw-1);
          DE      = new EGS_Float [flu_nbin];
          a_const = new EGS_Float [flu_nbin];
          for ( int i = 0; i < flu_nbin; i++ ){
            DE[i]      = flu_Emin*pow(expbw,i)*(expbw-1);
            a_const[i] = 1/flu_Emin*pow(1/expbw,i);
          }
       }
       /* Do not score below ECUT - PRM */
       if (flu_Emin < app->getEcut() - app->getRM() ){
           flu_Emin = app->getEcut() - app->getRM() ;
          /* Decrease number of bins, preserve bin width */
          flu_nbin = flu_s ?
                     ceil((log(flu_Emax / flu_Emin))/bw) :
                     ceil(    (flu_Emax - flu_Emin) /bw);
       }
       
       flu_a_i = bw; flu_a = 1.0/bw;
       
       /* Pre-calculated values for faster 1/stpwr evaluation */
       if (flu_stpwr){
          
          int n_media = app->getnMedia(); 
          
          EGS_Float lnE, lnEmin, lnEmax, lnEmid;

          i_dedx = new EGS_Interpolator [n_media];// stp powers
          dedx_i = new EGS_Interpolator [n_media];// its inverse
          for( int j = 0; j < n_media; j++ ){
              i_dedx[j] = *(app->eDEDX(j));
              EGS_Float Emin = i_dedx[j].getXmin();
              EGS_Float Emax = i_dedx[j].getXmax();
              int n = 1 + i_dedx[j].getIndex(Emax);// getIndex returns lower bin limit?
              EGS_Float bwidth = (Emax - Emin)/n;
#ifdef DEBUG 
              egsInformation("---> stpwr in %s : Emin=%g Emax=%g n=%d bw=%g\n",
              app->getMediumName(j), exp(Emin), exp(Emax), n, bwidth);
#endif             
              int nbins = n + 1;
              EGS_Float spwr_i[nbins]; 
              for (int k = 0; k < nbins; k++ ){
                  spwr_i[k] = 1.0 / i_dedx[j].interpolate(Emin+k*bwidth);
              }
              dedx_i[j].initialize(nbins, Emin, Emax, spwr_i);
#ifdef DEBUG 
              Emin = dedx_i[j].getXmin();
              Emax = dedx_i[j].getXmax();
              n = 1 + dedx_i[j].getIndex(Emax);// getIndex returns lower bin limit?
              bwidth = (Emax - Emin)/n;
              egsInformation("---> 1/stpwr in %s : lnEmin=%g lnEmax=%g n=%d bw=%g index(Emax)=%d\n",
              app->getMediumName(j), Emin, Emax, n, bwidth, dedx_i[j].getIndexFast(Emax));
              for (int k = 0; k < nbins; k++ ){
                egsInformation(" L(%g MeV) = %g MeV/cm 1/L = %g cm/MeV\n", 
                exp(Emin+k*bwidth),
                i_dedx[j].interpolate(Emin + k*bwidth), 
                dedx_i[j].interpolate(Emin + k*bwidth));
              }
              
#endif             
          }

          /* Determine stpwr at middle of each fluence scoring bin */
          lnEmin  = flu_s ? log(0.5*flu_Emin*(expbw+1)) : 0;
          Lmid_i  = new EGS_Float [flu_nbin*n_media];
          for( int j = 0; j < n_media; j++ ){
            for ( int i = 0; i < flu_nbin; i++ ){
                lnEmid = flu_s ? lnEmin + i*bw : log(flu_Emin+bw*(i+0.5));
                Lmid_i[i+j*flu_nbin] = 1/i_dedx[j].interpolate(lnEmid);
                //egsInformation(" 1/L(%g MeV) = %g cm/MeV\n",exp(lnEmid),Lmid_i[i+j*flu_nbin]);
            }
          }
       }
    }


    describeMe();
}

void EGS_VolumetricFluence::getNumberRegions(const string &str, vector<int> &regs) {
    app->getNumberRegions(str, regs);
}

void EGS_VolumetricFluence::getLabelRegions(const string &str, vector<int> &regs) {
    app->getLabelRegions(str, regs);
}

/* 
   Takes user inputs and sets up simulation parameters not requiring
   invoking an application method. This is later done in setApplication.
*/
void EGS_VolumetricFluence::initScoring(EGS_Input *inp) {

    if( !inp ) {
        egsWarning("AO type %s: null input?\n",otype.c_str()); return;
    }

    vector<string> name; int the_selection = 0;
    name.push_back("photon"); name.push_back("electron"); name.push_back("positron");
    the_selection = inp->getInput("scoring particle", name, 0);
    switch(the_selection){
        case 1:
          particle_name="electron";
          scoring_charge = electron;
          break;
        case 2:
          particle_name="positron";
          scoring_charge = positron;
          break;
        default:
          particle_name="photon";
          scoring_charge = photon;
    }    

    EGS_Float flu_Emin, flu_Emax, norma;
    int err_n    = inp->getInput("number of bins",flu_nbin);
    int err_i    = inp->getInput("minimum kinetic energy",flu_Emin);
    int err_f    = inp->getInput("maximum kinetic energy",flu_Emax);
    if (err_n) egsFatal("\n**** Missing input: number of bins. Aborting!\n\n");
    if (err_i) egsFatal("\n**** Missing input: minimum kinetic energy. Aborting!\n\n");
    if (err_f) egsFatal("\n**** Missing input: maximum kinetic energy. Aborting!\n\n");
    int err_norm = inp->getInput("normalization",norma);
    if (!err_norm) norm_u = norma;

    vector<string> scale;
    scale.push_back("linear"); scale.push_back("logarithmic");
    flu_s = inp->getInput("scale",scale,0);
    if( flu_s == 0 ) {
        flu_xmin = flu_Emin; flu_xmax = flu_Emax;
    }
    else {
        flu_xmin = log(flu_Emin); flu_xmax = log(flu_Emax);
    }
    flu_a = flu_nbin; flu_a /= (flu_xmax - flu_xmin);
    flu_b = -flu_xmin*flu_a;

    vector<string> output_spectra; 
    output_spectra.push_back("no");
    output_spectra.push_back("yes");
    verbose = inp->getInput("verbose",output_spectra,0);

    /* get region volume[s] in g/cm3 */
    vector <EGS_Float> v_in;
    inp->getInput("volumes",v_in);

    /* get dose regions */
    bool using_all_regions=true;
    vector <int> f_start, f_stop;
    if (!inp->getInput("regions",f_regionsString) && f_regionsString.length()>0) {
        using_all_regions = false;    // individual regions
    }
    else {
        int err1 = inp->getInput("start region",f_start);
        int err2 = inp->getInput("stop region",f_stop);
        if (!err1 && !err2) {
            if (f_start.size()==f_stop.size()) { // group of dose regions
                for (int i=0; i<f_start.size(); i++) {
                    int ir = f_start[i], fr = f_stop[i];
                    for (int ireg=ir; ireg<=fr; ireg++) {
                        f_region.push_back(ireg);
                    }
                }
                using_all_regions = false;
            }
            else egsWarning(
                    "EGS_VolumetricFluence::initScoring: \n"
                    "  Mismatch in start and stop region groups\n"
                    "  Calculating fluence in ALL regions.\n");
        }
    }

    //================================================
    // Check if one volume for each group requested.
    // Otherwise pass volumes read and if there is
    // a mismatch, then the first volume element
    // or 1g/cm3 will be used.
    //=================================================
    if (! using_all_regions && v_in.size() == f_start.size()) {
        // groups of regions with same volume
        for (int i=0; i<f_start.size(); i++) {
            int i_r = f_start[i], f_r = f_stop[i];
            for (int ireg=i_r; ireg<=f_r; ireg++) {
                vol_list.push_back(v_in[i]);
            }
        }
    }
    else if ( v_in.size() ) {
        vol_list = v_in;
    }
    else{
        vol_list.push_back(1.0);
    }

    /* Initialize data required to score charged particle fluence */
    if ( scoring_charge ){
       vector<string> method;
       method.push_back("flurz"); method.push_back("stpwr");   // 3rd order
                                  method.push_back("stpwrO5"); // 5th order
       flu_stpwr = eFluType(inp->getInput("method",method,1));
    }
}

#define REGIONS_ENTRIES 100
#define REGIONS_PER_LINE 25
void EGS_VolumetricFluence::describeMe(){
    char buf[128];
    sprintf(buf,"\nVolumetric %s fluence scoring\n",particle_name.c_str());
    description =  buf;
    description += "=================================\n";

    description +="\n - scoring regions: ";
    // Get the number of regions in the geometry.
    int start = 0, stop = 0, k = 0, entries = 0;
    /* List up to 100 scoring groups or regions */
    while (k < nreg && entries < REGIONS_ENTRIES){
        if( is_sensitive[k] ){
          start = k;
          entries++;
          while( is_sensitive[k] && k < nreg) {
              k++;
          }
          stop = k-1;
          if ( start < stop )
             sprintf(buf,"[%d - %d]",start,stop);
          else if ( k % REGIONS_PER_LINE )
             sprintf(buf," %d",start);
          else
             sprintf(buf," %d\n                    ",start);
          if (entries == REGIONS_ENTRIES){
             sprintf(buf,"... %d\n",max_reg);
          }
          description += buf;
        }
        k++;
    }

    for (int l = 0; l < nreg; l++){
        if( is_sensitive[l] ){
          egsInformation("  ---> region # %d volume = %g cm3\n",l,volume[l]);
        }
    }

    description += "\n\n - scoring in the ";
    EGS_Float Emin = flu_s ? exp(flu_xmin):flu_xmin, 
              Emax = flu_s ? exp(flu_xmax):flu_xmax;
    sprintf(buf,"%g MeV and %g MeV energy range",Emin,Emax);
    description += buf;
    if (flu_s)
       description += " on a logarithmic scale \n";
    else
       description += " on a linear scale \n";

    if (flu_stpwr){
      if (flu_stpwr == stpwr){
         description += "   O(eps^3) approach: accounts for change in stpwr\n";
         description +=                "   along the step with eps=edep/Eb\n";
      }
      else if (flu_stpwr == stpwrO5){
         description += "   O(eps^5) approach: accounts for change in stpwr\n";
         description += "   along the step with eps=edep/Eb\n";
      }
    }
    else
      description += "   Fluence calculated a-la-FLURZ using Lave=EDEP/TVSTEP.\n";

    if (norm_u != 1.0) {
        description += " Non-unity user-requested normalization = ";
        sprintf(buf,"%g\n",norm_u);
        description += buf;
    }

}
void EGS_VolumetricFluence::ouputResults(){

  
  EGS_Float src_norm = 1.0,          // default to number of histories in this run
            Fsrc = app->getFluence();// Fluence or number of primary histories
  egsInformation("\n\n last case = %lld source particles or fluence = %g\n\n",
                  current_ncase, Fsrc);

  if (Fsrc) 
     src_norm = Fsrc/current_ncase;// fluence or primary histories per histories run

  string normLabel = src_norm == 1 ? "history" : "MeV-1 cm-2";
  string src_type = app->sourceType();
  if ( src_type == "EGS_BeamSource" ){ 
      normLabel = "primary history";
     egsInformation("\n\n %s normalization = %g (primary histories per particle)\n\n",
                  src_type.c_str(), src_norm);
  }
  else if ( src_type == "EGS_CollimatedSource" || 
           (src_type == "EGS_ParallelBeam" && src_norm != 1)){ 
     egsInformation("\n\n %s normalization = %g (fluence per particle)\n\n",
                  src_type.c_str(), src_norm);
  }
  else{
        egsInformation("\n\n %s normalization = %g (histories per particle)\n\n",
                  src_type.c_str(), src_norm);

  }
#ifdef DEBUG
  EGS_Float fbins, d_fbins;
  egsInformation("\nNumber of covered bins distribution\n"
                 "--------------------------------------\n");
  
  int tot_bins = one_bin + multi_bin;
  egsInformation("\none_bin = %d [%-7.3lf\%] multi_bin = %d [%-7.3lf\%]\n", 
  one_bin, 100.0*one_bin/tot_bins, multi_bin,100.0*multi_bin/tot_bins);
  
  egsInformation("\n  # bins        freq         unc           percentage  \n");               
  for (int i=0; i<flu_nbin; i++) {
      binDist->currentResult(i,fbins, d_fbins);
      if (fbins){
         d_fbins = 100.0*d_fbins/fbins; fbins = current_ncase*fbins;
         egsInformation("   %d        %11.2f  [%-7.3lf\%]     %11.2f \%\n", 
         i+1, fbins, d_fbins, 100.*fbins/tot_bins);
      }
  }
#endif

  string suffix = "_" + particle_name + ".agr";
  string spe_name = app->constructIOFileName(suffix.c_str(),true);
  ofstream spe_output(spe_name.c_str(),ios::out); 
  if (!spe_output){
      egsFatal("\n EGS_VolumetricFluence: Error: Failed to open file %s\n",spe_name.c_str());
      exit(1);
  }

  spe_output << "# Volumetric " << particle_name.c_str() << " fluence \n";
  spe_output << "# \n";
  spe_output << "@    legend 0.2, 0.8\n";
  spe_output << "@    legend box linestyle 0\n";
  spe_output << "@    legend font 4\n";
  spe_output << "@    xaxis  label \"energy / MeV\"\n";
  spe_output << "@    xaxis  label char size 1.560000\n";
  spe_output << "@    xaxis  label font 4\n";
  spe_output << "@    xaxis  ticklabel font 4\n";
  if (src_norm == 1 || normLabel == "primary history")
     spe_output << "@    yaxis  label \"fluence / MeV\\S-1\\Ncm\\S-2\"\n";
  else{   
     spe_output << "@    yaxis  label \"fluence / MeV\\S-1\"\n";
  }
  spe_output << "@    yaxis  label char size 1.560000\n";
  spe_output << "@    yaxis  label font 4\n";
  spe_output << "@    yaxis  ticklabel font 4\n";
  spe_output << "@    title \""<< particle_name.c_str() << " fluence" <<"\"\n";
  spe_output << "@    title font 4\n";
  spe_output << "@    title size 1.500000\n";
  spe_output << "@    subtitle \"for each scoring region\"\n";
  spe_output << "@    subtitle font 4\n";
  spe_output << "@    subtitle size 1.000000\n";
 
  egsInformation("\n\n%s fluence scoring\n"
                     "=================================\n",particle_name.c_str());
  // Get the number of regions in the geometry.
  for (int j = 0; j < nreg; j++) {
      if ( !is_sensitive[j] ) continue;
      EGS_Float norm  = 1.0/src_norm;// per particle or fluence
                norm *= norm_u;      // user-requested normalization
                norm /= volume[j];   //per unit volume
                //norm *= flu_a;     //per unit bin width <- implicit in scoring!

      EGS_Float the_bw = flu_s? 1.0 : flu_a_i;// Implicit for log grid

      if (verbose){
         egsInformation("\nNormalization = Ncase/Fsrc/V = %g\n",norm);
         egsInformation("Volume[%d] = %g bw = %g nbins = %d\n",j,volume[j], the_bw, flu_nbin);
      }
      egsInformation("region # %d : ",j);

      double fe,dfe,fp,dfp;
      fluT->currentResult(j,fe,dfe);
      if (fe > 0) {
          dfe = 100*dfe/fe;
      }
      else {
          dfe = 100;
      }
      egsInformation(" total fluence [cm-2] = %10.4le +/- %-7.3lf\%\n",
                     fe*norm*the_bw,dfe);

      spe_output<<"@    s"<<j<<" errorbar linestyle 0\n";
      spe_output<<"@    s"<< j <<" legend \""<< "region # " << j <<"\"\n";
      spe_output<<"@target G0.S"<<j<<"\n";
      spe_output<<"@type xydy\n";
      if (verbose) egsInformation("\n   Emid/MeV    Flu/(MeV-1*cm-2)   DFlu/(MeV-1*cm-2)\n"
                     "---------------------------------------------------\n");
      for (int i=0; i<flu_nbin; i++) {
          flu[j]->currentResult(i,fe,dfe);
          EGS_Float e = (i+0.5-flu_b)/flu_a;
          if (flu_s) e = exp(e);
          spe_output << e <<" "<< fe *norm<<" "<< dfe *norm<< "\n";
          if (verbose) egsInformation("%11.6f  %14.6e  %14.6e\n",
                        e, fe*norm, dfe*norm);
      }
      spe_output << "&\n";

  }


  spe_output.close();
       
}

void EGS_VolumetricFluence::reportResults() {

    egsInformation("\nFluence Scoring (%s)\n",name.c_str());
    egsInformation("======================================================\n");

    ouputResults();
    
}

bool EGS_VolumetricFluence::storeState(ostream &data) const {
  if( !egsStoreI64(data,current_ncase)) return false;
  data << endl;
  if (!data.good()) return false;
  if( flu ) {
      for(int j = 0; j < nreg; j++) {
          if ( is_sensitive[j] ){ 
             if( !flu[j]->storeState(data) ) return false;
          }
      }
      if( !fluT->storeState(data) ) return false;
  }
  return true;  
}

bool  EGS_VolumetricFluence::setState(istream &data){
  if( !egsGetI64(data,current_ncase) ) return false;
  if (!data.good()) return false;
  if( flu ) {
      for(int j=0; j<nreg; j++) {
          if ( is_sensitive[j] ){ 
             if( !flu[j]->setState(data) ) return false;
          }
      }
      if( !fluT->setState(data) ) return false;
  }
  return true;
}

bool  EGS_VolumetricFluence::addState(istream &data){
   EGS_I64 tmp_case; if( !egsGetI64(data,tmp_case) ) return false;
   current_ncase += tmp_case;
   if (!data.good()) return false;
   /* fluence objects */
   if( flu ) {
       EGS_ScoringArray tg(flu_nbin);
       for(int j = 0; j < nreg; j++) {
           if ( is_sensitive[j] ){ 
             if( !tg.setState(data) ) return false;
             (*flu[j]) += tg;
           }
       }
       EGS_ScoringArray tgT(nreg);
       if( !tgT.setState(data) ) return false;
       (*fluT) += tgT;
   }

   return true;
}



extern "C" {

  EGS_FLUENCE_SCORING_EXPORT EGS_AusgabObject* 
                          createAusgabObject(EGS_Input *input, EGS_ObjectFactory *f) {
      const static char *func = "createAusgabObject(fluence_scoring)";
      if( !input ) {
          egsWarning("%s: null input?\n",func); return 0;
      }
  
      string type;
      int error = input->getInput("type",type);
      if ( !error && input->compare("planar",type) ) {
        EGS_PlanarFluence *result = new EGS_PlanarFluence("", f);
        result->setName(input);
        result->initScoring(input);
        return result;
      }
      else if ( !error && input->compare("volumetric",type) ){
        EGS_VolumetricFluence *result = new EGS_VolumetricFluence("", f);
        result->setName(input);
        result->initScoring(input);
        return result;
      }
      else{
        egsFatal("Invalid fluence type input?\n\n\n"); 
        return 0;
      }
  }

}
