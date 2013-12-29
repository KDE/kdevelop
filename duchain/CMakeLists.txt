kde4_add_library(kdevclangduchain SHARED
    parsesession.cpp
)

target_link_libraries(kdevclangduchain LINK_PRIVATE
    ${QT_QTCORE_LIBRARY}
)

target_link_libraries(kdevclangduchain LINK_PUBLIC
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
)

install(TARGETS kdevclangduchain ${INSTALL_TARGETS_DEFAULT_ARGS})
