/*
###############################################################################
#
#  EGSnrc egs_inprz beam source dialog
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
#  Author:          Ernesto Mainegra-Hing, 2006
#
#  Contributors:    Blake Walters
#
###############################################################################
*/


#include "beamsrcdlg.h"
#include "tools.h"
#include <qcombobox.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qlayout.h>
//#include <q3buttongroup.h>
#include <qvalidator.h>
//Added by qt3to4:
//#include <Q3HBoxLayout>
//#include <Q3VBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QLayout>

BeamSourceDlg::BeamSourceDlg(QWidget * parent, const char * name,
                             QStringList* beamuc,
                             QStringList* beaminp,
                             QStringList* pegsdat)
              : QDialog( parent )
{

        QStringList::Iterator it = beamuc->begin();
        EGS_HOME = *it;
        //qt3to4 -- BW
        //beamuc->remove(it);
        beamuc->erase(it);
        it = pegsdat->begin();
        //don't need to do this
        //pegsDir = *it;
        //pegsdat->remove(it);
        //pegsdat->erase(it);

        setGeometry(parent->pos().x()+parent->width()/4,
                    parent->pos().y()+parent->height()/4,
                    parent->width()/2, parent->height()/1.5);


        //qt3to4 -- BW
        //Q3VBoxLayout* topl = new Q3VBoxLayout(this);
         QVBoxLayout* topl = new QVBoxLayout(this);
        topl->setSpacing(6); topl->setMargin(11);

// BEAM user code
        //qt3to4 -- BW
        //Q3GroupBox* beamGroupBox = new Q3GroupBox( this, "beamGroupBox" );
        QGroupBox* beamGroupBox = new QGroupBox( "beamGroupBox", this );
        //beamGroupBox->setColumnLayout(0, Qt::Horizontal );
        //beamGroupBox->layout()->setSpacing( 6 );
        //beamGroupBox->layout()->setMargin( 11 );
        beamGroupBox->setTitle( "BEAM user code (library build only)" );
        //Q3HBoxLayout* beamGroupBoxLayout = new Q3HBoxLayout(
        QHBoxLayout* beamGroupBoxLayout = new QHBoxLayout(
                                  beamGroupBox );
        beamGroupBoxLayout->setAlignment( Qt::AlignVCenter );

        //Q3HBoxLayout* l1 = new Q3HBoxLayout;
        QHBoxLayout* l1 = new QHBoxLayout;
        l1->setSpacing(6); l1->setMargin(0);
        beam = new QComboBox(beamGroupBox);
        beam->setSizePolicy( QSizePolicy( (QSizePolicy::Policy)1,
                                    (QSizePolicy::Policy)0));
        beam->sizePolicy().setHorizontalStretch(232);
        beam->sizePolicy().setVerticalStretch(0);
        beam->sizePolicy().hasHeightForWidth();
        beam->addItems(*beamuc);

        l1->addWidget(beam);

        beamGroupBoxLayout->addLayout( l1 );

        topl->addWidget(beamGroupBox);
// BEAM input file
        //Q3GroupBox* inpGroupBox = new Q3GroupBox( this, "inpGroupBox" );
        QGroupBox* inpGroupBox = new QGroupBox( "inpGroupBox",this );
        //inpGroupBox->setColumnLayout(0, Qt::Vertical );
        //inpGroupBox->layout()->setSpacing( 6 );
        //inpGroupBox->layout()->setMargin( 11 );
        inpGroupBox->setTitle( tr("BEAM input file") );
        QHBoxLayout* inpGroupBoxLayout = new QHBoxLayout(
                                  inpGroupBox );
        inpGroupBoxLayout->setAlignment( Qt::AlignVCenter );

        QHBoxLayout* l2 = new QHBoxLayout;
        l2->setSpacing(6); l2->setMargin(0);
        inp = new QComboBox(inpGroupBox);
        inp->setSizePolicy( QSizePolicy( (QSizePolicy::Policy)1,
                                    (QSizePolicy::Policy)0));
        inp->sizePolicy().setHorizontalStretch(232);
        inp->sizePolicy().setVerticalStretch(0);
        inp->sizePolicy().hasHeightForWidth();
        inp->addItems(*beaminp);
        l2->addWidget(inp);
        inpGroupBoxLayout->addLayout( l2 );
        topl->addWidget(inpGroupBox);

// BEAM pegs4 data file
        QGroupBox* pegsGroupBox = new QGroupBox( "pegsGroupBox" , this);
        //pegsGroupBox->setColumnLayout(0, Qt::Vertical );
        //pegsGroupBox->layout()->setSpacing( 6 );
        //pegsGroupBox->layout()->setMargin( 11 );
        pegsGroupBox->setTitle( tr("BEAM pegs4 data file") );
        QHBoxLayout* pegsGroupBoxLayout = new QHBoxLayout(
                                  pegsGroupBox );
        pegsGroupBoxLayout->setAlignment( Qt::AlignVCenter );

        QHBoxLayout* l3 = new QHBoxLayout;
        l3->setSpacing(6); l3->setMargin(0);
        pegs = new QComboBox(pegsGroupBox);
        pegs->setSizePolicy( QSizePolicy( (QSizePolicy::Policy)1,
                                    (QSizePolicy::Policy)0));
        pegs->sizePolicy().setHorizontalStretch(232);
        pegs->sizePolicy().setVerticalStretch(0);
        pegs->sizePolicy().hasHeightForWidth();
        pegs->addItems(*pegsdat);
        pegs->addItem("pegsless");
        l3->addWidget(pegs);
        pegsGroupBoxLayout->addLayout( l3 );
        topl->addWidget(pegsGroupBox);

// BEAM weight window
        QGroupBox* weightGroupBox = new QGroupBox( "weightGroupBox", this );
        //weightGroupBox->setColumnLayout(0, Qt::Vertical );
        //weightGroupBox->layout()->setSpacing( 6 );
        //weightGroupBox->layout()->setMargin( 11 );
        weightGroupBox->setTitle( tr("Weight window for BEAM particles") );
        QHBoxLayout* weightGroupBoxLayout = new QHBoxLayout(
                                  weightGroupBox );
        weightGroupBoxLayout->setAlignment( Qt::AlignVCenter );
        QHBoxLayout* l4 = new QHBoxLayout;
        l4->setSpacing(6); l4->setMargin(0);

        minWeight = new QLineEdit("-1E30",weightGroupBox);
        maxWeight = new QLineEdit("1E30",weightGroupBox);
        minWeight->setAlignment( Qt::AlignRight);
        maxWeight->setAlignment( Qt::AlignRight);
        minWeight->setValidator( new QDoubleValidator(minWeight) );
        maxWeight->setValidator( new QDoubleValidator(maxWeight) );
        QLabel* min = new QLabel("min",weightGroupBox);
        QLabel* max = new QLabel("max",weightGroupBox);
        QSpacerItem* hSpacerW = new QSpacerItem(20,20,QSizePolicy::Expanding,
                                          QSizePolicy::Minimum);
        l4->addWidget(min);l4->addWidget(minWeight);
        l4->addItem(hSpacerW);
        l4->addWidget(max);l4->addWidget(maxWeight);
        weightGroupBoxLayout->addLayout( l4 );
        topl->addWidget(weightGroupBox);

// BEAM source options
        QGroupBox* optionsGroupBox = new QGroupBox( "optionsGroupBox", this );
        //optionsGroupBox->setColumnLayout(0, Qt::Vertical );
        //optionsGroupBox->layout()->setSpacing( 6 );
        //optionsGroupBox->layout()->setMargin( 11 );
        optionsGroupBox->setTitle( tr("BEAM source options") );
        QVBoxLayout* optionsGroupBoxLayout = new QVBoxLayout(
                                  optionsGroupBox );
        QHBoxLayout* l5 = new QHBoxLayout;
        l5->setSpacing(6); l5->setMargin(0);
        dist = new QLineEdit("0",optionsGroupBox);
        dist->setAlignment( Qt::AlignRight);
        dist->setValidator( new QDoubleValidator(dist) );
        QLabel* distL = new QLabel("DIST",optionsGroupBox);
        l5->addWidget(distL);l5->addWidget(dist);

        angle = new QLineEdit("0",optionsGroupBox);
        angle->setAlignment( Qt::AlignRight);
        angle->setValidator( new QDoubleValidator(angle) );
        QLabel* angleL = new QLabel("ANGLE",optionsGroupBox);
        l5->addWidget(angleL);l5->addWidget(angle);

        zoffset = new QLineEdit("0",optionsGroupBox);
        zoffset->setAlignment( Qt::AlignRight);
        zoffset->setValidator( new QDoubleValidator(zoffset) );
        QLabel* zL = new QLabel("ZOFFSET",optionsGroupBox);
        l5->addWidget(zL);l5->addWidget(zoffset);

        QHBoxLayout* l6 = new QHBoxLayout;
        l6->setSpacing(6); l6->setMargin(0);
        xoffset = new QLineEdit("0",optionsGroupBox);
        xoffset->setAlignment( Qt::AlignRight);
        xoffset->setValidator( new QDoubleValidator(xoffset) );
        QLabel* xL = new QLabel("XOFFSET",optionsGroupBox);
        l6->addWidget(xL);l6->addWidget(xoffset);
        yoffset = new QLineEdit("0",optionsGroupBox);
        yoffset->setAlignment( Qt::AlignRight);
        yoffset->setValidator( new QDoubleValidator(yoffset) );
        QLabel* yL = new QLabel("YOFFSET",optionsGroupBox);
        l6->addWidget(yL);l6->addWidget(yoffset);

        optionsGroupBoxLayout->addLayout( l5 );
        optionsGroupBoxLayout->addLayout( l6 );
        topl->addWidget(optionsGroupBox);

// Vertical spacer
//        QSpacerItem *vSpacer = new QSpacerItem(20,20,QSizePolicy::Minimum,
//                                          QSizePolicy::Expanding);
//        topl->addItem(vSpacer);

        QHBoxLayout* bl = new QHBoxLayout;
        bl->setSpacing(6); bl->setMargin(11);
        QSpacerItem *hSpacer = new QSpacerItem(20,20,QSizePolicy::Expanding,
                                          QSizePolicy::Minimum);
        bl->addItem(hSpacer);

        QPushButton* cancelButton = new QPushButton( "&Cancel", this );
        QPushButton* okButton     = new QPushButton( "O&k", this );

        bl->addWidget(okButton);
        bl->addWidget(cancelButton);

        topl->addLayout(bl);

        connect( okButton, SIGNAL(clicked()), this, SLOT(close()) );
        connect( cancelButton, SIGNAL(clicked()), this, SLOT(cancel()) );
        connect( beam, SIGNAL( activated(const QString&) ),
                 this, SLOT( beamUserCodeChanged(const QString&) ) );

}

void BeamSourceDlg::close()
{
    emit beamSourceDefined();
    QDialog::close();
}
void BeamSourceDlg::cancel()
{
    QDialog::close();
}

void BeamSourceDlg::beamUserCodeChanged(const QString& newDir)
{
    inp->clear();
    inp->addItems(listExtensionLess(EGS_HOME+newDir,"*.egsinp"));
}

void BeamSourceDlg::update_pegs( const QStringList pegsfiles )
{
    pegs->clear();
    pegs->addItems(pegsfiles);
}

