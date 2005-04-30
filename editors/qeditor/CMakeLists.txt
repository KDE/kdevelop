SUBDIRS( data )

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/compat )

SET(qeditorpart_PART_SRCS
cindent.cpp
cpp_colorizer.cpp
cs_colorizer.cpp
java_colorizer.cpp
koFind.cpp
koReplace.cpp
levelwidget.cpp
linenumberwidget.cpp
markerwidget.cpp
paragdata.cpp
parenmatcher.cpp
python_colorizer.cpp
qeditor.cpp
qeditor_arghint.cpp
qeditor_factory.cpp
qeditor_indenter.cpp
qeditor_part.cpp
qeditor_view.cpp
qeditorcodecompletion.cpp
qmake_colorizer.cpp
qsourcecolorizer.cpp
simple_indent.cpp
xml_colorizer.cpp
yyindent.cpp
js_colorizer.cpp
jsp_colorizer.cpp
ocaml_colorizer.cpp
python_indent.cpp
qeditor_settings.cpp
qeditor_texthint.cpp
qeditor_browserextension.cpp
pascal_colorizer.cpp
pascal_indent.cpp
ada_colorizer.cpp
ada_indent.cpp
sql_colorizer.cpp
)

SET( qeditorpart_UI
generalconfigpage.ui
gotolinedialog.ui
highlightingconfigpage.ui
indentconfigpage.ui
)

KDE_AUTOMOC(${qeditorpart_PART_SRCS})

KDE_ADD_UI_FILES(qeditorpart_PART_SRCS ${qeditorpart_UI} )

ADD_LIBRARY(qeditorpart SHARED ${qeditorpart_PART_SRCS})

TARGET_LINK_LIBRARIES(qeditorpart ${QT_LIBRARIES} kdecore kdeui kio kparts ktexteditor ${CMAKE_BINARY_DIR}/lib/libkdevelop.so)


#original Makefile.am contents follow:

## this has all of the subdirectories that make will recurse into.  if
## there are none, comment this out
#SUBDIRS = data
#
## set the include path for X, qt and KDE
#INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/compat $(all_includes)
#
## these are the headers for your project
#
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/qeditor.pot
#
#kde_module_LTLIBRARIES = libqeditorpart.la
#
## the Part's source, library search path, and link libraries
#libqeditorpart_la_SOURCES = cindent.cpp cpp_colorizer.cpp cs_colorizer.cpp java_colorizer.cpp koFind.cpp koReplace.cpp levelwidget.cpp linenumberwidget.cpp markerwidget.cpp paragdata.cpp parenmatcher.cpp python_colorizer.cpp qeditor.cpp qeditor_arghint.cpp qeditor_factory.cpp qeditor_indenter.cpp qeditor_part.cpp qeditor_view.cpp qeditorcodecompletion.cpp qmake_colorizer.cpp qsourcecolorizer.cpp simple_indent.cpp xml_colorizer.cpp yyindent.cpp generalconfigpage.ui gotolinedialog.ui highlightingconfigpage.ui indentconfigpage.ui js_colorizer.cpp jsp_colorizer.cpp ocaml_colorizer.cpp python_indent.cpp qeditor_settings.cpp qeditor_texthint.cpp qeditor_browserextension.cpp pascal_colorizer.cpp pascal_indent.cpp ada_colorizer.cpp ada_indent.cpp sql_colorizer.cpp
#
#libqeditorpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#libqeditorpart_la_LIBADD = $(LIB_KPARTS) $(LIB_KFILE) -lktexteditor $(top_builddir)/lib/libkdevelop.la
#
## this is where the desktop file will go
#partdesktopdir = $(kde_servicesdir)
#partdesktop_DATA = qeditor_part.desktop
#
## this is where the part's XML-GUI resource file goes
#partrcdir = $(kde_datadir)/qeditorpart
#partrc_DATA = qeditor_part.rc
