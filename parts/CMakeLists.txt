SET(KDEV_PART_LIBS ${QT_LIBRARIES} kdecore kdeui kparts kio ktexteditor ${CMAKE_BINARY_DIR}/lib/libkdevelop.so)


SUBDIRS( abbrev appwizard astyle bookmarks classview diff filecreate fileview fullscreen outputviews grepview valgrind distpart
 konsole tools regexptest doxygen fileselector tipofday filter openwith texttools replace partexplorer quickopen snippet
 filelist ctags2 scripting vcsmanager security)
# documentation uimode)



#original Makefile.am contents follow:

## This is the collection of parts. These are derived from various
## interfaces in kdevelop/lib/interfaces which are in turn derived
## from KPart.
#
#SUBDIRS = abbrev appwizard astyle bookmarks classview diff filecreate fileview \
#			fullscreen outputviews grepview valgrind distpart konsole tools regexptest doxygen \
#			fileselector tipofday filter uimode openwith texttools replace partexplorer \
#			quickopen snippet filelist ctags2 documentation scripting vcsmanager security
#
#DOXYGEN_EMPTY = YES
#include ../Doxyfile.am

#######################
#previous CMakeLists.txt contents follow:

#SUBDIRS( abbrev appwizard astyle bookmarks classview diff filecreate fileview  			fullscreen outputviews grepview valgrind distpart konsole tools regexptest doxygen  			fileselector tipofday filter uimode openwith texttools replace partexplorer  			quickopen snippet filelist ctags2 documentation scripting vcsmanager security )
#
