/********************************************************************************
** Form generated from reading UI file 'aboutform_rz.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ABOUTFORM_RZ_H
#define UI_ABOUTFORM_RZ_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_AboutForm
{
public:
    QVBoxLayout *vboxLayout;
    QVBoxLayout *vboxLayout1;
    QHBoxLayout *hboxLayout;
    QSpacerItem *spacer41_2;
    QLabel *pixmapLabel2;
    QSpacerItem *spacer41;
    QVBoxLayout *vboxLayout2;
    QLabel *nrcLabel;
    QLabel *aboutLabel;
    QSpacerItem *spacer49;
    QHBoxLayout *hboxLayout1;
    QSpacerItem *spacer47;
    QPushButton *OKAboutButton;
    QSpacerItem *spacer47_2;

    void setupUi(QDialog *AboutForm)
    {
        if (AboutForm->objectName().isEmpty())
            AboutForm->setObjectName(QString::fromUtf8("AboutForm"));
        AboutForm->resize(500, 565);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(AboutForm->sizePolicy().hasHeightForWidth());
        AboutForm->setSizePolicy(sizePolicy);
        AboutForm->setMinimumSize(QSize(0, 0));
        AboutForm->setMaximumSize(QSize(500, 700));
        vboxLayout = new QVBoxLayout(AboutForm);
        vboxLayout->setSpacing(6);
        vboxLayout->setContentsMargins(11, 11, 11, 11);
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        vboxLayout1 = new QVBoxLayout();
        vboxLayout1->setSpacing(6);
        vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
        hboxLayout = new QHBoxLayout();
        hboxLayout->setSpacing(6);
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        spacer41_2 = new QSpacerItem(28, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacer41_2);

        pixmapLabel2 = new QLabel(AboutForm);
        pixmapLabel2->setObjectName(QString::fromUtf8("pixmapLabel2"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(pixmapLabel2->sizePolicy().hasHeightForWidth());
        pixmapLabel2->setSizePolicy(sizePolicy1);
        pixmapLabel2->setPixmap(QPixmap(QString::fromUtf8("image0")));
        pixmapLabel2->setScaledContents(true);
        pixmapLabel2->setWordWrap(false);

        hboxLayout->addWidget(pixmapLabel2);

        spacer41 = new QSpacerItem(28, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacer41);


        vboxLayout1->addLayout(hboxLayout);

        vboxLayout2 = new QVBoxLayout();
        vboxLayout2->setSpacing(6);
        vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
        nrcLabel = new QLabel(AboutForm);
        nrcLabel->setObjectName(QString::fromUtf8("nrcLabel"));
        nrcLabel->setWordWrap(false);

        vboxLayout2->addWidget(nrcLabel);

        aboutLabel = new QLabel(AboutForm);
        aboutLabel->setObjectName(QString::fromUtf8("aboutLabel"));
        aboutLabel->setWordWrap(false);

        vboxLayout2->addWidget(aboutLabel);


        vboxLayout1->addLayout(vboxLayout2);


        vboxLayout->addLayout(vboxLayout1);

        spacer49 = new QSpacerItem(20, 43, QSizePolicy::Minimum, QSizePolicy::Expanding);

        vboxLayout->addItem(spacer49);

        hboxLayout1 = new QHBoxLayout();
        hboxLayout1->setSpacing(6);
        hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
        spacer47 = new QSpacerItem(121, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout1->addItem(spacer47);

        OKAboutButton = new QPushButton(AboutForm);
        OKAboutButton->setObjectName(QString::fromUtf8("OKAboutButton"));

        hboxLayout1->addWidget(OKAboutButton);

        spacer47_2 = new QSpacerItem(121, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout1->addItem(spacer47_2);


        vboxLayout->addLayout(hboxLayout1);


        retranslateUi(AboutForm);
        QObject::connect(OKAboutButton, SIGNAL(clicked()), AboutForm, SLOT(close()));

        QMetaObject::connectSlotsByName(AboutForm);
    } // setupUi

    void retranslateUi(QDialog *AboutForm)
    {
        AboutForm->setWindowTitle(QApplication::translate("AboutForm", "About egs_inprz: GUI for EGSnrc RZ user codes", 0, QApplication::UnicodeUTF8));
        nrcLabel->setText(QApplication::translate("AboutForm", "<p align=\"center\">\n"
"Copyright (C) year National Research Council Canada</p>", 0, QApplication::UnicodeUTF8));
        aboutLabel->setText(QApplication::translate("AboutForm", "<p align=\"center\"><b>Graphical User Interface for EGSnrc RZ user codes</b><br>\n"
"<b>  egs_inprz, version YYY </b><br><br>\n"
"Authors: Ernesto Mainegra and Iwan Kawrakow<br><br>\n"
"This program is free software. It is distributed\n"
"under the terms of the GNU Public License, version 2.\n"
"See the file GPL_License in the licenses folder of \n"
"the EGSnrcMP distribution for details.\n"
"<br><br>\n"
"This program uses the Qt toolkit Version XXX by Trolltech</p>", 0, QApplication::UnicodeUTF8));
        OKAboutButton->setText(QApplication::translate("AboutForm", "&OK", 0, QApplication::UnicodeUTF8));
        OKAboutButton->setShortcut(QApplication::translate("AboutForm", "Alt+O", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class AboutForm: public Ui_AboutForm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ABOUTFORM_RZ_H
