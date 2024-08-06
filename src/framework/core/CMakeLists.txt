set (input_dir ${PROJECT_SOURCE_DIR}/src/framework/core)
set (output_dir ${PROJECT_BINARY_DIR}/corsika/framework/core)
set (destination_dir ${PROJECT_SOURCE_DIR}/corsika/framework/core )

file (MAKE_DIRECTORY ${output_dir})

add_custom_command (
  OUTPUT  ${output_dir}/GeneratedParticleProperties.inc
          ${output_dir}/GeneratedParticleClasses.inc
          ${output_dir}/particle_db.pkl
  COMMAND ${input_dir}/code_generator.py ${input_dir}/ParticleData.xml
                                         ${input_dir}/NuclearData.xml
                                         ${input_dir}/ParticleClassNames.xml
  DEPENDS ${input_dir}/code_generator.py
          ${input_dir}/ParticleData.xml
          ${input_dir}/NuclearData.xml
          ${input_dir}/ParticleClassNames.xml
  WORKING_DIRECTORY
          ${output_dir}
  COMMENT "Read PYTHIA8 particle data and produce C++ source code GeneratedParticle[...].inc"
  VERBATIM
  )

set_source_files_properties (
  ${output_dir}/GeneratedParticleProperties.inc
  ${output_dir}/GeneratedParticleClasses.inc
  ${output_dir}/particle_db.pkl
  PROPERTIES GENERATED TRUE
  )

add_custom_command(
  OUTPUT  ${destination_dir}/GeneratedParticleProperties.inc
          ${destination_dir}/GeneratedParticleClasses.inc
  COMMAND ${CMAKE_COMMAND} -E copy ${output_dir}/GeneratedParticleProperties.inc ${destination_dir}/GeneratedParticleProperties.inc 
  COMMAND ${CMAKE_COMMAND} -E copy ${output_dir}/GeneratedParticleClasses.inc ${destination_dir}/GeneratedParticleClasses.inc   
  DEPENDS ${output_dir}/GeneratedParticleProperties.inc
          ${output_dir}/GeneratedParticleClasses.inc
          ${output_dir}/particle_db.pkl
)

add_custom_target (GenParticlesHeaders
  DEPENDS ${destination_dir}/GeneratedParticleProperties.inc
          ${destination_dir}/GeneratedParticleClasses.inc 
  )
add_dependencies (CORSIKA8 GenParticlesHeaders)
