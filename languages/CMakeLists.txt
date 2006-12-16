
#add_subdirectory(fake)
add_subdirectory(cpp)
if (CP_EXECUTABLE)
  add_subdirectory(csharp)
endif (CP_EXECUTABLE)
add_subdirectory(java)
add_subdirectory(ruby)
