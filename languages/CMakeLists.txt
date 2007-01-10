
#add_subdirectory(fake)
add_subdirectory(cpp)
add_subdirectory(csharp)
add_subdirectory(java)
if (NOT WIN32)
  add_subdirectory(ruby)
endif (NOT WIN32)