EXTRA_DIST = tip.database

SUBDIRS =

install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/pl/kdevelop/
	$(INSTALL_DATA) tip.database $(kde_htmldir)/pl/kdevelop/tip.database

uninstall-local:
	-rm -f $(kde_htmldir)/pl/kdevelop/tip.database
