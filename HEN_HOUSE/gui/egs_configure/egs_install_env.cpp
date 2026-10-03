/*
###############################################################################
#
#  EGSnrc configuration GUI environment setup
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
#  Author:          Ernesto Mainegra-Hing, 2015
#
#  Contributors:    Reid Townson
#
###############################################################################
*/


#include "egs_install.h"

#include <QFileInfo>

#define EGS_VIEW_DSO "linux-static"

void QInstallPage::environmentSetUp()
{

   resetProgressBar(100);
   setSubTitle("Setting up GUIs ...");
   updateProgress();
   qApp->processEvents();

   /* Extract GUI binaries */
   if (itsAZip()) extract( QCoreApplication::applicationFilePath().toLatin1().data(), henHouse().toLatin1() );
   qApp->processEvents();

#if defined(Q_OS_WIN32) || defined(WIN32)
   QChar s = QDir::separator();
   QString piecesWindows = henHouse() + "pieces" + s + "windows" + s;
   printProgress("\n ===> Copying GUI executables  ...    ");
   qApp->processEvents();
   if ( copy_files( piecesWindows, egsBinDir, "*.exe" ) )
     printProgress("OK \n");
   else
    printProgress("failed \n");
/*   printProgress((QString)"\n => Copying egs_view and egs++ library to dso area .... \n\n");
   QString dsoDir = henHouse() + "egs++" + s + "dso";
   if ( ! copyRecursively( piecesWindows + s + "win2k-cl", dsoDir) )
        printProgress( QString("\nError copying files!"));*/
#elif defined(Q_OS_LINUX)
    /* make symbolic link to statically built GUIs  */
    setup_static_guis();

    //**************************************************
    // NOT NEEDED since egs_view will pick the egspp
    // library from LD_LIBRARY_PATH
    //**************************************************
    /* make symbolic link linux32[64] to actual dso dir */
    //set_guis_dso();
#endif
    if (envCheckBox->isChecked())
        SaveAppSetting();//updates path and puts gui icons&folders if requested
    if (shortcutCheckBox->isChecked()){
        createEGSFolders();
        createBeamFolders();
    }
    cleanUp();
    emit environmentSet();
}

void QInstallPage::SaveAppSetting()
{

  setSubTitle("Configuring EGSnrc environment");

#ifdef WIN32

    QString smsg;

    resetProgressBar(7);

    /* Updating EGS_HOME related environment */

    printProgress("\nUpdating environment variable EGS_HOME to " + egsHome() );
    replaceUserEnvironmentVariable( "EGS_HOME", egsHome(), &smsg);

    // Update the user path variable with the EGS_HOME bin directory
    printProgress("\nSetting EGS_HOME bin directory in your path ....");
    update_path( homeBinDir );
    updateProgress();

    /* Updating HEN_HOUSE related environment */

    printProgress("\nUpdating environment variable HEN_HOUSE to " + henHouse() );
    replaceUserEnvironmentVariable( "HEN_HOUSE", henHouse(), &smsg);

    bool path_cleaned = cleanPath();// cleans PATH from previous EGSnrcMP
                                    // HEN_HOUSE entries

    // Update the user path variable with the EGS bin directory
    printProgress("\nSetting EGSnrc bin directory in your path ....");
    update_path( egsBinDir );
    updateProgress();

    // Update the user path variable with the previewRZ directory
    printProgress("\nSetting previewRZ directory in your path ....");
    update_path( henHouse() + QString("previewRZ") );
    updateProgress();

    // Update the user path variable with the dso/win2k-cl directory
    printProgress("\nSetting win-static DSO directory in your path ....");
    update_path( henHouse() + QDir::separator() + "egs++" +
                              QDir::separator() + "dso"   +
                              QDir::separator() + "win-static");
    updateProgress();

    // Update the user path variable with the dso/$my_machine directory
    printProgress("\nSetting DSO directory in your path ....");
    update_path( dsoDir );
    updateProgress();

    // Update the user path variable with the Fortran compiler directory
    // assuming the C and C++ copilers are in the same location.
    printProgress("\nSetting compiler directory in your path ....");
    update_path( fc->path() );
    updateProgress();

    /* Updating EGS_CONFIG related environment */
     printProgress("\nUpdating environment variable EGS_CONFIG to " + specFile);
     replaceUserEnvironmentVariable( "EGS_CONFIG", specFile, &smsg);
     updateProgress();

    /* Updating BEAMnrc related environment */
     QString omegahome = henHouse() + QString("omega") + QDir::separator();
     printProgress("\nUpdating environment variable OMEGA_HOME to " + omegahome + "\n");
     replaceUserEnvironmentVariable( "OMEGA_HOME", omegahome, &smsg);
     updateProgress();

     printProgress("\nWriting EGSnrc profile to " + egsnrcConfigHome() + " ...");
     if ( ! writeXdgProfile() )
         printProgress("\n Warning: could not write EGSnrc profile files.\n");

#else
  /* Setup EGSnrc environment in shell resource file */
     update_unix_env();

#endif

}

/*---------------------------------------------------------------
 *
 * PREPENDS the_dir to the PATH environment variable. If a location
 * is already set, then it is not duplicated.
 *
 *---------------------------------------------------------------*/
void QInstallPage::update_path( const QString& the_dir ){
  QString path = the_dir + ";"; QString smsg;

  if ( ! prepend2UserEnvironmentVariable( "PATH", path, &smsg) ){
/*    config_log << "\n Directory " << the_dir
      << " was already set in the user's PATH. \n"
      << "===================================\n" << smsg << "\n\n";*/
    printProgress("\n Directory " + the_dir + " already set in the user's PATH. \n"
                 + "===================================\n"
                 + smsg + "\n\n"
    );
  }
  else{
/*    config_log << "\n Directory " << the_dir
      << " successfully set in the user's PATH. \n";*/
    printProgress( "\n Directory " + the_dir + " successfully set in the user's PATH. \n");
  }
}

/*---------------------------------------------------------------
 *
 * Checks for entries related to the previous HEN_HOUSE or EGS_HOME
 * and removes them. Prevents the PATH variable from cluttering with
 * tons of entries when testing/experimenting with EGSnrcMP on Windows
 * For a regular user, installing EGSnrcMP on one location, this
 * is not necessary, but useful for the developer testing different
 * installations options.
 *
 *---------------------------------------------------------------*/
bool QInstallPage::cleanPath(){
#ifdef WIN32
  QString user_msg;
  QString path = getUserEnvironmentVariable( "PATH", &user_msg);
  //config_log << "Cleaning USER PATH = " << path << endl;
  printProgress("Cleaning USER PATH = " + path);
  QStringList pathl = path.split(";"),
              clean_path,
              henhouses;

  for ( QStringList::Iterator it0 = pathl.begin(); it0 != pathl.end(); ++it0 ) {
        if ( (*it0).contains( QString("previewRZ") ) ){
           QString a_hen_house = (*it0).remove(QString("previewRZ"));
           henhouses.append(a_hen_house);
           //config_log << tr("Found HEN_HOUSE : ") + a_hen_house << "\n";
           printProgress("Found HEN_HOUSE : " + a_hen_house);
        }
  }
  if (henhouses.count()==0){
      //config_log << "\nNo need to clean PATH variable!!!\n";
      printProgress("\nNo need to clean PATH variable!!!\n");
      return false;
  }
  for ( QStringList::Iterator it = pathl.begin(); it != pathl.end(); ++it ) {
    for ( QStringList::Iterator it1 = henhouses.begin(); it1 != henhouses.end(); ++it1 ){
        if (!(*it).contains( *it1 )) clean_path.append(*it);
    }
  }
  path = clean_path.join(";");
  QString smsg;
  if ( ! replaceUserEnvironmentVariable( "PATH", path, &smsg) ){
    //config_log << "\n Failed cleaning/replacing the PATH\n";
    printProgress("\n Failed cleaning/replacing the PATH\n");
    return false;
  }
  else{
    //config_log << "\n Successfully cleaned PATH variable!!!\n";
    printProgress("\n Successfully cleaned PATH variable!!!\n");
    return true;
  }
#else
  //config_log << "\n EGSnrcMP_setup::cleanPath() is Windows only\n";
  printProgress("\n EGSnrcMP_setup::cleanPath() is Windows only\n");
  return false;
#endif
}

/*---------------------------------------------
  CREATES SHORTCUTS TO THE EGSnrc GUI's
  ON USER'S REQUEST
-----------------------------------------------*/
void QInstallPage::createEGSFolders(){

    QStringList gui;
    gui.append("egs_gui");
    gui.append("egs_inprz");
    gui.append("egs_view");
    QChar s =  QDir::separator();
    bool desk_exists = true;
    QString lnk; QString target;

#ifdef WIN32
    QString home = getenv( "USERPROFILE");
    int res;
    // User requests the folder on the desktop
    QString DESKTOP = home + s +(QString)"desktop" + s;
    QDir desk( DESKTOP );
    if ( ! desk.exists()) {
      if (! desk.mkdir(DESKTOP) ){
        QString error = (QString)"\n Could not create directory " + DESKTOP;
        QMessageBox::warning(this,"Warning", error,"&OK",0,0,0,-1);
        printProgress( error);
        desk_exists = false;
      }
    }
    else{
    //if ( desk_exists ) {
      for ( QStringList::Iterator it = gui.begin(); it != gui.end(); ++it ) {
          /* Temporarily needed until egs_view is ported to qt4 */
          QString desc, return_message, icon;
          if( *it == "egs_inprz" )
            desc = "EGSnrc GUI for the RZ user codes";
          else if( *it == "egs_gui" )
            desc = "EGSnrc GUI for all user codes";
          else desc = "EGSnrc GUI for egs++ geometries";
          lnk       = DESKTOP + *it + (QString)".lnk";
          if( *it != "egs_view" )
            target =  QDir::toNativeSeparators( egsBinDir  + s + *it + ".exe" );
          else{
            target =  QDir::toNativeSeparators( henHouse() + "egs++" + s + "dso" + s + "win-static" + s + *it + ".exe");
            icon   = henHouse() + "egs++" + s + "view" + s + *it + ".ico";
          }
          if ( ! fileExists( target ) ) {
             printProgress( target + " not found !" );
             continue;
          }
          if( *it != "egs_view" )
             res = createShortcut( target.toLatin1().data() , lnk.toLatin1().data(),
                                     desc.toLatin1().data(), return_message );
          else
             res = createShortcut( target.toLatin1() , lnk.toLatin1(),
                                    desc.toLatin1(),icon.toLatin1(),0, return_message );

          if ( res == 0 ) {
             printProgress( *it + " shortcut added to Desktop !" );
          }
          else {
             printProgress( "Errors updating EGSnrc folder on Desktop !" );
             printProgress( QString("Exit status = %1").arg( res ) );
             printProgress( return_message);
          }
      }
    }
#else
    // User requests the folder on the desktop
      QStringList guiscript;QString scriptdummy(gui_script), script, scriptstr;
      guiscript.append("egui"); guiscript.append("rzgui"); guiscript.append("viewgui");
      QString lnkdummy(guilnk); QString lnkstr;
      QString DESKTOP = QString(getenv("HOME")) + s + QString("Desktop") + s;
      QDir desk( DESKTOP );
      if ( ! desk.exists()) {
        if (! desk.mkdir(DESKTOP) ){
          QString error = QString("\n Could not create directory ") + DESKTOP;
          QMessageBox::warning(this,"ERROR", error,"&OK",0,0,0,-1);
          printProgress( error );
          desk_exists = false;
        }
      }
      if ( desk_exists ) {
        QString icon; short igs=0;
        for ( QStringList::Iterator it = gui.begin(); it != gui.end(); ++it ) {
            target = egsBinDir + s + *it;
            icon   = *it != "egs_view" ?
                     henHouse() + s + "gui" + s +
                     *it + s + "images"  + s + "desktop_icon.png" :
                     henHouse() + s + "gui" + s +
                     "egs_inprz" + s + "images"  + s + "bianchi.png";
            if ( ! fileExists( target ) ) {
              printProgress( target + " not found !" );
              continue;
            }
            /* Create GUI script and replace environment values */
            script = henHouse() + s + "scripts" + s + guiscript[igs++];
            scriptstr = scriptdummy;
            scriptstr.replace( "the_egshome",  egsHome() );
            scriptstr.replace( "the_henhouse",  henHouse() );
            scriptstr.replace( "the_egsconfig",specFile );
            if (*it == "egs_view"){
              scriptstr.replace( "#exportVIEW","export"  );
              //**************************************
              // Hardcoding dso location for egs_view
              //**************************************
              scriptstr.replace( "the_dso", henHouse() +
                                 "egs++" + s + "dso" + s + QString(EGS_VIEW_DSO) );
              //scriptstr.replace( "the_dso", dsoDir );
              /* Needed in Gnome based systems such as Ubuntu 16.04 */
              //target += " -style fusion";
            }
            scriptstr.replace( "gui_exe",      target );
            if ( ! writeQString2File( scriptstr, script ) ) {
              printProgress( "\n Error: could not create file " + script );
              continue;
            }
            else {
              printProgress( script + " successfully created!");
              chmod( QString("u+x"), script );
            }
            /* Create GUI desktop shortcuts and replace keys with values */
            lnk    = DESKTOP + *it + (QString)".desktop";
            lnkstr = lnkdummy;
            lnkstr.replace( "gui_script", script );
            lnkstr.replace( "gui_name", *it );
            lnkstr.replace( "gui_icon", icon );
            if ( ! writeQString2File( lnkstr, lnk ) ) {
              printProgress( "\n Error: could not create file " + lnk );
              continue;
            }
            else {
              printProgress( lnk + " successfully added to Desktop !");
              chmod( QString("a+x"), lnk );
            }
        }
      }
#endif

}

/*-----------------------------------------------------------------
  CREATES A FOLDER ON THE DESKTOP ON USER'S REQUEST WITH SHORTCUTS
  TO THE EGSnrc GUI's.
-------------------------------------------------------------------*/
void QInstallPage::createBeamFolders(){
      QStringList gui;
      gui.append("beamnrc_gui.tcl");
      gui.append("dosxyznrc_gui.tcl");
      gui.append("beamdp_gui.tcl");
      QChar s =  QDir::separator();
      QStringList gui_dir;// Dirs and executables names not the same :-(
      gui_dir.append("beamnrc");
      gui_dir.append("dosxyznrc");
      gui_dir.append("beamdp");
      QStringList desc;// description <= Windows
      desc.append("TCL/TK GUI for BEAMnrc");
      desc.append("TCL/TK GUI for DOSXYZnrc");
      desc.append("TCL/TK GUI for BEAMDP");
      QString omega_gui = henHouse() + "omega/progs/gui/";
      QString msg, lnk, target, icon;
#ifdef WIN32
      QStringList icons;// icon files <= Linux
      icons.append("beamnrc.ico");
      icons.append("dosxyznrc.ico");
      icons.append("beamdp.ico");

      QString home = getenv( "USERPROFILE");
      QString os   = getenv( "OS");

      // User requests the folder on the desktop
      QString desktop = home + s + (QString)"desktop" + s;
      createWinShortcuts(desktop,omega_gui,gui,gui_dir,desc,icons,msg);
      printProgress( msg );
#else
     QStringList icons;// script files <= Linux
     icons.append("beamnrc.png");
     icons.append("dosxyznrc.png");
     icons.append("beamdp.png");

     QStringList scripts;// icon files <= Linux
     scripts.append("beamgui");
     scripts.append("beamxyzgui");
     scripts.append("beamdpgui");

     QString home = getenv( "HOME");
     // User requests the folder on the desktop
     QString desktop = home + s + (QString)"Desktop" + s ;
     createKDEShortcuts( desktop, omega_gui, gui, icons, scripts, gui_dir, msg );
     printProgress( msg );
#endif

}

void QInstallPage::createWinShortcuts( const QString& where,
                                       const QString& from,
                                             QStringList& link,
                                             QStringList& dir,
                                             QStringList& desc,
                                             QStringList& icons,
                                             QString& msg     )
{
      msg = QString();
      QString target, icon, lnkstr, lnk, descr, return_message;
      QChar s =  QDir::separator();
      int res, index = 0;
      //if ( make_dir(where) ) {
      if (QDir().mkpath(where) ){
          QStringList::Iterator itd = dir.begin();
          QStringList::Iterator ite = desc.begin();
          QStringList::Iterator iti = icons.begin();
          for ( QStringList::Iterator it = link.begin(); it != link.end(); ++it ) {
           target = from + *itd + s + *it;
           icon   = from + *itd + s + *iti;
           descr  = *ite;
           if (itd != dir.end()) itd++;
           if (ite != desc.end())ite++;
           if (iti != icons.end())iti++;
           if (!QFile::exists( target )){
           //if ( ! fileExists( target ) ) {
               msg += target + " not found !\n";
               continue;
           }
           if (!QFile::exists( icon )){
           //if ( ! fileExists( icon ) ) {
              msg += icon + " not found !\n";
              continue;
           }

           lnk = where + *it + QString(".lnk");
           res = createShortcut( target.toLatin1() , lnk.toLatin1(),
                       descr.toLatin1(),icon.toLatin1(),0, return_message );
           if ( res == 0 ) {
            //msg += *it + (QString)" shortcut added to Desktop !\n";
           }
           else {
            msg += "Errors updating shortcut !\n";
            msg += "Exit status = " + QString::number(res) + "\n";
            msg += return_message   + "\n";
           }
           index++;
          }
      }
}

void QInstallPage::createKDEShortcuts( const QString& where,
                                       const QString& from,
                                             QStringList& link,
                                             QStringList& icons,
                                             QStringList& scripts,
                                             QStringList& dir,
                                             QString& msg     )
{
       msg = QString();
       QString target, icon, lnkdummy(guilnk), lnkstr, lnk,
               scriptdummy(gui_script), script, scriptstr;
       QChar s =  QDir::separator();
       //if ( make_dir(where) ) {
       if (QDir().mkpath(where) ){
         QString henhouse  = henHouse(),
                 egsconfig = specFile;
         QStringList::Iterator itd = dir.begin();
         QStringList::Iterator iti = icons.begin();
         QStringList::Iterator its = scripts.begin();
         for ( QStringList::Iterator it = link.begin(); it != link.end(); ++it ) {
               target = from + *itd + s + *it;
               icon   = from + *itd + s + *iti;
               script = henhouse + s + "scripts" + s + *its;
               if (!QFile::exists( target )){
               //if ( ! fileExists( target ) ) {
                  msg += target + (QString)" not found !\n";
                  continue;
               }
               /* Create GUI script and replace environment values */
               scriptstr = scriptdummy;//re-setting original template
               scriptstr.replace( "the_egshome",  egsHome() );
               scriptstr.replace( "the_henhouse",  henHouse() );
               scriptstr.replace( "the_egsconfig", egsconfig );
               scriptstr.replace( "#exportBEAM","export"  );
               if (*itd != "beamdp")
                  scriptstr.replace( "ehBinDir", homeBinDir );
               else
                  scriptstr.replace( "ehBinDir", egsBinDir );
               scriptstr.replace( "gui_exe",      target );
               if ( ! writeQString2File( scriptstr, script ) ) {
                   msg +=QString("\n Error: could not create file ")
                               + script + QString("\n");
                   continue;
               }
               else {
                 msg += script + QString(" successfully created!\n");
                 chmod( QString("u+x"), script );
               }
               /* Create GUI desktop shortcuts and replace keys with values */
               lnk    = where + *it + QString(".desktop");
               lnkstr = lnkdummy;//re-setting original template
               lnkstr.replace( "gui_script", script );
               lnkstr.replace( "gui_name", *itd );
               lnkstr.replace( "gui_icon", icon);
               if ( ! writeQString2File( lnkstr, lnk ) ) {
                      msg +=QString("\n Error opening file " + lnk + " for writing!");
                      msg +=QString("\n Check that it exists or could be created ... \n");
               }
               else{
                      msg += lnk+(QString)" successfully added to "+ where +(QString)"\n";
               }
               if (itd != dir.end())  itd++;
               if (iti != icons.end())iti++;
               if (its != scripts.end())its++;
         }
       }
}

void QInstallPage::setup_static_guis(){
  /* Determine OS architecture */
  QString arch;
  if (canonical().contains("64") )
      arch = QString("_64");
  else
      arch = QString("_32");
  QString piecesLinux = henHouse() + QString("pieces") + QDir::separator()+
                                     QString("linux") + QDir::separator();
  QStringList gui;  QChar s =  QDir::separator(); QString lnkname, target, syslink;
  gui.append("egs_gui"); gui.append("egs_inprz"); gui.append("egs_view");
  for ( QStringList::Iterator it = gui.begin(); it != gui.end(); ++it ) {
      lnkname = egsBinDir + s + *it;
      target  = piecesLinux + *it + arch;
      syslink = QString("ln -s ") + target + QString(" ") + lnkname ;
      if (!fileExists(lnkname)){
        if (fileExists(target)){
            chmod( QString("u+x"), target );//  make executable
            if ( system( syslink.toLatin1() ) )
               printProgress("\nFailed executing " + syslink);
            else
               printProgress("\n->Created symbolic link " + lnkname);
        }
        else
               printProgress("\n-> " + target + " not found !");
      }
      else
        printProgress("\n-> " + lnkname + " seems to exist ... leaving as is!");
  }

}
/**************************************************************
 * NOT NEEDED. egs_view is not looking for the location below.
 * It will pick the egspp library defined by LD_LIBRARY_PATH
 *
 * Since egs_view is built statically using my_machine = linux32 or linux64 one needs
   to establish a sys link to the actual dso/my_machine
 ***************************************************************/
void QInstallPage::set_guis_dso(){
    QString dsoDirR = henHouse()     + QString("egs++") + QDir::separator()+
                      QString("dso") + QDir::separator()+ my_machine();
    QString dsoDirS = henHouse()     + QString("egs++") + QDir::separator() +
                      QString("dso") + QDir::separator() ;
    /* Determine OS architecture */
    if (canonical().contains("64") )
        dsoDirS += QString("linux64");
    else
        dsoDirS += QString("linux32");
   /*********************************************
    * Create symbolic link to actual dso folder
    * if different from dsoDir.
    ********************************************/
    if (!QDir(dsoDirS).exists()){
       QString syslink = QString("ln -s ") + dsoDirR + QString(" ") + dsoDirS ;
       if ( system( syslink.toLatin1() ) )
          printProgress(tr( "\nFailed creating symbolic link ") + dsoDirS );
       else
          printProgress( tr("\n->Created symbolic link ") + dsoDirS );
    }
    else
       printProgress( tr("\nNo need to create symbolic link ") + dsoDirS );
}

/**************************************************************
 * XDG / LOCALAPPDATA profile helpers (shared with shell configure)
 **************************************************************/
QString QInstallPage::egsnrcConfigHome(){
#ifdef Q_OS_WIN
    QString base = qgetenv("LOCALAPPDATA");
    if ( base.isEmpty() ) {
        QString prof = qgetenv("USERPROFILE");
        if ( ! prof.isEmpty() )
            base = prof + QDir::separator() + "AppData" + QDir::separator() + "Local";
    }
    if ( base.isEmpty() )
        return QString();
    return QDir::cleanPath( base + QDir::separator() + "EGSnrc" );
#else
    QString override = qgetenv("EGSNRC_CONFIG_HOME");
    if ( ! override.isEmpty() )
        return QDir::cleanPath( override );
    QString base = qgetenv("XDG_CONFIG_HOME");
    if ( base.isEmpty() ) {
        QString home = qgetenv("HOME");
        if ( home.isEmpty() )
            home = getenv( "HOME" );
        if ( home.isEmpty() )
            return QString();
        base = home + QString("/.config");
    }
    return QDir::cleanPath( base + QDir::separator() + "EGSnrc" );
#endif
}

QString QInstallPage::egsnrcProfileName(){
    QString name = field("egs_profile").toString().trimmed();
    if ( name.isEmpty() )
        name = egsnrcDefaultProfileName( henHouse() );
    name = egsnrcSanitizeProfileName( name );
    if ( name.isEmpty() )
        name = QString("default");
    return name;
}

QString QInstallPage::egsnrcProfileDir(){
    return egsnrcConfigHome() + QDir::separator() + "profiles" +
           QDir::separator() + egsnrcProfileName();
}

QString QInstallPage::egsnrcUserSpecsDir(){
    return egsnrcProfileDir() + QDir::separator() + "specs" + QDir::separator();
}

bool QInstallPage::installXdgShellFiles(){
    const QString cfgHome = egsnrcConfigHome();
    if ( cfgHome.isEmpty() )
        return false;
    QDir().mkpath( cfgHome );
    const QString srcDir = henHouse() + QString("scripts") + QDir::separator() +
                           QString("xdg") + QDir::separator();
    const QStringList files = QStringList()
        << QString("EGSnrc.bash") << QString("EGSnrc.csh") << QString("EGSnrc.fish");
    for ( QStringList::const_iterator it = files.begin(); it != files.end(); ++it ) {
        const QString src = srcDir + *it;
        const QString dst = cfgHome + QDir::separator() + *it;
        if ( QFile::exists( src ) ) {
            if ( QFile::exists( dst ) )
                QFile::remove( dst );
            QFile::copy( src, dst );
        }
    }
    return true;
}

bool QInstallPage::writeXdgProfile(){
    const QString cfgHome = egsnrcConfigHome();
    if ( cfgHome.isEmpty() )
        return false;
    const QString profile = egsnrcProfileName();
    const QString profDir = egsnrcProfileDir();
    QDir().mkpath( egsnrcUserSpecsDir() );

    if ( ! writeQString2File( profile + QString("\n"),
                              cfgHome + QDir::separator() + "active_profile" ) )
        return false;

    QString hh = henHouse();
    if ( ! hh.endsWith( QDir::separator() ) )
        hh += QDir::separator();
    if ( ! writeQString2File( hh, profDir + QDir::separator() + "hen_house" ) )
        return false;

    if ( ! writeQString2File( my_machine(),
                              profDir + QDir::separator() + "active_conf" ) )
        return false;

    QString env = QString("# EGSnrc profile environment — profile: %1\n").arg( profile );
    env += QString("export EGS_HOME=\"%1\"\n").arg( egsHome() );
    env += QString("export EGS_CONFIG=\"%1\"\n").arg( specFile );
    if ( ! writeQString2File( env, profDir + QDir::separator() + "env" ) )
        return false;

    return installXdgShellFiles();
}

bool QInstallPage::ensureUnixShellOneliner(){
    QString home = qgetenv("HOME");
    if ( home.isEmpty() )
        home = getenv( "HOME" );
    if ( home.isEmpty() )
        return false;

    QString shell = qgetenv("SHELL");
    if ( shell.isEmpty() )
        shell = getenv( "SHELL" );
    shell = shell.mid( shell.lastIndexOf( QDir::separator() ) + 1 );

    const QString cfgHome = egsnrcConfigHome();
    if ( cfgHome.isEmpty() )
        return false;

    QString oneliner;
    QString rcfile = home + QDir::separator();
    if ( shell == QString("tcsh") || shell == QString("csh") ) {
        oneliner = QString("if ( -f \"%1/EGSnrc.csh\" ) source \"%1/EGSnrc.csh\"\n")
                       .arg( cfgHome );
        rcfile += QString(".cshrc");
    }
    else if ( shell == QString("fish") ) {
        oneliner = QString("if test -f \"%1/EGSnrc.fish\"; source \"%1/EGSnrc.fish\"; end\n")
                       .arg( cfgHome );
        rcfile = QDir::homePath() + QDir::separator() + QString(".config") +
                 QDir::separator() + QString("fish") + QDir::separator() +
                 QString("config.fish");
    }
    else {
        oneliner = QString("[ -f \"%1/EGSnrc.bash\" ] && . \"%1/EGSnrc.bash\"\n")
                       .arg( cfgHome );
        if ( shell == QString("bash") || shell == QString("zsh") ||
             shell == QString("ksh") || shell == QString("sh") )
            rcfile += QString(".bashrc");
        else
            rcfile = home + QDir::separator() + QString(".profile");
    }

    if ( ! QFile::exists( rcfile ) )
        return writeQString2File( oneliner, rcfile );

    const QString content = readFile2QString( rcfile, QString() );
    if ( content.contains( cfgHome + QString("/EGSnrc.bash") ) ||
         content.contains( cfgHome + QString("/EGSnrc.csh") ) ||
         content.contains( cfgHome + QString("/EGSnrc.fish") ) )
        return true;

    printProgress( tr("\n->Appending EGSnrc shell entry to ") + rcfile );
    return appendQString2File( oneliner, rcfile );
}

/**************************************************************
 * Fixing LD_LIBRARY_PATH to point to egsnrc64 which provides
 * precompiled egspp and all dso files needed by egs_view.
 **************************************************************/
void QInstallPage::update_unix_env(){
    printProgress( tr("\n->Writing XDG EGSnrc profile to ") +
                   egsnrcConfigHome() + tr("\n") );
    if ( ! writeXdgProfile() )
        printProgress( tr("\n Warning: could not write XDG profile files.\n") );

    printProgress( tr("\n->Ensuring shell RC sources the XDG EGSnrc entry\n") );
    if ( ! ensureUnixShellOneliner() )
        printProgress( tr("\n Warning: could not update shell resource file.\n") );
}

void QInstallPage::cleanUp()
{
  //qDebug("Cleaning up directory %s",QDir::currentPath().toLatin1().data());
  delete_file("junk1"); delete_file("junk4");
  if ( fileExists( "file_size.o" ) ){
    move_file("file_size.o", egsLibDir + (QString)"/file_size.o");
  }
  else if ( fileExists( "file_size.obj" ) ){
    move_file( "file_size.obj", egsLibDir + (QString)"/file_size.o");
  }
  if ( fileExists( "NameDecoration.o" ) ){
    move_file("NameDecoration.o", egsLibDir + (QString)"/NameDecoration.o");
  }
  else if ( fileExists( "NameDecoration.obj" ) ){
    move_file( "NameDecoration.obj", egsLibDir + (QString)"/NameDecoration.o");
  }

  delete_files( QString("*.o;*.obj;*.map;*.rsp;*.f;*.c") );
  delete_file( "make.exe" );

}

/*! Copy files from source directory to target directory defined by the given filters
     To copy all files ending with either ".cpp" or ".h", you would use "*.cpp *.h"
  */
bool QInstallPage::copy_files( const QString& src, const QString& trgt,  const QString& filter ){
      QString source = src, target = trgt;
     if (!source.endsWith(QDir::separator()))  source += QDir::separator();
     if (!target.endsWith(QDir::separator())) target += QDir::separator();
     bool success = true; QDir dir( source );
     QFileInfoList filist = dir.entryInfoList( filter.split(" "),  QDir::Files, QDir::DirsFirst | QDir::Name );
     for ( int i = 0; i < filist.size(); i++ ) {
           qApp->processEvents();
           if ( filist.at(i).isFile() ){
              if ( ! copy( source + filist.at(i).fileName(), target + filist.at(i).fileName() ) ){
                 printProgress( "Failed copying " + source + filist.at(i).fileName() + " to "
                                                  + target + filist.at(i).fileName() );
                 success = success && false;
              }
          }
     }
     return success;
}

//----------------------------------------------------------------------
//    UNCOMPRESSING TOOLS TO AVOID LOCKING THE GUI
//   AND PRINTING MESSAGES DURING THE PROCESS
//----------------------------------------------------------------------
int res = -1; QTimer *timer = 0;
void QInstallPage::extract( const char* archive, const char* dir ){

   printProgress("\n\n => Extracting GUIs ... \n");

   // create an archive object
//    t = new EGSThread( this, archive , dir );
//    t->start();
//    emit threadRunning();

   EGS_Archive *ar = new EGS_Archive(0, this, 1);
   res = ar->extract( archive, dir );
   timer = new QTimer(this);
   connect(timer, SIGNAL(timeout()), this, SLOT(processExtaction()));
   timer->start(20);
}

QMutex my_mutex;

void QInstallPage::processExtaction(){

/*  while( !t->wait(20) ) {
    my_mutex.lock();
    qApp->processEvents();
    my_mutex.unlock();
  }

  int res= t->getStatus();
  */

  if ( res == 0 )
    printProgress( "\n Archive(s) successfully uncompressed ... \n");
  else if ( res == 8 )
        printProgress("\n Uncompressing process cancelled! \n");
  else{
        QString strm ="===== Debugging message ==========\n";
        strm += QString("Uncompressing return code = %1").arg(res);
        strm += QString("\n Could not uncompress archive(s), see ") +
                config_file->fileName()                             +
                QString(" for details ... \nQuitting early! \n" );
        printProgress(strm);
        QMessageBox::critical(this,"Error uncompressing archive(s)",
                              strm,"&OK",0,0,0,-1);
  }
  timer->stop();
  //emit AllDone();

}

void QInstallPage::customEvent(QEvent * event){

  switch (static_cast<EGSCustomEvent *>(event)->type()) {
    case CustomEventString:
    {
      printProgress(static_cast<EGSCustomEvent *>(event)->getStrData());
      qApp->processEvents();
      break;
    }
    case CustomEventInt:
    {
      progressBar->setValue( static_cast<EGSCustomEvent *>(event)->getIntData() );
      qApp->processEvents();
      break;
    }
    default:
    {
      qWarning("Unknown custom event type: %d", event->type());
    }
  }

}
