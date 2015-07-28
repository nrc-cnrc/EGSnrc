/*
###############################################################################
#
#  EGSnrc gui element item
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


#include "element_item.h"

ElementComboBox::ElementComboBox(QWidget *p, const char *name) :
  QComboBox(p,name) {
  insertItem("none");
  for(int j=1; j<=n_element; j++) {
    QString aux = QString("%1  %2 ").arg(j,3).arg(element_data[j-1].symbol,2);
    insertItem(aux);
  }
}

void ElementComboBox::mousePressEvent (QMouseEvent *e) {
  QComboBox::mousePressEvent(e);
}

ElementItem::ElementItem(QTable *t, EditType et ) : QTableItem( t, et, ""),
  cb(0) { setReplaceable( FALSE ); }

QWidget *ElementItem::createEditor() const {
  QComboBox *cbox = new ElementComboBox( table()->viewport() );
  QString s = text().right(2).stripWhiteSpace();
  cbox->setCurrentItem(0);
  for(int j=1; j<=n_element; j++) {
    if( s == element_data[j-1].symbol ) { cbox->setCurrentItem(j); break; }
  }
  ( (ElementItem*)this )->cb = cbox;
  QObject::connect(cbox,SIGNAL(activated(const QString &)),helper,
                   SLOT(setText(const QString &)));
/*
  //cbox->installEventFilter(tfilter);
  cbox->setFocusPolicy(QWidget::StrongFocus);
  //cbox->popup();
  //QListBox *l = cbox->listBox();
  //if( l ) l->raise();
  QRect r = cbox->geometry();
  int xm = (r.rLeft() + r.rRight())/2; int ym = (r.rTop() + r.rBottom())/2;
  QMouseEvent me( QEvent::MouseButtonPress, QPoint(xm,ym),
     Qt::LeftButton, 0 );
  QApplication::sendEvent(cbox,&me);
  QMouseEvent me1( QEvent::MouseButtonRelease, QPoint(xm,ym), Qt::LeftButton, 0
);
  QApplication::sendEvent(cbox,&me1);
  qDebug("ElementItem::createEditor: xm = %d ym = %d ",xm,ym);
*/
  return cbox;
}

void ElementItem::setContentFromEditor( QWidget *w ) {
    // the user changed the value of the combobox, so synchronize the
    // value of the item (its text), with the value of the combobox
    if ( w->inherits( "QComboBox" ) ) {
       QString s = ( (QComboBox*)w )->currentText();
       if( s == "none" ) setText("");
       else setText( s.right(3).stripWhiteSpace() );
#ifdef IK_DEBUG
       qDebug("In ElementItem::setContentFromEditor(), s = %s",s.latin1());
#endif
    }
    else
       QTableItem::setContentFromEditor( w );
    cb = 0;
}

int ElementItem::alignment() const { return Qt::AlignRight; }

void ElementItem::setText( const QString &s ) {
    QString s1 = s; s1.stripWhiteSpace();
#ifdef IK_DEBUG
    qDebug("In ElementItem::setText(%s)",s.latin1());
    qDebug("cb = 0x%x",cb);
#endif
    if ( cb ) {
       // initialize the combobox from the text
       cb->setCurrentItem(0);
#ifdef IK_DEBUG
       qDebug("after setCurrentItem(0)");
#endif
       for(int j=1; j<=n_element; j++) {
         if( s1 == element_data[j-1].symbol ) { cb->setCurrentItem(j); break; }
       }
#ifdef IK_DEBUG
       qDebug("after loop");
#endif
    }
#ifdef IK_DEBUG
    qDebug("Now calling QTableItem::setText()");
#endif
    QTableItem::setText( s1 );
}

HelperObject::HelperObject(QObject *p, const char *name) : QObject(p,name) {}
void HelperObject::setTable(QTable *t) { table = t; }
void HelperObject::setText(const QString &s) {
#ifdef IK_DEBUG
  qDebug("In HelperObject::setText(%s), %d %d",s.latin1(),
    table->currentRow(),table->currentColumn());
#endif
  //QTableItem *item = table->item(table->currentRow(),table->currentColumn());
  //table->setText(table->currentRow(),table->currentColumn(),s);
  //item->setText(s);
  table->setCurrentCell(table->currentRow(),table->currentColumn()+1);
}
void HelperObject::deleteRow() {
#ifdef IK_DEBUG
  qDebug("In HelperObject::deleteRow()");
#endif
}

void HelperObject::deleteSelection() {
#ifdef IK_DEBUG
  qDebug("In HelperObject::deleteSelection()");
#endif
}

void HelperObject::copySelection() {
#ifdef IK_DEBUG
  qDebug("In HelperObject::copySelection()");
#endif
}

void HelperObject::pasteSelection() {
#ifdef IK_DEBUG
  qDebug("In HelperObject::pasteSelection()");
#endif
}

TableEventHandler::TableEventHandler(QTable *parent, const char *name) :
  QObject(parent,name) {
  if( parent == 0 )
   qFatal("TableEventHandler::TableEventHandler: parent can not be null!");
  table = parent;
}

bool TableEventHandler::eventFilter(QObject *o, QEvent *e) {
  if( !o ) qFatal("TableEventHandler::eventFilter called with 0 object?");
  //qDebug("eventFilter: className() = %s",o->className());
  if( o->inherits("QComboBox") ) {
    QComboBox *cbox = (QComboBox *)o;
#ifdef IK_DEBUG
    qDebug("TableEventHandler::eventFilter: QComboBox object %d",e->type());
#endif
    if( e->type() == QEvent::FocusIn ) {
#ifdef IK_DEBUG
      qDebug(" =======> cbox grabs keyboard ");
#endif
      cbox->grabKeyboard(); return true;
    }
    if( e->type() == QEvent::FocusOut ) {
#ifdef IK_DEBUG
      qDebug(" =======> cbox releases keyboard");
#endif
      cbox->releaseKeyboard();
      return true;
    }
    if( e->type() == QEvent::KeyPress ) {
      QKeyEvent *ke = (QKeyEvent*)e;
#ifdef IK_DEBUG
      qDebug("Key event %d",ke->key());
#endif
    }
  }
  if( !o->inherits("QTable") ) {
    //qDebug("object is not a table -> returning"); return false;
    return false;
  }
  QTable *tab = (QTable *)o;
  //qDebug("In TableEventHandler::eventFilter: type = %d",e->type());
  int r = tab->currentRow(), c = tab->currentColumn();
  //qDebug(" r = %d c = %d",r,c);
  //QTableItem *item = tab->item(r,c);
  //if( item ) {
  //  qDebug("rtti: %d",item->rtti());
  //}
  /*
  switch ( e->type() ) {
    case QEvent::KeyPress:
      break;
    case QEvent::MouseButtonPress:
      QMouseEvent *me = (QMouseEvent *)e;
      if( me->button() == Qt::RightButton ) {
        qDebug("right button at %d %d %d %d",me->x(),me->y(),me->globalX(),
          me->globalY());
        return true;
      }
      break;
    default:
      break;
  }
  */
  if( e->type() == QEvent::MouseButtonPress ) {
    QMouseEvent *me = (QMouseEvent *)e;
    if( me->button() == Qt::RightButton ) {
#ifdef IK_DEBUG
      qDebug("right button at %d %d %d %d",me->x(),me->y(),me->globalX(),
          me->globalY());
#endif
      return true;
    }
  }
  else if( e->type() == QEvent::KeyPress ) {
    QKeyEvent *ke = (QKeyEvent*)e;
#ifdef IK_DEBUG
    qDebug("Key event: row %d col %d key %d",r,c,ke->key());
#endif
  }
  return false;
}
