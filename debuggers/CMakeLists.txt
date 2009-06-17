if(NOT WIN32 AND NOT APPLE )
  add_subdirectory(gdb)
  add_subdirectory(valgrind)
endif(NOT WIN32 AND NOT APPLE )
