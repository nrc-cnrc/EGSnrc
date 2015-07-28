/*
###############################################################################
#
#  EGSnrc user interface headers for egs_gui run output
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
#  Author:          Iwan Kawrakow, 2003
#
#  Contributors:
#
###############################################################################
*/


/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/


void PEGS_RunOutput::hideWindow()
{
  hide(); emit windowClosed();
}

void PEGS_RunOutput::insertText( const QString &s )
{
  run_output->insert(s);
}

void PEGS_RunOutput::viewErrors()
{
#ifdef IK_DEBUG
  qDebug("In PEGS_RunOutput::viewErrors()");
#endif
  view_errors->showErrors(ofile);
}

void PEGS_RunOutput::clearOutput() {
  run_output->clear();
}


void PEGS_RunOutput::setOutputFile( const QString &s )
{
#ifdef IK_DEBUG
  qDebug("PEGS_RunOutput::setOutputFile(%s)",s.latin1());
#endif
  ofile = s;
}


void PEGS_RunOutput::init()
{
  view_errors = new PEGS_ViewErrors(0,"Error view");
}
