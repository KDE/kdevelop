
include_directories(${CMAKE_SOURCE_DIR}/lib )


########### next target ###############

set(kdevutil_LIB_SRCS
    execcommand.cpp
    kfiltermodel.cpp
    kdevtreeview.cpp
    settings.cpp
)

kde4_automoc(${kdevutil_LIB_SRCS})
kde4_add_library(kdevutil SHARED ${kdevutil_LIB_SRCS})
target_link_libraries(kdevutil ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} kdevplatform)
set_target_properties(kdevutil PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS kdevutil DESTINATION ${LIB_INSTALL_DIR} )



########### install files ###############

install( FILES execcommand.h kfiltermodel.h kdevtreeview.h DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/util)


