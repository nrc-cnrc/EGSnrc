/*
###############################################################################
#
#  EGSnrc configuration GUI compiler headers
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


#ifndef EGS_COMPILERS_H
#define EGS_COMPILERS_H

#include <QCoreApplication>
#include <QWizardPage>
#include <QFormLayout>
#include<QLineEdit>
#include <QGroupBox>
#include <QComboBox>
#include <QPushButton>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QDialog>
#include <QFileDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
#include <QButtonGroup>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>

#include <iostream>

#include "egs_tools.h"

class QWizardPage;
class MCompiler;


class MCompilerSettings : public QDialog
{
  Q_OBJECT

public:
    MCompilerSettings(QWidget *parent, MCompiler* a_Compiler)
                     : QDialog(parent), m_Compiler(a_Compiler)
    {

    /* Top visible portion*/

    QFormLayout *topLayout = new QFormLayout;

    optEdit  = new QLineEdit; optEdit->setText(m_Compiler->optimization());optEdit->setCursorPosition(0);
    topLayout->addRow(tr("&Optimization options:"), optEdit);

    optionsEdit  = new QLineEdit; optionsEdit->setText(m_Compiler->options());
    topLayout->addRow(tr("&Standard options:"), optionsEdit);

    moreButton = new QPushButton(tr("&More"));
    moreButton->setCheckable(true); moreButton->setAutoDefault(false);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Vertical);
    buttonBox->addButton(moreButton, QDialogButtonBox::ActionRole);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    /* Bottom invisible portion*/

    extension = new QWidget;
    QFormLayout *extensionLayout = new QFormLayout;

    debugEdit  = new QLineEdit; debugEdit->setText(m_Compiler->debug());
    extensionLayout->addRow(tr("&Debug flag:"), debugEdit);

    outEdit  = new QLineEdit; outEdit->setText(m_Compiler->outflag());
    extensionLayout->addRow(tr("O&utput flag:"), outEdit);

    libEdit  = new QLineEdit; libEdit->setText(m_Compiler->libraries());
    extensionLayout->addRow(tr("&Libraries:"), libEdit);

    versionEdit = new QLineEdit; versionEdit->setText(m_Compiler->version());versionEdit->setCursorPosition(0);
    extensionLayout->addRow(tr("&Version:"), versionEdit);

    extension->setLayout(extensionLayout);

    connect(moreButton, SIGNAL(toggled(bool)), extension, SLOT(setVisible(bool)));


    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    mainLayout->addLayout(topLayout, 0, 0);
    mainLayout->addWidget(buttonBox, 0, 1);
    mainLayout->addWidget(extension, 1, 0, 1, 1);
    mainLayout->setRowStretch(2, 1);

    setLayout(mainLayout);

    setWindowTitle(tr("Settings"));

    extension->hide();

    }

    void setCompiler(MCompiler *a_Compiler){
      m_Compiler = a_Compiler;
      optEdit->setText(m_Compiler->optimization());optEdit->setCursorPosition(0);
      optionsEdit->setText(m_Compiler->options());
      debugEdit->setText(m_Compiler->debug());
      outEdit->setText(m_Compiler->outflag());
      libEdit->setText(m_Compiler->libraries());
      versionEdit->setText(m_Compiler->version()); versionEdit->setCursorPosition(0);
    }

signals:

private slots:

    void accept()
    {
      m_Compiler->setOptimization( optEdit->text() );
      m_Compiler->setOptions( optionsEdit->text() );
      m_Compiler->setDebug(debugEdit->text() );
      m_Compiler->setoutflag( outEdit->text() );
      m_Compiler->setLibs(libEdit->text());
      m_Compiler->setVersion(versionEdit->text());
      moreButton->setChecked(false);
      QDialog::accept();
    }
    void reject(){
      moreButton->setChecked(false);
      QDialog::reject();
    }

private:
    MCompiler *m_Compiler;
    QLineEdit *optEdit;
    QLineEdit *optionsEdit;
    QLineEdit *debugEdit;
    QLineEdit *outEdit;
    QLineEdit *libEdit;
    QLineEdit *versionEdit;
    QDialogButtonBox *buttonBox;
    QPushButton *moreButton, *closeButton;
    QWidget *extension;
};


class QCompilerPage : public QWizardPage
{

  Q_OBJECT

public:

  QCompilerPage(QWidget *parent, MCompiler *a_m, MCompiler *a_f,
                                  MCompiler *a_c, MCompiler *a_cpp )
                : QWizardPage(parent), settings(0), fc(a_f), cc(a_c), cpp(a_cpp), make(a_m)
  {

       setTitle("Compiler Selection Page");
       setSubTitle("Select make utility, Fortran, C, and C++ compilers");

       // the page layout
       QGridLayout *topl = new QGridLayout;

       // make utility:
       QGroupBox *gb = new QGroupBox(tr("make group box"),this);
       QVBoxLayout *gbvl = new QVBoxLayout;
       QHBoxLayout *gbl = new QHBoxLayout;
       gb->setTitle( tr("Make utility") );
       makeCB = new QComboBox(gb); makeCB->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
       gbl->addWidget(makeCB);
       QPushButton *b = new QPushButton(tr("..."),gb); b->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Preferred);
       connect(b,SIGNAL(clicked()),this,SLOT(selectMake()));
       gbl->addWidget(b); gbvl->addLayout(gbl); gbvl->addStretch(1); gb->setLayout(gbvl);

       topl->addWidget(gb,0,0);


       // Fortran compiler:
       gb = new QGroupBox("Fortran group box",this);
       gbvl = new QVBoxLayout(gb);
       gbl = new QHBoxLayout;
       gb->setTitle( tr("Fortran compiler") );
       fcCB = new QComboBox(gb); fcCB->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
       gbl->addWidget(fcCB);
       b = new QPushButton("...",gb); b->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Preferred);
       connect(b,SIGNAL(clicked()),this,SLOT(selectFortran()));
       gbl->addWidget(b); gbvl->addLayout(gbl);

       b = new QPushButton("Settings",gb); b->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
       connect(b,SIGNAL(clicked()),this,SLOT(fortranOptions()));

       gbvl->addWidget(b);

       topl->addWidget(gb,1,0,2,1);

       // C compiler:
       gb = new QGroupBox("C group box",this);
       gbvl = new QVBoxLayout(gb);
       gbl = new QHBoxLayout;//gbl->setSpacing(6); gbl->setMargin(11);
       gb->setTitle( tr("C compiler") );
       ccCB = new QComboBox(gb); ccCB->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
       gbl->addWidget(ccCB);
       b = new QPushButton("...",gb); b->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Preferred);
       connect(b,SIGNAL(clicked()),this,SLOT(selectC()));
       gbl->addWidget(b);

       gbvl->addLayout(gbl);

       b = new QPushButton("Settings",gb); b->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
       connect(b,SIGNAL(clicked()),this,SLOT(cOptions()));

       gbvl->addWidget(b);  gbvl->addStretch(1);

       topl->addWidget(gb,0,1);

       // C++ compiler:
       gb = new QGroupBox("C++ group box",this);
       gbvl = new QVBoxLayout(gb);
       gbl = new QHBoxLayout;
       gb->setTitle( tr("C++ compiler") );
       cppCB = new QComboBox(gb); cppCB->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
       gbl->addWidget(cppCB);
       b = new QPushButton("...",gb); b->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Preferred);
       connect(b,SIGNAL(clicked()),this,SLOT(selectCPP()));
       gbl->addWidget(b);

       gbvl->addLayout(gbl);

       QHBoxLayout *gbhl = new QHBoxLayout();
       b = new QPushButton("dso>>",gb); b->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
       connect(b,SIGNAL(clicked()),this,SLOT(dsoOptions()));
       gbhl->addWidget(b);

       b = new QPushButton("Settings",gb); b->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
       connect(b,SIGNAL(clicked()),this,SLOT(cppOptions()));
       gbhl->addWidget(b);

       gbvl->addLayout(gbhl);

       topl->addWidget(gb,1,1,2,1);

       this->setLayout(topl);

       setUpCompilerBoxes();

       connect(makeCB,SIGNAL(currentIndexChanged(const QString &)),this,SLOT(switchMake(const QString &)));
       connect(fcCB,  SIGNAL(currentIndexChanged(const QString &)),this,SLOT(switchFC(const QString &)));
       connect(ccCB,  SIGNAL(currentIndexChanged(const QString &)),this,SLOT(switchCC(const QString &)));
       connect(cppCB, SIGNAL(currentIndexChanged(const QString &)),this,SLOT(switchCPP(const QString &)));
  }
  ~QCompilerPage(){}

public slots:

  void initializePage(){
    makeCB->setCurrentIndex(makeCB->findText(make->name()));
    fcCB->setCurrentIndex(fcCB->findText(fc->name()));
    ccCB->setCurrentIndex(ccCB->findText(cc->name()));
    cppCB->setCurrentIndex(cppCB->findText(cpp->name()));
  }

  bool validatePage()
  {
    bool an_error = false;

    if (!make->exists()){
      errorMessage("make");
      an_error = true;
    }
    if (!fc->exists()){
      errorMessage("fc");
      an_error = true;
    }
    if (!cc->exists()){
      errorMessage("cc");
      an_error = true;
    }
    if (!fc->exists()){
      errorMessage("cpp");
      an_error = true;
    }

    return !an_error;

  }

  void errorMessage(const QString & tool){
    if (tool == "make")
       QMessageBox::critical(this, "Critical error!", "GNU make tool " + make->name() + " not found!");
    else if (tool == "fc")
       QMessageBox::critical(this, "Critical error!", "Compiler " + fc->name() + " not found!");
    else if (tool == "cc")
       QMessageBox::critical(this, "Critical error!", "Compiler " + cc->name() + " not found!");
    else if (tool == "cpp")
       QMessageBox::critical(this, "Critical error!", "Compiler " + cpp->name() + " not found!");
    else
       QMessageBox::critical(this, "Critical error!", "Unknown critical error");
  }


  /*
      find_programs_in_system( const QStringList & progs, const char & sep )

  Attempts to find programs provided in the list "progs" in any of the
  locations defined in the PATH environment variable. The separator used
  in the PATH variable must be also supplied. It returns a list of found
  programs.
  */
  QStringList find_programs_in_system( const QStringList & progs,
                                     const char & sep          )
  {
    QString pathval = getenv(QString("PATH").toLatin1());
    if ( !pathval.endsWith(sep) ) pathval.append(sep);

    //Extracting paths from environment variable PATH into a list
    QStringList dirs_in_path = pathval.split(sep);

    //Finding programs from list progs in the directory list
    QStringList progs_found;
    for ( QStringList::ConstIterator it1 = progs.begin();
                                 it1 != progs.end();
                                 ++it1 ) {
        for ( QStringList::Iterator it2 = dirs_in_path.begin();
                                it2 != dirs_in_path.end();
                                ++it2 ) {
#ifdef WIN32
            if ( fileExists(*it2 + QDir::separator() + *it1 + ".exe" ) ) progs_found += *it1;
#else
            if ( fileExists(*it2 + QDir::separator() + *it1 ) ) progs_found += *it1;
#endif
        }
    }
    progs_found.removeDuplicates();
    return progs_found;
  }

  MCompiler* getCompiler(Language l){
     QString full_name = QFileDialog::getOpenFileName(
                     this,tr("Select file"),
                     QDir::homePath(), tr("All files (*)")
     );
     QString name = full_name.right( full_name.length() - full_name.lastIndexOf(QDir::separator()) - 1 ),
             path = full_name.left(  full_name.lastIndexOf(QDir::separator()) - 1 );
     MCompiler *m = new MCompiler(l, name, path);
     if ( m->exists() ) return m;
     else return 0;
  }
  void selectMake(){
     MCompiler *m = getCompiler(GnuMake);
     if (m) {
       make = m;
       if (makeCB->findText(make->name()) == -1) makeCB->addItem(make->name());
     }
  }
  void selectFortran(){
     MCompiler *f = getCompiler(F);
     if (f){
       fc = f;
       if (fcCB->findText(fc->name()) == -1) fcCB->addItem(fc->name());
     }
  }
  void selectC(){
     MCompiler *c = getCompiler(C);
     if (c){
       cc = c;
       if (ccCB->findText(cc->name()) == -1) ccCB->addItem(cc->name());
     }
  }
  void selectCPP(){
     MCompiler *c = getCompiler(CPP);
     if (c){
       cpp = c;
       if (cppCB->findText(cpp->name()) == -1) cppCB->addItem(cpp->name());
     }
  }

  void switchFC(const QString &name){
    MCompiler c(F,name);
    if (c.exists()) *fc = c;
  }
  void switchCC(const QString &name){
    MCompiler c(C,name);
    if (c.exists()) *cc = c;
  }
  void switchCPP(const QString &name){
    MCompiler c(CPP,name);
    if (c.exists()) *cpp = c;
  }
  void switchMake(const QString &name){
    MCompiler c(GnuMake,name);
    if (c.exists()) *make = c;
  }

  void fortranOptions(){
    MCompiler l_f(*fc);
    if (!settings) settings = new MCompilerSettings(this,&l_f);
    else settings->setCompiler(&l_f);
    if (settings->exec())
       *fc = l_f;
  }

  void cOptions(){
    MCompiler l_f(*cc);
    if (!settings) settings = new MCompilerSettings(this,&l_f);
    else settings->setCompiler(&l_f);
    if (settings->exec())
       *cc = l_f;
  }
  void cppOptions(){
    MCompiler l_f(*cpp);
    if (!settings) settings = new MCompilerSettings(this,&l_f);
    else settings->setCompiler(&l_f);
    if (settings->exec())
       *cpp = l_f;
  }

  void dsoOptions(){
    QDialog *dsoDlg = new QDialog(this);
    QVBoxLayout *vl = new QVBoxLayout;
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                       QDialogButtonBox::Cancel,
                                                       Qt::Horizontal);
    connect(buttonBox, SIGNAL(accepted()), dsoDlg, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), dsoDlg, SLOT(reject()));
    QFormLayout *dsoLayout = new QFormLayout;

    QLineEdit *definesEdit  = new QLineEdit; definesEdit->setText(cpp->defines());
    dsoLayout->addRow(tr("Preprocessor &defines:"), definesEdit);

    QLineEdit *fpicEdit  = new QLineEdit; fpicEdit->setText(cpp->fpic());
    dsoLayout->addRow(tr("Position independent code (PIC) flag:"), fpicEdit);

    QLineEdit *sharedEdit  = new QLineEdit; sharedEdit->setText(cpp->shared());
    dsoLayout->addRow(tr("Shared library flag:"), sharedEdit);

    QLineEdit *dlopenEdit  = new QLineEdit; dlopenEdit->setText(cpp->extra());
    dsoLayout->addRow(tr("Output:"), dlopenEdit);
    QLineEdit *dsoPathEdit  = new QLineEdit; dsoPathEdit->setText(cpp->dsoPath());
    dsoLayout->addRow(tr("DSO path encoded in the executable:"), dsoPathEdit);
    QLineEdit *linkingEdit  = new QLineEdit;
    linkingEdit->setText( cpp->LinkPrefix() + "some_lib" + cpp->LinkSuffix() );
    dsoLayout->addRow(tr("Linking against library \"some_lib\":"), linkingEdit);
    QLineEdit *flibsEdit  = new QLineEdit; flibsEdit->setText(cpp->flibs());
    dsoLayout->addRow(tr("Fortran libraries needed by C++ linker:"), flibsEdit);

    definesEdit->setCursorPosition(0); fpicEdit->setCursorPosition(0);
    sharedEdit->setCursorPosition(0);  dlopenEdit->setCursorPosition(0);
    dsoPathEdit->setCursorPosition(0); linkingEdit->setCursorPosition(0);
    flibsEdit->setCursorPosition(0);

#if defined(Q_OS_MAC) || defined(Q_OS_DARWIN)
    QLineEdit *bundleEdit  = new QLineEdit; bundleEdit->setText(cpp->bundleOSX());
    dsoLayout->addRow(tr("Creating library bundles in OSX:"), bundleEdit);
    bundleEdit->setCursorPosition(0);
#endif

    vl->addLayout(dsoLayout); vl->addWidget(buttonBox);

    dsoDlg->setLayout(vl);

    if ( dsoDlg->exec() ){
       cpp->setDefines(definesEdit->text());
       cpp->setShared(sharedEdit->text());
       cpp->setfPIC(fpicEdit->text());
       cpp->setExtra(dlopenEdit->text());
       cpp->setDSOPath(dsoPathEdit->text());
       cpp->setLinkPrefixSuffix(linkingEdit->text());
       cpp->setFlibs(flibsEdit->text());
    }

  }

QString getVar(const QString & var) { return QString(getenv(var.toLatin1())); }

signals:

  void egsHomeChanged(const QString &);
  void henHouseChanged(const QString &);

private:

  void setUpCompilerBoxes(){

    QStringList makeprog, fcompiler, ccompiler, cppcompiler;
#ifdef WIN32
    makeprog << "make"          // GNU make
             << "gmake"         // GNU make
             << "mingw32-make"; // MinGW make

    fcompiler << "gfortran" << "mingw32-gfortran" // GNU Fortran 95
              << "x86_64-w64-mingw32-gfortran" << "i686-w64-mingw32-gfortran"
              <<  "g77"     << "mingw32-g77"      // GNU Fortran 77
              << "f77"             // LAHEY Fortran 77
              << "lf95"            // LAHEY Fortran 95
              << "df"              // DEC Compaq Fortran 90/95
              << "fl32"            // MS Fortran 77
              << "ifl"             // Intel Fortran < 8.0
              << "ifort"           // Intel Fortran >= 8.0
              << "pgf77" << "pgf90" << "pgfortran"; // Portland Group Fortran 77, 90 and 2003 Compilers

    ccompiler << "mingw32-gcc"  // GNU C
              <<"gcc"           // GNU C
              <<"cl"            // MS C/C++ compiler
              <<"pgcc"          // Portland Group C compiler
              <<"fcc";           // Fujitsu C compiler

    cppcompiler << "g++"
                << "g++4"
                << "mingw32-g++"
                << "cl" << "pgc++";

    const char* sep = ";";
#else
    makeprog << "make"
             << "gmake";

    fcompiler << "g77"        // GNU Fortran 77 compiler
              << "g95"        // GNU Fortran 95 Compiler
              << "gfortran"   // GNU Fortran 95 Compiler
              << "f77"        // LAHEY Fortran 77 Compiler
              << "lf95"       // LAHEY Fortran 95 Compiler
              << "pgf77"      // Portland Group Fortran 77 Compiler
              << "pgf90"      // Portland Group Fortran 90 Compiler
              << "pgfortran"  // Portland Group Fortran 2003 Compiler
              << "ifort"      // Intel Fortran >= 8.0
              << "xlf"
              << "frt" << "af77"
              << "fort77" << "f90" << "xlf90" << "epcf90"
              << "f95" << "fort" << "xlf95" << "asf77";

    ccompiler << "gcc"        // GNU C
              << "cc" << "xlc" << "cl" << "c89" << "c90" << "c99"
              << "icc";

    cppcompiler << "g++" << "g++4"
                << "icpc" << "aCC" << "CC" << "cxx"
                << "cc++" << "FCC" << "KCC"<< "RCC"
                << "xlC_r"<< "xlC" << "gpp"<< "cl";

    const char* sep = ":";
#endif

    makeprog = find_programs_in_system( makeprog , *sep );
    if ( makeCB->count() ) makeCB->clear();
    if (!makeprog.isEmpty()) makeCB->addItems(makeprog);

    fcompiler = find_programs_in_system( fcompiler , *sep );
    if ( fcCB->count() ) fcCB->clear();
    if (!fcompiler.isEmpty()) fcCB->addItems(fcompiler);

    ccompiler = find_programs_in_system( ccompiler , *sep );
    if ( ccCB->count() ) ccCB->clear();
    if (!ccompiler.isEmpty()) ccCB->addItems(ccompiler);

    cppcompiler = find_programs_in_system( cppcompiler , *sep );
    if ( cppCB->count() ) cppCB->clear();
    if (!cppcompiler.isEmpty()) cppCB->addItems(cppcompiler);

  }


  MCompilerSettings *settings;
  MCompiler     *fc, *cc, *cpp, *make; // Compilers + make utility
  QComboBox     *makeCB,
                *fcCB,
                *ccCB,
                *cppCB;
};

#endif