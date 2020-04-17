cmake_minimum_required(VERSION 3.1)

project(libQGSJetII)

set (
  MODEL_SOURCES
  qgsjet-II-04.f
  )

set (
  MODEL_HEADERS
  )


enable_language(Fortran)
add_library (QGSJetII SHARED ${MODEL_SOURCES})
set_property(TARGET QGSJetII PROPERTY POSITION_INDEPENDENT_CODE 1)

add_library (QGSJetII_static STATIC ${MODEL_SOURCES})
set_property(TARGET QGSJetII_static PROPERTY POSITION_INDEPENDENT_CODE 1)

