####### kdevelop will overwrite this part!!! (begin)##########
lib_LTLIBRARIES = libkdevelopkpp.la


INCLUDES = $(all_includes)


LDFLAGS =      


libkdevelopkpp_la_LDFLAGS = -version-info 2:0

libkdevelopkpp_la_METASOURCES = AUTO

libkdevelopkpp_la_SOURCES = cdevproject.cpp projectView.ui prefView.ui kpp.cpp cspec.cpp 


EXTRA_DIST = README buildrpm.sh cspec.cpp cspec.h kpp.cpp kpp.h prefView.ui projectView.ui template.spec cdevproject.cpp cdevproject.h template-kde.spec template-make.spec 

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(kde_datadir)/kdevelop/tools/
	$(INSTALL_DATA) $(srcdir)/buildrpm.sh $(DESTDIR)$(kde_datadir)/kdevelop/tools/buildrpm.sh
	$(mkinstalldirs) $(DESTDIR)$(kde_datadir)/kdevelop/
	$(INSTALL_DATA) $(srcdir)/template.spec $(DESTDIR)$(kde_datadir)/kdevelop/template.spec
	$(mkinstalldirs) $(DESTDIR)$(kde_datadir)/kdevelop/
	$(INSTALL_DATA) $(srcdir)/template-kde.spec $(DESTDIR)$(kde_datadir)/kdevelop/template-kde.spec
	$(mkinstalldirs) $(DESTDIR)$(kde_datadir)/kdevelop/
	$(INSTALL_DATA) $(srcdir)/template-make.spec $(DESTDIR)$(kde_datadir)/kdevelop/template-make.spec

uninstall-local:
	-rm -f $(DESTDIR)$(kde_datadir)/kdevelop/tools/buildrpm.sh
	-rm -f $(DESTDIR)$(kde_datadir)/kdevelop/template.spec
	-rm -f $(DESTDIR)$(kde_datadir)/kdevelop/template-kde.spec
	-rm -f $(DESTDIR)$(kde_datadir)/kdevelop/template-make.spec

####### kdevelop will overwrite this part!!! (end)############

KDE_OPTIONS = nofinal
