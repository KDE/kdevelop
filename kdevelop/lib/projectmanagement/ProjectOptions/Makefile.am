# set the include path for X, qt and KDE
INCLUDES= $(all_includes) -I$(top_srcdir)/kdevelop/lib/general -I$(top_srcdir)/kdevelop/lib/projectmanagement/KDevCompiler \
													-I$(top_srcdir)/kdevelop/lib/projectmanagement

noinst_LTLIBRARIES = libKDevProjectOptions.la

libKDevProjectOptions_la_SOURCES =  ProjectOptionsDlg.cpp ProjectOptionsDlgBase.ui GeneralPage.ui NotImplementedPage.ui \
																		DefaultPage.ui

noinst_HEADERS = ProjectOptionsDlgBase.h GeneralPage.h NotImplementedPage.h DefaultPage.h

METASOURCES = AUTO
