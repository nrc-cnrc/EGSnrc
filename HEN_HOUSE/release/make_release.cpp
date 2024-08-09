/*
###############################################################################
#
#  EGSnrc make_release utility
#  Copyright (C) 2018 National Research Council Canada
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
#  Author:       Iwan Kwrakow
#
#  Contributors: Ernesto Mainegra-Hing
#
###############################################################################
#
#  Reads a list of files to be archived for release with EGSnrc.
#
#  Initially, the whole system was distributed in several unix tar archives
#  (compressed and uncompressed).
#
#  Since the system is now downloaded or cloned from github, this utility is
#  now used to build pre-compiled Qt GUIs, statically linked to the Qt library.
#  The compressed tar archive is also appended to the egs_configure tool which
#  self-extracts the files upon installation.
#
#  Note that one could use this tool to create a compressed archive with
#  arbitrary files using the same format as in file egs_guis_list.
#
###############################################################################
*/


#include <qfile.h>
#include <qstring.h>
#include <qtextstream.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qstringlist.h>

#include <sys/types.h>
#include <unistd.h>

#include "egs_archive.h"

#include <cstdlib>

bool create_dir(const QString &);
bool check_dir(const QString &);

void add_directory(EGS_Archive *, const QString &);

int main(int argc, char **argv) {

    if( argc < 3 ) {
        qDebug("Usage: %s -l file_list [-exe exe_file] [-p win|unix]\n",argv[0]);
        qDebug("file_list: a file containing a list of files to distribute");
        qDebug("exe_file:  if argument present, resulting archive will be");
        qDebug("           appended to the file named exe_file");
        qDebug("-p:        specify a platform (unix is the default)");
        return 1;
    }

    //qDebug("cwd = %s",QDir::currentDirPath().toLatin1().data());
    int j;
    // check for the list_file and open it.
    QFile f;
    for(j=0; j<argc-1; j++) {
        QString arg = argv[j];
        if( arg == "-l" ) {
            f.setFileName(argv[j+1]);
            if( !f.open(QIODevice::ReadOnly) )
              qFatal("Failed to open file %s",argv[j+1]);
            break;
        }
    }

    // check for -exe option
    QStringList exe;
    for(j=0; j<argc-1; j++) {
        QString arg = argv[j];
        if( arg == "-exe" ) {
            //--------------------------------------------------
            //  Need to see whether there are more than one file
            //  before next -flag
            //---------------------------------------------------
            for(int k=j+1; k<argc; k++) {
                QString dexe = argv[k];
                if ( dexe.startsWith("-") ) {
                    break;
                }
                else {
                    QFileInfo fi(dexe);
                    if( !fi.exists() )
                        qFatal( "There is no exe file named %s",dexe.toLatin1().data() );
                    else
                        exe += dexe;
                }
            }
            //-------------------------------------------------------
        }
    }

    // check for -p option
    QString platform = "unix";
    for(j=0; j<argc-1; j++) {
        QString arg = argv[j];
        if( arg == "-p" ) {
            QString tmp = argv[j+1];
//-------------------------------------------------------
// Added possibility for creating both at the same time
//-------------------------------------------------------
            if( tmp == "win" || tmp == "unix" || tmp == "all" )
                platform = tmp;
            else qFatal("Unknown platform: %s",argv[j+1]);
//-------------------------------------------------------
        }
    }

    int mypid = getpid();
    QString ap_tmp = QString("_tmp_%1").arg(mypid);

    // Now process the list
    QString archive;
    int nline=0;
    QTextStream tl;
    QFile *tar_list = 0;
    QStringList moved_files;
    EGS_Archive *ar = 0;
    bool have_version = false;
    QString version_string;
    while( !f.atEnd() ) {
        QString line;
        line = f.readLine(1024);
        nline++;
        if( line.startsWith("#") ) continue;
        line = line.simplified();
        if( line.isEmpty() ) continue;
        QTextStream ts(&line,QIODevice::ReadOnly);

        if( line.startsWith("version: ") ) {
            QString junk;
            ts >> junk >> version_string;
            have_version = true;
            continue;
        }

        if( line.startsWith("tarfile:") || line.startsWith("end list") ) {
            if( tar_list ) {
                if( archive.isEmpty() )
                    qFatal("tar_list is not null but arvhive is empty ?");
                tar_list->close();
                qDebug("  creating tar file");
                QString command = QString("tar -cf %1.tar").arg(archive);
                command += QString(" --files-from=%1.list").arg(archive);
                //qDebug("Command: %s",command.toLatin1().data());
                command += QString(" --exclude=CVS");
                command += QString(" --exclude=\"*~\"");
                int res = system(command.toLatin1().data());
                if( res ) qDebug("tar failed ?");
                else {
                    command = QString("gzip -c %1.tar").arg(archive);
                    command += QString(" >%1.tar.gz").arg(archive);
                    qDebug("  creating gziped tar file");
                    res = system(command.toLatin1().data());
                    //command = QString("bzip2 -c %1.tar").arg(archive);
                    //command += QString(" >%1.tar.bz2").arg(archive);
                    //qDebug("  creating bziped tar file");
                    //res = system(command.toLatin1().data());
                }
                if( ar ) {
                    qDebug("  creating egsself file(s)");
                    ar->make();
                    QString zip_arc = archive + ".egsself";
                    ar->closeOutput(zip_arc.toLatin1().data());
                    ar->clearFiles();
                }
                if( moved_files.count() > 0 ) {
                    QDir d;
                    for(int i=0; i<moved_files.count(); i++) {
                        QStringList l = moved_files[i].split(" ");
                        //qDebug("Renaming %s to %s",l[1].toLatin1().data(),l[0].toLatin1().data());
                        d.rename(l[1],l[0]);
                        QString aux = l[1] + ap_tmp;
                        QFileInfo info(aux);
                        if( info.exists() ) {
                            //qDebug("Renaming %s to %s",aux.toLatin1().data(),l[1].toLatin1().data());
                            d.rename(aux,l[1]);
                        }
                    }
                }
                delete tar_list;
                tar_list=0;
            }
            if( line.startsWith("end list") ) return 0;
            QString junk;
            ts >> junk >> archive;
            if( !have_version ) qFatal("Found archive but version undefined!\n");
            archive = version_string + archive;
            qDebug("New archive: %s",archive.toLatin1().data());
            QString aux = archive + ".list";
            tar_list = new QFile(aux);
            if( !tar_list->open(QIODevice::WriteOnly) )
                qFatal("Failed to open list file %s",aux.toLatin1().data());
            tl.setDevice(tar_list);
            if( ar == 0 ) {
                ar = new EGS_Archive;
                //----------------------------------------------------------
                //       Adding all executable files
                //----------------------------------------------------------
                for ( QStringList::Iterator it = exe.begin(); it != exe.end(); ++it) {
                    if( !(*it).isEmpty() ) ar->addOutput((*it).toLatin1().data(),1);
                }
                //----------------------------------------------------------
            }
            QString zip_arc = archive + ".egsself";
            ar->addOutput(zip_arc.toLatin1().data(),0);
            continue;
        }
        if( archive.isEmpty() ) {
            qFatal("archive name undefined on line %d ?",nline);
        }
        //---------------------------------------------------------
        //            Platform check
        // (The comented if-statement would be better only if we
        //   have more platforms in the future )
        //---------------------------------------------------------
        if( line.startsWith("unix:") ) {
            //if( platform != "unix" && platform != "all" ) continue;
            if( platform == "win" ) continue;
            line = line.right(line.length()-5).simplified();
        }
        else if ( line.startsWith("win:") ) {
            //if( platform != "win" && platform != "all" ) continue;
            if( platform == "unix" ) continue;
            line = line.right(line.length()-4).simplified();
        }
        else line = line.simplified();
        //---------------------------------------------------------
        if( line.isEmpty() ) continue;
        //qDebug("Processing line: %s",line.toLatin1().data());
        QStringList l = line.split(" ");
        //qDebug("Line has %d entries",l.count());
        if( l.count() == 0 ) qFatal("zero count on line %d ?",nline);
        QString the_file;
        if( l.count() == 1 ) {
            QFileInfo info(line);
            if( !info.exists() ) qFatal("File %s does not exist",line.toLatin1().data());
            the_file = line;
            tl << line << "\n";
        }
        else {
            QFileInfo fi(l[1]);
            QDir d;
            QString aux = l[0];
            aux += " ";
            aux += l[1];
            moved_files << aux;
            if( fi.exists() ) {
                QString tmp_name = l[1] + ap_tmp;
                d.rename(l[1],tmp_name);
            }
            if( !d.rename(l[0],l[1]) )
                qFatal("failed to rename %s to %s",l[0].toLatin1().data(),l[1].toLatin1().data());
            tl << l[1] << "\n";
            the_file = l[1];
        }
        if( ar ) {
            QFileInfo fi(the_file);
            if( fi.isDir() ) add_directory(ar,the_file);
            else ar->addFile(the_file.toLatin1().data());
        }
    }
    return 0;
}

bool check_dir(const QString &fn) {
    QFileInfo fi(fn);
    QString dir = fi.path();
    QStringList list = dir.split("/");
    QString tmp;
    QDir d;
    for(QStringList::Iterator it=list.begin(); it != list.end(); ++it) {
        tmp += (*it);
        if( !d.exists(tmp) ) {
            //cout << "Making " << tmp.toLatin1().data() << endl;
            qDebug("Making %s",tmp.toLatin1().data());
            if( !d.mkdir(tmp) ) return false;
        }
        tmp += "/";
    }
    return true;
}

bool create_dir(const QString &dir) {
    QStringList list = dir.split("/");
    QString tmp;
    QDir d;
    for(QStringList::Iterator it=list.begin(); it != list.end(); ++it) {
        tmp += (*it);
        //cout << "Making " << tmp.toLatin1().data() << endl;
        qDebug("Making %s",tmp.toLatin1().data());
        if( !d.mkdir(tmp) ) return false;
        tmp += "/";
    }
    return true;
}

void add_directory(EGS_Archive *ar, const QString &dir) {
    QDir d(dir);
    QStringList list = d.entryList();
    for(unsigned int j=0; j<list.count(); j++) {
        if( list[j] != "." && list[j] != ".." && list[j] != "CVS" ) {
            QString aux = dir;
            if( aux[aux.length()-1] != '/' ) aux += '/';
            aux += list[j];
            QFileInfo fi(aux);
            if( fi.isDir() ) add_directory(ar,aux);
            else {
                ar->addFile(aux.toLatin1().data());
                //qDebug("added %s",aux.toLatin1().data());
            }
        }
    }
}
