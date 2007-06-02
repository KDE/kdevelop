add_subdirectory(sublime)
add_subdirectory(interfaces)
add_subdirectory(project)
add_subdirectory(editor)
add_subdirectory(language)
add_subdirectory(shell)
add_subdirectory(persistenthash)
add_subdirectory(plugins)
add_subdirectory(util)

# Needed by external plugins like Ruby and Python
install(FILES
  kdevast.h
DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/)
