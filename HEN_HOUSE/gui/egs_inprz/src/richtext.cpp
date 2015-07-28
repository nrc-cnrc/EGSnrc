/*
###############################################################################
#
#  Qt richtext
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


#include "datainp.h"
#include "richtext.h"

//#include <q3hbox.h>
//#include <q3hbox.h>
#include <qpushbutton.h>
//#include <q3textview.h>
#include <qbrush.h>
#include <qapplication.h>
//qt3to4 -- BW
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

//qt3to4 -- BW
MyRichText::MyRichText( QWidget *parent, const char *name )
//    : Q3VBox( parent, name )
      : QWidget(parent)
{
    //setMargin( 5 );

    //qt3to4 -- BW
    //view = new Q3TextView( this );
    view = new QTextEdit(this);
    view->setText( name );
    view->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
    view->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
    //Q3HBox *buttons = new Q3HBox( this );
    QWidget* bbox = new QWidget;
    QHBoxLayout *buttons = new QHBoxLayout( bbox );
    //buttons->setMargin( 5 );
    bClose = new QPushButton( "&Close" );
    buttons->addWidget(bClose);

    connect( bClose, SIGNAL( clicked() ), parent, SLOT( close() ) );

    //qt3to4 -- BW
    //now arrange things vertically
    QVBoxLayout* vbox = new QVBoxLayout(this);
    vbox->addWidget(view);
    vbox->addWidget(bbox);
}

MyRichText::~MyRichText()
{
   zap(view);
}
