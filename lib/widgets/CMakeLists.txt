
include_directories(${CMAKE_SOURCE_DIR}/lib )


########### next target ###############

set(kdevwidgets_LIB_SRCS
    kdevhtmlpart.cpp
    processlinemaker.cpp
    processwidget.cpp
    ksavealldialog.cpp
)

kde4_automoc(kdevwidgets ${kdevwidgets_LIB_SRCS})
kde4_add_library(kdevwidgets SHARED ${kdevwidgets_LIB_SRCS})
target_link_libraries(kdevwidgets ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KHTML_LIBS} kdevplatform)
set_target_properties(kdevwidgets PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS kdevwidgets DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install( FILES kdevhtml_partui.rc DESTINATION ${DATA_INSTALL_DIR}/kdevelop )
install( FILES kdevhtmlpart.h processlinemaker.h processwidget.h ksavealldialog.h DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/widgets)


