
INCLUDES= $(all_includes)

lib_LTLIBRARIES = libkdevclassparser.la
libkdevclassparser_la_LDFLAGS  = $(all_libraries) -no-undefined -version-info 1:0 
libkdevclassparser_la_LIBADD = $(LIB_KDECORE)

libkdevclassparser_la_SOURCES = ClassStore.cc ParsedClass.cc ParsedMethod.cc \
				ParsedAttribute.cc ParsedArgument.cc ParsedParent.cc ParsedSignalSlot.cc \
				PersistantClassStore.cc ParsedStruct.cc ClassTreeNode.cc ParsedContainer.cc \
				ParsedClassContainer.cc ParsedItem.cc ParsedScopeContainer.cc

noinst_HEADERS = ClassStore.h ParsedClass.h ParsedMethod.h ParsedAttribute.h ParsedArgument.h ParsedParent.h ParsedSignalSlot.h PersistantClassStore.h ParsedStruct.h ClassTreeNode.h ParsedItem.h ParsedContainer.h ParsedClassContainer.h ParsedScopeContainer.h ProgrammingByContract.h
