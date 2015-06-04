
C##############################################################################
C
C  EGSnrc date subroutines v2
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


C***************************************************************************
C
C   egs_fdate(out):  print a 24 char date and time string in the form
C                         'Tue Mar 18 08:16:42 2003'
C                    to the unit specified by out without end of line
C                    i.e. the sequence
C                    write(6,'(a,$)') 'Today is '
C                    call egs_fdate(6)
C                    write(6,'(a)') '. Have a nice date'
C                    should result in something like
C                    Today is Tue Mar 18 08:16:42 2003. Have a nice date
C                    printed to unit 6.
C
C***************************************************************************
      subroutine egs_fdate(ounit)
      integer ounit
      character*24 string
      call egs_get_fdate(string)
      write(ounit,'(a,$)') string
      return
      end

C***************************************************************************
C
C   egs_get_fdate(string) assignes a 24 char date and time string to string
C                         string must be at least 24 chars long, otherwise
C                         this subroutine has no effect.
C
C***************************************************************************

      subroutine egs_get_fdate(string)
      character*(*) string
      integer values(8),ind,iaux,i
      character dat*8,tim*10,zon*5,tmp*3
      if( len(string).lt.24 ) return
      call date_and_time(dat,tim,zon,values)
      call egs_weekday(values,tmp)
      string(1:3) = tmp(1:3)
      call egs_month(values(2),tmp)
      string(5:7) = tmp(1:3)
      string(9:10) = dat(7:8)
      string(12:13) = tim(1:2)
      string(14:14) = ':'
      string(15:16) = tim(3:4)
      string(17:17) = ':'
      string(18:19) = tim(5:6)
      string(21:24) = dat(1:4)
      return
      end


