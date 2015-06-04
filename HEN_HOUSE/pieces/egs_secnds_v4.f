
C##############################################################################
C
C  EGSnrc seconds timing subroutines v4
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
C real function egs_secnds(t0): returns seconds passed since midnight minus t0
C
C*****************************************************************************

      real function egs_secnds(t0)
      real t0
      character tim*8, tmp*2
      integer h,m,s
      call __time__(tim)
      tmp(1:2) = tim(1:2)
      read(tmp,*) h
      tmp(1:2) = tim(4:5)
      read(tmp,*) m
      tmp(1:2) = tim(7:8)
      read(tmp,*) s
      egs_secnds = 3600.*h + 60.*m + s - t0
      return
      end

C*****************************************************************************
C
C real function egs_tot_time()
C
C   On first call returns seconds passed since 1/1/1970
C   On subsequent calls returns
C     - seconds since last call, if flag = 0
C     - seconds since first call, else
C
C Since your compiler does not support the date_and_time or the fdate
C intrinsics, this is just a stub. It is up to you to implement it
C using whatever functionality your compiler provides.
C
C*****************************************************************************

      real function egs_tot_time(flag)
      integer flag
      egs_tot_time = 0
      return
      end
