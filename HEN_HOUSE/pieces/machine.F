
C##############################################################################
C
C  EGSnrc machine dependent functions and subroutines
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
C egs_system(command)  runs a system command and returns the status
C                      command must be null-terminated
C*****************************************************************************
#include "egs_config.h"

#ifdef SYSTEM_IS_FUNCTION
      integer function egs_system(command)
      character*(*) command
      integer system, istat
      istat = system(command)
      egs_system = istat
      return
      end
#elif defined HAVE_SYSTEM_STATUS
      integer function egs_system(command)
      character*(*) command
      integer istat
      call system(command,istat)
      egs_system = istat
      return
      end
#else
C
C If your compiler does not provide the system intrinsic as a function
C or as a suboutine that returns the exit status of the command,
C we assign zero as exit status and hope for the best.
C
      integer function egs_system(command)
      character*(*) command
      call system(command)
      egs_system = 0
      return
      end
#endif
C
C*****************************************************************************
C
C  egs_isdir(file_name)  Returns .true., if the string file_name points to
C                        an existing directory.
C
C*****************************************************************************
#ifdef HAVE_LSTAT
C
C Use the lstat intrinsic and then test for bit 14 being set in the mode
C element. This approach works on all Unix systems that I have access to
C (Linux, Aix, HP-UX, OSF1, Solaris, IRIX)
C The PGI compiler uses more than 13 elements for the array passed to lstat
C and the directory element is 5 instead of 3.
C The Lahey compiler appears to be OK with 13 elements but uses the first
C element to return the file mode
C
#ifdef PGI
#define LSTAT_ARRAY_SIZE 25
#define LSTAT_DIR_ELEMENT 5
#elif defined LAHEY
#define LSTAT_ARRAY_SIZE 13
#define LSTAT_DIR_ELEMENT 1
#else
#define LSTAT_ARRAY_SIZE 13
#define LSTAT_DIR_ELEMENT 3
#endif
      logical function egs_isdir(file_name)
      implicit none
      character*(*) file_name
      integer*4 egs_lnblnk, res, array(LSTAT_ARRAY_SIZE), l, lstat
      logical btest
      egs_isdir = .false.
      l = egs_lnblnk(file_name)
      if( l.lt.len(file_name) ) file_name(l+1:l+1) = char(0)
         ! On some systems lstat only works if the string is 0-terminated
      res = lstat(file_name,array)
      if( l.lt.len(file_name) ) file_name(l+1:l+1) = ' '
      if( res.eq.0 ) then
          if( btest(array(LSTAT_DIR_ELEMENT),14) ) egs_isdir = .true.
      end if
      return
      end
#elif defined INQUIRE_WORKS_FOR_DIRS
C
C This version uses the inquire intrinsic, which only tells us whether
C file_name points to an existing object (file, directory, link, etc.),
C not specifically an existing directory. But that's all we need.
C The reason for having different versions of this function is that on some
C systems (e.g. OSF1) the inquire intrinsic only returns .true. for existing
C files, not for existing directories.
C
      logical function egs_isdir(file_name)
      implicit none
      character*(*) file_name
      logical ex
      inquire(file=file_name,exist=ex)
      egs_isdir = ex
      return
      end
#else
C
C This is a dummy function only used if the other two methods to see
C whether a directory exists fail.
C
      logical function egs_isdir(file_name)
      implicit none
      character*(*) file_name
      egs_isdir = .true.
      return
      end
#endif

C
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
C****************************************************************************
C
      subroutine egs_fdate(ounit)
      integer ounit
      character*24 string
      call egs_get_fdate(string)
      write(ounit,'(a,$)') string
      return
      end
C
C***************************************************************************
C
C   egs_get_fdate(string) assignes a 24 char date and time string to string.
C                         string must be at least 24 chars long, otherwise
C                         this subroutine has no effect.
C
C***************************************************************************
C
#ifdef HAVE_FDATE
C
C Use the fdate fortran intrinsic.
C On some systems (e.g. AIX) one needs fdate_ instead of fdate and
C so, FDATE_FUNCTION is a macro that is set to fdate or fdate_.
C
      subroutine egs_get_fdate(string)
      character*(*) string
      if( len(string).ge.24 ) call FDATE_FUNCTION(string)
      return
      end

#elif defined HAVE_DATE_AND_TIME
C
C Use the date_and_time intrinsic.
C date_and_time is standard only in Fortran 90,95 but many f77 compilers
C have it anyway.
C
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
#else
C
C This is a stub in case neither fdate not date_and_time is available.
C
      subroutine egs_get_fdate(string)
      character*(*) string
      if( len(string).lt.24 ) return
      string = 'egs_get_fdate(): fix me!'
      return
      end

#endif
C
C*****************************************************************************
C
C egs_date_and_time(vnow)  Fill the 8-dimensional array vnow with the
C                          content of the F90 intrinsic date_and_time
C
C******************************************************************************
C
#ifdef HAVE_DATE_AND_TIME
C
C Simply use date_and_time
C
      subroutine egs_date_and_time(vnow)
      integer vnow(8)
      character dat*8,tim*10,zon*5
      call date_and_time(dat,tim,zon,vnow)
      return
      end

#elif defined HAVE_FDATE
C
C Use fdate and then convert to the date_and_time format for vnow.
C
      subroutine egs_date_and_time(vnow)
      integer vnow(8)
      character string*24,tmp*2,tmp1*4,tmp2*3
      integer   egs_conver_month
      call FDATE_FUNCTION(string)
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
      vnow(4) = 0 ! don't know the time zone in this case.
      return
      end

#else
C
C A stub in case neither date_and_time nor fdate is present.
C
      subroutine egs_date_and_time(vnow)
      integer vnow(8),i
      do i=1,8
        vnow(i)=0
      end do
      return
      end

#endif

C
C*************************************************************************
C
C egs_date(ounit): print a 11 char string in the form
C                     '18-Mar-2003'
C                  to the unit specified by ounit
C                  No end of line character is inserted
C
C*************************************************************************
C
#ifdef HAVE_FDATE
C
C Use fdate
C
      subroutine egs_date(ounit)
      integer ounit
      character string*24, dat*11
      call FDATE_FUNCTION(string)
      dat(1:2) = string(9:10)
      dat(3:3) = '-'
      dat(4:6) = string(5:7)
      dat(7:7) = '-'
      dat(8:11) = string(21:24)
      write(ounit,'(a,$)') dat
      return
      end

#elif defined HAVE_DATE_AND_TIME
C
C Use date_and_time
C
      subroutine egs_date(ounit)
      integer ounit
      character dat*8,tim*10,zon*5,tmp*3,daten*11
      integer values(8)
      call date_and_time(dat,tim,zon,values)
      daten(1:2)=dat(7:8)
      daten(3:3) = '-';
      call egs_month(values(2),tmp)
      daten(4:6) = tmp(1:3)
      daten(7:7) = '-'
      daten(8:11) = dat(1:4)
      write(onit,'(a,$)') daten
      return
      end

#elif defined HAVE_DATE
C
C Use date because neither of fdate or date_and_time is available.
C Note that the resulting string is different on different machines
C and that date is not Y2K complient.
C
      subroutine egs_date(ounit)
      integer ounit,egs_lnblnk
      character dat*20
      dat(:len(dat)) = ' '
      call DATE_FUNCTION(dat)
      write(ounit,'(a,$)') dat(:egs_lnblnk(dat))
      return
      end

#else
C
C If your compiler does not support the fdate or date_and_time intrinsics,
C even not the date intrinsic, the foillowing stub will get used.
C
      subroutine egs_date(ounit)
      integer ounit
      write(ounit,'(a,$)') 'egs_date: fixme'
      return
      end

#endif

C
C*************************************************************************
C
C egs_time(ounit): print a 8 char string in the form hh:mm:ss
C                  to the unit specified by ounit
C                  No end of line character is inserted
C
C*************************************************************************
C
#ifdef HAVE_FDATE
C
C Use fdate
C
      subroutine egs_time(ounit)
      integer ounit
      character string*24
      call FDATE_FUNCTION(string)
      write(ounit,'(a,$)') string(12:19)
      return
      end

#elif defined HAVE_DATE_AND_TIME
C
C Use date_and_time
C
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
      write(onit,'(a,$)') timen
      return
      end

#elif defined HAVE_TIME
C
C Use the time intrinsic.
C
      subroutine egs_time(ounit)
      integer ounit,egs_lnblnk
      character tim*20
      tim(1:len(tim)) = ' '
      call TIME_FUNCTION(tim)
      write(ounit,'(a,$)') tim(:egs_lnblnk(tim))
      return
      end

#else
C
C If your compiler does not support the fdate or date_and_time intrinsics,
C even not the time intrinsic, the following stub gets used.
C Fix it if you want to have the date printed to your output files.
C
      subroutine egs_time(ounit)
      integer ounit
      write(ounit,'(a,$)') 'egs_time: fixme'
      return
      end

#endif

C
C*****************************************************************************
C
C real function egs_secnds(t0): returns seconds passed since midnight minus t0
C
C real function egs_tot_time(flag)
C
C   On first call returns seconds passed since 1/1/1970
C   On subsequent calls returns
C     - seconds since last call, if flag = 0
C     - seconds since first call, else
C
C*****************************************************************************
#ifdef HAVE_DATE_AND_TIME
C
C Use date_and_time.
C On some systems this provides sub-second resolution.
C
      real function egs_secnds(t0)
      real t0,t1
      character dat*8,tim*10,zon*5
      integer values(8)
      call date_and_time(dat,tim,zon,values)
      t1 = 3600.*values(5) + 60.*values(6) + values(7) + 0.001*values(8)
      egs_secnds = t1 - t0
      return
      end

      real function egs_tot_time(flag)
      integer flag
      character dat*8,tim*10,zon*5
      integer vnow(8), vlast(8),i
      real t,egs_time_diff,t0
      data vlast/1970,1,1,5*0/,t0/-1/
      save vlast,t0
      call date_and_time(dat,tim,zon,vnow)
      t = egs_time_diff(vlast,vnow)
      do i=1,8
        vlast(i)=vnow(i)
      end do
      if( t0.lt.0 ) then
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

#elif defined HAVE_FDATE
      real function egs_secnds(t0)
      real t0
      character string*24,tmp*2
      integer   h,m,s
      call FDATE_FUNCTION(string)
      tmp(1:2) = string(12:13)
      read(tmp,*) h
      tmp(1:2) = string(15:16)
      read(tmp,*) m
      tmp(1:2) = string(18:19)
      read(tmp,*) s
      egs_secnds = 3600.*h + 60.*m + s - t0
      return
      end

      real function egs_tot_time(flag)
      integer flag
      character string*24,tmp*2,tmp1*4,tmp2*4
      integer vnow(8), vlast(8),i,egs_conver_month
      real t,egs_time_diff,t0
      data vlast/1970,1,1,5*0/,t0/-1/
      save vlast,t0
      call FDATE_FUNCTION(string)
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

#elif defined HAVE_SECNDS

      real function egs_secnds(t0)
      real t0
      egs_secnds = SECONDS_FUNCTION(t0)
      return
      end

      real function egs_tot_time(flag)
      integer flag
      egs_tot_time = 0
      return
      end

#elif defined HAVE_TIME

      real function egs_secnds(t0)
      real t0
      character tim*8, tmp*2
      integer h,m,s
      call TIME_FUNCTION(tim)
      tmp(1:2) = tim(1:2)
      read(tmp,*) h
      tmp(1:2) = tim(4:5)
      read(tmp,*) m
      tmp(1:2) = tim(7:8)
      read(tmp,*) s
      egs_secnds = 3600.*h + 60.*m + s - t0
      return
      end

      real function egs_tot_time(flag)
      integer flag
      egs_tot_time = 0
      return
      end

#else

      real function egs_secnds(t0)
      real t0
      egs_secnds = 0
      return
      end

      real function egs_tot_time(flag)
      integer flag
      egs_tot_time = 0
      return
      end

#endif

C****************************************************************************
C
C Returns the time difference between vstart and vend
C vstart and vend are integer arrays of dimension 8 with elements
C corresponding to the specification of the data_and_time routine, i.e.
C   array(1) = year
C   array(2) = month of the year   (1...12)
C   array(3) = day of the month    (1...31)
C   array(4) = difference in minutes from UTC
C   array(5) = hour of the day     (1...23)
C   array(6) = minute of the hour  (1...59)
C   array(7) = seconds of the minute (1...59)
C   array(8) = miliseconds of the second (1...999)
C
C Note: this implementation ignores the time difference from UTC field
C
C*****************************************************************************
      real function egs_time_diff(vstart,vend)
      integer    vstart(8),vend(8)
      real       egs_time_diff_o
      if( vend(1).lt.vstart(1).or.
     &  (vend(1).eq.vstart(1).and.vend(2).lt.vstart(2)) ) then
        egs_time_diff = -egs_time_diff_o(vend,vstart)
      else
        egs_time_diff = egs_time_diff_o(vstart,vend)
      end if
      return
      end

C******************************************************************************
C
C day difference between the dates specified by the integer arrays vstart and
C vend. The arrays are v(1)=year, v(2)=month, v(3)=day
C
C******************************************************************************
      integer function egs_day_diff(vstart,vend)
      integer vstart(3),vend(3),egs_day_diff_o
      if( vend(1).lt.vstart(1).or.
     &  (vend(1).eq.vstart(1).and.vend(2).lt.vstart(2)) ) then
        egs_day_diff = -egs_day_diff_o(vend,vstart)
      else
        egs_day_diff = egs_day_diff_o(vstart,vend)
      end if
      return
      end

C******************************************************************************
C Returns a 3-letter abreviation of the day of the week in the string day,
C given a day specified by the integer array values
C   values(1)=year, values(2)=month, values(3)=day
C******************************************************************************
      subroutine egs_weekday(values,day)
      character*(*) day
      integer       values(3)
      integer       days,vtmp(3),egs_day_diff,aux
      character*3   wdays(7)
      data wdays/'Mon','Tue','Wed','Thu','Fri','Sat','Sun'/
      vtmp(1) = 1970
      vtmp(2) = 1
      vtmp(3) = 1
      days = egs_day_diff(vtmp,values)
      aux = mod(days,7)
      days = 4 + aux
      if( days.gt.7 ) days = days - 7
      day(:len(day)) = ' '
      aux = min(len(day),3)
      day(:aux) = wdays(days)(:aux)
      return
      end

C*****************************************************************************
C Same as egs_day_diff above, but assumes that vend specifies a later date
C than vstart.
C*****************************************************************************
      integer function egs_day_diff_o(vstart,vend)
      integer vstart(3),vend(3)
      integer    days
      logical    next_month
      integer    tm,m,ty,y
      integer    mdays(12)
      data       mdays/31,28,31,30,31,30,31,31,30,31,30,31/
      days = 0
      ty = vstart(1)
      y  = vend(1)
      tm = vstart(2)
      m  = vend(2)
      next_month = .true.
      do while(next_month)
        if( tm.eq.m.and.ty.eq.y ) then
          next_month = .false.
        else
          days = days + mdays(tm)
          if( tm.eq.2.and.mod(ty,4).eq.0 ) days = days + 1
          tm = tm + 1
          if( tm.gt.12 ) then
            ty = ty + 1
            tm = 1
          end if
        end if
      end do
      days = days + vend(3) - vstart(3)
      egs_day_diff_o = days
      return
      end

C******************************************************************************
C Same as egs_time_diff above, but assumes that vend specifies a later date
C than vstart.
C******************************************************************************
      real function egs_time_diff_o(vstart,vend)
      integer    vstart(8),vend(8)
      integer    days,hours,minutes,secs,msecs
      integer    egs_day_diff_o
      days = egs_day_diff_o(vstart,vend)
      hours = vend(5) - vstart(5)
      minutes = vend(6) - vstart(6)
      secs = vend(7) - vstart(7)
      msecs = vend(8) - vstart(8)
      egs_time_diff_o = 3600.*(24.*days+hours)+60.*minutes+secs+
     &                  0.001*msecs
      return
      end

C******************************************************************************
C Returns in month a 3-letter abreviation of the month specified by mo, if
C mo is between 1 and 12, or an empty string otherwise.
C******************************************************************************
      subroutine egs_month(mo,month)
      integer mo
      character*(*) month
      integer iaux
      character*3   months(12)
      data months/'Jan','Feb','Mar','Apr','May','Jun', 'Jul','Aug','Sep'
     *,'Oct','Nov','Dec'/
      iaux = min(len(month),3)
      month(:len(month)) = ' '
      if( mo.ge.1.and.mo.le.12 ) month(:iaux) = months(mo)(:iaux)
      return
      end

C******************************************************************************
C Converts a 3-letter abreviation of a month to its corresponding integer
C value, if the string month is a valid month, or -1 otherwise.
C******************************************************************************
      integer function egs_conver_month(month)
      character*3 month
      character*3 months(12)
      integer i
      data months/'Jan','Feb','Mar','Apr','May','Jun', 'Jul','Aug','Sep'
     *,'Oct','Nov','Dec'/
      do i=1,12
        if( month.eq.months(i) ) then
          egs_conver_month = i
          return
        end if
      end do
      egs_conver_month = -1
      return
      end

C
C*****************************************************************************
C
C real function egs_etime(): returns CPU time consumed since the start of
C                            the program
C
C*****************************************************************************
C
#ifdef HAVE_ETIME
      real function egs_etime()
      real tarray(2),ETIME_FUNCTION
      egs_etime = ETIME_FUNCTION(tarray)
      return
      end
#else
      real function egs_etime()
      egs_etime = 0
      return
      end
#endif


C
C******************************************************************************
C
C Print the canonical system name as determined by the config.guess script
C or the Windows installation program to the unit specified by ounit.
C
C*****************************************************************************
C
      subroutine egs_print_canonical_system(ounit)
      integer ounit
      write(6,'(a,$)') CANONICAL_SYSTEM
      return
      end

C******************************************************************************
C
C Assign the canonical system name as determined by the config.guess script
C or the Windows installation program to the string pointed to by res
C
C******************************************************************************

      subroutine egs_get_canonical_system(res)
      character*(*) res
      integer l1,l2,egs_lnblnk
      l1 = egs_lnblnk(CANONICAL_SYSTEM)
      l2 = len(res)
      res(:l2) = ' '
      if( l2.ge.l1 ) then
        res(:l1) = CANONICAL_SYSTEM
      else
        res(:l2) = CANONICAL_SYSTEM
      end if
      return
      end

C
C******************************************************************************
C
C Print the configuration name as specified during the configuration
C process to the unit specified by ounit.
C
C*****************************************************************************

      subroutine egs_print_configuration_name(ounit)
      integer ounit
      write(6,'(a,$)') CONFIGURATION_NAME
      return
      end

C******************************************************************************
C
C Assign the configuration name as specified suring the configuration
C process to the string pointed to by res
C
C******************************************************************************

      subroutine egs_get_configuration_name(res)
      character*(*) res
      integer l1,l2,egs_lnblnk
      l1 = egs_lnblnk(CONFIGURATION_NAME)
      l2 = len(res)
      res(:l2) = ' '
      if( l2.ge.l1 ) then
        res(:l1) = CONFIGURATION_NAME
      else
        res(:l2) = CONFIGURATION_NAME
      end if
      return
      end

C
C*****************************************************************************
C
C Print the host name to the unit specified by ounit without inserting
C a new line character.
C
C*****************************************************************************

      subroutine egs_print_hostnm(ounit)
      integer ounit,egs_lnblnk
      character*256 string
      call egs_get_hostnm(string)
      write(ounit,'(a,$)') string(:egs_lnblnk(string))
      return
      end
C
C*****************************************************************************
C
C Assign the host name to the string pointed to be hname.
C
C*****************************************************************************
C
#ifdef HAVE_HOSTNM
      subroutine egs_get_hostnm(hname)
      character*(*) hname
      character*256 string
      integer res,HOSTNM_FUNCTION,egs_lnblnk,l1,l2,l
      res = HOSTNM_FUNCTION(string)
      if( res.ne.0 ) then
        write(6,'(a,a)') 'hostnm() returned with a non-zero status '
        stop
      end if
      l1 = egs_lnblnk(string)
      l2 = len(hname)
      hname(:l2) = ' '
      l = min(l1,l2)
      hname(:l) = string(:l)
      return
      end
#else
C
C If your compiler does not have a working version of the hostnm
C intrinsic, we attempt to obtain the hostname from various environment
C variables. This is NOT guaranteed to work. If it does not, you have
C to provide your own implementation if you wish to have the name of the
C host your program is running on to the output files.
C
      subroutine egs_get_hostnm(hname)
      character*(*) hname
      character*256 string
      integer egs_lnblnk
      string(:len(string)) = ' '
      call getenv('COMPUTERNAME',string)
      if( egs_lnblnk(string).eq.0 ) then
        call getenv('HOSTNAME',string)
        if( egs_lnblnk(string).eq.0 ) then
          call getenv('HOST',string)
          if( egs_lnblnk(string).eq.0 ) then
            string(:egs_lnblnk('egs_get_hostnm: fixme'))=
     &          'egs_get_hostnm: fixme'
            return
          end if
        end if
      end if
      l1 = egs_lnblnk(string)
      l2 = len(hname)
      hname(:l2) = ' '
      l = min(l1,l2)
      hname(:l) = string(:l)
      return
      end
#endif

#ifndef HAVE_LNBLNK
*****************************************************************************
C
C Some people love using lnblnk instead of the function
C egs_lnblnk provided by EGSnrc. Unfortunately lnblnk is not provided on
C some compilers and so, here is a replacement.
C
C*****************************************************************************
      integer function lnblnk(string)
      integer egs_lnblnk
      lnblnk = egs_lnblnk(string)
      return
      end
#endif

C*****************************************************************************
C
C egs_lnblnk(string) Returns the index of the last non-blank character in string
C
C*****************************************************************************
      integer function egs_lnblnk(string)
      character*(*) string
      integer i,j
      do i=len(string),1,-1
        j = ichar(string(i:i))
        if( j.ne.9.and.j.ne.10.and.j.ne.11.and.j.ne.12.and.j.ne.13.
     &      and.j.ne.32 ) then
          egs_lnblnk = i
          return
        end if
      end do
      egs_lnblnk = 0
      return
      end

