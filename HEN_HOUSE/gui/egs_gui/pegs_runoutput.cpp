/*
###############################################################################
#
#  EGSnrc gui pegs run output
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
#  Contributors:    Blake Walters
#
###############################################################################
*/


#include "pegs_runoutput.h"
#include <QTextEdit>

//#define RO_DEBUG

void PEGS_RunOutput::hideWindow()
{
    hide();
    emit windowClosed();
}

void PEGS_RunOutput::insertText( const QString &s )
{
    run_output->insertPlainText(s);
}

void PEGS_RunOutput::viewErrors()
{
#ifdef RO_DEBUG
    qDebug("In PEGS_RunOutput::viewErrors()");
#endif
//   view_errors->showErrors(ofile);
}

void PEGS_RunOutput::clearOutput() {
    run_output->clear();
#ifdef RO_DEBUG
    qDebug("PEGS_RunOutput::clearOutput()");
#endif
}

void PEGS_RunOutput::setOutputFile( const QString &s )
{
#ifdef RO_DEBUG
    qDebug("PEGS_RunOutput::setOutputFile(%s)",s.toLatin1().data());
#endif
    ofile = s;
}

void PEGS_RunOutput::init()
{
    //view_errors = new PEGS_ViewErrors(0);
}

// void PEGS_ViewErrors::showErrors( const QString &of )
// {
//     ofile = EGS_GUI_Widget::egsHome();
//     ofile += "pegs4/data/inputs/"; ofile += of;
//     ofile += ".pegs4err";
//     qDebug("Reading from file %s",ofile.latin1());
//     errors->clear();
//     setCaption(ofile);
//     QFile f(ofile);
//     if( f.exists() ) {
//         f.open(IO_ReadOnly);
//         QTextStream ts(&f); QString tmp; ts >> tmp;
//         errors->insert(tmp);
//     }
//     show();
// }
