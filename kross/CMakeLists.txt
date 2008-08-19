project(krosssupport)

#Aleix's note: please, don't add it yet until I can commit everything (involving the uiless shell)
option(GENERATE_INTERFACES "Generate kross interfaces at compile-time? You need kdevcpp to use it" OFF)
if(GENERATE_INTERFACES)
    add_subdirectory(xmltokross)
    get_directory_property(include_DIRS INCLUDE_DIRECTORIES)
    
    get_directory_property(inclist INCLUDE_DIRECTORIES)
    foreach(inc ${inclist})
        set(incs ${incs}:${inc})
    endforeach(inc)
    
    macro(kdev_create_kross_iface interface output includes)
        get_filename_component(weFile ${output} NAME_WE)
        get_filename_component(outPath ${output} PATH)
        get_filename_component(ifacePath ${interface} PATH)
        
        add_custom_command(OUTPUT ${output} ${headerFile}
                           COMMAND ${CMAKE_CURRENT_BINARY_DIR}/xmltokross/duchaintokross ${interface}
                                                -I${ifacePath}:${incs} -F${weFile} -i${includes} -D${outPath} -o ${output}
                           MAIN_DEPENDENCY ${interface}
                           DEPENDS xmltokross/duchaintokross )
        set(mocFile ${weFile}.moc)
        qt4_generate_moc( ${output} ${CMAKE_CURRENT_BINARY_DIR}/${mocFile})
    endmacro(kdev_create_kross_iface)

    kdev_create_kross_iface(${CMAKE_SOURCE_DIR}/interfaces/idocument.h
        ${CMAKE_SOURCE_DIR}/kross/wrappers/krossidocument.cpp interfaces/idocument.h)
    kdev_create_kross_iface(${CMAKE_SOURCE_DIR}/interfaces/iuicontroller.h
        ${CMAKE_SOURCE_DIR}/kross/wrappers/krossiuicontroller.cpp interfaces/iuicontroller.h)
    kdev_create_kross_iface(${CMAKE_SOURCE_DIR}/interfaces/context.h
        ${CMAKE_SOURCE_DIR}/kross/wrappers/krosscontext.cpp interfaces/context.h)
    kdev_create_kross_iface(${CMAKE_SOURCE_DIR}/interfaces/contextmenuextension.h
        ${CMAKE_SOURCE_DIR}/kross/wrappers/krosscontextmenuextension.cpp interfaces/contextmenuextension.h)
    kdev_create_kross_iface(${CMAKE_SOURCE_DIR}/project/projectmodel.h
        ${CMAKE_SOURCE_DIR}/kross/wrappers/krossprojectmodel.cpp project/projectmodel.h)
    kdev_create_kross_iface(${CMAKE_SOURCE_DIR}/vcs/vcsrevision.h
        ${CMAKE_SOURCE_DIR}/kross/wrappers/krossvcsrevision.cpp vcs/vcsrevision.h)
    kdev_create_kross_iface(${CMAKE_SOURCE_DIR}/vcs/vcslocation.h
        ${CMAKE_SOURCE_DIR}/kross/wrappers/krossvcslocation.cpp vcs/vcslocation.h)
    kdev_create_kross_iface(${CMAKE_SOURCE_DIR}/interfaces/ilanguage.h
        ${CMAKE_SOURCE_DIR}/kross/wrappers/krossilanguage.cpp interfaces/ilanguage.h)
    kdev_create_kross_iface(${CMAKE_SOURCE_DIR}/language/duchain/topducontext.h
        ${CMAKE_SOURCE_DIR}/kross/wrappers/krosstopducontext.cpp
        language/duchain/topducontext.h:language/duchain/parsingenvironment.h:language/interfaces/iproblem.h:language/editor/simplecursor.h)
    kdev_create_kross_iface(${CMAKE_SOURCE_DIR}/language/duchain/ducontext.h
        ${CMAKE_SOURCE_DIR}/kross/wrappers/krossducontext.cpp language/duchain/ducontext.h)
    kdev_create_kross_iface(${CMAKE_SOURCE_DIR}/language/duchain/declaration.h
        ${CMAKE_SOURCE_DIR}/kross/wrappers/krossdeclaration.cpp language/duchain/declaration.h:language/duchain/declarationid.h:language/duchain/types/indexedtype.h)
    kdev_create_kross_iface(${CMAKE_SOURCE_DIR}/language/interfaces/iproblem.h
        ${CMAKE_SOURCE_DIR}/kross/wrappers/krossiproblem.cpp language/interfaces/iproblem.h)
    kdev_create_kross_iface(${CMAKE_SOURCE_DIR}/language/duchain/use.h
        ${CMAKE_SOURCE_DIR}/kross/wrappers/krossuse.cpp language/duchain/use.h)
    kdev_create_kross_iface(${CMAKE_SOURCE_DIR}/language/duchain/symboltable.h
        ${CMAKE_SOURCE_DIR}/kross/wrappers/krosssymboltable.cpp language/duchain/symboltable.h)
    kdev_create_kross_iface(${CMAKE_SOURCE_DIR}/language/duchain/identifier.h
        ${CMAKE_SOURCE_DIR}/kross/wrappers/krossidentifier.cpp language/duchain/identifier.h:language/duchain/indexedstring.h)
    kdev_create_kross_iface(${CMAKE_SOURCE_DIR}/language/duchain/duchainlock.h
        ${CMAKE_SOURCE_DIR}/kross/wrappers/krossduchainlock.cpp language/duchain/duchainlock.h)
    kdev_create_kross_iface(${CMAKE_SOURCE_DIR}/project/interfaces/ibuildsystemmanager.h
        ${CMAKE_SOURCE_DIR}/kross/wrappers/krossibuildsystemmanager.cpp project/interfaces/ibuildsystemmanager.h)
    kdev_create_kross_iface(${CMAKE_SOURCE_DIR}/project/interfaces/iprojectfilemanager.h
        ${CMAKE_SOURCE_DIR}/kross/wrappers/krossiprojectfilemanager.cpp project/interfaces/iprojectfilemanager.h)
    kdev_create_kross_iface(${CMAKE_SOURCE_DIR}/project/interfaces/iprojectbuilder.h
        ${CMAKE_SOURCE_DIR}/kross/wrappers/krossiprojectbuilder.cpp project/interfaces/iprojectbuilder.h)
    kdev_create_kross_iface(${CMAKE_SOURCE_DIR}/interfaces/irun.h
        ${CMAKE_SOURCE_DIR}/kross/wrappers/krossirun.cpp interfaces/irun.h)




else(GENERATE_INTERFACES)
    message(STATUS "The used interfaces will be the ones from svn instead of generating them.")
endif(GENERATE_INTERFACES)

set(kdevkrossplugin_SRCS
    krossplugin.cpp
    krossbuildsystemmanager.cpp
    krossdistributedversioncontrol.cpp
    krosstoolviewfactory.cpp
    krossvcsjob.cpp
    wrappers/krossiuicontroller.cpp
    wrappers/krossidocument.cpp
    wrappers/krosscontext.cpp
    wrappers/krosscontextmenuextension.cpp
    wrappers/krossvcsrevision.cpp
    wrappers/krossvcslocation.cpp
    wrappers/krosstopducontext.cpp
    wrappers/krossducontext.cpp
    wrappers/krossprojectmodel.cpp
    wrappers/krossilanguage.cpp
    wrappers/krossdeclaration.cpp
    wrappers/krossuse.cpp
    wrappers/krossidentifier.cpp
    wrappers/krossduchainlock.cpp
    wrappers/krossiproblem.cpp
    wrappers/krossibuildsystemmanager.cpp
    wrappers/krossiprojectfilemanager.cpp
    wrappers/krossiprojectbuilder.cpp
    wrappers/krossirun.cpp
)

kde4_add_plugin(kdevkrossplugin ${kdevkrossplugin_SRCS})
target_link_libraries(kdevkrossplugin ${KDE4_KDEUI_LIBS} ${KDE4_KTEXTEDITOR_LIBS}
    kdevplatforminterfaces kdevplatformproject kdevplatformvcs kdevplatformlanguage ${KDE4_KROSSUI_LIBS})

install(TARGETS kdevkrossplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kdevkrossplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})

