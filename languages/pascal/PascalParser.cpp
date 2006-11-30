/* $ANTLR 2.7.7 (20061129): "pascal.g" -> "PascalParser.cpp"$ */
#include "PascalParser.hpp"
#include <antlr/NoViableAltException.hpp>
#include <antlr/SemanticException.hpp>
#include <antlr/ASTFactory.hpp>
#line 1 "pascal.g"
#line 8 "PascalParser.cpp"
PascalParser::PascalParser(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf, int k)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(tokenBuf,k)
{
}

PascalParser::PascalParser(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(tokenBuf,2)
{
}

PascalParser::PascalParser(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer, int k)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(lexer,k)
{
}

PascalParser::PascalParser(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(lexer,2)
{
}

PascalParser::PascalParser(const ANTLR_USE_NAMESPACE(antlr)ParserSharedInputState& state)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(state,2)
{
}

void PascalParser::compilationUnit() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST compilationUnit_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case PROGRAM:
		{
			program();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			compilationUnit_AST = RefPascalAST(currentAST.root);
			break;
		}
		case LIBRARY:
		{
			library();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			compilationUnit_AST = RefPascalAST(currentAST.root);
			break;
		}
		case UNIT:
		{
			unit();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			compilationUnit_AST = RefPascalAST(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_0);
		} else {
			throw;
		}
	}
	returnAST = compilationUnit_AST;
}

void PascalParser::program() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST program_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		programHeading();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case USES:
		{
			usesClause();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case BEGIN:
		case LABEL:
		case CONST:
		case RESOURCESTRING:
		case TYPE:
		case VAR:
		case PROCEDURE:
		case FUNCTION:
		case CONSTRUCTOR:
		case DESTRUCTOR:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		block();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(DOT);
		program_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_0);
		} else {
			throw;
		}
	}
	returnAST = program_AST;
}

void PascalParser::library() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST library_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp2_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp2_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp2_AST));
		}
		match(LIBRARY);
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(SEMI);
		{
		switch ( LA(1)) {
		case USES:
		{
			usesClause();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case EXPORTS:
		case BEGIN:
		case LABEL:
		case CONST:
		case RESOURCESTRING:
		case TYPE:
		case VAR:
		case PROCEDURE:
		case FUNCTION:
		case CONSTRUCTOR:
		case DESTRUCTOR:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		libraryBlock();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		exportsClause();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(END);
		match(DOT);
		library_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_0);
		} else {
			throw;
		}
	}
	returnAST = library_AST;
}

void PascalParser::unit() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST unit_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp6_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp6_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp6_AST));
		}
		match(UNIT);
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(SEMI);
		interfacePart();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		implementationPart();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case INITIALIZATION:
		{
			{
			initializationPart();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{
			switch ( LA(1)) {
			case FINALIZATION:
			{
				finalizationPart();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case END:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			}
			break;
		}
		case BEGIN:
		{
			realizationPart();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case END:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(END);
		unit_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_0);
		} else {
			throw;
		}
	}
	returnAST = unit_AST;
}

void PascalParser::programHeading() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST programHeading_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp9_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp9_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp9_AST));
		}
		match(PROGRAM);
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case LPAREN:
		{
			match(LPAREN);
			identifierList();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(RPAREN);
			break;
		}
		case SEMI:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(SEMI);
		programHeading_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_1);
		} else {
			throw;
		}
	}
	returnAST = programHeading_AST;
}

void PascalParser::usesClause() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST usesClause_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp13_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp13_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp13_AST));
		}
		match(USES);
		identifierList();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(SEMI);
		usesClause_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_2);
		} else {
			throw;
		}
	}
	returnAST = usesClause_AST;
}

void PascalParser::block() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST block_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		declarationPart();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		statementPart();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		block_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_3);
		} else {
			throw;
		}
	}
	returnAST = block_AST;
}

void PascalParser::identifier() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST identifier_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp15_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp15_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp15_AST));
		}
		match(IDENT);
		identifier_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_4);
		} else {
			throw;
		}
	}
	returnAST = identifier_AST;
}

void PascalParser::libraryBlock() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST libraryBlock_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		declarationPart();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case BEGIN:
		{
			statementPart();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case EXPORTS:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		libraryBlock_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_5);
		} else {
			throw;
		}
	}
	returnAST = libraryBlock_AST;
}

void PascalParser::exportsClause() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST exportsClause_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp16_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp16_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp16_AST));
		}
		match(EXPORTS);
		exportsList();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		exportsClause_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_6);
		} else {
			throw;
		}
	}
	returnAST = exportsClause_AST;
}

void PascalParser::declarationPart() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST declarationPart_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{ // ( ... )*
		for (;;) {
			switch ( LA(1)) {
			case LABEL:
			{
				labelDeclarationPart();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case CONST:
			{
				constantDeclarationPart();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case RESOURCESTRING:
			{
				resourcestringDeclarationPart();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case TYPE:
			{
				typeDeclarationPart();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case VAR:
			{
				variableDeclarationPart();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case PROCEDURE:
			case FUNCTION:
			case CONSTRUCTOR:
			case DESTRUCTOR:
			{
				procedureAndFunctionDeclarationPart();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			default:
			{
				goto _loop41;
			}
			}
		}
		_loop41:;
		} // ( ... )*
		declarationPart_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_7);
		} else {
			throw;
		}
	}
	returnAST = declarationPart_AST;
}

void PascalParser::statementPart() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST statementPart_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		compoundStatement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		statementPart_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_3);
		} else {
			throw;
		}
	}
	returnAST = statementPart_AST;
}

void PascalParser::exportsList() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST exportsList_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		exportsEntry();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				exportsEntry();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop11;
			}
			
		}
		_loop11:;
		} // ( ... )*
		{
		switch ( LA(1)) {
		case SEMI:
		{
			match(SEMI);
			break;
		}
		case END:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		exportsList_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_6);
		} else {
			throw;
		}
	}
	returnAST = exportsList_AST;
}

void PascalParser::exportsEntry() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST exportsEntry_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case LITERAL_index:
		{
			RefPascalAST tmp19_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp19_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp19_AST));
			}
			match(LITERAL_index);
			integerConstant();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case SEMI:
		case END:
		case COMMA:
		case LITERAL_name:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		{
		switch ( LA(1)) {
		case LITERAL_name:
		{
			RefPascalAST tmp20_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp20_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp20_AST));
			}
			match(LITERAL_name);
			stringConstant();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case SEMI:
		case END:
		case COMMA:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		exportsEntry_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_8);
		} else {
			throw;
		}
	}
	returnAST = exportsEntry_AST;
}

void PascalParser::integerConstant() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST integerConstant_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefPascalAST s_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefPascalAST n_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case NUM_INT:
		{
			unsignedInteger();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			integerConstant_AST = RefPascalAST(currentAST.root);
			break;
		}
		case PLUS:
		case MINUS:
		{
			sign();
			if (inputState->guessing==0) {
				s_AST = returnAST;
			}
			unsignedInteger();
			if (inputState->guessing==0) {
				n_AST = returnAST;
			}
			if ( inputState->guessing==0 ) {
				integerConstant_AST = RefPascalAST(currentAST.root);
#line 880 "pascal.g"
				integerConstant_AST=RefPascalAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(s_AST))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(n_AST))));
#line 780 "PascalParser.cpp"
				currentAST.root = integerConstant_AST;
				if ( integerConstant_AST!=RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
					integerConstant_AST->getFirstChild() != RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
					  currentAST.child = integerConstant_AST->getFirstChild();
				else
					currentAST.child = integerConstant_AST;
				currentAST.advanceChildToEnd();
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_9);
		} else {
			throw;
		}
	}
	returnAST = integerConstant_AST;
}

void PascalParser::stringConstant() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST stringConstant_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case STRING_LITERAL:
		{
			string();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			stringConstant_AST = RefPascalAST(currentAST.root);
			break;
		}
		case CHR:
		{
			constantChr();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			stringConstant_AST = RefPascalAST(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_10);
		} else {
			throw;
		}
	}
	returnAST = stringConstant_AST;
}

void PascalParser::identifierList() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST identifierList_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				identifier();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop378;
			}
			
		}
		_loop378:;
		} // ( ... )*
		if ( inputState->guessing==0 ) {
			identifierList_AST = RefPascalAST(currentAST.root);
#line 867 "pascal.g"
			identifierList_AST = RefPascalAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(IDLIST)))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(identifierList_AST))));
#line 880 "PascalParser.cpp"
			currentAST.root = identifierList_AST;
			if ( identifierList_AST!=RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				identifierList_AST->getFirstChild() != RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = identifierList_AST->getFirstChild();
			else
				currentAST.child = identifierList_AST;
			currentAST.advanceChildToEnd();
		}
		identifierList_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_11);
		} else {
			throw;
		}
	}
	returnAST = identifierList_AST;
}

void PascalParser::interfacePart() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST interfacePart_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp22_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp22_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp22_AST));
		}
		match(INTERFACE);
		{
		switch ( LA(1)) {
		case USES:
		{
			usesClause();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case IMPLEMENTATION:
		case CONST:
		case TYPE:
		case PROCEDURE:
		case FUNCTION:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		{ // ( ... )*
		for (;;) {
			switch ( LA(1)) {
			case CONST:
			{
				constantDeclarationPart();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case TYPE:
			{
				typeDeclarationPart();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case PROCEDURE:
			case FUNCTION:
			{
				procedureHeadersPart();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			default:
			{
				goto _loop24;
			}
			}
		}
		_loop24:;
		} // ( ... )*
		interfacePart_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_12);
		} else {
			throw;
		}
	}
	returnAST = interfacePart_AST;
}

void PascalParser::implementationPart() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST implementationPart_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp23_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp23_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp23_AST));
		}
		match(IMPLEMENTATION);
		{
		switch ( LA(1)) {
		case USES:
		{
			usesClause();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case END:
		case BEGIN:
		case INITIALIZATION:
		case LABEL:
		case CONST:
		case RESOURCESTRING:
		case TYPE:
		case VAR:
		case PROCEDURE:
		case FUNCTION:
		case CONSTRUCTOR:
		case DESTRUCTOR:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		declarationPart();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		implementationPart_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_13);
		} else {
			throw;
		}
	}
	returnAST = implementationPart_AST;
}

void PascalParser::initializationPart() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST initializationPart_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp24_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp24_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp24_AST));
		}
		match(INITIALIZATION);
		statement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == SEMI)) {
				match(SEMI);
				statement();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop34;
			}
			
		}
		_loop34:;
		} // ( ... )*
		initializationPart_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_14);
		} else {
			throw;
		}
	}
	returnAST = initializationPart_AST;
}

void PascalParser::finalizationPart() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST finalizationPart_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp26_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp26_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp26_AST));
		}
		match(FINALIZATION);
		statement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == SEMI)) {
				match(SEMI);
				statement();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop37;
			}
			
		}
		_loop37:;
		} // ( ... )*
		finalizationPart_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_6);
		} else {
			throw;
		}
	}
	returnAST = finalizationPart_AST;
}

void PascalParser::realizationPart() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST realizationPart_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp28_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp28_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp28_AST));
		}
		match(BEGIN);
		statement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == SEMI)) {
				match(SEMI);
				statement();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop29;
			}
			
		}
		_loop29:;
		} // ( ... )*
		realizationPart_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_6);
		} else {
			throw;
		}
	}
	returnAST = realizationPart_AST;
}

void PascalParser::constantDeclarationPart() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST constantDeclarationPart_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp30_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp30_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp30_AST));
		}
		match(CONST);
		{ // ( ... )+
		int _cnt47=0;
		for (;;) {
			if ((LA(1) == IDENT) && (LA(2) == EQUAL)) {
				constantDeclaration();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else if ((LA(1) == IDENT) && (LA(2) == COLON)) {
				typedConstantDeclaration();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				if ( _cnt47>=1 ) { goto _loop47; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
			}
			
			_cnt47++;
		}
		_loop47:;
		}  // ( ... )+
		constantDeclarationPart_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_2);
		} else {
			throw;
		}
	}
	returnAST = constantDeclarationPart_AST;
}

void PascalParser::typeDeclarationPart() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST typeDeclarationPart_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp31_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp31_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp31_AST));
		}
		match(TYPE);
		{ // ( ... )+
		int _cnt54=0;
		for (;;) {
			if ((LA(1) == IDENT)) {
				typeDeclaration();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				if ( _cnt54>=1 ) { goto _loop54; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
			}
			
			_cnt54++;
		}
		_loop54:;
		}  // ( ... )+
		typeDeclarationPart_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_2);
		} else {
			throw;
		}
	}
	returnAST = typeDeclarationPart_AST;
}

void PascalParser::procedureHeadersPart() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST procedureHeadersPart_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case PROCEDURE:
		{
			procedureHeader();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			procedureHeadersPart_AST = RefPascalAST(currentAST.root);
			break;
		}
		case FUNCTION:
		{
			functionHeader();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			procedureHeadersPart_AST = RefPascalAST(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_15);
		} else {
			throw;
		}
	}
	returnAST = procedureHeadersPart_AST;
}

void PascalParser::statement() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST statement_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case NUM_INT:
		{
			label();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(COLON);
			break;
		}
		case BEGIN:
		case CASE:
		case GOTO:
		case IF:
		case WHILE:
		case REPEAT:
		case FOR:
		case WITH:
		case RAISE:
		case TRY:
		case IDENT:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		{
		switch ( LA(1)) {
		case GOTO:
		case RAISE:
		case IDENT:
		{
			simpleStatement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case BEGIN:
		case CASE:
		case IF:
		case WHILE:
		case REPEAT:
		case FOR:
		case WITH:
		case TRY:
		{
			structuredStatement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		statement_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_16);
		} else {
			throw;
		}
	}
	returnAST = statement_AST;
}

void PascalParser::labelDeclarationPart() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST labelDeclarationPart_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp33_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp33_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp33_AST));
		}
		match(LABEL);
		label();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				label();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop44;
			}
			
		}
		_loop44:;
		} // ( ... )*
		match(SEMI);
		labelDeclarationPart_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_17);
		} else {
			throw;
		}
	}
	returnAST = labelDeclarationPart_AST;
}

void PascalParser::resourcestringDeclarationPart() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST resourcestringDeclarationPart_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp36_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp36_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp36_AST));
		}
		match(RESOURCESTRING);
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == IDENT)) {
				stringConstantDeclaration();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop50;
			}
			
		}
		_loop50:;
		} // ( ... )*
		resourcestringDeclarationPart_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_17);
		} else {
			throw;
		}
	}
	returnAST = resourcestringDeclarationPart_AST;
}

void PascalParser::variableDeclarationPart() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST variableDeclarationPart_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp37_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp37_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp37_AST));
		}
		match(VAR);
		variableDeclaration();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == SEMI) && (LA(2) == IDENT)) {
				match(SEMI);
				variableDeclaration();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop57;
			}
			
		}
		_loop57:;
		} // ( ... )*
		match(SEMI);
		variableDeclarationPart_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_17);
		} else {
			throw;
		}
	}
	returnAST = variableDeclarationPart_AST;
}

void PascalParser::procedureAndFunctionDeclarationPart() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST procedureAndFunctionDeclarationPart_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		procedureAndFunctionDeclaration();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		procedureAndFunctionDeclarationPart_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_17);
		} else {
			throw;
		}
	}
	returnAST = procedureAndFunctionDeclarationPart_AST;
}

void PascalParser::label() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST label_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		unsignedInteger();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		label_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_18);
		} else {
			throw;
		}
	}
	returnAST = label_AST;
}

void PascalParser::constantDeclaration() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST constantDeclaration_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(EQUAL);
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(SEMI);
		constantDeclaration_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_19);
		} else {
			throw;
		}
	}
	returnAST = constantDeclaration_AST;
}

void PascalParser::typedConstantDeclaration() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST typedConstantDeclaration_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(COLON);
		type();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(EQUAL);
		typedConstant();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(SEMI);
		typedConstantDeclaration_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_19);
		} else {
			throw;
		}
	}
	returnAST = typedConstantDeclaration_AST;
}

void PascalParser::stringConstantDeclaration() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST stringConstantDeclaration_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(EQUAL);
		string();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		stringConstantDeclaration_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_20);
		} else {
			throw;
		}
	}
	returnAST = stringConstantDeclaration_AST;
}

void PascalParser::string() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST string_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp46_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp46_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp46_AST));
		}
		match(STRING_LITERAL);
		string_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_21);
		} else {
			throw;
		}
	}
	returnAST = string_AST;
}

void PascalParser::typeDeclaration() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST typeDeclaration_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(EQUAL);
		type();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(SEMI);
		typeDeclaration_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_19);
		} else {
			throw;
		}
	}
	returnAST = typeDeclaration_AST;
}

void PascalParser::variableDeclaration() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST variableDeclaration_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  c = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefPascalAST c_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifierList();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		c = LT(1);
		if ( inputState->guessing == 0 ) {
			c_AST = astFactory->create(c);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(c_AST));
		}
		match(COLON);
		if ( inputState->guessing==0 ) {
#line 246 "pascal.g"
			c_AST->setType(VARDECL);
#line 1741 "PascalParser.cpp"
		}
		type();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		variableDeclaration_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_22);
		} else {
			throw;
		}
	}
	returnAST = variableDeclaration_AST;
}

void PascalParser::type() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST type_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case INTEGER:
		case SHORTINT:
		case SMALLINT:
		case LONGINT:
		case INT64:
		case BYTE:
		case WORD:
		case CARDINAL:
		case QWORD:
		case BOOLEAN:
		case BYTEBOOL:
		case LONGBOOL:
		case CHAR:
		case REAL:
		case SINGLE:
		case DOUBLE:
		case EXTENDED:
		case COMP:
		{
			simpleType();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			type_AST = RefPascalAST(currentAST.root);
			break;
		}
		case NUM_INT:
		case PLUS:
		case MINUS:
		case STRING_LITERAL:
		case CHR:
		case NUM_REAL:
		case IDENT:
		{
			subrangeTypeOrTypeIdentifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			type_AST = RefPascalAST(currentAST.root);
			break;
		}
		case LPAREN:
		{
			enumeratedType();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			type_AST = RefPascalAST(currentAST.root);
			break;
		}
		case STRING:
		{
			stringType();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			type_AST = RefPascalAST(currentAST.root);
			break;
		}
		case ARRAY:
		case PACKED:
		case RECORD:
		case SET:
		case FILE:
		case OBJECT:
		case CLASS:
		{
			structuredType();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			type_AST = RefPascalAST(currentAST.root);
			break;
		}
		case POINTER:
		{
			pointerType();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			type_AST = RefPascalAST(currentAST.root);
			break;
		}
		case PROCEDURE:
		case FUNCTION:
		{
			proceduralType();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			type_AST = RefPascalAST(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_23);
		} else {
			throw;
		}
	}
	returnAST = type_AST;
}

void PascalParser::procedureAndFunctionDeclaration() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST procedureAndFunctionDeclaration_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case PROCEDURE:
		{
			procedureDeclaration();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			procedureAndFunctionDeclaration_AST = RefPascalAST(currentAST.root);
			break;
		}
		case FUNCTION:
		{
			functionDeclaration();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			procedureAndFunctionDeclaration_AST = RefPascalAST(currentAST.root);
			break;
		}
		case CONSTRUCTOR:
		{
			constructorDeclaration();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			procedureAndFunctionDeclaration_AST = RefPascalAST(currentAST.root);
			break;
		}
		case DESTRUCTOR:
		{
			destructorDeclaration();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			procedureAndFunctionDeclaration_AST = RefPascalAST(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_17);
		} else {
			throw;
		}
	}
	returnAST = procedureAndFunctionDeclaration_AST;
}

void PascalParser::procedureDeclaration() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST procedureDeclaration_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		procedureHeader();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		subroutineBlock();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(SEMI);
		procedureDeclaration_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_17);
		} else {
			throw;
		}
	}
	returnAST = procedureDeclaration_AST;
}

void PascalParser::functionDeclaration() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST functionDeclaration_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		functionHeader();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		subroutineBlock();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(SEMI);
		functionDeclaration_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_17);
		} else {
			throw;
		}
	}
	returnAST = functionDeclaration_AST;
}

void PascalParser::constructorDeclaration() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST constructorDeclaration_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		constructorHeader();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(SEMI);
		subroutineBlock();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		constructorDeclaration_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_17);
		} else {
			throw;
		}
	}
	returnAST = constructorDeclaration_AST;
}

void PascalParser::destructorDeclaration() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST destructorDeclaration_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		destructorHeader();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(SEMI);
		subroutineBlock();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		destructorDeclaration_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_17);
		} else {
			throw;
		}
	}
	returnAST = destructorDeclaration_AST;
}

void PascalParser::compoundStatement() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST compoundStatement_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		if ((LA(1) == BEGIN) && (LA(2) == END)) {
			match(BEGIN);
			match(END);
			compoundStatement_AST = RefPascalAST(currentAST.root);
		}
		else if ((LA(1) == BEGIN) && (LA(2) == SEMI)) {
			match(BEGIN);
			{ // ( ... )+
			int _cnt312=0;
			for (;;) {
				if ((LA(1) == SEMI)) {
					match(SEMI);
				}
				else {
					if ( _cnt312>=1 ) { goto _loop312; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
				}
				
				_cnt312++;
			}
			_loop312:;
			}  // ( ... )+
			match(END);
			compoundStatement_AST = RefPascalAST(currentAST.root);
		}
		else if ((LA(1) == BEGIN) && (_tokenSet_24.member(LA(2)))) {
			match(BEGIN);
			statement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{ // ( ... )*
			for (;;) {
				if ((LA(1) == SEMI)) {
					match(SEMI);
					{
					switch ( LA(1)) {
					case BEGIN:
					case NUM_INT:
					case CASE:
					case GOTO:
					case IF:
					case WHILE:
					case REPEAT:
					case FOR:
					case WITH:
					case RAISE:
					case TRY:
					case IDENT:
					{
						statement();
						if (inputState->guessing==0) {
							astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
						}
						break;
					}
					case SEMI:
					case END:
					{
						break;
					}
					default:
					{
						throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
					}
					}
					}
				}
				else {
					goto _loop315;
				}
				
			}
			_loop315:;
			} // ( ... )*
			match(END);
			compoundStatement_AST = RefPascalAST(currentAST.root);
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_25);
		} else {
			throw;
		}
	}
	returnAST = compoundStatement_AST;
}

void PascalParser::procedureHeader() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST procedureHeader_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp61_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp61_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp61_AST));
		}
		match(PROCEDURE);
		{
		if ((LA(1) == IDENT) && (LA(2) == SEMI || LA(2) == LPAREN)) {
			identifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
		}
		else if ((LA(1) == IDENT) && (LA(2) == COLON)) {
			qualifiedMethodIdentifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
		{
		switch ( LA(1)) {
		case LPAREN:
		{
			formalParameterList();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case SEMI:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(SEMI);
		{ // ( ... )*
		for (;;) {
			if (((LA(1) >= PUBLIC && LA(1) <= FAR))) {
				modifiers();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				match(SEMI);
			}
			else {
				goto _loop68;
			}
			
		}
		_loop68:;
		} // ( ... )*
		procedureHeader_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_26);
		} else {
			throw;
		}
	}
	returnAST = procedureHeader_AST;
}

void PascalParser::subroutineBlock() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST subroutineBlock_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case BEGIN:
		case LABEL:
		case CONST:
		case RESOURCESTRING:
		case TYPE:
		case VAR:
		case PROCEDURE:
		case FUNCTION:
		case CONSTRUCTOR:
		case DESTRUCTOR:
		{
			block();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			subroutineBlock_AST = RefPascalAST(currentAST.root);
			break;
		}
		case EXTERNAL:
		{
			externalDirective();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			subroutineBlock_AST = RefPascalAST(currentAST.root);
			break;
		}
		case FORWARD:
		{
			match(FORWARD);
			subroutineBlock_AST = RefPascalAST(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_27);
		} else {
			throw;
		}
	}
	returnAST = subroutineBlock_AST;
}

void PascalParser::functionHeader() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST functionHeader_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		bool synPredMatched74 = false;
		if (((LA(1) == FUNCTION) && (LA(2) == IDENT))) {
			int _m74 = mark();
			synPredMatched74 = true;
			inputState->guessing++;
			try {
				{
				match(FUNCTION);
				identifier();
				match(COLON);
				type();
				match(SEMI);
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched74 = false;
			}
			rewind(_m74);
			inputState->guessing--;
		}
		if ( synPredMatched74 ) {
			RefPascalAST tmp65_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp65_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp65_AST));
			}
			match(FUNCTION);
			identifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(COLON);
			type();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(SEMI);
			{ // ( ... )*
			for (;;) {
				if (((LA(1) >= PUBLIC && LA(1) <= FAR))) {
					modifiers();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					match(SEMI);
				}
				else {
					goto _loop76;
				}
				
			}
			_loop76:;
			} // ( ... )*
			functionHeader_AST = RefPascalAST(currentAST.root);
		}
		else {
			bool synPredMatched78 = false;
			if (((LA(1) == FUNCTION) && (LA(2) == IDENT))) {
				int _m78 = mark();
				synPredMatched78 = true;
				inputState->guessing++;
				try {
					{
					match(FUNCTION);
					identifier();
					match(COLON);
					match(COLON);
					identifier();
					match(COLON);
					type();
					match(SEMI);
					}
				}
				catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
					synPredMatched78 = false;
				}
				rewind(_m78);
				inputState->guessing--;
			}
			if ( synPredMatched78 ) {
				RefPascalAST tmp69_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp69_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp69_AST));
				}
				match(FUNCTION);
				qualifiedMethodIdentifier();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				match(COLON);
				type();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				match(SEMI);
				{ // ( ... )*
				for (;;) {
					if (((LA(1) >= PUBLIC && LA(1) <= FAR))) {
						modifiers();
						if (inputState->guessing==0) {
							astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
						}
						match(SEMI);
					}
					else {
						goto _loop80;
					}
					
				}
				_loop80:;
				} // ( ... )*
				functionHeader_AST = RefPascalAST(currentAST.root);
			}
			else {
				bool synPredMatched82 = false;
				if (((LA(1) == FUNCTION) && (LA(2) == IDENT))) {
					int _m82 = mark();
					synPredMatched82 = true;
					inputState->guessing++;
					try {
						{
						match(FUNCTION);
						identifier();
						match(COLON);
						match(COLON);
						identifier();
						match(LPAREN);
						}
					}
					catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
						synPredMatched82 = false;
					}
					rewind(_m82);
					inputState->guessing--;
				}
				if ( synPredMatched82 ) {
					RefPascalAST tmp73_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if ( inputState->guessing == 0 ) {
						tmp73_AST = astFactory->create(LT(1));
						astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp73_AST));
					}
					match(FUNCTION);
					qualifiedMethodIdentifier();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					formalParameterList();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					match(COLON);
					type();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					match(SEMI);
					{ // ( ... )*
					for (;;) {
						if (((LA(1) >= PUBLIC && LA(1) <= FAR))) {
							modifiers();
							if (inputState->guessing==0) {
								astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
							}
							match(SEMI);
						}
						else {
							goto _loop84;
						}
						
					}
					_loop84:;
					} // ( ... )*
					functionHeader_AST = RefPascalAST(currentAST.root);
				}
				else if ((LA(1) == FUNCTION) && (LA(2) == IDENT)) {
					RefPascalAST tmp77_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if ( inputState->guessing == 0 ) {
						tmp77_AST = astFactory->create(LT(1));
						astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp77_AST));
					}
					match(FUNCTION);
					identifier();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					formalParameterList();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					match(COLON);
					type();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					match(SEMI);
					{ // ( ... )*
					for (;;) {
						if (((LA(1) >= PUBLIC && LA(1) <= FAR))) {
							modifiers();
							if (inputState->guessing==0) {
								astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
							}
							match(SEMI);
						}
						else {
							goto _loop86;
						}
						
					}
					_loop86:;
					} // ( ... )*
					functionHeader_AST = RefPascalAST(currentAST.root);
				}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_26);
		} else {
			throw;
		}
	}
	returnAST = functionHeader_AST;
}

void PascalParser::qualifiedMethodIdentifier() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST qualifiedMethodIdentifier_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(COLON);
		match(COLON);
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		qualifiedMethodIdentifier_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_28);
		} else {
			throw;
		}
	}
	returnAST = qualifiedMethodIdentifier_AST;
}

void PascalParser::formalParameterList() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST formalParameterList_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(LPAREN);
		parameterDeclaration();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == SEMI)) {
				match(SEMI);
				parameterDeclaration();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop96;
			}
			
		}
		_loop96:;
		} // ( ... )*
		match(RPAREN);
		formalParameterList_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_29);
		} else {
			throw;
		}
	}
	returnAST = formalParameterList_AST;
}

void PascalParser::modifiers() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST modifiers_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case PUBLIC:
		{
			match(PUBLIC);
			modifiers_AST = RefPascalAST(currentAST.root);
			break;
		}
		case ALIAS:
		{
			{
			match(ALIAS);
			stringConstant();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			}
			modifiers_AST = RefPascalAST(currentAST.root);
			break;
		}
		case INTERRUPT:
		{
			match(INTERRUPT);
			modifiers_AST = RefPascalAST(currentAST.root);
			break;
		}
		case REGISTER:
		case PASCAL:
		case CDECL:
		case STDCALL:
		case POPSTACK:
		case SAVEREGISTERS:
		case INLINE:
		case SAFECALL:
		case NEAR:
		case FAR:
		{
			callModifiers();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			modifiers_AST = RefPascalAST(currentAST.root);
			break;
		}
		case EXPORT:
		{
			match(EXPORT);
			modifiers_AST = RefPascalAST(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_22);
		} else {
			throw;
		}
	}
	returnAST = modifiers_AST;
}

void PascalParser::externalDirective() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST externalDirective_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp90_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp90_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp90_AST));
		}
		match(EXTERNAL);
		{
		switch ( LA(1)) {
		case STRING_LITERAL:
		case CHR:
		{
			stringConstant();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{
			switch ( LA(1)) {
			case LITERAL_name:
			{
				{
				RefPascalAST tmp91_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp91_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp91_AST));
				}
				match(LITERAL_name);
				stringConstant();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				}
				break;
			}
			case LITERAL_index:
			{
				{
				RefPascalAST tmp92_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp92_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp92_AST));
				}
				match(LITERAL_index);
				integerConstant();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				}
				break;
			}
			case ANTLR_USE_NAMESPACE(antlr)Token::EOF_TYPE:
			case SEMI:
			case END:
			case EXPORTS:
			case BEGIN:
			case INITIALIZATION:
			case LABEL:
			case CONST:
			case RESOURCESTRING:
			case TYPE:
			case VAR:
			case PROCEDURE:
			case FUNCTION:
			case CONSTRUCTOR:
			case DESTRUCTOR:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			break;
		}
		case ANTLR_USE_NAMESPACE(antlr)Token::EOF_TYPE:
		case SEMI:
		case END:
		case EXPORTS:
		case BEGIN:
		case INITIALIZATION:
		case LABEL:
		case CONST:
		case RESOURCESTRING:
		case TYPE:
		case VAR:
		case PROCEDURE:
		case FUNCTION:
		case CONSTRUCTOR:
		case DESTRUCTOR:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		externalDirective_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_27);
		} else {
			throw;
		}
	}
	returnAST = externalDirective_AST;
}

void PascalParser::functionHeaderEnding() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST functionHeaderEnding_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case COLON:
		{
			match(COLON);
			type();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(SEMI);
			{ // ( ... )*
			for (;;) {
				if (((LA(1) >= PUBLIC && LA(1) <= FAR))) {
					modifiers();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					match(SEMI);
				}
				else {
					goto _loop91;
				}
				
			}
			_loop91:;
			} // ( ... )*
			functionHeaderEnding_AST = RefPascalAST(currentAST.root);
			break;
		}
		case LPAREN:
		{
			formalParameterList();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(COLON);
			type();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(SEMI);
			{ // ( ... )*
			for (;;) {
				if (((LA(1) >= PUBLIC && LA(1) <= FAR))) {
					modifiers();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					match(SEMI);
				}
				else {
					goto _loop93;
				}
				
			}
			_loop93:;
			} // ( ... )*
			functionHeaderEnding_AST = RefPascalAST(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_0);
		} else {
			throw;
		}
	}
	returnAST = functionHeaderEnding_AST;
}

void PascalParser::parameterDeclaration() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST parameterDeclaration_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case IDENT:
		{
			valueParameter();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			parameterDeclaration_AST = RefPascalAST(currentAST.root);
			break;
		}
		case VAR:
		{
			variableParameter();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			parameterDeclaration_AST = RefPascalAST(currentAST.root);
			break;
		}
		case CONST:
		{
			constantParameter();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			parameterDeclaration_AST = RefPascalAST(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_30);
		} else {
			throw;
		}
	}
	returnAST = parameterDeclaration_AST;
}

void PascalParser::valueParameter() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST valueParameter_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		bool synPredMatched100 = false;
		if (((LA(1) == IDENT) && (LA(2) == COMMA || LA(2) == COLON))) {
			int _m100 = mark();
			synPredMatched100 = true;
			inputState->guessing++;
			try {
				{
				identifierList();
				match(COLON);
				match(ARRAY);
				match(OF);
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched100 = false;
			}
			rewind(_m100);
			inputState->guessing--;
		}
		if ( synPredMatched100 ) {
			identifierList();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(COLON);
			match(ARRAY);
			match(OF);
			type();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			valueParameter_AST = RefPascalAST(currentAST.root);
		}
		else if ((LA(1) == IDENT) && (LA(2) == COMMA || LA(2) == COLON)) {
			identifierList();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(COLON);
			type();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			valueParameter_AST = RefPascalAST(currentAST.root);
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_30);
		} else {
			throw;
		}
	}
	returnAST = valueParameter_AST;
}

void PascalParser::variableParameter() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST variableParameter_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp103_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp103_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp103_AST));
		}
		match(VAR);
		identifierList();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case COLON:
		{
			untypedParameterPart();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case SEMI:
		case RPAREN:
		case RBRACK:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		variableParameter_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_30);
		} else {
			throw;
		}
	}
	returnAST = variableParameter_AST;
}

void PascalParser::constantParameter() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST constantParameter_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp104_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp104_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp104_AST));
		}
		match(CONST);
		identifierList();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case COLON:
		{
			untypedParameterPart();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case SEMI:
		case RPAREN:
		case RBRACK:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		constantParameter_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_30);
		} else {
			throw;
		}
	}
	returnAST = constantParameter_AST;
}

void PascalParser::untypedParameterPart() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST untypedParameterPart_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		bool synPredMatched105 = false;
		if (((LA(1) == COLON) && (LA(2) == ARRAY))) {
			int _m105 = mark();
			synPredMatched105 = true;
			inputState->guessing++;
			try {
				{
				match(COLON);
				match(ARRAY);
				match(OF);
				type();
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched105 = false;
			}
			rewind(_m105);
			inputState->guessing--;
		}
		if ( synPredMatched105 ) {
			match(COLON);
			match(ARRAY);
			match(OF);
			type();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			untypedParameterPart_AST = RefPascalAST(currentAST.root);
		}
		else if ((LA(1) == COLON) && (_tokenSet_31.member(LA(2)))) {
			match(COLON);
			type();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			untypedParameterPart_AST = RefPascalAST(currentAST.root);
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_30);
		} else {
			throw;
		}
	}
	returnAST = untypedParameterPart_AST;
}

void PascalParser::callModifiers() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST callModifiers_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case REGISTER:
		{
			match(REGISTER);
			callModifiers_AST = RefPascalAST(currentAST.root);
			break;
		}
		case PASCAL:
		{
			match(PASCAL);
			callModifiers_AST = RefPascalAST(currentAST.root);
			break;
		}
		case CDECL:
		{
			match(CDECL);
			callModifiers_AST = RefPascalAST(currentAST.root);
			break;
		}
		case STDCALL:
		{
			match(STDCALL);
			callModifiers_AST = RefPascalAST(currentAST.root);
			break;
		}
		case POPSTACK:
		{
			match(POPSTACK);
			callModifiers_AST = RefPascalAST(currentAST.root);
			break;
		}
		case SAVEREGISTERS:
		{
			match(SAVEREGISTERS);
			callModifiers_AST = RefPascalAST(currentAST.root);
			break;
		}
		case INLINE:
		{
			match(INLINE);
			callModifiers_AST = RefPascalAST(currentAST.root);
			break;
		}
		case SAFECALL:
		{
			match(SAFECALL);
			callModifiers_AST = RefPascalAST(currentAST.root);
			break;
		}
		case NEAR:
		{
			match(NEAR);
			callModifiers_AST = RefPascalAST(currentAST.root);
			break;
		}
		case FAR:
		{
			match(FAR);
			callModifiers_AST = RefPascalAST(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_23);
		} else {
			throw;
		}
	}
	returnAST = callModifiers_AST;
}

void PascalParser::expression() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST expression_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		simpleExpression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case EQUAL:
		case LE:
		case GE:
		case LTH:
		case GT:
		case NOT_EQUAL:
		case IN:
		case IS:
		{
			expressionSign();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			simpleExpression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case DOT:
		case SEMI:
		case END:
		case COMMA:
		case RPAREN:
		case FINALIZATION:
		case OF:
		case DOTDOT:
		case RBRACK:
		case THEN:
		case ELSE:
		case DO:
		case UNTIL:
		case TO:
		case DOWNTO:
		case RBRACK2:
		case EXCEPT:
		case FINALLY:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		expression_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_32);
		} else {
			throw;
		}
	}
	returnAST = expression_AST;
}

void PascalParser::typedConstant() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST typedConstant_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		bool synPredMatched120 = false;
		if (((_tokenSet_33.member(LA(1))) && (_tokenSet_34.member(LA(2))))) {
			int _m120 = mark();
			synPredMatched120 = true;
			inputState->guessing++;
			try {
				{
				constant();
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched120 = false;
			}
			rewind(_m120);
			inputState->guessing--;
		}
		if ( synPredMatched120 ) {
			constant();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			typedConstant_AST = RefPascalAST(currentAST.root);
		}
		else {
			bool synPredMatched122 = false;
			if (((LA(1) == LPAREN) && (LA(2) == IDENT))) {
				int _m122 = mark();
				synPredMatched122 = true;
				inputState->guessing++;
				try {
					{
					match(LPAREN);
					identifier();
					match(COLON);
					}
				}
				catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
					synPredMatched122 = false;
				}
				rewind(_m122);
				inputState->guessing--;
			}
			if ( synPredMatched122 ) {
				recordConstant();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				typedConstant_AST = RefPascalAST(currentAST.root);
			}
			else {
				bool synPredMatched124 = false;
				if (((LA(1) == LPAREN) && (_tokenSet_35.member(LA(2))))) {
					int _m124 = mark();
					synPredMatched124 = true;
					inputState->guessing++;
					try {
						{
						arrayConstant();
						}
					}
					catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
						synPredMatched124 = false;
					}
					rewind(_m124);
					inputState->guessing--;
				}
				if ( synPredMatched124 ) {
					arrayConstant();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					typedConstant_AST = RefPascalAST(currentAST.root);
				}
				else if ((_tokenSet_36.member(LA(1))) && (_tokenSet_37.member(LA(2)))) {
					proceduralConstant();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					typedConstant_AST = RefPascalAST(currentAST.root);
				}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_22);
		} else {
			throw;
		}
	}
	returnAST = typedConstant_AST;
}

void PascalParser::constant() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST constant_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefPascalAST s_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefPascalAST n_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefPascalAST s2_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefPascalAST id_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case NUM_INT:
		case NUM_REAL:
		{
			unsignedNumber();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			constant_AST = RefPascalAST(currentAST.root);
			break;
		}
		case IDENT:
		{
			identifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			constant_AST = RefPascalAST(currentAST.root);
			break;
		}
		case STRING_LITERAL:
		{
			string();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			constant_AST = RefPascalAST(currentAST.root);
			break;
		}
		case CHR:
		{
			constantChr();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			constant_AST = RefPascalAST(currentAST.root);
			break;
		}
		default:
			if ((LA(1) == PLUS || LA(1) == MINUS) && (LA(2) == NUM_INT || LA(2) == NUM_REAL)) {
				sign();
				if (inputState->guessing==0) {
					s_AST = returnAST;
				}
				unsignedNumber();
				if (inputState->guessing==0) {
					n_AST = returnAST;
				}
				if ( inputState->guessing==0 ) {
					constant_AST = RefPascalAST(currentAST.root);
#line 901 "pascal.g"
					constant_AST=RefPascalAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(s_AST))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(n_AST))));
#line 3461 "PascalParser.cpp"
					currentAST.root = constant_AST;
					if ( constant_AST!=RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
						constant_AST->getFirstChild() != RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
						  currentAST.child = constant_AST->getFirstChild();
					else
						currentAST.child = constant_AST;
					currentAST.advanceChildToEnd();
				}
			}
			else if ((LA(1) == PLUS || LA(1) == MINUS) && (LA(2) == IDENT)) {
				sign();
				if (inputState->guessing==0) {
					s2_AST = returnAST;
				}
				identifier();
				if (inputState->guessing==0) {
					id_AST = returnAST;
				}
				if ( inputState->guessing==0 ) {
					constant_AST = RefPascalAST(currentAST.root);
#line 903 "pascal.g"
					constant_AST=RefPascalAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(s2_AST))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(id_AST))));
#line 3484 "PascalParser.cpp"
					currentAST.root = constant_AST;
					if ( constant_AST!=RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
						constant_AST->getFirstChild() != RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
						  currentAST.child = constant_AST->getFirstChild();
					else
						currentAST.child = constant_AST;
					currentAST.advanceChildToEnd();
				}
			}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_38);
		} else {
			throw;
		}
	}
	returnAST = constant_AST;
}

void PascalParser::recordConstant() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST recordConstant_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(LPAREN);
		{
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(COLON);
		constant();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == SEMI)) {
				match(SEMI);
				{
				identifier();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				match(COLON);
				constant();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				}
			}
			else {
				goto _loop134;
			}
			
		}
		_loop134:;
		} // ( ... )*
		match(RPAREN);
		recordConstant_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_22);
		} else {
			throw;
		}
	}
	returnAST = recordConstant_AST;
}

void PascalParser::arrayConstant() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST arrayConstant_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(LPAREN);
		{
		switch ( LA(1)) {
		case NUM_INT:
		case PLUS:
		case MINUS:
		case STRING_LITERAL:
		case CHR:
		case NUM_REAL:
		case IDENT:
		{
			constant();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case LPAREN:
		{
			arrayConstant();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				{
				switch ( LA(1)) {
				case NUM_INT:
				case PLUS:
				case MINUS:
				case STRING_LITERAL:
				case CHR:
				case NUM_REAL:
				case IDENT:
				{
					constant();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					break;
				}
				case LPAREN:
				{
					arrayConstant();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
			}
			else {
				goto _loop129;
			}
			
		}
		_loop129:;
		} // ( ... )*
		match(RPAREN);
		arrayConstant_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_39);
		} else {
			throw;
		}
	}
	returnAST = arrayConstant_AST;
}

void PascalParser::proceduralConstant() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST proceduralConstant_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		proceduralConstant_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_22);
		} else {
			throw;
		}
	}
	returnAST = proceduralConstant_AST;
}

void PascalParser::addressConstant() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST addressConstant_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp127_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp127_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp127_AST));
		}
		match(NUM_INT);
		addressConstant_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_16);
		} else {
			throw;
		}
	}
	returnAST = addressConstant_AST;
}

void PascalParser::simpleType() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST simpleType_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case INTEGER:
		case SHORTINT:
		case SMALLINT:
		case LONGINT:
		case INT64:
		case BYTE:
		case WORD:
		case CARDINAL:
		case QWORD:
		case BOOLEAN:
		case BYTEBOOL:
		case LONGBOOL:
		case CHAR:
		{
			ordinalType();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			simpleType_AST = RefPascalAST(currentAST.root);
			break;
		}
		case REAL:
		case SINGLE:
		case DOUBLE:
		case EXTENDED:
		case COMP:
		{
			realType();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			simpleType_AST = RefPascalAST(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_23);
		} else {
			throw;
		}
	}
	returnAST = simpleType_AST;
}

void PascalParser::subrangeTypeOrTypeIdentifier() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST subrangeTypeOrTypeIdentifier_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		constant();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case DOTDOT:
		{
			match(DOTDOT);
			constant();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case SEMI:
		case RPAREN:
		case EQUAL:
		case RBRACK:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		subrangeTypeOrTypeIdentifier_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_23);
		} else {
			throw;
		}
	}
	returnAST = subrangeTypeOrTypeIdentifier_AST;
}

void PascalParser::enumeratedType() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST enumeratedType_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		bool synPredMatched147 = false;
		if (((LA(1) == LPAREN) && (LA(2) == IDENT))) {
			int _m147 = mark();
			synPredMatched147 = true;
			inputState->guessing++;
			try {
				{
				match(LPAREN);
				identifier();
				match(ASSIGN);
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched147 = false;
			}
			rewind(_m147);
			inputState->guessing--;
		}
		if ( synPredMatched147 ) {
			match(LPAREN);
			assignedEnumList();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(RPAREN);
			enumeratedType_AST = RefPascalAST(currentAST.root);
		}
		else if ((LA(1) == LPAREN) && (LA(2) == IDENT)) {
			match(LPAREN);
			identifierList();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(RPAREN);
			enumeratedType_AST = RefPascalAST(currentAST.root);
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_40);
		} else {
			throw;
		}
	}
	returnAST = enumeratedType_AST;
}

void PascalParser::stringType() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST stringType_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp133_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp133_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp133_AST));
		}
		match(STRING);
		{
		switch ( LA(1)) {
		case LBRACK:
		{
			match(LBRACK);
			unsignedInteger();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(RBRACK);
			break;
		}
		case SEMI:
		case RPAREN:
		case EQUAL:
		case RBRACK:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		stringType_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_23);
		} else {
			throw;
		}
	}
	returnAST = stringType_AST;
}

void PascalParser::structuredType() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST structuredType_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case PACKED:
		{
			match(PACKED);
			break;
		}
		case ARRAY:
		case RECORD:
		case SET:
		case FILE:
		case OBJECT:
		case CLASS:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		{
		switch ( LA(1)) {
		case ARRAY:
		{
			arrayType();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case RECORD:
		{
			recordType();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case OBJECT:
		{
			objectType();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case CLASS:
		{
			classType();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case SET:
		{
			setType();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case FILE:
		{
			fileType();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		structuredType_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_23);
		} else {
			throw;
		}
	}
	returnAST = structuredType_AST;
}

void PascalParser::pointerType() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST pointerType_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp137_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp137_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp137_AST));
		}
		match(POINTER);
		typeIdentifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		pointerType_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_23);
		} else {
			throw;
		}
	}
	returnAST = pointerType_AST;
}

void PascalParser::proceduralType() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST proceduralType_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		bool synPredMatched185 = false;
		if (((LA(1) == PROCEDURE || LA(1) == FUNCTION) && (LA(2) == IDENT))) {
			int _m185 = mark();
			synPredMatched185 = true;
			inputState->guessing++;
			try {
				{
				proceduralTypePart1();
				match(SEMI);
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched185 = false;
			}
			rewind(_m185);
			inputState->guessing--;
		}
		if ( synPredMatched185 ) {
			proceduralTypePart1();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(SEMI);
			callModifiers();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			proceduralType_AST = RefPascalAST(currentAST.root);
		}
		else if ((LA(1) == PROCEDURE || LA(1) == FUNCTION) && (LA(2) == IDENT)) {
			proceduralTypePart1();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			proceduralType_AST = RefPascalAST(currentAST.root);
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_23);
		} else {
			throw;
		}
	}
	returnAST = proceduralType_AST;
}

void PascalParser::ordinalType() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST ordinalType_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case INTEGER:
		{
			match(INTEGER);
			ordinalType_AST = RefPascalAST(currentAST.root);
			break;
		}
		case SHORTINT:
		{
			match(SHORTINT);
			ordinalType_AST = RefPascalAST(currentAST.root);
			break;
		}
		case SMALLINT:
		{
			match(SMALLINT);
			ordinalType_AST = RefPascalAST(currentAST.root);
			break;
		}
		case LONGINT:
		{
			match(LONGINT);
			ordinalType_AST = RefPascalAST(currentAST.root);
			break;
		}
		case INT64:
		{
			match(INT64);
			ordinalType_AST = RefPascalAST(currentAST.root);
			break;
		}
		case BYTE:
		{
			match(BYTE);
			ordinalType_AST = RefPascalAST(currentAST.root);
			break;
		}
		case WORD:
		{
			match(WORD);
			ordinalType_AST = RefPascalAST(currentAST.root);
			break;
		}
		case CARDINAL:
		{
			match(CARDINAL);
			ordinalType_AST = RefPascalAST(currentAST.root);
			break;
		}
		case QWORD:
		{
			match(QWORD);
			ordinalType_AST = RefPascalAST(currentAST.root);
			break;
		}
		case BOOLEAN:
		{
			match(BOOLEAN);
			ordinalType_AST = RefPascalAST(currentAST.root);
			break;
		}
		case BYTEBOOL:
		{
			match(BYTEBOOL);
			ordinalType_AST = RefPascalAST(currentAST.root);
			break;
		}
		case LONGBOOL:
		{
			match(LONGBOOL);
			ordinalType_AST = RefPascalAST(currentAST.root);
			break;
		}
		case CHAR:
		{
			match(CHAR);
			ordinalType_AST = RefPascalAST(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_40);
		} else {
			throw;
		}
	}
	returnAST = ordinalType_AST;
}

void PascalParser::realType() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST realType_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case REAL:
		{
			match(REAL);
			realType_AST = RefPascalAST(currentAST.root);
			break;
		}
		case SINGLE:
		{
			match(SINGLE);
			realType_AST = RefPascalAST(currentAST.root);
			break;
		}
		case DOUBLE:
		{
			match(DOUBLE);
			realType_AST = RefPascalAST(currentAST.root);
			break;
		}
		case EXTENDED:
		{
			match(EXTENDED);
			realType_AST = RefPascalAST(currentAST.root);
			break;
		}
		case COMP:
		{
			match(COMP);
			realType_AST = RefPascalAST(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_23);
		} else {
			throw;
		}
	}
	returnAST = realType_AST;
}

void PascalParser::typeIdentifier() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST typeIdentifier_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		typeIdentifier_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_41);
		} else {
			throw;
		}
	}
	returnAST = typeIdentifier_AST;
}

void PascalParser::subrangeType() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST subrangeType_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		constant();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(DOTDOT);
		constant();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		subrangeType_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_0);
		} else {
			throw;
		}
	}
	returnAST = subrangeType_AST;
}

void PascalParser::assignedEnumList() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST assignedEnumList_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(ASSIGN);
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				{
				identifier();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				match(ASSIGN);
				expression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				}
			}
			else {
				goto _loop152;
			}
			
		}
		_loop152:;
		} // ( ... )*
		assignedEnumList_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_42);
		} else {
			throw;
		}
	}
	returnAST = assignedEnumList_AST;
}

void PascalParser::unsignedInteger() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST unsignedInteger_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp161_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp161_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp161_AST));
		}
		match(NUM_INT);
		unsignedInteger_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_43);
		} else {
			throw;
		}
	}
	returnAST = unsignedInteger_AST;
}

void PascalParser::arrayType() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST arrayType_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp162_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp162_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp162_AST));
		}
		match(ARRAY);
		match(LBRACK);
		arrayIndexType();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				arrayIndexType();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop161;
			}
			
		}
		_loop161:;
		} // ( ... )*
		match(RBRACK);
		match(OF);
		type();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		arrayType_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_23);
		} else {
			throw;
		}
	}
	returnAST = arrayType_AST;
}

void PascalParser::recordType() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST recordType_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp167_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp167_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp167_AST));
		}
		match(RECORD);
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == CASE || LA(1) == IDENT)) {
				fieldList();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop168;
			}
			
		}
		_loop168:;
		} // ( ... )*
		match(END);
		recordType_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_23);
		} else {
			throw;
		}
	}
	returnAST = recordType_AST;
}

void PascalParser::objectType() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST objectType_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp169_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp169_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp169_AST));
		}
		match(OBJECT);
		{
		switch ( LA(1)) {
		case LPAREN:
		{
			heritage();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case END:
		case PROCEDURE:
		case FUNCTION:
		case PUBLIC:
		case PRIVATE:
		case PROTECTED:
		case CONSTRUCTOR:
		case DESTRUCTOR:
		case IDENT:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		{
		switch ( LA(1)) {
		case END:
		case PROCEDURE:
		case FUNCTION:
		case CONSTRUCTOR:
		case DESTRUCTOR:
		case IDENT:
		{
			componentList();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case PUBLIC:
		case PRIVATE:
		case PROTECTED:
		{
			objectVisibilitySpecifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(END);
		objectType_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_23);
		} else {
			throw;
		}
	}
	returnAST = objectType_AST;
}

void PascalParser::classType() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST classType_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp171_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp171_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp171_AST));
		}
		match(CLASS);
		{
		switch ( LA(1)) {
		case LPAREN:
		{
			heritage();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case END:
		case PROCEDURE:
		case FUNCTION:
		case PUBLIC:
		case PRIVATE:
		case PROTECTED:
		case CONSTRUCTOR:
		case DESTRUCTOR:
		case CLASS:
		case PUBLISHED:
		case PROPERTY:
		case IDENT:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		{
		switch ( LA(1)) {
		case END:
		case PROCEDURE:
		case FUNCTION:
		case CONSTRUCTOR:
		case DESTRUCTOR:
		case CLASS:
		case PROPERTY:
		case IDENT:
		{
			classComponentList();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case PUBLIC:
		case PRIVATE:
		case PROTECTED:
		case PUBLISHED:
		{
			classVisibilitySpecifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(END);
		classType_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_23);
		} else {
			throw;
		}
	}
	returnAST = classType_AST;
}

void PascalParser::setType() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST setType_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp173_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp173_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp173_AST));
		}
		match(SET);
		match(OF);
		ordinalType();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		setType_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_23);
		} else {
			throw;
		}
	}
	returnAST = setType_AST;
}

void PascalParser::fileType() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST fileType_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp175_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp175_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp175_AST));
		}
		match(FILE);
		match(OF);
		type();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		fileType_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_23);
		} else {
			throw;
		}
	}
	returnAST = fileType_AST;
}

void PascalParser::arrayIndexType() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST arrayIndexType_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		if (((LA(1) >= INTEGER && LA(1) <= CHAR))) {
			ordinalType();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			arrayIndexType_AST = RefPascalAST(currentAST.root);
		}
		else {
			bool synPredMatched164 = false;
			if (((_tokenSet_36.member(LA(1))) && (_tokenSet_44.member(LA(2))))) {
				int _m164 = mark();
				synPredMatched164 = true;
				inputState->guessing++;
				try {
					{
					expression();
					match(DOTDOT);
					}
				}
				catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
					synPredMatched164 = false;
				}
				rewind(_m164);
				inputState->guessing--;
			}
			if ( synPredMatched164 ) {
				arraySubrangeType();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				arrayIndexType_AST = RefPascalAST(currentAST.root);
			}
			else if ((LA(1) == LPAREN) && (LA(2) == IDENT)) {
				enumeratedType();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				arrayIndexType_AST = RefPascalAST(currentAST.root);
			}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_45);
		} else {
			throw;
		}
	}
	returnAST = arrayIndexType_AST;
}

void PascalParser::arraySubrangeType() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST arraySubrangeType_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(DOTDOT);
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		arraySubrangeType_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_45);
		} else {
			throw;
		}
	}
	returnAST = arraySubrangeType_AST;
}

void PascalParser::fieldList() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST fieldList_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case IDENT:
		{
			fixedField();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			fieldList_AST = RefPascalAST(currentAST.root);
			break;
		}
		case CASE:
		{
			variantPart();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			fieldList_AST = RefPascalAST(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_46);
		} else {
			throw;
		}
	}
	returnAST = fieldList_AST;
}

void PascalParser::fixedField() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST fixedField_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifierList();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(COLON);
		type();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(SEMI);
		fixedField_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_46);
		} else {
			throw;
		}
	}
	returnAST = fixedField_AST;
}

void PascalParser::variantPart() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST variantPart_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp180_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp180_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp180_AST));
		}
		match(CASE);
		{
		if ((LA(1) == IDENT) && (LA(2) == COLON)) {
			identifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(COLON);
		}
		else if ((LA(1) == IDENT) && (LA(2) == OF)) {
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(OF);
		variant();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == SEMI)) {
				match(SEMI);
				variant();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop174;
			}
			
		}
		_loop174:;
		} // ( ... )*
		variantPart_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_46);
		} else {
			throw;
		}
	}
	returnAST = variantPart_AST;
}

void PascalParser::variant() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST variant_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{ // ( ... )+
		int _cnt177=0;
		for (;;) {
			if ((_tokenSet_33.member(LA(1)))) {
				constant();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				match(COMMA);
			}
			else {
				if ( _cnt177>=1 ) { goto _loop177; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
			}
			
			_cnt177++;
		}
		_loop177:;
		}  // ( ... )+
		match(COLON);
		match(LPAREN);
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == CASE || LA(1) == IDENT)) {
				fieldList();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop179;
			}
			
		}
		_loop179:;
		} // ( ... )*
		match(RPAREN);
		variant_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_47);
		} else {
			throw;
		}
	}
	returnAST = variant_AST;
}

void PascalParser::proceduralTypePart1() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST proceduralTypePart1_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case FUNCTION:
		{
			functionHeader();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case PROCEDURE:
		{
			procedureHeader();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		{
		switch ( LA(1)) {
		case OF:
		{
			match(OF);
			match(OBJECT);
			break;
		}
		case SEMI:
		case RPAREN:
		case EQUAL:
		case RBRACK:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		proceduralTypePart1_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_23);
		} else {
			throw;
		}
	}
	returnAST = proceduralTypePart1_AST;
}

void PascalParser::heritage() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST heritage_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(LPAREN);
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(RPAREN);
		heritage_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_48);
		} else {
			throw;
		}
	}
	returnAST = heritage_AST;
}

void PascalParser::componentList() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST componentList_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case IDENT:
		{
			{ // ( ... )+
			int _cnt196=0;
			for (;;) {
				if ((LA(1) == IDENT)) {
					fieldDefinition();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
				}
				else {
					if ( _cnt196>=1 ) { goto _loop196; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
				}
				
				_cnt196++;
			}
			_loop196:;
			}  // ( ... )+
			break;
		}
		case END:
		case PROCEDURE:
		case FUNCTION:
		case CONSTRUCTOR:
		case DESTRUCTOR:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		{
		switch ( LA(1)) {
		case PROCEDURE:
		case FUNCTION:
		case CONSTRUCTOR:
		case DESTRUCTOR:
		{
			{ // ( ... )+
			int _cnt199=0;
			for (;;) {
				if ((_tokenSet_49.member(LA(1)))) {
					methodDefinition();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
				}
				else {
					if ( _cnt199>=1 ) { goto _loop199; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
				}
				
				_cnt199++;
			}
			_loop199:;
			}  // ( ... )+
			break;
		}
		case END:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		componentList_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_6);
		} else {
			throw;
		}
	}
	returnAST = componentList_AST;
}

void PascalParser::objectVisibilitySpecifier() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST objectVisibilitySpecifier_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case PRIVATE:
		{
			match(PRIVATE);
			objectVisibilitySpecifier_AST = RefPascalAST(currentAST.root);
			break;
		}
		case PROTECTED:
		{
			match(PROTECTED);
			objectVisibilitySpecifier_AST = RefPascalAST(currentAST.root);
			break;
		}
		case PUBLIC:
		{
			match(PUBLIC);
			objectVisibilitySpecifier_AST = RefPascalAST(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_6);
		} else {
			throw;
		}
	}
	returnAST = objectVisibilitySpecifier_AST;
}

void PascalParser::fieldDefinition() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST fieldDefinition_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifierList();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(COLON);
		type();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(SEMI);
		fieldDefinition_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_50);
		} else {
			throw;
		}
	}
	returnAST = fieldDefinition_AST;
}

void PascalParser::methodDefinition() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST methodDefinition_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case FUNCTION:
		{
			functionHeader();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case PROCEDURE:
		{
			procedureHeader();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case CONSTRUCTOR:
		{
			constructorHeader();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case DESTRUCTOR:
		{
			destructorHeader();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(SEMI);
		methodDirectives();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		methodDefinition_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_51);
		} else {
			throw;
		}
	}
	returnAST = methodDefinition_AST;
}

void PascalParser::constructorHeader() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST constructorHeader_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp198_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp198_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp198_AST));
		}
		match(CONSTRUCTOR);
		{
		if ((LA(1) == IDENT) && (LA(2) == LPAREN)) {
			identifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
		}
		else if ((LA(1) == IDENT) && (LA(2) == COLON)) {
			qualifiedMethodIdentifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
		formalParameterList();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		constructorHeader_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_22);
		} else {
			throw;
		}
	}
	returnAST = constructorHeader_AST;
}

void PascalParser::destructorHeader() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST destructorHeader_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp199_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp199_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp199_AST));
		}
		match(DESTRUCTOR);
		{
		if ((LA(1) == IDENT) && (LA(2) == LPAREN)) {
			identifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
		}
		else if ((LA(1) == IDENT) && (LA(2) == COLON)) {
			qualifiedMethodIdentifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
		formalParameterList();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		destructorHeader_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_22);
		} else {
			throw;
		}
	}
	returnAST = destructorHeader_AST;
}

void PascalParser::methodDirectives() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST methodDirectives_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case VIRTUAL:
		{
			match(VIRTUAL);
			match(SEMI);
			{
			switch ( LA(1)) {
			case ABSTRACT:
			{
				match(ABSTRACT);
				match(SEMI);
				break;
			}
			case END:
			case PROCEDURE:
			case FUNCTION:
			case REGISTER:
			case PASCAL:
			case CDECL:
			case STDCALL:
			case POPSTACK:
			case SAVEREGISTERS:
			case INLINE:
			case SAFECALL:
			case NEAR:
			case FAR:
			case CONSTRUCTOR:
			case DESTRUCTOR:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			break;
		}
		case END:
		case PROCEDURE:
		case FUNCTION:
		case REGISTER:
		case PASCAL:
		case CDECL:
		case STDCALL:
		case POPSTACK:
		case SAVEREGISTERS:
		case INLINE:
		case SAFECALL:
		case NEAR:
		case FAR:
		case CONSTRUCTOR:
		case DESTRUCTOR:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		{
		switch ( LA(1)) {
		case REGISTER:
		case PASCAL:
		case CDECL:
		case STDCALL:
		case POPSTACK:
		case SAVEREGISTERS:
		case INLINE:
		case SAFECALL:
		case NEAR:
		case FAR:
		{
			callModifiers();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(SEMI);
			break;
		}
		case END:
		case PROCEDURE:
		case FUNCTION:
		case CONSTRUCTOR:
		case DESTRUCTOR:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		methodDirectives_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_51);
		} else {
			throw;
		}
	}
	returnAST = methodDirectives_AST;
}

void PascalParser::classComponentList() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST classComponentList_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case IDENT:
		{
			{ // ( ... )+
			int _cnt220=0;
			for (;;) {
				if ((LA(1) == IDENT)) {
					fieldDefinition();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
				}
				else {
					if ( _cnt220>=1 ) { goto _loop220; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
				}
				
				_cnt220++;
			}
			_loop220:;
			}  // ( ... )+
			break;
		}
		case END:
		case PROCEDURE:
		case FUNCTION:
		case CONSTRUCTOR:
		case DESTRUCTOR:
		case CLASS:
		case PROPERTY:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		{
		switch ( LA(1)) {
		case PROCEDURE:
		case FUNCTION:
		case CONSTRUCTOR:
		case DESTRUCTOR:
		case CLASS:
		case PROPERTY:
		{
			{ // ( ... )+
			int _cnt224=0;
			for (;;) {
				if ((_tokenSet_52.member(LA(1)))) {
					{
					switch ( LA(1)) {
					case PROCEDURE:
					case FUNCTION:
					case CONSTRUCTOR:
					case DESTRUCTOR:
					case CLASS:
					{
						classMethodDefinition();
						if (inputState->guessing==0) {
							astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
						}
						break;
					}
					case PROPERTY:
					{
						propertyDefinition();
						if (inputState->guessing==0) {
							astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
						}
						break;
					}
					default:
					{
						throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
					}
					}
					}
				}
				else {
					if ( _cnt224>=1 ) { goto _loop224; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
				}
				
				_cnt224++;
			}
			_loop224:;
			}  // ( ... )+
			break;
		}
		case END:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		classComponentList_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_6);
		} else {
			throw;
		}
	}
	returnAST = classComponentList_AST;
}

void PascalParser::classVisibilitySpecifier() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST classVisibilitySpecifier_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case PRIVATE:
		{
			match(PRIVATE);
			classVisibilitySpecifier_AST = RefPascalAST(currentAST.root);
			break;
		}
		case PROTECTED:
		{
			match(PROTECTED);
			classVisibilitySpecifier_AST = RefPascalAST(currentAST.root);
			break;
		}
		case PUBLIC:
		{
			match(PUBLIC);
			classVisibilitySpecifier_AST = RefPascalAST(currentAST.root);
			break;
		}
		case PUBLISHED:
		{
			match(PUBLISHED);
			classVisibilitySpecifier_AST = RefPascalAST(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_6);
		} else {
			throw;
		}
	}
	returnAST = classVisibilitySpecifier_AST;
}

void PascalParser::classMethodDefinition() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST classMethodDefinition_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case PROCEDURE:
		case FUNCTION:
		case CLASS:
		{
			{
			{
			switch ( LA(1)) {
			case CLASS:
			{
				match(CLASS);
				break;
			}
			case PROCEDURE:
			case FUNCTION:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			{
			switch ( LA(1)) {
			case FUNCTION:
			{
				functionHeader();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case PROCEDURE:
			{
				procedureHeader();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			}
			break;
		}
		case CONSTRUCTOR:
		{
			constructorHeader();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case DESTRUCTOR:
		{
			destructorHeader();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(SEMI);
		classMethodDirectives();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		classMethodDefinition_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_53);
		} else {
			throw;
		}
	}
	returnAST = classMethodDefinition_AST;
}

void PascalParser::propertyDefinition() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST propertyDefinition_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp211_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp211_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp211_AST));
		}
		match(PROPERTY);
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case COLON:
		case LBRACK:
		{
			propertyInterface();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case END:
		case PROCEDURE:
		case FUNCTION:
		case CONSTRUCTOR:
		case DESTRUCTOR:
		case CLASS:
		case PROPERTY:
		case LITERAL_read:
		case LITERAL_write:
		case DEFAULT:
		case LITERAL_nodefault:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		propertySpecifiers();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		propertyDefinition_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_53);
		} else {
			throw;
		}
	}
	returnAST = propertyDefinition_AST;
}

void PascalParser::classMethodDirectives() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST classMethodDirectives_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case VIRTUAL:
		case OVERRIDE:
		case MESSAGE:
		{
			directiveVariants();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(SEMI);
			break;
		}
		case END:
		case PROCEDURE:
		case FUNCTION:
		case REGISTER:
		case PASCAL:
		case CDECL:
		case STDCALL:
		case POPSTACK:
		case SAVEREGISTERS:
		case INLINE:
		case SAFECALL:
		case NEAR:
		case FAR:
		case CONSTRUCTOR:
		case DESTRUCTOR:
		case CLASS:
		case PROPERTY:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		{
		switch ( LA(1)) {
		case REGISTER:
		case PASCAL:
		case CDECL:
		case STDCALL:
		case POPSTACK:
		case SAVEREGISTERS:
		case INLINE:
		case SAFECALL:
		case NEAR:
		case FAR:
		{
			callModifiers();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(SEMI);
			break;
		}
		case END:
		case PROCEDURE:
		case FUNCTION:
		case CONSTRUCTOR:
		case DESTRUCTOR:
		case CLASS:
		case PROPERTY:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		classMethodDirectives_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_53);
		} else {
			throw;
		}
	}
	returnAST = classMethodDirectives_AST;
}

void PascalParser::directiveVariants() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST directiveVariants_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case VIRTUAL:
		{
			{
			match(VIRTUAL);
			{
			switch ( LA(1)) {
			case ABSTRACT:
			{
				match(ABSTRACT);
				match(SEMI);
				break;
			}
			case SEMI:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			}
			directiveVariants_AST = RefPascalAST(currentAST.root);
			break;
		}
		case OVERRIDE:
		{
			match(OVERRIDE);
			directiveVariants_AST = RefPascalAST(currentAST.root);
			break;
		}
		case MESSAGE:
		{
			{
			match(MESSAGE);
			{
			switch ( LA(1)) {
			case NUM_INT:
			case PLUS:
			case MINUS:
			{
				integerConstant();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case STRING_LITERAL:
			case CHR:
			{
				stringConstant();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			}
			directiveVariants_AST = RefPascalAST(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_22);
		} else {
			throw;
		}
	}
	returnAST = directiveVariants_AST;
}

void PascalParser::propertyInterface() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST propertyInterface_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LBRACK:
		{
			propertyParameterList();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case COLON:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(COLON);
		typeIdentifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case LITERAL_index:
		{
			RefPascalAST tmp220_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp220_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp220_AST));
			}
			match(LITERAL_index);
			integerConstant();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case END:
		case PROCEDURE:
		case FUNCTION:
		case CONSTRUCTOR:
		case DESTRUCTOR:
		case CLASS:
		case PROPERTY:
		case LITERAL_read:
		case LITERAL_write:
		case DEFAULT:
		case LITERAL_nodefault:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		propertyInterface_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_54);
		} else {
			throw;
		}
	}
	returnAST = propertyInterface_AST;
}

void PascalParser::propertySpecifiers() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST propertySpecifiers_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LITERAL_read:
		{
			readSpecifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case END:
		case PROCEDURE:
		case FUNCTION:
		case CONSTRUCTOR:
		case DESTRUCTOR:
		case CLASS:
		case PROPERTY:
		case LITERAL_write:
		case DEFAULT:
		case LITERAL_nodefault:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		{
		switch ( LA(1)) {
		case LITERAL_write:
		{
			writeSpecifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case END:
		case PROCEDURE:
		case FUNCTION:
		case CONSTRUCTOR:
		case DESTRUCTOR:
		case CLASS:
		case PROPERTY:
		case DEFAULT:
		case LITERAL_nodefault:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		{
		switch ( LA(1)) {
		case DEFAULT:
		case LITERAL_nodefault:
		{
			defaultSpecifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case END:
		case PROCEDURE:
		case FUNCTION:
		case CONSTRUCTOR:
		case DESTRUCTOR:
		case CLASS:
		case PROPERTY:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		propertySpecifiers_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_53);
		} else {
			throw;
		}
	}
	returnAST = propertySpecifiers_AST;
}

void PascalParser::propertyParameterList() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST propertyParameterList_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(LBRACK);
		parameterDeclaration();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == SEMI)) {
				match(SEMI);
				parameterDeclaration();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop246;
			}
			
		}
		_loop246:;
		} // ( ... )*
		match(RBRACK);
		propertyParameterList_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_55);
		} else {
			throw;
		}
	}
	returnAST = propertyParameterList_AST;
}

void PascalParser::readSpecifier() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST readSpecifier_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp224_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp224_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp224_AST));
		}
		match(LITERAL_read);
		fieldOrMethod();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		readSpecifier_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_56);
		} else {
			throw;
		}
	}
	returnAST = readSpecifier_AST;
}

void PascalParser::writeSpecifier() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST writeSpecifier_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp225_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp225_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp225_AST));
		}
		match(LITERAL_write);
		fieldOrMethod();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		writeSpecifier_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_57);
		} else {
			throw;
		}
	}
	returnAST = writeSpecifier_AST;
}

void PascalParser::defaultSpecifier() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST defaultSpecifier_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case DEFAULT:
		{
			{
			RefPascalAST tmp226_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp226_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp226_AST));
			}
			match(DEFAULT);
			{
			switch ( LA(1)) {
			case NUM_INT:
			case PLUS:
			case MINUS:
			case STRING_LITERAL:
			case CHR:
			case NUM_REAL:
			case IDENT:
			{
				constant();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case END:
			case PROCEDURE:
			case FUNCTION:
			case CONSTRUCTOR:
			case DESTRUCTOR:
			case CLASS:
			case PROPERTY:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			}
			defaultSpecifier_AST = RefPascalAST(currentAST.root);
			break;
		}
		case LITERAL_nodefault:
		{
			RefPascalAST tmp227_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp227_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp227_AST));
			}
			match(LITERAL_nodefault);
			defaultSpecifier_AST = RefPascalAST(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_53);
		} else {
			throw;
		}
	}
	returnAST = defaultSpecifier_AST;
}

void PascalParser::fieldOrMethod() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST fieldOrMethod_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		fieldOrMethod_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_56);
		} else {
			throw;
		}
	}
	returnAST = fieldOrMethod_AST;
}

void PascalParser::simpleExpression() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST simpleExpression_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		term();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if (((LA(1) >= PLUS && LA(1) <= XOR))) {
				{
				switch ( LA(1)) {
				case PLUS:
				{
					match(PLUS);
					break;
				}
				case MINUS:
				{
					match(MINUS);
					break;
				}
				case OR:
				{
					match(OR);
					break;
				}
				case XOR:
				{
					match(XOR);
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				term();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop263;
			}
			
		}
		_loop263:;
		} // ( ... )*
		simpleExpression_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_58);
		} else {
			throw;
		}
	}
	returnAST = simpleExpression_AST;
}

void PascalParser::expressionSign() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST expressionSign_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case LE:
		{
			match(LE);
			expressionSign_AST = RefPascalAST(currentAST.root);
			break;
		}
		case GE:
		{
			match(GE);
			expressionSign_AST = RefPascalAST(currentAST.root);
			break;
		}
		case LTH:
		{
			match(LTH);
			expressionSign_AST = RefPascalAST(currentAST.root);
			break;
		}
		case GT:
		{
			match(GT);
			expressionSign_AST = RefPascalAST(currentAST.root);
			break;
		}
		case NOT_EQUAL:
		{
			match(NOT_EQUAL);
			expressionSign_AST = RefPascalAST(currentAST.root);
			break;
		}
		case IN:
		{
			match(IN);
			expressionSign_AST = RefPascalAST(currentAST.root);
			break;
		}
		case IS:
		{
			match(IS);
			expressionSign_AST = RefPascalAST(currentAST.root);
			break;
		}
		case EQUAL:
		{
			match(EQUAL);
			expressionSign_AST = RefPascalAST(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_36);
		} else {
			throw;
		}
	}
	returnAST = expressionSign_AST;
}

void PascalParser::term() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST term_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		factor();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if (((LA(1) >= STAR && LA(1) <= SHR))) {
				{
				switch ( LA(1)) {
				case STAR:
				{
					match(STAR);
					break;
				}
				case SLASH:
				{
					match(SLASH);
					break;
				}
				case DIV:
				{
					match(DIV);
					break;
				}
				case MOD:
				{
					match(MOD);
					break;
				}
				case AND:
				{
					match(AND);
					break;
				}
				case SHL:
				{
					match(SHL);
					break;
				}
				case SHR:
				{
					match(SHR);
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				factor();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop267;
			}
			
		}
		_loop267:;
		} // ( ... )*
		term_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_59);
		} else {
			throw;
		}
	}
	returnAST = term_AST;
}

void PascalParser::factor() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST factor_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case LPAREN:
		{
			{
			match(LPAREN);
			expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(RPAREN);
			}
			factor_AST = RefPascalAST(currentAST.root);
			break;
		}
		case NUM_INT:
		case STRING_LITERAL:
		case CHR:
		case NIL:
		case NUM_REAL:
		{
			unsignedConstant();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			factor_AST = RefPascalAST(currentAST.root);
			break;
		}
		case NOT:
		{
			{
			match(NOT);
			factor();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			}
			factor_AST = RefPascalAST(currentAST.root);
			break;
		}
		case PLUS:
		case MINUS:
		{
			{
			{
			switch ( LA(1)) {
			case PLUS:
			{
				match(PLUS);
				break;
			}
			case MINUS:
			{
				match(MINUS);
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			factor();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			}
			factor_AST = RefPascalAST(currentAST.root);
			break;
		}
		case LBRACK:
		{
			setConstructor();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			factor_AST = RefPascalAST(currentAST.root);
			break;
		}
		case AT:
		{
			addressFactor();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			factor_AST = RefPascalAST(currentAST.root);
			break;
		}
		case TRUE:
		{
			RefPascalAST tmp252_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp252_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp252_AST));
			}
			match(TRUE);
			factor_AST = RefPascalAST(currentAST.root);
			break;
		}
		case FALSE:
		{
			RefPascalAST tmp253_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp253_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp253_AST));
			}
			match(FALSE);
			factor_AST = RefPascalAST(currentAST.root);
			break;
		}
		default:
			if ((LA(1) == IDENT) && (_tokenSet_60.member(LA(2)))) {
				identifierOrValueTypecastOrFunctionCall();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				factor_AST = RefPascalAST(currentAST.root);
			}
			else if ((LA(1) == IDENT) && (LA(2) == LBRACK)) {
				identifier();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				match(LBRACK);
				expression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				{ // ( ... )*
				for (;;) {
					if ((LA(1) == COMMA)) {
						match(COMMA);
						expression();
						if (inputState->guessing==0) {
							astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
						}
					}
					else {
						goto _loop274;
					}
					
				}
				_loop274:;
				} // ( ... )*
				match(RBRACK);
				factor_AST = RefPascalAST(currentAST.root);
			}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_61);
		} else {
			throw;
		}
	}
	returnAST = factor_AST;
}

void PascalParser::identifierOrValueTypecastOrFunctionCall() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST identifierOrValueTypecastOrFunctionCall_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		bool synPredMatched277 = false;
		if (((LA(1) == IDENT) && (LA(2) == LPAREN))) {
			int _m277 = mark();
			synPredMatched277 = true;
			inputState->guessing++;
			try {
				{
				identifier();
				match(LPAREN);
				expression();
				match(COMMA);
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched277 = false;
			}
			rewind(_m277);
			inputState->guessing--;
		}
		if ( synPredMatched277 ) {
			identifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(LPAREN);
			expressions();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(RPAREN);
			identifierOrValueTypecastOrFunctionCall_AST = RefPascalAST(currentAST.root);
		}
		else {
			bool synPredMatched279 = false;
			if (((LA(1) == IDENT) && (LA(2) == LPAREN))) {
				int _m279 = mark();
				synPredMatched279 = true;
				inputState->guessing++;
				try {
					{
					identifier();
					match(LPAREN);
					expression();
					match(RPAREN);
					}
				}
				catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
					synPredMatched279 = false;
				}
				rewind(_m279);
				inputState->guessing--;
			}
			if ( synPredMatched279 ) {
				identifier();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				match(LPAREN);
				expression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				match(RPAREN);
				identifierOrValueTypecastOrFunctionCall_AST = RefPascalAST(currentAST.root);
			}
			else if ((LA(1) == IDENT) && (_tokenSet_61.member(LA(2)))) {
				identifier();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				identifierOrValueTypecastOrFunctionCall_AST = RefPascalAST(currentAST.root);
			}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_61);
		} else {
			throw;
		}
	}
	returnAST = identifierOrValueTypecastOrFunctionCall_AST;
}

void PascalParser::unsignedConstant() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST unsignedConstant_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case NUM_INT:
		case NUM_REAL:
		{
			unsignedNumber();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			unsignedConstant_AST = RefPascalAST(currentAST.root);
			break;
		}
		case CHR:
		{
			constantChr();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			unsignedConstant_AST = RefPascalAST(currentAST.root);
			break;
		}
		case STRING_LITERAL:
		{
			string();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			unsignedConstant_AST = RefPascalAST(currentAST.root);
			break;
		}
		case NIL:
		{
			match(NIL);
			unsignedConstant_AST = RefPascalAST(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_61);
		} else {
			throw;
		}
	}
	returnAST = unsignedConstant_AST;
}

void PascalParser::setConstructor() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST setConstructor_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(LBRACK);
		{
		switch ( LA(1)) {
		case LPAREN:
		case NUM_INT:
		case LBRACK:
		case PLUS:
		case MINUS:
		case NOT:
		case TRUE:
		case FALSE:
		case AT:
		case STRING_LITERAL:
		case CHR:
		case NIL:
		case NUM_REAL:
		case IDENT:
		{
			setGroup();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{ // ( ... )*
			for (;;) {
				if ((LA(1) == COMMA)) {
					match(COMMA);
					setGroup();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
				}
				else {
					goto _loop290;
				}
				
			}
			_loop290:;
			} // ( ... )*
			break;
		}
		case RBRACK:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(RBRACK);
		setConstructor_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_61);
		} else {
			throw;
		}
	}
	returnAST = setConstructor_AST;
}

void PascalParser::addressFactor() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST addressFactor_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(AT);
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		addressFactor_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_61);
		} else {
			throw;
		}
	}
	returnAST = addressFactor_AST;
}

void PascalParser::expressions() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST expressions_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				expression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop286;
			}
			
		}
		_loop286:;
		} // ( ... )*
		expressions_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_42);
		} else {
			throw;
		}
	}
	returnAST = expressions_AST;
}

void PascalParser::functionCall() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST functionCall_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case LPAREN:
		{
			actualParameterList();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case SEMI:
		case END:
		case FINALIZATION:
		case AT:
		case ELSE:
		case UNTIL:
		case EXCEPT:
		case FINALLY:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		functionCall_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_62);
		} else {
			throw;
		}
	}
	returnAST = functionCall_AST;
}

void PascalParser::actualParameterList() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST actualParameterList_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(LPAREN);
		{
		switch ( LA(1)) {
		case LPAREN:
		case NUM_INT:
		case LBRACK:
		case PLUS:
		case MINUS:
		case NOT:
		case TRUE:
		case FALSE:
		case AT:
		case STRING_LITERAL:
		case CHR:
		case NIL:
		case NUM_REAL:
		case IDENT:
		{
			expressions();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case RPAREN:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(RPAREN);
		actualParameterList_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_62);
		} else {
			throw;
		}
	}
	returnAST = actualParameterList_AST;
}

void PascalParser::setGroup() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST setGroup_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case DOT:
		{
			match(DOT);
			match(DOT);
			expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case COMMA:
		case RBRACK:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		setGroup_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_45);
		} else {
			throw;
		}
	}
	returnAST = setGroup_AST;
}

void PascalParser::valueTypecast() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST valueTypecast_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		typeIdentifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(LPAREN);
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(RPAREN);
		valueTypecast_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_0);
		} else {
			throw;
		}
	}
	returnAST = valueTypecast_AST;
}

void PascalParser::simpleStatement() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST simpleStatement_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case GOTO:
		{
			gotoStatement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			simpleStatement_AST = RefPascalAST(currentAST.root);
			break;
		}
		case RAISE:
		{
			raiseStatement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			simpleStatement_AST = RefPascalAST(currentAST.root);
			break;
		}
		default:
			if ((LA(1) == IDENT) && (_tokenSet_63.member(LA(2)))) {
				assignmentStatement();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				simpleStatement_AST = RefPascalAST(currentAST.root);
			}
			else if ((LA(1) == IDENT) && (_tokenSet_64.member(LA(2)))) {
				procedureStatement();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				simpleStatement_AST = RefPascalAST(currentAST.root);
			}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_16);
		} else {
			throw;
		}
	}
	returnAST = simpleStatement_AST;
}

void PascalParser::structuredStatement() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST structuredStatement_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case BEGIN:
		{
			compoundStatement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			structuredStatement_AST = RefPascalAST(currentAST.root);
			break;
		}
		case WHILE:
		case REPEAT:
		case FOR:
		{
			repetitiveStatement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			structuredStatement_AST = RefPascalAST(currentAST.root);
			break;
		}
		case CASE:
		case IF:
		{
			conditionalStatement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			structuredStatement_AST = RefPascalAST(currentAST.root);
			break;
		}
		case TRY:
		{
			exceptionStatement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			structuredStatement_AST = RefPascalAST(currentAST.root);
			break;
		}
		case WITH:
		{
			withStatement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			structuredStatement_AST = RefPascalAST(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_16);
		} else {
			throw;
		}
	}
	returnAST = structuredStatement_AST;
}

void PascalParser::assignmentStatement() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST assignmentStatement_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifierOrArrayIdentifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		assignmentOperator();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		assignmentStatement_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_16);
		} else {
			throw;
		}
	}
	returnAST = assignmentStatement_AST;
}

void PascalParser::procedureStatement() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST procedureStatement_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case LPAREN:
		{
			actualParameterList();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case SEMI:
		case END:
		case FINALIZATION:
		case ELSE:
		case UNTIL:
		case EXCEPT:
		case FINALLY:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		procedureStatement_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_16);
		} else {
			throw;
		}
	}
	returnAST = procedureStatement_AST;
}

void PascalParser::gotoStatement() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST gotoStatement_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(GOTO);
		label();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		gotoStatement_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_16);
		} else {
			throw;
		}
	}
	returnAST = gotoStatement_AST;
}

void PascalParser::raiseStatement() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST raiseStatement_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp274_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp274_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp274_AST));
		}
		match(RAISE);
		{
		switch ( LA(1)) {
		case IDENT:
		{
			functionCall();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{
			switch ( LA(1)) {
			case AT:
			{
				match(AT);
				addressConstant();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case SEMI:
			case END:
			case FINALIZATION:
			case ELSE:
			case UNTIL:
			case EXCEPT:
			case FINALLY:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			break;
		}
		case SEMI:
		case END:
		case FINALIZATION:
		case ELSE:
		case UNTIL:
		case EXCEPT:
		case FINALLY:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		raiseStatement_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_16);
		} else {
			throw;
		}
	}
	returnAST = raiseStatement_AST;
}

void PascalParser::identifierOrArrayIdentifier() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST identifierOrArrayIdentifier_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		if ((LA(1) == IDENT) && (_tokenSet_65.member(LA(2)))) {
			identifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			identifierOrArrayIdentifier_AST = RefPascalAST(currentAST.root);
		}
		else if ((LA(1) == IDENT) && (LA(2) == LBRACK)) {
			identifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(LBRACK);
			expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{ // ( ... )*
			for (;;) {
				if ((LA(1) == COMMA)) {
					match(COMMA);
					expression();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
				}
				else {
					goto _loop302;
				}
				
			}
			_loop302:;
			} // ( ... )*
			match(RBRACK);
			identifierOrArrayIdentifier_AST = RefPascalAST(currentAST.root);
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_65);
		} else {
			throw;
		}
	}
	returnAST = identifierOrArrayIdentifier_AST;
}

void PascalParser::assignmentOperator() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST assignmentOperator_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case ASSIGN:
		{
			match(ASSIGN);
			assignmentOperator_AST = RefPascalAST(currentAST.root);
			break;
		}
		case PLUSEQ:
		{
			RefPascalAST tmp280_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp280_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp280_AST));
			}
			match(PLUSEQ);
			assignmentOperator_AST = RefPascalAST(currentAST.root);
			break;
		}
		case MINUSEQ:
		{
			RefPascalAST tmp281_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp281_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp281_AST));
			}
			match(MINUSEQ);
			assignmentOperator_AST = RefPascalAST(currentAST.root);
			break;
		}
		case STAREQ:
		{
			RefPascalAST tmp282_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp282_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp282_AST));
			}
			match(STAREQ);
			assignmentOperator_AST = RefPascalAST(currentAST.root);
			break;
		}
		case SLASHQE:
		{
			RefPascalAST tmp283_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp283_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp283_AST));
			}
			match(SLASHQE);
			assignmentOperator_AST = RefPascalAST(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_36);
		} else {
			throw;
		}
	}
	returnAST = assignmentOperator_AST;
}

void PascalParser::repetitiveStatement() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST repetitiveStatement_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case FOR:
		{
			forStatement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			repetitiveStatement_AST = RefPascalAST(currentAST.root);
			break;
		}
		case REPEAT:
		{
			repeatStatement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			repetitiveStatement_AST = RefPascalAST(currentAST.root);
			break;
		}
		case WHILE:
		{
			whileStatement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			repetitiveStatement_AST = RefPascalAST(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_16);
		} else {
			throw;
		}
	}
	returnAST = repetitiveStatement_AST;
}

void PascalParser::conditionalStatement() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST conditionalStatement_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case IF:
		{
			ifStatement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			conditionalStatement_AST = RefPascalAST(currentAST.root);
			break;
		}
		case CASE:
		{
			caseStatement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			conditionalStatement_AST = RefPascalAST(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_16);
		} else {
			throw;
		}
	}
	returnAST = conditionalStatement_AST;
}

void PascalParser::exceptionStatement() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST exceptionStatement_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		tryStatement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		exceptionStatement_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_16);
		} else {
			throw;
		}
	}
	returnAST = exceptionStatement_AST;
}

void PascalParser::withStatement() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST withStatement_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp284_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp284_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp284_AST));
		}
		match(WITH);
		recordVariableList();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(DO);
		statement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		withStatement_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_16);
		} else {
			throw;
		}
	}
	returnAST = withStatement_AST;
}

void PascalParser::ifStatement() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST ifStatement_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp286_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp286_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp286_AST));
		}
		match(IF);
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(THEN);
		statement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		if ((LA(1) == ELSE) && (_tokenSet_24.member(LA(2)))) {
			match(ELSE);
			statement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
		}
		else if ((_tokenSet_16.member(LA(1))) && (_tokenSet_66.member(LA(2)))) {
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
		ifStatement_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_16);
		} else {
			throw;
		}
	}
	returnAST = ifStatement_AST;
}

void PascalParser::caseStatement() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST caseStatement_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp289_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp289_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp289_AST));
		}
		match(CASE);
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(OF);
		caseListElement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == SEMI) && (_tokenSet_33.member(LA(2)))) {
				match(SEMI);
				caseListElement();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop320;
			}
			
		}
		_loop320:;
		} // ( ... )*
		{
		switch ( LA(1)) {
		case SEMI:
		{
			match(SEMI);
			match(ELSE);
			statement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{ // ( ... )*
			for (;;) {
				if ((LA(1) == SEMI)) {
					match(SEMI);
					statement();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
				}
				else {
					goto _loop323;
				}
				
			}
			_loop323:;
			} // ( ... )*
			break;
		}
		case END:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(END);
		caseStatement_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_16);
		} else {
			throw;
		}
	}
	returnAST = caseStatement_AST;
}

void PascalParser::forStatement() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST forStatement_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp296_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp296_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp296_AST));
		}
		match(FOR);
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(ASSIGN);
		forList();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(DO);
		statement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		forStatement_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_16);
		} else {
			throw;
		}
	}
	returnAST = forStatement_AST;
}

void PascalParser::repeatStatement() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST repeatStatement_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp299_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp299_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp299_AST));
		}
		match(REPEAT);
		statement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == SEMI)) {
				match(SEMI);
				{
				switch ( LA(1)) {
				case BEGIN:
				case NUM_INT:
				case CASE:
				case GOTO:
				case IF:
				case WHILE:
				case REPEAT:
				case FOR:
				case WITH:
				case RAISE:
				case TRY:
				case IDENT:
				{
					statement();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					break;
				}
				case SEMI:
				case UNTIL:
				{
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
			}
			else {
				goto _loop332;
			}
			
		}
		_loop332:;
		} // ( ... )*
		match(UNTIL);
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		repeatStatement_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_16);
		} else {
			throw;
		}
	}
	returnAST = repeatStatement_AST;
}

void PascalParser::whileStatement() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST whileStatement_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp302_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp302_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp302_AST));
		}
		match(WHILE);
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(DO);
		statement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		whileStatement_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_16);
		} else {
			throw;
		}
	}
	returnAST = whileStatement_AST;
}

void PascalParser::caseListElement() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST caseListElement_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		constList();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		RefPascalAST tmp304_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp304_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp304_AST));
		}
		match(COLON);
		statement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		caseListElement_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_67);
		} else {
			throw;
		}
	}
	returnAST = caseListElement_AST;
}

void PascalParser::constList() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST constList_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		constant();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				constant();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop327;
			}
			
		}
		_loop327:;
		} // ( ... )*
		if ( inputState->guessing==0 ) {
			constList_AST = RefPascalAST(currentAST.root);
#line 746 "pascal.g"
			constList_AST = RefPascalAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(CONSTLIST)))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(constList_AST))));
#line 8219 "PascalParser.cpp"
			currentAST.root = constList_AST;
			if ( constList_AST!=RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				constList_AST->getFirstChild() != RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = constList_AST->getFirstChild();
			else
				currentAST.child = constList_AST;
			currentAST.advanceChildToEnd();
		}
		constList_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_55);
		} else {
			throw;
		}
	}
	returnAST = constList_AST;
}

void PascalParser::forList() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST forList_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		initialValue();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case TO:
		{
			RefPascalAST tmp306_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp306_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp306_AST));
			}
			match(TO);
			break;
		}
		case DOWNTO:
		{
			RefPascalAST tmp307_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp307_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp307_AST));
			}
			match(DOWNTO);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		finalValue();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		forList_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_68);
		} else {
			throw;
		}
	}
	returnAST = forList_AST;
}

void PascalParser::initialValue() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST initialValue_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		initialValue_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_69);
		} else {
			throw;
		}
	}
	returnAST = initialValue_AST;
}

void PascalParser::finalValue() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST finalValue_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		finalValue_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_68);
		} else {
			throw;
		}
	}
	returnAST = finalValue_AST;
}

void PascalParser::recordVariableList() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST recordVariableList_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		variable();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				variable();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop341;
			}
			
		}
		_loop341:;
		} // ( ... )*
		recordVariableList_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_68);
		} else {
			throw;
		}
	}
	returnAST = recordVariableList_AST;
}

/** A variable is an id with a suffix and can look like:
 *  id
 *  id[expr,...]
 *  id.id
 *  id.id[expr,...]
 *  id^
 *  id^.id
 *  id^.id[expr,...]
 *  ...
 *
 *  LL has a really hard time with this construct as it's naturally
 *  left-recursive.  We have to turn into a simple loop rather than
 *  recursive loop, hence, the suffixes.  I keep in the same rule
 *  for easy tree construction.
 */
void PascalParser::variable() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST variable_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case AT:
		{
			RefPascalAST tmp309_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp309_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp309_AST));
			}
			match(AT);
			identifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case IDENT:
		{
			identifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		{ // ( ... )*
		for (;;) {
			switch ( LA(1)) {
			case LBRACK:
			{
				RefPascalAST tmp310_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp310_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp310_AST));
				}
				match(LBRACK);
				expression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				{ // ( ... )*
				for (;;) {
					if ((LA(1) == COMMA)) {
						match(COMMA);
						expression();
						if (inputState->guessing==0) {
							astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
						}
					}
					else {
						goto _loop346;
					}
					
				}
				_loop346:;
				} // ( ... )*
				match(RBRACK);
				break;
			}
			case LBRACK2:
			{
				RefPascalAST tmp313_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp313_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp313_AST));
				}
				match(LBRACK2);
				expression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				{ // ( ... )*
				for (;;) {
					if ((LA(1) == COMMA)) {
						match(COMMA);
						expression();
						if (inputState->guessing==0) {
							astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
						}
					}
					else {
						goto _loop348;
					}
					
				}
				_loop348:;
				} // ( ... )*
				match(RBRACK2);
				break;
			}
			case DOT:
			{
				RefPascalAST tmp316_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp316_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp316_AST));
				}
				match(DOT);
				identifier();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case POINTER:
			{
				RefPascalAST tmp317_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp317_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp317_AST));
				}
				match(POINTER);
				break;
			}
			default:
			{
				goto _loop349;
			}
			}
		}
		_loop349:;
		} // ( ... )*
		variable_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_70);
		} else {
			throw;
		}
	}
	returnAST = variable_AST;
}

void PascalParser::operatorDefinition() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST operatorDefinition_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp318_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp318_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp318_AST));
		}
		match(OPERATOR);
		{
		switch ( LA(1)) {
		case ASSIGN:
		{
			assignmentOperatorDefinition();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case PLUS:
		case MINUS:
		case STAR:
		case SLASH:
		{
			arithmeticOperatorDefinition();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case EQUAL:
		case LE:
		case GE:
		case LTH:
		case GT:
		{
			comparisonOperatorDefinition();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(COLON);
		type();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(SEMI);
		subroutineBlock();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		operatorDefinition_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_0);
		} else {
			throw;
		}
	}
	returnAST = operatorDefinition_AST;
}

void PascalParser::assignmentOperatorDefinition() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST assignmentOperatorDefinition_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(ASSIGN);
		match(LPAREN);
		valueParameter();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(RPAREN);
		assignmentOperatorDefinition_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_71);
		} else {
			throw;
		}
	}
	returnAST = assignmentOperatorDefinition_AST;
}

void PascalParser::arithmeticOperatorDefinition() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST arithmeticOperatorDefinition_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case PLUS:
		{
			match(PLUS);
			break;
		}
		case MINUS:
		{
			match(MINUS);
			break;
		}
		case SLASH:
		{
			match(SLASH);
			break;
		}
		default:
			if ((LA(1) == STAR) && (LA(2) == LPAREN)) {
				match(STAR);
			}
			else if ((LA(1) == STAR) && (LA(2) == STAR)) {
				{
				match(STAR);
				match(STAR);
				}
			}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(LPAREN);
		formalParameterList();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(RPAREN);
		arithmeticOperatorDefinition_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_71);
		} else {
			throw;
		}
	}
	returnAST = arithmeticOperatorDefinition_AST;
}

void PascalParser::comparisonOperatorDefinition() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST comparisonOperatorDefinition_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case EQUAL:
		{
			match(EQUAL);
			break;
		}
		case LE:
		{
			match(LE);
			break;
		}
		case GE:
		{
			match(GE);
			break;
		}
		case GT:
		{
			match(GT);
			break;
		}
		case LTH:
		{
			match(LTH);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(LPAREN);
		formalParameterList();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(RPAREN);
		comparisonOperatorDefinition_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_71);
		} else {
			throw;
		}
	}
	returnAST = comparisonOperatorDefinition_AST;
}

void PascalParser::tryStatement() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST tryStatement_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp339_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp339_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp339_AST));
		}
		match(TRY);
		{
		switch ( LA(1)) {
		case BEGIN:
		case NUM_INT:
		case CASE:
		case GOTO:
		case IF:
		case WHILE:
		case REPEAT:
		case FOR:
		case WITH:
		case RAISE:
		case TRY:
		case IDENT:
		{
			statements();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case EXCEPT:
		case FINALLY:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		exceptOrFinallyPart();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(END);
		tryStatement_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_16);
		} else {
			throw;
		}
	}
	returnAST = tryStatement_AST;
}

void PascalParser::statements() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST statements_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		statement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == SEMI)) {
				match(SEMI);
				statement();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop369;
			}
			
		}
		_loop369:;
		} // ( ... )*
		statements_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_72);
		} else {
			throw;
		}
	}
	returnAST = statements_AST;
}

void PascalParser::exceptOrFinallyPart() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST exceptOrFinallyPart_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case EXCEPT:
		{
			match(EXCEPT);
			{
			switch ( LA(1)) {
			case BEGIN:
			case NUM_INT:
			case CASE:
			case GOTO:
			case IF:
			case WHILE:
			case REPEAT:
			case FOR:
			case WITH:
			case RAISE:
			case TRY:
			case ON:
			case IDENT:
			{
				exceptionHandlers();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case END:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			exceptOrFinallyPart_AST = RefPascalAST(currentAST.root);
			break;
		}
		case FINALLY:
		{
			match(FINALLY);
			{
			switch ( LA(1)) {
			case BEGIN:
			case NUM_INT:
			case CASE:
			case GOTO:
			case IF:
			case WHILE:
			case REPEAT:
			case FOR:
			case WITH:
			case RAISE:
			case TRY:
			case IDENT:
			{
				statements();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case END:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			exceptOrFinallyPart_AST = RefPascalAST(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_6);
		} else {
			throw;
		}
	}
	returnAST = exceptOrFinallyPart_AST;
}

void PascalParser::exceptionHandlers() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST exceptionHandlers_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case BEGIN:
		case NUM_INT:
		case CASE:
		case GOTO:
		case IF:
		case WHILE:
		case REPEAT:
		case FOR:
		case WITH:
		case RAISE:
		case TRY:
		case IDENT:
		{
			statements();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			exceptionHandlers_AST = RefPascalAST(currentAST.root);
			break;
		}
		case ON:
		{
			exceptionHandler();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{ // ( ... )*
			for (;;) {
				if ((LA(1) == SEMI)) {
					match(SEMI);
					exceptionHandler();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
				}
				else {
					goto _loop372;
				}
				
			}
			_loop372:;
			} // ( ... )*
			{
			switch ( LA(1)) {
			case ELSE:
			{
				match(ELSE);
				statements();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case END:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			exceptionHandlers_AST = RefPascalAST(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_6);
		} else {
			throw;
		}
	}
	returnAST = exceptionHandlers_AST;
}

void PascalParser::exceptionHandler() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST exceptionHandler_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(ON);
		{
		if ((LA(1) == IDENT) && (LA(2) == COLON)) {
			identifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(COLON);
		}
		else if ((LA(1) == IDENT) && (LA(2) == DO)) {
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(DO);
		statement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		exceptionHandler_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_73);
		} else {
			throw;
		}
	}
	returnAST = exceptionHandler_AST;
}

void PascalParser::sign() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST sign_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case PLUS:
		{
			RefPascalAST tmp349_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp349_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp349_AST));
			}
			match(PLUS);
			sign_AST = RefPascalAST(currentAST.root);
			break;
		}
		case MINUS:
		{
			RefPascalAST tmp350_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp350_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp350_AST));
			}
			match(MINUS);
			sign_AST = RefPascalAST(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_74);
		} else {
			throw;
		}
	}
	returnAST = sign_AST;
}

void PascalParser::constantChr() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST constantChr_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp351_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp351_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp351_AST));
		}
		match(CHR);
		match(LPAREN);
		unsignedInteger();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(RPAREN);
		constantChr_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_75);
		} else {
			throw;
		}
	}
	returnAST = constantChr_AST;
}

void PascalParser::unsignedNumber() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST unsignedNumber_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case NUM_INT:
		{
			unsignedInteger();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			unsignedNumber_AST = RefPascalAST(currentAST.root);
			break;
		}
		case NUM_REAL:
		{
			unsignedReal();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			unsignedNumber_AST = RefPascalAST(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_76);
		} else {
			throw;
		}
	}
	returnAST = unsignedNumber_AST;
}

void PascalParser::unsignedReal() {
	returnAST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST unsignedReal_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp354_AST = RefPascalAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp354_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp354_AST));
		}
		match(NUM_REAL);
		unsignedReal_AST = RefPascalAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_76);
		} else {
			throw;
		}
	}
	returnAST = unsignedReal_AST;
}

void PascalParser::initializeASTFactory( ANTLR_USE_NAMESPACE(antlr)ASTFactory& factory )
{
	factory.setMaxNodeType(189);
}
const char* PascalParser::tokenNames[] = {
	"<0>",
	"EOF",
	"<2>",
	"NULL_TREE_LOOKAHEAD",
	"BLOCK",
	"IDLIST",
	"ELIST",
	"FUNC_CALL",
	"PROC_CALL",
	"SCALARTYPE",
	"TYPELIST",
	"VARIANT_TAG",
	"VARIANT_TAG_NO_ID",
	"VARIANT_CASE",
	"CONSTLIST",
	"FIELDLIST",
	"ARGDECLS",
	"VARDECL",
	"ARGDECL",
	"ARGLIST",
	"TYPEDECL",
	"FIELD",
	"DOT",
	"\"library\"",
	"SEMI",
	"\"end\"",
	"\"exports\"",
	"COMMA",
	"\"index\"",
	"\"name\"",
	"\"uses\"",
	"\"unit\"",
	"\"interface\"",
	"\"implementation\"",
	"\"begin\"",
	"\"program\"",
	"LPAREN",
	"RPAREN",
	"\"initialization\"",
	"\"finalization\"",
	"\"label\"",
	"\"const\"",
	"\"resourcestring\"",
	"EQUAL",
	"\"type\"",
	"\"var\"",
	"COLON",
	"\"procedure\"",
	"\"forward\"",
	"\"function\"",
	"\"array\"",
	"\"of\"",
	"\"external\"",
	"\"public\"",
	"\"alias\"",
	"INTERRUPT",
	"\"export\"",
	"\"register\"",
	"\"pascal\"",
	"\"cdecl\"",
	"\"stdcall\"",
	"\"popstack\"",
	"\"saveregisters\"",
	"\"inline\"",
	"\"safecall\"",
	"\"near\"",
	"\"far\"",
	"NUM_INT",
	"\"integer\"",
	"\"shortint\"",
	"\"smallint\"",
	"\"longint\"",
	"\"int64\"",
	"\"byte\"",
	"\"word\"",
	"\"cardinal\"",
	"\"qword\"",
	"\"boolean\"",
	"BYTEBOOL",
	"LONGBOOL",
	"\"char\"",
	"DOTDOT",
	"ASSIGN",
	"\"real\"",
	"\"single\"",
	"\"double\"",
	"\"extended\"",
	"\"comp\"",
	"\"string\"",
	"LBRACK",
	"RBRACK",
	"\"packed\"",
	"\"record\"",
	"\"case\"",
	"\"set\"",
	"\"file\"",
	"POINTER",
	"\"object\"",
	"\"virtual\"",
	"\"abstract\"",
	"\"private\"",
	"\"protected\"",
	"\"constructor\"",
	"\"destructor\"",
	"\"class\"",
	"\"override\"",
	"MESSAGE",
	"\"published\"",
	"\"property\"",
	"\"read\"",
	"\"write\"",
	"\"default\"",
	"\"nodefault\"",
	"LE",
	"GE",
	"LTH",
	"GT",
	"NOT_EQUAL",
	"\"in\"",
	"\"is\"",
	"PLUS",
	"MINUS",
	"\"or\"",
	"\"xor\"",
	"STAR",
	"SLASH",
	"\"div\"",
	"\"mod\"",
	"\"and\"",
	"\"shl\"",
	"\"shr\"",
	"\"not\"",
	"\"true\"",
	"\"false\"",
	"AT",
	"PLUSEQ",
	"MINUSEQ",
	"STAREQ",
	"SLASHQE",
	"\"goto\"",
	"\"if\"",
	"\"then\"",
	"\"else\"",
	"\"while\"",
	"\"do\"",
	"\"repeat\"",
	"\"until\"",
	"\"for\"",
	"\"to\"",
	"\"downto\"",
	"\"with\"",
	"LBRACK2",
	"RBRACK2",
	"\"operator\"",
	"\"raise\"",
	"\"try\"",
	"\"except\"",
	"\"finally\"",
	"\"on\"",
	"STRING_LITERAL",
	"\"chr\"",
	"\"nil\"",
	"NUM_REAL",
	"IDENT",
	"\"absolute\"",
	"\"as\"",
	"\"asm\"",
	"\"assembler\"",
	"\"break\"",
	"\"continue\"",
	"\"dispose\"",
	"\"exit\"",
	"\"inherited\"",
	"\"new\"",
	"\"self\"",
	"METHOD",
	"ADDSUBOR",
	"ASSIGNEQUAL",
	"SIGN",
	"FUNC",
	"NODE_NOT_EMIT",
	"MYASTVAR",
	"LF",
	"LCURLY",
	"RCURLY",
	"WS",
	"COMMENT_1",
	"COMMENT_2",
	"COMMENT_3",
	"EXPONENT",
	0
};

const unsigned long PascalParser::_tokenSet_0_data_[] = { 2UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_0(_tokenSet_0_data_,6);
const unsigned long PascalParser::_tokenSet_1_data_[] = { 1073741824UL, 177924UL, 0UL, 192UL, 0UL, 0UL, 0UL, 0UL };
// "uses" "begin" "label" "const" "resourcestring" "type" "var" "procedure" 
// "function" "constructor" "destructor" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_1(_tokenSet_1_data_,8);
const unsigned long PascalParser::_tokenSet_2_data_[] = { 100663296UL, 177990UL, 0UL, 192UL, 0UL, 0UL, 0UL, 0UL };
// "end" "exports" "implementation" "begin" "initialization" "label" "const" 
// "resourcestring" "type" "var" "procedure" "function" "constructor" "destructor" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_2(_tokenSet_2_data_,8);
const unsigned long PascalParser::_tokenSet_3_data_[] = { 121634818UL, 177988UL, 0UL, 192UL, 0UL, 0UL, 0UL, 0UL };
// EOF DOT SEMI "end" "exports" "begin" "initialization" "label" "const" 
// "resourcestring" "type" "var" "procedure" "function" "constructor" "destructor" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_3(_tokenSet_3_data_,8);
const unsigned long PascalParser::_tokenSet_4_data_[] = { 994050050UL, 706736UL, 101056512UL, 4294963649UL, 833972167UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF DOT SEMI "end" COMMA "index" "name" LPAREN RPAREN "finalization" 
// EQUAL COLON "procedure" "function" "of" DOTDOT ASSIGN LBRACK RBRACK 
// POINTER "constructor" "destructor" "class" "property" "read" "write" 
// "default" "nodefault" LE GE LTH GT NOT_EQUAL "in" "is" PLUS MINUS "or" 
// "xor" STAR SLASH "div" "mod" "and" "shl" "shr" AT PLUSEQ MINUSEQ STAREQ 
// SLASHQE "then" "else" "do" "until" "to" "downto" LBRACK2 RBRACK2 "except" 
// "finally" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_4(_tokenSet_4_data_,12);
const unsigned long PascalParser::_tokenSet_5_data_[] = { 67108864UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "exports" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_5(_tokenSet_5_data_,6);
const unsigned long PascalParser::_tokenSet_6_data_[] = { 33554432UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "end" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_6(_tokenSet_6_data_,6);
const unsigned long PascalParser::_tokenSet_7_data_[] = { 100663296UL, 68UL, 0UL, 0UL, 0UL, 0UL };
// "end" "exports" "begin" "initialization" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_7(_tokenSet_7_data_,6);
const unsigned long PascalParser::_tokenSet_8_data_[] = { 184549376UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI "end" COMMA 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_8(_tokenSet_8_data_,6);
const unsigned long PascalParser::_tokenSet_9_data_[] = { 788529154UL, 177988UL, 0UL, 127424UL, 0UL, 0UL, 0UL, 0UL };
// EOF SEMI "end" "exports" COMMA "name" "begin" "initialization" "label" 
// "const" "resourcestring" "type" "var" "procedure" "function" "constructor" 
// "destructor" "class" "property" "read" "write" "default" "nodefault" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_9(_tokenSet_9_data_,8);
const unsigned long PascalParser::_tokenSet_10_data_[] = { 1056964610UL, 177988UL, 0UL, 192UL, 0UL, 0UL, 0UL, 0UL };
// EOF SEMI "end" "exports" COMMA "index" "name" "begin" "initialization" 
// "label" "const" "resourcestring" "type" "var" "procedure" "function" 
// "constructor" "destructor" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_10(_tokenSet_10_data_,8);
const unsigned long PascalParser::_tokenSet_11_data_[] = { 16777216UL, 16416UL, 67108864UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RPAREN COLON RBRACK 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_11(_tokenSet_11_data_,8);
const unsigned long PascalParser::_tokenSet_12_data_[] = { 0UL, 2UL, 0UL, 0UL, 0UL, 0UL };
// "implementation" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_12(_tokenSet_12_data_,6);
const unsigned long PascalParser::_tokenSet_13_data_[] = { 33554432UL, 68UL, 0UL, 0UL, 0UL, 0UL };
// "end" "begin" "initialization" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_13(_tokenSet_13_data_,6);
const unsigned long PascalParser::_tokenSet_14_data_[] = { 33554432UL, 128UL, 0UL, 0UL, 0UL, 0UL };
// "end" "finalization" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_14(_tokenSet_14_data_,6);
const unsigned long PascalParser::_tokenSet_15_data_[] = { 0UL, 168450UL, 0UL, 0UL, 0UL, 0UL };
// "implementation" "const" "type" "procedure" "function" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_15(_tokenSet_15_data_,6);
const unsigned long PascalParser::_tokenSet_16_data_[] = { 50331648UL, 128UL, 0UL, 0UL, 805584896UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI "end" "finalization" "else" "until" "except" "finally" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_16(_tokenSet_16_data_,12);
const unsigned long PascalParser::_tokenSet_17_data_[] = { 100663296UL, 177988UL, 0UL, 192UL, 0UL, 0UL, 0UL, 0UL };
// "end" "exports" "begin" "initialization" "label" "const" "resourcestring" 
// "type" "var" "procedure" "function" "constructor" "destructor" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_17(_tokenSet_17_data_,8);
const unsigned long PascalParser::_tokenSet_18_data_[] = { 184549376UL, 16512UL, 0UL, 0UL, 805584896UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI "end" COMMA "finalization" COLON "else" "until" "except" "finally" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_18(_tokenSet_18_data_,12);
const unsigned long PascalParser::_tokenSet_19_data_[] = { 100663296UL, 177990UL, 0UL, 192UL, 0UL, 8UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "end" "exports" "implementation" "begin" "initialization" "label" "const" 
// "resourcestring" "type" "var" "procedure" "function" "constructor" "destructor" 
// IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_19(_tokenSet_19_data_,12);
const unsigned long PascalParser::_tokenSet_20_data_[] = { 100663296UL, 177988UL, 0UL, 192UL, 0UL, 8UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "end" "exports" "begin" "initialization" "label" "const" "resourcestring" 
// "type" "var" "procedure" "function" "constructor" "destructor" IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_20(_tokenSet_20_data_,12);
const unsigned long PascalParser::_tokenSet_21_data_[] = { 1061158914UL, 720868UL, 67239936UL, 4294840768UL, 825581575UL, 8UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF DOT SEMI "end" "exports" COMMA "index" "name" "begin" RPAREN "initialization" 
// "finalization" "label" "const" "resourcestring" EQUAL "type" "var" COLON 
// "procedure" "function" "of" DOTDOT RBRACK "constructor" "destructor" 
// "class" "property" LE GE LTH GT NOT_EQUAL "in" "is" PLUS MINUS "or" 
// "xor" STAR SLASH "div" "mod" "and" "shl" "shr" "then" "else" "do" "until" 
// "to" "downto" RBRACK2 "except" "finally" IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_21(_tokenSet_21_data_,12);
const unsigned long PascalParser::_tokenSet_22_data_[] = { 16777216UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_22(_tokenSet_22_data_,6);
const unsigned long PascalParser::_tokenSet_23_data_[] = { 16777216UL, 2080UL, 67108864UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RPAREN EQUAL RBRACK 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_23(_tokenSet_23_data_,8);
const unsigned long PascalParser::_tokenSet_24_data_[] = { 0UL, 4UL, 536870920UL, 0UL, 206215168UL, 8UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "begin" NUM_INT "case" "goto" "if" "while" "repeat" "for" "with" "raise" 
// "try" IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_24(_tokenSet_24_data_,12);
const unsigned long PascalParser::_tokenSet_25_data_[] = { 121634818UL, 178116UL, 0UL, 192UL, 805584896UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF DOT SEMI "end" "exports" "begin" "initialization" "finalization" 
// "label" "const" "resourcestring" "type" "var" "procedure" "function" 
// "constructor" "destructor" "else" "until" "except" "finally" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_25(_tokenSet_25_data_,12);
const unsigned long PascalParser::_tokenSet_26_data_[] = { 16777216UL, 1818406UL, 67108864UL, 192UL, 0UL, 0UL, 0UL, 0UL };
// SEMI "implementation" "begin" RPAREN "label" "const" "resourcestring" 
// EQUAL "type" "var" "procedure" "forward" "function" "of" "external" 
// RBRACK "constructor" "destructor" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_26(_tokenSet_26_data_,8);
const unsigned long PascalParser::_tokenSet_27_data_[] = { 117440514UL, 177988UL, 0UL, 192UL, 0UL, 0UL, 0UL, 0UL };
// EOF SEMI "end" "exports" "begin" "initialization" "label" "const" "resourcestring" 
// "type" "var" "procedure" "function" "constructor" "destructor" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_27(_tokenSet_27_data_,8);
const unsigned long PascalParser::_tokenSet_28_data_[] = { 16777216UL, 16400UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LPAREN COLON 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_28(_tokenSet_28_data_,6);
const unsigned long PascalParser::_tokenSet_29_data_[] = { 16777216UL, 16416UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RPAREN COLON 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_29(_tokenSet_29_data_,6);
const unsigned long PascalParser::_tokenSet_30_data_[] = { 16777216UL, 32UL, 67108864UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RPAREN RBRACK 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_30(_tokenSet_30_data_,8);
const unsigned long PascalParser::_tokenSet_31_data_[] = { 0UL, 426000UL, 3657039864UL, 50331907UL, 2147483648UL, 13UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LPAREN "procedure" "function" "array" NUM_INT "integer" "shortint" "smallint" 
// "longint" "int64" "byte" "word" "cardinal" "qword" "boolean" BYTEBOOL 
// LONGBOOL "char" "real" "single" "double" "extended" "comp" "string" 
// "packed" "record" "set" "file" POINTER "object" "class" PLUS MINUS STRING_LITERAL 
// "chr" NUM_REAL IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_31(_tokenSet_31_data_,12);
const unsigned long PascalParser::_tokenSet_32_data_[] = { 188743680UL, 524448UL, 67239936UL, 0UL, 825581568UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// DOT SEMI "end" COMMA RPAREN "finalization" "of" DOTDOT RBRACK "then" 
// "else" "do" "until" "to" "downto" RBRACK2 "except" "finally" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_32(_tokenSet_32_data_,12);
const unsigned long PascalParser::_tokenSet_33_data_[] = { 0UL, 0UL, 8UL, 50331648UL, 2147483648UL, 13UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// NUM_INT PLUS MINUS STRING_LITERAL "chr" NUM_REAL IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_33(_tokenSet_33_data_,12);
const unsigned long PascalParser::_tokenSet_34_data_[] = { 16777216UL, 16UL, 8UL, 0UL, 0UL, 12UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LPAREN NUM_INT NUM_REAL IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_34(_tokenSet_34_data_,12);
const unsigned long PascalParser::_tokenSet_35_data_[] = { 0UL, 16UL, 8UL, 50331648UL, 2147483648UL, 13UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LPAREN NUM_INT PLUS MINUS STRING_LITERAL "chr" NUM_REAL IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_35(_tokenSet_35_data_,12);
const unsigned long PascalParser::_tokenSet_36_data_[] = { 0UL, 16UL, 33554440UL, 50331648UL, 2147483768UL, 15UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LPAREN NUM_INT LBRACK PLUS MINUS "not" "true" "false" AT STRING_LITERAL 
// "chr" "nil" NUM_REAL IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_36(_tokenSet_36_data_,12);
const unsigned long PascalParser::_tokenSet_37_data_[] = { 16777216UL, 2064UL, 100663304UL, 4294836224UL, 2147483775UL, 15UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LPAREN EQUAL NUM_INT LBRACK RBRACK LE GE LTH GT NOT_EQUAL "in" 
// "is" PLUS MINUS "or" "xor" STAR SLASH "div" "mod" "and" "shl" "shr" 
// "not" "true" "false" AT STRING_LITERAL "chr" "nil" NUM_REAL IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_37(_tokenSet_37_data_,12);
const unsigned long PascalParser::_tokenSet_38_data_[] = { 184549378UL, 182304UL, 67239936UL, 4544UL, 0UL, 0UL, 0UL, 0UL };
// EOF SEMI "end" COMMA RPAREN EQUAL COLON "procedure" "function" DOTDOT 
// RBRACK "constructor" "destructor" "class" "property" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_38(_tokenSet_38_data_,8);
const unsigned long PascalParser::_tokenSet_39_data_[] = { 150994944UL, 32UL, 0UL, 0UL, 0UL, 0UL };
// SEMI COMMA RPAREN 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_39(_tokenSet_39_data_,6);
const unsigned long PascalParser::_tokenSet_40_data_[] = { 150994944UL, 2080UL, 67108864UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI COMMA RPAREN EQUAL RBRACK 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_40(_tokenSet_40_data_,8);
const unsigned long PascalParser::_tokenSet_41_data_[] = { 318767104UL, 165936UL, 67108864UL, 127424UL, 0UL, 0UL, 0UL, 0UL };
// SEMI "end" "index" LPAREN RPAREN EQUAL "procedure" "function" RBRACK 
// "constructor" "destructor" "class" "property" "read" "write" "default" 
// "nodefault" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_41(_tokenSet_41_data_,8);
const unsigned long PascalParser::_tokenSet_42_data_[] = { 0UL, 32UL, 0UL, 0UL, 0UL, 0UL };
// RPAREN 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_42(_tokenSet_42_data_,6);
const unsigned long PascalParser::_tokenSet_43_data_[] = { 792723458UL, 720868UL, 67239936UL, 4294963648UL, 825581575UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF DOT SEMI "end" "exports" COMMA "name" "begin" RPAREN "initialization" 
// "finalization" "label" "const" "resourcestring" EQUAL "type" "var" COLON 
// "procedure" "function" "of" DOTDOT RBRACK "constructor" "destructor" 
// "class" "property" "read" "write" "default" "nodefault" LE GE LTH GT 
// NOT_EQUAL "in" "is" PLUS MINUS "or" "xor" STAR SLASH "div" "mod" "and" 
// "shl" "shr" "then" "else" "do" "until" "to" "downto" RBRACK2 "except" 
// "finally" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_43(_tokenSet_43_data_,12);
const unsigned long PascalParser::_tokenSet_44_data_[] = { 0UL, 2064UL, 100794376UL, 4294836224UL, 2147483775UL, 15UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LPAREN EQUAL NUM_INT DOTDOT LBRACK RBRACK LE GE LTH GT NOT_EQUAL "in" 
// "is" PLUS MINUS "or" "xor" STAR SLASH "div" "mod" "and" "shl" "shr" 
// "not" "true" "false" AT STRING_LITERAL "chr" "nil" NUM_REAL IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_44(_tokenSet_44_data_,12);
const unsigned long PascalParser::_tokenSet_45_data_[] = { 134217728UL, 0UL, 67108864UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// COMMA RBRACK 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_45(_tokenSet_45_data_,8);
const unsigned long PascalParser::_tokenSet_46_data_[] = { 33554432UL, 32UL, 536870912UL, 0UL, 0UL, 8UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "end" RPAREN "case" IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_46(_tokenSet_46_data_,12);
const unsigned long PascalParser::_tokenSet_47_data_[] = { 50331648UL, 32UL, 536870912UL, 0UL, 0UL, 8UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI "end" RPAREN "case" IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_47(_tokenSet_47_data_,12);
const unsigned long PascalParser::_tokenSet_48_data_[] = { 33554432UL, 2260992UL, 0UL, 6640UL, 0UL, 8UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "end" "procedure" "function" "public" "private" "protected" "constructor" 
// "destructor" "class" "published" "property" IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_48(_tokenSet_48_data_,12);
const unsigned long PascalParser::_tokenSet_49_data_[] = { 0UL, 163840UL, 0UL, 192UL, 0UL, 0UL, 0UL, 0UL };
// "procedure" "function" "constructor" "destructor" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_49(_tokenSet_49_data_,8);
const unsigned long PascalParser::_tokenSet_50_data_[] = { 33554432UL, 163840UL, 0UL, 4544UL, 0UL, 8UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "end" "procedure" "function" "constructor" "destructor" "class" "property" 
// IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_50(_tokenSet_50_data_,12);
const unsigned long PascalParser::_tokenSet_51_data_[] = { 33554432UL, 163840UL, 0UL, 192UL, 0UL, 0UL, 0UL, 0UL };
// "end" "procedure" "function" "constructor" "destructor" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_51(_tokenSet_51_data_,8);
const unsigned long PascalParser::_tokenSet_52_data_[] = { 0UL, 163840UL, 0UL, 4544UL, 0UL, 0UL, 0UL, 0UL };
// "procedure" "function" "constructor" "destructor" "class" "property" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_52(_tokenSet_52_data_,8);
const unsigned long PascalParser::_tokenSet_53_data_[] = { 33554432UL, 163840UL, 0UL, 4544UL, 0UL, 0UL, 0UL, 0UL };
// "end" "procedure" "function" "constructor" "destructor" "class" "property" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_53(_tokenSet_53_data_,8);
const unsigned long PascalParser::_tokenSet_54_data_[] = { 33554432UL, 163840UL, 0UL, 127424UL, 0UL, 0UL, 0UL, 0UL };
// "end" "procedure" "function" "constructor" "destructor" "class" "property" 
// "read" "write" "default" "nodefault" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_54(_tokenSet_54_data_,8);
const unsigned long PascalParser::_tokenSet_55_data_[] = { 0UL, 16384UL, 0UL, 0UL, 0UL, 0UL };
// COLON 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_55(_tokenSet_55_data_,6);
const unsigned long PascalParser::_tokenSet_56_data_[] = { 33554432UL, 163840UL, 0UL, 119232UL, 0UL, 0UL, 0UL, 0UL };
// "end" "procedure" "function" "constructor" "destructor" "class" "property" 
// "write" "default" "nodefault" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_56(_tokenSet_56_data_,8);
const unsigned long PascalParser::_tokenSet_57_data_[] = { 33554432UL, 163840UL, 0UL, 102848UL, 0UL, 0UL, 0UL, 0UL };
// "end" "procedure" "function" "constructor" "destructor" "class" "property" 
// "default" "nodefault" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_57(_tokenSet_57_data_,8);
const unsigned long PascalParser::_tokenSet_58_data_[] = { 188743680UL, 526496UL, 67239936UL, 16646144UL, 825581568UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// DOT SEMI "end" COMMA RPAREN "finalization" EQUAL "of" DOTDOT RBRACK 
// LE GE LTH GT NOT_EQUAL "in" "is" "then" "else" "do" "until" "to" "downto" 
// RBRACK2 "except" "finally" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_58(_tokenSet_58_data_,12);
const unsigned long PascalParser::_tokenSet_59_data_[] = { 188743680UL, 526496UL, 67239936UL, 268304384UL, 825581568UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// DOT SEMI "end" COMMA RPAREN "finalization" EQUAL "of" DOTDOT RBRACK 
// LE GE LTH GT NOT_EQUAL "in" "is" PLUS MINUS "or" "xor" "then" "else" 
// "do" "until" "to" "downto" RBRACK2 "except" "finally" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_59(_tokenSet_59_data_,12);
const unsigned long PascalParser::_tokenSet_60_data_[] = { 188743680UL, 526512UL, 67239936UL, 4294836224UL, 825581575UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// DOT SEMI "end" COMMA LPAREN RPAREN "finalization" EQUAL "of" DOTDOT 
// RBRACK LE GE LTH GT NOT_EQUAL "in" "is" PLUS MINUS "or" "xor" STAR SLASH 
// "div" "mod" "and" "shl" "shr" "then" "else" "do" "until" "to" "downto" 
// RBRACK2 "except" "finally" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_60(_tokenSet_60_data_,12);
const unsigned long PascalParser::_tokenSet_61_data_[] = { 188743680UL, 526496UL, 67239936UL, 4294836224UL, 825581575UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// DOT SEMI "end" COMMA RPAREN "finalization" EQUAL "of" DOTDOT RBRACK 
// LE GE LTH GT NOT_EQUAL "in" "is" PLUS MINUS "or" "xor" STAR SLASH "div" 
// "mod" "and" "shl" "shr" "then" "else" "do" "until" "to" "downto" RBRACK2 
// "except" "finally" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_61(_tokenSet_61_data_,12);
const unsigned long PascalParser::_tokenSet_62_data_[] = { 50331648UL, 128UL, 0UL, 0UL, 805584960UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI "end" "finalization" AT "else" "until" "except" "finally" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_62(_tokenSet_62_data_,12);
const unsigned long PascalParser::_tokenSet_63_data_[] = { 0UL, 0UL, 33816576UL, 0UL, 1920UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// ASSIGN LBRACK PLUSEQ MINUSEQ STAREQ SLASHQE 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_63(_tokenSet_63_data_,12);
const unsigned long PascalParser::_tokenSet_64_data_[] = { 50331648UL, 144UL, 0UL, 0UL, 805584896UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI "end" LPAREN "finalization" "else" "until" "except" "finally" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_64(_tokenSet_64_data_,12);
const unsigned long PascalParser::_tokenSet_65_data_[] = { 0UL, 0UL, 262144UL, 0UL, 1920UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// ASSIGN PLUSEQ MINUSEQ STAREQ SLASHQE 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_65(_tokenSet_65_data_,12);
const unsigned long PascalParser::_tokenSet_66_data_[] = { 121634818UL, 178132UL, 570425352UL, 50331840UL, 4233025656UL, 15UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF DOT SEMI "end" "exports" "begin" LPAREN "initialization" "finalization" 
// "label" "const" "resourcestring" "type" "var" "procedure" "function" 
// NUM_INT LBRACK "case" "constructor" "destructor" PLUS MINUS "not" "true" 
// "false" AT "goto" "if" "else" "while" "repeat" "until" "for" "with" 
// "raise" "try" "except" "finally" "on" STRING_LITERAL "chr" "nil" NUM_REAL 
// IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_66(_tokenSet_66_data_,12);
const unsigned long PascalParser::_tokenSet_67_data_[] = { 50331648UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI "end" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_67(_tokenSet_67_data_,6);
const unsigned long PascalParser::_tokenSet_68_data_[] = { 0UL, 0UL, 0UL, 0UL, 65536UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "do" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_68(_tokenSet_68_data_,12);
const unsigned long PascalParser::_tokenSet_69_data_[] = { 0UL, 0UL, 0UL, 0UL, 3145728UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "to" "downto" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_69(_tokenSet_69_data_,12);
const unsigned long PascalParser::_tokenSet_70_data_[] = { 134217728UL, 0UL, 0UL, 0UL, 65536UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// COMMA "do" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_70(_tokenSet_70_data_,12);
const unsigned long PascalParser::_tokenSet_71_data_[] = { 0UL, 0UL, 0UL, 0UL, 0UL, 8UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_71(_tokenSet_71_data_,12);
const unsigned long PascalParser::_tokenSet_72_data_[] = { 33554432UL, 0UL, 0UL, 0UL, 805306368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "end" "except" "finally" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_72(_tokenSet_72_data_,12);
const unsigned long PascalParser::_tokenSet_73_data_[] = { 50331648UL, 0UL, 0UL, 0UL, 16384UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI "end" "else" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_73(_tokenSet_73_data_,12);
const unsigned long PascalParser::_tokenSet_74_data_[] = { 0UL, 0UL, 8UL, 0UL, 0UL, 12UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// NUM_INT NUM_REAL IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_74(_tokenSet_74_data_,12);
const unsigned long PascalParser::_tokenSet_75_data_[] = { 1061158914UL, 720868UL, 67239936UL, 4294840768UL, 825581575UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF DOT SEMI "end" "exports" COMMA "index" "name" "begin" RPAREN "initialization" 
// "finalization" "label" "const" "resourcestring" EQUAL "type" "var" COLON 
// "procedure" "function" "of" DOTDOT RBRACK "constructor" "destructor" 
// "class" "property" LE GE LTH GT NOT_EQUAL "in" "is" PLUS MINUS "or" 
// "xor" STAR SLASH "div" "mod" "and" "shl" "shr" "then" "else" "do" "until" 
// "to" "downto" RBRACK2 "except" "finally" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_75(_tokenSet_75_data_,12);
const unsigned long PascalParser::_tokenSet_76_data_[] = { 188743682UL, 706720UL, 67239936UL, 4294840768UL, 825581575UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF DOT SEMI "end" COMMA RPAREN "finalization" EQUAL COLON "procedure" 
// "function" "of" DOTDOT RBRACK "constructor" "destructor" "class" "property" 
// LE GE LTH GT NOT_EQUAL "in" "is" PLUS MINUS "or" "xor" STAR SLASH "div" 
// "mod" "and" "shl" "shr" "then" "else" "do" "until" "to" "downto" RBRACK2 
// "except" "finally" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_76(_tokenSet_76_data_,12);


