EXTRA_DIST = tip.database

SUBDIRS = . manual
		
install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(kde_htmldir)/de/kdevelop/
	$(INSTALL_DATA) tip.database $(DESTDIR)$(kde_htmldir)/de/kdevelop/tip.database

uninstall-local:
	-rm -f $(kde_htmldir)/de/kdevelop/tip.database


