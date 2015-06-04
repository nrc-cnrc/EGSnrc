
C##############################################################################
C
C  EGSnrc hostname subroutines v2
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


C*****************************************************************************
C
C Print the host name to the unit specified by ounit without inserting
C a new line character.
C
C As your compiler does not have a working version of the hostnm
C intrinsic, we attempt to obtain the hostname from various environment
C variables. This is NOT guaranteed to work. If it does not, you have
C to provide your own implementation if you wish to have the name of the
C host your program is running on to the output files.
C
C*****************************************************************************

      subroutine egs_print_hostnm(ounit)
      integer ounit
      character*256 string
      call egs_get_hostnm(string)
      write(ounit,'(a,$)') string(:lnblnk1(string))
      return
      end

C*****************************************************************************
C
C Assign the host name to the string pointed to be hname.
C
C See comments above !
C
C*****************************************************************************

      subroutine egs_get_hostnm(hname)
      character*(*) hname
      character*256 string
      string(:len(string)) = ' '
      call getenv('COMPUTERNAME',string)
      if( lnblnk1(string).eq.0 ) then
        call getenv('HOSTNAME',string)
        if( lnblnk1(string).eq.0 ) then
          call getenv('HOST',string)
          if( lnblnk1(string).eq.0 ) then
            string(:lnblnk1('egs_get_hostnm: fixme'))=
     &          'egs_get_hostnm: fixme'
            return
          end if
        end if
      end if
      l1 = lnblnk1(string)
      l2 = len(hname)
      hname(:l2) = ' '
      l = min(l1,l2)
      hname(:l) = string(:l)
      return
      end

