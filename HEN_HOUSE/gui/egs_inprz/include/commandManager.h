/*
###############################################################################
#
#  EGSnrc egs_inprz command manager headers
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


#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include <qobject.h>
//qt3to4 -- BW
//#include <q3process.h>
//#include <q3vbox.h>
//#include <q3textview.h>
#include <qpushbutton.h>
//#include <qapplication.h>
#include <qmessagebox.h>
//#include <qdialog.h>

#include <stdlib.h>

//qt3to4 -- BW
#include <QProcess>
#include <QTextEdit>

//!  Runs a command displaying interactively the output to a window
/*!
This class takes a command
*/
class CommandManager : public QDialog
{
    Q_OBJECT

public:
//qt3to4 -- BW
    CommandManager( QWidget * parent = 0, const char * name = 0, const QStringList & args = QStringList(""));
    CommandManager( QWidget * parent = 0, const char * name = 0, int width = 0, int height = 0, const QStringList & args = QStringList("") );
    CommandManager( QWidget * parent = 0, const char * name = 0, const QString & arg = 0 );
    ~CommandManager() {};
    void setEndMessage(const QString& message );

public slots:
    void readFromStdout();
    void readFromStderr();
    void scrollToTop();
    void killProcess();
private:
    //qt3to4 -- BW
    //Q3Process* proc;
    QProcess* proc;
    //qt3to4 -- BW
    //Q3TextEdit* output;
    QTextEdit* output;
    QPushButton* quitButton;
    QPushButton* killButton;
    QString EndMessage;
};

#endif
