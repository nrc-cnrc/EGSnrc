/*
###############################################################################
#
#  EGSnrc egs_inprz implementation headers
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
#  Contributors:    Iwan Kawrakow
#                   Blake Walters
#                   Cody Crewson
#                   Reid Townson
#
###############################################################################
*/


#ifndef INPUTRZIMPL_H
#define INPUTRZIMPL_H

#include "pegslessinputs.h"

#include "ui_inputRZ.h"

#include "datainp.h"
#include "inputs.h"
#include "eventfilter.h"
#include "beamsrcdlg.h"
#include <qtooltip.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <QLabel>

#include <QTextStream>

std::ifstream & operator >> ( std::ifstream & in, MInputRZ * r );
template <class X>
void get_col_content( const int &col, QTableWidget* t, std::vector<X> &result );

template <class X>
void get_col_explicit( const int &col, QTableWidget* t, std::vector<X> &result, X def);

template <class X>
void update_table( std::vector<X> *v, int ini, int count, QTableWidget* t );

template <class X>
std::vector<X> strip_repetitions( std::vector<X> v );

template <class X>
std::vector<X> del_element( std::vector<X> v, X e );


//!  Implementation of the Graphical User Interface class.
/*!
  This class inherits the original form (tab dialog)  and gives
  life to the different GUI components. All the GUI members are
  initialized and their interactions (signal-slot) defined.
  Each page of the tabbed dialog corresponds to one of the
  input blocks for the EGSnrc user codes: DOSRZnrc, CAVRZnrc,
  SPRRZnrc and FLURZnrc.
*/
class inputRZImpl : public QWidget, public Ui::InputRZForm
{
  Q_OBJECT

public:

inputRZImpl( QWidget* parent, const char* name,
             bool modal, Qt::WindowFlags f );

~inputRZImpl();

QString confErrors;
QString formErrors;
QString openErrors;
QString pegsErrors;
QString geoErrors;
QString previewErrors;
QString EGSfileName;
QString PEGSfileName;
QString EGSdir;
QString PEGSdir;
QString SPECdir;
QString CONFdir;
QString RDISTdir;
QString PHSPdir;
QString HEN_HOUSE;
QString HOME;
QString EGS_HOME;
QString EGS_CONFIG;
QString GUI_HOME;
QString usercodename;
QString The_Other_PEGS;
QString The_Other_Area;

bool egs_run_exists;
bool pprocess_exists;
bool previewRZ_exists;
bool egs_dir_changed;
bool pegs_dir_changed;
bool egs_compile_exists;
bool is_pegsless;
UserCodeType usercode;

bool    run_parallel;
int     num_jobs;
int     ini_job;
QString queue;

//public access stuff for PEGSless inputs
PEGSLESSInputs* Ppgls;

// a push button for the beam source

QPushButton* beamButton;

// Application clipboard
QClipboard *clipBoard;

//event filter for media definition combo box
ComboEvents *im_events;

v_string listMedia;

void Initialize();
void UpDateInputRZForm(  const MInputRZ*    Input );
void update_IOControl  ( const MIOInputs*   EGSio );
void update_MCInputs   ( const MMCInputs*   EGSmc );
void update_GEOInputs  ( const MGEOInputs*  EGSgeo );
void update_PHDInputs  ( const MPHDInputs*  EGSphd );
void update_CAVInputs  ( const MCAVInputs*  EGScav );
void update_SRCInputs  ( const MSRCInputs*  EGSsrc );
void update_MCTParam   ( const MMCPInputs*  EGSmcp );
void update_PEGSLESSParam   ( const PEGSLESSInputs*  EGSpgls );
void update_VarParam   ( const MVARInputs*  EGSvar );
void update_PlotControl( const MPLOTInputs* EGSplot );

void updateConfiguration( const QString & conf );

void clear_table( QTableWidget* t );
void clear_col( QTableWidget* t, int col);
int  Add_New_Item( const char* ItemName, QComboBox* cb );
int  Get_Item_Index( const char* ItemName, QComboBox* cb );
void validate_combo( const char* entry, QString error, QComboBox* cb );
void validate_radio( const char* entry, QString error, int count, QRadioButton **r );
void SetValidator( );
void rearrange_media( MGEOInputs* geo, v_string* med );

void open();
MInputRZ*       GetInputRZ();
MTitle*         GetTitle();
MIOInputs*      GetIO();
MMCInputs*      GetMC();
MGEOInputs*     GetGEO();
MPHDInputs*     GetPHD();
MCAVInputs*     GetCAV();
MSRCInputs*     GetSRC();
MMCPInputs*     GetMCP();
PEGSLESSInputs* GetPEGSLESS();
MVARInputs*     GetVAR();
MPLOTInputs*    GetPLOT();
void save();


int TotalTextLines( const QString& fname);

void update_mediaTable( const MGEOInputs* EGSgeo );
void fill_media_table( const MGEOInputs* EGSgeo );
void print_delimeter( const char* boundary , const char* section, QTextStream &t );
void SetInitialDir();
void DeactivateTable( QTableWidget* table );
void InitializeTwoColumnTable( QTableWidget* table);
void InitializeThreeColumnTable( QTableWidget* table, const QString& rvalue);
void InitializeTable( QTableWidget* t, const QStringList& s);
void InitializeTable( QTableWidget* t, const QStringList& s, v_float frac);
void InitializeTable( QTableWidget* t, const QString& s0, const QString& s1);
void InitializeTable( QTableWidget* t, const QString& s0, const QString& s1, const QString& s2);

void InitializePhotonXSection();
void InitializeEIIXSection();

UserCodeType GetUserCode();

QString  getExecutable();
QString  readVarFromConf( const QString& var );
QString  expandEnvVar( const QString& dirStr );
QString  find_usercode_name( const QString& dir );
QString  get_initial_usercode_area( QString* name );
QString  GetCurrentDir( const QString& rCodeName, const QString& rHome, const QString& rHenHouse );
QString  GetPEGSDir( const QString& rCodeName, const QString& rHome, const QString& rHenHouse );
QString  GetUserCodeDir( const QString& rCodeName);
QString  TextRadioBChecked( int count, QRadioButton **r );
QString  FToQStr( float Item );
QString  IntToQStr( int Item );
QStringList StrListToQStrList( v_string Item );

v_int    assign_medium_number(v_string med_list, v_string med_entry);
v_string getPEGSMedia( const QString& fname );
v_string getPEGSLESSMedia();
void     update_from_user_area();
void     update_from_data_area();
void     updateMediaLists();
void     update_files( const QString & rDirName, QComboBox* cb, const QString & rFilter );
void     update_EGSdir( const QString& newDir );
void     update_caption( const QString& str );

bool pegs_is_ok( QString fname );
bool pegsless_is_ok();
bool IsByRegionsEnabled( QComboBox* cb, QTableWidget* table );
bool IsByRegionsEnabled( QCheckBox* chk, QTableWidget* table );
bool IsByRegionsEnabled( QRadioButton* rbOn, QRadioButton* rbOff, QTableWidget* table );
bool configLibExists();
void SetInpfileName( QString inp_name );
void update_conf_files();

void reset_mediaTable();
void reset_customFFTable();
bool GetMedFromDCfile(QString f);

public slots:
    virtual void activate_fluence_table();
    virtual void activate_ff_table();
    virtual void update_source_type();
    virtual void update_usercode();
    virtual void set_cavity();
    virtual void set_group();
    virtual void set_individual();
    virtual void set_local_external();
    virtual void set_mono_spectrum();
    virtual void update_SprOutTableHeaders();
    virtual void update_MediaInput();
    virtual void mediaTable_clicked(int row, int col);
    virtual void mediaTable_singleclicked(int row, int col);
    virtual void enableDCfileInput(bool enable);
    virtual void medTypeChanged(const QString& str);
    virtual void pz_or_rhozTable_clicked(int row, int col);
    virtual void pz_or_rhozTable_singleclicked(int row, int col);
    virtual void customFFTable_clicked(int row, int col);
    virtual void customFFTable_singleclicked(int row, int col);
    virtual void UpDateInputRZFile();
    virtual void activate_PulseHDistInputs();
    virtual void OpenEGSInpFile();
    virtual void GetMDfile();
    virtual void GetDFfile();
    virtual void GetDFfileReturn();
    virtual void set_table_header_pbw();
    virtual void set_table_header_noa();
    virtual void enable_gaspEdit();
    virtual void inpmediumChanged(const QString& str);
    virtual void inpmediumSave(const QString& str);
    virtual void GetPEGSfile();
    virtual void GetSPECfile();
    virtual void GetRDISTfile();
    virtual void GetPHSPfile();
    virtual void getCONFFile();
    virtual void run_previewRZ();
    virtual void run_userCode();
    virtual void compile_userCode();
    virtual void set_cav_regions();
    virtual void set_electr();
    virtual void EnableTransportParamByRegions();
    virtual void Enable_BremsSplit();
    virtual void enable_plot();
    virtual void enable_external_plot();
    virtual void set_working_area();
    virtual void set_data_area();
    virtual void show_errors();
    virtual void show_help();
    virtual void show_about();
    virtual void checkErrors();
    virtual void checkErrors( const QString& fun );
    virtual void cleanChecked( bool status );
    virtual void change_pegs4_name();
    virtual void change_input_file();
    virtual void change_config_file();
    virtual void EGSFileNameChanged( const QString& str );
    virtual void PEGSFileNameChanged( const QString& str );
    virtual void update_range_rejection( );
    virtual void update_photon_forcing( );
    virtual void update_usercode_open();
    virtual void setupDefaultSettingsEditor();
    virtual void configure();
            void print();
            void setupBeamSource();
            void getBeamSource();
            void updateBeamSource(const QString& uc,
                                  const QString& pegs);

            void checkConfigLib();
            void checkExecutionAbility();
            void checkCompilationAbility();
            void checkPreviewRZ();
            void checkPrintAbility();
            void caught_errors();
            bool isTclTkInstalled();

protected:
  bool        eventFilter(QObject *obj, QEvent *ev);
  QStringList itemCopy;
  QList<QTableWidgetSelectionRange> copyRange;

private:

// needed to update geometry and media tables properly

    v_int globalNSlab;
    QString current_description_by;

    QString the_year;

    BeamSourceDlg* beamDlg;
    QLabel* beamLabel;

    v_string getMediaFromTable();

};

#endif
