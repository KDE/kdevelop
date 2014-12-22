add_definitions(-DTRANSLATION_DOMAIN=\"kdevexecutescript\")
project(executescript)

include_directories(
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

########### next target ###############

set(kdevexecutescript_PART_UIS
    scriptappconfig.ui
)

set(kdevexecutescript_PART_SRCS
    executescriptplugin.cpp
    scriptappconfig.cpp
    scriptappjob.cpp
)

ki18n_wrap_ui( kdevexecutescript_PART_SRCS ${kdevexecutescript_PART_UIS} )

kdevplatform_add_plugin(kdevexecutescript JSON kdevexecutescript.json SOURCES ${kdevexecutescript_PART_SRCS})

target_link_libraries(kdevexecutescript
    KDev::Interfaces
    KDev::Util
    KDev::Project
    KDev::OutputView)

########### install files ###############

install(FILES iexecutescriptplugin.h DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/executescript COMPONENT Devel)
