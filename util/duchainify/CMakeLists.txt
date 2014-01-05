
add_executable(duchainify main.cpp)
target_link_libraries(duchainify ${QT_QTCORE_LIBRARY} ${KDE4_KNOTIFYCONFIG_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${KDE4_THREADWEAVER_LIBRARIES}
    ${KDEVPLATFORM_INTERFACES_LIBRARIES} ${KDEVPLATFORM_LANGUAGE_LIBRARIES}  sublime kdevplatformutil kdevplatformoutputview kdevplatformshell kdevplatformtests)

