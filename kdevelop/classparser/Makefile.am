####### kdevelop will overwrite this part!!! (begin)##########

INCLUDES = $(all_includes)

libclassparser_a_METASOURCES = AUTO

noinst_LIBRARIES = libclassparser.a

libclassparser_a_SOURCES = tokenizer.cc main.cc PersistantClassStore.cc ParsedStruct.cc ParsedSignalSlot.cc ParsedScopeContainer.cc ParsedParent.cc ParsedMethod.cc ParsedItem.cc ParsedContainer.cc ParsedClassContainer.cc ParsedClass.cc ParsedAttribute.cc ParsedArgument.cc ClassTreeNode.cc ClassStore.cc ClassParser.cc 


EXTRA_DIST = ClassParser.cc ClassParser.h ClassStore.cc ClassStore.h ClassTreeNode.cc ClassTreeNode.h ParsedArgument.cc ParsedArgument.h ParsedAttribute.cc ParsedAttribute.h ParsedClass.cc ParsedClass.h ParsedClassContainer.cc ParsedClassContainer.h ParsedContainer.cc ParsedContainer.h ParsedItem.cc ParsedItem.h ParsedMethod.cc ParsedMethod.h ParsedParent.cc ParsedParent.h ParsedScopeContainer.cc ParsedScopeContainer.h ParsedSignalSlot.cc ParsedSignalSlot.h ParsedStruct.cc ParsedStruct.h PersistantClassStore.cc PersistantClassStore.h classparser.dia ProgrammingByContract.h tokenizer.l main.cc tokenizer.h tokenizer.cc 

####### kdevelop will overwrite this part!!! (end)############

tokenizer.cc: tokenizer.l
	$(LEX) -+ -otokenizer.cc $(srcdir)/tokenizer.l && cat tokenizer.cc | \
	sed -e "s,class istream;,#include <iostream\.h>," > tokenizer.cc.tmp && \
	mv tokenizer.cc.tmp tokenizer.cc

KDE_OPTIONS = nofinal
