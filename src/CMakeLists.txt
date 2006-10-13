
add_subdirectory(profiles)
# add_subdirectory(profileengine)
# add_subdirectory(newui)

include_directories(${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib/widgets ${CMAKE_SOURCE_DIR}/lib/editor ${CMAKE_SOURCE_DIR}/lib/project
    )


########### next target ###############

set(kdevshell_LIB_SRCS
#     generalinfowidget.cpp
#     languageselectwidget.cpp
#     partselectwidget.cpp
    splashscreen.cpp
    shellexport.h
)

set(kdevshell_UI
    generalinfowidgetbase.ui
)

kde4_automoc(kdevshell ${kdevshell_LIB_SRCS})
kde4_add_ui_files(kdevshell_LIB_SRCS ${kdevshell_UI})
kde4_add_library(kdevshell SHARED ${kdevshell_LIB_SRCS})
target_link_libraries(kdevshell ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} kdevplatform kdevutil
    kdevwidgets ${KDE4_KHTML_LIBS} ktexteditor knotifyconfig)
set_target_properties(kdevshell PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS kdevshell DESTINATION ${LIB_INSTALL_DIR} )

########### next target ###############

set(kdevelop_SRCS
    main.cpp
    kdevideextension.cpp
)

kde4_automoc(kdevelop ${kdevelop_SRCS})
kde4_add_executable(kdevelop ${kdevelop_SRCS})
target_link_libraries(kdevelop ${KDE4_KDECORE_LIBS} kdevshell)
install(TARGETS kdevelop DESTINATION ${BIN_INSTALL_DIR} )


########### next target ###############

# set(kdevassistant_SRCS
#     main_assistant.cpp
#     kdevassistantextension.cpp
# )
#
# kde4_automoc(kdevassistant ${kdevassistant_SRCS})
# kde4_add_executable(kdevassistant ${kdevassistant_SRCS})
# target_link_libraries(kdevassistant ${KDE4_KDECORE_LIBS} kdevshell)
# install(TARGETS kdevassistant DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install(FILES
#     generalinfowidget.h
#     languageselectwidget.h
#     partselectwidget.h
    splashscreen.h
    shellexport.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/shell )

install( FILES kdeveloprc kdevassistantrc DESTINATION ${CONFIG_INSTALL_DIR} )
install( FILES kdevelopui.rc kdevelop.notifyrc DESTINATION ${DATA_INSTALL_DIR}/kdevelop )
install( FILES projectprofiles DESTINATION ${DATA_INSTALL_DIR}/kdevelop/profiles )
# install( FILES kdevassistantui.rc DESTINATION ${DATA_INSTALL_DIR}/kdevassistant )


