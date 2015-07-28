/*
###############################################################################
#
#  EGSnrc user interface headers for egs_inprz execution dialog
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


/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/


#include "commandManager.h"
#include <qmessagebox.h>

void MExecutionDialog::init()
{
  QStringList tmpExecStr = tmpExecStr.split(" ", name(), false);
   the_hen_house = tmpExecStr[0];
}
void MExecutionDialog::update_batch()
{

//#ifdef WIN32
//    batchRadioButton->setEnabled(false);
//     batchGroupBox->setEnabled(false);
//#else
   if ( batchRadioButton->isChecked() )
       batchGroupBox->setEnabled(true);
   else
        batchGroupBox->setEnabled(false);
//#endif

}

void MExecutionDialog::run()
{

     QString exec_string = name();

     QString msg = "\n\nINTERACTIVE RUN COMPLETED !!!";

     //----------------------------
     // exec_string contains :
     //----------------------------
     // HEN_HOUSE + user-code name + executable + input file name + data file name
     //        0                          1                           2                          3                        4
     QStringList tmpExecStr = tmpExecStr.split(" ", exec_string, false);
     the_hen_house = tmpExecStr[0];
     QString vTitle = "Running : " + tmpExecStr[1] + " " + tmpExecStr[3] + " " + tmpExecStr[4] ;

     QStringList args;

#ifdef  WIN32
//---------------------------------------------------------------------------------
//   ON WINDOWS WE RUN THE CODE INTERACTIVELY ONLY
//---------------------------------------------------------------------------------
      args.push_back(      tmpExecStr[2] );
      args += "-i";args  += tmpExecStr[3];
      if(tmpExecStr[4]!="pegsless") args += "-p";args += tmpExecStr[4];
//---------------------------------------------------------------------------------
#else
     QString egs_run       = tmpExecStr[2]; // default
     QString snum_jobs = "";
     QString queueSys    = "batch=";
     QString queue          = "";
     if ( batchRadioButton->isChecked() ){               // run remotely
        egs_run = the_hen_house + "scripts/run_user_code_batch";
        if (NumJobSpinBox->value() == 1 ){
           //vTitle = "Sending remote job: " +  tmpExecStr[1] + " " + tmpExecStr[3] + " " + tmpExecStr[4] ;
           vTitle = "Sending remote job: " +  tmpExecStr[1];
           msg = "\n\nJOB SUBMITTED TO THE QUEUE !!!";
        }
       else{
   vTitle = "Sending " + NumJobSpinBox->text() + " remote jobs";
   //vTitle = "Sending" + NumJobSpinBox->text() + " remote jobs: " +
          //tmpExecStr[1] + " " + tmpExecStr[3] + " " + tmpExecStr[4]  ;
          snum_jobs = (QString)"p=" + NumJobSpinBox->text();
           msg = "\n\nJOBS SUBMITTED TO THE QUEUE !!!";
        }
       queue          = QueueComboBox->currentText () ;
       queueSys += queueSystemcomboBox->currentText () ;

       args.push_back(egs_run);
       args += tmpExecStr[1];
       args += tmpExecStr[3];
       args += tmpExecStr[4];
       args += queue;
       args += queueSys;
       args += snum_jobs;
     }
     else{
            args.push_back(egs_run);
            args += "-i";args  += tmpExecStr[3];
            if(tmpExecStr[4]!="pegsless") args += "-p";args += tmpExecStr[4];
     }

#endif

 //    QMessageBox::information( this, " INFO",args.join(", "), QMessageBox::Ok );

    CommandManager* run_egs = new CommandManager( 0, vTitle, args );
                    run_egs->setEndMessage( msg );
    //run_egs->exec(); //runs modal
    run_egs->show(); //runs modeless

}

void MExecutionDialog::showQueueTipLabel( int index )
{

}

void MExecutionDialog::getQueueingSystemOptions() {
  QString script_dir = the_hen_house + "scripts";
  QDir dir(script_dir);
  dir.setNameFilter("batch_options.*");
  QStringList list = dir.entryList();
  queueSystemcomboBox->clear();
  for(QStringList::Iterator it=list.begin(); it != list.end(); ++it) {
      QString aux = *it;
      if( !aux.endsWith("~") && !aux.endsWith(".bak") ) {
          aux.replace("batch_options.","");
          queueSystemcomboBox->insertItem(aux);
      }
  }
  char *ebs = getenv("EGS_BATCH_SYSTEM");
  if( ebs )
      queueSystemcomboBox->setCurrentText(ebs);
}





