file(GLOB jsfiles RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.js)
install(FILES ${jsfiles} DESTINATION ${DATA_INSTALL_DIR}/kdevqmljssupport/nodejsmodules)
