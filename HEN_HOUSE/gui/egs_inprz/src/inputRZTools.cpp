/*
###############################################################################
#
#  EGSnrc egs_inprz tools
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
#include <cstdlib>
#include <qlineedit.h>
#include <qapplication.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <iterator>
#include <qpushbutton.h>
#include <qdir.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qvalidator.h>
#include <typeinfo>
#include <egs_config_reader.h>

//**********************************************
// *********      GENERAL STUFF      ***********
//**********************************************

void inputRZImpl::validate_combo(const char* entry, QString error, QComboBox* cb)
{
	int iw = Get_Item_Index( entry, cb );
	if ( iw < 0 ) {
	    openErrors += error;
	    iw = 0; // assuming first item is default
	}
	cb->setCurrentIndex ( iw );
}

void inputRZImpl::validate_radio(const char* entry, QString error, int count, QRadioButton **r)
{
 	bool isValid = false;
  QString sEntry = entry;

 	for (int i = 0; i < count; i++)
	{
		if (sEntry.toUpper() == (r[i]->text()).toUpper()){
    		r[i]->setChecked( true );
    		isValid = true;
    		break;
   	}
	}

	if (!isValid){
	   openErrors += error;
	}
}

QString inputRZImpl::TextRadioBChecked(int count, QRadioButton **r)
{
 	for (int i = 0; i < count; i++)
	{
	    if ( r[i]->isChecked()){
	    	return r[i]->text();
	    }
	}
	return r[0]->text();

}


int inputRZImpl::Add_New_Item(const char* ItemName, QComboBox* cb)
{
	int index = 0;
              QString sItemName = ItemName;
 	bool ItemExist = false;
 	for( int i = 0 ; i < cb->count() ; i++ ) {
    	if ( sItemName.toUpper() == (cb->itemText(i)).toUpper() ) {
	      index = i;
	      ItemExist = true;
	      break;
	    }
	}

 	if ( !ItemExist ) {
	    cb->addItem( tr( ItemName ) );
	}
	return index;
}

int  inputRZImpl::Get_Item_Index(const char* ItemName, QComboBox* cb)
{
   QString sItemName = ItemName;
   for( int i = 0 ; i < cb->count() ; i++ ) {
       if ( sItemName.toUpper() == (cb->itemText(i)).toUpper() ) {
		    	return i;
       }
   }
   return -1;
}

//  gives you first user code directory in the user's area if it exists
// if it doesn't exist, sets current user code directory to HEN_HOUSE/usercode/
// and if it neither exists, it sets the current directory to rHome
QString inputRZImpl::GetCurrentDir( const QString& rCodeName, const QString& rHome, const QString& rHenHouse )
{
    QString current_dir  = rHome + QDir::separator()+ rCodeName + QDir::separator();
    QDir d(current_dir);
    if ( !d.exists() ) {
	     current_dir  = rHenHouse + QDir::separator() + rCodeName + QDir::separator();
	     if ( !d.cd(current_dir) ) {
	        current_dir = rHome;
	     }
    }

    return current_dir;
}

QString inputRZImpl::GetPEGSDir( const QString& rCodeName, const QString& rHome, const QString& rHenHouse )
{
    QFile f1( rHome + rCodeName );
    QFile f2( rHenHouse + rCodeName );

    if ( f1.open( QIODevice::ReadOnly ) ) {
        f1.close();
        return rHome;
    }
    else if ( f2.open( QIODevice::ReadOnly ) ) {
        f2.close();
        return rHenHouse;
    }
    else return QString();

}

QString inputRZImpl::find_usercode_name( const QString& dir )
{
//    QString name = "cavrznrc";
    QString name = "dosrznrc";
    if ( dir.indexOf("cavrznrc",0,Qt::CaseInsensitive) >= 0 ) {
	name = "cavrznrc";
    }
    else if ( dir.indexOf("dosrznrc",0,Qt::CaseInsensitive)  >= 0 ) {
	name = "dosrznrc";
    }
    else if ( dir.indexOf("sprrznrc",0,Qt::CaseInsensitive) >= 0 ) {
	name = "sprrznrc";
    }
    else if ( dir.indexOf("flurznrc",0,Qt::CaseInsensitive)  >= 0 ) {
	name = "flurznrc";
    }
    else{
	//name = "cavrznrc";
	name = "dosrznrc";
    }
    return name;
}

//!  Detects whether the current directory is one of the possible locations
//   for the user codes and which user code should be used.
/*!
  This function uses \em getenv() to obtain the environment variable \em $PWD.
  It then checks whether the current directory is one of
  the possible user code locations. If the current directory is a user code area,
  it updates the user code name (by default cavrznrc) and ::update_files returns
  the directory name. If it is not a user code directory it returns an empty string.
*/
QString inputRZImpl::get_initial_usercode_area( QString* name )
{
    QString pwd = QDir::currentPath ();
    *name = find_usercode_name( pwd );
    QString tmpEGSdir = QString::null;
    if ( ( pwd.contains( EGS_HOME ) > 0 && pwd.contains( *name ) > 0) ||
        ( pwd.contains( HEN_HOUSE ) > 0 && pwd.contains( *name ) > 0) ){
	tmpEGSdir = pwd + QDir::separator();
     }
    return tmpEGSdir;
}

void inputRZImpl::updateConfiguration( const QString & conf ){
 QString confi = conf;
 // Get current config file directory
 if (!HEN_HOUSE.isEmpty()){
    CONFdir = ironIt( HEN_HOUSE     + QDir::separator() +
                   (QString)"specs" + QDir::separator());
    // Here we remove any path to the config file.
    // It is mandatory to have it in $HEN_HOUSE/specs !!!!
    confi.remove(0, 1+ ironIt(confi).lastIndexOf( QDir::separator()) );
 }

 CONFcomboBox->setEditText( confi );
 Add_New_Item( confi.toLatin1().data(), CONFcomboBox );

 //QString f = conf;
 QString f=(conf.lastIndexOf(QDir::separator())<0)?ironIt(CONFdir+conf):conf;

 HEN_HOUSE  = readVarFromConf( "HEN_HOUSE" );
 EGS_HOME   = readVarFromConf( "EGS_HOME" );
 if (!HEN_HOUSE.endsWith(QDir::separator())) HEN_HOUSE.append(QDir::separator());
 if (!EGS_HOME.endsWith(QDir::separator()))  EGS_HOME.append(QDir::separator());
 EGS_CONFIG = f;

#ifdef WIN32
 if (!EGS_HOME.isEmpty())
      EGS_HOME.replace( 0, 1,
      QString(EGS_HOME[0]).toUpper());
 if (!EGS_CONFIG.isEmpty())         // make drive letter upper case
      EGS_CONFIG.replace( 0, 1,     // to be consistent with Qt Widgets
      QString(EGS_CONFIG[0]).toUpper());
#endif


 if ( HEN_HOUSE.isEmpty()){
      confErrors +=
      (QString)"<br>Variable HEN_HOUSE not found in configuration file"
      + EGS_CONFIG + (QString)"<br>";
 }
 if ( EGS_HOME.isEmpty()){
      confErrors +=
      (QString)"<br>Variable EGS_HOME not found in configuration file"
      + EGS_CONFIG + (QString)"<br>";
 }

 // Update config file directory
 CONFdir = ironIt( HEN_HOUSE        + QDir::separator() +
                   QString("specs") + QDir::separator());

//  //Update config file combo box if needed
//  confi.remove(0, 1+ ironIt(confi).lastIndexOf( QDir::separator()) );
//  CONFcomboBox->setEditText( confi );
//  Add_New_Item( confi.toLatin1().data(), CONFcomboBox );

}

//!  Sets initial environment variables like $HOME and $HEN_HOUSE,
//   and other useful location varibales
/*!
  This function uses \em getenv() to obtain the environment variables
  \em $HOME and \em $HEN_HOUSE. It also finds the GUI's location (needed
  only to find the html help files). It then checks whether the current
  directory is one of the possible user code locations, using
  \em get_initial_usercode_area(). If the current directory is a user code area,
  it updates the user code name (by default cavrznrc) and the user code area.
  If it is not a user code directory, then it sets the user code area to either
  \em $HOME/egsnrc/usercodename or \em $HEN_HOUSE/usercodename. Once the user
  code area is defined, the directories for the spectra, phase space and radial
  distribution files are set. It also updates the contents of the input file
  name combo box.

  Using class EGS_ConfigReader to get environment variables. It first check
  the EGS_CONFIG file and files included therein. If the value of a given key
  is still not found, then it tries to get it from the environment.

*/
void inputRZImpl::SetInitialDir()
{
 //qt3to4 -- BW
 //char SEP   = QDir::separator();
 QChar SEP   = QDir::separator();

 EGS_CONFIG = ironIt( getenv( "EGS_CONFIG" ) );
 QString HHini  = ironIt( getenv( "HEN_HOUSE" ) );// get HEN_HOUSE from environment
 if (!HHini.isEmpty()) HEN_HOUSE = HHini; // If available from environment, initialize it.
#ifdef WIN32
 if (!EGS_CONFIG.isEmpty())         // make drive letter upper case
      EGS_CONFIG.replace( 0, 1,     // to be consistent with Qt Widgets
      QString(EGS_CONFIG[0]).toUpper());
#endif

 updateConfiguration(EGS_CONFIG);// gets HEN_HOUSE from config file

 GUI_HOME = ironIt( HEN_HOUSE + SEP + (QString)"doc" + SEP +
                    (QString)"pirs801" + SEP );

 EGSdir = ironIt(  get_initial_usercode_area( &usercodename ) );
 if ( usercodename == "cavrznrc" )
      usercode     = cavrznrc;
 else if ( usercodename == "dosrznrc" )
      usercode     = dosrznrc;
 else if ( usercodename == "sprrznrc" )
      usercode     = sprrznrc;
 else if ( usercodename == "flurznrc" )
      usercode     = flurznrc;

 if ( EGSdir.isEmpty() )
      EGSdir   = ironIt( GetCurrentDir( usercodename, EGS_HOME, HEN_HOUSE ) );

 SPECdir = ironIt( HEN_HOUSE +  SEP + "spectra");
 RDISTdir= ironIt( GetCurrentDir( usercodename   , EGS_HOME, HEN_HOUSE ) );
 PHSPdir = ironIt( GetCurrentDir( ""             , EGS_HOME, HEN_HOUSE ) );
 CONFdir = ironIt( HEN_HOUSE + SEP + "specs" +SEP );

 PEGSdir = GetPEGSDir( PEGSfileName,
           ironIt( HEN_HOUSE + SEP + "pegs4" + SEP + "data" + SEP ),
           ironIt( EGS_HOME  + SEP + "pegs4" + SEP + "data" + SEP  ) );
 if ( PEGSdir.isEmpty() ){
      PEGSdir  = ironIt( GetCurrentDir(
                         (QString)"pegs4" + SEP + QString("data") + SEP,
                         HEN_HOUSE, EGS_HOME ) ) ;
 }

 The_Other_PEGS = EGS_HOME;
 The_Other_Area = EGS_HOME;

 egs_dir_changed = true;
 pegs_dir_changed = true;

 is_pegsless = false;

 update_from_user_area();
 update_from_data_area();

 disconnect( InputFileComboBox, SIGNAL( editTextChanged(const QString&) ),
             this, SLOT( EGSFileNameChanged(const QString&) ) );

 update_files( EGSdir, InputFileComboBox, "*.egsinp" );

 connect( InputFileComboBox, SIGNAL( editTextChanged(const QString&) ),
          this, SLOT( EGSFileNameChanged(const QString&) ) );

 //this was not done before -- BW
 update_files( PEGSdir, pegs4ComboBox, "*.pegs4dat" );
 connect( pegs4ComboBox, SIGNAL( editTextChanged(const QString&) ),
          this, SLOT( PEGSFileNameChanged(const QString&) ) );

 if (!HEN_HOUSE.endsWith(QDir::separator())) HEN_HOUSE += QDir::separator();
 if (!HHini.endsWith(QDir::separator()))     HHini     += QDir::separator();
 if (HEN_HOUSE.isEmpty() && !HHini.isEmpty()){
   HEN_HOUSE = HHini;
   confErrors +=
      (QString)"<br>Using variable HEN_HOUSE set in your environment to "
      + HEN_HOUSE + (QString)"<br>";
 }
 else if (HEN_HOUSE.isEmpty() && HHini.isEmpty()){
   confErrors +=
      (QString)"<br>Unknown HEN_HOUSE environment variable. Have you configured EGSnrc?"
      + (QString)"<br>";
 }
 else if (HEN_HOUSE != HHini){
   confErrors +=
      tr("<br>HEN_HOUSE environment variable (") + HHini +
      tr(") differs from the one in your configuration (") + HEN_HOUSE +
      tr("). Using the latter! <br>");
 }

}

// If exists, returns EGSnrc user code path in user area
// If it doesn't, it creates it automatically warning the user
QString inputRZImpl::GetUserCodeDir( const QString& rCodeName)
{
    QString current_dir  = ironIt( EGS_HOME + QDir::separator() + rCodeName + QDir::separator() );
    QDir d(current_dir);
    if ( !d.exists() ) {
      QString info = current_dir;
                   info += " hasn't been created yet ! ";
                   info += "\nCreated automatically." ;
       current_dir  = EGS_HOME;
       if ( !d.cd(current_dir) ) {
            if (!d.mkdir(current_dir)){
	       info = "Could not create " +  current_dir + " !!!";
	       info += "\n home area used instead";
               QMessageBox::warning ( this, "Attention", info, 1, 0, 0 );
               return  QDir::homePath();
              }
       }
       current_dir  += rCodeName + QDir::separator();
       if (!d.mkdir(current_dir)){
	    info = "Could not create " +  current_dir + " !!!";
            current_dir  = EGS_HOME;
	    info += "\n" + current_dir + " used instead";
            QMessageBox::warning ( this, "Attention", info, 1, 0, 0 );
            return  current_dir;
       }
       QMessageBox::warning ( this, "Attention", info, 1, 0, 0 );
    }
    return ironIt( current_dir );
}

QString inputRZImpl::FToQStr( float Item )
{
 QString str;
 //str = str.setNum( Item, 'f', 3 );
 str = str.setNum( Item, 'g', 5 );
 return str;
}

QString inputRZImpl::IntToQStr( int Item )
{
 QString str;
 str = str.setNum( Item, 10 );
 return str;
}

QStringList inputRZImpl::StrListToQStrList( v_string Item )
{
 QStringList str;
  std::vector<string>::iterator iter( Item.begin() );
  while ( iter != Item.end()) {
      str << (*iter).c_str();
      iter++;
  }
 return str;
}

v_int inputRZImpl::assign_medium_number(v_string med_list, v_string med_entry)
{
    v_int num;
    std::vector<string>::iterator iter1( med_entry.begin() );
    while ( iter1 != med_entry.end() ) {       // loops trough medium entries
        std::vector<string>::iterator iter2( med_list.begin() );
        int i = 0;
        int index = 0;
        while ( iter2 != med_list.end()  ) { // loops through list
          if ( *iter1 == *iter2 ) {      // match found
             index = i;// C-indexing since 0th element set to vacuum
             //index = i+1;// Fortran indexing starts at 1!
             break;
          }
        i++; iter2++;
        }
        num.push_back( index );
        iter1++;
    }

    return num;

}
//**********************************************
// *********        TABLE STUFF      ***********
//**********************************************

//! Clears the contents of table \em t
/*!
This table utility clears each tables's cell by calling clearCell()
and removes any existing cell widget through a call to clearCellWidget( )
*/
//qt3to4 -- BW
//void inputRZImpl::clear_table( Q3Table* t )
void inputRZImpl::clear_table( QTableWidget* t )
{
    //qt3to4 -- BW
    //for ( int i = 0; i < t->numRows(); i++){
        //for ( int j = 0; j < t->numCols(); j++ ){
               //t->clearCell( i, j );
               //if ( t->cellWidget( i, j ) )
               //     t->clearCellWidget( i, j );
        //}
    //}
    t->clearContents();
}

//! Clears the contents of table's column \em col
/*!
This table utility clears each column's cell by using setItem to set
the cell's item to null
*/
//qt3to4 -- BW
//void inputRZImpl::clear_col( Q3Table* t, int col)
void inputRZImpl::clear_col( QTableWidget* t, int col)
{
    //qt3to4 -- BW
    //for ( int row = 0; row < t->numRows(); row++)  {
    for ( int row = 0; row < t->rowCount(); row++)  {
         //t->clearCellWidget( row, col);
         //t->setText( row, col,"");
        //qt3to4 -- BW
        //t->clearCell( row, col );
        //if ( t->cellWidget( row, col ) )
             //qt3to4 -- BW
             //t->clearCellWidget( row, col );
             //t->removeCellWidget(row,col);
        t->setItem(row,col,0);
    }
}

void inputRZImpl::update_SprOutTableHeaders()
{

	//sproutHeader->setUpdatesEnabled( false );
	//sproutTable->horizontalHeader()->setUpdatesEnabled( false );

	if ( (sproutComboBox->currentText()).toLower() == "regions"){
           //qt3to4 -- BW
	   //sproutTable->horizontalHeader()->setLabel(0,"start region");
	   //sproutTable->horizontalHeader()->setLabel(1,"stop region");
           sproutTable->setHorizontalHeaderItem(0,new QTableWidgetItem("start region"));
           sproutTable->setHorizontalHeaderItem(1,new QTableWidgetItem("stop region"));
	}
	else {
           //qt3to4 -- BW
	   //sproutTable->horizontalHeader()->setLabel(0,"cylinders");
	   //sproutTable->horizontalHeader()->setLabel(1,"slabs");
           sproutTable->setHorizontalHeaderItem(0,new QTableWidgetItem("cylinders"));
           sproutTable->setHorizontalHeaderItem(1,new QTableWidgetItem("slabs"));

	}

	//sproutTable->horizontalHeader()->setUpdatesEnabled( true );

        //qt3to4 -- BW
	//sproutTable->showColumn(0);
	//sproutTable->showColumn(1);
        sproutTable->setColumnHidden(0,false);
        sproutTable->setColumnHidden(1,false);;

}
