EXTRA_DIST = tip.database

SUBDIRS =

install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/fr/kdevelop/
	$(INSTALL_DATA) tip.database $(kde_htmldir)/fr/kdevelop/tip.database

uninstall-local:
	-rm -f $(kde_htmldir)/fr/kdevelop/tip.database



