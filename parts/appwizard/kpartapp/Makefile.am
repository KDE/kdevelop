gideondatadir = ${kde_datadir}/gideon
commondatadir = ${gideondatadir}/template-common

kpartappdatadir = ${kde_datadir}/gideon/template-kpartapp
kpartappdata_DATA = src-Makefile.am app.cpp app.h app_part.cpp app_part.h main.cpp \
		    app_part.rc app_shell.rc

templatedatadir = ${gideondatadir}/templates
templatedata_DATA = kpartapp

BUILT_SOURCES = script.local

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local:
	$(INSTALL_DATA) $(srcdir)/script.local $(kpartappdatadir)/script
