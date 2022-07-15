/*
###############################################################################
#
#  EGSnrc egs++ geometry viewer main
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
#  Author:          Iwan Kawrakow, 2005
#
#  Contributors:    Frederic Tessier
#                   Manuel Stoeckl
#                   Ernesto Mainegra-Hing
#                   Cody Crewson
#                   Reid Townson
#
###############################################################################
*/


#include <QApplication>
#include <QtGlobal>
#if QT_VERSION >= 0x050000
    #include <QtWidgets>
#else
    #include <QWidget>
#endif
#include <qfile.h>
#include <qstring.h>
#include <qfiledialog.h>

#include "viewcontrol.h"
#include "egs_base_geometry.h"
#include "egs_functions.h"
#include "egs_input.h"
#include "egs_user_color.h"

#include <cstdio>

#ifdef VDEBUG
#include <fstream>
std::ofstream debug_output("view_debug");

#include "egs_functions.h"
#include <cstdio>
#include <cstdarg>
#include <cstdlib>

static char mybuf[8192];

void my_fatal_function(const char *msg,...) {
    va_list ap;
    va_start(ap, msg);
    vsprintf(mybuf,msg,ap);
    va_end(ap);
    debug_output << mybuf;
    exit(1);
}
void my_info_function(const char *msg,...) {
    va_list ap;
    va_start(ap, msg);
    vsprintf(mybuf,msg,ap);
    va_end(ap);
    debug_output << mybuf;
}
#endif

int main(int argc, char **argv) {
    if (argc >= 2 && (strcmp(argv[1], "-h") == 0 ||
                      strcmp(argv[1],"--help") == 0)) {
        egsFatal("Usage: %s [geometry_file] [tracks_file]\n", argv[0]);
        return 1;
    }

    QApplication a(argc, argv);
    QString input_file = argc >= 2 ? QString(argv[1]) :
                         QFileDialog::getOpenFileName(NULL,"Select geometry definition file");

#ifdef VDEBUG
    debug_output << "Using " << input_file.toLatin1().data() << "\n";
    egsSetInfoFunction(Information,my_info_function);
    egsSetInfoFunction(Warning,my_info_function);
    egsSetInfoFunction(Fatal,my_fatal_function);
#endif

    GeometryViewControl w;
    w.show();
    w.setFilename(input_file);

    QString tracks_file = QString("");
    QString config_file = QString("");
    if (argc >= 3) {
        QString argv2 = argv[2];
        if (argv2.endsWith("ptracks")) {
            tracks_file = argv2;
        }
        else {
            config_file = argv2;
        }
    }
    if (argc >= 4) {
        QString argv3 = argv[3];
        if (argv3.endsWith("ptracks")) {
            tracks_file = argv3;
        }
        else {
            config_file = argv3;
        }
    }

    w.setTracksFilename(tracks_file);
    if (!w.loadInput(false)) {
        return 1;
    }
    w.loadConfig(config_file);

    // Request a monospace font from the OS
#if QT_VERSION >= 0x040700
    QFont new_font = a.font();
    new_font.setStyleHint(QFont::Monospace);
    a.setFont(new_font);
#endif

    a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));

    return a.exec();
}
