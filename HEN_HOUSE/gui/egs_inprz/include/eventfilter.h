/*
###############################################################################
#
#  EGSnrc egs_inprz event filter headers
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
#
###############################################################################
*/


#ifndef MEVENTFILTER_H
#define MEVENTFILTER_H

#include <qwidget.h>
//Added by qt3to4:
#include <QEvent>
#include <iostream>
#include <string>
#include <vector>

using std::string;
class MEventFilter : public QWidget
{
    Q_OBJECT

public:
    MEventFilter( QWidget *parent = 0, const char *name = 0 );

    uint counter;
signals:
        void mouseMoving();
        void hideEvent();
        //qt3to4 -- BW
        void mouseButRelease();
        void mouseButPress();
//        void mouseButtonPressed();

protected:
        bool eventFilter ( QObject * o, QEvent * e );
};

class TableEvents : public QWidget
{
       Q_OBJECT
public:
       TableEvents(QWidget *parent=0,const char *name = 0 );
protected:
       bool eventFilter ( QObject * o, QEvent * e );
       std::vector<string> itemCopy;
};

class ComboEvents : public QWidget
{
       Q_OBJECT
public:
       ComboEvents( QWidget *parent = 0, const char *name = 0 );
signals:
       void keyReturn(const QString);
protected:
       bool eventFilter ( QObject * o, QEvent * e );
};
#endif
