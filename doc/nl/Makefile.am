EXTRA_DIST =tip.database

SUBDIRS = cref

install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/nl/kdevelop/
	$(INSTALL_DATA) tip.database $(kde_htmldir)/nl/kdevelop/tip.database

uninstall-local:
	-rm -f $(kde_htmldir)/nl/kdevelop/tip.database
