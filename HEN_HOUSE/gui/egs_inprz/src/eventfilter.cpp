/*
###############################################################################
#
#  EGSnrc egs_inprz event filter
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
#                   Cody Crewson
#
###############################################################################
*/


#include "eventfilter.h"
#include <qevent.h>
//#include <q3listbox.h>
//#include <q3table.h>
#include <qcombobox.h>
//Added by qt3to4:
#include <QKeyEvent>

//qt3to4 -- BW
#include <QTableWidget>

MEventFilter::MEventFilter( QWidget *parent, const char *name )
        : QWidget( parent )
{
        // install a filter on the parent (if any)
       //qt3to4 -- BW
       //if ( parent->inherits( "QListBox" ) )
        if ( parent->inherits( "QComboBox" ) )
       //if ( parent )
            parent->installEventFilter( this );
       else
//qt3to4 -- BW
            std::cout << "event filter only for QComboBox, aborting ..." << std::endl;
        hide();
        counter = 0;
}

bool MEventFilter::eventFilter( QObject *o, QEvent *e )
{
    if ( !o || !e )
	return false;

    switch ( e->type() ) {
    case QEvent::KeyPress:
	break;
    case QEvent::Hide:
	 emit hideEvent();
	break;
    case QEvent::KeyRelease:
    case QEvent::MouseMove:
	 emit mouseMoving();
              break;
    //qt3to4 -- BW
    case QEvent::MouseButtonRelease:
         emit mouseButRelease();
              break;
    case QEvent::MouseButtonPress:
         emit mouseButPress();
              break;
    default:
              break;
    }
    return false;
}

TableEvents::TableEvents( QWidget *parent, const char *name )
        : QWidget( parent )
{
       if(parent->inherits("QTableWidget"))
         parent->installEventFilter( this );
       else
//qt3to4 -- BW
         std::cout << "TableEvents filter only for QTables, aborting ..." << std::endl;
       hide();
}

bool TableEvents::eventFilter( QObject *o, QEvent *e)
{
    //qt3to4 -- BW
    //Q3Table *to = (Q3Table *)o;
    QTableWidget *to = (QTableWidget *)o;
    if( e->type() == QEvent::KeyPress) {
       QKeyEvent *ke = (QKeyEvent*)e;
       //if the user it at the last row and hits return, add a row
       if ( ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter ) {
         //qt3to4 -- BW
         //if( to->currentRow() >= to->numRows() - 1 ){
         if( to->currentRow() >= to->rowCount() - 1 ){
            //to->setNumRows( to->numRows() + 1 );
            to->setRowCount(to->rowCount() + 1 );
            to->setCurrentCell(to->rowCount()-1,to->currentColumn());
         }
       }

//qt3to4 -- BW
//       if ( ke->key() == Qt::Key_Delete || ( ke->key() == Qt::Key_X && ( ke->state() & Qt::ControlModifier ) == Qt::ControlModifier ) ) {
         if ( ke->key() == Qt::Key_Delete || ( ke->key() == Qt::Key_X && ke->modifiers() == Qt::ControlModifier ) ) {
         //qt3to4 -- BW
         QList<QTableWidgetSelectionRange> ts = to->selectedRanges();
         //if (to->numSelections() > 0) {
         if(!ts.isEmpty()) {
           //qt3to4 -- BW
           //Q3TableSelection ts = to->selection( to->currentSelection() );
           //go through rows, if entire row is selected delete the actual row
           //qt3to4 -- BW
           //int nrows = to->numRows();
           int nrows = to->rowCount();
           int nrdel = 0;
           std::vector<int> rowdel;
           //qt3to4 -- BW
           //only deal with one selected range
           //for (int irow = ts.topRow(); irow <= ts.bottomRow(); irow++) {
              //if(ts.leftCol()==0 && ts.rightCol() == to->numCols()-1 && nrows-nrdel>1) {

           /* logic below was to actually delete rows from table--works but disabled because row
              numbers were not updated, resulting in a slightly confusing table
           for (int irow = ts.first().topRow(); irow <= ts.first().bottomRow(); irow++) {
              if(ts.first().leftColumn()==0 && ts.first().rightColumn() == to->columnCount()-1 && nrows-nrdel>1) {
                rowdel.push_back(irow);
                nrdel++;
              }
              //qt3to4 -- BW
              //for (int icol = ts.leftCol(); icol <= ts.rightCol(); icol++){
              //
              //for (int icol = ts.first().leftColumn(); icol <= ts.first().rightColumn(); icol++){
                    //to->clearCell(irow,icol);
                    //QWidget *w = to->cellWidget(irow,icol);
               //     QTableWidgetItem *w = to->item(irow,icol);
                    //if(w) to->clearCellWidget(irow,icol);
                //    if(w) to->removeCellWidget(irow,icol);
              //}
           //}
         }

         //now delete rows
         for (int irow = rowdel.size()-1; irow >= 0; irow--) to->removeRow(rowdel[irow]);
         to->reset();
         */

         //delete only contents of cells selected
         for (int irow = ts.first().topRow(); irow <= ts.first().bottomRow(); irow++) {
            for(int icol = ts.first().leftColumn(); icol <= ts.first().rightColumn(); icol++) {
               to->setItem(irow,icol,0);
            }
         }

        }
       }
//qt3to4 -- BW
//       if ( ke->key() == Qt::Key_C && ( ke->state() & Qt::ControlModifier ) == Qt::ControlModifier ) {
       if ( ke->key() == Qt::Key_C && ke->modifiers() == Qt::ControlModifier ) {
             QString cellText;
             itemCopy.clear();
             //qt3to4 -- BW
             QList<QTableWidgetSelectionRange> ts = to->selectedRanges();
             if(!ts.isEmpty()) {
             //if(to->numSelections() > 0) {
                 //qt3to4 -- BW
                 //Q3TableSelection ts = to->selection( to->currentSelection() );
                 //for ( int irow = ts.topRow(); irow <= ts.bottomRow(); irow++){
                      //for ( int icol = ts.leftCol(); icol <= ts.rightCol(); icol++){
                 for ( int irow = ts.first().topRow(); irow <= ts.first().bottomRow(); irow++){
                      for ( int icol = ts.first().leftColumn(); icol <= ts.first().rightColumn(); icol++){
                             //qt3to4 -- BW
                             /*
                             cellText = to->text( irow, icol );
			     QWidget *w = to->cellWidget(irow,icol);
                             if(w) {
                               if(w->isA ("QComboBox")){
                                 QComboBox *e = (QComboBox *)w;
                                 //have to do this because text on face of
                                 //combobox is not read properly with text(i,j)
                                 cellText = e->currentText();
                               }
                             }
                             */
                             QString cellText;
                             QTableWidgetItem *w = to->item(irow,icol);
                             if(w) cellText = w->text();
                             if ( !cellText.isEmpty() ){
                                   itemCopy.push_back( cellText.toStdString() );
                             }
                             else
                                   itemCopy.push_back( "" );
                      }
                 }
              }
              else {
                  //qt3to4 -- BW
                  //cellText = to->text( to->currentRow(), to->currentColumn() );
                  QTableWidgetItem *w = to->item(to->currentRow(), to->currentColumn());
                  if (w) cellText = w->text();
                  if ( !cellText.isEmpty() )
                               itemCopy.push_back( cellText.toStdString() );
                  else      itemCopy.push_back( "" );
              }
            }

//qt3to4 -- BW
           // if ( ke->key() == Qt::Key_V && ( ke->state() & Qt::ControlModifier ) == Qt::ControlModifier ) {
            if ( ke->key() == Qt::Key_V && ke->modifiers() == Qt::ControlModifier ) {
              //qt3to4 -- BW
              QList<QTableWidgetSelectionRange> ts = to->selectedRanges();
              //if ( to->numSelections() > 0 && itemCopy.size() > 0 ) {
              if ( !ts.isEmpty() > 0 && itemCopy.size() > 0 ) {
                  //Q3TableSelection ts = to->selection( to->currentSelection() );
                  uint icount;
                  //qt3to4 -- BW
                  //for ( int irow = ts.topRow(); irow <= ts.bottomRow(); irow++){
                       //for ( int icol = ts.leftCol(); icol <= ts.rightCol(); icol++){
                  for ( int irow = ts.first().topRow(); irow <= ts.first().bottomRow(); irow++){
                       for ( int icol = ts.first().leftColumn(); icol <= ts.first().rightColumn(); icol++){
                          //icount = (irow - ts.topRow())*(ts.rightCol() - ts.leftCol()+1) + icol-ts.leftCol();
                          icount = (irow - ts.first().topRow())*(ts.first().rightColumn() - ts.first().leftColumn()+1) + icol-ts.first().leftColumn();
                                         //if ( icount < itemCopy.size() )
                                         if ( icount < itemCopy.size() ) {
                                            //to->setText( irow, icol, (itemCopy[icount]).c_str() );
                                            to->setItem(irow, icol, new QTableWidgetItem((itemCopy[icount]).c_str()));
                                         }
                      }
                   }
            }
            else {
                   //qt3to4 -- BW
                    if ( itemCopy.size() > 0 ) // there was not selection, copy first item only
                         //to->setText( to->currentRow(), to->currentColumn(), (itemCopy[0]).c_str() );
                         to->setItem(to->currentRow(), to->currentColumn(), new QTableWidgetItem((itemCopy[0]).c_str()));
             }
         }
    }
    return false;
}

ComboEvents::ComboEvents( QWidget *parent, const char *name )
        : QWidget( parent )
{
       if(parent->inherits("QComboBox")) {
         QComboBox* cb = (QComboBox*)parent;
         if(cb->isEditable())
           parent->installEventFilter( this );
         else
           std::cout << "ComboEvents filter only for editable ComboBoxes, aborting ..." << std::endl;
       }
       else
         std::cout << "ComboEvents filter only for ComboBoxes, aborting ..." << std::endl;
       hide();
}

bool ComboEvents::eventFilter( QObject *o, QEvent *e )
{
    if ( !o || !e )
        return false;

    switch ( e->type() ) {
    case QEvent::KeyPress:
    {
        QKeyEvent *ke = (QKeyEvent*)e;
        QComboBox *cb = (QComboBox *)o;
        if ( ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter ) emit keyReturn(cb->currentText());
        break;
    }
    default:
    {
        break;
    }
    }
    return false;
}
