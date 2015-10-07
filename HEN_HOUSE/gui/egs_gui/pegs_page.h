/*
###############################################################################
#
#  EGSnrc gui pegs page headers
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


#ifndef PEGS_PAGE_H
#define PEGS_PAGE_H

#include <QObject>
#include <QWidget>
#include <QEvent>
#include <QTableWidget>
#include <QItemDelegate>
#include <QModelIndex>
#include <QSize>
#include <qdialog.h>
#include <qcombobox.h>
#include "ui_pegs_page.h"
#include <string>

class EGS_ConfigReader;
class QProcess;
class PEGS_RunOutput;
class QTableWidget;

struct Element {
  int   Z;
  std::string symbol;
 float  aw;
 float  Iev;
 float  rho;
};

const int n_element = 100;

extern Element element_data[];

class TableEventHandler : public QObject {
    Q_OBJECT
public:
    TableEventHandler(QTableWidget *parent);
protected:
    bool eventFilter(QObject *o, QEvent *e);
private:
    QStringList itemCopy;
    QList<QTableWidgetSelectionRange> copyRange;
};

class ComboBoxDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    ComboBoxDelegate( QObject *parent = 0) ;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class EGS_PegsPage: public QWidget, public Ui::EGS_PegsPage
{
  Q_OBJECT

public:

EGS_PegsPage(QWidget* parent): QWidget(parent){setupUi(this);init();};
~EGS_PegsPage(){}

public slots:

    void initializeCompositionTable();
    void densityIcruChanged( bool is_on);
    void medtypeChanged( const QString &s );
    void getDensityFile();
    void newDataFileChecked(bool b);
    void appendDataFileChecked(bool b);
    void setOfile();
    void startPegs();
    void stopPegs();
    void readPegsStdout();
    void readPegsStderr();
    void pegsFinished();
    void showHideDetails();
    void outputClosed();
    void launchReturned();
    void setConfigReader(EGS_ConfigReader *r);

protected:
    void init();
private:
    bool output_is_active;
    PEGS_RunOutput *run_output;
    QProcess *pegs_process;
    double ae,ap,ue,up;
    int nelem;
    EGS_ConfigReader *config_reader;
    bool checkFields();
};
#endif // PEGS_PAGE_H
