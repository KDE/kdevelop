####### kdevelop will overwrite this part!!! (begin)##########

SUBDIRS = manual programming 

EXTRA_DIST = tip.database 

####### kdevelop will overwrite this part!!! (end)############
EXTRA_DIST = tip.database

SUBDIRS = manual programming

install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/pt/kdevelop/
	$(INSTALL_DATA) $(srcdir)/tip.database $(kde_htmldir)/pt/kdevelop/tip.database

uninstall-local:
	-rm -f $(kde_htmldir)/pt/kdevelop/tip.database

