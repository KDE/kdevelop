/* $ANTLR 2.7.2: "pascal.g" -> "PascalParser.cpp"$ */
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
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST compilationUnit_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case PROGRAM:
		{
			program();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			compilationUnit_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case LIBRARY:
		{
			library();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			compilationUnit_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case UNIT:
		{
			unit();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			compilationUnit_AST = static_cast<RefPascalAST>(currentAST.root);
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
			consume();
			consumeUntil(_tokenSet_0);
		} else {
			throw;
		}
	}
	returnAST = compilationUnit_AST;
}

void PascalParser::program() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST program_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		programHeading();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{
		switch ( LA(1)) {
		case USES:
		{
			usesClause();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(DOT);
		program_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_0);
		} else {
			throw;
		}
	}
	returnAST = program_AST;
}

void PascalParser::library() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST library_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp2_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp2_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp2_AST));
		}
		match(LIBRARY);
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(SEMI);
		{
		switch ( LA(1)) {
		case USES:
		{
			usesClause();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		exportsClause();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(DOT);
		library_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_0);
		} else {
			throw;
		}
	}
	returnAST = library_AST;
}

void PascalParser::unit() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST unit_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp5_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp5_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp5_AST));
		}
		match(UNIT);
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(SEMI);
		interfacePart();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		implementationPart();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{
		switch ( LA(1)) {
		case INITIALIZATION:
		{
			{
			initializationPart();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			{
			switch ( LA(1)) {
			case FINALIZATION:
			{
				finalizationPart();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
		unit_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_0);
		} else {
			throw;
		}
	}
	returnAST = unit_AST;
}

void PascalParser::programHeading() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST programHeading_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp8_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp8_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp8_AST));
		}
		match(PROGRAM);
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{
		switch ( LA(1)) {
		case LPAREN:
		{
			match(LPAREN);
			identifierList();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
		programHeading_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_1);
		} else {
			throw;
		}
	}
	returnAST = programHeading_AST;
}

void PascalParser::usesClause() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST usesClause_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp12_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp12_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp12_AST));
		}
		match(USES);
		identifierList();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(SEMI);
		usesClause_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_2);
		} else {
			throw;
		}
	}
	returnAST = usesClause_AST;
}

void PascalParser::block() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST block_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		declarationPart();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		statementPart();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		block_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_3);
		} else {
			throw;
		}
	}
	returnAST = block_AST;
}

void PascalParser::identifier() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST identifier_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp14_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp14_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp14_AST));
		}
		match(IDENT);
		identifier_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_4);
		} else {
			throw;
		}
	}
	returnAST = identifier_AST;
}

void PascalParser::exportsClause() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST exportsClause_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp15_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp15_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp15_AST));
		}
		match(EXPORTS);
		exportsList();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		exportsClause_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_5);
		} else {
			throw;
		}
	}
	returnAST = exportsClause_AST;
}

void PascalParser::exportsList() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST exportsList_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		exportsEntry();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				exportsEntry();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
			}
			else {
				goto _loop9;
			}
			
		}
		_loop9:;
		} // ( ... )*
		exportsList_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_5);
		} else {
			throw;
		}
	}
	returnAST = exportsList_AST;
}

void PascalParser::exportsEntry() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST exportsEntry_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{
		switch ( LA(1)) {
		case LITERAL_index:
		{
			RefPascalAST tmp17_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp17_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp17_AST));
			}
			match(LITERAL_index);
			integerConstant();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			break;
		}
		case DOT:
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
			RefPascalAST tmp18_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp18_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp18_AST));
			}
			match(LITERAL_name);
			stringConstant();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			break;
		}
		case DOT:
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
		exportsEntry_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_6);
		} else {
			throw;
		}
	}
	returnAST = exportsEntry_AST;
}

void PascalParser::integerConstant() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST integerConstant_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefPascalAST s_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefPascalAST n_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case NUM_INT:
		{
			unsignedInteger();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			integerConstant_AST = static_cast<RefPascalAST>(currentAST.root);
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
#line 839 "pascal.g"
				integerConstant_AST=RefPascalAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(s_AST))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(n_AST))));
#line 623 "PascalParser.cpp"
				currentAST.root = integerConstant_AST;
				if ( integerConstant_AST!=static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
					integerConstant_AST->getFirstChild() != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
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
			consume();
			consumeUntil(_tokenSet_7);
		} else {
			throw;
		}
	}
	returnAST = integerConstant_AST;
}

void PascalParser::stringConstant() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST stringConstant_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case STRING_LITERAL:
		{
			string();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			stringConstant_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case CHR:
		{
			constantChr();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			stringConstant_AST = static_cast<RefPascalAST>(currentAST.root);
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
			consume();
			consumeUntil(_tokenSet_8);
		} else {
			throw;
		}
	}
	returnAST = stringConstant_AST;
}

void PascalParser::identifierList() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST identifierList_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				identifier();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
			}
			else {
				goto _loop345;
			}
			
		}
		_loop345:;
		} // ( ... )*
		if ( inputState->guessing==0 ) {
			identifierList_AST = RefPascalAST(currentAST.root);
#line 826 "pascal.g"
			identifierList_AST = RefPascalAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(IDLIST)))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(identifierList_AST))));
#line 725 "PascalParser.cpp"
			currentAST.root = identifierList_AST;
			if ( identifierList_AST!=static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				identifierList_AST->getFirstChild() != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = identifierList_AST->getFirstChild();
			else
				currentAST.child = identifierList_AST;
			currentAST.advanceChildToEnd();
		}
		identifierList_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_9);
		} else {
			throw;
		}
	}
	returnAST = identifierList_AST;
}

void PascalParser::interfacePart() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST interfacePart_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp20_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp20_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp20_AST));
		}
		match(INTERFACE);
		{
		switch ( LA(1)) {
		case USES:
		{
			usesClause();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
				break;
			}
			case TYPE:
			{
				typeDeclarationPart();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
				break;
			}
			case PROCEDURE:
			case FUNCTION:
			{
				procedureHeadersPart();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
				break;
			}
			default:
			{
				goto _loop21;
			}
			}
		}
		_loop21:;
		} // ( ... )*
		interfacePart_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_10);
		} else {
			throw;
		}
	}
	returnAST = interfacePart_AST;
}

void PascalParser::implementationPart() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST implementationPart_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp21_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp21_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp21_AST));
		}
		match(IMPLEMENTATION);
		{
		switch ( LA(1)) {
		case USES:
		{
			usesClause();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		implementationPart_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_11);
		} else {
			throw;
		}
	}
	returnAST = implementationPart_AST;
}

void PascalParser::initializationPart() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST initializationPart_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp22_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp22_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp22_AST));
		}
		match(INITIALIZATION);
		statement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == SEMI)) {
				match(SEMI);
				statement();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
			}
			else {
				goto _loop31;
			}
			
		}
		_loop31:;
		} // ( ... )*
		initializationPart_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_12);
		} else {
			throw;
		}
	}
	returnAST = initializationPart_AST;
}

void PascalParser::finalizationPart() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST finalizationPart_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp24_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp24_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp24_AST));
		}
		match(FINALIZATION);
		statement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == SEMI)) {
				match(SEMI);
				statement();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
			}
			else {
				goto _loop34;
			}
			
		}
		_loop34:;
		} // ( ... )*
		finalizationPart_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_13);
		} else {
			throw;
		}
	}
	returnAST = finalizationPart_AST;
}

void PascalParser::realizationPart() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST realizationPart_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp26_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp26_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp26_AST));
		}
		match(BEGIN);
		statement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == SEMI)) {
				match(SEMI);
				statement();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
			}
			else {
				goto _loop26;
			}
			
		}
		_loop26:;
		} // ( ... )*
		realizationPart_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_13);
		} else {
			throw;
		}
	}
	returnAST = realizationPart_AST;
}

void PascalParser::constantDeclarationPart() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST constantDeclarationPart_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp28_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp28_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp28_AST));
		}
		match(CONST);
		{ // ( ... )+
		int _cnt44=0;
		for (;;) {
			if ((LA(1) == IDENT) && (LA(2) == EQUAL)) {
				constantDeclaration();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
			}
			else if ((LA(1) == IDENT) && (LA(2) == COLON)) {
				typedConstantDeclaration();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
			}
			else {
				if ( _cnt44>=1 ) { goto _loop44; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
			}
			
			_cnt44++;
		}
		_loop44:;
		}  // ( ... )+
		constantDeclarationPart_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_2);
		} else {
			throw;
		}
	}
	returnAST = constantDeclarationPart_AST;
}

void PascalParser::typeDeclarationPart() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST typeDeclarationPart_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp29_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp29_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp29_AST));
		}
		match(TYPE);
		{ // ( ... )+
		int _cnt51=0;
		for (;;) {
			if ((LA(1) == IDENT)) {
				typeDeclaration();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
			}
			else {
				if ( _cnt51>=1 ) { goto _loop51; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
			}
			
			_cnt51++;
		}
		_loop51:;
		}  // ( ... )+
		typeDeclarationPart_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_2);
		} else {
			throw;
		}
	}
	returnAST = typeDeclarationPart_AST;
}

void PascalParser::procedureHeadersPart() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST procedureHeadersPart_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case PROCEDURE:
		{
			procedureHeader();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			break;
		}
		case FUNCTION:
		{
			functionHeader();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
		{
		callModifiers();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(SEMI);
		}
		procedureHeadersPart_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_14);
		} else {
			throw;
		}
	}
	returnAST = procedureHeadersPart_AST;
}

void PascalParser::declarationPart() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST declarationPart_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{ // ( ... )*
		for (;;) {
			switch ( LA(1)) {
			case LABEL:
			{
				labelDeclarationPart();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
				break;
			}
			case CONST:
			{
				constantDeclarationPart();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
				break;
			}
			case RESOURCESTRING:
			{
				resourcestringDeclarationPart();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
				break;
			}
			case TYPE:
			{
				typeDeclarationPart();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
				break;
			}
			case VAR:
			{
				variableDeclarationPart();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
				break;
			}
			default:
			{
				goto _loop38;
			}
			}
		}
		_loop38:;
		} // ( ... )*
		declarationPart_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_11);
		} else {
			throw;
		}
	}
	returnAST = declarationPart_AST;
}

void PascalParser::statement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST statement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case NUM_INT:
		{
			label();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		statement_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_15);
		} else {
			throw;
		}
	}
	returnAST = statement_AST;
}

void PascalParser::statementPart() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST statementPart_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		compoundStatement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		statementPart_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_3);
		} else {
			throw;
		}
	}
	returnAST = statementPart_AST;
}

void PascalParser::labelDeclarationPart() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST labelDeclarationPart_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp33_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp33_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp33_AST));
		}
		match(LABEL);
		label();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				label();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
			}
			else {
				goto _loop41;
			}
			
		}
		_loop41:;
		} // ( ... )*
		match(SEMI);
		labelDeclarationPart_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_16);
		} else {
			throw;
		}
	}
	returnAST = labelDeclarationPart_AST;
}

void PascalParser::resourcestringDeclarationPart() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST resourcestringDeclarationPart_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp36_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp36_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp36_AST));
		}
		match(RESOURCESTRING);
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == IDENT)) {
				stringConstantDeclaration();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
			}
			else {
				goto _loop47;
			}
			
		}
		_loop47:;
		} // ( ... )*
		resourcestringDeclarationPart_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_16);
		} else {
			throw;
		}
	}
	returnAST = resourcestringDeclarationPart_AST;
}

void PascalParser::variableDeclarationPart() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST variableDeclarationPart_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp37_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp37_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp37_AST));
		}
		match(VAR);
		variableDeclaration();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == SEMI) && (LA(2) == IDENT)) {
				match(SEMI);
				variableDeclaration();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
			}
			else {
				goto _loop54;
			}
			
		}
		_loop54:;
		} // ( ... )*
		match(SEMI);
		variableDeclarationPart_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_16);
		} else {
			throw;
		}
	}
	returnAST = variableDeclarationPart_AST;
}

void PascalParser::procedureAndFunctionDeclarationPart() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST procedureAndFunctionDeclarationPart_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		procedureAndFunctionDeclaration();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		procedureAndFunctionDeclarationPart_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_16);
		} else {
			throw;
		}
	}
	returnAST = procedureAndFunctionDeclarationPart_AST;
}

void PascalParser::label() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST label_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		unsignedInteger();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		label_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_17);
		} else {
			throw;
		}
	}
	returnAST = label_AST;
}

void PascalParser::constantDeclaration() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST constantDeclaration_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(EQUAL);
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(SEMI);
		constantDeclaration_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_18);
		} else {
			throw;
		}
	}
	returnAST = constantDeclaration_AST;
}

void PascalParser::typedConstantDeclaration() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST typedConstantDeclaration_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(COLON);
		type();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(EQUAL);
		typedConstant();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(SEMI);
		typedConstantDeclaration_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_18);
		} else {
			throw;
		}
	}
	returnAST = typedConstantDeclaration_AST;
}

void PascalParser::stringConstantDeclaration() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST stringConstantDeclaration_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(EQUAL);
		string();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		stringConstantDeclaration_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_19);
		} else {
			throw;
		}
	}
	returnAST = stringConstantDeclaration_AST;
}

void PascalParser::string() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST string_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp46_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp46_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp46_AST));
		}
		match(STRING_LITERAL);
		string_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_20);
		} else {
			throw;
		}
	}
	returnAST = string_AST;
}

void PascalParser::typeDeclaration() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST typeDeclaration_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(EQUAL);
		type();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(SEMI);
		typeDeclaration_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_18);
		} else {
			throw;
		}
	}
	returnAST = typeDeclaration_AST;
}

void PascalParser::variableDeclaration() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST variableDeclaration_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  c = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefPascalAST c_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifierList();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		c = LT(1);
		if ( inputState->guessing == 0 ) {
			c_AST = astFactory->create(c);
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(c_AST));
		}
		match(COLON);
		if ( inputState->guessing==0 ) {
#line 241 "pascal.g"
			c_AST->setType(VARDECL);
#line 1721 "PascalParser.cpp"
		}
		type();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		variableDeclaration_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_21);
		} else {
			throw;
		}
	}
	returnAST = variableDeclaration_AST;
}

void PascalParser::type() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST type_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			type_AST = static_cast<RefPascalAST>(currentAST.root);
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			type_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case LPAREN:
		{
			enumeratedType();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			type_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case STRING:
		{
			stringType();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			type_AST = static_cast<RefPascalAST>(currentAST.root);
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			type_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case POINTER:
		{
			pointerType();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			type_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case PROCEDURE:
		case FUNCTION:
		{
			proceduralType();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			type_AST = static_cast<RefPascalAST>(currentAST.root);
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
			consume();
			consumeUntil(_tokenSet_22);
		} else {
			throw;
		}
	}
	returnAST = type_AST;
}

void PascalParser::procedureAndFunctionDeclaration() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST procedureAndFunctionDeclaration_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case PROCEDURE:
		{
			procedureDeclaration();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			procedureAndFunctionDeclaration_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case FUNCTION:
		{
			functionDeclaration();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			procedureAndFunctionDeclaration_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case CONSTRUCTOR:
		{
			constructorDeclaration();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			procedureAndFunctionDeclaration_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case DESTRUCTOR:
		{
			destructorDeclaration();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			procedureAndFunctionDeclaration_AST = static_cast<RefPascalAST>(currentAST.root);
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
			consume();
			consumeUntil(_tokenSet_16);
		} else {
			throw;
		}
	}
	returnAST = procedureAndFunctionDeclaration_AST;
}

void PascalParser::procedureDeclaration() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST procedureDeclaration_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		procedureHeader();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(SEMI);
		subroutineBlock();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(SEMI);
		procedureDeclaration_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_16);
		} else {
			throw;
		}
	}
	returnAST = procedureDeclaration_AST;
}

void PascalParser::functionDeclaration() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST functionDeclaration_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		functionHeader();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(SEMI);
		subroutineBlock();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(SEMI);
		functionDeclaration_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_16);
		} else {
			throw;
		}
	}
	returnAST = functionDeclaration_AST;
}

void PascalParser::constructorDeclaration() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST constructorDeclaration_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		constructorHeader();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(SEMI);
		subroutineBlock();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		constructorDeclaration_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_16);
		} else {
			throw;
		}
	}
	returnAST = constructorDeclaration_AST;
}

void PascalParser::destructorDeclaration() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST destructorDeclaration_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		destructorHeader();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(SEMI);
		subroutineBlock();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		destructorDeclaration_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_16);
		} else {
			throw;
		}
	}
	returnAST = destructorDeclaration_AST;
}

void PascalParser::compoundStatement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST compoundStatement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(BEGIN);
		statement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == SEMI)) {
				match(SEMI);
				statement();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
			}
			else {
				goto _loop282;
			}
			
		}
		_loop282:;
		} // ( ... )*
		match(END);
		compoundStatement_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_23);
		} else {
			throw;
		}
	}
	returnAST = compoundStatement_AST;
}

void PascalParser::procedureHeader() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST procedureHeader_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp58_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp58_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp58_AST));
		}
		match(PROCEDURE);
		{
		if ((LA(1) == IDENT) && (LA(2) == LPAREN)) {
			identifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
		}
		else if ((LA(1) == IDENT) && (LA(2) == COLON)) {
			qualifiedMethodIdentifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
		formalParameterList();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(SEMI);
		{ // ( ... )*
		for (;;) {
			if (((LA(1) >= PUBLIC && LA(1) <= FAR))) {
				modifiers();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
				match(SEMI);
			}
			else {
				goto _loop66;
			}
			
		}
		_loop66:;
		} // ( ... )*
		procedureHeader_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_24);
		} else {
			throw;
		}
	}
	returnAST = procedureHeader_AST;
}

void PascalParser::subroutineBlock() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST subroutineBlock_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			subroutineBlock_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case EXTERNAL:
		{
			externalDirective();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			subroutineBlock_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case FORWARD:
		{
			match(FORWARD);
			subroutineBlock_AST = static_cast<RefPascalAST>(currentAST.root);
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
			consume();
			consumeUntil(_tokenSet_25);
		} else {
			throw;
		}
	}
	returnAST = subroutineBlock_AST;
}

void PascalParser::functionHeader() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST functionHeader_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp62_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp62_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp62_AST));
		}
		match(FUNCTION);
		{
		if ((LA(1) == IDENT) && (LA(2) == LPAREN)) {
			identifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
		}
		else if ((LA(1) == IDENT) && (LA(2) == COLON)) {
			qualifiedMethodIdentifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
		formalParameterList();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(COLON);
		type();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(SEMI);
		{ // ( ... )*
		for (;;) {
			if (((LA(1) >= PUBLIC && LA(1) <= FAR))) {
				modifiers();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
				match(SEMI);
			}
			else {
				goto _loop73;
			}
			
		}
		_loop73:;
		} // ( ... )*
		functionHeader_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_24);
		} else {
			throw;
		}
	}
	returnAST = functionHeader_AST;
}

void PascalParser::callModifiers() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST callModifiers_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case REGISTER:
		{
			match(REGISTER);
			callModifiers_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case PASCAL:
		{
			match(PASCAL);
			callModifiers_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case CDECL:
		{
			match(CDECL);
			callModifiers_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case STDCALL:
		{
			match(STDCALL);
			callModifiers_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case POPSTACK:
		{
			match(POPSTACK);
			callModifiers_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case SAVEREGISTERS:
		{
			match(SAVEREGISTERS);
			callModifiers_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case INLINE:
		{
			match(INLINE);
			callModifiers_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case SAFECALL:
		{
			match(SAFECALL);
			callModifiers_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case NEAR:
		{
			match(NEAR);
			callModifiers_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case FAR:
		{
			match(FAR);
			callModifiers_AST = static_cast<RefPascalAST>(currentAST.root);
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
			consume();
			consumeUntil(_tokenSet_22);
		} else {
			throw;
		}
	}
	returnAST = callModifiers_AST;
}

void PascalParser::qualifiedMethodIdentifier() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST qualifiedMethodIdentifier_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(COLON);
		match(COLON);
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		qualifiedMethodIdentifier_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_26);
		} else {
			throw;
		}
	}
	returnAST = qualifiedMethodIdentifier_AST;
}

void PascalParser::formalParameterList() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST formalParameterList_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(LPAREN);
		parameterDeclaration();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{
		match(SEMI);
		parameterDeclaration();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		}
		match(RPAREN);
		formalParameterList_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_27);
		} else {
			throw;
		}
	}
	returnAST = formalParameterList_AST;
}

void PascalParser::modifiers() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST modifiers_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case PUBLIC:
		{
			match(PUBLIC);
			modifiers_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case ALIAS:
		{
			{
			match(ALIAS);
			stringConstant();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			}
			modifiers_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case INTERRUPT:
		{
			match(INTERRUPT);
			modifiers_AST = static_cast<RefPascalAST>(currentAST.root);
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			modifiers_AST = static_cast<RefPascalAST>(currentAST.root);
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
			consume();
			consumeUntil(_tokenSet_21);
		} else {
			throw;
		}
	}
	returnAST = modifiers_AST;
}

void PascalParser::externalDirective() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST externalDirective_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp84_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp84_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp84_AST));
		}
		match(EXTERNAL);
		{
		switch ( LA(1)) {
		case STRING_LITERAL:
		case CHR:
		{
			stringConstant();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			{
			switch ( LA(1)) {
			case LITERAL_name:
			{
				{
				RefPascalAST tmp85_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp85_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp85_AST));
				}
				match(LITERAL_name);
				stringConstant();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
				}
				break;
			}
			case LITERAL_index:
			{
				{
				RefPascalAST tmp86_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp86_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp86_AST));
				}
				match(LITERAL_index);
				integerConstant();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
				}
				break;
			}
			case ANTLR_USE_NAMESPACE(antlr)Token::EOF_TYPE:
			case SEMI:
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
			break;
		}
		case ANTLR_USE_NAMESPACE(antlr)Token::EOF_TYPE:
		case SEMI:
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
		externalDirective_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_25);
		} else {
			throw;
		}
	}
	returnAST = externalDirective_AST;
}

void PascalParser::parameterDeclaration() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST parameterDeclaration_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case IDENT:
		{
			valueParameter();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			parameterDeclaration_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case VAR:
		{
			variableParameter();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			parameterDeclaration_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case CONST:
		{
			constantParameter();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			parameterDeclaration_AST = static_cast<RefPascalAST>(currentAST.root);
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
			consume();
			consumeUntil(_tokenSet_28);
		} else {
			throw;
		}
	}
	returnAST = parameterDeclaration_AST;
}

void PascalParser::valueParameter() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST valueParameter_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		bool synPredMatched79 = false;
		if (((LA(1) == IDENT) && (LA(2) == COMMA || LA(2) == COLON))) {
			int _m79 = mark();
			synPredMatched79 = true;
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
				synPredMatched79 = false;
			}
			rewind(_m79);
			inputState->guessing--;
		}
		if ( synPredMatched79 ) {
			identifierList();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			match(COLON);
			match(ARRAY);
			match(OF);
			type();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			valueParameter_AST = static_cast<RefPascalAST>(currentAST.root);
		}
		else if ((LA(1) == IDENT) && (LA(2) == COMMA || LA(2) == COLON)) {
			identifierList();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			match(COLON);
			type();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			valueParameter_AST = static_cast<RefPascalAST>(currentAST.root);
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_28);
		} else {
			throw;
		}
	}
	returnAST = valueParameter_AST;
}

void PascalParser::variableParameter() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST variableParameter_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp91_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp91_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp91_AST));
		}
		match(VAR);
		identifierList();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{
		switch ( LA(1)) {
		case COLON:
		{
			untypedParameterPart();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
		variableParameter_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_28);
		} else {
			throw;
		}
	}
	returnAST = variableParameter_AST;
}

void PascalParser::constantParameter() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST constantParameter_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp92_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp92_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp92_AST));
		}
		match(CONST);
		identifierList();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{
		switch ( LA(1)) {
		case COLON:
		{
			untypedParameterPart();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
		constantParameter_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_28);
		} else {
			throw;
		}
	}
	returnAST = constantParameter_AST;
}

void PascalParser::untypedParameterPart() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST untypedParameterPart_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		bool synPredMatched84 = false;
		if (((LA(1) == COLON) && (LA(2) == ARRAY))) {
			int _m84 = mark();
			synPredMatched84 = true;
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
				synPredMatched84 = false;
			}
			rewind(_m84);
			inputState->guessing--;
		}
		if ( synPredMatched84 ) {
			match(COLON);
			match(ARRAY);
			match(OF);
			type();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			untypedParameterPart_AST = static_cast<RefPascalAST>(currentAST.root);
		}
		else if ((LA(1) == COLON) && (_tokenSet_29.member(LA(2)))) {
			match(COLON);
			type();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			untypedParameterPart_AST = static_cast<RefPascalAST>(currentAST.root);
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_28);
		} else {
			throw;
		}
	}
	returnAST = untypedParameterPart_AST;
}

void PascalParser::expression() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST expression_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		simpleExpression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{
		switch ( LA(1)) {
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			simpleExpression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			break;
		}
		case DOT:
		case SEMI:
		case COMMA:
		case END:
		case LPAREN:
		case RPAREN:
		case FINALIZATION:
		case OF:
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
		expression_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_30);
		} else {
			throw;
		}
	}
	returnAST = expression_AST;
}

void PascalParser::typedConstant() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST typedConstant_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		bool synPredMatched99 = false;
		if (((_tokenSet_31.member(LA(1))) && (_tokenSet_32.member(LA(2))))) {
			int _m99 = mark();
			synPredMatched99 = true;
			inputState->guessing++;
			try {
				{
				constant();
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched99 = false;
			}
			rewind(_m99);
			inputState->guessing--;
		}
		if ( synPredMatched99 ) {
			constant();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			typedConstant_AST = static_cast<RefPascalAST>(currentAST.root);
		}
		else {
			bool synPredMatched101 = false;
			if (((LA(1) == LPAREN) && (LA(2) == IDENT))) {
				int _m101 = mark();
				synPredMatched101 = true;
				inputState->guessing++;
				try {
					{
					match(LPAREN);
					identifier();
					match(COLON);
					}
				}
				catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
					synPredMatched101 = false;
				}
				rewind(_m101);
				inputState->guessing--;
			}
			if ( synPredMatched101 ) {
				recordConstant();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
				typedConstant_AST = static_cast<RefPascalAST>(currentAST.root);
			}
			else {
				bool synPredMatched103 = false;
				if (((LA(1) == LPAREN) && (_tokenSet_33.member(LA(2))))) {
					int _m103 = mark();
					synPredMatched103 = true;
					inputState->guessing++;
					try {
						{
						arrayConstant();
						}
					}
					catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
						synPredMatched103 = false;
					}
					rewind(_m103);
					inputState->guessing--;
				}
				if ( synPredMatched103 ) {
					arrayConstant();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
					}
					typedConstant_AST = static_cast<RefPascalAST>(currentAST.root);
				}
				else if ((_tokenSet_34.member(LA(1))) && (_tokenSet_35.member(LA(2)))) {
					proceduralConstant();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
					}
					typedConstant_AST = static_cast<RefPascalAST>(currentAST.root);
				}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_21);
		} else {
			throw;
		}
	}
	returnAST = typedConstant_AST;
}

void PascalParser::constant() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST constant_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefPascalAST s_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefPascalAST n_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefPascalAST s2_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefPascalAST id_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case NUM_INT:
		case NUM_REAL:
		{
			unsignedNumber();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			constant_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case IDENT:
		{
			identifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			constant_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case STRING_LITERAL:
		{
			string();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			constant_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case CHR:
		{
			constantChr();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			constant_AST = static_cast<RefPascalAST>(currentAST.root);
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
#line 860 "pascal.g"
					constant_AST=RefPascalAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(s_AST))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(n_AST))));
#line 3125 "PascalParser.cpp"
					currentAST.root = constant_AST;
					if ( constant_AST!=static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
						constant_AST->getFirstChild() != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
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
#line 862 "pascal.g"
					constant_AST=RefPascalAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(s2_AST))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(id_AST))));
#line 3148 "PascalParser.cpp"
					currentAST.root = constant_AST;
					if ( constant_AST!=static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
						constant_AST->getFirstChild() != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
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
			consume();
			consumeUntil(_tokenSet_36);
		} else {
			throw;
		}
	}
	returnAST = constant_AST;
}

void PascalParser::recordConstant() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST recordConstant_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(LPAREN);
		{
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(COLON);
		constant();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == SEMI)) {
				match(SEMI);
				{
				identifier();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
				match(COLON);
				constant();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
				}
			}
			else {
				goto _loop113;
			}
			
		}
		_loop113:;
		} // ( ... )*
		match(RPAREN);
		recordConstant_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_21);
		} else {
			throw;
		}
	}
	returnAST = recordConstant_AST;
}

void PascalParser::arrayConstant() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST arrayConstant_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			break;
		}
		case LPAREN:
		{
			arrayConstant();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
						astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
					}
					break;
				}
				case LPAREN:
				{
					arrayConstant();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
				goto _loop108;
			}
			
		}
		_loop108:;
		} // ( ... )*
		match(RPAREN);
		arrayConstant_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_37);
		} else {
			throw;
		}
	}
	returnAST = arrayConstant_AST;
}

void PascalParser::proceduralConstant() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST proceduralConstant_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		proceduralConstant_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_21);
		} else {
			throw;
		}
	}
	returnAST = proceduralConstant_AST;
}

void PascalParser::addressConstant() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST addressConstant_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp105_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp105_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp105_AST));
		}
		match(NUM_INT);
		addressConstant_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_15);
		} else {
			throw;
		}
	}
	returnAST = addressConstant_AST;
}

void PascalParser::simpleType() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST simpleType_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			simpleType_AST = static_cast<RefPascalAST>(currentAST.root);
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			simpleType_AST = static_cast<RefPascalAST>(currentAST.root);
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
			consume();
			consumeUntil(_tokenSet_22);
		} else {
			throw;
		}
	}
	returnAST = simpleType_AST;
}

void PascalParser::subrangeTypeOrTypeIdentifier() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST subrangeTypeOrTypeIdentifier_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		constant();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{
		switch ( LA(1)) {
		case DOTDOT:
		{
			match(DOTDOT);
			constant();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
		subrangeTypeOrTypeIdentifier_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_22);
		} else {
			throw;
		}
	}
	returnAST = subrangeTypeOrTypeIdentifier_AST;
}

void PascalParser::enumeratedType() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST enumeratedType_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		bool synPredMatched126 = false;
		if (((LA(1) == LPAREN) && (LA(2) == IDENT))) {
			int _m126 = mark();
			synPredMatched126 = true;
			inputState->guessing++;
			try {
				{
				match(LPAREN);
				identifier();
				match(ASSIGN);
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched126 = false;
			}
			rewind(_m126);
			inputState->guessing--;
		}
		if ( synPredMatched126 ) {
			match(LPAREN);
			assignedEnumList();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			match(RPAREN);
			enumeratedType_AST = static_cast<RefPascalAST>(currentAST.root);
		}
		else if ((LA(1) == LPAREN) && (LA(2) == IDENT)) {
			match(LPAREN);
			identifierList();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			match(RPAREN);
			enumeratedType_AST = static_cast<RefPascalAST>(currentAST.root);
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_22);
		} else {
			throw;
		}
	}
	returnAST = enumeratedType_AST;
}

void PascalParser::stringType() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST stringType_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp111_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp111_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp111_AST));
		}
		match(STRING);
		{
		switch ( LA(1)) {
		case LBRACK:
		{
			match(LBRACK);
			unsignedInteger();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
		stringType_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_22);
		} else {
			throw;
		}
	}
	returnAST = stringType_AST;
}

void PascalParser::structuredType() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST structuredType_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			break;
		}
		case RECORD:
		{
			recordType();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			break;
		}
		case OBJECT:
		{
			objectType();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			break;
		}
		case CLASS:
		{
			classType();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			break;
		}
		case SET:
		{
			setType();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			break;
		}
		case FILE:
		{
			fileType();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		structuredType_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_22);
		} else {
			throw;
		}
	}
	returnAST = structuredType_AST;
}

void PascalParser::pointerType() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST pointerType_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp115_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp115_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp115_AST));
		}
		match(POINTER);
		typeIdentifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		pointerType_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_22);
		} else {
			throw;
		}
	}
	returnAST = pointerType_AST;
}

void PascalParser::proceduralType() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST proceduralType_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		bool synPredMatched160 = false;
		if (((LA(1) == PROCEDURE || LA(1) == FUNCTION) && (LA(2) == IDENT))) {
			int _m160 = mark();
			synPredMatched160 = true;
			inputState->guessing++;
			try {
				{
				proceduralTypePart1();
				match(SEMI);
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched160 = false;
			}
			rewind(_m160);
			inputState->guessing--;
		}
		if ( synPredMatched160 ) {
			proceduralTypePart1();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			match(SEMI);
			callModifiers();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			proceduralType_AST = static_cast<RefPascalAST>(currentAST.root);
		}
		else if ((LA(1) == PROCEDURE || LA(1) == FUNCTION) && (LA(2) == IDENT)) {
			proceduralTypePart1();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			proceduralType_AST = static_cast<RefPascalAST>(currentAST.root);
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_22);
		} else {
			throw;
		}
	}
	returnAST = proceduralType_AST;
}

void PascalParser::ordinalType() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST ordinalType_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case INTEGER:
		{
			match(INTEGER);
			ordinalType_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case SHORTINT:
		{
			match(SHORTINT);
			ordinalType_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case SMALLINT:
		{
			match(SMALLINT);
			ordinalType_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case LONGINT:
		{
			match(LONGINT);
			ordinalType_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case INT64:
		{
			match(INT64);
			ordinalType_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case BYTE:
		{
			match(BYTE);
			ordinalType_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case WORD:
		{
			match(WORD);
			ordinalType_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case CARDINAL:
		{
			match(CARDINAL);
			ordinalType_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case QWORD:
		{
			match(QWORD);
			ordinalType_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case BOOLEAN:
		{
			match(BOOLEAN);
			ordinalType_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case BYTEBOOL:
		{
			match(BYTEBOOL);
			ordinalType_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case LONGBOOL:
		{
			match(LONGBOOL);
			ordinalType_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case CHAR:
		{
			match(CHAR);
			ordinalType_AST = static_cast<RefPascalAST>(currentAST.root);
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
			consume();
			consumeUntil(_tokenSet_38);
		} else {
			throw;
		}
	}
	returnAST = ordinalType_AST;
}

void PascalParser::realType() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST realType_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case REAL:
		{
			match(REAL);
			realType_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case SINGLE:
		{
			match(SINGLE);
			realType_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case DOUBLE:
		{
			match(DOUBLE);
			realType_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case EXTENDED:
		{
			match(EXTENDED);
			realType_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case COMP:
		{
			match(COMP);
			realType_AST = static_cast<RefPascalAST>(currentAST.root);
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
			consume();
			consumeUntil(_tokenSet_22);
		} else {
			throw;
		}
	}
	returnAST = realType_AST;
}

void PascalParser::typeIdentifier() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST typeIdentifier_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		typeIdentifier_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_39);
		} else {
			throw;
		}
	}
	returnAST = typeIdentifier_AST;
}

void PascalParser::subrangeType() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST subrangeType_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		constant();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(DOTDOT);
		constant();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		subrangeType_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_0);
		} else {
			throw;
		}
	}
	returnAST = subrangeType_AST;
}

void PascalParser::assignedEnumList() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST assignedEnumList_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(ASSIGN);
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				{
				identifier();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
				match(ASSIGN);
				expression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
				}
			}
			else {
				goto _loop131;
			}
			
		}
		_loop131:;
		} // ( ... )*
		assignedEnumList_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_40);
		} else {
			throw;
		}
	}
	returnAST = assignedEnumList_AST;
}

void PascalParser::unsignedInteger() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST unsignedInteger_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp139_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp139_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp139_AST));
		}
		match(NUM_INT);
		unsignedInteger_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_41);
		} else {
			throw;
		}
	}
	returnAST = unsignedInteger_AST;
}

void PascalParser::arrayType() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST arrayType_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp140_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp140_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp140_AST));
		}
		match(ARRAY);
		match(LBRACK);
		ordinalType();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				ordinalType();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
			}
			else {
				goto _loop140;
			}
			
		}
		_loop140:;
		} // ( ... )*
		match(RBRACK);
		match(OF);
		type();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		arrayType_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_22);
		} else {
			throw;
		}
	}
	returnAST = arrayType_AST;
}

void PascalParser::recordType() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST recordType_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp145_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp145_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp145_AST));
		}
		match(RECORD);
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == CASE || LA(1) == IDENT)) {
				fieldList();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
			}
			else {
				goto _loop143;
			}
			
		}
		_loop143:;
		} // ( ... )*
		match(END);
		recordType_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_22);
		} else {
			throw;
		}
	}
	returnAST = recordType_AST;
}

void PascalParser::objectType() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST objectType_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp147_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp147_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp147_AST));
		}
		match(OBJECT);
		{
		switch ( LA(1)) {
		case LPAREN:
		{
			heritage();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			break;
		}
		case PUBLIC:
		case PRIVATE:
		case PROTECTED:
		{
			objectVisibilitySpecifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
		objectType_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_22);
		} else {
			throw;
		}
	}
	returnAST = objectType_AST;
}

void PascalParser::classType() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST classType_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp149_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp149_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp149_AST));
		}
		match(CLASS);
		{
		switch ( LA(1)) {
		case LPAREN:
		{
			heritage();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
		classType_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_22);
		} else {
			throw;
		}
	}
	returnAST = classType_AST;
}

void PascalParser::setType() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST setType_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp151_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp151_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp151_AST));
		}
		match(SET);
		match(OF);
		ordinalType();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		setType_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_22);
		} else {
			throw;
		}
	}
	returnAST = setType_AST;
}

void PascalParser::fileType() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST fileType_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp153_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp153_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp153_AST));
		}
		match(FILE);
		match(OF);
		type();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		fileType_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_22);
		} else {
			throw;
		}
	}
	returnAST = fileType_AST;
}

void PascalParser::fieldList() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST fieldList_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case IDENT:
		{
			fixedField();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			fieldList_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case CASE:
		{
			variantPart();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			fieldList_AST = static_cast<RefPascalAST>(currentAST.root);
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
			consume();
			consumeUntil(_tokenSet_42);
		} else {
			throw;
		}
	}
	returnAST = fieldList_AST;
}

void PascalParser::fixedField() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST fixedField_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifierList();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(COLON);
		type();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(SEMI);
		fixedField_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_42);
		} else {
			throw;
		}
	}
	returnAST = fixedField_AST;
}

void PascalParser::variantPart() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST variantPart_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp157_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp157_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp157_AST));
		}
		match(CASE);
		{
		if ((LA(1) == IDENT) && (LA(2) == COLON)) {
			identifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(OF);
		variant();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == SEMI)) {
				match(SEMI);
				variant();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
			}
			else {
				goto _loop149;
			}
			
		}
		_loop149:;
		} // ( ... )*
		variantPart_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_42);
		} else {
			throw;
		}
	}
	returnAST = variantPart_AST;
}

void PascalParser::variant() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST variant_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{ // ( ... )+
		int _cnt152=0;
		for (;;) {
			if ((_tokenSet_31.member(LA(1)))) {
				constant();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
				match(COMMA);
			}
			else {
				if ( _cnt152>=1 ) { goto _loop152; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
			}
			
			_cnt152++;
		}
		_loop152:;
		}  // ( ... )+
		match(COLON);
		match(LPAREN);
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == CASE || LA(1) == IDENT)) {
				fieldList();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
			}
			else {
				goto _loop154;
			}
			
		}
		_loop154:;
		} // ( ... )*
		match(RPAREN);
		variant_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_43);
		} else {
			throw;
		}
	}
	returnAST = variant_AST;
}

void PascalParser::proceduralTypePart1() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST proceduralTypePart1_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case FUNCTION:
		{
			functionHeader();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			break;
		}
		case PROCEDURE:
		{
			procedureHeader();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
		proceduralTypePart1_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_22);
		} else {
			throw;
		}
	}
	returnAST = proceduralTypePart1_AST;
}

void PascalParser::heritage() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST heritage_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(LPAREN);
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(RPAREN);
		heritage_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_44);
		} else {
			throw;
		}
	}
	returnAST = heritage_AST;
}

void PascalParser::componentList() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST componentList_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case IDENT:
		{
			{ // ( ... )+
			int _cnt171=0;
			for (;;) {
				if ((LA(1) == IDENT)) {
					fieldDefinition();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
					}
				}
				else {
					if ( _cnt171>=1 ) { goto _loop171; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
				}
				
				_cnt171++;
			}
			_loop171:;
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
			int _cnt174=0;
			for (;;) {
				if ((_tokenSet_45.member(LA(1)))) {
					methodDefinition();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
					}
				}
				else {
					if ( _cnt174>=1 ) { goto _loop174; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
				}
				
				_cnt174++;
			}
			_loop174:;
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
		componentList_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_13);
		} else {
			throw;
		}
	}
	returnAST = componentList_AST;
}

void PascalParser::objectVisibilitySpecifier() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST objectVisibilitySpecifier_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case PRIVATE:
		{
			match(PRIVATE);
			objectVisibilitySpecifier_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case PROTECTED:
		{
			match(PROTECTED);
			objectVisibilitySpecifier_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case PUBLIC:
		{
			match(PUBLIC);
			objectVisibilitySpecifier_AST = static_cast<RefPascalAST>(currentAST.root);
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
			consume();
			consumeUntil(_tokenSet_13);
		} else {
			throw;
		}
	}
	returnAST = objectVisibilitySpecifier_AST;
}

void PascalParser::fieldDefinition() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST fieldDefinition_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifierList();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(COLON);
		type();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(SEMI);
		fieldDefinition_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_46);
		} else {
			throw;
		}
	}
	returnAST = fieldDefinition_AST;
}

void PascalParser::methodDefinition() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST methodDefinition_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case FUNCTION:
		{
			functionHeader();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			break;
		}
		case PROCEDURE:
		{
			procedureHeader();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			break;
		}
		case CONSTRUCTOR:
		{
			constructorHeader();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			break;
		}
		case DESTRUCTOR:
		{
			destructorHeader();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		methodDefinition_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_47);
		} else {
			throw;
		}
	}
	returnAST = methodDefinition_AST;
}

void PascalParser::constructorHeader() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST constructorHeader_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp175_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp175_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp175_AST));
		}
		match(CONSTRUCTOR);
		{
		if ((LA(1) == IDENT) && (LA(2) == LPAREN)) {
			identifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
		}
		else if ((LA(1) == IDENT) && (LA(2) == COLON)) {
			qualifiedMethodIdentifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
		formalParameterList();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		constructorHeader_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_21);
		} else {
			throw;
		}
	}
	returnAST = constructorHeader_AST;
}

void PascalParser::destructorHeader() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST destructorHeader_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp176_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp176_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp176_AST));
		}
		match(DESTRUCTOR);
		{
		if ((LA(1) == IDENT) && (LA(2) == LPAREN)) {
			identifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
		}
		else if ((LA(1) == IDENT) && (LA(2) == COLON)) {
			qualifiedMethodIdentifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
		formalParameterList();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		destructorHeader_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_21);
		} else {
			throw;
		}
	}
	returnAST = destructorHeader_AST;
}

void PascalParser::methodDirectives() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST methodDirectives_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
		methodDirectives_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_47);
		} else {
			throw;
		}
	}
	returnAST = methodDirectives_AST;
}

void PascalParser::classComponentList() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST classComponentList_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case IDENT:
		{
			{ // ( ... )+
			int _cnt195=0;
			for (;;) {
				if ((LA(1) == IDENT)) {
					fieldDefinition();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
					}
				}
				else {
					if ( _cnt195>=1 ) { goto _loop195; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
				}
				
				_cnt195++;
			}
			_loop195:;
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
			int _cnt199=0;
			for (;;) {
				if ((_tokenSet_48.member(LA(1)))) {
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
							astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
						}
						break;
					}
					case PROPERTY:
					{
						propertyDefinition();
						if (inputState->guessing==0) {
							astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
		classComponentList_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_13);
		} else {
			throw;
		}
	}
	returnAST = classComponentList_AST;
}

void PascalParser::classVisibilitySpecifier() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST classVisibilitySpecifier_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case PRIVATE:
		{
			match(PRIVATE);
			classVisibilitySpecifier_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case PROTECTED:
		{
			match(PROTECTED);
			classVisibilitySpecifier_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case PUBLIC:
		{
			match(PUBLIC);
			classVisibilitySpecifier_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case PUBLISHED:
		{
			match(PUBLISHED);
			classVisibilitySpecifier_AST = static_cast<RefPascalAST>(currentAST.root);
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
			consume();
			consumeUntil(_tokenSet_13);
		} else {
			throw;
		}
	}
	returnAST = classVisibilitySpecifier_AST;
}

void PascalParser::classMethodDefinition() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST classMethodDefinition_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
				break;
			}
			case PROCEDURE:
			{
				procedureHeader();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			break;
		}
		case DESTRUCTOR:
		{
			destructorHeader();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		classMethodDefinition_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_49);
		} else {
			throw;
		}
	}
	returnAST = classMethodDefinition_AST;
}

void PascalParser::propertyDefinition() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST propertyDefinition_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp188_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp188_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp188_AST));
		}
		match(PROPERTY);
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{
		switch ( LA(1)) {
		case COLON:
		case LBRACK:
		{
			propertyInterface();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		propertyDefinition_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_49);
		} else {
			throw;
		}
	}
	returnAST = propertyDefinition_AST;
}

void PascalParser::classMethodDirectives() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST classMethodDirectives_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case VIRTUAL:
		case OVERRIDE:
		case MESSAGE:
		{
			directiveVariants();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
		classMethodDirectives_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_49);
		} else {
			throw;
		}
	}
	returnAST = classMethodDirectives_AST;
}

void PascalParser::directiveVariants() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST directiveVariants_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
			directiveVariants_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case OVERRIDE:
		{
			match(OVERRIDE);
			directiveVariants_AST = static_cast<RefPascalAST>(currentAST.root);
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
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
				break;
			}
			case STRING_LITERAL:
			case CHR:
			{
				stringConstant();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
			directiveVariants_AST = static_cast<RefPascalAST>(currentAST.root);
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
			consume();
			consumeUntil(_tokenSet_21);
		} else {
			throw;
		}
	}
	returnAST = directiveVariants_AST;
}

void PascalParser::propertyInterface() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST propertyInterface_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LBRACK:
		{
			propertyParameterList();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{
		switch ( LA(1)) {
		case LITERAL_index:
		{
			RefPascalAST tmp197_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp197_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp197_AST));
			}
			match(LITERAL_index);
			integerConstant();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
		propertyInterface_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_50);
		} else {
			throw;
		}
	}
	returnAST = propertyInterface_AST;
}

void PascalParser::propertySpecifiers() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST propertySpecifiers_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LITERAL_read:
		{
			readSpecifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
		propertySpecifiers_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_49);
		} else {
			throw;
		}
	}
	returnAST = propertySpecifiers_AST;
}

void PascalParser::propertyParameterList() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST propertyParameterList_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(LBRACK);
		parameterDeclaration();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == SEMI)) {
				match(SEMI);
				parameterDeclaration();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
			}
			else {
				goto _loop221;
			}
			
		}
		_loop221:;
		} // ( ... )*
		match(RBRACK);
		propertyParameterList_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_51);
		} else {
			throw;
		}
	}
	returnAST = propertyParameterList_AST;
}

void PascalParser::readSpecifier() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST readSpecifier_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp201_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp201_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp201_AST));
		}
		match(LITERAL_read);
		fieldOrMethod();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		readSpecifier_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_52);
		} else {
			throw;
		}
	}
	returnAST = readSpecifier_AST;
}

void PascalParser::writeSpecifier() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST writeSpecifier_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp202_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp202_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp202_AST));
		}
		match(LITERAL_write);
		fieldOrMethod();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		writeSpecifier_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_53);
		} else {
			throw;
		}
	}
	returnAST = writeSpecifier_AST;
}

void PascalParser::defaultSpecifier() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST defaultSpecifier_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case DEFAULT:
		{
			{
			RefPascalAST tmp203_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp203_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp203_AST));
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
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
			defaultSpecifier_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case LITERAL_nodefault:
		{
			RefPascalAST tmp204_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp204_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp204_AST));
			}
			match(LITERAL_nodefault);
			defaultSpecifier_AST = static_cast<RefPascalAST>(currentAST.root);
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
			consume();
			consumeUntil(_tokenSet_49);
		} else {
			throw;
		}
	}
	returnAST = defaultSpecifier_AST;
}

void PascalParser::fieldOrMethod() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST fieldOrMethod_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		fieldOrMethod_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_52);
		} else {
			throw;
		}
	}
	returnAST = fieldOrMethod_AST;
}

void PascalParser::simpleExpression() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST simpleExpression_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		term();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
			}
			else {
				goto _loop238;
			}
			
		}
		_loop238:;
		} // ( ... )*
		simpleExpression_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_54);
		} else {
			throw;
		}
	}
	returnAST = simpleExpression_AST;
}

void PascalParser::expressionSign() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST expressionSign_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case LE:
		{
			match(LE);
			expressionSign_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case GE:
		{
			match(GE);
			expressionSign_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case LTH:
		{
			match(LTH);
			expressionSign_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case GT:
		{
			match(GT);
			expressionSign_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case NOT_EQUAL:
		{
			match(NOT_EQUAL);
			expressionSign_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case IN:
		{
			match(IN);
			expressionSign_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case IS:
		{
			match(IS);
			expressionSign_AST = static_cast<RefPascalAST>(currentAST.root);
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
			consume();
			consumeUntil(_tokenSet_34);
		} else {
			throw;
		}
	}
	returnAST = expressionSign_AST;
}

void PascalParser::term() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST term_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		factor();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
			}
			else {
				goto _loop242;
			}
			
		}
		_loop242:;
		} // ( ... )*
		term_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_55);
		} else {
			throw;
		}
	}
	returnAST = term_AST;
}

void PascalParser::factor() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST factor_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case LPAREN:
		{
			{
			match(LPAREN);
			expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			match(LPAREN);
			}
			factor_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case IDENT:
		{
			identifierOrValueTypecastOrFunctionCall();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			factor_AST = static_cast<RefPascalAST>(currentAST.root);
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			factor_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case NOT:
		{
			{
			match(NOT);
			factor();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			}
			factor_AST = static_cast<RefPascalAST>(currentAST.root);
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			}
			factor_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case LBRACK:
		{
			setConstructor();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			factor_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case AT:
		{
			addressFactor();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			factor_AST = static_cast<RefPascalAST>(currentAST.root);
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
			consume();
			consumeUntil(_tokenSet_56);
		} else {
			throw;
		}
	}
	returnAST = factor_AST;
}

void PascalParser::identifierOrValueTypecastOrFunctionCall() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST identifierOrValueTypecastOrFunctionCall_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		bool synPredMatched250 = false;
		if (((LA(1) == IDENT) && (LA(2) == LPAREN))) {
			int _m250 = mark();
			synPredMatched250 = true;
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
				synPredMatched250 = false;
			}
			rewind(_m250);
			inputState->guessing--;
		}
		if ( synPredMatched250 ) {
			identifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			match(LPAREN);
			expressions();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			match(RPAREN);
			identifierOrValueTypecastOrFunctionCall_AST = static_cast<RefPascalAST>(currentAST.root);
		}
		else {
			bool synPredMatched252 = false;
			if (((LA(1) == IDENT) && (LA(2) == LPAREN))) {
				int _m252 = mark();
				synPredMatched252 = true;
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
					synPredMatched252 = false;
				}
				rewind(_m252);
				inputState->guessing--;
			}
			if ( synPredMatched252 ) {
				identifier();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
				match(LPAREN);
				expression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
				match(RPAREN);
				identifierOrValueTypecastOrFunctionCall_AST = static_cast<RefPascalAST>(currentAST.root);
			}
			else if ((LA(1) == IDENT) && (_tokenSet_56.member(LA(2)))) {
				identifier();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
				identifierOrValueTypecastOrFunctionCall_AST = static_cast<RefPascalAST>(currentAST.root);
			}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_56);
		} else {
			throw;
		}
	}
	returnAST = identifierOrValueTypecastOrFunctionCall_AST;
}

void PascalParser::unsignedConstant() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST unsignedConstant_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case NUM_INT:
		case NUM_REAL:
		{
			unsignedNumber();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			unsignedConstant_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case CHR:
		{
			constantChr();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			unsignedConstant_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case STRING_LITERAL:
		{
			string();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			unsignedConstant_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case NIL:
		{
			match(NIL);
			unsignedConstant_AST = static_cast<RefPascalAST>(currentAST.root);
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
			consume();
			consumeUntil(_tokenSet_56);
		} else {
			throw;
		}
	}
	returnAST = unsignedConstant_AST;
}

void PascalParser::setConstructor() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST setConstructor_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
		case AT:
		case STRING_LITERAL:
		case CHR:
		case NIL:
		case NUM_REAL:
		case IDENT:
		{
			setGroup();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			{ // ( ... )*
			for (;;) {
				if ((LA(1) == COMMA)) {
					match(COMMA);
					setGroup();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
					}
				}
				else {
					goto _loop263;
				}
				
			}
			_loop263:;
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
		setConstructor_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_56);
		} else {
			throw;
		}
	}
	returnAST = setConstructor_AST;
}

void PascalParser::addressFactor() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST addressFactor_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(AT);
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		addressFactor_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_56);
		} else {
			throw;
		}
	}
	returnAST = addressFactor_AST;
}

void PascalParser::expressions() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST expressions_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				expression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
			}
			else {
				goto _loop259;
			}
			
		}
		_loop259:;
		} // ( ... )*
		expressions_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_40);
		} else {
			throw;
		}
	}
	returnAST = expressions_AST;
}

void PascalParser::functionCall() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST functionCall_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{
		switch ( LA(1)) {
		case LPAREN:
		{
			actualParameterList();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
		functionCall_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_57);
		} else {
			throw;
		}
	}
	returnAST = functionCall_AST;
}

void PascalParser::actualParameterList() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST actualParameterList_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
		case AT:
		case STRING_LITERAL:
		case CHR:
		case NIL:
		case NUM_REAL:
		case IDENT:
		{
			expressions();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
		actualParameterList_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_57);
		} else {
			throw;
		}
	}
	returnAST = actualParameterList_AST;
}

void PascalParser::setGroup() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST setGroup_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{
		switch ( LA(1)) {
		case DOT:
		{
			match(DOT);
			match(DOT);
			expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
		setGroup_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_58);
		} else {
			throw;
		}
	}
	returnAST = setGroup_AST;
}

void PascalParser::valueTypecast() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST valueTypecast_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		typeIdentifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(LPAREN);
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(RPAREN);
		valueTypecast_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_0);
		} else {
			throw;
		}
	}
	returnAST = valueTypecast_AST;
}

void PascalParser::simpleStatement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST simpleStatement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case GOTO:
		{
			gotoStatement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			simpleStatement_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case RAISE:
		{
			raiseStatement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			simpleStatement_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		default:
			if ((LA(1) == IDENT) && (_tokenSet_59.member(LA(2)))) {
				assignmentStatement();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
				simpleStatement_AST = static_cast<RefPascalAST>(currentAST.root);
			}
			else if ((LA(1) == IDENT) && (_tokenSet_60.member(LA(2)))) {
				procedureStatement();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
				simpleStatement_AST = static_cast<RefPascalAST>(currentAST.root);
			}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_15);
		} else {
			throw;
		}
	}
	returnAST = simpleStatement_AST;
}

void PascalParser::structuredStatement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST structuredStatement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case BEGIN:
		{
			compoundStatement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			structuredStatement_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case WHILE:
		case REPEAT:
		case FOR:
		{
			repetitiveStatement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			structuredStatement_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case CASE:
		case IF:
		{
			conditionalStatement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			structuredStatement_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case TRY:
		{
			exceptionStatement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			structuredStatement_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case WITH:
		{
			withStatement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			structuredStatement_AST = static_cast<RefPascalAST>(currentAST.root);
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
			consume();
			consumeUntil(_tokenSet_15);
		} else {
			throw;
		}
	}
	returnAST = structuredStatement_AST;
}

void PascalParser::assignmentStatement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST assignmentStatement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		assignmentOperator();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		assignmentStatement_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_15);
		} else {
			throw;
		}
	}
	returnAST = assignmentStatement_AST;
}

void PascalParser::procedureStatement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST procedureStatement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{
		switch ( LA(1)) {
		case LPAREN:
		{
			actualParameterList();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
		procedureStatement_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_15);
		} else {
			throw;
		}
	}
	returnAST = procedureStatement_AST;
}

void PascalParser::gotoStatement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST gotoStatement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(GOTO);
		label();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		gotoStatement_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_15);
		} else {
			throw;
		}
	}
	returnAST = gotoStatement_AST;
}

void PascalParser::raiseStatement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST raiseStatement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp245_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp245_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp245_AST));
		}
		match(RAISE);
		{
		switch ( LA(1)) {
		case IDENT:
		{
			functionCall();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			{
			switch ( LA(1)) {
			case AT:
			{
				match(AT);
				addressConstant();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
		raiseStatement_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_15);
		} else {
			throw;
		}
	}
	returnAST = raiseStatement_AST;
}

void PascalParser::assignmentOperator() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST assignmentOperator_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case ASSIGN:
		{
			match(ASSIGN);
			assignmentOperator_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case PLUSEQ:
		{
			RefPascalAST tmp248_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp248_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp248_AST));
			}
			match(PLUSEQ);
			assignmentOperator_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case MINUSEQ:
		{
			RefPascalAST tmp249_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp249_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp249_AST));
			}
			match(MINUSEQ);
			assignmentOperator_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case STAREQ:
		{
			RefPascalAST tmp250_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp250_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp250_AST));
			}
			match(STAREQ);
			assignmentOperator_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case SLASHQE:
		{
			RefPascalAST tmp251_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp251_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp251_AST));
			}
			match(SLASHQE);
			assignmentOperator_AST = static_cast<RefPascalAST>(currentAST.root);
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
			consume();
			consumeUntil(_tokenSet_34);
		} else {
			throw;
		}
	}
	returnAST = assignmentOperator_AST;
}

void PascalParser::repetitiveStatement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST repetitiveStatement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case FOR:
		{
			forStatement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			repetitiveStatement_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case REPEAT:
		{
			repeatStatement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			repetitiveStatement_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case WHILE:
		{
			whileStatement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			repetitiveStatement_AST = static_cast<RefPascalAST>(currentAST.root);
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
			consume();
			consumeUntil(_tokenSet_15);
		} else {
			throw;
		}
	}
	returnAST = repetitiveStatement_AST;
}

void PascalParser::conditionalStatement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST conditionalStatement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case IF:
		{
			ifStatement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			conditionalStatement_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case CASE:
		{
			caseStatement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			conditionalStatement_AST = static_cast<RefPascalAST>(currentAST.root);
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
			consume();
			consumeUntil(_tokenSet_15);
		} else {
			throw;
		}
	}
	returnAST = conditionalStatement_AST;
}

void PascalParser::exceptionStatement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST exceptionStatement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		tryStatement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		exceptionStatement_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_15);
		} else {
			throw;
		}
	}
	returnAST = exceptionStatement_AST;
}

void PascalParser::withStatement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST withStatement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp252_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp252_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp252_AST));
		}
		match(WITH);
		recordVariableList();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(DO);
		statement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		withStatement_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_15);
		} else {
			throw;
		}
	}
	returnAST = withStatement_AST;
}

void PascalParser::ifStatement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST ifStatement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp254_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp254_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp254_AST));
		}
		match(IF);
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(THEN);
		statement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{
		if ((LA(1) == ELSE) && (_tokenSet_61.member(LA(2)))) {
			match(ELSE);
			statement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
		}
		else if ((_tokenSet_15.member(LA(1))) && (_tokenSet_62.member(LA(2)))) {
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
		ifStatement_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_15);
		} else {
			throw;
		}
	}
	returnAST = ifStatement_AST;
}

void PascalParser::caseStatement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST caseStatement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp257_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp257_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp257_AST));
		}
		match(CASE);
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(OF);
		caseListElement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == SEMI) && (_tokenSet_31.member(LA(2)))) {
				match(SEMI);
				caseListElement();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
			}
			else {
				goto _loop287;
			}
			
		}
		_loop287:;
		} // ( ... )*
		{
		switch ( LA(1)) {
		case SEMI:
		{
			match(SEMI);
			match(ELSE);
			statement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			{ // ( ... )*
			for (;;) {
				if ((LA(1) == SEMI)) {
					match(SEMI);
					statement();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
		caseStatement_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_15);
		} else {
			throw;
		}
	}
	returnAST = caseStatement_AST;
}

void PascalParser::forStatement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST forStatement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp264_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp264_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp264_AST));
		}
		match(FOR);
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(ASSIGN);
		forList();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(DO);
		statement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		forStatement_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_15);
		} else {
			throw;
		}
	}
	returnAST = forStatement_AST;
}

void PascalParser::repeatStatement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST repeatStatement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp267_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp267_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp267_AST));
		}
		match(REPEAT);
		statement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == SEMI)) {
				match(SEMI);
				statement();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
			}
			else {
				goto _loop298;
			}
			
		}
		_loop298:;
		} // ( ... )*
		match(UNTIL);
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		repeatStatement_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_15);
		} else {
			throw;
		}
	}
	returnAST = repeatStatement_AST;
}

void PascalParser::whileStatement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST whileStatement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp270_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp270_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp270_AST));
		}
		match(WHILE);
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(DO);
		statement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		whileStatement_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_15);
		} else {
			throw;
		}
	}
	returnAST = whileStatement_AST;
}

void PascalParser::caseListElement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST caseListElement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		constList();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		RefPascalAST tmp272_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp272_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp272_AST));
		}
		match(COLON);
		statement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		caseListElement_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_63);
		} else {
			throw;
		}
	}
	returnAST = caseListElement_AST;
}

void PascalParser::constList() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST constList_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		constant();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				constant();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
			}
			else {
				goto _loop294;
			}
			
		}
		_loop294:;
		} // ( ... )*
		if ( inputState->guessing==0 ) {
			constList_AST = RefPascalAST(currentAST.root);
#line 705 "pascal.g"
			constList_AST = RefPascalAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(CONSTLIST)))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(constList_AST))));
#line 7729 "PascalParser.cpp"
			currentAST.root = constList_AST;
			if ( constList_AST!=static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				constList_AST->getFirstChild() != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = constList_AST->getFirstChild();
			else
				currentAST.child = constList_AST;
			currentAST.advanceChildToEnd();
		}
		constList_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_51);
		} else {
			throw;
		}
	}
	returnAST = constList_AST;
}

void PascalParser::forList() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST forList_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		initialValue();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{
		switch ( LA(1)) {
		case TO:
		{
			RefPascalAST tmp274_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp274_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp274_AST));
			}
			match(TO);
			break;
		}
		case DOWNTO:
		{
			RefPascalAST tmp275_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp275_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp275_AST));
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
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		forList_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_64);
		} else {
			throw;
		}
	}
	returnAST = forList_AST;
}

void PascalParser::initialValue() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST initialValue_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		initialValue_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_65);
		} else {
			throw;
		}
	}
	returnAST = initialValue_AST;
}

void PascalParser::finalValue() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST finalValue_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		finalValue_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_64);
		} else {
			throw;
		}
	}
	returnAST = finalValue_AST;
}

void PascalParser::recordVariableList() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST recordVariableList_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		variable();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				variable();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
			}
			else {
				goto _loop307;
			}
			
		}
		_loop307:;
		} // ( ... )*
		recordVariableList_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_64);
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
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST variable_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case AT:
		{
			RefPascalAST tmp277_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp277_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp277_AST));
			}
			match(AT);
			identifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			break;
		}
		case IDENT:
		{
			identifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
				RefPascalAST tmp278_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp278_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp278_AST));
				}
				match(LBRACK);
				expression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
				{ // ( ... )*
				for (;;) {
					if ((LA(1) == COMMA)) {
						match(COMMA);
						expression();
						if (inputState->guessing==0) {
							astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
						}
					}
					else {
						goto _loop312;
					}
					
				}
				_loop312:;
				} // ( ... )*
				match(RBRACK);
				break;
			}
			case LBRACK2:
			{
				RefPascalAST tmp281_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp281_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp281_AST));
				}
				match(LBRACK2);
				expression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
				{ // ( ... )*
				for (;;) {
					if ((LA(1) == COMMA)) {
						match(COMMA);
						expression();
						if (inputState->guessing==0) {
							astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
						}
					}
					else {
						goto _loop314;
					}
					
				}
				_loop314:;
				} // ( ... )*
				match(RBRACK2);
				break;
			}
			case DOT:
			{
				RefPascalAST tmp284_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp284_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp284_AST));
				}
				match(DOT);
				identifier();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
				break;
			}
			case POINTER:
			{
				RefPascalAST tmp285_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp285_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp285_AST));
				}
				match(POINTER);
				break;
			}
			default:
			{
				goto _loop315;
			}
			}
		}
		_loop315:;
		} // ( ... )*
		variable_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_66);
		} else {
			throw;
		}
	}
	returnAST = variable_AST;
}

void PascalParser::operatorDefinition() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST operatorDefinition_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp286_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp286_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp286_AST));
		}
		match(OPERATOR);
		{
		switch ( LA(1)) {
		case ASSIGN:
		{
			assignmentOperatorDefinition();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(COLON);
		type();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(SEMI);
		subroutineBlock();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		operatorDefinition_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_0);
		} else {
			throw;
		}
	}
	returnAST = operatorDefinition_AST;
}

void PascalParser::assignmentOperatorDefinition() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST assignmentOperatorDefinition_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(ASSIGN);
		match(LPAREN);
		valueParameter();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(RPAREN);
		assignmentOperatorDefinition_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_67);
		} else {
			throw;
		}
	}
	returnAST = assignmentOperatorDefinition_AST;
}

void PascalParser::arithmeticOperatorDefinition() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST arithmeticOperatorDefinition_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(RPAREN);
		arithmeticOperatorDefinition_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_67);
		} else {
			throw;
		}
	}
	returnAST = arithmeticOperatorDefinition_AST;
}

void PascalParser::comparisonOperatorDefinition() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST comparisonOperatorDefinition_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(RPAREN);
		comparisonOperatorDefinition_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_67);
		} else {
			throw;
		}
	}
	returnAST = comparisonOperatorDefinition_AST;
}

void PascalParser::tryStatement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST tryStatement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp307_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp307_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp307_AST));
		}
		match(TRY);
		statement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == SEMI)) {
				match(SEMI);
				statement();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
			}
			else {
				goto _loop330;
			}
			
		}
		_loop330:;
		} // ( ... )*
		exceptOrFinallyPart();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(END);
		tryStatement_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_15);
		} else {
			throw;
		}
	}
	returnAST = tryStatement_AST;
}

void PascalParser::exceptOrFinallyPart() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST exceptOrFinallyPart_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
			exceptOrFinallyPart_AST = static_cast<RefPascalAST>(currentAST.root);
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
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
			exceptOrFinallyPart_AST = static_cast<RefPascalAST>(currentAST.root);
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
			consume();
			consumeUntil(_tokenSet_13);
		} else {
			throw;
		}
	}
	returnAST = exceptOrFinallyPart_AST;
}

void PascalParser::exceptionHandlers() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST exceptionHandlers_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			exceptionHandlers_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case ON:
		{
			exceptionHandler();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			{ // ( ... )*
			for (;;) {
				if ((LA(1) == SEMI)) {
					match(SEMI);
					exceptionHandler();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
					}
				}
				else {
					goto _loop339;
				}
				
			}
			_loop339:;
			} // ( ... )*
			{
			switch ( LA(1)) {
			case ELSE:
			{
				match(ELSE);
				statements();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
			exceptionHandlers_AST = static_cast<RefPascalAST>(currentAST.root);
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
			consume();
			consumeUntil(_tokenSet_13);
		} else {
			throw;
		}
	}
	returnAST = exceptionHandlers_AST;
}

void PascalParser::statements() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST statements_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		statement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == SEMI)) {
				match(SEMI);
				statement();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
			}
			else {
				goto _loop336;
			}
			
		}
		_loop336:;
		} // ( ... )*
		statements_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_13);
		} else {
			throw;
		}
	}
	returnAST = statements_AST;
}

void PascalParser::exceptionHandler() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST exceptionHandler_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(ON);
		{
		if ((LA(1) == IDENT) && (LA(2) == COLON)) {
			identifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(DO);
		statement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		exceptionHandler_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_68);
		} else {
			throw;
		}
	}
	returnAST = exceptionHandler_AST;
}

void PascalParser::sign() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST sign_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case PLUS:
		{
			RefPascalAST tmp318_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp318_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp318_AST));
			}
			match(PLUS);
			sign_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case MINUS:
		{
			RefPascalAST tmp319_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp319_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp319_AST));
			}
			match(MINUS);
			sign_AST = static_cast<RefPascalAST>(currentAST.root);
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
			consume();
			consumeUntil(_tokenSet_69);
		} else {
			throw;
		}
	}
	returnAST = sign_AST;
}

void PascalParser::constantChr() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST constantChr_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp320_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp320_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp320_AST));
		}
		match(CHR);
		match(LPAREN);
		unsignedInteger();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		match(RPAREN);
		constantChr_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_70);
		} else {
			throw;
		}
	}
	returnAST = constantChr_AST;
}

void PascalParser::unsignedNumber() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST unsignedNumber_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case NUM_INT:
		{
			unsignedInteger();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			unsignedNumber_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case NUM_REAL:
		{
			unsignedReal();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			unsignedNumber_AST = static_cast<RefPascalAST>(currentAST.root);
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
			consume();
			consumeUntil(_tokenSet_71);
		} else {
			throw;
		}
	}
	returnAST = unsignedNumber_AST;
}

void PascalParser::unsignedReal() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST unsignedReal_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp323_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp323_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp323_AST));
		}
		match(NUM_REAL);
		unsignedReal_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_71);
		} else {
			throw;
		}
	}
	returnAST = unsignedReal_AST;
}

RefPascalAST PascalParser::getAST()
{
	return returnAST;
}

void PascalParser::initializeASTFactory( ANTLR_USE_NAMESPACE(antlr)ASTFactory& factory )
{
	factory.setMaxNodeType(188);
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
	"\"exports\"",
	"COMMA",
	"\"index\"",
	"\"name\"",
	"\"uses\"",
	"\"unit\"",
	"\"end\"",
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
	"\"export\"",
	"\"exit\"",
	"\"false\"",
	"\"inherited\"",
	"\"new\"",
	"\"self\"",
	"\"true\"",
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
	"EXPONENT",
	0
};

const unsigned long PascalParser::_tokenSet_0_data_[] = { 2UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_0(_tokenSet_0_data_,6);
const unsigned long PascalParser::_tokenSet_1_data_[] = { 536870912UL, 177924UL, 0UL, 96UL, 0UL, 0UL, 0UL, 0UL };
// "uses" "begin" "label" "const" "resourcestring" "type" "var" "procedure" 
// "function" "constructor" "destructor" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_1(_tokenSet_1_data_,8);
const unsigned long PascalParser::_tokenSet_2_data_[] = { 2147483648UL, 177990UL, 0UL, 96UL, 0UL, 0UL, 0UL, 0UL };
// "end" "implementation" "begin" "initialization" "label" "const" "resourcestring" 
// "type" "var" "procedure" "function" "constructor" "destructor" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_2(_tokenSet_2_data_,8);
const unsigned long PascalParser::_tokenSet_3_data_[] = { 2202009602UL, 177988UL, 0UL, 96UL, 0UL, 0UL, 0UL, 0UL };
// EOF DOT SEMI "exports" "end" "begin" "initialization" "label" "const" 
// "resourcestring" "type" "var" "procedure" "function" "constructor" "destructor" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_3(_tokenSet_3_data_,8);
const unsigned long PascalParser::_tokenSet_4_data_[] = { 2638217218UL, 706736UL, 2198011904UL, 4294965472UL, 104246523UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF DOT SEMI COMMA "index" "name" "end" LPAREN RPAREN "finalization" 
// EQUAL COLON "procedure" "function" "of" DOTDOT ASSIGN LBRACK RBRACK 
// POINTER "constructor" "destructor" "class" "property" "read" "write" 
// "default" "nodefault" LE GE LTH GT NOT_EQUAL "in" "is" PLUS MINUS "or" 
// "xor" STAR SLASH "div" "mod" "and" "shl" "shr" AT PLUSEQ MINUSEQ STAREQ 
// SLASHQE "then" "else" "do" "until" "to" "downto" LBRACK2 RBRACK2 "except" 
// "finally" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_4(_tokenSet_4_data_,12);
const unsigned long PascalParser::_tokenSet_5_data_[] = { 4194304UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// DOT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_5(_tokenSet_5_data_,6);
const unsigned long PascalParser::_tokenSet_6_data_[] = { 71303168UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// DOT COMMA 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_6(_tokenSet_6_data_,6);
const unsigned long PascalParser::_tokenSet_7_data_[] = { 2503999490UL, 177988UL, 0UL, 63712UL, 0UL, 0UL, 0UL, 0UL };
// EOF DOT SEMI COMMA "name" "end" "begin" "initialization" "label" "const" 
// "resourcestring" "type" "var" "procedure" "function" "constructor" "destructor" 
// "class" "property" "read" "write" "default" "nodefault" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_7(_tokenSet_7_data_,8);
const unsigned long PascalParser::_tokenSet_8_data_[] = { 2638217218UL, 177988UL, 0UL, 96UL, 0UL, 0UL, 0UL, 0UL };
// EOF DOT SEMI COMMA "index" "name" "end" "begin" "initialization" "label" 
// "const" "resourcestring" "type" "var" "procedure" "function" "constructor" 
// "destructor" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_8(_tokenSet_8_data_,8);
const unsigned long PascalParser::_tokenSet_9_data_[] = { 16777216UL, 16416UL, 33554432UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RPAREN COLON RBRACK 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_9(_tokenSet_9_data_,8);
const unsigned long PascalParser::_tokenSet_10_data_[] = { 0UL, 2UL, 0UL, 0UL, 0UL, 0UL };
// "implementation" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_10(_tokenSet_10_data_,6);
const unsigned long PascalParser::_tokenSet_11_data_[] = { 2147483648UL, 68UL, 0UL, 0UL, 0UL, 0UL };
// "end" "begin" "initialization" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_11(_tokenSet_11_data_,6);
const unsigned long PascalParser::_tokenSet_12_data_[] = { 2147483648UL, 128UL, 0UL, 0UL, 0UL, 0UL };
// "end" "finalization" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_12(_tokenSet_12_data_,6);
const unsigned long PascalParser::_tokenSet_13_data_[] = { 2147483648UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "end" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_13(_tokenSet_13_data_,6);
const unsigned long PascalParser::_tokenSet_14_data_[] = { 0UL, 168450UL, 0UL, 0UL, 0UL, 0UL };
// "implementation" "const" "type" "procedure" "function" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_14(_tokenSet_14_data_,6);
const unsigned long PascalParser::_tokenSet_15_data_[] = { 2164260864UL, 128UL, 0UL, 0UL, 100698112UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI "end" "finalization" "else" "until" "except" "finally" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_15(_tokenSet_15_data_,12);
const unsigned long PascalParser::_tokenSet_16_data_[] = { 2147483648UL, 177988UL, 0UL, 96UL, 0UL, 0UL, 0UL, 0UL };
// "end" "begin" "initialization" "label" "const" "resourcestring" "type" 
// "var" "procedure" "function" "constructor" "destructor" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_16(_tokenSet_16_data_,8);
const unsigned long PascalParser::_tokenSet_17_data_[] = { 2231369728UL, 16512UL, 0UL, 0UL, 100698112UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI COMMA "end" "finalization" COLON "else" "until" "except" "finally" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_17(_tokenSet_17_data_,12);
const unsigned long PascalParser::_tokenSet_18_data_[] = { 2147483648UL, 177990UL, 0UL, 96UL, 0UL, 1UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "end" "implementation" "begin" "initialization" "label" "const" "resourcestring" 
// "type" "var" "procedure" "function" "constructor" "destructor" IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_18(_tokenSet_18_data_,12);
const unsigned long PascalParser::_tokenSet_19_data_[] = { 2147483648UL, 177988UL, 0UL, 96UL, 0UL, 1UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "end" "begin" "initialization" "label" "const" "resourcestring" "type" 
// "var" "procedure" "function" "constructor" "destructor" IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_19(_tokenSet_19_data_,12);
const unsigned long PascalParser::_tokenSet_20_data_[] = { 2638217218UL, 720884UL, 33619968UL, 4294904032UL, 103197699UL, 1UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF DOT SEMI COMMA "index" "name" "end" "begin" LPAREN RPAREN "initialization" 
// "finalization" "label" "const" "resourcestring" EQUAL "type" "var" COLON 
// "procedure" "function" "of" DOTDOT RBRACK "constructor" "destructor" 
// "class" "property" LE GE LTH GT NOT_EQUAL "in" "is" PLUS MINUS "or" 
// "xor" STAR SLASH "div" "mod" "and" "shl" "shr" "then" "else" "do" "until" 
// "to" "downto" RBRACK2 "except" "finally" IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_20(_tokenSet_20_data_,12);
const unsigned long PascalParser::_tokenSet_21_data_[] = { 16777216UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_21(_tokenSet_21_data_,6);
const unsigned long PascalParser::_tokenSet_22_data_[] = { 16777216UL, 2080UL, 33554432UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RPAREN EQUAL RBRACK 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_22(_tokenSet_22_data_,8);
const unsigned long PascalParser::_tokenSet_23_data_[] = { 2202009602UL, 178116UL, 0UL, 96UL, 100698112UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF DOT SEMI "exports" "end" "begin" "initialization" "finalization" 
// "label" "const" "resourcestring" "type" "var" "procedure" "function" 
// "constructor" "destructor" "else" "until" "except" "finally" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_23(_tokenSet_23_data_,12);
const unsigned long PascalParser::_tokenSet_24_data_[] = { 16777216UL, 526368UL, 33554432UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RPAREN EQUAL "of" RBRACK 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_24(_tokenSet_24_data_,8);
const unsigned long PascalParser::_tokenSet_25_data_[] = { 2164260866UL, 177988UL, 0UL, 96UL, 0UL, 0UL, 0UL, 0UL };
// EOF SEMI "end" "begin" "initialization" "label" "const" "resourcestring" 
// "type" "var" "procedure" "function" "constructor" "destructor" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_25(_tokenSet_25_data_,8);
const unsigned long PascalParser::_tokenSet_26_data_[] = { 0UL, 16UL, 0UL, 0UL, 0UL, 0UL };
// LPAREN 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_26(_tokenSet_26_data_,6);
const unsigned long PascalParser::_tokenSet_27_data_[] = { 16777216UL, 16416UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RPAREN COLON 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_27(_tokenSet_27_data_,6);
const unsigned long PascalParser::_tokenSet_28_data_[] = { 16777216UL, 32UL, 33554432UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RPAREN RBRACK 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_28(_tokenSet_28_data_,8);
const unsigned long PascalParser::_tokenSet_29_data_[] = { 0UL, 426000UL, 3976003580UL, 25165953UL, 2952790016UL, 1UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LPAREN "procedure" "function" "array" NUM_INT "integer" "shortint" "smallint" 
// "longint" "int64" "byte" "word" "cardinal" "qword" "boolean" BYTEBOOL 
// LONGBOOL "char" "real" "single" "double" "extended" "comp" "string" 
// "packed" "record" "set" "file" POINTER "object" "class" PLUS MINUS STRING_LITERAL 
// "chr" NUM_REAL IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_29(_tokenSet_29_data_,12);
const unsigned long PascalParser::_tokenSet_30_data_[] = { 2235564032UL, 524464UL, 33554432UL, 0UL, 103197696UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// DOT SEMI COMMA "end" LPAREN RPAREN "finalization" "of" RBRACK "then" 
// "else" "do" "until" "to" "downto" RBRACK2 "except" "finally" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_30(_tokenSet_30_data_,12);
const unsigned long PascalParser::_tokenSet_31_data_[] = { 0UL, 0UL, 4UL, 25165824UL, 2952790016UL, 1UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// NUM_INT PLUS MINUS STRING_LITERAL "chr" NUM_REAL IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_31(_tokenSet_31_data_,12);
const unsigned long PascalParser::_tokenSet_32_data_[] = { 16777216UL, 16UL, 4UL, 0UL, 2147483648UL, 1UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LPAREN NUM_INT NUM_REAL IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_32(_tokenSet_32_data_,12);
const unsigned long PascalParser::_tokenSet_33_data_[] = { 0UL, 16UL, 4UL, 25165824UL, 2952790016UL, 1UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LPAREN NUM_INT PLUS MINUS STRING_LITERAL "chr" NUM_REAL IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_33(_tokenSet_33_data_,12);
const unsigned long PascalParser::_tokenSet_34_data_[] = { 0UL, 16UL, 16777220UL, 25165824UL, 4026531852UL, 1UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LPAREN NUM_INT LBRACK PLUS MINUS "not" AT STRING_LITERAL "chr" "nil" 
// NUM_REAL IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_34(_tokenSet_34_data_,12);
const unsigned long PascalParser::_tokenSet_35_data_[] = { 16777216UL, 16UL, 50331652UL, 4294901760UL, 4026531855UL, 1UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LPAREN NUM_INT LBRACK RBRACK LE GE LTH GT NOT_EQUAL "in" "is" PLUS 
// MINUS "or" "xor" STAR SLASH "div" "mod" "and" "shl" "shr" "not" AT STRING_LITERAL 
// "chr" "nil" NUM_REAL IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_35(_tokenSet_35_data_,12);
const unsigned long PascalParser::_tokenSet_36_data_[] = { 2231369730UL, 182304UL, 33619968UL, 2272UL, 0UL, 0UL, 0UL, 0UL };
// EOF SEMI COMMA "end" RPAREN EQUAL COLON "procedure" "function" DOTDOT 
// RBRACK "constructor" "destructor" "class" "property" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_36(_tokenSet_36_data_,8);
const unsigned long PascalParser::_tokenSet_37_data_[] = { 83886080UL, 32UL, 0UL, 0UL, 0UL, 0UL };
// SEMI COMMA RPAREN 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_37(_tokenSet_37_data_,6);
const unsigned long PascalParser::_tokenSet_38_data_[] = { 83886080UL, 2080UL, 33554432UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI COMMA RPAREN EQUAL RBRACK 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_38(_tokenSet_38_data_,8);
const unsigned long PascalParser::_tokenSet_39_data_[] = { 2298478592UL, 165936UL, 33554432UL, 63712UL, 0UL, 0UL, 0UL, 0UL };
// SEMI "index" "end" LPAREN RPAREN EQUAL "procedure" "function" RBRACK 
// "constructor" "destructor" "class" "property" "read" "write" "default" 
// "nodefault" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_39(_tokenSet_39_data_,8);
const unsigned long PascalParser::_tokenSet_40_data_[] = { 0UL, 32UL, 0UL, 0UL, 0UL, 0UL };
// RPAREN 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_40(_tokenSet_40_data_,6);
const unsigned long PascalParser::_tokenSet_41_data_[] = { 2503999490UL, 720884UL, 33619968UL, 4294965472UL, 103197699UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF DOT SEMI COMMA "name" "end" "begin" LPAREN RPAREN "initialization" 
// "finalization" "label" "const" "resourcestring" EQUAL "type" "var" COLON 
// "procedure" "function" "of" DOTDOT RBRACK "constructor" "destructor" 
// "class" "property" "read" "write" "default" "nodefault" LE GE LTH GT 
// NOT_EQUAL "in" "is" PLUS MINUS "or" "xor" STAR SLASH "div" "mod" "and" 
// "shl" "shr" "then" "else" "do" "until" "to" "downto" RBRACK2 "except" 
// "finally" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_41(_tokenSet_41_data_,12);
const unsigned long PascalParser::_tokenSet_42_data_[] = { 2147483648UL, 32UL, 268435456UL, 0UL, 0UL, 1UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "end" RPAREN "case" IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_42(_tokenSet_42_data_,12);
const unsigned long PascalParser::_tokenSet_43_data_[] = { 2164260864UL, 32UL, 268435456UL, 0UL, 0UL, 1UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI "end" RPAREN "case" IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_43(_tokenSet_43_data_,12);
const unsigned long PascalParser::_tokenSet_44_data_[] = { 2147483648UL, 2260992UL, 0UL, 3320UL, 0UL, 1UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "end" "procedure" "function" "public" "private" "protected" "constructor" 
// "destructor" "class" "published" "property" IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_44(_tokenSet_44_data_,12);
const unsigned long PascalParser::_tokenSet_45_data_[] = { 0UL, 163840UL, 0UL, 96UL, 0UL, 0UL, 0UL, 0UL };
// "procedure" "function" "constructor" "destructor" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_45(_tokenSet_45_data_,8);
const unsigned long PascalParser::_tokenSet_46_data_[] = { 2147483648UL, 163840UL, 0UL, 2272UL, 0UL, 1UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "end" "procedure" "function" "constructor" "destructor" "class" "property" 
// IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_46(_tokenSet_46_data_,12);
const unsigned long PascalParser::_tokenSet_47_data_[] = { 2147483648UL, 163840UL, 0UL, 96UL, 0UL, 0UL, 0UL, 0UL };
// "end" "procedure" "function" "constructor" "destructor" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_47(_tokenSet_47_data_,8);
const unsigned long PascalParser::_tokenSet_48_data_[] = { 0UL, 163840UL, 0UL, 2272UL, 0UL, 0UL, 0UL, 0UL };
// "procedure" "function" "constructor" "destructor" "class" "property" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_48(_tokenSet_48_data_,8);
const unsigned long PascalParser::_tokenSet_49_data_[] = { 2147483648UL, 163840UL, 0UL, 2272UL, 0UL, 0UL, 0UL, 0UL };
// "end" "procedure" "function" "constructor" "destructor" "class" "property" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_49(_tokenSet_49_data_,8);
const unsigned long PascalParser::_tokenSet_50_data_[] = { 2147483648UL, 163840UL, 0UL, 63712UL, 0UL, 0UL, 0UL, 0UL };
// "end" "procedure" "function" "constructor" "destructor" "class" "property" 
// "read" "write" "default" "nodefault" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_50(_tokenSet_50_data_,8);
const unsigned long PascalParser::_tokenSet_51_data_[] = { 0UL, 16384UL, 0UL, 0UL, 0UL, 0UL };
// COLON 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_51(_tokenSet_51_data_,6);
const unsigned long PascalParser::_tokenSet_52_data_[] = { 2147483648UL, 163840UL, 0UL, 59616UL, 0UL, 0UL, 0UL, 0UL };
// "end" "procedure" "function" "constructor" "destructor" "class" "property" 
// "write" "default" "nodefault" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_52(_tokenSet_52_data_,8);
const unsigned long PascalParser::_tokenSet_53_data_[] = { 2147483648UL, 163840UL, 0UL, 51424UL, 0UL, 0UL, 0UL, 0UL };
// "end" "procedure" "function" "constructor" "destructor" "class" "property" 
// "default" "nodefault" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_53(_tokenSet_53_data_,8);
const unsigned long PascalParser::_tokenSet_54_data_[] = { 2235564032UL, 524464UL, 33554432UL, 8323072UL, 103197696UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// DOT SEMI COMMA "end" LPAREN RPAREN "finalization" "of" RBRACK LE GE 
// LTH GT NOT_EQUAL "in" "is" "then" "else" "do" "until" "to" "downto" 
// RBRACK2 "except" "finally" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_54(_tokenSet_54_data_,12);
const unsigned long PascalParser::_tokenSet_55_data_[] = { 2235564032UL, 524464UL, 33554432UL, 134152192UL, 103197696UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// DOT SEMI COMMA "end" LPAREN RPAREN "finalization" "of" RBRACK LE GE 
// LTH GT NOT_EQUAL "in" "is" PLUS MINUS "or" "xor" "then" "else" "do" 
// "until" "to" "downto" RBRACK2 "except" "finally" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_55(_tokenSet_55_data_,12);
const unsigned long PascalParser::_tokenSet_56_data_[] = { 2235564032UL, 524464UL, 33554432UL, 4294901760UL, 103197699UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// DOT SEMI COMMA "end" LPAREN RPAREN "finalization" "of" RBRACK LE GE 
// LTH GT NOT_EQUAL "in" "is" PLUS MINUS "or" "xor" STAR SLASH "div" "mod" 
// "and" "shl" "shr" "then" "else" "do" "until" "to" "downto" RBRACK2 "except" 
// "finally" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_56(_tokenSet_56_data_,12);
const unsigned long PascalParser::_tokenSet_57_data_[] = { 2164260864UL, 128UL, 0UL, 0UL, 100698120UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI "end" "finalization" AT "else" "until" "except" "finally" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_57(_tokenSet_57_data_,12);
const unsigned long PascalParser::_tokenSet_58_data_[] = { 67108864UL, 0UL, 33554432UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// COMMA RBRACK 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_58(_tokenSet_58_data_,8);
const unsigned long PascalParser::_tokenSet_59_data_[] = { 0UL, 0UL, 131072UL, 0UL, 240UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// ASSIGN PLUSEQ MINUSEQ STAREQ SLASHQE 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_59(_tokenSet_59_data_,12);
const unsigned long PascalParser::_tokenSet_60_data_[] = { 2164260864UL, 144UL, 0UL, 0UL, 100698112UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI "end" LPAREN "finalization" "else" "until" "except" "finally" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_60(_tokenSet_60_data_,12);
const unsigned long PascalParser::_tokenSet_61_data_[] = { 0UL, 4UL, 268435460UL, 0UL, 25776896UL, 1UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "begin" NUM_INT "case" "goto" "if" "while" "repeat" "for" "with" "raise" 
// "try" IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_61(_tokenSet_61_data_,12);
const unsigned long PascalParser::_tokenSet_62_data_[] = { 2202009602UL, 178132UL, 285212676UL, 25165920UL, 4287224588UL, 1UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF DOT SEMI "exports" "end" "begin" LPAREN "initialization" "finalization" 
// "label" "const" "resourcestring" "type" "var" "procedure" "function" 
// NUM_INT LBRACK "case" "constructor" "destructor" PLUS MINUS "not" AT 
// "goto" "if" "else" "while" "repeat" "until" "for" "with" "raise" "try" 
// "except" "finally" "on" STRING_LITERAL "chr" "nil" NUM_REAL IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_62(_tokenSet_62_data_,12);
const unsigned long PascalParser::_tokenSet_63_data_[] = { 2164260864UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI "end" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_63(_tokenSet_63_data_,6);
const unsigned long PascalParser::_tokenSet_64_data_[] = { 0UL, 0UL, 0UL, 0UL, 8192UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "do" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_64(_tokenSet_64_data_,12);
const unsigned long PascalParser::_tokenSet_65_data_[] = { 0UL, 0UL, 0UL, 0UL, 393216UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "to" "downto" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_65(_tokenSet_65_data_,12);
const unsigned long PascalParser::_tokenSet_66_data_[] = { 67108864UL, 0UL, 0UL, 0UL, 8192UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// COMMA "do" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_66(_tokenSet_66_data_,12);
const unsigned long PascalParser::_tokenSet_67_data_[] = { 0UL, 0UL, 0UL, 0UL, 0UL, 1UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_67(_tokenSet_67_data_,12);
const unsigned long PascalParser::_tokenSet_68_data_[] = { 2164260864UL, 0UL, 0UL, 0UL, 2048UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI "end" "else" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_68(_tokenSet_68_data_,12);
const unsigned long PascalParser::_tokenSet_69_data_[] = { 0UL, 0UL, 4UL, 0UL, 2147483648UL, 1UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// NUM_INT NUM_REAL IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_69(_tokenSet_69_data_,12);
const unsigned long PascalParser::_tokenSet_70_data_[] = { 2638217218UL, 720884UL, 33619968UL, 4294904032UL, 103197699UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF DOT SEMI COMMA "index" "name" "end" "begin" LPAREN RPAREN "initialization" 
// "finalization" "label" "const" "resourcestring" EQUAL "type" "var" COLON 
// "procedure" "function" "of" DOTDOT RBRACK "constructor" "destructor" 
// "class" "property" LE GE LTH GT NOT_EQUAL "in" "is" PLUS MINUS "or" 
// "xor" STAR SLASH "div" "mod" "and" "shl" "shr" "then" "else" "do" "until" 
// "to" "downto" RBRACK2 "except" "finally" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_70(_tokenSet_70_data_,12);
const unsigned long PascalParser::_tokenSet_71_data_[] = { 2235564034UL, 706736UL, 33619968UL, 4294904032UL, 103197699UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF DOT SEMI COMMA "end" LPAREN RPAREN "finalization" EQUAL COLON "procedure" 
// "function" "of" DOTDOT RBRACK "constructor" "destructor" "class" "property" 
// LE GE LTH GT NOT_EQUAL "in" "is" PLUS MINUS "or" "xor" STAR SLASH "div" 
// "mod" "and" "shl" "shr" "then" "else" "do" "until" "to" "downto" RBRACK2 
// "except" "finally" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_71(_tokenSet_71_data_,12);


