/*
###############################################################################
#
#  EGSnrc gui element item headers
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
#  Contributors:    Frederic Tessier
#
###############################################################################
*/


#ifndef ELEMENT_ITEM_
#define ELEMENT_ITEM_

#include <qtable.h>
#include <qobject.h>
#include <qcombobox.h>

struct Element {
  int   Z;
  char *symbol;
 float  aw;
 float  Iev;
 float  rho;
};

const int n_element = 100;

class ElementComboBox : public QComboBox {
  Q_OBJECT
public:
  ElementComboBox(QWidget *p = 0, const char *name = 0);
  ~ElementComboBox() { /*qDebug("deleting ElementComboBox at 0x%x",this);*/ }
protected:
  void mousePressEvent ( QMouseEvent * );
};

class ElementItem : public QTableItem {

public:

    ElementItem( QTable *t, EditType et );
    QWidget *createEditor() const;
    void setContentFromEditor( QWidget *w );
    void setText( const QString &s );
    int alignment() const;
    int rtti() const { return 7777; }

private:
    QComboBox *cb;
};

class QTable;
class HelperObject : public QObject {
  Q_OBJECT
public:
  HelperObject(QObject *p=0, const char *name=0);
  void setTable(QTable *t);
public slots:
  void setText(const QString &);
  void deleteRow();
  void deleteSelection();
  void copySelection();
  void pasteSelection();
private:
  QTable *table;
  int    rb, re, cb, ce;
};

class TableEventHandler : public QObject {
  Q_OBJECT
public:
  TableEventHandler(QTable *parent, const char *name=0);
protected:
  bool eventFilter(QObject *o, QEvent *e);
private:
  QTable *table;
};

extern Element element_data[];
extern HelperObject *helper;

#endif
