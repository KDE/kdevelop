####### kdevelop will overwrite this part!!! (begin)##########

SUBDIRS = cref kde_libref manual programming tutorial welcome 

EXTRA_DIST = tip.database 

####### kdevelop will overwrite this part!!! (end)############
EXTRA_DIST =tip.database

SUBDIRS = kde_libref manual programming tutorial cref welcome

install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/en/kdevelop/
	$(INSTALL_DATA) $(srcdir)/tip.database $(kde_htmldir)/en/kdevelop/tip.database

uninstall-local:
	-rm -f $(kde_htmldir)/en/kdevelop/tip.database
