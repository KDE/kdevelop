####### kdevelop will overwrite this part!!! (begin)##########
lib_LTLIBRARIES = libkdevelopkpp.la


INCLUDES = $(all_includes)


LDFLAGS = 

libkdevelopkpp_la_LDFLAGS = -version-info 2:0

libkdevelopkpp_la_METASOURCES = AUTO

libkdevelopkpp_la_SOURCES = cdevproject.cpp projectView.ui prefView.ui kpp.cpp cspec.cpp


EXTRA_DIST = README buildrpm.sh cspec.cpp cspec.h kpp.cpp kpp.h prefView.ui projectView.ui template.spec cdevproject.cpp cdevproject.h template-kde.spec template-make.spec 

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/kdevelop/tools/
	$(INSTALL_DATA) $(srcdir)/buildrpm.sh $(kde_datadir)/kdevelop/tools/buildrpm.sh
	$(mkinstalldirs) $(kde_datadir)/kdevelop/
	$(INSTALL_DATA) $(srcdir)/template.spec $(kde_datadir)/kdevelop/template.spec
	$(mkinstalldirs) $(kde_datadir)/kdevelop/
	$(INSTALL_DATA) $(srcdir)/template-kde.spec $(kde_datadir)/kdevelop/template-kde.spec
	$(mkinstalldirs) $(kde_datadir)/kdevelop/
	$(INSTALL_DATA) $(srcdir)/template-make.spec $(kde_datadir)/kdevelop/template-make.spec

uninstall-local:
	-rm -f $(kde_datadir)/kdevelop/tools/buildrpm.sh
	-rm -f $(kde_datadir)/kdevelop/template.spec
	-rm -f $(kde_datadir)/kdevelop/template-kde.spec
	-rm -f $(kde_datadir)/kdevelop/template-make.spec

####### kdevelop will overwrite this part!!! (end)############
#SOMAJOR = 1
#SOMINOR = 0
#SOSUBMINOR = 0

# here you should add all header files, which contains the functionality
# of your shared lib (published for the users)
include_HEADERS =

# here you should add all header files, which shouldn´t be installed
noinst_HEADERS =

libkdevelopkpp_la_LDFLAGS = $(all_libraries) -no-undefined -version-info $(SOMAJOR):$(SOMINOR):$(SOSUBMINOR) $(LDFLAGS)

# add here the libraries which have to be include
# for building the shared lib
#
#  please consider also the usage of the make environment variables
#  like
#  $(LIB_KDEUI), $(LIB_QT), $(LIB_HTML), etc.
#  instead of the hardcoded entries, which have placed
#  in here.
#  The following line is only a suggestion!
#
libkdevelopkpp_la_LIBADD = -lkfile -lkhtml -lkdeui -lkdecore $(LIB_QT)

