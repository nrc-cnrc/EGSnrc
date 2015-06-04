
C##############################################################################
C
C  EGSnrc egs_date subroutine v3
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


C*************************************************************************
C
C egs_date(ounit): print a 11 char string in the form
C                     '18-Mar-2003'
C                  to the unit specified by ounit
C                  No end of line character is inserted
C
C Your compiler does not support the fdate or date_and_time intrinsics,
C but it has the date intrinsic. We are using it here, but the result
C is non-standard. If you want to have the standard string as above,
C fix this subroutine.
C
C*************************************************************************

      subroutine egs_date(ounit)
      integer ounit
      character dat*20
      dat(:len(dat)) = ' '
      call __date__(dat)
      write(ounit,'(a,$)') dat(:lnblnk1(dat))
      return
      end
