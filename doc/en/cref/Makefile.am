EXTRA_DIST =index.html

SUBDIRS = .
	
install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(kde_htmldir)/en/kdevelop/reference/C/
	$(INSTALL_DATA) index.html $(DESTDIR)$(kde_htmldir)/en/kdevelop/reference/C/cref.html

uninstall-local:
	-rm -f $(kde_htmldir)/en/kdevelop/reference/C/cref.html
