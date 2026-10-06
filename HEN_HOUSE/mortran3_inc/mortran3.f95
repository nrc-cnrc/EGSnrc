
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
!  are just going to maintain the bigger version, now called mortran3.f95
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
!  Development version of mortran3.f95, the "frozen" egs Fortran implementation
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
! unit(9)_ano(10)_lvl(11)_crs(12)_mdef(13)_cset(14)_frt(15)
! labl(16)_holr(17)_indf(18)_seqn(19)_nflg(20)_list(21)_mcom(22)
! nest(23)_nscn(24)_pcom(25)_lazy(26)_mrkl(27)_stng(28)_mtrc(29)
! seqf(30)_dflt(31)_base(32)_blgr(33)_free(34)_mrkf(35)_tmx(36)
! nmx(37)_rmx(38)_indc(39)_mxx(40)_sign(41)_sigr(42)_sigt(43)
! isk(44)_jsk(45)_ksk(46)_lsk(47)_msk(48)_nsk(49)_usk(50)
! ysk(51)_eof(52)_indm(53)_typ(54)_qfl(55)_mlst(56)_flst(57)
! ifl(58)_mode(59)_locs(60)_left(61)_mcnt(62)_merr(63)_kfe(64)
! kfb(65)_kte(66)_ktb(67)_ngn(68)_sq(69)_eol(70)_geo(71)
! xtr(72)_xtr(73)_xtr(74)_icur(75)_bla(76)_bat(77)_blb(78)
! trm(79)_lpr(80)_rpr(81)_lsb(82)_rsb(83)_lcb(84)_rcb(85)
! bq(86)_dq(87)_rtt(88)_ltt(89)_ded(90)_nil(91)_lns(92)
! rns(93)_rif(94)_lif(95)_rdq(96)_ldq(97)_rmb(98)_lmb(99)
! zro(100)_alt(101)_rng(102)_lip(103)_rip(104)_lq(105)_rq(106)
! ga(107)_gp(108)_lgb(109)_rgb(110)_nat(111)_nlb(112)_xtr(113)
! mf1(114)_mf2(115)_mf3(116)_mf4(117)_mf5(118)_mf6(119)_mf7(120)
! _mf8(121)_mf9(122)_mfa(123)_mfb(124)_mfc(125)_mfd(126)
! node4_nsm100_slop20_nsx120_bff132_mxu600000_mnxbf450741
! home525371_evlim1000_mxerr100_g1157_g0156_g2158_g11167
! g13169_g14170_r1325_r0324_r80404_t1457_t0456
! _t2458_t72528_w1175_w0174_w7181_w72246_w73247_w80254
! sreg1_ureg121_gbf157_wbf175_rbf325_tbf457_iset589
! oset689_morf789_trt889_0ct1145_1ct1265_2ct1385_3ct1505
! 4ct1625_5ct1745_6ct1865_7ct1985_isk2105_jsk2155_ksk2205
! _lsk2255_msk2305_nsk2355_usk2455_ysk2505_pjt2555_mbf2655
! -----------------------------------------------------------------------------
!
!
!##############################################################################


      implicit none
      include "mortran3_indices.f95"
      dimension o(ox_capacity)
      common /ibm/o
      integer o
      integer o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
      equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
                  (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
      integer o_c,o_mxc,o_m
      integer iw,a,d,mxd,kd,mq,f,t,ip,iq,n,n1,n2,n3,kc,kb,kp,p,q,ia, &
              ire,nb,kx,y,nerd
      integer ispec,linf
      equivalence(o(ox_crs),o_c),(o(ox_sigt),o_mxc),(o(ox_mcom),o_m)
!
      common/nrcc/ oflag
      integer      oflag

      save    iw,a,d,mxd,kd,mq,f,t,ip,iq,n,n1,n2,n3,kc,kb,kp,p,q,ia, &
              ire,nb,kx,y,nerd

      call open_units

      write(o_e,'(a,T55,$)') 'Mortran 3.2',' '
      call egs_fdate(o_e)
      write(o_e,*)

      o_v=100
      o_u=1000
      call inital(1)
      iw=450741
      o_x=o_w
      go to 10
 30 if (o_s>o_u) then
         if (o(ox_nest)>0) then
            call expand(-1)
            go to 30
         end if
         o_s=525371
         o_u=o_s-1
         go to 10
      end if
      o_m=o(ox_base)+o(o_s)
      macro_scan: do
      if(o(o_m)==0)exit
      o_m=o(o_m)
      d=o_m+4
      mxd=o(o_m+3)
      kd=0
      o(ox_sq)=0
      mq=0
      f=0
      a=0
      t=o_s
      o_v=o_s
      if(o(ox_mtrc)>=4) call mactrc(2,d,mxd)
      if(o(ox_mtrc)>=4) call mactrc(5,o_v,o_u+1)
      token_scan: do
         if(o_v>o_u)exit token_scan
         if (o(o_v)==o(ox_ldq)) then
            o_v=o_v+o(o_v+1)+3
            cycle token_scan
         end if
         if (mq/= 0) go to 140
            if (o(d)==o(ox_lcb)) then
               if (o(d+1)<=100.and.o(ox_base)>1505) then
                  o(o_s-2*(a+1)-1)=o_v
                  o(o_s-2*(a+1)-2)=o_v
                  d=d+1
                  mq=1
                  if(o(d-3)/= o(ox_ga))f=0
                  ip=d
                  iq=d
                  do
                     if(o(iq)==o(ox_rcb))exit
                     iq=iq+1
                     if(iq<=mxd)cycle
                     call mesage(14, 20, 0,0)
                     cycle macro_scan
                  end do
                  cycle token_scan
               end if
            end if
            if (o(d)/= o(o_v).or.o(o_v)>=o(ox_ga)) go to 230
               if( o(ox_lq) ==o(d))o(ox_sq)=o(ox_sq)+1
               if( o(ox_rq) ==o(d))o(ox_sq)=o(ox_sq)-1
               if(o(d)==o(ox_lmb))kd=1
               if(o(d)==o(ox_rmb))kd=0
               o_v=o_v+1
               t=o_v
 240          d=d+1
               if(d>=mxd) then
                  call cout()
                  call expand(a)
                  go to 30
               end if
               cycle token_scan
 230       continue
            if (o(o_v)==o(ox_bla).and.o(ox_sq)+kd==0) then
                  o_v=o_v+1
                  cycle token_scan
            end if
            if (o(d)==o(ox_ga)) then
               d=d+1
               n=o(d)
               n1=n/4
               n2=(n-4*n1)/2
               n3=n-4*n1-2*n2
            else if (o(d)==o(ox_gp)) then
               n1=1
               n2=1
               n3=o(ox_mode)
            else
               go to 130
            end if
            t=o_v
            o(o_s-2*(a+1)-1)=o_v
            o(o_s-2*(a+1)-2)=o_v
            a=a+1
            d=d+1
            f=d
            kc=0
            kb=0
            kp=0
            cycle token_scan
 140       p=ip
            q=iq
            ia=o_v
            if(o(ox_sq)/= 0)ia=0
            ire=ispec(p,q,o_v,o_u,ia)
            if(ire==1)exit token_scan
            if (ire==3) then
               a=a+1
               if(f/= 0)o(o_s-2*(a-1)-2)=t
               if(ia/= 0)o(o_s-2*(a)-1)=ia
               o(o_s-2*(a)-2)=o_v
               d=q
               mq=0
               go to 240
            end if
 130    continue
         if (o(o_v)==o(ox_bla).and.mq==1) then
            o_v=o_v+1
            cycle token_scan
         end if
                  if(f==0)cycle macro_scan
         do
                        if(o(t)==o(ox_rq))cycle macro_scan
            if (kd>0) then
            do
               if( o(ox_lmb) ==o(t))kd=kd+1
               if( o(ox_rmb) ==o(t))kd=kd-1
               if(kd==0)exit
               t=t+1
               if(t>o_u)cycle macro_scan
            end do
            exit
            end if
            if (o(t)==o(ox_lq)) then
               nb=1
               do
                  if(nb==0)exit
                  t=t+1
                  if(t>o_u)cycle macro_scan
                  if( o(ox_lq) ==o(t))nb=nb+1
                  if( o(ox_rq) ==o(t))nb=nb-1
              end do
            end if
            if (o(ox_sq)==0) then
               if( o(ox_lcb) ==o(t))kc=kc+1
               if( o(ox_rcb) ==o(t))kc=kc-1
               if(kc<0)cycle macro_scan
               if (n1==1) then
                  if(38==o(t))kp=kp+1
                  if(46==o(t))kp=kp-1
                  if(kp<0)cycle macro_scan
               end if
               if (n2==1) then
                  if( o(ox_lsb) ==o(t))kb=kb+1
                  if( o(ox_rsb) ==o(t))kb=kb-1
                  if(kb<0)cycle macro_scan
               end if
               if (o(t)==o(ox_trm).and.n3>0)cycle macro_scan
            end if
            t=t+1
            if (t>o_u) then
               if(o(ox_frt)==1)cycle macro_scan
               go to 10
            end if
            if (kc+kp+kb/= 0) cycle
            if(mq==0)d=f
            exit
         end do
         o_v=t
         o(o_s-2*(a)-2)=o_v
      end do token_scan
      if(o(ox_frt)==1)cycle macro_scan
      if (o_s==o_u) then
         o_s=525371
         o(o_s)=o(o_u)
         o_u=o_s
      end if
 10 o(ox_qfl)=o(ox_bla)
      kx=0
      do
         o_u=o_u+1
         if(o_u>ox_capacity)call mesage(15, 1, 0,0)
         o(o_u)=o(o_c)
         o_c=o_c+1
         if(o_c>o(ox_sigt))call nxtcrd()
         if (o(o_u)==o(ox_lcb)) then
            o(ox_qfl)=o(o_u)
            kx=1
            do
               if(kx==0)exit
               o_u=o_u+1
               if(o_u>ox_capacity)call mesage(15, 1, 0,0)
               o(o_u)=o(o_c)
               o_c=o_c+1
               if(o_c>o(ox_sigt))call nxtcrd()
               if( o(ox_lcb) ==o(o_u))kx=kx+1
               if( o(ox_rcb) ==o(o_u))kx=kx-1
            end do
            o(ox_qfl)=o(ox_bla)
         end if
         if (o(o_u)==o(ox_dq)) then
         do
            if(o(ox_lazy)/= 1)o(ox_qfl)=o(ox_dq)
            o(o_u)=o(ox_ldq)
            o_u=o_u+1
            y=o_u
            do
               if(o(o_c)==o(ox_dq).or.o_c>o(ox_sigt))exit
               o_u=o_u+1
               if(o_u>ox_capacity)call mesage(15, 1, 0,0)
               o(o_u)=o(o_c)
               o_c=o_c+1
            end do
            o_u=o_u+1
            o(o_u)=o(ox_rdq)
            o(y)=o_u-y-1
            if (o(y)==0) then
               o_u=o_u-2
               o(o_u)=o(ox_bla)
            end if
            if (o_c<=o_mxc) then
               o(ox_qfl)=o(ox_bla)
               o_c=o_c+1
               if(o_c>o(ox_sigt))call nxtcrd()
               go to 10
            else if (o(ox_lazy)==1) then
               o(ox_qfl)=o(ox_bla)
               call nxtcrd()
               go to 10
            else
               call nxtcrd()
               o_u=o_u+1
               cycle
            end if
         end do
         end if
         if (o(o_u)==o(ox_bq).and.o(ox_stng)>0) then
            o(ox_qfl)=o(o_u)
            o(o_u)=o(ox_lq)
            do
               o_u=o_u+1
               if(o_u>ox_capacity)call mesage(15, 1, 0,0)
               o(o_u)=o(o_c)
               o_c=o_c+1
               if(o_c>o(ox_sigt))call nxtcrd()
               if (o(o_u)/= o(ox_bq))cycle
               if (o(o_c)==o(ox_bq)) then
                  o_c=o_c+1
                  if(o_c>o(ox_sigt))call nxtcrd()
                  cycle
               end if
               o(o_u)=o(ox_rq)
               o(ox_qfl)=o(ox_bla)
               exit
            end do
         end if
         if (o(o_u)==o(ox_bla)) then
            if (o_s/=o_u) then
               if(o(o_u-1)==o(ox_bla))o_u=o_u-1
            end if
            do
               if(o_c==o_mxc .or.o(o_c)/= o(ox_bla))exit
               o_c=o_c+1
            end do
         end if
         if (kx==0) then
            if(o(o_u)==o(ox_trm))go to 30
            if( o(ox_lsb) ==o(o_u))o(ox_lvl)=o(ox_lvl)+1
            if( o(ox_rsb) ==o(o_u))o(ox_lvl)=o(ox_lvl)-1
         end if
      end do
      end do macro_scan
      if (o(ox_nest)>0) then
         o_x=o_x+1
         if(o_x>o(ox_mxx))call mesage(15, 3, 0,0)
         o(o_x)=o(o_s)
         o_s=o_s+1
         if (o(o_x)==o(ox_lns).and.o(ox_base)/=1265) then
            do
               o_x=o_x+1
               if(o_x>o(ox_mxx))call mesage(15, 3, 0,0)
               o(o_x)=o(o_s)
               o_s=o_s+1
               if(o(o_x)==o(ox_rns))exit
            end do
         end if
         go to 30
      end if
      do
         if (o(o_s)==o(ox_trm)) then
            if (o_w>=iw) o_w=linf (iw,o_w,15)
         else if (o(o_s)==o(ox_ldq)) then
            if (o(ox_ngn)==0) then
               nerd=linf(o_s+2,o_s+o(o_s+1)+1,12)
               o_s=o_s+o(o_s+1)+2
            end if
         else if (o(o_s)==o(ox_lns)) then
            o(ox_nscn)=1
         else if (o(o_s)==o(ox_rns)) then
            o(ox_nscn)=0
         else if (o(ox_ngn)==0) then
            o_w=o_w+1
            o(o_w)=o(o_s)
            if (o(o_w)==o(ox_lq)) then
               do
                  if(o(o_w)==o(ox_rq))exit
                  o_w=o_w+1
                  o_s=o_s+1
                  o(o_w)=o(o_s)
               end do
            end if
         end if
         o_s=o_s+1
         if(o_s>o_u .or. o(ox_nscn)==0)exit
      end do
      go to 30
      end
!_______________________________________________________________________
!__________________________________________
      integer function ispec(p,q,iv,u,t)
      implicit none
      include "mortran3_indices.f95"
      integer p,q,iv,u,t
      dimension o(ox_capacity)
      common /ibm/o
      integer o
      integer o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
      equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
                  (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
      integer mini
      integer c,m,b,v,iq,i,np,jmp,n,nul,w
      save    c,m,b,v,iq,i,np,jmp,n,nul,w
      c=0
      m=0
      b=0
      v=iv
      o(ox_jsk)=2155
!ADEFGHIJKLRSXYZ
      o(ox_ksk)=2205
         ispec_scan: do
      if(p<q) then
         if (o(v)==o(ox_bla)) then
            if (v==t) then
               v=v+1
               t=t+1
               if(v>u)then
                  ispec=1
                  return
               end if
               cycle ispec_scan
            end if
         end if
         if (o(p)<=100) then
            iq=q
            i=mini(p,q,np,1505)
            if(o(ox_mtrc)>=4) call mactrc(5,v,u)
            if(i==0)then
               ispec=2
               return
            end if
            o(ox_jsk)=o(ox_jsk)+1
            o(o(ox_jsk))=iq
            o(ox_jsk)=o(ox_jsk)+1
            o(o(ox_jsk))=np
            if(o(ox_jsk)>2204)call mesage(15, 19, 0,0)
            cycle ispec_scan
         end if
            if (o(p)>=o(ox_rq)) then
               call mesage(14, 19, p,q+11)
               ispec=2
               return
            end if
            p=p+1
            jmp=o(p-1)-100
            select case (jmp)
            case (1)
               if (m/= 0) then
                  n=1
                  do
                     if(n==0)exit
                     if( o(ox_lip) ==o(p))n=n+1
                     if( o(ox_rip) ==o(p))n=n-1
                     p=p+1
                     if(p>q)call mesage(14, 27, 0,0)
                  end do
                  p=p-1
               else
                  v=o(o(ox_ksk)-1)
               end if
            cycle ispec_scan
            case (3)
            o(ox_ksk)=o(ox_ksk)+1
            o(o(ox_ksk))=c
            o(ox_ksk)=o(ox_ksk)+1
            o(o(ox_ksk))=v
            o(ox_ksk)=o(ox_ksk)+1
            o(o(ox_ksk))=p
            if(o(ox_ksk)>2254)call mesage(15, 20, 0,0)
            c=0
            cycle ispec_scan
            case (4)
               p=p+2
               if (m==1) then
                  c=c+1
                  if (c<o(p-1)) then
                     p=o(o(ox_ksk))
                     cycle ispec_scan
                  end if
               end if
               if(c>=o(p-2))m=1
               if( m==0 ) v=o(o(ox_ksk)-1)
               if (o(ox_ksk)==2205) then
                  call mesage(14, 17, 0,0)
                  ispec=2
                  return
               end if
               nul=o(o(ox_ksk))
               o(ox_ksk)=o(ox_ksk)-1
               nul=o(o(ox_ksk))
               o(ox_ksk)=o(ox_ksk)-1
               c=o(o(ox_ksk))
               o(ox_ksk)=o(ox_ksk)-1
               go to 1300
            case (2)
            p=p+2
            m=0
            if (o(p-2)<=o(v).and.o(v)<=o(p-1)) then
               m=1
               v=v+1
               if(v>u)then
                  ispec=1
                  return
               end if
            end if
            go to 1300
            case (5)
            w=v
            m=1
            if (o(p)==o(ox_bla)) then
               do
                  if(o(w)/= o(ox_bla).or.o(p)/= o(ox_bla))exit
                  w=w-1
                  p=p+1
               end do
            end if
            do
               if(o(p)==o(ox_rq))exit
               if(o(p)/= o(w).or.w>u)m=0
               p=p+1
               w=w+1
            end do
            p=p+1
            if(m==1)v=w
            end select
 1300       if(m==1)cycle ispec_scan
            if(o(p)/= o(ox_alt) .and. o(p)/= o(ox_rip))then
               ispec=2
               return
            end if
         cycle ispec_scan
         end if
            if (o(ox_jsk)/=2155) then
         p=o(o(ox_jsk))
         o(ox_jsk)=o(ox_jsk)-1
         q=o(o(ox_jsk))
         o(ox_jsk)=o(ox_jsk)-1
            cycle ispec_scan
            end if
         exit ispec_scan
      end do ispec_scan
      iv=v
      ispec=3
      return
      end function ispec
!_______________________________________________________________________
!__________________________________________
      integer function mini(s,u,w,kct)
      implicit none
      include "mortran3_indices.f95"
      integer s,u,w,kct
      dimension o(ox_capacity)
      common /ibm/o
      integer o
      integer o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
      equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
                  (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
      integer m,l,v,p,q
      save    m,l,v,p,q
      mini=0
      m=kct+o(s)
!ABCDEFGHIJKNRTXYZ
      l=u-s
      do
         if (o(m)==0) then
            call mesage(14, 15, s,u+1)
            return
         end if
         m=o(m)
         v=s
         p=m+4
         q=o(m+3)
!   IF Q-PnL jM;
         if(o(ox_mtrc)>=6) call mactrc(2,p,q)
         do while (p<q)
            if(o(p)/= o(v))exit
            p=p+1
            v=v+1
         end do
         if (p>=q) then
         s=o(m+2)+4
         u=o(o(m+2)+3)
         mini=1
         w=v
            return
         end if
      end do
      end function mini
!_______________________________________________________________________
!________________________________________
      subroutine expand (f)
      implicit none
      include "mortran3_indices.f95"
      integer  f
      dimension o(ox_capacity)
      common /ibm/o
      integer o
      integer o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
      equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
                  (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
      integer lexp,llong
      integer kgn,t,sef,k,j,jv,p,q,nul,jmp,a,n,krap,kraq,z,y
      data kgn/0/
      save    kgn,t,sef,k,j,jv,p,q,nul,jmp,a,n,krap,kraq,z,y
!CDGHILM
      t=0
      if (f>=0) go to 1440
         o(ox_nest)=o(ox_nest)-1
         if(o(ox_nest)==0)o(ox_frt)=sef
         do k = 1,6
            o(k)=o(6+o_u+1-k)
         end do
         j=o(o(o(ox_ysk)))
         if (j>=11 .and. j<=12) then
            jv=lexp(o_w+1,o_x)
            if (o(ox_ifl)==0) then
               if (j==12) then
                  o_x=o_w+llong(o_w,jv,10,0)
               end if
               if (j==11) then
                  o_x=o_w+1
                  o(o_x)=jv
               end if
            else
               o_x=o_w
               o(ox_ifl)=o(ox_ifl)-1
               if (jv==0) then
                  p=o(ox_kfb)
                  q=o(ox_kfe)
               else
                  p=o(ox_ktb)
                  q=o(ox_kte)
               end if
               o(ox_isk)=o(ox_isk)+1
               o(o(ox_isk))=o(ox_kfe)+1
               o(ox_isk)=o(ox_isk)+1
               o(o(ox_isk))=o_e
               if(o(ox_isk)>2154)call mesage(15, 18, 0,0)
               o_b=p
               o_e=q
            end if
               end if
         nul=o(o(ox_ysk))
         o(ox_ysk)=o(ox_ysk)-1
         o_w=o(o(ox_ysk))
         o(ox_ysk)=o(ox_ysk)-1
         o(ox_base)=o(o(ox_ysk))
         o(ox_ysk)=o(ox_ysk)-1
         go to 1430
 1440    o(o_s-1)=f
         o(ox_mxx) = o_s-2*f-3
         o_r=o(o(ox_mcom)+2)
         o_b=o_r+4
         o_e=o(o_r+3)
         o_r=o(o_r)
         o(ox_mcnt)=o(ox_mcnt)+1
         if (o(ox_mcnt)>1000) then
            call mesage(14, 4, 0,0)
            o(ox_mtrc)=2
         end if
         if(o(ox_mtrc)>=2) call mactrc(22,o(ox_mcom),o_s)
         if( o(ox_nest)==0 )o_x=o_w
 1430 continue
 1580 continue
      do while (o_b<o_e)
         if (o(o_b)<o(ox_ga)) then
               o_x=o_x+1
               if(o_x>o(ox_mxx))call mesage(15, 3, 0,0)
               o(o_x)=o(o_b)
               o_b=o_b+1
               cycle
         end if
            o_b=o_b+1
            jmp=o(o_b-1)-o(ox_rq)
            select case (jmp)
            case (1)
               go to 1620
            case (2)
               go to 1630
            case (3)
               go to 1640
            case (4)
               go to 1650
            case default
               go to 1630
            end select
 1630       if (o(o_b)>=1 .and. o(o_b)<=o(o_s-1)) then
               a=o(o_s-2*(o(o_b))-1)
               n=o(o_s-2*(o(o_b))-2)
               do while (a<n)
                  o_x=o_x+1
                  if(o_x>o(ox_mxx))call mesage(15, 3, 0,0)
                  o(o_x)=o(a)
                  a=a+1
                  end do
               go to 1690
            else
            call mesage(15, 4, o_b,o_e)
            end if
 1620       if (o(o_b)==12) then
               o_b=o_b+1
               if (o(o_b)==23) then
                  if (o(ox_ngn)==0) then
                     kgn=1
                     o(ox_ngn)=1
                  else
                     kgn=kgn+1
                  end if
               else if (o(o_b)==16) then
                  if (o(ox_ngn)==1) kgn=kgn+1
               else if (o(o_b)==14) then
                  kgn=kgn-1
                  if(kgn<=0)o(ox_ngn)=0
                  end if
               if (o(ox_geo)/=0) then
                  if(o(ox_ngn)==0)o(ox_base)=1625
                  if(o(ox_ngn)/= 0)o(ox_base)=1145
               end if
            else
            if(o(ox_ngn)==0)call kat
            end if
            go to 1690
 1640       if (o(o_b)==10) then
               o_b=o_b+1
               if(1>o(o_b).or.o(o_b)>o(o_s-1) )call mesage(15, 4, o_b,o_e)
               o_x=o_x+llong(o_x,o(o_s-2*(o(o_b))-2)-o(o_s-2*(o(o_b))-1),10,0)
               if(o_x>o(ox_mxx))call mesage(15, 3, 0,0)
               o_b=o_b+2
               cycle
            else if (o(o_b)==13) then
               o_b=o_b+1
               t=1
               do
                  if( o(ox_lgb) ==o(o_b))t=t+1
                  if( o(ox_rgb) ==o(o_b))t=t-1
                  if(t==0)exit
                  o_x=o_x+1
                  if(o_x>o(ox_mxx))call mesage(15, 3, 0,0)
                  o(o_x)=o(o_b)
                  o_b=o_b+1
               end do
               go to 1690
            else if (o(o_b)==15) then
               o_x=o_x+1
               o(o_x)=o(ox_ldq)
               o_x=o_x+1
               krap=o_x
               kraq=o(ox_nest)
               go to 1690
            else
            o(ox_nest)=o(ox_nest)+1
            if(o(ox_nest)==1)sef=o(ox_frt)
            o(ox_frt)=1
            o(ox_ysk)=o(ox_ysk)+1
            o(o(ox_ysk))=o(ox_base)
            o(ox_ysk)=o(ox_ysk)+1
            o(o(ox_ysk))=o_w
            o(ox_ysk)=o(ox_ysk)+1
            o(o(ox_ysk))=o_b
            if(o(ox_ysk)>2554)call mesage(15, 34, 0,0)
            o_w=o_x
            go to 1690
            end if
 1650       if (krap/=0 .and. kraq==o(ox_nest)) then
               o_x=o_x+1
               o(o_s)=o(ox_rdq)
               o(krap)=o_x-krap-1
               krap=0
               cycle
            end if
            do k = 1,6
               o(o(ox_mxx)-k)=o(k)
            end do
            o_u=o(ox_mxx)-7
            o_v=o_u+1
            z=o(o(ox_ysk))
            y=o(z)
            if(o(z+2)==o(ox_lq))y=0
            if((9<y).and.(y<15))y=o(ox_dflt)
            if( 0>y.or.y>=8 )call mesage(15, 17, o_b,o_e)
            o(ox_base)=y*120+1145
            exit
 1690       o_b=o_b+1
         cycle
      end do
         if (o(ox_nest)<=0) then
            if (o(ox_isk)==2105) then
               if (o_r>0) then
                  o_b=o_r+4
                  o_e=o(o_r+3)
                  o_r=o(o_r)
                  go to 1580
               end if
            else
               o_e=o(o(ox_isk))
               o(ox_isk)=o(ox_isk)-1
               o_b=o(o(ox_isk))
               o(ox_isk)=o(ox_isk)-1
               go to 1580
            end if
         end if
      if(o(ox_mtrc)>=2) call mactrc(33,o_w,o_x)
      do while (o_x>o_w)
         o_v=o_v-1
         o(o_v)=o(o_x)
         o_x=o_x-1
      end do
      o_s=o_v
       return
      end subroutine expand
!TO EXPAND
!_______________________________________________________________________
!________________________________________
      subroutine kat
      implicit none
      include "mortran3_indices.f95"
      dimension o(ox_capacity)
      common /ibm/o
      integer o
      integer o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
      equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
                  (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
      integer l,m
      equivalence (o(ox_lsk),l),(o(ox_msk),m)
      integer llong,nib
      integer jmp,t,ibg,z,iuni,nba
      save    jmp,t,ibg,z,iuni,nba
!FGHIJKLPQ  YZ
      if(o(o_b)<19.or.o(o_b)>30) call mesage(15, 7, o_b-2,o_e)
      o_b=o_b+1
      jmp=o(o_b-1)-18
      select case (jmp)
      case (1)
         t=o(o_b)+78
         o_x=o_x+1
         o(o_x)=o(t)
         return
      case (2)
         call krunc(0)
         return
      case (3)
         if (o(o_b)==12) then
            o_x=o_x+llong(o_x,o(l-o(o_b+1))+o(o_b+2),10,1)
            o_b=o_b+2
            return
         else if (o(o_b)==16) then
            o(ox_labl)=o(ox_labl)+10
            o_x=o_x+llong(o_x,o(ox_labl),10,0)
            return
         else if (o(o_b)==28) then
            o_b=o_b+1
            o(ox_lsk)=o(ox_lsk)+1
            o(o(ox_lsk))=nib(o_x,10)
            if(o(ox_lsk)>2304)call mesage(15, 21, 0,0)
            if (o(o_b)==1) then
               z=o(l-1)
               o(l-1)=o(l)
               o(l)=z
            end if
         else if (o(o_b)==30) then
            o_b=o_b+1
            if(o(o_b)==1)o(l-1)=o(l)
            l=l-1
            if (l<2255) then
               call mesage(14, 7, 0,0)
               l=l+1
            end if
         else
            call mesage(15, 7, o_b-2,o_e)
            return
         end if
         if(o(o_b)/=1.and.o(o_b)/=0)then
            write(6,*) 'compilation error (stop 18)'
            call exit(18)
         end if
         return
      case (4)
         if (o(o_b)==28) then
            o_b=o_b+1
            o(ox_msk)=o(ox_msk)+1
            o(o(ox_msk))=o(o_b)
            if(o(ox_msk)>2354)call mesage(15, 22, 0,0)
         else if (o(o_b)==27) then
            o_b=o_b+1
            o(m)=o(o_b)
         else if (o(o_b)==30) then
            o_x=o_x+1
            o(o_x)=o(m)
            m=m-1
            if (m<2305) then
               call mesage(14, 8, 0,0)
               m=m+1
            end if
         else
            call mesage(15, 7, o_b-2,o_e)
         end if
         return
      case (5)
         ibg=o(o_x)
         o_x=o_x-1+llong(o_x-1,ibg,10,1)
         return
      case (6)
         call define(o_s)
         return
      case (7)
         o(ox_ifl)=1
         o_b=o_b+4
         o(ox_ktb)=o_b+o(o_b-1)
         o(ox_kte)=o_b+o(o_b-2)
         o(ox_kfb)=o_b+o(o_b-3)
         o(ox_kfe)=o_b+o(o_b-4)
         o_b=o_b-1
         return
      case (8)
         o(ox_labl)=o(ox_labl)+10
         o(ox_lsk)=o(ox_lsk)+1
         o(o(ox_lsk))=o(ox_labl)
         if(o(ox_lsk)>2304)call mesage(15, 21, 0,0)
         if (o(o_b)==1) then
            z=o(l-1)
            o(l-1)=o(l)
            o(l)=z
         end if
         if(o(o_b)/=1.and.o(o_b)/=0)then
            write(6,*) 'compilation error (stop 18)'
            call exit(18)
         end if
         return
      case (9)
         iuni=o(o_b)
         rewind iuni
         return
      case (10)
         t=o(o_b)
         o_b=o_b+1
         if (0<o(o_b).and.o(o_b)<5) then
            t=t+26*o(o_b)
            o_b=o_b+1
         end if
         nba=0
      case (11)
         if(o(ox_mtrc)>=1) call mactrc(22,o(ox_mcom),o_s)
         o_b=o_b-1
         return
      case (12)
         t=121+o(o_b)
         o_b=o_b+1
         nba=o(o_x)
         o_x=o_x-1
      case default
         call mesage(15, 7, o_b-2,o_e)
         return
      end select
      if (o(o_b)==43) then
         o(t)=o(t)+1
      else if (o(o_b)==42) then
         o(t)=o(t)-1
      else if (o(o_b)==41) then
         o(t)=o(o_x)
         o_x=o_x-1
      else if (o(o_b)==12) then
         o_x=o_x+1
         o(o_x)=o(t)
         if (nba>1) o_x=o_x-1+llong(o_x-1,o(t),nba,1)
      else
         call mesage(15, 7, o_b-3,o_e)
      end if
      return
      end subroutine kat
!_______________________________________________________________________
!__________________________________________
      subroutine cout
      implicit none
      include "mortran3_indices.f95"
      dimension o(ox_capacity)
      common /ibm/o
      integer o
      integer o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
      equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
                  (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
      integer linf
      integer t,nerd
      save    t,nerd
      t=o_s
      do
         if(t>o_v)exit
         if (o(t)==o(ox_ldq)) then
            nerd=linf(t+2,t+o(t+1)+1,12)
            t=t+o(t+1)+2
         end if
         t=t+1
      end do
      return
      end subroutine cout
!_______________________________________________________________________
!________________________________________
      subroutine define(s)
      implicit none
      include "mortran3_indices.f95"
      integer s
      dimension o(ox_capacity)
      common /ibm/o
      integer o
      integer o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
      equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
                  (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
!GHQUVWXYZ
      integer   l,k,a,t,n,z,b,e,m,p,lp,c,need,j,f,i,r
      logical found
      save      l,k,a,t,n,z,b,e,m,p,lp,c,need,j,f,i,r
      dimension l(3)
      if (o(s-2*(1)-1)==o(s-2*(1)-2)) then
         call mesage(14, 1, 0,0)
         return
      end if
      o(ox_typ)=o(ox_dflt)
      if ( o(s-2*(2)-1)/= o(s-2*(2)-2) )o(ox_typ)=o(o(s-2*(2)-1))
      do k = 1,3
         a=o(s-2*(k)-1)
         t=o(s-2*(k)-2)
         n=a-1
         z=0
          do while (a<=t)
            n=n+1
            o(n)=o(a)
            a=a+1
            if (o(n)==o(ox_bq)) then
               if (k==3.or.o(ox_stng)>0) then
                  if (z==0) then
                     o(n)=o(ox_lq)
                     z=1
                  else if (o(a)==o(ox_bq).and.a<=t) then
                     a=a+1
                  else
                     o(n)=o(ox_rq)
                     z=0
                  end if
               end if
            else if (o(n)==o(ox_bla)) then
                  if (z==0) then
                     do while (o(a)==o(ox_bla).and.a<t)
                        a=a+1
                     end do
                  end if
               else if (o(n)==o(ox_blb)) then
                  if (o(a)==o(ox_blb)) then
                     a=a+1
                  else
                     o(n)=o(ox_gp)
                  end if
               else if (o(n)==o(ox_bat)) then
                  if (o(a)==o(ox_bat)) then
                     a=a+1
                  else
                     o(n)=o(ox_ga)
                  end if
               end if
            if (o(ox_blgr)==0 .and. k==3) then
               if(o(n)==o(ox_lcb))o(n)=o(ox_lgb)
               if(o(n)==o(ox_rcb))o(n)=o(ox_rgb)
            end if
          cycle
          end do
         o(s-2*(k)-2)=n
         l(k)=o(s-2*(k)-2)-o(s-2*(k)-1)
         end do
      if (o(ox_typ)>=12 .and. o(ox_typ)<=14) then
         b=o(s-2*(1)-1)
         e=o(s-2*(1)-2)
         if (o(ox_typ)==14) then
            m=1505+o(b)
         else
            m=1625+o(b)
         end if
         found=.false.
         define_lookup: do
            if (o(m)==0) then
               if (o(ox_typ)==12) then
                  o(ox_typ)=o(ox_dflt)
                  exit define_lookup
               end if
               call mesage(14, 2, 0,0)
               return
            end if
            m=o(m)
            p=m+4
            lp=o(m+3)-p
            if (lp/=l(1)) cycle define_lookup
            a=b
            do while (o(p)==o(a))
               p=p+1
               a=a+1
               if (a>=e) then
                  found=.true.
                  exit define_lookup
               end if
            end do
         end do define_lookup
         if (found) then
            if (o(ox_typ)/=12) then
               if(o(m)/= 0)o((o(m))+1)=o(m+1)
               o(o(m+1))=o(m)
               c=o(m+2)
               o(m+2)=o(ox_ded)
               do while (c/=0)
                  o(c+2)=o(ox_ded)
                  c=o(c)
               end do
               if(o(ox_mdef)>0)call mactrc(13,0,s)
               return
            end if
            need=4+l(3)
            if(need>450740-o(ox_free)) call krunc(need)
            c=o(m+2)
            do while (o(c)/=0)
               c=o(c)
            end do
            o(c)=o(ox_free)
            o(o(ox_free))=0
            o(o(ox_free)+1)=c
            o(o(ox_free)+2)=o(ox_nil)
            o(o(ox_free)+3)=o(ox_free)+need
            o(ox_free)=o(ox_free)+need
            a=o(s-2*(3)-2)
            k=l(3)
            do j = 1,k
               o(o(ox_free)-j)=o(a-j)
            end do
            if(o(ox_mdef)>0)call mactrc(12,0,s)
            return
         end if
      end if
      need=l(1)+l(3)+2*4
      if (need>450740-o(ox_free)) call krunc(2+need)
      a=o(s-2*(1)-1)
      z=a+l(1)-1
      p=o(ox_free)
      f=p+4
      i=f
       do while (a<=z)
         o(f)=o(a)
         f=f+1
         a=a+1
       end do
      if (o(i)==o(ox_lcb) .and. o(ox_typ)>2) then
         call mesage(14, 21, o(s-2*(1)-1),o(s-2*(1)-2))
         return
      end if
      if (o(i)>o(ox_lq)) then
         call mesage(14, 9, o(s-2*(1)-1),o(s-2*(1)-2))
         return
      end if
      if (o(ox_typ)<0 .or. o(ox_typ)>7) then
         call mesage(14, 12, o(s-2*(1)-1),o(s-2*(1)-2))
         return
      end if
      i=o(i) + 1145 + o(ox_typ)*120
      o(p)=o(i)
      if(o(p)/= 0)o((o(p))+1)=p
      o(p+1)=i
      o(i)=p
      o(p+2)=f
      o(p+3)=f
      o(f)=0
      o(f+1)=p+2
      o(f+3)=f+4+l(3)
      o(f+2)=o(ox_nil)
      r=f
      f=p+need
      e=o(s-2*(3)-2)
      k=l(3)
      if (k/=0) then
         call doit(e-k,e)
         do j = 1,k
            o(f-j)=o(e-j)
         end do
      end if
      if (o(ox_mdef)>0) call mactrc(15,p,r)
      o(ox_free)= p +need
      o(ox_locs)=450740-o(ox_free)
      o(ox_left)=(100*o(ox_locs))/(450740-2655)
 return
      end subroutine define
!_______________________________________________________________________
!__________________________________________
      subroutine doit(a,z)
      implicit none
      include "mortran3_indices.f95"
      integer a,z
      dimension o(ox_capacity)
      common /ibm/o
      integer o
      integer o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
      equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
                  (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
      integer   s,x,t,p
      dimension s(200)
      save      s,x,t,p
      x=0
!BCDEFGHIJKLMNQRUVWY
      t=z
      do while (t>=a)
         if (o(t)==o(ox_lif)) then
            if (o(t-1)==o(ox_rif)) then
               s(x+1)=t+1
               s(x+2)=o(t)
               x=x+2
               t=t-1
            else
               s(x+1)=t+1
               x=x+1
               do
                  if(o(t)==o(ox_ga).and.o(t+1)==25)exit
                  t=t-1
                  if(t<a)return
               end do
               p=t+6
               o(p-1)=s(x)-p
               x=x-1
               if (s(x)==o(ox_lif)) then
                  o(p-4)= s(x-2)-p
                  o(p-3)= s(x-1)-p
                  o(p-2)= o(p-3)-2
                  x=x-3
               else
                  o(p-2)=s(x)-p
                  o(p-3)=s(x)-p
                  o(p-4)=s(x)-p
                  x=x-1
               end if
            end if
         else if (o(t)==o(ox_rif)) then
            s(x+1)=t
            x=x+1
         end if
         t=t-1
      end do
      return
      end subroutine doit
!_______________________________________________________________________
!__________________________________________
      subroutine krunc(r)
      implicit none
      include "mortran3_indices.f95"
      integer    r
      dimension o(ox_capacity)
!ABEFGHIJKLQTUVWXYZ
      common /ibm/o
      integer o
      integer o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
      equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
                  (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
      integer m,n,d,p,s,c,i
      save    m,n,d,p,s,c,i
      if (o(ox_mrkf)/=0) then
         m=o(ox_mrkf)
         do while (m/=o(ox_free))
            if (o(m+2)>=0) then
               if(o(m)/= 0)o((o(m))+1)=o(m+1)
               o(o(m+1))=o(m)
            end if
            o(m+2)=o(ox_ded)
            m=o(m+3)
         end do
      end if
      n=2655
      m=o(n+3)
      d=0
! PREDECESSOR  SUCCESSOR    CONCAT
      o(o(ox_free))=0
      do while (n/=o(ox_free))
         m=n
         n= o(m+3)
         p=o(m+1)
         s=o(m)
         c=o(m+2)
         if (c==o(ox_ded)) then
            d=d+ n-m
         else
            if (s>0) o(s+1)=o(s+1)-d
            if (c>0) o(n+1)=o(n+1)-d
            o(p)=o(p)-d
            o(m+3)=o(m+3)-d
!   MOVE IT
            do i = m,n
               o(i-d)=o(i)
            end do
         end if
      end do
      o(ox_free)=o(ox_free)-d
      o(o(ox_free))=0
      if (r>d)call mesage(15, 6, 0,0)
      o(ox_locs)=450740-o(ox_free)
      o(ox_left)=(100*o(ox_locs))/(450740-2655)
      call mesage(13,1,d,0)
       return
      end subroutine krunc
!_______________________________________________________________________
!__________________________________________
      integer function nib(l,b)
      implicit none
      include "mortran3_indices.f95"
      integer  l,b
      dimension o(ox_capacity)
      common /ibm/o
      integer o
      integer o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
      equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
                  (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
      integer m,n
      save    m,n
      m=1
      n=0
      do
         if(o(l)/= o(ox_bla))exit
         l=l-1
      end do
      do
         if(0>o(l).or.o(l)>=b)exit
         n=n+o(l)*m
         m=m*b
         l=l-1
      end do
      if (o(l)==42) then
         l=l-1
         n=-n
      end if
      nib=n
       return
      end function nib
!_______________________________________________________________________
!__________________________________________
      integer function llong(loc,num,b,ind)
      implicit none
      include "mortran3_indices.f95"
      integer  loc,num,b,ind
      dimension o(ox_capacity)
      common /ibm/o
      integer o
      integer o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
      equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
                  (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
      integer n,l,i,j,k
      save    n,l,i,j,k
      if(o(ox_ngn)/= 0)return
      n=iabs(num)
      l=loc
      i=n/b
      j=1
      if (ind== 0) then
         l=l+1
         o(l)=o(ox_bla)
      end if
      do while(i>0)
         i=i/b
         j=j+1
      end do
!COUNT DIGITS IN EXPANSION
      if (num<0) then
         l=l+1
         o(l)=42
      end if
      llong=l-loc+j
      do
         k=n/b
         o(l+j)=n-k*b
         n=k
         j=j-1
         if(j<=0)exit
      end do
 return
      end function llong
!_______________________________________________________________________
!________________________________________
      integer function lexp(a,z)
      implicit none
      include "mortran3_indices.f95"
      integer  a,z
      dimension o(ox_capacity)
      common /ibm/o
      integer o
      integer o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
      equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
                  (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
      integer   p,n
      dimension p(100)
!FGHIJKLMQRSUVWY
      equivalence(o(ox_nsk),n),( o(2555),p(1) )
      integer ispec
      integer b,c,nq,mxc,d,e,lone,mp,mq,mv,mu,mnn,t,x,k,jmp
      save    b,c,nq,mxc,d,e,lone,mp,mq,mv,mu,mnn,t,x,k,jmp
      if (o(a+1)==o(ox_lq)) then
         lexp=1
         b=a+1
         c=b+1
         nq=1
         mxc=z
         do
            if( o(ox_lq) ==o(c))nq=nq+1
            if( o(ox_rq) ==o(c))nq=nq-1
            if(nq==0)exit
            c=c+1
            if(c>mxc)call mesage(15, 35, 0,0)
         end do
         d=c+1
         e=d+1
         if (o(e)==o(ox_lq)) then
            do while(e<=z-1)
               if(o(b)/= o(e))lexp=0
               b=b+1
               e=e+1
            end do
            if(b/= d) lexp=0
         else
            lone=1
            mp=e
            mq=z
            mv=b+1
            mu=c
            if(ispec(mp,mq,mv,mu,lone)/= 3)lexp=0
         end if
         if(o(d)==89) lexp=1-lexp
         return
         end if
      lexp=0
      o(ox_nflg)=0
      n=2355
      mnn=n+2
      o(n-1)=38
      o(n)=0
      b=a
      do
         if(b>z)exit
         if (o(b)>=0 .and. o(b)<=9) then
            o(n)=0
            o(ox_nflg)=1
            do while(0<=o(b).and.o(b)<=9)
               o(n)=10*o(n)+o(b)
               b=b+1
            end do
            cycle
         end if
         if (o(b)==o(ox_bla)) then
            b=b+1
            cycle
         end if
         t=b
            if (o(t)>=10 .and. o(t)<=29) then
               x=o(t)*o(t+1)
               t=t+2
               if (o(t)==29.or.o(t)==13) then
                  x=x+o(t)
                  t=t+1
               end if
               x=x/20
               if (.not.((10<x).and.(x<33))) exit
            else
               x=o(t)
               t=t+1
               select case (x)
               case (45)
                  if (o(t)==45) then
                     t=t+1
                     x=36
                  end if
               case (89)
                  x=29
                  if (o(t)==41) then
                     t=t+1
                     x=16
                  end if
               case (85)
                  x=30
                  if (o(t)==41) then
                     t=t+1
                     x=14
                  end if
               case (41)
                  x=18
                  if (o(t)==86) then
                     t=t+1
                     x=11
                  end if
               case (86)
                  x=23
                  if (o(t)==41) then
                     t=t+1
                     x=11
                  end if
               case (87)
                  x=32
               case (88)
                  x=13
               end select
            end if
            if(p(x)==0)exit
            if(x<10.or.x>o(ox_rpr) .or. (x==o(ox_rpr).and.o(ox_nflg)==0)) &
               exit
            if (p(x)/100>p(o(n-1))/100 .or. x==o(ox_lpr)) then
               o(ox_nflg)=0
               o(ox_nsk)=o(ox_nsk)+1
               o(o(ox_nsk))=x
               o(ox_nsk)=o(ox_nsk)+1
               o(o(ox_nsk))=0
               if(o(ox_nsk)>2454)call mesage(15, 23, 0,0)
               b=t
               cycle
            end if
            if (n==mnn) then
               lexp=o(n)
               return
            end if
            k=p(o(n-1))
            o(ox_nflg)=1
            jmp=k-((k/100)*100)-1
            select case (jmp)
            case (1)
               o(n-2)=o(n)
               b=t
            case (2)
               if (o(n-2)==1 .or. o(n)==1) then
                  o(n-2)=1
               else
                  o(n-2)=0
               end if
            case (3)
               if (o(n-2)==1 .and. o(n)==1) then
                  o(n-2)=1
               else
                  o(n-2)=0
               end if
            case (4)
               o(n-2)=1-o(n)
            case (5)
               o(n-2)=o(n-2)-o(n)
            case (6)
               o(n-2)=o(n-2)+o(n)
            case (7)
               if (o(n)==0) then
                  call mesage(14, 10, 0,0)
                  exit
               end if
               o(n-2)=o(n-2)/o(n)
            case (8)
               o(n-2)=o(n-2)*o(n)
            case (9)
               o(n-2)=o(n-2)**o(n)
            case (10)
               if (o(n-2)>=o(n)) then
                  o(n-2)=1
               else
                  o(n-2)=0
               end if
            case (11)
               if (o(n-2)<=o(n)) then
                  o(n-2)=1
               else
                  o(n-2)=0
               end if
            case (12)
               if (o(n-2)/=o(n)) then
                  o(n-2)=1
               else
                  o(n-2)=0
               end if
            case (13)
               if (o(n-2)==o(n)) then
                  o(n-2)=1
               else
                  o(n-2)=0
               end if
            case (14)
               if (o(n-2)>o(n)) then
                  o(n-2)=1
               else
                  o(n-2)=0
               end if
            case (15)
               if (o(n-2)<o(n)) then
                  o(n-2)=1
               else
                  o(n-2)=0
               end if
            case default
               if (o(n)==0) then
                  call mesage(14, 10, 0,0)
                  exit
               end if
               o(n-2)=o(n-2)/o(n)
            end select
            n=n-2
      end do
   call mesage(14, 23, a,z+1)
 return
      end function lexp
!_______________________________________________________________________
!________________________________________
      subroutine nxtcrd
      implicit none
      include "mortran3_indices.f95"
      dimension o(ox_capacity)
      common /ibm/o
      integer o
      integer o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
      equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
                  (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
      integer mxr,mxt,mxn
      equivalence(o(ox_rmx),mxr),(o(ox_tmx),mxt),(o(ox_nmx),mxn)
      integer llong,linf
      integer k,j,w,i,l,m,y,nerd
      save    k,j,w,i,l,m,y,nerd
      o(ox_mcnt)=0
      if(o(ox_eof)==2) call mesage(16,1,0,0)
      do
      call rw(0,o(ox_unit),325,mxr)
      if (o(ox_eof)==1) then
         o(ox_eof)=0
         o(ox_unit)=o(o(ox_usk))
         o(ox_usk)=o(ox_usk)-1
         if (o(ox_unit)==0) then
            o(ox_eof)=2
            o(ox_frt)=1
            o(ox_crs)=o(ox_sigt)
            o(o(ox_crs))=o(ox_trm)
            return
         end if
      end if
      do k= 325,mxr
         o(k+132)=o(889+o(k))
      end do
      o(ox_seqn)=o(ox_seqn)+1
      o(ox_crs)=457
      o(157)=o(ox_bla)
      if (o(457) == 93) then
         if (o(458)==14) then
            o(157)=1
            o(325)=o(689+o(ox_bla))
            o(325+1)=o(325)
            o(ox_crs)=458+1
         else if (o(458)==93) then
               o(ox_eof)=1
               o(ox_frt)=1
         else if (o(458)>=10.and.o(458)<=30) then
               call cccard(o(458)-9)
               o(ox_crs)=o(ox_sigt)
               o(o(ox_crs))=o(ox_trm)
         end if
         if (o(ox_eof)==1.or.o(458)==22.or.o(458)==15) then
            o(ox_crs)=o(ox_sigt)
            o(o(ox_crs))=o(ox_trm)
            return
         end if
      end if
      if (o(ox_list)==1) then
         do j = 158,174
            o(j)=o(ox_bla)
         end do
         if (o(ox_seqf)==1) then
            j=llong(158,o(ox_seqn),10,0)
         else if (o(ox_seqf)==2) then
            do j = 1,8
               o(157+j)=o(o(ox_sigt)+j)
            end do
         end if
         if(o(ox_ngn)/= 0)o(167)=33
         o(169)=o(ox_qfl)
         j=llong(170,o(ox_lvl),10,0)
         if (o(ox_indm)==0) then
            w=174
            do k = 1,mxn
               w=w+1
               o(w)=o(456+k)
            end do
            call rw(18,o(ox_mlst),157,w)
         else
            i=mod(max0(o(ox_indm)*o(ox_lvl),0),90)
            l=457
            m=o(ox_sigt)
            do while(o(l)==o(ox_bla).and.l/=m)
               l=l+1
            end do
            do while(o(m)==o(ox_bla).and.l/=m)
               m=m-1
            end do
            do
               w=175
               do y = 1,i
                  o(w)=o(ox_bla)
                  w=w+1
               end do
               do while(w<=288.and.l<=m)
                  o(w)=o(l)
                  w=w+1
                  l=l+1
               end do
               call rw(18,o(ox_mlst),157,w-1)
               if(l>m)exit
               do k = 157,175
                  o(k)=o(ox_bla)
               end do
            end do
        end if
      end if
      if (o(ox_frt)==1) then
         if (o(457)/= 93) call rw(23,o(ox_flst),325,404)
      else if (o(ox_ano)==1) then
         nerd=linf(457,mxt,10)
      end if
      if(o(ox_frt)==1)cycle
      if (o(ox_cset)<2) return
      j=456
      k=457
      do while(k<=mxt)
         j=j+1
         o(j)=o(k)
         k=k+1
         if (o(j)==45) then
            if (o(k)==46) then
               o(j)=o(ox_rcb)
               k=k+1
            else if (o(k)==86) then
               o(j)=o(ox_rsb)
               k=k+1
            end if
         else if (o(k)==45) then
            if (o(j)==38) then
               o(j)=o(ox_lcb)
               k=k+1
            else if (o(j)==85) then
               o(j)=o(ox_lsb)
               k=k+1
            end if
         end if
      end do
      do while(j<=mxt)
         j=j+1
         o(j)=90
      end do
      exit
      end do
      return
      end subroutine nxtcrd
!_______________________________________________________________________
!__________________________________________
!_______________________________________________________________________
!________________________________________
      subroutine cccard(k)
      implicit none
      include "mortran3_indices.f95"
      integer k
      dimension o(ox_capacity)
      common /ibm/o
      integer o
      integer o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
      equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
                  (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
      integer n5
      integer jmp,i
      save    jmp,i
      jmp=k
!ABCDEFGHJLMNPQRSTUVWXYZ
      select case (jmp)
      case (1)
         o(ox_ano)=n5(0,2,1)
      case (2)
         o(ox_nmx)=n5(0,132,132)
         o(ox_rmx)=324+o(ox_nmx)
         o(ox_tmx)=456+o(ox_nmx)
      case (3)
         o(ox_sign)=n5(10,132,132)
         o(ox_sigt)=456+o(ox_sign)
         o(ox_sigr)=324+o(ox_sign)
      case (4)
         o(ox_mdef)=n5(0,2,0)
      case (6)
         o(ox_frt)=1
      case (9)
         o(ox_indm)=n5(0,50,2)
      case (12)
         o(ox_list)=1
      case (13)
         o(ox_frt)=0
      case (14)
         o(ox_list)=0
      case (17)
         o(ox_lazy)=n5(0,1,1)
      case (18)
         i=n5(1,99,8)
         rewind i
      case (19)
         o(ox_seqf)=n5(0,2,0)
      case (20)
         o(ox_mtrc)=n5(0,9,0)
      case (21)
         o(ox_usk)=o(ox_usk)+1
         o(o(ox_usk))=o(ox_unit)
         if(o(ox_usk)>2504)call mesage(15, 30, 0,0)
         o(ox_unit)=n5(1,99,o(ox_unit))
      case (5,7,8,10,11,15,16)
         call mesage(14, 22, 0,0)
      case default
         o(ox_ano)=n5(0,2,1)
      end select
      return
      end subroutine cccard
!_______________________________________________________________________
!__________________________________________
      integer function n5(l,m,k)
      implicit none
      include "mortran3_indices.f95"
      integer  l,m,k
      dimension o(ox_capacity)
      common /ibm/o
      integer o
      integer o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
      equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
                  (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
      integer nib
      integer j
      save    j
      j=457+4
      n5=nib(j,10)
      if (.not.((l<=n5).and.(n5<=m))) then
         call mesage(14, 3, 0,0)
         n5=k
      end if
      return
      end function n5
!_______________________________________________________________________
!________________________________________
      subroutine mactrc(w,r,s)
      implicit none
      include "mortran3_indices.f95"
      integer  w,r,s
      dimension o(ox_capacity)
      common /ibm/o
      integer o
      integer o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
      equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
                  (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
      integer k,m,t,n
      save    k,m,t,n
      if (o(ox_nest)>0.and.o(ox_mtrc)<3.and.w/= 4)return
      select case (w)
      case (2)
         call dumdum(175,90,37,37,37,22,42,29,27,34,90)
         o(175+4)=(o(ox_base)-1145)/120
         call crit(r,s)
      case (4)
         call dumdum(175,90,45,28,29,27,10,23,16,14,90)
         call crit(r,s)
      case (5)
         call dumdum(175,90,37,10,16,10,18,23,28,29,90)
         call crit(r,s)
      end select
      select case (w)
      case (33)
         if (o(ox_nest)==0) then
            call dumdum(175,90,90,27,14,13,30,12,14,13,90)
         else
            call dumdum(175,90,90,90,90,90,23,14,28,29,90)
            o(175+3)=o(ox_nest)
         end if
         call crit(r+1,s+1)
      case (15)
         call dumdum(175,90,90,13,14,15,18,23,14,13,90)
         call crit(r+4,o(r+3))
         if (r+4>=o(r+3)) then
            call dumdum(175,90,23,30,21,21,90,31,10,21,90)
            call crit(s,s)
         else
            call dumdum(175,90,90,90,90,31,10,21,30,14,90)
            o(175+2)=o(ox_typ)
            call crit(s+4,o(s+3))
         end if
      case (12)
         call dumdum(175,90,10,25,25,14,23,13,14,13,90)
         call crit(o(s-2*(3)-1),o(s-2*(3)-2) )
         call dumdum(175,90,90,90,90,90,90,90,29,24,90)
         call crit(o(s-2*(1)-1),o(s-2*(1)-2) )
      case (13)
         call dumdum(175,90,90,27,14,22,24,31,14,13,90)
         call crit(o(s-2*(1)-1),o(s-2*(1)-2) )
      case (22)
         if(o(r+4)<0)return
         call dumdum(175,90,90,22,10,29,12,17,14,13,90)
         call crit(r+4,o(r+3) )
         k=o(s-1)
         do m = 1,k
            call dumdum(175,90,90,90,90,10,27,16,90,90,90)
            o(175+8)=m
            t=o(s-2*(m)-1)
            n=o(s-2*(m)-2)
            if (t/= n) call crit(t,n)
         end do
      end select
      return
      end subroutine mactrc
!_______________________________________________________________________
!__________________________________________
      subroutine crit(a,z)
      implicit none
      include "mortran3_indices.f95"
      integer  a,z
      dimension o(ox_capacity)
      common /ibm/o
      integer o
      integer o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
      equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
                  (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
      integer b,c
      save    b,c
            b=a
            do while (b<z)
         c=175+9
          do while (c<246.and.b<z)
            c=c+1
            o(c)=o(b)
            b=b+1
            if(o(c)==o(ox_bla))o(c)=37
            if(o(c)>100+19)o(c)=100
            if(o(c)==-1)o(c)=74
            if(o(c)==-2)o(c)=82
         end do
         call rw(33,o(ox_mlst),175,c)
         call dumdum(175,90,90,90,90,90,90,90,90,90,90)
      end do
      return
      end subroutine crit
!_______________________________________________________________________
!________________________________________
      subroutine mesage (level,no,k1,k2)
      implicit none
      include "mortran3_indices.f95"
      integer level,no,k1,k2
      dimension o(ox_capacity)
      common /ibm/o
      integer o
      integer o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
      equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
                  (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
      common/junk/ verbose
      logical verbose
      if(level==14 .or. level==15)o(ox_merr)=o(ox_merr)+1
      call find(level,no,k1)
      if(k2/= 0)call mactrc(4,k1,k2)
            if (level==13) then
         call find(13,2,o(ox_locs))
         call find(13,3,o(ox_left))
            end if
            if (level<=14) then
         if(o(ox_merr)<100)return
         call find(15,10,0)
            end if
            if (o(ox_qfl)==o(ox_bq) .or. o(ox_qfl)==o(ox_dq)) then
         o(ox_merr)=o(ox_merr)+1
         if(o(ox_qfl)==o(ox_bq)) call find(16,5,0)
         if(o(ox_qfl)==o(ox_dq))call find(16,4,0)
            end if
      if(o(ox_lsk)>2255.or.o(ox_msk)>2305)o(ox_merr)=o(ox_merr)+1
            if (o(ox_msk)>2305) then
         call find(16,3,o(ox_msk)-2305)
            end if
            do while (o(ox_lsk)>2255)
         call find(16,2,o(o(ox_lsk)))
         o(ox_lsk)=o(ox_lsk)-1
            end do
      call find(13,3,o(ox_left))
            if (o(ox_merr)==0) then
         call find(16,6,0)
            else
          call find(16,7,o(ox_merr))
         write(6,*) 'compilation error (stop 12)'
         call exit(12) !stop 12
            end if
      if (o(ox_mode)==0)call inital(2)
      if( verbose ) write(6,*) 'OK'
      call exit(0) !stop 0
      end subroutine mesage
!_______________________________________________________________________
!__________________________________________
      subroutine find(l,n,v)
      implicit none
      include "mortran3_indices.f95"
      integer l,n,v
      dimension o(ox_capacity)
      common /ibm/o
      integer o
      integer o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
      equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
                  (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
      integer llong
      integer a,c,q,r,s,nerd,k
      save    a,c,q,r,s,nerd,k
      a=175
      c=a+9
      q=1865+l
 680 if (o(q)==0) go to 4610
         q=o(q)
         if(o(q+5)/= n)go to 680
         r=o(q+2)+4
         s=o(o(q+2)+3)
 4620    if(r>=s)go to 4621
            c=c+1
            o(c)=o(r)
            r=r+1
         go to 4620
 4621    continue
         go to 4600
 4610    c=c+llong(c,n,10,0)
 4600 continue
      if(l==15) call dumdum(175,90,45,45,45,15,10,29,10,21,90)
      if(l==14) call dumdum(175,90,45,32,10,27,23,18,23,16,90)
      if (l/= 13.and.l/= 16) go to 4640
         call dumdum(175,90,90,90,90,90,90,90,90,90,90)
         if(v/= 0) nerd=llong(a,v,10,0)
 4640 continue
      call rw(33,o(ox_mlst),a,c)
      if (o(ox_merr)>0) call rw(23,o(ox_flst),a,c)
      do k = a,c
         o(k)=o(ox_bla)
      end do
      c=175+8
      return
      end subroutine find
!_______________________________________________________________________
!________________________________________
      integer function linf(a,z,ind)
      implicit none
      include "mortran3_indices.f95"
      integer  a,z,ind
      dimension o(ox_capacity)
      common /ibm/o
      integer o
      integer o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
      equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
                  (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
      integer llong
      integer last,kur,kit,kut,kst,h,b,i,j,npl,d,koff,c,limc,k
!     LINF modes: packed comments and rule diagnostics.
      integer, parameter :: linf_comment_mode = 12
      integer, parameter :: linf_rule_diagnostic_mode = 15
      data last/0/,kur/0/,kit/1/,kut/0/
      save    last,kur,kit,kut,kst,h,b,i,j,npl,d,koff,c,limc,k
      linf=a-1
!CEFGLMNPQRS UVWXY
      o(ox_mcnt)=0
      if (ind==linf_comment_mode .and. (o(ox_pcom)==0.or.o(ox_ano)==1 &
       .or.o(ox_rdq)==o(a)))return
      kst=0
      h=0
      b=a
      i=175
      kut=kur
      if(o(b)==o(ox_icur)) then
         b=b+2
         kit=o(b-1)-1
         kur=kur+kit
         kut=kur
         if(kit==0) then
            kut=kut-1
            kit=1
         end if
      end if
      do j = 175,254
         o(j)=o(ox_bla)
      end do
      if (ind==linf_rule_diagnostic_mode) then
         do while (i<181 .and. b<=z)
            if (o(b)==o(ox_bla)) then
               b=b+1
               cycle
            end if
            if (o(b)>=0 .and. o(b)<=9) then
               o(i)=o(b)
               i=i+1
               b=b+1
            else
               exit
            end if
         end do
         npl=65
         d=45
         koff=7
      else
         npl=71
         d=12
         o(175)=d
         koff=max(min(80,o(ox_indc)),2)
      end if
      if (b>z .and. i>175) then
         call rw(33,o(ox_flst),175,181)
         if(ind==linf_rule_diagnostic_mode)call mesage(14, 14, 0,0)
         return
      end if
      c=174+koff+kut*o(ox_indf)
      if(kut>last)c=c-o(ox_indf)
      if(c>174+50)c=174+7
      i=c
      do while (b<=z)
         limc=min0(b+npl,z)
         do while (b<=limc .and. i<=246)
            if (ind==linf_rule_diagnostic_mode) then
               if (o(ox_holr)==1) then
                  if (o(b)==o(ox_lq)) then
                     if (i>246-4) exit
                     h=1
                     do while (o(b+h)/=o(ox_rq))
                        h=h+1
                     end do
                     o(b+h)=o(ox_bla)
                     if (h==1) then
                        o(b)=o(ox_bq)
                        h=0
                     else
                        i=i+llong(i-1,h-1,10,0)
                        o(i)=17
                        b=b+1
                        i=i+1
                     end if
                  end if
               else if (o(b)==o(ox_bq)) then
                  o(i)=o(b)
                  i=i+1
               end if
            end if
            if (o(b)==o(ox_eol)) then
               b=b+1
               exit
            end if
            o(i)=o(b)
            i=i+1
            b=b+1
         end do
         if (o(ox_seqf)==1) then
            do k = 1,8
               o(254-k+1)=o(o(ox_tmx)-k+1)
            end do
         else if (o(ox_seqf)==2) then
            k=llong(247,o(ox_seqn),10,0)
         end if
         do k = 175,246
            if (o(k)==o(ox_lq)) then
               o(k)=o(ox_bq)
               kst=1
            end if
            if (o(k)==o(ox_rq)) then
               o(k)=o(ox_bq)
               kst=0
            end if
         end do
        if (i>246 .and. ind==linf_rule_diagnostic_mode &
           .and. kst==0) then
          do while (o(i-1)<38 .and. kst>0)
               b=b-1
               i=i-1
               o(i)=o(ox_bla)
          end do
        end if
         do k = 175,254
            if(0>o(k).or.o(k)>100-1)o(k)=100-1
            o(k)=o(689+o(k))
         end do
!   LOOKOUT
         call rw(23,o(ox_flst),175,254)
         do k = 175,254
            o(k)=o(ox_bla)
         end do
         i=c
         if (ind==linf_rule_diagnostic_mode) then
            o(175+5)=d
         else
            o(175)=d
         end if
         if (kst>0) then
            i=181
            o(181-1)=45
         end if
      end do
      last=kur
      if(ind==linf_comment_mode) o(a)=o(ox_rdq)
 return
      end function linf
!_______________________________________________________________________
!__________________________________________
      subroutine dumdum(l,a,b,c,d,e,f,g,h,i,j)
      implicit none
      include "mortran3_indices.f95"
      integer l,a,b,c,d,e,f,g,h,i,j
      dimension o(ox_capacity)
      common /ibm/o
      integer o
      integer o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
      equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
                  (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
      o( l )=a
      o(l+1)=b
      o(l+2)=c
      o(l+3)=d
      o(l+4)=e
      o(l+5)=f
      o(l+6)=g
      o(l+7)=h
      o(l+8)=i
      o(l+9)=j
       return
      end subroutine dumdum
!_______________________________________________________________________
!__________________________________________
      subroutine rw(n,i,j,k)
      implicit none
      include "mortran3_indices.f95"
      integer  n,i,j,k
      character*1 ch(200)
      integer     hc(200)
      dimension o(ox_capacity)
      common /ibm/o
      integer o
      integer o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
      equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
                  (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
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

      nr=k+1-j
   if (n==0) then
         !write(44,*) 'nr = ',nr,k,j,i
         read(i,1,end=5130) (ch(m),m=1,nr)
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
         do m=1,nr
            o(j+m-1)=ichar(ch(m))
         end do
         return
         end if
       kof=589
      if (n==33) kof=100+kof
      if (n==23) kof=0
         do l=1,nr
            hc(l) = min(255,max(0, o(j+l-1) ))
            if (kof>0) hc(l)=o(hc(l)+kof)
            ch(l)=char( hc(l)  )
         end do
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
        do while (jj>=1)
           if( ch(jj)/= ' ' ) exit
           jj=jj-1
      end do
      nrr = jj

      ! Check for C-preprocessor statements
      ! If the first 6 chars on the line are blank and if the first non-blank
      ! char from coloumn 7 is a #, assume it is a C-preprocessor directive
      ! and make the # to be the first char on the line.
      nfirst=1
         jj=1
         do while (jj<=nrr)
            if( ch(jj)/= ' ' ) exit
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
              do while (jj<=nrr)
                 if( ch(jj)/= ' ' ) exit
                 jj=jj+1
              end do
          end if
          do ii=jj,nrr
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
              do while (ii>=1)
                 if( f_buffer(ii)/= ' ' ) exit
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
              do while (jj<nfl)
              ii = jj + 66
              if( ii>nfl ) ii = nfl
              write(i,'(a6,66a1)') '     *',(f_buffer(m),m=jj+1,ii)
              jj = ii
              end do
              nfl = 0
              return
          end if
          return
      endif

      if( .not.is_format ) then
           jj=7
           do while (jj<=nrr)
              if( ch(jj)/= ' ') exit
              jj=jj+1
          end do
          do ii=jj,jj+5
              itmp = ichar(ch(ii))
              if( itmp>=97.and.itmp<=122 ) itmp=itmp-32
              tmp1_string(ii+1-jj:ii+1-jj) = char(itmp)
          end do
          if( tmp1_string=='FORMAT' ) then
              is_format = .true.
              nfl = nrr
              do ii=1,nrr
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

      write(i,1) (ch(m),m=1,nrr)
 1 format(132a1)

!
!	NRC PATCH TO PUT DATE AND TIME AT EACH PAGE THROW
!
      if( ch(1)=='1' .and. i==6 ) then
          write(i,'(T55,$)') ' '
          call egs_fdate(i)
          write(i,*)
      end if
      return
 5130 o(ox_eof)=1
 return
      end subroutine rw
!_______________________________________________________________________
!________________________________________
      subroutine inital(ind)
      implicit none
      include "mortran3_indices.f95"
      integer  ind
      character*1 count
      dimension o(ox_capacity)
      common /ibm/o
      integer o
      integer o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
      equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
                  (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
      common /junk/ verbose
      logical verbose
      integer kount,k,iuni
      save    kount,k,iuni,count
      if (ind==2) then
         o(ox_mode)=1
         o(ox_seqn)=5
         kount=o(ox_free)+1
         o(ox_frt)=0
         o(ox_unit)=5
         write(7,1)kount,(o(k),k=1,kount)
 1    format(10z8)
 2    format(a1)
         if( verbose ) write(6,*) 'OK'
         call exit(0) !stop 0
         end if
      do k = 7,ox_capacity
         o(k)=0
      end do
      o(ox_unit)=o_b
      o(ox_mlst)=o_e
      o(ox_flst)=o_r
      iuni=o(ox_unit)
!     O(14)=O(4)   THIS IS NO LONGER USED IN FORTRAN 77 VERSIONS
      o(ox_labl)=o_u
      read(iuni,2)count
      rewind iuni
      if (count=='C') then
         call raw()
      else
         read (iuni,1) kount,(o(k),k=1,kount)
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
         call rw(23,o(ox_flst),325+3,325+79)
         call rw(23,o(ox_mlst),325+2,325+79)
         rewind iuni
         end if
      o(ox_qfl)=o(ox_bla)
      o(ox_mxx)=525371
      o(ox_base)=1625
      do k = 1,6
         o(k)=525371
      end do
      o(ox_rtt)=-11
      o(ox_ltt)=-12
      o(ox_icur)=-13
      o(ox_eol)=-14
      o(o_u)=o(ox_trm)
      o_w=450741-1
      o(o_w)=o(ox_trm)
      call dumdum(44,2105,2155,2205,2255,2305,2355,2455,2505,0,0)
      call nxtcrd()
       return
      end subroutine inital
!_______________________________________________________________________
!__________________________________________
      subroutine raw
      implicit none
      include "mortran3_indices.f95"
      dimension o(ox_capacity)
      common /ibm/o
      integer o
      integer o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
      equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
                  (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
      integer   p
      dimension p(100)
      equivalence(o(2555),p(1))
      integer  k,j
      save     k,j
      o(ox_nmx)=o_v
      o(ox_sign)=o_v
      o(ox_rmx)=324+o(ox_nmx)
      o(ox_tmx)=456+o(ox_nmx)
      o(ox_sigt)=456+o(ox_sign)
      o(ox_sigr)=324+o(ox_sign)
      call rw(0,o(ox_unit),325,o(ox_rmx))
      call rw(0,o(ox_unit),689,689+73)
      call rw(0,o(ox_unit),325,o(ox_rmx))
      call rw(0,o(ox_unit),689+74,789+70)
      do k = 689,788
         o(k-100)=o(k)
      end do
!MAKE INPUT AND OUTPUT SAME
! .(MNOSET+54)=.(MNOSET+41);     OUT-TRANSLATE L-ARROW TO =
      call rw(23,o(ox_mlst),325+41,404)
      call rw(23,o(ox_flst),325+41,404)
      do k = 1,100
            j=o(589+k-1)
         o(889+j)=k-1
!     WRITE(6,1) J,K-1
      end do
      o(ox_dflt)=4
      o(ox_free)=2655
      do k = 1,10
         o(k+100)=k+100
         o(k+110)=k+110
         o(100-k)=-k
      end do
!     BLANK BAT BLB TRM LPR RPR LSB RSB LCB RCB
      call dumdum(76, 90, 79, 78, 75, 38, 46, 74, 82, 83, 84)
      o(ox_bq)=77
      o(ox_dq)=80
      call dumdum(29001,29018,29016,29016,29016,29016,29009,0,3,0,0)
      call dumdum(29009,38,108,46,108,38,108,46,107,24,0)
      call define(29009)
      p(46)=101
      p(38)=202
      p(87)=303
      p(88)=404
      p(89)=505
      p(42)=706
      p(43)=707
      p(44)=808
      p(45)=809
      p( 36)=910
      p( 32)=303
      p( 12)=404
      p( 29)=505
      p( 11)=611
      p( 14)=612
      p( 16)=613
      p( 18)=614
      p( 23)=615
      p( 30)=616
      o(ox_dflt)=6
       return
      end subroutine raw

      subroutine open_units

      implicit none
      include "mortran3_indices.f95"
      dimension o(ox_capacity)
      common /ibm/o
      integer o
      integer o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
      equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
                  (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
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

        open(unit=1,file='fort.1',status='OLD',err=11)
        go to 12
 11   call getarg(0,arg)
        write(6,'(a,a)') arg(1:lnblnk1(arg)),' called without arguments'
        write(6,'(a)') '  but file fort.1 does not exist! '
        write(6,'(a)') '  quiting now.'
        call exit(1) !stop 1
 12   continue
        open(unit=5,file='fort.5',status='OLD',err=6)
 6   open(unit=6,file='fort.6',status='UNKNOWN')
        open(unit=7,file='fort.7',status='UNKNOWN')
        open(unit=2,file='fort.2',status='OLD',err=3)
 3   open(unit=3,file='fort.3',status='OLD',err=4)
 4   open(unit=4,file='fort.4',status='OLD',err=8)
 8   open(unit=8,file='fort.8',status='OLD',err=9)
 9   continue
        o_b=1   ! hex/raw data unit number
        o_e=6   ! listing unit number
        o_r=7   ! fortran/hex output unit number
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
              go to 88
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
            go to 104
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
      o_b=1   ! hex/raw data unit number
      o_e=8   ! listing unit number
      o_r=7   ! fortran/hex output unit number
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
      end function lnblnk1
