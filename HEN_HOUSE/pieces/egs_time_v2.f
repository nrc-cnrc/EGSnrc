
C##############################################################################
C
C  EGSnrc egs_time subroutine v2
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
C
C  Fixed bugs in this version of egs_time. They were never noticed before
C  because they were never needed. But with gcc 4.0 a lot of the data and
C  time stuff is not implemented and therefore these 2 get used.
C
C##############################################################################



C*************************************************************************
C
C egs_time(ounit): print a 8 char string in the form hh:mm:ss
C                  to the unit specified by ounit
C                  No end of line character is inserted
C
C*************************************************************************

      subroutine egs_time(ounit)
      integer ounit
      character dat*8,tim*10,zon*5,tmp*3,timen*8
      integer values(8)
      call date_and_time(dat,tim,zon,values)
      timen(1:2)=tim(1:2)
      timen(3:3) = ':';
      timen(4:5) = tim(3:4)
      timen(6:6) = ':'
      timen(7:8) = tim(5:6)
      write(ounit,'(a,$)') timen
      return
      end
