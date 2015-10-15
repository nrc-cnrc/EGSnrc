/*
###############################################################################
#
#  EGSnrc egs_inprz save
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
#  Author:          Ernesto Mainegra-Hing, 2003
#
#  Contributors:    Blake Walters
#
###############################################################################
*/


#include "inputRZImpl.h"
#include <qlineedit.h>
#include <qapplication.h>
//#include <q3filedialog.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qcheckbox.h>
//#include <q3groupbox.h>
//#include <q3table.h>
//Added by qt3to4:
//#include <Q3TextStream>
#include <iterator>
#include <qtabwidget.h>
#include <qpushbutton.h>
#include <qdir.h>
//#include <q3progressdialog.h>
//#include <q3buttongroup.h>
#include <qwidget.h>

//qt3to4 -- BW
#include <QTextStream>

//**********************************************
// *********        SAVING STUFF     ***********
//**********************************************

void inputRZImpl::UpDateInputRZFile()
{

//#ifdef WIN32
//#else
// Saves only in $HOME/egsnrc/usercodename
  EGSdir = GetUserCodeDir( usercodename );
  if ( OtherAreaRadioButton->isChecked() ){
      update_from_user_area(); // reset radio buttons for user codes
      disconnect( InputFileComboBox, SIGNAL( editTextChanged(const QString&) ), this,
                                     SLOT( EGSFileNameChanged(const QString&) ) );
      update_files( EGSdir, InputFileComboBox, "*.egsinp" );
      connect( InputFileComboBox, SIGNAL( editTextChanged(const QString&) ), this,
                                  SLOT( EGSFileNameChanged(const QString&) ) );
  }

//#endif
  QFile ftest( EGSdir + EGSfileName );
  if ( ftest.open( QIODevice::ReadOnly ) ) { //file exists, ask for permission to overwrite
       ftest.close();//close it first and reopen write-only
       QString warnStr = "File "  + EGSdir + EGSfileName + " exists!\n";
	       warnStr += "Overwriting original file on the disk.\n";
	       warnStr += "Do you really want to save?";
       QApplication::beep();
       QMessageBox mb( "B e w a r e",  warnStr,
                                       QMessageBox::Warning,
                                       QMessageBox::Yes | QMessageBox::Default,
                                       QMessageBox::No, 0 );
        switch( mb.exec() ) {
          case QMessageBox::Yes:
               // save and exit
               break;
          case QMessageBox::No:
               // exit without saving
               return;
               break;
        }
  }
  // we need this in case manual modifications ocurred
  // clean error buffers, if there were errors, defaults will be used
  openErrors = "";
  formErrors  = "";
  geoErrors   = "";
//Transfer info from form to variables
  EGSfileName      = InputFileComboBox->currentText ();
  PEGSfileName     = pegs4ComboBox->currentText ();
 //write info to EGS input file
  save();
  //update input file name combo list in case edit line modified....
  Add_New_Item( EGSfileName.toLatin1().data() , InputFileComboBox );
  Add_New_Item( PEGSfileName.toLatin1().data(),     pegs4ComboBox );
  //checkErrors("UpDateInputRZFile()");
  //checkErrors();
  checkExecutionAbility();
  checkPrintAbility();
  checkPreviewRZ();
}


MInputRZ* inputRZImpl::GetInputRZ()
{
      MInputRZ* finp = new MInputRZ;

      finp->mTitle= GetTitle();
      finp->mIO   = GetIO();
      finp->mMC   = GetMC();
      finp->mGEO  = GetGEO();geoErrors=finp->mGEO->errors;formErrors+=geoErrors;
      finp->mPHD  = GetPHD();
      finp->mCAV  = GetCAV();
      finp->mSRC  = GetSRC();
      finp->mMCP  = GetMCP();
      finp->mPGLS = GetPEGSLESS();
      finp->mVAR  = GetVAR();
      finp->mPLOT = GetPLOT();
      finp->SetUserCode( usercode );

      return finp;
}

MTitle* inputRZImpl::GetTitle()
{
	MTitle* EGStitle = new MTitle;
	EGStitle->str  = TitleEdit->text();
	return EGStitle;
}

MIOInputs* inputRZImpl::GetIO()
{
	MIOInputs* EGSio = new MIOInputs;

	QRadioButton *rb[3] ;
	rb[0] = noRandRadioButton ;
	rb[1] = lastRandRadioButton ;
	rb[2] = allRandRadioButton ;

	EGSio->iwatch     = iwatchComboBox->currentText();
	EGSio->strnd      = TextRadioBChecked(3, rb);
	EGSio->irestart   = irestartComboBox->currentText();
	EGSio->strdat     = (storeDataCheckBox->isChecked()) ? "yes" : "no";
	EGSio->outopt     = outoptComboBox->currentText ();
	EGSio->etransport = etransportComboBox->currentText ();
 	EGSio->sproutopt  = sproutComboBox->currentText ();
	EGSio->dosereg.minPln  = minPlnSpinBox->value();
	EGSio->dosereg.maxPln  = maxPlnSpinBox->value();
	EGSio->dosereg.minCyl  = minCylSpinBox->value();
	EGSio->dosereg.maxCyl  = maxCylSpinBox->value();
 	get_col_content(0,sproutTable,EGSio->sproutreg.start_cyl);
	get_col_content(1,sproutTable,EGSio->sproutreg.stop_slab);

  	if ( usercode == flurznrc) {
	   EGSio->printFluSpec = PrintFluSpeComboBox->currentText();

	   if ( EGSio->printFluSpec.toLower() == "specified" ){
          	     get_col_content( 0, ListFluTable, EGSio->ListFluStart );
          	     get_col_content( 1, ListFluTable, EGSio->ListFluStop );
                 }

  	   EGSio->iprimary = IPRIMARYComboBox->currentText();

                 EGSio->slote = sloteFluEdit->text();

  	   if ( EGSio->slote.toFloat( 0 ) == 0.0f ) {
          	     get_col_content( 0, sloteFluTable, EGSio->bintop );
     	   }
  	}

  //delete[] rb;

	return EGSio;
}

MMCInputs* inputRZImpl::GetMC()
{
	MMCInputs* EGSmc = new MMCInputs;

	EGSmc->ncase   = ncaseEdit->text();
	EGSmc->maxcpu  = maxCPUEdit->text();
	EGSmc->ifull   = ifullComboBox->currentText();
	EGSmc->stat    = statEdit->text();
	EGSmc->kerma   = (kermaCheckBox->isChecked()) ? "yes" : "no";
	EGSmc->photreg = (photregCheckBox->isChecked()) ? "yes" : "no";
	EGSmc->rnd.clear();
	EGSmc->rnd.push_back(rand1SpinBox->value());
	EGSmc->rnd.push_back(rand2SpinBox->value());

	return EGSmc;
}

MGEOInputs* inputRZImpl::GetGEO()
{

 MGEOInputs* EGSgeo = new MGEOInputs;

 QRadioButton *rb[3] ;
 rb[0] = groupRadioButton ;
 rb[1] = individualRadioButton ;
 rb[2] = cavityRadioButton ;

 EGSgeo->inp_meth       = TextRadioBChecked(3, rb);
 // this is to solve a discrepancy between CAVRZnrc and this GUI
 if ( EGSgeo->inp_meth.toLower() == "cavity description" )
     EGSgeo->inp_meth = "cavity information";
 EGSgeo->zface          = ZFaceEdit->text();

 if ( EGSgeo->inp_meth.toLower() == "groups" ){
    get_col_content( 0, geometryTable, EGSgeo->nslab );
    get_col_content( 1, geometryTable, EGSgeo->slabs );
 }
 else{
    get_col_content( 0, geometryTable, EGSgeo->slabs );
 }

 get_col_content( 0, cylTable, EGSgeo->radii );

 v_string medium;
 // Use the current value, not the new value in the combo box
 EGSgeo->description_by = current_description_by;
 get_col_explicit( 0, mediaTable, medium, string("NO MEDIUM") );
//  medium.insert( medium.begin(), "VACUUM");//<=== zero element in media list
 if ( current_description_by.toLower() == "regions" ) {
    get_col_explicit( 1, mediaTable, EGSgeo->start_reg,0 );
    get_col_explicit( 2, mediaTable, EGSgeo->stop_reg,0 );
 }
 else {
    get_col_explicit( 1, mediaTable, EGSgeo->start_Z,0 );
    get_col_explicit( 2, mediaTable, EGSgeo->stop_Z,0 );
    get_col_explicit( 3, mediaTable, EGSgeo->start_ring,0 );
    get_col_explicit( 4, mediaTable, EGSgeo->stop_ring,0 );

 }

 //rearrange media and regions and eliminate NO MEDIUM regions and ZERO regions
 rearrange_media( EGSgeo, &medium );
 if ( current_description_by.toLower() == "regions" ) {
    //resize to avoid media without assigned regions, i.e., in sprrznrc
    if ( EGSgeo->mednum.size() > EGSgeo->start_reg.size() )
       EGSgeo->mednum.resize( EGSgeo->start_reg.size(), 0 );
    if (EGSgeo->start_reg.size() != EGSgeo->stop_reg.size() )
        EGSgeo->errors += "Wrong region assignment, start-stop region mismatch. <br>";
    if (EGSgeo->start_reg.size() != EGSgeo->mednum.size() )
       EGSgeo->errors += "Wrong medium-region assignment. <br> # of regions = "     +
                         QString::number(EGSgeo->start_reg.size(),10) + "<br>" +
                         QString("mednum size = ")+ QString::number(EGSgeo->mednum.size(),10) + "<br>";
 }
 else{
    //resize to avoid media without assigned regions, i.e., in sprrznrc
    if ( EGSgeo->mednum.size() > EGSgeo->start_Z.size() )
       EGSgeo->mednum.resize( EGSgeo->start_Z.size(), 0 );
    if ( EGSgeo->start_Z.size() != EGSgeo->stop_Z.size() )
        EGSgeo->errors += "Wrong region assignment, start-stop ring region mismatch.<br>";
    if ( EGSgeo->start_ring.size() != EGSgeo->stop_ring.size() )
        EGSgeo->errors += "Wrong region assignment, start-stop ring region mismatch.<br>";
    if (  EGSgeo->start_Z.size() != EGSgeo->start_ring.size() )
        EGSgeo->errors += "Same number of inputs for planes and rings needed. <br>";
    if (EGSgeo->start_Z.size() > EGSgeo->mednum.size() ||
        EGSgeo->start_ring.size() > EGSgeo->mednum.size())
       EGSgeo->errors += "Wrong medium-region assignment. <br> # of planes = "     +
                         QString::number(EGSgeo->start_Z.size(),10) + "<br>" +
                         QString("# of rings = ")+ QString::number(EGSgeo->start_ring.size(),10) + "<br>"+
                         QString("mednum size = ")+ QString::number(EGSgeo->mednum.size(),10) + "<br>";
 }

 if ( ! EGSgeo->errors.isEmpty() )
        EGSgeo->errors = "***  Error in GEOMETRY *** <br>" + EGSgeo->errors;

 EGSgeo->mapRegions();
 // Time to update this
 EGSgeo->description_by = mediaComboBox->currentText();
 current_description_by = EGSgeo->description_by;

 return EGSgeo;

}

//!  Obtains media list and sets medium number for regions or planes-cylinders
/*!
The media assignment table can be filled out in an arbitrary (flexible) manner.
- Get media list from column 0 of media table. Strip repetitions
- Independently, create a list with media names and corresponding
  regions/planes-cylinders with non-zero entries.
- Get corresponding medium number from media list
It makes sure that empty cells between valid entries are
taken into account.
*/
void inputRZImpl::rearrange_media( MGEOInputs* geo, v_string* med )
{
v_string tmpMed   = *med;
v_string medium;

//-----------------------------------------
//        create media list
//-----------------------------------------
bool has_vacuum = false;
for ( uint k = 0; k < tmpMed.size() ; k++){
  if ( tmpMed[k] != "NO MEDIUM")
       medium.push_back( tmpMed[k] );
  if ( tmpMed[k] == "VACUUM") has_vacuum = true;
  if ( tmpMed[k] == "vacuum") has_vacuum = true;
}
if (!has_vacuum) medium.insert( medium.begin(), "VACUUM");//<=== zero element in media list
//remove duplicates
geo->media  = strip_repetitions( medium );
//---------------------------------------

v_int tmpStart, tmpStop, tmpStartCyl, tmpStopCyl;
if ( current_description_by.toLower() == "regions" ) {
   tmpStart = geo->start_reg; tmpStop = geo->stop_reg;
   geo->start_reg.clear(); geo->stop_reg.clear();
}
else{
   tmpStart = geo->start_Z; tmpStop = geo->stop_Z;
   tmpStartCyl = geo->start_ring; tmpStopCyl = geo->stop_ring;
   geo->start_Z.clear(); geo->stop_Z.clear();
   geo->start_ring.clear(); geo->stop_ring.clear();
}

//-----------------------------------------
// get map of regions and their medium
//-----------------------------------------
med->clear();

for ( uint i = 0; i < tmpMed.size() ; i++){
  if ( tmpMed[i] != "NO MEDIUM")
          med->push_back( tmpMed[i] );
}
for ( uint i = 0; i < tmpStart.size() ; i++){
     if ( tmpStart[i] != 0 && tmpStop[i] != 0 ){ // non-zero start and stop reg/planes
       //if ( geo->description_by.toLower() == "regions" ) { // description by regions
       if ( current_description_by.toLower() == "regions" ) { // description by regions
          geo->start_reg.push_back( tmpStart[i] );
          geo->stop_reg.push_back( tmpStop[i] );
       }
       else{
        if ( tmpStartCyl[i] != 0 && tmpStopCyl[i] != 0 ){// non-zero start and stop cylinders
             geo->start_Z.push_back( tmpStart[i] );
             geo->stop_Z.push_back( tmpStop[i] );
             geo->start_ring.push_back( tmpStartCyl[i] );
             geo->stop_ring.push_back( tmpStopCyl[i] );
        }
       }
     }
}

//-----------------------------------------
//get corresponding media numbers
//-----------------------------------------
geo->mednum = assign_medium_number( geo->media, *med );

}


MPHDInputs* inputRZImpl::GetPHD()
{
	MPHDInputs* EGSphd = new MPHDInputs;

	get_col_content( 0, phdTable, EGSphd->sreg );
	EGSphd->slote  = SLOTEEdit->text() ;
	EGSphd->deltae = DELTAEEdit->text() ;
	get_col_content( 1, phdTable, EGSphd->bintop );

	return EGSphd;
}


MCAVInputs* inputRZImpl::GetCAV()
{

	MCAVInputs* EGScav = new MCAVInputs;
              if ( usercode != cavrznrc ) return EGScav;
	if ( gr_indGroupBox->isEnabled() ) {
	//if (EGSgeo->inp_meth != "cavity information") {
	   get_col_content( 0, cavTable, EGScav->cav_reg );
	   EGScav->ncavreg = EGScav->ncavreg.setNum( EGScav->cav_reg.size(),10 );
	}
	else if ( cavityGroupBox->isEnabled() ) {
	   EGScav->wall_thick = wallthickEdit->text();
	   EGScav->cav_rad    = cavradEdit->text();
	   EGScav->cav_len    = cavlenEdit->text();
	   EGScav->electr_rad = electradEdit->text();
	   //EGScav->wall_mat   = wallmaterialEdit->text();
	   //EGScav->electr_mat = electrmatEdit->text();
	   EGScav->wall_mat   = wallmaterialComboBox->currentText();
	   EGScav->electr_mat = electrmatComboBox->currentText();
	}
	else {
	   cout << "Error getting info for cavity!!!!" << endl;
	}
	return EGScav;

}


MSRCInputs* inputRZImpl::GetSRC()
{
  MSRCInputs* EGSsrc = new MSRCInputs;
  QRadioButton *rb[5] ;

  EGSsrc->isource   = sourceComboBox->currentText();

  EGSsrc->srcopt.clear();

  if ( EGSsrc->isource == "21" || EGSsrc->isource == "22" ){
    if ( imodeComboBox->currentIndex() == 0 )
         EGSsrc->srcopt.push_back( 0 );
    else
         EGSsrc->srcopt.push_back( 2 );
  }
  else if ( EGSsrc->isource == "23" ) {
    EGSsrc->srcopt.push_back((beamDlg->beam_dist()).toFloat(0));
    EGSsrc->srcopt.push_back((beamDlg->beam_angle()).toFloat(0));
    EGSsrc->srcopt.push_back((beamDlg->beam_zoffset()).toFloat(0));
    EGSsrc->srcopt.push_back((beamDlg->beam_xoffset()).toFloat(0));
    EGSsrc->srcopt.push_back((beamDlg->beam_yoffset()).toFloat(0));
  }
  else
    EGSsrc->srcopt.push_back( (temp1Edit->text() ).toFloat( 0 ) );

  EGSsrc->srcopt.push_back( (temp2Edit->text() ).toFloat( 0 ) );
  EGSsrc->srcopt.push_back( (temp3Edit->text() ).toFloat( 0 ) );
  EGSsrc->srcopt.push_back( (temp4Edit->text() ).toFloat( 0 ) );

  if ( EGSsrc->isource == "22" ){// got more options !
     EGSsrc->srcopt.push_back( (temp5Edit->text() ).toFloat( 0 ) );
     EGSsrc->srcopt.push_back( (temp6Edit->text() ).toFloat( 0 ) );
     EGSsrc->srcopt.push_back( (temp7Edit->text() ).toFloat( 0 ) );
  }

  rb[0] = eRadioButton ;
  rb[1] = phRadioButton ;
  rb[2] = pRadioButton ;
  rb[3] = chargedRadioButton ;
  rb[4] = allRadioButton ;
  EGSsrc->iniparticle = TextRadioBChecked(5, rb);

  rb[0] = monoenergeticRadioButton ;
  rb[1] = spectrumRadioButton ;
  EGSsrc->typenergy = TextRadioBChecked(2, rb);

  if ( EGSsrc->typenergy.toLower() == "monoenergetic" ) {
   EGSsrc->inienergy = ini_energyEdit->text();
  }
  else {
   EGSsrc->spe_file=ironIt(SPECdir          +
                           QDir::separator()+
                           specfnameComboBox->currentText() );
   EGSsrc->speciout = (ioutspCheckBox->isChecked()) ? "include" : "none";
  }

  if ( EGSsrc->isource == "20" ) {
     rb[0] = localRadioButton ;
     rb[1] = externalRadioButton ;
     EGSsrc->modein = TextRadioBChecked(2, rb);

     if ( EGSsrc->modein.toLower() == "external" ) {
      EGSsrc->dist_file = ironIt( RDISTdir + QDir::separator() +
                                 raddistfnameComboBox->currentText());
      EGSsrc->rdistiout = (ioutrdistCheckBox->isChecked()) ? "include" : "none";
     }
     else {
        get_col_content( 0, raddistTable, EGSsrc->rdistf );
        get_col_content( 1, raddistTable, EGSsrc->rpdf );
        EGSsrc->nrdist = EGSsrc->nrdist.setNum( EGSsrc->rdistf.size(),10 );
     }
  }

  if ( ( EGSsrc->isource == "21" ) || ( EGSsrc->isource == "22" ) ) {
    if (phasespaceComboBox->currentText().lastIndexOf( QDir::separator() )<0){
        EGSsrc->phsp_file = ironIt(PHSPdir + QDir::separator() +
                            phasespaceComboBox->currentText());
    }
    else{
        EGSsrc->phsp_file = phasespaceComboBox->currentText();
    }
  }

  if ( EGSsrc->isource == "23" ) {
      EGSsrc->beam_code = beamDlg->beam_code();
      EGSsrc->inp_file  = beamDlg->beam_inp();
      EGSsrc->pegs_file = beamDlg->beam_pegs();
      EGSsrc->weight_win.clear();
      EGSsrc->weight_win.push_back((beamDlg->beam_min()).toFloat(0));
      EGSsrc->weight_win.push_back((beamDlg->beam_max()).toFloat(0));
  }

  //delete[] rb;
  return EGSsrc;
}

PEGSLESSInputs* inputRZImpl::GetPEGSLESS()
{

        int ind = inpmediumComboBox->currentIndex();
        //first save data for the medium currently defined in the form
        if(ind<Ppgls->ninpmedia) {
          //only save if we are not at define new medium
          Ppgls->inpmedium[ind]=inpmediumComboBox->currentText();
          Ppgls->spec_by_pz[ind]=true;
          if(rhozRadioButton->isChecked()) Ppgls->spec_by_pz[ind]=false;
          //see if any elements are present in the table
          int nrow=0;
          int nelements=0;
          while(pz_or_rhozTable->item(nrow,0)!=0 && nrow < 12) {
             if(nrow==0) {
             //clear existing list of elements
               Ppgls->elements[ind].clear();
               Ppgls->pz_or_rhoz[ind].clear();
             }
             //qt3to4 -- BW
             //Ppgls->elements[ind].push_back(pz_or_rhozTable->text(nrow,0));
             //Ppgls->pz_or_rhoz[ind].push_back(pz_or_rhozTable->text(nrow,1));
           if(pz_or_rhozTable->item(nrow,0))
             Ppgls->elements[ind].push_back(pz_or_rhozTable->item(nrow,0)->text().toStdString());
           if(pz_or_rhozTable->item(nrow,1))
             Ppgls->pz_or_rhoz[ind].push_back(pz_or_rhozTable->item(nrow,1)->text().toStdString());
             nelements++;
             nrow++;
          }
          Ppgls->nelements[ind]=nelements;
          //now save other data
          Ppgls->rho[ind]=rhoEdit->text();
          Ppgls->spr[ind]=spComboBox->currentText();
          Ppgls->bc[ind]=bcComboBox->currentText();
          Ppgls->gasp[ind]=gaspEdit->text();
          Ppgls->isgas[ind]=isGasCheckBox->isChecked();
          if(Ppgls->isgas[ind] &&
            (Ppgls->gasp[ind]=="" || Ppgls->gasp[ind].toFloat()<=0.0)) Ppgls->gasp[ind]="1.0";
          gaspEdit->setText(Ppgls->gasp[ind]);
          Ppgls->dffile[ind]=DFEdit->text();
          Ppgls->sterncid[ind]=sterncidEdit->text();
        }

        PEGSLESSInputs* EGSpgls = new PEGSLESSInputs;

        EGSpgls->AE = AEEdit->text();
        EGSpgls->UE = UEEdit->text();
        EGSpgls->AP = APEdit->text();
        EGSpgls->UP = UPEdit->text();
        EGSpgls->matdatafile = MDFEdit->text();

       //now go through and copy the data for media specified in the input file
       //from Ppgls, skip blank media and media that start with a blank character
        for(int i=0; i<Ppgls->ninpmedia; i++) {
           if(Ppgls->inpmedium[i]!="" || Ppgls->inpmedium[i].indexOf(" ",0)==0) {
              EGSpgls->inpmedium[EGSpgls->ninpmedia]=Ppgls->inpmedium[i];
              EGSpgls->nelements[EGSpgls->ninpmedia]=Ppgls->nelements[i];
              EGSpgls->spec_by_pz[EGSpgls->ninpmedia]=Ppgls->spec_by_pz[i];
              for(int j=0; j<Ppgls->nelements[i];j++) {
                EGSpgls->elements[EGSpgls->ninpmedia].push_back(Ppgls->elements[i][j]);
                EGSpgls->pz_or_rhoz[EGSpgls->ninpmedia].push_back(Ppgls->pz_or_rhoz[i][j]);
              }
              EGSpgls->rho[EGSpgls->ninpmedia]=Ppgls->rho[i];
              EGSpgls->spr[EGSpgls->ninpmedia]=Ppgls->spr[i];
              EGSpgls->bc[EGSpgls->ninpmedia]=Ppgls->bc[i];
              EGSpgls->gasp[EGSpgls->ninpmedia]=Ppgls->gasp[i];
              EGSpgls->isgas[EGSpgls->ninpmedia]=Ppgls->isgas[i];
              EGSpgls->dffile[EGSpgls->ninpmedia]=Ppgls->dffile[i];
              EGSpgls->sterncid[EGSpgls->ninpmedia]=Ppgls->sterncid[i];
              EGSpgls->ninpmedia++; //ninpmedia starts at 0
           }
        }

        return EGSpgls;
}

MMCPInputs* inputRZImpl::GetMCP()
{
	MMCPInputs* EGSmcp = new MMCPInputs;
	QRadioButton *rb[5] ;

  	EGSmcp->GECUT         = GECUTEdit->text();
  	EGSmcp->GPCUT         = GPCUTEdit->text();
  	EGSmcp->GSMAX         = GSMAXEdit->text();
  	EGSmcp->ESTEPE        = ESTEPEEdit->text();
  	EGSmcp->XImax         = XImaxEdit->text();
  	EGSmcp->SkinD         = SkinDepthEdit->text();
  	EGSmcp->BoundXAlg     = BCAComboBox->currentText();
  	EGSmcp->ESTEPAlg      = estep_algorithmComboBox->currentText();
  	EGSmcp->Spin          = ( SpinCheckBox->isChecked() ) ? "on" : "off";
  	EGSmcp->BremsSampling = BremsAngSamplingComboBox->currentText();
  	EGSmcp->BremsXSection = BremsXSectionComboBox->currentText();
  	EGSmcp->PairSampling  = PairAngSamplingComboBox->currentText();
  	EGSmcp->PhotXSection  = photonXSectioncomboBox->currentText();
  	EGSmcp->EIIXSection   = EIIcomboBox->currentText();
  	EGSmcp->PhotXSectionOut=
        ( photonXSectionOutCheckBox->isChecked() ) ? "on" : "off";

	EGSmcp->BC = BoundComptoncomboBox->currentText();
  	if ( ( EGSmcp->BC.toLower() == "on in regions" ) ||
       	     ( EGSmcp->BC.toLower() == "off in regions" ) ) {
       	       get_col_content( 0, BoundComptonTable, EGSmcp->BCstart );
       	       get_col_content( 1, BoundComptonTable, EGSmcp->BCstop  );
  	}

	EGSmcp->PE = PEcomboBox->currentText();
  	if ( ( EGSmcp->PE.toLower() == "on in regions" ) ||
       	     ( EGSmcp->PE.toLower() == "off in regions" ) ) {
       	       get_col_content( 0, PEAngSamplingTable, EGSmcp->PEstart );
       	       get_col_content( 1, PEAngSamplingTable, EGSmcp->PEstop  );
  	}

	EGSmcp->RAY = RayleighcomboBox->currentText();
  	if ( ( EGSmcp->RAY.toLower() == "on in regions" ) ||
       	     ( EGSmcp->RAY.toLower() == "off in regions" ) ) {
       	       get_col_content( 0, RayleighTable, EGSmcp->RAYstart );
       	       get_col_content( 1, RayleighTable, EGSmcp->RAYstop  );
  	}
        else if (EGSmcp->RAY.toLower() == "custom"){
       	       get_col_content( 0, customFFTable, EGSmcp->ff_media );
       	       get_col_content( 1, customFFTable, EGSmcp->ff_files  );
        }

	EGSmcp->RELAX = RelaxationcomboBox->currentText();
  	if ( ( EGSmcp->RELAX.toLower() == "on in regions" ) ||
       	     ( EGSmcp->RELAX.toLower() == "off in regions" ) ) {
       	       get_col_content( 0, RelaxationsTable, EGSmcp->RELAXstart );
       	       get_col_content( 1, RelaxationsTable, EGSmcp->RELAXstop  );
  	}

 	EGSmcp->SetPCUTbyRegion = ( PCUTCheckBox->isChecked() ) ? true : false;
  	if ( EGSmcp->SetPCUTbyRegion ) {
     	     get_col_content( 0, PCUTTable, EGSmcp->PCUT );
     	     get_col_content( 1, PCUTTable, EGSmcp->PCUTstart );
     	     get_col_content( 2, PCUTTable, EGSmcp->PCUTstop  );
  	}

  	EGSmcp->SetECUTbyRegion = ( ECUTCheckBox->isChecked() ) ? true : false;
  	if ( EGSmcp->SetECUTbyRegion ) {
     	     get_col_content( 0, ECUTTable, EGSmcp->ECUT );
     	     get_col_content( 1, ECUTTable, EGSmcp->ECUTstart );
     	     get_col_content( 2, ECUTTable, EGSmcp->ECUTstop  );
  	}

  	EGSmcp->SetSMAXbyRegion = ( SMAXCheckBox->isChecked() ) ? true : false;
  	if ( EGSmcp->SetSMAXbyRegion ) {
     	     get_col_content( 0, SMAXTable, EGSmcp->SMAX );
     	     get_col_content( 1, SMAXTable, EGSmcp->SMAXstart );
     	     get_col_content( 2, SMAXTable, EGSmcp->SMAXstop  );
  	}

  //delete[] rb;

	return EGSmcp;
}


MVARInputs* inputRZImpl::GetVAR()
{
	MVARInputs* EGSvar = new MVARInputs;
  	if ( ( usercode == dosrznrc ) ||
       	     ( usercode == flurznrc ) ){
     	     EGSvar->BremsSplitting = ( BremsSplitCheckBox->isChecked() ) ? "on" : "off";
     	     EGSvar->nBrems         = BremsSplitSpinBox->value();
     	     EGSvar->chargedPartRR  = ( ChargedPartRRCheckBox->isChecked() ) ? "on" : "off";
  	}

  	EGSvar->eRangeRej      = ( eRangeRejCheckBox->isChecked() ) ? "on" : "off";
  	EGSvar->ESAVEIN        = ESAVEINEdit->text();

  	if ( usercode != flurznrc ) {
     	     EGSvar->RRDepth        = RRDepthEdit->text();    // flurznrc wont use this
     	     EGSvar->RRFraction     = RRFractionEdit->text(); // flurznrc wont use this
     	     EGSvar->ExpoTrafoC     = ExpTrafoCEdit->text(); // flurznrc wont use this
  	}

  	EGSvar->PhotonForcing  = ( PhotonForcingCheckBox->isChecked() ) ? "on" : "off";
  	EGSvar->startForcing   = StartForcingSpinBox->value();
  	EGSvar->stopForcing    = StopForcingSpinBox->value();

  	if ( ( usercode == dosrznrc ) ||
       	     ( usercode == cavrznrc ) ) { // only implemented in these user codes
     	     EGSvar->CSEnhancement  = CSEnhancementSpinBox->value();
             get_col_content( 0, CSEnhancementTable, EGSvar->CSEnhanStart );
             get_col_content( 1, CSEnhancementTable, EGSvar->CSEnhanStop  );
  	}

  	if ( usercode == cavrznrc )
     	     EGSvar->nsplit = photonSplitSpinBox->value();

	return EGSvar;

}

MPLOTInputs* inputRZImpl::GetPLOT()
{

  MPLOTInputs* EGSplot = new MPLOTInputs;
  EGSplot->Plotting  = ( plotCheckBox->isChecked() ) ? "on" : "off";
  if ( EGSplot->Plotting.toLower() == "on" ) {

       if ( usercode == dosrznrc ) {
            EGSplot->LinePrnOut = ( LinePrnOutCheckBox->isChecked() ) ? "on" : "off";
            EGSplot->ExtPlotOut = ( ExtPlotOutCheckBox->isChecked() ) ? "on" : "off";
       }

       if ( ( ( EGSplot->ExtPlotOut.toLower() == "on" ) &&
              ( usercode == dosrznrc ) ) ||
              ( usercode == flurznrc )   ){
     	      EGSplot->ExtPlotType = ExternalPlotTypeComboBox->currentText();
       }

       get_col_content( 0, PlotRegionsTable, EGSplot->PlotIX );
       get_col_content( 1, PlotRegionsTable, EGSplot->PlotIZ );

       if ( usercode == flurznrc ) {
          //EGSplot->DrawFluPlot = DrawFluPlotsComboBox->currentText();
          QString strDraw = DrawFluPlotsComboBox->currentText();
          QString all = "all";
          EGSplot->DrawFluPlot = (strDraw == "Primaries and total") ? all : strDraw;
          EGSplot->eminusPlot =  ( eminusPlotCheckBox->isChecked() ) ? "on" : "off";
          EGSplot->eplusPlot  =  ( eplusPlotCheckBox->isChecked() ) ? "on" : "off";
          EGSplot->ePlot      =  ( ePlotCheckBox->isChecked() ) ? "on" : "off";
          EGSplot->gPlot      =  ( gammaPlotCheckBox->isChecked() ) ? "on" : "off";

          get_col_content( 0, SpecPlotTable, EGSplot->SpecPlotStart );
          get_col_content( 1, SpecPlotTable, EGSplot->SpecPlotStop );

       }
  }

  return EGSplot;
}

void inputRZImpl::update_caption( const QString& str )
{
   QString StdCaption   = "GUI for RZ EGSnrc user codes.";
           StdCaption +=  tr(" Copyright ") + the_year + tr(" NRC Canada");
   QString Caption = "[" + str + "] "+ StdCaption;
   this->setWindowTitle( Caption );
}

void inputRZImpl::save()
{

    QFile f( EGSdir + EGSfileName );
    if ( !f.open( QIODevice::WriteOnly ) ) {
        QString errStr = "Could not open file "  + EGSdir + EGSfileName;
	       errStr += " to write !!!";
	       errStr += "\nPlease check file settings and try again!";
        QMessageBox::information( this, " Error!!!",errStr, QMessageBox::Ok );
        return;
    }

    update_caption( EGSfileName );

    //qt3to4 -- BW
    //Q3TextStream t( &f );
    QTextStream t( &f );

    MInputRZ * inp = new MInputRZ;
               inp = GetInputRZ();

    t << inp;

    f.close();

    //delete inp;
    zap(inp);

}
