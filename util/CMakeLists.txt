
########### next target ###############

set(kdevplatformutil_LIB_SRCS
    processlinemaker.cpp
    commandexecutor.cpp
    environmentselectionwidget.cpp
    environmentgrouplist.cpp
)

set (kdevplatformutil_LIB_UI
    runoptions.ui
)

kde4_add_ui_files(kdevplatformutil_LIB_SRCS ${kdevplatformutil_LIB_US})
kde4_add_library(kdevplatformutil SHARED ${kdevplatformutil_LIB_SRCS})
target_link_libraries(kdevplatformutil ${KDE4_KDEUI_LIBS} kdevplatforminterfaces)
set_target_properties(kdevplatformutil PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kdevplatformutil DESTINATION ${LIB_INSTALL_DIR} )

########### install files ###############

install( FILES
    processlinemaker.h
    commandexecutor.h
    utilexport.h
    environmentselectionwidget.h
    environmentgrouplist.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/util)


