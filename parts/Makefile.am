# This is the collection of parts. These are derived from various
# interfaces in kdevelop/lib/interfaces which are in turn derived
# from KPart.

#kdevelop: SCRIPTING_SUBDIR = scripting
#kdevelop: QEDITOR_SUBDIR = qeditor
#kdevelop: SVN_SUBDIR = svn

if include_scripting
SCRIPTING_SUBDIR = scripting
endif

if include_qeditor
QEDITOR_SUBDIR = qeditor
endif

if include_svn
SVN_SUBDIR = svn
endif

SUBDIRS = doctreeview classview appwizard outputviews grepview fileview 	  adaproject adasupport 	  cppsupport pythonsupport perlsupport phpsupport fortransupport 	  autoproject scriptproject trollproject customproject 	  cvs perforce clearcase antproject diff valgrind sqlsupport 	  debugger distpart konsole astyle tools ctags regexptest doxygen 	  manager fileselector visualboyadvance tipofday abbrev filter 	  uimode editor-chooser openwith history filecreate           texttools javasupport javadebugger replace rubysupport closer bashsupport pascalsupport pascalproject           partexplorer bookmarks fullscreen           $(QEDITOR_SUBDIR) $(SVN_SUBDIR) $(SCRIPTING_SUBDIR)

