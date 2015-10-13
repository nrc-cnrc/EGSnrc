/*
###############################################################################
#
#  EGSnrc gui pegs run output headers
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


#ifndef PEGS_RUNOUTPUT_H
#define PEGS_RUNOUTPUT_H

#include "ui_pegs_runoutput.h"
//#include "ui_pegs_viewerrors.h"
#include <qdialog.h>

class QTextEdit;

// class PEGS_ViewErrors :  public QDialog, public Ui::PEGS_ViewErrors
// {
//     Q_OBJECT
// public:
//     PEGS_ViewErrors(QWidget* parent): QDialog(parent){setupUi(this);};
//     ~PEGS_ViewErrors(){}
//
//     QTextEdit* errors;
//
// public slots:
//     void showErrors( const QString & of );
//
// private:
//     QString ofile;
// };

class PEGS_RunOutput :  public QDialog, public Ui::PEGS_RunOutput
{
    Q_OBJECT
public:
    PEGS_RunOutput(QWidget* parent): QDialog(parent){setupUi(this);init();};
    ~PEGS_RunOutput(){}

public slots:
    void hideWindow();
    void insertText( const QString & s );
    void viewErrors();
    void clearOutput();
    void setOutputFile( const QString & s );

signals:
    void windowClosed();

protected slots:
    void init();

private:
    QString ofile;
    //PEGS_ViewErrors *view_errors;

};
#endif // PEGS_RUNOUTPUT_H