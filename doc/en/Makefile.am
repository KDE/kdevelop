####### kdevelop will overwrite this part!!! (begin)##########

SUBDIRS = cref kde_libref manual programming tutorial about 

EXTRA_DIST = tip.database 

####### kdevelop will overwrite this part!!! (end)############

install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/en/kdevelop/
	$(INSTALL_DATA) $(srcdir)/tip.database $(kde_htmldir)/en/kdevelop/tip.database

uninstall-local:
	-rm -f $(kde_htmldir)/en/kdevelop/tip.database
