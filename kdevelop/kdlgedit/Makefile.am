####### kdevelop will overwrite this part!!! (begin)##########


EXTRA_DIST = kdlgdialogs.cpp kdlgdialogs.h kdlgeditwidget.cpp kdlgeditwidget.h kdlgpropwidget.cpp kdlgpropwidget.h kdlgwidgets.cpp kdlgwidgets.h kdlgedit.cpp kdlgedit.h kdlgnewwidget.cpp kdlgnewwidget.h 

####### kdevelop will overwrite this part!!! (end)############
# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

noinst_LIBRARIES = libkdlgedit.a

# libkdlgedit_a_METASOURCES =  kdlgdialogs.moc.cpp kdlgedit.moc.cpp kdlgeditwidget.moc.cpp kdlgnewwidget.moc.cpp kdlgproplvis.moc.cpp kdlgpropwidget.moc.cpp kdlgwidgets.moc.cpp 
libkdlgedit_a_METASOURCES =  kdlgdialogs.moc.cpp kdlgedit.moc.cpp kdlgeditwidget.moc.cpp kdlgnewwidget.moc.cpp kdlgproplvis.moc.cpp kdlgpropwidget.moc.cpp kdlgwidgets.moc.cpp 

libkdlgedit_a_SOURCES = kdlgedit.cpp kdlgeditwidget.cpp kdlgpropwidget.cpp kdlgwidgets.cpp kdlgdialogs.cpp kdlgnewwidget.cpp\
                       kdlgedit.moc.cpp kdlgeditwidget.moc.cpp kdlgpropwidget.moc.cpp kdlgwidgets.moc.cpp kdlgdialogs.moc.cpp kdlgnewwidget.moc.cpp kdlgproplvis.moc.cpp

noinst_HEADERS = kdlgedit.h kdlgeditwidget.h kdlgpropwidget.h kdlgwidgets.h kdlgdialogs.h kdlgnewwidget.h kdlgproplvis.h





kdlgdialogs.moc.cpp: kdlgdialogs.h
	$(MOC) kdlgdialogs.h -o kdlgdialogs.moc.cpp

kdlgedit.moc.cpp: kdlgedit.h
	$(MOC) kdlgedit.h -o kdlgedit.moc.cpp

kdlgeditwidget.moc.cpp: kdlgeditwidget.h
	$(MOC) kdlgeditwidget.h -o kdlgeditwidget.moc.cpp

kdlgnewwidget.moc.cpp: kdlgnewwidget.h
	$(MOC) kdlgnewwidget.h -o kdlgnewwidget.moc.cpp

kdlgproplvis.moc.cpp: kdlgproplvis.h
	$(MOC) kdlgproplvis.h -o kdlgproplvis.moc.cpp

kdlgpropwidget.moc.cpp: kdlgpropwidget.h
	$(MOC) kdlgpropwidget.h -o kdlgpropwidget.moc.cpp

kdlgwidgets.moc.cpp: kdlgwidgets.h
	$(MOC) kdlgwidgets.h -o kdlgwidgets.moc.cpp

