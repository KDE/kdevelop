####### kdevelop will overwrite this part!!! (begin)##########

INCLUDES = $(all_includes)

libkpp_a_METASOURCES = AUTO

noinst_LIBRARIES = libkpp.a

libkpp_a_SOURCES = cdevproject.cpp projectView.ui prefView.ui kpp.cpp cspec.cpp 


EXTRA_DIST = README buildrpm.sh cspec.cpp cspec.h kpp.cpp kpp.h prefView.ui projectView.ui template.spec cdevproject.cpp cdevproject.h template-kde.spec template-make.spec 

####### kdevelop will overwrite this part!!! (end)############

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/kdevelop/tools/
	$(INSTALL_DATA) buildrpm.sh $(kde_datadir)/kdevelop/tools/buildrpm.sh
	$(mkinstalldirs) $(kde_datadir)/kdevelop/
	$(INSTALL_DATA) template.spec $(kde_datadir)/kdevelop/template.spec
	$(mkinstalldirs) 
	$(INSTALL_DATA) template-make.spec blah
	$(mkinstalldirs) $(datadir)/kdevelop/
	$(INSTALL_DATA) template-kde.spec $(datadir)/kdevelop/templates

uninstall-local:
	-rm -f $(kde_datadir)/kdevelop/tools/buildrpm.sh
	-rm -f $(kde_datadir)/kdevelop/template.spec
	-rm -f blah
	-rm -f $(datadir)/kdevelop/templates

