# Here resides the C/C++ support part.

SUBDIRS = templates

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/sourceinfo -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevcppsupport.la
libkdevcppsupport_la_LDFLAGS = $(LEXLIB) $(all_libraries) $(KDE_PLUGIN)
libkdevcppsupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevcppsupport_la_SOURCES = cppsupportpart.cpp \
			       cppsupportwidget.cpp \
			       classparser.cpp \
			       tokenizer.cpp \
	                       ccconfigwidget.cpp \
	                       ccconfigwidgetbase.ui \
			       cppnewclassdlgbase.ui \
			       cppnewclassdlg.cpp \
			       cppaddmethoddlg.cpp \
			       cppaddmethoddlgbase.ui \
			       cclonefunctiondlg.cpp \
			       addclassattributedlg.cpp \
			       cppcodecompletion.cpp \
			       cppccparser.cpp \
			       parsedvariable.cpp \
			       scope.cpp \
			       simpleparser.cpp \
			       kdevregexp.cpp \
			       problemreporter.cpp \
			       driver.cpp \
			       errors.cpp \
			       lexer.cpp \
			       parser.cpp \
			       ast.cpp \
			       configproblemreporter.ui \
			       backgroundparser.cpp 

EXTRA_DIST = tokenizer.l tokenizer-cc.l
DISTCLEANFILES = tokenizer.cpp

tokenizer.cpp: tokenizer.l
	flex -+ -otokenizer.cpp $(srcdir)/tokenizer.l && cat tokenizer.cpp | \
	sed -e "s,class istream;,#include <iostream\.h>," > tokenizer.cpp.tmp && \
	mv tokenizer.cpp.tmp tokenizer.cpp

tokenizer-cc.cpp: tokenizer.l
	flex -+ -otokenizer-cc.cpp $(srcdir)/tokenizer-cc.l && cat tokenizer-cc.cpp | \
	sed -e "s,class istream;,#include <iostream\.h>," > tokenizer-cc.cpp.tmp && \
	mv tokenizer-cc.cpp.tmp tokenizer-cc.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevcppsupport.desktop kdevcsupport.desktop

rcdir = $(kde_datadir)/kdevcppsupport
rc_DATA = kdevcppsupport.rc

templatedir = ${kde_datadir}/kdevabbrev/templates
template_DATA = cpptemplates
