add_definitions(-DTRANSLATION_DOMAIN=\"kdevprojectfilter\")
##############################
# PLUGIN #####################
##############################

set( projectfilter_SRCS
    projectfilterprovider.cpp
    projectfilter.cpp
    projectfilterdebug.cpp
    filter.cpp
    projectfilterconfigpage.cpp
    projectfilterdebug.cpp
    filter.cpp
    filtermodel.cpp
    comboboxdelegate.cpp
)

ki18n_wrap_ui(projectfilter_SRCS projectfiltersettings.ui)
kconfig_add_kcfg_files(projectfilter_SRCS projectfiltersettings.kcfgc)

kdevplatform_add_plugin(kdevprojectfilter JSON kdevprojectfilter.json SOURCES ${projectfilter_SRCS})

target_link_libraries(kdevprojectfilter
    KDev::Project
    KDev::Util
    KDev::Interfaces
)

add_subdirectory(tests)
