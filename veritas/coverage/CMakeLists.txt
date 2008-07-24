add_subdirectory(tests)

add_definitions( ${KDE4_ENABLE_EXCEPTIONS} )
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9003 )

include_directories(
    ${KDE4_INCLUDES}
    ${kdevplatform_SOURCE_DIR}/interfaces
    ${kdevplatform_SOURCE_DIR}/outputview
    ${kdevplatform_BINARY_DIR}/veritas)

########### next target ###############

set(veritascoverage_SRCS
    coverageplugin.cpp
    coveredfile.cpp
    lcovinfoparser.cpp
    outputview/covoutputjob.cpp
    outputview/covoutputmodel.cpp
    outputview/covoutputdelegate.cpp)
qt4_automoc(${veritascoverage_SRCS})
kde4_add_plugin(kdevcoverage ${veritascoverage_SRCS})
target_link_libraries(kdevcoverage
    kdevplatformoutputview
    kdevplatforminterfaces
    kdevplatformutil
    kdevplatformproject
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDE4_KDEUI_LIBS})
install(TARGETS kdevcoverage DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES kdevcoverage.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevcoverage.rc DESTINATION ${DATA_INSTALL_DIR}/kdevcoverage )
install( PROGRAMS lcov_geninfo DESTINATION ${BIN_INSTALL_DIR} )
