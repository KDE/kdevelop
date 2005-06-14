#this file contains the following macros:
# ADD_FILE_DEPENDANCY
# KDE_ADD_DCOP_SKELS
# KDE_ADD_DCOP_STUBS
# KDE_ADD_MOC_FILES
# KDE_ADD_UI_FILES
# KDE_ADD_KCFG_FILES
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

MACRO(KDE_ADD_DCOP_STUBS _sources)
   FOREACH (_current_FILE ${ARGN})

      IF(${_current_FILE} MATCHES "^/.+") #abs path
         SET(_tmp_FILE ${_current_FILE})
      ELSE(${_current_FILE} MATCHES "^/.+")
         SET(_tmp_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${_current_FILE})
      ENDIF(${_current_FILE} MATCHES "^/.+")

      GET_FILENAME_COMPONENT(_basename ${_tmp_FILE} NAME_WE)

	  SET(_stub_CPP ${CMAKE_CURRENT_BINARY_DIR}/${_basename}_stub.cpp)
#	  SET(_stub_H ${CMAKE_CURRENT_BINARY_DIR}/${_basename}_stub.h)
      SET(_kidl ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.kidl)

	  ADD_CUSTOM_COMMAND(OUTPUT ${_kidl}
         COMMAND ${DCOPIDL}
         ARGS ${tmp_FILE} > ${_kidl}
         DEPENDS ${tmp_FILE}
      )

	  ADD_CUSTOM_COMMAND(OUTPUT ${_stub_CPP}
         COMMAND ${DCOPIDL2CPP}
         ARGS --c++-suffix cpp --no-signals --no-skel ${_kidl}
         DEPENDS ${_kidl}
      )

      SET(${_sources} ${${_sources}} ${_stub_CPP})

   ENDFOREACH (_current_FILE)

ENDMACRO(KDE_ADD_DCOP_STUBS)

MACRO(KDE_ADD_KCFG_FILES _sources)
   FOREACH (_current_FILE ${ARGN})

      IF(${_current_FILE} MATCHES "^/.+") #abs path
         SET(_tmp_FILE ${_current_FILE})
      ELSE(${_current_FILE} MATCHES "^/.+")
         SET(_tmp_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${_current_FILE})
      ENDIF(${_current_FILE} MATCHES "^/.+")

      GET_FILENAME_COMPONENT(_basename ${_tmp_FILE} NAME_WE)

      STRING(REGEX REPLACE "(.+)settings" "${CMAKE_CURRENT_SOURCE_DIR}/\\1.kcfg" _kcfg_FILE "${_basename}")

	  SET(_src_FILE ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cpp)
#	  SET(_stub_H ${CMAKE_CURRENT_BINARY_DIR}/${_basename}_stub.h)
      SET(_header_FILE ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.h)

#MESSAGE(STATUS "____________ kcfg: ${_kcfg_FILE}")
      ADD_CUSTOM_COMMAND(OUTPUT ${_src_FILE}
         COMMAND ${KCFGC}
         ARGS ${_kcfg_FILE} ${_tmp_FILE}
         DEPENDS ${_tmp_FILE} )


      SET(${_sources} ${${_sources}} ${_src_FILE})

   ENDFOREACH (_current_FILE)

ENDMACRO(KDE_ADD_KCFG_FILES)





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

         IF(${_current_FILE} MATCHES "^/.+")
            SET(_tmp_FILE ${_current_FILE})
         ELSE(${_current_FILE} MATCHES "^/.+")
            SET(_tmp_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${_current_FILE})
         ENDIF(${_current_FILE} MATCHES "^/.+")
         GET_FILENAME_COMPONENT(_abs_FILE ${_tmp_FILE} ABSOLUTE)
         GET_FILENAME_COMPONENT(_abs_PATH ${_abs_FILE} PATH)

         STRING(REGEX MATCHALL "#include +[^ ]+\\.moc[\">]" _match "${_contents}")
         IF(_match)
            FOREACH (_current_MOC_INC ${_match})
               STRING(REGEX MATCH "[^ <\"]+\\.moc" _current_MOC "${_current_MOC_INC}")

               GET_FILENAME_COMPONENT(_basename ${_current_MOC} NAME_WE)
#               SET(_header ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.h)
               SET(_header ${_abs_PATH}/${_basename}.h)
               SET(_moc    ${CMAKE_CURRENT_BINARY_DIR}/${_current_MOC})

               ADD_CUSTOM_COMMAND(OUTPUT ${_moc}
                  COMMAND moc
                  ARGS ${_header} -o ${_moc}
                  DEPENDS ${_header}
               )

               ADD_FILE_DEPENDANCY(${CMAKE_CURRENT_SOURCE_DIR}/${_current_FILE} ${_moc})

            ENDFOREACH (_current_MOC_INC)
         ENDIF(_match)

      ENDIF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${_current_FILE})
   ENDFOREACH (_current_FILE)
ENDMACRO(KDE_AUTOMOC)
