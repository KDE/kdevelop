# Here resides the Java support part.

SUBDIRS = org kdejava

INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/sourceinfo  $(jni_includes) $(all_includes)

kde_module_LTLIBRARIES = libkdevjavasupport.la
libkdevjavasupport_la_LDFLAGS = $(LEXLIB) $(all_libraries) $(KDE_PLUGIN) 
libkdevjavasupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la -lqtjava -lkdejava  
#libkdevjavasupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la -lqtjava -lkdejava $(JVMLIBS) 

libkdevjavasupport_la_SOURCES = javasupportfactory.cpp \
			       classparser.cpp \
			       tokenizer.cpp

EXTRA_DIST = tokenizer.l
tokenizer.cpp: tokenizer.l
	flex -+ -otokenizer.cpp $(srcdir)/tokenizer.l

java_JAVA = kdevjavapluginapi.java JavaClassParser.java \
	JavaSupportFactory.java JavaSupportPart.java \
	JavaAddClassAttributeDialog.java   JavaAddClassMethodDialog.java


javadir = $(kde_datadir)/kdevjavasupport

kdevjavasupport.jar: classjava.stamp
	$(JAR) cf kdevjavasupport.jar *.class

kdevjavapluginapi.jar: org/kde/koala/classjava.stamp
	$(JAR) cf kdevjavapluginapi.jar org/kde/koala/*.class

datadir = $(kde_datadir)/kdevjavasupport
data_DATA = kdevjavasupport.jar kdevjavapluginapi.jar

JAVAROOT = .
CLASSPATH = .:${kde_libraries}/java/qtjava.jar:${kde_libraries}/java/koala.jar
JAVACFLAGS = -classpath $(CLASSPATH)

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevjavasupport.desktop
