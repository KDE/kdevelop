set(printers_destination "${KDE_INSTALL_DATADIR}/kdevgdb/printers")
install(FILES helper.py qt.py kde.py gdbinit DESTINATION "${printers_destination}")
install(DIRECTORY qtcreator_debugger DESTINATION "${printers_destination}"
        PATTERN "__pycache__" EXCLUDE)
