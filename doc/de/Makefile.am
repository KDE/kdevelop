EXTRA_DIST = tip.database

SUBDIRS = manual
		
install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/de/kdevelop/
	$(INSTALL_DATA) tip.database $(kde_htmldir)/de/kdevelop/tip.database

uninstall-local:
	-rm -f $(kde_htmldir)/de/kdevelop/tip.database


