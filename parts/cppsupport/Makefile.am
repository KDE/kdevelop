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
			       parsedvariable.cpp \
			       scope.cpp

EXTRA_DIST = tokenizer.l tokenizer-cc.l
tokenizer.cpp: tokenizer.l
	flex -+ -otokenizer.cpp $(srcdir)/tokenizer.l && cat tokenizer.cpp | \
	sed -e "s,class istream;,#include <iostream\.h>," > tokenizer.cpp.tmp && \
	mv tokenizer.cpp.tmp tokenizer.cpp

tokenizer-cc.cpp: tokenizer.l
	lfex -+ -otokenizer-cc.cpp $(srcdir)/tokenizer-cc.l && cat tokenizer-cc.cpp | \
	sed -e "s,class istream;,#include <iostream\.h>," > tokenizer-cc.cpp.tmp && \
	mv tokenizer-cc.cpp.tmp tokenizer-cc.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevcppsupport.desktop kdevcsupport.desktop

rcdir = $(kde_datadir)/kdevcppsupport
rc_DATA = kdevcppsupport.rc
