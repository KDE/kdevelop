option(CPP_DEBUG_UI_LOCKUP "Enable this setting to debug UI lockups in the C++ support" OFF)
option(CPP_DEBUG_EXTRA "Enable excessive debug output from C++ support" OFF)
option(CPP_DEBUG_INCLUDE_PATHS "Enable include path debugging for C++ support" ON)
option(CPP_DEBUG_LEXERCACHE "Enable debugging the C++ lexer cache" OFF)
option(CPP_DEBUG_CONTEXT_RANGES "Enable debugging the C++ context ranges" OFF)
option(CPP_DEBUG_UPDATE_MATCHING "Enable debugging the C++ update matching" OFF)

set(DEBUG_LEXERCACHE ${CPP_DEBUG_LEXERCACHE})
set(DEBUG_UI_LOCKUP ${CPP_DEBUG_UI_LOCKUP})
set(DEBUG ${CPP_DEBUG_EXTRA})
set(DEBUG_INCLUDE_PATHS ${CPP_DEBUG_INCLUDE_PATHS})
set(DEBUG_CONTEXT_RANGES ${CPP_DEBUG_CONTEXT_RANGES})
set(DEBUG_UPDATE_MATCHING ${CPP_DEBUG_UPDATE_MATCHING})

mark_as_advanced(CPP_DEBUG_UI_LOCKUP CPP_DEBUG_EXTRA CPP_DEBUG_INCLUDE_PATHS)
mark_as_advanced(CPP_DEBUG_LEXERCACHE CPP_DEBUG_CONTEXT_RANGES)
mark_as_advanced(CPP_DEBUG_UPDATE_MATCHING)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cppdebughelper.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/cppdebughelper.h) 

