INCLUDES = $(all_includes)

METASOURCES = AUTO

noinst_LTLIBRARIES = libkdevcppparser.la

libkdevcppparser_la_SOURCES = \
	ast.cpp             codemodel_finder.cpp     default_visitor.cpp  memorypool.cpp     type_compiler.cpp \
	binder.cpp          compiler_utils.cpp       dumptree.cpp         name_compiler.cpp  visitor.cpp \
	class_compiler.cpp  control.cpp              lexer.cpp            parser.cpp \
	codemodel.cpp       declarator_compiler.cpp  listnode.cpp         tokens.cpp \
	problem.cpp

noinst_HEADERS = \
	ast.h               codemodel.h            dumptree.h    name_compiler.h  type_compiler.h \
	binder.h            compiler_utils.h       ksharedptr.h  parser.h         visitor.h \
	class_compiler.h    control.h              lexer.h       rxx_allocator.h \
	codemodel_finder.h  declarator_compiler.h  listnode.h    symbol.h \
	codemodel_fwd.h     default_visitor.h      memorypool.h  tokens.h \
	problem.h
