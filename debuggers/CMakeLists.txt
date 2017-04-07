include_directories(common)

include_directories(${CMAKE_CURRENT_BINARY_DIR}) # for config files in common

if(NOT WIN32)
    # TODO: Make compile under Windows
    add_subdirectory(common)
    add_subdirectory(lldb)
    add_subdirectory(gdb)
endif()
