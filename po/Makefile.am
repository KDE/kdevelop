####### kdevelop will overwrite this part!!! (begin)##########


####### kdevelop will overwrite this part!!! (end)############

GMOFILES = da.gmo de.gmo es.gmo fr.gmo hu.gmo pl.gmo pt.gmo ru.gmo sk.gmo fi.gmo sv.gmo
POFILES = $(GMOFILES:.gmo=.po) 

localedir = $(kde_locale)

MAINTAINERCLEANFILES = $(GMOFILES)

GMSGFMT = @GMSGFMT@
MSGFMT = @MSGFMT@
XGETTEXT = @XGETTEXT@
EXTRA_DIST = $(PACKAGE).pot $(POFILES) $(GMOFILES)


all: all-@USE_NLS@
install-data-local: install-@USE_NLS@

SUFFIXES = .po .gmo

.po.mo:
	$(MSGFMT) -o $@ $<

.po.gmo:
	file=$(srcdir)/`echo $* | sed 's,.*/,,'`.gmo \
	&& rm -f $$file && $(GMSGFMT) -o $$file $< 

all-yes: $(GMOFILES)
all-no:

install-yes: all
	@catalogs='$(GMOFILES)'; \
	for cat in $$catalogs; do \
	destdir=$(localedir); \
	lang=`echo $$cat | sed 's/\.gmo//'`; \
	name=$(PACKAGE).mo \
	dir=$$destdir/$$lang/LC_MESSAGES; \
	$(mkinstalldirs) $(DESTDIR)$$dir; \
	$(INSTALL_DATA) $$cat $(DESTDIR)$$dir/$$name; \
	  echo "installing $$cat as $$dir/$$name"; \
	done

install-no:

uninstall-local:
	@catalogs='$(GMOFILES)'; \
	for cat in $$catalogs; do \
	destdir=$(localedir); \
	lang=`echo $$cat | sed 's/\.gmo//'`; \
	name=$(PACKAGE).mo; \
	dir=$$destdir/$$lang/LC_MESSAGES; \
	rm -f $$cat $$dir/$$name; \
	echo "removing $$dir/$$name" ; \
	done

merge:
	@catalogs='$(POFILES)'; \
	for cat in $$catalogs; do \
	name=$(PACKAGE).pot ; \
	echo $$cat $$name; \
	msgmerge $$cat $$name > $$cat.new ; \
	if diff $$cat $$cat.new; then \
		rm $$cat.new;  \
	else  \
		mv $$cat.new $$cat ; \
	fi; \
	done

