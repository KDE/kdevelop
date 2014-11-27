add_definitions(-DTRANSLATION_DOMAIN=\"kdevexternalscript\")
########### next target ###############

set(kdevexternalscript_PART_SRCS
    externalscriptplugin.cpp
    externalscriptview.cpp
    externalscriptitem.cpp
    externalscriptjob.cpp
    editexternalscript.cpp
    externalscriptdebug.cpp
)

set(kdevexternalscript_PART_UI
    externalscriptview.ui
    editexternalscript.ui
)

ki18n_wrap_ui(kdevexternalscript_PART_SRCS ${kdevexternalscript_PART_UI})

add_library(kdevexternalscript MODULE ${kdevexternalscript_PART_SRCS})

target_link_libraries(kdevexternalscript
    KF5::TextEditor KF5::KIOWidgets KF5::Parts KF5::NewStuff
    KDev::Language KDev::Interfaces KDev::Project
    KDev::Util KDev::OutputView
)

install( TARGETS kdevexternalscript DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION} )

########### install files ###############
configure_file(kdevexternalscript.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevexternalscript.desktop)
kcoreaddons_desktop_to_json(kdevexternalscript ${CMAKE_CURRENT_BINARY_DIR}/kdevexternalscript.desktop)
install( FILES kdevexternalscript.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevexternalscript )
