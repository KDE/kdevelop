SUBDIRS( templates subclassing_template newclass_templates file_templates  	app_templates compiler pcsimporter debugger doc )

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/languages/lib/designer_integration ${CMAKE_SOURCE_DIR}/languages/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/catalog ${CMAKE_SOURCE_DIR}/lib/compat ${CMAKE_SOURCE_DIR}/lib/cppparser ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/interfaces/external ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/bdb/include ${CMAKE_SOURCE_DIR})

SET(kdevcppsupport_PART_SRCS
cppsupportpart.cpp
ccconfigwidget.cpp
kdevdriver.cpp
cppcodecompletion.cpp
problemreporter.cpp
backgroundparser.cpp
ast_utils.cpp
store_walker.cpp
KDevCppSupportIface.cpp
cppsupportfactory.cpp
tag_creator.cpp
codeinformationrepository.cpp
doxydoc.cpp
cppcodecompletionconfig.cpp
cppnewclassdlg.cpp
classgeneratorconfig.cpp
subclassingdlg.cpp
addattributedialog.cpp
addmethoddialog.cpp
cppsupport_utils.cpp
createpcsdialog.cpp
cppimplementationwidget.cpp
qtdesignercppintegration.cpp
creategettersetterconfiguration.cpp
creategettersetterconfiguration.h
creategettersetterdialog.cpp
creategettersetterdialog.h
storeconverter.cpp
)

SET( kdevcppsupport_UI
ccconfigwidgetbase.ui
configproblemreporter.ui
cppnewclassdlgbase.ui
classgeneratorconfigbase.ui
subclassingdlgbase.ui
addattributedialogbase.ui
addmethoddialogbase.ui
createpcsdialogbase.ui
creategettersetter.ui
)

SET( kdevcppsupport_DCOP_SRCS
KDevCppSupportIface.h
)

KDE_AUTOMOC(${kdevcppsupport_PART_SRCS})

KDE_ADD_UI_FILES(kdevcppsupport_PART_SRCS ${kdevcppsupport_UI} )

KDE_ADD_DCOP_SKELS(kdevcppsupport_PART_SRCS ${kdevcppsupport_DCOP_SRCS})

ADD_LIBRARY(kdevcppsupport SHARED ${kdevcppsupport_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevcppsupport ${KDEV_LANG_LIBS} DCOP kdevcatalog kdevcppparser lang_interfaces designerintegration)


SET(rpp_SRCS main.cpp)

ADD_EXECUTABLE(r++ ${rpp_SRCS})

TARGET_LINK_LIBRARIES(r++ ${QT_LIBRARIES} kdecore kdevcppsupport kdevcatalog kdevcppparser ${CMAKE_BINARY_DIR}/lib/libkdevelop.so)

INSTALL_TARGETS(/bin r++)

#original Makefile.am contents follow:

## Here resides the C/C++ support part.
#
#SUBDIRS = templates subclassing_template newclass_templates file_templates \
#	app_templates compiler pcsimporter debugger doc
#
#INCLUDES = -I$(top_srcdir)/languages/lib/designer_integration \
#	-I$(top_srcdir)/languages/lib/interfaces -I$(top_srcdir)/lib/catalog -I$(top_srcdir)/lib/compat \
#	-I$(top_srcdir)/lib/cppparser -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util \
#	-I$(top_builddir)/languages/lib/designer_integration -I$(top_builddir)/bdb/build_unix -I$(top_srcdir)/bdb/include \
#	$(all_includes)
#
#kde_module_LTLIBRARIES = libkdevcppsupport.la
#libkdevcppsupport_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevcppsupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
#	$(top_builddir)/lib/catalog/libkdevcatalog.la $(top_builddir)/lib/cppparser/libkdevcppparser.la \
#	$(top_builddir)/languages/lib/interfaces/liblang_interfaces.la \
#	$(top_builddir)/languages/lib/designer_integration/libdesignerintegration.la
#
#libkdevcppsupport_la_SOURCES = cppsupportpart.cpp ccconfigwidget.cpp \
#			ccconfigwidgetbase.ui kdevdriver.cpp cppcodecompletion.cpp problemreporter.cpp \
#			configproblemreporter.ui backgroundparser.cpp ast_utils.cpp store_walker.cpp \
#			KDevCppSupportIface.cpp KDevCppSupportIface.skel cppsupportfactory.cpp tag_creator.cpp \
#			codeinformationrepository.cpp doxydoc.cpp cppcodecompletionconfig.cpp cppnewclassdlgbase.ui \
#			cppnewclassdlg.cpp classgeneratorconfig.cpp classgeneratorconfigbase.ui subclassingdlg.cpp \
#			subclassingdlgbase.ui addattributedialogbase.ui addmethoddialogbase.ui addattributedialog.cpp \
#			addmethoddialog.cpp cppsupport_utils.cpp createpcsdialogbase.ui createpcsdialog.cpp \
#			cppimplementationwidget.cpp qtdesignercppintegration.cpp creategettersetterconfiguration.cpp \
#		creategettersetterconfiguration.h creategettersetterdialog.cpp creategettersetterdialog.h \
#		creategettersetter.ui storeconverter.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevcppsupport.desktop kdevcsupport.desktop
#
#rcdir = $(kde_datadir)/kdevcppsupport
#rc_DATA = kdevcppsupport.rc
#
#templatedir = ${kde_datadir}/kdevabbrev/templates
#template_DATA = cpptemplates
#
#configurationdir = ${kde_datadir}/kdevcppsupport
#configuration_DATA = configuration
#
#AM_CXXFLAGS = -DAST_DEBUG
#
#bin_PROGRAMS = r++
#r___LDFLAGS = $(all_libraries)
#r___SOURCES = main.cpp
#r___DEPENDENCIES = libkdevcppsupport.la
#r___LDADD = libkdevcppsupport.la $(top_builddir)/lib/catalog/libkdevcatalog.la $(top_builddir)/lib/cppparser/libkdevcppparser.la
#noinst_HEADERS = cppimplementationwidget.h qtdesignercppintegration.h \
#	storeconverter.h
