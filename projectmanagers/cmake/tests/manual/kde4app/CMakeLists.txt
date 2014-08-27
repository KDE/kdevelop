project(kde4app)

find_package(KDE4 REQUIRED)
include (KDE4Defaults)

include_directories( ${KDE4_INCLUDES} ${QT_INCLUDES} )

set(kde4app_SRCS
   kde4app.cpp
   main.cpp
   kde4appview.cpp
 )

kde4_add_ui_files(kde4app_SRCS kde4appview_base.ui prefs_base.ui)

kde4_add_kcfg_files(kde4app_SRCS settings.kcfgc )

kde4_add_executable(kde4app ${kde4app_SRCS})

target_link_libraries(kde4app ${KDE4_KDEUI_LIBS} )

install(TARGETS kde4app ${INSTALL_TARGETS_DEFAULT_ARGS} )

kde4_add_unit_test(testkde4app kde4app.cpp)

########### install files ###############

install( PROGRAMS kde4app.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES kde4app.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES kde4appui.rc  DESTINATION  ${KXMLGUI_INSTALL_DIR}/kde4app )

add_subdirectory(tests)
