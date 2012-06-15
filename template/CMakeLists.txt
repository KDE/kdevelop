
set (
    TEMPLATE_FILTERS_INSTALL_ARGS
    LIBRARY DESTINATION "${LIB_INSTALL_DIR}/grantlee/${Grantlee_VERSION_MAJOR}.${Grantlee_VERSION_MINOR}"
    ARCHIVE DESTINATION "${LIB_INSTALL_DIR}/grantlee/${Grantlee_VERSION_MAJOR}.${Grantlee_VERSION_MINOR}"
    RUNTIME DESTINATION "${BIN_INSTALL_DIR}/grantlee/${Grantlee_VERSION_MAJOR}.${Grantlee_VERSION_MINOR}"
)

include (${Grantlee_USE_FILE})

add_subdirectory(filters)
add_subdirectory(templates)
