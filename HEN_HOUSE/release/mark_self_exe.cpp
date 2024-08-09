/*
###############################################################################
#
#  EGSnrc self-extracting archive utility
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
#  Puts an end marker (and pad if necessary) to a executable so that it can be
#  made self-extracting by simply appending the archive files.
#
###############################################################################
*/


#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

#define the_marker "THE_END-DAS_ENDE-FIN"
const char *part1 = "____";
const char *part2 = "egsnrc";
const char *part3 = "xp";
const char *part4 = "archive";

int main(int argc, char **argv) {

    if( argc < 2 ) {
        cout << "Usage: " << argv[0] << " self_exe_file_name\n";
        return 1;
    }

    // make the marker
    string marker;
    marker = part1;
    marker += part2;
    marker += part1;
    marker += part3;
    marker += part1;
    marker += part4;
    marker += part1;
    //cout << "Using end marker:\n<" << marker.c_str() << ">\n";

    // determine endianess
    int nl = 0x12345678;
    char endian;
    unsigned char *p = (unsigned char *)(&nl);
    if ( p[0] == 0x12 && p[1] == 0x34 && p[2] == 0x56 && p[3] == 0x78 )
        endian = 0;
    else {
        if ( p[0] == 0x78 && p[1] == 0x56 && p[2] == 0x34 && p[3] == 0x12 )
            endian = 1;
        else {
            cout << "Unsupported endiannes\n";
            exit(1);
        }
    }
    //if( endian == 0 ) cout << "Machine is big endian\n";
    //else cout << "Machine is little endian\n";

    int msize = marker.size()+1;
    ofstream *o;
    char *buf = new char [msize];
    ifstream in(argv[1], ios::binary);
    if( !in ) {
        //cout << "File " << argv[1] << " does not exist => creating new\n";
        o = new ofstream(argv[1], ios::binary | ios::trunc);
    }
    else {
        bool has_marker = false;
        while ( !in.eof() ) {
            in.read(buf,msize);
            buf[msize-1] = 0;
            if( marker == buf ) {
                has_marker = true;
                break;
            }
        }
        if( has_marker ) {
            //cout << "\nThe file has already an end marker!\n";
            return 0;
        }

        // the eof in the while loop causes the fail io stream state flag to be set, and
        // seek and tell don't work anymore if this flag is set. First clear the state flags,
        // then seek the end of the file before calling tellg().
        in.clear();
        in.seekg(0, ios::end);

        unsigned long fsize = in.tellg();
        int nn = fsize/msize + 1;
        int npad = nn*msize - fsize;
        char c = 0;
        //cout << "fsize = " << fsize << " npad = " << npad << endl;
        o = new ofstream(argv[1], ios::binary | ios::app);
        if ( ! (*o) ) {
            cout << "Failed to open " << argv[2] << " for writing\n";
            return 1;
        }
        o->seekp(fsize,ios::beg);
        for(int j=0; j<npad; j++) o->write(&c,1);
    }
    o->write(marker.c_str(),msize);
    o->write(&endian,1);
    o->close();

    return 0;

}
