add_custom_command (
  OUTPUT  ${PROJECT_BINARY_DIR}/GeneratedParticleProperties.inc
          ${PROJECT_BINARY_DIR}/particle_db.pkl
  COMMAND ${PROJECT_SOURCE_DIR}/Framework/Particles/pdxml_reader.py 
          ${PROJECT_SOURCE_DIR}/Framework/Particles/ParticleData.xml
          ${PROJECT_SOURCE_DIR}/Framework/Particles/NuclearData.xml
          ${PROJECT_SOURCE_DIR}/Framework/Particles/ParticleClassNames.xml
  DEPENDS pdxml_reader.py
          ParticleData.xml
          NuclearData.xml
          ParticleClassNames.xml
  WORKING_DIRECTORY
          ${PROJECT_BINARY_DIR}/
  COMMENT "Read PYTHIA8 particle data and produce C++ source code GeneratedParticleProperties.inc"
  VERBATIM
  )

add_dependencies (CORSIKA8 ${PROJECT_BINARY_DIR}/Framework/Particles/GeneratedParticleProperties.inc)
