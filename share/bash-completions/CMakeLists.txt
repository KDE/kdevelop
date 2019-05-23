include(../../cmake/modules/BashCompletion.cmake)

install(FILES kdevelop_completion.bash DESTINATION "${BASH_COMPLETION_COMPLETIONSDIR}" RENAME kdevelop)
