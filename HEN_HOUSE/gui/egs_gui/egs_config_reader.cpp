/*
###############################################################################
#
#  EGSnrc gui configuration reader
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


#include "egs_config_reader.h"

#include <qmap.h>
#include <qfile.h>
#include <qstringlist.h>
#include <qregexp.h>
#include <qdir.h>
#include <qmessagebox.h>

#include <cstdlib>
#include <string>

#include <QTextStream>

using namespace std;

//#define CR_DEBUG

#ifdef CR_DEBUG
#include <fstream>
ofstream cr_debug("cr_debug.out");
#endif

class EGS_PrivateConfigReader {

public:

    EGS_PrivateConfigReader();
    EGS_PrivateConfigReader(const QString &file);
    void setConfig(const QString &file);
    void addFile(const QString &file);
    QString getVariable(const QString &key,bool ironit);
    QString simplify(const QString &value,bool ironit);
    QString ironIt(const QString &);
    int checkConfigFile(const QString &);

    QString               the_config;
    QMap<QString,QString> map;

};

QString EGS_ConfigReader::getConfig() const {
    return p->the_config;
}

EGS_PrivateConfigReader::EGS_PrivateConfigReader() {
    char *egs_config = getenv("EGS_CONFIG");
    if( !QString(egs_config).isEmpty() ) setConfig(egs_config);
    else the_config = QString();
}

EGS_PrivateConfigReader::EGS_PrivateConfigReader(const QString &file) {
    setConfig(file);
}

QString EGS_PrivateConfigReader::ironIt(const QString &v) {
    QString aux = "/+|"; aux += "\\\\"; aux += "+";
    QRegExp re(aux);
    QStringList list = v.split(re,QString::SkipEmptyParts);
#ifdef CR_DEBUG
    cr_debug << "ironIt gets " << list.count() << " elements :" << list.join(",").toLatin1().data() << endl;
#endif
    QString res; if( v.startsWith("/") ) res = "/";
    for(QStringList::iterator it=list.begin(); it != list.end(); it++) {
        if( it != list.begin() ) res += QDir::separator();
        res += *it;
    }
    if( v.endsWith("/") || v.endsWith("\\") ) res += QDir::separator();
#ifdef CR_DEBUG
    cr_debug << "ironIt returns: " << res.toLatin1().data() << endl;
#endif
    return res;
}

QString EGS_PrivateConfigReader::simplify(const QString &value,bool ironit) {
    QString junk = "\\$\\((\\w+)\\)";
    QRegExp re(junk);
    QString res; int pos = 0;
    while(1) {
        int pos1 = re.indexIn(value,pos);
        if( pos1 < 0 ) { res += value.mid(pos); break; }
        res += value.mid(pos,pos1-pos);
        res += getVariable(re.cap(1),ironit);
        pos = pos1 + re.matchedLength();
    }
    if( ironit ) return ironIt(res);
    return res;
}

void EGS_ConfigReader::setVariable(const QString &key, const QString &v){
    p->map.insert(key,v);
}

QString EGS_PrivateConfigReader::getVariable(const QString &key, bool ironit) {
    QString res = QString();
    if( map.contains(key) ) res = simplify(map[key],ironit);
    else {
        char *var = getenv(key.toLatin1().data());
        if( var ) res = ironIt(var);
    }
#ifdef CR_DEBUG
    cr_debug << "getVariable(" << key.toLatin1().data() << "): " << res.toLatin1().data() << endl;
#endif
    return res;
}

void EGS_PrivateConfigReader::setConfig(const QString &file) {
#ifdef CR_DEBUG
    cr_debug << "EGS_PrivateConfigReader::setConfig: "
        << file.toLatin1().data() << endl;
#endif
    if (!checkConfigFile(file)){
       the_config = file; //map.clear();
       addFile(file);
    }
    else the_config = QString();
}

int EGS_PrivateConfigReader::checkConfigFile(const QString &file) {
    QFile f(file);
    if( !f.open(QFile::ReadOnly) ) return 1;
    QTextStream ts(&f); QString the_line;
    int res = 0;
    while( !ts.atEnd() ) {
        QString line = ts.readLine();
        if( line.startsWith("include") ) {
#ifdef WIN32
            int pos = line.indexOf("unix.spec");
#else
            int pos = line.indexOf("windows.spec");
#endif
            if( pos >= 0 ) { res = 2; break; }
        }
    }
    return res;
}

void EGS_PrivateConfigReader::addFile(const QString &file) {
#ifdef CR_DEBUG
    cr_debug << "EGS_PrivateConfigReader::addFile: " << file.toLatin1().data() << endl;
#endif
    QFile f(file);
    if( !f.open(QFile::ReadOnly) ) return;
    QTextStream ts(&f); QString the_line;
    while( !ts.atEnd() ) {
        QString line = ts.readLine();
        if( line.startsWith("#") ) continue;
        QString line1 = line.simplified();
        if( line1.isEmpty() ) continue;
        if( !line1.endsWith("\\\\") && line1.endsWith("\\") ) {
            line1[line1.length()-1] = ' '; the_line += line1;
            continue;
        }
        the_line += line;
        if( the_line.startsWith("include") ) {
            QStringList list = the_line.split(" ");
            list.pop_front(); QString inc = list.join(" ");
            addFile(simplify(inc,true));
            the_line = "";
        }
        else {
            QRegExp rx("\\b(\\w+)\\s*=\\s*");
            int pos = rx.indexIn(the_line);
            if( pos >=0 ) {
#ifdef CR_DEBUG
                cr_debug << "inserting <" << rx.cap(1).toLatin1().data()
                    << "> <" << the_line.mid(pos+rx.matchedLength()).toLatin1().data()
                    << ">\n";
#endif

                map.insert(rx.cap(1),the_line.mid(pos+rx.matchedLength()));
            }
            else {
                QRegExp rx1("\\b(\\w+)\\s*:=\\s*");
                int pos = rx1.indexIn(the_line);
                if( pos >=0 ) {
                    QString aux = the_line.mid(pos+rx1.matchedLength());
                    QString val;
                    if( aux == "$(shell echo \\)" ) val = "\\";
                    else if( aux == "$(subst /,\\,/)" ) val = "\\";
                    else if( aux == "$(subst \\,\\,\\)" ) val = "\\";
                    else val = simplify(aux,false);
#ifdef CR_DEBUG
                    cr_debug << "inserting <" <<
                        rx1.cap(1).toLatin1().data() << "> <" << val.toLatin1().data() << ">\n";
#endif
                    map.insert(rx1.cap(1),val);
                }
            }
            the_line = "";
        }
    }
}

EGS_ConfigReader::EGS_ConfigReader() {
    p = new EGS_PrivateConfigReader;
}

EGS_ConfigReader::EGS_ConfigReader(const QString &file) {
    p = new EGS_PrivateConfigReader(file);
}

void EGS_ConfigReader::setConfig(const QString &file) {
    p->setConfig(file);
}

QString EGS_ConfigReader::getVariable(const QString &key,bool ironit) {
    return p->getVariable(key,ironit);
}

EGS_ConfigReader::~EGS_ConfigReader() { if (p){delete p;p=0;} }

int EGS_ConfigReader::checkConfigFile(const QString &file) {
    return p->checkConfigFile(file);
}
