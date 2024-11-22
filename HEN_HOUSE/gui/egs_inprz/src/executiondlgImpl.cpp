/*
###############################################################################
#
#  EGSnrc egs_inprz execution dialog
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
#  Author:          Ernesto Mainegra-Hing, 2002
#
#  Contributors:    Blake Walters
#                   Cody Crewson
#                   Reid Townson
#
###############################################################################
*/


#include "executiondlgImpl.h"
#include "tooltips.h"
#include "queuedef.h"
#include <qapplication.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qtooltip.h>
//#include <q3whatsthis.h>

//qt3to4 -- BW
#include <qdir.h>

ExecutiondlgImpl::ExecutiondlgImpl( QWidget* parent, const char* name, bool modal, Qt::WindowFlags f )
//qt3to4 -- BW
//           : MExecutionDialog( parent, name, modal, f )
             : QDialog(parent)
{

  //qt3to4 -- BW
  setupUi(this);
  the_command = name;
  init();

  //Q3WhatsThis::add ( interactiveRadioButton, RUN_STAND_ALONE );
  interactiveRadioButton->setWhatsThis(RUN_STAND_ALONE );
  interactiveRadioButton->setToolTip(RUN_STAND_ALONE );

  //Q3WhatsThis::add (  batchRadioButton, RUN_PARALLEL );
  batchRadioButton->setWhatsThis(RUN_PARALLEL );
  batchRadioButton->setToolTip(RUN_PARALLEL );

  //Q3WhatsThis::add (  NumJobSpinBox, NUMBER_OF_JOBS );
  NumJobSpinBox->setWhatsThis(NUMBER_OF_JOBS );
  NumJobSpinBox->setToolTip(NUMBER_OF_JOBS );

  //Q3WhatsThis::add (  QueueComboBox, QUEUE_TYPE );//      since it is implementation dependent
                                                    //     i.e., the batch submission system being used
    QueueComboBox->setWhatsThis(QUEUE_TYPE );
    QueueComboBox->setToolTip(QUEUE_TYPE );
//  QToolTip::add(  StartJobSpinBox, START_JOB );
//  QWhatsThis::add (  StartJobSpinBox, START_JOB );

// specify type of queue
  QueueComboBox->clear();
  for (uint i = 0; i < sizeof queue_type / sizeof(char *) ; i++)
         QueueComboBox->addItem( queue_type[i] );
// For specific settings of the BATCH SUBMISSION SYSTEM
   getQueueingSystemOptions();

}

ExecutiondlgImpl::~ExecutiondlgImpl()
{

}

//qt3to4 -- BW
void ExecutiondlgImpl::getQueueingSystemOptions() {
  QString script_dir = the_hen_house + "scripts";
  QDir dir(script_dir);
  dir.setNameFilters(QStringList("batch_options.*"));
  QStringList list = dir.entryList();
  queueSystemcomboBox->clear();
  for(QStringList::Iterator it=list.begin(); it != list.end(); ++it) {
      QString aux = *it;
      if( !aux.endsWith("~") && !aux.endsWith(".bak") ) {
          aux.replace("batch_options.","");
          queueSystemcomboBox->addItem(aux);
      }
  }
  char *ebs = getenv("EGS_BATCH_SYSTEM");
  if( ebs )
      queueSystemcomboBox->setItemText(queueSystemcomboBox->currentIndex(),ebs);
}
//qt3to4 -- BW
void ExecutiondlgImpl::init()
{
  QStringList tmpExecStr = the_command.split(" ");
   the_hen_house = tmpExecStr[0];
}
//qt3to4 -- BW
void ExecutiondlgImpl::update_batch()
{
   if ( batchRadioButton->isChecked() )
       batchGroupBox->setEnabled(true);
   else
        batchGroupBox->setEnabled(false);
}

//qt3to4 -- BW
void ExecutiondlgImpl::run()
{

     QString exec_string = the_command;

     QString msg = "\n\nINTERACTIVE RUN COMPLETED !!!";

     //----------------------------
     // exec_string contains :
     //----------------------------
     // HEN_HOUSE + user-code name + executable + input file name + data file name
     //        0                          1                           2                          3                        4
     QStringList tmpExecStr = exec_string.split(" ");
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

    CommandManager* run_egs = new CommandManager( 0, vTitle.toLatin1().data(), args );
                    run_egs->setEndMessage( msg );
    //run_egs->exec(); //runs modal
    run_egs->show(); //runs modeless

}
