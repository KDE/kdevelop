#SUBDIRS( build_unix )

IF (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/build_unix/Makefile)
   EXEC_PROGRAM(chmod ARGS 755 ${CMAKE_CURRENT_SOURCE_DIR}/dist/configure)
   EXEC_PROGRAM(${CMAKE_CURRENT_SOURCE_DIR}/dist/configure ${CMAKE_CURRENT_SOURCE_DIR}/build_unix/ ARGS --prefix=${CMAKE_INSTALL_PREFIX}/kdevbdb)
ENDIF (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/build_unix/Makefile)

ADD_CUSTOM_TARGET(bdb3 ALL COMMAND make -C build_unix)


#original Makefile.am contents follow:

#SUBDIRS=build_unix

#######################
#previous CMakeLists.txt contents follow:

