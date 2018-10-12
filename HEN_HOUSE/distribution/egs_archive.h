/*
###############################################################################
#
#  EGSnrc archive utility headers
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
#  A simple tool to compress/uncompress, add/extract files to an archive,
#  used by the graphical EGSnrc configuration tool egs_configure.
#
###############################################################################
*/


#ifndef EGS_ARCHIVE_
#define EGS_ARCHIVE_

#include <qtextedit.h>

class PrivateArchive;

/*****************************************************************************
 * A simple class to compress/uncompress, add/extract files to a
 * archive used by the graphical EGSnrc installer.
 ****************************************************************************/
class EGS_Archive {

public:

    EGS_Archive();
    ~EGS_Archive();

    void addOutput(const char *, int type);
    void closeOutput(const char *);
    void addFile(const char *);
    void clearFiles();
    void setIODevice( QTextEdit*  io_dev );

    int  make();
    int extract(const char *archive, const char *start_dir=0) const;
    int listFiles(const char *archive) const;

    bool isZipFile(const char *archive) const;

private:

    PrivateArchive *p;

};

#endif
