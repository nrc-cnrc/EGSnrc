/*
###############################################################################
#
#  EGSnrc configuration GUI licence
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
#  Author:          Ernesto Mainegra-Hing, 2015
#
#  Contributors:
#
###############################################################################
*/


#ifndef EGS_LICENCE_H
#define EGS_LICENCE_H

#include <QWizardPage>
#include <QLabel>
#include <QScrollArea>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QRadioButton>

class QWizardPage;

class QLicencePage : public QWizardPage
{

  Q_OBJECT

public:

  QLicencePage(QWidget * parent, const QString & year, const QString & version) :
               QWizardPage(parent), the_year(year), the_version(version)
  {

    setTitle("EGSnrc distributed under the terms of the AGPL Licence");
    setSubTitle("Press Next to proceed.");
    QHBoxLayout *hl = new QHBoxLayout(this);
    QScrollArea *scroll = new QScrollArea(this);
    scroll->setBackgroundRole(QPalette::Base);
    hl->addWidget(scroll);
    lic = new QLabel(); lic->setOpenExternalLinks(true); lic->setTextInteractionFlags(Qt::TextBrowserInteraction);
    //lic->setText(QString(EGS_LICENCE).replace("the_year",year).replace("the_version",version));
    //lic->setText(QString(EGS_LICENCE).replace("the_year",year));
    lic->setText(
    "<p>    Copyright (C) 2015 National Research Council Canada</p>"\
    "<p>EGSnrc is free software: you can redistribute it and/or modify it under<br>"\
    "the terms of the GNU Affero General Public License as published by the<br>"\
    "Free Software Foundation, either version 3 of the License, or (at your<br>"\
    "option) any later version.<br></p>"\
    "<p>EGSnrc is distributed in the hope that it will be useful, but WITHOUT ANY<br>"\
    "WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS<br>"\
    "FOR A PARTICULAR PURPOSE.  See the <a href=\"http://www.gnu.org/licenses/agpl.html\">GNU Affero General Public License</a> for<br>"\
    "more details.<br></p>"
    );
    scroll->setWidget(lic);

  }
  ~QLicencePage(){}

public slots:

signals:

private:

  QLabel *lic;
  QString the_year,
          the_version;

};

#endif