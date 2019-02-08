C
C      The subroutine reads the database, allocates a number of arrays
C      that are passed through the interface of this module, and fills
C      out these arrays
C
C      Version 01: - Reads ATOMIC masses from Audi Wapstra.
C                  - The electronic binding energy is removed in this
C                    routine but is present in the data contained in the
C                    file DataSet01.dat.
C
C      Version 02: - Reads NUCLEAR masses from a table that is a mix
C                    of Audi-Wapstra 2003 and new Jyvaskyla masses.
C                    The mass of a given nuclide is taken as the
C                    weighted average of the original Audi-Wapstra
C                    evaluation and the Jyvaskyla mass, see
C
C                   J.R. Taylor, An Introduction to Error Analysis
C                   2nd Ed., University Science Books, 1997
C
C                  - Electronic correction has been removed from the
C                    data contained in table DataSet02.dat. Also a bug
C                    relative to deltaVpn data has been fixed.
C
C      Version 03: - Adds proton radius for spherical nuclei.
C                  - Adds experimental error in binding energy of
C                    spherical nuclei
C                  - Adds flags for deformed nuclei, delta^(3)_n and
C                    delta^(3)_p, and odd g.s.,which indicate if the
C                    corresponding masses have been measured (1) or
C                    evaluated (0).
C
C      Version 04: - Adds rough estimate of axial deformation of SD states
C                  - Adds one nucleus in the list of SD states
C
C                        ------------------
C
C      Spherical nuclei:
C        - IZsphe, INsphe: proton number Z and neutron number N
C        - Bsphe: experimental binding energy
C        - dBsphe: experimental error in binding energy
C        - R0sphe: experimental diffraction radius
C        - SIGsphe: experimental surface thickness
C        - RMSspheCharge: experimental r.m.s charge radius
C        - RMSspheProton: r.m.s proton radius computed from the charge radius
C
C      Deformed nuclei:
C        - IZdefo, INdefo: proton number Z and neutron number N
C        - Bdefo: experimental binding energy
C        - dBdefo: experimental error in binding energy
C        - b2defo: beta_2 value of g.s. quadrupole deformation (SLY4 calculation)
C        - IsOKdefo: status of binding energy: 1 = measured, 0 = evaluated
C
C      Odd-even mass differences:
C        - IZd3n, INd3n: proton number and neutron number related to the
C                        neutron odd-even mass difference
C        - DELd3n, ERRd3n: delta3 (neutrons) and relative error
C        - IsOKd3n: status of binding energy: 1 = measured, 0 = evaluated
C        - IZd3p, INd3p: proton number and neutron number related to the
C                        proton odd-even mass difference
C        - DELd3p, ERRd3p: delta3 (protons) and relative error
C        - IsOKd3p: status of binding energy: 1 = measured, 0 = evaluated
C
C      Super-deformed states and fission isomers:
C        - IZsupd, INsupd: proton number Z and neutron number N
C        - Bsupd: experimental binding energy
C        - ESDsupd: energy of the SD bandhead or fission isomer
C        - b2supd: rough estimate of the beta_2 value of the SD state
C
C      Giant monopole resonance
C        - IZmono, INmono: proton number Z and neutron number N
C        - Emono: experimental energy
c
C      Giant dipole resonance
C        - IZdipo, INdipo: proton number Z and neutron number N
C        - Emdipo: experimental energy
C
C      Odd-mass nuclei:
C        - IZodd, INodd: proton number Z and neutron number N
C        - SPINodd: experimental g.s. spin
C        - IPodd: experimental g.s. parity
C        - IsOKodd: status of binding energy: 1 = measured, 0 = evaluated
C
C      One quasi-particle state in Odd-mass superheavy nuclei:
C        - IZqpSH, INqpSH: proton number Z and neutron number N
C        - NQPqpSH: number of q.p. states
C        - EqpSH: experimental excitation energy
C        - LABqpSH: experimental Nilsson label
C        - SPINqpSH: experimental spin
C        - IPqpSH: experimental parity
C
C      Position of the first 2+ state
C        - IZtwop, INtwop: proton number Z and neutron number N
C        - Etwop: experimental energy of the 2+ state
C        - dEtwop: error bar on the energy
C        - BE2twop: experimental BE2
C        - dBE2twop: error bar on the BE2
C
C      Delta Vpn
C        - IZdvpn, INdvpn: proton number Z and neutron number N
C        - ExcMASdvpn: Mass excess
C        - ExcERRdvpn: Error on mass excess
C        - BnucMASdvpn: binding energy per nucleon B/A
C        - BnucERRdvpn: error (in %) on B/A
C        - DelVPNdvpn: delta Vpn
C        - DelERRdvpn: error on delta Vpn
C
C      Terminating states:
C        - IZterm, INterm: proton number Z and neutron number N
C        - SPINterm, IP1term: spin Imax and parity for the f7/2 state
C        - Eterm: energy of the f7/2 state
C        - SpindETerm, IP2term: spin Imax and parity for the
C                               d3/2^(-1)*f7/2 state
C        - dEterm: experimental energy difference between the two
C                  configurations
C
C      IMPORTANT REMARK
C
C          Experimental binding energies were extracted from Audi-Wapstra
C          mass tables. BY DEFINITION, THEY DO INCLUDE A CONTRIBUTION FROM
C          THE BINDING ENERGY OF THE ELECTRONS. To cancel out this effect
C          and obtain the true NUCLEAR binding energy, a correction is added
C          systematically, which goes (in MeV) as
C
C                    CorrELEC * Z^(2.39), CorrELEC = 1.433.10^(-5)
C
C
      SUBROUTINE GetData()
      PARAMETER
     *         (NDSPHE=80,NDDEFO=250,NDSUPD=20,NDMONO=3,NDDIPO=3)
      PARAMETER
     *         (NDTWOP=600,NDTERM=20,ND_ODD=650,NDDVPN=150)
      PARAMETER
     *         (NDQPSH=5,NDSHEL=5,ND_D3N=250,ND_D3P=250)
      CHARACTER
     *          Keyword*8,LABqpSH*5
C
      COMMON
     *      /NUCLEI_SPHERI/ IZsphe(1:NDSPHE),INsphe(1:NDSPHE),
     *                                       IsOKsphe(1:NDSPHE)
     *      /NUCLEI_DEFORM/ IZdefo(1:NDDEFO),INdefo(1:NDDEFO),
     *                                       IsOKdefo(1:NDDEFO)
     *      /NUCLEI_DELT3N/ IZd3n (1:ND_D3N),INd3n (1:ND_D3N),
     *                                       IsOKd3n(1:ND_D3N)
     *      /NUCLEI_DELT3P/ IZd3p (1:ND_D3P),INd3p (1:ND_D3P),
     *                                       IsOKd3p(1:ND_D3P)
     *      /NUCLEI_SDSTAT/ IZsupd(1:NDSUPD),INsupd(1:NDSUPD)
     *      /NUCLEI_MONOPO/ IZmono(1:NDMONO),INmono(1:NDMONO)
     *      /NUCLEI_DIPOLE/ IZdipo(1:NDDIPO),INdipo(1:NDDIPO)
     *      /NUCLEI_ODDNUC/ IZodd (1:ND_ODD),INodd (1:ND_ODD),
     *                                       IsOKodd(1:ND_ODD)
     *      /NUCLEI_SHELEM/ IZqpSH(1:NDSHEL),INqpSH(1:NDSHEL)
     *      /NUCLEI_TWOPLU/ IZtwop(1:NDTWOP),INtwop(1:NDTWOP)
     *      /NUCLEI_TERMIN/ IZterm(1:NDTERM),INterm(1:NDTERM)
     *      /NUCLEI_DELTPN/ IZdvpn(1:NDDVPN),INdvpn(1:NDDVPN)
      COMMON
     *      /DATABA_SIZVEC/ NUMsphe, NUModd,  NUMdefo,
     *                      NUMd3n,  NUMd3p,  NUMsupd,
     *                      NUMmono, NUMdipo, NUMtwop,
     *                      NUMdvpn, NUMterm, NUMqpSH
      COMMON
     *      /DATABA_SPHERI/ Bsphe(1:NDSPHE),dBsphe(1:NDSPHE),
     *                      R0sphe(1:NDSPHE),SIGsphe(1:NDSPHE),
     *                      RMSspheCharge(1:NDSPHE),
     *                      RMSspheProton(1:NDSPHE)
      COMMON
     *      /DATABA_DEFORM/ Bdefo(1:NDDEFO),dBdefo(1:NDDEFO),
     *                      b2defo(1:NDDEFO)
      COMMON
     *      /DATABA_DELT3N/ DELd3n(1:ND_D3N),ERRd3n(1:ND_D3N)
     *      /DATABA_DELT3P/ DELd3p(1:ND_D3P),ERRd3p(1:ND_D3P)
      COMMON
     *      /DATABA_SDSTAT/ Bsupd(1:NDSUPD),ESDsupd(1:NDSUPD),
     *                                      b2supd(1:NDSUPD)
      COMMON
     *      /DATABA_MONOPO/ Emono(1:NDMONO),dEmono(1:NDMONO)
      COMMON
     *      /DATABA_DIPOLE/ Edipo(1:NDDIPO)
      COMMON
     *      /DATABA_ODDSPI/ SPINodd(1:ND_ODD)
     *      /DATABA_ODDPAR/ IPodd(1:ND_ODD)
      COMMON
     *      /DATABA_QPNUMB/ NQPqpSH(1:NDSHEL)
     *      /DATABA_QPEEXC/ EqpSH(1:NDSHEL,1:NDQPSH)
     *      /DATABA_QPSPIN/ SPINqpSH(1:NDSHEL,1:NDQPSH)
     *      /DATABA_QPPARI/ IPqpSH(1:NDSHEL,1:NDQPSH)
     *      /DATABA_QPLABL/ LABqpSH(1:NDSHEL,1:NDQPSH)
      COMMON
     *      /DATABA_TWOPLU/ Etwop(1:NDTWOP),dEtwop(1:NDTWOP),
     *                      BE2twop(1:NDTWOP),dBE2twop(1:NDTWOP)
      COMMON
     *      /DATABA_DELTPN/ ExcMASdvpn(1:NDDVPN),
     *                      ExcERRdvpn(1:NDDVPN),
     *                      BnucMASdvpn(1:NDDVPN),
     *                      BnucERRdvpn(1:NDDVPN),
     *                      DelVPNdvpn(1:NDDVPN),
     *                      DelERRdvpn(1:NDDVPN)
      COMMON
     *      /DATABA_TERMIN/ SPINterm(1:NDTERM),Eterm(1:NDTERM),
     *                      SPINdEterm(1:NDTERM),dEterm(1:NDTERM)
     *      /PARITY_TERMIN/ IP1Term(1:NDTERM),IP2term(1:NDTERM)
C
      n_unit = 15
C
      OPEN (n_unit, FILE='DataSet04.dat', ACTION='READ', IOSTAT=ierror)
      IF (ierror .NE. 0) STOP 'Impossible to read: ./DataSet04.dat'
C
      Keyword = 'XXXXXXXX'
C
C      Scanning of the database proceeds by keywords
C
 1    CONTINUE
C
      READ (n_unit,*) Keyword
C
      IF (Keyword .EQ. 'END_DATA') THEN
          CLOSE (n_unit)
          RETURN
      END IF
C
      IF (Keyword .EQ. 'SPHERICA') THEN
C
          READ (n_unit,*) NUMsphe
C
          DO i=1, NUMsphe
              READ (n_unit,*) IZsphe(i),INsphe(i),Bsphe(i),
     *                        dBsphe(i),R0sphe(i),SIGsphe(i),
     *                        RMSspheCharge(i),RMSspheProton(i)
          END DO
C
      END IF
C
      IF (Keyword .EQ. 'DEFORMED') THEN
C
          READ (n_unit,*) NUMdefo
C
          DO i=1, NUMdefo
              READ (n_unit,*) IZdefo(i),INdefo(i),Bdefo(i),
     *                        dBdefo(i),b2defo(i),IsOKdefo(i)
          END DO
C
      END IF
C
      IF (Keyword .EQ. 'DELTA3_N') THEN
C
          READ (n_unit,*) NUMd3n
C
          DO i=1, NUMd3n
              READ (n_unit,*) IZd3n(i),INd3n(i),DELd3n(i),ERRd3n(i),
     *                        IsOKd3n(i)
          END DO
C
      END IF
C
      IF (Keyword .EQ. 'DELTA3_P') THEN
C
          READ (n_unit,*) NUMd3p
C
          DO i=1, NUMd3p
              READ (n_unit,*) INd3p(i),IZd3p(i),DELd3p(i),ERRd3p(i),
     *                        IsOKd3p(i)
          END DO
C
      END IF
C
      IF (Keyword .EQ. 'SDSTATES') THEN
C
          READ (n_unit,*) NUMsupd
C
          DO i=1, NUMsupd
              READ (n_unit,*) IZsupd(i),INsupd(i),Bsupd(i),ESDsupd(i),
     *                                                     b2supd(i)
          END DO
C
      END IF
C
      IF (Keyword .EQ. 'MONOPRES') THEN
C
          READ (n_unit,*) NUMmono
C
          DO i=1, NUMmono
              READ (n_unit,*) IZmono(i),INmono(i),Emono(i),dEmono(i)
          END DO
C
      END IF
C
      IF (Keyword .EQ. 'DIPOLRES') THEN
C
          READ (n_unit,*) NUMdipo
C
          DO i=1, NUMdipo
              READ (n_unit,*) IZdipo(i),INdipo(i),Edipo(i)
          END DO
C
      END IF
C
      IF (Keyword .EQ. 'ODDNUCLE') THEN
C
          READ (n_unit,*) NUModd
C
          DO i=1, NUModd
              READ (n_unit,*) IZodd(i),INodd(i),SPINodd(i),IPodd(i),
     *                        IsOKodd(i)
          END DO
C
      END IF
C
      IF (Keyword .EQ. 'QPSHELEM') THEN
C
          READ (n_unit,*) NUMqpSH
C
          DO i=1, NUMqpSH
              READ (n_unit,*) IZqpSH(i),INqpSH(i),NQPqpSH(i),
     *                        EqpSH(i,1),LABqpSH(i,1),
     *                        SPINqpSH(i,1),IPqpSH(i,1)
              DO j=2,NQPqpSH(i)
                 READ (n_unit,*) EqpSH(i,j),LABqpSH(i,j),
     *                           SPINqpSH(i,j),IPqpSH(i,j)
              END DO
          END DO
C
      END IF
C
      IF (Keyword .EQ. '2+ENERGY') THEN
C
          READ (n_unit,*) NUMtwop
C
          DO i=1, NUMtwop
              READ (n_unit,*) IZtwop(i),INtwop(i),Etwop(i),dEtwop(i),
     *                        BE2twop(i),dBE2twop(i)
          END DO
C
      END IF
C
      IF (Keyword .EQ. 'DELTAVPN') THEN
C
          READ (n_unit,*) NUMdvpn
C
          DO i=1, NUMdvpn
              READ (n_unit,*) IZdvpn(i),INdvpn(i),
     *                        ExcMASdvpn(i), ExcERRdvpn(i),
     *                        BnucMASdvpn(i), BnucERRdvpn(i),
     *                        DelVPNdvpn(i), DelERRdvpn(i)
          END DO
C
      END IF
C
      IF (Keyword .EQ. 'TERMINAT') THEN
C
          READ (n_unit,*) NUMterm
C
          DO i=1, NUMterm
              READ (n_unit,*) IZterm(i),INterm(i),SPINterm(i),
     *                        IP1term(i),Eterm(i),SpindETerm(i),
     *                        IP2term(i),dEterm(i)
          END DO
C
      END IF
C
      GO TO 1
C
      END

C      This subroutine only prints the data so that the user can verify
C      that everything has been read properly.

      SUBROUTINE PrintData()
      PARAMETER
     *         (NDSPHE=80,NDDEFO=250,NDSUPD=20,NDMONO=3,NDDIPO=3)
      PARAMETER
     *         (NDTWOP=600,NDTERM=20,ND_ODD=650,NDDVPN=150)
      PARAMETER
     *         (NDQPSH=5,NDSHEL=5,ND_D3N=250,ND_D3P=250)
      PARAMETER
     *         (NDVALS=4000,NDPROT=140,NDNEUT=200)
      CHARACTER
     *          Keyword*8,LABqpSH*5
C
      COMMON
     *       /WAPDAT/ BINDNG(0:NDPROT,0:NDNEUT),
     *                ERRORB(0:NDPROT,0:NDNEUT),
     *                IVALID(0:NDPROT,0:NDNEUT)
      COMMON
     *       /OEMALL/ OEMVAL(1:NDPROT,1:NDNEUT,0:1)
C
      COMMON
     *      /NUCLEI_SPHERI/ IZsphe(1:NDSPHE),INsphe(1:NDSPHE),
     *                                       IsOKsphe(1:NDSPHE)
     *      /NUCLEI_DEFORM/ IZdefo(1:NDDEFO),INdefo(1:NDDEFO),
     *                                       IsOKdefo(1:NDDEFO)
     *      /NUCLEI_DELT3N/ IZd3n (1:ND_D3N),INd3n (1:ND_D3N),
     *                                       IsOKd3n(1:ND_D3N)
     *      /NUCLEI_DELT3P/ IZd3p (1:ND_D3P),INd3p (1:ND_D3P),
     *                                       IsOKd3p(1:ND_D3P)
     *      /NUCLEI_SDSTAT/ IZsupd(1:NDSUPD),INsupd(1:NDSUPD),
     *                                       b2supd(1:NDSUPD)
     *      /NUCLEI_MONOPO/ IZmono(1:NDMONO),INmono(1:NDMONO)
     *      /NUCLEI_DIPOLE/ IZdipo(1:NDDIPO),INdipo(1:NDDIPO)
     *      /NUCLEI_ODDNUC/ IZodd (1:ND_ODD),INodd (1:ND_ODD),
     *                                       IsOKodd(1:ND_ODD)
     *      /NUCLEI_SHELEM/ IZqpSH(1:NDSHEL),INqpSH(1:NDSHEL)
     *      /NUCLEI_TWOPLU/ IZtwop(1:NDTWOP),INtwop(1:NDTWOP)
     *      /NUCLEI_DELTPN/ IZdvpn(1:NDDVPN),INdvpn(1:NDDVPN)
     *      /NUCLEI_TERMIN/ IZterm(1:NDTERM),INterm(1:NDTERM)
C
      COMMON
     *      /DATABA_SIZVEC/ NUMsphe, NUModd,  NUMdefo,
     *                      NUMd3n,  NUMd3p,  NUMsupd,
     *                      NUMmono, NUMdipo, NUMtwop,
     *                      NUMdvpn, NUMterm, NUMqpSH
      COMMON
     *      /DATABA_SPHERI/ Bsphe(1:NDSPHE),dBsphe(1:NDSPHE),
     *                      R0sphe(1:NDSPHE),SIGsphe(1:NDSPHE),
     *                      RMSspheCharge(1:NDSPHE),
     *                      RMSspheProton(1:NDSPHE)
      COMMON
     *      /DATABA_DEFORM/ Bdefo(1:NDDEFO),dBdefo(1:NDDEFO),
     *                      b2defo(1:NDDEFO)
      COMMON
     *      /DATABA_DELT3N/ DELd3n(1:ND_D3N),ERRd3n(1:ND_D3N)
     *      /DATABA_DELT3P/ DELd3p(1:ND_D3P),ERRd3p(1:ND_D3P)
      COMMON
     *      /DATABA_SDSTAT/ Bsupd(1:NDSUPD),ESDsupd(1:NDSUPD),
     *                                      b2supd(1:NDSUPD)
      COMMON
     *      /DATABA_MONOPO/ Emono(1:NDMONO),dEmono(1:NDMONO)
      COMMON
     *      /DATABA_DIPOLE/ Edipo(1:NDDIPO)
      COMMON
     *      /DATABA_ODDSPI/ SPINodd(1:ND_ODD)
     *      /DATABA_ODDPAR/ IPodd(1:ND_ODD)
      COMMON
     *      /DATABA_QPNUMB/ NQPqpSH(1:NDSHEL)
     *      /DATABA_QPEEXC/ EqpSH(1:NDSHEL,1:NDQPSH)
     *      /DATABA_QPSPIN/ SPINqpSH(1:NDSHEL,1:NDQPSH)
     *      /DATABA_QPPARI/ IPqpSH(1:NDSHEL,1:NDQPSH)
     *      /DATABA_QPLABL/ LABqpSH(1:NDSHEL,1:NDQPSH)
      COMMON
     *      /DATABA_TWOPLU/ Etwop(1:NDTWOP),dEtwop(1:NDTWOP),
     *                      BE2twop(1:NDTWOP),dBE2twop(1:NDTWOP)
      COMMON
     *      /DATABA_DELTPN/ ExcMASdvpn(1:NDDVPN),
     *                      ExcERRdvpn(1:NDDVPN),
     *                      BnucMASdvpn(1:NDDVPN),
     *                      BnucERRdvpn(1:NDDVPN),
     *                      DelVPNdvpn(1:NDDVPN),
     *                      DelERRdvpn(1:NDDVPN)
      COMMON
     *      /DATABA_TERMIN/ SPINterm(1:NDTERM),Eterm(1:NDTERM),
     *                      SPINdEterm(1:NDTERM),dEterm(1:NDTERM)
     *      /PARITY_TERMIN/ IP1Term(1:NDTERM),IP2term(1:NDTERM)
C
      Keyword = 'SPHERICA'
      WRITE (6,'(A8)') Keyword
      WRITE (6,'(9X,I5)') NUMsphe
C
      DO i=1, NUMsphe
         WRITE (6,'(9X,2I5,1X,6F11.5)')
     *                  IZsphe(i),INsphe(i),
     *                  Bsphe(i),dBsphe(i),
     *                  R0sphe(i),SIGsphe(i),
     *                  RMSspheCharge(i),
     *                  RMSspheProton(i)
      END DO
C
      Keyword = 'DEFORMED'
      WRITE (6,'(A8)') Keyword
      WRITE (6,'(10X,I5)') NUMdefo
C
      DO i=1, NUMdefo
         WRITE (6,'(9X,2I5,F12.5,2F11.5,i4)') IZdefo(i),INdefo(i),
     *                                        Bdefo(i),dBdefo(i),
     *                                        b2defo(i),IsOKdefo(i)
      END DO
C
      Keyword = 'DELTA3_N'
      WRITE (6,'(A8)') Keyword
      WRITE (6,'(10X,I5)') NUMd3n
C
      DO i=1, NUMd3n
         WRITE (6,'(9X,2I5,2F12.6,i4)') IZd3n(i),INd3n(i),
     *                                  DELd3n(i),ERRd3n(i),
     *                                  IsOKd3n(i)
      END DO
C
      Keyword = 'DELTA3_P'
      WRITE (6,'(A8)') Keyword
      WRITE (6,'(10X,I5)') NUMd3p
C
      DO i=1, NUMd3p
         WRITE (6,'(9X,2I5,2F12.6,i4)') INd3p(i),IZd3p(i),
     *                                  DELd3p(i),ERRd3p(i),
     *                                  IsOKd3p(i)
      END DO
C
      Keyword = 'SDSTATES'
      WRITE (6,'(A8)') Keyword
      WRITE (6,'(9X,I5)') NUMsupd
C
      DO i=1, NUMsupd
         WRITE (6,'(9X,2I5,F12.5,2F9.3)') IZsupd(i),INsupd(i),
     *                                    Bsupd(i),ESDsupd(i),
     *                                             b2supd(i)
      END DO
C
      Keyword = 'MONOPRES'
      WRITE (6,'(A8)') Keyword
      WRITE (6,'(9X,I5)') NUMmono

      DO i=1, NUMmono
         WRITE (6,'(9X,2I5,1X,F9.2,F10.2)') IZmono(i),INmono(i),
     *                                       Emono(i),dEmono(i)
      END DO
C
      Keyword = 'DIPOLRES'
      WRITE (6,'(A8)') Keyword
      WRITE (6,'(9X,I5)') NUMdipo
C
      DO i=1, NUMdipo
         WRITE (6,'(9X,2I5,1X,F9.2)') IZdipo(i),INdipo(i),Edipo(i)
      END DO
C
      Keyword = 'ODDNUCLE'
      WRITE (6,'(A8)') Keyword
      WRITE (6,'(9X,I5)') NUModd
C
      DO i=1, NUModd
         WRITE (6,'(9X,2I5,F6.1,I5,i4)') IZodd(i),INodd(i),
     *                                   SPINodd(i),IPodd(i),
     *                                   IsOKodd(i)
      END DO
C
      Keyword = 'QPSHELEM'
      WRITE (6,'(A8)') Keyword
      WRITE (6,'(9X,I5)') NUMqpSH
C
      DO i=1, NUMqpSH
         WRITE (6,'(9X,2I5,I4,F10.5,4X,A5,F8.1,5X,I2)')
     *              IZqpSH(i),INqpSH(i),NQPqpSH(i),
     *                        EqpSH(i,1),LABqpSH(i,1),
     *                        SPINqpSH(i,1),IPqpSH(i,1)
         DO j=2,NQPqpSH(i)
            WRITE (6,'(23X,F10.5,4X,A5,F8.1,5X,I2)')
     *              EqpSH(i,j),LABqpSH(i,j),
     *              SPINqpSH(i,j),IPqpSH(i,j)
         END DO
      END DO
C
      Keyword = '2+ENERGY'
      WRITE (6,'(A8)') Keyword
      WRITE (6,'(9X,I5)') NUMtwop
C
      DO i=1, NUMtwop
         WRITE (6,'(9X,2I5,4F11.5)') IZtwop(i),INtwop(i),Etwop(i),
     *                               dEtwop(i),BE2twop(i),dBE2twop(i)
      END DO
C
      Keyword = 'DELTAVPN'
      WRITE (6,'(A8)') Keyword
      WRITE (6,'(9X,I5)') NUMdvpn
C
      DO i=1, NUMdvpn
         WRITE (6,'(9X,2I5,F12.1,F10.1,F15.2,F10.2,F14.4,F12.4)')
     *              IZdvpn(i),INdvpn(i),
     *                        ExcMASdvpn(i), ExcERRdvpn(i),
     *                        BnucMASdvpn(i), BnucERRdvpn(i),
     *                        DelVPNdvpn(i), DelERRdvpn(i)
      END DO
C
      Keyword = 'TERMINAT'
      WRITE (6,'(A8)') Keyword
      WRITE (6,'(9X,I5)') NUMterm
C
      DO i=1, NUMterm
         WRITE (6,'(9X,2I5,F9.1,I10,F12.3,F8.1,I8,F9.3)')
     *              IZterm(i),INterm(i),SPINterm(i),IP1term(i),
     *              Eterm(i),SpindETerm(i),IP2term(i),dEterm(i)
      END DO
C
      RETURN
      END
C
C      This subroutine only initializes all the common blocks
C
      SUBROUTINE InitializeData()
      PARAMETER
     *         (NDSPHE=80,NDDEFO=250,NDSUPD=20,NDMONO=3,NDDIPO=3)
      PARAMETER
     *         (NDTWOP=600,NDTERM=20,ND_ODD=650,NDDVPN=150)
      PARAMETER
     *         (NDQPSH=5,NDSHEL=5,ND_D3N=250,ND_D3P=250)
C
      CHARACTER
     *          LABqpSH*5
C
      COMMON
     *      /NUCLEI_SPHERI/ IZsphe(1:NDSPHE),INsphe(1:NDSPHE),
     *                                       IsOKsphe(1:NDSPHE)
     *      /NUCLEI_DEFORM/ IZdefo(1:NDDEFO),INdefo(1:NDDEFO),
     *                                       IsOKdefo(1:NDDEFO)
     *      /NUCLEI_DELT3N/ IZd3n (1:ND_D3N),INd3n (1:ND_D3N),
     *                                       IsOKd3n(1:ND_D3N)
     *      /NUCLEI_DELT3P/ IZd3p (1:ND_D3P),INd3p (1:ND_D3P),
     *                                       IsOKd3p(1:ND_D3P)
     *      /NUCLEI_SDSTAT/ IZsupd(1:NDSUPD),INsupd(1:NDSUPD)
     *      /NUCLEI_MONOPO/ IZmono(1:NDMONO),INmono(1:NDMONO)
     *      /NUCLEI_DIPOLE/ IZdipo(1:NDDIPO),INdipo(1:NDDIPO)
     *      /NUCLEI_ODDNUC/ IZodd (1:ND_ODD),INodd (1:ND_ODD),
     *                                       IsOKodd(1:ND_ODD)
     *      /NUCLEI_SHELEM/ IZqpSH(1:NDSHEL),INqpSH(1:NDSHEL)
     *      /NUCLEI_TWOPLU/ IZtwop(1:NDTWOP),INtwop(1:NDTWOP)
     *      /NUCLEI_DELTPN/ IZdvpn(1:NDDVPN),INdvpn(1:NDDVPN)
     *      /NUCLEI_TERMIN/ IZterm(1:NDTERM),INterm(1:NDTERM)
C
      COMMON
     *      /DATABA_SIZVEC/ NUMsphe, NUModd,  NUMdefo,
     *                      NUMd3n,  NUMd3p,  NUMsupd,
     *                      NUMmono, NUMdipo, NUMtwop,
     *                      NUMdvpn, NUMterm, NUMqpSH
      COMMON
     *      /DATABA_SPHERI/ Bsphe(1:NDSPHE),dBsphe(1:NDSPHE),
     *                      R0sphe(1:NDSPHE),SIGsphe(1:NDSPHE),
     *                      RMSspheCharge(1:NDSPHE),
     *                      RMSspheProton(1:NDSPHE)
      COMMON
     *      /DATABA_DEFORM/ Bdefo(1:NDDEFO),dBdefo(1:NDDEFO),
     *                      b2defo(1:NDDEFO)
      COMMON
     *      /DATABA_DELT3N/ DELd3n(1:ND_D3N),ERRd3n(1:ND_D3N)
     *      /DATABA_DELT3P/ DELd3p(1:ND_D3P),ERRd3p(1:ND_D3P)
      COMMON
     *      /DATABA_SDSTAT/ Bsupd(1:NDSUPD),ESDsupd(1:NDSUPD),
     *                                      b2supd(1:NDSUPD)
      COMMON
     *      /DATABA_MONOPO/ Emono(1:NDMONO),dEmono(1:NDMONO)
      COMMON
     *      /DATABA_DIPOLE/ Edipo(1:NDDIPO)
      COMMON
     *      /DATABA_ODDSPI/ SPINodd(1:ND_ODD)
     *      /DATABA_ODDPAR/ IPodd(1:ND_ODD)
      COMMON
     *      /DATABA_QPNUMB/ NQPqpSH(1:NDSHEL)
     *      /DATABA_QPEEXC/ EqpSH(1:NDSHEL,1:NDQPSH)
     *      /DATABA_QPSPIN/ SPINqpSH(1:NDSHEL,1:NDQPSH)
     *      /DATABA_QPPARI/ IPqpSH(1:NDSHEL,1:NDQPSH)
     *      /DATABA_QPLABL/ LABqpSH(1:NDSHEL,1:NDQPSH)
      COMMON
     *      /DATABA_TWOPLU/ Etwop(1:NDTWOP),dEtwop(1:NDTWOP),
     *                      BE2twop(1:NDTWOP),dBE2twop(1:NDTWOP)
      COMMON
     *      /DATABA_DELTPN/ ExcMASdvpn(1:NDDVPN),
     *                      ExcERRdvpn(1:NDDVPN),
     *                      BnucMASdvpn(1:NDDVPN),
     *                      BnucERRdvpn(1:NDDVPN),
     *                      DelVPNdvpn(1:NDDVPN),
     *                      DelERRdvpn(1:NDDVPN)
      COMMON
     *      /DATABA_TERMIN/ SPINterm(1:NDTERM),Eterm(1:NDTERM),
     *                      SPINdEterm(1:NDTERM),dEterm(1:NDTERM)
     *      /PARITY_TERMIN/ IP1Term(1:NDTERM),IP2term(1:NDTERM)
C
      NUMsphe = 1
      NUModd = 1
      NUMdefo = 1
      NUMsupd = 1
      NUMmono = 1
      NUMdipo = 1
      NUMtwop = 1
      NUMdvpn = 1
      NUMterm = 1
      NUMqpSH = 1
C
      DO i=1,NDSPHE
         IZsphe(i) = 82
         INsphe(i) = 126
         Bsphe(i) = 0.0
         dBsphe(i) = 0.0
         R0sphe(i) = 0.0
         SIGsphe(i) = 0.0
         RMSspheCharge(i) = 0.0
         RMSspheProton(i) = 0.0
      END DO
C
      DO i=1,NDDEFO
         IZdefo(i) = 82
         INdefo(i) = 126
         Bdefo(i) = 0.0
         dBdefo(i) = 0.0
         b2defo(i) = 0.0
         IsOKdefo(i) = 0
      END DO
C
      DO i=1,ND_D3N
         IZd3n(i) = 82
         INd3n(i) = 126
         DELd3n(i) = 0.0
         ERRd3n(i) = 0.0
         IsOKd3n(i) = 0
      END DO
C
      DO i=1,ND_D3P
         IZd3p(i) = 82
         INd3p(i) = 126
         DELd3p(i) = 0.0
         ERRd3p(i) = 0.0
         IsOKd3p(i) = 0
      END DO
C
      DO i=1,NDSUPD
         IZsupd(i) = 82
         INsupd(i) = 126
         Bsupd(i) = 0.0
         ESDsupd(i) = 0.0
         b2supd(i) = 0.7
      END DO
C
      DO i=1,NDMONO
         IZmono(i) = 82
         INmono(i) = 126
         Emono(i) = 0.0
         dEmono(i) = 0.0
      END DO
C
      DO i=1,NDDIPO
         IZdipo(i) = 82
         INdipo(i) = 126
         Edipo(i) = 0.0
      END DO
C
      DO i=1,ND_ODD
         IZodd(i) = 82
         INodd(i) = 126
         SPINodd(i) = 0.0
         IPodd(i) = 0
         IsOKodd(i) = 0
      END DO
C
      DO i=1,NDSHEL
         IZqpSH(i) = 82
         INqpSH(i) = 126
         NQPqpSH(i) = 1
         DO j=1,NDQPSH
            EqpSH(i,j)=0.0
            SPINqpSH(i,j)=0.5
            IPqpSH(i,j)=+1
            LABqpSH(i,j)='XXXXX'
         END DO
      END DO
C
      DO i=1,NDTWOP
         IZtwop(i) = 82
         INtwop(i) = 126
         Etwop(i) = 0.0
         dEtwop(i) = 0.0
         BE2twop(i) = 0.0
         dBE2twop(i) = 0.0
      END DO
C
      DO i=1,NDDVPN
         IZdvpn(i) = 82
         INdvpn(i) = 126
         ExcMASdvpn(i) = 0.0
         ExcERRdvpn(i) = 0.0
         BnucMASdvpn(i) = 0.0
         BnucERRdvpn(i) = 0.0
         DelVPNdvpn(i) = 0.0
         DelERRdvpn(i) = 0.0
      END DO
C
      DO i=1,NDTERM
         IZterm(i) = 82
         INterm(i) = 126
         SPINterm(i) = 0.0
         Eterm(i) = 0.0
         SPINdEterm(i) = 0.0
         dEterm(i) = 0.0
         IP1term(i) = 0
         IP2term(i) = 0
      END DO
C
      RETURN
      END

