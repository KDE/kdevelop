####### kdevelop will overwrite this part!!! (begin)##########

SUBDIRS = programming tutorial welcome 

EXTRA_DIST = tip.database 

####### kdevelop will overwrite this part!!! (end)############

install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/fr/kdevelop/
	$(INSTALL_DATA) $(srcdir)/tip.database $(kde_htmldir)/fr/kdevelop/tip.database

uninstall-local:
	-rm -f $(kde_htmldir)/fr/kdevelop/tip.database



