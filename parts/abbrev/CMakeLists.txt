SET(kdevabbrev_SRCS abbrevpart.cpp abbrevconfigwidget.cpp addtemplatedlg.cpp )

KDE_ADD_UI_FILES(kdevabbrev_SRCS addtemplatedlgbase.ui abbrevconfigwidgetbase.ui)

KDE_AUTOMOC(${kdevabbrev_SRCS})

ADD_LIBRARY(kdevabbrev SHARED ${kdevabbrev_SRCS})
