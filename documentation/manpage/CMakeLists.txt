
set(kdevmanpage_SRCS
    manpagedocumentation.cpp
    manpageplugin.cpp
    manpagemodel.cpp
    manpagedocumentationwidget.cpp
)

add_library(kdevmanpage MODULE ${kdevmanpage_SRCS})
target_link_libraries(kdevmanpage ${KDE4_KDEUI_LIBS} KF5::TextEditor ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
${KDEVPLATFORM_DOCUMENTATION_LIBRARIES} ${KDEVPLATFORM_INTERFACES_LIBRARIES} ${QT_QTWEBKIT_LIBRARY} Qt5::Gui)

install(TARGETS kdevmanpage DESTINATION ${PLUGIN_INSTALL_DIR})
configure_file(kdevmanpage.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevmanpage.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevmanpage.desktop DESTINATION ${SERVICES_INSTALL_DIR})
