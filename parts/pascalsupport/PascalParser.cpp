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

void PascalParser::program() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST program_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		programHeading();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		{
		switch ( LA(1)) {
		case INTERFACE:
		{
			match(INTERFACE);
			break;
		}
		case IMPLEMENTATION:
		case USES:
		case LABEL:
		case CONST:
		case TYPE:
		case FUNCTION:
		case PROCEDURE:
		case VAR:
		case BEGIN:
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
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		match(DOT);
		program_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_0);
	}
	returnAST = program_AST;
}

void PascalParser::programHeading() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST programHeading_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case PROGRAM:
		{
			RefPascalAST tmp3_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp3_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp3_AST));
			match(PROGRAM);
			identifier();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			match(LPAREN);
			identifierList();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			match(RPAREN);
			match(SEMI);
			programHeading_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case UNIT:
		{
			RefPascalAST tmp7_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp7_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp7_AST));
			match(UNIT);
			identifier();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			match(SEMI);
			programHeading_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	returnAST = programHeading_AST;
}

void PascalParser::block() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST block_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{ // ( ... )*
		for (;;) {
			switch ( LA(1)) {
			case LABEL:
			{
				labelDeclarationPart();
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				break;
			}
			case CONST:
			{
				constantDefinitionPart();
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				break;
			}
			case TYPE:
			{
				typeDefinitionPart();
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				break;
			}
			case VAR:
			{
				variableDeclarationPart();
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				break;
			}
			case FUNCTION:
			case PROCEDURE:
			{
				procedureAndFunctionDeclarationPart();
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				break;
			}
			case USES:
			{
				usesUnitsPart();
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				break;
			}
			case IMPLEMENTATION:
			{
				RefPascalAST tmp9_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				tmp9_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp9_AST));
				match(IMPLEMENTATION);
				break;
			}
			default:
			{
				goto _loop7;
			}
			}
		}
		_loop7:;
		} // ( ... )*
		compoundStatement();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		block_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_2);
	}
	returnAST = block_AST;
}

void PascalParser::identifier() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST identifier_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp10_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp10_AST = astFactory->create(LT(1));
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp10_AST));
		match(IDENT);
		identifier_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_3);
	}
	returnAST = identifier_AST;
}

void PascalParser::identifierList() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST identifierList_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifier();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				identifier();
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			else {
				goto _loop80;
			}
			
		}
		_loop80:;
		} // ( ... )*
		identifierList_AST = RefPascalAST(currentAST.root);
#line 377 "pascal.g"
		identifierList_AST = RefPascalAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(IDLIST)))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(identifierList_AST))));
#line 248 "PascalParser.cpp"
		currentAST.root = identifierList_AST;
		if ( identifierList_AST!=static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			identifierList_AST->getFirstChild() != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = identifierList_AST->getFirstChild();
		else
			currentAST.child = identifierList_AST;
		currentAST.advanceChildToEnd();
		identifierList_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_4);
	}
	returnAST = identifierList_AST;
}

void PascalParser::labelDeclarationPart() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST labelDeclarationPart_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp12_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp12_AST = astFactory->create(LT(1));
		astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp12_AST));
		match(LABEL);
		label();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				label();
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			else {
				goto _loop11;
			}
			
		}
		_loop11:;
		} // ( ... )*
		match(SEMI);
		labelDeclarationPart_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_5);
	}
	returnAST = labelDeclarationPart_AST;
}

void PascalParser::constantDefinitionPart() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST constantDefinitionPart_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp15_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp15_AST = astFactory->create(LT(1));
		astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp15_AST));
		match(CONST);
		constantDefinition();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == SEMI) && (LA(2) == IDENT)) {
				match(SEMI);
				constantDefinition();
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			else {
				goto _loop15;
			}
			
		}
		_loop15:;
		} // ( ... )*
		match(SEMI);
		constantDefinitionPart_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_5);
	}
	returnAST = constantDefinitionPart_AST;
}

void PascalParser::typeDefinitionPart() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST typeDefinitionPart_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp18_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp18_AST = astFactory->create(LT(1));
		astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp18_AST));
		match(TYPE);
		typeDefinition();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == SEMI) && (LA(2) == IDENT)) {
				match(SEMI);
				typeDefinition();
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			else {
				goto _loop26;
			}
			
		}
		_loop26:;
		} // ( ... )*
		match(SEMI);
		typeDefinitionPart_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_5);
	}
	returnAST = typeDefinitionPart_AST;
}

/** Yields a list of VARDECL-rooted subtrees with VAR at the overall root */
void PascalParser::variableDeclarationPart() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST variableDeclarationPart_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp21_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp21_AST = astFactory->create(LT(1));
		astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp21_AST));
		match(VAR);
		variableDeclaration();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == SEMI) && (LA(2) == IDENT)) {
				match(SEMI);
				variableDeclaration();
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			else {
				goto _loop67;
			}
			
		}
		_loop67:;
		} // ( ... )*
		match(SEMI);
		variableDeclarationPart_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_5);
	}
	returnAST = variableDeclarationPart_AST;
}

void PascalParser::procedureAndFunctionDeclarationPart() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST procedureAndFunctionDeclarationPart_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		procedureOrFunctionDeclaration();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		match(SEMI);
		procedureAndFunctionDeclarationPart_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_5);
	}
	returnAST = procedureAndFunctionDeclarationPart_AST;
}

void PascalParser::usesUnitsPart() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST usesUnitsPart_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp25_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp25_AST = astFactory->create(LT(1));
		astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp25_AST));
		match(USES);
		identifierList();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		match(SEMI);
		usesUnitsPart_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_5);
	}
	returnAST = usesUnitsPart_AST;
}

void PascalParser::compoundStatement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST compoundStatement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(BEGIN);
		statements();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		match(END);
		compoundStatement_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_6);
	}
	returnAST = compoundStatement_AST;
}

void PascalParser::label() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST label_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		unsignedInteger();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		label_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_7);
	}
	returnAST = label_AST;
}

void PascalParser::unsignedInteger() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST unsignedInteger_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp29_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp29_AST = astFactory->create(LT(1));
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp29_AST));
		match(NUM_INT);
		unsignedInteger_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_8);
	}
	returnAST = unsignedInteger_AST;
}

void PascalParser::constantDefinition() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST constantDefinition_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifier();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		RefPascalAST tmp30_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp30_AST = astFactory->create(LT(1));
		astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp30_AST));
		match(EQUAL);
		constant();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		constantDefinition_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_9);
	}
	returnAST = constantDefinition_AST;
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
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			constant_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case IDENT:
		{
			identifier();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			constant_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case STRING_LITERAL:
		{
			string();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			constant_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case CHR:
		{
			constantChr();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			constant_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		default:
			if ((LA(1) == PLUS || LA(1) == MINUS) && (LA(2) == NUM_INT || LA(2) == NUM_REAL)) {
				sign();
				s_AST = returnAST;
				unsignedNumber();
				n_AST = returnAST;
				constant_AST = RefPascalAST(currentAST.root);
#line 169 "pascal.g"
				constant_AST=RefPascalAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(s_AST))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(n_AST))));
#line 588 "PascalParser.cpp"
				currentAST.root = constant_AST;
				if ( constant_AST!=static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
					constant_AST->getFirstChild() != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
					  currentAST.child = constant_AST->getFirstChild();
				else
					currentAST.child = constant_AST;
				currentAST.advanceChildToEnd();
			}
			else if ((LA(1) == PLUS || LA(1) == MINUS) && (LA(2) == IDENT)) {
				sign();
				s2_AST = returnAST;
				identifier();
				id_AST = returnAST;
				constant_AST = RefPascalAST(currentAST.root);
#line 171 "pascal.g"
				constant_AST=RefPascalAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(s2_AST))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(id_AST))));
#line 605 "PascalParser.cpp"
				currentAST.root = constant_AST;
				if ( constant_AST!=static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
					constant_AST->getFirstChild() != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
					  currentAST.child = constant_AST->getFirstChild();
				else
					currentAST.child = constant_AST;
				currentAST.advanceChildToEnd();
			}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_10);
	}
	returnAST = constant_AST;
}

void PascalParser::constantChr() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST constantChr_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp31_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp31_AST = astFactory->create(LT(1));
		astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp31_AST));
		match(CHR);
		match(LPAREN);
		unsignedInteger();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		match(RPAREN);
		constantChr_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_8);
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
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			unsignedNumber_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case NUM_REAL:
		{
			unsignedReal();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_8);
	}
	returnAST = unsignedNumber_AST;
}

void PascalParser::sign() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST sign_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case PLUS:
		{
			RefPascalAST tmp34_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp34_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp34_AST));
			match(PLUS);
			sign_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case MINUS:
		{
			RefPascalAST tmp35_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp35_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp35_AST));
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
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_11);
	}
	returnAST = sign_AST;
}

void PascalParser::string() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST string_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp36_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp36_AST = astFactory->create(LT(1));
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp36_AST));
		match(STRING_LITERAL);
		string_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_8);
	}
	returnAST = string_AST;
}

void PascalParser::unsignedReal() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST unsignedReal_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp37_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp37_AST = astFactory->create(LT(1));
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp37_AST));
		match(NUM_REAL);
		unsignedReal_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_8);
	}
	returnAST = unsignedReal_AST;
}

void PascalParser::typeDefinition() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST typeDefinition_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  e = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefPascalAST e_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifier();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		e = LT(1);
		e_AST = astFactory->create(e);
		astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(e_AST));
		match(EQUAL);
#line 203 "pascal.g"
		e_AST->setType(TYPEDECL);
#line 781 "PascalParser.cpp"
		{
		switch ( LA(1)) {
		case LPAREN:
		case IDENT:
		case CHR:
		case NUM_INT:
		case NUM_REAL:
		case PLUS:
		case MINUS:
		case STRING_LITERAL:
		case CHAR:
		case BOOLEAN:
		case INTEGER:
		case REAL:
		case STRING:
		case PACKED:
		case ARRAY:
		case RECORD:
		case SET:
		case FILE:
		case POINTER:
		{
			type();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			break;
		}
		case FUNCTION:
		{
			functionType();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			break;
		}
		case PROCEDURE:
		{
			procedureType();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		typeDefinition_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_9);
	}
	returnAST = typeDefinition_AST;
}

void PascalParser::type() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST type_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case LPAREN:
		case IDENT:
		case CHR:
		case NUM_INT:
		case NUM_REAL:
		case PLUS:
		case MINUS:
		case STRING_LITERAL:
		case CHAR:
		case BOOLEAN:
		case INTEGER:
		case REAL:
		case STRING:
		{
			simpleType();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			type_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case PACKED:
		case ARRAY:
		case RECORD:
		case SET:
		case FILE:
		{
			structuredType();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			type_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case POINTER:
		{
			pointerType();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_12);
	}
	returnAST = type_AST;
}

void PascalParser::functionType() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST functionType_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp38_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp38_AST = astFactory->create(LT(1));
		astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp38_AST));
		match(FUNCTION);
		{
		switch ( LA(1)) {
		case LPAREN:
		{
			formalParameterList();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
		resultType();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		functionType_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_9);
	}
	returnAST = functionType_AST;
}

void PascalParser::procedureType() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST procedureType_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp40_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp40_AST = astFactory->create(LT(1));
		astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp40_AST));
		match(PROCEDURE);
		{
		switch ( LA(1)) {
		case LPAREN:
		{
			formalParameterList();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
		procedureType_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_9);
	}
	returnAST = procedureType_AST;
}

void PascalParser::formalParameterList() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST formalParameterList_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp41_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp41_AST = astFactory->create(LT(1));
		astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp41_AST));
		match(LPAREN);
		formalParameterSection();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == SEMI)) {
				match(SEMI);
				formalParameterSection();
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			else {
				goto _loop75;
			}
			
		}
		_loop75:;
		} // ( ... )*
		match(RPAREN);
		formalParameterList_AST = RefPascalAST(currentAST.root);
#line 360 "pascal.g"
		formalParameterList_AST->setType(ARGDECLS);
#line 1003 "PascalParser.cpp"
		formalParameterList_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_13);
	}
	returnAST = formalParameterList_AST;
}

void PascalParser::resultType() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST resultType_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		typeIdentifier();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		resultType_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_9);
	}
	returnAST = resultType_AST;
}

void PascalParser::simpleType() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST simpleType_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		if ((LA(1) == LPAREN)) {
			scalarType();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			simpleType_AST = static_cast<RefPascalAST>(currentAST.root);
		}
		else if ((_tokenSet_14.member(LA(1))) && (_tokenSet_15.member(LA(2)))) {
			subrangeType();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			simpleType_AST = static_cast<RefPascalAST>(currentAST.root);
		}
		else if ((_tokenSet_16.member(LA(1))) && (_tokenSet_17.member(LA(2)))) {
			typeIdentifier();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			simpleType_AST = static_cast<RefPascalAST>(currentAST.root);
		}
		else if ((LA(1) == STRING) && (LA(2) == LBRACK)) {
			stringtype();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			simpleType_AST = static_cast<RefPascalAST>(currentAST.root);
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_17);
	}
	returnAST = simpleType_AST;
}

void PascalParser::structuredType() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST structuredType_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case PACKED:
		{
			RefPascalAST tmp44_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp44_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp44_AST));
			match(PACKED);
			unpackedStructuredType();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			structuredType_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case ARRAY:
		case RECORD:
		case SET:
		case FILE:
		{
			unpackedStructuredType();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			structuredType_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_12);
	}
	returnAST = structuredType_AST;
}

void PascalParser::pointerType() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST pointerType_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp45_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp45_AST = astFactory->create(LT(1));
		astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp45_AST));
		match(POINTER);
		typeIdentifier();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		pointerType_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_12);
	}
	returnAST = pointerType_AST;
}

void PascalParser::scalarType() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST scalarType_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp46_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp46_AST = astFactory->create(LT(1));
		astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp46_AST));
		match(LPAREN);
		identifierList();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		match(RPAREN);
		scalarType_AST = RefPascalAST(currentAST.root);
#line 234 "pascal.g"
		scalarType_AST->setType(SCALARTYPE);
#line 1151 "PascalParser.cpp"
		scalarType_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_17);
	}
	returnAST = scalarType_AST;
}

void PascalParser::subrangeType() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST subrangeType_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		constant();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		RefPascalAST tmp48_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp48_AST = astFactory->create(LT(1));
		astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp48_AST));
		match(DOTDOT);
		constant();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		subrangeType_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_17);
	}
	returnAST = subrangeType_AST;
}

void PascalParser::typeIdentifier() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST typeIdentifier_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case IDENT:
		{
			identifier();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			typeIdentifier_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case CHAR:
		{
			RefPascalAST tmp49_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp49_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp49_AST));
			match(CHAR);
			typeIdentifier_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case BOOLEAN:
		{
			RefPascalAST tmp50_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp50_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp50_AST));
			match(BOOLEAN);
			typeIdentifier_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case INTEGER:
		{
			RefPascalAST tmp51_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp51_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp51_AST));
			match(INTEGER);
			typeIdentifier_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case REAL:
		{
			RefPascalAST tmp52_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp52_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp52_AST));
			match(REAL);
			typeIdentifier_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case STRING:
		{
			RefPascalAST tmp53_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp53_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp53_AST));
			match(STRING);
			typeIdentifier_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_18);
	}
	returnAST = typeIdentifier_AST;
}

void PascalParser::stringtype() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST stringtype_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp54_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp54_AST = astFactory->create(LT(1));
		astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp54_AST));
		match(STRING);
		match(LBRACK);
		{
		switch ( LA(1)) {
		case IDENT:
		{
			identifier();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			break;
		}
		case NUM_INT:
		case NUM_REAL:
		{
			unsignedNumber();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(RBRACK);
		stringtype_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_17);
	}
	returnAST = stringtype_AST;
}

void PascalParser::unpackedStructuredType() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST unpackedStructuredType_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case ARRAY:
		{
			arrayType();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			unpackedStructuredType_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case RECORD:
		{
			recordType();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			unpackedStructuredType_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case SET:
		{
			setType();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			unpackedStructuredType_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case FILE:
		{
			fileType();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			unpackedStructuredType_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_12);
	}
	returnAST = unpackedStructuredType_AST;
}

void PascalParser::arrayType() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST arrayType_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		if ((LA(1) == ARRAY) && (LA(2) == LBRACK)) {
			RefPascalAST tmp57_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp57_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp57_AST));
			match(ARRAY);
			match(LBRACK);
			typeList();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			match(RBRACK);
			match(OF);
			componentType();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			arrayType_AST = static_cast<RefPascalAST>(currentAST.root);
		}
		else if ((LA(1) == ARRAY) && (LA(2) == LBRACK2)) {
			RefPascalAST tmp61_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp61_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp61_AST));
			match(ARRAY);
			match(LBRACK2);
			typeList();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			match(RBRACK2);
			match(OF);
			componentType();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			arrayType_AST = static_cast<RefPascalAST>(currentAST.root);
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_12);
	}
	returnAST = arrayType_AST;
}

void PascalParser::recordType() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST recordType_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp65_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp65_AST = astFactory->create(LT(1));
		astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp65_AST));
		match(RECORD);
		fieldList();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		match(END);
		recordType_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_12);
	}
	returnAST = recordType_AST;
}

void PascalParser::setType() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST setType_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp67_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp67_AST = astFactory->create(LT(1));
		astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp67_AST));
		match(SET);
		match(OF);
		baseType();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		setType_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_12);
	}
	returnAST = setType_AST;
}

void PascalParser::fileType() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST fileType_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		if ((LA(1) == FILE) && (LA(2) == OF)) {
			RefPascalAST tmp69_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp69_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp69_AST));
			match(FILE);
			match(OF);
			type();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			fileType_AST = static_cast<RefPascalAST>(currentAST.root);
		}
		else if ((LA(1) == FILE) && (_tokenSet_12.member(LA(2)))) {
			RefPascalAST tmp71_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp71_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp71_AST));
			match(FILE);
			fileType_AST = static_cast<RefPascalAST>(currentAST.root);
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_12);
	}
	returnAST = fileType_AST;
}

void PascalParser::typeList() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST typeList_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		indexType();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				indexType();
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			else {
				goto _loop45;
			}
			
		}
		_loop45:;
		} // ( ... )*
		typeList_AST = RefPascalAST(currentAST.root);
#line 273 "pascal.g"
		typeList_AST = RefPascalAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(TYPELIST)))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(typeList_AST))));
#line 1505 "PascalParser.cpp"
		currentAST.root = typeList_AST;
		if ( typeList_AST!=static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			typeList_AST->getFirstChild() != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = typeList_AST->getFirstChild();
		else
			currentAST.child = typeList_AST;
		currentAST.advanceChildToEnd();
		typeList_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_19);
	}
	returnAST = typeList_AST;
}

void PascalParser::componentType() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST componentType_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		type();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		componentType_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_12);
	}
	returnAST = componentType_AST;
}

void PascalParser::indexType() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST indexType_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		simpleType();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		indexType_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_20);
	}
	returnAST = indexType_AST;
}

void PascalParser::fieldList() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST fieldList_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case IDENT:
		{
			fixedPart();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			{
			if ((LA(1) == SEMI) && (LA(2) == CASE)) {
				match(SEMI);
				variantPart();
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			else if ((LA(1) == SEMI) && (LA(2) == RPAREN || LA(2) == END)) {
				match(SEMI);
			}
			else if ((LA(1) == RPAREN || LA(1) == END)) {
			}
			else {
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			
			}
			break;
		}
		case CASE:
		{
			variantPart();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		fieldList_AST = RefPascalAST(currentAST.root);
#line 292 "pascal.g"
		fieldList_AST=RefPascalAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(FIELDLIST)))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(fieldList_AST))));
#line 1604 "PascalParser.cpp"
		currentAST.root = fieldList_AST;
		if ( fieldList_AST!=static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			fieldList_AST->getFirstChild() != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = fieldList_AST->getFirstChild();
		else
			currentAST.child = fieldList_AST;
		currentAST.advanceChildToEnd();
		fieldList_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_21);
	}
	returnAST = fieldList_AST;
}

void PascalParser::fixedPart() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST fixedPart_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		recordSection();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == SEMI) && (LA(2) == IDENT)) {
				match(SEMI);
				recordSection();
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			else {
				goto _loop54;
			}
			
		}
		_loop54:;
		} // ( ... )*
		fixedPart_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_12);
	}
	returnAST = fixedPart_AST;
}

void PascalParser::variantPart() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST variantPart_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp76_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp76_AST = astFactory->create(LT(1));
		astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp76_AST));
		match(CASE);
		tag();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		match(OF);
		variant();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == SEMI) && (_tokenSet_14.member(LA(2)))) {
				match(SEMI);
				variant();
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			else if ((LA(1) == SEMI) && (_tokenSet_12.member(LA(2)))) {
				match(SEMI);
			}
			else {
				goto _loop58;
			}
			
		}
		_loop58:;
		} // ( ... )*
		variantPart_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_21);
	}
	returnAST = variantPart_AST;
}

void PascalParser::recordSection() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST recordSection_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifierList();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		match(COLON);
		type();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		recordSection_AST = RefPascalAST(currentAST.root);
#line 301 "pascal.g"
		recordSection_AST = RefPascalAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(FIELD)))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(recordSection_AST))));
#line 1710 "PascalParser.cpp"
		currentAST.root = recordSection_AST;
		if ( recordSection_AST!=static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			recordSection_AST->getFirstChild() != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = recordSection_AST->getFirstChild();
		else
			currentAST.child = recordSection_AST;
		currentAST.advanceChildToEnd();
		recordSection_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_12);
	}
	returnAST = recordSection_AST;
}

void PascalParser::tag() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST tag_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefPascalAST id_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefPascalAST t_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefPascalAST t2_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		if ((LA(1) == IDENT) && (LA(2) == COLON)) {
			identifier();
			id_AST = returnAST;
			RefPascalAST tmp81_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp81_AST = astFactory->create(LT(1));
			match(COLON);
			typeIdentifier();
			t_AST = returnAST;
			tag_AST = RefPascalAST(currentAST.root);
#line 309 "pascal.g"
			tag_AST=RefPascalAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(3))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(VARIANT_TAG)))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(id_AST))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(t_AST))));
#line 1748 "PascalParser.cpp"
			currentAST.root = tag_AST;
			if ( tag_AST!=static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				tag_AST->getFirstChild() != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = tag_AST->getFirstChild();
			else
				currentAST.child = tag_AST;
			currentAST.advanceChildToEnd();
		}
		else if ((_tokenSet_16.member(LA(1))) && (LA(2) == OF)) {
			typeIdentifier();
			t2_AST = returnAST;
			tag_AST = RefPascalAST(currentAST.root);
#line 310 "pascal.g"
			tag_AST=RefPascalAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(VARIANT_TAG_NO_ID)))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(t2_AST))));
#line 1763 "PascalParser.cpp"
			currentAST.root = tag_AST;
			if ( tag_AST!=static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				tag_AST->getFirstChild() != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = tag_AST->getFirstChild();
			else
				currentAST.child = tag_AST;
			currentAST.advanceChildToEnd();
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_22);
	}
	returnAST = tag_AST;
}

void PascalParser::variant() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST variant_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  c = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefPascalAST c_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		constList();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		c = LT(1);
		c_AST = astFactory->create(c);
		astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(c_AST));
		match(COLON);
#line 314 "pascal.g"
		c_AST->setType(VARIANT_CASE);
#line 1801 "PascalParser.cpp"
		match(LPAREN);
		fieldList();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		match(RPAREN);
		variant_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_12);
	}
	returnAST = variant_AST;
}

void PascalParser::constList() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST constList_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		constant();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				constant();
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			else {
				goto _loop83;
			}
			
		}
		_loop83:;
		} // ( ... )*
		constList_AST = RefPascalAST(currentAST.root);
#line 382 "pascal.g"
		constList_AST = RefPascalAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(CONSTLIST)))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(constList_AST))));
#line 1841 "PascalParser.cpp"
		currentAST.root = constList_AST;
		if ( constList_AST!=static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			constList_AST->getFirstChild() != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = constList_AST->getFirstChild();
		else
			currentAST.child = constList_AST;
		currentAST.advanceChildToEnd();
		constList_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_23);
	}
	returnAST = constList_AST;
}

void PascalParser::baseType() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST baseType_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		simpleType();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		baseType_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_12);
	}
	returnAST = baseType_AST;
}

void PascalParser::variableDeclaration() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST variableDeclaration_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  c = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefPascalAST c_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifierList();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		c = LT(1);
		c_AST = astFactory->create(c);
		astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(c_AST));
		match(COLON);
#line 341 "pascal.g"
		c_AST->setType(VARDECL);
#line 1893 "PascalParser.cpp"
		type();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		variableDeclaration_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_9);
	}
	returnAST = variableDeclaration_AST;
}

void PascalParser::procedureOrFunctionDeclaration() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST procedureOrFunctionDeclaration_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case PROCEDURE:
		{
			procedureDeclaration();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			procedureOrFunctionDeclaration_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case FUNCTION:
		{
			functionDeclaration();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			procedureOrFunctionDeclaration_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_9);
	}
	returnAST = procedureOrFunctionDeclaration_AST;
}

void PascalParser::procedureDeclaration() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST procedureDeclaration_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp85_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp85_AST = astFactory->create(LT(1));
		astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp85_AST));
		match(PROCEDURE);
		identifier();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		{
		switch ( LA(1)) {
		case LPAREN:
		{
			formalParameterList();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
		block();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		procedureDeclaration_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_9);
	}
	returnAST = procedureDeclaration_AST;
}

void PascalParser::functionDeclaration() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST functionDeclaration_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp87_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp87_AST = astFactory->create(LT(1));
		astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp87_AST));
		match(FUNCTION);
		identifier();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		{
		switch ( LA(1)) {
		case LPAREN:
		{
			formalParameterList();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
		resultType();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		match(SEMI);
		block();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		functionDeclaration_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_9);
	}
	returnAST = functionDeclaration_AST;
}

void PascalParser::formalParameterSection() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST formalParameterSection_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case IDENT:
		{
			parameterGroup();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			formalParameterSection_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case VAR:
		{
			RefPascalAST tmp90_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp90_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp90_AST));
			match(VAR);
			parameterGroup();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			formalParameterSection_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case FUNCTION:
		{
			RefPascalAST tmp91_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp91_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp91_AST));
			match(FUNCTION);
			parameterGroup();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			formalParameterSection_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case PROCEDURE:
		{
			RefPascalAST tmp92_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp92_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp92_AST));
			match(PROCEDURE);
			parameterGroup();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			formalParameterSection_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_24);
	}
	returnAST = formalParameterSection_AST;
}

void PascalParser::parameterGroup() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST parameterGroup_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefPascalAST ids_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefPascalAST t_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifierList();
		ids_AST = returnAST;
		match(COLON);
		typeIdentifier();
		t_AST = returnAST;
		parameterGroup_AST = RefPascalAST(currentAST.root);
#line 372 "pascal.g"
		parameterGroup_AST = RefPascalAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(3))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(ARGDECL)))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(ids_AST))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(t_AST))));
#line 2107 "PascalParser.cpp"
		currentAST.root = parameterGroup_AST;
		if ( parameterGroup_AST!=static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			parameterGroup_AST->getFirstChild() != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = parameterGroup_AST->getFirstChild();
		else
			currentAST.child = parameterGroup_AST;
		currentAST.advanceChildToEnd();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_24);
	}
	returnAST = parameterGroup_AST;
}

void PascalParser::statement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST statement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case NUM_INT:
		{
			label();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			RefPascalAST tmp94_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp94_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp94_AST));
			match(COLON);
			unlabelledStatement();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			statement_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case SEMI:
		case IDENT:
		case END:
		case CASE:
		case AT:
		case GOTO:
		case BEGIN:
		case IF:
		case ELSE:
		case WHILE:
		case REPEAT:
		case UNTIL:
		case FOR:
		case WITH:
		{
			unlabelledStatement();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			statement_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_25);
	}
	returnAST = statement_AST;
}

void PascalParser::unlabelledStatement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST unlabelledStatement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case SEMI:
		case IDENT:
		case END:
		case AT:
		case GOTO:
		case ELSE:
		case UNTIL:
		{
			simpleStatement();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			unlabelledStatement_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case CASE:
		case BEGIN:
		case IF:
		case WHILE:
		case REPEAT:
		case FOR:
		case WITH:
		{
			structuredStatement();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			unlabelledStatement_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_25);
	}
	returnAST = unlabelledStatement_AST;
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
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			simpleStatement_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case SEMI:
		case END:
		case ELSE:
		case UNTIL:
		{
			emptyStatement();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			simpleStatement_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		default:
			if ((LA(1) == IDENT || LA(1) == AT) && (_tokenSet_26.member(LA(2)))) {
				assignmentStatement();
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				simpleStatement_AST = static_cast<RefPascalAST>(currentAST.root);
			}
			else if ((LA(1) == IDENT) && (_tokenSet_27.member(LA(2)))) {
				procedureStatement();
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				simpleStatement_AST = static_cast<RefPascalAST>(currentAST.root);
			}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_25);
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
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			structuredStatement_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case CASE:
		case IF:
		{
			conditionalStatement();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			structuredStatement_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case WHILE:
		case REPEAT:
		case FOR:
		{
			repetetiveStatement();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			structuredStatement_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case WITH:
		{
			withStatement();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_25);
	}
	returnAST = structuredStatement_AST;
}

void PascalParser::assignmentStatement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST assignmentStatement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		variable();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		RefPascalAST tmp95_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp95_AST = astFactory->create(LT(1));
		astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp95_AST));
		match(ASSIGN);
		expression();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		assignmentStatement_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_25);
	}
	returnAST = assignmentStatement_AST;
}

void PascalParser::procedureStatement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST procedureStatement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefPascalAST id_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefPascalAST args_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifier();
		id_AST = returnAST;
		{
		switch ( LA(1)) {
		case LPAREN:
		{
			match(LPAREN);
			parameterList();
			args_AST = returnAST;
			match(RPAREN);
			break;
		}
		case SEMI:
		case END:
		case ELSE:
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
		procedureStatement_AST = RefPascalAST(currentAST.root);
#line 500 "pascal.g"
		procedureStatement_AST = RefPascalAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(3))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(PROC_CALL)))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(id_AST))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(args_AST))));
#line 2385 "PascalParser.cpp"
		currentAST.root = procedureStatement_AST;
		if ( procedureStatement_AST!=static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			procedureStatement_AST->getFirstChild() != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = procedureStatement_AST->getFirstChild();
		else
			currentAST.child = procedureStatement_AST;
		currentAST.advanceChildToEnd();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_25);
	}
	returnAST = procedureStatement_AST;
}

void PascalParser::gotoStatement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST gotoStatement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp98_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp98_AST = astFactory->create(LT(1));
		astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp98_AST));
		match(GOTO);
		label();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		gotoStatement_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_25);
	}
	returnAST = gotoStatement_AST;
}

void PascalParser::emptyStatement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST emptyStatement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		emptyStatement_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_25);
	}
	returnAST = emptyStatement_AST;
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
			RefPascalAST tmp99_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp99_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp99_AST));
			match(AT);
			identifier();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			break;
		}
		case IDENT:
		{
			identifier();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
				RefPascalAST tmp100_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				tmp100_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp100_AST));
				match(LBRACK);
				expression();
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				{ // ( ... )*
				for (;;) {
					if ((LA(1) == COMMA)) {
						match(COMMA);
						expression();
						astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
					}
					else {
						goto _loop95;
					}
					
				}
				_loop95:;
				} // ( ... )*
				match(RBRACK);
				break;
			}
			case LBRACK2:
			{
				RefPascalAST tmp103_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				tmp103_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp103_AST));
				match(LBRACK2);
				expression();
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				{ // ( ... )*
				for (;;) {
					if ((LA(1) == COMMA)) {
						match(COMMA);
						expression();
						astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
					}
					else {
						goto _loop97;
					}
					
				}
				_loop97:;
				} // ( ... )*
				match(RBRACK2);
				break;
			}
			case DOT:
			{
				RefPascalAST tmp106_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				tmp106_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp106_AST));
				match(DOT);
				identifier();
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				break;
			}
			case POINTER:
			{
				RefPascalAST tmp107_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				tmp107_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp107_AST));
				match(POINTER);
				break;
			}
			default:
			{
				goto _loop98;
			}
			}
		}
		_loop98:;
		} // ( ... )*
		variable_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_28);
	}
	returnAST = variable_AST;
}

void PascalParser::expression() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST expression_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		simpleExpression();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		{ // ( ... )*
		for (;;) {
			if ((_tokenSet_29.member(LA(1)))) {
				{
				switch ( LA(1)) {
				case EQUAL:
				{
					RefPascalAST tmp108_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					tmp108_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp108_AST));
					match(EQUAL);
					break;
				}
				case NOT_EQUAL:
				{
					RefPascalAST tmp109_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					tmp109_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp109_AST));
					match(NOT_EQUAL);
					break;
				}
				case LTH:
				{
					RefPascalAST tmp110_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					tmp110_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp110_AST));
					match(LTH);
					break;
				}
				case LE:
				{
					RefPascalAST tmp111_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					tmp111_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp111_AST));
					match(LE);
					break;
				}
				case GE:
				{
					RefPascalAST tmp112_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					tmp112_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp112_AST));
					match(GE);
					break;
				}
				case GT:
				{
					RefPascalAST tmp113_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					tmp113_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp113_AST));
					match(GT);
					break;
				}
				case IN:
				{
					RefPascalAST tmp114_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					tmp114_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp114_AST));
					match(IN);
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				simpleExpression();
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			else {
				goto _loop102;
			}
			
		}
		_loop102:;
		} // ( ... )*
		expression_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_30);
	}
	returnAST = expression_AST;
}

void PascalParser::simpleExpression() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST simpleExpression_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		term();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		{ // ( ... )*
		for (;;) {
			if ((_tokenSet_31.member(LA(1)))) {
				{
				switch ( LA(1)) {
				case PLUS:
				{
					RefPascalAST tmp115_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					tmp115_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp115_AST));
					match(PLUS);
					break;
				}
				case MINUS:
				{
					RefPascalAST tmp116_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					tmp116_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp116_AST));
					match(MINUS);
					break;
				}
				case OR:
				{
					RefPascalAST tmp117_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					tmp117_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp117_AST));
					match(OR);
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				term();
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			else {
				goto _loop106;
			}
			
		}
		_loop106:;
		} // ( ... )*
		simpleExpression_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_32);
	}
	returnAST = simpleExpression_AST;
}

void PascalParser::term() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST term_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		signedFactor();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		{ // ( ... )*
		for (;;) {
			if (((LA(1) >= STAR && LA(1) <= AND))) {
				{
				switch ( LA(1)) {
				case STAR:
				{
					RefPascalAST tmp118_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					tmp118_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp118_AST));
					match(STAR);
					break;
				}
				case SLASH:
				{
					RefPascalAST tmp119_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					tmp119_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp119_AST));
					match(SLASH);
					break;
				}
				case DIV:
				{
					RefPascalAST tmp120_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					tmp120_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp120_AST));
					match(DIV);
					break;
				}
				case MOD:
				{
					RefPascalAST tmp121_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					tmp121_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp121_AST));
					match(MOD);
					break;
				}
				case AND:
				{
					RefPascalAST tmp122_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					tmp122_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp122_AST));
					match(AND);
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				signedFactor();
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			else {
				goto _loop110;
			}
			
		}
		_loop110:;
		} // ( ... )*
		term_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_33);
	}
	returnAST = term_AST;
}

void PascalParser::signedFactor() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST signedFactor_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case PLUS:
		{
			RefPascalAST tmp123_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp123_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp123_AST));
			match(PLUS);
			break;
		}
		case MINUS:
		{
			RefPascalAST tmp124_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp124_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp124_AST));
			match(MINUS);
			break;
		}
		case LPAREN:
		case IDENT:
		case CHR:
		case NUM_INT:
		case NUM_REAL:
		case STRING_LITERAL:
		case LBRACK:
		case LBRACK2:
		case AT:
		case NOT:
		case NIL:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		factor();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		signedFactor_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_34);
	}
	returnAST = signedFactor_AST;
}

void PascalParser::factor() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST factor_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case LPAREN:
		{
			match(LPAREN);
			expression();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			match(RPAREN);
			factor_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case CHR:
		case NUM_INT:
		case NUM_REAL:
		case STRING_LITERAL:
		case NIL:
		{
			unsignedConstant();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			factor_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case LBRACK:
		case LBRACK2:
		{
			set();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			factor_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case NOT:
		{
			RefPascalAST tmp127_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp127_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp127_AST));
			match(NOT);
			factor();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			factor_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		default:
			if ((LA(1) == IDENT || LA(1) == AT) && (_tokenSet_35.member(LA(2)))) {
				variable();
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				factor_AST = static_cast<RefPascalAST>(currentAST.root);
			}
			else if ((LA(1) == IDENT) && (LA(2) == LPAREN)) {
				functionDesignator();
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				factor_AST = static_cast<RefPascalAST>(currentAST.root);
			}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_34);
	}
	returnAST = factor_AST;
}

void PascalParser::functionDesignator() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST functionDesignator_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefPascalAST id_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefPascalAST args_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifier();
		id_AST = returnAST;
		match(LPAREN);
		parameterList();
		args_AST = returnAST;
		match(RPAREN);
		functionDesignator_AST = RefPascalAST(currentAST.root);
#line 476 "pascal.g"
		functionDesignator_AST = RefPascalAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(3))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(FUNC_CALL)))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(id_AST))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(args_AST))));
#line 2954 "PascalParser.cpp"
		currentAST.root = functionDesignator_AST;
		if ( functionDesignator_AST!=static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			functionDesignator_AST->getFirstChild() != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = functionDesignator_AST->getFirstChild();
		else
			currentAST.child = functionDesignator_AST;
		currentAST.advanceChildToEnd();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_34);
	}
	returnAST = functionDesignator_AST;
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
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			unsignedConstant_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case CHR:
		{
			constantChr();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			unsignedConstant_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case STRING_LITERAL:
		{
			string();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			unsignedConstant_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case NIL:
		{
			RefPascalAST tmp130_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp130_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp130_AST));
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
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_34);
	}
	returnAST = unsignedConstant_AST;
}

void PascalParser::set() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST set_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case LBRACK:
		{
			RefPascalAST tmp131_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp131_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp131_AST));
			match(LBRACK);
			elementList();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			match(RBRACK);
			set_AST = RefPascalAST(currentAST.root);
#line 485 "pascal.g"
			set_AST->setType(SET);
#line 3042 "PascalParser.cpp"
			set_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case LBRACK2:
		{
			RefPascalAST tmp133_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp133_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp133_AST));
			match(LBRACK2);
			elementList();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			match(RBRACK2);
			set_AST = RefPascalAST(currentAST.root);
#line 486 "pascal.g"
			set_AST->setType(SET);
#line 3058 "PascalParser.cpp"
			set_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_34);
	}
	returnAST = set_AST;
}

void PascalParser::parameterList() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST parameterList_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		actualParameter();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				actualParameter();
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			else {
				goto _loop118;
			}
			
		}
		_loop118:;
		} // ( ... )*
		parameterList_AST = RefPascalAST(currentAST.root);
#line 481 "pascal.g"
		parameterList_AST = RefPascalAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(ARGLIST)))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(parameterList_AST))));
#line 3101 "PascalParser.cpp"
		currentAST.root = parameterList_AST;
		if ( parameterList_AST!=static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			parameterList_AST->getFirstChild() != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = parameterList_AST->getFirstChild();
		else
			currentAST.child = parameterList_AST;
		currentAST.advanceChildToEnd();
		parameterList_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_36);
	}
	returnAST = parameterList_AST;
}

void PascalParser::actualParameter() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST actualParameter_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		expression();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		actualParameter_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_37);
	}
	returnAST = actualParameter_AST;
}

void PascalParser::elementList() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST elementList_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case LPAREN:
		case IDENT:
		case CHR:
		case NUM_INT:
		case NUM_REAL:
		case PLUS:
		case MINUS:
		case STRING_LITERAL:
		case LBRACK:
		case LBRACK2:
		case AT:
		case NOT:
		case NIL:
		{
			element();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			{ // ( ... )*
			for (;;) {
				if ((LA(1) == COMMA)) {
					match(COMMA);
					element();
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
				}
				else {
					goto _loop122;
				}
				
			}
			_loop122:;
			} // ( ... )*
			elementList_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case RBRACK:
		case RBRACK2:
		{
			elementList_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_19);
	}
	returnAST = elementList_AST;
}

void PascalParser::element() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST element_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		expression();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		{
		switch ( LA(1)) {
		case DOTDOT:
		{
			RefPascalAST tmp137_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp137_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp137_AST));
			match(DOTDOT);
			expression();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			break;
		}
		case COMMA:
		case RBRACK:
		case RBRACK2:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		element_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_20);
	}
	returnAST = element_AST;
}

void PascalParser::empty() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST empty_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		empty_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_0);
	}
	returnAST = empty_AST;
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
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			conditionalStatement_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case CASE:
		{
			caseStatement();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_25);
	}
	returnAST = conditionalStatement_AST;
}

void PascalParser::repetetiveStatement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST repetetiveStatement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case WHILE:
		{
			whileStatement();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			repetetiveStatement_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case REPEAT:
		{
			repeatStatement();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			repetetiveStatement_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		case FOR:
		{
			forStatement();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			repetetiveStatement_AST = static_cast<RefPascalAST>(currentAST.root);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_25);
	}
	returnAST = repetetiveStatement_AST;
}

void PascalParser::withStatement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST withStatement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp138_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp138_AST = astFactory->create(LT(1));
		astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp138_AST));
		match(WITH);
		recordVariableList();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		match(DO);
		statement();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		withStatement_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_25);
	}
	returnAST = withStatement_AST;
}

void PascalParser::statements() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST statements_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		statement();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == SEMI)) {
				match(SEMI);
				statement();
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			else {
				goto _loop135;
			}
			
		}
		_loop135:;
		} // ( ... )*
		statements_AST = RefPascalAST(currentAST.root);
#line 533 "pascal.g"
		statements_AST = RefPascalAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(BLOCK)))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(statements_AST))));
#line 3382 "PascalParser.cpp"
		currentAST.root = statements_AST;
		if ( statements_AST!=static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			statements_AST->getFirstChild() != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = statements_AST->getFirstChild();
		else
			currentAST.child = statements_AST;
		currentAST.advanceChildToEnd();
		statements_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_38);
	}
	returnAST = statements_AST;
}

void PascalParser::ifStatement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST ifStatement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp141_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp141_AST = astFactory->create(LT(1));
		astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp141_AST));
		match(IF);
		expression();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		match(THEN);
		statement();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		{
		if ((LA(1) == ELSE) && (_tokenSet_39.member(LA(2)))) {
			match(ELSE);
			statement();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		}
		else if ((_tokenSet_25.member(LA(1))) && (_tokenSet_40.member(LA(2)))) {
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
		ifStatement_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_25);
	}
	returnAST = ifStatement_AST;
}

void PascalParser::caseStatement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST caseStatement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp144_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp144_AST = astFactory->create(LT(1));
		astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp144_AST));
		match(CASE);
		expression();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		match(OF);
		caseListElement();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == SEMI) && (_tokenSet_14.member(LA(2)))) {
				match(SEMI);
				caseListElement();
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			else {
				goto _loop141;
			}
			
		}
		_loop141:;
		} // ( ... )*
		{
		switch ( LA(1)) {
		case SEMI:
		{
			match(SEMI);
			match(ELSE);
			statements();
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
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
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_25);
	}
	returnAST = caseStatement_AST;
}

void PascalParser::caseListElement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST caseListElement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		constList();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		RefPascalAST tmp150_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp150_AST = astFactory->create(LT(1));
		astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp150_AST));
		match(COLON);
		statement();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		caseListElement_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_41);
	}
	returnAST = caseListElement_AST;
}

void PascalParser::whileStatement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST whileStatement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp151_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp151_AST = astFactory->create(LT(1));
		astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp151_AST));
		match(WHILE);
		expression();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		match(DO);
		statement();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		whileStatement_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_25);
	}
	returnAST = whileStatement_AST;
}

void PascalParser::repeatStatement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST repeatStatement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp153_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp153_AST = astFactory->create(LT(1));
		astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp153_AST));
		match(REPEAT);
		statements();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		match(UNTIL);
		expression();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		repeatStatement_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_25);
	}
	returnAST = repeatStatement_AST;
}

void PascalParser::forStatement() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST forStatement_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefPascalAST tmp155_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp155_AST = astFactory->create(LT(1));
		astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp155_AST));
		match(FOR);
		identifier();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		match(ASSIGN);
		forList();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		match(DO);
		statement();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		forStatement_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_25);
	}
	returnAST = forStatement_AST;
}

void PascalParser::forList() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST forList_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		initialValue();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		{
		switch ( LA(1)) {
		case TO:
		{
			RefPascalAST tmp158_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp158_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp158_AST));
			match(TO);
			break;
		}
		case DOWNTO:
		{
			RefPascalAST tmp159_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp159_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp159_AST));
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
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		forList_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_42);
	}
	returnAST = forList_AST;
}

void PascalParser::initialValue() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST initialValue_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		expression();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		initialValue_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_43);
	}
	returnAST = initialValue_AST;
}

void PascalParser::finalValue() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST finalValue_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		expression();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		finalValue_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_42);
	}
	returnAST = finalValue_AST;
}

void PascalParser::recordVariableList() {
	returnAST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefPascalAST recordVariableList_AST = static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		variable();
		astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				variable();
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST));
			}
			else {
				goto _loop155;
			}
			
		}
		_loop155:;
		} // ( ... )*
		recordVariableList_AST = static_cast<RefPascalAST>(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_42);
	}
	returnAST = recordVariableList_AST;
}

RefPascalAST PascalParser::getAST()
{
	return returnAST;
}

void PascalParser::initializeASTFactory( ANTLR_USE_NAMESPACE(antlr)ASTFactory& factory )
{
	factory.setMaxNodeType(108);
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
	"\"interface\"",
	"DOT",
	"\"program\"",
	"LPAREN",
	"RPAREN",
	"SEMI",
	"\"unit\"",
	"IDENT",
	"\"implementation\"",
	"\"uses\"",
	"\"label\"",
	"COMMA",
	"\"const\"",
	"EQUAL",
	"\"chr\"",
	"NUM_INT",
	"NUM_REAL",
	"PLUS",
	"MINUS",
	"STRING_LITERAL",
	"\"type\"",
	"\"function\"",
	"COLON",
	"\"procedure\"",
	"DOTDOT",
	"\"char\"",
	"\"boolean\"",
	"\"integer\"",
	"\"real\"",
	"\"string\"",
	"\"packed\"",
	"LBRACK",
	"RBRACK",
	"\"array\"",
	"\"of\"",
	"LBRACK2",
	"RBRACK2",
	"\"record\"",
	"\"end\"",
	"\"case\"",
	"\"set\"",
	"\"file\"",
	"POINTER",
	"\"var\"",
	"ASSIGN",
	"AT",
	"NOT_EQUAL",
	"LTH",
	"LE",
	"GE",
	"GT",
	"\"in\"",
	"\"or\"",
	"STAR",
	"SLASH",
	"\"div\"",
	"\"mod\"",
	"\"and\"",
	"\"not\"",
	"\"nil\"",
	"\"goto\"",
	"\"begin\"",
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

const unsigned long PascalParser::_tokenSet_0_data_[] = { 2UL, 0UL, 0UL, 0UL };
// EOF 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_0(_tokenSet_0_data_,4);
const unsigned long PascalParser::_tokenSet_1_data_[] = { 3225419776UL, 11269UL, 524290UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "interface" "implementation" "uses" "label" "const" "type" "function" 
// "procedure" "var" "begin" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_1(_tokenSet_1_data_,8);
const unsigned long PascalParser::_tokenSet_2_data_[] = { 142606336UL, 0UL, 0UL, 0UL };
// DOT SEMI 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_2(_tokenSet_2_data_,4);
const unsigned long PascalParser::_tokenSet_3_data_[] = { 243269632UL, 392188298UL, 895549429UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// DOT LPAREN RPAREN SEMI COMMA EQUAL PLUS MINUS COLON DOTDOT LBRACK RBRACK 
// "of" LBRACK2 RBRACK2 "end" POINTER ASSIGN NOT_EQUAL LTH LE GE GT "in" 
// "or" STAR SLASH "div" "mod" "and" "then" "else" "do" "until" "to" "downto" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_3(_tokenSet_3_data_,8);
const unsigned long PascalParser::_tokenSet_4_data_[] = { 201326592UL, 4096UL, 0UL, 0UL };
// RPAREN SEMI COLON 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_4(_tokenSet_4_data_,4);
const unsigned long PascalParser::_tokenSet_5_data_[] = { 3221225472UL, 11269UL, 524290UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "implementation" "uses" "label" "const" "type" "function" "procedure" 
// "var" "begin" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_5(_tokenSet_5_data_,8);
const unsigned long PascalParser::_tokenSet_6_data_[] = { 142606336UL, 268435456UL, 71303168UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// DOT SEMI "end" "else" "until" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_6(_tokenSet_6_data_,8);
const unsigned long PascalParser::_tokenSet_7_data_[] = { 134217728UL, 268439554UL, 71303168UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI COMMA COLON "end" "else" "until" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_7(_tokenSet_7_data_,8);
const unsigned long PascalParser::_tokenSet_8_data_[] = { 201326592UL, 356536714UL, 895549424UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// RPAREN SEMI COMMA EQUAL PLUS MINUS COLON DOTDOT RBRACK "of" RBRACK2 
// "end" NOT_EQUAL LTH LE GE GT "in" "or" STAR SLASH "div" "mod" "and" 
// "then" "else" "do" "until" "to" "downto" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_8(_tokenSet_8_data_,8);
const unsigned long PascalParser::_tokenSet_9_data_[] = { 134217728UL, 0UL, 0UL, 0UL };
// SEMI 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_9(_tokenSet_9_data_,4);
const unsigned long PascalParser::_tokenSet_10_data_[] = { 201326592UL, 339759106UL, 0UL, 0UL };
// RPAREN SEMI COMMA COLON DOTDOT RBRACK RBRACK2 "end" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_10(_tokenSet_10_data_,4);
const unsigned long PascalParser::_tokenSet_11_data_[] = { 536870912UL, 96UL, 0UL, 0UL };
// IDENT NUM_INT NUM_REAL 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_11(_tokenSet_11_data_,4);
const unsigned long PascalParser::_tokenSet_12_data_[] = { 201326592UL, 268435456UL, 0UL, 0UL };
// RPAREN SEMI "end" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_12(_tokenSet_12_data_,4);
const unsigned long PascalParser::_tokenSet_13_data_[] = { 134217728UL, 4096UL, 0UL, 0UL };
// SEMI COLON 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_13(_tokenSet_13_data_,4);
const unsigned long PascalParser::_tokenSet_14_data_[] = { 536870912UL, 1008UL, 0UL, 0UL };
// IDENT "chr" NUM_INT NUM_REAL PLUS MINUS STRING_LITERAL 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_14(_tokenSet_14_data_,4);
const unsigned long PascalParser::_tokenSet_15_data_[] = { 570425344UL, 16480UL, 0UL, 0UL };
// LPAREN IDENT NUM_INT NUM_REAL DOTDOT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_15(_tokenSet_15_data_,4);
const unsigned long PascalParser::_tokenSet_16_data_[] = { 536870912UL, 1015808UL, 0UL, 0UL };
// IDENT "char" "boolean" "integer" "real" "string" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_16(_tokenSet_16_data_,4);
const unsigned long PascalParser::_tokenSet_17_data_[] = { 201326592UL, 339738626UL, 0UL, 0UL };
// RPAREN SEMI COMMA RBRACK RBRACK2 "end" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_17(_tokenSet_17_data_,4);
const unsigned long PascalParser::_tokenSet_18_data_[] = { 201326592UL, 356515842UL, 0UL, 0UL };
// RPAREN SEMI COMMA RBRACK "of" RBRACK2 "end" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_18(_tokenSet_18_data_,4);
const unsigned long PascalParser::_tokenSet_19_data_[] = { 0UL, 71303168UL, 0UL, 0UL };
// RBRACK RBRACK2 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_19(_tokenSet_19_data_,4);
const unsigned long PascalParser::_tokenSet_20_data_[] = { 0UL, 71303170UL, 0UL, 0UL };
// COMMA RBRACK RBRACK2 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_20(_tokenSet_20_data_,4);
const unsigned long PascalParser::_tokenSet_21_data_[] = { 67108864UL, 268435456UL, 0UL, 0UL };
// RPAREN "end" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_21(_tokenSet_21_data_,4);
const unsigned long PascalParser::_tokenSet_22_data_[] = { 0UL, 16777216UL, 0UL, 0UL };
// "of" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_22(_tokenSet_22_data_,4);
const unsigned long PascalParser::_tokenSet_23_data_[] = { 0UL, 4096UL, 0UL, 0UL };
// COLON 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_23(_tokenSet_23_data_,4);
const unsigned long PascalParser::_tokenSet_24_data_[] = { 201326592UL, 0UL, 0UL, 0UL };
// RPAREN SEMI 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_24(_tokenSet_24_data_,4);
const unsigned long PascalParser::_tokenSet_25_data_[] = { 134217728UL, 268435456UL, 71303168UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI "end" "else" "until" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_25(_tokenSet_25_data_,8);
const unsigned long PascalParser::_tokenSet_26_data_[] = { 545259520UL, 35651584UL, 5UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// DOT IDENT LBRACK LBRACK2 POINTER ASSIGN 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_26(_tokenSet_26_data_,8);
const unsigned long PascalParser::_tokenSet_27_data_[] = { 167772160UL, 268435456UL, 71303168UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LPAREN SEMI "end" "else" "until" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_27(_tokenSet_27_data_,8);
const unsigned long PascalParser::_tokenSet_28_data_[] = { 201326592UL, 356532618UL, 895549428UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// RPAREN SEMI COMMA EQUAL PLUS MINUS DOTDOT RBRACK "of" RBRACK2 "end" 
// ASSIGN NOT_EQUAL LTH LE GE GT "in" "or" STAR SLASH "div" "mod" "and" 
// "then" "else" "do" "until" "to" "downto" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_28(_tokenSet_28_data_,8);
const unsigned long PascalParser::_tokenSet_29_data_[] = { 0UL, 8UL, 1008UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EQUAL NOT_EQUAL LTH LE GE GT "in" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_29(_tokenSet_29_data_,8);
const unsigned long PascalParser::_tokenSet_30_data_[] = { 201326592UL, 356532226UL, 895483904UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// RPAREN SEMI COMMA DOTDOT RBRACK "of" RBRACK2 "end" "then" "else" "do" 
// "until" "to" "downto" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_30(_tokenSet_30_data_,8);
const unsigned long PascalParser::_tokenSet_31_data_[] = { 0UL, 384UL, 1024UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// PLUS MINUS "or" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_31(_tokenSet_31_data_,8);
const unsigned long PascalParser::_tokenSet_32_data_[] = { 201326592UL, 356532234UL, 895484912UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// RPAREN SEMI COMMA EQUAL DOTDOT RBRACK "of" RBRACK2 "end" NOT_EQUAL LTH 
// LE GE GT "in" "then" "else" "do" "until" "to" "downto" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_32(_tokenSet_32_data_,8);
const unsigned long PascalParser::_tokenSet_33_data_[] = { 201326592UL, 356532618UL, 895485936UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// RPAREN SEMI COMMA EQUAL PLUS MINUS DOTDOT RBRACK "of" RBRACK2 "end" 
// NOT_EQUAL LTH LE GE GT "in" "or" "then" "else" "do" "until" "to" "downto" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_33(_tokenSet_33_data_,8);
const unsigned long PascalParser::_tokenSet_34_data_[] = { 201326592UL, 356532618UL, 895549424UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// RPAREN SEMI COMMA EQUAL PLUS MINUS DOTDOT RBRACK "of" RBRACK2 "end" 
// NOT_EQUAL LTH LE GE GT "in" "or" STAR SLASH "div" "mod" "and" "then" 
// "else" "do" "until" "to" "downto" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_34(_tokenSet_34_data_,8);
const unsigned long PascalParser::_tokenSet_35_data_[] = { 746586112UL, 392184202UL, 895549425UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// DOT RPAREN SEMI IDENT COMMA EQUAL PLUS MINUS DOTDOT LBRACK RBRACK "of" 
// LBRACK2 RBRACK2 "end" POINTER NOT_EQUAL LTH LE GE GT "in" "or" STAR 
// SLASH "div" "mod" "and" "then" "else" "do" "until" "to" "downto" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_35(_tokenSet_35_data_,8);
const unsigned long PascalParser::_tokenSet_36_data_[] = { 67108864UL, 0UL, 0UL, 0UL };
// RPAREN 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_36(_tokenSet_36_data_,4);
const unsigned long PascalParser::_tokenSet_37_data_[] = { 67108864UL, 2UL, 0UL, 0UL };
// RPAREN COMMA 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_37(_tokenSet_37_data_,4);
const unsigned long PascalParser::_tokenSet_38_data_[] = { 0UL, 268435456UL, 67108864UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "end" "until" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_38(_tokenSet_38_data_,8);
const unsigned long PascalParser::_tokenSet_39_data_[] = { 671088640UL, 805306400UL, 1323040776UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI IDENT NUM_INT "end" "case" AT "goto" "begin" "if" "else" "while" 
// "repeat" "until" "for" "with" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_39(_tokenSet_39_data_,8);
const unsigned long PascalParser::_tokenSet_40_data_[] = { 713031680UL, 840958960UL, 1323237384UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// DOT LPAREN SEMI IDENT "chr" NUM_INT NUM_REAL PLUS MINUS STRING_LITERAL 
// LBRACK LBRACK2 "end" "case" AT "not" "nil" "goto" "begin" "if" "else" 
// "while" "repeat" "until" "for" "with" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_40(_tokenSet_40_data_,8);
const unsigned long PascalParser::_tokenSet_41_data_[] = { 134217728UL, 268435456UL, 0UL, 0UL };
// SEMI "end" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_41(_tokenSet_41_data_,4);
const unsigned long PascalParser::_tokenSet_42_data_[] = { 0UL, 0UL, 16777216UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "do" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_42(_tokenSet_42_data_,8);
const unsigned long PascalParser::_tokenSet_43_data_[] = { 0UL, 0UL, 805306368UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "to" "downto" 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalParser::_tokenSet_43(_tokenSet_43_data_,8);


