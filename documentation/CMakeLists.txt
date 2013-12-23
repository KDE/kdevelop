macro_log_feature(QT_QTHELP_FOUND "QtHelp" "The Help module for the Qt toolkit, needed for the qthelp plugin" "http://qt-project.org/" FALSE)

if(QT_QTHELP_FOUND)
 macro_optional_add_subdirectory(qthelp)
endif()
macro_optional_add_subdirectory(manpage)
