C> @brief Application for radiative energy loss calculations
C>
C>  EGSnrc application to calculate <g>, the average fraction of kinetic
C>  energy transferred to charged particles by photons lost to radiation when
C>  electrons slow down, or the radiative yield Y for electron beams.
C>  For photons, quantities such as kerma, collision kerma, <mu_tr/rho> and
C>  <mu_en/rho> are also calculated.
C>
C>  Implemented a type=1 calculation that can run until a prescribed accuracy
C>  is reached at any stage during the calculation. In a type=1 calculation
C>  mu_tr is calculated first, then mu_en is obtained from mu_en = mu_tr*(1-g),
C>  where g is the fraction lost to radiation from slowing down electrons. The
C>  advantage is that when g is small, mu_en converges much faster to the
C>  desired accuracy compared to a type=0 calculation.
C>  @IK
C>  @date 2000
C>  @copyright National Research Council Canada
C> @param[out] g average fraction of the kinetic energy of secondary charged
C>             particles (produced in all the types of interactions) that is
C>             subsequently lost in radiative (photon-emitting) energy-loss
C>             processes as the particles slow to rest in the medium.
C> (Taken from the
C> <a href="http://physics.nist.gov/PhysRefData/XrayMassCoef/chap3.html">
C> NIST web page</a>)
C> @param e_brem    Average energy lost to bremsstrahlung
C> @param ebrem_tmp As above for the current history
C> @param e_rad  Average energy lost to bremsstrahlung & kinetic energy
C>               transferred in annihilation radiation (ie only annihilation
C>               in flight. Note this used to include fluorescent photons, but
C>               they should be excluded from e_tot as well.
C> @param erad_tmp As above for the current history
C> @param e_tot  average energy released per particle
C>               includes subthreshold energy (iarg=4)
C> @param etot_tmp  As above for the current history
C> @param e_radc  = sum (etot_tmp*erad_tmp)
C> @param e_bremc = sum (etot_tmp*ebrem_tmp)
C> @param[out] e_brem/e_tot the average fraction of the kinetic energy
C>             subsequently lost in bremsstrahlung (photon-emitting) events
C> @param[out] e_rad/e_tot same as g above
C> @param anorm
C> @param npgi
C> @param npei
C> @param E_ave   Average spectrum energy
C> @param factor  Converts e_mutr and e_muen scored as MeV cm^2/g to Gy cm^2
C> @param de_pulsei
C> @param e_mutr The correct definition for a spectrum is that given by Attix
C>  \f[
C>    \overline{\mu}_\mathrm{tr}/\rho =
C>     \int \psi(E) \mu_\mathrm{tr}(E)/\rho dE/\int \psi(E) dE
C>  \f]
C>  This is equivalent to the ICRU60/ICRU85 definition as long as one sums
C>  the total energy transferred from the spectrum and then divides by the
C>  average energy incident.
C>  For a while we scored <mu_tr/rho> directly, but this can differ
C>  substantially from the correct results and amounts to averaging
C>  over the fluence, not the energy fluence
C> @param e_muen
C> @param mutr
C> @param muen
C> @param Eave  average energy of incident spectrum: either actual or sampled
      program calculate_g
      implicit none
      COMMON/THRESH/RMT2,RMSQ, AP(1),AE(1),UP(1),UE(1),TE(1),THMOLL(1)
      real*8 RMT2,  RMSQ,  AP,  AE,  UP,  UE,  TE,  THMOLL
      COMMON/BOUNDS/ECUT(1),PCUT(1),VACDST
      real*8 ECUT,  PCUT,  VACDST
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      common/score/ e_tot,e_tot2,e_brem,e_brem2,e_bremc,e_rad,e_rad2,e_r
     *adc, etot_tmp,ebrem_tmp,erad_tmp,my_gle,accu,ncase,calc_type, duri
     *ng_pe_compt,during_eii,verbose
      real*8 e_tot,e_tot2,e_brem,e_brem2,e_rad,e_rad2, etot_tmp,ebrem_tm
     *p,erad_tmp,e_radc,e_bremc
      integer*8 ncase
      real*8 my_gle,accu
      integer*4 calc_type,during_pe_compt,during_eii
      logical verbose
      COMMON/PHOTIN/ EBINDA(1), GE0(1),GE1(1), GMFP0(2000,1),GMFP1(2000,
     *1),GBR10(2000,1),GBR11(2000,1),GBR20(2000,1),GBR21(2000,1), RCO0(1
     *),RCO1(1), RSCT0(100,1),RSCT1(100,1), COHE0(2000,1),COHE1(2000,1),
     *  PHOTONUC0(2000,1),PHOTONUC1(2000,1), DPMFP, MPGEM(1,1), NGR(1)
      real*8 EBINDA,  GE0,GE1,  GMFP0,GMFP1,  GBR10,GBR11,  GBR20,GBR21,
     *  RCO0,RCO1,  RSCT0,RSCT1,  COHE0,COHE1,   PHOTONUC0,PHOTONUC1,  D
     *PMFP
      integer*4
     *                  MPGEM,
     *                          NGR
      COMMON/STACK/ E(50),X(50),Y(50),Z(50),U(50),V(50),W(50),DNEAR(50),
     *WT(50),IQ(50),IR(50),LATCH(50), LATCHI,NP,NPold
      DOUBLE PRECISION E
      real*8 X,Y,Z,  U,V,W,  DNEAR,  WT
      integer*4 IQ,  IR,  LATCH,  LATCHI, NP,  NPold
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      COMMON/EPCONT/EDEP,EDEP_LOCAL,TSTEP,TUSTEP,USTEP,TVSTEP,VSTEP, RHO
     *F,EOLD,ENEW,EKE,ELKE,GLE,E_RANGE, x_final,y_final,z_final, u_final
     *,v_final,w_final, IDISC,IROLD,IRNEW,IAUSFL(35)
      DOUBLE PRECISION EDEP,  EDEP_LOCAL
      real*8 TSTEP,  TUSTEP,  USTEP,  VSTEP,  TVSTEP,  RHOF,  EOLD,  ENE
     *W,  EKE,  ELKE,  GLE,  E_RANGE, x_final,y_final,z_final,  u_final,
     *v_final,w_final
      integer*4 IDISC,  IROLD,  IRNEW,  IAUSFL
      common/energies/ eis,neis
      real*8 eis(2000)
      integer*4 neis
      character*80 datfile(50)
      real*8 aux,aux2,total,anorm,sum,sum2,xtest,xtest2
C> The correct definition of mu_tr/rho for a spectrum is that given by
C> Attix = int[psi mu_tr/rho dE]/int[psi dE]  This is equivalent to the
C> ICRU60 defn as long as one sums the total energy transferred from
C> the spectrum and then divides by the average energy incident
C> For a while we scored <mu_tr/rho> directly, but this can differ
C> substantially from the correct results and amounts to averaging
C> over the fluence, not the energy fluence
      real*8 e_mutr, e_mutr2, e_muen, e_muen2, mutr, mutr2, muen, muen2
      real*8 ert, ert2, ett, ett2, g, g_rel_err
      real*8 weight,weight2,cov,covt,rel_e
      integer*8 nperbatch,ncasei,icase,nmutr,nmuen
      real*8 de_pulsei, Eave, err_Eave,factor,gmfp
      real*8 t_mutr, t_muen
      real*8 ein,uin,vin,win,wtin,xin,yin,zin,ecut_ask,pcut_ask
      real*8 gbr1,gbr2,rnno,err_frac
      integer*4 iqin,irin,ip
      integer*4 itimes, ntimes
      real*4 cpu,etime,time_array(2)
      integer*4 datcount
      integer*4 npgi,npei,nspliti,nbini,lmy_gle
      integer*4 nbatch,  ibatch,  i,j
      call egs_init
      call inputs
      ecut_ask = ecut(1)
      pcut_ask = pcut(1)
      WRITE(6,1010)
1010  FORMAT(' CALL HATCH to get cross-section data'/)
      CALL HATCH
      write(*,'(//)')
      write(*,*) '******************************************'
      WRITE(6,1020)
1020  FORMAT('* Start g value calculation: Version 1.5 *')
      write(*,*) '******************************************'
      WRITE(6,1030)(media(j,1),j=1,24)
1030  FORMAT(/'          MEDIUM is: ',24A1/)
      WRITE(6,1040)AE(1)-PRM, AP(1)
1040  FORMAT(' knock-on electrons can be created and any electron follow
     *ed down to' /T40,F8.3,' MeV kinetic energy'/ '   brem photons canb
     *e created and any photon followed down to      ', /T40,F8.3,' MeV 
     *')
      WRITE(6,1050)UE(1)-rm, UP(1)
1050  FORMAT(' electron and photon upper kinetic energies are:',F8.3,F11
     *.3, ' MeV respectively')
      IF ((ecut_ask .LT. ae(1) .OR. pcut_ask .LT. ap(1) )) THEN
        WRITE(6,1060)ecut_ask, AE(1), pcut_ask, AP(1)
1060    FORMAT(//'******************************************************
     *********'/ 'There is a mismatch between asked for and available cu
     *t-offs  '/ 'Asked for ECUT of', F10.4,' MeV and have AE of',F10.4,
     *' MeV'/ 'Asked for PCUT of', F10.4,' MeV and have AP of',F10.4,' M
     *eV'/ 'Exiting until there is a match'/ '**************************
     *************************************'/)
        STOP
      END IF
      cpu = etime(time_array)
      write(6,'(a,f9.2)') ' CPU time so far: ',cpu
      WRITE(6,1070)
1070  FORMAT(/' Starting shower simulation ...')
      nbatch = 10
      nperbatch = ncase/nbatch
      IF((nperbatch .EQ. 0))nperbatch = 1
      call source_sumry(6)
      IF (( accu .GT. 0 .AND. calc_type .EQ. 1)) THEN
        write(*,*) '---------------------'
        write(6,'(a,a,f8.4,a)') ' Calculation type 1: ', 'Any calculatio
     *n loop interrupted when accuracy better than ', accu*100.,'%'
        write(*,*) '---------------------'
      END IF
      write(*,*)
      write(6,*) 'Number of histories to simulate = ',ncase
      factor = 160.2176462
      IF ((neis.GT.1)) THEN
        ntimes = neis
        write(6,'(//A)') '====================================='
        write(6,'(A,I4,A)') 'Will loop through ',ntimes,' energies'
        write(6,'(A//)') '====================================='
      ELSE
        ntimes = 1
      END IF
      DO 1081 itimes=1,ntimes
        e_tot=0
        e_tot2=0
        e_rad=0
        e_rad2=0
        e_brem=0
        e_brem2=0
        e_bremc=0
        e_radc=0
        during_pe_compt=0
        during_eii=0
        e_mutr=0.0
        e_mutr2=0.0
        e_muen=0.0
        e_muen2=0.0
        mutr=0.0
        mutr2=0.0
        muen=0.0
        muen2=0.0
        nmutr=0
        nmuen=0
        call source_get_eave(Eave)
        call source_switch_energy(itimes)
        IF (( calc_type .EQ. 1 )) THEN
          write(6,*)
          write(6,*) '=> First calculating mutr only'
          write(6,*)
          x(1)=0
          y(1)=0
          z(1)=0
          wt(1)=1
          ir(1)=1
          medium=1
          ibatch=1
          t_mutr = etime(time_array)
          DO 1091 icase=1,ncase
1100        call source_sample(iqin,irin,ein,xin,yin,zin,uin,vin,win,wti
     *      n)
            IF (( iqin .NE. 0 )) THEN
              write(6,*) 'type=1 calculation only works for photons!'
              call exit(1)
            END IF
            IF (( ein .LT. ap(1))) THEN
              WRITE(6,1110)ein, pcut_ask
1110          FORMAT(/' -> Photon energy of ' , f10.5,' MeV '/ '    belo
     *w cut-off AP = ',f10.5,' MeV.'/ '    Discard and resample source.'
     *)
              goto 1100
            END IF
            my_gle = log(ein)
            gle = my_gle
            Lmy_gle=ge1(MEDIUM)*my_gle+ge0(MEDIUM)
            gmfp=gmfp1(Lmy_gle,MEDIUM)*my_gle+gmfp0(Lmy_gle,MEDIUM)
            gbr1=gbr11(Lmy_gle,MEDIUM)*my_gle+gbr10(Lmy_gle,MEDIUM)
            gbr2=gbr21(Lmy_gle,MEDIUM)*my_gle+gbr20(Lmy_gle,MEDIUM)
            np = 1
            e(np) = ein
            u(np)=0
            v(np)=0
            w(np)=1
            iq(np) = 0
            IF((rng_seed .GT. 128))call ranmar_get
            rnno = rng_array(rng_seed)*twom24
            rng_seed = rng_seed + 1
            edep = 0
            IF (( rnno .LT. gbr1 .AND. ein .GT. rmt2 )) THEN
              call pair
            ELSE IF(( rnno .LT. gbr2 )) THEN
              call compt
            ELSE
              call photo
            END IF
            DO 1121 ip=1,np
              IF((iq(ip) .NE. 0))edep = edep + e(ip)-prm
1121        CONTINUE
1122        CONTINUE
            nmutr = nmutr + 1
            IF ((gmfp .GT. 0)) THEN
              aux = edep/gmfp/rho(1)
            ELSE
              aux = 0
            END IF
            e_mutr = e_mutr + aux
            e_mutr2 = e_mutr2 + aux*aux
            IF (( mod(icase,10000) .EQ. 0 .AND. accu .GT. 0 )) THEN
              xtest = e_mutr
              xtest2 = e_mutr2
              xtest = xtest/nmutr
              xtest2 = xtest2/nmutr
              xtest2 = xtest2 - xtest*xtest
              IF (( xtest2 .GT. 0 )) THEN
                xtest2 = sqrt(xtest2/(nmutr-1))
              END IF
              IF((xtest2/xtest .LT. accu))GO TO1092
              IF (( xtest2/xtest/(nbatch-ibatch+1) .LT. accu)) THEN
                write(6,'(a,i2,a,i2,a,f8.2,a,f11.8,a,f8.4,a)') '+ Finish
     *ed ',ibatch,'th portion of ',nbatch, ', cpu time = ',etime(time_ar
     *          ray)-cpu, ' sec. <mu_tr/rho> = ', xtest/Eave,' cm^2/g ['
     *          , 100*xtest2/xtest,'%]'
                call flush(6)
                ibatch = ibatch + 1
              END IF
            END IF
1091      CONTINUE
1092      CONTINUE
          write(6,'(a,i2,a,i2,a,f8.2,a,f11.8,a,f8.4,a)') '+ Finished ',i
     *    batch,'th portion of ',nbatch, ', cpu time = ',etime(time_arra
     *    y)-cpu, ' sec. <mu_tr/rho> = ', xtest/Eave,' cm^2/g [', 100*xt
     *    est2/xtest,'%]'
          call flush(6)
          xtest = e_mutr
          xtest2 = e_mutr2
          xtest = xtest/nmutr
          xtest2 = xtest2/nmutr
          xtest2 = xtest2 - xtest*xtest
          IF (( xtest2 .GT. 0 )) THEN
            xtest2 = sqrt(xtest2/(nmutr-1))
          END IF
          IF (( accu .GT. 0 .AND. xtest2/xtest .LT. accu )) THEN
            write(6,*)
            write(6,*) 'mu_tr converged after ',nmutr,' histories'
            write(6,*)
          END IF
          write(6,1130) ' <E*mu_tr/rho>     = ',factor*xtest, ' 10^-12 G
     *y cm^2  +/- ', 100*xtest2/xtest,'%'
          write(6,*)
          write(6,*)
          write(6,*) '=> Now calculating g and muen'
          write(6,*)
          call flush(6)
          ibatch = 1
          t_muen = etime(time_array)
          DO 1141 icase=1,ncase
1150        call source_sample(iqin,irin,ein,xin,yin,zin,uin,vin,win,wti
     *      n)
            IF (( iqin .NE. 0 )) THEN
              write(6,*) 'type=1 calculation only works for photons!'
              call exit(1)
            END IF
            IF (( ein .LT. ap(1))) THEN
              WRITE(6,1160)ein, pcut_ask
1160          FORMAT(/' -> Photon energy of ' , f10.5,' MeV '/ '    belo
     *w cut-off AP = ',f10.5,' MeV.'/ '    Discard and resample source.'
     *)
              goto 1150
            END IF
            ebrem_tmp=0.0
            erad_tmp=0.0
            etot_tmp=0.0
            call shower(iqin,ein,xin,yin,zin,uin,vin,win,irin,wtin)
            my_gle = log(ein)
            gle = my_gle
            Lmy_gle=ge1(MEDIUM)*my_gle+ge0(MEDIUM)
            gmfp=gmfp1(Lmy_gle,MEDIUM)*my_gle+gmfp0(Lmy_gle,MEDIUM)
            weight = 0.0
            weight2 = 0.0
            aux = 0.0
            IF ((gmfp .GT. 0)) THEN
              weight = 1./gmfp/rho(1)
              weight2 = weight*weight
              aux = etot_tmp*weight
            END IF
            e_mutr = e_mutr + aux
            e_mutr2 = e_mutr2 + aux*aux
            IF ((gmfp .GT. 0)) THEN
              aux = (etot_tmp - erad_tmp)*weight
            ELSE
              aux = 0
            END IF
            e_muen = e_muen + aux
            e_muen2 = e_muen2 + aux*aux
            nmutr = nmutr+1
            nmuen = nmuen + 1
            e_tot = e_tot + etot_tmp*weight
            e_tot2 = e_tot2 + etot_tmp*etot_tmp*weight2
            e_rad = e_rad + erad_tmp*weight
            e_rad2 = e_rad2 + erad_tmp*erad_tmp*weight2
            e_radc = e_radc + etot_tmp*erad_tmp*weight2
            IF (( mod(icase,10000) .EQ. 0 .AND. accu .GT. 0 )) THEN
              IF (( e_rad .GT. 0 )) THEN
                ert = e_rad
                ert2 = e_rad2
                ert = ert/nmuen
                ert2 = ert2/nmuen
                ert2 = ert2 - ert*ert
                IF (( ert2 .GT. 0 )) THEN
                  ert2 = sqrt(ert2/(nmuen-1))
                END IF
                ett = e_tot
                ett2 = e_tot2
                ett = ett/nmuen
                ett2 = ett2/nmuen
                ett2 = ett2 - ett*ett
                IF (( ett2 .GT. 0 )) THEN
                  ett2 = sqrt(ett2/(nmuen-1))
                END IF
                cov = (e_radc/nmuen-ert*ett)/nmuen
                covt= 2*(e_radc/nmuen/ert/ett-1)/nmuen
                rel_e = ert*sqrt((ert2/ert)**2+(ett2/ett)**2-covt)/(ett-
     *          ert)
                IF((rel_e .LT. accu/2))GO TO1142
              ELSE
                IF (( icase .GT. 100000 )) THEN
                  rel_e = 0
                  GO TO1142
                END IF
              END IF
              IF (( rel_e/(nbatch-ibatch+1) .LT. accu/2)) THEN
                IF ((verbose .AND. ert2 .GT. 0 .AND. ett2 .GT. 0)) THEN
                  write(6,'(a,i2,a,i2,a,f8.2,a,f11.8,a,f8.4,a,a,f8.4)')
     *            '+ Finished ',ibatch,'th portion out of ',nbatch, ', c
     *pu time = ',etime(time_array)-cpu,' sec. 1-g = ', 1-e_rad/e_tot,'
     *[',100*rel_e,'%]', ' Corr = Cov/s_rad/s_tot =',cov/ert2/ett2
                ELSE
                  write(6,'(a,i2,a,i2,a,f8.2,a,f11.8,a,f8.4,a)') '+ Fini
     *shed ',ibatch,'th portion out of ',nbatch, ', cpu time = ',etime(t
     *            ime_array)-cpu,' sec. 1-g = ', 1-e_rad/e_tot,' [',100*
     *            rel_e,'%]'
                END IF
                call flush(6)
                ibatch = ibatch + 1
              END IF
            END IF
1141      CONTINUE
1142      CONTINUE
          IF ((verbose .AND. ert2 .GT. 0 .AND. ett2 .GT. 0)) THEN
            write(6,'(a,i2,a,i2,a,f8.2,a,f11.8,a,f8.4,a,a,f8.4)') '+ Fin
     *ished ',ibatch,'th portion out of ',nbatch, ', cpu time = ',etime(
     *      time_array)-cpu,' sec. 1-g = ', 1-e_rad/e_tot,' [',100*rel_e
     *      ,'%]', ' Corr = Cov/s_rad/s_tot =',cov/ert2/ett2
          ELSE
            write(6,'(a,i2,a,i2,a,f8.2,a,f11.8,a,f8.4,a)') '+ Finished '
     *      ,ibatch,'th portion out of ',nbatch, ', cpu time = ',etime(t
     *      ime_array)-cpu,' sec. 1-g = ', 1-e_rad/e_tot,' [',100*rel_e,
     *      '%]'
          END IF
          call flush(6)
          t_mutr = etime(time_array)-t_mutr
          t_muen = etime(time_array)-t_muen
          IF (( accu .GT. 0 .AND. ert2 .LT. accu )) THEN
            write(6,*)
            write(6,*) '1-g converged after ',nmuen,' histories'
            write(6,*)
            write(6,*)
          END IF
          e_mutr = e_mutr/nmutr
          e_mutr2 = e_mutr2/nmutr
          e_mutr2 = e_mutr2 - e_mutr*e_mutr
          IF (( e_mutr2 .GT. 0 )) THEN
            e_mutr2 = sqrt(e_mutr2/(nmutr-1))
          END IF
          e_muen = e_muen/nmuen
          e_muen2 = e_muen2/nmuen
          e_muen2 = e_muen2 - e_muen*e_muen
          IF (( e_muen2 .GT. 0 )) THEN
            e_muen2 = sqrt(e_muen2/(nmuen-1))
          END IF
          e_tot = e_tot/nmuen
          e_tot2 = e_tot2/nmuen
          e_tot2 = e_tot2 - e_tot*e_tot
          IF (( e_tot2 .GT. 0 )) THEN
            e_tot2 = sqrt(e_tot2/(nmuen-1))
          END IF
          e_rad = e_rad/nmuen
          e_rad2 = e_rad2/nmuen
          e_rad2 = e_rad2 - e_rad*e_rad
          IF (( e_rad2 .GT. 0 )) THEN
            e_rad2 = sqrt(e_rad2/(nmuen-1))
          END IF
          IF((e_rad .LT. 1e-15*e_tot))e_rad = 1e-15*e_tot
          write(6,'(a)') '-----------------------------'
          write(6,'(a)') 'Final results (calc. type 1):'
          write(6,'(a/)') '-----------------------------'
          call source_get_eave(Eave)
          write(6,'(a,f10.5,a)') ' Average spectrum energy: ',Eave,' MeV
     *'
          call source_get_samplede(Eave,err_Eave)
          err_Eave = err_Eave/Eave
          write(6,'(a,f10.5,a,f8.4,a/)') ' Average sampled energy : ', E
     *    ave,' MeV [',100*err_Eave,'%]'
          write(6,1130) ' K/phi    = <E*mu_tr/rho>         = ', factor*e
     *    _mutr, ' 10^-12 Gy cm^2 [',100*e_mutr2/e_mutr,'%]'
          covt = 2*(e_radc/nmuen/e_rad/e_tot-1)/nmuen
          g_rel_err = sqrt((e_rad2/e_rad)**2+(e_tot2/e_tot)**2-covt)
          g = e_rad/e_tot
          e_rad2=e_rad*g_rel_err/(e_tot-e_rad)
          write(6,1130) ' Kcol/phi = <E*mu_tr/rho>*(1-<g>) = ', factor*e
     *    _mutr*(1-e_rad/e_tot), ' 10^-12 Gy cm^2 [', 100*sqrt((e_mutr2/
     *    e_mutr)**2+e_rad2**2),'%]'
          write(6,*)
          write(6,'(a,1PE12.4,a,0PF7.4,a)') ' <g>
     *       = ', g,' [',100*g_rel_err,'%]'
          write(6,'(a,F12.6,a,0PF7.4,a)') ' 1-<g>
     *     = ', 1-e_rad/e_tot,' [',100*e_rad2,'%]'
          write(6,*)
          write(6,1130) '<mu_tr> = <E*mu_tr/rho>/Eave         = ', e_mut
     *    r/Eave,'   cm^2/g [', 100*sqrt((e_mutr2/e_mutr)**2+err_Eave**2
     *    ),'%]'
          write(6,1130) '<mu_en> = <E*mu_tr/rho>*(1-<g>)/Eave = ', e_mut
     *    r*(1-e_rad/e_tot)/Eave,'   cm^2/g [', 100*sqrt((e_mutr2/e_mutr
     *    )**2+e_rad2**2+err_Eave**2),'%]'
          write(6,'(a,1PE12.6,a)') '<E*mu_en/rho>
     * = ', e_mutr*(1-e_rad/e_tot),'   MeV cm^2/g'
          IF ((verbose)) THEN
            write(*,*)
            write(*,*) '--------------------------'
            write(*,*) 'Calculation type 0 results:'
            write(*,*) '--------------------------'
            write(*,*)
            write(6,1130) ' Kcol/phi   = <E*mu_en/rho>      = ', factor*
     *      e_muen,' 10^-12 Gy cm^2       [', 100*e_muen2/e_muen,'%]'
            write(6,1130) '<mu_en>     = <E*mu_en/rho>/Eave = ', e_muen/
     *      Eave,'   cm^2/g [', 100*sqrt((e_muen2/e_muen)**2+err_Eave**2
     *      ),'%]'
            write(*,'(/a,f12.1,a/)') '-> Efficiency gain over calculatio
     *n type 0: ', ((e_muen2/e_muen)**2+err_Eave**2)*t_muen/ ((e_mutr2/e
     *      _mutr)**2+e_rad2**2+err_Eave**2)/t_mutr, ' times'
            write(*,*) '--------------------------'
          END IF
          goto 1170
1130      FORMAT(a,1PE12.6,a,0PF7.4,a)
        END IF
        call flush(6)
        write(6,*)
        write(6,*) '-------------------------------------------------'
        write(6,*) '=> Fixed number of histories (calculation type 0)'
        write(6,*) '-------------------------------------------------'
        write(6,*)
        weight = 1.0
        weight2 = 1.0
        DO 1181 icase=1,ncase
          call source_sample(iqin,irin,ein,xin,yin,zin,uin,vin,win,wtin)
          IF (( .false. )) THEN
            write(18,*) ' ******* new shower, e = ',ein,' iq = ',iqin
          END IF
          ebrem_tmp = 0.0
          erad_tmp = 0.0
          IF (( iqin .ne. 0 )) THEN
            etot_tmp = (ein - rm)
          ELSE
            etot_tmp = 0
          END IF
          call shower(iqin,ein,xin,yin,zin,uin,vin,win,irin,wtin)
          IF (( .false. )) THEN
            write(18,*) ' energy released: ',etot_tmp,' lost to brems: '
     *      ,ebrem_tmp, ' erad: ',erad_tmp
          END IF
          IF (( iqin .EQ. 0 )) THEN
            my_gle = log(ein)
            Lmy_gle=ge1(MEDIUM)*my_gle+ge0(MEDIUM)
            gmfp=gmfp1(Lmy_gle,MEDIUM)*my_gle+gmfp0(Lmy_gle,MEDIUM)
            weight = 0.0
            weight2 = 0.0
            aux = 0.0
            IF ((gmfp .GT. 0)) THEN
              weight = 1./gmfp/rho(1)
              weight2 = weight*weight
              aux = etot_tmp*weight
            END IF
            e_mutr = e_mutr + aux
            e_mutr2 = e_mutr2 + aux*aux
            IF ((gmfp .GT. 0)) THEN
              aux = (etot_tmp - erad_tmp)*weight
            ELSE
              aux = 0
            END IF
            e_muen = e_muen + aux
            e_muen2 = e_muen2 + aux*aux
          END IF
          e_tot = e_tot + etot_tmp*weight
          e_tot2 = e_tot2 + etot_tmp*etot_tmp*weight2
          e_brem = e_brem + ebrem_tmp*weight
          e_brem2 = e_brem2 + ebrem_tmp*ebrem_tmp*weight2
          e_rad = e_rad + erad_tmp*weight
          e_rad2 = e_rad2 + erad_tmp*erad_tmp*weight2
          e_radc = e_radc + etot_tmp*erad_tmp*weight2
          e_bremc = e_bremc + etot_tmp*ebrem_tmp*weight2
          ibatch = icase/nperbatch
          IF (( ibatch*nperbatch .EQ. icase )) THEN
            write(6,'(a,i2,a,i2,a,f9.2,a)') '+ Finished part ',ibatch,'
     *out of ',nbatch, ', cpu time = ',etime(time_array)-cpu,' sec.'
            call flush(6)
          END IF
1181    CONTINUE
1182    CONTINUE
        WRITE(6,1190)
1190    FORMAT(/' Finished shower simulation ')
        write(*,*)
        write(6,'(a)') '-----------------------------'
        write(6,'(a)') 'Final results (calc. type 0):'
        write(6,'(a/)') '-----------------------------'
        call source_get_eave(Eave)
        write(6,*)
        write(6,'('' Average spectrum energy: '',1PE10.4, '' MeV'')') Ea
     *  ve
        call source_get_samplede(Eave,err_Eave)
        err_Eave = max(0.0, err_Eave)
        write(6,'('' Average sampled energy: '',1PE10.4, '' +/-'',1PE10.
     *2,                    ''  MeV'')') Eave,err_Eave
        e_tot = e_tot/ncase
        e_tot2 = e_tot2/ncase
        e_tot2 = e_tot2 - e_tot*e_tot
        IF (( e_tot2 .GT. 0 )) THEN
          e_tot2 = sqrt(e_tot2/(ncase-1))
        END IF
        e_brem = e_brem/ncase
        e_brem2 = e_brem2/ncase
        e_brem2 = e_brem2 - e_brem*e_brem
        IF (( e_brem2 .GT. 0 )) THEN
          e_brem2 = sqrt(e_brem2/(ncase-1))
        END IF
        e_rad = e_rad/ncase
        e_rad2 = e_rad2/ncase
        e_rad2 = e_rad2 - e_rad*e_rad
        IF (( e_rad2 .GT. 0 )) THEN
          e_rad2 = sqrt(e_rad2/(ncase-1))
        END IF
        factor = 160.2176462
        IF (( e_mutr .GT. 0 )) THEN
          write(6,'(/)')
          e_mutr = e_mutr/ncase
          e_mutr2 = e_mutr2/ncase
          e_mutr2 = e_mutr2 - e_mutr*e_mutr
          IF (( e_mutr2 .GT. 0 )) THEN
            e_mutr2 = sqrt(e_mutr2/(ncase-1))
          END IF
          write(6,800)factor*e_mutr, 100*e_mutr2/e_mutr
800       format(' <E*mu_tr/rho> i.e., K/phi =',1PE12.4,'    10^-12 Gy c
     *m^2 +/-', 0PF7.3,' %')
          write(6,801)e_mutr/Eave
801       format(' <E*mu_tr/rho>/Eave        =',1PE12.4,'    cm^2/g')
          write(6,*) 'The above is the spectrum averaged coefficient'
        END IF
        IF (( e_muen .GT. 0 )) THEN
          e_muen = e_muen/ncase
          e_muen2 = e_muen2/ncase
          e_muen2 = e_muen2 - e_muen*e_muen
          IF (( e_muen2 .GT. 0 )) THEN
            e_muen2 = sqrt(e_muen2/(ncase-1))
          END IF
          write(6,*)
          write(6,'('' <E*mu_en/rho> i.e., Kcol/phi ='',
     *                     1PE12.4,''    10^-12 Gy cm^2 +/-'',
     *                                   0PF7.3,'' %'')') factor*e_muen,
     *     100*e_muen2/e_muen
          write(6,'('' <E*mu_en/rho>/Eave           ='',1PE12.4,''    cm
     *^2/g'')') e_muen/Eave
          write(6,*) 'The above is the spectrum averaged coefficient'
          write(6,'(/)')
        END IF
        WRITE(6,1200)e_tot,e_tot2, 100*e_tot2/e_tot
1200    FORMAT(' Ave energy released per particle:  ',1PE12.4, ' MeV +/-
     *', 1PE10.3, ' [', 0PF7.3,' %]')
        WRITE(6,1210)e_brem,e_brem2, 100*e_brem2/e_brem
1210    FORMAT(' Ave energy lost to bremsstrahlung: ',1PE12.4, ' MeV +/-
     *', 1PE10.3, ' [', 0PF7.3,' %]')
        WRITE(6,1220)e_rad,e_rad2, 100*e_rad2/e_rad
1220    FORMAT(' Ave energy lost to all radiation:  ',1PE12.4, ' MeV +/-
     *', 1PE10.3, ' [', 0PF7.3,' %]')
        err_frac = sqrt((e_brem2/e_brem)**2+(e_tot2/e_tot)**2)
        WRITE(6,1230)e_brem/e_tot, err_frac*(e_brem/e_tot), 100.*err_fra
     *  c
1230    FORMAT(/'fractions   g(brem) = ',1PE12.4,' +/-',1PE12.4,' [',0PF
     *7.3,' %]')
        err_frac = sqrt((e_rad2/e_rad)**2+(e_tot2/e_tot)**2)
        WRITE(6,1240)e_rad/e_tot,err_frac*(e_rad/e_tot),100*err_frac
1240    FORMAT('         g(all rad) = ',1PE12.4,' +/-',1PE12.4,' [',0PF7
     *.3,' %]'/)
        write(6,*) 'The above fraction error estimates are made ignoring
     * correlations'
        write(6,*) ' between energy released and energy lost to radiatio
     *n '
        e_radc = e_radc/ncase
        e_bremc = e_bremc/ncase
        IF (( e_rad .GT. 0 )) THEN
          e_radc = (e_radc/e_tot/e_rad-1)/(ncase-1)
        END IF
        IF (( e_brem .GT. 0 )) THEN
          e_bremc = (e_bremc/e_tot/e_brem-1)/(ncase-1)
        END IF
        e_radc = (e_rad2/e_rad)**2+(e_tot2/e_tot)**2-2*e_radc
        IF (( e_radc .GT. 0 )) THEN
          e_radc = sqrt(e_radc)
        END IF
        e_bremc = (e_brem2/e_brem)**2+(e_tot2/e_tot)**2-2*e_bremc
        IF (( e_bremc .GT. 0 )) THEN
          e_bremc = sqrt(e_bremc)
        END IF
        write(6,*) ' Fractional uncertainties with correlations are: '
        WRITE(6,1250)100*e_bremc
1250    FORMAT('   brems:          ',F8.4,' %')
        WRITE(6,1260)100*e_radc
1260    FORMAT('   all radiative:  ',F8.4,' %')
        call flush(6)
1170    CONTINUE
1081  CONTINUE
1082  CONTINUE
      call egs_finish
      end
C> Subroutine to read user inputs and transport parameters
      subroutine inputs
      implicit none
      COMMON/EPCONT/EDEP,EDEP_LOCAL,TSTEP,TUSTEP,USTEP,TVSTEP,VSTEP, RHO
     *F,EOLD,ENEW,EKE,ELKE,GLE,E_RANGE, x_final,y_final,z_final, u_final
     *,v_final,w_final, IDISC,IROLD,IRNEW,IAUSFL(35)
      DOUBLE PRECISION EDEP,  EDEP_LOCAL
      real*8 TSTEP,  TUSTEP,  USTEP,  VSTEP,  TVSTEP,  RHOF,  EOLD,  ENE
     *W,  EKE,  ELKE,  GLE,  E_RANGE, x_final,y_final,z_final,  u_final,
     *v_final,w_final
      integer*4 IDISC,  IROLD,  IRNEW,  IAUSFL
      COMMON/GetInput/ ALLOWED_INPUTS(100,0:5),   VALUES_SOUGHT(100),  C
     *HAR_VALUE(100,100),  VALUE(100,100),  DEFAULT(100),  VALUE_MIN(100
     *),  VALUE_MAX(100),  NVALUE(100),  TYPE(100),      ERROR_FLAGS(100
     *),   i_errors,  NMIN, NMAX,   ERROR_FLAG,  DELIMETER
      character ALLOWED_INPUTS*64,VALUES_SOUGHT*64, CHAR_VALUE*256,DELIM
     *ETER*64
      real*8 VALUE,DEFAULT,VALUE_MIN,VALUE_MAX
      integer*4 NVALUE,TYPE,NMIN,NMAX,ERROR_FLAG,ERROR_FLAGS,i_errors
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      COMMON/MISC/  DUNIT,KMPI,KMPO,RHOR(1),MED(1),IRAYLR(1),IPHOTONUCR(
     *1)
      real*8 DUNIT,  RHOR
      integer*4 KMPI,  KMPO
      integer*2 MED,  IRAYLR,  IPHOTONUCR
      COMMON/UPHIOT/THETA,SINTHE,COSTHE,SINPHI, COSPHI,PI,TWOPI,PI5D2
      real*8 THETA,  SINTHE,  COSTHE,  SINPHI,  COSPHI,  PI,TWOPI,PI5D2
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      COMMON/BREMPR/ DL1(8,1),DL2(8,1),DL3(8,1),DL4(8,1),DL5(8,1),DL6(8,
     *1), ALPHI(2,1),BPAR(2,1),DELPOS(2,1), WA(1,50),PZ(1,50),ZELEM(1,50
     *),RHOZ(1,50), PWR2I(50), DELCM(1),ZBRANG(1),LZBRANG(1),NNE(1), IBR
     *DST,IPRDST,ibr_nist,pair_nrc,itriplet, ASYM(1,50,2)
      CHARACTER*4 ASYM
      real*8 DL1,DL2,DL3,DL4,DL5,DL6,   ALPHI,  BPAR,  DELPOS,  WA,  PZ,
     *  ZELEM,  RHOZ,  PWR2I,  DELCM,  ZBRANG,  LZBRANG
      integer*4 NNE,  IBRDST,  IPRDST,  ibr_nist,  itriplet,  pair_nrc
      common/score/ e_tot,e_tot2,e_brem,e_brem2,e_bremc,e_rad,e_rad2,e_r
     *adc, etot_tmp,ebrem_tmp,erad_tmp,my_gle,accu,ncase,calc_type, duri
     *ng_pe_compt,during_eii,verbose
      real*8 e_tot,e_tot2,e_brem,e_brem2,e_rad,e_rad2, etot_tmp,ebrem_tm
     *p,erad_tmp,e_radc,e_bremc
      integer*8 ncase
      real*8 my_gle,accu
      integer*4 calc_type,during_pe_compt,during_eii
      logical verbose
      character*60 medium_name
      integer*4 i,j,ispin,jrn1,jrn2
      integer*4 ival
      real*8 emax
      integer*4 lnblnk
      call get_transport_parameter(6)
      accu = -1
      calc_type = 0
      verbose = .false.
      ival = 0
      delimeter = 'NONE'
      ival = ival + 1
      values_sought(ival) = 'MEDIA'
      type(ival) = 2
      nvalue(ival) = 0
      NMIN = IVAL
      NMAX = IVAL
      CALL GET_INPUT
      IF((error_flag .GT. 0))stop
      nmed = nvalue(ival)
      IF (( nmed .GT. 1 )) THEN
        write(6,*) ' Just ',1,' allowed!'
        stop
      END IF
      DO 1271 i=1,nmed
        DO 1281 j=1,24
          media(j,i) = ' '
1281    CONTINUE
1282    CONTINUE
        read(char_value(ival,i),'(24a1)') (media(j,i),j=1,lnblnk(char_va
     *  lue(ival,i)))
1271  CONTINUE
1272  CONTINUE
      DO 1291 i=1,1
        med(i) = 1
1291  CONTINUE
1292  CONTINUE
      dunit = 1
      ival = ival + 1
      values_sought(ival) = 'INITIAL RANDOM NO. SEEDS'
      type(ival) = 0
      nvalue(ival) = 2
      value_min(ival) = 0
      value_max(ival) = 9999999
      default(ival) = 0
      NMIN = IVAL
      NMAX = IVAL
      CALL GET_INPUT
      jrn1 = value(ival,1)
      jrn2 = value(ival,2)
      ixx = jrn1
      jxx = jrn2
      call init_ranmar
      ival = ival + 1
      values_sought(ival) = 'NUMBER OF HISTORIES'
      type(ival) = 0
      nvalue(ival) = 1
      value_min(ival) = 2
      default(ival) = 10000
      NMIN = IVAL
      NMAX = IVAL
      CALL GET_INPUT
      ncase = value(ival,1)
      ival = ival + 1
      values_sought(ival) = 'TURN OFF BREMS ANGLES'
      type(ival) = 3
      nvalue(ival) = 1
      allowed_inputs(ival,0) = 'NO'
      allowed_inputs(ival,1) = 'YES'
      NMIN = IVAL
      NMAX = IVAL
      CALL GET_INPUT
      IF (( error_flag .EQ. 0 )) THEN
        IF((value(ival,1) .GT. 0))ibrdst=-1
      ELSE
        error_flag = 0
      END IF
      ival = ival + 1
      values_sought(ival) = 'ACCURACY'
      type(ival) = 0
      nvalue(ival) = 1
      value_min(ival) = -1e30
      value_max(ival) = 0.1
      default(ival) = -1
      NMIN = IVAL
      NMAX = IVAL
      CALL GET_INPUT
      IF (( error_flag .EQ. 0 )) THEN
        accu = value(ival,1)
      ELSE
        error_flag = 0
      END IF
      ival = ival + 1
      values_sought(ival) = 'CALCULATION TYPE'
      type(ival) = 0
      nvalue(ival) = 1
      value_min(ival) = 0
      value_max(ival) = 1
      default(ival) = 0
      NMIN = IVAL
      NMAX = IVAL
      CALL GET_INPUT
      IF (( error_flag .EQ. 0 )) THEN
        calc_type = value(ival,1)
      ELSE
        error_flag = 0
      END IF
      ival = ival + 1
      values_sought(ival) = 'VERBOSE'
      nvalue(ival) = 1
      type(ival) = 3
      allowed_inputs(ival,0) = 'NO'
      allowed_inputs(ival,1) = 'YES'
      NMIN = IVAL
      NMAX = IVAL
      CALL GET_INPUT
      IF (( error_flag .EQ. 0 )) THEN
        IF (( value(ival,1) .EQ. 1 )) THEN
          verbose = .true.
        END IF
      ELSE
        error_flag = 0
      END IF
      call source
      DO 1301 j=1,33
        iausfl(j) = 0
1301  CONTINUE
1302  CONTINUE
      iausfl( 5) = 1
      iausfl( 8) = 1
      iausfl(10) = 1
      iausfl(12) = 1
      iausfl(14) = 1
      iausfl(15) = 1
      iausfl(17) = 1
      iausfl(18) = 1
      iausfl(19) = 1
      iausfl(20) = 1
      iausfl(21) = 1
      iausfl(34) = 1
      iausfl(35) = 1
      iausfl(32) = 1
      iausfl(33) = 1
      return
      end
      subroutine howfar
      COMMON/STACK/ E(50),X(50),Y(50),Z(50),U(50),V(50),W(50),DNEAR(50),
     *WT(50),IQ(50),IR(50),LATCH(50), LATCHI,NP,NPold
      DOUBLE PRECISION E
      real*8 X,Y,Z,  U,V,W,  DNEAR,  WT
      integer*4 IQ,  IR,  LATCH,  LATCHI, NP,  NPold
      IF((wt(np) .LE. 0))idisc = 1
      return
      end
      subroutine ausgab(iarg)
      implicit none
      integer*4 iarg,irl,iql,jp
      real*8 aux
      real*8 usave,vsave,wsave,wtsave,eesave,rnno,ee
      integer*4 isplit,ip
      common/score/ e_tot,e_tot2,e_brem,e_brem2,e_bremc,e_rad,e_rad2,e_r
     *adc, etot_tmp,ebrem_tmp,erad_tmp,my_gle,accu,ncase,calc_type, duri
     *ng_pe_compt,during_eii,verbose
      real*8 e_tot,e_tot2,e_brem,e_brem2,e_rad,e_rad2, etot_tmp,ebrem_tm
     *p,erad_tmp,e_radc,e_bremc
      integer*8 ncase
      real*8 my_gle,accu
      integer*4 calc_type,during_pe_compt,during_eii
      logical verbose
      COMMON/EPCONT/EDEP,EDEP_LOCAL,TSTEP,TUSTEP,USTEP,TVSTEP,VSTEP, RHO
     *F,EOLD,ENEW,EKE,ELKE,GLE,E_RANGE, x_final,y_final,z_final, u_final
     *,v_final,w_final, IDISC,IROLD,IRNEW,IAUSFL(35)
      DOUBLE PRECISION EDEP,  EDEP_LOCAL
      real*8 TSTEP,  TUSTEP,  USTEP,  VSTEP,  TVSTEP,  RHOF,  EOLD,  ENE
     *W,  EKE,  ELKE,  GLE,  E_RANGE, x_final,y_final,z_final,  u_final,
     *v_final,w_final
      integer*4 IDISC,  IROLD,  IRNEW,  IAUSFL
      COMMON/STACK/ E(50),X(50),Y(50),Z(50),U(50),V(50),W(50),DNEAR(50),
     *WT(50),IQ(50),IR(50),LATCH(50), LATCHI,NP,NPold
      DOUBLE PRECISION E
      real*8 X,Y,Z,  U,V,W,  DNEAR,  WT
      integer*4 IQ,  IR,  LATCH,  LATCHI, NP,  NPold
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      IF (( iarg .EQ. 19 .OR. iarg .EQ. 17 )) THEN
        during_pe_compt = 1
        return
      END IF
      IF (( iarg .EQ. 20 .OR. iarg .EQ. 18 )) THEN
        during_pe_compt = 0
      END IF
      IF (( iarg .EQ. 31 )) THEN
        during_eii = 1
        return
      END IF
      IF (( iarg .EQ. 32 )) THEN
        during_eii = 0
        return
      END IF
      IF (( iarg .EQ. 4 .AND. during_eii .EQ. 0 .AND. during_pe_compt .E
     *Q. 0 )) THEN
        IF (( .false. )) THEN
          write(19,*) ' Depositing iarg 4 ',edep
        END IF
        etot_tmp = etot_tmp + edep
        return
      END IF
      IF (( iarg .EQ. 4 .AND. ( during_eii .EQ. 1 .OR. during_pe_compt .
     *EQ. 1 ) )) THEN
        return
      END IF
      IF (( iarg .EQ. 34 )) THEN
        IF (( during_pe_compt .EQ. 1 )) THEN
          IF (( .false. )) THEN
            write(19,*) ' Depositing photo iarg 27 ',edep_local
          END IF
          etot_tmp = etot_tmp + edep_local
          return
        ELSE
          return
        END IF
      END IF
      IF (( iarg .EQ. 33 )) THEN
        IF (( during_eii .EQ. 1 )) THEN
          IF (( .false. )) THEN
            write(19,*) ' Depositing eii iarg 25 ',edep_local
          END IF
          erad_tmp = erad_tmp + edep_local
          return
        ELSE
          return
        END IF
      END IF
      IF (( iarg .EQ. 16 .OR. iarg .EQ. 18 .OR. iarg .EQ. 20 )) THEN
        IF (( .false. )) THEN
          write(18,*) ' iarg = ',iarg,' np = ',np
        END IF
        DO 1311 ip=NPold,NP
          IF (( .false. )) THEN
            write(18,*) '    ',ip,iq(ip),wt(ip),e(ip)
          END IF
          IF (( iq(ip) .NE. 0 )) THEN
            etot_tmp = etot_tmp + e(ip) - prm
          ELSE
            wt(ip) = 0
            e(ip) = 0
          END IF
1311    CONTINUE
1312    CONTINUE
        return
      END IF
      IF (( iarg .EQ. 7 )) THEN
        IF (( iq(np) .EQ. 0 )) THEN
          ebrem_tmp = ebrem_tmp + e(np)
          erad_tmp = erad_tmp + e(np)
          IF (( .false. )) THEN
            write(18,*) ' brems: ',e(np),wt(np)
          END IF
          wt(np) = 0
          e(np) = 0
        ELSE
          ebrem_tmp = ebrem_tmp + e(np-1)
          erad_tmp = erad_tmp + e(np-1)
          IF (( .false. )) THEN
            write(18,*) ' brems: ',e(np-1),wt(np-1)
          END IF
          wt(np-1) = 0
          e(np-1) = 0
        END IF
        return
      END IF
      IF (( iarg .EQ. 13 .OR. iarg .EQ. 14 )) THEN
        IF (( .false. )) THEN
          write(18,*) ' annihilation '
        END IF
        IF (( iarg .EQ. 13 )) THEN
          erad_tmp = erad_tmp + e(np) + e(np-1) - 2*prm
        END IF
        e(np) = 0
        wt(np) = 0
        e(np-1) = 0
        wt(np-1) = 0
        return
      END IF
      IF (( iarg .EQ. 9 .OR. iarg .EQ. 11 )) THEN
        IF (( .false. )) THEN
          write(18,*) ' Moller or Bhabha '
        END IF
        DO 1321 ip=npold,np
          IF (( iq(ip) .EQ. 0 )) THEN
            erad_tmp = erad_tmp + e(ip)
            wt(ip) = 0
            e(ip) = 0
          END IF
1321    CONTINUE
1322    CONTINUE
        return
      END IF
      WRITE(6,1330)
1330  FORMAT('We should not get here!!!!')
      write(6,*) 'IARG = ',iarg
      return
      end
      subroutine source
      implicit none
      COMMON/GetInput/ ALLOWED_INPUTS(100,0:5),   VALUES_SOUGHT(100),  C
     *HAR_VALUE(100,100),  VALUE(100,100),  DEFAULT(100),  VALUE_MIN(100
     *),  VALUE_MAX(100),  NVALUE(100),  TYPE(100),      ERROR_FLAGS(100
     *),   i_errors,  NMIN, NMAX,   ERROR_FLAG,  DELIMETER
      character ALLOWED_INPUTS*64,VALUES_SOUGHT*64, CHAR_VALUE*256,DELIM
     *ETER*64
      real*8 VALUE,DEFAULT,VALUE_MIN,VALUE_MAX
      integer*4 NVALUE,TYPE,NMIN,NMAX,ERROR_FLAG,ERROR_FLAGS,i_errors
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      COMMON/UPHIOT/THETA,SINTHE,COSTHE,SINPHI, COSPHI,PI,TWOPI,PI5D2
      real*8 THETA,  SINTHE,  COSTHE,  SINPHI,  COSPHI,  PI,TWOPI,PI5D2
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      common/energies/ eis,neis
      real*8 eis(2000)
      integer*4 neis
      integer*4 ival
      integer*4 mono
      integer*4 nensrc,mode,iqi,source_type
      real*8 ensrcd(0:1000),srcpdf(1000),srcpdf_at(1000)
      integer*4 srcbin_at(1000)
      integer*4 i
      real*8 ei,eave,sum,ui,vi,wi,angle,rbeam,distance
      real*8 esum,esum2,ecount,aux,aux2
      integer*4 ounit
      real*8 emax,e,es,des,eik
      integer*4 iqin,irin
      real*8 ein,xin,yin,zin,uin,vin,win,wtin
      real*8 r,phi,alias_sample
      real*8 bwidth
      real*8 e_min, e_max
      integer*4 itimes
      save mono,nensrc,mode,ensrcd,srcpdf,srcpdf_at,srcbin_at,eave,sum,
     *esum,esum2,ecount,ei,iqi,ui,vi,wi,angle,rbeam,distance, source_typ
     *e,eik
      character*256 spec_file
      ival = 0
      esum=0
      esum2=0
      ecount=0
      ival = ival + 1
      values_sought(ival) = 'INCIDENT SPECTRUM'
      type(ival) = 3
      allowed_inputs(ival,0) = 'MONO-ENERGY'
      allowed_inputs(ival,1) = 'SPECTRUM'
      allowed_inputs(ival,2) = 'MONO-ENERGY-LIN-RANGE'
      allowed_inputs(ival,3) = 'MONO-ENERGY-LOG-RANGE'
      nvalue(ival) = 1
      delimeter = 'SOURCE INPUT'
      NMIN = ival
      NMAX = ival
      CALL GET_INPUT
      mono = value(ival,1)
      ival = ival + 1
      values_sought(ival) = 'INCIDENT CHARGE'
      type(ival) = 0
      nvalue(ival) = 1
      value_min(ival) = -1
      value_max(ival) = 1
      default(ival) = -1
      NMIN = ival
      NMAX = ival
      CALL GET_INPUT
      iqi = value(ival,1)
      IF (( mono .EQ. 0 )) THEN
        ival = ival + 1
        values_sought(ival) = 'INCIDENT KINETIC ENERGY'
        type(ival) = 0
        nvalue(ival) = 0
        value_min(ival) = 0
        value_max(ival) = 1e10
        default(ival) = 1
        NMIN = ival
        NMAX = ival
        CALL GET_INPUT
        ei = value(ival,1)
        eik = ei
        IF (( iqi .NE. 0 )) THEN
          ei = ei + rm
        END IF
        neis = nvalue(ival)
        IF ((neis.GT.2000)) THEN
          write(6,*) '!!!!!!!!!    ERROR  !!!!!!!!!!!!!!!!!!!!!!!!'
          write(6,*) 'too many energies requested, increase MXENE.'
          write(6,*) 'MXENE =',2000,'neis = ',neis
          stop
        END IF
        write(6,*) 'number of energies = ', neis
        DO 1341 i=1,neis
          eis(i)=value(ival,i)+rm*abs(iqi)
          write(6,'(A,I5,A,f8.5)') 'E(',i,')=', value(ival,i)
1341    CONTINUE
1342    CONTINUE
      ELSE IF((mono .EQ. 2)) THEN
        ival = ival + 1
        values_sought(ival) = 'INCIDENT KINETIC ENERGY'
        type(ival) = 0
        nvalue(ival) = 3
        value_min(ival) = 0
        value_max(ival) = 1e10
        default(ival) = 1
        NMIN = ival
        NMAX = ival
        CALL GET_INPUT
        ei = value(ival,1)
        eik = ei
        IF (( iqi .NE. 0 )) THEN
          ei = ei + rm
        END IF
        bwidth = value(ival,3)
        neis = abs(value(ival,2)-value(ival,1))/bwidth+1
        IF ((neis.GT.2000)) THEN
          write(6,*) '!!!!!!!!!    ERROR  !!!!!!!!!!!!!!!!!!!!!!!!'
          write(6,*) 'too many energies requested, increase MXENE.'
          write(6,*) 'MXENE =',2000,' neis = ',neis
          stop
        END IF
        write(6,*) 'Linear energy grid '
        write(6,*) '========================'
        write(6,*) 'Emin = ', value(ival,1),' Emax = ', value(ival,2)
        write(6,*) 'bwidth = ', bwidth
        write(6,*) 'number of energies = (Emax-Emin)/bwidth+1= ', neis
        write(6,*)
        DO 1351 i=0,neis-1
          eis(i+1)=value(ival,1)+i*bwidth+rm*abs(iqi)
          write(6,'(A,I5,A,f8.5)')'E(',i+1,')=', eis(i+1)
1351    CONTINUE
1352    CONTINUE
      ELSE IF((mono .EQ. 3)) THEN
        ival = ival + 1
        values_sought(ival) = 'INCIDENT KINETIC ENERGY'
        type(ival) = 0
        nvalue(ival) = 3
        value_min(ival) = 0
        value_max(ival) = 1e10
        default(ival) = 1
        NMIN = ival
        NMAX = ival
        CALL GET_INPUT
        ei = value(ival,1)
        eik = ei
        IF (( iqi .NE. 0 )) THEN
          ei = ei + rm
        END IF
        e_min = value(ival,1) + abs(iqi)*rm
        e_max = value(ival,2) + abs(iqi)*rm
        neis = int(value(ival,3))
        bwidth = log(e_max/e_min)/(neis-1)
        IF ((neis.GT.2000)) THEN
          write(6,*) '!!!!!!!!!    ERROR  !!!!!!!!!!!!!!!!!!!!!!!!'
          write(6,*) 'too many energies requested, increase MXENE.'
          write(6,*) 'MXENE =',2000,' neis = ',neis
          stop
        END IF
        write(6,*) 'Logarithmic energy grid '
        write(6,*) '========================'
        write(6,*) 'Emin = ', value(ival,1),' Emax = ', value(ival,2)
        write(6,*) 'number of energies = ', neis
        write(6,*) 'bwidth = log(Emax/Emin)/n = ', bwidth
        write(6,*)
        DO 1361 i=0,neis-1
          eis(i+1)=exp(log(e_min)+i*bwidth)
          write(6,'(A,I5,A,f15.8)')'E(',i+1,')=', eis(i+1)
1361    CONTINUE
1362    CONTINUE
      ELSE
        neis = 0
        ival = ival + 1
        values_sought(ival) = 'SPECTRUM FILE'
        type(ival) = 2
        nvalue(ival) = 1
        NMIN = ival
        NMAX = ival
        CALL GET_INPUT
        read(char_value(ival,1),'(a)') spec_file
        open(9,file=spec_file,status='old')
        read(9,*)
        read(9,*) nensrc,ensrcd(0),mode
        IF (( nensrc .GT. 1000 )) THEN
          write(6,*) ' Too many bins in spectrum!'
          stop
        END IF
        read(9,*) (ensrcd(i),srcpdf(i),i=1,nensrc)
        close(9)
        IF (( mode .EQ. 1 )) THEN
          DO 1371 i=1,nensrc
            srcpdf(i) = srcpdf(i)*(ensrcd(i)-ensrcd(i-1))
1371      CONTINUE
1372      CONTINUE
        END IF
        eave = 0
        sum = 0
        DO 1381 i=1,nensrc
          sum = sum + srcpdf(i)
          eave = eave + 0.5*srcpdf(i)*(ensrcd(i)+ensrcd(i-1))
1381    CONTINUE
1382    CONTINUE
        eave = eave/sum
        call prepare_alias_sampling(nensrc,srcpdf,srcpdf_at,srcbin_at)
      END IF
      ival = ival + 1
      values_sought(ival) = 'SOURCE TYPE'
      type(ival) = 0
      nvalue(ival) = 1
      value_min(ival) = 0
      value_max(ival) = 1
      default(ival) = 0
      NMIN = ival
      NMAX = ival
      CALL GET_INPUT
      source_type = value(ival,1)
      IF (( source_type .EQ. 0 )) THEN
        ival = ival + 1
        values_sought(ival) = 'INCIDENT ANGLE'
        type(ival) = 0
        nvalue(ival) = 1
        value_min(ival) = 0
        value_max(ival) = 90
        default(ival) = 0
        NMIN = ival
        NMAX = ival
        CALL GET_INPUT
        angle = value(ival,1)
        wi = angle/180*PI
        wi = cos(wi)
        ui = sqrt((1-wi)*(1+wi))
        vi = 0
        write(6,*) ' ui vi wi = ',ui,vi,wi
      ELSE IF(( source_type .EQ. 1 )) THEN
        ival = ival + 1
        values_sought(ival) = 'SOURCE RBEAM'
        type(ival) = 0
        value_min(ival) = 0
        value_max(ival) = 1e8
        default(ival) = 1
        NMIN = ival
        NMAX = ival
        CALL GET_INPUT
        rbeam = value(ival,1)
        ival = ival + 1
        values_sought(ival) = 'SOURCE DISTANCE'
        type(ival) = 0
        value_min(ival) = 0
        value_max(ival) = 1e8
        default(ival) = 100
        NMIN = ival
        NMAX = ival
        CALL GET_INPUT
        distance = value(ival,1)
      ELSE
        write(6,*) ' Unknown source type!'
        stop
      END IF
      return
      entry source_sumry(ounit)
      write(ounit,'(/a)') '                        Source
     *                   '
      write(ounit,'(a)') '==============================================
     *=================='
      write(ounit,'(a,$)') ' Incident charge              : '
      IF (( iqi .EQ. -1 )) THEN
        write(ounit,'(a)') 'Electron'
      ELSE IF(( iqi .EQ. 0 )) THEN
        write(ounit,'(a)') 'Photon'
      ELSE
        write(ounit,'(a)') 'Positron'
      END IF
      write(ounit,'(a,i2)') ' Incident spectrum            : ',mono
      IF (( mono .NE. 1 )) THEN
        IF ((neis .EQ. 1)) THEN
          write(ounit,'(a,$)') ' Incident kinetic energy (MeV): '
          write(ounit,'(f15.8)') ei-rm*abs(iqi)
        END IF
      ELSE
        write(ounit,'(a,f10.5)') ' Average spectrum energy (MeV): ',eave
        write(ounit,'(a,f10.5)') ' Maximum spectrum energy (MeV): ',ensr
     *  cd(nensrc)
      END IF
      write(ounit,'(a,i2)') ' Source type                  : ',source_ty
     *pe
      IF (( source_type .EQ. 0 )) THEN
        write(ounit,'(a,f7.4)') ' Incident angle               : ',angle
      ELSE
        write(ounit,'(a,f7.4)') ' Beam size on front face      : ',rbeam
        write(ounit,'(a,f7.4)') ' Source-face distance         : ',dista
     *  nce
      END IF
      write(ounit,'(//)')
      return
      entry source_get_emax(emax)
      IF (( mono .EQ. 0 .OR. mono .EQ. 2 .OR. mono .EQ. 3 )) THEN
        IF (( iqi .EQ. 0 )) THEN
          emax = ei
        ELSE
          emax = ei-rm
        END IF
      ELSE
        emax = ensrcd(nensrc)
      END IF
      return
      entry source_get_eave(e)
      IF (( mono .EQ. 0 .OR. mono .EQ. 2 .OR. mono .EQ. 3 )) THEN
        IF (( iqi .EQ. 0 )) THEN
          e = ei
        ELSE
          e = ei-rm
        END IF
      ELSE
        e = eave
      END IF
      return
      entry source_get_samplede(es,des)
      IF((ecount .LE. 2))return
      aux = esum/ecount
      aux2 = esum2/ecount
      aux2 = (aux2 - aux*aux)/(ecount-1)
      IF (( aux2 .GT. 0 )) THEN
        aux2 = sqrt(aux2)
      END IF
      es = aux
      des = aux2
      return
      entry source_set_energy(ein)
      ei = ein
      return
      entry source_sample(iqin,irin,ein,xin,yin,zin,uin,vin,win,wtin)
      iqin = iqi
      irin = 1
      IF (( mono .EQ. 0 .OR. mono .EQ. 2 .OR. mono .EQ. 3 )) THEN
        ein = ei
        wtin = 1
      ELSE
        ein = alias_sample(nensrc,ensrcd,srcpdf_at,srcbin_at)
        wtin = 1
        eik = ein
        IF (( iqin .NE. 0 )) THEN
          ein = ein + rm
        END IF
      END IF
      ecount = ecount + 1
      esum = esum + eik
      esum2 = esum2 + eik*eik
      IF (( source_type .EQ. 0 )) THEN
        xin = 0
        uin = 0
        zin = 0
        uin = ui
        vin = vi
        win = wi
      ELSE
        IF((rng_seed .GT. 128))call ranmar_get
        r = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        r = rbeam*sqrt(r)
        IF((rng_seed .GT. 128))call ranmar_get
        phi = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        phi = 2*phi*PI
        xin = r*cos(phi)
        yin = r*sin(phi)
        aux = 1/sqrt(xin*xin + yin*yin + distance*distance)
        uin = xin*aux
        vin = yin*aux
        win = distance*aux
        zin = 0
      END IF
      return
      entry source_switch_energy(itimes)
      IF ((mono .EQ.1)) THEN
        return
      END IF
      ei=eis(itimes)
      eik = ei
      ecount = 0
      esum = 0
      esum2 = 0
      IF ((neis .GT. 1)) THEN
        write(6,'(a,$)') '===> Incident kinetic energy (MeV): '
        write(6,'(f15.8,10x,a)') ei-abs(iqi)*rm, '      ****************
     *****'
      END IF
      return
      end
      subroutine test_brems
      implicit none
      COMMON/ELECIN/ esig_e(1),psig_e(1), esige_max, psige_max, range_ep
     *(0:1,500,1), E_array(500,1), etae_ms0(500,1),etae_ms1(500,1),etap_
     *ms0(500,1),etap_ms1(500,1),q1ce_ms0(500,1),q1ce_ms1(500,1),q1cp_ms
     *0(500,1),q1cp_ms1(500,1),q2ce_ms0(500,1),q2ce_ms1(500,1),q2cp_ms0(
     *500,1),q2cp_ms1(500,1),blcce0(500,1),blcce1(500,1), EKE0(1),EKE1(1
     *), XR0(1),TEFF0(1),BLCC(1),XCC(1), ESIG0(500,1),ESIG1(500,1),PSIG0
     *(500,1),PSIG1(500,1),EDEDX0(500,1),EDEDX1(500,1),PDEDX0(500,1),PDE
     *DX1(500,1),EBR10(500,1),EBR11(500,1),PBR10(500,1),PBR11(500,1),PBR
     *20(500,1),PBR21(500,1),TMXS0(500,1),TMXS1(500,1), expeke1(1), IUNR
     *ST(1),EPSTFL(1),IAPRIM(1), sig_ismonotone(0:1,1)
      real*8 esig_e,   psig_e,   esige_max,  psige_max,  range_ep,  E_ar
     *ray,  etae_ms0,etae_ms1,  etap_ms0,etap_ms1,  q1ce_ms0,q1ce_ms1,
     *q1cp_ms0,q1cp_ms1,  q2ce_ms0,q2ce_ms1,  q2cp_ms0,q2cp_ms1,  blcce0
     *,blcce1,   expeke1,  EKE0,EKE1, XR0,  TEFF0,  BLCC,  XCC,  ESIG0,E
     *SIG1,  PSIG0,PSIG1,  EDEDX0,EDEDX1,  PDEDX0,PDEDX1,  EBR10,EBR11,
     * PBR10,PBR11,  PBR20,PBR21,  TMXS0,TMXS1
      integer*4 IUNRST,  EPSTFL,  IAPRIM
      logical sig_ismonotone
      COMMON/EPCONT/EDEP,EDEP_LOCAL,TSTEP,TUSTEP,USTEP,TVSTEP,VSTEP, RHO
     *F,EOLD,ENEW,EKE,ELKE,GLE,E_RANGE, x_final,y_final,z_final, u_final
     *,v_final,w_final, IDISC,IROLD,IRNEW,IAUSFL(35)
      DOUBLE PRECISION EDEP,  EDEP_LOCAL
      real*8 TSTEP,  TUSTEP,  USTEP,  VSTEP,  TVSTEP,  RHOF,  EOLD,  ENE
     *W,  EKE,  ELKE,  GLE,  E_RANGE, x_final,y_final,z_final,  u_final,
     *v_final,w_final
      integer*4 IDISC,  IROLD,  IRNEW,  IAUSFL
      common/score/ e_tot,e_tot2,e_brem,e_brem2,e_bremc,e_rad,e_rad2,e_r
     *adc, etot_tmp,ebrem_tmp,erad_tmp,my_gle,accu,ncase,calc_type, duri
     *ng_pe_compt,during_eii,verbose
      real*8 e_tot,e_tot2,e_brem,e_brem2,e_rad,e_rad2, etot_tmp,ebrem_tm
     *p,erad_tmp,e_radc,e_bremc
      integer*8 ncase
      real*8 my_gle,accu
      integer*4 calc_type,during_pe_compt,during_eii
      logical verbose
      COMMON/STACK/ E(50),X(50),Y(50),Z(50),U(50),V(50),W(50),DNEAR(50),
     *WT(50),IQ(50),IR(50),LATCH(50), LATCHI,NP,NPold
      DOUBLE PRECISION E
      real*8 X,Y,Z,  U,V,W,  DNEAR,  WT
      integer*4 IQ,  IR,  LATCH,  LATCHI, NP,  NPold
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      integer*4 icase,iqin,irin,ip,lelke
      real*8 ein, xin,yin,zin, uin,vin,win,wtin
      real*8 dedx,ebr1,sig,sig_brem,eb
      real*8 sum,sum2
      call source_sample(iqin,irin,ein,xin,yin,zin,uin,vin,win,wtin)
      np=1
      e(np)=ein
      iq(np)=-1
      ir(np)=1
      wt(np)=1
      x(np)=0
      y(np)=0
      z(np)=0
      u(np)=0
      v(np)=0
      w(np)=1
      elke = log(ein-prm)
      medium=1
      Lelke=eke1(MEDIUM)*elke+eke0(MEDIUM)
      dedx=ededx1(Lelke,MEDIUM)*elke+ededx0(Lelke,MEDIUM)
      ebr1=ebr11(Lelke,MEDIUM)*elke+ebr10(Lelke,MEDIUM)
      sig=esig1(Lelke,MEDIUM)*elke+esig0(Lelke,MEDIUM)
      sig_brem = sig*ebr1
      write(6,*) ' Incident energy: ',ein-prm,elke
      write(6,*) ' dedx: ',dedx
      write(6,*) ' sig:  ',sig
      write(6,*) ' ebr1: ',ebr1
      write(6,*) ' Brems cross section: ',sig_brem
      DO 1391 icase=1,ncase
        np=1
        e(np)=ein
        call brems
        eb=0
        DO 1401 ip=1,np
          IF ((iq(ip) .EQ. 0)) THEN
            eb = eb + e(ip)
          END IF
1401    CONTINUE
1402    CONTINUE
        sum = sum + eb
        sum2 = sum2 + eb*eb
1391  CONTINUE
1392  CONTINUE
      sum = sum/ncase
      sum2 = sum2/ncase
      sum2 = sum2 - sum*sum
      IF (( sum2 .GT. 0 )) THEN
        sum2 = sqrt(sum2/(ncase-1))
      END IF
      write(6,*) ' Average energy per brem: ',sum,' +/- ',sum2
      write(6,*) ' Rad. stopping power: ',sum*sig_brem,' +/- ',sum2*sig_
     *brem
      stop
      end
      subroutine test_compton
      implicit none
      COMMON/PHOTIN/ EBINDA(1), GE0(1),GE1(1), GMFP0(2000,1),GMFP1(2000,
     *1),GBR10(2000,1),GBR11(2000,1),GBR20(2000,1),GBR21(2000,1), RCO0(1
     *),RCO1(1), RSCT0(100,1),RSCT1(100,1), COHE0(2000,1),COHE1(2000,1),
     *  PHOTONUC0(2000,1),PHOTONUC1(2000,1), DPMFP, MPGEM(1,1), NGR(1)
      real*8 EBINDA,  GE0,GE1,  GMFP0,GMFP1,  GBR10,GBR11,  GBR20,GBR21,
     *  RCO0,RCO1,  RSCT0,RSCT1,  COHE0,COHE1,   PHOTONUC0,PHOTONUC1,  D
     *PMFP
      integer*4
     *                  MPGEM,
     *                          NGR
      COMMON/EPCONT/EDEP,EDEP_LOCAL,TSTEP,TUSTEP,USTEP,TVSTEP,VSTEP, RHO
     *F,EOLD,ENEW,EKE,ELKE,GLE,E_RANGE, x_final,y_final,z_final, u_final
     *,v_final,w_final, IDISC,IROLD,IRNEW,IAUSFL(35)
      DOUBLE PRECISION EDEP,  EDEP_LOCAL
      real*8 TSTEP,  TUSTEP,  USTEP,  VSTEP,  TVSTEP,  RHOF,  EOLD,  ENE
     *W,  EKE,  ELKE,  GLE,  E_RANGE, x_final,y_final,z_final,  u_final,
     *v_final,w_final
      integer*4 IDISC,  IROLD,  IRNEW,  IAUSFL
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      common/score/ e_tot,e_tot2,e_brem,e_brem2,e_bremc,e_rad,e_rad2,e_r
     *adc, etot_tmp,ebrem_tmp,erad_tmp,my_gle,accu,ncase,calc_type, duri
     *ng_pe_compt,during_eii,verbose
      real*8 e_tot,e_tot2,e_brem,e_brem2,e_rad,e_rad2, etot_tmp,ebrem_tm
     *p,erad_tmp,e_radc,e_bremc
      integer*8 ncase
      real*8 my_gle,accu
      integer*4 calc_type,during_pe_compt,during_eii
      logical verbose
      COMMON/STACK/ E(50),X(50),Y(50),Z(50),U(50),V(50),W(50),DNEAR(50),
     *WT(50),IQ(50),IR(50),LATCH(50), LATCHI,NP,NPold
      DOUBLE PRECISION E
      real*8 X,Y,Z,  U,V,W,  DNEAR,  WT
      integer*4 IQ,  IR,  LATCH,  LATCHI, NP,  NPold
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      integer*4 icase,iqin,irin,ip,lgle
      real*8 ein, xin,yin,zin, uin,vin,win,wtin
      real*8 gmfp,gbr1,gbr2
      real*8 sumr,sumr2,sume,sume2,sumtr,sumtr2
      real*8 es,des,ee,factor
      medium = 1
      DO 1411 icase=1,ncase
        call source_sample(iqin,irin,ein,xin,yin,zin,uin,vin,win,wtin)
        IF (( iqin .NE. 0 )) THEN
          write(6,*) ' test_compton: only works for photons!'
          stop
        END IF
        gle = log(ein)
        Lgle=ge1(MEDIUM)*gle+ge0(MEDIUM)
        gmfp=gmfp1(Lgle,MEDIUM)*gle+gmfp0(Lgle,MEDIUM)
        gbr1=GBR11(LGLE,MEDIUM)*GLE+GBR10(LGLE,MEDIUM)
        GBR2=GBR21(LGLE,MEDIUM)*GLE+GBR20(LGLE,MEDIUM)
        np=1
        e(np)=ein
        iq(np)=0
        ir(np)=1
        wt(np)=1
        x(np)=0
        y(np)=0
        z(np)=0
        u(np)=0
        v(np)=0
        w(np)=1
        call compt
        IF (( np .GT. 1 )) THEN
          IF (( iq(1) .EQ. 0 )) THEN
            ee = ein - e(1)
          ELSE
            ee = ein - e(2)
          END IF
          sume = sume + ee
          sume2 = sume2 + ee*ee
          ee = ee*(gbr2-gbr1) + (1-gbr2)*ein
          IF (( ein .GT. 2*prm .AND. gbr1 .GT. 0 )) THEN
            ee = ee + gbr1*(ein-2*prm)
          END IF
          ee = ee/gmfp/rho(medium)
          sumtr = sumtr + ee
          sumtr2 = sumtr2 + ee*ee
        ELSE
          sumr = sumr + 1
          sumr2 = sumr2 + 1
        END IF
1411  CONTINUE
1412  CONTINUE
      sumr = sumr/ncase
      sumr2 = sumr2/ncase
      sumr2 = sumr2 - sumr*sumr
      IF (( sumr2 .GT. 0 )) THEN
        sumr2 = sqrt(sumr2/(ncase-1))
      END IF
      sume = sume/ncase
      sume2 = sume2/ncase
      sume2 = sume2 - sume*sume
      IF (( sume2 .GT. 0 )) THEN
        sume2 = sqrt(sume2/(ncase-1))
      END IF
      sumtr = sumtr/ncase
      sumtr2 = sumtr2/ncase
      sumtr2 = sumtr2 - sumtr*sumtr
      IF (( sumtr2 .GT. 0 )) THEN
        sumtr2 = sqrt(sumtr2/(ncase-1))
      END IF
      call source_get_samplede(es,des)
      write(6,*) ' Average sampled energy:   ',es,' +/- ',des
      write(6,*) ' Average number of Compton rejections: ',sumr,' +/- ',
     *sumr2
      write(6,*) ' Average energy released in Compton events: ',sume,' +
     */- ',sume2
      factor = 160.2176462
      write(6,*) ' mutr: ',factor*sumtr,' +/- ',factor*sumtr2
      stop
      return
      end
C> @cond
      subroutine init_ranmar
      implicit none
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      integer*4 s,t
      integer*4 i,j,k,l,m,ii,jj
      IF((ixx .LE. 0 .OR. ixx .GT. 31328))ixx = 1802
      IF((jxx .LE. 0 .OR. jxx .GT. 30081))jxx = 9373
      i = mod(ixx/177,177) + 2
      j = mod(ixx, 177) + 2
      k = mod(jxx/169,178) + 1
      l = mod(jxx, 169)
      DO 1421 ii=1,97
        s = 0
        t = 8388608
        DO 1431 jj=1,24
          m = mod(mod(i*j,179)*k,179)
          IF (( fool_optimizer .EQ. 999 )) THEN
            write(6,*) i,j,k,m,s,t
          END IF
          i = j
          IF (( fool_optimizer .EQ. 999 )) THEN
            write(6,*) i,j,k,m,s,t
          END IF
          j = k
          IF (( fool_optimizer .EQ. 999 )) THEN
            write(6,*) i,j,k,m,s,t
          END IF
          k = m
          IF (( fool_optimizer .EQ. 999 )) THEN
            write(6,*) i,j,k,m,s,t
          END IF
          l = mod(53*l+1,169)
          IF (( fool_optimizer .EQ. 999 )) THEN
            write(6,*) i,j,k,m,s,t
          END IF
          IF((mod(l*m,64) .GE. 32))s = s + t
          IF (( fool_optimizer .EQ. 999 )) THEN
            write(6,*) i,j,k,m,s,t
          END IF
          t = t/2
          IF (( fool_optimizer .EQ. 999 )) THEN
            write(6,*) i,j,k,m,s,t
          END IF
1431    CONTINUE
1432    CONTINUE
        urndm(ii) = s
1421  CONTINUE
1422  CONTINUE
      crndm = 362436
      cdrndm = 7654321
      cmrndm = 16777213
      twom24 = 1./16777216.
      ixx = 97
      jxx = 33
      rng_seed = 128 + 1
      return
      end
      subroutine ranmar_get
      implicit none
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      integer*4 i,iopt
      IF((rng_seed .EQ. 999999))call init_ranmar
      DO 1441 i=1,128
        iopt = urndm(ixx) - urndm(jxx)
        IF((iopt .LT. 0))iopt = iopt + 16777216
        urndm(ixx) = iopt
        ixx = ixx - 1
        jxx = jxx - 1
        IF ((ixx .EQ. 0)) THEN
          ixx = 97
        ELSE IF(( jxx .EQ. 0 )) THEN
          jxx = 97
        END IF
        crndm = crndm - cdrndm
        IF((crndm .LT. 0))crndm = crndm + cmrndm
        iopt = iopt - crndm
        IF((iopt .LT. 0))iopt = iopt + 16777216
        rng_array(i) = iopt
1441  CONTINUE
1442  CONTINUE
      rng_seed = 1
      return
      end
      subroutine radc_init
      implicit none
      common/rad_compton/ radc_sigs(0:128),radc_sigd(0:128), radc_frej(0
     *:128,0:32), radc_x(8929), radc_fdat(13917), radc_Smax(13917), radc
     *_emin, radc_emax,radc_dw, radc_dle, radc_dlei, radc_le1, radc_bins
     *(13917), radc_ixmin1(13917),radc_ixmax1(13917), radc_ixmin2(13917)
     *,radc_ixmax2(13917), radc_ixmin3(13917),radc_ixmax3(13917), radc_i
     *xmin4(13917),radc_ixmax4(13917), radc_startx(0:128),radc_startb(0:
     *128)
      real*8 radc_sigs,  radc_sigd,  radc_frej,  radc_fdat,  radc_Smax,
     * radc_emin,  radc_emax,  radc_lemin, radc_dw,  radc_dle,  radc_dle
     *i,  radc_x,  radc_le1
      integer*2 radc_bins,  radc_ixmin1,radc_ixmax1, radc_ixmin2,radc_ix
     *max2, radc_ixmin3,radc_ixmax3, radc_ixmin4,radc_ixmax4, radc_start
     *x,radc_startb
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      common/compton_data/ iz_array(1538),  be_array(1538),  Jo_array(15
     *38),  erfJo_array(1538),   ne_array(1538),  shn_array(1538),
     *shell_array(200,1), eno_array(200,1), eno_atbin_array(200,1), n_sh
     *ell(1), radc_flag,  ibcmp(1)
      integer*4 iz_array,ne_array,shn_array,eno_atbin_array, shell_array
     *,n_shell,radc_flag
      real*8 be_array,Jo_array,erfJo_array,eno_array
      integer*2 ibcmp
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      COMMON/PHOTIN/ EBINDA(1), GE0(1),GE1(1), GMFP0(2000,1),GMFP1(2000,
     *1),GBR10(2000,1),GBR11(2000,1),GBR20(2000,1),GBR21(2000,1), RCO0(1
     *),RCO1(1), RSCT0(100,1),RSCT1(100,1), COHE0(2000,1),COHE1(2000,1),
     *  PHOTONUC0(2000,1),PHOTONUC1(2000,1), DPMFP, MPGEM(1,1), NGR(1)
      real*8 EBINDA,  GE0,GE1,  GMFP0,GMFP1,  GBR10,GBR11,  GBR20,GBR21,
     *  RCO0,RCO1,  RSCT0,RSCT1,  COHE0,COHE1,   PHOTONUC0,PHOTONUC1,  D
     *PMFP
      integer*4
     *                  MPGEM,
     *                          NGR
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      character radc_file*256
      integer*4 lnblnk1,egs_get_unit
      integer*4 radc_unit, want_radc_unit
      integer*4 nskip,i,j,ne,nu,ny1,ny2,nw,jh,jl
      integer*4 lgle,icheck
      real*8 aux1,gmfp,gmfp_old,gbr1,gbr1_old,gbr2,gbr2_old,gle,frad, ac
     *heck,acheck1,sum,aux
      integer*4 nx,nbox,ixtot,ibtot
      IF (( radc_flag .EQ. 0 )) THEN
        write(i_log,*) 'Radiative Compton corrections not requested'
        return
      END IF
      write(i_log,*) ' '
      write(i_log,*) 'Radiative Compton corrections requested:'
      write(i_log,'(a,$)') '    Reading radiative Compton data ...'
      DO 1451 i=1,len(radc_file)
        radc_file(i:i) = ' '
1451  CONTINUE
1452  CONTINUE
      radc_file = hen_house(:lnblnk1(hen_house)) // 'data' // '/' // 'ra
     *d_compton1.data'
      want_radc_unit = 81
      radc_unit = egs_get_unit(want_radc_unit)
      IF (( radc_unit .LT. 1 )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) 'radc_init: failed to get a free fortran unit'
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      open(radc_unit,file=radc_file,status='old',err=1460)
      read(radc_unit,*,err=1470) radc_emin,radc_emax,radc_dw,ne,nu
      IF (( ne .NE. 128 .OR. nu .NE. 32 )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) 'radc_init: inconsistent data'
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      radc_dle = log(radc_emax/radc_emin)/ne
      radc_dlei = 1/radc_dle
      read(radc_unit,*,err=1470) (radc_sigs(i),i=0,ne)
      DO 1481 i=0,ne
        read(radc_unit,*,err=1470) (radc_frej(i,j),j=0,nu)
1481  CONTINUE
1482  CONTINUE
      read(radc_unit,*)
      read(radc_unit,*,err=1470) (radc_sigd(i),i=0,ne)
      ixtot=1
      ibtot=1
      DO 1491 i=0,ne
        read(radc_unit,*) nx,nbox
        IF (( ixtot-1+nx .GT. 8929 )) THEN
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,*) 'Incosistent number of box boundaries'
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        END IF
        IF (( ibtot-1+nbox .GT. 13917 )) THEN
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,*) 'Incosistent number of boxes'
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        END IF
        radc_startx(i) = ixtot
        radc_startb(i) = ibtot
        read(radc_unit,*,err=1470) (radc_x(j),j=ixtot,ixtot-1+nx)
        DO 1501 j=ibtot,ibtot-1+nbox
          read(radc_unit,*,err=1470) radc_Smax(j),radc_fdat(j), radc_bin
     *    s(j), radc_ixmin1(j),radc_ixmax1(j), radc_ixmin2(j),radc_ixmax
     *    2(j), radc_ixmin3(j),radc_ixmax3(j), radc_ixmin4(j),radc_ixmax
     *    4(j)
1501    CONTINUE
1502    CONTINUE
        ixtot = ixtot + nx
        ibtot = ibtot + nbox
1491  CONTINUE
1492  CONTINUE
      close(radc_unit)
      write(i_log,*) ' OK'
      radc_le1 = -log(radc_emin*prm)*radc_dlei
      write(i_log,'(a,$)') '    Modifying cross sections and branching r
     *atios ...'
      DO 1511 medium=1,nmed
        DO 1521 i=1,mge(medium)
          gle = (i - ge0(medium))/ge1(medium)
          lgle = i
          gmfp=gmfp1(Lgle,MEDIUM)*gle+gmfp0(Lgle,MEDIUM)
          gbr1=gbr11(Lgle,MEDIUM)*gle+gbr10(Lgle,MEDIUM)
          gbr2=gbr21(Lgle,MEDIUM)*gle+gbr20(Lgle,MEDIUM)
          IF (( exp(gle) .GT. radc_emin*prm )) THEN
            acheck = radc_dlei*gle + radc_le1
            icheck = acheck
            acheck = acheck - icheck
            acheck1 = 1 - acheck
            frad = (radc_sigs(icheck)+radc_sigd(icheck))*acheck1 + (radc
     *      _sigs(icheck+1)+radc_sigd(icheck+1))*acheck
          ELSE
            frad = 1
          END IF
          aux = 1/(1 + (gbr2-gbr1)*(frad-1))
          aux1 = (gbr2-gbr1)*frad
          gmfp = gmfp*aux
          gbr1 = gbr1*aux
          gbr2 = gbr1 + aux1*aux
          IF (( i .GT. 1 )) THEN
            gmfp1(i-1,medium) = (gmfp - gmfp_old)*ge1(medium)
            gmfp0(i-1,medium) = gmfp - gmfp1(i-1,medium)*gle
            gbr11(i-1,medium) = (gbr1 - gbr1_old)*ge1(medium)
            gbr10(i-1,medium) = gbr1 - gbr11(i-1,medium)*gle
            gbr21(i-1,medium) = (gbr2 - gbr2_old)*ge1(medium)
            gbr20(i-1,medium) = gbr2 - gbr21(i-1,medium)*gle
          END IF
          gmfp_old = gmfp
          gbr1_old = gbr1
          gbr2_old = gbr2
1521    CONTINUE
1522    CONTINUE
        gmfp1(mge(medium),medium) = gmfp1(mge(medium)-1,medium)
        gmfp0(mge(medium),medium) = gmfp0(mge(medium)-1,medium)
        gbr11(mge(medium),medium) = gbr11(mge(medium)-1,medium)
        gbr10(mge(medium),medium) = gbr10(mge(medium)-1,medium)
        gbr21(mge(medium),medium) = gbr21(mge(medium)-1,medium)
        gbr20(mge(medium),medium) = gbr20(mge(medium)-1,medium)
1511  CONTINUE
1512  CONTINUE
      write(i_log,*) 'OK'
      return
1460  CONTINUE
      write(i_log,'(/a)') '***************** Error: '
      write(i_log,*) 'Failed to open rad_compton.data'
      write(i_log,'(/a)') '***************** Quiting now.'
      call exit(1)
1470  CONTINUE
      write(i_log,'(/a)') '***************** Error: '
      write(i_log,*) 'Error while reading radiative Compton data'
      write(i_log,'(/a)') '***************** Quiting now.'
      call exit(1)
      end
      subroutine sample_double_compton(wo,ie)
      implicit none
      real*8 wo
      integer*4 ie
      common/rad_compton/ radc_sigs(0:128),radc_sigd(0:128), radc_frej(0
     *:128,0:32), radc_x(8929), radc_fdat(13917), radc_Smax(13917), radc
     *_emin, radc_emax,radc_dw, radc_dle, radc_dlei, radc_le1, radc_bins
     *(13917), radc_ixmin1(13917),radc_ixmax1(13917), radc_ixmin2(13917)
     *,radc_ixmax2(13917), radc_ixmin3(13917),radc_ixmax3(13917), radc_i
     *xmin4(13917),radc_ixmax4(13917), radc_startx(0:128),radc_startb(0:
     *128)
      real*8 radc_sigs,  radc_sigd,  radc_frej,  radc_fdat,  radc_Smax,
     * radc_emin,  radc_emax,  radc_lemin, radc_dw,  radc_dle,  radc_dle
     *i,  radc_x,  radc_le1
      integer*2 radc_bins,  radc_ixmin1,radc_ixmax1, radc_ixmin2,radc_ix
     *max2, radc_ixmin3,radc_ixmax3, radc_ixmin4,radc_ixmax4, radc_start
     *x,radc_startb
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      COMMON/STACK/ E(50),X(50),Y(50),Z(50),U(50),V(50),W(50),DNEAR(50),
     *WT(50),IQ(50),IR(50),LATCH(50), LATCHI,NP,NPold
      DOUBLE PRECISION E
      real*8 X,Y,Z,  U,V,W,  DNEAR,  WT
      integer*4 IQ,  IR,  LATCH,  LATCHI, NP,  NPold
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      real*8 rnno,y1,y2,yw,s_max,asamp,alpha1, yb1,yb2,yb3,yb4,dy1,dy2,d
     *y3,dy4,wo_save,Vol
      real*8 cost1,cost2,w1,w2,cost12,cphi,acphi,a1,b1,z1,z2
      real*8 facct,aux,ax,bx,w1_max,w1t,ww1tot,zz,facw1, k1,k2,k3,k1i,k2
     *i,k3i,k1p,k2p,k3p,k1pi,k2pi,k3pi, a,b,c,Ac,Bc,rho,s,xx,Xc, px1,py1
     *,pz1,px2,py2,pz2,pxe,pye,pze,pp,Ep, sindel,cosdel,sinpsi,sphi,sint
     *1,sint2
      real*8 xphi,xphi2,yphi,yphi2,rhophi2
      integer*4 ibin,nbox,ix
      IF (( np+2 .GT. 50 )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,'(//,3a,/,2(a,i9))') ' In subroutine ','sample_doubl
     *e_compton', ' stack size exceeded! ',' $MAXSTACK = ',50,' np = ',n
     *  p+2
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      IF((rng_seed .GT. 128))call ranmar_get
      rnno = rng_array(rng_seed)*twom24
      rng_seed = rng_seed + 1
      nbox = radc_startb(ie+1) - radc_startb(ie)
      ibin = radc_startb(ie) + rnno*nbox
      IF((rng_seed .GT. 128))call ranmar_get
      rnno = rng_array(rng_seed)*twom24
      rng_seed = rng_seed + 1
      IF((rnno .GT. radc_fdat(ibin)))ibin = radc_startb(ie) + radc_bins(
     *ibin)
      s_max = radc_Smax(ibin)
      ix = radc_startx(ie)
      yb1 = radc_x(ix+radc_ixmin1(ibin))
      dy1 = radc_x(ix+radc_ixmax1(ibin)) - yb1
      yb2 = radc_x(ix+radc_ixmin2(ibin))
      dy2 = radc_x(ix+radc_ixmax2(ibin)) - yb2
      yb3 = radc_x(ix+radc_ixmin3(ibin))
      dy3 = radc_x(ix+radc_ixmax3(ibin)) - yb3
      yb4 = radc_x(ix+radc_ixmin4(ibin))
      dy4 = radc_x(ix+radc_ixmax4(ibin)) - yb4
      Vol = dy1*dy2*dy3*dy4
1530  CONTINUE
      wo_save = wo
      wo = radc_emin*exp(radc_dle*ie)
      asamp = 0.25*wo*(1+wo)
      alpha1 = log(0.5*(1+sqrt(1+4*asamp)))
1540  CONTINUE
1541    CONTINUE
        IF((rng_seed .GT. 128))call ranmar_get
        rnno = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        y1 = yb1 + dy1*rnno
        facct = 2*y1
        y1 = y1*y1
        IF((rng_seed .GT. 128))call ranmar_get
        rnno = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        y2 = yb2 + dy2*rnno
        IF((rng_seed .GT. 128))call ranmar_get
        rnno = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        yw = yb3 + dy3*rnno
        IF((rng_seed .GT. 128))call ranmar_get
        rnno = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        yphi = yb4 + dy4*rnno
        IF (( y2 .GT. 0 )) THEN
          z2 = 2*exp(y2*alpha1)-1
          cost2 = 1 - (z2*z2-1)/(2*asamp)
          z1 = z2/(1+y1*(z2-1))
          cost1 = 1 - 2*(z1*z1-1)/(z2*z2-1)
        ELSE
          z2 = 1
          z1 = 1
          cost2 = 1
          cost1 = 2*y1-1
        END IF
        facct = facct*alpha1/asamp*z1*z1*z1
        acphi = sqrt((1-cost1*cost1)*(1-cost2*cost2))
        a1 = 1 + wo*(1+wo)*(1-cost1)*(1-cost2)
        b1 = wo*acphi
        IF (( abs(yphi-0.5) .GT. 1e-4 )) THEN
          aux = tan(3.14159265358979323846*yphi)
          aux = aux*aux
          cphi = (a1-b1-(a1+b1)*aux)/(a1-b1+(a1+b1)*aux)
        ELSE
          cphi = -1
        END IF
        facct = facct*(a1 + b1*cphi)/sqrt(a1*a1-b1*b1)
        cost12 = cost1*cost2+acphi*cphi
        ax = 2 + wo*(1-cost1) + wo*(1-cost2)
        bx = wo*(1-cost12)/(ax*ax)
        IF (( bx .GT. 1e-3 )) THEN
          w1_max = wo*(1-sqrt(1-4*bx))/(2*bx*ax)
        ELSE
          w1_max = wo/ax*(1 + bx*(1 + bx*(2 + 5*bx)))
        END IF
        w1t = wo/(1 + wo*(1-cost1))
        IF((w1_max .LE. radc_dw))goto 1540
        ww1tot = log(w1_max*(w1t-radc_dw)/(radc_dw*(w1t-w1_max)))
        zz = exp(yw*ww1tot)
        w1 = zz*w1t*radc_dw/(w1t+(zz-1)*radc_dw)
        facw1 = w1*(w1t-w1)*ww1tot/w1t
        w2 = (wo - w1*(1+wo*(1-cost1)))/(1+wo*(1-cost2)-w1*(1-cost12))
        IF((w1 .GT. w2))goto 1540
        k1 = w1
        k2 = w2
        k3 = -wo
        k1p = -w1*(1 + wo*(1-cost1) - w2*(1-cost12))
        k2p = -w2*(1 + wo*(1-cost2) - w1*(1-cost12))
        k3p = wo*(1 - w1*(1-cost1) - w2*(1-cost2))
        k1i = 1/k1
        k2i = 1/k2
        k3i = 1/k3
        k1pi = 1/k1p
        k2pi = 1/k2p
        k3pi = 1/k3p
        a = k1i + k2i + k3i
        b = k1pi + k2pi + k3pi
        c = k1i*k1pi + k2i*k2pi + k3i*k3pi
        xx = k1 + k2 + k3
        zz = k1*k1p + k2*k2p + k3*k3p
        Ac = k1*k2*k3
        Bc = k1p*k2p*k3p
        rho = k1*k1pi+k1p*k1i+k2*k2pi+k2p*k2i+k3*k3pi+k3p*k3i
        Xc = 2*(a*b-c)*((a+b)*(xx+2)-(a*b-c)-8)-2*xx*(a*a+b*b)- 8*c+4*xx
     *  /(Ac*Bc)*((Ac+Bc)*(xx+1)-(a*Ac+b*Bc)*(2+zz*(1-xx)/xx)+ xx*xx*(1-
     *  zz)+2*zz)-2*rho*(a*b+c*(1-xx))
        s = Xc*Vol*w1*w2*facct*facw1/(wo*(1+wo*(1-cost2)-w1*(1-cost12)))
        IF((rng_seed .GT. 128))call ranmar_get
        rnno = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        IF(rnno*s_max.LE.s)GO TO1542
      GO TO 1541
1542  CONTINUE
      wo = wo_save
      asamp = 0.25*wo*(1+wo)
      alpha1 = log(0.5*(1+sqrt(1+4*asamp)))
      z2 = 2*exp(y2*alpha1)-1
      cost2 = 1 - (z2*z2-1)/(2*asamp)
      z1 = z2/(1+y1*(z2-1))
      cost1 = 1 - 2*(z1*z1-1)/(z2*z2-1)
      acphi = sqrt((1-cost1*cost1)*(1-cost2*cost2))
      a1 = 1 + wo*(1+wo)*(1-cost1)*(1-cost2)
      b1 = wo*acphi
      IF (( abs(yphi-0.5) .GT. 1e-4 )) THEN
        aux = tan(3.14159265358979323846*yphi)
        aux = aux*aux
        cphi = (a1-b1-(a1+b1)*aux)/(a1-b1+(a1+b1)*aux)
      ELSE
        cphi = -1
      END IF
      cost12 = cost1*cost2+acphi*cphi
      ax = 2 + wo*(1-cost1) + wo*(1-cost2)
      bx = wo*(1-cost12)/(ax*ax)
      IF (( bx .GT. 1e-3 )) THEN
        w1_max = wo*(1-sqrt(1-4*bx))/(2*bx*ax)
      ELSE
        w1_max = wo/ax*(1 + bx*(1 + bx*(2 + 5*bx)))
      END IF
      w1t = wo/(1 + wo*(1-cost1))
      IF (( w1_max .GT. radc_dw )) THEN
        ww1tot = log(w1_max*(w1t-radc_dw)/(radc_dw*(w1t-w1_max)))
        zz = exp(yw*ww1tot)
        w1 = zz*w1t*radc_dw/(w1t+(zz-1)*radc_dw)
        w2 = (wo - w1*(1+wo*(1-cost1)))/(1+wo*(1-cost2)-w1*(1-cost12))
        IF (( w1 .LT. w2 )) THEN
          sphi = sqrt(1 - cphi*cphi)
          sint1 = sqrt(1-cost1*cost1)
          sint2 = sqrt(1-cost2*cost2)
        ELSE
          goto 1530
        END IF
      ELSE
        goto 1530
      END IF
      px1 = w1*sint1*cphi
      py1 = w1*sint1*sphi
      pz1 = w1*cost1
      px2 = w2*sint2
      py2 = 0
      pz2 = w2*cost2
1551  CONTINUE
        IF((rng_seed .GT. 128))call ranmar_get
        xphi = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        xphi = 2*xphi - 1
        xphi2 = xphi*xphi
        IF((rng_seed .GT. 128))call ranmar_get
        yphi = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        yphi2 = yphi*yphi
        rhophi2 = xphi2 + yphi2
        IF(rhophi2.LE.1)GO TO1552
      GO TO 1551
1552  CONTINUE
      rhophi2 = 1/rhophi2
      cphi = (xphi2 - yphi2)*rhophi2
      sphi = 2*xphi*yphi*rhophi2
      aux = px1*sphi
      px1 = px1*cphi - py1*sphi
      py1 = aux + py1*cphi
      py2 = sphi*px2
      px2 = cphi*px2
      pxe = -px1 - px2
      pye = -py1 - py2
      pze = wo - pz1 - pz2
      Ep = wo - w1 - w2 + 1
      pp = 1/sqrt(pxe*pxe + pye*pye + pze*pze)
      NPold = np
      X(np+1)=X(np)
      Y(np+1)=Y(np)
      Z(np+1)=Z(np)
      IR(np+1)=IR(np)
      WT(np+1)=WT(np)
      DNEAR(np+1)=DNEAR(np)
      LATCH(np+1)=LATCH(np)
      X(np+2)=X(np)
      Y(np+2)=Y(np)
      Z(np+2)=Z(np)
      IR(np+2)=IR(np)
      WT(np+2)=WT(np)
      DNEAR(np+2)=DNEAR(np)
      LATCH(np+2)=LATCH(np)
      a = u(np)
      b = v(np)
      c = w(np)
      sinpsi = a*a + b*b
      IF (( sinpsi .GT. 1e-20 )) THEN
        sinpsi = sqrt(sinpsi)
        sindel = b/sinpsi
        cosdel = a/sinpsi
        u(np) = (c*cosdel*px2 - sindel*py2 + a*pz2)/w2
        v(np) = (c*sindel*px2 + cosdel*py2 + b*pz2)/w2
        w(np) = (c*pz2 - sinpsi*px2)/w2
        iq(np) = 0
        E(np) = w2*prm
        np = np+1
        u(np) = (c*cosdel*px1 - sindel*py1 + a*pz1)/w1
        v(np) = (c*sindel*px1 + cosdel*py1 + b*pz1)/w1
        w(np) = (c*pz1 - sinpsi*px1)/w1
        iq(np) = 0
        E(np) = w1*prm
        np = np+1
        u(np) = (c*cosdel*pxe - sindel*pye + a*pze)*pp
        v(np) = (c*sindel*pxe + cosdel*pye + b*pze)*pp
        w(np) = (c*pze - sinpsi*pxe)*pp
        iq(np) = -1
        E(np) = Ep*prm
      ELSE
        u(np) = px2/w2
        v(np) = py2/w2
        w(np) = c*pz2/w2
        iq(np) = 0
        E(np) = w2*prm
        np = np+1
        u(np) = px1/w1
        v(np) = py1/w1
        w(np) = c*pz1/w1
        iq(np) = 0
        E(np) = w1*prm
        np = np+1
        u(np) = pxe*pp
        v(np) = pye*pp
        w(np) = c*pze*pp
        iq(np) = -1
        E(np) = Ep*prm
      END IF
      return
      end
      SUBROUTINE GET_INPUT
      IMPLICIT NONE
      COMMON/GetInput/ ALLOWED_INPUTS(100,0:5),   VALUES_SOUGHT(100),  C
     *HAR_VALUE(100,100),  VALUE(100,100),  DEFAULT(100),  VALUE_MIN(100
     *),  VALUE_MAX(100),  NVALUE(100),  TYPE(100),      ERROR_FLAGS(100
     *),   i_errors,  NMIN, NMAX,   ERROR_FLAG,  DELIMETER
      character ALLOWED_INPUTS*64,VALUES_SOUGHT*64, CHAR_VALUE*256,DELIM
     *ETER*64
      real*8 VALUE,DEFAULT,VALUE_MIN,VALUE_MAX
      integer*4 NVALUE,TYPE,NMIN,NMAX,ERROR_FLAG,ERROR_FLAGS,i_errors
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      CHARACTER*256 TEXT
      CHARACTER*256 KEEPTEXT
      CHARACTER*256 ORIGTEXT
      CHARACTER*256 TEXTPIECE
      CHARACTER*40 DELIM_START
      CHARACTER*40 DELIM_END
      CHARACTER*64 VNAME
      CHARACTER*64 VNAME1
      integer*4 CURSOR
      integer*4 IINDEX
      integer*4 iVNAME
      integer*4 IVAL
      integer*4 UNITNUM
      integer*4 ERR
      integer*4 I,J,K,CHECK
      integer*4 LINE
      integer*4 INT_VALUE
      integer*4 INT_VALUE_MIN
      integer*4 INT_VALUE_MAX
      logical ALLOWED
      logical START_FOUND
      integer*4 ifound,length,lll,Kconvert
      integer*4 lnblnk1
      logical IDEBUG
      character*1 blank
      integer*4 error_level
      integer*4 the_level
      data blank/' '/
      data error_level/1/
      save error_level
      IDEBUG = .false.
      ERROR_FLAG = 0
      IF ((IDEBUG)) THEN
        WRITE(6,1560)NMIN,NMAX, 100
1560    FORMAT(' Entering get_inputs seeking values', I5,' to', I5, '  w
     *ith a max allowed of',I5)
      END IF
      IF ((NMAX .LT. NMIN .OR. NMAX .GT. 100)) THEN
        WRITE(6,1570)NMAX, NMIN, 100
1570    FORMAT(//' Error entering get_inputs: Asked for values from',I5,
     *' to',I5, '    with a max of',I5//' This implies a bug in the call
     *ing routine'/ ' Fix it up and try again.  Stopping now.')
        STOP
      END IF
      ERR=i_errors
      UNITNUM=i_input
      DELIM_START=':START '//DELIMETER(:lnblnk1(DELIMETER))//':'
      DELIM_END=':STOP '//DELIMETER(:lnblnk1(DELIMETER))//':'
      DO 1581 Kconvert=1,lnblnk1(DELIM_START)
        CURSOR=ICHAR(DELIM_START(Kconvert:Kconvert))
        IF (((CURSOR.GE.97).AND.(CURSOR.LE.122))) THEN
          CURSOR=CURSOR-32
          DELIM_START(Kconvert:Kconvert)=CHAR(CURSOR)
        END IF
1581  CONTINUE
1582  CONTINUE
      DO 1591 Kconvert=1,lnblnk1(DELIM_END)
        CURSOR=ICHAR(DELIM_END(Kconvert:Kconvert))
        IF (((CURSOR.GE.97).AND.(CURSOR.LE.122))) THEN
          CURSOR=CURSOR-32
          DELIM_END(Kconvert:Kconvert)=CHAR(CURSOR)
        END IF
1591  CONTINUE
1592  CONTINUE
      IF ((IDEBUG)) THEN
        WRITE(6,1600)DELIM_START,DELIM_END
1600    FORMAT(' start and stop delimeters are:'/ A/A/)
      END IF
      DO 1611 I=NMIN,NMAX
        REWIND (UNITNUM)
        LINE=0
        CHECK=0
        ERROR_FLAGS(I)=0
        IF ((TYPE(I) .EQ. 0 .OR. TYPE(I) .EQ. 1)) THEN
          VALUE(I,1) = DEFAULT(I)
        END IF
        IF ((TYPE(I) .EQ. 3)) THEN
          VALUE(I,1) = 0
        END IF
        VNAME=VALUES_SOUGHT(I)
        iVNAME=lnblnk1(VNAME)
        IF (( ivname .LT. 1 )) THEN
          IF (( error_level .GT. 0 )) THEN
            write(ERR,*) ' ======================= Warning =============
     *======== '
            write(ERR,*) '    Empty VALUES_SOUGHT passt to Get_Inputs()!
     *         '
            write(ERR,*) ' =============================================
     *======== '
          END IF
          ERROR_FLAG=1
          ERROR_FLAGS(I)=1
          goto 1620
        END IF
        DO 1631 Kconvert=1,lnblnk1(vname)
          CURSOR=ICHAR(vname(Kconvert:Kconvert))
          IF (((CURSOR.GE.97).AND.(CURSOR.LE.122))) THEN
            CURSOR=CURSOR-32
            vname(Kconvert:Kconvert)=CHAR(CURSOR)
          END IF
1631    CONTINUE
1632    CONTINUE
        iindex = 0
        IF ((DELIMETER .EQ. 'NONE')) THEN
          start_found = .true.
        ELSE
          start_found = .false.
        END IF
1641    IF(iindex.NE.0)GO TO 1642
1650      CONTINUE
          LINE=LINE+1
          IF (( start_found )) THEN
            READ(UNITNUM,END=1660,ERR=1670,FMT='(A256)') TEXT
          ELSE
            READ(UNITNUM,END=1680,ERR=1670,FMT='(A256)') TEXT
          END IF
          length = len(text)
1691      IF(index(text,blank).NE.1)GO TO 1692
            IF (( length .GE. 2 )) THEN
              text=text(2:)
            ELSE
              GO TO1692
            END IF
            length = length - 1
          GO TO 1691
1692      CONTINUE
          ifound = INDEX(text,'#')
          IF (( ifound .GT. 1 )) THEN
            text = text(1:ifound-1)
          ELSE
            IF (( ifound .EQ. 1 )) THEN
              text = blank
            END IF
          END IF
          ifound = INDEX(text,';')
          IF (( ifound .GT. 1 )) THEN
            text = text(1:ifound-1)
          ELSE
            IF (( ifound .EQ. 1 )) THEN
              text = blank
            END IF
          END IF
          length = lnblnk1(TEXT)
          TEXT=TEXT(:length)
          origtext = text(:length)
          DO 1701 Kconvert=1,lnblnk1(text)
            CURSOR=ICHAR(text(Kconvert:Kconvert))
            IF (((CURSOR.GE.97).AND.(CURSOR.LE.122))) THEN
              CURSOR=CURSOR-32
              text(Kconvert:Kconvert)=CHAR(CURSOR)
            END IF
1701      CONTINUE
1702      CONTINUE
          IF (( .NOT.start_found )) THEN
            IF ((INDEX(TEXT,DELIM_START) .NE. 0 )) THEN
              start_found = .true.
            END IF
            goto 1650
          END IF
          iindex=INDEX(TEXT,VNAME(:iVNAME))
          IF (( DELIMETER.NE.'NONE' )) THEN
            IF ((INDEX(TEXT,DELIM_END).NE.0)) THEN
              IF (( error_level .GT. 0 )) THEN
                WRITE (ERR,*) '***************ERROR***************'
                WRITE (ERR,*) '>>',VALUES_SOUGHT(I)(:lnblnk1(VALUES_SOUG
     *          HT(I))), '<<',' NOT FOUND'
                WRITE (ERR,*) 'END OF DELIMETER: ',DELIMETER(:lnblnk1(DE
     *          LIMETER))
              END IF
              ERROR_FLAG=1
              ERROR_FLAGS(I)=1
              GOTO 1620
            END IF
          END IF
        GO TO 1641
1642    CONTINUE
        CHECK=0
        IF (( idebug )) THEN
          write(i_log,*) ' ******* Found: '
          write(i_log,'(a,$)') ' text:     '
          length = lnblnk1(text)
          IF (( length .GT. 0 )) THEN
            DO 1711 lll=1,length
              write(i_log,'(a1,$)') text(lll:lll)
1711        CONTINUE
1712        CONTINUE
            write(i_log,*)
          END IF
          write(i_log,'(a,$)') ' origtext: '
          length = lnblnk1(origtext)
          IF (( length .GT. 0 )) THEN
            DO 1721 lll=1,length
              write(i_log,'(a1,$)') origtext(lll:lll)
1721        CONTINUE
1722        CONTINUE
            write(i_log,*)
          END IF
        END IF
        IINDEX=IINDEX+iVNAME
        TEXT=TEXT(IINDEX:)
        origtext=origtext(iindex:)
        IF (( idebug )) THEN
          write(i_log,*) ' After removing vname: '
          write(i_log,'(a,$)') ' text:     '
          length = lnblnk1(text)
          IF (( length .GT. 0 )) THEN
            DO 1731 lll=1,length
              write(i_log,'(a1,$)') text(lll:lll)
1731        CONTINUE
1732        CONTINUE
            write(i_log,*)
          END IF
          write(i_log,'(a,$)') ' origtext: '
          length = lnblnk1(origtext)
          IF (( length .GT. 0 )) THEN
            DO 1741 lll=1,length
              write(i_log,'(a1,$)') origtext(lll:lll)
1741        CONTINUE
1742        CONTINUE
            write(i_log,*)
          END IF
        END IF
        IINDEX=INDEX(TEXT,'=')
        IF ((IINDEX.NE.0)) THEN
          TEXT=TEXT(IINDEX+1:)
          origtext=origtext(iindex+1:)
        ELSE
          IINDEX=INDEX(TEXT,':')
          IF ((IINDEX.NE.0)) THEN
            TEXT=TEXT(IINDEX+1:)
            origtext=origtext(iindex+1:)
          END IF
        END IF
        IF (( idebug )) THEN
          write(i_log,*) ' After removing leading equals: '
          write(i_log,'(a,$)') ' text:     '
          length = lnblnk1(text)
          IF (( length .GT. 0 )) THEN
            DO 1751 lll=1,length
              write(i_log,'(a1,$)') text(lll:lll)
1751        CONTINUE
1752        CONTINUE
            write(i_log,*)
          END IF
          write(i_log,'(a,$)') ' origtext: '
          length = lnblnk1(origtext)
          IF (( length .GT. 0 )) THEN
            DO 1761 lll=1,length
              write(i_log,'(a1,$)') origtext(lll:lll)
1761        CONTINUE
1762        CONTINUE
            write(i_log,*)
          END IF
        END IF
        IF (( (lnblnk1(TEXT).EQ.0) .OR. (lnblnk1(TEXT).EQ.1) )) THEN
          IF ((vname(:ivname).EQ.'TITLE')) THEN
            READ (UNITNUM,FMT='(A256)') TEXTPIECE
            IF ((lnblnk1(TEXTPIECE).NE.0)) THEN
              TEXT=TEXTPIECE(:lnblnk1(TEXTPIECE))
              length = len(text)
1771          IF(index(text,blank).NE.1)GO TO 1772
                IF (( length .GE. 2 )) THEN
                  text=text(2:)
                ELSE
                  GO TO1772
                END IF
                length = length - 1
              GO TO 1771
1772          CONTINUE
              length = len(origtext)
1781          IF(index(origtext,blank).NE.1)GO TO 1782
                IF (( length .GE. 2 )) THEN
                  origtext=origtext(2:)
                ELSE
                  GO TO1782
                END IF
                length = length - 1
              GO TO 1781
1782          CONTINUE
              GOTO 1790
            END IF
          END IF
          IF (( error_level .GT. 0 )) THEN
            WRITE (ERR,*) '*************ERROR*************'
            WRITE (ERR,*) 'VALUE SOUGHT: ',VALUES_SOUGHT(I)
            WRITE (ERR,*) 'VALUE NOT THERE!!'
          END IF
          ERROR_FLAG=1
          ERROR_FLAGS(I)=1
          RETURN
        END IF
1790    CONTINUE
        iindex = index(text,'DEFAULT')
        IF (( iindex .NE. 0 )) THEN
          IF (( type(i) .NE. 2 )) THEN
            IF (( type(i) .NE. 3 )) THEN
              VALUE(I,1)=DEFAULT(I)
            ELSE
              VALUE(I,1)=0
            END IF
            goto 1620
          END IF
        END IF
        IF (((TYPE(I) .EQ. 0).OR.(TYPE(I) .EQ. 1))) THEN
          IVAL=1
          IF (( idebug )) THEN
            write(i_log,*) ' *** Reading an integer or a real value! '
          END IF
1801      CONTINUE
            IF (( idebug )) THEN
              write(i_log,*) ' In LOOP, ival = ',ival
            END IF
            IF ((lnblnk1(TEXT).EQ.0)) THEN
              IF (( error_level .GT. 0 )) THEN
                WRITE(ERR,*) '*************ERROR*************'
                WRITE (ERR,*) 'VALUE SOUGHT: ',VALUES_SOUGHT(I)
                WRITE (ERR,*) 'VALUE NOT THERE!!'
              END IF
              ERROR_FLAG=1
              ERROR_FLAGS(I)=1
              RETURN
            END IF
            READ(TEXT,END=1810,ERR=1820,FMT=*) VALUE(I,IVAL)
            IF (( idebug )) THEN
              write(i_log,*) ' Read value: ',ival,VALUE(I,IVAL)
            END IF
            IF (((VALUE(I,IVAL).GT.VALUE_MAX(I)).OR.(VALUE(I,IVAL).LT.VA
     *      LUE_MIN(I)))) THEN
              IF ((TYPE(I).EQ.0)) THEN
                INT_VALUE=DEFAULT(I)
                IF (( error_level .GT. 0 )) THEN
                  WRITE(ERR,*) '************WARNING************'
                  WRITE(ERR,1830) INT_VALUE, VALUES_SOUGHT(I)(:lnblnk1(V
     *            ALUES_SOUGHT(I)))
                END IF
1830            FORMAT ( 'Default= ',I9,' used for: ', A )
                INT_VALUE=VALUE(I,IVAL)
                INT_VALUE_MIN=VALUE_MIN(I)
                INT_VALUE_MAX=VALUE_MAX(I)
                IF (( error_level .GT. 0 )) THEN
                  WRITE(ERR,1840) VALUES_SOUGHT(I)(:lnblnk1(VALUES_SOUGH
     *            T(I))), INT_VALUE, INT_VALUE_MIN,INT_VALUE_MAX
                END IF
1840            FORMAT (A,'=', I9,' should be between ', I9,' and ', I9)
              END IF
              IF ((TYPE(I).EQ.1)) THEN
                IF (( error_level .GT. 0 )) THEN
                  WRITE(ERR,*) '************WARNING************'
                  WRITE(ERR,1850) DEFAULT(I), VALUES_SOUGHT(I)(:lnblnk1(
     *            VALUES_SOUGHT(I)))
1850              FORMAT ( 'Default= ',F12.6,' used for: ', A )
                  WRITE(ERR,1860) VALUES_SOUGHT(I)(:lnblnk1(VALUES_SOUGH
     *            T(I))), VALUE(I,IVAL), VALUE_MIN(I),VALUE_MAX(I)
1860              FORMAT (A,'=', F12.6,' should be between ', G14.6,' an
     *d ', G14.6)
                END IF
              END IF
              VALUE(I,IVAL)=DEFAULT(I)
            END IF
            IF((IVAL .EQ. NVALUE(I)))GO TO1802
            IF (((INDEX(TEXT,',').NE.0).OR.(lnblnk1(TEXT).EQ.0))) THEN
              IF (( idebug )) THEN
                write(i_log,*) ' A comma or a blank text found -> '
                write(i_log,*) ' searching for further input'
              END IF
              TEXT=TEXT(INDEX(TEXT,',')+1:)
1871          IF(lnblnk1(TEXT).NE.0)GO TO 1872
                IF (( idebug )) THEN
                  write(i_log,*) ' Empty text -> reading next line! '
                END IF
                LINE=LINE+1
                READ (UNITNUM,END=1810,ERR=1820,FMT='(A256)') TEXT
                length = len(text)
1881            IF(index(text,blank).NE.1)GO TO 1882
                  IF (( length .GE. 2 )) THEN
                    text=text(2:)
                  ELSE
                    GO TO1882
                  END IF
                  length = length - 1
                GO TO 1881
1882            CONTINUE
                ifound = INDEX(text,'#')
                IF (( ifound .GT. 1 )) THEN
                  text = text(1:ifound-1)
                ELSE
                  IF (( ifound .EQ. 1 )) THEN
                    text = blank
                  END IF
                END IF
                ifound = INDEX(text,';')
                IF (( ifound .GT. 1 )) THEN
                  text = text(1:ifound-1)
                ELSE
                  IF (( ifound .EQ. 1 )) THEN
                    text = blank
                  END IF
                END IF
                length = lnblnk1(TEXT)
                TEXT=TEXT(:length)
                origtext = text(:length)
                DO 1891 Kconvert=1,lnblnk1(text)
                  CURSOR=ICHAR(text(Kconvert:Kconvert))
                  IF (((CURSOR.GE.97).AND.(CURSOR.LE.122))) THEN
                    CURSOR=CURSOR-32
                    text(Kconvert:Kconvert)=CHAR(CURSOR)
                  END IF
1891            CONTINUE
1892            CONTINUE
                DO 1901 K=1,NMAX
                  vname1 = VALUES_SOUGHT(K)
                  length = lnblnk1(vname1)
                  IF (( length .GT. 0 )) THEN
                    length = len(vname1)
1911                IF(index(vname1,blank).NE.1)GO TO 1912
                      IF (( length .GE. 2 )) THEN
                        vname1=vname1(2:)
                      ELSE
                        GO TO1912
                      END IF
                      length = length - 1
                    GO TO 1911
1912                CONTINUE
                    DO 1921 Kconvert=1,lnblnk1(vname1)
                      CURSOR=ICHAR(vname1(Kconvert:Kconvert))
                      IF (((CURSOR.GE.97).AND.(CURSOR.LE.122))) THEN
                        CURSOR=CURSOR-32
                        vname1(Kconvert:Kconvert)=CHAR(CURSOR)
                      END IF
1921                CONTINUE
1922                CONTINUE
                    IF ((INDEX(TEXT,vname1(:length)).NE.0)) THEN
                      IF (( error_level .GT. 0 )) THEN
                        WRITE(ERR,*) '************ERROR************'
                        WRITE(ERR,*) 'VALUE SOUGHT: ',VALUES_SOUGHT(I)
                        WRITE(ERR,*) KEEPTEXT(:lnblnk1(KEEPTEXT)), '<--C
     *OMMA INDICATES ANOTHER INPUT'
                        WRITE(ERR,*) 'SEARCHED NEXT LINE: ', TEXT(:lnbln
     *                  k1(TEXT))
                        WRITE(ERR,*) 'BUT NO OTHER INPUT WAS DETECTED'
                      END IF
                      ERROR_FLAG=1
                      ERROR_FLAGS(I)=1
                    END IF
                  END IF
1901            CONTINUE
1902            CONTINUE
                IF (( idebug )) THEN
                  write(i_log,*) ' Next line: '
                  write(i_log,'(a,$)') ' text:     '
                  length = lnblnk1(text)
                  IF (( length .GT. 0 )) THEN
                    DO 1931 lll=1,length
                      write(i_log,'(a1,$)') text(lll:lll)
1931                CONTINUE
1932                CONTINUE
                    write(i_log,*)
                  END IF
                  write(i_log,'(a,$)') ' origtext: '
                  length = lnblnk1(origtext)
                  IF (( length .GT. 0 )) THEN
                    DO 1941 lll=1,length
                      write(i_log,'(a1,$)') origtext(lll:lll)
1941                CONTINUE
1942                CONTINUE
                    write(i_log,*)
                  END IF
                END IF
              GO TO 1871
1872          CONTINUE
            ELSE
              GO TO1802
            END IF
            IVAL=IVAL+1
          GO TO 1801
1802      CONTINUE
          IF (((NVALUE(I).NE.0).AND.(NVALUE(I).NE.IVAL))) THEN
            IF (( error_level .GT. 0 )) THEN
              WRITE (ERR,*) '**************ERROR**************'
              WRITE (ERR,*) 'VALUE SOUGHT: ', VALUES_SOUGHT(I)
              WRITE (ERR,*) 'ASKED FOR', NVALUE(I),' NUMERICAL INPUT(S)'
              WRITE (ERR,*) 'HOWEVER,', IVAL, ' WERE DETECTED'
            END IF
            ERROR_FLAG=1
            ERROR_FLAGS(I)=1
          ELSE
            NVALUE(I)=IVAL
          END IF
1810      CONTINUE
        END IF
        IF (((TYPE(I) .EQ. 2) .OR. (TYPE(I) .EQ. 3))) THEN
          IVAL=1
          IF (( idebug )) THEN
            write(i_log,*) ' Trying to read a string! '
          END IF
1951      CONTINUE
            IF (( idebug )) THEN
              write(i_log,*) ' In LOOP, ival = ',ival
            END IF
            IF ((lnblnk1(TEXT).EQ.0)) THEN
              IF (( error_level .GT. 0 )) THEN
                WRITE(ERR,*) '*************ERROR*************'
                WRITE (ERR,*) 'VALUE SOUGHT: ',VALUES_SOUGHT(I)
                WRITE (ERR,*) 'VALUE NOT THERE!!'
              END IF
              ERROR_FLAG=1
              ERROR_FLAGS(I)=1
              RETURN
            END IF
            IF ((vname(:ivname).EQ.'TITLE')) THEN
              TEXTPIECE=origtext
              GOTO 1960
            END IF
            iindex = INDEX(origtext,',')
            IF (( iindex .NE. 0 )) THEN
              TEXTPIECE=origtext(:iindex-1)
            ELSE
              TEXTPIECE=origtext
            END IF
1960        CONTINUE
            READ(TEXTPIECE,ERR=1970,FMT='(A256)') CHAR_VALUE(I,IVAL)
            length = len(CHAR_VALUE(I,IVAL))
1981        IF(index(CHAR_VALUE(I,IVAL),blank).NE.1)GO TO 1982
              IF (( length .GE. 2 )) THEN
                CHAR_VALUE(I,IVAL)=CHAR_VALUE(I,IVAL)(2:)
              ELSE
                GO TO1982
              END IF
              length = length - 1
            GO TO 1981
1982        CONTINUE
            IF (( idebug )) THEN
              write(i_log,*) ' Read the following char string: '
              length = lnblnk1(CHAR_VALUE(I,IVAL))
              IF (( length .GT. 0 )) THEN
                DO 1991 lll=1,length
                  write(i_log,'(a1,$)') CHAR_VALUE(I,IVAL)(lll:lll)
1991            CONTINUE
1992            CONTINUE
                write(i_log,*)
              END IF
            END IF
            IF ((TYPE(I) .EQ. 3)) THEN
              DO 2001 Kconvert=1,lnblnk1(CHAR_VALUE(I,IVAL))
                CURSOR=ICHAR(CHAR_VALUE(I,IVAL)(Kconvert:Kconvert))
                IF (((CURSOR.GE.97).AND.(CURSOR.LE.122))) THEN
                  CURSOR=CURSOR-32
                  CHAR_VALUE(I,IVAL)(Kconvert:Kconvert)=CHAR(CURSOR)
                END IF
2001          CONTINUE
2002          CONTINUE
              ALLOWED=.FALSE.
              DO 2011 K=0,5
                vname1 = ALLOWED_INPUTS(I,K)
                length = len(ALLOWED_INPUTS(I,K))
2021            IF(index(ALLOWED_INPUTS(I,K),blank).NE.1)GO TO 2022
                  IF (( length .GE. 2 )) THEN
                    ALLOWED_INPUTS(I,K)=ALLOWED_INPUTS(I,K)(2:)
                  ELSE
                    GO TO2022
                  END IF
                  length = length - 1
                GO TO 2021
2022            CONTINUE
                DO 2031 Kconvert=1,lnblnk1(ALLOWED_INPUTS(I,K))
                  CURSOR=ICHAR(ALLOWED_INPUTS(I,K)(Kconvert:Kconvert))
                  IF (((CURSOR.GE.97).AND.(CURSOR.LE.122))) THEN
                    CURSOR=CURSOR-32
                    ALLOWED_INPUTS(I,K)(Kconvert:Kconvert)=CHAR(CURSOR)
                  END IF
2031            CONTINUE
2032            CONTINUE
                IF ((ALLOWED_INPUTS(I,K).EQ.CHAR_VALUE(I,IVAL))) THEN
                  ALLOWED=.TRUE.
                  VALUE(I,IVAL)=K
                  IF (( idebug )) THEN
                    write(i_log,*) ' Found a allowed_value match ',k
                  END IF
                END IF
2011          CONTINUE
2012          CONTINUE
              IF ((.NOT.ALLOWED)) THEN
                WRITE(ERR,*) '*************ERROR*************'
                IF ((IVAL.NE.1)) THEN
                  WRITE (ERR,*) 'VALUE SOUGHT: ',VALUES_SOUGHT(I)
                  WRITE (ERR,*) 'SHOULD HAVE ONE INPUT ONLY'
                  WRITE (ERR,*) 'APPARENT STATE: COMMA INDICATING SECOND
     * VALUE'
                ELSE
                  WRITE (ERR,*) 'VALUE SOUGHT: ',VALUES_SOUGHT(I)
                  WRITE(ERR,*) 'INPUT-->', CHAR_VALUE(I,IVAL)(:lnblnk1(C
     *            HAR_VALUE(I,IVAL))), '<--NOT ALLOWED'
                  WRITE(ERR,*) 'OPTIONS ARE:'
                  WRITE(ERR,2040) (ALLOWED_INPUTS(I,K)(:lnblnk1(ALLOWED_
     *            INPUTS(I,K))),K=0,5)
                END IF
2040            FORMAT(A40)
                ERROR_FLAG=1
                ERROR_FLAGS(I)=1
              END IF
            END IF
            IF ((vname(:ivname).EQ.'TITLE')) THEN
              GO TO1952
            END IF
            DO 2051 K=1,LEN(KEEPTEXT)
              KEEPTEXT(K:K)=' '
2051        CONTINUE
2052        CONTINUE
            KEEPTEXT(:lnblnk1(TEXT))=TEXT
            iindex = INDEX(TEXT,',')
            IF (( iindex .NE. 0 .OR. lnblnk1(TEXT).EQ.0 )) THEN
              TEXT=TEXT(INDEX(TEXT,',')+1:)
              origtext=origtext(iindex+1:)
2061          IF(lnblnk1(TEXT).NE.0)GO TO 2062
                LINE=LINE+1
                READ (UNITNUM,ERR=1970,FMT='(A256)') TEXT
                length = len(text)
2071            IF(index(text,blank).NE.1)GO TO 2072
                  IF (( length .GE. 2 )) THEN
                    text=text(2:)
                  ELSE
                    GO TO2072
                  END IF
                  length = length - 1
                GO TO 2071
2072            CONTINUE
                ifound = INDEX(text,'#')
                IF (( ifound .GT. 1 )) THEN
                  text = text(1:ifound-1)
                ELSE
                  IF (( ifound .EQ. 1 )) THEN
                    text = blank
                  END IF
                END IF
                ifound = INDEX(text,';')
                IF (( ifound .GT. 1 )) THEN
                  text = text(1:ifound-1)
                ELSE
                  IF (( ifound .EQ. 1 )) THEN
                    text = blank
                  END IF
                END IF
                length = lnblnk1(TEXT)
                TEXT=TEXT(:length)
                origtext = text(:length)
                DO 2081 Kconvert=1,lnblnk1(text)
                  CURSOR=ICHAR(text(Kconvert:Kconvert))
                  IF (((CURSOR.GE.97).AND.(CURSOR.LE.122))) THEN
                    CURSOR=CURSOR-32
                    text(Kconvert:Kconvert)=CHAR(CURSOR)
                  END IF
2081            CONTINUE
2082            CONTINUE
                DO 2091 K=1,NMAX
                  vname1 = VALUES_SOUGHT(K)
                  length = lnblnk1(vname1)
                  IF (( length .GT. 0 )) THEN
                    length = len(vname1)
2101                IF(index(vname1,blank).NE.1)GO TO 2102
                      IF (( length .GE. 2 )) THEN
                        vname1=vname1(2:)
                      ELSE
                        GO TO2102
                      END IF
                      length = length - 1
                    GO TO 2101
2102                CONTINUE
                    DO 2111 Kconvert=1,lnblnk1(vname1)
                      CURSOR=ICHAR(vname1(Kconvert:Kconvert))
                      IF (((CURSOR.GE.97).AND.(CURSOR.LE.122))) THEN
                        CURSOR=CURSOR-32
                        vname1(Kconvert:Kconvert)=CHAR(CURSOR)
                      END IF
2111                CONTINUE
2112                CONTINUE
                    IF ((INDEX(TEXT,vname1(:length)).NE.0)) THEN
                      WRITE(ERR,*) '************ERROR************'
                      WRITE(ERR,*) 'VALUE SOUGHT: ',VALUES_SOUGHT(I)
                      WRITE(ERR,*) KEEPTEXT(:lnblnk1(KEEPTEXT)), '<--COM
     *MA INDICATES ANOTHER INPUT'
                      WRITE(ERR,*) 'SEARCHED NEXT LINE: ', TEXT(:lnblnk1
     *                (TEXT))
                      WRITE(ERR,*) 'BUT NO OTHER INPUT WAS DETECTED'
                      ERROR_FLAG=1
                      ERROR_FLAGS(I)=1
                    END IF
                  END IF
2091            CONTINUE
2092            CONTINUE
              GO TO 2061
2062          CONTINUE
            ELSE
              GO TO1952
            END IF
            IVAL=IVAL+1
          GO TO 1951
1952      CONTINUE
          IF (((NVALUE(I).NE.0).AND.(NVALUE(I).NE.IVAL))) THEN
            IF (( error_level .GT. 0 )) THEN
              WRITE (ERR,*) '*******************ERROR*******************
     *'
              WRITE (ERR,*) 'VALUE SOUGHT: ', VALUES_SOUGHT(I)
              WRITE (ERR,*) 'ASKED FOR', NVALUE(I),' INPUT(S)'
              WRITE (ERR,*) 'HOWEVER,', IVAL, ' WERE DETECTED'
            END IF
            ERROR_FLAG=1
            ERROR_FLAGS(I)=1
          ELSE
            NVALUE(I)=IVAL
          END IF
        END IF
        goto 1620
1660    IF (( error_level .GT. 0 )) THEN
          WRITE (ERR,*) '******************ERROR***********************'
          WRITE (ERR,*) 'END OF FILE REACHED BUT VALUE SOUGHT NOT FOUND'
          WRITE (ERR,*) 'PROBABLY A MISSING/MISSPELLED END DELIMETER'
          WRITE (ERR,*) 'VALUE SOUGHT: >>', VALUES_SOUGHT(I)(:lnblnk1(VA
     *    LUES_SOUGHT(I))),'<<'
          WRITE (ERR,*) 'END DELIMETER: >>', DELIM_END(:lnblnk1(DELIM_EN
     *    D)),'<<'
        END IF
        ERROR_FLAG=1
        ERROR_FLAGS(I)=1
        goto 1620
1680    IF (( error_level .GT. 0 )) THEN
          WRITE (ERR,*) '******************ERROR***********************'
          WRITE (ERR,*) 'END OF FILE REACHED BUT VALUE SOUGHT NOT FOUND'
          WRITE (ERR,*) 'PROBABLY A MISSING/MISSPELLED START DELIMETER'
          WRITE (ERR,*) 'VALUE SOUGHT: >>', VALUES_SOUGHT(I)(:lnblnk1(VA
     *    LUES_SOUGHT(I))),'<<'
          WRITE (ERR,*) 'START DELIMETER: >>', DELIM_START(:lnblnk1(DELI
     *    M_START)),'<<'
        END IF
        ERROR_FLAG=1
        ERROR_FLAGS(I)=1
        goto 1620
1820    IF (( error_level .GT. 0 )) THEN
          WRITE (ERR,*) '***************ERROR***************'
          IF ((IVAL.GT.1)) THEN
            J=IVAL
          ELSE
            J=1
          END IF
          WRITE (ERR,*) 'ERROR READING VALUE SOUGHT: ', VALUES_SOUGHT(I)
          WRITE (ERR,*) 'LINE #',LINE
          WRITE (ERR,*) 'COULD NOT READ THE VALUE!!'
          WRITE (ERR,*) 'SHOULD BE AN INTEGER OR A REAL...'
          WRITE (ERR,*) 'IS THERE AN EXTRA COMMA AT THE END OF YOUR INPU
     *T?'
        END IF
        ERROR_FLAG=1
        ERROR_FLAGS(I)=1
        GOTO 1620
1970    IF (( error_level .GT. 0 )) THEN
          WRITE (ERR,*) '***************ERROR***************'
          WRITE (ERR,*) 'ERROR READING VALUE SOUGHT: ', VALUES_SOUGHT(I)
          WRITE (ERR,*) 'LINE #',LINE
          WRITE (ERR,*) 'COULD NOT READ THE STRING !!'
        END IF
        ERROR_FLAG=1
        ERROR_FLAGS(I)=1
1620    CONTINUE
1611  CONTINUE
1612  CONTINUE
      RETURN
1670  WRITE (ERR,*) '***************ERROR***************'
      WRITE (ERR,*) 'ERROR READING TEXT ', TEXT,' ON LINE ',LINE
      goto 2120
2120  CONTINUE
      ERROR_FLAG=1
      ERROR_FLAGS(I)=1
      RETURN
      entry get_input_set_error_level(the_level)
      error_level = the_level
      return
      END
      subroutine get_transport_parameter(ounit)
      implicit none
      character*80 line
      character*512 toUpper
      integer*4 ounit
      COMMON/GetInput/ ALLOWED_INPUTS(100,0:5),   VALUES_SOUGHT(100),  C
     *HAR_VALUE(100,100),  VALUE(100,100),  DEFAULT(100),  VALUE_MIN(100
     *),  VALUE_MAX(100),  NVALUE(100),  TYPE(100),      ERROR_FLAGS(100
     *),   i_errors,  NMIN, NMAX,   ERROR_FLAG,  DELIMETER
      character ALLOWED_INPUTS*64,VALUES_SOUGHT*64, CHAR_VALUE*256,DELIM
     *ETER*64
      real*8 VALUE,DEFAULT,VALUE_MIN,VALUE_MAX
      integer*4 NVALUE,TYPE,NMIN,NMAX,ERROR_FLAG,ERROR_FLAGS,i_errors
      COMMON/BOUNDS/ECUT(1),PCUT(1),VACDST
      real*8 ECUT,  PCUT,  VACDST
      common/ET_control/ smaxir(1),estepe,ximax,  skindepth_for_bca,tran
     *sport_algorithm, bca_algorithm,exact_bca,spin_effects
      real*8 smaxir,  estepe,  ximax,      skindepth_for_bca
      integer*4 transport_algorithm, bca_algorithm
      logical exact_bca,  spin_effects
      COMMON/EDGE/binding_energies(30,100), interaction_prob(6,100), rel
     *axation_prob(39,100), edge_energies(16,100), edge_number(100), edg
     *e_a(16,100), edge_b(16,100), edge_c(16,100), edge_d(16,100), IEDGF
     *L(1),IPHTER(1)
      real*8 binding_energies,  interaction_prob,    relaxation_prob,  e
     *dge_energies,  edge_a,edge_b,edge_c,edge_d
      integer*2 IEDGFL,  IPHTER
      integer*4 edge_number
      common/compton_data/ iz_array(1538),  be_array(1538),  Jo_array(15
     *38),  erfJo_array(1538),   ne_array(1538),  shn_array(1538),
     *shell_array(200,1), eno_array(200,1), eno_atbin_array(200,1), n_sh
     *ell(1), radc_flag,  ibcmp(1)
      integer*4 iz_array,ne_array,shn_array,eno_atbin_array, shell_array
     *,n_shell,radc_flag
      real*8 be_array,Jo_array,erfJo_array,eno_array
      integer*2 ibcmp
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      COMMON/MISC/  DUNIT,KMPI,KMPO,RHOR(1),MED(1),IRAYLR(1),IPHOTONUCR(
     *1)
      real*8 DUNIT,  RHOR
      integer*4 KMPI,  KMPO
      integer*2 MED,  IRAYLR,  IPHOTONUCR
      COMMON/BREMPR/ DL1(8,1),DL2(8,1),DL3(8,1),DL4(8,1),DL5(8,1),DL6(8,
     *1), ALPHI(2,1),BPAR(2,1),DELPOS(2,1), WA(1,50),PZ(1,50),ZELEM(1,50
     *),RHOZ(1,50), PWR2I(50), DELCM(1),ZBRANG(1),LZBRANG(1),NNE(1), IBR
     *DST,IPRDST,ibr_nist,pair_nrc,itriplet, ASYM(1,50,2)
      CHARACTER*4 ASYM
      real*8 DL1,DL2,DL3,DL4,DL5,DL6,   ALPHI,  BPAR,  DELPOS,  WA,  PZ,
     *  ZELEM,  RHOZ,  PWR2I,  DELCM,  ZBRANG,  LZBRANG
      integer*4 NNE,  IBRDST,  IPRDST,  ibr_nist,  itriplet,  pair_nrc
      common/eii_data/ eii_xsection_a( 10000),  eii_xsection_b( 10000),
     * eii_cons(1), eii_a(40),  eii_b(40),  eii_L_factor,  eii_z(40),  e
     *ii_sh(40),  eii_nshells(100),  eii_nsh(1),  eii_first(1,50),  eii_
     *no(1,50),  eii_flag
      real*8 eii_xsection_a,eii_xsection_b,eii_a,eii_b,eii_cons,eii_L_fa
     *ctor
      integer*4 eii_z,eii_sh,eii_nshells
      integer*4 eii_first,eii_no
      integer*4 eii_elements,eii_flag,eii_nsh
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      COMMON/rayleigh_inputs/iray_ff_media(1),iray_ff_file(1)
      character*24 iray_ff_media
      character*128 iray_ff_file
      common/emf_inputs/ExIN,EyIN,EzIN,  EMLMTIN,  BxIN, ByIN, BzIN,  Bx
     *, By, Bz,  Bx_new, By_new, Bz_new,  emfield_on
      real*8 ExIN,EyIN,EzIN, EMLMTIN, BxIN,ByIN,BzIN, Bx,By,Bz, Bx_new,B
     *y_new,Bz_new
      logical emfield_on
      common/x_options/eadl_relax,  mcdf_pe_xsections
      logical eadl_relax, mcdf_pe_xsections
      integer*4 ival,num_ecut,num_pcut,num_smax,num_incoh,num_radc,num_c
     *oh,num_relax, num_pe_ang,num_brems_ang,num_brems_cs,num_pair_cs, n
     *um_ffmed,num_ffiles, num_pair_ang,num_eii,num_eii_L,num_estepe,num
     *_ximax,num_triplet, num_pxsec,num_pxsec_out, num_cxsec, num_photon
     *uc, num_photonuc_xsec, num_efield, num_bfield, num_emlmt, num_spin
     *,num_bca,num_alg,num_skin,itmp,iitmp,i,j,k,istart,iend, egs_open_f
     *ile,lnblnk1
      logical ecut_inregions,pcut_inregions,smax_inregions, incoh_inregi
     *ons,coh_inregions,relax_inregions, pe_inregions,aux_inregions,phot
     *onuc_inregions
      character*15 output_strings(14)
      save output_strings,line
      save ecut_inregions,pcut_inregions,smax_inregions, incoh_inregions
     *,coh_inregions,relax_inregions, pe_inregions,aux_inregions,photonu
     *c_inregions, num_photonuc
      DO 2131 k=1,80
        line(k:k) = '='
2131  CONTINUE
2132  CONTINUE
      delimeter = 'MC TRANSPORT PARAMETER'
      ival = 0
      ecut_inregions=.false.
      pcut_inregions=.false.
      smax_inregions=.false.
      incoh_inregions=.false.
      coh_inregions=.false.
      relax_inregions=.false.
      pe_inregions=.false.
      aux_inregions=.false.
      photonuc_inregions=.false.
      i_errors=15
      i_errors=egs_open_file(i_errors,0,1,'.errors')
      write(i_errors,*) ' If you are not trying to reset transport param
     *eters, '
      write(i_errors,*) ' ignore all the output until the message '
      write(i_errors,*) ' ******************** end input transport param
     *eter *********************** '
      write(i_errors,*)
      ival = ival + 1
      num_ecut = ival
      values_sought(ival) = 'Global ECUT'
      nvalue(ival) = 1
      type(ival) = 1
      value_min(ival) = 0
      value_max(ival) = 1e15
      default(ival) = 0.
      ival = ival + 1
      num_pcut = ival
      values_sought(ival) = 'Global PCUT'
      nvalue(ival) = 1
      type(ival) = 1
      value_min(ival) = 0
      value_max(ival) = 1e15
      default(ival) = 0.
      ival = ival + 1
      num_smax = ival
      values_sought(ival) = 'Global SMAX'
      nvalue(ival) = 1
      type(ival) = 1
      value_min(ival) = 0
      value_max(ival) = 1e15
      default(ival) = 1e10
      ival = ival + 1
      num_incoh = ival
      values_sought(ival) = 'Bound Compton scattering'
      nvalue(ival) = 1
      type(ival) = 3
      allowed_inputs(ival,0) = 'Off'
      allowed_inputs(ival,1) = 'On'
      allowed_inputs(ival,2) = 'On in Regions'
      allowed_inputs(ival,3) = 'Off in Regions'
      allowed_inputs(ival,4) = 'Simple'
      allowed_inputs(ival,5) = 'norej'
      ival = ival + 1
      num_radc = ival
      values_sought(ival) = 'Radiative Compton corrections'
      nvalue(ival) = 1
      type(ival) = 3
      allowed_inputs(ival,0) = 'Off'
      allowed_inputs(ival,1) = 'On'
      ival = ival + 1
      num_coh = ival
      values_sought(ival) = 'Rayleigh scattering'
      nvalue(ival) = 1
      type(ival) = 3
      allowed_inputs(ival,0) = 'Off'
      allowed_inputs(ival,1) = 'On'
      allowed_inputs(ival,2) = 'On in Regions'
      allowed_inputs(ival,3) = 'Off in Regions'
      allowed_inputs(ival,4) = 'custom'
      ival = ival + 1
      num_relax = ival
      values_sought(ival) = 'Atomic relaxations'
      nvalue(ival) = 1
      type(ival) = 3
      allowed_inputs(ival,0) = 'Off'
      allowed_inputs(ival,1) = 'On'
      allowed_inputs(ival,2) = 'On in Regions'
      allowed_inputs(ival,3) = 'Off in Regions'
      allowed_inputs(ival,4) = 'eadl'
      allowed_inputs(ival,5) = 'simple'
      ival = ival + 1
      num_pe_ang = ival
      values_sought(ival) = 'Photoelectron angular sampling'
      nvalue(ival) = 1
      type(ival) = 3
      allowed_inputs(ival,0) = 'Off'
      allowed_inputs(ival,1) = 'On'
      allowed_inputs(ival,2) = 'On in Regions'
      allowed_inputs(ival,3) = 'Off in Regions'
      ival = ival + 1
      num_brems_ang = ival
      values_sought(ival) = 'Brems angular sampling'
      nvalue(ival) = 1
      type(ival) = 3
      allowed_inputs(ival,0) = 'Simple'
      allowed_inputs(ival,1) = 'KM'
      ival = ival + 1
      num_brems_cs = ival
      values_sought(ival) = 'Brems cross sections'
      nvalue(ival) = 1
      type(ival) = 3
      allowed_inputs(ival,0) = 'BH'
      allowed_inputs(ival,1) = 'NIST'
      allowed_inputs(ival,2) = 'NRC'
      ival = ival + 1
      num_pair_ang = ival
      values_sought(ival) = 'Pair angular sampling'
      nvalue(ival) = 1
      type(ival) = 3
      allowed_inputs(ival,0) = 'Off'
      allowed_inputs(ival,1) = 'Simple'
      allowed_inputs(ival,2) = 'KM'
      allowed_inputs(ival,3) = 'Uniform'
      allowed_inputs(ival,4) = 'Blend'
      ival = ival + 1
      num_pair_cs = ival
      values_sought(ival) = 'Pair cross sections'
      nvalue(ival) = 1
      type(ival) = 3
      allowed_inputs(ival,0) = 'BH'
      allowed_inputs(ival,1) = 'NRC'
      ival = ival + 1
      num_triplet = ival
      values_sought(ival) = 'Triplet production'
      nvalue(ival) = 1
      type(ival) = 3
      allowed_inputs(ival,0) = 'Off'
      allowed_inputs(ival,1) = 'On'
      ival = ival + 1
      num_spin = ival
      values_sought(ival) = 'Spin effects'
      nvalue(ival) = 1
      type(ival) = 3
      allowed_inputs(ival,0) = 'Off'
      allowed_inputs(ival,1) = 'On'
      ival = ival + 1
      num_eii = ival
      values_sought(ival) = 'Electron Impact Ionization'
      nvalue(ival) = 1
      type(ival) = 2
      ival = ival + 1
      num_eii_L= ival
      values_sought(ival) = 'scale L EII cross-sections'
      nvalue(ival) = 1
      type(ival) = 1
      value_min(ival) = 0.0
      value_max(ival) = 1.0e+9
      default(ival) = 1.0
      ival = ival + 1
      num_estepe = ival
      values_sought(ival) = 'ESTEPE'
      nvalue(ival) = 1
      type(ival) = 1
      value_min(ival) = 1e-5
      value_max(ival) = 1
      default(ival) = 0.25
      ival = ival + 1
      num_ximax = ival
      values_sought(ival) = 'XImax'
      nvalue(ival) = 1
      type(ival) = 1
      value_min(ival) = 0
      value_max(ival) = 1
      default(ival) = 0.5
      ival = ival + 1
      num_bca = ival
      values_sought(ival) = 'Boundary crossing algorithm'
      nvalue(ival) = 1
      type(ival) = 3
      allowed_inputs(ival,0) = 'Exact'
      allowed_inputs(ival,1) = 'PRESTA-I'
      ival = ival + 1
      num_skin = ival
      values_sought(ival) = 'Skin depth for BCA'
      nvalue(ival) = 1
      type(ival) = 1
      value_min(ival) = -1
      value_max(ival) = 1e15
      default(ival) = 3
      ival = ival + 1
      num_alg = ival
      values_sought(ival) = 'Electron-step algorithm'
      nvalue(ival) = 1
      type(ival) = 3
      allowed_inputs(ival,0) = 'PRESTA-II'
      allowed_inputs(ival,1) = 'PRESTA-I'
      ival = ival + 1
      num_pxsec = ival
      values_sought(ival) = 'Photon cross sections'
      nvalue(ival) = 1
      type(ival) = 2
      ival = ival + 1
      num_pxsec_out = ival
      values_sought(ival) = 'Photon cross-sections output'
      nvalue(ival) = 1
      type(ival) = 3
      allowed_inputs(ival,0) = 'Off'
      allowed_inputs(ival,1) = 'On'
      ival = ival + 1
      num_cxsec = ival
      values_sought(ival) = 'Compton cross sections'
      nvalue(ival) = 1
      type(ival) = 2
      ival = ival + 1
      num_efield = ival
      values_sought(ival) = 'Electric Field'
      nvalue(ival) = 3
      type(ival) = 1
      value_min(ival) = -1e15
      value_max(ival) = 1e15
      default(ival) = 0
      ival = ival + 1
      num_bfield = ival
      values_sought(ival) = 'Magnetic Field'
      nvalue(ival) = 3
      type(ival) = 1
      value_min(ival) = -1e10
      value_max(ival) = 1e10
      default(ival) = 0
      ival = ival + 1
      num_emlmt = ival
      values_sought(ival) = 'EM ESTEPE'
      nvalue(ival) = 1
      type(ival) = 1
      value_min(ival) = 0.0
      value_max(ival) = 1.0
      default(ival) = 0.02
      ival = ival + 1
      num_photonuc = ival
      values_sought(ival) = 'Photonuclear attenuation'
      nvalue(ival) = 1
      type(ival) = 3
      allowed_inputs(ival,0) = 'Off'
      allowed_inputs(ival,1) = 'On'
      allowed_inputs(ival,2) = 'On in Regions'
      allowed_inputs(ival,3) = 'Off in Regions'
      ival = ival + 1
      num_photonuc_xsec = ival
      values_sought(ival) = 'Photonuclear cross sections'
      nvalue(ival) = 1
      type(ival) = 2
      Nmin = num_ecut
      Nmax = num_photonuc_xsec
      CALL GET_INPUT
      IF (( error_flags(num_ecut) .EQ. 0 )) THEN
        DO 2141 j=1,1
          ecut(j) = value(num_ecut,1)
2141    CONTINUE
2142    CONTINUE
      END IF
      IF (( error_flags(num_pcut) .EQ. 0 )) THEN
        DO 2151 j=1,1
          pcut(j) = value(num_pcut,1)
2151    CONTINUE
2152    CONTINUE
      END IF
      IF (( error_flags(num_smax) .EQ. 0 )) THEN
        DO 2161 j=1,1
          smaxir(j) = value(num_smax,1)
2161    CONTINUE
2162    CONTINUE
      END IF
      IF (( error_flags(num_brems_ang) .EQ. 0 )) THEN
        ibrdst = value(num_brems_ang,1)
      END IF
      IF (( error_flags(num_brems_cs) .EQ. 0 )) THEN
        ibr_nist = value(num_brems_cs,1)
      END IF
      IF (( error_flags(num_radc) .EQ. 0 )) THEN
        radc_flag = value(num_radc,1)
      END IF
      IF (( error_flags(num_pair_ang) .EQ. 0 )) THEN
        iprdst = value(num_pair_ang,1)
      END IF
      IF (( error_flags(num_pair_cs) .EQ. 0 )) THEN
        pair_nrc = value(num_pair_cs,1)
      END IF
      IF (( error_flags(num_triplet) .EQ. 0 )) THEN
        itriplet = value(num_triplet,1)
      END IF
      IF (( error_flags(num_eii_L) .EQ. 0 )) THEN
        eii_L_factor = value(num_eii_L,1)
      END IF
      IF (( error_flags(num_estepe) .EQ. 0 )) THEN
        estepe = value(num_estepe,1)
      END IF
      IF (( error_flags(num_ximax) .EQ. 0 )) THEN
        ximax = value(num_ximax,1)
      END IF
      IF (( error_flags(num_bca) .EQ. 0 )) THEN
        bca_algorithm = value(num_bca,1)
        IF (( bca_algorithm .EQ. 0 )) THEN
          exact_bca = .true.
        END IF
      END IF
      IF (( error_flags(num_alg) .EQ. 0 )) THEN
        transport_algorithm = value(num_alg,1)
      END IF
      IF (( error_flags(num_skin) .EQ. 0 )) THEN
        skindepth_for_bca = value(num_skin,1)
      END IF
      IF (( error_flags(num_spin) .EQ. 0 )) THEN
        itmp = value(num_spin,1)
        IF (( itmp .EQ. 1 )) THEN
          spin_effects = .true.
        ELSE
          spin_effects = .false.
        END IF
      END IF
      IF (( error_flags(num_eii) .EQ. 0 )) THEN
        eii_xfile = char_value(num_eii,1)
        eii_flag=1
        IF ((toUpper(eii_xfile(:lnblnk1(eii_xfile))).eq.'ON' .OR. toUppe
     *  r(eii_xfile(:lnblnk1(eii_xfile))).eq.'IK' )) THEN
          eii_xfile = 'ik'
          write(i_log,*) '==> Using default EII data compilation ', eii_
     *    xfile(:lnblnk1(eii_xfile))
        ELSE IF((toUpper(eii_xfile(:lnblnk1(eii_xfile))).eq.'OFF')) THEN
          eii_xfile='Off'
          eii_flag=0
        ELSE
          write(i_log,'(/a)') '***************** Warning: '
          write(i_log,*) '==> Using non-default EII data compilation ',
     *    eii_xfile(:lnblnk1(eii_xfile))
        END IF
      END IF
      IF (( error_flags(num_pxsec) .EQ. 0 )) THEN
        photon_xsections = char_value(num_pxsec,1)
        IF (( toUpper( photon_xsections(:lnblnk1(photon_xsections)) ) .E
     *  Q. 'MCDF-XCOM' )) THEN
          mcdf_pe_xsections = .true.
          photon_xsections = 'xcom'
        ELSE IF(( toUpper( photon_xsections(:lnblnk1(photon_xsections))
     *  ) .EQ. 'MCDF-EPDL' )) THEN
          mcdf_pe_xsections = .true.
          photon_xsections = 'epdl'
        ELSE
          mcdf_pe_xsections = .false.
        END IF
      END IF
      IF (( error_flags(num_pxsec_out) .EQ. 0 )) THEN
        xsec_out = value(num_pxsec_out,1)
      END IF
      IF (( error_flags(num_cxsec) .EQ. 0 )) THEN
        comp_xsections = char_value(num_cxsec,1)
      END IF
      IF (( error_flags(num_photonuc_xsec) .EQ. 0 )) THEN
        photonuc_xsections = char_value(num_photonuc_xsec,1)
      END IF
      IF (( error_flags(num_efield) .EQ. 0 )) THEN
        ExIN = value(num_efield,1)
        EyIN = value(num_efield,2)
        EzIN = value(num_efield,3)
        IF (( error_flags(num_emlmt) .EQ. 0 )) THEN
          EMLMTIN=value(num_emlmt,1)
        END IF
        IF (( ExIN**2+EyIN**2+EzIN**2 .GT. 0 )) THEN
          emfield_on=.true.
        END IF
      END IF
      IF (( error_flags(num_bfield) .EQ. 0 )) THEN
        BxIN = value(num_bfield,1)
        ByIN = value(num_bfield,2)
        BzIN = value(num_bfield,3)
        Bx=BxIN
        By=ByIN
        Bz=BzIN
        Bx_new=BxIN
        By_new=ByIN
        Bz_new=BzIN
        IF (( error_flags(num_emlmt) .EQ. 0 )) THEN
          EMLMTIN=value(num_emlmt,1)
        END IF
        IF (( BxIN**2+ByIN**2+BzIN**2 .GT. 0 )) THEN
          emfield_on=.true.
        END IF
      END IF
      IF (( error_flags(num_coh) .EQ. 0 )) THEN
        IF ((value(num_coh,1) .EQ. 4)) THEN
          write(*,'(/a/)') ' ===> custom ff requested!'
          ival = ival + 1
          num_ffmed = ival
          values_sought(ival) = 'ff media names'
          type(ival) = 2
          nvalue(ival) = 0
          ival = ival + 1
          num_ffiles = ival
          values_sought(ival) = 'ff file names'
          type(ival) = 2
          nvalue(ival) = 0
          Nmin = num_ffmed
          Nmax = num_ffiles
          CALL GET_INPUT
          IF (( error_flags(num_ffmed) .GT. 0 )) THEN
            write(i_log,'(/a)') '***************** Error: '
            write(i_log,'(a/,a,I3)') 'Error reading custom ff! Terminati
     *ng ...', ' error_flag = ', error_flags(num_ffmed)
            write(i_log,'(/a)') '***************** Quiting now.'
            call exit(1)
          END IF
          IF (( error_flags(num_ffiles) .GT. 0 )) THEN
            write(i_log,'(/a)') '***************** Error: '
            write(i_log,'(a/,a,I3)') 'Error reading ff file names! Termi
     *nating ...', ' error_flag = ', error_flags(num_ffiles)
            write(i_log,'(/a)') '***************** Quiting now.'
            call exit(1)
          END IF
          IF ((nvalue(num_ffmed).GT.1)) THEN
            write(i_log,'(/a)') '***************** Error: '
            write(i_log,'(a,a,i3,a)') '***** Number of media with custom
     * ff larger ', 'than maximum number of media $MXMED = ',1, ' increa
     *se $MXMED and try again!!!'
            write(i_log,'(/a)') '***************** Quiting now.'
            call exit(1)
          END IF
          DO 2171 i=1,nvalue(num_ffmed)
            iray_ff_media(i) = char_value(num_ffmed,i)
            iray_ff_file(i) = char_value(num_ffiles,i)
2171      CONTINUE
2172      CONTINUE
          value(num_coh,1) = 1
        END IF
        write(*,'(/)')
      END IF
      aux_inregions = .false.
      IF (( error_flags(num_incoh) .EQ. 0 )) THEN
        write(i_log,*) 'Bound Compton start region'
        itmp = value(num_incoh,1)
        IF (( itmp .EQ. 2 .OR. itmp .EQ. 3 )) THEN
          ival = ival + 1
          values_sought(ival) = 'Bound Compton start region'
          nvalue(ival) = 0
          type(ival) = 0
          value_min(ival) = 1
          value_max(ival) = 1
          default(ival) = 1
          ival = ival + 1
          values_sought(ival) = 'Bound Compton stop region'
          nvalue(ival) = 0
          type(ival) = 0
          value_min(ival) = 1
          value_max(ival) = 1
          default(ival) = 1
          Nmin = ival-1
          Nmax = ival
          CALL GET_INPUT
          IF (( error_flags(ival-1) .EQ. 0 .AND. error_flags(ival) .EQ.
     *    0 )) THEN
            IF (( nvalue(ival) .EQ. nvalue(ival-1) )) THEN
              iitmp = itmp-2
              DO 2181 j=1,1
                ibcmp(j) = iitmp
2181          CONTINUE
2182          CONTINUE
              iitmp = 1 - iitmp
              DO 2191 k=1,nvalue(ival)
                istart = value(ival-1,k)
                iend = value(ival,k)
                write(i_log,*) 'Bound Compton start region',istart
                write(i_log,*) 'Bound Compton stop region',iend
                IF (( istart .LE. iend )) THEN
                  DO 2201 j=istart,iend
                    ibcmp(j) = iitmp
2201              CONTINUE
2202              CONTINUE
                  aux_inregions = .true.
                END IF
2191          CONTINUE
2192          CONTINUE
            ELSE
              value(num_incoh,1) = ibcmp(1)
            END IF
          ELSE
            value(num_incoh,1) = ibcmp(1)
          END IF
        ELSE
          IF((itmp .GT. 3))itmp = itmp-2
          write(i_log,*) ' Setting all to ',itmp
          DO 2211 j=1,1
            ibcmp(j) = itmp
2211      CONTINUE
2212      CONTINUE
        END IF
      ELSE
        IF ((ibcmp(1) .EQ. 2 .OR. ibcmp(1) .EQ. 3)) THEN
          value(num_incoh,1) = ibcmp(1)+2
        ELSE
          value(num_incoh,1) = ibcmp(1)
        END IF
      END IF
      incoh_inregions = aux_inregions
      aux_inregions = .false.
      IF (( error_flags(num_coh) .EQ. 0 )) THEN
        write(i_log,*) 'Rayleigh start region'
        itmp = value(num_coh,1)
        IF (( itmp .EQ. 2 .OR. itmp .EQ. 3 )) THEN
          ival = ival + 1
          values_sought(ival) = 'Rayleigh start region'
          nvalue(ival) = 0
          type(ival) = 0
          value_min(ival) = 1
          value_max(ival) = 1
          default(ival) = 1
          ival = ival + 1
          values_sought(ival) = 'Rayleigh stop region'
          nvalue(ival) = 0
          type(ival) = 0
          value_min(ival) = 1
          value_max(ival) = 1
          default(ival) = 1
          Nmin = ival-1
          Nmax = ival
          CALL GET_INPUT
          IF (( error_flags(ival-1) .EQ. 0 .AND. error_flags(ival) .EQ.
     *    0 )) THEN
            IF (( nvalue(ival) .EQ. nvalue(ival-1) )) THEN
              iitmp = itmp-2
              DO 2221 j=1,1
                iraylr(j) = iitmp
2221          CONTINUE
2222          CONTINUE
              iitmp = 1 - iitmp
              DO 2231 k=1,nvalue(ival)
                istart = value(ival-1,k)
                iend = value(ival,k)
                write(i_log,*) 'Rayleigh start region',istart
                write(i_log,*) 'Rayleigh stop region',iend
                IF (( istart .LE. iend )) THEN
                  DO 2241 j=istart,iend
                    iraylr(j) = iitmp
2241              CONTINUE
2242              CONTINUE
                  aux_inregions = .true.
                END IF
2231          CONTINUE
2232          CONTINUE
            ELSE
              value(num_coh,1) = iraylr(1)
            END IF
          ELSE
            value(num_coh,1) = iraylr(1)
          END IF
        ELSE
          IF((itmp .GT. 3))itmp = itmp-2
          write(i_log,*) ' Setting all to ',itmp
          DO 2251 j=1,1
            iraylr(j) = itmp
2251      CONTINUE
2252      CONTINUE
        END IF
      ELSE
        IF ((iraylr(1) .EQ. 2 .OR. iraylr(1) .EQ. 3)) THEN
          value(num_coh,1) = iraylr(1)+2
        ELSE
          value(num_coh,1) = iraylr(1)
        END IF
      END IF
      coh_inregions = aux_inregions
      aux_inregions = .false.
      IF (( error_flags(num_relax) .EQ. 0 )) THEN
        write(i_log,*) 'Relaxations start region'
        itmp = value(num_relax,1)
        IF (( itmp .EQ. 2 .OR. itmp .EQ. 3 )) THEN
          ival = ival + 1
          values_sought(ival) = 'Relaxations start region'
          nvalue(ival) = 0
          type(ival) = 0
          value_min(ival) = 1
          value_max(ival) = 1
          default(ival) = 1
          ival = ival + 1
          values_sought(ival) = 'Relaxations stop region'
          nvalue(ival) = 0
          type(ival) = 0
          value_min(ival) = 1
          value_max(ival) = 1
          default(ival) = 1
          Nmin = ival-1
          Nmax = ival
          CALL GET_INPUT
          IF (( error_flags(ival-1) .EQ. 0 .AND. error_flags(ival) .EQ.
     *    0 )) THEN
            IF (( nvalue(ival) .EQ. nvalue(ival-1) )) THEN
              iitmp = itmp-2
              DO 2261 j=1,1
                iedgfl(j) = iitmp
2261          CONTINUE
2262          CONTINUE
              iitmp = 1 - iitmp
              DO 2271 k=1,nvalue(ival)
                istart = value(ival-1,k)
                iend = value(ival,k)
                write(i_log,*) 'Relaxations start region',istart
                write(i_log,*) 'Relaxations stop region',iend
                IF (( istart .LE. iend )) THEN
                  DO 2281 j=istart,iend
                    iedgfl(j) = iitmp
2281              CONTINUE
2282              CONTINUE
                  aux_inregions = .true.
                END IF
2271          CONTINUE
2272          CONTINUE
            ELSE
              value(num_relax,1) = iedgfl(1)
            END IF
          ELSE
            value(num_relax,1) = iedgfl(1)
          END IF
        ELSE
          IF((itmp .GT. 3))itmp = itmp-2
          write(i_log,*) ' Setting all to ',itmp
          DO 2291 j=1,1
            iedgfl(j) = itmp
2291      CONTINUE
2292      CONTINUE
        END IF
      ELSE
        IF ((iedgfl(1) .EQ. 2 .OR. iedgfl(1) .EQ. 3)) THEN
          value(num_relax,1) = iedgfl(1)+2
        ELSE
          value(num_relax,1) = iedgfl(1)
        END IF
      END IF
      relax_inregions = aux_inregions
      aux_inregions = .false.
      IF (( error_flags(num_pe_ang) .EQ. 0 )) THEN
        write(i_log,*) 'PE sampling start region'
        itmp = value(num_pe_ang,1)
        IF (( itmp .EQ. 2 .OR. itmp .EQ. 3 )) THEN
          ival = ival + 1
          values_sought(ival) = 'PE sampling start region'
          nvalue(ival) = 0
          type(ival) = 0
          value_min(ival) = 1
          value_max(ival) = 1
          default(ival) = 1
          ival = ival + 1
          values_sought(ival) = 'PE sampling stop region'
          nvalue(ival) = 0
          type(ival) = 0
          value_min(ival) = 1
          value_max(ival) = 1
          default(ival) = 1
          Nmin = ival-1
          Nmax = ival
          CALL GET_INPUT
          IF (( error_flags(ival-1) .EQ. 0 .AND. error_flags(ival) .EQ.
     *    0 )) THEN
            IF (( nvalue(ival) .EQ. nvalue(ival-1) )) THEN
              iitmp = itmp-2
              DO 2301 j=1,1
                iphter(j) = iitmp
2301          CONTINUE
2302          CONTINUE
              iitmp = 1 - iitmp
              DO 2311 k=1,nvalue(ival)
                istart = value(ival-1,k)
                iend = value(ival,k)
                write(i_log,*) 'PE sampling start region',istart
                write(i_log,*) 'PE sampling stop region',iend
                IF (( istart .LE. iend )) THEN
                  DO 2321 j=istart,iend
                    iphter(j) = iitmp
2321              CONTINUE
2322              CONTINUE
                  aux_inregions = .true.
                END IF
2311          CONTINUE
2312          CONTINUE
            ELSE
              value(num_pe_ang,1) = iphter(1)
            END IF
          ELSE
            value(num_pe_ang,1) = iphter(1)
          END IF
        ELSE
          IF((itmp .GT. 3))itmp = itmp-2
          write(i_log,*) ' Setting all to ',itmp
          DO 2331 j=1,1
            iphter(j) = itmp
2331      CONTINUE
2332      CONTINUE
        END IF
      ELSE
        IF ((iphter(1) .EQ. 2 .OR. iphter(1) .EQ. 3)) THEN
          value(num_pe_ang,1) = iphter(1)+2
        ELSE
          value(num_pe_ang,1) = iphter(1)
        END IF
      END IF
      pe_inregions = aux_inregions
      aux_inregions = .false.
      IF (( error_flags(num_photonuc) .EQ. 0 )) THEN
        write(i_log,*) 'Photonuclear start region'
        itmp = value(num_photonuc,1)
        IF (( itmp .EQ. 2 .OR. itmp .EQ. 3 )) THEN
          ival = ival + 1
          values_sought(ival) = 'Photonuclear start region'
          nvalue(ival) = 0
          type(ival) = 0
          value_min(ival) = 1
          value_max(ival) = 1
          default(ival) = 1
          ival = ival + 1
          values_sought(ival) = 'Photonuclear stop region'
          nvalue(ival) = 0
          type(ival) = 0
          value_min(ival) = 1
          value_max(ival) = 1
          default(ival) = 1
          Nmin = ival-1
          Nmax = ival
          CALL GET_INPUT
          IF (( error_flags(ival-1) .EQ. 0 .AND. error_flags(ival) .EQ.
     *    0 )) THEN
            IF (( nvalue(ival) .EQ. nvalue(ival-1) )) THEN
              iitmp = itmp-2
              DO 2341 j=1,1
                iphotonucr(j) = iitmp
2341          CONTINUE
2342          CONTINUE
              iitmp = 1 - iitmp
              DO 2351 k=1,nvalue(ival)
                istart = value(ival-1,k)
                iend = value(ival,k)
                write(i_log,*) 'Photonuclear start region',istart
                write(i_log,*) 'Photonuclear stop region',iend
                IF (( istart .LE. iend )) THEN
                  DO 2361 j=istart,iend
                    iphotonucr(j) = iitmp
2361              CONTINUE
2362              CONTINUE
                  aux_inregions = .true.
                END IF
2351          CONTINUE
2352          CONTINUE
            ELSE
              value(num_photonuc,1) = iphotonucr(1)
            END IF
          ELSE
            value(num_photonuc,1) = iphotonucr(1)
          END IF
        ELSE
          IF((itmp .GT. 3))itmp = itmp-2
          write(i_log,*) ' Setting all to ',itmp
          DO 2371 j=1,1
            iphotonucr(j) = itmp
2371      CONTINUE
2372      CONTINUE
        END IF
      ELSE
        IF ((iphotonucr(1) .EQ. 2 .OR. iphotonucr(1) .EQ. 3)) THEN
          value(num_photonuc,1) = iphotonucr(1)+2
        ELSE
          value(num_photonuc,1) = iphotonucr(1)
        END IF
      END IF
      photonuc_inregions = aux_inregions
      aux_inregions = .false.
      ival = ival + 1
      num_ecut = ival
      values_sought(ival) = 'Set ECUT'
      nvalue(ival) = 0
      type(ival) = 1
      value_min(ival) = 0.
      value_max(ival) = 1e15
      default(ival) = 0.
      ival = ival + 1
      values_sought(ival) = 'Set ECUT start region'
      nvalue(ival) = 0
      type(ival) = 0
      value_min(ival) = 1
      value_max(ival) = 1
      default(ival) = 1
      ival = ival + 1
      values_sought(ival) = 'Set ECUT stop region'
      nvalue(ival) = 0
      type(ival) = 0
      value_min(ival) = 1
      value_max(ival) = 1
      default(ival) = 1
      Nmin = num_ecut
      Nmax = num_ecut+2
      error_flag = 0
      CALL GET_INPUT
      IF (( error_flag .EQ. 0 )) THEN
        IF (( nvalue(num_ecut) .EQ. nvalue(ival) .AND. nvalue(ival-1) .E
     *  Q. nvalue(ival) )) THEN
          DO 2381 k=1,nvalue(ival)
            istart = value(ival-1,k)
            iend = value(ival,k)
            IF (( istart .LE. iend )) THEN
              DO 2391 j=istart,iend
                ecut(j) = value(num_ecut,k)
2391          CONTINUE
2392          CONTINUE
              aux_inregions = .true.
            END IF
2381      CONTINUE
2382      CONTINUE
        END IF
      END IF
      ecut_inregions = aux_inregions
      aux_inregions = .false.
      ival = ival + 1
      num_pcut = ival
      values_sought(ival) = 'Set PCUT'
      nvalue(ival) = 0
      type(ival) = 1
      value_min(ival) = 0.
      value_max(ival) = 1e15
      default(ival) = 0.
      ival = ival + 1
      values_sought(ival) = 'Set PCUT start region'
      nvalue(ival) = 0
      type(ival) = 0
      value_min(ival) = 1
      value_max(ival) = 1
      default(ival) = 1
      ival = ival + 1
      values_sought(ival) = 'Set PCUT stop region'
      nvalue(ival) = 0
      type(ival) = 0
      value_min(ival) = 1
      value_max(ival) = 1
      default(ival) = 1
      Nmin = num_pcut
      Nmax = num_pcut+2
      error_flag = 0
      CALL GET_INPUT
      IF (( error_flag .EQ. 0 )) THEN
        IF (( nvalue(num_pcut) .EQ. nvalue(ival) .AND. nvalue(ival-1) .E
     *  Q. nvalue(ival) )) THEN
          DO 2401 k=1,nvalue(ival)
            istart = value(ival-1,k)
            iend = value(ival,k)
            IF (( istart .LE. iend )) THEN
              DO 2411 j=istart,iend
                pcut(j) = value(num_pcut,k)
2411          CONTINUE
2412          CONTINUE
              aux_inregions = .true.
            END IF
2401      CONTINUE
2402      CONTINUE
        END IF
      END IF
      pcut_inregions = aux_inregions
      aux_inregions = .false.
      ival = ival + 1
      num_smax = ival
      values_sought(ival) = 'Set SMAX'
      nvalue(ival) = 0
      type(ival) = 1
      value_min(ival) = 0.
      value_max(ival) = 1e15
      default(ival) = 0.
      ival = ival + 1
      values_sought(ival) = 'Set SMAX start region'
      nvalue(ival) = 0
      type(ival) = 0
      value_min(ival) = 1
      value_max(ival) = 1
      default(ival) = 1
      ival = ival + 1
      values_sought(ival) = 'Set SMAX stop region'
      nvalue(ival) = 0
      type(ival) = 0
      value_min(ival) = 1
      value_max(ival) = 1
      default(ival) = 1
      Nmin = num_smax
      Nmax = num_smax+2
      error_flag = 0
      CALL GET_INPUT
      IF (( error_flag .EQ. 0 )) THEN
        IF (( nvalue(num_smax) .EQ. nvalue(ival) .AND. nvalue(ival-1) .E
     *  Q. nvalue(ival) )) THEN
          DO 2421 k=1,nvalue(ival)
            istart = value(ival-1,k)
            iend = value(ival,k)
            IF (( istart .LE. iend )) THEN
              DO 2431 j=istart,iend
                smaxir(j) = value(num_smax,k)
2431          CONTINUE
2432          CONTINUE
              aux_inregions = .true.
            END IF
2421      CONTINUE
2422      CONTINUE
        END IF
      END IF
      smax_inregions = aux_inregions
      write(i_errors,*)
      write(i_errors,*) ' ******************** end input transport param
     *eter *********************** '
      write(i_errors,*)
      IF ((value(num_relax,1) .GT. 0 .AND. value(num_relax,1) .LT. 5)) T
     *HEN
        eadl_relax = .true.
        IF ((value(num_relax,1) .EQ. 1)) THEN
          value(num_relax,1)=4
        END IF
      ELSE
        IF ((mcdf_pe_xsections .AND. value(num_relax,1) .EQ. 5)) THEN
          eadl_relax = .true.
          value(num_relax,1)=4
          write(i_log,'(/a)') '***************** Warning: '
          write(i_log,'(a/,a/,a/)') '    Simplified atomic relaxation no
     *t allowed', '    with shellwise PE cross sections. Resetting', '
     *  to detailed EADL atomic relaxation!!!'
        ELSE
          eadl_relax = .false.
        END IF
      END IF
      output_strings(1) = allowed_inputs(num_pair_ang,iprdst)
      itmp = value(num_incoh,1)
      output_strings(2) = allowed_inputs(num_incoh,itmp)
      output_strings(12) = allowed_inputs(num_radc,radc_flag)
      itmp = value(num_coh,1)
      output_strings(3) = allowed_inputs(num_coh,itmp)
      itmp = value(num_relax,1)
      output_strings(4) = allowed_inputs(num_relax,itmp)
      itmp = value(num_pe_ang,1)
      output_strings(5) = allowed_inputs(num_pe_ang,itmp)
      output_strings(6) = allowed_inputs(num_brems_ang,ibrdst)
      output_strings(7) = allowed_inputs(num_bca,bca_algorithm)
      output_strings(8) = allowed_inputs(num_alg,transport_algorithm)
      output_strings(9) = allowed_inputs(num_brems_cs,ibr_nist)
      output_strings(10) = allowed_inputs(num_pair_cs,pair_nrc)
      output_strings(11) = allowed_inputs(num_triplet,itriplet)
      itmp = value(num_photonuc,1)
      output_strings(14) = allowed_inputs(num_photonuc,itmp)
      entry show_transport_parameter(ounit)
      IF((ounit .LE. 0))return
      write(ounit,*)
      write(ounit,'(a)') line
      write(ounit,*)
      write(ounit,'(a,/)') '                   Electron/Photon transport
     * parameter'
      write(ounit,'(a,/)') line
      IF ((mcdf_pe_xsections)) THEN
        write(ounit,'(a,38x,a,a)') ' Photon cross sections', 'mcdf-',pho
     *  ton_xsections(:lnblnk1(photon_xsections))
      ELSE
        write(ounit,'(a,38x,a)') ' Photon cross sections', photon_xsecti
     *  ons(:lnblnk1(photon_xsections))
      END IF
      write(ounit,'(a,37x,a)') ' Compton cross sections', comp_xsections
     *(:lnblnk1(comp_xsections))
      write(ounit,'(a,$)') ' Photon transport cutoff(MeV)'
      IF (( pcut_inregions )) THEN
        write(ounit,'(32x,a)') 'Set in regions'
      ELSE
        IF (( pcut(1) .GT. 1e-4 )) THEN
          write(ounit,'(32x,g14.4)') pcut(1)
        ELSE
          write(ounit,'(32x,a)') 'AP(medium)'
        END IF
      END IF
      write(ounit,'(a,39x,a3)') ' Pair angular sampling',output_strings(
     *1)
      write(ounit,'(a,41x,a3)') ' Pair cross sections',output_strings(10
     *)
      write(ounit,'(a,42x,a3)') ' Triplet production',output_strings(11)
      write(ounit,'(a,36x,a14)') ' Bound Compton scattering',output_stri
     *ngs(2)
      write(ounit,'(a,31x,a14)') ' Radiative Compton corrections',output
     *_strings(12)
      write(ounit,'(a,41x,a14)') ' Rayleigh scattering',output_strings(3
     *)
      write(ounit,'(a,42x,a14)') ' Atomic relaxations',output_strings(4)
      write(ounit,'(a,30x,a14)') ' Photoelectron angular sampling',outpu
     *t_strings(5)
      IF (( value(num_photonuc,1) .GT. 0 )) THEN
        write(ounit,'(a,36x,a14)') ' Photonuclear attenuation',output_st
     *  rings(14)
        write(ounit,'(a,33x,a)') ' Photonuclear cross sections', photonu
     *  c_xsections(:lnblnk1(photonuc_xsections))
      END IF
      write(ounit,*)
      write(ounit,'(a,$)') ' Electron transport cutoff(MeV)'
      IF (( ecut_inregions )) THEN
        write(ounit,'(30x,a)') 'Set in regions'
      ELSE
        IF (( ecut(1) .GT. 1e-4 )) THEN
          write(ounit,'(30x,f7.4)') ecut(1)
        ELSE
          write(ounit,'(30x,a)') 'AE(medium)'
        END IF
      END IF
      write(ounit,'(a,30x,a4)') ' Bremsstrahlung cross sections',output_
     *strings(9)
      write(ounit,'(a,29x,a3)') ' Bremsstrahlung angular sampling',outpu
     *t_strings(6)
      IF (( spin_effects )) THEN
        write(ounit,'(a,48x,a)') ' Spin effects','On'
      ELSE
        write(ounit,'(a,48x,a)') ' Spin effects','Off'
      END IF
      write(ounit,'(a,34x,a)') ' Electron Impact Ionization',eii_xfile(:
     *lnblnk1(eii_xfile))
      IF ((eii_L_factor .NE. 1.0)) THEN
        write(ounit,'(a,25x,f6.4)') ' L-shell EII xsections scaling fact
     *or',eii_L_factor
      END IF
      write(ounit,'(a,$)') ' Maxium electron step in cm (SMAX)'
      IF (( smax_inregions )) THEN
        write(ounit,'(27x,a)') 'Set in regions'
      ELSE
        IF (( smaxir(1) .GT. 1e-4 )) THEN
          write(ounit,'(27x,g14.4)') smaxir(1)
        ELSE
          write(ounit,'(27x,a)') 'Restriction is off'
        END IF
      END IF
      write(ounit,'(a,16x,f6.4)') ' Maximum fractional energy loss/step
     *(ESTEPE)',estepe
      write(ounit,'(a,21x,f6.4)') ' Maximum 1st elastic moment/step (XIM
     *AX)',ximax
      write(ounit,'(a,33x,a10)') ' Boundary crossing algorithm',output_s
     *trings(7)
      write(ounit,'(a,22x,g9.4)') ' Skin-depth for boundary crossing (MF
     *P)',skindepth_for_bca
      write(ounit,'(a,37x,a10)') ' Electron-step algorithm',output_strin
     *gs(8)
      IF (( ExIN.NE.0 .OR. EyIN.NE.0 .OR. EzIN.NE.0 )) THEN
        write(ounit,'(a,38x,3f10.2)') ' Electric Field [V/cm]', ExIN,EyI
     *  N,EzIN
      END IF
      IF (( Bx.NE.0 .OR. By.NE.0 .OR. Bz.NE.0 )) THEN
        write(ounit,'(a,41x,3f10.2)') ' Magnetic Field [T]', Bx,By,Bz
      END IF
      IF (( ExIN.NE.0 .OR. EyIN.NE.0 .OR. EzIN.NE.0 .OR. Bx.NE.0 .OR. By
     *.NE.0 .OR. Bz.NE.0 )) THEN
        write(ounit,'(a,50x,f10.2)') ' EM ESTEPE',EMLMTIN
      END IF
      write(ounit,*)
      write(ounit,'(a)') line
      write(ounit,*)
      return
      end
      subroutine set_elastic_parameter
      implicit none
      integer*4 ounit
      COMMON/GetInput/ ALLOWED_INPUTS(100,0:5),   VALUES_SOUGHT(100),  C
     *HAR_VALUE(100,100),  VALUE(100,100),  DEFAULT(100),  VALUE_MIN(100
     *),  VALUE_MAX(100),  NVALUE(100),  TYPE(100),      ERROR_FLAGS(100
     *),   i_errors,  NMIN, NMAX,   ERROR_FLAG,  DELIMETER
      character ALLOWED_INPUTS*64,VALUES_SOUGHT*64, CHAR_VALUE*256,DELIM
     *ETER*64
      real*8 VALUE,DEFAULT,VALUE_MIN,VALUE_MAX
      integer*4 NVALUE,TYPE,NMIN,NMAX,ERROR_FLAG,ERROR_FLAGS,i_errors
      COMMON/ELECIN/ esig_e(1),psig_e(1), esige_max, psige_max, range_ep
     *(0:1,500,1), E_array(500,1), etae_ms0(500,1),etae_ms1(500,1),etap_
     *ms0(500,1),etap_ms1(500,1),q1ce_ms0(500,1),q1ce_ms1(500,1),q1cp_ms
     *0(500,1),q1cp_ms1(500,1),q2ce_ms0(500,1),q2ce_ms1(500,1),q2cp_ms0(
     *500,1),q2cp_ms1(500,1),blcce0(500,1),blcce1(500,1), EKE0(1),EKE1(1
     *), XR0(1),TEFF0(1),BLCC(1),XCC(1), ESIG0(500,1),ESIG1(500,1),PSIG0
     *(500,1),PSIG1(500,1),EDEDX0(500,1),EDEDX1(500,1),PDEDX0(500,1),PDE
     *DX1(500,1),EBR10(500,1),EBR11(500,1),PBR10(500,1),PBR11(500,1),PBR
     *20(500,1),PBR21(500,1),TMXS0(500,1),TMXS1(500,1), expeke1(1), IUNR
     *ST(1),EPSTFL(1),IAPRIM(1), sig_ismonotone(0:1,1)
      real*8 esig_e,   psig_e,   esige_max,  psige_max,  range_ep,  E_ar
     *ray,  etae_ms0,etae_ms1,  etap_ms0,etap_ms1,  q1ce_ms0,q1ce_ms1,
     *q1cp_ms0,q1cp_ms1,  q2ce_ms0,q2ce_ms1,  q2cp_ms0,q2cp_ms1,  blcce0
     *,blcce1,   expeke1,  EKE0,EKE1, XR0,  TEFF0,  BLCC,  XCC,  ESIG0,E
     *SIG1,  PSIG0,PSIG1,  EDEDX0,EDEDX1,  PDEDX0,PDEDX1,  EBR10,EBR11,
     * PBR10,PBR11,  PBR20,PBR21,  TMXS0,TMXS1
      integer*4 IUNRST,  EPSTFL,  IAPRIM
      logical sig_ismonotone
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      integer*4 imed,ival,lnblnk1,nchanged
      character*24 medname
      ounit = i_log
      ounit = i_log
      delimeter = 'MC TRANSPORT PARAMETER'
      call get_input_set_error_level(0)
      ival = 0
      DO 2441 imed=1,nmed
        call egs_get_medium_name(imed,medname)
        ival = ival + 1
        values_sought(ival) = 'scale elastic scattering in '// medname(:
     *  lnblnk1(medname))
        nvalue(ival) = 1
        type(ival) = 1
        value_min(ival) = 1e-3
        value_max(ival) = 1e3
        default(ival) = 1
2441  CONTINUE
2442  CONTINUE
      Nmin = 1
      Nmax = nmed
      CALL GET_INPUT
      nchanged = 0
      DO 2451 imed=1,nmed
        IF((error_flags(imed) .EQ. 0))nchanged = nchanged + 1
2451  CONTINUE
2452  CONTINUE
      IF (( nchanged .GT. 0 )) THEN
        write(ounit,'(//a)') '================ Elastic scattering scaled
     * as follows =================='
        DO 2461 imed=1,nmed
          IF (( error_flags(imed) .EQ. 0 )) THEN
            call egs_get_medium_name(imed,medname)
            xcc(imed) = xcc(imed)*value(imed,1)
            blcc(imed) = blcc(imed)*value(imed,1)
            write(ounit,'(a,t30,f10.6)') medname(:lnblnk1(medname)), val
     *      ue(imed,1)
          END IF
2461    CONTINUE
2462    CONTINUE
        write(ounit,'(a//)') '==========================================
     *=============================='
      END IF
      return
      end
      SUBROUTINE GET_INPUT_PLUS(UNITNUM,DELIM_START,DELIM_END)
      IMPLICIT NONE
      COMMON/GetInput/ ALLOWED_INPUTS(100,0:5),   VALUES_SOUGHT(100),  C
     *HAR_VALUE(100,100),  VALUE(100,100),  DEFAULT(100),  VALUE_MIN(100
     *),  VALUE_MAX(100),  NVALUE(100),  TYPE(100),      ERROR_FLAGS(100
     *),   i_errors,  NMIN, NMAX,   ERROR_FLAG,  DELIMETER
      character ALLOWED_INPUTS*64,VALUES_SOUGHT*64, CHAR_VALUE*256,DELIM
     *ETER*64
      real*8 VALUE,DEFAULT,VALUE_MIN,VALUE_MAX
      integer*4 NVALUE,TYPE,NMIN,NMAX,ERROR_FLAG,ERROR_FLAGS,i_errors
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      CHARACTER*256 TEXT
      CHARACTER*256 KEEPTEXT
      CHARACTER*256 ORIGTEXT
      CHARACTER*256 TEXTPIECE
      CHARACTER*40 DELIM_START
      CHARACTER*40 DELIM_END
      CHARACTER*40 ENDSTRING
      CHARACTER*64 VNAME
      CHARACTER*64 VNAME1
      integer*4 CURSOR
      integer*4 IINDEX
      integer*4 iVNAME
      integer*4 IVAL
      integer*4 UNITNUM
      integer*4 ERR
      integer*4 I,J,K,CHECK
      integer*4 LINE
      integer*4 INT_VALUE
      integer*4 INT_VALUE_MIN
      integer*4 INT_VALUE_MAX
      logical ALLOWED
      logical START_FOUND
      integer*4 ifound,length,lll,Kconvert
      integer*4 lnblnk1
      logical IDEBUG,end_string
      character*1 blank
      integer*4 error_level
      integer*4 the_level
      data blank/' '/
      data error_level/1/
      save error_level
      IDEBUG = .false.
      ERROR_FLAG = 0
      IF ((IDEBUG)) THEN
        WRITE(6,2470)NMIN,NMAX, 100
2470    FORMAT(' Entering get_inputs seeking values', I5,' to', I5, '  w
     *ith a max allowed of',I5)
      END IF
      IF ((NMAX .LT. NMIN .OR. NMAX .GT. 100)) THEN
        WRITE(6,2480)NMAX, NMIN, 100
2480    FORMAT(//' Error entering get_inputs: Asked for values from',I5,
     *' to',I5, '    with a max of',I5//' This implies a bug in the call
     *ing routine'/ ' Fix it up and try again.  Stopping now.')
        STOP
      END IF
      ERR=i_errors
      DELIM_START=DELIM_START(:lnblnk1(DELIM_START))
      DELIM_END=DELIM_END(:lnblnk1(DELIM_END))
      length = len(DELIM_START)
2491  IF(index(DELIM_START,blank).NE.1)GO TO 2492
        IF (( length .GE. 2 )) THEN
          DELIM_START=DELIM_START(2:)
        ELSE
          GO TO2492
        END IF
        length = length - 1
      GO TO 2491
2492  CONTINUE
      length = len(DELIM_END)
2501  IF(index(DELIM_END,blank).NE.1)GO TO 2502
        IF (( length .GE. 2 )) THEN
          DELIM_END=DELIM_END(2:)
        ELSE
          GO TO2502
        END IF
        length = length - 1
      GO TO 2501
2502  CONTINUE
      DO 2511 Kconvert=1,lnblnk1(DELIM_START)
        CURSOR=ICHAR(DELIM_START(Kconvert:Kconvert))
        IF (((CURSOR.GE.97).AND.(CURSOR.LE.122))) THEN
          CURSOR=CURSOR-32
          DELIM_START(Kconvert:Kconvert)=CHAR(CURSOR)
        END IF
2511  CONTINUE
2512  CONTINUE
      DO 2521 Kconvert=1,lnblnk1(DELIM_END)
        CURSOR=ICHAR(DELIM_END(Kconvert:Kconvert))
        IF (((CURSOR.GE.97).AND.(CURSOR.LE.122))) THEN
          CURSOR=CURSOR-32
          DELIM_END(Kconvert:Kconvert)=CHAR(CURSOR)
        END IF
2521  CONTINUE
2522  CONTINUE
      length = len(ENDSTRING)
2531  IF(index(ENDSTRING,blank).NE.1)GO TO 2532
        IF (( length .GE. 2 )) THEN
          ENDSTRING=ENDSTRING(2:)
        ELSE
          GO TO2532
        END IF
        length = length - 1
      GO TO 2531
2532  CONTINUE
      IF ((ENDSTRING.EQ.blank)) THEN
        end_string=.false.
      ELSE
        DO 2541 Kconvert=1,lnblnk1(ENDSTRING)
          CURSOR=ICHAR(ENDSTRING(Kconvert:Kconvert))
          IF (((CURSOR.GE.97).AND.(CURSOR.LE.122))) THEN
            CURSOR=CURSOR-32
            ENDSTRING(Kconvert:Kconvert)=CHAR(CURSOR)
          END IF
2541    CONTINUE
2542    CONTINUE
        end_string=.false.
      END IF
      IF ((IDEBUG)) THEN
        WRITE(6,2550)DELIM_START,DELIM_END
2550    FORMAT(' start and stop delimeters are:'/ A/A/)
      END IF
      DO 2561 I=NMIN,NMAX
        REWIND (UNITNUM)
        LINE=0
        CHECK=0
        ERROR_FLAGS(I)=0
        IF ((TYPE(I) .EQ. 0 .OR. TYPE(I) .EQ. 1)) THEN
          VALUE(I,1) = DEFAULT(I)
        END IF
        IF ((TYPE(I) .EQ. 3)) THEN
          VALUE(I,1) = 0
        END IF
        VNAME=VALUES_SOUGHT(I)
        iVNAME=lnblnk1(VNAME)
        IF (( ivname .LT. 1 )) THEN
          IF (( error_level .GT. 0 )) THEN
            write(ERR,*) ' ======================= Warning =============
     *======== '
            write(ERR,*) '    Empty VALUES_SOUGHT passt to Get_Inputs()!
     *         '
            write(ERR,*) ' =============================================
     *======== '
          END IF
          ERROR_FLAG=1
          ERROR_FLAGS(I)=1
          goto 1620
        END IF
        DO 2571 Kconvert=1,lnblnk1(vname)
          CURSOR=ICHAR(vname(Kconvert:Kconvert))
          IF (((CURSOR.GE.97).AND.(CURSOR.LE.122))) THEN
            CURSOR=CURSOR-32
            vname(Kconvert:Kconvert)=CHAR(CURSOR)
          END IF
2571    CONTINUE
2572    CONTINUE
        iindex = 0
        IF ((DELIM_START .EQ. 'NONE')) THEN
          start_found = .true.
        ELSE
          start_found = .false.
        END IF
2581    IF(iindex.NE.0)GO TO 2582
1650      CONTINUE
          LINE=LINE+1
          IF (( start_found )) THEN
            READ(UNITNUM,END=1660,ERR=1670,FMT='(A256)') TEXT
          ELSE
            READ(UNITNUM,END=1680,ERR=1670,FMT='(A256)') TEXT
          END IF
          length = len(text)
2591      IF(index(text,blank).NE.1)GO TO 2592
            IF (( length .GE. 2 )) THEN
              text=text(2:)
            ELSE
              GO TO2592
            END IF
            length = length - 1
          GO TO 2591
2592      CONTINUE
          ifound = INDEX(text,'#')
          IF (( ifound .GT. 1 )) THEN
            text = text(1:ifound-1)
          ELSE
            IF (( ifound .EQ. 1 )) THEN
              text = blank
            END IF
          END IF
          ifound = INDEX(text,';')
          IF (( ifound .GT. 1 )) THEN
            text = text(1:ifound-1)
          ELSE
            IF (( ifound .EQ. 1 )) THEN
              text = blank
            END IF
          END IF
          length = lnblnk1(TEXT)
          TEXT=TEXT(:length)
          origtext = text(:length)
          DO 2601 Kconvert=1,lnblnk1(text)
            CURSOR=ICHAR(text(Kconvert:Kconvert))
            IF (((CURSOR.GE.97).AND.(CURSOR.LE.122))) THEN
              CURSOR=CURSOR-32
              text(Kconvert:Kconvert)=CHAR(CURSOR)
            END IF
2601      CONTINUE
2602      CONTINUE
          IF (( .NOT.start_found )) THEN
            IF ((INDEX(TEXT,DELIM_START) .NE. 0 )) THEN
              start_found = .true.
            END IF
            goto 1650
          END IF
          iindex=INDEX(TEXT,VNAME(:iVNAME))
          IF (( DELIM_END.NE.'NONE' )) THEN
            IF ((INDEX(TEXT,DELIM_END).NE.0)) THEN
              IF (( error_level .GT. 0 )) THEN
                WRITE (ERR,*) '***************ERROR***************'
                WRITE (ERR,*) '>>',VALUES_SOUGHT(I)(:lnblnk1(VALUES_SOUG
     *          HT(I))), '<<',' NOT FOUND'
                WRITE (ERR,*) 'END OF DELIMETER: ',DELIM_END
              END IF
              ERROR_FLAG=1
              ERROR_FLAGS(I)=1
              GOTO 1620
            END IF
          END IF
        GO TO 2581
2582    CONTINUE
        CHECK=0
        IF (( idebug )) THEN
          write(i_log,*) ' ******* Found: '
          write(i_log,'(a,$)') ' text:     '
          length = lnblnk1(text)
          IF (( length .GT. 0 )) THEN
            DO 2611 lll=1,length
              write(i_log,'(a1,$)') text(lll:lll)
2611        CONTINUE
2612        CONTINUE
            write(i_log,*)
          END IF
          write(i_log,'(a,$)') ' origtext: '
          length = lnblnk1(origtext)
          IF (( length .GT. 0 )) THEN
            DO 2621 lll=1,length
              write(i_log,'(a1,$)') origtext(lll:lll)
2621        CONTINUE
2622        CONTINUE
            write(i_log,*)
          END IF
        END IF
        IINDEX=IINDEX+iVNAME
        TEXT=TEXT(IINDEX:)
        origtext=origtext(iindex:)
        IF (( idebug )) THEN
          write(i_log,*) ' After removing vname: '
          write(i_log,'(a,$)') ' text:     '
          length = lnblnk1(text)
          IF (( length .GT. 0 )) THEN
            DO 2631 lll=1,length
              write(i_log,'(a1,$)') text(lll:lll)
2631        CONTINUE
2632        CONTINUE
            write(i_log,*)
          END IF
          write(i_log,'(a,$)') ' origtext: '
          length = lnblnk1(origtext)
          IF (( length .GT. 0 )) THEN
            DO 2641 lll=1,length
              write(i_log,'(a1,$)') origtext(lll:lll)
2641        CONTINUE
2642        CONTINUE
            write(i_log,*)
          END IF
        END IF
        IINDEX=INDEX(TEXT,'=')
        IF ((IINDEX.NE.0)) THEN
          TEXT=TEXT(IINDEX+1:)
          origtext=origtext(iindex+1:)
        ELSE
          IINDEX=INDEX(TEXT,':')
          IF ((IINDEX.NE.0)) THEN
            TEXT=TEXT(IINDEX+1:)
            origtext=origtext(iindex+1:)
          END IF
        END IF
        IF (( idebug )) THEN
          write(i_log,*) ' After removing leading equals: '
          write(i_log,'(a,$)') ' text:     '
          length = lnblnk1(text)
          IF (( length .GT. 0 )) THEN
            DO 2651 lll=1,length
              write(i_log,'(a1,$)') text(lll:lll)
2651        CONTINUE
2652        CONTINUE
            write(i_log,*)
          END IF
          write(i_log,'(a,$)') ' origtext: '
          length = lnblnk1(origtext)
          IF (( length .GT. 0 )) THEN
            DO 2661 lll=1,length
              write(i_log,'(a1,$)') origtext(lll:lll)
2661        CONTINUE
2662        CONTINUE
            write(i_log,*)
          END IF
        END IF
        IF (( (lnblnk1(TEXT).EQ.0) .OR. (lnblnk1(TEXT).EQ.1) )) THEN
          IF ((vname(:ivname).EQ.'TITLE')) THEN
            READ (UNITNUM,FMT='(A256)') TEXTPIECE
            IF ((lnblnk1(TEXTPIECE).NE.0)) THEN
              TEXT=TEXTPIECE(:lnblnk1(TEXTPIECE))
              length = len(text)
2671          IF(index(text,blank).NE.1)GO TO 2672
                IF (( length .GE. 2 )) THEN
                  text=text(2:)
                ELSE
                  GO TO2672
                END IF
                length = length - 1
              GO TO 2671
2672          CONTINUE
              length = len(origtext)
2681          IF(index(origtext,blank).NE.1)GO TO 2682
                IF (( length .GE. 2 )) THEN
                  origtext=origtext(2:)
                ELSE
                  GO TO2682
                END IF
                length = length - 1
              GO TO 2681
2682          CONTINUE
              GOTO 1790
            END IF
          END IF
          IF (( error_level .GT. 0 )) THEN
            WRITE (ERR,*) '*************ERROR*************'
            WRITE (ERR,*) 'VALUE SOUGHT: ',VALUES_SOUGHT(I)
            WRITE (ERR,*) 'VALUE NOT THERE!!'
          END IF
          ERROR_FLAG=1
          ERROR_FLAGS(I)=1
          RETURN
        END IF
1790    CONTINUE
        iindex = index(text,'DEFAULT')
        IF (( iindex .NE. 0 )) THEN
          IF (( type(i) .NE. 2 )) THEN
            IF (( type(i) .NE. 3 )) THEN
              VALUE(I,1)=DEFAULT(I)
            ELSE
              VALUE(I,1)=0
            END IF
            goto 1620
          END IF
        END IF
        IF (((TYPE(I) .EQ. 0).OR.(TYPE(I) .EQ. 1))) THEN
          IVAL=1
          IF (( idebug )) THEN
            write(i_log,*) ' *** Reading an integer or a real value! '
          END IF
2691      CONTINUE
            IF (( idebug )) THEN
              write(i_log,*) ' In LOOP, ival = ',ival
            END IF
            IF ((lnblnk1(TEXT).EQ.0)) THEN
              IF (( error_level .GT. 0 )) THEN
                WRITE(ERR,*) '*************ERROR*************'
                WRITE (ERR,*) 'VALUE SOUGHT: ',VALUES_SOUGHT(I)
                WRITE (ERR,*) 'VALUE NOT THERE!!'
              END IF
              ERROR_FLAG=1
              ERROR_FLAGS(I)=1
              RETURN
            END IF
            READ(TEXT,END=1810,ERR=1820,FMT=*) VALUE(I,IVAL)
            IF (( idebug )) THEN
              write(i_log,*) ' Read value: ',ival,VALUE(I,IVAL)
            END IF
            IF (((VALUE(I,IVAL).GT.VALUE_MAX(I)).OR.(VALUE(I,IVAL).LT.VA
     *      LUE_MIN(I)))) THEN
              IF ((TYPE(I).EQ.0)) THEN
                INT_VALUE=DEFAULT(I)
                IF (( error_level .GT. 0 )) THEN
                  WRITE(ERR,*) '************WARNING************'
                  WRITE(ERR,1830) INT_VALUE, VALUES_SOUGHT(I)(:lnblnk1(V
     *            ALUES_SOUGHT(I)))
                END IF
1830            FORMAT ( 'Default= ',I9,' used for: ', A )
                INT_VALUE=VALUE(I,IVAL)
                INT_VALUE_MIN=VALUE_MIN(I)
                INT_VALUE_MAX=VALUE_MAX(I)
                IF (( error_level .GT. 0 )) THEN
                  WRITE(ERR,1840) VALUES_SOUGHT(I)(:lnblnk1(VALUES_SOUGH
     *            T(I))), INT_VALUE, INT_VALUE_MIN,INT_VALUE_MAX
                END IF
1840            FORMAT (A,'=', I9,' should be between ', I9,' and ', I9)
              END IF
              IF ((TYPE(I).EQ.1)) THEN
                IF (( error_level .GT. 0 )) THEN
                  WRITE(ERR,*) '************WARNING************'
                  WRITE(ERR,1850) DEFAULT(I), VALUES_SOUGHT(I)(:lnblnk1(
     *            VALUES_SOUGHT(I)))
1850              FORMAT ( 'Default= ',F12.6,' used for: ', A )
                  WRITE(ERR,1860) VALUES_SOUGHT(I)(:lnblnk1(VALUES_SOUGH
     *            T(I))), VALUE(I,IVAL), VALUE_MIN(I),VALUE_MAX(I)
1860              FORMAT (A,'=', F12.6,' should be between ', G14.6,' an
     *d ', G14.6)
                END IF
              END IF
              VALUE(I,IVAL)=DEFAULT(I)
            END IF
            IF((IVAL .EQ. NVALUE(I)))GO TO2692
            IF (((INDEX(TEXT,',').NE.0).OR.(lnblnk1(TEXT).EQ.0))) THEN
              IF (( idebug )) THEN
                write(i_log,*) ' A comma or a blank text found -> '
                write(i_log,*) ' searching for further input'
              END IF
              TEXT=TEXT(INDEX(TEXT,',')+1:)
2701          IF(lnblnk1(TEXT).NE.0)GO TO 2702
                IF (( idebug )) THEN
                  write(i_log,*) ' Empty text -> reading next line! '
                END IF
                LINE=LINE+1
                READ (UNITNUM,END=1810,ERR=1820,FMT='(A256)') TEXT
                length = len(text)
2711            IF(index(text,blank).NE.1)GO TO 2712
                  IF (( length .GE. 2 )) THEN
                    text=text(2:)
                  ELSE
                    GO TO2712
                  END IF
                  length = length - 1
                GO TO 2711
2712            CONTINUE
                ifound = INDEX(text,'#')
                IF (( ifound .GT. 1 )) THEN
                  text = text(1:ifound-1)
                ELSE
                  IF (( ifound .EQ. 1 )) THEN
                    text = blank
                  END IF
                END IF
                ifound = INDEX(text,';')
                IF (( ifound .GT. 1 )) THEN
                  text = text(1:ifound-1)
                ELSE
                  IF (( ifound .EQ. 1 )) THEN
                    text = blank
                  END IF
                END IF
                length = lnblnk1(TEXT)
                TEXT=TEXT(:length)
                origtext = text(:length)
                DO 2721 Kconvert=1,lnblnk1(text)
                  CURSOR=ICHAR(text(Kconvert:Kconvert))
                  IF (((CURSOR.GE.97).AND.(CURSOR.LE.122))) THEN
                    CURSOR=CURSOR-32
                    text(Kconvert:Kconvert)=CHAR(CURSOR)
                  END IF
2721            CONTINUE
2722            CONTINUE
                DO 2731 K=1,NMAX
                  vname1 = VALUES_SOUGHT(K)
                  length = lnblnk1(vname1)
                  IF (( length .GT. 0 )) THEN
                    length = len(vname1)
2741                IF(index(vname1,blank).NE.1)GO TO 2742
                      IF (( length .GE. 2 )) THEN
                        vname1=vname1(2:)
                      ELSE
                        GO TO2742
                      END IF
                      length = length - 1
                    GO TO 2741
2742                CONTINUE
                    DO 2751 Kconvert=1,lnblnk1(vname1)
                      CURSOR=ICHAR(vname1(Kconvert:Kconvert))
                      IF (((CURSOR.GE.97).AND.(CURSOR.LE.122))) THEN
                        CURSOR=CURSOR-32
                        vname1(Kconvert:Kconvert)=CHAR(CURSOR)
                      END IF
2751                CONTINUE
2752                CONTINUE
                    IF ((INDEX(TEXT,vname1(:length)).NE.0)) THEN
                      IF (( error_level .GT. 0 )) THEN
                        WRITE(ERR,*) '************ERROR************'
                        WRITE(ERR,*) 'VALUE SOUGHT: ',VALUES_SOUGHT(I)
                        WRITE(ERR,*) KEEPTEXT(:lnblnk1(KEEPTEXT)), '<--C
     *OMMA INDICATES ANOTHER INPUT'
                        WRITE(ERR,*) 'SEARCHED NEXT LINE: ', TEXT(:lnbln
     *                  k1(TEXT))
                        WRITE(ERR,*) 'BUT NO OTHER INPUT WAS DETECTED'
                      END IF
                      ERROR_FLAG=1
                      ERROR_FLAGS(I)=1
                    END IF
                  END IF
2731            CONTINUE
2732            CONTINUE
                IF (( idebug )) THEN
                  write(i_log,*) ' Next line: '
                  write(i_log,'(a,$)') ' text:     '
                  length = lnblnk1(text)
                  IF (( length .GT. 0 )) THEN
                    DO 2761 lll=1,length
                      write(i_log,'(a1,$)') text(lll:lll)
2761                CONTINUE
2762                CONTINUE
                    write(i_log,*)
                  END IF
                  write(i_log,'(a,$)') ' origtext: '
                  length = lnblnk1(origtext)
                  IF (( length .GT. 0 )) THEN
                    DO 2771 lll=1,length
                      write(i_log,'(a1,$)') origtext(lll:lll)
2771                CONTINUE
2772                CONTINUE
                    write(i_log,*)
                  END IF
                END IF
              GO TO 2701
2702          CONTINUE
            ELSE
              GO TO2692
            END IF
            IVAL=IVAL+1
          GO TO 2691
2692      CONTINUE
          IF (((NVALUE(I).NE.0).AND.(NVALUE(I).NE.IVAL))) THEN
            IF (( error_level .GT. 0 )) THEN
              WRITE (ERR,*) '**************ERROR**************'
              WRITE (ERR,*) 'VALUE SOUGHT: ', VALUES_SOUGHT(I)
              WRITE (ERR,*) 'ASKED FOR', NVALUE(I),' NUMERICAL INPUT(S)'
              WRITE (ERR,*) 'HOWEVER,', IVAL, ' WERE DETECTED'
            END IF
            ERROR_FLAG=1
            ERROR_FLAGS(I)=1
          ELSE
            NVALUE(I)=IVAL
          END IF
1810      CONTINUE
        END IF
        IF (((TYPE(I) .EQ. 2) .OR. (TYPE(I) .EQ. 3))) THEN
          IVAL=1
          IF (( idebug )) THEN
            write(i_log,*) ' Trying to read a string! '
          END IF
2781      CONTINUE
            IF (( idebug )) THEN
              write(i_log,*) ' In LOOP, ival = ',ival
            END IF
            IF ((lnblnk1(TEXT).EQ.0)) THEN
              IF (( error_level .GT. 0 )) THEN
                WRITE(ERR,*) '*************ERROR*************'
                WRITE (ERR,*) 'VALUE SOUGHT: ',VALUES_SOUGHT(I)
                WRITE (ERR,*) 'VALUE NOT THERE!!'
              END IF
              ERROR_FLAG=1
              ERROR_FLAGS(I)=1
              RETURN
            END IF
            IF ((vname(:ivname).EQ.'TITLE')) THEN
              TEXTPIECE=origtext
              GOTO 1960
            END IF
            iindex = INDEX(origtext,',')
            IF (( iindex .NE. 0 )) THEN
              TEXTPIECE=origtext(:iindex-1)
            ELSE
              TEXTPIECE=origtext
            END IF
1960        CONTINUE
            READ(TEXTPIECE,ERR=1970,FMT='(A256)') CHAR_VALUE(I,IVAL)
            length = len(CHAR_VALUE(I,IVAL))
2791        IF(index(CHAR_VALUE(I,IVAL),blank).NE.1)GO TO 2792
              IF (( length .GE. 2 )) THEN
                CHAR_VALUE(I,IVAL)=CHAR_VALUE(I,IVAL)(2:)
              ELSE
                GO TO2792
              END IF
              length = length - 1
            GO TO 2791
2792        CONTINUE
            IF (( idebug )) THEN
              write(i_log,*) ' Read the following char string: '
              length = lnblnk1(CHAR_VALUE(I,IVAL))
              IF (( length .GT. 0 )) THEN
                DO 2801 lll=1,length
                  write(i_log,'(a1,$)') CHAR_VALUE(I,IVAL)(lll:lll)
2801            CONTINUE
2802            CONTINUE
                write(i_log,*)
              END IF
            END IF
            IF ((TYPE(I) .EQ. 3)) THEN
              DO 2811 Kconvert=1,lnblnk1(CHAR_VALUE(I,IVAL))
                CURSOR=ICHAR(CHAR_VALUE(I,IVAL)(Kconvert:Kconvert))
                IF (((CURSOR.GE.97).AND.(CURSOR.LE.122))) THEN
                  CURSOR=CURSOR-32
                  CHAR_VALUE(I,IVAL)(Kconvert:Kconvert)=CHAR(CURSOR)
                END IF
2811          CONTINUE
2812          CONTINUE
              ALLOWED=.FALSE.
              DO 2821 K=0,5
                vname1 = ALLOWED_INPUTS(I,K)
                length = len(ALLOWED_INPUTS(I,K))
2831            IF(index(ALLOWED_INPUTS(I,K),blank).NE.1)GO TO 2832
                  IF (( length .GE. 2 )) THEN
                    ALLOWED_INPUTS(I,K)=ALLOWED_INPUTS(I,K)(2:)
                  ELSE
                    GO TO2832
                  END IF
                  length = length - 1
                GO TO 2831
2832            CONTINUE
                DO 2841 Kconvert=1,lnblnk1(ALLOWED_INPUTS(I,K))
                  CURSOR=ICHAR(ALLOWED_INPUTS(I,K)(Kconvert:Kconvert))
                  IF (((CURSOR.GE.97).AND.(CURSOR.LE.122))) THEN
                    CURSOR=CURSOR-32
                    ALLOWED_INPUTS(I,K)(Kconvert:Kconvert)=CHAR(CURSOR)
                  END IF
2841            CONTINUE
2842            CONTINUE
                IF ((ALLOWED_INPUTS(I,K).EQ.CHAR_VALUE(I,IVAL))) THEN
                  ALLOWED=.TRUE.
                  VALUE(I,IVAL)=K
                  IF (( idebug )) THEN
                    write(i_log,*) ' Found a allowed_value match ',k
                  END IF
                END IF
2821          CONTINUE
2822          CONTINUE
              IF ((.NOT.ALLOWED)) THEN
                WRITE(ERR,*) '*************ERROR*************'
                IF ((IVAL.NE.1)) THEN
                  WRITE (ERR,*) 'VALUE SOUGHT: ',VALUES_SOUGHT(I)
                  WRITE (ERR,*) 'SHOULD HAVE ONE INPUT ONLY'
                  WRITE (ERR,*) 'APPARENT STATE: COMMA INDICATING SECOND
     * VALUE'
                ELSE
                  WRITE (ERR,*) 'VALUE SOUGHT: ',VALUES_SOUGHT(I)
                  WRITE(ERR,*) 'INPUT-->', CHAR_VALUE(I,IVAL)(:lnblnk1(C
     *            HAR_VALUE(I,IVAL))), '<--NOT ALLOWED'
                  WRITE(ERR,*) 'OPTIONS ARE:'
                  WRITE(ERR,2040) (ALLOWED_INPUTS(I,K)(:lnblnk1(ALLOWED_
     *            INPUTS(I,K))),K=0,5)
                END IF
2040            FORMAT(A40)
                ERROR_FLAG=1
                ERROR_FLAGS(I)=1
              END IF
            END IF
            IF ((vname(:ivname).EQ.'TITLE')) THEN
              GO TO2782
            END IF
            DO 2851 K=1,LEN(KEEPTEXT)
              KEEPTEXT(K:K)=' '
2851        CONTINUE
2852        CONTINUE
            KEEPTEXT(:lnblnk1(TEXT))=TEXT
            iindex = INDEX(TEXT,',')
            IF (( iindex .NE. 0 .OR. lnblnk1(TEXT).EQ.0 )) THEN
              TEXT=TEXT(INDEX(TEXT,',')+1:)
              origtext=origtext(iindex+1:)
2861          IF(lnblnk1(TEXT).NE.0)GO TO 2862
                LINE=LINE+1
                READ (UNITNUM,ERR=1970,FMT='(A256)') TEXT
                length = len(text)
2871            IF(index(text,blank).NE.1)GO TO 2872
                  IF (( length .GE. 2 )) THEN
                    text=text(2:)
                  ELSE
                    GO TO2872
                  END IF
                  length = length - 1
                GO TO 2871
2872            CONTINUE
                ifound = INDEX(text,'#')
                IF (( ifound .GT. 1 )) THEN
                  text = text(1:ifound-1)
                ELSE
                  IF (( ifound .EQ. 1 )) THEN
                    text = blank
                  END IF
                END IF
                ifound = INDEX(text,';')
                IF (( ifound .GT. 1 )) THEN
                  text = text(1:ifound-1)
                ELSE
                  IF (( ifound .EQ. 1 )) THEN
                    text = blank
                  END IF
                END IF
                length = lnblnk1(TEXT)
                TEXT=TEXT(:length)
                origtext = text(:length)
                DO 2881 Kconvert=1,lnblnk1(text)
                  CURSOR=ICHAR(text(Kconvert:Kconvert))
                  IF (((CURSOR.GE.97).AND.(CURSOR.LE.122))) THEN
                    CURSOR=CURSOR-32
                    text(Kconvert:Kconvert)=CHAR(CURSOR)
                  END IF
2881            CONTINUE
2882            CONTINUE
                DO 2891 K=1,NMAX
                  vname1 = VALUES_SOUGHT(K)
                  length = lnblnk1(vname1)
                  IF (( length .GT. 0 )) THEN
                    length = len(vname1)
2901                IF(index(vname1,blank).NE.1)GO TO 2902
                      IF (( length .GE. 2 )) THEN
                        vname1=vname1(2:)
                      ELSE
                        GO TO2902
                      END IF
                      length = length - 1
                    GO TO 2901
2902                CONTINUE
                    DO 2911 Kconvert=1,lnblnk1(vname1)
                      CURSOR=ICHAR(vname1(Kconvert:Kconvert))
                      IF (((CURSOR.GE.97).AND.(CURSOR.LE.122))) THEN
                        CURSOR=CURSOR-32
                        vname1(Kconvert:Kconvert)=CHAR(CURSOR)
                      END IF
2911                CONTINUE
2912                CONTINUE
                    IF ((INDEX(TEXT,vname1(:length)).NE.0)) THEN
                      WRITE(ERR,*) '************ERROR************'
                      WRITE(ERR,*) 'VALUE SOUGHT: ',VALUES_SOUGHT(I)
                      WRITE(ERR,*) KEEPTEXT(:lnblnk1(KEEPTEXT)), '<--COM
     *MA INDICATES ANOTHER INPUT'
                      WRITE(ERR,*) 'SEARCHED NEXT LINE: ', TEXT(:lnblnk1
     *                (TEXT))
                      WRITE(ERR,*) 'BUT NO OTHER INPUT WAS DETECTED'
                      ERROR_FLAG=1
                      ERROR_FLAGS(I)=1
                    END IF
                  END IF
2891            CONTINUE
2892            CONTINUE
              GO TO 2861
2862          CONTINUE
            ELSE
              GO TO2782
            END IF
            IVAL=IVAL+1
          GO TO 2781
2782      CONTINUE
          IF (((NVALUE(I).NE.0).AND.(NVALUE(I).NE.IVAL))) THEN
            IF (( error_level .GT. 0 )) THEN
              WRITE (ERR,*) '*******************ERROR*******************
     *'
              WRITE (ERR,*) 'VALUE SOUGHT: ', VALUES_SOUGHT(I)
              WRITE (ERR,*) 'ASKED FOR', NVALUE(I),' INPUT(S)'
              WRITE (ERR,*) 'HOWEVER,', IVAL, ' WERE DETECTED'
            END IF
            ERROR_FLAG=1
            ERROR_FLAGS(I)=1
          ELSE
            NVALUE(I)=IVAL
          END IF
        END IF
        goto 1620
1660    IF (( error_level .GT. 0 )) THEN
          WRITE (ERR,*) '******************ERROR***********************'
          WRITE (ERR,*) 'END OF FILE REACHED BUT VALUE SOUGHT NOT FOUND'
          WRITE (ERR,*) 'PROBABLY A MISSING/MISSPELLED END DELIMETER'
          WRITE (ERR,*) 'VALUE SOUGHT: >>', VALUES_SOUGHT(I)(:lnblnk1(VA
     *    LUES_SOUGHT(I))),'<<'
          WRITE (ERR,*) 'END DELIMETER: >>', DELIM_END(:lnblnk1(DELIM_EN
     *    D)),'<<'
        END IF
        ERROR_FLAG=1
        ERROR_FLAGS(I)=1
        goto 1620
1680    IF (( error_level .GT. 0 )) THEN
          WRITE (ERR,*) '******************ERROR***********************'
          WRITE (ERR,*) 'END OF FILE REACHED BUT VALUE SOUGHT NOT FOUND'
          WRITE (ERR,*) 'PROBABLY A MISSING/MISSPELLED START DELIMETER'
          WRITE (ERR,*) 'VALUE SOUGHT: >>', VALUES_SOUGHT(I)(:lnblnk1(VA
     *    LUES_SOUGHT(I))),'<<'
          WRITE (ERR,*) 'START DELIMETER: >>', DELIM_START(:lnblnk1(DELI
     *    M_START)),'<<'
        END IF
        ERROR_FLAG=1
        ERROR_FLAGS(I)=1
        goto 1620
1820    IF (( error_level .GT. 0 )) THEN
          WRITE (ERR,*) '***************ERROR***************'
          IF ((IVAL.GT.1)) THEN
            J=IVAL
          ELSE
            J=1
          END IF
          WRITE (ERR,*) 'ERROR READING VALUE SOUGHT: ', VALUES_SOUGHT(I)
          WRITE (ERR,*) 'LINE #',LINE
          WRITE (ERR,*) 'COULD NOT READ THE VALUE!!'
          WRITE (ERR,*) 'SHOULD BE AN INTEGER OR A REAL...'
          WRITE (ERR,*) 'IS THERE AN EXTRA COMMA AT THE END OF YOUR INPU
     *T?'
        END IF
        ERROR_FLAG=1
        ERROR_FLAGS(I)=1
        GOTO 1620
1970    IF (( error_level .GT. 0 )) THEN
          WRITE (ERR,*) '***************ERROR***************'
          WRITE (ERR,*) 'ERROR READING VALUE SOUGHT: ', VALUES_SOUGHT(I)
          WRITE (ERR,*) 'LINE #',LINE
          WRITE (ERR,*) 'COULD NOT READ THE STRING !!'
        END IF
        ERROR_FLAG=1
        ERROR_FLAGS(I)=1
1620    CONTINUE
2561  CONTINUE
2562  CONTINUE
      RETURN
1670  WRITE (ERR,*) '***************ERROR***************'
      WRITE (ERR,*) 'ERROR READING TEXT ', TEXT,' ON LINE ',LINE
      goto 2120
2120  CONTINUE
      ERROR_FLAG=1
      ERROR_FLAGS(I)=1
      RETURN
      entry get_input_plus_set_error_level(the_level)
      error_level = the_level
      return
      END
      subroutine get_media_inputs(ounit)
      implicit none
      integer*4 ounit
      COMMON/GetInput/ ALLOWED_INPUTS(100,0:5),   VALUES_SOUGHT(100),  C
     *HAR_VALUE(100,100),  VALUE(100,100),  DEFAULT(100),  VALUE_MIN(100
     *),  VALUE_MAX(100),  NVALUE(100),  TYPE(100),      ERROR_FLAGS(100
     *),   i_errors,  NMIN, NMAX,   ERROR_FLAG,  DELIMETER
      character ALLOWED_INPUTS*64,VALUES_SOUGHT*64, CHAR_VALUE*256,DELIM
     *ETER*64
      real*8 VALUE,DEFAULT,VALUE_MIN,VALUE_MAX
      integer*4 NVALUE,TYPE,NMIN,NMAX,ERROR_FLAG,ERROR_FLAGS,i_errors
      COMMON/BOUNDS/ECUT(1),PCUT(1),VACDST
      real*8 ECUT,  PCUT,  VACDST
      common/ET_control/ smaxir(1),estepe,ximax,  skindepth_for_bca,tran
     *sport_algorithm, bca_algorithm,exact_bca,spin_effects
      real*8 smaxir,  estepe,  ximax,      skindepth_for_bca
      integer*4 transport_algorithm, bca_algorithm
      logical exact_bca,  spin_effects
      COMMON/EDGE/binding_energies(30,100), interaction_prob(6,100), rel
     *axation_prob(39,100), edge_energies(16,100), edge_number(100), edg
     *e_a(16,100), edge_b(16,100), edge_c(16,100), edge_d(16,100), IEDGF
     *L(1),IPHTER(1)
      real*8 binding_energies,  interaction_prob,    relaxation_prob,  e
     *dge_energies,  edge_a,edge_b,edge_c,edge_d
      integer*2 IEDGFL,  IPHTER
      integer*4 edge_number
      common/compton_data/ iz_array(1538),  be_array(1538),  Jo_array(15
     *38),  erfJo_array(1538),   ne_array(1538),  shn_array(1538),
     *shell_array(200,1), eno_array(200,1), eno_atbin_array(200,1), n_sh
     *ell(1), radc_flag,  ibcmp(1)
      integer*4 iz_array,ne_array,shn_array,eno_atbin_array, shell_array
     *,n_shell,radc_flag
      real*8 be_array,Jo_array,erfJo_array,eno_array
      integer*2 ibcmp
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      COMMON/MISC/  DUNIT,KMPI,KMPO,RHOR(1),MED(1),IRAYLR(1),IPHOTONUCR(
     *1)
      real*8 DUNIT,  RHOR
      integer*4 KMPI,  KMPO
      integer*2 MED,  IRAYLR,  IPHOTONUCR
      COMMON/BREMPR/ DL1(8,1),DL2(8,1),DL3(8,1),DL4(8,1),DL5(8,1),DL6(8,
     *1), ALPHI(2,1),BPAR(2,1),DELPOS(2,1), WA(1,50),PZ(1,50),ZELEM(1,50
     *),RHOZ(1,50), PWR2I(50), DELCM(1),ZBRANG(1),LZBRANG(1),NNE(1), IBR
     *DST,IPRDST,ibr_nist,pair_nrc,itriplet, ASYM(1,50,2)
      CHARACTER*4 ASYM
      real*8 DL1,DL2,DL3,DL4,DL5,DL6,   ALPHI,  BPAR,  DELPOS,  WA,  PZ,
     *  ZELEM,  RHOZ,  PWR2I,  DELCM,  ZBRANG,  LZBRANG
      integer*4 NNE,  IBRDST,  IPRDST,  ibr_nist,  itriplet,  pair_nrc
      common/eii_data/ eii_xsection_a( 10000),  eii_xsection_b( 10000),
     * eii_cons(1), eii_a(40),  eii_b(40),  eii_L_factor,  eii_z(40),  e
     *ii_sh(40),  eii_nshells(100),  eii_nsh(1),  eii_first(1,50),  eii_
     *no(1,50),  eii_flag
      real*8 eii_xsection_a,eii_xsection_b,eii_a,eii_b,eii_cons,eii_L_fa
     *ctor
      integer*4 eii_z,eii_sh,eii_nshells
      integer*4 eii_first,eii_no
      integer*4 eii_elements,eii_flag,eii_nsh
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      COMMON/rayleigh_inputs/iray_ff_media(1),iray_ff_file(1)
      character*24 iray_ff_media
      character*128 iray_ff_file
      common/emf_inputs/ExIN,EyIN,EzIN,  EMLMTIN,  BxIN, ByIN, BzIN,  Bx
     *, By, Bz,  Bx_new, By_new, Bz_new,  emfield_on
      real*8 ExIN,EyIN,EzIN, EMLMTIN, BxIN,ByIN,BzIN, Bx,By,Bz, Bx_new,B
     *y_new,Bz_new
      logical emfield_on
      common/x_options/eadl_relax,  mcdf_pe_xsections
      logical eadl_relax, mcdf_pe_xsections
      COMMON/MEDINP/inpdensity_file(1),inpasym(1,50), inpstrn(24,1),pz4(
     *1,50), rhoz4(1,50),wa4(1,50),inpgasp(1)
      character*256 inpdensity_file
      CHARACTER*4 inpasym,inpstrn
      real*4 pz4,rhoz4,wa4,inpgasp
      COMMON/ELECIN/ esig_e(1),psig_e(1), esige_max, psige_max, range_ep
     *(0:1,500,1), E_array(500,1), etae_ms0(500,1),etae_ms1(500,1),etap_
     *ms0(500,1),etap_ms1(500,1),q1ce_ms0(500,1),q1ce_ms1(500,1),q1cp_ms
     *0(500,1),q1cp_ms1(500,1),q2ce_ms0(500,1),q2ce_ms1(500,1),q2cp_ms0(
     *500,1),q2cp_ms1(500,1),blcce0(500,1),blcce1(500,1), EKE0(1),EKE1(1
     *), XR0(1),TEFF0(1),BLCC(1),XCC(1), ESIG0(500,1),ESIG1(500,1),PSIG0
     *(500,1),PSIG1(500,1),EDEDX0(500,1),EDEDX1(500,1),PDEDX0(500,1),PDE
     *DX1(500,1),EBR10(500,1),EBR11(500,1),PBR10(500,1),PBR11(500,1),PBR
     *20(500,1),PBR21(500,1),TMXS0(500,1),TMXS1(500,1), expeke1(1), IUNR
     *ST(1),EPSTFL(1),IAPRIM(1), sig_ismonotone(0:1,1)
      real*8 esig_e,   psig_e,   esige_max,  psige_max,  range_ep,  E_ar
     *ray,  etae_ms0,etae_ms1,  etap_ms0,etap_ms1,  q1ce_ms0,q1ce_ms1,
     *q1cp_ms0,q1cp_ms1,  q2ce_ms0,q2ce_ms1,  q2cp_ms0,q2cp_ms1,  blcce0
     *,blcce1,   expeke1,  EKE0,EKE1, XR0,  TEFF0,  BLCC,  XCC,  ESIG0,E
     *SIG1,  PSIG0,PSIG1,  EDEDX0,EDEDX1,  PDEDX0,PDEDX1,  EBR10,EBR11,
     * PBR10,PBR11,  PBR20,PBR21,  TMXS0,TMXS1
      integer*4 IUNRST,  EPSTFL,  IAPRIM
      logical sig_ismonotone
      COMMON/THRESH/RMT2,RMSQ, AP(1),AE(1),UP(1),UE(1),TE(1),THMOLL(1)
      real*8 RMT2,  RMSQ,  AP,  AE,  UP,  UE,  TE,  THMOLL
      COMMON/ELEMTB/NET,ITBL(100),WATBL(100),RHOTBL(100),ASYMT(100)
      integer*4 NET
      real*4 ITBL,WATBL,RHOTBL
      CHARACTER*4 ASYMT
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      integer*4 ival,ival_media,ival_medfile,i,j,k,ival_ae,ival_ue,ival_
     *ap,ival_up, ival_rho,ival_elements,ival_rhoz,ival_iunrst,ival_iapr
     *im,ival_gasp, ival_pz,ival_sterncid, ival_densityfile,medfile_erro
     *r,ival_outfile, egs_open_file,lnblnk1,i_medfile,egs_get_unit,i_med
     *err,mindex,eindex, i_density,i01,length,i_outfile
      real*8 ecut_min, pcut_min
      logical medfile_specified,densityfile_specified,elements_specified
     *, outfile_specified(1)
      logical iunrst_specified,stern_specified,iaprim_specified, gasp_sp
     *ecified,rho_specified,start_delim_found,end_delim_found, spec_by_p
     *z,spec_by_rhoz,df_if_elem_mismatch(1), df_if_rho_mismatch(1)
      logical ex
      integer*4 CURSOR,Kconvert
      real*4 ZTBL
      real*8 EKE,ELKE,TMXSO,DEDXE,DEDXP,EFRACT,SIGE,SIGP,BREME,BREMP,ETA
     *B(16), EIE,PLOTE(300),PLOTEM(300),PLOTEEN(300), PLOTEMP(300), PLOT
     *EMS(300)
      integer*4 IPLOTE,IFLAG1,IFLAG2,LELKE
      CHARACTER*60 GRAPHTITLE,XAXIS,YAXISPcom,YAXISPmfp,YAXISE,YAXISEmfp
     *, SUBTITLE,SERIES
      DATA ETAB/1.,1.25,1.5,1.75,2.,2.5,3.,3.5,4.,4.5,5.,5.5,6.,7.,8.,9.
     */
      character*24 medium_name,med_tmp,sterncid_tmp
      character*256 density_file,material_file,tmp_string, spoutput_file
     *(1)
      character*80 text_string, text_save, title
      character*40 delim_start,delim_end
      character*1 blank
      character*512 toUpper
      integer*4 nne_tmp,iaprim_tmp,epstfl_tmp,iunrst_tmp
      real*8 rho_tmp,rhoz_tmp(50),z_tmp(50),pz_tmp(50),ae_tmp,ap_tmp, ue
     *_tmp,up_tmp,gasp_tmp
      CHARACTER*4 asym_tmp(50)
      integer*4 nepst_df,nne_df
      real*8 iev_df,rho_df,z_df(50),rhoz_df(50),rhoz_tot
      CHARACTER*4 asym_df(50)
      data blank/' '/
      save medfile_specified,material_file,df_if_elem_mismatch,df_if_rho
     *_mismatch, spoutput_file,outfile_specified
      call get_input_set_error_level(0)
      call get_input_plus_set_error_level(0)
      IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THEN
        i_mederr=17
        i_mederr=egs_open_file(i_mederr,0,1,'.mederr')
      END IF
      ecut_min=999.
      pcut_min=999.
      DO 2921 i=1,1
        IF((ecut(i).LT.ecut_min))ecut_min=ecut(i)
        IF((pcut(i).LT.pcut_min))pcut_min=pcut(i)
2921  CONTINUE
2922  CONTINUE
      delimeter = 'MEDIA DEFINITION'
      ival = 0
      ival = ival + 1
      ival_medfile = ival
      values_sought(ival) = 'material data file'
      nvalue(ival) = 1
      type(ival) = 2
      Nmin = ival_medfile
      Nmax = ival_medfile
      CALL GET_INPUT
      IF ((error_flags(ival_medfile).EQ.0)) THEN
        material_file=char_value(ival_medfile,1)
        medfile_specified=.true.
        i_medfile=17
        i_medfile=egs_get_unit(i_medfile)
        IF ((i_medfile .LT. 1)) THEN
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,'(a)') 'Error: Failed to get available fortran uni
     *t for', ' medium data file.'
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        END IF
        open(i_medfile,file=material_file,status='old',err=2930)
        medfile_specified=.true.
      ELSE
        IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THEN
          write(i_mederr,*)' Warning: material data file not supplied.'
        END IF
        IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THEN
          write(i_mederr,*)' Thus, you must define media explicitly in i
     *nput file'
        END IF
        IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THEN
          write(i_mederr,*)' or via density correction file.'
        END IF
        medfile_specified=.false.
      END IF
      ival = ival + 1
      ival_ae = ival
      values_sought(ival) = 'ae'
      nvalue(ival) = 1
      type(ival) = 1
      value_min(ival) = 0
      value_max(ival) = 999.
      default(ival) = ecut_min
      ival = ival + 1
      ival_ap = ival
      values_sought(ival) = 'ap'
      nvalue(ival) = 1
      type(ival) = 1
      value_min(ival) = 0
      value_max(ival) = 999.
      default(ival) = pcut_min
      ival = ival + 1
      ival_ue = ival
      values_sought(ival) = 'ue'
      nvalue(ival) = 1
      type(ival) = 1
      value_min(ival) = 0
      value_max(ival) = 999.
      default(ival) = 50 + prm
      ival = ival + 1
      ival_up = ival
      values_sought(ival) = 'up'
      nvalue(ival) = 1
      type(ival) = 1
      value_min(ival) = 0
      value_max(ival) = 999.
      default(ival) = 50.0
      Nmin=ival_ae
      Nmax=ival_up
      CALL GET_INPUT
      IF ((error_flags(ival_ae).EQ.0)) THEN
        ae_tmp=value(ival_ae,1)
      ELSE
        IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THEN
          write(i_mederr,*)' Warning: AE for media not supplied.  Will u
     *se min. ECUT.'
        END IF
        ae_tmp=ecut_min
      END IF
      IF ((error_flags(ival_ap).EQ.0)) THEN
        ap_tmp=value(ival_ap,1)
      ELSE
        IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THEN
          write(i_mederr,*)' Warning: AP for media not supplied.  Will u
     *se min. PCUT.'
        END IF
        ap_tmp=pcut_min
      END IF
      IF ((error_flags(ival_ue).EQ.0)) THEN
        ue_tmp=value(ival_ue,1)
      ELSE
        IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THEN
          write(i_mederr,*)' Warning: UE for media not supplied.  Will u
     *se                     50.5109989461 MeV'
        END IF
        ue_tmp=50 + prm
      END IF
      IF ((error_flags(ival_up).EQ.0)) THEN
        up_tmp=value(ival_up,1)
      ELSE
        IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THEN
          write(i_mederr,*)' Warning: UP for media not supplied.  Will u
     *se 50.0 MeV'
        END IF
        up_tmp=50.
      END IF
      IF ((ue_tmp.LE.ae_tmp)) THEN
        IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THEN
          write(i_mederr,*)' Error: UE <= AE.  Adjust value(s) and try a
     *gain.'
        END IF
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,'(a)') ' Error: UE <= AE.  Adjust value(s) and try a
     *gain.'
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      IF ((up_tmp.LE.ap_tmp)) THEN
        IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THEN
          write(i_mederr,*)' Error: UP <= AP.  Adjust value(s) and try a
     *gain.'
        END IF
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,'(a)') ' Error: UP <= AP.  Adjust value(s) and try a
     *gain.'
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      DO 2941 i=1,NMED
        DO 2951 j=1,24
          medium_name(j:j)=media(j,i)
2951    CONTINUE
2952    CONTINUE
        elements_specified=.false.
        rho_specified=.false.
        densityfile_specified=.false.
        stern_specified=.false.
        iunrst_specified=.false.
        iaprim_specified=.false.
        gasp_specified=.false.
        spec_by_rhoz=.false.
        spec_by_pz=.false.
        df_if_elem_mismatch(i)=.false.
        df_if_rho_mismatch(i)=.false.
        sterncid_tmp=medium_name
        gasp_tmp=0.0
        iunrst_tmp=0
        iaprim_tmp=0
        epstfl_tmp=0
        density_file=' '
        IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THEN
          write(i_mederr,*)' '
        END IF
        IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THEN
          write(i_mederr,*)' For medium: ',medium_name
        END IF
        delimeter=medium_name(:lnblnk1(medium_name))
        ival=0
        ival=ival+1
        ival_elements=ival
        values_sought(ival) = 'elements'
        type(ival) = 2
        nvalue(ival) = 0
        nmin=ival_elements
        nmax=ival_elements
        CALL GET_INPUT
        IF ((error_flags(ival_elements).EQ.0)) THEN
          DO 2961 j=1,nvalue(ival_elements)
            DO 2971 Kconvert=1,lnblnk1(char_value(ival_elements,j))
              CURSOR=ICHAR(char_value(ival_elements,j)(Kconvert:Kconvert
     *        ))
              IF (((CURSOR.GE.97).AND.(CURSOR.LE.122))) THEN
                CURSOR=CURSOR-32
                char_value(ival_elements,j)(Kconvert:Kconvert)=CHAR(CURS
     *          OR)
              END IF
2971        CONTINUE
2972        CONTINUE
2961      CONTINUE
2962      CONTINUE
          ival=ival+1
          ival_pz=ival
          nne_tmp=nvalue(ival_elements)
          values_sought(ival)='number of atoms'
          type(ival)=0
          nvalue(ival)=nne_tmp
          nmin=ival_pz
          nmax=ival_pz
          CALL GET_INPUT
          IF ((nne_tmp.GT.1 .AND. error_flags(ival_pz).EQ.0)) THEN
            DO 2981 j=1,nne_tmp
              asym_tmp(j)=char_value(ival_elements,j)
              pz_tmp(j)=value(ival_pz,j)
2981        CONTINUE
2982        CONTINUE
            elements_specified=.true.
            spec_by_pz=.true.
          ELSE
            ival=ival+1
            ival_rhoz=ival
            values_sought(ival)='mass fractions'
            type(ival)=1
            nvalue(ival)=nne_tmp
            nmin=ival_rhoz
            nmax=ival_rhoz
            IF ((nne_tmp.EQ.1)) THEN
              value_min(ival)=0.0
              value_max(ival)=1.e15
              default(ival)=1.
            END IF
            CALL GET_INPUT
            IF ((error_flags(ival_rhoz).EQ.0)) THEN
              DO 2991 j=1,nne_tmp
                asym_tmp(j)=char_value(ival_elements,j)
                rhoz_tmp(j)=value(ival_rhoz,j)
2991          CONTINUE
2992          CONTINUE
              elements_specified=.true.
              spec_by_rhoz=.true.
            END IF
          END IF
          IF ((nne_tmp.EQ.1 .AND. .NOT.elements_specified)) THEN
            asym_tmp(1)=char_value(ival_elements,1)
            pz_tmp(1)=1
            elements_specified=.true.
            spec_by_pz=.true.
          END IF
          IF ((elements_specified)) THEN
            IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THE
     *      N
              write(i_mederr,*)' Composition specified in .egsinp file.'
            END IF
          END IF
        END IF
        ival=ival+1
        ival_rho=ival
        values_sought(ival) = 'rho'
        type(ival)=1
        nvalue(ival)=1
        value_min(ival)=0.
        value_max(ival)=1e15
        default(ival)=1.0
        nmin=ival_rho
        nmax=ival_rho
        CALL GET_INPUT
        IF ((error_flags(ival_rho).EQ.0)) THEN
          rho_tmp=value(ival_rho,1)
          rho_specified=.true.
          IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THEN
            write(i_mederr,*)' Rho specified in .egsinp file.'
          END IF
        END IF
        ival=ival+1
        ival_sterncid=ival
        values_sought(ival)='sterncid'
        type(ival)=2
        nvalue(ival)=1
        nmin=ival_sterncid
        nmax=ival_sterncid
        CALL GET_INPUT
        IF ((error_flags(ival_sterncid).EQ.0)) THEN
          sterncid_tmp=char_value(ival_sterncid,1)
          stern_specified=.true.
          IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THEN
            write(i_mederr,*)' STERNCID specified in .egsinp file.'
          END IF
        END IF
        ival=ival+1
        ival_iunrst=ival
        values_sought(ival)='stopping powers'
        type(ival)=3
        nvalue(ival)=1
        allowed_inputs(ival,0)='restricted total'
        allowed_inputs(ival,1)='unrestricted collision'
        allowed_inputs(ival,2)='unrestricted collision and radiative'
        allowed_inputs(ival,3)='unrestricted collision and restricted ra
     *diative'
        allowed_inputs(ival,4)='restricted collision and unrestricted ra
     *diative'
        allowed_inputs(ival,5)='unrestricted radiative'
        nmin=ival_iunrst
        nmax=ival_iunrst
        CALL GET_INPUT
        IF ((error_flags(ival_iunrst).EQ.0)) THEN
          iunrst_tmp=value(ival_iunrst,1)
          iunrst_specified=.true.
          IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THEN
            write(i_mederr,*)' IUNRST specified in .egsinp file.'
          END IF
        END IF
        ival=ival+1
        ival_iaprim=ival
        values_sought(ival)='bremsstrahlung correction'
        type(ival)=3
        nvalue(ival)=1
        allowed_inputs(ival,0)='KM'
        allowed_inputs(ival,1)='NRC'
        allowed_inputs(ival,2)='none'
        nmin=ival_iaprim
        nmax=ival_iaprim
        CALL GET_INPUT
        IF ((error_flags(ival_iaprim).EQ.0)) THEN
          iaprim_tmp=value(ival_iaprim,1)
          iaprim_specified=.true.
          IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THEN
            write(i_mederr,*)' IAPRIM specified in .egsinp file.'
          END IF
        END IF
        ival=ival+1
        ival_gasp=ival
        values_sought(ival)='gas pressure'
        type(ival)=1
        nvalue(ival)=1
        value_min(ival)=0.
        value_max(ival)=1e15
        default(ival)=0.0
        nmin=ival_gasp
        nmax=ival_gasp
        CALL GET_INPUT
        IF ((error_flags(ival_gasp).EQ.0)) THEN
          gasp_tmp=value(ival_gasp,1)
          gasp_specified=.true.
          IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THEN
            write(i_mederr,*)' GASP specified in .egsinp file.'
          END IF
        END IF
        ival=ival+1
        ival_densityfile=ival
        values_sought(ival)='density correction file'
        type(ival) = 2
        nvalue(ival)=1
        nmin=ival_densityfile
        nmax=ival_densityfile
        CALL GET_INPUT
        IF ((error_flags(ival_densityfile).EQ.0)) THEN
          density_file=char_value(ival_densityfile,1)
          densityfile_specified=.true.
          IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THEN
            write(i_mederr,*)' Density correction file specified in .egs
     *inp file.'
          END IF
        END IF
        ival = ival+1
        ival_outfile = ival
        values_sought(ival) = 'e- stopping power output file'
        type(ival) = 2
        nvalue(ival) =1
        nmin=ival_outfile
        nmax=ival_outfile
        CALL GET_INPUT
        IF ((error_flags(ival_outfile).EQ.0)) THEN
          spoutput_file(i)=char_value(ival_outfile,1)
          outfile_specified(i)=.true.
          IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THEN
            write(i_mederr,*)' e- stopping powers will be output to ', s
     *      poutput_file(i)
          END IF
        ELSE
          outfile_specified(i)=.false.
        END IF
        IF ((medfile_specified .AND. (.NOT.elements_specified .OR. .NOT.
     *  rho_specified .OR. .NOT.iunrst_specified .OR. .NOT.iaprim_specif
     *  ied .OR. .NOT.gasp_specified .OR. .NOT.stern_specified .OR. .NOT
     *  .densityfile_specified))) THEN
          rewind(i_medfile)
          start_delim_found=.false.
          end_delim_found=.false.
3001      IF((.NOT.(.NOT.start_delim_found)).AND.(.NOT.(.NOT.end_delim_f
     *    ound)))GO TO 3002
            read(i_medfile,'(a)',end=3010)text_string
            text_save=text_string
            text_string=toUpper(text_string(:lnblnk1(text_string)))
            mindex=index(text_string,'MEDIUM')
            eindex=index(text_string,'=')
            IF ((mindex.GT.0 .AND. eindex.GT.mindex)) THEN
              text_string=text_save(eindex+1:)
              text_string=text_string(:lnblnk1(text_string))
              length = len(text_string)
3021          IF(index(text_string,blank).NE.1)GO TO 3022
                IF (( length .GE. 2 )) THEN
                  text_string=text_string(2:)
                ELSE
                  GO TO3022
                END IF
                length = length - 1
              GO TO 3021
3022          CONTINUE
              IF ((text_string.EQ.medium_name)) THEN
                delim_start=text_save
                start_delim_found=.true.
              ELSE IF((start_delim_found)) THEN
                delim_end=text_save
                end_delim_found=.true.
              END IF
            END IF
          GO TO 3001
3002      CONTINUE
3010      IF ((.NOT.start_delim_found)) THEN
            IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THE
     *      N
              write(i_mederr,*)' Warning: Data for ',medium_name,' not f
     *ound'
            END IF
            IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THE
     *      N
              write(i_mederr,*)' in material data file.'
            END IF
          ELSE
            IF ((.NOT.end_delim_found)) THEN
              delim_end='NONE'
            END IF
            ival=0
            IF ((.NOT.elements_specified)) THEN
              ival=ival+1
              ival_elements=ival
              values_sought(ival) = 'elements'
              type(ival) = 2
              nvalue(ival) = 0
              nmin=ival_elements
              nmax=ival_elements
              CALL GET_INPUT_PLUS(i_medfile,delim_start,delim_end)
              IF ((error_flags(ival_elements).EQ.0)) THEN
                ival=ival+1
                ival_pz=ival
                nne_tmp=nvalue(ival_elements)
                values_sought(ival)='number of atoms'
                type(ival)=0
                nvalue(ival)=nne_tmp
                nmin=ival_pz
                nmax=ival_pz
                CALL GET_INPUT_PLUS(i_medfile,delim_start,delim_end)
                IF ((nne_tmp.GT.1 .AND. error_flags(ival_pz).EQ.0)) THEN
                  DO 3031 j=1,nne_tmp
                    asym_tmp(j)=char_value(ival_elements,j)
                    pz_tmp(j)=value(ival_pz,j)
3031              CONTINUE
3032              CONTINUE
                  elements_specified=.true.
                  spec_by_pz=.true.
                ELSE
                  ival=ival+1
                  ival_rhoz=ival
                  values_sought(ival)='mass fractions'
                  type(ival)=1
                  nvalue(ival)=nne_tmp
                  nmin=ival_rhoz
                  nmax=ival_rhoz
                  IF ((nne_tmp.EQ.1)) THEN
                    value_min(ival)=0.0
                    value_max(ival)=1.e15
                    default(ival)=1.
                  END IF
                  CALL GET_INPUT_PLUS(i_medfile,delim_start,delim_end)
                  IF ((error_flags(ival_rhoz).EQ.0)) THEN
                    DO 3041 j=1,nne_tmp
                      asym_tmp(j)=char_value(ival_elements,j)
                      rhoz_tmp(j)=value(ival_rhoz,j)
3041                CONTINUE
3042                CONTINUE
                    elements_specified=.true.
                    spec_by_rhoz=.true.
                  END IF
                END IF
                IF ((nne_tmp.EQ.1 .AND. .NOT.elements_specified)) THEN
                  asym_tmp(1)=char_value(ival_elements,1)
                  pz_tmp(1)=1
                  elements_specified=.true.
                  spec_by_pz=.true.
                END IF
                IF ((elements_specified)) THEN
                  IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel
     *            )) THEN
                    write(i_mederr,*)' Composition specified in material
     * data file'
                  END IF
                END IF
              END IF
            END IF
            IF ((.NOT.rho_specified)) THEN
              ival=ival+1
              ival_rho=ival
              values_sought(ival) = 'rho'
              type(ival)=1
              nvalue(ival)=1
              value_min(ival)=0.
              value_max(ival)=1e15
              default(ival)=1.0
              nmin=ival_rho
              nmax=ival_rho
              CALL GET_INPUT_PLUS(i_medfile,delim_start,delim_end)
              IF ((error_flags(ival_rho).EQ.0)) THEN
                rho_tmp=value(ival_rho,1)
                rho_specified=.true.
                IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel))
     *           THEN
                  write(i_mederr,*)' Rho specified in material data file
     *'
                END IF
              END IF
            END IF
            IF ((.NOT.stern_specified)) THEN
              ival=ival+1
              ival_sterncid=ival
              values_sought(ival)='sterncid'
              type(ival)=2
              nvalue(ival)=1
              nmin=ival_sterncid
              nmax=ival_sterncid
              CALL GET_INPUT_PLUS(i_medfile,delim_start,delim_end)
              IF ((error_flags(ival_sterncid).EQ.0)) THEN
                sterncid_tmp=char_value(ival_sterncid,1)
                stern_specified=.true.
                IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel))
     *           THEN
                  write(i_mederr,*)' STERNCID specified in material data
     * file'
                END IF
              END IF
            END IF
            IF ((.NOT.iunrst_specified)) THEN
              ival=ival+1
              ival_iunrst=ival
              values_sought(ival)='stopping powers'
              type(ival)=3
              nvalue(ival)=1
              allowed_inputs(ival,0)='restricted total'
              allowed_inputs(ival,1)='unrestricted collision'
              allowed_inputs(ival,2)='unrestricted collision and radiati
     *ve'
              allowed_inputs(ival,3)= 'unrestricted collision and restri
     *cted radiative'
              allowed_inputs(ival,4)= 'restricted collision and unrestri
     *cted radiative'
              allowed_inputs(ival,5)='unrestricted radiative'
              nmin=ival_iunrst
              nmax=ival_iunrst
              CALL GET_INPUT_PLUS(i_medfile,delim_start,delim_end)
              IF ((error_flags(ival_iunrst).EQ.0)) THEN
                iunrst_tmp=value(ival_iunrst,1)
                iunrst_specified=.true.
                IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel))
     *           THEN
                  write(i_mederr,*)' IUNRST specified in material data f
     *ile'
                END IF
              END IF
            END IF
            IF ((.NOT.iaprim_specified)) THEN
              ival=ival+1
              ival_iaprim=ival
              values_sought(ival)='bremsstrahlung correction'
              type(ival)=3
              nvalue(ival)=1
              allowed_inputs(ival,0)='KM'
              allowed_inputs(ival,1)='NRC'
              allowed_inputs(ival,2)='none'
              nmin=ival_iaprim
              nmax=ival_iaprim
              CALL GET_INPUT_PLUS(i_medfile,delim_start,delim_end)
              IF ((error_flags(ival_iaprim).EQ.0)) THEN
                iaprim_tmp=value(ival_iaprim,1)
                iaprim_specified=.true.
                IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel))
     *           THEN
                  write(i_mederr,*)' IAPRIM specified in material data f
     *ile'
                END IF
              END IF
            END IF
            IF ((.NOT.gasp_specified)) THEN
              ival=ival+1
              ival_gasp=ival
              values_sought(ival)='gas pressure'
              type(ival)=1
              nvalue(ival)=1
              value_min(ival)=0.
              value_max(ival)=1.e15
              default(ival)=0.
              nmin=ival_gasp
              nmax=ival_gasp
              CALL GET_INPUT_PLUS(i_medfile,delim_start,delim_end)
              IF ((error_flags(ival_gasp).EQ.0)) THEN
                gasp_tmp=value(ival_gasp,1)
                gasp_specified=.true.
                IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel))
     *           THEN
                  write(i_mederr,*)' GASP specified in material data fil
     *e'
                END IF
              END IF
            END IF
            IF ((.NOT.densityfile_specified)) THEN
              ival=ival+1
              ival_densityfile=ival
              values_sought(ival)='density correction file'
              type(ival) = 2
              nvalue(ival)=1
              nmin=ival_densityfile
              nmax=ival_densityfile
              CALL GET_INPUT_PLUS(i_medfile,delim_start,delim_end)
              IF ((error_flags(ival_densityfile).EQ.0)) THEN
                density_file=char_value(ival_densityfile,1)
                densityfile_specified=.true.
                IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel))
     *           THEN
                  write(i_mederr,*)' Density correction file specified i
     *n material data file.'
                END IF
              END IF
            END IF
          END IF
        END IF
        IF ((densityfile_specified)) THEN
          write(*,*)' density_file ',density_file
          IF ((index(density_file,'/').GT.0)) THEN
            tmp_string=density_file(:lnblnk1(density_file))
            inquire(file=tmp_string,exist=ex)
            IF ((.NOT.ex)) THEN
              IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) T
     *        HEN
                write(i_mederr,*)' Error: Density correction file ',tmp_
     *          string
              END IF
              IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) T
     *        HEN
                write(i_mederr,*)' cannot be found.'
              END IF
            END IF
          ELSE
            density_file=density_file(:lnblnk1(density_file))//'.density
     *'
            tmp_string=egs_home(:lnblnk1(egs_home)) // 'pegs4' // '/' //
     *       'density_corrections' // '/' // density_file
            inquire(file=tmp_string,exist=ex)
            IF((ex))goto 3050
            tmp_string=egs_home(:lnblnk1(egs_home)) // 'pegs4' // '/' //
     *       'density_corrections' // '/' // 'elements' // '/' // densit
     *      y_file
            inquire(file=tmp_string,exist=ex)
            IF((ex))goto 3050
            tmp_string=egs_home(:lnblnk1(egs_home)) // 'pegs4' // '/' //
     *       'density_corrections' // '/' // 'compounds' // '/' // densi
     *      ty_file
            inquire(file=tmp_string,exist=ex)
            IF((ex))goto 3050
            tmp_string=egs_home(:lnblnk1(egs_home)) // 'pegs4' // '/' //
     *       'density' // '/' // density_file
            inquire(file=tmp_string,exist=ex)
            IF((ex))goto 3050
            tmp_string=hen_house(:lnblnk1(hen_house)) // 'pegs4' // '/'
     *      // 'density_corrections' // '/' // 'elements' // '/' // dens
     *      ity_file
            inquire(file=tmp_string,exist=ex)
            IF((ex))goto 3050
            tmp_string=hen_house(:lnblnk1(hen_house)) // 'pegs4' // '/'
     *      // 'density_corrections' // '/' // 'compounds' // '/' // den
     *      sity_file
            inquire(file=tmp_string,exist=ex)
            IF((ex))goto 3050
            IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THE
     *      N
              write(i_mederr,*)' Error: Density correction file', densit
     *        y_file
            END IF
            IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THE
     *      N
              write(i_mederr,*)' does not exist in'
            END IF
            IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THE
     *      N
              write(i_mederr,*)' $EGS_HOME/pegs4/density_corrections, '
            END IF
            IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THE
     *      N
              write(i_mederr,*)' $EGS_HOME/pegs4/density_corrections/ele
     *ments, '
            END IF
            IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THE
     *      N
              write(i_mederr,*)' $EGS_HOME/pegs4/density_corrections/com
     *pounds, '
            END IF
            IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THE
     *      N
              write(i_mederr,*)' $EGS_HOME/pegs4/density, '
            END IF
            IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THE
     *      N
              write(i_mederr,*)' $HEN_HOUSE/pegs4/density_corrections/el
     *ements or '
            END IF
            IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THE
     *      N
              write(i_mederr,*)' $HEN_HOUSE/pegs4/density_corrections/co
     *mpounds.'
            END IF
3050        CONTINUE
          END IF
        END IF
        IF ((densityfile_specified)) THEN
          i_density=19
          i_density=egs_get_unit(i_density)
          IF ((i_density .LT. 1)) THEN
            write(i_log,'(/a)') '***************** Error: '
            write(i_log,'(a)') 'Error: Failed to get available fortran u
     *nit for', ' density correction file.'
            write(i_log,'(/a)') '***************** Quiting now.'
            call exit(1)
          END IF
          open(i_density,file=tmp_string,status='old',err=3060)
          density_file=tmp_string
          densityfile_specified=.true.
          epstfl_tmp=1
          read(i_density,'(a)')title
          read(i_density,*)nepst_df,iev_df,rho_df,nne_df
          read(i_density,*)(z_df(j),rhoz_df(j),j=1,nne_df)
          DO 3071 j=1,nne_df
            i01=z_df(j)
            asym_df(j)=ASYMT(i01)
3071      CONTINUE
3072      CONTINUE
          IF ((elements_specified)) THEN
            IF ((nne_tmp.NE.nne_df)) THEN
              df_if_elem_mismatch(i)=.true.
            ELSE
              rhoz_tot=0.
              DO 3081 j=1,nne_tmp
                IF ((spec_by_pz)) THEN
                  i01=ZTBL(asym_tmp(j))
                  rhoz_tmp(j)=pz_tmp(j)*WATBL(i01)
                END IF
                rhoz_tot=rhoz_tot+rhoz_tmp(j)
3081          CONTINUE
3082          CONTINUE
              DO 3091 j=1,nne_df
                DO 3101 k=1,nne_tmp
                  IF ((asym_df(j).EQ.asym_tmp(k))) THEN
                    IF ((rhoz_df(j).GT.(1+0.01)*rhoz_tmp(k)/rhoz_tot .OR
     *              . rhoz_df(j).LT.(1-0.01)*rhoz_tmp(k)/rhoz_tot)) THEN
                      df_if_elem_mismatch(i)=.true.
                    END IF
                    exit
                  END IF
3101            CONTINUE
3102            CONTINUE
                IF((k.GT.nne_tmp))df_if_elem_mismatch(i)=.true.
                IF ((df_if_elem_mismatch(i))) THEN
                  exit
                END IF
3091          CONTINUE
3092          CONTINUE
            END IF
            IF ((df_if_elem_mismatch(i))) THEN
              IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) T
     *        HEN
                write(i_mederr,*)' Warning: composition specified in den
     *sity correction', ' file is not the same as that'
              END IF
              IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) T
     *        HEN
                write(i_mederr,*)' specified in input or material data f
     *ile.'
              END IF
              IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) T
     *        HEN
                write(i_mederr,*)' Will use the composition specified in
     * the density correction file.'
              END IF
              nne_tmp=nne_df
              DO 3111 j=1,nne_tmp
                z_tmp(j)=z_df(j)
                rhoz_tmp(j)=rhoz_df(j)
                asym_tmp(j)=asym_df(j)
3111          CONTINUE
3112          CONTINUE
              spec_by_rhoz=.true.
            END IF
          ELSE
            IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THE
     *      N
              write(i_mederr,*)' Composition specified in density correc
     *tion file'
            END IF
            nne_tmp=nne_df
            DO 3121 j=1,nne_tmp
              z_tmp(j)=z_df(j)
              rhoz_tmp(j)=rhoz_df(j)
              asym_tmp(j)=asym_df(j)
3121        CONTINUE
3122        CONTINUE
            spec_by_rhoz=.true.
            elements_specified=.true.
          END IF
          IF ((rho_specified)) THEN
            IF ((rho_df.GT.(1+0.01)*rho_tmp .OR. rho_df.LT.(1-0.01)*rho_
     *      tmp)) THEN
              IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) T
     *        HEN
                write(i_mederr,*)' Warning: rho specified in density cor
     *rection', ' file is not the same as that'
              END IF
              IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) T
     *        HEN
                write(i_mederr,*)' specified in input or material data f
     *ile.'
              END IF
              IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) T
     *        HEN
                write(i_mederr,*)' Will use rho as specified in the dens
     *ity correction file.'
              END IF
              rho_tmp=rho_df
              df_if_rho_mismatch(i)=.true.
            END IF
          ELSE
            rho_tmp=rho_df
            rho_specified=.true.
            IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THE
     *      N
              write(i_mederr,*)' Rho specified in density correction fil
     *e'
            END IF
          END IF
          IF ((gasp_specified)) THEN
            IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THE
     *      N
              write(i_mederr,*)' Warning: gas pressure input not require
     *d', ' when using density correction file.  Will set GASP=0.'
            END IF
            gasp_specified=.false.
            gasp_tmp=0.
          END IF
          close(i_density)
        END IF
        IF ((elements_specified .AND. rho_specified)) THEN
          ae(i)=ae_tmp
          ue(i)=ue_tmp
          ap(i)=ap_tmp
          up(i)=up_tmp
          DO 3131 j=1,24
            inpstrn(j,i) = sterncid_tmp(j:j)
3131      CONTINUE
3132      CONTINUE
          nne(i)=nne_tmp
          rho(i)=rho_tmp
          DO 3141 j=1,nne_tmp
            inpasym(i,j)=asym_tmp(j)
            zelem(i,j)=ZTBL(asym_tmp(j))
            i01=zelem(i,j)
            wa(i,j)=WATBL(i01)
            wa4(i,j)=WATBL(i01)
            IF ((spec_by_rhoz)) THEN
              rhoz(i,j)=rhoz_tmp(j)
              rhoz4(i,j)=rhoz_tmp(j)
              pz(i,j)=rhoz(i,j)/wa(i,j)
              pz4(i,j)=rhoz4(i,j)/wa4(i,j)
            ELSE IF((spec_by_pz)) THEN
              pz(i,j)=pz_tmp(j)
              pz4(i,j)=pz_tmp(j)
              rhoz(i,j)=pz(i,j)*wa(i,j)
              rhoz4(i,j)=pz4(i,j)*wa4(i,j)
            END IF
3141      CONTINUE
3142      CONTINUE
          iunrst(i)=iunrst_tmp
          iaprim(i)=iaprim_tmp
          epstfl(i)=epstfl_tmp
          inpgasp(i)=gasp_tmp
          inpdensity_file(i)=density_file
        ELSE
          IF ((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel)) THEN
            write(i_mederr,*)' Error: Medium ',medium_name,' not correct
     *ly defined.'
          END IF
        END IF
2941  CONTINUE
2942  CONTINUE
      IF((medfile_specified))close(i_medfile)
      IF((n_parallel.EQ.0 .OR. i_parallel.EQ.first_parallel))close(i_med
     *err)
      entry show_media_parameters(ounit)
      IF((ounit .LE. 0))return
      IF ((is_pegsless)) THEN
        write(ounit,*)
        write(ounit,*)' Medium data: '
        write(ounit,*)
        write(ounit,'(a,1p,e14.5,a,e14.5,a)')' AE = ',ae(1),' MeV,  UE =
     * ',ue(1),' MeV'
        write(ounit,'(a,1p,e14.5,a,e14.5,a)')' AP = ',ap(1),' MeV,  UP =
     * ',up(1),' MeV'
        write(ounit,*)
        IF ((medfile_specified)) THEN
          write(ounit,*)' Material data file: ',material_file
        ELSE
          write(ounit,*)' No material data file supplied.  Material data
     * obtained from'
          write(ounit,*)' .egsinp file or density correction file.'
        END IF
        write(ounit,*)
        DO 3151 i=1,nmed
          write(ounit,'(a,24a1)')'   Medium: ',(media(j,i),j=1,24)
          write(ounit,'(a,24a1)')' Sterncid: ',(inpstrn(j,i),j=1,24)
          write(ounit,'(a,1p,e14.5,a)')'     rho: ',rho(i),' g/cm^3'
          write(ounit,'(a,24a4)')' Elements: ',(inpasym(i,j),j=1,nne(i))
          write(ounit,'(a,1p,12e14.5)')'    rhoz: ',(rhoz(i,j),j=1,nne(i
     *    ))
          write(ounit,'(a,1p,12e14.5)')'      pz: ',(pz(i,j),j=1,nne(i))
          write(ounit,'(a,i5)')'  iunrst: ',iunrst(i)
          write(ounit,'(a,i5)')'  iaprim: ',iaprim(i)
          write(ounit,'(a,1p,e14.5,a)')'    gasp: ',inpgasp(i),' atm.'
          IF ((epstfl(i).EQ.1)) THEN
            write(ounit,*)' density correction file: ', inpdensity_file(
     *      i)(:lnblnk1(inpdensity_file(i)))
            IF ((df_if_elem_mismatch(i))) THEN
              write(ounit,*)' ****Warning: composition specified in dens
     *ity correction', ' file is not the same as that'
              write(ounit,*)' specified in input or material data file.'
              write(ounit,*) ' Will use the composition specified in the
     * density correction file.'
            END IF
            IF ((df_if_rho_mismatch(i))) THEN
              write(ounit,*)' ****Warning: rho specified in density corr
     *ection', ' file is not the same as that'
              write(ounit,*)' specified in input or material data file.'
              write(ounit,*) ' Will use rho as specified in the density
     *correction file.'
            END IF
          END IF
          write(ounit,*)
          IF ((outfile_specified(i) .AND. (n_parallel.EQ.0 .OR. i_parall
     *    el.EQ.first_parallel))) THEN
            inquire(file=spoutput_file(i),exist=ex)
            IF ((ex)) THEN
              write(i_log,'(/a)') '***************** Warning: '
              write(i_log,'(a)') 'Warning: stopping power output file ',
     *         spoutput_file(i),'already exists.  Will overwrite.'
            END IF
            i_outfile=20
            i_outfile=egs_get_unit(i_outfile)
            IF ((i_outfile .LT. 1)) THEN
              write(i_log,'(/a)') '***************** Warning: '
              write(i_log,'(a)') 'Warning: Failed to get available fortr
     *an unit for', ' stopping power output file.'
            END IF
            open(i_outfile,file=spoutput_file(i),status='unknown',err=31
     *      60)
            goto 3170
3160        write(i_log,'(/a)') '***************** Warning: '
            write(i_log,'(a)') 'Warning: Failed to open stopping power o
     *utput file ', spoutput_file(i)
            goto 3180
3170        IFLAG1=0
            IFLAG2=0
            IPLOTE=0
            MEDIUM=i
            XAXIS = 'kinetic energy / MeV'
            YAXISE = 'dE/drhoX MeV/g/cm\\S2\\N'
            YAXISEmfp = 'mean free path / cm'
            YAXISPmfp = 'mean free path / cm'
            write(GRAPHTITLE,'(24a1)')(media(j,i),j=1,24)
            SUBTITLE = 'Electron data'
            DO 3191 j=1,8
              DO 3201 k=1,16
                EKE=ETAB(k)*10.**(j-4)
                IF ((EKE .LE. AE(1)-PRM)) THEN
                  IF ((IFLAG1 .EQ. 0)) THEN
                    IFLAG1=1
                    EKE=AE(1)-PRM
                  ELSE
                    EKE=0.0
                  END IF
                END IF
                IF ((EKE .GT. UE(1)-PRM)) THEN
                  IF ((IFLAG2 .EQ. 0)) THEN
                    IFLAG2=1
                    EKE=UE(1)-PRM
                  ELSE
                    EKE=1.E30
                  END IF
                END IF
                EIE=EKE+PRM
                TMXSO=0.0
                DEDXE=0.0
                DEDXP=0.0
                EFRACT=0.0
                IF ((EIE .GE. AE(1)-0.0001 .AND. EIE .LE. UE(1)+0.001))
     *          THEN
                  ELKE=LOG(EKE)
                  LELKE=EKE1(MEDIUM)*ELKE+EKE0(MEDIUM)
                  DEDXE=EDEDX1(LELKE,MEDIUM)*ELKE+EDEDX0(LELKE,MEDIUM)
                  IPLOTE=IPLOTE+1
                  PLOTEEN(IPLOTE)=EKE
                  PLOTE(IPLOTE)=DEDXE/RHO(MEDIUM)
                END IF
3201          CONTINUE
3202          CONTINUE
3191        CONTINUE
3192        CONTINUE
            IF ((IPLOTE.GT.0)) THEN
              IF ((iunrst(i).EQ.0)) THEN
                SERIES='restricted total stopping power'
              ELSE IF((iunrst(i).EQ.1)) THEN
                SERIES='unrestricted collision stopping power'
              ELSE IF((iunrst(i).EQ.2)) THEN
                SERIES='unrestricted collision + radiative stopping powe
     *r'
              ELSE IF((iunrst(i).EQ.3)) THEN
                SERIES='unrestricted collision + restricted radiative st
     *opping power'
              ELSE IF((iunrst(i).EQ.4)) THEN
                SERIES='restricted collision + unrestricted radiative st
     *opping power'
              ELSE IF((iunrst(i).EQ.5)) THEN
                SERIES='unrestricted radiative stopping power'
              END IF
              CALL MEDXVGRPLOT(PLOTEEN,PLOTE,IPLOTE,0,SERIES, XAXIS,YAXI
     *        SE,GRAPHTITLE,SUBTITLE,i_outfile,2)
            END IF
            close(i_outfile)
3180        CONTINUE
          END IF
3151    CONTINUE
3152    CONTINUE
      END IF
      return
2930  write(i_log,'(/a)') '***************** Error: '
      write(i_log,'(a)') 'Error: Cannot open material data file',materia
     *l_file
      write(i_log,'(/a)') '***************** Quiting now.'
      call exit(1)
      return
3060  write(i_log,'(/a)') '***************** Error: '
      write(i_log,'(a)') 'Error: Cannot open density correction file: ',
     * density_file(:lnblnk1(density_file))
      write(i_log,'(/a)') '***************** Quiting now.'
      call exit(1)
      return
      end
      SUBROUTINE MEDXVGRPLOT (X, Y, NPTS, CURVENUM, SERIESTITLE, XTITLE,
     * YTITLE, GRAPHTITLE, SUBTITLE, UNITNUM, AXISTYPE)
      IMPLICIT NONE
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      integer*4 MAX
      PARAMETER (MAX = 400)
      integer*4 NPTS,NPTS1,CURVENUM, COUNT,UNITNUM,TYPE,AXISTYPE
      real*8 X(NPTS),Y(NPTS),ERRY(NPTS),YMIN,SMALLESTX, SMALLESTY,FUDGE
      integer*4 TITLELENGTH,SUBLENGTH,XAXISLENGTH,YAXISLENGTH,SERIESLENG
     *TH
      integer*4 LOGX, LOGY
      CHARACTER*(*) SUBTITLE
      CHARACTER*(*) GRAPHTITLE,XTITLE,YTITLE,SERIESTITLE
      logical TESTFILE, ALLPOS
      FUDGE = 1.e-10
      IF (( NPTS .gt. MAX)) THEN
        WRITE(6,3210)NPTS, MAX
3210    FORMAT(//' **************************'/ ' Number of points asked
     * for =', I5, ' is greater than max allowed of', I4/ ' Setting NPTS
     * to MAX, you could adjust MAX in xvgrplot.mortran'/ ' ************
     ***************'//)
        NPTS1 = MAX
      ELSE
        NPTS1 = NPTS
      END IF
      INQUIRE(UNIT = UNITNUM,OPENED=TESTFILE)
      IF ((.NOT.TESTFILE)) THEN
        WRITE(6,3220) UNITNUM
3220    FORMAT (//'  ---------Error in Subroutine XVGRPLOT---------' ,/'
     *   Unit specified (',I2,') is not open.' ,/'   Unit must be opened
     * before using subroutine.' ,/'   Data not written to file.' ,/'  -
     *---------------------------------------------'//)
        RETURN
      END IF
      TITLELENGTH = 61
      SUBLENGTH = 61
      XAXISLENGTH = 61
      YAXISLENGTH = 61
      SERIESLENGTH = 61
3231  CONTINUE
        TITLELENGTH = TITLELENGTH - 1
        IF(((GRAPHTITLE(TITLELENGTH:TITLELENGTH) .NE. ' ')))GO TO3232
      GO TO 3231
3232  CONTINUE
3241  CONTINUE
        SUBLENGTH = SUBLENGTH - 1
        IF(((SUBTITLE(SUBLENGTH:SUBLENGTH) .NE. ' ')))GO TO3242
      GO TO 3241
3242  CONTINUE
3251  CONTINUE
        XAXISLENGTH = XAXISLENGTH - 1
        IF(((XTITLE(XAXISLENGTH:XAXISLENGTH) .NE. ' ')))GO TO3252
      GO TO 3251
3252  CONTINUE
3261  CONTINUE
        YAXISLENGTH = YAXISLENGTH - 1
        IF(((YTITLE(YAXISLENGTH:YAXISLENGTH) .NE. ' ')))GO TO3262
      GO TO 3261
3262  CONTINUE
3271  CONTINUE
        SERIESLENGTH = SERIESLENGTH - 1
        IF(((SERIESTITLE(SERIESLENGTH:SERIESLENGTH) .NE. ' ')))GO TO3272
      GO TO 3271
3272  CONTINUE
      LOGX = 0
      LOGY = 0
      ALLPOS=.TRUE.
      IF (( X(1).EQ.0.0 )) THEN
        SMALLESTX = 0.1
      ELSE
        SMALLESTX=X(1)
      END IF
      IF (( Y(1).EQ.0.0 )) THEN
        SMALLESTY = 0.1
      ELSE
        SMALLESTY=Y(1)
      END IF
      DO 3281 COUNT=1,NPTS1
        IF (((X(COUNT) .LT. SMALLESTX) .AND. (X(COUNT).NE.0.))) THEN
          SMALLESTX=X(COUNT)
        END IF
        IF (((Y(COUNT) .LT. SMALLESTY) .AND. (Y(COUNT).NE.0.))) THEN
          SMALLESTY=Y(COUNT)
        END IF
        IF (((X(COUNT) .LT. 0.).OR.(Y(COUNT) .LT. 0.))) THEN
          ALLPOS=.FALSE.
        END IF
3281  CONTINUE
3282  CONTINUE
      IF ((ALLPOS)) THEN
        DO 3291 COUNT=1,NPTS1
          IF ((X(COUNT).EQ.0.)) THEN
            X(COUNT)=SMALLESTX*FUDGE
          END IF
          IF ((Y(COUNT).EQ.0.)) THEN
            Y(COUNT)=SMALLESTY*FUDGE
          END IF
3291    CONTINUE
3292    CONTINUE
      END IF
      IF ((AXISTYPE .GT. 0)) THEN
        DO 3301 COUNT=1,NPTS1
          IF ((X(COUNT) .LE. 0.)) THEN
            LOGX = 1
          END IF
          IF ((Y(COUNT) .LE. 0.)) THEN
            LOGY = 1
          END IF
3301    CONTINUE
3302    CONTINUE
      END IF
      IF ((CURVENUM .EQ. 0)) THEN
        IF ((AXISTYPE .EQ. 0)) THEN
          WRITE(UNITNUM,3310) 'xy'
        ELSE IF((AXISTYPE .EQ. 1)) THEN
          WRITE(UNITNUM,3310) 'logy'
          WRITE(UNITNUM,3320)
        ELSE IF((AXISTYPE .EQ. 2)) THEN
          WRITE(UNITNUM,3310) 'logx'
          WRITE(UNITNUM,3320)
        ELSE IF((AXISTYPE .EQ. 3)) THEN
          WRITE(UNITNUM,3310) 'logxy'
          WRITE(UNITNUM,3320)
          WRITE(UNITNUM,3330)
        ELSE
          WRITE(6,3340) AXISTYPE
3340      FORMAT (//'  ------------Error in Subroutine XVGRPLOT---------
     *--' ,/'   AXISTYPE specified (',I2,') is not a valid option.' ,/' 
     *----------------------------------------------'//)
          RETURN
        END IF
3310    FORMAT ('@g0 type ',A,' ')
3320    FORMAT ('@    xaxis  ticklabel format exponential')
3330    FORMAT ('@    yaxis  ticklabel format exponential')
        WRITE(UNITNUM,3350) GRAPHTITLE(1:TITLELENGTH) ,SUBTITLE(1:SUBLEN
     *  GTH) ,XTITLE(1:XAXISLENGTH) ,YTITLE(1:YAXISLENGTH)
3350    FORMAT ('@    title "',A,'"'/ ,'@    subtitle "',A,'"'/ ,'@    l
     *egend on'/ ,'@    legend box linestyle 0'/ ,'@    legend x1 0.6'/,
     *'@    legend y1 0.75'/ ,'@    view xmin 0.250000'/ ,'@    xaxis  l
     *abel "',A,'"'/ ,'@    timestamp on'/ ,'@    yaxis  label "',A,'"')
      END IF
      IF ((AXISTYPE .EQ. 1 .AND. LOGY .EQ. 1)) THEN
        WRITE(UNITNUM,3310) 'xy'
        WRITE(6,3360)
3360    FORMAT (/' ----------WARNING from Subroutine XVGRPLOT---------',
     */'  Log scale requested for Y axis when one or more   ' ,/'  Ydata
     * points are 0 or negative.                  ' ,//'  Y axis scale c
     *hanged to linear.                   ' ,/' ------------------------
     *---------------------------'/)
      END IF
      IF ((AXISTYPE .EQ. 2 .AND. LOGX .EQ. 1)) THEN
        WRITE(UNITNUM,3310) 'xy'
        WRITE(6,3370)
3370    FORMAT (/' ----------WARNING from Subroutine XVGRPLOT---------',
     */'  Log scale requested for X axis when one or more   ' ,/'  Xdata
     * points are 0 or negative.                  ' ,//'  X axis scale c
     *hanged to linear.                   ' ,/' ------------------------
     *---------------------------'/)
      END IF
      IF ((AXISTYPE .EQ. 3 .AND. (LOGX .EQ. 1 .OR. LOGY .EQ. 1))) THEN
        IF ((LOGX .EQ. 1 .AND. LOGY .EQ. 1)) THEN
          WRITE(UNITNUM,3310) 'xy'
          WRITE(6,3380)
3380      FORMAT (/' ----------WARNING from Subroutine XVGRPLOT---------
     *' ,/'  Log scale requested for X axis and Y axis when    ' ,/'  on
     *e or more X and Y data points are 0 or negative.' ,//'  X and Y ax
     *es scales changed to linear.            ' ,/' --------------------
     *-------------------------------'/)
        ELSE IF((LOGX .EQ. 1)) THEN
          WRITE(UNITNUM,3310) 'logy'
          WRITE(6,3370)
        ELSE
          WRITE(UNITNUM,3310) 'logx'
          WRITE(6,3360)
        END IF
      END IF
      IF ((CURVENUM .LT. 10 )) THEN
        WRITE(UNITNUM,'(''@    s'',I1,'' on'')') CURVENUM
      ELSE
        WRITE(UNITNUM,'(''@    s'',I2,'' on'')') CURVENUM
      END IF
      WRITE(UNITNUM,3390) CURVENUM,SERIESTITLE(1:SERIESLENGTH)
3390  FORMAT ('@    legend string ',I2,' "',A,'"')
      WRITE(UNITNUM,3400)
3400  FORMAT ('@TYPE xy')
      IF ((CURVENUM .LT. 10)) THEN
        WRITE(UNITNUM,3410) CURVENUM
        IF ((CURVENUM .EQ. 9)) THEN
          WRITE(UNITNUM,3420) CURVENUM, CURVENUM+1
        ELSE
          WRITE(UNITNUM,3430) CURVENUM, CURVENUM+1
        END IF
      ELSE
        WRITE(UNITNUM,3440) CURVENUM
        WRITE(UNITNUM,3450) CURVENUM, CURVENUM+1
      END IF
3410  FORMAT ('@    s',I1,' errorbar length 0.000000')
3440  FORMAT ('@    s',I2,' errorbar length 0.000000')
3420  FORMAT ('@    s',I1,' symbol color ',I2)
3430  FORMAT ('@    s',I1,' symbol color ',I1)
3450  FORMAT ('@    s',I2,' symbol color ',I2)
      DO 3461 COUNT=1,NPTS1
        WRITE(UNITNUM,3470) X(COUNT),Y(COUNT)
3461  CONTINUE
3462  CONTINUE
3470  FORMAT (1PE15.4,1PE15.4)
      WRITE(UNITNUM,'(''&'')')
      RETURN
      END
      SUBROUTINE WATCH(IARG,IWATCH)
      implicit none
      integer*4 iarg,iwatch,IP,ICOUNT,JHSTRY,J,N
      real*8 KE
      integer*4 graph_unit
      integer egs_open_file
      integer*4 ku,kr,ka
      COMMON/BOUNDS/ECUT(1),PCUT(1),VACDST
      real*8 ECUT,  PCUT,  VACDST
      COMMON/STACK/ E(50),X(50),Y(50),Z(50),U(50),V(50),W(50),DNEAR(50),
     *WT(50),IQ(50),IR(50),LATCH(50), LATCHI,NP,NPold
      DOUBLE PRECISION E
      real*8 X,Y,Z,  U,V,W,  DNEAR,  WT
      integer*4 IQ,  IR,  LATCH,  LATCHI, NP,  NPold
      COMMON/EPCONT/EDEP,EDEP_LOCAL,TSTEP,TUSTEP,USTEP,TVSTEP,VSTEP, RHO
     *F,EOLD,ENEW,EKE,ELKE,GLE,E_RANGE, x_final,y_final,z_final, u_final
     *,v_final,w_final, IDISC,IROLD,IRNEW,IAUSFL(35)
      DOUBLE PRECISION EDEP,  EDEP_LOCAL
      real*8 TSTEP,  TUSTEP,  USTEP,  VSTEP,  TVSTEP,  RHOF,  EOLD,  ENE
     *W,  EKE,  ELKE,  GLE,  E_RANGE, x_final,y_final,z_final,  u_final,
     *v_final,w_final
      integer*4 IDISC,  IROLD,  IRNEW,  IAUSFL
      common/egs_vr/ e_max_rr(1),  prob_RR,  nbr_split,  i_play_RR,
     * i_survived_RR,
     *     n_RR_warning,                                        i_do_rr(
     *1)
      real*8          e_max_rr,prob_RR
      integer*4       nbr_split,i_play_RR,i_survived_RR,n_RR_warning
      integer*2     i_do_rr
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      DATA ICOUNT/0/,JHSTRY/1/ graph_unit/-1/
      save ICOUNT,JHSTRY,graph_unit
      ku = 13
      kr = 0
      ka = 1
      IF ((IARG .EQ. -99)) THEN
        DO 3481 J=1,29
          IAUSFL(J)=1
3481    CONTINUE
3482    CONTINUE
        IAUSFL(22)=0
        IAUSFL(23)=0
        IAUSFL(24)=0
      END IF
      IF ((IARG .EQ. -1)) THEN
        IF ((IWATCH .EQ. 4)) THEN
          IF (( graph_unit .LT. 0 )) THEN
            graph_unit = egs_open_file(ku,kr,ka,'.egsgph')
          END IF
          WRITE(graph_unit,3490) 0,0,0,0.0,0.0,0.0,0.0,JHSTRY
          JHSTRY=JHSTRY+1
        ELSE
          WRITE(6,3500)JHSTRY
3500      FORMAT(' END OF HISTORY',I8,3X,40('*')/)
          JHSTRY=JHSTRY+1
          ICOUNT=ICOUNT+2
          RETURN
        END IF
      END IF
      IF (( (IWATCH .NE. 4) .AND. ((ICOUNT .GE. 50) .OR. (ICOUNT .EQ. 0)
     * .OR. (IARG .EQ. -99)) )) THEN
        ICOUNT=1
        WRITE(6,3510)
3510    FORMAT(//T39,' NP',3X,'ENERGY  Q REGION    X',7X, 'Y',7X,'Z',6X,
     *'U',6X,'V',6X,'W',6X,'LATCH',2X,'WEIGHT'/)
      END IF
      IF (((IWATCH .EQ. 4) .AND. (IARG .GE. 0) .AND. (IARG .NE. 5))) THE
     *N
        IF((graph_unit .LT. 0))graph_unit = egs_open_file(ku,kr,ka,'.egs
     *gph')
        WRITE(graph_unit,3490) NP,IQ(NP),IR(NP),X(NP),Y(NP),Z(NP),E(NP)
3490    FORMAT(2I4,1X,I6,4G15.8,I12)
      END IF
      IF((IARG .EQ. 5 .OR. IARG .LT. 0))RETURN
      IF((IWATCH .EQ. 4))RETURN
      KE=E(NP)
      IF ((IQ(NP).NE.0)) THEN
        KE=E(NP)-PRM
      END IF
      IF ((IARG .EQ. 0 .AND. IWATCH .EQ. 2)) THEN
        ICOUNT=ICOUNT+1
        WRITE(6,3520)NP,KE,IQ(NP),IR(NP),X(NP),Y(NP),Z(NP),U(NP),V(NP),
     *  W(NP),LATCH(NP),WT(NP)
3520    FORMAT(T11,'STEP ABOUT TO OCCUR', T36,':',I5,F9.3,2I4,3F8.3,3F7.
     *3,I10,1PE10.3)
      ELSE IF((IARG .EQ. 0)) THEN
        RETURN
      END IF
      IF (( IARG .EQ. 1)) THEN
        ICOUNT=ICOUNT+1
        WRITE(6,3530)NP,KE,IQ(NP),IR(NP),X(NP),Y(NP),Z(NP),U(NP),V(NP),
     *  W(NP),LATCH(NP),WT(NP)
3530    FORMAT(' Discard  AE,AP<E<ECUT',T36,':',I5,F9.3,2I4,3F8.3,3F7.3,
     *I10,1PE10.3)
      ELSE IF((IARG .EQ. 2)) THEN
        ICOUNT=ICOUNT+1
        WRITE(6,3540)NP,KE,IQ(NP),IR(NP),X(NP),Y(NP),Z(NP),U(NP),V(NP),
     *  W(NP),LATCH(NP),WT(NP)
3540    FORMAT(' Discard  E<AE,AP',T36,':',I5,F9.3,2I4,3F8.3,3F7.3,I10,1
     *PE10.3)
      ELSE IF((IARG .EQ. 3)) THEN
        ICOUNT=ICOUNT+1
        WRITE(6,3550)NP,KE,IQ(NP),IR(NP),X(NP),Y(NP),Z(NP),U(NP),V(NP),
     *  W(NP),LATCH(NP),WT(NP)
3550    FORMAT(' Discard -user request',T36,':',I5,F9.3,2I4,3F8.3,3F7.3,
     *I10,1PE10.3)
      ELSE IF((IARG .EQ. 4)) THEN
        WRITE(6,3560)EDEP,IR(NP)
3560    FORMAT(T10,'Local energy deposition',T36,':',F12.5,' MeV in regi
     *on ',I6)
      ELSE IF((IARG .EQ. 6)) THEN
        ICOUNT=ICOUNT+1
        WRITE(6,3570)NP,KE,IQ(NP),IR(NP),X(NP),Y(NP),Z(NP),U(NP),V(NP),
     *  W(NP),LATCH(NP),WT(NP)
3570    FORMAT(' bremsstrahlung  about to occur',T36,':',I5,F9.3,2I4,3F8
     *.3,3F7.3,I10,1PE10.3)
      ELSE IF((IARG .EQ. 7)) THEN
        IF ((nbr_split .EQ.1)) THEN
          DO 3581 IP=NPold,NP
            IF ((IQ(IP).EQ.-1)) THEN
              KE = E(IP) - RM
              ICOUNT=ICOUNT+1
              WRITE(6,3590)IP,KE,IQ(IP),IR(IP),X(IP),Y(IP),Z(IP),U(IP),V
     *        (IP), W(IP),LATCH(IP),WT(IP)
3590          FORMAT(T10,'Resulting electron',T36,':',I5,F9.3,2I4,3F8.3,
     *3F7.3,I10,1PE10.3)
            ELSE
              KE = E(IP)
              ICOUNT=ICOUNT+1
              WRITE(6,3600)IP,KE,IQ(IP),IR(IP),X(IP),Y(IP),Z(IP),U(IP),V
     *        (IP), W(IP),LATCH(IP),WT(IP)
3600          FORMAT(T10,'Resulting photon',T36,':',I5,F9.3,2I4,3F8.3,3F
     *7.3,I10,1PE10.3)
            END IF
3581      CONTINUE
3582      CONTINUE
        ELSE
          KE = E(NPold) - RM
          ICOUNT=ICOUNT+1
          WRITE(6,3610)NPold,KE,IQ(NPold),IR(NPold),X(NPold),Y(NPold),Z(
     *    NPold),U(NPold),V(NPold), W(NPold),LATCH(NPold),WT(NPold)
3610      FORMAT(T10,'Resulting electron',T36,':',I5,F9.3,2I4,3F8.3,3F7.
     *3,I10,1PE10.3)
          DO 3621 IP=NPold+1,NP
            KE= E(IP)
            IF ((IP .EQ. NPold+1)) THEN
              ICOUNT=ICOUNT+1
              WRITE(6,3630)IP,KE,IQ(IP),IR(IP),X(IP),Y(IP),Z(IP),U(IP),V
     *        (IP), W(IP),LATCH(IP),WT(IP)
3630          FORMAT(T10,'Split photons',T36,':',I5,F9.3,2I4,3F8.3,3F7.3
     *,I10,1PE10.3)
            ELSE
              ICOUNT=ICOUNT+1
              WRITE(6,3640)IP,KE,IQ(IP),IR(IP),X(IP),Y(IP),Z(IP),U(IP),V
     *        (IP), W(IP),LATCH(IP),WT(IP)
3640          FORMAT(T36,':',I5,F9.3,2I4,3F8.3,3F7.3,I10,1PE10.3)
            END IF
3621      CONTINUE
3622      CONTINUE
        END IF
      ELSE IF((IARG .EQ. 8)) THEN
        ICOUNT=ICOUNT+1
        WRITE(6,3650)NP,KE,IQ(NP),IR(NP),X(NP),Y(NP),Z(NP),U(NP),V(NP),
     *  W(NP),LATCH(NP),WT(NP)
3650    FORMAT(' Moller   about to occur',T36,':',I5,F9.3,2I4,3F8.3,3F7.
     *3,I10,1PE10.3)
      ELSE IF((IARG .EQ. 9)) THEN
        IF ((NP.EQ.NPold)) THEN
          ICOUNT=ICOUNT+1
          WRITE(6,3660)NP,KE,IQ(NP),IR(NP),X(NP),Y(NP),Z(NP),U(NP),V(NP)
     *    , W(NP),LATCH(NP),WT(NP)
3660      FORMAT(T11,'Interaction rejected',T36,':',I5,F9.3,2I4,3F8.3,3F
     *7.3,I10,1PE10.3)
        ELSE
          DO 3671 IP=NPold,NP
            KE = E(IP) - ABS(IQ(NP))*RM
            IF ((IP.EQ.NPold)) THEN
              ICOUNT=ICOUNT+1
              WRITE(6,3680)IP,KE,IQ(IP),IR(IP),X(IP),Y(IP),Z(IP),U(IP),V
     *        (IP), W(IP),LATCH(IP),WT(IP)
3680          FORMAT(T11,'Resulting electrons',T36,':',I5,F9.3,2I4,3F8.3
     *,3F7.3,I10,1PE10.3)
            ELSE
              ICOUNT=ICOUNT+1
              WRITE(6,3690)IP,KE,IQ(IP),IR(IP),X(IP),Y(IP),Z(IP),U(IP),V
     *        (IP), W(IP),LATCH(IP),WT(IP)
3690          FORMAT(T36,':',I5,F9.3,2I4,3F8.3,3F7.3,I10,1PE10.3)
            END IF
3671      CONTINUE
3672      CONTINUE
        END IF
      ELSE IF((IARG .EQ. 10)) THEN
        ICOUNT=ICOUNT+1
        WRITE(6,3700)NP,KE,IQ(NP),IR(NP),X(NP),Y(NP),Z(NP),U(NP),V(NP),
     *  W(NP),LATCH(NP),WT(NP)
3700    FORMAT(' Bhabba   about to occur',T36,':',I5,F9.3,2I4,3F8.3,3F7.
     *3,I10,1PE10.3)
      ELSE IF((IARG .EQ. 11)) THEN
        IF ((NP.EQ.NPold)) THEN
          ICOUNT=ICOUNT+1
          WRITE(6,3710)NP,KE,IQ(NP),IR(NP),X(NP),Y(NP),Z(NP),U(NP),V(NP)
     *    , W(NP),LATCH(NP),WT(NP)
3710      FORMAT(T11,'Interaction rejected',T36,':',I5,F9.3,2I4,3F8.3,3F
     *7.3,I10,1PE10.3)
        ELSE
          DO 3721 IP=NPold,NP
            KE = E(IP) - ABS(IQ(IP))*RM
            IF ((IP.EQ.NPold)) THEN
              ICOUNT=ICOUNT+1
              WRITE(6,3730)IP,KE,IQ(IP),IR(IP),X(IP),Y(IP),Z(IP),U(IP),V
     *        (IP), W(IP),LATCH(IP),WT(IP)
3730          FORMAT(T11,'Resulting e- or e+',T36,':',I5,F9.3,2I4,3F8.3,
     *3F7.3,I10,1PE10.3)
            ELSE
              ICOUNT=ICOUNT+1
              WRITE(6,3740)IP,KE,IQ(IP),IR(IP),X(IP),Y(IP),Z(IP),U(IP),V
     *        (IP), W(IP),LATCH(IP),WT(IP)
3740          FORMAT(T36,':',I5,F9.3,2I4,3F8.3,3F7.3,I10,1PE10.3)
            END IF
3721      CONTINUE
3722      CONTINUE
        END IF
      ELSE IF((IARG .EQ. 12)) THEN
        ICOUNT=ICOUNT+1
        WRITE(6,3750)NP,KE,IQ(NP),IR(NP),X(NP),Y(NP),Z(NP),U(NP),V(NP),
     *  W(NP),LATCH(NP),WT(NP)
3750    FORMAT(' Positron about to decay in flight',T36,':',I5,F9.3,2I4,
     *3F8.3,3F7.3,I10,1PE10.3)
      ELSE IF((IARG .EQ. 13)) THEN
        IF ((NP.EQ.NPold)) THEN
          ICOUNT=ICOUNT+1
          WRITE(6,3760)NP,KE,IQ(NP),IR(NP),X(NP),Y(NP),Z(NP),U(NP),V(NP)
     *    , W(NP),LATCH(NP),WT(NP)
3760      FORMAT(T11,'Interaction rejected',T36,':',I5,F9.3,2I4,3F8.3,3F
     *7.3,I10,1PE10.3)
        ELSE
          DO 3771 IP=NPold,NP
            KE = E(IP) - ABS(IQ(IP))*RM
            IF ((IP.EQ.NPold)) THEN
              ICOUNT=ICOUNT+1
              WRITE(6,3780)IP,KE,IQ(IP),IR(IP),X(IP),Y(IP),Z(IP),U(IP),V
     *        (IP), W(IP),LATCH(IP),WT(IP)
3780          FORMAT(T11,'Resulting photons',T36,':',I5,F9.3,2I4,3F8.3,3
     *F7.3,I10,1PE10.3)
            ELSE
              ICOUNT=ICOUNT+1
              WRITE(6,3790)IP,KE,IQ(IP),IR(IP),X(IP),Y(IP),Z(IP),U(IP),V
     *        (IP), W(IP),LATCH(IP),WT(IP)
3790          FORMAT(T36,':',I5,F9.3,2I4,3F8.3,3F7.3,I10,1PE10.3)
            END IF
3771      CONTINUE
3772      CONTINUE
        END IF
      ELSE IF((IARG .EQ. 28)) THEN
        ICOUNT=ICOUNT+1
        WRITE(6,3800)NP,KE,IQ(NP),IR(NP),X(NP),Y(NP),Z(NP),U(NP),V(NP),
     *  W(NP),LATCH(NP),WT(NP)
3800    FORMAT(' Positron will annihilate at rest',T36,':',I5,F9.3,2I4,3
     *F8.3,3F7.3,I10,1PE10.3)
      ELSE IF((IARG .EQ. 14)) THEN
        IF ((NP.EQ.NPold)) THEN
          ICOUNT=ICOUNT+1
          WRITE(6,3810)NP,KE,IQ(NP),IR(NP),X(NP),Y(NP),Z(NP),U(NP),V(NP)
     *    , W(NP),LATCH(NP),WT(NP)
3810      FORMAT(T11,'Interaction rejected',T36,':',I5,F9.3,2I4,3F8.3,3F
     *7.3,I10,1PE10.3)
        ELSE
          DO 3821 IP=NPold,NP
            KE = E(IP) - ABS(IQ(IP))*RM
            IF ((IP.EQ.NPold)) THEN
              ICOUNT=ICOUNT+1
              WRITE(6,3830)IP,KE,IQ(IP),IR(IP),X(IP),Y(IP),Z(IP),U(IP),V
     *        (IP), W(IP),LATCH(IP),WT(IP)
3830          FORMAT(' Positron annihilates at rest',T36,':',I5,F9.3,2I4
     *,3F8.3,3F7.3,I10,1PE10.3)
            ELSE
              ICOUNT=ICOUNT+1
              WRITE(6,3840)IP,KE,IQ(IP),IR(IP),X(IP),Y(IP),Z(IP),U(IP),V
     *        (IP), W(IP),LATCH(IP),WT(IP)
3840          FORMAT(T36,':',I5,F9.3,2I4,3F8.3,3F7.3,I10,1PE10.3)
            END IF
3821      CONTINUE
3822      CONTINUE
        END IF
      ELSE IF((IARG .EQ. 15)) THEN
        ICOUNT=ICOUNT+1
        WRITE(6,3850)NP,KE,IQ(NP),IR(NP),X(NP),Y(NP),Z(NP),U(NP),V(NP),
     *  W(NP),LATCH(NP),WT(NP)
3850    FORMAT(' Pair production about to occur',T36,':',I5,F9.3,2I4,3F8
     *.3,3F7.3,I10,1PE10.3)
      ELSE IF((IARG .EQ. 16)) THEN
        IF ((NP.EQ.NPold .AND. i_survived_rr .EQ. 0)) THEN
          ICOUNT=ICOUNT+1
          WRITE(6,3860)NP,KE,IQ(NP),IR(NP),X(NP),Y(NP),Z(NP),U(NP),V(NP)
     *    , W(NP),LATCH(NP),WT(NP)
3860      FORMAT(T11,'Interaction rejected',T36,':',I5,F9.3,2I4,3F8.3,3F
     *7.3,I10,1PE10.3)
        ELSE IF((NP.EQ.NPold .AND. i_survived_rr .GT. 0)) THEN
          WRITE(6,3870)i_survived_rr,prob_rr
3870      FORMAT(T10,'Russian Roulette eliminated ',I2, ' particle(s) wi
     *th probability ',F8.5)
          ICOUNT=ICOUNT+1
          WRITE(6,3880)NP,KE,IQ(NP),IR(NP),X(NP),Y(NP),Z(NP),U(NP),V(NP)
     *    , W(NP),LATCH(NP),WT(NP)
3880      FORMAT(T10,'Now on top of stack',T36,':',I5,F9.3,2I4,3F8.3,3F7
     *.3,I10,1PE10.3)
        ELSE
          DO 3891 IP=NPold,NP
            KE = E(IP) - ABS(IQ(IP))*RM
            IF ((IP.EQ.NPold)) THEN
              ICOUNT=ICOUNT+1
              WRITE(6,3900)IP,KE,IQ(IP),IR(IP),X(IP),Y(IP),Z(IP),U(IP),V
     *        (IP), W(IP),LATCH(IP),WT(IP)
3900          FORMAT(T11,'Resulting pair',T36,':',I5,F9.3,2I4,3F8.3,3F7.
     *3,I10,1PE10.3)
            ELSE
              ICOUNT=ICOUNT+1
              WRITE(6,3910)IP,KE,IQ(IP),IR(IP),X(IP),Y(IP),Z(IP),U(IP),V
     *        (IP), W(IP),LATCH(IP),WT(IP)
3910          FORMAT(T36,':',I5,F9.3,2I4,3F8.3,3F7.3,I10,1PE10.3)
            END IF
3891      CONTINUE
3892      CONTINUE
          IF ((i_survived_rr .GT. 0)) THEN
            WRITE(6,3920)i_survived_rr,prob_rr
3920        FORMAT(T10,'Russian Roulette eliminated ',I2,'              
     *                  particle(s) with probability ',F8.5)
            ICOUNT=ICOUNT+1
            WRITE(6,3930)NP,KE,IQ(NP),IR(NP),X(NP),Y(NP),Z(NP),U(NP),V(N
     *      P), W(NP),LATCH(NP),WT(NP)
3930        FORMAT(T10,'Now on top of stack',T36,':',I5,F9.3,2I4,3F8.3,3
     *F7.3,I10,1PE10.3)
          END IF
        END IF
      ELSE IF((IARG .EQ. 17)) THEN
        ICOUNT=ICOUNT+1
        WRITE(6,3940)NP,KE,IQ(NP),IR(NP),X(NP),Y(NP),Z(NP),U(NP),V(NP),
     *  W(NP),LATCH(NP),WT(NP)
3940    FORMAT(' Compton  about to occur',T36,':',I5,F9.3,2I4,3F8.3,3F7.
     *3,I10,1PE10.3)
      ELSE IF((IARG .EQ. 18)) THEN
        IF ((NP .EQ. NPold .AND. i_survived_rr .EQ. 0)) THEN
          ICOUNT=ICOUNT+1
          WRITE(6,3950)NP,KE,IQ(NP),IR(NP),X(NP),Y(NP),Z(NP),U(NP),V(NP)
     *    , W(NP),LATCH(NP),WT(NP)
3950      FORMAT(T11,'Interaction rejected',T36,':',I5,F9.3,2I4,3F8.3,3F
     *7.3,I10,1PE10.3)
        ELSE IF((NP .GT. NPold)) THEN
          DO 3961 IP=NPold,NPold+1
            KE = E(IP) - ABS(IQ(IP))*RM
            IF ((IQ(IP).NE.0)) THEN
              ICOUNT=ICOUNT+1
              WRITE(6,3970)IP,KE,IQ(IP),IR(IP),X(IP),Y(IP),Z(IP),U(IP),V
     *        (IP), W(IP),LATCH(IP),WT(IP)
3970          FORMAT(T11,'compton electron created',T36,':',I5,F9.3,2I4,
     *3F8.3,3F7.3,I10,1PE10.3)
            ELSE
              ICOUNT=ICOUNT+1
              WRITE(6,3980)IP,KE,IQ(IP),IR(IP),X(IP),Y(IP),Z(IP),U(IP),V
     *        (IP), W(IP),LATCH(IP),WT(IP)
3980          FORMAT(T11,'compton scattered photon',T36,':',I5,F9.3,2I4,
     *3F8.3,3F7.3,I10,1PE10.3)
            END IF
3961      CONTINUE
3962      CONTINUE
        END IF
        IF ((i_survived_rr .GT. 0)) THEN
          WRITE(6,3990)i_survived_rr,prob_rr
3990      FORMAT(T10,'Russian Roulette eliminated ',I2, ' particle(s) wi
     *th probability ',F8.5)
          ICOUNT=ICOUNT+1
          WRITE(6,4000)NP,KE,IQ(NP),IR(NP),X(NP),Y(NP),Z(NP),U(NP),V(NP)
     *    , W(NP),LATCH(NP),WT(NP)
4000      FORMAT(T10,'Now on top of stack',T36,':',I5,F9.3,2I4,3F8.3,3F7
     *.3,I10,1PE10.3)
        END IF
      ELSE IF((IARG .EQ. 19)) THEN
        ICOUNT=ICOUNT+1
        WRITE(6,4010)NP,KE,IQ(NP),IR(NP),X(NP),Y(NP),Z(NP),U(NP),V(NP),
     *  W(NP),LATCH(NP),WT(NP)
4010    FORMAT(' Photoelectric about to occur',T36,':',I5,F9.3,2I4,3F8.3
     *,3F7.3,I10,1PE10.3)
      ELSE IF((IARG .EQ. 20)) THEN
        IF ((NPold.EQ.NP .AND. IQ(NP).EQ.0 .AND. i_survived_rr .EQ. 0))
     *  THEN
          ICOUNT=ICOUNT+1
          WRITE(6,4020)NP,KE,IQ(NP),IR(NP),X(NP),Y(NP),Z(NP),U(NP),V(NP)
     *    , W(NP),LATCH(NP),WT(NP)
4020      FORMAT(T11,'Photon energy below N-shell',/, T11,'Photon discar
     *ded',T36,':',I5,F9.3,2I4,3F8.3,3F7.3,I10,1PE10.3)
        ELSE IF((IQ(NPold) .EQ. -1 .AND. i_survived_rr .EQ. 0)) THEN
          KE= E(NPold)-RM
          ICOUNT=ICOUNT+1
          WRITE(6,4030)NPold,KE,IQ(NPold),IR(NPold),X(NPold),Y(NPold),Z(
     *    NPold),U(NPold),V(NPold), W(NPold),LATCH(NPold),WT(NPold)
4030      FORMAT(T10,'Resulting photoelectron',T36,':',I5,F9.3,2I4,3F8.3
     *,3F7.3,I10,1PE10.3)
        ELSE IF((i_survived_rr .GT. 0)) THEN
          IF ((NP.EQ.NPold-1 .OR. IQ(NPold) .NE. -1)) THEN
            IF ((i_survived_rr .GT. 1)) THEN
              WRITE(6,4040)i_survived_rr-1,prob_rr
4040          FORMAT(T10,'Russian Roulette eliminated ',I4, ' particle(s
     *) with probability ',F8.5,' plus')
            END IF
            WRITE(6,4050)prob_rr
4050        FORMAT(T10,'Russian Roulette eliminated resulting photoelect
     *ron', ' with probability ',F8.5)
          ELSE
            KE = E(NPold) - RM
            ICOUNT=ICOUNT+1
            WRITE(6,4060)NPold,KE,IQ(NPold),IR(NPold),X(NPold),Y(NPold),
     *      Z(NPold),U(NPold),V(NPold), W(NPold),LATCH(NPold),WT(NPold)
4060        FORMAT(T10,'Resulting photoelectron?',T36,':',I5,F9.3,2I4,3F
     *8.3,3F7.3,I10,1PE10.3)
            WRITE(6,4070)i_survived_rr,prob_rr
4070        FORMAT(T10,'Russian Roulette eliminated ',I4, ' particle(s)w
     *ith probability ',F8.5)
          END IF
          ICOUNT=ICOUNT+1
          WRITE(6,4080)NP,KE,IQ(NP),IR(NP),X(NP),Y(NP),Z(NP),U(NP),V(NP)
     *    , W(NP),LATCH(NP),WT(NP)
4080      FORMAT(T10,'Now on top of stack',T36,':',I5,F9.3,2I4,3F8.3,3F7
     *.3,I10,1PE10.3)
        END IF
      ELSE IF((IARG .EQ. 24)) THEN
        ICOUNT=ICOUNT+1
        WRITE(6,4090)NP,KE,IQ(NP),IR(NP),X(NP),Y(NP),Z(NP),U(NP),V(NP),
     *  W(NP),LATCH(NP),WT(NP)
4090    FORMAT(' Rayleigh scattering occured',T36,':',I5,F9.3,2I4,3F8.3,
     *3F7.3,I10,1PE10.3)
      ELSE IF((IARG .EQ. 25)) THEN
        ICOUNT=ICOUNT+1
        WRITE(6,4100)NP,KE,IQ(NP),IR(NP),X(NP),Y(NP),Z(NP),U(NP),V(NP),
     *  W(NP),LATCH(NP),WT(NP)
4100    FORMAT(T10,'Fluorescent X-ray created',T36,':',I5,F9.3,2I4,3F8.3
     *,3F7.3,I10,1PE10.3)
      ELSE IF((IARG .EQ. 26)) THEN
        ICOUNT=ICOUNT+1
        WRITE(6,4110)NP,KE,IQ(NP),IR(NP),X(NP),Y(NP),Z(NP),U(NP),V(NP),
     *  W(NP),LATCH(NP),WT(NP)
4110    FORMAT(T10,'Coster-Kronig e- created',T36,':',I5,F9.3,2I4,3F8.3,
     *3F7.3,I10,1PE10.3)
      ELSE IF((IARG .EQ. 27)) THEN
        ICOUNT=ICOUNT+1
        WRITE(6,4120)NP,KE,IQ(NP),IR(NP),X(NP),Y(NP),Z(NP),U(NP),V(NP),
     *  W(NP),LATCH(NP),WT(NP)
4120    FORMAT(T10,'Auger electron created',T36,':',I5,F9.3,2I4,3F8.3,3F
     *7.3,I10,1PE10.3)
      END IF
      IF ((IARG .EQ. 0 .AND. IWATCH .EQ. 2)) THEN
        WRITE(6,4130)USTEP,TUSTEP,VSTEP,TVSTEP,EDEP
4130    FORMAT(T5,'USTEP,TUSTEP,VSTEP,TVSTEP,EDEP',T36,':    ',5(1PE13.4
     *))
        ICOUNT=ICOUNT+1
      END IF
      IF((NP .EQ. 1 .OR. IARG .EQ. 0))RETURN
      IF (( IARG .LE. 3)) THEN
        N=NP-1
        KE = E(N) - ABS(IQ(N))*RM
        ICOUNT=ICOUNT+1
        WRITE(6,4140)N,KE,IQ(N),IR(N),X(N),Y(N),Z(N),U(N),V(N), W(N),LAT
     *  CH(N),WT(N)
4140    FORMAT(T10,'Now on top of stack',T36,':',I5,F9.3,2I4,3F8.3,3F7.3
     *,I10,1PE10.3)
      END IF
      RETURN
      END
      SUBROUTINE SIGMA(NDATA,ISTAT,MODE,IERR)
      implicit none
      integer*4 NDATA,ISTAT,MODE,IERR
      COMMON/ERROR/DATA(1,2)
      real*8 data
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      integer*4 n,non0,i
      real*8 stat,sdenom
      real*8 emax,avg,error,datum,argmnt
      DATA EMAX/99.9/
      IERR=0
      IF (((MODE .LT. 0) .OR. (MODE .GT. 2))) THEN
        MODE=2
        IERR=1
      END IF
      IF (((NDATA.LE.0).OR.(NDATA.GT.1).OR.(ISTAT.LE.0).OR.(ISTAT.GT.2))
     *) THEN
        IERR=-1
        RETURN
      END IF
      IF ((ISTAT .EQ. 1)) THEN
        IERR=10
        DO 4151 N=1,NDATA
          DATA(N,2)=EMAX
4151    CONTINUE
4152    CONTINUE
        RETURN
      END IF
      IF ((MODE.NE.0)) THEN
        STAT=FLOAT(ISTAT)
        SDENOM=STAT*(STAT-1.)
      END IF
      DO 4161 N=1,NDATA
        NON0=0
        AVG=0.0
        ERROR=0.0
        DO 4171 I=1,ISTAT
          DATUM=DATA(N,I)
          IF ((DATUM.NE.0.0)) THEN
            NON0=NON0+1
            AVG=AVG+DATUM
            ERROR=ERROR+DATUM**2
          END IF
4171    CONTINUE
4172    CONTINUE
        IF ((NON0 .EQ. 0)) THEN
          IERR=11
          ERROR=EMAX
          GOTO 4180
        ELSE IF(((NON0 .EQ. 1) .AND. (MODE .EQ. 0))) THEN
          ERROR=EMAX
          GOTO4180
        ELSE
          IF ((MODE .EQ. 0)) THEN
            STAT=FLOAT(NON0)
            SDENOM=STAT*(STAT-1.)
          END IF
        END IF
        AVG=AVG/STAT
        ARGMNT=ERROR-STAT*AVG**2
        IF ((ARGMNT.LT.0.0)) THEN
          WRITE(6,4190)ARGMNT,ERROR,STAT,AVG,SDENOM
4190      FORMAT(' ***** - SQ RT IN SIGMA. ARGMNT,ERROR,STAT,AVG,SDENOM=
     *'/' ',5E12.4)
          ARGMNT=0.0
        END IF
        ERROR=SQRT(ARGMNT/SDENOM)
        IF ((AVG .EQ. 0.)) THEN
          ERROR=EMAX
        ELSE
          ERROR=100.*ERROR/ABS(AVG)
        END IF
        IF((MODE .EQ. 2))AVG=AVG*STAT
4180    CONTINUE
        DATA(N,1)=AVG
        DATA(N,2)=MIN(EMAX,ERROR)
4161  CONTINUE
4162  CONTINUE
      RETURN
      END
      subroutine prepare_alias_sampling(nsbin,fs_array,ws_array,ibin_arr
     *ay)
      implicit none
      integer*4 nsbin,ibin_array(nsbin)
      real*8 fs_array(nsbin),ws_array(nsbin)
      integer*4 i,j_l,j_h
      real*8 sum,aux
      sum = 0
      DO 4201 i=1,nsbin
        IF((fs_array(i) .LT. 1e-30))fs_array(i) = 1e-30
        ws_array(i) = -fs_array(i)
        ibin_array(i) = 1
        sum = sum + fs_array(i)
4201  CONTINUE
4202  CONTINUE
      sum = sum/nsbin
      DO 4211 i=1,nsbin-1
        DO 4221 j_h=1,nsbin
          IF (( ws_array(j_h) .LT. 0 )) THEN
            IF((abs(ws_array(j_h)) .GT. sum))GOTO 4230
          END IF
4221    CONTINUE
4222    CONTINUE
        j_h = nsbin
4230    CONTINUE
          DO 4231 j_l=1,nsbin
          IF (( ws_array(j_l) .LT. 0 )) THEN
            IF((abs(ws_array(j_l)) .LT. sum))GOTO 4240
          END IF
4231    CONTINUE
4232    CONTINUE
        j_l = nsbin
4240    aux = sum - abs(ws_array(j_l))
        ws_array(j_h) = ws_array(j_h) + aux
        ws_array(j_l) = -ws_array(j_l)/sum
        ibin_array(j_l) = j_h
        IF((i .EQ. nsbin-1))ws_array(j_h) = 1
4211  CONTINUE
4212  CONTINUE
      return
      end
      real*8 function alias_sample(nsbin,xs_array,ws_array,ibin_array)
      implicit none
      integer*4 nsbin,ibin_array(nsbin)
      real*8 xs_array(0:nsbin),ws_array(nsbin)
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      real*8 v1,v2,aj
      integer*4 j
      IF((rng_seed .GT. 128))call ranmar_get
      v1 = rng_array(rng_seed)*twom24
      rng_seed = rng_seed + 1
      IF((rng_seed .GT. 128))call ranmar_get
      v2 = rng_array(rng_seed)*twom24
      rng_seed = rng_seed + 1
      aj = 1 + v1*nsbin
      j = aj
      IF((j .GT. nsbin))j = nsbin
      aj = aj - j
      IF (( aj .GT. ws_array(j) )) THEN
        j = ibin_array(j)
      END IF
      alias_sample = (1-v2)*xs_array(j-1) + v2*xs_array(j)
      return
      end
C##############################################################################
C
C   This file was automatically generated by configure version 2.0
C   It contains various subroutines and functions for date, time,
C   CPU time, host name, etc.
C
C   Attention: all changes will be lost the next time you run configure!
C
C##############################################################################


C##############################################################################
C
C  EGSnrc egs_system subroutine v1
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
      integer function egs_system(command)
      character*(*) command
      integer system, istat
      istat = system(command)
      egs_system = istat
      return
      end

C##############################################################################
C
C  EGSnrc egs_isdir subroutine v1
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
C  egs_isdir(file_name)  Returns .true., if the string file_name points to
C                        an existing directory. This version uses the lstat
C                        intrinsic and then tests for bit 14 being set in
C                        the mode element. This works on all Unix systems
C                        that I have access to (Linux, Aix, HP-UX, OSF1,
C                        Solaris, IRIX)
C
C*****************************************************************************

      logical function egs_isdir(file_name)
      implicit none
      character*(*) file_name
      integer*4 lnblnk1, res, array(13), l, lstat
      logical btest
      egs_isdir = .false.
      l = lnblnk1(file_name)
      if( l.lt.len(file_name) ) file_name(l+1:l+1) = char(0)
         ! On some systems lstat only works if the string is 0-terminated
      res = lstat(file_name,array)
      if( l.lt.len(file_name) ) file_name(l+1:l+1) = ' '
      if( res.eq.0 ) then
            ! Amost all compilers that have the lstat intrinsic return the
            ! file mode in the 3rd array element. But the PGI compiler has
            ! its own opinion on the subject and returns it in the 5th element
            ! That's why the relevant element is written as 3
            ! here, 3 gets replaced by the appropriate element
            ! by the configure script.
          if( btest(array(3),14) ) egs_isdir = .true.
      end if
      return
      end

C##############################################################################
C
C  EGSnrc date subroutines v1
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
C
C***************************************************************************

      subroutine egs_fdate(ounit)
      integer ounit
      character*24 string
      call fdate(string)
      write(ounit,'(a,$)') string
      end

C***************************************************************************
C
C   egs_get_fdate(string) assignes a 24 char date and time string to string
C                         string must be at least 24 chars long, otherwise
C                         this subroutine has no effect.
C
C***************************************************************************

      subroutine egs_get_fdate(string)
      character*(*) string
      if( len(string).ge.24 ) call fdate(string)
      return
      end

C##############################################################################
C
C  EGSnrc egs_date_and_time subroutine v1
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


      subroutine egs_date_and_time(vnow)
      integer vnow(8)
      character dat*8,tim*10,zon*5
      call date_and_time(dat,tim,zon,vnow)
      return
      end

C##############################################################################
C
C  EGSnrc egs_date subroutine v1
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
C egs_date(ounit): print a 11 char string in the form
C                     '18-Mar-2003'
C                  to the unit specified by ounit
C                  No end of line character is inserted
C
C*************************************************************************

      subroutine egs_date(ounit)
      integer ounit
      character string*24, dat*11
      call fdate(string)
      dat(1:2) = string(9:10)
      dat(3:3) = '-'
      dat(4:6) = string(5:7)
      dat(7:7) = '-'
      dat(8:11) = string(21:24)
      write(ounit,'(a,$)') dat
      return
      end

C##############################################################################
C
C  EGSnrc egs_time subroutine v1
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


C $Id: egs_time_v1.f,v 1.1 2003/07/11 19:17:08 iwan Exp $
C*************************************************************************
C
C egs_time(ounit): print a 8 char string in the form hh:mm:ss
C                  to the unit specified by ounit
C                  No end of line character is inserted
C
C*************************************************************************

      subroutine egs_time(ounit)
      integer ounit
      character string*24
      call fdate(string)
      write(ounit,'(a,$)') string(12:19)
      return
      end

C##############################################################################
C
C  EGSnrc seconds timing subroutines v1
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
      real t0,t1
      character dat*8,tim*10,zon*5
      integer values(8)
      call date_and_time(dat,tim,zon,values)
      t1 = 3600.*values(5) + 60.*values(6) + values(7) + 0.001*values(8)
      egs_secnds = t1 - t0
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

C##############################################################################
C
C  EGSnrc date and time subroutines
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
C
C Returns a 3-letter abreviation of the day of the week in the string day,
C given a day specified by the integer array values
C   values(1)=year, values(2)=month, values(3)=day
C
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
C
C Same as egs_day_diff above, but assumes that vend specifies a later date
C than vstart.
C
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
C
C Same as egs_time_diff above, but assumes that vend specifies a later date
C than vstart.
C
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
C
C Returns in month a 3-letter abreviation of the month specified by mo, if
C mo is between 1 and 12, or an empty string otherwise.
C
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
C
C Converts a 3-letter abreviation of a month to its corresponding integer
C value, if the string month is a valid month, or -1 otherwise.
C
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


C##############################################################################
C
C  EGSnrc egs_etime subroutine
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
C real function egs_etime(): returns CPU time consumed since the start of
C                            the program
C
C*****************************************************************************

      real function egs_etime()
      real tarray(2),etime
      egs_etime = etime(tarray)
      return
      end

C##############################################################################
C
C  EGSnrc canonical system name subroutines
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


C******************************************************************************
C
C Print the canonical system name as determined by the config.guess script
C or the Windows installation program to the unit specified by ounit.
C
C*****************************************************************************

      subroutine egs_print_canonical_system(ounit)
      integer ounit
      write(6,'(a,$)') 'x86_64-unknown-linux-gnu'
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
      integer l1,l2
      l1 = lnblnk1('x86_64-unknown-linux-gnu')
      l2 = len(res)
      res(:l2) = ' '
      if( l2.ge.l1 ) then
        res(:l1) = 'x86_64-unknown-linux-gnu'
      else
        res(:l2) = 'x86_64-unknown-linux-gnu'
      end if
      return
      end


C##############################################################################
C
C  EGSnrc configuration name subroutines
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


C******************************************************************************
C
C Print the configuration name as specified suring the configuration
C process to the unit specified by ounit.
C
C*****************************************************************************

      subroutine egs_print_configuration_name(ounit)
      integer ounit
      write(6,'(a,$)') 'cluster'
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
      integer l1,l2
      l1 = lnblnk1('cluster')
      l2 = len(res)
      res(:l2) = ' '
      if( l2.ge.l1 ) then
        res(:l1) = 'cluster'
      else
        res(:l2) = 'cluster'
      end if
      return
      end


C##############################################################################
C
C  EGSnrc hostname subroutines v1
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
C Print the host name to the unit specified by ounit without inserting
C a new line character.
C
C*****************************************************************************

      subroutine egs_print_hostnm(ounit)
      integer ounit
      character*256 string
      integer res,hostnm,lnblnk1
      res = hostnm(string)
      if( res.ne.0 ) then
        write(6,'(a,a)') 'hostnm returned with a non-zero status '
        stop
      end if
      write(ounit,'(a,$)') string(:lnblnk1(string))
      return
      end

C*****************************************************************************
C
C Assign the host name to the string pointed to be hname.
C
C*****************************************************************************

      subroutine egs_get_hostnm(hname)
      character*(*) hname
      character*256 string
      integer res,hostnm,lnblnk1,l1,l2,l
      res = hostnm(string)
      if( res.ne.0 ) then
        write(6,'(a,a)') 'hostnm returned with a non-zero status '
        stop
      end if
      l1 = lnblnk1(string)
      l2 = len(hname)
      hname(:l2) = ' '
      l = min(l1,l2)
      hname(:l) = string(:l)
      return
      end

      subroutine egs_init
      implicit none
      common/my_times/ t_elapsed, t_cpu, t_first
      real*8 t_elapsed, t_cpu
      integer t_first(8)
      real egs_tot_time,egs_etime
      real*8 dum
      call egs_set_defaults
      call egs_check_arguments
      call egs_init1
      return
      end
      subroutine egs_init1
      implicit none
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      COMMON/MISC/  DUNIT,KMPI,KMPO,RHOR(1),MED(1),IRAYLR(1),IPHOTONUCR(
     *1)
      real*8 DUNIT,  RHOR
      integer*4 KMPI,  KMPO
      integer*2 MED,  IRAYLR,  IPHOTONUCR
      common/my_times/ t_elapsed, t_cpu, t_first
      real*8 t_elapsed, t_cpu
      integer t_first(8)
      real egs_tot_time,egs_etime
      integer l, lnblnk1, l1, l2
      integer i
      character arg*256,tmp_string*512, tmp1_string*512, ucode_dir*512,
     *line*80, line1*80,dattim*24
      logical have_input,egs_isdir,egs_strip_extension,ex, on_egs_home,i
     *s_opened
      integer*4 mypid
      integer getpid
      integer istat, egs_system, u, pos1, pos2,egs_get_unit,itmp
      real*8 dum
      t_elapsed = 0
      t_cpu = egs_etime()
      dum = egs_tot_time(1)
      call egs_date_and_time(t_first)
      DO 4251 i=1,len(line)
        line(i:i) = '='
4251  CONTINUE
4252  CONTINUE
      DO 4261 i=1,len(line1)
        line1(i:i) = '.'
4261  CONTINUE
4262  CONTINUE
      IF ((.NOT.is_pegsless)) THEN
        on_egs_home = .false.
        inquire(file=pegs_file,exist=ex)
        IF (( ex )) THEN
          kmpi=egs_get_unit(kmpi)
          IF ((kmpi.LT.0)) THEN
            write(i_log,'(/a)') '***************** Error: '
            write(i_log,*) 'failed to get a free Fortran I/O unit for pe
     *gs file'
            write(i_log,'(/a)') '***************** Quiting now.'
            call exit(1)
          END IF
          open(kmpi,file=pegs_file,status='old',err=4270)
          goto 4280
        END IF
        arg = pegs_file(:lnblnk1(pegs_file))
        ex = egs_strip_extension(arg,'.pegs4dat')
        l = lnblnk1(egs_home)
        l1 = lnblnk1('pegs4data') + 2*lnblnk1('/')
        l2 = lnblnk1(arg) + lnblnk1('.pegs4dat')
        IF (( l + l1 + l2 .GT. 256 )) THEN
          write(i_log,'(/a)') '***************** Warning: '
          write(i_log,*) 'pegs4 data file name (including absolute path)
     *'
          write(i_log,'(a,i4,a)') 'is too long (',l+l1+l2,') characters'
        ELSE
          pegs_file = egs_home(:lnblnk1(egs_home)) // 'pegs4' // '/' //
     *    'data' // '/' // arg(:lnblnk1(arg)) // '.pegs4dat'
          inquire(file=pegs_file,exist=ex)
          IF (( ex )) THEN
            kmpi=egs_get_unit(kmpi)
            IF ((kmpi.LT.0)) THEN
              write(i_log,'(/a)') '***************** Error: '
              write(i_log,*) 'failed to get a free Fortran I/O unit for
     *pegs file'
              write(i_log,'(/a)') '***************** Quiting now.'
              call exit(1)
            END IF
            open(kmpi,file=pegs_file,status='old',err=4270)
            on_egs_home = .true.
            goto 4280
          END IF
        END IF
        l = lnblnk1(hen_house)
        IF (( l + l1 + l2 .GT. 256 )) THEN
          write(i_log,'(/a)') '***************** Warning: '
          write(i_log,*) 'pegs4 data file name (including absolute path)
     *'
          write(i_log,'(a,i4,a)') 'is too long (',l+l1+l2,') characters'
        ELSE
          pegs_file = hen_house(:lnblnk1(hen_house)) // 'pegs4' // '/' /
     *    / 'data' // '/' // arg(:lnblnk1(arg)) // '.pegs4dat'
          inquire(file=pegs_file,exist=ex)
          IF (( ex )) THEN
            kmpi=egs_get_unit(kmpi)
            IF ((kmpi.LT.0)) THEN
              write(i_log,'(/a)') '***************** Error: '
              write(i_log,*) 'failed to get a free Fortran I/O unit for
     *pegs file'
              write(i_log,'(/a)') '***************** Quiting now.'
              call exit(1)
            END IF
            open(kmpi,file=pegs_file,status='old',err=4270)
            goto 4280
          END IF
        END IF
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) 'could not find pegs4 file named ',arg(:lnblnk1(a
     *  rg))
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
4280  CONTINUE
      DO 4291 i=1,len(tmp_string)
        tmp_string(i:i) = ' '
4291  CONTINUE
4292  CONTINUE
      tmp_string = hen_house(:lnblnk1(hen_house)) // 'data' // '/'
      i_nist_data=76
      i_incoh=78
      i_photo_relax=77
      i_photo_cs=79
      i_mscat=11
      DO 4301 i=1,len(tmp1_string)
        tmp1_string(i:i) = ' '
4301  CONTINUE
4302  CONTINUE
      tmp1_string = tmp_string(:lnblnk1(tmp_string)) // 'photo_cs.data'
      inquire(file=tmp1_string,exist=ex,opened=is_opened,number=itmp)
      IF (( .NOT.ex )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) 'EGSnrc data file ','photo_cs.data',' does not ex
     *ist'
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      IF (( .NOT.is_opened )) THEN
        i_photo_cs=egs_get_unit(i_photo_cs)
        IF ((i_photo_cs.LT.0)) THEN
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,*) 'failed to get a free Fortran I/O unit for data
     * file ', tmp1_string(:lnblnk1(tmp1_string))
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        END IF
        open(i_photo_cs,file=tmp1_string,status='old',err=4310)
      ELSE
        i_photo_cs = itmp
      END IF
      DO 4321 i=1,len(tmp1_string)
        tmp1_string(i:i) = ' '
4321  CONTINUE
4322  CONTINUE
      tmp1_string = tmp_string(:lnblnk1(tmp_string)) // 'msnew.data'
      inquire(file=tmp1_string,exist=ex,opened=is_opened,number=itmp)
      IF (( .NOT.ex )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) 'EGSnrc data file ','msnew.data',' does not exist
     *'
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      IF (( .NOT.is_opened )) THEN
        i_mscat=egs_get_unit(i_mscat)
        IF ((i_mscat.LT.0)) THEN
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,*) 'failed to get a free Fortran I/O unit for data
     * file ', tmp1_string(:lnblnk1(tmp1_string))
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        END IF
        open(i_mscat,file=tmp1_string,status='old',err=4310)
      ELSE
        i_mscat = itmp
      END IF
      DO 4331 i=1,len(tmp1_string)
        tmp1_string(i:i) = ' '
4331  CONTINUE
4332  CONTINUE
      tmp1_string = tmp_string(:lnblnk1(tmp_string)) // 'incoh.data'
      inquire(file=tmp1_string,exist=ex,opened=is_opened,number=itmp)
      IF (( .NOT.ex )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) 'EGSnrc data file ','incoh.data',' does not exist
     *'
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      IF (( .NOT.is_opened )) THEN
        i_incoh=egs_get_unit(i_incoh)
        IF ((i_incoh.LT.0)) THEN
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,*) 'failed to get a free Fortran I/O unit for data
     * file ', tmp1_string(:lnblnk1(tmp1_string))
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        END IF
        open(i_incoh,file=tmp1_string,status='old',err=4310)
      ELSE
        i_incoh = itmp
      END IF
      DO 4341 i=1,len(tmp1_string)
        tmp1_string(i:i) = ' '
4341  CONTINUE
4342  CONTINUE
      tmp1_string = tmp_string(:lnblnk1(tmp_string)) // 'photo_relax.dat
     *a'
      inquire(file=tmp1_string,exist=ex,opened=is_opened,number=itmp)
      IF (( .NOT.ex )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) 'EGSnrc data file ','photo_relax.data',' does not
     * exist'
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      IF (( .NOT.is_opened )) THEN
        i_photo_relax=egs_get_unit(i_photo_relax)
        IF ((i_photo_relax.LT.0)) THEN
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,*) 'failed to get a free Fortran I/O unit for data
     * file ', tmp1_string(:lnblnk1(tmp1_string))
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        END IF
        open(i_photo_relax,file=tmp1_string,status='old',err=4310)
      ELSE
        i_photo_relax = itmp
      END IF
      DO 4351 i=1,len(ucode_dir)
        ucode_dir(i:i) = ' '
4351  CONTINUE
4352  CONTINUE
      ucode_dir = egs_home(:lnblnk1(egs_home)) // user_code(:lnblnk1(use
     *r_code)) // '/'
      have_input = .false.
      i_input=5
      IF (( lnblnk1(input_file) .GT. 0 )) THEN
        have_input = .true.
        l = lnblnk1(egs_home)
        l1 = lnblnk1(user_code)+1
        l2 = lnblnk1(input_file) + lnblnk1('.egsinp')
        IF (( l + l1 + l2 .GT. 1024 )) THEN
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,*) 'input file name (including path) is too long '
     *    ,l+l1+l2
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        END IF
        ex = egs_strip_extension(input_file,'.egsinp')
        tmp_string = ucode_dir(:lnblnk1(ucode_dir)) // input_file(:lnbln
     *  k1(input_file)) // '.egsinp'
        inquire(file=tmp_string,exist=ex,opened=is_opened)
        IF (( .NOT.ex )) THEN
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,*) 'Input file ',tmp_string(:lnblnk1(tmp_string)),
     *    ' does not exist.'
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        END IF
        IF ((.NOT.is_opened)) THEN
          open(i_input,file=tmp_string,status='old',err=4360)
        ELSE
          rewind(i_input)
        END IF
      END IF
      DO 4371 i=1,len(work_dir)
        work_dir(i:i) = ' '
4371  CONTINUE
4372  CONTINUE
      work_dir = 'egsrun_'
      mypid = getpid()
      call egs_itostring(work_dir,mypid,.false.)
      call egs_get_hostnm(host_name)
      IF((lnblnk1(host_name) .LT. 1))host_name = 'unknown'
      IF (( have_input )) THEN
        work_dir = work_dir(:lnblnk1(work_dir)) // '_' // input_file(:ln
     *  blnk1(input_file)) // '_' // host_name(:lnblnk1(host_name)) // '
     */'
      ELSE
        work_dir = work_dir(:lnblnk1(work_dir)) // '_noinput_' // host_n
     *  ame(:lnblnk1(host_name)) // '/'
      END IF
      DO 4381 i=1,len(tmp_string)
        tmp_string(i:i) = ' '
4381  CONTINUE
4382  CONTINUE
      tmp_string = ucode_dir(:lnblnk1(ucode_dir)) // work_dir(:lnblnk1(w
     *ork_dir))
      DO 4391 i=1,lnblnk1(tmp_string)
        IF (( tmp_string(i:i) .EQ. '/' )) THEN
          tmp_string(i:i) = '/'
        END IF
4391  CONTINUE
4392  CONTINUE
      ex = egs_isdir(tmp_string)
      IF (( ex )) THEN
        work_dir = 'egsrun_p_' // input_file(:lnblnk1(input_file)) // ho
     *  st_name(:lnblnk1(host_name)) // '/'
        tmp_string = ucode_dir(:lnblnk1(ucode_dir)) // work_dir(:lnblnk1
     *  (work_dir))
      END IF
      tmp1_string = 'mkdir ' // tmp_string(:lnblnk1(tmp_string))
      l = lnblnk1(tmp1_string)
      tmp1_string(l+1:l+1) = char(0)
      istat = egs_system(tmp1_string)
      IF (( istat .NE. 0 )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) 'failed to create working directory ',tmp1_string
     *  (:lnblnk1(tmp1_string))
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      call egs_open_units(.true.)
      write(i_log,'(a)') line
      write(i_log,'(a,a,t55,a,$)') 'EGSnrc version 4 for ','x86_64-unkno
     *wn-linux-gnu',' '
      call egs_get_fdate(dattim)
      write(i_log,'(a,/,a)') dattim,line
      pos1 = lnblnk1('output file(s)')
      pos2 = 80 - lnblnk1('cluster')
      pos2 = min(pos2,80-lnblnk1(user_code))
      DO 4401 i=1,len(tmp_string)
        tmp_string(i:i) = ' '
4401  CONTINUE
4402  CONTINUE
      tmp_string = pegs_file
      call egs_strip_path(tmp_string)
      ex = egs_strip_extension(tmp_string,'.pegs4dat')
      IF (( on_egs_home )) THEN
        tmp_string = tmp_string(:lnblnk1(tmp_string)) // ' on EGS_HOME'
      ELSE
        tmp_string = tmp_string(:lnblnk1(tmp_string)) // ' on HEN_HOUSE'
      END IF
      IF (( lnblnk1(tmp_string) .GT. lnblnk1(pegs_file) )) THEN
        DO 4411 i=1,len(tmp_string)
          tmp_string(i:i) = ' '
4411    CONTINUE
4412    CONTINUE
        tmp_string = pegs_file
      END IF
      pos2 = min(pos2,80-lnblnk1(tmp_string))
      pos2 = min(pos2,80-lnblnk1(host_name))
      IF((have_input))pos2 = min(pos2,80-lnblnk1(input_file))
      pos2 = min(pos2,80-lnblnk1(output_file))
      IF((pos2 .LT. pos1+2))pos2 = pos1 + 2
      write(i_log,'(a,$)') 'configuration'
      l = pos2 - lnblnk1('configuration')
      write(i_log,'(a,$)') line1(:l)
      write(i_log,'(a)') 'cluster'
      write(i_log,'(a,$)') 'user code'
      l = pos2 - lnblnk1('user code')
      write(i_log,'(a,$)') line1(:l)
      write(i_log,'(a)') user_code(:lnblnk1(user_code))
      write(i_log,'(a,$)') 'pegs file'
      l = pos2 - lnblnk1('pegs file')
      write(i_log,'(a,$)') line1(:l)
      write(i_log,'(a)') tmp_string(:lnblnk1(tmp_string))
      write(i_log,'(a,$)') 'using host'
      l = pos2 - lnblnk1('using host')
      write(i_log,'(a,$)') line1(:l)
      write(i_log,'(a)') host_name(:lnblnk1(host_name))
      IF (( have_input )) THEN
        write(i_log,'(a,$)') 'input file'
        l = pos2 - lnblnk1('input file')
        write(i_log,'(a,$)') line1(:l)
        write(i_log,'(a)') input_file(:lnblnk1(input_file))
      END IF
      write(i_log,'(a,$)') 'output file(s)'
      l = pos2 - lnblnk1('output file(s)')
      write(i_log,'(a,$)') line1(:l)
      write(i_log,'(a)') output_file(:lnblnk1(output_file))
      IF (( n_parallel .GT. 0 )) THEN
        write(i_log,'(a,$)') 'number of parallel jobs'
        l = pos2 - lnblnk1('number of parallel jobs')
        write(i_log,'(a,$)') line1(:l)
        write(i_log,'(i2)') n_parallel
        write(i_log,'(a,$)') 'job number'
        l = pos2 - lnblnk1('job number')
        write(i_log,'(a,$)') line1(:l)
        write(i_log,'(i2)') i_parallel
      END IF
      write(i_log,'(a)') line
      return
4360  write(i_log,'(/a)') '***************** Error: '
      write(i_log,*) 'failed to open input file ',tmp_string(:lnblnk1(tm
     *p_string))
      write(i_log,'(/a)') '***************** Quiting now.'
      call exit(1)
4270  write(i_log,'(/a)') '***************** Error: '
      write(i_log,*) 'failed to open existing pegs file ',pegs_file(:lnb
     *lnk1(pegs_file))
      write(i_log,'(/a)') '***************** Quiting now.'
      call exit(1)
4310  write(i_log,'(/a)') '***************** Error: '
      write(i_log,*) 'failed to open EGSnrc data file ',tmp1_string(:lnb
     *lnk1(tmp1_string))
      write(i_log,'(/a)') '***************** Quiting now.'
      call exit(1)
      return
      end
      subroutine egs_check_arguments
      implicit none
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      character arg*256,tmp_string*512, line1*80
      logical have_arg,egs_isdir,egs_strip_extension,ex, on_egs_home
      integer narg, iargc, i, lnblnk1, l, l2,i_help,egs_get_unit
      narg = iargc()
      IF((narg .LT. 1))return
      have_arg = .false.
      DO 4421 i=1,narg-1
        call getarg(i,arg)
        l = lnblnk1(arg)
        IF (( ( l .EQ. lnblnk1('-H') .AND. arg(:l) .EQ. '-H' ) .OR. ( l
     *  .EQ. lnblnk1('--hen-house') .AND. arg(:l) .EQ. '--hen-house' ) )
     *  ) THEN
          have_arg = .true.
          call getarg(i+1,arg)
          GO TO4422
        END IF
4421  CONTINUE
4422  CONTINUE
      IF (( have_arg )) THEN
        l = lnblnk1(arg)
        DO 4431 i=1,len(hen_house)
          hen_house(i:i) = ' '
4431    CONTINUE
4432    CONTINUE
        IF (( l .GT. 0 )) THEN
          IF (( l .GT. 254 )) THEN
            write(i_log,'(/a)') '***************** Error: '
            write(i_log,'(a,i5)') ' HEN_HOUSE argument is too long',l
            write(i_log,'(/a)') '***************** Quiting now.'
            call exit(1)
          END IF
          hen_house(:l) = arg(:lnblnk1(arg))
          IF((hen_house(l:l) .NE. '/'))hen_house(l+1:l+1) = '/'
        ELSE
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,'(a)') ' empty argument after -H'
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        END IF
        DO 4441 i=1,lnblnk1(hen_house)
          IF (( hen_house(i:i) .EQ. '/' )) THEN
            hen_house(i:i) = '/'
          END IF
4441    CONTINUE
4442    CONTINUE
      END IF
      IF (( .NOT.egs_isdir(hen_house) )) THEN
        write(i_log,'(/a)') '***************** Warning: '
        write(i_log,'(a,a)') ' HEN_HOUSE directory ',hen_house(:lnblnk1(
     *  hen_house))
        write(i_log,'(a)') 'does not exist. Hope you know what you are d
     *oing.'
      END IF
      have_arg = .false.
      DO 4451 i=1,narg
        call getarg(i,arg)
        l = lnblnk1(arg)
        IF (( ( l .EQ. lnblnk1('-h') .AND. arg(:l) .EQ. '-h' ) .OR. ( l
     *  .EQ. lnblnk1('--help') .AND. arg(:l) .EQ. '--help' ) )) THEN
          have_arg = .true.
          GO TO4452
        END IF
4451  CONTINUE
4452  CONTINUE
      IF (( have_arg )) THEN
        call getarg(0,arg)
        call egs_strip_path(arg)
        write(i_log,'(//,a,a,a,//)') 'Usage: ',arg(:lnblnk1(arg)),' [arg
     *s] '
        tmp_string = hen_house(:lnblnk1(hen_house)) // 'pieces/help_mess
     *age'
        i_help=98
        i_help=egs_get_unit(i_help)
        IF ((i_help.LT.0)) THEN
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,*) 'failed to get a free Fortran I/O unit for help
     * file'
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        END IF
        open(i_help,file=tmp_string,status='old',err=4460)
4471    CONTINUE
          read(i_help,'(a)',err=4480,end=4480) line1
          write(i_log,'(a)') line1
        GO TO 4471
4472    CONTINUE
4480    CONTINUE
        call exit(0)
4460    CONTINUE
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) 'Did not find the help_message file!'
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      have_arg = .false.
      DO 4491 i=1,narg
        call getarg(i,arg)
        l = lnblnk1(arg)
        IF (( ( l .EQ. lnblnk1('-b') .AND. arg(:l) .EQ. '-b' ) .OR. ( l
     *  .EQ. lnblnk1('--batch') .AND. arg(:l) .EQ. '--batch' ) )) THEN
          have_arg = .true.
          GO TO4492
        END IF
4491  CONTINUE
4492  CONTINUE
      IF((have_arg))is_batch = .true.
      have_arg = .false.
      DO 4501 i=1,narg-1
        call getarg(i,arg)
        l = lnblnk1(arg)
        IF (( ( l .EQ. lnblnk1('-P') .AND. arg(:l) .EQ. '-P' ) .OR. ( l
     *  .EQ. lnblnk1('--parallel') .AND. arg(:l) .EQ. '--parallel' ) ))
     *  THEN
          have_arg = .true.
          call getarg(i+1,arg)
          GO TO4502
        END IF
4501  CONTINUE
4502  CONTINUE
      IF (( have_arg )) THEN
        read(arg,*,err=4510) n_parallel
        IF((n_parallel .LT. 0))goto 4510
        goto 4520
4510    CONTINUE
        write(i_log,'(/a)') '***************** Warning: '
        write(i_log,*) ' Wrong/missing parallel job number argument, -P
     *option ignored'
        n_parallel = 0
4520    CONTINUE
      END IF
      have_arg = .false.
      DO 4531 i=1,narg-1
        call getarg(i,arg)
        l = lnblnk1(arg)
        IF (( ( l .EQ. lnblnk1('-j') .AND. arg(:l) .EQ. '-j' ) .OR. ( l
     *  .EQ. lnblnk1('--job') .AND. arg(:l) .EQ. '--job' ) )) THEN
          have_arg = .true.
          call getarg(i+1,arg)
          GO TO4532
        END IF
4531  CONTINUE
4532  CONTINUE
      IF (( have_arg )) THEN
        read(arg,*,err=4540) i_parallel
        IF((i_parallel .LT. 0))goto 4540
        goto 4550
4540    CONTINUE
        write(i_log,'(/a)') '***************** Warning: '
        write(i_log,*) ' Wrong/missing job argument, -j option ognored'
        i_parallel = 0
4550    CONTINUE
      END IF
      have_arg = .false.
      DO 4561 i=1,narg-1
        call getarg(i,arg)
        l = lnblnk1(arg)
        IF (( ( l .EQ. lnblnk1('-f') .AND. arg(:l) .EQ. '-f' ) .OR. ( l
     *  .EQ. lnblnk1('--first-job') .AND. arg(:l) .EQ. '--first-job' ) )
     *  ) THEN
          have_arg = .true.
          call getarg(i+1,arg)
          GO TO4562
        END IF
4561  CONTINUE
4562  CONTINUE
      IF (( have_arg )) THEN
        read(arg,*,err=4570) first_parallel
        IF((first_parallel .LT. 1))goto 4570
        goto 4580
4570    CONTINUE
        write(i_log,'(/a)') '***************** Warning: '
        write(i_log,*) ' Wrong/missing first job argument, -f option ogn
     *ored'
        first_parallel = 1
4580    CONTINUE
      END IF
      IF (( n_parallel .GT. 0 .OR. i_parallel .GT. 0 )) THEN
        IF (( n_parallel*i_parallel .EQ. 0 )) THEN
          write(i_log,'(/a)') '***************** Warning: '
          write(i_log,*) 'You need to specify number of jobs AND job num
     *ber ', '=> will not use parallel run '
          n_parallel = 0
          i_parallel = 0
        END IF
        IF (( first_parallel .GT. i_parallel )) THEN
          write(i_log,'(/a)') '***************** Warning: '
          write(i_log,*) 'i_parallel (',i_parallel, ') can not be smalle
     *r than first_parallel (',first_parallel,')'
          first_parallel = i_parallel
        END IF
      END IF
      have_arg = .false.
      DO 4591 i=1,narg-1
        call getarg(i,arg)
        l = lnblnk1(arg)
        IF (( ( l .EQ. lnblnk1('-e') .AND. arg(:l) .EQ. '-e' ) .OR. ( l
     *  .EQ. lnblnk1('--egs-home') .AND. arg(:l) .EQ. '--egs-home' ) ))
     *  THEN
          have_arg = .true.
          call getarg(i+1,arg)
          GO TO4592
        END IF
4591  CONTINUE
4592  CONTINUE
      IF (( have_arg )) THEN
        l = lnblnk1(arg)
        DO 4601 i=1,len(egs_home)
          egs_home(i:i) = ' '
4601    CONTINUE
4602    CONTINUE
        IF (( l .EQ. 0 )) THEN
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,'(a)') ' empty argument after -e'
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        END IF
        IF (( l .GT. 254 )) THEN
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,'(a,i5)') ' EGS_HOME argument is too long ',l
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        END IF
        egs_home(:l) = arg(:lnblnk1(arg))
        IF((egs_home(l:l) .NE. '/'))egs_home(l+1:l+1) = '/'
        DO 4611 i=1,lnblnk1(egs_home)
          IF (( egs_home(i:i) .EQ. '/' )) THEN
            egs_home(i:i) = '/'
          END IF
4611    CONTINUE
4612    CONTINUE
      END IF
      IF (( .NOT.egs_isdir(egs_home) )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) ' EGS_HOME directory ',egs_home(:lnblnk1(egs_home
     *  )),' does not exist.'
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      on_egs_home = .false.
      is_pegsless=.false.
      have_arg = .false.
      DO 4621 i=1,narg-1
        call getarg(i,arg)
        l = lnblnk1(arg)
        IF (( ( l .EQ. lnblnk1('-p') .AND. arg(:l) .EQ. '-p' ) .OR. ( l
     *  .EQ. lnblnk1('--pegs-file') .AND. arg(:l) .EQ. '--pegs-file' ) )
     *  ) THEN
          have_arg = .true.
          call getarg(i+1,arg)
          GO TO4622
        END IF
4621  CONTINUE
4622  CONTINUE
      IF (( .NOT.have_arg )) THEN
        write(i_log,'(/a)') '***************** Warning: '
        write(i_log,*) 'No pegs4 file name supplied.  Will assume you ar
     *e running    in pegs-less mode with media details specified in inp
     *ut file.'
        is_pegsless=.true.
      ELSE
        pegs_file = arg(:lnblnk1(arg))
      END IF
      call egs_get_usercode(user_code)
      have_arg = .false.
      DO 4631 i=1,narg-1
        call getarg(i,arg)
        l = lnblnk1(arg)
        IF (( ( l .EQ. lnblnk1('-i') .AND. arg(:l) .EQ. '-i' ) .OR. ( l
     *  .EQ. lnblnk1('--input') .AND. arg(:l) .EQ. '--input' ) )) THEN
          have_arg = .true.
          call getarg(i+1,arg)
          GO TO4632
        END IF
4631  CONTINUE
4632  CONTINUE
      IF (( have_arg )) THEN
        ex = egs_strip_extension(arg,'.egsinp')
        l2 = lnblnk1(arg) + lnblnk1('.egsinp')
        IF (( l2 .GT. 256 )) THEN
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,*) 'input file name is too long ',l2
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        END IF
        input_file = arg(:lnblnk1(arg))
      END IF
      have_arg = .false.
      DO 4641 i=1,narg-1
        call getarg(i,arg)
        l = lnblnk1(arg)
        IF (( ( l .EQ. lnblnk1('-o') .AND. arg(:l) .EQ. '-o' ) .OR. ( l
     *  .EQ. lnblnk1('--output') .AND. arg(:l) .EQ. '--output' ) )) THEN
          have_arg = .true.
          call getarg(i+1,arg)
          GO TO4642
        END IF
4641  CONTINUE
4642  CONTINUE
      IF (( have_arg )) THEN
        l = lnblnk1(arg)
        IF (( l .GT. 256 )) THEN
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,*) 'output file name is too long ',l
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        END IF
        output_file(:l) = arg(:lnblnk1(arg))
      ELSE
        IF (( lnblnk1(input_file) .GT. 0 )) THEN
          output_file(:lnblnk1(input_file)) = input_file(:lnblnk1(input_
     *    file))
        ELSE
          output_file = 'test'
        END IF
      END IF
      return
      end
      subroutine egs_open_units(flag)
      implicit none
      logical flag
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      character tmp_string*1024, tmp1_string*1024, tmp2_string*1024, uco
     *de_dir*1024, input_line*100, arg*20
      integer i,lnblnk1,u,l,istart,egs_get_unit,i_iofile
      logical ex,is_open
      DO 4651 i=1,len(tmp_string)
        tmp_string(i:i) = ' '
4651  CONTINUE
4652  CONTINUE
      DO 4661 i=1,len(ucode_dir)
        ucode_dir(i:i) = ' '
4661  CONTINUE
4662  CONTINUE
      ucode_dir = egs_home(:lnblnk1(egs_home)) // user_code(:lnblnk1(use
     *r_code)) // '/'
      IF (( flag )) THEN
        tmp_string = ucode_dir(:lnblnk1(ucode_dir)) // work_dir(:lnblnk1
     *  (work_dir))
      ELSE
        tmp_string = ucode_dir(:lnblnk1(ucode_dir))
      END IF
      tmp_string = tmp_string(:lnblnk1(tmp_string)) // output_file(:lnbl
     *nk1(output_file))
      IF (( i_parallel .GT. 0 )) THEN
        tmp_string = tmp_string(:lnblnk1(tmp_string)) // '_w'
        call egs_itostring(tmp_string,i_parallel,.false.)
      END IF
      DO 4671 i=1,len(tmp1_string)
        tmp1_string(i:i) = ' '
4671  CONTINUE
4672  CONTINUE
      i_log=6
      IF (( is_batch )) THEN
        tmp1_string = tmp_string(:lnblnk1(tmp_string)) // '.egslog'
        open(i_log,file=tmp1_string,status='unknown',err=4680)
      END IF
      DO 4691 i=1,len(tmp2_string)
        tmp2_string(i:i) = ' '
4691  CONTINUE
4692  CONTINUE
      tmp2_string = ucode_dir(:lnblnk1(ucode_dir)) // user_code(:lnblnk1
     *(user_code)) // '.io'
      inquire(file=tmp2_string,exist=ex)
      n_files = 0
      IF (( ex )) THEN
        i_iofile=99
        i_iofile=egs_get_unit(i_iofile)
        IF ((i_iofile.LT.1)) THEN
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,*) 'failed to get a free Fortran I/O unit for .io
     *file'
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        END IF
        open(i_iofile,file=tmp2_string,status='old',err=4700)
4711    CONTINUE
          read(i_iofile,'(a)',err=4720,end=4720) input_line
          IF((input_line(1:1) .EQ. '#'))GO TO4711
          read(input_line,*,err=4730,end=4730) u
          istart = 1
          DO 4741 i=lnblnk1(input_line),1,-1
            IF (( input_line(i:i) .EQ. ' ' )) THEN
              istart = i+1
              GO TO4742
            END IF
4741      CONTINUE
4742      CONTINUE
          DO 4751 i=1,len(arg)
            arg(i:i) = ' '
4751      CONTINUE
4752      CONTINUE
          DO 4761 i=istart,lnblnk1(input_line)
            arg(i+1-istart:i+1-istart) = input_line(i:i)
4761      CONTINUE
4762      CONTINUE
          inquire(unit=u,opened=is_open)
          IF (( is_open )) THEN
            write(i_log,'(/a)') '***************** Warning: '
            write(i_log,'(a,i3,a,a,a,/,a,/,a,/)') 'Unit ',u,' which you
     *want to connect to a ', arg(:lnblnk1(arg)),' file ', 'is already i
     *n use. Will assume this code is being used as', 'a shared library
     *source and this file will be opened explicitly.'
          ELSE
            n_files = n_files + 1
            IF (( n_files .GT. 20 )) THEN
              write(i_log,'(/a)') '***************** Error: '
              write(i_log,*) 'Too many units requested in .io.', ' Incre
     *as $mx_units and retry'
              write(i_log,'(/a)') '***************** Quiting now.'
              call exit(1)
            END IF
            file_units(n_files) = u
            DO 4771 i=1,len(file_extensions(n_files))
              file_extensions(n_files)(i:i) = ' '
4771        CONTINUE
4772        CONTINUE
            l = lnblnk1(arg)
            IF (( l .GT. 10 )) THEN
              write(i_log,'(/a)') '***************** Error: '
              write(i_log,*) 'extension ',arg(:lnblnk1(arg)),' is longer
     * than ', 10,' chars. ', 'Increase $max_extension_length and retry
     *'
              write(i_log,'(/a)') '***************** Quiting now.'
              call exit(1)
            END IF
            file_extensions(n_files) = arg(:lnblnk1(arg))
            tmp1_string = tmp_string(:lnblnk1(tmp_string)) // arg(:lnbln
     *      k1(arg))
            open(u,file=tmp1_string,status='unknown')
          END IF
4730      CONTINUE
        GO TO 4711
4712    CONTINUE
4720    close(i_iofile)
      END IF
      return
4680  write(i_log,'(/a)') '***************** Error: '
      write(i_log,*) 'failed to open output file ',tmp1_string(:lnblnk1(
     *tmp1_string))
      write(i_log,'(/a)') '***************** Quiting now.'
      call exit(1)
4700  write(i_log,'(/a)') '***************** Error: '
      write(i_log,*) 'failed to open existing .io file',tmp2_string(:lnb
     *lnk1(tmp2_string))
      write(i_log,'(/a)') '***************** Quiting now.'
      call exit(1)
      return
      end
      subroutine egs_finish
      implicit none
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      common/my_times/ t_elapsed, t_cpu, t_first
      real*8 t_elapsed, t_cpu
      integer t_first(8)
      real egs_tot_time,egs_etime
      character line*80,base*512,base1*512,tmp_string*512,junk_file*128,
     *fname*512
      character dattim*24
      integer i,l,lnblnk1,istat,egs_system,n_open,unlink,i_junk,egs_get_
     *unit
      logical is_open,egs_isdir
      real*8 t1,t2,tt_cpu
      DO 4781 i=1,len(line)
        line(i:i) = '='
4781  CONTINUE
4782  CONTINUE
      IF (( n_parallel .EQ. 0 .OR. i_parallel .GT. 0 )) THEN
        t_elapsed = egs_tot_time(1)
        tt_cpu = egs_etime() - t_cpu
        t1 = t_elapsed
        t2 = t1/3600
        write(i_log,'(//a,/,a,/)') line,'Finished simulation'
        write(i_log,'(2x,a,t30,f9.1,a,f7.3,a)') 'Elapsed time: ',t1,' s
     *(',t2,' h)'
        t1 = tt_cpu
        t2 = t1/3600
        write(i_log,'(2x,a,t30,f9.1,a,f7.3,a)') 'CPU time:',t1,' s (',t2
     *  ,' h)'
        write(i_log,'(2x,a,t30,f10.3)') 'Ratio:',t_elapsed/tt_cpu
      END IF
      call egs_get_fdate(dattim)
      write(i_log,'(//a,t56,a,/,a)') 'End of run ',dattim,line
      n_open=0
      DO 4791 i=1,len(base)
        base(i:i) = ' '
4791  CONTINUE
4792  CONTINUE
      base = egs_home(:lnblnk1(egs_home)) // user_code(:lnblnk1(user_cod
     *e))
      DO 4801 i=1,99
        IF (( is_batch .OR. i .NE. i_log )) THEN
          inquire(i,opened=is_open)
          IF (( is_open )) THEN
            inquire(i,name=fname)
            IF ((index(fname(:lnblnk1(fname)),base(:lnblnk1(base))).GT.0
     *      )) THEN
              close(i)
              n_open = n_open+1
            END IF
          END IF
        END IF
4801  CONTINUE
4802  CONTINUE
      IF (( lnblnk1(work_dir) .EQ. 0 )) THEN
        return
      END IF
      DO 4811 i=1,len(base)
        base(i:i) = ' '
4811  CONTINUE
4812  CONTINUE
      base = egs_home(:lnblnk1(egs_home)) // user_code(:lnblnk1(user_cod
     *e)) // '/' // work_dir(:lnblnk1(work_dir))
      DO 4821 i=1,lnblnk1(base)
        IF (( base(i:i) .EQ. '/' )) THEN
          base(i:i) = '/'
        END IF
4821  CONTINUE
4822  CONTINUE
      IF (( egs_isdir(base) )) THEN
        DO 4831 i=1,len(tmp_string)
          tmp_string(i:i) = ' '
4831    CONTINUE
4832    CONTINUE
        DO 4841 i=1,len(junk_file)
          junk_file(i:i) = ' '
4841    CONTINUE
4842    CONTINUE
        junk_file = work_dir(:lnblnk1(work_dir))
        l = lnblnk1(junk_file)
        junk_file(l:l) = ' '
        junk_file = junk_file(:lnblnk1(junk_file)) // '_junk'
        tmp_string = base(:lnblnk1(base)) // junk_file(:lnblnk1(junk_fil
     *  e))
        i_junk=99
        i_junk=egs_get_unit(i_junk)
        IF ((i_junk.LT.0)) THEN
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,*) 'failed to get a free Fortran I/O unit for junk
     * file'
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        END IF
        open(i_junk,file=tmp_string,status='unknown')
        write(i_junk,*) 'junk'
        close(i_junk)
        DO 4851 i=1,len(base1)
          base1(i:i) = ' '
4851    CONTINUE
4852    CONTINUE
        base = egs_home(:lnblnk1(egs_home)) // user_code(:lnblnk1(user_c
     *  ode)) // '/' // work_dir(:lnblnk1(work_dir))
        base1 = egs_home(:lnblnk1(egs_home)) // user_code(:lnblnk1(user_
     *  code))
        DO 4861 i=1,len(tmp_string)
          tmp_string(i:i) = ' '
4861    CONTINUE
4862    CONTINUE
        tmp_string = 'mv -f ' // base(:lnblnk1(base)) // '*  ' // base1(
     *  :lnblnk1(base1))
        l = lnblnk1(tmp_string)+1
        tmp_string(l:l) = char(0)
        istat = egs_system(tmp_string)
        IF (( istat .NE. 0 )) THEN
          write(i_log,'(/a)') '***************** Warning: '
          write(i_log,*) 'Moving files from working directory failed ?'
          write(i_log,*) '=> will not remove working directory'
        ELSE
          DO 4871 i=1,len(tmp_string)
            tmp_string(i:i) = ' '
4871      CONTINUE
4872      CONTINUE
          tmp_string = 'rm -rf ' // base(:lnblnk1(base))
          l = lnblnk1(tmp_string)+1
          tmp_string(l:l) = char(0)
          istat = egs_system(tmp_string)
          IF (( istat .NE. 0 )) THEN
            write(i_log,'(/a)') '***************** Warning: '
            write(i_log,*) 'Failed to remove working directory ', work_d
     *      ir(:lnblnk1(work_dir))
          END IF
          DO 4881 i=1,len(tmp_string)
            tmp_string(i:i) = ' '
4881      CONTINUE
4882      CONTINUE
          tmp_string = base1(:lnblnk1(base1)) // '/' // junk_file(:lnbln
     *    k1(junk_file))
          l = lnblnk1(tmp_string)+1
          tmp_string(l:l) = char(0)
          istat = unlink(tmp_string)
        END IF
      END IF
      DO 4891 i=1,len(work_dir)
        work_dir(i:i) = ' '
4891  CONTINUE
4892  CONTINUE
      return
      end
      subroutine egs_set_defaults
      implicit none
      COMMON/BOUNDS/ECUT(1),PCUT(1),VACDST
      real*8 ECUT,  PCUT,  VACDST
      COMMON/BREMPR/ DL1(8,1),DL2(8,1),DL3(8,1),DL4(8,1),DL5(8,1),DL6(8,
     *1), ALPHI(2,1),BPAR(2,1),DELPOS(2,1), WA(1,50),PZ(1,50),ZELEM(1,50
     *),RHOZ(1,50), PWR2I(50), DELCM(1),ZBRANG(1),LZBRANG(1),NNE(1), IBR
     *DST,IPRDST,ibr_nist,pair_nrc,itriplet, ASYM(1,50,2)
      CHARACTER*4 ASYM
      real*8 DL1,DL2,DL3,DL4,DL5,DL6,   ALPHI,  BPAR,  DELPOS,  WA,  PZ,
     *  ZELEM,  RHOZ,  PWR2I,  DELCM,  ZBRANG,  LZBRANG
      integer*4 NNE,  IBRDST,  IPRDST,  ibr_nist,  itriplet,  pair_nrc
      common/compton_data/ iz_array(1538),  be_array(1538),  Jo_array(15
     *38),  erfJo_array(1538),   ne_array(1538),  shn_array(1538),
     *shell_array(200,1), eno_array(200,1), eno_atbin_array(200,1), n_sh
     *ell(1), radc_flag,  ibcmp(1)
      integer*4 iz_array,ne_array,shn_array,eno_atbin_array, shell_array
     *,n_shell,radc_flag
      real*8 be_array,Jo_array,erfJo_array,eno_array
      integer*2 ibcmp
      COMMON/EDGE/binding_energies(30,100), interaction_prob(6,100), rel
     *axation_prob(39,100), edge_energies(16,100), edge_number(100), edg
     *e_a(16,100), edge_b(16,100), edge_c(16,100), edge_d(16,100), IEDGF
     *L(1),IPHTER(1)
      real*8 binding_energies,  interaction_prob,    relaxation_prob,  e
     *dge_energies,  edge_a,edge_b,edge_c,edge_d
      integer*2 IEDGFL,  IPHTER
      integer*4 edge_number
      COMMON/ELECIN/ esig_e(1),psig_e(1), esige_max, psige_max, range_ep
     *(0:1,500,1), E_array(500,1), etae_ms0(500,1),etae_ms1(500,1),etap_
     *ms0(500,1),etap_ms1(500,1),q1ce_ms0(500,1),q1ce_ms1(500,1),q1cp_ms
     *0(500,1),q1cp_ms1(500,1),q2ce_ms0(500,1),q2ce_ms1(500,1),q2cp_ms0(
     *500,1),q2cp_ms1(500,1),blcce0(500,1),blcce1(500,1), EKE0(1),EKE1(1
     *), XR0(1),TEFF0(1),BLCC(1),XCC(1), ESIG0(500,1),ESIG1(500,1),PSIG0
     *(500,1),PSIG1(500,1),EDEDX0(500,1),EDEDX1(500,1),PDEDX0(500,1),PDE
     *DX1(500,1),EBR10(500,1),EBR11(500,1),PBR10(500,1),PBR11(500,1),PBR
     *20(500,1),PBR21(500,1),TMXS0(500,1),TMXS1(500,1), expeke1(1), IUNR
     *ST(1),EPSTFL(1),IAPRIM(1), sig_ismonotone(0:1,1)
      real*8 esig_e,   psig_e,   esige_max,  psige_max,  range_ep,  E_ar
     *ray,  etae_ms0,etae_ms1,  etap_ms0,etap_ms1,  q1ce_ms0,q1ce_ms1,
     *q1cp_ms0,q1cp_ms1,  q2ce_ms0,q2ce_ms1,  q2cp_ms0,q2cp_ms1,  blcce0
     *,blcce1,   expeke1,  EKE0,EKE1, XR0,  TEFF0,  BLCC,  XCC,  ESIG0,E
     *SIG1,  PSIG0,PSIG1,  EDEDX0,EDEDX1,  PDEDX0,PDEDX1,  EBR10,EBR11,
     * PBR10,PBR11,  PBR20,PBR21,  TMXS0,TMXS1
      integer*4 IUNRST,  EPSTFL,  IAPRIM
      logical sig_ismonotone
      COMMON/EPCONT/EDEP,EDEP_LOCAL,TSTEP,TUSTEP,USTEP,TVSTEP,VSTEP, RHO
     *F,EOLD,ENEW,EKE,ELKE,GLE,E_RANGE, x_final,y_final,z_final, u_final
     *,v_final,w_final, IDISC,IROLD,IRNEW,IAUSFL(35)
      DOUBLE PRECISION EDEP,  EDEP_LOCAL
      real*8 TSTEP,  TUSTEP,  USTEP,  VSTEP,  TVSTEP,  RHOF,  EOLD,  ENE
     *W,  EKE,  ELKE,  GLE,  E_RANGE, x_final,y_final,z_final,  u_final,
     *v_final,w_final
      integer*4 IDISC,  IROLD,  IRNEW,  IAUSFL
      common/CH_steps/ count_pII_steps,count_all_steps,is_ch_step
      real*8 count_pII_steps,count_all_steps
      logical is_ch_step
      common/ET_control/ smaxir(1),estepe,ximax,  skindepth_for_bca,tran
     *sport_algorithm, bca_algorithm,exact_bca,spin_effects
      real*8 smaxir,  estepe,  ximax,      skindepth_for_bca
      integer*4 transport_algorithm, bca_algorithm
      logical exact_bca,  spin_effects
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      COMMON/MISC/  DUNIT,KMPI,KMPO,RHOR(1),MED(1),IRAYLR(1),IPHOTONUCR(
     *1)
      real*8 DUNIT,  RHOR
      integer*4 KMPI,  KMPO
      integer*2 MED,  IRAYLR,  IPHOTONUCR
      COMMON/PHOTIN/ EBINDA(1), GE0(1),GE1(1), GMFP0(2000,1),GMFP1(2000,
     *1),GBR10(2000,1),GBR11(2000,1),GBR20(2000,1),GBR21(2000,1), RCO0(1
     *),RCO1(1), RSCT0(100,1),RSCT1(100,1), COHE0(2000,1),COHE1(2000,1),
     *  PHOTONUC0(2000,1),PHOTONUC1(2000,1), DPMFP, MPGEM(1,1), NGR(1)
      real*8 EBINDA,  GE0,GE1,  GMFP0,GMFP1,  GBR10,GBR11,  GBR20,GBR21,
     *  RCO0,RCO1,  RSCT0,RSCT1,  COHE0,COHE1,   PHOTONUC0,PHOTONUC1,  D
     *PMFP
      integer*4
     *                  MPGEM,
     *                          NGR
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      COMMON/STACK/ E(50),X(50),Y(50),Z(50),U(50),V(50),W(50),DNEAR(50),
     *WT(50),IQ(50),IR(50),LATCH(50), LATCHI,NP,NPold
      DOUBLE PRECISION E
      real*8 X,Y,Z,  U,V,W,  DNEAR,  WT
      integer*4 IQ,  IR,  LATCH,  LATCHI, NP,  NPold
      COMMON/THRESH/RMT2,RMSQ, AP(1),AE(1),UP(1),UE(1),TE(1),THMOLL(1)
      real*8 RMT2,  RMSQ,  AP,  AE,  UP,  UE,  TE,  THMOLL
      COMMON/UPHIIN/SINC0,SINC1,SIN0(1002),SIN1(1002)
      real*8 SINC0,SINC1,SIN0,SIN1
      COMMON/UPHIOT/THETA,SINTHE,COSTHE,SINPHI, COSPHI,PI,TWOPI,PI5D2
      real*8 THETA,  SINTHE,  COSTHE,  SINPHI,  COSPHI,  PI,TWOPI,PI5D2
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      common/egs_vr/ e_max_rr(1),  prob_RR,  nbr_split,  i_play_RR,
     * i_survived_RR,
     *     n_RR_warning,                                        i_do_rr(
     *1)
      real*8          e_max_rr,prob_RR
      integer*4       nbr_split,i_play_RR,i_survived_RR,n_RR_warning
      integer*2     i_do_rr
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      common/spin_data/ spin_rej(1,0:1,0: 31,0:15,0:31), espin_min,espin
     *_max,espml,b2spin_min,b2spin_max, dbeta2,dbeta2i,dlener,dleneri,dq
     *q1,dqq1i, fool_intel_optimizer
      real*4 spin_rej,espin_min,espin_max,espml,b2spin_min,b2spin_max, d
     *beta2,dbeta2i,dlener,dleneri,dqq1,dqq1i
      logical fool_intel_optimizer
      common/eii_data/ eii_xsection_a( 10000),  eii_xsection_b( 10000),
     * eii_cons(1), eii_a(40),  eii_b(40),  eii_L_factor,  eii_z(40),  e
     *ii_sh(40),  eii_nshells(100),  eii_nsh(1),  eii_first(1,50),  eii_
     *no(1,50),  eii_flag
      real*8 eii_xsection_a,eii_xsection_b,eii_a,eii_b,eii_cons,eii_L_fa
     *ctor
      integer*4 eii_z,eii_sh,eii_nshells
      integer*4 eii_first,eii_no
      integer*4 eii_elements,eii_flag,eii_nsh
      COMMON/rayleigh_inputs/iray_ff_media(1),iray_ff_file(1)
      character*24 iray_ff_media
      character*128 iray_ff_file
      common/emf_inputs/ExIN,EyIN,EzIN,  EMLMTIN,  BxIN, ByIN, BzIN,  Bx
     *, By, Bz,  Bx_new, By_new, Bz_new,  emfield_on
      real*8 ExIN,EyIN,EzIN, EMLMTIN, BxIN,ByIN,BzIN, Bx,By,Bz, Bx_new,B
     *y_new,Bz_new
      logical emfield_on
      common/x_options/eadl_relax,  mcdf_pe_xsections
      logical eadl_relax, mcdf_pe_xsections
      integer i,j,lnblnk1
      CHARACTER*4 MEDIA1(24)
      EQUIVALENCE(MEDIA1(1),MEDIA(1,1))
      character fool_dec
      data MEDIA1/'N','A','I',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','
     *',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '/
      data fool_dec/'/'/
      data fool_intel_optimizer/.false./
      vacdst = 1e8
      DO 4901 i=1,1
        ecut(i) = 0.
        pcut(i) = 0.
        ibcmp(i) = 3
        iedgfl(i) = 1
        iphter(i) = 1
        smaxir(i) = 1e10
        i_do_rr(i) = 0
        e_max_rr(i) = 0
        med(i) = 1
        rhor(i) = 0
        iraylr(i) = 1
        iphotonucr(i) = 0
4901  CONTINUE
4902  CONTINUE
      eii_flag = 0
      eii_xfile = 'Off'
      eii_L_factor = 1.0
      xsec_out = 0
      photon_xsections = 'xcom'
      comp_xsections = 'default'
      eadl_relax = .true.
      mcdf_pe_xsections = .false.
      photonuc_xsections = 'default'
      ExIN=0
      EyIN=0
      EzIN=0
      BxIN=0
      ByIN=0
      BzIN=0
      EMLMTIN=0.02
      Bx=BxIN
      By=ByIN
      Bz=BzIN
      Bx_new=Bx
      By_new=By
      Bz_new=Bz
      emfield_on=.false.
      IF (( ExIN**2+EyIN**2+EzIN**2 + BxIN**2+ByIN**2+BzIN**2 .GT. 0 ))
     *THEN
        emfield_on=.true.
      END IF
      DO 4911 i=1,1
        iraylm(i) = 0
        DO 4921 j=1,len(iray_ff_file(i))
          iray_ff_file(i)(j:j) = ' '
4921    CONTINUE
4922    CONTINUE
        DO 4931 j=1,len(iray_ff_media(i))
          iray_ff_media(i)(j:j) = ' '
4931    CONTINUE
4932    CONTINUE
        ae(i)=0
        ap(i)=0
        ue(i)=0
        up(i)=0
        te(i)=0
        thmoll(i)=0
4911  CONTINUE
4912  CONTINUE
      DO 4941 i=1,30
        DO 4951 j=1,100
          binding_energies(i,j) = 0
4951    CONTINUE
4952    CONTINUE
4941  CONTINUE
4942  CONTINUE
      ibrdst = 1
      ibr_nist = 0
      pair_nrc = 0
      itriplet = 0
      iprdst = 1
      rhof = 1
      DO 4961 i=1,5
        iausfl(i) = 1
4961  CONTINUE
4962  CONTINUE
      DO 4971 i=6,35
        iausfl(i) = 0
4971  CONTINUE
4972  CONTINUE
      ximax = 0.5
      estepe = 0.25
      skindepth_for_bca = 3
      transport_algorithm = 0
      bca_algorithm = 0
      exact_bca = .true.
      spin_effects = .true.
      count_pII_steps = 0
      count_all_steps = 0
      radc_flag = 0
      nmed = 1
      kmpi = 12
      kmpo = 8
      dunit = 1
      rng_seed = 999999
      latchi = 0
      rmt2 = 2*rm
      rmsq = rm*rm
      pi = 4*datan(1d0)
      twopi = 2*pi
      pi5d2 = 2.5*pi
      nbr_split = 1
      i_play_RR = 0
      i_survived_RR = 0
      prob_RR = -1
      n_RR_warning = 0
      DO 4981 i=1,len(hen_house)
        hen_house(i:i) = ' '
4981  CONTINUE
4982  CONTINUE
      i = lnblnk1('/home/mainegra/production/HEN_HOUSE/')
      hen_house(:i) = '/home/mainegra/production/HEN_HOUSE/'
      IF (( '/' .NE. fool_dec )) THEN
        DO 4991 j=1,i
          IF((hen_house(j:j) .EQ. '/'))hen_house(j:j) = '/'
4991    CONTINUE
4992    CONTINUE
      END IF
      IF((hen_house(i:i) .NE. '/'))hen_house(i+1:i+1) = '/'
      n_files = 0
      DO 5001 i=1,len(egs_home)
        egs_home(i:i) = ' '
5001  CONTINUE
5002  CONTINUE
      call getenv('EGS_HOME',egs_home)
      i = lnblnk1(egs_home)
      IF (( '/' .NE. fool_dec )) THEN
        DO 5011 j=1,i
          IF((egs_home(j:j) .EQ. '/'))egs_home(j:j) = '/'
5011    CONTINUE
5012    CONTINUE
      END IF
      IF((i .GT. 0 .AND. egs_home(i:i) .NE. '/'))egs_home(i+1:i+1) = '/'
      DO 5021 i=1,len(input_file)
        input_file(i:i) = ' '
5021  CONTINUE
5022  CONTINUE
      DO 5031 i=1,len(output_file)
        output_file(i:i) = ' '
5031  CONTINUE
5032  CONTINUE
      DO 5041 i=1,len(work_dir)
        work_dir(i:i) = ' '
5041  CONTINUE
5042  CONTINUE
      DO 5051 i=1,len(pegs_file)
        pegs_file(i:i) = ' '
5051  CONTINUE
5052  CONTINUE
      DO 5061 i=1,len(host_name)
        host_name(i:i) = ' '
5061  CONTINUE
5062  CONTINUE
      n_parallel = 0
      i_parallel = 0
      n_chunk = 0
      is_batch = .false.
      first_parallel = 1
      return
      end
      subroutine egs_combine_runs(combine_routine,extension)
      implicit none
      external combine_routine
      character*(*) extension
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      character*1024 tmp_string,base,command,outfile,parfile_name,base1,
     * text_string
      integer lnblnk1,istat,ipar,egs_system,egs_open_file
      integer*4 i,k,j,numparfiles,textindex
      logical ex,iwin
      iwin=.false.
      DO 5071 i=1,len(base)
        base(i:i) = ' '
5071  CONTINUE
5072  CONTINUE
      base = egs_home(:lnblnk1(egs_home)) // user_code(:lnblnk1(user_cod
     *e)) // '/' // output_file(:lnblnk1(output_file)) // '_w'
      DO 5081 i=1,len(base1)
        base1(i:i) = ' '
5081  CONTINUE
5082  CONTINUE
      base1 = egs_home(:lnblnk1(egs_home)) // user_code(:lnblnk1(user_co
     *de)) // '/' // output_file(:lnblnk1(output_file)) // '_w*' // exte
     *nsion(:lnblnk1(extension))
      DO 5091 i=1,len(outfile)
        outfile(i:i) = ' '
5091  CONTINUE
5092  CONTINUE
      outfile = egs_home(:lnblnk1(egs_home)) // user_code(:lnblnk1(user_
     *code)) // '/' // 'parfiles_tmp'
      DO 5101 i=1,len(command)
        command(i:i) = ' '
5101  CONTINUE
5102  CONTINUE
      command = 'ls ' // base1(:lnblnk1(base1)) // ' | wc -l > ' // outf
     *ile(:lnblnk1(outfile))
      istat = egs_system(command(:lnblnk1(command)))
      write(*,*) '-> Initial istat = ', istat
      IF ((istat.NE.0)) THEN
        command = 'dir ' // base1(:lnblnk1(base1)) // ' | find "File(s)"
     * > ' // outfile(:lnblnk1(outfile))
        istat = egs_system(command(:lnblnk1(command)))
        write(*,*) '-> Windows istat = ', istat
        IF ((istat.NE.0)) THEN
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,*) ' Failed to write number of output files from p
     *arallel runs.'
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        ELSE
          iwin=.true.
        END IF
      END IF
      ipar=1
      ipar=egs_open_file(ipar,0,1,outfile(:lnblnk1(outfile)))
      IF ((iwin)) THEN
        write(*,*) '-> About to read parfiles_tmp... ', istat
        read(ipar,'(a)',err=5110,end=5110) text_string
        text_string = text_string(:lnblnk1(text_string))
        textindex = index(text_string,'File(s)')
        text_string = text_string(:textindex-1)
        read(text_string,'(i256)',err=5110) numparfiles
      ELSE
        read(ipar,'(i256)',err=5110,end=5110) numparfiles
      END IF
      close(ipar)
      DO 5121 i=1,len(command)
        command(i:i) = ' '
5121  CONTINUE
5122  CONTINUE
      IF ((iwin)) THEN
        command = 'del /Q ' // outfile(:lnblnk1(outfile))
      ELSE
        command = 'rm -f ' // outfile(:lnblnk1(outfile))
      END IF
      istat = egs_system(command(:lnblnk1(command)))
      IF ((istat.NE.0)) THEN
        write(i_log,'(/a)') '***************** Warning: '
        write(i_log,*) ' Failed to delete list of output files from para
     *llel runs.'
      END IF
      k=1
      j=1
5131  IF(j.GT.numparfiles)GO TO 5132
        DO 5141 i=1,len(tmp_string)
          tmp_string(i:i) = ' '
5141    CONTINUE
5142    CONTINUE
        tmp_string = base(:lnblnk1(base))
        call egs_itostring(tmp_string,k,.false.)
        tmp_string = tmp_string(:lnblnk1(tmp_string)) // extension(:lnbl
     *  nk1(extension))
        inquire(file=tmp_string,exist=ex)
        IF (( ex )) THEN
          call combine_routine(tmp_string)
          j=j+1
        END IF
        k=k+1
      GO TO 5131
5132  CONTINUE
      return
5110  write(*,*) '-> Failed reading parfiles_tmp... ', istat
      write(i_log,'(/a)') '***************** Error: '
      write(i_log,*) ' Failed to read number of output files from parall
     *el runs.'
      write(i_log,'(/a)') '***************** Quiting now.'
      call exit(1)
      end
      logical function egs_strip_extension(filen,fext)
      implicit none
      character*(*) filen,fext
      integer l1,l2,lnblnk1,i
      l1 = lnblnk1(filen)
      l2 = lnblnk1(fext)
      IF (( l1 .GE. l2 .AND. filen(l1-l2+1:l1) .EQ. fext(:l2) )) THEN
        egs_strip_extension = .true.
        DO 5151 i=l1-l2+1,len(filen)
          filen(i:i) = ' '
5151    CONTINUE
5152    CONTINUE
      ELSE
        egs_strip_extension = .false.
      END IF
      return
      end
      logical function egs_is_absolute_path(fn)
      implicit none
      character*(*) fn
      integer i,lnblnk1
      DO 5161 i=1,lnblnk1(fn)
        IF (( fn(i:i) .EQ. '/' )) THEN
          egs_is_absolute_path = .true.
          return
        END IF
5161  CONTINUE
5162  CONTINUE
      egs_is_absolute_path = .false.
      return
      end
      integer function egs_get_unit(iunit)
      implicit none
      integer*4 iunit, i
      logical is_open
      IF (( iunit .GT. 0 )) THEN
        inquire(iunit,opened=is_open)
        IF (( .NOT.is_open )) THEN
          egs_get_unit = iunit
          return
        END IF
      END IF
      DO 5171 i=1,99
        inquire(i,opened=is_open)
        IF (( .NOT.is_open )) THEN
          egs_get_unit = i
          return
        END IF
5171  CONTINUE
5172  CONTINUE
      egs_get_unit = -1
      return
      end
      integer function egs_open_file(iunit,rl,action,extension)
      implicit none
      integer*4 iunit, rl, action
      character*(*) extension
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      logical egs_is_absolute_path,is_open
      integer egs_get_unit
      integer i,lnblnk1
      character*1024 tmp_string,error_string
      integer*4 the_unit
      egs_open_file = -1
      the_unit = egs_get_unit(iunit)
      IF (( the_unit .LT. 0 )) THEN
        IF (( action .EQ. 0 )) THEN
          egs_open_file = -1
          return
        END IF
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) 'No free Fortran I/O units left'
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      IF (( egs_is_absolute_path(extension) )) THEN
        inquire(file=extension,opened=is_open)
        IF ((is_open)) THEN
          inquire(file=extension,number=the_unit)
          write(i_log,'(/a)') '***************** Warning: '
          write(i_log,'(a,a,/,a,i3,/,a,/,a)') 'File ',extension(:lnblnk1
     *    (extension)), ' is already opened and connected to unit ',the_
     *    unit, ' Will not try to re-open this file, assuming it has bee
     *n opened', ' by the .io file.'
        ELSE IF(( rl .EQ. 0 )) THEN
          open(the_unit,file=extension,status='unknown')
        ELSE
          open(the_unit,file=extension,status='unknown',form='unformatte
     *d', access='direct', recl=rl)
        END IF
        egs_open_file = the_unit
        return
      END IF
      DO 5181 i=1,len(tmp_string)
        tmp_string(i:i) = ' '
5181  CONTINUE
5182  CONTINUE
      tmp_string = egs_home(:lnblnk1(egs_home)) // user_code(:lnblnk1(us
     *er_code)) // '/' // work_dir(:lnblnk1(work_dir)) // output_file(:l
     *nblnk1(output_file))
      IF (( i_parallel .GT. 0 )) THEN
        tmp_string = tmp_string(:lnblnk1(tmp_string)) // '_w'
        call egs_itostring(tmp_string,i_parallel,.false.)
      END IF
      tmp_string = tmp_string(:lnblnk1(tmp_string)) // extension(:lnblnk
     *1(extension))
      inquire(file=tmp_string,opened=is_open)
      IF ((is_open)) THEN
        inquire(file=tmp_string,number=the_unit)
        write(i_log,'(/a)') '***************** Warning: '
        write(i_log,'(a,a,/,a,i3,/,a,/,a,/)') 'File ',tmp_string(:lnblnk
     *  1(tmp_string)), ' is already opened and connected to unit ',the_
     *  unit, ' Will not try to re-open this file, assuming it has been
     *opened', ' by specifying it in the .io file.'
      ELSE IF(( rl .EQ. 0 )) THEN
        open(the_unit,file=tmp_string,status='unknown',err=5190)
      ELSE
        open(the_unit,file=tmp_string,status='unknown',form='unformatted
     *', access='direct', recl=rl,err=5190)
      END IF
      egs_open_file = the_unit
      return
5190  error_string = 'In egs_open_file: failed to open file ' // tmp_str
     *ing(:lnblnk1(tmp_string)) // char(10) // 'iunit = '
      call egs_itostring(error_string,iunit,.false.)
      error_string = error_string(:lnblnk1(error_string)) // ' the_unit
     *= '
      call egs_itostring(error_string,the_unit,.false.)
      write(i_log,'(/a)') '***************** Error: '
      write(i_log,'(a)') error_string(:lnblnk1(error_string))
      write(i_log,'(/a)') '***************** Quiting now.'
      call exit(1)
      end
      integer function egs_open_datfile(iunit,rl,action,extension)
      implicit none
      integer*4 iunit,rl,action
      character*(*) extension
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      integer i,the_unit,lnblnk1,egs_get_unit
      logical egs_is_absolute_path
      character base*1024, fn*1024
      egs_open_datfile = -1
      the_unit = egs_get_unit(iunit)
      IF (( the_unit .LT. 0 )) THEN
        IF (( action .EQ. 0 )) THEN
          egs_open_datfile = -1
          return
        END IF
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) 'No free Fortran I/O units left'
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      IF (( egs_is_absolute_path(extension) )) THEN
        IF (( rl .EQ. 0 )) THEN
          open(the_unit,file=extension,status='old',err=5200)
        ELSE
          open(the_unit,file=extension,status='old',form='unformatted',
     *    access='direct',recl=rl,err=5200)
        END IF
        egs_open_datfile = the_unit
        return
5200    CONTINUE
        IF (( action .EQ. 0 )) THEN
          egs_open_datfile = -2
          return
        END IF
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) 'Failed to open file ',extension(:lnblnk1(extensi
     *  on))
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      DO 5211 i=1,len(base)
        base(i:i) = ' '
5211  CONTINUE
5212  CONTINUE
      DO 5221 i=1,len(fn)
        fn(i:i) = ' '
5221  CONTINUE
5222  CONTINUE
      base = egs_home(:lnblnk1(egs_home)) // user_code(:lnblnk1(user_cod
     *e)) // '/'
      IF (( i_parallel .GT. 0 )) THEN
        fn = base(:lnblnk1(base)) // output_file(:lnblnk1(output_file))
     *  // '_w'
        call egs_itostring(fn,i_parallel,.false.)
        fn = fn(:lnblnk1(fn)) // extension(:lnblnk1(extension))
      ELSE
        fn = base(:lnblnk1(base)) // output_file(:lnblnk1(output_file))
     *  // extension(:lnblnk1(extension))
      END IF
      IF (( rl .EQ. 0 )) THEN
        open(the_unit,file=fn,status='old',err=5230)
      ELSE
        open(the_unit,file=fn,status='old',form='unformatted',access='di
     *rect', recl=rl,err=5230)
      END IF
      egs_open_datfile = the_unit
      return
5230  CONTINUE
      write(i_log,'(/a)') '***************** Warning: '
      write(i_log,'(a,a)') 'Failed to open ',fn(:lnblnk1(fn))
      DO 5241 i=1,len(fn)
        fn(i:i) = ' '
5241  CONTINUE
5242  CONTINUE
      IF (( i_parallel .GT. 0 )) THEN
        fn = base(:lnblnk1(base)) // input_file(:lnblnk1(input_file)) //
     *   '_w'
        call egs_itostring(fn,i_parallel,.false.)
        fn = fn(:lnblnk1(fn)) // extension(:lnblnk1(extension))
      ELSE
        fn = base(:lnblnk1(base)) // input_file(:lnblnk1(input_file)) //
     *   extension(:lnblnk1(extension))
      END IF
      IF (( rl .EQ. 0 )) THEN
        open(the_unit,file=fn,status='old',err=5250)
      ELSE
        open(the_unit,file=fn,status='old',form='unformatted',access='di
     *rect', recl=rl,err=5250)
      END IF
      egs_open_datfile = the_unit
      return
5250  CONTINUE
      write(i_log,'(/a)') '***************** Error: '
      write(i_log,*) 'Failed to open data file'
      write(i_log,'(/a)') '***************** Quiting now.'
      call exit(1)
      end
      integer function egs_open_file_junk(iunit,do_it_anyway,filen)
      implicit none
      integer*4 iunit
      logical do_it_anyway
      character*(*) filen
      logical aux
      integer*4 the_unit,i
      inquire(file=filen,exist=aux)
      IF (( .NOT.aux )) THEN
        egs_open_file_junk = -2
        return
      END IF
      IF (( iunit .LT. 0 )) THEN
        the_unit = -iunit
      ELSE
        the_unit = iunit
      END IF
      IF (( the_unit .NE. 0 )) THEN
        inquire(unit=the_unit,opened=aux)
        IF (( aux )) THEN
          IF (( .NOT.do_it_anyway )) THEN
            egs_open_file_junk = -4
            return
          END IF
          IF((iunit .LT. 0))the_unit = 0
        END IF
      END IF
      IF (( the_unit .EQ. 0 )) THEN
        DO 5261 i=1,99
          inquire(unit=i,opened=aux)
          IF (( .NOT.aux )) THEN
            the_unit = i
            GO TO5262
          END IF
5261    CONTINUE
5262    CONTINUE
        IF (( the_unit .EQ. 0 )) THEN
          egs_open_file_junk = -1
          return
        END IF
      END IF
      open(the_unit,file=filen,status='old',err=5270)
      egs_open_file_junk = the_unit
      return
5270  egs_open_file_junk = -3
      return
      end
      subroutine egs_strip_path(fname)
      implicit none
      character*(*) fname
      integer i,l,l1,lnblnk1,j
      character slash
      slash = '/'
      l = lnblnk1(fname)
      DO 5281 i=1,l
        IF (( fname(i:i) .EQ. slash )) THEN
          fname(i:i) = '/'
        END IF
5281  CONTINUE
5282  CONTINUE
      DO 5291 i=l,1,-1
        IF (( fname(i:i) .EQ. '/' .OR. fname(i:i) .EQ. slash )) THEN
          l1 = l-i
          fname(:l1) = fname(i+1:l)
          DO 5301 j=l1+1,len(fname)
            fname(j:j) = ' '
5301      CONTINUE
5302      CONTINUE
          return
        END IF
5291  CONTINUE
5292  CONTINUE
      return
      end
      subroutine replace_env(fname)
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      character*(*) fname
      character*256 dirname
      integer indsep,ind1,ind2
      indsep = index(fname,'/')
      IF((indsep .LE. 0))return
      ind1=index(fname,'$')
      ind2=index(fname,'~')
      IF ((ind1.EQ.1)) THEN
        call getenv(fname(2:indsep-1),dirname)
        IF ((dirname.EQ.' ')) THEN
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,'(a,a/,a)') ' Error in file name: ',fname(:lnblnk1
     *    (fname)), ' First element in name does not specify a defined e
     *nvironment variable.'
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        END IF
        fname=dirname(:lnblnk1(dirname))//fname(indsep:)
        write(i_log,'(//a,a/)') ' Retrieving file: ',fname(:lnblnk1(fnam
     *  e))
      ELSE IF((ind2.EQ.1)) THEN
        call getenv('HOME',dirname)
        IF ((dirname.EQ.' ')) THEN
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,'(a,a/,a)') ' Error in file name: ',fname(:lnblnk1
     *    (fname)), ' HOME is undefined.'
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        END IF
        fname=dirname(:lnblnk1(dirname))//fname(indsep:)
        write(i_log,'(//a,a/)') ' Retrieving file: ',fname(:lnblnk1(fnam
     *  e))
      END IF
      return
      end
      subroutine egs_get_usercode(ucode)
      implicit none
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      character*(*) ucode
      character*512 arg
      integer l,l1,lnblnk1,i
      call getarg(0,arg)
      call egs_strip_path(arg)
      l = lnblnk1(arg)
      IF (( arg(l-3:l) .EQ. '.exe' )) THEN
        arg(l-3:l) = ' '
        l = l - 4
      END IF
      IF (( arg(l-5:l) .EQ. '_debug' )) THEN
        arg(l-5:l) = ' '
        l = l-5
      END IF
      IF (( arg(l-5:l) .EQ. '_noopt' )) THEN
        arg(l-5:l) = ' '
        l = l-5
      END IF
      l1 = len(ucode)
      IF (( l .GT. l1 )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) ' user code name is too long (',l,' chars)'
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      DO 5311 i=1,len(ucode)
        ucode(i:i) = ' '
5311  CONTINUE
5312  CONTINUE
      ucode(:l) = arg(:l)
      return
      end
      subroutine egs_itostring(string,i,leave_space)
      implicit none
      character*(*) string
      integer*4 i
      integer l,lnblnk1,idiv,itmp,iaux
      logical first,leave_space
      l = lnblnk1(string)+1
      IF((l .GT. 1 .AND. leave_space))l=l+1
      idiv = 1000000000
      itmp = i
      first = .false.
      do while(idiv.gt.0)
      iaux = itmp/idiv
      IF (( (iaux .GT. 0 .OR. first ) .AND. l .LE. len(string) )) THEN
        string(l:l) = char(iaux+48)
        first = .true.
        l = l+1
      END IF
      itmp = itmp - iaux*idiv
      idiv = idiv/10
      end do
      return
      end
      real*8 function egs_rndm()
      implicit none
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      IF((rng_seed .GT. 128))call ranmar_get
      egs_rndm = rng_array(rng_seed)*twom24
      rng_seed = rng_seed + 1
      return
      end
      integer function egs_add_medium(medname)
      implicit none
      character*(*) medname
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      integer*4 i,l,imed,medname_len
      character c
      logical same
      l = min(len(medname),24)
      medname_len = l
      DO 5321 i=1,l
        c = medname(i:i)
        IF (( ichar(c) .EQ. 0 )) THEN
          medname_len = i-1
          GO TO5322
        END IF
5321  CONTINUE
5322  CONTINUE
      DO 5331 imed=1,nmed
        l = 24
        DO 5341 i=1,24
          IF (( media(i,imed)(1:1) .EQ. ' ' )) THEN
            l = i-1
            GO TO5342
          END IF
5341    CONTINUE
5342    CONTINUE
        IF (( l .EQ. medname_len )) THEN
          same = .true.
          DO 5351 i=1,l
            c = medname(i:i)
            IF (( c .NE. media(i,imed)(1:1) )) THEN
              same = .false.
              GO TO5352
            END IF
5351      CONTINUE
5352      CONTINUE
          IF (( same )) THEN
            egs_add_medium = imed
            return
          END IF
        END IF
5331  CONTINUE
5332  CONTINUE
      nmed = nmed + 1
      IF (( nmed .GT. 1 )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,'(a,/,a,i3,a)') 'In egs_add_medium: maximum number o
     *f media exceeded ', 'Increase the macro $MXMED (currently ',1,') a
     *nd retry'
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      l = min(len(medname),24)
      DO 5361 i=1,l
        c = medname(i:i)
        IF (( ichar(c) .EQ. 0 )) THEN
          l = i-1
          GO TO5362
        END IF
        media(i,nmed) = ' '
        media(i,nmed)(1:1) = c
5361  CONTINUE
5362  CONTINUE
      IF (( l .LT. 24 )) THEN
        DO 5371 i=l+1,24
          media(i,nmed) = ' '
5371    CONTINUE
5372    CONTINUE
      END IF
      egs_add_medium = nmed
      return
      end
      subroutine egs_get_medium_name(imed,medname)
      implicit none
      character*(*) medname
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      integer*4 i,l,imed
      DO 5381 i=1,len(medname)
        medname(i:i) = ' '
5381  CONTINUE
5382  CONTINUE
      IF (( imed .LT. 1 .OR. imed .GT. nmed )) THEN
        return
      END IF
      l = 24
      DO 5391 l=24,1,-1
        IF((media(l,imed)(1:1) .NE. ' '))GO TO5392
5391  CONTINUE
5392  CONTINUE
      l = min(l,len(medname))
      DO 5401 i=1,l
        medname(i:i) = media(i,imed)(1:1)
5401  CONTINUE
5402  CONTINUE
      return
      end
      subroutine egs_get_electron_data(func,imed,which)
      implicit none
      integer*4 imed,which
      external func
      COMMON/ELECIN/ esig_e(1),psig_e(1), esige_max, psige_max, range_ep
     *(0:1,500,1), E_array(500,1), etae_ms0(500,1),etae_ms1(500,1),etap_
     *ms0(500,1),etap_ms1(500,1),q1ce_ms0(500,1),q1ce_ms1(500,1),q1cp_ms
     *0(500,1),q1cp_ms1(500,1),q2ce_ms0(500,1),q2ce_ms1(500,1),q2cp_ms0(
     *500,1),q2cp_ms1(500,1),blcce0(500,1),blcce1(500,1), EKE0(1),EKE1(1
     *), XR0(1),TEFF0(1),BLCC(1),XCC(1), ESIG0(500,1),ESIG1(500,1),PSIG0
     *(500,1),PSIG1(500,1),EDEDX0(500,1),EDEDX1(500,1),PDEDX0(500,1),PDE
     *DX1(500,1),EBR10(500,1),EBR11(500,1),PBR10(500,1),PBR11(500,1),PBR
     *20(500,1),PBR21(500,1),TMXS0(500,1),TMXS1(500,1), expeke1(1), IUNR
     *ST(1),EPSTFL(1),IAPRIM(1), sig_ismonotone(0:1,1)
      real*8 esig_e,   psig_e,   esige_max,  psige_max,  range_ep,  E_ar
     *ray,  etae_ms0,etae_ms1,  etap_ms0,etap_ms1,  q1ce_ms0,q1ce_ms1,
     *q1cp_ms0,q1cp_ms1,  q2ce_ms0,q2ce_ms1,  q2cp_ms0,q2cp_ms1,  blcce0
     *,blcce1,   expeke1,  EKE0,EKE1, XR0,  TEFF0,  BLCC,  XCC,  ESIG0,E
     *SIG1,  PSIG0,PSIG1,  EDEDX0,EDEDX1,  PDEDX0,PDEDX1,  EBR10,EBR11,
     * PBR10,PBR11,  PBR20,PBR21,  TMXS0,TMXS1
      integer*4 IUNRST,  EPSTFL,  IAPRIM
      logical sig_ismonotone
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      real*8 lemin,lemax
      lemin = (1 - eke0(imed))/eke1(imed)
      lemax = (meke(imed) - eke0(imed))/eke1(imed)
      IF (( which .EQ. 1 )) THEN
        call func(meke(imed),lemin,lemax,esig0(1,imed),esig1(1,imed))
      ELSE IF(( which .EQ. 2 )) THEN
        call func(meke(imed),lemin,lemax,psig0(1,imed),psig1(1,imed))
      ELSE IF(( which .EQ. 3 )) THEN
        call func(meke(imed),lemin,lemax,ededx0(1,imed),ededx1(1,imed))
      ELSE IF(( which .EQ. 4 )) THEN
        call func(meke(imed),lemin,lemax,pdedx0(1,imed),pdedx1(1,imed))
      ELSE IF(( which .EQ. 5 )) THEN
        call func(meke(imed),lemin,lemax,ebr10(1,imed),ebr11(1,imed))
      ELSE IF(( which .EQ. 6 )) THEN
        call func(meke(imed),lemin,lemax,pbr10(1,imed),pbr11(1,imed))
      ELSE IF(( which .EQ. 7 )) THEN
        call func(meke(imed),lemin,lemax,pbr20(1,imed),pbr21(1,imed))
      ELSE IF(( which .EQ. 8 )) THEN
        call func(meke(imed),lemin,lemax,tmxs0(1,imed),tmxs1(1,imed))
      ELSE IF(( which .EQ. 9 )) THEN
        call func(meke(imed),lemin,lemax,range_ep(0,1,imed),range_ep(1,1
     *  ,imed))
      ELSE
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) 'Unknown electron data type ',which
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      return
      end
      subroutine egs_get_photon_data(func,imed,which)
      implicit none
      integer*4 imed,which
      external func
      COMMON/PHOTIN/ EBINDA(1), GE0(1),GE1(1), GMFP0(2000,1),GMFP1(2000,
     *1),GBR10(2000,1),GBR11(2000,1),GBR20(2000,1),GBR21(2000,1), RCO0(1
     *),RCO1(1), RSCT0(100,1),RSCT1(100,1), COHE0(2000,1),COHE1(2000,1),
     *  PHOTONUC0(2000,1),PHOTONUC1(2000,1), DPMFP, MPGEM(1,1), NGR(1)
      real*8 EBINDA,  GE0,GE1,  GMFP0,GMFP1,  GBR10,GBR11,  GBR20,GBR21,
     *  RCO0,RCO1,  RSCT0,RSCT1,  COHE0,COHE1,   PHOTONUC0,PHOTONUC1,  D
     *PMFP
      integer*4
     *                  MPGEM,
     *                          NGR
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      real*8 lemin,lemax
      lemin = (1 - ge0(imed))/ge1(imed)
      lemax = (mge(imed) - ge0(imed))/ge1(imed)
      IF (( which .EQ. 1 )) THEN
        call func(mge(imed),lemin,lemax,gmfp0(1,imed),gmfp1(1,imed))
      ELSE IF(( which .EQ. 2 )) THEN
        call func(mge(imed),lemin,lemax,gbr10(1,imed),gbr11(1,imed))
      ELSE IF(( which .EQ. 3 )) THEN
        call func(mge(imed),lemin,lemax,gbr20(1,imed),gbr21(1,imed))
      ELSE IF(( which .EQ. 4 )) THEN
        call func(mge(imed),lemin,lemax,cohe0(1,imed),cohe1(1,imed))
      ELSE IF(( which .EQ. 5 )) THEN
        call func(mge(imed),lemin,lemax,PHOTONUC0(1,imed),PHOTONUC1(1,im
     *  ed))
      ELSE
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) 'Unknown photon data type ',which
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      return
      end
      subroutine egs_print_binding_energies
      implicit none
      COMMON/EDGE/binding_energies(30,100), interaction_prob(6,100), rel
     *axation_prob(39,100), edge_energies(16,100), edge_number(100), edg
     *e_a(16,100), edge_b(16,100), edge_c(16,100), edge_d(16,100), IEDGF
     *L(1),IPHTER(1)
      real*8 binding_energies,  interaction_prob,    relaxation_prob,  e
     *dge_energies,  edge_a,edge_b,edge_c,edge_d
      integer*2 IEDGFL,  IPHTER
      integer*4 edge_number
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      integer*4 i,j
      integer*4 lnblnk1
      character*3 labels(16)
      data labels/'  K',' L1',' L2',' L3', ' M1',' M2',' M3',' M4',' M5'
     *, ' N1',' N2',' N3',' N4',' N5',' N6',' N7'/
      write(i_log,'(a,a,a)') 'Binding energies from ',photon_xsections(:
     *lnblnk1(photon_xsections)), ' photon cross section library'
      DO 5411 j=1,100
        DO 5421 i=1,16
          IF (( binding_energies(i,j) .GT. 0 )) THEN
            write(i_log,'(a,i3,a,a,a,1pe12.4,a)') ' Eb(',j,',',labels(i)
     *      ,') = ',binding_energies(i,j),' MeV'
          END IF
5421    CONTINUE
5422    CONTINUE
5411  CONTINUE
5412  CONTINUE
      return
      end
      subroutine egs_scale_xcc(imed,factor)
      implicit none
      integer*4 imed
      real*8 factor
      COMMON/ELECIN/ esig_e(1),psig_e(1), esige_max, psige_max, range_ep
     *(0:1,500,1), E_array(500,1), etae_ms0(500,1),etae_ms1(500,1),etap_
     *ms0(500,1),etap_ms1(500,1),q1ce_ms0(500,1),q1ce_ms1(500,1),q1cp_ms
     *0(500,1),q1cp_ms1(500,1),q2ce_ms0(500,1),q2ce_ms1(500,1),q2cp_ms0(
     *500,1),q2cp_ms1(500,1),blcce0(500,1),blcce1(500,1), EKE0(1),EKE1(1
     *), XR0(1),TEFF0(1),BLCC(1),XCC(1), ESIG0(500,1),ESIG1(500,1),PSIG0
     *(500,1),PSIG1(500,1),EDEDX0(500,1),EDEDX1(500,1),PDEDX0(500,1),PDE
     *DX1(500,1),EBR10(500,1),EBR11(500,1),PBR10(500,1),PBR11(500,1),PBR
     *20(500,1),PBR21(500,1),TMXS0(500,1),TMXS1(500,1), expeke1(1), IUNR
     *ST(1),EPSTFL(1),IAPRIM(1), sig_ismonotone(0:1,1)
      real*8 esig_e,   psig_e,   esige_max,  psige_max,  range_ep,  E_ar
     *ray,  etae_ms0,etae_ms1,  etap_ms0,etap_ms1,  q1ce_ms0,q1ce_ms1,
     *q1cp_ms0,q1cp_ms1,  q2ce_ms0,q2ce_ms1,  q2cp_ms0,q2cp_ms1,  blcce0
     *,blcce1,   expeke1,  EKE0,EKE1, XR0,  TEFF0,  BLCC,  XCC,  ESIG0,E
     *SIG1,  PSIG0,PSIG1,  EDEDX0,EDEDX1,  PDEDX0,PDEDX1,  EBR10,EBR11,
     * PBR10,PBR11,  PBR20,PBR21,  TMXS0,TMXS1
      integer*4 IUNRST,  EPSTFL,  IAPRIM
      logical sig_ismonotone
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      IF (( imed .GT. 0 .AND. imed .LE. nmed )) THEN
        xcc(imed) = xcc(imed)*factor
      END IF
      return
      end
      subroutine egs_write_string(ounit,string)
      implicit none
      integer*4 ounit
      character*(*) string
      write(ounit,'(a,$)') string
      call flush(ounit)
      return
      end
      subroutine egs_swap_2(c)
      character c(2),tmp
      tmp=c(2)
      c(2)=c(1)
      c(1)=tmp
      return
      end
      subroutine egs_swap_4(c)
      character c(4),tmp
      tmp=c(4)
      c(4)=c(1)
      c(1)=tmp
      tmp=c(3)
      c(3)=c(2)
      c(2)=tmp
      return
      end
      subroutine set_spline(x,f,a,b,c,d,n)
      implicit none
      integer*4 n
      real*8 x(n),f(n),a(n),b(n),c(n),d(n)
      integer*4 m1,m2,m,mr
      real*8 s,r
      m1 = 2
      m2 = n-1
      s = 0
      DO 5431 m=1,m2
        d(m) = x(m+1) - x(m)
        r = (f(m+1) - f(m))/d(m)
        c(m) = r - s
        s = r
5431  CONTINUE
5432  CONTINUE
      s=0
      r=0
      c(1)=0
      c(n)=0
      DO 5441 m=m1,m2
        c(m) = c(m) + r*c(m-1)
        b(m) = 2*(x(m-1) - x(m+1)) - r*s
        s = d(m)
        r = s/b(m)
5441  CONTINUE
5442  CONTINUE
      mr = m2
      DO 5451 m=m1,m2
        c(mr) = (d(mr)*c(mr+1) - c(mr))/b(mr)
        mr = mr - 1
5451  CONTINUE
5452  CONTINUE
      DO 5461 m=1,m2
        s = d(m)
        r = c(m+1) - c(m)
        d(m) = r/s
        c(m) = 3*c(m)
        b(m) = (f(m+1)-f(m))/s - (c(m)+r)*s
        a(m) = f(m)
5461  CONTINUE
5462  CONTINUE
      return
      end
      real*8 function spline(s,x,a,b,c,d,n)
      implicit none
      integer*4 n
      real*8 s,x(n),a(n),b(n),c(n),d(n)
      integer m_lower,m_upper,direction,m,ml,mu,mav
      real*8 q
      IF (( x(1) .GT. x(n) )) THEN
        direction = 1
        m_lower = n
        m_upper = 0
      ELSE
        direction = 0
        m_lower = 0
        m_upper = n
      END IF
      IF (( s .GE. x(m_upper + direction) )) THEN
        m = m_upper + 2*direction - 1
      ELSE IF(( s .LE. x(m_lower+1-direction) )) THEN
        m = m_lower - 2*direction + 1
      ELSE
        ml = m_lower
        mu = m_upper
5471    IF(iabs(mu-ml).LE.1)GO TO 5472
          mav = (ml+mu)/2
          IF (( s .LT. x(mav) )) THEN
            mu = mav
          ELSE
            ml = mav
          END IF
        GO TO 5471
5472    CONTINUE
        m = mu + direction - 1
      END IF
      q = s - x(m)
      spline = a(m) + q*(b(m) + q*(c(m) + q*d(m)))
      return
      end
      subroutine prepare_alias_table(nsbin,xs_array,fs_array,ws_array,ib
     *in_array)
      implicit none
      integer nsbin
      integer*4 ibin_array(nsbin)
      real*8 xs_array(0:nsbin),fs_array(0:nsbin),ws_array(nsbin)
      integer*4 i,j_l,j_h
      real*8 sum,aux
      sum = 0
      DO 5481 i=1,nsbin
        aux = 0.5*(fs_array(i)+fs_array(i-1))*(xs_array(i)-xs_array(i-1)
     *  )
        IF((aux .LT. 1e-30))aux = 1e-30
        ws_array(i) = -aux
        ibin_array(i) = 1
        sum = sum + aux
5481  CONTINUE
5482  CONTINUE
      sum = sum/nsbin
      DO 5491 i=1,nsbin-1
        DO 5501 j_h=1,nsbin
          IF (( ws_array(j_h) .LT. 0 )) THEN
            IF((abs(ws_array(j_h)) .GT. sum))GOTO 5510
          END IF
5501    CONTINUE
5502    CONTINUE
        j_h = nsbin
5510    CONTINUE
          DO 5511 j_l=1,nsbin
          IF (( ws_array(j_l) .LT. 0 )) THEN
            IF((abs(ws_array(j_l)) .LT. sum))GOTO 5520
          END IF
5511    CONTINUE
5512    CONTINUE
        j_l = nsbin
5520    aux = sum - abs(ws_array(j_l))
        ws_array(j_h) = ws_array(j_h) + aux
        ws_array(j_l) = -ws_array(j_l)/sum
        ibin_array(j_l) = j_h
        IF((i .EQ. nsbin-1))ws_array(j_h) = 1
5491  CONTINUE
5492  CONTINUE
      return
      end
      real*8 function alias_sample1(nsbin,xs_array,fs_array,ws_array,ibi
     *n_array)
      implicit none
      integer nsbin
      integer*4 ibin_array(nsbin)
      real*8 xs_array(0:nsbin),fs_array(0:nsbin),ws_array(nsbin)
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      integer*4 j
      real*8 r1,r2,aj,x,dx,a,rnno1
      IF((rng_seed .GT. 128))call ranmar_get
      r1 = rng_array(rng_seed)*twom24
      rng_seed = rng_seed + 1
      IF((rng_seed .GT. 128))call ranmar_get
      r2 = rng_array(rng_seed)*twom24
      rng_seed = rng_seed + 1
      aj = 1 + r1*nsbin
      j = aj
      aj = aj - j
      IF((aj .GT. ws_array(j)))j = ibin_array(j)
      x = xs_array(j-1)
      dx = xs_array(j)-x
      IF (( fs_array(j-1) .GT. 0 )) THEN
        a = fs_array(j)/fs_array(j-1)-1
        IF (( abs(a) .LT. 0.2 )) THEN
          rnno1 = 0.5*(1-r2)*a
          alias_sample1 = x + r2*dx*(1+rnno1*(1-r2*a))
        ELSE
          alias_sample1 = x - dx/a*(1-sqrt(1+r2*a*(2+a)))
        END IF
      ELSE
        alias_sample1 = x + dx*sqrt(r2)
      END IF
      return
      end
      subroutine prepare_alias_histogram(nsbin,ws_array,ibin_array)
      implicit none
      integer*4 nsbin,ibin_array(nsbin)
      real*8 ws_array(nsbin)
      integer*4 i,j_l,j_h
      real*8 sum,aux
      sum = 0
      DO 5531 i=1,nsbin
        sum = sum + ws_array(i)
        ibin_array(i) = -1
5531  CONTINUE
5532  CONTINUE
      sum = sum/nsbin
      DO 5541 i=1,nsbin-1
        DO 5551 j_h=1,nsbin
          IF((ibin_array(j_h) .LT. 0 .AND. ws_array(j_h) .GT. sum))GO TO
     *    5552
5551    CONTINUE
5552    CONTINUE
        DO 5561 j_l=1,nsbin
          IF((ibin_array(j_l) .LT. 0 .AND. ws_array(j_l) .LT. sum))GO TO
     *    5562
5561    CONTINUE
5562    CONTINUE
        aux = sum - ws_array(j_l)
        ws_array(j_h) = ws_array(j_h) - aux
        ws_array(j_l) = ws_array(j_l)/sum
        ibin_array(j_l) = j_h
5541  CONTINUE
5542  CONTINUE
      DO 5571 i=1,nsbin
        IF (( ibin_array(i) .LT. 0 )) THEN
          ibin_array(i) = i
          ws_array(i) = 1
        END IF
5571  CONTINUE
5572  CONTINUE
      return
      end
      integer*4 function sample_alias_histogram(nsbin,ws_array,ibin_arra
     *y)
      implicit none
      integer*4 nsbin,ibin_array(*)
      real*8 ws_array(*)
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      real*8 r1,r2
      integer*4 ibin
      IF((rng_seed .GT. 128))call ranmar_get
      r1 = rng_array(rng_seed)*twom24
      rng_seed = rng_seed + 1
      IF((rng_seed .GT. 128))call ranmar_get
      r2 = rng_array(rng_seed)*twom24
      rng_seed = rng_seed + 1
      ibin = 1 + nsbin*r1
      IF((r2 .GT. ws_array(ibin)))ibin = ibin_array(ibin)
      sample_alias_histogram = ibin
      return
      end
      subroutine gauss_legendre(x1,x2,x,w,n)
      implicit none
      integer*4 n
      real*8 x1,x2,x(n),w(n)
      real*8 eps,Pi
      parameter (eps = 3.D-14,Pi=3.141592654D0)
      integer*4 i,m,j
      real*8 xm,xl,z,z1,p1,p2,p3,pp
      m = (n + 1)/2
      xm=0.5d0*(x2+x1)
      xl=0.5d0*(x2-x1)
      DO 5581 i=1,m
        z=cos(Pi*(i-.25d0)/(n+.5d0))
5591    CONTINUE
          p1=1.d0
          p2=0.d0
          DO 5601 j=1,n
            p3 = p2
            p2 = p1
            p1=((2.d0*j-1.d0)*z*p2-(j-1.d0)*p3)/j
5601      CONTINUE
5602      CONTINUE
          pp=n*(z*p1-p2)/(z*z-1.d0)
          z1=z
          z=z1-p1/pp
          IF(((abs(z-z1) .LT. eps)))GO TO5592
        GO TO 5591
5592    CONTINUE
        x(i)=xm-xl*z
        x(n+1-i)=xm+xl*z
        w(i)=2.d0*xl/((1.d0-z*z)*pp*pp)
        w(n+1-i)=w(i)
5581  CONTINUE
5582  CONTINUE
      return
      end
      integer function lnblnk1(string)
      character*(*) string
      integer i
      DO 5611 i=len(string),1,-1
        j = ichar(string(i:i))
        IF (( j .EQ. 0 )) THEN
          lnblnk1 = i-1
          return
        END IF
        IF (( j .NE. 9 .AND. j .NE. 10 .AND. j .NE. 11 .AND. j .NE. 12 .
     *  AND. j .NE. 13 .AND. j .NE. 32 )) THEN
          lnblnk1 = i
          return
        END IF
5611  CONTINUE
5612  CONTINUE
      lnblnk1 = 0
      return
      end
      real*8 FUNCTION ERF1(X)
      implicit none
      real*8 x
      double precision A(0:22,2)
      double precision CONST,  BN,BN1,BN2,  Y,FAC
      integer*4 N,  K,  NLIM(2)
      DATA A/ 1.0954712997776232 , -0.2891754011269890 , 0.1104563986337
     *951 , -0.0412531882278565 , 0.0140828380706516 , -0.00432929544743
     *14 , 0.0011982719015923 , -0.0002999729623532 , 0.0000683258603789
     * , -0.0000142469884549 , 0.0000027354087728 , -0.0000004861912872
     *, 0.0000000803872762 , -0.0000000124184183 , 0.0000000017995326 ,
     *-0.0000000002454795 , 0.0000000000316251 , -0.0000000000038590 , 0
     *.0000000000004472 , -0.0000000000000493 , 0.0000000000000052 , -0.
     *0000000000000005 , 0.0000000000000001 , 0.9750834237085559 , -0.02
     *40493938504146 , 0.0008204522408804 , -0.0000434293081303 , 0.0000
     *030184470340 , -0.0000002544733193 , 0.0000000248583530 , -0.00000
     *00027317201 , 0.0000000003308472 , 0.0000000000001464 , -0.0000000
     *000000244 , 0.0000000000000042 , -0.0000000000000008 , 0.000000000
     *0000001 , 9*0.0 /
      DATA NLIM/ 22,16 /
      DATA CONST/ 1.128379167095513 /
      IF (( x .GT. 3 )) THEN
        y = 3/x
        k = 2
      ELSE
        y = x/3
        k = 1
      END IF
      FAC = 2.0 * ( 2.0 * Y*Y - 1.0 )
      BN1 = 0.0
      BN = 0.0
      DO 5621 n=NLIM(K),0,-1
        BN2 = BN1
        BN1 = BN
        BN = FAC * BN1 - BN2 + A(N,K)
5621  CONTINUE
5622  CONTINUE
      IF (( k .EQ. 1 )) THEN
        erf1 = CONST * Y * ( BN - BN1 )
      ELSE
        erf1 = 1 - CONST * EXP(-X**2) * ( BN - BN2 + A(0,K) )/(4.0 * X)
      END IF
      RETURN
      end
      real*8 FUNCTION ZERO()
      implicit none
      integer*4 i
      real*8 x, xtemp
      x = 1.E-20
      DO 5631 i=1,100
        IF ((x .EQ. 0.0)) THEN
          GO TO5632
        ELSE
          xtemp = x
        END IF
        x = x/1.E5
5631  CONTINUE
5632  CONTINUE
      x = xtemp
      DO 5641 i=1,5
        IF ((x .NE. 0.0)) THEN
          xtemp = x
        ELSE
          GO TO5642
        END IF
        x = x/10
5641  CONTINUE
5642  CONTINUE
      x = xtemp
      DO 5651 i=2,10
        IF ((x .NE. 0.0)) THEN
          xtemp = x
        ELSE
          GO TO5652
        END IF
        x = x/i
5651  CONTINUE
5652  CONTINUE
      zero = xtemp
      return
      end
      character*512 function toUpper(a_string)
      character*(*) a_string
      character*512 the_string
      integer*4 cursor, i, lnblnk1
      toUpper = a_string
      the_string = a_string
      DO 5661 i=1,lnblnk1(the_string)
        cursor=ICHAR(the_string(i:i))
        IF (((cursor.GE.97).AND.(cursor.LE.122))) THEN
          cursor=cursor-32
          toUpper(i:i)=CHAR(cursor)
        END IF
5661  CONTINUE
5662  CONTINUE
      return
      end
      integer*1 function egs_read_byte(iunit, jrec)
      implicit none
      integer iunit, jrec, i, j, ierr
      integer*1 i_1
      character c_1
      equivalence (i_1,c_1)
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      read(iunit,rec=jrec,IOSTAT=ierr) c_1
      IF ((ierr.ne.0)) THEN
        write(i_log,'(/a)') '***************** Warning: '
        write(i_log,*) ' *** egs_read_byte: ERROR READING A byte *** '
        write(i_log,*) ' From unit ',iunit,' position ',jrec,' bytes'
        egs_read_byte = -1
        return
      END IF
      jrec = jrec + 1
      egs_read_byte = i_1
      return
      end
      integer*2 function egs_read_short(iunit, jrec)
      implicit none
      integer iunit, jrec, i, j, ierr
      integer*2 i_2
      character c_2(2)
      equivalence (i_2,c_2)
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      j = 0
      DO 5671 i=jrec,jrec+1
        j = j + 1
        read(iunit,rec=i,IOSTAT=ierr) c_2(j)
        IF ((ierr.ne.0)) THEN
          write(i_log,'(/a)') '***************** Warning: '
          write(i_log,*) ' *** egs_read_short: ERROR READING short integ
     *er *** '
          write(i_log,*) ' From unit ',iunit,' position ',jrec,' bytes'
          egs_read_short = -1
          return
        END IF
5671  CONTINUE
5672  CONTINUE
      jrec = jrec + 2
      egs_read_short = i_2
      return
      end
      integer*4 function egs_read_int(iunit, jrec)
      implicit none
      integer iunit, jrec, i, j, ierr
      integer*4 i_4
      character c_4(4)
      equivalence (i_4,c_4)
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      j = 0
      DO 5681 i=jrec,jrec+3
        j = j + 1
        read(iunit,rec=i,IOSTAT=ierr) c_4(j)
        IF ((ierr.ne.0)) THEN
          write(i_log,'(/a)') '***************** Warning: '
          write(i_log,*) ' *** egs_read_int: ERROR READING integer *** '
          write(i_log,*) ' From unit ',iunit,' position ',jrec,' bytes'
          egs_read_int = -1
          return
        END IF
5681  CONTINUE
5682  CONTINUE
      jrec = jrec + 4
      egs_read_int = i_4
      return
      end
      real*4 function egs_read_real(iunit, jrec)
      implicit none
      integer iunit, jrec, i, j, ierr
      real*4 r_4
      character c_4(4)
      equivalence (r_4,c_4)
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      j = 0
      DO 5691 i=jrec,jrec+3
        j = j + 1
        read(iunit,rec=i,IOSTAT=ierr) c_4(j)
        IF ((ierr.ne.0)) THEN
          write(i_log,'(/a)') '***************** Warning: '
          write(i_log,*) ' *** egs_read_real: ERROR READING float *** '
          write(i_log,*) ' From unit ',iunit,' position ',jrec,' bytes'
          egs_read_real = -1
          return
        END IF
5691  CONTINUE
5692  CONTINUE
      jrec = jrec + 4
      egs_read_real = r_4
      return
      end
      integer*4 function ibsearch(a, nsh, b)
      implicit none
      real*8 a, b(*)
      integer*4 min,max,help,nsh
      real*8 x
      min = 1
      max = nsh
      x = a
5701  IF(min.GE.max-1)GO TO 5702
        help = (max+min)/2
        IF (( b(help).le.x)) THEN
          min = help
        ELSE
          max = help
        END IF
      GO TO 5701
5702  CONTINUE
      ibsearch = min
      return
      end
      SUBROUTINE EFUNS(E,V)
      implicit none
      real*4 E,V(8)
      real*4 BREM,AMOLL,BHAB,ANNIH,ESIG,PSIG
      real*4 BREMTM,AMOLTM,BHABTM,ANIHTM,SPTOTE,SPTOTP,TMXS,THBREM
      COMMON/THRESHP/APP,AEP,UPP,UEP,THBREMP,THMOLLP,TEP,IUNRSTP
      real*4 APP,AEP,UPP,UEP,THBREMP,THMOLLP,TEP
      integer*4 IUNRSTP
      COMMON/DERCON/RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      real*4 RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      THBREM=RMP+APP
      IF ((IUNRSTP.EQ.0 .OR. IUNRSTP.EQ.1 .OR. IUNRSTP.EQ.5)) THEN
        BREM=BREMTM(E)
        AMOLL=AMOLTM(E)
        BHAB=BHABTM(E)
        ANNIH=ANIHTM(E)
        ESIG=BREM+AMOLL
        V(1)=ESIG
        PSIG=BREM+BHAB+ANNIH
        V(2)=PSIG
        V(3)=SPTOTE(E,AEP,APP)
        V(4)=SPTOTP(E,AEP,APP)
        IF ((ESIG.GT.0.0)) THEN
          V(5)=BREM/ESIG
        ELSE
          IF ((THBREM.LE.THMOLLP)) THEN
            V(5)=1.0
          ELSE
            V(5)=0.0
          END IF
        END IF
        V(6)=BREM/PSIG
        V(7)=(BREM+BHAB)/PSIG
        V(8)=TMXS(E)
      ELSE IF((IUNRSTP.EQ.2)) THEN
        V(1)=0.0
        V(2)=0.0
        V(5)=0.0
        V(6)=0.0
        V(7)=0.0
        V(3) = SPTOTE(E,E,E)
        V(4) = SPTOTP(E,E,E)
        V(8) = TMXS(E)
      ELSE IF((IUNRSTP.EQ.3)) THEN
        BREM=BREMTM(E)
        ANNIH=ANIHTM(E)
        V(1)=BREM
        V(2)=BREM + ANNIH
        V(3)=SPTOTE(E,E,APP)
        V(4)=SPTOTP(E,E,APP)
        V(5)=1.0
        V(6)=BREM/V(2)
        V(7)=V(6)
        V(8)=TMXS(E)
      ELSE IF((IUNRSTP.EQ.4)) THEN
        V(1)=AMOLTM(E)
        V(2)=BHABTM(E)
        V(3)=SPTOTE(E,AEP,E)
        V(4)=SPTOTP(E,AEP,E)
        V(5)=0.0
        V(6)=0.0
        V(7)=1.0
        V(8)=TMXS(E)
      ELSE
        WRITE(6,5710)IUNRSTP
5710    FORMAT(//'*********IUNRST=',I4,' NOT ALLOWED BY EFUNS*****'/ ' I
     *UNRST=6 OR 7 ONLY ALLOWED WITH CALL OR PLTN OPTIONS'//)
        call exit(20)
      END IF
      RETURN
      END
      real*4 FUNCTION BREMTM(E0)
      implicit none
      real*4 E0,BREMRM
      COMMON/THRESHP/APP,AEP,UPP,UEP,THBREMP,THMOLLP,TEP,IUNRSTP
      real*4 APP,AEP,UPP,UEP,THBREMP,THMOLLP,TEP
      integer*4 IUNRSTP
      COMMON/DERCON/RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      real*4 RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      IF ((E0.LE.APP+RMP)) THEN
        BREMTM=0.
      ELSE
        BREMTM=BREMRM(E0,APP,E0-RMP)
      END IF
      RETURN
      END
      real*4 FUNCTION BREMRM(E,K1,K2)
      implicit none
      real*4 E,K1,K2
      integer*4 I
      real*4 BREMRZ
      COMMON/MIXDAT/NEP,LMED,PZP(50),ZELEMP(50),WAP(50),RHOZP(50), GASPP
     *,EZ,TPZ,IDSTRN(24)
      integer*4 NEP,LMED
      real*4 PZP,ZELEMP,WAP,RHOZP,GASPP,EZ,TPZ
      CHARACTER*4 IDSTRN
      BREMRM=0.
      DO 5721 I=1,NEP
        BREMRM=BREMRM+PZP(I)*BREMRZ(ZELEMP(I),E,K1,K2)
5721  CONTINUE
5722  CONTINUE
      RETURN
      END
      real*4 FUNCTION BREMRZ(Z,E,K1,K2)
      implicit none
      real*4 Z,E,K1,K2
      EXTERNAL BREMFZ
      real*4 DUMMY,BREMDZ,QD,BREMFZ
      DUMMY=BREMDZ(Z,E,K1)
      BREMRZ=QD(BREMFZ,K1,K2,'BREMFZ')
      RETURN
      END
      real*4 FUNCTION BREMDZ(Z,E,K)
      implicit none
      real*4 Z,E,K,BRMSDZ
      BREMDZ=BRMSDZ(Z,E,K)/K
      RETURN
      END
      real*4 FUNCTION BREMFZ(K)
      implicit none
      real*4 K,BRMSFZ
      BREMFZ=BRMSFZ(K)/K
      RETURN
      END
      real*4 FUNCTION BRMSFZ(K)
      implicit none
      real*4 K
      real*4 EMKLOC,DELTA,SB1,SB2,EE
      COMMON/LBREMZ/CONST,DELC,EBREMZ,DELTAM,XLNZ
      real*4 CONST,DELC,EBREMZ,DELTAM,XLNZ
      EMKLOC=EBREMZ-K
      IF ((EMKLOC.EQ.0.0)) THEN
        EMKLOC=1.E-25
      END IF
      DELTA=DELC*K/EMKLOC
      IF ((DELTA.GE.DELTAM)) THEN
        BRMSFZ=0.0
      ELSE
        IF ((DELTA.LE.1.)) THEN
          SB1=20.867+DELTA*(-3.242+DELTA*0.625)-XLNZ
          SB2=20.209+DELTA*(-1.930+DELTA*(-0.086))-XLNZ
        ELSE
          SB1=21.12-4.184*LOG(DELTA+0.952)-XLNZ
          SB2=SB1
        END IF
        EE=EMKLOC/EBREMZ
        BRMSFZ=CONST*((1.+EE*EE)*SB1-0.666667*EE*SB2)
      END IF
      RETURN
      END
      real*4 FUNCTION AMOLTM(E0)
      implicit none
      real*4 E0
      real*4 T0,AMOLRM
      COMMON/THRESHP/APP,AEP,UPP,UEP,THBREMP,THMOLLP,TEP,IUNRSTP
      real*4 APP,AEP,UPP,UEP,THBREMP,THMOLLP,TEP
      integer*4 IUNRSTP
      COMMON/DERCON/RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      real*4 RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      IF ((E0.LE.THMOLLP)) THEN
        AMOLTM=0.
      ELSE
        T0=E0-RMP
        AMOLTM=AMOLRM(E0,AEP,T0*0.5+RMP)
      END IF
      RETURN
      END
      real*4 FUNCTION AMOLRM(EN0,EN1,EN2)
      implicit none
      real*4 EN0,EN1,EN2
      real*4 T0,T1,T2,TM,EM,C1,C2,BETASQ,CMOLL2,EPS1,EPSP1,EPS2,EPSP2
      COMMON/PMCONS/PIP,C,RME,HBAR,ECGS,EMKS,AN
      real*4 PIP,C,RME,HBAR,ECGS,EMKS,AN
      COMMON/MOLVAR/WM,ZC,ZT,ZB,ZF,ZS,ZE,ZX,ZA,ZG,ZP,ZV,ZU, RLCP,EDEN,RH
     *OP,XCCP,BLCCP,TEFF0P,XR0P
      real*4 WM,ZC,ZT,ZB,ZF,ZS,ZE,ZX,ZA,ZG,ZP,ZV,ZU,RLCP,EDEN,RHOP, XCCP
     *,BLCCP,TEFF0P,XR0P
      COMMON/DERCON/RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      real*4 RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      T0=EN0-RMP
      T1=EN1-RMP
      T2=EN2-RMP
      TM=T0/RMP
      EM=TM+1.
      C1=(TM/EM)**2
      C2=(2.*TM+1.)/EM**2
      BETASQ=1.-1./EM**2
      CMOLL2=RLCP*EDEN*2.*PIP*R0**2/(BETASQ*TM)
      EPS1=T1/T0
      EPSP1=1.-EPS1
      EPS2=T2/T0
      EPSP2=1.-EPS2
      AMOLRM=CMOLL2*(C1*(EPS2-EPS1)+1./EPS1-1./EPS2+1./EPSP2-1./EPSP1 -C
     *2*LOG(EPS2*EPSP1/(EPS1*EPSP2)))
      RETURN
      END
      real*4 FUNCTION BHABTM(E0)
      implicit none
      real*4 E0,BHABRM
      COMMON/THRESHP/APP,AEP,UPP,UEP,THBREMP,THMOLLP,TEP,IUNRSTP
      real*4 APP,AEP,UPP,UEP,THBREMP,THMOLLP,TEP
      integer*4 IUNRSTP
      IF ((E0.LE.AEP)) THEN
        BHABTM=0.
      ELSE
        BHABTM=BHABRM(E0,AEP,E0)
      END IF
      RETURN
      END
      real*4 FUNCTION BHABRM(EN0,EN1,EN2)
      implicit none
      real*4 EN0,EN1,EN2
      real*4 T0,T1,T2,TM,EM,Y,BETASI,CBHAB2,B1,B2,B3,B4,EPS1,EPS2
      COMMON/PMCONS/PIP,C,RME,HBAR,ECGS,EMKS,AN
      real*4 PIP,C,RME,HBAR,ECGS,EMKS,AN
      COMMON/MOLVAR/WM,ZC,ZT,ZB,ZF,ZS,ZE,ZX,ZA,ZG,ZP,ZV,ZU, RLCP,EDEN,RH
     *OP,XCCP,BLCCP,TEFF0P,XR0P
      real*4 WM,ZC,ZT,ZB,ZF,ZS,ZE,ZX,ZA,ZG,ZP,ZV,ZU,RLCP,EDEN,RHOP, XCCP
     *,BLCCP,TEFF0P,XR0P
      COMMON/DERCON/RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      real*4 RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      T0=EN0-RMP
      T1=EN1-RMP
      T2=EN2-RMP
      TM=T0/RMP
      EM=TM+1.
      Y=1./(TM+2.)
      BETASI=1./(1.-1./EM**2)
      CBHAB2=RLCP*EDEN*2.*PIP*R0**2/TM
      B1=2.-Y**2
      B2=3.-Y*(6.-Y*(1.-Y*2.))
      B3=2.-Y*(10.-Y*(16.-Y*8.))
      B4=1.-Y*(6.-Y*(12.-Y*8.))
      EPS1=T1/T0
      EPS2=T2/T0
      BHABRM=CBHAB2*(BETASI*(1./EPS1-1./EPS2)-B1*LOG(EPS2/EPS1) +B2*(EPS
     *2-EPS1)+EPS2*EPS2*(EPS2*B4/3.-0.5*B3) - EPS1*EPS1*(EPS1*B4/3.-0.5*
     *B3))
      RETURN
      END
      real*4 FUNCTION ANIHTM(E0)
      implicit none
      real*4 E0
      real*4 GAM,P0P2,P0P,CANIH
      COMMON/PMCONS/PIP,C,RME,HBAR,ECGS,EMKS,AN
      real*4 PIP,C,RME,HBAR,ECGS,EMKS,AN
      COMMON/MOLVAR/WM,ZC,ZT,ZB,ZF,ZS,ZE,ZX,ZA,ZG,ZP,ZV,ZU, RLCP,EDEN,RH
     *OP,XCCP,BLCCP,TEFF0P,XR0P
      real*4 WM,ZC,ZT,ZB,ZF,ZS,ZE,ZX,ZA,ZG,ZP,ZV,ZU,RLCP,EDEN,RHOP, XCCP
     *,BLCCP,TEFF0P,XR0P
      COMMON/DERCON/RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      real*4 RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      GAM=E0/RMP
      P0P2=GAM*GAM-1.0
      P0P=SQRT(P0P2)
      CANIH=RLCP*EDEN*PIP*R0**2/(GAM+1.)
      ANIHTM=CANIH*((GAM*GAM+4.*GAM+1.)/P0P2*LOG(GAM+P0P) -(GAM+3.)/P0P)
      RETURN
      END
      real*4 FUNCTION SPTOTP(E0,EE,EG)
      implicit none
      real*4 E0,EE,EG
      real*4 SPIONP,BRMSTM
      COMMON/THRESHP/APP,AEP,UPP,UEP,THBREMP,THMOLLP,TEP,IUNRSTP
      real*4 APP,AEP,UPP,UEP,THBREMP,THMOLLP,TEP
      integer*4 IUNRSTP
      IF ((IUNRSTP.EQ.0)) THEN
        SPTOTP=SPIONP(E0,EE)+BRMSTM(E0,EG)
      ELSE IF((IUNRSTP.EQ.1)) THEN
        SPTOTP=SPIONP(E0,E0)
      ELSE IF((IUNRSTP.EQ.2)) THEN
        SPTOTP=SPIONP(E0,E0)+BRMSTM(E0,E0)
      ELSE IF((IUNRSTP.EQ.3)) THEN
        SPTOTP=SPIONP(E0,E0)+BRMSTM(E0,EG)
      ELSE IF((IUNRSTP.EQ.4)) THEN
        SPTOTP=SPIONP(E0,EE)+BRMSTM(E0,E0)
      ELSE IF((IUNRSTP.EQ.5)) THEN
        SPTOTP=BRMSTM(E0,E0)
      ELSE IF((IUNRSTP.EQ.6)) THEN
        SPTOTP=BRMSTM(E0,EG)
      ELSE IF((IUNRSTP.EQ.7)) THEN
        SPTOTP=SPIONP(E0,EE)
      END IF
      RETURN
      END
      real*4 FUNCTION SPTOTE(E0,EE,EG)
      implicit none
      real*4 E0,EE,EG
      real*4 SPIONE,BRMSTM
      COMMON/THRESHP/APP,AEP,UPP,UEP,THBREMP,THMOLLP,TEP,IUNRSTP
      real*4 APP,AEP,UPP,UEP,THBREMP,THMOLLP,TEP
      integer*4 IUNRSTP
      IF ((IUNRSTP.EQ.0)) THEN
        SPTOTE=SPIONE(E0,EE)+BRMSTM(E0,EG)
      ELSE IF((IUNRSTP.EQ.1)) THEN
        SPTOTE=SPIONE(E0,E0)
      ELSE IF((IUNRSTP.EQ.2)) THEN
        SPTOTE=SPIONE(E0,E0)+BRMSTM(E0,E0)
      ELSE IF((IUNRSTP.EQ.3)) THEN
        SPTOTE=SPIONE(E0,E0)+BRMSTM(E0,EG)
      ELSE IF((IUNRSTP.EQ.4)) THEN
        SPTOTE=SPIONE(E0,EE)+BRMSTM(E0,E0)
      ELSE IF((IUNRSTP.EQ.5)) THEN
        SPTOTE=BRMSTM(E0,E0)
      ELSE IF((IUNRSTP.EQ.6)) THEN
        SPTOTE=BRMSTM(E0,EG)
      ELSE IF((IUNRSTP.EQ.7)) THEN
        SPTOTE=SPIONE(E0,EE)
      END IF
      RETURN
      END
      real*4 FUNCTION SPIONE(E0,EE)
      implicit none
      real*4 E0,EE,SPIONB
      SPIONE=SPIONB(E0,EE,.FALSE.)
      RETURN
      END
      real*4 FUNCTION SPIONB(E0,EE,POSITR)
      implicit none
      real*4 E0,EE
      LOGICAL POSITR
      real*4 G,EEM,T,ETA2,BETA2,ALETA2,X,D,FTERM,TP2,D2,D3,D4,DELTA
      integer*4 I
      COMMON/DERCON/RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      real*4 RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      COMMON/LSPION/CBAR,X0,X1,SK,TOLN10,AFACT,SPC1,SPC2,IEV
      real*4 CBAR,X0,X1,SK,TOLN10,AFACT,SPC1,SPC2,IEV
      COMMON/EPSTAR/EPSTEN(150),EPSTD(150),WEPST(20), EPSTTL,NEPST,IEPST
     *,EPSTFLP, NELEPS,ZEPST(20),IAPRFL,IAPRIMP
      integer*4 ZEPST,NELEPS,IAPRFL,NEPST,IEPST,EPSTFLP,IAPRIMP
      CHARACTER EPSTTL*80
      real*4 EPSTEN,EPSTD,WEPST
      G=E0/RMP
      EEM=EE/RMP-1.
      T=G-1
      ETA2=T*(G+1.)
      BETA2=ETA2/G**2
      ALETA2=LOG(ETA2)
      X=0.21715*ALETA2
      IF ((.NOT.POSITR)) THEN
        D=AMIN1(EEM,0.5*T)
        FTERM=-1.-BETA2+LOG((T-D)*D)+T/(T-D) +(D*D/2.+(2.*T+1.)*LOG(1.-D
     *  /T))/(G*G)
      ELSE
        D=AMIN1(EEM,T)
        TP2=T+2.
        D2=D*D
        D3=D*D2
        D4=D*D3
        FTERM=LOG(T*D)-(BETA2/T)*( T + 2.*D - (3.*D2/2.)/TP2 -(D-D3/3.)/
     *  (TP2*TP2)-(D2/2.-T*D3/3.+D4/4.)/TP2**3)
      END IF
      IF ((EPSTFLP .EQ. 0)) THEN
        IF ((X.LE.X0)) THEN
          DELTA=0.0
        ELSE IF((X.LT.X1)) THEN
          DELTA=TOLN10*X - CBAR + AFACT*(X1 - X)**SK
        ELSE
          DELTA=TOLN10*X - CBAR
        END IF
      ELSE
        IF ((E0 .GE. EPSTEN(IEPST))) THEN
          IF ((E0 .EQ. EPSTEN(IEPST))) THEN
            GO TO 5730
          END IF
          DO 5741 I=IEPST,NEPST-1
            IF ((E0.LT.EPSTEN(I+1))) THEN
              IEPST = I
              GO TO 5730
            END IF
5741      CONTINUE
5742      CONTINUE
          IEPST = NEPST
          GO TO 5730
        ELSE
          DO 5751 I=IEPST,2,-1
            IF ((E0 .GE. EPSTEN(I-1))) THEN
              IEPST = I-1
              GO TO 5730
            END IF
5751      CONTINUE
5752      CONTINUE
          IEPST = 1
        END IF
5730    IF ((IEPST .LT. NEPST)) THEN
          DELTA = EPSTD(IEPST) + (E0 - EPSTEN(IEPST))/ (EPSTEN(IEPST+1)
     *    - EPSTEN(IEPST)) * (EPSTD(IEPST+1) - EPSTD(IEPST))
        ELSE
          DELTA = EPSTD(NEPST)
        END IF
      END IF
      SPIONB=(SPC1/BETA2)*(LOG(T + 2.) - SPC2 + FTERM - DELTA)
      RETURN
      END
      real*4 FUNCTION SPIONP(E0,EE)
      implicit none
      real*4 E0,EE,SPIONB
      SPIONP=SPIONB(E0,EE,.TRUE.)
      RETURN
      END
      real*4 FUNCTION BRMSTM(E0,EG)
      implicit none
      real*4 E0,EG,BRMSRM,AU,zero
      parameter (zero=0)
      COMMON/DERCON/RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      real*4 RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      IF ((E0.LE.RMP)) THEN
        BRMSTM=0.
      ELSE
        AU=AMIN1(EG,E0-RMP)
        BRMSTM=BRMSRM(E0,zero,AU)
      END IF
      RETURN
      END
      real*4 FUNCTION BRMSRM(E,K1,K2)
      implicit none
      real*4 E,K1,K2,BRMSRZ
      integer*4 I
      COMMON/MIXDAT/NEP,LMED,PZP(50),ZELEMP(50),WAP(50),RHOZP(50), GASPP
     *,EZ,TPZ,IDSTRN(24)
      integer*4 NEP,LMED
      real*4 PZP,ZELEMP,WAP,RHOZP,GASPP,EZ,TPZ
      CHARACTER*4 IDSTRN
      BRMSRM=0.
      DO 5761 I=1,NEP
        BRMSRM=BRMSRM+PZP(I)*BRMSRZ(ZELEMP(I),E,K1,K2)
5761  CONTINUE
5762  CONTINUE
      RETURN
      END
      real*4 FUNCTION BRMSRZ(Z,E,K1,K2)
      implicit none
      real*4 Z,E,K1,K2
      EXTERNAL BRMSFZ
      real*4 DUMMY,BRMSDZ,QD,BRMSFZ
      DUMMY=BRMSDZ(Z,E,K1)
      BRMSRZ=QD(BRMSFZ,K1,K2,'BRMSFZ')
      RETURN
      END
      real*4 FUNCTION BRMSDZ(Z,EA,K)
      implicit none
      real*4 Z,EA,K
      real*4 APRIM,XSIFP,FCOULCP,BRMSFZ
      COMMON/PMCONS/PIP,C,RME,HBAR,ECGS,EMKS,AN
      real*4 PIP,C,RME,HBAR,ECGS,EMKS,AN
      COMMON/DERCON/RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      real*4 RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      COMMON/MOLVAR/WM,ZC,ZT,ZB,ZF,ZS,ZE,ZX,ZA,ZG,ZP,ZV,ZU, RLCP,EDEN,RH
     *OP,XCCP,BLCCP,TEFF0P,XR0P
      real*4 WM,ZC,ZT,ZB,ZF,ZS,ZE,ZX,ZA,ZG,ZP,ZV,ZU,RLCP,EDEN,RHOP, XCCP
     *,BLCCP,TEFF0P,XR0P
      COMMON/LBREMZ/CONST,DELC,EBREMZ,DELTAM,XLNZ
      real*4 CONST,DELC,EBREMZ,DELTAM,XLNZ
      EBREMZ=EA
      DELC=136.*Z**(-1./3.)*RMP/EBREMZ
      CONST=APRIM(Z,EBREMZ)*(AN*RHOP/WM)*R0**2*FSC*Z*(Z+XSIFP(Z))*RLCP
      XLNZ=4./3.*LOG(Z)
      IF((EBREMZ.GE.50))XLNZ=XLNZ+4.*FCOULCP(Z)
      DELTAM=EXP((21.12-XLNZ)/4.184)-0.952
      BRMSDZ=BRMSFZ(K)
      RETURN
      END
      real*4 FUNCTION APRIM(Z,E)
      implicit none
      real*4 Z,E
      integer*4 napre,naprz,ie,iz,aprim_unit,egs_get_unit,lnblnk1
      real*4 EM,AINTP
      character aprim_file*256
      COMMON/DERCON/RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      real*4 RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      COMMON/EPSTAR/EPSTEN(150),EPSTD(150),WEPST(20), EPSTTL,NEPST,IEPST
     *,EPSTFLP, NELEPS,ZEPST(20),IAPRFL,IAPRIMP
      integer*4 ZEPST,NELEPS,IAPRFL,NEPST,IEPST,EPSTFLP,IAPRIMP
      CHARACTER EPSTTL*80
      real*4 EPSTEN,EPSTD,WEPST
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      real*4 APRIMD(115,14),EPRIM(115),ZPRIM(14),APRIMZ(115)
      DATA APRIMD/ 1.32,1.26,1.18,1.13,1.09,1.07,1.05,1.04,1.03, 1.02,8*
     *1.0, 97*0.0, 1.34,1.27,1.19,1.13,1.09,1.07,1.05,1.04,1.03,1.02, 8*
     *1.0, 97*0.0, 1.39,1.30,1.21,1.14,1.10,1.07,1.05,1.04,1.03,1.02,0.9
     *94, 2*0.991,0.990,2*0.989,2*0.988, 97*0.0, 1.46,1.34,1.23,1.15,1.1
     *1,1.08, 1.06,1.05,1.03,1.02,0.989, 0.973,0.971,0.969,0.967,0.965,2
     **0.963, 97*0.0, 1.55,1.40,1.26,1.17,1.12,1.09,1.07,1.05,1.03,1.02,
     *0.955,0.935, 0.930,0.925,0.920,0.915,2*0.911, 97*0.0,  1035*0.0/,
     *EPRIM / 2.,3.,4.,5.,6.,7.,8.,9.,10.,11.,21.,31.,41.,51.,61.,71.,81
     *.,91.,  97*0.0/, ZPRIM /6.,13.,29.,50.,79., 9*0.0/
      save APRIMD,EPRIM,ZPRIM,APRIMZ,napre,naprz
      IF ((IAPRIMP.EQ.0)) THEN
        IF ((IAPRFL .EQ. 0)) THEN
          IAPRFL=1
        END IF
        IF ((E.GE.50)) THEN
          APRIM=1.
        ELSE
          EM=E/RMP
          DO 5771 IE=1,18
            APRIMZ(IE)= AINTP(Z,ZPRIM,5,APRIMD(IE,1),115,.FALSE.,.FALSE.
     *      )
5771      CONTINUE
5772      CONTINUE
          APRIM=AINTP(EM,EPRIM,18,APRIMZ,1,.FALSE.,.FALSE.)
        END IF
      ELSE IF((IAPRIMP.EQ.1)) THEN
        IF ((IAPRFL.EQ.0)) THEN
          aprim_file = hen_house(:lnblnk1(hen_house)) // 'pegs4' // '/'
     *    // 'aprime.data'
          aprim_unit=22
          aprim_unit=egs_get_unit(aprim_unit)
          IF (( aprim_unit .LT. 1 )) THEN
            write(i_log,'(/a)') '***************** Error: '
            write(i_log,*) 'APRIM: failed to get a free fortran unit'
            write(i_log,'(/a)') '***************** Quiting now.'
            call exit(1)
          END IF
          open(aprim_unit,file=aprim_file,status='old',err=5780)
          READ(aprim_unit,*) NAPRZ, NAPRE
          IF ((NAPRZ.GT.14)) THEN
            WRITE(6,5790)
5790        FORMAT(//,' TOO MANY ELEMENTS FOR APRIME INTERPOLATION:', /,
     *'   CHANGE $NAPRZ AND RECOMPILE PEGS')
            call exit(24)
          END IF
          IF ((NAPRE.GT.115)) THEN
            WRITE(6,5800)
5800        FORMAT(//,' TOO MANY ENERGIES FOR APRIME INTERPOLATION:', /,
     *'   CHANGE $NAPRE AND RECOMPILE PEGS')
            call exit(24)
          END IF
          READ(aprim_unit,*) (EPRIM(IE),IE=1,NAPRE)
          DO 5811 IE=1,NAPRE
            EPRIM(IE)=1.+EPRIM(IE)/RMP
5811      CONTINUE
5812      CONTINUE
          DO 5821 IZ=1,NAPRZ
            READ(aprim_unit,*)ZPRIM(IZ),(APRIMD(IE,IZ),IE=1,NAPRE)
5821      CONTINUE
5822      CONTINUE
          IAPRFL=1
          close(aprim_unit)
        END IF
        EM=E/RMP
        DO 5831 IE=1,NAPRE
          APRIMZ(IE)= AINTP(Z,ZPRIM,NAPRZ,APRIMD(IE,1),115,.TRUE.,.FALSE
     *    .)
5831    CONTINUE
5832    CONTINUE
        APRIM=AINTP(EM,EPRIM,NAPRE,APRIMZ,1,.FALSE.,.FALSE.)
      ELSE IF((IAPRIMP.EQ.2)) THEN
        IF ((IAPRFL .EQ. 0)) THEN
          IAPRFL=1
        END IF
        APRIM=1.0
      ELSE
        WRITE(6,5840)IAPRIMP
5840    FORMAT(//,' ILLEGAL VALUE FOR IAPRIM: ',I4)
        call exit(24)
      END IF
      RETURN
5780  write(i_log,'(/a)') '***************** Error: '
      write(i_log,*) 'Cannot open file $HEN_HOUSE/pegs4/aprime.data'
      write(i_log,'(/a)') '***************** Quiting now.'
      call exit(1)
      RETURN
      END
      real*4 FUNCTION AINTP(X,XA,NX,YA,ISK,XLOG,YLOG)
      implicit none
      integer*4 NX,ISK
      real*4 X
      real*4 XA(NX),YA(ISK,NX)
      LOGICAL XLOG,YLOG,XLOGL
      integer*4 I,J
      real*4 XI,XJ,XV,YI,YJ
      XLOGL=XLOG
      DO 5851 J=2,NX
        IF((X.LT.XA(J)))GO TO 5860
5851  CONTINUE
5852  CONTINUE
      J=NX
5860  I=J-1
      IF ((XA(I).LE.0.0)) THEN
        XLOGL=.FALSE.
      END IF
      IF ((.NOT.XLOGL)) THEN
        XI=XA(I)
        XJ=XA(J)
        XV=X
      ELSE
        XI=LOG(XA(I))
        XJ=LOG(XA(J))
        XV=LOG(X)
      END IF
      IF ((YLOG.AND.(YA(1,I).EQ.0.0.OR.YA(1,J).EQ.0.0))) THEN
        AINTP=0.0
      ELSE
        IF ((YLOG)) THEN
          YI=LOG(YA(1,I))
          YJ=LOG(YA(1,J))
          IF ((XJ.EQ.XI)) THEN
            AINTP=YI
          ELSE
            AINTP=(YI*(XJ-XV)+YJ*(XV-XI))/(XJ-XI)
          END IF
          AINTP=EXP(AINTP)
        ELSE
          YI=YA(1,I)
          YJ=YA(1,J)
          IF ((XJ.EQ.XI)) THEN
            AINTP=YI
          ELSE
            AINTP=(YI*(XJ-XV)+YJ*(XV-XI))/(XJ-XI)
          END IF
        END IF
      END IF
      RETURN
      END
      real*4 FUNCTION TMXS(E)
      implicit none
      real*4 E,TMXB
      real*4 SAFETY,TABSMX
      DATA SAFETY/0.8/,TABSMX/10.0/
      save SAFETY,TABSMX
      TMXS=AMIN1(TMXB(E)*SAFETY,TABSMX)
      RETURN
      END
      real*4 FUNCTION TMXB(E)
      implicit none
      real*4 E
      real*4 ESQ,BETA2,PX2
      COMMON/DERCON/RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      real*4 RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      COMMON/MOLVAR/WM,ZC,ZT,ZB,ZF,ZS,ZE,ZX,ZA,ZG,ZP,ZV,ZU, RLCP,EDEN,RH
     *OP,XCCP,BLCCP,TEFF0P,XR0P
      real*4 WM,ZC,ZT,ZB,ZF,ZS,ZE,ZX,ZA,ZG,ZP,ZV,ZU,RLCP,EDEN,RHOP, XCCP
     *,BLCCP,TEFF0P,XR0P
      ESQ=E**2
      BETA2=1.0-RMPSQ/ESQ
      PX2=ESQ*BETA2/XCCP**2
      TMXB=PX2*BETA2/LOG(BLCCP*PX2)
      RETURN
      END
      real*4 FUNCTION ALKE(E)
      implicit none
      real*4 E
      COMMON/DERCON/RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      real*4 RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      ALKE=LOG(E-RMP)
      RETURN
      END
      real*4 FUNCTION ALKEI(X)
      implicit none
      real*4 x
      COMMON/DERCON/RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      real*4 RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      ALKEI=EXP(X) + RMP
      RETURN
      END
      SUBROUTINE PWLF1(NI,NIMX,XL,XU,XR,EP,ZTHR,ZEP,NIP,XFUN,XFI, AX,BX,
     *NALM,NFUN,AF,BF,VFUNS)
      implicit none
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      integer*4 NI,NIMX,NIP,NALM,NFUN
      real*4 XL,XU,XR,EP,AX,BX,XFUN,XFI
      EXTERNAL XFI,VFUNS,XFUN
      real*4 AF(NALM,NFUN),BF(NALM,NFUN),ZTHR(NFUN),ZEP(NFUN)
      LOGICAL QFIT
      integer*4 NL,NU,IPRN,NJ,NK
      real*4 REM
      NL=0
      NU=1
      IPRN=0
5871  CONTINUE
        NJ=MIN0(NU,NIMX)
        IF((QFIT(NJ,XL,XU,XR,EP,ZTHR,ZEP,REM,NIP,XFUN,XFI, AX,BX,NALM,NF
     *  UN,AF,BF,VFUNS,0)))GO TO5872
        IF ((NU.GE.NIMX)) THEN
          NI=NJ
          RETURN
        END IF
        NL=NU
        NU=NU*2
      GO TO 5871
5872  CONTINUE
      NU=NJ
5881  IF(NU.LE.NL+1)GO TO 5882
        NJ=(NL+NU)/2
        NK=NJ
        IF ((QFIT(NJ,XL,XU,XR,EP,ZTHR,ZEP,REM,NIP,XFUN,XFI, AX,BX,NALM,N
     *  FUN,AF,BF,VFUNS,0))) THEN
          NU=NJ
        ELSE
          NL=NK
        END IF
      GO TO 5881
5882  CONTINUE
      NI=NU
      IF((NI.EQ.NJ))RETURN
      IF((.NOT.QFIT(NI,XL,XU,XR,EP,ZTHR,ZEP,REM,NIP,XFUN,XFI, AX,BX,NALM
     *,NFUN,AF,BF,VFUNS,0)))WRITE(6,5890)NI
5890  FORMAT(' CATASTROPHE---DOES NOT FIT WHEN IT SHOULD,NI=',I5)
      RETURN
      END
      LOGICAL FUNCTION QFIT(NJ,XL,XH,XR,EP,ZTHR,ZEP,REM,NJP,XFUN,XFI, AX
     *,BX,NALM,NFUN,AF,BF,VFUNS,IPRN)
      implicit none
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      EXTERNAL VFUNS
      real*4 XFUN,XFI
      integer*4 NJ,NALM,NFUN,NJP,IPRN
      real*4 XL,XH,XR,AX,BX,REM,EP
      real*4 FSXL(10),FSXH(10),FIP(10),FFIP(10),AFIP(10),RE(10),AER(10)
      real*4 AF(NALM,NFUN),BF(NALM,NFUN),ZTHR(NFUN),ZEP(NFUN)
      real*4 XS,XFL,XFH,XFS,XM,DX,W,XLL,SXFL,XSXF,SXFH,DSXF,WIP, SXFIP,X
     *IP
      integer*4 NI,NIP,ISUB,IFUN,JSUB,IP
      integer*4 nkp
      DATA NKP/3/
      save nkp
      IF ((XH.LE.XL)) THEN
        WRITE(6,5900)XL,XH
5900    FORMAT(' QFIT ERROR:XL SHOULD BE < XH. XL,XH=',2G14.6)
        QFIT=.FALSE.
        RETURN
      END IF
      XS=AMAX1(XL,AMIN1(XH,XR))
      NI=NJ-2
      IF ((((XS.EQ.XL.OR.XS.EQ.XH).AND.NI.GE.1).OR.NI.GE.2)) THEN
        XFL=XFUN(XL)
      ELSE
        QFIT=.FALSE.
        RETURN
      END IF
      XFH=XFUN(XH)
      XFS=XFUN(XS)
      XM=AMAX1(XFH-XFS,XFS-XFL)
      DX=XFH-XFL
      W=XM/AMAX1(1.,AINT(NI*XM/DX))
      NI=NI-AINT(NI-DX/W)
      NIP=MAX0(NKP,(NJP+NI-1)/NI)
      NIP=(NIP/2)*2+1
      IF ((XFH-XFS.LE.XFS-XFL)) THEN
        XLL=XFL
      ELSE
        XLL=XFH-NI*W
      END IF
      AX=1./W
      BX=2.-XLL*AX
      REM=0.0
      QFIT=.TRUE.
      SXFL=AMAX1(XLL,XFL)
      ISUB=0
      XSXF=XFI(SXFL)
      CALL VFUNS(XSXF,FSXL)
      IF((IPRN.NE.0))WRITE(6,2040) ISUB,SXFL,XSXF,(FSXL(IFUN),IFUN=1,NFU
     *N)
2040  FORMAT(' QFIT:ISUB,SXF,XSXF,FSX()=',I4,1P,9G11.4/(1X,12G11.4))
      DO 5911 ISUB=1,NI
        JSUB=ISUB+1
        SXFH=AMIN1(XLL+W*ISUB,XH)
        XSXF=XFI(SXFH)
        CALL VFUNS(XSXF,FSXH)
        IF((IPRN.NE.0))WRITE(6,2040)ISUB,SXFH,XSXF,(FSXH(IFUN),IFUN=1,NF
     *  UN)
        DSXF=SXFH-SXFL
        DO 5921 IFUN=1,NFUN
          AF(JSUB,IFUN)=(FSXH(IFUN)-FSXL(IFUN))/DSXF
          BF(JSUB,IFUN)=(FSXL(IFUN)*SXFH-FSXH(IFUN)*SXFL)/DSXF
5921    CONTINUE
5922    CONTINUE
        WIP=DSXF/(NIP+1)
        DO 5931 IP=1,NIP
          SXFIP=SXFL+IP*WIP
          XIP=XFI(SXFIP)
          CALL VFUNS(XIP,FIP)
          DO 5941 IFUN=1,NFUN
            FFIP(IFUN)=AF(JSUB,IFUN)*SXFIP+BF(JSUB,IFUN)
            AFIP(IFUN)=ABS(FIP(IFUN))
            AER(IFUN)=ABS(FFIP(IFUN)-FIP(IFUN))
            RE(IFUN)=0.0
            IF ((FIP(IFUN).NE.0.0)) THEN
              RE(IFUN)=AER(IFUN)/AFIP(IFUN)
            END IF
            IF ((AFIP(IFUN).GE.ZTHR(IFUN))) THEN
              REM=AMAX1(REM,RE(IFUN))
            ELSE IF((AER(IFUN).GT.ZEP(IFUN))) THEN
              QFIT=.FALSE.
            END IF
5941      CONTINUE
5942      CONTINUE
          IF ((IPRN.NE.0)) THEN
            WRITE(6,5950)ISUB,IP,SXFIP,XIP,REM,QFIT,(FIP(IFUN),FFIP(IFUN
     *      ), RE(IFUN),AER(IFUN),IFUN=1,NFUN)
5950        FORMAT(1X,2I4,1P,2G12.5,6P,F12.0,L2,1P,2G11.4,6P,F11.0,1P,G1
     *1.4/ (1X,3(1P,2G11.4,6P,F11.0,1P,G11.4)))
          END IF
5931    CONTINUE
5932    CONTINUE
        SXFL=SXFH
        DO 5961 IFUN=1,NFUN
          FSXL(IFUN)=FSXH(IFUN)
5961    CONTINUE
5962    CONTINUE
5911  CONTINUE
5912  CONTINUE
      DO 5971 IFUN=1,NFUN
        AF(1,IFUN)=AF(2,IFUN)
        BF(1,IFUN)=BF(2,IFUN)
        AF(NI+2,IFUN)=AF(NI+1,IFUN)
        BF(NI+2,IFUN)=BF(NI+1,IFUN)
5971  CONTINUE
5972  CONTINUE
      QFIT=QFIT.AND.REM.LE.EP
      NJ=NI+2
      RETURN
      END
      real*4 FUNCTION QD(F,A,B,MSG)
      implicit none
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      real*4 A,B,F
      EXTERNAL F
      CHARACTER*6 MSG
      logical first_time
      data first_time/.true./
      save first_time
      real*4 DCADRE,ADUM,BDUM,ERRDUM
      integer*4 IER
      ADUM=A
      BDUM=B
      QD=DCADRE(F,ADUM,BDUM,1.E-16,1.E-5,ERRDUM,IER)
      IF ((IER.GT.66)) THEN
        WRITE(6,5980)IER,MSG,A,B,QD,ERRDUM
5980    FORMAT(' DCADRE CODE=',I4,' FOR INTEGRAL ',A6,' FROM ',1P,G14.6,
     *' TO ',G14.6, ',QD=',G14.6,'+-',G14.6)
      END IF
      RETURN
      END
      real*4 FUNCTION DCADRE(F,A,B,AERR,RERR,ERROR,IER)
      implicit none
      DIMENSION T(10,10),R(10),AIT(10),DIF(10),RN(4),TS(2049)
      DIMENSION IBEGS(30),BEGIN(30),FINIS(30),EST(30)
      DIMENSION REGLSV(30)
      LOGICAL H2CONV,AITKEN,RIGHT,REGLAR,REGLSV
      real*4 T,R,AIT,DIF,RN,TS,BEGIN,FINIS,EST,AITLOW
      real*4 H2TOL,AITTOL,LENGTH,JUMPTL,ZERO,P1,HALF,ONE
      real*4 TWO,FOUR,FOURP5,TEN,HUN,CADRE,ERROR,A,B
      real*4 AERR,RERR,STEPMN,STEPNM,STAGE,CUREST,FNSIZE
      real*4 PREVER,BEG,FBEG,END,FEND,STEP,ASTEP,TABS,HOVN
      real*4 FN,SUM,SUMABS,ABSI,VINT,TABTLM,ERGL,ERGOAL
      real*4 ERRA,ERRR,FEXTRP,ERRER,DIFF,SING,FEXTM1,ALG4O2
      real*4 H2NXT,SINGNX,SLOPE,FBEG2,ALPHA
      real*4 ERRET,H2TFEX,FI
      real*4 RVAL,F
      integer*4 IBEGS,IER,ISTAGE,IBEG,IEND,L,N,LM1,N2,ISTEP,II,III,I,IST
     *EP2,IT,NNLEFT
      integer*4 MAXTS,MAXTBL,MXSTGE
      DATA AITLOW,H2TOL,AITTOL,JUMPTL,MAXTS,MAXTBL,MXSTGE/1.1D0,.15D0, .
     *1D0,.01D0,2049,10,30/
      DATA RN(1),RN(2),RN(3),RN(4)/.7142005D0,.3466282D0,.843751D0, .126
     *3305D0/
      DATA ZERO,P1,HALF,ONE,TWO,FOUR,FOURP5,TEN,HUN/0.0D0,0.1D0,0.5D0, 1
     *.0D0,2.0D0,4.0D0,4.5D0,10.0D0,100.0D0/
      save MAXTS,MAXTBL,MXSTGE
      ALG4O2=LOG10(TWO)
      CADRE=ZERO
      ERROR=ZERO
      CUREST=ZERO
      VINT=ZERO
      IER=0
      LENGTH=ABS(B-A)
      IF((LENGTH.EQ.ZERO))GO TO 215
      IF((RERR.GT.P1.OR.RERR.LT.ZERO))GO TO 210
      IF((AERR.EQ.ZERO.AND.(RERR+HUN).LE.HUN))GO TO 210
      ERRR=RERR
      ERRA=ABS(AERR)
      STEPMN=(LENGTH/FLOAT(2**MXSTGE))
      STEPNM=DMAX1(LENGTH,ABS(A),ABS(B))*TEN
      STAGE=HALF
      ISTAGE=1
      FNSIZE=ZERO
      PREVER=ZERO
      REGLAR=.FALSE.
      BEG=A
      RVAL=BEG
      FBEG=F(RVAL)*HALF
      TS(1)=FBEG
      IBEG=1
      END=B
      RVAL=END
      FEND=F(RVAL)*HALF
      TS(2)=FEND
      IEND=2
5     RIGHT=.FALSE.
10    STEP=END - BEG
      ASTEP=ABS(STEP)
      IF((ASTEP.LT.STEPMN))GO TO 205
      IF((STEPNM+ASTEP.EQ.STEPNM))GO TO 205
      T(1,1)=FBEG + FEND
      TABS=ABS(FBEG) + ABS(FEND)
      L=1
      N=1
      H2CONV=.FALSE.
      AITKEN=.FALSE.
15    LM1=L
      L=L + 1
      N2=N + N
      FN=N2
      ISTEP=(IEND - IBEG)/N
      IF((ISTEP.GT.1))GO TO 25
      II=IEND
      IEND=IEND + N
      IF((IEND.GT.MAXTS))GO TO 200
      HOVN=STEP/FN
      III=IEND
      FI=ONE
      DO 5991 I=1,N2,2
        TS(III)=TS(II)
        RVAL=END-FI*HOVN
        TS(III-1)=F(RVAL)
        FI=FI+TWO
        III=III-2
        II=II-1
5991  CONTINUE
5992  CONTINUE
      ISTEP=2
25    ISTEP2=IBEG + ISTEP/2
      SUM=ZERO
      SUMABS=ZERO
      DO 6001 I=ISTEP2,IEND,ISTEP
        SUM=SUM + TS(I)
        SUMABS=SUMABS + ABS(TS(I))
6001  CONTINUE
6002  CONTINUE
      T(L,1)=T(L-1,1)*HALF+SUM/FN
      TABS=TABS*HALF+SUMABS/FN
      ABSI=ASTEP*TABS
      N=N2
      IT=1
      VINT=STEP*T(L,1)
      TABTLM=TABS*TEN
      FNSIZE=DMAX1(FNSIZE,ABS(T(L,1)))
      ERGL=ASTEP*FNSIZE*TEN
      ERGOAL=STAGE*DMAX1(ERRA,ERRR*ABS(CUREST+VINT))
      FEXTRP=ONE
      DO 6011 I=1,LM1
        FEXTRP=FEXTRP*FOUR
        T(I,L)=T(L,I) - T(L-1,I)
        T(L,I+1)=T(L,I) + T(I,L)/(FEXTRP-ONE)
6011  CONTINUE
6012  CONTINUE
      ERRER=ASTEP*ABS(T(1,L))
      IF((L.GT.2))GO TO 40
      IF((TABS+P1*ABS(T(1,2)).EQ.TABS))GO TO 135
      GO TO 15
40    DO 45 I=2,LM1
      DIFF=ZERO
      IF((TABTLM+ABS(T(I-1,L)).NE.TABTLM))DIFF=T(I-1,LM1)/T(I-1,L)
      T(I-1,LM1)=DIFF
45    CONTINUE
      IF((ABS(FOUR-T(1,LM1)).LE.H2TOL))GO TO 60
      IF((T(1,LM1).EQ.ZERO))GO TO 55
      IF((ABS(TWO-ABS(T(1,LM1))).LT.JUMPTL))GO TO 130
      IF((L.EQ.3))GO TO 15
      H2CONV=.FALSE.
      IF((ABS((T(1,LM1)-T(1,L-2))/T(1,LM1)).LE.AITTOL))GO TO 75
50    IF(REGLAR) GO TO 55
      IF((L.EQ.4))GO TO 15
55    IF(ERRER.GT.ERGOAL.AND.(ERGL+ERRER).NE.ERGL) GO TO 175
      GO TO 145
60    IF(H2CONV) GO TO 65
      AITKEN=.FALSE.
      H2CONV=.TRUE.
65    FEXTRP=FOUR
70    IT=IT + 1
      VINT=STEP*T(L,IT)
      ERRER=ABS(STEP/(FEXTRP-ONE)*T(IT-1,L))
      IF((ERRER.LE.ERGOAL))GO TO 160
      IF((ERGL+ERRER.EQ.ERGL))GO TO 160
      IF((IT.EQ.LM1))GO TO 125
      IF((T(IT,LM1).EQ.ZERO))GO TO 70
      IF((T(IT,LM1).LE.FEXTRP))GO TO 125
      IF((ABS(T(IT,LM1)/FOUR-FEXTRP)/FEXTRP.LT.AITTOL))FEXTRP=FEXTRP*FOU
     *R
      GO TO 70
75    IF(T(1,LM1).LT.AITLOW) GO TO 175
      IF((AITKEN))GO TO 80
      H2CONV=.FALSE.
      AITKEN=.TRUE.
80    FEXTRP=T(L-2,LM1)
      IF((FEXTRP.GT.FOURP5))GO TO 65
      IF((FEXTRP.LT.AITLOW))GO TO 175
      IF((ABS(FEXTRP-T(L-3,LM1))/T(1,LM1).GT.H2TOL))GO TO 175
      SING=FEXTRP
      FEXTM1=ONE/(FEXTRP - ONE)
      AIT(1)=ZERO
      DO 85 I=2,L
      AIT(I)=T(I,1) + (T(I,1)-T(I-1,1))*FEXTM1
      R(I)=T(1,I-1)
      DIF(I)=AIT(I) - AIT(I-1)
85    CONTINUE
      IT=2
90    VINT=STEP*AIT(L)
      ERRER=ERRER*FEXTM1
      IF((ERRER.GT.ERGOAL.AND.(ERGL+ERRER).NE.ERGL))GO TO 95
      ALPHA=LOG10(SING)/ALG4O2 - ONE
      IER=MAX0(IER,65)
      GO TO 160
95    IT=IT + 1
      IF((IT.EQ.LM1))GO TO 125
      IF((IT.GT.3))GO TO 100
      H2NXT=FOUR
      SINGNX=SING+SING
100   IF(H2NXT.LT.SINGNX) GO TO 105
      FEXTRP=SINGNX
      SINGNX=SINGNX+SINGNX
      GO TO 110
105   FEXTRP=H2NXT
      H2NXT=FOUR*H2NXT
110   DO 115 I=IT,LM1
      R(I+1)=ZERO
      IF((TABTLM+ABS(DIF(I+1)).NE.TABTLM))R(I+1)=DIF(I)/DIF(I+1)
115   CONTINUE
      H2TFEX=-H2TOL*FEXTRP
      IF((R(L)-FEXTRP.LT.H2TFEX))GO TO 125
      IF((R(L-1)-FEXTRP.LT.H2TFEX))GO TO 125
      ERRER=ASTEP*ABS(DIF(L))
      FEXTM1=ONE/(FEXTRP - ONE)
      DO 120 I=IT,L
      AIT(I)=AIT(I) + DIF(I)*FEXTM1
      DIF(I)=AIT(I) - AIT(I-1)
120   CONTINUE
      GO TO 90
125   FEXTRP=DMAX1(PREVER/ERRER,AITLOW)
      PREVER=ERRER
      IF((L.LT.5))GO TO 15
      IF((L-IT.GT.2.AND.ISTAGE.LT.MXSTGE))GO TO 170
      ERRET=ERRER/(FEXTRP**(MAXTBL-L))
      IF((ERRET.GT.ERGOAL.AND.(ERGL+ERRET).NE.ERGL))GO TO 170
      GO TO 15
130   IF(ERRER.GT.ERGOAL.AND.(ERGL+ERRER).NE.ERGL) GO TO 170
      DIFF=ABS(T(1,L))*(FN+FN)
      GO TO 160
135   SLOPE=(FEND-FBEG)*TWO
      FBEG2=FBEG+FBEG
      DO 140 I=1,4
      RVAL=BEG+RN(I)*STEP
      DIFF=ABS(F(RVAL) - FBEG2-RN(I)*SLOPE)
      IF((TABTLM+DIFF.NE.TABTLM))GO TO 155
140   CONTINUE
      GO TO 160
145   SLOPE=(FEND-FBEG)*TWO
      FBEG2=FBEG+FBEG
      I=1
150   RVAL=BEG+RN(I)*STEP
      DIFF=ABS(F(RVAL) - FBEG2-RN(I)*SLOPE)
155   ERRER=DMAX1(ERRER,ASTEP*DIFF)
      IF((ERRER.GT.ERGOAL.AND.(ERGL+ERRER).NE.ERGL))GO TO 175
      I=I+1
      IF((I.LE.4))GO TO 150
      IER=66
160   CADRE=CADRE + VINT
      ERROR=ERROR + ERRER
      IF((RIGHT))GO TO 165
      ISTAGE=ISTAGE - 1
      IF((ISTAGE.EQ.0))GO TO 220
      REGLAR=REGLSV(ISTAGE)
      BEG=BEGIN(ISTAGE)
      END=FINIS(ISTAGE)
      CUREST=CUREST - EST(ISTAGE+1) + VINT
      IEND=IBEG - 1
      FEND=TS(IEND)
      IBEG=IBEGS(ISTAGE)
      GO TO 180
165   CUREST=CUREST + VINT
      STAGE=STAGE+STAGE
      IEND=IBEG
      IBEG=IBEGS(ISTAGE)
      END=BEG
      BEG=BEGIN(ISTAGE)
      FEND=FBEG
      FBEG=TS(IBEG)
      GO TO 5
170   REGLAR=.TRUE.
175   IF(ISTAGE.EQ.MXSTGE) GO TO 205
      IF((RIGHT))GO TO 185
      REGLSV(ISTAGE+1)=REGLAR
      BEGIN(ISTAGE)=BEG
      IBEGS(ISTAGE)=IBEG
      STAGE=STAGE*HALF
180   RIGHT=.TRUE.
      BEG=(BEG+END)*HALF
      IBEG=(IBEG+IEND)/2
      TS(IBEG)=TS(IBEG)*HALF
      FBEG=TS(IBEG)
      GO TO 10
185   NNLEFT=IBEG - IBEGS(ISTAGE)
      IF((IEND+NNLEFT.GE.MAXTS))GO TO 200
      III=IBEGS(ISTAGE)
      II=IEND
      DO 190 I=III,IBEG
      II=II + 1
      TS(II)=TS(I)
190   CONTINUE
      DO 195 I=IBEG,II
      TS(III)=TS(I)
      III=III + 1
195   CONTINUE
      IEND=IEND + 1
      IBEG=IEND - NNLEFT
      FEND=FBEG
      FBEG=TS(IBEG)
      FINIS(ISTAGE)=END
      END=BEG
      BEG=BEGIN(ISTAGE)
      BEGIN(ISTAGE)=END
      REGLSV(ISTAGE)=REGLAR
      ISTAGE=ISTAGE + 1
      REGLAR=REGLSV(ISTAGE)
      EST(ISTAGE)=VINT
      CUREST=CUREST + EST(ISTAGE)
      GO TO 5
200   IER=131
      GO TO 215
205   IER=132
      GO TO 215
210   IER=133
215   CADRE=CUREST + VINT
220   DCADRE=CADRE
9000  CONTINUE
9005  RETURN
      END
      SUBROUTINE SPINIT(density_file)
      implicit none
      COMMON/PMCONS/PIP,C,RME,HBAR,ECGS,EMKS,AN
      real*4 PIP,C,RME,HBAR,ECGS,EMKS,AN
      COMMON/SPCOMM/MEDTBL(24,73), NUMSTMED,STDATA(6,73)
      CHARACTER*4 MEDTBL
      integer*4 NUMSTMED
      real*4 STDATA
      COMMON/DERCON/RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      real*4 RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      COMMON/MOLVAR/WM,ZC,ZT,ZB,ZF,ZS,ZE,ZX,ZA,ZG,ZP,ZV,ZU, RLCP,EDEN,RH
     *OP,XCCP,BLCCP,TEFF0P,XR0P
      real*4 WM,ZC,ZT,ZB,ZF,ZS,ZE,ZX,ZA,ZG,ZP,ZV,ZU,RLCP,EDEN,RHOP, XCCP
     *,BLCCP,TEFF0P,XR0P
      COMMON/BREMPRP/DLP1(6),DLP2(6),DLP3(6),DLP4(6),DLP5(6),DLP6(6), DE
     *LCMP,ALPHIP(2),BPARP(2),DELPOSP(2)
      real*4 dlP1,dlP2,dlP3,dlP4,dlP5,dlP6,delcmP,alphiP,bparP,delposP
      COMMON/ELEMTB/NET,ITBL(100),WATBL(100),RHOTBL(100),ASYMT(100)
      integer*4 NET
      real*4 ITBL,WATBL,RHOTBL
      CHARACTER*4 ASYMT
      COMMON/LSPION/CBAR,X0,X1,SK,TOLN10,AFACT,SPC1,SPC2,IEV
      real*4 CBAR,X0,X1,SK,TOLN10,AFACT,SPC1,SPC2,IEV
      COMMON/EPSTAR/EPSTEN(150),EPSTD(150),WEPST(20), EPSTTL,NEPST,IEPST
     *,EPSTFLP, NELEPS,ZEPST(20),IAPRFL,IAPRIMP
      integer*4 ZEPST,NELEPS,IAPRFL,NEPST,IEPST,EPSTFLP,IAPRIMP
      CHARACTER EPSTTL*80
      real*4 EPSTEN,EPSTD,WEPST
      COMMON/THRESHP/APP,AEP,UPP,UEP,THBREMP,THMOLLP,TEP,IUNRSTP
      real*4 APP,AEP,UPP,UEP,THBREMP,THMOLLP,TEP
      integer*4 IUNRSTP
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      COMMON/MIXDAT/NEP,LMED,PZP(50),ZELEMP(50),WAP(50),RHOZP(50), GASPP
     *,EZ,TPZ,IDSTRN(24)
      integer*4 NEP,LMED
      real*4 PZP,ZELEMP,WAP,RHOZP,GASPP,EZ,TPZ
      CHARACTER*4 IDSTRN
      COMMON/MISC/  DUNIT,KMPI,KMPO,RHOR(1),MED(1),IRAYLR(1),IPHOTONUCR(
     *1)
      real*8 DUNIT,  RHOR
      integer*4 KMPI,  KMPO
      integer*2 MED,  IRAYLR,  IPHOTONUCR
      real*4 IMEV
      integer*4 IM,J,IZ,IE,I,ICHECK,IESPEL,IPEGEL,density_unit,lnblnk1,e
     *gs_get_unit
      real*4 VPLASM,ALIADG,EDENL,ALGASP,EPSTRH, TLRNCE,EPSTWT
      CHARACTER*256 density_file
      TOLN10=2.0*LOG(10.0)
      IM=-100
      IF ((EPSTFLP .LT. 0 .OR. EPSTFLP .GT. 1)) THEN
        EPSTFLP = 0
      END IF
      IF ((EPSTFLP.EQ.0)) THEN
6020    CONTINUE
          DO 6021 IM=1,NUMSTMED
          DO 6031 J=1,LMED
            IF((IDSTRN(J).NE.MEDTBL(J,IM)))GO TO 6021
6031      CONTINUE
6032      CONTINUE
          AFACT=STDATA(1,IM)
          SK=STDATA(2,IM)
          X0=STDATA(3,IM)
          X1=STDATA(4,IM)
          IEV=STDATA(5,IM)
          CBAR=STDATA(6,IM)
          IMEV=IEV*1.0E-6
          VPLASM=SQRT(EDEN*R0*C**2/PIP)
          GO TO 6040
6021    CONTINUE
6022    CONTINUE
        IM=0
        IF ((NEP.EQ.1)) THEN
          IZ=ZELEMP(1)
          IF ((IZ.EQ.1.OR.IZ.EQ.7.OR.IZ.EQ.8)) THEN
            WRITE(6,6050)
6050        FORMAT(' STOPPED IN SUBROUTINE SPINIT BECAUSE THIS',/, ' ELE
     *MENT (H, N, OR O) CAN ONLY EXIST AS A DIATOMIC MOLECULE.',/, ' REM
     *EDY:  USE COMP OPTION FOR H2, N2, OR O2 WITH NE=2,PZ=1,1'/, '     
     *AND, IN THE CASE OF A GAS, DEFINE STERNHEIMER ID',/, '   (I.E., ID
     *STRN) LIKE H2-GAS')
            call exit(21)
          END IF
          IEV=ITBL(IZ)
        ELSE
          ALIADG=0.0
          DO 6061 IE=1,NEP
            IZ=ZELEMP(IE)
            IF ((IZ.EQ.1)) THEN
              IEV=19.2
            ELSE IF((IZ.EQ.6)) THEN
              IF ((GASPP.EQ.0.0)) THEN
                IEV=81.0
              ELSE
                IEV=70.0
              END IF
            ELSE IF((IZ.EQ.7)) THEN
              IEV=82.0
            ELSE IF((IZ.EQ.8)) THEN
              IF ((GASPP.EQ.0.0)) THEN
                IEV=106.0
              ELSE
                IEV=97.0
              END IF
            ELSE IF((IZ.EQ.9)) THEN
              IEV=112.0
            ELSE IF((IZ.EQ.17)) THEN
              IEV=180.0
            ELSE
              IEV=1.13*ITBL(IZ)
            END IF
            ALIADG=ALIADG + PZP(IE)*ZELEMP(IE)*LOG(IEV)
6061      CONTINUE
6062      CONTINUE
          ALIADG=ALIADG/ZC
          IEV=EXP(ALIADG)
        END IF
        IMEV=IEV*1.0E-6
        IF ((GASPP.EQ.0.0)) THEN
          EDENL=EDEN
        ELSE
          EDENL=EDEN/GASPP
        END IF
        VPLASM = SQRT(EDENL*R0*C**2/PIP)
        CBAR=1. + 2.*LOG(IMEV/(HBAR*2*PIP*VPLASM/ERGMEV))
        IF ((NEP.EQ.1.AND.INT(ZELEMP(1)).EQ.2.AND.GASPP.NE.0.0)) THEN
          X0=2.191
          X1=3.0
          SK=3.297
        ELSE IF((NEP.EQ.2.AND.INT(ZELEMP(1)).EQ.1 .AND.INT(ZELEMP(2)).EQ
     *  .1)) THEN
          IF ((GASPP.EQ.0.0)) THEN
            X0=0.425
            X1=2.0
            SK=5.949
          ELSE
            X0=1.837
            X1=3.0
            SK=4.754
          END IF
        ELSE
          SK=3.0
          IF ((GASPP.EQ.0.0)) THEN
            IF ((IEV.LT.100.0)) THEN
              IF ((CBAR.LT.3.681)) THEN
                X0=0.2
                X1=2.0
              ELSE
                X0=0.326*CBAR - 1.0
                X1=2.0
              END IF
            ELSE
              IF ((CBAR.LT.5.215)) THEN
                X0=0.2
                X1=3.0
              ELSE
                X0=0.326*CBAR - 1.5
                X1=3.0
              END IF
            END IF
            IF ((X0.GE.X1)) THEN
              WRITE(6,6070)X0,X1,CBAR
6070          FORMAT(' STOPPED IN SPINIT DUE TO X0.GE.X1 , X0,X1,CBAR=',
     *3G15.5,/ ,' IF THIS IS GAS, YOU MUST DEFINE GASP(ATM)')
              call exit(21)
            END IF
          ELSE
            IF ((CBAR.LT.10.0)) THEN
              X0=1.6
              X1=4.0
            ELSE IF((CBAR.LT.10.5)) THEN
              X0=1.7
              X1=4.0
            ELSE IF((CBAR.LT.11.0)) THEN
              X0=1.8
              X1=4.0
            ELSE IF((CBAR.LT.11.5)) THEN
              X0=1.9
              X1=4.0
            ELSE IF((CBAR.LT.12.25)) THEN
              X0=2.0
              X1=4.0
            ELSE IF((CBAR.LT.13.804)) THEN
              X0=2.0
              X1=5.0
            ELSE
              X0=0.326*CBAR - 2.5
              X1=5.0
            END IF
          END IF
        END IF
6040    IF ((GASPP.NE.0.0)) THEN
          ALGASP=LOG(GASPP)
          CBAR=CBAR - ALGASP
          X0=X0 - ALGASP/TOLN10
          X1=X1 - ALGASP/TOLN10
        END IF
        IF ((IM.EQ.0)) THEN
          AFACT=(CBAR - TOLN10*X0)/(X1 - X0)**SK
        END IF
      ELSE
        density_file=density_file(:lnblnk1(density_file))
        density_unit=20
        density_unit=egs_get_unit(density_unit)
        IF (( density_unit .LT. 1 )) THEN
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,*) 'SPINIT: failed to get a free fortran unit'
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        END IF
        open(density_unit,file=density_file,status='old',err=3060)
        READ(density_unit,6080)EPSTTL
6080    FORMAT(A)
        READ(density_unit,*) NEPST,IEV,EPSTRH,NELEPS
        READ(density_unit,*) (ZEPST(I),WEPST(I),I=1,NELEPS)
        READ(density_unit,*) (EPSTEN(I),EPSTD(I),I=1,NEPST)
        close(density_unit)
        IF ((NEPST.GT.150)) THEN
          WRITE(6,6090)NEPST
6090      FORMAT(//' *****NEPST=',I4,' IS GREATER THAN THE 150 ALLOWED')
          call exit(22)
        END IF
        DO 6101 I=1,NEPST
          EPSTEN(I) = EPSTEN(I) + RMP
6101    CONTINUE
6102    CONTINUE
        IMEV = IEV*1.E-06
        IF (( AEP .LT. EPSTEN(1))) THEN
          WRITE(6,6110)EPSTEN(1),AEP
6110      FORMAT(//' ****LOWEST ENERGY INPUT FOR DENSITY EFFECT IS',1P,E
     *10.3/ T20,'WHICH IS HIGHER THAN THE VALUE OF AE=',1P,E10.3,' MEV'/
     * ' ***IT HAS BEEN SET TO AE***'//)
          EPSTEN(1) = AEP
        END IF
        IF (( UEP .GT. EPSTEN(NEPST))) THEN
          WRITE(6,6120)EPSTEN(NEPST),UEP
6120      FORMAT(//' ****HIGHEST ENERGY INPUT FOR DENSITY EFFECT IS',1P,
     *E10.3/ T20,'WHICH IS LOWER THAN THE VALUE OF UE=',1P,E10.3,' MEV'/
     * ' ***IT HAS BEEN SET TO UE***'//)
          EPSTEN(NEPST) = UEP
        END IF
        ICHECK=0
        TLRNCE=0.01
        IF((NELEPS.NE.NEP))ICHECK=1
        IF(((ICHECK.EQ.0) .AND. ( (EPSTRH.LT.((1.0-TLRNCE)*RHOP)) .OR. (
     *  EPSTRH.GT.((1.0+TLRNCE)*RHOP)) )))ICHECK=1
        EPSTWT = 0.0
        DO 6131 I=1,NEP
          EPSTWT = EPSTWT + RHOZP(I)
6131    CONTINUE
6132    CONTINUE
        IF ((EPSTWT.EQ.0.0)) THEN
          WRITE(6,6140)
6140      FORMAT(//' *****IN SPINIT***SOMETHING WRONG, MOLECULAR WEIGHTO
     *F', 'MOLECULE IS ZERO (I.E. SUM OF RHOZ)***'//)
        END IF
        IF ((ICHECK.EQ.0)) THEN
          IESPEL=0
          ICHECK=1
6151      CONTINUE
            IESPEL=IESPEL+1
            IPEGEL=0
6161        CONTINUE
              IPEGEL=IPEGEL+1
              IF ((INT(ZELEMP(IPEGEL)).EQ.ZEPST(IESPEL))) THEN
                ICHECK=0
                GO TO6162
              END IF
              IF(IPEGEL.GE.NEP)GO TO6162
            GO TO 6161
6162        CONTINUE
            IF(((ICHECK.EQ.0)  .AND. ( (WEPST(IESPEL).LT.((1.0-TLRNCE)*R
     *      HOZP(IPEGEL)/EPSTWT)) .OR. (WEPST(IESPEL).GT.((1.0+TLRNCE)*R
     *      HOZP(IPEGEL)/EPSTWT)) )))ICHECK=1
            IF(IESPEL.GE.NELEPS)GO TO6152
          GO TO 6151
6152      CONTINUE
        END IF
        IF ((ICHECK.EQ.1)) THEN
          WRITE(6,6170)
6170      FORMAT(////'0*** COMPOSITION IN INPUT DENSITY FILE DOES NOT MA
     *TCH ', ' THAT BEING USED BY PEGS'//' ***** QUITTING EARLY***'////)
          call exit(23)
        END IF
      END IF
      SPC1=2.*PIP*R0**2*RMP*EDEN*RLCP
      SPC2=LOG((IMEV/RMP)**2/2.0)
      RETURN
3060  write(i_log,'(/a)') '***************** Error: '
      write(i_log,*) ' Failed to open density file ',density_file
      write(i_log,'(/a)') '***************** Quiting now.'
      call exit(1)
      RETURN
      END
      SUBROUTINE MIX
      implicit none
      integer*4 I,IZZ
      real*4 AL183,ZAB,V2000
      real*4 FCOULCP,XSIFP
      COMMON/MIMSD/BMIN
      real*4 BMIN
      COMMON/MIXDAT/NEP,LMED,PZP(50),ZELEMP(50),WAP(50),RHOZP(50), GASPP
     *,EZ,TPZ,IDSTRN(24)
      integer*4 NEP,LMED
      real*4 PZP,ZELEMP,WAP,RHOZP,GASPP,EZ,TPZ
      CHARACTER*4 IDSTRN
      COMMON/MOLVAR/WM,ZC,ZT,ZB,ZF,ZS,ZE,ZX,ZA,ZG,ZP,ZV,ZU, RLCP,EDEN,RH
     *OP,XCCP,BLCCP,TEFF0P,XR0P
      real*4 WM,ZC,ZT,ZB,ZF,ZS,ZE,ZX,ZA,ZG,ZP,ZV,ZU,RLCP,EDEN,RHOP, XCCP
     *,BLCCP,TEFF0P,XR0P
      COMMON/PMCONS/PIP,C,RME,HBAR,ECGS,EMKS,AN
      real*4 PIP,C,RME,HBAR,ECGS,EMKS,AN
      COMMON/DERCON/RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      real*4 RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      COMMON/ADLEN/ALRAD(4),ALRADP(4),A1440,A183
      real*4 ALRAD,ALRADP,A1440,A183
      COMMON/BREMPRP/DLP1(6),DLP2(6),DLP3(6),DLP4(6),DLP5(6),DLP6(6), DE
     *LCMP,ALPHIP(2),BPARP(2),DELPOSP(2)
      real*4 dlP1,dlP2,dlP3,dlP4,dlP5,dlP6,delcmP,alphiP,bparP,delposP
      real*4 XSI(20),ZZX(20),FZC(20),FCOUL(20),ZZ(20)
      IF ((GASPP.NE.0.0)) THEN
        RHOP=GASPP*RHOP
      END IF
      AL183 = LOG(A183)
      TPZ=0.0
      WM=0.0
      ZC=0.0
      ZT=0.0
      ZB=0.0
      ZF=0.0
      ZS=0.0
      ZE=0.0
      ZX=0.0
      ZAB=0.0
      DO 6181 I=1,NEP
        TPZ = TPZ + PZP(I)
        WM = WM + PZP(I)*WAP(I)
        ZC = ZC + PZP(I)*ZELEMP(I)
        FZC(I) =(FSC*ZELEMP(I))**2
        FCOUL(I) = FCOULCP(ZELEMP(I))
        XSI(I) = XSIFP (ZELEMP(I))
        ZZX(I) = PZP(I)*ZELEMP(I)*(ZELEMP(I)+XSI(I))
        IF ((ZELEMP(I).LE.4.0)) THEN
          IZZ=ZELEMP(I)
          ZAB=ZAB+ZZX(I)*ALRAD(IZZ)
        ELSE
          ZAB=ZAB+ZZX(I)*(AL183+LOG(ZELEMP(I)**(-1./3.)))
        END IF
        ZT = ZT + ZZX(I)
        ZB = ZB + ZZX(I)*LOG(ZELEMP(I)**(-1./3.))
        ZF = ZF + ZZX(I)*FCOUL(I)
        ZZ(I) = PZP(I)*ZELEMP(I)*(ZELEMP(I)+1.0)
        ZS = ZS + ZZ(I)
        ZE = ZE + ZZ(I)*((-2./3.)*LOG(ZELEMP(I)))
        ZX = ZX + ZZ(I)*LOG(1.+3.34*FZC(I))
6181  CONTINUE
6182  CONTINUE
      EZ = ZC/TPZ
      ZA = AL183*ZT
      ZG = ZB/ZT
      ZP = ZB/ZA
      ZV = (ZB-ZF)/ZT
      ZU = (ZB-ZF)/ZA
      EDEN=AN*RHOP/WM*ZC
      RLCP = 1./( (AN*RHOP/WM)*4.0*FSC*R0**2*(ZAB-ZF) )
      BLCCP= A6680*RHOP*ZS*EXP(ZE/ZS)*RLCP/(WM*EXP(ZX/ZS))
      TEFF0P = ( EXP(BMIN)/BMIN )/BLCCP
      XCCP= (A22P9/RADDEG) * SQRT( ZS*RHOP*RLCP/WM )
      XR0P = XCCP*SQRT(TEFF0P*BMIN)
      RETURN
      END
      SUBROUTINE DIFFER
      implicit none
      real*4 AL183,F10,F20,A1DEN,A2DEN,B1DEN,B2DEN,C1DEN,C2DEN
      INTEGER I
      COMMON/MOLVAR/WM,ZC,ZT,ZB,ZF,ZS,ZE,ZX,ZA,ZG,ZP,ZV,ZU, RLCP,EDEN,RH
     *OP,XCCP,BLCCP,TEFF0P,XR0P
      real*4 WM,ZC,ZT,ZB,ZF,ZS,ZE,ZX,ZA,ZG,ZP,ZV,ZU,RLCP,EDEN,RHOP, XCCP
     *,BLCCP,TEFF0P,XR0P
      COMMON/BREMPRP/DLP1(6),DLP2(6),DLP3(6),DLP4(6),DLP5(6),DLP6(6), DE
     *LCMP,ALPHIP(2),BPARP(2),DELPOSP(2)
      real*4 dlP1,dlP2,dlP3,dlP4,dlP5,dlP6,delcmP,alphiP,bparP,delposP
      COMMON/DBRPR/ALFP1(2),ALFP2(2),AL2
      real*4 ALFP1,ALFP2,al2
      COMMON/DERCON/RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      real*4 RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      COMMON/ADLEN/ALRAD(4),ALRADP(4),A1440,A183
      real*4 ALRAD,ALRADP,A1440,A183
      AL2 = LOG(2.)
      AL183= LOG(A183)
      ALPHIP(1)= AL2*(4./3. + 1./(9.*AL183*(1.+ZP)))
      ALPHIP(2)= AL2*(4./3. + 1./(9.*AL183*(1.+ZU)))
      ALFP1(1)= 2./3. - 1./(36.*AL183*(1.+ZP))
      ALFP1(2)= 2./3. - 1./(36.*AL183*(1.+ZU))
      ALFP2(1)= (1./12.)*(4./3. + 1./(9.*AL183*(1+ZP)))
      ALFP2(2)= (1./12.)*(4./3. + 1./(9.*AL183*(1+ZU)))
      BPARP(1)= ALFP1(1)/(ALFP1(1)+ALFP2(1))
      BPARP(2)= ALFP1(2)/(ALFP1(2)+ALFP2(2))
      DELCMP= 136.0*EXP(ZG)*RMP
      DELPOSP(1)= (EXP((21.12+4.*ZG)/4.184)-0.952)/DELCMP
      DELPOSP(2)= (EXP((21.12+4.*ZV)/4.184)-0.952)/DELCMP
      F10=4.*AL183
      F20=F10 - 2./3.
      A1DEN =3.0*F10- F20 + 8.0*ZG
      A2DEN =3.0*F10- F20 + 8.0*ZV
      B1DEN = F10 + 4.0*ZG
      B2DEN = F10 + 4.0*ZV
      C1DEN = 3.0*F10+ F20 + 16.0*ZG
      C2DEN = 3.0*F10+ F20 + 16.0*ZV
      DLP1(1)= (3.0*20.867-20.209+8.0*ZG)/A1DEN
      DLP2(1)= (3.0*(-3.242)-(-1.930))/A1DEN
      DLP3(1)= (3.0*(0.625)-(0.086))/A1DEN
      DLP4(1)= (2.0*21.12+8.0*ZG)/A1DEN
      DLP5(1)= 2.0*(-4.184)/A1DEN
      DLP6(1)= 0.952
      DLP1(4)= (3.0*20.867-20.209+8.0*ZV)/A2DEN
      DLP2(4)= (3.0*(-3.242)-(-1.930))/A2DEN
      DLP3(4)= (3.0*(0.625)-(0.086))/A2DEN
      DLP4(4)= (2.0*21.12+8.0*ZV)/A2DEN
      DLP5(4)= 2.0*(-4.184)/A2DEN
      DLP6(4)= 0.952
      DLP1(2)= (20.867+4.0*ZG)/B1DEN
      DLP2(2)= -3.242/B1DEN
      DLP3(2)= 0.625/B1DEN
      DLP4(2)= (21.12+4.0*ZG)/B1DEN
      DLP5(2)= -4.184/B1DEN
      DLP6(2)= 0.952
      DLP1(5)= (20.867+4.0*ZV)/B2DEN
      DLP2(5)= -3.242/B2DEN
      DLP3(5)= 0.625/B2DEN
      DLP4(5)= (21.12+4.0*ZV)/B2DEN
      DLP5(5)= -4.184/B2DEN
      DLP6(5)= 0.952
      DLP1(3)= (3.0*20.867+20.209+16.0*ZG)/C1DEN
      DLP2(3)= (3.0*(-3.242)+(-1.930))/C1DEN
      DLP3(3)= (3.0*0.625+(-0.086))/C1DEN
      DLP4(3)= (4.0*21.12+16.0*ZG)/C1DEN
      DLP5(3)= 4.0*(-4.184)/C1DEN
      DLP6(3)= 0.952
      DLP1(6)= (3.0*20.867+20.209+16.0*ZV)/C2DEN
      DLP2(6)= (3.0*(-3.242)+(-1.930))/C2DEN
      DLP3(6)= (3.0*0.625+(-0.086))/C2DEN
      DLP4(6)= (4.0*21.12+16.0*ZV)/C2DEN
      DLP5(6)= 4.0*(-4.184)/C2DEN
      DLP6(6)= 0.952
      RETURN
      END
      real*4 function FCOULCP(Z)
      implicit none
      real*4 Z,ASQ
      COMMON/DERCON/RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      real*4 RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      ASQ=(FSC*Z)**2
      FCOULCP = ASQ*(1.0/(1.0+ASQ)+0.20206+ASQ*(-0.0369+ ASQ*(0.0083+ASQ
     **(-0.002))))
      RETURN
      END
      real*4 function XSIFP(Z)
      implicit none
      real*4 Z,FCOULCP
      integer*4 IZ
      COMMON/ADLEN/ALRAD(4),ALRADP(4),A1440,A183
      real*4 ALRAD,ALRADP,A1440,A183
      IF ((Z.LE.4.0)) THEN
        IZ=Z
        XSIFP=ALRADP(IZ)/(ALRAD(IZ)-FCOULCP(Z))
      ELSE
        XSIFP=ALOG(A1440*Z**(-2./3.))/(ALOG(A183*Z**(-1./3.))-FCOULCP(Z)
     *  )
      END IF
      RETURN
      END
      real*4 FUNCTION ZTBL(IASYM)
      implicit none
      COMMON/ELEMTB/NET,ITBL(100),WATBL(100),RHOTBL(100),ASYMT(100)
      integer*4 NET
      real*4 ITBL,WATBL,RHOTBL
      CHARACTER*4 ASYMT
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      CHARACTER*4 IASYM,IA
      integer*4 ie
      DATA IA/'A'/
      save ia
      IF ((IASYM.EQ.IA)) THEN
        ZTBL=18.0
        RETURN
      END IF
      DO 6191 IE=1,NET
        IF ((IASYM.EQ.ASYMT(IE))) THEN
          ZTBL=IE
          RETURN
        END IF
6191  CONTINUE
6192  CONTINUE
      WRITE(6,6200)IASYM,NET
6200  FORMAT(1X,A2,' NOT AN ATOMIC SYMBOL FOR AN ELEMENT WITH Z LE ',I3)
      ZTBL=0.0
      RETURN
      END
      SUBROUTINE ANNIH
      implicit none
      COMMON/QDEBUG/QDEBUG
      LOGICAL QDEBUG
      COMMON/STACK/ E(50),X(50),Y(50),Z(50),U(50),V(50),W(50),DNEAR(50),
     *WT(50),IQ(50),IR(50),LATCH(50), LATCHI,NP,NPold
      DOUBLE PRECISION E
      real*8 X,Y,Z,  U,V,W,  DNEAR,  WT
      integer*4 IQ,  IR,  LATCH,  LATCHI, NP,  NPold
      COMMON/UPHIOT/THETA,SINTHE,COSTHE,SINPHI, COSPHI,PI,TWOPI,PI5D2
      real*8 THETA,  SINTHE,  COSTHE,  SINPHI,  COSPHI,  PI,TWOPI,PI5D2
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      common/egs_vr/ e_max_rr(1),  prob_RR,  nbr_split,  i_play_RR,
     * i_survived_RR,
     *     n_RR_warning,                                        i_do_rr(
     *1)
      real*8          e_max_rr,prob_RR
      integer*4       nbr_split,i_play_RR,i_survived_RR,n_RR_warning
      integer*2     i_do_rr
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      DOUBLE PRECISION PAVIP,  PESG1,  PESG2
      real*8 AVIP,  A,                  G,T,P,                      POT,
     *
     *     EP0,                                                 WSAMP,
     *                       RNNO01,
     *                     RNNO02,
     *                                   EP,
     * REJF,                                                       ESG1,
     *                                      ESG2,
     *               aa,bb,cc,sinpsi,sindel,cosdel,us,vs,cphi,sphi
      integer*4
     *                     ibr
      real*8 xphi,xphi2,yphi,yphi2,rhophi2
      integer*4 ip
      NPold = NP
      IF (( nbr_split .LE. 0 )) THEN
        return
      END IF
      PAVIP=E(NP)+PRM
      AVIP=PAVIP
      A=AVIP/RM
      G=A-1.0
      T=G-1.0
      P=SQRT(A*T)
      POT=P/T
      EP0=1.0/(A+P)
      WSAMP=LOG((1.0-EP0)/EP0)
      aa = u(np)
      bb = v(np)
      cc = w(np)
      sinpsi = aa*aa + bb*bb
      IF (( sinpsi .GT. 1e-20 )) THEN
        sinpsi = sqrt(sinpsi)
        sindel = bb/sinpsi
        cosdel = aa/sinpsi
      END IF
      IF (( nbr_split .GT. 1 )) THEN
        wt(np) = wt(np)/nbr_split
      END IF
      DO 6211 ibr=1,nbr_split
        IF (( np+1 .GT. 50 )) THEN
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,'(//a,i6,a//)') ' Stack overflow in ANNIH! np = ',
     *    np+1, ' Increase $MXSTACK and try again'
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        END IF
6221    CONTINUE
          IF((rng_seed .GT. 128))call ranmar_get
          RNNO01 = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          EP=EP0*EXP(RNNO01*WSAMP)
          IF((rng_seed .GT. 128))call ranmar_get
          RNNO02 = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          REJF = 1 - (EP*A-1)**2/(EP*(A*A-2))
          IF(((RNNO02 .LE. REJF)))GO TO6222
        GO TO 6221
6222    CONTINUE
        ESG1=AVIP*EP
        PESG1=ESG1
        E(NP)=PESG1
        IQ(NP)=0
        IF (( ibr .EQ. 1 )) THEN
          ip = npold
        ELSE
          ip = np-1
        END IF
        X(np)=X(ip)
        Y(np)=Y(ip)
        Z(np)=Z(ip)
        IR(np)=IR(ip)
        WT(np)=WT(ip)
        DNEAR(np)=DNEAR(ip)
        LATCH(np)=LATCH(ip)
        COSTHE=MIN(1.0,(ESG1-RM)*POT/ESG1)
        SINTHE=SQRT(1.0-COSTHE*COSTHE)
6231    CONTINUE
          IF((rng_seed .GT. 128))call ranmar_get
          xphi = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          xphi = 2*xphi - 1
          xphi2 = xphi*xphi
          IF((rng_seed .GT. 128))call ranmar_get
          yphi = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          yphi2 = yphi*yphi
          rhophi2 = xphi2 + yphi2
          IF(rhophi2.LE.1)GO TO6232
        GO TO 6231
6232    CONTINUE
        rhophi2 = 1/rhophi2
        cphi = (xphi2 - yphi2)*rhophi2
        sphi = 2*xphi*yphi*rhophi2
        IF (( sinpsi .GE. 1e-10 )) THEN
          us = sinthe*cphi
          vs = sinthe*sphi
          u(np) = cc*cosdel*us - sindel*vs + aa*costhe
          v(np) = cc*sindel*us + cosdel*vs + bb*costhe
          w(np) = cc*costhe - sinpsi*us
        ELSE
          u(np) = sinthe*cphi
          v(np) = sinthe*sphi
          w(np) = cc*costhe
        END IF
        np = np + 1
        PESG2=PAVIP-PESG1
        esg2 = pesg2
        e(np) = pesg2
        iq(np) = 0
        X(np)=X(np-1)
        Y(np)=Y(np-1)
        Z(np)=Z(np-1)
        IR(np)=IR(np-1)
        WT(np)=WT(np-1)
        DNEAR(np)=DNEAR(np-1)
        LATCH(np)=LATCH(np-1)
        COSTHE=MIN(1.0,(ESG2-RM)*POT/ESG2)
        SINTHE=-SQRT(1.0-COSTHE*COSTHE)
        IF (( sinpsi .GE. 1e-10 )) THEN
          us = sinthe*cphi
          vs = sinthe*sphi
          u(np) = cc*cosdel*us - sindel*vs + aa*costhe
          v(np) = cc*sindel*us + cosdel*vs + bb*costhe
          w(np) = cc*costhe - sinpsi*us
        ELSE
          u(np) = sinthe*cphi
          v(np) = sinthe*sphi
          w(np) = cc*costhe
        END IF
        np = np + 1
6211  CONTINUE
6212  CONTINUE
      np = np-1
      RETURN
      END
      SUBROUTINE ANNIH_AT_REST
      implicit none
      COMMON/QDEBUG/QDEBUG
      LOGICAL QDEBUG
      COMMON/STACK/ E(50),X(50),Y(50),Z(50),U(50),V(50),W(50),DNEAR(50),
     *WT(50),IQ(50),IR(50),LATCH(50), LATCHI,NP,NPold
      DOUBLE PRECISION E
      real*8 X,Y,Z,  U,V,W,  DNEAR,  WT
      integer*4 IQ,  IR,  LATCH,  LATCHI, NP,  NPold
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      common/egs_vr/ e_max_rr(1),  prob_RR,  nbr_split,  i_play_RR,
     * i_survived_RR,
     *     n_RR_warning,                                        i_do_rr(
     *1)
      real*8          e_max_rr,prob_RR
      integer*4       nbr_split,i_play_RR,i_survived_RR,n_RR_warning
      integer*2     i_do_rr
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      real*8 costhe,sinthe,cphi,sphi
      integer*4 ibr,ip
      real*8 xphi,xphi2,yphi,yphi2,rhophi2
      NPold = NP
      IF (( np+2*nbr_split-1 .GT. 50 )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,'(//,3a,/,2(a,i9))') ' In subroutine ','ANNIH_AT_RES
     *T', ' stack size exceeded! ',' $MAXSTACK = ',50,' np = ',np+2*nbr_
     *  split-1
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      IF (( nbr_split .GT. 1 )) THEN
        wt(np) = wt(np)/nbr_split
      END IF
      DO 6241 ibr=1,nbr_split
        IF((rng_seed .GT. 128))call ranmar_get
        costhe = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        costhe = 2*costhe-1
        sinthe = sqrt(max(0.0,(1-costhe)*(1+costhe)))
6251    CONTINUE
          IF((rng_seed .GT. 128))call ranmar_get
          xphi = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          xphi = 2*xphi - 1
          xphi2 = xphi*xphi
          IF((rng_seed .GT. 128))call ranmar_get
          yphi = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          yphi2 = yphi*yphi
          rhophi2 = xphi2 + yphi2
          IF(rhophi2.LE.1)GO TO6252
        GO TO 6251
6252    CONTINUE
        rhophi2 = 1/rhophi2
        cphi = (xphi2 - yphi2)*rhophi2
        sphi = 2*xphi*yphi*rhophi2
        e(np) = prm
        iq(np) = 0
        IF (( ibr .EQ. 1 )) THEN
          ip = npold
        ELSE
          ip = np-1
        END IF
        X(np)=X(ip)
        Y(np)=Y(ip)
        Z(np)=Z(ip)
        IR(np)=IR(ip)
        WT(np)=WT(ip)
        DNEAR(np)=DNEAR(ip)
        LATCH(np)=LATCH(ip)
        u(np) = sinthe*cphi
        v(np) = sinthe*sphi
        w(np) = costhe
        np = np+1
        e(np) = prm
        iq(np) = 0
        X(np)=X(np-1)
        Y(np)=Y(np-1)
        Z(np)=Z(np-1)
        IR(np)=IR(np-1)
        WT(np)=WT(np-1)
        DNEAR(np)=DNEAR(np-1)
        LATCH(np)=LATCH(np-1)
        u(np) = -u(np-1)
        v(np) = -v(np-1)
        w(np) = -w(np-1)
        np = np+1
6241  CONTINUE
6242  CONTINUE
      np = np-1
      return
      end
      SUBROUTINE BHABHA
      implicit none
      COMMON/QDEBUG/QDEBUG
      LOGICAL QDEBUG
      COMMON/STACK/ E(50),X(50),Y(50),Z(50),U(50),V(50),W(50),DNEAR(50),
     *WT(50),IQ(50),IR(50),LATCH(50), LATCHI,NP,NPold
      DOUBLE PRECISION E
      real*8 X,Y,Z,  U,V,W,  DNEAR,  WT
      integer*4 IQ,  IR,  LATCH,  LATCHI, NP,  NPold
      COMMON/THRESH/RMT2,RMSQ, AP(1),AE(1),UP(1),UE(1),TE(1),THMOLL(1)
      real*8 RMT2,  RMSQ,  AP,  AE,  UP,  UE,  TE,  THMOLL
      COMMON/UPHIOT/THETA,SINTHE,COSTHE,SINPHI, COSPHI,PI,TWOPI,PI5D2
      real*8 THETA,  SINTHE,  COSTHE,  SINPHI,  COSPHI,  PI,TWOPI,PI5D2
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      common/egs_vr/ e_max_rr(1),  prob_RR,  nbr_split,  i_play_RR,
     * i_survived_RR,
     *     n_RR_warning,                                        i_do_rr(
     *1)
      real*8          e_max_rr,prob_RR
      integer*4       nbr_split,i_play_RR,i_survived_RR,n_RR_warning
      integer*2     i_do_rr
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      DOUBLE PRECISION PEIP,  PEKIN,  PEKSE2,  PESE1,  PESE2,  H1,  DCOS
     *TH
      real*8 EIP,  EKIN,  T0,  E0,  E02,  YY,  Y2,YP,YP2, BETA2,  EP0,
     *EP0C,  B1,B2,B3,B4,  RNNO03,RNNO04, BR,  REJF2,  ESE1,  ESE2
      NPold = NP
      PEIP=E(NP)
      EIP=PEIP
      PEKIN=PEIP-PRM
      EKIN=PEKIN
      T0=EKIN/RM
      E0=T0+1.
      YY=1./(T0+2.)
      E02=E0*E0
      BETA2=(E02-1.)/E02
      EP0=TE(MEDIUM)/EKIN
      EP0C=1.-EP0
      Y2=YY*YY
      YP=1.-2.*YY
      YP2=YP*YP
      B4=YP2*YP
      B3=B4+YP2
      B2=YP*(3.+Y2)
      B1=2.-Y2
6261  CONTINUE
        IF((rng_seed .GT. 128))call ranmar_get
        RNNO03 = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        BR=EP0/(1.-EP0C*RNNO03)
        IF((rng_seed .GT. 128))call ranmar_get
        RNNO04 = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        REJF2=(1.0-BETA2*BR*(B1-BR*(B2-BR*(B3-BR*B4))))
        IF((RNNO04.LE.REJF2))GO TO6262
      GO TO 6261
6262  CONTINUE
      IF (( np+1 .GT. 50 )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,'(//,3a,/,2(a,i9))') ' In subroutine ','BHABHA', ' s
     *tack size exceeded! ',' $MAXSTACK = ',50,' np = ',np+1
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      IF ((BR.LT.0.5)) THEN
        IQ(NP+1)=-1
      ELSE
        IQ(NP)=-1
        IQ(NP+1)=1
        BR=1.-BR
      END IF
      BR=max(BR,0.0)
      PEKSE2=BR*EKIN
      PESE1=PEIP-PEKSE2
      PESE2=PEKSE2+PRM
      ESE1=PESE1
      ESE2=PESE2
      E(NP)=PESE1
      E(NP+1)=PESE2
      H1=(PEIP+PRM)/PEKIN
      DCOSTH=MIN(1.0D0,H1*(PESE1-PRM)/(PESE1+PRM))
      SINTHE=DSQRT(1.D0-DCOSTH)
      COSTHE=DSQRT(DCOSTH)
      CALL UPHI(2,1)
      NP=NP+1
      DCOSTH=H1*(PESE2-PRM)/(PESE2+PRM)
      SINTHE=-DSQRT(1.D0-DCOSTH)
      COSTHE=DSQRT(DCOSTH)
      CALL UPHI(3,2)
      RETURN
      END
      SUBROUTINE BREMS
      implicit none
      COMMON/QDEBUG/QDEBUG
      LOGICAL QDEBUG
      COMMON/BREMPR/ DL1(8,1),DL2(8,1),DL3(8,1),DL4(8,1),DL5(8,1),DL6(8,
     *1), ALPHI(2,1),BPAR(2,1),DELPOS(2,1), WA(1,50),PZ(1,50),ZELEM(1,50
     *),RHOZ(1,50), PWR2I(50), DELCM(1),ZBRANG(1),LZBRANG(1),NNE(1), IBR
     *DST,IPRDST,ibr_nist,pair_nrc,itriplet, ASYM(1,50,2)
      CHARACTER*4 ASYM
      real*8 DL1,DL2,DL3,DL4,DL5,DL6,   ALPHI,  BPAR,  DELPOS,  WA,  PZ,
     *  ZELEM,  RHOZ,  PWR2I,  DELCM,  ZBRANG,  LZBRANG
      integer*4 NNE,  IBRDST,  IPRDST,  ibr_nist,  itriplet,  pair_nrc
      COMMON/EPCONT/EDEP,EDEP_LOCAL,TSTEP,TUSTEP,USTEP,TVSTEP,VSTEP, RHO
     *F,EOLD,ENEW,EKE,ELKE,GLE,E_RANGE, x_final,y_final,z_final, u_final
     *,v_final,w_final, IDISC,IROLD,IRNEW,IAUSFL(35)
      DOUBLE PRECISION EDEP,  EDEP_LOCAL
      real*8 TSTEP,  TUSTEP,  USTEP,  VSTEP,  TVSTEP,  RHOF,  EOLD,  ENE
     *W,  EKE,  ELKE,  GLE,  E_RANGE, x_final,y_final,z_final,  u_final,
     *v_final,w_final
      integer*4 IDISC,  IROLD,  IRNEW,  IAUSFL
      common/nist_brems/ nb_fdata(0:50,100,1), nb_xdata(0:50,100,1), nb_
     *wdata(50,100,1), nb_idata(50,100,1), nb_emin(1),nb_emax(1), nb_lem
     *in(1),nb_lemax(1), nb_dle(1),nb_dlei(1), log_ap(1)
      real*8 nb_fdata,nb_xdata,nb_wdata,nb_emin,nb_emax,nb_lemin,nb_lema
     *x, nb_dle,nb_dlei,log_ap
      integer*4 nb_idata
      COMMON/STACK/ E(50),X(50),Y(50),Z(50),U(50),V(50),W(50),DNEAR(50),
     *WT(50),IQ(50),IR(50),LATCH(50), LATCHI,NP,NPold
      DOUBLE PRECISION E
      real*8 X,Y,Z,  U,V,W,  DNEAR,  WT
      integer*4 IQ,  IR,  LATCH,  LATCHI, NP,  NPold
      COMMON/THRESH/RMT2,RMSQ, AP(1),AE(1),UP(1),UE(1),TE(1),THMOLL(1)
      real*8 RMT2,  RMSQ,  AP,  AE,  UP,  UE,  TE,  THMOLL
      COMMON/UPHIOT/THETA,SINTHE,COSTHE,SINPHI, COSPHI,PI,TWOPI,PI5D2
      real*8 THETA,  SINTHE,  COSTHE,  SINPHI,  COSPHI,  PI,TWOPI,PI5D2
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      common/egs_vr/ e_max_rr(1),  prob_RR,  nbr_split,  i_play_RR,
     * i_survived_RR,
     *     n_RR_warning,                                        i_do_rr(
     *1)
      real*8          e_max_rr,prob_RR
      integer*4       nbr_split,i_play_RR,i_survived_RR,n_RR_warning
      integer*2     i_do_rr
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      DOUBLE PRECISION PEIE,  PESG,  PESE
      real*8 EIE,  EKIN,  brmin,  waux,  aux,  r1,  ajj,  alias_sample1,
     * RNNO06,  RNNO07,  BR,  ESG,  ESE,  DELTA,  phi1,  phi2,  REJF
      real*8 a,b,c,                               sinpsi, sindel, cosdel
     *, us, vs,
     *                                                ztarg,
     *             tteie,                                    beta,
     *                       y2max,
     *      y2maxi,                                                   tt
     *ese,                                      rjarg1,rjarg2,rjarg3,rej
     *min,rejmid,rejmax,rejtop,rejtst,
     *                 esedei,                                 y2tst,
     *                             y2tst1,
     *                                           rtest,
     *                            xphi,yphi,xphi2,yphi2,rhophi2,cphi,sph
     *i
      integer*4
     *                 L,L1,ibr,jj,j
      real*8 z2max,z2maxi,aux1,aux3,aux4,aux5,aux2,weight
      IF((nbr_split .LT. 1))return
      NPold = NP
      PEIE=E(NP)
      EIE=PEIE
      weight = wt(np)/nbr_split
      IF ((EIE.LT.50.0)) THEN
        L=1
      ELSE
        L=3
      END IF
      L1 = L+1
      ekin = peie-prm
      brmin = ap(medium)/ekin
      waux = elke - log_ap(medium)
      IF (( ibrdst .GE. 0 )) THEN
        a = u(np)
        b = v(np)
        c = w(np)
        sinpsi = a*a + b*b
        IF (( sinpsi .GT. 1e-20 )) THEN
          sinpsi = sqrt(sinpsi)
          sindel = b/sinpsi
          cosdel = a/sinpsi
        END IF
        ztarg = zbrang(medium)
        tteie = eie/rm
        beta = sqrt((tteie-1)*(tteie+1))/tteie
        y2max = 2*beta*(1+beta)*tteie*tteie
        y2maxi = 1/y2max
        IF (( ibrdst .EQ. 1 )) THEN
          z2max = y2max+1
          z2maxi = sqrt(z2max)
        END IF
      END IF
      IF (( ibr_nist .GE. 1 )) THEN
        ajj = 1 + (waux + log_ap(medium) - nb_lemin(medium))*nb_dlei(med
     *  ium)
        jj = ajj
        ajj = ajj - jj
        IF (( jj .GT. 100 )) THEN
          jj = 100
          ajj = -1
        END IF
      END IF
      DO 6271 ibr=1,nbr_split
        IF (( ibr_nist .GE. 1 )) THEN
          IF (( ekin .GT. nb_emin(medium) )) THEN
            IF((rng_seed .GT. 128))call ranmar_get
            r1 = rng_array(rng_seed)*twom24
            rng_seed = rng_seed + 1
            IF (( r1 .LT. ajj )) THEN
              j = jj+1
            ELSE
              j = jj
            END IF
            br = alias_sample1(50,nb_xdata(0,j,medium), nb_fdata(0,j,med
     *      ium), nb_wdata(1,j,medium),nb_idata(1,j,medium))
          ELSE
            IF((rng_seed .GT. 128))call ranmar_get
            br = rng_array(rng_seed)*twom24
            rng_seed = rng_seed + 1
          END IF
          esg = ap(medium)*exp(br*waux)
          pesg = esg
          pese = peie - pesg
          ese = pese
        ELSE
6281      CONTINUE
            IF((rng_seed .GT. 128))call ranmar_get
            rnno06 = rng_array(rng_seed)*twom24
            rng_seed = rng_seed + 1
            IF((rng_seed .GT. 128))call ranmar_get
            rnno07 = rng_array(rng_seed)*twom24
            rng_seed = rng_seed + 1
            br = brmin*exp(rnno06*waux)
            esg = ekin*br
            pesg = esg
            pese = peie - pesg
            ese = pese
            delta = esg/eie/ese*delcm(medium)
            aux = ese/eie
            IF (( delta .LT. 1 )) THEN
              phi1 = dl1(l,medium)+delta*(dl2(l,medium)+delta*dl3(l,medi
     *        um))
              phi2 = dl1(l1,medium)+delta*(dl2(l1,medium)+ delta*dl3(l1,
     *        medium))
            ELSE
              phi1 = dl4(l,medium)+dl5(l,medium)*log(delta+dl6(l,medium)
     *        )
              phi2 = phi1
            END IF
            rejf = (1+aux*aux)*phi1 - 2*aux*phi2/3
            IF(((rnno07 .LT. rejf)))GO TO6282
          GO TO 6281
6282      CONTINUE
        END IF
        np=np+1
        IF (( np .GT. 50 )) THEN
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,'(//a,i6,a//)') ' Stack overflow in BREMS! np = ',
     *    np+1, ' Increase $MXSTACK and try again'
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        END IF
        e(np) = pesg
        iq(np) = 0
        X(np)=X(np-1)
        Y(np)=Y(np-1)
        Z(np)=Z(np-1)
        IR(np)=IR(np-1)
        WT(np)=WT(np-1)
        DNEAR(np)=DNEAR(np-1)
        LATCH(np)=LATCH(np-1)
        wt(np) = weight
        IF (( ibrdst .LT. 0 )) THEN
          u(np) = u(npold)
          v(np) = v(npold)
          w(np) = w(npold)
        ELSE
          IF (( ibrdst .EQ. 1 )) THEN
            ttese = ese/rm
            esedei = ttese/tteie
            rjarg1 = 1+esedei*esedei
            rjarg2 = rjarg1 + 2*esedei
            aux = 2*ese*tteie/esg
            aux = aux*aux
            aux1 = aux*ztarg
            IF (( aux1 .GT. 10 )) THEN
              rjarg3 = lzbrang(medium) + (1-aux1)/aux1**2
            ELSE
              rjarg3 = log(aux/(1+aux1))
            END IF
            rejmax = rjarg1*rjarg3-rjarg2
6291        CONTINUE
              IF((rng_seed .GT. 128))call ranmar_get
              y2tst = rng_array(rng_seed)*twom24
              rng_seed = rng_seed + 1
              IF((rng_seed .GT. 128))call ranmar_get
              rtest = rng_array(rng_seed)*twom24
              rng_seed = rng_seed + 1
              aux3 = z2maxi/(y2tst+(1-y2tst)*z2maxi)
              rtest = rtest*aux3*rejmax
              y2tst = aux3**2-1
              y2tst1 = esedei*y2tst/aux3**4
              aux4 = 16*y2tst1-rjarg2
              aux5 = rjarg1-4*y2tst1
              IF((rtest .LT. aux4 + aux5*rjarg3))GO TO6292
              aux2 = log(aux/(1+aux1/aux3**4))
              rejtst = aux4+aux5*aux2
              IF(((rtest .LT. rejtst )))GO TO6292
            GO TO 6291
6292        CONTINUE
          ELSE
            IF((rng_seed .GT. 128))call ranmar_get
            y2tst = rng_array(rng_seed)*twom24
            rng_seed = rng_seed + 1
            y2tst = y2tst/(1-y2tst+y2maxi)
          END IF
          costhe = 1 - 2*y2tst*y2maxi
          sinthe = sqrt(max((1-costhe)*(1+costhe),0.0))
6301      CONTINUE
            IF((rng_seed .GT. 128))call ranmar_get
            xphi = rng_array(rng_seed)*twom24
            rng_seed = rng_seed + 1
            xphi = 2*xphi - 1
            xphi2 = xphi*xphi
            IF((rng_seed .GT. 128))call ranmar_get
            yphi = rng_array(rng_seed)*twom24
            rng_seed = rng_seed + 1
            yphi2 = yphi*yphi
            rhophi2 = xphi2 + yphi2
            IF(rhophi2.LE.1)GO TO6302
          GO TO 6301
6302      CONTINUE
          rhophi2 = 1/rhophi2
          cphi = (xphi2 - yphi2)*rhophi2
          sphi = 2*xphi*yphi*rhophi2
          IF (( sinpsi .GE. 1e-10 )) THEN
            us = sinthe*cphi
            vs = sinthe*sphi
            u(np) = c*cosdel*us - sindel*vs + a*costhe
            v(np) = c*sindel*us + cosdel*vs + b*costhe
            w(np) = c*costhe - sinpsi*us
          ELSE
            u(np) = sinthe*cphi
            v(np) = sinthe*sphi
            w(np) = c*costhe
          END IF
        END IF
6271  CONTINUE
6272  CONTINUE
      e(npold) = pese
      RETURN
      END
      SUBROUTINE COMPT
      implicit none
      common/compton_data/ iz_array(1538),  be_array(1538),  Jo_array(15
     *38),  erfJo_array(1538),   ne_array(1538),  shn_array(1538),
     *shell_array(200,1), eno_array(200,1), eno_atbin_array(200,1), n_sh
     *ell(1), radc_flag,  ibcmp(1)
      integer*4 iz_array,ne_array,shn_array,eno_atbin_array, shell_array
     *,n_shell,radc_flag
      real*8 be_array,Jo_array,erfJo_array,eno_array
      integer*2 ibcmp
      COMMON/QDEBUG/QDEBUG
      LOGICAL QDEBUG
      COMMON/EPCONT/EDEP,EDEP_LOCAL,TSTEP,TUSTEP,USTEP,TVSTEP,VSTEP, RHO
     *F,EOLD,ENEW,EKE,ELKE,GLE,E_RANGE, x_final,y_final,z_final, u_final
     *,v_final,w_final, IDISC,IROLD,IRNEW,IAUSFL(35)
      DOUBLE PRECISION EDEP,  EDEP_LOCAL
      real*8 TSTEP,  TUSTEP,  USTEP,  VSTEP,  TVSTEP,  RHOF,  EOLD,  ENE
     *W,  EKE,  ELKE,  GLE,  E_RANGE, x_final,y_final,z_final,  u_final,
     *v_final,w_final
      integer*4 IDISC,  IROLD,  IRNEW,  IAUSFL
      COMMON/STACK/ E(50),X(50),Y(50),Z(50),U(50),V(50),W(50),DNEAR(50),
     *WT(50),IQ(50),IR(50),LATCH(50), LATCHI,NP,NPold
      DOUBLE PRECISION E
      real*8 X,Y,Z,  U,V,W,  DNEAR,  WT
      integer*4 IQ,  IR,  LATCH,  LATCHI, NP,  NPold
      COMMON/THRESH/RMT2,RMSQ, AP(1),AE(1),UP(1),UE(1),TE(1),THMOLL(1)
      real*8 RMT2,  RMSQ,  AP,  AE,  UP,  UE,  TE,  THMOLL
      COMMON/UPHIOT/THETA,SINTHE,COSTHE,SINPHI, COSPHI,PI,TWOPI,PI5D2
      real*8 THETA,  SINTHE,  COSTHE,  SINPHI,  COSPHI,  PI,TWOPI,PI5D2
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      common/egs_vr/ e_max_rr(1),  prob_RR,  nbr_split,  i_play_RR,
     * i_survived_RR,
     *     n_RR_warning,                                        i_do_rr(
     *1)
      real*8          e_max_rr,prob_RR
      integer*4       nbr_split,i_play_RR,i_survived_RR,n_RR_warning
      integer*2     i_do_rr
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      common/relax_data/ relax_first(3000),  relax_ntran(3000),  relax_s
     *tate(10000),  relax_prob(10000),  relax_atbin(10000),  relax_ntot
      real*8 relax_prob
      integer*4 relax_first, relax_ntran, relax_state, relax_atbin, rela
     *x_ntot
      DOUBLE PRECISION PEIG,  PESG,  PESE
      real*8 ko,  broi,  broi2,  bro,  bro1,  alph1,  alph2,  alpha,  rn
     *no15,rnno16,rnno17,rnno18,rnno19,  br,  temp,  rejf3,  rejmax,  Uj
     *,  Jo,  br2,  fpz,fpz1, qc,  qc2,  af,  Fmax,  frej,  eta_incoh, e
     *ta,  aux,aux1,aux2,aux3,aux4,  pzmax,  pz,  pz2,  rnno_RR
      integer*4 irl,  i,  j,  iarg,  ip
      common/rad_compton/ radc_sigs(0:128),radc_sigd(0:128), radc_frej(0
     *:128,0:32), radc_x(8929), radc_fdat(13917), radc_Smax(13917), radc
     *_emin, radc_emax,radc_dw, radc_dle, radc_dlei, radc_le1, radc_bins
     *(13917), radc_ixmin1(13917),radc_ixmax1(13917), radc_ixmin2(13917)
     *,radc_ixmax2(13917), radc_ixmin3(13917),radc_ixmax3(13917), radc_i
     *xmin4(13917),radc_ixmax4(13917), radc_startx(0:128),radc_startb(0:
     *128)
      real*8 radc_sigs,  radc_sigd,  radc_frej,  radc_fdat,  radc_Smax,
     * radc_emin,  radc_emax,  radc_lemin, radc_dw,  radc_dle,  radc_dle
     *i,  radc_x,  radc_le1
      integer*2 radc_bins,  radc_ixmin1,radc_ixmax1, radc_ixmin2,radc_ix
     *max2, radc_ixmin3,radc_ixmax3, radc_ixmin4,radc_ixmax4, radc_start
     *x,radc_startb
      real*8 acheck,acheck1,sig_sc,sig_dc,beta_rad,alpha_rad,ux,au
      integer*4 icheck,iu
      logical first_time
      integer*4 ibcmpl
      NPold = NP
      peig=E(NP)
      ko = peig/rm
      broi = 1 + 2*ko
      IF (( radc_flag .EQ. 1 .AND. ko .GT. radc_emin .AND. ko .LT. radc_
     *emax )) THEN
        acheck = radc_dlei*gle + radc_le1
        icheck = acheck
        acheck = acheck - icheck
        acheck1 = 1 - acheck
        sig_sc = radc_sigs(icheck)*acheck1 + radc_sigs(icheck+1)*acheck
        sig_dc = radc_sigd(icheck)*acheck1 + radc_sigd(icheck+1)*acheck
        IF((rng_seed .GT. 128))call ranmar_get
        rnno15 = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        IF((rng_seed .GT. 128))call ranmar_get
        rnno16 = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        IF((rnno16 .LT. acheck))icheck = icheck+1
        IF (( rnno15*(sig_sc + sig_dc) .LT. sig_dc )) THEN
          call sample_double_compton(ko,icheck)
          return
        END IF
        beta_rad = ko*(1+ko)
        alpha_rad = 1/log(1 + 2*beta_rad)
      END IF
      irl = ir(np)
      first_time = .true.
      ibcmpl = ibcmp(irl)
6310  CONTINUE
      IF (( ibcmpl .GT. 0 )) THEN
        IF((rng_seed .GT. 128))call ranmar_get
        rnno17 = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        rnno17 = 1 + rnno17*n_shell(medium)
        i = int(rnno17)
        IF((rnno17 .GT. eno_array(i,medium)))i = eno_atbin_array(i,mediu
     *  m)
        j = shell_array(i,medium)
        Uj = be_array(j)
        IF (( ko .LE. Uj )) THEN
          IF (( ibcmpl .EQ. 1 )) THEN
            goto 6320
          ELSE
            goto 6310
          END IF
        END IF
        Jo = Jo_array(j)
      END IF
6330  CONTINUE
      IF (( ko .GT. 2 )) THEN
        IF (( first_time )) THEN
          broi2 = broi*broi
          alph1 = Log(broi)
          bro = 1/broi
          alph2 = ko*(broi+1)*bro*bro
          alpha = alph1+alph2
        END IF
6341    CONTINUE
          IF((rng_seed .GT. 128))call ranmar_get
          rnno15 = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          IF((rng_seed .GT. 128))call ranmar_get
          rnno16 = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          IF (( rnno15*alpha .LT. alph1 )) THEN
            br = Exp(alph1*rnno16)*bro
          ELSE
            br = Sqrt(rnno16*broi2 + (1-rnno16))*bro
          END IF
          temp = (1-br)/(ko*br)
          sinthe = Max(0.,temp*(2-temp))
          aux = 1+br*br
          rejf3 = aux - br*sinthe
          IF((rng_seed .GT. 128))call ranmar_get
          rnno19 = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          IF((rnno19*aux.le.rejf3))GO TO6342
        GO TO 6341
6342    CONTINUE
      ELSE
        IF (( first_time )) THEN
          bro = 1./broi
          bro1 = 1 - bro
          rejmax = broi + bro
        END IF
6351    CONTINUE
          IF((rng_seed .GT. 128))call ranmar_get
          rnno15 = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          IF((rng_seed .GT. 128))call ranmar_get
          rnno16 = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          br = bro + bro1*rnno15
          temp = (1-br)/(ko*br)
          sinthe = Max(0.,temp*(2-temp))
          rejf3 = 1 + br*br - br*sinthe
          IF((rnno16*br*rejmax.le.rejf3))GO TO6352
        GO TO 6351
6352    CONTINUE
      END IF
      first_time = .false.
      IF ((br .LT. bro .OR. br .GT. 1)) THEN
        IF (( br .LT. 0.99999/broi .OR. br .GT. 1.00001 )) THEN
          write(i_log,'(/a)') '***************** Warning: '
          write(i_log,*) ' sampled br outside of allowed range! ',ko,1./
     *    broi,br
        END IF
        goto 6330
      END IF
      IF (( radc_flag .EQ. 1 .AND. ko .GT. radc_emin .AND. ko .LT. radc_
     *emax )) THEN
        ux = log(1 + beta_rad*temp)*alpha_rad
        au = ux*32
        iu = au
        au = au - iu
        rejf3 = radc_frej(icheck,iu)*(1-au) + radc_frej(icheck,iu+1)*au
        IF((rng_seed .GT. 128))call ranmar_get
        rnno15 = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        IF((rnno15 .GT. rejf3))goto 6330
      END IF
      costhe = 1 - temp
      IF (( ibcmp(irl) .EQ. 0 )) THEN
        Uj = 0
        goto 6360
      END IF
      br2 = br*br
      aux = ko*(ko-Uj)*temp
      aux1 = 2*aux + Uj*Uj
      pzmax = aux - Uj
      IF (( pzmax .LT. 0 .AND. pzmax*pzmax .GE. aux1 )) THEN
        IF (( ibcmpl .EQ. 1 )) THEN
          goto 6320
        ELSE
          goto 6310
        END IF
      END IF
      pzmax = pzmax/sqrt(aux1)
      qc2 = 1 + br*br - 2*br*costhe
      qc = sqrt(qc2)
      IF (( pzmax .GT. 1 )) THEN
        pzmax = 1
        af = 0
        Fmax = 1
        fpz = 1
        goto 6370
      END IF
      aux3 = 1 + 2*Jo*abs(pzmax)
      aux4 = 0.5*(1-aux3*aux3)
      fpz = 0.5*exp(aux4)
      af = qc*(1+br*(br-costhe)/qc2)
      IF (( af .LT. 0 )) THEN
        IF((pzmax .GT. 0))fpz = 1 - fpz
        IF((rng_seed .GT. 128))call ranmar_get
        eta_incoh = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        IF (( eta_incoh .GT. fpz )) THEN
          IF (( ibcmpl .EQ. 1 )) THEN
            goto 6320
          ELSE
            goto 6310
          END IF
        END IF
        af = 0
        Fmax = 1
        goto 6370
      END IF
      IF (( pzmax .LT. -0.15 )) THEN
        Fmax = 1-af*0.15
        fpz1 = fpz*Fmax*Jo
      ELSE IF(( pzmax .LT. 0.15 )) THEN
        Fmax = 1 + af*pzmax
        aux3 = 1/(1+0.33267252734*aux3)
        aux4 = fpz*aux3*(0.3480242+aux3*(-0.0958798+aux3*0.7478556)) + e
     *  rfJo_array(j)
        IF (( pzmax .GT. 0 )) THEN
          fpz1 = (1 - Fmax*fpz)*Jo - 0.62665706866*af*aux4
          fpz = 1 - fpz
        ELSE
          fpz1 = Fmax*fpz*Jo - 0.62665706866*af*aux4
        END IF
      ELSE
        Fmax = 1 + af*0.15
        fpz1 = (1 - Fmax*fpz)*Jo
        fpz = 1 - fpz
      END IF
      IF((rng_seed .GT. 128))call ranmar_get
      eta_incoh = rng_array(rng_seed)*twom24
      rng_seed = rng_seed + 1
      IF ((eta_incoh*Jo .GT. fpz1 )) THEN
        IF (( ibcmpl .EQ. 1 )) THEN
          goto 6320
        ELSE
          goto 6310
        END IF
      END IF
6370  CONTINUE
      IF (( ibcmpl .NE. 2 )) THEN
        IF((rng_seed .GT. 128))call ranmar_get
        rnno18 = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        rnno18 = rnno18*fpz
        IF (( rnno18 .LT. 0.5 )) THEN
          rnno18 = Max(1e-30,2*rnno18)
          pz = 0.5*(1-Sqrt(1-2*Log(rnno18)))/Jo
        ELSE
          rnno18 = 2*(1-rnno18)
          pz = 0.5*(Sqrt(1-2*Log(rnno18))-1)/Jo
        END IF
        IF((abs(pz) .GT. 1))goto 6370
        IF (( pz .LT. 0.15 )) THEN
          IF (( pz .LT. -0.15 )) THEN
            frej = 1 - af*0.15
          ELSE
            frej = 1 + af*pz
          END IF
          IF((rng_seed .GT. 128))call ranmar_get
          eta = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          IF((eta*Fmax .GT. frej))goto 6370
        END IF
      ELSE
        pz = 0
        Uj = 0
      END IF
      pz2 = pz*pz
      IF (( abs(pz) .LT. 0.01 )) THEN
        br = br*(1 + pz*(qc + (br2-costhe)*pz))
      ELSE
        aux = 1 - pz2*br*costhe
        aux1 = 1 - pz2*br2
        aux2 = qc2 - br2*pz2*sinthe
        IF (( aux2 .GT. 1e-10 )) THEN
          br = br/aux1*(aux+pz*Sqrt(aux2))
        END IF
      END IF
      Uj = Uj*prm
6360  pesg = br*peig
      pese = peig - pesg - Uj + prm
      sinthe = Sqrt(sinthe)
      call uphi(2,1)
      e(np) = pesg
      aux = 1 + br*br - 2*br*costhe
      IF (( aux .GT. 1e-8 )) THEN
        costhe = (1-br*costhe)/Sqrt(aux)
        sinthe = (1-costhe)*(1+costhe)
        IF (( sinthe .GT. 0 )) THEN
          sinthe = -Sqrt(sinthe)
        ELSE
          sinthe = 0
        END IF
      ELSE
        costhe = 0
        sinthe = -1
      END IF
      np = np + 1
      IF (( np .GT. 50 )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,'(//,3a,/,2(a,i9))') ' In subroutine ','COMPT', ' st
     *ack size exceeded! ',' $MAXSTACK = ',50,' np = ',np
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      call uphi(3,2)
      e(np) = pese
      iq(np) = -1
      IF (( ibcmpl .EQ. 1 .OR. ibcmpl .EQ. 3 )) THEN
        IF (( Uj .GT. 1e-3 )) THEN
          edep = pzero
          call relax(Uj,shn_array(j),iz_array(j))
        ELSE
          edep = Uj
          edep_local = edep
          IARG=33
          IF ((IAUSFL(IARG+1).NE.0)) THEN
            CALL AUSGAB(IARG)
          END IF
        END IF
        IF (( edep .GT. 0 )) THEN
          IARG=4
          IF ((IAUSFL(IARG+1).NE.0)) THEN
            CALL AUSGAB(IARG)
          END IF
        END IF
      END IF
      i_survived_RR = 0
      IF (( i_play_RR .EQ. 1 )) THEN
        IF (( prob_RR .LE. 0 )) THEN
          IF (( n_RR_warning .LT. 50 )) THEN
            n_RR_warning = n_RR_warning + 1
            WRITE(6,6380)prob_RR
6380        FORMAT('**** Warning, attempt to play Roussian Roulette with
     * prob_RR<=0! ',g14.6)
          END IF
        ELSE
          ip = NPold+1
6391      CONTINUE
            IF (( iq(ip) .NE. 0 )) THEN
              IF((rng_seed .GT. 128))call ranmar_get
              rnno_RR = rng_array(rng_seed)*twom24
              rng_seed = rng_seed + 1
              IF (( rnno_RR .LT. prob_RR )) THEN
                wt(ip) = wt(ip)/prob_RR
                ip = ip + 1
              ELSE
                i_survived_RR = i_survived_RR + 1
                IF ((ip .LT. np)) THEN
                  e(ip) = e(np)
                  iq(ip) = iq(np)
                  wt(ip) = wt(np)
                  u(ip) = u(np)
                  v(ip) = v(np)
                  w(ip) = w(np)
                END IF
                np = np-1
              END IF
            ELSE
              ip = ip+1
            END IF
            IF(((ip .GT. np)))GO TO6392
          GO TO 6391
6392      CONTINUE
          IF (( np .EQ. 0 )) THEN
            np = 1
            e(np) = 0
            iq(np) = 0
            wt(np) = 0
          END IF
        END IF
      END IF
      return
6320  return
      end
      SUBROUTINE old_COMPT
      implicit none
      common/compton_data/ iz_array(1538),  be_array(1538),  Jo_array(15
     *38),  erfJo_array(1538),   ne_array(1538),  shn_array(1538),
     *shell_array(200,1), eno_array(200,1), eno_atbin_array(200,1), n_sh
     *ell(1), radc_flag,  ibcmp(1)
      integer*4 iz_array,ne_array,shn_array,eno_atbin_array, shell_array
     *,n_shell,radc_flag
      real*8 be_array,Jo_array,erfJo_array,eno_array
      integer*2 ibcmp
      COMMON/QDEBUG/QDEBUG
      LOGICAL QDEBUG
      COMMON/EPCONT/EDEP,EDEP_LOCAL,TSTEP,TUSTEP,USTEP,TVSTEP,VSTEP, RHO
     *F,EOLD,ENEW,EKE,ELKE,GLE,E_RANGE, x_final,y_final,z_final, u_final
     *,v_final,w_final, IDISC,IROLD,IRNEW,IAUSFL(35)
      DOUBLE PRECISION EDEP,  EDEP_LOCAL
      real*8 TSTEP,  TUSTEP,  USTEP,  VSTEP,  TVSTEP,  RHOF,  EOLD,  ENE
     *W,  EKE,  ELKE,  GLE,  E_RANGE, x_final,y_final,z_final,  u_final,
     *v_final,w_final
      integer*4 IDISC,  IROLD,  IRNEW,  IAUSFL
      COMMON/STACK/ E(50),X(50),Y(50),Z(50),U(50),V(50),W(50),DNEAR(50),
     *WT(50),IQ(50),IR(50),LATCH(50), LATCHI,NP,NPold
      DOUBLE PRECISION E
      real*8 X,Y,Z,  U,V,W,  DNEAR,  WT
      integer*4 IQ,  IR,  LATCH,  LATCHI, NP,  NPold
      COMMON/THRESH/RMT2,RMSQ, AP(1),AE(1),UP(1),UE(1),TE(1),THMOLL(1)
      real*8 RMT2,  RMSQ,  AP,  AE,  UP,  UE,  TE,  THMOLL
      COMMON/UPHIOT/THETA,SINTHE,COSTHE,SINPHI, COSPHI,PI,TWOPI,PI5D2
      real*8 THETA,  SINTHE,  COSTHE,  SINPHI,  COSPHI,  PI,TWOPI,PI5D2
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      common/egs_vr/ e_max_rr(1),  prob_RR,  nbr_split,  i_play_RR,
     * i_survived_RR,
     *     n_RR_warning,                                        i_do_rr(
     *1)
      real*8          e_max_rr,prob_RR
      integer*4       nbr_split,i_play_RR,i_survived_RR,n_RR_warning
      integer*2     i_do_rr
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      common/relax_data/ relax_first(3000),  relax_ntran(3000),  relax_s
     *tate(10000),  relax_prob(10000),  relax_atbin(10000),  relax_ntot
      real*8 relax_prob
      integer*4 relax_first, relax_ntran, relax_state, relax_atbin, rela
     *x_ntot
      DOUBLE PRECISION PEIG,  PESG,  PESE
      real*8 ko,  broi,  broi2,  bro,  bro1,  alph1,  alph2,  alpha,  rn
     *no15,rnno16,rnno17,rnno18,rnno19,  br,  temp,  rejf3,  rejmax,  Uj
     *,  br2,  aux,aux1,aux2, pzmax2,  pz,  pz2,  rnno_RR
      integer*4 irl,  i,  j,  iarg,  ip
      i_survived_RR = 0
      NPold = NP
      peig=E(NP)
      ko = peig/rm
      broi = 1 + 2*ko
      irl = ir(np)
      IF (( ibcmp(irl) .EQ. 1 )) THEN
        IF((rng_seed .GT. 128))call ranmar_get
        rnno17 = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        DO 6401 i=1,n_shell(medium)
          rnno17 = rnno17 - eno_array(i,medium)
          IF((rnno17 .LE. 0))GO TO6402
6401    CONTINUE
6402    CONTINUE
        j = shell_array(i,medium)
        Uj = be_array(j)
        IF (( ko .LE. Uj )) THEN
          goto 6410
        END IF
      END IF
6420  CONTINUE
      IF (( ko .GT. 2 )) THEN
        broi2 = broi*broi
        alph1 = Log(broi)
        alph2 = ko*(broi+1)/broi2
        alpha = alph1/(alph1+alph2)
6431    CONTINUE
          IF((rng_seed .GT. 128))call ranmar_get
          rnno15 = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          IF((rng_seed .GT. 128))call ranmar_get
          rnno16 = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          IF (( rnno15 .LT. alpha )) THEN
            br = Exp(alph1*rnno16)/broi
          ELSE
            br = Sqrt(rnno16 + (1-rnno16)/broi2)
          END IF
          temp = (1-br)/ko/br
          sinthe = Max(0.,temp*(2-temp))
          rejf3 = 1 - br*sinthe/(1+br*br)
          IF((rng_seed .GT. 128))call ranmar_get
          rnno19 = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          IF((rnno19.le.rejf3))GO TO6432
        GO TO 6431
6432    CONTINUE
      ELSE
        bro = 1./broi
        bro1 = 1 - bro
        rejmax = broi + bro
6441    CONTINUE
          IF((rng_seed .GT. 128))call ranmar_get
          rnno15 = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          IF((rng_seed .GT. 128))call ranmar_get
          rnno16 = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          br = bro + bro1*rnno15
          temp = (1-br)/ko/br
          sinthe = Max(0.,temp*(2-temp))
          rejf3 = (br + 1./br - sinthe)/rejmax
          IF((rnno16.le.rejf3))GO TO6442
        GO TO 6441
6442    CONTINUE
      END IF
      IF ((br .LT. 1./broi .OR. br .GT. 1)) THEN
        IF (( br .LT. 0.99999/broi .OR. br .GT. 1.00001 )) THEN
          write(i_log,'(/a)') '***************** Warning: '
          write(i_log,*) ' sampled br outside of allowed range! ',ko,1./
     *    broi,br
        END IF
        goto 6420
      END IF
      IF (( ibcmp(irl) .EQ. 0 )) THEN
        Uj = 0
        costhe = 1 - temp
        goto 6450
      END IF
      br2 = br*br
      costhe = 1 - temp
      aux = ko*(ko-Uj)*temp
      aux1 = aux-Uj
      pzmax2 = aux1*aux1/(2*aux+Uj*Uj)
6460  CONTINUE
      IF((rng_seed .GT. 128))call ranmar_get
      rnno18 = rng_array(rng_seed)*twom24
      rng_seed = rng_seed + 1
      IF (( rnno18 .LT. 0.5 )) THEN
        rnno18 = Max(1e-30,2*rnno18)
        pz = 0.5*(1-Sqrt(1-2*Log(rnno18)))/Jo_array(j)
        pz2 = pz*pz
        IF (( (pz2 .LE. pzmax2) .AND. (aux1 .LT. 0) )) THEN
          goto 6410
        END IF
      ELSE
        IF (( aux1 .LT. 0 )) THEN
          goto 6410
        END IF
        rnno18 = 2*(1-rnno18)
        pz = 0.5*(Sqrt(1-2*Log(rnno18))-1)/Jo_array(j)
        pz2 = pz*pz
        IF (( pz2 .GE. pzmax2 )) THEN
          goto 6410
        END IF
      END IF
      IF((abs(pz) .GT. 1))goto 6460
      aux = 1 - pz2*br*costhe
      aux1 = 1 - pz2*br2
      aux2 = 1-2*br*costhe+br2*(1-pz2*sinthe)
      IF (( aux2 .GT. 1e-10 )) THEN
        br = br/aux1*(aux+pz*Sqrt(aux2))
      END IF
      Uj = Uj*prm
6450  pesg = br*peig
      pese = peig - pesg - Uj + prm
      sinthe = Sqrt(sinthe)
      call uphi(2,1)
      e(np) = pesg
      aux = 1 + br*br - 2*br*costhe
      IF (( aux .GT. 1e-8 )) THEN
        costhe = (1-br*costhe)/Sqrt(aux)
        sinthe = (1-costhe)*(1+costhe)
        IF (( sinthe .GT. 0 )) THEN
          sinthe = -Sqrt(sinthe)
        ELSE
          sinthe = 0
        END IF
      ELSE
        costhe = 0
        sinthe = -1
      END IF
      np = np + 1
      IF (( np .GT. 50 )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,'(//,3a,/,2(a,i9))') ' In subroutine ','COMPT', ' st
     *ack size exceeded! ',' $MAXSTACK = ',50,' np = ',np
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      call uphi(3,2)
      e(np) = pese
      iq(np) = -1
      IF (( ibcmp(irl) .EQ. 1 )) THEN
        IF (( Uj .GT. 1e-3 )) THEN
          edep = 0
          call relax(Uj,shn_array(j),iz_array(j))
        ELSE
          edep = Uj
        END IF
        IF (( edep .GT. 0 )) THEN
          IARG=4
          IF ((IAUSFL(IARG+1).NE.0)) THEN
            CALL AUSGAB(IARG)
          END IF
        END IF
      END IF
      i_survived_RR = 0
      IF (( i_play_RR .EQ. 1 )) THEN
        IF (( prob_RR .LE. 0 )) THEN
          IF (( n_RR_warning .LT. 50 )) THEN
            n_RR_warning = n_RR_warning + 1
            WRITE(6,6470)prob_RR
6470        FORMAT('**** Warning, attempt to play Roussian Roulette with
     * prob_RR<=0! ',g14.6)
          END IF
        ELSE
          ip = NPold+1
6481      CONTINUE
            IF (( iq(ip) .NE. 0 )) THEN
              IF((rng_seed .GT. 128))call ranmar_get
              rnno_RR = rng_array(rng_seed)*twom24
              rng_seed = rng_seed + 1
              IF (( rnno_RR .LT. prob_RR )) THEN
                wt(ip) = wt(ip)/prob_RR
                ip = ip + 1
              ELSE
                i_survived_RR = i_survived_RR + 1
                IF ((ip .LT. np)) THEN
                  e(ip) = e(np)
                  iq(ip) = iq(np)
                  wt(ip) = wt(np)
                  u(ip) = u(np)
                  v(ip) = v(np)
                  w(ip) = w(np)
                END IF
                np = np-1
              END IF
            ELSE
              ip = ip+1
            END IF
            IF(((ip .GT. np)))GO TO6482
          GO TO 6481
6482      CONTINUE
          IF (( np .EQ. 0 )) THEN
            np = 1
            e(np) = 0
            iq(np) = 0
            wt(np) = 0
          END IF
        END IF
      END IF
      return
6410  return
      end
      SUBROUTINE ELECTR(IRCODE)
      implicit none
      integer*4 IRCODE
      COMMON/QDEBUG/QDEBUG
      LOGICAL QDEBUG
      COMMON/BOUNDS/ECUT(1),PCUT(1),VACDST
      real*8 ECUT,  PCUT,  VACDST
      COMMON/ELECIN/ esig_e(1),psig_e(1), esige_max, psige_max, range_ep
     *(0:1,500,1), E_array(500,1), etae_ms0(500,1),etae_ms1(500,1),etap_
     *ms0(500,1),etap_ms1(500,1),q1ce_ms0(500,1),q1ce_ms1(500,1),q1cp_ms
     *0(500,1),q1cp_ms1(500,1),q2ce_ms0(500,1),q2ce_ms1(500,1),q2cp_ms0(
     *500,1),q2cp_ms1(500,1),blcce0(500,1),blcce1(500,1), EKE0(1),EKE1(1
     *), XR0(1),TEFF0(1),BLCC(1),XCC(1), ESIG0(500,1),ESIG1(500,1),PSIG0
     *(500,1),PSIG1(500,1),EDEDX0(500,1),EDEDX1(500,1),PDEDX0(500,1),PDE
     *DX1(500,1),EBR10(500,1),EBR11(500,1),PBR10(500,1),PBR11(500,1),PBR
     *20(500,1),PBR21(500,1),TMXS0(500,1),TMXS1(500,1), expeke1(1), IUNR
     *ST(1),EPSTFL(1),IAPRIM(1), sig_ismonotone(0:1,1)
      real*8 esig_e,   psig_e,   esige_max,  psige_max,  range_ep,  E_ar
     *ray,  etae_ms0,etae_ms1,  etap_ms0,etap_ms1,  q1ce_ms0,q1ce_ms1,
     *q1cp_ms0,q1cp_ms1,  q2ce_ms0,q2ce_ms1,  q2cp_ms0,q2cp_ms1,  blcce0
     *,blcce1,   expeke1,  EKE0,EKE1, XR0,  TEFF0,  BLCC,  XCC,  ESIG0,E
     *SIG1,  PSIG0,PSIG1,  EDEDX0,EDEDX1,  PDEDX0,PDEDX1,  EBR10,EBR11,
     * PBR10,PBR11,  PBR20,PBR21,  TMXS0,TMXS1
      integer*4 IUNRST,  EPSTFL,  IAPRIM
      logical sig_ismonotone
      COMMON/EPCONT/EDEP,EDEP_LOCAL,TSTEP,TUSTEP,USTEP,TVSTEP,VSTEP, RHO
     *F,EOLD,ENEW,EKE,ELKE,GLE,E_RANGE, x_final,y_final,z_final, u_final
     *,v_final,w_final, IDISC,IROLD,IRNEW,IAUSFL(35)
      DOUBLE PRECISION EDEP,  EDEP_LOCAL
      real*8 TSTEP,  TUSTEP,  USTEP,  VSTEP,  TVSTEP,  RHOF,  EOLD,  ENE
     *W,  EKE,  ELKE,  GLE,  E_RANGE, x_final,y_final,z_final,  u_final,
     *v_final,w_final
      integer*4 IDISC,  IROLD,  IRNEW,  IAUSFL
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      COMMON/MISC/  DUNIT,KMPI,KMPO,RHOR(1),MED(1),IRAYLR(1),IPHOTONUCR(
     *1)
      real*8 DUNIT,  RHOR
      integer*4 KMPI,  KMPO
      integer*2 MED,  IRAYLR,  IPHOTONUCR
      COMMON/STACK/ E(50),X(50),Y(50),Z(50),U(50),V(50),W(50),DNEAR(50),
     *WT(50),IQ(50),IR(50),LATCH(50), LATCHI,NP,NPold
      DOUBLE PRECISION E
      real*8 X,Y,Z,  U,V,W,  DNEAR,  WT
      integer*4 IQ,  IR,  LATCH,  LATCHI, NP,  NPold
      COMMON/THRESH/RMT2,RMSQ, AP(1),AE(1),UP(1),UE(1),TE(1),THMOLL(1)
      real*8 RMT2,  RMSQ,  AP,  AE,  UP,  UE,  TE,  THMOLL
      COMMON/UPHIIN/SINC0,SINC1,SIN0(1002),SIN1(1002)
      real*8 SINC0,SINC1,SIN0,SIN1
      COMMON/UPHIOT/THETA,SINTHE,COSTHE,SINPHI, COSPHI,PI,TWOPI,PI5D2
      real*8 THETA,  SINTHE,  COSTHE,  SINPHI,  COSPHI,  PI,TWOPI,PI5D2
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      common/ET_control/ smaxir(1),estepe,ximax,  skindepth_for_bca,tran
     *sport_algorithm, bca_algorithm,exact_bca,spin_effects
      real*8 smaxir,  estepe,  ximax,      skindepth_for_bca
      integer*4 transport_algorithm, bca_algorithm
      logical exact_bca,  spin_effects
      common/CH_steps/ count_pII_steps,count_all_steps,is_ch_step
      real*8 count_pII_steps,count_all_steps
      logical is_ch_step
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      common/egs_vr/ e_max_rr(1),  prob_RR,  nbr_split,  i_play_RR,
     * i_survived_RR,
     *     n_RR_warning,                                        i_do_rr(
     *1)
      real*8          e_max_rr,prob_RR
      integer*4       nbr_split,i_play_RR,i_survived_RR,n_RR_warning
      integer*2     i_do_rr
      common/emf_inputs/ExIN,EyIN,EzIN,  EMLMTIN,  BxIN, ByIN, BzIN,  Bx
     *, By, Bz,  Bx_new, By_new, Bz_new,  emfield_on
      real*8 ExIN,EyIN,EzIN, EMLMTIN, BxIN,ByIN,BzIN, Bx,By,Bz, Bx_new,B
     *y_new,Bz_new
      logical emfield_on
      common/eii_data/ eii_xsection_a( 10000),  eii_xsection_b( 10000),
     * eii_cons(1), eii_a(40),  eii_b(40),  eii_L_factor,  eii_z(40),  e
     *ii_sh(40),  eii_nshells(100),  eii_nsh(1),  eii_first(1,50),  eii_
     *no(1,50),  eii_flag
      real*8 eii_xsection_a,eii_xsection_b,eii_a,eii_b,eii_cons,eii_L_fa
     *ctor
      integer*4 eii_z,eii_sh,eii_nshells
      integer*4 eii_first,eii_no
      integer*4 eii_elements,eii_flag,eii_nsh
      real*8 lambda_max, sigratio, u_tmp, v_tmp, w_tmp
      LOGICAL random_tustep
      DOUBLE PRECISION  demfp,  peie,  total_tstep,  total_de
      real*8 ekems,  elkems,  chia2,  etap,  lambda,  blccl,  xccl,  xi,
     *  xi_corr,  ms_corr, p2,  beta2,  de,  save_de,  dedx,  dedx0,  de
     *dxmid,  ekei,  elkei,  aux,  ebr1,  eie,  ekef,  elkef,  ekeold,
     *eketmp,  elktmp,  fedep,  tuss,  pbr1,  pbr2,  range,  rfict,  rnn
     *e1,  rnno24,  rnno25,  rnnotu,  rnnoss,  sig,  sig0,  sigf,  skind
     *epth,  ssmfp,  tmxs,  tperp,  ustep0,  uscat,  vscat,  wscat,  xtr
     *ans,  ytrans,  ztrans,  cphi,sphi
      real*8 xphi,xphi2,yphi,yphi2,rhophi2
      integer*4 iarg,  idr,  ierust,  irl,  lelec,  qel,  lelke,  lelkem
     *s,  lelkef,  lelktmp,  ibr
      logical  callhowfar,   domultiple,  dosingle,   callmsdist,
     *                findindex,
     *              spin_index,                                   comput
     *e_tstep
     *
      data ierust/0/
      save ierust
      ircode = 1
      irold = ir(np)
      irl = irold
      medium = med(irl)
6490  CONTINUE
6491    CONTINUE
        lelec = iq(np)
        qel = (1+lelec)/2
        peie = e(np)
        eie = peie
        IF ((eie .LE. ecut(irl))) THEN
          go to 6500
        END IF
        IF ((WT(NP) .EQ. 0.0)) THEN
          go to 6510
        END IF
6520    CONTINUE
6521      CONTINUE
          compute_tstep = .true.
          eke = eie - rm
          IF ((medium .NE. 0)) THEN
            IF((rng_seed .GT. 128))call ranmar_get
            RNNE1 = rng_array(rng_seed)*twom24
            rng_seed = rng_seed + 1
            IF ((RNNE1.EQ.0.0)) THEN
              RNNE1=1.E-30
            END IF
            DEMFP=MAX(-LOG(RNNE1),1.E-5)
            elke = log(eke)
            Lelke=eke1(MEDIUM)*elke+eke0(MEDIUM)
            IF (( sig_ismonotone(qel,medium) )) THEN
              IF ((lelec .LT. 0)) THEN
                sigf=esig1(Lelke,MEDIUM)*elke+esig0(Lelke,MEDIUM)
                dedx0=ededx1(Lelke,MEDIUM)*elke+ededx0(Lelke,MEDIUM)
                sigf = sigf/dedx0
              ELSE
                sigf=psig1(Lelke,MEDIUM)*elke+psig0(Lelke,MEDIUM)
                dedx0=pdedx1(Lelke,MEDIUM)*elke+pdedx0(Lelke,MEDIUM)
                sigf = sigf/dedx0
              END IF
              sig0 = sigf
            ELSE
              IF (( lelec .LT. 0 )) THEN
                sig0 = esig_e(medium)
              ELSE
                sig0 = psig_e(medium)
              END IF
            END IF
          END IF
6530      CONTINUE
6531        CONTINUE
            IF ((medium .EQ. 0)) THEN
              tstep = vacdst
              ustep = tstep
              tustep = ustep
              callhowfar = .true.
              ustep = tustep
            ELSE
              RHOF=RHOR(IRL)/RHO(MEDIUM)
              sig = sig0
              IF ((sig .LE. 0)) THEN
                tstep = vacdst
                sig0 = 1.E-15
              ELSE
                IF (( compute_tstep )) THEN
                  total_de = demfp/sig
                  fedep = total_de
                  ekef = eke - fedep
                  IF (( ekef .LE. E_array(1,medium) )) THEN
                    tstep = vacdst
                  ELSE
                    elkef = Log(ekef)
                    Lelkef=eke1(MEDIUM)*elkef+eke0(MEDIUM)
                    IF (( lelkef .EQ. lelke )) THEN
                      fedep = 1 - ekef/eke
                      elktmp = 0.5*(elke+elkef+0.25*fedep*fedep*(1+fedep
     *                *(1+0.875*fedep)))
                      lelktmp = lelke
                      IF ((lelec .LT. 0)) THEN
                        dedxmid=ededx1(Lelktmp,MEDIUM)*elktmp+ededx0(Lel
     *                  ktmp,MEDIUM)
                        dedxmid = 1/dedxmid
                        aux = ededx1(lelktmp,medium)*dedxmid
                      ELSE
                        dedxmid=pdedx1(Lelktmp,MEDIUM)*elktmp+pdedx0(Lel
     *                  ktmp,MEDIUM)
                        dedxmid = 1/dedxmid
                        aux = pdedx1(lelktmp,medium)*dedxmid
                      END IF
                      aux = aux*(1+2*aux)*(fedep/(2-fedep))**2/6
                      tstep = fedep*eke*dedxmid*(1+aux)
                    ELSE
                      ekei = E_array(lelke,medium)
                      elkei = (lelke - eke0(medium))/eke1(medium)
                      fedep = 1 - ekei/eke
                      elktmp = 0.5*(elke+elkei+0.25*fedep*fedep*(1+fedep
     *                *(1+0.875*fedep)))
                      lelktmp = lelke
                      IF ((lelec .LT. 0)) THEN
                        dedxmid=ededx1(Lelktmp,MEDIUM)*elktmp+ededx0(Lel
     *                  ktmp,MEDIUM)
                        dedxmid = 1/dedxmid
                        aux = ededx1(lelktmp,medium)*dedxmid
                      ELSE
                        dedxmid=pdedx1(Lelktmp,MEDIUM)*elktmp+pdedx0(Lel
     *                  ktmp,MEDIUM)
                        dedxmid = 1/dedxmid
                        aux = pdedx1(lelktmp,medium)*dedxmid
                      END IF
                      aux = aux*(1+2*aux)*(fedep/(2-fedep))**2/6
                      tuss = fedep*eke*dedxmid*(1+aux)
                      ekei = E_array(lelkef+1,medium)
                      elkei = (lelkef + 1 - eke0(medium))/eke1(medium)
                      fedep = 1 - ekef/ekei
                      elktmp = 0.5*(elkei+elkef+0.25*fedep*fedep*(1+fede
     *                p*(1+0.875*fedep)))
                      lelktmp = lelkef
                      IF ((lelec .LT. 0)) THEN
                        dedxmid=ededx1(Lelktmp,MEDIUM)*elktmp+ededx0(Lel
     *                  ktmp,MEDIUM)
                        dedxmid = 1/dedxmid
                        aux = ededx1(lelktmp,medium)*dedxmid
                      ELSE
                        dedxmid=pdedx1(Lelktmp,MEDIUM)*elktmp+pdedx0(Lel
     *                  ktmp,MEDIUM)
                        dedxmid = 1/dedxmid
                        aux = pdedx1(lelktmp,medium)*dedxmid
                      END IF
                      aux = aux*(1+2*aux)*(fedep/(2-fedep))**2/6
                      tstep = fedep*ekei*dedxmid*(1+aux)
                      tstep=tstep+tuss+ range_ep(qel,lelke,medium)-range
     *                _ep(qel,lelkef+1,medium)
                    END IF
                  END IF
                  total_tstep = tstep
                  compute_tstep = .false.
                END IF
                tstep = total_tstep/rhof
              END IF
              IF ((lelec .LT. 0)) THEN
                dedx0=ededx1(Lelke,MEDIUM)*elke+ededx0(Lelke,MEDIUM)
              ELSE
                dedx0=pdedx1(Lelke,MEDIUM)*elke+pdedx0(Lelke,MEDIUM)
              END IF
              dedx = rhof*dedx0
              tmxs=tmxs1(Lelke,MEDIUM)*elke+tmxs0(Lelke,MEDIUM)
              tmxs = tmxs/rhof
              ekei = E_array(lelke,medium)
              elkei = (lelke - eke0(medium))/eke1(medium)
              fedep = 1 - ekei/eke
              elktmp = 0.5*(elke+elkei+0.25*fedep*fedep*(1+fedep*(1+0.87
     *        5*fedep)))
              lelktmp = lelke
              IF ((lelec .LT. 0)) THEN
                dedxmid=ededx1(Lelktmp,MEDIUM)*elktmp+ededx0(Lelktmp,MED
     *          IUM)
                dedxmid = 1/dedxmid
                aux = ededx1(lelktmp,medium)*dedxmid
              ELSE
                dedxmid=pdedx1(Lelktmp,MEDIUM)*elktmp+pdedx0(Lelktmp,MED
     *          IUM)
                dedxmid = 1/dedxmid
                aux = pdedx1(lelktmp,medium)*dedxmid
              END IF
              aux = aux*(1+2*aux)*(fedep/(2-fedep))**2/6
              range = fedep*eke*dedxmid*(1+aux)
              range = (range + range_ep(qel,lelke,medium))/rhof
              random_tustep = .false.
              IF ((random_tustep)) THEN
                IF((rng_seed .GT. 128))call ranmar_get
                rnnotu = rng_array(rng_seed)*twom24
                rng_seed = rng_seed + 1
                tmxs = rnnotu*min(tmxs,smaxir(irl))
              ELSE
                tmxs = min(tmxs,smaxir(irl))
              END IF
              tustep = min(tstep,tmxs,range)
              tperp = 1e10
              dnear(np) = tperp
              IF (( i_do_rr(irl) .EQ. 1 .AND. e(np) .LT. e_max_rr(irl) )
     *        ) THEN
                IF ((tperp .GE. range)) THEN
                  idisc = 50 + 49*iq(np)
                  go to 6510
                END IF
              END IF
              blccl = rhof*blcc(medium)
              xccl = rhof*xcc(medium)
              p2 = eke*(eke+rmt2)
              beta2 = p2/(p2 + rmsq)
              IF (( spin_effects )) THEN
                IF ((lelec .LT. 0)) THEN
                  etap=etae_ms1(Lelke,MEDIUM)*elke+etae_ms0(Lelke,MEDIUM
     *            )
                ELSE
                  etap=etap_ms1(Lelke,MEDIUM)*elke+etap_ms0(Lelke,MEDIUM
     *            )
                END IF
                ms_corr=blcce1(Lelke,MEDIUM)*elke+blcce0(Lelke,MEDIUM)
                blccl = blccl/etap/(1+0.25*etap*xccl/blccl/p2)*ms_corr
              END IF
              ssmfp=beta2/blccl
              skindepth = skindepth_for_bca*ssmfp
              tustep = min(tustep,max(tperp,skindepth))
              count_all_steps = count_all_steps + 1
              is_ch_step = .false.
              IF (((tustep .LE. tperp) .AND. ((.NOT.exact_bca) .OR. (tus
     *        tep .GT. skindepth)))) THEN
                callhowfar = .false.
                domultiple = .false.
                dosingle = .false.
                callmsdist = .true.
                tuss = range - range_ep(qel,lelke,medium)/rhof
                IF (( tuss .GE. tustep )) THEN
                  IF (( lelec .LT. 0 )) THEN
                    dedxmid=ededx1(Lelke,MEDIUM)*elke+ededx0(Lelke,MEDIU
     *              M)
                    aux = ededx1(lelke,medium)/dedxmid
                  ELSE
                    dedxmid=pdedx1(Lelke,MEDIUM)*elke+pdedx0(Lelke,MEDIU
     *              M)
                    aux = pdedx1(lelke,medium)/dedxmid
                  END IF
                  de = dedxmid*tustep*rhof
                  fedep = de/eke
                  de = de*(1-0.5*fedep*aux*(1-0.333333*fedep*(aux-1- 0.2
     *            5*fedep*(2-aux*(4-aux)))))
                ELSE
                  lelktmp = lelke
                  tuss = (range - tustep)*rhof
                  IF (( tuss .LE. 0 )) THEN
                    de = eke - TE(medium)*0.99
                  ELSE
6541                IF(tuss.GE.range_ep(qel,lelktmp,medium))GO TO 6542
                      lelktmp = lelktmp - 1
                    GO TO 6541
6542                CONTINUE
                    elktmp = (lelktmp+1-eke0(medium))/eke1(medium)
                    eketmp = E_array(lelktmp+1,medium)
                    tuss = (range_ep(qel,lelktmp+1,medium) - tuss)/rhof
                    IF (( lelec .LT. 0 )) THEN
                      dedxmid=ededx1(Lelktmp,MEDIUM)*elktmp+ededx0(Lelkt
     *                mp,MEDIUM)
                      aux = ededx1(lelktmp,medium)/dedxmid
                    ELSE
                      dedxmid=pdedx1(Lelktmp,MEDIUM)*elktmp+pdedx0(Lelkt
     *                mp,MEDIUM)
                      aux = pdedx1(lelktmp,medium)/dedxmid
                    END IF
                    de = dedxmid*tuss*rhof
                    fedep = de/eketmp
                    de = de*(1-0.5*fedep*aux*(1-0.333333*fedep*(aux-1- 0
     *              .25*fedep*(2-aux*(4-aux)))))
                    de = de + eke - eketmp
                  END IF
                END IF
                tvstep = tustep
                is_ch_step = .true.
                IF ((transport_algorithm .EQ. 0)) THEN
                  call msdist_pII (  eke,de,tustep,rhof,medium,qel,spin_
     *            effects, u(np),v(np),w(np),x(np),y(np),z(np),  uscat,v
     *            scat,wscat,xtrans,ytrans,ztrans,ustep )
                ELSE
                  call msdist_pI (  eke,de,tustep,rhof,medium,qel,spin_e
     *            ffects, u(np),v(np),w(np),x(np),y(np),z(np),  uscat,vs
     *            cat,wscat,xtrans,ytrans,ztrans,ustep )
                END IF
              ELSE
                callmsdist = .false.
                IF ((exact_bca)) THEN
                  domultiple = .false.
                  IF((rng_seed .GT. 128))call ranmar_get
                  rnnoss = rng_array(rng_seed)*twom24
                  rng_seed = rng_seed + 1
                  IF (( rnnoss .LT. 1.e-30 )) THEN
                    rnnoss = 1.e-30
                  END IF
                  lambda = - Log(1 - rnnoss)
                  lambda_max = 0.5*blccl*rm/dedx*(eke/rm+1)**3
                  IF (( lambda .GE. 0 .AND. lambda_max .GT. 0 )) THEN
                    IF (( lambda .LT. lambda_max )) THEN
                      tuss=lambda*ssmfp*(1-0.5*lambda/lambda_max)
                    ELSE
                      tuss = 0.5 * lambda * ssmfp
                    END IF
                    IF ((tuss .LT. tustep)) THEN
                      tustep = tuss
                      dosingle = .true.
                    ELSE
                      dosingle = .false.
                    END IF
                  ELSE
                    write(i_log,'(/a)') '***************** Warning: '
                    write(i_log,*) ' lambda > lambda_max: ', lambda,lamb
     *              da_max,' eke dedx: ',eke,dedx, ' ir medium blcc: ',i
     *              r(np),medium,blcc(medium), ' position = ',x(np),y(np
     *              ),z(np)
                    dosingle = .false.
                    np=np-1
                    return
                  END IF
                  ustep = tustep
                ELSE
                  dosingle = .false.
                  domultiple = .true.
                  ekems = eke - 0.5*tustep*dedx
                  p2 = ekems*(ekems+rmt2)
                  beta2 = p2/(p2 + rmsq)
                  chia2 = xccl/(4*blccl*p2)
                  xi = 0.5*xccl/p2/beta2*tustep
                  IF (( spin_effects )) THEN
                    elkems = Log(ekems)
                    Lelkems=eke1(MEDIUM)*elkems+eke0(MEDIUM)
                    IF ((lelec .LT. 0)) THEN
                      etap=etae_ms1(Lelkems,MEDIUM)*elkems+etae_ms0(Lelk
     *                ems,MEDIUM)
                      xi_corr=q1ce_ms1(Lelkems,MEDIUM)*elkems+q1ce_ms0(L
     *                elkems,MEDIUM)
                    ELSE
                      etap=etap_ms1(Lelkems,MEDIUM)*elkems+etap_ms0(Lelk
     *                ems,MEDIUM)
                      xi_corr=q1cp_ms1(Lelkems,MEDIUM)*elkems+q1cp_ms0(L
     *                elkems,MEDIUM)
                    END IF
                    chia2 = chia2*etap
                    xi = xi*xi_corr
                    ms_corr=blcce1(Lelkems,MEDIUM)*elkems+blcce0(Lelkems
     *              ,MEDIUM)
                    blccl = blccl*ms_corr
                  ELSE
                    xi_corr = 1
                    etap = 1
                  END IF
                  xi = xi*(Log(1+1./chia2)-1/(1+chia2))
                  IF (( xi .LT. 0.1 )) THEN
                    ustep = tustep*(1 - xi*(0.5 - xi*0.166667))
                  ELSE
                    ustep = tustep*(1 - Exp(-xi))/xi
                  END IF
                END IF
                IF ((ustep .LT. tperp)) THEN
                  callhowfar = .false.
                ELSE
                  callhowfar = .true.
                END IF
              END IF
            END IF
            irold = ir(np)
            irnew = ir(np)
            idisc = 0
            ustep0 = ustep
            IF ((idisc .GT. 0)) THEN
              go to 6510
            END IF
            IF ((ustep .LE. 0)) THEN
              IF ((ustep .LT. -1e-4)) THEN
                ierust = ierust + 1
                WRITE(6,6550)ierust,ustep,dedx,e(np)-prm, ir(np),irnew,i
     *          rold,x(np),y(np),z(np)
6550            FORMAT(i4,' Negative ustep = ',e12.5,' dedx=',F8.4,' ke=
     *',F8.4, ' ir,irnew,irold =',3i4,' x,y,z =',4e10.3)
                IF ((ierust .GT. 1000)) THEN
                  WRITE(6,6560)
6560              FORMAT(////' Called exit---too many ustep errors'///)
                  call exit(1)
                END IF
              END IF
              ustep = 0
            END IF
            IF ((ustep .EQ. 0 .OR. medium .EQ. 0)) THEN
              IF ((ustep .NE. 0)) THEN
                vstep = ustep
                tvstep = vstep
                edep = pzero
                e_range = vacdst
                IARG=0
                IF ((IAUSFL(IARG+1).NE.0)) THEN
                  CALL AUSGAB(IARG)
                END IF
                x(np) = x(np) + u(np)*vstep
                y(np) = y(np) + v(np)*vstep
                z(np) = z(np) + w(np)*vstep
                dnear(np) = dnear(np) - vstep
              END IF
              IF ((irnew .NE. irold)) THEN
                ir(np) = irnew
                irl = irnew
                medium = med(irl)
              END IF
              IF ((ustep .NE. 0)) THEN
                IARG=5
                IF ((IAUSFL(IARG+1).NE.0)) THEN
                  CALL AUSGAB(IARG)
                END IF
              END IF
              IF ((eie .LE. ecut(irl))) THEN
                go to 6500
              END IF
              IF ((ustep .NE. 0 .AND. idisc .LT. 0)) THEN
                go to 6510
              END IF
              GO TO 6521
            END IF
            vstep = ustep
            IF ((callhowfar)) THEN
              IF ((exact_bca)) THEN
                tvstep = vstep
                IF ((tvstep .NE. tustep)) THEN
                  dosingle = .false.
                END IF
              ELSE
                IF (( vstep .LT. ustep0 )) THEN
                  ekems = eke - 0.5*tustep*vstep/ustep0*dedx
                  p2 = ekems*(ekems+rmt2)
                  beta2 = p2/(p2 + rmsq)
                  chia2 = xccl/(4*blccl*p2)
                  xi = 0.5*xccl/p2/beta2*vstep
                  IF (( spin_effects )) THEN
                    elkems = Log(ekems)
                    Lelkems=eke1(MEDIUM)*elkems+eke0(MEDIUM)
                    IF ((lelec .LT. 0)) THEN
                      etap=etae_ms1(Lelkems,MEDIUM)*elkems+etae_ms0(Lelk
     *                ems,MEDIUM)
                      xi_corr=q1ce_ms1(Lelkems,MEDIUM)*elkems+q1ce_ms0(L
     *                elkems,MEDIUM)
                    ELSE
                      etap=etap_ms1(Lelkems,MEDIUM)*elkems+etap_ms0(Lelk
     *                ems,MEDIUM)
                      xi_corr=q1cp_ms1(Lelkems,MEDIUM)*elkems+q1cp_ms0(L
     *                elkems,MEDIUM)
                    END IF
                    chia2 = chia2*etap
                    xi = xi*xi_corr
                    ms_corr=blcce1(Lelkems,MEDIUM)*elkems+blcce0(Lelkems
     *              ,MEDIUM)
                    blccl = blccl*ms_corr
                  ELSE
                    xi_corr = 1
                    etap = 1
                  END IF
                  xi = xi*(Log(1+1./chia2)-1/(1+chia2))
                  IF (( xi .LT. 0.1 )) THEN
                    tvstep = vstep*(1 + xi*(0.5 + xi*0.333333))
                  ELSE
                    IF (( xi .LT. 0.999999 )) THEN
                      tvstep = -vstep*Log(1 - xi)/xi
                    ELSE
                      write(i_log,*) ' Stoped in SET-TVSTEP because xi >
     * 1! '
                      write(i_log,*) ' Medium: ',medium
                      write(i_log,*) ' Initial energy: ',eke
                      write(i_log,*) ' Average step energy: ',ekems
                      write(i_log,*) ' tustep: ',tustep
                      write(i_log,*) ' ustep0: ',ustep0
                      write(i_log,*) ' vstep:  ',vstep
                      write(i_log,*) ' ==> xi = ',xi
                      write(i_log,'(/a)') '***************** Error: '
                      write(i_log,*) 'This is a fatal error condition'
                      write(i_log,'(/a)') '***************** Quiting now
     *.'
                      call exit(1)
                    END IF
                  END IF
                ELSE
                  tvstep = tustep
                END IF
              END IF
              tuss = range - range_ep(qel,lelke,medium)/rhof
              IF (( tuss .GE. tvstep )) THEN
                IF (( lelec .LT. 0 )) THEN
                  dedxmid=ededx1(Lelke,MEDIUM)*elke+ededx0(Lelke,MEDIUM)
                  aux = ededx1(lelke,medium)/dedxmid
                ELSE
                  dedxmid=pdedx1(Lelke,MEDIUM)*elke+pdedx0(Lelke,MEDIUM)
                  aux = pdedx1(lelke,medium)/dedxmid
                END IF
                de = dedxmid*tvstep*rhof
                fedep = de/eke
                de = de*(1-0.5*fedep*aux*(1-0.333333*fedep*(aux-1- 0.25*
     *          fedep*(2-aux*(4-aux)))))
              ELSE
                lelktmp = lelke
                tuss = (range - tvstep)*rhof
                IF (( tuss .LE. 0 )) THEN
                  de = eke - TE(medium)*0.99
                ELSE
6571              IF(tuss.GE.range_ep(qel,lelktmp,medium))GO TO 6572
                    lelktmp = lelktmp - 1
                  GO TO 6571
6572              CONTINUE
                  elktmp = (lelktmp+1-eke0(medium))/eke1(medium)
                  eketmp = E_array(lelktmp+1,medium)
                  tuss = (range_ep(qel,lelktmp+1,medium) - tuss)/rhof
                  IF (( lelec .LT. 0 )) THEN
                    dedxmid=ededx1(Lelktmp,MEDIUM)*elktmp+ededx0(Lelktmp
     *              ,MEDIUM)
                    aux = ededx1(lelktmp,medium)/dedxmid
                  ELSE
                    dedxmid=pdedx1(Lelktmp,MEDIUM)*elktmp+pdedx0(Lelktmp
     *              ,MEDIUM)
                    aux = pdedx1(lelktmp,medium)/dedxmid
                  END IF
                  de = dedxmid*tuss*rhof
                  fedep = de/eketmp
                  de = de*(1-0.5*fedep*aux*(1-0.333333*fedep*(aux-1- 0.2
     *            5*fedep*(2-aux*(4-aux)))))
                  de = de + eke - eketmp
                END IF
              END IF
            ELSE
              tvstep = tustep
              IF (( .NOT.callmsdist )) THEN
                tuss = range - range_ep(qel,lelke,medium)/rhof
                IF (( tuss .GE. tvstep )) THEN
                  IF (( lelec .LT. 0 )) THEN
                    dedxmid=ededx1(Lelke,MEDIUM)*elke+ededx0(Lelke,MEDIU
     *              M)
                    aux = ededx1(lelke,medium)/dedxmid
                  ELSE
                    dedxmid=pdedx1(Lelke,MEDIUM)*elke+pdedx0(Lelke,MEDIU
     *              M)
                    aux = pdedx1(lelke,medium)/dedxmid
                  END IF
                  de = dedxmid*tvstep*rhof
                  fedep = de/eke
                  de = de*(1-0.5*fedep*aux*(1-0.333333*fedep*(aux-1- 0.2
     *            5*fedep*(2-aux*(4-aux)))))
                ELSE
                  lelktmp = lelke
                  tuss = (range - tvstep)*rhof
                  IF (( tuss .LE. 0 )) THEN
                    de = eke - TE(medium)*0.99
                  ELSE
6581                IF(tuss.GE.range_ep(qel,lelktmp,medium))GO TO 6582
                      lelktmp = lelktmp - 1
                    GO TO 6581
6582                CONTINUE
                    elktmp = (lelktmp+1-eke0(medium))/eke1(medium)
                    eketmp = E_array(lelktmp+1,medium)
                    tuss = (range_ep(qel,lelktmp+1,medium) - tuss)/rhof
                    IF (( lelec .LT. 0 )) THEN
                      dedxmid=ededx1(Lelktmp,MEDIUM)*elktmp+ededx0(Lelkt
     *                mp,MEDIUM)
                      aux = ededx1(lelktmp,medium)/dedxmid
                    ELSE
                      dedxmid=pdedx1(Lelktmp,MEDIUM)*elktmp+pdedx0(Lelkt
     *                mp,MEDIUM)
                      aux = pdedx1(lelktmp,medium)/dedxmid
                    END IF
                    de = dedxmid*tuss*rhof
                    fedep = de/eketmp
                    de = de*(1-0.5*fedep*aux*(1-0.333333*fedep*(aux-1- 0
     *              .25*fedep*(2-aux*(4-aux)))))
                    de = de + eke - eketmp
                  END IF
                END IF
              END IF
            END IF
            save_de = de
            edep = de
            ekef = eke - de
            eold = eie
            enew = eold - de
            IF (( .NOT.callmsdist )) THEN
              IF (( domultiple )) THEN
                lambda = blccl*tvstep/beta2/etap/(1+chia2)
                xi = xi/xi_corr
                findindex = .true.
                spin_index = .true.
                call mscat(lambda,chia2,xi,elkems,beta2,qel,medium, spin
     *          _effects,findindex,spin_index, costhe,sinthe)
              ELSE
                IF ((dosingle)) THEN
                  ekems = Max(ekef,ecut(irl)-rm)
                  p2 = ekems*(ekems + rmt2)
                  beta2 = p2/(p2 + rmsq)
                  chia2 = xcc(medium)/(4*blcc(medium)*p2)
                  IF (( spin_effects )) THEN
                    elkems = Log(ekems)
                    Lelkems=eke1(MEDIUM)*elkems+eke0(MEDIUM)
                    IF ((lelec .LT. 0)) THEN
                      etap=etae_ms1(Lelkems,MEDIUM)*elkems+etae_ms0(Lelk
     *                ems,MEDIUM)
                    ELSE
                      etap=etap_ms1(Lelkems,MEDIUM)*elkems+etap_ms0(Lelk
     *                ems,MEDIUM)
                    END IF
                    chia2 = chia2*etap
                  END IF
                  call sscat(chia2,elkems,beta2,qel,medium, spin_effects
     *            ,costhe,sinthe)
                ELSE
                  theta = 0
                  sinthe = 0
                  costhe = 1
                END IF
              END IF
            END IF
            e_range = range
            IF (( callmsdist )) THEN
              u_final = uscat
              v_final = vscat
              w_final = wscat
              x_final = xtrans
              y_final = ytrans
              z_final = ztrans
            ELSE
              x_final = x(np) + u(np)*vstep
              y_final = y(np) + v(np)*vstep
              z_final = z(np) + w(np)*vstep
              IF (( domultiple .OR. dosingle )) THEN
                u_tmp = u(np)
                v_tmp = v(np)
                w_tmp = w(np)
                call uphi(2,1)
                u_final = u(np)
                v_final = v(np)
                w_final = w(np)
                u(np) = u_tmp
                v(np) = v_tmp
                w(np) = w_tmp
              ELSE
                u_final = u(np)
                v_final = v(np)
                w_final = w(np)
              END IF
            END IF
            IARG=0
            IF ((IAUSFL(IARG+1).NE.0)) THEN
              CALL AUSGAB(IARG)
            END IF
            x(np) = x_final
            y(np) = y_final
            z(np) = z_final
            u(np) = u_final
            v(np) = v_final
            w(np) = w_final
            dnear(np) = dnear(np) - vstep
            irold = ir(np)
            peie = peie - edep
            eie = peie
            e(np) = peie
            IF (( irnew .EQ. irl .AND. eie .LE. ecut(irl))) THEN
              go to 6500
            END IF
            medold = medium
            IF ((medium .NE. 0)) THEN
              ekeold = eke
              eke = eie - rm
              elke = log(eke)
              Lelke=eke1(MEDIUM)*elke+eke0(MEDIUM)
            END IF
            IF ((irnew .NE. irold)) THEN
              ir(np) = irnew
              irl = irnew
              medium = med(irl)
            END IF
            IARG=5
            IF ((IAUSFL(IARG+1).NE.0)) THEN
              CALL AUSGAB(IARG)
            END IF
            IF ((eie .LE. ecut(irl))) THEN
              go to 6500
            END IF
            IF ((idisc .LT. 0)) THEN
              go to 6510
            END IF
            IF((medium .NE. medold))GO TO 6521
            demfp = demfp - save_de*sig
            total_de = total_de - save_de
            total_tstep = total_tstep - tvstep*rhof
            IF (( total_tstep .LT. 1e-9 )) THEN
              demfp = 0
            END IF
            IF(((demfp .LT. 1.E-5)))GO TO6532
          GO TO 6531
6532      CONTINUE
          IF ((lelec .LT. 0)) THEN
            sigf=esig1(Lelke,MEDIUM)*elke+esig0(Lelke,MEDIUM)
            dedx0=ededx1(Lelke,MEDIUM)*elke+ededx0(Lelke,MEDIUM)
            sigf = sigf/dedx0
          ELSE
            sigf=psig1(Lelke,MEDIUM)*elke+psig0(Lelke,MEDIUM)
            dedx0=pdedx1(Lelke,MEDIUM)*elke+pdedx0(Lelke,MEDIUM)
            sigf = sigf/dedx0
          END IF
          sigratio = sigf/sig0
          IF((rng_seed .GT. 128))call ranmar_get
          rfict = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          IF(((rfict .LE. sigratio)))GO TO6522
        GO TO 6521
6522    CONTINUE
        IF ((lelec .LT. 0)) THEN
          ebr1=ebr11(Lelke,MEDIUM)*elke+ebr10(Lelke,MEDIUM)
          IF((rng_seed .GT. 128))call ranmar_get
          rnno24 = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          IF ((rnno24 .LE. ebr1)) THEN
            go to 6590
          ELSE
            IF ((e(np) .LE. thmoll(medium) .AND. eii_flag .EQ. 0)) THEN
              IF ((ebr1 .LE. 0)) THEN
                go to 6490
              END IF
              go to 6590
            END IF
            IARG=8
            IF ((IAUSFL(IARG+1).NE.0)) THEN
              CALL AUSGAB(IARG)
            END IF
            call moller
            IARG=9
            IF ((IAUSFL(IARG+1).NE.0)) THEN
              CALL AUSGAB(IARG)
            END IF
            IF((iq(np) .EQ. 0))return
          END IF
          go to 6490
        END IF
        pbr1=pbr11(Lelke,MEDIUM)*elke+pbr10(Lelke,MEDIUM)
        IF((rng_seed .GT. 128))call ranmar_get
        rnno25 = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        IF ((rnno25 .LT. pbr1)) THEN
          go to 6590
        END IF
        pbr2=pbr21(Lelke,MEDIUM)*elke+pbr20(Lelke,MEDIUM)
        IF ((rnno25 .LT. pbr2)) THEN
          IARG=10
          IF ((IAUSFL(IARG+1).NE.0)) THEN
            CALL AUSGAB(IARG)
          END IF
          call bhabha
          IARG=11
          IF ((IAUSFL(IARG+1).NE.0)) THEN
            CALL AUSGAB(IARG)
          END IF
          IF((iq(np) .EQ. 0))return
        ELSE
          IARG=12
          IF ((IAUSFL(IARG+1).NE.0)) THEN
            CALL AUSGAB(IARG)
          END IF
          call annih
          IARG=13
          IF ((IAUSFL(IARG+1).NE.0)) THEN
            CALL AUSGAB(IARG)
          END IF
          GO TO 6492
        END IF
      GO TO 6491
6492  CONTINUE
      return
6590  IARG=6
      IF ((IAUSFL(IARG+1).NE.0)) THEN
        CALL AUSGAB(IARG)
      END IF
      call brems
      IARG=7
      IF ((IAUSFL(IARG+1).NE.0)) THEN
        CALL AUSGAB(IARG)
      END IF
      IF ((iq(np) .EQ. 0)) THEN
        return
      ELSE
        go to 6490
      END IF
6500  IF (( medium .GT. 0 )) THEN
        IF ((eie .GT. ae(medium))) THEN
          idr = 1
          IF ((lelec .LT. 0)) THEN
            edep = e(np) - prm
          ELSE
            EDEP=PEIE-PRM
          END IF
        ELSE
          idr = 2
          edep = e(np) - prm
        END IF
      ELSE
        idr = 1
        edep = e(np) - prm
      END IF
      IARG=idr
      IF ((IAUSFL(IARG+1).NE.0)) THEN
        CALL AUSGAB(IARG)
      END IF
6600  CONTINUE
      IF ((lelec .GT. 0)) THEN
        IF ((edep .LT. peie)) THEN
          IARG=28
          IF ((IAUSFL(IARG+1).NE.0)) THEN
            CALL AUSGAB(IARG)
          END IF
          call annih_at_rest
          IARG=14
          IF ((IAUSFL(IARG+1).NE.0)) THEN
            CALL AUSGAB(IARG)
          END IF
          return
        END IF
      END IF
      np = np - 1
      ircode = 2
      return
6510  idisc = abs(idisc)
      IF (((lelec .LT. 0) .OR. (idisc .EQ. 99))) THEN
        edep = e(np) - prm
      ELSE
        edep = e(np) + prm
      END IF
      IARG=3
      IF ((IAUSFL(IARG+1).NE.0)) THEN
        CALL AUSGAB(IARG)
      END IF
      IF((idisc .EQ. 99))goto 6600
      np = np - 1
      ircode = 2
      return
      end
      SUBROUTINE HATCH
      implicit none
      character*512 toUpper
      COMMON/QDEBUG/QDEBUG
      LOGICAL QDEBUG
      COMMON/BOUNDS/ECUT(1),PCUT(1),VACDST
      real*8 ECUT,  PCUT,  VACDST
      COMMON/BREMPR/ DL1(8,1),DL2(8,1),DL3(8,1),DL4(8,1),DL5(8,1),DL6(8,
     *1), ALPHI(2,1),BPAR(2,1),DELPOS(2,1), WA(1,50),PZ(1,50),ZELEM(1,50
     *),RHOZ(1,50), PWR2I(50), DELCM(1),ZBRANG(1),LZBRANG(1),NNE(1), IBR
     *DST,IPRDST,ibr_nist,pair_nrc,itriplet, ASYM(1,50,2)
      CHARACTER*4 ASYM
      real*8 DL1,DL2,DL3,DL4,DL5,DL6,   ALPHI,  BPAR,  DELPOS,  WA,  PZ,
     *  ZELEM,  RHOZ,  PWR2I,  DELCM,  ZBRANG,  LZBRANG
      integer*4 NNE,  IBRDST,  IPRDST,  ibr_nist,  itriplet,  pair_nrc
      COMMON/ELECIN/ esig_e(1),psig_e(1), esige_max, psige_max, range_ep
     *(0:1,500,1), E_array(500,1), etae_ms0(500,1),etae_ms1(500,1),etap_
     *ms0(500,1),etap_ms1(500,1),q1ce_ms0(500,1),q1ce_ms1(500,1),q1cp_ms
     *0(500,1),q1cp_ms1(500,1),q2ce_ms0(500,1),q2ce_ms1(500,1),q2cp_ms0(
     *500,1),q2cp_ms1(500,1),blcce0(500,1),blcce1(500,1), EKE0(1),EKE1(1
     *), XR0(1),TEFF0(1),BLCC(1),XCC(1), ESIG0(500,1),ESIG1(500,1),PSIG0
     *(500,1),PSIG1(500,1),EDEDX0(500,1),EDEDX1(500,1),PDEDX0(500,1),PDE
     *DX1(500,1),EBR10(500,1),EBR11(500,1),PBR10(500,1),PBR11(500,1),PBR
     *20(500,1),PBR21(500,1),TMXS0(500,1),TMXS1(500,1), expeke1(1), IUNR
     *ST(1),EPSTFL(1),IAPRIM(1), sig_ismonotone(0:1,1)
      real*8 esig_e,   psig_e,   esige_max,  psige_max,  range_ep,  E_ar
     *ray,  etae_ms0,etae_ms1,  etap_ms0,etap_ms1,  q1ce_ms0,q1ce_ms1,
     *q1cp_ms0,q1cp_ms1,  q2ce_ms0,q2ce_ms1,  q2cp_ms0,q2cp_ms1,  blcce0
     *,blcce1,   expeke1,  EKE0,EKE1, XR0,  TEFF0,  BLCC,  XCC,  ESIG0,E
     *SIG1,  PSIG0,PSIG1,  EDEDX0,EDEDX1,  PDEDX0,PDEDX1,  EBR10,EBR11,
     * PBR10,PBR11,  PBR20,PBR21,  TMXS0,TMXS1
      integer*4 IUNRST,  EPSTFL,  IAPRIM
      logical sig_ismonotone
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      COMMON/MISC/  DUNIT,KMPI,KMPO,RHOR(1),MED(1),IRAYLR(1),IPHOTONUCR(
     *1)
      real*8 DUNIT,  RHOR
      integer*4 KMPI,  KMPO
      integer*2 MED,  IRAYLR,  IPHOTONUCR
      COMMON/PHOTIN/ EBINDA(1), GE0(1),GE1(1), GMFP0(2000,1),GMFP1(2000,
     *1),GBR10(2000,1),GBR11(2000,1),GBR20(2000,1),GBR21(2000,1), RCO0(1
     *),RCO1(1), RSCT0(100,1),RSCT1(100,1), COHE0(2000,1),COHE1(2000,1),
     *  PHOTONUC0(2000,1),PHOTONUC1(2000,1), DPMFP, MPGEM(1,1), NGR(1)
      real*8 EBINDA,  GE0,GE1,  GMFP0,GMFP1,  GBR10,GBR11,  GBR20,GBR21,
     *  RCO0,RCO1,  RSCT0,RSCT1,  COHE0,COHE1,   PHOTONUC0,PHOTONUC1,  D
     *PMFP
      integer*4
     *                  MPGEM,
     *                          NGR
      COMMON/STACK/ E(50),X(50),Y(50),Z(50),U(50),V(50),W(50),DNEAR(50),
     *WT(50),IQ(50),IR(50),LATCH(50), LATCHI,NP,NPold
      DOUBLE PRECISION E
      real*8 X,Y,Z,  U,V,W,  DNEAR,  WT
      integer*4 IQ,  IR,  LATCH,  LATCHI, NP,  NPold
      COMMON/THRESH/RMT2,RMSQ, AP(1),AE(1),UP(1),UE(1),TE(1),THMOLL(1)
      real*8 RMT2,  RMSQ,  AP,  AE,  UP,  UE,  TE,  THMOLL
      COMMON/UPHIIN/SINC0,SINC1,SIN0(1002),SIN1(1002)
      real*8 SINC0,SINC1,SIN0,SIN1
      COMMON/UPHIOT/THETA,SINTHE,COSTHE,SINPHI, COSPHI,PI,TWOPI,PI5D2
      real*8 THETA,  SINTHE,  COSTHE,  SINPHI,  COSPHI,  PI,TWOPI,PI5D2
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      common/ET_control/ smaxir(1),estepe,ximax,  skindepth_for_bca,tran
     *sport_algorithm, bca_algorithm,exact_bca,spin_effects
      real*8 smaxir,  estepe,  ximax,      skindepth_for_bca
      integer*4 transport_algorithm, bca_algorithm
      logical exact_bca,  spin_effects
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      common/x_options/eadl_relax,  mcdf_pe_xsections
      logical eadl_relax, mcdf_pe_xsections
      common/egs_vr/ e_max_rr(1),  prob_RR,  nbr_split,  i_play_RR,
     * i_survived_RR,
     *     n_RR_warning,                                        i_do_rr(
     *1)
      real*8          e_max_rr,prob_RR
      integer*4       nbr_split,i_play_RR,i_survived_RR,n_RR_warning
      integer*2     i_do_rr
      COMMON/LBREMZ/CONST,DELC,EBREMZ,DELTAM,XLNZ
      real*4 CONST,DELC,EBREMZ,DELTAM,XLNZ
      COMMON/PMCONS/PIP,C,RME,HBAR,ECGS,EMKS,AN
      real*4 PIP,C,RME,HBAR,ECGS,EMKS,AN
      COMMON/DERCON/RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      real*4 RADDEG,FSC,ERGMEV,R0,RMP,RMPT2,RMPSQ,A22P9,A6680
      COMMON/EPSTAR/EPSTEN(150),EPSTD(150),WEPST(20), EPSTTL,NEPST,IEPST
     *,EPSTFLP, NELEPS,ZEPST(20),IAPRFL,IAPRIMP
      integer*4 ZEPST,NELEPS,IAPRFL,NEPST,IEPST,EPSTFLP,IAPRIMP
      CHARACTER EPSTTL*80
      real*4 EPSTEN,EPSTD,WEPST
      COMMON/MOLVAR/WM,ZC,ZT,ZB,ZF,ZS,ZE,ZX,ZA,ZG,ZP,ZV,ZU, RLCP,EDEN,RH
     *OP,XCCP,BLCCP,TEFF0P,XR0P
      real*4 WM,ZC,ZT,ZB,ZF,ZS,ZE,ZX,ZA,ZG,ZP,ZV,ZU,RLCP,EDEN,RHOP, XCCP
     *,BLCCP,TEFF0P,XR0P
      COMMON/LSPION/CBAR,X0,X1,SK,TOLN10,AFACT,SPC1,SPC2,IEV
      real*4 CBAR,X0,X1,SK,TOLN10,AFACT,SPC1,SPC2,IEV
      COMMON/PWLFIN/EPE,ZTHRE(8),ZEPE(8),NIPE,NALE
      real*4 EPE,ZTHRE,ZEPE
      integer*4 NIPE,NALE
      COMMON/RSLTS/NEL,AXE,BXE,AFE(500,8),BFE(500,8)
      real*4 AXE,BXE,AFE,BFE
      integer*4 NEL
      COMMON/SPCOMM/MEDTBL(24,73), NUMSTMED,STDATA(6,73)
      CHARACTER*4 MEDTBL
      integer*4 NUMSTMED
      real*4 STDATA
      COMMON/MIXDAT/NEP,LMED,PZP(50),ZELEMP(50),WAP(50),RHOZP(50), GASPP
     *,EZ,TPZ,IDSTRN(24)
      integer*4 NEP,LMED
      real*4 PZP,ZELEMP,WAP,RHOZP,GASPP,EZ,TPZ
      CHARACTER*4 IDSTRN
      COMMON/ADLEN/ALRAD(4),ALRADP(4),A1440,A183
      real*4 ALRAD,ALRADP,A1440,A183
      COMMON/MIMSD/BMIN
      real*4 BMIN
      COMMON/THRESHP/APP,AEP,UPP,UEP,THBREMP,THMOLLP,TEP,IUNRSTP
      real*4 APP,AEP,UPP,UEP,THBREMP,THMOLLP,TEP
      integer*4 IUNRSTP
      COMMON/BREMPRP/DLP1(6),DLP2(6),DLP3(6),DLP4(6),DLP5(6),DLP6(6), DE
     *LCMP,ALPHIP(2),BPARP(2),DELPOSP(2)
      real*4 dlP1,dlP2,dlP3,dlP4,dlP5,dlP6,delcmP,alphiP,bparP,delposP
      COMMON/ELEMTB/NET,ITBL(100),WATBL(100),RHOTBL(100),ASYMT(100)
      integer*4 NET
      real*4 ITBL,WATBL,RHOTBL
      CHARACTER*4 ASYMT
      COMMON/MEDINP/inpdensity_file(1),inpasym(1,50), inpstrn(24,1),pz4(
     *1,50), rhoz4(1,50),wa4(1,50),inpgasp(1)
      character*256 inpdensity_file
      CHARACTER*4 inpasym,inpstrn
      real*4 pz4,rhoz4,wa4,inpgasp
      real*4 XSIFP,WADUM,PZDUM,RHOZDUM,RLCDUM,ALKE,ALKEI
      integer*4 I01
      EXTERNAL ALKE,ALKEI,EFUNS
      CHARACTER*4 MEDTB1(24,20),MEDTB2(24,20),MEDTB3(24,20),MEDTB4(24,13
     *)
      EQUIVALENCE (MEDTBL(1,1),MEDTB1(1,1))
      EQUIVALENCE (MEDTBL(1,21),MEDTB2(1,1))
      EQUIVALENCE (MEDTBL(1,41),MEDTB3(1,1))
      EQUIVALENCE (MEDTBL(1,61),MEDTB4(1,1))
      real*4 STDAT1(6,20),STDAT2(6,20),STDAT3(6,20),STDAT4(6,13)
      EQUIVALENCE (STDATA(1,1),STDAT1(1,1))
      EQUIVALENCE (STDATA(1,21),STDAT2(1,1))
      EQUIVALENCE (STDATA(1,41),STDAT3(1,1))
      EQUIVALENCE (STDATA(1,61),STDAT4(1,1))
      CHARACTER*4 MBUF(72),MDLABL(8)
      real*8 ACD ,  ADEV ,  ASD ,  COST ,  CTHET ,  DEL ,  DFACT ,  DFAC
     *TI,  DUNITO,  DUNITR,  FNSSS ,  P ,  PZNORM,  RDEV ,  S2C2 ,  S2C2
     *MN,  S2C2MX,  SINT ,  SX ,  SXX ,  SXY ,   SY ,   WID ,  XS ,  XS0
     * ,  XS1 ,  XSI ,  WSS ,  YS ,  ZEROS(3)
      integer*4 I ,  I1ST ,  IB ,  ID ,  IE ,  IL ,  IM ,  IRAYL ,  IRN
     *,  ISTEST,  ISUB ,  ISS ,  IZ ,   IZZ ,  J ,  JR ,  LCTHET,  LMDL
     *,  LMDN ,  LTHETA,  MD ,  MXSINC,  NCMFP ,   NEKE ,   NGE ,   NGRI
     *M ,  NISUB ,  NLEKE ,    NM ,  NRANGE,    NRNA ,  NSEKE ,   NSGE ,
     *   NSINSS,  LOK(1)
      character*256 tmp_string
      integer*4 lnblnk1
      DATA MDLABL/' ','M','E','D','I','U','M','='/,LMDL/8/,LMDN/24/,DUNI
     *TO/1./
      DATA I1ST/1/,NSINSS/37/,MXSINC/1002/,ISTEST/0/,NRNA/1000/
      PIP=3.1415926536
      C=2.997925E+10
      HBAR=1.05450E-27
      ECGS=4.80298E-10
      EMKS=1.60210E-19
      AN=6.02252E+23
      RADDEG=180./PIP
      FSC = ECGS**2/(HBAR*C)
      ERGMEV = (1.E+6)*(EMKS*1.E+7)
      RME = PRM/C**2*ERGMEV
      RMP = PRM
      R0 = (ECGS**2)/(RME*C**2)
      RMPSQ = RMP*RMP
      A22P9 = RADDEG*SQRT(4.*PIP*AN)*ECGS**2/ERGMEV
      A6680 = 4.0*PIP*AN*(HBAR/(RME*C))**2*(0.885**2/(1.167*1.13))
      DATA AFACT/0.0/,SK/0.0/,X0/0.0/,X1/0.0/,CBAR/0.0/,IEV/0.0/
      DATA LMED/24/,NUMSTMED/73/
      DATA EPE/.01/,ZTHRE,ZEPE/16*0.0/,NIPE/20/,NALE/500/
      DATA BMIN/4.5/
      DATA ALRAD/5.31,4.79,4.74,4.71/,ALRADP/6.144,5.621,5.805,5.924/, A
     *1440/1194.0/,A183/184.15/
      DATA MEDTB1/ 'H','2','-','G','A','S',18*' ','H','2','-','L','I','Q
     *','U','I','D',15*' ','H','E','-','G','A','S',18*' ','L','I',22*' '
     *, 'B','E',22*' ','C','-','2','.','2','6','5',' ','G','/','C','M','
     **','*','3',9*' ','C','-','1','.','7','0',' ','G','/','C','M','*','
     **','3',10*' ', 'N','2','-','G','A','S',18*' ','O','2','-','G','A',
     *'S',18*' ','N','E','-','G','A','S',18*' ','N','A',22*' ', 'M','G',
     *22*' ','A','L',22*' ','S','I',22*' ','A','R','-','G','A','S',18*'
     *', 'K',23*' ','C','A',22*' ','T','I',22*' ','V',23*' ','M','N',22*
     *' ' /
      DATA MEDTB2/ 'F','E',22*' ','C','O',22*' ','N','I',22*' ','C','U',
     *22*' ','Z','N',22*' ', 'G','E',22*' ','S','E',22*' ','K','R','-','
     *G','A','S',18*' ','R','B',22*' ', 'M','O',22*' ','A','G',22*' ','C
     *','D',22*' ','I','N',22*' ','S','N',22*' ', 'X','E','-','G','A','S
     *',18*' ','C','S',22*' ','G','D',22*' ','T','A',22*' ', 'W',23*' ',
     *'P','T',22*' ' /
      DATA MEDTB3/ 'A','U',22*' ','H','G',22*' ','P','B',22*' ','R','N',
     *'-','G','A','S',18*' ', 'U',23*' ', 'A','I','R','-','G','A','S',17
     **' ','C','O','2','-','G','A','S',17*' ','P','O','L','Y','E','T','H
     *','Y','L','E','N','E',12*' ', 'P','O','L','Y','P','R','O','P','Y',
     *'L','E','N','E',11*' ','X','Y','L','E','N','E',18*' ','T','O','L',
     *'U','E','N','E',17*' ', 'N','Y','L','O','N',19*' ','V','I','N','Y'
     *,'L','T','O','L','U','E','N','E',12*' ','A','1','5','0','-','P','L
     *','A','S','T','I','C',12*' ', 'S','T','I','L','B','E','N','E',16*'
     * ','P','O','L','Y','S','T','Y','R','E','N','E',13*' ','A','N','T',
     *'H','R','A','C','E','N','E',14*' ', 'L','E','X','A','N',19*' ','L'
     *,'U','C','I','T','E',18*' ','H','2','O',21*' ' /
      DATA MEDTB4/ 'M','Y','L','A','R',19*' ', 'K','A','P','T','O','N',1
     *8*' ','L','I','F',21*' ','P','O','L','Y','V','I','N','Y','L','-','
     *C','L',12*' ', 'P','Y','R','E','X','-','G','L','A','S','S',13*' ',
     *'S','I','O','2',20*' ','C','A','F','2',20*' ', 'P','H','O','T','O'
     *,'E','M','U','L','S','I','O','N',11*' ','A','G','C','L',20*' ','N'
     *,'A','I',21*' ', 'L','I','I',21*' ','A','G','B','R',20*' ','C','S'
     *,'I',21*' ' /
      DATA STDAT1/ 0.03535,6.790,1.864,3.5,19.2,9.584, 0.09179,5.831,0.4
     *76,2.0,21.8,3.263, 0.0114,7.625,2.202,4.0,41.8,11.139, 0.3492,3.23
     *3,0.0966,2.0,40.0,3.122, 0.3518,3.034,-0.0089,2.0,63.7,2.785, 0.58
     *48,2.360,-0.0089,2.0,78.0,2.868, 0.7154,2.191,-0.0089,2.0,78.0,3.1
     *55, 0.2120,3.041,1.738,4.0,82.0,10.540, 0.2666,2.825,1.754,4.0,95.
     *0,10.700, 0.1202,3.357,2.073,4.5,137.0,11.904, 0.2204,3.103,0.4515
     *,2.8,149.0,5.053, 0.1714,3.223,0.2386,2.8,156.0,4.530, 0.3346,2.79
     *5,0.0966,2.5,166.0,4.239, 0.3755,2.720,0.0966,2.5,173.0,4.435, 0.1
     *902,2.982,1.764,4.5,188.0,11.948, 0.3041,2.674,0.2386,3.0,190.0,5.
     *642, 0.2177,2.874,0.1751,3.0,191.0,5.040, 0.1782,2.946,0.0485,3.0,
     *233.0,4.445, 0.1737,2.935,-0.0089,3.0,245.0,4.266, 0.1996,2.812,-0
     *.0089,3.0,272.0,4.270 /
      DATA STDAT2/ 0.2101,2.771,-0.0089,3.0,286.0,4.291, 0.2229,2.713,-0
     *.0089,3.0,297.0,4.260, 0.2504,2.619,-0.0089,3.0,311.0,4.312, 0.255
     *7,2.613,-0.0089,3.0,322.0,4.419, 0.3163,2.468,0.0485,3.0,330.0,4.6
     *91, 0.2809,2.647,0.2386,3.0,350.0,5.141, 0.2979,2.635,0.2386,3.0,3
     *48.0,5.321, 0.1519,3.030,1.716,4.8,352.0,12.512, 0.1450,3.078,0.45
     *15,3.5,363.0,6.478, 0.2228,2.824,0.1751,3.0,424.0,4.879, 0.3091,2.
     *563,-0.0089,3.0,470.0,5.063, 0.1853,2.819,0.0485,3.3,469.0,5.273,
     *0.2004,2.790,0.1751,3.3,487.0,5.517, 0.1898,2.839,0.2386,3.3,488.0
     *,5.534, 0.1329,3.020,1.563,5.0,482.0,12.728, 0.2214,2.784,0.4515,3
     *.5,488.0,6.914, 0.2068,2.686,0.0485,3.5,591.0,5.874, 0.1663,2.805,
     *0.1751,3.5,718.0,5.526, 0.1499,2.870,0.1751,3.5,727.0,5.406, 0.146
     *5,2.903,0.0966,3.5,790.0,5.473 /
      DATA STDAT3/ 0.1533,2.881,0.0966,3.5,790.0,5.575, 0.1824,2.798,0.2
     *386,3.5,800.0,5.961, 0.1861,2.814,0.2386,3.5,823.0,6.202, 0.1130,3
     *.023,1.537,5.3,794.0,13.284, 0.1362,3.034,0.2386,3.5,890.0,5.869,
     *0.2466,2.879,1.742,4.0,85.7,10.595, 0.1999,3.022,1.648,4.0,88.7,10
     *.239, 0.4875,2.544,0.1379,2.0,57.4,3.002, 0.2493,2.975,0.1537,2.3,
     *59.2,3.126, 0.2755,2.911,0.1695,2.3,61.8,3.270, 0.2830,2.890,0.172
     *2,2.3,62.5,3.303, 0.5345,2.439,0.1336,2.0,63.9,3.063, 0.3495,2.749
     *,0.1467,2.2,64.7,3.201, 0.5462,2.435,0.1329,2.0,65.1,3.110, 0.2989
     *,2.851,0.1731,2.3,67.7,3.367, 0.3670,2.724,0.1647,2.2,68.7,3.300,
     *0.5858,2.364,0.1146,2.0,69.5,3.151, 0.3865,2.664,0.1608,2.2,73.1,3
     *.321, 0.3996,2.606,0.1824,2.2,74.0,3.330, 0.2065,3.007,0.2400,2.5,
     *75.0,3.502 /
      DATA STDAT4/ 0.3124,2.782,0.1561,2.3,78.7,3.326, 0.4061,2.614,0.14
     *92,2.2,79.3,3.342, 0.1308,3.476,0.0171,2.5,94.0,3.167, 0.1873,2.96
     *2,0.1558,2.8,108.2,4.053, 0.2988,2.805,0.1479,2.5,134.0,3.971, 0.1
     *440,3.220,0.1385,2.8,139.2,4.003, 0.3750,2.592,0.0676,2.5,166.0,4.
     *065, 0.3416,2.496,0.1009,3.0,331.0,5.332, 0.1243,3.002,-0.0138,3.5
     *,398.4,5.344, 0.1560,2.926,0.1203,3.5,452.0,6.057, 0.1785,2.845,0.
     *0892,3.5,485.1,6.267, 0.1351,2.976,0.0358,3.5,487.2,5.616, 0.1796,
     *2.840,0.0395,3.5,553.1,6.281 /
      DATA NET/100/
      DATA ITBL/19.2,41.8,40.,63.7,76.0,78.0,82.0,95.0,115.,137., 149.,1
     *56.,166.,173.,173.,180.,174.,188.,190.,191.,216.,233.,245., 257.,2
     *72.,286.,297.,311.,322.,330.,334.,350.,347.,348.,357.,352., 363.,3
     *66.,379.,393.,417.,424.,428.,441.,449.,470.,470.,469.,488., 488.,4
     *87.,485.,491.,482.,488.,491.,501.,523.,535.,546.,560.,574., 580.,5
     *91.,614.,628.,650.,658.,674.,684.,694.,705.,718.,727.,736., 746.,7
     *57.,790.,790.,800.,810.,823.,823.,830.,825.,794.,827.,826., 841.,8
     *47.,878.,890.,902.,921.,934.,939.,952.,966.,980.,994./
      DATA WATBL/1.00797,4.0026,6.939,9.0122,10.811,12.01115,14.0067, 15
     *.9994,18.9984,20.183,22.9898,24.312,26.9815,28.088,30.9738, 32.064
     *,35.453,39.948,39.102,40.08,44.956,47.90,50.942,51.998, 54.9380,55
     *.847,58.9332,58.71,63.54,65.37,69.72,72.59,74.9216, 78.96,79.808,8
     *3.80,85.47,87.62,88.905,91.22,92.906,95.94,99.0, 101.07,102.905,10
     *6.4,107.87,112.4,114.82,118.69,121.75,127.60, 126.9044,131.30,132.
     *905,137.34,138.91, 140.12,140.907,144.24,147.,150.35,151.98,157.25
     *,158.924,162.50, 164.930,167.26,168.934,173.04,174.97,178.49,180.9
     *48,183.85, 186.2,190.2,192.2,195.08,196.987,200.59,204.37,207.19,2
     *08.980, 210.,210.,222.,223.,226.,227.,232.036,231.,238.03,237.,242
     *., 243.,247.,247.,248.,254.,253./
      DATA RHOTBL/0.0808,0.19,0.534,1.85,2.5,2.26,1.14,1.568,1.5,1.0, 0.
     *9712,1.74,2.702,2.4,1.82,2.07,2.2,1.65,0.86,1.55,3.02,4.54, 5.87,7
     *.14,7.3,7.86,8.71,8.90,8.9333,7.140,5.91,5.36,5.73,4.80, 4.2,3.4,1
     *.53,2.6,4.47,6.4,8.57,9.01,11.50,12.20,12.50,12.,10.5, 8.65,7.30,7
     *.31,6.684,6.24,4.93,2.7,1.873,3.5,6.15,6.90,6.769, 7.007, 1. ,7.54
     *,5.17,7.87,8.25,8.56,8.80,9.06,9.32,6.96,9.85, 11.40,16.60,19.30,2
     *0.53,22.48,22.42,21.45,19.30,14.19,11.85, 11.34,9.78,9.30, 1. ,4.,
     * 1. ,5., 1. ,11.0,15.37,18.90, 20.5,19.737,11.7,7.,1. , 1. , 1. ,
     *1. /
      DATA ASYMT/'H','HE','LI','BE','B','C','N','O','F','NE', 'NA','MG',
     *'AL','SI','P','S','CL','AR','K','CA','SC','TI', 'V','CR','MN','FE'
     *,'CO','NI','CU','ZN','GA','GE','AS','SE','BR', 'KR','RB','SR','Y',
     *'ZR','NB','MO','TC','RU','RH','PD','AG','CD', 'IN','SN','SB','TE',
     *'I','XE','CS','BA','LA','CE','PR','ND', 'PM','SM','EU','GD','TB','
     *DY','HO','ER','TM','YB','LU','HF','TA', 'W','RE','OS','IR','PT','A
     *U','HG','TL','PB','BI','PO','AT','RN', 'FR','RA','AC','TH','PA','U
     *','NP','PU','AM','CM','BK','CF','ES', 'FM'/
      DATA EPSTFLP/0/,IEPST/1/,IAPRIMP/1/,IAPRFL/0/
6610  FORMAT(1X,14I5)
6620  FORMAT(1X,1PE14.5,4E14.5)
6630  FORMAT(72A1)
      IF ((I1ST.NE.0)) THEN
        I1ST=0
        DO 6641 J=1,1
          IF ((SMAXIR(J).LE.0.0)) THEN
            SMAXIR(J)=1E10
          END IF
6641    CONTINUE
6642    CONTINUE
        NISUB=MXSINC-2
        FNSSS=NSINSS
        WID=PI5D2/FLOAT(NISUB)
        WSS=WID/(FNSSS-1.0)
        ZEROS(1)=0.
        ZEROS(2)=PI
        ZEROS(3)=TWOPI
        DO 6651 ISUB=1,MXSINC
          SX=0.
          SY=0.
          SXX=0.
          SXY=0.
          XS0=WID*FLOAT(ISUB-2)
          XS1=XS0+WID
          IZ=0
          DO 6661 IZZ=1,3
            IF (((XS0.LE.ZEROS(IZZ)).AND.(ZEROS(IZZ).LE.XS1))) THEN
              IZ=IZZ
              GO TO6662
            END IF
6661      CONTINUE
6662      CONTINUE
          IF ((IZ.EQ.0)) THEN
            XSI=XS0
          ELSE
            XSI=ZEROS(IZ)
          END IF
          DO 6671 ISS=1,NSINSS
            XS=WID*FLOAT(ISUB-2)+WSS*FLOAT(ISS-1)-XSI
            YS=SIN(XS+XSI)
            SX=SX+XS
            SY=SY+YS
            SXX=SXX+XS*XS
            SXY=SXY+XS*YS
6671      CONTINUE
6672      CONTINUE
          IF ((IZ.NE.0)) THEN
            SIN1(ISUB)=SXY/SXX
            SIN0(ISUB)=-SIN1(ISUB)*XSI
          ELSE
            DEL=FNSSS*SXX-SX*SX
            SIN1(ISUB)=(FNSSS*SXY-SY*SX)/DEL
            SIN0(ISUB)=(SY*SXX-SX*SXY)/DEL - SIN1(ISUB)*XSI
          END IF
6651    CONTINUE
6652    CONTINUE
        SINC0=2.0
        SINC1=1.0/WID
        IF ((ISTEST.NE.0)) THEN
          ADEV=0.
          RDEV=0.
          S2C2MN=10.
          S2C2MX=0.
          DO 6681 ISUB=1,NISUB
            DO 6691 ISS=1,NSINSS
              THETA=WID*FLOAT(ISUB-1)+WSS*FLOAT(ISS-1)
              CTHET=PI5D2-THETA
              SINTHE=sin(THETA)
              COSTHE=sin(CTHET)
              SINT=SIN(THETA)
              COST=COS(THETA)
              ASD=ABS(SINTHE-SINT)
              ACD=ABS(COSTHE-COST)
              ADEV=max(ADEV,ASD,ACD)
              IF((SINT.NE.0.0))RDEV=max(RDEV,ASD/ABS(SINT))
              IF((COST.NE.0.0))RDEV=max(RDEV,ACD/ABS(COST))
              S2C2=SINTHE**2+COSTHE**2
              S2C2MN=min(S2C2MN,S2C2)
              S2C2MX=max(S2C2MX,S2C2)
              IF ((ISUB.LT.11)) THEN
                write(i_log,'(1PE20.7,4E20.7)') THETA,SINTHE,SINT,COSTHE
     *          ,COST
              END IF
6691        CONTINUE
6692        CONTINUE
6681      CONTINUE
6682      CONTINUE
          write(i_log,'(a,2i5)') ' SINE TESTS,MXSINC,NSINSS=',MXSINC,NSI
     *    NSS
          write(i_log,'(a,1PE16.8,3e16.8)') ' ADEV,RDEV,S2C2(MN,MX) =',
     *    ADEV,RDEV,S2C2MN,S2C2MX
          ADEV=0.
          RDEV=0.
          S2C2MN=10.
          S2C2MX=0.
          DO 6701 IRN=1,NRNA
            IF((rng_seed .GT. 128))call ranmar_get
            THETA = rng_array(rng_seed)*twom24
            rng_seed = rng_seed + 1
            THETA=THETA*PI5D2
            CTHET=PI5D2-THETA
            SINTHE=sin(THETA)
            COSTHE=sin(CTHET)
            SINT=SIN(THETA)
            COST=COS(THETA)
            ASD=ABS(SINTHE-SINT)
            ACD=ABS(COSTHE-COST)
            ADEV=max(ADEV,ASD,ACD)
            IF((SINT.NE.0.0))RDEV=max(RDEV,ASD/ABS(SINT))
            IF((COST.NE.0.0))RDEV=max(RDEV,ACD/ABS(COST))
            S2C2=SINTHE**2+COSTHE**2
            S2C2MN=min(S2C2MN,S2C2)
            S2C2MX=max(S2C2MX,S2C2)
6701      CONTINUE
6702      CONTINUE
          write(i_log,'(a,i7,a)') ' TEST AT ',NRNA,' RANDOM ANGLES IN (0
     *,5*PI/2)'
          write(i_log,'(1PE16.8,3E16.8)') ' ADEV,RDEV,S2C2(MN,MX) =', AD
     *    EV,RDEV,S2C2MN,S2C2MX
        END IF
        P=1.
        DO 6711 I=1,50
          PWR2I(I)=P
          P=P/2.
6711    CONTINUE
6712    CONTINUE
      END IF
      DO 6721 J=1,NMED
6730    CONTINUE
          DO 6731 I=1,1
          IF ((IRAYLR(I).EQ.1.AND.MED(I).EQ.J)) THEN
            IRAYLM(J)=1
            GO TO 6732
          END IF
6731    CONTINUE
6732    CONTINUE
6721  CONTINUE
6722  CONTINUE
      IPHOTONUC=0
      DO 6741 J=1,NMED
6750    CONTINUE
          DO 6751 I=1,1
          IF ((IPHOTONUCR(I).EQ.1.AND.MED(I).EQ.J)) THEN
            IPHOTONUCM(J)=1
            IPHOTONUC=1
            GO TO 6752
          END IF
6751    CONTINUE
6752    CONTINUE
6741  CONTINUE
6742  CONTINUE
      write(i_log,'(a,i3)') ' ===> Photonuclear flag: ', iphotonuc
      IF((.NOT.is_pegsless))REWIND KMPI
      NM=0
      DO 6761 IM=1,NMED
        LOK(IM)=0
        IF ((IRAYLM(IM).EQ.1)) THEN
          write(i_log,'(a,i3/)') ' RAYLEIGH OPTION REQUESTED FOR MEDIUM
     *NUMBER',IM
        END IF
6761  CONTINUE
6762  CONTINUE
      DO 6771 IM=1,NMED
        IF ((IPHOTONUCM(IM).EQ.1)) THEN
          write(i_log,'(a,i3/)') ' PHOTONUCLEAR REQUESTED FOR MEDIUM NUM
     *BER',IM
        END IF
6771  CONTINUE
6772  CONTINUE
      IF ((.NOT.is_pegsless)) THEN
6780    CONTINUE
6781      CONTINUE
6790      CONTINUE
6791        CONTINUE
            READ(KMPI,6630,END=6800)MBUF
            DO 6811 IB=1,LMDL
              IF((MBUF(IB).NE.MDLABL(IB)))GO TO 6791
6811        CONTINUE
6812        CONTINUE
6820        CONTINUE
              DO 6821 IM=1,NMED
              DO 6831 IB=1,LMDN
                IL=LMDL+IB
                IF((MBUF(IL).NE.MEDIA(IB,IM)))GO TO 6821
                IF((IB.EQ.LMDN))GO TO 6792
6831          CONTINUE
6832          CONTINUE
6821        CONTINUE
6822        CONTINUE
          GO TO 6791
6792      CONTINUE
          IF((LOK(IM).NE.0))GO TO 6790
          LOK(IM)=1
          NM=NM+1
          read(kmpi,'(a)',err=6840) tmp_string
          goto 6850
6840      write(i_log,'(/a)') '***************** Error: '
          write(i_log,*) 'Error while reading pegs4 file'
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
6850      CONTINUE
          read(tmp_string,1,ERR=6860)  (MBUF(I),I=1,5),RHO(IM),NNE(IM),I
     *    UNRST(IM),EPSTFL(IM),IAPRIM(IM)
1         FORMAT(5A1,5X,F11.0,4X,I2,9X,I1,9X,I1,9X,I1)
          GO TO 6870
6860      CONTINUE
          write(i_log,*) 'Found medium with gas pressure'
          read(tmp_string,2) (MBUF(I),I=1,5),RHO(IM),NNE(IM),IUNRST(IM),
     *    EPSTFL(IM), IAPRIM(IM)
2         FORMAT(5A1,5X,F11.0,4X,I2,26X,I1,9X,I1,9X,I1)
6870      CONTINUE
            DO 6871 IE=1,NNE(IM)
            READ(KMPI,6880)(MBUF(I),I=1,6),(ASYM(IM,IE,I),I=1,2), ZELEM(
     *      IM,IE),WA(IM,IE),PZ(IM,IE),RHOZ(IM,IE)
6880        FORMAT (6A1,2A1,3X,F3.0,3X,F9.0,4X,F12.0,6X,F12.0)
6871      CONTINUE
6872      CONTINUE
          READ(KMPI,6620) RLC(IM),AE(IM),AP(IM),UE(IM),UP(IM)
          TE(IM)=AE(IM)-RM
          THMOLL(IM)=TE(IM)*2. + RM
          READ(KMPI,6610) MSGE(IM),MGE(IM),MSEKE(IM),MEKE(IM),MLEKE(IM),
     *    MCMFP(IM),MRANGE(IM),IRAYL
          NSGE=MSGE(IM)
          NGE=MGE(IM)
          NSEKE=MSEKE(IM)
          NEKE=MEKE(IM)
          NLEKE=MLEKE(IM)
          NCMFP=MCMFP(IM)
          NRANGE=MRANGE(IM)
          READ(KMPI,6620)(DL1(I,IM),DL2(I,IM),DL3(I,IM),DL4(I,IM),DL5(I,
     *    IM),DL6(I,IM),I=1,6)
          READ(KMPI,6620)DELCM(IM),(ALPHI(I,IM),BPAR(I,IM),DELPOS(I,IM),
     *    I=1,2)
          READ(KMPI,6620)XR0(IM),TEFF0(IM),BLCC(IM),XCC(IM)
          READ(KMPI,6620)EKE0(IM),EKE1(IM)
          READ(KMPI,6620) (ESIG0(I,IM),ESIG1(I,IM),PSIG0(I,IM),PSIG1(I,I
     *    M),EDEDX0(I,IM),EDEDX1(I,IM),PDEDX0(I,IM),PDEDX1(I,IM),EBR10(I
     *    ,IM),EBR11(I,IM),PBR10(I,IM),PBR11(I,IM),PBR20(I,IM),PBR21(I,I
     *    M),TMXS0(I,IM),TMXS1(I,IM),I=1,NEKE)
          READ(KMPI,6620)EBINDA(IM),GE0(IM),GE1(IM)
          READ(KMPI,6620)(GMFP0(I,IM),GMFP1(I,IM),GBR10(I,IM),GBR11(I,IM
     *    ),GBR20(I,IM),GBR21(I,IM),I=1,NGE)
          IF ((IRAYL.EQ.1)) THEN
            READ(KMPI,6610) NGR(IM)
            NGRIM=NGR(IM)
            READ(KMPI,6620)RCO0(IM),RCO1(IM)
            READ(KMPI,6620)(RSCT0(I,IM),RSCT1(I,IM),I=1,NGRIM)
            READ(KMPI,6620)(COHE0(I,IM),COHE1(I,IM),I=1,NGE)
            write(i_log,'(a,i3,a)') ' Rayleigh data available for medium
     *', IM, ' in PEGS4 data set.'
          END IF
          IF ((IRAYLM(IM).EQ.1)) THEN
            IF ((IRAYL.NE.1)) THEN
              IF ((toUpper(photon_xsections(:lnblnk1(photon_xsections)))
     *        .EQ.'PEGS4')) THEN
                write(i_log,'(/a)') '***************** Error: '
                write(i_log,'(a,i3 /,a /,a)') ' IN HATCH: REQUESTED RAYL
     *EIGH OPTION FOR MEDIUM', IM,' BUT RAYLEIGH DATA NOT INCLUDED IN PE
     *GS4 FILE.', ' YOU WILL NOT BE ABLE TO USE THE PEGS4 DATA WITH RAYL
     *EIGH ON!'
                write(i_log,'(/a)') '***************** Quiting now.'
                call exit(1)
              ELSE
                write(i_log,'(/a)') '***************** Warning: '
                write(i_log,'(a,i3 /,a)') ' IN HATCH: REQUESTED RAYLEIGH
     * OPTION FOR MEDIUM', IM,' BUT RAYLEIGH DATA NOT INCLUDED IN PEGS4
     *FILE.'
              END IF
            ELSE
              IF ((toUpper(photon_xsections(:lnblnk1(photon_xsections)))
     *        .EQ.'PEGS4')) THEN
                call egs_init_rayleigh_sampling(IM)
              END IF
            END IF
          END IF
          IF((NM.GE.NMED))GO TO6782
        GO TO 6781
6782    CONTINUE
        CLOSE (UNIT=KMPI)
        DUNITR=DUNIT
        IF ((DUNIT.LT.0.0)) THEN
          ID=MAX0(1,MIN0(1,int(-DUNIT)))
          DUNIT=RLC(ID)
        END IF
        IF ((DUNIT.NE.1.0)) THEN
          write(i_log,'(a,1PE14.5,E14.5,a)') ' DUNIT REQUESTED&USED ARE:
     * ', DUNITR,DUNIT,'(CM.)'
        END IF
        DO 6891 IM=1,NMED
          DFACT=RLC(IM)/DUNIT
          DFACTI=1.0/DFACT
          I=1
            GO TO 6903
6901        I=I+1
6903        IF(I-(MEKE(IM)).GT.0)GO TO 6902
            ESIG0(I,IM)=ESIG0(I,IM)*DFACTI
            ESIG1(I,IM)=ESIG1(I,IM)*DFACTI
            PSIG0(I,IM)=PSIG0(I,IM)*DFACTI
            PSIG1(I,IM)=PSIG1(I,IM)*DFACTI
            EDEDX0(I,IM)=EDEDX0(I,IM)*DFACTI
            EDEDX1(I,IM)=EDEDX1(I,IM)*DFACTI
            PDEDX0(I,IM)=PDEDX0(I,IM)*DFACTI
            PDEDX1(I,IM)=PDEDX1(I,IM)*DFACTI
            TMXS0(I,IM)=TMXS0(I,IM)*DFACT
            TMXS1(I,IM)=TMXS1(I,IM)*DFACT
          GO TO 6901
6902      CONTINUE
          TEFF0(IM)=TEFF0(IM)*DFACT
          BLCC(IM)=BLCC(IM)*DFACTI
          XCC(IM)=XCC(IM)*SQRT(DFACTI)
          RLDU(IM)=RLC(IM)/DUNIT
          I=1
            GO TO 6913
6911        I=I+1
6913        IF(I-(MGE(IM)).GT.0)GO TO 6912
            GMFP0(I,IM)=GMFP0(I,IM)*DFACT
            GMFP1(I,IM)=GMFP1(I,IM)*DFACT
          GO TO 6911
6912      CONTINUE
6891    CONTINUE
6892    CONTINUE
        VACDST=VACDST*DUNITO/DUNIT
        DUNITO=DUNIT
      ELSE
        write(i_log,*) ' PEGSLESS INPUT.  CALCULATING ELECTRON CROSS-SEC
     *TIONS.'
        call get_media_inputs(-1)
        DO 6921 IM=1,NMED
          AEP=AE(IM)
          UEP=UE(IM)
          APP=AP(IM)
          UPP=UP(IM)
          NEP=NNE(IM)
          IUNRSTP=IUNRST(IM)
          IAPRIMP=IAPRIM(IM)
          EPSTFLP=EPSTFL(IM)
          GASPP=INPGASP(IM)
          RHOP=RHO(IM)
          DO 6931 J=1,NEP
            ZELEMP(J)=ZELEM(IM,J)
            PZP(J)=PZ4(IM,J)
            RHOZP(J)=RHOZ4(IM,J)
            WAP(J)=WA4(IM,J)
6931      CONTINUE
6932      CONTINUE
          DO 6941 IB=1,LMDN
            IDSTRN(IB)=INPSTRN(IB,IM)
6941      CONTINUE
6942      CONTINUE
          TEP=AEP-RMP
          THMOLLP=AEP+TEP
          IF ((UEP.LE.AEP)) THEN
            write(i_log,'(a,24a1)')'  Error: Material not defined: ', (m
     *      edia(j,IM),j=1,24)
            write(i_log,'(/a)') '***************** Error: '
            write(i_log,*) 'Material used in the geometry was not define
     *d in the' ,' material data.'
            write(i_log,'(/a)') '***************** Quiting now.'
            call exit(1)
          END IF
          CALL MIX
          CALL SPINIT(inpdensity_file(IM))
          CALL DIFFER
          CALL PWLF1(NEL,NALE,AEP,UEP,THMOLLP,EPE,ZTHRE,ZEPE,NIPE,ALKE,
     *    ALKEI,AXE,BXE,500,8,AFE,BFE,EFUNS)
          TE(IM)=AE(IM)-RM
          THMOLL(IM)=TE(IM)*2. + RM
          RLC(IM)=RLCP
          XCC(IM)=XCCP
          BLCC(IM)=BLCCP
          XR0(IM)=XR0P
          TEFF0(IM)=TEFF0P
          DELCM(IM)=DELCMP
          DO 6951 I=1,2
            ALPHI(I,IM)=ALPHIP(I)
            BPAR(I,IM)=BPARP(I)
            DELPOS(I,IM)=DELPOSP(I)
6951      CONTINUE
6952      CONTINUE
          DO 6961 I=1,6
            DL1(I,IM)=DLP1(I)
            DL2(I,IM)=DLP2(I)
            DL3(I,IM)=DLP3(I)
            DL4(I,IM)=DLP4(I)
            DL5(I,IM)=DLP5(I)
            DL6(I,IM)=DLP6(I)
6961      CONTINUE
6962      CONTINUE
          MSGE(IM)=0
          MSEKE(IM)=0
          MLEKE(IM)=0
          MCMFP(IM)=0
          MRANGE(IM)=0
          MGE(IM)=2000
          MEKE(IM)=NEL
          NSGE=MSGE(IM)
          NGE=MGE(IM)
          NSEKE=MSEKE(IM)
          NEKE=MEKE(IM)
          NLEKE=MLEKE(IM)
          NCMFP=MCMFP(IM)
          NRANGE=MRANGE(IM)
          EKE0(IM)=BXE
          EKE1(IM)=AXE
          DO 6971 I=1,NEKE
            ESIG0(I,IM)=BFE(I,1)
            ESIG1(I,IM)=AFE(I,1)
            PSIG0(I,IM)=BFE(I,2)
            PSIG1(I,IM)=AFE(I,2)
            EDEDX0(I,IM)=BFE(I,3)
            EDEDX1(I,IM)=AFE(I,3)
            PDEDX0(I,IM)=BFE(I,4)
            PDEDX1(I,IM)=AFE(I,4)
            EBR10(I,IM)=BFE(I,5)
            EBR11(I,IM)=AFE(I,5)
            PBR10(I,IM)=BFE(I,6)
            PBR11(I,IM)=AFE(I,6)
            PBR20(I,IM)=BFE(I,7)
            PBR21(I,IM)=AFE(I,7)
            TMXS0(I,IM)=BFE(I,8)
            TMXS1(I,IM)=AFE(I,8)
6971      CONTINUE
6972      CONTINUE
6921    CONTINUE
6922    CONTINUE
        DUNITR=DUNIT
        IF ((DUNIT.LT.0.0)) THEN
          ID=MAX0(1,MIN0(1,int(-DUNIT)))
          DUNIT=RLC(ID)
        END IF
        IF ((DUNIT.NE.1.0)) THEN
          write(i_log,'(a,1PE14.5,E14.5,a)') ' DUNIT REQUESTED&USED ARE:
     * ', DUNITR,DUNIT,'(CM.)'
        END IF
        DO 6981 IM=1,NMED
          DFACT=RLC(IM)/DUNIT
          DFACTI=1.0/DFACT
          I=1
            GO TO 6993
6991        I=I+1
6993        IF(I-(MEKE(IM)).GT.0)GO TO 6992
            ESIG0(I,IM)=ESIG0(I,IM)*DFACTI
            ESIG1(I,IM)=ESIG1(I,IM)*DFACTI
            PSIG0(I,IM)=PSIG0(I,IM)*DFACTI
            PSIG1(I,IM)=PSIG1(I,IM)*DFACTI
            EDEDX0(I,IM)=EDEDX0(I,IM)*DFACTI
            EDEDX1(I,IM)=EDEDX1(I,IM)*DFACTI
            PDEDX0(I,IM)=PDEDX0(I,IM)*DFACTI
            PDEDX1(I,IM)=PDEDX1(I,IM)*DFACTI
            TMXS0(I,IM)=TMXS0(I,IM)*DFACT
            TMXS1(I,IM)=TMXS1(I,IM)*DFACT
          GO TO 6991
6992      CONTINUE
          TEFF0(IM)=TEFF0(IM)*DFACT
          BLCC(IM)=BLCC(IM)*DFACTI
          XCC(IM)=XCC(IM)*SQRT(DFACTI)
          RLDU(IM)=RLC(IM)/DUNIT
          I=1
            GO TO 7003
7001        I=I+1
7003        IF(I-(MGE(IM)).GT.0)GO TO 7002
            GMFP0(I,IM)=GMFP0(I,IM)*DFACT
            GMFP1(I,IM)=GMFP1(I,IM)*DFACT
          GO TO 7001
7002      CONTINUE
6981    CONTINUE
6982    CONTINUE
        VACDST=VACDST*DUNITO/DUNIT
        DUNITO=DUNIT
        call show_media_parameters(i_log)
      END IF
      DO 7011 JR=1,1
        MD=MED(JR)
        IF (((MD.GE.1).AND.(MD.LE.NMED))) THEN
          ECUT(JR)=max(ECUT(JR),AE(MD))
          PCUT(JR)=max(PCUT(JR),AP(MD))
          IF ((RHOR(JR).EQ.0.0)) THEN
            RHOR(JR)=RHO(MD)
          END IF
        END IF
7011  CONTINUE
7012  CONTINUE
      IF ((IBRDST.EQ.1)) THEN
        DO 7021 IM=1,NMED
          ZBRANG(IM)=0.0
          PZNORM=0.0
          DO 7031 IE=1,NNE(IM)
            ZBRANG(IM)= ZBRANG(IM)+PZ(IM,IE)*ZELEM(IM,IE)*(ZELEM(IM,IE)+
     *      1.0)
            PZNORM=PZNORM+PZ(IM,IE)
7031      CONTINUE
7032      CONTINUE
          ZBRANG(IM)=(8.116224E-05)*(ZBRANG(IM)/PZNORM)**(1./3.)
          LZBRANG(IM)=-log(ZBRANG(IM))
7021    CONTINUE
7022    CONTINUE
      END IF
      IF ((IPRDST.GT.0)) THEN
        DO 7041 IM=1,NMED
          ZBRANG(IM)=0.0
          PZNORM=0.0
          DO 7051 IE=1,NNE(IM)
            ZBRANG(IM)= ZBRANG(IM)+PZ(IM,IE)*ZELEM(IM,IE)*(ZELEM(IM,IE)+
     *      1.0)
            PZNORM=PZNORM+PZ(IM,IE)
7051      CONTINUE
7052      CONTINUE
          ZBRANG(IM)=(8.116224E-05)*(ZBRANG(IM)/PZNORM)**(1./3.)
7041    CONTINUE
7042    CONTINUE
      END IF
      IF ((toUpper(photon_xsections(:lnblnk1(photon_xsections))) .EQ. 'P
     *EGS4')) THEN
        write(i_log,'(/a)') '***************** Warning: '
        write(i_log,'(6(a/))') 'Using photon data from PEGS4 file!!!', '
     *However, the new Rayleigh angular sampling will be used.', 'The or
     *iginal EGS4 angular sampling undersamples large scattering ', 'ang
     *les. This may have little impact as Rayleigh scattering ', 'is for
     *ward peaked.', '**************************************************
     ********'
      ELSE
        call egs_init_user_photon(photon_xsections,comp_xsections, photo
     *  nuc_xsections,xsec_out)
      END IF
      call mscati
      IF (( eadl_relax .AND. photon_xsections .EQ. 'xcom' )) THEN
        call init_compton
        call EDGSET(1,1)
      ELSE
        call EDGSET(1,1)
        call init_compton
      END IF
      IF (( xsec_out .EQ. 1 .AND. eadl_relax)) THEN
        call egs_print_binding_energies
      END IF
      call fix_brems
      IF (( ibr_nist .GE. 1 )) THEN
        call init_nist_brems
      END IF
      IF (( pair_nrc .EQ. 1 )) THEN
        call init_nrc_pair
      END IF
      call eii_init
      call init_triplet
      IF ((NMED.EQ.1)) THEN
        write(i_log,*) 'EGSnrc SUCCESSFULLY ''HATCHED'' FOR ONE MEDIUM.'
      ELSE
        write(i_log,'(a,i5,a)') 'EGSnrc SUCCESSFULLY ''HATCHED'' FOR ',N
     *  MED,' MEDIA.'
      END IF
      RETURN
6800  write(i_log,'(a,i2//,a/,a/)') ' END OF FILE ON UNIT ',KMPI, ' PROG
     *RAM STOPPED IN HATCH BECAUSE THE', ' FOLLOWING NAMES WERE NOT RECO
     *GNIZED:'
      DO 7061 IM=1,NMED
        IF ((LOK(IM).NE.1)) THEN
          write(i_log,'(40x,a,24a1,a)') '''',(MEDIA(I,IM),I=1,LMDN),''''
        END IF
7061  CONTINUE
7062  CONTINUE
      STOP
      END
      subroutine fix_brems
      implicit none
      COMMON/BREMPR/ DL1(8,1),DL2(8,1),DL3(8,1),DL4(8,1),DL5(8,1),DL6(8,
     *1), ALPHI(2,1),BPAR(2,1),DELPOS(2,1), WA(1,50),PZ(1,50),ZELEM(1,50
     *),RHOZ(1,50), PWR2I(50), DELCM(1),ZBRANG(1),LZBRANG(1),NNE(1), IBR
     *DST,IPRDST,ibr_nist,pair_nrc,itriplet, ASYM(1,50,2)
      CHARACTER*4 ASYM
      real*8 DL1,DL2,DL3,DL4,DL5,DL6,   ALPHI,  BPAR,  DELPOS,  WA,  PZ,
     *  ZELEM,  RHOZ,  PWR2I,  DELCM,  ZBRANG,  LZBRANG
      integer*4 NNE,  IBRDST,  IPRDST,  ibr_nist,  itriplet,  pair_nrc
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      common/nist_brems/ nb_fdata(0:50,100,1), nb_xdata(0:50,100,1), nb_
     *wdata(50,100,1), nb_idata(50,100,1), nb_emin(1),nb_emax(1), nb_lem
     *in(1),nb_lemax(1), nb_dle(1),nb_dlei(1), log_ap(1)
      real*8 nb_fdata,nb_xdata,nb_wdata,nb_emin,nb_emax,nb_lemin,nb_lema
     *x, nb_dle,nb_dlei,log_ap
      integer*4 nb_idata
      COMMON/THRESH/RMT2,RMSQ, AP(1),AE(1),UP(1),UE(1),TE(1),THMOLL(1)
      real*8 RMT2,  RMSQ,  AP,  AE,  UP,  UE,  TE,  THMOLL
      integer*4 medium,i
      real*8 Zt,Zb,Zf,Zg,Zv,fmax1,fmax2,Zi,pi,fc,xi,aux, XSIF,FCOULC
      DO 7071 medium=1,nmed
        log_ap(medium) = log(ap(medium))
        Zt = 0
        Zb = 0
        Zf = 0
        DO 7081 i=1,NNE(medium)
          Zi = ZELEM(medium,i)
          pi = PZ(medium,i)
          fc = FCOULC(Zi)
          xi = XSIF(Zi)
          aux = pi*Zi*(Zi + xi)
          Zt = Zt + aux
          Zb = Zb - aux*Log(Zi)/3
          Zf = Zf + aux*fc
7081    CONTINUE
7082    CONTINUE
        Zv = (Zb - Zf)/Zt
        Zg = Zb/Zt
        fmax1 = 2*(20.863 + 4*Zg) - 2*(20.029 + 4*Zg)/3
        fmax2 = 2*(20.863 + 4*Zv) - 2*(20.029 + 4*Zv)/3
        dl1(1,medium) = (20.863 + 4*Zg)/fmax1
        dl2(1,medium) = -3.242/fmax1
        dl3(1,medium) = 0.625/fmax1
        dl4(1,medium) = (21.12+4*Zg)/fmax1
        dl5(1,medium) = -4.184/fmax1
        dl6(1,medium) = 0.952
        dl1(2,medium) = (20.029+4*Zg)/fmax1
        dl2(2,medium) = -1.93/fmax1
        dl3(2,medium) = -0.086/fmax1
        dl4(2,medium) = (21.12+4*Zg)/fmax1
        dl5(2,medium) = -4.184/fmax1
        dl6(2,medium) = 0.952
        dl1(3,medium) = (20.863 + 4*Zv)/fmax2
        dl2(3,medium) = -3.242/fmax2
        dl3(3,medium) = 0.625/fmax2
        dl4(3,medium) = (21.12+4*Zv)/fmax2
        dl5(3,medium) = -4.184/fmax2
        dl6(3,medium) = 0.952
        dl1(4,medium) = (20.029+4*Zv)/fmax2
        dl2(4,medium) = -1.93/fmax2
        dl3(4,medium) = -0.086/fmax2
        dl4(4,medium) = (21.12+4*Zv)/fmax2
        dl5(4,medium) = -4.184/fmax2
        dl6(4,medium) = 0.952
        dl1(5,medium) = (3*(20.863 + 4*Zg) - (20.029 + 4*Zg))
        dl2(5,medium) = (3*(-3.242) - (-1.930))
        dl3(5,medium) = (3*(0.625)-(-0.086))
        dl4(5,medium) = (2*21.12+8*Zg)
        dl5(5,medium) = (2*(-4.184))
        dl6(5,medium) = 0.952
        dl1(6,medium) = (3*(20.863 + 4*Zg) + (20.029 + 4*Zg))
        dl2(6,medium) = (3*(-3.242) + (-1.930))
        dl3(6,medium) = (3*0.625+(-0.086))
        dl4(6,medium) = (4*21.12+16*Zg)
        dl5(6,medium) = (4*(-4.184))
        dl6(6,medium) = 0.952
        dl1(7,medium) = (3*(20.863 + 4*Zv) - (20.029 + 4*Zv))
        dl2(7,medium) = (3*(-3.242) - (-1.930))
        dl3(7,medium) = (3*(0.625)-(-0.086))
        dl4(7,medium) = (2*21.12+8*Zv)
        dl5(7,medium) = (2*(-4.184))
        dl6(7,medium) = 0.952
        dl1(8,medium) = (3*(20.863 + 4*Zv) + (20.029 + 4*Zv))
        dl2(8,medium) = (3*(-3.242) + (-1.930))
        dl3(8,medium) = (3*0.625+(-0.086))
        dl4(8,medium) = (4*21.12+16*Zv)
        dl5(8,medium) = (4*(-4.184))
        dl6(8,medium) = 0.952
        bpar(2,medium) = dl1(7,medium)/(3*dl1(8,medium) + dl1(7,medium))
        bpar(1,medium) = 12*dl1(8,medium)/(3*dl1(8,medium) + dl1(7,mediu
     *  m))
7071  CONTINUE
7072  CONTINUE
      return
      end
      real*8 function FCOULC(Z)
      implicit none
      real*8 Z
      real*8 fine,asq
      data fine/137.03604/
      asq = Z/fine
      asq = asq*asq
      FCOULC = asq*(1.0/(1.0+ASQ)+0.20206+ASQ*(-0.0369+ASQ*(0.0083+ASQ*(
     *-0.002))))
      return
      end
      real*8 function XSIF(Z)
      implicit none
      real*8 Z
      integer*4 iZ
      real*8 alrad(4),alradp(4),a1440,a183,FCOULC
      data alrad/5.31,4.79,4.74,4.71/
      data alradp/6.144,5.621,5.805,5.924/
      data a1440/1194.0/,A183/184.15/
      IF (( Z .LE. 4 )) THEN
        iZ = Z
        xsif = alradp(iZ)/(alrad(iZ) - FCOULC(Z))
      ELSE
        xsif = Log(A1440*Z**(-0.666667))/(Log(A183*Z**(-0.33333))-FCOULC
     *  (Z))
      END IF
      return
      end
      subroutine init_compton
      implicit none
      common/compton_data/ iz_array(1538),  be_array(1538),  Jo_array(15
     *38),  erfJo_array(1538),   ne_array(1538),  shn_array(1538),
     *shell_array(200,1), eno_array(200,1), eno_atbin_array(200,1), n_sh
     *ell(1), radc_flag,  ibcmp(1)
      integer*4 iz_array,ne_array,shn_array,eno_atbin_array, shell_array
     *,n_shell,radc_flag
      real*8 be_array,Jo_array,erfJo_array,eno_array
      integer*2 ibcmp
      COMMON/BREMPR/ DL1(8,1),DL2(8,1),DL3(8,1),DL4(8,1),DL5(8,1),DL6(8,
     *1), ALPHI(2,1),BPAR(2,1),DELPOS(2,1), WA(1,50),PZ(1,50),ZELEM(1,50
     *),RHOZ(1,50), PWR2I(50), DELCM(1),ZBRANG(1),LZBRANG(1),NNE(1), IBR
     *DST,IPRDST,ibr_nist,pair_nrc,itriplet, ASYM(1,50,2)
      CHARACTER*4 ASYM
      real*8 DL1,DL2,DL3,DL4,DL5,DL6,   ALPHI,  BPAR,  DELPOS,  WA,  PZ,
     *  ZELEM,  RHOZ,  PWR2I,  DELCM,  ZBRANG,  LZBRANG
      integer*4 NNE,  IBRDST,  IPRDST,  ibr_nist,  itriplet,  pair_nrc
      COMMON/EDGE/binding_energies(30,100), interaction_prob(6,100), rel
     *axation_prob(39,100), edge_energies(16,100), edge_number(100), edg
     *e_a(16,100), edge_b(16,100), edge_c(16,100), edge_d(16,100), IEDGF
     *L(1),IPHTER(1)
      real*8 binding_energies,  interaction_prob,    relaxation_prob,  e
     *dge_energies,  edge_a,edge_b,edge_c,edge_d
      integer*2 IEDGFL,  IPHTER
      integer*4 edge_number
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      COMMON/MISC/  DUNIT,KMPI,KMPO,RHOR(1),MED(1),IRAYLR(1),IPHOTONUCR(
     *1)
      real*8 DUNIT,  RHOR
      integer*4 KMPI,  KMPO
      integer*2 MED,  IRAYLR,  IPHOTONUCR
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      common/x_options/eadl_relax,  mcdf_pe_xsections
      logical eadl_relax, mcdf_pe_xsections
      integer*4 i,j,iz,nsh,j_l,j_h
      real*8 aux,pztot,atav
      real*8 aux_erf,erf1
      logical getd
      call radc_init
      getd = .false.
      DO 7091 j=1,1
        medium = med(j)
        IF (( medium .GT. 0 .AND. medium .LE. nmed)) THEN
          IF (( ibcmp(j) .GT. 0 )) THEN
            getd = .true.
            GO TO7092
          END IF
        END IF
7091  CONTINUE
7092  CONTINUE
      IF (( .NOT.getd )) THEN
        IF (( eadl_relax .AND. photon_xsections .EQ. 'xcom' )) THEN
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,'(a,/a,/a)') 'You must turn ON Compton binding cor
     *rections when using', 'a detailed atomic relaxation (eadl_relax=tr
     *ue) since ', 'binding energies taken from incoh.data below 1 keV!'
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        END IF
        write(i_log,'(a/)') ' Bound Compton scattering not requested! '
        return
      END IF
      write(i_log,'(/a$)') 'Bound Compton scattering requested, reading
     *data ......'
      rewind(i_incoh)
      DO 7101 j=1,18
        read(i_incoh,*)
7101  CONTINUE
7102  CONTINUE
      iz = 0
      DO 7111 j=1,1538
        read(i_incoh,*) iz_array(j),shn_array(j),ne_array(j), Jo_array(j
     *  ),be_array(j)
        Jo_array(j) = Jo_array(j)*137.
        be_array(j) = be_array(j)*1e-6/PRM
        aux_erf = 0.70710678119*(1+0.3*Jo_array(j))
        erfJo_array(j) = 0.82436063535*(erf1(aux_erf)-1)
        IF ((eadl_relax)) THEN
          IF ((iz_array(j) .NE. iz)) THEN
            shn_array(j) = 1
            iz = iz_array(j)
          ELSE
            shn_array(j) = shn_array(j-1)+1
          END IF
          IF ((binding_energies(shn_array(j),iz_array(j)) .GT. 0)) THEN
            be_array(j) = binding_energies(shn_array(j),iz_array(j))/PRM
          ELSE IF((photon_xsections .EQ. 'xcom')) THEN
            binding_energies(shn_array(j),iz_array(j)) = be_array(j)*PRM
          END IF
        END IF
7111  CONTINUE
7112  CONTINUE
      write(i_log,*) ' Done'
      write(i_log,'(/a)') ' Initializing Bound Compton scattering ......
     *'
      DO 7121 medium=1,nmed
        pztot = 0
        nsh = 0
        DO 7131 i=1,nne(medium)
          iz = int(zelem(medium,i))
          DO 7141 j=1,1538
            IF (( iz .EQ. iz_array(j) )) THEN
              nsh = nsh + 1
              IF (( nsh .GT. 200 )) THEN
                write(i_log,'(/a)') '***************** Error: '
                write(i_log,'(/a,i3,a,i4,a/,a)') ' For medium ',medium,
     *          ' the number of shells is > ',200,'!', ' Increase the pa
     *rameter $MXMDSH! '
                write(i_log,'(/a)') '***************** Quiting now.'
                call exit(1)
              END IF
              shell_array(nsh,medium) = j
              aux = pz(medium,i)*ne_array(j)
              eno_array(nsh,medium) = aux
              pztot = pztot + aux
            END IF
7141      CONTINUE
7142      CONTINUE
7131    CONTINUE
7132    CONTINUE
        IF (( nsh .EQ. 0 )) THEN
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,'(a,i3,a)') ' Medium ',medium,' has zero shells! '
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        END IF
        n_shell(medium) = nsh
        write(i_log,'(a,i3,a,i3,a)') ' Medium ',medium,' has ',nsh,' she
     *lls: '
        DO 7151 i=1,nsh
          j = shell_array(i,medium)
          eno_array(i,medium) = eno_array(i,medium)/pztot
          write(i_log,'(i4,i5,i4,f9.5,e10.3,f10.3)') i,j,shn_array(j),en
     *    o_array(i,medium), Jo_array(j),be_array(j)*PRM*1000.
          eno_array(i,medium) = -eno_array(i,medium)
          eno_atbin_array(i,medium) = i
7151    CONTINUE
7152    CONTINUE
        atav = 1./nsh
        DO 7161 i=1,nsh-1
          DO 7171 j_h=1,nsh-1
            IF (( eno_array(j_h,medium) .LT. 0 )) THEN
              IF((abs(eno_array(j_h,medium)) .GT. atav))GO TO7172
            END IF
7171      CONTINUE
7172      CONTINUE
          DO 7181 j_l=1,nsh-1
            IF (( eno_array(j_l,medium) .LT. 0 )) THEN
              IF((abs(eno_array(j_l,medium)) .LT. atav))GO TO7182
            END IF
7181      CONTINUE
7182      CONTINUE
          aux = atav - abs(eno_array(j_l,medium))
          eno_array(j_h,medium) = eno_array(j_h,medium) + aux
          eno_array(j_l,medium) = -eno_array(j_l,medium)/atav + j_l
          eno_atbin_array(j_l,medium) = j_h
          IF((i .EQ. nsh-1))eno_array(j_h,medium) = 1 + j_h
7161    CONTINUE
7162    CONTINUE
        DO 7191 i=1,nsh
          IF (( eno_array(i,medium) .LT. 0 )) THEN
            eno_array(i,medium) = 1 + i
          END IF
7191    CONTINUE
7192    CONTINUE
7121  CONTINUE
7122  CONTINUE
      write(i_log,'(a/)') ' ...... Done.'
      getd = .false.
      DO 7201 j=1,1
        IF (( iedgfl(j) .GT. 0 .AND. iedgfl(j) .LE. 100 )) THEN
          getd = .true.
          GO TO7202
        END IF
7201  CONTINUE
7202  CONTINUE
      IF((getd))return
      write(i_log,'(/a)') '***************** Error: '
      write(i_log,'(/a,/a,/a,/a)') ' In subroutine init_compton: ', '
     *Scattering off bound electrons creates atomic vacancies,', '   pot
     *entially starting an atomic relaxation cascade. ', '   Please turn
     * ON atomic relaxations.'
      write(i_log,'(/a)') '***************** Quiting now.'
      call exit(1)
      return
      end
      SUBROUTINE MOLLER
      implicit none
      COMMON/QDEBUG/QDEBUG
      LOGICAL QDEBUG
      COMMON/STACK/ E(50),X(50),Y(50),Z(50),U(50),V(50),W(50),DNEAR(50),
     *WT(50),IQ(50),IR(50),LATCH(50), LATCHI,NP,NPold
      DOUBLE PRECISION E
      real*8 X,Y,Z,  U,V,W,  DNEAR,  WT
      integer*4 IQ,  IR,  LATCH,  LATCHI, NP,  NPold
      COMMON/THRESH/RMT2,RMSQ, AP(1),AE(1),UP(1),UE(1),TE(1),THMOLL(1)
      real*8 RMT2,  RMSQ,  AP,  AE,  UP,  UE,  TE,  THMOLL
      COMMON/UPHIOT/THETA,SINTHE,COSTHE,SINPHI, COSPHI,PI,TWOPI,PI5D2
      real*8 THETA,  SINTHE,  COSTHE,  SINPHI,  COSPHI,  PI,TWOPI,PI5D2
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      common/egs_vr/ e_max_rr(1),  prob_RR,  nbr_split,  i_play_RR,
     * i_survived_RR,
     *     n_RR_warning,                                        i_do_rr(
     *1)
      real*8          e_max_rr,prob_RR
      integer*4       nbr_split,i_play_RR,i_survived_RR,n_RR_warning
      integer*2     i_do_rr
      common/eii_data/ eii_xsection_a( 10000),  eii_xsection_b( 10000),
     * eii_cons(1), eii_a(40),  eii_b(40),  eii_L_factor,  eii_z(40),  e
     *ii_sh(40),  eii_nshells(100),  eii_nsh(1),  eii_first(1,50),  eii_
     *no(1,50),  eii_flag
      real*8 eii_xsection_a,eii_xsection_b,eii_a,eii_b,eii_cons,eii_L_fa
     *ctor
      integer*4 eii_z,eii_sh,eii_nshells
      integer*4 eii_first,eii_no
      integer*4 eii_elements,eii_flag,eii_nsh
      COMMON/ELECIN/ esig_e(1),psig_e(1), esige_max, psige_max, range_ep
     *(0:1,500,1), E_array(500,1), etae_ms0(500,1),etae_ms1(500,1),etap_
     *ms0(500,1),etap_ms1(500,1),q1ce_ms0(500,1),q1ce_ms1(500,1),q1cp_ms
     *0(500,1),q1cp_ms1(500,1),q2ce_ms0(500,1),q2ce_ms1(500,1),q2cp_ms0(
     *500,1),q2cp_ms1(500,1),blcce0(500,1),blcce1(500,1), EKE0(1),EKE1(1
     *), XR0(1),TEFF0(1),BLCC(1),XCC(1), ESIG0(500,1),ESIG1(500,1),PSIG0
     *(500,1),PSIG1(500,1),EDEDX0(500,1),EDEDX1(500,1),PDEDX0(500,1),PDE
     *DX1(500,1),EBR10(500,1),EBR11(500,1),PBR10(500,1),PBR11(500,1),PBR
     *20(500,1),PBR21(500,1),TMXS0(500,1),TMXS1(500,1), expeke1(1), IUNR
     *ST(1),EPSTFL(1),IAPRIM(1), sig_ismonotone(0:1,1)
      real*8 esig_e,   psig_e,   esige_max,  psige_max,  range_ep,  E_ar
     *ray,  etae_ms0,etae_ms1,  etap_ms0,etap_ms1,  q1ce_ms0,q1ce_ms1,
     *q1cp_ms0,q1cp_ms1,  q2ce_ms0,q2ce_ms1,  q2cp_ms0,q2cp_ms1,  blcce0
     *,blcce1,   expeke1,  EKE0,EKE1, XR0,  TEFF0,  BLCC,  XCC,  ESIG0,E
     *SIG1,  PSIG0,PSIG1,  EDEDX0,EDEDX1,  PDEDX0,PDEDX1,  EBR10,EBR11,
     * PBR10,PBR11,  PBR20,PBR21,  TMXS0,TMXS1
      integer*4 IUNRST,  EPSTFL,  IAPRIM
      logical sig_ismonotone
      COMMON/EPCONT/EDEP,EDEP_LOCAL,TSTEP,TUSTEP,USTEP,TVSTEP,VSTEP, RHO
     *F,EOLD,ENEW,EKE,ELKE,GLE,E_RANGE, x_final,y_final,z_final, u_final
     *,v_final,w_final, IDISC,IROLD,IRNEW,IAUSFL(35)
      DOUBLE PRECISION EDEP,  EDEP_LOCAL
      real*8 TSTEP,  TUSTEP,  USTEP,  VSTEP,  TVSTEP,  RHOF,  EOLD,  ENE
     *W,  EKE,  ELKE,  GLE,  E_RANGE, x_final,y_final,z_final,  u_final,
     *v_final,w_final
      integer*4 IDISC,  IROLD,  IRNEW,  IAUSFL
      COMMON/EDGE/binding_energies(30,100), interaction_prob(6,100), rel
     *axation_prob(39,100), edge_energies(16,100), edge_number(100), edg
     *e_a(16,100), edge_b(16,100), edge_c(16,100), edge_d(16,100), IEDGF
     *L(1),IPHTER(1)
      real*8 binding_energies,  interaction_prob,    relaxation_prob,  e
     *dge_energies,  edge_a,edge_b,edge_c,edge_d
      integer*2 IEDGFL,  IPHTER
      integer*4 edge_number
      COMMON/BREMPR/ DL1(8,1),DL2(8,1),DL3(8,1),DL4(8,1),DL5(8,1),DL6(8,
     *1), ALPHI(2,1),BPAR(2,1),DELPOS(2,1), WA(1,50),PZ(1,50),ZELEM(1,50
     *),RHOZ(1,50), PWR2I(50), DELCM(1),ZBRANG(1),LZBRANG(1),NNE(1), IBR
     *DST,IPRDST,ibr_nist,pair_nrc,itriplet, ASYM(1,50,2)
      CHARACTER*4 ASYM
      real*8 DL1,DL2,DL3,DL4,DL5,DL6,   ALPHI,  BPAR,  DELPOS,  WA,  PZ,
     *  ZELEM,  RHOZ,  PWR2I,  DELCM,  ZBRANG,  LZBRANG
      integer*4 NNE,  IBRDST,  IPRDST,  ibr_nist,  itriplet,  pair_nrc
      DOUBLE PRECISION PEIE,  PEKSE2,  PESE1,  PESE2,  PEKIN,  H1,  DCOS
     *TH
      real*8 EIE,  EKIN,  T0,  E0,  EXTRAE,  E02,  EP0,  G2,G3,  GMAX,
     *BR,  R,  REJF4,  RNNO27,  RNNO28,  ESE1,  ESE2
      real*8 sigm,pbrem,rsh,Uj,sig_j
      integer*4 lelke,iele,ish,nsh,ifirst,i,jj,iZ,iarg
      NPold = NP
      PEIE=E(NP)
      EIE=PEIE
      PEKIN=PEIE-PRM
      EKIN=PEKIN
      IF (( eii_flag .GT. 0 .AND. eii_nsh(medium) .GT. 0 )) THEN
        Lelke=eke1(MEDIUM)*elke+eke0(MEDIUM)
        sigm=esig1(Lelke,MEDIUM)*elke+esig0(Lelke,MEDIUM)
        pbrem=ebr11(Lelke,MEDIUM)*elke+ebr10(Lelke,MEDIUM)
        sigm = sigm*(1 - pbrem)
        IF((rng_seed .GT. 128))call ranmar_get
        rsh = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        rsh = sigm*rsh
        DO 7211 iele=1,nne(medium)
          iZ = int(zelem(medium,iele)+0.5)
          nsh = eii_no(medium,iele)
          IF (( nsh .GT. 0 )) THEN
            ifirst = eii_first(medium,iele)
            DO 7221 ish=1,nsh
              Uj = binding_energies(ish,iZ)
              IF (( ekin .GT. Uj .AND. (Uj .GT. te(medium) .OR. Uj .GT.
     *        ap(medium)) )) THEN
                jj = ifirst + ish - 1
                i = eii_a(jj)*elke + eii_b(jj) + (jj-1)*250
                sig_j = eii_xsection_a(i)*elke + eii_xsection_b(i)
                sig_j = sig_j*pz(medium,iele)*eii_cons(medium)
                rsh = rsh - sig_j
                IF (( rsh .LT. 0 )) THEN
                  IARG=31
                  IF ((IAUSFL(IARG+1).NE.0)) THEN
                    CALL AUSGAB(IARG)
                  END IF
                  call eii_sample(ish,iZ,Uj)
                  IARG=32
                  IF ((IAUSFL(IARG+1).NE.0)) THEN
                    CALL AUSGAB(IARG)
                  END IF
                  return
                END IF
              END IF
7221        CONTINUE
7222        CONTINUE
          END IF
7211    CONTINUE
7212    CONTINUE
      END IF
      IF((ekin .LE. 2*te(medium)))return
      T0=EKIN/RM
      E0=T0+1.0
      EXTRAE = EIE - THMOLL(MEDIUM)
      E02=E0*E0
      EP0=TE(MEDIUM)/EKIN
      G2=T0*T0/E02
      G3=(2.*T0+1.)/E02
      GMAX=(1.+1.25*G2)
7231  CONTINUE
        IF((rng_seed .GT. 128))call ranmar_get
        RNNO27 = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        BR = TE(MEDIUM)/(EKIN-EXTRAE*RNNO27)
        R=BR/(1.-BR)
        IF((rng_seed .GT. 128))call ranmar_get
        RNNO28 = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        REJF4=(1.+G2*BR*BR+R*(R-G3))
        RNNO28=GMAX*RNNO28
        IF((RNNO28.LE.REJF4))GO TO7232
      GO TO 7231
7232  CONTINUE
      PEKSE2=BR*EKIN
      PESE1=PEIE-PEKSE2
      PESE2=PEKSE2+PRM
      ESE1=PESE1
      ESE2=PESE2
      E(NP)=PESE1
      IF (( np+1 .GT. 50 )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,'(//,3a,/,2(a,i9))') ' In subroutine ','MOLLER', ' s
     *tack size exceeded! ',' $MAXSTACK = ',50,' np = ',np+1
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      E(NP+1)=PESE2
      H1=(PEIE+PRM)/PEKIN
      DCOSTH=H1*(PESE1-PRM)/(PESE1+PRM)
      SINTHE=DSQRT(1.D0-DCOSTH)
      COSTHE=DSQRT(DCOSTH)
      CALL UPHI(2,1)
      NP=NP+1
      IQ(NP)=-1
      DCOSTH=H1*(PESE2-PRM)/(PESE2+PRM)
      SINTHE=-DSQRT(1.D0-DCOSTH)
      COSTHE=DSQRT(DCOSTH)
      CALL UPHI(3,2)
      RETURN
      END
      subroutine mscati
      implicit none
      real*8 ededx,ei,eil,eip1,eip1l,si,sip1,eke,elke,aux,ecutmn,tstbm,t
     *stbmn
      real*8 p2,beta2,dedx0,ekef,elkef,estepx,ektmp,elktmp,chi_a2
      integer*4 i,leil,leip1l,neke,lelke,lelkef,lelktmp
      logical ise_monoton, isp_monoton
      real*8 sigee,sigep,sig,sige_old,sigp_old
      COMMON/BOUNDS/ECUT(1),PCUT(1),VACDST
      real*8 ECUT,  PCUT,  VACDST
      COMMON/ELECIN/ esig_e(1),psig_e(1), esige_max, psige_max, range_ep
     *(0:1,500,1), E_array(500,1), etae_ms0(500,1),etae_ms1(500,1),etap_
     *ms0(500,1),etap_ms1(500,1),q1ce_ms0(500,1),q1ce_ms1(500,1),q1cp_ms
     *0(500,1),q1cp_ms1(500,1),q2ce_ms0(500,1),q2ce_ms1(500,1),q2cp_ms0(
     *500,1),q2cp_ms1(500,1),blcce0(500,1),blcce1(500,1), EKE0(1),EKE1(1
     *), XR0(1),TEFF0(1),BLCC(1),XCC(1), ESIG0(500,1),ESIG1(500,1),PSIG0
     *(500,1),PSIG1(500,1),EDEDX0(500,1),EDEDX1(500,1),PDEDX0(500,1),PDE
     *DX1(500,1),EBR10(500,1),EBR11(500,1),PBR10(500,1),PBR11(500,1),PBR
     *20(500,1),PBR21(500,1),TMXS0(500,1),TMXS1(500,1), expeke1(1), IUNR
     *ST(1),EPSTFL(1),IAPRIM(1), sig_ismonotone(0:1,1)
      real*8 esig_e,   psig_e,   esige_max,  psige_max,  range_ep,  E_ar
     *ray,  etae_ms0,etae_ms1,  etap_ms0,etap_ms1,  q1ce_ms0,q1ce_ms1,
     *q1cp_ms0,q1cp_ms1,  q2ce_ms0,q2ce_ms1,  q2cp_ms0,q2cp_ms1,  blcce0
     *,blcce1,   expeke1,  EKE0,EKE1, XR0,  TEFF0,  BLCC,  XCC,  ESIG0,E
     *SIG1,  PSIG0,PSIG1,  EDEDX0,EDEDX1,  PDEDX0,PDEDX1,  EBR10,EBR11,
     * PBR10,PBR11,  PBR20,PBR21,  TMXS0,TMXS1
      integer*4 IUNRST,  EPSTFL,  IAPRIM
      logical sig_ismonotone
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      COMMON/MISC/  DUNIT,KMPI,KMPO,RHOR(1),MED(1),IRAYLR(1),IPHOTONUCR(
     *1)
      real*8 DUNIT,  RHOR
      integer*4 KMPI,  KMPO
      integer*2 MED,  IRAYLR,  IPHOTONUCR
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      common/ET_control/ smaxir(1),estepe,ximax,  skindepth_for_bca,tran
     *sport_algorithm, bca_algorithm,exact_bca,spin_effects
      real*8 smaxir,  estepe,  ximax,      skindepth_for_bca
      integer*4 transport_algorithm, bca_algorithm
      logical exact_bca,  spin_effects
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      IF (( bca_algorithm .EQ. 0 )) THEN
        exact_bca = .true.
      ELSE
        exact_bca = .false.
      END IF
      IF (( estepe .LE. 0 .OR. estepe .GE. 1)) THEN
        estepe = 0.25
      END IF
      IF (( ximax .LE. 0 .OR. ximax .GE. 1 )) THEN
        IF (( exact_bca )) THEN
          ximax = 0.5
        ELSE
          ximax = 0.5
        END IF
      END IF
      IF ((transport_algorithm .NE. 0 .AND. transport_algorithm .NE. 1 .
     *AND. transport_algorithm .NE. 2 )) THEN
        transport_algorithm = 0
      END IF
      IF (( skindepth_for_bca .LE. 1e-4 )) THEN
        IF (( .NOT.exact_bca )) THEN
          write(i_log,*) ' old PRESTA calculates default min. step-size
     *for BCA: '
          ecutmn = 1e30
          DO 7241 i=1,1
            IF (( med(i) .GT. 0 .AND. med(i) .LE. nmed )) THEN
              ecutmn = Min(ecutmn,ecut(i))
            END IF
7241      CONTINUE
7242      CONTINUE
          write(i_log,*) '     minimum ECUT found: ',ecutmn
          tstbmn = 1e30
          DO 7251 medium=1,nmed
            tstbm = (ecutmn-prm)*(ecutmn+prm)/ecutmn**2
            tstbm = blcc(medium)*tstbm*(ecutmn/xcc(medium))**2
            aux = Log(tstbm)
            IF((aux .GT. 300))write(i_log,*) 'aux > 300 ? ',aux
            tstbm = Log(tstbm/aux)
            tstbmn = Min(tstbmn,tstbm)
7251      CONTINUE
7252      CONTINUE
          write(i_log,*) '     default BLCMIN is: ',tstbmn
          skindepth_for_bca = Exp(tstbmn)
          write(i_log,*) '     this corresponds to ',skindepth_for_bca,
     *    ' elastic MFPs '
        ELSE
          skindepth_for_bca = 3
        END IF
      END IF
      call init_ms_SR
      DO 7261 medium=1,nmed
        blcc(medium) = 1.16699413758864573*blcc(medium)
        xcc(medium) = xcc(medium)**2
7261  CONTINUE
7262  CONTINUE
      IF (( spin_effects )) THEN
        call init_spin
      END IF
      write(i_log,*) ' '
      esige_max = 0
      psige_max = 0
      DO 7271 medium=1,nmed
        sigee = 1E-15
        sigep = 1E-15
        neke = meke(medium)
        ise_monoton = .true.
        isp_monoton = .true.
        sige_old = -1
        sigp_old = -1
        DO 7281 i=1,neke
          ei = exp((float(i) - eke0(medium))/eke1(medium))
          eil = log(ei)
          leil = i
          ededx=ededx1(Leil,MEDIUM)*eil+ededx0(Leil,MEDIUM)
          sig=esig1(Leil,MEDIUM)*eil+esig0(Leil,MEDIUM)
          sig = sig/ededx
          IF((sig .GT. sigee))sigee = sig
          IF((sig .LT. sige_old))ise_monoton = .false.
          sige_old = sig
          ededx=pdedx1(Leil,MEDIUM)*eil+pdedx0(Leil,MEDIUM)
          sig=psig1(Leil,MEDIUM)*eil+psig0(Leil,MEDIUM)
          sig = sig/ededx
          IF((sig .GT. sigep))sigep = sig
          IF((sig .LT. sigp_old))isp_monoton = .false.
          sigp_old = sig
7281    CONTINUE
7282    CONTINUE
        write(i_log,*) ' Medium ',medium,' sige = ',sigee,sigep,' monoto
     *ne = ', ise_monoton,isp_monoton
        sig_ismonotone(0,medium) = ise_monoton
        sig_ismonotone(1,medium) = isp_monoton
        esig_e(medium) = sigee
        psig_e(medium) = sigep
        IF((sigee .GT. esige_max))esige_max = sigee
        IF((sigep .GT. psige_max))psige_max = sigep
7271  CONTINUE
7272  CONTINUE
      write(i_log,*) ' '
      write(i_log,*) ' Initializing tmxs for estepe = ',estepe,' and xim
     *ax = ',ximax
      write(i_log,*) ' '
      DO 7291 medium=1,nmed
        ei = exp((1 - eke0(medium))/eke1(medium))
        eil = log(ei)
        leil = 1
        E_array(1,medium) = ei
        expeke1(medium) = Exp(1./eke1(medium))-1
        range_ep(0,1,medium) = 0
        range_ep(1,1,medium) = 0
        neke = meke(medium)
        DO 7301 i=1,neke - 1
          eip1 = exp((float(i + 1) - eke0(medium))/eke1(medium))
          E_array(i+1,medium) = eip1
          eke = 0.5*(eip1+ei)
          elke = Log(eke)
          Lelke=eke1(MEDIUM)*elke+eke0(MEDIUM)
          ededx=pdedx1(Lelke,MEDIUM)*elke+pdedx0(Lelke,MEDIUM)
          aux = pdedx1(i,medium)/ededx
          range_ep(1,i+1,medium) = range_ep(1,i,medium) + (eip1-ei)/eded
     *    x*(1+aux*(1+2*aux)*((eip1-ei)/eke)**2/24)
          ededx=ededx1(Lelke,MEDIUM)*elke+ededx0(Lelke,MEDIUM)
          aux = ededx1(i,medium)/ededx
          range_ep(0,i+1,medium) = range_ep(0,i,medium) + (eip1-ei)/eded
     *    x*(1+aux*(1+2*aux)*((eip1-ei)/eke)**2/24)
          ei = eip1
7301    CONTINUE
7302    CONTINUE
        eil = (1 - eke0(medium))/eke1(medium)
        ei = Exp(eil)
        leil = 1
        p2 = ei*(ei+2*rm)
        beta2 = p2/(p2+rm*rm)
        chi_a2 = Xcc(medium)/(4*p2*blcc(medium))
        dedx0=ededx1(Leil,MEDIUM)*eil+ededx0(Leil,MEDIUM)
        estepx = 2*p2*beta2*dedx0/ei/Xcc(medium)/(Log(1+1./chi_a2)*(1+ch
     *  i_a2)-1)
        estepx = estepx*ximax
        IF (( estepx .GT. estepe )) THEN
          estepx = estepe
        END IF
        si = estepx*ei/dedx0
        DO 7311 i=1,neke - 1
          elke = (i + 1 - eke0(medium))/eke1(medium)
          eke = Exp(elke)
          lelke = i+1
          p2 = eke*(eke+2*rm)
          beta2 = p2/(p2+rm*rm)
          chi_a2 = Xcc(medium)/(4*p2*blcc(medium))
          ededx=ededx1(Lelke,MEDIUM)*elke+ededx0(Lelke,MEDIUM)
          estepx = 2*p2*beta2*ededx/eke/ Xcc(medium)/(Log(1+1./chi_a2)*(
     *    1+chi_a2)-1)
          estepx = estepx*ximax
          IF (( estepx .GT. estepe )) THEN
            estepx = estepe
          END IF
          ekef = (1-estepx)*eke
          IF (( ekef .LE. E_array(1,medium) )) THEN
            sip1 = (E_array(1,medium) - ekef)/dedx0
            ekef = E_array(1,medium)
            elkef = (1 - eke0(medium))/eke1(medium)
            lelkef = 1
          ELSE
            elkef = Log(ekef)
            Lelkef=eke1(MEDIUM)*elkef+eke0(MEDIUM)
            leip1l = lelkef + 1
            eip1l = (leip1l - eke0(medium))/eke1(medium)
            eip1 = E_array(leip1l,medium)
            aux = (eip1 - ekef)/eip1
            elktmp = 0.5*(elkef+eip1l+0.25*aux*aux*(1+aux*(1+0.875*aux))
     *      )
            ektmp = 0.5*(ekef+eip1)
            lelktmp = lelkef
            ededx=ededx1(Lelktmp,MEDIUM)*elktmp+ededx0(Lelktmp,MEDIUM)
            aux = ededx1(lelktmp,medium)/ededx
            sip1 = (eip1 - ekef)/ededx*( 1+aux*(1+2*aux)*((eip1-ekef)/ek
     *      tmp)**2/24)
          END IF
          sip1 = sip1 + range_ep(0,lelke,medium) - range_ep(0,lelkef+1,m
     *    edium)
          tmxs1(i,medium) = (sip1 - si)*eke1(medium)
          tmxs0(i,medium) = sip1 - tmxs1(i,medium)*elke
          si = sip1
7311    CONTINUE
7312    CONTINUE
        tmxs0(neke,medium) = tmxs0(neke - 1,medium)
        tmxs1(neke,medium) = tmxs1(neke - 1,medium)
7291  CONTINUE
7292  CONTINUE
      return
      end
      subroutine mscat(lambda,chia2,q1,elke,beta2,qel,medium, spin_effec
     *ts,find_index,spin_index, cost,sint)
      implicit none
      real*8 lambda, chia2,q1,elke,beta2,cost,sint
      integer*4 qel,medium
      logical spin_effects,find_index,spin_index
      common/ms_data/ ums_array(0:63,0:7,0:31), fms_array(0:63,0:7,0:31)
     *, wms_array(0:63,0:7,0:31), ims_array(0:63,0:7,0:31), llammin,llam
     *max,dllamb,dllambi,dqms,dqmsi
      real*4 ums_array,fms_array,wms_array, llammin,llammax,dllamb,dllam
     *bi,dqms,dqmsi
      integer*2 ims_array
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      real*8 sprob,explambda,wsum,wprob,xi,rejf,spin_rejection, cosz,sin
     *z,phi,omega2,llmbda,ai,aj,ak,a,u,du,x1,rnno
      integer*4 icount,i,j,k
      save i,j,omega2
      IF ((lambda .LE. 13.8)) THEN
        IF((rng_seed .GT. 128))call ranmar_get
        sprob = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        explambda = Exp(-lambda)
        IF ((sprob .LT. explambda)) THEN
          cost = 1
          sint = 0
          return
        END IF
        wsum = (1+lambda)*explambda
        IF (( sprob .LT. wsum )) THEN
7320      CONTINUE
          IF((rng_seed .GT. 128))call ranmar_get
          xi = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          xi = 2*chia2*xi/(1 - xi + chia2)
          cost = 1 - xi
          IF (( spin_effects )) THEN
            rejf = spin_rejection(qel,medium,elke,beta2,q1,cost, spin_in
     *      dex,.false.)
            IF((rng_seed .GT. 128))call ranmar_get
            rnno = rng_array(rng_seed)*twom24
            rng_seed = rng_seed + 1
            IF (( rnno .GT. rejf )) THEN
              GOTO 7320
            END IF
          END IF
          sint = sqrt(xi*(2 - xi))
          return
        END IF
        IF (( lambda .LE. 1 )) THEN
          wprob = explambda
          wsum = explambda
          cost = 1
          sint = 0
          icount = 0
7331      CONTINUE
            icount = icount + 1
            IF((icount .GT. 20))GO TO7332
            wprob = wprob*lambda/icount
            wsum = wsum + wprob
7340        CONTINUE
            IF((rng_seed .GT. 128))call ranmar_get
            xi = rng_array(rng_seed)*twom24
            rng_seed = rng_seed + 1
            xi = 2*chia2*xi/(1 - xi + chia2)
            cosz = 1 - xi
            IF (( spin_effects )) THEN
              rejf = spin_rejection(qel,medium,elke,beta2,q1,cosz, spin_
     *        index,.false.)
              IF((rng_seed .GT. 128))call ranmar_get
              rnno = rng_array(rng_seed)*twom24
              rng_seed = rng_seed + 1
              IF (( rnno .GT. rejf )) THEN
                GOTO 7340
              END IF
            END IF
            sinz = xi*(2 - xi)
            IF (( sinz .GT. 1.e-20 )) THEN
              sinz = Sqrt(sinz)
              IF((rng_seed .GT. 128))call ranmar_get
              xi = rng_array(rng_seed)*twom24
              rng_seed = rng_seed + 1
              phi = xi*6.2831853
              cost = cost*cosz - sint*sinz*Cos(phi)
              sint = Sqrt(Max(0.0,(1-cost)*(1+cost)))
            END IF
            IF((( wsum .GT. sprob)))GO TO7332
          GO TO 7331
7332      CONTINUE
          return
        END IF
      END IF
      IF ((lambda .LE. 1e5 )) THEN
        IF ((find_index)) THEN
          llmbda = log(lambda)
          ai = llmbda*dllambi
          i = ai
          ai = ai - i
          IF((rng_seed .GT. 128))call ranmar_get
          xi = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          IF((xi .LT. ai))i = i + 1
          IF (( q1 .LT. 1e-3 )) THEN
            j = 0
          ELSE IF(( q1 .LT. 0.5 )) THEN
            aj = q1*dqmsi
            j = aj
            aj = aj - j
            IF((rng_seed .GT. 128))call ranmar_get
            xi = rng_array(rng_seed)*twom24
            rng_seed = rng_seed + 1
            IF((xi .LT. aj))j = j + 1
          ELSE
            j = 7
          END IF
          IF ((llmbda .LT. 2.2299)) THEN
            omega2 = chia2*(lambda + 4)*(1.347006 + llmbda*( 0.209364 -
     *      llmbda*(0.45525 - llmbda*(0.50142 - 0.081234*llmbda))))
          ELSE
            omega2 = chia2*(lambda + 4)*(-2.77164 + llmbda*(2.94874 - ll
     *      mbda*(0.1535754 - llmbda*0.00552888)))
          END IF
          find_index = .false.
        END IF
7350    CONTINUE
        IF((rng_seed .GT. 128))call ranmar_get
        xi = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        ak = xi*31
        k = ak
        ak = ak - k
        IF((ak .GT. wms_array(i,j,k)))k = ims_array(i,j,k)
        a = fms_array(i,j,k)
        u = ums_array(i,j,k)
        du = ums_array(i,j,k+1) - u
        IF((rng_seed .GT. 128))call ranmar_get
        xi = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        IF (( abs(a) .LT. 0.2 )) THEN
          x1 = 0.5*(1-xi)*a
          u = u + xi*du*(1+x1*(1-xi*a))
        ELSE
          u = u - du/a*(1-Sqrt(1+xi*a*(2+a)))
        END IF
        xi = omega2*u/(1 + 0.5*omega2 - u)
        IF (( xi .GT. 1.99999 )) THEN
          xi = 1.99999
        END IF
        cost = 1 - xi
        IF (( spin_effects )) THEN
          rejf=spin_rejection(qel,medium,elke,beta2,q1,cost,spin_index,.
     *    false.)
          IF((rng_seed .GT. 128))call ranmar_get
          rnno = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          IF (( rnno .GT. rejf )) THEN
            GOTO 7350
          END IF
        END IF
        sint = sqrt(xi*(2-xi))
        return
      END IF
      write(i_log,*) ' '
      write(i_log,*) ' *************************************'
      write(i_log,*) ' Maximum step size in mscat exceeded! '
      write(i_log,*) ' Maximum step size initialized: 100000'
      write(i_log,*) ' Present lambda: ',lambda
      write(i_log,*) ' chia2: ',chia2
      write(i_log,*) ' q1 elke beta2: ',q1,elke,beta2
      write(i_log,*) ' medium: ',medium
      write(i_log,'(/a)') '***************** Error: '
      write(i_log,*) ' Stopping execution'
      write(i_log,'(/a)') '***************** Quiting now.'
      call exit(1)
      end
      real*8 function spin_rejection(qel,medium,elke,beta2,q1,cost, spin
     *_index,is_single)
      implicit none
      real*8 elke,beta2,q1,cost
      integer*4 qel,medium
      logical spin_index,is_single
      common/spin_data/ spin_rej(1,0:1,0: 31,0:15,0:31), espin_min,espin
     *_max,espml,b2spin_min,b2spin_max, dbeta2,dbeta2i,dlener,dleneri,dq
     *q1,dqq1i, fool_intel_optimizer
      real*4 spin_rej,espin_min,espin_max,espml,b2spin_min,b2spin_max, d
     *beta2,dbeta2i,dlener,dleneri,dqq1,dqq1i
      logical fool_intel_optimizer
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      real*8 rnno,ai,qq1,aj,xi,ak
      integer*4 i,j,k
      save i,j
      IF (( spin_index )) THEN
        spin_index = .false.
        IF (( beta2 .GE. b2spin_min )) THEN
          ai = (beta2 - b2spin_min)*dbeta2i
          i = ai
          ai = ai - i
          i = i + 15 + 1
        ELSE IF(( elke .GT. espml )) THEN
          ai = (elke - espml)*dleneri
          i = ai
          ai = ai - i
        ELSE
          i = 0
          ai = -1
        END IF
        IF((rng_seed .GT. 128))call ranmar_get
        rnno = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        IF((rnno .LT. ai))i = i + 1
        IF (( is_single )) THEN
          j = 0
        ELSE
          qq1 = 2*q1
          qq1 = qq1/(1 + qq1)
          aj = qq1*dqq1i
          j = aj
          IF (( j .GE. 15 )) THEN
            j = 15
          ELSE
            aj = aj - j
            IF((rng_seed .GT. 128))call ranmar_get
            rnno = rng_array(rng_seed)*twom24
            rng_seed = rng_seed + 1
            IF((rnno .LT. aj))j = j + 1
          END IF
        END IF
      END IF
      xi = Sqrt(0.5*(1-cost))
      ak = xi*31
      k = ak
      ak = ak - k
      spin_rejection = (1-ak)*spin_rej(medium,qel,i,j,k) + ak*spin_rej(m
     *edium,qel,i,j,k+1)
      return
      end
      subroutine sscat(chia2,elke,beta2,qel,medium,spin_effects,cost,sin
     *t)
      implicit none
      real*8 chia2,elke,beta2,cost,sint
      integer*4 qel,medium
      logical spin_effects
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      real*8 xi,rnno,rejf,spin_rejection,qzero
      logical spin_index
      spin_index = .true.
7360  CONTINUE
      IF((rng_seed .GT. 128))call ranmar_get
      xi = rng_array(rng_seed)*twom24
      rng_seed = rng_seed + 1
      xi = 2*chia2*xi/(1 - xi + chia2)
      cost = 1 - xi
      IF (( spin_effects )) THEN
        qzero=0
        rejf = spin_rejection(qel,medium,elke,beta2,qzero,cost,spin_inde
     *  x,.true.)
        IF((rng_seed .GT. 128))call ranmar_get
        rnno = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        IF((rnno .GT. rejf))goto 7360
      END IF
      sint = sqrt(xi*(2 - xi))
      return
      end
      subroutine init_ms_SR
      implicit none
      common/ms_data/ ums_array(0:63,0:7,0:31), fms_array(0:63,0:7,0:31)
     *, wms_array(0:63,0:7,0:31), ims_array(0:63,0:7,0:31), llammin,llam
     *max,dllamb,dllambi,dqms,dqmsi
      real*4 ums_array,fms_array,wms_array, llammin,llammax,dllamb,dllam
     *bi,dqms,dqmsi
      integer*2 ims_array
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      integer*4 i,j,k
      write(i_log,'(/a,$)') 'Reading screened Rutherford MS data .......
     *........ '
      rewind(i_mscat)
      DO 7371 i=0,63
        DO 7381 j=0,7
          read(i_mscat,*) (ums_array(i,j,k),k=0,31)
          read(i_mscat,*) (fms_array(i,j,k),k=0,31)
          read(i_mscat,*) (wms_array(i,j,k),k=0,31-1)
          read(i_mscat,*) (ims_array(i,j,k),k=0,31-1)
          DO 7391 k=0,31-1
            fms_array(i,j,k) = fms_array(i,j,k+1)/fms_array(i,j,k)-1
            ims_array(i,j,k) = ims_array(i,j,k)-1
7391      CONTINUE
7392      CONTINUE
          fms_array(i,j,31)=fms_array(i,j,31-1)
7381    CONTINUE
7382    CONTINUE
7371  CONTINUE
7372  CONTINUE
      write(i_log,'(a)') ' done '
      llammin = Log(1.)
      llammax = Log(1e5)
      dllamb = (llammax-llammin)/63
      dllambi = 1./dllamb
      dqms = 0.5/7
      dqmsi = 1./dqms
      return
      end
      subroutine init_spin
      implicit none
      common/spin_data/ spin_rej(1,0:1,0: 31,0:15,0:31), espin_min,espin
     *_max,espml,b2spin_min,b2spin_max, dbeta2,dbeta2i,dlener,dleneri,dq
     *q1,dqq1i, fool_intel_optimizer
      real*4 spin_rej,espin_min,espin_max,espml,b2spin_min,b2spin_max, d
     *beta2,dbeta2i,dlener,dleneri,dqq1,dqq1i
      logical fool_intel_optimizer
      COMMON/ELECIN/ esig_e(1),psig_e(1), esige_max, psige_max, range_ep
     *(0:1,500,1), E_array(500,1), etae_ms0(500,1),etae_ms1(500,1),etap_
     *ms0(500,1),etap_ms1(500,1),q1ce_ms0(500,1),q1ce_ms1(500,1),q1cp_ms
     *0(500,1),q1cp_ms1(500,1),q2ce_ms0(500,1),q2ce_ms1(500,1),q2cp_ms0(
     *500,1),q2cp_ms1(500,1),blcce0(500,1),blcce1(500,1), EKE0(1),EKE1(1
     *), XR0(1),TEFF0(1),BLCC(1),XCC(1), ESIG0(500,1),ESIG1(500,1),PSIG0
     *(500,1),PSIG1(500,1),EDEDX0(500,1),EDEDX1(500,1),PDEDX0(500,1),PDE
     *DX1(500,1),EBR10(500,1),EBR11(500,1),PBR10(500,1),PBR11(500,1),PBR
     *20(500,1),PBR21(500,1),TMXS0(500,1),TMXS1(500,1), expeke1(1), IUNR
     *ST(1),EPSTFL(1),IAPRIM(1), sig_ismonotone(0:1,1)
      real*8 esig_e,   psig_e,   esige_max,  psige_max,  range_ep,  E_ar
     *ray,  etae_ms0,etae_ms1,  etap_ms0,etap_ms1,  q1ce_ms0,q1ce_ms1,
     *q1cp_ms0,q1cp_ms1,  q2ce_ms0,q2ce_ms1,  q2cp_ms0,q2cp_ms1,  blcce0
     *,blcce1,   expeke1,  EKE0,EKE1, XR0,  TEFF0,  BLCC,  XCC,  ESIG0,E
     *SIG1,  PSIG0,PSIG1,  EDEDX0,EDEDX1,  PDEDX0,PDEDX1,  EBR10,EBR11,
     * PBR10,PBR11,  PBR20,PBR21,  TMXS0,TMXS1
      integer*4 IUNRST,  EPSTFL,  IAPRIM
      logical sig_ismonotone
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      COMMON/BREMPR/ DL1(8,1),DL2(8,1),DL3(8,1),DL4(8,1),DL5(8,1),DL6(8,
     *1), ALPHI(2,1),BPAR(2,1),DELPOS(2,1), WA(1,50),PZ(1,50),ZELEM(1,50
     *),RHOZ(1,50), PWR2I(50), DELCM(1),ZBRANG(1),LZBRANG(1),NNE(1), IBR
     *DST,IPRDST,ibr_nist,pair_nrc,itriplet, ASYM(1,50,2)
      CHARACTER*4 ASYM
      real*8 DL1,DL2,DL3,DL4,DL5,DL6,   ALPHI,  BPAR,  DELPOS,  WA,  PZ,
     *  ZELEM,  RHOZ,  PWR2I,  DELCM,  ZBRANG,  LZBRANG
      integer*4 NNE,  IBRDST,  IPRDST,  ibr_nist,  itriplet,  pair_nrc
      COMMON/THRESH/RMT2,RMSQ, AP(1),AE(1),UP(1),UE(1),TE(1),THMOLL(1)
      real*8 RMT2,  RMSQ,  AP,  AE,  UP,  UE,  TE,  THMOLL
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      real*8 eta_array(0:1,0: 31), c_array(0:1,0: 31),g_array(0:1,0: 31)
     *, earray(0: 31),tmp_array(0: 31), sum_Z2,sum_Z,sum_A,sum_pz,Z,tmp,
     *Z23,g_m,g_r,sig,dedx, tau,tauc,beta2,eta,gamma,fmax, eil,e,si1e,si
     *2e,si1p,si2p,aae,etap, elarray(0: 31),farray(0: 31), af(0: 31),bf(
     *0: 31),cf(0: 31), df(0: 31),spline,dloge,eloge
      real*4 dum1,dum2,dum3,aux_o
      real*4 fmax_array(0:15)
      integer*2 i2_array(512),ii2
      integer*4 iq,i,j,k,i_ele,iii,iZ,iiZ,n_ener,n_q,n_point,je,neke, nd
     *ata,leil,length,ii4,irec
      character spin_file*256
      character*6 string
      integer*4 lnblnk1
      integer*4 spin_unit, rec_length, want_spin_unit
      integer egs_get_unit
      character data_version*32,endianess*4
      logical swap
      real*8 fine,TF_constant
      parameter (fine=137.03604,TF_constant=0.88534138)
      real*4 tmp_4
      character c_2(2), c_4(4)
      equivalence (ii2,c_2), (tmp_4,c_4)
      DO 7401 i=1,len(spin_file)
        spin_file(i:i) = ' '
7401  CONTINUE
7402  CONTINUE
      spin_file = hen_house(:lnblnk1(hen_house)) // 'data' // '/' // 'sp
     *inms.data'
      want_spin_unit = 61
      spin_unit = egs_get_unit(want_spin_unit)
      IF (( spin_unit .LT. 1 )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) 'init_spin: failed to get a free fortran unit'
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      rec_length = 276*4
      open(spin_unit,file=spin_file,form='unformatted',access='direct',
     *status='old',recl=rec_length,err=7410)
      read(spin_unit,rec=1,err=7420) data_version,endianess, espin_min,e
     *spin_max,b2spin_min,b2spin_max
      swap = endianess.ne.'1234'
      IF (( swap )) THEN
        tmp_4 = espin_min
        call egs_swap_4(c_4)
        espin_min = tmp_4
        tmp_4 = espin_max
        call egs_swap_4(c_4)
        espin_max = tmp_4
        tmp_4 = b2spin_min
        call egs_swap_4(c_4)
        b2spin_min = tmp_4
        tmp_4 = b2spin_max
        call egs_swap_4(c_4)
        b2spin_max = tmp_4
      END IF
      write(i_log,'(//a,a)') 'Reading spin data base from ',spin_file(:l
     *nblnk1(spin_file))
      write(i_log,'(a)') data_version
      write(i_log,'(a,a,a)') 'Data generated on a machine with ',endiane
     *ss, ' endianess'
      write(i_log,'(a,a)') 'The endianess of this CPU is ','1234'
      IF((swap))write(i_log,'(a)') '=> will need to do byte swaping'
      write(i_log,'(a,2f9.2,2f9.5,//)') 'Ranges: ',espin_min,espin_max,
     *b2spin_min,b2spin_max
      n_ener = 15
      n_q = 15
      n_point = 31
      dloge = log(espin_max/espin_min)/n_ener
      eloge = log(espin_min)
      earray(0) = espin_min
      IF (( fool_intel_optimizer )) THEN
        write(25,*) 'Energy grid:'
      END IF
      DO 7431 i=1,n_ener
        eloge = eloge + dloge
        earray(i) = exp(eloge)
        IF (( fool_intel_optimizer )) THEN
          write(25,*) i,earray(i)
        END IF
7431  CONTINUE
7432  CONTINUE
      dbeta2 = (b2spin_max - b2spin_min)/n_ener
      beta2 = b2spin_min
      earray(n_ener+1) = espin_max
      DO 7441 i=n_ener+2,2*n_ener+1
        beta2 = beta2 + dbeta2
        IF (( beta2 .LT. 0.999 )) THEN
          earray(i) = prm*1000.0*(1/sqrt(1-beta2)-1)
        ELSE
          earray(i) = 50585.1
        END IF
        IF (( fool_intel_optimizer )) THEN
          write(25,*) i,earray(i)
        END IF
7441  CONTINUE
7442  CONTINUE
      espin_min = espin_min/1000
      espin_max = espin_max/1000
      dlener = Log(espin_max/espin_min)/15
      dleneri = 1/dlener
      espml = Log(espin_min)
      dbeta2 = (b2spin_max-b2spin_min)/15
      dbeta2i = 1/dbeta2
      dqq1 = 0.5/15
      dqq1i = 1/dqq1
      DO 7451 medium=1,NMED
        write(i_log,'(a,i4,a,$)') '  medium ',medium,' .................
     *.... '
        DO 7461 iq=0,1
          DO 7471 i=0, 31
            eta_array(iq,i)=0
            c_array(iq,i)=0
            g_array(iq,i)=0
            DO 7481 j=0,15
              DO 7491 k=0,31
                spin_rej(medium,iq,i,j,k) = 0
7491          CONTINUE
7492          CONTINUE
7481        CONTINUE
7482        CONTINUE
7471      CONTINUE
7472      CONTINUE
7461    CONTINUE
7462    CONTINUE
        sum_Z2=0
        sum_A=0
        sum_pz=0
        sum_Z=0
        DO 7501 i_ele=1,NNE(medium)
          Z = ZELEM(medium,i_ele)
          iZ = int(Z+0.5)
          IF (( fool_intel_optimizer )) THEN
            write(25,*) ' Z = ',iZ
          END IF
          tmp = PZ(medium,i_ele)*Z*(Z+1)
          sum_Z2 = sum_Z2 + tmp
          sum_Z = sum_Z + PZ(medium,i_ele)*Z
          sum_A = sum_A + PZ(medium,i_ele)*WA(medium,i_ele)
          sum_pz = sum_pz + PZ(medium,i_ele)
          Z23 = Z**0.6666667
          DO 7511 iq=0,1
            DO 7521 i=0, 31
              irec = 1 + (iz-1)*4*(n_ener+1) + 2*iq*(n_ener+1) + i+1
              IF (( fool_intel_optimizer )) THEN
                write(25,*) '**** energy ',i,earray(i),irec
              END IF
              read(spin_unit,rec=irec,err=7420) dum1,dum2,dum3,aux_o,fma
     *        x_array,i2_array
              IF (( swap )) THEN
                tmp_4 = dum1
                call egs_swap_4(c_4)
                dum1 = tmp_4
                tmp_4 = dum2
                call egs_swap_4(c_4)
                dum2 = tmp_4
                tmp_4 = dum3
                call egs_swap_4(c_4)
                dum3 = tmp_4
                tmp_4 = aux_o
                call egs_swap_4(c_4)
                aux_o = tmp_4
              END IF
              eta_array(iq,i)=eta_array(iq,i)+tmp*Log(Z23*aux_o)
              tau = earray(i)/prm*0.001
              beta2 = tau*(tau+2)/(tau+1)**2
              eta = Z23/(fine*TF_constant)**2*aux_o/4/tau/(tau+2)
              c_array(iq,i)=c_array(iq,i)+ tmp*(Log(1+1/eta)-1/(1+eta))*
     *        dum1*dum3
              g_array(iq,i)=g_array(iq,i)+tmp*dum2
              DO 7531 j=0,15
                tmp_4 = fmax_array(j)
                IF((swap))call egs_swap_4(c_4)
                DO 7541 k=0,31
                  ii2 = i2_array((n_point+1)*j + k+1)
                  IF((swap))call egs_swap_2(c_2)
                  ii4 = ii2
                  IF((ii4 .LT. 0))ii4 = ii4 + 65536
                  dum1 = ii4
                  dum1 = dum1*tmp_4/65535
                  spin_rej(medium,iq,i,j,k) = spin_rej(medium,iq,i,j,k)
     *            + tmp*dum1
7541            CONTINUE
7542            CONTINUE
7531          CONTINUE
7532          CONTINUE
7521        CONTINUE
7522        CONTINUE
7511      CONTINUE
7512      CONTINUE
7501    CONTINUE
7502    CONTINUE
        DO 7551 iq=0,1
          DO 7561 i=0, 31
            DO 7571 j=0,15
              fmax = 0
              DO 7581 k=0,31
                IF (( spin_rej(medium,iq,i,j,k) .GT. fmax )) THEN
                  fmax = spin_rej(medium,iq,i,j,k)
                END IF
7581          CONTINUE
7582          CONTINUE
              DO 7591 k=0,31
                spin_rej(medium,iq,i,j,k) = spin_rej(medium,iq,i,j,k)/fm
     *          ax
7591          CONTINUE
7592          CONTINUE
7571        CONTINUE
7572        CONTINUE
7561      CONTINUE
7562      CONTINUE
7551    CONTINUE
7552    CONTINUE
        IF (( fool_intel_optimizer )) THEN
          write(25,*) 'Spin corrections as read in from file'
        END IF
        DO 7601 i=0, 31
          tau = earray(i)/prm*0.001
          beta2 = tau*(tau+2)/(tau+1)**2
          DO 7611 iq=0,1
            aux_o = Exp(eta_array(iq,i)/sum_Z2)/(fine*TF_constant)**2
            eta_array(iq,i) = 0.26112447*aux_o*blcc(medium)/xcc(medium)
            eta = aux_o/4/tau/(tau+2)
            gamma = 3*(1+eta)*(Log(1+1/eta)*(1+2*eta)-2)/ (Log(1+1/eta)*
     *      (1+eta)-1)
            g_array(iq,i) = g_array(iq,i)/sum_Z2/gamma
            c_array(iq,i) = c_array(iq,i)/sum_Z2/(Log(1+1/eta)-1/(1+eta)
     *      )
7611      CONTINUE
7612      CONTINUE
          IF (( fool_intel_optimizer )) THEN
            write(25,*) i,earray(i),eta_array(0,i),eta_array(1,i), c_arr
     *      ay(0,i),c_array(1,i),g_array(0,i),g_array(1,i)
          END IF
7601    CONTINUE
7602    CONTINUE
        eil = (1 - eke0(medium))/eke1(medium)
        e = Exp(eil)
        IF (( e .LE. espin_min )) THEN
          si1e = eta_array(0,0)
          si1p = eta_array(1,0)
        ELSE
          IF (( e .LE. espin_max )) THEN
            aae = (eil-espml)*dleneri
            je = aae
            aae = aae - je
          ELSE
            tau = e/prm
            beta2 = tau*(tau+2)/(tau+1)**2
            aae = (beta2 - b2spin_min)*dbeta2i
            je = aae
            aae = aae - je
            je = je + 15 + 1
          END IF
          si1e = (1-aae)*eta_array(0,je) + aae*eta_array(0,je+1)
          si1p = (1-aae)*eta_array(1,je) + aae*eta_array(1,je+1)
        END IF
        neke = meke(medium)
        IF (( fool_intel_optimizer )) THEN
          write(25,*) 'Interpolation table for eta correction'
        END IF
        DO 7621 i=1,neke - 1
          eil = (i+1 - eke0(medium))/eke1(medium)
          e = Exp(eil)
          IF (( e .LE. espin_min )) THEN
            si2e = eta_array(0,0)
            si2p = eta_array(1,0)
          ELSE
            IF (( e .LE. espin_max )) THEN
              aae = (eil-espml)*dleneri
              je = aae
              aae = aae - je
            ELSE
              tau = e/prm
              beta2 = tau*(tau+2)/(tau+1)**2
              aae = (beta2 - b2spin_min)*dbeta2i
              je = aae
              aae = aae - je
              je = je + 15 + 1
            END IF
            si2e = (1-aae)*eta_array(0,je) + aae*eta_array(0,je+1)
            si2p = (1-aae)*eta_array(1,je) + aae*eta_array(1,je+1)
          END IF
          etae_ms1(i,medium) = (si2e - si1e)*eke1(medium)
          etae_ms0(i,medium) = si2e - etae_ms1(i,medium)*eil
          etap_ms1(i,medium) = (si2p - si1p)*eke1(medium)
          etap_ms0(i,medium) = si2p - etap_ms1(i,medium)*eil
          IF (( fool_intel_optimizer )) THEN
            write(25,*) i,e,si2e,si2p,etae_ms1(i,medium), etae_ms0(i,med
     *      ium),etap_ms1(i,medium),etap_ms0(i,medium)
          END IF
          si1e = si2e
          si1p = si2p
7621    CONTINUE
7622    CONTINUE
        etae_ms1(neke,medium) = etae_ms1(neke-1,medium)
        etae_ms0(neke,medium) = etae_ms0(neke-1,medium)
        etap_ms1(neke,medium) = etap_ms1(neke-1,medium)
        etap_ms0(neke,medium) = etap_ms0(neke-1,medium)
        IF (( fool_intel_optimizer )) THEN
          write(25,*) 'elarray:'
        END IF
        DO 7631 i=0,15
          elarray(i) = Log(earray(i)/1000)
          farray(i) = c_array(0,i)
          IF (( fool_intel_optimizer )) THEN
            write(25,*) elarray(i),earray(i)
          END IF
7631    CONTINUE
7632    CONTINUE
        DO 7641 i=15+1, 31-1
          elarray(i) = Log(earray(i+1)/1000)
          farray(i) = c_array(0,i+1)
          IF (( fool_intel_optimizer )) THEN
            write(25,*) elarray(i),earray(i+1)
          END IF
7641    CONTINUE
7642    CONTINUE
        ndata =  31+1
        IF (( ue(medium) .GT. 1e5 )) THEN
          elarray(ndata-1) = Log(ue(medium))
        ELSE
          elarray(ndata-1) = Log(1e5)
        END IF
        farray(ndata-1) = 1
        call set_spline(elarray,farray,af,bf,cf,df,ndata)
        eil = (1 - eke0(medium))/eke1(medium)
        si1e = spline(eil,elarray,af,bf,cf,df,ndata)
        IF (( fool_intel_optimizer )) THEN
          write(25,*) 'Interpolation table for q1 correction (e-)'
        END IF
        DO 7651 i=1,neke-1
          eil = (i+1 - eke0(medium))/eke1(medium)
          si2e = spline(eil,elarray,af,bf,cf,df,ndata)
          q1ce_ms1(i,medium) = (si2e - si1e)*eke1(medium)
          q1ce_ms0(i,medium) = si2e - q1ce_ms1(i,medium)*eil
          IF (( fool_intel_optimizer )) THEN
            write(25,*) Exp(eil),si2e,q1ce_ms1(i,medium), q1ce_ms0(i,med
     *      ium)
          END IF
          si1e = si2e
7651    CONTINUE
7652    CONTINUE
        q1ce_ms1(neke,medium) = q1ce_ms1(neke-1,medium)
        q1ce_ms0(neke,medium) = q1ce_ms0(neke-1,medium)
        IF (( fool_intel_optimizer )) THEN
          write(25,*) 'Postrons:'
        END IF
        DO 7661 i=0,15
          farray(i) = c_array(1,i)
7661    CONTINUE
7662    CONTINUE
        DO 7671 i=15+1, 31-1
          farray(i) = c_array(1,i+1)
7671    CONTINUE
7672    CONTINUE
        call set_spline(elarray,farray,af,bf,cf,df,ndata)
        eil = (1 - eke0(medium))/eke1(medium)
        si1e = spline(eil,elarray,af,bf,cf,df,ndata)
        IF (( fool_intel_optimizer )) THEN
          write(25,*) 'Interpolation table for q1 correction (e+)'
        END IF
        DO 7681 i=1,neke-1
          eil = (i+1 - eke0(medium))/eke1(medium)
          si2e = spline(eil,elarray,af,bf,cf,df,ndata)
          q1cp_ms1(i,medium) = (si2e - si1e)*eke1(medium)
          q1cp_ms0(i,medium) = si2e - q1cp_ms1(i,medium)*eil
          IF (( fool_intel_optimizer )) THEN
            write(25,*) Exp(eil),si2e,q1cp_ms1(i,medium), q1cp_ms0(i,med
     *      ium)
          END IF
          si1e = si2e
7681    CONTINUE
7682    CONTINUE
        q1cp_ms1(neke,medium) = q1cp_ms1(neke-1,medium)
        q1cp_ms0(neke,medium) = q1cp_ms0(neke-1,medium)
        DO 7691 i=0,15
          farray(i) = g_array(0,i)
7691    CONTINUE
7692    CONTINUE
        DO 7701 i=15+1, 31-1
          farray(i) = g_array(0,i+1)
7701    CONTINUE
7702    CONTINUE
        call set_spline(elarray,farray,af,bf,cf,df,ndata)
        eil = (1 - eke0(medium))/eke1(medium)
        si1e = spline(eil,elarray,af,bf,cf,df,ndata)
        IF (( fool_intel_optimizer )) THEN
          write(25,*) 'Interpolation table for q2 correction (e-)'
        END IF
        DO 7711 i=1,neke-1
          eil = (i+1 - eke0(medium))/eke1(medium)
          si2e = spline(eil,elarray,af,bf,cf,df,ndata)
          q2ce_ms1(i,medium) = (si2e - si1e)*eke1(medium)
          q2ce_ms0(i,medium) = si2e - q2ce_ms1(i,medium)*eil
          IF (( fool_intel_optimizer )) THEN
            write(25,*) Exp(eil),si2e,q2ce_ms1(i,medium), q2ce_ms0(i,med
     *      ium)
          END IF
          si1e = si2e
7711    CONTINUE
7712    CONTINUE
        q2ce_ms1(neke,medium) = q2ce_ms1(neke-1,medium)
        q2ce_ms0(neke,medium) = q2ce_ms0(neke-1,medium)
        DO 7721 i=0,15
          farray(i) = g_array(1,i)
7721    CONTINUE
7722    CONTINUE
        DO 7731 i=15+1, 31-1
          farray(i) = g_array(1,i+1)
7731    CONTINUE
7732    CONTINUE
        call set_spline(elarray,farray,af,bf,cf,df,ndata)
        eil = (1 - eke0(medium))/eke1(medium)
        si1e = spline(eil,elarray,af,bf,cf,df,ndata)
        IF (( fool_intel_optimizer )) THEN
          write(25,*) 'Interpolation table for q2 correction (e+)'
        END IF
        DO 7741 i=1,neke-1
          eil = (i+1 - eke0(medium))/eke1(medium)
          si2e = spline(eil,elarray,af,bf,cf,df,ndata)
          q2cp_ms1(i,medium) = (si2e - si1e)*eke1(medium)
          q2cp_ms0(i,medium) = si2e - q2cp_ms1(i,medium)*eil
          IF (( fool_intel_optimizer )) THEN
            write(25,*) Exp(eil),si2e,q2cp_ms1(i,medium), q2cp_ms0(i,med
     *      ium)
          END IF
          si1e = si2e
7741    CONTINUE
7742    CONTINUE
        q2cp_ms1(neke,medium) = q2cp_ms1(neke-1,medium)
        q2cp_ms0(neke,medium) = q2cp_ms0(neke-1,medium)
        tauc = te(medium)/prm
        si1e = 1
        DO 7751 i=1,neke-1
          eil = (i+1 - eke0(medium))/eke1(medium)
          e = Exp(eil)
          leil=i+1
          tau=e/prm
          IF (( tau .GT. 2*tauc )) THEN
            sig=esig1(Leil,MEDIUM)*eil+esig0(Leil,MEDIUM)
            dedx=ededx1(Leil,MEDIUM)*eil+ededx0(Leil,MEDIUM)
            sig = sig/dedx
            IF (( sig .GT. 1e-6 )) THEN
              etap=etae_ms1(Leil,MEDIUM)*eil+etae_ms0(Leil,MEDIUM)
              eta = 0.25*etap*xcc(medium)/blcc(medium)/tau/(tau+2)
              g_r = (1+2*eta)*Log(1+1/eta)-2
              g_m = Log(0.5*tau/tauc)+ (1+((tau+2)/(tau+1))**2)*Log(2*(t
     *        au-tauc+2)/(tau+4))- 0.25*(tau+2)*(tau+2+2*(2*tau+1)/(tau+
     *        1)**2)* Log((tau+4)*(tau-tauc)/tau/(tau-tauc+2))+ 0.5*(tau
     *        -2*tauc)*(tau+2)*(1/(tau-tauc)-1/(tau+1)**2)
              IF (( g_m .LT. g_r )) THEN
                g_m = g_m/g_r
              ELSE
                g_m = 1
              END IF
              si2e = 1 - g_m*sum_Z/sum_Z2
            ELSE
              si2e = 1
            END IF
          ELSE
            si2e = 1
          END IF
          blcce1(i,medium) = (si2e - si1e)*eke1(medium)
          blcce0(i,medium) = si2e - blcce1(i,medium)*eil
          si1e = si2e
7751    CONTINUE
7752    CONTINUE
        blcce1(neke,medium) = blcce1(neke-1,medium)
        blcce0(neke,medium) = blcce0(neke-1,medium)
        write(i_log,'(a)') ' done'
7451  CONTINUE
7452  CONTINUE
      close(spin_unit)
      return
7410  write(i_log,'(/a)') '***************** Error: '
      write(i_log,'(a,a)') 'Failed to open spin data file ',spin_file(:l
     *nblnk1(spin_file))
      write(i_log,'(/a)') '***************** Quiting now.'
      call exit(1)
7420  write(i_log,'(/a)') '***************** Error: '
      write(i_log,*) 'Error while reading spin data file for element',iZ
      write(i_log,'(/a)') '***************** Quiting now.'
      call exit(1)
      return
      end
      subroutine init_spin_old
      implicit none
      common/spin_data/ spin_rej(1,0:1,0: 31,0:15,0:31), espin_min,espin
     *_max,espml,b2spin_min,b2spin_max, dbeta2,dbeta2i,dlener,dleneri,dq
     *q1,dqq1i, fool_intel_optimizer
      real*4 spin_rej,espin_min,espin_max,espml,b2spin_min,b2spin_max, d
     *beta2,dbeta2i,dlener,dleneri,dqq1,dqq1i
      logical fool_intel_optimizer
      COMMON/ELECIN/ esig_e(1),psig_e(1), esige_max, psige_max, range_ep
     *(0:1,500,1), E_array(500,1), etae_ms0(500,1),etae_ms1(500,1),etap_
     *ms0(500,1),etap_ms1(500,1),q1ce_ms0(500,1),q1ce_ms1(500,1),q1cp_ms
     *0(500,1),q1cp_ms1(500,1),q2ce_ms0(500,1),q2ce_ms1(500,1),q2cp_ms0(
     *500,1),q2cp_ms1(500,1),blcce0(500,1),blcce1(500,1), EKE0(1),EKE1(1
     *), XR0(1),TEFF0(1),BLCC(1),XCC(1), ESIG0(500,1),ESIG1(500,1),PSIG0
     *(500,1),PSIG1(500,1),EDEDX0(500,1),EDEDX1(500,1),PDEDX0(500,1),PDE
     *DX1(500,1),EBR10(500,1),EBR11(500,1),PBR10(500,1),PBR11(500,1),PBR
     *20(500,1),PBR21(500,1),TMXS0(500,1),TMXS1(500,1), expeke1(1), IUNR
     *ST(1),EPSTFL(1),IAPRIM(1), sig_ismonotone(0:1,1)
      real*8 esig_e,   psig_e,   esige_max,  psige_max,  range_ep,  E_ar
     *ray,  etae_ms0,etae_ms1,  etap_ms0,etap_ms1,  q1ce_ms0,q1ce_ms1,
     *q1cp_ms0,q1cp_ms1,  q2ce_ms0,q2ce_ms1,  q2cp_ms0,q2cp_ms1,  blcce0
     *,blcce1,   expeke1,  EKE0,EKE1, XR0,  TEFF0,  BLCC,  XCC,  ESIG0,E
     *SIG1,  PSIG0,PSIG1,  EDEDX0,EDEDX1,  PDEDX0,PDEDX1,  EBR10,EBR11,
     * PBR10,PBR11,  PBR20,PBR21,  TMXS0,TMXS1
      integer*4 IUNRST,  EPSTFL,  IAPRIM
      logical sig_ismonotone
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      COMMON/BREMPR/ DL1(8,1),DL2(8,1),DL3(8,1),DL4(8,1),DL5(8,1),DL6(8,
     *1), ALPHI(2,1),BPAR(2,1),DELPOS(2,1), WA(1,50),PZ(1,50),ZELEM(1,50
     *),RHOZ(1,50), PWR2I(50), DELCM(1),ZBRANG(1),LZBRANG(1),NNE(1), IBR
     *DST,IPRDST,ibr_nist,pair_nrc,itriplet, ASYM(1,50,2)
      CHARACTER*4 ASYM
      real*8 DL1,DL2,DL3,DL4,DL5,DL6,   ALPHI,  BPAR,  DELPOS,  WA,  PZ,
     *  ZELEM,  RHOZ,  PWR2I,  DELCM,  ZBRANG,  LZBRANG
      integer*4 NNE,  IBRDST,  IPRDST,  ibr_nist,  itriplet,  pair_nrc
      COMMON/THRESH/RMT2,RMSQ, AP(1),AE(1),UP(1),UE(1),TE(1),THMOLL(1)
      real*8 RMT2,  RMSQ,  AP,  AE,  UP,  UE,  TE,  THMOLL
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      real*8 eta_array(0:1,0: 31), c_array(0:1,0: 31),g_array(0:1,0: 31)
     *, earray(0: 31),tmp_array(0: 31), sum_Z2,sum_Z,sum_A,sum_pz,Z,tmp,
     *Z23,g_m,g_r,sig,dedx, dum1,dum2,dum3,aux_o,tau,tauc,beta2,eta,gamm
     *a,fmax, eil,e,si1e,si2e,si1p,si2p,aae,etap, elarray(0: 31),farray(
     *0: 31), af(0: 31),bf(0: 31),cf(0: 31), df(0: 31),spline
      integer*4 iq,i,j,k,i_ele,iii,iZ,iiZ,n_ener,n_q,n_point,je,neke, nd
     *ata,leil,length,want_spin_unit,spin_unit,egs_get_unit
      character spin_file*256
      character*6 string
      integer*4 lnblnk1
      real*8 fine,TF_constant
      parameter (fine=137.03604,TF_constant=0.88534138)
      DO 7761 i=1,len(spin_file)
        spin_file(i:i) = ' '
7761  CONTINUE
7762  CONTINUE
      spin_file = hen_house(:lnblnk1(hen_house)) // 'data' // '/' // 'sp
     *inms' // '/' // 'z000'
      length = lnblnk1(spin_file)
      DO 7771 medium=1,NMED
        write(i_log,'(a,i4,a,$)') '  Initializing spin data for medium '
     *  ,medium, ' ..................... '
        DO 7781 iq=0,1
          DO 7791 i=0, 31
            eta_array(iq,i)=0
            c_array(iq,i)=0
            g_array(iq,i)=0
            DO 7801 j=0,15
              DO 7811 k=0,31
                spin_rej(medium,iq,i,j,k) = 0
7811          CONTINUE
7812          CONTINUE
7801        CONTINUE
7802        CONTINUE
7791      CONTINUE
7792      CONTINUE
7781    CONTINUE
7782    CONTINUE
        sum_Z2=0
        sum_A=0
        sum_pz=0
        sum_Z=0
        DO 7821 i_ele=1,NNE(medium)
          Z = ZELEM(medium,i_ele)
          iZ = int(Z+0.5)
          tmp = PZ(medium,i_ele)*Z*(Z+1)
          iii = iZ/100
          spin_file(length-2:length-2) = char(iii+48)
          iiZ = iZ - iii*100
          iii = iiZ/10
          spin_file(length-1:length-1) = char(iii+48)
          iiZ = iiZ - 10*iii
          spin_file(length:length) = char(iiZ+48)
          want_spin_unit = 61
          spin_unit = egs_get_unit(want_spin_unit)
          IF (( spin_unit .LT. 1 )) THEN
            write(i_log,'(/a)') '***************** Error: '
            write(i_log,*) 'init_spin: failed to get a free fortran unit
     *'
            write(i_log,'(/a)') '***************** Quiting now.'
            call exit(1)
          END IF
          open(spin_unit,file=spin_file,status='old',err=7830)
          read(spin_unit,*) espin_min,espin_max,b2spin_min,b2spin_max
          read(spin_unit,*) n_ener,n_q,n_point
          IF (( n_ener .NE. 15 .OR. n_q .NE. 15 .OR. n_point .NE. 31)) T
     *    HEN
            write(i_log,'(/a)') '***************** Error: '
            write(i_log,*) ' Wrong spin file for Z = ',iZ
            write(i_log,'(/a)') '***************** Quiting now.'
            call exit(1)
          END IF
          sum_Z2 = sum_Z2 + tmp
          sum_Z = sum_Z + PZ(medium,i_ele)*Z
          sum_A = sum_A + PZ(medium,i_ele)*WA(medium,i_ele)
          sum_pz = sum_pz + PZ(medium,i_ele)
          Z23 = Z**0.6666667
          DO 7841 iq=0,1
            read(spin_unit,*)
            read(spin_unit,*)
            DO 7851 i=0, 31
              read(spin_unit,'(a,g14.6)') string,earray(i)
              read(spin_unit,*) dum1,dum2,dum3,aux_o
              eta_array(iq,i)=eta_array(iq,i)+tmp*Log(Z23*aux_o)
              tau = earray(i)/prm*0.001
              beta2 = tau*(tau+2)/(tau+1)**2
              eta = Z23/(fine*TF_constant)**2*aux_o/4/tau/(tau+2)
              c_array(iq,i)=c_array(iq,i)+ tmp*(Log(1+1/eta)-1/(1+eta))*
     *        dum1*dum3
              g_array(iq,i)=g_array(iq,i)+tmp*dum2
              DO 7861 j=0,15
                read(spin_unit,*) tmp_array
                DO 7871 k=0,31
                  spin_rej(medium,iq,i,j,k) = spin_rej(medium,iq,i,j,k)
     *            + tmp*tmp_array(k)
7871            CONTINUE
7872            CONTINUE
7861          CONTINUE
7862          CONTINUE
7851        CONTINUE
7852        CONTINUE
7841      CONTINUE
7842      CONTINUE
          close(spin_unit)
7821    CONTINUE
7822    CONTINUE
        DO 7881 iq=0,1
          DO 7891 i=0, 31
            DO 7901 j=0,15
              fmax = 0
              DO 7911 k=0,31
                IF (( spin_rej(medium,iq,i,j,k) .GT. fmax )) THEN
                  fmax = spin_rej(medium,iq,i,j,k)
                END IF
7911          CONTINUE
7912          CONTINUE
              DO 7921 k=0,31
                spin_rej(medium,iq,i,j,k) = spin_rej(medium,iq,i,j,k)/fm
     *          ax
7921          CONTINUE
7922          CONTINUE
7901        CONTINUE
7902        CONTINUE
7891      CONTINUE
7892      CONTINUE
7881    CONTINUE
7882    CONTINUE
        DO 7931 i=0, 31
          tau = earray(i)/prm*0.001
          beta2 = tau*(tau+2)/(tau+1)**2
          DO 7941 iq=0,1
            aux_o = Exp(eta_array(iq,i)/sum_Z2)/(fine*TF_constant)**2
            eta_array(iq,i) = 0.26112447*aux_o*blcc(medium)/xcc(medium)
            eta = aux_o/4/tau/(tau+2)
            gamma = 3*(1+eta)*(Log(1+1/eta)*(1+2*eta)-2)/ (Log(1+1/eta)*
     *      (1+eta)-1)
            g_array(iq,i) = g_array(iq,i)/sum_Z2/gamma
            c_array(iq,i) = c_array(iq,i)/sum_Z2/(Log(1+1/eta)-1/(1+eta)
     *      )
7941      CONTINUE
7942      CONTINUE
7931    CONTINUE
7932    CONTINUE
        espin_min = espin_min/1000
        espin_max = espin_max/1000
        dlener = Log(espin_max/espin_min)/15
        dleneri = 1/dlener
        espml = Log(espin_min)
        dbeta2 = (b2spin_max-b2spin_min)/15
        dbeta2i = 1/dbeta2
        dqq1 = 0.5/15
        dqq1i = 1/dqq1
        eil = (1 - eke0(medium))/eke1(medium)
        e = Exp(eil)
        IF (( e .LE. espin_min )) THEN
          si1e = eta_array(0,0)
          si1p = eta_array(1,0)
        ELSE
          IF (( e .LE. espin_max )) THEN
            aae = (eil-espml)*dleneri
            je = aae
            aae = aae - je
          ELSE
            tau = e/prm
            beta2 = tau*(tau+2)/(tau+1)**2
            aae = (beta2 - b2spin_min)*dbeta2i
            je = aae
            aae = aae - je
            je = je + 15 + 1
          END IF
          si1e = (1-aae)*eta_array(0,je) + aae*eta_array(0,je+1)
          si1p = (1-aae)*eta_array(1,je) + aae*eta_array(1,je+1)
        END IF
        neke = meke(medium)
        DO 7951 i=1,neke - 1
          eil = (i+1 - eke0(medium))/eke1(medium)
          e = Exp(eil)
          IF (( e .LE. espin_min )) THEN
            si2e = eta_array(0,0)
            si2p = eta_array(1,0)
          ELSE
            IF (( e .LE. espin_max )) THEN
              aae = (eil-espml)*dleneri
              je = aae
              aae = aae - je
            ELSE
              tau = e/prm
              beta2 = tau*(tau+2)/(tau+1)**2
              aae = (beta2 - b2spin_min)*dbeta2i
              je = aae
              aae = aae - je
              je = je + 15 + 1
            END IF
            si2e = (1-aae)*eta_array(0,je) + aae*eta_array(0,je+1)
            si2p = (1-aae)*eta_array(1,je) + aae*eta_array(1,je+1)
          END IF
          etae_ms1(i,medium) = (si2e - si1e)*eke1(medium)
          etae_ms0(i,medium) = si2e - etae_ms1(i,medium)*eil
          etap_ms1(i,medium) = (si2p - si1p)*eke1(medium)
          etap_ms0(i,medium) = si2p - etap_ms1(i,medium)*eil
          si1e = si2e
          si1p = si2p
7951    CONTINUE
7952    CONTINUE
        etae_ms1(neke,medium) = etae_ms1(neke-1,medium)
        etae_ms0(neke,medium) = etae_ms0(neke-1,medium)
        etap_ms1(neke,medium) = etap_ms1(neke-1,medium)
        etap_ms0(neke,medium) = etap_ms0(neke-1,medium)
        DO 7961 i=0,15
          elarray(i) = Log(earray(i)/1000)
          farray(i) = c_array(0,i)
7961    CONTINUE
7962    CONTINUE
        DO 7971 i=15+1, 31-1
          elarray(i) = Log(earray(i+1)/1000)
          farray(i) = c_array(0,i+1)
7971    CONTINUE
7972    CONTINUE
        ndata =  31+1
        IF (( ue(medium) .GT. 1e5 )) THEN
          elarray(ndata-1) = Log(ue(medium))
        ELSE
          elarray(ndata-1) = Log(1e5)
        END IF
        farray(ndata-1) = 1
        call set_spline(elarray,farray,af,bf,cf,df,ndata)
        eil = (1 - eke0(medium))/eke1(medium)
        si1e = spline(eil,elarray,af,bf,cf,df,ndata)
        DO 7981 i=1,neke-1
          eil = (i+1 - eke0(medium))/eke1(medium)
          si2e = spline(eil,elarray,af,bf,cf,df,ndata)
          q1ce_ms1(i,medium) = (si2e - si1e)*eke1(medium)
          q1ce_ms0(i,medium) = si2e - q1ce_ms1(i,medium)*eil
          si1e = si2e
7981    CONTINUE
7982    CONTINUE
        q1ce_ms1(neke,medium) = q1ce_ms1(neke-1,medium)
        q1ce_ms0(neke,medium) = q1ce_ms0(neke-1,medium)
        DO 7991 i=0,15
          farray(i) = c_array(1,i)
7991    CONTINUE
7992    CONTINUE
        DO 8001 i=15+1, 31-1
          farray(i) = c_array(1,i+1)
8001    CONTINUE
8002    CONTINUE
        call set_spline(elarray,farray,af,bf,cf,df,ndata)
        eil = (1 - eke0(medium))/eke1(medium)
        si1e = spline(eil,elarray,af,bf,cf,df,ndata)
        DO 8011 i=1,neke-1
          eil = (i+1 - eke0(medium))/eke1(medium)
          si2e = spline(eil,elarray,af,bf,cf,df,ndata)
          q1cp_ms1(i,medium) = (si2e - si1e)*eke1(medium)
          q1cp_ms0(i,medium) = si2e - q1cp_ms1(i,medium)*eil
          si1e = si2e
8011    CONTINUE
8012    CONTINUE
        q1cp_ms1(neke,medium) = q1cp_ms1(neke-1,medium)
        q1cp_ms0(neke,medium) = q1cp_ms0(neke-1,medium)
        DO 8021 i=0,15
          farray(i) = g_array(0,i)
8021    CONTINUE
8022    CONTINUE
        DO 8031 i=15+1, 31-1
          farray(i) = g_array(0,i+1)
8031    CONTINUE
8032    CONTINUE
        call set_spline(elarray,farray,af,bf,cf,df,ndata)
        eil = (1 - eke0(medium))/eke1(medium)
        si1e = spline(eil,elarray,af,bf,cf,df,ndata)
        DO 8041 i=1,neke-1
          eil = (i+1 - eke0(medium))/eke1(medium)
          si2e = spline(eil,elarray,af,bf,cf,df,ndata)
          q2ce_ms1(i,medium) = (si2e - si1e)*eke1(medium)
          q2ce_ms0(i,medium) = si2e - q2ce_ms1(i,medium)*eil
          si1e = si2e
8041    CONTINUE
8042    CONTINUE
        q2ce_ms1(neke,medium) = q2ce_ms1(neke-1,medium)
        q2ce_ms0(neke,medium) = q2ce_ms0(neke-1,medium)
        DO 8051 i=0,15
          farray(i) = g_array(1,i)
8051    CONTINUE
8052    CONTINUE
        DO 8061 i=15+1, 31-1
          farray(i) = g_array(1,i+1)
8061    CONTINUE
8062    CONTINUE
        call set_spline(elarray,farray,af,bf,cf,df,ndata)
        eil = (1 - eke0(medium))/eke1(medium)
        si1e = spline(eil,elarray,af,bf,cf,df,ndata)
        DO 8071 i=1,neke-1
          eil = (i+1 - eke0(medium))/eke1(medium)
          si2e = spline(eil,elarray,af,bf,cf,df,ndata)
          q2cp_ms1(i,medium) = (si2e - si1e)*eke1(medium)
          q2cp_ms0(i,medium) = si2e - q2cp_ms1(i,medium)*eil
8071    CONTINUE
8072    CONTINUE
        q2cp_ms1(neke,medium) = q2cp_ms1(neke-1,medium)
        q2cp_ms0(neke,medium) = q2cp_ms0(neke-1,medium)
        tauc = te(medium)/prm
        si1e = 1
        DO 8081 i=1,neke-1
          eil = (i+1 - eke0(medium))/eke1(medium)
          e = Exp(eil)
          leil=i+1
          tau=e/prm
          IF (( tau .GT. 2*tauc )) THEN
            sig=esig1(Leil,MEDIUM)*eil+esig0(Leil,MEDIUM)
            dedx=ededx1(Leil,MEDIUM)*eil+ededx0(Leil,MEDIUM)
            sig = sig/dedx
            IF (( sig .GT. 1e-6 )) THEN
              etap=etae_ms1(Leil,MEDIUM)*eil+etae_ms0(Leil,MEDIUM)
              eta = 0.25*etap*xcc(medium)/blcc(medium)/tau/(tau+2)
              g_r = (1+2*eta)*Log(1+1/eta)-2
              g_m = Log(0.5*tau/tauc)+ (1+((tau+2)/(tau+1))**2)*Log(2*(t
     *        au-tauc+2)/(tau+4))- 0.25*(tau+2)*(tau+2+2*(2*tau+1)/(tau+
     *        1)**2)* Log((tau+4)*(tau-tauc)/tau/(tau-tauc+2))+ 0.5*(tau
     *        -2*tauc)*(tau+2)*(1/(tau-tauc)-1/(tau+1)**2)
              IF (( g_m .LT. g_r )) THEN
                g_m = g_m/g_r
              ELSE
                g_m = 1
              END IF
              si2e = 1 - g_m*sum_Z/sum_Z2
            ELSE
              si2e = 1
            END IF
          ELSE
            si2e = 1
          END IF
          blcce1(i,medium) = (si2e - si1e)*eke1(medium)
          blcce0(i,medium) = si2e - blcce1(i,medium)*eil
          si1e = si2e
8081    CONTINUE
8082    CONTINUE
        blcce1(neke,medium) = blcce1(neke-1,medium)
        blcce0(neke,medium) = blcce0(neke-1,medium)
        write(i_log,'(a)') ' done'
7771  CONTINUE
7772  CONTINUE
      return
7830  write(i_log,*) ' ******************** Error in init_spin *********
     *********** '
      write(i_log,'(a,a)') '  could not open file ',spin_file
      write(i_log,'(/a)') '***************** Error: '
      write(i_log,*) ' terminating execution '
      write(i_log,'(/a)') '***************** Quiting now.'
      call exit(1)
      end
      subroutine msdist_pII ( e0,eloss,tustep,rhof,med,qel,spin_effects,
     *u0,v0,w0,x0,y0,z0,  us,vs,ws,xf,yf,zf,ustep )
      implicit none
      real*8 e0,  eloss,  rhof,  tustep,  u0,  v0,  w0,  x0,  y0,  z0
      integer*4 med, qel
      logical spin_effects
      real*8 us,  vs,  ws,  xf,  yf,  zf,  ustep
      real*8 b,  blccc,  xcccc,  c,  eta,eta1,  chia2,  chilog,  cphi0,
     *  cphi1,  cphi2,  w1,  w2,  w1v2,  delta,  e,  elke,  beta2,  etap
     *,  xi_corr,  ms_corr, tau,  tau2,  epsilon,  epsilonp,  temp,temp1
     *, temp2,  factor,  gamma,  lambda,   p2,  p2i,  q1,  rhophi2,  sin
     *t0,  sint02,  sint0i,  sint1,  sint2,  sphi0,   sphi1,  sphi2,  u2
     *p,  u2,  v2,  ut,  vt,  wt,  xi,  xphi,  xphi2,  yphi,  yphi2
      logical find_index,  spin_index
      integer*4 lelke
      COMMON/ELECIN/ esig_e(1),psig_e(1), esige_max, psige_max, range_ep
     *(0:1,500,1), E_array(500,1), etae_ms0(500,1),etae_ms1(500,1),etap_
     *ms0(500,1),etap_ms1(500,1),q1ce_ms0(500,1),q1ce_ms1(500,1),q1cp_ms
     *0(500,1),q1cp_ms1(500,1),q2ce_ms0(500,1),q2ce_ms1(500,1),q2cp_ms0(
     *500,1),q2cp_ms1(500,1),blcce0(500,1),blcce1(500,1), EKE0(1),EKE1(1
     *), XR0(1),TEFF0(1),BLCC(1),XCC(1), ESIG0(500,1),ESIG1(500,1),PSIG0
     *(500,1),PSIG1(500,1),EDEDX0(500,1),EDEDX1(500,1),PDEDX0(500,1),PDE
     *DX1(500,1),EBR10(500,1),EBR11(500,1),PBR10(500,1),PBR11(500,1),PBR
     *20(500,1),PBR21(500,1),TMXS0(500,1),TMXS1(500,1), expeke1(1), IUNR
     *ST(1),EPSTFL(1),IAPRIM(1), sig_ismonotone(0:1,1)
      real*8 esig_e,   psig_e,   esige_max,  psige_max,  range_ep,  E_ar
     *ray,  etae_ms0,etae_ms1,  etap_ms0,etap_ms1,  q1ce_ms0,q1ce_ms1,
     *q1cp_ms0,q1cp_ms1,  q2ce_ms0,q2ce_ms1,  q2cp_ms0,q2cp_ms1,  blcce0
     *,blcce1,   expeke1,  EKE0,EKE1, XR0,  TEFF0,  BLCC,  XCC,  ESIG0,E
     *SIG1,  PSIG0,PSIG1,  EDEDX0,EDEDX1,  PDEDX0,PDEDX1,  EBR10,EBR11,
     * PBR10,PBR11,  PBR20,PBR21,  TMXS0,TMXS1
      integer*4 IUNRST,  EPSTFL,  IAPRIM
      logical sig_ismonotone
      COMMON/THRESH/RMT2,RMSQ, AP(1),AE(1),UP(1),UE(1),TE(1),THMOLL(1)
      real*8 RMT2,  RMSQ,  AP,  AE,  UP,  UE,  TE,  THMOLL
      COMMON/UPHIOT/THETA,SINTHE,COSTHE,SINPHI, COSPHI,PI,TWOPI,PI5D2
      real*8 THETA,  SINTHE,  COSTHE,  SINPHI,  COSPHI,  PI,TWOPI,PI5D2
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      common/CH_steps/ count_pII_steps,count_all_steps,is_ch_step
      real*8 count_pII_steps,count_all_steps
      logical is_ch_step
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      medium = med
      count_pII_steps = count_pII_steps + 1
      blccc = blcc(medium)
      xcccc = xcc(medium)
      e = e0 - 0.5*eloss
      tau = e/prm
      tau2 = tau*tau
      epsilon = eloss/e0
      epsilonp= eloss/e
      e = e * (1 - epsilonp*epsilonp*(6+10*tau+5*tau2)/(24*tau2+72*tau+4
     *8))
      p2 = e*(e + rmt2)
      beta2 = p2/(p2 + rmsq)
      chia2 = xcccc/(4*p2*blccc)
      lambda = 0.5*tustep*rhof*blccc/beta2
      temp2 = 0.166666*(4+tau*(6+tau*(7+tau*(4+tau))))* (epsilonp/((tau+
     *1)*(tau+2)))**2
      lambda = lambda*(1 - temp2)
      IF (( spin_effects )) THEN
        elke = Log(e)
        Lelke=eke1(MEDIUM)*elke+eke0(MEDIUM)
        IF (( lelke .LT. 1 )) THEN
          lelke = 1
          elke = (1 - eke0(medium))/eke1(medium)
        END IF
        IF (( qel .EQ. 0 )) THEN
          etap=etae_ms1(Lelke,MEDIUM)*elke+etae_ms0(Lelke,MEDIUM)
          xi_corr=q1ce_ms1(Lelke,MEDIUM)*elke+q1ce_ms0(Lelke,MEDIUM)
          gamma=q2ce_ms1(Lelke,MEDIUM)*elke+q2ce_ms0(Lelke,MEDIUM)
        ELSE
          etap=etap_ms1(Lelke,MEDIUM)*elke+etap_ms0(Lelke,MEDIUM)
          xi_corr=q1cp_ms1(Lelke,MEDIUM)*elke+q1cp_ms0(Lelke,MEDIUM)
          gamma=q2cp_ms1(Lelke,MEDIUM)*elke+q2cp_ms0(Lelke,MEDIUM)
        END IF
        ms_corr=blcce1(Lelke,MEDIUM)*elke+blcce0(Lelke,MEDIUM)
      ELSE
        etap = 1
        xi_corr = 1
        gamma = 1
        ms_corr = 1
      END IF
      chia2 = chia2*etap
      lambda = lambda/(etap*(1+chia2))*ms_corr
      chilog = Log(1 + 1/chia2)
      q1 = 2*chia2*(chilog*(1 + chia2) - 1)
      gamma = 6*chia2*(1 + chia2)*(chilog*(1 + 2*chia2) - 2)/q1*gamma
      xi = q1*lambda
      find_index = .true.
      spin_index = .true.
      call mscat(lambda,chia2,xi,elke,beta2,qel,medium, spin_effects,fin
     *d_index,spin_index, w1,sint1)
8091  CONTINUE
        IF((rng_seed .GT. 128))call ranmar_get
        xphi = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        xphi = 2*xphi - 1
        xphi2 = xphi*xphi
        IF((rng_seed .GT. 128))call ranmar_get
        yphi = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        yphi2 = yphi*yphi
        rhophi2 = xphi2 + yphi2
        IF(rhophi2.LE.1)GO TO8092
      GO TO 8091
8092  CONTINUE
      rhophi2 = 1/rhophi2
      cphi1 = (xphi2 - yphi2)*rhophi2
      sphi1 = 2*xphi*yphi*rhophi2
      call mscat(lambda,chia2,xi,elke,beta2,qel,medium, spin_effects,fin
     *d_index,spin_index, w2,sint2)
8101  CONTINUE
        IF((rng_seed .GT. 128))call ranmar_get
        xphi = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        xphi = 2*xphi - 1
        xphi2 = xphi*xphi
        IF((rng_seed .GT. 128))call ranmar_get
        yphi = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        yphi2 = yphi*yphi
        rhophi2 = xphi2 + yphi2
        IF(rhophi2.LE.1)GO TO8102
      GO TO 8101
8102  CONTINUE
      rhophi2 = 1/rhophi2
      cphi2 = (xphi2 - yphi2)*rhophi2
      sphi2 = 2*xphi*yphi*rhophi2
      u2 = sint2*cphi2
      v2 = sint2*sphi2
      u2p = w1*u2 + sint1*w2
      us = u2p*cphi1 - v2*sphi1
      vs = u2p*sphi1 + v2*cphi1
      ws = w1*w2 - sint1*u2
      xi = 2*xi*xi_corr
      IF((rng_seed .GT. 128))call ranmar_get
      eta = rng_array(rng_seed)*twom24
      rng_seed = rng_seed + 1
      eta = Sqrt(eta)
      eta1 = 0.5*(1 - eta)
      delta = 0.9082483-(0.1020621-0.0263747*gamma)*xi
      temp1 = 2 + tau
      temp = (2+tau*temp1)/((tau+1)*temp1)
      temp = temp - (tau+1)/((tau+2)*(chilog*(1+chia2)-1))
      temp = temp * epsilonp
      temp1 = 1 - temp
      delta = delta + 0.40824829*(epsilon*(tau+1)/((tau+2)* (chilog*(1+c
     *hia2)-1)*(chilog*(1+2*chia2)-2)) - 0.25*temp*temp)
      b = eta*delta
      c = eta*(1-delta)
      w1v2 = w1*v2
      ut = b*sint1*cphi1 + c*(cphi1*u2 - sphi1*w1v2) + eta1*us*temp1
      vt = b*sint1*sphi1 + c*(sphi1*u2 + cphi1*w1v2) + eta1*vs*temp1
      wt = eta1*(1+temp) + b*w1 + c*w2 + eta1*ws*temp1
      ustep = tustep*sqrt(ut*ut + vt*vt + wt*wt)
      sint02 = u0**2 + v0**2
      IF ((sint02 .GT. 1e-20)) THEN
        sint0 = sqrt(sint02)
        sint0i = 1/sint0
        cphi0 = sint0i*u0
        sphi0 = sint0i*v0
        u2p = w0*us + sint0*ws
        ws = w0*ws - sint0*us
        us = u2p*cphi0 - vs*sphi0
        vs = u2p*sphi0 + vs*cphi0
        u2p = w0*ut + sint0*wt
        wt = w0*wt - sint0*ut
        ut = u2p*cphi0 - vt*sphi0
        vt = u2p*sphi0 + vt*cphi0
      ELSE
        wt = w0*wt
        ws = w0*ws
      END IF
      xf = x0 + tustep*ut
      yf = y0 + tustep*vt
      zf = z0 + tustep*wt
      return
      end
      subroutine msdist_pI ( e0,eloss,tustep,rhof,medium,qel,spin_effect
     *s,u0,v0,w0,x0,y0,z0,  us,vs,ws,xf,yf,zf,ustep )
      implicit none
      real*8 e0,  eloss,  rhof,  tustep,  u0,  v0,  w0,  x0,  y0,  z0
      integer*4 medium, qel
      logical spin_effects
      real*8 us,  vs,  ws,  xf,  yf,  zf,  ustep
      real*8 blccc,  xcccc,  z,r,z2,r2,  r2max, chia2,  chilog,  cphi0,
     *  cphi,  sphi,  e,  elke,  beta2,  etap,  xi_corr,  ms_corr, epsil
     *on,  temp,  factor,  lambda,  p2,  p2i,  q1,  rhophi2,  sint,  sin
     *t0,  sint02,  sint0i,  sphi0,   u2p,  ut,  vt,  wt,  xi,  xphi,  x
     *phi2,  yphi,  yphi2
      logical find_index,  spin_index
      integer*4 lelke
      COMMON/ELECIN/ esig_e(1),psig_e(1), esige_max, psige_max, range_ep
     *(0:1,500,1), E_array(500,1), etae_ms0(500,1),etae_ms1(500,1),etap_
     *ms0(500,1),etap_ms1(500,1),q1ce_ms0(500,1),q1ce_ms1(500,1),q1cp_ms
     *0(500,1),q1cp_ms1(500,1),q2ce_ms0(500,1),q2ce_ms1(500,1),q2cp_ms0(
     *500,1),q2cp_ms1(500,1),blcce0(500,1),blcce1(500,1), EKE0(1),EKE1(1
     *), XR0(1),TEFF0(1),BLCC(1),XCC(1), ESIG0(500,1),ESIG1(500,1),PSIG0
     *(500,1),PSIG1(500,1),EDEDX0(500,1),EDEDX1(500,1),PDEDX0(500,1),PDE
     *DX1(500,1),EBR10(500,1),EBR11(500,1),PBR10(500,1),PBR11(500,1),PBR
     *20(500,1),PBR21(500,1),TMXS0(500,1),TMXS1(500,1), expeke1(1), IUNR
     *ST(1),EPSTFL(1),IAPRIM(1), sig_ismonotone(0:1,1)
      real*8 esig_e,   psig_e,   esige_max,  psige_max,  range_ep,  E_ar
     *ray,  etae_ms0,etae_ms1,  etap_ms0,etap_ms1,  q1ce_ms0,q1ce_ms1,
     *q1cp_ms0,q1cp_ms1,  q2ce_ms0,q2ce_ms1,  q2cp_ms0,q2cp_ms1,  blcce0
     *,blcce1,   expeke1,  EKE0,EKE1, XR0,  TEFF0,  BLCC,  XCC,  ESIG0,E
     *SIG1,  PSIG0,PSIG1,  EDEDX0,EDEDX1,  PDEDX0,PDEDX1,  EBR10,EBR11,
     * PBR10,PBR11,  PBR20,PBR21,  TMXS0,TMXS1
      integer*4 IUNRST,  EPSTFL,  IAPRIM
      logical sig_ismonotone
      COMMON/THRESH/RMT2,RMSQ, AP(1),AE(1),UP(1),UE(1),TE(1),THMOLL(1)
      real*8 RMT2,  RMSQ,  AP,  AE,  UP,  UE,  TE,  THMOLL
      COMMON/UPHIOT/THETA,SINTHE,COSTHE,SINPHI, COSPHI,PI,TWOPI,PI5D2
      real*8 THETA,  SINTHE,  COSTHE,  SINPHI,  COSPHI,  PI,TWOPI,PI5D2
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      blccc = blcc(medium)
      xcccc = xcc(medium)
      e = e0 - 0.5*eloss
      p2 = e*(e + rmt2)
      p2i = 1/p2
      chia2 = xcccc*p2i/(4*blccc)
      beta2 = p2/(p2 + rmsq)
      lambda = tustep*rhof*blccc/beta2
      factor = 1/(1 + 0.9784671*e)
      epsilon= eloss/e0
      epsilon= epsilon/(1-0.5*epsilon)
      temp = 0.25*(1 - factor*(1 - 0.333333*factor))*epsilon**2
      lambda = lambda*(1 + temp)
      IF (( spin_effects )) THEN
        elke = Log(e)
        Lelke=eke1(MEDIUM)*elke+eke0(MEDIUM)
        IF (( lelke .LT. 1 )) THEN
          lelke = 1
          elke = (1 - eke0(medium))/eke1(medium)
        END IF
        IF (( qel .EQ. 0 )) THEN
          etap=etae_ms1(Lelke,MEDIUM)*elke+etae_ms0(Lelke,MEDIUM)
          xi_corr=q1ce_ms1(Lelke,MEDIUM)*elke+q1ce_ms0(Lelke,MEDIUM)
        ELSE
          etap=etap_ms1(Lelke,MEDIUM)*elke+etap_ms0(Lelke,MEDIUM)
          xi_corr=q1cp_ms1(Lelke,MEDIUM)*elke+q1cp_ms0(Lelke,MEDIUM)
        END IF
        ms_corr=blcce1(Lelke,MEDIUM)*elke+blcce0(Lelke,MEDIUM)
      ELSE
        etap = 1
        xi_corr = 1
        ms_corr = 1
      END IF
      chia2 = xcccc*p2i/(4*blccc)*etap
      lambda = lambda/etap/(1+chia2)*ms_corr
      chilog = Log(1 + 1/chia2)
      q1 = 2*chia2*(chilog*(1 + chia2) - 1)
      xi = q1*lambda
      find_index = .true.
      spin_index = .true.
      call mscat(lambda,chia2,xi,elke,beta2,qel,medium, spin_effects,fin
     *d_index,spin_index, ws,sint)
8111  CONTINUE
        IF((rng_seed .GT. 128))call ranmar_get
        xphi = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        xphi = 2*xphi - 1
        xphi2 = xphi*xphi
        IF((rng_seed .GT. 128))call ranmar_get
        yphi = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        yphi2 = yphi*yphi
        rhophi2 = xphi2 + yphi2
        IF(rhophi2.LE.1)GO TO8112
      GO TO 8111
8112  CONTINUE
      rhophi2 = 1/rhophi2
      cphi = (xphi2 - yphi2)*rhophi2
      sphi = 2*xphi*yphi*rhophi2
      us = sint*cphi
      vs = sint*sphi
      xi = xi*xi_corr
      IF (( xi .LT. 0.1 )) THEN
        z = 1 - xi*(0.5 - xi*(0.166666667 - 0.041666667*xi))
      ELSE
        z = (1 - Exp(-xi))/xi
      END IF
      r = 0.5*sint
      r2 = r*r
      z2 = z*z
      r2max = 1 - z2
      IF (( r2max .LT. r2 )) THEN
        r2 = r2max
        r = Sqrt(r2)
      END IF
      ut = r*cphi
      vt = r*sphi
      wt = z
      ustep = Sqrt(z2 + r2)*tustep
      sint02 = u0**2 + v0**2
      IF ((sint02 .GT. 1e-20)) THEN
        sint0 = sqrt(sint02)
        sint0i = 1/sint0
        cphi0 = sint0i*u0
        sphi0 = sint0i*v0
        u2p = w0*us + sint0*ws
        ws = w0*ws - sint0*us
        us = u2p*cphi0 - vs*sphi0
        vs = u2p*sphi0 + vs*cphi0
        u2p = w0*ut + sint0*wt
        wt = w0*wt - sint0*ut
        ut = u2p*cphi0 - vt*sphi0
        vt = u2p*sphi0 + vt*cphi0
      ELSE
        wt = w0*wt
        ws = w0*ws
      END IF
      xf = x0 + tustep*ut
      yf = y0 + tustep*vt
      zf = z0 + tustep*wt
      return
      end
      SUBROUTINE PAIR
      implicit none
      COMMON/QDEBUG/QDEBUG
      LOGICAL QDEBUG
      COMMON/BREMPR/ DL1(8,1),DL2(8,1),DL3(8,1),DL4(8,1),DL5(8,1),DL6(8,
     *1), ALPHI(2,1),BPAR(2,1),DELPOS(2,1), WA(1,50),PZ(1,50),ZELEM(1,50
     *),RHOZ(1,50), PWR2I(50), DELCM(1),ZBRANG(1),LZBRANG(1),NNE(1), IBR
     *DST,IPRDST,ibr_nist,pair_nrc,itriplet, ASYM(1,50,2)
      CHARACTER*4 ASYM
      real*8 DL1,DL2,DL3,DL4,DL5,DL6,   ALPHI,  BPAR,  DELPOS,  WA,  PZ,
     *  ZELEM,  RHOZ,  PWR2I,  DELCM,  ZBRANG,  LZBRANG
      integer*4 NNE,  IBRDST,  IPRDST,  ibr_nist,  itriplet,  pair_nrc
      common/nrc_pair/ nrcp_fdata(65,84,1), nrcp_wdata(65,84,1), nrcp_id
     *ata(65,84,1), nrcp_xdata(65), nrcp_emin, nrcp_emax, nrcp_dle, nrcp
     *_dlei
      real*8 nrcp_fdata,nrcp_wdata,nrcp_xdata, nrcp_emin, nrcp_emax, nrc
     *p_dle, nrcp_dlei
      integer*4 nrcp_idata
      COMMON/STACK/ E(50),X(50),Y(50),Z(50),U(50),V(50),W(50),DNEAR(50),
     *WT(50),IQ(50),IR(50),LATCH(50), LATCHI,NP,NPold
      DOUBLE PRECISION E
      real*8 X,Y,Z,  U,V,W,  DNEAR,  WT
      integer*4 IQ,  IR,  LATCH,  LATCHI, NP,  NPold
      COMMON/THRESH/RMT2,RMSQ, AP(1),AE(1),UP(1),UE(1),TE(1),THMOLL(1)
      real*8 RMT2,  RMSQ,  AP,  AE,  UP,  UE,  TE,  THMOLL
      COMMON/UPHIOT/THETA,SINTHE,COSTHE,SINPHI, COSPHI,PI,TWOPI,PI5D2
      real*8 THETA,  SINTHE,  COSTHE,  SINPHI,  COSPHI,  PI,TWOPI,PI5D2
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      COMMON/EPCONT/EDEP,EDEP_LOCAL,TSTEP,TUSTEP,USTEP,TVSTEP,VSTEP, RHO
     *F,EOLD,ENEW,EKE,ELKE,GLE,E_RANGE, x_final,y_final,z_final, u_final
     *,v_final,w_final, IDISC,IROLD,IRNEW,IAUSFL(35)
      DOUBLE PRECISION EDEP,  EDEP_LOCAL
      real*8 TSTEP,  TUSTEP,  USTEP,  VSTEP,  TVSTEP,  RHOF,  EOLD,  ENE
     *W,  EKE,  ELKE,  GLE,  E_RANGE, x_final,y_final,z_final,  u_final,
     *v_final,w_final
      integer*4 IDISC,  IROLD,  IRNEW,  IAUSFL
      common/triplet_data/ a_triplet(250,1), b_triplet(250,1), dl_triple
     *t, dli_triplet, bli_triplet, log_4rm
      real*8 a_triplet,b_triplet,dl_triplet, dli_triplet, bli_triplet, l
     *og_4rm
      common/egs_vr/ e_max_rr(1),  prob_RR,  nbr_split,  i_play_RR,
     * i_survived_RR,
     *     n_RR_warning,                                        i_do_rr(
     *1)
      real*8          e_max_rr,prob_RR
      integer*4       nbr_split,i_play_RR,i_survived_RR,n_RR_warning
      integer*2     i_do_rr
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      DOUBLE PRECISION PEIG,  PESE1,  PESE2
      real*8 EIG,  ESE2,  RNNO30,RNNO31,rnno32,rnno33,rnno34,  DELTA,  R
     *EJF,  rejmax,  aux1,aux2,  Amax,  Bmax,  del0,  br,
     *                               Eminus,Eplus,Eavail,rnno_RR
      integer*4
     *                     L,L1
      real*8 ESE,  PSE,  ZTARG,  TTEIG,  TTESE,  TTPSE,  ESEDEI, ESEDER,
     * XIMIN,  XIMID,  REJMIN, REJMID, REJTOP, YA,XITRY,GALPHA,GBETA,  X
     *ITST,  REJTST_on_REJTOP ,  REJTST, RTEST
      integer*4 ICHRG
      real*8 k,xx,abin,rbin,alias_sample1
      integer*4 ibin, iq1, iq2, iprdst_use
      logical do_nrc_pair
      integer*4 itrip
      real*8 ftrip
      NPold = NP
      IF (( i_play_RR .EQ. 1 )) THEN
        i_survived_RR = 0
        IF (( prob_RR .LE. 0 )) THEN
          IF (( n_RR_warning .LT. 50 )) THEN
            n_RR_warning = n_RR_warning + 1
            write(i_log,'(/a)') '***************** Warning: '
            write(i_log,'(a,g14.6)') 'Attempt to play Russian Roulette w
     *ith prob_RR<0! '
          END IF
        ELSE
          IF((rng_seed .GT. 128))call ranmar_get
          rnno_RR = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          IF (( rnno_RR .GT. prob_RR )) THEN
            i_survived_RR =2
            IF (( np .GT. 1 )) THEN
              np = np-1
            ELSE
              wt(np) = 0
              e(np) = 0
            END IF
            return
          ELSE
            wt(np) = wt(np)/prob_RR
          END IF
        END IF
      END IF
      IF (( np+1 .GT. 50 )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,'(//,3a,/,2(a,i9))') ' In subroutine ','PAIR', ' sta
     *ck size exceeded! ',' $MAXSTACK = ',50,' np = ',np+1
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      PEIG=E(NP)
      EIG=PEIG
      do_nrc_pair = .false.
      IF (( itriplet .GT. 0 .AND. eig .GT. 4*rm )) THEN
        itrip = dli_triplet*gle + bli_triplet
        ftrip = a_triplet(itrip,medium)*gle + b_triplet(itrip,medium)
        IF((rng_seed .GT. 128))call ranmar_get
        rnno34 = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        IF (( rnno34 .LT. ftrip )) THEN
          call sample_triplet
          return
        END IF
      END IF
      IF (( pair_nrc .EQ. 1 )) THEN
        k = eig/rm
        IF (( k .LT. nrcp_emax )) THEN
          do_nrc_pair = .true.
          IF (( k .LE. nrcp_emin )) THEN
            ibin = 1
          ELSE
            abin = 1 + log((k-2)/(nrcp_emin-2))*nrcp_dlei
            ibin = abin
            abin = abin - ibin
            IF((rng_seed .GT. 128))call ranmar_get
            rbin = rng_array(rng_seed)*twom24
            rng_seed = rng_seed + 1
            IF((rbin .LT. abin))ibin = ibin + 1
          END IF
          xx = alias_sample1(64,nrcp_xdata, nrcp_fdata(1,ibin,medium),nr
     *    cp_wdata(1,ibin,medium), nrcp_idata(1,ibin,medium))
          IF (( xx .GT. 0.5 )) THEN
            pese1 = prm*(1 + xx*(k-2))
            iq1 = 1
            pese2 = peig - pese1
            iq2 = -1
          ELSE
            pese2 = prm*(1 + xx*(k-2))
            iq2 = 1
            pese1 = peig - pese2
            iq1 = -1
          END IF
        END IF
      END IF
      IF (( .NOT.do_nrc_pair )) THEN
        IF ((EIG.LE.2.1)) THEN
          IF((rng_seed .GT. 128))call ranmar_get
          RNNO30 = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          IF((rng_seed .GT. 128))call ranmar_get
          rnno34 = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          PESE2 = PRM + 0.5*RNNO30*(PEIG-2*PRM)
          PESE1 = PEIG - PESE2
          IF (( rnno34 .LT. 0.5 )) THEN
            iq1 = -1
            iq2 = 1
          ELSE
            iq1 = 1
            iq2 = -1
          END IF
        ELSE
          IF ((EIG.LT.50.)) THEN
            L = 5
            L1 = L + 1
            delta = 4*delcm(medium)/eig
            IF (( delta .LT. 1 )) THEN
              Amax = dl1(l,medium)+delta*(dl2(l,medium)+delta*dl3(l,medi
     *        um))
              Bmax = dl1(l1,medium)+delta*(dl2(l1,medium)+delta*dl3(l1,m
     *        edium))
            ELSE
              aux2 = log(delta+dl6(l,medium))
              Amax = dl4(l,medium)+dl5(l,medium)*aux2
              Bmax = dl4(l1,medium)+dl5(l1,medium)*aux2
            END IF
            aux1 = 1 - rmt2/eig
            aux1 = aux1*aux1
            aux1 = aux1*Amax/3
            aux1 = aux1/(Bmax+aux1)
          ELSE
            L = 7
            Amax = dl1(l,medium)
            Bmax = dl1(l+1,medium)
            aux1 = bpar(2,medium)*(1-bpar(1,medium)*rm/eig)
          END IF
          del0 = eig*delcm(medium)
          Eavail = eig - rmt2
8121      CONTINUE
            IF((rng_seed .GT. 128))call ranmar_get
            RNNO30 = rng_array(rng_seed)*twom24
            rng_seed = rng_seed + 1
            IF((rng_seed .GT. 128))call ranmar_get
            RNNO31 = rng_array(rng_seed)*twom24
            rng_seed = rng_seed + 1
            IF((rng_seed .GT. 128))call ranmar_get
            RNNO34 = rng_array(rng_seed)*twom24
            rng_seed = rng_seed + 1
            IF (( rnno30 .GT. aux1 )) THEN
              br = 0.5*rnno31
              rejmax = Bmax
              l1 = l+1
            ELSE
              IF((rng_seed .GT. 128))call ranmar_get
              rnno32 = rng_array(rng_seed)*twom24
              rng_seed = rng_seed + 1
              IF((rng_seed .GT. 128))call ranmar_get
              rnno33 = rng_array(rng_seed)*twom24
              rng_seed = rng_seed + 1
              br = 0.5*(1-max(rnno31,rnno32,rnno33))
              rejmax = Amax
              l1 = l
            END IF
            Eminus = br*Eavail + rm
            Eplus = eig - Eminus
            delta = del0/(Eminus*Eplus)
            IF (( delta .LT. 1 )) THEN
              rejf = dl1(l1,medium)+delta*(dl2(l1,medium)+delta*dl3(l1,m
     *        edium))
            ELSE
              rejf = dl4(l1,medium)+dl5(l1,medium)*log(delta+dl6(l1,medi
     *        um))
            END IF
            IF((( rnno34*rejmax .LE. rejf )))GO TO8122
          GO TO 8121
8122      CONTINUE
          pese2 = Eminus
          pese1 = peig - pese2
          IF((rng_seed .GT. 128))call ranmar_get
          RNNO34 = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          IF (( rnno34 .LT. 0.5 )) THEN
            iq1 = -1
            iq2 = 1
          ELSE
            iq1 = 1
            iq2 = -1
          END IF
        END IF
      END IF
      ESE2=PESE2
      E(NP)=PESE1
      E(NP+1)=PESE2
      IF (( iprdst .GT. 0 )) THEN
        IF (( iprdst .EQ. 4 )) THEN
          IF((rng_seed .GT. 128))call ranmar_get
          rtest = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          gbeta = PESE1/(PESE1+10)
          IF (( rtest .LT. gbeta )) THEN
            iprdst_use = 1
          ELSE
            iprdst_use = 4
          END IF
        ELSE IF(( iprdst .EQ. 2 .AND. eig .LT. 4.14 )) THEN
          iprdst_use = 1
        ELSE
          iprdst_use = iprdst
        END IF
        DO 8131 ichrg=1,2
          IF ((ICHRG.EQ.1)) THEN
            ESE=PESE1
          ELSE
            ESE=ESE2
            IF (( iprdst .EQ. 4 )) THEN
              gbeta = ESE/(ESE+10)
              IF((rng_seed .GT. 128))call ranmar_get
              rtest = rng_array(rng_seed)*twom24
              rng_seed = rng_seed + 1
              IF (( rtest .LT. gbeta )) THEN
                iprdst_use = 1
              ELSE
                iprdst_use = 4
              END IF
            END IF
          END IF
          IF (( iprdst_use .EQ. 1 )) THEN
            PSE=SQRT(MAX(0.0,(ESE-RM)*(ESE+RM)))
            IF((rng_seed .GT. 128))call ranmar_get
            COSTHE = rng_array(rng_seed)*twom24
            rng_seed = rng_seed + 1
            COSTHE=1.0-2.0*COSTHE
            SINTHE=RM*SQRT((1.0-COSTHE)*(1.0+COSTHE))/(PSE*COSTHE+ESE)
            COSTHE=(ESE*COSTHE+PSE)/(PSE*COSTHE+ESE)
          ELSE IF(( iprdst_use .EQ. 2 )) THEN
            ZTARG=ZBRANG(MEDIUM)
            TTEIG=EIG/RM
            TTESE=ESE/RM
            TTPSE=SQRT((TTESE-1.0)*(TTESE+1.0))
            ESEDEI=TTESE/(TTEIG-TTESE)
            ESEDER=1.0/ESEDEI
            XIMIN=1.0/(1.0+(3.141593*TTESE)**2)
            REJMIN = 2.0+3.0*(ESEDEI+ESEDER) - 4.00*(ESEDEI+ESEDER+1.0-4
     *      .0*(XIMIN-0.5)**2)*( 1.0+0.25*LOG( ((1.0+ESEDER)*(1.0+ESEDEI
     *      )/(2.*TTEIG))**2+ZTARG*XIMIN**2 ) )
            YA=(2.0/TTEIG)**2
            XITRY=MAX(0.01,MAX(XIMIN,MIN(0.5,SQRT(YA/ZTARG))))
            GALPHA=1.0+0.25*LOG(YA+ZTARG*XITRY**2)
            GBETA=0.5*ZTARG*XITRY/(YA+ZTARG*XITRY**2)
            GALPHA=GALPHA-GBETA*(XITRY-0.5)
            XIMID=GALPHA/(3.0*GBETA)
            IF ((GALPHA.GE.0.0)) THEN
              XIMID=0.5-XIMID+SQRT(XIMID**2+0.25)
            ELSE
              XIMID=0.5-XIMID-SQRT(XIMID**2+0.25)
            END IF
            XIMID=MAX(0.01,MAX(XIMIN,MIN(0.5,XIMID)))
            REJMID = 2.0+3.0*(ESEDEI+ESEDER) - 4.00*(ESEDEI+ESEDER+1.0-4
     *      .0*(XIMID-0.5)**2)*( 1.0+0.25*LOG( ((1.0+ESEDER)*(1.0+ESEDEI
     *      )/(2.*TTEIG))**2+ZTARG*XIMID**2 ) )
            REJTOP=1.02*MAX(REJMIN,REJMID)
8141        CONTINUE
              IF((rng_seed .GT. 128))call ranmar_get
              XITST = rng_array(rng_seed)*twom24
              rng_seed = rng_seed + 1
              REJTST = 2.0+3.0*(ESEDEI+ESEDER) - 4.00*(ESEDEI+ESEDER+1.0
     *        -4.0*(XITST-0.5)**2)*( 1.0+0.25*LOG( ((1.0+ESEDER)*(1.0+ES
     *        EDEI)/(2.*TTEIG))**2+ZTARG*XITST**2 ) )
              IF((rng_seed .GT. 128))call ranmar_get
              RTEST = rng_array(rng_seed)*twom24
              rng_seed = rng_seed + 1
              THETA=SQRT(1.0/XITST-1.0)/TTESE
              REJTST_on_REJTOP = REJTST/REJTOP
              IF((((RTEST .LE. REJTST_on_REJTOP) .AND. (THETA .LT. PI) )
     *        ))GO TO8142
            GO TO 8141
8142        CONTINUE
            SINTHE=SIN(THETA)
            COSTHE=COS(THETA)
          ELSE IF(( iprdst_use .EQ. 3 )) THEN
            IF((rng_seed .GT. 128))call ranmar_get
            COSTHE = rng_array(rng_seed)*twom24
            rng_seed = rng_seed + 1
            COSTHE=1.0-2.0*COSTHE
            sinthe=(1-costhe)*(1+costhe)
            IF (( sinthe .GT. 0 )) THEN
              sinthe = sqrt(sinthe)
            ELSE
              sinthe = 0
            END IF
          ELSE
            IF((rng_seed .GT. 128))call ranmar_get
            costhe = rng_array(rng_seed)*twom24
            rng_seed = rng_seed + 1
            costhe=1-2*sqrt(costhe)
            sinthe=(1-costhe)*(1+costhe)
            IF (( sinthe .GT. 0 )) THEN
              sinthe=sqrt(sinthe)
            ELSE
              sinthe=0
            END IF
          END IF
          IF (( ichrg .EQ. 1 )) THEN
            CALL UPHI(2,1)
          ELSE
            sinthe=-sinthe
            NP=NP+1
            CALL UPHI(3,2)
          END IF
8131    CONTINUE
8132    CONTINUE
        iq(np) = iq2
        iq(np-1) = iq1
        return
      ELSE
        THETA=0
      END IF
      CALL UPHI(1,1)
      NP=NP+1
      SINTHE=-SINTHE
      CALL UPHI(3,2)
      IQ(NP)=iq2
      IQ(NP-1)=iq1
      RETURN
      END
      subroutine sample_triplet
      implicit none
      COMMON/EPCONT/EDEP,EDEP_LOCAL,TSTEP,TUSTEP,USTEP,TVSTEP,VSTEP, RHO
     *F,EOLD,ENEW,EKE,ELKE,GLE,E_RANGE, x_final,y_final,z_final, u_final
     *,v_final,w_final, IDISC,IROLD,IRNEW,IAUSFL(35)
      DOUBLE PRECISION EDEP,  EDEP_LOCAL
      real*8 TSTEP,  TUSTEP,  USTEP,  VSTEP,  TVSTEP,  RHOF,  EOLD,  ENE
     *W,  EKE,  ELKE,  GLE,  E_RANGE, x_final,y_final,z_final,  u_final,
     *v_final,w_final
      integer*4 IDISC,  IROLD,  IRNEW,  IAUSFL
      COMMON/STACK/ E(50),X(50),Y(50),Z(50),U(50),V(50),W(50),DNEAR(50),
     *WT(50),IQ(50),IR(50),LATCH(50), LATCHI,NP,NPold
      DOUBLE PRECISION E
      real*8 X,Y,Z,  U,V,W,  DNEAR,  WT
      integer*4 IQ,  IR,  LATCH,  LATCHI, NP,  NPold
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      COMMON/THRESH/RMT2,RMSQ, AP(1),AE(1),UP(1),UE(1),TE(1),THMOLL(1)
      real*8 RMT2,  RMSQ,  AP,  AE,  UP,  UE,  TE,  THMOLL
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      real*8 fmax_array(250), eta_p_array(250), eta_Ep_array(250), eta_c
     *ostp_array(250), eta_costm_array(250), ebin_array(250), wp_array(2
     *50), qmin_array(250)
      real*8 kmin, kmax, dlogki, alogkm, prmi, tiny_eta
      real*8 ai,rnno,k,qmin,qmax,aux,a1,a2,a3,D,px1,px2,pp_min,pp_max, E
     *p_min,Ep_max,k2p2,k2p2x,peig,b,aux1,aux12,D1,aux3,xmin,xmax, aux6,
     *aux7,uu,cphi,sphi,cphi_factor,aux5,phi,tmp
      real*8 Er,pr,pr2,eta_pr
      real*8 Ep,pp,pp2,wEp,cost_p,sint_p,eta_Ep,mup_min,wmup, eta_costp,
     *Epp,pp_sintp,pp_sntp2
      real*8 Em,pm,pm2,cost_m,sint_m,Emm,wmum,pm_sintm, eta_costm
      real*8 k2,k3,s2,s3,k2k3i,k22,k32,q2,aux4,S_1,S_2,sigma
      real*8 ppx, ppy, ppz, pmx, pmy, pmz, prx, pry, prz, a,c,sindel,cos
     *del,sinpsi
      integer*4 i
      logical use_it
      integer*4 iscore
      logical is_initialized
      data is_initialized/.false./
      save is_initialized,fmax_array,eta_p_array,eta_Ep_array,eta_costp_
     *array, eta_costm_array,ebin_array,wp_array,qmin_array, kmin,kmax,d
     *logki,alogkm,prmi,tiny_eta
      IF (( .NOT.is_initialized )) THEN
        is_initialized = .true.
        tiny_eta = 1e-6
        DO 8151 i=1,250
          fmax_array(i) = -1
8151    CONTINUE
8152    CONTINUE
        kmax = 0
        kmin = 4.1*prm
        DO 8161 i=1,nmed
          IF((up(i) .GT. kmax))kmax = UP(i)
8161    CONTINUE
8162    CONTINUE
        IF((kmax .LE. kmin))return
        dlogki = 250 - 1
        dlogki = dlogki/log(kmax/kmin)
        alogkm = 1 - dlogki*log(kmin)
        prmi = 1/prm
        DO 8171 i=1,250
          k = 4.1*exp((i-1.)/dlogki)
          ebin_array(i) = k
          qmin = 4*k/(k*(k-1)+(k+1)*sqrt(k*(k-4)))
          qmax = (k*(k-1) + (k+1)*sqrt(k*(k-4)))/(2*k+1)
          qmin_array(i) = qmin
          wp_array(i) = log(qmax/qmin)
8171    CONTINUE
8172    CONTINUE
      END IF
      peig = e(np)
      IF((peig .LE. 4*prm))return
      IF (( np+2 .GT. 50 )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,'(//,3a,/,2(a,i9))') ' In subroutine ','sample_tripl
     *et', ' stack size exceeded! ',' $MAXSTACK = ',50,' np = ',np+2
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      IF (( peig .LE. kmin )) THEN
        i = 1
      ELSE IF(( peig .GE. kmax )) THEN
        i = 250
      ELSE
        ai = alogkm + dlogki*gle
        i = ai
        ai = ai - i
        IF((rng_seed .GT. 128))call ranmar_get
        rnno = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        IF (( rnno .LT. ai )) THEN
          i = i+1
        END IF
      END IF
      k = ebin_array(i)
8180  CONTINUE
      IF((rng_seed .GT. 128))call ranmar_get
      eta_pr = rng_array(rng_seed)*twom24
      rng_seed = rng_seed + 1
      IF((eta_pr .LT. tiny_eta))eta_pr = tiny_eta
      pr = qmin_array(i)*exp(eta_pr*wp_array(i))
      pr2 = pr*pr
      Er = sqrt(1+pr2)
      aux = Er-pr-1
      a1=(k-pr)*(1-Er-k*aux)
      a2=1+k-Er
      a3=1/(aux*(pr+Er-2*k-1))
      D = a2*sqrt(aux*(2*k*Er+k*k*aux-pr*(Er+pr+1)/2))
      px1 = (a1 + D)*a3
      px2 = (a1 - D)*a3
      IF (( px1 .LT. px2 )) THEN
        pp_min = px1
        pp_max = px2
      ELSE
        pp_min = px2
        pp_max = px1
      END IF
      Ep_min = sqrt(1 + pp_min*pp_min)
      Ep_max = sqrt(1 + pp_max*pp_max)
      IF((rng_seed .GT. 128))call ranmar_get
      eta_Ep = rng_array(rng_seed)*twom24
      rng_seed = rng_seed + 1
      IF((eta_Ep .LT. tiny_eta))eta_Ep = tiny_eta
      wEp = Ep_max - Ep_min
      Ep = Ep_min + eta_Ep*wEp
      pp2 = Ep*Ep - 1
      pp = sqrt(pp2)
      k2p2 = k*k + pp2
      Em = k + 1 - Er - Ep
      pm2 = Em*Em-1
      pm = sqrt(pm2)
      mup_min = (k2p2 - (pr + pm)*(pr + pm))/(2*k*pp)
      IF((rng_seed .GT. 128))call ranmar_get
      eta_costp = rng_array(rng_seed)*twom24
      rng_seed = rng_seed + 1
      IF((eta_costp .LT. tiny_eta))eta_costp = tiny_eta
      Epp = Ep/pp
      wmup = log((Epp-1)/(Epp-mup_min))
      cost_p = Epp - (Epp - mup_min)*exp(wmup*eta_costp)
      wmup = wmup*(cost_p - Epp)
      sint_p = 1-cost_p*cost_p
      IF (( sint_p .GT. 1e-20 )) THEN
        sint_p = sqrt(sint_p)
      ELSE
        sint_p = 1e-10
      END IF
      k2p2x = k2p2 - 2*k*pp*cost_p
      b = pr2-k2p2x-pm2
      aux1 = k - pp*cost_p
      aux12 = aux1*aux1
      pp_sintp = pp*sint_p
      pp_sntp2 = pp_sintp*pp_sintp
      D1 = pm2*(aux12+pp_sntp2)-b*b/4
      IF (( D1 .LE. 0 )) THEN
        goto 8180
      END IF
      D = 2*pp_sintp*sqrt(D1)
      aux3 = 0.5/(aux12+pp_sntp2)
      xmin = (-b*aux1-D)*aux3
      xmax = (-b*aux1+D)*aux3
      IF((rng_seed .GT. 128))call ranmar_get
      eta_costm = rng_array(rng_seed)*twom24
      rng_seed = rng_seed + 1
      IF((eta_costm .LT. tiny_eta))eta_costm = tiny_eta
      aux6 = sqrt((Em-xmin)/(Em-xmax))
      aux7 = aux6*tan(1.570796326794897*eta_costm)
      uu = (aux7-1)/(aux7+1)
      cost_m = 0.5*(xmax + xmin + 2*uu*(xmax-xmin)/(1+uu*uu))
      wmum = sqrt((xmax-cost_m)*(cost_m-xmin))
      wmum = wmum*aux6*(Em-cost_m)/(Em-xmin)
      cost_m = cost_m/pm
      sint_m = sqrt(1-cost_m*cost_m)
      pm_sintm = pm*sint_m
      cphi = (b + 2*pm*cost_m*aux1)/(2*pp_sintp*pm_sintm)
      IF (( abs(cphi) .GE. 1 )) THEN
        goto 8180
      END IF
      sphi = sqrt(1-cphi*cphi)
      k3 = k*(pp*cost_p - Ep)
      k2 = k*(pm*cost_m - Em)
      k22 = k2*k2
      k32 = k3*k3
      k2k3i = 1/(k2*k3)
      s2 = pp*pm*(cost_p*cost_m + sint_p*sint_m*cphi) - Ep*Em
      s3 = k2 - Em + 1 - s2
      q2 = 2*(Er-1)
      S_1 = k32+k22+(q2-2)*s2-(1-q2/2)*(k32+k22)*k2k3i
      aux4 = k3*Ep-k2*Em
      S_2 = -q2*(Ep*Ep+Em*Em) + 2*s2 - (2*aux4*aux4 - k22 - k32)*k2k3i
      sigma = abs(pp*pm2*pm*k2k3i/(q2*q2*(Em*s3+Er))*(S_1*(1-q2/4)+S_2*(
     *1+q2/4)))
      cphi_factor = abs(2*Er*pm2-Em*(k2p2x-pr2-pm2))/(2*pp_sintp*pm_sint
     *m*pm2*sphi)
      sigma = sigma*cphi_factor*wEp*wmup*wmum*wp_array(i)*pr2/Er
      IF (( sigma .LT. 0 )) THEN
        write(i_log,'(/a)') '***************** Warning: '
        write(i_log,*) 'In triplet sigma < 0 ? ',sigma
      END IF
      use_it = .true.
      IF (( sigma .LT. fmax_array(i) )) THEN
        IF((rng_seed .GT. 128))call ranmar_get
        rnno = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        IF (( sigma .LT. fmax_array(i)*rnno )) THEN
          use_it = .false.
        END IF
      END IF
      IF (( use_it )) THEN
        fmax_array(i) = sigma
        eta_p_array(i) = eta_pr
        eta_Ep_array(i) = eta_Ep
        eta_costp_array(i) = eta_costp
        eta_costm_array(i) = eta_costm
      ELSE
        eta_pr = eta_p_array(i)
        eta_Ep = eta_Ep_array(i)
        eta_costp = eta_costp_array(i)
        eta_costm = eta_costm_array(i)
      END IF
      k = peig*prmi
      aux5 = k*(k-1)+(k+1)*sqrt(k*(k-4))
      qmin = 4*k/aux5
      qmax = aux5/(2*k+1)
      pr = qmin*exp(eta_pr*log(qmax/qmin))
      pr2 = pr*pr
      Er = sqrt(1+pr2)
      aux = Er-pr-1
      a1=(k-pr)*(1-Er-k*aux)
      a2=1+k-Er
      a3=1/(aux*(pr+Er-2*k-1))
      D = a2*sqrt(aux*(2*k*Er+k*k*aux-pr*(Er+pr+1)/2))
      px1 = (a1 + D)*a3
      px2 = (a1 - D)*a3
      IF (( px1 .LT. px2 )) THEN
        pp_min = px1
        pp_max = px2
      ELSE
        pp_min = px2
        pp_max = px1
      END IF
      Ep_min = sqrt(1 + pp_min*pp_min)
      Ep_max = sqrt(1 + pp_max*pp_max)
      wEp = Ep_max - Ep_min
      Ep = Ep_min + eta_Ep*wEp
      pp2 = Ep*Ep - 1
      pp = sqrt(pp2)
      k2p2 = k*k + pp2
      Em = k + 1 - Er - Ep
      pm2 = Em*Em-1
      pm = sqrt(pm2)
      mup_min = (k2p2 - (pr + pm)*(pr + pm))/(2*k*pp)
      Epp = Ep/pp
      wmup = log((Epp-1)/(Epp-mup_min))
      cost_p = Epp - (Epp - mup_min)*exp(wmup*eta_costp)
      sint_p = sqrt(1-cost_p*cost_p)
      k2p2x = k2p2 - 2*k*pp*cost_p
      b = pr2-k2p2x-pm2
      aux1 = k - pp*cost_p
      aux12 = aux1*aux1
      pp_sintp = pp*sint_p
      pp_sntp2 = pp_sintp*pp_sintp
      D1 = pm2*(aux12+pp_sntp2)-b*b/4
      IF (( D1 .LE. 0 )) THEN
        goto 8180
      END IF
      D = 2*pp_sintp*sqrt(D1)
      aux3 = 0.5/(aux12+pp_sntp2)
      xmin = (-b*aux1-D)*aux3
      xmax = (-b*aux1+D)*aux3
      aux6 = sqrt((Em-xmin)/(Em-xmax))
      aux7 = aux6*tan(1.570796326794897*eta_costm)
      uu = (aux7-1)/(aux7+1)
      cost_m = 0.5*(xmax + xmin + 2*uu*(xmax-xmin)/(1+uu*uu))/pm
      sint_m = sqrt(1-cost_m*cost_m)
      pm_sintm = pm*sint_m
      cphi = (b + 2*pm*cost_m*aux1)/(2*pp_sintp*pm_sintm)
      IF (( abs(cphi) .GE. 1 )) THEN
        goto 8180
      END IF
      sphi = sqrt(1-cphi*cphi)
      IF((rng_seed .GT. 128))call ranmar_get
      phi = rng_array(rng_seed)*twom24
      rng_seed = rng_seed + 1
      phi = phi*6.283185307179586
      ppx = pp*sint_p
      ppy = 0
      pmx = pm*sint_m*cphi
      pmy = pm*sint_m*sphi
      cphi = cos(phi)
      sphi = sin(phi)
      tmp = ppx*sphi
      ppx = ppx*cphi - ppy*sphi
      ppy = tmp + ppy*cphi
      tmp = pmx*sphi
      pmx = pmx*cphi - pmy*sphi
      pmy = tmp + pmy*cphi
      ppz = pp*cost_p
      pmz = pm*cost_m
      prx = -ppx-pmx
      pry = -ppy-pmy
      prz = k - ppz - pmz
      NPold = np
      X(np)=X(np)
      Y(np)=Y(np)
      Z(np)=Z(np)
      IR(np)=IR(np)
      WT(np)=WT(np)
      DNEAR(np)=DNEAR(np)
      LATCH(np)=LATCH(np)
      X(np+1)=X(np)
      Y(np+1)=Y(np)
      Z(np+1)=Z(np)
      IR(np+1)=IR(np)
      WT(np+1)=WT(np)
      DNEAR(np+1)=DNEAR(np)
      LATCH(np+1)=LATCH(np)
      X(np+2)=X(np+1)
      Y(np+2)=Y(np+1)
      Z(np+2)=Z(np+1)
      IR(np+2)=IR(np+1)
      WT(np+2)=WT(np+1)
      DNEAR(np+2)=DNEAR(np+1)
      LATCH(np+2)=LATCH(np+1)
      pp = 1/pp
      pm = 1/pm
      pr = 1/pr
      a = u(np)
      b = v(np)
      c = w(np)
      sinpsi = a*a + b*b
      IF (( sinpsi .GT. 1e-20 )) THEN
        sinpsi = sqrt(sinpsi)
        sindel = b/sinpsi
        cosdel = a/sinpsi
        IF (( Ep .GT. Em )) THEN
          u(np) = pp*(c*cosdel*ppx - sindel*ppy + a*ppz)
          v(np) = pp*(c*sindel*ppx + cosdel*ppy + b*ppz)
          w(np) = pp*(c*ppz - sinpsi*ppx)
          iq(np) = 1
          E(np) = Ep*prm
          u(np+1) = pm*(c*cosdel*pmx - sindel*pmy + a*pmz)
          v(np+1) = pm*(c*sindel*pmx + cosdel*pmy + b*pmz)
          w(np+1) = pm*(c*pmz - sinpsi*pmx)
          iq(np+1) = -1
          E(np+1) = Em*prm
        ELSE
          u(np+1) = pp*(c*cosdel*ppx - sindel*ppy + a*ppz)
          v(np+1) = pp*(c*sindel*ppx + cosdel*ppy + b*ppz)
          w(np+1) = pp*(c*ppz - sinpsi*ppx)
          iq(np+1) = 1
          E(np+1) = Ep*prm
          u(np) = pm*(c*cosdel*pmx - sindel*pmy + a*pmz)
          v(np) = pm*(c*sindel*pmx + cosdel*pmy + b*pmz)
          w(np) = pm*(c*pmz - sinpsi*pmx)
          iq(np) = -1
          E(np) = Em*prm
        END IF
        np = np + 2
        u(np) = pr*(c*cosdel*prx - sindel*pry + a*prz)
        v(np) = pr*(c*sindel*prx + cosdel*pry + b*prz)
        w(np) = pr*(c*prz - sinpsi*prx)
        iq(np) = -1
        E(np) = Er*prm
      ELSE
        IF (( Ep .GT. Em )) THEN
          u(np) = pp*ppx
          v(np) = pp*ppy
          w(np) = c*pp*ppz
          iq(np) = 1
          E(np) = Ep*prm
          u(np+1) = pm*pmx
          v(np+1) = pm*pmy
          w(np+1) = c*pm*pmz
          iq(np+1) = -1
          E(np+1) = Em*prm
        ELSE
          u(np+1) = pp*ppx
          v(np+1) = pp*ppy
          w(np+1) = c*pp*ppz
          iq(np+1) = 1
          E(np+1) = Ep*prm
          u(np) = pm*pmx
          v(np) = pm*pmy
          w(np) = c*pm*pmz
          iq(np) = -1
          E(np) = Em*prm
        END IF
        np = np + 2
        u(np) = pr*prx
        v(np) = pr*pry
        w(np) = c*pr*prz
        iq(np) = -1
        E(np) = Er*prm
      END IF
      return
      end
      SUBROUTINE PHOTO
      implicit none
      COMMON/BOUNDS/ECUT(1),PCUT(1),VACDST
      real*8 ECUT,  PCUT,  VACDST
      COMMON/BREMPR/ DL1(8,1),DL2(8,1),DL3(8,1),DL4(8,1),DL5(8,1),DL6(8,
     *1), ALPHI(2,1),BPAR(2,1),DELPOS(2,1), WA(1,50),PZ(1,50),ZELEM(1,50
     *),RHOZ(1,50), PWR2I(50), DELCM(1),ZBRANG(1),LZBRANG(1),NNE(1), IBR
     *DST,IPRDST,ibr_nist,pair_nrc,itriplet, ASYM(1,50,2)
      CHARACTER*4 ASYM
      real*8 DL1,DL2,DL3,DL4,DL5,DL6,   ALPHI,  BPAR,  DELPOS,  WA,  PZ,
     *  ZELEM,  RHOZ,  PWR2I,  DELCM,  ZBRANG,  LZBRANG
      integer*4 NNE,  IBRDST,  IPRDST,  ibr_nist,  itriplet,  pair_nrc
      COMMON/QDEBUG/QDEBUG
      LOGICAL QDEBUG
      COMMON/EDGE/binding_energies(30,100), interaction_prob(6,100), rel
     *axation_prob(39,100), edge_energies(16,100), edge_number(100), edg
     *e_a(16,100), edge_b(16,100), edge_c(16,100), edge_d(16,100), IEDGF
     *L(1),IPHTER(1)
      real*8 binding_energies,  interaction_prob,    relaxation_prob,  e
     *dge_energies,  edge_a,edge_b,edge_c,edge_d
      integer*2 IEDGFL,  IPHTER
      integer*4 edge_number
      COMMON/EPCONT/EDEP,EDEP_LOCAL,TSTEP,TUSTEP,USTEP,TVSTEP,VSTEP, RHO
     *F,EOLD,ENEW,EKE,ELKE,GLE,E_RANGE, x_final,y_final,z_final, u_final
     *,v_final,w_final, IDISC,IROLD,IRNEW,IAUSFL(35)
      DOUBLE PRECISION EDEP,  EDEP_LOCAL
      real*8 TSTEP,  TUSTEP,  USTEP,  VSTEP,  TVSTEP,  RHOF,  EOLD,  ENE
     *W,  EKE,  ELKE,  GLE,  E_RANGE, x_final,y_final,z_final,  u_final,
     *v_final,w_final
      integer*4 IDISC,  IROLD,  IRNEW,  IAUSFL
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      COMMON/PHOTIN/ EBINDA(1), GE0(1),GE1(1), GMFP0(2000,1),GMFP1(2000,
     *1),GBR10(2000,1),GBR11(2000,1),GBR20(2000,1),GBR21(2000,1), RCO0(1
     *),RCO1(1), RSCT0(100,1),RSCT1(100,1), COHE0(2000,1),COHE1(2000,1),
     *  PHOTONUC0(2000,1),PHOTONUC1(2000,1), DPMFP, MPGEM(1,1), NGR(1)
      real*8 EBINDA,  GE0,GE1,  GMFP0,GMFP1,  GBR10,GBR11,  GBR20,GBR21,
     *  RCO0,RCO1,  RSCT0,RSCT1,  COHE0,COHE1,   PHOTONUC0,PHOTONUC1,  D
     *PMFP
      integer*4
     *                  MPGEM,
     *                          NGR
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      COMMON/STACK/ E(50),X(50),Y(50),Z(50),U(50),V(50),W(50),DNEAR(50),
     *WT(50),IQ(50),IR(50),LATCH(50), LATCHI,NP,NPold
      DOUBLE PRECISION E
      real*8 X,Y,Z,  U,V,W,  DNEAR,  WT
      integer*4 IQ,  IR,  LATCH,  LATCHI, NP,  NPold
      COMMON/UPHIOT/THETA,SINTHE,COSTHE,SINPHI, COSPHI,PI,TWOPI,PI5D2
      real*8 THETA,  SINTHE,  COSTHE,  SINPHI,  COSPHI,  PI,TWOPI,PI5D2
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      common/x_options/eadl_relax,  mcdf_pe_xsections
      logical eadl_relax, mcdf_pe_xsections
      common/egs_vr/ e_max_rr(1),  prob_RR,  nbr_split,  i_play_RR,
     * i_survived_RR,
     *     n_RR_warning,                                        i_do_rr(
     *1)
      real*8          e_max_rr,prob_RR
      integer*4       nbr_split,i_play_RR,i_survived_RR,n_RR_warning
      integer*2     i_do_rr
      common/relax_data/ relax_first(3000),  relax_ntran(3000),  relax_s
     *tate(10000),  relax_prob(10000),  relax_atbin(10000),  relax_ntot
      real*8 relax_prob
      integer*4 relax_first, relax_ntran, relax_state, relax_atbin, rela
     *x_ntot
      real*8 EELEC,  BETA,  GAMMA,  ALPHA,  RATIO,  RNPHT,  FKAPPA, XI,
     * SINTH2, RNPHT2
      DOUBLE PRECISION PEIG
      real*8 BR,  sigma,  aux,aux1,  probs(50),  sigtot,  e_vac,  rnno_R
     *R
      integer*4 IARG,  iZ,   irl,  ints(50),  j,ip,  n_warning,  k
      logical do_relax
      save n_warning
      data n_warning/0/
      IF (( mcdf_pe_xsections )) THEN
        call egs_shellwise_photo()
        return
      END IF
      NPold = NP
      PEIG=E(NP)
      irl = ir(np)
      IF (( peig .LT. edge_energies(2,1) )) THEN
        IF (( n_warning .LT. 100 )) THEN
          n_warning = n_warning + 1
          write(i_log,*) ' Subroutine PHOTO called with E = ',peig, ' wh
     *ich is below the current min. energy of 1 keV! '
          write(i_log,*) ' Converting now this photon to an electron, '
          write(i_log,*) ' but you should check your code! '
        END IF
        iq(np) = -1
        e(np) = peig + prm
        return
      END IF
      iZ = iedgfl(irl)
      do_relax = .false.
      edep = pzero
      IF (( iedgfl(irl) .NE. 0 )) THEN
        IF (( nne(medium) .EQ. 1 )) THEN
          iZ = int( zelem(medium,1) + 0.5 )
          DO 8191 j=1,edge_number(iZ)
            IF((peig .GE. edge_energies(j,iZ)))GO TO8192
8191      CONTINUE
8192      CONTINUE
        ELSE
          aux = peig*peig
          aux1 = aux*peig
          aux = aux*Sqrt(peig)
          sigtot = 0
          DO 8201 k=1,nne(medium)
            iZ = int( zelem(medium,k) + 0.5 )
            IF (( iZ .LT. 1 .OR. iZ .GT. 100 )) THEN
              write(i_log,*) ' Error in PHOTO: '
              write(i_log,'(/a)') '***************** Error: '
              write(i_log,*) '   Atomic number of element ',k, ' in medi
     *um ',medium,' is not between 1 and ',100
              write(i_log,'(/a)') '***************** Quiting now.'
              call exit(1)
            END IF
            IF (( peig .GT. edge_energies(1,iZ) )) THEN
              j = 1
              sigma = (edge_a(1,iZ) + edge_b(1,iZ)/peig + edge_c(1,iZ)/a
     *        ux + edge_d(1,iZ)/aux1)/peig
            ELSE
              DO 8211 j=2,edge_number(iZ)
                IF((peig .GE. edge_energies(j,iZ)))GO TO8212
8211          CONTINUE
8212          CONTINUE
              sigma = edge_a(j,iZ) + gle*(edge_b(j,iZ) + gle*(edge_c(j,i
     *        Z) + gle*edge_d(j,iZ) ))
              sigma = Exp(sigma)
            END IF
            sigma = sigma * pz(medium,k)
            sigtot = sigtot + sigma
            probs(k) = sigma
            ints(k) = j
8201      CONTINUE
8202      CONTINUE
          IF((rng_seed .GT. 128))call ranmar_get
          br = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          br = br*sigtot
          DO 8221 k=1,nne(medium)
            br = br - probs(k)
            IF((br .LE. 0))GO TO8222
8221      CONTINUE
8222      CONTINUE
          iZ = int( zelem(medium,k) + 0.5 )
          j = ints(k)
        END IF
        IF (( peig .LE. binding_energies(6,iZ) )) THEN
          iq(np) = -1
          e(np) = peig + prm
        ELSE
          IF((rng_seed .GT. 128))call ranmar_get
          br = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          DO 8231 k=1,5
            IF (( peig .GT. binding_energies(k,iZ) )) THEN
              IF((br .LT. interaction_prob(k,iZ)))GO TO8232
              br = (br - interaction_prob(k,iZ))/(1-interaction_prob(k,i
     *        Z))
            END IF
8231      CONTINUE
8232      CONTINUE
          IF ((eadl_relax .AND. k .GT. 4)) THEN
            iq(np) = -1
            e(np) = peig + prm
          ELSE
            e_vac = binding_energies(k,iZ)
            e(np) = peig - e_vac + prm
            do_relax = .true.
            iq(np) = -1
          END IF
        END IF
      ELSE
        e(np) = peig + prm
        iq(np) = -1
      END IF
      IF (( iq(np) .EQ. -1 )) THEN
        IF ((IPHTER(IR(NP)).EQ.1)) THEN
          EELEC=E(NP)
          IF ((EELEC.GT.ECUT(IR(NP)))) THEN
            BETA=SQRT((EELEC-RM)*(EELEC+RM))/EELEC
            GAMMA=EELEC/RM
            ALPHA=0.5*GAMMA-0.5+1./GAMMA
            RATIO=BETA/ALPHA
8241        CONTINUE
              IF((rng_seed .GT. 128))call ranmar_get
              RNPHT = rng_array(rng_seed)*twom24
              rng_seed = rng_seed + 1
              RNPHT=2.*RNPHT-1.
              IF ((RATIO.LE.0.2)) THEN
                FKAPPA=RNPHT+0.5*RATIO*(1.-RNPHT)*(1.+RNPHT)
                IF (( gamma .LT. 100 )) THEN
                  COSTHE=(BETA+FKAPPA)/(1.+BETA*FKAPPA)
                ELSE
                  IF (( fkappa .GT. 0 )) THEN
                    costhe = 1 - (1-fkappa)*(gamma-3)/(2*(1+fkappa)*(gam
     *              ma-1)**3)
                  ELSE
                    COSTHE=(BETA+FKAPPA)/(1.+BETA*FKAPPA)
                  END IF
                END IF
                xi = (1+beta*fkappa)*gamma*gamma
              ELSE
                XI=GAMMA*GAMMA*(1.+ALPHA*(SQRT(1.+RATIO*(2.*RNPHT+RATIO)
     *          )-1.))
                COSTHE=(1.-1./XI)/BETA
              END IF
              SINTH2=MAX(0.,(1.-COSTHE)*(1.+COSTHE))
              IF((rng_seed .GT. 128))call ranmar_get
              RNPHT2 = rng_array(rng_seed)*twom24
              rng_seed = rng_seed + 1
              IF(RNPHT2.LE.0.5*(1.+GAMMA)*SINTH2*XI/GAMMA)GO TO8242
            GO TO 8241
8242        CONTINUE
            SINTHE=SQRT(SINTH2)
            CALL UPHI(2,1)
          END IF
        END IF
      END IF
      IF (( do_relax )) THEN
        call relax(e_vac,k,iZ)
      END IF
      IF (( EDEP .GT. 0 )) THEN
        IARG=4
        IF ((IAUSFL(IARG+1).NE.0)) THEN
          CALL AUSGAB(IARG)
        END IF
      END IF
      i_survived_RR = 0
      IF (( i_play_RR .EQ. 1 )) THEN
        IF (( prob_RR .LE. 0 )) THEN
          IF (( n_RR_warning .LT. 50 )) THEN
            n_RR_warning = n_RR_warning + 1
            WRITE(6,8250)prob_RR
8250        FORMAT('**** Warning, attempt to play Roussian Roulette with
     * prob_RR<=0! ',g14.6)
          END IF
        ELSE
          ip = NPold
8261      CONTINUE
            IF (( iq(ip) .NE. 0 )) THEN
              IF((rng_seed .GT. 128))call ranmar_get
              rnno_RR = rng_array(rng_seed)*twom24
              rng_seed = rng_seed + 1
              IF (( rnno_RR .LT. prob_RR )) THEN
                wt(ip) = wt(ip)/prob_RR
                ip = ip + 1
              ELSE
                i_survived_RR = i_survived_RR + 1
                IF ((ip .LT. np)) THEN
                  e(ip) = e(np)
                  iq(ip) = iq(np)
                  wt(ip) = wt(np)
                  u(ip) = u(np)
                  v(ip) = v(np)
                  w(ip) = w(np)
                END IF
                np = np-1
              END IF
            ELSE
              ip = ip+1
            END IF
            IF(((ip .GT. np)))GO TO8262
          GO TO 8261
8262      CONTINUE
          IF (( np .EQ. 0 )) THEN
            np = 1
            e(np) = 0
            iq(np) = 0
            wt(np) = 0
          END IF
        END IF
      END IF
      return
      end
      subroutine egs_shellwise_photo
      implicit none
      COMMON/BOUNDS/ECUT(1),PCUT(1),VACDST
      real*8 ECUT,  PCUT,  VACDST
      COMMON/BREMPR/ DL1(8,1),DL2(8,1),DL3(8,1),DL4(8,1),DL5(8,1),DL6(8,
     *1), ALPHI(2,1),BPAR(2,1),DELPOS(2,1), WA(1,50),PZ(1,50),ZELEM(1,50
     *),RHOZ(1,50), PWR2I(50), DELCM(1),ZBRANG(1),LZBRANG(1),NNE(1), IBR
     *DST,IPRDST,ibr_nist,pair_nrc,itriplet, ASYM(1,50,2)
      CHARACTER*4 ASYM
      real*8 DL1,DL2,DL3,DL4,DL5,DL6,   ALPHI,  BPAR,  DELPOS,  WA,  PZ,
     *  ZELEM,  RHOZ,  PWR2I,  DELCM,  ZBRANG,  LZBRANG
      integer*4 NNE,  IBRDST,  IPRDST,  ibr_nist,  itriplet,  pair_nrc
      COMMON/QDEBUG/QDEBUG
      LOGICAL QDEBUG
      COMMON/EDGE/binding_energies(30,100), interaction_prob(6,100), rel
     *axation_prob(39,100), edge_energies(16,100), edge_number(100), edg
     *e_a(16,100), edge_b(16,100), edge_c(16,100), edge_d(16,100), IEDGF
     *L(1),IPHTER(1)
      real*8 binding_energies,  interaction_prob,    relaxation_prob,  e
     *dge_energies,  edge_a,edge_b,edge_c,edge_d
      integer*2 IEDGFL,  IPHTER
      integer*4 edge_number
      COMMON/EPCONT/EDEP,EDEP_LOCAL,TSTEP,TUSTEP,USTEP,TVSTEP,VSTEP, RHO
     *F,EOLD,ENEW,EKE,ELKE,GLE,E_RANGE, x_final,y_final,z_final, u_final
     *,v_final,w_final, IDISC,IROLD,IRNEW,IAUSFL(35)
      DOUBLE PRECISION EDEP,  EDEP_LOCAL
      real*8 TSTEP,  TUSTEP,  USTEP,  VSTEP,  TVSTEP,  RHOF,  EOLD,  ENE
     *W,  EKE,  ELKE,  GLE,  E_RANGE, x_final,y_final,z_final,  u_final,
     *v_final,w_final
      integer*4 IDISC,  IROLD,  IRNEW,  IAUSFL
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      COMMON/PHOTIN/ EBINDA(1), GE0(1),GE1(1), GMFP0(2000,1),GMFP1(2000,
     *1),GBR10(2000,1),GBR11(2000,1),GBR20(2000,1),GBR21(2000,1), RCO0(1
     *),RCO1(1), RSCT0(100,1),RSCT1(100,1), COHE0(2000,1),COHE1(2000,1),
     *  PHOTONUC0(2000,1),PHOTONUC1(2000,1), DPMFP, MPGEM(1,1), NGR(1)
      real*8 EBINDA,  GE0,GE1,  GMFP0,GMFP1,  GBR10,GBR11,  GBR20,GBR21,
     *  RCO0,RCO1,  RSCT0,RSCT1,  COHE0,COHE1,   PHOTONUC0,PHOTONUC1,  D
     *PMFP
      integer*4
     *                  MPGEM,
     *                          NGR
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      COMMON/STACK/ E(50),X(50),Y(50),Z(50),U(50),V(50),W(50),DNEAR(50),
     *WT(50),IQ(50),IR(50),LATCH(50), LATCHI,NP,NPold
      DOUBLE PRECISION E
      real*8 X,Y,Z,  U,V,W,  DNEAR,  WT
      integer*4 IQ,  IR,  LATCH,  LATCHI, NP,  NPold
      COMMON/UPHIOT/THETA,SINTHE,COSTHE,SINPHI, COSPHI,PI,TWOPI,PI5D2
      real*8 THETA,  SINTHE,  COSTHE,  SINPHI,  COSPHI,  PI,TWOPI,PI5D2
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      common/x_options/eadl_relax,  mcdf_pe_xsections
      logical eadl_relax, mcdf_pe_xsections
      common/egs_vr/ e_max_rr(1),  prob_RR,  nbr_split,  i_play_RR,
     * i_survived_RR,
     *     n_RR_warning,                                        i_do_rr(
     *1)
      real*8          e_max_rr,prob_RR
      integer*4       nbr_split,i_play_RR,i_survived_RR,n_RR_warning
      integer*2     i_do_rr
      common/relax_data/ relax_first(3000),  relax_ntran(3000),  relax_s
     *tate(10000),  relax_prob(10000),  relax_atbin(10000),  relax_ntot
      real*8 relax_prob
      integer*4 relax_first, relax_ntran, relax_state, relax_atbin, rela
     *x_ntot
      common/pe_shell_data/ pe_xsection(500,100,0:16),  pe_elem_prob(500
     *,100,1),   pe_energy(500,100),  pe_zsorted(100,1), pe_be(100,16),
     * pe_nshell(100),  pe_zpos(100),  pe_nge(100),  pe_ne
      real*8 pe_be, pe_energy, pe_xsection, pe_elem_prob
      integer*4 pe_zsorted, pe_nshell, pe_zpos, pe_nge, pe_ne
      real*8 EELEC,  BETA,  GAMMA,  ALPHA,  RATIO,  RNPHT,  FKAPPA, XI,
     * SINTH2, RNPHT2
      DOUBLE PRECISION PEIG
      real*8 BR,  sigma,  aux,aux1,  probs(50),  sigtot,  e_vac,  rnno_R
     *R
      integer*4 IARG,  iZ,   irl,  ints(50),  j,ip,  n_warning,  k
      logical do_relax
      save n_warning
      real*8 slope, logE, int_prob
      integer*4 zpos, ibsearch
      data n_warning/0/
      NPold = NP
      PEIG=E(NP)
      irl = ir(np)
      do_relax = .false.
      IF (( peig .LT. 0.001 )) THEN
        IF (( n_warning .LT. 100 )) THEN
          n_warning = n_warning + 1
          write(i_log,*) ' Subroutine egs_shellwise_photo called with E
     *= ', peig,' which is below the current min. energy of ', 0.001,' k
     *eV! '
          write(i_log,*) ' Converting photon to a photo-electron, '
          write(i_log,*) ' but you should check your source and/or appli
     *cation! '
        END IF
        iq(np) = -1
        e(np) = peig + prm
        return
      END IF
      edep = pzero
      IF (( iedgfl(irl) .NE. 0 )) THEN
        j = -1
        IF (( nne(medium) .EQ. 1 )) THEN
          iZ = int( zelem(medium,1) + 0.5 )
          zpos = pe_zpos(iZ)
          IF (( pe_nshell(zpos) .GT. 0)) THEN
            logE = log(peig)
            j = ibsearch(logE,pe_nge(zpos),pe_energy(1,zpos))
          END IF
        ELSE
          IF((rng_seed .GT. 128))call ranmar_get
          br = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          logE = log(peig)
          DO 8271 k=nne(medium),1,-1
            iZ = int( zelem(medium,k) + 0.5 )
            zpos = pe_zpos(iZ)
            IF (( iZ .LT. 1 .OR. iZ .GT. 100 )) THEN
              write(i_log,*) ' Error in egs_shellwise_photo: '
              write(i_log,'(/a)') '***************** Error: '
              write(i_log,*) '   Atomic number of element ',k, ' in medi
     *um ',medium,' is not between 1 and ',100
              write(i_log,'(/a)') '***************** Quiting now.'
              call exit(1)
            END IF
            j = ibsearch(logE,pe_nge(zpos),pe_energy(1,zpos))
            slope = pe_elem_prob(j+1,k,medium) - pe_elem_prob(j,k,medium
     *      )
            slope = slope/(pe_energy(j+1,zpos)-pe_energy(j,zpos))
            int_prob = pe_elem_prob(j,k,medium)+slope*(logE-pe_energy(j,
     *      zpos))
            br = br - exp(int_prob)
            IF((br .LE. 0))GO TO8272
8271      CONTINUE
8272      CONTINUE
        END IF
        IF (( peig .LT. pe_be(zpos,pe_nshell(zpos)) .OR. pe_nshell(zpos)
     *   .EQ. 0 )) THEN
          iq(np) = -1
          e(np) = peig + prm
        ELSE
          IF((rng_seed .GT. 128))call ranmar_get
          br = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          sigtot = 0
          DO 8281 k=1,pe_nshell(zpos)
            IF (( peig .GT. pe_be(zpos,k) )) THEN
              slope = pe_xsection(j+1,zpos,k) - pe_xsection(j,zpos,k)
              slope = slope/(pe_energy(j+1,zpos)-pe_energy(j,zpos))
              int_prob=pe_xsection(j,zpos,k)+slope*(logE-pe_energy(j,zpo
     *        s))
              br = br - exp(int_prob)
              sigtot = sigtot + exp(int_prob)
              IF((br .LE. 0))GO TO8282
            END IF
8281      CONTINUE
8282      CONTINUE
          IF ((k .GT. pe_nshell(zpos))) THEN
            iq(np) = -1
            e(np) = peig + prm
          ELSE
            e_vac = pe_be(zpos,k)
            e(np) = peig - e_vac + prm
            do_relax = .true.
            iq(np) = -1
          END IF
        END IF
      ELSE
        e(np) = peig + prm
        iq(np) = -1
      END IF
      IF (( iq(np) .EQ. -1 )) THEN
        IF ((IPHTER(IR(NP)).EQ.1)) THEN
          EELEC=E(NP)
          IF ((EELEC.GT.ECUT(IR(NP)))) THEN
            BETA=SQRT((EELEC-RM)*(EELEC+RM))/EELEC
            GAMMA=EELEC/RM
            ALPHA=0.5*GAMMA-0.5+1./GAMMA
            RATIO=BETA/ALPHA
8291        CONTINUE
              IF((rng_seed .GT. 128))call ranmar_get
              RNPHT = rng_array(rng_seed)*twom24
              rng_seed = rng_seed + 1
              RNPHT=2.*RNPHT-1.
              IF ((RATIO.LE.0.2)) THEN
                FKAPPA=RNPHT+0.5*RATIO*(1.-RNPHT)*(1.+RNPHT)
                IF (( gamma .LT. 100 )) THEN
                  COSTHE=(BETA+FKAPPA)/(1.+BETA*FKAPPA)
                ELSE
                  IF (( fkappa .GT. 0 )) THEN
                    costhe = 1 - (1-fkappa)*(gamma-3)/(2*(1+fkappa)*(gam
     *              ma-1)**3)
                  ELSE
                    COSTHE=(BETA+FKAPPA)/(1.+BETA*FKAPPA)
                  END IF
                END IF
                xi = (1+beta*fkappa)*gamma*gamma
              ELSE
                XI=GAMMA*GAMMA*(1.+ALPHA*(SQRT(1.+RATIO*(2.*RNPHT+RATIO)
     *          )-1.))
                COSTHE=(1.-1./XI)/BETA
              END IF
              SINTH2=MAX(0.,(1.-COSTHE)*(1.+COSTHE))
              IF((rng_seed .GT. 128))call ranmar_get
              RNPHT2 = rng_array(rng_seed)*twom24
              rng_seed = rng_seed + 1
              IF(RNPHT2.LE.0.5*(1.+GAMMA)*SINTH2*XI/GAMMA)GO TO8292
            GO TO 8291
8292        CONTINUE
            SINTHE=SQRT(SINTH2)
            CALL UPHI(2,1)
          END IF
        END IF
      END IF
      IF (( do_relax )) THEN
        call egs_eadl_relax(iZ,k)
      END IF
      IF (( EDEP .GT. 0 )) THEN
        IARG=4
        IF ((IAUSFL(IARG+1).NE.0)) THEN
          CALL AUSGAB(IARG)
        END IF
      END IF
      i_survived_RR = 0
      IF (( i_play_RR .EQ. 1 )) THEN
        IF (( prob_RR .LE. 0 )) THEN
          IF (( n_RR_warning .LT. 50 )) THEN
            n_RR_warning = n_RR_warning + 1
            WRITE(6,8300)prob_RR
8300        FORMAT('**** Warning, attempt to play Roussian Roulette with
     * prob_RR<=0! ',g14.6)
          END IF
        ELSE
          ip = NPold
8311      CONTINUE
            IF (( iq(ip) .NE. 0 )) THEN
              IF((rng_seed .GT. 128))call ranmar_get
              rnno_RR = rng_array(rng_seed)*twom24
              rng_seed = rng_seed + 1
              IF (( rnno_RR .LT. prob_RR )) THEN
                wt(ip) = wt(ip)/prob_RR
                ip = ip + 1
              ELSE
                i_survived_RR = i_survived_RR + 1
                IF ((ip .LT. np)) THEN
                  e(ip) = e(np)
                  iq(ip) = iq(np)
                  wt(ip) = wt(np)
                  u(ip) = u(np)
                  v(ip) = v(np)
                  w(ip) = w(np)
                END IF
                np = np-1
              END IF
            ELSE
              ip = ip+1
            END IF
            IF(((ip .GT. np)))GO TO8312
          GO TO 8311
8312      CONTINUE
          IF (( np .EQ. 0 )) THEN
            np = 1
            e(np) = 0
            iq(np) = 0
            wt(np) = 0
          END IF
        END IF
      END IF
      return
      end
      subroutine egs_read_shellwise_pe
      implicit none
      COMMON/BREMPR/ DL1(8,1),DL2(8,1),DL3(8,1),DL4(8,1),DL5(8,1),DL6(8,
     *1), ALPHI(2,1),BPAR(2,1),DELPOS(2,1), WA(1,50),PZ(1,50),ZELEM(1,50
     *),RHOZ(1,50), PWR2I(50), DELCM(1),ZBRANG(1),LZBRANG(1),NNE(1), IBR
     *DST,IPRDST,ibr_nist,pair_nrc,itriplet, ASYM(1,50,2)
      CHARACTER*4 ASYM
      real*8 DL1,DL2,DL3,DL4,DL5,DL6,   ALPHI,  BPAR,  DELPOS,  WA,  PZ,
     *  ZELEM,  RHOZ,  PWR2I,  DELCM,  ZBRANG,  LZBRANG
      integer*4 NNE,  IBRDST,  IPRDST,  ibr_nist,  itriplet,  pair_nrc
      COMMON/EDGE/binding_energies(30,100), interaction_prob(6,100), rel
     *axation_prob(39,100), edge_energies(16,100), edge_number(100), edg
     *e_a(16,100), edge_b(16,100), edge_c(16,100), edge_d(16,100), IEDGF
     *L(1),IPHTER(1)
      real*8 binding_energies,  interaction_prob,    relaxation_prob,  e
     *dge_energies,  edge_a,edge_b,edge_c,edge_d
      integer*2 IEDGFL,  IPHTER
      integer*4 edge_number
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      COMMON/PHOTIN/ EBINDA(1), GE0(1),GE1(1), GMFP0(2000,1),GMFP1(2000,
     *1),GBR10(2000,1),GBR11(2000,1),GBR20(2000,1),GBR21(2000,1), RCO0(1
     *),RCO1(1), RSCT0(100,1),RSCT1(100,1), COHE0(2000,1),COHE1(2000,1),
     *  PHOTONUC0(2000,1),PHOTONUC1(2000,1), DPMFP, MPGEM(1,1), NGR(1)
      real*8 EBINDA,  GE0,GE1,  GMFP0,GMFP1,  GBR10,GBR11,  GBR20,GBR21,
     *  RCO0,RCO1,  RSCT0,RSCT1,  COHE0,COHE1,   PHOTONUC0,PHOTONUC1,  D
     *PMFP
      integer*4
     *                  MPGEM,
     *                          NGR
      COMMON/THRESH/RMT2,RMSQ, AP(1),AE(1),UP(1),UE(1),TE(1),THMOLL(1)
      real*8 RMT2,  RMSQ,  AP,  AE,  UP,  UE,  TE,  THMOLL
      common/x_options/eadl_relax,  mcdf_pe_xsections
      logical eadl_relax, mcdf_pe_xsections
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      common/pe_shell_data/ pe_xsection(500,100,0:16),  pe_elem_prob(500
     *,100,1),   pe_energy(500,100),  pe_zsorted(100,1), pe_be(100,16),
     * pe_nshell(100),  pe_zpos(100),  pe_nge(100),  pe_ne
      real*8 pe_be, pe_energy, pe_xsection, pe_elem_prob
      integer*4 pe_zsorted, pe_nshell, pe_zpos, pe_nge, pe_ne
      integer*4 lnblnk1,egs_get_unit,pe_sw_unit,ounit,egs_open_file
      integer*4 sorted(100),i,j,k,l,m
      real*8 z_sorted(100),pz_sorted(100)
      real*8 rest_xs(500,100)
      real*8 tmp_e(500,16), tmp_xs(500,16)
      real*8 new_e(500),deltaEb,slope
      integer*4 zread(100),ib(16),ibsearch
      character data_dir*128,pe_sw_file*144
      integer*4 medio,iZ,iZpos,egs_read_int,pos,curr_rec
      real*4 egs_read_real,e_r, e_old,sigma_r
      integer*2 nz, egs_read_short,ish, i_nshell,i_nge
      logical is_open, is_there, shift_required
      character*3 labels(16)
      data labels/'  K',' L1',' L2',' L3', ' M1',' M2',' M3',' M4',' M5'
     *, ' N1',' N2',' N3',' N4',' N5',' N6',' N7'/
      write(i_log,'(/a$)') ' Reading renormalized photoelectric cross se
     *ctions ......'
      data_dir = hen_house(:lnblnk1(hen_house)) // 'data' // '/'
      pe_sw_file = data_dir(:lnblnk1(data_dir)) // 'photo_shellwise.data
     *'
      pe_sw_unit = egs_get_unit(0)
      IF (( pe_sw_unit .LT. 1 )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) 'egs_init_shellwise_pe: failed to get a free Fort
     *ran I/O unit'
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      open(pe_sw_unit,file=pe_sw_file,status='old', form='UNFORMATTED',A
     *CCESS='direct',recl=1, err=8320)
      GOTO 8330
8320  write(i_log,'(/a)') '***************** Error: '
      write(i_log,'(2a)') 'egs_init_shellwise_pe: failed to open ', pe_s
     *w_file
      write(i_log,'(/a)') '***************** Quiting now.'
      call exit(1)
8330  is_open = .true.
      DO 8341 medio=1,nmed
        DO 8351 i=1,nne(medio)
          pe_nshell(i*medio) = 0
          pe_nge(i*medio) = 0
          pe_zsorted(i,medio) = 0
8351    CONTINUE
8352    CONTINUE
8341  CONTINUE
8342  CONTINUE
      DO 8361 l=1,100
        pe_zpos(l) = -1
        DO 8371 k=1,500
          pe_energy(k,l) = 0.0
          DO 8381 m=1,16
            pe_xsection(k,l,m) = 0.0
8381      CONTINUE
8382      CONTINUE
8371    CONTINUE
8372    CONTINUE
        DO 8391 k=1,16
          pe_be(l,k) = -99
8391    CONTINUE
8392    CONTINUE
8361  CONTINUE
8362  CONTINUE
      curr_rec = 1
      iZpos = 0
      nz = egs_read_short(pe_sw_unit,curr_rec)
      DO 8401 medio=1,nmed
        DO 8411 i=1,nne(medio)
          z_sorted(i) = zelem(medio,i)
8411    CONTINUE
8412    CONTINUE
        call egs_heap_sort(nne(medio),z_sorted,sorted)
        DO 8421 i=1,nne(medio)
          pe_zsorted(i,medio) = z_sorted(i)
8421    CONTINUE
8422    CONTINUE
        DO 8431 i=1,nne(medio)
          iZ = z_sorted(i)
          is_there = .false.
          DO 8441 j=1,medio-1
            DO 8451 k=1,nne(j)
              IF (( iZ .EQ. pe_zsorted(k,j) )) THEN
                is_there = .true.
                GO TO8452
              END IF
8451        CONTINUE
8452        CONTINUE
8441      CONTINUE
8442      CONTINUE
          IF((is_there))GO TO8431
          iZpos = iZpos + 1
          zread(iZpos) = iZ
          pe_zpos(iZ) = iZpos
          pos = 3 + (iZ-1)*4
          curr_rec = egs_read_int(pe_sw_unit,pos) + 1
          i_nge = egs_read_short(pe_sw_unit,curr_rec)
          i_nshell = egs_read_short(pe_sw_unit,curr_rec)
          pe_nge(iZpos) = i_nge
          pe_nshell(iZpos) = i_nshell
          e_old = -1.0
          ish = 0
          DO 8461 j=1,i_nge
            e_r = egs_read_real(pe_sw_unit,curr_rec)
            sigma_r = egs_read_real(pe_sw_unit,curr_rec)
            pe_energy(j,iZpos) = e_r
            pe_xsection(j,iZpos,0) = sigma_r
            rest_xs(j,iZpos) = sigma_r
            DO 8471 k=1,i_nshell
              sigma_r = egs_read_real(pe_sw_unit,curr_rec)
              pe_xsection(j,iZpos,k) = sigma_r
              rest_xs(j,iZpos) = rest_xs(j,iZpos) - sigma_r
8471        CONTINUE
8472        CONTINUE
            IF ((e_r - e_old .LT. 1e-15)) THEN
              pe_be(iZpos,i_nshell-ish) = e_r
              ish = ish + 1
            END IF
            e_old = e_r
8461      CONTINUE
8462      CONTINUE
8431    CONTINUE
8432    CONTINUE
8401  CONTINUE
8402  CONTINUE
      pe_ne = iZpos
      DO 8481 i=1,pe_ne
        iZ = zread(i)
        IF ((pe_nshell(i) .EQ. 0)) THEN
          DO 8491 j=1,pe_nge(i)
            pe_energy(j,i) = log(pe_energy(j,i))
8491      CONTINUE
8492      CONTINUE
          GO TO8481
        END IF
        DO 8501 l=1,pe_nshell(i)
          IF (( pe_be(i,l) .NE. binding_energies(l,iZ))) THEN
            shift_required = .true.
            deltaEb = binding_energies(l,iZ)-pe_be(i,l)
          ELSE
            shift_required =.false.
          END IF
          is_there = .false.
          DO 8511 j=1,pe_nge(i)
            tmp_e(j,l) = pe_energy(j,i)
            tmp_xs(j,l) = pe_xsection(j,i,l)
            IF (( shift_required .AND. pe_energy(j,i) .GE. pe_be(i,l) ))
     *       THEN
              tmp_e(j,l) = tmp_e(j,l) + deltaEb
              IF ((pe_energy(j,i) .EQ. pe_be(i,l) .AND. .NOT.is_there))
     *        THEN
                ib(l) = j
                is_there = .true.
              END IF
              IF ((l .EQ. 1)) THEN
                new_e(j) = tmp_e(j,l)
              ELSE IF((j .LT. ib(l-1))) THEN
                new_e(j) = tmp_e(j,l)
              END IF
            END IF
8511      CONTINUE
8512      CONTINUE
          pe_be(i,l) = binding_energies(l,iZ)
8501    CONTINUE
8502    CONTINUE
        DO 8521 l=2,pe_nshell(i)
          DO 8531 j=1,pe_nge(i)
            IF (( new_e(j) .GE. pe_be(i,l-1) )) THEN
              m = ibsearch(new_e(j),pe_nge(i),tmp_e(1,l))
              slope = log(tmp_xs(m+1,l)/tmp_xs(m,l))
              slope = slope/log(tmp_e(m+1,l)/tmp_e(m,l))
              pe_xsection(j,i,l) = log(tmp_xs(m,l))
              pe_xsection(j,i,l) = pe_xsection(j,i,l) + slope*log(new_e(
     *        j)/tmp_e(m,l))
              pe_xsection(j,i,l) = exp(pe_xsection(j,i,l))
            END IF
8531      CONTINUE
8532      CONTINUE
8521    CONTINUE
8522    CONTINUE
        DO 8541 j=1,pe_nge(i)
          IF (( j .LT. ib(pe_nshell(i)))) THEN
            new_e(j) = pe_energy(j,i)
          END IF
          m = ibsearch(new_e(j),pe_nge(i),pe_energy(1,i))
          slope = log(rest_xs(m+1,i)/rest_xs(m,i))
          slope = slope/log(pe_energy(m+1,i)/pe_energy(m,i))
          pe_xsection(j,i,0) = log(rest_xs(m,i))
          pe_xsection(j,i,0) = pe_xsection(j,i,0) + slope*log(new_e(j)/p
     *    e_energy(m,i))
          pe_xsection(j,i,0) = exp(pe_xsection(j,i,0))
          DO 8551 l=1,pe_nshell(i)
            pe_xsection(j,i,0) = pe_xsection(j,i,0) + pe_xsection(j,i,l)
8551      CONTINUE
8552      CONTINUE
8541    CONTINUE
8542    CONTINUE
        DO 8561 j=1,pe_nge(i)
          pe_energy(j,i) = log(new_e(j))
          DO 8571 l=1,pe_nshell(i)
            pe_xsection(j,i,l) = log(pe_xsection(j,i,l)/pe_xsection(j,i,
     *      0))
8571      CONTINUE
8572      CONTINUE
8561    CONTINUE
8562    CONTINUE
8481  CONTINUE
8482  CONTINUE
      write(i_log,'(a/)') ' done'
      IF((is_open))close(pe_sw_unit)
      return
      end
      SUBROUTINE RELAX(energy,n,iZ)
      implicit none
      integer*4 n,iZ
      real*8 energy
      integer*4 vac_array(50),  n_vac,  shell
      integer*4 final,finala,  final1,final2,   iql,  irl
      integer*4 first_transition(5), last_transition(5)
      integer*4 final_state(39)
      integer*4 k, np_old, ip, iarg
      real*8 e_array(50),  Ei,Ef,  Ex,  eta,  e_check,  min_E,ekcut,pkcu
     *t,elcut
      real*8 xphi,yphi,xphi2,yphi2,rhophi2, cphi,sphi
      COMMON/EPCONT/EDEP,EDEP_LOCAL,TSTEP,TUSTEP,USTEP,TVSTEP,VSTEP, RHO
     *F,EOLD,ENEW,EKE,ELKE,GLE,E_RANGE, x_final,y_final,z_final, u_final
     *,v_final,w_final, IDISC,IROLD,IRNEW,IAUSFL(35)
      DOUBLE PRECISION EDEP,  EDEP_LOCAL
      real*8 TSTEP,  TUSTEP,  USTEP,  VSTEP,  TVSTEP,  RHOF,  EOLD,  ENE
     *W,  EKE,  ELKE,  GLE,  E_RANGE, x_final,y_final,z_final,  u_final,
     *v_final,w_final
      integer*4 IDISC,  IROLD,  IRNEW,  IAUSFL
      COMMON/STACK/ E(50),X(50),Y(50),Z(50),U(50),V(50),W(50),DNEAR(50),
     *WT(50),IQ(50),IR(50),LATCH(50), LATCHI,NP,NPold
      DOUBLE PRECISION E
      real*8 X,Y,Z,  U,V,W,  DNEAR,  WT
      integer*4 IQ,  IR,  LATCH,  LATCHI, NP,  NPold
      COMMON/BOUNDS/ECUT(1),PCUT(1),VACDST
      real*8 ECUT,  PCUT,  VACDST
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      COMMON/EDGE/binding_energies(30,100), interaction_prob(6,100), rel
     *axation_prob(39,100), edge_energies(16,100), edge_number(100), edg
     *e_a(16,100), edge_b(16,100), edge_c(16,100), edge_d(16,100), IEDGF
     *L(1),IPHTER(1)
      real*8 binding_energies,  interaction_prob,    relaxation_prob,  e
     *dge_energies,  edge_a,edge_b,edge_c,edge_d
      integer*2 IEDGFL,  IPHTER
      integer*4 edge_number
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      common/relax_data/ relax_first(3000),  relax_ntran(3000),  relax_s
     *tate(10000),  relax_prob(10000),  relax_atbin(10000),  relax_ntot
      real*8 relax_prob
      integer*4 relax_first, relax_ntran, relax_state, relax_atbin, rela
     *x_ntot
      common/x_options/eadl_relax,  mcdf_pe_xsections
      logical eadl_relax, mcdf_pe_xsections
      common/user_relax/ u_relax,ish_relax,iZ_relax
      real*8 u_relax
      integer*4 ish_relax, iZ_relax
      data first_transition/1,20,27,33,38/
      data last_transition/19,26,32,37,39/
      data final_state/  4,3,5,6,  202,302,402,404,403,303,  502,503,504
     *,602,603,604,  505,605,606,  13,14,  5,6,  505,605,606,  14,  5,6,
     *  505,605,606,  5,6,  505,605,606,  6,  606/
      save first_transition,last_transition,final_state
      IF ((eadl_relax)) THEN
        call egs_eadl_relax(iZ,n)
        return
      END IF
      IF (( n .LT. 1 .OR. n .GT. 6 )) THEN
        return
      END IF
      iz_relax = iZ
      irl = ir(np)
      ekcut = ecut(irl)-rm
      pkcut = pcut(irl)
      min_E = 0.001
      IF (( energy .LE. min_E )) THEN
        edep = edep + energy
        edep_local = energy
        IARG=34
        IF ((IAUSFL(IARG+1).NE.0)) THEN
          CALL AUSGAB(IARG)
        END IF
        return
      END IF
      n_vac = 1
      vac_array(n_vac) = n
      np_old = np
      e_check = 0
      e_array(n_vac) = energy
8580  CONTINUE
8581    CONTINUE
        shell = vac_array(n_vac)
        Ei = e_array(n_vac)
        n_vac = n_vac - 1
        IF (( Ei .LE. min_E )) THEN
          edep = edep + Ei
          edep_local = Ei
          IARG=34
          IF ((IAUSFL(IARG+1).NE.0)) THEN
            CALL AUSGAB(IARG)
          END IF
          IF((n_vac .GT. 0))goto 8580
          GO TO8582
        END IF
        ish_relax = shell
        u_relax = Ei
        IF (( shell .EQ. 6 )) THEN
          IF (( Ei .GT. ekcut )) THEN
            np = np + 1
            IF (( np .GT. 50 )) THEN
              write(i_log,'(/a)') '***************** Error: '
              write(i_log,'(//,3a,/,2(a,i9))') ' In subroutine ','RELAX'
     *        , ' stack size exceeded! ',' $MAXSTACK = ',50,' np = ',np
              write(i_log,'(/a)') '***************** Quiting now.'
              call exit(1)
            END IF
            e(np) = Ei + prm
            iq(np) = -1
            X(np)=X(np-1)
            Y(np)=Y(np-1)
            Z(np)=Z(np-1)
            IR(np)=IR(np-1)
            WT(np)=WT(np-1)
            DNEAR(np)=DNEAR(np-1)
            LATCH(np)=LATCH(np-1)
            IF((rng_seed .GT. 128))call ranmar_get
            eta = rng_array(rng_seed)*twom24
            rng_seed = rng_seed + 1
            eta = 2*eta - 1
            w(np) = eta
            eta = (1-eta)*(1+eta)
            IF (( eta .GT. 1e-20 )) THEN
              eta = Sqrt(eta)
8591          CONTINUE
                IF((rng_seed .GT. 128))call ranmar_get
                xphi = rng_array(rng_seed)*twom24
                rng_seed = rng_seed + 1
                xphi = 2*xphi - 1
                xphi2 = xphi*xphi
                IF((rng_seed .GT. 128))call ranmar_get
                yphi = rng_array(rng_seed)*twom24
                rng_seed = rng_seed + 1
                yphi2 = yphi*yphi
                rhophi2 = xphi2 + yphi2
                IF(rhophi2.LE.1)GO TO8592
              GO TO 8591
8592          CONTINUE
              rhophi2 = 1/rhophi2
              cphi = (xphi2 - yphi2)*rhophi2
              sphi = 2*xphi*yphi*rhophi2
              u(np) = eta*cphi
              v(np) = eta*sphi
            ELSE
              u(np) = 0
              v(np) = 0
              w(np) = 1
            END IF
            IARG=27
            IF ((IAUSFL(IARG+1).NE.0)) THEN
              CALL AUSGAB(IARG)
            END IF
          ELSE
            edep = edep + Ei
            edep_local = Ei
            IARG=34
            IF ((IAUSFL(IARG+1).NE.0)) THEN
              CALL AUSGAB(IARG)
            END IF
          END IF
          IF((n_vac .GT. 0))goto 8580
          GO TO8582
        END IF
        IF((rng_seed .GT. 128))call ranmar_get
        eta = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        DO 8601 k=first_transition(shell),last_transition(shell)-1
          eta = eta - relaxation_prob(k,iZ)
          IF((eta .LE. 0))GO TO8602
8601    CONTINUE
8602    CONTINUE
        final = final_state(k)
        finala = final
        IF (( final .LT. 100 )) THEN
          IF (( final .LT. 10 )) THEN
            iql = 0
            elcut = pkcut
          ELSE
            final = final - 10
            iql = -1
            elcut = ekcut
          END IF
          Ef = binding_energies(final,iZ)
          Ex = Ei - Ef
          n_vac = n_vac + 1
          vac_array(n_vac) = final
          e_array(n_vac) = Ef
        ELSE
          final1 = final/100
          final2 = final - final1*100
          n_vac = n_vac + 1
          vac_array(n_vac) = final1
          e_array(n_vac) = binding_energies(final1,iZ)
          n_vac = n_vac + 1
          vac_array(n_vac) = final2
          e_array(n_vac) = binding_energies(final2,iZ)
          iql = -1
          Ex = Ei - e_array(n_vac) - e_array(n_vac-1)
          elcut = ekcut
        END IF
        IF (( Ex .LE. elcut )) THEN
          edep = edep + Ex
          IF (( finala .LT. 10 )) THEN
            edep_local = Ex
            IARG=33
            IF ((IAUSFL(IARG+1).NE.0)) THEN
              CALL AUSGAB(IARG)
            END IF
          ELSE
            edep_local = Ex
            IARG=34
            IF ((IAUSFL(IARG+1).NE.0)) THEN
              CALL AUSGAB(IARG)
            END IF
          END IF
        ELSE
          np = np + 1
          IF (( np .GT. 50 )) THEN
            write(i_log,'(/a)') '***************** Error: '
            write(i_log,'(//,3a,/,2(a,i9))') ' In subroutine ','RELAX',
     *      ' stack size exceeded! ',' $MAXSTACK = ',50,' np = ',np
            write(i_log,'(/a)') '***************** Quiting now.'
            call exit(1)
          END IF
          iq(np) = iql
          IF (( iql .EQ. 0 )) THEN
            e(np) = Ex
          ELSE
            e(np) = Ex + rm
          END IF
          X(np)=X(np-1)
          Y(np)=Y(np-1)
          Z(np)=Z(np-1)
          IR(np)=IR(np-1)
          WT(np)=WT(np-1)
          DNEAR(np)=DNEAR(np-1)
          LATCH(np)=LATCH(np-1)
          IF((rng_seed .GT. 128))call ranmar_get
          eta = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          eta = 2*eta - 1
          w(np) = eta
          eta = (1-eta)*(1+eta)
          IF (( eta .GT. 1e-20 )) THEN
            eta = Sqrt(eta)
8611        CONTINUE
              IF((rng_seed .GT. 128))call ranmar_get
              xphi = rng_array(rng_seed)*twom24
              rng_seed = rng_seed + 1
              xphi = 2*xphi - 1
              xphi2 = xphi*xphi
              IF((rng_seed .GT. 128))call ranmar_get
              yphi = rng_array(rng_seed)*twom24
              rng_seed = rng_seed + 1
              yphi2 = yphi*yphi
              rhophi2 = xphi2 + yphi2
              IF(rhophi2.LE.1)GO TO8612
            GO TO 8611
8612        CONTINUE
            rhophi2 = 1/rhophi2
            cphi = (xphi2 - yphi2)*rhophi2
            sphi = 2*xphi*yphi*rhophi2
            u(np) = eta*cphi
            v(np) = eta*sphi
          ELSE
            u(np) = 0
            v(np) = 0
            w(np) = 1
          END IF
          IF (( finala .LT. 10 )) THEN
            IARG=25
            IF ((IAUSFL(IARG+1).NE.0)) THEN
              CALL AUSGAB(IARG)
            END IF
          ELSE IF(( finala .LT. 100 )) THEN
            IARG=26
            IF ((IAUSFL(IARG+1).NE.0)) THEN
              CALL AUSGAB(IARG)
            END IF
          ELSE
            IARG=27
            IF ((IAUSFL(IARG+1).NE.0)) THEN
              CALL AUSGAB(IARG)
            END IF
          END IF
        END IF
      GO TO 8581
8582  CONTINUE
      return
      end
      subroutine egs_init_relax
      implicit none
      COMMON/BREMPR/ DL1(8,1),DL2(8,1),DL3(8,1),DL4(8,1),DL5(8,1),DL6(8,
     *1), ALPHI(2,1),BPAR(2,1),DELPOS(2,1), WA(1,50),PZ(1,50),ZELEM(1,50
     *),RHOZ(1,50), PWR2I(50), DELCM(1),ZBRANG(1),LZBRANG(1),NNE(1), IBR
     *DST,IPRDST,ibr_nist,pair_nrc,itriplet, ASYM(1,50,2)
      CHARACTER*4 ASYM
      real*8 DL1,DL2,DL3,DL4,DL5,DL6,   ALPHI,  BPAR,  DELPOS,  WA,  PZ,
     *  ZELEM,  RHOZ,  PWR2I,  DELCM,  ZBRANG,  LZBRANG
      integer*4 NNE,  IBRDST,  IPRDST,  ibr_nist,  itriplet,  pair_nrc
      COMMON/EDGE/binding_energies(30,100), interaction_prob(6,100), rel
     *axation_prob(39,100), edge_energies(16,100), edge_number(100), edg
     *e_a(16,100), edge_b(16,100), edge_c(16,100), edge_d(16,100), IEDGF
     *L(1),IPHTER(1)
      real*8 binding_energies,  interaction_prob,    relaxation_prob,  e
     *dge_energies,  edge_a,edge_b,edge_c,edge_d
      integer*2 IEDGFL,  IPHTER
      integer*4 edge_number
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      COMMON/PHOTIN/ EBINDA(1), GE0(1),GE1(1), GMFP0(2000,1),GMFP1(2000,
     *1),GBR10(2000,1),GBR11(2000,1),GBR20(2000,1),GBR21(2000,1), RCO0(1
     *),RCO1(1), RSCT0(100,1),RSCT1(100,1), COHE0(2000,1),COHE1(2000,1),
     *  PHOTONUC0(2000,1),PHOTONUC1(2000,1), DPMFP, MPGEM(1,1), NGR(1)
      real*8 EBINDA,  GE0,GE1,  GMFP0,GMFP1,  GBR10,GBR11,  GBR20,GBR21,
     *  RCO0,RCO1,  RSCT0,RSCT1,  COHE0,COHE1,   PHOTONUC0,PHOTONUC1,  D
     *PMFP
      integer*4
     *                  MPGEM,
     *                          NGR
      COMMON/THRESH/RMT2,RMSQ, AP(1),AE(1),UP(1),UE(1),TE(1),THMOLL(1)
      real*8 RMT2,  RMSQ,  AP,  AE,  UP,  UE,  TE,  THMOLL
      common/x_options/eadl_relax,  mcdf_pe_xsections
      logical eadl_relax, mcdf_pe_xsections
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      common/relax_data/ relax_first(3000),  relax_ntran(3000),  relax_s
     *tate(10000),  relax_prob(10000),  relax_atbin(10000),  relax_ntot
      real*8 relax_prob
      integer*4 relax_first, relax_ntran, relax_state, relax_atbin, rela
     *x_ntot
      common/shell_data/ shell_be(3000),  shell_type(3000),  shell_num(3
     *000),  shell_Z(3000),  shell_eadl(100,30),  shell_ntot
      real*8 shell_be
      integer*4 shell_type,shell_Z,shell_ntot,shell_num,shell_eadl
      integer*4 lnblnk1,egs_get_unit,relax_unit,ounit,egs_open_file
      integer*4 sorted(100),i,j,k,k1,k2,m
      real*8 z_sorted(100),pz_sorted(100)
      character data_dir*128,relax_file*144
      integer*4 ish,medio,iZ,ntran
      real*8 Ec, Pc, tmp, min_be, sumw,Ex
      logical is_open, is_there
      real*8 wtmp(300)
      integer*4 itmp(300)
      integer*4 pos, curr_rec, sh_eadl
      integer*4 nz, nshell, tr_type
      integer*4 ttype
      real*4 be_r, prob_r
      DO 8621 iZ=1,100
        DO 8631 k=1,30
          shell_eadl(iZ,k) = -1
8631    CONTINUE
8632    CONTINUE
8621  CONTINUE
8622  CONTINUE
      min_be = 0.001
      write(i_log,'(/a)') ' Reading EADL relaxation data ......'
      data_dir = hen_house(:lnblnk1(hen_house)) // 'data' // '/'
      relax_file = data_dir(:lnblnk1(data_dir)) // 'relax.data'
      relax_unit = egs_get_unit(0)
      IF (( relax_unit .LT. 1 )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) 'egs_init_relax: failed to get a free Fortran I/O
     * unit'
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      open(relax_unit,file=relax_file,status='old', form='UNFORMATTED',A
     *CCESS='direct',recl=4, err=8640)
      GOTO 8650
8640  write(i_log,'(/a)') '***************** Error: '
      write(i_log,'(2a)') 'egs_init_relax: failed to open ', relax_file
      write(i_log,'(/a)') '***************** Quiting now.'
      call exit(1)
8650  is_open = .true.
      curr_rec = 1
      read(relax_unit,rec=curr_rec) nz
      shell_ntot = 0
      relax_ntot = 0
      DO 8661 medio=1,nmed
        DO 8671 i=1,nne(medio)
          z_sorted(i) = zelem(medio,i)
8671    CONTINUE
8672    CONTINUE
        call egs_heap_sort(nne(medio),z_sorted,sorted)
        DO 8681 i=1,nne(medio)
          iZ = z_sorted(i)
          is_there = .false.
          DO 8691 j=1,shell_ntot
            IF (( iZ .EQ. shell_Z(j) )) THEN
              is_there = .true.
              GO TO8692
            END IF
8691      CONTINUE
8692      CONTINUE
          IF((is_there))GO TO8681
          pos = iZ + 1
          read(relax_unit,rec=pos) curr_rec
          read(relax_unit,rec=curr_rec) nshell
          IF (( shell_ntot + nshell .GT. 3000 )) THEN
            write(i_log,'(/a)') '***************** Error: '
            write(i_log,'(a,i5,a/,a//)') ' Too many shells to fit in the
     * list: ', shell_ntot + nshell,' (at least).', ' Increase the param
     *eter $MAXSHELL and retry '
            write(i_log,'(/a)') '***************** Quiting now.'
            call exit(1)
          END IF
          write(i_log,'(a,i3,a,i2,a)') '  Z = ',iZ,' has ',nshell,' shel
     *ls'
          DO 8701 ish=shell_ntot+1,shell_ntot+nshell
            curr_rec = curr_rec+1
            read(relax_unit,rec=curr_rec) shell_type(ish)
            curr_rec = curr_rec+1
            read(relax_unit,rec=curr_rec) ntran
            curr_rec = curr_rec+1
            read(relax_unit,rec=curr_rec) be_r
            shell_be(ish) = be_r
            shell_Z(ish) = iZ
            shell_num(ish) = ish - shell_ntot
            shell_eadl(iZ,shell_num(ish)) = ish
            IF ((binding_energies(shell_num(ish),iZ) .GT. 0)) THEN
              shell_be(ish) = binding_energies(shell_num(ish),iZ)
            ELSE IF(( photon_xsections .EQ. 'epdl' )) THEN
              binding_energies(shell_num(ish),iZ) = shell_be(ish)
            END IF
            DO 8711 k=1,ntran
              curr_rec = curr_rec+1
              read(relax_unit,rec=curr_rec) itmp(k)
              curr_rec = curr_rec+1
              read(relax_unit,rec=curr_rec) prob_r
              wtmp(k)=prob_r
              IF ((itmp(k).LT.64)) THEN
                itmp(k) = itmp(k) + 1
              ELSE
                itmp(k) = itmp(k) + 65
              END IF
8711        CONTINUE
8712        CONTINUE
            IF (( shell_be(ish) .LT. min_be )) THEN
              relax_first(ish) = -1
              relax_ntran(ish) = -1
            ELSE
              sumw = 0
              DO 8721 k=1,ntran
                sumw = sumw + wtmp(k)
8721          CONTINUE
8722          CONTINUE
              IF (( sumw .GT. 1 )) THEN
                DO 8731 k=1,ntran
                  wtmp(k) = wtmp(k)/sumw
8731            CONTINUE
8732            CONTINUE
              ELSE IF(( sumw .LT. 1 )) THEN
                ntran = ntran + 1
                itmp(ntran) = -1
                wtmp(ntran) = 1-sumw
              END IF
              IF (( relax_ntot + ntran .GT. 10000 )) THEN
                write(i_log,'(/a)') '***************** Error: '
                write(i_log,'(a,i5,a/,a/)') ' Too many relaxation transi
     *tions: ', relax_ntot + ntran,' (at least).', ' Increase $MAXRELAX
     *and retry '
                write(i_log,'(/a)') '***************** Quiting now.'
                call exit(1)
              END IF
              relax_first(ish) = relax_ntot+1
              relax_ntran(ish) = ntran
              call prepare_alias_histogram(ntran,wtmp, relax_atbin(relax
     *        _ntot+1))
              DO 8741 k=1,ntran
                j = relax_ntot + k
                relax_state(j) = itmp(k)
                relax_prob(j) = wtmp(k)
8741          CONTINUE
8742          CONTINUE
              relax_ntot = relax_ntot + ntran
            END IF
8701      CONTINUE
8702      CONTINUE
          shell_ntot = shell_ntot + nshell
8681    CONTINUE
8682    CONTINUE
8661  CONTINUE
8662  CONTINUE
      write(i_log,'(a/)') ' ...... Done.'
      IF((is_open))close(relax_unit)
      return
      stop
      end
      subroutine egs_eadl_relax(iZ, shell_egs)
      implicit none
      common/relax_data/ relax_first(3000),  relax_ntran(3000),  relax_s
     *tate(10000),  relax_prob(10000),  relax_atbin(10000),  relax_ntot
      real*8 relax_prob
      integer*4 relax_first, relax_ntran, relax_state, relax_atbin, rela
     *x_ntot
      common/relax_for_user/ rfu_E0,  rfu_E,  rfu_Z,  rfu_j0,  rfu_n0,
     *rfu_t0,  rfu_j,  rfu_n,  rfu_t
      integer*4 rfu_Z,rfu_j0,rfu_n0,rfu_t0,rfu_j,rfu_n,rfu_t
      real*8 rfu_E0,rfu_E
      common/shell_data/ shell_be(3000),  shell_type(3000),  shell_num(3
     *000),  shell_Z(3000),  shell_eadl(100,30),  shell_ntot
      real*8 shell_be
      integer*4 shell_type,shell_Z,shell_ntot,shell_num,shell_eadl
      COMMON/STACK/ E(50),X(50),Y(50),Z(50),U(50),V(50),W(50),DNEAR(50),
     *WT(50),IQ(50),IR(50),LATCH(50), LATCHI,NP,NPold
      DOUBLE PRECISION E
      real*8 X,Y,Z,  U,V,W,  DNEAR,  WT
      integer*4 IQ,  IR,  LATCH,  LATCHI, NP,  NPold
      COMMON/THRESH/RMT2,RMSQ, AP(1),AE(1),UP(1),UE(1),TE(1),THMOLL(1)
      real*8 RMT2,  RMSQ,  AP,  AE,  UP,  UE,  TE,  THMOLL
      COMMON/EPCONT/EDEP,EDEP_LOCAL,TSTEP,TUSTEP,USTEP,TVSTEP,VSTEP, RHO
     *F,EOLD,ENEW,EKE,ELKE,GLE,E_RANGE, x_final,y_final,z_final, u_final
     *,v_final,w_final, IDISC,IROLD,IRNEW,IAUSFL(35)
      DOUBLE PRECISION EDEP,  EDEP_LOCAL
      real*8 TSTEP,  TUSTEP,  USTEP,  VSTEP,  TVSTEP,  RHOF,  EOLD,  ENE
     *W,  EKE,  ELKE,  GLE,  E_RANGE, x_final,y_final,z_final,  u_final,
     *v_final,w_final
      integer*4 IDISC,  IROLD,  IRNEW,  IAUSFL
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      COMMON/UPHIOT/THETA,SINTHE,COSTHE,SINPHI, COSPHI,PI,TWOPI,PI5D2
      real*8 THETA,  SINTHE,  COSTHE,  SINPHI,  COSPHI,  PI,TWOPI,PI5D2
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      COMMON/BOUNDS/ECUT(1),PCUT(1),VACDST
      real*8 ECUT,  PCUT,  VACDST
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      COMMON/MISC/  DUNIT,KMPI,KMPO,RHOR(1),MED(1),IRAYLR(1),IPHOTONUCR(
     *1)
      real*8 DUNIT,  RHOR
      integer*4 KMPI,  KMPO
      integer*2 MED,  IRAYLR,  IPHOTONUCR
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      common/x_options/eadl_relax,  mcdf_pe_xsections
      logical eadl_relax, mcdf_pe_xsections
      common/user_relax/ u_relax,ish_relax,iZ_relax
      real*8 u_relax
      integer*4 ish_relax, iZ_relax
      real*8 Ec,Pc,min_E,rnno,Evac,Ef,Ef1,Ef2,Ex,Ecc, cost,sint,cphi,sph
     *i
      integer*4 shell, shell_egs, iZ, iarg
      integer*4 irl,vacs(100),nvac,vac,new_state,iqf,np_save,new1,new2
      integer*4 sample_alias_histogram
      real*8 xphi,xphi2,yphi,yphi2,rhophi2
      shell = shell_eadl(iZ,shell_egs)
      IF (( shell .LT. 1 .OR. shell .GT. 3000 )) THEN
        return
      END IF
      irl = ir(np)
      Ec = ecut(irl) - rm
      Pc = pcut(irl)
      min_E = 0.001
      Evac = shell_be(shell)
      rfu_Z = shell_Z(shell)
      rfu_j0 = shell
      rfu_n0 = shell_num(shell)
      rfu_t0 = shell_type(shell)
      rfu_E0 = Evac
      IF ((shell_egs .GT. 4 .AND. .NOT.mcdf_pe_xsections)) THEN
        edep = edep + Evac
        edep_local = Evac
        IARG=34
        IF ((IAUSFL(IARG+1).NE.0)) THEN
          CALL AUSGAB(IARG)
        END IF
        return
      END IF
      vac = shell
      Nvac = 0
      np_save = np
8751  CONTINUE
        IF (( Evac .LT. min_E .OR. relax_ntran(vac) .LT. 1 )) THEN
          edep = edep + Evac
          edep_local = Evac
          IARG=34
          IF ((IAUSFL(IARG+1).NE.0)) THEN
            CALL AUSGAB(IARG)
          END IF
          go to 8760
        END IF
        new_state = sample_alias_histogram(relax_ntran(vac), relax_prob(
     *  relax_first(vac)), relax_atbin(relax_first(vac)))
        IF (( new_state .LT. 0 )) THEN
          Ef = 0
          iqf = -1
          Ecc = Ec
        ELSE
          new_state = relax_state(relax_first(vac)+new_state-1)
          IF (( new_state .LE. 64 )) THEN
            iqf = 0
            new_state = new_state + vac - shell_num(vac)
            Ef = shell_be(new_state)
            Nvac = Nvac + 1
            vacs(Nvac) = new_state
            Ecc = Pc
          ELSE
            iqf = -1
            new1 = new_state/64
            new2 = new_state - 64*new1
            new1 = new1 + vac - shell_num(vac)
            new2 = new2 + vac - shell_num(vac)
            Ef1 = shell_be(new1)
            Ef2 = shell_be(new2)
            Nvac = Nvac + 1
            vacs(Nvac) = new1
            Nvac = Nvac + 1
            vacs(Nvac) = new2
            Ef = Ef1 + Ef2
            Ecc = Ec
          END IF
        END IF
        Ex = Evac - Ef
        edep_local = 0
        IF (( Ex .GT. Ecc )) THEN
          np = np + 1
          IF (( np .GT. 50 )) THEN
            write(i_log,'(/a)') '***************** Warning: '
            write(i_log,'(3(a,f10.6),a,i2)') 'Evac = ',Evac, ' Ef = ',Ef
     *      ,  ' min_E = ', min_E,' iq = ',iqf
            write(i_log,'(/a)') '***************** Error: '
            write(i_log,'(//,3a,/,2(a,i9),/,a)') ' In subroutine ','new_
     *relax', ' stack size exceeded! ',' $MXSTACK = ',50,' np = ',np, '
     *Increase $MXSTACK and try again '
            write(i_log,'(/a)') '***************** Quiting now.'
            call exit(1)
          END IF
          iq(np) = iqf
          X(np)=X(np_save)
          Y(np)=Y(np_save)
          Z(np)=Z(np_save)
          IR(np)=IR(np_save)
          WT(np)=WT(np_save)
          DNEAR(np)=DNEAR(np_save)
          LATCH(np)=LATCH(np_save)
          IF((rng_seed .GT. 128))call ranmar_get
          rnno = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          cost = 2*rnno-1
          sint = 1-cost*cost
          IF (( sint .GT. 0 )) THEN
            sint = sqrt(sint)
8771        CONTINUE
              IF((rng_seed .GT. 128))call ranmar_get
              xphi = rng_array(rng_seed)*twom24
              rng_seed = rng_seed + 1
              xphi = 2*xphi - 1
              xphi2 = xphi*xphi
              IF((rng_seed .GT. 128))call ranmar_get
              yphi = rng_array(rng_seed)*twom24
              rng_seed = rng_seed + 1
              yphi2 = yphi*yphi
              rhophi2 = xphi2 + yphi2
              IF(rhophi2.LE.1)GO TO8772
            GO TO 8771
8772        CONTINUE
            rhophi2 = 1/rhophi2
            cphi = (xphi2 - yphi2)*rhophi2
            sphi = 2*xphi*yphi*rhophi2
            u(np) = sint*cphi
            v(np) = sint*sphi
            w(np) = cost
          ELSE
            u(np) = 0
            v(np) = 0
            w(np) = cost
          END IF
          rfu_j = vac
          rfu_n = shell_num(vac)
          rfu_t = shell_type(vac)
          rfu_E = shell_be(vac)
          IF (( iqf .EQ. 0 )) THEN
            e(np) = Ex
            IARG=25
            IF ((IAUSFL(IARG+1).NE.0)) THEN
              CALL AUSGAB(IARG)
            END IF
          ELSE
            e(np) = Ex + rm
            IARG=27
            IF ((IAUSFL(IARG+1).NE.0)) THEN
              CALL AUSGAB(IARG)
            END IF
          END IF
        ELSE
          edep = edep + Ex
          IF (( iqf .EQ. 0 )) THEN
            edep_local = Ex
            IARG=33
            IF ((IAUSFL(IARG+1).NE.0)) THEN
              CALL AUSGAB(IARG)
            END IF
          ELSE
            edep_local = Ex
            IARG=34
            IF ((IAUSFL(IARG+1).NE.0)) THEN
              CALL AUSGAB(IARG)
            END IF
          END IF
        END IF
8760    CONTINUE
        IF((Nvac .EQ. 0))GO TO8752
        vac = vacs(Nvac)
        Evac = shell_be(vac)
        Nvac = Nvac - 1
      GO TO 8751
8752  CONTINUE
      return
      end
      subroutine init_triplet
      implicit none
      COMMON/BREMPR/ DL1(8,1),DL2(8,1),DL3(8,1),DL4(8,1),DL5(8,1),DL6(8,
     *1), ALPHI(2,1),BPAR(2,1),DELPOS(2,1), WA(1,50),PZ(1,50),ZELEM(1,50
     *),RHOZ(1,50), PWR2I(50), DELCM(1),ZBRANG(1),LZBRANG(1),NNE(1), IBR
     *DST,IPRDST,ibr_nist,pair_nrc,itriplet, ASYM(1,50,2)
      CHARACTER*4 ASYM
      real*8 DL1,DL2,DL3,DL4,DL5,DL6,   ALPHI,  BPAR,  DELPOS,  WA,  PZ,
     *  ZELEM,  RHOZ,  PWR2I,  DELCM,  ZBRANG,  LZBRANG
      integer*4 NNE,  IBRDST,  IPRDST,  ibr_nist,  itriplet,  pair_nrc
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      common/triplet_data/ a_triplet(250,1), b_triplet(250,1), dl_triple
     *t, dli_triplet, bli_triplet, log_4rm
      real*8 a_triplet,b_triplet,dl_triplet, dli_triplet, bli_triplet, l
     *og_4rm
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      real*8 energies(55), sig_pair(100,55), sig_triplet(100,55), f_trip
     *let(55), sigp(55), sigt(55), as(55), bs(55), cs(55), ds(55)
      character*128 triplet_data_file
      integer*4 want_triplet_unit, triplet_unit, triplet_out
      integer*4 i, iel, imed, lnblnk1, egs_get_unit, ntrip, iz1, izi, if
     *irst
      real*8 logE, f_new, f_old, spline
      IF((itriplet .EQ. 0))return
      DO 8781 i=1,len(triplet_data_file)
        triplet_data_file(i:i) = ' '
8781  CONTINUE
8782  CONTINUE
      triplet_data_file = hen_house(:lnblnk1(hen_house)) // 'data' // '/
     *' // 'triplet.data'
      want_triplet_unit = 63
      triplet_unit = egs_get_unit(want_triplet_unit)
      IF (( triplet_unit .LT. 1 )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) 'init_triplet: failed to get a free Fortran I/O u
     *nit'
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      open(triplet_unit,file=triplet_data_file,err=8790)
      write(i_log,'(a,$)') ' init_triplet: reading triplet data ... '
      read(triplet_unit,*) ntrip
      IF (( ntrip .GT. 55 )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) 'Max. number of data points per element is ',55
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      read(triplet_unit,*,err=8800) (energies(i),i=1,ntrip)
      DO 8811 iel=1,100
        read(triplet_unit,*)
        read(triplet_unit,*,err=8800) (sig_pair(iel,i),i=1,ntrip)
        read(triplet_unit,*,err=8800) (sig_triplet(iel,i),i=1,ntrip)
8811  CONTINUE
8812  CONTINUE
      write(i_log,*) 'OK'
      ifirst = 0
      DO 8821 i=1,ntrip
        IF((ifirst .EQ. 0 .AND. energies(i) .GT. 4.01*rm))ifirst = i
        energies(i) = log(energies(i))
8821  CONTINUE
8822  CONTINUE
      log_4rm = log(4*rm)
      energies(ifirst-1) = log_4rm
      dl_triplet = (energies(ntrip) - log_4rm)/250
      dli_triplet = 1/dl_triplet
      bli_triplet = 1 - log_4rm/dl_triplet
      DO 8831 imed=1,nmed
        write(i_log,'(a,i3,a,$)') '   Preparing triplet fraction data fo
     *r medium ',imed,' ... '
        iz1 = zelem(imed,1) + 0.1
        DO 8841 i=1,ntrip
          sigp(i) = pz(imed,1)*sig_pair(iz1,i)
          sigt(i) = pz(imed,1)*sig_triplet(iz1,i)
          DO 8851 iel=2,nne(imed)
            izi = zelem(imed,iel) + 0.1
            sigp(i) = sigp(i) + pz(imed,iel)*sig_pair(izi,i)
            sigt(i) = sigt(i) + pz(imed,iel)*sig_triplet(izi,i)
8851      CONTINUE
8852      CONTINUE
8841    CONTINUE
8842    CONTINUE
        DO 8861 i=ifirst,ntrip
          f_triplet(i-ifirst+2) = sigt(i)/(sigp(i) + sigt(i))
8861    CONTINUE
8862    CONTINUE
        f_triplet(1) = 0
        call set_spline(energies(ifirst-1),f_triplet,as,bs,cs,ds,ntrip-i
     *  first+2)
        logE = log_4rm
        f_old = 0
        DO 8871 i=1,250-1
          logE = logE + dl_triplet
          f_new = spline(logE,energies(ifirst-1),as,bs,cs,ds,ntrip-ifirs
     *    t+2)
          a_triplet(i,imed) = (f_new - f_old)*dli_triplet
          b_triplet(i,imed) = f_new - a_triplet(i,imed)*logE
          f_old = f_new
8871    CONTINUE
8872    CONTINUE
        write(i_log,*) 'OK'
8831  CONTINUE
8832  CONTINUE
      close(triplet_unit)
      return
8790  CONTINUE
      write(i_log,'(/a)') '***************** Error: '
      write(i_log,'(a,a)') ' init_triplet: failed to open the data file
     *', triplet_data_file(:lnblnk1(triplet_data_file))
      write(i_log,'(/a)') '***************** Quiting now.'
      call exit(1)
8800  CONTINUE
      write(i_log,'(/a)') '***************** Error: '
      write(i_log,*) ' init_triplet: error while reading triplet data '
      write(i_log,'(/a)') '***************** Quiting now.'
      call exit(1)
      return
      end
      SUBROUTINE EDGSET(NREGLO,NREGHI)
      implicit none
      COMMON/EDGE/binding_energies(30,100), interaction_prob(6,100), rel
     *axation_prob(39,100), edge_energies(16,100), edge_number(100), edg
     *e_a(16,100), edge_b(16,100), edge_c(16,100), edge_d(16,100), IEDGF
     *L(1),IPHTER(1)
      real*8 binding_energies,  interaction_prob,    relaxation_prob,  e
     *dge_energies,  edge_a,edge_b,edge_c,edge_d
      integer*2 IEDGFL,  IPHTER
      integer*4 edge_number
      common/x_options/eadl_relax,  mcdf_pe_xsections
      logical eadl_relax, mcdf_pe_xsections
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      integer NREGLO,NREGHI
      integer*4 i,j,k,jj,iz
      logical do_relax
      logical got_data
      save got_data
      data got_data/.false./
      IF((got_data))return
      write(i_log,'(a/,a)') 'Output from subroutine EDGSET:', '=========
     *====================='
      do_relax = .false.
      DO 8881 j=1,1
        IF (( iedgfl(j) .GT. 0 .AND. iedgfl(j) .LE. 100 )) THEN
          do_relax = .true.
          GO TO8882
        END IF
8881  CONTINUE
8882  CONTINUE
      IF (( .NOT.do_relax )) THEN
        IF ((eadl_relax)) THEN
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,'(a,/a)') 'You must turn ON atomic relaxations whe
     *n requesting', 'detailed atomic relaxation (eadl_relax=true)!'
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        END IF
        write(i_log,'(a/)') ' Atomic relaxations not requested! '
        return
      END IF
      write(i_log,'(a/)') ' Atomic relaxations requested! '
      write(i_log,'(a$)') ' Reading simplified photo-absorption data ...
     *..'
      got_data = .true.
      rewind(i_photo_relax)
      DO 8891 i=1,100
        IF ((eadl_relax)) THEN
          read(i_photo_relax,*)
        ELSE
          read(i_photo_relax,*) j,(binding_energies(k,i),k=1,6)
          DO 8901 k=1,6
            binding_energies(k,i) = binding_energies(k,i)*1e-6
8901      CONTINUE
8902      CONTINUE
        END IF
8891  CONTINUE
8892  CONTINUE
      read(i_photo_relax,*)
      DO 8911 i=1,100
        read(i_photo_relax,*) j,(interaction_prob(k,i),k=1,5)
        interaction_prob(6,i)=1.01
8911  CONTINUE
8912  CONTINUE
      write(i_log,'(a)') ' Done'
      write(i_log,'(/a$)') ' Reading simplified relaxation data .....'
      read(i_photo_relax,*)
      DO 8921 i=1,100
        read(i_photo_relax,*) j,(relaxation_prob(k,i),k=1,19)
8921  CONTINUE
8922  CONTINUE
      read(i_photo_relax,*)
      DO 8931 i=1,100
        read(i_photo_relax,*) j,(relaxation_prob(k,i),k=20,26)
8931  CONTINUE
8932  CONTINUE
      read(i_photo_relax,*)
      DO 8941 i=1,100
        read(i_photo_relax,*) j,(relaxation_prob(k,i),k=27,32)
8941  CONTINUE
8942  CONTINUE
      read(i_photo_relax,*)
      DO 8951 i=1,100
        read(i_photo_relax,*) j,(relaxation_prob(k,i),k=33,37)
8951  CONTINUE
8952  CONTINUE
      read(i_photo_relax,*)
      DO 8961 i=1,100
        read(i_photo_relax,*) j,relaxation_prob(38,i)
8961  CONTINUE
8962  CONTINUE
      write(i_log,'(a)') ' Done'
      write(i_log,'(/a$)') ' Reading parametrized XCOM photo cross secti
     *on data .....'
      rewind(i_photo_cs)
      DO 8971 i=1,100
        read(i_photo_cs,*) j,edge_number(i)
        DO 8981 j=1,edge_number(i)
          read(i_photo_cs,*) edge_a(j,i),edge_b(j,i),edge_c(j,i), edge_d
     *    (j,i),edge_energies(j,i)
8981    CONTINUE
8982    CONTINUE
8971  CONTINUE
8972  CONTINUE
      write(i_log,'(a)') ' Done'
      IF ((eadl_relax)) THEN
        call egs_init_relax
      END IF
      RETURN
      END
      SUBROUTINE PHOTON(IRCODE)
      implicit none
      integer*4 IRCODE
      COMMON/QDEBUG/QDEBUG
      LOGICAL QDEBUG
      COMMON/BOUNDS/ECUT(1),PCUT(1),VACDST
      real*8 ECUT,  PCUT,  VACDST
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      COMMON/MISC/  DUNIT,KMPI,KMPO,RHOR(1),MED(1),IRAYLR(1),IPHOTONUCR(
     *1)
      real*8 DUNIT,  RHOR
      integer*4 KMPI,  KMPO
      integer*2 MED,  IRAYLR,  IPHOTONUCR
      COMMON/EPCONT/EDEP,EDEP_LOCAL,TSTEP,TUSTEP,USTEP,TVSTEP,VSTEP, RHO
     *F,EOLD,ENEW,EKE,ELKE,GLE,E_RANGE, x_final,y_final,z_final, u_final
     *,v_final,w_final, IDISC,IROLD,IRNEW,IAUSFL(35)
      DOUBLE PRECISION EDEP,  EDEP_LOCAL
      real*8 TSTEP,  TUSTEP,  USTEP,  VSTEP,  TVSTEP,  RHOF,  EOLD,  ENE
     *W,  EKE,  ELKE,  GLE,  E_RANGE, x_final,y_final,z_final,  u_final,
     *v_final,w_final
      integer*4 IDISC,  IROLD,  IRNEW,  IAUSFL
      COMMON/PHOTIN/ EBINDA(1), GE0(1),GE1(1), GMFP0(2000,1),GMFP1(2000,
     *1),GBR10(2000,1),GBR11(2000,1),GBR20(2000,1),GBR21(2000,1), RCO0(1
     *),RCO1(1), RSCT0(100,1),RSCT1(100,1), COHE0(2000,1),COHE1(2000,1),
     *  PHOTONUC0(2000,1),PHOTONUC1(2000,1), DPMFP, MPGEM(1,1), NGR(1)
      real*8 EBINDA,  GE0,GE1,  GMFP0,GMFP1,  GBR10,GBR11,  GBR20,GBR21,
     *  RCO0,RCO1,  RSCT0,RSCT1,  COHE0,COHE1,   PHOTONUC0,PHOTONUC1,  D
     *PMFP
      integer*4
     *                  MPGEM,
     *                          NGR
      COMMON/STACK/ E(50),X(50),Y(50),Z(50),U(50),V(50),W(50),DNEAR(50),
     *WT(50),IQ(50),IR(50),LATCH(50), LATCHI,NP,NPold
      DOUBLE PRECISION E
      real*8 X,Y,Z,  U,V,W,  DNEAR,  WT
      integer*4 IQ,  IR,  LATCH,  LATCHI, NP,  NPold
      COMMON/THRESH/RMT2,RMSQ, AP(1),AE(1),UP(1),UE(1),TE(1),THMOLL(1)
      real*8 RMT2,  RMSQ,  AP,  AE,  UP,  UE,  TE,  THMOLL
      COMMON/UPHIOT/THETA,SINTHE,COSTHE,SINPHI, COSPHI,PI,TWOPI,PI5D2
      real*8 THETA,  SINTHE,  COSTHE,  SINPHI,  COSPHI,  PI,TWOPI,PI5D2
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      common/egs_vr/ e_max_rr(1),  prob_RR,  nbr_split,  i_play_RR,
     * i_survived_RR,
     *     n_RR_warning,                                        i_do_rr(
     *1)
      real*8          e_max_rr,prob_RR
      integer*4       nbr_split,i_play_RR,i_survived_RR,n_RR_warning
      integer*2     i_do_rr
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      DOUBLE PRECISION PEIG
      real*8 EIG,  RNNO35,  GMFPR0,  GMFP,  COHFAC,  RNNO37,  XXX,  X2,
     * Q2,  CSQTHE,  REJF,  RNNORJ,  RNNO36,  GBR1,  GBR2,  T,   PHOTONU
     *CFAC,  RNNO39
      integer*4 IARG,  IDR,  IRL,  LGLE,  LXXX
      IRCODE=1
      PEIG=E(NP)
      EIG=PEIG
      IRL=IR(NP)
      medium = med(irl)
      IF ((EIG .LE. PCUT(IRL))) THEN
        GO TO 8990
      END IF
9000  CONTINUE
9001    CONTINUE
        IF ((WT(NP) .EQ. 0.0)) THEN
          go to 9010
        END IF
        GLE=LOG(EIG)
        dpmfp = 0
        IROLD=IR(NP)
9020    CONTINUE
9021      CONTINUE
          IF ((MEDIUM.NE.0)) THEN
            LGLE=GE1(MEDIUM)*GLE+GE0(MEDIUM)
            GMFPR0=GMFP1(LGLE,MEDIUM)*GLE+GMFP0(LGLE,MEDIUM)
          END IF
9030      CONTINUE
9031        CONTINUE
            IF ((MEDIUM.EQ.0)) THEN
              TSTEP=VACDST
            ELSE
              RHOF=RHOR(IRL)/RHO(MEDIUM)
              GMFP=GMFPR0/RHOF
              IF ((IRAYLR(IRL).EQ.1)) THEN
                COHFAC=COHE1(LGLE,MEDIUM)*GLE+COHE0(LGLE,MEDIUM)
                GMFP=GMFP*COHFAC
              END IF
              IF ((IPHOTONUCR(IRL).EQ.1)) THEN
                PHOTONUCFAC=PHOTONUC1(LGLE,MEDIUM)*GLE+PHOTONUC0(LGLE,ME
     *          DIUM)
                GMFP=GMFP*PHOTONUCFAC
              END IF
              TSTEP=GMFP*DPMFP
            END IF
            IRNEW=IR(NP)
            IDISC=0
            USTEP=TSTEP
            TUSTEP=USTEP
            IF((wt(np) .LE. 0))idisc = 1
            IF ((IDISC.GT.0)) THEN
              GO TO 9010
            END IF
            VSTEP=USTEP
            TVSTEP=VSTEP
            EDEP=PZERO
            x_final = x(np) + u(np)*vstep
            y_final = y(np) + v(np)*vstep
            z_final = z(np) + w(np)*vstep
            IARG=0
            IF ((IAUSFL(IARG+1).NE.0)) THEN
              CALL AUSGAB(IARG)
            END IF
            x(np) = x_final
            y(np) = y_final
            z(np) = z_final
            DNEAR(NP)=DNEAR(NP)-USTEP
            IF ((MEDIUM.NE.0)) THEN
              DPMFP=MAX(0.,DPMFP-USTEP/GMFP)
            END IF
            IROLD=IR(NP)
            MEDOLD=MEDIUM
            IF ((IRNEW.NE.IROLD)) THEN
              ir(np) = irnew
              irl = irnew
              medium = med(irl)
            END IF
            IARG=5
            IF ((IAUSFL(IARG+1).NE.0)) THEN
              CALL AUSGAB(IARG)
            END IF
            IF ((EIG.LE.PCUT(IRL))) THEN
              GO TO 8990
            END IF
            IF((IDISC.LT.0))GO TO 9010
            IF((MEDIUM.NE.MEDOLD))GO TO 9032
            IF ((MEDIUM.NE.0.AND.DPMFP.LE.1.E-5)) THEN
              GO TO 9022
            END IF
          GO TO 9031
9032      CONTINUE
        GO TO 9021
9022    CONTINUE
        IF ((IRAYLR(IRL).EQ.1)) THEN
          IF((rng_seed .GT. 128))call ranmar_get
          RNNO37 = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          IF ((RNNO37.LE.(1.0-COHFAC))) THEN
            IARG=23
            IF ((IAUSFL(IARG+1).NE.0)) THEN
              CALL AUSGAB(IARG)
            END IF
            NPold = NP
            call egs_rayleigh_sampling(MEDIUM,E(NP),GLE,LGLE,COSTHE,SINT
     *      HE)
            CALL UPHI(2,1)
            IARG=24
            IF ((IAUSFL(IARG+1).NE.0)) THEN
              CALL AUSGAB(IARG)
            END IF
            GOTO 9000
          END IF
        END IF
        IF ((IPHOTONUCR(IRL).EQ.1)) THEN
          IF((rng_seed .GT. 128))call ranmar_get
          RNNO39 = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          IF ((RNNO39.LE.(1.0-PHOTONUCFAC))) THEN
            IARG=29
            IF ((IAUSFL(IARG+1).NE.0)) THEN
              CALL AUSGAB(IARG)
            END IF
            call PHOTONUC
            IARG=30
            IF ((IAUSFL(IARG+1).NE.0)) THEN
              CALL AUSGAB(IARG)
            END IF
            GOTO 9000
          END IF
        END IF
        IF((rng_seed .GT. 128))call ranmar_get
        RNNO36 = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        GBR1=GBR11(LGLE,MEDIUM)*GLE+GBR10(LGLE,MEDIUM)
        IF (((RNNO36.LE.GBR1).AND.(E(NP).GT.RMT2) )) THEN
          IARG=15
          IF ((IAUSFL(IARG+1).NE.0)) THEN
            CALL AUSGAB(IARG)
          END IF
          CALL PAIR
          IARG=16
          IF ((IAUSFL(IARG+1).NE.0)) THEN
            CALL AUSGAB(IARG)
          END IF
          IF (( iq(np) .NE. 0 )) THEN
            GO TO 9002
          ELSE
            goto 9040
          END IF
        END IF
        GBR2=GBR21(LGLE,MEDIUM)*GLE+GBR20(LGLE,MEDIUM)
        IF ((RNNO36.LT.GBR2)) THEN
          IARG=17
          IF ((IAUSFL(IARG+1).NE.0)) THEN
            CALL AUSGAB(IARG)
          END IF
          CALL COMPT
          IARG=18
          IF ((IAUSFL(IARG+1).NE.0)) THEN
            CALL AUSGAB(IARG)
          END IF
          IF((IQ(NP).NE.0))GO TO 9002
        ELSE
          IARG=19
          IF ((IAUSFL(IARG+1).NE.0)) THEN
            CALL AUSGAB(IARG)
          END IF
          CALL PHOTO
          IF ((NP .EQ. 0 .OR. NP .LT. NPOLD )) THEN
            RETURN
          END IF
          IARG=20
          IF ((IAUSFL(IARG+1).NE.0)) THEN
            CALL AUSGAB(IARG)
          END IF
          IF((IQ(NP) .NE. 0))GO TO 9002
        END IF
9040    PEIG=E(NP)
        EIG=PEIG
        IF((EIG.LT.PCUT(IRL)))GO TO 8990
      GO TO 9001
9002  CONTINUE
      RETURN
8990  IF (( medium .GT. 0 )) THEN
        IF ((EIG.GT.AP(MEDIUM))) THEN
          IDR=1
        ELSE
          IDR=2
        END IF
      ELSE
        IDR=1
      END IF
      EDEP=PEIG
      IARG=IDR
      IF ((IAUSFL(IARG+1).NE.0)) THEN
        CALL AUSGAB(IARG)
      END IF
      IRCODE=2
      NP=NP-1
      RETURN
9010  EDEP=PEIG
      IARG=3
      IF ((IAUSFL(IARG+1).NE.0)) THEN
        CALL AUSGAB(IARG)
      END IF
      IRCODE=2
      NP=NP-1
      RETURN
      END
      SUBROUTINE SHOWER(IQI,EI,XI,YI,ZI,UI,VI,WI,IRI,WTI)
      implicit none
      COMMON/QDEBUG/QDEBUG
      LOGICAL QDEBUG
      COMMON/STACK/ E(50),X(50),Y(50),Z(50),U(50),V(50),W(50),DNEAR(50),
     *WT(50),IQ(50),IR(50),LATCH(50), LATCHI,NP,NPold
      DOUBLE PRECISION E
      real*8 X,Y,Z,  U,V,W,  DNEAR,  WT
      integer*4 IQ,  IR,  LATCH,  LATCHI, NP,  NPold
      COMMON/UPHIOT/THETA,SINTHE,COSTHE,SINPHI, COSPHI,PI,TWOPI,PI5D2
      real*8 THETA,  SINTHE,  COSTHE,  SINPHI,  COSPHI,  PI,TWOPI,PI5D2
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      real*8 EI,  XI,YI,ZI, UI,VI,WI, WTI
      integer*4 IQI,  IRI
      DOUBLE PRECISION DEG,  DPGL,  DEI,  DPI,  DCSTH,  DCOSTH,  PI0MSQ
      real*8 DNEARI,  CSTH
      integer*4 IRCODE
      DATA PI0MSQ/1.8215416D4/
      NP=1
      NPold = NP
      DNEARI=0.0
      IQ(1)=IQI
      E(1)=EI
      U(1)=UI
      V(1)=VI
      W(1)=WI
      X(1)=XI
      Y(1)=YI
      Z(1)=ZI
      IR(1)=IRI
      WT(1)=WTI
      DNEAR(1)=DNEARI
      LATCH(1)=LATCHI
      IF ((IQI .EQ. 2)) THEN
        IF ((EI**2 .LE. PI0MSQ)) THEN
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,'(//a/,a,g15.5,a)') ' Stopped in subroutine SHOWER
     *---PI-ZERO option invoked', ' but the total energy was too small (
     *EI=',EI,' MeV)'
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        END IF
        IF((rng_seed .GT. 128))call ranmar_get
        CSTH = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        DCSTH=CSTH
        DEI=EI
        DPI=DSQRT(DEI*DEI-PI0MSQ)
        DEG=DEI+DPI*DCSTH
        DPGL=DPI+DEI*DCSTH
        DCOSTH=DPGL/DEG
        COSTHE=DCOSTH
        SINTHE=DSQRT(1.D0-DCOSTH*DCOSTH)
        IQ(1)=0
        E(1)=DEG/2.
        CALL UPHI(2,1)
        NP=2
        DEG=DEI-DPI*DCSTH
        DPGL=DPI-DEI*DCSTH
        DCOSTH=DPGL/DEG
        COSTHE=DCOSTH
        SINTHE=-DSQRT(1.D0-DCOSTH*DCOSTH)
        IQ(2)=0
        E(2)=DEG/2.
        CALL UPHI(3,2)
      END IF
9051  CONTINUE
        IF((np .LE. 0))GO TO9052
        IF (( iq(np) .EQ. 0 )) THEN
          call photon(ircode)
        ELSE
          call electr(ircode)
        END IF
      GO TO 9051
9052  CONTINUE
      RETURN
      END
      SUBROUTINE UPHI(IENTRY,LVL)
      implicit none
      COMMON/QDEBUG/QDEBUG
      LOGICAL QDEBUG
      COMMON/EPCONT/EDEP,EDEP_LOCAL,TSTEP,TUSTEP,USTEP,TVSTEP,VSTEP, RHO
     *F,EOLD,ENEW,EKE,ELKE,GLE,E_RANGE, x_final,y_final,z_final, u_final
     *,v_final,w_final, IDISC,IROLD,IRNEW,IAUSFL(35)
      DOUBLE PRECISION EDEP,  EDEP_LOCAL
      real*8 TSTEP,  TUSTEP,  USTEP,  VSTEP,  TVSTEP,  RHOF,  EOLD,  ENE
     *W,  EKE,  ELKE,  GLE,  E_RANGE, x_final,y_final,z_final,  u_final,
     *v_final,w_final
      integer*4 IDISC,  IROLD,  IRNEW,  IAUSFL
      COMMON/STACK/ E(50),X(50),Y(50),Z(50),U(50),V(50),W(50),DNEAR(50),
     *WT(50),IQ(50),IR(50),LATCH(50), LATCHI,NP,NPold
      DOUBLE PRECISION E
      real*8 X,Y,Z,  U,V,W,  DNEAR,  WT
      integer*4 IQ,  IR,  LATCH,  LATCHI, NP,  NPold
      COMMON/UPHIIN/SINC0,SINC1,SIN0(1002),SIN1(1002)
      real*8 SINC0,SINC1,SIN0,SIN1
      COMMON/UPHIOT/THETA,SINTHE,COSTHE,SINPHI, COSPHI,PI,TWOPI,PI5D2
      real*8 THETA,  SINTHE,  COSTHE,  SINPHI,  COSPHI,  PI,TWOPI,PI5D2
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      integer IENTRY,LVL
      real*8 CTHET,  RNNO38,  PHI,  CPHI,  A,B,C,  SINPS2,  SINPSI,  US,
     *VS,  SINDEL,COSDEL
      integer*4 IARG,  LPHI,LTHETA,LCTHET,LCPHI
      real*8 xphi,xphi2,yphi,yphi2,rhophi2
      save CTHET,PHI,CPHI,A,B,C,SINPS2,SINPSI,US,VS,SINDEL,COSDEL
      IARG=21
      IF ((IAUSFL(IARG+1).NE.0)) THEN
        CALL AUSGAB(IARG)
      END IF
      GO TO (9060,9070,9080),IENTRY
      GO TO 9090
9060  CONTINUE
      SINTHE=sin(THETA)
      CTHET=PI5D2-THETA
      COSTHE=sin(CTHET)
9070  CONTINUE
9101  CONTINUE
        IF((rng_seed .GT. 128))call ranmar_get
        xphi = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        xphi = 2*xphi - 1
        xphi2 = xphi*xphi
        IF((rng_seed .GT. 128))call ranmar_get
        yphi = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        yphi2 = yphi*yphi
        rhophi2 = xphi2 + yphi2
        IF(rhophi2.LE.1)GO TO9102
      GO TO 9101
9102  CONTINUE
      rhophi2 = 1/rhophi2
      cosphi = (xphi2 - yphi2)*rhophi2
      sinphi = 2*xphi*yphi*rhophi2
9080  GO TO (9110,9120,9130),LVL
      GO TO 9090
9110  A=U(NP)
      B=V(NP)
      C=W(NP)
      GO TO 9140
9130  A=U(NP-1)
      B=V(NP-1)
      C=W(NP-1)
9120  X(NP)=X(NP-1)
      Y(NP)=Y(NP-1)
      Z(NP)=Z(NP-1)
      IR(NP)=IR(NP-1)
      WT(NP)=WT(NP-1)
      DNEAR(NP)=DNEAR(NP-1)
      LATCH(NP)=LATCH(NP-1)
9140  SINPS2=A*A+B*B
      IF ((SINPS2.LT.1.0E-20)) THEN
        U(NP)=SINTHE*COSPHI
        V(NP)=SINTHE*SINPHI
        W(NP)=C*COSTHE
      ELSE
        SINPSI=SQRT(SINPS2)
        US=SINTHE*COSPHI
        VS=SINTHE*SINPHI
        SINDEL=B/SINPSI
        COSDEL=A/SINPSI
        U(NP)=C*COSDEL*US-SINDEL*VS+A*COSTHE
        V(NP)=C*SINDEL*US+COSDEL*VS+B*COSTHE
        W(NP)=-SINPSI*US+C*COSTHE
      END IF
      IARG=22
      IF ((IAUSFL(IARG+1).NE.0)) THEN
        CALL AUSGAB(IARG)
      END IF
      RETURN
9090  write(i_log,'(/a)') '***************** Error: '
      write(i_log,'(a,2i6)') ' STOPPED IN UPHI WITH IENTRY,LVL=',IENTRY,
     *LVL
      write(i_log,'(/a)') '***************** Quiting now.'
      call exit(1)
      END
      subroutine init_nist_brems
      implicit none
      real*8 energy_array(57),x_array(54), cs_array(57,54,100)
      real*8 xi_array(54)
      real*8 x_gauss(64),w_gauss(64)
      integer*4 nmix,kmix,i,n,k,j,ii
      integer*4 ngauss,i_gauss
      integer*4 lnblnk1,egs_get_unit
      integer*4 ifirst,ilast,nener,neke,leil
      real*8 cs(57,54),ee(57),ele(57)
      real*8 csx(54),afx(54),bfx(54),cfx(54),dfx(54)
      real*8 cse(57),afe(57),bfe(57),cfe(57),dfe(57)
      real*8 Z,sumA
      real*8 emin,xi,res,spline,eil,ei,beta2,aux,sigb,sigt,ebr1,ebr2
      real*8 sigee,sigep,sige,si_esig,si1_esig,si_ebr1,si1_ebr1,ededx, s
     *ig_bhabha,si_psig,si1_psig,si_pbr1,si1_pbr1,si_pbr2,si1_pbr2
      integer*4 iz
      real*8 ple,qle,x,f,error,max_error,x_max_error,f_max_error
      integer*4 ndat,k_max_error
      character tmp_string*512, tmp1_string*512
      integer itmp
      real*8 amu
      parameter (amu = 1660.5655)
      logical ex,is_opened
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      COMMON/BREMPR/ DL1(8,1),DL2(8,1),DL3(8,1),DL4(8,1),DL5(8,1),DL6(8,
     *1), ALPHI(2,1),BPAR(2,1),DELPOS(2,1), WA(1,50),PZ(1,50),ZELEM(1,50
     *),RHOZ(1,50), PWR2I(50), DELCM(1),ZBRANG(1),LZBRANG(1),NNE(1), IBR
     *DST,IPRDST,ibr_nist,pair_nrc,itriplet, ASYM(1,50,2)
      CHARACTER*4 ASYM
      real*8 DL1,DL2,DL3,DL4,DL5,DL6,   ALPHI,  BPAR,  DELPOS,  WA,  PZ,
     *  ZELEM,  RHOZ,  PWR2I,  DELCM,  ZBRANG,  LZBRANG
      integer*4 NNE,  IBRDST,  IPRDST,  ibr_nist,  itriplet,  pair_nrc
      COMMON/ELECIN/ esig_e(1),psig_e(1), esige_max, psige_max, range_ep
     *(0:1,500,1), E_array(500,1), etae_ms0(500,1),etae_ms1(500,1),etap_
     *ms0(500,1),etap_ms1(500,1),q1ce_ms0(500,1),q1ce_ms1(500,1),q1cp_ms
     *0(500,1),q1cp_ms1(500,1),q2ce_ms0(500,1),q2ce_ms1(500,1),q2cp_ms0(
     *500,1),q2cp_ms1(500,1),blcce0(500,1),blcce1(500,1), EKE0(1),EKE1(1
     *), XR0(1),TEFF0(1),BLCC(1),XCC(1), ESIG0(500,1),ESIG1(500,1),PSIG0
     *(500,1),PSIG1(500,1),EDEDX0(500,1),EDEDX1(500,1),PDEDX0(500,1),PDE
     *DX1(500,1),EBR10(500,1),EBR11(500,1),PBR10(500,1),PBR11(500,1),PBR
     *20(500,1),PBR21(500,1),TMXS0(500,1),TMXS1(500,1), expeke1(1), IUNR
     *ST(1),EPSTFL(1),IAPRIM(1), sig_ismonotone(0:1,1)
      real*8 esig_e,   psig_e,   esige_max,  psige_max,  range_ep,  E_ar
     *ray,  etae_ms0,etae_ms1,  etap_ms0,etap_ms1,  q1ce_ms0,q1ce_ms1,
     *q1cp_ms0,q1cp_ms1,  q2ce_ms0,q2ce_ms1,  q2cp_ms0,q2cp_ms1,  blcce0
     *,blcce1,   expeke1,  EKE0,EKE1, XR0,  TEFF0,  BLCC,  XCC,  ESIG0,E
     *SIG1,  PSIG0,PSIG1,  EDEDX0,EDEDX1,  PDEDX0,PDEDX1,  EBR10,EBR11,
     * PBR10,PBR11,  PBR20,PBR21,  TMXS0,TMXS1
      integer*4 IUNRST,  EPSTFL,  IAPRIM
      logical sig_ismonotone
      COMMON/THRESH/RMT2,RMSQ, AP(1),AE(1),UP(1),UE(1),TE(1),THMOLL(1)
      real*8 RMT2,  RMSQ,  AP,  AE,  UP,  UE,  TE,  THMOLL
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      common/nist_brems/ nb_fdata(0:50,100,1), nb_xdata(0:50,100,1), nb_
     *wdata(50,100,1), nb_idata(50,100,1), nb_emin(1),nb_emax(1), nb_lem
     *in(1),nb_lemax(1), nb_dle(1),nb_dlei(1), log_ap(1)
      real*8 nb_fdata,nb_xdata,nb_wdata,nb_emin,nb_emax,nb_lemin,nb_lema
     *x, nb_dle,nb_dlei,log_ap
      integer*4 nb_idata
      common/spin_data/ spin_rej(1,0:1,0: 31,0:15,0:31), espin_min,espin
     *_max,espml,b2spin_min,b2spin_max, dbeta2,dbeta2i,dlener,dleneri,dq
     *q1,dqq1i, fool_intel_optimizer
      real*4 spin_rej,espin_min,espin_max,espml,b2spin_min,b2spin_max, d
     *beta2,dbeta2i,dlener,dleneri,dqq1,dqq1i
      logical fool_intel_optimizer
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      DO 9151 i=1,len(tmp_string)
        tmp_string(i:i) = ' '
9151  CONTINUE
9152  CONTINUE
      tmp_string = hen_house(:lnblnk1(hen_house)) // 'data' // '/'
      IF (( ibr_nist .EQ. 1 )) THEN
        DO 9161 i=1,len(tmp1_string)
          tmp1_string(i:i) = ' '
9161    CONTINUE
9162    CONTINUE
        tmp1_string = tmp_string(:lnblnk1(tmp_string)) // 'nist_brems.da
     *ta'
        inquire(file=tmp1_string,exist=ex,opened=is_opened,number=itmp)
        IF (( .NOT.ex )) THEN
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,*) 'EGSnrc data file ','nist_brems.data',' does no
     *t exist'
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        END IF
        IF (( .NOT.is_opened )) THEN
          i_nist_data=egs_get_unit(i_nist_data)
          IF ((i_nist_data.LT.0)) THEN
            write(i_log,'(/a)') '***************** Error: '
            write(i_log,*) 'failed to get a free Fortran I/O unit for da
     *ta file ', tmp1_string(:lnblnk1(tmp1_string))
            write(i_log,'(/a)') '***************** Quiting now.'
            call exit(1)
          END IF
          open(i_nist_data,file=tmp1_string,status='old',err=4310)
        ELSE
          i_nist_data = itmp
        END IF
      ELSE IF((ibr_nist .EQ. 2)) THEN
        DO 9171 i=1,len(tmp1_string)
          tmp1_string(i:i) = ' '
9171    CONTINUE
9172    CONTINUE
        tmp1_string = tmp_string(:lnblnk1(tmp_string)) // 'nrc_brems.dat
     *a'
        inquire(file=tmp1_string,exist=ex,opened=is_opened,number=itmp)
        IF (( .NOT.ex )) THEN
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,*) 'EGSnrc data file ','nrc_brems.data',' does not
     * exist'
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        END IF
        IF (( .NOT.is_opened )) THEN
          i_nist_data=egs_get_unit(i_nist_data)
          IF ((i_nist_data.LT.0)) THEN
            write(i_log,'(/a)') '***************** Error: '
            write(i_log,*) 'failed to get a free Fortran I/O unit for da
     *ta file ', tmp1_string(:lnblnk1(tmp1_string))
            write(i_log,'(/a)') '***************** Quiting now.'
            call exit(1)
          END IF
          open(i_nist_data,file=tmp1_string,status='old',err=4310)
        ELSE
          i_nist_data = itmp
        END IF
      ELSE
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) ' init_nist_brems: unknown value of ibr_nist!
     *                  ibr_nist = ', ibr_nist
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      rewind(i_nist_data)
      read(i_nist_data,*)
      read(i_nist_data,*) nmix,kmix
      IF ((kmix .GT. 54)) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) ' init_nist_brems: to many k values in data file!
     *'
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      IF ((nmix .GT. 57)) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) ' init_nist_brems: to many T values in data file!
     *'
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      read(i_nist_data,*) (energy_array(n),n=1,nmix)
      DO 9181 n=1,nmix
        energy_array(n) = 1.0*energy_array(n)
9181  CONTINUE
9182  CONTINUE
      read(i_nist_data,*) (x_array(k),k=1,kmix)
      read(i_nist_data,*)
      DO 9191 i=1,100
        read(i_nist_data,*) ((cs_array(n,k,i),n=1,nmix),k=1,kmix)
9191  CONTINUE
9192  CONTINUE
      close(i_nist_data)
      DO 9201 k=1,kmix
        xi_array(k)=Log(1-x_array(k)+1e-6)
        IF (( fool_intel_optimizer )) THEN
          write(i_log,*) 'xi_array(k): ',xi_array(k)
        END IF
9201  CONTINUE
9202  CONTINUE
      ngauss = 64
      call gauss_legendre(0d0,1d0,x_gauss,w_gauss,ngauss)
      write(i_log,*) ' '
      IF ((ibr_nist .EQ. 1)) THEN
        write(i_log,*) 'Using NIST brems cross sections! '
      ELSE IF((ibr_nist .EQ. 2)) THEN
        write(i_log,*) 'Using NRC brems cross sections! '
      END IF
      write(i_log,*) ' '
      DO 9211 medium=1,nmed
        log_ap(medium) = log(ap(medium))
        write(i_log,*) ' Initializing brems data for medium ',medium,'..
     *.'
        emin = max(ae(medium) - rm, ap(medium))
        DO 9221 i=1,nmix
          IF((energy_array(i) .GE. emin))GO TO9222
9221    CONTINUE
9222    CONTINUE
        ifirst = i
        DO 9231 i=nmix,1,-1
          IF((energy_array(i) .LT. ue(medium) - rm))GO TO9232
9231    CONTINUE
9232    CONTINUE
        ilast = i+1
        IF (( ifirst .LT. 1 .OR. ilast .GT. nmix )) THEN
          write(i_log,*) ' init_nist_brems: data available only for '
          write(i_log,*) energy_array(1),' <= E <= ',energy_array(nmix)
          write(i_log,*) ' will use spline interpolations to get cross '
          write(i_log,*) ' sections beyond the available data but this m
     *ay'
          write(i_log,*) ' produce nonsense!'
          IF((ifirst .LT. 1))ifirst=1
          IF((ilast .GT. nmix))ilast = nmix
        END IF
        DO 9241 i=ifirst,ilast
          ii = i+1 - ifirst
          ee(ii) = energy_array(i)
          ele(ii) = log(ee(ii))
          sumA = 0
          DO 9251 j=1,NNE(medium)
            sumA = sumA + pz(medium,j)*wa(medium,j)
9251      CONTINUE
9252      CONTINUE
          sumA = sumA*amu
          DO 9261 k=1,kmix
            cs(ii,k) = 0
            DO 9271 j=1,NNE(medium)
              Z = zelem(medium,j)
              iz = int(Z+0.1)
              Z = Z*Z/sumA
              cs(ii,k) = cs(ii,k) + pz(medium,j)*Z*cs_array(i,k,iz)
9271        CONTINUE
9272        CONTINUE
            csx(k) = Log(cs(ii,k))
9261      CONTINUE
9262      CONTINUE
          call set_spline(xi_array,csx,afx,bfx,cfx,dfx,kmix)
          cse(ii) = 0
          aux = Log(ee(ii)/ap(medium))
          DO 9281 i_gauss=1,ngauss
            xi = log(1 - ap(medium)/ee(ii)*exp(x_gauss(i_gauss)*aux)+1e-
     *      6)
            res = spline(xi,xi_array,afx,bfx,cfx,dfx,kmix)
            cse(ii) = cse(ii) + w_gauss(i_gauss)*exp(res)
9281      CONTINUE
9282      CONTINUE
9241    CONTINUE
9242    CONTINUE
        nener = ilast - ifirst + 1
        call set_spline(ele,cse,afe,bfe,cfe,dfe,nener)
        neke = meke(medium)
        sigee = 1E-15
        sigep = 1E-15
        DO 9291 i=1,neke
          eil = (float(i) - eke0(medium))/eke1(medium)
          ei = exp(eil)
          leil = i
          beta2 = ei*(ei+2*rm)/(ei+rm)**2
          IF (( ei .LE. ap(medium) )) THEN
            sigb = 1e-30
          ELSE
            sigb = spline(eil,ele,afe,bfe,cfe,dfe,nener)
            sigb = sigb*log(ei/ap(medium))/beta2*rho(medium)
          END IF
          sigt=esig1(Leil,MEDIUM)*eil+esig0(Leil,MEDIUM)
          ebr1=ebr11(Leil,MEDIUM)*eil+ebr10(Leil,MEDIUM)
          IF((sigt .LT. 0))sigt = 0
          IF((ebr1 .GT. 1))ebr1 = 1
          IF((ebr1 .LT. 0))ebr1 = 0
          IF (( i .GT. 1 )) THEN
            si_esig = si1_esig
            si_ebr1 = si1_ebr1
            si1_esig = sigt*(1 - ebr1) + sigb
            si1_ebr1 = sigb/si1_esig
            esig1(i-1,medium) = (si1_esig - si_esig)*eke1(medium)
            esig0(i-1,medium) = si1_esig - esig1(i-1,medium)*eil
            ebr11(i-1,medium) = (si1_ebr1 - si_ebr1)*eke1(medium)
            ebr10(i-1,medium) = si1_ebr1 - ebr11(i-1,medium)*eil
          ELSE
            si1_esig = sigt*(1 - ebr1) + sigb
            si1_ebr1 = sigb/si1_esig
          END IF
          sigt=psig1(Leil,MEDIUM)*eil+psig0(Leil,MEDIUM)
          ebr1=pbr11(Leil,MEDIUM)*eil+pbr10(Leil,MEDIUM)
          ebr2=pbr21(Leil,MEDIUM)*eil+pbr20(Leil,MEDIUM)
          IF((sigt .LT. 0))sigt = 0
          IF((ebr1 .GT. 1))ebr1 = 1
          IF((ebr1 .LT. 0))ebr1 = 0
          IF((ebr2 .GT. 1))ebr2 = 1
          IF((ebr2 .LT. 0))ebr2 = 0
          sig_bhabha = sigt*(ebr2 - ebr1)
          IF((sig_bhabha .LT. 0))sig_bhabha = 0
          IF (( i .GT. 1 )) THEN
            si_psig = si1_psig
            si_pbr1 = si1_pbr1
            si_pbr2 = si1_pbr2
            si1_psig = sigt*(1 - ebr1) + sigb
            si1_pbr1 = sigb/si1_psig
            si1_pbr2 = (sigb + sig_bhabha)/si1_psig
            psig1(i-1,medium) = (si1_psig - si_psig)*eke1(medium)
            psig0(i-1,medium) = si1_psig - psig1(i-1,medium)*eil
            pbr11(i-1,medium) = (si1_pbr1 - si_pbr1)*eke1(medium)
            pbr10(i-1,medium) = si1_pbr1 - pbr11(i-1,medium)*eil
            pbr21(i-1,medium) = (si1_pbr2 - si_pbr2)*eke1(medium)
            pbr20(i-1,medium) = si1_pbr2 - pbr21(i-1,medium)*eil
          ELSE
            si1_psig = sigt*(1 - ebr1) + sigb
            si1_pbr1 = sigb/si1_psig
            si1_pbr2 = (sigb + sig_bhabha)/si1_psig
          END IF
          ededx=ededx1(Leil,MEDIUM)*eil+ededx0(Leil,MEDIUM)
          sige = si1_esig/ededx
          IF((sige .GT. sigee))sigee = sige
          ededx=pdedx1(Leil,MEDIUM)*eil+pdedx0(Leil,MEDIUM)
          sige = si1_psig/ededx
          IF((sige .GT. sigep))sigep = sige
9291    CONTINUE
9292    CONTINUE
        esig1(neke,medium) = esig1(neke-1,medium)
        esig0(neke,medium) = esig0(neke-1,medium)
        ebr11(neke,medium) = ebr11(neke-1,medium)
        ebr10(neke,medium) = ebr10(neke-1,medium)
        psig1(neke,medium) = psig1(neke-1,medium)
        psig0(neke,medium) = psig0(neke-1,medium)
        pbr11(neke,medium) = pbr11(neke-1,medium)
        pbr10(neke,medium) = pbr10(neke-1,medium)
        pbr21(neke,medium) = pbr21(neke-1,medium)
        pbr20(neke,medium) = pbr20(neke-1,medium)
        write(i_log,*) ' Max. new cross sections per energy loss: ',sige
     *  e,sigep
        esig_e(medium) = sigee
        psig_e(medium) = sigep
        IF((sigee .GT. esige_max))esige_max = sigee
        IF((sigep .GT. psige_max))psige_max = sigep
        nb_emin(medium) = energy_array(ifirst)
        IF (( nb_emin(medium) .LE. ap(medium) )) THEN
          nb_emin(medium) = energy_array(ifirst+1)
        END IF
        nb_emax(medium) = energy_array(ilast)
        nb_lemin(medium) = log(nb_emin(medium))
        nb_lemax(medium) = log(nb_emax(medium))
        nb_dle(medium) = (nb_lemax(medium) - nb_lemin(medium))/(100-1)
        nb_dlei(medium) = 1/nb_dle(medium)
        eil = nb_lemin(medium) - nb_dle(medium)
        DO 9301 i=1,100
          eil = eil + nb_dle(medium)
          ei = exp(eil)
          DO 9311 ii=1,nener
            IF((ei .LT. ee(ii)))GO TO9312
9311      CONTINUE
9312      CONTINUE
          ii = ii-1
          IF((ii .LT. 1))ii = 1
          IF((ii .GT. nener-1))ii = nener-1
          ple = (eil - ele(ii))/(ele(ii+1)-ele(ii))
          qle = 1 - ple
          DO 9321 k=1,kmix
            csx(k) = log(qle*cs(ii,k) + ple*cs(ii+1,k))
9321      CONTINUE
9322      CONTINUE
          call set_spline(xi_array,csx,afx,bfx,cfx,dfx,kmix)
          x = ap(medium)/ei
          aux = -log(x)
          xi = log(1 - x+1e-6)
          res = spline(xi,xi_array,afx,bfx,cfx,dfx,kmix)
          nb_xdata(0,i,medium) = 0
          nb_fdata(0,i,medium) = exp(res)
          DO 9331 k=1,kmix
            IF((x_array(k) .GT. x))GO TO9332
9331      CONTINUE
9332      CONTINUE
          IF((k .GT. kmix))k = kmix
          ndat = 0
          DO 9341 j=k+1,kmix-1
            ndat = ndat+1
            nb_xdata(ndat,i,medium) = log(x_array(j)/x)/aux
            nb_fdata(ndat,i,medium) = exp(csx(j))
            IF (( fool_intel_optimizer )) THEN
              write(i_log,*) 'nb_xdata(ndat,i,medium): ', nb_xdata(ndat,
     *        i,medium)
            END IF
9341      CONTINUE
9342      CONTINUE
          ndat = ndat+1
          nb_xdata(ndat,i,medium) = 1
          nb_fdata(ndat,i,medium) = exp(csx(kmix))
          IF((ndat .GE. 50))goto 9350
9361      CONTINUE
            x_max_error = 0
            f_max_error = 0
            k_max_error = 0
            max_error = 0
            DO 9371 k=0,ndat-1
              x = 0.5*(nb_xdata(k,i,medium) + nb_xdata(k+1,i,medium))
              f = 0.5*(nb_fdata(k,i,medium) + nb_fdata(k+1,i,medium))
              xi = log(1 - ap(medium)/ei*exp(x*aux)+1e-6)
              res = spline(xi,xi_array,afx,bfx,cfx,dfx,kmix)
              res = exp(res)
              error = abs(1-f/res)
              IF (( error .GT. max_error )) THEN
                x_max_error = x
                f_max_error = res
                max_error = error
                k_max_error = k
              END IF
9371        CONTINUE
9372        CONTINUE
            ndat = ndat+1
            DO 9381 k=ndat,k_max_error+2,-1
              nb_xdata(k,i,medium) = nb_xdata(k-1,i,medium)
              nb_fdata(k,i,medium) = nb_fdata(k-1,i,medium)
9381        CONTINUE
9382        CONTINUE
            nb_xdata(k_max_error+1,i,medium) = x_max_error
            nb_fdata(k_max_error+1,i,medium) = f_max_error
            IF(((ndat .EQ. 50)))GO TO9362
          GO TO 9361
9362      CONTINUE
9350      call prepare_alias_table(50,nb_xdata(0,i,medium), nb_fdata(0,i
     *    ,medium),nb_wdata(1,i,medium),nb_idata(1,i,medium))
9301    CONTINUE
9302    CONTINUE
9211  CONTINUE
9212  CONTINUE
      write(i_log,*) ' '
      write(i_log,*) ' '
      return
4310  write(i_log,'(/a)') '***************** Error: '
      write(i_log,*) 'failed to open EGSnrc data file ',tmp1_string(:lnb
     *lnk1(tmp1_string))
      write(i_log,'(/a)') '***************** Quiting now.'
      call exit(1)
      return
      end
      subroutine init_nrc_pair
      implicit none
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      COMMON/BREMPR/ DL1(8,1),DL2(8,1),DL3(8,1),DL4(8,1),DL5(8,1),DL6(8,
     *1), ALPHI(2,1),BPAR(2,1),DELPOS(2,1), WA(1,50),PZ(1,50),ZELEM(1,50
     *),RHOZ(1,50), PWR2I(50), DELCM(1),ZBRANG(1),LZBRANG(1),NNE(1), IBR
     *DST,IPRDST,ibr_nist,pair_nrc,itriplet, ASYM(1,50,2)
      CHARACTER*4 ASYM
      real*8 DL1,DL2,DL3,DL4,DL5,DL6,   ALPHI,  BPAR,  DELPOS,  WA,  PZ,
     *  ZELEM,  RHOZ,  PWR2I,  DELCM,  ZBRANG,  LZBRANG
      integer*4 NNE,  IBRDST,  IPRDST,  ibr_nist,  itriplet,  pair_nrc
      COMMON/ELECIN/ esig_e(1),psig_e(1), esige_max, psige_max, range_ep
     *(0:1,500,1), E_array(500,1), etae_ms0(500,1),etae_ms1(500,1),etap_
     *ms0(500,1),etap_ms1(500,1),q1ce_ms0(500,1),q1ce_ms1(500,1),q1cp_ms
     *0(500,1),q1cp_ms1(500,1),q2ce_ms0(500,1),q2ce_ms1(500,1),q2cp_ms0(
     *500,1),q2cp_ms1(500,1),blcce0(500,1),blcce1(500,1), EKE0(1),EKE1(1
     *), XR0(1),TEFF0(1),BLCC(1),XCC(1), ESIG0(500,1),ESIG1(500,1),PSIG0
     *(500,1),PSIG1(500,1),EDEDX0(500,1),EDEDX1(500,1),PDEDX0(500,1),PDE
     *DX1(500,1),EBR10(500,1),EBR11(500,1),PBR10(500,1),PBR11(500,1),PBR
     *20(500,1),PBR21(500,1),TMXS0(500,1),TMXS1(500,1), expeke1(1), IUNR
     *ST(1),EPSTFL(1),IAPRIM(1), sig_ismonotone(0:1,1)
      real*8 esig_e,   psig_e,   esige_max,  psige_max,  range_ep,  E_ar
     *ray,  etae_ms0,etae_ms1,  etap_ms0,etap_ms1,  q1ce_ms0,q1ce_ms1,
     *q1cp_ms0,q1cp_ms1,  q2ce_ms0,q2ce_ms1,  q2cp_ms0,q2cp_ms1,  blcce0
     *,blcce1,   expeke1,  EKE0,EKE1, XR0,  TEFF0,  BLCC,  XCC,  ESIG0,E
     *SIG1,  PSIG0,PSIG1,  EDEDX0,EDEDX1,  PDEDX0,PDEDX1,  EBR10,EBR11,
     * PBR10,PBR11,  PBR20,PBR21,  TMXS0,TMXS1
      integer*4 IUNRST,  EPSTFL,  IAPRIM
      logical sig_ismonotone
      common/nrc_pair/ nrcp_fdata(65,84,1), nrcp_wdata(65,84,1), nrcp_id
     *ata(65,84,1), nrcp_xdata(65), nrcp_emin, nrcp_emax, nrcp_dle, nrcp
     *_dlei
      real*8 nrcp_fdata,nrcp_wdata,nrcp_xdata, nrcp_emin, nrcp_emax, nrc
     *p_dle, nrcp_dlei
      integer*4 nrcp_idata
      COMMON/THRESH/RMT2,RMSQ, AP(1),AE(1),UP(1),UE(1),TE(1),THMOLL(1)
      real*8 RMT2,  RMSQ,  AP,  AE,  UP,  UE,  TE,  THMOLL
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      character nrcp_file*256, endianess*4
      integer egs_get_unit
      integer*4 nrcp_unit, want_nrcp_unit, rec_length
      integer*4 i, lnblnk1
      real*8 tmp, ddx, xx, Z
      real*4 emin, emax
      integer*4 ne, nb, ix, ie, irec, i_ele, nbb, iz
      character endian, cdum( 243)
      logical swap
      real*4 tmp_4, tarray(65)
      integer*4 itmp_4
      character c_4(4), ic_4(4)
      equivalence (tmp_4,c_4), (itmp_4, ic_4)
      DO 9391 i=1,len(nrcp_file)
        nrcp_file(i:i) = ' '
9391  CONTINUE
9392  CONTINUE
      nrcp_file = hen_house(:lnblnk1(hen_house)) // 'data' // '/' // 'pa
     *ir_nrc1.data'
      want_nrcp_unit = 62
      nrcp_unit = egs_get_unit(want_nrcp_unit)
      IF (( nrcp_unit .LT. 1 )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) 'init_nrc_pair: failed to get a free fortran unit
     *'
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      rec_length = 65*4
      open(nrcp_unit,file=nrcp_file,form='unformatted',access='direct',
     *status='old',recl=rec_length,err=9400)
      read(nrcp_unit,rec=1,err=9410) emin, emax, ne, nb, endian, cdum
      IF (( ichar(endian) .EQ. 0 )) THEN
        endianess = '1234'
      ELSE
        endianess = '4321'
      END IF
      swap = endianess.ne.'1234'
      IF (( swap )) THEN
        tmp_4 = emin
        call egs_swap_4(c_4)
        emin = tmp_4
        tmp_4 = emax
        call egs_swap_4(c_4)
        emax = tmp_4
        itmp_4 = ne
        call egs_swap_4(ic_4)
        ne = itmp_4
        itmp_4 = nb
        call egs_swap_4(ic_4)
        nb = itmp_4
      END IF
      write(i_log,'(//a,a)') 'Reading NRC pair data base from ',nrcp_fil
     *e(:lnblnk1(nrcp_file))
      write(i_log,'(a,a,a)') 'Data generated on a machine with ',endiane
     *ss,' endianess'
      write(i_log,'(a,a)') 'The endianess of this CPU is ','1234'
      IF (( swap )) THEN
        write(i_log,'(a)') '=> will need to do byte swaping'
      END IF
      write(i_log,'(a,2f9.3)') 'Energy range of the data: ',emin,emax
      IF (( nb .NE. 65 )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) 'Inconsistent x-grid size'
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      IF (( ne .NE. 84 )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) 'Inconsistent energy grid size'
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      nrcp_emin = emin
      nrcp_emax = emax
      nrcp_dle = log((emax-2)/(emin-2))/(ne-1)
      nrcp_dlei = 1/nrcp_dle
      nbb = nb/2
      ddx = sqrt(0.5)/nbb
      DO 9421 ix=0,nbb
        xx = ddx*ix
        nrcp_xdata(ix+1) = xx*xx
9421  CONTINUE
9422  CONTINUE
      do ix=nbb-1,0,-1
        xx = ddx*ix
        nrcp_xdata(nb-ix) = 1 - xx*xx
      end do
      DO 9441 medium=1,NMED
        write(i_log,'(a,i4,a,$)') '  medium ',medium,' .................
     *.... '
        DO 9451 ie=1,84
          DO 9461 ix=1,65
            nrcp_fdata(ix,ie,medium) = 0
9461      CONTINUE
9462      CONTINUE
9451    CONTINUE
9452    CONTINUE
        DO 9471 i_ele=1,NNE(medium)
          Z = ZELEM(medium,i_ele)
          iz = int(Z+0.5)
          tmp = PZ(medium,i_ele)*Z*Z
          irec = (iz-1)*ne + 2
          DO 9481 ie=1,84
            read(nrcp_unit,rec=irec,err=9410) tarray
            DO 9491 ix=1,65
              tmp_4 = tarray(ix)
              IF (( swap )) THEN
                call egs_swap_4(c_4)
              END IF
              nrcp_fdata(ix,ie,medium)=nrcp_fdata(ix,ie,medium)+tmp*tmp_
     *        4
9491        CONTINUE
9492        CONTINUE
            irec = irec + 1
9481      CONTINUE
9482      CONTINUE
9471    CONTINUE
9472    CONTINUE
        DO 9501 ie=1,84
          call prepare_alias_table(nb-1,nrcp_xdata,nrcp_fdata(1,ie,mediu
     *    m), nrcp_wdata(1,ie,medium),nrcp_idata(1,ie,medium))
9501    CONTINUE
9502    CONTINUE
        write(i_log,'(a)') ' done'
9441  CONTINUE
9442  CONTINUE
      write(i_log,*) ' '
      close(nrcp_unit)
      return
9400  CONTINUE
      write(i_log,'(/a)') '***************** Error: '
      write(i_log,*) 'Failed to open NRC pair data file'
      write(i_log,'(/a)') '***************** Quiting now.'
      call exit(1)
9410  CONTINUE
      write(i_log,'(/a)') '***************** Error: '
      write(i_log,*) 'I/O error while reading NRC pair data file'
      write(i_log,'(/a)') '***************** Quiting now.'
      call exit(1)
      end
      subroutine vmc_electron(ircode)
      implicit none
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      integer*4 ircode
      write(i_log,'(/a)') '***************** Error: '
      write(i_log,'(//a//)') ' ********* VMC Transport option not in thi
     *s distribution ****** '
      write(i_log,'(/a)') '***************** Quiting now.'
      call exit(1)
      end
      subroutine egs_init_default_rng
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      ixx=0
      jxx=0
      call init_ranmar
      return
      end
      subroutine egs_init_rng(arg1,arg2)
      integer*4 arg1,arg2
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      ixx = arg1
      jxx = arg2
      call init_ranmar
      return
      end
      subroutine egs_get_rndm(ran)
      real*8 ran
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      IF((rng_seed .GT. 128))call ranmar_get
      ran = rng_array(rng_seed)*twom24
      rng_seed = rng_seed + 1
      return
      end
      subroutine egs_get_rndm_array(n,rarray)
      integer*4 n
      real*8 rarray(*)
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      real*8 rtmp
      integer*4 i
      IF((n .LT. 1))return
      DO 9511 i=1,n
        IF((rng_seed .GT. 128))call ranmar_get
        rtmp = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        rarray(i) = rtmp
9511  CONTINUE
9512  CONTINUE
      return
      end
      subroutine eii_init
      implicit none
      COMMON/BREMPR/ DL1(8,1),DL2(8,1),DL3(8,1),DL4(8,1),DL5(8,1),DL6(8,
     *1), ALPHI(2,1),BPAR(2,1),DELPOS(2,1), WA(1,50),PZ(1,50),ZELEM(1,50
     *),RHOZ(1,50), PWR2I(50), DELCM(1),ZBRANG(1),LZBRANG(1),NNE(1), IBR
     *DST,IPRDST,ibr_nist,pair_nrc,itriplet, ASYM(1,50,2)
      CHARACTER*4 ASYM
      real*8 DL1,DL2,DL3,DL4,DL5,DL6,   ALPHI,  BPAR,  DELPOS,  WA,  PZ,
     *  ZELEM,  RHOZ,  PWR2I,  DELCM,  ZBRANG,  LZBRANG
      integer*4 NNE,  IBRDST,  IPRDST,  ibr_nist,  itriplet,  pair_nrc
      COMMON/EDGE/binding_energies(30,100), interaction_prob(6,100), rel
     *axation_prob(39,100), edge_energies(16,100), edge_number(100), edg
     *e_a(16,100), edge_b(16,100), edge_c(16,100), edge_d(16,100), IEDGF
     *L(1),IPHTER(1)
      real*8 binding_energies,  interaction_prob,    relaxation_prob,  e
     *dge_energies,  edge_a,edge_b,edge_c,edge_d
      integer*2 IEDGFL,  IPHTER
      integer*4 edge_number
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      common/eii_data/ eii_xsection_a( 10000),  eii_xsection_b( 10000),
     * eii_cons(1), eii_a(40),  eii_b(40),  eii_L_factor,  eii_z(40),  e
     *ii_sh(40),  eii_nshells(100),  eii_nsh(1),  eii_first(1,50),  eii_
     *no(1,50),  eii_flag
      real*8 eii_xsection_a,eii_xsection_b,eii_a,eii_b,eii_cons,eii_L_fa
     *ctor
      integer*4 eii_z,eii_sh,eii_nshells
      integer*4 eii_first,eii_no
      integer*4 eii_elements,eii_flag,eii_nsh
      COMMON/ELECIN/ esig_e(1),psig_e(1), esige_max, psige_max, range_ep
     *(0:1,500,1), E_array(500,1), etae_ms0(500,1),etae_ms1(500,1),etap_
     *ms0(500,1),etap_ms1(500,1),q1ce_ms0(500,1),q1ce_ms1(500,1),q1cp_ms
     *0(500,1),q1cp_ms1(500,1),q2ce_ms0(500,1),q2ce_ms1(500,1),q2cp_ms0(
     *500,1),q2cp_ms1(500,1),blcce0(500,1),blcce1(500,1), EKE0(1),EKE1(1
     *), XR0(1),TEFF0(1),BLCC(1),XCC(1), ESIG0(500,1),ESIG1(500,1),PSIG0
     *(500,1),PSIG1(500,1),EDEDX0(500,1),EDEDX1(500,1),PDEDX0(500,1),PDE
     *DX1(500,1),EBR10(500,1),EBR11(500,1),PBR10(500,1),PBR11(500,1),PBR
     *20(500,1),PBR21(500,1),TMXS0(500,1),TMXS1(500,1), expeke1(1), IUNR
     *ST(1),EPSTFL(1),IAPRIM(1), sig_ismonotone(0:1,1)
      real*8 esig_e,   psig_e,   esige_max,  psige_max,  range_ep,  E_ar
     *ray,  etae_ms0,etae_ms1,  etap_ms0,etap_ms1,  q1ce_ms0,q1ce_ms1,
     *q1cp_ms0,q1cp_ms1,  q2ce_ms0,q2ce_ms1,  q2cp_ms0,q2cp_ms1,  blcce0
     *,blcce1,   expeke1,  EKE0,EKE1, XR0,  TEFF0,  BLCC,  XCC,  ESIG0,E
     *SIG1,  PSIG0,PSIG1,  EDEDX0,EDEDX1,  PDEDX0,PDEDX1,  EBR10,EBR11,
     * PBR10,PBR11,  PBR20,PBR21,  TMXS0,TMXS1
      integer*4 IUNRST,  EPSTFL,  IAPRIM
      logical sig_ismonotone
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      COMMON/THRESH/RMT2,RMSQ, AP(1),AE(1),UP(1),UE(1),TE(1),THMOLL(1)
      real*8 RMT2,  RMSQ,  AP,  AE,  UP,  UE,  TE,  THMOLL
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      integer*4 imed,iele,ish,nsh,iZ,j,i,itmp,nskip,nbin,ii,nsh_tot,iii,
     *k
      integer*4 jj,jjj
      integer*4 lnblnk1
      integer*4 tmp_array(100)
      integer*4 want_eii_unit,eii_unit,eii_out,egs_open_file
      integer egs_get_unit
      real*8 e_eii_min,emax,fmax,aux_array(250)
      real*8 sigo,loge,tau,beta2,p2,uwm,Wmax
      real*8 ss_0, ss_1, sh_0, sh_1, aux, av_e, con_med, dedx_old, sigm_
     *old
      real*8 dedx,e,sig,sigm,wbrem,sum_a,sum_z,sum_pz,sum_wa,Ec,Ecc
      real*8 sum_sh,sum_occn,U,sum_sigma,sum_dedx
      real*8 sigma,sigma_old,wbrem_old,sig_j,de
      integer*4 lloge
      logical check_it,is_monotone,getd
      real*8 sigma_max
      character eii_file*128
      character*512 toUpper
      integer*4 occn_numbers(4)
      real*8 cons
      parameter (cons = 0.153536)
      data occn_numbers/2,2,2,4/
      DO 9521 j=1,100
        eii_nshells(j) = 0
9521  CONTINUE
9522  CONTINUE
      DO 9531 j=1,1
        eii_nsh(j) = 0
9531  CONTINUE
9532  CONTINUE
      IF (( eii_flag .EQ. 0 )) THEN
        return
      END IF
      getd = .false.
      DO 9541 j=1,1
        IF (( iedgfl(j) .GT. 0 .AND. iedgfl(j) .LE. 100 )) THEN
          getd = .true.
          GO TO9542
        END IF
9541  CONTINUE
9542  CONTINUE
      IF (( .NOT.getd )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,'(/a,/a,/a,/a)') ' In subroutine eii_init: ', '   Sc
     *attering off bound electrons creates atomic vacancies,', '   poten
     *tially starting an atomic relaxation cascade. ', '   Please turn O
     *N atomic relaxations.'
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      e_eii_min = 1e30
      DO 9551 imed=1,nmed
        IF((ae(imed)-rm .LT. e_eii_min))e_eii_min = ae(imed) - rm
        IF((ap(imed) .LT. e_eii_min))e_eii_min = ap(imed)
9551  CONTINUE
9552  CONTINUE
      write(i_log,*) ' '
      write(i_log,*) 'eii_init: minimum threshold energy found: ',e_eii_
     *min
      DO 9561 imed=1,nmed
        DO 9571 iele=1,nne(imed)
          iZ = int(zelem(imed,iele)+0.5)
          IF (( eii_nshells(iZ) .EQ. 0 )) THEN
            nsh = 0
            DO 9581 ish=1,4
              IF((binding_energies(ish,iZ) .GT. e_eii_min))nsh = nsh+1
9581        CONTINUE
9582        CONTINUE
            eii_nshells(iZ) = nsh
          END IF
9571    CONTINUE
9572    CONTINUE
9561  CONTINUE
9562  CONTINUE
      nsh = 0
      DO 9591 iZ=1,100
        nsh = nsh + eii_nshells(iZ)
9591  CONTINUE
9592  CONTINUE
      IF (( nsh .EQ. 0 )) THEN
        write(i_log,*) '*** EII requested but no shells with binding ene
     *rgies '
        write(i_log,*) '    above the specified threshold found'
        write(i_log,*) '    => turning off EII'
        eii_flag = 0
        return
      END IF
      IF (( nsh .GT. 40 )) THEN
        write(i_log,*) '*** Number of shells with binding energies great
     *er than '
        write(i_log,*) '    the specified thresholds is ',nsh
        write(i_log,*) '    This is more than the allocated arrays can h
     *old'
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) '    Increase the macro $MAX_EII_SHELLS and retry
     *'
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      write(i_log,*) 'eii_init: number of shells to simulate EII: ',nsh
      nsh_tot = nsh
      tmp_array(1) = 0
      DO 9601 j=2,100
        tmp_array(j) = tmp_array(j-1) + eii_nshells(j-1)
9601  CONTINUE
9602  CONTINUE
      DO 9611 imed=1,nmed
        nsh = 0
        DO 9621 iele=1,nne(imed)
          iZ = int(zelem(imed,iele)+0.5)
          eii_no(imed,iele) = eii_nshells(iZ)
          nsh = nsh + eii_nshells(iZ)
          IF (( eii_nshells(iZ) .GT. 0 )) THEN
            eii_first(imed,iele) = tmp_array(iZ) + 1
          ELSE
            eii_first(imed,iele) = 0
          END IF
9621    CONTINUE
9622    CONTINUE
        eii_nsh(imed) = nsh
9611  CONTINUE
9612  CONTINUE
      DO 9631 i=1,len(eii_file)
        eii_file(i:i) = ' '
9631  CONTINUE
9632  CONTINUE
      eii_file = hen_house(:lnblnk1(hen_house)) // 'data' // '/' // 'eii
     *_'// eii_xfile(:lnblnk1(eii_xfile)) //'.data'
      want_eii_unit = 62
      eii_unit = egs_get_unit(want_eii_unit)
      IF (( eii_unit .LT. 1 )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) 'eii_init: failed to get a free Fortran I/O unit'
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      open(eii_unit,file=eii_file(:lnblnk1(eii_file)),status='old',err=9
     *640)
      write(i_log,'(//a,a)') 'Opened EII data file ',eii_file(:lnblnk1(e
     *ii_file))
      write(i_log,'(a,$)') ' eii_init: reading EII data ... '
      read(eii_unit,*,err=9650,end=9650) nskip
      DO 9661 j=1,nskip
        read(eii_unit,*,err=9650,end=9650)
9661  CONTINUE
9662  CONTINUE
      read(eii_unit,*,err=9650,end=9650) emax,nbin
      IF (( nbin .NE. 250 )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) 'Inconsistent EII data file'
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      IF ((xsec_out .EQ. 1)) THEN
        eii_out = egs_open_file(93,0,1,'.eiixsec')
      END IF
      ii = 0
      DO 9671 j=1,100
        read(eii_unit,*,err=9650,end=9650) iZ,nsh
        IF ((xsec_out .EQ. 1 .AND. eii_nshells(iZ) .GT. 0)) THEN
          write(eii_out,*) '================================='
          write(eii_out,'(a,i3)') 'EII xsections for element Z = ',iZ
          write(eii_out,*) '================================='
        END IF
        IF (( nsh .LT. eii_nshells(iZ) )) THEN
          write(i_log,*) 'EII data file has data for ',nsh,' shells for
     *element '
          write(i_log,*) iZ,' but according'
          write(i_log,*) 'to binding energies and thresholds ',eii_nshel
     *    ls(iZ)
          write(i_log,*) 'shells are required'
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,*) 'This is a fatal error.'
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        END IF
        DO 9681 ish=1,nsh
          read(eii_unit,*,err=9650,end=9650) fmax
          read(eii_unit,*,err=9650,end=9650) aux_array
          IF ((ish.GT.1 .AND. ish .LT. 5)) THEN
            fmax = fmax*eii_L_factor
          END IF
          IF (( ish .LE. eii_nshells(iZ) )) THEN
            IF ((xsec_out .EQ. 1)) THEN
              IF ((ish .EQ. 1)) THEN
                write(eii_out,'(a,f10.2,a)') 'K-shell sigma_max = ',fmax
     *          ,' b/atom'
              ELSE IF((ish .EQ. 2)) THEN
                write(eii_out,'(a,f9.2,a)') '=> LI-shell sigma_max = ',f
     *          max,' b/atom'
              ELSE IF((ish .EQ. 3)) THEN
                write(eii_out,'(a,f8.2,a)') '=> LII-shell sigma_max = ',
     *          fmax,' b/atom'
              ELSE IF((ish .EQ. 4)) THEN
                write(eii_out,'(a,f8.2,a)') '=> LIII-shell sigma_max = '
     *          ,fmax,' b/atom'
              ELSE
                write(eii_out,*) '=> Wrong number of shells!'
              END IF
              write(eii_out,*) '   E/keV     sigma/(b/atom)'
              write(eii_out,*) '---------------------------'
            END IF
            ii = ii+1
            eii_z(ii) = iZ
            eii_sh(ii) = ish
            eii_a(ii) = nbin
            eii_a(ii) = eii_a(ii)/log(emax/binding_energies(ish,iZ))
            eii_b(ii) = 1 - eii_a(ii)*log(binding_energies(ish,iZ))
            DO 9691 k=1,nbin
              IF (( k .GT. 1 )) THEN
                sigo = fmax*aux_array(k-1)
              ELSE
                sigo = 0
              END IF
              loge = (k - eii_b(ii))/eii_a(ii)
              iii = nbin*(ii-1)+k
              eii_xsection_a(iii) = (fmax*aux_array(k)-sigo)*eii_a(ii)
              eii_xsection_b(iii) = sigo - eii_xsection_a(iii)*loge
              IF ((xsec_out .EQ. 1)) THEN
                write(eii_out,'(f12.2,2X,10f9.2)') Exp((k+1-eii_b(ii))/e
     *          ii_a(ii))*1000.0,fmax*aux_array(k)
              END IF
9691        CONTINUE
9692        CONTINUE
          END IF
9681    CONTINUE
9682    CONTINUE
        IF (( ii .EQ. nsh_tot )) THEN
          GO TO9672
        END IF
9671  CONTINUE
9672  CONTINUE
      close(eii_unit)
      IF ((xsec_out .EQ. 1)) THEN
        close(eii_out)
      END IF
      write(i_log,*) ' OK '
      write(i_log,*) ' '
      DO 9701 imed=1,nmed
        Ec = ae(imed) - rm
        Ecc = min(Ec,ap(imed))
        sum_z=0
        sum_pz=0
        sum_a=0
        sum_wa=0
        DO 9711 iele=1,nne(imed)
          sum_z = sum_z + pz(imed,iele)*zelem(imed,iele)
          sum_pz = sum_pz + pz(imed,iele)
          sum_wa = sum_wa + rhoz(imed,iele)
          sum_a = sum_a + pz(imed,iele)*wa(imed,iele)
9711    CONTINUE
9712    CONTINUE
        con_med = rho(imed)/1.6605655/sum_a
        eii_cons(imed) = con_med
        IF (( eii_nsh(imed) .GT. 0 )) THEN
          is_monotone = .true.
          sigma_max = 0
          DO 9721 j=1,meke(imed)
            loge = (j - eke0(imed))/eke1(imed)
            e = Exp(loge)
            tau = e/rm
            beta2 = tau*(tau+2)/(tau+1)**2
            p2 = 2*rm*tau*(tau+2)
            lloge = j
            medium = imed
            dedx=ededx1(Lloge,MEDIUM)*loge+ededx0(Lloge,MEDIUM)
            IF (( e .GT. ap(medium) .OR. e .GT. 2*Ec )) THEN
              sig=esig1(Lloge,MEDIUM)*loge+esig0(Lloge,MEDIUM)
            ELSE
              sig = 0
            END IF
            IF (( e .GT. 2*Ec )) THEN
              wbrem=ebr11(Lloge,MEDIUM)*loge+ebr10(Lloge,MEDIUM)
              sigm = sig*(1-wbrem)
            ELSE
              sigm = 0
              wbrem = 1
            END IF
            sum_occn=0
            sum_sigma=0
            sum_dedx=0
            DO 9731 iele=1,nne(imed)
              iZ = int(zelem(imed,iele)+0.5)
              sum_sh = 0
              DO 9741 ish=1,eii_no(imed,iele)
                jj = eii_first(imed,iele) + ish - 1
                jjj = eii_sh(jj)
                U = binding_energies(jjj,iZ)
                Wmax = (e+U)/2
                uwm = U/Wmax
                IF (( U .LT. e .AND. U .GT. Ecc )) THEN
                  sum_sh = sum_sh + occn_numbers(jjj)
                  ss_0 = 2*(log(p2/U)-uwm**3*log(p2/Wmax)- (beta2+0.8333
     *            33)*(1-uwm**3))/3/U
                  sh_0 = ((1-uwm)*(1+uwm/(2-uwm))+U*(Wmax-U)/(e+rm)**2 -
     *             (2*tau+1)/(tau+1)**2*uwm/2*log((2-uwm)/uwm))/U
                  ss_1 = log(p2/U)-uwm**2*log(p2/Wmax)- (beta2+1)*(1-uwm
     *            **2)
                  sh_1 = log(Wmax/U/(2-uwm))+2*(Wmax-U)/(2*Wmax-U) +(Wma
     *            x**2-U**2)/(e+rm)**2/2 -(2*tau+1)/(tau+1)**2*log((2*Wm
     *            ax-U)/Wmax)
                  av_E = (ss_1 + sh_1)/(ss_0 + sh_0)
                  i = eii_a(jjj)*loge + eii_b(jjj)
                  i = (jj-1)*250 + i
                  sig_j = eii_xsection_a(i)*loge + eii_xsection_b(i)
                  sig_j = sig_j*pz(imed,iele)*con_med
                  sum_sigma = sum_sigma + sig_j
                  sum_dedx = sum_dedx + sig_j*av_E
                END IF
9741          CONTINUE
9742          CONTINUE
              sum_occn = sum_occn + sum_sh*pz(imed,iele)
9731        CONTINUE
9732        CONTINUE
            sigm = sigm + sum_sigma
            dedx = dedx - sum_dedx
            aux = Ec/e
            IF (( e .GT. 2*Ec )) THEN
              sigo = cons*sum_occn*rho(imed)/(beta2*Ec)*( (1-2*aux)*(1+a
     *        ux/(1-aux)+(tau/(tau+1))**2*aux/2)- (2*tau+1)/(tau+1)**2*a
     *        ux*log((1-aux)/aux))/sum_a
              de = cons*sum_occn*rho(imed)/beta2*( log(0.25/aux/(1-aux))
     *        +(1-2*aux)/(1-aux)+ (tau/(tau+1))**2*(1-4*aux*aux)/8- (2*t
     *        au+1)/(tau+1)**2*log(2*(1-aux)))/sum_a
              sigm = sigm - sigo
              dedx = dedx + de
            END IF
            sigma = sigm + wbrem*sig
            IF((sigma/dedx .GT. sigma_max))sigma_max = sigma/dedx
            IF (( sigma .GT. 0 )) THEN
              wbrem = wbrem*sig/sigma
            ELSE
              wbrem = 1
            END IF
            IF (( j .GT. 1 )) THEN
              ededx1(j-1,imed) = (dedx - dedx_old)*eke1(imed)
              ededx0(j-1,imed) = dedx - ededx1(j-1,imed)*loge
              esig1(j-1,imed) = (sigma - sigma_old)*eke1(imed)
              esig0(j-1,imed) = sigma - esig1(j-1,imed)*loge
              ebr11(j-1,imed) = (wbrem - wbrem_old)*eke1(imed)
              ebr10(j-1,imed) = wbrem - ebr11(j-1,imed)*loge
              IF((sigma/dedx .LT. sigma_old/dedx_old))is_monotone = .fal
     *        se.
            END IF
            dedx_old = dedx
            sigm_old = sigm
            sigma_old = sigma
            wbrem_old = wbrem
9721      CONTINUE
9722      CONTINUE
          ededx1(meke(imed),imed) = ededx1(meke(imed)-1,imed)
          ededx0(meke(imed),imed) = ededx0(meke(imed)-1,imed)
          esig1(meke(imed),imed) = esig1(meke(imed)-1,imed)
          esig0(meke(imed),imed) = esig0(meke(imed)-1,imed)
          ebr11(meke(imed),imed) = ebr11(meke(imed)-1,imed)
          ebr10(meke(imed),imed) = ebr10(meke(imed)-1,imed)
          write(i_log,*) 'eii_init: for medium ',imed,' adjusted sige =
     *', sigma_max,' monotone = ',is_monotone
          sig_ismonotone(0,imed) = is_monotone
          esig_e(imed) = sigma_max
        END IF
9701  CONTINUE
9702  CONTINUE
      return
9650  write(i_log,'(/a)') '***************** Error: '
      write(i_log,*) 'I/O error while reading EII data'
      write(i_log,'(/a)') '***************** Quiting now.'
      call exit(1)
9640  write(i_log,'(/a)') '***************** Error: '
      write(i_log,'(//a,a,/a,/a/)') 'Failed to open EII data file ',eii_
     *file(:lnblnk1(eii_file)), 'Make sure file exists in your $HEN_HOUS
     *E/data directory!', '****BEWARE of case sensitive file names!!!'
      write(i_log,'(/a)') '***************** Quiting now.'
      call exit(1)
      return
      end
      subroutine eii_sample(ish,iZ,Uj)
      implicit none
      integer*4 ish,iZ
      real*8 Uj
      COMMON/EPCONT/EDEP,EDEP_LOCAL,TSTEP,TUSTEP,USTEP,TVSTEP,VSTEP, RHO
     *F,EOLD,ENEW,EKE,ELKE,GLE,E_RANGE, x_final,y_final,z_final, u_final
     *,v_final,w_final, IDISC,IROLD,IRNEW,IAUSFL(35)
      DOUBLE PRECISION EDEP,  EDEP_LOCAL
      real*8 TSTEP,  TUSTEP,  USTEP,  VSTEP,  TVSTEP,  RHOF,  EOLD,  ENE
     *W,  EKE,  ELKE,  GLE,  E_RANGE, x_final,y_final,z_final,  u_final,
     *v_final,w_final
      integer*4 IDISC,  IROLD,  IRNEW,  IAUSFL
      common/eii_data/ eii_xsection_a( 10000),  eii_xsection_b( 10000),
     * eii_cons(1), eii_a(40),  eii_b(40),  eii_L_factor,  eii_z(40),  e
     *ii_sh(40),  eii_nshells(100),  eii_nsh(1),  eii_first(1,50),  eii_
     *no(1,50),  eii_flag
      real*8 eii_xsection_a,eii_xsection_b,eii_a,eii_b,eii_cons,eii_L_fa
     *ctor
      integer*4 eii_z,eii_sh,eii_nshells
      integer*4 eii_first,eii_no
      integer*4 eii_elements,eii_flag,eii_nsh
      common/egs_vr/ e_max_rr(1),  prob_RR,  nbr_split,  i_play_RR,
     * i_survived_RR,
     *     n_RR_warning,                                        i_do_rr(
     *1)
      real*8          e_max_rr,prob_RR
      integer*4       nbr_split,i_play_RR,i_survived_RR,n_RR_warning
      integer*2     i_do_rr
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      COMMON/STACK/ E(50),X(50),Y(50),Z(50),U(50),V(50),W(50),DNEAR(50),
     *WT(50),IQ(50),IR(50),LATCH(50), LATCHI,NP,NPold
      DOUBLE PRECISION E
      real*8 X,Y,Z,  U,V,W,  DNEAR,  WT
      integer*4 IQ,  IR,  LATCH,  LATCHI, NP,  NPold
      COMMON/THRESH/RMT2,RMSQ, AP(1),AE(1),UP(1),UE(1),TE(1),THMOLL(1)
      real*8 RMT2,  RMSQ,  AP,  AE,  UP,  UE,  TE,  THMOLL
      COMMON/UPHIOT/THETA,SINTHE,COSTHE,SINPHI, COSPHI,PI,TWOPI,PI5D2
      real*8 THETA,  SINTHE,  COSTHE,  SINPHI,  COSPHI,  PI,TWOPI,PI5D2
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      common/relax_data/ relax_first(3000),  relax_ntran(3000),  relax_s
     *tate(10000),  relax_prob(10000),  relax_atbin(10000),  relax_ntot
      real*8 relax_prob
      integer*4 relax_first, relax_ntran, relax_state, relax_atbin, rela
     *x_ntot
      real*8 T,tau,tau1,tau12,tau2,p2,beta2,c1,c2,Wmax,xmax,fm_s,fm_h,pr
     *ob_s,prob
      real*8 r1,r2,r3,wx,wxx,aux,frej
      real*8 peie,pese1,pese2,dcosth,h1
      integer*4 iarg
      real*8 eta,cphi,sphi
      integer*4 np_save,ip,j
      real*8 xphi,xphi2,yphi,yphi2,rhophi2
      peie = e(np)
      T = peie - rm
      tau = T/rm
      tau1 = tau+1
      tau12 = tau1*tau1
      tau2 = tau*tau
      p2 = tau2 + 2*tau
      beta2 = p2/tau12
      Wmax = 0.5*(T+Uj)
      xmax = Uj/Wmax
      c1 = (Wmax/peie)**2
      c2 = (2*tau+1)/tau12
      fm_s = log(rmt2*p2/Uj) - beta2 - 0.5
      prob_s = 0.66666667*fm_s*(1+xmax+xmax*xmax)
      fm_h = 2 + c1 - c2
      IF((fm_h .LT. 1))fm_h = 1
      prob = fm_h + prob_s
9751  CONTINUE
        IF((rng_seed .GT. 128))call ranmar_get
        r1 = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        IF((rng_seed .GT. 128))call ranmar_get
        r2 = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        IF((rng_seed .GT. 128))call ranmar_get
        r3 = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
        IF (( r1*prob .LT. fm_h )) THEN
          wx = 1/(r2*xmax+1-r2)
          wxx = wx*xmax
          aux = wxx/(2-wxx)
          frej = (1 + aux*(aux-c2)+c1*wxx*wxx)/fm_h
        ELSE
          wx = 1/(r2*xmax**3+1-r2)**0.333333333
          frej = 1 - log(wx)/fm_s
        END IF
        IF((( r3 .LT. frej )))GO TO9752
      GO TO 9751
9752  CONTINUE
      wx = wx*Uj
      h1 = (peie + prm)/T
      pese1 = peie - wx
      e(np) = pese1
      dcosth = h1*(pese1-prm)/(pese1+prm)
      sinthe = dsqrt(1-dcosth)
      costhe = dsqrt(dcosth)
      call uphi(2,1)
      pese2 = wx - Uj + prm
      edep_local = 0
      IF (( pese2 .GT. ae(medium) )) THEN
        IF (( np+1 .GT. 50 )) THEN
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,'(//,3a,/,2(a,i9))') ' In subroutine ','eii_sample
     *', ' stack size exceeded! ',' $MAXSTACK = ',50,' np = ',np+1
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        END IF
        np = np+1
        e(np) = pese2
        dcosth = h1*(pese2-prm)/(pese2+prm)
        sinthe = -dsqrt(1-dcosth)
        costhe = dsqrt(dcosth)
        iq(np) = -1
        call uphi(3,2)
        edep = 0
      ELSE
        edep = wx - Uj
        edep_local = edep
        IARG=34
        IF ((IAUSFL(IARG+1).NE.0)) THEN
          CALL AUSGAB(IARG)
        END IF
      END IF
      call relax(Uj,ish,iZ)
      IF (( edep .GT. 0 )) THEN
        IARG=4
        IF ((IAUSFL(IARG+1).NE.0)) THEN
          CALL AUSGAB(IARG)
        END IF
      END IF
      return
      end
      subroutine egs_scale_photon_xsection(imed,fac,which)
      implicit none
      integer*4 imed,which
      real*8 fac
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      COMMON/PHOTIN/ EBINDA(1), GE0(1),GE1(1), GMFP0(2000,1),GMFP1(2000,
     *1),GBR10(2000,1),GBR11(2000,1),GBR20(2000,1),GBR21(2000,1), RCO0(1
     *),RCO1(1), RSCT0(100,1),RSCT1(100,1), COHE0(2000,1),COHE1(2000,1),
     *  PHOTONUC0(2000,1),PHOTONUC1(2000,1), DPMFP, MPGEM(1,1), NGR(1)
      real*8 EBINDA,  GE0,GE1,  GMFP0,GMFP1,  GBR10,GBR11,  GBR20,GBR21,
     *  RCO0,RCO1,  RSCT0,RSCT1,  COHE0,COHE1,   PHOTONUC0,PHOTONUC1,  D
     *PMFP
      integer*4
     *                  MPGEM,
     *                          NGR
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      integer*4 ifirst,ilast,medium,j
      logical has_r
      real*8 gle,gmfp,gbr1,gbr2,cohfac,aux,gmfp_old,gbr1_old,gbr2_old,co
     *hfac_old
      character*8 strings(5)
      data strings/'photon','Rayleigh','Compton','pair','photo'/
      IF (( which .LT. 0 .OR. which .GT. 4 )) THEN
        return
      END IF
      IF (( imed .GT. 0 .AND. imed .LE. nmed )) THEN
        ifirst = imed
        ilast = imed
      ELSE
        ifirst = 1
        ilast = nmed
      END IF
      IF (( which .EQ. 1 )) THEN
        has_r = .false.
        DO 9761 medium=ifirst,ilast
          IF (( iraylm(medium) .EQ. 1 )) THEN
            has_r = .true.
          END IF
9761    CONTINUE
9762    CONTINUE
        IF((.NOT.has_r))return
      END IF
      write(i_log,*) ' '
      DO 9771 medium=ifirst,ilast
        write(i_log,'(a,a,a,i3,a,f9.5)') 'Scaling ',strings(which+1),' x
     *-section data for medium', medium,' with ',fac
        DO 9781 j=1,mge(medium)
          gle = (j - ge0(medium))/ge1(medium)
          gmfp = gmfp0(j,medium) + gmfp1(j,medium)*gle
          gbr1 = gbr10(j,medium) + gbr11(j,medium)*gle
          gbr2 = gbr20(j,medium) + gbr21(j,medium)*gle
          IF (( iraylm(medium) .EQ. 1 )) THEN
            cohfac = cohe0(j,medium) + cohe1(j,medium)*gle
          ELSE
            cohfac = 1
          END IF
          IF (( which .EQ. 0 )) THEN
            gmfp = gmfp/fac
          ELSE IF(( which .EQ. 1 )) THEN
            cohfac = cohfac/(fac*(1-cohfac)+cohfac)
          ELSE
            IF (( which .EQ. 2 )) THEN
              aux = fac*(gbr2-gbr1) + gbr1 + 1 - gbr2
              gbr2 = (gbr1 + fac*(gbr2-gbr1))/aux
              gbr1 = gbr1/aux
            ELSE IF(( which .EQ. 3 )) THEN
              aux = fac*gbr1 + 1 - gbr1
              gbr2 = (fac*gbr1 + gbr2-gbr1)/aux
              gbr1 = fac*gbr1/aux
            ELSE
              aux = gbr2 + fac*(1-gbr2)
              gbr1 = gbr1/aux
              gbr2 = gbr2/aux
            END IF
            gmfp = gmfp/aux
            cohfac = cohfac*aux/(aux*cohfac + 1 - cohfac)
          END IF
          IF (( j .GT. 1 )) THEN
            gmfp1(j-1,medium) = (gmfp - gmfp_old)*ge1(medium)
            gmfp0(j-1,medium) = gmfp - gmfp1(j-1,medium)*gle
            gbr11(j-1,medium) = (gbr1 - gbr1_old)*ge1(medium)
            gbr10(j-1,medium) = gbr1 - gbr11(j-1,medium)*gle
            gbr21(j-1,medium) = (gbr2 - gbr2_old)*ge1(medium)
            gbr20(j-1,medium) = gbr2 - gbr21(j-1,medium)*gle
            cohe1(j-1,medium) = (cohfac - cohfac_old)*ge1(medium)
            cohe0(j-1,medium) = cohfac - cohe1(j-1,medium)*gle
          END IF
          gmfp_old = gmfp
          gbr1_old = gbr1
          gbr2_old = gbr2
          cohfac_old = cohfac
9781    CONTINUE
9782    CONTINUE
        gmfp1(mge(medium),medium) = gmfp1(mge(medium)-1,medium)
        gmfp0(mge(medium),medium) = gmfp0(mge(medium)-1,medium)
        gbr11(mge(medium),medium) = gbr11(mge(medium)-1,medium)
        gbr10(mge(medium),medium) = gbr10(mge(medium)-1,medium)
        gbr21(mge(medium),medium) = gbr21(mge(medium)-1,medium)
        gbr20(mge(medium),medium) = gbr20(mge(medium)-1,medium)
        cohe1(mge(medium),medium) = cohe1(mge(medium)-1,medium)
        cohe0(mge(medium),medium) = cohe0(mge(medium)-1,medium)
9771  CONTINUE
9772  CONTINUE
      return
      end
      subroutine egs_init_user_photon(prefix,comp_prefix,photonuc_prefix
     *,out)
      implicit none
      character*(*) prefix, comp_prefix,  photonuc_prefix
      integer*4 out
      COMMON/BREMPR/ DL1(8,1),DL2(8,1),DL3(8,1),DL4(8,1),DL5(8,1),DL6(8,
     *1), ALPHI(2,1),BPAR(2,1),DELPOS(2,1), WA(1,50),PZ(1,50),ZELEM(1,50
     *),RHOZ(1,50), PWR2I(50), DELCM(1),ZBRANG(1),LZBRANG(1),NNE(1), IBR
     *DST,IPRDST,ibr_nist,pair_nrc,itriplet, ASYM(1,50,2)
      CHARACTER*4 ASYM
      real*8 DL1,DL2,DL3,DL4,DL5,DL6,   ALPHI,  BPAR,  DELPOS,  WA,  PZ,
     *  ZELEM,  RHOZ,  PWR2I,  DELCM,  ZBRANG,  LZBRANG
      integer*4 NNE,  IBRDST,  IPRDST,  ibr_nist,  itriplet,  pair_nrc
      COMMON/EDGE/binding_energies(30,100), interaction_prob(6,100), rel
     *axation_prob(39,100), edge_energies(16,100), edge_number(100), edg
     *e_a(16,100), edge_b(16,100), edge_c(16,100), edge_d(16,100), IEDGF
     *L(1),IPHTER(1)
      real*8 binding_energies,  interaction_prob,    relaxation_prob,  e
     *dge_energies,  edge_a,edge_b,edge_c,edge_d
      integer*2 IEDGFL,  IPHTER
      integer*4 edge_number
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      COMMON/PHOTIN/ EBINDA(1), GE0(1),GE1(1), GMFP0(2000,1),GMFP1(2000,
     *1),GBR10(2000,1),GBR11(2000,1),GBR20(2000,1),GBR21(2000,1), RCO0(1
     *),RCO1(1), RSCT0(100,1),RSCT1(100,1), COHE0(2000,1),COHE1(2000,1),
     *  PHOTONUC0(2000,1),PHOTONUC1(2000,1), DPMFP, MPGEM(1,1), NGR(1)
      real*8 EBINDA,  GE0,GE1,  GMFP0,GMFP1,  GBR10,GBR11,  GBR20,GBR21,
     *  RCO0,RCO1,  RSCT0,RSCT1,  COHE0,COHE1,   PHOTONUC0,PHOTONUC1,  D
     *PMFP
      integer*4
     *                  MPGEM,
     *                          NGR
      COMMON/THRESH/RMT2,RMSQ, AP(1),AE(1),UP(1),UE(1),TE(1),THMOLL(1)
      real*8 RMT2,  RMSQ,  AP,  AE,  UP,  UE,  TE,  THMOLL
      common/compton_data/ iz_array(1538),  be_array(1538),  Jo_array(15
     *38),  erfJo_array(1538),   ne_array(1538),  shn_array(1538),
     *shell_array(200,1), eno_array(200,1), eno_atbin_array(200,1), n_sh
     *ell(1), radc_flag,  ibcmp(1)
      integer*4 iz_array,ne_array,shn_array,eno_atbin_array, shell_array
     *,n_shell,radc_flag
      real*8 be_array,Jo_array,erfJo_array,eno_array
      integer*2 ibcmp
      common/x_options/eadl_relax,  mcdf_pe_xsections
      logical eadl_relax, mcdf_pe_xsections
      integer*4 lnblnk1,egs_get_unit,medium, photo_unit,pair_unit,raylei
     *gh_unit,triplet_unit, ounit,egs_open_file,compton_unit,  photonuc_
     *unit
      integer*4 nge,sorted(50),i,j,k,iz,iz_old,ndat
      real*8 z_sorted(50),pz_sorted(50)
      real*8 sig_photo(2000),sig_pair(2000),sig_triplet(2000), sig_rayle
     *igh(2000),sig_compton(2000)
      real*8 sigma,cohe,gmfp,gbr1,gbr2,sig_KN,gle,e,sig_p
      real*8 cohe_old,gmfp_old,gbr1_old,gbr2_old,  sig_photonuc(2000), p
     *hotonuc, photonuc_old
      real*8 etmp(2000),ftmp(2000)
      real*8 sumZ,sumA,con1,con2,egs_KN_sigma0
      real*8 bc_emin,bc_emax,bc_dle,bc_data(183),bc_tmp(183),bcf,aj
      integer*4 bc_ne
      logical input_compton_data,  input_photonuc_data
      character data_dir*128,photo_file*140,pair_file*140,rayleigh_file*
     *144, triplet_file*142,tmp_string*144,compton_file*144,  photonuc_f
     *ile*144
      write(i_log,'(/a$)') '(Re)-initializing photon cross sections'
      write(i_log,'(a,a/)') ' with files from the series: ', prefix(:lnb
     *lnk1(prefix))
      write(i_log,'(a,a)') ' Compton cross sections: ',comp_prefix(:lnbl
     *nk1(comp_prefix))
      IF ((iphotonuc .EQ. 1)) THEN
        write(i_log,'(a,a)') ' Photonuclear cross sections: ', photonuc_
     *  prefix(:lnblnk1(photonuc_prefix))
        input_photonuc_data = .false.
        IF ((lnblnk1(photonuc_prefix) .GT. 0 .AND. photonuc_prefix(1:7)
     *  .NE. 'default')) THEN
          input_photonuc_data = .true.
        END IF
      END IF
      input_compton_data = .false.
      IF (( ibcmp(1) .GT. 1 .AND. lnblnk1(comp_prefix) .GT. 0 )) THEN
        IF((comp_prefix(1:7) .NE. 'default'))input_compton_data = .true.
      END IF
      data_dir = hen_house(:lnblnk1(hen_house)) // 'data' // '/'
      photo_file = data_dir(:lnblnk1(data_dir)) // prefix(:lnblnk1(prefi
     *x)) // '_photo.data'
      pair_file = data_dir(:lnblnk1(data_dir)) // prefix(:lnblnk1(prefix
     *)) // '_pair.data'
      triplet_file = data_dir(:lnblnk1(data_dir)) // prefix(:lnblnk1(pre
     *fix)) // '_triplet.data'
      rayleigh_file = data_dir(:lnblnk1(data_dir)) // prefix(:lnblnk1(pr
     *efix)) // '_rayleigh.data'
      IF (( input_compton_data )) THEN
        compton_file = data_dir(:lnblnk1(data_dir)) // comp_prefix(:lnbl
     *  nk1(comp_prefix)) // '_compton.data'
      ELSE
        compton_file = data_dir(:lnblnk1(data_dir)) // 'compton_sigma.da
     *ta'
      END IF
      write(i_log,'(a,a)') ' Using Compton cross sections from ', compto
     *n_file(:lnblnk1(compton_file))
      IF ((iphotonuc .EQ. 1)) THEN
        IF (( input_photonuc_data )) THEN
          photonuc_file = data_dir(:lnblnk1(data_dir)) // photonuc_prefi
     *    x(:lnblnk1(photonuc_prefix)) // '_photonuc.data'
        ELSE
          photonuc_file = data_dir(:lnblnk1(data_dir)) // 'iaea_photonuc
     *.data'
        END IF
        write(i_log,'(a,a)') ' Using photonuclear cross sections from ',
     *   photonuc_file(:lnblnk1(photonuc_file))
      END IF
      photo_unit = 83
      photo_unit = egs_get_unit(photo_unit)
      IF (( photo_unit .LT. 1 )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) 'egs_init_user_photon: failed to get a free Fortr
     *an I/O unit'
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      tmp_string = photo_file
      open(photo_unit,file=photo_file,status='old',err=9790)
      pair_unit = 84
      pair_unit = egs_get_unit(pair_unit)
      IF (( pair_unit .LT. 1 )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) 'egs_init_user_photon: failed to get a free Fortr
     *an I/O unit'
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      tmp_string = pair_file
      open(pair_unit,file=pair_file,status='old',err=9790)
      triplet_unit = 85
      triplet_unit = egs_get_unit(triplet_unit)
      IF (( triplet_unit .LT. 1 )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) 'egs_init_user_photon: failed to get a free Fortr
     *an I/O unit'
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      tmp_string = triplet_file
      open(triplet_unit,file=triplet_file,status='old',err=9790)
      rayleigh_unit = 86
      rayleigh_unit = egs_get_unit(rayleigh_unit)
      IF (( rayleigh_unit .LT. 1 )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) 'egs_init_user_photon: failed to get a free Fortr
     *an I/O unit'
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      tmp_string = rayleigh_file
      open(rayleigh_unit,file=rayleigh_file,status='old',err=9790)
      IF (( ibcmp(1) .GT. 1 )) THEN
        compton_unit = 88
        compton_unit = egs_get_unit(compton_unit)
        IF (( compton_unit .LT. 1 )) THEN
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,*) 'egs_init_user_photon: failed to get a free For
     *tran I/O unit'
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        END IF
        tmp_string = compton_file
        open(compton_unit,file=compton_file,status='old',err=9790)
      END IF
      IF (( iphotonuc .EQ. 1 )) THEN
        photonuc_unit = 89
        photonuc_unit = egs_get_unit(photonuc_unit)
        IF (( photonuc_unit .LT. 1 )) THEN
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,*) 'egs_init_user_photon: failed to get a free For
     *tran I/O unit'
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        END IF
        tmp_string = photonuc_file
        open(photonuc_unit,file=photonuc_file,status='old',err=9790)
      END IF
      IF (( out .EQ. 1 )) THEN
        ounit = egs_open_file(87,0,1,'.xsections')
        write(ounit,'(/a,a,a)') 'Photon cross sections initialized from
     *', prefix(:lnblnk1(prefix)),' data files'
        write(ounit,'(a,/)') '==========================================
     *=================================='
        write(ounit,'(a,/)') 'Grid energies and cross sections are outpu
     *t'
        IF ((iphotonuc .EQ. 1)) THEN
          write(ounit,'(5x,a,t19,a,t34,a,t49,a,t64,a,t79,a)') 'Energy','
     * GMFP(cm) ',' Pair ','Compton',' GMFP(cm) ', ' GMFP(cm) '
          write(ounit,'(5x,a,t19,a,t34,a,t49,a,t64,a,t79,a/)') '(MeV)','
     *no Rayleigh','(fraction)','(fraction)','with Rayleigh', 'w/ Ray +
     *photnuc'
        ELSE
          write(ounit,'(5x,a,t19,a,t34,a,t49,a,t64,a)') 'Energy',' GMFP(
     *cm) ',' Pair ','Compton',' GMFP(cm) '
          write(ounit,'(5x,a,t19,a,t34,a,t49,a,t64,a/)') '(MeV)','no Ray
     *leigh','(fraction)','(fraction)','with Rayleigh'
        END IF
      END IF
      DO 9801 iz=1,100
        read(photo_unit,*) ndat
        read(photo_unit,*) (etmp(k),ftmp(k),k=1,ndat)
        k = 0
        DO 9811 j=ndat,2,-1
          IF (( etmp(j)-etmp(j-1) .LT. 1e-5 )) THEN
            k = k+1
            IF (( k .LE. 30 )) THEN
              binding_energies(k,iz) = exp(etmp(j))
            ELSE
              write(i_log,'(/a)') '***************** Error: '
              write(i_log,'(i3,a,i3,//a)') k,' binding energies read exc
     *eeding array size of', 30,'Increase $MXSHXSEC in egsnrc.macros!'
              write(i_log,'(/a)') '***************** Quiting now.'
              call exit(1)
            END IF
            IF((.NOT.eadl_relax .AND. k .GE. 4))GO TO9812
          END IF
9811    CONTINUE
9812    CONTINUE
9801  CONTINUE
9802  CONTINUE
      IF ((mcdf_pe_xsections)) THEN
        call egs_read_shellwise_pe()
      END IF
      DO 9821 medium=1,nmed
        mge(medium) = 2000
        nge = 2000
        ge1(medium) = nge-1
        ge1(medium) = ge1(medium)/log(up(medium)/ap(medium))
        ge0(medium) = 1 - ge1(medium)*log(ap(medium))
        write(i_log,'(a,i3,a,$)') ' Working on medium ',medium,' ... '
        IF (( out .EQ. 1 )) THEN
          write(ounit,'(/,2x,a,i3,a,24a1/)') 'Medium ',medium,': ', (med
     *    ia(k,medium),k=1,24)
        END IF
        sumZ=0
        sumA=0
        DO 9831 i=1,nne(medium)
          z_sorted(i) = zelem(medium,i)
          sumZ = sumZ + pz(medium,i)*zelem(medium,i)
          sumA = sumA + pz(medium,i)*wa(medium,i)
9831    CONTINUE
9832    CONTINUE
        con1 = sumZ*rho(medium)/(sumA*1.6605655)
        con2 = rho(medium)/(sumA*1.6605655)
        call egs_heap_sort(nne(medium),z_sorted,sorted)
        DO 9841 i=1,nne(medium)
          pz_sorted(i) = pz(medium,sorted(i))
9841    CONTINUE
9842    CONTINUE
        IF ((mcdf_pe_xsections)) THEN
          call egsi_get_shell_data(medium,nge,nne(medium),z_sorted,pz_so
     *    rted, ge1(medium),ge0(medium),sig_photo)
        ELSE
          call egsi_get_data(0,photo_unit,nge,nne(medium),z_sorted,pz_so
     *    rted, ge1(medium),ge0(medium),sig_photo)
        END IF
        call egsi_get_data(0,rayleigh_unit,nge,nne(medium),z_sorted,pz_s
     *  orted, ge1(medium),ge0(medium),sig_rayleigh)
        call egsi_get_data(1,pair_unit,nge,nne(medium),z_sorted,pz_sorte
     *  d, ge1(medium),ge0(medium),sig_pair)
        call egsi_get_data(2,triplet_unit,nge,nne(medium),z_sorted,pz_so
     *  rted, ge1(medium),ge0(medium),sig_triplet)
        IF (( iphotonuc .EQ. 1 )) THEN
          call egsi_get_data(3,photonuc_unit,nge,nne(medium),z_sorted,pz
     *    _sorted, ge1(medium),ge0(medium),sig_photonuc)
        END IF
        IF (( ibcmp(1) .GT. 1 )) THEN
          IF (( input_compton_data )) THEN
            call egsi_get_data(0,compton_unit,nge,nne(medium), z_sorted,
     *      pz_sorted,ge1(medium),ge0(medium), sig_compton)
          ELSE
            rewind(compton_unit)
            read(compton_unit,*) bc_emin,bc_emax,bc_ne
            IF (( bc_ne .GT. 183 )) THEN
              write(i_log,'(/a)') '***************** Error: '
              write(i_log,*) 'Number of input Compton data exceeds array
     * size'
              write(i_log,'(/a)') '***************** Quiting now.'
              call exit(1)
            END IF
            bc_dle = log(bc_emax/bc_emin)/(bc_ne-1)
            DO 9851 j=1,bc_ne
              bc_data(j) = 0
9851        CONTINUE
9852        CONTINUE
            iz_old = 1
            DO 9861 i=1,nne(medium)
              iz = int(z_sorted(i)+0.5)
              DO 9871 j=iz_old,iz
                read(compton_unit,*) (bc_tmp(k),k=1,bc_ne)
9871          CONTINUE
9872          CONTINUE
              DO 9881 j=1,bc_ne
                bc_data(j)=bc_data(j)+pz_sorted(i)*z_sorted(i)*bc_tmp(j)
9881          CONTINUE
9882          CONTINUE
              iz_old = iz+1
9861        CONTINUE
9862        CONTINUE
            DO 9891 j=1,bc_ne
              bc_data(j)=log(bc_data(j)/sumZ)
9891        CONTINUE
9892        CONTINUE
          END IF
        END IF
        call egs_init_rayleigh(medium,sig_rayleigh)
        DO 9901 i=1,nge
          gle = (i - ge0(medium))/ge1(medium)
          e = exp(gle)
          sig_KN = sumZ*egs_KN_sigma0(e)
          IF (( ibcmp(1) .GT. 1 )) THEN
            IF (( input_compton_data )) THEN
              sig_KN = sig_compton(i)
            ELSE
              IF (( e .LE. bc_emin )) THEN
                bcf = exp(bc_data(1))
              ELSE IF(( e .LT. bc_emax )) THEN
                aj = 1 + log(e/bc_emin)/bc_dle
                j = int(aj)
                aj = aj - j
                bcf = exp(bc_data(j)*(1-aj) + bc_data(j+1)*aj)
              ELSE
                bcf = 1
              END IF
              sig_KN = sig_KN*bcf
            END IF
          END IF
          sig_p = sig_pair(i) + sig_triplet(i)
          sigma = sig_KN + sig_p + sig_photo(i)
          gmfp = 1/(sigma*con2)
          gbr1 = sig_p/sigma
          gbr2 = gbr1 + sig_KN/sigma
          cohe = sigma/(sig_rayleigh(i) + sigma)
          photonuc = sigma/(sig_photonuc(i) + sigma)
          IF (( out .EQ. 1 )) THEN
            IF ((iphotonucm(medium) .EQ. 1)) THEN
              write(ounit,'(6(1pe15.6))') e,gmfp,gbr1,gbr2-gbr1, gmfp*co
     *        he,gmfp*cohe*photonuc
            ELSE
              write(ounit,'(5(1pe15.6))') e,gmfp,gbr1,gbr2-gbr1,gmfp*coh
     *        e
            END IF
          END IF
          IF (( i .GT. 1 )) THEN
            gmfp1(i-1,medium) = (gmfp - gmfp_old)*ge1(medium)
            gmfp0(i-1,medium) = gmfp - gmfp1(i-1,medium)*gle
            gbr11(i-1,medium) = (gbr1 - gbr1_old)*ge1(medium)
            gbr10(i-1,medium) = gbr1 - gbr11(i-1,medium)*gle
            gbr21(i-1,medium) = (gbr2 - gbr2_old)*ge1(medium)
            gbr20(i-1,medium) = gbr2 - gbr21(i-1,medium)*gle
            cohe1(i-1,medium) = (cohe - cohe_old)*ge1(medium)
            cohe0(i-1,medium) = cohe - cohe1(i-1,medium)*gle
            photonuc1(i-1,medium) = (photonuc - photonuc_old)*ge1(medium
     *      )
            photonuc0(i-1,medium) = photonuc - photonuc1(i-1,medium)*gle
          END IF
          gmfp_old = gmfp
          gbr1_old = gbr1
          gbr2_old = gbr2
          cohe_old = cohe
          photonuc_old = photonuc
9901    CONTINUE
9902    CONTINUE
        gmfp1(nge,medium) = gmfp1(nge-1,medium)
        gmfp0(nge,medium) = gmfp - gmfp1(nge,medium)*gle
        gbr11(nge,medium) = gbr11(nge-1,medium)
        gbr10(nge,medium) = gbr1 - gbr11(nge,medium)*gle
        gbr21(nge,medium) = gbr21(nge-1,medium)
        gbr20(nge,medium) = gbr2 - gbr21(nge,medium)*gle
        cohe1(nge,medium) = cohe1(nge-1,medium)
        cohe0(nge,medium) = cohe - cohe1(nge,medium)*gle
        photonuc1(nge,medium) = photonuc1(nge-1,medium)
        photonuc0(nge,medium) = photonuc - photonuc1(nge,medium)*gle
        write(i_log,'(a)') 'OK'
9821  CONTINUE
9822  CONTINUE
      close(photo_unit)
      close(pair_unit)
      close(triplet_unit)
      close(rayleigh_unit)
      IF (( iphotonuc .EQ. 1 )) THEN
        close(photonuc_unit)
      END IF
      IF (( ibcmp(1) .GT. 1 )) THEN
        close(compton_unit)
      END IF
      IF (( out .EQ. 1 )) THEN
        close(ounit)
      END IF
      return
9790  CONTINUE
      write(i_log,'(/a)') '***************** Error: '
      write(i_log,'(//a,a)') 'Failed to open data file ',tmp_string(:lnb
     *lnk1(tmp_string))
      write(i_log,'(/a)') '***************** Quiting now.'
      call exit(1)
      return
      end
      subroutine egs_init_rayleigh(medium,sig_rayleigh)
      implicit none
      COMMON/THRESH/RMT2,RMSQ, AP(1),AE(1),UP(1),UE(1),TE(1),THMOLL(1)
      real*8 RMT2,  RMSQ,  AP,  AE,  UP,  UE,  TE,  THMOLL
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      COMMON/MISC/  DUNIT,KMPI,KMPO,RHOR(1),MED(1),IRAYLR(1),IPHOTONUCR(
     *1)
      real*8 DUNIT,  RHOR
      integer*4 KMPI,  KMPO
      integer*2 MED,  IRAYLR,  IPHOTONUCR
      COMMON/PHOTIN/ EBINDA(1), GE0(1),GE1(1), GMFP0(2000,1),GMFP1(2000,
     *1),GBR10(2000,1),GBR11(2000,1),GBR20(2000,1),GBR21(2000,1), RCO0(1
     *),RCO1(1), RSCT0(100,1),RSCT1(100,1), COHE0(2000,1),COHE1(2000,1),
     *  PHOTONUC0(2000,1),PHOTONUC1(2000,1), DPMFP, MPGEM(1,1), NGR(1)
      real*8 EBINDA,  GE0,GE1,  GMFP0,GMFP1,  GBR10,GBR11,  GBR20,GBR21,
     *  RCO0,RCO1,  RSCT0,RSCT1,  COHE0,COHE1,   PHOTONUC0,PHOTONUC1,  D
     *PMFP
      integer*4
     *                  MPGEM,
     *                          NGR
      COMMON/BREMPR/ DL1(8,1),DL2(8,1),DL3(8,1),DL4(8,1),DL5(8,1),DL6(8,
     *1), ALPHI(2,1),BPAR(2,1),DELPOS(2,1), WA(1,50),PZ(1,50),ZELEM(1,50
     *),RHOZ(1,50), PWR2I(50), DELCM(1),ZBRANG(1),LZBRANG(1),NNE(1), IBR
     *DST,IPRDST,ibr_nist,pair_nrc,itriplet, ASYM(1,50,2)
      CHARACTER*4 ASYM
      real*8 DL1,DL2,DL3,DL4,DL5,DL6,   ALPHI,  BPAR,  DELPOS,  WA,  PZ,
     *  ZELEM,  RHOZ,  PWR2I,  DELCM,  ZBRANG,  LZBRANG
      integer*4 NNE,  IBRDST,  IPRDST,  ibr_nist,  itriplet,  pair_nrc
      COMMON/rayleigh_inputs/iray_ff_media(1),iray_ff_file(1)
      character*24 iray_ff_media
      character*128 iray_ff_file
      COMMON/rayleigh_sampling/xgrid(100,1), fcum(100,1), b_array(100,1)
     *, c_array(100,1), i_array(100,1), pmax0(2000,1),pmax1(2000,1)
      real*8 xgrid, fcum, b_array, c_array,pmax0, pmax1
      integer*4 i_array
      real*8 xval(100),aff(100,100),ff(100,1)
      real*8 xsc, fsc
      real*8 sig_rayleigh(2000), pe_array(2000,1)
      real*8 e,egs_rayleigh_sigma,gmfp,gle,conv,dle,dlei,sumA
      real*8 totRayleigh2,pzmin
      real*8 emin, emax
      integer*4 i,j,k,ff_unit, egs_get_unit, ne
      integer*4 lnblnk1, EOF, nff, medium, ncustom
      character dummy*24, afac_file*128, ff_file*128
      IF ((iraylm(medium).EQ.0)) THEN
        return
      END IF
      ncustom=0
      write(dummy,'(24a1)')(media(j,medium),j=1,24)
      ff_file=' '
      DO 9911 i=1,1
        IF ((lnblnk1(iray_ff_file(i)).NE.0)) THEN
          ncustom = ncustom + 1
        END IF
9911  CONTINUE
9912  CONTINUE
      DO 9921 i=1,ncustom
        IF ((dummy(:lnblnk1(dummy)) .EQ. iray_ff_media(i))) THEN
          ff_file = iray_ff_file(i)
        END IF
9921  CONTINUE
9922  CONTINUE
      ff_unit = egs_get_unit(0)
      IF (( ff_unit .LT. 1 )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) 'egs_init_rayleigh: failed to get a free Fortran
     *I/O unit'
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      IF (( lnblnk1(ff_file) .GT. 0)) THEN
        open(ff_unit,file=ff_file(:lnblnk1(ff_file)), status='old',err=9
     *  930)
        GOTO 9940
9930    write(i_log,'(/a)') '***************** Error: '
        write(i_log,'(2a)') 'egs_init_rayleigh: failed to open custom ff
     * file ', ff_file(:lnblnk1(ff_file))
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
9940    write(i_log,'(/2a)') 'Opened custom ff file ',ff_file(:lnblnk1(f
     *  f_file))
        j = 0
9951    CONTINUE
          j = j + 1
          read(ff_unit,*,IOSTAT=EOF) xsc, fsc
          IF((EOF .LT. 0))GO TO9952
          IF ((j .LE. 100)) THEN
            xgrid(j,medium)=xsc
            ff(j,medium)=fsc
          END IF
        GO TO 9951
9952    CONTINUE
        nff = j-1
        IF ((nff .GT. 100)) THEN
          write(i_log,'(/a)') '***************** Error: '
          write(i_log,'(a,/,a,i5,a,i5,/,a)') 'subroutine egs_init_raylei
     *gh: form factors size too small!!', '$XRAYFF =  ', 100,', and need
     * to be ',nff, ' and try again!!!'
          write(i_log,'(/a)') '***************** Quiting now.'
          call exit(1)
        END IF
        IF((xgrid(1,medium) .LT. 1e-6))xgrid(1,medium) = 1e-4
        write(*,*) '\n  -> ', nff, ' values of mol. ff read!'
        sumA = 0.0
        DO 9961 j=1,nne(medium)
          sumA=sumA+PZ(medium,j)*WA(medium,j)
9961    CONTINUE
9962    CONTINUE
        DO 9971 j=1,MGE(medium)
          gle=(j-GE0(medium))/GE1(medium)
          e=exp(gle)
          sig_rayleigh(j)=egs_rayleigh_sigma(medium,e,nff, xgrid(1,mediu
     *    m),ff(1,medium))*sumA
9971    CONTINUE
9972    CONTINUE
      ELSE
        DO 9981 i=1,len(afac_file)
          afac_file(i:i) = ' '
9981    CONTINUE
9982    CONTINUE
        afac_file = hen_house(:lnblnk1(hen_house))//'pegs4'//'/'//'pgs4f
     *orm.dat'
        open(ff_unit,file=afac_file(:lnblnk1(afac_file)), status='old',e
     *  rr=9990)
        GOTO 10000
9990    write(i_log,'(/a)') '***************** Error: '
        write(i_log,'(2a)') 'egs_init_rayleigh: failed to open atomic ff
     * file', afac_file(:lnblnk1(afac_file))
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
10000   read(ff_unit,*) xval, aff
        DO 10011 i=1,100
          ff(i,medium) = 0.0
          xgrid(i,medium)=xval(i)
          DO 10021 j=1,nne(medium)
            ff(i,medium)=ff(i,medium)+PZ(medium,j)*aff(i,int(zelem(mediu
     *      m,j)))**2
10021     CONTINUE
10022     CONTINUE
          ff(i,medium) = sqrt(ff(i,medium))
10011   CONTINUE
10012   CONTINUE
        nff = 100
        IF((xgrid(1,medium) .LT. 1e-6))xgrid(1,medium) = 1e-4
        write(i_log,'(/a,i4,a)') '  -> ', nff, ' atomic ff values comput
     *ed!'
      END IF
      close(ff_unit)
      emin = exp((1 - ge0(medium))/ge1(medium))
      emax = exp((mge(medium) - ge0(medium))/ge1(medium))
      call prepare_rayleigh_data(nff,xgrid(1,medium),ff(1,medium), mge(m
     *edium),emin,emax, pe_array(1,medium),100, fcum(1,medium),i_array(1
     *,medium), b_array(1,medium),c_array(1,medium))
      ne=MGE(medium)
      dle=log(up(medium)/ap(medium))/(ne-1)
      dlei=1/dle
      DO 10031 i=1,ne-1
        gle = (i - ge0(medium))/ge1(medium)
        pmax1(i,medium)=(pe_array(i+1,medium)-pe_array(i,medium))*ge1(me
     *  dium)
        pmax0(i,medium)=pe_array(i,medium)-pmax1(i,medium)*gle
10031 CONTINUE
10032 CONTINUE
      pmax0(ne,medium)=pmax0(ne-1,medium)
      pmax1(ne,medium)=pmax1(ne-1,medium)
      return
      end
      subroutine egs_init_rayleigh_sampling(medium)
      implicit none
      COMMON/THRESH/RMT2,RMSQ, AP(1),AE(1),UP(1),UE(1),TE(1),THMOLL(1)
      real*8 RMT2,  RMSQ,  AP,  AE,  UP,  UE,  TE,  THMOLL
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      COMMON/MISC/  DUNIT,KMPI,KMPO,RHOR(1),MED(1),IRAYLR(1),IPHOTONUCR(
     *1)
      real*8 DUNIT,  RHOR
      integer*4 KMPI,  KMPO
      integer*2 MED,  IRAYLR,  IPHOTONUCR
      COMMON/PHOTIN/ EBINDA(1), GE0(1),GE1(1), GMFP0(2000,1),GMFP1(2000,
     *1),GBR10(2000,1),GBR11(2000,1),GBR20(2000,1),GBR21(2000,1), RCO0(1
     *),RCO1(1), RSCT0(100,1),RSCT1(100,1), COHE0(2000,1),COHE1(2000,1),
     *  PHOTONUC0(2000,1),PHOTONUC1(2000,1), DPMFP, MPGEM(1,1), NGR(1)
      real*8 EBINDA,  GE0,GE1,  GMFP0,GMFP1,  GBR10,GBR11,  GBR20,GBR21,
     *  RCO0,RCO1,  RSCT0,RSCT1,  COHE0,COHE1,   PHOTONUC0,PHOTONUC1,  D
     *PMFP
      integer*4
     *                  MPGEM,
     *                          NGR
      COMMON/BREMPR/ DL1(8,1),DL2(8,1),DL3(8,1),DL4(8,1),DL5(8,1),DL6(8,
     *1), ALPHI(2,1),BPAR(2,1),DELPOS(2,1), WA(1,50),PZ(1,50),ZELEM(1,50
     *),RHOZ(1,50), PWR2I(50), DELCM(1),ZBRANG(1),LZBRANG(1),NNE(1), IBR
     *DST,IPRDST,ibr_nist,pair_nrc,itriplet, ASYM(1,50,2)
      CHARACTER*4 ASYM
      real*8 DL1,DL2,DL3,DL4,DL5,DL6,   ALPHI,  BPAR,  DELPOS,  WA,  PZ,
     *  ZELEM,  RHOZ,  PWR2I,  DELCM,  ZBRANG,  LZBRANG
      integer*4 NNE,  IBRDST,  IPRDST,  ibr_nist,  itriplet,  pair_nrc
      COMMON/rayleigh_inputs/iray_ff_media(1),iray_ff_file(1)
      character*24 iray_ff_media
      character*128 iray_ff_file
      COMMON/rayleigh_sampling/xgrid(100,1), fcum(100,1), b_array(100,1)
     *, c_array(100,1), i_array(100,1), pmax0(2000,1),pmax1(2000,1)
      real*8 xgrid, fcum, b_array, c_array,pmax0, pmax1
      integer*4 i_array
      real*8 xval(100),aff(100,100),ff(100,1)
      real*8 xsc, fsc
      real*8 sig_rayleigh(2000), pe_array(2000,1)
      real*8 e,egs_rayleigh_sigma,gmfp,gle,conv,dle,dlei,sumA
      real*8 totRayleigh2,pzmin
      real*8 emin, emax
      integer*4 i,j,k,ff_unit, egs_get_unit, ne
      integer*4 lnblnk1, EOF, nff, medium, ncustom
      character dummy*24, afac_file*128, ff_file*128
      IF ((iraylm(medium).EQ.0)) THEN
        return
      END IF
      ff_unit = egs_get_unit(0)
      IF (( ff_unit .LT. 1 )) THEN
        write(i_log,'(/a)') '***************** Error: '
        write(i_log,*) 'egs_init_rayleigh: failed to get a free Fortran
     *I/O unit'
        write(i_log,'(/a)') '***************** Quiting now.'
        call exit(1)
      END IF
      DO 10041 i=1,len(afac_file)
        afac_file(i:i) = ' '
10041 CONTINUE
10042 CONTINUE
      afac_file = hen_house(:lnblnk1(hen_house))//'pegs4'//'/'//'pgs4for
     *m.dat'
      open(ff_unit,file=afac_file(:lnblnk1(afac_file)),status='old',err=
     *9990)
      GOTO 10000
9990  write(i_log,'(/a)') '***************** Error: '
      write(i_log,'(2a)') 'egs_init_rayleigh_sampling: failed to open at
     *omic ff file ', afac_file(:lnblnk1(afac_file))
      write(i_log,'(/a)') '***************** Quiting now.'
      call exit(1)
10000 read(ff_unit,*) xval, aff
      DO 10051 i=1,100
        ff(i,medium) = 0.0
        xgrid(i,medium)=xval(i)
        DO 10061 j=1,nne(medium)
          ff(i,medium)=ff(i,medium)+PZ(medium,j)*aff(i,int(zelem(medium,
     *    j)))**2
10061   CONTINUE
10062   CONTINUE
        ff(i,medium) = sqrt(ff(i,medium))
10051 CONTINUE
10052 CONTINUE
      nff = 100
      IF((xgrid(1,medium) .LT. 1e-6))xgrid(1,medium) = 1e-4
      write(i_log,'(/a,i4,a)') '  -> ', nff, ' atomic ff values computed
     *!'
      close(ff_unit)
      emin = exp((1 - ge0(medium))/ge1(medium))
      emax = exp((mge(medium) - ge0(medium))/ge1(medium))
      call prepare_rayleigh_data(nff,xgrid(1,medium),ff(1,medium), mge(m
     *edium),emin,emax, pe_array(1,medium),100, fcum(1,medium),i_array(1
     *,medium), b_array(1,medium),c_array(1,medium))
      ne=MGE(medium)
      DO 10071 i=1,ne-1
        gle = (i - ge0(medium))/ge1(medium)
        pmax1(i,medium)=(pe_array(i+1,medium)-pe_array(i,medium))*ge1(me
     *  dium)
        pmax0(i,medium)=pe_array(i,medium)-pmax1(i,medium)*gle
10071 CONTINUE
10072 CONTINUE
      pmax0(ne,medium)=pmax0(ne-1,medium)
      pmax1(ne,medium)=pmax1(ne-1,medium)
      return
      end
      real*8 function egs_rayleigh_sigma(imed,E,ndat,x,f)
      implicit none
      integer*4 i, j, k,imed, ndat
      real*8 hc2,conv,b,hc
      parameter (hc = 0.0123984768438,hc2=0.0001537222280)
      real*8 x(100), f(100), zero, E, xmax
      real*8 x1,x2,pow_x1,pow_x2,raysig,C,C2,f1,f2
      C=2.*hc2/(E*E)
      C2=C*C
      xmax=E/hc
      egs_rayleigh_sigma = 0.0
      DO 10081 i=1,ndat-1
        IF((x(i) .EQ. 0.0))x(i) = zero()
        IF((x(i+1) .EQ. 0.0))x(i+1) = zero()
        IF((f(i) .EQ. 0.0))f(i) = zero()
        IF((f(i+1) .EQ. 0.0))f(i+1) = zero()
        b = log(f(i+1)/f(i))/log(x(i+1)/x(i))
        x1=x(i)
        x2=x(i+1)
        IF ((x2 .GT. xmax)) THEN
          x2=xmax
        END IF
        pow_x1=x1**(2*b)
        pow_x2=x2**(2*b)
        raysig = pow_x2*(x2**2/(b+1)-(C*x2**4)/(b+2)+(C2*x2**6)/(2*b+6))
        raysig = raysig - pow_x1*(x1**2/(b+1)-(C*x1**4)/(b+2)+(C2*x1**6)
     *  /(2*b+6))
        raysig = raysig*f(i)*f(i)/pow_x1
        egs_rayleigh_sigma = egs_rayleigh_sigma + raysig
        IF ((x(i+1).GT.xmax)) THEN
          GO TO10082
        END IF
10081 CONTINUE
10082 CONTINUE
      egs_rayleigh_sigma = 0.49893439187842413747*C*egs_rayleigh_sigma
      return
      end
      subroutine egs_rayleigh_sampling(medium,e,gle,lgle,costhe,sinthe)
      implicit none
      real*8 e
      real*8 gle,costhe,sinthe,pmax,xv,xmax,csqthe
      real*8 rnnray1,rnnray0,hc_i,twice_hc2,dwi
      parameter(hc_i=80.65506856998,twice_hc2=0.000307444456)
      integer*4 lgle,ib,ibin,medium, trials
      common/randomm/ rng_array(128), urndm(97), crndm, cdrndm, cmrndm,
     *i4opt, ixx, jxx, fool_optimizer, twom24, rng_seed
      integer*4 urndm, crndm, cdrndm, cmrndm, i4opt, ixx, jxx, fool_opti
     *mizer,rng_seed,rng_array
      real*4 twom24
      COMMON/rayleigh_sampling/xgrid(100,1), fcum(100,1), b_array(100,1)
     *, c_array(100,1), i_array(100,1), pmax0(2000,1),pmax1(2000,1)
      real*8 xgrid, fcum, b_array, c_array,pmax0, pmax1
      integer*4 i_array
      dwi = 100-1
      pmax=pmax1(Lgle,MEDIUM)*gle+pmax0(Lgle,MEDIUM)
      xmax = hc_i*e
10091 CONTINUE
        IF((rng_seed .GT. 128))call ranmar_get
        rnnray1 = rng_array(rng_seed)*twom24
        rng_seed = rng_seed + 1
10101   CONTINUE
          IF((rng_seed .GT. 128))call ranmar_get
          rnnray0 = rng_array(rng_seed)*twom24
          rng_seed = rng_seed + 1
          rnnray0 = rnnray0*pmax
          ibin = 1 + rnnray0*dwi
          ib = i_array(ibin,medium)
          IF (( i_array(ibin+1,medium) .GT. ib )) THEN
10111       CONTINUE
              IF((rnnray0.LT.fcum(ib+1,medium)))GO TO10112
              ib=ib+1
            GO TO 10111
10112       CONTINUE
          END IF
          rnnray0 = (rnnray0 - fcum(ib,medium))*c_array(ib,medium)
          xv = xgrid(ib,medium)*exp(log(1+rnnray0)*b_array(ib,medium))
          IF(((xv .LT. xmax)))GO TO10102
        GO TO 10101
10102   CONTINUE
        xv = xv/e
        costhe = 1 - twice_hc2*xv*xv
        csqthe=costhe*costhe
        IF((( 2*rnnray1 .LT. 1 + csqthe )))GO TO10092
      GO TO 10091
10092 CONTINUE
      sinthe=sqrt(1.0-csqthe)
      return
      end
      subroutine prepare_rayleigh_data(ndat,x,f, ne,emin,emax,pe_array,
     *ncbin,fcum,i_array, b_array,c_array)
      implicit none
      integer*4 ndat
      real*8 x(ndat),  f(ndat)
      integer*4 ne
      real*8 emin,  emax,  pe_array(ne)
      integer*4 ncbin
      real*8 fcum(ndat)
      integer*4 i_array(ncbin)
      real*8 b_array(ndat),  c_array(ndat)
      real*8 zero
      real*8 sum0,a,b,x1,x2,pow_x1,pow_x2,dle,e,xmax, anorm,anorm1,anorm
     *2,w,dw,xold,t,aux
      integer*4 i,j,k,ibin
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      write(*,'(a$)') '      preparing data for Rayleigh sampling ... '
      DO 10121 i=1,ndat
        IF((f(i) .EQ. 0.0))f(i) = zero()
10121 CONTINUE
10122 CONTINUE
      sum0=0
      fcum(1)=0
      DO 10131 i=1,ndat-1
        b = log(f(i+1)/f(i))/log(x(i+1)/x(i))
        b_array(i) = b
        x1 = x(i)
        x2 = x(i+1)
        pow_x1 = x1**(2*b)
        pow_x2 = x2**(2*b)
        sum0=sum0+f(i)*f(i)*(x2*x2*pow_x2-x1*x1*pow_x1)/((1+b)*pow_x1)
        fcum(i+1) = sum0
10131 CONTINUE
10132 CONTINUE
      dle = log(emax/emin)/(ne-1)
      i = 1
      DO 10141 j=1,ne
        e = emin*exp(dle*(j-1))
        xmax = 20.607544d0*2*e/prm
        DO 10151 k=i,ndat-1
          IF((xmax .GE. x(k) .AND. xmax .LT. x(k+1)))GO TO10152
10151   CONTINUE
10152   CONTINUE
        i = k
        b = b_array(i)
        x1 = x(i)
        x2 = xmax
        pow_x1 = x1**(2*b)
        pow_x2 = x2**(2*b)
        pe_array(j) = fcum(i) + f(i)*f(i)*(x2*x2*pow_x2-x1*x1*pow_x1)/((
     *  1+b)*pow_x1)
10141 CONTINUE
10142 CONTINUE
      i_array(ncbin) = i
      anorm = 1d0/sqrt(pe_array(ne))
      anorm1 = 1.005d0/pe_array(ne)
      anorm2 = 1d0/pe_array(ne)
      DO 10161 j=1,ne
        pe_array(j) = pe_array(j)*anorm1
        IF((pe_array(j) .GT. 1))pe_array(j) = 1
10161 CONTINUE
10162 CONTINUE
      DO 10171 j=1,ndat
        f(j) = f(j)*anorm
        fcum(j) = fcum(j)*anorm2
        c_array(j) = (1+b_array(j))/(x(j)*f(j))**2
10171 CONTINUE
10172 CONTINUE
      dw = 1d0/(ncbin-1)
      xold = x(1)
      ibin = 1
      b = b_array(1)
      pow_x1 = x(1)**(2*b)
      i_array(1) = 1
      DO 10181 i=2,ncbin-1
        w = dw
10191   CONTINUE
          x1 = xold
          x2 = x(ibin+1)
          t = x1*x1*x1**(2*b)
          pow_x2 = x2**(2*b)
          aux=f(ibin)*f(ibin)*(x2*x2*pow_x2-t)/((1+b)*pow_x1)
          IF (( aux .GT. w )) THEN
            xold = exp(log(t+w*(1+b)*pow_x1/f(ibin)/f(ibin))/(2+2*b))
            i_array(i) = ibin
            GO TO10192
          END IF
          w = w - aux
          xold = x2
          ibin = ibin+1
          b = b_array(ibin)
          pow_x1 = xold**(2*b)
        GO TO 10191
10192   CONTINUE
10181 CONTINUE
10182 CONTINUE
      DO 10201 j=1,ndat
        b_array(j) = 0.5/(1 + b_array(j))
10201 CONTINUE
10202 CONTINUE
      write(*,'(a /)') 'done'
      return
      end
      real*8 function egs_KN_sigma0(e)
      implicit none
      real*8 e
      real*8 con,ko,c1,c2,c3,eps1,eps2
      data con/0.1274783851/
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      ko = e/prm
      IF (( ko .LT. 0.01 )) THEN
        egs_KN_sigma0 = 8.*con/3.*(1-ko*(2-ko*(5.2-13.3*ko)))/prm
        return
      END IF
      c1 = 1./(ko*ko)
      c2 = 1. - 2*(1+ko)*c1
      c3 = (1+2*ko)*c1
      eps2 = 1
      eps1 = 1./(1+2*ko)
      egs_KN_sigma0 = (c1*(1./eps1-1./eps2)+c2*log(eps2/eps1)+eps2*(c3+0
     *.5*eps2)- eps1*(c3+0.5*eps1))/e*con
      return
      end
      real*8 function egs_KN_sigma1(e)
      implicit none
      real*8 e
      real*8 con,ko,c1,c2,c3,eps1,eps2
      data con/0.1274783851/
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      ko = e/prm
      c1 = 1./(ko*ko)
      c2 = 1. - 2*(1+ko)*c1
      c3 = (1+2*ko)*c1
      eps2 = 1
      eps1 = 1./(1+2*ko)
      egs_KN_sigma1 = c1*(1./eps1-1./eps2)
      egs_KN_sigma1 = egs_KN_sigma1 + log(eps2/eps1)*(c2 - c1) - c2*(eps
     *2-eps1)
      egs_KN_sigma1 = egs_KN_sigma1 + c3*(eps2-eps1)*(1-0.5*(eps1+eps2))
      egs_KN_sigma1 = egs_KN_sigma1 + (eps2-eps1)*(0.5*(eps1+eps2)-(eps1
     **eps1+eps2*eps2+eps1*eps2)/3)
      egs_KN_sigma1 = egs_KN_sigma1*con
      return
      end
      subroutine egsi_get_data(flag,iunit,n,ne,zsorted,pz_sorted,ge1,ge0
     *,data)
      implicit none
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      real*8 eth
      integer*4 flag,iunit,n,ne
      real*8 ge1,ge0,zsorted(*),pz_sorted(*),data(*)
      real*8 etmp(2000),ftmp(2000)
      real*8 gle,sig,p,e
      integer*4 i,j,k,kk,iz,iz_old,ndat,iiz
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      rewind(iunit)
      iz_old = 0
      DO 10211 k=1,n
        data(k) = 0
10211 CONTINUE
10212 CONTINUE
      DO 10221 i=1,ne
        iiz = int(zsorted(i)+0.5)
        DO 10231 iz=iz_old+1,iiz
          read(iunit,*,err=10240) ndat
          IF (( ndat .GT. 2000 )) THEN
            write(i_log,'(/a)') '***************** Error: '
            write(i_log,*) 'Too many input data points. Max. is ',2000
            write(i_log,'(/a)') '***************** Quiting now.'
            call exit(1)
          END IF
          IF (( flag .EQ. 0 .OR. flag .EQ. 3)) THEN
            read(iunit,*,err=10240) (etmp(k),ftmp(k),k=1,ndat)
          ELSE
            read(iunit,*,err=10240) (etmp(k+1),ftmp(k+1), k=1,ndat)
            IF (( flag .EQ. 1 )) THEN
              eth = 2*rm
            ELSE
              eth = 4*rm
            END IF
            ndat = ndat + 1
            DO 10251 k=2,ndat
              ftmp(k) = ftmp(k) - 3*log(1-eth/exp(etmp(k)))
10251       CONTINUE
10252       CONTINUE
            ftmp(1) = ftmp(2)
            etmp(1) = log(eth)
          END IF
10231   CONTINUE
10232   CONTINUE
        iz_old = iiz
        DO 10261 k=1,n
          gle = (k - ge0)/ge1
          e = exp(gle)
          IF (( gle .LT. etmp(1) .OR. gle .GE. etmp(ndat) )) THEN
            IF (( flag .EQ. 0 )) THEN
              write(i_log,'(/a)') '***************** Error: '
              write(i_log,*) 'Energy ',exp(gle), ' is outside the availa
     *ble data range of ', exp(etmp(1)),exp(etmp(ndat))
              write(i_log,'(/a)') '***************** Quiting now.'
              call exit(1)
            ELSE IF((flag .EQ. 1 .OR. flag .EQ. 2)) THEN
              IF (( gle .LT. etmp(1) )) THEN
                sig = 0
              ELSE
                sig = exp(ftmp(ndat))
              END IF
            ELSE
              sig = 0
            END IF
          ELSE
            DO 10271 kk=1,ndat-1
              IF((gle .GE. etmp(kk) .AND. gle .LT. etmp(kk+1)))GO TO1027
     *        2
10271       CONTINUE
10272       CONTINUE
            IF (( flag .NE. 3)) THEN
              p = (gle - etmp(kk))/(etmp(kk+1) - etmp(kk))
              sig = exp(p*ftmp(kk+1) + (1-p)*ftmp(kk))
            ELSE
              p = (e - exp(etmp(kk)))/(exp(etmp(kk+1)) - exp(etmp(kk)))
              sig = p*exp(ftmp(kk+1)) + (1-p)*exp(ftmp(kk))
            END IF
          END IF
          IF(((flag .EQ. 1 .OR. flag .EQ. 2) .AND. e .GT. eth))sig = sig
     *    *(1-eth/e)**3
          data(k) = data(k) + pz_sorted(i)*sig
10261   CONTINUE
10262   CONTINUE
10221 CONTINUE
10222 CONTINUE
      return
10240 CONTINUE
      write(i_log,'(/a)') '***************** Error: '
      write(i_log,*) 'Error while reading user photon cross sections fro
     *m unit ', iunit
      write(i_log,'(/a)') '***************** Quiting now.'
      call exit(1)
      return
      end
      subroutine egsi_get_shell_data(imed,n,ne,zsorted,pz_sorted,ge1,ge0
     *,data)
      implicit none
      common /egs_io/ file_extensions(20), file_units(20), user_code,  i
     *nput_file,  output_file, pegs_file,  hen_house,  egs_home,  work_d
     *ir,  host_name,  n_parallel,  i_parallel,  first_parallel, n_max_p
     *arallel, n_chunk,  n_files, i_input,  i_log,  i_incoh,  i_nist_dat
     *a,  i_mscat,  i_photo_cs,  i_photo_relax,  xsec_out,  is_batch,  i
     *s_pegsless
      character input_file*256, output_file*256, pegs_file*256, file_ext
     *ensions*10, hen_house*128, egs_home*128, work_dir*128, user_code*6
     *4, host_name*64
      integer*4 n_parallel, i_parallel, first_parallel,n_max_parallel, n
     *_chunk, file_units, n_files,i_input,i_log,i_incoh, i_nist_data,i_m
     *scat,i_photo_cs,i_photo_relax, xsec_out
      logical is_batch,is_pegsless
      COMMON/BREMPR/ DL1(8,1),DL2(8,1),DL3(8,1),DL4(8,1),DL5(8,1),DL6(8,
     *1), ALPHI(2,1),BPAR(2,1),DELPOS(2,1), WA(1,50),PZ(1,50),ZELEM(1,50
     *),RHOZ(1,50), PWR2I(50), DELCM(1),ZBRANG(1),LZBRANG(1),NNE(1), IBR
     *DST,IPRDST,ibr_nist,pair_nrc,itriplet, ASYM(1,50,2)
      CHARACTER*4 ASYM
      real*8 DL1,DL2,DL3,DL4,DL5,DL6,   ALPHI,  BPAR,  DELPOS,  WA,  PZ,
     *  ZELEM,  RHOZ,  PWR2I,  DELCM,  ZBRANG,  LZBRANG
      integer*4 NNE,  IBRDST,  IPRDST,  ibr_nist,  itriplet,  pair_nrc
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      COMMON/MEDIA/  RLC(1),RLDU(1),RHO(1),MSGE(1),MGE(1),MSEKE(1),MEKE(
     *1),MLEKE(1),MCMFP(1),MRANGE(1),IRAYLM(1),IPHOTONUCM(1), MEDIA(24,1
     *), photon_xsections, comp_xsections, photonuc_xsections,eii_xfile,
     *IPHOTONUC,NMED
      CHARACTER*4 MEDIA
      real*8 RLC,  RLDU,  RHO,  apx, upx
      integer*4 MSGE,  MGE,  MSEKE, MEKE,  MLEKE, MCMFP, MRANGE, IRAYLM,
     *  IPHOTONUCM, IPHOTONUC, NMED
      character*16 eii_xfile
      character*16 photon_xsections
      character*16 comp_xsections
      character*16 photonuc_xsections
      common/pe_shell_data/ pe_xsection(500,100,0:16),  pe_elem_prob(500
     *,100,1),   pe_energy(500,100),  pe_zsorted(100,1), pe_be(100,16),
     * pe_nshell(100),  pe_zpos(100),  pe_nge(100),  pe_ne
      real*8 pe_be, pe_energy, pe_xsection, pe_elem_prob
      integer*4 pe_zsorted, pe_nshell, pe_zpos, pe_nge, pe_ne
      integer*4 n,  ne,  ndat
      real*8 ge1,ge0,zsorted(*),pz_sorted(*),data(*)
      real*8 sigma(500),sigmaMedium
      real*4 etmp(2000),ftmp(2000)
      real*4 gle,sig,p
      integer*4 i,j,k,kk,iz,zpos,imed
      DO 10281 k=1,n
        data(k) = 0
10281 CONTINUE
10282 CONTINUE
      DO 10291 k=1,ne
        sigma(k) = 0
10291 CONTINUE
10292 CONTINUE
      DO 10301 i=1,ne
        iz = int(zsorted(i)+0.5)
        zpos = pe_zpos(iz)
        ndat = pe_nge(zpos)
        DO 10311 k=1,ndat
          pe_elem_prob(k,i,imed) = pz_sorted(i)*pe_xsection(k,zpos,0)
          etmp(k) = pe_energy(k,zpos)
          ftmp(k) = log(pe_xsection(k,zpos,0))
10311   CONTINUE
10312   CONTINUE
        DO 10321 k=1,n
          gle = (k - ge0)/ge1
          IF (( gle .LT. etmp(1) .OR. gle .GE. etmp(ndat) )) THEN
            write(i_log,'(/a)') '***************** Error: '
            write(i_log,*) 'egsi_get_shell_data: Energy ',exp(gle), ' is
     * outside the available data range of ', exp(etmp(1)),exp(etmp(ndat
     *      ))
            write(i_log,'(/a)') '***************** Quiting now.'
            call exit(1)
          ELSE
            DO 10331 kk=1,ndat-1
              IF((gle .GE. etmp(kk) .AND. gle .LT. etmp(kk+1)))GO TO1033
     *        2
10331       CONTINUE
10332       CONTINUE
            p = (gle - etmp(kk))/(etmp(kk+1) - etmp(kk))
            sig = exp(p*ftmp(kk+1) + (1-p)*ftmp(kk))
          END IF
          data(k) = data(k) + pz_sorted(i)*sig
10321   CONTINUE
10322   CONTINUE
10301 CONTINUE
10302 CONTINUE
      DO 10341 i=1,ne
        iz = int(zsorted(i)+0.5)
        zpos = pe_zpos(iz)
        ndat = pe_nge(zpos)
        DO 10351 k=1,ndat
          sig = sigmaMedium(imed,pe_energy(k,zpos))
          pe_elem_prob(k,i,imed) = log(pe_elem_prob(k,i,imed)/sig)
10351   CONTINUE
10352   CONTINUE
10341 CONTINUE
10342 CONTINUE
      return
      end
      real*8 function sigmaMedium(imed, logE)
      implicit none
      COMMON/BREMPR/ DL1(8,1),DL2(8,1),DL3(8,1),DL4(8,1),DL5(8,1),DL6(8,
     *1), ALPHI(2,1),BPAR(2,1),DELPOS(2,1), WA(1,50),PZ(1,50),ZELEM(1,50
     *),RHOZ(1,50), PWR2I(50), DELCM(1),ZBRANG(1),LZBRANG(1),NNE(1), IBR
     *DST,IPRDST,ibr_nist,pair_nrc,itriplet, ASYM(1,50,2)
      CHARACTER*4 ASYM
      real*8 DL1,DL2,DL3,DL4,DL5,DL6,   ALPHI,  BPAR,  DELPOS,  WA,  PZ,
     *  ZELEM,  RHOZ,  PWR2I,  DELCM,  ZBRANG,  LZBRANG
      integer*4 NNE,  IBRDST,  IPRDST,  ibr_nist,  itriplet,  pair_nrc
      common/pe_shell_data/ pe_xsection(500,100,0:16),  pe_elem_prob(500
     *,100,1),   pe_energy(500,100),  pe_zsorted(100,1), pe_be(100,16),
     * pe_nshell(100),  pe_zpos(100),  pe_nge(100),  pe_ne
      real*8 pe_be, pe_energy, pe_xsection, pe_elem_prob
      integer*4 pe_zsorted, pe_nshell, pe_zpos, pe_nge, pe_ne
      real*8 logE, slope, sigma
      integer*4 k,imed,Z,zpos,m,ibsearch
      sigmaMedium = 0
      DO 10361 k=1,nne(imed)
        Z = int( zelem(imed,k) + 0.5 )
        zpos = pe_zpos(Z)
        m = ibsearch(logE,pe_nge(zpos),pe_energy(1,zpos))
        slope = log(pe_xsection(m+1,zpos,0)/pe_xsection(m,zpos,0))
        slope = slope/(pe_energy(m+1,zpos)-pe_energy(m,zpos))
        sigma = log(pe_xsection(m,zpos,0))
        sigma = sigma + slope*(logE - pe_energy(m,zpos))
        sigma = exp(sigma)
        sigmaMedium = sigmaMedium + pz(imed,k)*sigma
10361 CONTINUE
10362 CONTINUE
      return
      end
      subroutine egs_heap_sort(n,rarray,jarray)
      implicit none
      integer*4 n,jarray(*)
      real*8 rarray(*)
      integer*4 i,ir,j,l,ira
      real*8 rra
      DO 10371 i=1,n
        jarray(i)=i
10371 CONTINUE
10372 CONTINUE
      IF((n .LT. 2))return
      l=n/2+1
      ir=n
10381 CONTINUE
        IF ((l .GT. 1)) THEN
          l=l-1
          rra=rarray(l)
          ira=l
        ELSE
          rra=rarray(ir)
          ira=jarray(ir)
          rarray(ir)=rarray(1)
          jarray(ir)=jarray(1)
          ir=ir-1
          IF ((ir .EQ. 1)) THEN
            rarray(1)=rra
            jarray(1)=ira
            return
          END IF
        END IF
        i=l
        j=l+l
10391   CONTINUE
          IF((j .GT. ir))GO TO10392
          IF ((j .LT. ir)) THEN
            IF((rarray(j) .LT. rarray(j+1)))j=j+1
          END IF
          IF ((rra .LT. rarray(j))) THEN
            rarray(i)=rarray(j)
            jarray(i)=jarray(j)
            i=j
            j=j+j
          ELSE
            j=ir+1
          END IF
        GO TO 10391
10392   CONTINUE
        rarray(i)=rra
        jarray(i)=ira
      GO TO 10381
10382 CONTINUE
      return
      end
      SUBROUTINE PHOTONUC
      implicit none
      COMMON/STACK/ E(50),X(50),Y(50),Z(50),U(50),V(50),W(50),DNEAR(50),
     *WT(50),IQ(50),IR(50),LATCH(50), LATCHI,NP,NPold
      DOUBLE PRECISION E
      real*8 X,Y,Z,  U,V,W,  DNEAR,  WT
      integer*4 IQ,  IR,  LATCH,  LATCHI, NP,  NPold
      COMMON/EPCONT/EDEP,EDEP_LOCAL,TSTEP,TUSTEP,USTEP,TVSTEP,VSTEP, RHO
     *F,EOLD,ENEW,EKE,ELKE,GLE,E_RANGE, x_final,y_final,z_final, u_final
     *,v_final,w_final, IDISC,IROLD,IRNEW,IAUSFL(35)
      DOUBLE PRECISION EDEP,  EDEP_LOCAL
      real*8 TSTEP,  TUSTEP,  USTEP,  VSTEP,  TVSTEP,  RHOF,  EOLD,  ENE
     *W,  EKE,  ELKE,  GLE,  E_RANGE, x_final,y_final,z_final,  u_final,
     *v_final,w_final
      integer*4 IDISC,  IROLD,  IRNEW,  IAUSFL
      COMMON/USEFUL/PZERO,PRM,PRMT2,RM,MEDIUM,MEDOLD
      DOUBLE PRECISION PZERO,  PRM,  PRMT2
      real*8 RM
      integer*4 MEDIUM,  MEDOLD
      DATA RM,PRM,PRMT2,PZERO/0.5109989461,0.5109989461,1.0219978922,0.D
     *0/
      npold = np
      edep = pzero
      e(np) = pzero
      wt(np) = 0
      return
      end
C> @endcond
