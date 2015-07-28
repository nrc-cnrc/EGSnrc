/*
###############################################################################
#
#  EGSnrc egs_inprz command manager
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
#  Contributors:    Blake Walters
#
###############################################################################
*/


#include "commandManager.h"
#include <qvariant.h>   // first for gcc 2.7.2
#include <qstringlist.h>
#include <qtimer.h>
#include <stdlib.h>
//qt3to4 -- BW
#include <QTextEdit>
#include <QProcess>


CommandManager::CommandManager(QWidget * parent, const char * name, const QStringList & args )
	: QDialog( parent)
{

    EndMessage = "\n\n EXECUTION COMPLETED ! ! ! "; // default

    // Layout
    setFixedSize ( 550, 500 ) ;
    //setCaption( name );
    setWindowTitle(name);
    //qt3to4 -- BW
    //output = new Q3TextEdit( this );
    output = new QTextEdit( this );
    output->setReadOnly( true );
    output->setGeometry( QRect( 20, 20, 510, 430 ) );
    //qt3to4 -- BW
    //output->setHScrollBarMode ( Q3ScrollView::Auto );
    output->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    //output->setWordWrap( Q3TextEdit::NoWrap );
    output->setWordWrapMode(QTextOption::NoWrap);
//    output->setTextFormat( Qt::RichText );

    quitButton = new QPushButton( tr("&Close"), this );
    quitButton->move(390, 465);
    connect( quitButton, SIGNAL(clicked()),
            this, SLOT(close()) );
    quitButton->setEnabled( false );

    killButton = new QPushButton( tr("&Kill"), this );
    killButton->move(270, 465);
    connect( killButton, SIGNAL(clicked()),
            this, SLOT(killProcess()) );

    // QProcess related code
    //qt3to4 -- BW
    //proc = new Q3Process( this );
    proc = new QProcess( this );

    //proc->clearArguments();
    // Set up the command and arguments.
    //qt3to4 -- BW
    //proc->setArguments ( args );

   //proc->setCommunication(QProcess::Stdout | QProcess::Stderr | QProcess::DupStderr);
   connect( proc, SIGNAL(readyReadStandardOutput()),
            this, SLOT(readFromStdout()) );
   connect( proc, SIGNAL(readyReadStandardError()),
            this, SLOT(readFromStderr()) );
    connect( proc, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(scrollToTop()) );

//    output->setText( (QString)"<u>EXECUTING  :</u>"+args.join( " " )  );
    //qt3to4 -- BW
    //output->insert( (QString)"EXECUTING  : "+args.join( " " ) +(QString)"\n",
    //		       (uint)Q3TextEdit::CheckNewLines | Q3TextEdit::RemoveSelected  );
    output->insertPlainText( (QString)"EXECUTING  : "+args.join( " " ) +(QString)"\n");

   //qt3to4 -- BW
    QString prog = args.first();
    QStringList args2 = args.mid(1);
    proc->start(prog,args2);

    //qt3to4 -- BW
    //if ( !proc->start() ) {
    if(proc->error()==QProcess::FailedToStart) {
        // error handling
        //QString errorM = "Could not start the command: \n" +args.join( "\n" );
        QString errorM = args.join( "\n" );
        QMessageBox::critical( 0,
                tr("Fatal error"),
	  tr("Could not start the command: \ni %1").arg(errorM),
                tr("Quit") );
        //exit( -1 );
    }
}

CommandManager::CommandManager(QWidget * parent, const char * name, int width, int height, const QStringList & args )
	: QDialog( parent )
{
    EndMessage = "\n\n EXECUTION COMPLETED ! ! ! "; // default
    // Layout
    setFixedSize ( width, height ) ;
    setWindowTitle( name );
    //qt3to4 -- BW
    //output = new Q3TextEdit( this );
    output = new QTextEdit( this );
    output->setReadOnly( true );
    output->setGeometry( QRect( 20, 20, width - 40, height - 100 ) );

    quitButton = new QPushButton( tr("Hide"), this );
    quitButton->move(width - 160, height - 75);
    connect( quitButton, SIGNAL(clicked()),
            this, SLOT(close()) );

    // QProcess related code
    //qt3to4 -- BW
    //proc = new Q3Process( this );
    proc = new QProcess( this );

    // Set up the command and arguments.
    //qt3to4 -- BW
    //proc->setArguments ( args );

   connect( proc, SIGNAL(readyReadStandardOutput()),
            this, SLOT(readFromStdout()) );
   connect( proc, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(scrollToTop()) );

    //qt3to4 -- BW
    QString prog = args.first();
    QStringList args2 = args.mid(1);
    proc->start(prog,args2);

    //qt3to4 -- BW
    //if ( !proc->start() ) {
    if(proc->error()==QProcess::FailedToStart) {
        // error handling
        QString errorM = args.join( "\n" );
        QMessageBox::critical( 0,
                tr("Fatal error"),
	  tr("Could not start the command: \n %1").arg(errorM),
                tr("Quit") );
        //exit( -1 );
    }
}

CommandManager::CommandManager(QWidget * parent, const char * name, const QString & arg )
	: QDialog( parent )
{
    EndMessage = "\n\n EXECUTION COMPLETED ! ! ! "; // default
    // Layout
    setFixedSize ( 550, 500 ) ;
    setWindowTitle( name );
    //qt3to4 -- BW
    //output = new Q3TextEdit( this );
    output = new QTextEdit( this );
    output->setText ( arg );
    output->setReadOnly( true );
    output->setGeometry( QRect( 20, 20, 510, 430 ) );

    quitButton = new QPushButton( tr("&Close"), this );
    quitButton->move(390, 465);
    connect( quitButton, SIGNAL(clicked()),
            this, SLOT(close()) );

}


void CommandManager::readFromStdout()
{
    // Read and process the data.
    // Bear in mind that the data might be output in chunks.
       //qt3to4 -- BW
       //output->insert( proc->readStdout(), false, true, true );
       output->insertPlainText( proc->readAllStandardOutput() );
}

void CommandManager::readFromStderr()
{
    // Read and process the data.
    // Bear in mind that the data might be output in chunks.
       //qt3to4 -- BW
       //output->insert( proc->readStderr(), false, true, true );
       output->insertPlainText( proc->readAllStandardError() );
}

void CommandManager::scrollToTop()
{
   //output->setContentsPos( 0, 0 );
    quitButton->setEnabled( true );
    killButton->setEnabled( false );
    //qt3to4 -- BW
    //output->insert( EndMessage, false, true, true );
    output->insertPlainText( EndMessage);
}

void CommandManager::killProcess()
{
    //qt3to4 -- BW
    //proc->tryTerminate();
    proc->terminate();
    QTimer::singleShot( 15000, proc, SLOT( kill() ) );//wait 15 secs
    //qt3to4 -- BW
    //output->insert( "\n\n PROCESS HALTED AT USER'S REQUEST ! ! ! ", false, true, true );
    output->insertPlainText( "\n\n PROCESS HALTED AT USER'S REQUEST ! ! ! ");
    quitButton->setEnabled( true );
}

void CommandManager::setEndMessage(const QString& message ){
    EndMessage = message;
}

