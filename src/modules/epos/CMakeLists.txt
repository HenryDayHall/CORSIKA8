set (input_dir ${PROJECT_SOURCE_DIR}/src/modules/epos)
set (output_dir ${PROJECT_BINARY_DIR}/corsika/modules/epos)
set (destination_dir ${PROJECT_SOURCE_DIR}/corsika/modules/epos )

file (MAKE_DIRECTORY ${output_dir})

add_custom_command (
  OUTPUT  ${output_dir}/Generated.inc
  COMMAND ${input_dir}/code_generator.py 
          ${PROJECT_BINARY_DIR}/corsika/framework/core/particle_db.pkl
          ${input_dir}/epos_codes.dat
  DEPENDS ${input_dir}/code_generator.py
          ${input_dir}/epos_codes.dat
          GenParticlesHeaders # for particle_db.pkl
  WORKING_DIRECTORY
          ${output_dir}/
  COMMENT "Generate conversion tables for particle codes EPOS <-> CORSIKA"
  VERBATIM
  )
 
 add_custom_command(
  OUTPUT  ${destination_dir}/Generated.inc
  COMMAND ${CMAKE_COMMAND} -E copy ${output_dir}/Generated.inc ${destination_dir}/Generated.inc 
  DEPENDS ${output_dir}/Generated.inc
)

add_custom_target (SourceDirLinkEpos DEPENDS ${destination_dir}/Generated.inc)
add_dependencies (CORSIKA8 SourceDirLinkEpos)
