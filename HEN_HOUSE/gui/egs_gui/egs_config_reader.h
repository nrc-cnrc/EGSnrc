/*
###############################################################################
#
#  EGSnrc gui configuration reader headers
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
#  Contributors:
#
###############################################################################
*/


#ifndef EGS_CONFIG_READER__
#define EGS_CONFIG_READER__

#include <qstring.h>

class EGS_PrivateConfigReader;

class EGS_ConfigReader {

public:

    EGS_ConfigReader();
    EGS_ConfigReader(const QString &file);
    ~EGS_ConfigReader();

    void setConfig(const QString &file);
    QString getConfig() const;

    QString getVariable(const QString &key, bool ironit=false);
    void    setVariable(const QString &key, const QString &value);

    int    checkConfigFile(const QString &file);

private:

    EGS_PrivateConfigReader *p;

};

#endif
