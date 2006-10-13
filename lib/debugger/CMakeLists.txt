
include_directories( ${CMAKE_SOURCE_DIR}/lib )


########### next target ###############

set(kdevlang_debugger_LIB_SRCS
    kdevdebugger.cpp
    debugger.cpp
)

kde4_automoc(kdevlang_debugger ${kdevlang_debugger_LIB_SRCS})
kde4_add_library(kdevlang_debugger SHARED ${kdevlang_debugger_LIB_SRCS})
target_link_libraries(kdevlang_debugger ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} kdevplatform)
set_target_properties(kdevlang_debugger PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS kdevlang_debugger DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

#install( FILES DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/languages/debugger)


