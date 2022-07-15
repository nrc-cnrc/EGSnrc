/*
###############################################################################
#
#  EGSnrc egs_inprz form
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
#  Contributors:    Iwan Kawrakow
#                   Frederic Tessier
#                   Blake Walters
#                   Cody Crewson
#                   Reid Townson
#
###############################################################################
*/

#include <QtGlobal>
#include "tooltips.h"
#include "inputRZImpl.h"
#include "errordlg.h"
#include "commandManager.h"
#include "executiondlgImpl.h"
#include "eventfilter.h"
#include "pegslessinputs.h"
#include <qevent.h>
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
#include <QPixmap>
#include <iterator>
#include <qtabwidget.h>
#include <qpushbutton.h>
#include <qdir.h>
//#include <q3progressdialog.h>
//#include <q3buttongroup.h>
#include <qwidget.h>
#include <qpainter.h>
//#include <q3paintdevicemetrics.h>
#include <qprinter.h>
#include <qvalidator.h>
#include <typeinfo>
//#include <q3whatsthis.h>
//#include <q3process.h>
#include <qdatetime.h>
//#include <qsettings.h>
//#include <qlibrary.h>

#include <QProcess>

#define PIRS801 "pirs801.html"

//**********************************
//***** FORM INITIALIZATION   *****
//**********************************
void inputRZImpl::Initialize()
{

//Initialize the variables with defaults

 the_year = (QDate::currentDate()).toString("yyyy");

 this->setStyleSheet("QToolTip { color: black; background-color: #feffcd }");

//  usercode     = cavrznrc;
  usercode     = dosrznrc;
//  usercodename = "cavrznrc";
  usercodename = "dosrznrc";
  EGSfileName  = "dosrznrc_template.egsinp";
  PEGSfileName = "521icru.pegs4dat";


confErrors   = "";
formErrors   =  "";
openErrors  =  "";
pegsErrors  =  "";
geoErrors    =  "";

  SetInitialDir();
  QString confiname = EGS_CONFIG;

  //****************************
  // Loading NRC logo image file
  //****************************
  QPixmap* pm = new QPixmap(HEN_HOUSE+"/gui/egs_inprz/images/nrc-ribbon.png");
  bannerLabel->setPixmap(*pm);
  bannerLabel->setScaledContents(true);

  //******************************
  // Loading little guy image file
  //******************************
  pm->load(HEN_HOUSE+"gui/egs_inprz/images/bianchi.png");
  theguyLabel->setPixmap(*pm);
  theguyLabel->setScaledContents(true);

#ifndef WIN32
  //******************************
  // Loading icon file
  //******************************
  pm->load(HEN_HOUSE+"gui/egs_inprz/images/desktop_icon.png");
  setWindowIcon(*pm);
  setWindowIconText("egs_inprz");
#endif

 //*****************************************
 // Disabling button to start configuration
 // GUI. Feature deprecated here and only
 // available in egs_gui and egs_install.
 // We'll see how many users notice !!!!
 //*****************************************
 ConfigurationButton->hide();

/*
  if ( ! confiname.isEmpty() ){
     CONFcomboBox->setCurrentText(
     confiname.remove(0, 1+ confiname.lastIndexOf( QDir::separator()) ) );
  }
*/
  connect( CONFcomboBox, SIGNAL( editTextChanged(const QString&) ),
           this,         SLOT( checkConfigLib() ) );

  //checkCompilationAbility();

// The condition below is now checked in slot checkPreviewRZ()
//  previewRZ_exists = fileExists( HEN_HOUSE + "/previewRZ/previewRZ.tcl" ) &&
//                     isTclTkInstalled();

  //Initialize table defining regions to list fluence
  InitializeTwoColumnTable( ListFluTable );
  ListFluTable->setEnabled( false );

  //Initialize table defining tops of output bins in MeV
  sloteFluTable->setHorizontalHeaderItem(0,new QTableWidgetItem("bin top [MeV]"));

  //Initialize the stopping power output mode table
  InitializeTable( sproutTable, "start region", "stop region" );
  sproutTable->resizeColumnsToContents();

  //Initialize the pulse height distribution table
  InitializeTable( phdTable, "sensitive region", "bin top [MeV]" );
  //Initialize media table
  QStringList smed; smed << "medium" << "start region" << "stop region";
  v_float frmed; frmed.push_back(0.5); frmed.push_back(0.25); frmed.push_back(0.25);
  //InitializeTable( mediaTable, "medium", "start region", "stop region" );
  InitializeTable( mediaTable, smed, frmed);

  //Initialize the pegsless medium table
  InitializeTable( pz_or_rhozTable, "Element", "Fraction by weight" );
  //Initialize FF table
  //customFFTable->setValidator(false);
  QStringList sl; sl.append("medium");sl.append("FF file (full path)");
  v_float fra;
  fra.push_back(0.20); fra.push_back(0.80);
  InitializeTable( customFFTable, sl, fra);
  customFFgroupBox->setEnabled( false );

  //Initialize geometry tables
    globalNSlab.push_back(100);

  QStringList s; s.append( "# slabs" ); s.append("thickness [cm]");
  fra[0]=0.3;fra[1]=0.70;//fra[2]=0.35;
  InitializeTable( geometryTable, s, fra );
  s.clear(); s.append( "radius [cm]" );
  InitializeTable( cylTable,  s );

  //Initialize cavity iformation
  cavTable->setHorizontalHeaderItem(0,new QTableWidgetItem("region #"));

  //Initialize source information
  InitializeTable( raddistTable, "Radial bin top", "Probability");
  phasespaceGroupBox->setToolTip(PHSP_FILE );
 /**************************************/
  /* Initialize beam source information */
  /**************************************/
  beamButton= new QPushButton( "&Setup beam source", sourceoptionsGroupBox );
  beamButton->setGeometry( 30, 250, 250, 30 );
  QString egs_home_bin = EGS_HOME +
                         tr("bin")+ QDir::separator() +
                         readVarFromConf("my_machine");
#ifndef WIN32
  QStringList beamuc = list_items(egs_home_bin,"libBEAM*");
#else
  QStringList beamuc = list_items(egs_home_bin,"BEAM*.dll");
#endif
  for ( QStringList::Iterator it = beamuc.begin();
                              it != beamuc.end(); ++it ) {
        (*it).remove("lib",Qt::CaseSensitive);
        (*it).remove((*it).lastIndexOf("."),(*it).length()-(*it).lastIndexOf("."));
  }
  QString inpDir = !beamuc.isEmpty() ? EGS_HOME + beamuc[0] : EGS_HOME;
  beamuc.prepend(EGS_HOME);
  //give user access to pegs data files in EGS_HOME and HEN_HOUSE
  QChar sep = QDir::separator();
  QString PEGSdir1, PEGSdir2;
  PEGSdir1  = ironIt( EGS_HOME + sep + "pegs4" + sep + "data" + sep);
  PEGSdir2  = ironIt( HEN_HOUSE + sep + "pegs4" + sep + "data" + sep);
  QStringList pegsfiles = strip_extension(
                          list_items(PEGSdir1,"*.pegs4dat")+list_items(PEGSdir2,"*.pegs4dat"),".pegs4dat");
  //get rid of duplicates
  pegsfiles.removeDuplicates();
  QStringList inpfiles = strip_extension( list_items(inpDir,"*.egsinp" ),
                         ".egsinp" );
  beamDlg = new BeamSourceDlg(this,"beam_source",&beamuc,&inpfiles,&pegsfiles);

  connect( beamButton, SIGNAL(clicked()), this, SLOT(setupBeamSource()) );
  connect( beamDlg, SIGNAL(beamSourceDefined()), this, SLOT(getBeamSource()) );
  beamLabel = new QLabel(sourceoptionsGroupBox);
  beamLabel->setGeometry( 20, 20, 300, 230 );
  beamLabel->setText(tr(
              "Click on the button below to enter<br>"
              "source information!<br>"
              "Make sure that a BEAM source has been<br>"
              "built as a library or the combo boxes<br>"
              "will appear empty on the input dialog !<br>"
              "(See PIRS-702 and the BEAM manual<br>"
              "for info on building a source library)"));

  changeTextColor(beamLabel,"red");
  beamDlg->get_beam()->setToolTip(BEAM_CODE  );
  beamDlg->get_inp()->setToolTip(BEAM_INP  );
  beamDlg->get_pegs()->setToolTip(BEAM_PEGS  );
  beamDlg->get_min()->setToolTip(BEAM_MIN_WEIGHT  );
  beamDlg->get_max()->setToolTip(BEAM_MAX_WEIGHT  );
  beamDlg->get_dist()->setToolTip(BEAM_DIST  );
  beamDlg->get_angle()->setToolTip(BEAM_ANGLE  );
  beamDlg->get_zoffset()->setToolTip(BEAM_ZOFFSET  );
  beamDlg->get_xoffset()->setToolTip(BEAM_XOFFSET  );
  beamDlg->get_yoffset()->setToolTip(BEAM_YOFFSET  );

  /******************************************************/

  //Initialize MC transport parameters
  InitializeTwoColumnTable( BoundComptonTable);
  InitializeTwoColumnTable( PEAngSamplingTable);
  InitializeTwoColumnTable( RayleighTable);
  InitializeTwoColumnTable( RelaxationsTable);

  InitializeThreeColumnTable( PCUTTable, "PCUT");
  InitializeThreeColumnTable( ECUTTable, "ECUT");
  InitializeThreeColumnTable( SMAXTable, "SMAX");

// Tool tips for General Tab
  UserCodeAreaButtonGroup->setWhatsThis(USER_CODE_AREA );
  UserCodeAreaButtonGroup->setToolTip(USER_CODE_AREA );

  PEGSDataAreaButtonGroup->setWhatsThis(PEGS4_AREA );
  PEGSDataAreaButtonGroup->setToolTip(PEGS4_AREA );


  OpenFileButton->setWhatsThis(OPEN_INPUT_FILE );
  Pegs4FileButton->setWhatsThis(OPEN_PEGS_FILE );
  OpenFileButton->setToolTip(OPEN_INPUT_FILE );
  Pegs4FileButton->setToolTip(OPEN_PEGS_FILE );

 // Tool tips for I/O Tab
 DoseRegGroupBox->setWhatsThis(DOSE_REGIONS );
 DoseRegGroupBox->setToolTip(DOSE_REGIONS );

 sloteFluEdit->setWhatsThis(SLOTE_FLU );
 sloteFluEdit->setToolTip(SLOTE_FLU );

 sloteFluLabel->setWhatsThis(SLOTE_FLU );
 sloteFluLabel->setToolTip(SLOTE_FLU );

 sloteFluTable->setWhatsThis(TOP_BIN_FLU );
 sloteFluTable->setToolTip(TOP_BIN_FLU );

// Tool tips for MC input Tab
maxCPUEdit->setWhatsThis(CPU_TIME  );
 maxCPULabel->setWhatsThis(CPU_TIME  );

maxCPUEdit->setToolTip(CPU_TIME  );
 maxCPULabel->setToolTip(CPU_TIME  );

statEdit->setWhatsThis(ACCURACY  );
statLabel->setWhatsThis(ACCURACY  );
statEdit->setToolTip(ACCURACY  );
statLabel->setToolTip(ACCURACY  );

randGroupBox->setWhatsThis(RND_GEN  );
randGroupBox->setToolTip(RND_GEN  );

rand1SpinBox->setWhatsThis(RND_1  );
rand1Label->setWhatsThis( RND_1  );
rand1SpinBox->setToolTip(RND_1  );
rand1Label->setToolTip( RND_1  );

rand2SpinBox->setWhatsThis(RND_2  );
rand2Label->setWhatsThis(RND_2  );
rand2SpinBox->setToolTip(RND_2  );
rand2Label->setToolTip(RND_2  );

SLOTEEdit->setWhatsThis(SLOTE  );
SLOTELabel->setWhatsThis(SLOTE  );
SLOTEEdit->setToolTip(SLOTE  );
SLOTELabel->setToolTip(SLOTE  );

DELTAEEdit->setWhatsThis(DELTAE  );
DELTAELabel->setWhatsThis( DELTAE  );
DELTAEEdit->setToolTip(DELTAE  );
DELTAELabel->setToolTip( DELTAE  );

phdTable->setWhatsThis(QString(SENSITIVE_VOL) + "\n" + QString(BIN_TOP)  +
                 QString(INDEPENDENT));
phdTable->setToolTip(QString(SENSITIVE_VOL) + "\n" + QString(BIN_TOP)  +
                 QString(INDEPENDENT));

kermaCheckBox->setWhatsThis(KERMA  );
kermaCheckBox->setToolTip(KERMA  );

photregCheckBox->setWhatsThis(KERMA  );
photregCheckBox->setToolTip(KERMA  );

 // Tool tips for geometry input Tab
groupRadioButton->setWhatsThis(GROUPS  );
individualRadioButton->setWhatsThis(INDIVIDUAL  );
cavityRadioButton->setWhatsThis(CAVITY  );
groupRadioButton->setToolTip(GROUPS  );
individualRadioButton->setToolTip(INDIVIDUAL  );
cavityRadioButton->setToolTip(CAVITY  );

CavityInfoLabel->hide();

// Tool tips for source input Tab

  //tool tip for source number combo box
  sourceComboBox->setToolTip("select source number"  );

imodeComboBox->addItem( tr( "No ZLAST" ) );
imodeComboBox->addItem( tr( "With ZLAST" ) );
imodeComboBox->hide();

localRadioButton->setWhatsThis( RAD_DIS_LOCAL);
externalRadioButton->setWhatsThis(RAD_DIS_EXTERNAL);
localRadioButton->setToolTip( RAD_DIS_LOCAL);
externalRadioButton->setToolTip(RAD_DIS_EXTERNAL);

phasespaceGroupBox->setWhatsThis(PHSP_FILE );
phasespaceGroupBox->setToolTip(PHSP_FILE );

  // Tool tips for Transport Parameters Tab

  GPCUTGroupBox->setWhatsThis(GLOBAL_PCUT );
  GECUTGroupBox->setWhatsThis(GLOBAL_ECUT );
  GSMAXGroupBox->setWhatsThis(GLOBAL_SMAX );
  ESTEPELabel->setWhatsThis(ESTEPE_TIP );
  ESTEPEEdit->setWhatsThis(ESTEPE_TIP );
  XImaxEdit->setWhatsThis(XIMAX_TIP );
  XImaxLabel->setWhatsThis(XIMAX_TIP );
  SkinDepthEdit->setWhatsThis(SKINDEPTH_TIP );
  SkinDepthLabel->setWhatsThis(SKINDEPTH_TIP );
  SpinCheckBox->setWhatsThis(SPIN_EFFECTS );
  BCAGroupBox->setWhatsThis(BCA_TIP );
  PairAngSamplingGroupBox->setWhatsThis(PAIR_ANG_SAMPLING );
  BremsAngSamplingGroupBox->setWhatsThis(BREMS_ANG_SAMPLING );
  BoundComptongroupBox->setWhatsThis(BOUND_COMPTON );
  PEgroupBox->setWhatsThis(PE_ANG_SAMPLING );
  RayleighgroupBox->setWhatsThis(RAYLEIGH_SCAT );
  RelaxationgroupBox->setWhatsThis( RELAXATION_TIP );
  estep_algorithmGroupBox->setWhatsThis(ESTEPE_ALGORITHM );
  BremsXSectionGroupBox->setWhatsThis(BREMS_XSECTION );
  photonXSectiongroupBox->setWhatsThis(PHOTON_XSECTION );
  EIIgroupBox->setWhatsThis(EII_XSECTION );

  GPCUTGroupBox->setToolTip(GLOBAL_PCUT );
  GECUTGroupBox->setToolTip(GLOBAL_ECUT );
  GSMAXGroupBox->setToolTip(GLOBAL_SMAX );
  ESTEPELabel->setToolTip(ESTEPE_TIP );
  ESTEPEEdit->setToolTip(ESTEPE_TIP );
  XImaxEdit->setToolTip(XIMAX_TIP );
  XImaxLabel->setToolTip(XIMAX_TIP );
  SkinDepthEdit->setToolTip(SKINDEPTH_TIP );
  SkinDepthLabel->setToolTip(SKINDEPTH_TIP );
  SpinCheckBox->setToolTip(SPIN_EFFECTS );
  BCAGroupBox->setToolTip(BCA_TIP );
  PairAngSamplingGroupBox->setToolTip(PAIR_ANG_SAMPLING );
  BremsAngSamplingGroupBox->setToolTip(BREMS_ANG_SAMPLING );
  BoundComptongroupBox->setToolTip(BOUND_COMPTON );
  PEgroupBox->setToolTip(PE_ANG_SAMPLING );
  RayleighgroupBox->setToolTip(RAYLEIGH_SCAT );
  RelaxationgroupBox->setToolTip( RELAXATION_TIP );
  estep_algorithmGroupBox->setToolTip(ESTEPE_ALGORITHM );
  BremsXSectionGroupBox->setToolTip(BREMS_XSECTION );
  photonXSectiongroupBox->setToolTip(PHOTON_XSECTION );
  EIIgroupBox->setToolTip(EII_XSECTION );

  PCUTGroupBox->setWindowTitle( PCUTGroupBox->title() );
  ECUTGroupBox->setWindowTitle( ECUTGroupBox->title() );
  SMAXGroupBox ->setWindowTitle(  SMAXGroupBox ->title() );
 BoundComptonGroupBox->setWindowTitle( BoundComptonGroupBox->title() );
 PEAngSamGroupBox->setWindowTitle( PEAngSamGroupBox->title() );
 RayleighGroupBox->setWindowTitle( RayleighGroupBox->title() );
 RelaxationsGroupBox->setWindowTitle( RelaxationsGroupBox->title() );
  //Initialize Variance Reduction

  eRangeRejCheckBox->setWhatsThis(RANGE_REJECTION );
  ESAVEINEdit->setWhatsThis(RR_ESAVEIN );
  ESAVEINLabel->setWhatsThis(RR_ESAVEIN );
  PhotonForcingCheckBox->setWhatsThis(PHOTON_FORCING );
  StartForcingLabel->setWhatsThis(START_PHOTON_FORCING );
  StopForcingLabel->setWhatsThis(STOP_PHOTON_FORCING );
  StartForcingSpinBox->setWhatsThis(START_PHOTON_FORCING );
  StopForcingSpinBox->setWhatsThis(STOP_PHOTON_FORCING );
  ExpTrafoCLabel->setWhatsThis(EXPONENTIAL_TRANSPORT );
  ExpTrafoCEdit->setWhatsThis( EXPONENTIAL_TRANSPORT );
  photonSplitSpinBox->setWhatsThis(PHOTON_SPLITTING );
  photonSplitLabel->setWhatsThis(PHOTON_SPLITTING );
  RRDepthLabel->setWhatsThis(RUSSIAN_ROULETTE_DEPTH );
  RRDepthEdit->setWhatsThis(RUSSIAN_ROULETTE_DEPTH );
  RRFractionLabel->setWhatsThis(RUSSIAN_ROULETTE_FRACTION );
  RRFractionEdit->setWhatsThis(RUSSIAN_ROULETTE_FRACTION );

  eRangeRejCheckBox->setToolTip(RANGE_REJECTION );
  ESAVEINEdit->setToolTip(RR_ESAVEIN );
  ESAVEINLabel->setToolTip(RR_ESAVEIN );
  PhotonForcingCheckBox->setToolTip(PHOTON_FORCING );
  StartForcingLabel->setToolTip(START_PHOTON_FORCING );
  StopForcingLabel->setToolTip(STOP_PHOTON_FORCING );
  StartForcingSpinBox->setToolTip(START_PHOTON_FORCING );
  StopForcingSpinBox->setToolTip(STOP_PHOTON_FORCING );
  ExpTrafoCLabel->setToolTip(EXPONENTIAL_TRANSPORT );
  ExpTrafoCEdit->setToolTip( EXPONENTIAL_TRANSPORT );
  photonSplitSpinBox->setToolTip(PHOTON_SPLITTING );
  photonSplitLabel->setToolTip(PHOTON_SPLITTING );
  RRDepthLabel->setToolTip(RUSSIAN_ROULETTE_DEPTH );
  RRDepthEdit->setToolTip(RUSSIAN_ROULETTE_DEPTH );
  RRFractionLabel->setToolTip(RUSSIAN_ROULETTE_FRACTION );
  RRFractionEdit->setToolTip(RUSSIAN_ROULETTE_FRACTION );

  CSEnhancementGroupBox->setWhatsThis(CS_ENHANCEMENT_CAVRZNRC  );
  CSEnhancementSpinBox->setWhatsThis(CS_ENHANCEMENT_FACTOR  );
  BremsSplitCheckBox->setWhatsThis(BREMS_SPLITTING );
  BremsSplitSpinBox->setWhatsThis(BREMS_SPLITTING_FACTOR );
  BremsSplitTextLabel->setWhatsThis(BREMS_SPLITTING_FACTOR );
  ChargedPartRRCheckBox->setWhatsThis(CHARGED_PARTICLE_RR );

  CSEnhancementGroupBox->setToolTip(CS_ENHANCEMENT_CAVRZNRC  );
  CSEnhancementSpinBox->setToolTip(CS_ENHANCEMENT_FACTOR  );
  BremsSplitCheckBox->setToolTip(BREMS_SPLITTING );
  BremsSplitSpinBox->setToolTip(BREMS_SPLITTING_FACTOR );
  BremsSplitTextLabel->setToolTip(BREMS_SPLITTING_FACTOR );
  ChargedPartRRCheckBox->setToolTip(CHARGED_PARTICLE_RR );

  BremsSplitSpinBox->setEnabled( false );
  BremsSplitTextLabel->setEnabled( false );

  //Initialize the cross section enhancement table
  InitializeTwoColumnTable( CSEnhancementTable );

  //qt3to4 -- BW -- below is redundant and results in zero width columns
  //int fullWidth  = CSEnhancementTable->visibleWidth();
  //QRect vrect = CSEnhancementTable->visibleRegion().boundingRect();
  //int fullWidth = vrect.width();
  //CSEnhancementTable->setColumnWidth(0,  fullWidth/2);
  //CSEnhancementTable->setColumnWidth(1,fullWidth/2);

  //Initialize Plotting Control

  PlotGroupBox->setEnabled( false );

  //Initialize the plot regions table
  InitializeTable( PlotRegionsTable, "radial IX", "planar IZ" );

  PlotRegionsTable->setWhatsThis(QString(RADIAL_PLOT) + "\n" +QString(PLANAR_PLOT) + QString(INDEPENDENT));
  PlotRegionsTable->setToolTip(QString(RADIAL_PLOT) + "\n" +QString(PLANAR_PLOT) + QString(INDEPENDENT));
  //Initialize the plot regions table
  InitializeTable( SpecPlotTable, "start", "stop" );
  //Some tool tips for Media Definition tab

  MDFEdit->setToolTip(MATERIAL_DATA_FILE);
  inpmediaGroupBox->setToolTip(INPMEDIA_DEF);
  inpmediumComboBox->setToolTip(MEDIUM_NAME);

  //event handler for inpmediumComboBox
  im_events = new ComboEvents(inpmediumComboBox,0);
  connect(im_events,SIGNAL(keyReturn(QString)),this,SLOT(inpmediumSave(QString)));

  //Initialize some stuff

  QClipboard *clipBoard = QApplication::clipboard();

  //update_usercode(); <= inside UpDateInputRZForm now !!!!

  EnableTransportParamByRegions();

  InitializePhotonXSection();

  InitializeEIIXSection();

  // Initially try loading dosrznrc_template.egsinp
  // else using defaults
  if ( QFile::exists( EGSdir + EGSfileName ) ){
     SetInpfileName( ironIt(EGSdir + EGSfileName) );
  }
  else{
     EGSfileName  = "new_file.egsinp";
     MInputRZ* Input = new MInputRZ;
     UpDateInputRZForm( Input );
     checkExecutionAbility();
     checkPrintAbility();
     checkPreviewRZ();
     zap(Input);
  }
  //checkConfigLib(); <= this calls updateConfiguration a second
  //                     time.(SetInitialDir calls it first)
  //                     Calls checkExecutionAbility too!
  //Therefore using the code below instead.
/***********************************************
 *   Not provided anymore by egs_inprz
 *                             EMH, 26 March 2013
 ***********************************************/
//   if ( ! configLibExists() ){
//    ConfigurationButton->setEnabled( false );
//   }
//   else{
//     ConfigurationButton->setEnabled( true );
//   }


  caught_errors();


  TabWidgetRZ->setCurrentIndex(0);

}

//****************************************************
// *********            FORM STUFF         ***********
//****************************************************

void inputRZImpl::UpDateInputRZForm( const MInputRZ* Input )
{
	//Transfer info from EGS input file to form

	//...user code

   	switch ( usercode ) {
     	case cavrznrc:
                cavrzRadioButton->setChecked( true );
                usercodename   = "cavrznrc";
	      	break;
     	case dosrznrc:
       	        dosrzRadioButton->setChecked( true );
	        usercodename   = "dosrznrc";
       	        break;
     	case sprrznrc:
	        sprrzRadioButton->setChecked( true );
	        usercodename   = "sprrznrc";
	        break;
     	case flurznrc:
	        flurzRadioButton->setChecked( true );
	        usercodename   = "flurznrc";
	        break;
	}

	// extract available media from pegs4 data file to a list
        listMedia = getPEGSMedia( PEGSdir + PEGSfileName );
	// pass media  list to media table to update combo box editor content

	wallmaterialComboBox->addItems( StrListToQStrList( listMedia ) );
	electrmatComboBox->addItems( StrListToQStrList( listMedia ) );

        if ( EGSfileName != InputFileComboBox->currentText() ) {
             disconnect(InputFileComboBox,SIGNAL(editTextChanged(const QString&)),
                        this, SLOT( EGSFileNameChanged(const QString&) ) );
             InputFileComboBox->setEditText ( EGSfileName );
	     Add_New_Item( EGSfileName.toLatin1().data(), InputFileComboBox );
             connect(InputFileComboBox,SIGNAL(editTextChanged(const QString&)),
                     this, SLOT( EGSFileNameChanged(const QString&) ) );
	}

	if ( egs_dir_changed ) // add list of egsinp files
        {
              disconnect(InputFileComboBox,SIGNAL(editTextChanged(const QString&) ),
                         this, SLOT( EGSFileNameChanged(const QString&) ) );
              update_files( EGSdir, InputFileComboBox, "*.egsinp" );
              connect( InputFileComboBox, SIGNAL( editTextChanged(const QString&) ),
                       this, SLOT( EGSFileNameChanged(const QString&) ) );
         }

         if ( PEGSfileName != pegs4ComboBox->currentText() ) {
 	    pegs4ComboBox->setEditText ( PEGSfileName );/*produces a call
                                                          to checkErrors()*/
//--------- ^
	    Add_New_Item( PEGSfileName.toLatin1().data(), pegs4ComboBox );
         }

         set_data_area();

  	 if ( pegs_dir_changed ) // add list of pegsdat files
                   update_files( PEGSdir, pegs4ComboBox, "*.pegs4dat" );

//checkErrors(); => not used here anymore, since it is automatically called
//--------- ^	    when the text in the PEGS4 combo box is modified.
//                  Leave this message here for now!!!!!!!!

// call to the function below must be after setting the current input file name
	 update_usercode();

         update_files( SPECdir, specfnameComboBox, "*.spectrum *.ensrc" );
         update_files( RDISTdir, raddistfnameComboBox, "*.*" );
         update_files( PHSPdir, phasespaceComboBox, "*.egsphsp1 *.IAEAphsp" );

         disconnect( CONFcomboBox, SIGNAL( editTextChanged(const QString&) ),
                     this, SLOT( checkConfigLib() ) );
         update_files( CONFdir, CONFcomboBox, "*.conf" );
         connect( CONFcomboBox, SIGNAL( editTextChanged(const QString&) ),
                 this, SLOT( checkConfigLib() ) );
         //update_conf_files();

	QString ifullt = Input->mMC->GetIFULL();

	//... TITLE
	TitleEdit->setText( Input->mTitle->str );
	//... I/O Control
	update_IOControl( Input->mIO );
               //... MC Inputs
	update_MCInputs( Input->mMC );
	//... GEO Inputs
	update_GEOInputs( Input->mGEO );
	//... PHD Inputs
//	if ( ( usercodename.toLower() == "dosrznrc") &&
//	     ( ifull.toLower() == "pulse height distribution") ){
	     update_PHDInputs( Input->mPHD );
//             }
	//... CAV Inputs
	update_CAVInputs( Input->mCAV );
	//... SRC Inputs
	update_SRCInputs( Input->mSRC );
	//... MC transport parameters
 	update_MCTParam( Input->mMCP );
        //... PEGSLESS parameters
        update_PEGSLESSParam( Input->mPGLS);
	//... Variance Reduction
 	update_VarParam( Input->mVAR );
	//... Plot Control
 	update_PlotControl( Input->mPLOT );


        update_caption( EGSfileName );

        //cout << "... Updated form!!!" << endl;
        //TabWidgetRZ->setCurrentWidget(GItab);
        //TabWidgetRZ->setCurrentIndex(0);
}

// update form when a pegs4 data file is selected from combo box
//i.e. when ACTIVATED signal emmited
// CURRENTLY DISCONNECTED
void inputRZImpl::change_pegs4_name()
{
     PEGSfileName     = pegs4ComboBox->currentText ();

     if ( !PEGSfileName.isNull() ) {
          //checkErrors();
          checkExecutionAbility();

          listMedia = getPEGSMedia( PEGSdir + PEGSfileName );
          update_from_data_area();
     }
}

// update form when a configuration file is selected from combo box
//i.e. when ACTIVATED signal emmited
// CURRENTLY CONNECTED
void inputRZImpl::change_config_file()
{
    updateConfiguration( CONFcomboBox->currentText() );
}


// update form when an inp file is selected from combo box
//i.e. when ACTIVATED signal emmited
// CURRENTLY CONNECTED
void inputRZImpl::change_input_file()
{
    EGSfileName      = InputFileComboBox->currentText ();
    if ( check_file( EGSdir + EGSfileName ) && !EGSfileName.isEmpty()  ){
    //if ( !EGSfileName.isEmpty() )
        open();
    }
    //checkErrors("change_input_file()");
    //checkErrors();
    checkExecutionAbility();
    checkPrintAbility();
    checkPreviewRZ();
}

//!  Updates automatically inp file name while user types or modifies text and check for its existence .
/*!
Any time the text is modified in the input file name box, this function updates the EGSnrc
input file name and cheks if the corresponding input file exists (it looks in the defined
egsnrc user code area). If the file does exist, the \em Execute, \em PreviewRZ and \em Print
buttons are enabled, otherwise they are automatically disabled.
*/
void inputRZImpl::EGSFileNameChanged( const QString& str )
{
    EGSfileName = str ;
    if ( check_file( EGSdir + EGSfileName ) && !EGSfileName.isEmpty() ){
         egs_dir_changed = false; //changes in file name are in current dir
        InputFileComboBox->showPopup();
        InputFileComboBox->setCurrentIndex( Add_New_Item(InputFileComboBox->lineEdit()->text().toLatin1().data(),
						          InputFileComboBox) );
        InputFileComboBox->lineEdit()->setFocus();
    }
    else {
         ExecuteButton->setEnabled( false );
         PreviewRZButton->setEnabled( false );
         PrintButton->setEnabled( false );
    }
}

//!  Updates automatically pegs4 file name while user types or modifies text and check for its existence .
/*!
Any time the text is modified in the pegs4 file name box, this function updates
the pegs4 data file name and cheks whether the corresponding pegs4 data file
exists (it looks in the defined pegs4 data area). If the file does exist,
the \em Execute button is enabled, otherwise it is automatically disabled. The check
is performed through a call to checkErrors().
*/
void inputRZImpl::PEGSFileNameChanged( const QString& str )
{
    PEGSfileName      = str ;
    if ( check_file( PEGSdir + PEGSfileName ) && !PEGSfileName.isEmpty() )  { // data file exists
         pegs_dir_changed = false; //changes in file name are in current dir
          listMedia = getPEGSMedia( PEGSdir + PEGSfileName );
          update_from_data_area();

        QString tmpText;
         tmpText = wallmaterialComboBox->currentText();
         wallmaterialComboBox->clear();
         wallmaterialComboBox->addItems( StrListToQStrList( listMedia ) );
         wallmaterialComboBox->setEditText ( tmpText );

          tmpText = electrmatComboBox->currentText();
          electrmatComboBox->clear();
          electrmatComboBox->addItems( StrListToQStrList( listMedia ) );
          electrmatComboBox->setEditText ( tmpText );

          reset_mediaTable();
          reset_customFFTable();

         //checkErrors("PEGSFileNameChanged");
         //checkErrors();
         checkExecutionAbility();
     }
}

void inputRZImpl::update_IOControl( const MIOInputs* EGSio )
{
	//... I/O control

	// IWATCH
	QString error = QString(WARNING_IWATCH);
	validate_combo( EGSio->iwatch.toLatin1().data() , error, iwatchComboBox );

	// STORE INITIAL RANDOM NUMBERS
	error = QString(WARNING_STRAND);
	QRadioButton *rb[3] ;
	rb[0] = noRandRadioButton ;
	rb[1] = lastRandRadioButton ;
	rb[2] = allRandRadioButton ;
	validate_radio( EGSio->strnd.toLatin1().data(), error, 3, rb );

        //delete[] rb;

	// IRESTART
	error = QString(WARNING_IRESTART);
	validate_combo( EGSio->irestart.toLatin1().data() , error, irestartComboBox );

  	// STORE DATA ARRAYS
	if ( EGSio->strdat.toLower() == "yes" ){
	    storeDataCheckBox->setChecked( true );
	}
	else if (EGSio->strdat.toLower() == "no") {
	    storeDataCheckBox->setChecked( false );
	}
	else {
	    openErrors += QString(WARNING_STRDAT) + "\n";
	}

	if (usercode == dosrznrc) {
	// ELECTRON TRANSPORT
	   error = QString(WARNING_ETRANS);
	   validate_combo( EGSio->etransport.toLatin1().data(), error, etransportComboBox );

	// DOSE OUTPUT REGIONS
	   DoseRegGroupBox->setEnabled( true );
	   minPlnSpinBox->setValue( EGSio->dosereg.minPln );
	   maxPlnSpinBox->setValue( EGSio->dosereg.maxPln );
	   minCylSpinBox->setValue( EGSio->dosereg.minCyl );
	   maxCylSpinBox->setValue( EGSio->dosereg.maxCyl );
	}

  	v_int v[2];
	// STOPPING POWER OUTPUT REGIONS
//	if (usercode == sprrznrc) {
	    error = QString(WARNING_SPROUT);
	    validate_combo( EGSio->sproutopt.toLatin1().data(), error, sproutComboBox );

	    v[0] = EGSio->sproutreg.start_cyl;
	    v[1] = EGSio->sproutreg.stop_slab;
      	    update_SprOutTableHeaders();
	    update_table(v, 0, 2, sproutTable);
//	}
//	else if (usercode != flurznrc) {
	// OUTPUT OPTIONS
	    error = QString(WARNING_OUTOPT);
	    validate_combo( EGSio->outopt.toLatin1().data() , error, outoptComboBox );
//	}

//  	if ( usercode == flurznrc) {
	   validate_combo( EGSio->printFluSpec.toLatin1().data(), "validation ERROR", PrintFluSpeComboBox );
	   if ( EGSio->printFluSpec.toLower() == "specified" ){
	      v[0] = EGSio->ListFluStart;
	      v[1] = EGSio->ListFluStop;
	      update_table(v, 0, 2, ListFluTable);
   	   }
	   validate_combo( EGSio->iprimary.toLatin1().data(), "validation ERROR", IPRIMARYComboBox );
	   sloteFluEdit->setText( EGSio->slote );
   	   v_float vi[1];
  	   //if ( EGSio->slote.toFloat( 0 ) == 0.0f ) {
	      vi[0] = EGSio->bintop;
	      update_table( vi, 0, 1, sloteFluTable );
   	   //}
//  	}
}

void inputRZImpl::update_MCInputs( const MMCInputs* EGSmc )
{
   // NUMBER OF HISTORIES
   ncaseEdit->setText(EGSmc->ncase);

   // MAX CPU HOURS ALLOWED
   maxCPUEdit->setText(EGSmc->maxcpu);

   if ( ( usercode != sprrznrc ) && ( usercode != flurznrc ) ){

      // IFULL
      QString error = QString(WARNING_IFULL);
      validate_combo( EGSmc->ifull.toLatin1().data() , error, ifullComboBox );

      // STATISTICAL ACCURACY SOUGHT
      statEdit->setText(EGSmc->stat);
   }

   if (usercode == dosrznrc) {// SCORE KERMA
      if ( EGSmc->kerma.toLower() == "yes" ){
         kermaCheckBox->setChecked( true );
      }
      else if (EGSmc->kerma.toLower() == "no") {
         kermaCheckBox->setChecked( false );
      }
      else {
         openErrors += QString(WARNING_KERMA) + "\n";
      }
      if ( (ifullComboBox->currentText()).toLower() == "pulse height distribution")
               phdGroupBox->setEnabled( true );
   }
   else if ( usercode != flurznrc ) {// PHOTON REGENERATION
      if ( EGSmc->photreg.toLower() == "yes" ){
         photregCheckBox->setChecked( true );
      }
      else if (EGSmc->photreg.toLower() == "no") {
         photregCheckBox->setChecked( false );
      }
      else {
         openErrors += QString(WARNING_PHOTREG) + "\n";
      }
   }

   // INITIAL RANDOM NO. SEEDS
   rand1SpinBox->setValue( EGSmc->rnd[0] );
   rand2SpinBox->setValue( EGSmc->rnd[1] );
}

void inputRZImpl::update_GEOInputs( const MGEOInputs* EGSgeo )
{
	// INPUT METHOD
	QString error = QString(WARNING_INP_METHOD);
 	QRadioButton *rb[3] ;
	rb[0] = groupRadioButton ;
	rb[1] = individualRadioButton ;
	rb[2] = cavityRadioButton ;
	// there is a discrepancy between CAVRZnrc and this GUI !!!!
	QString inp_meth = EGSgeo->inp_meth;
              if ( inp_meth.toLower() == "cavity information" )
	     inp_meth = "cavity description" ;
	validate_radio( inp_meth.toLatin1().data(), error, 3, rb );

        //delete[] rb;

	if ( EGSgeo->inp_meth.toLower() != "cavity information" ){
	   // Z FRONT FACE
	   ZFaceEdit->setText(EGSgeo->zface);

	   // GEOMETRY TABLE
	   if ( EGSgeo->inp_meth.toLower() == "groups" ){
	       set_group();
	       v_int vi[1];
	       vi[0] = EGSgeo->nslab;
	       update_table(vi, 0, 1, geometryTable);
	       v_float v[2];
	       v[0] = EGSgeo->slabs;
	       v[1] = EGSgeo->slabs;
	       update_table(v, 1, 2, geometryTable);
	       v[0] = EGSgeo->radii;
	       update_table(v, 0, 1, cylTable);
	   }
	   else {
	       set_individual();
	       globalNSlab = EGSgeo->nslab;
	       v_float v[1];
	       v[0] = EGSgeo->slabs;
	       update_table(v, 0, 1, geometryTable);
	       v[0] = EGSgeo->radii;
	       update_table(v, 0, 1, cylTable);
	   }
	   // MEDIA DESCRIPTION
	   QString error = QString(WARNING_MEDIA_DESCRIPTION);
	   validate_combo( EGSgeo->description_by.toLatin1().data() , error, mediaComboBox );
           current_description_by = EGSgeo->description_by;

	   // MEDIA TABLE
	   update_mediaTable( EGSgeo );
	   //fill_media_table( EGSgeo );
	}
       else set_cavity();

}

void inputRZImpl::update_PHDInputs(const MPHDInputs* EGSphd )
{
     v_int v[1];
     v[0] = EGSphd->sreg ;
     update_table(v, 0, 1, phdTable);

     if ( EGSphd->slote.toFloat(0) < 0.0f ) {
        v_float vi[2];
        vi[0] = EGSphd->bintop ;
        vi[1] = EGSphd->bintop ;
        update_table(vi, 1, 2, phdTable);
    }

     SLOTEEdit->setText( EGSphd->slote ) ;
     DELTAEEdit->setText( EGSphd->deltae ) ;
}

void inputRZImpl::update_CAVInputs( const MCAVInputs*  EGScav )
{
if ( EGScav->inp_meth.toLower() != "cavity information" ) {
      v_int v[1];
      v[0] = EGScav->cav_reg;
      update_table(v, 0, 1, cavTable);
  }
else{
      wallthickEdit->setText( EGScav->wall_thick );
      cavradEdit->setText( EGScav->cav_rad );
      cavlenEdit->setText( EGScav->cav_len );
      electradEdit->setText( EGScav->electr_rad );

   QString error = "Cavity wall material  <b>" + EGScav->wall_mat +
		"</b> not found in current pegs4 data file !<br>";
   validate_combo( EGScav->wall_mat.toLatin1().data() , error, wallmaterialComboBox );
   wallmaterialComboBox->setItemText(wallmaterialComboBox->currentIndex(),EGScav->wall_mat);

   error = "Electrode material <b>" + EGScav->electr_mat +
	 "</b> not found in current pegs4 data file !<br>";
   validate_combo( EGScav->electr_mat.toLatin1().data() , error, electrmatComboBox );
   electrmatComboBox->setItemText(electrmatComboBox->currentIndex(),EGScav->electr_mat);

      if ( EGScav->electr_rad.toFloat(0) > 0.0 )
         thimbleRadioButton->setChecked( true );
      set_cav_regions();
      set_electr();
}
}

void inputRZImpl::update_SRCInputs( const MSRCInputs* EGSsrc )
{
	validate_combo( EGSsrc->isource.toLatin1().data(), "ERROR in source number",
                        sourceComboBox );


	update_source_type();

	if ( EGSsrc->isource == "23" ){
           updateBeamSource(EGSsrc->beam_code, EGSsrc->pegs_file);
           beamDlg->set_beam_code(EGSsrc->beam_code);
           beamDlg->set_beam_inp(EGSsrc->inp_file);
           beamDlg->set_beam_pegs(EGSsrc->pegs_file);
           beamDlg->set_beam_min(FToQStr(EGSsrc->weight_win[0]));
           beamDlg->set_beam_max(FToQStr(EGSsrc->weight_win[1]));
           beamDlg->set_beam_dist(   FToQStr(EGSsrc->srcopt[0]));
           beamDlg->set_beam_angle(  FToQStr(EGSsrc->srcopt[1]));
           beamDlg->set_beam_zoffset(FToQStr(EGSsrc->srcopt[2]));
           beamDlg->set_beam_xoffset(FToQStr(EGSsrc->srcopt[3]));
           beamDlg->set_beam_yoffset(FToQStr(EGSsrc->srcopt[4]));
           getBeamSource();// creates a label with above info
        }
	if ( EGSsrc->isource == "21" || EGSsrc->isource == "22" ){
	     if ( EGSsrc->srcopt[0] == 0)
	        imodeComboBox->setCurrentIndex( 0 );
	     else // assume only two possible options: 0 or 2
	        imodeComboBox->setCurrentIndex( 1 );
	}
	else{
	      temp1Edit->setText( FToQStr( EGSsrc->srcopt[0] ) );
	}
	temp2Edit->setText( FToQStr( EGSsrc->srcopt[1] ) );
	temp3Edit->setText( FToQStr( EGSsrc->srcopt[2] ) );
	temp4Edit->setText( FToQStr( EGSsrc->srcopt[3] ) );
	temp5Edit->setText( FToQStr( EGSsrc->srcopt[4] ) );
	temp6Edit->setText( FToQStr( EGSsrc->srcopt[5] ) );
	temp7Edit->setText( FToQStr( EGSsrc->srcopt[6] ) );
 	QRadioButton *rb[5] ;
	rb[0] = eRadioButton ;
	rb[1] = phRadioButton ;
	rb[2] = pRadioButton ;
	rb[3] = chargedRadioButton ;
	rb[4] = allRadioButton ;
	validate_radio( EGSsrc->iniparticle.toLatin1().data(), "ERROR in ini particle", 5, rb );

	rb[0] = monoenergeticRadioButton ;
	rb[1] = spectrumRadioButton ;
	validate_radio( EGSsrc->typenergy.toLatin1().data(), "ERROR in energy option", 2, rb );


        int dirPos;

	if ( EGSsrc->typenergy.toLower() == "monoenergetic" ) {
	    ini_energyEdit->setText(EGSsrc->inienergy);
	}
	else {
	    QString spe_file = EGSsrc->spe_file;
	    dirPos = spe_file.lastIndexOf( '/' );
	    if ( dirPos >= 0 ){
	      SPECdir = spe_file;
              SPECdir = expandEnvVar(SPECdir.remove(dirPos+1,spe_file.length() - dirPos+1 ));
	      spe_file = spe_file.remove(0, dirPos+1);
	    }
   	    specfnameComboBox->setEditText ( spe_file );
	    Add_New_Item( spe_file.toLatin1().data(), specfnameComboBox );
	    if ( EGSsrc->speciout.toLower() == "include" )
                ioutspCheckBox->setChecked( true );
	}

	if ( EGSsrc->isource == "20" ) {
	   rb[0] = localRadioButton ;
	   rb[1] = externalRadioButton ;
	   validate_radio( EGSsrc->modein.toLatin1().data(), "ERROR in ini particle", 2, rb );

	   if ( EGSsrc->modein.toLower() == "external" ) {
	    QString dist_file = EGSsrc->dist_file;
	    dirPos = dist_file.lastIndexOf( '/' );
	    if ( dirPos >= 0 ){
	      RDISTdir = dist_file;
	      RDISTdir = expandEnvVar(RDISTdir.remove(dirPos+1,RDISTdir.length() - dirPos+1 ));
	      dist_file = dist_file.remove(0, dirPos+1);
	    }
   	    raddistfnameComboBox->setEditText ( dist_file );
	    Add_New_Item( dist_file.toLatin1().data(), raddistfnameComboBox );
	    if ( EGSsrc->rdistiout.toLower() == "include" )
                ioutrdistCheckBox->setChecked( true );
	   }
	   else {
	      v_float v[2];
	      v[0] = EGSsrc->rdistf;
	      v[1] = EGSsrc->rpdf;
	      update_table( v, 0, 2, raddistTable );
	   }
	}

	if ( ( EGSsrc->isource == "21" ) || ( EGSsrc->isource == "22" ) ) {
	    QString phsp_file = EGSsrc->phsp_file;
	    dirPos = phsp_file.lastIndexOf( '/' );
	    if ( dirPos >= 0 ){
	      PHSPdir = phsp_file;
	      PHSPdir = expandEnvVar(PHSPdir.remove(dirPos+1,PHSPdir.length() - dirPos+1 ));
	      phsp_file = phsp_file.remove(0, dirPos+1);
	    }
	    phasespaceComboBox->setEditText( phsp_file );
            Add_New_Item( phsp_file.toLatin1().data(), raddistfnameComboBox );
            update_files( PHSPdir, phasespaceComboBox, "*.egsphsp1 *.IAEAphsp" );
	}
}


//PEGSLESS routines

void inputRZImpl::update_PEGSLESSParam( const PEGSLESSInputs* EGSpgls)
{

 QStringList medlst;

 AEEdit->setText(EGSpgls->AE);
 UEEdit->setText(EGSpgls->UE);
 APEdit->setText(EGSpgls->AP);
 UPEdit->setText(EGSpgls->UP);

 MDFEdit->setText(EGSpgls->matdatafile);

 //set some defaults in the input window for material specified in .egsinp file
 isGasCheckBox->setChecked(false);
 enable_gaspEdit();
 medTypeChanged("Element");
 DCcheckBox->setChecked(false);
 enableDCfileInput(false);
 ICRUradCheckBox->setChecked(false);
 DFEdit->setText("");

 //set public (adjustable) values equal to values passed
 Ppgls = new PEGSLESSInputs;

 Ppgls->matdatafile = EGSpgls->matdatafile;
 Ppgls->ninpmedia = EGSpgls->ninpmedia;
 for(int i=0; i<Ppgls->ninpmedia; i++) {
   medlst += EGSpgls->inpmedium[i];
   Ppgls->inpmedium[i]=EGSpgls->inpmedium[i];
   Ppgls->nelements[i]=EGSpgls->nelements[i];
   Ppgls->spec_by_pz[i]=EGSpgls->spec_by_pz[i];
   for(int j=0; j<Ppgls->nelements[i];j++) {
     Ppgls->elements[i].push_back(EGSpgls->elements[i][j]);
     Ppgls->pz_or_rhoz[i].push_back(EGSpgls->pz_or_rhoz[i][j]);
   }
   Ppgls->rho[i]=EGSpgls->rho[i];
   Ppgls->rho_scale[i]=EGSpgls->rho_scale[i];
   Ppgls->spr[i]=EGSpgls->spr[i];
   Ppgls->bc[i]=EGSpgls->bc[i];
   Ppgls->gasp[i]=EGSpgls->gasp[i];
   Ppgls->isgas[i]=EGSpgls->isgas[i];
   Ppgls->dffile[i]=EGSpgls->dffile[i];
   Ppgls->use_dcfile[i]=EGSpgls->use_dcfile[i];
   Ppgls->sterncid[i]=EGSpgls->sterncid[i];
 }


 if(EGSpgls->ninpmedia>0) {
    Ppgls->inpmedind=0; //set input medium index
    inpmediumComboBox->setEditText(EGSpgls->inpmedium[0]);
    if(!GetMedFromDCfile(Ppgls->dffile[0])) {
       if(Ppgls->use_dcfile[0]) {
           QString errStr = "Could not read composition from specified density correction file for medium "  + EGSpgls->inpmedium[0];
           QMessageBox::information( this, " Warning",errStr, QMessageBox::Ok );
       }
       DCcheckBox->setChecked(false);
       enableDCfileInput(false);
    //fill the table based on previous info
       if(Ppgls->nelements[0]==1) medTypeChanged("Element");
       else if(Ppgls->spec_by_pz[0]) medTypeChanged("Compound");
       else medTypeChanged("Mixture");

    //now populate the element table
      for (int i=0; i<EGSpgls->nelements[0]; i++) {
        QString item1 = QString::fromStdString(EGSpgls->elements[0][i]);
        QString item2 =  QString::fromStdString(EGSpgls->pz_or_rhoz[0][i]);
        pz_or_rhozTable->setItem(i,0,new QTableWidgetItem(item1));
        pz_or_rhozTable->setItem(i,1,new QTableWidgetItem(item2));
      }

      //get the other values
      rhoEdit->setText(EGSpgls->rho[0]);
      DFEdit->setText(EGSpgls->dffile[0]);
    }
    else {
      DCcheckBox->setChecked(true);
      enableDCfileInput(true);
    }
    //get parameters that do not depend on density correction file specified
    gaspEdit->setText(EGSpgls->gasp[0]);
    isGasCheckBox->setChecked(EGSpgls->isgas[0]);
    enable_gaspEdit();
    if(EGSpgls->bc[0]=="NRC") ICRUradCheckBox->setChecked(true);
    else ICRUradCheckBox->setChecked(false);
  }

  if(EGSpgls->ninpmedia>0 || EGSpgls->matdatafile!="" ) {
   //assume the user wants to go PEGSless
    PEGSlessRadioButton->setChecked(true);
    set_data_area();
  }

  medlst+="define new medium";
  inpmediumComboBox->clear();
  inpmediumComboBox->addItems(medlst);
}

void inputRZImpl::inpmediumSave( const QString& str)
{

  int ind=inpmediumComboBox->currentIndex();
  Ppgls->inpmedind=ind;
  if(inpmediumComboBox->currentText()!="define new medium"){
    //note: above allows blanks in medium name...probably not a good idea
    if(ind==Ppgls->ninpmedia) {
      //a new medium has been added
      Ppgls->ninpmedia++;
      //add option to define new medium to end of combobox list
      inpmediumComboBox->addItem("define new medium");
    }
    //save data for current medium
    Ppgls->inpmedium[ind]=inpmediumComboBox->currentText();
    if(ICRUradCheckBox->isChecked()) Ppgls->bc[ind]="NRC";
    else Ppgls->bc[ind]="KM";
    Ppgls->isgas[ind]=isGasCheckBox->isChecked();
    Ppgls->gasp[ind]=gaspEdit->text();
    if(Ppgls->isgas[ind] && (Ppgls->gasp[ind]=="" || Ppgls->gasp[ind].toFloat()<=0.0)) Ppgls->gasp[ind]="1.0";
    gaspEdit->setText(Ppgls->gasp[ind]);
    Ppgls->spec_by_pz[ind]=false;
    if(medTypeComboBox->currentText()=="Compound") Ppgls->spec_by_pz[ind]=true;
    //see if any elements are present in the table
    int nrow=0;
    int nelements=0;
    while(pz_or_rhozTable->item(nrow,0)!=0 && nrow < pz_or_rhozTable->rowCount()) {
       if(nrow==0) {
         //clear existing list of elements
         Ppgls->elements[ind].clear();
         Ppgls->pz_or_rhoz[ind].clear();
       }
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
    Ppgls->rho_scale[ind]=1.;
    if(rhoScaleComboBox->currentIndex()==1) Ppgls->rho_scale[ind]=0.001;
    Ppgls->dffile[ind]=DFEdit->text();
    if(DCcheckBox->isChecked())  Ppgls->use_dcfile[ind]=true;
    else Ppgls->use_dcfile[ind]=false;
  }
  //update list of available media
  listMedia = getPEGSLESSMedia();
  updateMediaLists();
}

void inputRZImpl::GetDFfileReturn()
{
//slot actuated when return pressed in DC file entry box
   if(!GetMedFromDCfile(DFEdit->text())){
      QString errStr = "Could not open density correction file "  + DFEdit->text();
               errStr += "\nPlease select another file and try again!";
        QMessageBox::information( this, " Warning",errStr, QMessageBox::Ok );
   }
}

bool inputRZImpl::GetMedFromDCfile(QString f)
{
   if ( !f.isEmpty() ) {
      QString fname=f;
      QChar s = QDir::separator();
      //if the file separator character is in the name, assume
      //full path specified along with .density extension
      if(!QFile(fname).exists()) {
        //start looking following same priority as in
        //get_media_inputs
        //assume .density ext not included
        fname=ironIt(EGS_HOME + s + "pegs4" + s + "density_corrections" + s + f + ".density");
        if(!QFile(fname).exists()) {
          //look in subdirectories
          fname=ironIt(EGS_HOME + s + "pegs4" + s + "density_corrections" + s + "elements" + s + f + ".density");
          if(!QFile(fname).exists()) {
             fname=ironIt(EGS_HOME + s + "pegs4" + s + "density_corrections" + s + "compounds" + s + f + ".density");
             if(!QFile(fname).exists()) {
             //look in density subdirectory in case still there
               fname=ironIt(EGS_HOME + s + "pegs4" + s + "density" + s + f + ".density");
               if(!QFile(fname).exists()) {
                //look in HEN_HOUSE
                fname=ironIt(HEN_HOUSE + s + "pegs4" + s + "density_corrections" + s + "elements" + s + f + ".density");
                if(!QFile(fname).exists()) {
                  fname=ironIt(HEN_HOUSE + s + "pegs4" + s + "density_corrections" + s + "compounds" + s + f + ".density");
                  if(!QFile(fname).exists()) return false;
                }
               }
             }
           }
          }
      }
      //now open the file if possible and read the composition of
      //the medium
      QFile df(fname);
      QFileInfo fi(fname);
      if(!df.open(QFile::ReadOnly)) return false;
      char buf[256]; df.readLine(buf,255);//ignore first line
      QTextStream data(&df);
      int ndat, nelem; double Iev, rho;
      data >> ndat >> Iev >> rho >> nelem;
      if(nelem==1) {
        medTypeChanged("Element");
      }
      else {
        medTypeChanged("Mixture");
      }
      //modify elements of medium def. tab
      rhoEdit->setText(QString("%1").arg(rho));
      rhoScaleComboBox->setCurrentIndex(0);
      //first clear the element table
      for( int j=0; j< pz_or_rhozTable->rowCount(); j++) {
         pz_or_rhozTable->setItem(j,0,0); pz_or_rhozTable->setItem(j,1,0);
      }
      //now put the elements of this medium in
      for(int j=0; j<nelem; j++) {
         int iz; double frac; data >> iz >> frac;
         pz_or_rhozTable->setItem(j,0,new QTableWidgetItem(QString::fromStdString(element_data[iz-1].symbol)));
         pz_or_rhozTable->setItem(j,1,new QTableWidgetItem(QString("%1").arg(frac)));
      }
      DFEdit->setText(fi.absoluteFilePath());
      return true;
    }
    return false;
};

void inputRZImpl::inpmediumChanged( const QString& str)
{

  int ind = inpmediumComboBox->currentIndex();

  Ppgls->inpmedind=ind;

  if(ind<Ppgls->ninpmedia) {

    //first, fill the table based on previous info
    if(Ppgls->nelements[ind]==1) medTypeChanged("Element");
    else if(Ppgls->spec_by_pz[ind]) medTypeChanged("Compound");
    else medTypeChanged("Mixture");

       //now populate the element table
    for (int i=0; i<Ppgls->nelements[ind]; i++) {
         QString item1 = QString::fromStdString(Ppgls->elements[ind][i]);
         QString item2 = QString::fromStdString(Ppgls->pz_or_rhoz[ind][i]);
         pz_or_rhozTable->setItem(i,0,new QTableWidgetItem(item1));
         pz_or_rhozTable->setItem(i,1,new QTableWidgetItem(item2));
    }
       //clear any other items in there
    for (int i=Ppgls->nelements[ind]; i<pz_or_rhozTable->rowCount(); i++) {
         pz_or_rhozTable->setItem(i,0,0);
         pz_or_rhozTable->setItem(i,1,0);
    }
    //get the other values
    rhoEdit->setText(Ppgls->rho[ind]);
    if(Ppgls->rho_scale[ind]<1.) rhoScaleComboBox->setCurrentIndex(1);
    else rhoScaleComboBox->setCurrentIndex(0);
    DFEdit->setText(Ppgls->dffile[ind]);
    gaspEdit->setText(Ppgls->gasp[ind]);
    isGasCheckBox->setChecked(Ppgls->isgas[ind]);
    enable_gaspEdit();
    if(Ppgls->bc[ind]=="NRC") ICRUradCheckBox->setChecked(true);
    else ICRUradCheckBox->setChecked(false);
    DCcheckBox->setChecked(false);
    enableDCfileInput(false);

    //if the user wants to use a dc file, then attempt to read in
    //medium comp. from it
    if(Ppgls->use_dcfile[ind]) {
       if(!GetMedFromDCfile(Ppgls->dffile[ind])) {
           QString errStr = "Could not read composition from specified density correction file for medium "  + Ppgls->inpmedium[ind];
           QMessageBox::information( this, " Warning",errStr, QMessageBox::Ok );
           Ppgls->use_dcfile[ind]=false;
           DCcheckBox->setChecked(false);
           enableDCfileInput(false);
       }
       else {
           DCcheckBox->setChecked(true);
           enableDCfileInput(true);
       }
    }
  }
  else if(ind==Ppgls->ninpmedia) {
    //prepare to add a new medium
    //clear the table and set defaults
    //default to Element: we could quibble with this
    medTypeChanged("Element");
    //clear the element table
    for (int i=0; i<pz_or_rhozTable->rowCount(); i++) {
        pz_or_rhozTable->setItem(i,0,0);
        pz_or_rhozTable->setItem(i,1,0);
    }
    rhoEdit->setText("");
    rhoScaleComboBox->setCurrentIndex(0);
    gaspEdit->setText("");
    isGasCheckBox->setChecked(false);
    enable_gaspEdit();
    DFEdit->setText("");
    enableDCfileInput(false);
    DCcheckBox->setChecked(false);
    ICRUradCheckBox->setChecked(false);
  }
  //now update the list of media
  QStringList medlst;
  //at this point, delete media that are blank or have blank as first character
  int k=0;
  for(int i=0; i<Ppgls->ninpmedia; i++) {
      if(Ppgls->inpmedium[i]!="" &&
         Ppgls->inpmedium[i].indexOf(" ",0)!=0) {
          if(ind==i) ind=k;
          Ppgls->inpmedium[k]=Ppgls->inpmedium[i];
          Ppgls->spec_by_pz[k]=Ppgls->spec_by_pz[i];
          Ppgls->nelements[k]=Ppgls->nelements[i];
          //save temporary vector strings
          v_string tempelements = Ppgls->elements[i];
          v_string temppz_or_rhoz = Ppgls->pz_or_rhoz[i];
          Ppgls->elements[k].clear();
          Ppgls->pz_or_rhoz[k].clear();
          for (int j=0; j<Ppgls->nelements[k]; j++) {
             Ppgls->elements[k].push_back(tempelements[j]);
             Ppgls->pz_or_rhoz[k].push_back(temppz_or_rhoz[j]);
          }
          Ppgls->rho[k]=Ppgls->rho[i];
          Ppgls->rho_scale[k]=Ppgls->rho_scale[i];
          Ppgls->spr[k]=Ppgls->spr[i];
          Ppgls->bc[k]=Ppgls->bc[i];
          Ppgls->gasp[k]=Ppgls->gasp[i];
          Ppgls->dffile[k]=Ppgls->dffile[i];
          Ppgls->use_dcfile[k]=Ppgls->use_dcfile[i];
          Ppgls->sterncid[k]=Ppgls->sterncid[i];
          medlst+=Ppgls->inpmedium[k];
          k++;
      }
  }
  Ppgls->ninpmedia=k;
  if(ind>Ppgls->ninpmedia) ind=0;
  medlst+="define new medium";
  inpmediumComboBox->clear();
  inpmediumComboBox->addItems(medlst);
  inpmediumComboBox->setCurrentIndex(ind);
}

void inputRZImpl::pz_or_rhozTable_clicked(int row, int col) {
   if(col == 0) {
     QString str;
     if(pz_or_rhozTable->item(row,col)) str = pz_or_rhozTable->item(row,col)->text();
     //if elements has not bee initialized yet, do so
    //these global variables are defined in peglessinputs.h
     if(element_list.size()==0) {
     //copy element symbols into elements
        for(int j=0; j<n_element; j++) element_list << QString::fromStdString(element_data[j].symbol);
     }
     QComboBox *e = new QComboBox;
     e->addItems(element_list);
     if(!str.isNull()) {//see if it's already in the media list
         bool found=false;
         for(int i=0; i<e->count(); i++) {
              if(str == e->itemText(i)){
                found=true;
                e->setCurrentIndex(i);
                break;
              }
         }
         if(!found) {
              e->insertItem(0,str);
              e->setCurrentIndex(0);
         }
     }
     pz_or_rhozTable->setCellWidget(row,col,e);
   }
}

void inputRZImpl::pz_or_rhozTable_singleclicked( int row, int col) {
   if(col == 0) {
     QWidget *editor = pz_or_rhozTable->cellWidget( row, col );
     if(!editor) {//protect any text already there
       QTableWidgetItem* w =  pz_or_rhozTable->item(row,col);
       if(w) {
          QString str = w->text();
          pz_or_rhozTable->setItem(row,col,new QTableWidgetItem(str));
       }
     }
   }
//if we click away from a comboBox, freeze it and just display the text
   for(int irow =0; irow <pz_or_rhozTable->rowCount(); irow++) {
     if(row != irow || col !=0) {
       QWidget *editor = pz_or_rhozTable->cellWidget( irow, 0 );
       if(editor) {
        if(string(editor->metaObject()->className())=="QComboBox"){
          QComboBox* cb = (QComboBox*)editor;
          QString str = cb->currentText();
          pz_or_rhozTable->removeCellWidget(irow,0);
          pz_or_rhozTable->setItem(irow,0,new QTableWidgetItem(str));
        }
      }
     }
   }
}

void inputRZImpl::enableDCfileInput(bool is_checked) {
   pz_or_rhozTable->setEnabled (!is_checked);
   DFgroupBox->setEnabled(is_checked);
   rhoGroupBox->setEnabled(!is_checked);
   medTypeGroupBox->setEnabled(!is_checked);
   isGasCheckBox->setEnabled(!is_checked);
   enable_gaspEdit();
}

enum MedIndex {Elem, Comp, Mixt};

void inputRZImpl::medTypeChanged( const QString &s) {

  if( s == "Compound" ) {
     pz_or_rhozTable->setHorizontalHeaderItem(1,new QTableWidgetItem("Stoichiometric index"));
     medTypeComboBox->setCurrentIndex(Comp);
  }
  else if(s== "Mixture") {
     pz_or_rhozTable->setHorizontalHeaderItem(1,new QTableWidgetItem("Fraction by weight"));
     medTypeComboBox->setCurrentIndex(Mixt);
  }
  else if(s=="Element") {
     pz_or_rhozTable->setHorizontalHeaderItem(1,new QTableWidgetItem("Fraction by weight"));
     medTypeComboBox->setCurrentIndex(Elem);
  }
  bool ro = s == "Element";
  for(int j=0; j < pz_or_rhozTable->rowCount(); j++) {
    if( ro && j>0 ) {
      pz_or_rhozTable->setItem(j,0,0); pz_or_rhozTable->setItem(j,1,0);
    }
  }
}

//end of PEGSLESS routines

void inputRZImpl::update_MCTParam( const MMCPInputs*  EGSmcp )
{
  validate_combo( EGSmcp->PairSampling.toLatin1().data(), "ERROR in Pair Angular Sampling",
                          PairAngSamplingComboBox );
  validate_combo( EGSmcp->BremsSampling.toLatin1().data(), "ERROR in Brems Angular Sampling",
                          BremsAngSamplingComboBox );
  validate_combo( EGSmcp->BremsXSection.toLatin1().data(), "ERROR in Brems X Section",
                          BremsXSectionComboBox );
  validate_combo( EGSmcp->ESTEPAlg.toLatin1().data(), "ERROR in ESTEP Algorithm",
                          estep_algorithmComboBox );
  validate_combo( EGSmcp->BoundXAlg.toLatin1().data(), "ERROR in Boundary Crossing Algorithm",
                          BCAComboBox );

  validate_combo( EGSmcp->PhotXSection.toLatin1().data(), "ERROR in photon X Section",
                          photonXSectioncomboBox );
  validate_combo( EGSmcp->EIIXSection.toLatin1().data(), "ERROR in EII X Section",
                          EIIcomboBox );

  GPCUTEdit->setText( EGSmcp->GPCUT );
  GECUTEdit->setText( EGSmcp->GECUT );
  GSMAXEdit->setText( EGSmcp->GSMAX );
  ESTEPEEdit->setText( EGSmcp->ESTEPE );
  XImaxEdit->setText( EGSmcp->XImax );
  SkinDepthEdit->setText( EGSmcp->SkinD );

  v_float v[1];
  v_int  vi[3];

  v[0]  = EGSmcp->PCUT;
  vi[0] = EGSmcp->PCUTstart;
  vi[1] = EGSmcp->PCUTstart;
  vi[2] = EGSmcp->PCUTstop;
  if ( EGSmcp->SetPCUTbyRegion ){
      PCUTCheckBox->setChecked( true );
  }
  else {
      PCUTCheckBox->setChecked( false );
  }
  update_table(  v, 0, 1, PCUTTable );
  update_table( vi, 1, 3, PCUTTable );

  v[0]  = EGSmcp->ECUT;
  vi[0] = EGSmcp->ECUTstart;
  vi[1] = EGSmcp->ECUTstart;
  vi[2] = EGSmcp->ECUTstop;
  if ( EGSmcp->SetECUTbyRegion ){
      ECUTCheckBox->setChecked( true );
  }
  else {
      ECUTCheckBox->setChecked( false );
  }
  update_table(  v, 0, 1, ECUTTable );
  update_table( vi, 1, 3, ECUTTable );

  v[0]  = EGSmcp->SMAX;
  vi[0] = EGSmcp->SMAXstart;
  vi[1] = EGSmcp->SMAXstart;
  vi[2] = EGSmcp->SMAXstop;
  if ( EGSmcp->SetSMAXbyRegion ){
      SMAXCheckBox->setChecked( true );
  }
  else {
      SMAXCheckBox->setChecked( false );
  }
  update_table(  v, 0, 1, SMAXTable );
  update_table( vi, 1, 3, SMAXTable );

  QRadioButton *rb[5] ;

  validate_combo( EGSmcp->BC.toLatin1().data(), "ERROR in Bound Compton option",
                  BoundComptoncomboBox );
  vi[0] = EGSmcp->BCstart;
  vi[1] = EGSmcp->BCstop;
  update_table(  vi, 0, 2, BoundComptonTable );

  validate_combo( EGSmcp->PE.toLatin1().data(), "ERROR in PE ang. sampling option",
                  PEcomboBox );
  vi[0] = EGSmcp->PEstart;
  vi[1] = EGSmcp->PEstop;
  update_table(  vi, 0, 2, PEAngSamplingTable );

  validate_combo( EGSmcp->RAY.toLatin1().data(), "ERROR in Rayleigh option",
                  RayleighcomboBox );
  vi[0] = EGSmcp->RAYstart;
  vi[1] = EGSmcp->RAYstop;
  update_table(  vi, 0, 2, RayleighTable );
  if (EGSmcp->RAY.toLower()=="custom"){
    v_string vstr[2]; vstr[0]=EGSmcp->ff_media; vstr[1]=EGSmcp->ff_files;
    //qt3to4 -- BW
    vector<QString> vqstr[2];
    for(int i=0; i<2; i++) {
       for(int j=0; j<vstr[i].size(); j++) {
           vqstr[i].push_back(vstr[i][j].c_str());
       }
    }
    update_table(  vstr, 0, 2, customFFTable );
    //update_table(  vqstr, 0, 2, customFFTable );
  }

  validate_combo( EGSmcp->RELAX.toLatin1().data(), "ERROR in Atomic Relaxation option",
                  RelaxationcomboBox );
  vi[0] = EGSmcp->RELAXstart;
  vi[1] = EGSmcp->RELAXstop;
  update_table(  vi, 0, 2, RelaxationsTable );

  if ( EGSmcp->PhotXSectionOut.toLower() == "on" ) {
     photonXSectionOutCheckBox->setChecked( true );
  }
  else if ( EGSmcp->PhotXSectionOut.toLower() == "off" ){
     photonXSectionOutCheckBox->setChecked( false );
  }
  else {
     openErrors += "WRONG text for spin option\n";
  }

  if ( EGSmcp->Spin.toLower() == "on" ) {
     SpinCheckBox->setChecked( true );
  }
  else if ( EGSmcp->Spin.toLower() == "off" ){
     SpinCheckBox->setChecked( false );
  }
  else {
     openErrors += "WRONG text for spin option\n";
  }

  //delete[] rb;

}

void inputRZImpl::update_VarParam( const MVARInputs*  EGSvar )
{

  if ( EGSvar->eRangeRej.toLower() == "on"){
      eRangeRejCheckBox->setChecked( true );
      ESAVEINEdit->setEnabled( true );
      ESAVEINLabel->setEnabled( true );
  }
  else {
      eRangeRejCheckBox->setChecked( false );
      ESAVEINEdit->setEnabled( false );
      ESAVEINLabel->setEnabled( false );
  }

 ESAVEINEdit->setText( EGSvar->ESAVEIN );

 if ( ( usercode == dosrznrc ) ||
      ( usercode == flurznrc ) ){
    if ( EGSvar->BremsSplitting.toLower() == "on")
      BremsSplitCheckBox->setChecked( true );
    else BremsSplitCheckBox->setChecked( false );

    if ( EGSvar->chargedPartRR.toLower() == "on")
      ChargedPartRRCheckBox->setChecked( true );
    else ChargedPartRRCheckBox->setChecked( false );

    BremsSplitSpinBox->setValue( EGSvar->nBrems );
 }

 if ( usercode != flurznrc ) {
    RRDepthEdit->setText( EGSvar->RRDepth );
    RRFractionEdit->setText( EGSvar->RRFraction );

    ExpTrafoCEdit->setText( EGSvar->ExpoTrafoC );
 }

 if ( EGSvar->PhotonForcing.toLower() == "on"){
      PhotonForcingCheckBox->setChecked( true );
      StartForcingSpinBox->setEnabled( true );
      StartForcingLabel->setEnabled( true );
      StopForcingSpinBox->setEnabled( true );
      StopForcingLabel->setEnabled( true );
  }
 else {
     PhotonForcingCheckBox->setChecked( false );
      StartForcingSpinBox->setEnabled( false );
      StartForcingLabel->setEnabled( false );
      StopForcingSpinBox->setEnabled( false );
      StopForcingLabel->setEnabled( false );
 }

 StartForcingSpinBox->setValue( EGSvar->startForcing );
 StopForcingSpinBox->setValue( EGSvar->stopForcing );

 if ( ( usercode == dosrznrc ) ||
      ( usercode == cavrznrc ) ){

     CSEnhancementSpinBox->setValue( EGSvar->CSEnhancement );

     v_int vi[2];
     vi[0] = EGSvar->CSEnhanStart;
     vi[1] = EGSvar->CSEnhanStop;
     update_table(  vi, 0, 2, CSEnhancementTable );
 }

 if ( usercode == cavrznrc ) photonSplitSpinBox->setValue( EGSvar->nsplit );

}

void inputRZImpl::update_PlotControl( const MPLOTInputs* EGSplot )
{

  if ( EGSplot->Plotting.toLower() == "on" ) {

       plotCheckBox->setChecked( true );
       PlotGroupBox->setEnabled( true );

       if ( EGSplot->LinePrnOut.toLower() == "on" )
            LinePrnOutCheckBox->setChecked( true );
       else LinePrnOutCheckBox->setChecked( false );

       if ( ( EGSplot->ExtPlotOut.toLower() == "on" ) ||
            ( usercode == flurznrc ) ){
            ExtPlotOutCheckBox->setChecked( true );
     	      validate_combo( EGSplot->ExtPlotType.toLatin1().data(), "ERROR in External Plot Type",
                            ExternalPlotTypeComboBox );
       }
       else { ExtPlotOutCheckBox->setChecked( false ); }

       if ( usercode == flurznrc ) {
         QString drawFluPLot = EGSplot->DrawFluPlot;
         if ( drawFluPLot.toLower() == "none") drawFluPLot = "total";
         if ( drawFluPLot.toLower() == "all") drawFluPLot = "Primaries and total";
         validate_combo( drawFluPLot.toLatin1().data(), "ERROR in Fluence Plot",
         DrawFluPlotsComboBox );
          if ( EGSplot->eminusPlot.toLower() == "on" )
               eminusPlotCheckBox->setChecked( true );
          else eminusPlotCheckBox->setChecked( false );

          if ( EGSplot->eplusPlot.toLower() == "on" )
               eplusPlotCheckBox->setChecked( true );
          else eplusPlotCheckBox->setChecked( false );

          if ( EGSplot->ePlot.toLower() == "on" )
               ePlotCheckBox->setChecked( true );
          else ePlotCheckBox->setChecked( false );

          if ( EGSplot->gPlot.toLower() == "on" )
               gammaPlotCheckBox->setChecked( true );
          else gammaPlotCheckBox->setChecked( false );


//          if (EGSplot->DrawFluPlot.toLower() == "none")
//                 PlotRegionsGroupBox->setEnabled( false );
//          else PlotRegionsGroupBox->setEnabled( true );
// it should also disable the other table!!!!!
       }
  }
  else {
      PlotGroupBox->setEnabled( false );
      plotCheckBox->setChecked(false);
  }

       v_int vi[2];
       vi[0] = EGSplot->PlotIX;
       vi[1] = EGSplot->PlotIZ;
       update_table(  vi, 0, 2, PlotRegionsTable );

          vi[0] = EGSplot->SpecPlotStart;
          vi[1] = EGSplot->SpecPlotStop;
          update_table(  vi, 0, 2, SpecPlotTable );


}

void inputRZImpl::update_source_type()
{

   QString str = sourceComboBox->currentText();
   src20GroupBox->setEnabled( false );
   in_particleButtonGroup->setEnabled(true);
   imodeComboBox->hide();
   phasespaceGroupBox->setEnabled( false );
   if ( (allRadioButton->isChecked()      ||
         chargedRadioButton->isChecked()) &&
         str.toInt(0, 10) < 21) {
       phRadioButton->setChecked( true );
   }
   allRadioButton->setEnabled( false );
   chargedRadioButton->setEnabled( false );

   /*
   QToolTip::add(  temp1Edit, tr( "" ) );
   QToolTip::add(  temp2Edit, tr( "" ) );
   QToolTip::add(  temp3Edit, tr( "" ) );
   QToolTip::add(  temp4Edit, tr( "" ) );
   QToolTip::add(  temp5Edit, tr( "" ) );
   QToolTip::add(  temp6Edit, tr( "" ) );
   QToolTip::add(  temp7Edit, tr( "" ) );
   */

   /*
   Q3WhatsThis::add(  temp1Edit, tr( "" ) );
   Q3WhatsThis::add(  temp2Edit, tr( "" ) );
   Q3WhatsThis::add(  temp3Edit, tr( "" ) );
   Q3WhatsThis::add(  temp4Edit, tr( "" ) );
   Q3WhatsThis::add(  temp5Edit, tr( "" ) );
   Q3WhatsThis::add(  temp6Edit, tr( "" ) );
   Q3WhatsThis::add(  temp7Edit, tr( "" ) );
   */

   temp1Edit->setWhatsThis(tr( "" ) );
   temp2Edit->setWhatsThis(tr( "" ) );
   temp3Edit->setWhatsThis(tr( "" ) );
   temp4Edit->setWhatsThis(tr( "" ) );
   temp5Edit->setWhatsThis(tr( "" ) );
   temp6Edit->setWhatsThis(tr( "" ) );
   temp7Edit->setWhatsThis(tr( "" ) );

   temp1Edit->setToolTip(tr( "" ) );
   temp2Edit->setToolTip(tr( "" ) );
   temp3Edit->setToolTip(tr( "" ) );
   temp4Edit->setToolTip(tr( "" ) );
   temp5Edit->setToolTip(tr( "" ) );
   temp6Edit->setToolTip(tr( "" ) );
   temp7Edit->setToolTip(tr( "" ) );

   modifyAllWidgets( temp1Edit->parent(),true,false );
   modifyAllWidgets( temp7Edit->parent(),true,false );

   beamLabel->hide();
   beamButton->hide();
   temp1Label->setText("temp1");
   temp2Label->setText("temp2");
   temp3Label->setText("temp3");
   temp4Label->setText("temp4");
   temp5Label->setText("temp5");
   temp6Label->setText("temp6");
   temp7Label->setText("temp7");

   energyButtonGroup->setEnabled( true );
   set_mono_spectrum();

   switch ( str.toInt(0, 10) ) {
     case 0:

        enableAllWidgets( temp1Edit->parent(),true );
        temp1Label->setText("RBEAM");
        temp2Label->setText("UINC");
        temp3Label->setText("VINC");
        temp4Label->setText("WINC");
        sourceoptionsGroupBox->setTitle(
        "PARALLEL BEAM INCIDENT FROM THE FRONT" );
        /*
        QToolTip::add(  temp1Edit,tr( "radius of parallel beam in cm"
                          "\n(defaults to maximum geometry radius)" ) );
        QToolTip::add(  temp2Edit, tr( "incident x-axis direction cosine" ) );
        QToolTip::add(  temp3Edit, tr( "incident y-axis direction cosine" ) );
        QToolTip::add(  temp4Edit, tr( "incident z-axis direction cosine" ) );
        */
        /*
        Q3WhatsThis::add(temp1Edit, tr( "radius of parallel beam in cm"
                          "\n(defaults to maximum geometry radius)" ) );
        Q3WhatsThis::add(  temp2Edit, tr( "incident x-axis direction cosine" ) );
        Q3WhatsThis::add(  temp3Edit, tr( "incident y-axis direction cosine" ) );
        Q3WhatsThis::add(  temp4Edit, tr( "incident z-axis direction cosine" ) );
        */
        temp1Edit->setWhatsThis(tr( "radius of parallel beam in cm"
                          "\n(defaults to maximum geometry radius)" ) );
        temp2Edit->setWhatsThis(tr( "incident x-axis direction cosine" ) );
        temp3Edit->setWhatsThis(tr( "incident y-axis direction cosine" ) );
        temp4Edit->setWhatsThis(tr( "incident z-axis direction cosine" ) );
        temp1Edit->setToolTip(tr( "radius of parallel beam in cm"
                          "\n(defaults to maximum geometry radius)" ) );
        temp2Edit->setToolTip(tr( "incident x-axis direction cosine" ) );
        temp3Edit->setToolTip(tr( "incident y-axis direction cosine" ) );
        temp4Edit->setToolTip(tr( "incident z-axis direction cosine" ) );
     break;
     case 1:
        temp1Edit->setEnabled( true );
        temp2Edit->setEnabled( true );
        temp1Label->setEnabled( true );
        temp2Label->setEnabled( true );
        temp1Label->setText("DISTZ");
        temp2Label->setText("RBEAM");
       sourceoptionsGroupBox->setTitle(
               "POINT SOURCE ON AXIS INCIDENT FROM FRONT" );
       /*
       Q3WhatsThis::add(  temp1Edit,
       tr( "distance from front of target in cm.\n(defaults to 100 cm)" ) );
       Q3WhatsThis::add(  temp2Edit,
       tr("beam radius at front of target in cm.\n(defaults to MAX radius)"));
       */
       temp1Edit->setWhatsThis(tr( "distance from front of target in cm.\n(defaults to 100 cm)" ) );
       temp2Edit->setWhatsThis(tr("beam radius at front of target in cm.\n(defaults to MAX radius)"));

       temp1Edit->setToolTip(tr( "distance from front of target in cm.\n(defaults to 100 cm)" ) );
       temp2Edit->setToolTip(tr("beam radius at front of target in cm.\n(defaults to MAX radius)"));
       break;
     case 2:
       sourceoptionsGroupBox->setTitle(
               "BROAD PARALLEL BEAM INCIDENT FROM FRONT" );
       break;
     case 3:
        enableAllWidgets( temp1Edit->parent(),true );
        temp1Label->setText("RMINBM");
        temp2Label->setText("RBEAM");
        temp3Label->setText("ZSMIN");
        temp4Label->setText("ZSMAX");
        sourceoptionsGroupBox->setTitle(
                "ISOTROPICALLY RADIATING DISK OF FINITE SIZE" );
        /*
        Q3WhatsThis::add(  temp1Edit, tr( "inner radius of source region" ) );
        Q3WhatsThis::add(  temp2Edit, tr( "outer radius of source region" ) );
        Q3WhatsThis::add(  temp3Edit, tr( "min z value for source" ) );
        Q3WhatsThis::add(  temp4Edit, tr( "max z value for source" ) );
        */
        temp1Edit->setWhatsThis(tr( "inner radius of source region" ) );
        temp2Edit->setWhatsThis(tr( "outer radius of source region" ) );
        temp3Edit->setWhatsThis(tr( "min z value for source" ) );
        temp4Edit->setWhatsThis(tr( "max z value for source" ) );

        temp1Edit->setToolTip(tr( "inner radius of source region" ) );
        temp2Edit->setToolTip(tr( "outer radius of source region" ) );
        temp3Edit->setToolTip(tr( "min z value for source" ) );
        temp4Edit->setToolTip(tr( "max z value for source" ) );
        break;
     case 4:
        temp1Edit->setEnabled( true );
        temp1Label->setEnabled( true );
        temp1Label->setText("RCAXIS");
        sourceoptionsGroupBox->setTitle("CENTRAL AXIS FLUENCE VS BEAM RADIUS");
        //Q3WhatsThis::add(  temp1Edit,
          //      tr( "radius of central axis scoring zone (cm)" ) );
        temp1Edit->setWhatsThis(tr( "radius of central axis scoring zone (cm)" ) );
        temp1Edit->setToolTip(tr( "radius of central axis scoring zone (cm)" ) );
        break;
     case 10:
        temp1Edit->setEnabled( true );
        temp2Edit->setEnabled( true );
        temp1Label->setEnabled( true );
        temp2Label->setEnabled( true );
        temp1Label->setText("XBEAM");
        temp2Label->setText("ZBEAM");
        sourceoptionsGroupBox->setTitle(
                "PARALLEL BEAM INCIDENT FROM THE SIDE" );
        /*
        Q3WhatsThis::add(  temp1Edit,
                tr( "half-width of the rectangular beam in cm" ) );
        Q3WhatsThis::add(  temp2Edit,
                tr( "half-height of the rectangular beam in cm" ) );
        */
        temp1Edit->setWhatsThis(tr( "half-width of the rectangular beam in cm" ) );
        temp2Edit->setWhatsThis(tr( "half-height of the rectangular beam in cm" ) );

        temp1Edit->setToolTip(tr( "half-width of the rectangular beam in cm" ) );
        temp2Edit->setToolTip(tr( "half-height of the rectangular beam in cm" ) );
        break;
     case 11:
        temp1Edit->setEnabled( true );
        temp2Edit->setEnabled( true );
        temp3Edit->setEnabled( true );
        temp1Label->setEnabled( true );
        temp2Label->setEnabled( true );
        temp3Label->setEnabled( true );
        temp1Label->setText("DISTRH");
        temp2Label->setText("XBEAM");
        temp3Label->setText("ZBEAM");
        sourceoptionsGroupBox->setTitle("POINT SOURCE INCIDENT FROM THE SIDE");
        /*
        Q3WhatsThis::add(  temp1Edit,
                tr( "source distance from middle of target in cm" ) );
        Q3WhatsThis::add(  temp2Edit,
                tr( "beam half-width at center of target in cm" ) );
        Q3WhatsThis::add(  temp3Edit,
                tr( "beam half-height at center of target in cm" ) );
        */
        temp1Edit->setWhatsThis( tr( "source distance from middle of target in cm" ) );
        temp2Edit->setWhatsThis(tr( "beam half-width at center of target in cm" ) );
        temp3Edit->setWhatsThis(tr( "beam half-height at center of target in cm" ) );

        temp1Edit->setToolTip( tr( "source distance from middle of target in cm" ) );
        temp2Edit->setToolTip(tr( "beam half-width at center of target in cm" ) );
        temp3Edit->setToolTip(tr( "beam half-height at center of target in cm" ) );
        break;
     case 12:
        temp1Edit->setEnabled( true );
        temp2Edit->setEnabled( true );
        temp1Label->setEnabled( true );
        temp2Label->setEnabled( true );
        temp1Label->setText("DISTRH");
        temp2Label->setText("DISTZ");
        sourceoptionsGroupBox->setTitle( "POINT SOURCE OFF AXIS" );
        /*
        Q3WhatsThis::add(temp1Edit, tr("point source distance off the Z-axis"));
        Q3WhatsThis::add(temp2Edit, tr( "perpendicular point source distance"
                                       " away from front face" ) );
        */
        temp1Edit->setWhatsThis(tr("point source distance off the Z-axis"));
        temp2Edit->setWhatsThis(tr( "perpendicular point source distance"
                                       " away from front face" ) );

        temp1Edit->setToolTip(tr("point source distance off the Z-axis"));
        temp2Edit->setToolTip(tr( "perpendicular point source distance"
                                       " away from front face" ) );
        break;
     case 13:
        temp1Edit->setEnabled( true );
        temp2Edit->setEnabled( true );
        temp3Edit->setEnabled( true );
        temp1Label->setEnabled( true );
        temp2Label->setEnabled( true );
        temp3Label->setEnabled( true );
        temp1Label->setText("UINC");
        temp2Label->setText("VINC");
        temp3Label->setText("WINC");
        sourceoptionsGroupBox->setTitle( "PARALLEL BEAM FROM ANY ANGLE" );
        /*
      	Q3WhatsThis::add(  temp1Edit, tr( "incident x-axis direction cosine" ) );
      	Q3WhatsThis::add(  temp2Edit, tr( "incident y-axis direction cosine" ) );
      	Q3WhatsThis::add(  temp3Edit, tr( "incident z-axis direction cosine" ) );
        */
        temp1Edit->setWhatsThis(tr( "incident x-axis direction cosine" ) );
        temp2Edit->setWhatsThis(tr( "incident y-axis direction cosine" ) );
        temp3Edit->setWhatsThis(tr( "incident z-axis direction cosine" ) );

        temp1Edit->setToolTip(tr( "incident x-axis direction cosine" ) );
        temp2Edit->setToolTip(tr( "incident y-axis direction cosine" ) );
        temp3Edit->setToolTip(tr( "incident z-axis direction cosine" ) );
      	break;
     case 14:
        temp1Edit->setEnabled( true );
        temp2Edit->setEnabled( true );
        temp3Edit->setEnabled( true );
        temp1Label->setEnabled( true );
        temp2Label->setEnabled( true );
        temp3Label->setEnabled( true );
        temp1Label->setText("DISTZ");
        temp2Label->setText("RBEAM");
        temp3Label->setText("RMINBM");
        sourceoptionsGroupBox->setTitle(
        "POINT SOURCE ON AXIS INCIDENT FROM FRONT"
        );
        /*
        Q3WhatsThis::add(  temp1Edit,
        tr( "point source distance from front of target in cm" ) );
        Q3WhatsThis::add(  temp2Edit,
        tr( "beam radius at front of target in cm" ) );
        Q3WhatsThis::add(  temp3Edit,
        tr( "all histories terminated below this radius \n"
            "by source routines giving them zero weight" ) );
        */
        temp1Edit->setWhatsThis(tr( "point source distance from front of target in cm" ) );
        temp2Edit->setWhatsThis(tr( "beam radius at front of target in cm" ) );
        temp3Edit->setWhatsThis(tr( "all histories terminated below this radius \n"
            "by source routines giving them zero weight" ) );

        temp1Edit->setToolTip(tr( "point source distance from front of target in cm" ) );
        temp2Edit->setToolTip(tr( "beam radius at front of target in cm" ) );
        temp3Edit->setToolTip(tr( "all histories terminated below this radius \n"
            "by source routines giving them zero weight" ) );
        break;
     case 15:
        temp1Edit->setEnabled( true );
        temp2Edit->setEnabled( true );
        temp1Label->setEnabled( true );
        temp2Label->setEnabled( true );
        temp1Label->setText("DIST");
        temp2Label->setText("ANGLE");
        sourceoptionsGroupBox->setTitle( "POINT SOURCE OFF AXIS" );
        /*
        Q3WhatsThis::add(  temp1Edit,
        tr( "distance of geometry  centre to source in cm" ) );
        Q3WhatsThis::add(temp2Edit,tr("angle of rotation around the x-axis" ) );
        */
        temp1Edit->setWhatsThis(tr( "distance of geometry  centre to source in cm" ) );
        temp2Edit->setWhatsThis(tr("angle of rotation around the x-axis" ) );

        temp1Edit->setToolTip(tr( "distance of geometry  centre to source in cm" ) );
        temp2Edit->setToolTip(tr("angle of rotation around the x-axis" ) );
        break;
     case 16:
        enableAllWidgets( temp1Edit->parent(),true );
        temp1Label->setText("DIST");
        temp2Label->setText("ANGLE");
        temp3Label->setText("TEMP3");
        temp4Label->setText("TEMP4");
        sourceoptionsGroupBox->setTitle(
        "EXTENDED (CIRC. OR RECT.) SOURCE OFF AXIS"
        );
        /*
        Q3WhatsThis::add(  temp1Edit,
        tr( "distance of geometry  centre to source in cm" ) );
        Q3WhatsThis::add(  temp2Edit,
        tr( "angle of rotation around the x-axis" ) );
        Q3WhatsThis::add(  temp3Edit, tr( SOURCE16_TEMP3 ) );
        Q3WhatsThis::add(  temp4Edit, tr( SOURCE16_TEMP4 ) );
        */
        temp1Edit->setWhatsThis(tr( "distance of geometry  centre to source in cm" ) );
        temp2Edit->setWhatsThis(tr( "angle of rotation around the x-axis" ) );
        temp3Edit->setWhatsThis(tr( SOURCE16_TEMP3 ) );
        temp4Edit->setWhatsThis(tr( SOURCE16_TEMP4 ) );

        temp1Edit->setToolTip(tr( "distance of geometry  centre to source in cm" ) );
        temp2Edit->setToolTip(tr( "angle of rotation around the x-axis" ) );
        temp3Edit->setToolTip(tr( SOURCE16_TEMP3 ) );
        temp4Edit->setToolTip(tr( SOURCE16_TEMP4 ) );
        break;
     case 20:
       sourceoptionsGroupBox->setTitle( "" );
       src20GroupBox->setEnabled( true );
       break;
     case 21:
        enableAllWidgets( temp1Edit->parent(),true );
        imodeComboBox->show();
        temp1Label->setText("IMODE");
        temp2Label->setText("NRCYCL");
        temp3Label->setText("IPARALLEL");
        temp4Label->setText("PARNUM");
        sourceoptionsGroupBox->setTitle(
                "PHASE-SPACE DATA, INCIDENT ON FRONT FACE"
        );
        /*
        Q3WhatsThis::add(  imodeComboBox, tr( "variables/record" ) );
        Q3WhatsThis::add(  temp2Edit,
        tr( "Number of times to recycle each particle in a phase\n"
            "space source." ) );
        Q3WhatsThis::add(  temp3Edit,
        tr( "Number of times to partition a phase space file.\n"
            "(only meaningful in single job runs)" ) );
        Q3WhatsThis::add(  temp4Edit,
        tr( "Phase space file partition to use (<= IPARALLEL)\n"
            "(only meaningful in single job runs)" ) );
        */
        imodeComboBox->setWhatsThis(tr( "variables/record" ) );
        temp2Edit->setWhatsThis(tr( "Number of times to recycle each particle in a phase\n"
            "space source." ) );
        temp3Edit->setWhatsThis(tr( "Number of times to partition a phase space file.\n"
            "(only meaningful in single job runs)" ) );
        temp4Edit->setWhatsThis(tr( "Phase space file partition to use (<= IPARALLEL)\n"
            "(only meaningful in single job runs)" ) );

        temp2Edit->setToolTip(tr( "Number of times to recycle each particle in a phase\n"
            "space source." ) );
        temp3Edit->setToolTip(tr( "Number of times to partition a phase space file.\n"
            "(only meaningful in single job runs)" ) );
        temp4Edit->setToolTip(tr( "Phase space file partition to use (<= IPARALLEL)\n"
            "(only meaningful in single job runs)" ) );

        phasespaceGroupBox->setEnabled( true );
        allRadioButton->setEnabled( true );
        chargedRadioButton->setEnabled( true );
        energyButtonGroup->setEnabled( false );
       //ini_energyLabel->setEnabled( false );
        IniEgroupBox->setEnabled( false );
        ini_energyEdit->setEnabled( false );
        specfnameGroupBox->setEnabled( false );
        ioutspGroupBox->setEnabled( false );

        break;
     case 22:
        enableAllWidgets( temp1Edit->parent(),true );
        enableAllWidgets( temp7Edit->parent(),true );
        imodeComboBox->show();
        temp1Label->setText("IMODE");
        temp2Label->setText("DIST");
        temp3Label->setText("ANGLE");
        temp4Label->setText("ZOFFSET");
        temp5Label->setText("NRCYCL");
        temp6Label->setText("IPARALLEL");
        temp7Label->setText("PARNUM");
        sourceoptionsGroupBox->setTitle(
                "PHASE-SPACE DATA, INCIDENT FROM ANY ANGLE"
        );
        /*
        Q3WhatsThis::add(  imodeComboBox, tr( "variables/record" ) );
        Q3WhatsThis::add(  temp2Edit,
        tr( "phase-space plane distance to point of rotation in cm" ) );
        Q3WhatsThis::add(  temp3Edit, tr( "Angle of rotation in degrees" ) );
        Q3WhatsThis::add(  temp4Edit, tr( "Point of rotation" ) );
        Q3WhatsThis::add(  temp5Edit,
        tr( "Number of times to recycle each particle in a phase\n"
            "space source." ) );
        Q3WhatsThis::add(  temp6Edit,
        tr( "Number of times to partition a phase space file.\n"
            "(only meaningful in single job runs)" ) );
        Q3WhatsThis::add(  temp7Edit,
        tr( "Phase space file partition to use\n"
            "(only meaningful in single job runs)" ) );
        */
        imodeComboBox->setWhatsThis(tr( "variables/record" ) );
        temp2Edit->setWhatsThis( tr( "phase-space plane distance to point of rotation in cm" ) );
        temp3Edit->setWhatsThis(tr( "Angle of rotation in degrees" ) );
        temp4Edit->setWhatsThis(tr( "Point of rotation" ) );
        temp5Edit->setWhatsThis(tr( "Number of times to recycle each particle in a phase\n"
            "space source." ) );
        temp6Edit->setWhatsThis(tr( "Number of times to partition a phase space file.\n"
            "(only meaningful in single job runs)" ) );
        temp7Edit->setWhatsThis(tr( "Phase space file partition to use\n"
            "(only meaningful in single job runs)" ) );

        imodeComboBox->setToolTip(tr( "variables/record" ) );
        temp2Edit->setToolTip( tr( "phase-space plane distance to point of rotation in cm" ) );
        temp3Edit->setToolTip(tr( "Angle of rotation in degrees" ) );
        temp4Edit->setToolTip(tr( "Point of rotation" ) );
        temp5Edit->setToolTip(tr( "Number of times to recycle each particle in a phase\n"
            "space source." ) );
        temp6Edit->setToolTip(tr( "Number of times to partition a phase space file.\n"
            "(only meaningful in single job runs)" ) );
        temp7Edit->setToolTip(tr( "Phase space file partition to use\n"
            "(only meaningful in single job runs)" ) );

        phasespaceGroupBox->setEnabled( true );
        allRadioButton->setEnabled( true );
        chargedRadioButton->setEnabled( true );
        energyButtonGroup->setEnabled( false );
        //ini_energyLabel->setEnabled( false );
        IniEgroupBox->setEnabled( false );
        ini_energyEdit->setEnabled( false );
        specfnameGroupBox->setEnabled( false );
        ioutspGroupBox->setEnabled( false );
        break;
     case 23:
        sourceoptionsGroupBox->setTitle("BEAM TREATMENT HEAD SIMULATION");
        hideAllWidgets( temp1Edit->parent() );
        hideAllWidgets( temp7Edit->parent() );
        beamLabel->show();
        beamButton->show();
        beamButton->setEnabled(true);
        in_particleButtonGroup->setEnabled(false);
        energyButtonGroup->setEnabled( false );
        IniEgroupBox->setEnabled(false);
        phasespaceGroupBox->setEnabled( false );
        src20GroupBox->setEnabled(false);
        break;
   }

}

void inputRZImpl::setupBeamSource()
{
    //we don't need to update pegs directory because we want access to both
    //beamDlg->update_pegs(listExtensionLess(PEGSdir,"*.pegs4dat"));
    beamDlg->move(this->pos().x()+this->width()/4,
                  this->pos().y()+this->height()/4);
    beamDlg->show();
}



void inputRZImpl::updateBeamSource(const QString& uc,
                                   const QString& pegs)
{
  beamDlg->beamUserCodeChanged( uc );

  /* want access to all pegs files in both directories
  QString pegsFile = pegs+".pegs4dat";
  QString dir = ironIt(EGS_HOME + "pegs4/data/");

  if (fileExists(dir+pegsFile)){
   beamDlg->update_pegs(listExtensionLess(dir,"*.pegs4dat"));
  }
  else{
   dir = ironIt(HEN_HOUSE + "pegs4/data/");
   if (fileExists(dir+pegsFile)){
    beamDlg->update_pegs(listExtensionLess(dir,"*.pegs4dat"));
   }
   else{beamDlg->update_pegs(listExtensionLess(PEGSdir,"*.pegs4dat"));}
  }
  */
}


void inputRZImpl::getBeamSource()
{
    QString src23  =tr("<b>user-code :</b> ")+beamDlg->beam_code()+tr("<br><br>");
            src23 +=tr("<b>input file:</b> ")+beamDlg->beam_inp()+tr("<br><br>");
            src23 +=tr("<b>pegs4 file:</b> ")+beamDlg->beam_pegs()+tr("<br><br>");
            src23 +=tr("<b>weight window:</b>  min = ")+beamDlg->beam_min();
            src23 +=tr("; max = ")           +beamDlg->beam_max()+
                    tr("<br><br><b><u>Source options</u></b><br>");
            src23 +=tr("DIST = ")+beamDlg->beam_dist()+tr(";")+
                     tr(" ANGLE = ")+beamDlg->beam_angle()+tr(";");
            src23 +=tr(" ZOFFSET = ")+beamDlg->beam_zoffset()+tr("<br>");
            src23 +=tr("XOFFSET = ")+beamDlg->beam_xoffset()+tr(";");
            src23 +=tr(" YOFFSET = ")+beamDlg->beam_yoffset();
    beamLabel->setText(src23);
    changeTextColor(beamLabel,"black");
}

/*
Unassigned media are appended to the media table unless the
medium is number 1, in which case it is inserted at the top,
setting all regions to medium 1.
And not only if medium 1 isn't assigned, but whenever the
first medium assigned to a region isn't medium 1.

It is very easy to establish a correspondence between medium number
(explicitly entered in the input file) and the proper medium name.
For this reason, I was first going through the medium numbers and
filling out the media table with the medium names and at the same
time the corresponding region numbers (plane/cyl numbers).
*/

void inputRZImpl::fill_media_table( const MGEOInputs* EGSgeo )
{

     v_string medium;
     v_int    mednum_i = EGSgeo->mednum;
     v_string media_i  = EGSgeo->media;

     v_int vr[3];
     vr[1] = EGSgeo->start_reg; vr[2] = EGSgeo->stop_reg;
     v_int vs[5];
     vs[1] = EGSgeo->start_Z; vs[2] = EGSgeo->stop_Z;
     vs[3] = EGSgeo->start_ring; vs[4] = EGSgeo->stop_ring;

// create media name list "medium" from media number list "mednum"
	for ( uint j = 0; j < mednum_i.size(); j++) {
	   if (mednum_i[j] < media_i.size()) medium.push_back( media_i[ mednum_i[j] ] );
	}
// Check that first medium is medium 1 since by default, all regions are medium 1
        if (medium[0] != media_i[1]){
           medium.insert( medium.begin(), media_i[1] );
           if (EGSgeo->description_by.toLower() == "regions") {
                vr[1].insert( vr[1].begin(), 2);
                vr[2].insert( vr[2].begin(), EGSgeo->radii.size() * (EGSgeo->nslabs()) + 1);
           }
           else{
                vs[1].insert( vs[1].begin(), 1);
                vs[2].insert( vs[2].begin(), EGSgeo->nslabs());
                vs[3].insert( vs[3].begin(), 1);
                vs[4].insert( vs[4].begin(), EGSgeo->radii.size());
           }
        }
        bool med_assigned;

// check wether some media weren't assigned and add them to the table
// Medium 1 was already taken care of above ...
     for ( uint i = 0; i < media_i.size(); i++) {
         med_assigned = false;
         for ( uint j = 0; j < mednum_i.size(); j++) {
             if ( i == (uint) mednum_i[j]) { // medium asigned to regions ...
                 med_assigned = true;
                 break;
             }
         }
         if (!med_assigned && i > 1){//include media not assigned to regions
            medium.push_back( media_i[i] );
         }
     }

    v_string vstr[1];
    vstr[0] = medium;
     vector<QString> vqs[1];
     for(int j=0; j<vstr[0].size(); j++) {
         vqs[0].push_back(vstr[0][j].c_str());
     }
     //update_table(vs, 0, 1, mediaTable);
     update_table(vqs, 0, 1, mediaTable);

     if (EGSgeo->description_by.toLower() == "regions")
               update_table(vr, 1, 3, mediaTable);
     else      update_table(vs, 1, 5, mediaTable);

}

void inputRZImpl::update_MediaInput()
{
     MGEOInputs* EGSgeo = new MGEOInputs;
     EGSgeo = GetGEO(); // get geometry and media info from form
     update_mediaTable( EGSgeo );
     zap(EGSgeo);
}

// Gets media list from the table when loading a pegs4 file.
v_string inputRZImpl::getMediaFromTable(){

  v_string tmpMed, medium;

  //get media from table
  get_col_explicit( 0, mediaTable, tmpMed, string("NO MEDIUM") );
  //-----------------------------------------
  //        create media list
  //-----------------------------------------
  for ( uint k = 0; k < tmpMed.size() ; k++){
      if ( tmpMed[k] != "NO MEDIUM")
         medium.push_back( tmpMed[k] );
  }
  //remove duplicates
  return strip_repetitions( medium );
}

void inputRZImpl::update_mediaTable( const MGEOInputs* EGSgeo )
{

	clear_table(mediaTable);

 	mediaTable->horizontalHeader()->setUpdatesEnabled( false );

	if ( EGSgeo->description_by == "regions"){

           mediaTable->setColumnCount(3);

	   int width = mediaTable->columnWidth(0);

           mediaTable->setHorizontalHeaderItem(1,new QTableWidgetItem("start region"));
           mediaTable->setHorizontalHeaderItem(2,new QTableWidgetItem("stop region"));

	   mediaTable->horizontalHeader()->setUpdatesEnabled( true );

	   mediaTable->showColumn(1);
	   mediaTable->showColumn(2);

	   mediaTable->setColumnWidth(1,width);
	   mediaTable->setColumnWidth(2,width);
	}
	else {

           mediaTable->setColumnCount(5);

	   int halfWidth = mediaTable->columnWidth(0)/2;

           mediaTable->setHorizontalHeaderItem(1,new QTableWidgetItem("start Z"));
           mediaTable->setHorizontalHeaderItem(2,new QTableWidgetItem("stop Z"));
           mediaTable->setHorizontalHeaderItem(3,new QTableWidgetItem("start R"));
           mediaTable->setHorizontalHeaderItem(4,new QTableWidgetItem("stop R"));

	   mediaTable->horizontalHeader()->setUpdatesEnabled( true );

	   mediaTable->showColumn(1);
	   mediaTable->showColumn(2);
	   mediaTable->showColumn(3);
	   mediaTable->showColumn(4);

	   mediaTable->setColumnWidth(1,halfWidth);
	   mediaTable->setColumnWidth(2,halfWidth);
	   mediaTable->setColumnWidth(3,halfWidth);
	   mediaTable->setColumnWidth(4,halfWidth);

	}

       if ( EGSgeo->mednum.size() > 0 && EGSgeo->media.size() > 0 )
            fill_media_table( EGSgeo );

}

//Table used to keep media currently selected in mediaTable in case where
//the medium data is changed
void inputRZImpl::reset_mediaTable ()
{
    for(int row=0; row<mediaTable->rowCount(); row++) {
       QWidget *editor = mediaTable->cellWidget( row, 0 );
       if(editor) {
         if(string(editor->metaObject()->className())=="QComboBox") {
            QComboBox *eeditor = (QComboBox *)editor;
            QString str = eeditor->currentText();
            //qt3to4 -- BW
            //mediaTable->clearCellWidget(row, 0);
            //mediaTable->setText(row,0,str);
            mediaTable->removeCellWidget(row,0);
            mediaTable->setItem(row,0,new QTableWidgetItem(str));
         }
       }
    }
}

//do the same for customFFTable
void inputRZImpl::reset_customFFTable ()
{
    //qt3to4 -- BW
    //for(int row=0; row<customFFTable->numRows(); row++) {
    for(int row=0; row<customFFTable->rowCount(); row++) {
       QWidget *editor = customFFTable->cellWidget( row, 0 );
       if(editor) {
         if(string(editor->metaObject()->className())=="QComboBox") {
            QComboBox *eeditor = (QComboBox *)editor;
            QString str = eeditor->currentText();
            //customFFTable->clearCellWidget(row, 0);
            //customFFTable->setText(row,0,str);
            customFFTable->removeCellWidget(row, 0);
            customFFTable->setItem(row,0,new QTableWidgetItem(str));
         }
       }
    }
}

void inputRZImpl::update_usercode_open()
{
    update_usercode();
    if (egs_dir_changed)
     change_input_file();
}


void inputRZImpl::update_usercode()
{

    if (cavrzRadioButton->isChecked()){
       usercodename   = "cavrznrc";
       usercode   = cavrznrc;
       cavityRadioButton->setEnabled( true );

       //etransportComboBox->setEnabled( false );
       //etransportLabel->setEnabled( false );

       ifullComboBox->clear();
       ifullComboBox->addItem( tr( "dose and stoppers" ) );
       ifullComboBox->addItem( tr( "Aatt and Ascat" ) );
       ifullComboBox->addItem( tr( "Ap" ) );
       ifullComboBox->addItem( tr( "Afl and <s>g/w" ) );

       outoptComboBox->clear();
       outoptComboBox->addItem( tr( "short" ) );
       outoptComboBox->addItem( tr( "cavity details" ) );


       TabWidgetRZ->setTabEnabled(TabWidgetRZ->indexOf(CItab), true );

       set_cav_regions();

       photonSplitGroupBox->setEnabled( true );
       BremsSplitGroupBox->setEnabled( false );

       //Q3WhatsThis::add( CSEnhancementGroupBox, CS_ENHANCEMENT_CAVRZNRC  );
       CSEnhancementGroupBox->setWhatsThis(CS_ENHANCEMENT_CAVRZNRC  );
       CSEnhancementGroupBox->setToolTip(CS_ENHANCEMENT_CAVRZNRC  );

       CSEnhancementTable->setEnabled( false );
       CSEnhancement_RegionsLabel1->setEnabled( false );
       CSEnhancement_RegionsLabel2->setEnabled( false );

       TabWidgetRZ->setTabEnabled(TabWidgetRZ->indexOf(PLOTtab), false );

    }
    else {
       cavityRadioButton->setEnabled( false );

       if (cavityRadioButton->isChecked() ){
          cavityRadioButton->setChecked( false );
          individualRadioButton->setChecked( true );
      }

       TabWidgetRZ->setTabEnabled(TabWidgetRZ->indexOf(CItab), false );

       photonSplitGroupBox->setEnabled( false );
       BremsSplitGroupBox->setEnabled( true );
    }

    if (dosrzRadioButton->isChecked()){

       usercode   = dosrznrc;
       usercodename   = "dosrznrc";

       DoseRegGroupBox->setEnabled( true );
       kermaGroupBox->setEnabled( true );
       photregGroupBox->setEnabled( false );

       etransportComboBox->setEnabled( true );
       etransportLabel->setEnabled( true );

       ifullComboBox->clear();
       ifullComboBox->addItem( tr( "dose and stoppers" ) );
       ifullComboBox->addItem( tr( "entrance regions"  ) );
       ifullComboBox->addItem( tr( "pulse height distribution" ) );
       ifullComboBox->addItem( tr( "scatter fraction" ) );

       outoptComboBox->clear();
       outoptComboBox->addItem( tr( "short" ) );
       outoptComboBox->addItem( tr( "dose summary" ) );
       outoptComboBox->addItem( tr( "material summary" ) );
       outoptComboBox->addItem( tr( "material and dose summary" ) );
       outoptComboBox->addItem( tr( "long" ) );

       //Q3WhatsThis::add( CSEnhancementGroupBox, CS_ENHANCEMENT_DOSRZNRC  );
       CSEnhancementGroupBox->setWhatsThis(CS_ENHANCEMENT_DOSRZNRC  );
       CSEnhancementGroupBox->setToolTip(CS_ENHANCEMENT_DOSRZNRC  );

       CSEnhancementTable->setEnabled( true );
       CSEnhancement_RegionsLabel1->setEnabled( true );
       CSEnhancement_RegionsLabel2->setEnabled( true );

       TabWidgetRZ->setTabEnabled(TabWidgetRZ->indexOf(PLOTtab), true );

       eminusPlotCheckBox->setEnabled( false );
       eplusPlotCheckBox->setEnabled( false );
       ePlotCheckBox->setEnabled( false );
       gammaPlotCheckBox->setEnabled( false );
       DrawFluPlotsGroupBox->setEnabled( false );
       SpecPlotGroupBox->setEnabled( false );

    }
    else {
       DoseRegGroupBox->setEnabled( false );
       kermaGroupBox->setEnabled( false );
       photregGroupBox->setEnabled( true );
       phdGroupBox->setEnabled( false );

       etransportComboBox->setEnabled( false );
       etransportLabel->setEnabled( false );

       eminusPlotCheckBox->setEnabled( true );
       eplusPlotCheckBox->setEnabled( true );
       ePlotCheckBox->setEnabled( true );
       gammaPlotCheckBox->setEnabled( true );
       DrawFluPlotsGroupBox->setEnabled( true );
       SpecPlotGroupBox->setEnabled( true );

    }

    SPRRegGroupBox->setEnabled( false );

    if ( ( sprrzRadioButton->isChecked() ) ||
         ( flurzRadioButton->isChecked() ) ){
	     ifullGroupBox->setEnabled( false );
                   outoptComboBox->setEnabled( false );
                   outoptTextLabel->setEnabled( false );
	     statEdit->setEnabled( false );
	     statLabel->setEnabled( false );

                   CSEnhancementGroupBox->setEnabled( false );

                   if (sprrzRadioButton->isChecked()){
                       usercodename   = "sprrznrc";
                       usercode   = sprrznrc;
	         SPRRegGroupBox->setEnabled( true );
                       TabWidgetRZ->setTabEnabled(TabWidgetRZ->indexOf(PLOTtab), false );
                  }

                 if (flurzRadioButton->isChecked()){
                     usercodename   = "flurznrc";
                     usercode   = flurznrc;
                     PrintFluSpecGroupBox->setEnabled( true );
                     IPRIMARYGroupBox->setEnabled( true );
                     sloteFluGroupBox->setEnabled( true );

                     TabWidgetRZ->setTabEnabled(TabWidgetRZ->indexOf(PLOTtab), true );

                     LinePrnOutCheckBox->setEnabled( false );
                     ExtPlotOutCheckBox->setChecked( true );
                     ExtPlotOutCheckBox->setEnabled( false );

	       photregGroupBox->setEnabled( false );
                     RRGroupBox->setEnabled( false );
                     ExpTrafoCGroupBox->setEnabled( false );

                     PlotRegionsGroupBox->setTitle( "Regions to plot integral fluence vs position" );
                }
                else {
                     PrintFluSpecGroupBox->setEnabled( false );
                     IPRIMARYGroupBox->setEnabled( false );
                     sloteFluGroupBox->setEnabled( false );

	       photregGroupBox->setEnabled( true );
                     RRGroupBox->setEnabled( true );
                     ExpTrafoCGroupBox->setEnabled( true );

                     PlotRegionsGroupBox->setTitle( "Define plot regions" );
               }

    }
    else {
              ifullGroupBox->setEnabled( true );
              outoptComboBox->setEnabled( true );
              outoptTextLabel->setEnabled( true );
             statEdit->setEnabled( true );
             statLabel->setEnabled( true );

             PrintFluSpecGroupBox->setEnabled( false );
             IPRIMARYGroupBox->setEnabled( false );
             sloteFluGroupBox->setEnabled( false );

             CSEnhancementGroupBox->setEnabled( true );
              RRGroupBox->setEnabled( true );
               ExpTrafoCGroupBox->setEnabled( true );

             LinePrnOutCheckBox->setEnabled( true );
            //ExtPlotOutCheckBox->setChecked( false );
            ExtPlotOutCheckBox->setEnabled( true );

             PlotRegionsGroupBox->setTitle( "Define plot regions" );
    }

    set_working_area(); // this changes EGSdir

    checkCompilationAbility();

/*
    if (!check_file(EGSdir + EGSfileName) || EGSfileName.isEmpty() ){// if no file, nothing to do
         ExecuteButton->setEnabled( false );
         PreviewRZButton->setEnabled( false );
         PrintButton->setEnabled( false );
    }
    else{
         ExecuteButton->setEnabled( true );
         PreviewRZButton->setEnabled( true );
         PrintButton->setEnabled( true );
    }
*/
}

/*
void inputRZImpl::update_run_mode()
{

   if (standaloneRadioButton->isChecked()){

      parallelGroupBox->setEnabled( false );
      run_parallel = false;
   }
   else {

      parallelGroupBox->setEnabled( true );
      if ( NumJobSpinBox->value() <= 1 )
	  StartJobSpinBox->setEnabled( false );
      else
	  StartJobSpinBox->setEnabled( true );
      run_parallel = true;
   }

   // update run mode
   if ( run_parallel ) {
       num_jobs = NumJobSpinBox->value();
       ini_job  = StartJobSpinBox->value();
       queue    = QueueComboBox->currentText();
   }

}
*/

void inputRZImpl::set_working_area()
{
// note that PEGSDir is set initially where it finds pegsfile
// therefore we are just setting the dir for inp files
   QString tmpDir;
   //qt3to4 -- BW
   //char s = QDir::separator();
   QChar s = QDir::separator();

   if ( HOMERadioButton->isChecked() ){
      tmpDir   = ironIt( EGS_HOME + s + usercodename + s);
   }

   if ( HEN_HOUSERadioButton->isChecked() ){
      tmpDir   = ironIt( HEN_HOUSE + s + (QString)"user_codes" + s + usercodename + s);
   }

   if ( OtherAreaRadioButton->isChecked() ){
       tmpDir = The_Other_Area;
       /*
       QString tmp1 = EGS_HOME + s + usercodename + s;
       QString tmp2 = HEN_HOUSE + s + usercodename + s;
       if (EGSdir != tmp1 && EGSdir != tmp2)
           tmpDir   = EGSdir; //<=== leave it unchanged
       else
           tmpDir   = EGS_HOME + s;//<== requesting to go somewhere else
      */
   }

   if ( EGSdir != tmpDir ) {
       EGSdir = tmpDir;
       egs_dir_changed = true;
   }
   else
       egs_dir_changed = false;

   if ( egs_dir_changed ){
        disconnect( InputFileComboBox, SIGNAL( editTextChanged(const QString&) ), this,
  	                                               SLOT( EGSFileNameChanged(const QString&) ) );
         update_files( EGSdir, InputFileComboBox, "*.egsinp" );
         connect( InputFileComboBox, SIGNAL( editTextChanged(const QString&) ), this,
		                              SLOT( EGSFileNameChanged(const QString&) ) );
  }

}

void inputRZImpl::set_data_area()
{
  QString tmpDir;
   //qt3to4 -- BW
   //char s = QDir::separator();
   char s = QDir::separator().toLatin1();

   if ( HOMEPegsRadioButton->isChecked() )
       tmpDir  = ironIt( EGS_HOME + s + "pegs4" + s + "data" + s);

   if ( HEN_HOUSEPegsRadioButton->isChecked() )
      tmpDir  = ironIt( HEN_HOUSE + s + "pegs4" + s + "data" + s);

   if ( OtherPegsAreaRadioButton->isChecked() )
      tmpDir  = The_Other_PEGS;

   if ( PEGSlessRadioButton->isChecked() ){
      is_pegsless=true;
      TabWidgetRZ->setTabEnabled(TabWidgetRZ->indexOf(MDTab), true );
      pegs4GroupBox->setEnabled(false);
      tmpDir = "";
      //see if any media are already defined
      listMedia = getPEGSLESSMedia();
      updateMediaLists();
   }
   else{
      is_pegsless=false;
      TabWidgetRZ->setTabEnabled(TabWidgetRZ->indexOf(MDTab), false );
      pegs4GroupBox->setEnabled(true);
   }

   if ( PEGSdir != tmpDir ) {
       PEGSdir = tmpDir;
       pegs_dir_changed = true;
   }
   else
       pegs_dir_changed = false;

   if ( pegs_dir_changed && !is_pegsless)
       update_files( PEGSdir, pegs4ComboBox, "*.pegs4dat" );// add list of pegsdat files


}

void inputRZImpl::update_from_user_area()
{
    EGSdir   = ironIt( EGSdir );
    //qt3to4 -- BW
    //char s = QDir::separator();
    QChar s =  QDir::separator();

    if ( EGSdir == ironIt( HEN_HOUSE + s + QString("user_codes") + s + usercodename + s ) )
        HEN_HOUSERadioButton->setChecked( true );
    else if ( EGSdir == ironIt( EGS_HOME + s + usercodename + s ) )
        HOMERadioButton->setChecked( true );
    else{
        OtherAreaRadioButton->setChecked( true );
    }
}

void inputRZImpl::update_from_data_area()
{

    PEGSdir   = ironIt( PEGSdir );

    //qt3to4 -- BW
    //char s = QDir::separator();
    QChar s = QDir::separator();
    if ( PEGSdir == ironIt( HEN_HOUSE+s+"pegs4"+s+"data"+s ) )
        HEN_HOUSEPegsRadioButton->setChecked( true );
    else if ( PEGSdir == ironIt( EGS_HOME+s+"pegs4"+s+"data"+s ) )
        HOMEPegsRadioButton->setChecked( true );
    else
        OtherPegsAreaRadioButton->setChecked( true );

}

void inputRZImpl::update_EGSdir( const QString& newDir )
{
    if (EGSdir != newDir ) {
        EGSdir = newDir;
        egs_dir_changed = true;
    }
    else   egs_dir_changed = false;

     update_from_user_area();
}

//!  Updates the contents of the file name combo box with the files
//   in rDirName that pass the filter rFilter.
/*!
Every time this function is invoked, \em rDirName is checked for existence.
If it does not exist, a warning is issued and the function exits. In the case
of user code directories, it checks in both places, \em $HOME
and \em $HEN_HOUSE.If the directory exists,  the file name combo box is
cleared and updated with the files in this directory that satisfied the filter,
sorted alphabetically by name (ignoring case).
*/
void inputRZImpl::update_files( const QString & rDirName, QComboBox* cb, const QString & rFilter )
{

    QString dirName = rDirName;
    QDir d( dirName );
    QChar s = QDir::separator();
    QString homeDir = EGS_HOME.endsWith(s) ? EGS_HOME + usercodename + s : EGS_HOME + s + usercodename + s;
    QString hen_houseDir = HEN_HOUSE.endsWith(s) ? HEN_HOUSE + usercodename + s : HEN_HOUSE + s + usercodename + s;

    if ( !d.exists() ) {
       if      ( dirName == homeDir )      dirName = hen_houseDir;
       else if ( dirName == hen_houseDir ) dirName = homeDir;

         if ( !d.cd( dirName)) {
             QString information = (QString)"<b>Directory</b> " +
                                                   dirName      +
                                   (QString)" used in your input file does not exist !<br>";
             openErrors += information;
             return;
       }
       if ( dirName == homeDir || dirName == hen_houseDir )
          update_EGSdir( dirName );
    }


    d.setNameFilters( rFilter.split(" ") );
    d.setSorting( QDir::Name | QDir::IgnoreCase );

    const QFileInfoList list = d.entryInfoList();
    QFileInfoList::const_iterator it;
    QFileInfo fi;

    QString tmpText;
    if ( cb->isEditable() ) {
         tmpText = cb->currentText();
         cb->clear();
         cb->setEditText ( tmpText );
    }
    else {
         tmpText = cb->currentText();
         cb->clear();
    }
    QStringList lst;
    if (!tmpText.isEmpty()) lst += tmpText;
    //Add_New_Item( tmpText, cb );

   //qt3to4 -- BW
   //while ( (fi=it.current()) ) {           // for each file...
   for ( it=list.constBegin(); it!=list.constEnd(); ++it ) {
     //qt3to4 -- BW
     //++it;                               // go to next list element
     //qt3to4 -- BW
     //if ( fi->fileName() == ".." || fi->fileName() == "." )
       fi = *it;
       if ( fi.fileName() == ".." || fi.fileName() == "." )
         continue;
     //qt3to4 -- BW
     //lst += fi->fileName();
       lst += fi.fileName();
    }
    cb->addItems( lst );
}

void inputRZImpl::set_local_external()
{
 if ( localRadioButton->isChecked() ) {
	 raddistTable->setEnabled( true );
	 raddistfnameGroupBox->setEnabled( false );
	 ioutrdistGroupBox->setEnabled( false );
 }
 if ( externalRadioButton->isChecked() ) {
   DeactivateTable( raddistTable );
	 raddistTable->setEnabled( false );
	 raddistfnameGroupBox->setEnabled( true );
	 ioutrdistGroupBox->setEnabled( true );
 }
}

void inputRZImpl::set_mono_spectrum()
{
 if ( monoenergeticRadioButton->isChecked() ) {
	 ini_energyEdit->setEnabled( true );
	 //ini_energyLabel->setEnabled( true );
         IniEgroupBox->setEnabled( true );
	 specfnameGroupBox->setEnabled( false );
	 ioutspGroupBox->setEnabled( false );

 }
 if ( spectrumRadioButton->isChecked() ) {
	 ini_energyEdit->setEnabled( false );
	 //ini_energyLabel->setEnabled( false );
         IniEgroupBox->setEnabled( false );
	 specfnameGroupBox->setEnabled( true );
	 specfnameComboBox->setEnabled( true );
	 ioutspGroupBox->setEnabled( true );
 }

}

void inputRZImpl::set_cavity()
{
        if (cavityRadioButton->isChecked()){

	   DeactivateTable( geometryTable );
	   DeactivateTable( cylTable );
                 geometryTable->setEnabled( false );
                 cylTable->setEnabled( false );
	   DeactivateTable( mediaTable );
	   mediaGroupBox->setEnabled( false );
	   ZFaceLabel->setEnabled( false );
	   ZFaceEdit->setEnabled( false );

	}
	else {

	   geometryTable->setEnabled( true );
	   cylTable->setEnabled( true );
	   mediaGroupBox->setEnabled( true );
	   ZFaceLabel->setEnabled( true );
	   ZFaceEdit->setEnabled( true );
	}
}

void inputRZImpl::set_group()
{
    //qt3to4 -- BW
    //if ( geometryTable->numCols() == 1){
    if ( geometryTable->columnCount() == 1){
       v_float slab_thickness;
       get_col_content( 0,  geometryTable, slab_thickness );

       //qt3to4 -- BW
       QRect vrect = geometryTable->visibleRegion().boundingRect();
       int width = vrect.width()/2;
       //int width  = geometryTable->visibleWidth()/2;

       int width0 = geometryTable->columnWidth(0);
       int width1 = 5*width0/8;
            width0 = 3*width0/8;

      geometryTable->horizontalHeader()->setUpdatesEnabled( false );
      //qt3to4 -- BW
      //geometryTable->setNumCols(2);
      //geometryTable->horizontalHeader()->setLabel(0,"# slabs");
      //geometryTable->horizontalHeader()->setLabel(1,"thickness [cm]");
      geometryTable->setColumnCount(2);
      geometryTable->setHorizontalHeaderItem(0,new QTableWidgetItem("# slabs"));
      geometryTable->setHorizontalHeaderItem(1,new QTableWidgetItem("thickness [cm]"));
      geometryTable->horizontalHeader()->setUpdatesEnabled( true );

      geometryTable->showColumn(0);
      geometryTable->showColumn(1);

      geometryTable->setColumnWidth( 0,  width0);
      geometryTable->setColumnWidth( 1,  width1);

      // clear the whole column
     clear_col( geometryTable, 0);
     clear_col( geometryTable, 1);

      v_int v[1];
      v[0] = globalNSlab;
      update_table(v, 0, 1, geometryTable);
      v_float vs[2];
      vs[0] = slab_thickness;
      vs[1] = slab_thickness;
      update_table(vs, 1, 2, geometryTable);

   }


  // ZFaceLabel->setEnabled( true );
   //ZFaceEdit->setEnabled( true );

}

void inputRZImpl::set_individual()
{
    // if switching to individual input mode, save # slabs info in global variables
    //qt3to4 -- BW
    //if ( geometryTable->numCols() == 2){
    if ( geometryTable->columnCount() == 2){
       v_float cyl_depth;
       get_col_content( 0,  geometryTable, globalNSlab );
       get_col_content( 1,  geometryTable, cyl_depth );
       int width0 = geometryTable->columnWidth(0);
       int width1 = geometryTable->columnWidth(1);

       geometryTable->horizontalHeader()->setUpdatesEnabled( false );
       //geometryTable->horizontalHeader()->setLabel(0,"       ");
      //qt3to4 -- BW
      //geometryTable->setNumCols(1);
      //geometryTable->horizontalHeader()->setLabel(0,"depth [cm]");
      geometryTable->setColumnCount(1);
      geometryTable->setHorizontalHeaderItem(0,new QTableWidgetItem("depth [cm]"));
      geometryTable->horizontalHeader()->setUpdatesEnabled( true );
      geometryTable->showColumn(0);
      //geometryTable->showColumn(1);
      geometryTable->setColumnWidth( 0,  width0+width1);
      //geometryTable->setColumnWidth( 1,  width1);

      // clear the whole column
     clear_col( geometryTable, 0);

//     int row = geometryTable->currentRow();
//     geometryTable->clearCellWidget( row, 0);
//     geometryTable->setColumnReadOnly ( 0, true );

     v_float vs[1];
     vs[0] = cyl_depth;
      update_table(vs, 0, 1, geometryTable);
   }

   //ZFaceLabel->setEnabled( false );
   //ZFaceEdit->setEnabled( false );

}

void inputRZImpl::activate_PulseHDistInputs()
{
	if ( ( (ifullComboBox->currentText()).toLower() == "pulse height distribution") &&
 	     ( usercodename.toLower() == "dosrznrc" ) ) {
	   phdGroupBox->setEnabled( true );
	}
	else {
	   phdGroupBox->setEnabled( false );
	}
}

void inputRZImpl::activate_fluence_table()
{
	if ( (PrintFluSpeComboBox->currentText()).toLower() == "specified" ) {
	   ListFluTable->setEnabled( true );
	}
	else {
	   ListFluTable->setEnabled( false );
	}
}

void inputRZImpl::activate_ff_table()
{
	if ( (RayleighcomboBox->currentText()).toLower() == "custom" ) {
	   customFFgroupBox->setEnabled( true );
	}
	else {
	   customFFgroupBox->setEnabled( false );
	}
}

void inputRZImpl::show_help()
{
#ifdef WIN32
    QString ENVVAR = "ProgramFiles";
    QString ProgramFiles      = getenv(ENVVAR.toLatin1());
    QString p1 = ProgramFiles+"\\Internet Explorer\\Iexplore ";
                 p1 += GUI_HOME;
                 p1 += "html\\" + QString(PIRS801);
                 //p1 += "html\\index.html";
   // QProcess related code
    //qt3to4 -- BW
    //Q3Process* proc = new Q3Process( this );
    QProcess* proc = new QProcess( this );
     // Set up the command and arguments.
    //qt3to4 -- BW
    //proc->setArguments ( QStringList::split( " ",p1,false ) );
    QStringList args = p1.split(" ");
    QString prog = args.first();
    args = args.mid(1);
    proc->start(prog, args);
    //qt3to4 -- BW
    //if ( !proc->start() ) {
    if(proc->error()==QProcess::FailedToStart){
        // error handling
        QString errorM = p1;
        QMessageBox::warning( this,
                tr("Error"),
	            tr("Could not start the command: \n") + errorM,
                1,0,0 );
    }
#else
    QStringList browser;
    browser.append("firefox");
    browser += "konqueror";
    browser += "seamonkey";
    browser += "opera";
    browser += "chrome";
    //QString strr = QString(" ") + GUI_HOME + QString("html/index.html");
    QString strr = QString(" ") + GUI_HOME + QString("html/") + QString(PIRS801);
    QString str  = browser[0] + strr;
    //qt3to4 -- BW
    //Q3Process* proc = new Q3Process( this ); int i = 0;
    QProcess* proc = new QProcess( this ); int i = 0;
    //qt3to4 -- BW
    //proc->setArguments ( QStringList::split( " ",str,false ) );
    QString prog = browser[0];
    QStringList args = str.split(" ");
    args = args.mid(1);
    //while( !proc->start() ){
    proc->start(prog, args);
    while(proc->error()==QProcess::FailedToStart) {
         if (i >= browser.size()-1 ){
            QString error  = "Couldn't find any of these browsers:\n";
                    error += browser.join("\n");
            QMessageBox::warning ( this, "Beware", error, 1, 0, 0 );
            break;
         }
         else{
            //qt3to4 -- BW
            //proc->clearArguments(); str  = browser[++i] + strr;
            //proc->setArguments ( QStringList::split( " ",str,false ) );
            prog = browser[++i];
            proc->start(prog, args);
         }
    }
#endif

}

void inputRZImpl::run_previewRZ()
{
   QRadioButton *rb[3] ;
   rb[0] = groupRadioButton ;
   rb[1] = individualRadioButton ;
   rb[2] = cavityRadioButton ;
   QString inp_meth = TextRadioBChecked(3, rb);
   if ( inp_meth != "cavity information" ){
       QString p  = HEN_HOUSE;

       p += "/previewRZ/previewRZ.tcl " + EGSdir + EGSfileName;
#ifndef WIN32
       //p += "/previewRZ/previewRZ "     + EGSdir + EGSfileName;
#else
       //p += "/previewRZ/previewRZ.tcl " + EGSdir + EGSfileName;
       p.prepend( "wish.exe ");
#endif

       //p += "/previewRZ/previewRZ " + EGSdir + EGSfileName + " &";
       p = ironIt( p );
//     if ( system( p ) )
//        QMessageBox::warning ( this, "Beware",
//                               tr("error running \n"+p),
//                               1, 0, 0 );

       // QProcess related code
       //qt3to4 -- BW
       //Q3Process* proc_view = new Q3Process( this );
       QProcess* proc_view = new QProcess( this );
       // Set up the command and arguments.
       //qt3to4 -- BW
       QStringList args = p.split(" ");
       QString prog = args.first();
       args = args.mid(1);
       //proc_view->setArguments ( QStringList::split( " ",p,false ) );
//#ifdef WIN32
//       QString the_path    = getenv("PATH");
//       QString the_pathext = getenv("PATHEXT");
//       QString the_error   = p +
//                             (QString)"\n PATH = "+ the_path +
//                             (QString)"\nPATHEXT=" + the_pathext;
//       QStringList env;
//                   env.append((QString)"PATH="    + the_path);
//                   env.append((QString)"PATHEXT=" + the_pathext);
//       if ( !proc_view->launch( QString::null, &env) ) {
//#else
       QString the_error = p;
       //qt3to4 -- BW
       proc_view->start(prog,args);
       //if ( !proc_view->start() ) {
       if(proc_view->error()==QProcess::FailedToStart){
//#endif
         // error handling
         QMessageBox::critical( 0,
                tr("Error"),
	        tr("Could not start the command: \n %1").arg(the_error),
                tr("Quit") );
       }

   }
   else {
       QString error  = "Can't read cavity information, aborting previewRZ !\n";
               error += "previewRZ does not offer this option yet !!!";
       QMessageBox::warning ( this, "Beware", error, 1, 0, 0 );
   }
}

void inputRZImpl::cleanChecked( bool status )
{
      if ( CleanradioButton->isChecked() )
         ExecuteButton->setEnabled( false );
      else
         //checkErrors();
         checkExecutionAbility();
}

QString inputRZImpl::getExecutable()
{
    if ( CONFcomboBox->count() == 0            ||
         CONFcomboBox->currentText().isEmpty() ){
	return QString::null;
    }

    QString machine = readVarFromConf("my_machine");

    //qt3to4 -- BW
    //char s = QDir::separator();
   char s = QDir::separator().toLatin1();

   QString executable = usercodename;
   if ( DebugradioButton->isChecked() )
        executable += "_debug";
   else if ( NoOptradioButton->isChecked() )
        executable += "_noopt";
   else if ( CleanradioButton->isChecked() )
       return QString::null;
#ifdef WIN32
    executable += ".exe";
#endif
    executable = EGS_HOME + s + "bin" + s + machine + s + executable;

    QFile fexe( executable );
    if ( fexe.exists() )
       return executable;
    else
       return QString::null;
}

void inputRZImpl::set_electr()
{
  if ( parallelplateRadioButton->isChecked() ) {
     electradEdit->setEnabled( false ) ;
     electradLabel->setEnabled( false ) ;
     electradEdit->setText( "0.0" ) ;
     //EGScav->electr_rad = 0.0; do I need this really ????

     //electrmatEdit->setEnabled( false ) ;
     electrmatComboBox->setEnabled( false ) ;
     electrmatLabel->setEnabled( false ) ;
  }
  else {
     electradEdit->setEnabled( true ) ;
     electradLabel->setEnabled( true ) ;

     //electrmatEdit->setEnabled( true ) ;
     electrmatComboBox->setEnabled( true ) ;
     electrmatLabel->setEnabled( true ) ;
  }
}

//qt3to4 -- BW
//bool inputRZImpl::IsByRegionsEnabled( QComboBox* cb, Q3Table* table )
bool inputRZImpl::IsByRegionsEnabled( QComboBox* cb, QTableWidget* table )
{
   bool set_by_regions;
   //qt3to4 -- BW
   //Q3GroupBox* frame = (Q3GroupBox*)table->parent();
   QGroupBox* frame = (QGroupBox*)table->parent();
   QString state = "Turn OFF ";

   if ( cb->currentText().toLower() == "on in regions" ) {
        set_by_regions = true ;
        state = "Turn ON ";
   }
   else if ( cb->currentText().toLower() == "off in regions" ) {
        set_by_regions = true ;
        state = "Turn OFF ";
   }
   else {
        set_by_regions = false ;
        DeactivateTable( table );
        state = "";
   }

   table->setEnabled( set_by_regions );
   frame->setTitle( state + frame->windowTitle() );

   return set_by_regions;
}

//qt3to4 -- BW
//bool inputRZImpl::IsByRegionsEnabled( QCheckBox* chk, Q3Table* table )
bool inputRZImpl::IsByRegionsEnabled( QCheckBox* chk, QTableWidget* table )
{
   bool set_by_regions;

   if ( chk->isChecked() ) {
        set_by_regions = true ;
   }
   else {
        set_by_regions = false ;
        DeactivateTable( table );
   }

   table->setEnabled( set_by_regions );

   return set_by_regions;
}

//qt3to4 -- BW
//bool inputRZImpl::IsByRegionsEnabled( QRadioButton* rbOn, QRadioButton* rbOff, Q3Table* table )
bool inputRZImpl::IsByRegionsEnabled( QRadioButton* rbOn, QRadioButton* rbOff, QTableWidget* table )
{
   bool set_by_regions;
   //qt3to4 -- BW
   //Q3GroupBox* frame = (Q3GroupBox*)table->parent();
   QGroupBox* frame = (QGroupBox*)table->parent();
   QString state = "Turn OFF ";

   if ( rbOn->isChecked() ) {
      	set_by_regions = true ;
        state = "Turn ON ";
   }
   else if ( rbOff->isChecked() ) {
      	set_by_regions = true ;
        state = "Turn OFF ";
   }
   else {
      	set_by_regions = false ;
        DeactivateTable( table );
        state = "";
   }

   table->setEnabled( set_by_regions );
   table->setEnabled( set_by_regions );
   frame->setTitle( state + frame->windowTitle() );
   return set_by_regions;
}

void inputRZImpl::EnableTransportParamByRegions()
{
   //TabWidgetRZ->setTabEnabled(MCTPRegTab, true );

   bool by_reg = false ;
   bool by_reg_tmp;

   by_reg_tmp = IsByRegionsEnabled( BoundComptoncomboBox,
                                    BoundComptonTable );
   by_reg = by_reg || by_reg_tmp;
   by_reg_tmp = IsByRegionsEnabled( PEcomboBox,
                                    PEAngSamplingTable );
   by_reg = by_reg || by_reg_tmp;
   by_reg_tmp = IsByRegionsEnabled( RayleighcomboBox,
                                    RayleighTable );
   by_reg = by_reg || by_reg_tmp;
   by_reg_tmp = IsByRegionsEnabled( RelaxationcomboBox,
                                    RelaxationsTable );
   by_reg = by_reg || by_reg_tmp;

   by_reg_tmp = IsByRegionsEnabled( PCUTCheckBox, PCUTTable );
   by_reg = by_reg || by_reg_tmp;
   by_reg_tmp = IsByRegionsEnabled( ECUTCheckBox, ECUTTable );
   by_reg = by_reg || by_reg_tmp;
   by_reg_tmp = IsByRegionsEnabled( SMAXCheckBox, SMAXTable );
   by_reg = by_reg || by_reg_tmp;

   TabWidgetRZ->setTabEnabled(TabWidgetRZ->indexOf(MCTPRegTab), by_reg );
}

//qt3to4 -- BW
//void inputRZImpl::DeactivateTable( Q3Table* rTable )
void inputRZImpl::DeactivateTable( QTableWidget* rTable )
{// Needed to avoid segmentation fault when disabling table with
 // active cell. Once I learn a bit more, this might fade away!!!
 // Last edited cell is lost
     unsigned row = rTable->currentRow();
     unsigned col = rTable->currentColumn();
     //qt3to4 -- BW
     //rTable->clearCellWidget( row, col );
     rTable->removeCellWidget( row, col );
}

void inputRZImpl::Enable_BremsSplit()
{
     if ( BremsSplitCheckBox->isChecked() ){
          BremsSplitSpinBox->setEnabled( true );
          BremsSplitTextLabel->setEnabled( true );
     }
     else {
          BremsSplitSpinBox->setEnabled( false );
          BremsSplitTextLabel->setEnabled( false );
     }

}

void inputRZImpl::enable_plot()
{
     if ( plotCheckBox->isChecked() ) {
          PlotGroupBox->setEnabled( true );
          enable_external_plot();
     }
     else {
          PlotGroupBox->setEnabled( false );
     }
}

void inputRZImpl::enable_external_plot()
{
     if ( ExtPlotOutCheckBox->isChecked() ) {
          ExternalPlotTypeGroupBox->setEnabled( true );
     }
     else {
          ExternalPlotTypeGroupBox->setEnabled( false );
     }
}

#define ABOUT_NRC  "Copyright (C) year National Research Council Canada"
#define ABOUT_TEXT "Graphical User Interface for EGSnrc RZ user codes"\
                   "\n\n            egs_inprz, version YYY "\
                   "\n\nAuthors: Ernesto Mainegra, Iwan Kawrakow, and Blake Walters"\
                   "\n\nThis program is distributed under the terms of the"\
                   "\nGNU Affero General Public License, version 3."\
                   "\n\nUsing Qt Version XXX"

void inputRZImpl::show_about()
{

  QString qt_version(qVersion());
  QString aboutText = QString(ABOUT_TEXT);
  aboutText.replace("XXX",qt_version); aboutText.replace("YYY",tr(VERSION));//defined in datainp.h
  QString nrcText = QString(ABOUT_NRC); nrcText.replace("year",the_year);
  QString title = QString("About egs_inprz");
  QMessageBox::about ( this, title, nrcText+"\n\n"+aboutText );

}
void inputRZImpl::show_errors()
{
 CommandManager* d = new CommandManager( this, "Error Dialog",
                     confErrors +
                     openErrors +
                     pegsErrors +
                     previewErrors +
                    formErrors );
 d->exec();
 zap(d);
}

void inputRZImpl::caught_errors()
{
    if ( ! confErrors.isEmpty() ||
         ! openErrors.isEmpty() ||
         ! pegsErrors.isEmpty() ||
         ! previewErrors.isEmpty() ||
         ! formErrors.isEmpty() ){
         logfileButton->setEnabled( true );
    }
    else logfileButton->setEnabled( false );
}


void inputRZImpl::setupDefaultSettingsEditor()
{
/*
    QDockWindow *dw = new QDockWindow( QDockWindow::OutsideDock, this );
    dw->setResizeEnabled( TRUE );
    dw->setCloseMode( QDockWindow::Always );
    settingsEditor = new PropertyEditor( dw );
    dw->setWidget( settingsEditor );
    dw->setFixedExtentWidth( 250 );
    dw->setWindowTitle( tr( "Default Settings Editor" ) );
    QWhatsThis::add( settingsEditor,
		     tr("<b>The Default Settings Editor</b>"
			"<p>You can change the appearance and behavior of the selected widget in the "
			"property editor.</p>"
			"(These connections can also be made using the connection tool.)" ) );
    dw->show();
*/

 //MDefaultSettingsForm* dlg = new MDefaultSettingsForm();
 //dlg->show();
}

void inputRZImpl::SetValidator( )
{
  //QIntValidator*        vInt     = new QIntValidator( this );
  QDoubleValidator* vDou   = new QDoubleValidator( this );
  QDoubleValidator* vDpos = new QDoubleValidator( this );
  vDpos->setBottom( 0.0 );

// accept only positive values
   ncaseEdit->setValidator( vDpos );
   maxCPUEdit->setValidator( vDpos );
   statEdit->setValidator( vDpos );
    wallthickEdit->setValidator( vDpos );
    cavradEdit->setValidator( vDpos );
    cavlenEdit->setValidator( vDpos );
    electradEdit->setValidator( vDpos );
    ini_energyEdit->setValidator( vDpos );
    GPCUTEdit->setValidator( vDpos );
    GECUTEdit->setValidator( vDpos );
    ESTEPEEdit->setValidator( vDpos );
    ESAVEINEdit->setValidator( vDpos );

// accept any values
    sloteFluEdit->setValidator( vDou );
    SLOTEEdit->setValidator( vDou );
    DELTAEEdit->setValidator( vDou );
    ZFaceEdit->setValidator( vDou );
    temp1Edit->setValidator( vDou );
    temp2Edit->setValidator( vDou );
    temp3Edit->setValidator( vDou );
    temp4Edit->setValidator( vDou );
    temp5Edit->setValidator( vDou );
    temp6Edit->setValidator( vDou );
    temp7Edit->setValidator( vDou );
    GSMAXEdit->setValidator( vDou );
    XImaxEdit->setValidator( vDou );
    SkinDepthEdit->setValidator( vDou );
    RRDepthEdit->setValidator( vDou );
    RRFractionEdit->setValidator( vDou );
    ExpTrafoCEdit->setValidator( vDou );

}

void inputRZImpl::update_photon_forcing( )
{
    if (PhotonForcingCheckBox->isChecked()){
	StartForcingLabel->setEnabled( true );
	StopForcingLabel->setEnabled( true );
	StartForcingSpinBox->setEnabled( true );
	StopForcingSpinBox->setEnabled( true );
    }
    else{
	StartForcingLabel->setEnabled( false );
	StopForcingLabel->setEnabled( false );
	StartForcingSpinBox->setEnabled( false );
	StopForcingSpinBox->setEnabled( false );
    }
}

void inputRZImpl::update_range_rejection( )
{
    if (eRangeRejCheckBox->isChecked()){
	ESAVEINLabel->setEnabled( true );
	ESAVEINEdit->setEnabled( true );
    }
    else{
	ESAVEINLabel->setEnabled( false );
	ESAVEINEdit->setEnabled( false );
    }
}

//qt3to4 -- BW
//void inputRZImpl::InitializeTable( Q3Table* t, const QStringList& s )
void inputRZImpl::InitializeTable( QTableWidget* t, const QStringList& s)
{

    t->horizontalHeader()->setUpdatesEnabled( false );
    t->setHorizontalHeaderLabels(s);
    t->horizontalHeader()->setUpdatesEnabled( true );
    //Proper way to resize table columns to fit the table -- EMH July 2015
    #if QT_VERSION >= 0x050000
        t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    #else
        t->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    #endif
    t->installEventFilter(this);}

//qt3to4 -- BW
//void inputRZImpl::InitializeTable( Q3Table* t, const QStringList& s, v_float frac )
void inputRZImpl::InitializeTable( QTableWidget* t, const QStringList& s, v_float frac)
{
   /* Avoid accessing unallocated memory by using minimum value */
   //qt3to4 -- BW
   //int ncols = t->numCols();
   int ncols = t->columnCount();
   if (ncols>s.size()) ncols = s.size();
   if (ncols>frac.size()) ncols = frac.size();

   t->horizontalHeader()->setUpdatesEnabled( false );
   //qt3to4 -- BW
   t->setHorizontalHeaderLabels(s);
   t->horizontalHeader()->setUpdatesEnabled( true );

   //Better viewport width estimate -- EMH July 2015
   int width = t->horizontalHeader()->width();
   for ( int i = 0;  i <  ncols; i++ )  {
     t->setColumnWidth( i, frac[i]*width );
   }

   //Proper way to resize last column to fit the table -- EMH July 2015
   t->horizontalHeader()->setStretchLastSection(true);

   t->installEventFilter(this);
}

//qt3to4 -- BW
//void inputRZImpl::InitializeTable( Q3Table* t, const QString& s0, const QString& s1 )
void inputRZImpl::InitializeTable( QTableWidget* t, const QString& s0, const QString& s1)
{
    t->horizontalHeader()->setUpdatesEnabled( false );
    //qt3to4 -- BW
    t->setHorizontalHeaderItem(0,new QTableWidgetItem(s0));
    t->setHorizontalHeaderItem(1,new QTableWidgetItem(s1));
    t->horizontalHeader()->setUpdatesEnabled( true );
    //Proper way to resize table columns to fit the table -- EMH July 2015
    #if QT_VERSION >= 0x050000
        t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    #else
        t->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    #endif

    t->installEventFilter(this);
}

//qt3to4 -- BW
//void inputRZImpl::InitializeTable( Q3Table* t, const QString& s0,
void inputRZImpl::InitializeTable( QTableWidget* t, const QString& s0,
                                              const QString& s1,
                                              const QString& s2)
{
    //qt3to4 -- BW
    t->horizontalHeader()->setUpdatesEnabled( false );
    t->setHorizontalHeaderItem(0, new QTableWidgetItem(s0) );
    t->setHorizontalHeaderItem(1, new QTableWidgetItem(s1) );
    t->setHorizontalHeaderItem(2, new QTableWidgetItem(s2) );
    t->horizontalHeader()->setUpdatesEnabled( true );
    //Proper way to resize table columns to fit the table -- EMH July 2015
    #if QT_VERSION >= 0x050000
        t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    #else
        t->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    #endif

    t->installEventFilter(this);
}


//qt3to4 -- BW
//void inputRZImpl::InitializeTwoColumnTable( Q3Table* table )
void inputRZImpl::InitializeTwoColumnTable( QTableWidget* table)
{
    //qt3to4 -- BW
    table->horizontalHeader()->setUpdatesEnabled( false );
    table->setHorizontalHeaderItem(0, new QTableWidgetItem("start"));
    table->setHorizontalHeaderItem(1, new QTableWidgetItem("stop"));
    table->horizontalHeader()->setUpdatesEnabled( true );
    //Proper way to resize table columns to fit the table -- EMH July 2015
    #if QT_VERSION >= 0x050000
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    #else
        table->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    #endif

    table->installEventFilter(this);
}

//qt3to4 -- BW
//void inputRZImpl::InitializeThreeColumnTable( Q3Table* table, const QString& rvalue )
void inputRZImpl::InitializeThreeColumnTable( QTableWidget* table, const QString& rvalue)
{
     //qt3to4 -- BW
    table->horizontalHeader()->setUpdatesEnabled( false );
    table->setHorizontalHeaderItem(0, new QTableWidgetItem(rvalue));
    table->setHorizontalHeaderItem(1, new QTableWidgetItem("start"));
    table->setHorizontalHeaderItem(2, new QTableWidgetItem("stop"));
    table->horizontalHeader()->setUpdatesEnabled( true );
    //Proper way to resize table columns to fit the table -- EMH July 2015
    #if QT_VERSION >= 0x050000
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    #else
        table->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    #endif

    table->installEventFilter(this);
}
/**********************************************************************
 * Reads data files in HEN_HOUSE/data and figures out which are
 * the prefixes used for the photon xsection data bases. The convention
 * is prefix_pair.data, prefix_rayleigh.data etc. Here we check for
 * *_pair.data as the initial key and make sure the rest of the files
 * are also in that location. A list of prefixes is then shown in the
 * GUI's combo box photonXSectioncomboBox.
 **********************************************************************/
void inputRZImpl::InitializePhotonXSection(){

photonXSectioncomboBox->clear();

QString key            = "_pair.data";
QString data_dir       = ironIt(HEN_HOUSE+QString("/data/"));
QStringList ini = strip_extension(list_items(data_dir,QString("*")+key),key);
if (ini.isEmpty()) {
    pegsErrors += QString("<br><b>No files found with key *") + key +
                  QString("<br>in directory ") + data_dir +
                  QString("</b><br>");
    return;
}

QStringList prefix;
for(QStringList::Iterator it = ini.begin(); it != ini.end(); ++it ) {
    if(fileExists(data_dir+(*it)+"_rayleigh.data") &&
       fileExists(data_dir+(*it)+"_photo.data")    &&
       fileExists(data_dir+(*it)+"_triplet.data")  ){
       prefix += (*it);
    }
}
/* Insert prefixes requesting use of either xcom or epdl library
 * with renormalized photoelectric xsections */
prefix +="mcdf-xcom"; prefix +="mcdf-epdl";
/* Insert prefix "pegs4" which requests use of the pegs4 data */
prefix +="PEGS4";

photonXSectioncomboBox->addItems(prefix);

}

/**********************************************************************
 * Reads data files in HEN_HOUSE/data and figures out which are the
 * suffixes used for the EII xsection data bases. Convention is
 * eii_suffix.data. We check for eii_*.data files.
 * A list of suffixes is then shown in the GUI's combo box EIIcomboBox.
 *
 **********************************************************************/
void inputRZImpl::InitializeEIIXSection(){

EIIcomboBox->clear();

QString prefix = "eii_", ext = ".data",
        data_dir = ironIt(HEN_HOUSE+QString("/data/"));
QStringList suffix = strip_str(strip_str(list_items(data_dir,prefix+QString("*")),ext),prefix);
if (suffix.isEmpty()) {
    pegsErrors += QString("<br><b>No ")+  prefix +
                  QString("_*.data files found ") +
                  QString("<br>in directory ") + data_dir +
                  QString("</b><br>");
    return;
}
suffix.prepend("Off"); suffix.prepend("On");
EIIcomboBox->addItems(suffix);

}

/*******************************/
// Event filter for InputRZForm:
/*******************************/
// Currently catches QTableWidget events for custom editing.
// Implemented operations such as copy, cut, paste and delete.
// If end of table reached, pressing enter adds an extra row.
// QTableWidget objects need to invoke installEventFilter(this)
// to use this eventFilter.
//                                            -- EMH July 2015
/***************************************************************/
bool inputRZImpl::eventFilter(QObject *o, QEvent *e){

   //if (o==cylTable){cout << "It caught an event on cylTable!!!" << endl;}

   if (string(o->metaObject()->className())=="QTableWidget"){
      //cout << "It caught an event on table " << o->objectName().toStdString().c_str() << endl;

     QTableWidget *to = (QTableWidget *)o;
     if( e->type() == QEvent::KeyPress) {
         QKeyEvent *ke = (QKeyEvent*)e;
         // Return advances one row, unless at bottom
         // in which case a row is added
         if ( ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter ) {
           if( to->currentRow() >= to->rowCount() - 1 ){
              to->setRowCount(to->rowCount() + 1 );
              to->setCurrentCell(to->rowCount()-1,to->currentColumn());
           }
           else to->setCurrentCell(to->currentRow()+1,to->currentColumn());
           return true;
         }
         //Contents of selected cells deleted
         if ( ke->key() == Qt::Key_Delete ){
             QList<QTableWidgetSelectionRange> ts = to->selectedRanges();
             if(!ts.isEmpty()) {
               for (int irow = ts.first().topRow(); irow <= ts.first().bottomRow(); irow++) {
                 for(int icol = ts.first().leftColumn(); icol <= ts.first().rightColumn(); icol++) {
                     to->setItem(irow,icol,0);
                 }
               }
             }
             return true;
         }
         //Contents of selected cells copied to a clipboard and deleted
         if ( ke->key() == Qt::Key_X && ke->modifiers() == Qt::ControlModifier ){
             QString cellText; itemCopy.clear(); copyRange.clear();
             QList<QTableWidgetSelectionRange> ts = to->selectedRanges();
             if(!ts.isEmpty()) {
               for (int irow = ts.first().topRow(); irow <= ts.first().bottomRow(); irow++) {
                 for(int icol = ts.first().leftColumn(); icol <= ts.first().rightColumn(); icol++) {
                     QTableWidgetItem *w = to->item(irow,icol);
                     if(w) cellText = w->text();
                     if ( !cellText.isEmpty() ){
                        itemCopy << cellText;
                     }
                     else
                        itemCopy << "";
                     to->setItem(irow,icol,0);
                 }
               }
               copyRange = ts;
               //cout << itemCopy.join(", ").toLatin1().data() << endl;
             }
             return true;
         }
         //Contents of selected cells copied to a clipboard
         if ( ke->key() == Qt::Key_C && ke->modifiers() == Qt::ControlModifier ) {
             QString cellText; itemCopy.clear(); copyRange.clear();
             QList<QTableWidgetSelectionRange> ts = to->selectedRanges();
             if(!ts.isEmpty()) {
                for ( int irow = ts.first().topRow(); irow <= ts.first().bottomRow(); irow++){
                     for ( int icol = ts.first().leftColumn(); icol <= ts.first().rightColumn(); icol++){
                         QTableWidgetItem *w = to->item(irow,icol);
                         if(w) cellText = w->text();
                         if ( !cellText.isEmpty() ){
                            itemCopy << cellText;
                         }
                         else
                            itemCopy << " ";
                     }
                }
                copyRange = ts;
                //cout << itemCopy.join(", ").toLatin1().data() << endl;
             }
             else {
                  QTableWidgetItem *w = to->item(to->currentRow(), to->currentColumn());
                  if (w) cellText = w->text();
                  if ( !cellText.isEmpty() )
                       itemCopy << cellText;
                  else itemCopy << "";
             }
             return true;
         }
         //Full content of the clipboard pasted at first cell of a selected cell range.
         if ( ke->key() == Qt::Key_V && ke->modifiers() == Qt::ControlModifier &&
              !itemCopy.isEmpty() && !copyRange.isEmpty()){
            QList<QTableWidgetSelectionRange> cs = to->selectedRanges();
            uint top = cs.first().topRow(), left = cs.first().leftColumn(), icount = 0;
            QTableWidgetSelectionRange ts = QTableWidgetSelectionRange(
                                            top , left,
                                            top  + copyRange.first().rowCount()-1,
                                            left + copyRange.first().columnCount()-1);
            for ( int irow = ts.topRow(); irow <= ts.bottomRow(); irow++){
                for ( int icol = ts.leftColumn(); icol <= ts.rightColumn(); icol++){
                    if ( ++icount <= itemCopy.size() )
                       to->setItem(irow, icol, new QTableWidgetItem(itemCopy[icount-1]));
                }
            }
            return true;
         }
     }
     //return to->eventFilter(o, e) ;
     return false;
   }
   else{
     return false;
   }

}
