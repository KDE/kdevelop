kde4_header()

include_directories(${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/interfaces )


########### next target ###############

set(kdevwidgets_LIB_SRCS
    kdevhtmlpart.cpp
    processlinemaker.cpp
    processwidget.cpp
    ksavealldialog.cpp
)

kde4_automoc(${kdevwidgets_LIB_SRCS})
kde4_add_library(kdevwidgets SHARED ${kdevwidgets_LIB_SRCS})
target_link_libraries(kdevwidgets ${KDE4_KDECORE_LIBS} kdeui khtml kdevinterfaces)
set_target_properties(kdevwidgets PROPERTIES VERSION 4.0.0 SOVERSION 4)
install_targets(/lib kdevwidgets)


########### install files ###############

install_files(${DATA_INSTALL_DIR}/kdevelop FILES kdevhtml_partui.rc)
install_files(/include/kdevelop/widgets FILES kdevhtmlpart.h processlinemaker.h processwidget.h ksavealldialog.h)


kde4_footer()
