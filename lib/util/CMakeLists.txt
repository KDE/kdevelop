
include_directories(${CMAKE_SOURCE_DIR}/lib )


########### next target ###############

set(koncreteutil_LIB_SRCS
#     execcommand.cpp
    kfiltermodel.cpp
    kdevtreeview.cpp
#     settings.cpp
    kdevitemmodel.cpp
)

kde4_automoc(${koncreteutil_LIB_SRCS})
kde4_add_library(koncreteutil SHARED ${koncreteutil_LIB_SRCS})
target_link_libraries(koncreteutil ${KDE4_KDEUI_LIBS} koncreteinterfaces)
set_target_properties(koncreteutil PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS koncreteutil DESTINATION ${LIB_INSTALL_DIR} )



########### install files ###############

install( FILES execcommand.h kfiltermodel.h kdevtreeview.h DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/util)


