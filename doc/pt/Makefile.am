EXTRA_DIST = tip.database

SUBDIRS = manual programming

install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/pt/kdevelop/
	$(INSTALL_DATA) $(srcdir)/tip.database $(kde_htmldir)/pt/kdevelop/tip.database

uninstall-local:
	-rm -f $(kde_htmldir)/pt/kdevelop/tip.database

