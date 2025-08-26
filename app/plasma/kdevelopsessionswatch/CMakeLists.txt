set(KDevelopSessionsWatch_SRCS
    kdevelopsessionswatch.cpp
    kdevelopsessionsobserver.cpp
    sessionfilestracker.cpp
)

add_library(KDevelopSessionsWatch SHARED ${KDevelopSessionsWatch_SRCS})

generate_export_header(KDevelopSessionsWatch)

install(TARGETS KDevelopSessionsWatch ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)

target_link_libraries(KDevelopSessionsWatch
    PUBLIC
        Qt::Core
    PRIVATE
        KF6::KIOGui
)
