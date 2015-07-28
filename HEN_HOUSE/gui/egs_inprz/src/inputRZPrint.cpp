/*
###############################################################################
#
#  EGSnrc egs_inprz print
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


#include "inputRZImpl.h"
#include <qapplication.h>
#include <qfile.h>
//#include <q3filedialog.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qcombobox.h>
//Added by qt3to4:
//#include <Q3TextStream>
#include <iterator>
#include <qdir.h>
//#include <q3progressdialog.h>
//#include <q3buttongroup.h>
#include <qwidget.h>
#include <qpainter.h>
//#include <q3paintdevicemetrics.h>
#include <qprinter.h>
#include <qdatetime.h>

//qt3to4 -- BW
#include <QTextStream>
#include <QProgressDialog>
#include <QPrintDialog>

//**********************************************
// *********      PRINTING STUFF      ***********
//**********************************************
int inputRZImpl::TotalTextLines( const QString& fname)
{
       QFile              fi( fname );
       //qt3to4 -- BW
       //Q3TextStream ts( &fi );
       QTextStream ts( &fi );
       QString          t;
       int numLines = 0;
       if ( fi.open( QIODevice::ReadOnly ) ) {
           do {
                t = ts.readLine();
	  numLines++;
          //qt3to4 -- BW
           //} while ( !ts.eof() );
          } while ( !ts.atEnd() );
       }
       fi.close();
       return numLines;
}

void inputRZImpl::print()
{
    QString fname = EGSdir + EGSfileName;
    QPrinter* printer = new QPrinter;
    printer->setPageSize( QPrinter::Letter );
    const int Margin = 20;
    const int MarginY = 30;
    int pageNo = 1;

    QPrintDialog pdialog(printer,this);

    if ( pdialog.exec()) {

cout << "let's print !!!" << endl;


        QPainter paint( printer );
        //if( !paint.begin( printer ) )              // paint on printer
        //     return;

       static const char *fonts[] = { "Helvetica", "Courier", "Times", 0 };
       static int	 sizes[] = { 10, 12, 18, 24, 36, 0 };
       int yPos = 0;

       QFont fontNormal( fonts[1], sizes[0] );
       QFont fontBold( fonts[0], sizes[0] );
       fontBold.setWeight( QFont::Black);

       paint.setFont( fontBold );
       QFontMetrics fmBold = paint.fontMetrics();
       paint.setFont( fontNormal );
       QFontMetrics fm = paint.fontMetrics();

       //qt3to4 -- BW
       //Q3PaintDeviceMetrics metrics( printer ); // need width/height

       QFile              fi( fname );
       //qt3to4 -- BW
       //Q3TextStream ts( &fi );
       QTextStream ts( &fi );
       QString          t;
       int StepsPerPage =  500;

       int LinesPerPage = (printer->height() - 2* MarginY) / fm.lineSpacing();
       if ( fi.open( QIODevice::ReadOnly ) ) {

            int NumOfPages = 1 + TotalTextLines( fname ) / LinesPerPage;
            int totalSteps = StepsPerPage * NumOfPages;

            QString msg( "Printing (page " );
                         msg += QString::number( pageNo );
                         msg += ")...";
            //qt3to4 -- BW
            //Q3ProgressDialog* dialog = new Q3ProgressDialog(msg, "Cancel",
             //                                                               totalSteps, this, "progress", true);
            QProgressDialog* dialog = new QProgressDialog(msg, "Cancel",0,totalSteps);
            //dialog->setTotalSteps( totalSteps );
            //dialog->setProgress( 0 );
            dialog->setValue(0);
            dialog->setMinimumDuration( 0 );

            int progress_count = 0;

           QString gui = "EGSnrc GUI for RZ Geometries:  " + EGSfileName;
           QDate currDate = QDate::currentDate();
           QString date = currDate.toString ( "dd.MM.yyyy" );
           paint.setFont( fontBold );
           paint.drawText( Margin, 0,  printer->width() - 2*Margin, fmBold.lineSpacing(),
                        Qt::AlignLeft | Qt::RichText, gui );
           paint.drawText( Margin, 0,  printer->width() - 2*Margin, fmBold.lineSpacing(),
                        Qt::AlignRight, date );
           paint.drawLine( Margin, MarginY - 10, printer->width() - Margin, MarginY - 10 );
           paint.drawLine( Margin, printer->height() - MarginY + 10,
                                    printer->width() - Margin, printer->height() - MarginY + 10 );
           paint.drawText( Margin, printer->height() - fmBold.lineSpacing(),
	                       printer->width(), fmBold.lineSpacing(),
                                     Qt::AlignHCenter, QString::number( pageNo ) );
           paint.setFont( fontNormal );

           do {
                progress_count++;
                qApp->processEvents();
                if ( dialog->wasCanceled() )
                     dialog->close();
                t = ts.readLine();

                if ( MarginY + yPos + fm.lineSpacing() > printer->height() - MarginY ) {
                         do{
                                    //dialog->setProgress( ++progress_count );
                                    dialog->setValue(++progress_count );
                         }while ( progress_count < StepsPerPage*pageNo );
	           msg = "Printing (page " + QString::number( ++pageNo ) + ")...";
	           dialog->setLabelText( msg );
                         printer->newPage();             // no more room on this page
                         yPos = 0;                       // back to top of page
                         paint.setFont( fontBold );
                         paint.drawText( Margin, 0,  printer->width() - 2*Margin, fmBold.lineSpacing(),
                                                   Qt::AlignLeft | Qt::RichText, gui );
                         paint.drawText( Margin, 0,  printer->width() - 2*Margin, fmBold.lineSpacing(),
                                                  Qt::AlignRight, date );
                        paint.drawLine( Margin, MarginY - 10, printer->width() - Margin, MarginY - 10 );
	          paint.drawLine( Margin, printer->height() - MarginY + 10,
                                    printer->width() - Margin, printer->height() - MarginY + 10 );
                        paint.drawText( Margin, printer->height() - fmBold.lineSpacing(),
	                       printer->width(), fmBold.lineSpacing(),
                                     Qt::AlignHCenter, QString::number( pageNo ) );
                         paint.setFont( fontNormal );
                }

                paint.drawText( Margin, MarginY + yPos,
                        printer->width(), fm.lineSpacing(),
                        Qt::TextExpandTabs | Qt::TextDontClip, t );

                yPos = yPos + fm.lineSpacing();

               dialog->setValue( progress_count );

           //qt3to4 -- BW
           //} while ( !ts.eof() );
          } while ( !ts.atEnd() );

          do{
               dialog->setValue( ++progress_count );
           }while ( progress_count < totalSteps );

          dialog->setValue( totalSteps );
          zap(dialog);
       }
       else {
               QMessageBox::critical( 0,
                        tr("Error printing "),
	          tr("Couldn't open file %1").arg(fname),
                        tr("Quit") );
       }

       fi.close();

    }
    else {
             QMessageBox::information( this, "Printing status", "Printing aborted");
    }

    delete printer;
}
