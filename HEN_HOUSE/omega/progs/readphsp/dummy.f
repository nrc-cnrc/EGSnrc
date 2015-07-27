
C##############################################################################
C
C  EGSnrc dummy routines for readphsp utility
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
C  Author:          George Ding, 1995 (assumed)
C
C  Contributors:
C
C##############################################################################


C dummy routine for readphsp GXD
C      CALL HLIMIT(3000000)
C      STOP
C      END
C note above commented out

C  added character def'ns so no warnings on Linux


C     SUBROUTINE HLIMIT(3000000)
      SUBROUTINE HLIMIT(I)
      J=I
      WRITE(6,1000)
1000  FORMAT(/' This code was complied without paw lib availabe !!.')
      WRITE(6,1001)
1001  FORMAT(/' Sorry, Please try other opt except paw.')
      STOP
      END

C     SUBROUTINE HROPEN(IOUT,'APTUPLE',NAMEOUT,'NQ',LRECL,ISTAT)
      SUBROUTINE HROPEN(IOUT,APTUPLE,NAMEOUT,NQ,LRECL,ISTAT)
      CHARACTER*40 APTUPLE,NQ
      CHARACTER*70 NAMEOUT
      WRITE(6,1000)
1000  FORMAT(/' This code was complied without paw lib availabe !!.')
      WRITE(6,1001)
1001  FORMAT(/' Sorry, Please try other opt except paw. ')
      STOP
      END


C     SUBROUTINE HBOOKN(NTUPLE,'beam',10,'APTUPLE',NPRIME,CHTAGS)
      SUBROUTINE HBOOKN(NTUPLE,beam,I,APTUPLE,NPRIME,CHTAGS)
      CHARACTER*40 APTUPLE
      WRITE(6,1000)
1000  FORMAT(/' This code was complied without paw lib availabe !!.')
      WRITE(6,1001)
1001  FORMAT(/' Sorry, Please try other opt except paw.')
      STOP
      END

C     SUBROUTINE HROUT(0,ICYCLE,' ')
      SUBROUTINE HROUT(NO,ICYCLE,MISS)
      CHARACTER*40 MISS
      WRITE(6,1000)
1000  FORMAT(/' This code was complied without paw lib availabe !!.')
      WRITE(6,1001)
1001  FORMAT(/' Sorry, Please try other opt except paw.')
      STOP
      END

C     SUBROUTINE  HREND('APTUPLE')
      SUBROUTINE  HREND(APTUPLE)
      CHARACTER*40 APTUPLE
      WRITE(6,1000)
1000  FORMAT(/' This code was complied without paw lib availabe!!.')
      WRITE(6,1001)
1001  FORMAT(/' Sorry, Please try other opt except paw.')
      STOP
      END


      SUBROUTINE  HFN(NTUPLE,XTUPLE)
      WRITE(6,1000)
1000  FORMAT(/' This code was complied without paw lib availabe!!.')
      WRITE(6,1001)
1001  FORMAT(/' Sorry, Please try other opt except paw.')
      STOP
      END

C THE FOLLOWING SUBROUTINES ARE USED FOR CWN:                 DARYOUSH

      SUBROUTINE HBSET(BSIZE,I,IERR)
      CHARACTER*80 BSIZE
      WRITE(6,1000)
1000  FORMAT(/' This code was complied without paw lib availabe!!.')
      WRITE(6,1001)
1001  FORMAT(/' Sorry, Please try other opt except paw.')
      STOP
      END

      SUBROUTINE HBNT(NTUPLE_CWN,BEAM,MISS)
      CHARACTER*80 BEAM,MISS
      WRITE(6,1000)
1000  FORMAT(/' This code was complied without paw lib availabe!!.')
      WRITE(6,1001)
1001  FORMAT(/' Sorry, Please try other opt except paw.')
      STOP
      END

      SUBROUTINE HBNAME(NTUPLE_CWN,phasesp,e,exy)
      CHARACTER*80 phasesp,exy
      WRITE(6,1000)
1000  FORMAT(/' This code was complied without paw lib availabe!!.')
      WRITE(6,1001)
1001  FORMAT(/' Sorry, Please try other opt except paw.')
      STOP
      END

      SUBROUTINE HFNT(NTUPLE_CWN)
      WRITE(6,1000)
1000  FORMAT(/' This code was complied without paw lib availabe!!.')
      WRITE(6,1001)
1001  FORMAT(/' Sorry, Please try other opt except paw.')
      STOP
      END


