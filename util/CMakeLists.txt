
########### next target ###############

set(kdevplatformutil_LIB_SRCS
    processlinemaker.cpp
    commandexecutor.cpp
    envwidget.cpp
)

set(kdevplatformutil_LIB_UI
    envsettings.ui
)

kde4_add_ui_files(kdevplatformutil_LIB_SRCS ${kdevplatformutil_LIB_UI})
kde4_add_library(kdevplatformutil SHARED ${kdevplatformutil_LIB_SRCS})
target_link_libraries(kdevplatformutil ${KDE4_KDEUI_LIBS} kdevplatforminterfaces)
set_target_properties(kdevplatformutil PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS kdevplatformutil DESTINATION ${LIB_INSTALL_DIR} )



########### install files ###############

install( FILES
    processlinemaker.h
    commandexecutor.h
    utilexport.h
    envwidget.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/util)


