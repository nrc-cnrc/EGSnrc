/*
###############################################################################
#
#  EGSnrc egs_inprz execution dialog headers
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


#ifndef EXECUTIONDLGIMPL_H
#define EXECUTIONDLGIMPL_H
//qt3to4 -- BW
//#include "executiondialog.h"
#include "ui_executiondialog.h"
#include "eventfilter.h"
//qt3to4 -- BW
#include "commandManager.h"

//qt3to4 -- BW
//#include <q3listbox.h>
#include <qlabel.h>
#include <qcombobox.h>

#define zap(x) if(x){delete(x); x=0;}
//qt3to4 -- BW
//class ExecutiondlgImpl : public MExecutionDialog
class ExecutiondlgImpl : public QDialog, public Ui::MExecutionDialog
{
  Q_OBJECT

public:

ExecutiondlgImpl( QWidget* parent, const char* name,  bool modal, Qt::WindowFlags f );

~ExecutiondlgImpl();

//QLabel* tipLabel;
//MEventFilter* eFilter;

//qt3to4 -- BW
QString the_hen_house;
QString the_command;
void getQueueingSystemOptions();
void init();

public slots:

//virtual void showQueueTipLabel( int index );
//void ShowHideTip( int index, QLabel* tipL, QListBox* lb, const QString& text );
//void MovingMouseOnQueueBox( );

//qt3to4 -- BW
void update_batch();
void run();

};
#endif
