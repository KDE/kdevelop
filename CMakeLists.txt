project(kdevelop4-custom-buildsystem)

find_package(KDE4 4.3.82 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
find_package(KDevPlatform 0.9.97 REQUIRED)

include_directories( ${KDE4_INCLUDES} ${KDEVPLATFORM_INCLUDE_DIR})

add_definitions( ${QT_DEFINITIONS} ${KDE4_DEFINITIONS} )

