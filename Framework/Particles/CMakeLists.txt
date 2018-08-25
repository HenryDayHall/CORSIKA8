
add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/Framework/Particles/GeneratedParticleProperties.inc
                   COMMAND ${PROJECT_SOURCE_DIR}/Framework/Particles/pdxml_reader.py ${PROJECT_SOURCE_DIR}/Framework/Particles/ParticleData.xml ${PROJECT_SOURCE_DIR}/Framework/Particles/ParticleClassNames.xml
                   DEPENDS pdxml_reader.py ParticleData.xml ParticleClassNames.xml
                   WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/Framework/Particles/
                   COMMENT "Read PYTHIA8 particle data and produce C++ source code GeneratedParticleProperties.inc"
                   VERBATIM)


#set (PARTICLES_SOURCES )
set (PARTICLE_HEADERS Particles.h ${PROJECT_BINARY_DIR}/Framework/Particles/GeneratedParticleProperties.inc)

add_library (CORSIKAparticles INTERFACE)

#set_target_properties (CORSIKAparticles PROPERTIES VERSION ${PROJECT_VERSION})
#set_target_properties (CORSIKAparticles PROPERTIES SOVERSION 1)

#set_target_properties (CORSIKAparticles PROPERTIES PUBLIC_HEADER "${PARTICLES_HEADERS}")
                                                    
# target dependencies on other libraries (also header only)
#target_link_libraries (CORSIKAparticles CORSIKAunits)

#target_include_directories (CORSIKAparticles PRIVATE   ${EIGEN3_INCLUDE_DIR})
target_include_directories (CORSIKAparticles INTERFACE ${EIGEN3_INCLUDE_DIR})
target_include_directories (CORSIKAparticles INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/Framework>
                                                       $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/Framework>
                                                       $<INSTALL_INTERFACE:include/Framework>
                                                      )

#install (TARGETS CORSIKAparticles
#         LIBRARY DESTINATION lib
#         ARCHIVE DESTINATION lib
#         PUBLIC_HEADER DESTINATION include/Particles)

install (FILES ${PARTICLE_HEADERS} DESTINATION include/Particles)

# code testing
add_executable (testParticles testParticles.cc ${PROJECT_BINARY_DIR}/Framework/Particles/GeneratedParticleProperties.inc)
target_link_libraries (testParticles CORSIKAparticles CORSIKAunits CORSIKAthirdparty) # for catch2
add_test (NAME testParticles COMMAND testParticles)

