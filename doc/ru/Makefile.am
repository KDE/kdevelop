####### kdevelop will overwrite this part!!! (begin)##########

SUBDIRS = cref manual programming tutorial welcome 

EXTRA_DIST = tip.database 

####### kdevelop will overwrite this part!!! (end)############
SUBDIRS = welcome manual tutorial cref programming

EXTRA_DIST = tip.database

install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/ru/kdevelop/
	$(INSTALL_DATA) $(srcdir)/tip.database $(kde_htmldir)/ru/kdevelop/tip.database

uninstall-local:
	-rm -f $(kde_htmldir)/ru/kdevelop/tip.database
