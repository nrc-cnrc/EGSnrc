
C##############################################################################
C
C  EGSnrc egs_time subroutine v3
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
C egs_time(ounit): print a 8 char string in the form hh:mm:ss
C                  to the unit specified by ounit
C                  No end of line character is inserted
C
C Your compiler does not support the fdate or date_and_time intrinsics,
C but it has the time intrinsic. We are using it here, but the result
C may be non-standard. If you want to have the standard string as above,
C fix this subroutine.
C
C*************************************************************************

      subroutine egs_time(ounit)
      integer ounit
      character tim*20
      tim(1:len(tim)) = ' '
      call __time__(tim)
      write(ounit,'(a,$)') tim(:lnblnk1(tim))
      return
      end
