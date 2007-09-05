qtopia_project(qtopia app) # see buildsystem.html for more project keywords
TARGET=%{APPNAMELC}
CONFIG+=qtopia_main
CONFIG+=no_singleexec
CONFIG+=no_quicklaunch
CONFIG+=no_tr

FORMS=%{APPNAMELC}base.ui
HEADERS=%{APPNAMELC}.h
SOURCES=main.cpp %{APPNAMELC}.cpp

desktop.files=%{APPNAMELC}.desktop
desktop.path=/apps/Applications
desktop.hint=desktop

pics.files=pics/*
pics.path=/pics/%{APPNAMELC}
pics.hint=pics

help.source=help
help.files=%{APPNAME}.html
help.hint=help

INSTALLS+=desktop pics help

pkg.name=%{APPNAME}
pkg.desc=%{APPNAME} Application
pkg.version=1.0.0-1
pkg.maintainer=%{AUTHOR} %{EMAIL}
pkg.license=GPL
pkg.domain=window
