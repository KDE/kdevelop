include(../../cmake/modules/BashCompletion.cmake)

if (BASH_COMPLETION_FOUND)
    install(FILES kdevelop_completion.bash DESTINATION "${BASH_COMPLETION_COMPLETIONSDIR}" RENAME kdevelop)
endif()
