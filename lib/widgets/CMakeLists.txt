SUBDIRS(propeditor)

SET(kdevwidgets_SRCS 
flagboxes.cpp 
qcomboview.cpp
klistviewaction.cpp
kcomboview.cpp
resizablecombo.cpp
kdevhtmlpart.cpp
processlinemaker.cpp
processwidget.cpp
ksavealldialog.cpp
)

KDE_AUTOMOC(${kdevwidgets_SRCS})

ADD_LIBRARY(kdevwidgets SHARED ${kdevwidgets_SRCS})
