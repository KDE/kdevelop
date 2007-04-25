
include_directories(
	${CMAKE_SOURCE_DIR}/lib
	${CMAKE_SOURCE_DIR}/lib/interfaces
	${CMAKE_SOURCE_DIR}/plugins/teamwork/lib
    )

add_subdirectory(lib)

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

set (
    kdevteamwork_UI
    kdevteamwork_editpatch.ui
    kdevteamwork_managepatches.ui
    kdevteamwork_filecollaborationsession.ui
    kdevteamwork_indocumentcontextlines.ui
    kdevteamwork_messagehistory.ui
    kdevteamwork_indocumentmessagebuttons.ui
    kdevteamwork_textmessage.ui
    kdevteamwork_indocumentmessagestate.ui
    kdevteamwork_timestampwidget.ui
    kdevteamwork_interface.ui
    kdevteamwork_userinfo.ui
    kdevteamwork_internalmessage.ui
    kdevteamwork_vectortimestampwidget.ui
    kdevteamwork_list.ui
)


#ADD_DEFINITIONS( -fexceptions -Wall -fno-inline)
ADD_DEFINITIONS(-fexceptions -Wall)
SET(CMAKE_BUILD_TYPE release)

kde4_automoc(${kdevteamwork_PART_SRCS})
kde4_add_ui_files(kdevteamwork_PART_SRCS ${kdevteamwork_UI} )
kde4_add_plugin(kdevteamwork ${kdevteamwork_PART_SRCS})

target_link_libraries(kdevteamwork ${KDE4_KDEUI_LIBS} ${KDE4_KHTML_LIBS} kdevplatforminterfaces network dynamictext diff2)

#kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdevteamwork)
install(TARGETS kdevteamwork DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES kdevteamwork.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevteamwork.rc DESTINATION ${DATA_INSTALL_DIR}/kdevteamwork )


