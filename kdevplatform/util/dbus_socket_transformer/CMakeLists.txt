
check_cxx_source_compiles("
#include <sys/types.h>
#include <sys/socket.h>
int main() {
    int fd;
    char b[2];
    send(fd, b, 2, MSG_NOSIGNAL);
    return 0;
}
" HAVE_MSG_NOSIGNAL)

if(HAVE_MSG_NOSIGNAL)
    add_definitions(-DHAVE_MSG_NOSIGNAL=1)
endif()

add_executable(kdev_dbus_socket_transformer main.cpp)

if (HAIKU)
    target_link_libraries(kdev_dbus_socket_transformer network)
endif ()

ecm_mark_nongui_executable(kdev_dbus_socket_transformer)
install(TARGETS kdev_dbus_socket_transformer ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )
