EXTRA_DIST = tip.database

SUBDIRS =

install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/sv/kdevelop/
	$(INSTALL_DATA) tip.database $(kde_htmldir)/sv/kdevelop/tip.database

uninstall-local:
	-rm -f $(kde_htmldir)/sv/kdevelop/tip.database

