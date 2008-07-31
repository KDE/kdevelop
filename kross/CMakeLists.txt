project(krosssupport)

add_subdirectory(xmltokross)

include_directories(
    ${KDE4_INCLUDES}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/interfaces
    ${CMAKE_SOURCE_DIR}/project
    ${CMAKE_SOURCE_DIR}/project/interfaces
    ${CMAKE_SOURCE_DIR}/language
    ${CMAKE_SOURCE_DIR}/language/duchain
    ${CMAKE_SOURCE_DIR}/vcs
    ${CMAKE_SOURCE_DIR}/vcs/interfaces
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers
)

find_program(CPPTOXML cpptoxml)

if(CPPTOXML)
    macro(kdev_create_kross_iface interface output includes)
        get_filename_component(weFile ${output} NAME_WE)
        get_filename_component(outPath ${output} PATH)
        
        add_custom_command(OUTPUT ${output} ${headerFile}
                            COMMAND ${CPPTOXML} -C${CMAKE_CURRENT_SOURCE_DIR}/xmltokross/rppconfig.h -Q/smthwrong/${QT_INCLUDE_DIR} ${interface} |
                                ${CMAKE_CURRENT_BINARY_DIR}/xmltokross/xmltokross -D${outPath} -F${weFile} -I${includes} > ${output}
                            MAIN_DEPENDENCY ${interface}
                            DEPENDS xmltokross )
        set(mocFile ${weFile}.moc)
        qt4_generate_moc( ${output} ${CMAKE_CURRENT_BINARY_DIR}/${mocFile})
    endmacro(kdev_create_kross_iface)

    kdev_create_kross_iface(${CMAKE_SOURCE_DIR}/interfaces/idocument.h
        ${CMAKE_SOURCE_DIR}/kross/wrappers/krossidocument.cpp idocument.h)
    kdev_create_kross_iface(${CMAKE_SOURCE_DIR}/interfaces/iuicontroller.h
        ${CMAKE_SOURCE_DIR}/kross/wrappers/krossiuicontroller.cpp iuicontroller.h)
    kdev_create_kross_iface(${CMAKE_SOURCE_DIR}/interfaces/context.h
        ${CMAKE_SOURCE_DIR}/kross/wrappers/krosscontext.cpp context.h)
    kdev_create_kross_iface(${CMAKE_SOURCE_DIR}/interfaces/contextmenuextension.h
        ${CMAKE_SOURCE_DIR}/kross/wrappers/krosscontextmenuextension.cpp contextmenuextension.h)
    kdev_create_kross_iface(${CMAKE_SOURCE_DIR}/project/projectmodel.h
        ${CMAKE_SOURCE_DIR}/kross/wrappers/krossprojectmodel.cpp projectmodel.h)
    kdev_create_kross_iface(${CMAKE_SOURCE_DIR}/vcs/vcsrevision.h
        ${CMAKE_SOURCE_DIR}/kross/wrappers/krossvcsrevision.cpp vcsrevision.h)
    kdev_create_kross_iface(${CMAKE_SOURCE_DIR}/vcs/vcslocation.h
        ${CMAKE_SOURCE_DIR}/kross/wrappers/krossvcslocation.cpp vcslocation.h)
    kdev_create_kross_iface(${CMAKE_SOURCE_DIR}/language/duchain/topducontext.h
        ${CMAKE_SOURCE_DIR}/kross/wrappers/krosstopducontext.cpp topducontext.h)
endif(CPPTOXML)

set(kdevkrossplugin_SRCS
    krossplugin.cpp
    krossbuildsystemmanager.cpp
    krossdistributedversioncontrol.cpp
    krosstoolviewfactory.cpp
    krossvcsjob.cpp
#     projectitemadaptors.cpp
    wrappers/krossiuicontroller.cpp
    wrappers/krossidocument.cpp
    wrappers/krosscontext.cpp
    wrappers/krosscontextmenuextension.cpp
    wrappers/krossvcsrevision.cpp
    wrappers/krossvcslocation.cpp
#     wrappers/krosstopducontext.cpp
    wrappers/krossprojectmodel.cpp
)

kde4_add_plugin(kdevkrossplugin ${kdevkrossplugin_SRCS})
target_link_libraries(kdevkrossplugin ${KDE4_KDEUI_LIBS}
    kdevplatforminterfaces kdevplatformproject kdevplatformvcs ${KDE4_KROSSUI_LIBS})

install(TARGETS kdevkrossplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kdevkrossplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})

