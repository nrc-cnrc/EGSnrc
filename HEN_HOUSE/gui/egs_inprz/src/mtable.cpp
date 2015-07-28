/*
###############################################################################
#
#  EGSnrc egs_inprz mtable
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
#  Author:          Ernesto Mainegra-Hing, 2003
#
#  Contributors:
#
###############################################################################
*/


#include <qcombobox.h>
#include <qhbox.h>
#include <qvbox.h>
//#include "datainp.h"
#include "../include/mtable.h"
//#ifndef QT_NO_TABLE
#include <qpainter.h>
#include <qkeycode.h>
#include <qlineedit.h>
#include <qapplication.h>
#include <qtimer.h>
#include <qobjectlist.h>
#include <stdlib.h>
#include <limits.h>
#include <qvalidator.h>
MTable::MTable( QWidget *parent, const char *name )
    : validate(true), QTable( parent, name )
{
    itemList.push_back("nothing yet!");
    installEventFilter( this );
}

MTable::MTable( int numRows, int numCols, QWidget *parent, const char *name )
    : validate(true), QTable( numRows, numCols, parent, name )
{
    itemList.push_back("nothing yet!");
    installEventFilter( this );
}

MTable::~MTable()
{
    itemList.clear();
}

MTable::CellType MTable::celltype() const
{
    return ctype;
}

void MTable::setCellType( CellType ct )
{
    ctype = ct;
}

QWidget *MTable::createBoxEditor( int row, int col ) const
{
    QWidget *e = 0;
    if ( col == 0 && ctype == ComboBox){

          e = new QComboBox( true, viewport(), "qt_editor_cb" );
          //e = new QComboBox( true, viewport(), "e" );
          ( ( QComboBox*)e )->setEditable( false );
          ( ( QComboBox*)e )->setDuplicatesEnabled ( false );

          QString strNoMed = "VACUUM";
          ( ( QComboBox*)e )->insertItem( strNoMed );

          QString str;
          for ( uint i = 0; i < itemList.size(); i++) {
            str = itemList[i].c_str();
            ( ( QComboBox*)e )->insertItem( str );
          }

           // only for non-editable combo boxes
          QString cellText = text( row, col );
          if ( !cellText.isEmpty() ){
             //Match of cellText with one combo box item resets current item
             ( ( QComboBox*)e )->setCurrentText( cellText );
              //this applies if no match
              if ( ( ( QComboBox*)e )->currentItem() == 0 )
	         ( ( QComboBox*)e )->insertItem( strNoMed, 0 );
          }

           // only for editable combo boxes
          //( ( QComboBox*)e )->setEditText( text( row, col ) );
          //( ( QComboBox*)e )->lineEdit()->setText( text( row, col ) );

     } else {
          //e = new QLineEdit( viewport() );
          e = new QLineEdit( viewport(), "qt_lineeditor" );
          if (validate)
           ((QLineEdit*)e)->setValidator(new QDoubleValidator(e));

          ( (QLineEdit*)e )->setFrame( FALSE );
          ( (QLineEdit*)e )->setText( text( row, col ) );
     }

    e->installEventFilter( this );
    return e;
}

QWidget *MTable::createEditor( int row, int col, bool initFromCell ) const
{
    QTableItem *i = item( row, col );
    if ( initFromCell && i && !i->isReplaceable() ){
        return QTable::createEditor( row, col, initFromCell );
    }
    else{	// this is happening all the time
        return createBoxEditor( row, col );
    }
}

void MTable::setCellContentFromEditor( int row, int col )
{
    QWidget *editor = cellWidget( row, col );
    if ( !editor ) return;
    clearCell( row, col );
    if ( editor->inherits( "QComboBox" ) ){
	  QString str = ((QComboBox*)editor )->currentText();
                //if ( str == "no medium" ) str = "";
	  setText( row, col, str );
    }
    else if ( editor->inherits( "QLineEdit" ) )
	   setText( row, col, ( (QLineEdit*)editor )->text() );
}

void MTable::stopEditing()
{
     int cCol = currentColumn();
     int cRow = currentRow();

     endEdit( cRow, cCol, TRUE, TRUE );
}

void MTable::endEdit( int row, int col, bool accept, bool replace )
{
    QWidget *editor = cellWidget( row, col );
    if ( !editor ) return;
    if (!editor->inherits("QComboBox")){
         QTable::endEdit( row, col, accept, replace );
         zap(editor);
         //if (editor) delete editor;// <== this seems neccessary to return control to the Table
         return;
    }
    else{
             setCellContentFromEditor( row, col );
              if ( row == currEditRow() && col == currEditCol() )
	     setEditMode( NotEditing, -1, -1 );
              viewport()->setFocus();
              updateCell( row, col );
              clearCellWidget( row, col );
              zap (editor);
              //if (editor) delete editor;// <== this seems neccessary to return control to the Table
               emit valueChanged( row, col );
    }
}


//     My own version of the table's actions
//    In principle it does the same as QTable, but
//       deletes selected cells
//      closes application on ESCAPE
bool MTable::eventFilter( QObject *o, QEvent *e )
{
    if ( !o || !e )
	return QScrollView::eventFilter( o, e );

    //QWidget *editorWidget = cellWidget( currentRow(), currentColumn() );

    switch ( e->type() ) {
    case QEvent::KeyPress:
	if ( !isEditing() ) {
                  QKeyEvent *ke = (QKeyEvent*)e;
	    if ( ke->key() == Key_Escape ) {
                           QApplication::sendEvent( parentWidget ( FALSE ) , e );
		return TRUE;
	    }

	    if ( ke->key() == Key_Return || ke->key() == Key_Enter ) {
		if ( currentRow() >= numRows() - 1 ){
		    setUpdatesEnabled( false );
		    setNumRows( numRows() + 10 );
		    setUpdatesEnabled( true );
		}
	        activateNextCell();
                ensureCellVisible ( currentRow(), currentColumn() );
		return TRUE;
	    }

	    if ( ke->key() == Key_Delete ) {
	            if (numSelections() > 0) {
		    QTableSelection ts = selection( currentSelection() );
		    for ( int icol = ts.leftCol(); icol <= ts.rightCol(); icol++){
			for ( int irow = ts.topRow(); irow <= ts.bottomRow(); irow++){
                                         clearCell( irow, icol );
		      }
		    }
		    setCurrentCell ( ts.anchorRow(), ts.anchorCol() );
		    clearSelection ( TRUE );
		}
	              else {
                                 clearCell( currentRow(), currentColumn() );
	              }
		return TRUE;
	    }
	    if ( ke->key() == Key_C && ( ke->state() & ControlButton ) == ControlButton ) {
	              QString cellText;
		 itemCopy.clear();
		if (numSelections() > 0) {
	                  QTableSelection ts;
		    ts = selection( currentSelection() );
		    for ( int irow = ts.topRow(); irow <= ts.bottomRow(); irow++){
		      for ( int icol = ts.leftCol(); icol <= ts.rightCol(); icol++){
		             cellText = text( irow, icol );
                                         if ( !cellText.isEmpty() )
                                              itemCopy.push_back( cellText.latin1() );
		             else
                                              itemCopy.push_back( "" );
		      }
		    }
		}
		else {
	                   cellText = text( currentRow(), currentColumn() );
                                 if ( !cellText.isEmpty() )
                                              itemCopy.push_back( cellText.latin1() );
                                 else      itemCopy.push_back( "" );
		}
		return TRUE;
	    }
	    if ( ke->key() == Key_V && ( ke->state() & ControlButton ) == ControlButton ) {
		if ( numSelections() > 0 && itemCopy.size() > 0 ) {
		    QTableSelection ts = selection( currentSelection() );
		    uint icount;
		    for ( int irow = ts.topRow(); irow <= ts.bottomRow(); irow++){
		       for ( int icol = ts.leftCol(); icol <= ts.rightCol(); icol++){
		             //icount = (icol - ts.leftCol())*(ts.bottomRow() - ts.topRow()+1) + irow-ts.topRow();
		             icount = (irow - ts.topRow())*(ts.rightCol() - ts.leftCol()+1) + icol-ts.leftCol();
                                         if ( icount < itemCopy.size() )
                                            setText( irow, icol, (itemCopy[icount]).c_str() );
		      }
		    }
		}
		else {
		    if ( itemCopy.size() > 0 ) // there was not selection, copy first item only
		         setText( currentRow(), currentColumn(), (itemCopy[0]).c_str() );
		}
		return TRUE;
	    }
	    if ( ke->key() == Key_X && ( ke->state() & ControlButton ) == ControlButton ) {
		QString cellText;
		if (numSelections() > 0) {
		    itemCopy.clear();
		   QTableSelection ts;
		    ts = selection( currentSelection() );
		    for ( int irow = ts.topRow(); irow <= ts.bottomRow(); irow++){
		      for ( int icol = ts.leftCol(); icol <= ts.rightCol(); icol++){
		             cellText = text( irow, icol );
                                         if ( !cellText.isEmpty() )
                                              itemCopy.push_back( cellText.latin1() );
		             else
                                              itemCopy.push_back( "" );
                                         clearCell( irow, icol );
		      }
		    }
		    setCurrentCell ( ts.anchorRow(), ts.anchorCol() );
		    clearSelection ( TRUE );
		}
		else {
	                   cellText = text( currentRow(), currentColumn() );
                                 if ( !cellText.isEmpty() )
                                              itemCopy.push_back( cellText.latin1() );
                                 else      itemCopy.push_back( "" );
                                clearCell( currentRow(), currentColumn() );
		}
		return TRUE;
	    }

	    if ( currentColumn() == 0 &&
	         ctype == ComboBox   &&
                       ke->key() != Key_Left  && ke->key() != Key_Right &&
                       ke->key() != Key_Up  && ke->key() != Key_Down &&
                       ke->key() != Key_Control && ke->key() != Key_Alt &&
                       ke->key() != Key_Shift ) {
		//QApplication::beep ();
		keyPressEvent( (QKeyEvent*)e );
		return true;
	    }
	}
        else{
            QKeyEvent *ke = (QKeyEvent*)e;
	    if ( ke->key() == Key_Return || ke->key() == Key_Enter ) {
                stopEditing();
		if ( currentRow() >= numRows() - 1 ){
		    setUpdatesEnabled( false );
		    setNumRows( numRows() + 10 );
		    setUpdatesEnabled( true );
		}
                //else {stopEditing();}
                activateNextCell();
		return true;
	    }
        }
        break;
    default:
	break;
    }

    return QTable::eventFilter( o, e ) ;
}


/*
void MTable::keyPressEvent( QKeyEvent* e )
{

}
*/

//#endif // QT_NO_TABLE
