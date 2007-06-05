
add_subdirectory(tests) 

include_directories(${KDEVPLATFORM_INCLUDE_DIR} ${KDEVPLATFORM_INCLUDE_DIR}/projectmanager/interfaces 
    ${KDEVPLATFORM_INCLUDE_DIR}/project ${KDEVPLATFORM_INCLUDE_DIR}/util
    )


########### next target ###############

set(kdevautomakeparser_STAT_SRCS
    automakeast.cpp
    automakedriver.cpp
    automakeparser.cpp
    makefileinterface.cpp
)

kde4_automoc(${kdevautomakeparser_STAT_SRCS})
kde4_add_library(kdevautomakeparser STATIC ${kdevautomakeparser_STAT_SRCS})


########### next target ###############

set(kdevautomakeimporter_PART_SRCS
    automakeprojectmodel.cpp
    automakeimporter.cpp
)

kde4_automoc(${kdevautomakeimporter_PART_SRCS})
kde4_add_plugin(kdevautomakeimporter ${kdevautomakeimporter_PART_SRCS})
target_link_libraries(kdevautomakeimporter ${KDE4_KDECORE_LIBS} kdevautomakeparser kdevplatform)

install(TARGETS kdevautomakeimporter DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevautomakeimporter.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


