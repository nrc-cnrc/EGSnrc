/*
###############################################################################
#
#  EGSnrc egs_inprz data input headers
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
#  Author:          Ernesto Mainegra-Hing, 2001
#
#  Contributors:    Blake Walters
#
###############################################################################
*/


#ifndef DATAINP_H
#define DATAINP_H

#include "de_parser.h"
#include "tools.h"

#include <qfile.h>
#include <fstream>
//qt3to4 -- BW
//#include <q3textstream.h>
//#include <q3table.h>

//qt3to4 -- BW
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextStream>

using namespace std;

//#define zap(x) if(x){delete(x); x=0;}

#define VERSION "2.0"

#define WARNING_USER      "couldn't guess user code\n using cavrznrc!"
#define WARNING_DEFAULTS  "<b>Error reading input file, defaults used instead.<br>" \
"(correct errors and press <i>Save</i> button to write defaults to file)</b><br>"\
                          "<br>"
//                          "<hr><br>"
#define WARNING_DEFAULT   "\nDefault used instead!"
#define WARNING_IWATCH    "Wrong IWATCH in input file!\n"
#define WARNING_STRAND    "Wrong value for random number store mode !\n"
#define WARNING_STRDAT    "Wrong value for data store mode !\n"
#define WARNING_IRESTART  "Wrong value for irestart mode !\n"
#define WARNING_OUTOPT    "Wrong value for output options !\n"
#define WARNING_ETRANS    "Wrong value for e- transport options !\n"
#define WARNING_SPROUT    "Wrong value for stopp. pwr out option !\n"
#define WARNING_IFULL     "Wrong value for ifull option !\n"
#define WARNING_KERMA     "Wrong value for kerma option !\n"
#define WARNING_PHOTREG   "Wrong value for photon regeneration !\n"
#define WARNING_INP_METHOD "Wrong value for input method !\n"
#define WARNING_MEDIA_DESCRIPTION "Wrong value for media description !\n"

enum UserCodeType { cavrznrc, dosrznrc, sprrznrc, flurznrc };

/*
typedef std::vector<char*>  v_ptr;
typedef std::vector<int>    v_int;
typedef std::vector<float>  v_float;
typedef std::vector<string> v_string;
*/
/*
 ***********************************************************
 *
 *                  U T I L I T Y               F U N C T I O N S
 *  (global functions, maybe I should create a general utility class)
 *
 ***********************************************************
 */
QString parseStr( std::ifstream & in, const QString & id );
// defined in inputblock.cpp:
QString getIt( string &code,
              QString def,
              QString & error,
              DE_Parser *p );
//qt3to4 -- BW
void print_delimeter( const QString& boundary,
                      const QString& section,
                      //Q3TextStream &t );
                      QTextStream &t );
/*
// defined in inputRZImplemantation.cpp :
bool check_file( const QString & fname );
bool set_file( const QString& ext, const QString& name );
void chmod( const QString & attrib, const QString file );
bool copy(const QString& source, const QString& target);
bool getVariable(QIODevice *inp, const QString &key, QString &value);
QString simplifyFileNames( const QString& str );
QString stripRepetitions( const QString& str , const QString& expr );
QString simplifySeparators( const QString& str );
QString strightenItUp( const QString& str );
QString forwardItUp( const QString& str );
QString ironIt( const QString& str );
*/
/*
 ***********************************************************
 *
 *                 O P E R A T O R S
 *
 ***********************************************************
 */
/*
QTextStream & operator << ( QTextStream & ts, v_int & v );
QTextStream & operator << ( QTextStream & ts, v_float & v );
QTextStream & operator << ( QTextStream & ts, v_string & v );
QTextStream & operator << ( QTextStream & ts, string & str );
QTextStream & operator >> ( QTextStream & ts, string & str );
*/
/*
 ***********************************************************
 *
 *                  T E M P L A T E S
 *
 ***********************************************************
 */

template <class X>
std::vector<X> getThemAll( string &code, std::vector<X> mydef,
                           QString & error, DE_Parser *p )
{
    std::vector<X> result;
    X def = mydef[0];
    if ( p->get_input( code, result ) )
    {
         error += "value sought not found for " ;
         error += code.c_str(); error += "<br>";
         return mydef;
    }
    else return result;
}

template <class X>
std::vector<X> getThem( string &code, X min, X max, std::vector<X> mydef,
                        QString & error, DE_Parser *p )
{
    std::vector<X> result;
    X def = mydef[0];
    if ( p->get_input( code, result, min, max, def ) )
    {
         error += "value sought not found for " ;
         error += code.c_str(); error += "<br>";
         return mydef;
    }
    else return result;
}

// use this to catch errors in input file when reading single int,
// bool or float values if you are happy with just using the default,
// call get_input directly instead
template <class X>
X getItsafe( string &code, X xmin, X xmax, X xdef,
             QString & error, DE_Parser *p )
{
	X res;
	res = p->get_input( code, xmin, xmax, xdef );
	if ( res == xdef )
    	{
	    error += "wrong value sought or not found for " ;
            error += code.c_str(); error += "<br>";
	}
	return res;
}

template <class X>
int delete_element( std::vector<X> * x, X e )
{
     int i =0;
     //std::vector<X>::iterator iter(x->begin());
     typename vector<X>::iterator iter = x->begin();
     while ( iter < x->end() ) {
         if ( *iter == e ) {     // item detected
              x->erase(iter);    // delete item
              return i;           // return position
         }
         iter++;              // advance to next different item
         i++;
      }
    return x->size()+1; // no item detected;
                        // set vacuum position outside vector range
 }

template <class X>
std::vector<X> del_element( std::vector<X> v, X e )
{
     std::vector<X> x = v;
     //std::vector<X>::iterator iter(x.begin());
     typename vector<X>::iterator iter = x.begin();
     while ( iter < x.end() ) {
         if ( *iter == e ) {     // item detected
              x.erase(iter);        // delete item
         }
         iter++;              // advance to next different item
      }

     return x;
 }

template <class X>
std::vector<X> strip_repetitions( std::vector<X> v )
{
    std::vector<X> x = v;
    //std::vector<X>::iterator iter1(x.begin());
    typename vector<X>::iterator iter1 = x.begin();
    while ( iter1 != x.end() ) {
        //std::vector<X>::iterator iter2(iter1);
        typename vector<X>::iterator iter2 = iter1;
        iter2++;
        while ( iter2 != x.end() ) {
            if ( *iter1 == *iter2 ) { // item repetition detected
                x.erase(iter2);        // delete repeated item
            }
            else {
                iter2++;
            }
        }
        iter1++;              // advance to next different item
    }

    return x;
}

//**********************************************
// *********        TABLE TEMPLATES      ***********
//**********************************************

//! Retrieves the contents from column \em col of table \em t to \result
/*!
This table utility is a generic template that can retirieve text from a cell
and convert it to \em any type trough the use of a TextStream. The content
of column \col is passed to a vector of type X.
*/
template <class X>
void get_col_content( const int &col, QTableWidget* t, std::vector<X> &result)
{
    X val;
    result.clear(); // reseting values, so clear before
    //qt3to4 -- BW
    //for ( int i = 0; i < t->numRows(); i++ ){
        //str = t->text(i,col);
     for ( int i = 0; i < t->rowCount(); i++ ){
        QTableWidgetItem *qtwi = t->item(i,col);
        QString str;
        if (qtwi) str = qtwi->text();

        if (!str.isEmpty()) { // not an empty cell
            //qt3to4 -- BW
            //Q3TextStream ts( str, QIODevice::ReadWrite );
            QTextStream ts( &str, QIODevice::ReadWrite );
            ts >> val;
            result.push_back( val );
        }
        else{// got empty cell
            //result.push_back( val );
        }
    }

}

//! Retrieves the contents from column \em col of table \em t to \result
/*!
This table utility is a generic template that can retrieve text from a cell
and convert it to \em any type trough the use of a TextStream. The content
of column \col is passed to a vector of type X. If the cell is empty, a default
value is used.
*/
template <class X>
//qt3to4 -- BW
//void get_col_explicit( const int &col, Q3Table* t, std::vector<X> &result, X def)
void get_col_explicit( const int &col, QTableWidget* t, std::vector<X> &result, X def)
{
    X val;
    result.clear(); // reseting values, so clear before
    //qt3to4 -- BW
    //for ( int i = 0; i < t->numRows(); i++ ){
        //str = t->text(i,col);
    for ( int i = 0; i < t->rowCount(); i++ ){
        QTableWidgetItem *qtwi = t->item(i,col);
        QString str;
        if (qtwi) str = qtwi->text();
        if (!str.isEmpty()) { // not an empty cell
            //qt3to4 -- BW
            //Q3TextStream ts( str, QIODevice::ReadWrite );
            QTextStream ts( &str, QIODevice::ReadWrite );
            ts >> val;
            result.push_back( val );
        }
        else{// got empty cell
            result.push_back( def );
        }
    }

}

//! Updates contents of \em count columns in table \em t starting from column \ini
/*!
This table utility is a generic template takes \em an array of vectors of any type
and updates the content of \count columns starting from column \ini. The values
in the vectors are passed to the table by constructing a text stream that operates
on the Unicode QString, \em str, through an internal device. Once this QString is
updated, it is used to set the text in the corresponding cell.
*/
template <class X>
//qt3to4 -- BW
//void update_table(std::vector<X> *v, int ini, int count, Q3Table* t)
void update_table(std::vector<X> *v, int ini, int count, QTableWidget* t)
{

    //clear_table(t); ==> member of class inputRZImpl and this is global

    int i;

    //qt3to4 -- BW
    //for ( i = 0; i < t->numRows(); i++){
    for ( i = 0; i < t->rowCount(); i++){
        for ( int j = ini; j < count; j++ ){
            //qt3to4 -- BW
            //t->clearCell( i, j );
            t->setItem(i,j,0);
        }
    }

    for ( i = ini; i < count; i++) {
        //std::vector<X>::iterator iter(v[i].begin());
        typename vector<X>::iterator iter = v[i].begin();
        int j = 0;
        unsigned short int vsize = v[i].size();
        //qt3to4 -- BW
        //if ( vsize > t->numRows()) t->setNumRows( vsize );
        if ( vsize > t->rowCount()) t->setRowCount( vsize );
        while ( iter != v[i].end()) {
            QString str;
            //qt3to4 -- BW
            //Q3TextStream ts( &str, QIODevice::WriteOnly );
            QTextStream ts( &str, QIODevice::WriteOnly );
            ts << *iter++;
            //qt3to4 -- BW
            //t->setText( j, i, str );
            t->setItem(j,i, new QTableWidgetItem(str));
            //t->setText( j, i, str.setNum(*iter++,10) );
            j++;
        }
    }

}


#endif	// DATAINP_H
