
INCLUDES= $(all_includes)
noinst_LIBRARIES = libclassparser.a
noinst_HEADERS = tokenizer.h ClassParser.h ClassStore.h ParsedClass.h ParsedMethod.h ParsedAttribute.h ParsedArgument.h ParsedParent.h ParsedSignalSlot.h ParsedSignalText.h PersistantClassStore.h ParsedStruct.h ClassTreeNode.h ParsedItem.h ParsedClassItem.h ParsedContainer.h ParsedClassContainer.h tokenizer.l

libclassparser_a_SOURCES = tokenizer.cc ClassParser.cc ClassStore.cc ParsedClass.cc ParsedMethod.cc ParsedAttribute.cc ParsedArgument.cc ParsedParent.cc ParsedSignalSlot.cc ParsedSignalText.cc PersistantClassStore.cc ParsedStruct.cc ClassTreeNode.cc ParsedContainer.cc ParsedClassContainer.cc

tokenizer.cc: tokenizer.l
	$(LEX) -+ tokenizer.l && mv lex.yy.cc tokenizer.cc
