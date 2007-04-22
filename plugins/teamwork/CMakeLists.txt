
include_directories( ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/dynamictext ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib/widgets ${CMAKE_SOURCE_DIR}/lib/network ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/libdiff2 ${ccgnu2_includes}/cc++2
    )


########### next target ###############

set(kdevteamwork_PART_SRCS
    kdevteamwork.cpp
    kdevteamwork_part.cpp
    kdevteamwork_client.cpp
    kdevteamwork_messages.cpp
    kdevteamwork_user.cpp
    patchesmanager.cpp
    utils.cpp
    collaborationmanager.cpp
    patchmessage.cpp
    messagemanager.cpp
    conversationmanager.cpp
    guimessagehistory.cpp
    messagehistorymanager.cpp
    kdevteamwork_messageshower.cpp
    filecollaborationmessages.cpp
    filecollaborationmanager.cpp
    filecollaborationsession.cpp
    filecollaboration.cpp
    kdevutils.cpp
    documentwrapper.cpp
    indocumentmessage.cpp 
    indocumentreference.cpp 
    kdevteamwork_helpers.cpp
    messageusertab.cpp
    verify.cpp
    filesynchronizemessage.cpp
    vectortimestampeditor.cpp
    qdynamictext.cpp
    teamworkfoldermanager.cpp
    editpatch.cpp
    messagesendmanager.cpp
    safelogger.cpp
    safesignaller.cpp
)

set (kdevteamwork_UI
      kdevteamwork_interface.ui)

ADD_DEFINITIONS( -fexceptions -Wall -fno-inline)
Kde4_automoc(${kdevteamwork_PART_SRCS})
kde4_add_plugin(kdevteamwork ${kdevteamwork_PART_SRCS})
kde4_add_ui_files(kdevteamwork_PART_SRCS ${kdevteamwork_UI} )

target_link_libraries(kdevteamwork ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KHTML_LIBS} kdevplatforminterfaces network dynamictext boost_serialization boost_thread diff2)

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdevteamwork)
#install_targets(${PLUGIN_INSTALL_DIR} kdevteamwork)
install(TARGETS kdevteamwork DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES kdevteamwork.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevteamwork.rc DESTINATION ${DATA_INSTALL_DIR}/kdevteamwork )



