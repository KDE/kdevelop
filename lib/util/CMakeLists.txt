kde4_header()

include_directories(${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})


########### next target ###############

set(kdevutil_LIB_SRCS
    execcommand.cpp
    kfiltermodel.cpp
    kdevtreeview.cpp
    configwidgetproxy.cpp
    settings.cpp
)

kde4_automoc(${kdevutil_LIB_SRCS})
kde4_add_library(kdevutil SHARED ${kdevutil_LIB_SRCS})
target_link_libraries(kdevutil ${KDE4_KDECORE_LIBS} kdeui kdevinterfaces)
set_target_properties(kdevutil PROPERTIES VERSION 4.0.0 SOVERSION 4)
install_targets(/lib kdevutil)



########### install files ###############

install_files(/include/kdevelop/util FILES execcommand.h configwidgetproxy.h kfiltermodel.h kdevtreeview.h)


kde4_footer()
