
!##############################################################################
!
!  Mortran3 source code
!  Copyright (C) 1993 Stanford Linear Accelerator Center
!  Copyright (C) 2015 National Research Council Canada
!
!  This file is part of EGSnrc.
!
!  EGSnrc is free software: you can redistribute it and/or modify it under
!  the terms of the GNU Affero General Public License as published by the
!  Free Software Foundation, either version 3 of the License, or (at your
!  option) any later version.
!
!  EGSnrc is distributed in the hope that it will be useful, but WITHOUT ANY
!  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
!  FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for
!  more details.
!
!  You should have received a copy of the GNU Affero General Public License
!  along with EGSnrc. If not, see <http://www.gnu.org/licenses/>.
!
!##############################################################################
!
!  Author:          Stanford Linear Accelerator Center
!
!  Contributors:    Ray Cowan
!                   Dave Rogers
!                   Alex Bielajew
!                   Iwan Kawrakow
!
!##############################################################################
!
!  Mortran3 is originally copyright by SLAC and is distributed by NRC under
!  the terms of the AGPL 3.0 licence, in agreement with SLAC. Mortran has a
!  long history  dating back to the 1970s, and certainly involving many
!  contributors. The contributors named above are only those who could be
!  identified from this file's revision history.
!
!
!  Iwan Kawrakow, 2003:
!
!  Changes for EGSnrc Version 4. Make use of command line arguments to open
!  input/output files. After these changes Mortran 3 can be run without first
!  making symbolik links to the necessary files, e.g., mortran3.exe -d
!
!
!  Dave Rogers, 1996:
!
!  Added a few explicit file=fort.n statements to make HP port
!
!  On Sept 21, 1996 the 128K buffer version of mortran3.f (SID 1.8) was
!  replaced with the 300K buffer version which had previously been maintained
!  as big_mortran3.f. Given the fact that memories are so much bigger now, we
!  are just going to maintain the bigger version, now called mortran3.f
!
!  At NRC, to recover the old 128K version: sccs get -r1.8 mortran3.f to see
!  the minor changes to big_mortran3.f made prior to renaming it as mortran3.f,
!  see the residual big_mortran3.f file.
!
!  Note that this version uses the DNTIME routine consistently since this is
!  what the compilers either handle, or the extra routines for the RS6000 and
!  Linux system work with.
!
!
!  Ray F. Cowan, 1993:
!
!  Development version of mortran3.f, the "frozen" egs Fortran implementation
!  of mortran3 for EGS users.
!
!
!  Rev. 2.  Attempt to increase buffer spaces so that mortran3 does not run out
!  -------  of "Rule space".  Mods are to globally change all occurrences of
!           the following constants:
!
!  Name    Old value   New value  Comment
!  ----    ---------   ---------  -------
!  MXU     300000      600000     Max size of buffer O.
!  MNXBF   225739      450741     Minimum (low end) of X-buffer.
!  ?????   225738      450740     Apparently MNXBF-1, found in code.
!  HOME    112369      525371     Location halfway between X-buffer start
!                                 (MNXBF) and top end of O (MXU).
!
!  This fix provided was provided by Thies Schoenborn
!
!  The previous revision increased buffer size to 300k from 128k
!
!  Note that:  MNXBF = MXMBF + 1 + 76, where MXMBF is top end of MBF, the
!  "main" buffer, which starts at MNMBF=2655.  See constants at beginning of
!  code below.  MXMBF = MNMBF + ((MXU-MXPJT)*3/4) - 1, where MXPJT is top end
!  of the PJT buffer, which precedes MBF in O, so MXPJT=2654.  HOME is given
!  by HOME = MNXBF + (MXU - MNXBF)/2.  Apparently MBF is assigned 3/4 of the
!  space remaining in O at the time of its definition, and XBF is assigned the
!  remaining 1/4 of O, and HOME is a pointer halfway through XBF.  In general,
!  buffers have names like PBF, MBF, XBF, etc., and the low (min) and high
!  (max) ends of each buffer are denoted by names like MNMBF and MXMBF.
!
!  WARNING:  none of this is absolutely certain at this time.
!
!
! -----------------------------------------------------------------------------
! MORSOR77 BITNET FRON SLAC APRIL 3 1985  D.W.O.R.
! Copyright Stanford University Mortran 3.1 (FORTRAN 77)
! UNIT(9)_ANO(10)_LVL(11)_CRS(12)_MDEF(13)_CSET(14)_FRT(15)
! LABL(16)_HOLR(17)_INDF(18)_SEQN(19)_NFLG(20)_LIST(21)_MCOM(22)
! NEST(23)_NSCN(24)_PCOM(25)_LAZY(26)_MRKL(27)_STNG(28)_MTRC(29)
! SEQF(30)_DFLT(31)_BASE(32)_BLGR(33)_FREE(34)_MRKF(35)_TMX(36)
! NMX(37)_RMX(38)_INDC(39)_MXX(40)_SIGN(41)_SIGR(42)_SIGT(43)
! ISK(44)_JSK(45)_KSK(46)_LSK(47)_MSK(48)_NSK(49)_USK(50)
! YSK(51)_EOF(52)_INDM(53)_TYP(54)_QFL(55)_MLST(56)_FLST(57)
! IFL(58)_MODE(59)_LOCS(60)_LEFT(61)_MCNT(62)_MERR(63)_KFE(64)
! KFB(65)_KTE(66)_KTB(67)_NGN(68)_SQ(69)_EOL(70)_GEO(71)
! XTR(72)_XTR(73)_XTR(74)_ICUR(75)_BLA(76)_BAT(77)_BLB(78)
! TRM(79)_LPR(80)_RPR(81)_LSB(82)_RSB(83)_LCB(84)_RCB(85)
! BQ(86)_DQ(87)_RTT(88)_LTT(89)_DED(90)_NIL(91)_LNS(92)
! RNS(93)_RIF(94)_LIF(95)_RDQ(96)_LDQ(97)_RMB(98)_LMB(99)
! ZRO(100)_ALT(101)_RNG(102)_LIP(103)_RIP(104)_LQ(105)_RQ(106)
! GA(107)_GP(108)_LGB(109)_RGB(110)_NAT(111)_NLB(112)_XTR(113)
! MF1(114)_MF2(115)_MF3(116)_MF4(117)_MF5(118)_MF6(119)_MF7(120)
! _MF8(121)_MF9(122)_MFA(123)_MFB(124)_MFC(125)_MFD(126)
! NODE4_NSM100_SLOP20_NSX120_BFF132_MXU600000_MNXBF450741
! HOME525371_EVLIM1000_MXERR100_G1157_G0156_G2158_G11167
! G13169_G14170_R1325_R0324_R80404_T1457_T0456
! _T2458_T72528_W1175_W0174_W7181_W72246_W73247_W80254
! SREG1_UREG121_GBF157_WBF175_RBF325_TBF457_ISET589
! OSET689_MORF789_TRT889_0CT1145_1CT1265_2CT1385_3CT1505
! 4CT1625_5CT1745_6CT1865_7CT1985_ISK2105_JSK2155_KSK2205
! _LSK2255_MSK2305_NSK2355_USK2455_YSK2505_PJT2555_MBF2655
! -----------------------------------------------------------------------------
!
!
!##############################################################################


      implicit none
      DIMENSION O(600000)
      COMMON /IBM/O
      integer O
      integer O_B,O_E,O_R,O_S,O_V,O_U,O_W,O_X
      equivalence (O(1),O_B),(O(2),O_E),(O(3),O_R),(O(4),O_S), &
                  (O(5),O_V),(O(6),O_U),(O(7),O_W),(O(8),O_X)
      integer O_C,O_MXC,O_M
      integer iw,a,d,mxd,kd,mq,f,t,ip,iq,n,n1,n2,n3,kc,kb,kp,p,q,ia, &
              ire,nb,kx,y,nerd
      integer ispec,linf
      EQUIVALENCE(O(12),O_C),(O(43),O_MXC),(O(22),O_M)
!
      common/nrcc/ oflag
      integer      oflag

      save    iw,a,d,mxd,kd,mq,f,t,ip,iq,n,n1,n2,n3,kc,kb,kp,p,q,ia, &
              ire,nb,kx,y,nerd

      call open_units

      write(O_E,'(a,T55,$)') 'Mortran 3.2',' '
      call egs_fdate(O_E)
      write(O_E,*)

      O_V=100
      O_U=1000
      CALL INITAL(1)
      IW=450741
      O_X=O_W
      GO TO 10
 30 IF (O_S>O_U) THEN
         IF (O(23)>0) THEN
            CALL EXPAND(-1)
            GO TO 30
         END IF
         O_S=525371
         O_U=O_S-1
         GO TO 10
      END IF
      O_M=O(32)+O(O_S)
      MACRO_SCAN: DO
      IF(O(O_M)==0)EXIT
      O_M=O(O_M)
      D=O_M+4
      MXD=O(O_M+3)
      KD=0
      O(69)=0
      MQ=0
      F=0
      A=0
      T=O_S
      O_V=O_S
      IF(O(29)>=4) CALL MACTRC(2,D,MXD)
      IF(O(29)>=4) CALL MACTRC(5,O_V,O_U+1)
      TOKEN_SCAN: DO
         IF(O_V>O_U)EXIT TOKEN_SCAN
         IF (O(O_V)==O(97)) THEN
            O_V=O_V+O(O_V+1)+3
            CYCLE TOKEN_SCAN
         END IF
         IF (MQ/= 0) GO TO 140
            IF (O(D)==O(84)) THEN
               IF (O(D+1)<=100.AND.O(32)>1505) THEN
                  O(O_S-2*(A+1)-1)=O_V
                  O(O_S-2*(A+1)-2)=O_V
                  D=D+1
                  MQ=1
                  IF(O(D-3)/= O(107))F=0
                  IP=D
                  IQ=D
                  DO
                     IF(O(IQ)==O(85))EXIT
                     IQ=IQ+1
                     IF(IQ<=MXD)CYCLE
                     CALL MESAGE(14, 20, 0,0)
                     CYCLE MACRO_SCAN
                  END DO
                  CYCLE TOKEN_SCAN
               END IF
            END IF
            IF (O(D)/= O(O_V).OR.O(O_V)>=O(107)) GO TO 230
               IF( O(105) ==O(D))O(69)=O(69)+1
               IF( O(106) ==O(D))O(69)=O(69)-1
               IF(O(D)==O(99))KD=1
               IF(O(D)==O(98))KD=0
               O_V=O_V+1
               T=O_V
 240          D=D+1
               IF(D>=MXD) THEN
                  CALL COUT()
                  CALL EXPAND(A)
                  GO TO 30
               END IF
               CYCLE TOKEN_SCAN
 230       CONTINUE
            IF (O(O_V)==O(76).AND.O(69)+KD==0) THEN
                  O_V=O_V+1
                  CYCLE TOKEN_SCAN
            END IF
            IF (O(D)==O(107)) THEN
               D=D+1
               N=O(D)
               N1=N/4
               N2=(N-4*N1)/2
               N3=N-4*N1-2*N2
            ELSE IF (O(D)==O(108)) THEN
               N1=1
               N2=1
               N3=O(59)
            ELSE
               GO TO 130
            END IF
            T=O_V
            O(O_S-2*(A+1)-1)=O_V
            O(O_S-2*(A+1)-2)=O_V
            A=A+1
            D=D+1
            F=D
            KC=0
            KB=0
            KP=0
            CYCLE TOKEN_SCAN
 140       P=IP
            Q=IQ
            IA=O_V
            IF(O(69)/= 0)IA=0
            IRE=ISPEC(P,Q,O_V,O_U,IA)
            IF(IRE==1)EXIT TOKEN_SCAN
            IF (IRE==3) THEN
               A=A+1
               IF(F/= 0)O(O_S-2*(A-1)-2)=T
               IF(IA/= 0)O(O_S-2*(A)-1)=IA
               O(O_S-2*(A)-2)=O_V
               D=Q
               MQ=0
               GO TO 240
            END IF
 130    CONTINUE
         IF (O(O_V)==O(76).AND.MQ==1) THEN
            O_V=O_V+1
            CYCLE TOKEN_SCAN
         END IF
                  IF(F==0)CYCLE MACRO_SCAN
         DO
                        IF(O(T)==O(106))CYCLE MACRO_SCAN
            IF (KD>0) THEN
            DO
               IF( O(99) ==O(T))KD=KD+1
               IF( O(98) ==O(T))KD=KD-1
               IF(KD==0)EXIT
               T=T+1
               IF(T>O_U)CYCLE MACRO_SCAN
            END DO
            EXIT
            END IF
            IF (O(T)==O(105)) THEN
               NB=1
               DO
                  IF(NB==0)EXIT
                  T=T+1
                  IF(T>O_U)CYCLE MACRO_SCAN
                  IF( O(105) ==O(T))NB=NB+1
                  IF( O(106) ==O(T))NB=NB-1
              END DO
            END IF
            IF (O(69)==0) THEN
               IF( O(84) ==O(T))KC=KC+1
               IF( O(85) ==O(T))KC=KC-1
               IF(KC<0)CYCLE MACRO_SCAN
               IF (N1==1) THEN
                  IF(38==O(T))KP=KP+1
                  IF(46==O(T))KP=KP-1
                  IF(KP<0)CYCLE MACRO_SCAN
               END IF
               IF (N2==1) THEN
                  IF( O(82) ==O(T))KB=KB+1
                  IF( O(83) ==O(T))KB=KB-1
                  IF(KB<0)CYCLE MACRO_SCAN
               END IF
               IF (O(T)==O(79).AND.N3>0)CYCLE MACRO_SCAN
            END IF
            T=T+1
            IF (T>O_U) THEN
               IF(O(15)==1)CYCLE MACRO_SCAN
               GO TO 10
            END IF
            IF (KC+KP+KB/= 0) CYCLE
            IF(MQ==0)D=F
            EXIT
         END DO
         O_V=T
         O(O_S-2*(A)-2)=O_V
      END DO TOKEN_SCAN
      IF(O(15)==1)CYCLE MACRO_SCAN
      IF (O_S==O_U) THEN
         O_S=525371
         O(O_S)=O(O_U)
         O_U=O_S
      END IF
 10 O(55)=O(76)
      KX=0
      DO
         O_U=O_U+1
         IF(O_U>600000)CALL MESAGE(15, 1, 0,0)
         O(O_U)=O(O_C)
         O_C=O_C+1
         IF(O_C>O(43))CALL NXTCRD()
         IF (O(O_U)==O(84)) THEN
            O(55)=O(O_U)
            KX=1
            DO
               IF(KX==0)EXIT
               O_U=O_U+1
               IF(O_U>600000)CALL MESAGE(15, 1, 0,0)
               O(O_U)=O(O_C)
               O_C=O_C+1
               IF(O_C>O(43))CALL NXTCRD()
               IF( O(84) ==O(O_U))KX=KX+1
               IF( O(85) ==O(O_U))KX=KX-1
            END DO
            O(55)=O(76)
         END IF
         IF (O(O_U)==O(87)) THEN
         DO
            IF(O(26)/= 1)O(55)=O(87)
            O(O_U)=O(97)
            O_U=O_U+1
            Y=O_U
            DO
               IF(O(O_C)==O(87).OR.O_C>O(43))EXIT
               O_U=O_U+1
               IF(O_U>600000)CALL MESAGE(15, 1, 0,0)
               O(O_U)=O(O_C)
               O_C=O_C+1
            END DO
            O_U=O_U+1
            O(O_U)=O(96)
            O(Y)=O_U-Y-1
            IF (O(Y)==0) THEN
               O_U=O_U-2
               O(O_U)=O(76)
            END IF
            IF (O_C<=O_MXC) THEN
               O(55)=O(76)
               O_C=O_C+1
               IF(O_C>O(43))CALL NXTCRD()
               GO TO 10
            ELSE IF (O(26)==1) THEN
               O(55)=O(76)
               CALL NXTCRD()
               GO TO 10
            ELSE
               CALL NXTCRD()
               O_U=O_U+1
               CYCLE
            END IF
         END DO
         END IF
         IF (O(O_U)==O(86).AND.O(28)>0) THEN
            O(55)=O(O_U)
            O(O_U)=O(105)
            DO
               O_U=O_U+1
               IF(O_U>600000)CALL MESAGE(15, 1, 0,0)
               O(O_U)=O(O_C)
               O_C=O_C+1
               IF(O_C>O(43))CALL NXTCRD()
               IF (O(O_U)/= O(86))CYCLE
               IF (O(O_C)==O(86)) THEN
                  O_C=O_C+1
                  IF(O_C>O(43))CALL NXTCRD()
                  CYCLE
               END IF
               O(O_U)=O(106)
               O(55)=O(76)
               EXIT
            END DO
         END IF
         IF (O(O_U)==O(76)) THEN
            IF (O_S/=O_U) THEN
               IF(O(O_U-1)==O(76))O_U=O_U-1
            END IF
            DO
               IF(O_C==O_MXC .OR.O(O_C)/= O(76))EXIT
               O_C=O_C+1
            END DO
         END IF
         IF (KX==0) THEN
            IF(O(O_U)==O(79))GO TO 30
            IF( O(82) ==O(O_U))O(11)=O(11)+1
            IF( O(83) ==O(O_U))O(11)=O(11)-1
         END IF
      END DO
      END DO MACRO_SCAN
      IF (O(23)>0) THEN
         O_X=O_X+1
         IF(O_X>O(40))CALL MESAGE(15, 3, 0,0)
         O(O_X)=O(O_S)
         O_S=O_S+1
         IF (O(O_X)==O(92).AND.O(32)/=1265) THEN
            DO
               O_X=O_X+1
               IF(O_X>O(40))CALL MESAGE(15, 3, 0,0)
               O(O_X)=O(O_S)
               O_S=O_S+1
               IF(O(O_X)==O(93))EXIT
            END DO
         END IF
         GO TO 30
      END IF
      DO
         IF (O(O_S)==O(79)) THEN
            IF (O_W>=IW) O_W=LINF (IW,O_W,15)
         ELSE IF (O(O_S)==O(97)) THEN
            IF (O(68)==0) THEN
               NERD=LINF(O_S+2,O_S+O(O_S+1)+1,12)
               O_S=O_S+O(O_S+1)+2
            END IF
         ELSE IF (O(O_S)==O(92)) THEN
            O(24)=1
         ELSE IF (O(O_S)==O(93)) THEN
            O(24)=0
         ELSE IF (O(68)==0) THEN
            O_W=O_W+1
            O(O_W)=O(O_S)
            IF (O(O_W)==O(105)) THEN
               DO
                  IF(O(O_W)==O(106))EXIT
                  O_W=O_W+1
                  O_S=O_S+1
                  O(O_W)=O(O_S)
               END DO
            END IF
         END IF
         O_S=O_S+1
         IF(O_S>O_U .OR. O(24)==0)EXIT
      END DO
      GO TO 30
      END
!_______________________________________________________________________
!__________________________________________
      integer FUNCTION ISPEC(P,Q,IV,U,T)
      implicit none
      integer p,q,iv,u,t
      DIMENSION O(600000)
      COMMON /IBM/O
      integer O
      integer O_B,O_E,O_R,O_S,O_V,O_U,O_W,O_X
      equivalence (O(1),O_B),(O(2),O_E),(O(3),O_R),(O(4),O_S), &
                  (O(5),O_V),(O(6),O_U),(O(7),O_W),(O(8),O_X)
      integer mini
      integer c,m,b,v,iq,i,np,jmp,n,nul,w
      save    c,m,b,v,iq,i,np,jmp,n,nul,w
      C=0
      M=0
      B=0
      V=IV
      O(45)=2155
!ADEFGHIJKLRSXYZ
      O(46)=2205
         ISPEC_SCAN: DO
      IF(P<Q) THEN
         IF (O(V)==O(76)) THEN
            IF (V==T) THEN
               V=V+1
               T=T+1
               IF(V>U)THEN
                  ISPEC=1
                  RETURN
               END IF
               CYCLE ISPEC_SCAN
            END IF
         END IF
         IF (O(P)<=100) THEN
            IQ=Q
            I=MINI(P,Q,NP,1505)
            IF(O(29)>=4) CALL MACTRC(5,V,U)
            IF(I==0)THEN
               ISPEC=2
               RETURN
            END IF
            O(45)=O(45)+1
            O(O(45))=IQ
            O(45)=O(45)+1
            O(O(45))=NP
            IF(O(45)>2204)CALL MESAGE(15, 19, 0,0)
            CYCLE ISPEC_SCAN
         END IF
            IF (O(P)>=O(106)) THEN
               CALL MESAGE(14, 19, P,Q+11)
               ISPEC=2
               RETURN
            END IF
            P=P+1
            JMP=O(P-1)-100
            SELECT CASE (JMP)
            CASE (1)
               IF (M/= 0) THEN
                  N=1
                  DO
                     IF(N==0)EXIT
                     IF( O(103) ==O(P))N=N+1
                     IF( O(104) ==O(P))N=N-1
                     P=P+1
                     IF(P>Q)CALL MESAGE(14, 27, 0,0)
                  END DO
                  P=P-1
               ELSE
                  V=O(O(46)-1)
               END IF
            CYCLE ISPEC_SCAN
            CASE (3)
            O(46)=O(46)+1
            O(O(46))=C
            O(46)=O(46)+1
            O(O(46))=V
            O(46)=O(46)+1
            O(O(46))=P
            IF(O(46)>2254)CALL MESAGE(15, 20, 0,0)
            C=0
            CYCLE ISPEC_SCAN
            CASE (4)
               P=P+2
               IF (M==1) THEN
                  C=C+1
                  IF (C<O(P-1)) THEN
                     P=O(O(46))
                     CYCLE ISPEC_SCAN
                  END IF
               END IF
               IF(C>=O(P-2))M=1
               IF( M==0 ) V=O(O(46)-1)
               IF (O(46)==2205) THEN
                  CALL MESAGE(14, 17, 0,0)
                  ISPEC=2
                  RETURN
               END IF
               NUL=O(O(46))
               O(46)=O(46)-1
               NUL=O(O(46))
               O(46)=O(46)-1
               C=O(O(46))
               O(46)=O(46)-1
               GO TO 1300
            CASE (2)
            P=P+2
            M=0
            IF (O(P-2)<=O(V).AND.O(V)<=O(P-1)) THEN
               M=1
               V=V+1
               IF(V>U)THEN
                  ISPEC=1
                  RETURN
               END IF
            END IF
            GO TO 1300
            CASE (5)
            W=V
            M=1
            IF (O(P)==O(76)) THEN
               DO
                  IF(O(W)/= O(76).OR.O(P)/= O(76))EXIT
                  W=W-1
                  P=P+1
               END DO
            END IF
            DO
               IF(O(P)==O(106))EXIT
               IF(O(P)/= O(W).OR.W>U)M=0
               P=P+1
               W=W+1
            END DO
            P=P+1
            IF(M==1)V=W
            END SELECT
 1300       IF(M==1)CYCLE ISPEC_SCAN
            IF(O(P)/= O(101) .AND. O(P)/= O(104))THEN
               ISPEC=2
               RETURN
            END IF
         CYCLE ISPEC_SCAN
         END IF
            IF (O(45)/=2155) THEN
         P=O(O(45))
         O(45)=O(45)-1
         Q=O(O(45))
         O(45)=O(45)-1
            CYCLE ISPEC_SCAN
            END IF
         EXIT ISPEC_SCAN
      END DO ISPEC_SCAN
      IV=V
      ISPEC=3
      RETURN
      END FUNCTION ISPEC
!_______________________________________________________________________
!__________________________________________
      integer FUNCTION MINI(S,U,W,KCT)
      implicit none
      integer s,u,w,kct
      DIMENSION O(600000)
      COMMON /IBM/O
      integer O
      integer O_B,O_E,O_R,O_S,O_V,O_U,O_W,O_X
      equivalence (O(1),O_B),(O(2),O_E),(O(3),O_R),(O(4),O_S), &
                  (O(5),O_V),(O(6),O_U),(O(7),O_W),(O(8),O_X)
      integer m,l,v,p,q
      save    m,l,v,p,q
      MINI=0
      M=KCT+O(S)
!ABCDEFGHIJKNRTXYZ
      L=U-S
      DO
         IF (O(M)==0) THEN
            CALL MESAGE(14, 15, S,U+1)
            RETURN
         END IF
         M=O(M)
         V=S
         P=M+4
         Q=O(M+3)
!   IF Q-PnL jM;
         IF(O(29)>=6) CALL MACTRC(2,P,Q)
         DO WHILE (P<Q)
            IF(O(P)/= O(V))EXIT
            P=P+1
            V=V+1
         END DO
         IF (P>=Q) THEN
         S=O(M+2)+4
         U=O(O(M+2)+3)
         MINI=1
         W=V
            RETURN
         END IF
      END DO
      END FUNCTION MINI
!_______________________________________________________________________
!________________________________________
      SUBROUTINE EXPAND (F)
      implicit none
      integer  f
      DIMENSION O(600000)
      COMMON /IBM/O
      integer O
      integer O_B,O_E,O_R,O_S,O_V,O_U,O_W,O_X
      equivalence (O(1),O_B),(O(2),O_E),(O(3),O_R),(O(4),O_S), &
                  (O(5),O_V),(O(6),O_U),(O(7),O_W),(O(8),O_X)
      integer lexp,llong
      integer kgn,t,sef,k,j,jv,p,q,nul,jmp,a,n,krap,kraq,z,y
      DATA KGN/0/
      save    kgn,t,sef,k,j,jv,p,q,nul,jmp,a,n,krap,kraq,z,y
!CDGHILM
      T=0
      IF (F>=0) GO TO 1440
         O(23)=O(23)-1
         IF(O(23)==0)O(15)=SEF
         DO K = 1,6
            O(K)=O(6+O_U+1-K)
         END DO
         J=O(O(O(51)))
         IF (J>=11 .AND. J<=12) THEN
            JV=LEXP(O_W+1,O_X)
            IF (O(58)==0) THEN
               IF (J==12) THEN
                  O_X=O_W+LLONG(O_W,JV,10,0)
               END IF
               IF (J==11) THEN
                  O_X=O_W+1
                  O(O_X)=JV
               END IF
            ELSE
               O_X=O_W
               O(58)=O(58)-1
               IF (JV==0) THEN
                  P=O(65)
                  Q=O(64)
               ELSE
                  P=O(67)
                  Q=O(66)
               END IF
               O(44)=O(44)+1
               O(O(44))=O(64)+1
               O(44)=O(44)+1
               O(O(44))=O_E
               IF(O(44)>2154)CALL MESAGE(15, 18, 0,0)
               O_B=P
               O_E=Q
            END IF
               END IF
         NUL=O(O(51))
         O(51)=O(51)-1
         O_W=O(O(51))
         O(51)=O(51)-1
         O(32)=O(O(51))
         O(51)=O(51)-1
         GO TO 1430
 1440    O(O_S-1)=F
         O(40) = O_S-2*F-3
         O_R=O(O(22)+2)
         O_B=O_R+4
         O_E=O(O_R+3)
         O_R=O(O_R)
         O(62)=O(62)+1
         IF (O(62)>1000) THEN
            CALL MESAGE(14, 4, 0,0)
            O(29)=2
         END IF
         IF(O(29)>=2) CALL MACTRC(22,O(22),O_S)
         IF( O(23)==0 )O_X=O_W
 1430 CONTINUE
 1580 CONTINUE
      DO WHILE (O_B<O_E)
         IF (O(O_B)<O(107)) THEN
               O_X=O_X+1
               IF(O_X>O(40))CALL MESAGE(15, 3, 0,0)
               O(O_X)=O(O_B)
               O_B=O_B+1
               CYCLE
         END IF
            O_B=O_B+1
            JMP=O(O_B-1)-O(106)
            SELECT CASE (JMP)
            CASE (1)
               GO TO 1620
            CASE (2)
               GO TO 1630
            CASE (3)
               GO TO 1640
            CASE (4)
               GO TO 1650
            CASE DEFAULT
               GO TO 1630
            END SELECT
 1630       IF (O(O_B)>=1 .AND. O(O_B)<=O(O_S-1)) THEN
               A=O(O_S-2*(O(O_B))-1)
               N=O(O_S-2*(O(O_B))-2)
               DO WHILE (A<N)
                  O_X=O_X+1
                  IF(O_X>O(40))CALL MESAGE(15, 3, 0,0)
                  O(O_X)=O(A)
                  A=A+1
                  END DO
               GO TO 1690
            ELSE
            CALL MESAGE(15, 4, O_B,O_E)
            END IF
 1620       IF (O(O_B)==12) THEN
               O_B=O_B+1
               IF (O(O_B)==23) THEN
                  IF (O(68)==0) THEN
                     KGN=1
                     O(68)=1
                  ELSE
                     KGN=KGN+1
                  END IF
               ELSE IF (O(O_B)==16) THEN
                  IF (O(68)==1) KGN=KGN+1
               ELSE IF (O(O_B)==14) THEN
                  KGN=KGN-1
                  IF(KGN<=0)O(68)=0
                  END IF
               IF (O(71)/=0) THEN
                  IF(O(68)==0)O(32)=1625
                  IF(O(68)/= 0)O(32)=1145
               END IF
            ELSE
            IF(O(68)==0)CALL KAT
            END IF
            GO TO 1690
 1640       IF (O(O_B)==10) THEN
               O_B=O_B+1
               IF(1>O(O_B).OR.O(O_B)>O(O_S-1) )CALL MESAGE(15, 4, O_B,O_E)
               O_X=O_X+LLONG(O_X,O(O_S-2*(O(O_B))-2)-O(O_S-2*(O(O_B))-1),10,0)
               IF(O_X>O(40))CALL MESAGE(15, 3, 0,0)
               O_B=O_B+2
               CYCLE
            ELSE IF (O(O_B)==13) THEN
               O_B=O_B+1
               T=1
               DO
                  IF( O(109) ==O(O_B))T=T+1
                  IF( O(110) ==O(O_B))T=T-1
                  IF(T==0)EXIT
                  O_X=O_X+1
                  IF(O_X>O(40))CALL MESAGE(15, 3, 0,0)
                  O(O_X)=O(O_B)
                  O_B=O_B+1
               END DO
               GO TO 1690
            ELSE IF (O(O_B)==15) THEN
               O_X=O_X+1
               O(O_X)=O(97)
               O_X=O_X+1
               KRAP=O_X
               KRAQ=O(23)
               GO TO 1690
            ELSE
            O(23)=O(23)+1
            IF(O(23)==1)SEF=O(15)
            O(15)=1
            O(51)=O(51)+1
            O(O(51))=O(32)
            O(51)=O(51)+1
            O(O(51))=O_W
            O(51)=O(51)+1
            O(O(51))=O_B
            IF(O(51)>2554)CALL MESAGE(15, 34, 0,0)
            O_W=O_X
            GO TO 1690
            END IF
 1650       IF (KRAP/=0 .AND. KRAQ==O(23)) THEN
               O_X=O_X+1
               O(O_S)=O(96)
               O(KRAP)=O_X-KRAP-1
               KRAP=0
               CYCLE
            END IF
            DO K = 1,6
               O(O(40)-K)=O(K)
            END DO
            O_U=O(40)-7
            O_V=O_U+1
            Z=O(O(51))
            Y=O(Z)
            IF(O(Z+2)==O(105))Y=0
            IF((9<Y).AND.(Y<15))Y=O(31)
            IF( 0>Y.OR.Y>=8 )CALL MESAGE(15, 17, O_B,O_E)
            O(32)=Y*120+1145
            EXIT
 1690       O_B=O_B+1
         CYCLE
      END DO
         IF (O(23)<=0) THEN
            IF (O(44)==2105) THEN
               IF (O_R>0) THEN
                  O_B=O_R+4
                  O_E=O(O_R+3)
                  O_R=O(O_R)
                  GO TO 1580
               END IF
            ELSE
               O_E=O(O(44))
               O(44)=O(44)-1
               O_B=O(O(44))
               O(44)=O(44)-1
               GO TO 1580
            END IF
         END IF
      IF(O(29)>=2) CALL MACTRC(33,O_W,O_X)
      DO WHILE (O_X>O_W)
         O_V=O_V-1
         O(O_V)=O(O_X)
         O_X=O_X-1
      END DO
      O_S=O_V
       RETURN
      END SUBROUTINE EXPAND
!TO EXPAND
!_______________________________________________________________________
!________________________________________
      SUBROUTINE KAT
      implicit none
      DIMENSION O(600000)
      COMMON /IBM/O
      integer O
      integer O_B,O_E,O_R,O_S,O_V,O_U,O_W,O_X
      equivalence (O(1),O_B),(O(2),O_E),(O(3),O_R),(O(4),O_S), &
                  (O(5),O_V),(O(6),O_U),(O(7),O_W),(O(8),O_X)
      integer l,m
      EQUIVALENCE (O(47),L),(O(48),M)
      integer llong,nib
      integer jmp,t,ibg,z,iuni,nba
      save    jmp,t,ibg,z,iuni,nba
!FGHIJKLPQ  YZ
      IF(O(O_B)<19.OR.O(O_B)>30) CALL MESAGE(15, 7, O_B-2,O_E)
      O_B=O_B+1
      JMP=O(O_B-1)-18
      SELECT CASE (JMP)
      CASE (1)
         T=O(O_B)+78
         O_X=O_X+1
         O(O_X)=O(T)
         RETURN
      CASE (2)
         CALL KRUNC(0)
         RETURN
      CASE (3)
         IF (O(O_B)==12) THEN
            O_X=O_X+LLONG(O_X,O(L-O(O_B+1))+O(O_B+2),10,1)
            O_B=O_B+2
            RETURN
         ELSE IF (O(O_B)==16) THEN
            O(16)=O(16)+10
            O_X=O_X+LLONG(O_X,O(16),10,0)
            RETURN
         ELSE IF (O(O_B)==28) THEN
            O_B=O_B+1
            O(47)=O(47)+1
            O(O(47))=NIB(O_X,10)
            IF(O(47)>2304)CALL MESAGE(15, 21, 0,0)
            IF (O(O_B)==1) THEN
               Z=O(L-1)
               O(L-1)=O(L)
               O(L)=Z
            END IF
         ELSE IF (O(O_B)==30) THEN
            O_B=O_B+1
            IF(O(O_B)==1)O(L-1)=O(L)
            L=L-1
            IF (L<2255) THEN
               CALL MESAGE(14, 7, 0,0)
               L=L+1
            END IF
         ELSE
            CALL MESAGE(15, 7, O_B-2,O_E)
            RETURN
         END IF
         IF(O(O_B)/=1.AND.O(O_B)/=0)THEN
            write(6,*) 'compilation error (stop 18)'
            call exit(18)
         END IF
         RETURN
      CASE (4)
         IF (O(O_B)==28) THEN
            O_B=O_B+1
            O(48)=O(48)+1
            O(O(48))=O(O_B)
            IF(O(48)>2354)CALL MESAGE(15, 22, 0,0)
         ELSE IF (O(O_B)==27) THEN
            O_B=O_B+1
            O(M)=O(O_B)
         ELSE IF (O(O_B)==30) THEN
            O_X=O_X+1
            O(O_X)=O(M)
            M=M-1
            IF (M<2305) THEN
               CALL MESAGE(14, 8, 0,0)
               M=M+1
            END IF
         ELSE
            CALL MESAGE(15, 7, O_B-2,O_E)
         END IF
         RETURN
      CASE (5)
         IBG=O(O_X)
         O_X=O_X-1+LLONG(O_X-1,IBG,10,1)
         RETURN
      CASE (6)
         CALL DEFINE(O_S)
         RETURN
      CASE (7)
         O(58)=1
         O_B=O_B+4
         O(67)=O_B+O(O_B-1)
         O(66)=O_B+O(O_B-2)
         O(65)=O_B+O(O_B-3)
         O(64)=O_B+O(O_B-4)
         O_B=O_B-1
         RETURN
      CASE (8)
         O(16)=O(16)+10
         O(47)=O(47)+1
         O(O(47))=O(16)
         IF(O(47)>2304)CALL MESAGE(15, 21, 0,0)
         IF (O(O_B)==1) THEN
            Z=O(L-1)
            O(L-1)=O(L)
            O(L)=Z
         END IF
         IF(O(O_B)/=1.AND.O(O_B)/=0)THEN
            write(6,*) 'compilation error (stop 18)'
            call exit(18)
         END IF
         RETURN
      CASE (9)
         IUNI=O(O_B)
         REWIND IUNI
         RETURN
      CASE (10)
         T=O(O_B)
         O_B=O_B+1
         IF (0<O(O_B).AND.O(O_B)<5) THEN
            T=T+26*O(O_B)
            O_B=O_B+1
         END IF
         NBA=0
      CASE (11)
         IF(O(29)>=1) CALL MACTRC(22,O(22),O_S)
         O_B=O_B-1
         RETURN
      CASE (12)
         T=121+O(O_B)
         O_B=O_B+1
         NBA=O(O_X)
         O_X=O_X-1
      CASE DEFAULT
         CALL MESAGE(15, 7, O_B-2,O_E)
         RETURN
      END SELECT
      IF (O(O_B)==43) THEN
         O(T)=O(T)+1
      ELSE IF (O(O_B)==42) THEN
         O(T)=O(T)-1
      ELSE IF (O(O_B)==41) THEN
         O(T)=O(O_X)
         O_X=O_X-1
      ELSE IF (O(O_B)==12) THEN
         O_X=O_X+1
         O(O_X)=O(T)
         IF (NBA>1) O_X=O_X-1+LLONG(O_X-1,O(T),NBA,1)
      ELSE
         CALL MESAGE(15, 7, O_B-3,O_E)
      END IF
      RETURN
      END SUBROUTINE KAT
!_______________________________________________________________________
!__________________________________________
      SUBROUTINE COUT
      implicit none
      DIMENSION O(600000)
      COMMON /IBM/O
      integer O
      integer O_B,O_E,O_R,O_S,O_V,O_U,O_W,O_X
      equivalence (O(1),O_B),(O(2),O_E),(O(3),O_R),(O(4),O_S), &
                  (O(5),O_V),(O(6),O_U),(O(7),O_W),(O(8),O_X)
      integer linf
      integer t,nerd
      save    t,nerd
      T=O_S
      DO
         IF(T>O_V)EXIT
         IF (O(T)==O(97)) THEN
            NERD=LINF(T+2,T+O(T+1)+1,12)
            T=T+O(T+1)+2
         END IF
         T=T+1
      END DO
      RETURN
      END SUBROUTINE COUT
!_______________________________________________________________________
!________________________________________
      SUBROUTINE DEFINE(S)
      implicit none
      integer s
      DIMENSION O(600000)
      COMMON /IBM/O
      integer O
      integer O_B,O_E,O_R,O_S,O_V,O_U,O_W,O_X
      equivalence (O(1),O_B),(O(2),O_E),(O(3),O_R),(O(4),O_S), &
                  (O(5),O_V),(O(6),O_U),(O(7),O_W),(O(8),O_X)
!GHQUVWXYZ
      integer   l,k,a,t,n,z,b,e,m,p,lp,c,need,j,f,i,r
      logical found
      save      l,k,a,t,n,z,b,e,m,p,lp,c,need,j,f,i,r
      DIMENSION L(3)
      IF (O(S-2*(1)-1)==O(S-2*(1)-2)) THEN
         CALL MESAGE(14, 1, 0,0)
         RETURN
      END IF
      O(54)=O(31)
      IF ( O(S-2*(2)-1)/= O(S-2*(2)-2) )O(54)=O(O(S-2*(2)-1))
      DO K = 1,3
         A=O(S-2*(K)-1)
         T=O(S-2*(K)-2)
         N=A-1
         Z=0
          DO WHILE (A<=T)
            N=N+1
            O(N)=O(A)
            A=A+1
            IF (O(N)==O(86)) THEN
               IF (K==3.OR.O(28)>0) THEN
                  IF (Z==0) THEN
                     O(N)=O(105)
                     Z=1
                  ELSE IF (O(A)==O(86).AND.A<=T) THEN
                     A=A+1
                  ELSE
                     O(N)=O(106)
                     Z=0
                  END IF
               END IF
            ELSE IF (O(N)==O(76)) THEN
                  IF (Z==0) THEN
                     DO WHILE (O(A)==O(76).AND.A<T)
                        A=A+1
                     END DO
                  END IF
               ELSE IF (O(N)==O(78)) THEN
                  IF (O(A)==O(78)) THEN
                     A=A+1
                  ELSE
                     O(N)=O(108)
                  END IF
               ELSE IF (O(N)==O(77)) THEN
                  IF (O(A)==O(77)) THEN
                     A=A+1
                  ELSE
                     O(N)=O(107)
                  END IF
               END IF
            IF (O(33)==0 .AND. K==3) THEN
               IF(O(N)==O(84))O(N)=O(109)
               IF(O(N)==O(85))O(N)=O(110)
            END IF
          CYCLE
          END DO
         O(S-2*(K)-2)=N
         L(K)=O(S-2*(K)-2)-O(S-2*(K)-1)
         END DO
      IF (O(54)>=12 .AND. O(54)<=14) THEN
         B=O(S-2*(1)-1)
         E=O(S-2*(1)-2)
         IF (O(54)==14) THEN
            M=1505+O(B)
         ELSE
            M=1625+O(B)
         END IF
         found=.false.
         DEFINE_LOOKUP: DO
            IF (O(M)==0) THEN
               IF (O(54)==12) THEN
                  O(54)=O(31)
                  EXIT DEFINE_LOOKUP
               END IF
               CALL MESAGE(14, 2, 0,0)
               RETURN
            END IF
            M=O(M)
            P=M+4
            LP=O(M+3)-P
            IF (LP/=L(1)) CYCLE DEFINE_LOOKUP
            A=B
            DO WHILE (O(P)==O(A))
               P=P+1
               A=A+1
               IF (A>=E) THEN
                  found=.true.
                  EXIT DEFINE_LOOKUP
               END IF
            END DO
         END DO DEFINE_LOOKUP
         IF (found) THEN
            IF (O(54)/=12) THEN
               IF(O(M)/= 0)O((O(M))+1)=O(M+1)
               O(O(M+1))=O(M)
               C=O(M+2)
               O(M+2)=O(90)
               DO WHILE (C/=0)
                  O(C+2)=O(90)
                  C=O(C)
               END DO
               IF(O(13)>0)CALL MACTRC(13,0,S)
               RETURN
            END IF
            NEED=4+L(3)
            IF(NEED>450740-O(34)) CALL KRUNC(NEED)
            C=O(M+2)
            DO WHILE (O(C)/=0)
               C=O(C)
            END DO
            O(C)=O(34)
            O(O(34))=0
            O(O(34)+1)=C
            O(O(34)+2)=O(91)
            O(O(34)+3)=O(34)+NEED
            O(34)=O(34)+NEED
            A=O(S-2*(3)-2)
            K=L(3)
            DO J = 1,K
               O(O(34)-J)=O(A-J)
            END DO
            IF(O(13)>0)CALL MACTRC(12,0,S)
            RETURN
         END IF
      END IF
      NEED=L(1)+L(3)+2*4
      IF (NEED>450740-O(34)) CALL KRUNC(2+NEED)
      A=O(S-2*(1)-1)
      Z=A+L(1)-1
      P=O(34)
      F=P+4
      I=F
       DO WHILE (A<=Z)
         O(F)=O(A)
         F=F+1
         A=A+1
       END DO
      IF (O(I)==O(84) .AND. O(54)>2) THEN
         CALL MESAGE(14, 21, O(S-2*(1)-1),O(S-2*(1)-2))
         RETURN
      END IF
      IF (O(I)>O(105)) THEN
         CALL MESAGE(14, 9, O(S-2*(1)-1),O(S-2*(1)-2))
         RETURN
      END IF
      IF (O(54)<0 .OR. O(54)>7) THEN
         CALL MESAGE(14, 12, O(S-2*(1)-1),O(S-2*(1)-2))
         RETURN
      END IF
      I=O(I) + 1145 + O(54)*120
      O(P)=O(I)
      IF(O(P)/= 0)O((O(P))+1)=P
      O(P+1)=I
      O(I)=P
      O(P+2)=F
      O(P+3)=F
      O(F)=0
      O(F+1)=P+2
      O(F+3)=F+4+L(3)
      O(F+2)=O(91)
      R=F
      F=P+NEED
      E=O(S-2*(3)-2)
      K=L(3)
      IF (K/=0) THEN
         CALL DOIT(E-K,E)
         DO J = 1,K
            O(F-J)=O(E-J)
         END DO
      END IF
      IF (O(13)>0) CALL MACTRC(15,P,R)
      O(34)= P +NEED
      O(60)=450740-O(34)
      O(61)=(100*O(60))/(450740-2655)
 RETURN
      END SUBROUTINE DEFINE
!_______________________________________________________________________
!__________________________________________
      SUBROUTINE DOIT(A,Z)
      implicit none
      integer a,z
      DIMENSION O(600000)
      COMMON /IBM/O
      integer O
      integer O_B,O_E,O_R,O_S,O_V,O_U,O_W,O_X
      equivalence (O(1),O_B),(O(2),O_E),(O(3),O_R),(O(4),O_S), &
                  (O(5),O_V),(O(6),O_U),(O(7),O_W),(O(8),O_X)
      integer   s,x,t,p
      DIMENSION S(200)
      save      s,x,t,p
      X=0
!BCDEFGHIJKLMNQRUVWY
      T=Z
      DO WHILE (T>=A)
         IF (O(T)==O(95)) THEN
            IF (O(T-1)==O(94)) THEN
               S(X+1)=T+1
               S(X+2)=O(T)
               X=X+2
               T=T-1
            ELSE
               S(X+1)=T+1
               X=X+1
               DO
                  IF(O(T)==O(107).AND.O(T+1)==25)EXIT
                  T=T-1
                  IF(T<A)RETURN
               END DO
               P=T+6
               O(P-1)=S(X)-P
               X=X-1
               IF (S(X)==O(95)) THEN
                  O(P-4)= S(X-2)-P
                  O(P-3)= S(X-1)-P
                  O(P-2)= O(P-3)-2
                  X=X-3
               ELSE
                  O(P-2)=S(X)-P
                  O(P-3)=S(X)-P
                  O(P-4)=S(X)-P
                  X=X-1
               END IF
            END IF
         ELSE IF (O(T)==O(94)) THEN
            S(X+1)=T
            X=X+1
         END IF
         T=T-1
      END DO
      RETURN
      END SUBROUTINE DOIT
!_______________________________________________________________________
!__________________________________________
      SUBROUTINE KRUNC(R)
      implicit none
      integer    r
      DIMENSION O(600000)
!ABEFGHIJKLQTUVWXYZ
      COMMON /IBM/O
      integer O
      integer O_B,O_E,O_R,O_S,O_V,O_U,O_W,O_X
      equivalence (O(1),O_B),(O(2),O_E),(O(3),O_R),(O(4),O_S), &
                  (O(5),O_V),(O(6),O_U),(O(7),O_W),(O(8),O_X)
      integer m,n,d,p,s,c,i
      save    m,n,d,p,s,c,i
      IF (O(35)/=0) THEN
         M=O(35)
         DO WHILE (M/=O(34))
            IF (O(M+2)>=0) THEN
               IF(O(M)/= 0)O((O(M))+1)=O(M+1)
               O(O(M+1))=O(M)
            END IF
            O(M+2)=O(90)
            M=O(M+3)
         END DO
      END IF
      N=2655
      M=O(N+3)
      D=0
! PREDECESSOR  SUCCESSOR    CONCAT
      O(O(34))=0
      DO WHILE (N/=O(34))
         M=N
         N= O(M+3)
         P=O(M+1)
         S=O(M)
         C=O(M+2)
         IF (C==O(90)) THEN
            D=D+ N-M
         ELSE
            IF (S>0) O(S+1)=O(S+1)-D
            IF (C>0) O(N+1)=O(N+1)-D
            O(P)=O(P)-D
            O(M+3)=O(M+3)-D
!   MOVE IT
            DO I = M,N
               O(I-D)=O(I)
            END DO
         END IF
      END DO
      O(34)=O(34)-D
      O(O(34))=0
      IF (R>D)CALL MESAGE(15, 6, 0,0)
      O(60)=450740-O(34)
      O(61)=(100*O(60))/(450740-2655)
      CALL MESAGE(13,1,D,0)
       RETURN
      END SUBROUTINE KRUNC
!_______________________________________________________________________
!__________________________________________
      integer FUNCTION NIB(L,B)
      implicit none
      integer  l,b
      DIMENSION O(600000)
      COMMON /IBM/O
      integer O
      integer O_B,O_E,O_R,O_S,O_V,O_U,O_W,O_X
      equivalence (O(1),O_B),(O(2),O_E),(O(3),O_R),(O(4),O_S), &
                  (O(5),O_V),(O(6),O_U),(O(7),O_W),(O(8),O_X)
      integer m,n
      save    m,n
      M=1
      N=0
      DO
         IF(O(L)/= O(76))EXIT
         L=L-1
      END DO
      DO
         IF(0>O(L).OR.O(L)>=B)EXIT
         N=N+O(L)*M
         M=M*B
         L=L-1
      END DO
      IF (O(L)==42) THEN
         L=L-1
         N=-N
      END IF
      NIB=N
       RETURN
      END FUNCTION NIB
!_______________________________________________________________________
!__________________________________________
      integer FUNCTION LLONG(LOC,NUM,B,IND)
      implicit none
      integer  loc,num,b,ind
      DIMENSION O(600000)
      COMMON /IBM/O
      integer O
      integer O_B,O_E,O_R,O_S,O_V,O_U,O_W,O_X
      equivalence (O(1),O_B),(O(2),O_E),(O(3),O_R),(O(4),O_S), &
                  (O(5),O_V),(O(6),O_U),(O(7),O_W),(O(8),O_X)
      integer n,l,i,j,k
      save    n,l,i,j,k
      IF(O(68)/= 0)RETURN
      N=IABS(NUM)
      L=LOC
      I=N/B
      J=1
      IF (IND== 0) THEN
         L=L+1
         O(L)=O(76)
      END IF
      DO WHILE(I>0)
         I=I/B
         J=J+1
      END DO
!COUNT DIGITS IN EXPANSION
      IF (NUM<0) THEN
         L=L+1
         O(L)=42
      END IF
      LLONG=L-LOC+J
      DO
         K=N/B
         O(L+J)=N-K*B
         N=K
         J=J-1
         IF(J<=0)EXIT
      END DO
 RETURN
      END FUNCTION LLONG
!_______________________________________________________________________
!________________________________________
      integer FUNCTION LEXP(A,Z)
      implicit none
      integer  a,z
      DIMENSION O(600000)
      COMMON /IBM/O
      integer O
      integer O_B,O_E,O_R,O_S,O_V,O_U,O_W,O_X
      equivalence (O(1),O_B),(O(2),O_E),(O(3),O_R),(O(4),O_S), &
                  (O(5),O_V),(O(6),O_U),(O(7),O_W),(O(8),O_X)
      integer   p,n
      DIMENSION P(100)
!FGHIJKLMQRSUVWY
      EQUIVALENCE(O(49),N),( O(2555),P(1) )
      integer ispec
      integer b,c,nq,mxc,d,e,lone,mp,mq,mv,mu,mnn,t,x,k,jmp
      save    b,c,nq,mxc,d,e,lone,mp,mq,mv,mu,mnn,t,x,k,jmp
      IF (O(A+1)==O(105)) THEN
         LEXP=1
         B=A+1
         C=B+1
         NQ=1
         MXC=Z
         DO
            IF( O(105) ==O(C))NQ=NQ+1
            IF( O(106) ==O(C))NQ=NQ-1
            IF(NQ==0)EXIT
            C=C+1
            IF(C>MXC)CALL MESAGE(15, 35, 0,0)
         END DO
         D=C+1
         E=D+1
         IF (O(E)==O(105)) THEN
            DO WHILE(E<=Z-1)
               IF(O(B)/= O(E))LEXP=0
               B=B+1
               E=E+1
            END DO
            IF(B/= D) LEXP=0
         ELSE
            LONE=1
            MP=E
            MQ=Z
            MV=B+1
            MU=C
            IF(ISPEC(MP,MQ,MV,MU,LONE)/= 3)LEXP=0
         END IF
         IF(O(D)==89) LEXP=1-LEXP
         RETURN
         END IF
      LEXP=0
      O(20)=0
      N=2355
      MNN=N+2
      O(N-1)=38
      O(N)=0
      B=A
      DO
         IF(B>Z)EXIT
         IF (O(B)>=0 .AND. O(B)<=9) THEN
            O(N)=0
            O(20)=1
            DO WHILE(0<=O(B).AND.O(B)<=9)
               O(N)=10*O(N)+O(B)
               B=B+1
            END DO
            CYCLE
         END IF
         IF (O(B)==O(76)) THEN
            B=B+1
            CYCLE
         END IF
         T=B
            IF (O(T)>=10 .AND. O(T)<=29) THEN
               X=O(T)*O(T+1)
               T=T+2
               IF (O(T)==29.OR.O(T)==13) THEN
                  X=X+O(T)
                  T=T+1
               END IF
               X=X/20
               IF (.NOT.((10<X).AND.(X<33))) EXIT
            ELSE
               X=O(T)
               T=T+1
               SELECT CASE (X)
               CASE (45)
                  IF (O(T)==45) THEN
                     T=T+1
                     X=36
                  END IF
               CASE (89)
                  X=29
                  IF (O(T)==41) THEN
                     T=T+1
                     X=16
                  END IF
               CASE (85)
                  X=30
                  IF (O(T)==41) THEN
                     T=T+1
                     X=14
                  END IF
               CASE (41)
                  X=18
                  IF (O(T)==86) THEN
                     T=T+1
                     X=11
                  END IF
               CASE (86)
                  X=23
                  IF (O(T)==41) THEN
                     T=T+1
                     X=11
                  END IF
               CASE (87)
                  X=32
               CASE (88)
                  X=13
               END SELECT
            END IF
            IF(P(X)==0)EXIT
            IF(X<10.OR.X>O(81) .OR. (X==O(81).AND.O(20)==0)) &
               EXIT
            IF (P(X)/100>P(O(N-1))/100 .OR. X==O(80)) THEN
               O(20)=0
               O(49)=O(49)+1
               O(O(49))=X
               O(49)=O(49)+1
               O(O(49))=0
               IF(O(49)>2454)CALL MESAGE(15, 23, 0,0)
               B=T
               CYCLE
            END IF
            IF (N==MNN) THEN
               LEXP=O(N)
               RETURN
            END IF
            K=P(O(N-1))
            O(20)=1
            JMP=K-((K/100)*100)-1
            SELECT CASE (JMP)
            CASE (1)
               O(N-2)=O(N)
               B=T
            CASE (2)
               IF (O(N-2)==1 .OR. O(N)==1) THEN
                  O(N-2)=1
               ELSE
                  O(N-2)=0
               END IF
            CASE (3)
               IF (O(N-2)==1 .AND. O(N)==1) THEN
                  O(N-2)=1
               ELSE
                  O(N-2)=0
               END IF
            CASE (4)
               O(N-2)=1-O(N)
            CASE (5)
               O(N-2)=O(N-2)-O(N)
            CASE (6)
               O(N-2)=O(N-2)+O(N)
            CASE (7)
               IF (O(N)==0) THEN
                  CALL MESAGE(14, 10, 0,0)
                  EXIT
               END IF
               O(N-2)=O(N-2)/O(N)
            CASE (8)
               O(N-2)=O(N-2)*O(N)
            CASE (9)
               O(N-2)=O(N-2)**O(N)
            CASE (10)
               IF (O(N-2)>=O(N)) THEN
                  O(N-2)=1
               ELSE
                  O(N-2)=0
               END IF
            CASE (11)
               IF (O(N-2)<=O(N)) THEN
                  O(N-2)=1
               ELSE
                  O(N-2)=0
               END IF
            CASE (12)
               IF (O(N-2)/=O(N)) THEN
                  O(N-2)=1
               ELSE
                  O(N-2)=0
               END IF
            CASE (13)
               IF (O(N-2)==O(N)) THEN
                  O(N-2)=1
               ELSE
                  O(N-2)=0
               END IF
            CASE (14)
               IF (O(N-2)>O(N)) THEN
                  O(N-2)=1
               ELSE
                  O(N-2)=0
               END IF
            CASE (15)
               IF (O(N-2)<O(N)) THEN
                  O(N-2)=1
               ELSE
                  O(N-2)=0
               END IF
            CASE DEFAULT
               IF (O(N)==0) THEN
                  CALL MESAGE(14, 10, 0,0)
                  EXIT
               END IF
               O(N-2)=O(N-2)/O(N)
            END SELECT
            N=N-2
      END DO
   CALL MESAGE(14, 23, A,Z+1)
 RETURN
      END FUNCTION LEXP
!_______________________________________________________________________
!________________________________________
      SUBROUTINE NXTCRD
      implicit none
      DIMENSION O(600000)
      COMMON /IBM/O
      integer O
      integer O_B,O_E,O_R,O_S,O_V,O_U,O_W,O_X
      equivalence (O(1),O_B),(O(2),O_E),(O(3),O_R),(O(4),O_S), &
                  (O(5),O_V),(O(6),O_U),(O(7),O_W),(O(8),O_X)
      integer mxr,mxt,mxn
      EQUIVALENCE(O(38),MXR),(O(36),MXT),(O(37),MXN)
      integer llong,linf
      integer k,j,w,i,l,m,y,nerd
      save    k,j,w,i,l,m,y,nerd
      O(62)=0
      IF(O(52)==2) CALL MESAGE(16,1,0,0)
      DO
      CALL RW(0,O(9),325,MXR)
      IF (O(52)==1) THEN
         O(52)=0
         O(9)=O(O(50))
         O(50)=O(50)-1
         IF (O(9)==0) THEN
            O(52)=2
            O(15)=1
            O(12)=O(43)
            O(O(12))=O(79)
            RETURN
         END IF
      END IF
      DO K= 325,MXR
         O(K+132)=O(889+O(K))
      END DO
      O(19)=O(19)+1
      O(12)=457
      O(157)=O(76)
      IF (O(457) == 93) THEN
         IF (O(458)==14) THEN
            O(157)=1
            O(325)=O(689+O(76))
            O(325+1)=O(325)
            O(12)=458+1
         ELSE IF (O(458)==93) THEN
               O(52)=1
               O(15)=1
         ELSE IF (O(458)>=10.AND.O(458)<=30) THEN
               CALL CCCARD(O(458)-9)
               O(12)=O(43)
               O(O(12))=O(79)
         END IF
         IF (O(52)==1.OR.O(458)==22.OR.O(458)==15) THEN
            O(12)=O(43)
            O(O(12))=O(79)
            RETURN
         END IF
      END IF
      IF (O(21)==1) THEN
         DO J = 158,174
            O(J)=O(76)
         END DO
         IF (O(30)==1) THEN
            J=LLONG(158,O(19),10,0)
         ELSE IF (O(30)==2) THEN
            DO J = 1,8
               O(157+J)=O(O(43)+J)
            END DO
         END IF
         IF(O(68)/= 0)O(167)=33
         O(169)=O(55)
         J=LLONG(170,O(11),10,0)
         IF (O(53)==0) THEN
            W=174
            DO K = 1,MXN
               W=W+1
               O(W)=O(456+K)
            END DO
            CALL RW(18,O(56),157,W)
         ELSE
            I=MOD(MAX0(O(53)*O(11),0),90)
            L=457
            M=O(43)
            DO WHILE(O(L)==O(76).AND.L/=M)
               L=L+1
            END DO
            DO WHILE(O(M)==O(76).AND.L/=M)
               M=M-1
            END DO
            DO
               W=175
               DO Y = 1,I
                  O(W)=O(76)
                  W=W+1
               END DO
               DO WHILE(W<=288.AND.L<=M)
                  O(W)=O(L)
                  W=W+1
                  L=L+1
               END DO
               CALL RW(18,O(56),157,W-1)
               IF(L>M)EXIT
               DO K = 157,175
                  O(K)=O(76)
               END DO
            END DO
        END IF
      END IF
      IF (O(15)==1) THEN
         IF (O(457)/= 93) CALL RW(23,O(57),325,404)
      ELSE IF (O(10)==1) THEN
         NERD=LINF(457,MXT,10)
      END IF
      IF(O(15)==1)CYCLE
      IF (O(14)<2) RETURN
      J=456
      K=457
      DO WHILE(K<=MXT)
         J=J+1
         O(J)=O(K)
         K=K+1
         IF (O(J)==45) THEN
            IF (O(K)==46) THEN
               O(J)=O(85)
               K=K+1
            ELSE IF (O(K)==86) THEN
               O(J)=O(83)
               K=K+1
            END IF
         ELSE IF (O(K)==45) THEN
            IF (O(J)==38) THEN
               O(J)=O(84)
               K=K+1
            ELSE IF (O(J)==85) THEN
               O(J)=O(82)
               K=K+1
            END IF
         END IF
      END DO
      DO WHILE(J<=MXT)
         J=J+1
         O(J)=90
      END DO
      EXIT
      END DO
      RETURN
      END SUBROUTINE NXTCRD
!_______________________________________________________________________
!__________________________________________
!_______________________________________________________________________
!________________________________________
      SUBROUTINE CCCARD(K)
      implicit none
      integer k
      DIMENSION O(600000)
      COMMON /IBM/O
      integer O
      integer O_B,O_E,O_R,O_S,O_V,O_U,O_W,O_X
      equivalence (O(1),O_B),(O(2),O_E),(O(3),O_R),(O(4),O_S), &
                  (O(5),O_V),(O(6),O_U),(O(7),O_W),(O(8),O_X)
      integer n5
      integer jmp,i
      save    jmp,i
      JMP=K
!ABCDEFGHJLMNPQRSTUVWXYZ
      SELECT CASE (JMP)
      CASE (1)
         O(10)=N5(0,2,1)
      CASE (2)
         O(37)=N5(0,132,132)
         O(38)=324+O(37)
         O(36)=456+O(37)
      CASE (3)
         O(41)=N5(10,132,132)
         O(43)=456+O(41)
         O(42)=324+O(41)
      CASE (4)
         O(13)=N5(0,2,0)
      CASE (6)
         O(15)=1
      CASE (9)
         O(53)=N5(0,50,2)
      CASE (12)
         O(21)=1
      CASE (13)
         O(15)=0
      CASE (14)
         O(21)=0
      CASE (17)
         O(26)=N5(0,1,1)
      CASE (18)
         I=N5(1,99,8)
         REWIND I
      CASE (19)
         O(30)=N5(0,2,0)
      CASE (20)
         O(29)=N5(0,9,0)
      CASE (21)
         O(50)=O(50)+1
         O(O(50))=O(9)
         IF(O(50)>2504)CALL MESAGE(15, 30, 0,0)
         O(9)=N5(1,99,O(9))
      CASE (5,7,8,10,11,15,16)
         CALL MESAGE(14, 22, 0,0)
      CASE DEFAULT
         O(10)=N5(0,2,1)
      END SELECT
      RETURN
      END SUBROUTINE CCCARD
!_______________________________________________________________________
!__________________________________________
      integer FUNCTION N5(L,M,K)
      implicit none
      integer  l,m,k
      DIMENSION O(600000)
      COMMON /IBM/O
      integer O
      integer O_B,O_E,O_R,O_S,O_V,O_U,O_W,O_X
      equivalence (O(1),O_B),(O(2),O_E),(O(3),O_R),(O(4),O_S), &
                  (O(5),O_V),(O(6),O_U),(O(7),O_W),(O(8),O_X)
      integer nib
      integer j
      save    j
      J=457+4
      N5=NIB(J,10)
      IF (.NOT.((L<=N5).AND.(N5<=M))) THEN
         CALL MESAGE(14, 3, 0,0)
         N5=K
      END IF
      RETURN
      END FUNCTION N5
!_______________________________________________________________________
!________________________________________
      SUBROUTINE MACTRC(W,R,S)
      implicit none
      integer  w,r,s
      DIMENSION O(600000)
      COMMON /IBM/O
      integer O
      integer O_B,O_E,O_R,O_S,O_V,O_U,O_W,O_X
      equivalence (O(1),O_B),(O(2),O_E),(O(3),O_R),(O(4),O_S), &
                  (O(5),O_V),(O(6),O_U),(O(7),O_W),(O(8),O_X)
      integer k,m,t,n
      save    k,m,t,n
      IF (O(23)>0.AND.O(29)<3.AND.W/= 4)RETURN
      SELECT CASE (W)
      CASE (2)
         CALL DUMDUM(175,90,37,37,37,22,42,29,27,34,90)
         O(175+4)=(O(32)-1145)/120
         CALL CRIT(R,S)
      CASE (4)
         CALL DUMDUM(175,90,45,28,29,27,10,23,16,14,90)
         CALL CRIT(R,S)
      CASE (5)
         CALL DUMDUM(175,90,37,10,16,10,18,23,28,29,90)
         CALL CRIT(R,S)
      END SELECT
      SELECT CASE (W)
      CASE (33)
         IF (O(23)==0) THEN
            CALL DUMDUM(175,90,90,27,14,13,30,12,14,13,90)
         ELSE
            CALL DUMDUM(175,90,90,90,90,90,23,14,28,29,90)
            O(175+3)=O(23)
         END IF
         CALL CRIT(R+1,S+1)
      CASE (15)
         CALL DUMDUM(175,90,90,13,14,15,18,23,14,13,90)
         CALL CRIT(R+4,O(R+3))
         IF (R+4>=O(R+3)) THEN
            CALL DUMDUM(175,90,23,30,21,21,90,31,10,21,90)
            CALL CRIT(S,S)
         ELSE
            CALL DUMDUM(175,90,90,90,90,31,10,21,30,14,90)
            O(175+2)=O(54)
            CALL CRIT(S+4,O(S+3))
         END IF
      CASE (12)
         CALL DUMDUM(175,90,10,25,25,14,23,13,14,13,90)
         CALL CRIT(O(S-2*(3)-1),O(S-2*(3)-2) )
         CALL DUMDUM(175,90,90,90,90,90,90,90,29,24,90)
         CALL CRIT(O(S-2*(1)-1),O(S-2*(1)-2) )
      CASE (13)
         CALL DUMDUM(175,90,90,27,14,22,24,31,14,13,90)
         CALL CRIT(O(S-2*(1)-1),O(S-2*(1)-2) )
      CASE (22)
         IF(O(R+4)<0)RETURN
         CALL DUMDUM(175,90,90,22,10,29,12,17,14,13,90)
         CALL CRIT(R+4,O(R+3) )
         K=O(S-1)
         DO M = 1,K
            CALL DUMDUM(175,90,90,90,90,10,27,16,90,90,90)
            O(175+8)=M
            T=O(S-2*(M)-1)
            N=O(S-2*(M)-2)
            IF (T/= N) CALL CRIT(T,N)
         END DO
      END SELECT
      RETURN
      END SUBROUTINE MACTRC
!_______________________________________________________________________
!__________________________________________
      SUBROUTINE CRIT(A,Z)
      implicit none
      integer  a,z
      DIMENSION O(600000)
      COMMON /IBM/O
      integer O
      integer O_B,O_E,O_R,O_S,O_V,O_U,O_W,O_X
      equivalence (O(1),O_B),(O(2),O_E),(O(3),O_R),(O(4),O_S), &
                  (O(5),O_V),(O(6),O_U),(O(7),O_W),(O(8),O_X)
      integer b,c
      save    b,c
            B=A
            DO WHILE (B<Z)
         C=175+9
          DO WHILE (C<246.AND.B<Z)
            C=C+1
            O(C)=O(B)
            B=B+1
            IF(O(C)==O(76))O(C)=37
            IF(O(C)>100+19)O(C)=100
            IF(O(C)==-1)O(C)=74
            IF(O(C)==-2)O(C)=82
         END DO
         CALL RW(33,O(56),175,C)
         CALL DUMDUM(175,90,90,90,90,90,90,90,90,90,90)
      END DO
      RETURN
      END SUBROUTINE CRIT
!_______________________________________________________________________
!________________________________________
      SUBROUTINE MESAGE (LEVEL,NO,K1,K2)
      implicit none
      integer level,no,k1,k2
      DIMENSION O(600000)
      COMMON /IBM/O
      integer O
      integer O_B,O_E,O_R,O_S,O_V,O_U,O_W,O_X
      equivalence (O(1),O_B),(O(2),O_E),(O(3),O_R),(O(4),O_S), &
                  (O(5),O_V),(O(6),O_U),(O(7),O_W),(O(8),O_X)
      common/junk/ verbose
      logical verbose
      IF(LEVEL==14 .OR. LEVEL==15)O(63)=O(63)+1
      CALL FIND(LEVEL,NO,K1)
      IF(K2/= 0)CALL MACTRC(4,K1,K2)
            IF (LEVEL==13) THEN
         CALL FIND(13,2,O(60))
         CALL FIND(13,3,O(61))
            END IF
            IF (LEVEL<=14) THEN
         IF(O(63)<100)RETURN
         CALL FIND(15,10,0)
            END IF
            IF (O(55)==O(86) .OR. O(55)==O(87)) THEN
         O(63)=O(63)+1
         IF(O(55)==O(86)) CALL FIND(16,5,0)
         IF(O(55)==O(87))CALL FIND(16,4,0)
            END IF
      IF(O(47)>2255.OR.O(48)>2305)O(63)=O(63)+1
            IF (O(48)>2305) THEN
         CALL FIND(16,3,O(48)-2305)
            END IF
            DO WHILE (O(47)>2255)
         CALL FIND(16,2,O(O(47)))
         O(47)=O(47)-1
            END DO
      CALL FIND(13,3,O(61))
            IF (O(63)==0) THEN
         CALL FIND(16,6,0)
            ELSE
          CALL FIND(16,7,O(63))
         write(6,*) 'compilation error (stop 12)'
         call exit(12) !stop 12
            END IF
      IF (O(59)==0)CALL INITAL(2)
      if( verbose ) write(6,*) 'OK'
      call exit(0) !stop 0
      END SUBROUTINE MESAGE
!_______________________________________________________________________
!__________________________________________
      SUBROUTINE FIND(L,N,V)
      implicit none
      integer l,n,v
      DIMENSION O(600000)
      COMMON /IBM/O
      integer O
      integer O_B,O_E,O_R,O_S,O_V,O_U,O_W,O_X
      equivalence (O(1),O_B),(O(2),O_E),(O(3),O_R),(O(4),O_S), &
                  (O(5),O_V),(O(6),O_U),(O(7),O_W),(O(8),O_X)
      integer llong
      integer a,c,q,r,s,nerd,k
      save    a,c,q,r,s,nerd,k
      A=175
      C=A+9
      Q=1865+L
 680 IF (O(Q)==0) GO TO 4610
         Q=O(Q)
         IF(O(Q+5)/= N)GO TO 680
         R=O(Q+2)+4
         S=O(O(Q+2)+3)
 4620    IF(R>=S)GO TO 4621
            C=C+1
            O(C)=O(R)
            R=R+1
         GO TO 4620
 4621    CONTINUE
         GO TO 4600
 4610    C=C+LLONG(C,N,10,0)
 4600 CONTINUE
      IF(L==15) CALL DUMDUM(175,90,45,45,45,15,10,29,10,21,90)
      IF(L==14) CALL DUMDUM(175,90,45,32,10,27,23,18,23,16,90)
      IF (L/= 13.AND.L/= 16) GO TO 4640
         CALL DUMDUM(175,90,90,90,90,90,90,90,90,90,90)
         IF(V/= 0) NERD=LLONG(A,V,10,0)
 4640 CONTINUE
      CALL RW(33,O(56),A,C)
      IF (O(63)>0) CALL RW(23,O(57),A,C)
      DO K = A,C
         O(K)=O(76)
      END DO
      C=175+8
      RETURN
      END SUBROUTINE FIND
!_______________________________________________________________________
!________________________________________
      integer FUNCTION LINF(A,Z,IND)
      implicit none
      integer  a,z,ind
      DIMENSION O(600000)
      COMMON /IBM/O
      integer O
      integer O_B,O_E,O_R,O_S,O_V,O_U,O_W,O_X
      equivalence (O(1),O_B),(O(2),O_E),(O(3),O_R),(O(4),O_S), &
                  (O(5),O_V),(O(6),O_U),(O(7),O_W),(O(8),O_X)
      integer llong
      integer last,kur,kit,kut,kst,h,b,i,j,npl,d,koff,c,limc,k
!     LINF modes: packed comments and rule diagnostics.
      integer, parameter :: linf_comment_mode = 12
      integer, parameter :: linf_rule_diagnostic_mode = 15
      DATA LAST/0/,KUR/0/,KIT/1/,KUT/0/
      save    last,kur,kit,kut,kst,h,b,i,j,npl,d,koff,c,limc,k
      LINF=A-1
!CEFGLMNPQRS UVWXY
      O(62)=0
      IF (IND==linf_comment_mode .AND. (O(25)==0.OR.O(10)==1 &
       .OR.O(96)==O(A)))RETURN
      KST=0
      H=0
      B=A
      I=175
      KUT=KUR
      IF(O(B)==O(75)) THEN
         B=B+2
         KIT=O(B-1)-1
         KUR=KUR+KIT
         KUT=KUR
         IF(KIT==0) THEN
            KUT=KUT-1
            KIT=1
         END IF
      END IF
      DO J = 175,254
         O(J)=O(76)
      END DO
      IF (IND==linf_rule_diagnostic_mode) THEN
         DO WHILE (I<181 .AND. B<=Z)
            IF (O(B)==O(76)) THEN
               B=B+1
               CYCLE
            END IF
            IF (O(B)>=0 .AND. O(B)<=9) THEN
               O(I)=O(B)
               I=I+1
               B=B+1
            ELSE
               EXIT
            END IF
         END DO
         NPL=65
         D=45
         KOFF=7
      ELSE
         NPL=71
         D=12
         O(175)=D
         KOFF=MAX(MIN(80,O(39)),2)
      END IF
      IF (B>Z .AND. I>175) THEN
         CALL RW(33,O(57),175,181)
         IF(IND==linf_rule_diagnostic_mode)CALL MESAGE(14, 14, 0,0)
         RETURN
      END IF
      C=174+KOFF+KUT*O(18)
      IF(KUT>LAST)C=C-O(18)
      IF(C>174+50)C=174+7
      I=C
      DO WHILE (B<=Z)
         LIMC=MIN0(B+NPL,Z)
         DO WHILE (B<=LIMC .AND. I<=246)
            IF (IND==linf_rule_diagnostic_mode) THEN
               IF (O(17)==1) THEN
                  IF (O(B)==O(105)) THEN
                     IF (I>246-4) EXIT
                     H=1
                     DO WHILE (O(B+H)/=O(106))
                        H=H+1
                     END DO
                     O(B+H)=O(76)
                     IF (H==1) THEN
                        O(B)=O(86)
                        H=0
                     ELSE
                        I=I+LLONG(I-1,H-1,10,0)
                        O(I)=17
                        B=B+1
                        I=I+1
                     END IF
                  END IF
               ELSE IF (O(B)==O(86)) THEN
                  O(I)=O(B)
                  I=I+1
               END IF
            END IF
            IF (O(B)==O(70)) THEN
               B=B+1
               EXIT
            END IF
            O(I)=O(B)
            I=I+1
            B=B+1
         END DO
         IF (O(30)==1) THEN
            DO K = 1,8
               O(254-K+1)=O(O(36)-K+1)
            END DO
         ELSE IF (O(30)==2) THEN
            K=LLONG(247,O(19),10,0)
         END IF
         DO K = 175,246
            IF (O(K)==O(105)) THEN
               O(K)=O(86)
               KST=1
            END IF
            IF (O(K)==O(106)) THEN
               O(K)=O(86)
               KST=0
            END IF
         END DO
        IF (I>246 .AND. IND==linf_rule_diagnostic_mode &
           .AND. KST==0) THEN
          DO WHILE (O(I-1)<38 .AND. KST>0)
               B=B-1
               I=I-1
               O(I)=O(76)
          END DO
        END IF
         DO K = 175,254
            IF(0>O(K).OR.O(K)>100-1)O(K)=100-1
            O(K)=O(689+O(K))
         END DO
!   LOOKOUT
         CALL RW(23,O(57),175,254)
         DO K = 175,254
            O(K)=O(76)
         END DO
         I=C
         IF (IND==linf_rule_diagnostic_mode) THEN
            O(175+5)=D
         ELSE
            O(175)=D
         END IF
         IF (KST>0) THEN
            I=181
            O(181-1)=45
         END IF
      END DO
      LAST=KUR
      IF(IND==linf_comment_mode) O(A)=O(96)
 RETURN
      END FUNCTION LINF
!_______________________________________________________________________
!__________________________________________
      SUBROUTINE DUMDUM(L,A,B,C,D,E,F,G,H,I,J)
      implicit none
      integer L,A,B,C,D,E,F,G,H,I,J
      DIMENSION O(600000)
      COMMON /IBM/O
      integer O
      integer O_B,O_E,O_R,O_S,O_V,O_U,O_W,O_X
      equivalence (O(1),O_B),(O(2),O_E),(O(3),O_R),(O(4),O_S), &
                  (O(5),O_V),(O(6),O_U),(O(7),O_W),(O(8),O_X)
      O( L )=A
      O(L+1)=B
      O(L+2)=C
      O(L+3)=D
      O(L+4)=E
      O(L+5)=F
      O(L+6)=G
      O(L+7)=H
      O(L+8)=I
      O(L+9)=J
       RETURN
      END SUBROUTINE DUMDUM
!_______________________________________________________________________
!__________________________________________
      SUBROUTINE RW(N,I,J,K)
      implicit none
      integer  N,I,J,K
      CHARACTER*1 CH(200)
      INTEGER     HC(200)
      DIMENSION O(600000)
      COMMON /IBM/O
      integer O
      integer O_B,O_E,O_R,O_S,O_V,O_U,O_W,O_X
      equivalence (O(1),O_B),(O(2),O_E),(O(3),O_R),(O(4),O_S), &
                  (O(5),O_V),(O(6),O_U),(O(7),O_W),(O(8),O_X)
!  IN RW
!
      common/nrcc/ oflag
      integer      oflag
!
      logical skip_input,is_format,is_quote,is_quote_last
      integer nr,m,kof,l,nrr,nformat,itmp,nfl
      integer nn6,nn7,jj,nfirst,ii,m1
      character tmp_string*8, tmp1_string*6, f_buffer(8192)
      equivalence (ch,tmp_string)
      data nn6,nn7,nfl/0,0,0/, &
           skip_input/.false./, is_format/.false./, is_quote/.false./
      data nformat/0/
      save nn6,nn7,hc,ch,nformat,nfl
      save nr,m,kof,l,nrr,skip_input,is_format,is_quote
      save f_buffer

      NR=K+1-J
   IF (N==0) THEN
         !write(44,*) 'nr = ',nr,k,j,i
         READ(I,1,END=5130) (CH(M),M=1,NR)
         if( i/= 1 ) then
         do m=1,nr-1
           if( .not.skip_input ) then
             ! the following does not work because of possibly
             ! having 2 or more / in a format statement
             !
             !if( ch(m)=='/'.and.ch(m+1)=='/' ) then
             !  ! a C++ style comment
             !  write(44,*) 'found C++ style comment: '
             !  write(44,1) (CH(ii),ii=1,NR)
             !  do m1=m,nr
             !    ch(m1) = ' '
             !  end do
             !  GO TO 50
             !end if
             if( ch(m)=='/'.and.ch(m+1)=='*' ) then
               ! A C style comment
               skip_input = .true.
               !write(44,*) 'found begin of C style comment '
               !write(44,1) (CH(ii),ii=1,NR)
               ch(m) = ' '
               ch(m+1) = ' '
             endif
           end if
           if( skip_input ) then
             if( ch(m)=='*'.and.ch(m+1)=='/' ) then
               !write(44,*) 'found end of C style comment '
               skip_input = .false.
               ch(m+1) = ' '
             endif
             ch(m) = ' '
           end if
         end do
         endif
         !write(44,1) (CH(M),M=1,NR)
         DO M=1,NR
            O(J+M-1)=ICHAR(CH(M))
         END DO
         RETURN
         END IF
       KOF=589
      IF (N==33) KOF=100+KOF
      IF (N==23) KOF=0
         DO L=1,NR
            HC(L) = MIN(255,MAX(0, O(J+L-1) ))
            IF (KOF>0) HC(L)=O(HC(L)+KOF)
            CH(L)=CHAR( HC(L)  )
         END DO
      if( i==oflag ) then
        nn6 = nn6+1
        if( nn6==1 ) return
      end if
      if( i==7 ) then
        nn7 = nn7+1
        if( nn7==1 ) return
      end if
      !nrr=nr
      !do jj=nr,1,-1
      !  if( ch(jj)==' ' ) then
      !     nrr=nrr-1
      !  else
      !     GO TO 20
      !  end if
      !end do
      ! GCC versions < 3.4.0 miscompile the above when optimization is on.
      ! --IK, Nov 23 2005.
        jj=nr
        DO WHILE (jj>=1)
           if( ch(jj)/= ' ' ) EXIT
           jj=jj-1
      end do
      nrr = jj

      ! Check for C-preprocessor statements
      ! If the first 6 chars on the line are blank and if the first non-blank
      ! char from coloumn 7 is a #, assume it is a C-preprocessor directive
      ! and make the # to be the first char on the line.
      nfirst=1
         jj=1
         DO WHILE (jj<=nrr)
            if( ch(jj)/= ' ' ) EXIT
            jj=jj+1
      end do
      if( ch(jj)=='#' ) then
        do ii=jj,nrr
          ch(ii+1-jj) = ch(ii)
        end do
        do ii=nrr+2-jj,200
          ch(ii) = ' '
        end do
        nrr = nrr + 1 - jj
        if( tmp_string=='#include' ) then
          do ii=1,nrr
            if( ichar(ch(ii))==39 ) ch(ii) = char(34)
          end do
        end if
      endif

      if(i==7) then
      if( is_format ) then
          jj = 7
          if( .not.is_quote ) then
              DO WHILE (jj<=nrr)
                 if( ch(jj)/= ' ' ) EXIT
                 jj=jj+1
              end do
          end if
          do ii=jj,Nrr
              nfl = nfl + 1
              if( nfl>8192 ) then
                  write(8,*)
                  write(8,*) '*** Mortran error: format buffer is full'
                  write(8,*) '  this may be due to an unclosed quote or'
                  write(8,*) '  bracket in a format statement'
                  write(8,*) '  Format buffer content: '
                  write(8,'(72a1)') (f_buffer(m),m=1,nfl-1)
                  write(8,*)
                  write(8,*) '*** Quiting now'
                  call exit(77) ! stop 77
              end if
              f_buffer(nfl) = ch(ii)
              if( ichar(ch(ii))==39 ) then
                  if( is_quote ) then
                      is_quote = .false.
                  else
                      is_quote = .true.
                  end if
              end if
              if( .not.is_quote ) then
                  if( ch(ii)=='(' ) then
                      nformat = nformat + 1
                  else if( ch(ii)==')') then
                      nformat = nformat - 1
                  end if
              end if
          end do
          if( .not.is_quote ) then
              ii=nfl
              DO WHILE (ii>=1)
                 if( f_buffer(ii)/= ' ' ) EXIT
                 ii=ii-1
              end do
              nfl = ii
          end if
          if( nformat==0 ) then
              is_format = .false.
              is_quote = .false.
              jj = 72
              if( nfl<= 72 ) jj = nfl
              write(i,'(72a1)') (f_buffer(ii),ii=1,jj)
              DO WHILE (jj<nfl)
              ii = jj + 66
              if( ii>nfl ) ii = nfl
              write(i,'(a6,66a1)') '     *',(f_buffer(m),m=jj+1,ii)
              jj = ii
              END DO
              nfl = 0
              return
          end if
          return
      endif

      if( .not.is_format ) then
           jj=7
           DO WHILE (jj<=nrr)
              if( ch(jj)/= ' ') EXIT
              jj=jj+1
          end do
          do ii=jj,jj+5
              itmp = ichar(ch(ii))
              if( itmp>=97.and.itmp<=122 ) itmp=itmp-32
              tmp1_string(ii+1-jj:ii+1-jj) = char(itmp)
          end do
          if( tmp1_string=='FORMAT' ) then
              is_format = .true.
              nfl = Nrr
              do ii=1,Nrr
                  f_buffer(ii) = ch(ii)
              end do
          endif
      endif

      if( is_format ) then
          do jj=7,nrr
              if( ichar(ch(jj))==39 ) then
                  if( is_quote ) then
                       is_quote = .false.
                   else
                       is_quote = .true.
                   endif
              end if
              if( .not.is_quote ) then
                  if( ch(jj)=='(' ) then
                      nformat = nformat + 1
                  else if( ch(jj)==')' ) then
                      nformat = nformat - 1
                  endif
              endif
          end do
          if( nformat==0 ) then
              is_format = .false.
              if( is_quote ) write(8,*) 'Unclosed quote?'
              is_quote = .false.
              nfl = 0
          endif
      end if

      if( is_format ) return
      endif

      write(i,1) (ch(m),m=1,Nrr)
 1 FORMAT(132A1)

!
!	NRC PATCH TO PUT DATE AND TIME AT EACH PAGE THROW
!
      if( CH(1)=='1' .AND. I==6 ) then
          write(I,'(T55,$)') ' '
          call egs_fdate(I)
          write(I,*)
      end if
      RETURN
 5130 O(52)=1
 RETURN
      END SUBROUTINE RW
!_______________________________________________________________________
!________________________________________
      SUBROUTINE INITAL(IND)
      implicit none
      integer  ind
      CHARACTER*1 COUNT
      DIMENSION O(600000)
      COMMON /IBM/O
      integer O
      integer O_B,O_E,O_R,O_S,O_V,O_U,O_W,O_X
      equivalence (O(1),O_B),(O(2),O_E),(O(3),O_R),(O(4),O_S), &
                  (O(5),O_V),(O(6),O_U),(O(7),O_W),(O(8),O_X)
      common /junk/ verbose
      logical verbose
      integer kount,k,iuni
      save    kount,k,iuni,count
      IF (IND==2) THEN
         O(59)=1
         O(19)=5
         KOUNT=O(34)+1
         O(15)=0
         O(9)=5
         WRITE(7,1)KOUNT,(O(K),K=1,KOUNT)
 1    FORMAT(10Z8)
 2    FORMAT(A1)
         if( verbose ) write(6,*) 'OK'
         call exit(0) !stop 0
         END IF
      DO K = 7,600000
         O(K)=0
      END DO
      O(9)=O_B
      O(56)=O_E
      O(57)=O_R
      IUNI=O(9)
!     O(14)=O(4)   THIS IS NO LONGER USED IN FORTRAN 77 VERSIONS
      O(16)=O_U
      READ(IUNI,2)COUNT
      REWIND IUNI
      IF (COUNT=='C') THEN
         CALL RAW()
      ELSE
         READ (IUNI,1) KOUNT,(O(K),K=1,KOUNT)
!
!     The following line
!        CALL RW(23,O(57),325+2,325+79)
!     has been changed to conform with our (NRCC) current version.
!     The effect seems to be that the
!     %%1Copyright ....
!     line at the end of MORNEW77.RAW gets properly handled.
!     Previous to this date, the old version was in our standard
!     distribution.
!     Alex Bielajew 88/09/20
!
         CALL RW(23,O(57),325+3,325+79)
         CALL RW(23,O(56),325+2,325+79)
         REWIND IUNI
         END IF
      O(55)=O(76)
      O(40)=525371
      O(32)=1625
      DO K = 1,6
         O(K)=525371
      END DO
      O(88)=-11
      O(89)=-12
      O(75)=-13
      O(70)=-14
      O(O_U)=O(79)
      O_W=450741-1
      O(O_W)=O(79)
      CALL DUMDUM(44,2105,2155,2205,2255,2305,2355,2455,2505,0,0)
      CALL NXTCRD()
       RETURN
      END SUBROUTINE INITAL
!_______________________________________________________________________
!__________________________________________
      SUBROUTINE RAW
      implicit none
      DIMENSION O(600000)
      COMMON /IBM/O
      integer O
      integer O_B,O_E,O_R,O_S,O_V,O_U,O_W,O_X
      equivalence (O(1),O_B),(O(2),O_E),(O(3),O_R),(O(4),O_S), &
                  (O(5),O_V),(O(6),O_U),(O(7),O_W),(O(8),O_X)
      integer   p
      DIMENSION P(100)
      EQUIVALENCE(O(2555),P(1))
      integer  k,j
      save     k,j
      O(37)=O_V
      O(41)=O_V
      O(38)=324+O(37)
      O(36)=456+O(37)
      O(43)=456+O(41)
      O(42)=324+O(41)
      CALL RW(0,O(9),325,O(38))
      CALL RW(0,O(9),689,689+73)
      CALL RW(0,O(9),325,O(38))
      CALL RW(0,O(9),689+74,789+70)
      DO K = 689,788
         O(K-100)=O(K)
      END DO
!MAKE INPUT AND OUTPUT SAME
! .(MNOSET+54)=.(MNOSET+41);     OUT-TRANSLATE L-ARROW TO =
      CALL RW(23,O(56),325+41,404)
      CALL RW(23,O(57),325+41,404)
      DO K = 1,100
            J=O(589+K-1)
         O(889+J)=K-1
!     WRITE(6,1) J,K-1
      END DO
      O(31)=4
      O(34)=2655
      DO K = 1,10
         O(K+100)=K+100
         O(K+110)=K+110
         O(100-K)=-K
      END DO
!     BLANK BAT BLB TRM LPR RPR LSB RSB LCB RCB
      CALL DUMDUM(76, 90, 79, 78, 75, 38, 46, 74, 82, 83, 84)
      O(86)=77
      O(87)=80
      CALL DUMDUM(29001,29018,29016,29016,29016,29016,29009,0,3,0,0)
      CALL DUMDUM(29009,38,108,46,108,38,108,46,107,24,0)
      CALL DEFINE(29009)
      P(46)=101
      P(38)=202
      P(87)=303
      P(88)=404
      P(89)=505
      P(42)=706
      P(43)=707
      P(44)=808
      P(45)=809
      P( 36)=910
      P( 32)=303
      P( 12)=404
      P( 29)=505
      P( 11)=611
      P( 14)=612
      P( 16)=613
      P( 18)=614
      P( 23)=615
      P( 30)=616
      O(31)=6
       RETURN
      END SUBROUTINE RAW

      subroutine open_units

      implicit none
      DIMENSION O(600000)
      COMMON /IBM/O
      integer O
      integer O_B,O_E,O_R,O_S,O_V,O_U,O_W,O_X
      equivalence (O(1),O_B),(O(2),O_E),(O(3),O_R),(O(4),O_S), &
                  (O(5),O_V),(O(6),O_U),(O(7),O_W),(O(8),O_X)
      common/nrcc/ oflag
      integer      oflag
      common/junk/verbose
      logical verbose
      character*100 arg,next_file
      logical  next,ignore_missing,is_there

      integer iargc,narg,nd,nfs,nfe,nc,no7,no8,i,j,lnblnk1

      narg = iargc()
      verbose = .true.
      ignore_missing = .false.
      if(narg==0) then

        ! no arguments -> assume program is called from a script
        ! making all necessary links and hope for the best

        OPEN(UNIT=1,file='fort.1',STATUS='OLD',err=11)
        GO TO 12
 11   call getarg(0,arg)
        write(6,'(a,a)') arg(1:lnblnk1(arg)),' called without arguments'
        write(6,'(a)') '  but file fort.1 does not exist! '
        write(6,'(a)') '  quiting now.'
        call exit(1) !stop 1
 12   continue
        OPEN(UNIT=5,file='fort.5',STATUS='OLD',ERR=6)
 6   OPEN(UNIT=6,file='fort.6',STATUS='UNKNOWN')
        OPEN(UNIT=7,file='fort.7',STATUS='UNKNOWN')
        OPEN(UNIT=2,file='fort.2',STATUS='OLD',ERR=3)
 3   OPEN(UNIT=3,file='fort.3',STATUS='OLD',ERR=4)
 4   OPEN(UNIT=4,file='fort.4',STATUS='OLD',ERR=8)
 8   OPEN(UNIT=8,file='fort.8',STATUS='OLD',ERR=9)
 9   CONTINUE
        O_B=1   ! hex/raw data unit number
        O_E=6   ! listing unit number
        O_R=7   ! fortran/hex output unit number
        oflag = 6
        if( verbose ) then
          write(6,'(a)') 'mortran3.exe: using standard fort.* units'
          write(6,'(/a,$)') '  Mortran compiling ... '
        end if
!        call getarg(0,arg)
!        write(6,'(a,a)') arg(1:lnblnk1(arg)),
!     &    ': using standard fort.* units'
!        write(6,'(/a,$)') '  Mortran compiling ... '
        return

      end if

      call getarg(1,arg)
      if( arg(1:)=='-help' ) then
        write(6,'(/a/,a/)') &
       'Mortran version 3.2 by Ray F. Cowan with modifications by ', &
       'Dave Rogers, Alex Bielajew and Iwan Kawrakow'
        write(6,'(/a/)') 'Usage: mortran3.exe [ options ]'
        write(6,'(a)') &
       ' If no options are given, mortran3 will attempt to use the ', &
       ' following units (fort.*) files:', &
       '   1:  Read raw or hex mortran data ', &
       '   5:  Read mortran job file ', &
       '   7:  Write hex data (if data creation) or fortran output', &
       '   8:  Write listing (useful for debuging)'
        write(6,'(a)') &
       ' Symbolik links to the raw or hex data and to the mortran job', &
       ' file must therefore have been made prior to invoking the ', &
       ' mortran3 command. It is also useful to link unit 7 and 8 to ', &
       ' the Desired fortran and listing output files. '
        write(6,'(a)') &
       ' Note that the concept of symbolik links does not exist under', &
       ' Windows and so, one has to either copy the input files to ', &
       ' fort.1 and fort.5 or to use mortran3 with the options ', &
       ' described below.'
        write(6,*)
        write(6,'(a)') &
       ' The current version recognizes the following options:', &
       '   -d file_name  use file name as the name of the raw or hex', &
       '                 data file', &
       '   -c file_name  use file name as the name of a configuration', &
       '                 file. This file must contain file names, one', &
       '                 per line, that will be concatenated in the ', &
       '                 order given and the result will be used as a', &
       '                 mortran job input.', &
       '   -f f1 f2 ...  similar as the -c option, but now the files ', &
       '                 to be concatenated are given directly on the ', &
       '                 command line as f1 f2 etc.'
        write(6,'(a)') &
       '   -o7 fname     write mortran3 output to the file fname', &
       '                 this is either the hex data file in the ', &
       '                 case of hex data generation or the fortran ', &
       '                 result of the mortran compilation.', &
       '   -o8 fname     write listing output to the file fname', &
       '   -help         print this message and exit', &
       '   -s            make the pre-processor run silently', &
       '   -i            ignore missing files specified via -c or -f', &
       '                 instead of aborting, as in the case without -i'
        write(6,'(//)')
        call exit(0) !stop 0
      end if
      nd=-1
      nfs=-1
      nfe=-2
      nc=-1
      no7=-1
      no8=-1
      do i=1,narg
        call getarg(i,arg)
        if( arg(1:2)=='-s' ) then
          verbose = .false.
          continue
        end if
        if( arg(1:2)=='-i' ) then
          ignore_missing = .true.
          continue
        end if
        if( arg(1:2)=='-c' ) then
          if( i<narg ) then
            call getarg(i+1,arg)
            if( arg(1:1)/= '-' ) nc=i+1
          end if
        else if( arg(1:2)=='-f' ) then
          nfs=i+1
          do j=i+1,narg
            call getarg(j,arg)
            if( arg(1:1)=='-' ) then
              GO TO 88
            end if
            nfe=j
          end do
 88     continue
        else if( arg(1:2)=='-d' ) then
          if( i<narg ) then
            call getarg(i+1,arg)
            if( arg(1:1)/= '-' ) nd=i+1
          end if
        else if( arg(1:3)=='-o7' ) then
          if( i<narg ) then
            call getarg(i+1,arg)
            if( arg(1:1)/= '-' ) no7=i+1
          end if
        else if( arg(1:3)=='-o8') then
          if( i<narg ) then
            call getarg(i+1,arg)
            if( arg(1:1)/= '-' ) no8=i+1
          end if
        end if
      end do

      !call getarg(0,arg)
      !write(6,'(a,a)') arg(1:lnblnk1(arg)),': using following I/O units'
      if( verbose ) then
      write(6,'(a)') 'mortran3.exe: using following I/O units'
      end if
      if( nd>0 ) then
        call getarg(nd,arg)
        if( verbose ) then
        write(6,'(a,a)') '  raw/hex data file: ',arg(1:lnblnk1(arg))
        end if
        open(1,file=arg(1:lnblnk1(arg)),status='old',err=101)
      else
        write(6,'(a)') '  raw/hex data file must be specified using'
        write(6,'(a,a,a)') '   ',arg(1:lnblnk1(arg)),' -d data_file '
        call exit(2) !stop 2
      end if
      if( (nc>0).or.(nfs<=nfe) ) then
        if(nc>0) then
          open(12,file='mortjob.mortran')
          write(12,'(a)') '%L'
          call getarg(nc,arg)
          if( verbose ) then
          write(6,'(a,a)') '  configuration file: ',arg(1:lnblnk1(arg))
          end if
          open(11,file=arg(1:lnblnk1(arg)),status='old',err=101)
          next = .true.
          do while (next)
            read(11,'(a)',end=102,err=102) next_file
            if( lnblnk1(next_file)>0 ) then
              inquire(file=next_file,exist=is_there)
              if( is_there ) then
                if( verbose ) write(6,'(a,a)') '  -> appending ', &
                              next_file(1:lnblnk1(next_file))
                call add_file(12,next_file)
              else
                if( ignore_missing ) then
                  if( verbose ) write(6,'(a,a,a)') '  file "', &
                    next_file(1:lnblnk1(next_file)), &
                    '" does not exist. Ignoring it.'
                else
                  write(6,'(a,a,a)') '  file "', &
                    next_file(1:lnblnk1(next_file)), &
                    '" does not exist. Quiting now.'
                  call exit(99) !stop 99
                end if
              end if
            end if
            GO TO 104
 102       next = .false.
 104       continue
          end do
          close(11)
          write(12,'(/a)') '%%'
          close(12)
          arg = 'mortjob.mortran'
          open(5,file=arg,status='old',err=101)
        else
          if( nfs==nfe ) then ! a single command line file
            call getarg(nfs,next_file)
            if( verbose ) then
            write(6,'(a,a)') '  command line file: ', &
               next_file(1:lnblnk1(next_file))
            end if
            open(5,file=next_file,status='old',err=101)
          else
            if( verbose ) then
            write(6,'(a,a)') '  command line files: '
            end if
            open(12,file='mortjob.mortran')
            write(12,'(a)') '%L'
            do j=nfs,nfe
              call getarg(j,next_file)
              inquire(file=next_file,exist=is_there)
              if( is_there ) then
                if( verbose ) write(6,'(a,a)') '  -> appending ', &
                              next_file(1:lnblnk1(next_file))
                call add_file(12,next_file)
              else
                if( ignore_missing ) then
                  if( verbose ) write(6,'(a,a,a)') '  file "', &
                    next_file(1:lnblnk1(next_file)), &
                    '" does not exist. Ignoring it.'
                else
                  write(6,'(a,a,a)') '  file "', &
                    next_file(1:lnblnk1(next_file)), &
                    '" does not exist. Quiting now.'
                  call exit(99) !stop 99
                end if
              end if
            end do
            write(12,'(/,a2)') '%%'
            close(12)
            if( verbose ) write(6,'(a)') '  generated mortjob.mortran'
            arg = 'mortjob.mortran'
            open(5,file=arg,status='old',err=101)
          end if
        end if
      end if
      if( no7>0 ) then
        call getarg(no7,arg)
        if( verbose ) write(6,'(a,a)') '  fortran output: ', &
            arg(1:lnblnk1(arg))
        open(7,file=arg(1:lnblnk1(arg)))
      else
        if( verbose ) write(6,'(a)') '  fortran output: fort.7'
        open(7,file='fort.7',status='unknown')
      end if
      if( no8>0 ) then
        call getarg(no8,arg)
        if( verbose )write(6,'(a,a)') '  mortlst output: ', &
            arg(1:lnblnk1(arg))
        open(8,file=arg(1:lnblnk1(arg)),err=101)
      else
        if( verbose )write(6,'(a)') '  mortlst output: fort.8'
        open(8,file='fort.8',status='unknown')
      endif
      O_B=1   ! hex/raw data unit number
      O_E=8   ! listing unit number
      O_R=7   ! fortran/hex output unit number
      if( verbose ) write(6,'(/a,$)') '  Mortran compiling ... '
      oflag = 8
      return

 101 write(6,'(a,a,a)') ' file ',arg(1:lnblnk1(arg)),' does not exist!'
      call exit(3) !stop 3
      end

      subroutine add_file(iu,the_file)
      integer*4  iu
      character*(*) the_file
      logical next_line
      character*250 line
      open(99,file=the_file,status='old',err=100)
      next_line = .true.
      lineno = 0
      do while (next_line)
        read(99,'(a)',err=101,end=101) line
        lineno = lineno + 1
        if( lnblnk1(line)>80 ) then
          write(6,'(a,i5,a,a,a)') &
            '*** Warning: line ',lineno,' in file ', &
           the_file(1:lnblnk1(the_file)),' is longer than 80 chars!'
        end if
        ! remove tabs
        do j=1,lnblnk1(line)
          if( ichar(line(j:j))==9) line(j:j)=' '
        end do
        !write(iu,'(a)') line(1:lnblnk1(line))
        !On AIX the above fails with I/O error if line is blank
        if(lnblnk1(line)>0) then
          write(iu,'(a)') line(1:lnblnk1(line))
        else
          write(iu,*)
        end if
      end do
 101 continue
      close(99)
      return

 100 continue
      write(6,'(a,a)') 'add_file: could not open file ', &
        the_file(1:lnblnk1(the_file))
      call exit(4) !stop 4
      end

      integer*4 function lnblnk1(c)
      character c*(*)
      integer*4 j
      do j=len(c),1,-1
        if( c(j:j)/= ' ' ) then
          lnblnk1 = j
          return
        end if
      end do
      lnblnk1=0
      return
      END FUNCTION lnblnk1
