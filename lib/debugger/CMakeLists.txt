kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/interfaces )


########### next target ###############

set(kdevlang_debugger_LIB_SRCS
    kdevdebugger.cpp
    debugger.cpp
)

kde4_automoc(${kdevlang_debugger_LIB_SRCS})
kde4_add_library(kdevlang_debugger SHARED ${kdevlang_debugger_LIB_SRCS})
target_link_libraries(kdevlang_debugger ${KDE4_KDECORE_LIBS} kparts kdevinterfaces)
set_target_properties(kdevlang_debugger PROPERTIES VERSION 4.0.0 SOVERSION 4)
install_targets(/lib kdevlang_debugger)


########### install files ###############

install_files(/include/kdevelop/languages/debugger FILES)


kde4_footer()
