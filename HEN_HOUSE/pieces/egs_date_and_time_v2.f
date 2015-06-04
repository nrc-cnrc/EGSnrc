
C##############################################################################
C
C  EGSnrc egs_date_and_time subroutine v2
C  Copyright (C) 2015 National Research Council Canada
C
C  This file is part of EGSnrc.
C
C  EGSnrc is free software: you can redistribute it and/or modify it under
C  the terms of the GNU Affero General Public License as published by the
C  Free Software Foundation, either version 3 of the License, or (at your
C  option) any later version.
C
C  EGSnrc is distributed in the hope that it will be useful, but WITHOUT ANY
C  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
C  FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for
C  more details.
C
C  You should have received a copy of the GNU Affero General Public License
C  along with EGSnrc. If not, see <http://www.gnu.org/licenses/>.
C
C##############################################################################
C
C  Author:          Iwan Kawrakow, 2003
C
C  Contributors:
C
C##############################################################################


      subroutine egs_date_and_time(vnow)
      integer vnow(*)
      character string*24,tmp*2,tmp1*4,tmp2*3
      integer   egs_conver_month
      call __fdate__(string)
      tmp(1:2) = string(12:13)
      read(tmp,*) vnow(5)
      tmp(1:2) = string(15:16)
      read(tmp,*) vnow(6)
      tmp(1:2) = string(18:19)
      read(tmp,*) vnow(7)
      vnow(8) = 0
      tmp(1:2) = string(9:10)
      read(tmp,*) vnow(3)
      tmp1(1:4) = string(21:24)
      read(tmp1,*) vnow(1)
      tmp2(1:3) = string(5:7)
      vnow(2) = egs_conver_month(tmp2)
      vnow(4) = 0
      return
      end
