EXTRA_DIST = tip.database


install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/sv/kdevelop/
	$(INSTALL_DATA) $(srcdir)/tip.database $(kde_htmldir)/sv/kdevelop/tip.database

uninstall-local:
	-rm -f $(kde_htmldir)/sv/kdevelop/tip.database

