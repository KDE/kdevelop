
project(KDevVcsCommonPlugin)

add_definitions( ${KDE4_ENABLE_EXCEPTIONS} -DKDE_DEFAULT_DEBUG_AREA=9528 )



########### next target ###############

set(kdevcscommonplugin_PART_SRCS
    kdevvcscommonplugin.cpp
)

kde4_add_plugin(kdevvcscommon ${kdevcscommonplugin_PART_SRCS} )

target_link_libraries(kdevvcscommon
                    ${QT_QTDESIGNER_LIBRARY}
                    ${KDE4_KDEUI_LIBS}
                    ${KDE4_KIO_LIBS}
                    ${KDE4_KTEXTEDITOR_LIBS}
                    kdevplatforminterfaces
                    kdevplatformvcs
                    kdevplatformlanguage
                    kdevplatformproject
                    ${KDE4_KPARTS_LIBRARY}
                    )

install(TARGETS kdevvcscommon DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############
install( FILES kdevvcscommon.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
