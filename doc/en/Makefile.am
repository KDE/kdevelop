EXTRA_DIST =tip.database

SUBDIRS = addendum kde_libref manual programming tutorial cref

install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/en/kdevelop/
	$(INSTALL_DATA) tip.database $(kde_htmldir)/en/kdevelop/tip.database

uninstall-local:
	-rm -f $(kde_htmldir)/en/kdevelop/tip.database
