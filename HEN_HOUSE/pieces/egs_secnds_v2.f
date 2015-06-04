
C##############################################################################
C
C  EGSnrc seconds timing subroutines v2
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
      character string*24,tmp*2
      integer   h,m,s
      call __fdate__(string)
      tmp(1:2) = string(12:13)
      read(tmp,*) h
      tmp(1:2) = string(15:16)
      read(tmp,*) m
      tmp(1:2) = string(18:19)
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
C*****************************************************************************

      real function egs_tot_time(flag)
      integer flag
      character string*24,tmp*2,tmp1*4,tmp2*4
      integer vnow(8), vlast(8),i,egs_conver_month
      real t,egs_time_diff,t0
      data vlast/1970,1,1,5*0/,t0/-1/
      save vlast,t0
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
      t = egs_time_diff(vlast,vnow)
      do i=1,8
        vlast(i)=vnow(i)
      end do
      if(t0.lt.0) then
        t0 = 0
        egs_tot_time = t
      else
        t0 = t0 + t
        if(flag.eq.0) then
          egs_tot_time = t
        else
          egs_tot_time = t0
        end if
      end if
      return
      end
