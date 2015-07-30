
C##############################################################################
C
C  Mortran3 source code
C  Copyright (C) 1993 Stanford Linear Accelerator Center
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
C  Author:          Stanford Linear Accelerator Center
C
C  Contributors:    Ray Cowan
C                   Dave Rogers
C                   Alex Bielajew
C                   Iwan Kawrakow
C
C##############################################################################
C
C  Mortran3 is originally copyright by SLAC and is distributed by NRC under
C  the terms of the AGPL 3.0 licence, in agreement with SLAC. Mortran has a
C  long history  dating back to the 1970s, and certainly involving many
C  contributors. The contributors named above are only those who could be
C  identified from this file's revision history.
C
C
C  Iwan Kawrakow, 2003:
C
C  Changes for EGSnrc Version 4. Make use of command line arguments to open
C  input/output files. After these changes Mortran 3 can be run without first
C  making symbolik links to the necessary files, e.g., mortran3.exe -d
C
C
C  Dave Rogers, 1996:
C
C  Added a few explicit file=fort.n statements to make HP port
C
C  On Sept 21, 1996 the 128K buffer version of mortran3.f (SID 1.8) was
C  replaced with the 300K buffer version which had previously been maintained
C  as big_mortran3.f. Given the fact that memories are so much bigger now, we
C  are just going to maintain the bigger version, now called mortran3.f
C
C  At NRC, to recover the old 128K version: sccs get -r1.8 mortran3.f to see
C  the minor changes to big_mortran3.f made prior to renaming it as mortran3.f,
C  see the residual big_mortran3.f file.
C
C  Note that this version uses the DNTIME routine consistently since this is
C  what the compilers either handle, or the extra routines for the RS6000 and
C  Linux system work with.
C
C
C  Ray F. Cowan, 1993:
C
C  Development version of mortran3.f, the "frozen" egs Fortran implementation
C  of mortran3 for EGS users.
C
C
C  Rev. 2.  Attempt to increase buffer spaces so that mortran3 does not run out
C  -------  of "Rule space".  Mods are to globally change all occurrences of
C           the following constants:
C
C  Name    Old value   New value  Comment
C  ----    ---------   ---------  -------
C  MXU     300000      600000     Max size of buffer O.
C  MNXBF   225739      450741     Minimum (low end) of X-buffer.
C  ?????   225738      450740     Apparently MNXBF-1, found in code.
C  HOME    112369      525371     Location halfway between X-buffer start
C                                 (MNXBF) and top end of O (MXU).
C
C  This fix provided was provided by Thies Schoenborn
C
C  The previous revision increased buffer size to 300k from 128k
C
C  Note that:  MNXBF = MXMBF + 1 + 76, where MXMBF is top end of MBF, the
C  "main" buffer, which starts at MNMBF=2655.  See constants at beginning of
C  code below.  MXMBF = MNMBF + ((MXU-MXPJT)*3/4) - 1, where MXPJT is top end
C  of the PJT buffer, which precedes MBF in O, so MXPJT=2654.  HOME is given
C  by HOME = MNXBF + (MXU - MNXBF)/2.  Apparently MBF is assigned 3/4 of the
C  space remaining in O at the time of its definition, and XBF is assigned the
C  remaining 1/4 of O, and HOME is a pointer halfway through XBF.  In general,
C  buffers have names like PBF, MBF, XBF, etc., and the low (min) and high
C  (max) ends of each buffer are denoted by names like MNMBF and MXMBF.
C
C  WARNING:  none of this is absolutely certain at this time.
C
C
C -----------------------------------------------------------------------------
C MORSOR77 BITNET FRON SLAC APRIL 3 1985  D.W.O.R.
C Copyright Stanford University Mortran 3.1 (FORTRAN 77)
C UNIT(9)_ANO(10)_LVL(11)_CRS(12)_MDEF(13)_CSET(14)_FRT(15)
C LABL(16)_HOLR(17)_INDF(18)_SEQN(19)_NFLG(20)_LIST(21)_MCOM(22)
C NEST(23)_NSCN(24)_PCOM(25)_LAZY(26)_MRKL(27)_STNG(28)_MTRC(29)
C SEQF(30)_DFLT(31)_BASE(32)_BLGR(33)_FREE(34)_MRKF(35)_TMX(36)
C NMX(37)_RMX(38)_INDC(39)_MXX(40)_SIGN(41)_SIGR(42)_SIGT(43)
C ISK(44)_JSK(45)_KSK(46)_LSK(47)_MSK(48)_NSK(49)_USK(50)
C YSK(51)_EOF(52)_INDM(53)_TYP(54)_QFL(55)_MLST(56)_FLST(57)
C IFL(58)_MODE(59)_LOCS(60)_LEFT(61)_MCNT(62)_MERR(63)_KFE(64)
C KFB(65)_KTE(66)_KTB(67)_NGN(68)_SQ(69)_EOL(70)_GEO(71)
C XTR(72)_XTR(73)_XTR(74)_ICUR(75)_BLA(76)_BAT(77)_BLB(78)
C TRM(79)_LPR(80)_RPR(81)_LSB(82)_RSB(83)_LCB(84)_RCB(85)
C BQ(86)_DQ(87)_RTT(88)_LTT(89)_DED(90)_NIL(91)_LNS(92)
C RNS(93)_RIF(94)_LIF(95)_RDQ(96)_LDQ(97)_RMB(98)_LMB(99)
C ZRO(100)_ALT(101)_RNG(102)_LIP(103)_RIP(104)_LQ(105)_RQ(106)
C GA(107)_GP(108)_LGB(109)_RGB(110)_NAT(111)_NLB(112)_XTR(113)
C MF1(114)_MF2(115)_MF3(116)_MF4(117)_MF5(118)_MF6(119)_MF7(120)
C _MF8(121)_MF9(122)_MFA(123)_MFB(124)_MFC(125)_MFD(126)
C NODE4_NSM100_SLOP20_NSX120_BFF132_MXU600000_MNXBF450741
C HOME525371_EVLIM1000_MXERR100_G1157_G0156_G2158_G11167
C G13169_G14170_R1325_R0324_R80404_T1457_T0456
C _T2458_T72528_W1175_W0174_W7181_W72246_W73247_W80254
C SREG1_UREG121_GBF157_WBF175_RBF325_TBF457_ISET589
C OSET689_MORF789_TRT889_0CT1145_1CT1265_2CT1385_3CT1505
C 4CT1625_5CT1745_6CT1865_7CT1985_ISK2105_JSK2155_KSK2205
C _LSK2255_MSK2305_NSK2355_USK2455_YSK2505_PJT2555_MBF2655
C -----------------------------------------------------------------------------
C
C
C##############################################################################


      implicit none
      DIMENSIONO(600000)
      COMMON /IBM/O
      integer O
      integer b,e,r,s,v,u,w,x,c,mxc,m
      integer iw,a,d,mxd,kd,mq,f,t,ip,iq,n,n1,n2,n3,kc,kb,kp,p,q,ia,
     &        ire,nb,kx,y,nerd
      integer ispec,linf
      EQUIVALENCE(O(1),B),(O(2),E),(O(3),R),(O(4),S), (O(5),V),(O(6),U),
     *(O(7),W),(O(8),X),(O(12),C),(O(43),MXC),(O(22),M)
C
      common/nrcc/ oflag
      integer      oflag

      save    iw,a,d,mxd,kd,mq,f,t,ip,iq,n,n1,n2,n3,kc,kb,kp,p,q,ia,
     &        ire,nb,kx,y,nerd

      call open_units

      write(O(2),'(a,T55,$)') 'Mortran 3.2',' '
      call egs_fdate(O(2))
      write(O(2),*)

      O(5)=100
      O(6)=1000
      CALLINITAL(1)
      IW=450741
      X=W
      GOTO10
20    CALLCOUT
      CALLEXPAND(A)
30    IF (S.LE.U) GOTO 50
         IF (O(23).LE.0) GOTO 70
            CALLEXPAND(-1)
            GOTO30
70       CONTINUE
         S=525371
         U=S-1
         GOTO10
50    CONTINUE
      M=O(32)+O(S)
80    IF(O(M).EQ.0)GOTO90
      M=O(M)
      D=M+4
      MXD=O(M+3)
      KD=0
      O(69)=0
      MQ=0
      F=0
      A=0
      T=S
      V=S
      IF(O(29).GE.4) CALLMACTRC(2,D,MXD)
      IF(O(29).GE.4) CALLMACTRC(5,V,U+1)
100   IF(V.GT.U)GOTO101
         IF (O(V).NE.O(97)) GOTO 120
            V=V+O(V+1)+3
            GOTO100
120      CONTINUE
         IF (MQ.NE.0) GOTO 140
            IF (O(D).NE.O(84)) GOTO 160
               IF (O(D+1).GT.100.OR.O(32).LE.1505) GOTO 180
                  O(S-2*(A+1)-1)=V
                  O(S-2*(A+1)-2)=V
                  D=D+1
                  MQ=1
                  IF(O(D-3).NE.O(107))F=0
                  IP=D
                  IQ=D
190               IF(O(IQ).EQ.O(85))GOTO191
                     IQ=IQ+1
                     IF (IQ.LE.MXD) GOTO 210
                        CALLMESAGE(14, 20, 0,0)
                        GOTO80
210                  CONTINUE
                  GOTO 190
191               CONTINUE
                  GOTO100
180            CONTINUE
160         CONTINUE
            IF (O(D).NE.O(V).OR.O(V).GE.O(107)) GOTO 230
               IF( O(105) .EQ.O(D))O(69)=O(69)+1
               IF( O(106) .EQ.O(D))O(69)=O(69)-1
               IF(O(D).EQ.O(99))KD=1
               IF(O(D).EQ.O(98))KD=0
               V=V+1
               T=V
240            D=D+1
               IF(D.GE.MXD)GOTO20
               GOTO100
230         CONTINUE
            IF (O(V).NE.O(76)) GOTO 260
               IF (O(69)+KD.NE.0) GOTO 280
                  V=V+1
                  GOTO100
280            CONTINUE
260         CONTINUE
            IF (O(D).NE.O(107)) GOTO 300
               D=D+1
               N=O(D)
               N1=N/4
               N2=(N-4*N1)/2
               N3=N-4*N1-2*N2
               GOTO310
300         CONTINUE
            IF (O(D).NE.O(108)) GOTO 330
               N1=1
               N2=1
               N3=O(59)
310            T=V
               O(S-2*(A+1)-1)=V
               O(S-2*(A+1)-2)=V
               A=A+1
               D=D+1
               F=D
               KC=0
               KB=0
               KP=0
               GOTO100
330         CONTINUE
            GOTO 130
140         P=IP
            Q=IQ
            IA=V
            IF(O(69).NE.0)IA=0
            IRE=ISPEC(P,Q,V,U,IA)
            IF(IRE.EQ.1)GOTO101
            IF (IRE.NE.3) GOTO 350
               A=A+1
               IF(F.NE.0)O(S-2*(A-1)-2)=T
               IF(IA.NE.0)O(S-2*(A)-1)=IA
               O(S-2*(A)-2)=V
               D=Q
               MQ=0
               GOTO240
350         CONTINUE
130      CONTINUE
         IF (O(V).NE.O(76)) GOTO 370
            IF (MQ.NE.1) GOTO 390
               V=V+1
               GOTO100
390         CONTINUE
370      CONTINUE
         IF(F.EQ.0)GOTO80
400      CONTINUE
            IF(O(T).EQ.O(106))GOTO80
            IF (KD.LE.0) GOTO 420
430            IF( O(99) .EQ.O(T))KD=KD+1
               IF( O(98) .EQ.O(T))KD=KD-1
               IF(KD.EQ.0)GOTO440
               T=T+1
               IF(T.GT.U)GOTO80
               GOTO430
420         CONTINUE
            IF (O(T).NE.O(105)) GOTO 460
               NB=1
470            IF(NB.EQ.0)GOTO471
                  T=T+1
                  IF(T.GT.U)GOTO80
                  IF( O(105) .EQ.O(T))NB=NB+1
                  IF( O(106) .EQ.O(T))NB=NB-1
               GOTO 470
471            CONTINUE
460         CONTINUE
            IF (O(69).NE.0) GOTO 490
               IF( O(84) .EQ.O(T))KC=KC+1
               IF( O(85) .EQ.O(T))KC=KC-1
               IF(KC.LT.0)GOTO80
               IF (N1.NE.1) GOTO 510
                  IF(38.EQ.O(T))KP=KP+1
                  IF(46.EQ.O(T))KP=KP-1
                  IF(KP.LT.0)GOTO80
510            CONTINUE
               IF (N2.NE.1) GOTO 530
                  IF( O(82) .EQ.O(T))KB=KB+1
                  IF( O(83) .EQ.O(T))KB=KB-1
                  IF(KB.LT.0)GOTO80
530            CONTINUE
               IF (O(T).NE.O(79)) GOTO 550
                  IF(N3.GT.0)GOTO80
550            CONTINUE
490         CONTINUE
            T=T+1
            IF (T.LE.U) GOTO 570
               IF(O(15).EQ.1)GOTO80
               GOTO10
570         CONTINUE
            IF (KC+KP+KB.NE.0) GOTO 590
               IF(MQ.EQ.0)D=F
440            V=T
               O(S-2*(A)-2)=V
               GOTO401
590         CONTINUE
         GOTO 400
401      CONTINUE
      GOTO 100
101   CONTINUE
      IF(O(15).EQ.1)GOTO80
      IF (S.NE.U) GOTO 610
         S=525371
         O(S)=O(U)
         U=S
610   CONTINUE
10    O(55)=O(76)
      KX=0
620   CONTINUE
         U=U+1
         IF(U.GT.600000)CALLMESAGE(15, 1, 0,0)
         O(U)=O(C)
         C=C+1
         IF(C.GT.O(43))CALLNXTCRD
         IF (O(U).NE.O(84)) GOTO 640
            O(55)=O(U)
            KX=1
650         IF(KX.EQ.0)GOTO651
               U=U+1
               IF(U.GT.600000)CALLMESAGE(15, 1, 0,0)
               O(U)=O(C)
               C=C+1
               IF(C.GT.O(43))CALLNXTCRD
               IF( O(84) .EQ.O(U))KX=KX+1
               IF( O(85) .EQ.O(U))KX=KX-1
            GOTO 650
651         CONTINUE
            O(55)=O(76)
640      CONTINUE
         IF (O(U).NE.O(87)) GOTO 670
680         IF(O(26).NE.1)O(55)=O(87)
            O(U)=O(97)
            U=U+1
            Y=U
690         IF(O(C).EQ.O(87).OR.C.GT.O(43))GOTO691
               U=U+1
               IF(U.GT.600000)CALLMESAGE(15, 1, 0,0)
               O(U)=O(C)
               C=C+1
            GOTO 690
691         CONTINUE
            U=U+1
            O(U)=O(96)
            O(Y)=U-Y-1
            IF (O(Y).NE.0) GOTO 710
               U=U-2
               O(U)=O(76)
710         CONTINUE
            IF (C.LE.MXC) GOTO 730
               IF (O(26).NE.1) GOTO 750
                  O(55)=O(76)
                  CALLNXTCRD
                  GOTO10
750            CONTINUE
               CALLNXTCRD
               U=U+1
               GOTO680
730         CONTINUE
            O(55)=O(76)
            C=C+1
            IF(C.GT.O(43))CALLNXTCRD
            GOTO10
670      CONTINUE
         IF (O(U).NE.O(86).OR.O(28).LE.0) GOTO 770
            O(55)=O(U)
            O(U)=O(105)
780         CONTINUE
               U=U+1
               IF(U.GT.600000)CALLMESAGE(15, 1, 0,0)
               O(U)=O(C)
               C=C+1
               IF(C.GT.O(43))CALLNXTCRD
               IF (O(U).NE.O(86)) GOTO 800
                  IF (O(C).EQ.O(86)) GOTO 820
                     O(U)=O(106)
                     O(55)=O(76)
                     GOTO781
820               CONTINUE
                  C=C+1
                  IF(C.GT.O(43))CALLNXTCRD
800            CONTINUE
            GOTO 780
781         CONTINUE
770      CONTINUE
         IF (O(U).NE.O(76)) GOTO 840
            IF (S.EQ.U) GOTO 860
               IF(O(U-1).EQ.O(76))U=U-1
860         CONTINUE
870         CONTINUE
               IF(C.EQ.MXC .OR.O(C).NE.O(76))GOTO871
               C=C+1
            GOTO 870
871         CONTINUE
840      CONTINUE
         IF (KX.NE.0) GOTO 890
            IF(O(U).EQ.O(79))GOTO621
            IF( O(82) .EQ.O(U))O(11)=O(11)+1
            IF( O(83) .EQ.O(U))O(11)=O(11)-1
890      CONTINUE
      GOTO 620
621   CONTINUE
      GOTO30
90    IF (O(23).LE.0) GOTO 910
         X=X+1
         IF(X.GT.O(40))CALLMESAGE(15, 3, 0,0)
         O(X)=O(S)
         S=S+1
         IF (O(X).NE.O(92).OR.O(32).EQ.1265) GOTO 930
940         CONTINUE
               X=X+1
               IF(X.GT.O(40))CALLMESAGE(15, 3, 0,0)
               O(X)=O(S)
               S=S+1
               IF(O(X).EQ.O(93))GOTO941
            GOTO 940
941         CONTINUE
930      CONTINUE
         GOTO30
910   CONTINUE
950   CONTINUE
         IF (O(S).NE.O(79)) GOTO 970
            IF (W.GE.IW) W=LINF (IW,W,15)
            GOTO 960
970         IF  (O(S).NE.O(97))GOTO980
            IF (O(68).NE.0) GOTO 1000
               NERD=LINF(S+2,S+O(S+1)+1,12)
               S=S+O(S+1)+2
1000        CONTINUE
            GOTO 960
980         IF  (O(S).NE.O(92))GOTO1010
            O(24)=1
            GOTO 960
1010        IF  (O(S).NE.O(93))GOTO1020
            O(24)=0
            GOTO 960
1020        IF  (O(68).NE.0)GOTO1030
            W=W+1
            O(W)=O(S)
            IF (O(W).NE.O(105)) GOTO 1050
1060           IF(O(W).EQ.O(106))GOTO1061
                  W=W+1
                  S=S+1
                  O(W)=O(S)
               GOTO 1060
1061           CONTINUE
1050        CONTINUE
960      CONTINUE
1030     CONTINUE
         S=S+1
         IF(S.GT.U .OR. O(24).EQ.0)GOTO951
      GOTO 950
951   CONTINUE
      GOTO30
      END
C_______________________________________________________________________
C__________________________________________
      integer FUNCTION ISPEC(P,Q,IV,U,T)
      implicit none
      integer p,q,iv,u,t
      DIMENSIONO(600000)
      COMMON /IBM/O
      integer O
      integer mini
      integer c,m,b,v,iq,i,np,jmp,n,nul,w
      save    c,m,b,v,iq,i,np,jmp,n,nul,w
      C=0
      M=0
      B=0
      V=IV
      O(45)=2155
CADEFGHIJKLRSXYZ
      O(46)=2205
1070  IF(P.GE.Q)GOTO1071
         IF (O(V).NE.O(76)) GOTO 1090
            IF (V.NE.T) GOTO 1110
               V=V+1
               T=T+1
               IF(V.GT.U)GOTO1120
               GOTO1070
1110        CONTINUE
1090     CONTINUE
         IF (O(P).GT.100) GOTO 1140
            IQ=Q
            I=MINI(P,Q,NP,1505)
            IF(O(29).GE.4) CALLMACTRC(5,V,U)
            IF(I.EQ.0)GOTO90
            O(45)=O(45)+1
            O(O(45))=IQ
            O(45)=O(45)+1
            O(O(45))=NP
            IF(O(45).GT.2204)CALLMESAGE(15, 19, 0,0)
            GOTO 1130
1140        IF  (O(P).GE.O(106))GOTO1150
            P=P+1
            JMP=O(P-1)-100
            GOTO(1160,1170,1180,1190,1200),JMP
1160        IF (M.NE.0) GOTO 1220
               V=O(O(46)-1)
               GOTO 1210
1220           N=1
1230           IF(N.EQ.0)GOTO1231
                  IF( O(103) .EQ.O(P))N=N+1
                  IF( O(104) .EQ.O(P))N=N-1
                  P=P+1
                  IF(P.GT.Q)CALLMESAGE(14, 27, 0,0)
               GOTO 1230
1231           CONTINUE
               P=P-1
1210        CONTINUE
            GOTO1070
1180        O(46)=O(46)+1
            O(O(46))=C
            O(46)=O(46)+1
            O(O(46))=V
            O(46)=O(46)+1
            O(O(46))=P
            IF(O(46).GT.2254)CALLMESAGE(15, 20, 0,0)
            C=0
            GOTO1070
1190        P=P+2
            IF (M.NE.1) GOTO 1250
               C=C+1
               IF (C.GE.O(P-1)) GOTO 1270
                  P=O(O(46))
                  GOTO1070
1270           CONTINUE
1250        CONTINUE
            IF(C.GE.O(P-2))M=1
            IF( M.EQ.0 ) V=O(O(46)-1)
            IF (.NOT.((O(46).EQ.2205))) GOTO 1290
               CALLMESAGE(14, 17, 0,0)
               GOTO90
1290        CONTINUE
            NUL=O(O(46))
            O(46)=O(46)-1
            NUL=O(O(46))
            O(46)=O(46)-1
            C=O(O(46))
            O(46)=O(46)-1
            GOTO1300
1170        P=P+2
            M=0
            IF (O(P-2).GT.O(V).OR.O(V).GT.O(P-1)) GOTO 1320
               M=1
               V=V+1
               IF(V.GT.U)GOTO1120
1320        CONTINUE
            GOTO1300
1200        W=V
            M=1
            IF (O(P).NE.O(76)) GOTO 1340
1350           IF(O(W).NE.O(76).OR.O(P).NE.O(76))GOTO1351
                  W=W-1
                  P=P+1
               GOTO 1350
1351           CONTINUE
1340        CONTINUE
1360        IF(O(P).EQ.O(106))GOTO1361
               IF(O(P).NE.O(W).OR.W.GT.U)M=0
               P=P+1
               W=W+1
            GOTO 1360
1361        CONTINUE
            P=P+1
            IF(M.EQ.1)V=W
1300        IF(M.EQ.1)GOTO1070
            IF(O(P).NE.O(101) .AND. O(P).NE.O(104))GOTO90
            GOTO 1130
1150        CALLMESAGE(14, 19, P,Q+11)
            GOTO90
1130     CONTINUE
      GOTO 1070
1071  CONTINUE
      IF (.NOT.(.NOT.(O(45).EQ.2155))) GOTO 1380
         P=O(O(45))
         O(45)=O(45)-1
         Q=O(O(45))
         O(45)=O(45)-1
         GOTO1070
1380  CONTINUE
      IV=V
      ISPEC=3
      GOTO 99999
90    ISPEC=2
      GOTO 99999
1120  ISPEC=1
99999 RETURN
      END
C_______________________________________________________________________
C__________________________________________
      integer FUNCTION MINI(S,U,W,KCT)
      implicit none
      integer s,u,w,kct
      DIMENSIONO(600000)
      COMMON /IBM/O
      integer O
      integer m,l,v,p,q
      save    m,l,v,p,q
      MINI=0
      M=KCT+O(S)
CABCDEFGHIJKNRTXYZ
      L=U-S
1390  IF (O(M).EQ.0) GOTO 1410
         M=O(M)
         V=S
         P=M+4
         Q=O(M+3)
C   IF Q-PnL jM;
         IF(O(29).GE.6) CALLMACTRC(2,P,Q)
1420     IF(P.GE.Q)GOTO1421
            IF(O(P).NE.O(V))GOTO1390
            P=P+1
            V=V+1
         GOTO 1420
1421     CONTINUE
         S=O(M+2)+4
         U=O(O(M+2)+3)
         MINI=1
         W=V
         GOTO 99999
1410  CONTINUE
      CALLMESAGE(14, 15, S,U+1)
99999 RETURN
      END
C_______________________________________________________________________
C________________________________________
      SUBROUTINE EXPAND (F)
      implicit none
      integer  f
      DIMENSIONO(600000)
      COMMON /IBM/O
      integer O
      integer b,e,r,s,v,u,w,x
      EQUIVALENCE(O(1),B),(O(2),E),(O(3),R),(O(4),S), (O(5),V),(O(6),U),
     *(O(7),W),(O(8),X)
      integer lexp,llong
      integer kgn,t,sef,k,j,jv,p,q,nul,jmp,a,n,krap,kraq,z,y
      DATA KGN/0/
      save    kgn,t,sef,k,j,jv,p,q,nul,jmp,a,n,krap,kraq,z,y
CCDGHILM
      T=0
      IF (F.GE.0) GOTO 1440
         O(23)=O(23)-1
         IF(O(23).EQ.0)O(15)=SEF
         DO 1451 K = 1,6
            O(K)=O(6+U+1-K)
1451     CONTINUE
         J=O(O(O(51)))
         IF (11.GT.J.OR.J.GT.12) GOTO 1470
            JV=LEXP(W+1,X)
            IF (O(58).NE.0) GOTO 1490
               IF (J.NE.12) GOTO 1510
                  X=W+LLONG(W,JV,10,0)
1510           CONTINUE
               IF (J.NE.11) GOTO 1530
                  X=W+1
                  O(X)=JV
1530           CONTINUE
               GOTO 1480
1490           X=W
               O(58)=O(58)-1
               IF (JV.NE.0) GOTO 1550
                  P=O(65)
                  Q=O(64)
                  GOTO 1540
1550              P=O(67)
                  Q=O(66)
1540           CONTINUE
               O(44)=O(44)+1
               O(O(44))=O(64)+1
               O(44)=O(44)+1
               O(O(44))=E
               IF(O(44).GT.2154)CALLMESAGE(15, 18, 0,0)
               B=P
               E=Q
1480        CONTINUE
1470     CONTINUE
         NUL=O(O(51))
         O(51)=O(51)-1
         W=O(O(51))
         O(51)=O(51)-1
         O(32)=O(O(51))
         O(51)=O(51)-1
         GOTO 1430
1440     O(S-1)=F
         O(40) = S-2*F-3
         R=O(O(22)+2)
         B=R+4
         E=O(R+3)
         R=O(R)
         O(62)=O(62)+1
         IF (O(62).LE.1000) GOTO 1570
            CALLMESAGE(14, 4, 0,0)
            O(29)=2
1570     CONTINUE
         IF(O(29).GE.2) CALLMACTRC(22,O(22),S)
         IF( O(23).EQ.0 )X=W
1430  CONTINUE
1580  CONTINUE
1590     IF(B.GE.E)GOTO1591
            IF (O(B).GE.O(107)) GOTO 1610
               X=X+1
               IF(X.GT.O(40))CALLMESAGE(15, 3, 0,0)
               O(X)=O(B)
               B=B+1
               GOTO1590
1610        CONTINUE
            B=B+1
            JMP=O(B-1)-O(106)
            GOTO(1620,1630,1640,1650),JMP
1630        IF (1.GT.O(B).OR.O(B).GT.O(S-1)) GOTO 1670
               A=O(S-2*(O(B))-1)
               N=O(S-2*(O(B))-2)
1680           IF(A.GE.N)GOTO1681
                  X=X+1
                  IF(X.GT.O(40))CALLMESAGE(15, 3, 0,0)
                  O(X)=O(A)
                  A=A+1
               GOTO 1680
1681           CONTINUE
               GOTO1690
1670        CONTINUE
            CALLMESAGE(15, 4, B,E)
1620        IF (O(B).NE.12) GOTO 1710
               B=B+1
               IF (O(B).NE.23) GOTO 1730
                  IF (O(68).NE.0) GOTO 1750
                     KGN=1
                     O(68)=1
                     GOTO 1740
1750                 KGN=KGN+1
1740              CONTINUE
                  GOTO 1720
1730              IF  (O(B).NE.16)GOTO1760
                  IF (O(68).NE.1) GOTO 1780
                     KGN=KGN+1
1780              CONTINUE
                  GOTO 1720
1760              IF  (O(B).NE.14)GOTO1790
                  KGN=KGN-1
                  IF(KGN.LE.0)O(68)=0
1720           CONTINUE
1790           CONTINUE
               IF (O(71).EQ.0) GOTO 1810
                  IF(O(68).EQ.0)O(32)=1625
                  IF(O(68).NE.0)O(32)=1145
1810           CONTINUE
               GOTO1690
1710        CONTINUE
            IF(O(68).EQ.0)CALLKAT
            GOTO1690
1640        IF (O(B).NE.10) GOTO 1830
               B=B+1
               IF(1.GT.O(B).OR.O(B).GT.O(S-1) )CALLMESAGE(15, 4, B,E)
               X=X+LLONG(X,O(S-2*(O(B))-2)-O(S-2*(O(B))-1),10,0)
               IF(X.GT.O(40))CALLMESAGE(15, 3, 0,0)
               B=B+2
               GOTO1590
1830        CONTINUE
            IF (O(B).NE.13) GOTO 1850
               B=B+1
               T=1
1860           CONTINUE
                  IF( O(109) .EQ.O(B))T=T+1
                  IF( O(110) .EQ.O(B))T=T-1
                  IF(T.EQ.0)GOTO1861
                  X=X+1
                  IF(X.GT.O(40))CALLMESAGE(15, 3, 0,0)
                  O(X)=O(B)
                  B=B+1
               GOTO 1860
1861           CONTINUE
               GOTO1690
1850        CONTINUE
            IF (O(B).NE.15) GOTO 1880
               X=X+1
               O(X)=O(97)
               X=X+1
               KRAP=X
               KRAQ=O(23)
               GOTO1690
1880        CONTINUE
            O(23)=O(23)+1
            IF(O(23).EQ.1)SEF=O(15)
            O(15)=1
            O(51)=O(51)+1
            O(O(51))=O(32)
            O(51)=O(51)+1
            O(O(51))=W
            O(51)=O(51)+1
            O(O(51))=B
            IF(O(51).GT.2554)CALLMESAGE(15, 34, 0,0)
            W=X
            GOTO1690
1650        IF (KRAP.EQ.0.OR.KRAQ.NE.O(23)) GOTO 1900
               X=X+1
               O(S)=O(96)
               O(KRAP)=X-KRAP-1
               KRAP=0
               GOTO1590
1900        CONTINUE
            DO 1911 K = 1,6
               O(O(40)-K)=O(K)
1911        CONTINUE
            U=O(40)-7
            V=U+1
            Z=O(O(51))
            Y=O(Z)
            IF(O(Z+2).EQ.O(105))Y=0
            IF((9.LT.Y).AND.(Y.LT.15))Y=O(31)
            IF( 0.GT.Y.OR.Y.GE.8 )CALLMESAGE(15, 17, B,E)
            O(32)=Y*120+1145
            GOTO1591
1690        B=B+1
         GOTO 1590
1591     CONTINUE
         IF(O(23).GT.0)GOTO1581
         IF (.NOT.((O(44).EQ.2105))) GOTO 1930
            IF(R.LE.0)GOTO1581
            B=R+4
            E=O(R+3)
            R=O(R)
            GOTO 1920
1930        E=O(O(44))
            O(44)=O(44)-1
            B=O(O(44))
            O(44)=O(44)-1
1920     CONTINUE
      GOTO 1580
1581  CONTINUE
      IF(O(29).GE.2) CALLMACTRC(33,W,X)
1940  IF(X.LE.W)GOTO1941
         V=V-1
         O(V)=O(X)
         X=X-1
      GOTO 1940
1941  CONTINUE
      S=V
99999 RETURN
      END
CTO EXPAND
C_______________________________________________________________________
C________________________________________
      SUBROUTINE KAT
      implicit none
      DIMENSIONO(600000)
      COMMON /IBM/O
      integer O
      integer b,e,r,s,v,u,w,x,l,m
      EQUIVALENCE(O(1),B),(O(2),E),(O(3),R),(O(4),S), (O(5),V),(O(6),U),
     *(O(7),W),(O(8),X)
      EQUIVALENCE (O(47),L),(O(48),M)
      integer llong,nib
      integer jmp,t,ibg,z,iuni,nba
      save    jmp,t,ibg,z,iuni,nba
CFGHIJKLPQ  YZ
      IF(O(B).LT.19.OR.O(B).GT.30) CALLMESAGE(15, 7, B-2,E)
      B=B+1
      JMP=O(B-1)-18
      GOTO(1950,1960,1970,1390,1980,1990,2000,2010,2020,2030,2040,2050),
     *JMP
1950  T=O(B)+78
      X=X+1
      O(X)=O(T)
      GOTO 99999
1960  CALLKRUNC(0)
      GOTO 99999
1970  IF (O(B).NE.12) GOTO 2070
         X=X+LLONG(X,O(L-O(B+1))+O(B+2),10,1)
         B=B+2
         GOTO 99999
2070  CONTINUE
      IF (O(B).NE.16) GOTO 2090
         O(16)=O(16)+10
         X=X+LLONG(X,O(16),10,0)
         GOTO 99999
2090  CONTINUE
      IF (O(B).NE.28) GOTO 2110
         B=B+1
         O(47)=O(47)+1
         O(O(47))=NIB(X,10)
         IF(O(47).GT.2304)CALLMESAGE(15, 21, 0,0)
         GOTO2120
2110  CONTINUE
      IF (O(B).NE.30) GOTO 2140
         B=B+1
         IF(O(B).EQ.1)O(L-1)=O(L)
         L=L-1
         IF (L.GE.2255) GOTO 2160
            CALLMESAGE(14, 7, 0,0)
            L=L+1
2160     CONTINUE
         GOTO1300
2140  CONTINUE
      CALLMESAGE(15, 7, B-2,E)
1390  IF (O(B).NE.28) GOTO 2180
         B=B+1
         O(48)=O(48)+1
         O(O(48))=O(B)
         IF(O(48).GT.2354)CALLMESAGE(15, 22, 0,0)
         GOTO 99999
2180  CONTINUE
      IF (O(B).NE.27) GOTO 2200
         B=B+1
         O(M)=O(B)
         GOTO 99999
2200  CONTINUE
      IF (O(B).NE.30) GOTO 2220
         X=X+1
         O(X)=O(M)
         M=M-1
         IF (M.GE.2305) GOTO 2240
            CALLMESAGE(14, 8, 0,0)
            M=M+1
2240     CONTINUE
         GOTO 99999
2220  CONTINUE
      CALLMESAGE(15, 7, B-2,E)
1980  IBG=O(X)
      X=X-1+LLONG(X-1,IBG,10,1)
      GOTO 99999
1990  CALLDEFINE(S)
      GOTO 99999
2000  O(58)=1
      B=B+4
      O(67)=B+O(B-1)
      O(66)=B+O(B-2)
      O(65)=B+O(B-3)
      O(64)=B+O(B-4)
      B=B-1
      GOTO 99999
2010  O(16)=O(16)+10
      O(47)=O(47)+1
      O(O(47))=O(16)
      IF(O(47).GT.2304)CALLMESAGE(15, 21, 0,0)
2120  IF (O(B).NE.1) GOTO 2260
         Z=O(L-1)
         O(L-1)=O(L)
         O(L)=Z
2260  CONTINUE
1300  IF(O(B).NE.1.AND.O(B).NE.0) then
        !write(6,*) ' Mortran compilation error (stop 18)'
        write(6,*) 'compilation error (stop 18)'
        call exit(18) !stop 18
      endif
      GOTO 99999
2020  IUNI=O(B)
      REWIND IUNI
      GOTO 99999
2040  IF(O(29).GE.1) CALLMACTRC(22,O(22),S)
      B=B-1
      GOTO 99999
2030  T=O(B)
      B=B+1
      IF (0.GE.O(B).OR.O(B).GE.5) GOTO 2280
         T=T+26*O(B)
         B=B+1
2280  CONTINUE
      NBA=0
      GOTO2290
2050  T=121+O(B)
      B=B+1
      NBA=O(X)
      X=X-1
2290  IF (O(B).NE.43) GOTO 2310
         O(T)=O(T)+1
         GOTO 99999
2310  CONTINUE
      IF (O(B).NE.42) GOTO 2330
         O(T)=O(T)-1
         GOTO 99999
2330  CONTINUE
      IF (O(B).NE.41) GOTO 2350
         O(T)=O(X)
         X=X-1
         GOTO 99999
2350  CONTINUE
      IF (O(B).NE.12) GOTO 2370
         X=X+1
         O(X)=O(T)
         IF (NBA.GT.1) X=X-1+LLONG(X-1,O(T),NBA,1)
         GOTO 99999
2370  CONTINUE
      CALLMESAGE(15, 7, B-3,E)
99999 RETURN
      END
C_______________________________________________________________________
C__________________________________________
      SUBROUTINE COUT
      implicit none
      DIMENSIONO(600000)
      COMMON /IBM/O
      integer O
      integer  b,e,r,s,v,u,w,x
      EQUIVALENCE(O(1),B),(O(2),E),(O(3),R),(O(4),S), (O(5),V),(O(6),U),
     *(O(7),W),(O(8),X)
      integer linf
      integer t,nerd
      save    t,nerd
      T=S
2380  IF(T.GT.V)GOTO2381
         IF (O(T).NE.O(97)) GOTO 2400
            NERD=LINF(T+2,T+O(T+1)+1,12)
            T=T+O(T+1)+2
2400     CONTINUE
         T=T+1
      GOTO 2380
2381  CONTINUE
      RETURN
      END
C_______________________________________________________________________
C________________________________________
      SUBROUTINE DEFINE(S)
      implicit none
      integer s
      DIMENSIONO(600000)
      COMMON /IBM/O
      integer O
CGHQUVWXYZ
      integer   l,k,a,t,n,z,b,e,m,p,lp,c,need,j,f,i,r
      save      l,k,a,t,n,z,b,e,m,p,lp,c,need,j,f,i,r
      DIMENSION L(3)
      IF (O(S-2*(1)-1).NE.O(S-2*(1)-2)) GOTO 2420
         CALLMESAGE(14, 1, 0,0)
         GOTO 99999
2420  CONTINUE
      O(54)=O(31)
      IF ( O(S-2*(2)-1).NE.O(S-2*(2)-2) )O(54)=O(O(S-2*(2)-1))
      DO 2431 K = 1,3
         A=O(S-2*(K)-1)
         T=O(S-2*(K)-2)
         N=A-1
         Z=0
2440     IF(A.GT.T)GOTO2441
            N=N+1
            O(N)=O(A)
            A=A+1
            IF (O(N).NE.O(86)) GOTO 2460
               IF (K.NE.3.AND.O(28).LE.0) GOTO 2480
                  IF (Z.NE.0) GOTO 2500
                     O(N)=O(105)
                     Z=1
                     GOTO 2490
2500                 IF (O(A).NE.O(86).OR.A.GT.T) GOTO 2520
                        A=A+1
                        GOTO 2510
2520                    O(N)=O(106)
                        Z=0
2510                 CONTINUE
2490              CONTINUE
2480           CONTINUE
               GOTO 2450
2460           IF  (O(N).NE.O(76))GOTO2530
               IF (Z.NE.0) GOTO 2550
2560              IF(O(A).NE.O(76).OR.A.GE.T)GOTO2561
                     A=A+1
                  GOTO 2560
2561              CONTINUE
2550           CONTINUE
               GOTO 2450
2530           IF  (O(N).NE.O(78))GOTO2570
               IF (O(A).NE.O(78)) GOTO 2590
                  A=A+1
                  GOTO 2580
2590              O(N)=O(108)
2580           CONTINUE
               GOTO 2450
2570           IF  (O(N).NE.O(77))GOTO2600
               IF (O(A).NE.O(77)) GOTO 2620
                  A=A+1
                  GOTO 2610
2620              O(N)=O(107)
2610           CONTINUE
2450        CONTINUE
2600        CONTINUE
            IF (O(33).NE.0) GOTO 2640
               IF (K.NE.3) GOTO 2660
                  IF(O(N).EQ.O(84))O(N)=O(109)
                  IF(O(N).EQ.O(85))O(N)=O(110)
2660           CONTINUE
2640        CONTINUE
         GOTO 2440
2441     CONTINUE
         O(S-2*(K)-2)=N
         L(K)=O(S-2*(K)-2)-O(S-2*(K)-1)
2431  CONTINUE
      IF (12.GT.O(54).OR.O(54).GT.14) GOTO 2680
         B=O(S-2*(1)-1)
         E=O(S-2*(1)-2)
         IF (O(54).NE.14) GOTO 2700
            M=1505+O(B)
            GOTO 2690
2700        M=1625+O(B)
2690     CONTINUE
2710     IF (O(M).NE.0) GOTO 2730
            IF (O(54).NE.12) GOTO 2750
               O(54)=O(31)
               GOTO2760
2750        CONTINUE
            CALLMESAGE(14, 2, 0,0)
            GOTO 99999
2730     CONTINUE
         M=O(M)
         P = M+4
         LP=O(M+3)-P
         IF(LP.NE.L(1))GOTO2710
         A=B
2770     IF(O(P).NE.O(A))GOTO2771
            P=P+1
            A=A+1
            IF(A.GE.E)GOTO2780
         GOTO 2770
2771     CONTINUE
C   TRACE9;
C      D-LINK PAT         CAT PTR   MARK PAT
         GOTO2710
2780     IF (O(54).EQ.12) GOTO 2800
            IF(O(M).NE.0)O((O(M))+1)=O(M+1)
            O(O(M+1))=O(M)
            C=O(M+2)
            O(M+2)=O(90)
2810        IF(C.EQ.0)GOTO2811
               O(C+2)=O(90)
               C=O(C)
            GOTO 2810
2811        CONTINUE
            IF(O(13).GT.0)CALLMACTRC(13,0,S)
            GOTO 99999
2800     CONTINUE
         NEED= 4+L(3)
         IF(NEED.GT. 450740-O(34)) CALLKRUNC(NEED)
         C=O(M+2)
2820     IF(O(C).EQ.0)GOTO2821
            C=O(C)
         GOTO 2820
2821     CONTINUE
         O(C)=O(34)
         O(O(34))=0
         O(O(34)+1)=C
         O(O(34)+2)=O(91)
         O(O(34)+3)=O(34)+NEED
         O(34)=O(34)+NEED
         A=O(S-2*(3)-2)
         K=L(3)
         DO 2831 J = 1,K
            O(O(34)-J)=O(A-J)
2831     CONTINUE
         IF(O(13).GT.0)CALLMACTRC(12,0,S)
         GOTO 99999
2680  CONTINUE
2760  NEED=L(1)+L(3)+2*4
      IF (NEED.GT.450740-O(34)) CALLKRUNC(2+NEED)
      A=O(S-2*(1)-1)
      Z=A+L(1)-1
      P=O(34)
      F=P+4
      I=F
2840  IF(A.GT.Z)GOTO2841
         O(F)=O(A)
         F=F+1
         A=A+1
      GOTO 2840
2841  CONTINUE
      IF (O(I).NE.O(84).OR.O(54).LE.2) GOTO 2860
         CALLMESAGE(14, 21, O(S-2*(1)-1),O(S-2*(1)-2))
         GOTO 99999
2860  CONTINUE
      IF (O(I).LE.O(105)) GOTO 2880
         CALLMESAGE(14, 9, O(S-2*(1)-1),O(S-2*(1)-2))
         GOTO 99999
2880  CONTINUE
      IF ((0.LE.O(54)).AND.(O(54).LE.7)) GOTO 2900
         CALLMESAGE(14, 12, O(S-2*(1)-1),O(S-2*(1)-2))
         GOTO 99999
2900  CONTINUE
      I=O(I) + 1145 + O(54)*120
      O(P)=O(I)
      IF(O(P).NE.0)O((O(P))+1)=P
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
      IF (K.EQ.0) GOTO 2920
         CALLDOIT(E-K,E)
         DO 2931 J = 1,K
            O(F-J)=O(E-J)
2931     CONTINUE
2920  CONTINUE
      IF (O(13).GT.0) CALLMACTRC(15,P,R)
      O(34)= P +NEED
      O(60)=450740-O(34)
      O(61)=(100*O(60))/(450740-2655)
99999 RETURN
      END
C_______________________________________________________________________
C__________________________________________
      SUBROUTINEDOIT(A,Z)
      implicit none
      integer a,z
      DIMENSIONO(600000)
      COMMON /IBM/O
      integer O
      integer   s,x,t,p
      DIMENSION S(200)
      save      s,x,t,p
      X=0
CBCDEFGHIJKLMNQRUVWY
      T=Z
2940  IF(T.LT.A)GOTO2941
         IF (O(T).NE.O(95)) GOTO 2960
            IF (O(T-1).NE.O(94)) GOTO 2980
               S(X+1)=T+1
               S(X+2)=O(T)
               X=X+2
               T=T-1
               GOTO 2970
2980           S(X+1)= T+1
               X=X+1
2990           IF(O(T).EQ.O(107).AND.O(T+1).EQ.25)GOTO2991
                  T=T-1
                  IF(T.LT.A)GOTO 99999
               GOTO 2990
2991           CONTINUE
               P=T+6
               O(P-1)= S(X)-P
               X=X-1
               IF (S(X).NE.O(95)) GOTO 3010
                  O(P-4)= S(X-2)-P
                  O(P-3)= S(X-1)-P
                  O(P-2)= O(P-3)-2
                  X=X-3
                  GOTO 3000
3010              O(P-2)=S(X)-P
                  O(P-3)=S(X)-P
                  O(P-4)=S(X)-P
                  X=X-1
3000           CONTINUE
2970        CONTINUE
            GOTO 2950
2960        IF  (O(T).NE.O(94))GOTO3020
            S(X+1)= T
            X=X+1
2950     CONTINUE
3020     CONTINUE
         T=T-1
      GOTO 2940
2941  CONTINUE
99999 RETURN
      END
C_______________________________________________________________________
C__________________________________________
      SUBROUTINE KRUNC(R)
      implicit none
      integer    r
      DIMENSIONO(600000)
CABEFGHIJKLQTUVWXYZ
      COMMON /IBM/O
      integer O
      integer m,n,d,p,s,c,i
      save    m,n,d,p,s,c,i
      IF (O(35).EQ.0) GOTO 3040
         M=O(35)
3050     IF(M.EQ.O(34))GOTO3051
            IF (O(M+2).LT.0) GOTO 3070
               IF(O(M).NE.0)O((O(M))+1)=O(M+1)
               O(O(M+1))=O(M)
3070        CONTINUE
            O(M+2)=O(90)
            M=O(M+3)
         GOTO 3050
3051     CONTINUE
3040  CONTINUE
      N=2655
      M=O(N+3)
      D=0
C PREDECESSOR  SUCCESSOR    CONCAT
      O(O(34))=0
3080  IF(N.EQ.O(34))GOTO3081
         M=N
         N= O(M+3)
         P=O(M+1)
         S=O(M)
         C=O(M+2)
         IF (C.NE.O(90)) GOTO 3100
            D=D+ N-M
            GOTO3080
3100     CONTINUE
         IF (S.GT.0) O(S+1)=O(S+1)-D
         IF (C.GT.0) O(N+1)=O(N+1)-D
         O(P)=O(P)-D
         O(M+3)=O(M+3)-D
C   MOVE IT
         DO 3111 I = M,N
            O(I-D)=O(I)
3111     CONTINUE
      GOTO 3080
3081  CONTINUE
      O(34)=O(34)-D
      O(O(34))=0
      IF (R.GT.D)CALLMESAGE(15, 6, 0,0)
      O(60)=450740-O(34)
      O(61)=(100*O(60))/(450740-2655)
      CALLMESAGE(13,1,D,0)
99999 RETURN
      END
C_______________________________________________________________________
C__________________________________________
      integer FUNCTION NIB(L,B)
      implicit none
      integer  l,b
      DIMENSIONO(600000)
      COMMON /IBM/O
      integer O
      integer m,n
      save    m,n
      M=1
      N=0
3120  IF(O(L).NE.O(76))GOTO3121
         L=L-1
      GOTO 3120
3121  CONTINUE
3130  IF(0.GT.O(L).OR.O(L).GE.B)GOTO3131
         N=N+O(L)*M
         M=M*B
         L=L-1
      GOTO 3130
3131  CONTINUE
      IF (O(L).NE.42) GOTO 3150
         L=L-1
         N=-N
3150  CONTINUE
      NIB=N
99999 RETURN
      END
C_______________________________________________________________________
C__________________________________________
      integer FUNCTION LLONG(LOC,NUM,B,IND)
      implicit none
      integer  loc,num,b,ind
      DIMENSIONO(600000)
      COMMON /IBM/O
      integer O
      integer n,l,i,j,k
      save    n,l,i,j,k
      IF(O(68).NE.0)GOTO 99999
      N=IABS(NUM)
      L=LOC
      I=N/B
      J=1
      IF (IND.NE.0) GOTO 3170
         L=L+1
         O(L)=O(76)
3170  CONTINUE
3180  IF(I.LE.0)GOTO3181
         I=I/B
         J=J+1
      GOTO 3180
3181  CONTINUE
CCOUNT DIGITS IN EXPANSION
      IF (NUM.GE.0) GOTO 3200
         L=L+1
         O(L)=42
3200  CONTINUE
      LLONG=L-LOC+J
3210  CONTINUE
         K=N/B
         O(L+J)=N-K*B
         N=K
         J=J-1
         IF(J.LE.0)GOTO3211
      GOTO 3210
3211  CONTINUE
99999 RETURN
      END
C_______________________________________________________________________
C________________________________________
      integer FUNCTION LEXP(A,Z)
      implicit none
      integer  a,z
      DIMENSIONO(600000)
      COMMON /IBM/O
      integer O
      integer   p,n
      DIMENSION P(100)
CFGHIJKLMQRSUVWY
      EQUIVALENCE(O(49),N),( O(2555),P(1) )
      integer ispec
      integer b,c,nq,mxc,d,e,lone,mp,mq,mv,mu,mnn,t,x,k,jmp
      save    b,c,nq,mxc,d,e,lone,mp,mq,mv,mu,mnn,t,x,k,jmp
      IF (O(A+1).NE.O(105)) GOTO 3230
         LEXP=1
         B=A+1
         C=B+1
         NQ=1
         MXC=Z
3240     CONTINUE
            IF( O(105) .EQ.O(C))NQ=NQ+1
            IF( O(106) .EQ.O(C))NQ=NQ-1
            IF(NQ.EQ.0)GOTO3241
            C=C+1
            IF(C.GT.MXC)CALLMESAGE(15, 35, 0,0)
         GOTO 3240
3241     CONTINUE
         D=C+1
         E=D+1
         IF (O(E).EQ.O(105)) GOTO 3260
            LONE=1
            MP=E
            MQ=Z
            MV=B+1
            MU=C
            IF(ISPEC(MP,MQ,MV,MU,LONE).NE.3)LEXP=0
            GOTO 3250
3260        IF(E.GT.Z-1)GOTO3261
               IF(O(B).NE.O(E))LEXP=0
               B=B+1
               E=E+1
            GOTO 3260
3261        CONTINUE
            IF(B.NE.D) LEXP=0
3250     CONTINUE
         IF(O(D).EQ.89) LEXP=1-LEXP
         GOTO 99999
3230  CONTINUE
      LEXP=0
      O(20)=0
      N=2355
      MNN=N+2
      O(N-1)=38
      O(N)=0
      B=A
3270  IF(B.GT.Z)GOTO3271
         IF (0.GT.O(B).OR.O(B).GT.9) GOTO 3290
            O(N)=0
            O(20)=1
3300        IF(0.GT.O(B).OR.O(B).GT.9)GOTO3301
               O(N)=10*O(N)+O(B)
               B=B+1
            GOTO 3300
3301        CONTINUE
            GOTO 3280
3290        IF  (O(B).NE.O(76))GOTO3310
            B=B+1
            GOTO 3280
3310        T=B
            IF (10.GT.O(T).OR.O(T).GT.29) GOTO 3330
               X=O(T)*O(T+1)
               T=T+2
               IF (O(T).NE.29.AND.O(T).NE.13) GOTO 3350
                  X=X+O(T)
                  T=T+1
3350           CONTINUE
               X=X/20
               IF((10.LT.X).AND.(X.LT.33))GOTO3360
               GOTO3370
3330        CONTINUE
            X=O(T)
            T=T+1
            IF (X.NE.45.OR.O(T).NE.45) GOTO 3390
               T=T+1
               X=36
               GOTO 3380
3390           IF  (X.NE.89)GOTO3400
               X=29
               IF (O(T).NE.41) GOTO 3420
                  T=T+1
                  X=16
3420           CONTINUE
               GOTO 3380
3400           IF  (X.NE.85)GOTO3430
               X=30
               IF (O(T).NE.41) GOTO 3450
                  T=T+1
                  X=14
3450           CONTINUE
               GOTO 3380
3430           IF  (X.NE.41)GOTO3460
               X=18
               IF (O(T).NE.86) GOTO 3480
                  T=T+1
                  X=11
3480           CONTINUE
               GOTO 3380
3460           IF  (X.NE.86)GOTO3490
               X=23
               IF (O(T).NE.41) GOTO 3510
                  T=T+1
                  X=11
3510           CONTINUE
               GOTO 3380
3490           IF  (X.NE.(87))GOTO3520
               X=32
               GOTO 3380
3520           IF  (X.NE.(88))GOTO3530
               X=13
3380        CONTINUE
3530        CONTINUE
            IF(P(X).EQ.0)GOTO3370
3360        IF(X.LT.10.OR.X.GT.O(81) .OR. (X.EQ.O(81).AND.O(20).EQ.0))
     *      GOTO3370
            IF (P(X)/100.LE.P(O(N-1))/100.AND.X.NE.O(80)) GOTO 3550
               O(20)=0
               O(49)=O(49)+1
               O(O(49))=X
               O(49)=O(49)+1
               O(O(49))=0
               IF(O(49).GT.2454)CALLMESAGE(15, 23, 0,0)
               B=T
               GOTO3270
3550        CONTINUE
            IF (N.NE.MNN) GOTO 3570
               LEXP=O(N)
               GOTO 99999
3570        CONTINUE
            K=P(O(N-1))
            O(20)=1
            JMP=K-((K/100)*100)-1
            GOTO(2000,3580,3590,3600,3610,3620,3630,3640,3650,3660,3670,
     *      3680,3690,3700,3710),JMP
3630        IF (O(N).NE.0) GOTO 3730
               CALLMESAGE(14, 10, 0,0)
               GOTO3370
3730        CONTINUE
            O(N-2) = O(N-2) / O(N)
            GOTO3740
3640        O(N-2) = O(N-2) * O(N)
            GOTO3740
3620        O(N-2) = O(N-2) + O(N)
            GOTO3740
3610        O(N-2) = O(N-2) - O(N)
            GOTO3740
3650        O(N-2) = O(N-2) **O(N)
            GOTO3740
2000        O(N-2) = O(N)
            B=T
            GOTO3740
3600        O(N-2) = 1-O(N)
            GOTO3740
3680        IF(O(N-2) .NE. O(N))GOTO3750
            GOTO3760
3690        IF(O(N-2) .EQ. O(N))GOTO3750
            GOTO3760
3670        IF(O(N-2) .LE. O(N))GOTO3750
            GOTO3760
3700        IF(O(N-2) .GT. O(N))GOTO3750
            GOTO3760
3660        IF(O(N-2) .GE. O(N))GOTO3750
            GOTO3760
3710        IF(O(N-2) .LT. O(N))GOTO3750
            GOTO3760
3590        IF(O(N-2).EQ.1 .AND. O(N).EQ.1)GOTO3750
            GOTO3760
3580        IF(O(N-2).EQ.1 .OR. O(N).EQ.1)GOTO3750
            GOTO3760
3750        O(N-2)=1
            GOTO3740
3760        O(N-2)=0
3740        N=N-2
3280     CONTINUE
      GOTO 3270
3271  CONTINUE
3370  CALLMESAGE(14, 23, A,Z+1)
99999 RETURN
      END
C_______________________________________________________________________
C________________________________________
      SUBROUTINE NXTCRD
      implicit none
      DIMENSIONO(600000)
      COMMON /IBM/O
      integer O
      integer mxr,mxt,mxn
      EQUIVALENCE(O(38),MXR),(O(36),MXT),(O(37),MXN)
      integer llong,linf
      integer k,j,w,i,l,m,y,nerd
      save    k,j,w,i,l,m,y,nerd
      O(62)=0
      IF(O(52).EQ.2) CALLMESAGE(16,1,0,0)
3770  CALLRW(0,O(9),325,MXR)
      IF (O(52).NE.1) GOTO 3790
         O(52)=0
         O(9)=O(O(50))
         O(50)=O(50)-1
         IF (O(9).NE.0) GOTO 3810
            O(52)=2
            O(15)=1
            O(12)=O(43)
            O(O(12))=O(79)
            GOTO 99999
3810     CONTINUE
3790  CONTINUE
      DO 3821 K= 325,MXR
      O(K+132)=O(889+O(K))
3821  CONTINUE
      O(19)=O(19)+1
      O(12)=457
      O(157)=O(76)
      IF (O(457).NE.93) GOTO 3860
         IF (O(458).NE.14) GOTO 3880
            O(157)=1
            O(325)=O(689+O(76))
            O(325+1)=O(325)
            O(12)=458+1
            GOTO 3870
3880        IF  (O(458).NE.93)GOTO3890
            O(52)=1
            O(15)=1
            GOTO 3870
3890        IF  (10.GT.O(458).OR.O(458).GT.30)GOTO3900
            CALLCCCARD(O(458)-9)
            O(12)=O(43)
            O(O(12))=O(79)
3870     CONTINUE
3900     CONTINUE
         IF (O(52).NE.1.AND.O(458).NE.22.AND.O(458).NE.15) GOTO 3920
            O(12)=O(43)
            O(O(12))=O(79)
            GOTO 99999
3920     CONTINUE
3860  CONTINUE
3930  IF (O(21).NE.1) GOTO 3950
         DO 3961 J = 158,174
            O(J)=O(76)
3961     CONTINUE
         IF (O(30).NE.1) GOTO 3980
            J=LLONG(158,O(19),10,0)
            GOTO 3970
3980        IF  (O(30).NE.2)GOTO3990
            DO 4001 J = 1,8
               O(157+J)=O(O(43)+J)
4001        CONTINUE
3970     CONTINUE
3990     CONTINUE
         IF(O(68).NE.0)O(167)=33
         O(169)=O(55)
         J=LLONG(170,O(11),10,0)
         IF (O(53).NE.0) GOTO 4020
            W=174
            DO 4031 K = 1,MXN
               W=W+1
               O(W)=O(456+K)
4031        CONTINUE
            CALLRW(18,O(56),157,W)
            GOTO 4010
4020        I=MOD(MAX0(O(53)*O(11),0),90)
            L=457
            M=O(43)
4040        IF(O(L).NE.O(76).OR.L.EQ.M)GOTO4041
               L=L+1
            GOTO 4040
4041        CONTINUE
4050        IF(O(M).NE.O(76).OR.L.EQ.M)GOTO4051
               M=M-1
            GOTO 4050
4051        CONTINUE
4060        CONTINUE
               W=175
               DO 4071 Y = 1,I
                  O(W)=O(76)
                  W=W+1
4071           CONTINUE
4080           IF(W.GT.288 .OR. L.GT.M)GOTO4081
                  O(W)=O(L)
                  W=W+1
                  L=L+1
               GOTO 4080
4081           CONTINUE
               CALLRW(18,O(56),157,W-1)
               IF(L.GT.M)GOTO4061
               DO 4091 K = 157,175
                  O(K)=O(76)
4091           CONTINUE
            GOTO 4060
4061        CONTINUE
4010     CONTINUE
3950  CONTINUE
      IF (O(15).NE.1) GOTO 4110
         IF (O(457).NE.93 ) CALLRW(23,O(57),325,404)
         GOTO 4100
4110     IF  (O(10).NE.1)GOTO4120
         NERD=LINF(457,MXT,10)
4100  CONTINUE
4120  CONTINUE
      IF(O(15).EQ.1)GOTO3770
      IF (O(14).LT.2) GOTO 99999
      J=456
      K=457
4130  IF(K.GT.MXT)GOTO4131
         J=J+1
         O(J)=O(K)
         K=K+1
         IF (O(J).NE.45) GOTO 4150
            IF (O(K).NE.46) GOTO 4170
               O(J)=O(85)
               K=K+1
               GOTO 4160
4170           IF  (O(K).NE.86)GOTO4180
               O(J)=O(83)
               K=K+1
4160        CONTINUE
4180        CONTINUE
            GOTO 4140
4150        IF  (O(K).NE.45)GOTO4190
            IF (O(J).NE.38) GOTO 4210
               O(J)=O(84)
               K=K+1
               GOTO 4200
4210           IF  (O(J).NE.85)GOTO4220
               O(J)=O(82)
               K=K+1
4200        CONTINUE
4220        CONTINUE
4140     CONTINUE
4190     CONTINUE
      GOTO 4130
4131  CONTINUE
4230  IF(J.GT.MXT)GOTO4231
         J=J+1
         O(J)=90
      GOTO 4230
4231  CONTINUE
99999 RETURN
      END
C_______________________________________________________________________
C__________________________________________
C_______________________________________________________________________
C________________________________________
      SUBROUTINE CCCARD(K)
      implicit none
      integer k
      DIMENSIONO(600000)
      COMMON /IBM/O
      integer O
      integer n5
      integer jmp,i
      save    jmp,i
      JMP=K
      GOTO(680,4240,310,4250,3640,4260,3640,3640,4270,3640,3640,1970,
     *1390,1980,3640,3640,2010,2020,2030,2040,2050),JMP
CABCDEFGHJLMNPQRSTUVWXYZ
680   O(10) =N5(0,2,1)
      GOTO 99999
4240  O(37) =N5(0,132,132)
      O(38)=324+O(37)
      O(36)=456+O(37)
      GOTO 99999
310   O(41)=N5(10,132,132)
      O(43)=456+O(41)
      O(42)=324+O(41)
      GOTO 99999
4250  O(13)=N5(0,2,0)
      GOTO 99999
4260  O(15) =1
      GOTO 99999
4270  O(53)=N5(0,50,2)
      GOTO 99999
1970  O(21)=1
      GOTO 99999
1390  O(15) =0
      GOTO 99999
1980  O(21)=0
      GOTO 99999
2010  O(26)=N5(0,1,1)
      GOTO 99999
2020  I =N5(1,99,8)
      REWIND I
      GOTO 99999
2030  O(30)=N5(0,2,0)
      GOTO 99999
2040  O(29)=N5(0,9,0)
      GOTO 99999
2050  O(50)=O(50)+1
      O(O(50))=O(9)
      IF(O(50).GT.2504)CALLMESAGE(15, 30, 0,0)
      O(9)=N5(1,99,O(9))
      GOTO 99999
3640  CALLMESAGE(14, 22, 0,0)
99999 RETURN
      END
C_______________________________________________________________________
C__________________________________________
      integer FUNCTION N5(L,M,K)
      implicit none
      integer  l,m,k
      DIMENSIONO(600000)
      COMMON /IBM/O
      integer O
      integer nib
      integer j
      save    j
      J=457+4
      N5=NIB(J,10)
      IF ((L.LE.N5).AND.(N5.LE.M)) GOTO 4290
         CALLMESAGE(14, 3, 0,0)
         N5=K
4290  CONTINUE
99999 RETURN
      END
C_______________________________________________________________________
C________________________________________
      SUBROUTINE MACTRC(W,R,S)
      implicit none
      integer  w,r,s
      DIMENSIONO(600000)
      COMMON /IBM/O
      integer O
      integer k,m,t,n
      save    k,m,t,n
      IF (O(23).GT.0.AND.O(29).LT.3.AND.W.NE.4)GOTO 99999
      IF (W.NE.2) GOTO 4310
         CALLDUMDUM(175,90,37,37,37,22,42,29,27,34,90)
         O(175+4)=(O(32)-1145)/120
         CALLCRIT(R,S)
         GOTO 4300
4310     IF  (W.NE.4)GOTO4320
         CALLDUMDUM(175,90,45,28,29,27,10,23,16,14,90)
         CALLCRIT(R,S)
         GOTO 4300
4320     IF  (W.NE.5)GOTO4330
         CALLDUMDUM(175,90,37,10,16,10,18,23,28,29,90)
         CALLCRIT(R,S)
4300  CONTINUE
4330  CONTINUE
      IF (W.NE.33) GOTO 4350
         IF (O(23).NE.0) GOTO 4370
            CALLDUMDUM(175,90,90,27,14,13,30,12,14,13,90)
            GOTO 4360
4370        CALLDUMDUM(175,90,90,90,90,90,23,14,28,29,90)
            O(175+3)=O(23)
4360     CONTINUE
         CALLCRIT(R+1,S+1)
         GOTO 4340
4350     IF  (W.NE.15)GOTO4380
         CALLDUMDUM(175,90,90,13,14,15,18,23,14,13,90)
         CALLCRIT(R+4,O(R+3))
         IF (R+4.GE.O(R+3)) GOTO 4400
            CALLDUMDUM(175,90,90,90,90,31,10,21,30,14,90)
            O(175+2)=O(54)
            CALLCRIT(S+4,O(S+3))
            GOTO 4390
4400        CALLDUMDUM(175,90,23,30,21,21,90,31,10,21,90)
            CALLCRIT(S,S)
4390     CONTINUE
         GOTO 4340
4380     IF  (W.NE.12)GOTO4410
         CALLDUMDUM(175,90,10,25,25,14,23,13,14,13,90)
         CALLCRIT(O(S-2*(3)-1),O(S-2*(3)-2) )
         CALLDUMDUM(175,90,90,90,90,90,90,90,29,24,90)
         CALLCRIT(O(S-2*(1)-1),O(S-2*(1)-2) )
         GOTO 4340
4410     IF  (W.NE.13)GOTO4420
         CALLDUMDUM(175,90,90,27,14,22,24,31,14,13,90)
         CALLCRIT(O(S-2*(1)-1),O(S-2*(1)-2) )
         GOTO 4340
4420     IF  (W.NE.22)GOTO4430
         IF(O(R+4).LT.0)GOTO 99999
         CALLDUMDUM(175,90,90,22,10,29,12,17,14,13,90)
         CALLCRIT(R+4,O(R+3) )
         K=O(S-1)
         IF (K.EQ.0) GOTO 4450
            DO 4461 M = 1,K
               CALLDUMDUM(175,90,90,90,90,10,27,16,90,90,90)
               O(175+8)=M
               T=O(S-2*(M)-1)
               N=O(S-2*(M)-2)
               IF (T.NE.N) CALLCRIT(T,N)
4461        CONTINUE
4450     CONTINUE
4340  CONTINUE
4430  CONTINUE
99999 RETURN
      END
C_______________________________________________________________________
C__________________________________________
      SUBROUTINE CRIT(A,Z)
      implicit none
      integer  a,z
      DIMENSIONO(600000)
      COMMON /IBM/O
      integer O
      integer b,c
      save    b,c
      B=A
4470  IF(B.GE.Z)GOTO4471
         C=175+9
4480     IF(C.GE.246.OR.B.GE.Z)GOTO4481
            C=C+1
            O(C)=O(B)
            B=B+1
            IF(O(C).EQ.O(76))O(C)=37
            IF(O(C).GT.100+19)O(C)=100
            IF(O(C).EQ.-1)O(C)=74
            IF(O(C).EQ.-2)O(C)=82
         GOTO 4480
4481     CONTINUE
         CALLRW(33,O(56),175,C)
         CALLDUMDUM(175,90,90,90,90,90,90,90,90,90,90)
      GOTO 4470
4471  CONTINUE
99999 RETURN
      END
C_______________________________________________________________________
C________________________________________
      SUBROUTINE MESAGE (LEVEL,NO,K1,K2)
      implicit none
      integer level,no,k1,k2
      DIMENSIONO(600000)
      COMMON /IBM/O
      integer O
      common/junk/ verbose
      logical verbose
      IF(LEVEL.EQ.14 .OR. LEVEL.EQ.15)O(63)=O(63)+1
      CALLFIND(LEVEL,NO,K1)
      IF(K2.NE.0)CALLMACTRC(4,K1,K2)
      IF (LEVEL.NE.13) GOTO 4500
         CALLFIND(13,2,O(60))
         CALLFIND(13,3,O(61))
4500  CONTINUE
      IF (LEVEL.GT.14) GOTO 4520
         IF(O(63).LT.100)RETURN
         CALLFIND(15,10,0)
4520  CONTINUE
      IF (O(55).NE.O(86).AND.O(55).NE.O(87)) GOTO 4540
         O(63)=O(63)+1
         IF(O(55).EQ.O(86)) CALLFIND(16,5,0)
         IF(O(55).EQ.O(87))CALLFIND(16,4,0)
4540  CONTINUE
      IF(O(47).GT.2255.OR.O(48).GT.2305)O(63)=O(63)+1
      IF (O(48).LE.2305) GOTO 4560
         CALLFIND(16,3,O(48)-2305)
4560  CONTINUE
4570  IF(O(47).LE.2255)GOTO4571
         CALLFIND(16,2,O(O(47)))
         O(47)=O(47)-1
      GOTO 4570
4571  CONTINUE
      CALLFIND(13,3,O(61))
      IF (O(63).NE.0) GOTO 4590
         CALLFIND(16,6,0)
         GOTO 4580
4590     CALLFIND(16,7,O(63))
         write(6,*) 'compilation error (stop 12)'
         call exit(12) !stop 12
4580  CONTINUE
      IF (O(59).EQ.0)CALLINITAL(2)
      if( verbose ) write(6,*) 'OK'
      call exit(0) !stop 0
      END
C_______________________________________________________________________
C__________________________________________
      SUBROUTINE FIND(L,N,V)
      implicit none
      integer l,n,v
      DIMENSIONO(600000)
      COMMON /IBM/O
      integer O
      integer llong
      integer a,c,q,r,s,nerd,k
      save    a,c,q,r,s,nerd,k
      A=175
      C=A+9
      Q=1865+L
680   IF (O(Q).EQ.0) GOTO 4610
         Q=O(Q)
         IF(O(Q+5).NE.N)GOTO680
         R=O(Q+2)+4
         S=O(O(Q+2)+3)
4620     IF(R.GE.S)GOTO4621
            C=C+1
            O(C)=O(R)
            R=R+1
         GOTO 4620
4621     CONTINUE
         GOTO 4600
4610     C=C+LLONG(C,N,10,0)
4600  CONTINUE
      IF(L.EQ.15) CALLDUMDUM(175,90,45,45,45,15,10,29,10,21,90)
      IF(L.EQ.14) CALLDUMDUM(175,90,45,32,10,27,23,18,23,16,90)
      IF (L.NE.13.AND.L.NE.16) GOTO 4640
         CALLDUMDUM(175,90,90,90,90,90,90,90,90,90,90)
         IF(V.NE.0) NERD=LLONG(A,V,10,0)
4640  CONTINUE
      CALLRW(33,O(56),A,C)
      IF (O(63).GT.0) CALLRW(23,O(57),A,C)
      DO 4651 K = A,C
         O(K)=O(76)
4651  CONTINUE
      C=175+8
99999 RETURN
      END
C_______________________________________________________________________
C________________________________________
      integer FUNCTION LINF(A,Z,IND)
      implicit none
      integer  a,z,ind
      DIMENSIONO(600000)
      COMMON /IBM/O
      integer O
      integer llong
      integer last,kur,kit,kut,kst,h,b,i,j,npl,d,koff,c,limc,k
      DATA LAST/0/,KUR/0/,KIT/1/,KUT/0/
      save    last,kur,kit,kut,kst,h,b,i,j,npl,d,koff,c,limc,k
      LINF=A-1
CCEFGLMNPQRS UVWXY
      O(62)=0
      IF (IND.EQ.12 .AND. (O(25).EQ.0.OR.O(10).EQ.1
     * .OR.O(96).EQ.O(A)))GOTO 99999
      KST=0
      H=0
      B=A
      I=175
      KUT=KUR
      IF(O(B).NE.O(75))GOTO430
      B=B+2
      KIT=O(B-1)-1
      KUR=KUR+KIT
      KUT=KUR
      IF(KIT.NE.0)GOTO430
      KUT=KUT-1
      KIT=1
430   CONTINUE
      DO 4661 J = 175,254
         O(J)=O(76)
4661  CONTINUE
      IF (IND.NE.15) GOTO 4680
4690     IF(I.GE.181.OR.B.GT.Z)GOTO4691
            IF (O(B).NE.O(76)) GOTO 4710
               B=B+1
               GOTO4690
4710        CONTINUE
            IF (0.GT.O(B).OR.O(B).GT.9) GOTO 4730
               O(I)=O(B)
               I=I+1
               B=B+1
               GOTO 4720
4730           GOTO4691
4720        CONTINUE
         GOTO 4690
4691     CONTINUE
         NPL=65
         D=45
         KOFF=7
         GOTO 4670
4680     NPL=71
         D=12
         O(175)=D
         KOFF=MAX(MIN(80,O(39)),2)
4670  CONTINUE
      IF (B.LE.Z) GOTO 4750
         IF (I.LE.175) GOTO 4770
            CALLRW(33,O(57),175,181)
            IF(IND.EQ.15)CALLMESAGE(14, 14, 0,0)
            GOTO 99999
4770     CONTINUE
4750  CONTINUE
      C=174+KOFF+KUT*O(18)
      IF(KUT.GT.LAST)C=C-O(18)
      IF(C.GT.174+50)C=174+7
      I=C
4780  IF(B.GT.Z)GOTO4781
         LIMC=MIN0(B+NPL,Z)
4790     IF(B.GT.LIMC.OR.I.GT.246)GOTO4791
            IF (IND.NE.15) GOTO 4810
               IF (O(17).NE.1) GOTO 4830
                  IF (O(B).NE.O(105)) GOTO 4850
                     IF(I.GT.246-4)GOTO4791
                     H=1
4860                 IF(O(B+H).EQ.O(106))GOTO4861
                        H=H+1
                     GOTO 4860
4861                 CONTINUE
                     O(B+H)=O(76)
                     IF (H.NE.1) GOTO 4880
                        O(B)=O(86)
                        H=0
                        GOTO 4870
4880                    I=I+LLONG(I-1,H-1,10,0)
                        O(I)=17
                        B=B+1
                        I=I+1
4870                 CONTINUE
4850              CONTINUE
                  GOTO 4820
4830              IF (O(B).NE.O(86)) GOTO 4900
                     O(I)=O(B)
                     I=I+1
4900              CONTINUE
4820           CONTINUE
4810        CONTINUE
            IF (O(B).NE.O(70)) GOTO 4920
               B=B+1
               GOTO4791
4920        CONTINUE
            O(I)=O(B)
            I=I+1
            B=B+1
         GOTO 4790
4791     CONTINUE
         IF (O(30).NE.1) GOTO 4940
            DO 4951 K = 1,8
               O(254-K+1)=O(O(36)-K+1)
4951        CONTINUE
            GOTO 4930
4940        IF  (O(30).NE.2)GOTO4960
            K=LLONG(247,O(19),10,0)
4930     CONTINUE
4960     CONTINUE
         DO 4971 K = 175,246
            IF (O(K).NE.O(105)) GOTO 4990
               O(K)=O(86)
               KST=1
4990        CONTINUE
            IF (O(K).NE.O(106)) GOTO 5010
               O(K)=O(86)
               KST=0
5010        CONTINUE
4971     CONTINUE
         IF (I.LE.246.OR.IND.NE.15.OR.KST.NE.0) GOTO 5030
5040        IF(O(I-1).GE.38.OR.KST.LE.0)GOTO5041
               B=B-1
               I=I-1
               O(I)=O(76)
            GOTO 5040
5041        CONTINUE
5030     CONTINUE
         DO 5051 K = 175,254
            IF(0.GT.O(K).OR.O(K).GT.100-1)O(K)=100-1
            O(K)=O(689+O(K))
5051     CONTINUE
C   LOOKOUT
         CALLRW(23,O(57),175,254)
         DO 5061 K = 175,254
            O(K)=O(76)
5061     CONTINUE
         I=C
         IF (IND.NE.15) GOTO 5080
            O(175+5)=D
            GOTO 5070
5080        O(175)=D
5070     CONTINUE
         IF (KST.LE.0) GOTO 5100
            I=181
            O(181-1)=45
5100     CONTINUE
      GOTO 4780
4781  CONTINUE
      LAST=KUR
      IF(IND.EQ.12) O(A)=O(96)
99999 RETURN
      END
C_______________________________________________________________________
C__________________________________________
      SUBROUTINE DUMDUM(L,A,B,C,D,E,F,G,H,I,J)
      implicit none
      integer L,A,B,C,D,E,F,G,H,I,J
      DIMENSIONO(600000)
      COMMON /IBM/O
      integer O
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
99999 RETURN
      END
C_______________________________________________________________________
C__________________________________________
      SUBROUTINE RW(N,I,J,K)
      implicit none
      integer  N,I,J,K
      CHARACTER*1 CH(200)
      INTEGER     HC(200)
      DIMENSIONO(600000)
      COMMON /IBM/O
      integer O
C  IN RW
C
      common/nrcc/ oflag
      integer      oflag
C
      logical skip_input,is_format,is_quote,is_quote_last
      integer nr,m,kof,l,nrr,nformat,itmp,nfl
      integer nn6,nn7,jj,nfirst,ii,m1
      character tmp_string*8, tmp1_string*6, f_buffer(8192)
      equivalence (ch,tmp_string)
      data nn6,nn7,nfl/0,0,0/,
     &     skip_input/.false./, is_format/.false./, is_quote/.false./
      data nformat/0/
      save nn6,nn7,hc,ch,nformat,nfl
      save nr,m,kof,l,nrr,skip_input,is_format,is_quote
      save f_buffer

      NR=K+1-J
      IF (N.NE.0) GOTO 5120
         !write(44,*) 'nr = ',nr,k,j,i
         READ(I,1,END=5130) (CH(M),M=1,NR)
         if( i.ne.1 ) then
         do m=1,nr-1
           if( .not.skip_input ) then
             ! the following does not work because of possibly
             ! having 2 or more / in a format statement
             !
             !if( ch(m).eq.'/'.and.ch(m+1).eq.'/' ) then
             !  ! a C++ style comment
             !  write(44,*) 'found C++ style comment: '
             !  write(44,1) (CH(ii),ii=1,NR)
             !  do m1=m,nr
             !    ch(m1) = ' '
             !  end do
             !  goto 50
             !end if
             if( ch(m).eq.'/'.and.ch(m+1).eq.'*' ) then
               ! A C style comment
               skip_input = .true.
               !write(44,*) 'found begin of C style comment '
               !write(44,1) (CH(ii),ii=1,NR)
               ch(m) = ' '
               ch(m+1) = ' '
             endif
           end if
           if( skip_input ) then
             if( ch(m).eq.'*'.and.ch(m+1).eq.'/' ) then
               !write(44,*) 'found end of C style comment '
               skip_input = .false.
               ch(m+1) = ' '
             endif
             ch(m) = ' '
           end if
         end do
50       continue
         endif
         !write(44,1) (CH(M),M=1,NR)
         DO 99 M=1,NR
99       O(J+M-1)=ICHAR(CH(M))
         GOTO 99999
5120  CONTINUE
       KOF=589
      IF (N.EQ.33) KOF=100+KOF
      IF (N.EQ.23) KOF=0
         DO 5181 L=1,NR
         HC(L) = MIN(255,MAX(0, O(J+L-1) ))
         IF (KOF.GT.0) HC(L)=O(HC(L)+KOF)
5181        CH(L)=CHAR( HC(L)  )
      if( i.eq.oflag ) then
        nn6 = nn6+1
        if( nn6.eq.1 ) return
      end if
      if( i.eq.7 ) then
        nn7 = nn7+1
        if( nn7.eq.1 ) return
      end if
      !nrr=nr
      !do jj=nr,1,-1
      !  if( ch(jj).eq.' ' ) then
      !     nrr=nrr-1
      !  else
      !     goto 20
      !  end if
      !end do
      ! GCC versions < 3.4.0 miscompile the above when optimization is on.
      ! --IK, Nov 23 2005.
      do jj=nr,1,-1
          if( ch(jj).ne.' ' ) goto 20
      end do
20    continue
      nrr = jj

      ! Check for C-preprocessor statements
      ! If the first 6 chars on the line are blank and if the first non-blank
      ! char from coloumn 7 is a #, assume it is a C-preprocessor directive
      ! and make the # to be the first char on the line.
      nfirst=1
      do jj=1,nrr
        if( ch(jj).ne.' ' ) goto 31
      end do
31    continue
      if( ch(jj).eq.'#' ) then
        do ii=jj,nrr
          ch(ii+1-jj) = ch(ii)
        end do
        do ii=nrr+2-jj,200
          ch(ii) = ' '
        end do
        nrr = nrr + 1 - jj
        if( tmp_string.eq.'#include' ) then
          do ii=1,nrr
            if( ichar(ch(ii)).eq.39 ) ch(ii) = char(34)
          end do
        end if
      endif
30    continue

      if(i.eq.7) then
      if( is_format ) then
          jj = 7
          if( .not.is_quote ) then
              do jj=7,nrr
                  if( ch(jj).ne.' ' ) goto 66
              end do
          end if
66        continue
          do ii=jj,Nrr
              nfl = nfl + 1
              if( nfl.gt.8192 ) then
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
              if( ichar(ch(ii)).eq.39 ) then
                  if( is_quote ) then
                      is_quote = .false.
                  else
                      is_quote = .true.
                  end if
              end if
              if( .not.is_quote ) then
                  if( ch(ii).eq.'(' ) then
                      nformat = nformat + 1
                  else if( ch(ii).eq.')') then
                      nformat = nformat - 1
                  end if
              end if
          end do
          if( .not.is_quote ) then
              do ii=nfl,1,-1
                  if( f_buffer(ii).ne.' ' ) goto 62
              end do
62            continue
              nfl = ii
          end if
          if( nformat.eq.0 ) then
              is_format = .false.
              is_quote = .false.
              jj = 72
              if( nfl.le. 72 ) jj = nfl
              write(i,'(72a1)') (f_buffer(ii),ii=1,jj)
67            continue
              if( jj.ge.nfl ) then
                  nfl = 0
                  return
              end if
              ii = jj + 66
              if( ii.gt.nfl ) ii = nfl
              write(i,'(a6,66a1)') '     *',(f_buffer(m),m=jj+1,ii)
              jj = ii
              goto 67
          end if
          return
      endif

      if( .not.is_format ) then
          do jj=7,nrr
              if( ch(jj).ne.' ') goto 61
          end do
61        continue
          do ii=jj,jj+5
              itmp = ichar(ch(ii))
              if( itmp.ge.97.and.itmp.le.122 ) itmp=itmp-32
              tmp1_string(ii+1-jj:ii+1-jj) = char(itmp)
          end do
          if( tmp1_string.eq.'FORMAT' ) then
              is_format = .true.
              nfl = Nrr
              do ii=1,Nrr
                  f_buffer(ii) = ch(ii)
              end do
          endif
      endif

      if( is_format ) then
          do jj=7,nrr
              if( ichar(ch(jj)).eq.39 ) then
                  if( is_quote ) then
                       is_quote = .false.
                   else
                       is_quote = .true.
                   endif
              end if
              if( .not.is_quote ) then
                  if( ch(jj).eq.'(' ) then
                      nformat = nformat + 1
                  else if( ch(jj).eq.')' ) then
                      nformat = nformat - 1
                  endif
              endif
          end do
          if( nformat.eq.0 ) then
              is_format = .false.
              if( is_quote ) write(8,*) 'Unclosed quote?'
              is_quote = .false.
              nfl = 0
          endif
      end if

      if( is_format ) return
      endif

      write(i,1) (ch(m),m=1,Nrr)
1     FORMAT(132A1)

C
C	NRC PATCH TO PUT DATE AND TIME AT EACH PAGE THROW
C
      if( CH(1).EQ.'1' .AND. I.EQ.6 ) then
          write(I,'(T55,$)') ' '
          call egs_fdate(I)
          write(I,*)
      end if
      GOTO 99999
5130  O(52)=1
99999 RETURN
      END
C_______________________________________________________________________
C________________________________________
      SUBROUTINE INITAL(IND)
      implicit none
      integer  ind
      CHARACTER*1 COUNT
      DIMENSIONO(600000)
      COMMON /IBM/O
      integer O
      common /junk/ verbose
      logical verbose
      integer kount,k,iuni
      integer b,e,r,s,v,u,w,x
      EQUIVALENCE(O(1),B),(O(2),E),(O(3),R),(O(4),S), (O(5),V),(O(6),U),
     *(O(7),W),(O(8),X)
      save    kount,k,iuni,count
      IF (IND.NE.2) GOTO 5200
         O(59)=1
         O(19)=5
         KOUNT=O(34)+1
         O(15)=0
         O(9)=5
         WRITE(7,1)KOUNT,(O(K),K=1,KOUNT)
1        FORMAT(10Z8)
2        FORMAT(A1)
         if( verbose ) write(6,*) 'OK'
         call exit(0) !stop 0
5200  CONTINUE
      DO 5211 K = 7,600000
         O(K)=0
5211  CONTINUE
      O(9)=O(1)
      O(56)=O(2)
      O(57)=O(3)
      IUNI=O(9)
C     O(14)=O(4)   THIS IS NO LONGER USED IN FORTRAN 77 VERSIONS
      O(16)=O(6)
      READ(IUNI,2)COUNT
      REWIND IUNI
      IF (COUNT.EQ.'C') GOTO 5230
         READ (IUNI,1) KOUNT,(O(K),K=1,KOUNT)
C
C     The following line
C        CALLRW(23,O(57),325+2,325+79)
C     has been changed to conform with our (NRCC) current version.
C     The effect seems to be that the
C     %%1Copyright ....
C     line at the end of MORNEW77.RAW gets properly handled.
C     Previous to this date, the old version was in our standard
C     distribution.
C     Alex Bielajew 88/09/20
C
         CALLRW(23,O(57),325+3,325+79)
         CALLRW(23,O(56),325+2,325+79)
         REWIND IUNI
         GOTO 5220
5230     CALLRAW
5220  CONTINUE
      O(55)=O(76)
      O(40)=525371
      O(32)=1625
      DO 5241 K = 1,6
         O(K)=525371
5241  CONTINUE
      O(88)=-11
      O(89)=-12
      O(75)=-13
      O(70)=-14
      O(U)=O(79)
      W=450741-1
      O(W)=O(79)
      CALLDUMDUM(44,2105,2155,2205,2255,2305,2355,2455,2505,0,0)
      CALLNXTCRD
99999 RETURN
      END
C_______________________________________________________________________
C__________________________________________
      SUBROUTINE RAW
      implicit none
      DIMENSIONO(600000)
      COMMON /IBM/O
      integer O
      integer b,e,r,s,v,u,w,x
      EQUIVALENCE(O(1),B),(O(2),E),(O(3),R),(O(4),S), (O(5),V),(O(6),U),
     *(O(7),W),(O(8),X)
      integer   p
      DIMENSION P(100)
      EQUIVALENCE(O(2555),P(1))
      integer  k,j
      save     k,j
      O(37)=O(5)
      O(41)=O(5)
      O(38)=324+O(37)
      O(36)=456+O(37)
      O(43)=456+O(41)
      O(42)=324+O(41)
      CALLRW(0,O(9),325,O(38))
      CALLRW(0,O(9),689,689+73)
      CALLRW(0,O(9),325,O(38))
      CALLRW(0,O(9),689+74,789+70)
      DO 5251 K = 689,788
         O(K-100)=O(K)
5251  CONTINUE
CMAKE INPUT AND OUTPUT SAME
C .(MNOSET+54)=.(MNOSET+41);     OUT-TRANSLATE L-ARROW TO =
      CALLRW(23,O(56),325+41,404)
      CALLRW(23,O(57),325+41,404)
      DO 5261 K = 1,100
            J=O(589+K-1)
         O(889+J)=K-1
C     WRITE(6,1) J,K-1
1     FORMAT(2I5)
5261  CONTINUE
      O(31)=4
      O(34)=2655
      DO 5291 K = 1,10
         O(K+100)=K+100
         O(K+110)=K+110
         O(100-K)=-K
5291  CONTINUE
C     BLANK BAT BLB TRM LPR RPR LSB RSB LCB RCB
      CALLDUMDUM(76, 90, 79, 78, 75, 38, 46, 74, 82, 83, 84)
      O(86)=77
      O(87)=80
      CALLDUMDUM(29001,29018,29016,29016,29016,29016,29009,0,3,0,0)
      CALLDUMDUM(29009,38,108,46,108,38,108,46,107,24,0)
      CALLDEFINE(29009)
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
99999 RETURN
      END


      subroutine open_units

      implicit none
      DIMENSIONO(600000)
      COMMON /IBM/O
      integer O
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
      if(narg.eq.0) then

        ! no arguments -> assume program is called from a script
        ! making all necessary links and hope for the best

        OPEN(UNIT=1,file='fort.1',STATUS='OLD',err=11)
        goto 12
11      call getarg(0,arg)
        write(6,'(a,a)') arg(1:lnblnk1(arg)),' called without arguments'
        write(6,'(a)') '  but file fort.1 does not exist! '
        write(6,'(a)') '  quiting now.'
        call exit(1) !stop 1
12      continue
        OPEN(UNIT=5,file='fort.5',STATUS='OLD',ERR=6)
6       OPEN(UNIT=6,file='fort.6',STATUS='UNKNOWN')
        OPEN(UNIT=7,file='fort.7',STATUS='UNKNOWN')
        OPEN(UNIT=2,file='fort.2',STATUS='OLD',ERR=3)
3       OPEN(UNIT=3,file='fort.3',STATUS='OLD',ERR=4)
4       OPEN(UNIT=4,file='fort.4',STATUS='OLD',ERR=8)
8       OPEN(UNIT=8,file='fort.8',STATUS='OLD',ERR=9)
9       CONTINUE
        O(1)=1   ! hex/raw data unit number
        O(2)=6   ! listing unit number
        O(3)=7   ! fortran/hex output unit number
        oflag = 6
        if( verbose ) then
          write(6,'(a)') 'mortran3.exe: using standard fort.* units'
          write(6,'(/a,$)') '  Mortran compiling ... '
        end if
c        call getarg(0,arg)
c        write(6,'(a,a)') arg(1:lnblnk1(arg)),
c     &    ': using standard fort.* units'
c        write(6,'(/a,$)') '  Mortran compiling ... '
        return

      end if

      call getarg(1,arg)
      if( arg(1:).eq.'-help' ) then
        write(6,'(/a/,a/)')
     & 'Mortran version 3.2 by Ray F. Cowan with modifications by ',
     & 'Dave Rogers, Alex Bielajew and Iwan Kawrakow'
        write(6,'(/a/)') 'Usage: mortran3.exe [ options ]'
        write(6,'(a)')
     & ' If no options are given, mortran3 will attempt to use the ',
     & ' following units (fort.*) files:',
     & '   1:  Read raw or hex mortran data ',
     & '   5:  Read mortran job file ',
     & '   7:  Write hex data (if data creation) or fortran output',
     & '   8:  Write listing (useful for debuging)'
        write(6,'(a)')
     & ' Symbolik links to the raw or hex data and to the mortran job',
     & ' file must therefore have been made prior to invoking the ',
     & ' mortran3 command. It is also useful to link unit 7 and 8 to ',
     & ' the Desired fortran and listing output files. '
        write(6,'(a)')
     & ' Note that the concept of symbolik links does not exist under',
     & ' Windows and so, one has to either copy the input files to ',
     & ' fort.1 and fort.5 or to use mortran3 with the options ',
     & ' described below.'
        write(6,*)
        write(6,'(a)')
     & ' The current version recognizes the following options:',
     & '   -d file_name  use file name as the name of the raw or hex',
     & '                 data file',
     & '   -c file_name  use file name as the name of a configuration',
     & '                 file. This file must contain file names, one',
     & '                 per line, that will be concatenated in the ',
     & '                 order given and the result will be used as a',
     & '                 mortran job input.',
     & '   -f f1 f2 ...  similar as the -c option, but now the files ',
     & '                 to be concatenated are given directly on the ',
     & '                 command line as f1 f2 etc.'
        write(6,'(a)')
     & '   -o7 fname     write mortran3 output to the file fname',
     & '                 this is either the hex data file in the ',
     & '                 case of hex data generation or the fortran ',
     & '                 result of the mortran compilation.',
     & '   -o8 fname     write listing output to the file fname',
     & '   -help         print this message and exit',
     & '   -s            make the pre-processor run silently',
     & '   -i            ignore missing files specified via -c or -f',
     & '                 instead of aborting, as in the case without -i'
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
        if( arg(1:2).eq.'-s' ) then
          verbose = .false.
          continue
        end if
        if( arg(1:2).eq.'-i' ) then
          ignore_missing = .true.
          continue
        end if
        if( arg(1:2).eq.'-c' ) then
          if( i.lt.narg ) then
            call getarg(i+1,arg)
            if( arg(1:1).ne.'-' ) nc=i+1
          end if
        else if( arg(1:2).eq.'-f' ) then
          nfs=i+1
          do j=i+1,narg
            call getarg(j,arg)
            if( arg(1:1).eq.'-' ) then
              goto 88
            end if
            nfe=j
          end do
88        continue
        else if( arg(1:2).eq.'-d' ) then
          if( i.lt.narg ) then
            call getarg(i+1,arg)
            if( arg(1:1).ne.'-' ) nd=i+1
          end if
        else if( arg(1:3).eq.'-o7' ) then
          if( i.lt.narg ) then
            call getarg(i+1,arg)
            if( arg(1:1).ne.'-' ) no7=i+1
          end if
        else if( arg(1:3).eq.'-o8') then
          if( i.lt.narg ) then
            call getarg(i+1,arg)
            if( arg(1:1).ne.'-' ) no8=i+1
          end if
        end if
      end do

      !call getarg(0,arg)
      !write(6,'(a,a)') arg(1:lnblnk1(arg)),': using following I/O units'
      if( verbose ) then
      write(6,'(a)') 'mortran3.exe: using following I/O units'
      end if
      if( nd.gt.0 ) then
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
      if( (nc.gt.0).or.(nfs.le.nfe) ) then
        if(nc.gt.0) then
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
            if( lnblnk1(next_file).gt.0 ) then
              inquire(file=next_file,exist=is_there)
              if( is_there ) then
                if( verbose ) write(6,'(a,a)') '  -> appending ',
     &                        next_file(1:lnblnk1(next_file))
                call add_file(12,next_file)
              else
                if( ignore_missing ) then
                  if( verbose ) write(6,'(a,a,a)') '  file "',
     &              next_file(1:lnblnk1(next_file)),
     &              '" does not exist. Ignoring it.'
                else
                  write(6,'(a,a,a)') '  file "',
     &              next_file(1:lnblnk1(next_file)),
     &              '" does not exist. Quiting now.'
                  call exit(99) !stop 99
                end if
              end if
            end if
            goto 104
102         next = .false.
104         continue
          end do
          close(11)
          write(12,'(/a)') '%%'
          close(12)
          arg = 'mortjob.mortran'
          open(5,file=arg,status='old',err=101)
        else
          if( nfs.eq.nfe ) then ! a single command line file
            call getarg(nfs,next_file)
            if( verbose ) then
            write(6,'(a,a)') '  command line file: ',
     &         next_file(1:lnblnk1(next_file))
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
                if( verbose ) write(6,'(a,a)') '  -> appending ',
     &                        next_file(1:lnblnk1(next_file))
                call add_file(12,next_file)
              else
                if( ignore_missing ) then
                  if( verbose ) write(6,'(a,a,a)') '  file "',
     &              next_file(1:lnblnk1(next_file)),
     &              '" does not exist. Ignoring it.'
                else
                  write(6,'(a,a,a)') '  file "',
     &              next_file(1:lnblnk1(next_file)),
     &              '" does not exist. Quiting now.'
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
      if( no7.gt.0 ) then
        call getarg(no7,arg)
        if( verbose ) write(6,'(a,a)') '  fortran output: ',
     &      arg(1:lnblnk1(arg))
        open(7,file=arg(1:lnblnk1(arg)))
      else
        if( verbose ) write(6,'(a)') '  fortran output: fort.7'
        open(7,file='fort.7',status='unknown')
      end if
      if( no8.gt.0 ) then
        call getarg(no8,arg)
        if( verbose )write(6,'(a,a)') '  mortlst output: ',
     &      arg(1:lnblnk1(arg))
        open(8,file=arg(1:lnblnk1(arg)),err=101)
      else
        if( verbose )write(6,'(a)') '  mortlst output: fort.8'
        open(8,file='fort.8',status='unknown')
      endif
      O(1)=1   ! hex/raw data unit number
      O(2)=8   ! listing unit number
      O(3)=7   ! fortran/hex output unit number
      if( verbose ) write(6,'(/a,$)') '  Mortran compiling ... '
      oflag = 8
      return

101   write(6,'(a,a,a)') ' file ',arg(1:lnblnk1(arg)),' does not exist!'
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
        if( lnblnk1(line).gt.80 ) then
          write(6,'(a,i5,a,a,a)')
     &      '*** Warning: line ',lineno,' in file ',
     &     the_file(1:lnblnk1(the_file)),' is longer than 80 chars!'
        end if
        ! remove tabs
        do j=1,lnblnk1(line)
          if( ichar(line(j:j)).eq.9) line(j:j)=' '
        end do
        !write(iu,'(a)') line(1:lnblnk1(line))
        !On AIX the above fails with I/O error if line is blank
        if(lnblnk1(line).gt.0) then
          write(iu,'(a)') line(1:lnblnk1(line))
        else
          write(iu,*)
        end if
      end do
101   continue
      close(99)
      return

100   continue
      write(6,'(a,a)') 'add_file: could not open file ',
     &  the_file(1:lnblnk1(the_file))
      call exit(4) !stop 4
      end

      integer*4 function lnblnk1(c)
      character c*(*)
      integer*4 j
      do j=len(c),1,-1
        if( c(j:j).ne.' ' ) then
          lnblnk1 = j
          return
        end if
      end do
      lnblnk1=0
      return
      end
