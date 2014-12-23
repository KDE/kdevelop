project(executeplasmoid)
add_definitions(-DTRANSLATION_DOMAIN=\"kdevexecuteplasmoid\")
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

########### next target ###############

set(kdevexecuteplasmoid_PART_UIS
    plasmoidexecutionconfig.ui
)

set(kdevexecuteplasmoid_PART_SRCS
    executeplasmoidplugin.cpp
    plasmoidexecutionconfig.cpp
    plasmoidexecutionjob.cpp
)

ki18n_wrap_ui( kdevexecuteplasmoid_PART_SRCS ${kdevexecuteplasmoid_PART_UIS} )
kdevplatform_add_plugin(kdevexecuteplasmoid JSON kdevexecuteplasmoid.json SOURCES ${kdevexecuteplasmoid_PART_SRCS})
target_link_libraries(kdevexecuteplasmoid
    KDev::Language
    KDev::Interfaces
    KDev::Util
    KDev::Project
    KDev::OutputView)
