INCLUDES = $(all_includes)

##noinst_LTLIBRARIES = libstructure.la
##libstructure_la_SOURCES =

TESTS = test-kdict
##test-trie
##test-string-trie

noinst_PROGRAMS = test-kdict
##test-string-trie

test_kdict_SOURCES = test-kdict.cpp
test_kdict_LDADD = $(LIB_QT) $(LIB_KDECORE)
test_kdict_LDFLAGS = $(all_libraries) $(KDE_RPATH)
##test_trie_SOURCES=test-trie.cpp
##test_trie_LDADD=$(all_libraries) -lqt-mt
##test_string_trie_SOURCES=test-string-trie.cxx

METASOURCES = AUTO
