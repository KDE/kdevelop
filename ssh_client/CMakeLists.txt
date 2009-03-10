project (kssh_client)
find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include_directories(${KDE4_INCLUDES})
set(kssh_client_SRCS main.cpp)
kde4_add_executable(kssh_client ${kssh_client_SRCS})
target_link_libraries(kssh_client ${KDE4_KDEUI_LIBS} ssh2)
install(TARGETS kssh_client  ${INSTALL_TARGETS_DEFAULT_ARGS})

