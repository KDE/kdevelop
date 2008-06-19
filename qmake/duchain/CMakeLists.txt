
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

set(duchain_STAT_SRCS
    contextbuilder.cpp
    qmakeeditorintegrator.cpp
    )

# Note: This library doesn't follow API/ABI/BC rules and shouldn't have a SOVERSION
#       Its only purpose is to support the plugin without needing to add all source files
#       to the plugin target
kde4_add_library( kdev4qmakeduchain SHARED ${duchain_STAT_SRCS})
target_link_libraries(kdev4qmakeduchain ${KDE4_KDECORE_LIBS} kdev4qmakeparser ${KDEVPLATFORM_LANGUAGE_LIBRARY} ${KDE4_KTEXTEDITOR_LIBS} )

install(TARGETS kdev4qmakeduchain ${INSTALL_TARGETS_DEFAULT_ARGS})

