INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../)

ADD_DEFINITIONS(-fexceptions)

ADD_LIBRARY(antlr STATIC  
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
