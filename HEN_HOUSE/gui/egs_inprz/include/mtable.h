/*
###############################################################################
#
#  EGSnrc egs_inprz mtable headers
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


#ifndef MTABLE_H
#define MTABLE_H

#include <qtable.h>
//#include "datainp.h"
#include <string>
#include <vector>
using namespace std;
typedef std::vector<string> v_string;

#define zap(x) if(x){delete(x); x=0;}

//!  Extension of the original QTable class.
/*!
This class extends the features of the QTable class offered in the Qt library. It allows
the developer to define the editor for the first column (this can be generalized to any column)
as a LineEdit or ComboBox. The need to have two different editors arises from the way the
PEGS4 media table was designed, i.e., allowing the user to select among the media defined
in the pegs4 data file in use. Furthermore, this extension implements the copy, cut and paste
functions for the table to make table editing easier and more flexible.
*/

class MTable : public QTable
{

    Q_OBJECT
    Q_ENUMS( CellType )
    Q_PROPERTY( CellType celltype READ celltype WRITE setCellType )

public:
    MTable( QWidget *parent = 0, const char *name = 0 );
    MTable( int numRows, int numCols,
    	    QWidget *parent = 0, const char *name = 0 );
    ~MTable();
    void setItemList( v_string il ) { itemList = il; }
    void stopEditing();
    void setValidator(bool val){validate=val;};

    enum CellType { LineEdit, ComboBox };
    void setCellType( CellType ct );
    CellType celltype() const;

protected:
    virtual QWidget *createEditor( int row, int col, bool initFromCell ) const;
               QWidget *createBoxEditor( int row, int col ) const;
    virtual void setCellContentFromEditor( int row, int col );
    virtual void endEdit( int row, int col, bool accept, bool replace );

//               void keyPressEvent( QKeyEvent* e );
             bool eventFilter( QObject *o, QEvent *e );

v_string itemList;
v_string itemCopy;

private:
CellType  ctype;
bool validate;
};
#endif // MTABLE_H
