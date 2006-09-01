
add_subdirectory(parser)
add_subdirectory(duchain)
#add_subdirectory(app_templates)

include_directories( ${CMAKE_TOP_BUILD_DIR}/ ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/weaver ${CMAKE_SOURCE_DIR}/lib/editor ${CMAKE_SOURCE_DIR}/languages/cpp/duchain ${CMAKE_SOURCE_DIR}/languages/cpp/parser
    )


########### next target ###############

set(kdevcppduchain_SRCS
    duchain/cppeditorintegrator.cpp
    duchain/duchain.cpp
    duchain/contextbuilder.cpp
    duchain/declarationbuilder.cpp
    duchain/usebuilder.cpp
    duchain/ducontext.cpp
    duchain/topducontext.cpp
    duchain/declaration.cpp
    duchain/definition.cpp
    duchain/classmemberdeclaration.cpp
    duchain/classfunctiondeclaration.cpp
    duchain/use.cpp
    duchain/dumpchain.cpp
    duchain/typesystem.cpp
    duchain/typebuilder.cpp
    duchain/cpptypes.cpp
    duchain/dumptypes.cpp
    duchain/typerepository.cpp
    duchain/smartconverter.cpp
    duchain/symboltable.cpp
)

set(kdevcpplanguagesupport_PART_SRCS
    cpplanguagesupport.cpp
    parsejob.cpp
    preprocessjob.cpp
    codedelegate.cpp
    codeproxy.cpp
    cpphighlighting.cpp
    cppcodecompletion.cpp
    cppcodecompletionmodel.cpp
    ${kdevcppduchain_SRCS}
)

kde4_automoc(${kdevcpplanguagesupport_PART_SRCS})
kde4_add_plugin(kdevcpplanguagesupport ${kdevcpplanguagesupport_PART_SRCS})
target_link_libraries(kdevcpplanguagesupport ${KDE4_KDECORE_LIBS} kdevplatform kdevcppparser ThreadWeaver kdevcpprpp)
kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdevcpplanguagesupport)
install(TARGETS kdevcpplanguagesupport DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevcppsupport.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


