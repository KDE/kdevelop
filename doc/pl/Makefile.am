####### kdevelop will overwrite this part!!! (begin)##########


EXTRA_DIST = tip.database 

####### kdevelop will overwrite this part!!! (end)############
EXTRA_DIST = tip.database


install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/pl/kdevelop/
	$(INSTALL_DATA) $(srcdir)/tip.database $(kde_htmldir)/pl/kdevelop/tip.database

uninstall-local:
	-rm -f $(kde_htmldir)/pl/kdevelop/tip.database
