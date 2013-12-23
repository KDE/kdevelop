cmake_minimum_required (VERSION 2.8)

project(cppcheck)

find_package(KDE4 REQUIRED)
find_package(KDevPlatform 1.5.0 REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})


include( MacroLibrary )
include(KDevPluginVersion)

include_directories(
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${KDE4_INCLUDES}
    ${cppcheck_BINARY_DIR}
    ${cppcheck_SOURCE_DIR}
    ${cppcheck_SOURCE_DIR}/config
    ${cppcheck_SOURCE_DIR}/models
    ${cppcheck_SOURCE_DIR}/parsers
    ${cppcheck_SOURCE_DIR}/jobs
    ${cppcheck_SOURCE_DIR}/views
)


########### next target ###############

set(kdevcppcheck_PART_SRCS
    job.cpp
    marks.cpp
    imodel.cpp
    modelwrapper.cpp
    iparser.cpp
    plugin.cpp
    widget.cpp
    models/cppcheckmodel.cpp
    models/cppcheckitemsimpl.cpp
    models/cppcheck_file_model.cpp
    models/cppcheck_severity_model.cpp
    models/cppcheck_file_item.cpp
    models/cppcheck_severity_item.cpp
    parsers/cppcheckparser.cpp
    views/cppcheckview.cpp
)

set(kdevcppcheck_CONFIG_SRCS

)

set(kdevcppcheck_PART_UI
)

kde4_add_ui_files(kdevcppcheck_PART_SRCS  ${kdevcppcheck_PART_UI})

kde4_add_plugin(kdevcppcheck ${kdevcppcheck_PART_SRCS} ${kdevcppcheck_CONFIG_SRCS})

target_link_libraries(kdevcppcheck
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDEVPLATFORM_OUTPUTVIEW_LIBRARIES}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KUTILS_LIBS}
    ${KDEVPLATFORM_UTIL_LIBRARIES}
    ${KDE4_KTEXTEDITOR_LIBS}
    ${QT_QTNETWORK_LIBRARY})

install(TARGETS kdevcppcheck DESTINATION ${PLUGIN_INSTALL_DIR} )
########### install files ###############

configure_file(kdevcppcheck.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevcppcheck.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevcppcheck.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kdevcppcheck.rc DESTINATION ${DATA_INSTALL_DIR}/kdevcppcheck )






#preferences-plugin
set(cppcheckpreferences_PART_SRCS 
    config/genericconfigpage.cpp
)
kde4_add_ui_files(cppcheckpreferences_PART_SRCS config/ui/genericconfig.ui)

kde4_add_plugin(kcm_kdev_cppcheck ${cppcheckpreferences_PART_SRCS})

target_link_libraries(kcm_kdev_cppcheck
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDE4_KIO_LIBS}
    ${KDE4_KUTILS_LIBS}
    ${KDE4_KFILE_LIBS}
)

install(TARGETS kcm_kdev_cppcheck  DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES  kcm_kdev_cppcheck.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

