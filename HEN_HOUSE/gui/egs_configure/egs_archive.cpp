/*
###############################################################################
#
#  EGSnrc configuration GUI self-extracting archive
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
#  Contributors:    Ernesto Mainegra-Hing
#
###############################################################################
*/


#include "egs_archive.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>

#include <zlib.h>

#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qapplication.h>
#include <qmutex.h>
#include <qregexp.h>

#define the_marker "THE_END-DAS_ENDE-FIN"

using namespace std;

const char *part1 = "____";
const char *part2 = "egsnrc";
const char *part3 = "xp";
const char *part4 = "archive";

#ifdef WIN32
const char dir_sep = 92;
#else
const char dir_sep = '/';
#endif

QMutex *the_mutex = 0;

class PrivateArchive {
public:
  PrivateArchive( QWidget* o, const int &totS );
  ~PrivateArchive() {
    if( !zip_block ) delete [] zip_block;
    for(unsigned int j=0; j<outputs.size(); j++) {
      fstream *o = outputs[j]; o->close(); delete o;
    }
  }

  string marker;
  vector<string> files;
  vector<fstream *> outputs;
  vector<string> output_files;
  string ziped_content;

  void setIODevice( QTextEdit*  io_dev );
  void printProgress( const QString& s )const;
  void printProgress(const float & frac)const;
  void printProgress();
  void increaseSteps();
  bool zip(unsigned int);
  bool putAll(unsigned int);
  int  make();
  int  extract(const char *a, const char *start) const;
  void stop();
  bool isZipFile(const char *archive) const;
  int  listFiles(const char *a) const;
  bool checkDir(const char *) const;
  inline void swapBytes(unsigned char *b) const {
    unsigned char c; c = b[0]; b[0] = b[3]; b[3] = c;
    c = b[1]; b[1] = b[2]; b[2] = c;
  }

  unsigned int orig_size, ziped_size;
  int steps, totSteps;
  Bytef       *zip_block;
  char         endian;
  QTextEdit*   monitor;
  private:
  bool m_exit;
  QWidget* receiver;
};

PrivateArchive::PrivateArchive( QWidget* o, const int &totS ):
steps(0), totSteps(totS), zip_block(0), monitor(0), m_exit(false), receiver(o)
{
  marker = part1; marker += part2; marker += part1; marker += part3;
  marker += part1; marker += part4; marker += part1;
  int nl = 0x12345678;
  unsigned char *p = (unsigned char *)(&nl);
  if ( p[0] == 0x12 && p[1] == 0x34 && p[2] == 0x56 && p[3] == 0x78 )
    endian = 0;
  else {
    if ( p[0] == 0x78 && p[1] == 0x56 && p[2] == 0x34 && p[3] == 0x12 )
       endian = 1;
    else {
      cout << "Unsupported endiannes\n"; exit(1);
    }
  }
//  if( endian == 0 ) cout << "Machine is big endian\n";
//  else cout << "Machine is little endian\n";
}

EGS_Archive::EGS_Archive(QWidget* o):
receiver(o)
{
  p = new PrivateArchive( receiver, 1 );
}

EGS_Archive::EGS_Archive(QMutex *m, QWidget* o, const int & totS):
receiver(o)
{
//cout << "# of archives = "<< totS << endl;
  the_mutex = m;
  p = new PrivateArchive( receiver, totS );
}

EGS_Archive::~EGS_Archive() {
  delete p;
}

void EGS_Archive::clearFiles() {
  p->files.erase(p->files.begin(),p->files.end());
}

void EGS_Archive::closeOutput(const char *oname) {
  unsigned int osize = p->output_files.size();
  for(unsigned int j=0; j<osize; j++) {
    if( p->output_files[j] == oname ) {
      p->output_files[j] = p->output_files[osize-1];
      fstream *o = p->outputs[j]; o->close(); delete o;
      p->outputs[j] = p->outputs[osize-1];
      p->output_files.pop_back(); p->outputs.pop_back();
      return;
    }
  }
}

void EGS_Archive::addOutput(const char *oname, int type) {
  fstream *o; int msize = p->marker.size()+1;
  if( type == 0 ) {
    o = new fstream(oname, ios::out | ios::binary | ios::trunc);
    o->write(p->marker.c_str(),msize);
    o->write(&p->endian,1);
  }
  else {
    ifstream in(oname, ios::binary);
    if( !in ) {
      cout << "Failed to open " << oname << " ?"; exit(1);
    }
    bool has_marker = false; int msize = p->marker.size()+1;
    //cout << "msize = " << msize << endl;
    char *buf = new char [msize];
    while ( !in.eof() ) {
      in.read(buf,msize); buf[msize-1] = 0;
      //cout << "got <" << buf << ">\n";
      if( p->marker == buf ) { has_marker = true; break; }
    }
    o = new fstream(oname, ios::out | ios::binary | ios::app );
    //cout << "seting position to " << in.tellg() << endl;
    o->seekp(in.tellg(),ios::beg);
    if( !has_marker) {
      unsigned int fsize = in.tellg();
      int nn = fsize/msize + 1;
      int npad = nn*msize - fsize; char c = 0;
      //cout << "padding " << npad << " chars\n";
      for(int j=0; j<npad; j++) o->write(&c,1);
      o->write(p->marker.c_str(),msize);
      o->write(&p->endian,1);
    }
    //else cout << "has marker\n";
  }
  p->outputs.push_back(o);
  p->output_files.push_back(oname);
}

void EGS_Archive::addFile(const char *fname) {
  p->files.push_back(fname);
}

int EGS_Archive::make() {
  return p->make();
}

int PrivateArchive::make() {
  for(unsigned int j=0; j<files.size(); j++) {
    if( zip(j) ) {
      if( !putAll(j) ) {
        cout << "Failed to write to archives\n"; return 2;
        printProgress( (QString)"\nFailed to write to archives\n");
      }
    } else {
      //cout << "Failed to zip file " << files[j] << endl;
      printProgress( (QString)"\nFailed to zip file " + (QString)files[j].c_str() + (QString)"\n");
      return 1;
    }
  }
  return 0;
}

bool PrivateArchive::zip(unsigned int j) {
  ifstream in(files[j].c_str(),ios::binary);
  if( !in ) {
    //cout << "Failed to open file " << files[j] << endl;
    printProgress( (QString)"\nFailed to open file " + (QString)files[j].c_str() + (QString)"\n");
    return false;
  }
  in.seekg (0, ios::end);
  orig_size = in.tellg();
  in.seekg (0, ios::beg);
  Bytef *zip_input = new Bytef [orig_size];
  in.read((char*)zip_input,orig_size);
  in.close();
  ziped_size = (unsigned int) (1.001*orig_size + 12);
  if( !zip_block ) { delete [] zip_block; zip_block = 0; }
  zip_block = new Bytef [ziped_size];
  int err;
  uLongf the_ziped_size = ziped_size;
  if( (err = compress2(zip_block,&the_ziped_size,zip_input,orig_size,9) ) != Z_OK){
    //cout << "Failed to zip file " << files[j] << endl;
    printProgress( (QString)"\nFailed to zip file " + (QString)files[j].c_str() + (QString)"\n");
    if (err == Z_MEM_ERROR) cout << "not enough memory\n";
    else if (err == Z_BUF_ERROR ) cout << "not enough room in the output buffer\n";
    else if (err == Z_STREAM_ERROR) cout << "level parameter is invalid\n";
    else cout << "Unknown error\n";
    printProgress((QString)"See log file for more details ...\n");
    return false;
  }
  ziped_size=the_ziped_size;
  return true;
}

bool PrivateArchive::putAll(unsigned int j) {
  if( !zip_block ) return false;
  for(unsigned int i=0; i<outputs.size(); i++) {
    fstream *o = outputs[i];
    o->write((char *)&ziped_size, sizeof(unsigned int));
    o->write((char *)&orig_size, sizeof(unsigned int));
    int nl = files[j].size()+1;
    o->write((char *)&nl, sizeof(int));
    o->write(files[j].c_str(),nl);
    o->write((char *)zip_block, ziped_size);
  }
  delete [] zip_block; zip_block = 0;
  return true;
}

int EGS_Archive::extract(const char *archive, const char *sdir) const {

  int res = p->extract(archive,sdir);
  p->increaseSteps();
  //p->printProgress();
  //return p->extract(archive,sdir);
  return res;
}

/*****************************************************************
 Added since I needed to know whether install has components attached
                                                          EMH
*****************************************************************/
bool EGS_Archive::isZipFile(const char *archive) const {
    return p->isZipFile(archive);
}

bool PrivateArchive::isZipFile(const char *archive) const {
  ifstream in(archive, ios::binary);
  if( !in ) {
    //cout << "Failed to open file " << archive << endl;
    printProgress( (QString)"\nFailed to open file " + (QString)archive + (QString)"\n");
    return false;
  }
  int msize = marker.size() + 1;
  char *buf = new char [ msize ];
  //bool found_marker = false;
  while ( !in.eof() ) {
    in.read(buf,msize); buf[msize-1] = 0;
    if( marker == buf ) { return true;  }
  }
  return false;
}

QMutex egs_mutex;

void PrivateArchive::printProgress( const QString& s )const{
    egs_mutex.lock();
    QApplication::postEvent(receiver, new EGSCustomEvent(s));
    qApp->processEvents();
    egs_mutex.unlock();
}
void PrivateArchive::printProgress(){
    int prog = int( 100*steps/totSteps );
    QApplication::postEvent(receiver, new EGSCustomEvent(prog));
    qApp->processEvents();
}

void PrivateArchive::printProgress(const float & frac)const{
    int prog = int(100*(frac+float(steps))/float(totSteps));
    QApplication::postEvent(receiver, new EGSCustomEvent(prog));
    qApp->processEvents();
}

void PrivateArchive::increaseSteps(){steps++;}

void EGS_Archive::setIODevice( QTextEdit*  io_dev ){
    p->setIODevice( io_dev );
}

void PrivateArchive::setIODevice( QTextEdit*  io_dev ){
    monitor = io_dev;
}

//************************************************************

int EGS_Archive::listFiles(const char *archive) const {
  return p->listFiles(archive);
}

int PrivateArchive::listFiles(const char *archive) const {
  ifstream in(archive, ios::binary);
  if( !in ) {
    //cout << "Failed to open file " << archive << endl;
    return 1;
  }
  int msize = marker.size() + 1;
  char *buf = new char [ msize ];
  bool found_marker = false;
  while ( !in.eof() ) {
    in.read(buf,msize); buf[msize-1] = 0;
    if( marker == buf ) { found_marker = true; break; }
  }
  if( !found_marker ) {
    //cout << "File " << archive << " seems to not contain an EGSnrc archive\n";
    return 2;
  }
  char a_endian; in.read(&a_endian,1);
  if( a_endian < 0 || a_endian > 1 ) {
    //cout << "Archive created on a machine with unsupported endianess\n";
    return 7;
  }
  bool swap = a_endian != endian;
  unsigned int osize, zsize; int nsize; char *nam;
  while( !in.eof() ) {
    in.read((char *)&zsize, sizeof(unsigned int));
    if( in.eof() || !in.good() ) return 0;
    in.read((char *)&osize, sizeof(unsigned int));
    in.read((char *)&nsize, sizeof(int));
    if( !in.good() ) return 3;
    if( swap ) {
      swapBytes((unsigned char *)&osize);
      swapBytes((unsigned char *)&zsize);
      swapBytes((unsigned char *)&nsize);
    }
    nam = new char [nsize];
    in.read(nam, nsize);
    if( !in.good() || in.gcount() != nsize ) {
      delete [] nam; return 3;
    }
    //cout << nam << endl;
    delete [] nam;
    char *zip_buf = new char [zsize];
    in.read(zip_buf,zsize);
    delete [] zip_buf;
    if( !in.good() || in.gcount() != zsize ) return 3;
  }
  return 0;
}

bool PrivateArchive::checkDir(const char *file_nam) const {
  QString fn(file_nam);
  fn = fn.replace((QRegExp)"\\","/");
  fn = fn.replace((QRegExp)"//","/");
  QFileInfo fi(fn);
  QString dir = fi.absolutePath();
  QDir dd = fi.absoluteDir();
  int nup = 0;

QString d = dir;
  while( !dd.exists() ) {
           d.truncate( d.lastIndexOf("/") );
           dd.setPath( d  );
           nup++;
  }

  QStringList list = dir.split("/");
  QString tmp=dd.path(); tmp += "/";
  for(int j=list.count()-nup; j<list.count(); j++) {
      tmp += list[j]; tmp += "/";
      if( !dd.mkdir(tmp) ) {
	  printProgress( (QString) "Making " + tmp + (QString)" failed\n");
	  return false;
      }
  }
  return true;
}

void PrivateArchive::stop(){
  m_exit = true;
}

void EGS_Archive::stop(){
    if ( p ) p->stop();
}


int PrivateArchive::extract(const char *archive, const char *sdir) const {

  ifstream in(archive, ios::binary);
  if( !in ) {
    //cout << "Failed to open file " << archive << endl;
    printProgress("\nFailed to open file\n");
    return 1;
  }
  /*** get archive size ***/
  in.seekg (0, ios::end);
  unsigned int the_ar_size = in.tellg(),
               the_exe_size = 0,
               the_done_size = 0;
  in.seekg (0, ios::beg);
  /***********************/
  int msize = marker.size() + 1;
  char *buf = new char [ msize ];
  bool found_marker = false;
  while ( !in.eof() ) {
    in.read(buf,msize); buf[msize-1] = 0; the_exe_size += msize;
    if( marker == buf ) { found_marker = true; break; }
  }
  if( !found_marker ) {
    //cout << "File " << archive << " seems to not contain an EGSnrc archive\n";
    printProgress( (QString)"File " + (QString)archive +
		     (QString)" seems to not contain an EGSnrc archive\n");
    return 2;
  }
  the_ar_size -= the_exe_size;
//cout << "archive size = " << the_ar_size << endl;
  char a_endian; in.read(&a_endian,1);
  if( a_endian < 0 || a_endian > 1 ) {
    //cout << "Archive created on a machine with unsupported endianess\n";
    printProgress( "\nArchive created on a machine with unsupported endianess\n");
    return 7;
  }
  bool swap = a_endian != endian;
  unsigned int osize, zsize; int nsize; char *nam;
  string dir;
  if( sdir ) {
    dir = sdir; if( dir[dir.size()-1] != dir_sep ) dir += dir_sep;
  }
  while( !in.eof() ) {

      if ( m_exit ){
         printProgress( "\nTermination requested. Stopping ...\n" );
         return 8;
      }
    in.read((char *)&zsize, sizeof(unsigned int));
    if( in.eof() || !in.good() ) return 0;
    in.read((char *)&osize, sizeof(unsigned int));
    in.read((char *)&nsize, sizeof(int));
    if( !in.good() ) {
        printProgress(
        (QString)"\nNon-zero return code 31 while uncompressing archive " +
        (QString)archive);
        return 31;
    }
    if( swap ) {
      swapBytes((unsigned char *)&osize);
      swapBytes((unsigned char *)&zsize);
      swapBytes((unsigned char *)&nsize);
    }
    nam = new char [nsize];
    in.read(nam, nsize);
    if( !in.good() || in.gcount() != nsize ) {
      printProgress(
      (QString)"\nNon-zero return code 32 while uncompressing archive " +
      (QString)archive);
      delete [] nam; return 32;
    }

   QString fn(dir.c_str()); fn.append(nam);
   fn = fn.replace("\\","/");
   fn = fn.replace("//","/");
   string ofile = fn.toStdString();
//    string ofile = dir + nam;

    QString progress = (QString)"Inflating "  +
                       (QString)ofile.c_str() + (QString)"\n";
    printProgress( progress );

    if( !checkDir( ofile.c_str()) ) {
      //cout << "Failed to create " << ofile.c_str();
      printProgress( (QString)"Failed to create " +
                     (QString)ofile.c_str()       +
                     (QString)"\n" );
       return 5;
    }
    ofstream out(ofile.c_str(),ios::binary | ios::trunc);
    if( !out ) {
      //cout << "Failed to open " << ofile << " for writing\n";
      printProgress( (QString)"Failed to open " +
                     (QString)ofile.c_str()     +
                     (QString)" for writing\n" );
      return 4;
    }
    delete [] nam;
    Bytef *zip_buf = new Bytef [zsize];
    in.read((char *)zip_buf,zsize);
    if( !in.good() || in.gcount() != zsize ) {
      printProgress((QString)"\nNon-zero return code 33 while uncompressing archive " +
                   (QString)archive + (QString)"...\n");
      printProgress( (QString)"Error uncompressing " +
                     (QString)ofile.c_str()          +
                     (QString)"\n" );
      delete [] zip_buf; return 33;
    }
    Bytef *unzip_buf = new Bytef [osize];
    uLongf the_osize = osize;
    uncompress(unzip_buf,&the_osize,zip_buf,zsize);
    out.write((char *)unzip_buf,osize);
    delete [] zip_buf; delete [] unzip_buf;
    out.close();
    the_done_size += zsize;
    printProgress(float(the_done_size)/float(the_ar_size));
  }
  return 0;
}

EGSThread::EGSThread( const QString& archi, const QString& rdir )
    : archive(archi),dir(rdir), status(0), receiver(0)
{
    m_exit = false;
    ar = 0;
}

EGSThread::EGSThread( QWidget* o, const QString& archi, const QString& rdir )
    : archive(archi), dir(rdir), status(0), receiver(o)
{
    m_exit = false;
    ar = 0;
}


EGSThread::EGSThread( )
    : status(0)
{
    archive ="";
    dir     ="";
    m_exit = false;
    ar = 0;
}

void EGSThread::run()
{
    status = extract(archive, dir );
    msleep(30);
}

int EGSThread::extract( const QString& archi, const QString& rdir ){
//   create an archive object
    ar = new EGS_Archive(receiver);
    int res = ar->extract( archi.toLatin1(), rdir.toLatin1() );
    delete ar;
    return res;
}
