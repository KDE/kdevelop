
#add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9020 )
add_subdirectory(tests) 

########### next target ###############

set(kdevautomakeparser_STAT_SRCS
    automakeast.cpp
    automakedriver.cpp
    automakeparser.cpp
    makefileinterface.cpp
)


kde4_add_library(kdevautomakeparser STATIC ${kdevautomakeparser_STAT_SRCS})


########### next target ###############

set(kdevautomakeimporter_PART_SRCS
    automakeprojectmodel.cpp
    automakeimporter.cpp
)


kde4_add_plugin(kdevautomakeimporter ${kdevautomakeimporter_PART_SRCS})
target_link_libraries(kdevautomakeimporter ${KDE4_KDECORE_LIBS} kdevautomakeparser kdevplatform)

install(TARGETS kdevautomakeimporter DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevautomakeimporter.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


