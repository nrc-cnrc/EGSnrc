/*
###############################################################################
#
#  EGSnrc egs_inprz beam source dialog headers
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
#  Author:          Ernesto Mainegra-Hing, 2006
#
#  Contributors:    Blake Walters
#
###############################################################################
*/


#ifndef BEAMSRCDLG_H
#define BEAMSRCDLG_H

#include <qcombobox.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qlayout.h>
//qt3to4 -- BW
//#include <q3buttongroup.h>
#include <qlineedit.h>


class BeamSourceDlg : public QDialog
{
  Q_OBJECT

    public:
      BeamSourceDlg(QWidget * parent, const char * name,
                    QStringList* beamuc,
                    QStringList* beaminp,
                    QStringList* pegs);
      ~BeamSourceDlg(){};
    void update_pegs( const QStringList pegsfiles );
    QString beam_code(){return beam->currentText();};
    QString beam_inp() {return  inp->currentText();};
    QString beam_pegs(){return  pegs->currentText();};
    QString beam_min(){return minWeight->text();};
    QString beam_max(){return maxWeight->text();};
    QString beam_dist(){return dist->text();};
    QString beam_angle(){return angle->text();};
    QString beam_xoffset(){return xoffset->text();};
    QString beam_yoffset(){return yoffset->text();};
    QString beam_zoffset(){return zoffset->text();};
    QComboBox* get_beam(){return beam;};
    QComboBox* get_inp(){return inp;};
    QComboBox* get_pegs(){return pegs;};
    QLineEdit* get_min(){return minWeight;};
    QLineEdit* get_max(){return maxWeight;};
    QLineEdit* get_dist(){return dist;};
    QLineEdit* get_angle(){return angle;};
    QLineEdit* get_zoffset(){return zoffset;};
    QLineEdit* get_xoffset(){return xoffset;};
    QLineEdit* get_yoffset(){return yoffset;};
    void set_beam_code(const QString& s){beam->setItemText(beam->currentIndex(),s);};
    void set_beam_inp(const QString& s) {inp->setItemText(inp->currentIndex(),s);};
    void set_beam_pegs(const QString& s){pegs->setItemText(pegs->currentIndex(),s);};
    void set_beam_min(const QString& s){minWeight->setText(s);};
    void set_beam_max(const QString& s){maxWeight->setText(s);};
    void set_beam_dist(const QString& s){dist->setText(s);};
    void set_beam_angle(const QString& s){angle->setText(s);};
    void set_beam_xoffset(const QString& s){xoffset->setText(s);};
    void set_beam_yoffset(const QString& s){yoffset->setText(s);};
    void set_beam_zoffset(const QString& s){zoffset->setText(s);};

public slots:
        void beamUserCodeChanged(const QString& newDir);
virtual void close();
        void cancel();

    signals:
     void beamSourceDefined();

    private:
     QComboBox* beam;
     QComboBox* inp;
     QComboBox* pegs;
     QLineEdit* minWeight;
     QLineEdit* maxWeight;
     QLineEdit* dist;
     QLineEdit* angle;
     QLineEdit* xoffset;
     QLineEdit* yoffset;
     QLineEdit* zoffset;
     QString pegsDir;
     QString EGS_HOME;
};

#endif
