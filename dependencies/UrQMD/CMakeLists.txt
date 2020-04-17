cmake_minimum_required(VERSION 3.1)

project(libUrQMD)

set (
  MODEL_SOURCES
  urqmdInterface.F
  addpart.f
  angdis.f
  anndec.f
  blockres.f
  boxprg.f
  cascinit.f
  coload.f
  dectim.f
  delpart.f
  detbal.f
  dwidth.f
  error.f
  getmass.f
  getspin.f
  init.f
  iso.f
  ityp2pdg.f
  jdecay2.f
  make22.f
  numrec.f
  output.f
  paulibl.f
  proppot.f
  saveinfo.f
  scatter.f
  siglookup.f
  string.f
  tabinit.f
  urqmd.f
  whichres.f
)

set (
  MODEL_HEADERS
  )

enable_language(Fortran)
add_library (UrQMD SHARED ${MODEL_SOURCES})
set_property(TARGET UrQMD PROPERTY POSITION_INDEPENDENT_CODE 1)

add_library (UrQMD_static STATIC ${MODEL_SOURCES})
set_property(TARGET UrQMD_static PROPERTY POSITION_INDEPENDENT_CODE 1)


