
C##############################################################################
C
C  EGSnrc seconds timing subroutines stub
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
C Since your compiler does not support the date_and_time or the fdate
C intrinsics, even not the time intrinsic, this is just a stub.
C It is up to you to implement it
C using whatever functionality your compiler provides.
C
C*****************************************************************************

      real function egs_secnds(t0)
      real t0
      egs_secnds = 0
      return
      end

C*****************************************************************************
C
C real function egs_tot_time()
C
C   On first call returns seconds passed since 1/1/1970
C   On subsequent calls returns seconds since last call.
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
