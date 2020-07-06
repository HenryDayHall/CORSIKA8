c Preprocessed Standard Conex subroutine in 2 file s
c conex_mod8.F for all hadronic model independent part of Conex
c (conex_cors8.F for all hadronic model independent part of Conex)
c   Utilities routines
c   (created by K. Werner; updated by S. Ostapchenko, T. Pierog,
c    V. Chernatkin and D. Heck (Corsika parts))
c Last modifications 03.07.2020 adapt to CORSIKA8 by T.Pierog

#ifdef __STD__
#define __GHEISHA__
#define __QGSJET__
#define __ANALYSIS__
#endif


      subroutine IniModel(model)
      implicit double precision (a-h,o-z)
#include "conex.h"
#include "conex.incnex"

      if(model.eq.1.or.(ilowegy.eq.1.and.MCleModel.eq.1)) then
#ifndef __NEXUS__
         stop'please compile with requested model'
#else
         call IniNexus
#endif
      endif
      if(model.eq.2.or.(ilowegy.eq.1.and.MCleModel.eq.2)) then
#ifndef __QGSJET__
         stop'please compile with requested model'
#else
         call IniQGSjet
#endif
      endif
      if(ilowegy.eq.1.and.MCleModel.eq.3) then
#ifndef __GHEISHA__
         stop'please compile with requested model'
#else
         call IniGheisha
#endif
      endif
      if(model.eq.4.or.(ilowegy.eq.1.and.MCleModel.eq.4)) then
#ifndef __EPOS__
         stop'please compile with requested model'
#else
         call IniEpos
#endif
      endif
      if(model.eq.5) then
#ifndef __SIBYLL21__
         stop'please compile with requested model'
#else
         call IniSibyll
#endif
      endif
      if(model.eq.6.or.(ilowegy.eq.1.and.MCleModel.eq.6)) then
#ifndef __QGSJETII__
         stop'please compile with requested model.'
#else
         call IniQGSJetII
#endif
      endif
      if(ilowegy.eq.1.and.MCleModel.eq.7) then
#ifndef __FLUKA__
         stop'please compile with requested model'
#else
         call IniFluka
#endif
      endif
      if(ilowegy.eq.1.and.MCleModel.eq.8) then
#ifndef __FLUKA__
         stop'please compile with requested model'
#else
         call IniUrQMD
#endif
      endif
      if(model.eq.9.or.(ilowegy.eq.1.and.MCleModel.eq.9)) then
#ifndef __DPMJET__
         stop'please compile with requested model'
#else
         call IniDPMJET
#endif
      endif

      end

      subroutine IniEvtModel(model)
c additional initialization procedure for neXus

#ifdef __NEXUS__
      if(model.eq.1)then
        call IniEvtNex
      endif
#endif

c additional initialization procedure for QGSjet
#ifdef __QGSJET__
      if(model.eq.2)then
        call IniEvtQGS
      endif
#endif

c additional initialization procedure for GHEISHA

#ifdef __GHEISHA__
      if(model.eq.3)then
        call IniEvtGhe
      endif
#endif

c additional initialization procedure for EPOS

#ifdef __EPOS__
      if(model.eq.4)then
        call IniEvtEpo
      endif
#endif

c additional initialization procedure for SIBYLL

#ifdef __SIBYLL21__
      if(model.eq.5)then
        call IniEvtSib
      endif
#endif

c additional initialization procedure for QGSjet
#ifdef __QGSJETII__
      if(model.eq.6)then
        call IniEvtQGSII
      endif
#endif

#ifdef __FLUKA__
      if(model.eq.7)then
        call IniEvtFlu
      endif
#endif

#ifdef __URQMD__
      if(model.eq.8)then
        call IniEvtUrq
      endif
#endif

#ifdef __DPMJET__
      if(model.eq.9)then
        call IniEvtDPM
      endif
#endif

      end

      subroutine emsModel(modelxs,iret)
      implicit none
      integer modelxs,iret
#ifdef __NEXUS__
        if(modelxs.eq.1)call emsnex(iret)           !nexus
#endif
#ifdef __QGSJET__
        if(modelxs.eq.2)call emsqgs(iret)           !QGSjet
#endif
#ifdef __GHEISHA__
        if(modelxs.eq.3)call emsghe(iret)           !GHEISHA
#endif
#ifdef __EPOS__
        if(modelxs.eq.4)call emsepo(iret)           !EPOS
#endif
#ifdef __SIBYLL21__
        if(modelxs.eq.5)call emssib(iret)           !SIBYLL
#endif
#ifdef __QGSJETII__
        if(modelxs.eq.6)call emsqgsII(iret)         !QGSjet-II
#endif
#ifdef __FLUKA__
        if(modelxs.eq.7)call emsflu(iret)           !FLUKA
#endif
#ifdef __URQMD__
        if(modelxs.eq.8)call emsurq(iret)           !URQMD
#endif
#ifdef __DPMJET__
        if(modelxs.eq.9)call emsdpm(iret)           !DPMJET
#endif
      end


      subroutine MODELSIGMA(model,np,ek,am,Siginemb)
      implicit double precision (a-h,o-z)
#include "conex.h"
#include "conexep.h"
#include "conex.incnex"
      Siginemb=0d0
      e=ek
      et=ek+am
      pl=sqrt(max(0.d0,(et+am)*(et-am)))
      if(model.eq.0)then                !mixed model
#ifdef __MODEL__
        Ipar=np
        iclproxs=2                     !nucleons
        if(np.eq.2.or.np.eq.6)then
          iclproxs=1                   !pions
          Ipar=2
        elseif(np.ge.3.and.np.le.5)then
          iclproxs=3                   !kaons
          Ipar=3
        elseif(np.eq.7)then
          Ipar=1
        endif
        call EPOSIGMA(Ipar,e,Siginemb1)  !EPOS cs called with kinetic energy
        call QGSSIGMAII(Ipar,et,Siginemb2)  !qgsjet-II cs called with total energy
        Siginemb=0.5*(Siginemb1+Siginemb2)
#else
        stop'please compile with requested model (MODELSIGMA)'
#endif
#ifdef __NEXUS__
      elseif(model.eq.1)then                !neXus
        Ipar=np
        iclproxs=2                     !nucleons
        if(np.eq.2.or.np.eq.6)then
          iclproxs=1                   !pions
          Ipar=2
        elseif(np.ge.3.and.np.le.5)then
          iclproxs=3                   !kaons
          Ipar=3
        elseif(np.eq.7)then
          Ipar=1
        endif
        call NXSSIGMA(Ipar,e,Siginemb)  !nexus cs called with kinetic energy
#endif
#ifdef __QGSJET__
      elseif(model.eq.2)then             !QGSJet
        Ipar=np
        iclproxs=2                     !nucleons
        if(np.eq.2.or.np.eq.6)then
          iclproxs=1                   !pions
          Ipar=2
        elseif(np.ge.3.and.np.le.5)then
          iclproxs=3                   !kaons
          Ipar=3
        elseif(np.eq.7)then
          Ipar=1
        endif
        call QGSSIGMA(Ipar,et,Siginemb)  !qgsjet cs called with total energy
#endif
#ifdef __GHEISHA__
      elseif(model.eq.3)then           !Gheisha
        Ipar=np
        if(np.eq.6)Ipar=2
        if(np.eq.7)Ipar=1
        call GHESIGMA(Ipar,e,Siginemb,SIGEL) !Gheisha cs called with kinetic energy
#endif
#ifdef __EPOS__
      elseif(model.eq.4)then                !EPOS
        Ipar=np
        iclproxs=2                     !nucleons
        if(np.eq.2.or.np.eq.6)then
          iclproxs=1                   !pions
          Ipar=2
        elseif(np.ge.3.and.np.le.5)then
          iclproxs=3                   !kaons
          Ipar=3
        elseif(np.eq.7)then
          Ipar=1
        endif
        call EPOSIGMA(Ipar,e,Siginemb)  !EPOS cs called with kinetic energy
#endif
#ifdef __SIBYLL21__
      elseif(model.eq.5)then           !Sibyll
        Ipar=np
        call SIBSIGMA(Ipar,et,Siginemb) !Sibyll cs called with total energy
#endif
#ifdef __QGSJETII__
      elseif(model.eq.6)then           !QGSJet-II
        Ipar=np
        iclproxs=2                     !nucleons
        if(np.eq.2.or.np.eq.6)then
          iclproxs=1                   !pions
          Ipar=2
        elseif(np.ge.3.and.np.le.5)then
          iclproxs=3                   !kaons
          Ipar=3
        elseif(np.eq.7)then
          Ipar=1
        endif
        call QGSSIGMAII(Ipar,et,Siginemb)  !qgsjet-II cs called with total energy
#endif
#ifdef __FLUKA__
      elseif(model.eq.7)then           !FLUKA
        pl=sqrt(max(0.d0,(et+am)*(et-am)))
        Ipar=np
        iclproxs=2                     !nucleons
        if(np.eq.2.or.np.eq.6)then
          iclproxs=1                   !pions
        elseif(np.ge.3.and.np.le.5)then
          iclproxs=3                   !kaons
        endif
        call FLUSIGMA(Ipar,e,pl,Siginemb)  !FLUKA cs called with kinetic energy and momentum
#endif
#ifdef __URQMD__
      elseif(model.eq.8)then           !URQMD
        Ipar=np
        call URQSIGMA(Ipar,e,Siginemb)  !URQMD cs called with kinetic energy
#endif
#ifdef __DPMJET__
      elseif(model.eq.9)then           !DPMJET
        iclproxs=2                     !nucleons
        if(np.eq.2.or.np.eq.6)then
          iclproxs=1                   !pions
          Ipar=2
        elseif(np.ge.3.and.np.le.5)then
          iclproxs=3                   !kaons
          Ipar=3
        elseif(np.eq.7)then
          Ipar=1
        else
          Ipar=np
        endif
        call DPMSIGMA(Ipar,e,Siginemb)  !DPMJET cs called with kinetic energy
#endif
      else
        stop'please compile with requested model (MODELSIGMA)'
      endif
      end
      

#ifdef __NEXUS__

C-----------------------------------------------------------------------
      SUBROUTINE NXSSIGMA(Ipar,Ekin,SIGINEL)
C-----------------------------------------------------------------------
C     neXus3 cross sections                      (RE 06/03)
C
C     input:  Ipar     1        p/n-air
C                      2        pi-air
C                      3        K-air
C                      >10      nucleus (A=Ipar/10) - air
C             Ekin     projectile kinetic energy (GeV)
C
C     output: SIGINEL  inelastic cross section (mb)
C-----------------------------------------------------------------------
      implicit double precision (a-h,o-z)



      if(Ipar.le.3)then

        SIGINEL =  cxnxscrse(Ekin,1,0,0)

      elseif(Ipar.ge.10)then            !Nucleus

        ma=Ipar/10
        SIGINEL =  cxnxscrse(Ekin,ma,0,0)

      else

        write(*,*)'Particle',Ipar
        stop'No neXus cross section for this particle'

      endif


      END
#endif

#ifdef __QGSJET__

C-----------------------------------------------------------------------
      SUBROUTINE QGSSIGMA(Ipar,Etot,SIGINEL)
C-----------------------------------------------------------------------
C     QGSjet03 cross sections                      (RE 06/03)
C
C     input:  Ipar     1        p/n-air
C                      2        pi-air
C                      3        K-air
C                      >10      nucleus (A=Ipar/10) - air
C             Etot     projectile total energy (GeV)
C
C     output: SIGINEL  inelastic cross section (mb)
C-----------------------------------------------------------------------
      implicit double precision (a-h,o-z)


      if(Ipar.le.3)then

        SIGINEL = qgscrse(Etot,1,0,0)

      else if(Ipar.ge.10)then            !Nucleus

        ma=Ipar/10
        SIGINEL = qgscrse(Etot,ma,0,0)

      else

        write(*,*)'Particle',Ipar
        stop'No QGSJet cross section for this particle'

      endif

      END

#endif

#ifdef __GHEISHA__
C-----------------------------------------------------------------------
      SUBROUTINE GHESIGMA(Idi,Ekin,SIGINEL,SIGEL)
C-----------------------------------------------------------------------
C     Gheisha cross sections                      (RE 06/03)
C
C     input:  Id       1        p/n-air
C                      2        pi-air ((pip+pim)/2)
C                      3        K-air ((Kp+Km)/2)
C                      4        Kl-air
C                      5        Ks-air
C                      >10      nucleus (A=Ipar/10) - air
C             Ekin     projectile kinetic energy (GeV)
C
C     output: SIGINEL  inelastic cross section (mb)
C             (iquasiel=0 : without quasi-elastic, iquasiel=1, with)
C-----------------------------------------------------------------------
      implicit double precision (a-h,o-z)
      common/ghecsquel/anquasiel,iquasiel
      real anquasiel
      real GHETABCX(0:10,21)
      data (GHETABCX(0,k),k=1,21) /
     &  0.255860E+03,0.281492E+03,0.295567E+03,0.144902E+03,0.224367E+03
     & ,0.243389E+03,0.163012E+03
     & ,0.255860E+03,33.9845428,37.1037254,18.7059803,32.0806122
     & ,33.2575989,22.2932968
     & ,0.176698E+03,0.133947E+03,0.133947E+03,0.779889E+02,0.728328E+02
     & ,0.726277E+02,0.778469E+02 /
      data (GHETABCX(1,k),k=1,21) /
     &  0.258983E+03,0.266648E+03,0.279980E+03,0.141426E+03,0.234702E+03
     & ,0.254316E+03,0.159121E+03
     & ,177.488327,21.369585,24.2307854,8.23070812,22.1758099
     & ,21.7302551,11.6256256
     & ,0.156630E+03,0.139122E+03,0.139122E+03,0.655561E+02,0.612038E+02
     & ,0.611662E+02,0.654509E+02 /
      data (GHETABCX(2,k),k=1,21) /
     &  0.267444E+03,0.263215E+03,0.276376E+03,0.142543E+03,0.236345E+03
     & ,0.255972E+03,0.160372E+03
     & ,0.79815197,3.7912941,4.9971962,1.24174988,3.74159217
     & ,5.56045818,2.3606503
     & ,0.144324E+03,0.132730E+03,0.574985E+03,0.607903E+02,0.572235E+02
     & ,0.571501E+02,0.574256E+02 /
      data (GHETABCX(3,k),k=1,21) /
     &  0.277400E+03,0.259933E+03,0.272929E+03,0.143490E+03,0.227779E+03
     & ,0.297157E+03,0.161437E+03
     & ,1.07984018,2.40868378,1.89947307,0.138723105,1.1398896
     & ,0.499438465,0.820548415
     & ,0.129844E+03,0.119041E+03,0.119041E+03,0.504331E+02,0.505850E+02
     & ,0.505178E+02,0.503575E+02 /
      data (GHETABCX(4,k),k=1,21) /
     &  0.280586E+03,0.256632E+03,0.269463E+03,0.144764E+03,0.262507E+03
     & ,0.236638E+03,0.162875E+03
     & ,1.35714746,1.60609698,1.82574916,0.421879053,0.650433302
     & ,0.971257389,0.646709323
     & ,0.122093E+03,0.106377E+03,0.106377E+03,0.445833E+02,0.439863E+02
     & ,0.439316E+02,0.445313E+02 /
      data (GHETABCX(5,k),k=1,21) /
     &  0.280586E+03,0.252974E+03,0.265623E+03,0.146090E+03,0.211412E+03
     & ,0.228997E+03,0.164368E+03
     & ,0.825052261,0.267928243,1.04222524,0.734463155,1.49724424
     & ,0.232366323,0.334274232
     & ,0.116319E+03,0.939736E+02,0.939736E+02,0.398568E+02,0.388705E+02
     & ,0.388460E+02,0.398301E+02 /
      data (GHETABCX(6,k),k=1,21) /
     &  0.280586E+03,0.246789E+03,0.259129E+03,0.146254E+03,0.208417E+03
     & ,0.225764E+03,0.164535E+03
     & ,0.554720938,0.262696952,0.516794324,0.285888463,0.411290705
     & ,0.,0.33578822
     & ,0.109096E+03,0.868448E+02,0.868448E+02,0.358476E+02,0.355725E+02
     & ,0.355502E+02,0.358203E+02 /
      data (GHETABCX(7,k),k=1,21) /
     &  0.280586E+03,0.244759E+03,0.256997E+03,0.150972E+03,0.211365E+03
     & ,0.228963E+03,0.169843E+03
     & ,0.,0.,0.,0.,0.,0.,0.
     & ,0.107451E+03,0.849422E+02,0.849422E+02,0.326919E+02,0.334941E+02
     & ,0.334845E+02,0.326771E+02 /
      data (GHETABCX(8,k),k=1,21) /
     &  0.280586E+03,0.206607E+03,0.216938E+03,0.158192E+03,0.197834E+03
     & ,0.214317E+03,0.177968E+03
     & ,0.,0.,0.,0.,0.,0.,0.
     & ,0.918145E+02,0.637644E+02,0.637644E+02,0.243442E+02,0.231964E+02
     & ,0.231947E+02,0.243439E+02 /
      data (GHETABCX(9,k),k=1,21) /
     &  0.280586E+03,0.205548E+03,0.215825E+03,0.165259E+03,0.191394E+03
     & ,0.207343E+03,0.185917E+03
     & ,0.,0.,0.,0.,0.,0.,0.
     & ,0.872840E+02,0.525249E+02,0.525249E+02,0.229987E+02,0.194206E+02
     & ,0.194205E+02,0.229985E+02 /
      data (GHETABCX(10,k),k=1,21) /
     &  0.280586E+03,0.204708E+03,0.214943E+03,0.178999E+03,0.188278E+03
     & ,0.203968E+03,0.201374E+03
     & ,0.,0.,0.,0.,0.,0.,0.
     & ,0.834731E+02,0.370992E+02,0.370992E+02,0.197360E+02,0.185518E+02
     & ,0.185518E+02,0.197360E+02 /
      SAVE


      Id=Idi
      if(Id.eq.100)Id=1

      if(Id.le.5)then

      Ipar=Id
      if(Ekin.ge.3.7d0)then
        al = log(Ekin/3.7037d0)/1.098612289d0  !ln(Ekin/3.7037)/ln(3) (4 bins between 3.7 and 100 Gev)
        al = al+7.d0
        i1 = int(al)
      else
        al = log(Ekin)/0.18310205d0     !ln(Ekin)/ln(3)/6 (7 bins between 1 and 3 Gev)
        i1 = int(al)
      endif
      if(i1.lt.0) then
        i1 = 0
      else if(i1.gt.9) then              !crazy xs above 100 GeV
        i1 = 9
      endif
      i2 = i1+1
      dl =min(1.d0, al - i1)
      dl1=max(0.d0,1.d0-dl)


      if(iquasiel.eq.0)then

c Inelastic = Inelastic from gheisha - quasi-elastic !

      if(Ipar.eq.1)then
      SIGINEL = dble(GHETABCX(i2,Ipar)-GHETABCX(i2,Ipar+7))*dl
     &        + dble(GHETABCX(i1,Ipar)-GHETABCX(i1,Ipar+7))*dl1
      SIGEL = dble(GHETABCX(i2,Ipar+14))*dl + dble(GHETABCX(i1,Ipar+14))
     &        *dl1
      elseif(Ipar.eq.2)then
      SIGINEL = 0.5d0*(dble(GHETABCX(i2,Ipar)+GHETABCX(i2,Ipar+1)
     &                     -GHETABCX(i2,Ipar+7)-GHETABCX(i2,Ipar+8))*dl
     &        + dble(GHETABCX(i1,Ipar)+GHETABCX(i1,Ipar+1)
     &              -GHETABCX(i1,Ipar+7)-GHETABCX(i1,Ipar+8))*dl1)
      SIGEL = 0.5d0*(dble(GHETABCX(i2,Ipar+14)+GHETABCX(i2,Ipar+15))*dl
     &      + dble(GHETABCX(i1,Ipar+14)+GHETABCX(i1,Ipar+15))*dl1)
      elseif(Ipar.eq.3)then
      SIGINEL = 0.5d0*(dble(GHETABCX(i2,Ipar+1)+GHETABCX(i2,Ipar+2)
     &                     -GHETABCX(i2,Ipar+8)-GHETABCX(i2,Ipar+9))*dl
     &        + dble(GHETABCX(i1,Ipar+1)+GHETABCX(i1,Ipar+2)
     &              -GHETABCX(i1,Ipar+8)-GHETABCX(i1,Ipar+9))*dl1)
      SIGEL = 0.5d0*(dble(GHETABCX(i2,Ipar+15)+GHETABCX(i2,Ipar+16))*dl
     &      + dble(GHETABCX(i1,Ipar+15)+GHETABCX(i1,Ipar+16))*dl1)
      elseif(Ipar.ge.4)then
      Ipar=Ipar+2
      SIGINEL = dble(GHETABCX(i2,Ipar)-GHETABCX(i2,Ipar+7))*dl
     &        + dble(GHETABCX(i1,Ipar)-GHETABCX(i1,Ipar+7))*dl1
      SIGEL = dble(GHETABCX(i2,Ipar+14))*dl + dble(GHETABCX(i1,Ipar+14))
     &        *dl1
      endif

      else

c Inelastic = Inelastic from gheisha !

      if(Ipar.eq.1)then
      SIGINEL = dble(GHETABCX(i2,Ipar))*dl
     &        + dble(GHETABCX(i1,Ipar))*dl1
      SIGEL = dble(GHETABCX(i2,Ipar+14))*dl + dble(GHETABCX(i1,Ipar+14))
     &        *dl1
      elseif(Ipar.eq.2)then
      SIGINEL = 0.5d0*(dble(GHETABCX(i2,Ipar)+GHETABCX(i2,Ipar+1)
     &                     )*dl
     &        + dble(GHETABCX(i1,Ipar)+GHETABCX(i1,Ipar+1)
     &              )*dl1)
      SIGEL = 0.5d0*(dble(GHETABCX(i2,Ipar+14)+GHETABCX(i2,Ipar+15))*dl
     &      + dble(GHETABCX(i1,Ipar+14)+GHETABCX(i1,Ipar+15))*dl1)
      elseif(Ipar.eq.3)then
      SIGINEL = 0.5d0*(dble(GHETABCX(i2,Ipar+1)+GHETABCX(i2,Ipar+2)
     &                     )*dl
     &        + dble(GHETABCX(i1,Ipar+1)+GHETABCX(i1,Ipar+2)
     &              )*dl1)
      SIGEL = 0.5d0*(dble(GHETABCX(i2,Ipar+15)+GHETABCX(i2,Ipar+16))*dl
     &      + dble(GHETABCX(i1,Ipar+15)+GHETABCX(i1,Ipar+16))*dl1)
      elseif(Ipar.ge.4)then
      Ipar=Ipar+2
      SIGINEL = dble(GHETABCX(i2,Ipar))*dl
     &        + dble(GHETABCX(i1,Ipar))*dl1
      SIGEL = dble(GHETABCX(i2,Ipar+14))*dl + dble(GHETABCX(i1,Ipar+14))
     &        *dl1
      endif

      endif

      else                      !Nucleus

      if(Id.eq.10)then                !nucleon
        Ipar=1120
      elseif(Id.eq.20)then                !deuterium
        Ipar=17
      elseif(Id.eq.30)then            !tritium
        Ipar=18
      elseif(Id.eq.40)then            !alpha
        Ipar=19
      else       !should not be call (no cs for nucleus in gheisha)
        write(*,*)'Nucleus-air cross section request for Gheisha !!!'
     &,id,Ekin
        write(*,*)'Energy too low for nucleus primary collision ...'
        siginel=1e-30
        sigel=1e-30
        return
      endif

      call ghecrse(Ekin,Ipar,0,7,14,sigi,sige)
      SIGINEL = sigi
      SIGEL = sige

      endif


      END
#endif


#ifdef __EPOS__

C-----------------------------------------------------------------------
      SUBROUTINE EPOSIGMA(Ipar,Ekin,SIGINEL)
C-----------------------------------------------------------------------
C     EPOS cross sections                      (TP 06/0t)
C
C     input:  Ipar     1        p/n-air
C                      2        pi-air
C                      3        K-air
C                      >10      nucleus (A=Ipar/10) - air
C             Ekin     projectile kinetic energy (GeV)
C
C     output: SIGINEL  inelastic cross section (mb)
C-----------------------------------------------------------------------
      implicit double precision (a-h,o-z)



      if(Ipar.le.3)then

        SIGINEL =  cxepocrse(Ekin,1,0,0)

      elseif(Ipar.ge.10)then            !Nucleus

        ma=Ipar/10
        SIGINEL =  cxepocrse(Ekin,ma,0,0)

      else

        write(*,*)'Particle',Ipar
        stop'No EPOS cross section for this particle'

      endif


      END
#endif

#ifdef __SIBYLL21__

C-----------------------------------------------------------------------
      SUBROUTINE SIBSIGMA(Ipari,Etot,SIGINEL)
C-----------------------------------------------------------------------
C     Sibyll 2.1 cross sections
C
C     input:  Ipar    1 : proton / air
c                     2 : charged pions / air
c                     3 : charged kaons / air
c                     4 : K-long / air
c                     5 : K-short / air
c                     6 : neutral pion / air
c                     7 : neutron / air
c                   >10 : nucleus (A=Ipar/10) / air
C             Etot     projectile total energy (GeV)
C
C     output: SIGINEL  inelastic cross section (mb)
C-----------------------------------------------------------------------
      implicit double precision (a-h,o-z)
      dimension isibcx(7)
      data isibcx/13,7,9,11,12,6,14/          !conversion from Ipar to Sibyll code

      Ipar=Ipari
      ma=Ipar/10
      if(ma.eq.1)Ipar=ma

      if(Ipar.le.7)then

        SIGINEL =  sibcrse(Etot,1,isibcx(Ipar))

      elseif(Ipar.ge.10)then            !Nucleus

        SIGINEL =  sibcrse(Etot,ma,0)

      else

        write(*,*)'Particle',Ipar
        stop'No Sibyll cross section for this particle'

      endif


      END
#endif

#ifdef __QGSJETII__

C-----------------------------------------------------------------------
      SUBROUTINE QGSSIGMAII(Ipar,Etot,SIGINEL)
C-----------------------------------------------------------------------
C     QGSjet-II cross sections                      (RE 06/03)
C
C     input:  Ipar     1        p/n-air
C                      2        pi-air
C                      3        K-air
C                      >10      nucleus (A=Ipar/10) - air
C             Etot     projectile total energy (GeV)
C
C     output: SIGINEL  inelastic cross section (mb)
C-----------------------------------------------------------------------
      implicit double precision (a-h,o-z)


      if(Ipar.le.3)then

        SIGINEL = qgsIIcrse(Etot,1,0,0)

      else if(Ipar.ge.10)then            !Nucleus

        ma=Ipar/10
        SIGINEL = qgsIIcrse(Etot,ma,0,0)

      else

        write(*,*)'Particle',Ipar
        stop'No QGSJet-II cross section for this particle'

      endif


      END

#endif

#ifdef __DPMJET__

C-----------------------------------------------------------------------
      SUBROUTINE DPMSIGMA(Ipar,Etot,SIGINEL)
C-----------------------------------------------------------------------
C     DPMJETIII cross sections
C
C     input:  Ipar    1 : proton / air
c                     2 : charged pions / air
c                     3 : charged kaons / air
c                     4 : K-long / air
c                     5 : K-short / air
c                     6 : neutral pion / air
c                     7 : neutron / air
c                   >10 : nucleus (A=Ipar/10) / air
C             Etot   projectile lab energy per nucleon (GeV)
C
C     output: SIGINEL  inelastic cross section (mb)
C-----------------------------------------------------------------------
      implicit none
      double precision Etot,SIGINEL,dpmcrse
      integer Ipar,ma

      ma=max(1,Ipar/10)

      SIGINEL = dpmcrse(Etot,ma,0)


      END
#endif

#ifdef __FLUKA__

C-----------------------------------------------------------------------
      SUBROUTINE FLUSIGMA(Ipar,Ek,Pl,SIGINEL)
C-----------------------------------------------------------------------
C     FLUKA cross sections                      (TP 09/11)
C
C     input:  Ipar     CONEX simple Particle type
C                      >10      nucleus (A=Ipar/10) - air
C             Etot     projectile kinetic energy (GeV) per nucleon
C             Pl       projectile momentum (GeV/c) per nucleon
C
C     output: SIGINEL  inelastic cross section (mb)
C-----------------------------------------------------------------------
      implicit none
#include "conex.incnex"
      integer icnx2nxs(7),Ipar,ma
      double precision siginel,Ek,Pl,fflucrse,flucrse
      data icnx2nxs/1120, 120, 130, -20, 20, 110, 1220/


      if(Ipar.le.7)then

        xsekin=Ek
        xspnll=Pl
        idprojxs=icnx2nxs(Ipar)
        SIGINEL = fflucrse()
        if(Ipar.eq.2.or.Ipar.eq.3)then   !average between + and -
          idprojxs=-icnx2nxs(Ipar)
          SIGINEL = 0.5d0*(SIGINEL+fflucrse())
        endif

      else if(Ipar.ge.10)then            !Nucleus

        ma=Ipar/10
        SIGINEL = flucrse(Ek,ma,0,0)

      else

        write(*,*)'Particle',Ipar
        stop'No FLUKA cross section for this particle'

      endif


      END

#endif

#ifdef __URQMD__

C-----------------------------------------------------------------------
      SUBROUTINE URQSIGMA(Ipari,Ek,SIGINEL)
C-----------------------------------------------------------------------
C     UrQMD 1.3 cross sections
C
C     input:  Ipar    1 : proton / air
c                     2 : charged pions / air
c                     3 : charged kaons / air
c                     4 : K-long / air
c                     5 : K-short / air
c                     6 : neutral pion / air
c                     7 : neutron / air
c                   >10 : nucleus (A=Ipar/10) / air
C             Ek     projectile kinetic energy (GeV)
C
C     output: SIGINEL  inelastic cross section (mb)
C-----------------------------------------------------------------------
      implicit none
      double precision Ek,SIGINEL,urqcrse
      integer icnx2nxs(7),Ipar,Ipari,ma,idpj
      data icnx2nxs/1120, 120, 130, -20, 20, 110, 1220/

      SIGINEL=0d0
      Ipar=Ipari
      ma=Ipar/10
      if(ma.eq.1)Ipar=ma

      if(Ipar.le.7)then

        idpj=icnx2nxs(Ipar)
        SIGINEL = urqcrse(Ek,idpj)
        if(Ipar.le.3.or.Ipar.eq.7)then !average between + and -
          idpj=-icnx2nxs(Ipar)
          SIGINEL = 0.5d0*(SIGINEL+urqcrse(Ek,idpj))
        endif

      else

        write(*,*)'Particle',Ipar
        stop'No URQMD cross section for this particle'

      endif

      END
#endif

#ifndef __CORSIKA8__
      
#ifdef __NEXUS__
c-----------------------------------------------------------------------
      function rangen()
c-----------------------------------------------------------------------
c generates a safe random number simple precision
c (for NEXUS : no argument)
c-----------------------------------------------------------------------
      double precision dummy,drangen
 1    rangen=real(drangen(dummy))
      if(rangen.le.0..or.rangen.ge.1.)goto 1

      return
      end

#endif
#ifdef __EPOS__
c-----------------------------------------------------------------------
      function rangen()
c-----------------------------------------------------------------------
c generates a safe random number simple precision
c (for NEXUS : no argument)
c-----------------------------------------------------------------------
      double precision dummy,drangen
 1    rangen=real(drangen(dummy))
      if(rangen.le.0..or.rangen.ge.1.)goto 1

      return
      end

#endif

#endif
#ifdef __URQMD__
c 18.11.2011 Link routines between UrQMD 1.3 and CONEX.
c author T. Pierog based on CORSIKA and EPOS link to UrQMD

#ifdef __STD__
#define __GHEISHA__
#define __QGSJET__
#define __ANALYSIS__
#endif


c-----------------------------------------------------------------------
      subroutine IniUrQMD
c-----------------------------------------------------------------------
c Primary initialization for UrQMD 1.31
c-----------------------------------------------------------------------
      implicit none
c CONEX includes
#include "conex.h"
#include "conex.incnex"
#if !__CXCORSIKA__ && !__CORSIKA8__

      character*500 furqdat
      integer ifurqdat, nfurqdat
      common/urqfname/  furqdat, ifurqdat, nfurqdat

      include 'boxinc.f'
      include 'inputs.f'
      include 'options.f'

c commons from coms.f
      integer Ap, At, Zp, Zt, npart, nbar, nmes, ctag
      integer nsteps,ranseed,event,eos,dectag,uid_cnt
      integer NHardRes,NSoftRes,NDecRes,NElColl,NBlColl
      common /sys/ npart, nbar, nmes, ctag,nsteps,uid_cnt,
     +             ranseed,event,Ap,At,Zp,Zt,eos,dectag,
     +             NHardRes,NSoftRes,NDecRes,NElColl,NBlColl

c local
      INTEGER          i,io,ia,ie,id
      CHARACTER        CTPStrg(numctp)*60, CTOStrng(numcto)*60
      integer mxie,mxid,mxia
      parameter (mxie=41,mxid=10,mxia=3)
      character adum
      double precision sig_u1,ekdummy
      integer iamaxu,idmaxu,iemaxu
      common /cxs_u1/ sig_u1(mxie,mxid,mxia),iamaxu,idmaxu,iemaxu
      double precision xs(3),bim(3)
      common /cxs_u2/ xs
      integer iudebug
      data bim/6.d0,6.d0,7.d0/
      integer init
      data init/0/
      SAVE

      if(init.ge.1)return
      init=init+1
#ifdef __CXDEBUG__
      call utisx1('iniurqmd  ',4)
      write(*,'(a)')'initialize URQMD ...'
#endif

C-----------------------------------------------------------------------

      IF ( isx.ge.2 ) THEN
        IUDEBUG = isx-1
      ELSE
        IUDEBUG = 0
      ENDIF

      WRITE (*,*)
     $   '############################################################'
      WRITE (*,*)
     $   '##                                                        ##'
      WRITE (*,*)
     $   '##     UrQMD 1.3.1  University of Frankfurt               ##'
      WRITE (*,*)
     $   '##                  urqmd@th.physik.uni-frankfurt.de      ##'
      WRITE (*,*)
     $   '##                                                        ##'
      WRITE (*,*)
     $   '############################################################'
      WRITE (*,*)
     $   '##                                                        ##'
      WRITE (*,*)
     $   '##     please cite when using this model:                 ##'
      WRITE (*,*)
     $   '##     S.A.Bass et al. Prog.Part.Nucl.Phys. 41 (1998) 225 ##'
      WRITE (*,*)
     $   '##     M.Bleicher et al. J.Phys. G25  (1999) 1859         ##'
      WRITE (*,*)
     $   '##                                                        ##'
      WRITE (*,*)
     $   '############################################################'

C  SET THE 'LARGE' CROSS-SECTIONS FOR ALL 3 TARGET ELEMENTS
      DO  I = 1, 3
        XS(I) = 10.D0 * PI * BIM(I)**2
      ENDDO

C  SET NMAX TO DEFAULT VALUE
      call set0
      call params

C  THIS IS THE SUBSTITUE FOR THE URQMD INPUT ROUTINE
C  INITIALIZE COUNTERS
      boxflag = 0
      mbflag  = 0
      edens   = 0.d0
      para    = 0
      solid   = 0
      mbox    = 0
      io      = 0

C  THE FOLLOWING FLAGS CHECK, WHETHER ALL NECESSARY INPUT IS GIVEN
C  PROJECTILE
      prspflg = 0
C  TARGET
      trspflg = 0
C
      srtflag = 0
      firstev = 0
C  EXCITATION FUNCTION
      nsrt    = 1
      npb     = 1
      efuncflag = 0
C  DEFAULT NUMBER OF EVENTS
      nevents = 1
C  DEFAULT NUMBER OF TIMESTEPS
      nsteps  = 1000

C  SKIP CONDITIONS ON UNIT 13, 14, 15, 16 & 18
C  SUPPRESS ALL OUTPUT
      bf13 = .true.
      bf14 = .true.
      bf15 = .true.
      bf16 = .true.
      bf18 = .true.
      bf19 = .true.
      bf20 = .true.
C  SET DEBUG OUTPUT DEPENDING ON CHOSEN DEBUG LEVEL
C  SET THE OUTPUT OF UNITS 13, 14, 15 TO THE DEBUG OUTPUT UNIT
      IF     ( IUDEBUG .EQ. 1 ) THEN
        bf13 = .true.
        bf14 = .false.
        call uounit(14,IFCK)
        bf15 = .true.
      ELSEIF ( IUDEBUG .EQ. 2 ) THEN
        bf13 = .false.
        call uounit(13,IFCK)
        bf14 = .true.
        bf15 = .true.
      ELSEIF ( IUDEBUG .GT. 2 ) THEN
        bf13 = .true.
        bf14 = .true.
        bf15 = .false.
        call uounit(15,IFCK)
      ENDIF
      do  i = 1, numcto
         CTOdc(i) = '  '
      enddo
      do  i = 1, numctp
         CTPdc(i) = '  '
      enddo
      do  i = 1, maxstables
         stabvec(i) = 0
      enddo
      nstable = 0

C  DEFAULT SETTINGS FOR CTParam AND CTOption
C  DEFAULT SETTINGS FOR CTParam
      CTParam(1)=1.d0
      CTPStrg(1)='scaling factor for decay-width'
      CTParam(2)=0.52d0
      CTPStrg(2)='used for minimal stringmass & el/inel cut in makestr'
      CTParam(3)=2.d0
      CTPStrg(3)='velocity exponent for modified AQM'
      CTParam(4)=0.3d0
      CTPStrg(4)='transverse pion mass, used in make22 & strexct'
      CTParam(5)=0.d0
      CTPStrg(5)='probabil. for quark rearrangement in cluster'
      CTParam(6)=0.37d0
      CTPstrg(6)='strangeness probability'
      CTParam(7)=0.d0
      CTPStrg(7)='charm probability (not yet implemented in UQMD)'
      CTParam(8)=0.093d0
      CTPStrg(8)='probability to create a diquark'
      CTParam(9)=0.35d0
      CTPStrg(9)='kinetic energy cut off for last string break'
      CTParam(10)=0.25d0
      CTPStrg(10)='min. kinetic energy for hadron in string'
      CTParam(11)=0.d0
      CTPStrg(11)='fraction of non groundstate resonances'
      CTParam(12)=.5d0
      CTPStrg(12)='probability for rho 770 in String'
      CTParam(13)=.27d0
      CTPStrg(13)='probability for rho 1450 (rest->rho1700)'
      CTParam(14)=.49d0
      CTPStrg(14)='probability for omega 782'
      CTParam(15)=.27d0
      CTPStrg(15)='probability for omega 1420(rest->om1600)'
      CTParam(16)=1.0d0
      CTPStrg(16)='mass cut betw. rho770 and rho 1450'
      CTParam(17)=1.6d0
      CTPSTRG(17)='mass cut betw. rho1450 and rho1700'
      CTParam(18)=.85d0
      CTPStrg(18)='mass cut betw. om 782 and om1420'
      CTParam(19)=1.55d0
      CTPStrg(19)='mass cut betw. om1420 and om1600'
      CTParam(20)=0.0d0
      CTPStrg(20)=' distance for second projectile'
      CTParam(21)=0.0d0
      CTPStrg(21)=' deformation parameter'

      CTParam(25)=.9d0
      CTPStrg(25)=' probability for diquark not to break'
      CTParam(26)=50.d0
      CTPStrg(26)=' maximum trials to get string masses'
      CTParam(27)=1.d0
      CTPStrg(27)=' scaling factor for xmin in string excitation'
      CTParam(28)=1.d0
      CTPStrg(28)=' scaling factor for transverse fermi motion'
      CTParam(29)=0.4d0
      CTPStrg(29)=' single strange di-quark suppression factor '
      CTParam(30)=1.5d0
      CTPStrg(30)=' radius offset for initialization  '
      CTParam(31)=1.6d0
      CTPStrg(31)=' sigma of gaussian for tranverse momentum tranfer '
      CTParam(32)=0.d0
      CTPStrg(32)=' alpha-1 for valence quark distribution  '
      CTParam(33)=2.5d0
      CTPStrg(33)=' betav for valence quark distribution  (DPM)'
      CTParam(34)=0.1d0
      CTPStrg(34)=' minimal x multiplied with ecm  '
      CTParam(35)=3.0d0
      CTPStrg(35)=' offset for cut for the FSM '
      CTParam(36)=0.275d0
      CTPStrg(36)=' fragmentation function parameter a  '
      CTParam(37)=0.42d0
      CTPStrg(37)=' fragmentation function parameter b  '
      CTParam(38)=1.08d0
      CTPStrg(38)=' diquark pt scaling factor '
      CTParam(39)=0.8d0
      CTPStrg(39)=' strange quark pt scaling factor '
      CTParam(40)=0.5d0
      CTPStrg(40)=' betas-1 for valence quark distribution (LEM)'
      CTParam(41)=0.d0
      CTPStrg(41)=' distance of initialization'
      CTParam(42)=0.55d0
      CTPStrg(42)=' width of gaussian -> pt in string-fragmentation '
      CTParam(43)=5.d0
      CTPStrg(43)=' maximum kinetic energy in mesonic clustr '
      CTParam(44)=0.8d0
      CTPStrg(44)=' prob. of double vs. single excitation for AQM inel.'
      CTParam(45)=0.5d0
      CTPStrg(45)=' offset for minimal mass generation of strings'
      CTParam(46)=800000.d0
      CTPStrg(46)=' maximal number of rejections for initialization'
      CTParam(47)=1.0d0
      CTPStrg(47)=' field feynman fragmentation funct. param. a'
      CTParam(48)=2.0d0
      CTPStrg(48)=' field feynman fragmentation funct. param. b'

      CTParam(50)=1.d0
      CTPStrg(50)=' enhancement factor for 0- mesons'
      CTParam(51)=1.d0
      CTPStrg(51)=' enhancement factor for 1- mesons'
      CTParam(52)=1.d0
      CTPStrg(52)=' enhancement factor for 0+ mesons'
      CTParam(53)=1.d0
      CTPStrg(53)=' enhancement factor for 1+ mesons'
      CTParam(54)=1.d0
      CTPStrg(54)=' enhancement factor for 2+ mesons'
      CTParam(55)=1.d0
      CTPStrg(55)=' enhancement factor for 1+-mesons'
      CTParam(56)=1.d0
      CTPStrg(56)=' enhancement factor for 1-*mesons'
      CTParam(57)=1.d0
      CTPStrg(57)=' enhancement factor for 1-*mesons'
      CTParam(58)=1.d0
      CTPStrg(58)=' scaling factor for DP time-delay'

C  DEFAULT SETTINGS FOR CTOption
      CTOption(1)=1                  ! hjd1
      CTOStrng(1)=' resonance widths are mass dependent '
      CTOption(2)=0
      CTOStrng(2)=' conservation of scattering plane'
      CTOption(3)=0
      CTOStrng(3)=' use modified detailed balance'
      CTOption(4)=0
      CTOStrng(4)=' no initial conf. output '
      CTOption(5)=0
      CTOStrng(5)=' fixed random impact parameter'
      CTOption(6)=0
      CTOStrng(6)=' no first collisions inside proj/target'
      CTOption(7)=0
      CTOStrng(7)=' elastic cross-section enabled (<>0:total=inelast)'
      CTOption(8)=0
      CTOStrng(8)=' extrapolate branching ratios '
      CTOption(9)=0
      CTOStrng(9)=' use tabulated pp cross-sections '
      CTOption(10)=0
      CTOStrng(10)=' enable Pauli Blocker'
      CTOption(11)=0
      CTOStrng(11)=' mass reduction for cascade initialization'
      CTOption(12)=0
      CTOStrng(12)=' string condition =0 (.ne.0 no strings)'
      CTOption(13)=0
      CTOStrng(13)=' enhanced file16 output '
      CTOption(14)=0
      CTOStrng(14)=' cos(the) is distributet between -1..1 '
      CTOption(15)=0
      CTOStrng(15)=' allow mm&mb-scattering'
      CTOption(16)=0
      CTOStrng(16)=' propagate without collisions'
      CTOption(17)=0
      CTOStrng(17)=' colload after every timestep '
      CTOption(18)=0
      CTOStrng(18)=' final decay of unstable particles'
      CTOption(19)=0
      CTOStrng(19)=' allow bbar annihilaion'
      CTOption(20)=0
      CTOStrng(20)=' dont generate e+e- instead of bbar'
      CTOption(21)=0
      CTOStrng(21)=' use field feynman frgm. function'
      CTOption(22)=1
      CTOStrng(22)=' use lund excitation function'
      CTOption(23)=0
      CTOStrng(23)=' lorentz contraction of projectile & targed'
      CTOption(24)=2      ! 1 is default    2 means fast method
      CTOStrng(24)=' Wood-Saxon initialization'
      CTOption(25)=0
      CTOStrng(25)=' phase space corrections for resonance mass'
      CTOption(26)=0
      CTOStrng(26)=' use z -> 1-z for diquark-pairs'
      CTOption(27)=1             ! hjd1
      CTOStrng(27)=' reference frame (1=target, 2=projectile, else=cms)'
      CTOption(28)=0
      CTOStrng(28)=' propagate spectators also '
      CTOption(29)=2
      CTOStrng(29)=' no transverse momentum in clustr '
      CTOption(30)=1
      CTOStrng(30)=' frozen fermi motion '
      CTOption(31)=0
      CTOStrng(31)='  reduced mass spectrum in string'
      CTOption(32)=0
      CTOStrng(32)=' masses are distributed acc. to m-dep. widths'
      CTOption(33)=0
      CTOStrng(33)=' use tables & m-dep. for pmean in fprwdt & fwidth'
      CTOption(34)=1
      CTOStrng(34)=' lifetme according to m-dep. width'
      CTOption(35)=1
      CTOStrng(35)=' generate high precision tables'
      CTOption(36)=0
      CTOStrng(36)=' normalize Breit-Wigners with m.dep. widths '
      CTOption(37)=0
      CTOStrng(37)=' heavy quarks form di-quark clusters'
      CTOption(38)=0
      CTOStrng(38)=' scale p-pbar to b-bbar with equal p_lab '
      CTOption(39)=0
      CTOStrng(39)=' dont call pauliblocker'
      CTOption(40)=0
      CTOStrng(40)=' read old fort.14 file '
      CTOption(41)=0
      CTOStrng(41)=' generate extended output for cto40'
      CTOption(42)=0
      CTOStrng(42)=' hadrons now have color fluctuations'
      CTOption(43)=0
      CTOStrng(43)=' dont generate dimuon intead of dielectron output'
      CTOption(44)=0
      CTOStrng(44)=' not used at the moment'
      CTOption(45)=0
      CTOStrng(45)=' not used at the moment'

C  INITIALIZE ARRAYS FOR SPECIAL PRO/TAR COMBINATIONS
      do  i = 1, 2
         spityp(i) = 0
         spiso3(i) = 0
      enddo

C  INITIALIZE ARRAYS FOR SPECIAL PARTICLES
      EoS = 0

C  READ CROSS-SECTION FILES
Cdh   CALL URQREC()

C  INITIALIZES SOME ARRAYS
      call strini      ! initialize mixing angles for meson-multipletts
      call loginit

      IF ( CTOption(33) .EQ. 0  .OR.  CTOption(9) .EQ. 0 ) THEN
        call loadwtab(io)
        IF ( IUDEBUG .GT. 0 ) WRITE(IFCK,*) 'URQINI: AFTER LOADWTAB'
      ENDIF

C READ URQMD TOTAL CROSS SECTION TABLE
c
c   ie=1..41   E=10.0**(float(ie)/10-1.0-0.05)  (bin-middle)
c   id=1..9    p,ap,n,an,pi+,pi-,K+,K-,KS
c   ia=1..3    N,O,Ar
c
      if(ifurqdat.eq.1)then
        OPEN(UNIT=76,FILE=furqdat(1:nfurqdat),STATUS='OLD')
      else
        OPEN(UNIT=76,FILE='UrQMD-1.3.1-xs.dat',STATUS='OLD')
      endif
      read(76,*) adum,iamaxu,idmaxu,iemaxu
      do ia=1,iamaxu
        do id=1,idmaxu
          do ie=1,iemaxu
            read(76,*) ekdummy,sig_u1(ie,id,ia)
          enddo
          read(76,*)
          read(76,*)
        enddo
      enddo
      close(76)

C  IN CASE OF CASCADE MODE, THE POTENTIALS NEED NOT BE CALCULATED

C  CALCULATE NORMALIZATION OF RESONANCES DISTRIBUTION...
      call norm_init
#endif


      xsegymin=0.25d0

#ifdef __CXDEBUG__
      call utisx2
#endif

      end

c-----------------------------------------------------------------------
      subroutine IniEvtUrq
c-----------------------------------------------------------------------
c Initialization for each type of event (for given proj, targ and egy)
c-----------------------------------------------------------------------
      implicit none
#include "conex.h"
#include "conex.incnex"

      include 'boxinc.f'
      include 'inputs.f'
      include 'options.f'

c commons from coms.f
      integer Ap, At, Zp, Zt, npart, nbar, nmes, ctag
      integer nsteps,ranseed,event,eos,dectag,uid_cnt
      integer NHardRes,NSoftRes,NDecRes,NElColl,NBlColl
      common /sys/ npart, nbar, nmes, ctag,nsteps,uid_cnt,
     +             ranseed,event,Ap,At,Zp,Zt,eos,dectag,
     +             NHardRes,NSoftRes,NDecRes,NElColl,NBlColl

      real*8  time,  acttime, bdist, ebeam, bimp,bmin,ecm
      common /rsys/ time,acttime,bdist,bimp,bmin,ebeam,ecm

      real*8
     +     gw, sgw, delr, fdel, dt,
     +     da, db,
     +     Cb0, Yuk0, Pau0, Sky20, Sky30, gamSky, gamYuk, drPau, dpPau,
     +     dtimestep
c 19 real*8
      common /pots/ Cb0, Yuk0, Pau0, Sky20, Sky30, gamSky,
     +              gamYuk, drPau, dpPau, gw, sgw, delr, fdel,
     +              dt,da, db,dtimestep

c local

      integer ICUTBL(200),ICU2I3(200),BIM(3)
      common /urqmdproj/ICUTBL,ICU2I3
C  CONVERSION TABLE CORSIKA TO URQMD
      DATA ICUTBL/
     &   0,   0,   0,   0,   0,   0, 101, 101, 101, 106,    ! 10
     & 106,-106,   1,   1,  -1,-106, 102,  27,  40,  40,    ! 20
     &  40,  49,  49,  55,  -1, -27, -40, -40, -40, -49,    ! 30
     & -49, -55,   0,   0,   0,   0,   0,   0,   0,   0,    ! 40
     & 160 * 0 /
C  TABLE FOR ISOSPIN COMPONENT 2*I3  (CONVERSION CORSIKA TO URQMD)
      DATA ICU2I3/
     &   0,   0,   0,   0,   0,   0,   0,   2,  -2,  -1,
     &   1,  -1,  -1,   1,  -1,   1,   0,   0,   2,   0,
     &  -2,   1,  -1,   0,   1,   0,  -2,   0,   2,  -1,
     &   1,   0,   0,   0,   0,   0,   0,   0,   0,   0,
     & 160 * 0 /
      DATA BIM / 6.D0, 6.D0, 7.D0 /
      
      double precision caltim,outtim,nucrad,furqcrse
      integer idtmp,idtrafocx,lt

      if(maprojxs.gt.210)
     &  stop'Mass too big for UrQMD (Mprj<210) !'
      call cxiclass(idprojxs,iclproxs)
      bmin=sngl(xsbminim)
      bdist=xsbmaxim
C  SELECT AND INITIALIZE TARGET
      trspflg   =  0
      At  = matargxs
      Zt  = latargxs
      lt=1
      if(matargxs.eq.16)then
        lt=2
      elseif(matargxs.eq.40)then
        lt=3
      endif

C  SET THE PROJECTILE
      if(laprojxs.ge.0)then
C  PROJECTILE IS A NUCLEUS
        prspflg = 0
        Ap  = maprojxs
        Zp  = laprojxs
      else
C  PROJECTILE IS A SPECIAL PARTICLE
        Ap  = 1
        prspflg = 1
        idtmp=idtrafocx('nxs','cor',idprojxs)
        spityp(1) = ICUTBL(idtmp)
        spiso3(1) = ICU2I3(idtmp)
      endif

C  ENERGY OF COLLISION (LAB-SYSTEM)
      ebeam   = dble(xsekin)
C  eos: impact parameter
      eos     = 0
C  nev: number of events
      nevents = 1
C  tim: time of propagation
      caltim  = 200.d0
      outtim  = 200.d0
C  fast CASCADE mode
      if ( eos .eq. 0 ) dtimestep = outtim
C
      nsteps   = int(0.01d0+caltim/dtimestep)
      outsteps = int(0.01d0+outtim/dtimestep)

      if ( boxflag .eq. 0 ) then

C  initialize nuclei (projectile and target) and store them

C  initialize normal projectile
        if ( prspflg .eq. 0 ) then
          call cascinit(Zp,Ap,1)
        endif
C  initialize normal target
        if ( At .ne. 0 ) then
          if ( trspflg .eq. 0 ) then
            call cascinit(Zt,At,2)
          endif
        endif
      endif



      xsbmax=nucrad(maprojxs)+nucrad(matargxs)+2*CTParam(30)
#if !__CXCORSIKA__ && !__CORSIKA8__

      xsurqincs=furqcrse()
#else
      xsurqincs=1d20
#endif

      CTOption(5)=1
      if ( prspflg.eq.0 ) then
        bdist = BIM(LT)
      else
        bdist = xsbmax
      endif
      if(xsekin.lt.xsegymin)xsurqincs=0.     !below xsegymin, no interaction


#ifdef __CXDEBUG__
      if(isx.ge.2)write(ifck,*)
     &  'UrQMD used with (Ek,idproj,matarg,xs) ',xsekin,idprojxs
     &                                          ,matargxs,xsurqincs
#endif

      return
      end

c-----------------------------------------------------------------------
      subroutine emsurq(iret)
c-----------------------------------------------------------------------
c  call UrQMD to simulate interaction
c-----------------------------------------------------------------------
      implicit none
#include "conex.h"
#include "conex.incnex"
      integer ICUTBL(200),ICU2I3(200)
      common /urqmdproj/ICUTBL,ICU2I3
      include 'boxinc.f'
      include 'inputs.f'
      include 'options.f'
c commons from coms.f
      integer Ap, At, Zp, Zt, npart, nbar, nmes, ctag
      integer nsteps,ranseed,event,eos,dectag,uid_cnt
      integer NHardRes,NSoftRes,NDecRes,NElColl,NBlColl
      common /sys/ npart, nbar, nmes, ctag,nsteps,uid_cnt,
     +             ranseed,event,Ap,At,Zp,Zt,eos,dectag,
     +             NHardRes,NSoftRes,NDecRes,NElColl,NBlColl

      real*8  time,  acttime, bdist, ebeam, bimp,bmin,ecm
      common /rsys/ time,acttime,bdist,bimp,bmin,ebeam,ecm

      integer nmax
      parameter (nmax = 500) ! maximum number of particles
      integer spin(nmax),ncoll(nmax),charge(nmax),strid(nmax),
     +        ityp(nmax),lstcoll(nmax),iso3(nmax),origin(nmax),uid(nmax)
      real*8
     +     r0(nmax), rx(nmax), ry(nmax), rz(nmax),
     +     p0(nmax), px(nmax), py(nmax), pz(nmax),
     +     fmass(nmax), rww(nmax),
     +     dectime(nmax)
      common/isys/spin,ncoll,charge,ityp,lstcoll,iso3,origin,strid,
     +            uid
      common /coor/ r0, rx, ry, rz, p0, px, py, pz, fmass, rww, dectime

c local
      double precision fmasscx,p0cx
      integer iret,i,idepos,idpdg,j,nn,idtrafocx,pdgid,ist
     *,nptlini,np1,np2,ip


      iret=0
      if(abs(xsurqincs).lt.1d-20)goto 1002   !no interaction
#ifdef __CXDEBUG__
      if(isx.ge.4)call cxalist('Determine URMQD Production&',0,0,0)
#endif
      nptlxs=0

C  NOW THE REAL WORK IS DONE
      CALL URQMD( 0 )

      if(isx.ge.5)write(ifck,*)'UrQMD: ctag,NElColl=',ctag,NElColl,npart

      if(ctag.le.NElColl)goto 1002   !elastic interaction, included in xs
      
      call cxconre
      call cxconwr(ist)

      ncolxs=1
      nevtxs=1
      xsbimp=bimp
      xsphi=0.
      nptlini=nptlxs+1

c set projectile and target as non final particles
      do i=1,maprojxs
        istptlxs(i)=1
      enddo
      do j=1,matargxs
        istptlxs(j)=1
      enddo

#ifdef __CXDEBUG__
      if(isx.ge.5)write(ifck,'(a,i5)')
     $         ' number of particles from UrQMD :',npart
#endif

      do nn=1,npart

#ifdef __CXDEBUG__
          if(isx.ge.7)write(ifck,'(a,i4,a,i4,i5,2a,5(e11.4,1x))')
     $    ' URQMD particle ',nn,' id,iso3 :',ityp(nn),iso3(nn)
     $  , ' before conversion'
     $  , ' E, P and mass :',p0(nn),px(nn),py(nn),pz(nn),fmass(nn)
#endif

        idpdg=pdgid(ityp(nn),iso3(nn))
        idepos=idtrafocx('pdg','nxs',idpdg)
        
        nptlxs=nptlxs+1
               
c masses are not constant in UrQMD, use the CONEX one.
        call cxidmass(idepos,fmasscx)
        p0cx=px(nn)**2+py(nn)**2+pz(nn)**2+fmasscx**2
        if(p0cx.gt.0d0)then
          p0cx=sqrt(p0cx)
        else !error
          goto 1001
        endif
        iorptlxs(nptlxs)=0
        jorptlxs(nptlxs)=0
        istptlxs(nptlxs)=0
        xsorptl(4,nptlxs)= r0(nn)
        xsorptl(1,nptlxs)=rx(nn)
        xsorptl(2,nptlxs)=ry(nn)
        xsorptl(3,nptlxs)=rz(nn)
        xsptl(4,nptlxs)=p0cx
        xsptl(1,nptlxs)=px(nn)
        xsptl(2,nptlxs)=py(nn)
        xsptl(3,nptlxs)=pz(nn)
        xsptl(5,nptlxs)=fmasscx
        idptlxs(nptlxs)=idepos
        ityptlxs(nptlxs)=0


#ifdef __CXDEBUG__
        if(isx.ge.5)write(ifck,'(a,i5,a,i5,a,4(e11.4,1x),f6.3)')
     $         ' particle from URQMD ',nptlxs,' id :',idptlxs(nptlxs)
     $         , ' momentum :',(xsptl(i,nptlxs),i=1,5)
#endif
                                  
      enddo

c Decay particles with short life time

      np1=nptlini
41    np2=nptlxs
      do 42 ip=np1,np2
        call cxhdecas(ip,iret)
        if(iret.ne.0)goto 1001
42    continue
      np1=np2+1
      if(np1.le.nptlxs)goto 41

 1000 return

 1001 iret=1 
      goto 1000 

 1002 iret=-1     !elastic
      goto 1000 

      end

c------------------------------------------------------------------------------
      double precision function furqcrse()
c------------------------------------------------------------------------------
c hadron/nucleus-nucleus particle production cross section with UrQMD.
c------------------------------------------------------------------------------
#include "conex.incnex"
      double precision urqcrse
      furqcrse=urqcrse(xsekin,idprojxs)
      return
      end
c------------------------------------------------------------------------------
      double precision function urqcrse(ek,idpro)
c------------------------------------------------------------------------------
c hadron/nucleus-nucleus particle production cross section with UrQMD.
c------------------------------------------------------------------------------
      implicit none
#include "conex.h"
      integer mxie,mxid,mxia
      parameter (mxie=41,mxid=10,mxia=3)
      double precision sig_u1,xsu
      integer iamaxu,idmaxu,iemaxu
      common /cxs_u1/ sig_u1(mxie,mxid,mxia),iamaxu,idmaxu,iemaxu
      double precision xs(3)
      common /cxs_u2/ xs

      dimension we(3)
      double precision ye,we,ek
      integer je,i,id,ia,idpro


      urqcrse = 0.d0
      if(ek.gt.900.d0)return
      ye = log10(ek)*10.d0+10.5d0
      if ( ye. lt. 1d0 ) ye = 1.d0
      je = min(iemaxu-2,int(ye))
      we(2) = ye-je
      we(3) = we(2)*(we(2)-1.d0)*.5d0
      we(1) = 1.d0-we(2)+we(3)
      we(2) = we(2)-2.d0*we(3)
      
      id=0
      if    ( idpro .eq. 1120 ) then
        id = 1
      elseif( idpro .eq. -1120) then
        id = 2
      elseif( idpro .eq. 1220 ) then
        id = 3
      elseif( idpro .eq. -1220) then
        id = 4
      elseif( idpro .eq.  120 ) then
        id = 5
      elseif( idpro .eq.  -120) then
        id = 6
      elseif( idpro .eq. 130 ) then
        id = 7
      elseif( idpro .eq. -130) then
        id = 8
      elseif( idpro .eq. 20  .or.  idpro .eq. -20 ) then
        id = 9
      endif
      
      if(id.ne.0)then

c     ia=1..3
c  N,O,Ar
        do  ia = 1,3
          xsu=0d0
          do i = 1,3
            xsu  = xsu + sig_u1(je+i-1,id,ia)*we(i)
          enddo
          urqcrse  = urqcrse + airw(ia)*xsu
        enddo

      else

        do ia = 1,3
          urqcrse = urqcrse + airw(ia)*XS(ia)
        enddo

      endif


      return
      end


#if !__CXCORSIKA__ && !__CORSIKA8__

C=======================================================================

      DOUBLE PRECISION FUNCTION RANF(idum)

C-----------------------------------------------------------------------
C  FL(UKA) R(A)ND(O)M (GENERATOR)
C  THIS FUNCTON IS CALLED FROM URQMD ROUTINES.
C-----------------------------------------------------------------------
      double precision drangen
      ranf=drangen(dble(idum))

      RETURN
      END
 
#endif
#endif
#ifdef __FLUKA__
c 06.09.2011 Link routines between FLUKA2011 and CONEX.
c author T. Pierog based on CORSIKA and EPOS link to FLUKA

#ifdef __STD__
#define __GHEISHA__
#define __QGSJET__
#define __ANALYSIS__
#endif

c-----------------------------------------------------------------------
      subroutine IniFluka
c-----------------------------------------------------------------------
c Primary initialization for FLUKA
c-----------------------------------------------------------------------
#if !__CXCORSIKA__ && !__CORSIKA8__

      INCLUDE '(DBLPRC)'  !
      INCLUDE '(DIMPAR)'  !
      INCLUDE '(IOUNIT)'  !
      INCLUDE '(FLKCMP)'  !
      INCLUDE '(FLKMAT)'  !
      INCLUDE '(PAREVT)'  !
      INCLUDE '(PHNCCM)'  !
      INCLUDE '(CTITLE)'  !

C  FLUKA 2011.2 BLOCK DATA PROGRAMS
      EXTERNAL BDINPT,BDTRNS,BDHDR1,BDHDR2,BDHDR3,BDPART,BDPRDC,
     &         BDNOPT,BDEVAP,BDPREE

      DIMENSION        WHAT (6)
      CHARACTER        SDUM*10
      DATA             WHAT / 6 * ZERZER /
#endif

c CONEX includes
#include "conex.h"
#include "conex.incnex"
      data init/0/
      save init

      if(init.ge.1)return
      init=init+1
#ifdef __CXDEBUG__
      call utisx1('inifluka  ',4)
      write(*,'(a)')'initialize FLUKA ...'
#endif

#if !__CXCORSIKA__ && !__CORSIKA8__

C open fluka output/error files
      OPEN(UNIT=LUNOUT,FORM='FORMATTED',STATUS='UNKNOWN',
     *     FILE=fnck(1:nfnck-5)//'fkout')
      OPEN(UNIT=LUNERR,FORM='FORMATTED',STATUS='UNKNOWN',
     *     FILE=fnck(1:nfnck-5)//'fkerr')

      CALL CMSPPR
C  CLEAR FLUKA STORAGE AREAS
      ITEXMX = 10000000
      CALL ZEROIN
      LEVPRT = .TRUE.
      LDEEXG = .TRUE.
      LHEAVY = .TRUE.
      IFISS  = 1
      LGDHPR = .TRUE.
C  CHARMED PARTICLES SHOULD NOT DECAY WHEN ORIGINATING
      LCHDCY = .FALSE.

c random numbers initialization
        NREGS        = 4
C  FLUKA MATERIAL NUMBER 1 IS NITROGEN
        MMAT         = 3
        MEDFLK(2,1)  = MMAT
        ZTAR  (MMAT) = 7
        AMSS  (MMAT) = 14.007D+00
        RHO   (MMAT) = ONEONE
        RHPHNC(MMAT) = ONEONE
        IFPHNC(MMAT) = 1111
        MSSNUM(MMAT) = 0
        AOCMBM(MMAT) = RHO(MMAT) / AMSS(MMAT) * AVOGAD * 1.D-24
        ICOMP (MMAT) = 0
        MATNAM(MMAT) = 'NITROGEN'

C  FLUKA MATERIAL NUMBER 2 IS OXYGEN
        IMAT         = 4
        MEDFLK(3,1)  = IMAT
        ZTAR  (IMAT) = 8
        AMSS  (IMAT) = 15.9994D+00
        RHO   (IMAT) = ONEONE
        RHPHNC(IMAT) = ONEONE
        IFPHNC(IMAT) = 1111
        MSSNUM(IMAT) = 0
        AOCMBM(IMAT) = RHO(IMAT) / AMSS(IMAT) * AVOGAD * 1.D-24
        ICOMP (IMAT) = 0
        MATNAM(IMAT) = 'OXYGEN'

C  FLUKA MATERIAL NUMBER 3 IS ARGON
        JMAT         = 5
        MEDFLK(4,1)  = JMAT
        ZTAR  (JMAT) = 18
        AMSS  (JMAT) = 39.948D+000
        RHO   (JMAT) = ONEONE
        RHPHNC(JMAT) = ONEONE
        IFPHNC(JMAT) = 1111
        MSSNUM(JMAT) = 0
        AOCMBM(JMAT) = RHO(JMAT) / AMSS(JMAT) * AVOGAD * 1.D-24
        ICOMP (JMAT) = 0
        MATNAM(JMAT) = 'ARGON'

C  FLUKA MATERIAL NUMBER 4 IS AIR
        NMAT         = 6
        MEDFLK(1,1)  = NMAT
        ICOMP (NMAT) = 1
        ICOMPL(NMAT) = 3
        MATNUM(1)    = 3
        MATNUM(2)    = 4
        MATNUM(3)    = 5
        RHO   (NMAT) = ONEONE
        RHPHNC(NMAT) = ONEONE
        IFPHNC(NMAT) = 1111
        CONTNT(1)    = 0.92561D-03
        CONTNT(2)    = 0.28361D-03
        CONTNT(3)    = 0.15776D-04
        RENORM       = RHO(NMAT) / ( CONTNT(1) + CONTNT(2) + CONTNT(3) )
        CONTNT(1)    = CONTNT (1) * RENORM
        CONTNT(2)    = CONTNT (2) * RENORM
        CONTNT(3)    = CONTNT (3) * RENORM
        MATNAM(NMAT) = 'AIR'
C
      CALL EVVINI( WHAT,SDUM )
      CALL SETITB

#endif


      xsegymin=0.01d0

#ifdef __CXDEBUG__
      call utisx2
#endif
      end

c-----------------------------------------------------------------------
      subroutine IniEvtFlu
c-----------------------------------------------------------------------
c Initialization for each type of event (for given proj, targ and egy)
c-----------------------------------------------------------------------
      implicit double precision (a-h,o-z)
#include "conex.h"
#include "conex.incnex"
      double precision fflucrse


      if(maprojxs.gt.1)
     &  stop'Mass too big for Fluka (Mprj=1) !'
      call cxiclass(idprojxs,iclproxs)
#if !__CXCORSIKA__ && !__CORSIKA8__

      xsbmax=10000.d0
      xsfluincs=fflucrse()
#else
      xsbmax=20d0
      xsfluincs=1d20
#endif

      if(xsekin.lt.xsegymin)xsfluincs=0.          !below egymin, no interaction
c      if(iclproxs.eq.3.and.xsekin.lt.0.1d0)xsfluincs=0.     !elastic event for kaons below 100MeV (problem) !


#ifdef __CXDEBUG__
      if(isx.ge.2)write(ifck,*)
     &  'Fluka used with (Ek,idproj,matarg,xs) ',xsekin,idprojxs
     &                                          ,matargxs,xsfluincs
#endif


      return
      end

c-----------------------------------------------------------------------
      subroutine emsflu(iret)
c-----------------------------------------------------------------------
c  call Fluka to simulate interaction
c-----------------------------------------------------------------------
      INCLUDE '(DBLPRC)'   !
      INCLUDE '(DIMPAR)'   !
      INCLUDE '(FHEAVY)'
      INCLUDE '(GENSTK)'   !
      INCLUDE '(PAREVT)'   !
      INCLUDE '(PART2)'    !
      INCLUDE '(RESNUC)'    !
#include "conex.h"
#include "conex.incnex"
      double precision P0(5),P1,EKIN1,PPROJ,TXX,TYY,TZZ,WEE,POO,PRES(4)

      iret=0
      if(abs(xsfluincs).lt.1d-20)goto 1002   !no interaction
#ifdef __CXDEBUG__
      if(isx.ge.4)call cxalist('Determine FLUKA Production&',0,0,0)
#endif
      nptlxs=0
      np=0


C  CONVERT PARTICLE TYPE TO FLUKA
      KPROJ  = idtrafocx('nxs','flk',idprojxs)
      EKIN1  = xsekin
C  CALCULATE MOMENTUM PPROJ
      PPROJ = SQRT( EKIN1 * (EKIN1 + 2.D00 * AAM(IPTOKP(KPROJ)) ) )
      LEVFIN = .FALSE.
C  TARGET
      if(latargxs.eq.7)then
        MMMAT = 3
      elseif(latargxs.eq.8)then
        MMMAT = 4
      elseif(latargxs.eq.18)then
        MMMAT = 5
      else
        stop'Wrong mass in air component (FLUKA) !'
      endif
C  USE THE FLUKA-INTERNALLY USED DIRECTION COSINES:
      TXX = 0.D0
      TYY = 0.D0
      TZZ = 1.D0
      WEE = ONEONE
      IJ  = KPROJ
C  CALCULATE THE MOMENTUM WITH FLUKA MASSES
      POO = PPROJ
C  NOW INTERACTION IS PERFORMED
      CALL EVENTV( IJ,POO,EKIN1,TXX,TYY,TZZ,WEE,MMMAT )
c      write(ifck,*)'target used',ibtar,ichtar,mmmat,ibres
      
      matargxs=IBTAR    !update mass in case of isotope
      
      ist=1
      call cxconre
      call cxconwr(ist)

      nptlini=nptlxs+1

      if(NP.eq.0.and.NPHEAV.eq.0)goto 1001         !no interaction

      ncolxs=1
      nevtxs=1
      xsbimp=0.d0
      xsphi=0.d0

C  NOW TREAT THE REMAINING TARGET NUCLEUS

#ifdef __CXDEBUG__
      if(isx.ge.5)write(ifck,550)IBRES,ICRES,EKRES 
 550  format('FLUKA: remaining nucleus',/,
     *                        '  mass=',I3,' charge=',I3,
     *                             ' ekin=',g9.2)
#endif
      if(IBRES.gt.1)then

        if(IBRES.gt.matargxs)then
          write(ifck,*)'emsflu:more nucleons in target remnant than '
     *                ,'in target ! ... skip event.'
          goto 1001
        endif
        
        ekin1=EKRES/dble(IBRES)
        PRES(4)=sqrt(PXRES*PXRES+PYRES*PYRES+PZRES*PZRES)
        if(PRES(4).gt.0d0)then
          PRES(3)=PZRES/PRES(4) !Pz
          PRES(2)=PYRES/PRES(4) !Py
          PRES(1)=PXRES/PRES(4) !Px
        elseif(abs(ekin1).lt.1d-6)then
          PRES(3)=0.d0 !Pz
          PRES(2)=0.d0 !Py
          PRES(1)=0.d0 !Px
        else         !ekin and momentum not consistent
          write(*,*)'emsflu: ekin and momentum not consistent !'
     *                ,' ... skip event.'
          goto 1001
        endif

        nptlxs=maprojxs+matargxs+1    !register spectators

        do k=1,IBRES
          
c final particle
          nptlxs=nptlxs-1             !go backwards
          istptlxs(nptlxs)=0
          if(nptlxs.gt.mxptlxs)stop'FlUKA: mxptlxs too small'

          if(k.le.ICRES)then   !protons
            id=1120
            P0(5)=AAM(1)        !Mass
          else                  !neutrons
            id=1220
            P0(5)=AAM(8)        !Mass
          endif
            
          P0(4)=ekin1+P0(5)                   !Energy
          P1=sqrt((P0(4)+P0(5))*(P0(4)-P0(5))) !Momentum
          P0(3)=PRES(3)*P1                      !Pz
          P0(2)=PRES(2)*P1                      !Py
          P0(1)=PRES(1)*P1                      !Px

          call flk2cnx(id,P0,1)

        enddo
      endif

      nptlxs=maprojxs+matargxs

#ifdef __CXDEBUG__
        if(isx.ge.5)write(ifck,'(a,i5)')
     $         ' number of particles from Fluka :',NP
#endif
        do 500 k=1,NP

c LLIST is the code of final particle, P - its 4-momentum and mass.
          ic=KPART(k)
          icm=IPTOKP(ic)          !convert to internal FLUKA ID to use AAM
            
#ifdef __CXDEBUG__
          if(isx.ge.7)write(ifck,'(a,i5,a,i5,2a,5(e11.4,1x))')
     $    ' FLUKA particle ',k,' id :',ic,' before conversion'
     $  , ' Ekin, cos and mass :',TKI(k),CXR(k),CYR(k),CZR(k),AAM(icm)
#endif

          if(ic.le.-3)then      !send He-3 in fragments
            NPHEAV=NPHEAV+1
            if(NPHEAV.gt.MXHEAV)goto 1001   !no room left, skip event
            KHEAVY(NPHEAV)=abs(ic)
            TKHEAV(NPHEAV)=TKI(k)
            CXHEAV(NPHEAV)=CXR(k)
            CYHEAV(NPHEAV)=CYR(k)
            CZHEAV(NPHEAV)=CZR(k)
            goto 500
          endif

c final particle
          nptlxs=nptlxs+1
          istptlxs(nptlxs)=0
          if(nptlxs.gt.mxptlxs)stop 'FLUKA: mxptlxs too small'


          id=idtrafocx('flk','nxs',ic)
#ifdef __CXDEBUG__
          if(isx.ge.7)write(ifck,'(a,i5,a,i5,a)')
     $         ' CONEX particle ',nptlxs,' id :',id,' after conversion'
#endif
            
          P0(5)=AAM(icm)                         !Mass
          P0(4)=TKI(k)+P0(5)                   !Energy
          P1=sqrt((P0(4)+P0(5))*(P0(4)-P0(5))) !Momentum
          P0(3)=CZR(k)*P1                      !Pz
          P0(2)=CYR(k)*P1                      !Py
          P0(1)=CXR(k)*P1                      !Px

          call flk2cnx(id,P0,0)

 500      continue


C  NOW TREAT THE HEAVY FRAGMENTS

#ifdef __CXDEBUG__
        if(isx.ge.5)write(ifck,'(a,i5)')
     $         ' number of fragments from FLUKA :',NPHEAV
#endif

        do j=1,NPHEAV

c LLIST is the code of final particle, P - its 4-momentum and mass.
         ic=KHEAVY(J)
            
#ifdef __CXDEBUG__
         if(isx.ge.7)write(ifck,'(a,i5,a,i5,2a,4(e11.4,1x))')
     $       ' FLUKA fragment ',j,' id :',ic,' before conversion'
     $     , ' Ekin and cos :',TKHEAV(J),CXHEAV(J),CYHEAV(J),CZHEAV(J)
#endif
         IF ( ic .LE. 0 .OR. ic .gt.12 ) THEN
#ifdef __CXDEBUG__
              if (isx .ge.1)WRITE(*,*)
     *          'emsflu: WRONG FRAGMENT: KHEAVY=',KHEAVY(J)
#endif
           GOTO 1001
         ENDIF
         iz  = ICHEAV(ic)
         ia  = IBHEAV(ic)
         
         ekin1=TKHEAV(J)/dble(ia)
         PRES(3)=CZHEAV(J)      !Pz
         PRES(2)=CYHEAV(J)      !Py
         PRES(1)=CXHEAV(J)      !Px

c         if(ia.eq.2.and.iz.eq.1)then       !deuterium
c           id=17
c           P0(5)=AAM(-3)        !Mass
c           ia=1
c           iz=-1                !no to update the id and the mass in the loop
c         elseif(ia.eq.3.and.iz.eq.1)then   !tritium
c           id=18
c           P0(5)=AAM(-4)        !Mass
c           ia=1
c           iz=-1                !no to update the id and the mass in the loop
c         elseif(ia.eq.4.and.iz.eq.2)then   !helium
c           id=19
c           P0(5)=AAM(-6)        !Mass
c           ia=1
c           iz=-1                !no to update the id and the mass in the loop
c         endif                  !other nucleus

         do k=1,ia
c final particle
          nptlxs=nptlxs+1
          if(k.le.iz)then       !protons
            id=1120
            P0(5)=AAM(1)        !Mass
          elseif(iz.ge.0)then   !neutrons
            id=1220
            P0(5)=AAM(8)        !Mass
          endif
            
          istptlxs(nptlxs)=0
          if(nptlxs.gt.mxptlxs)stop'FLUKA: mxptl too small'


#ifdef __CXDEBUG__
          if(isx.ge.7)write(ifck,'(a,i5,a,i5,a)')
     $         ' CONEX particle ',nptlxs,' id :',id,' after conversion'
#endif
            
          P0(4)=ekin1+P0(5)                    !Energy
          P1=sqrt((P0(4)+P0(5))*(P0(4)-P0(5))) !Momentum
          P0(3)=PRES(3)*P1                      !Pz
          P0(2)=PRES(2)*P1                      !Py
          P0(1)=PRES(1)*P1                      !Px

          call flk2cnx(id,P0,0)
          
         enddo

        enddo

c Decay particles with short life time

      np1=nptlini
41    np2=nptlxs
      do 42 ip=np1,np2
        call cxhdecas(ip,iret)
        if(iret.ne.0)goto 1001
42    continue
      np1=np2+1
      if(np1.le.nptlxs)goto 41


1000  return

1001  iret=1 
      goto 1000 

 1002 iret=-1     !elastic
      goto 1000 

      end

c------------------------------------------------------------------------------
      subroutine flk2cnx(id,P0,ir)
c------------------------------------------------------------------------------
c fill EPOS stack with particle from FLUKA
c P0 particle momentum
c id of particle
c ir =1 for remnant
c------------------------------------------------------------------------------
#include "conex.h"
#include "conex.incnex"
      double precision P0(5)

      xsptl(1,nptlxs)=P0(1)  !P_x
      xsptl(2,nptlxs)=P0(2)  !P_y
      xsptl(3,nptlxs)=P0(3)  !P_z
      xsptl(4,nptlxs)=P0(4)  !E
      xsptl(5,nptlxs)=P0(5)  !mass
      ityptlxs(nptlxs)=0
      if(ir.eq.0)then
        iorptlxs(nptlxs)=1
        jorptlxs(nptlxs)=maprojxs+matargxs
      else
        iorptlxs(nptlxs)=-1
        jorptlxs(nptlxs)=0
      endif
      ifrptlxs(1,nptlxs)=0
      ifrptlxs(2,nptlxs)=0
      xsorptl(1,nptlxs)=0.d0
      xsorptl(2,nptlxs)=0.d0
      xsorptl(3,nptlxs)=0.d0
      xsorptl(4,nptlxs)=0.d0
      xstivptl(1,nptlxs)=0.
      xstivptl(2,nptlxs)=0.
      idptlxs(nptlxs)=id
          
c Put particle in cms frame.
c      call cxutlob5(xsyhaha, xsptl(1,nptlxs), xsptl(2,nptlxs)
c     *, xsptl(3,nptlxs), xsptl(4,nptlxs), xsptl(5,nptlxs))
            
#ifdef __CXDEBUG__
      if(isx.ge.5)write(ifck,'(a,i5,a,i5,a,4(e11.4,1x),f6.3)')
     $         ' particle from FLUKA ',nptlxs,' id :',idptlxs(nptlxs)
     $         , ' momentum :',(xsptl(i,nptlxs),i=1,5)
#endif

      return
      end

c------------------------------------------------------------------------------
      double precision function fflucrse()
c------------------------------------------------------------------------------
c hadron-proton particle production cross section with Fluka.
c ekin0 - kinetic energy
c plab0 - particle momentum
c------------------------------------------------------------------------------
#include "conex.incnex"
      double precision EKIN1,PPROJ,SIGREA,ZLDUM
C-----------------------------------------------------------------------

C  CONVERT PARTICLE TYPE TO FLUKA
      KPROJ  = idtrafocx('nxs','flk',idprojxs)
      EKIN1  = xsekin
      PPROJ = xspnll
C  MATERIAL NUMBER 1 IS CURRENT MATERIAL
      MMAT  = 6
C  GET THE CROSS SECTION: SIGREA IS MICROSCOPIC CROSS SECTION (MB)
C  ZLDUM IS MACROSCOPIC CROSS SECTION (CM^-1)
      CALL SIGINM( KPROJ,MMAT,EKIN1,PPROJ,SIGREA,ZLDUM )
      fflucrse= SIGREA

      return
      end



c------------------------------------------------------------------------------
      double precision function flucrse(ek,mapro,matar,id)
c------------------------------------------------------------------------------
c inelastic cross section of FLUKA
c if id=0, target = air
c ek - kinetic energy in GeV
c maproj - projec mass number     (1<maproj<64)
c matarg - projec mass number
c id - target id (0=air)
c------------------------------------------------------------------------------
      implicit none
#include "conex.h"
#include "conex.incnex"
      double precision ek,SINE,AMPRO,AMTAR
      integer lapro,latar,mapro,matar,id,mt,lt,k
      flucrse=0d0
      lapro=max(1,mapro/2)
      latar=max(1,matar/2)
      if(id.eq.0)then
        do k=1,3
          mt=int(aira(k))
          lt=max(1,mt/2)
          call SIGIAA(lapro,mapro,AMPRO,lt,mt,AMTAR,ek,SINE)
          flucrse=flucrse+airw(k)*SINE
        enddo
      else
        call SIGIAA(lapro,mapro,AMPRO,latar,matar,AMTAR,ek,SINE)
        flucrse=SINE
      endif
      return
      end

#if !__CXCORSIKA__ && !__CORSIKA8__

C=======================================================================

      DOUBLE PRECISION FUNCTION FLRNDM()

C-----------------------------------------------------------------------
C  FL(UKA) R(A)ND(O)M (GENERATOR)
C  THIS FUNCTON IS CALLED FROM FLUKA ROUTINES.
C-----------------------------------------------------------------------
      double precision dum,drangen
      flrndm=drangen(dum)

      RETURN
      END
 
C=======================================================================

      SUBROUTINE ZEREMF

C-----------------------------------------------------------------------
C  THIS DUMMY SUBROUTINE IS NECESSARY TO OVERRIDE A FLUKA SUBROUTINE
C  WITH IDENTICAL NAME WHICH OTHERWISE WOULD ERASE SOME EPOS COMMONS.
C  THIS SUBROUTINE IS CALLED FROM ZEROIN.
C-----------------------------------------------------------------------

      INCLUDE '(DBLPRC)'
      INCLUDE '(DIMPAR)'
      INCLUDE '(IOUNIT)'
C-----------------------------------------------------------------------

      RETURN
      END

#endif
#endif
#ifdef __GHEISHA__
c Last modifications 03.07.2020 Compatibility with CORSIKA8 by T.Pierog
c 07.04.2008 Compatibility gcc4
c 18.01.2007 update to compile with CORSIKA with GHEISHA in double precision
c 19.05.2004 Link routines between Gheisha and Conex.
c author T. Pierog

#ifdef __STD__
#define __GHEISHA__
#define __QGSJET__
#define __ANALYSIS__
#endif

c-----------------------------------------------------------------------
      subroutine IniGheisha
c-----------------------------------------------------------------------
c Primary initialization for Gheisha
c-----------------------------------------------------------------------
      implicit double precision (a-h,o-z)
#include "conex.incnex"
      COMMON /GHECRELABCT/ELCUT
      DOUBLE PRECISION ELCUT(4)
      data init/0/
      save init

      if(init.ge.1)return
      init=init+1

#ifdef __CXDEBUG__
      call utisx1('inighe  ',4)
      write(*,'(a)')'initialize Gheisha ...'
#endif

c common model parameters setting
      call nghini
      xsegymin=0.05d0
      elcut(1)=enymin

#ifdef __CXDEBUG__
      call utisx2
#endif
      end

c-----------------------------------------------------------------------
      subroutine IniEvtGhe
c-----------------------------------------------------------------------
c Initialization for each type of event (for given proj, targ and egy)
c-----------------------------------------------------------------------
      implicit double precision (a-h,o-z)
#include "conex.h"
#include "conex.incnex"
c Gheisha Common
      real anquasiel
      common/ghecsquel/anquasiel,iquasiel

        anquasiel=0.
        if(maprojxs.gt.1)
     &  stop'Nucleus too big for Gheisha (Mmax=1) !'
        xsbminim=0.d0
        xsbmaxim=10000.d0
        idp=(idprojxs/10)*10  !convert to bound state
        icp=idtrafocx('nxs','ghe',idp)
        if(icp.eq.8)icp=icp-nint(sign(1.d0,drangen(dummy)-0.5d0)) !pi0 (or rho0) = pi+ or pi-
        ict=idtrafocx('nxs','ghe',idtargxs)
        xsgheincs=CxGheSig(xspnll,xsekin,icp,ict
     &                   ,matargxs,latargxs)
        if(xsekin.lt.xsegymin)xsgheincs=0.d0           !below egymin, no interaction
        xsbmax=20.d0
#ifdef __CXDEBUG__
        if(isx.ge.2)write(ifck,*)
     &  'Gheisha used with (E,pz,proj,targ(A,Z))',xselab,xspnll,icp,ict
     &  ,'(',matargxs,',',latargxs,')'
#endif

      return
      end

c------------------------------------------------------------------------------
      subroutine ghecrse(ek,idpro,idtar,latar,matar,sigi,sige)
c------------------------------------------------------------------------------
c inelastic and elastic cross section from gheisha
c ek - kinetic lab energy
c idpro - id of projectile
c idtar - id of target(idtar=0 corresponds to air)
c latar - charge of target
c matar - mass of target
c output :
c   sigi - inelastic cross section
c   sige - elastic cross section
c------------------------------------------------------------------------------
      implicit double precision (a-h,o-z)
#include "conex.incnex"
c Gheisha Common
      COMMON/GSECTI/ AIEL(20),AIIN(20),AIFI(20),AICA(20),ALAM,K0FLAG
      DOUBLE PRECISION AIEL,AIIN,AIFI,AICA,ALAM
      INTEGER K0FLAG
      PARAMETER (KKK=3)
#if !__CXCORSIKA__ && !__CORSIKA8__

      COMMON /GHECGCOMP/ ACOMP,ZCOMP,WCOMP,KK
#else
      COMMON /CGCOMP/ ACOMP,ZCOMP,WCOMP,KK
#endif
      DOUBLE PRECISION ACOMP(KKK),ZCOMP(KKK),WCOMP(KKK)

      call cxidmass(idpro,amp)
      p=sqrt(max(0.d0,(ek+2.d0*amp)*ek))
      icp=idtrafocx('nxs','ghe',idpro)
      if(icp.eq.8)icp=icp-nint(sign(1.d0,drangen(dummy)-0.5d0))     !pi0 = pi+ or pi-
      ict=idtrafocx('nxs','ghe',idtar)
      sigi=CxGheSig(p,ek,icp,ict,matar,latar)
      sige=0.d0
      do k=1,kk
        sige=sige+AIEL(k)
      enddo

      return
      end

c-----------------------------------------------------------------------
      subroutine emsghe(iret)
c-----------------------------------------------------------------------
c  call gheisha to simulate interaction
C  itypr  = -1 REACTION CROSS SECTIONS NOT YET TABULATED/PROGRAMMED
C         =  0 NO INTERACTION
C         =  1 ELASTIC SCATTERING
C         =  2 INELASTIC SCATTERING
C         =  3 NUCLEAR FISSION WITH INELEASTIC SCATTERING
C         =  4 NEUTRON CAPTURE
c-----------------------------------------------------------------------
      implicit double precision (a-h,o-z)
#include "conex.h"
#include "conex.incnex"
      real anquasiel
      common/ghecsquel/anquasiel,iquasiel


      iret=0
      itypr=0

      if(abs(xsgheincs).lt.1d-20)goto 1001   !no interaction
#ifdef __CXDEBUG__
      if(isx.ge.4)call cxalist('Determine Gheisha Production&',0,0,0)
#endif
      ntry=0
 10   continue
      ntry=ntry+1
      if(ntry.gt.10)goto 1001   !no interaction

      ncolxs=1
      nevtxs=1
      nptlxs=0
      xsbimp=0.d0
      xsphi=0.d0

      ist=1
      call cxconre
      call cxconwr(ist)

      call CXGHEI(itypr)

      if(itypr.le.1)goto 1000   !no interaction

c Decay particles with short life time

      np1=1
41    np2=nptlxs
      do 42 ip=np1,np2
        call cxhdecas(ip,iret)
        if(iret.ne.0)goto 1001
42    continue
      np1=np2+1
      if(np1.le.nptlxs)goto 41

      mlim=0
      nmes=0
      if(iabs(idprojxs).le.1000)mlim=1
      do i=1,nptlxs
        if(istptlxs(i).eq.0.and.iabs(idptlxs(i)).lt.1000)nmes=nmes+1
      enddo
      if(nmes.eq.mlim)itypr=5

      if(itypr.eq.5)then
        if(ntry.eq.1)anquasiel=anquasiel+1.
#ifdef __CXDEBUG__
        if(isx.ge.2)write(ifck,*)'Quasi-elastic event'
#endif
        if(iquasiel.eq.0)then
          if(ntry.le.100)then
            goto 10
          else
            nptlxs=0
          endif
        endif
      endif

      if(itypr.eq.-1)stop'Problem in Gheisha'

1000  return

1001  iret=1 
      goto 1000 

      end

c--------------------------------------------------------------------------
      subroutine cx2ghe(idghe,pnex)
c--------------------------------------------------------------------------
      implicit double precision (a-h,o-z)
#include "conex.h"
#include "conex.incnex"
      dimension pnex(5)
      idnex=idptlxs(1)
      idghe=idtrafocx('nxs','ghe',idnex)
      if(idghe.eq.8)idghe=idghe-nint(sign(1.d0,drangen(dummy)-0.5d0)) !pi0 = pi+ or pi-
      do ii=1,2
        pnex(ii)=0.d0
      enddo
      pnex(3)=xspnll
      pnex(4)=xselab
      pnex(5)=xsamproj
#ifdef __CXDEBUG__
      if(isx.ge.5)write(ifck,'(a,i5,a,5(e11.4,1x))')
     $       ' Initial Gheisha particle, id :',idghe
     $     , ' momentum :',(pnex(k),k=1,5)
#endif
      end

c--------------------------------------------------------------------------
      subroutine ghe2cx(idghe,pnex)
c--------------------------------------------------------------------------
c  Put gheisha particle with momentum pnex into conex stack
c  imod=1 put new particle
c--------------------------------------------------------------------------
      implicit double precision (a-h,o-z)
#include "conex.h"
#include "conex.incnex"
      dimension pnex(5)

      idnex=idtrafocx('ghe','nxs',idghe)
      nptlxs=nptlxs+1
      if(nptlxs.gt.mxptlxs)stop'gheisha: mxptlxs too small'
      istptlxs(nptlxs)=0
      ifrptlxs(1,nptlxs)=0
      ifrptlxs(2,nptlxs)=0
      idptlxs(nptlxs)=idnex
      P=0.d0
      do ii=1,5
        xsptl(ii,nptlxs)=pnex(ii)
        P=P+pnex(ii)*pnex(ii)
      enddo
      xsptl(4,nptlxs)=SQRT(P)
      ityptlxs(nptlxs)=0
      iorptlxs(nptlxs)=1
      jorptlxs(nptlxs)=maprojxs+matargxs
      xsorptl(1,nptlxs)=0.d0
      xsorptl(2,nptlxs)=0.d0
      xsorptl(3,nptlxs)=0.d0
      xsorptl(4,nptlxs)=0.d0
      xstivptl(1,nptlxs)=0.d0
      xstivptl(2,nptlxs)=0.d0

c Put particle in cms frame.
c      call cxutlob5(xsyhaha, xsptl(1,nptlxs), xsptl(2,nptlxs)
c     *, xsptl(3,nptlxs), xsptl(4,nptlxs), xsptl(5,nptlxs))

#ifdef __CXDEBUG__
      if(isx.ge.5)write(ifck,'(a,i5,a,i5,a,4(e11.4,1x),f6.3)')
     $       ' particle from Gheisha ',nptlxs,' id :',idptlxs(nptlxs)
     $  , ' momentum :',(xsptl(k,nptlxs),k=1,5)
#endif

      end

c-----------------------------------------------------------------------
*CMZ :          24/04/2003  by  T. PIEROG IK FZK KARLSRUHE
*-- Author :    The CONEX development group   24/04/2003
C=======================================================================

      SUBROUTINE CXGHEI(INTE)

C-----------------------------------------------------------------------
C  C(ONE)X GHE(ISHA) I(NTERFACE)
C
C  MAIN STEERING SUBROUT. FOR HADRON PACKAGE GHEISHA ***
C  THIS SUBROUTINE IS CALLED FROM EMSGHE.
C
C  RETURN INTE (SEE DEFINITION BELOW)
C
C  ORIGIN  : F.CARMINATI, H.FESEFELDT (SUBROUT. GHESIG)
C  REDESIGN FOR CORSIKA : P. GABRIEL IK1  FZK KARLSRUHE
C  REDESIGN FOR CONEX : T. PIEROG IK3  FZK KARLSRUHE
C-----------------------------------------------------------------------

      IMPLICIT DOUBLE PRECISION(A-H, O-Z)
      dimension pnex(5)
      common/cxair/airz(3),aira(3),airw(3),airavz,airava,airi(3)
*KEEP,CGCOMP.
      PARAMETER (KKK=3)
#if !__CXCORSIKA__ && !__CORSIKA8__

      COMMON /GHECGCOMP/ ACOMP,ZCOMP,WCOMP,KK
#else
      COMMON /CGCOMP/ ACOMP,ZCOMP,WCOMP,KK
#endif
      DOUBLE PRECISION ACOMP(KKK),ZCOMP(KKK),WCOMP(KKK)
*KEEP,ELABCT.
#if !__CXCORSIKA__ && !__CORSIKA8__

      COMMON /GHECRELABCT/ELCUT
#else
      COMMON /CRELABCT/ELCUT
#endif
      DOUBLE PRECISION ELCUT(4)
c$$$*KEEP,RUNPAR.
      COMMON /CONEXDBINC/MCXDBUG
      INTEGER          MCXDBUG

      COMMON/GSECTI/   AIEL(20),AIIN(20),AIFI(20),AICA(20),ALAM,K0FLAG
      DOUBLE PRECISION AIEL,AIIN,AIFI,AICA,ALAM
      INTEGER          K0FLAG

C --- GHEISHA COMMONS ---
      PARAMETER (MXGKGH=100)
      PARAMETER (MXGKPV=MXGKGH)
      COMMON /VECUTY/  PV(10,MXGKPV)

      COMMON /CONSTS/  PI,TWPI,PIBTW,MP,MPI,MMU,MEL,MKCH,MK0,SMP,SMPI,
     $                 SMU,CT,CTKCH,CTK0,
     $                 ML0,MSP,MS0,MSM,MX0,MXM,CTL0,CTSP,CTSM,CTX0,CTXM,
     $                 RMASS(35),RCHARG(35)

      DOUBLE PRECISION MP,MPI,MMU,MEL,MKCH,MK0,
     *                 ML0,MSP,MS0,MSM,MX0,MXM

      PARAMETER (MXEVEN=12*MXGKGH)
      COMMON /GEVENT /  NSIZE,NCUR,NEXT,NTOT,EVE(MXEVEN)

      COMMON /PRNTFL/  INBCD,NEWBCD,INBIN,NEWBIN,NPEVT,NEVTP,
     *                 LPRT,NPRT(10)
      LOGICAL          LPRT,NPRT


C --- "NEVENT" CHANGED TO "KEVENT" IN COMMON /CURPAR/ DUE TO CLASH ---
C --- WITH VARIABLE "NEVENT" IN GEANT COMMON ---

      PARAMETER (MXGKCU=MXGKGH)
      COMMON /CURPAR/  WEIGHT(10),DDELTN,IFILE,IRUN,NEVT,KEVENT,SHFLAG,
     $                 ITHST,ITTOT,ITLST,IFRND,TOFCUT,CMOM(5),CENG(5),
     $                 RS,S,ENP(10),NP,NM,NN,NR,NO,NZ,IPA(MXGKCU),
     $                 ATNO2,ZNO2

C --- "IPART" CHANGED TO "KPART" IN COMMON /RESULT/ DUE TO CLASH ---
C --- WITH VARIABLE "IPART" IN GEANT COMMON ---

      COMMON /RESULT/  XEND,YEND,ZEND,RCA,RCE,AMAS,NCH,TOF,PX,PY,PZ,
     $                 USERW,INTCT,P,EN,EK,AMASQ,DELTN,ITK,NTK,KPART,
     $                 IND,LCALO,ICEL,SINL,COSL,SINP,COSP,
     $                 XOLD,YOLD,ZOLD,POLD,PXOLD,PYOLD,PZOLD,
     $                 XSCAT,YSCAT,ZSCAT,PSCAT,PXSCAT,PYSCAT,PZSCAT
      DOUBLE PRECISION NCH,INTCT

C --- "ABSL(21)" CHANGED TO "ABSLTH(21)" IN COMMON /MAT/ DUE TO CLASH ---
C --- WITH VARIABLE "ABSL" IN GEANT COMMON ---

      COMMON /MAT   /  DEN(21),RADLTH(21),ATNO(21),ZNO(21),ABSLTH(21),
     *                 CDEN(21),X0DEN(21),X1DEN(21),RION(21),
     *                 FRAC1(21,10),DEN1(21,10),ATNO1(21,10),
     *                 ZNO1(21,10),
     *                 PARMAT(21,10),MATID(21),MATID1(21,24),MDEN(21),
     *                 IFRAT,IFRAC(21),LMAT

*     DIMENSION IPELOS(35)
c$$$      REAL EMAX,EEESQ

      DIMENSION RNDM(1)

c$$$      DIMENSION KIPART(48),IKPART(35)
c$$$C --- ANGLES FOR NEW COUPLING WITH CORSIKA D. HECK DEC. 2000
c$$$      DOUBLE PRECISION PHIRAN,PHIG,THETG

      SAVE

C --- DATA STMTS. FOR GEANT/GHEISHA PARTICLE CODE CONVERSIONS ---
C --- KIPART(I)=GHEISHA CODE CORRESPONDING TO GEANT   CODE I ---
C --- IKPART(I)=GEANT   CODE CORRESPONDING TO GHEISHA CODE I ---

c$$$      DATA KIPART/
c$$$     $               1,   3,   4,   2,   5,   6,   8,   7,
c$$$     $               9,  12,  10,  13,  16,  14,  15,  11,
c$$$     $              35,  18,  20,  21,  22,  26,  27,  33,
c$$$     $              17,  19,  23,  24,  25,  28,  29,  34,
c$$$     $              35,  35,  35,  35,  35,  35,  35,  35,
c$$$     $              35,  35,  35,  35,  30,  31,  32,  35/
c$$$
c$$$      DATA IKPART/
c$$$     $               1,   4,   2,   3,   5,   6,   8,   7,
c$$$     $               9,  11,  16,  10,  12,  14,  15,  13,
c$$$     $              25,  18,  26,  19,  20,  21,  27,  28,
c$$$     $              29,  22,  23,  30,  31,  45,  46,  47,
c$$$     $              24,  32,  48/


C --- DENOTE STABLE PARTICLES ACCORDING TO GHEISHA CODE ---
C --- STABLE : GAMMA, NEUTRINO, ELECTRON, PROTON AND HEAVY FRAGMENTS ---
C --- WHEN STOPPING THESE PARTICLES ONLY LOOSE THEIR KINETIC ENERGY ---
*     DATA IPELOS/
*    $             1,   1,   0,   1,   0,   0,   0,   0,
*    $             0,   0,   0,   0,   0,   1,   0,   0,
*    $             0,   0,   0,   0,   0,   0,   0,   0,
*    $             0,   0,   0,   0,   0,   1,   1,   1,
*    $             0,   0,   1/

C --- LOWERBOUND OF KINETIC ENERGY BIN IN N CROSS-SECTION TABLES ---
      DATA TEKLOW /0.0001D0/

C --- KINETIC ENERGY TO SWITCH FROM "CASN" TO "GNSLWD" FOR N CASCADE ---
      DATA SWTEKN /0.05D0/
C-----------------------------------------------------------------------

C --- INITIALIZE RELEVANT GHEISHA VARIABLES ---

      call cx2ghe(KPART,pnex)



C  MIXING OF NEUTRAL KAONS
      IF ( KPART .EQ. 11  .OR.  KPART .EQ. 12 ) THEN
        CALL GRNDM( RNDM,1 )
        IF ( RNDM(1) .LT. 0.5 ) THEN
          KPART = 11
        ELSE
          KPART = 12
        ENDIF
      ENDIF
      KKPART = KPART

C --- TRANSPORT THE TRACK NUMBER TO GHEISHA AND INITIALIZE SOME NUMBERS
C --- NTK=ITRA   ITRA = CURRENT TRACK NUMBER IN GEANT (GCKINE)
      NTK   = 0
      INTCT = 0.0D0
      NEXT  = 1
      NTOT  = 0
      INTE  = 0
      TOF   = 0.0D0

c$$$C --- RESET ITYPE
c$$$      SECPAR(1) = 0.D0

C --- FILL RESULT COMMON FOR THIS TRACK WITH CORSIKA VALUES ---

c      iptl=1
      AMAS  = RMASS(KPART)
      NCH   = RCHARG(KPART)
      XEND  = 0.D0  !xorptl(1,iptl)
      YEND  = 0.D0  !xorptl(2,iptl)
      ZEND  = 0.D0  !xorptl(3,iptl)
      USERW = 0.0D0

      P=sqrt(pnex(1)**2+pnex(2)**2+pnex(3)**2)
      PX = pnex(1)/P           
      PY = pnex(2)/P          
      PZ = pnex(3)/P         

      AMASQ=AMAS*AMAS
      EN = SQRT(AMASQ+P*P)
      EK = ABS ( EN - ABS(AMAS) )

      if(abs(P-SQRT((EN-AMAS)*(EN+AMAS))).gt.1.d-3)
     $write(MCXDBUG,'("On-Shell problem in Gheisha:  ",2(g12.6,1x),i3)')
     $ P,SQRT((EN-AMAS)*(EN+AMAS)),kpart


#ifdef __CXDEBUG__
      if(nprt(9))
     $     write(MCXDBUG,'("in:  ",5(g12.6,1x),i3)') P*PX,P*PY,P*PZ,EN
     $     ,sqrt(amasq),kpart
#endif

      SINL=0.0D0
      COSL=1.0D0
      SINP=0.0D0
      COSP=1.0D0
C
      IF (ABS(P) .LE. 1.0E-10) GO TO 1
      SINL=PZ
      COSL=SQRT(ABS(1.0-SINL**2))
C
 1    CONTINUE
      CALL GRNDM(RNDM,1)
      PHI=RNDM(1)*TWPI
      IF ((PX .EQ. 0.0D0) .AND. (PY .EQ. 0.0D0)) GOTO 3
      IF (ABS(PX) .LT. 1.D-10) GOTO 2
      PHI=ATAN2(PY,PX)
      GOTO 3
C
 2    CONTINUE
      IF (PY .GT. 0.0D0) PHI=PI/2.0D0
      IF (PY .LE. 0.0D0) PHI=3.0*PI/2.0D0
C
 3    CONTINUE
      SINP=SIN(PHI)
      COSP=COS(PHI)

C --- SET GHEISHA INDEX FOR THE CURRENT MEDIUM ALWAYS TO 1 ---
      IND = 1

C --- TRANSFER GLOBAL MATERIAL CONSTANTS FOR CURRENT MEDIUM ---
C --- DETAILED DATA FOR COMPOUNDS IS OBTAINED VIA SUBROUT. COMPO ---

      ATNO(IND+1)  = airava !14.56        !atnxs, ztnxs : see cxghesig
      ZNO(IND+1)   = airavz ! 7.265
      DEN(IND+1)   = 0.0D0
      RADLTH(IND+1)= 0.0D0
      ABSLTH(IND+1)= 0.0D0

C --- SETUP PARMAT FOR PHYSICS STEERING ---
      PARMAT(IND+1,10)=0.0D0

c  5   CONTINUE

C --- INDICATE LIGHT (<= PI) AND HEAVY PARTICLES (HISTORICALLY) ---
C --- CALIM CODE ---
      J = 2
      TEST = RMASS(7)-0.001D0
      IF (ABS(AMAS) .LT. TEST) J=1

C *** DIVISION INTO VARIOUS INTERACTION CHANNELS DENOTED BY "INTE" ***
C THE CONVENTION FOR "INT" IS THE FOLLOWING

C INTE  = -1 REACTION CROSS-SECTIONS NOT YET TABULATED/PROGRAMMED
C       =  0 NO INTERACTION
C       =  1 ELASTIC SCATTERING
C       =  2 INELASTIC SCATTERING
C       =  3 NUCLEAR FISSION WITH INELASTIC SCATTERING
C       =  4 NEUTRON CAPTURE

C --- INTACT CODE ---
      ALAM1 = 0.0D0
      CALL GRNDM( RNDM,1 )
      RAT   = RNDM(1)*ALAM

C ---  DEFAULT VALUES FOR AIR
      ATNO2 = airava !14.56
      ZNO2  = airavz !7.265

      DO  K = 1, KK
        ATNO2 = ACOMP(K)
        ZNO2  = ZCOMP(K)

C --- TRY FOR ELASTIC SCATTERING ---            !elastic scattering not use
ctp        INTE = 1
ctp        ALAM1 = ALAM1+AIEL(K)
ctp        IF (RAT .LT. ALAM1) GOTO 8

C --- TRY FOR INELASTIC SCATTERING ---
        INTE = 2
        ALAM1 = ALAM1+AIIN(K)
        IF (RAT .LT. ALAM1) GOTO 8

C --- TRY FOR NEUTRON CAPTURE ---               !neutron capture scattering not use
ctp        INTE = 4
ctp        ALAM1 = ALAM1+AICA(K)
ctp        IF (RAT .LT. ALAM1) GOTO 8

      ENDDO

C --- NO REACTION SELECTED ==> ELASTIC SCATTERING ---
      INTE = 1

C *** TAKE ACTION ACCORDING TO SELECTED REACTION CHANNEL ***
C --- FOLLOWING CODE IS A TRANSLATION OF "CALIM" INTO GEANT JARGON ---

 8    CONTINUE
#ifdef __CXDEBUG__
      IF (NPRT(9)) WRITE(MCXDBUG,1001) INTE
 1001 FORMAT(' *CXGHEI* INTERACTION TYPE CHOSEN INTE = ',I3)
#endif

      IF (INTE .NE. 4) GOTO 10

C --- NEUTRON CAPTURE ---
#ifdef __CXDEBUG__
      IF (NPRT(9)) WRITE(MCXDBUG,2000)
 2000 FORMAT(' *CXGHEI* SUBROUT. CAPTUR WILL BE CALLED')
#endif
      CALL CAPTUR( NOPT )
      GOTO 40

 10   CONTINUE
      IF (INTE .NE. 3) GO TO 11
C --- NUCLEAR FISSION ---
#ifdef __CXDEBUG__
      IF (NPRT(9)) PRINT 2001
 2001 FORMAT(' *GHEISH* ROUTINE FISSIO WILL BE CALLED')
#endif
c      ISTOP=1
      TKIN=FISSIO(EK)
      GO TO 40

 11   CONTINUE

C --- ELASTIC AND INELASTIC SCATTERING ---
      PV(1,MXGKPV) = P*PX
      PV(2,MXGKPV) = P*PY
      PV(3,MXGKPV) = P*PZ
      PV(4,MXGKPV) = EN
      PV(5,MXGKPV) = AMAS
      PV(6,MXGKPV) = NCH
      PV(7,MXGKPV) = TOF
      PV(8,MXGKPV) = KPART
      PV(9,MXGKPV) = 0.D0
      PV(10,MXGKPV)= USERW

C --- ADDITIONAL PARAMETERS TO SIMULATE FERMI MOTION AND EVAPORATION ---
      DO  JENP = 1, 10
        ENP(JENP) = 0.D0
      ENDDO
      ENP(5) = EK
      ENP(6) = EN
      ENP(7) = P

      IF (INTE .NE. 1) GOTO 12

C *** ELASTIC SCATTERING PROCESSES ***

C --- ONLY NUCLEAR INTERACTIONS FOR HEAVY FRAGMENTS ---
      IF ((KPART .GE. 30) .AND. (KPART .LE. 32)) GOTO 35

C --- NORMAL ELASTIC SCATTERING FOR LIGHT MEDIA ---
      IF (ATNO2 .LT. 1.5D0) GOTO 35

C --- COHERENT ELASTIC SCATTERING FOR HEAVY MEDIA ---
#ifdef __CXDEBUG__
      IF (NPRT(9)) WRITE(MCXDBUG,2002)
 2002 FORMAT(' *CXGHEI* SUBROUT. COSCAT WILL BE CALLED')
#endif
      CALL COSCAT
      GOTO 40

C *** NON-ELASTIC SCATTERING PROCESSES ***
 12   CONTINUE

C --- ONLY NUCLEAR INTERACTIONS FOR HEAVY FRAGMENTS ---
      IF ((KPART .GE. 30) .AND. (KPART .LE. 32)) GOTO 35

C *** USE SOMETIMES NUCLEAR REACTION SUBROUT. "NUCREC" FOR LOW ENERGY
C *** PROTON AND NEUTRON SCATTERING ***
      CALL GRNDM( RNDM,1 )
      TEST1 = RNDM(1)
      TEST2 = 4.5D0*(EK-0.01D0)
      IF ((KPART .EQ. 14) .AND. (TEST1 .GT. TEST2)) GOTO 85
      IF ((KPART .EQ. 16) .AND. (TEST1 .GT. TEST2)) GOTO 86

C *** FERMI MOTION AND EVAPORATION ***
      TKIN = CINEMA(EK)
      PV(9,MXGKPV) = TKIN
      ENP(5) = EK+TKIN
C --- CHECK FOR LOWERBOUND OF EKIN IN CROSS-SECTION TABLES ---
      IF (ENP(5) .LE. TEKLOW) ENP(5)=TEKLOW
      ENP(6) = ENP(5)+ABS(AMAS)
      ENP(7) = (ENP(6)-AMAS)*(ENP(6)+AMAS)
      ENP(7) = SQRT(ABS(ENP(7)))
      TKIN   = FERMIG(ENP(5))
      ENP(5) = ENP(5)+TKIN
C --- CHECK FOR LOWERBOUND OF EKIN IN CROSS-SECTION TABLES ---
      IF (ENP(5) .LE. TEKLOW) ENP(5)=TEKLOW
      ENP(6) = ENP(5)+ABS(AMAS)
      ENP(7) = (ENP(6)-AMAS)*(ENP(6)+AMAS)
      ENP(7) = SQRT(ABS(ENP(7)))
      TKIN   = EXNU(ENP(5))
      ENP(5) = ENP(5)-TKIN
C --- CHECK FOR LOWERBOUND OF EKIN IN CROSS-SECTION TABLES ---
      IF (ENP(5) .LE. TEKLOW) ENP(5)=TEKLOW
      ENP(6) = ENP(5)+ABS(AMAS)
      ENP(7) = (ENP(6)-AMAS)*(ENP(6)+AMAS)
      ENP(7) = SQRT(ABS(ENP(7)))

C *** IN CASE OF ENERGY ABOVE CUT-OFF LET THE PARTICLE CASCADE ***
      IF ( ENP(5) .GT. ELCUT(1)) GOTO 35

C --- SECOND CHANCE FOR ANTI-BARYONS DUE TO POSSIBLE ANNIHILATION ---
      IF ((AMAS .GE. 0.0D0) .OR. (KPART .LE. 14)) GOTO 13
      ANNI = 1.3D0*P
      IF (ANNI .GT. 0.4D0) ANNI=0.4D0
      CALL GRNDM( RNDM,1 )
      TEST = RNDM(1)
      IF (TEST .GT. ANNI) GOTO 35

C *** PARTICLE WITH ENERGY BELOW CUT-OFF ***
C --- ==> ONLY NUCLEAR EVAPORATION AND QUASI-ELASTIC SCATTERING ---
 13   CONTINUE

#ifdef __CXDEBUG__
      IF (NPRT(9))WRITE(MCXDBUG,1002) KPART,EK,EN,P,ENP(5),ENP(6),ENP(7)
 1002 FORMAT(' *CXGHEI* ENERGY BELOW CUT-OFF FOR GHEISHA PARTICLE ',I3/
     $ ' EK,EN,P,ENP(5),ENP(6),ENP(7) = ',6(G12.5,1X))
#endif

ctp We don't wan't nuclear evaporation or quasi-elastic scattering -> no interaction
ctp      GOTO 40


      IF ((KPART .NE. 14) .AND. (KPART .NE. 16)) GOTO 14
      IF (KPART .EQ. 16) GOTO 86
 
C --- SLOW PROTON ---
 85   CONTINUE
#ifdef __CXDEBUG__
      IF (NPRT(9)) WRITE(MCXDBUG,2003) EK,KPART
 2003 FORMAT(' *CXGHEI* SUBROUT. NUCREC WILL BE CALLED',
     $ ' EK = ',G12.5,' GEV  KPART = ',I3)
#endif
      CALL NUCREC( NOPT,2 )

      IF (NOPT .NE. 0) GOTO 50

#ifdef __CXDEBUG__
      IF (NPRT(9)) WRITE(MCXDBUG,2004)EK,KPART
 2004 FORMAT(' *CXGHEI* SUBROUT. COSCAT WILL BE CALLED',
     $ ' EK = ',G12.5,' GEV  KPART = ',I3)
#endif
      CALL COSCAT
      GOTO 40

C --- SLOW NEUTRON ---
 86   CONTINUE
#ifdef __CXDEBUG__
      IF (NPRT(9)) WRITE(MCXDBUG,2015)
#endif
      NUCFLG = 0
      CALL GNSLWD( NUCFLG,INTE,NFL,TEKLOW )
      IF (NUCFLG .NE. 0) GOTO 50
      GOTO 40

C --- OTHER SLOW PARTICLES ---
 14   CONTINUE
      IPA(1) = KPART
C --- DECIDE FOR PROTON OR NEUTRON TARGET ---
      IPA(2) = 16
      CALL GRNDM( RNDM,1 )
      TEST1 = RNDM(1)
      TEST2 = ZNO2/ATNO2
      IF (TEST1 .LT. TEST2) IPA(2)=14
      AVERN = 0.0D0
      NFL = 1
      IF (IPA(2) .EQ. 16) NFL=2
      IPPP = KPART
#ifdef __CXDEBUG__
      IF (NPRT(9)) WRITE(MCXDBUG,2005)
 2005 FORMAT(' *CXGHEI* SUBROUT. TWOB WILL BE CALLED')
#endif
      CALL TWOB( IPPP,NFL,AVERN )
      GOTO 40

C --- INITIALIZATION OF CASCADE QUANTITIES ---
 35   CONTINUE

C *** CASCADE GENERATION ***
C --- CALCULATE FINAL STATE MULTIPLICITY AND LONGITUDINAL AND ---
C --- TRANSVERSE MOMENTUM DISTRIBUTIONS ---

C --- FIXED PARTICLE TYPE TO STEER THE CASCADE ---
      KKPART = KPART

C --- NO CASCADE FOR LEPTONS ---
      IF (KKPART .LE. 6) GOTO 9999

C *** WHAT TO DO WITH "NEW PARTICLES" FOR GHEISHA ?????? ***
C --- RETURN FOR THE TIME BEING ---
      IF (KKPART .GE. 35) GOTO 9999

C --- CASCADE OF HEAVY FRAGMENTS
      IF ((KKPART .GE. 30) .AND. (KKPART .LE. 32)) GOTO 390

C --- INITIALIZE THE IPA ARRAY ---
*     CALL VZERO( IPA(1),MXGKCU )
CDH
      DO  III = 1, MXGKCU
        IPA(III) = 0
      ENDDO


C --- CASCADE OF OMEGA - AND OMEGA - BAR ---
      IF (KKPART .EQ. 33) GOTO 330
      IF (KKPART .EQ. 34) GOTO 331

      NVEPAR = KKPART-17
      IF (NVEPAR .LE. 0) GOTO 15
      GOTO (318,319,320,321,322,323,324,325,326,327,328,329),NVEPAR

 15   CONTINUE
      NVEPAR = KKPART-6
      GOTO (307,308,309,310,311,312,313,314,315,316,317,318),NVEPAR

C --- PI+ CASCADE ---
 307  CONTINUE
#ifdef __CXDEBUG__
      IF (NPRT(9)) WRITE(MCXDBUG,2006)
 2006 FORMAT(' *CXGHEI* SUBROUT. CASPIP WILL BE CALLED')
#endif
      CALL CASPIP( J,INTE,NFL )
      GOTO 40

C --- PI0 ==> NO CASCADE ---
 308  CONTINUE
      GOTO 40

C --- PI- CASCADE ---
 309  CONTINUE
#ifdef __CXDEBUG__
      IF (NPRT(9)) WRITE(MCXDBUG,2007)
 2007 FORMAT(' *CXGHEI* SUBROUT. CASPIM WILL BE CALLED')
#endif
      CALL CASPIM( J,INTE,NFL )
      GOTO 40

C --- K+ CASCADE ---
 310  CONTINUE
#ifdef __CXDEBUG__
      IF (NPRT(9)) WRITE(MCXDBUG,2008)
 2008 FORMAT(' *CXGHEI* SUBROUT. CASKP WILL BE CALLED')
#endif
      CALL CASKP( J,INTE,NFL )
      GOTO 40

C --- K0 CASCADE ---
 311  CONTINUE
#ifdef __CXDEBUG__
      IF (NPRT(9)) WRITE(MCXDBUG,2009)
 2009 FORMAT(' *CXGHEI* SUBROUT. CASK0 WILL BE CALLED')
#endif
      CALL CASK0( J,INTE,NFL )
      GOTO 40

C --- K0 BAR CASCADE ---
 312  CONTINUE
#ifdef __CXDEBUG__
      IF (NPRT(9)) WRITE(MCXDBUG,2010)
 2010 FORMAT(' *CXGHEI* SUBROUT. CASK0B WILL BE CALLED')
#endif
      CALL CASK0B( J,INTE,NFL )
      GOTO 40

C --- K- CASCADE ---
 313  CONTINUE
#ifdef __CXDEBUG__
      IF (NPRT(9)) WRITE(MCXDBUG,2011)
 2011 FORMAT(' *CXGHEI* SUBROUT. CASKM WILL BE CALLED')
#endif
      CALL CASKM( J,INTE,NFL )
      GOTO 40

C --- PROTON CASCADE ---
 314  CONTINUE
#ifdef __CXDEBUG__
      IF (NPRT(9)) WRITE(MCXDBUG,2012)
 2012 FORMAT(' *CXGHEI* SUBROUT. CASP WILL BE CALLED')
#endif
      CALL CASP( J,INTE,NFL )
      GOTO 40

C --- PROTON BAR CASCADE ---
 315  CONTINUE
#ifdef __CXDEBUG__
      IF (NPRT(9)) WRITE(MCXDBUG,2013)
 2013 FORMAT(' *CXGHEI* SUBROUT. CASPB WILL BE CALLED')
#endif
      CALL CASPB( J,INTE,NFL )
      GOTO 40

C --- NEUTRON CASCADE ---
 316  CONTINUE
      NUCFLG = 0
      IF (EK .GT. SWTEKN) THEN
         CALL CASN( J,INTE,NFL )
#ifdef __CXDEBUG__
         IF (NPRT(9)) WRITE(MCXDBUG,2014)
 2014 FORMAT(' *CXGHEI* SUBROUT. CASN WILL BE CALLED')
#endif
      ELSE
         CALL GNSLWD( NUCFLG,INTE,NFL,TEKLOW )
#ifdef __CXDEBUG__
         IF (NPRT(9)) WRITE(MCXDBUG,2015)
 2015 FORMAT(' *CXGHEI* SUBROUT. GNSLWD WILL BE CALLED')
#endif
      ENDIF
      IF (NUCFLG .NE. 0) GOTO 50
      GOTO 40

C --- NEUTRON BAR CASCADE ---
 317  CONTINUE
#ifdef __CXDEBUG__
      IF (NPRT(9)) WRITE(MCXDBUG,2016)
 2016 FORMAT(' *CXGHEI* SUBROUT. CASNB WILL BE CALLED')
#endif
      CALL CASNB( J,INTE,NFL )
      GOTO 40

C --- LAMBDA CASCADE ---
 318  CONTINUE
#ifdef __CXDEBUG__
      IF (NPRT(9)) WRITE(MCXDBUG,2017)
 2017 FORMAT(' *CXGHEI* SUBROUT. CASL0 WILL BE CALLED')
#endif
      CALL CASL0( J,INTE,NFL )
      GOTO 40

C --- LAMBDA BAR CASCADE ---
 319  CONTINUE
#ifdef __CXDEBUG__
      IF (NPRT(9)) WRITE(MCXDBUG,2018)
 2018 FORMAT(' *CXGHEI* SUBROUT. CASAL0 WILL BE CALLED')
#endif
      CALL CASAL0( J,INTE,NFL )
      GOTO 40

C --- SIGMA + CASCADE ---
 320  CONTINUE
#ifdef __CXDEBUG__
      IF (NPRT(9)) WRITE(MCXDBUG,2019)
 2019 FORMAT(' *CXGHEI* SUBROUT. CASSP WILL BE CALLED')
#endif
      CALL CASSP( J,INTE,NFL )
      GOTO 40

C --- SIGMA 0 ==> NO CASCADE ---
 321  CONTINUE
      GOTO 40

C --- SIGMA - CASCADE ---
 322  CONTINUE
#ifdef __CXDEBUG__
      IF (NPRT(9)) WRITE(MCXDBUG,2020)
 2020 FORMAT(' *CXGHEI* SUBROUT. CASSM WILL BE CALLED')
#endif
      CALL CASSM( J,INTE,NFL )
      GOTO 40

C --- SIGMA + BAR CASCADE ---
 323  CONTINUE
#ifdef __CXDEBUG__
      IF (NPRT(9)) WRITE(MCXDBUG,2021)
 2021 FORMAT(' *CXGHEI* SUBROUT. CASASP WILL BE CALLED')
#endif
      CALL CASASP( J,INTE,NFL )
      GOTO 40

C --- SIGMA 0 BAR ==> NO CASCADE ---
 324  CONTINUE
      GOTO 40

C --- SIGMA - BAR CASCADE ---
 325  CONTINUE
#ifdef __CXDEBUG__
      IF (NPRT(9)) WRITE(MCXDBUG,2022)
 2022 FORMAT(' *CXGHEI* SUBROUT. CASASM WILL BE CALLED')
#endif
      CALL CASASM( J,INTE,NFL )
      GOTO 40

C --- XI 0 CASCADE ---
 326  CONTINUE
#ifdef __CXDEBUG__
      IF (NPRT(9)) PRINT 2023
 2023 FORMAT(' *CXGHEI* SUBROUT. CASX0 WILL BE CALLED')
#endif
      CALL CASX0( J,INTE,NFL )
      GOTO 40

C --- XI - CASCADE ---
 327  CONTINUE
#ifdef __CXDEBUG__
      IF (NPRT(9)) PRINT 2024
 2024 FORMAT(' *CXGHEI* SUBROUT. CASXM WILL BE CALLED')
#endif
      CALL CASXM( J,INTE,NFL )
      GOTO 40

C --- XI 0 BAR CASCADE ---
 328  CONTINUE
#ifdef __CXDEBUG__
      IF (NPRT(9)) PRINT 2025
 2025 FORMAT(' *CXGHEI* SUBROUT. CASAX0 WILL BE CALLED')
#endif
      CALL CASAX0( J,INTE,NFL )
      GOTO 40

C --- XI - BAR CASCADE ---
 329  CONTINUE
#ifdef __CXDEBUG__
      IF (NPRT(9)) PRINT 2026
 2026 FORMAT(' *CXGHEI* SUBROUT. CASAXM WILL BE CALLED')
#endif
      CALL CASAXM( J,INTE,NFL )
      GOTO 40

C --- OMEGA - CASCADE ---
 330  CONTINUE
#ifdef __CXDEBUG__
      IF (NPRT(9)) PRINT 2027
 2027 FORMAT(' *CXGHEI* SUBROUT. CASOM WILL BE CALLED')
#endif
      CALL CASOM( J,INTE,NFL )
      GOTO 40

C --- OMEGA - BAR CASCADE ---
 331  CONTINUE
#ifdef __CXDEBUG__
      IF (NPRT(9)) PRINT 2028
 2028 FORMAT(' *CXGHEI* SUBROUT. CASAOM WILL BE CALLED')
#endif
      CALL CASAOM( J,INTE,NFL )
      GOTO 40

C --- HEAVY FRAGMENT CASCADE ---
 390  CONTINUE
#ifdef __CXDEBUG__
      IF (NPRT(9)) WRITE(MCXDBUG,2090)
 2090 FORMAT(' *CXGHEI* SUBROUT. CASFRG WILL BE CALLED')
#endif
      NUCFLG = 0
      CALL CASFRG( NUCFLG,INTE,NFL )
      IF (NUCFLG .NE. 0) GOTO 50

C *** CHECK WHETHER THERE ARE NEW PARTICLES GENERATED ***
 40   CONTINUE
      IF ((NTOT .NE. 0) .OR. (KKPART .NE. KPART)) GOTO 50

C
C --- NO SECONDARIES GENERATED AND PARTICLE IS STILL THE SAME ---
C --- ==> COPY PROJECTILE BACK IN CONEX ---
C --- In case of crazy momentum value ==> no change to CONEX stack ---
      IF (P .LT. 0.) GO TO 41
      call ghe2cx(KPART,pnex)
      
  41   CONTINUE

#ifdef __CXDEBUG__
       IF (NPRT(9)) WRITE(MCXDBUG,1003)NTOT,KPART,KKPART
 1003 FORMAT(' *GHEISH* NO SEC. GEN. NTOT,KPART,KKPART = ',
     $ 3(I3,1X)/
     $ ' CURRENT PARTICLE ON THE STACK AGAIN')
#endif
      GO TO 9999
C
C *** CURRENT PARTICLE IS NOT THE SAME AS IN THE BEGINNING OR/AND ***
C *** ONE OR MORE SECONDARIES HAVE BEEN GENERATED ***
 50   CONTINUE

#ifdef __CXDEBUG__
      IF (NPRT(9)) WRITE(MCXDBUG,1004)NTOT,KPART,KKPART
 1004 FORMAT(' *CXGHEI* SEC. GEN. NTOT,KPART,KKPART = ',
     $ 3(I3,1X))
#endif

C --- INITIAL PARTICLE TYPE HAS BEEN CHANGED ==> PUT NEW TYPE ON ---
C --- THE TEMPORARY STACK ---

C --- MAKE CHOICE BETWEEN K0 LONG / K0 SHORT ---
      IF ((KPART .NE. 11) .AND. (KPART .NE. 12)) GOTO 52
      CALL GRNDM( RNDM,1 )
      KPART = int(11.5D0+RNDM(1))

 52   CONTINUE

C --- IN CASE THE NEW PARTICLE IS A NEUTRINO ==> FORGET IT ---
      IF (KPART .EQ. 2) GOTO 60
      pnex(1)=PX*P
      pnex(2)=PY*P
      pnex(3)=PZ*P
      pnex(4)=0.d0
      pnex(5)=abs(RMASS(KPART))
      call ghe2cx(KPART,pnex)

C *** CHECK WHETHER SECONDARIES HAVE BEEN GENERATED AND COPY THEM ***
C *** ALSO ON THE GEANT STACK ***
 60   CONTINUE

C --- ALL QUANTITIES ARE TAKEN FROM THE GHEISHA STACK WHERE THE ---
C --- CONVENTION IS THE FOLLOWING ---
C
C EVE(INDEX+ 1)= X
C EVE(INDEX+ 2)= Y
C EVE(INDEX+ 3)= Z
C EVE(INDEX+ 4)= NCAL
C EVE(INDEX+ 5)= NCELL
C EVE(INDEX+ 6)= MASS
C EVE(INDEX+ 7)= CHARGE
C EVE(INDEX+ 8)= TOF
C EVE(INDEX+ 9)= PX
C EVE(INDEX+10)= PY
C EVE(INDEX+11)= PZ
C EVE(INDEX+12)= TYPE

      IF ( NTOT .LE. 0 ) GOTO 9999

C --- ONE OR MORE SECONDARIES HAVE BEEN GENERATED ---
      DO 61  L = 1, NTOT
        INDEX = (L-1)*12
        JND = int(EVE(INDEX+12))

C --- MAKE CHOICE BETWEEN K0 LONG / K0 SHORT ---
        IF ((JND .NE. 11) .AND. (JND .NE. 12)) GOTO 63
        CALL GRNDM( RNDM,1 )
        JND = int(11.5D0+RNDM(1))

C --- FORGET ABOUT NEUTRINOS ---
 63     CONTINUE
        IF ( JND .EQ. 2 ) GOTO 61


        PLX = EVE(INDEX+9)
        PLY = EVE(INDEX+10)
        PLZ = EVE(INDEX+11)
      pnex(1)=PLX
      pnex(2)=PLY
      pnex(3)=PLZ
      pnex(4)=0.d0
      pnex(5)=abs(EVE(INDEX+6)) ! RMASS()
      call ghe2cx(JND,pnex)

   61 CONTINUE


 9999 CONTINUE

      RETURN
      END

*CMZ :          05/02/2003  09.12.43  by  D. HECK IK FZK KARLSRUHE
*-- Author :    The CORSIKA development group   21/04/1994
C=======================================================================

      SUBROUTINE NGHINI

C-----------------------------------------------------------------------
C  N(EXUS) GH(EISHA) INI(TIALIZATION)
C  INITIALIZATION OF RELEVANT GHEISHA VARIABLES.
C  THIS SUBROUTINE IS CALLED FROM IniGheisha.
C
C  ORIGIN  : GHEISHA SUBROUT. "GHEINI", F.CARMINATI
C  REDESIGN FOR CORSIKA : P. GABRIEL IK1  FZK KARLSRUHE
C  REDESIGN FOR CONEX : P. T. PIEROG IK3  FZK KARLSRUHE
C-----------------------------------------------------------------------


      IMPLICIT DOUBLE PRECISION(A-H, O-Z)
      COMMON /CONEXDBINC/MCXDBUG
      INTEGER          MCXDBUG

#if !__CXCORSIKA__ && !__CORSIKA8__

      COMMON/GSECTI/ AIEL(20),AIIN(20),AIFI(20),AICA(20),ALAM,K0FLAG
      DOUBLE PRECISION AIEL,AIIN,AIFI,AICA,ALAM
      INTEGER K0FLAG

C --- GHEISHA COMMONS ---
C --- INITIALIZATION FLAGS FOR VARIOUS GHEISHA ROUTINES ---
      COMMON /KGINIT/  KGINIT(50)

      COMMON /CONSTS/  PI,TWPI,PIBTW,MP,MPI,MMU,MEL,MKCH,MK0,SMP,SMPI,
     $                 SMU,CT,CTKCH,CTK0,
     $                 ML0,MSP,MS0,MSM,MX0,MXM,CTL0,CTSP,CTSM,CTX0,CTXM,
     $                 RMASS(35),RCHARG(35)

      DOUBLE PRECISION MP,MPI,MMU,MEL,MKCH,MK0,
     *                 ML0,MSP,MS0,MSM,MX0,MXM

      PARAMETER (MXGKGH=100)
      PARAMETER (MXEVEN=12*MXGKGH)
      COMMON /GEVENT /  NSIZE,NCUR,NEXT,NTOT,EVE(MXEVEN)

      PARAMETER (MXGKPV=MXGKGH)
      COMMON /VECUTY/  PV(10,MXGKPV)

C --- BOUNDARY LIMITS FOR ARGUMENTS OF INTRINSIC FUNCTIONS ---
C --- XL DENOTES LOWER BOUND WHEREAS XU DENOTES UPPER BOUND ---
      COMMON /LIMITS/  EXPXL,EXPXU


C --- "NEVENT" CHANGED TO "KEVENT" IN COMMON /CURPAR/ DUE TO CLASH ---
C --- WITH VARIABLE "NEVENT" IN GEANT COMMON ---

      PARAMETER (MXGKCU=MXGKGH)
      COMMON /CURPAR/  WEIGHT(10),DDELTN,IFILE,IRUN,NEVT,KEVENT,SHFLAG,
     $                 ITHST,ITTOT,ITLST,IFRND,TOFCUT,CMOM(5),CENG(5),
     $                 RS,S,ENP(10),NP,NM,NN,NR,NO,NZ,IPA(MXGKCU),
     $                 ATNO2,ZNO2
#endif
      COMMON /PRNTFL/  INBCD,NEWBCD,INBIN,NEWBIN,NPEVT,NEVTP,
     *                 LPRT,NPRT(10)
      LOGICAL          LPRT,NPRT

      SAVE

C Conex COMMON
      integer ighenexs
      common /cxighnx/ ighenexs(35)
      common/xscnsta/xspi,xsainfin 
#ifdef __CXDEBUG__
      parameter (mxisx=200)
      character*500 subisx,textisx
      integer isx,nisx,isxsub,isxsave,isxxsave
      common/cxisx/isx,nisx,subisx(mxisx),isxsub(mxisx)
     &      ,isxsave,isxxsave,textisx
#endif
      character*500 fnho,fnck,fnwle,fnwhe,fndkz,fndkl,fndks,fndkm
     &,fnilo,fndke,fndkn,fndkg,fnwgh,fnwgl
      integer ifho,ifck,ifwle,ifwhe,ifdkz,ifdkl,ifdks,ifdkm,ifilo
     &,ifdke,ifdkn,ifdkg,ifwgh,ifwgl
      common /cxfiles/fnho,ifho,fnck,ifck,fnwle,ifwle,fnwhe,ifwhe
     &,fndkz,ifdkz,fndkl,ifdkl,fndks,ifdks,fndkm,ifdkm,fnilo,ifilo
     &,fndke,ifdke,fndkn,ifdkn,fndkg,ifdkg,fnwgh,ifwgh
     &,fnwgl,ifwgl
C-----------------------------------------------------------------------


C --- SET GHEISHA I/O UNITS TO THE SAME AS FOR CONEX --
#if !__CXCORSIKA__ && !__CORSIKA8__

      INBCD=6
      NEWBCD=6
#endif
#ifdef __CXDEBUG__
      IF (isx.ge.8) NEWBCD=ifck
      MCXDBUG=ifck
#else
      MCXDBUG=6
#endif

C --- INITIALIZE ALL GHEISHA PRINT FLAGS AS FALSE ---
C --- ACTIVATION IS DONE BY "DEBUG" STEERING CARD ---
#if !__CXCORSIKA__ && !__CORSIKA8__

      DO  J = 1, 10
        NPRT(J)=.FALSE.
      ENDDO
#endif
      jmax=0
#ifdef __CXDEBUG__
      if(isx.ge.8)jmax=min(isx-1,10)
#endif
      do j=1,jmax
        NPRT(J)=.TRUE.
      enddo
#ifdef __CXDEBUG__
      if(isx.ge.8)NPRT(9)=.TRUE.
#endif
#if !__CXCORSIKA__ && !__CORSIKA8__

      LPRT=.FALSE.
      DO  I = 1, MXGKPV
        DO  J = 1, 10
          PV(J,I)=0.D0
        ENDDO
      ENDDO

C --- INITIALIZE KGINIT ARRAY ---
      DO  J = 1, 50
        KGINIT(J)=0
      ENDDO

C --- INITIALIZE SOME CUT-OFF PARAMETERS WITH CONEX VALUES ---
      TOFCUT=xsainfin !1.0E+20
      NSIZE=MXEVEN
      K0FLAG=0
      CENG(3)=0.D0
      CENG(4)=0.D0

C --- INITIALIZE PI, 2*PI, PI/2 AND PARTICLE PARAMETERS ---
      PI=xspi
      TWPI=2.0D0*PI
      PIBTW=PI/2.0D0
C *** GAMMA ***
      call cxidmass(ighenexs(1),am)
      RMASS(1)=am
      RCHARG(1)=0.0D0
C *** NEUTRINO ***
      call cxidmass(ighenexs(2),am)
      RMASS(2)=am
      RCHARG(2)=0.0D0
C *** E+ ***
      call cxidmass(ighenexs(3),am)
      RMASS(3)=am
      RCHARG(3)=1.0D0
C *** E- ***
      call cxidmass(ighenexs(4),am)
      RMASS(4)=am
      RCHARG(4)=-1.0D0
C *** MU+ ***
      call cxidmass(ighenexs(5),am)
      RMASS(5)=am
      RCHARG(5)=1.0D0
C *** MU- ***
      call cxidmass(ighenexs(6),am)
      RMASS(6)=am
      RCHARG(6)=-1.0D0
C *** PI+ ***
      call cxidmass(ighenexs(7),am)
      RMASS(7)=am
      RCHARG(7)=1.0D0
      CT=780.4
C *** PI0 ***
      call cxidmass(ighenexs(8),am)
      RMASS(8)=am
      RCHARG(8)=0.0D0
C *** PI- ***
      call cxidmass(ighenexs(9),am)
      RMASS(9)=am
      RCHARG(9)=-1.0D0
C *** K+ ***
      call cxidmass(ighenexs(10),am)
      RMASS(10)=am
      RCHARG(10)=1.0D0
      CTKCH=370.9
C *** K0 SHORT (==> K0) ***
      call cxidmass(ighenexs(11),am)
      RMASS(11)=am
      RCHARG(11)=0.0D0
      CTK0=2.675
C *** K0 LONG (==> K0 BAR) ***
      call cxidmass(ighenexs(12),am)
      RMASS(12)=-am
      RCHARG(12)=0.0D0
C *** K- ***
      call cxidmass(ighenexs(13),am)
      RMASS(13)=am
      RCHARG(13)=-1.0D0
C *** P ***
      call cxidmass(ighenexs(14),am)
      RMASS(14)=am
      RCHARG(14)=1.0D0
C *** P BAR ***
      call cxidmass(ighenexs(15),am)
      RMASS(15)=-am
      RCHARG(15)=-1.0D0
C *** N ***
      call cxidmass(ighenexs(16),am)
      RMASS(16)=am
      RCHARG(16)=0.0D0
C *** N BAR ***
      call cxidmass(ighenexs(17),am)
      RMASS(17)=-am
      RCHARG(17)=0.0D0
C *** L0 ***
      call cxidmass(ighenexs(18),am)
      RMASS(18)=am
      RCHARG(18)=0.0D0
      CTL0=7.89
C *** L0 BAR ***
      call cxidmass(ighenexs(19),am)
      RMASS(19)=-am
      RCHARG(19)=0.0D0
C *** S+ ***
      call cxidmass(ighenexs(20),am)
      RMASS(20)=am
      RCHARG(20)=1.0D0
      CTSP=2.40
C *** S0 ***
      call cxidmass(ighenexs(21),am)
      RMASS(21)=am
      RCHARG(21)=0.0D0
C *** S- ***
      call cxidmass(ighenexs(22),am)
      RMASS(22)=am
      RCHARG(22)=-1.0D0
      CTSM=4.44
C *** S+ BAR ***
      call cxidmass(ighenexs(23),am)
      RMASS(23)=-am
      RCHARG(23)=-1.0D0
C *** S0 BAR ***
      call cxidmass(ighenexs(24),am)
      RMASS(24)=-am
      RCHARG(24)=0.0D0
C *** S- BAR ***
      call cxidmass(ighenexs(25),am)
      RMASS(25)=-am
      RCHARG(25)=1.0D0
C *** XI0 ***
      call cxidmass(ighenexs(26),am)
      RMASS(26)=am
      RCHARG(26)=0.0D0
      CTX0=8.69
C *** XI- ***
      call cxidmass(ighenexs(27),am)
      RMASS(27)=am
      RCHARG(27)=-1.0D0
      CTXM=4.92
C *** XI0 BAR ***
      call cxidmass(ighenexs(28),am)
      RMASS(28)=-am
      RCHARG(28)=0.0D0
      CTX0=8.69
C *** XI- BAR ***
      call cxidmass(ighenexs(29),am)
      RMASS(29)=-am
      RCHARG(29)=1.0D0
C *** DEUTERON ***
      call cxidmass(ighenexs(30),am)
      RMASS(30)=am
      RCHARG(30)=1.0D0
C *** TRITON ***
      call cxidmass(ighenexs(31),am)
      RMASS(31)=am
      RCHARG(31)=1.0D0
C *** ALPHA ***
      call cxidmass(ighenexs(32),am)
      RMASS(32)=am
      RCHARG(32)=2.0D0
C *** OMEGA- ***
      call cxidmass(ighenexs(33),am)
      RMASS(33)=am
      RCHARG(33)=-1.0D0
C *** OMEGA- BAR ***
      call cxidmass(ighenexs(34),am)
      RMASS(34)=-am
      RCHARG(34)=1.0D0
C *** NEW PARTICLE (GEANTINO) ***
      RMASS(35)=0.0D0
      RCHARG(35)=0.0D0

#ifdef __CXDEBUG__
      IF (NPRT(4))
     $ WRITE(MCXDBUG,1000) (I,RMASS(I),RCHARG(I),I=1,33),
     $            CT,CTKCH,CTK0,CTL0,CTSP,CTSM,CTX0,CTXM
 1000 FORMAT(' *CGHINI* === GHEISHA PARTICLE PROPERTIES ==='/
     $ '0INDEX',5X,'MASS (GEV)',5X,'CHARGE'/1H /
     $ 33(1H ,1X,I3,5X,F11.6,6X,F5.2/),
     $ '0PI +-  CT = ',G12.5,' K  +-  CT = ',G12.5/
     $ ' K0     CT = ',G12.5,' L0     CT = ',G12.5/
     $ ' S+     CT = ',G12.5,' S-     CT = ',G12.5/
     $ ' X0     CT = ',G12.5,' X-     CT = ',G12.5)
#endif

      MP=RMASS(14)
      MPI=RMASS(7)
      MMU=RMASS(5)
      MEL=RMASS(3)
      MKCH=RMASS(10)
      MK0=RMASS(11)
      SMP=MP**2
      SMPI=MPI**2
      SMU=MMU**2
      ML0=RMASS(18)
      MSP=RMASS(20)
      MS0=RMASS(21)
      MSM=RMASS(22)
      MX0=RMASS(26)
      MXM=RMASS(27)

C --- LOAD LIMITS FOR INTRINSIC FUNCTION ARGUMENTS ---
      EXPXL = -82.0D0
      EXPXU =  82.0D0

#ifdef __CXDEBUG__
      IF (NPRT(10)) WRITE(MCXDBUG,1001) EXPXL,EXPXU
 1001 FORMAT(' *GHEINI* === INTRINSIC FUNCTION BOUNDARIES ==='/
     $ ' EXPXL,EXPXU = ',2(G12.5,1X))
#endif
#endif

      RETURN
      END

*CMZ :          25/04/2003 by  T. Pierog IK FZK KARLSRUHE
*-- Author :    The CONEX development group   25/04/2003
C=======================================================================

      DOUBLE PRECISION FUNCTION CXGHESIG( PPART,EKIN,KPART,ITARG,IATARG
     *                                 ,IZTARG )

C-----------------------------------------------------------------------
C  GH(EISHA) SIG(MA)
C
C  CALCULATION OF THE PROBABILITIES FOR (IN)ELASTIC INTERACTIONS ***
C  THIS FUNCTION IS CALLED FROM BOX2
C  ARGUMENTS:
C   PPART  = R*8 PARTICLE MOMENTUM (GEV)
C   EKIN   = R*8 KINETIC ENERGY (GEV)
C   KPART  = INCIDENT PARTICLE TYPE
C   IATAR  = TARGET PARTICLE MASS (A)
C   IZTAR  = TARGET PARTICLE CHARGE (Z)
C
C  ORIGIN  : F.CARMINATI, H.FESEFELDT (SUBROUT. GHESIG)
C  REDESIGN FOR CORSIKA : P. GABRIEL IK1  FZK KARLSRUHE
C  REDESIGN FOR CONEX : T. PIEROG IK3  FZK KARLSRUHE
C-----------------------------------------------------------------------
C  *** KPART DENOTES THE GHEISHA PARTICLE INDEX ***
C
C  CONVENTION :
C
C   PARTICLE                 IPART
C   ------------------------------
C   GAMMA                    1
C   NEUTRINO                 2
C   POSITRON                 3
C   ELECTRON                 4
C   MUON +                   5
C   MUON -                   6
C   PION +                   7
C   PION 0                   8
C   PION -                   9
C   KAON +                  10
C   KAON 0 S  (= K(0))      11
C   KAON 0 L  (= K(0) BAR)  12
C   KAON -                  13
C   PROTON                  14
C   PROTON BAR              15
C   NEUTRON                 16
C   NEUTRON BAR             17
C   LAMBDA                  18
C   LAMBDA BAR              19
C   SIGMA +                 20
C   SIGMA 0                 21
C   SIGMA -                 22
C   SIGMA + BAR             23
C   SIGMA 0 BAR             24
C   SIGMA - BAR             25
C   XSI 0                   26
C   XSI -                   27
C   XSI 0 BAR               28
C   XSI - BAR               29
C   DEUTERON                30
C   TRITON                  31
C   ALPHA                   32
C   OMEGA -                 33
C   OMEGA - BAR             34
C   NEW PARTICLES           35
C
C-----------------------------------------------------------------------

      IMPLICIT DOUBLE PRECISION(A-H, O-Z)
      common/cxair/airz(3),aira(3),airw(3),airavz,airava,airi(3)
      PARAMETER (KKK=3)
#if !__CXCORSIKA__ && !__CORSIKA8__

      COMMON /GHECGCOMP/ ACOMP,ZCOMP,WCOMP,KK
#else
      COMMON /CGCOMP/ ACOMP,ZCOMP,WCOMP,KK
#endif
      DOUBLE PRECISION  ACOMP(KKK),ZCOMP(KKK),WCOMP(KKK)
      COMMON /CONEXDBINC/MCXDBUG
      INTEGER          MCXDBUG

      COMMON/GSECTI/ AIEL(20),AIIN(20),AIFI(20),AICA(20),ALAM,K0FLAG
      DOUBLE PRECISION AIEL,AIIN,AIFI,AICA,ALAM
      INTEGER K0FLAG

C --- GHEISHA COMMONS ---
      COMMON /RESULT/  XEND,YEND,ZEND,RCA,RCE,AMAS,NCH,TOF,PX,PY,PZ,
     *                 USERW,INTCT,P,EN,EK,AMASQ,DELTN,ITK,NTK,IPART,
     *                 IND,LCALO,ICEL,SINL,COSL,SINP,COSP,
     *                 XOLD,YOLD,ZOLD,POLD,PXOLD,PYOLD,PZOLD,
     *                 XSCAT,YSCAT,ZSCAT,PSCAT,PXSCAT,PYSCAT,PZSCAT
      DOUBLE PRECISION NCH,INTCT

      COMMON /PRNTFL/  INBCD,NEWBCD,INBIN,NEWBIN,NPEVT,NEVTP,
     *                 LPRT,NPRT(10)
      LOGICAL          LPRT,NPRT


      DIMENSION        ALPHA(35),ALPHAC(41),IPART2(7),CSA(4)
      DIMENSION        PARTEL(35),PARTIN(35),INTRC(35)
*     DIMENSION        ICORR(35)

C --- DIMENSION STATEMENTS FOR CROSS-SECTION DATA ---
      DIMENSION        PLAB(41),CSEL(35,41),CSIN(35,41),CSPIEL(3,41),
     $                 CSPIIN(3,41),CSPNEL(3,41),CSPNIN(3,41),
     $                 ELAB(17),CNLWAT(15),CNLWEL(15,17),CNLWIN(15,17),
     $                 CSCAP(100)

C --- DIMENSION STMTS. FOR GEANT/GHEISHA PARTICLE CODE CONVERSIONS ---
C --- KIPART(I)=GHEISHA CODE CORRESPONDING TO GEANT   CODE I ---
C --- IKPART(I)=GEANT   CODE CORRESPONDING TO GHEISHA CODE I ---


      SAVE

C --- CROSS-SECTION DATA BY "PCSDAT" 01-FEB-1989 ---
      DATA PLAB /
     $  0.00000D0  , 0.10000D0  , 0.15000D0  , 0.20000D0  , 0.25000D0  ,
     $  0.30000D0  , 0.35000D0  , 0.40000D0  , 0.45000D0  , 0.50000D0  ,
     $  0.55000D0  , 0.60000D0  , 0.65000D0  , 0.70000D0  , 0.75000D0  ,
     $  0.80000D0  , 0.85000D0  , 0.90000D0  , 0.95000D0  ,  1.0000D0  ,
     $   1.1000D0  ,  1.2000D0  ,  1.3000D0  ,  1.4000D0  ,  1.5000D0  ,
     $   1.6000D0  ,  1.8000D0  ,  2.0000D0  ,  2.2000D0  ,  2.4000D0  ,
     $   2.6000D0  ,  2.8000D0  ,  3.0000D0  ,  4.0000D0  ,  5.0000D0  ,
     $   6.0000D0  ,  8.0000D0  ,  10.000D0  ,  20.000D0  ,  100.00D0  ,
     $   1000.0D0  /

C  ELASTIC SCATTERING CROSS-SECTIONS ON FREE PROTONS
C  GAMMA, NEUTRINO, POSITRON, ELECTRON, MU(+), MU(-)
      DATA ((CSEL(I,J),I=1,6),J=1,41) / 246 * 0.D0 /
C  PI(0)
      DATA (CSEL( 8,J),J=1,41) / 41 * 0.D0 /
C  SIGMA(0)
      DATA (CSEL(21,J),J=1,41) / 41 * 0.D0 /
C  SIGMA(0)_BAR
      DATA (CSEL(24,J),J=1,41) / 41 * 0.D0 /
C  DEUTERIUM, TRITIUM, ALPHA
      DATA ((CSEL(I,J),I=30,32),J=1,41) / 123 * 0.D0 /
C  NEW PARTICLES
      DATA (CSEL(35,J),J=1,41) / 41 * 0.D0 /
C  PI(+)
      DATA (CSEL( 7,J),J=1,41) /
     $  0.00000D0  ,  6.0000D0  ,  20.000D0  ,  71.000D0  ,  155.00D0  ,
     $   195.00D0  ,  130.00D0  ,  78.000D0  ,  60.000D0  ,  32.000D0  ,
     $   23.500D0  ,  18.500D0  ,  15.000D0  ,  12.500D0  ,  10.000D0  ,
     $   9.1000D0  ,  8.6000D0  ,  8.8000D0  ,  9.5000D0  ,  10.600D0  ,
     $   13.000D0  ,  15.500D0  ,  17.100D0  ,  17.200D0  ,  16.200D0  ,
     $   15.000D0  ,  12.300D0  ,  10.200D0  ,  9.0000D0  ,  8.0000D0  ,
     $   7.3000D0  ,  6.8000D0  ,  6.5000D0  ,  5.8000D0  ,  5.4000D0  ,
     $   5.2000D0  ,  5.0000D0  ,  4.9000D0  ,  3.8000D0  ,  3.2000D0  ,
     $   3.5000D0  /
C  PI(-)
      DATA (CSEL( 9,J),J=1,41) /
     $  0.00000D0  ,  1.0000D0  ,  3.0000D0  ,  8.0000D0  ,  18.000D0  ,
     $   25.000D0  ,  27.500D0  ,  12.300D0  ,  10.600D0  ,  11.000D0  ,
     $   12.500D0  ,  14.500D0  ,  17.000D0  ,  19.400D0  ,  19.800D0  ,
     $   16.800D0  ,  14.000D0  ,  14.800D0  ,  20.000D0  ,  26.100D0  ,
     $   19.500D0  ,  15.000D0  ,  12.800D0  ,  11.500D0  ,  10.500D0  ,
     $   9.8000D0  ,  8.8000D0  ,  8.2000D0  ,  7.8000D0  ,  7.5000D0  ,
     $   7.2000D0  ,  7.0000D0  ,  6.8000D0  ,  6.1000D0  ,  5.7000D0  ,
     $   5.4000D0  ,  4.9000D0  ,  4.6000D0  ,  4.0000D0  ,  3.3000D0  ,
     $   3.5000D0  /
C  K(+)
      DATA (CSEL(10,J),J=1,41) /
     $   10.000D0  ,  11.200D0  ,  11.300D0  ,  11.400D0  ,  11.500D0  ,
     $   11.600D0  ,  11.800D0  ,  12.000D0  ,  12.100D0  ,  12.200D0  ,
     $   12.300D0  ,  12.400D0  ,  12.500D0  ,  12.500D0  ,  12.500D0  ,
     $   12.400D0  ,  12.300D0  ,  12.200D0  ,  12.000D0  ,  11.800D0  ,
     $   11.200D0  ,  11.500D0  ,  9.9000D0  ,  9.4000D0  ,  8.8000D0  ,
     $   8.4000D0  ,  7.5000D0  ,  6.9000D0  ,  6.3000D0  ,  5.9000D0  ,
     $   5.5000D0  ,  5.2000D0  ,  5.0000D0  ,  4.0000D0  ,  3.5000D0  ,
     $   3.3000D0  ,  3.1000D0  ,  3.1000D0  ,  3.0000D0  ,  2.5000D0  ,
     $   3.0000D0  /
C  K(0) SHORT  (= K(0))
      DATA (CSEL(11,J),J=1,41) /
     $   10.000D0  ,  11.200D0  ,  11.300D0  ,  11.400D0  ,  11.500D0  ,
     $   11.600D0  ,  11.800D0  ,  12.000D0  ,  12.100D0  ,  12.200D0  ,
     $   12.300D0  ,  12.400D0  ,  12.500D0  ,  12.500D0  ,  12.500D0  ,
     $   12.400D0  ,  12.300D0  ,  12.200D0  ,  12.000D0  ,  11.800D0  ,
     $   11.200D0  ,  11.500D0  ,  9.9000D0  ,  9.4000D0  ,  8.8000D0  ,
     $   8.4000D0  ,  7.5000D0  ,  6.9000D0  ,  6.3000D0  ,  5.9000D0  ,
     $   5.5000D0  ,  5.2000D0  ,  5.0000D0  ,  4.0000D0  ,  3.5000D0  ,
     $   3.3000D0  ,  3.1000D0  ,  3.1000D0  ,  3.0000D0  ,  2.5000D0  ,
     $   3.0000D0  /
C  K(0) LONG (= K(0)_BAR)
      DATA (CSEL(12,J),J=1,41) /
     $   160.83D0  ,  82.800D0  ,  58.575D0  ,  43.683D0  ,  34.792D0  ,
     $   28.650D0  ,  24.367D0  ,  20.917D0  ,  18.192D0  ,  16.300D0  ,
     $   14.608D0  ,  13.017D0  ,  12.250D0  ,  11.700D0  ,  12.017D0  ,
     $   14.075D0  ,  15.842D0  ,  16.433D0  ,  16.042D0  ,  15.008D0  ,
     $   12.575D0  ,  10.708D0  ,  9.2000D0  ,  8.0167D0  ,  7.2833D0  ,
     $   7.0750D0  ,  6.6333D0  ,  6.1250D0  ,  5.6583D0  ,  5.2750D0  ,
     $   4.9333D0  ,  4.6250D0  ,  4.4583D0  ,  3.7333D0  ,  3.3833D0  ,
     $   3.1833D0  ,  2.9833D0  ,  2.7500D0  ,  2.3667D0  ,  2.2000D0  ,
     $   2.6000D0  /
C  K(-)
      DATA (CSEL(13,J),J=1,41) /
     $   300.00D0  ,  140.00D0  ,  97.000D0  ,  70.000D0  ,  55.000D0  ,
     $   45.000D0  ,  37.000D0  ,  31.000D0  ,  26.000D0  ,  23.000D0  ,
     $   20.000D0  ,  17.000D0  ,  15.500D0  ,  14.500D0  ,  14.700D0  ,
     $   18.500D0  ,  22.000D0  ,  23.000D0  ,  22.500D0  ,  20.700D0  ,
     $   16.500D0  ,  14.000D0  ,  11.500D0  ,  9.6000D0  ,  8.6000D0  ,
     $   8.5000D0  ,  8.3000D0  ,  7.6000D0  ,  7.0000D0  ,  6.4000D0  ,
     $   5.9000D0  ,  5.5000D0  ,  5.3000D0  ,  4.4000D0  ,  4.1000D0  ,
     $   3.9000D0  ,  3.7000D0  ,  3.3000D0  ,  2.6000D0  ,  2.5000D0  ,
     $   3.0000D0  /
C  PROTON
      DATA (CSEL(14,J),J=1,41) /
     $   1100.0D0  ,  115.00D0  ,  105.00D0  ,  100.00D0  ,  56.000D0  ,
     $   40.000D0  ,  27.000D0  ,  22.000D0  ,  21.000D0  ,  20.000D0  ,
     $   20.000D0  ,  20.000D0  ,  20.500D0  ,  21.000D0  ,  22.000D0  ,
     $   23.000D0  ,  24.000D0  ,  24.000D0  ,  24.400D0  ,  24.500D0  ,
     $   25.000D0  ,  25.500D0  ,  26.000D0  ,  26.500D0  ,  27.000D0  ,
     $   27.000D0  ,  26.000D0  ,  23.000D0  ,  21.500D0  ,  20.000D0  ,
     $   19.000D0  ,  18.000D0  ,  17.000D0  ,  13.000D0  ,  11.500D0  ,
     $   10.300D0  ,  9.4000D0  ,  9.0000D0  ,  8.8000D0  ,  7.0000D0  ,
     $   7.5000D0  /
C  PROTON_BAR
      DATA (CSEL(15,J),J=1,41) /
     $   200.00D0  ,  163.00D0  ,  141.00D0  ,  120.00D0  ,  111.00D0  ,
     $   99.500D0  ,  92.500D0  ,  86.500D0  ,  82.000D0  ,  78.000D0  ,
     $   74.000D0  ,  71.000D0  ,  67.500D0  ,  65.000D0  ,  62.500D0  ,
     $   59.700D0  ,  58.100D0  ,  56.300D0  ,  54.700D0  ,  52.700D0  ,
     $   50.000D0  ,  48.400D0  ,  47.000D0  ,  46.000D0  ,  45.200D0  ,
     $   42.800D0  ,  39.200D0  ,  36.300D0  ,  32.800D0  ,  30.400D0  ,
     $   28.100D0  ,  26.300D0  ,  24.500D0  ,  19.250D0  ,  16.840D0  ,
     $   14.600D0  ,  12.340D0  ,  11.210D0  ,  8.8500D0  ,  7.5000D0  ,
     $   7.5000D0  /
C  NEUTRON
      DATA (CSEL(16,J),J=1,41) /
     $   4200.0D0  ,  440.00D0  ,  420.00D0  ,  400.00D0  ,  230.00D0  ,
     $   160.00D0  ,  105.00D0  ,  80.000D0  ,  62.000D0  ,  50.000D0  ,
     $   45.000D0  ,  41.000D0  ,  38.000D0  ,  36.000D0  ,  35.000D0  ,
     $   34.000D0  ,  33.000D0  ,  32.000D0  ,  31.500D0  ,  31.000D0  ,
     $   30.500D0  ,  30.000D0  ,  29.500D0  ,  29.000D0  ,  28.500D0  ,
     $   28.000D0  ,  26.000D0  ,  23.000D0  ,  21.500D0  ,  20.000D0  ,
     $   19.000D0  ,  18.000D0  ,  17.000D0  ,  13.000D0  ,  11.500D0  ,
     $   10.300D0  ,  9.4000D0  ,  9.0000D0  ,  8.8000D0  ,  7.0000D0  ,
     $   7.5000D0  /
C  NEUTRON_BAR
      DATA (CSEL(17,J),J=1,41) /
     $   185.88D0  ,  133.23D0  ,  119.37D0  ,  102.86D0  ,  93.102D0  ,
     $   82.752D0  ,  76.205D0  ,  71.008D0  ,  67.366D0  ,  64.096D0  ,
     $   60.891D0  ,  58.501D0  ,  55.735D0  ,  53.773D0  ,  51.839D0  ,
     $   49.671D0  ,  48.485D0  ,  47.045D0  ,  45.803D0  ,  44.306D0  ,
     $   42.623D0  ,  41.786D0  ,  41.115D0  ,  40.630D0  ,  40.129D0  ,
     $   38.242D0  ,  35.233D0  ,  32.662D0  ,  29.639D0  ,  27.573D0  ,
     $   25.536D0  ,  23.948D0  ,  22.356D0  ,  17.723D0  ,  15.614D0  ,
     $   13.653D0  ,  11.675D0  ,  10.653D0  ,  8.6198D0  ,  7.4464D0  ,
     $   7.4821D0  /
C  LAMBDA
      DATA (CSEL(18,J),J=1,41) /
     $   1100.0D0  ,  115.00D0  ,  105.00D0  ,  100.00D0  ,  56.000D0  ,
     $   40.000D0  ,  27.000D0  ,  22.000D0  ,  21.000D0  ,  20.000D0  ,
     $   20.000D0  ,  19.067D0  ,  19.333D0  ,  19.500D0  ,  19.833D0  ,
     $   20.567D0  ,  21.800D0  ,  22.900D0  ,  23.869D0  ,  23.809D0  ,
     $   22.161D0  ,  21.488D0  ,  19.732D0  ,  19.433D0  ,  19.345D0  ,
     $   19.029D0  ,  18.121D0  ,  16.280D0  ,  15.258D0  ,  14.280D0  ,
     $   13.644D0  ,  12.963D0  ,  12.316D0  ,  9.5333D0  ,  8.4333D0  ,
     $   7.5728D0  ,  6.9696D0  ,  6.7518D0  ,  6.6175D0  ,  5.6000D0  ,
     $   6.1145D0  /
C  LAMBDA_BAR
      DATA (CSEL(19,J),J=1,41) /
     $   157.65D0  ,  73.701D0  ,  76.096D0  ,  68.571D0  ,  57.305D0  ,
     $   49.257D0  ,  43.616D0  ,  40.024D0  ,  38.098D0  ,  36.287D0  ,
     $   34.674D0  ,  33.105D0  ,  31.712D0  ,  30.685D0  ,  29.613D0  ,
     $   28.602D0  ,  28.336D0  ,  28.075D0  ,  27.786D0  ,  27.215D0  ,
     $   26.380D0  ,  26.146D0  ,  25.108D0  ,  24.783D0  ,  24.360D0  ,
     $   23.219D0  ,  21.431D0  ,  20.095D0  ,  18.382D0  ,  17.267D0  ,
     $   16.100D0  ,  15.175D0  ,  14.271D0  ,  11.573D0  ,  10.305D0  ,
     $   9.1471D0  ,  8.0149D0  ,  7.4349D0  ,  6.2499D0  ,  5.8928D0  ,
     $   6.0774D0  /
C  SIGMA(+)
      DATA (CSEL(20,J),J=1,41) /
     $   1100.0D0  ,  115.00D0  ,  105.00D0  ,  100.00D0  ,  56.000D0  ,
     $   40.000D0  ,  27.000D0  ,  22.000D0  ,  21.000D0  ,  20.000D0  ,
     $   20.000D0  ,  19.067D0  ,  19.333D0  ,  19.500D0  ,  19.833D0  ,
     $   20.567D0  ,  21.800D0  ,  22.900D0  ,  23.869D0  ,  23.809D0  ,
     $   22.161D0  ,  21.488D0  ,  19.732D0  ,  19.433D0  ,  19.345D0  ,
     $   19.029D0  ,  18.121D0  ,  16.280D0  ,  15.258D0  ,  14.280D0  ,
     $   13.644D0  ,  12.963D0  ,  12.316D0  ,  9.5333D0  ,  8.4333D0  ,
     $   7.5728D0  ,  6.9696D0  ,  6.7518D0  ,  6.6175D0  ,  5.6000D0  ,
     $   6.1145D0  /
C  SIGMA(-)
      DATA (CSEL(22,J),J=1,41) /
     $   1100.0D0  ,  115.00D0  ,  105.00D0  ,  100.00D0  ,  56.000D0  ,
     $   40.000D0  ,  27.000D0  ,  22.000D0  ,  21.000D0  ,  20.000D0  ,
     $   20.000D0  ,  19.067D0  ,  19.333D0  ,  19.500D0  ,  19.833D0  ,
     $   20.567D0  ,  21.800D0  ,  22.900D0  ,  23.869D0  ,  23.809D0  ,
     $   22.161D0  ,  21.488D0  ,  19.732D0  ,  19.433D0  ,  19.345D0  ,
     $   19.029D0  ,  18.121D0  ,  16.280D0  ,  15.258D0  ,  14.280D0  ,
     $   13.644D0  ,  12.963D0  ,  12.316D0  ,  9.5333D0  ,  8.4333D0  ,
     $   7.5728D0  ,  6.9696D0  ,  6.7518D0  ,  6.6175D0  ,  5.6000D0  ,
     $   6.1145D0  /
C  SIGMA(+)_BAR
      DATA (CSEL(23,J),J=1,41) /
     $   185.88D0  ,  133.23D0  ,  119.37D0  ,  102.86D0  ,  93.102D0  ,
     $   82.752D0  ,  76.205D0  ,  71.008D0  ,  67.366D0  ,  64.096D0  ,
     $   60.891D0  ,  58.104D0  ,  55.241D0  ,  53.140D0  ,  50.934D0  ,
     $   48.660D0  ,  47.566D0  ,  46.585D0  ,  45.581D0  ,  44.003D0  ,
     $   41.134D0  ,  39.374D0  ,  36.878D0  ,  35.523D0  ,  34.503D0  ,
     $   32.334D0  ,  29.365D0  ,  27.370D0  ,  24.705D0  ,  22.921D0  ,
     $   21.229D0  ,  19.879D0  ,  18.559D0  ,  14.625D0  ,  12.758D0  ,
     $   11.041D0  ,  9.3440D0  ,  8.5484D0  ,  6.7104D0  ,  6.0000D0  ,
     $   6.1131D0  /
C  SIGMA(-)_BAR
      DATA (CSEL(25,J),J=1,41) /
     $   157.65D0  ,  73.701D0  ,  76.096D0  ,  68.571D0  ,  57.305D0  ,
     $   49.257D0  ,  43.616D0  ,  40.024D0  ,  38.098D0  ,  36.287D0  ,
     $   34.674D0  ,  33.105D0  ,  31.712D0  ,  30.685D0  ,  29.613D0  ,
     $   28.602D0  ,  28.336D0  ,  28.075D0  ,  27.786D0  ,  27.215D0  ,
     $   26.380D0  ,  26.146D0  ,  25.108D0  ,  24.783D0  ,  24.360D0  ,
     $   23.219D0  ,  21.431D0  ,  20.095D0  ,  18.382D0  ,  17.267D0  ,
     $   16.100D0  ,  15.175D0  ,  14.271D0  ,  11.573D0  ,  10.305D0  ,
     $   9.1471D0  ,  8.0149D0  ,  7.4349D0  ,  6.2499D0  ,  5.8928D0  ,
     $   6.0774D0  /
C  XI(0)
      DATA (CSEL(26,J),J=1,41) /
     $   1100.0D0  ,  115.00D0  ,  105.00D0  ,  100.00D0  ,  56.000D0  ,
     $   40.000D0  ,  27.000D0  ,  22.000D0  ,  21.000D0  ,  20.000D0  ,
     $   20.000D0  ,  18.133D0  ,  18.167D0  ,  18.000D0  ,  17.667D0  ,
     $   18.133D0  ,  19.600D0  ,  21.800D0  ,  23.338D0  ,  23.118D0  ,
     $   19.323D0  ,  17.476D0  ,  13.464D0  ,  12.367D0  ,  11.691D0  ,
     $   11.057D0  ,  10.242D0  ,  9.5593D0  ,  9.0151D0  ,  8.5591D0  ,
     $   8.2884D0  ,  7.9253D0  ,  7.6311D0  ,  6.0667D0  ,  5.3667D0  ,
     $   4.8456D0  ,  4.5392D0  ,  4.5036D0  ,  4.4351D0  ,  4.2000D0  ,
     $   4.7289D0  /
C  XI(-)
      DATA (CSEL(27,J),J=1,41) /
     $   1100.0D0  ,  115.00D0  ,  105.00D0  ,  100.00D0  ,  56.000D0  ,
     $   40.000D0  ,  27.000D0  ,  22.000D0  ,  21.000D0  ,  20.000D0  ,
     $   20.000D0  ,  18.133D0  ,  18.167D0  ,  18.000D0  ,  17.667D0  ,
     $   18.133D0  ,  19.600D0  ,  21.800D0  ,  23.338D0  ,  23.118D0  ,
     $   19.323D0  ,  17.476D0  ,  13.464D0  ,  12.367D0  ,  11.691D0  ,
     $   11.057D0  ,  10.242D0  ,  9.5593D0  ,  9.0151D0  ,  8.5591D0  ,
     $   8.2884D0  ,  7.9253D0  ,  7.6311D0  ,  6.0667D0  ,  5.3667D0  ,
     $   4.8456D0  ,  4.5392D0  ,  4.5036D0  ,  4.4351D0  ,  4.2000D0  ,
     $   4.7289D0  /
C  XI(0)_BAR
      DATA (CSEL(28,J),J=1,41) /
     $   157.65D0  ,  73.701D0  ,  76.096D0  ,  68.571D0  ,  57.305D0  ,
     $   49.257D0  ,  43.616D0  ,  40.024D0  ,  38.098D0  ,  36.287D0  ,
     $   34.674D0  ,  32.708D0  ,  31.218D0  ,  30.052D0  ,  28.707D0  ,
     $   27.591D0  ,  27.417D0  ,  27.615D0  ,  27.564D0  ,  26.913D0  ,
     $   24.891D0  ,  23.734D0  ,  20.871D0  ,  19.677D0  ,  18.734D0  ,
     $   17.311D0  ,  15.563D0  ,  14.803D0  ,  13.448D0  ,  12.615D0  ,
     $   11.794D0  ,  11.106D0  ,  10.474D0  ,  8.4745D0  ,  7.4498D0  ,
     $   6.5350D0  ,  5.6835D0  ,  5.3300D0  ,  4.3406D0  ,  4.4464D0  ,
     $   4.7083D0  /
C  XI(-)_BAR
      DATA (CSEL(29,J),J=1,41) /
     $   143.53D0  ,  43.935D0  ,  54.462D0  ,  51.429D0  ,  39.407D0  ,
     $   32.510D0  ,  27.321D0  ,  24.532D0  ,  23.465D0  ,  22.383D0  ,
     $   21.566D0  ,  20.209D0  ,  19.453D0  ,  18.825D0  ,  18.046D0  ,
     $   17.562D0  ,  17.802D0  ,  18.360D0  ,  18.667D0  ,  18.519D0  ,
     $   17.514D0  ,  17.120D0  ,  14.985D0  ,  14.306D0  ,  13.663D0  ,
     $   12.753D0  ,  11.596D0  ,  11.165D0  ,  10.287D0  ,  9.7882D0  ,
     $   9.2294D0  ,  8.7539D0  ,  8.3300D0  ,  6.9480D0  ,  6.2234D0  ,
     $   5.5881D0  ,  5.0189D0  ,  4.7733D0  ,  4.1104D0  ,  4.3929D0  ,
     $   4.6905D0  /
C  OMEGA(-)
      DATA (CSEL(33,J),J=1,41) /
     $   1100.0D0  ,  115.00D0  ,  105.00D0  ,  100.00D0  ,  56.000D0  ,
     $   40.000D0  ,  27.000D0  ,  22.000D0  ,  21.000D0  ,  20.000D0  ,
     $   20.000D0  ,  18.133D0  ,  18.167D0  ,  18.000D0  ,  17.667D0  ,
     $   18.133D0  ,  19.600D0  ,  21.800D0  ,  23.338D0  ,  23.118D0  ,
     $   19.323D0  ,  17.476D0  ,  13.464D0  ,  12.367D0  ,  11.691D0  ,
     $   11.057D0  ,  10.242D0  ,  9.5593D0  ,  9.0151D0  ,  8.5591D0  ,
     $   8.2884D0  ,  7.9253D0  ,  7.6311D0  ,  6.0667D0  ,  5.3667D0  ,
     $   4.8456D0  ,  4.5392D0  ,  4.5036D0  ,  4.4351D0  ,  4.2000D0  ,
     $   4.7289D0  /
C  OMEGA(-)_BAR
      DATA (CSEL(34,J),J=1,41) /
     $   143.53D0  ,  43.935D0  ,  54.462D0  ,  51.429D0  ,  39.407D0  ,
     $   32.510D0  ,  27.321D0  ,  24.532D0  ,  23.465D0  ,  22.383D0  ,
     $   21.566D0  ,  20.209D0  ,  19.453D0  ,  18.825D0  ,  18.046D0  ,
     $   17.562D0  ,  17.802D0  ,  18.360D0  ,  18.667D0  ,  18.519D0  ,
     $   17.514D0  ,  17.120D0  ,  14.985D0  ,  14.306D0  ,  13.663D0  ,
     $   12.753D0  ,  11.596D0  ,  11.165D0  ,  10.287D0  ,  9.7882D0  ,
     $   9.2294D0  ,  8.7539D0  ,  8.3300D0  ,  6.9480D0  ,  6.2234D0  ,
     $   5.5881D0  ,  5.0189D0  ,  4.7733D0  ,  4.1104D0  ,  4.3929D0  ,
     $   4.6905D0  /

C  INELASTIC CROSS-SECTIONS ON FREE PROTONS
C  GAMMA, NEUTRINO, POSITRON, ELECTRON, MU(+), MU(-)
      DATA ((CSIN(I,J),I=1,6),J=1,41) / 246 * 0.D0 /
C  PI(0)
      DATA (CSIN( 8,J),J=1,41) / 41 * 0.D0 /
C  SIGMA(0)
      DATA (CSIN(21,J),J=1,41) / 41 * 0.D0 /
C  SIGMA(0)_BAR
      DATA (CSIN(24,J),J=1,41) / 41 * 0.D0 /
C  DEUTERIUM, TRITIUM, ALPHA
      DATA ((CSIN(I,J),I=30,32),J=1,41) / 123 * 0.D0 /
C  NEW PARTICLES
      DATA (CSIN(35,J),J=1,41) / 41 * 0.D0 /
C  PI(+)
      DATA (CSIN( 7,J),J=1,41) /
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  ,
     $  0.00000D0  , 0.00000D0  , 0.50000D0  ,  1.2000D0  ,  1.7000D0  ,
     $   2.2500D0  ,  3.0000D0  ,  3.6000D0  ,  4.5000D0  ,  5.4000D0  ,
     $   6.3000D0  ,  8.6000D0  ,  9.0000D0  ,  10.000D0  ,  11.500D0  ,
     $   14.000D0  ,  17.000D0  ,  19.500D0  ,  22.000D0  ,  24.000D0  ,
     $   21.500D0  ,  18.500D0  ,  19.000D0  ,  20.500D0  ,  22.200D0  ,
     $   23.000D0  ,  23.300D0  ,  23.000D0  ,  21.000D0  ,  20.500D0  ,
     $   20.200D0  ,  20.100D0  ,  20.000D0  ,  20.000D0  ,  20.000D0  ,
     $   21.000D0  /
C  PI(-)
      DATA (CSIN( 9,J),J=1,41) /
     $  0.00000D0  ,  3.0000D0  ,  9.2000D0  ,  20.500D0  ,  36.500D0  ,
     $   45.000D0  ,  28.000D0  ,  19.500D0  ,  15.500D0  ,  14.200D0  ,
     $   15.500D0  ,  17.500D0  ,  20.000D0  ,  23.000D0  ,  26.000D0  ,
     $   20.000D0  ,  23.000D0  ,  26.500D0  ,  32.000D0  ,  35.000D0  ,
     $   28.500D0  ,  22.000D0  ,  22.500D0  ,  23.500D0  ,  24.000D0  ,
     $   24.500D0  ,  26.000D0  ,  27.500D0  ,  27.500D0  ,  27.000D0  ,
     $   26.500D0  ,  25.500D0  ,  25.000D0  ,  23.000D0  ,  22.500D0  ,
     $   22.200D0  ,  22.000D0  ,  22.000D0  ,  21.200D0  ,  20.700D0  ,
     $   21.000D0  /
C  K(+)
      DATA (CSIN(10,J),J=1,41) /
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  ,
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  ,
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  ,
     $  0.50000D0  ,  1.5000D0  ,  2.7000D0  ,  3.8000D0  ,  4.8000D0  ,
     $   6.5000D0  ,  7.6000D0  ,  8.4000D0  ,  9.0000D0  ,  9.4000D0  ,
     $   9.8000D0  ,  10.500D0  ,  11.000D0  ,  11.500D0  ,  11.800D0  ,
     $   12.200D0  ,  12.400D0  ,  12.600D0  ,  13.200D0  ,  13.500D0  ,
     $   13.700D0  ,  14.000D0  ,  14.200D0  ,  14.500D0  ,  16.400D0  ,
     $   17.000D0  /
C  K(0) SHORT  (= K(0))
      DATA (CSIN(11,J),J=1,41) /
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  ,
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  ,
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  ,
     $  0.50000D0  ,  1.5000D0  ,  2.7000D0  ,  3.8000D0  ,  4.8000D0  ,
     $   6.5000D0  ,  7.6000D0  ,  8.4000D0  ,  9.0000D0  ,  9.4000D0  ,
     $   9.8000D0  ,  10.500D0  ,  11.000D0  ,  11.500D0  ,  11.800D0  ,
     $   12.200D0  ,  12.400D0  ,  12.600D0  ,  13.200D0  ,  13.500D0  ,
     $   13.700D0  ,  14.000D0  ,  14.200D0  ,  14.500D0  ,  16.400D0  ,
     $   17.000D0  /
C  K(0) LONG  (=  K(0)_BAR)
      DATA (CSIN(12,J),J=1,41) /
     $   266.67D0  ,  133.33D0  ,  83.333D0  ,  57.083D0  ,  44.500D0  ,
     $   33.250D0  ,  24.583D0  ,  20.833D0  ,  18.333D0  ,  16.083D0  ,
     $   15.625D0  ,  15.083D0  ,  14.833D0  ,  15.083D0  ,  15.833D0  ,
     $   17.042D0  ,  18.958D0  ,  20.758D0  ,  22.533D0  ,  22.825D0  ,
     $   21.250D0  ,  18.567D0  ,  17.767D0  ,  18.100D0  ,  19.933D0  ,
     $   20.783D0  ,  21.225D0  ,  21.000D0  ,  20.558D0  ,  20.258D0  ,
     $   20.017D0  ,  19.767D0  ,  19.600D0  ,  19.183D0  ,  18.850D0  ,
     $   18.575D0  ,  18.350D0  ,  18.175D0  ,  17.808D0  ,  17.558D0  ,
     $   19.250D0  /
C  K(-)
      DATA (CSIN(13,J),J=1,41) /
     $   400.00D0  ,  200.00D0  ,  120.00D0  ,  81.000D0  ,  62.000D0  ,
     $   47.000D0  ,  35.000D0  ,  28.000D0  ,  24.000D0  ,  21.000D0  ,
     $   19.500D0  ,  19.000D0  ,  18.800D0  ,  19.000D0  ,  20.000D0  ,
     $   21.000D0  ,  23.000D0  ,  25.000D0  ,  27.000D0  ,  27.500D0  ,
     $   25.500D0  ,  22.000D0  ,  20.800D0  ,  21.000D0  ,  23.000D0  ,
     $   24.000D0  ,  24.000D0  ,  23.800D0  ,  23.000D0  ,  22.500D0  ,
     $   22.000D0  ,  21.600D0  ,  21.400D0  ,  21.000D0  ,  20.500D0  ,
     $   20.200D0  ,  19.800D0  ,  19.500D0  ,  18.600D0  ,  17.500D0  ,
     $   20.000D0  /
C  PROTON
      DATA (CSIN(14,J),J=1,41) /
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  ,
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  ,
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  ,
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.10000D0  ,  1.5000D0  ,
     $   7.0000D0  ,  12.000D0  ,  17.000D0  ,  19.500D0  ,  20.500D0  ,
     $   22.000D0  ,  23.500D0  ,  24.800D0  ,  25.800D0  ,  26.500D0  ,
     $   27.000D0  ,  27.500D0  ,  28.000D0  ,  30.000D0  ,  31.000D0  ,
     $   32.000D0  ,  32.500D0  ,  32.500D0  ,  33.000D0  ,  33.500D0  ,
     $   34.000D0  /
C  PROTON_BAR
      DATA (CSIN(15,J),J=1,41) /
     $   1500.0D0  ,  1160.0D0  ,  310.00D0  ,  230.00D0  ,  178.00D0  ,
     $   153.00D0  ,  134.00D0  ,  124.00D0  ,  113.00D0  ,  106.00D0  ,
     $   101.00D0  ,  96.000D0  ,  92.000D0  ,  89.000D0  ,  87.000D0  ,
     $   84.000D0  ,  81.000D0  ,  78.500D0  ,  76.500D0  ,  75.000D0  ,
     $   72.000D0  ,  70.000D0  ,  68.000D0  ,  64.500D0  ,  63.000D0  ,
     $   62.000D0  ,  61.000D0  ,  59.500D0  ,  58.500D0  ,  56.500D0  ,
     $   56.500D0  ,  56.000D0  ,  55.500D0  ,  52.000D0  ,  50.000D0  ,
     $   48.000D0  ,  45.000D0  ,  44.000D0  ,  39.200D0  ,  34.500D0  ,
     $   34.500D0  /
C  NEUTRON
      DATA (CSIN(16,J),J=1,41) /
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  ,
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  ,
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  ,
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.10000D0  ,  1.5000D0  ,
     $   7.0000D0  ,  12.000D0  ,  17.000D0  ,  19.500D0  ,  20.500D0  ,
     $   22.000D0  ,  23.500D0  ,  24.800D0  ,  25.800D0  ,  26.500D0  ,
     $   27.000D0  ,  27.500D0  ,  28.000D0  ,  30.000D0  ,  31.000D0  ,
     $   32.000D0  ,  32.500D0  ,  32.500D0  ,  33.000D0  ,  33.500D0  ,
     $   34.000D0  /
C  NEUTRON_BAR
      DATA (CSIN(17,J),J=1,41) /
     $   1394.1D0  ,  948.17D0  ,  262.43D0  ,  197.14D0  ,  149.30D0  ,
     $   127.25D0  ,  110.39D0  ,  101.79D0  ,  92.834D0  ,  87.104D0  ,
     $   83.109D0  ,  79.099D0  ,  75.965D0  ,  73.627D0  ,  72.161D0  ,
     $   69.889D0  ,  67.595D0  ,  65.595D0  ,  64.057D0  ,  63.054D0  ,
     $   61.377D0  ,  60.434D0  ,  59.485D0  ,  56.970D0  ,  55.931D0  ,
     $   55.398D0  ,  54.827D0  ,  53.538D0  ,  52.861D0  ,  51.247D0  ,
     $   51.344D0  ,  50.992D0  ,  50.644D0  ,  47.876D0  ,  46.358D0  ,
     $   44.887D0  ,  42.577D0  ,  41.815D0  ,  38.180D0  ,  34.254D0  ,
     $   34.418D0  /
C  LAMBDA
      DATA (CSIN(18,J),J=1,41) /
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  ,
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  ,
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  ,
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.97815D-01,  1.4577D0  ,
     $   6.2052D0  ,  10.112D0  ,  12.902D0  ,  14.300D0  ,  14.688D0  ,
     $   15.505D0  ,  16.379D0  ,  17.554D0  ,  18.309D0  ,  18.920D0  ,
     $   19.389D0  ,  19.804D0  ,  20.284D0  ,  22.000D0  ,  22.733D0  ,
     $   23.527D0  ,  24.097D0  ,  24.382D0  ,  24.816D0  ,  26.800D0  ,
     $   27.719D0  /
C  LAMBDA_BAR
      DATA (CSIN(19,J),J=1,41) /
     $   1182.4D0  ,  524.50D0  ,  167.30D0  ,  131.43D0  ,  91.895D0  ,
     $   75.743D0  ,  63.184D0  ,  57.376D0  ,  52.502D0  ,  49.313D0  ,
     $   47.326D0  ,  44.762D0  ,  43.222D0  ,  42.015D0  ,  41.221D0  ,
     $   40.244D0  ,  39.504D0  ,  39.145D0  ,  38.860D0  ,  38.731D0  ,
     $   37.987D0  ,  37.814D0  ,  36.326D0  ,  34.750D0  ,  33.953D0  ,
     $   33.635D0  ,  33.349D0  ,  32.938D0  ,  32.785D0  ,  32.092D0  ,
     $   32.373D0  ,  32.312D0  ,  32.329D0  ,  31.261D0  ,  30.597D0  ,
     $   30.073D0  ,  29.228D0  ,  29.182D0  ,  27.683D0  ,  27.107D0  ,
     $   27.956D0  /
C  SIGMA(+)
      DATA (CSIN(20,J),J=1,41) /
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  ,
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  ,
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  ,
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.97815D-01,  1.4577D0  ,
     $   6.2052D0  ,  10.112D0  ,  12.902D0  ,  14.300D0  ,  14.688D0  ,
     $   15.505D0  ,  16.379D0  ,  17.554D0  ,  18.309D0  ,  18.920D0  ,
     $   19.389D0  ,  19.804D0  ,  20.284D0  ,  22.000D0  ,  22.733D0  ,
     $   23.527D0  ,  24.097D0  ,  24.382D0  ,  24.816D0  ,  26.800D0  ,
     $   27.719D0  /
C  SIGMA(-)
      DATA (CSIN(22,J),J=1,41) /
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  ,
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  ,
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  ,
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.97815D-01,  1.4577D0  ,
     $   6.2052D0  ,  10.112D0  ,  12.902D0  ,  14.300D0  ,  14.688D0  ,
     $   15.505D0  ,  16.379D0  ,  17.554D0  ,  18.309D0  ,  18.920D0  ,
     $   19.389D0  ,  19.804D0  ,  20.284D0  ,  22.000D0  ,  22.733D0  ,
     $   23.527D0  ,  24.097D0  ,  24.382D0  ,  24.816D0  ,  26.800D0  ,
     $   27.719D0  /
C  SIGMA(+)_BAR
      DATA (CSIN(23,J),J=1,41) /
     $   1394.1D0  ,  948.17D0  ,  262.43D0  ,  197.14D0  ,  149.30D0  ,
     $   127.25D0  ,  110.39D0  ,  101.79D0  ,  92.834D0  ,  87.104D0  ,
     $   83.109D0  ,  78.563D0  ,  75.292D0  ,  72.760D0  ,  70.900D0  ,
     $   68.467D0  ,  66.314D0  ,  64.955D0  ,  63.746D0  ,  62.623D0  ,
     $   59.233D0  ,  56.946D0  ,  53.355D0  ,  49.810D0  ,  48.090D0  ,
     $   46.839D0  ,  45.695D0  ,  44.863D0  ,  44.062D0  ,  42.599D0  ,
     $   42.684D0  ,  42.328D0  ,  42.041D0  ,  39.508D0  ,  37.880D0  ,
     $   36.299D0  ,  34.075D0  ,  33.553D0  ,  29.723D0  ,  27.600D0  ,
     $   28.120D0  /
C  SIGMA(-)_BAR
      DATA (CSIN(25,J),J=1,41) /
     $   1182.4D0  ,  524.50D0  ,  167.30D0  ,  131.43D0  ,  91.895D0  ,
     $   75.743D0  ,  63.184D0  ,  57.376D0  ,  52.502D0  ,  49.313D0  ,
     $   47.326D0  ,  44.762D0  ,  43.222D0  ,  42.015D0  ,  41.221D0  ,
     $   40.244D0  ,  39.504D0  ,  39.145D0  ,  38.860D0  ,  38.731D0  ,
     $   37.987D0  ,  37.814D0  ,  36.326D0  ,  34.750D0  ,  33.953D0  ,
     $   33.635D0  ,  33.349D0  ,  32.938D0  ,  32.785D0  ,  32.092D0  ,
     $   32.373D0  ,  32.312D0  ,  32.329D0  ,  31.261D0  ,  30.597D0  ,
     $   30.073D0  ,  29.228D0  ,  29.182D0  ,  27.683D0  ,  27.107D0  ,
     $   27.956D0  /
C  XI(0)
      DATA (CSIN(26,J),J=1,41) /
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  ,
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  ,
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  ,
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.95639D-01,  1.4154D0  ,
     $   5.4104D0  ,  8.2240D0  ,  8.8031D0  ,  9.1000D0  ,  8.8761D0  ,
     $   9.0095D0  ,  9.2576D0  ,  10.307D0  ,  10.818D0  ,  11.341D0  ,
     $   11.778D0  ,  12.108D0  ,  12.569D0  ,  14.000D0  ,  14.467D0  ,
     $   15.054D0  ,  15.694D0  ,  16.263D0  ,  16.632D0  ,  20.100D0  ,
     $   21.438D0  /
C  XI(-)
      DATA (CSIN(27,J),J=1,41) /
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  ,
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  ,
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  ,
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.95639D-01,  1.4154D0  ,
     $   5.4104D0  ,  8.2240D0  ,  8.8031D0  ,  9.1000D0  ,  8.8761D0  ,
     $   9.0095D0  ,  9.2576D0  ,  10.307D0  ,  10.818D0  ,  11.341D0  ,
     $   11.778D0  ,  12.108D0  ,  12.569D0  ,  14.000D0  ,  14.467D0  ,
     $   15.054D0  ,  15.694D0  ,  16.263D0  ,  16.632D0  ,  20.100D0  ,
     $   21.438D0  /
C  XI(0)_BAR
      DATA (CSIN(28,J),J=1,41) /
     $   1182.4D0  ,  524.50D0  ,  167.30D0  ,  131.43D0  ,  91.895D0  ,
     $   75.743D0  ,  63.184D0  ,  57.376D0  ,  52.502D0  ,  49.313D0  ,
     $   47.326D0  ,  44.225D0  ,  42.549D0  ,  41.148D0  ,  39.960D0  ,
     $   38.822D0  ,  38.223D0  ,  38.505D0  ,  38.549D0  ,  38.301D0  ,
     $   35.843D0  ,  34.326D0  ,  30.196D0  ,  27.590D0  ,  26.112D0  ,
     $   25.076D0  ,  24.217D0  ,  24.264D0  ,  23.985D0  ,  23.445D0  ,
     $   23.713D0  ,  23.647D0  ,  23.726D0  ,  22.892D0  ,  22.119D0  ,
     $   21.485D0  ,  20.726D0  ,  20.921D0  ,  19.226D0  ,  20.454D0  ,
     $   21.658D0  /
C  XI(-)_BAR
      DATA (CSIN(29,J),J=1,41) /
     $   1076.5D0  ,  312.66D0  ,  119.74D0  ,  98.571D0  ,  63.193D0  ,
     $   49.990D0  ,  39.579D0  ,  35.168D0  ,  32.335D0  ,  30.417D0  ,
     $   29.434D0  ,  27.325D0  ,  26.514D0  ,  25.775D0  ,  25.120D0  ,
     $   24.711D0  ,  24.818D0  ,  25.600D0  ,  26.106D0  ,  26.355D0  ,
     $   25.220D0  ,  24.760D0  ,  21.681D0  ,  20.060D0  ,  19.044D0  ,
     $   18.474D0  ,  18.044D0  ,  18.301D0  ,  18.347D0  ,  18.192D0  ,
     $   18.557D0  ,  18.639D0  ,  18.870D0  ,  18.769D0  ,  18.478D0  ,
     $   18.372D0  ,  18.302D0  ,  18.735D0  ,  18.206D0  ,  20.207D0  ,
     $   21.576D0  /
C  OMEGA(-)
      DATA (CSIN(33,J),J=1,41) /
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  ,
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  ,
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  ,
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.95639D-01,  1.4154D0  ,
     $   5.4104D0  ,  8.2240D0  ,  8.8031D0  ,  9.1000D0  ,  8.8761D0  ,
     $   9.0095D0  ,  9.2576D0  ,  10.307D0  ,  10.818D0  ,  11.341D0  ,
     $   11.778D0  ,  12.108D0  ,  12.569D0  ,  14.000D0  ,  14.467D0  ,
     $   15.054D0  ,  15.694D0  ,  16.263D0  ,  16.632D0  ,  20.100D0  ,
     $   21.438D0  /
C  OMEGA(-)_BAR
      DATA (CSIN(34,J),J=1,41) /
     $   1076.5D0  ,  312.66D0  ,  119.74D0  ,  98.571D0  ,  63.193D0  ,
     $   49.990D0  ,  39.579D0  ,  35.168D0  ,  32.335D0  ,  30.417D0  ,
     $   29.434D0  ,  27.325D0  ,  26.514D0  ,  25.775D0  ,  25.120D0  ,
     $   24.711D0  ,  24.818D0  ,  25.600D0  ,  26.106D0  ,  26.355D0  ,
     $   25.220D0  ,  24.760D0  ,  21.681D0  ,  20.060D0  ,  19.044D0  ,
     $   18.474D0  ,  18.044D0  ,  18.301D0  ,  18.347D0  ,  18.192D0  ,
     $   18.557D0  ,  18.639D0  ,  18.870D0  ,  18.769D0  ,  18.478D0  ,
     $   18.372D0  ,  18.302D0  ,  18.735D0  ,  18.206D0  ,  20.207D0  ,
     $   21.576D0  /

C  ELASTIC CROSS-SECTION FOR MEDI WITH PIONS
C  ALUMINIUM
      DATA (CSPIEL( 1,J),J=1,41) /
     $  0.00000D0  ,  350.00D0  ,  580.00D0  ,  600.00D0  ,  550.00D0  ,
     $   450.00D0  ,  410.00D0  ,  370.00D0  ,  340.00D0  ,  230.00D0  ,
     $   220.00D0  ,  205.00D0  ,  180.00D0  ,  155.00D0  ,  145.00D0  ,
     $   140.00D0  ,  160.00D0  ,  195.00D0  ,  235.00D0  ,  250.00D0  ,
     $   270.00D0  ,  280.00D0  ,  300.00D0  ,  300.00D0  ,  290.00D0  ,
     $   285.00D0  ,  265.00D0  ,  240.00D0  ,  230.00D0  ,  222.00D0  ,
     $   204.00D0  ,  196.00D0  ,  190.00D0  ,  170.00D0  ,  170.00D0  ,
     $   160.00D0  ,  150.00D0  ,  140.00D0  ,  120.00D0  ,  80.000D0  ,
     $   80.000D0  /
C  COPPER
      DATA (CSPIEL( 2,J),J=1,41) /
     $  0.00000D0  ,  700.00D0  ,  1000.0D0  ,  1200.0D0  ,  1300.0D0  ,
     $   1300.0D0  ,  1250.0D0  ,  1250.0D0  ,  1100.0D0  ,  1000.0D0  ,
     $   940.00D0  ,  740.00D0  ,  700.00D0  ,  670.00D0  ,  660.00D0  ,
     $   670.00D0  ,  680.00D0  ,  700.00D0  ,  735.00D0  ,  800.00D0  ,
     $   810.00D0  ,  820.00D0  ,  820.00D0  ,  810.00D0  ,  800.00D0  ,
     $   800.00D0  ,  700.00D0  ,  600.00D0  ,  500.00D0  ,  470.00D0  ,
     $   440.00D0  ,  410.00D0  ,  380.00D0  ,  330.00D0  ,  330.00D0  ,
     $   330.00D0  ,  330.00D0  ,  330.00D0  ,  285.00D0  ,  240.00D0  ,
     $   240.00D0  /
C  LEAD
      DATA (CSPIEL( 3,J),J=1,41) /
     $  0.00000D0  ,  1700.0D0  ,  2200.0D0  ,  2200.0D0  ,  1800.0D0  ,
     $   1300.0D0  ,  1200.0D0  ,  900.00D0  ,  900.00D0  ,  1000.0D0  ,
     $   1100.0D0  ,  1300.0D0  ,  1400.0D0  ,  1420.0D0  ,  1490.0D0  ,
     $   1560.0D0  ,  1580.0D0  ,  1690.0D0  ,  1795.0D0  ,  2000.0D0  ,
     $   2070.0D0  ,  2140.0D0  ,  2050.0D0  ,  2010.0D0  ,  1970.0D0  ,
     $   1880.0D0  ,  1690.0D0  ,  1500.0D0  ,  1420.0D0  ,  1390.0D0  ,
     $   1350.0D0  ,  1360.0D0  ,  1370.0D0  ,  1280.0D0  ,  1290.0D0  ,
     $   1295.0D0  ,  1250.0D0  ,  1200.0D0  ,  1050.0D0  ,  900.00D0  ,
     $   900.00D0  /
C  INELASTIC CROSS-SECTION FOR MEDIA WITH PIONS
C  ALIMINUIM
      DATA (CSPIIN( 1,J),J=1,41) /
     $  0.00000D0  ,  200.00D0  ,  320.00D0  ,  500.00D0  ,  600.00D0  ,
     $   600.00D0  ,  590.00D0  ,  530.00D0  ,  510.00D0  ,  470.00D0  ,
     $   430.00D0  ,  425.00D0  ,  420.00D0  ,  425.00D0  ,  425.00D0  ,
     $   430.00D0  ,  430.00D0  ,  435.00D0  ,  435.00D0  ,  440.00D0  ,
     $   430.00D0  ,  430.00D0  ,  420.00D0  ,  420.00D0  ,  420.00D0  ,
     $   415.00D0  ,  415.00D0  ,  410.00D0  ,  410.00D0  ,  408.00D0  ,
     $   406.00D0  ,  404.00D0  ,  400.00D0  ,  380.00D0  ,  340.00D0  ,
     $   340.00D0  ,  340.00D0  ,  340.00D0  ,  340.00D0  ,  340.00D0  ,
     $   340.00D0  /
C  COPPER
      DATA (CSPIIN( 2,J),J=1,41) /
     $  0.00000D0  ,  400.00D0  ,  800.00D0  ,  1000.0D0  ,  1100.0D0  ,
     $   1200.0D0  ,  1150.0D0  ,  1050.0D0  ,  1000.0D0  ,  900.00D0  ,
     $   860.00D0  ,  860.00D0  ,  850.00D0  ,  850.00D0  ,  840.00D0  ,
     $   830.00D0  ,  820.00D0  ,  810.00D0  ,  805.00D0  ,  800.00D0  ,
     $   800.00D0  ,  800.00D0  ,  800.00D0  ,  800.00D0  ,  800.00D0  ,
     $   800.00D0  ,  800.00D0  ,  800.00D0  ,  800.00D0  ,  780.00D0  ,
     $   760.00D0  ,  740.00D0  ,  720.00D0  ,  720.00D0  ,  700.00D0  ,
     $   690.00D0  ,  680.00D0  ,  670.00D0  ,  665.00D0  ,  660.00D0  ,
     $   660.00D0  /
C  LEAD
      DATA (CSPIIN( 3,J),J=1,41) /
     $  0.00000D0  ,  1000.0D0  ,  1900.0D0  ,  2600.0D0  ,  2900.0D0  ,
     $   3000.0D0  ,  2800.0D0  ,  2600.0D0  ,  2500.0D0  ,  2300.0D0  ,
     $   2200.0D0  ,  2000.0D0  ,  1900.0D0  ,  1880.0D0  ,  1860.0D0  ,
     $   1840.0D0  ,  1820.0D0  ,  1810.0D0  ,  1805.0D0  ,  1800.0D0  ,
     $   1780.0D0  ,  1760.0D0  ,  1750.0D0  ,  1740.0D0  ,  1730.0D0  ,
     $   1720.0D0  ,  1710.0D0  ,  1700.0D0  ,  1680.0D0  ,  1660.0D0  ,
     $   1650.0D0  ,  1640.0D0  ,  1630.0D0  ,  1620.0D0  ,  1610.0D0  ,
     $   1605.0D0  ,  1600.0D0  ,  1600.0D0  ,  1550.0D0  ,  1500.0D0  ,
     $   1500.0D0  /
C  ELASTIC CROSS-SECTION FOR MEDI WITH NUCLEONS
C  ALUMINIUM
      DATA (CSPNEL( 1,J),J=1,41) /
     $   2100.0D0  ,  1800.0D0  ,  1500.0D0  ,  1050.0D0  ,  900.00D0  ,
     $   950.00D0  ,  800.00D0  ,  650.00D0  ,  570.00D0  ,  390.00D0  ,
     $   300.00D0  ,  240.00D0  ,  230.00D0  ,  230.00D0  ,  220.00D0  ,
     $   220.00D0  ,  225.00D0  ,  225.00D0  ,  240.00D0  ,  240.00D0  ,
     $   290.00D0  ,  330.00D0  ,  335.00D0  ,  350.00D0  ,  355.00D0  ,
     $   370.00D0  ,  350.00D0  ,  330.00D0  ,  310.00D0  ,  290.00D0  ,
     $   270.00D0  ,  265.00D0  ,  260.00D0  ,  230.00D0  ,  210.00D0  ,
     $   210.00D0  ,  200.00D0  ,  200.00D0  ,  190.00D0  ,  180.00D0  ,
     $   180.00D0  /
C  COPPER
      DATA (CSPNEL( 2,J),J=1,41) /
     $   3800.0D0  ,  2900.0D0  ,  1850.0D0  ,  1550.0D0  ,  1450.0D0  ,
     $   1520.0D0  ,  1460.0D0  ,  1300.0D0  ,  1140.0D0  ,  880.00D0  ,
     $   700.00D0  ,  620.00D0  ,  540.00D0  ,  560.00D0  ,  460.00D0  ,
     $   460.00D0  ,  470.00D0  ,  470.00D0  ,  480.00D0  ,  480.00D0  ,
     $   580.00D0  ,  600.00D0  ,  610.00D0  ,  620.00D0  ,  620.00D0  ,
     $   620.00D0  ,  590.00D0  ,  580.00D0  ,  460.00D0  ,  440.00D0  ,
     $   420.00D0  ,  400.00D0  ,  480.00D0  ,  430.00D0  ,  380.00D0  ,
     $   380.00D0  ,  380.00D0  ,  380.00D0  ,  380.00D0  ,  380.00D0  ,
     $   380.00D0  /
C  LEAD
      DATA (CSPNEL( 3,J),J=1,41) /
     $   7000.0D0  ,  6000.0D0  ,  4500.0D0  ,  3350.0D0  ,  2700.0D0  ,
     $   3000.0D0  ,  3550.0D0  ,  3970.0D0  ,  3280.0D0  ,  2490.0D0  ,
     $   2100.0D0  ,  1510.0D0  ,  1440.0D0  ,  1370.0D0  ,  1370.0D0  ,
     $   1370.0D0  ,  1400.0D0  ,  1400.0D0  ,  1420.0D0  ,  1420.0D0  ,
     $   1440.0D0  ,  1460.0D0  ,  1460.0D0  ,  1450.0D0  ,  1450.0D0  ,
     $   1470.0D0  ,  1400.0D0  ,  1400.0D0  ,  1380.0D0  ,  1370.0D0  ,
     $   1360.0D0  ,  1350.0D0  ,  1340.0D0  ,  1330.0D0  ,  1320.0D0  ,
     $   1310.0D0  ,  1305.0D0  ,  1300.0D0  ,  1300.0D0  ,  1300.0D0  ,
     $   1300.0D0  /
C  INELASTIC CROSS-SECTION FOR MEDI WITH NUCLEONS
C  ALUMINIUM
      DATA (CSPNIN( 1,J),J=1,41) /
     $  0.00000D0  ,  200.00D0  ,  400.00D0  ,  800.00D0  ,  800.00D0  ,
     $   550.00D0  ,  500.00D0  ,  450.00D0  ,  430.00D0  ,  410.00D0  ,
     $   400.00D0  ,  390.00D0  ,  380.00D0  ,  370.00D0  ,  370.00D0  ,
     $   370.00D0  ,  365.00D0  ,  365.00D0  ,  360.00D0  ,  360.00D0  ,
     $   360.00D0  ,  360.00D0  ,  365.00D0  ,  370.00D0  ,  375.00D0  ,
     $   380.00D0  ,  400.00D0  ,  410.00D0  ,  420.00D0  ,  430.00D0  ,
     $   440.00D0  ,  440.00D0  ,  440.00D0  ,  440.00D0  ,  440.00D0  ,
     $   440.00D0  ,  440.00D0  ,  440.00D0  ,  440.00D0  ,  440.00D0  ,
     $   440.00D0  /
C  COPPER
      DATA (CSPNIN( 2,J),J=1,41) /
     $  0.00000D0  ,  400.00D0  ,  950.00D0  ,  1050.0D0  ,  1050.0D0  ,
     $   980.00D0  ,  940.00D0  ,  900.00D0  ,  860.00D0  ,  820.00D0  ,
     $   800.00D0  ,  780.00D0  ,  760.00D0  ,  740.00D0  ,  740.00D0  ,
     $   740.00D0  ,  730.00D0  ,  730.00D0  ,  720.00D0  ,  720.00D0  ,
     $   720.00D0  ,  720.00D0  ,  730.00D0  ,  740.00D0  ,  750.00D0  ,
     $   760.00D0  ,  800.00D0  ,  820.00D0  ,  820.00D0  ,  820.00D0  ,
     $   820.00D0  ,  820.00D0  ,  820.00D0  ,  820.00D0  ,  820.00D0  ,
     $   820.00D0  ,  820.00D0  ,  820.00D0  ,  820.00D0  ,  820.00D0  ,
     $   820.00D0  /
C  LEAD
      DATA (CSPNIN( 3,J),J=1,41) /
     $  0.00000D0  , 0.00000D0  ,  500.00D0  ,  1450.0D0  ,  1700.0D0  ,
     $   1800.0D0  ,  1750.0D0  ,  1730.0D0  ,  1720.0D0  ,  1710.0D0  ,
     $   1700.0D0  ,  1690.0D0  ,  1660.0D0  ,  1630.0D0  ,  1630.0D0  ,
     $   1630.0D0  ,  1600.0D0  ,  1600.0D0  ,  1580.0D0  ,  1580.0D0  ,
     $   1580.0D0  ,  1580.0D0  ,  1600.0D0  ,  1630.0D0  ,  1650.0D0  ,
     $   1670.0D0  ,  1760.0D0  ,  1800.0D0  ,  1800.0D0  ,  1800.0D0  ,
     $   1800.0D0  ,  1800.0D0  ,  1800.0D0  ,  1800.0D0  ,  1800.0D0  ,
     $   1800.0D0  ,  1800.0D0  ,  1800.0D0  ,  1800.0D0  ,  1800.0D0  ,
     $   1800.0D0  /
      DATA ELAB /
     $  0.10000D-03, 0.20000D-03, 0.30000D-03, 0.40000D-03, 0.50000D-03,
     $  0.70000D-03, 0.10000D-02, 0.20000D-02, 0.30000D-02, 0.40000D-02,
     $  0.50000D-02, 0.70000D-02, 0.10000D-01, 0.15000D-01, 0.20000D-01,
     $  0.25000D-01, 0.32700D-01/
C  TABLES FOR VARIOUS ATOMIC WEIGHTS
      DATA CNLWAT /
     $   1.0000D0  ,  16.000D0  ,  27.000D0  ,  56.000D0  ,  59.000D0  ,
     $   64.000D0  ,  91.000D0  ,  112.00D0  ,  119.00D0  ,  127.00D0  ,
     $   137.00D0  ,  181.00D0  ,  207.00D0  ,  209.00D0  ,  238.00D0  /
      DATA (CNLWEL( 1,J),J=1,17) /
     $   6000.0D0  ,  5500.0D0  ,  5200.0D0  ,  4900.0D0  ,  4800.0D0  ,
     $   4400.0D0  ,  4000.0D0  ,  2900.0D0  ,  2200.0D0  ,  1800.0D0  ,
     $   1400.0D0  ,  1100.0D0  ,  900.00D0  ,  700.00D0  ,  600.00D0  ,
     $   560.00D0  ,  520.00D0  /
      DATA (CNLWEL( 2,J),J=1,17) /
     $   5400.0D0  ,  5050.0D0  ,  4800.0D0  ,  4600.0D0  ,  4399.0D0  ,
     $   4090.0D0  ,  3700.0D0  ,  2600.0D0  ,  1950.0D0  ,  1600.0D0  ,
     $   1300.0D0  ,  900.00D0  ,  700.00D0  ,  800.00D0  ,  1050.0D0  ,
     $   1250.0D0  ,  1320.0D0  /
      DATA (CNLWEL( 3,J),J=1,17) /
     $   5500.0D0  ,  5150.0D0  ,  4900.0D0  ,  4699.0D0  ,  4490.0D0  ,
     $   4150.0D0  ,  3750.0D0  ,  2790.0D0  ,  2100.0D0  ,  1650.0D0  ,
     $   1300.0D0  ,  950.00D0  ,  800.00D0  ,  860.00D0  ,  1000.0D0  ,
     $   1090.0D0  ,  1080.0D0  /
      DATA (CNLWEL( 4,J),J=1,17) /
     $   5499.0D0  ,  4970.0D0  ,  4450.0D0  ,  4080.0D0  ,  3750.0D0  ,
     $   3380.0D0  ,  2900.0D0  ,  2400.0D0  ,  2380.0D0  ,  2350.0D0  ,
     $   2300.0D0  ,  2100.0D0  ,  1720.0D0  ,  1370.0D0  ,  1200.0D0  ,
     $   1060.0D0  ,  870.00D0  /
      DATA (CNLWEL( 5,J),J=1,17) /
     $   5399.0D0  ,  4710.0D0  ,  4180.0D0  ,  3760.0D0  ,  3460.0D0  ,
     $   3150.0D0  ,  2730.0D0  ,  2270.0D0  ,  1850.0D0  ,  1850.0D0  ,
     $   2130.0D0  ,  2330.0D0  ,  2120.0D0  ,  1640.0D0  ,  1310.0D0  ,
     $   1100.0D0  ,  1050.0D0  /
      DATA (CNLWEL( 6,J),J=1,17) /
     $   5099.0D0  ,  4405.0D0  ,  3825.0D0  ,  3455.0D0  ,  3125.0D0  ,
     $   2695.0D0  ,  2350.0D0  ,  1850.0D0  ,  1580.0D0  ,  1820.0D0  ,
     $   2050.0D0  ,  2210.0D0  ,  2000.0D0  ,  1590.0D0  ,  1310.0D0  ,
     $   1120.0D0  ,  1040.0D0  /
      DATA (CNLWEL( 7,J),J=1,17) /
     $   6290.0D0  ,  5960.0D0  ,  5640.0D0  ,  5370.0D0  ,  5150.0D0  ,
     $   4800.0D0  ,  4250.0D0  ,  3150.0D0  ,  2470.0D0  ,  2100.0D0  ,
     $   2230.0D0  ,  2420.0D0  ,  2450.0D0  ,  2050.0D0  ,  1760.0D0  ,
     $   1550.0D0  ,  1330.0D0  /
      DATA (CNLWEL( 8,J),J=1,17) /
     $   6885.0D0  ,  6650.0D0  ,  6350.0D0  ,  6150.0D0  ,  6000.0D0  ,
     $   5700.0D0  ,  5360.0D0  ,  4250.0D0  ,  2800.0D0  ,  1870.0D0  ,
     $   1810.0D0  ,  1820.0D0  ,  2170.0D0  ,  2450.0D0  ,  2150.0D0  ,
     $   1700.0D0  ,  1390.0D0  /
      DATA (CNLWEL( 9,J),J=1,17) /
     $   6600.0D0  ,  6500.0D0  ,  6400.0D0  ,  6249.0D0  ,  6190.0D0  ,
     $   5950.0D0  ,  5520.0D0  ,  4250.0D0  ,  2750.0D0  ,  1900.0D0  ,
     $   1850.0D0  ,  1950.0D0  ,  2340.0D0  ,  2800.0D0  ,  2540.0D0  ,
     $   2100.0D0  ,  1760.0D0  /
      DATA (CNLWEL(10,J),J=1,17) /
     $   7400.0D0  ,  7200.0D0  ,  6999.0D0  ,  6840.0D0  ,  6655.0D0  ,
     $   6320.0D0  ,  5820.0D0  ,  4400.0D0  ,  2850.0D0  ,  2000.0D0  ,
     $   1800.0D0  ,  1800.0D0  ,  2150.0D0  ,  2600.0D0  ,  2350.0D0  ,
     $   1950.0D0  ,  2100.0D0  /
      DATA (CNLWEL(11,J),J=1,17) /
     $   7900.0D0  ,  7700.0D0  ,  7499.0D0  ,  7390.0D0  ,  7202.0D0  ,
     $   6810.0D0  ,  6360.0D0  ,  4920.0D0  ,  3450.0D0  ,  2600.0D0  ,
     $   2200.0D0  ,  1950.0D0  ,  2300.0D0  ,  2800.0D0  ,  2650.0D0  ,
     $   2250.0D0  ,  2050.0D0  /
      DATA (CNLWEL(12,J),J=1,17) /
     $   7900.0D0  ,  7750.0D0  ,  7699.0D0  ,  7590.0D0  ,  7450.0D0  ,
     $   7200.0D0  ,  6850.0D0  ,  5650.0D0  ,  4400.0D0  ,  3700.0D0  ,
     $   3400.0D0  ,  2800.0D0  ,  2700.0D0  ,  3100.0D0  ,  3250.0D0  ,
     $   3100.0D0  ,  2750.0D0  /
      DATA (CNLWEL(13,J),J=1,17) /
     $   6100.0D0  ,  5950.0D0  ,  5750.0D0  ,  5599.0D0  ,  5440.0D0  ,
     $   5200.0D0  ,  4800.0D0  ,  4300.0D0  ,  5800.0D0  ,  5750.0D0  ,
     $   4800.0D0  ,  3420.0D0  ,  2650.0D0  ,  3200.0D0  ,  3650.0D0  ,
     $   3500.0D0  ,  2980.0D0  /
      DATA (CNLWEL(14,J),J=1,17) /
     $   6100.0D0  ,  5950.0D0  ,  5750.0D0  ,  5599.0D0  ,  5440.0D0  ,
     $   5200.0D0  ,  4800.0D0  ,  4300.0D0  ,  5800.0D0  ,  5750.0D0  ,
     $   4800.0D0  ,  3420.0D0  ,  2650.0D0  ,  3200.0D0  ,  3650.0D0  ,
     $   3500.0D0  ,  2980.0D0  /
      DATA (CNLWEL(15,J),J=1,17) /
     $   6600.0D0  ,  6350.0D0  ,  6100.0D0  ,  5899.0D0  ,  5690.0D0  ,
     $   5300.0D0  ,  4850.0D0  ,  4450.0D0  ,  5650.0D0  ,  5700.0D0  ,
     $   4950.0D0  ,  3850.0D0  ,  3050.0D0  ,  3050.0D0  ,  3460.0D0  ,
     $   3650.0D0  ,  3340.0D0  /
      DATA (CNLWIN( 1,J),J=1,17) / 17*0.0D0   /
      DATA (CNLWIN( 2,J),J=1,17) /
     $  0.00000D0  , 0.00000D0  , 0.00000D0  , 0.00000D0  ,  1.0000D0  ,
     $   10.000D0  ,  50.000D0  ,  100.00D0  ,  200.00D0  ,  300.00D0  ,
     $   400.00D0  ,  600.00D0  ,  700.00D0  ,  750.00D0  ,  700.00D0  ,
     $   700.00D0  ,  680.00D0  /
      DATA (CNLWIN( 3,J),J=1,17) /
     $  0.00000D0  , 0.00000D0  , 0.00000D0  ,  1.0000D0  ,  10.000D0  ,
     $   50.000D0  ,  100.00D0  ,  260.00D0  ,  450.00D0  ,  600.00D0  ,
     $   700.00D0  ,  800.00D0  ,  900.00D0  ,  940.00D0  ,  900.00D0  ,
     $   860.00D0  ,  820.00D0  /
      DATA (CNLWIN( 4,J),J=1,17) /
     $   1.0000D0  ,  80.000D0  ,  200.00D0  ,  320.00D0  ,  400.00D0  ,
     $   520.00D0  ,  700.00D0  ,  1000.0D0  ,  1120.0D0  ,  1200.0D0  ,
     $   1200.0D0  ,  1200.0D0  ,  1180.0D0  ,  1130.0D0  ,  1100.0D0  ,
     $   1090.0D0  ,  1080.0D0  /
      DATA (CNLWIN( 5,J),J=1,17) /
     $   1.0000D0  ,  90.000D0  ,  220.00D0  ,  340.00D0  ,  420.00D0  ,
     $   550.00D0  ,  720.00D0  ,  1080.0D0  ,  1300.0D0  ,  1400.0D0  ,
     $   1420.0D0  ,  1420.0D0  ,  1380.0D0  ,  1260.0D0  ,  1190.0D0  ,
     $   1150.0D0  ,  1100.0D0  /
      DATA (CNLWIN( 6,J),J=1,17) /
     $   1.0000D0  ,  95.000D0  ,  225.00D0  ,  345.00D0  ,  425.00D0  ,
     $   555.00D0  ,  750.00D0  ,  1150.0D0  ,  1500.0D0  ,  1680.0D0  ,
     $   1700.0D0  ,  1690.0D0  ,  1550.0D0  ,  1360.0D0  ,  1240.0D0  ,
     $   1180.0D0  ,  1120.0D0  /
      DATA (CNLWIN( 7,J),J=1,17) /
     $   10.000D0  ,  140.00D0  ,  260.00D0  ,  380.00D0  ,  450.00D0  ,
     $   600.00D0  ,  750.00D0  ,  1200.0D0  ,  1580.0D0  ,  1800.0D0  ,
     $   1820.0D0  ,  1830.0D0  ,  1800.0D0  ,  1750.0D0  ,  1690.0D0  ,
     $   1650.0D0  ,  1620.0D0  /
      DATA (CNLWIN( 8,J),J=1,17) /
     $   15.000D0  ,  150.00D0  ,  300.00D0  ,  400.00D0  ,  500.00D0  ,
     $   650.00D0  ,  840.00D0  ,  1500.0D0  ,  2100.0D0  ,  2130.0D0  ,
     $   2140.0D0  ,  2130.0D0  ,  2080.0D0  ,  2000.0D0  ,  1950.0D0  ,
     $   1900.0D0  ,  1860.0D0  /
      DATA (CNLWIN( 9,J),J=1,17) /
     $  0.00000D0  , 0.00000D0  , 0.00000D0  ,  1.0000D0  ,  10.000D0  ,
     $   150.00D0  ,  380.00D0  ,  1000.0D0  ,  1650.0D0  ,  2100.0D0  ,
     $   2100.0D0  ,  2100.0D0  ,  2060.0D0  ,  1950.0D0  ,  1860.0D0  ,
     $   1800.0D0  ,  1740.0D0  /
      DATA (CNLWIN(10,J),J=1,17) /
     $  0.00000D0  , 0.00000D0  ,  1.0000D0  ,  10.000D0  ,  45.000D0  ,
     $   180.00D0  ,  380.00D0  ,  1050.0D0  ,  1900.0D0  ,  2300.0D0  ,
     $   2300.0D0  ,  2200.0D0  ,  2150.0D0  ,  2000.0D0  ,  1900.0D0  ,
     $   1800.0D0  ,  1750.0D0  /
      DATA (CNLWIN(11,J),J=1,17) /
     $  0.00000D0  , 0.00000D0  ,  1.0000D0  ,  10.000D0  ,  48.000D0  ,
     $   190.00D0  ,  390.00D0  ,  1080.0D0  ,  2000.0D0  ,  2400.0D0  ,
     $   2400.0D0  ,  2300.0D0  ,  2200.0D0  ,  2100.0D0  ,  1950.0D0  ,
     $   1850.0D0  ,  1800.0D0  /
      DATA (CNLWIN(12,J),J=1,17) /
     $  0.00000D0  , 0.00000D0  ,  1.0000D0  ,  10.000D0  ,  50.000D0  ,
     $   200.00D0  ,  400.00D0  ,  1100.0D0  ,  2100.0D0  ,  2500.0D0  ,
     $   2500.0D0  ,  2450.0D0  ,  2300.0D0  ,  2100.0D0  ,  2000.0D0  ,
     $   1900.0D0  ,  1850.0D0  /
      DATA (CNLWIN(13,J),J=1,17) /
     $  0.00000D0  , 0.00000D0  , 0.00000D0  ,  1.0000D0  ,  10.000D0  ,
     $   100.00D0  ,  350.00D0  ,  900.00D0  ,  1400.0D0  ,  2000.0D0  ,
     $   2300.0D0  ,  2380.0D0  ,  2400.0D0  ,  2300.0D0  ,  2250.0D0  ,
     $   2200.0D0  ,  2120.0D0  /
      DATA (CNLWIN(14,J),J=1,17) /
     $  0.00000D0  , 0.00000D0  , 0.00000D0  ,  1.0000D0  ,  10.000D0  ,
     $   100.00D0  ,  350.00D0  ,  900.00D0  ,  1400.0D0  ,  2000.0D0  ,
     $   2300.0D0  ,  2380.0D0  ,  2400.0D0  ,  2300.0D0  ,  2250.0D0  ,
     $   2200.0D0  ,  2120.0D0  /
      DATA (CNLWIN(15,J),J=1,17) /
     $  0.00000D0  , 0.00000D0  , 0.00000D0  ,  1.0000D0  ,  10.000D0  ,
     $   100.00D0  ,  400.00D0  ,  950.00D0  ,  1600.0D0  ,  2200.0D0  ,
     $   2550.0D0  ,  2750.0D0  ,  2700.0D0  ,  2600.0D0  ,  2540.0D0  ,
     $   2450.0D0  ,  2360.0D0  /
      DATA (CSCAP(J),J=1,50) /
     $   6.0000D0  ,  5.7000D0  ,  5.5000D0  ,  5.3000D0  ,  5.2000D0  ,
     $   5.1000D0  ,  5.0000D0  ,  4.9000D0  ,  4.8000D0  ,  4.8000D0  ,
     $   4.8000D0  ,  4.8000D0  ,  4.8000D0  ,  4.8000D0  ,  4.8000D0  ,
     $   4.8000D0  ,  4.9000D0  ,  5.0000D0  ,  5.2000D0  ,  5.5000D0  ,
     $   6.0000D0  ,  6.7000D0  ,  7.5000D0  ,  8.5000D0  ,  10.000D0  ,
     $   12.000D0  ,  14.500D0  ,  19.000D0  ,  26.500D0  ,  40.000D0  ,
     $   75.000D0  ,  120.00D0  ,  180.00D0  ,  260.00D0  ,  360.00D0  ,
     $   330.00D0  ,  60.000D0  ,  7.0000D0  ,  9.5000D0  ,  20.000D0  ,
     $   75.000D0  ,  140.00D0  ,  250.00D0  ,  360.00D0  ,  480.00D0  ,
     $   580.00D0  ,  590.00D0  ,  500.00D0  ,  300.00D0  ,  100.00D0  /
      DATA (CSCAP(J),J=51,100) /
     $   200.00D0  ,  300.00D0  ,  400.00D0  ,  470.00D0  ,  500.00D0  ,
     $   430.00D0  ,  100.00D0  ,  20.000D0  ,  22.000D0  ,  40.000D0  ,
     $   560.00D0  ,  950.00D0  ,  1000.0D0  ,  1000.0D0  ,  1000.0D0  ,
     $   990.00D0  ,  920.00D0  ,  860.00D0  ,  790.00D0  ,  740.00D0  ,
     $   650.00D0  ,  600.00D0  ,  540.00D0  ,  470.00D0  ,  440.00D0  ,
     $   390.00D0  ,  360.00D0  ,  340.00D0  ,  320.00D0  ,  310.00D0  ,
     $   280.00D0  ,  2.0000D0  ,  2.5000D0  ,  6.0000D0  ,  13.000D0  ,
     $   38.000D0  ,  65.000D0  ,  140.00D0  ,  280.00D0  ,  300.00D0  ,
     $   430.00D0  ,  580.00D0  ,  650.00D0  ,  800.00D0  ,  920.00D0  ,
     $   1100.0D0  ,  1250.0D0  ,  1400.0D0  ,  1550.0D0  ,  1700.0D0  /
C --- END OF CROSS-SECTION DATA STATEMENTS ---

C --- DATA STMTS. FOR GEANT/GHEISHA PARTICLE CODE CONVERSIONS ---
C --- KIPART(I)=GHEISHA CODE CORRESPONDING TO GEANT   CODE I ---
C --- IKPART(I)=GEANT   CODE CORRESPONDING TO GHEISHA CODE I ---

c$$$      DATA KIPART/
c$$$     $               1,   3,   4,   2,   5,   6,   8,   7,
c$$$     $               9,  12,  10,  13,  16,  14,  15,  11,
c$$$     $              35,  18,  20,  21,  22,  26,  27,  33,
c$$$     $              17,  19,  23,  24,  25,  28,  29,  34,
c$$$     $              35,  35,  35,  35,  35,  35,  35,  35,
c$$$     $              35,  35,  35,  35,  30,  31,  32,  35/

*     DATA IKPART/
*    $               1,   4,   2,   3,   5,   6,   8,   7,
*    $               9,  11,  16,  10,  12,  14,  15,  13,
*    $              25,  18,  26,  19,  20,  21,  27,  28,
*    $              29,  22,  23,  30,  31,  45,  46,  47,
*    $              24,  32,  48/


C     PARAMETER (ONETHR=1./3.)
      DATA ONETHR   / .33333333D0/
      DATA ALPHA    / 6*0.7D0,
     +                0.75D0 ,0.75D0 ,0.75D0 ,
     +                0.76D0,0.76D0 ,0.76D0 ,0.76D0 ,
     +                0.685D0,0.63D0 ,0.685D0,0.63D0,0.685D0,0.63D0,
     +                3*0.685D0,3*0.63D0,2*0.685D0,2*0.63D0,
     +                3*0.7D0,0.685D0,0.63D0,0.7D0/
      DATA ALPHAC    /1.2D0,1.2D0,1.2D0,1.15D0,0.90D0,0.91D0,0.98D0,
     +                1.06D0,1.10D0,1.11D0,1.10D0,1.08D0,1.05D0,1.01D0,
     +                0.985D0,0.962D0,0.945D0,0.932D0,0.925D0,0.920D0,
     +                0.920D0,0.921D0,0.922D0,0.923D0,0.928D0,0.931D0,
     +                0.940D0,0.945D0,0.950D0,0.955D0,0.958D0,0.962D0,
     +                0.965D0,0.976D0,0.982D0,0.988D0,0.992D0,1.010D0,
     +                1.020D0,1.030D0,1.040D0/
      DATA PARTEL/6*0.D0,29*1.D0/
      DATA PARTIN/6*0.D0,1.00D0,0.00D0,1.05D0,1.20D0,1.35D0,1.30D0,
     +            1.20D0,1.00D0,1.30D0,1.00D0,1.30D0,1.00D0,1.30D0,
     +            1.00D0,1.00D0,1.00D0,1.30D0,1.30D0,1.30D0,1.00D0,
     +            1.00D0,1.30D0,1.30D0,1.00D0,1.D0,1.D0,1.D0,1.3D0,
     +            1.D0/
*     DATA ICORR /14*1, 0, 1, 0, 1, 0, 3*1, 3*0, 2*1, 2*0, 4*1, 2*0/
C--  SET INTRC TO 0 FOR IPART = 26-29, 33, 34  ( XI'S AND OMEGA'S )
C-DH- DATA INTRC /6*0, 1, 0, 12*1, 0, 2*2, 0, 1, 4*0, 3*1, 3*0 /
C--  RESET INTRC FOR IPART = 26-29, 33, 34  ( XI'S AND OMEGA'S )
      DATA INTRC /6*0, 1, 0, 12*1, 0, 2*2, 0, 10*1, 0/

C  CROSS-SECTIONS ON NUCLEUS ARE KNOWN ONLY FOR PIONS AND PROTONS.
C  THE GENERAL LAW SIGMA(A)=1.25*SIGMA(TOT,PROTON)*A**ALPHA IS VALID
C  ONLY FOR MOMENTA > 2 GEV.THE PARAMETRIZATION DONE HERE GIVES ONLY
C  A BEHAVIOUR AVERAGED OVER MOMENTA AND PARTICLE TYPES.
C  FOR A DETECTOR WITH ONLY A FEW MATERIALS IT'S OF COURSE MUCHBETTER
C  TO USE TABLES OF THE MEASURED CROSS-SECTIONS .
C  FOR ELEMENTS WITH THE FOLLOWING ATOMIC NUMBERS MEASURED CROSS-
C  SECTIONS ARE AVAILABLE (SEE "PCSDATA").

C                 H   AL     CU     PB
      DATA  CSA  /1.D0 ,27.00D0 ,63.54D0 ,207.19D0 /
      DATA IPART2/9,8,7,11,10,13,12/
      SAVE ALPHA,ALPHAC,PARTEL,PARTIN,CSA,IPART2,INTRC


C --- INITIALIZE CXGHESIG AND SWITCH TO GHEISHA PARTICLE CODE ---
      CXGHESIG=0.0D0
      IPART=KPART

C --- NO INTERACTION FOR GAMMAS, NEUTRINOS, ELECTRONS, POSITRONS, MUONS,
C --- NEUTRAL PIONS, NEUTRAL SIGMAS AND ANTISIGMAS AND NEW PARTICLES.
      IF ( INTRC(IPART) .EQ. 0 ) GOTO 160
      P=PPART
      EK=EKIN
#if !__CXCORSIKA__ && !__CORSIKA8__

      if(itarg.eq.0)then
        KK       = 3
        WCOMP(1) = airw(1)
        WCOMP(2) = airw(2)
        WCOMP(3) = airw(3)
        ACOMP(1) = aira(1)
        ACOMP(2) = aira(2)
        ACOMP(3) = aira(3)
        ZCOMP(1) = airz(1)
        ZCOMP(2) = airz(2)
        ZCOMP(3) = airz(3)
      else
        KK       = 1
        WCOMP(KK)= 1.D0
        ACOMP(KK)= dble(IATARG)
        ZCOMP(KK)= dble(IZTARG)
      endif
#endif

C --- INITIALIZE THE CROSS-SECTIONS WITH 0.0 ---
      DO  K = 1, KK
        AIEL(K)=0.0D0
        AIIN(K)=0.0D0
        AICA(K)=0.0D0
      ENDDO
C
      IF     ((IPART .GE. 30) .AND. (IPART .LE. 32)) THEN

C --- TAKE GEOMETRICAL CROSS-SECTIONS FOR INELASTIC SCATTERING ---
C --- OF DEUTERONS, TRITONS AND ALPHAS ---
        IF     ( IPART .EQ. 30 ) THEN
          APART=2.0D0**ONETHR
        ELSEIF ( IPART .EQ. 31 ) THEN
          APART=3.0D0**ONETHR
        ELSEIF ( IPART .EQ. 32 ) THEN
          APART=4.0D0**ONETHR
        ENDIF
        DO  K = 1, KK
          AIIN(K)=49.0D0*(APART+ACOMP(K)**ONETHR)**2
        ENDDO
#ifdef __CXDEBUG__
        IF (NPRT(9)) WRITE(MCXDBUG,10000)
#endif

      ELSEIF ((IPART .EQ. 16) .AND. (EK .LE. 0.0327D0)) THEN

C --- USE TABLES FOR LOW ENERGY NEUTRONS ---
C --- GET ENERGY BIN ---
        JE2=17
        DO  J = 2, 17
          IF (EK .LT. ELAB(J)) THEN
            JE2=J
            GOTO 40
          ENDIF
        ENDDO

   40   JE1=JE2-1
        EKX=MAX(EK,1.0D-9)
        DELAB=ELAB(JE2)-ELAB(JE1)
        DO 70  K = 1, KK
C --- GET A BIN ---
          JA2=15
          DO  J = 2, 15
            IF (ACOMP(K) .LT. CNLWAT(J)) THEN
              JA2=J
              GOTO 60
            ENDIF
          ENDDO
   60     JA1=JA2-1
          DNLWAT=CNLWAT(JA2)-CNLWAT(JA1)

C --- USE LINEAR INTERPOLATION OR EXTRAPOLATION BY Y=RCE*X+RCA*X+B ---

C --- ELASTIC CROSS-SECTION ---
C --- E INTERPOLATION OR EXTRAPOLATION AT JA1 ---
          DY  = CNLWEL(JA1,JE2)-CNLWEL(JA1,JE1)
          RCE = DY/DELAB
C --- A INTERPOLATION OR EXTRAPOLATION AT JE1 ---
          DY  = CNLWEL(JA2,JE1)-CNLWEL(JA1,JE1)
          RCA = DY/DNLWAT
          B   = CNLWEL(JA1,JE1)-RCE*ELAB(JE1)-RCA*CNLWAT(JA1)
          AIEL(K) = RCE*EK+RCA*ACOMP(K)+B

C --- INELASTIC CROSS-SECTION ---
C --- E INTERPOLATION OR EXTRAPOLATION AT JA1 ---
          DY  = CNLWIN(JA1,JE2)-CNLWIN(JA1,JE1)
          RCE = DY/DELAB
C --- A INTERPOLATION OR EXTRAPOLATION AT JE1 ---
          DY  = CNLWIN(JA2,JE1)-CNLWIN(JA1,JE1)
          RCA = DY/DNLWAT
          B   = CNLWIN(JA1,JE1)-RCE*ELAB(JE1)-RCA*CNLWAT(JA1)
          AIIN(K) = RCE*EK+RCA*ACOMP(K)+B
          IZNO    = int(ZCOMP(K)+0.01D0)
          AICA(K) = 11.12D0*CSCAP(IZNO)/(EKX*1.0E6)**0.577D0
   70   CONTINUE
#ifdef __CXDEBUG__
        IF (NPRT(9)) WRITE(MCXDBUG,10100)
#endif
      ELSE

C --- USE PARAMETRIZATION OF CROSS-SECTION DATA FOR ALL OTHER CASES ---

#ifdef __CXDEBUG__
        IF (NPRT(9)) WRITE(MCXDBUG,10200)
#endif

C --- GET MOMENTUM BIN ---
        J = 40
        DO  I = 2, 41
          IF (P .LT. PLAB(I)) THEN
            J=I-1
            GOTO 90
          ENDIF
        ENDDO

C --- START WITH  CROSS-SECTIONS FOR SCATTERING ON FREE PROTONS ---
C --- USE LINEAR INTERPOLATION OR EXTRAPOLATION BY Y=RC*X+B     ---
   90   DX  = PLAB(J+1)-PLAB(J)
C --- ELASTIC CROSS-SECTION ---
        DY  = CSEL(IPART,J+1)-CSEL(IPART,J)
        RC  = DY/DX
        B   = CSEL(IPART,J)-RC*PLAB(J)
        AIELIN = RC*P+B
C --- INELASTIC CROSS-SECTION ---
        DY  = CSIN(IPART,J+1)-CSIN(IPART,J)
        RC  = DY/DX
        B   = CSIN(IPART,J)-RC*PLAB(J)
        AIININ = RC*P+B
        ALPH   = ALPHA(IPART)
        IF ( IPART .LT. 14 ) THEN
          DY  = ALPHAC(J+1)-ALPHAC(J)
          RC  = DY/DX
          B   = ALPHAC(J)-RC*PLAB(J)
          CORFAC = RC*P+B
          ALPH   = ALPH*CORFAC

          IPART3 = IPART2(IPART-6)

C --- ELASTIC CROSS-SECTION ---
          DY  = CSEL(IPART3,J+1)-CSEL(IPART3,J)
          RC  = DY/DX
          B   = CSEL(IPART3,J)-RC*PLAB(J)
          XSECEL = RC*P+B
C --- INELASTIC CROSS-SECTION ---
          DY  = CSIN(IPART3,J+1)-CSIN(IPART3,J)
          RC  = DY/DX
          B   = CSIN(IPART3,J)-RC*PLAB(J)
          XSECIN = RC*P+B

        ENDIF

C --- NOW MAKE CROSS-SECTIONS FOR COMPONENT K OF COMPOSITION
        DO 100  K = 1, KK
          AIEL(K) = AIELIN
          AIIN(K) = AIININ

          IF ( ACOMP(K) .GE. 1.5D0 ) THEN

C --- A-DEPENDENCE FROM PARAMETRIZATION ---
            CREL = 1.0D0
            CRIN = 1.0D0
C --- GET MEDIUM BIN  1=HYDR.  2=AL  3=CU  4=PB ---
            I = 3
            IF ( ACOMP(K) .LT. 50.0D0 )  I = 2
            IF ( ACOMP(K) .GT. 100.0D0 ) I = 4
            IF     ((IPART .EQ. 14) .OR. (IPART .EQ. 16)) THEN

C --- PROTONS AND NEUTRONS ---

C --- ELASTIC CROSS-SECTION ---
              DY=CSPNEL(I-1,J+1)-CSPNEL(I-1,J)
              RC=DY/DX
              B=CSPNEL(I-1,J)-RC*PLAB(J)
              XSECEL=RC*P+B
C --- INELASTIC CROSS-SECTION ---
              DY=CSPNIN(I-1,J+1)-CSPNIN(I-1,J)
              RC=DY/DX
              B=CSPNIN(I-1,J)-RC*PLAB(J)
              XSECIN=RC*P+B
              IF (AIEL(K) .GE. 0.001D0) CREL=XSECEL/(0.36D0*AIEL(K)*
     +        CSA(I)**1.17D0)
              AITOT=AIEL(K)+AIIN(K)
              IF (AITOT .GE. 0.001D0) CRIN=XSECIN/(AITOT*CSA(I)**
     +          ALPH)

            ELSEIF (IPART .LT. 15) THEN

C --- CALCULATE CORRECTION FACTORS FROM VALUES ON AL,CU,PB FOR ALL ---
C --- MESONS USE LINEAR INTERPOLATION OR EXTRAPOLATION BY Y=RC*X+B ---
C --- NOTE THAT DATA IS ONLY AVAILABLE FOR PIONS AND PROTONS
              WGCH=0.5D0
              IF (ACOMP(K) .LT. 20.0D0)
     +                         WGCH=0.5D0+0.5D0*EXP(-(ACOMP(K)-1.0D0))
              AIEL(K)=WGCH*AIEL(K)+(1.0D0-WGCH)*XSECEL
              AIIN(K)=WGCH*AIIN(K)+(1.0D0-WGCH)*XSECIN

C --- THIS SECTION NOT FOR KAONS ---
              IF (IPART .LT. 10) THEN

C --- ELASTIC CROSS-SECTION ---
                DY=CSPIEL(I-1,J+1)-CSPIEL(I-1,J)
                RC=DY/DX
                B=CSPIEL(I-1,J)-RC*PLAB(J)
                XSPIEL=RC*P+B
C --- INELASTIC CROSS-SECTION ---
                DY=CSPIIN(I-1,J+1)-CSPIIN(I-1,J)
                RC=DY/DX
                B=CSPIIN(I-1,J)-RC*PLAB(J)
                XSPIIN=RC*P+B

                IF (AIEL(K) .GE. 0.001D0) CREL=XSPIEL/(0.36D0* AIEL(K)
     +               *CSA(I)**1.17D0)
                AITOT=AIEL(K)+AIIN(K)
                IF (AITOT .GE. 0.001D0) CRIN=XSPIIN/(AITOT*CSA(I)
     +               **ALPH)
              ENDIF
            ENDIF
            AIIN(K)=CRIN*(AIIN(K)+AIEL(K))*ACOMP(K)**ALPH
            AIEL(K)=CREL*0.36D0*AIEL(K)*ACOMP(K)**1.17D0
            AIEL(K)=AIEL(K)*PARTEL(IPART)
            AIIN(K)=AIIN(K)*PARTIN(IPART)
          ENDIF
  100   CONTINUE

      ENDIF

C --- CALCULATE INTERACTION PROBABILITY ---

      ALAM=0.0D0
      DO  K = 1, KK
        AIEL(K) = AIEL(K)*WCOMP(K)
        AIIN(K) = AIIN(K)*WCOMP(K)
        AICA(K) = AICA(K)*WCOMP(K)
        ALAM = ALAM  + AIIN(K)!+ AIEL(K) + AICA(K)    !tttt only inelastic cs for MC
      ENDDO

C --- PASS THE CROSS-SECTION (MBARN) TO CORSIKA ---
      CXGHESIG=ALAM

      GOTO 999

 160  CONTINUE
C --- PRINTOUT OF SKIPPED PARTICLES IN CASE OF INTERFACE DEBUG ---
#ifdef __CXDEBUG__
      IF (NPRT(9)) WRITE(MCXDBUG,10300) IPART
10000 FORMAT(' *CXGHESIG* GEOM X-SECT. FOR INEL. SCAT. OF D,T AND ALPH')
10100 FORMAT(' *CXGHESIG* X-SECT. FROM LOW ENERGY NEUTRON TABLES')
10200 FORMAT(' *CXGHESIG* X-SECT. FROM PARAMETRIZATION OF DATA')
10300 FORMAT(' *CXGHESIG* GHEISHA PARTICLE ',I3,' SKIPPED')
#endif
  999 RETURN
      END

#if !__CXCORSIKA__ && !__CORSIKA8__

*CMZ :          28/02/2002  10.19.04  by  D. HECK IK FZK KARLSRUHE
*-- Author :    CERN PROGLIB# M103
C=======================================================================

      SUBROUTINE FLPSOR(A,N)

C-----------------------------------------------------------------------
C CERN PROGLIB# M103    FLPSOR          .VERSION KERNFOR  3.15  820113
C ORIG. 29/04/78
C-----------------------------------------------------------------------
C   SORT THE ONE-DIMENSIONAL FLOATING POINT ARRAY A(1),...,A(N) BY
C   INCREASING VALUES
C
C     PROGRAM  M103  TAKEN FROM CERN PROGRAM LIBRARY,  29-APR-78
C-----------------------------------------------------------------------

      IMPLICIT DOUBLE PRECISION(A-H, O-Z)
      DIMENSION A(*)
      COMMON /SLATE/ LT(20),RT(20)
      INTEGER R,RT
      SAVE
C-----------------------------------------------------------------------

      LEVEL=1
      LT(1)=1
      RT(1)=N
   10 L=LT(LEVEL)
      R=RT(LEVEL)
      LEVEL=LEVEL-1
   20 IF ( R .GT. L ) GOTO 200
      IF ( LEVEL ) 50,50,10
C
C   SUBDIVIDE THE INTERVAL L,R
C     L : LOWER LIMIT OF THE INTERVAL (INPUT)
C     R : UPPER LIMIT OF THE INTERVAL (INPUT)
C     J : UPPER LIMIT OF LOWER SUB-INTERVAL (OUTPUT)
C     I : LOWER LIMIT OF UPPER SUB-INTERVAL (OUTPUT)
C
  200 I=L
      J=R
      M=(L+R)/2
      X=A(M)
  220 IF ( A(I) .GE. X ) GOTO 230
      I=I+1
      GOTO 220
  230 IF ( A(J) .LE. X ) GOTO 231
      J=J-1
      GOTO 230
C
  231 IF ( I .GT. J ) GOTO 232
      W=A(I)
      A(I)=A(J)
      A(J)=W
      I=I+1
      J=J-1
      IF (I .LE. J ) GOTO 220
C
  232 LEVEL=LEVEL+1
      IF ( (R-I) .GE. (J-L) ) GOTO 30
      LT(LEVEL)=L
      RT(LEVEL)=J
      L=I
      GOTO 20
   30 LT(LEVEL)=I
      RT(LEVEL)=R
      R=J
      GOTO 20
   50 RETURN
      END


C=======================================================================

      SUBROUTINE GRNDM(RVEC,LENV)

C-----------------------------------------------------------------------
C  G(HEISHA) R(A)ND(O)M (NUMBER GENERATOR)
C
C  THIS SUBROUTINE IS CALLED FROM GHEISHA ROUTINES.
C  ARGUMENTS:
C   RVEC   = VECTOR FIELD TO BE FILLED WITH RANDOM NUMBERS(REAL)
C   LENV   = LENGTH OF VECTOR (# OF RANDNUMBERS TO BE GENERATED)
C-- Author :    T. PIEROG IK FZK KARLSRUHE   25/04/2003
C-----------------------------------------------------------------------
      DOUBLE PRECISION RVEC(*),drangen
      INTEGER          IVEC,LENV
C-----------------------------------------------------------------------

      DO   IVEC = 1, LENV
        RVEC(IVEC) = drangen(dble(IVEC))
      ENDDO

      RETURN
      END
#endif

#endif
#ifdef __QGSJET__
c Last modifications 07.04.2008 Compatibility gcc4 by T.Pierog
c 18.01.2007 update to compile with CORSIKA
c 17.11.2006 add ifragm=0,1 or 2
c 19.05.2003 Link routines between QGSJet03 and CONEX.
c author T. Pierog

#ifdef __STD__
#define __GHEISHA__
#define __QGSJET__
#define __ANALYSIS__
#endif

c-----------------------------------------------------------------------
      subroutine IniQGSJet
c-----------------------------------------------------------------------
c Primary initialization for QGSJet
c-----------------------------------------------------------------------
      implicit double precision (a-h,o-z)
#include "conex.h"
#include "conex.incnex"
#if !__CXCORSIKA__ && !__CORSIKA8__

      COMMON /Q_DEBUG/  DEBUG
      COMMON /Q_AREA43/ MONIOU
      integer debug
      double precision BQGS,BMAXQGS,BMAXNEX,BMINNEX,XA(64,3),XB(64,3)
      COMMON /Q_QGSNEX1/ XA,XB,BQGS,BMAXQGS,BMAXNEX,BMINNEX       !ctp
#endif
      data init/0/
      save init

      if(init.ge.1)return
      init=init+1

#ifdef __CXDEBUG__
      call utisx1('iniqgs  ',4)
      write(*,'(a)')'initialize QGSJet ...'
#endif

#if !__CXCORSIKA__ && !__CORSIKA8__

      debug=0
#ifdef __CXDEBUG__
      if(isx.ge.8)debug=isx-7
      moniou=ifck
#else
      moniou=6
#endif
c common model parameters setting
      call psaset
c particular model parameters setting
      call xxaset
c common initialization procedure
      call qgspsaini
      BMAXNEX=dble(xsbmaxim)
      BMINNEX=dble(xsbminim)
#endif
      if(ilowegy.ne.1.or.MCleModel.eq.2)xsegymin=0.1d0
      if(MCModel.eq.2)xsegymax=min(xsegymax,5.d13)


#ifdef __CXDEBUG__
      call utisx2
#endif
      end

c-----------------------------------------------------------------------
      subroutine IniEvtQGS
c-----------------------------------------------------------------------
c Initialization for each type of event (for given proj, targ and egy)
c-----------------------------------------------------------------------
      implicit double precision (a-h,o-z)
#include "conex.h"
#include "conex.incnex"
#if !__CXCORSIKA__ && !__CORSIKA8__

c QGSJet Common
      double precision XA(64,3),XB(64,3),BQGS,BMAXQGS,BMAXNEX,BMINNEX,e0
      COMMON /Q_QGSNEX1/ XA,XB,BQGS,BMAXQGS,BMAXNEX,BMINNEX
#endif


      if(matargxs.gt.64.or.maprojxs.gt.64)
     & stop'Nucleus too big for QGSJet (Mmax=64) !'
      call cxiclass(idprojxs,iclproxs)
      call cxiclass(idtargxs,icltarxs)
      e0=dble(xselab)
      idp=(idprojxs/10)*10      !convert to bound state
      icp=idtrafocx('nxs','qgs',idp)
      if(icp.eq.0)icp=1-2*int(drangen(dummy)+0.5d0)     !pi0 or rho0 = pi+ or pi-
      call xxaini(e0,icp,maprojxs,matargxs)
#if !__CXCORSIKA__ && !__CORSIKA8__

      xsbmax=BMAXQGS
      xsqgsincs=fqgscrse(xselab,maprojxs,matargxs)
#else
      xsbmax=20d0
      xsqgsincs=1d20
#endif
      if(xsekin.lt.xsegymin)xsqgsincs=0.d0          !below egymin, no interaction
#ifdef __CXDEBUG__
      if(isx.ge.2)write(ifck,*)
     &  'QGSJet used with (E,proj,maproj,matarg,bmax)',e0,icp,maprojxs
     &  ,matargxs,xsbmax
#endif

      return
      end

c-----------------------------------------------------------------------
      subroutine emsqgs(iret)
c-----------------------------------------------------------------------
c  call qgsjet to simulate interaction
c-----------------------------------------------------------------------
      implicit double precision (a-h,o-z)
#include "conex.h"
#include "conex.incnex"
#if !__CXCORSIKA__ && !__CORSIKA8__

      double precision XA(64,3),XB(64,3),BQGS,BMAXQGS,BMAXNEX,BMINNEX
      COMMON /Q_QGSNEX1/ XA,XB,BQGS,BMAXQGS,BMAXNEX,BMINNEX
      common /q_area12/ nsp
      common /q_area14/ esp(4,95000),ich(95000)
      double precision esp
c NSF - number of secondary fragments;
c IAF(i) - mass of the i-th fragment
      COMMON /Q_AREA13/ NSF,IAF(56)
      COMMON /Q_AREA99/ NWT
#else
      common /area12/ nsp
      common /area14/ esp(4,95000),ich(95000)
      double precision esp
c NSF - number of secondary fragments;
c IAF(i) - mass of the i-th fragment
      COMMON /AREA13/ NSF,IAF(56)
      COMMON /AREA99/ NWT
#endif

      iret=0
      if(abs(xsqgsincs).lt.1d-20)goto 1001   !no interaction
#ifdef __CXDEBUG__
      if(isx.ge.4)call cxalist('Determine QGSJet Production&',0,0,0)
#endif
      nptlxs=0
      nsp=0
      nsf=0
      nwt=0
      call psconf

      ncolxs=1
      nevtxs=1
#if !__CXCORSIKA__ && !__CORSIKA8__

      if(BQGS.ge.0.d0)then
        xsbimp=BQGS
        xsphi=2.d0*xspi*drangen(dummy)
      else
#endif
        xsbimp=0.d0
        xsphi=0.d0
#if !__CXCORSIKA__ && !__CORSIKA8__

      endif

#ifdef __CXDEBUG__
      if(isx.ge.6)write(ifck,*)'impact parameter ',xsbimp
#endif

#endif

      ist=1
      call cxconre
      call cxconwr(ist)

      nptlini=nptlxs+1


c keep the projectile spectators as fragments
      if(ifragm.eq.2)then

        if(NSF.gt.0)then
          do is=1,NSF           !count the number of spectators
#ifdef __CXDEBUG__
          if(isx.ge.7)write(ifck,'(a,i5,a,i5)')
     $       ' Projecticle Fragment ',is,' Mass :',IAF(is)
#endif
            nptlxs=nptlxs+1
            istptlxs(nptlxs)=0
            if(IAF(is).eq.1)then
              id=idptlxs(is)
              xsptl(3,nptlxs)=xsptl(3,is)
              xsptl(4,nptlxs)=xsptl(4,is)
              xsptl(5,nptlxs)=xsptl(5,is)
            else
              if(IAF(is).eq.2)then
                id=17
              elseif(IAF(is).eq.3)then
                id=18
              elseif(IAF(is).eq.4)then
                id=19
              else
                id=IAF(is)*100
              endif
              call cxidmass(id,am)
              xsptl(4,nptlxs)=dble(IAF(is))*xsptl(4,is)      !Etot
              xsptl(5,nptlxs)=am                             !mass
              pz2tmp=(xsptl(4,nptlxs)+am)*(xsptl(4,nptlxs)-am)
              if(pz2tmp.gt.0.d0)then
                xsptl(3,nptlxs)=sqrt(pz2tmp)                 !Pz
              else
                write(*,*)'Warning in emsqgs !'
                write(*,*)'energy of fragment too small :',IAF(is),am
     &                     ,xsptl(4,nptlxs)
                xsptl(3,nptlxs)=xsptl(4,nptlxs)
              endif
            endif
            xsptl(1,nptlxs)=0.d0 !P_x
            xsptl(2,nptlxs)=0.d0 !P_y
            ityptlxs(nptlxs)=0
            iorptlxs(nptlxs)=1
            jorptlxs(nptlxs)=maprojxs+matargxs
            ifrptlxs(1,nptlxs)=0
            ifrptlxs(2,nptlxs)=0
            xsorptl(1,nptlxs)=0.d0
            xsorptl(2,nptlxs)=0.d0
            xsorptl(3,nptlxs)=0.d0
            xsorptl(4,nptlxs)=0.d0
            xstivptl(1,nptlxs)=0.d0
            xstivptl(2,nptlxs)=0.d0
            idptlxs(nptlxs)=id
#ifdef __CXDEBUG__
            if(isx.ge.5)write(ifck,'(a,i5,a,i5,a,4(e11.4,1x),f6.3)')
     $       ' Fragment from qgsjet ',nptlxs,' id :',idptlxs(nptlxs)
     $  , ' momentum :',(xsptl(k,nptlxs),k=1,5)
#endif

          enddo
        endif

c make the projectile spectators as free nucleons

      else
        ns=0
        if(NSF.gt.0)then
          do is=1,NSF           !count the number of spectators
            ns=ns+IAF(is)
          enddo
          if(ifragm.eq.1)then
c  remaining nucleus is one fragment
            nptlxs=nptlxs+1
            istptlxs(nptlxs)=0
            xsptl(1,nptlxs)=0.d0
            xsptl(2,nptlxs)=0.d0
            xsptl(4,nptlxs)=0.d0
            inucl=0
            do is=1,ns
              inucl=inucl+1
              xsptl(4,nptlxs)=xsptl(4,nptlxs)+xsptl(4,is)
            enddo
            idnucl=100*inucl
            call cxidmass(idnucl,am)
            xsptl(5,nptlxs)=am  !mass
            ptot=(xsptl(4,nptlxs)+am)*(xsptl(4,nptlxs)-am)
            xsptl(3,nptlxs)=sqrt(ptot)
            ityptlxs(nptlxs)=0
            istptlxs(nptlxs)=0
            iorptlxs(nptlxs)=1
            jorptlxs(nptlxs)=maprojxs
            ifrptlxs(1,nptlxs)=0
            ifrptlxs(2,nptlxs)=0
            xsorptl(1,nptlxs)=xsorptl(1,1)
            xsorptl(2,nptlxs)=xsorptl(2,1)
            xsorptl(3,nptlxs)=xsorptl(3,1)
            xsorptl(4,nptlxs)=xsorptl(4,1)
            xstivptl(1,nptlxs)=xstivptl(1,1)
            xstivptl(2,nptlxs)=xstivptl(2,1)
            idptlxs(nptlxs)=idnucl
          else
            do is=1,ns          !make the ns first projectile nucleon actives
              istptlxs(is)=0
            enddo
          endif
        endif
      endif

c restore target spectators
      ns=0
      if(NWT.lt.matargxs)then
        ns=matargxs-NWT
        do is=maprojxs+1,maprojxs+ns  !make the ns first target nucleon actives
          istptlxs(is)=0
        enddo
      endif


      do is=1,nsp

c ich is the type of secondary hadron, esp - its transverse momentum,
c and its energy
c the following notations for the particles types are used: 0 - pi0, 1 -
c pi+,
c -1 - pi-, 2 - p, -2 - p, 3 - n, -3 - n, 4 - k+, -4 - k-, 5 - k0s, -5 -
c k0l
          ic=ich(is)
#ifdef __CXDEBUG__
          if(isx.ge.7)write(ifck,'(a,i5,a,i5,2a,4(e11.4,1x))')
     $       ' qgsjet particle ',is,' id :',ic,' before conversion'
     $     , ' momentum :',(esp(k,is),k=1,4)
#endif

            nptlxs=nptlxs+1
            if(nptlxs.gt.mxptlxs)stop'qgsjet: mxptlxs too small'
            id=idtrafocx('qgs','nxs',ic)
#ifdef __CXDEBUG__
            if(isx.ge.7)write(ifck,'(a,i5,a,i5,a)')
     $       ' conex particle ',nptlxs,' id :',id,' after conversion'
#endif
            call cxidmass(id,am)


            xsptl(1,nptlxs)=esp(3,is)    !P_x
            xsptl(2,nptlxs)=esp(4,is)    !P_y
            xsptl(3,nptlxs)=esp(2,is)    !P_z
            xsptl(4,nptlxs)=esp(1,is)    !E
            xsptl(5,nptlxs)=am           !mass
            istptlxs(nptlxs)=0
            ityptlxs(nptlxs)=0
            iorptlxs(nptlxs)=1
            jorptlxs(nptlxs)=maprojxs+matargxs
            ifrptlxs(1,nptlxs)=0
            ifrptlxs(2,nptlxs)=0
            xsorptl(1,nptlxs)=0.d0
            xsorptl(2,nptlxs)=0.d0
            xsorptl(3,nptlxs)=0.d0
            xsorptl(4,nptlxs)=0.d0
            xstivptl(1,nptlxs)=0.d0
            xstivptl(2,nptlxs)=0.d0
            idptlxs(nptlxs)=id


#ifdef __CXDEBUG__
            if(isx.ge.5)write(ifck,'(a,i5,a,i5,a,4(e11.4,1x),f6.3)')
     $       ' particle from qgsjet ',nptlxs,' id :',idptlxs(nptlxs)
     $  , ' momentum :',(xsptl(k,nptlxs),k=1,5)
#endif

      enddo

c Decay particles with short life time

      np1=nptlini
41    np2=nptlxs
      do 42 ip=np1,np2
        call cxhdecas(ip,iret)
        if(iret.ne.0)goto 1001
42    continue
      np1=np2+1
      if(np1.le.nptlxs)goto 41


1000  return

1001  iret=1
      goto 1000

      end


c------------------------------------------------------------------------------
      double precision function fqgscrse(egy,mapr,matg)
c------------------------------------------------------------------------------
c hadron-nucleus (hadron-proton) and nucl-nucl particle production cross section
c with qgsjet.
c egy - total lab energy
c maproj - projec mass number     (1<maproj<64)
c matarg - target mass number     (1<matarg<64)
c------------------------------------------------------------------------------
      implicit double precision (a-h,o-z)
#include "conex.incnex"
      dimension wk(3),wa(3),wb(3)
#if !__CXCORSIKA__ && !__CORSIKA8__

      double precision gsect,qgsasect
      COMMON /Q_XSECT/  GSECT(10,5,4)
      COMMON /Q_AREA48/ QGSASECT(10,6,4)
#else
      double precision gsect,asect
      COMMON /XSECT/  GSECT(10,5,4)
      COMMON /AREA48/ ASECT(10,6,4)
#endif

      fqgscrse=0.d0
      ye=max(1.d0,log10(egy))
      je=min(8,int(ye))

      wk(2)=ye-je
      wk(3)=wk(2)*(wk(2)-1.d0)*.5d0
      wk(1)=1.d0-wk(2)+wk(3)
      wk(2)=wk(2)-2.d0*wk(3)

      ya=dble(matg)
      ya=log(ya)*0.72134752d0+1.d0         !0.72134752=1/ln(4)
      ja=min(int(ya),2)
      wa(2)=ya-ja
      wa(3)=wa(2)*(wa(2)-1.d0)*.5d0
      wa(1)=1.d0-wa(2)+wa(3)
      wa(2)=wa(2)-2.d0*wa(3)

      if(mapr.eq.1)then

        do i=1,3
          do m=1,3
         fqgscrse=fqgscrse+gsect(je+i-1,iclproxs,ja+m-1)*wk(i)*wa(m)
          enddo
        enddo

      else

        yb=mapr
        yb=log(yb*0.5d0)*1.442695d0+1.d0          !1.442695=1/ln(2)
        jb=min(int(yb),4)
        wb(2)=yb-jb
        wb(3)=wb(2)*(wb(2)-1.d0)*.5d0
        wb(1)=1.d0-wb(2)+wb(3)
        wb(2)=wb(2)-2.d0*wb(3)

        do i=1,3
          do m=1,3
            do n=1,3
#if !__CXCORSIKA__ && !__CORSIKA8__

              fqgscrse=fqgscrse+qgsasect(je+i-1,jb+n-1,ja+m-1)
     &                     *wk(i)*wa(m)*wb(n)
#else
              fqgscrse=fqgscrse+asect(je+i-1,jb+n-1,ja+m-1)
     &                     *wk(i)*wa(m)*wb(n)
#endif
            enddo
          enddo
        enddo

      endif

      fqgscrse=exp(fqgscrse)
      return
      end

c------------------------------------------------------------------------------
      double precision function qgscrse(egy,mapro,matar,id)
c------------------------------------------------------------------------------
c inelastic cross section of qgsjet
c (id=0 corresponds to air)
c egy - total lab energy
c maproj - projec mass number     (1<maproj<64)
c matarg - target mass number     (1<matarg<64)
c------------------------------------------------------------------------------
      implicit double precision (a-h,o-z)
#include "conex.h"

      qgscrse=0.d0
      if(id.eq.0)then
        do k=1,3
          mt=int(aira(k))
          qgscrse=qgscrse+airw(k)*fqgscrse(egy,mapro,mt)
        enddo
      else
        qgscrse=fqgscrse(egy,mapro,matar)
      endif

      return
      end

#if !__CXCORSIKA__ && !__CORSIKA8__

c--------------------------------------------------------------------
      double precision function psran(b10)


c--------------------------------------------------------------------
c Random number generator
c--------------------------------------------------------------------
      double precision b10,drangen
      psran=drangen(b10)

      return
      end

#endif
#endif
#ifdef __QGSJETII__
c Last modifications 07.04.2008 Compatibility gcc4 by T.Pierog
c 18.01.2007 update to compile with CORSIKA
c 17.11.2006 update ifragm definition
c 19.05.2003 Link routines between QGSJET-II and CONEX.
c author T. Pierog

#ifdef __STD__
#define __GHEISHA__
#define __QGSJET__
#define __ANALYSIS__
#endif

c-----------------------------------------------------------------------
      subroutine IniQGSJETII
c-----------------------------------------------------------------------
c Primary initialization for QGSJET-II
c-----------------------------------------------------------------------
      parameter(iapmax=208)
      implicit double precision (a-h,o-z)
#include "conex.h"
#include "conex.incnex"
#if !__CXCORSIKA__ && !__CORSIKA8__

      integer debug
      common /qgdebug/ debug
      common /qgarr43/ moniou
      double precision bqgs,bmaxqgs,bmaxnex,bminnex,xan,xbn
      common /qgsIInex1/xan(iapmax,3),xbn(iapmax,3)
     *,bqgs,bmaxqgs,bmaxnex,bminnex
#endif
      CHARACTER DATDIR*(132)
      data init/0/
      save init

      if(init.ge.1)return
      init=init+1

#ifdef __CXDEBUG__
      call utisx1('iniqgsII  ',4)
      write(*,'(a)')'initialize QGSJET-II ...'
#endif

#if !__CXCORSIKA__ && !__CORSIKA8__

      debug=0
#ifdef __CXDEBUG__
      if(isx.ge.8)debug=isx-7
      moniou=ifck
#else
      moniou=6
#endif
c model parameter setting
      call qgset
c common initialization procedure
      DATDIR="qgsjetII"
      call qgaini(DATDIR)
      BMAXNEX=dble(xsbmaxim)
      BMINNEX=dble(xsbminim)
#endif

      if(ilowegy.ne.1.or.MCleModel.eq.6)xsegymin=0.1d0
      if(MCModel.eq.6)xsegymax=min(xsegymax,5.d13)


#ifdef __CXDEBUG__
      call utisx2
#endif
      end

c-----------------------------------------------------------------------
      subroutine IniEvtQGSII
c-----------------------------------------------------------------------
c Initialization for each type of event (for given proj, targ and egy)
c-----------------------------------------------------------------------
      implicit double precision (a-h,o-z)
#include "conex.h"
#include "conex.incnex"
c QGSJET-II Common
      parameter(iapmax=208)
      double precision bqgs,bmaxqgs,bmaxnex,bminnex,xan,xbn,e0,qgsect
      common /qgsIInex1/xan(iapmax,3),xbn(iapmax,3)
     *,bqgs,bmaxqgs,bmaxnex,bminnex


      if(matargxs.gt.iapmax.or.maprojxs.gt.iapmax)
     & stop'Nucleus too big for QGSJET-II (Mmax=64) !'
      call cxiclass(idprojxs,iclproxs)
      call cxiclass(idtargxs,icltarxs)
      e0=dble(xselab)
      idp=(idprojxs/10)*10      !convert to bound state
      icp=idtrafocx('nxs','qgs',idp)
      if(icp.eq.0)icp=1-2*int(drangen(dummy)+0.5d0)     !pi0 or rho0 = pi+ or pi-
c      if(abs(icp).eq.6)icp=3          !replace lambda by neutron -> TP200309 not done like this in CORSIKA (only decay)
      if(abs(icp).gt.5)
     &  stop'Projectile not allowed in QGSJET-II !'
      call qgini(e0,icp,maprojxs,matargxs)
#if !__CXCORSIKA__ && !__CORSIKA8__

      xsbmax=BMAXQGS
      xsqgsIIincs=qgsect(e0,iclproxs,maprojxs,matargxs)
#else
      xsbmax=20d0
      xsqgsIIincs=1d20
#endif
      if(xsekin.lt.xsegymin)xsqgsIIincs=0.d0          !below egymin, no interaction
#ifdef __CXDEBUG__
      if(isx.ge.2)write(ifck,*)
     &  'QGSJET-II used with (E,proj,maproj,matarg,bmax)',e0,icp
     &  ,maprojxs,matargxs,xsbmax
#endif

      return
      end

c-----------------------------------------------------------------------
      subroutine emsqgsII(iret)
c-----------------------------------------------------------------------
c  call qgsjet-II to simulate interaction
c-----------------------------------------------------------------------
      implicit double precision (a-h,o-z)
#include "conex.h"
#include "conex.incnex"
      parameter(iapmax=208,nptmax=95000)
      double precision bqgs,bmaxqgs,bmaxnex,bminnex,xan,xbn,esp
      common /qgsIInex1/xan(iapmax,3),xbn(iapmax,3)
     *,bqgs,bmaxqgs,bmaxnex,bminnex
      common /qgarr12/ nsp
      common /qgarr14/ esp(4,nptmax),ich(nptmax)
c nsf - number of secondary fragments;
c iaf(i) - mass of the i-th fragment
      common /qgarr13/ nsf,iaf(iapmax)
      common /qgarr55/ nwt,nwp

      iret=0
      if(abs(xsqgsIIincs).lt.1d-20)goto 1001   !no interaction
#ifdef __CXDEBUG__
      if(isx.ge.4)call cxalist('Determine QGSJet-II Production&',0,0,0)
#endif
      nptlxs=0
      nsp=0
      nsf=0
      nwt=0
      call qgconf

      ncolxs=1
      nevtxs=1
      if(BQGS.ge.0.d0)then
        xsbimp=BQGS
        xsphi=2.d0*xspi*drangen(dummy)
      else
        xsbimp=0.d0
        xsphi=0.d0
      endif

#if !__CXCORSIKA__ && !__CORSIKA8__


#ifdef __CXDEBUG__
      if(isx.ge.6)write(ifck,*)'impact parameter ',xsbimp
#endif

#endif

      ist=1
      call cxconre
      call cxconwr(ist)

      nptlini=nptlxs+1


c keep the projectile spectators as fragments
      if(ifragm.eq.2)then

        if(NSF.gt.0)then
          do is=1,NSF           !count the number of spectators
#ifdef __CXDEBUG__
          if(isx.ge.7)write(ifck,'(a,i5,a,i5)')
     $       ' Projecticle Fragment ',is,' Mass :',IAF(is)
#endif
            nptlxs=nptlxs+1
            istptlxs(nptlxs)=0
            if(IAF(is).eq.1)then
              id=idptlxs(is)
              xsptl(3,nptlxs)=xsptl(3,is)
              xsptl(4,nptlxs)=xsptl(4,is)
              xsptl(5,nptlxs)=xsptl(5,is)
            else
              if(IAF(is).eq.2)then
                id=17
              elseif(IAF(is).eq.3)then
                id=18
              elseif(IAF(is).eq.4)then
                id=19
              else
                id=IAF(is)*100
              endif
              call cxidmass(id,am)
              xsptl(4,nptlxs)=dble(IAF(is))*xsptl(4,is)      !Etot
              xsptl(5,nptlxs)=am                             !mass
              pz2tmp=(xsptl(4,nptlxs)+am)*(xsptl(4,nptlxs)-am)
              if(pz2tmp.gt.0.d0)then
                xsptl(3,nptlxs)=sqrt(pz2tmp)                 !Pz
              else
                write(*,*)'Warning in emsqgs !'
                write(*,*)'energy of fragment too small :',IAF(is),am
     &                     ,xsptl(4,nptlxs)
                xsptl(3,nptlxs)=xsptl(4,nptlxs)
              endif
            endif
            xsptl(1,nptlxs)=0.d0 !P_x
            xsptl(2,nptlxs)=0.d0 !P_y
            ityptlxs(nptlxs)=0
            iorptlxs(nptlxs)=1
            jorptlxs(nptlxs)=maprojxs+matargxs
            ifrptlxs(1,nptlxs)=0
            ifrptlxs(2,nptlxs)=0
            xsorptl(1,nptlxs)=0.d0
            xsorptl(2,nptlxs)=0.d0
            xsorptl(3,nptlxs)=0.d0
            xsorptl(4,nptlxs)=0.d0
            xstivptl(1,nptlxs)=0.d0
            xstivptl(2,nptlxs)=0.d0
            idptlxs(nptlxs)=id
#ifdef __CXDEBUG__
            if(isx.ge.5)write(ifck,'(a,i5,a,i5,a,4(e11.4,1x),f6.3)')
     $       ' Fragment from qgsjet ',nptlxs,' id :',idptlxs(nptlxs)
     $  , ' momentum :',(xsptl(k,nptlxs),k=1,5)
#endif

          enddo
        endif

c make the projectile spectators as free nucleons

      else
        ns=0
        if(NSF.gt.0)then
          do is=1,NSF           !count the number of spectators
            ns=ns+IAF(is)
          enddo
          if(ifragm.eq.1)then
c  remaining nucleus is one fragment
            nptlxs=nptlxs+1
            istptlxs(nptlxs)=0
            xsptl(1,nptlxs)=0.d0
            xsptl(2,nptlxs)=0.d0
            xsptl(4,nptlxs)=0.d0
            inucl=0
            do is=1,ns
              inucl=inucl+1
              xsptl(4,nptlxs)=xsptl(4,nptlxs)+xsptl(4,is)
            enddo
            idnucl=100*inucl
            call cxidmass(idnucl,am)
            xsptl(5,nptlxs)=am  !mass
            ptot=(xsptl(4,nptlxs)+am)*(xsptl(4,nptlxs)-am)
            xsptl(3,nptlxs)=sqrt(ptot)
            ityptlxs(nptlxs)=0
            istptlxs(nptlxs)=0
            iorptlxs(nptlxs)=1
            jorptlxs(nptlxs)=maprojxs
            ifrptlxs(1,nptlxs)=0
            ifrptlxs(2,nptlxs)=0
            xsorptl(1,nptlxs)=xsorptl(1,1)
            xsorptl(2,nptlxs)=xsorptl(2,1)
            xsorptl(3,nptlxs)=xsorptl(3,1)
            xsorptl(4,nptlxs)=xsorptl(4,1)
            xstivptl(1,nptlxs)=xstivptl(1,1)
            xstivptl(2,nptlxs)=xstivptl(2,1)
            idptlxs(nptlxs)=idnucl
          else
            do is=1,ns          !make the ns first projectile nucleon actives
              istptlxs(is)=0
            enddo
          endif
        endif
      endif

c restore target spectators
      ns=0
      if(NWT.lt.matargxs)then
        ns=matargxs-NWT
        do is=maprojxs+1,maprojxs+ns  !make the ns first target nucleon actives
          istptlxs(is)=0
        enddo
      endif


      do is=1,nsp

c ich is the type of secondary hadron, esp - its transverse momentum,
c and its energy
c the following notations for the particles types are used: 0 - pi0, 1 -
c pi+,
c -1 - pi-, 2 - p, -2 - p, 3 - n, -3 - n, 4 - k+, -4 - k-, 5 - k0s, -5 -
c k0l
          ic=ich(is)
#ifdef __CXDEBUG__
          if(isx.ge.7)write(ifck,'(a,i5,a,i5,2a,4(e11.4,1x))')
     $       ' qgsjet particle ',is,' id :',ic,' before conversion'
     $     , ' momentum :',(esp(k,is),k=1,4)
#endif

            nptlxs=nptlxs+1
            if(nptlxs.gt.mxptlxs)stop'qgsjet: mxptlxs too small'
            id=idtrafocx('qgs','nxs',ic)
#ifdef __CXDEBUG__
            if(isx.ge.7)write(ifck,'(a,i5,a,i5,a)')
     $       ' conex particle ',nptlxs,' id :',id,' after conversion'
#endif
            call cxidmass(id,am)


            xsptl(1,nptlxs)=esp(3,is)    !P_x
            xsptl(2,nptlxs)=esp(4,is)    !P_y
            xsptl(3,nptlxs)=esp(2,is)    !P_z
            xsptl(4,nptlxs)=esp(1,is)    !E
            xsptl(5,nptlxs)=am           !mass
            istptlxs(nptlxs)=0
            ityptlxs(nptlxs)=0
            iorptlxs(nptlxs)=1
            jorptlxs(nptlxs)=maprojxs+matargxs
            ifrptlxs(1,nptlxs)=0
            ifrptlxs(2,nptlxs)=0
            xsorptl(1,nptlxs)=0.d0
            xsorptl(2,nptlxs)=0.d0
            xsorptl(3,nptlxs)=0.d0
            xsorptl(4,nptlxs)=0.d0
            xstivptl(1,nptlxs)=0.d0
            xstivptl(2,nptlxs)=0.d0
            idptlxs(nptlxs)=id


#ifdef __CXDEBUG__
            if(isx.ge.5)write(ifck,'(a,i5,a,i5,a,4(e11.4,1x),f6.3)')
     $       ' particle from qgsjet ',nptlxs,' id :',idptlxs(nptlxs)
     $  , ' momentum :',(xsptl(k,nptlxs),k=1,5)
#endif

      enddo

c Decay particles with short life time

      np1=nptlini
41    np2=nptlxs
      do 42 ip=np1,np2
        call cxhdecas(ip,iret)
        if(iret.ne.0)goto 1001
42    continue
      np1=np2+1
      if(np1.le.nptlxs)goto 41


1000  return

1001  iret=1
      goto 1000

      end

c------------------------------------------------------------------------------
      double precision function qgsIIcrse(egy,mapro,matar,id)
c------------------------------------------------------------------------------
c inelastic cross section of qgsjet-II
c (id=0 corresponds to air)
c egy - total lab energy
c maproj - projec mass number     (1<maproj<64)
c matarg - target mass number     (1<matarg<64)
c------------------------------------------------------------------------------
      implicit double precision (a-h,o-z)
#include "conex.h"
#include "conex.incnex"

      qgsIIcrse=0.d0
      if(id.eq.0)then
        do k=1,3
          mt=int(aira(k))
          qgsIIcrse=qgsIIcrse+airw(k)*qgsect(egy,iclproxs,mapro,mt)
        enddo
      else
        qgsIIcrse=qgsect(egy,iclproxs,mapro,matar)
      endif

      return
      end

#if !__CXCORSIKA__ && !__CORSIKA8__

c--------------------------------------------------------------------
      double precision function qgran(b10)
c--------------------------------------------------------------------
c Random number generator
c--------------------------------------------------------------------
      double precision b10,drangen
      qgran=drangen(b10)

      return
      end


      subroutine  LzmaOpenFile(name)
      character*256 name,name2
      name2=name
      end

      subroutine  LzmaCloseFile()
      end

      subroutine LzmaFillArray(dum,idum)
      double precision dum,dum2
      integer idum,idum2
      dum2=dum
      idum2=idum
      end

      integer function size(array)
      double precision array(*)
      size=int(array(1))
      end
#endif

#endif
#ifdef __SIBYLL21__
c Last modifications 07.04.2008 Compatibility gcc4 by T.Pierog
c 18.05.2010 update to compile with CORSIKA after modification of 2008
c 18.01.2007 update to compile with CORSIKA
c 29.11.2004 Link routines between SIBYLL2.1 and CONEX.
c author T. Pierog


#ifdef __STD__
#define __GHEISHA__
#define __QGSJET__
#define __ANALYSIS__
#endif


c-----------------------------------------------------------------------
      subroutine IniSibyll
c-----------------------------------------------------------------------
c Primary initialization for Sibyll
c-----------------------------------------------------------------------
      implicit double precision (a-h,o-z)
#include "conex.h"
#include "conex.incnex"
      INTEGER NCALL, NDEBUG, LUN
      COMMON /S_DEBUG/ NCALL, NDEBUG, LUN
      DOUBLE PRECISION CBR
      INTEGER KDEC,LBARP,IDB
      COMMON /S_CSYDEC/ CBR(223+16+12+8), KDEC(1338+6*(16+12+8)),
     &     LBARP(99), IDB(99)
      data init/0/
      save init

      if(init.ge.1)return
      init=init+1

#ifdef __CXDEBUG__
      call utisx1('inisib  ',4)
      write(*,'(a)')'initialize Sibyll ...'
#endif

#if !__CXCORSIKA__ && !__CORSIKA8__

      Ndebug=0
#endif
#ifdef __CXDEBUG__
      if(isx.ge.8)Ndebug=isx-7
#endif

#if !__CXCORSIKA__ && !__CORSIKA8__


c    divert output
      lun = 7

C... SIBYLL initialization
      CALL SIBYLL_INI

C...Cross sections for nucleus-nucleus and hadron nucleus
      CALL NUC_NUC_INI

C...define all particles as unstable
      do i=1,99
        IDB(i) = abs(IDB(i))   ! >0 means unstable
      enddo
c      EgyHiLoLim=max(100.d0,EgyHiLoLim)

#endif

      xsegymax=min(xsegymax,5d13)

#ifdef __CXDEBUG__
      call utisx2
#endif
      end

c-----------------------------------------------------------------------
      subroutine IniEvtSib
c-----------------------------------------------------------------------
c Initialization for each type of event (for given proj, targ and egy)
c-----------------------------------------------------------------------
      implicit double precision (a-h,o-z)
#include "conex.h"
#include "conex.incnex"
      DOUBLE PRECISION CBR
      INTEGER KDEC,LBARP,IDB
      COMMON /S_CSYDEC/ CBR(223+16+12+8), KDEC(1338+6*(16+12+8)),
     &     LBARP(99), IDB(99)


      if((matargxs.gt.18.and.idtargxs.ne.0).or.maprojxs.gt.56)
     &  stop'Mass too big for Sibyll (Mtrg<18, Mprj<56) !'
      id=idtrafocx('nxs','sib',idprojxs)
      ida=abs(id)
      if(ida.lt.6.or.ida.gt.99.or.(ida.gt.14.and.ida.lt.21))then
        write(ifmt,*)'Wrong projectile for Sibyll:',ida
        stop'projectile not allowed in Sibyll !'
      endif
      if(idtargxs.ne.0.and.idtargxs.ne.1120)
     &  stop'target not allowed in Sibyll !'
      call cxiclass(idprojxs,iclproxs)
      xsbmax=20.d0
      idtrgsibxs=idtargxs
      xssibincs=fsibcrse(xsecms)
      if(xsekin.lt.xsegymin)xssibincs=0.d0          !below xsegymin, no interaction


      if(nrnodyxs.gt.0)then                      !decaying particles
        do nod=1,nrnodyxs
          idd=abs(idtrafocx('nxs','sib',nodyxs(nod)))
          if(idd.lt.50)IDB(idd) = -abs(IDB(idd))
        enddo
      endif

!in Sibyll, all hadronic particle above 15 has to decay (interaction not allowed)
      do iid=15,33
        IDB(iid) = abs(IDB(iid))
      enddo
c Strange baryons (34 to 39) can interact
      do iid=40,99
        IDB(iid) = abs(IDB(iid))
      enddo

#ifdef __CXDEBUG__
      if(isx.ge.2)then
        call ranfgt(seed)       !get seed before shower
        write(ifck,*)
     &  'Sibyll used with (E,proj,maproj,matarg)',xsecms,id,maprojxs
     &  ,matargxs,seed
      endif
#endif

      return
      end

c-----------------------------------------------------------------------
      subroutine emssib(iret)
c-----------------------------------------------------------------------
c  call Sibyll to simulate interaction
c-----------------------------------------------------------------------
      implicit double precision (a-h,o-z)
#include "conex.h"
#include "conex.incnex"
C  SIBYLL
      DOUBLE PRECISION P
      INTEGER NP,LLIST,NP_max
      PARAMETER (NP_max=8000)
      COMMON /S_PLIST/ P(NP_max,5), LLIST(NP_max), NP
      INTEGER NW_max
      PARAMETER (NW_max = 20)
      INTEGER NNSOF,NNJET,JDIF,NWD,NJET,NSOF
      COMMON /S_CHIST/ NNSOF(NW_max),NNJET(NW_max),
     &     JDIF(NW_max),NWD,NJET,NSOF

      COMMON /S_PLNUC/ PA(5,40000), LLA(40000), NPA
      COMMON /S_CLDIF/ LDIFF
      INTEGER          LDIFF
#ifdef CONEX_EXTENSIONS
      PARAMETER (IAMAX=56)
      COMMON /S_NUCLST/ LBEGIN(IAMAX), LEND(IAMAX), LUSED
#endif

      iret=0
      if(abs(xssibincs).lt.1d-20)goto 1001   !no interaction
#ifdef __CXDEBUG__
      if(isx.ge.4)call cxalist('Determine Sibyll Production&',0,0,0)
#endif

#ifdef CONEX_EXTENSIONS
      imaxnuc=0
#endif
      nptlxs=0
      NP=0
      NPA=0
      LDIFF=0        !all types of events

      ncolxs=1
      nevtxs=1
      xsbimp=0.d0
      xsphi=0.d0

      ist=1
      call cxconre
      call cxconwr(ist)

#ifdef CONEX_EXTENSIONS
      do ru_cnt=1,nptlxs
        idnucrct(ru_cnt)=0      ! particle do not originat from interaction        
      enddo
#endif
      itrg=matargxs
      if(itrg.gt.18)stop'target not allowed in Sibyll (2) !'
      if(maprojxs.eq.1)then             !hadronic projectile
        L0=idtrafocx('nxs','sib',idprojxs)
c itrg=13 is proton target
        CALL SIBYLL (L0, itrg, xsecms)
        CALL DECSIB
c#ifdef CONEX_EXTENSIONS
c        if (doResampling) then
c          call resampleSIBYLL(NP, xselab, idprojxs, SQS, xsamproj)
c        endif
c#endif
#ifdef __CXDEBUG__
        if(isx.ge.5)write(ifck,'(a,i5)')
     $         ' number of particles from Sibyll :',NP
#endif
#ifdef CONEX_EXTENSIONS
        imaxnuc=1
        npnucs(1)=0
#endif
        do k=1,NP

c LLIST is the code of final particle, P - its 4-momentum and mass.
          ic=LLIST(k)

#ifdef __CXDEBUG__
          if(isx.ge.7)write(ifck,'(a,i5,a,i5,2a,4(e11.4,1x))')
     $       ' Sibyll particle ',k,' id :',ic,' before conversion'
     $     , ' momentum :',(P(k,i),i=1,5)
#endif

          nptlxs=nptlxs+1
          if(nptlxs.gt.mxptlxs)stop'Sibyll: mxptlxs too small'

          if(abs(ic).ge.10000)then
            ic=ic-sign(10000,ic)
            istptlxs(nptlxs)=1
#ifdef CONEX_EXTENSIONS
            idnucrct(nptlxs)=0 ! BAD PARTICLE (instable, non-final)
#endif
          else
            istptlxs(nptlxs)=0
#ifdef CONEX_EXTENSIONS
            idnucrct(nptlxs)=1 ! all particles from the same nucleon-air interaction
            npnucs(1)=npnucs(1)+1
#endif
          endif

          id=idtrafocx('sib','nxs',ic)
#ifdef __CXDEBUG__
          if(isx.ge.7)write(ifck,'(a,i5,a,i5,a)')
     $         ' conex particle ',nptlxs,' id :',id,' after conversion'
#endif


          xsptl(1,nptlxs)=P(k,1)   !P_x
          xsptl(2,nptlxs)=P(k,2)   !P_y
          xsptl(3,nptlxs)=P(k,3)   !P_z
          xsptl(4,nptlxs)=P(k,4)   !E
          xsptl(5,nptlxs)=P(k,5)   !mass
          ityptlxs(nptlxs)=0
          iorptlxs(nptlxs)=1
          jorptlxs(nptlxs)=maprojxs+matargxs
          ifrptlxs(1,nptlxs)=0
          ifrptlxs(2,nptlxs)=0
          xsorptl(1,nptlxs)=0.d0
          xsorptl(2,nptlxs)=0.d0
          xsorptl(3,nptlxs)=0.d0
          xsorptl(4,nptlxs)=0.d0
          xstivptl(1,nptlxs)=0.d0
          xstivptl(2,nptlxs)=0.d0
          idptlxs(nptlxs)=id


#ifdef __CXDEBUG__
          if(isx.ge.5)write(ifck,'(a,i5,a,i5,a,4(e11.4,1x),f6.3)')
     $         ' particle from Sibyll ',nptlxs,' id :',idptlxs(nptlxs)
     $         , ' momentum :',(xsptl(i,nptlxs),i=1,5)
#endif


        enddo
        ns=0
        if(NWD.lt.matargxs)then
          ns=matargxs-NWD
          do is=maprojxs+1,maprojxs+ns !make the ns first target nucleon actives
            istptlxs(is)=0
          enddo
        endif
      else                      !for nucleus projectile
        ns=0           !number of projectile spectators
        nbar=0
        IAP = maprojxs
        CALL SIBNUC (IAP, itrg, xsecms)
#ifdef __CXDEBUG__
        if(isx.ge.5)write(ifck,'(a,i5)')
     $         ' number of particles from Sibyll :',NPA
#endif
#ifdef CONEX_EXTENSIONS
        imaxnuc=LUSED
        do ID_NUC=1,LUSED
          npnucs(ID_NUC)=1+LEND(ID_NUC)-LBEGIN(ID_NUC)
c ru          write(*,*) 'SIBYLL SEMI-SUPERPOSITION MAPPING:',
c ru     &         ' ID_NUC,LBEGIN, LEND, npnucs', ID_NUC, 
c ru     &         LBEGIN(ID_NUC), LEND(ID_NUC), npnucs(ID_NUC)
        enddo
#endif
        do k=1,NPA

c LLIST is the code of final particle, P - its 4-momentum and mass.
          ic=LLA(k)

#ifdef __CXDEBUG__
          if(isx.ge.7)write(ifck,'(a,i5,a,i5,2a,5(e11.4,1x))')
     $       ' Sibyll particle ',k,' id :',ic,' before conversion'
     $     , ' momentum :',(PA(i,k),i=1,5)
#endif


          nNuc=0
          if(ic.ge.1001) then                !count spectators
            nNuc=ic-1000
            if(ifragm.le.1
     &         .or.dble(PA(4,k))/dble(nNuc).lt.xsegymin)then   !nuclear interaction only above min energy, otherwise : fragmentation
              ns=ns+nNuc
              goto 100
            elseif(ic.eq.1001)then
              if(drangen(dummy).lt.0.45d0) then
                ic = 13
              else
                ic = 14
              endif
              nNuc=0
            else
              ptm=sqrt(PA(1,k)*PA(1,k)+PA(2,k)*PA(2,k)+PA(5,k)*PA(5,k))
              PA(4,k)=PA(4,k)*float(nNuc)            !energy by nucleon
              PA(3,k)=sign(sqrt((PA(4,k)+ptm)*(PA(4,k)-ptm)),PA(3,k))
            endif
          endif
          nptlxs=nptlxs+1
          if(nptlxs.gt.mxptlxs)stop'Sibyll: mxptlxs too small'
            id=idtrafocx('sib','nxs',ic)
#ifdef __CXDEBUG__
          if(isx.ge.7)write(ifck,'(a,i5,a,i5,a)')
     $         ' conex particle ',nptlxs,' id :',id,' after conversion'
#endif


          nbar=nbar+nNuc
          if(abs(id).gt.1000.and.nNuc.eq.0)then
            nbar=nbar+sign(1,id)
          endif
#ifdef CONEX_EXTENSIONS
          idnucrct(nptlxs)=0 ! initial: no nucleon interaction
          do ID_NUC=1,LUSED
            if (k.ge.LBEGIN(ID_NUC).and.k.le.LEND(ID_NUC))then 
              idnucrct(nptlxs)=ID_NUC
              goto 4557 ! break loop
            endif
          enddo
 4557     xsptl(1,nptlxs)=PA(1,k)   !P_x
#else
          xsptl(1,nptlxs)=PA(1,k)   !P_x
#endif
          xsptl(2,nptlxs)=PA(2,k)   !P_y
          xsptl(3,nptlxs)=PA(3,k)   !P_z
          xsptl(4,nptlxs)=PA(4,k)   !E
          xsptl(5,nptlxs)=PA(5,k)   !mass
          istptlxs(nptlxs)=0
          ityptlxs(nptlxs)=0
          iorptlxs(nptlxs)=1
          jorptlxs(nptlxs)=maprojxs+matargxs
          ifrptlxs(1,nptlxs)=0
          ifrptlxs(2,nptlxs)=0
          xsorptl(1,nptlxs)=0.d0
          xsorptl(2,nptlxs)=0.d0
          xsorptl(3,nptlxs)=0.d0
          xsorptl(4,nptlxs)=0.d0
          xstivptl(1,nptlxs)=0.d0
          xstivptl(2,nptlxs)=0.d0
          idptlxs(nptlxs)=id


#ifdef __CXDEBUG__
          if(isx.ge.5)write(ifck,'(a,i5,a,i5,a,4(e11.4,1x),f6.3,a,i3)')
     $         ' particle from Sibyll ',nptlxs,' id :',idptlxs(nptlxs)
     $         , ' momentum :',(xsptl(i,nptlxs),i=1,5), ' nbar :',nbar
#endif


 100    enddo
        ntw=nbar-(maprojxs-ns)
        nsf=0
        if(ntw.lt.matargxs)then
          nts=matargxs-ntw
          do is=maprojxs+1,maprojxs+nts !make the nts first target nucleon actives (not wounded)
            istptlxs(is)=0
          enddo
        else
          nsf=maprojxs+matargxs-nbar
        endif
#ifdef __CXDEBUG__
          if(isx.ge.5)write(ifck,'(a,i3,a,i3,a,i2,a)')
     $         ' target spectators :',matargxs-ntw
     $        ,' projectile spectators (ns) :',nsf,' (',ns,')'
#endif
        if((ifragm.le.1.or.nsf.gt.ns).and.nsf.le.maprojxs)then
          if(ifragm.eq.2)ns=nsf-ns
          if(ifragm.eq.1.and.ns.gt.0)then
c  remaining nucleus is one fragment
            nptlxs=nptlxs+1
            istptlxs(nptlxs)=0
#ifdef CONEX_EXTENSIONS
            idnucrct(nptlxs)=0  ! initial: no nucleon interaction
#endif
            xsptl(1,nptlxs)=0.d0
            xsptl(2,nptlxs)=0.d0
            xsptl(4,nptlxs)=0.d0
            inucl=0
            do is=1,ns
              inucl=inucl+1
              xsptl(4,nptlxs)=xsptl(4,nptlxs)+xsptl(4,is)
            enddo
            idnucl=100*inucl
            call cxidmass(idnucl,am)
            xsptl(5,nptlxs)=am  !mass
            ptot=(xsptl(4,nptlxs)+am)*(xsptl(4,nptlxs)-am)
            xsptl(3,nptlxs)=sqrt(ptot)
            ityptlxs(nptlxs)=0
            istptlxs(nptlxs)=0
            iorptlxs(nptlxs)=1
            jorptlxs(nptlxs)=maprojxs
            ifrptlxs(1,nptlxs)=0
            ifrptlxs(2,nptlxs)=0
            xsorptl(1,nptlxs)=xsorptl(1,1)
            xsorptl(2,nptlxs)=xsorptl(2,1)
            xsorptl(3,nptlxs)=xsorptl(3,1)
            xsorptl(4,nptlxs)=xsorptl(4,1)
            xstivptl(1,nptlxs)=xstivptl(1,1)
            xstivptl(2,nptlxs)=xstivptl(2,1)
            idptlxs(nptlxs)=idnucl
          else
            do is=1,ns         !make the nsf first projectile nucleon actives (not wounded)
              istptlxs(is)=0
            enddo
          endif
        endif
      endif

c Decay particles with short life time

      np1=1
41    np2=nptlxs
      do 42 ip=np1,np2
        call cxhdecas(ip,iret)
        if(iret.ne.0)goto 1001
42    continue
      np1=np2+1
      if(np1.le.nptlxs)goto 41



1000  return

1001  iret=1
      goto 1000

      end


c------------------------------------------------------------------------------
      double precision function fsibcrse(egy)
c------------------------------------------------------------------------------
c hadron-proton particle production cross section with Sibyll.
c egy - center of mass energy
c------------------------------------------------------------------------------
      implicit double precision (a-h,o-z)
      DIMENSION SDIF(3)
#include "conex.incnex"

      if(iclproxs.eq.1)then
        L=2
      elseif(iclproxs.eq.2)then
        L=1
      else
        L=3
      endif
      call SIB_SIGMA_HP(L,egy,ST,SEL,SINEL,SDIF,SL,RHO)
      fsibcrse=SINEL

      return
      end

c------------------------------------------------------------------------------
      double precision function sibcrse(egy,mapro,id)
c------------------------------------------------------------------------------
c inelastic cross section of Sibyll (only for hadron-air and
c nucleus-air)
c egy - lab energy per nucleon in GeV
c maproj - projec mass number     (1<maproj<64)
c id - proj id (sibyll code)
c------------------------------------------------------------------------------
      implicit double precision (a-h,o-z)
#include "conex.h"
      COMMON /CLENNN/ SSIGNUC(60), ALNUC(60)

      sibcrse=1.d+35
      E0=egy*1.e-3        !e0 in TeV
      if(mapro.eq.1)then
        AL0=FPNI(E0,id)       !Sibyll interaction lenght
      else
        CALL  SIGNUC_INI(mapro,E0) !  fills SSIGNUC and ALNUC
        AL0=ALNUC(mapro)       !Sibyll interaction lenght
      endif

c conversion to cross section with conex air composition
      if(AL0.gt.0.d0)sibcrse=airava/(avog*AL0)

      return
      end

#if !__CXCORSIKA__ && !__CORSIKA8__

c--------------------------------------------------------------------
      double precision function S_RNDM(idum)
c--------------------------------------------------------------------
c random number generator
c--------------------------------------------------------------------
      double precision drangen
 1    S_RNDM=drangen(dble(idum))
      if(S_RNDM.le.0d0.or.S_RNDM.ge.1d0) goto 1

      return
      end

C=======================================================================

      DOUBLE PRECISION FUNCTION GASDEV(IDUM)

C-----------------------------------------------------------------------
C   Gaussian deviation
c   linked to corsikas gaussian random number generator to keep
c   random number sequence intact.
C-----------------------------------------------------------------------
      IMPLICIT NONE

      DOUBLE PRECISION RANNOR,XMEAN,XDEV
      INTEGER          IDUM
      SAVE
      EXTERNAL         RANNOR
C-----------------------------------------------------------------------
      GASDEV = IDUM
      XMEAN  = 0.D0
      XDEV   = 1.D0
      GASDEV = RANNOR(XMEAN,XDEV)

      RETURN
      END

#endif
#endif
#ifdef __EPOS__
c Last modifications 03.07.2020 Compatibility with CORSIKA8 by T.Pierog
c 07.04.2008 Compatibility gcc4 
c 18.01.2007 update to compile with CORSIKA
c 05.09.2005 Add QGSJET-II subroutine definition to compile
c with preprocessing command -D__ALL__ without __QGSJETII__
c 18.05.04 Link file for conex with nexus
c author T. Pierog

#ifdef __STD__
#define __GHEISHA__
#define __QGSJET__
#define __ANALYSIS__
#endif

c-----------------------------------------------------------------------
      subroutine IniEpos
c-----------------------------------------------------------------------
#include "epos.inc"
#include "epos.incems"
      double precision seedp

c Conex common

      double precision airz,aira,airw,airavz,airava,airi
      common/cxair/airz(3),aira(3),airw(3),airavz,airava,airi(3)
      common/xsfiles/ixsfcx
      character*500 xsfnch,xsfnii,xsfnid,xsfnie,xsfnrj,xsfncs
      common/xsfname/  xsfnch, xsfnii, xsfnid, xsfnie, xsfnrj, xsfncs
      common/xsnfname/nxsfnch,nxsfnii,nxsfnid,nxsfnie,nxsfnrj,nxsfncs
      double precision xsegymin,xsegymax,xselab,xsecms,xsekin,xspnll
     *,xsengy
      common/xsenrgy/xsegymin,xsegymax,xselab,xsecms,xsekin,xspnll
     *,xsengy
      parameter(mxnodyxs=20)
      common/xsnodcy/nrnodyxs,nodyxs(mxnodyxs)
      common/xsappli/iapplxs,modelxs
      common/xsevent/neventxs,iframexs
      integer ifragm
      common/cxfragm/ifragm
c      data inicnt/0/
      data init/0/
      save init

      if(init.ge.1)return
      init=init+1

#ifdef __CXDEBUG__

      call utisx1('iniepos ',4)
      write(*,'(a)')'initialize EPOS ...'
#endif

      if(ilowegy.ne.1.or.MCleModel.eq.4)xsegymin=dble(0.5*egymin**2)
      if(MCModel.eq.4)xsegymax=min(xsegymax,dble(0.5*egymax**2))
      nrnody=nrnodyxs
      do i=1,nrnody
        nody(i)= nodyxs(i)
      enddo
#if !__CXCORSIKA__ && !__CORSIKA8__

      inicnt=inicnt+1
c      isetcs=2                  ! epos cross-section from tabulated calculation (h-A and AA)
      isetcs=3                  ! epos cross-section from tabulated simulations (h-A and A-A)
      infragm=ifragm
      isigma=0                  !do not print out the cross section on screen
      ionudi=1
      

      nfnii=nxsfnii             ! epos file name
      fnii=xsfnii
      nfnid=nxsfnid
      fnid=xsfnid
      nfnie=nxsfnie
      fnie=xsfnie
      nfnrj=nxsfnrj
      fnrj=xsfnrj
      nfncs=nxsfncs
      fncs=xsfncs
      nfnch=nxsfnch
      fnch=xsfnch

c air

      do i=1,3
        airanxs(i)=sngl(aira(i))
        airznxs(i)=sngl(airz(i))
        airwnxs(i)=sngl(airw(i))
      enddo
      airavanxs=sngl(airava)
      airavznxs=sngl(airavz)

      iappl  = iapplxs
      nevent = neventxs
      iframe = iframexs

      if(fnch(1:nfnch).ne.'none')
     &  open(ifcx,file=fnch(1:nfnch),status='unknown')

      call iclass(idproj,iclpro)
      call iclass(idtarg,icltar)
      if(inicnt.eq.1)then
        call ranfgt(seedp)      !not to change the seed ...
        call hdecin(.false.)
        call hnbspd(iospec)
        ktnbod=0
        call hnbpajini
        if(iclegy2.gt.1)then
          egyfac=(egymax*1.0001/egylow)**(1./float(iclegy2-1))
        else
          egyfac=1.
        endif
      endif
      maproj=mamx               !to set difnuc up to the maximum mass
      call conini
      call psaini
      call ranfst(seedp)        ! ... after this initialization

#endif

#ifdef __CXDEBUG__
      call utisx2
#endif

      end


c-----------------------------------------------------------------------
      subroutine IniEvtEpo
c-----------------------------------------------------------------------
c Initialization for each type of event (for given proj, targ and egy)
c-----------------------------------------------------------------------
#include "epos.inc"
      common/geom/rmproj,rmtarg,bmax,bkmx
      double precision tpro,zpro,ttar,ztar,ttaus,detap,detat
      common/cttaus/tpro,zpro,ttar,ztar,ttaus,detap,detat /ctain/mtain
      double precision rcproj,rctarg
      common/geom1/rcproj,rctarg
c Conex common
#ifdef __CXDEBUG__
      parameter (mxisx=200)
      character*500 subisx,textisx
      common/cxisx/isx,nisx,subisx(mxisx),isxsub(mxisx)   !also in gheisha_nexus
     &      ,isxsave,isxxsave,textisx
      character*500 fnho,fnck,fnwle,fnwhe,fndkz,fndkl,fndks,fndkm
     &,fnilo,fndke,fndkn,fndkg,fnwgh,fnwgl
      common /cxfiles/fnho,ifho,fnck,ifck,fnwle,ifwle,fnwhe,ifwhe
     &,fndkz,ifdkz,fndkl,ifdkl,fndks,ifdks,fndkm,ifdkm,fnilo,ifilo
     &,fndke,ifdke,fndkn,ifdkn,fndkg,ifdkg,fnwgh,ifwgh
     &,fnwgl,ifwgl
#endif
      double precision xsegymin,xsegymax,xselab,xsecms,xsekin,xspnll
     *,xsengy
      common/xsenrgy/xsegymin,xsegymax,xselab,xsecms,xsekin,xspnll
     *,xsengy
      common/xsnucl1/laprojxs,maprojxs,latargxs,matargxs
      common/xshadr2/idprojxs,idtargxs
      double precision xsamproj,xsamtarg,xsypjtl,xsyhaha,xspnullx
      common/xschadron/xsamproj,xsamtarg,xsypjtl,xsyhaha,xspnullx
      double precision xsrmproj,xsrmtarg,xsbmax,xsbminim,xsbmaxim,xsbkmx
      common/xsgeom/xsrmproj,xsrmtarg,xsbmax,xsbminim,xsbmaxim,xsbkmx
      double precision xsrcproj,xsrctarg
      common/xsgeom1/xsrcproj,xsrctarg
      double precision xstpro,xszpro,xsttar,xsztar,xsttaus
     &,xsdetap,xsdetat
      common/xscttaus/xstpro,xszpro,xsttar,xsztar,xsttaus
     &,xsdetap,xsdetat

      maproj=maprojxs
      laproj=laprojxs
      matarg=matargxs
      latarg=latargxs
      idproj=idprojxs
      idtarg=idtargxs
      amproj=xsamproj
      amtarg=xsamtarg
      call idspin(idproj,ispin,jspin,istra)
      isoproj=sign(1,idproj)*ispin
      call idspin(idtarg,ispin,jspin,istra)
      isotarg=sign(1,idtarg)*ispin

      engy=sngl(xsengy)
      elab=sngl(xselab)
      ecms=sngl(xsecms)
      ekin=sngl(xsekin)
      pnll=sngl(xspnll)
      pnullx=sngl(xspnullx)
      yhaha=sngl(xsyhaha)
      ypjtl=sngl(xsypjtl)
      detap=xsdetap
      detat=xsdetat
      tpro=xstpro
      zpro=xszpro
      ttar=xsttar
      ztar=xsztar

c      xsbminim=dble(bminim)     !not needed and can interfer with other MC
c      xsbmaxim=dble(bmaxim)

      call iclass(idproj,iclpro)
      call iclass(idtarg,icltar)
      call emsini(engy,idproj,idtarg)
      call paramini(1)
      bkmxndif=conbmxndif()
      bkmx=conbmx()
      xsbkmx=dble(bkmx)

      if(maproj.gt.1.or.matarg.gt.1)then
        xsbmax=xsrmproj+xsrmtarg
      else
        xsbmax=xsbkmx
      endif

      bimevt=-1
      bmax=sngl(xsbmax)
      rmproj=sngl(xsrmproj)
      rmtarg=sngl(xsrmtarg)
      rcproj=xsrcproj
      rctarg=xsrctarg
      call xsigma                          !set some variabkle according to xs
      if(idtarg.eq.0)idtarg=1120           !air = nucleus
#ifdef __CXDEBUG__
      if(isx.ge.2)write(ifck,*)
     &  'Epos used with (E,proj,maproj,matarg,bmax)',elab,idproj
     &  ,maproj,matarg,bmax
#endif
      return
      end

c-----------------------------------------------------------------------
      double precision function cxepocrse(ek,mapro,matar,id)
c-----------------------------------------------------------------------
c Epos cross section in double precision
c-----------------------------------------------------------------------
#include "epos.inc"
      double precision ek
      common/xshad10/iclproxs,icltarxs      !in conex.incnex

      iclpro=iclproxs
      icltar=icltarxs

      cxepocrse=dble(eposcrse(sngl(ek),mapro,matar,id))

      end

c-----------------------------------------------------------------------
      subroutine EposInput
c-----------------------------------------------------------------------
#include "epos.inc"
      common/cxsubro/isubin                 !also in conex.inc
      nopen=0
#ifdef __CXSUB__
      ifop=isubin
#else
      ifop=5
#endif
      call aread
      end

c-----------------------------------------------------------------------
      subroutine emsepo(iret)
c-----------------------------------------------------------------------
c  call epos to simulate interaction
c-----------------------------------------------------------------------
c Conex common
      implicit double precision (a-h,o-z)
#include "conex.h"
#include "conex.incnex"

c epos common
      parameter (mxptl=200000)   !max nr of particles in epos particle list
      common/col3/ncol,kolpt
      common/prnt1/iprmpt,ish,ishsub,irandm,irewch,iecho,modsho,idensi
      common/cptl/nptl,pptl(5,mxptl),iorptl(mxptl),idptl(mxptl)
     *,istptl(mxptl),tivptl(2,mxptl),ifrptl(2,mxptl),jorptl(mxptl)
     *,xorptl(4,mxptl),ibptl(4,mxptl),ityptl(mxptl)
      real pptl,xorptl,tivptl,gaumx,wtmini,wtstep
      common/wana2/isphis,ispall,wtmini,wtstep,iwcent,iana,nbdky
      common/othe1/istore,istmax,gaumx,irescl,ntrymx,nclean,iopdg,ioidch
      integer        iLHC,ipytune
      common/LHCtune/iLHC,ipytune
      integer       iorsce,iorsdf,iorshh,ionudi
      common/cjinti/iorsce,iorsdf,iorshh,ionudi


c      double precision pfrx(mamxs),pfry(mamxs)
c      integer          ityp(mamxs)

      nptlxs=0
      nevtxs=1
      iret=0
c      irest = maprojxs*100+abs(laprojxs)
c      inew=0

      call utpri('emsepo',ish,ishini,4)

      call emsaaa(iret)

      if(iret.ne.0)goto 1001

      ncolxs=ncol

      if(ish.ge.2)call alist('list before fragmentation&',1,nptl)

      iclu=0
      if(iLHC.eq.1.and.iorsdf.eq.3)iclu=1 !in case of fusion, don't use Z first time
      call gakfra(iclu,iret)
      if(iret.gt.0)goto 1001
      if(ish.ge.4)
     &     call alist('list after fragmentation&',1,nptl)
      if(irescl.eq.1)then
        call utghost(iret)
        if(iret.gt.0)goto 1001
      endif

      nbdky=nptl
      call bjinta(ier)
      if(ier.eq.1)goto 1001
      if(ish.ge.4)
     &       call alist('list after int/decays&',1,nptl)
      if(irescl.eq.1)then
        call utresc(iret)
        if(ish.ge.4)call alist('list after rescaling&',1,nptl)
        if(iret.gt.0)goto 1001
      endif

      if(ifragm.gt.0.and.(maprojxs.gt.1.or.matargxs.gt.1))then
        call emsfrag(iret)
        if(iret.gt.0)goto 1001
      endif

      if(ish.ge.2)call alistf('EPOS&')

      do is=1,nptl

        if(istptl(is).eq.0)then
          nptlxs=nptlxs+1
          if(nptlxs.gt.mxptlxs)stop'Epos: mxptlxs too small'
          id=idptl(is)
          if(id.gt.10000)id=mod(id,10000)/10*100           !proj A

          call cxidmass(id,am)

          istptlxs(nptlxs)=istptl(is)
c          if ( is .le. maprojxs  .and.
c     *          iorptl(is) .eq. 0  .and.  istptl(is) .eq. 0 ) then
c            if ( ifragm .ne. 0 ) then
cc  compose projectile spectators to remaining nucleus
c              istptlxs(nptlxs)=1
c              idrest = nptlxs
c              if     ( id .eq. 1120 ) then
c                inew  = inew + 101
c                irest = irest - 101
c              elseif ( id .eq. 1220 ) then
c                inew  = inew + 100
c                irest = irest - 100
c              endif
c            endif
c          elseif( is .le. maprojxs  .and.
c     *          iorptl(is) .eq. 0  .and.  istptl(is) .eq. 1 ) then
c              jorptl(is)=1
c          endif
          xsptl(1,nptlxs)=dble(pptl(1,is))
          xsptl(2,nptlxs)=dble(pptl(2,is))
          xsptl(3,nptlxs)=dble(pptl(3,is))
          xsptl(4,nptlxs)=dble(pptl(4,is))
          xsptl(5,nptlxs)=am    !mass
          ityptlxs(nptlxs)=ityptl(is)
          iorptlxs(nptlxs)=iorptl(is)
          jorptlxs(nptlxs)=jorptl(is)
          ifrptlxs(1,nptlxs)=ifrptl(1,is)
          ifrptlxs(2,nptlxs)=ifrptl(2,is)
          xsorptl(1,nptlxs)=dble(xorptl(1,is))
          xsorptl(2,nptlxs)=dble(xorptl(2,is))
          xsorptl(3,nptlxs)=dble(xorptl(3,is))
          xsorptl(4,nptlxs)=dble(xorptl(4,is))
          xstivptl(1,nptlxs)=dble(tivptl(1,is))
          xstivptl(2,nptlxs)=dble(tivptl(2,is))
          idptlxs(nptlxs)=id


#ifdef __CXDEBUG__
          if(isx.ge.5)write(ifck,'(a,i5,a,i5,a,4(e11.4,1x),f6.3)')
     $       ' particle from EPOS ',nptlxs,' id :',idptlxs(nptlxs)
     $  , ' momentum :',(xsptl(k,nptlxs),k=1,5)
#endif

        endif


      enddo
c      if(inew.eq.0)goto 1000
c
c      nptlxsa=nptlxs
c
c      if( inew .eq. 100 .or. inew .eq. 101 ) then
cc  remaining nucleus is single neutron or proton
c        istptlxs(idrest)=0
c        goto 1000
c
c      elseif ( ifragm .ge. 2 ) then
cc  remaining nucleus is evaporating nucleons and alpha particles
c        jfin  = 0
c        call cxvapor( maprojxs,inew,jfin,ityp,pfrx,pfry )
c        if ( jfin .eq. 0 ) goto 1000
cc loop to treat the remanents of the desintegrated fragment
c        do  135  j = 1, jfin
c          if(mod(ityp(j),100).eq.0)then
c            inucl= ityp(j)/100
c            idnucl=100*inucl
c          else
c            inucl=1
c            idnucl=ityp(j)
c          endif
c          ea = dble(inucl) * dble(pptl(4,idrest))
c#ifdef __CXDEBUG__
c          if (isx.ge.6) write(ifck,*) 'fragment: j,id,ea=',
c     *                                       j,idnucl,ea
c#endif
cc  momenta squared
c          call cxidmass(idnucl,am)
c          ptm = ( ea - dble(am) ) * ( ea + dble(am) )
c          pt2 = pfrx(j)**2 + pfry(j)**2
c          if ( pt2 .ge. ptm ) then
c#ifdef __CXDEBUG__
c            if (isx.ge.2) write(ifck,*) 'emsepo: pt reject particle',j
c#endif
c            pfrx(j)=0.d0
c            pfry(j)=0.d0
c          endif
c          plong=ptm-pt2
c          if ( plong .ge. 0.d0)then
c            plong  = sqrt( plong )
c          else
c#ifdef __CXDEBUG__
c            if (isx.ge.2) write(ifck,*) 'emsepo: ptm reject particle',j
c#endif
c            do is=1,inucl
c              if(istptlxs(is).eq.1.and.jorptlxs(is).eq.0)istptlxs(is)=0
c            enddo
c            goto 135
c          endif
c          nptlxs=nptlxs+1
c          istptlxs(nptlxs)=0
c          xsptl(1,nptlxs)=pfrx(j)
c          xsptl(2,nptlxs)=pfry(j)
c          xsptl(3,nptlxs)=plong
c          xsptl(4,nptlxs)=ea
c          xsptl(5,nptlxs)=am    !mass
c          ityptlxs(nptlxs)=0
c          iorptlxs(nptlxs)=1
c          jorptlxs(nptlxs)=maprojxs
c          ifrptlxs(1,nptlxs)=0
c          ifrptlxs(2,nptlxs)=0
c          xsorptl(1,nptlxs)=xsorptl(1,idrest)
c          xsorptl(2,nptlxs)=xsorptl(2,idrest)
c          xsorptl(3,nptlxs)=xsorptl(3,idrest)
c          xsorptl(4,nptlxs)=xsorptl(4,idrest)
c          xstivptl(1,nptlxs)=xstivptl(1,idrest)
c          xstivptl(2,nptlxs)=xstivptl(2,idrest)
c          idptlxs(nptlxs)=idnucl
c 135    continue
c
c        elseif ( ifragm .eq. 1 ) then
cc  remaining nucleus is one fragment
c          nptlxs=nptlxs+1
c          istptlxs(nptlxs)=0
c          xsptl(1,nptlxs)=0.d0
c          xsptl(2,nptlxs)=0.d0
c          xsptl(4,nptlxs)=0.d0
c          inucl=0
c          do is=1,maprojxs
c            if(iorptl(is).eq.0.and.jorptl(is).eq.0)then
c              inucl=inucl+1
c              xsptl(4,nptlxs)=xsptl(4,nptlxs)+dble(pptl(4,is))
c            endif
c          enddo
c          idnucl=100*inucl
c          call cxidmass(idnucl,am)
c          xsptl(5,nptlxs)=am    !mass
c          ptot=(xsptl(4,nptlxs)+am)*(xsptl(4,nptlxs)-am)
c          xsptl(3,nptlxs)=sqrt(ptot)
c          ityptlxs(nptlxs)=0
c          istptlxs(nptlxs)=0
c          iorptlxs(nptlxs)=1
c          jorptlxs(nptlxs)=maprojxs
c          ifrptlxs(1,nptlxs)=0
c          ifrptlxs(2,nptlxs)=0
c          xsorptl(1,nptlxs)=xsorptl(1,idrest)
c          xsorptl(2,nptlxs)=xsorptl(2,idrest)
c          xsorptl(3,nptlxs)=xsorptl(3,idrest)
c          xsorptl(4,nptlxs)=xsorptl(4,idrest)
c          xstivptl(1,nptlxs)=xstivptl(1,idrest)
c          xstivptl(2,nptlxs)=xstivptl(2,idrest)
c          idptlxs(nptlxs)=idnucl
c        endif
c
c#ifdef __CXDEBUG__
c      do is=nptlxsa+1,nptlxs
c          if(isx.ge.5)write(ifck,'(a,i5,a,i5,a,4(e11.4,1x),f6.3)')
c     $       ' particle from EPOS ',is,' id :',idptlxs(is)
c     $  , ' momentum :',(xsptl(k,is),k=1,5)
c      enddo
c#endif


 1000 call utprix('emsepo',ish,ishini,4)
      return


1001  iret=1
      goto 1000

      end

#if !__CXCORSIKA__ && !__CORSIKA8__

c-----------------------------------------------------------------------
      subroutine ranfini(seed,iseq,iqq)
c-----------------------------------------------------------------------
      double precision    seed,dummy
      integer iseq,iqq,idum
      dummy=seed
      idum=iqq
      idum=iseq
      return
      end

c-----------------------------------------------------------------------
      subroutine ranfcv(seed)
c-----------------------------------------------------------------------
c Convert input seed to EPOS random number seed
c Useless with ranf
c-----------------------------------------------------------------------
      double precision seed,dummy
      dummy=seed

      return
      end
#endif
#endif
