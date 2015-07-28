/*
###############################################################################
#
#  EGSnrc egs_inprz combo box tooltip headers
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
#
###############################################################################
*/


#ifndef COMBOBOXTOOLTIP_H
#define COMBOBOXTOOLTIP_H
#include "datainp.h"
#include "eventfilter.h"

//qt3to4 -- BW
//#include <q3listbox.h>
#include <qlabel.h>
#include <qcombobox.h>

#define zap(x) if(x){delete(x); x=0;}

/*!
 // This class implements a tool tip for ListBoxItems inside combo boxes
 // created from scratch since ListBoxItems ain't widgets
*/
class ComboBoxToolTip : public QWidget
{
  Q_OBJECT

public:

  ComboBoxToolTip(QWidget *parent, const char *name, const char* rtips[], const int& r_size );
 ~ComboBoxToolTip();

  QLabel*       tipLabel;
  MEventFilter* eFilter;
  //qt3to4 -- BW
  //QListBox*     lb;
  QComboBox*    lb;
  const char**  tips;
  uint          tipSize;

public slots:

        void setTips( const char** rtips, const int& r_size );
virtual void showTipLabel( int index );
        //qt3to4 -- BW
        //void ShowHideTip( int index, QLabel* tipL, Q3ListBox* lb, const QString& text );
        void ShowHideTip( int index, QLabel* tipL, QComboBox* lb, const QString& text );
        void MovingMouseOnBox( );
	void PressingMouseButton();
};
#endif

