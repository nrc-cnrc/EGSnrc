/*
###############################################################################
#
#  Qt richtext headers
#  Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
#
#  This file is part of an example program for Qt. This example
#  program may be used, distributed and modified without limitation.
#
###############################################################################
#
#  Author:          Trolltech, 1992
#
#  Contributors:    Ernesto Mainegra-Hing
#                   Blake Walters
#
###############################################################################
*/


#ifndef RICHTEXT_H
#define RICHTEXT_H

//qt3to4 -- BW
//#include <q3vbox.h>
#include <QFrame>

//class Q3TextView;
class QPushButton;
class QTextEdit;

//qt3to4 -- BW
//class MyRichText : public Q3VBox
class MyRichText : public QWidget
{
    Q_OBJECT

public:
    MyRichText( QWidget *parent = 0, const char *name = 0 );
    ~MyRichText();

protected:
    //qt3to4 -- BW
    //Q3TextView *view;
    QTextEdit *view;
    QPushButton *bClose; //, *bNext, *bPrev;
    //int num;

//protected slots:
//    void prev();
//    void next();

};

#endif
