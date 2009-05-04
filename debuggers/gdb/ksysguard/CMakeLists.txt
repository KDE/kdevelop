
add_subdirectory( processcore )
add_subdirectory( processui )

check_include_files(sys/ptrace.h HAVE_SYS_PTRACE_H)

configure_file(config-ksysguard.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ksysguard.h )


