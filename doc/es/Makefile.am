EXTRA_DIST = tip.database


install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/es/kdevelop/
	$(INSTALL_DATA) tip.database $(kde_htmldir)/es/kdevelop/tip.database

uninstall-local:
	-rm -f $(kde_htmldir)/es/kdevelop/tip.database

