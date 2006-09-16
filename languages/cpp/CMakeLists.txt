
add_subdirectory(parser)
add_subdirectory(duchain)
#add_subdirectory(app_templates)

include_directories( ${CMAKE_TOP_BUILD_DIR}/ ${CMAKE_SOURCE_DIR}/lib ${THREADWEAVER_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/lib/editor ${CMAKE_SOURCE_DIR}/languages/cpp/duchain ${CMAKE_SOURCE_DIR}/languages/cpp/parser ${CMAKE_SOURCE_DIR}/languages/cpp/duchain/viewer ${CMAKE_SOURCE_DIR}/lib/project
    )


########### next target ###############

set(kdevcpplanguage_LIB_SRCS
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
    duchain/forwarddeclaration.cpp
    duchain/duchainbase.cpp
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

kde4_automoc(${kdevcpplanguage_LIB_SRCS})
kde4_add_library(kdevcpplanguage SHARED ${kdevcpplanguage_LIB_SRCS})
target_link_libraries(kdevcpplanguage ${KDE4_KDECORE_LIBS} kdevplatform kdevcppparser kdevcpprpp )
set_target_properties(kdevcpplanguage PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS kdevcpplanguage DESTINATION ${LIB_INSTALL_DIR} )

kde4_automoc(${kdevcpplanguagesupport_PART_SRCS})
kde4_add_plugin(kdevcpplanguagesupport ${kdevcpplanguagesupport_PART_SRCS})
target_link_libraries(kdevcpplanguagesupport kdevcpplanguage ThreadWeaver)
kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdevcpplanguagesupport ThreadWeaver)
install(TARGETS kdevcpplanguagesupport DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevcppsupport.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


