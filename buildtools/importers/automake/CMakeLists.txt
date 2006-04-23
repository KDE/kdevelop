kde4_header()

add_subdirectory(tests) 

include_directories(${CMAKE_SOURCE_DIR}/lib/projectmanager/interfaces 
    ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/project ${CMAKE_SOURCE_DIR}/lib/util
    ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})


add_definitions(-fPIC)

########### next target ###############

set(kdevautomakeparser_STAT_SRCS
    autotoolsast.cpp
    autotoolsdriver.cpp
    autotools_yacc.cpp
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
target_link_libraries(kdevautomakeimporter ${KDE4_KDECORE_LIBS} kdevautomakeparser kdevinterfaces)
kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdevautomakeimporter)
install_targets(${PLUGIN_INSTALL_DIR} kdevautomakeimporter)


########### install files ###############

install_files(${SERVICES_INSTALL_DIR} FILES kdevautomakeimporter.desktop)


kde4_footer()
