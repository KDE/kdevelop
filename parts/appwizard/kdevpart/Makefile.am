gideondatadir = ${kde_datadir}/gideon
commondatadir = ${gideondatadir}/template-common

kpartplugindatadir = ${gideondatadir}/template-kdevpart
kpartplugindata_DATA = src-Makefile.am kdevpart_part.cpp kdevpart_part.h \
		kdevpart_factory.h kdevpart_factory.cpp kdevpart_widget.h kdevpart_widget.cpp \
		kdevpart_part.rc kdevpart.desktop

templatedatadir = ${gideondatadir}/templates
templatedata_DATA = kdevpart

BUILT_SOURCES = script.local

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local:
	$(INSTALL_DATA) $(srcdir)/script.local $(kpartplugindatadir)/script
