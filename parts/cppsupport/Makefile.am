# Here resides the C/C++ support part.

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/sourceinfo $(all_includes)

kde_module_LTLIBRARIES = libkdevcppsupport.la
libkdevcppsupport_la_LDFLAGS = $(LEXLIB) $(all_libraries) $(KDE_PLUGIN)
libkdevcppsupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la $(top_builddir)/lib/interfaces/keditor/libkeditorinterface.la

libkdevcppsupport_la_SOURCES = cppsupportfactory.cpp \
			       cppsupportpart.cpp \
			       classparser.cpp \
			       tokenizer.cpp \
			       addclassmethoddlg.cpp \
			       addclassattributedlg.cpp \
			       cppcodecompletion.cpp \
			       cppcodecompletionparser.cpp \
			       ParsedVariable.cpp \
			       Scope.cpp

EXTRA_DIST = tokenizer.l
tokenizer.cpp: tokenizer.l
	flex -+ -otokenizer.cpp $(srcdir)/tokenizer.l && cat tokenizer.cpp | \
	sed -e "s,class istream;,#include <iostream\.h>," > tokenizer.cpp.tmp && \
	mv tokenizer.cpp.tmp tokenizer.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevcppsupport.desktop kdevcsupport.desktop

rcdir = $(kde_datadir)/kdevcppsupport
rc_DATA = kdevcppsupport.rc
