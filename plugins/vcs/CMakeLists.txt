add_subdirectory(interfaces)

# if(SUBVERSIONFOUND)
#     add_subdirectory(subversion)
# endif(SUBVERSIONFOUND)

install(FILES
    interfaces/iversioncontrol.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/vcs/interfaces
)
