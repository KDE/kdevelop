INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../)

ADD_DEFINITIONS(-fexceptions)

SET(antlr_STAT_SRCS
ANTLRUtil.cpp
ASTFactory.cpp
ASTNULLType.cpp
ASTRefCount.cpp
BaseAST.cpp
BitSet.cpp
CharBuffer.cpp
CharScanner.cpp
CommonAST.cpp
CommonASTWithHiddenTokens.cpp
CommonHiddenStreamToken.cpp
CommonToken.cpp
InputBuffer.cpp
LLkParser.cpp
MismatchedCharException.cpp
MismatchedTokenException.cpp
NoViableAltException.cpp
NoViableAltForCharException.cpp
Parser.cpp
RecognitionException.cpp
String.cpp
Token.cpp
TokenBuffer.cpp
TokenStreamBasicFilter.cpp
TokenStreamHiddenTokenFilter.cpp
TokenStreamSelector.cpp
TreeParser.cpp
)

ADD_LIBRARY(antlr STATIC ${antlr_STAT_SRCS})



#original Makefile.am contents follow:

#
## Make #include <antlr/xxx> work..
#INCLUDES = -I$(srcdir)/..
#KDE_CXXFLAGS = $(USE_EXCEPTIONS)
#
#noinst_LTLIBRARIES = libantlr.la
#
#libantlr_la_LDFLAGS = -no-undefined
#
#libantlr_la_SOURCES = ANTLRUtil.cpp ASTFactory.cpp ASTNULLType.cpp ASTRefCount.cpp BaseAST.cpp BitSet.cpp CharBuffer.cpp CharScanner.cpp CommonAST.cpp CommonASTWithHiddenTokens.cpp CommonHiddenStreamToken.cpp CommonToken.cpp InputBuffer.cpp LLkParser.cpp MismatchedCharException.cpp MismatchedTokenException.cpp NoViableAltException.cpp NoViableAltForCharException.cpp Parser.cpp RecognitionException.cpp String.cpp Token.cpp TokenBuffer.cpp TokenStreamBasicFilter.cpp TokenStreamHiddenTokenFilter.cpp TokenStreamSelector.cpp TreeParser.cpp

