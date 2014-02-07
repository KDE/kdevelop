add_subdirectory(kcm_widget)

include_directories( ${KDE4_INCLUDES} ${KDEVPLATFORM_INCLUDE_DIR} )

add_definitions( ${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

set( customDefinesAndIncludes_INCLUDE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE )

set( kdevcustomdefinesandincludesmanager_SRCS
        customdefinesandincludesmanager.cpp
        settingsmanager.cpp
        settingsconverter.cpp
    )
kde4_add_library( kdevcustomdefinesandincludesmanager SHARED
        ${kdevcustomdefinesandincludesmanager_SRCS})
target_link_libraries( kdevcustomdefinesandincludesmanager LINK_PRIVATE
        ${KDEVPLATFORM_PROJECT_LIBRARIES}
        ${KDEVPLATFORM_LANGUAGE_LIBRARIES} )

install( TARGETS kdevcustomdefinesandincludesmanager
        ${INSTALL_TARGETS_DEFAULT_ARGS} )