####### kdevelop will overwrite this part!!! (begin)##########

INCLUDES = $(all_includes)

libclassparser_a_METASOURCES = USE_AUTOMOC

noinst_LIBRARIES = libclassparser.a

libclassparser_a_SOURCES = ParsedSignalText.cc ParsedSignalSlot.cc ParsedParent.cc ParsedMethod.cc ParsedClass.cc ParsedAttribute.cc ParsedArgument.cc ClassStore.cc ClassParser.cc 

EXTRA_DIST = ClassParser.cc ClassParser.h ClassStore.cc ClassStore.h ParsedArgument.cc ParsedArgument.h ParsedAttribute.cc ParsedAttribute.h ParsedClass.cc ParsedClass.h ParsedMethod.cc ParsedMethod.h ParsedParent.cc ParsedParent.h ParsedSignalSlot.cc ParsedSignalSlot.h ParsedSignalText.cc ParsedSignalText.h tokenizer.h tokenizer.l 

####### kdevelop will overwrite this part!!! (end)############
INCLUDES= $(all_includes)
noinst_LIBRARIES = libclassparser.a
noinst_HEADERS = tokenizer.h ClassParser.h ClassStore.h ParsedClass.h ParsedMethod.h ParsedAttribute.h ParsedArgument.h ParsedParent.h ParsedSignalSlot.h ParsedSignalText.h PersistantClassStore.h ParsedStruct.h ClassTreeNode.h
libclassparser_a_SOURCES = tokenizer.cc ClassParser.cc ClassStore.cc ParsedClass.cc ParsedMethod.cc ParsedAttribute.cc ParsedArgument.cc ParsedParent.cc ParsedSignalSlot.cc ParsedSignalText.cc PersistantClassStore.cc ParsedStruct.cc ClassTreeNode.cc

tokenizer.cc: tokenizer.l
	$(LEX) -+ tokenizer.l && mv lex.yy.cc tokenizer.cc
