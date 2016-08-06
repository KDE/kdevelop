add_subdirectory(tests)

install(FILES helpers.py qt.py
    DESTINATION ${KDE_INSTALL_DATADIR}/kdevlldb/formatters)
