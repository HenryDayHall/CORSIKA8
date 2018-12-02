
add_subdirectory (NullModel)
add_subdirectory (Sibyll)
add_subdirectory (StackInspector)
add_subdirectory (TrackingLine)

#add_custom_target(CORSIKAprocesses)
add_library (CORSIKAprocesses INTERFACE)
add_dependencies(CORSIKAprocesses ProcessNullModel)
add_dependencies(CORSIKAprocesses ProcessSibyll)
add_dependencies(CORSIKAprocesses ProcessStackInspector)
add_dependencies(CORSIKAprocesses ProcessTrackingLine)
