      PROGRAM main
      COMMON
     *      /DATABA_SIZVEC/ NUMsphe, NUModd,  NUMdefo,
     *                      NUMd3n,  NUMd3p,  NUMsupd,
     *                      NUMmono, NUMdipo, NUMtwop,
     *                      NUMdvpn, NUMterm, NUMqpSH
C
      CALL InitializeData()
C
      CALL GetData()
C
      CALL PrintData()
C
      STOP
C
      END
C
      INCLUDE 'input04.f'


