/*
###############################################################################
#
#  EGSnrc egs_inprz DE parser
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


#include "de_parser.h"

#include <cstring>
#include <iostream>
#include <algorithm>
#ifdef WIN32
#include <sstream>
#else
#include <strstream>
#endif
#include <fstream>

// according to my C++ book, the method in class string
// that removes portions of a string is called 'erase',
// in the g++ Linux compiler based on gcc 2.7.2.2.f.2
// this method is 'remove' (there is no method 'erase' at all)
// To make things easier, below is a pre-processor definition
// which is either 'erase' or 'remove', depending on the
// compiler


#define DE_ERASE erase
// #define DE_ERASE remove

template <class X>
int getinput(string &code, std::vector<string> &code_words,
             std::vector<string> &results, std::vector<X> &result, bool remove)
{
  int i;
  std::vector<string>::iterator pc = code_words.begin();
  std::vector<string>::iterator pr = results.begin();
  for(i=0; i<code_words.size(); i++,pc++,pr++) {
    if(code == code_words[i]) {
       //istrstream in(results[i].c_str());
	STR_STREAM(results[i],junk,in);
       int error = 0;
       while ( 1 ) {
         X tmp;
         in >> tmp;
         if( !in.fail() ) {
           result.push_back(tmp);
         }
         if( in.eof() ) {
           if( result.size() <= 0) error = 1;
           break;
         }
         if( !in.good() ) {
           if( result.size() <= 0) error = 2;
           break;
         }
       }
	   delete junk;
       if( remove ) {
         code_words.erase(pc); results.erase(pr);
       }
       return error;
    }
  }
  return 2;
}

template <class X>
int getinput(string &code, std::vector<string> &code_words,
             std::vector<string> &results, std::vector<X> &result,
             X xmin, X xmax, X xdef,bool remove)
{
  int error = getinput(code,code_words,results,result,remove);
  if( error ) return error;
  for (int i=0; i<result.size(); i++)
    if( result[i] < xmin || result[i] > xmax ) result[i] = xdef;
  return error;
}

template <class X>
X getinput(string &code, std::vector<string> &code_words,
           std::vector<string> &results, X xmin, X xmax, X xdef,
           bool remove)
{
  int i;
  std::vector<string>::iterator pc = code_words.begin();
  std::vector<string>::iterator pr = results.begin();
  X res;
  for(i=0; i<code_words.size(); i++,pc++,pr++) {
    if(code == code_words[i]) {
	STR_STREAM(results[i],junk,in);
       in >> res;
       if( !in.fail() ) {
         if( res < xmin || res > xmax ) res = xdef;
       }
       else res = xdef;
       if(remove) {
         code_words.erase(pc); results.erase(pr);
       }
	   delete junk;
       return res;
    }
  }
  return xdef;
}

int DE_Parser::get_input(string &code, std::vector<int> &result) {
  return getinput(code, code_words, results, result, multiple_entries);
}

int DE_Parser::get_input(string &code, std::vector<Float> &result) {
  return getinput(code, code_words, results, result, multiple_entries);
}

int DE_Parser::get_input(string &code, std::vector<string> &result) {
  return getinput(code, code_words, results, result, multiple_entries);
}

int DE_Parser::get_input(string &code, std::vector<int> &result,
                         int xmin, int xmax, int xdef) {
  return getinput(code, code_words, results, result, xmin, xmax, xdef,
                  multiple_entries);
}

int DE_Parser::get_input(string &code, std::vector<Float> &result,
                         Float xmin, Float xmax, Float xdef) {
  return getinput(code, code_words, results, result, xmin, xmax, xdef,
                  multiple_entries);
}

int DE_Parser::get_input(string &code, int xmin, int xmax, int xdef)
{
  return getinput(code, code_words, results, xmin, xmax, xdef,
                  multiple_entries);
}

Float DE_Parser::get_input(string &code, Float xmin, Float xmax, Float xdef)
{
  return getinput(code, code_words, results, xmin, xmax, xdef,
                  multiple_entries);
}

bool DE_Parser::get_input(string &code, bool xmin, bool xmax, bool xdef)
{
  return getinput(code, code_words, results, xmin, xmax, xdef,
                  multiple_entries);
}

DE_Parser::DE_Parser(int out, char *delim, std::istream &stream, bool me)
{
  delimeter = delim;
  output = out;
  results.push_back(get_section(stream));
  decoded = false;
  multiple_entries = me;
}

DE_Parser::DE_Parser(size_t &n, string codes[], int out,
					 char *delim, std::istream &stream, bool me)
{
  delimeter = delim;
  output = out;
  multiple_entries = me;
  size_t i;
  string s = get_section(stream);
  for(i=0; i<n; i++) {
    RETRY: ;
    string aux = get_tokens(s,codes[i],me);
    if( aux.size() > 0 ) {
      results.push_back(aux);
      code_words.push_back(codes[i]);
      if( me ) goto RETRY;
    }
  }
  decoded = true;
}

DE_Parser::DE_Parser(std::vector<string> &codes, int out,
					 char *delim, std::istream &stream, bool me)
{
  delimeter=delim;
  output = out;
  size_t i;
  string s = get_section(stream);
  for(i=0; i<codes.size(); i++) {
    RETRY: ;
    string aux = get_tokens(s,codes[i],me);
    if( aux.size() > 0 ) {
      results.push_back(aux);
      code_words.push_back(codes[i]);
      if( me ) goto RETRY;
    }
  }
  decoded = true;
  multiple_entries = me;
}

DE_Parser::DE_Parser(std::vector<string> &codes, string &section,
                     int out, const char *delim, bool me)
{
  delimeter=delim;
  output = out;
  size_t i;
  STR_STREAM(section,junk,in);
  string s = get_section(in);
  for(i=0; i<codes.size(); i++) {
    RETRY: ;
    string aux = get_tokens(s,codes[i],me);
    if( aux.size() > 0 ) {
      results.push_back(aux);
      code_words.push_back(codes[i]);
      if( me ) goto RETRY;
    }
  }
  decoded = true;
  multiple_entries = me;
  delete junk;
}

DE_Parser::DE_Parser(size_t &n, char **codes, int out,
					 char *delim, std::istream &stream, bool me)
{
  delimeter = delim;
  output = out;
  size_t i; string tmp;
  for(i=0; i<n; i++) { tmp = codes[i]; code_words.push_back(tmp); }
  string s = get_section(stream);
  for(i=0; i<n; i++) {
    string tmp = codes[i];
    RETRY: ;
    string aux = get_tokens(s,tmp,me);
    if( aux.size() > 0 ) {
      results.push_back(aux);
      code_words.push_back(codes[i]);
      if( me ) goto RETRY;
    }
  }
  decoded = true;
  multiple_entries = me;
}

string DE_Parser::get_line(std::istream &file) {
  char buf[1024];
  size_t ind;
  file.getline(buf,1024); string s(buf);
  if( (ind = s.find("#")) < s.size() ) s = s.DE_ERASE(ind);
  if( !s.size() ) return s;
  if( (ind = s.find("!")) < s.size() ) s = s.DE_ERASE(ind);
  if( !s.size() ) return s;
  // remove blanks from the end;
  ind=s.size()-1;
  while( ind > 0 && isspace(s[ind]) ) ind--;
  s = s.DE_ERASE(ind+1,s.size()-ind-1);
  if( !s.size() ) return s;
  // remove blanks from begin
  ind=0;
  while( ind < s.size() && isspace(s[ind]) ) ind++;
  s = s.DE_ERASE(0,ind);
  return s;
}

bool DE_Parser::get_section(string &s, const char *delim) {
  if (decoded) return false;
  string d = delim;
  std::transform(d.begin(),d.end(),d.begin(),toupper);
  size_t ind;
  while( (ind = d.find(" ")) < d.size() ) d = d.DE_ERASE(ind,(size_t)1);
  string start_delimeter(":START");
  string end_delimeter(":STOP");
  start_delimeter = start_delimeter + d + ":";
  end_delimeter = end_delimeter + d + ":";
  string sb,se;
  size_t indb = get_index(results[0], start_delimeter, sb);
  if( indb > results[0].size() ) return false;
  size_t inde = get_index(results[0], end_delimeter, se);
  if( inde > results[0].size() ) return false;
  if( indb >= inde ) return false;
  s.assign(results[0],indb,inde-indb);
  int i,j = inde;
  for(i=0; i<se.size(); i++)
    for( ; j<results[0].size() && toupper(results[0][j]) != se[i]; j++);
  results[0].DE_ERASE(indb,j+1-indb);
  j = 0;
  for(i=0; i<sb.size(); i++)
    for( ; j<s.size() && toupper(s[j]) != sb[i]; j++);
  s.DE_ERASE(0,j+1);
  return true;
}

string DE_Parser::get_section(std::istream &file) {

  string d = delimeter;
  size_t ind;
  string result;

  // rewind the file
  file.clear();
  file.seekg(0);

  std::transform(d.begin(),d.end(),d.begin(),toupper);

  string s;
  if( d == "NONE" ) {
    while( !file.eof() && file.good() ) {
      s = get_line(file);
      if( !s.empty() ) {
        result += s; ind=result.size()-1;
        if( result[ind] == ',' || result[ind] == '\\') ;
        else result += "\n";
      }
    }
    return result;
  }

  while( (ind = d.find(" ")) < d.size() ) d = d.DE_ERASE(ind,(size_t)1);
  string start_delimeter("START");
  string end_delimeter("STOP");

  start_delimeter = start_delimeter + d;
  end_delimeter = end_delimeter + d;

  string message = "DE_Parser::get_section():  ";
  int k;
  while( !file.eof() && file.good() ) {
    s = get_line(file);
    std::transform(s.begin(),s.end(),s.begin(),toupper);
    while((ind = s.find(" ")) < s.size() ) s = s.DE_ERASE(ind,(size_t)1);
    if( s.find(start_delimeter) < s.size() ) { // found begin of section
      while( !file.eof() ) {
        s = get_line(file);
        if( !s.empty() ) {
          string temp(s);
		  std::transform(temp.begin(),temp.end(),temp.begin(),toupper);
          while((ind = temp.find(" ")) < temp.size() )
              temp = temp.DE_ERASE(ind,(size_t)1);
          if( temp.find(end_delimeter) < temp.size() ) {
            return result;
          }
          result += s; ind=result.size()-1;
          if( result[ind] == ',' || result[ind] == '\\') ;
          else result += "\n";
        }
      }
      if( output == 2 ) {
        message += "found start delimeter '";
        message += delimeter; message += "'";
        int len = message.size();
        message += "\n                              ";
        message += "but no corresponding end delimeter";
        message += "\n                              ";
        message += "Check your input file!";
		std::cout << std::endl;
        for(k=0; k<len; k++) std::cout << '*';
        std::cout << std::endl << message.c_str() << std::endl;
        for(k=0; k<len; k++) std::cout << '*';
        std::cout << std::endl << std::endl;
      }
      return result;
    }
  }
  if( output == 1 ) {
    message += "delimeter '";
    message += delimeter;
    message += "' not found.";
    std::cout << std::endl;
    for(k=0; k<message.size(); k++) std::cout << '*';
    std::cout << std::endl << message.c_str() << std::endl;
    for(k=0; k<message.size(); k++) std::cout << '*';
    std::cout << std::endl << std::endl;
  }
  return result;
}

size_t DE_Parser::get_index(string &section, string &code, string &c1) {

  string s(section), c(code);
  size_t ind;

  std::transform(s.begin(),s.end(),s.begin(),toupper);
  std::transform(c.begin(),c.end(),c.begin(),toupper);
  size_t len;
  len = s.size();
  std::vector<int> sb(len,0);
  size_t i,j=0,old_ind=0;
  while((ind = s.find(" ")) < s.size() ) {
    for(i=old_ind; i<ind; i++) sb[i]=j; old_ind=ind; j++;
    s = s.DE_ERASE(ind,(size_t)1);
  }
  while((ind = c.find(" ")) < c.size() ) c = c.DE_ERASE(ind,(size_t)1);
  ind = s.find(c);
  if( ind < s.size() ) { c1 = c; return ind+sb[ind]; }

  string message; int k,len1;
  if( output == 2 ) {
    message = "DE_Parser::get_index   Sought value '";
    message += code + "' not found";
    len1 = message.size();
    std::cout << std::endl;
    for(k=0; k<len1; k++) std::cout << '*';
    std::cout << std::endl << message.c_str() << std::endl;
    message =
     "               will try to catch simple spelling errors";
    std::cout << message.c_str() << std::endl;
  }
  char tmp;
  for(i=0; i<c.size()-1; i++) {
    c1 = c; tmp = c1[i+1]; c1[i+1] = c1[i]; c1[i] = tmp;
    ind = s.find(c1);
    if(ind < s.size() ) {
      if( output == 2 ) {
        std::cout << "Found " << c1.c_str() << " instead of " << c.c_str() << "!\n";
        for(k=0; k<len1; k++) std::cout << '*'; std::cout << std::endl;
      }
      return ind+sb[ind];
    }
  }
  if( output == 2 ) {
    message =
    "                          failed.";
    std::cout << message.c_str() << std::endl;
    for(k=0; k<len1; k++) std::cout << '*';
    std::cout << std::endl << std::endl;
  }

  c1 = c; return section.size();
}

string DE_Parser::get_tokens(string &section, string &code,
                             bool remove) {
  size_t ind,i,j;
  string result,found;

  ind = get_index(section,code,found);
  if( ind >= section.size() ) return result; // i.e. return an empty string
  result = result.assign(section,ind,section.size()-ind);
  for(i=0; i<found.size(); i++) {
    for(j=0; j<result.size() && toupper(result[j]) != found[i]; j++);
    result.DE_ERASE(0,j+1);
    if(remove) section.DE_ERASE(ind,j+1);
  }
  ind = result.find("\n");
  result = result.DE_ERASE(ind);
  // remove =,: if it is the first non-blanck char
  ind = result.find_first_not_of(" ");
  if( result[ind] == '=' || result[ind] == ':' )
       result = result.DE_ERASE(ind,1);
  // replace commas, line continuators with blanks
  for(ind=0; ind<result.size(); ind++) {
    if( result[ind] == ',' || result[ind] == '\\' || result[ind] == ';')
       result[ind] = ' ';
  }
  return result;
}
