file(GLOB qmlfiles RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.qml)
install(FILES ${qmlfiles} DESTINATION ${DATA_INSTALL_DIR}/kdevqmljssupport/qmlplugins)