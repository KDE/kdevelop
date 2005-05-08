#this file contains the following macros:
# ADD_FILE_DEPENDANCY
# KDE_ADD_DCOP_SKELS
# KDE_ADD_MOC_FILES
# KDE_ADD_UI_FILES
# KDE_AUTOMOC
# KDE_TARGET_LINK_CONV_LIBRARIES
# KDE_CREATE_LIBTOOL_FILE

#neundorf@kde.org

#this should better be part of cmake:
#add an additional file to the list of files a source file depends on
MACRO(ADD_FILE_DEPENDANCY file)
   SET(${file}_deps ${${file}_deps} ${ARGN})
   SET_SOURCE_FILES_PROPERTIES(
      ${file}
      PROPERTIES
      OBJECT_DEPENDS
      "${${file}_deps}"
   )
ENDMACRO(ADD_FILE_DEPENDANCY)


#this is a hack right now to support the behaviour of libtool convenience libs
#for other platforms:
#Darwin: -all_load $convenience
#GNU ld:  --whole-archive $convenience --no-whole-archive (depending on the ld version! must test if it supports the flag)
#Solaris 2: -z allextract $convenience -z defaultextract
#cmake 2,2 will come with some feature to support this in some way
 MACRO (KDE_TARGET_LINK_CONV_LIBRARIES _target)
   TARGET_LINK_LIBRARIES(${_target} "-Wl,--whole-archive" ${ARGN} "-Wl,--no-whole-archive")
ENDMACRO (KDE_TARGET_LINK_CONV_LIBRARIES)


#create the kidl and skeletion file for dcop stuff
#usage: KDE_ADD_COP_SKELS(foo_SRCS ${dcop_headers})
MACRO(KDE_ADD_DCOP_SKELS _sources)
   FOREACH (_current_FILE ${ARGN})
      GET_FILENAME_COMPONENT(_basename ${_current_FILE} NAME_WE)

	  SET(_skel ${CMAKE_CURRENT_BINARY_DIR}/${_basename}_skel.cpp)
      SET(_kidl ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.kidl)

	  ADD_CUSTOM_COMMAND(OUTPUT ${_kidl}
         COMMAND ${DCOPIDL}
         ARGS ${CMAKE_CURRENT_SOURCE_DIR}/${_current_FILE} > ${_kidl}
         DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_current_FILE}
      )

	  ADD_CUSTOM_COMMAND(OUTPUT ${_skel}
         COMMAND ${DCOPIDL2CPP}
         ARGS --c++-suffix cpp --no-signals --no-stub ${_kidl}
         DEPENDS ${_kidl}
      )

      SET(${_sources} ${${_sources}} ${_skel})

   ENDFOREACH (_current_FILE)

ENDMACRO(KDE_ADD_DCOP_SKELS)

#create the moc files and add them to the list of sources
#usage: KDE_ADD_MOC_FILES(foo_SRCS ${moc_headers})
MACRO(KDE_ADD_MOC_FILES _sources)
   FOREACH (_current_FILE ${ARGN})
	  GET_FILENAME_COMPONENT(_basename ${_current_FILE} NAME_WE)
      GET_FILENAME_COMPONENT(_path ${_current_FILE} PATH)
      SET(_moc ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc.cpp)

      ADD_CUSTOM_COMMAND(OUTPUT ${_moc}
         COMMAND moc
         ARGS ${CMAKE_CURRENT_SOURCE_DIR}/${_current_FILE} -o ${_moc}
         DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_current_FILE}
      )

      SET(${_sources} ${${_sources}} ${_moc})

   ENDFOREACH (_current_FILE)
ENDMACRO(KDE_ADD_MOC_FILES)

##create the moc files aka automoc from automake
##usage: KDE_CREATE_AUTOMOC_FILES(${foo_automoc_SRCS})
#MACRO(KDE_CREATE_AUTOMOC_FILES )
#   FOREACH (_current_FILE ${ARGN})
#      GET_FILENAME_COMPONENT(_basename ${_current_FILE} NAME_WE)
#      SET(_moc ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc)
#      SET(_header ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.h)
#
#      ADD_CUSTOM_COMMAND(OUTPUT ${_moc}
#         COMMAND moc
#         ARGS ${_header} -o ${_moc}
#         DEPENDS ${_header}
#      )
#
#      ADD_FILE_DEPENDANCY(${CMAKE_CURRENT_SOURCE_DIR}/${_current_FILE} ${_moc})
#
#   ENDFOREACH (_current_FILE)
#ENDMACRO(KDE_CREATE_AUTOMOC_FILES)

#create the implementation files from the ui files and add them to the list of sources
#usage: KDE_ADD_UI_FILES(foo_SRCS ${ui_files})
MACRO(KDE_ADD_UI_FILES _sources )
   FOREACH (_current_FILE ${ARGN})

	  GET_FILENAME_COMPONENT(_basename ${_current_FILE} NAME_WE)
      SET(_header ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.h)
      SET(_src ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cpp)
	  SET(_moc ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc.cpp)

      ADD_CUSTOM_COMMAND(OUTPUT ${_header}
         COMMAND uic
         ARGS  -nounload -o ${_header} ${CMAKE_CURRENT_SOURCE_DIR}/${_current_FILE}
         DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_current_FILE}
      )

#	  ADD_CUSTOM_COMMAND(OUTPUT ${_src}
#         COMMAND uic
#         ARGS -nounload -tr tr2i18n -o ${_src} -impl ${_header} ${CMAKE_CURRENT_SOURCE_DIR}/${_current_FILE}
#         DEPENDS ${_header}
#      )

	  ADD_CUSTOM_COMMAND(OUTPUT ${_src}
         COMMAND ${CMAKE_COMMAND}
         ARGS
         -DKDE_UIC_FILE:STRING=${CMAKE_CURRENT_SOURCE_DIR}/${_current_FILE}
         -DKDE_UIC_CPP_FILE:STRING=${_src}
         -DKDE_UIC_H_FILE:STRING=${_header}
         -P ${CMAKE_SOURCE_DIR}/cmake/kdeuic.cmake
         DEPENDS ${_header}
      )

      ADD_CUSTOM_COMMAND(OUTPUT ${_moc}
         COMMAND moc
         ARGS ${_header} -o ${_moc}
         DEPENDS ${_header}
      )

      SET(${_sources} ${${_sources}} ${_src} ${_moc} )

   ENDFOREACH (_current_FILE)
ENDMACRO(KDE_ADD_UI_FILES)

MACRO(KDE_AUTOMOC)
   SET(_matching_FILES )
   FOREACH (_current_FILE ${ARGN})
      IF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${_current_FILE})

         FILE(READ ${CMAKE_CURRENT_SOURCE_DIR}/${_current_FILE} _contents)

         STRING(REGEX MATCHALL "#include +[^ ]+\\.moc[\">]" _match "${_contents}")
         IF(_match)
            FOREACH (_current_MOC_INC ${_match})
               STRING(REGEX MATCH "[^ <\"]+\\.moc" _current_MOC "${_current_MOC_INC}")

               GET_FILENAME_COMPONENT(_basename ${_current_MOC} NAME_WE)
               SET(_header ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.h)
               SET(_moc    ${CMAKE_CURRENT_BINARY_DIR}/${_current_MOC})

               ADD_CUSTOM_COMMAND(OUTPUT ${_moc}
                  COMMAND moc
                  ARGS ${_header} -o ${_moc}
                  DEPENDS ${_header}
               )

               ADD_FILE_DEPENDANCY(${CMAKE_CURRENT_SOURCE_DIR}/${_current_FILE} ${_moc})

            ENDFOREACH (_current_MOC_INC)
         ENDIF(_match)

#         GET_FILENAME_COMPONENT(_basename ${_current_FILE} NAME_WE)
#         SET(_moc ${_basename}.moc)
#         STRING(REGEX MATCH "#include.+${_moc}" _match "${_contents}")
#         IF(_match)
#            KDE_CREATE_AUTOMOC_FILES(${_current_FILE})
#         ENDIF(_match)

      ENDIF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${_current_FILE})
   ENDFOREACH (_current_FILE)
ENDMACRO(KDE_AUTOMOC)

MACRO(KDE_CREATE_LIBTOOL_FILE _target )
   GET_TARGET_PROPERTY(_target_location ${_target} LOCATION)

   GET_FILENAME_COMPONENT(_laname ${_target_location} NAME_WE)
   GET_FILENAME_COMPONENT(_soname ${_target_location} NAME)
   SET(_laname ${_laname}.la)

   FILE(WRITE ${_laname} "# ${_laname} - a libtool library file, generated by cmake \n")
   FILE(APPEND ${_laname} "# The name that we can dlopen(3).\n")
   FILE(APPEND ${_laname} "dlname='${_soname}'\n")
   FILE(APPEND ${_laname} "# Names of this library\n")
   FILE(APPEND ${_laname} "library_names='${_soname} ${_soname} ${_soname}'\n")
   FILE(APPEND ${_laname} "# The name of the static archive\n")
   FILE(APPEND ${_laname} "old_library=''\n")
   FILE(APPEND ${_laname} "# Libraries that this one depends upon.\n")
   FILE(APPEND ${_laname} "dependency_libs='${${_target}_LIB_DEPENDS}'\n")
   FILE(APPEND ${_laname} "# Version information.\ncurrent=0\nage=0\nrevision=0\n")
   FILE(APPEND ${_laname} "# Is this an already installed library?\ninstalled=yes\n")
   FILE(APPEND ${_laname} "# Should we warn about portability when linking against -modules?\nshouldnotlink=yes\n")
   FILE(APPEND ${_laname} "# Files to dlopen/dlpreopen\ndlopen=''\ndlpreopen=''\n")
   FILE(APPEND ${_laname} "# Directory that this library needs to be installed in:\n")
   FILE(APPEND ${_laname} "libdir='${CMAKE_INSTALL_PREFIX}/lib/kde3'\n")

   INSTALL_FILES(/lib/kde3/lib FILES ${_laname})
ENDMACRO(KDE_CREATE_LIBTOOL_FILE)

