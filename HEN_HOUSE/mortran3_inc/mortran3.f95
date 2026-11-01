
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

program mortran3
    ! Translate Mortran source by tokenizing input, matching macro rules, and expanding them into Fortran output.
    implicit none
    include "mortran3_indices.f95"
    integer :: o(ox_capacity)
    common /ibm/o
    integer :: o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
    equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
        (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
    integer :: o_c,o_mxc,o_m
    integer :: iw,a,d,mxd,kd,mq,f,t,ip,iq,n,n1,n2,n3,kc,kb,kp,p,q,ia, &
        ire,nb,kx,y,nerd
    integer :: ispec,linf
    logical :: first_pass
    equivalence(o(ox_crs),o_c),(o(ox_sigt),o_mxc),(o(ox_mcom),o_m)
!
    common/nrcc/ oflag
    integer :: oflag

    save    iw,a,d,mxd,kd,mq,f,t,ip,iq,n,n1,n2,n3,kc,kb,kp,p,q,ia, &
        ire,nb,kx,y,nerd

    ! Open configured input/output units before writing the version banner.
    call open_units

    write(o_e,'(a,1x)',advance='no') 'Mortran 3.2'
    call egs_fdate(o_e)
    write(o_e,*)

    ! Seed the shared-buffer cursors, load the initial tables/options, and set
    ! the first scan-buffer fill as the first pass through the main loop.
    o_v=100
    o_u=1000
    call inital(1)
    iw=ox_mnxbf_base
    o_x=o_w
    first_pass=.true.
    ! Alternate between matching rules against buffered input and loading more
    ! tokens. After an expansion, resume its parent before scanning new input.
    translator_cycle: do
        if (.not.first_pass) then
            if (o_s>o_u) then
                if (o(ox_nest)>0) then
                    call expand(-1)
                    cycle translator_cycle
                end if
                o_s=ox_home_base
                o_u=o_s-1
                first_pass=.true.
            else
                ! Select the rule chain keyed by the next source token.
                o_m=o(ox_base)+o(o_s)
            end if
        end if
        ! Try the rules for the current token in table order; if none match,
        ! extend the scan buffer with more input below.
        macro_scan: do
            if (.not.first_pass) then
                macro_match: block
                    if(o(o_m)==0)exit
                    o_m=o(o_m)
                    ! Load this candidate's pattern bounds and reset state used
                    ! to track nested delimiters and captured arguments.
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
                    ! Compare this candidate pattern with the buffered source.
                    ! Quoted source strings are packed tokens and are skipped whole.
                    token_scan: do
                        if(o_v>o_u)exit token_scan
                        if (o(o_v)==o(ox_ldq)) then
                            o_v=o_v+o(o_v+1)+3
                            cycle token_scan
                        end if
                        if (mq==0) then
                            if (o(d)==o(ox_lcb)) then
                                if (o(d+1)<=100.and.o(ox_base)>ox_ct3_base) then
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
                            if (o(d)==o(o_v).and.o(o_v)<o(ox_ga)) then
                                if( o(ox_lq) ==o(d))o(ox_sq)=o(ox_sq)+1
                                if( o(ox_rq) ==o(d))o(ox_sq)=o(ox_sq)-1
                                if(o(d)==o(ox_lmb))kd=1
                                if(o(d)==o(ox_rmb))kd=0
                                o_v=o_v+1
                                t=o_v
                                d=d+1
                                if(d>=mxd) then
                                    call cout()
                                    call expand(a)
                                    cycle translator_cycle
                                end if
                                cycle token_scan
                            end if
                            if (o(o_v)==o(ox_bla).and.o(ox_sq)+kd==0) then
                                o_v=o_v+1
                                cycle token_scan
                            end if
                            if (o(d)==o(ox_ga).or.o(d)==o(ox_gp)) then
                                ! Begin capturing a macro argument and remember its
                                ! source bounds; the following scan balances its delimiters.
                                if (o(d)==o(ox_ga)) then
                                    d=d+1
                                    n=o(d)
                                    n1=n/4
                                    n2=(n-4*n1)/2
                                    n3=n-4*n1-2*n2
                                else
                                    n1=1
                                    n2=1
                                    n3=o(ox_mode)
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
                            end if
                        end if
                        if (mq/=0) then
                            p=ip
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
                                d=d+1
                                if(d>=mxd) then
                                    call cout()
                                    call expand(a)
                                    cycle translator_cycle
                                end if
                                cycle token_scan
                            end if
                        end if
                        if (o(o_v)==o(ox_bla).and.mq==1) then
                            o_v=o_v+1
                            cycle token_scan
                        end if
                        ! A candidate with no argument capture cannot recover from
                        ! a mismatch; otherwise scan to the end of the current argument.
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
                                exit macro_match
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
                        o_s=ox_home_base
                        o(o_s)=o(o_u)
                        o_u=o_s
                    end if
                end block macro_match
            end if
            first_pass=.false.
            ! No rule expanded at this scan point. Append normalized input tokens
            ! to the scan buffer, preserving quoted strings and nested groups.
            input_restart: do
                o(ox_qfl)=o(ox_bla)
                kx=0
                do
                    o_u=o_u+1
                    if(o_u>ox_capacity)call mesage(15, 1, 0,0)
                    o(o_u)=o(o_c)
                    o_c=o_c+1
                    if(o_c>o(ox_sigt))call nxtcrd()
                    if (o(o_u)==o(ox_lcb)) then
                        ! Copy a complete nested brace group without treating its
                        ! contents as top-level quote/statement delimiters.
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
                        ! Pack double-quoted text as one length-prefixed token so
                        ! macro matching cannot inspect its contents as syntax.
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
                                cycle input_restart
                            else if (o(ox_lazy)==1) then
                                o(ox_qfl)=o(ox_bla)
                                call nxtcrd()
                                cycle input_restart
                            else
                                call nxtcrd()
                                o_u=o_u+1
                                cycle
                            end if
                        end do
                    end if
                    if (o(o_u)==o(ox_bq).and.o(ox_stng)>0) then
                        ! When string recognition is enabled, pack the alternate
                        ! quoted form and treat doubled delimiters as escaped data.
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
                        ! Collapse runs of source blanks to one scan-buffer blank.
                        if (o_s/=o_u) then
                            if(o(o_u-1)==o(ox_bla))o_u=o_u-1
                        end if
                        do
                            if(o_c==o_mxc .or.o(o_c)/= o(ox_bla))exit
                            o_c=o_c+1
                        end do
                    end if
                    if (kx==0) then
                        if(o(o_u)==o(ox_trm))cycle translator_cycle
                        if( o(ox_lsb) ==o(o_u))o(ox_lvl)=o(ox_lvl)+1
                        if( o(ox_rsb) ==o(o_u))o(ox_lvl)=o(ox_lvl)-1
                    end if
                end do
            end do input_restart
        end do macro_scan
        if (o(ox_nest)>0) then
            ! Inside a replacement, copy an unmatched token through unchanged.
            ! Copy a line-sequence region as a unit except in the corresponding table.
            o_x=o_x+1
            if(o_x>o(ox_mxx))call mesage(15, 3, 0,0)
            o(o_x)=o(o_s)
            o_s=o_s+1
            if (o(o_x)==o(ox_lns).and.o(ox_base)/=ox_ct1_base) then
                do
                    o_x=o_x+1
                    if(o_x>o(ox_mxx))call mesage(15, 3, 0,0)
                    o(o_x)=o(o_s)
                    o_s=o_s+1
                    if(o(o_x)==o(ox_rns))exit
                end do
            end if
            cycle translator_cycle
        end if
        ! Outside expansion, transfer scanned tokens to output until the buffer
        ! ends or a line-sequence boundary is reached; handle strings and markers.
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
        cycle translator_cycle
    end do translator_cycle
end program mortran3
!_______________________________________________________________________
!__________________________________________
! Match encoded specifiers against source text, including alternatives and repetition.
integer function ispec(p,q,iv,u,t)
    ! Match one encoded specifier against the source buffer. Returns 1 when the
    ! source ends while a match may continue, 2 for a mismatch/invalid pattern,
    ! or 3 for a complete match.
    ! p: Current pattern position; updated while matching and backtracking.
    ! q: Exclusive end of the pattern range; may change when a nested specifier is selected.
    ! iv: Starting source position; updated to the final position on success.
    ! u: Inclusive upper bound of the source-buffer range.
    ! t: Position of a special blank token; advanced when that token is consumed.
    implicit none
    include "mortran3_indices.f95"
    integer :: p,q,iv,u,t
    integer :: o(ox_capacity)
    common /ibm/o
    integer :: o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
    equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
        (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
    integer :: mini
    integer :: c,m,b,v,iq,i,np,jmp,n,nul,w
    save    c,m,b,v,iq,i,np,jmp,n,nul,w
    c=0
    m=0
    b=0
    v=iv
    ! jsk records pending nested-specifier alternatives; ksk records the
    ! source position, pattern position, and repetition count for nested groups.
    o(ox_jsk)=ox_jsk_base
!ADEFGHIJKLRSXYZ
    o(ox_ksk)=ox_ksk_base
    ispec_scan: do
        if(p<q) then
            ! A designated blank token must consume the corresponding source
            ! blank, keeping both cursors synchronized.
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
                ! Resolve a table-backed literal/specifier. mini rewrites p/q
                ! to the selected pattern and returns its matched prefix in np.
                iq=q
                i=mini(p,q,np,1505)
                if(o(ox_mtrc)>=4) call mactrc(5,v,u)
                if(i==0)then
                    ispec=2
                    return
                end if
                ! Save the enclosing pattern end and the continuation point so
                ! another table alternative can be tried after a later failure.
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
                ! Alternative separator: after a successful branch, skip the
                ! rest of its group; after failure, restart at the saved source
                ! position for the next branch.
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
                ! Open a nested group/repetition: save its count, source cursor,
                ! and pattern start, then begin matching the group from count 0.
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
                ! Close a quantified group. The two following values are its
                ! repetition bounds; repeat while below the upper bound, then
                ! restore the enclosing group's saved state.
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
                if (o(ox_ksk)==ox_ksk_base) then
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
            case (2)
                ! Match one source token against the inclusive encoded range.
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
            case (5)
                ! Match the encoded quoted literal exactly. Leading pattern
                ! blanks are aligned with the source before comparing tokens.
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
            if(m==1)cycle ispec_scan
            ! A failed atom can only be recovered at an alternative separator
            ! or a group close; any other next token makes the whole match fail.
            if(o(p)/= o(ox_alt) .and. o(p)/= o(ox_rip))then
                ispec=2
                return
            end if
            cycle ispec_scan
        end if
        if (o(ox_jsk)/=ox_jsk_base) then
            ! The current pattern range is exhausted; restore the most recent
            ! table alternative and continue matching it.
            p=o(o(ox_jsk))
            o(ox_jsk)=o(ox_jsk)-1
            q=o(o(ox_jsk))
            o(ox_jsk)=o(ox_jsk)-1
            cycle ispec_scan
        end if
        exit ispec_scan
    end do ispec_scan
    ! Reaching the end with no pending alternatives means the full specifier
    ! matched; publish the advanced source cursor to the caller.
    iv=v
    ispec=3
    return
end function ispec
!_______________________________________________________________________
!__________________________________________
! Find a macro rule whose literal prefix matches the source text.
integer function mini(s,u,w,kct)
    ! s: Start of the pattern text; set to the selected rule's replacement start on success.
    ! u: Exclusive end of the pattern text; set to the selected rule's replacement end on success.
    ! w: Receives the position immediately after the matched literal prefix.
    ! kct: Offset used to locate the rule table for the pattern.
    implicit none
    include "mortran3_indices.f95"
    integer :: s,u,w,kct
    integer :: o(ox_capacity)
    common /ibm/o
    integer :: o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
    equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
        (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
    integer :: m,l,v,p,q
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
! Expand a macro rule or restore state when returning from a nested expansion.
subroutine expand (f)
    ! Expand the selected rule into the output stack, or resume a suspended
    ! parent expansion when f is negative.
    ! f: Argument count for a new expansion; negative requests state restoration.
    implicit none
    include "mortran3_indices.f95"
    integer :: f
    integer :: o(ox_capacity)
    common /ibm/o
    integer :: o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
    equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
        (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
    integer :: lexp,llong
    integer :: kgn,t,sef,k,j,jv,p,q,nul,jmp,a,n,krap,kraq,z,y
    data kgn/0/
    save    kgn,t,sef,k,j,jv,p,q,nul,jmp,a,n,krap,kraq,z,y
!CDGHILM
    t=0
    if (f<0) then
        ! A child expansion has finished. Drop its nesting level and restore
        ! the six active table-head values saved when its replacement ended.
        o(ox_nest)=o(ox_nest)-1
        if(o(ox_nest)==0)o(ox_frt)=sef
        do k = 1,6
            o(k)=o(6+o_u+1-k)
        end do
        ! Evaluator replacement markers 11 and 12 require evaluating the
        ! expression accumulated on the output stack before resuming the parent.
        j=o(o(o(ox_ysk)))
        if (j>=11 .and. j<=12) then
            jv=lexp(o_w+1,o_x)
            if (o(ox_ifl)==0) then
                ! Outside evaluator-IF mode, return the value as either decimal
                ! text (12) or a single numeric token (11).
                if (j==12) then
                    o_x=o_w+llong(o_w,jv,10,0)
                end if
                if (j==11) then
                    o_x=o_w+1
                    o(o_x)=jv
                end if
            else
                ! Inside evaluator-IF mode, choose the true/false pattern range
                ! and save the enclosing range so it can be restored afterward.
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
        ! Pop the child frame: restore its output start and active rule-table base.
        nul=o(o(ox_ysk))
        o(ox_ysk)=o(ox_ysk)-1
        o_w=o(o(ox_ysk))
        o(ox_ysk)=o(ox_ysk)-1
        o(ox_base)=o(o(ox_ysk))
        o(ox_ysk)=o(ox_ysk)-1
    else
        ! Record the argument count and reserve the remaining space below o_s
        ! for this rule's captured argument ranges and its generated output.
        o(o_s-1)=f
        o(ox_mxx)=o_s-2*f-3
        ! Locate the selected rule's replacement body and retain the link to
        ! the next rule descriptor for the later rule-chain check.
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
        ! A top-level expansion starts a fresh output stack; nested expansions
        ! append to the parent's output instead.
        if( o(ox_nest)==0 )o_x=o_w
    end if
    ! Interpret replacement tokens until the rule is exhausted or it invokes a
    ! nested macro/evaluator expansion that must run before this one can resume.
    expansion_restart: do
        do while (o_b<o_e)
            if (o(o_b)<o(ox_ga)) then
                ! Ordinary replacement characters are copied literally.
                o_x=o_x+1
                if(o_x>o(ox_mxx))call mesage(15, 3, 0,0)
                o(o_x)=o(o_b)
                o_b=o_b+1
                cycle
            end if
            o_b=o_b+1
            jmp=o(o_b-1)-o(ox_rq)
            ! Interpret the encoded replacement item and update output or expansion state.
            expansion_dispatch: block
                if (jmp==4) exit expansion_dispatch
                if (jmp/=3) then
                    if (jmp/=1) then
                        ! A valid parameter number selects one captured argument;
                        ! copy its saved source range into the replacement output.
                        if (o(o_b)>=1 .and. o(o_b)<=o(o_s-1)) then
                            a=o(o_s-2*(o(o_b))-1)
                            n=o(o_s-2*(o(o_b))-2)
                            do while (a<n)
                                o_x=o_x+1
                                if(o_x>o(ox_mxx))call mesage(15, 3, 0,0)
                                o(o_x)=o(a)
                                a=a+1
                            end do
                            o_b=o_b+1
                            cycle
                        else
                            call mesage(15, 4, o_b,o_e)
                        end if
                    end if
                    if (o(o_b)==12) then
                        ! Update the nested generation-control state from the
                        ! following marker and switch table families when needed.
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
                            if(o(ox_ngn)==0)o(ox_base)=ox_ct4_base
                            if(o(ox_ngn)/= 0)o(ox_base)=ox_ct0_base
                        end if
                    else
                        ! Execute a primitive replacement operator unless
                        ! generation-only text is currently being suppressed.
                        if(o(ox_ngn)==0)call kat
                    end if
                    o_b=o_b+1
                    cycle
                end if
                if (o(o_b)==10) then
                    ! Emit the length of a captured argument as decimal text.
                    o_b=o_b+1
                    if(1>o(o_b).or.o(o_b)>o(o_s-1) )call mesage(15, 4, o_b,o_e)
                    o_x=o_x+llong(o_x,o(o_s-2*(o(o_b))-2)-o(o_s-2*(o(o_b))-1),10,0)
                    if(o_x>o(ox_mxx))call mesage(15, 3, 0,0)
                    o_b=o_b+2
                    cycle
                else if (o(o_b)==13) then
                    ! Copy a balanced brace group literally, including nested braces.
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
                else if (o(o_b)==15) then
                    ! Start a packed quoted output token; krap remembers its
                    ! length slot and kraq the nesting level where it began.
                    o_x=o_x+1
                    o(o_x)=o(ox_ldq)
                    o_x=o_x+1
                    krap=o_x
                    kraq=o(ox_nest)
                else
                    ! Suspend this rule and push its table, output, and input
                    ! cursors before dispatching the nested rule expansion.
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
                end if
                o_b=o_b+1
                cycle
                end block expansion_dispatch
                if (krap/=0 .and. kraq==o(ox_nest)) then
                ! Close a packed quoted token only when control returns to the
                ! nesting level at which its opening marker was seen.
                o_x=o_x+1
                o(o_s)=o(ox_rdq)
                o(krap)=o_x-krap-1
                krap=0
                cycle
            end if
            ! The replacement is exhausted. Preserve the active table heads,
            ! then select the rule-table family recorded by the current frame.
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
            o(ox_base)=y*120+ox_ct0_base
            exit
        end do
        if (o(ox_nest)<=0) then
            if (o(ox_isk)==ox_isk_base) then
                if (o_r>0) then
                    ! Try the next candidate rule in the chain.
                    o_b=o_r+4
                    o_e=o(o_r+3)
                    o_r=o(o_r)
                    cycle expansion_restart
                end if
            else
                ! An evaluator-IF range ended; restore its saved input bounds.
                o_e=o(o(ox_isk))
                o(ox_isk)=o(ox_isk)-1
                o_b=o(o(ox_isk))
                o(ox_isk)=o(ox_isk)-1
                cycle expansion_restart
            end if
        end if
        ! Return the current expansion output to the caller. A suspended parent
        ! is resumed later through expand(-1).
        exit expansion_restart
    end do expansion_restart
    if(o(ox_mtrc)>=2) call mactrc(33,o_w,o_x)
    ! Pop expanded characters into the source buffer in their original order,
    ! then point o_s at the beginning of the newly expanded text.
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
! Execute a Mortran rule operator during macro expansion.
subroutine kat
    implicit none
    include "mortran3_indices.f95"
    integer :: o(ox_capacity)
    common /ibm/o
    integer :: o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
    equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
        (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
    integer :: l,m
    equivalence (o(ox_lsk),l),(o(ox_msk),m)
    integer :: llong,nib
    integer :: jmp,t,ibg,z,iuni,nba
    save    jmp,t,ibg,z,iuni,nba
!FGHIJKLPQ  YZ
    ! The current buffer item is the encoded operator. Advance past it, then
    ! use its code (19..30) to select one of the twelve primitive operations.
    if(o(o_b)<19.or.o(o_b)>30) call mesage(15, 7, o_b-2,o_e)
    o_b=o_b+1
    jmp=o(o_b-1)-18
    ! After GA (107), the operator letter is a token from J through U (19..30).
    ! Subtracting 18 selects these primitive operations:
    !   J (19): push a value from the table at offset 78.
    !   K (20): recover/compact buffer storage.
    !   L (21): manipulate the Fortran-label stack.
    !   M (22): manipulate the macro-variable stack.
    !   N (23): convert the top output value to decimal text.
    !   O (24): define a rule; P (25): set evaluator-IF ranges.
    !   Q (26): allocate and push a numeric label; R (27): rewind a unit.
    !   S (28): read/update a table entry; T (29): emit a macro trace.
    !   U (30): use the table at offset 121, optionally formatting its value.
    ! For example, @SS= encodes GA,S,S,=: S selects table entry 28 (STNG),
    ! and '=' assigns the output-stack value to that entry.
    select case (jmp)
    case (1) ! @J: push a value from the table at offset 78.
        ! Read a value from the table area at offset 78 and push it on the
        ! output stack.
        t=o(o_b)+78
        o_x=o_x+1
        o(o_x)=o(t)
        return
    case (2) ! @K: recover/compact buffer storage.
        ! Run recovery and return; recovery manages its own buffer state.
        call krunc(0)
        return
    case (3) ! @L: perform a Fortran-label stack operation.
        ! The selector chooses a label/stack operation. Some selectors also
        ! consume a following option from the encoded operator stream.
        if (o(o_b)==12) then ! @LC...: format a value from the label stack.
            ! Format the decimal value assembled from the selected stack entry
            ! and the following operand, then append it to the output buffer.
            o_x=o_x+llong(o_x,o(l-o(o_b+1))+o(o_b+2),10,1)
            o_b=o_b+2
            return
        else if (o(o_b)==16) then ! @LG...: allocate and emit a label.
            ! Allocate the next numeric label (labels advance by ten) and emit it.
            o(ox_labl)=o(ox_labl)+10
            o_x=o_x+llong(o_x,o(ox_labl),10,0)
            return
        else if (o(o_b)==28) then ! @LS...: parse and push a label value.
            ! Convert the output-buffer number to an integer and push it on the
            ! label stack. Option 1 swaps the two top label-stack entries.
            o_b=o_b+1
            o(ox_lsk)=o(ox_lsk)+1
            o(o(ox_lsk))=nib(o_x,10)
            if(o(ox_lsk)>2304)call mesage(15, 21, 0,0)
            if (o(o_b)==1) then ! @LS1: swap the two top label-stack entries.
                z=o(l-1)
                o(l-1)=o(l)
                o(l)=z
            end if
        else if (o(o_b)==30) then ! @LU...: pop a label-stack entry.
            ! Pop the label stack; option 1 copies its top value to the entry
            ! immediately below it before the pop. Do not cross the stack base.
            o_b=o_b+1
            if(o(o_b)==1)o(l-1)=o(l) ! @LU1: copy the top entry before popping.
            l=l-1
            if (l<ox_lsk_base) then
                call mesage(14, 7, 0,0)
                l=l+1
            end if
        else
            call mesage(15, 7, o_b-2,o_e)
            return
        end if
        if(o(o_b)/=1.and.o(o_b)/=0)then ! @LS/@LU option must be 0 or 1.
            write(6,*) 'compilation error (stop 18)'
            call exit(18)
        end if
        return
    case (4) ! @M: perform a macro-variable stack operation.
        ! Operate on the macro-variable stack: selector 28 pushes an encoded
        ! value, 27 replaces the current entry, and 30 pops it to output.
        if (o(o_b)==28) then ! @MS: push a value onto the macro-variable stack.
            o_b=o_b+1
            o(ox_msk)=o(ox_msk)+1
            o(o(ox_msk))=o(o_b)
            if(o(ox_msk)>2354)call mesage(15, 22, 0,0)
        else if (o(o_b)==27) then ! @MR: replace the current macro-variable value.
            o_b=o_b+1
            o(m)=o(o_b)
        else if (o(o_b)==30) then ! @MU: pop the macro-variable value to output.
            o_x=o_x+1
            o(o_x)=o(m)
            m=m-1
            if (m<ox_msk_base) then
                call mesage(14, 8, 0,0)
                m=m+1
            end if
        else
            call mesage(15, 7, o_b-2,o_e)
        end if
        return
    case (5) ! @N: convert the top output value to decimal text.
        ! Replace the top output item with its decimal character representation.
        ibg=o(o_x)
        o_x=o_x-1+llong(o_x-1,ibg,10,1)
        return
    case (6) ! @O: define a rule.
        ! Define the rule represented by the current source-buffer range.
        call define(o_s)
        return
    case (7) ! @P: set evaluator-IF ranges.
        ! Enter evaluator conditional mode and derive the bounds of its two
        ! encoded buffer ranges from the four relative lengths that follow.
        o(ox_ifl)=1
        o_b=o_b+4
        o(ox_ktb)=o_b+o(o_b-1)
        o(ox_kte)=o_b+o(o_b-2)
        o(ox_kfb)=o_b+o(o_b-3)
        o(ox_kfe)=o_b+o(o_b-4)
        o_b=o_b-1
        return
    case (8) ! @Q: allocate and push a numeric label.
        ! Allocate and push the next numeric label. Option 1 swaps the top two
        ! label-stack entries; only options 0 and 1 are valid here.
        o(ox_labl)=o(ox_labl)+10
        o(ox_lsk)=o(ox_lsk)+1
        o(o(ox_lsk))=o(ox_labl)
        if(o(ox_lsk)>2304)call mesage(15, 21, 0,0)
        if (o(o_b)==1) then ! @Q1: swap the two top label-stack entries.
            z=o(l-1)
            o(l-1)=o(l)
            o(l)=z
        end if
        if(o(o_b)/=1.and.o(o_b)/=0)then ! @Q option must be 0 or 1.
            write(6,*) 'compilation error (stop 18)'
            call exit(18)
        end if
        return
    case (9) ! @R: rewind an input unit.
        ! Rewind the input unit identified by the following buffer item.
        iuni=o(o_b)
        rewind iuni
        return
    case (10) ! @S: read or update a table entry.
        ! Select a target table entry. An optional bank selector (1..4) adds
        ! a 26-entry bank offset; the operator itself is handled below.
        t=o(o_b)
        o_b=o_b+1
        if (0<o(o_b).and.o(o_b)<5) then
            t=t+26*o(o_b)
            o_b=o_b+1
        end if
        nba=0
    case (11) ! @T: emit a macro trace.
        ! Emit a macro trace when tracing is enabled, then restore o_b to the
        ! operator token before returning.
        if(o(ox_mtrc)>=1) call mactrc(22,o(ox_mcom),o_s)
        o_b=o_b-1
        return
    case (12) ! @U: access the table at offset 121.
        ! Select an entry in the table beginning at 121. Save the requested
        ! radix from output and remove it before the shared action.
        t=121+o(o_b)
        o_b=o_b+1
        nba=o(o_x)
        o_x=o_x-1
    case default
        call mesage(15, 7, o_b-2,o_e)
        return
    end select
    ! Cases 10 and 12 share the final action: increment, decrement, assign
    ! from the output stack, or push a target value (optionally radix-formatted).
    if (o(o_b)==43) then ! @S/@U...+: increment the selected table entry.
        o(t)=o(t)+1
    else if (o(o_b)==42) then ! @S/@U...-: decrement the selected table entry.
        o(t)=o(t)-1
    else if (o(o_b)==41) then ! @S/@U...=: assign the output value to the entry.
        o(t)=o(o_x)
        o_x=o_x-1
    else if (o(o_b)==12) then ! @S/@U...C: push the entry, optionally radix-formatted.
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
! Write packed comments from the scanned source range to the listing.
subroutine cout
    implicit none
    include "mortran3_indices.f95"
    integer :: o(ox_capacity)
    common /ibm/o
    integer :: o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
    equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
        (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
    integer :: linf
    integer :: t,nerd
    save    t,nerd
    ! Scan the active source range for packed comment delimiters.
    t=o_s
    do
        if(t>o_v)exit
        if (o(t)==o(ox_ldq)) then
            ! The following slot stores the payload length; list the payload,
            ! then advance over the complete encoded comment block.
            nerd=linf(t+2,t+o(t+1)+1,12)
            t=t+o(t+1)+2
        end if
        t=t+1
    end do
    return
end subroutine cout
!_______________________________________________________________________
!________________________________________
! Process a macro definition and update the rule tables as required.
subroutine define(s)
    ! Process captured definition fields and add, extend, or remove a rule in
    ! the appropriate rule/specifier table.
    ! s: Stack index holding the definition's key, optional type, and replacement ranges.
    implicit none
    include "mortran3_indices.f95"
    integer :: s
    integer :: o(ox_capacity)
    common /ibm/o
    integer :: o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
    equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
        (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
!GHQUVWXYZ
    integer :: l(3),k,a,t,n,z,b,e,m,p,lp,c,need,j,f,i,r
    logical :: found
    save      l,k,a,t,n,z,b,e,m,p,lp,c,need,j,f,i,r
    ! The key/pattern field must not be empty.
    if (o(s-2*(1)-1)==o(s-2*(1)-2)) then
        call mesage(14, 1, 0,0)
        return
    end if
    ! Use the default rule type unless the definition supplied an explicit one.
    o(ox_typ)=o(ox_dflt)
    if ( o(s-2*(2)-1)/= o(s-2*(2)-2) )o(ox_typ)=o(o(s-2*(2)-1))
    ! Compact and normalize each captured field in place. The key, type, and
    ! replacement use slightly different quote/brace rules.
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
                ! Convert enabled alternate quotes to internal quote markers;
                ! doubled delimiters are data, not the end of the quoted span.
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
                ! Collapse repeated blanks outside quoted text.
                if (z==0) then
                    do while (o(a)==o(ox_bla).and.a<t)
                        a=a+1
                    end do
                end if
            else if (o(n)==o(ox_blb)) then
                ! A doubled BLB marker is literal; otherwise convert it to
                ! the internal GP marker in the pattern.
                if (o(a)==o(ox_blb)) then
                    a=a+1
                else
                    o(n)=o(ox_gp)
                end if
            else if (o(n)==o(ox_bat)) then
                ! A doubled BAT marker is literal; otherwise convert it to
                ! the internal GA marker in the pattern.
                if (o(a)==o(ox_bat)) then
                    a=a+1
                else
                    o(n)=o(ox_ga)
                end if
            end if
            if (o(ox_blgr)==0 .and. k==3) then
                ! In the replacement only, optionally protect braces from the
                ! normal source grouping rules.
                if(o(n)==o(ox_lcb))o(n)=o(ox_lgb)
                if(o(n)==o(ox_rcb))o(n)=o(ox_rgb)
            end if
            cycle
        end do
        o(s-2*(k)-2)=n
        l(k)=o(s-2*(k)-2)-o(s-2*(k)-1)
    end do
    ! Types 12..14 address existing specifier tables rather than creating a
    ! regular rule immediately. Type 14 selects the alternate table family.
    if (o(ox_typ)>=12 .and. o(ox_typ)<=14) then
        b=o(s-2*(1)-1)
        e=o(s-2*(1)-2)
        if (o(ox_typ)==14) then
            m=ox_ct3_base+o(b)
        else
            m=ox_ct4_base+o(b)
        end if
        found=.false.
        ! Follow the selected bucket's chain and compare candidate keys by
        ! normalized length and token contents.
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
                ! Removal marks the matching specifier and its attached entries
                ! as deleted, preserving linked records for table traversal.
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
            ! Type 12 appends a replacement to the existing specifier's entry
            ! chain, allocating and linking one compact record.
            need=4+l(3)
            if(need>ox_mnxbf_base-1-o(ox_free)) call krunc(need)
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
    ! Allocate a normal rule as a key record plus a replacement record.
    need=l(1)+l(3)+2*4
    if (need>ox_mnxbf_base-1-o(ox_free)) call krunc(2+need)
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
    ! Reject unsupported grouped keys, invalid leading tokens, and table types
    ! outside the eight regular rule families.
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
    ! Link the key record into the bucket selected by its type and first token;
    ! initialize its replacement record immediately after the key.
    i=o(i)+ox_ct0_base + o(ox_typ)*120
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
        ! Resolve relative offsets used by conditional constructs, then copy
        ! the normalized replacement into its allocated record.
        call doit(e-k,e)
        do j = 1,k
            o(f-j)=o(e-j)
        end do
    end if
    if (o(ox_mdef)>0) call mactrc(15,p,r)
    ! Publish the new free-space position and remaining-capacity counters.
    o(ox_free)= p +need
    o(ox_locs)=ox_mnxbf_base-1-o(ox_free)
    o(ox_left)=(100*o(ox_locs))/(ox_mnxbf_base-1-ox_mbf_base)
    return
end subroutine define
!_______________________________________________________________________
!__________________________________________
! Fix relative offsets for conditional constructs in a macro replacement.
subroutine doit(a,z)
    ! Resolve relative branch offsets in encoded evaluator IF/ELSE constructs
    ! within a macro replacement, modifying the replacement buffer in place.
    ! a: First index of the replacement-text range to process.
    ! z: Last index of the replacement-text range to process (inclusive).
    implicit none
    include "mortran3_indices.f95"
    integer :: a,z
    integer :: o(ox_capacity)
    common /ibm/o
    integer :: o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
    equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
        (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
    integer :: s(200),x,t,p
    save      s,x,t,p
    x=0
!BCDEFGHIJKLMNQRUVWY
    ! Scan backward so each branch target is known when its relative offset is patched.
    t=z
    do while (t>=a)
        if (o(t)==o(ox_lif)) then
            if (o(t-1)==o(ox_rif)) then
                ! Save an adjacent RIF/LIF pair as a two-marker branch boundary.
                s(x+1)=t+1
                s(x+2)=o(t)
                x=x+2
                t=t-1
            else
                ! Save this LIF's target boundary, then find the preceding GA/25
                ! marker that anchors the generated conditional offset fields.
                s(x+1)=t+1
                x=x+1
                do
                    if(o(t)==o(ox_ga).and.o(t+1)==25)exit
                    t=t-1
                    if(t<a)return
                end do
                ! The three offset slots are located relative to the GA/25 marker.
                p=t+6
                o(p-1)=s(x)-p
                x=x-1
                if (s(x)==o(ox_lif)) then
                    ! An enclosing LIF supplies distinct branch destinations;
                    ! encode each as an offset from p and pop the three saved entries.
                    o(p-4)= s(x-2)-p
                    o(p-3)= s(x-1)-p
                    o(p-2)= o(p-3)-2
                    x=x-3
                else
                    ! Otherwise all three generated jumps target the same saved
                    ! boundary; pop that single pending branch target.
                    o(p-2)=s(x)-p
                    o(p-3)=s(x)-p
                    o(p-4)=s(x)-p
                    x=x-1
                end if
            end if
        else if (o(t)==o(ox_rif)) then
            ! Remember a RIF boundary for the matching LIF encountered earlier
            ! in this reverse scan.
            s(x+1)=t
            x=x+1
        end if
        t=t-1
    end do
    return
end subroutine doit
!_______________________________________________________________________
!__________________________________________
! Reclaim unused macro storage and compact the macro buffer.
subroutine krunc(r)
    ! Unlink deleted macro records and compact the remaining macro buffer in place.
    ! r: Minimum free slots required afterward; zero requests compaction only.
    implicit none
    include "mortran3_indices.f95"
    integer :: r
    integer :: o(ox_capacity)
!ABEFGHIJKLQTUVWXYZ
    common /ibm/o
    integer :: o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
    equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
        (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
    integer :: m,n,d,p,s,c,i
    save    m,n,d,p,s,c,i
    if (o(ox_mrkf)/=0) then
        ! Walk the marked-record chain, unlink each eligible record from its
        ! rule bucket, and mark it deleted before the storage-compaction pass.
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
    n=ox_mbf_base
    m=o(n+3)
    d=0
! PREDECESSOR  SUCCESSOR    CONCAT
    ! A zero sentinel at the free pointer terminates the physical-record chain.
    o(o(ox_free))=0
    do while (n/=o(ox_free))
        m=n
        n= o(m+3)
        ! Save this record's links/status before deciding whether to reclaim or move it.
        p=o(m+1)
        s=o(m)
        c=o(m+2)
        if (c==o(ox_ded)) then
            ! Deleted record: accumulate its full extent; later live records
            ! will be shifted left by the accumulated amount.
            d=d+ n-m
        else
            ! Live record: subtract reclaimed space from every link into or
            ! beyond this record before moving its contents down in the buffer.
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
    ! Publish the compacted end of storage and restore its zero sentinel.
    o(ox_free)=o(ox_free)-d
    o(o(ox_free))=0
    ! Fail if compaction could not satisfy the caller's requested allocation.
    if (r>d)call mesage(15, 6, 0,0)
    ! Recompute free-space statistics and report the number of reclaimed slots.
    o(ox_locs)=ox_mnxbf_base-1-o(ox_free)
    o(ox_left)=(100*o(ox_locs))/(ox_mnxbf_base-1-ox_mbf_base)
    call mesage(13,1,d,0)
    return
end subroutine krunc
!_______________________________________________________________________
!__________________________________________
! Parse a signed integer from the buffer in the specified base.
integer function nib(l,b)
    ! l: Starting buffer index; moved backward to the character preceding the parsed integer.
    ! b: Radix used to interpret the digits.
    implicit none
    include "mortran3_indices.f95"
    integer :: l,b
    integer :: o(ox_capacity)
    common /ibm/o
    integer :: o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
    equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
        (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
    integer :: m,n
    save    m,n
    m=1
    n=0
    ! Skip blanks before the number, then accumulate digits from least to
    ! most significant as they are stored in the buffer.
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
    ! The internal sign marker immediately before the digits makes the value negative.
    if (o(l)==42) then
        l=l-1
        n=-n
    end if
    nib=n
    return
end function nib
!_______________________________________________________________________
!__________________________________________
! Convert an integer to the specified base in the output buffer.
integer function llong(loc,num,b,ind)
    ! loc: Buffer index immediately before the formatted result.
    ! num: Integer value to format.
    ! b: Radix used to format the value.
    ! ind: Set to zero to prefix a blank; nonzero omits the leading blank.
    implicit none
    include "mortran3_indices.f95"
    integer :: loc,num,b,ind
    integer :: o(ox_capacity)
    common /ibm/o
    integer :: o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
    equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
        (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
    integer :: n,l,i,j,k
    save    n,l,i,j,k
    if(o(ox_ngn)/= 0)return
    n=abs(num)
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
! Evaluate a Mortran expression and return its truth value.
integer function lexp(a,z)
    ! Evaluate an encoded evaluator expression and return its integer result;
    ! comparisons and logical operations produce 0 or 1.
    ! Quoted operands use a separate literal/specifier matching path.
    ! a: First index of the expression in the shared buffer.
    ! z: Last index of the expression in the shared buffer (inclusive).
    implicit none
    include "mortran3_indices.f95"
    integer :: a,z
    integer :: o(ox_capacity)
    common /ibm/o
    integer :: o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
    equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
        (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
    integer :: p(100),n
!FGHIJKLMQRSUVWY
    equivalence(o(ox_nsk),n),( o(ox_pjt_base),p(1) )
    integer :: ispec
    integer :: b,c,nq,mxc,d,e,lone,mp,mq,mv,mu,mnn,t,x,k,jmp
    save    b,c,nq,mxc,d,e,lone,mp,mq,mv,mu,mnn,t,x,k,jmp
    ! If the expression starts with a quoted token, compare quoted values
    ! directly instead of sending them through the numeric operator parser.
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
        ! d points to the operator after the left quoted value; e is the first
        ! token of the right operand.
        d=c+1
        e=d+1
        if (o(e)==o(ox_lq)) then
            ! Two quoted operands are compared token-for-token and must have
            ! equal encoded lengths.
            do while(e<=z-1)
                if(o(b)/= o(e))lexp=0
                b=b+1
                e=e+1
            end do
            if(b/= d) lexp=0
        else
            ! Otherwise treat the right operand as a specifier and match it
            ! against the contents of the left quoted value.
            lone=1
            mp=e
            mq=z
            mv=b+1
            mu=c
            if(ispec(mp,mq,mv,mu,lone)/= 3)lexp=0
        end if
        ! The encoded not-equal form complements the comparison result.
        if(o(d)==89) lexp=1-lexp
        return
    end if
    ! Numeric expressions use a shared value/operator stack. Start with a
    ! false result, an empty numeric flag, and a left-parenthesis sentinel.
    lexp=0
    o(ox_nflg)=0
    n=ox_nsk_base
    mnn=n+2
    o(n-1)=38
    o(n)=0
    b=a
    do
        if(b>z)exit
        if (o(b)>=0 .and. o(b)<=9) then
            ! Accumulate consecutive digit tokens into one integer operand.
            o(n)=0
            o(ox_nflg)=1
            do while(0<=o(b).and.o(b)<=9)
                o(n)=10*o(n)+o(b)
                b=b+1
            end do
            cycle
        end if
        if (o(b)==o(ox_bla)) then
            ! Blanks do not participate in expression evaluation.
            b=b+1
            cycle
        end if
        t=b
        if (o(t)>=10 .and. o(t)<=29) then
            ! Some operators are encoded across multiple adjacent tokens; map
            ! their packed spelling to the operator-table index in x.
            x=o(t)*o(t+1)
            t=t+2
            if (o(t)==29.or.o(t)==13) then
                x=x+o(t)
                t=t+1
            end if
            x=x/20
            if (.not.((10<x).and.(x<33))) exit
        else
            ! Single-token operators are mapped to the same table-index form;
            ! several letter-coded forms have optional second-token variants.
            x=o(t)
            t=t+1
            select case (x)
            case (45)
                ! Map "**" to internal exponent index 36; "*" stays index 45.
                if (o(t)==45) then
                    t=t+1
                    x=36
                end if
            case (89)
                ! Map "~" to logical NOT index 29, or "~=" to inequality index 16.
                x=29
                if (o(t)==41) then
                    t=t+1
                    x=16
                end if
            case (85)
                ! Map "<" to index 30, or "<=" to less-or-equal index 14.
                x=30
                if (o(t)==41) then
                    t=t+1
                    x=14
                end if
            case (41)
                ! Map "=" to equality index 18, or "=>" to greater-or-equal 11.
                x=18
                if (o(t)==86) then
                    t=t+1
                    x=11
                end if
            case (86)
                ! Map ">" to index 23, or ">=" to greater-or-equal index 11.
                x=23
                if (o(t)==41) then
                    t=t+1
                    x=11
                end if
            case (87)
                ! Map "|" to logical OR index 32.
                x=32
            case (88)
                ! Map "&" to operator-table index 13.
                x=13
            end select
        end if
        ! p(x) encodes operator precedence in its hundreds and the reduction
        ! operation in the remaining digits.
        if(p(x)==0)exit
        ! Reject unsupported tokens and operators that cannot follow the
        ! current operand state (notably a right parenthesis without an operand).
        if(x<10.or.x>o(ox_rpr) .or. (x==o(ox_rpr).and.o(ox_nflg)==0)) &
            exit
        if (p(x)/100>p(o(n-1))/100 .or. x==o(ox_lpr)) then
            ! A higher-precedence operator, or a left parenthesis, is pushed
            ! with a fresh operand slot; otherwise reduce the pending operator.
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
            ! The sentinel is now the only pending operator, so the current
            ! operand is the complete expression value.
            lexp=o(n)
            return
        end if
        k=p(o(n-1))
        o(ox_nflg)=1
        jmp=k-((k/100)*100)-1
        ! Reduce one pending operation. Arithmetic results remain integers;
        ! logical and comparison results are normalized to 0 or 1.
        select case (jmp)
        case (1)
            ! Close a parenthesized operand, retain its value, and resume after ')'.
            o(n-2)=o(n)
            b=t
        case (2)
            ! Logical OR: true only when either operand is exactly 1.
            if (o(n-2)==1 .or. o(n)==1) then
                o(n-2)=1
            else
                o(n-2)=0
            end if
        case (3)
            ! Logical AND: true only when both operands are exactly 1.
            if (o(n-2)==1 .and. o(n)==1) then
                o(n-2)=1
            else
                o(n-2)=0
            end if
        case (4)
            ! Unary logical NOT, represented as 1 minus the operand.
            o(n-2)=1-o(n)
        case (5)
            ! Subtract the right operand from the left operand.
            o(n-2)=o(n-2)-o(n)
        case (6)
            ! Add the right operand to the left operand.
            o(n-2)=o(n-2)+o(n)
        case (7)
            ! Divide the left operand by the right operand using integer division.
            if (o(n)==0) then
                ! Division by zero is diagnosed before attempting the reduction.
                call mesage(14, 10, 0,0)
                exit
            end if
            o(n-2)=o(n-2)/o(n)
        case (8)
            ! Multiply the two operands.
            o(n-2)=o(n-2)*o(n)
        case (9)
            ! Raise the left operand to the right operand's power.
            o(n-2)=o(n-2)**o(n)
        case (10)
            ! Greater-than-or-equal comparison.
            if (o(n-2)>=o(n)) then
                o(n-2)=1
            else
                o(n-2)=0
            end if
        case (11)
            ! Less-than-or-equal comparison.
            if (o(n-2)<=o(n)) then
                o(n-2)=1
            else
                o(n-2)=0
            end if
        case (12)
            ! Inequality comparison.
            if (o(n-2)/=o(n)) then
                o(n-2)=1
            else
                o(n-2)=0
            end if
        case (13)
            ! Equality comparison.
            if (o(n-2)==o(n)) then
                o(n-2)=1
            else
                o(n-2)=0
            end if
        case (14)
            ! Strict greater-than comparison.
            if (o(n-2)>o(n)) then
                o(n-2)=1
            else
                o(n-2)=0
            end if
        case (15)
            ! Strict less-than comparison.
            if (o(n-2)<o(n)) then
                o(n-2)=1
            else
                o(n-2)=0
            end if
        case default
            ! Fallback integer-division operation; guard against a zero divisor.
            if (o(n)==0) then
                call mesage(14, 10, 0,0)
                exit
            end if
            o(n-2)=o(n-2)/o(n)
        end select
        ! The right operand and operator have been consumed; continue with the
        ! reduced value in the left operand slot.
        n=n-2
    end do
    ! Reaching the end without reducing to one value means the expression was
    ! malformed or contained an unsupported operator.
    call mesage(14, 23, a,z+1)
    return
end function lexp
!_______________________________________________________________________
!________________________________________
! Read the next source record and prepare it for translation.
subroutine nxtcrd
    implicit none
    include "mortran3_indices.f95"
    integer :: o(ox_capacity)
    common /ibm/o
    integer :: o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
    equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
        (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
    integer :: mxr,mxt,mxn
    equivalence(o(ox_rmx),mxr),(o(ox_tmx),mxt),(o(ox_nmx),mxn)
    integer :: llong,linf
    integer :: k,j,w,i,l,m,y,nerd
    save    k,j,w,i,l,m,y,nerd
    ! Reset per-record macro accounting; EOF state 2 means all input is already exhausted.
    o(ox_mcnt)=0
    if(o(ox_eof)==2) call mesage(16,1,0,0)
    do
        ! Read the next physical record from the active input unit.
        call rw(0,o(ox_unit),ox_rbf_base,mxr)
        if (o(ox_eof)==1) then
            ! Resume the previous source unit, or signal end of all input when the stack is empty.
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
        ! Map raw input characters to the internal codes used by the scanner.
        do k=ox_rbf_base,mxr
            o(k+132)=o(ox_trt_base+o(k))
        end do
        ! Count this record and initialize the translated-record cursors.
        o(ox_seqn)=o(ox_seqn)+1
        o(ox_crs)=ox_tbf_base
        o(ox_gbf_base)=o(ox_bla)
        ! A record beginning with '%' (token 93) is a control card, not source
        ! text. Its second character selects a special action or a cccard setting.
        if (o(ox_tbf_base) == 93) then
            if (o(ox_t2_base)==14) then
                ! %E ejects a listing page: set the page-control byte, blank
                ! its two-character prefix, and advance past the directive.
                o(ox_gbf_base)=1
                o(ox_rbf_base)=o(ox_oset_base+o(ox_bla))
                o(ox_rbf_base+1)=o(ox_rbf_base)
                o(ox_crs)=ox_t2_base+1
            else if (o(ox_t2_base)==93) then
                ! %% marks end of input; EOF handling below resumes any stacked unit.
                o(ox_eof)=1
                o(ox_frt)=1
            else if (o(ox_t2_base)>=10.and.o(ox_t2_base)<=30) then
                ! Letters A..U map to tokens 10..30; subtract 9 to get cccard's
                ! selector 1..21. The selector reads any numeric suffix via n5.
                call cccard(o(ox_t2_base)-9)
                o(ox_crs)=o(ox_sigt)
                o(o(ox_crs))=o(ox_trm)
            end if
            if (o(ox_eof)==1.or.o(ox_t2_base)==22.or.o(ox_t2_base)==15) then
                ! %% ends input; %F and %M switch modes and end this read here.
                o(ox_crs)=o(ox_sigt)
                o(o(ox_crs))=o(ox_trm)
                return
            end if
        end if
        if (o(ox_list)==1) then
            ! Build the listing form of this record in the listing buffer.
            do j=ox_g2_base,ox_w0_base
                o(j)=o(ox_bla)
            end do
            if (o(ox_seqf)==1) then
                ! Emit a generated sequence number in the listing prefix.
                j=llong(ox_g2_base,o(ox_seqn),10,0)
            else if (o(ox_seqf)==2) then
                ! Or preserve the source record's fixed-format sequence field.
                do j = 1,8
                    o(ox_gbf_base+j)=o(o(ox_sigt)+j)
                end do
            end if
            if(o(ox_ngn)/= 0)o(ox_g11_base)=33
            o(ox_g13_base)=o(ox_qfl)
            j=llong(ox_g14_base,o(ox_lvl),10,0)
            if (o(ox_indm)==0) then
                ! Without continuation indentation, write the complete record
                ! through the configured listing conversion.
                w=ox_w0_base
                do k = 1,mxn
                    w=w+1
                    o(w)=o(ox_t0_base+k)
                end do
                call rw(18,o(ox_mlst),ox_gbf_base,w)
            else
                ! With indentation enabled, trim edge blanks and emit the record
                ! in bounded segments, prefixing each segment with its indent.
                i=mod(max(o(ox_indm)*o(ox_lvl),0),90)
                l=ox_tbf_base
                m=o(ox_sigt)
                do while(o(l)==o(ox_bla).and.l/=m)
                    l=l+1
                end do
                do while(o(m)==o(ox_bla).and.l/=m)
                    m=m-1
                end do
                do
                    w=ox_wbf_base
                    do y = 1,i
                        o(w)=o(ox_bla)
                        w=w+1
                    end do
                    do while(w<=288.and.l<=m)
                        o(w)=o(l)
                        w=w+1
                        l=l+1
                    end do
                    call rw(18,o(ox_mlst),ox_gbf_base,w-1)
                    if(l>m)exit
                    do k=ox_gbf_base,ox_wbf_base
                        o(k)=o(ox_bla)
                    end do
                end do
            end if
        end if
        if (o(ox_frt)==1) then
            ! Forced listing writes the original record; annotation mode instead
            ! emits the translated record with source-position markers.
            if (o(ox_tbf_base)/= 93) call rw(23,o(ox_flst),ox_rbf_base,ox_r8_base)
        else if (o(ox_ano)==1) then
            nerd=linf(ox_tbf_base,mxt,10)
        end if
        ! Forced-listing/control records are consumed here and never reach the
        ! main translator's source scanner.
        if(o(ox_frt)==1)cycle
        if (o(ox_cset)<2) return
        ! When alternate delimiters are enabled, map (* ... *) to braces and
        ! <* ... *> to square brackets in the translated record.
        j=ox_t0_base
        k=ox_tbf_base
        do while(k<=mxt)
            j=j+1
            o(j)=o(k)
            k=k+1
            if (o(j)==45) then ! 45 = *
                if (o(k)==46) then ! 46 = )
                    o(j)=o(ox_rcb)
                    k=k+1
                else if (o(k)==86) then ! 86 = >
                    o(j)=o(ox_rsb)
                    k=k+1
                end if
            else if (o(k)==45) then ! 45 = *
                if (o(j)==38) then ! 38 = (
                    o(j)=o(ox_lcb)
                    k=k+1
                else if (o(j)==85) then ! 85 = <
                    o(j)=o(ox_lsb)
                    k=k+1
                end if
            end if
        end do
        ! Pad the remainder of the fixed-size translated-record area with blanks.
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
! Apply settings from a Mortran control card.
subroutine cccard(k)
    ! k: Control-card selector identifying the setting to apply.
    implicit none
    include "mortran3_indices.f95"
    integer :: k
    integer :: o(ox_capacity)
    common /ibm/o
    integer :: o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
    equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
        (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
    integer :: n5
    integer :: jmp,i
    save    jmp,i
    ! nxtcrd maps the letter token in %A..%U (10..30) to selector k=1..21;
    ! %E is handled there as a listing page eject instead of reaching this table.
    jmp=k
!ABCDEFGHJLMNPQRSTUVWXYZ
    select case (jmp)
    case (1) ! %A: set Mortran-source annotation mode.
        o(ox_ano)=n5(0,2,1)
    case (2) ! %B: set the raw and translated record-buffer limit.
        o(ox_nmx)=n5(0,132,132)
        o(ox_rmx)=ox_r0_base+o(ox_nmx)
        o(ox_tmx)=ox_t0_base+o(ox_nmx)
    case (3) ! %C: set the active input line width.
        o(ox_sign)=n5(10,132,132)
        o(ox_sigt)=ox_t0_base+o(ox_sign)
        o(ox_sigr)=ox_r0_base+o(ox_sign)
    case (4) ! %D: set macro-definition tracing level.
        o(ox_mdef)=n5(0,2,0)
    case (6) ! %F: enter Fortran mode.
        o(ox_frt)=1
    case (9) ! %I: set listing indentation per nesting level.
        o(ox_indm)=n5(0,50,2)
    case (12) ! %L: turn the Mortran listing on.
        o(ox_list)=1
    case (13) ! %M: return to Mortran mode.
        o(ox_frt)=0
    case (14) ! %N: turn the Mortran listing off.
        o(ox_list)=0
    case (17) ! %Q: set quoted-comment continuation mode.
        o(ox_lazy)=n5(0,1,1)
    case (18) ! %R: rewind the input unit selected by the numeric suffix.
        i=n5(1,99,8)
        rewind i
    case (19) ! %S: set source-sequence-field handling mode.
        o(ox_seqf)=n5(0,2,0)
    case (20) ! %T: set macro-tracing level.
        o(ox_mtrc)=n5(0,9,0)
    case (21) ! %U: push the current unit and switch to the requested input unit.
        o(ox_usk)=o(ox_usk)+1
        o(o(ox_usk))=o(ox_unit)
        if(o(ox_usk)>2504)call mesage(15, 30, 0,0)
        o(ox_unit)=n5(1,99,o(ox_unit))
    case (5,7,8,10,11,15,16) ! Unsupported here; %E is handled specially by nxtcrd.
        call mesage(14, 22, 0,0)
    case default ! Out-of-range selector: read it as an annotation-mode value.
        o(ox_ano)=n5(0,2,1)
    end select
    return
end subroutine cccard
!_______________________________________________________________________
!__________________________________________
! Read a decimal suffix from the current control card, defaulting if out of range.
integer function n5(l,m,k)
    ! l: Minimum allowed value.
    ! m: Maximum allowed value.
    ! k: Default value returned when the parsed value is outside the allowed range.
    implicit none
    include "mortran3_indices.f95"
    integer :: l,m,k
    integer :: o(ox_capacity)
    common /ibm/o
    integer :: o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
    equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
        (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
    integer :: nib
    integer :: j
    save    j
    j=ox_tbf_base+4
    n5=nib(j,10)
    if (.not.((l<=n5).and.(n5<=m))) then
        call mesage(14, 3, 0,0)
        n5=k
    end if
    return
end function n5
!_______________________________________________________________________
!________________________________________
! Write macro-processing trace messages to the listing.
subroutine mactrc(w,r,s)
    ! w: Trace-event selector.
    ! r: First buffer position or value used by the selected trace event.
    ! s: Second buffer position or limit used by the selected trace event.
    implicit none
    include "mortran3_indices.f95"
    integer :: w,r,s
    integer :: o(ox_capacity)
    common /ibm/o
    integer :: o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
    equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
        (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
    integer :: k,m,t,n
    save    k,m,t,n
    ! Suppress most nested trace details at low trace levels, but retain
    ! selector 4 diagnostics.
    if (o(ox_nest)>0.and.o(ox_mtrc)<3.and.w/= 4)return
    ! Emit progress markers for attempts, unexpected input, and retries.
    select case (w)
    case (2)
        call dumdum(ox_wbf_base,90,37,37,37,22,42,29,27,34,90) ! " ___M-TRY "
        o(ox_wbf_base+4)=(o(ox_base)-ox_ct0_base)/120
        call crit(r,s)
    case (4)
        call dumdum(ox_wbf_base,90,45,28,29,27,10,23,16,14,90) ! " *STRANGE "
        call crit(r,s)
    case (5)
        call dumdum(ox_wbf_base,90,37,10,16,10,18,23,28,29,90) ! " _AGAINST "
        call crit(r,s)
    end select
    ! Emit detailed events for reductions, definitions, rule updates, and
    ! successful matches (including captured arguments for selector 22).
    select case (w)
    case (33)
        if (o(ox_nest)==0) then
            call dumdum(ox_wbf_base,90,90,27,14,13,30,12,14,13,90) ! "  REDUCED "
        else
            call dumdum(ox_wbf_base,90,90,90,90,90,23,14,28,29,90) ! "     NEST "
            o(ox_wbf_base+3)=o(ox_nest)
        end if
        call crit(r+1,s+1)
    case (15)
        call dumdum(ox_wbf_base,90,90,13,14,15,18,23,14,13,90) ! "  DEFINED "
        call crit(r+4,o(r+3))
        if (r+4>=o(r+3)) then
            call dumdum(ox_wbf_base,90,23,30,21,21,90,31,10,21,90) ! " NULL VAL "
            call crit(s,s)
        else
            call dumdum(ox_wbf_base,90,90,90,90,31,10,21,30,14,90) ! "    VALUE "
            o(ox_wbf_base+2)=o(ox_typ)
            call crit(s+4,o(s+3))
        end if
    case (12)
        call dumdum(ox_wbf_base,90,10,25,25,14,23,13,14,13,90) ! " APPENDED "
        call crit(o(s-2*(3)-1),o(s-2*(3)-2) )
        call dumdum(ox_wbf_base,90,90,90,90,90,90,90,29,24,90) ! "       TO "
        call crit(o(s-2*(1)-1),o(s-2*(1)-2) )
    case (13)
        call dumdum(ox_wbf_base,90,90,27,14,22,24,31,14,13,90) ! "  REMOVED "
        call crit(o(s-2*(1)-1),o(s-2*(1)-2) )
    case (22)
        if(o(r+4)<0)return
        call dumdum(ox_wbf_base,90,90,22,10,29,12,17,14,13,90) ! "  MATCHED "
        call crit(r+4,o(r+3) )
        k=o(s-1)
        do m = 1,k
            call dumdum(ox_wbf_base,90,90,90,90,10,27,16,90,90,90) ! "    ARG   "
            o(ox_wbf_base+8)=m
            t=o(s-2*(m)-1)
            n=o(s-2*(m)-2)
            if (t/= n) call crit(t,n)
        end do
    end select
    return
end subroutine mactrc
!_______________________________________________________________________
!__________________________________________
! Format and write a text fragment to the Mortran listing.
subroutine crit(a,z)
    ! a: First index of the text fragment in the shared buffer.
    ! z: Exclusive end index of the text fragment.
    implicit none
    include "mortran3_indices.f95"
    integer :: a,z
    integer :: o(ox_capacity)
    common /ibm/o
    integer :: o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
    equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
        (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
    integer :: b,c
    save    b,c
    b=a
    ! Copy the text into the listing workspace in chunks that fit its buffer.
    do while (b<z)
        c=ox_wbf_base+9
        do while (c<ox_w72_base.and.b<z)
            c=c+1
            o(c)=o(b)
            b=b+1
            ! Convert internal blanks and marker values to listing-safe codes.
            if(o(c)==o(ox_bla))o(c)=37
            if(o(c)>100+19)o(c)=100
            if(o(c)==-1)o(c)=74
            if(o(c)==-2)o(c)=82
        end do
        ! Mode 33 applies the alternate character map used by listing output.
        call rw(33,o(ox_mlst),ox_wbf_base,c)
        call dumdum(ox_wbf_base,90,90,90,90,90,90,90,90,90,90) ! "          "
    end do
    return
end subroutine crit
!_______________________________________________________________________
!________________________________________
! Report diagnostics and terminate when the message level requires it.
subroutine mesage (level,no,k1,k2)
    ! level: Diagnostic severity and message group.
    ! no: Message number within the selected group.
    ! k1: Value or buffer position supplied to the message template.
    ! k2: Optional end position for a macro trace; zero suppresses that trace.
    implicit none
    include "mortran3_indices.f95"
    integer :: level,no,k1,k2
    integer :: o(ox_capacity)
    common /ibm/o
    integer :: o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
    equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
        (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
    common/junk/ verbose
    logical :: verbose
    ! Count warning/error groups before rendering the selected message.
    if(level==14 .or. level==15)o(ox_merr)=o(ox_merr)+1
    call find(level,no,k1)
    ! Attach a macro trace when the caller provides a nonempty source range.
    if(k2/= 0)call mactrc(4,k1,k2)
    if (level==13) then
        call find(13,2,o(ox_locs))
        call find(13,3,o(ox_left))
    end if
    if (level<=14) then
        ! Let nonfatal diagnostics return to the translator, but stop once the
        ! counted messages reach one hundred.
        if(o(ox_merr)<100)return
        call find(15,10,0)
    end if
    ! On finalization, report unterminated quotes and unclosed definition fields.
    if (o(ox_qfl)==o(ox_bq) .or. o(ox_qfl)==o(ox_dq)) then
        o(ox_merr)=o(ox_merr)+1
        if(o(ox_qfl)==o(ox_bq)) call find(16,5,0)
        if(o(ox_qfl)==o(ox_dq))call find(16,4,0)
    end if
    if(o(ox_lsk)>ox_lsk_base.or.o(ox_msk)>ox_msk_base)o(ox_merr)=o(ox_merr)+1
    if (o(ox_msk)>ox_msk_base) then
        call find(16,3,o(ox_msk)-ox_msk_base)
    end if
    do while (o(ox_lsk)>ox_lsk_base)
        call find(16,2,o(o(ox_lsk)))
        o(ox_lsk)=o(ox_lsk)-1
    end do
    call find(13,3,o(ox_left))
    if (o(ox_merr)==0) then
        call find(16,6,0)
    else
        ! Any accumulated error prevents success and terminates compilation.
        call find(16,7,o(ox_merr))
        write(6,*) 'compilation error (stop 12)'
        call exit(12) !stop 12
    end if
    ! Table-generation mode writes the packed table from inital before exit.
    if (o(ox_mode)==0)call inital(2)
    if( verbose ) write(6,*) 'OK'
    call exit(0) !stop 0
end subroutine mesage
!_______________________________________________________________________
!__________________________________________
! Look up a message template and write the diagnostic with its values.
subroutine find(l,n,v)
    ! l: Diagnostic message group.
    ! n: Message number within the group.
    ! v: Numeric value inserted into messages that accept a value.
    implicit none
    include "mortran3_indices.f95"
    integer :: l,n,v
    integer :: o(ox_capacity)
    common /ibm/o
    integer :: o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
    equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
        (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
    integer :: llong
    integer :: a,c,q,r,s,nerd,k
    save    a,c,q,r,s,nerd,k
    a=ox_wbf_base
    c=a+9
    ! Follow the selected message group's table chain until its number matches.
    q=ox_ct6_base+l
    do
        if (o(q)==0) then
            ! If no template is registered, print the message number itself.
            c=c+llong(c,n,10,0)
            exit
        end if
        q=o(q)
        if(o(q+5)/=n)cycle
        r=o(q+2)+4
        s=o(o(q+2)+3)
        do while (r<s)
            c=c+1
            o(c)=o(r)
            r=r+1
        end do
        exit
    end do
    ! Prefix fatal and warning groups; informational groups may include a value.
    if(l==15) call dumdum(ox_wbf_base,90,45,45,45,15,10,29,10,21,90) ! " ***FATAL "
    if(l==14) call dumdum(ox_wbf_base,90,45,32,10,27,23,18,23,16,90) ! " *WARNING "
    if (l==13.or.l==16) then
        call dumdum(ox_wbf_base,90,90,90,90,90,90,90,90,90,90) ! "          "
        if(v/= 0) nerd=llong(a,v,10,0)
    end if
    ! Always write the mapped listing; mirror diagnostics to the other output
    ! stream once an error has been recorded.
    call rw(33,o(ox_mlst),a,c)
    if (o(ox_merr)>0) call rw(23,o(ox_flst),a,c)
    do k = a,c
        o(k)=o(ox_bla)
    end do
    c=ox_wbf_base+8
    return
end subroutine find
!_______________________________________________________________________
!________________________________________
! Format and write source text, comments, and rule diagnostics to the listing.
integer function linf(a,z,ind)
    ! a: First index of the source text in the shared buffer.
    ! z: Last index of the source-text range (inclusive).
    ! ind: Output mode, including packed-comment and rule-diagnostic modes.
    implicit none
    include "mortran3_indices.f95"
    integer :: a,z,ind
    integer :: o(ox_capacity)
    common /ibm/o
    integer :: o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
    equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
        (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
    integer :: llong
    integer :: last,kur,kit,kut,kst,h,b,i,j,npl,d,koff,c,limc,k
!     LINF modes: packed comments and rule diagnostics.
    integer, parameter :: linf_comment_mode = 12
    integer, parameter :: linf_rule_diagnostic_mode = 15
    data last/0/,kur/0/,kit/1/,kut/0/
    save    last,kur,kit,kut,kst,h,b,i,j,npl,d,koff,c,limc,k
    linf=a-1
!CEFGLMNPQRS UVWXY
    o(ox_mcnt)=0
    ! Skip packed-comment output when comments are disabled, annotation mode is
    ! active, or the range already starts with a closing-comment marker.
    if (ind==linf_comment_mode .and. (o(ox_pcom)==0.or.o(ox_ano)==1 &
                                      .or.o(ox_rdq)==o(a)))return
    kst=0
    h=0
    b=a
    i=ox_wbf_base
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
    do j = ox_wbf_base,ox_w80_base
        o(j)=o(ox_bla)
    end do
    if (ind==linf_rule_diagnostic_mode) then
        ! Rule diagnostics have a narrower text field and a dedicated prefix.
        do while (i<ox_w7_base .and. b<=z)
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
        ! Source and comment lines use the configured indentation and width.
        npl=71
        d=12
        o(ox_wbf_base)=d
        koff=max(min(80,o(ox_indc)),2)
    end if
    if (b>z .and. i>ox_wbf_base) then
        call rw(33,o(ox_flst),ox_wbf_base,ox_w7_base)
        if(ind==linf_rule_diagnostic_mode)call mesage(14, 14, 0,0)
        return
    end if
    c=ox_w0_base+koff+kut*o(ox_indf)
    if(kut>last)c=c-o(ox_indf)
    if(c>ox_w0_base+50)c=ox_w0_base+7
    i=c
    do while (b<=z)
        ! Fill one listing record, stopping at its capacity or a source EOL.
        limc=min(b+npl,z)
        do while (b<=limc .and. i<=ox_w72_base)
            if (ind==linf_rule_diagnostic_mode) then
                if (o(ox_holr)==1) then
                    if (o(b)==o(ox_lq)) then
                        if (i>ox_w72_base-4) exit
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
                o(ox_w80_base-k+1)=o(o(ox_tmx)-k+1)
            end do
        else if (o(ox_seqf)==2) then
            k=llong(ox_w73_base,o(ox_seqn),10,0)
        end if
        do k=ox_wbf_base,ox_w72_base
            if (o(k)==o(ox_lq)) then
                o(k)=o(ox_bq)
                kst=1
            end if
        ! Write the prepared record directly, then clear the workspace for the
        ! next segment of a long source range.
            if (o(k)==o(ox_rq)) then
                o(k)=o(ox_bq)
                kst=0
            end if
        end do
        if (i>ox_w72_base .and. ind==linf_rule_diagnostic_mode &
            .and. kst==0) then
            do while (o(i-1)<38 .and. kst>0)
                b=b-1
                i=i-1
                o(i)=o(ox_bla)
            end do
        end if
        do k = ox_wbf_base,ox_w80_base
            if(0>o(k).or.o(k)>100-1)o(k)=100-1
            o(k)=o(ox_oset_base+o(k))
        end do
!   LOOKOUT
        call rw(23,o(ox_flst),ox_wbf_base,ox_w80_base)
        do k = ox_wbf_base,ox_w80_base
            o(k)=o(ox_bla)
        end do
        i=c
        if (ind==linf_rule_diagnostic_mode) then
            o(ox_wbf_base+5)=d
        else
            o(ox_wbf_base)=d
        end if
        if (kst>0) then
            i=ox_w7_base
            o(ox_w7_base-1)=45
        end if
    end do
    last=kur
    ! Mark an emitted packed comment so cout will not list it again.
    if(ind==linf_comment_mode) o(a)=o(ox_rdq)
    return
end function linf
!_______________________________________________________________________
!__________________________________________
! Store ten values consecutively starting at index l.
subroutine dumdum(l,a,b,c,d,e,f,g,h,i,j)
    ! l: First destination index in the shared buffer.
    ! a-j: Ten values stored consecutively beginning at l.
    implicit none
    include "mortran3_indices.f95"
    integer :: l,a,b,c,d,e,f,g,h,i,j
    integer :: o(ox_capacity)
    common /ibm/o
    integer :: o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
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
! Read source records and write translated or listing output.
subroutine rw(n,i,j,k)
    ! Read one source record or write a buffer range using the selected character map.
    ! n: Zero reads; 23 writes raw character values; other values write through a map,
    !    with 33 selecting the alternate map bank used by trace/listing output.
    ! i: Fortran I/O unit to read from or write to.
    ! j: First index of the buffer range.
    ! k: Last index of the buffer range (inclusive).
    implicit none
    include "mortran3_indices.f95"
    integer, parameter :: rw_buffer_size = 200
    integer :: n,i,j,k
    character(len=1) :: ch(rw_buffer_size)
    integer :: hc(rw_buffer_size)
    integer :: o(ox_capacity)
    common /ibm/o
    integer :: o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
    equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
        (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
!  IN RW
!
    common/nrcc/ oflag
    integer :: oflag
!
    logical :: skip_input,is_format,is_quote,is_quote_last
    integer :: nr,m,kof,l,nrr,nformat,itmp,nfl
    integer :: nn6,nn7,jj,nfirst,ii,m1
    character(len=8) :: tmp_string
    character(len=6) :: tmp1_string
    character(len=1) :: f_buffer(8192)
    equivalence (ch,tmp_string)
    data nn6,nn7,nfl/0,0,0/, &
        skip_input/.false./, is_format/.false./, is_quote/.false./
    data nformat/0/
    save nn6,nn7,hc,ch,nformat,nfl
    save nr,m,kof,l,nrr,skip_input,is_format,is_quote
    save f_buffer

    ! Number of buffer elements requested; the range endpoints are inclusive.
    nr=k+1-j
    if (n==0) then
        !write(44,*) 'nr = ',nr,k,j,i
        ! Read source characters from the selected unit into the buffer.
        read(i,1,end=5130) (ch(m),m=1,nr)
        if( i/= 1 ) then
            ! Strip C-style block comments from non-raw input units. The
            ! skip flag persists across records so multi-line comments work.
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
        ! Keep raw character codes in the shared buffer; nxtcrd performs the
        ! Mortran token-map conversion after this read.
        do m=1,nr
            o(j+m-1)=ichar(ch(m))
        end do
        return
    end if
    ! Select the output character map: normal bank, alternate bank for mode 33,
    ! or no mapping for mode 23.
    kof=ox_iset_base
    if (n==33) kof=100+kof
    if (n==23) kof=0
    do l=1,nr
        hc(l) = min(255,max(0, o(j+l-1) ))
        if (kof>0) hc(l)=o(hc(l)+kof)
        ch(l)=char( hc(l)  )
    end do
    if( i==oflag ) then
        ! Skip the first write to the designated listing unit.
        nn6 = nn6+1
        if( nn6==1 ) return
    end if
    if( i==7 ) then
        ! Likewise suppress the initial record on the generated-source unit.
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

    ! Move a C-preprocessor directive's first nonblank character (#) to
    ! column 1, since generated-code consumers expect it there.
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
        do ii=nrr+2-jj,rw_buffer_size
            ch(ii) = ' '
        end do
        nrr = nrr + 1 - jj
        ! C-preprocessor include paths conventionally use double quotes.
        if( tmp_string=='#include' ) then
            do ii=1,nrr
                if( ichar(ch(ii))==39 ) ch(ii) = char(34)
            end do
        end if
    endif

    if(i==7) then
        ! Unit 7 receives fixed-form Fortran. FORMAT statements may exceed a
        ! record, so buffer the statement until parentheses balance outside
        ! quoted character strings, then emit it using continuation records.
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
                ! The buffered FORMAT statement is complete and can be emitted.
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
            ! Detect FORMAT in the statement field after the six-column label.
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
            ! Count parentheses in the statement field, ignoring quoted text.
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

    ! Write the converted character range, omitting trailing blanks.
    write(i,1) (ch(m),m=1,nrr)
1   format(132a1)

!
!        NRC PATCH TO PUT DATE AND TIME AT EACH PAGE THROW
!
    if( ch(1)=='1' .and. i==6 ) then
        ! Add a timestamp after a listing page eject.
        write(i,'(T55)',advance='no')
        call egs_fdate(i)
        write(i,*)
    end if
    return
5130 o(ox_eof)=1
    ! Signal end-of-input to the translator's shared state.
    return
end subroutine rw
!_______________________________________________________________________
!________________________________________
! Load raw or packed Mortran tables and initialize translator state.
! With ind=2, write the generated table to unit 7 and terminate.
subroutine inital(ind)
    ! ind: Initialization mode; 2 writes the generated table and terminates.
    implicit none
    include "mortran3_indices.f95"
    integer :: ind
    character(len=1) :: count
    integer :: o(ox_capacity)
    common /ibm/o
    integer :: o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
    equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
        (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
    common /junk/ verbose
    logical :: verbose
    integer :: kount,k,iuni
    save    kount,k,iuni,count
    if (ind==2) then
        ! Serialize the in-memory table in packed-table format, then exit.
        o(ox_mode)=1
        o(ox_seqn)=5
        kount=o(ox_free)+1
        o(ox_frt)=0
        o(ox_unit)=5
        write(7,1)kount,(o(k),k=1,kount)
1       format(10z8)
2       format(a1)
        if( verbose ) write(6,*) 'OK'
        call exit(0) !stop 0
    end if
    ! Clear table and workspace storage while preserving the six reserved slots.
    do k = 7,ox_capacity
        o(k)=0
    end do
    o(ox_unit)=o_b
    o(ox_mlst)=o_e
    o(ox_flst)=o_r
    iuni=o(ox_unit)
!     O(14)=O(4)   THIS IS NO LONGER USED IN FORTRAN 77 VERSIONS
    o(ox_labl)=o_u
    ! Raw tables begin with the marker C; other files contain a packed table.
    read(iuni,2)count
    rewind iuni
    if (count=='C') then
        call raw()
    else
        ! Load the packed table and copy its saved listing records to the
        ! listing and translated-output units before rewinding the source.
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
        call rw(23,o(ox_flst),ox_rbf_base+3,ox_rbf_base+79)
        call rw(23,o(ox_mlst),ox_rbf_base+2,ox_rbf_base+79)
        rewind iuni
    end if
    ! Establish the translator's allocation boundaries and initial sentinels.
    o(ox_qfl)=o(ox_bla)
    o(ox_mxx)=ox_home_base
    o(ox_base)=ox_ct4_base
    do k = 1,6
        o(k)=ox_home_base
    end do
    o(ox_rtt)=-11
    o(ox_ltt)=-12
    o(ox_icur)=-13
    o(ox_eol)=-14
    o(o_u)=o(ox_trm)
    o_w=ox_mnxbf_base-1
    o(o_w)=o(ox_trm)
    call dumdum(44,ox_isk_base,ox_jsk_base,ox_ksk_base,ox_lsk_base, &
                ox_msk_base,ox_nsk_base,ox_usk_base,ox_ysk_base,0,0)
    call nxtcrd()
    return
end subroutine inital
!_______________________________________________________________________
!__________________________________________
! Convert raw table data into character mappings and built-in rule definitions.
subroutine raw
    implicit none
    include "mortran3_indices.f95"
    integer :: o(ox_capacity)
    common /ibm/o
    integer :: o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
    equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
        (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
    integer :: p(100)
    equivalence(o(ox_pjt_base),p(1))
    integer :: k,j
    save     k,j
    ! Derive the raw-table storage limits from the configured alphabet size.
    o(ox_nmx)=o_v
    o(ox_sign)=o_v
    o(ox_rmx)=ox_r0_base+o(ox_nmx)
    o(ox_tmx)=ox_t0_base+o(ox_nmx)
    o(ox_sigt)=ox_t0_base+o(ox_sign)
    o(ox_sigr)=ox_r0_base+o(ox_sign)
    ! Read the raw character maps and token tables into their buffer regions.
    call rw(0,o(ox_unit),ox_rbf_base,o(ox_rmx))
    call rw(0,o(ox_unit),ox_oset_base,ox_oset_base+73)
    call rw(0,o(ox_unit),ox_rbf_base,o(ox_rmx))
    call rw(0,o(ox_unit),ox_oset_base+74,ox_morf_base+70)
    do k=ox_oset_base,ox_oset_base+99
        o(k-100)=o(k)
    end do
!MAKE INPUT AND OUTPUT SAME
! .(MNOSET+54)=.(MNOSET+41);     OUT-TRANSLATE L-ARROW TO =
    ! Preserve the raw table text in both listing and translated-output files.
    call rw(23,o(ox_mlst),ox_rbf_base+41,ox_r8_base)
    call rw(23,o(ox_flst),ox_rbf_base+41,ox_r8_base)
    do k = 1,100
        j=o(ox_iset_base+k-1)
        o(ox_trt_base+j)=k-1
!     WRITE(6,1) J,K-1
    end do
    ! Set default table values and initialize the punctuation token codes.
    o(ox_dflt)=4
    o(ox_free)=ox_mbf_base
    do k = 1,10
        o(k+100)=k+100
        o(k+110)=k+110
        o(100-k)=-k
    end do
!     BLANK BAT BLB TRM LPR RPR LSB RSB LCB RCB
    call dumdum(76, 90, 79, 78, 75, 38, 46, 74, 82, 83, 84)
    o(ox_bq)=77
    o(ox_dq)=80
    ! Install the bootstrap rule that lets the ordinary definition machinery
    ! parse subsequent built-in rules from the raw table.
    ! Set up the captured-field ranges for a built-in definition: key, empty
    ! type field, and replacement. The token sequence below fills those ranges.
    call dumdum(29001,29018,29016,29016,29016,29016,29009,0,3,0,0)
    ! Encode the bootstrap rule (GP) GP (GP) -> GA O. Here 38='(', 108=GP,
    ! 46=')', 107=GA, and 24='O'; the final zero terminates the replacement.
    call dumdum(29009,38,108,46,108,38,108,46,107,24,0)
    ! Register syntax for (pattern) type (replacement); the GA O action routes
    ! each matched definition through kat/define.
    call define(29009)
    ! Initialize lexp's operator table (p aliases o(ox_pjt_base)). Each value
    ! is 100*precedence + (reduction case + 1); lexp computes the case as
    ! mod(p(x),100)-1. Higher hundreds bind more tightly.
    p(46)=101   ! ')': precedence 1; closing-delimiter marker, not reduced itself.
    p(38)=202   ! '(': precedence 2; reduction case 1 closes the group.
    p(87)=303   ! '|': precedence 3; reduction case 2, logical OR.
    p(88)=404   ! '&': precedence 4; reduction case 3, logical AND.
    p(89)=505   ! '~': precedence 5; reduction case 4, logical NOT.
    p(42)=706   ! '-': precedence 7; reduction case 5, subtraction.
    p(43)=707   ! '+': precedence 7; reduction case 6, addition.
    p(44)=808   ! '/': precedence 8; reduction case 7, division.
    p(45)=809   ! '*': precedence 8; reduction case 8, multiplication.
    p( 36)=910  ! '**': precedence 9; reduction case 9, exponentiation.
    p( 32)=303  ! OR word/operator index: same precedence and action as '|'.
    p( 12)=404  ! AND word/operator index: same precedence and action as '&'.
    p( 29)=505  ! NOT word/operator index: same precedence and action as '~'.
    p( 11)=611  ! '>=': precedence 6; reduction case 10.
    p( 14)=612  ! '<=': precedence 6; reduction case 11.
    p( 16)=613  ! '/=': precedence 6; reduction case 12.
    p( 18)=614  ! '==': precedence 6; reduction case 13.
    p( 23)=615  ! '>': precedence 6; reduction case 14.
    p( 30)=616  ! '<': precedence 6; reduction case 15.
    ! Potential legacy mismatch: lexp maps token 88 ('&') to index 13, but
    ! this table sets p(88), not p(13); check that path before relying on '&'.
    ! The bootstrap rule used type 4; subsequent definitions default to family 6.
    o(ox_dflt)=6
    return
end subroutine raw

! Parse command-line options and open Mortran input and output files.
subroutine open_units
    use, intrinsic :: iso_fortran_env, only: int32

    implicit none
    include "mortran3_indices.f95"
    integer :: o(ox_capacity)
    common /ibm/o
    integer :: o_b,o_e,o_r,o_s,o_v,o_u,o_w,o_x
    equivalence (o(ox_b),o_b),(o(ox_e),o_e),(o(ox_r),o_r),(o(ox_s),o_s), &
        (o(ox_v),o_v),(o(ox_u),o_u),(o(ox_w),o_w),(o(ox_x),o_x)
    common/nrcc/ oflag
    integer :: oflag
    common/junk/verbose
    logical :: verbose
    character(len=4096) :: arg,next_file
    logical :: ignore_missing,is_there

    integer :: iargc,narg,nd,nfs,nfe,nc,no7,no8,i,j,open_status
    integer(int32) :: lnblnk1

    narg = iargc()
    verbose = .true.
    ignore_missing = .false.
    if(narg==0) then

        ! Preserve the legacy fort.* convention for script-driven invocations.
        ! no arguments -> assume program is called from a script
        ! making all necessary links and hope for the best

        open(unit=1,file='fort.1',status='OLD',iostat=open_status)
        if (open_status/=0) then
            call getarg(0,arg)
            write(6,'(a,a)') arg(1:lnblnk1(arg)),' called without arguments'
            write(6,'(a)') '  but file fort.1 does not exist! '
            write(6,'(a)') '  quiting now.'
            call exit(1) !stop 1
        end if
        open(unit=5,file='fort.5',status='OLD',iostat=open_status)
        open(unit=6,file='fort.6',status='UNKNOWN')
        open(unit=7,file='fort.7',status='UNKNOWN')
        open(unit=2,file='fort.2',status='OLD',iostat=open_status)
        open(unit=3,file='fort.3',status='OLD',iostat=open_status)
        open(unit=4,file='fort.4',status='OLD',iostat=open_status)
        open(unit=8,file='fort.8',status='OLD',iostat=open_status)
        o_b=1   ! hex/raw data unit number
        o_e=6   ! listing unit number
        o_r=7   ! fortran/hex output unit number
        oflag = 6
        if( verbose ) then
            write(6,'(a)') 'mortran3.exe: using standard fort.* units'
            write(6,'(/a)',advance='no') '  Mortran compiling ... '
        end if
!        call getarg(0,arg)
!        write(6,'(a,a)') arg(1:lnblnk1(arg)),
!     &    ': using standard fort.* units'
!        write(6,'(/a)',advance='no') '  Mortran compiling ... '
        return

    end if

    call getarg(1,arg)
    if( arg(1:)=='-help' ) then
        ! Keep command-line usage documentation in the executable itself.
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
    ! Record option argument positions first; file handling happens below.
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
                if( arg(1:1)=='-' ) exit
                nfe=j
            end do
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
        ! The raw/packed table is always required when explicit options are used.
        call getarg(nd,arg)
        if( verbose ) then
            write(6,'(a,a)') '  raw/hex data file: ',arg(1:lnblnk1(arg))
        end if
        open(1,file=arg(1:lnblnk1(arg)),status='old',iostat=open_status)
        if (open_status/=0) call open_error()
    else
        write(6,'(a)') '  raw/hex data file must be specified using'
        write(6,'(a,a,a)') '   ',arg(1:lnblnk1(arg)),' -d data_file '
        call exit(2) !stop 2
    end if
    if( (nc>0).or.(nfs<=nfe) ) then
        if(nc>0) then
            ! A -c file lists source files to concatenate into one Mortran job.
            open(12,file='mortjob.mortran')
            write(12,'(a)') '%L'
            call getarg(nc,arg)
            if( verbose ) then
                write(6,'(a,a)') '  configuration file: ',arg(1:lnblnk1(arg))
            end if
            open(11,file=arg(1:lnblnk1(arg)),status='old',iostat=open_status)
            if (open_status/=0) call open_error()
            do
                read(11,'(a)',iostat=open_status) next_file
                if (open_status/=0) exit
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
            end do
            close(11)
            write(12,'(/a)') '%%'
            close(12)
            arg = 'mortjob.mortran'
            open(5,file=arg,status='old',iostat=open_status)
            if (open_status/=0) call open_error()
        else
            if( nfs==nfe ) then ! a single command line file
                ! A single -f input can be read directly without a temporary job.
                call getarg(nfs,next_file)
                if( verbose ) then
                    write(6,'(a,a)') '  command line file: ', &
                        next_file(1:lnblnk1(next_file))
                end if
                open(5,file=next_file,status='old',iostat=open_status)
                if (open_status/=0) call open_error()
            else
                ! Multiple -f inputs are concatenated in command-line order.
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
                open(5,file=arg,status='old',iostat=open_status)
                if (open_status/=0) call open_error()
            end if
        end if
    end if
    ! Open the generated Fortran and listing destinations, using legacy names
    ! unless -o7 or -o8 supplied explicit paths.
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
        open(8,file=arg(1:lnblnk1(arg)),iostat=open_status)
        if (open_status/=0) call open_error()
    else
        if( verbose )write(6,'(a)') '  mortlst output: fort.8'
        open(8,file='fort.8',status='unknown')
    endif
    o_b=1   ! hex/raw data unit number
    o_e=8   ! listing unit number
    o_r=7   ! fortran/hex output unit number
    if( verbose ) write(6,'(/a)',advance='no') '  Mortran compiling ... '
    oflag = 8
    return

contains
    subroutine open_error()
        write(6,'(a,a,a)') ' file ',arg(1:lnblnk1(arg)),' does not exist!'
        call exit(3) !stop 3
    end subroutine open_error
end subroutine open_units

! Append a source file to the combined Mortran input.
subroutine add_file(iu,the_file)
    ! iu: Output unit receiving the source file contents.
    ! the_file: Path of the source file to append.
    use, intrinsic :: iso_fortran_env, only: int32
    implicit none
    integer(int32) :: iu
    integer :: read_status,lineno,j
    integer(int32) :: lnblnk1
    character(len=*) :: the_file
    character(len=250) :: line
    open(99,file=the_file,status='old',iostat=read_status)
    if (read_status/=0) then
        write(6,'(a,a)') 'add_file: could not open file ', &
            the_file(1:lnblnk1(the_file))
        call exit(4) !stop 4
        return
    end if
    lineno = 0
    do
        read(99,'(a)',iostat=read_status) line
        if (read_status/=0) exit
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
    close(99)
    return
end subroutine add_file

! Return the position of the last non-blank character, or zero if none.
function lnblnk1(c)
    ! c: Character string to inspect; trailing blanks are ignored.
    use, intrinsic :: iso_fortran_env, only: int32
    implicit none
    integer(int32) :: lnblnk1
    character(len=*) :: c
    integer(int32) :: j
    do j=len(c),1,-1
        if( c(j:j)/= ' ' ) then
            lnblnk1 = j
            return
        end if
    end do
    lnblnk1=0
    return
end function lnblnk1
