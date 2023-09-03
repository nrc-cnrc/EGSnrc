/*
###############################################################################
#
#  EGSnrc egs_inprz implementation
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
#  Author:          Ernesto Mainegra-Hing, 2001
#
#  Contributors:    Iwan Kawrakow
#                   Blake Walters
#                   Cody Crewson
#                   Reid Townson
#
###############################################################################
*/


#include "tooltips.h"
#include "inputRZImpl.h"
#include "errordlg.h"
#include "commandManager.h"
#include "executiondlgImpl.h"
#include <qlineedit.h>
#include <qapplication.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <iterator>
#include <qtabwidget.h>
#include <qpushbutton.h>
#include <qdir.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qprinter.h>
#include <qvalidator.h>
#include <typeinfo>
#include <qsettings.h>
#include <qlibrary.h>
#include <egs_config_reader.h>

#include <QTextStream>
#include <iostream>
#include <QFileDialog>
#include <QProgressDialog>
using namespace std;

#define zapM(x)  cout<<"deleting "<< string(x->metaObject()->className())<< endl;if(x){delete(x);x=0;}

inputRZImpl::inputRZImpl( QWidget* parent, const char* name,
             bool modal, Qt::WindowFlags f )
             : QWidget(parent)
{

    setupUi(this);
    Initialize();
    SetValidator( );
    connect( PrintButton, SIGNAL(clicked()), this, SLOT(print()) );

}

inputRZImpl::~inputRZImpl()
{
  zap( mediaTable );
}

//**********************************
//*****   COMPILATION STUFF    *****
//**********************************
void inputRZImpl::compile_userCode()
{
    //qt3to4
    //char s = QDir::separator();
    char s = QDir::separator().toLatin1();
    QString egs_compiler = readVarFromConf("make_prog");
    if ( egs_compiler.isEmpty() ) egs_compiler = "make";// default is GNU make

    // If the make command contains an argument already (e.g. -j12) we need to
    // separate the actual make command in the QStringList
    QStringList args = egs_compiler.split(" ");

   QString code_generation = "opt"; // default
   if ( DebugradioButton->isChecked() )
        code_generation = "debug";
   else if ( NoOptradioButton->isChecked() )
        code_generation = "noopt";
   else if ( CleanradioButton->isChecked() )
        code_generation = "clean";
   else{// optimization desired
       code_generation = "opt"; // default
   }

   QDir::setCurrent( ironIt( EGS_HOME + QDir::separator() + usercodename) );

   args += code_generation;
//   if ( code_generation.toLower() != "clean"){
   QString dummy =  (QString)"EGS_CONFIG=" +
                     ironIt( HEN_HOUSE + s +
                     (QString)"specs"  + s +
                     CONFcomboBox->currentText());
   args += dummy;
   //args += dummy.replace("\\", "/");
//   }

   QString vTitle = "Compiling RZ user code " + usercodename.toUpper();
   // let's run the compiler comand
   CommandManager* compile_egs = new CommandManager( this, vTitle.toLatin1().data(), args );
   //compile_egs->show(); //runs modeless
   compile_egs->exec(); //runs modal
   //checkErrors();
   checkExecutionAbility();
}

//**********************************
//*******   EXECUTION STUFF   ******
//**********************************
void inputRZImpl::run_userCode()
{
     QString inpf  = EGSfileName;
     //qt3to4 -- BW
     //char s = QDir::separator();
     char s = QDir::separator().toLatin1();
     QString the_user_code_area = ironIt( EGS_HOME+s+usercodename+s );

     if ( EGSdir != the_user_code_area  ){
	 QString exe_msg   =
             "Do you want to copy the input file to your user code area?\n";
                 exe_msg+="Current input file location is: "+EGSdir;
                 exe_msg+="\nIf not, execution will abort !";
	switch( QMessageBox::warning( this, "user code area :" +
                the_user_code_area,
                exe_msg,
                "Yes",  "No", 0, 0, 1 ) ){
	case 0: // The user clicked the yes button or pressed Enter
	                  copy( EGSdir                         +EGSfileName,
			  the_user_code_area+EGSfileName);
		    break;
	case 1: // The user clicked the no or pressed Escape
		    return;
		    break;
	}

     }

//    inpf.remove(inpf.find(".egsinp",1),7);
//  PEGSdir is updated on getPEGSfile and a separator is appended
     QString datf;
     if(is_pegsless) datf = "pegsless";
     else  datf  = PEGSdir + PEGSfileName;
//     datf.remove(datf.find(".pegs4dat",1),9);
     QString exe = getExecutable();
     if (exe.isEmpty() ){
        QString exe_error = "No executable found, try compiling first !";
        QMessageBox::critical( 0,  tr("Error !!!"), tr( exe_error.toLatin1() ), tr("OK") );
        return;
     }

     QDir::setCurrent( EGS_HOME + s + usercodename );
     //QDir::setCurrent( EGSdir );

/*  -----------------------------------------------
    This is used to run the user code interactively
     -----------------------------------------------
     QString vTitle = "Running : " + exe;
     QStringList args;
     args.push_back( exe );
      args += "-i"; args += inpf ;
      args += "-p"; args += datf ;
    CommandManager* run_egs = new CommandManager( 0, vTitle, args );
    run_egs->show(); //runs modeless
    ------------------------------------------------
*/

     QString exec_str = HEN_HOUSE    + " " +
			usercodename + " " +
			exe                       + " " +
			inpf                      + " " +
			datf;
     ExecutiondlgImpl* executionDialog = new ExecutiondlgImpl( this, exec_str.toLatin1(),
                                                               false, 0);
     executionDialog->inputFileLabel->setText(EGSfileName);
#ifdef WIN32
      executionDialog->batchRadioButton->setEnabled(false);
#endif
     executionDialog->show();
}

void inputRZImpl::set_cav_regions()
{
  if ( ( groupRadioButton->isChecked()      ) ||
       ( individualRadioButton->isChecked() ) ){
       gr_indGroupBox->setEnabled( true );
       cavityGroupBox->setEnabled( false );
       CavityInfoLabel->hide();
  }
  else {
       gr_indGroupBox->setEnabled( false );
       cavityGroupBox->setEnabled( true );
       CavityInfoLabel->show();
  }
}

//**********************************************
// *********  CONFIGURATION  STUFF   ***********
//**********************************************
/* Commented out as this is not offered in egs_inprz anymore */
bool inputRZImpl::configLibExists(){
/*    char s = QDir::separator();
    QString machine = readVarFromConf( "my_machine" );
    QString lib = ironIt(HEN_HOUSE + s + (QString)"bin" + s + machine);
    QDir dir( lib );
    if ( ! dir.exists() ) return false;
    QStringList lst = dir.entryList( "*.*" );
    for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
	if ( ( *it ).contains( "egsconfig" ) ){
                confErrors = QString::null;
                return true;
	}
    }
    confErrors += (QString)"<br><b>Configuration library not found in</b> <i>" +
            lib + (QString)"</i> !!!<br>";*/
    return false;
}

QString inputRZImpl::readVarFromConf( const QString& var )
{
//qt3to4 -- BW
//char s = QDir::separator();
QChar s = QDir::separator();
QString val = QString();
QString egsconf = ((CONFcomboBox->currentText()).lastIndexOf(QDir::separator())<0)?
   ironIt( HEN_HOUSE + s + (QString)"specs" + s + CONFcomboBox->currentText() ):
   CONFcomboBox->currentText();
 if ( !egsconf.isEmpty() ){
  EGS_ConfigReader* egs = new EGS_ConfigReader(egsconf);
  int res = egs->checkConfigFile(egsconf);
  if (!res){
   val = egs->getVariable(var,true);
  }
  else if(res == 1){
   confErrors += (QString)"<br><b> Could not find or read config file: </b>" +
                 egsconf;
   confErrors += (QString)"<br> EGS_CONFIG must point to a valid file!";
   confErrors += (QString)"<br> EGS_CONFIG is needed for compiling and "+
                 (QString)"executing EGSnrc user codes.";
  }
  else if( res == 2 ) {
   confErrors += tr("<br>Reading variable <b>") + var +
                 tr("</b> from what seems to be a ")  +
#ifdef WIN32
                 tr("Unix/Linux config file!!!\n");
#else
                 tr("MS Windows config file!!!\n");
#endif

   val = egs->getVariable(var,true);

   confErrors += tr("<br><i>") + var +
                 tr(" = ")     + val +
                 tr("</i><br>");
  }
 }
 else{
  confErrors += tr("<br>Empty config file pased to <b>updateConfiguration</b>");
 }
 return val;
}

/***************************************************************************
 * Commented out as egs_inprz will not be offering this capability anymore.
 * As of the 2013 release this feature will only be offered by egs_gui as
 * a more general purpose GUI. egs_inprz should just deal with RZ codes input
 *
 *                                                         EMH, 26 March 2013
 *
 ****************************************************************************/
void inputRZImpl::configure()
{
/*    QString machine = readVarFromConf( "my_machine" );

    typedef  QDialog* (*CreateEGS)( QWidget*, const QString& );
    char        s = QDir::separator();
    QLibrary* lib = new QLibrary(  ironIt (HEN_HOUSE + s + (QString)"bin" + s +
                                   machine   + s + (QString)"egsconfig")  );

    if( !lib->load() ) {
	confErrors = (QString)"<b>Failed to load library </b><i>" +
                      lib->library().toLatin1() + (QString)"</i><br>";
        checkConfigLib();
	return;
    }
    CreateEGS create2 =(CreateEGS)  lib->resolve("create2");
    if ( create2 ) {
       QDialog *config = create2( this, EGS_CONFIG );
       if ( config->exec() == QDialog::Accepted )
	   update_conf_files();
    }
    else{
	confErrors = (QString)"Failed resolving \"create1\" !!!!<br>";
        checkConfigLib();
    }*/

}

void inputRZImpl::update_conf_files(){

    disconnect( CONFcomboBox, SIGNAL( editTextChanged(const QString&) ),
                this, SLOT( checkConfigLib() ) );
    update_files( CONFdir, CONFcomboBox, "*.conf" );
    connect( CONFcomboBox, SIGNAL( editTextChanged(const QString&) ),
             this, SLOT( checkConfigLib() ) );

 }


//**********************************************
// *********      READING STUFF      ***********
//**********************************************

//!  Verify validity of current information available in the GUI
/*!
Uses check_file() to verify that the input file exists and  pegs_is_ok()
to check whether the pegs4 data file exists and to search it  for the
media supplied in the media table of the geometry information tab .
This function also checks for errors in the information inside
an input or pegs4 file and in case errors are detected, it enables the
\em view \em errors button in the \ref  page3 "general information tab".
If the geometry information is correct the \em PreviewRZ button is enabled.
The \em Execute, \em PreviewRZ and \em Print buttons
are accordingly enabled/disabled.
*/
void inputRZImpl::checkErrors( const QString& fun )
{
    checkErrors();

//qt3to4 -- BW
//   cout << "Called from " << fun << endl;
     cout << "Called from " << fun.toStdString() << endl;

}

void inputRZImpl::checkErrors()
{

    checkExecutionAbility();

    checkConfigLib();

    checkPreviewRZ();

    checkPrintAbility();

    caught_errors();
}

void inputRZImpl::checkConfigLib(){
 QString egsconf = ((CONFcomboBox->currentText()).lastIndexOf(QDir::separator())<0)?
                   ironIt( HEN_HOUSE + QDir::separator() +
                   (QString)"specs"  + QDir::separator() +
                   CONFcomboBox->currentText() ):
                   CONFcomboBox->currentText();
 confErrors = QString();
 if (fileExists(egsconf)){
    updateConfiguration( egsconf );
    checkExecutionAbility();
    checkCompilationAbility();
    checkPreviewRZ();
    checkPrintAbility();

/*    if ( ! configLibExists() ){
      ConfigurationButton->setEnabled( false );
    }
    else{
      ConfigurationButton->setEnabled( true );
    }*/
    caught_errors();
 }
}


void inputRZImpl::checkCompilationAbility(){

  //qt3to4 -- BW
  //char s = QDir::separator();
  char s = QDir::separator().toLatin1();

  QString missing_files = QString();

  if (!check_file( HEN_HOUSE + s + "makefiles" + s + "standard_makefile" ) )
      missing_files += ironIt( HEN_HOUSE + s + "makefiles" + s + "standard_makefile<br>");
  if (!check_file( EGS_HOME + s + usercodename + s + "Makefile" ) )
      missing_files += ironIt(  EGS_HOME + s + usercodename + s + "Makefile<br>" );
  if (!check_file( EGS_HOME + s + usercodename + s +  usercodename + ".mortran" ) )
      missing_files += ironIt(  EGS_HOME + s + usercodename + s +  usercodename + ".mortran<br>" );
  if (!check_file( EGS_HOME + s + usercodename + s +  usercodename + ".make" ) )
      missing_files += ironIt(  EGS_HOME + s + usercodename + s +  usercodename + ".make<br>" );

  if ( missing_files.isEmpty() )
      compileButton->setEnabled(true);
  else{
      compileButton->setEnabled(false);
      missing_files =
      "<br><b>Compilation disabled since following files missing:</b><br>" +
                      missing_files;
      confErrors += missing_files;
  }

  caught_errors();

}

void inputRZImpl::checkExecutionAbility(){

    if ( ((!is_pegsless && pegs_is_ok( PEGSdir + PEGSfileName ) && pegsErrors.isEmpty()) ||
          (is_pegsless && pegsless_is_ok() && pegsErrors.isEmpty())) &&
       ( openErrors.isEmpty() ) &&
       ( formErrors.isEmpty() )                         &&
         check_file( EGSdir + EGSfileName )             && // check input file or directory exists
         !EGSfileName.isEmpty()                         && // check non-blank input file name
         !getExecutable().isEmpty()                     ){
           ExecuteButton->setEnabled( true );
       }
     else {
         ExecuteButton->setEnabled( false );
    }

    caught_errors();
}

void inputRZImpl::checkPreviewRZ(){
 previewErrors = QString();
 previewRZ_exists = fileExists( ironIt(HEN_HOUSE + "/previewRZ/previewRZ.tcl") );
 if ( ! previewRZ_exists )
  previewErrors += tr("<br><b>previewRZ</b> utility not found in the $HEN_HOUSE : ") +
                     HEN_HOUSE +
                     tr("<br> You won't be able to preview RZ geometries!!!");

 previewRZ_exists = previewRZ_exists && isTclTkInstalled();

 if ( previewRZ_exists && geoErrors.isEmpty() &&
    check_file(EGSdir + EGSfileName) && !EGSfileName.isEmpty() ){
    PreviewRZButton->setEnabled( true );
 }
 else {
   PreviewRZButton->setEnabled( false );
 }
 caught_errors();
}

bool inputRZImpl::isTclTkInstalled(){
#ifdef WIN32
  QStringList wish = find_programs_in_system(QStringList("wish.exe"),*(";"));
  if ( wish.count() > 0 && wish.contains((QString)"wish.exe") > 0 )
#else
  QStringList wish = find_programs_in_system(QStringList("wish"),*(":"));
  if ( wish.count() > 0 && wish.contains((QString)"wish") > 0 )
#endif
     return true;
  else{
     previewErrors += tr("<br>Couldn't find <i>wish</i>. ") +
                      tr("Assuming <b>Tcl/Tk</b> is not installed.<br>")+
                      tr("You won't be able to use <b>previewRZ</b>!!!");
     return false;
  }
}


void inputRZImpl::checkPrintAbility(){
   if ( check_file(EGSdir + EGSfileName)   &&
        !EGSfileName.isEmpty() ) {
     PrintButton->setEnabled( true );
   }
   else {
     PrintButton->setEnabled( false );
     //confErrors += tr("<br>No input file for printing!");
   }
 caught_errors();
}



//! Sets input file name from argument passed at command line
/*!
When the input file name is passed trough the command line (with or without extension), this function
can be used to set the correct directory, file name and identify the user code to work with. It updates
the user code area, opens the input file and updates the input file name's combo box (both, Edit Line
and List Box). For the later I found it neccessary to temporarily disconnect the signal used in the
combo box's Edit Line to interactively detect input files while modifying the text. This is due to the
fact that updating the List Box contents in the Combo Box, also needs modifying the Edit Line.
*/
void inputRZImpl::SetInpfileName( QString inp_name )
{
    QString tmpDir = EGSdir;
    //qt3to4 -- BW
    //char s         = QDir::separator();
     char s = QDir::separator().toLatin1();
    if ( !inp_name.isEmpty() ) {
       EGSfileName = ironIt( inp_name ); // may be a full path name
       QFileInfo fi( EGSfileName );
       tmpDir = ironIt( fi.absolutePath() + s );

       //cout << tmpDir << " vs. " << QDir::currentDirPath()+s << endl;
       //if (fi.absolutePath() !=  QDir::currentDirPath() ||
         if ( EGSfileName.indexOf(s) >= 0 )
          EGSfileName.remove( 0, tmpDir.length() );

       if (EGSfileName.indexOf(".egsinp") < 0)
           EGSfileName = EGSfileName + ".egsinp";


       if( tmpDir != ironIt( EGS_HOME   + s + usercodename + s) &&
           tmpDir != ironIt( HEN_HOUSE + s + usercodename + s) )
          The_Other_Area = tmpDir;

       if ( tmpDir != EGSdir) {
       //cout << tmpDir << " vs. " << EGSdir << endl;
              EGSdir = tmpDir;
              usercodename = find_usercode_name( EGSdir );
   	      update_from_user_area();
              egs_dir_changed = true;
       }
       else egs_dir_changed = false;

       if ( !EGSfileName.isEmpty() ) {
           open();
           checkExecutionAbility();
           checkPrintAbility();
           checkPreviewRZ();
      }
    }


}

void inputRZImpl::OpenEGSInpFile()
{
    QString tmpDir = EGSdir;
    //qt3to4 -- BW
    //char s = QDir::separator();
    char s = QDir::separator().toLatin1();
    QDir d(tmpDir);
    if ( !d.exists() ) {
       tmpDir = GetCurrentDir( usercodename, EGS_HOME, HEN_HOUSE );
    }
    //qt3to4 -- BW
    //QString f = Q3FileDialog::getOpenFileName( tmpDir, "EGS input files (*.egsinp)", this );
    QString f = QFileDialog::getOpenFileName( this,"",tmpDir, "EGS input files (*.egsinp)");
    if ( !f.isEmpty() ) {
       EGSfileName = f;
       QFileInfo fi( EGSfileName );
       tmpDir = ironIt( fi.path() + s );
       EGSfileName.remove( 0, tmpDir.length() );


       if( tmpDir != ironIt( EGS_HOME   + s + usercodename + s) &&
           tmpDir != ironIt( HEN_HOUSE + s + usercodename + s) )
          The_Other_Area = tmpDir;

       if ( tmpDir != EGSdir) {
            EGSdir = tmpDir;
            usercodename = find_usercode_name( EGSdir );
   	    update_from_user_area();
            egs_dir_changed = true;
       }
       else egs_dir_changed = false;

       if ( !EGSfileName.isEmpty() ) {
           open();
           //checkErrors();
           checkExecutionAbility();
           checkPrintAbility();
           checkPreviewRZ();
      }
    }
}

//initiate comboboxes in column 0 of mediaTable
/*
//version below used QComboTableItems.  Use QComboBoxes instead because they offer more flexibility
void inputRZImpl::mediaTable_clicked( int row, int col) {
   if(col == 0) {//just use default line editor if not in column 0
       QString str = mediaTable->text(row,col);
       QStringList medlist=StrListToQStrList( listMedia ) ; //fill up a list of currently available media
       QComboTableItem *e = new QComboTableItem(mediaTable,medlist,false);
       mediaTable->setItem(row,col,e);
       if(!str.isNull()) e->setCurrentItem(str);
   }
}
*/

void inputRZImpl::mediaTable_clicked( int row, int col) {
   if(col == 0) {//just use default line editor if not in column 0
       //qt3to4 -- BW
       QString str;
       if(mediaTable->item(row,col)) str = mediaTable->item(row,col)->text();
       //QString str = mediaTable->text(row,col);
       QStringList medlist=StrListToQStrList( listMedia ) ; //fill up a list of currently available media
       QComboBox *e = new QComboBox;
       e->addItems(medlist);
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
       mediaTable->setCellWidget(row,col,e);
   }
}

//protects column 0 from being editable if not already a comboBox and single clicked
void inputRZImpl::mediaTable_singleclicked( int row, int col) {
   if(col == 0) {
     QWidget *editor = mediaTable->cellWidget( row, col );
     if(!editor) {//protect any text already there
       //qt3to4 -- BW
       //QString str = mediaTable->text(row,col);
       QTableWidgetItem* w =  mediaTable->item(row,col);
       if(w) {
          QString str = w->text();
          mediaTable->setItem(row,col,new QTableWidgetItem(str));
       }
     }
   }
//if we click away from a comboBox, freeze it and just display the text
   for(int irow =0; irow < mediaTable->rowCount(); irow++) {
     if(row != irow || col !=0) {
       QWidget *editor = mediaTable->cellWidget( irow, 0 );
       if(editor) {
        if(string(editor->metaObject()->className())=="QComboBox"){
          QComboBox* cb = (QComboBox*)editor;
          QString str = cb->currentText();
          mediaTable->removeCellWidget(irow,0);
          mediaTable->setItem(irow,0,new QTableWidgetItem(str));
        }
      }
     }
   }
}

//do the same for customFFTable
void inputRZImpl::customFFTable_clicked( int row, int col) {
   if(col == 0) {//just use default line editor if not in column 0
       //qt3to4 -- BW
       QString str;
       //QString str = customFFTable->text(row,col);
       if(customFFTable->item(row,col)) str = customFFTable->item(row,col)->text();
       QStringList medlist=StrListToQStrList( listMedia ) ; //fill up a list of currently available media
       QComboBox *e = new QComboBox;
       e->addItems(medlist);
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
       customFFTable->setCellWidget(row,col,e);
   }
}
void inputRZImpl::customFFTable_singleclicked( int row, int col) {
   if(col == 0) {
     QWidget *editor = customFFTable->cellWidget( row, col );
     if(!editor) {//protect any text already there
       QTableWidgetItem* w =  customFFTable->item(row,col);
       if(w) {
          QString str = w->text();
          customFFTable->setItem(row,col,new QTableWidgetItem(str));
       }
     }
   }

   //if we click away from a comboBox, freeze it and just display the text
   for(int irow =0; irow < customFFTable->rowCount(); irow++) {
     if(row != irow || col !=0) {
       QWidget *editor = customFFTable->cellWidget( irow, 0 );
       if(editor) {
        if(string(editor->metaObject()->className())=="QComboBox"){
          QComboBox* cb = (QComboBox*)editor;
          QString str = cb->currentText();
          customFFTable->removeCellWidget(irow,0);
          customFFTable->setItem(irow,0,new QTableWidgetItem(str));
        }
      }
     }
   }
}


//stuff for PEGSless implementation
void inputRZImpl::GetMDfile()
{
   QString tmpDir;
   char s = QDir::separator().toLatin1();
   //try EGS_HOME/pegs4/data, then HEN_HOUSE/pegs4/data then $HOME/pegs4/data
   tmpDir = GetCurrentDir( "pegs4/data", EGS_HOME, HEN_HOUSE );
   QDir d(tmpDir);
   QString f = QFileDialog::getOpenFileName( this,"",tmpDir, "material data file (*)" );
   if ( !f.isEmpty() ) {
      Ppgls->matdatafile = f;
      MDFEdit->setText(Ppgls->matdatafile);
      //update list of available media
      listMedia = getPEGSLESSMedia();
      updateMediaLists();
   }
}

void inputRZImpl::GetDFfile()
{
   QString start_dir;
   char s = QDir::separator().toLatin1();
   if(DFSearchComboBox->currentText() == "HEN_HOUSE")
     start_dir = ironIt(HEN_HOUSE + s + "pegs4" + s + "density_corrections" + s);
   else start_dir = ironIt(EGS_HOME + s + "pegs4" + s + "density_corrections" + s);
   if (!QDir(start_dir).exists()) start_dir.chop(20);
   QString f = QFileDialog::getOpenFileName(
               this, tr("Select a density correction file"),start_dir,
               tr("density correction file (*.density)"));

   //populate the media table
   if(!GetMedFromDCfile(f)){
      QString errStr = "Could not open density correction file "  + f;
               errStr += "\nPlease select another file and try again!";
        QMessageBox::information( this, " Warning",errStr, QMessageBox::Ok );
   }

   //set strings to output to .egsinp file
   int ind=inpmediumComboBox->currentIndex();
   Ppgls->inpmedind=ind;
   Ppgls->dffile[Ppgls->inpmedind] = f;
}

void inputRZImpl::set_table_header_noa()
{
   pz_or_rhozTable->setHorizontalHeaderItem(1,new QTableWidgetItem("no. of atoms"));
}

void inputRZImpl::set_table_header_pbw()
{
   pz_or_rhozTable->setHorizontalHeaderItem(1,new QTableWidgetItem("mass fractions"));
}

v_string inputRZImpl::getPEGSLESSMedia( )
{
    v_string lmed1,lmed2;
    lmed1.push_back( "NO MEDIA DEFINED" );

    QString fname = Ppgls->matdatafile;

    if(fname != "") {

       QFile f( fname );
       if ( f.open( QIODevice::ReadOnly ) ) {

          QTextStream ts( &f );
          QString     t;
          int         i;

          do {
            t = ts.readLine();
            i = t.indexOf( "MEDIUM=", 0, Qt::CaseInsensitive );
            if ( i > -1 ) { //found medium
               t.simplified();
               t.remove( 0, i + 7 );
               t.trimmed();
               lmed2.push_back( t.toStdString() );
            }
          } while ( !ts.atEnd() );

          f.close();
       }
    }

    //now add media to be specified in .egsinp file if there are no
    //duplicates with those in medium file

    int k;
    for (int i=0; i<Ppgls->ninpmedia; i++) {
       k=0;
       for (int j=0; j<lmed2.size(); j++) {
          if(Ppgls->inpmedium[i]==QString::fromStdString(lmed2[j])) break;
          k++;
       }
       if(k==lmed2.size()) lmed2.push_back(Ppgls->inpmedium[i].toStdString());
    }

    if ( !lmed2.empty() ) return lmed2;
    return lmed1;

}

bool inputRZImpl::pegsless_is_ok()
{

    pegsErrors = ""; v_string med;

    if (!cavityRadioButton->isChecked())
        med = getMediaFromTable();
    else {
        med.push_back(wallmaterialComboBox->currentText().toStdString());
       if (electradEdit->text().toFloat() > 0.0)
           med.push_back(electrmatComboBox->currentText().toStdString());
    }

    med = del_element( med, string("VACUUM"));

    bool PegslessOk = true;
    QString error = "Following media have not been defined:<br>";

    if(Ppgls->matdatafile!="") {

    QFile f1( Ppgls->matdatafile);
    if ( !f1.open( QIODevice::ReadOnly ) ) {
        pegsErrors += "<b>Could not open material data file:</b><br><i>" + Ppgls->matdatafile +
                      "</i><br>";
        return false;
    }
    QTextStream ts( &f1 );

    std::vector<string>::iterator iter(med.begin());
    QString t;
    while ( iter != med.end() ) {
       bool found = false;
       QString strmed = (*iter).c_str();
       QString strsought1 = "MEDIUM=";
       QString strsought2 = strmed;
       do {
            t = ts.readLine();
            int i = t.indexOf( strsought1, 0, Qt::CaseInsensitive );
            int j = t.indexOf(strsought2, 0 );
            if ( i > -1 && j > i) {
                   found = true;
                   break;
            }
       } while ( !ts.atEnd() );

       if (!found) {//check media defined in input file
           for (int i=0; i<Ppgls->ninpmedia; i++) {
              if(strmed==Ppgls->inpmedium[i]) {
                 found=true;
                 break;
              }
           }
       }
       error += (*iter).c_str() + QString("<br>");
       f1.close(); f1.open(QIODevice::ReadOnly);
       iter++;
       PegslessOk = PegslessOk && found;
    }

    f1.close();
    }  else {

    //look through media defined in .egsinp file

      std::vector<string>::iterator iter(med.begin());
      while ( iter != med.end() ) {
        bool found = false;
        QString strmed = (*iter).c_str();
        for (int i=0; i<Ppgls->ninpmedia; i++) {
           if(strmed==Ppgls->inpmedium[i]) {
             found=true;
             break;
           }
        }
        error += (*iter).c_str() + QString("<br>");
        iter++;
        PegslessOk=PegslessOk && found;
      }
    }

    error += "<i>Please, define media in .egsinp file or select the correct material data file <br>";
    error += "in order to be able to run the user code !!!</i><br>";

    if ( !PegslessOk ) pegsErrors += error ;

    return PegslessOk;
}


void inputRZImpl::updateMediaLists()
{
 //not really specific to PEGSLESS implementation
//but wanted to avoid typing this over and over
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

        checkExecutionAbility(); //put this in here for now
}

void inputRZImpl::enable_gaspEdit()
{
//enable gaspEdit edit box if isGasCheckBox is checked
//disable it if it is unchecked
    if (isGasCheckBox->isChecked() && !DCcheckBox->isChecked()){
        gaspLabel->setEnabled( true );
        gaspEdit->setEnabled( true );
        gaspUnits->setEnabled( true );
    }
    else{
        gaspLabel->setEnabled( false );
        gaspEdit->setEnabled( false );
        gaspUnits->setEnabled( false );
    }
}
//end stuff for PEGSless implementation

/*----------------------------------------------------------------------------
  Opens a file open dialog box on the location PEGSdir is pointing to.
  If this location does not exists, it looks for EGS_HOME/pegs4/data,
  HEN_HOUSE/pegs4/data and if these aren't found either, it starts on $HOME

  Once a PEGS4 file was selected, the location is stripped from the name and
  used to update PEGSdir if it changed.

 Right now, one only sees the file name without the path. Maybe I should
 change this.

 This is not totally satisfactory ...

------------------------------------------------------------------------------*/
void inputRZImpl::GetPEGSfile()
{
    QString tmpDir = PEGSdir;
    //qt3to4 -- BW
    //char s = QDir::separator();
    char s = QDir::separator().toLatin1();
    QDir d(tmpDir);
    if ( !d.exists() ) {
       tmpDir = GetCurrentDir( "pegs4/data", EGS_HOME, HEN_HOUSE );
    }
    //qt3to4 -- BW
    //QString f = Q3FileDialog::getOpenFileName( tmpDir, "PEGS data files (*.pegs4dat)", this );
    QString f = QFileDialog::getOpenFileName( this,"",tmpDir, "PEGS data files (*.pegs4dat)" );
    if ( !f.isEmpty() ) {
       PEGSfileName = f;
       QFileInfo fi( PEGSfileName );
       tmpDir = fi.path() + s;
       PEGSfileName.remove( 0, tmpDir.length() );

       if ( tmpDir != ironIt(HEN_HOUSE + s + (QString)"pegs4" + s + "data" + s)  &&
            tmpDir != ironIt(EGS_HOME    + s + (QString)"pegs4" + s + "data" + s )  )
          The_Other_PEGS = tmpDir;

       if ( tmpDir != PEGSdir) {
	   PEGSdir = tmpDir;
	   pegs_dir_changed = true;
       }
       else  pegs_dir_changed = false;

       if ( !PEGSfileName.isNull() ) {

          //checkErrors();
          checkExecutionAbility();

         pegs4ComboBox->setEditText ( PEGSfileName );
         Add_New_Item( PEGSfileName.toLatin1().data(), pegs4ComboBox );

          listMedia	 = getPEGSMedia( PEGSdir + PEGSfileName );

          update_from_data_area();

       }
    }

}

void inputRZImpl::getCONFFile()
{
    QString d = ironIt(HEN_HOUSE        + QDir::separator() +
                       QString("specs") + QDir::separator() );
    //qt3to4 -- BW
    QString f = ironIt(
                //Q3FileDialog::getOpenFileName( d,
                QFileDialog::getOpenFileName( this,"",d,
                             "configuration files (*.conf)")
                );
    if (!f.isEmpty()){
      f.remove(0, 1+ ironIt(f).lastIndexOf( QDir::separator()) );
      CONFcomboBox->setEditText(f);
    }
}


void inputRZImpl::GetSPECfile()
{
    //qt3to4 -- BW
    //Q3FileDialog* fd = new Q3FileDialog(SPECdir, QString::null, this);
    QFileDialog* fd = new QFileDialog(this,"",SPECdir, QString());
    QStringList filters;
    filters << "*.spectrum" << "*.ensrc";
    fd->setNameFilters( filters );
    QString f;
    QStringList flst;
    if ( fd->exec() == QDialog::Accepted )
        flst = fd->selectedFiles();
        if(!flst.isEmpty())  f=flst[0];


    if ( !f.isEmpty() ) {
       QString spe_file = f;
       QFileInfo fi( spe_file );
       SPECdir = ironIt(fi.path() + "/");
       spe_file.remove( 0, SPECdir.length() );

       if ( !spe_file.isNull() ) {
          specfnameComboBox->setEditText ( spe_file );
          Add_New_Item( spe_file.toLatin1().data(), specfnameComboBox );
       }
    }

    update_files( SPECdir, specfnameComboBox, "*.spectrum *.ensrc" );

}

void inputRZImpl::GetRDISTfile()
{
    RDISTdir = GetCurrentDir( usercodename, EGS_HOME, HEN_HOUSE );
    QString f = QFileDialog::getOpenFileName( this,"",RDISTdir, QString());
    if ( !f.isEmpty() ) {
       QString dist_file = f;
       QFileInfo fi( dist_file );
       RDISTdir = fi.path() + "/";
       dist_file.remove( 0, RDISTdir.length() );

       if ( !dist_file.isNull() ) {
           raddistfnameComboBox->setEditText ( dist_file );
           Add_New_Item( dist_file.toLatin1().data(), raddistfnameComboBox );
       }
    }

    update_files( RDISTdir, raddistfnameComboBox, "*.*" );

}

void inputRZImpl::GetPHSPfile()
{
    PHSPdir  = PHSPdir.isEmpty()? GetCurrentDir( "", EGS_HOME, HEN_HOUSE ):PHSPdir;
    QString f = QFileDialog::getOpenFileName( this,"",PHSPdir,
                             "Phase-space files (*.egsphsp1 *.IAEAphsp);;Any files (*)");
    //QString f = QFileDialog::getOpenFileName( PHSPdir, QString::null, this );
    if ( !f.isEmpty() ) {
       QString phsp_file = f;
       QFileInfo fi( phsp_file );
       PHSPdir = fi.path() + "/";
       phsp_file.remove( 0, PHSPdir.length() );

       if ( !phsp_file.isNull() ) {
          phasespaceComboBox->setEditText ( phsp_file );
          Add_New_Item( phsp_file.toLatin1().data(), phasespaceComboBox );
       }
    }
    update_files( PHSPdir, phasespaceComboBox, "*.egsphsp1 *.IAEAphsp" );
}

void inputRZImpl::open()
{// input values from a file. If a value is not found,
 // previous (default) defined value is used instead.

	std::ifstream inp;
	inp.open( (EGSdir + EGSfileName).toLatin1().data() );
	if (false == inp.is_open()){
	    QString error = "<b>Input file </b><i>" + EGSdir +
                            EGSfileName  + "</i> not found.<br>";
                  openErrors = error;
	    QMessageBox::warning ( this, "Beware", error, 1, 0, 0 );
	    return;
	}

 	openErrors = "";
 	geoErrors  = "";

	usercode = GetUserCode();

	MInputRZ* Input =  new MInputRZ;
	Input->SetUserCode( usercode );
	inp >> Input;

	//if ( !Input->errors.isEmpty() )
	if ( Input->gotErrors() ) {
         openErrors = QString(WARNING_DEFAULTS) + Input->getErrors() +
                      openErrors;
	}
        geoErrors  = Input->getGEOErrors();

        UpDateInputRZForm( Input );
        inp.close();

        //delete Input;
        zap(Input);

}

//!  Check that the pegs4 data file \em fname exists and that
//  there is information about the media defined in the GUI.
/*!
This function verifies that the pegs4 data file \em fname exists.
If it does not exist,it returns a \em false value and
if it does exist, it returns a \em true value and the file is searched
for the media defined in the media table of the
geometry information tab.
*/
bool inputRZImpl::pegs_is_ok( QString fname )
{

    pegsErrors = "";

    v_string med;
    if (!cavityRadioButton->isChecked())
        med = getMediaFromTable();
    else {
        //MCAVInputs* EGScav = GetCAV();
        med.push_back(wallmaterialComboBox->currentText().toStdString());
       //qt3to4 -- BW
       //if (EGScav->electr_rad > 0.0)
       if (electradEdit->text().toFloat() > 0.0)
           med.push_back(electrmatComboBox->currentText().toStdString());
    }

    med = del_element( med, string("VACUUM"));

    bool PegsOk = true;
    QString error =  "<b>Selected pegs4 data file:</b><br><i>" + fname +
                     "</i><br>";
            error += "Following media were not found in pegs4 data file:<br>";

    QFile f1( fname );
    if ( !f1.open( QIODevice::ReadOnly ) ) {
        pegsErrors += "<b>Could not open pegs4 data file:</b><br><i>" + fname +
                      "</i><br>";
        return false;
    }
    //qt3to4 -- BW
    //Q3TextStream ts( &f1 );
    QTextStream ts( &f1 );

    int step_number = med.size();
    //qt3to4 -- BW
    //Q3ProgressDialog* dialog = new Q3ProgressDialog("Checking existing media...",
    //                                              "Cancel", step_number,
    //                                              this, "progress", true);
    QProgressDialog* dialog = new QProgressDialog("Checking existing media...","Cancel",0,step_number);
    dialog->setValue(0);
    dialog->setMinimumDuration( 0 );

    std::vector<string>::iterator iter(med.begin());
    int progress_count = 0;
    QString t;
    while ( iter != med.end() ) {
       progress_count++;
       qApp->processEvents();
       if ( dialog->wasCanceled() )
          return false;
       bool found = false;
       QString strmed = (*iter).c_str();
       QString strsought = "MEDIUM=" + strmed + " ";
       do {
	    t = ts.readLine();
	    int i = t.indexOf( strsought, 0, Qt::CaseSensitive );
    	    if ( i > -1 ) {
		   found = true;
		   break;
	    }
       //qt3to4 -- BW
       //} while ( !ts.eof() );
       } while ( !ts.atEnd() );

       if (!found) error += (*iter).c_str() + QString("<br>");
       f1.close(); f1.open(QIODevice::ReadOnly);
       PegsOk = PegsOk && found;
       iter++;
       dialog->setValue( progress_count );
    }

    f1.close();
    zap(dialog);

    error += "<i>Please, correct media names or get the right pegs4 file <br>";
    error += "in order to be able to run the user code !!!</i><br>";

    if ( !PegsOk ) pegsErrors += error ;

    return PegsOk;
}

UserCodeType inputRZImpl::GetUserCode()
{

    QString t;
    int i = -1;
    //UserCodeType usr = cavrznrc;
    //UserCodeType usr = dosrznrc;
    UserCodeType usr = usercode;

    QFile f1( EGSdir + EGSfileName );
    if ( !f1.open( QIODevice::ReadOnly ) ) {
     openErrors += "<b>Error reading file</b><i>" +
                   EGSdir + EGSfileName +
                   "</i> doesn't exist !!!<br>";
        return usr;
    }
    //qt3to4 -- BW
    //Q3TextStream ts( &f1 );
    QTextStream ts( &f1 );

    do { // while blank or comment line
     t = ts.readLine();
     i = t.indexOf( "cavity",0,Qt::CaseInsensitive );
     if ( i > -1 ) {usr = cavrznrc;break;}
     i = t.indexOf( "spr output",0,Qt::CaseInsensitive  );
     if ( i > -1 ) {usr = sprrznrc;break;}
     i = t.indexOf( "dose zbound",0,Qt::CaseInsensitive );
     if ( i > -1 ) {usr = dosrznrc;break;}
     i = t.indexOf( "print fluence spectra",0,Qt::CaseInsensitive );
     if ( i > -1 ) {usr = flurznrc;break;}
    //qt3to4 -- BW
    //}while ( i == -1 && !ts.eof() );
    }while ( i == -1 && !ts.atEnd() );

    if ( i == -1 ) {
	    //QString error = WARNING_USER;
	    //QMessageBox::warning ( this, "Attention!", error, 1, 0, 0 );
      openErrors += "<b>Couldn't guess user code from </b><i>" +
                    EGSdir + EGSfileName + "<br>";
    //openErrors += "</i><br><b>using CAVRZNRC as default !!!</b><br>";
    //openErrors += "</i><br><b>using DOSRZNRC as default !!!</b><br>";
    }
    f1.close();
    return usr;
}

v_string inputRZImpl::getPEGSMedia( const QString& fname )
{
    v_string lmed1;
    lmed1.push_back( "NO MEDIUM" );

    QFile f( fname );
    if ( !f.open( QIODevice::ReadOnly ) ) {
        return lmed1;
    }

    //qt3to4 -- BW
    //Q3TextStream ts( &f );
    QTextStream ts( &f );
    QString     t;
    v_string    lmed2;
    int         i;

    do {
       t = ts.readLine();
       i = t.indexOf( "MEDIUM=", 0 );
  	   if ( i > -1 ) { //found medium
          //cout << t.toStdString() << endl;
          //cout << "MEDIUM starts at " << i << endl;
          t.simplified();
          t.remove( 0, i + 7 );
          //cout << t << endl;

          i = t.indexOf( " ", 0 );
  	      if ( i > -1 ) { //found medium
             //cout << "STERNCID starts at " << i << endl;
             t.truncate( i );
             t.trimmed();
             //cout << t << endl;
             lmed2.push_back( t.toStdString() );
          }
       }
    //qt3to4 -- BW
    //} while ( !ts.eof() );
    } while ( !ts.atEnd() );

    f.close();

    if ( !lmed2.empty() ) return lmed2;
    return lmed1;

}
