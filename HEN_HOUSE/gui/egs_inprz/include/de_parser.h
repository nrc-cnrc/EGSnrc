/*
###############################################################################
#
#  EGSnrc egs_inprz DE parser headers
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
#  Author:          Ernesto Mainegra-Hing, 2003
#
#  Contributors:
#
###############################################################################
#
#  This file was derived from the vmc++ DE_Parser class written by Iwan
#  Kawrakow, starting in 1999. A DE_Parser object is used by many classes
#  in vmc++ to retrieve initialization information from an input file.
#
###############################################################################
*/


#ifndef DE_Parser__

#define DE_Parser__

#include <iostream>
#include <string>
#include <vector>

typedef float Float;
//typedef double Float;


#ifdef WIN32
#define STR_STREAM(a,b,c) char *b = new char [a.size()+1]; strcpy(b,a.c_str()); std::istringstream c(b);
#else
#define STR_STREAM(a,b,c) char *b = new char [a.size()+1]; strcpy(b,a.c_str()); std::istrstream c(b);
#endif

using std::string;

template <class X>
int getinput(string &code, std::vector<string> &code_words,
             std::vector<string> &results, std::vector<X> &result, bool remove = false);

template <class X>
int getinput(string &code, std::vector<string> &code_words,
             std::vector<string> &results, std::vector<X> &result,
             X xmin, X xmax, X xdef, bool remove = false);

template <class X>
X getinput(string &code, std::vector<string> &code_words,
           std::vector<string> &results, X xmin, X xmax, X xdef,
           bool remove = false);

//!  An expression parser for the MORPH format.
/*!
  This generic expression parser can read single and multiple valued variables
  of any variable type used in EGSnrc by using theproperty of  polimorphism in C++ .
It has been written to comply with the MORPH input format  used in the EGSnrc user codes.

*/

class DE_Parser {

  private:
    int            output;
    string         delimeter;
    std::vector<string> code_words;
    std::vector<string> results;
    bool           decoded;
    bool           multiple_entries;

  public:

    DE_Parser(int out, char *delim, std::istream &stream, bool me = false);
    DE_Parser(size_t &n, string codes[], int out,
		char *delim, std::istream &stream, bool me = false);
    DE_Parser(std::vector<string> &codes, int out, char *delim,
		std::istream &stream, bool me = false);
    DE_Parser(std::vector<string> &codes, string &section, int out,
              const char *delim, bool me = false);
    DE_Parser(size_t &n, char **codes, int out,
		char *delim, std::istream &stream, bool me = false);
    std::vector<string> get_data() { return results; };
    string get_data(size_t i) { return results[i]; };
    int out() { return output; };

    void set_up(string &section, char *delim);

    int get_input(string &code, std::vector<int> &result);
    int get_input(string &code, std::vector<Float> &result);
    int get_input(string &code, std::vector<string> &result);

    int get_input(string &code, std::vector<int> &result,
                  int xmin, int xmax, int xdef);
    int get_input(string &code, std::vector<Float> &result,
                  Float xmin, Float xmax, Float xdef);

    int get_input(string &code, int xmin, int xmax, int xdef);
    Float get_input(string &code, Float xmin, Float xmax, Float xdef);
    bool get_input(string &code, bool xmin, bool xmax, bool xdef);

    string get_section(std::istream &file);
    bool   get_section(string &s, const char *delim);

  private:
	  string get_line(std::istream &file);
    size_t get_index(string &section, string &code, string &c1);
    string get_tokens(string &section, string &code, bool remove = false);

};

#endif
