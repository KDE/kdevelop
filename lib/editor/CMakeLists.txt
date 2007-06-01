include_directories(
    ${CMAKE_SOURCE_DIR}/lib/interfaces
)

set(kdevplatformeditor_LIB_SRCS
    documentrange.cpp
    documentrangeobject.cpp
    documentcursor.cpp
    documentcursorobject.cpp
    editorintegrator.cpp
)

kde4_automoc(${kdevplatformeditor_LIB_SRCS})

kde4_add_library(kdevplatformeditor SHARED ${kdevplatformeditor_LIB_SRCS})
target_link_libraries(kdevplatformeditor ${KDE4_KPARTS_LIBS} ${KDE4_KTEXTEDITOR_LIBS})
set_target_properties(kdevplatformeditor PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS kdevplatformeditor DESTINATION ${LIB_INSTALL_DIR} )

########### install files ###############

install(FILES
    documentrange.h
    documentrangeobject.h
    documentcursor.h
    documentcursorobject.h
    editorintegrator.h
    editorexport.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/editor
 )
