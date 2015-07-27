/*
###############################################################################
#
#  EGSnrc summarize beam model source code
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
#  Author:          Zdenko Sego, 2005
#
#  Contributors:    Iwan Kawrakow
#                   Blake Walters
#
###############################################################################
*/


#include "sources.h"

/************************************************/
/* ring						*/
/************************************************/
ring::ring(int s,int c,int l)
{
   s_type = (source_type)s;
   charge = c;
   latch = l;

   relint = 0;
   particle_relint = 0;

   d = 0;
   IR = 0;
   OR = 0;
   virtualOR = 0;
}

void ring::put_pars(double *p)
{
   d = p[0];
   IR = p[1];
   OR = p[2];
   virtualOR = p[3];
}

void ring::print(void) const
{
   char *sname;

   if((IR == 0.0) && (OR == 0.0))
      sname = "Point Src";
   else
      sname = "Ring";

   cout << setw(W_SNAME) << sname;
   cout << setw(W_STYPE) << s_type;
   cout << setw(W_CHARGE) << charge;
   cout << setw(W_LATCH) << latch;
   cout << setiosflags(ios::fixed);
   cout << setw(W_D) << setprecision(P_D) << d;
//   cout << setw(W_EN) << setprecision(P_EN) << my_energy_spectrum->get_average(E_IN);
//   cout << setw(W_EN) << setprecision(P_EN) << my_energy_spectrum->get_average(E_OUT);
   cout << setw(W_RELINT) << setprecision(P_RELINT) << relint;
   cout << setw(W_RELINT) << setprecision(P_RELINT) << particle_relint;
   cout << endl;
}

void ring::print_parameters(void) const
{
   cout << "\tIR=" << IR << endl;
   cout << "\tOR=" << OR << endl;
   cout << "\tvirtualOR=" << virtualOR << endl;
}

/************************************************/
/* applicator					*/
/************************************************/
applicator::applicator(int s,int c, int l)
{
   s_type = (source_type)s;
   charge = c;
   latch = l;

   relint = 0;
   particle_relint = 0;

   d = 0;
   Xmin = 0;
   Xmax = 0;
   Ymin = 0;
   Ymax = 0;
   absXmax = 0;
   absYmax = 0;
}

void applicator::put_pars(double *p)
{
   d = p[0];
   Xmin = p[1];
   Xmax = p[2];
   Ymin = p[3];
   Ymax = p[4];
   absXmax= p[5];
   absYmax = p[6];
}

void applicator::print(void) const
{
   cout << setw(W_SNAME) << "Applicator";
   cout << setw(W_STYPE) << s_type;
   cout << setw(W_CHARGE) << charge;
   cout << setw(W_LATCH) << latch;
   cout << setiosflags(ios::fixed);
   cout << setw(W_D) << setprecision(P_D) << d;
//   cout << setw(W_EN) << setprecision(P_EN) << my_energy_spectrum->get_average(E_IN);
//   cout << setw(W_EN) << setprecision(P_EN) << my_energy_spectrum->get_average(E_OUT);
   cout << setw(W_RELINT) << setprecision(P_RELINT) << relint;
   cout << setw(W_RELINT) << setprecision(P_RELINT) << particle_relint;
   cout << endl;
}

void applicator::print_parameters(void) const
{
   cout << "\tXmin=" << Xmin << endl;
   cout << "\tXmax=" << Xmax << endl;
   cout << "\tYmin=" << Ymin << endl;
   cout << "\tYmax=" << Ymax << endl;
   cout << "\tabsXmax=" << absXmax << endl;
   cout << "\tabsYmax=" << absYmax << endl;
}


/************************************************/
/* collimator					*/
/************************************************/
collimator::collimator(int s,int c,int l)
{
   s_type = (source_type)s;
   charge = c;
   latch = l;

   relint = 0;
   particle_relint = 0;

   Xmin = 0;
   Xmax = 0;
   Ymin = 0;
   Ymax = 0;
   absXmax = 0;
   absYmax = 0;
   orientation = 0;
}

void collimator::put_pars(double *p)
{
   d = p[0];
   Xmin = p[1];
   Xmax = p[2];
   Ymin = p[3];
   Ymax = p[4];
   absXmax = p[5];
   absYmax = p[6];
   orientation = (int)p[7];
}

void collimator::print(void) const
{
   cout << setw(W_SNAME) << "Collimator";
   cout << setw(W_STYPE) << s_type;
   cout << setw(W_CHARGE) << charge;
   cout << setw(W_LATCH) << latch;
   cout << setiosflags(ios::fixed);
   cout << setw(W_D) << setprecision(P_D) << d;
//   cout << setw(W_EN) << setprecision(P_EN) << my_energy_spectrum->get_average(E_IN);
//   cout << setw(W_EN) << setprecision(P_EN) << my_energy_spectrum->get_average(E_OUT);
   cout << setw(W_RELINT) << setprecision(P_RELINT) << relint;
   cout << setw(W_RELINT) << setprecision(P_RELINT) << particle_relint;
   cout << endl;
}

void collimator::print_parameters(void) const
{
   cout << "\tXmin=" << Xmin << endl;
   cout << "\tXmax=" << Xmax << endl;
   cout << "\tYmin=" << Ymin << endl;
   cout << "\tYmax=" << Ymax << endl;
   cout << "\tabsXmax=" << absXmax << endl;
   cout << "\tabsYmax=" << absYmax << endl;
}

/************************************************/
/* circplane					*/
/************************************************/
circplane::circplane(int s,int c, int l)
{
   s_type = (source_type)s;
   charge = c;
   latch = l;

   relint = 0;
   particle_relint = 0;

   d = 0;
   R = 0;
}

void circplane::put_pars(double *p)
{
   d = p[0];
   R = p[1];
}

void circplane::print(void) const
{
  cout << setw(W_SNAME) << "Circ.plane";
  cout << setw(W_STYPE) << s_type;
  cout << setw(W_CHARGE) << charge;
  cout << setw(W_LATCH) << latch;
  cout << setiosflags(ios::fixed);
  cout << setw(W_D) << setprecision(P_D) << d;
//  cout << setw(W_EN) << setprecision(P_EN) << my_energy_spectrum->get_average(E_IN);
//  cout << setw(W_EN) << setprecision(P_EN) << my_energy_spectrum->get_average(E_OUT);
  cout << setw(W_RELINT) << setprecision(P_RELINT) << relint;
  cout << setw(W_RELINT) << setprecision(P_RELINT) << particle_relint;
  cout << endl;
}

void circplane::print_parameters(void) const
{
   cout << "\tR=" << R << endl;
}

/************************************************/
/* rectplane					*/
/************************************************/
rectplane::rectplane(int s,int c, int l)
{
   s_type = (source_type)s;
   charge = c;
   latch = l;

   relint = 0;
   particle_relint = 0;

   d = 0;
   Xmin = 0;
   Xmax = 0;
   Ymin = 0;
   Ymax = 0;
}

void rectplane::put_pars(double *p)
{
   d = p[0];
   Xmin = p[1];
   Xmax = p[2];
   Ymin = p[3];
   Ymax = p[4];
}

void rectplane::print(void) const
{
   cout << setw(W_SNAME) << "Rect.plane";
   cout << setw(W_STYPE) << s_type;
   cout << setw(W_CHARGE) << charge;
   cout << setw(W_LATCH) << latch;
   cout << setiosflags(ios::fixed);
   cout << setw(W_D) << setprecision(P_D) << d;
//   cout << setw(W_EN) << setprecision(P_EN) << my_energy_spectrum->get_average(E_IN);
//   cout << setw(W_EN) << setprecision(P_EN) << my_energy_spectrum->get_average(E_OUT);
   cout << setw(W_RELINT) << setprecision(P_RELINT) << relint;
   cout << setw(W_RELINT) << setprecision(P_RELINT) << particle_relint;
   cout << endl;
}

void rectplane::print_parameters(void) const
{
   cout << "\tXmin=" << Xmin << endl;
   cout << "\tXmax=" << Xmax << endl;
   cout << "\tYmin=" << Ymin << endl;
   cout << "\tYmax=" << Ymax << endl;
}
