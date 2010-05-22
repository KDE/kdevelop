option(CPP_DEBUG_UI_LOCKUP "Enable this setting to debug UI lockups in the C++ support" OFF)
option(CPP_DEBUG_EXTRA "Enable excessive debug output from C++ support" OFF)
option(CPP_DEBUG_INCLUDE_PATHS "Enable include path debugging for C++ support" ON)
option(CPP_DEBUG_LEXERCACHE "Enable debugging the C++ lexer cache" OFF)
option(CPP_DEBUG_CONTEXT_RANGES "Enable debugging the C++ context ranges" OFF)
option(CPP_DEBUG_UPDATE_MATCHING "Enable debugging the C++ update matching" OFF)

macro_bool_to_01(CPP_DEBUG_LEXERCACHE DEBUG_LEXERCACHE)
macro_bool_to_01(CPP_DEBUG_UI_LOCKUP DEBUG_UI_LOCKUP)
macro_bool_to_01(CPP_DEBUG_EXTRA DEBUG)
macro_bool_to_01(CPP_DEBUG_INCLUDE_PATHS DEBUG_INCLUDE_PATHS)
macro_bool_to_01(CPP_DEBUG_CONTEXT_RANGES DEBUG_CONTEXT_RANGES)
macro_bool_to_01(CPP_DEBUG_UPDATE_MATCHING DEBUG_UPDATE_MATCHING)

mark_as_advanced(CPP_DEBUG_UI_LOCKUP CPP_DEBUG_EXTRA CPP_DEBUG_INCLUDE_PATHS)
mark_as_advanced(CPP_DEBUG_LEXERCACHE CPP_DEBUG_CONTEXT_RANGES)
mark_as_advanced(CPP_DEBUG_UPDATE_MATCHING)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cppdebughelper.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/cppdebughelper.h) 

