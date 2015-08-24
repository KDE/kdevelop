file(GLOB jsfiles RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.js)
file(GLOB qmlfiles RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.qml)
install(FILES ${jsfiles} ${qmlfiles} DESTINATION ${DATA_INSTALL_DIR}/kdevqmljssupport/nodejsmodules)
