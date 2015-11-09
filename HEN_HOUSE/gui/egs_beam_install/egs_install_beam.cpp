/*
###############################################################################
#
#  EGSnrc configuration GUI beamnrc setup
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
#  Contributors:
#
###############################################################################
*/


#include "egs_install.h"

#ifdef WIN32
#define PROGS_N 5
#else
#define PROGS_N 6
#endif

static const char* progs_dir[] = {
             "addphsp",
             "beamdp",
             "ctcreate",
             "readphsp",
             "statdose"
#ifndef WIN32
             ,"dosxyz_show"
#endif
};

void QInstallPage::beamInstall()
{
     /*if (!beamCheckBox->isChecked()){
         emit beamDone();
         return;
     }*/
     resetProgressBar( NUMBER_OF_STEPS - beamBuild );
     setSubTitle("Configuring BEAMnrc");

     disconnect( this, SIGNAL( nextBuildStep( ushort )),
                 this, SLOT( buildEGSnrc( ushort )) );
        connect( this, SIGNAL( nextBuildStep( ushort )),
                 this, SLOT( buildBEAMnrc( ushort )) );
        connect( this, SIGNAL( exampleModulesCopied() ),
                 this, SLOT( finalize_beam_setup() ) );

     installing_beam = true;

     buildBEAMnrc( beamBuild );
}

void QInstallPage::buildBEAMnrc( ushort code )
{
  switch( code ){
    case beamBuild:
         updateProgress();
         buildFlag = addPhSp;
         printProgress("\n\n => Compiling tool to create BEAMnrc user codes ...\n\n");
         buildEGSCode( henHouse() + "omega/beamnrc/tools/" );
         break;
    case addPhSp:
         updateProgress();
         buildFlag = beamDP;
         printProgress(tr("\n\n => Compiling BEAMnrc tool ") +
                       QString(progs_dir[0])                 +
                       tr(" ...\n\n")
                 );
         buildEGSCode( henHouse() + tr("omega/progs/") + QString(progs_dir[0]) + QDir::separator() );
         break;
    case beamDP:
         updateProgress();
         buildFlag = ctCreate;
         printProgress(tr("\n\n => Compiling BEAMnrc tool ") +
                       QString(progs_dir[1])                 +
                       tr(" ...\n\n")
                 );
         buildEGSCode( henHouse() + tr("omega/progs/") + QString(progs_dir[1]) + QDir::separator() );
         break;
    case ctCreate:
         updateProgress();
         buildFlag = readPhSp;
         printProgress(tr("\n\n => Compiling BEAMnrc tool ") +
                       QString(progs_dir[2])                 +
                       tr(" ...\n\n")
                 );
         delete_files(henHouse() + tr("omega/progs/") + QString(progs_dir[2]) + QDir::separator(),"*.o;*.obj");
         buildEGSCode( henHouse() + tr("omega/progs/") + QString(progs_dir[2]) + QDir::separator() );
         break;
    case readPhSp:
         updateProgress();
         buildFlag = statDose;
         printProgress(tr("\n\n => Compiling BEAMnrc tool ") +
                       QString(progs_dir[3])                 +
                       tr(" ...\n\n")
                 );
         buildEGSCode( henHouse() + tr("omega/progs/") + QString(progs_dir[3]) + QDir::separator() );
         break;
    case statDose:
         updateProgress();
#ifndef WIN32
         buildFlag = DOSxyzShow;
#else
         buildFlag = buildDOSXYZnrc;
#endif
         printProgress(tr("\n\n => Compiling BEAMnrc tool ") +
                       QString(progs_dir[4])                 +
                       tr(" ...\n\n")
                 );
         buildEGSCode( henHouse() + tr("omega/progs/") + QString(progs_dir[4]) + QDir::separator() );
         break;
#ifndef WIN32
    case DOSxyzShow:
         updateProgress();
         buildFlag = buildDOSXYZnrc;
         printProgress(tr("\n\n => Compiling BEAMnrc tool ") +
                       QString(progs_dir[5])                 +
                       tr(" ...\n\n")
                 );
         buildEGSCode( henHouse() + tr("omega/progs/") + QString(progs_dir[5]) + QDir::separator() );
         break;
#endif
    case buildDOSXYZnrc:
         updateProgress();
         buildFlag = exampleModules;
         printProgress("\n\n => Compiling DOSXYZnrc(in-phantom dose calculation tool)...\n\n");
         buildEGSCode( egsHome() + "dosxyznrc" );
         break;
    case exampleModules:
         updateProgress();
         //if ( ucCheckBox->isChecked() )
         if ( needsUCs())
            copy_example_modules();
         emit beamDone();
         break;
    default:
          emit AllDone();
          break;
  }
}

/*
*****************************************************************
  STEP 4. Copy standard accelerator modules and example modules
          to EGSnrcMP user area.
*****************************************************************
*/
void QInstallPage::copy_example_modules(){

   printProgress(
   (QString)"\n => Copying example accelerators to user area ....\n\n");

   QString eh = egsHome(); eh.chop(1);
   QString source = henHouse() + tr("omega/beamnrc/BEAMnrc_examples");
   if ( ! copyRecursively( source, eh ) ){
     printProgress( tr("\nError copying example accelerator files from ")   +
                    henHouse() + tr("omega/beamnrc/BEAMnrc_examples/ to ") +
                    egsHome());
   }

   // Copy the example modules to EGS_HOME/beamnrc/spec_modules
   createDir( egsHome() + tr("beamnrc") );
   createDir( egsHome() + tr("beamnrc/spec_modules") );
   QString target = egsHome() + tr("beamnrc/spec_modules");
   if ( ! copyFilesRecursively( source, target, QString("*.module") ) ){
     printProgress( tr("\nError copying example accelerator modules from ")   +
                    henHouse() + tr("omega/beamnrc/BEAMnrc_examples to ") +
                    egsHome() + tr("beamnrc/spec_modules\n") );
   }

   foreach (const QString &dirName, QDir(egsHome()).entryList( QDir::Dirs | QDir::NoDotAndDotDot)) {
           if (dirName.startsWith("EX"))
              QDir(egsHome() + dirName).rename(egsHome()+dirName, egsHome()+"BEAM_" + dirName);
   }

}

/*
******************************************************************
  STEP 5. Create shortcuts, show final message, create README file
******************************************************************
*/
void QInstallPage::finalize_beam_setup(){

    emit beamDone();
}
