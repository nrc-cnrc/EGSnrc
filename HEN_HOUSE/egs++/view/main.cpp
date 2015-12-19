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
#
###############################################################################
*/


#include <qapplication.h>
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

    QApplication a(argc, argv);
    QString input_file = argc >= 2 ? QString(argv[1]) :
                         QFileDialog::getOpenFileName(NULL,"Select geometry definition file");
    QString tracks_file = argc >= 3 ? argv[2] : "";
    //if( argc < 2 ) egsFatal("\nUsage: %s geometry_file\n\n",argv[0]);
    //QFile file(argv[1]);
#ifdef VDEBUG
    debug_output << "Using " << input_file.toLatin1().data() << "\n";
    egsSetInfoFunction(Information,my_info_function);
    egsSetInfoFunction(Warning,my_info_function);
    egsSetInfoFunction(Fatal,my_fatal_function);
#endif

    QFile file(input_file);
    if (!file.exists()) {
        egsFatal("\nFile %s does not exist\n\n",argv[1]);
    }

#ifdef VDEBUG
    debug_output << "About to construct EGS_Input object\n";
#endif
    EGS_Input input;
#ifdef VDEBUG
    debug_output << "OK, parsing input\n";
#endif

    //input.setContentFromFile(argv[1]);
    input.setContentFromFile(input_file.toUtf8().constData());
#ifdef VDEBUG
    debug_output << "Finished parsing\n";
#endif
#ifdef VIEW_DEBUG
    input.print(0,cerr);
#endif

    EGS_BaseGeometry *g = EGS_BaseGeometry::createGeometry(&input);
#ifdef VDEBUG
    debug_output << "Got geometry\n";
#endif
    if (!g) egsFatal("\nThe input file %s seems to not define a valid"
                         " geometry\n\n",argv[1]);

    EGS_Float xmin = -50, xmax = 50;
    EGS_Float ymin = -50, ymax = 50;
    EGS_Float zmin = -50, zmax = 50;
    EGS_Input *vc = input.takeInputItem("view control");
    std::vector<EGS_UserColor> user_colors;
    if (vc) {
        EGS_Float tmp;
        if (!vc->getInput("xmin",tmp)) {
            xmin = tmp;
        }
        if (!vc->getInput("xmax",tmp)) {
            xmax = tmp;
        }
        if (!vc->getInput("ymin",tmp)) {
            ymin = tmp;
        }
        if (!vc->getInput("ymax",tmp)) {
            ymax = tmp;
        }
        if (!vc->getInput("zmin",tmp)) {
            zmin = tmp;
        }
        if (!vc->getInput("zmax",tmp)) {
            zmax = tmp;
        }
        EGS_Input *uc;
        while ((uc = vc->takeInputItem("set color")) != 0) {
            vector<string> inp;
            int err = uc->getInput("set color",inp);
            if (!err && (inp.size() == 4 || inp.size() == 5)) {
                qDebug("found color input %s %s %s %s",inp[0].c_str(),inp[1].c_str(),inp[2].c_str(),inp[3].c_str());
                EGS_UserColor ucolor;
                ucolor.medname = inp[0];
                sscanf(inp[1].c_str(),"%d",&ucolor.red);
                sscanf(inp[2].c_str(),"%d",&ucolor.green);
                sscanf(inp[3].c_str(),"%d",&ucolor.blue);
                if (inp.size() == 5) {
                    sscanf(inp[4].c_str(),"%d",&ucolor.alpha);
                }
                else {
                    ucolor.alpha = 255;
                }
                qDebug("Using rgb=(%d,%d,%d %d) for medium %s",ucolor.red,ucolor.green,ucolor.blue,
                       ucolor.alpha,ucolor.medname.c_str());
                user_colors.push_back(ucolor);
            }
            else {
                qWarning("Wrong 'set color' input");
            }
            delete uc;
        }
        delete vc;
    }

    GeometryViewControl w;
    w.show();
    w.setFilename(input_file);
    w.setTracksFilename(tracks_file);
    if (w.setGeometry(g,user_colors,xmin,xmax,ymin,ymax,zmin,zmax,0)) {
        return 1;
    }

    a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));

    return a.exec();
}
