# set the include path for X, qt and KDE
INCLUDES= $(all_includes) \
					-I$(top_srcdir)/kdevelop/lib/general \
					-I$(top_srcdir)/kdevelop/lib/Build/KDevCompiler \
					-I$(top_srcdir)/kdevelop/lib/Build/KDevLinker \
					-I$(top_srcdir)/kdevelop/lib/projectmanagement \
					-I$(top_srcdir)/kdevelop/lib/ComponentManager

noinst_LTLIBRARIES = libKDevProjectOptions.la

libKDevProjectOptions_la_SOURCES =  ProjectOptionsDlg.cpp ProjectInfoWidgetBase.ui ProjectInfoWidget.cpp \
																		ProjectSpaceInfoWidgetBase.ui ProjectSpaceInfoWidget.cpp

noinst_HEADERS =	ProjectOptionsDlg.h ProjectInfoWidgetBase.h ProjectInfoWidget.h \
									ProjectSpaceInfoWidgetBase.h ProjectSpaceInfoWidget.h

METASOURCES = AUTO
