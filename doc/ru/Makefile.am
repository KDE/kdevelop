SUBDIRS = welcome manual tutorial programming cref

EXTRA_DIST = tip.database

install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/ru/kdevelop/
	$(INSTALL_DATA) tip.database $(kde_htmldir)/ru/kdevelop/tip.database

uninstall-local:
	-rm -f $(kde_htmldir)/ru/kdevelop/tip.database
