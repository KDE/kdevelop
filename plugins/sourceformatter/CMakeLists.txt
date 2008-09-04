
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9500 )

########### shared lib ################

set(kdevsourceformatterinterface_LIB_SRCS
    sourceformattermanager.cpp
    isourceformatter.cpp
)

kde4_add_library(kdevsourceformatterinterface SHARED ${kdevsourceformatterinterface_LIB_SRCS})
target_link_libraries(kdevsourceformatterinterface
    ${KDE4_KDEUI_LIBS}
    kdevplatforminterfaces kdevplatformlanguage
    kdevplatformproject)

install(TARGETS kdevsourceformatterinterface DESTINATION ${LIB_INSTALL_DIR})

########### plugin ###############

set(kdevsourceformatter_SRCS
    sourceformatterplugin.cpp
)

kde4_add_plugin(kdevsourceformatter ${kdevsourceformatter_SRCS})
target_link_libraries(kdevsourceformatter
    ${KDE4_KDEUI_LIBS} ${KDE4_KTEXTEDITOR_LIBS}
    ${KDE4_KPARTS_LIBS} kdevplatforminterfaces kdevplatformlanguage
    kdevplatformproject kdevsourceformatterinterface)

install(TARGETS kdevsourceformatter DESTINATION ${PLUGIN_INSTALL_DIR})

########## KCModule ###################

set(kcm_kdevsourceformattersettings_SRCS
    editstyledialog.cpp
    sourceformattersettings.cpp
)

set(kcm_kdevsourceformattersettings_UI
    editstyledialog.ui
    sourceformattersettings.ui
)

kde4_add_ui_files(kcm_kdevsourceformattersettings_SRCS ${kcm_kdevsourceformattersettings_UI} )
kde4_add_plugin(kcm_kdevsourceformattersettings ${kcm_kdevsourceformattersettings_SRCS})
target_link_libraries(kcm_kdevsourceformattersettings
    ${KDE4_KDEUI_LIBS} ${KDE4_KTEXTEDITOR_LIBS}
    ${KDE4_KPARTS_LIBS} kdevplatforminterfaces kdevplatformlanguage
    kdevplatformproject kdevsourceformatterinterface)

install(TARGETS kcm_kdevsourceformattersettings DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES kdevsourceformatter.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kcm_kdevsourceformattersettings.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(FILES kdevsourceformatter.rc DESTINATION ${DATA_INSTALL_DIR}/kdevsourceformatter)

install(FILES
    isourceformatter.h
    sourceformatter_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/sourceformatter/interfaces
)
