/*
###############################################################################
#
#  EGSnrc egs_inprz combo box tooltip
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


#include "comboboxtooltip.h"
#include <qapplication.h>
#include <qtooltip.h>
#include <qevent.h>
//qt3to4 -- BW
#include <QListView>
#include <QAbstractItemView>

//Added by qt3to4:
//#include <QTextOStream>
#include <QLabel>
//#include <Q3Frame>
//qt3to4 -- BW
//#include <Q3ComboBox>

ComboBoxToolTip::ComboBoxToolTip(QWidget *parent, const char *name, const char* rtips[], const int& r_size  )
        : QWidget( parent )
{

 // tool tip for ListBoxItems inside combo boxes
 // created from scratch since ListBoxItems ain't widgets

//qt3to4 -- BW
//tipLabel = new QLabel( QApplication::desktop(), "toolTipTip",Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::WStyle_Tool | Qt::WX11BypassWM );
tipLabel = new QLabel("toolTipTip",parent,Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint |Qt::Tool | Qt::X11BypassWindowManagerHint );

 tipLabel->setPalette( QToolTip::palette());
 tipLabel->hide();
 hide();

//qt3to4 -- BW
 if ( parent->inherits( "QComboBox" ) ){
//if ( parent->inherits( "Q3ComboBox" ) ){
      //qt3to4 -- BW
      //lb = ((QComboBox*)parent)->listBox();
      //lb = ((Q3ComboBox*)parent)->listBox();
      lb = (QComboBox*)parent;
      eFilter = new MEventFilter( lb , 0);
  }
 else {
      eFilter = new MEventFilter( parent, 0);
  }

connect( eFilter, SIGNAL( mouseMoving() ), this, SLOT( MovingMouseOnBox() ) );
connect( eFilter, SIGNAL( hideEvent() ), tipLabel, SLOT( hide() ) );
//connect( parent, SIGNAL( activated(int) ), this, SLOT( showTipLabel( int) ) );
//qt3to4 -- BW -- this causes a bus error and is actually not a QComboBox signal
//connect( lb, SIGNAL( selected(int) ), this, SLOT( showTipLabel( int) ) );
connect( eFilter, SIGNAL( mouseButRelease() ), tipLabel, SLOT( hide() ) );
//connect( eFilter, SIGNAL( mouseButPress() ), this, SLOT( MovingMouseOnBox()  ) );
tips = rtips;
tipSize = r_size;;

// this gives you the name of the object <= useful for debugging
//cout << "No. of tips for "<< parent->name()<<  " : " << tipSize << endl;

}

ComboBoxToolTip::~ComboBoxToolTip()
{
   zap(eFilter);
   zap(tipLabel);
}

void ComboBoxToolTip::setTips( const char** rtips, const int& r_size )
{
   tips = rtips;
   tipSize = r_size;
}

void ComboBoxToolTip::PressingMouseButton()
{
       tipLabel->hide();
}

void ComboBoxToolTip::MovingMouseOnBox( )
{
   int index = lb->currentIndex();
        index = ( index >= tipSize ) ? tipSize : index;

   QString text;
   QTextStream( &text ) << tips[index];
   ShowHideTip( index, tipLabel, lb, text  );
}

void ComboBoxToolTip::showTipLabel( int index )
{
   index = ( index >= tipSize ) ? tipSize : index;
   QString text;
   QTextStream( &text ) << tips[index];
   ShowHideTip( index, tipLabel, lb, text  );
}

//qt3to4 -- BW
//void ComboBoxToolTip::ShowHideTip( int index, QLabel* tipL, Q3ListBox* lb, const QString& text )
void ComboBoxToolTip::ShowHideTip( int index, QLabel* tipL, QComboBox* lb, const QString& text )
{

 //if ( lb->isVisible() && !lb->isHidden() && lb->hasMouse() )
 if ( lb->isVisible() && !lb->isHidden() )
  {
        //qt3to4 -- BW
        QAbstractItemView* qv = lb->view();
        //qt3to4 -- BW
        //QRect rect = lb->itemRect ( lb->item( index ) );
        QRect rect = qv->visualRect(lb->rootModelIndex());
        tipL->setMargin(1);
        tipL->setIndent(0);
        //qt3to4 -- BW
        //tipL->setAutoMask( FALSE );
        tipL->setFrameStyle( QFrame::Plain | QFrame::Box );
        tipL->setLineWidth( 1 );
        tipL->setAlignment( Qt::AlignLeft | Qt::AlignTop );
        //tipL->polish();
        tipL->setText(text);
        tipL->adjustSize();
        QPoint pos;
   	      pos.setX( rect.right() + 15 );
	      pos.setY( rect.top() );
	      pos = lb->mapToGlobal( pos );
         tipL->move( pos.x(), pos.y() );
         tipL->show();
         tipL->raise();
         return;
  }
  else{
	if (!tipL->isHidden() )
		tipL->hide();
  }

  /*
  if ( !lb->isVisible() || lb->isHidden() ||
       !lb->hasMouse()) {
            tipL->hide();
  }
  */


}
