SET(kdevpropertyeditor_LIB_SRCS
childproperty.cpp
pcombobox.cpp
pdummywidget.cpp
ppointedit.cpp
propertymachinefactory.cpp
pstringlistedit.cpp
multiproperty.cpp
pcursoredit.cpp
pfontbutton.cpp
prectedit.cpp
propertywidget.cpp
psymbolcombo.cpp
pcheckbox.cpp
pdateedit.cpp
pfontcombo.cpp
property.cpp
psizeedit.cpp
purledit.cpp
pcolorbutton.cpp
pdatetimeedit.cpp
plineedit.cpp
propertyeditor.cpp
psizepolicyedit.cpp
pyesnobutton.cpp
pcolorcombo.cpp
pdoublenuminput.cpp
ppixmapedit.cpp
propertylist.cpp
pspinbox.cpp
propertywidgetproxy.cpp
plinestyleedit.cpp
qeditlistbox.cpp
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

KDE_AUTOMOC(${kdevpropertyeditor_LIB_SRCS})
KDE_ADD_MOC_FILES(kdevpropertyeditor_LIB_SRCS qeditlistbox.h)

ADD_LIBRARY(kdevpropertyeditor SHARED ${kdevpropertyeditor_LIB_SRCS})

TARGET_LINK_LIBRARIES(kdevpropertyeditor ${QT_LIBRARIES} kdecore kdeui kio)

INSTALL_TARGETS(/lib kdevpropertyeditor)
INSTALL_FILES(/include/kdevelop/propeditor ".+\\.h")

#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/compat $(all_includes)
#METASOURCES = AUTO
#
#lib_LTLIBRARIES =  libkdevpropertyeditor.la
#libkdevpropertyeditor_la_LIBADD = $(LIB_KIO)
#libkdevpropertyeditor_la_LDFLAGS = -no-undefined $(all_libraries)
#libkdevpropertyeditor_la_SOURCES = childproperty.cpp  pcombobox.cpp        pdummywidget.cpp  ppointedit.cpp      propertymachinefactory.cpp  pstringlistedit.cpp\
#multiproperty.cpp  pcursoredit.cpp      pfontbutton.cpp   prectedit.cpp       propertywidget.cpp          psymbolcombo.cpp\
#pcheckbox.cpp      pdateedit.cpp        pfontcombo.cpp    property.cpp        psizeedit.cpp               purledit.cpp\
#pcolorbutton.cpp   pdatetimeedit.cpp    plineedit.cpp     propertyeditor.cpp  psizepolicyedit.cpp         pyesnobutton.cpp\
#pcolorcombo.cpp    pdoublenuminput.cpp  ppixmapedit.cpp   propertylist.cpp    pspinbox.cpp propertywidgetproxy.cpp plinestyleedit.cpp qeditlistbox.cpp
#
#kdevpropeditorincludedir = $(includedir)/kdevelop/propeditor
#kdevpropeditorinclude_HEADERS = childproperty.h pcombobox.h pdummywidget.h ppointedit.h propertymachinefactory.h pcursoredit.h pfontbutton.h prectedit.h propertywidget.h pdateedit.h pfontcombo.h property.h psizeedit.h pdatetimeedit.h plineedit.h propertyeditor.h psizepolicyedit.h pdoublenuminput.h ppixmapedit.h propertylist.h pspinbox.h propertywidgetproxy.h multiproperty.h pyesnobutton.h purledit.h psymbolcombo.h pstringlistedit.h pcolorcombo.h pcolorbutton.h pcheckbox.h plinestyleedit.h
#
#DOXYGEN_REFERENCES = dcop interfaces kdecore kdefx kdeui khtml kmdi kio kjs kparts kutils
#DOXYGEN_PROJECTNAME = KDevelop Property Editing Library
#include ../../../Doxyfile.am
