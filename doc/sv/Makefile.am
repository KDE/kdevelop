EXTRA_DIST = tip.database

SUBDIRS =
		
install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(kde_htmldir)/sv/kdevelop/
	$(INSTALL_DATA) tip.database $(DESTDIR)$(kde_htmldir)/sv/kdevelop/tip.database

uninstall-local:
	-rm -f $(kde_htmldir)/sv/kdevelop/tip.database

