/*
###############################################################################
#
#  EGSnrc egs++ file info headers
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
#  Contributors:
#
###############################################################################
*/


#ifndef EGS_FILE_INFO_
#define EGS_FILE_INFO_

#include "egs_libconfig.h"

#include <string>
using std::string;

class EGS_EXPORT EGS_FileInfo {

public:

    EGS_FileInfo() : checked(false) { };
    EGS_FileInfo(const string &name) : checked(false) { setFile(name); };

    void setFile(const string &name) { fname = name; };

    bool exists() const;


private:

    bool   checked;
    string fname;

};

#endif
