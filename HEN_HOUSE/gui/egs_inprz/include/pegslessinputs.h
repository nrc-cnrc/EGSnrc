/*
###############################################################################
#
#  EGSnrc egs_inprz pegsless input headers
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
#  Author:          Blake Walters, 2013
#
#  Contributors:
#
###############################################################################
#
#  This file was originally derived from the file mcinputs.h written by
#  Ernesto Mainegra-Hing, starting in 2001.
#
###############################################################################
*/


#ifndef PEGSLESSINPUTS_H
#define PEGSLESSINPUTS_H
#define MXMED 12
#define MXEL 14

#include "inputblock.h"
#include <QTextStream>

struct Element {
  int   Z;
  std::string symbol;
 float  aw;
 float  Iev;
 float  rho;
};

extern QStringList element_list;

extern int n_element;

extern Element element_data[];

class PEGSLESSInputs : public MInputBlock
{
  public:
   PEGSLESSInputs();
  ~PEGSLESSInputs();

 	QString AE;
 	QString UE;
 	QString AP;
        QString UP;

        QString matdatafile;

 	int ninpmedia;
        int inpmedind;
        int nelements[MXMED];

        bool spec_by_pz[MXMED];
        QString inpmedium[MXMED];
        v_string elements[MXMED];
        v_string pz_or_rhoz[MXMED];
        QString rho[MXMED];
        QString spr[MXMED];
        QString bc[MXMED];
        QString gasp[MXMED];
        bool isgas[MXMED];
        bool use_dcfile[MXMED];
        QString dffile[MXMED];
        QString sterncid[MXMED];
        float rho_scale[MXMED];
};
std::ifstream & operator >> ( std::ifstream & in, PEGSLESSInputs * rPEGSLESS );
QTextStream   & operator << ( QTextStream &    t, PEGSLESSInputs * rPEGSLESS );
#endif
