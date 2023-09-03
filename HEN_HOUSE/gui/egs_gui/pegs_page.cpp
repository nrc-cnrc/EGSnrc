/*
###############################################################################
#
#  EGSnrc gui pegs page
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
#  Contributors:    Blake Walters
#                   Reid Townson
#                   Cody Crewson
#
###############################################################################
*/


#include "pegs_page.h"

#include <QApplication>
#include <QKeyEvent>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QTextStream>
#include <QtGlobal>
#if QT_VERSION >= 0x050000
    #include <QtWidgets>
#else
    #include <QWidget>
#endif

#include <qstringlist.h>
#include <qfiledialog.h>
#include <qdir.h>
#include <qprocess.h>
#include <qregexp.h>

#include <math.h>

#include "egs_gui_widget.h"
#include "egs_config_reader.h"
#include "pegs_runoutput.h"
#include <iostream>

//#define PP_DEBUG

QStringList *elements = 0;

TableEventHandler *tfilter = 0;

const double rm=0.5109989461;

Element element_data[] = {
    {1,"H",1.0079,19.2,8.3748e-05},
    {2,"He",4.0026,41.8,0.00016632},
    {3,"Li",6.941,40,0.534},
    {4,"Be",9.0122,63.7,1.848},
    {5,"B",10.812,76,2.37},
    {6,"C",12.011,78,2},
    {7,"N",14.0067,82,0.0011653},
    {8,"O",15.9994,95,0.0013315},
    {9,"F",18.9984,115,0.0015803},
    {10,"Ne",20.1797,137,0.0008385},
    {11,"Na",22.9898,149,0.971},
    {12,"Mg",24.305,156,1.74},
    {13,"Al",26.9815,166,2.702},
    {14,"Si",28.0855,173,2.33},
    {15,"P",30.9738,173,2.2},
    {16,"S",32.066,180,2},
    {17,"Cl",35.4527,174,0.0029947},
    {18,"Ar",39.948,188,0.001662},
    {19,"K",39.0983,190,0.862},
    {20,"Ca",40.078,191,1.55},
    {21,"Sc",44.9559,216,2.989},
    {22,"Ti",47.88,233,4.54},
    {23,"V",50.9415,245,6.11},
    {24,"Cr",51.9961,257,7.18},
    {25,"Mn",54.938,272,7.44},
    {26,"Fe",55.847,286,7.874},
    {27,"Co",58.9332,297,8.9},
    {28,"Ni",58.69,311,8.902},
    {29,"Cu",63.546,322,8.96},
    {30,"Zn",65.39,330,7.133},
    {31,"Ga",69.723,334,5.904},
    {32,"Ge",72.61,350,5.323},
    {33,"As",74.9216,347,5.73},
    {34,"Se",78.96,348,4.5},
    {35,"Br",79.904,357,0.0070722},
    {36,"Kr",83.8,352,0.0034783},
    {37,"Rb",85.4678,363,1.532},
    {38,"Sr",87.62,366,2.54},
    {39,"Y",88.9059,379,4.469},
    {40,"Zr",91.224,393,6.506},
    {41,"Nb",92.9064,417,8.57},
    {42,"Mo",95.94,424,10.22},
    {43,"Tc",97.9072,428,11.5},
    {44,"Ru",101.07,441,12.41},
    {45,"Rh",102.906,449,12.41},
    {46,"Pd",106.42,470,12.02},
    {47,"Ag",107.868,470,10.5},
    {48,"Cd",112.411,469,8.65},
    {49,"In",114.82,488,7.31},
    {50,"Sn",118.71,488,7.31},
    {51,"Sb",121.75,487,6.691},
    {52,"Te",127.6,485,6.24},
    {53,"I",126.904,491,4.93},
    {54,"Xe",131.29,482,0.0054854},
    {55,"Cs",132.905,488,1.873},
    {56,"Ba",137.327,491,3.5},
    {57,"La",138.905,501,6.154},
    {58,"Ce",140.115,523,6.657},
    {59,"Pr",140.908,535,6.71},
    {60,"Nd",144.24,546,6.9},
    {61,"Pm",144.913,560,7.22},
    {62,"Sm",150.36,574,7.46},
    {63,"Eu",151.965,580,5.243},
    {64,"Gd",157.25,591,7.9004},
    {65,"Tb",158.925,614,8.229},
    {66,"Dy",162.5,628,8.55},
    {67,"Ho",164.93,650,8.795},
    {68,"Er",167.26,658,9.066},
    {69,"Tm",168.934,674,9.321},
    {70,"Yb",173.04,684,6.73},
    {71,"Lu",174.967,694,9.84},
    {72,"Hf",178.49,705,13.31},
    {73,"Ta",180.948,718,16.654},
    {74,"W",183.85,727,19.3},
    {75,"Re",186.207,736,21.02},
    {76,"Os",190.2,746,22.57},
    {77,"Ir",192.22,757,22.42},
    {78,"Pt",195.08,790,21.45},
    {79,"Au",196.966,790,19.32},
    {80,"Hg",200.59,800,13.546},
    {81,"Tl",204.383,810,11.72},
    {82,"Pb",207.2,823,11.34},
    {83,"Bi",208.98,823,9.747},
    {84,"Po",208.982,830,9.32},
    {85,"At",209.987,825,9.32},
    {86,"Rn",222.018,794,0.0090662},
    {87,"Fr",223.02,827,1},
    {88,"Ra",226.025,826,5},
    {89,"Ac",227.028,841,10.07},
    {90,"Th",232.038,847,11.72},
    {91,"Pa",231.036,878,15.37},
    {92,"U",238.029,890,18.95},
    {93,"Np",237.048,902,20.25},
    {94,"Pu",239.052,921,19.84},
    {95,"Am",243.061,934,13.67},
    {96,"Cm",247.07,939,13.51},
    {97,"Bk",247.07,952,14},
    {98,"Cf",251.08,966,10},
    {99,"Es",252.083,980,10},
    {100,"Fm",257.095,994,10}
};

void EGS_PegsPage::setConfigReader(EGS_ConfigReader *r) {
    config_reader = r;
}

void EGS_PegsPage::init()
{
#ifdef PP_DEBUG
  qDebug("In EGS_PegsPage::init()");
#endif

  config_reader = 0;
  connect(new_data_file,SIGNAL(toggled(bool)),this,
                        SLOT(newDataFileChecked(bool)));
  connect(append_to_datafile,SIGNAL(toggled(bool)),this,
                             SLOT(appendDataFileChecked(bool)));
  connect(ofile_b,SIGNAL(clicked()),this,
                  SLOT(setOfile()));
  connect(go_button,SIGNAL(clicked()),this,SLOT(startPegs()));
  connect(cancel_button,SIGNAL(clicked()),this,SLOT(stopPegs()));

  new_data_file->setChecked(true); cancel_button->setEnabled(false);
  frt_err=false;
  gasp_err=false;
  pegs_process = new QProcess;
  connect(pegs_process,SIGNAL(readyReadStandardOutput()),this,SLOT(readPegsStdout()));
  connect(pegs_process,SIGNAL(readyReadStandardError()),this,SLOT(readPegsStderr()));
  connect(pegs_process,SIGNAL(finished(int, QProcess::ExitStatus)),this,SLOT(pegsFinished()));

  run_output = new PEGS_RunOutput(0);
  connect(run_output,SIGNAL(windowClosed()),this, SLOT(outputClosed()));

  initializeCompositionTable();

}

void EGS_PegsPage::initializeCompositionTable(){
  composition_table->setItemDelegate(new ComboBoxDelegate(composition_table));
#ifdef PP_DEBUG
  qDebug("ctable->objectName() = %s",composition_table->objectName().toLatin1().data());
#endif
  if( elements == 0 ) {
    elements = new QStringList;
    for(int j=0; j<n_element; j++) (*elements) << QString::fromStdString(element_data[j].symbol);
  }
  dc_icru_check->setChecked(false); dc_icru_check->setChecked(true);
  for(int j=0; j<composition_table->rowCount(); j++) {
    QTableWidgetItem *newItem = new QTableWidgetItem(tr(""));
    composition_table->setItem(j,0,newItem);
  }

  if( tfilter == 0 ) tfilter = new TableEventHandler(composition_table);
  composition_table->installEventFilter(tfilter);

}

void EGS_PegsPage::densityIcruChanged( bool is_on) {
#ifdef PP_DEBUG
  qDebug("EGS_PegsPage::densityIcruChanged: %d",is_on);
#endif
  composition_table->setEnabled( !is_on );
  dc_group->setEnabled( is_on );
  rho_group->setEnabled( !is_on );
  medtype_cbox->setEnabled( !is_on );
  is_gas->setEnabled( !is_on );
  enable_gaspEdit();
  if(is_on) {
    //see if a file has already been specified
    if(!dc_file->text().isEmpty()) {
    //now check to see if this points to a file that exists
     QFileInfo dfi(dc_file->text());
     if( !dfi.exists() ) {
      QMessageBox::critical(this,"Warning",
              QString("Density correction file %1 does not exist ?").arg(dc_file->text()),QMessageBox::Ok,0);
      return;
     }
     else{
      //read contents
      readDensityFile(dc_file->text());
     }
   }
  }
}

void EGS_PegsPage::enable_gaspEdit()
{
   if(is_gas->isChecked() && !dc_icru_check->isChecked()) {
    gaspLabel->setEnabled(true);
    gaspEdit->setEnabled(true);
    gaspUnits->setEnabled(true);
   }
   else {
    gaspLabel->setEnabled(false);
    gaspEdit->setEnabled(false);
    gaspUnits->setEnabled(false);
   }
}

void EGS_PegsPage::medtypeChanged( const QString &s )
{
#ifdef PP_DEBUG
  qDebug("In EGS_PegsPage::medtypeChanged(%s)",s.toLatin1().data());
#endif
  if( s == "Compound" ) composition_table->setHorizontalHeaderItem(1,new QTableWidgetItem("Stoichiometric index"));
  else composition_table->setHorizontalHeaderItem(1,new QTableWidgetItem(" Fraction by weight "));
  composition_table->adjustSize();
  bool ro = s == "Element";
  for(int j=0; j < composition_table->rowCount(); j++) {
    if( ro ) {
      composition_table->setItem(j,0,0); composition_table->setItem(j,1,0);
    }
  }
}

enum MedIndex {Elem, Comp, Mixt};

void EGS_PegsPage::getDensityFile() {
#ifdef PP_DEBUG
  qDebug("In EGS_PegsPage::getDensityFile()");
#endif
  QString start_dir;
  if( !config_reader ) config_reader = new EGS_ConfigReader;
  if( dc_where->currentText() == "HEN_HOUSE" )
    start_dir = config_reader->getVariable("HEN_HOUSE",true);
  else start_dir = config_reader->getVariable("EGS_HOME",true);
  start_dir += "pegs4"; start_dir += QDir::separator();
  start_dir += "density_corrections";
  QString dfile = QFileDialog::getOpenFileName(this,tr("Select a density correction file"),
                                               start_dir,
                                               tr("Density correction files (*.density)"));
#ifdef PP_DEBUG
  qDebug("dfile = %s",dfile.toLatin1().data());
#endif
  if( dfile.isEmpty() ) return;
  QFileInfo fi(dfile);
  readDensityFile(dfile);
  dc_file->setText(fi.absoluteFilePath());
}

void EGS_PegsPage::readDensityFile(QString dfile) {
//open and read contents of dc file (dfile) & update medium
//composition table & medium type
  QFile f(dfile);
  if( !f.open(QFile::ReadOnly) ) return;
  char buf[256]; f.readLine(buf,255); // ignore first line
#ifdef PP_DEBUG
  qDebug("got line: %s",buf);
#endif
  QTextStream data(&f);
  int ndat; double Iev, rho;
  data >> ndat >> Iev >> rho >> nelem;
#ifdef PP_DEBUG
  qDebug(" ndat = %d Iev = %lf rho = %lf nelem = %d",ndat,Iev,rho,nelem);
#endif
  if( nelem == 1 ) {
    medtype_cbox->setCurrentIndex(Elem);
    medtypeChanged("Element");
  }
  else {
#ifdef PP_DEBUG
  qDebug("==> Changing medium type to mixture since nelem = %d!",nelem);
#endif
    medtype_cbox->setCurrentIndex(Mixt);
    medtypeChanged("Mixture");
  }
  rho_le->setText(QString("%1").arg(rho));
  int j;
  for(j=0; j < composition_table->rowCount(); j++) {
    composition_table->setItem(j,0,0); composition_table->setItem(j,1,0);
  }
  for(j=0; j<nelem; j++) {
    int iz; double frac; data >> iz >> frac;
    composition_table->setItem(j,0,new QTableWidgetItem(QString::fromStdString(element_data[iz-1].symbol)));
    composition_table->setItem(j,1,new QTableWidgetItem(QString("%1").arg(frac)));
  }
}

void EGS_PegsPage::newDataFileChecked(bool b) {
#ifdef PP_DEBUG
  qDebug("In EGS_PegsPage::newDataFileChecked(%d)",b);
#endif
  append_to_datafile->setChecked(!b);
}

void EGS_PegsPage::appendDataFileChecked(bool b) {
#ifdef PP_DEBUG
  qDebug("In EGS_PegsPage::appendDataFileChecked(%d)",b);
#endif
  new_data_file->setChecked(!b);
}

void EGS_PegsPage::setOfile() {
  QString s; if( !config_reader ) config_reader = new EGS_ConfigReader;
  QString start_dir = config_reader->getVariable("EGS_HOME",true);
  start_dir += "pegs4"; start_dir += QDir::separator();
  start_dir += "data";
  if( append_to_datafile->isChecked() )
    s = QFileDialog::getOpenFileName(this,tr("Select a PEGS file"),
                                     start_dir, tr("PEGS files (*.pegs4dat)"));
  else
    s = QFileDialog::getSaveFileName(this,tr("Select a PEGS file"),
                                     start_dir,tr("PEGS files (*.pegs4dat)"));
  if( !s.isEmpty() ) {
    QFileInfo fi(s);
    ofile_le->setText(fi.baseName());
  }
}

void EGS_PegsPage::startPegs() {
#ifdef PP_DEBUG
  qDebug("In EGS_PegsPage::startPegs()");
#endif
  if( !checkFields() ) return;
  if( !config_reader ) config_reader = new EGS_ConfigReader;
  QString executable = config_reader->getVariable("HEN_HOUSE",true);
  executable += "bin"; executable += QDir::separator();
  executable += config_reader->getVariable("my_machine");
  executable += QDir::separator();
  executable += "pegs4.exe";
  QFileInfo fi(executable);
  if( !fi.exists() ) {
      QMessageBox::critical(this,"Error",
              QString("%1 does not exist ?").arg(executable),QMessageBox::Ok,0);
      return;
  }
  if( !fi.isExecutable() ) {
    QMessageBox::critical(this,"Error",
     QString("%1 is not executable ?").arg(executable),QMessageBox::Ok,0);
    return;
  }

  // process arguments
  QStringList args;
  //pegs_process->clearArguments();
  //pegs_process->addArgument(executable);
  args << "-e";//pegs_process->addArgument("-e");
  args << config_reader->getVariable("EGS_HOME",true);//pegs_process->addArgument();
  args << "-h";//pegs_process->addArgument("-h");
  args << config_reader->getVariable("HEN_HOUSE",true);//pegs_process->addArgument();
  args << "-o";//pegs_process->addArgument("-o");
  args << ofile_le->text();//pegs_process->addArgument();
  run_output->setOutputFile(ofile_le->text());
  if( append_to_datafile->isChecked() )
    args << "-a";//pegs_process->addArgument("-a");
  if( dc_icru_check->isChecked() ) {
    QFileInfo dfi(dc_file->text());
    if( !dfi.exists() ) {
      QMessageBox::critical(this,"Error",
              QString("Density correction file %1 does not exist ?").arg(dc_file->text()),QMessageBox::Ok,0);
      return;
    }
    args << "-d";//pegs_process->addArgument("-d");
    if(dc_file->text().endsWith(".density"))
    args << dc_file->text().remove(dc_file->text().lastIndexOf(".density"),8);//pegs_process->addArgument();
    else
    args << dc_file->text();
  }
  //QStringList list = pegs_process->arguments();
#ifdef PP_DEBUG
  qDebug("Executing: <%s>",args.join(" ").toLatin1().data());
#endif

  // PEGS input
  QString input; QTextStream ts( &input, QIODevice::WriteOnly );
  ts << "ENER\n &INP AE=" << ae << ",UE=" << ue << ",AP=" << ap << ",UP="
     << up << " &END\n";
  if( medtype_cbox->currentText() == "Element" ) {
    ts << "ELEM\n &INP ";
  }
  else {
    if( medtype_cbox->currentText() == "Compound" ) {
      ts << "COMP\n &INP NE=" << nelem << ",PZ=";
    }
    else {
      ts << "MIXT\n &INP NE=" << nelem << ",RHOZ=";
    }
    for(int j=0; j<nelem; j++) {
      bool is_ok;
      double w = composition_table->item(j,1)->text().toDouble(&is_ok);
      if( !is_ok ) {
        QString err = QString("Wrong input in row %1, column 1").arg(j);
        QMessageBox::critical(this,"Error",err,QMessageBox::Ok,0);
        return;
      }
      ts << w << ",";
    }
  }
  ts << "RHO="; bool is_ok;
  double rho = rho_le->text().toDouble(&is_ok);
  if( !is_ok ) {
    QString err = QString("Wrong mass density input");
    QMessageBox::critical(this,"Error",err,QMessageBox::Ok,0);
    return;
  }
  if( comboBox2->currentText() == "kg/m**3" ) rho *= 0.001;//convert to g/cm**3
  ts << rho;
  if( is_gas->isChecked() && !dc_icru_check->isChecked() && gaspEdit->text().toFloat()>0.0) ts << ",GASP= " << gaspEdit->text();
  ts << ",EPSTFL=" << dc_icru_check->isChecked() << ",IAPRIM="
     << rad_icru_check->isChecked() << ",IRAYL=" << rayleigh_check->isChecked()
     << " &END\n";
  ts << medname_le->text() << "\n";
  for(int j=0; j<nelem; j++) {
    QString e = composition_table->item(j,0)->text().toUpper();
    if( j > 0 ) ts << " ";
    ts << e; if( e.length() == 1 ) ts << " ";
  }
  ts << "\n";
  ts << "PWLF\n &INP  &END\nDECK\n &INP  &END\n";
#ifdef PP_DEBUG
  qDebug("Input string:\n%s",input.toLatin1().data());
#endif

  run_output->clearOutput();
  //pegs_process->launch(input);
  pegs_process->start(executable,args);
  if (!pegs_process->waitForStarted()){
    QMessageBox::critical(this,"Error",
     QString("PEGS failed, exit status was %1").arg(pegs_process->exitStatus()),
      QMessageBox::Ok,0);
    go_button->setEnabled(true);
    cancel_button->setEnabled(false);
    return;
  }
  pegs_process->write(input.toLatin1());
  pegs_process->closeWriteChannel();

  go_button->setEnabled(false);
  cancel_button->setEnabled(true);
}

void EGS_PegsPage::stopPegs() {
#ifdef PP_DEBUG
  qDebug("In EGS_PegsPage::stopPegs()");
#endif
  go_button->setEnabled(true);
  cancel_button->setEnabled(false);
}

void EGS_PegsPage::readPegsStdout() {
#ifdef PP_DEBUG
  qDebug("In EGS_PegsPage::readPegsStdout()");
#endif
  QString tmp = pegs_process->readAllStandardOutput();
  gasp_err=tmp.contains(QString("YOU MUST DEFINE GASP"));
  run_output->insertText(tmp);
}

void EGS_PegsPage::readPegsStderr() {
#ifdef PP_DEBUG
  qDebug("In EGS_PegsPage::readPegsStderr()");
#endif
  QString tmp = pegs_process->readAllStandardError();
  frt_err=tmp.contains(QString("Fortran runtime error"));
  run_output->insertText(tmp);
}

void EGS_PegsPage::pegsFinished() {
#ifdef PP_DEBUG
  qDebug("In EGS_PegsPage::pegsFinished()");
#endif
  if(frt_err)
    QMessageBox::critical(this,"Error",
     QString("PEGS failed with runtime error."),
      QMessageBox::Ok,0);
  else if(gasp_err)
    QMessageBox::critical(this,"Error",
     QString("PEGS failed: Define medium as gas."),
      QMessageBox::Ok,0);
  else if( pegs_process->exitStatus() == 0 ) // QProcess::NormalExit = 0
    QMessageBox::information(this,"PEGS finished","PEGS finished successfuly",
       QMessageBox::Ok);
  else                                  // QProcess::CrashExit = 1
    QMessageBox::critical(this,"Error",
     QString("PEGS failed, exit status was %1").arg(pegs_process->exitStatus()),
      QMessageBox::Ok,0);
  go_button->setEnabled(true);
  cancel_button->setEnabled(false);
}

bool EGS_PegsPage::checkFields() {
  bool res = true; if( !config_reader ) config_reader = new EGS_ConfigReader;
  if( medname_le->text().isEmpty() ) {
    QMessageBox::critical(this,"Error",
      "You must give the medium a name",QMessageBox::Ok,0);
    res = false;
  }
  if( dc_icru_check->isChecked() ) {
    if( dc_file->text().isEmpty() ) {
      QMessageBox::critical(this,"Error",
        "You must define the density correction file",QMessageBox::Ok,0);
      res = false;
    }
  }
  else {
    if( rho_le->text().isEmpty() ) {
      QMessageBox::critical(this,"Error",
        "You must define the mass density",QMessageBox::Ok,0);
      res = false;
    }
    nelem=0;
    for(int j=0; j<20; j++) {
      if( !composition_table->item(j,0) ||
          !composition_table->item(j,1) ) break;
      if( composition_table->item(j,0)->text().isEmpty() ||
          composition_table->item(j,1)->text().isEmpty() ) break;
      nelem = j+1;
    }
#ifdef PP_DEBUG
    qDebug("nelem = %d type = %s",nelem,medtype_cbox->currentText().toLatin1().data());
#endif
    if( !nelem ) {
      QMessageBox::critical(this,"Error",
        "You must define the medium composition",QMessageBox::Ok,0);
      res = false;
    }
    if( nelem == 1 && medtype_cbox->currentText() != "Element" ) {
      QString msg=QString("You have specified the medium to be %1").arg(
           medtype_cbox->currentText());
      msg += "\nbut there is only a single element specified in the";
      msg += "\nmedium composition table. ";
      QMessageBox::critical(this,"Error",msg,QMessageBox::Ok,0);
      res = false;
    }
  }
  if( ae_le->text().isEmpty() || ue_le->text().isEmpty() ||
      ap_le->text().isEmpty() || up_le->text().isEmpty() ) {
    QString err="You must define the energy range for the data";
    err += "\n  electrons:  AE...UE";
    err += "\n    photons:  AP...UP";
    QMessageBox::critical(this,"Error",err,QMessageBox::Ok,0);
    res = false;
  }
  else {
    bool ok_ae, ok_ap, ok_ue, ok_up;
    ae = ae_le->text().toDouble(&ok_ae);
    ap = ap_le->text().toDouble(&ok_ap);
    ue = ue_le->text().toDouble(&ok_ue);
    up = up_le->text().toDouble(&ok_up);
    if( !ok_ae || !ok_ap || !ok_ue || !ok_up ) {
      QString err="Please check input for ";
      if( !ok_ae ) err += "AE";
      if( !ok_ap ) err += ", AP";
      if( !ok_ue ) err += ", UE";
      if( !ok_up ) err += ", UP";
      QMessageBox::critical(this,"Error",err,QMessageBox::Ok,0);
      res = false;
    }
    else {
      if( ae_units->currentText() == "keV" ) ae *= 0.001;
      if( ae < rm ) {
        QMessageBox::critical(this,"Error",
         "AE can not be less than the electron rest energy!",QMessageBox::Ok,0);
        res = false;
      }
      if( ap_units->currentText() == "keV" ) ap *= 0.001;
      if( ue_units->currentText() == "keV" ) ue *= 0.001;
      else if( ue_units->currentText() == "GeV" ) ue *= 1000;
      if( up_units->currentText() == "keV" ) up *= 0.001;
      else if( up_units->currentText() == "GeV" ) up *= 1000;
      if( ae >= ue || ap >= up ) {
        QString err;
        if( ae >= ue ) err += "AE >= UE ?  ";
        if( ap >= up ) err += "AP >= UP ?  ";
        QMessageBox::critical(this,"Error",err,QMessageBox::Ok,0);
        res = false;
      }
    }
  }
  if( ofile_le->text().isEmpty() ) {
    QMessageBox::critical(this,"Error","You must specify the output file name",
       QMessageBox::Ok,0);
    res = false;
  }
  else {
    QString fn = config_reader->getVariable("EGS_HOME",true);
    fn += "pegs4"; fn += QDir::separator(); fn += "data";
    fn += QDir::separator(); fn += ofile_le->text().replace(QRegExp("\\.pegs4dat$"), ""); fn += ".pegs4dat";
    QFileInfo fi(fn);
    if( new_data_file->isChecked() && fi.exists() ) {
      QString err="You have specified to create a new PEGS data file";
      err+=QString("\nbut a file named %1 alread exists").arg(fn);
      QMessageBox::critical(this,"Error",err,QMessageBox::Ok,0);
      res = false;
    }
    else if( append_to_datafile->isChecked() && !fi.exists() ) {
      QString err="You have specified to append the data to";
      err+="\nan existing PEGS file, but a file named\n  ";
      err+=fn;
      err+="\ndoes not exist";
      QMessageBox::critical(this,"Error",err,QMessageBox::Ok,0);
      res = false;
    }
  }
  return res;
}


void EGS_PegsPage::showHideDetails()
{
  if( output_is_active ) {
    details_b->setText("&Details>>"); run_output->hide();
    output_is_active = false;
  }
  else {
    details_b->setText("<<&Details"); run_output->show();
    output_is_active = true;
  }
}

void EGS_PegsPage::outputClosed()
{
  output_is_active = false;
  details_b->setText("&Details>>");
}

void EGS_PegsPage::launchReturned() {
#ifdef PP_DEBUG
  qDebug("In EGS_PegsPage::launchReturned()");
#endif
}

ComboBoxDelegate::ComboBoxDelegate(QObject *parent)
    : QItemDelegate(parent)
{}

QWidget *ComboBoxDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem & option ,
    const QModelIndex & index) const
{
    if (index.column()==0){
       QComboBox *editor = new QComboBox(parent); int ei = 0;
       QString s = index.model()->data(index, Qt::EditRole).toString();
       for(int j=1; j<=n_element; j++) {
          QString aux = QString("%1  %2 ").arg(j,3).arg(QString::fromStdString(element_data[j-1].symbol),2);
          editor->addItem(aux);
          if( s.toStdString() == element_data[j-1].symbol ) ei = j-1;
       }
       editor->setCurrentIndex(ei);
       return editor;
    }
    else return QItemDelegate::createEditor(parent,option,index);
}

void ComboBoxDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{

    if (QString(editor->metaObject()->className())==tr("QComboBox")){
       QString text = index.model()->data(index, Qt::EditRole).toString();
       QComboBox *cb = static_cast<QComboBox*>(editor);
       int i = 0;
       for(int j=1; j<=n_element; j++) {
          if( text.toStdString() == element_data[j-1].symbol ) { i = j-1; }
       }
       if (i != -1)
          cb->setCurrentIndex(i);
       else if (cb->isEditable())
            cb->setEditText(text);
        else
            cb->setCurrentIndex(0);
    }
    else QItemDelegate::setEditorData(editor,index);
}

void ComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    if (QString(editor->metaObject()->className())==tr("QComboBox")){
       QComboBox *cb = static_cast<QComboBox*>(editor);
       QString str = cb->itemText(cb->currentIndex());
       model->setData(index, QVariant(str.right(3).trimmed()), Qt::EditRole);
    }
    else{
      QItemDelegate::setModelData(editor,model,index);
    }
}

void ComboBoxDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

TableEventHandler::TableEventHandler(QTableWidget *parent) :
  QObject(parent) {
  if( parent == 0 )
   qFatal("TableEventHandler::TableEventHandler: parent can not be null!");
}


bool TableEventHandler::eventFilter(QObject *o, QEvent *e) {
  if( !o ) qWarning("TableEventHandler::eventFilter called with 0 object?");
  if( QString(o->metaObject()->className()) != tr("QTableWidget") ) {
#ifdef EI_DEBUG
      qDebug("Only QTableWidget objects accepted! Returning!");
#endif
      return false;
  }
  QTableWidget *to = (QTableWidget *)o;
  if( e->type() == QEvent::KeyPress ) {
    QKeyEvent *ke = (QKeyEvent*)e;
    if(ke->matches(QKeySequence::Copy) ){
       QString cellText; itemCopy.clear(); copyRange.clear();
       QList<QTableWidgetSelectionRange> ts = to->selectedRanges();
       if(!ts.isEmpty()) {
          for ( int irow = ts.first().topRow(); irow <= ts.first().bottomRow(); irow++){
               for ( int icol = ts.first().leftColumn(); icol <= ts.first().rightColumn(); icol++){
                   QTableWidgetItem *w = to->item(irow,icol);
                   if(w) cellText = w->text();
                   if ( !cellText.isEmpty() ){
                      itemCopy << cellText;
                   }
                   else
                      itemCopy << " ";
               }
          }
          copyRange = ts;
          //cout << itemCopy.join(", ").toLatin1().data() << endl;
       }
       else {
            QTableWidgetItem *w = to->item(to->currentRow(), to->currentColumn());
            if (w) cellText = w->text();
            if ( !cellText.isEmpty() )
                 itemCopy << cellText;
            else itemCopy << "";
       }
       return true;
    }
    else if(ke->matches(QKeySequence::Paste) && !itemCopy.isEmpty() && !copyRange.isEmpty()){
       QList<QTableWidgetSelectionRange> cs = to->selectedRanges();
       int top = cs.first().topRow(), left = cs.first().leftColumn(), icount = 0;
       QTableWidgetSelectionRange ts = QTableWidgetSelectionRange(
                                       top , left,
                                       top  + copyRange.first().rowCount()-1,
                                       left + copyRange.first().columnCount()-1);
       for ( int irow = ts.topRow(); irow <= ts.bottomRow(); irow++){
         for ( int icol = ts.leftColumn(); icol <= ts.rightColumn(); icol++){
             if ( ++icount <= itemCopy.size() )
                to->setItem(irow, icol, new QTableWidgetItem(itemCopy[icount-1]));
         }
       }
       return true;
    }
    else if(ke->matches(QKeySequence::Cut) ){
       QString cellText; itemCopy.clear(); copyRange.clear();
       QList<QTableWidgetSelectionRange> ts = to->selectedRanges();
       if(!ts.isEmpty()) {
         for (int irow = ts.first().topRow(); irow <= ts.first().bottomRow(); irow++) {
           for(int icol = ts.first().leftColumn(); icol <= ts.first().rightColumn(); icol++) {
               QTableWidgetItem *w = to->item(irow,icol);
               if(w) cellText = w->text();
               if ( !cellText.isEmpty() ){
                  itemCopy << cellText;
               }
               else
                  itemCopy << "";
               to->setItem(irow,icol,0);
           }
         }
         copyRange = ts;
         //cout << itemCopy.join(", ").toLatin1().data() << endl;
       }
       return true;
    }
    else if(ke->matches(QKeySequence::Delete) ){
       QList<QTableWidgetSelectionRange> ts = to->selectedRanges();
       if(!ts.isEmpty()) {
         for (int irow = ts.first().topRow(); irow <= ts.first().bottomRow(); irow++) {
           for(int icol = ts.first().leftColumn(); icol <= ts.first().rightColumn(); icol++) {
               to->setItem(irow,icol,0);
           }
         }
       }
       return true;
    }
    else
        return false;
  }
  return false;
}
