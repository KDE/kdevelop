appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
templatedir = ${appwizarddatadir}/templates
gtk2mmappdir = ${appwizarddatadir}/template-gtk2mmapp

gtk2mmapp_DATA = acconfig.h       autogen.sh    gtk2mm.glade   main_window.cc        main_window_glade.hh \
	Makefile.am  app-Makefile.am  configure.in  gtk2mm.gladep  main_window.hh \
	app.kdevelop     gtk2mm.cc    gtk2mmapp      main_window_glade.cc  src-Makefile.am config.h gtk2mm.png

template_DATA = gtk2mmapp

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(gtk2mmappdir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(gtk2mmappdir)/script
