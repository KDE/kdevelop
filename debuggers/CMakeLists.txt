include_directories(common)

if(NOT WIN32)
    add_subdirectory(common)
    add_subdirectory(gdb)
endif()

