
########### next target ###############

set(kdevplatformutil_LIB_SRCS
    processlinemaker.cpp
)

kde4_automoc(${kdevplatformutil_LIB_SRCS})
kde4_add_library(kdevplatformutil SHARED ${kdevplatformutil_LIB_SRCS})
target_link_libraries(kdevplatformutil ${KDE4_KDEUI_LIBS} kdevplatforminterfaces)
set_target_properties(kdevplatformutil PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS kdevplatformutil DESTINATION ${LIB_INSTALL_DIR} )



########### install files ###############

install( FILES 
	processlinemaker.h
	utilexport.h
	DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/util)


