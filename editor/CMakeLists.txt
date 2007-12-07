include_directories(
    ${CMAKE_SOURCE_DIR}/interfaces
)

set(kdevplatformeditor_LIB_SRCS
    documentrange.cpp
    documentrangeobject.cpp
    documentcursor.cpp
    documentcursorobject.cpp
    editorintegrator.cpp
    editorintegratorstatic.cpp
    hashedstring.cpp
)


kde4_add_library(kdevplatformeditor SHARED ${kdevplatformeditor_LIB_SRCS})
target_link_libraries(kdevplatformeditor ${KDE4_KPARTS_LIBS} ${KDE4_KTEXTEDITOR_LIBS})
set_target_properties(kdevplatformeditor PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kdevplatformeditor DESTINATION ${LIB_INSTALL_DIR} )

########### install files ###############

install(FILES
    documentrange.h
    documentrangeobject.h
    documentcursor.h
    documentcursorobject.h
    editorintegrator.h
    editorexport.h
    hashedstring.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/editor
 )
