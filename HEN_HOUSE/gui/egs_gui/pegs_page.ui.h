/*
###############################################################################
#
#  EGSnrc user interface headers for egs_gui pegs page
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
#                   Ernesto Mainegra-Hing
#
###############################################################################
*/


/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include <qstringlist.h>
#include <qlistbox.h>
#include <qpopupmenu.h>
#include <qapplication.h>
#include <qfiledialog.h>
#include <qdir.h>
#include <qprocess.h>
#include <qmessagebox.h>
#include <qregexp.h>

#include "element_item.h"
#include "egs_gui_widget.h"
#include "egs_config_reader.h"

QStringList *elements = 0;
int n_pegs_page_objects = 0;

HelperObject *helper = 0;
TableEventHandler *tfilter = 0;

QPopupMenu  *menue = 0;

const double rm=0.5110034;

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
#ifdef IK_DEBUG
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
  new_data_file->setChecked(true);
  cancel_button->setEnabled(false);

  pegs_process = new QProcess;
  connect(pegs_process,SIGNAL(readyReadStdout()),this,SLOT(readPegsStdout()));
  connect(pegs_process,SIGNAL(readyReadStderr()),this,SLOT(readPegsStderr()));
  connect(pegs_process,SIGNAL(processExited()),this,SLOT(pegsFinished()));
  connect(pegs_process,SIGNAL(launchFinished()),this,SLOT(launchReturned()));

  run_output = new PEGS_RunOutput(0,"PEGS output window");
  connect(run_output,SIGNAL(windowClosed()),this,
       SLOT(outputClosed()));

  composition_table->adjustColumn(0);
  composition_table->adjustColumn(1);
#ifdef IK_DEBUG
  qDebug("ctable->className() = %s",composition_table->className());
#endif
  //composition_table->adjustColumn(2);
  //composition_table->setCurrentCell(0,3);
  if( helper == 0 ) {
    helper = new HelperObject; helper->setTable(composition_table);
  }
  if( menue == 0 ) {
    menue = new QPopupMenu(this);
    //menue->insertItem("&Delete",helper,SLOT(deleteRow()));
    menue->insertItem("Cut",helper,SLOT(deleteSelection()));
    menue->insertItem("Copy",helper,SLOT(copySelection()));
    menue->insertItem("Paste",helper,SLOT(pasteSelection()));
  }
  if( tfilter == 0 ) {
    tfilter = new TableEventHandler(composition_table,"table event filter");
    //composition_table->installEventFilter(tfilter);
  }
  if( elements == 0 ) {
    elements = new QStringList;
    for(int j=0; j<n_element; j++) (*elements) += element_data[j].symbol;
  }
  /*
  if( element_list == 0 ) {
    //element_list = new QListBox(0,"Element list box");
    element_list = new JunkListBox(0,"Element list box");
    qDebug("Created elist at 0x%x",element_list);
    for(int j=1; j<=n_element; j++) {
      QString aux; aux.setNum(j); QString aux1;
      if( j < 10 ) aux1 = "  " + aux;
      else if( j < 100 ) aux1 = " " + aux;
      else aux1 = aux;
      aux = element_data[j-1].symbol;
      if( aux.length() == 1 ) aux1 += "   " + aux;
      else aux1 += "  " + aux;
      element_list->insertItem(aux1);
    }
  }
  */
  int j;
  for(j=0; j<20; j++) {
    ElementItem *ci = new ElementItem(composition_table,
                             QTableItem::WhenCurrent );
    composition_table->setItem(j,0,ci);
    composition_table->setRowReadOnly(j,false);
    //if( j > 0 ) composition_table->setRowReadOnly(j,true);
  }
  // There is a bug on the Windows version of Qt that makes
  // the first ElementItem set to none (instead of empty)
  // and not being reset when reading a density correction file,
  // unless we make the table editable first.
  dc_icru_check->setChecked(false);
  dc_icru_check->setChecked(true);
  for(j=0; j<20; j++) {
    composition_table->setText(j,0,"");
    composition_table->setText(j,1,"");
  }
  n_pegs_page_objects++;
}

void EGS_PegsPage::tableCellChanged( int row, int col)
{
  qWarning("In EGS_PegsPage::tableCellChanged: %d %d",row,col);
  QTableItem *item = composition_table->item(row,col);
#ifdef IK_DEBUG
  if( item ) qDebug("Item is not null %x",item); else qDebug("Item is null");
#endif
  if( col == 0 && !item ) {
    composition_table->setItem(row,col,
        new QComboTableItem(composition_table,*elements));
  }
}


void EGS_PegsPage::dencityIcruChanged( bool is_on) {
#ifdef IK_DEBUG
  qDebug("EGS_PegsPage::dencityIcruChanged: %d",is_on);
#endif
  composition_table->setEnabled( !is_on );
  dc_group->setEnabled( is_on );
  rho_group->setEnabled( !is_on );
  medtype_cbox->setEnabled( !is_on );
}


void EGS_PegsPage::cellClicked( int row, int col, int button, const QPoint &p )
{
#ifdef IK_DEBUG
  qDebug("In cellClicked(%d,%d,%d,%d,%d)",row,col,button,p.x(),p.y());
#endif
  if( button == Qt::RightButton ) {
    menue->exec(p);
  }
}

void EGS_PegsPage::cellDoubleClicked( int row, int col, int button, const QPoint &p )
{
#ifdef IK_DEBUG
  qDebug("In cellDoubleClicked(%d,%d,%d,%d,%d)",row,col,button,p.x(),p.y());
#endif
}

void EGS_PegsPage::cellPressed( int row, int col, int button, const QPoint &p )
{
  QPoint pp = composition_table->mapToGlobal(p);
#ifdef IK_DEBUG
  qDebug("In cellPressed(%d,%d,%d,%d,%d)",row,col,button,p.x(),p.y());
  qDebug("global: %d,%d",pp.x(),pp.y());
#endif
  if( button == Qt::RightButton ) {
    //QPoint p1 = composition_table->pos(); p1 += p;
    menue->exec(pp);
  }
}


void EGS_PegsPage::medtypeChanged( const QString &s )
{
#ifdef IK_DEBUG
  qDebug("In EGS_PegsPage::medtypeChanged(%s)",s.latin1());
#endif
  if( s == "Compound" ) composition_table->horizontalHeader()->setLabel(1,"Stoichiometric index");
  else composition_table->horizontalHeader()->setLabel(1," Fraction by weight ");
  composition_table->adjustColumn(1);
  bool ro = s == "Element";
  for(int j=1; j<20; j++) {
    composition_table->setRowReadOnly(j,ro);
    if( ro ) {
      composition_table->setText(j,0,"");
      composition_table->setText(j,1,"");
    }
  }
}


void EGS_PegsPage::getDensityFile() {
#ifdef IK_DEBUG
  qDebug("In EGS_PegsPage::getDensityFile()");
#endif
  QString start_dir;
  if( !config_reader ) config_reader = new EGS_ConfigReader;
  if( dc_where->currentText() == "HEN_HOUSE" )
    start_dir = config_reader->getVariable("HEN_HOUSE",true);
  else start_dir = config_reader->getVariable("EGS_HOME",true);
  start_dir += "pegs4"; start_dir += QDir::separator();
  start_dir += "density_corrections";
  QString dfile = QFileDialog::getOpenFileName(start_dir,
    "Density correction files (*.density)",this,"density file dialog",
    "Select a density correction file");
#ifdef IK_DEBUG
  qDebug("dfile = %s",dfile.latin1());
#endif
  if( dfile.isEmpty() ) return;
  QFileInfo fi(dfile);
  QFile f(dfile);
  if( !f.open(IO_ReadOnly ) ) return;
  //Q_INT8 c;
  //while ( !data.atEnd() ) {
  //  data >> c; if ( c == '\n' ) break;
  //  qDebug("Got %d",c);
  //}
  char buf[256]; f.readLine(buf,255); // ignore first line
#ifdef IK_DEBUG
  qDebug("got line: %s",buf);
#endif
  //f.readLine(buf,255); qDebug("got line: %s",buf);
  QTextStream data(&f);
  int ndat; double Iev, rho;
  data >> ndat >> Iev >> rho >> nelem;
#ifdef IK_DEBUG
  qDebug(" ndat = %d Iev = %lf rho = %lf nelem = %d",ndat,Iev,rho,nelem);
#endif
  if( nelem == 1 ) {
    medtype_cbox->setCurrentText("Element");
    medtypeChanged("Element");
  }
  else {
    medtype_cbox->setCurrentText("Mixture");
    medtypeChanged("Mixture");
  }
  //composition_table->horizontalHeader()->setLabel(1," Fraction by weight ");
  rho_le->setText(QString("%1").arg(rho));
  int j;
  for(j=0; j<20; j++) {
    composition_table->setText(j,0,"");
    composition_table->setText(j,1,"");
  }
  for(j=0; j<nelem; j++) {
    int iz; double frac; data >> iz >> frac;
    composition_table->setText(j,0,element_data[iz-1].symbol);
    composition_table->setText(j,1,QString("%1").arg(frac));
    //composition_table->setRowReadOnly(j,false);
  }
  dc_file->setText(fi.baseName(true));
}

void EGS_PegsPage::newDataFileChecked(bool b) {
#ifdef IK_DEBUG
  qDebug("In EGS_PegsPage::newDataFileChecked(%d)",b);
#endif
  append_to_datafile->setChecked(!b);
}

void EGS_PegsPage::appendDataFileChecked(bool b) {
#ifdef IK_DEBUG
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
    s = QFileDialog::getOpenFileName(start_dir,
    "PEGS files (*.pegs4dat)",this,"pegs save file dialog",
    "Select a PEGS file");
  else
    s = QFileDialog::getSaveFileName(start_dir,"PEGS files (*.pegs4dat)",this,
        "Select a PEGS file");
  if( !s.isEmpty() ) {
    QFileInfo fi(s);
    ofile_le->setText(fi.baseName());
  }
}

void EGS_PegsPage::startPegs() {
#ifdef IK_DEBUG
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
  pegs_process->clearArguments();
  pegs_process->addArgument(executable);
  pegs_process->addArgument("-e");
  pegs_process->addArgument(config_reader->getVariable("EGS_HOME",true));
  pegs_process->addArgument("-h");
  pegs_process->addArgument(config_reader->getVariable("HEN_HOUSE",true));
  pegs_process->addArgument("-o");
  pegs_process->addArgument(ofile_le->text());
  run_output->setOutputFile(ofile_le->text());
  if( append_to_datafile->isChecked() )
    pegs_process->addArgument("-a");
  if( dc_icru_check->isChecked() ) {
    pegs_process->addArgument("-d");
    pegs_process->addArgument(dc_file->text());
  }
  QStringList list = pegs_process->arguments();
#ifdef IK_DEBUG
  qDebug("Executing: <%s>",list.join(" ").latin1());
#endif

  // PEGS input
  QString input; QTextStream ts( &input, IO_WriteOnly );
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
      double w = composition_table->text(j,1).toDouble(&is_ok);
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
  if( is_gas->isChecked() ) ts << ",GASP=1.0";
  ts << ",EPSTFL=" << dc_icru_check->isChecked() << ",IAPRIM="
     << rad_icru_check->isChecked() << ",IRAYL=" << rayleigh_check->isChecked()
     << " &END\n";
  ts << medname_le->text() << "\n";
  for(int j=0; j<nelem; j++) {
    QString e = composition_table->text(j,0).upper();
    if( j > 0 ) ts << " ";
    ts << e; if( e.length() == 1 ) ts << " ";
  }
  ts << "\n";
  ts << "PWLF\n &INP  &END\nDECK\n &INP  &END\n";
#ifdef IK_DEBUG
  qDebug("Input string:\n%s",input.latin1());
#endif

  run_output->clearOutput();
  pegs_process->launch(input);

  go_button->setEnabled(false);
  cancel_button->setEnabled(true);
}

void EGS_PegsPage::stopPegs() {
#ifdef IK_DEBUG
  qDebug("In EGS_PegsPage::stopPegs()");
#endif
  go_button->setEnabled(true);
  cancel_button->setEnabled(false);
}

void EGS_PegsPage::readPegsStdout() {
#ifdef IK_DEBUG
  qDebug("In EGS_PegsPage::readPegsStdout()");
#endif
  QString tmp = pegs_process->readStdout();
  run_output->insertText(tmp);
}

void EGS_PegsPage::readPegsStderr() {
#ifdef IK_DEBUG
  qDebug("In EGS_PegsPage::readPegsStderr()");
#endif
  QString tmp = pegs_process->readStderr();
  run_output->insertText(tmp);
}

void EGS_PegsPage::pegsFinished() {
#ifdef IK_DEBUG
  qDebug("In EGS_PegsPage::pegsFinished()");
#endif
  if( pegs_process->normalExit() && pegs_process->exitStatus() == 0 )
    QMessageBox::information(this,"PEGS finished","PEGS finished successfuly",
       QMessageBox::Ok);
  else
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
      if( composition_table->text(j,0).isEmpty() ||
          composition_table->text(j,1).isEmpty() ) break;
      nelem = j+1;
    }
#ifdef IK_DEBUG
    qDebug("nelem = %d type = %s",nelem,medtype_cbox->currentText().latin1());
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
#ifdef IK_DEBUG
  qDebug("In EGS_PegsPage::launchReturned()");
#endif
}
