EXTRA_DIST =tip.database

SUBDIRS = manual welcome tutorial programming kde_libref

install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/hu/kdevelop/
	$(INSTALL_DATA) $(srcdir)/tip.database $(kde_htmldir)/hu/kdevelop/tip.database

uninstall-local:
	-rm -f $(kde_htmldir)/hu/kdevelop/tip.database
