/* $ANTLR 2.7.7 (20061129): "java.g" -> "JavaRecognizer.cpp"$ */
#include "JavaRecognizer.hpp"
#include <antlr/NoViableAltException.hpp>
#include <antlr/SemanticException.hpp>
#include <antlr/ASTFactory.hpp>
#line 1 "java.g"
#line 8 "JavaRecognizer.cpp"
JavaRecognizer::JavaRecognizer(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf, int k)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(tokenBuf,k)
{
}

JavaRecognizer::JavaRecognizer(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(tokenBuf,2)
{
}

JavaRecognizer::JavaRecognizer(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer, int k)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(lexer,k)
{
}

JavaRecognizer::JavaRecognizer(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(lexer,2)
{
}

JavaRecognizer::JavaRecognizer(const ANTLR_USE_NAMESPACE(antlr)ParserSharedInputState& state)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(state,2)
{
}

void JavaRecognizer::compilationUnit() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST compilationUnit_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LITERAL_package:
		{
			packageDefinition();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case ANTLR_USE_NAMESPACE(antlr)Token::EOF_TYPE:
		case FINAL:
		case ABSTRACT:
		case STRICTFP:
		case SEMI:
		case LITERAL_import:
		case LITERAL_private:
		case LITERAL_public:
		case LITERAL_protected:
		case LITERAL_static:
		case LITERAL_transient:
		case LITERAL_native:
		case LITERAL_threadsafe:
		case LITERAL_synchronized:
		case LITERAL_volatile:
		case LITERAL_class:
		case LITERAL_interface:
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
			if ((LA(1) == LITERAL_import)) {
				importDefinition();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop4;
			}
			
		}
		_loop4:;
		} // ( ... )*
		{ // ( ... )*
		for (;;) {
			if ((_tokenSet_0.member(LA(1)))) {
				typeDefinition();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop6;
			}
			
		}
		_loop6:;
		} // ( ... )*
		match(ANTLR_USE_NAMESPACE(antlr)Token::EOF_TYPE);
		compilationUnit_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_1);
		} else {
			throw;
		}
	}
	returnAST = compilationUnit_AST;
}

void JavaRecognizer::packageDefinition() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST packageDefinition_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  p = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST p_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		p = LT(1);
		if ( inputState->guessing == 0 ) {
			p_AST = astFactory->create(p);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(p_AST));
		}
		match(LITERAL_package);
		if ( inputState->guessing==0 ) {
#line 196 "java.g"
			p_AST->setType(PACKAGE_DEF);
#line 137 "JavaRecognizer.cpp"
		}
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(SEMI);
		packageDefinition_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_2);
		} else {
			throw;
		}
	}
	returnAST = packageDefinition_AST;
}

void JavaRecognizer::importDefinition() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST importDefinition_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  i = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST i_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		i = LT(1);
		if ( inputState->guessing == 0 ) {
			i_AST = astFactory->create(i);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(i_AST));
		}
		match(LITERAL_import);
		if ( inputState->guessing==0 ) {
#line 202 "java.g"
			i_AST->setType(IMPORT);
#line 174 "JavaRecognizer.cpp"
		}
		identifierStar();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(SEMI);
		importDefinition_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_2);
		} else {
			throw;
		}
	}
	returnAST = importDefinition_AST;
}

void JavaRecognizer::typeDefinition() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST typeDefinition_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST m_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case FINAL:
		case ABSTRACT:
		case STRICTFP:
		case LITERAL_private:
		case LITERAL_public:
		case LITERAL_protected:
		case LITERAL_static:
		case LITERAL_transient:
		case LITERAL_native:
		case LITERAL_threadsafe:
		case LITERAL_synchronized:
		case LITERAL_volatile:
		case LITERAL_class:
		case LITERAL_interface:
		{
			modifiers();
			if (inputState->guessing==0) {
				m_AST = returnAST;
			}
			{
			switch ( LA(1)) {
			case LITERAL_class:
			{
				classDefinition(m_AST);
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case LITERAL_interface:
			{
				interfaceDefinition(m_AST);
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
			typeDefinition_AST = RefJavaAST(currentAST.root);
			break;
		}
		case SEMI:
		{
			match(SEMI);
			typeDefinition_AST = RefJavaAST(currentAST.root);
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
			recover(ex,_tokenSet_3);
		} else {
			throw;
		}
	}
	returnAST = typeDefinition_AST;
}

void JavaRecognizer::identifier() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST identifier_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefJavaAST tmp5_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp5_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp5_AST));
		}
		match(IDENT);
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == DOT)) {
				RefJavaAST tmp6_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp6_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp6_AST));
				}
				match(DOT);
				RefJavaAST tmp7_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp7_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp7_AST));
				}
				match(IDENT);
			}
			else {
				goto _loop23;
			}
			
		}
		_loop23:;
		} // ( ... )*
		identifier_AST = RefJavaAST(currentAST.root);
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

void JavaRecognizer::identifierStar() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST identifierStar_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefJavaAST tmp8_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp8_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp8_AST));
		}
		match(IDENT);
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == DOT) && (LA(2) == IDENT)) {
				RefJavaAST tmp9_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp9_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp9_AST));
				}
				match(DOT);
				RefJavaAST tmp10_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp10_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp10_AST));
				}
				match(IDENT);
			}
			else {
				goto _loop26;
			}
			
		}
		_loop26:;
		} // ( ... )*
		{
		switch ( LA(1)) {
		case DOT:
		{
			RefJavaAST tmp11_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp11_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp11_AST));
			}
			match(DOT);
			RefJavaAST tmp12_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp12_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp12_AST));
			}
			match(STAR);
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
		identifierStar_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_5);
		} else {
			throw;
		}
	}
	returnAST = identifierStar_AST;
}

void JavaRecognizer::modifiers() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST modifiers_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{ // ( ... )*
		for (;;) {
			if ((_tokenSet_6.member(LA(1)))) {
				modifier();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop30;
			}
			
		}
		_loop30:;
		} // ( ... )*
		if ( inputState->guessing==0 ) {
			modifiers_AST = RefJavaAST(currentAST.root);
#line 290 "java.g"
			modifiers_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(MODIFIERS,"MODIFIERS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(modifiers_AST))));
#line 420 "JavaRecognizer.cpp"
			currentAST.root = modifiers_AST;
			if ( modifiers_AST!=RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				modifiers_AST->getFirstChild() != RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = modifiers_AST->getFirstChild();
			else
				currentAST.child = modifiers_AST;
			currentAST.advanceChildToEnd();
		}
		modifiers_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_7);
		} else {
			throw;
		}
	}
	returnAST = modifiers_AST;
}

void JavaRecognizer::classDefinition(
	RefJavaAST modifiers
) {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST classDefinition_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST sc_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST ic_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST cb_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(LITERAL_class);
		RefJavaAST tmp14_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp14_AST = astFactory->create(LT(1));
		}
		match(IDENT);
		superClassClause();
		if (inputState->guessing==0) {
			sc_AST = returnAST;
		}
		implementsClause();
		if (inputState->guessing==0) {
			ic_AST = returnAST;
		}
		classBlock();
		if (inputState->guessing==0) {
			cb_AST = returnAST;
		}
		if ( inputState->guessing==0 ) {
			classDefinition_AST = RefJavaAST(currentAST.root);
#line 319 "java.g"
			classDefinition_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(6))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(CLASS_DEF,"CLASS_DEF")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(modifiers))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(tmp14_AST))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(sc_AST))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(ic_AST))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(cb_AST))));
#line 475 "JavaRecognizer.cpp"
			currentAST.root = classDefinition_AST;
			if ( classDefinition_AST!=RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				classDefinition_AST->getFirstChild() != RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = classDefinition_AST->getFirstChild();
			else
				currentAST.child = classDefinition_AST;
			currentAST.advanceChildToEnd();
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_8);
		} else {
			throw;
		}
	}
	returnAST = classDefinition_AST;
}

void JavaRecognizer::interfaceDefinition(
	RefJavaAST modifiers
) {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST interfaceDefinition_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST ie_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST cb_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(LITERAL_interface);
		RefJavaAST tmp16_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp16_AST = astFactory->create(LT(1));
		}
		match(IDENT);
		interfaceExtends();
		if (inputState->guessing==0) {
			ie_AST = returnAST;
		}
		classBlock();
		if (inputState->guessing==0) {
			cb_AST = returnAST;
		}
		if ( inputState->guessing==0 ) {
			interfaceDefinition_AST = RefJavaAST(currentAST.root);
#line 335 "java.g"
			interfaceDefinition_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(5))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(INTERFACE_DEF,"INTERFACE_DEF")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(modifiers))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(tmp16_AST))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(ie_AST))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(cb_AST))));
#line 524 "JavaRecognizer.cpp"
			currentAST.root = interfaceDefinition_AST;
			if ( interfaceDefinition_AST!=RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				interfaceDefinition_AST->getFirstChild() != RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = interfaceDefinition_AST->getFirstChild();
			else
				currentAST.child = interfaceDefinition_AST;
			currentAST.advanceChildToEnd();
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
	returnAST = interfaceDefinition_AST;
}

/** A declaration is the creation of a reference or primitive-type variable
 *  Create a separate Type/Var tree for each var in the var list.
 */
void JavaRecognizer::declaration() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST declaration_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST m_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST t_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST v_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		modifiers();
		if (inputState->guessing==0) {
			m_AST = returnAST;
		}
		typeSpec(false);
		if (inputState->guessing==0) {
			t_AST = returnAST;
		}
		variableDefinitions(m_AST,t_AST);
		if (inputState->guessing==0) {
			v_AST = returnAST;
		}
		if ( inputState->guessing==0 ) {
			declaration_AST = RefJavaAST(currentAST.root);
#line 220 "java.g"
			declaration_AST = v_AST;
#line 573 "JavaRecognizer.cpp"
			currentAST.root = declaration_AST;
			if ( declaration_AST!=RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				declaration_AST->getFirstChild() != RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = declaration_AST->getFirstChild();
			else
				currentAST.child = declaration_AST;
			currentAST.advanceChildToEnd();
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_5);
		} else {
			throw;
		}
	}
	returnAST = declaration_AST;
}

void JavaRecognizer::typeSpec(
	bool addImagNode
) {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST typeSpec_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case IDENT:
		{
			classTypeSpec(addImagNode);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			typeSpec_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_void:
		case LITERAL_boolean:
		case LITERAL_byte:
		case LITERAL_char:
		case LITERAL_short:
		case LITERAL_int:
		case LITERAL_float:
		case LITERAL_long:
		case LITERAL_double:
		{
			builtInTypeSpec(addImagNode);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			typeSpec_AST = RefJavaAST(currentAST.root);
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
	returnAST = typeSpec_AST;
}

void JavaRecognizer::variableDefinitions(
	RefJavaAST mods, RefJavaAST t
) {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST variableDefinitions_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		variableDeclarator((RefJavaAST)getASTFactory()->dupTree((antlr::RefAST)mods),
				   (RefJavaAST)getASTFactory()->dupTree((antlr::RefAST)t));
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				variableDeclarator((RefJavaAST)getASTFactory()->dupTree((antlr::RefAST)mods),
					   (RefJavaAST)getASTFactory()->dupTree((antlr::RefAST)t));
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop59;
			}
			
		}
		_loop59:;
		} // ( ... )*
		variableDefinitions_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_5);
		} else {
			throw;
		}
	}
	returnAST = variableDefinitions_AST;
}

void JavaRecognizer::classTypeSpec(
	bool addImagNode
) {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST classTypeSpec_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  lb = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lb_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == LBRACK)) {
				lb = LT(1);
				if ( inputState->guessing == 0 ) {
					lb_AST = astFactory->create(lb);
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(lb_AST));
				}
				match(LBRACK);
				if ( inputState->guessing==0 ) {
#line 233 "java.g"
					lb_AST->setType(ARRAY_DECLARATOR);
#line 715 "JavaRecognizer.cpp"
				}
				match(RBRACK);
			}
			else {
				goto _loop15;
			}
			
		}
		_loop15:;
		} // ( ... )*
		if ( inputState->guessing==0 ) {
			classTypeSpec_AST = RefJavaAST(currentAST.root);
#line 234 "java.g"
			
						if ( addImagNode ) {
							classTypeSpec_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(TYPE,"TYPE")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(classTypeSpec_AST))));
						}
					
#line 734 "JavaRecognizer.cpp"
			currentAST.root = classTypeSpec_AST;
			if ( classTypeSpec_AST!=RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				classTypeSpec_AST->getFirstChild() != RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = classTypeSpec_AST->getFirstChild();
			else
				currentAST.child = classTypeSpec_AST;
			currentAST.advanceChildToEnd();
		}
		classTypeSpec_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_10);
		} else {
			throw;
		}
	}
	returnAST = classTypeSpec_AST;
}

void JavaRecognizer::builtInTypeSpec(
	bool addImagNode
) {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST builtInTypeSpec_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  lb = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lb_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		builtInType();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == LBRACK)) {
				lb = LT(1);
				if ( inputState->guessing == 0 ) {
					lb_AST = astFactory->create(lb);
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(lb_AST));
				}
				match(LBRACK);
				if ( inputState->guessing==0 ) {
#line 244 "java.g"
					lb_AST->setType(ARRAY_DECLARATOR);
#line 782 "JavaRecognizer.cpp"
				}
				match(RBRACK);
			}
			else {
				goto _loop18;
			}
			
		}
		_loop18:;
		} // ( ... )*
		if ( inputState->guessing==0 ) {
			builtInTypeSpec_AST = RefJavaAST(currentAST.root);
#line 245 "java.g"
			
						if ( addImagNode ) {
							builtInTypeSpec_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(TYPE,"TYPE")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(builtInTypeSpec_AST))));
						}
					
#line 801 "JavaRecognizer.cpp"
			currentAST.root = builtInTypeSpec_AST;
			if ( builtInTypeSpec_AST!=RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				builtInTypeSpec_AST->getFirstChild() != RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = builtInTypeSpec_AST->getFirstChild();
			else
				currentAST.child = builtInTypeSpec_AST;
			currentAST.advanceChildToEnd();
		}
		builtInTypeSpec_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_10);
		} else {
			throw;
		}
	}
	returnAST = builtInTypeSpec_AST;
}

void JavaRecognizer::builtInType() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST builtInType_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case LITERAL_void:
		{
			RefJavaAST tmp20_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp20_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp20_AST));
			}
			match(LITERAL_void);
			builtInType_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_boolean:
		{
			RefJavaAST tmp21_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp21_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp21_AST));
			}
			match(LITERAL_boolean);
			builtInType_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_byte:
		{
			RefJavaAST tmp22_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp22_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp22_AST));
			}
			match(LITERAL_byte);
			builtInType_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_char:
		{
			RefJavaAST tmp23_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp23_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp23_AST));
			}
			match(LITERAL_char);
			builtInType_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_short:
		{
			RefJavaAST tmp24_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp24_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp24_AST));
			}
			match(LITERAL_short);
			builtInType_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_int:
		{
			RefJavaAST tmp25_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp25_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp25_AST));
			}
			match(LITERAL_int);
			builtInType_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_float:
		{
			RefJavaAST tmp26_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp26_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp26_AST));
			}
			match(LITERAL_float);
			builtInType_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_long:
		{
			RefJavaAST tmp27_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp27_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp27_AST));
			}
			match(LITERAL_long);
			builtInType_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_double:
		{
			RefJavaAST tmp28_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp28_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp28_AST));
			}
			match(LITERAL_double);
			builtInType_AST = RefJavaAST(currentAST.root);
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
			recover(ex,_tokenSet_11);
		} else {
			throw;
		}
	}
	returnAST = builtInType_AST;
}

void JavaRecognizer::type() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST type_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case IDENT:
		{
			identifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			type_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_void:
		case LITERAL_boolean:
		case LITERAL_byte:
		case LITERAL_char:
		case LITERAL_short:
		case LITERAL_int:
		case LITERAL_float:
		case LITERAL_long:
		case LITERAL_double:
		{
			builtInType();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			type_AST = RefJavaAST(currentAST.root);
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
			recover(ex,_tokenSet_12);
		} else {
			throw;
		}
	}
	returnAST = type_AST;
}

void JavaRecognizer::modifier() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST modifier_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case LITERAL_private:
		{
			RefJavaAST tmp29_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp29_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp29_AST));
			}
			match(LITERAL_private);
			modifier_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_public:
		{
			RefJavaAST tmp30_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp30_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp30_AST));
			}
			match(LITERAL_public);
			modifier_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_protected:
		{
			RefJavaAST tmp31_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp31_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp31_AST));
			}
			match(LITERAL_protected);
			modifier_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_static:
		{
			RefJavaAST tmp32_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp32_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp32_AST));
			}
			match(LITERAL_static);
			modifier_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_transient:
		{
			RefJavaAST tmp33_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp33_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp33_AST));
			}
			match(LITERAL_transient);
			modifier_AST = RefJavaAST(currentAST.root);
			break;
		}
		case FINAL:
		{
			RefJavaAST tmp34_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp34_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp34_AST));
			}
			match(FINAL);
			modifier_AST = RefJavaAST(currentAST.root);
			break;
		}
		case ABSTRACT:
		{
			RefJavaAST tmp35_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp35_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp35_AST));
			}
			match(ABSTRACT);
			modifier_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_native:
		{
			RefJavaAST tmp36_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp36_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp36_AST));
			}
			match(LITERAL_native);
			modifier_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_threadsafe:
		{
			RefJavaAST tmp37_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp37_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp37_AST));
			}
			match(LITERAL_threadsafe);
			modifier_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_synchronized:
		{
			RefJavaAST tmp38_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp38_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp38_AST));
			}
			match(LITERAL_synchronized);
			modifier_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_volatile:
		{
			RefJavaAST tmp39_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp39_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp39_AST));
			}
			match(LITERAL_volatile);
			modifier_AST = RefJavaAST(currentAST.root);
			break;
		}
		case STRICTFP:
		{
			RefJavaAST tmp40_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp40_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp40_AST));
			}
			match(STRICTFP);
			modifier_AST = RefJavaAST(currentAST.root);
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
			recover(ex,_tokenSet_13);
		} else {
			throw;
		}
	}
	returnAST = modifier_AST;
}

void JavaRecognizer::superClassClause() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST superClassClause_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST id_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LITERAL_extends:
		{
			match(LITERAL_extends);
			identifier();
			if (inputState->guessing==0) {
				id_AST = returnAST;
			}
			break;
		}
		case LCURLY:
		case LITERAL_implements:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		if ( inputState->guessing==0 ) {
			superClassClause_AST = RefJavaAST(currentAST.root);
#line 325 "java.g"
			superClassClause_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(EXTENDS_CLAUSE,"EXTENDS_CLAUSE")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(id_AST))));
#line 1185 "JavaRecognizer.cpp"
			currentAST.root = superClassClause_AST;
			if ( superClassClause_AST!=RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				superClassClause_AST->getFirstChild() != RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = superClassClause_AST->getFirstChild();
			else
				currentAST.child = superClassClause_AST;
			currentAST.advanceChildToEnd();
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_14);
		} else {
			throw;
		}
	}
	returnAST = superClassClause_AST;
}

void JavaRecognizer::implementsClause() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST implementsClause_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  i = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST i_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LITERAL_implements:
		{
			i = LT(1);
			if ( inputState->guessing == 0 ) {
				i_AST = astFactory->create(i);
			}
			match(LITERAL_implements);
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
					goto _loop46;
				}
				
			}
			_loop46:;
			} // ( ... )*
			break;
		}
		case LCURLY:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		if ( inputState->guessing==0 ) {
			implementsClause_AST = RefJavaAST(currentAST.root);
#line 363 "java.g"
			implementsClause_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(IMPLEMENTS_CLAUSE,"IMPLEMENTS_CLAUSE")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(implementsClause_AST))));
#line 1259 "JavaRecognizer.cpp"
			currentAST.root = implementsClause_AST;
			if ( implementsClause_AST!=RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				implementsClause_AST->getFirstChild() != RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = implementsClause_AST->getFirstChild();
			else
				currentAST.child = implementsClause_AST;
			currentAST.advanceChildToEnd();
		}
		implementsClause_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_15);
		} else {
			throw;
		}
	}
	returnAST = implementsClause_AST;
}

void JavaRecognizer::classBlock() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST classBlock_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(LCURLY);
		{ // ( ... )*
		for (;;) {
			switch ( LA(1)) {
			case FINAL:
			case ABSTRACT:
			case STRICTFP:
			case LITERAL_void:
			case LITERAL_boolean:
			case LITERAL_byte:
			case LITERAL_char:
			case LITERAL_short:
			case LITERAL_int:
			case LITERAL_float:
			case LITERAL_long:
			case LITERAL_double:
			case IDENT:
			case LITERAL_private:
			case LITERAL_public:
			case LITERAL_protected:
			case LITERAL_static:
			case LITERAL_transient:
			case LITERAL_native:
			case LITERAL_threadsafe:
			case LITERAL_synchronized:
			case LITERAL_volatile:
			case LITERAL_class:
			case LITERAL_interface:
			case LCURLY:
			{
				field();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case SEMI:
			{
				match(SEMI);
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
		match(RCURLY);
		if ( inputState->guessing==0 ) {
			classBlock_AST = RefJavaAST(currentAST.root);
#line 345 "java.g"
			classBlock_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(OBJBLOCK,"OBJBLOCK")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(classBlock_AST))));
#line 1341 "JavaRecognizer.cpp"
			currentAST.root = classBlock_AST;
			if ( classBlock_AST!=RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				classBlock_AST->getFirstChild() != RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = classBlock_AST->getFirstChild();
			else
				currentAST.child = classBlock_AST;
			currentAST.advanceChildToEnd();
		}
		classBlock_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_16);
		} else {
			throw;
		}
	}
	returnAST = classBlock_AST;
}

void JavaRecognizer::interfaceExtends() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST interfaceExtends_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  e = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST e_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LITERAL_extends:
		{
			e = LT(1);
			if ( inputState->guessing == 0 ) {
				e_AST = astFactory->create(e);
			}
			match(LITERAL_extends);
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
					goto _loop42;
				}
				
			}
			_loop42:;
			} // ( ... )*
			break;
		}
		case LCURLY:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		if ( inputState->guessing==0 ) {
			interfaceExtends_AST = RefJavaAST(currentAST.root);
#line 354 "java.g"
			interfaceExtends_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(EXTENDS_CLAUSE,"EXTENDS_CLAUSE")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(interfaceExtends_AST))));
#line 1416 "JavaRecognizer.cpp"
			currentAST.root = interfaceExtends_AST;
			if ( interfaceExtends_AST!=RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				interfaceExtends_AST->getFirstChild() != RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = interfaceExtends_AST->getFirstChild();
			else
				currentAST.child = interfaceExtends_AST;
			currentAST.advanceChildToEnd();
		}
		interfaceExtends_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_15);
		} else {
			throw;
		}
	}
	returnAST = interfaceExtends_AST;
}

void JavaRecognizer::field() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST field_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST mods_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST h_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST s_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST cd_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST id_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST t_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST param_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST rt_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST tc_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST s2_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST v_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST s3_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST s4_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		if ((_tokenSet_13.member(LA(1))) && (_tokenSet_17.member(LA(2)))) {
			modifiers();
			if (inputState->guessing==0) {
				mods_AST = returnAST;
			}
			{
			switch ( LA(1)) {
			case LITERAL_class:
			{
				classDefinition(mods_AST);
				if (inputState->guessing==0) {
					cd_AST = returnAST;
				}
				if ( inputState->guessing==0 ) {
					field_AST = RefJavaAST(currentAST.root);
#line 378 "java.g"
					field_AST = cd_AST;
#line 1474 "JavaRecognizer.cpp"
					currentAST.root = field_AST;
					if ( field_AST!=RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
						field_AST->getFirstChild() != RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
						  currentAST.child = field_AST->getFirstChild();
					else
						currentAST.child = field_AST;
					currentAST.advanceChildToEnd();
				}
				break;
			}
			case LITERAL_interface:
			{
				interfaceDefinition(mods_AST);
				if (inputState->guessing==0) {
					id_AST = returnAST;
				}
				if ( inputState->guessing==0 ) {
					field_AST = RefJavaAST(currentAST.root);
#line 381 "java.g"
					field_AST = id_AST;
#line 1495 "JavaRecognizer.cpp"
					currentAST.root = field_AST;
					if ( field_AST!=RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
						field_AST->getFirstChild() != RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
						  currentAST.child = field_AST->getFirstChild();
					else
						currentAST.child = field_AST;
					currentAST.advanceChildToEnd();
				}
				break;
			}
			default:
				if ((LA(1) == IDENT) && (LA(2) == LPAREN)) {
					ctorHead();
					if (inputState->guessing==0) {
						h_AST = returnAST;
					}
					constructorBody();
					if (inputState->guessing==0) {
						s_AST = returnAST;
					}
					if ( inputState->guessing==0 ) {
						field_AST = RefJavaAST(currentAST.root);
#line 375 "java.g"
						field_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(4))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(CTOR_DEF,"CTOR_DEF")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(mods_AST))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(h_AST))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(s_AST))));
#line 1520 "JavaRecognizer.cpp"
						currentAST.root = field_AST;
						if ( field_AST!=RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
							field_AST->getFirstChild() != RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
							  currentAST.child = field_AST->getFirstChild();
						else
							currentAST.child = field_AST;
						currentAST.advanceChildToEnd();
					}
				}
				else if (((LA(1) >= LITERAL_void && LA(1) <= IDENT)) && (_tokenSet_18.member(LA(2)))) {
					typeSpec(false);
					if (inputState->guessing==0) {
						t_AST = returnAST;
					}
					{
					if ((LA(1) == IDENT) && (LA(2) == LPAREN)) {
						RefJavaAST tmp47_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
						if ( inputState->guessing == 0 ) {
							tmp47_AST = astFactory->create(LT(1));
						}
						match(IDENT);
						match(LPAREN);
						parameterDeclarationList();
						if (inputState->guessing==0) {
							param_AST = returnAST;
						}
						match(RPAREN);
						declaratorBrackets(t_AST);
						if (inputState->guessing==0) {
							rt_AST = returnAST;
						}
						{
						switch ( LA(1)) {
						case LITERAL_throws:
						{
							throwsClause();
							if (inputState->guessing==0) {
								tc_AST = returnAST;
							}
							break;
						}
						case SEMI:
						case LCURLY:
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
						case LCURLY:
						{
							compoundStatement();
							if (inputState->guessing==0) {
								s2_AST = returnAST;
							}
							break;
						}
						case SEMI:
						{
							RefJavaAST tmp50_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
							if ( inputState->guessing == 0 ) {
								tmp50_AST = astFactory->create(LT(1));
							}
							match(SEMI);
							break;
						}
						default:
						{
							throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
						}
						}
						}
						if ( inputState->guessing==0 ) {
							field_AST = RefJavaAST(currentAST.root);
#line 396 "java.g"
							field_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(7))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(METHOD_DEF,"METHOD_DEF")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(mods_AST))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(TYPE,"TYPE")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(rt_AST))))))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(tmp47_AST))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(param_AST))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(tc_AST))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(s2_AST))));
#line 1602 "JavaRecognizer.cpp"
							currentAST.root = field_AST;
							if ( field_AST!=RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
								field_AST->getFirstChild() != RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
								  currentAST.child = field_AST->getFirstChild();
							else
								currentAST.child = field_AST;
							currentAST.advanceChildToEnd();
						}
					}
					else if ((LA(1) == IDENT) && (_tokenSet_19.member(LA(2)))) {
						variableDefinitions(mods_AST,t_AST);
						if (inputState->guessing==0) {
							v_AST = returnAST;
						}
						RefJavaAST tmp51_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
						if ( inputState->guessing == 0 ) {
							tmp51_AST = astFactory->create(LT(1));
						}
						match(SEMI);
						if ( inputState->guessing==0 ) {
							field_AST = RefJavaAST(currentAST.root);
#line 405 "java.g"
							field_AST = v_AST;
#line 1626 "JavaRecognizer.cpp"
							currentAST.root = field_AST;
							if ( field_AST!=RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
								field_AST->getFirstChild() != RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
								  currentAST.child = field_AST->getFirstChild();
							else
								currentAST.child = field_AST;
							currentAST.advanceChildToEnd();
						}
					}
					else {
						throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
					}
					
					}
				}
			else {
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
		}
		else if ((LA(1) == LITERAL_static) && (LA(2) == LCURLY)) {
			match(LITERAL_static);
			compoundStatement();
			if (inputState->guessing==0) {
				s3_AST = returnAST;
			}
			if ( inputState->guessing==0 ) {
				field_AST = RefJavaAST(currentAST.root);
#line 411 "java.g"
				field_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(STATIC_INIT,"STATIC_INIT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(s3_AST))));
#line 1658 "JavaRecognizer.cpp"
				currentAST.root = field_AST;
				if ( field_AST!=RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
					field_AST->getFirstChild() != RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
					  currentAST.child = field_AST->getFirstChild();
				else
					currentAST.child = field_AST;
				currentAST.advanceChildToEnd();
			}
		}
		else if ((LA(1) == LCURLY)) {
			compoundStatement();
			if (inputState->guessing==0) {
				s4_AST = returnAST;
			}
			if ( inputState->guessing==0 ) {
				field_AST = RefJavaAST(currentAST.root);
#line 415 "java.g"
				field_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(INSTANCE_INIT,"INSTANCE_INIT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(s4_AST))));
#line 1677 "JavaRecognizer.cpp"
				currentAST.root = field_AST;
				if ( field_AST!=RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
					field_AST->getFirstChild() != RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
					  currentAST.child = field_AST->getFirstChild();
				else
					currentAST.child = field_AST;
				currentAST.advanceChildToEnd();
			}
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_20);
		} else {
			throw;
		}
	}
	returnAST = field_AST;
}

void JavaRecognizer::ctorHead() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST ctorHead_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefJavaAST tmp53_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp53_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp53_AST));
		}
		match(IDENT);
		match(LPAREN);
		parameterDeclarationList();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(RPAREN);
		{
		switch ( LA(1)) {
		case LITERAL_throws:
		{
			throwsClause();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case LCURLY:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		ctorHead_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_15);
		} else {
			throw;
		}
	}
	returnAST = ctorHead_AST;
}

void JavaRecognizer::constructorBody() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST constructorBody_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  lc = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lc_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		lc = LT(1);
		if ( inputState->guessing == 0 ) {
			lc_AST = astFactory->create(lc);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(lc_AST));
		}
		match(LCURLY);
		if ( inputState->guessing==0 ) {
#line 419 "java.g"
			lc_AST->setType(SLIST);
#line 1771 "JavaRecognizer.cpp"
		}
		{
		if ((LA(1) == LITERAL_this || LA(1) == LITERAL_super) && (LA(2) == LPAREN)) {
			explicitConstructorInvocation();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
		}
		else if ((_tokenSet_21.member(LA(1))) && (_tokenSet_22.member(LA(2)))) {
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
		{ // ( ... )*
		for (;;) {
			if ((_tokenSet_23.member(LA(1)))) {
				statement();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop55;
			}
			
		}
		_loop55:;
		} // ( ... )*
		match(RCURLY);
		constructorBody_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_20);
		} else {
			throw;
		}
	}
	returnAST = constructorBody_AST;
}

void JavaRecognizer::parameterDeclarationList() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST parameterDeclarationList_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case FINAL:
		case LITERAL_void:
		case LITERAL_boolean:
		case LITERAL_byte:
		case LITERAL_char:
		case LITERAL_short:
		case LITERAL_int:
		case LITERAL_float:
		case LITERAL_long:
		case LITERAL_double:
		case IDENT:
		{
			parameterDeclaration();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{ // ( ... )*
			for (;;) {
				if ((LA(1) == COMMA)) {
					match(COMMA);
					parameterDeclaration();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
				}
				else {
					goto _loop80;
				}
				
			}
			_loop80:;
			} // ( ... )*
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
		if ( inputState->guessing==0 ) {
			parameterDeclarationList_AST = RefJavaAST(currentAST.root);
#line 508 "java.g"
			parameterDeclarationList_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(PARAMETERS,"PARAMETERS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(parameterDeclarationList_AST))));
#line 1872 "JavaRecognizer.cpp"
			currentAST.root = parameterDeclarationList_AST;
			if ( parameterDeclarationList_AST!=RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				parameterDeclarationList_AST->getFirstChild() != RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = parameterDeclarationList_AST->getFirstChild();
			else
				currentAST.child = parameterDeclarationList_AST;
			currentAST.advanceChildToEnd();
		}
		parameterDeclarationList_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_24);
		} else {
			throw;
		}
	}
	returnAST = parameterDeclarationList_AST;
}

void JavaRecognizer::declaratorBrackets(
	RefJavaAST typ
) {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST declaratorBrackets_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  lb = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lb_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		if ( inputState->guessing==0 ) {
			declaratorBrackets_AST = RefJavaAST(currentAST.root);
#line 452 "java.g"
			declaratorBrackets_AST=typ;
#line 1908 "JavaRecognizer.cpp"
			currentAST.root = declaratorBrackets_AST;
			if ( declaratorBrackets_AST!=RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				declaratorBrackets_AST->getFirstChild() != RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = declaratorBrackets_AST->getFirstChild();
			else
				currentAST.child = declaratorBrackets_AST;
			currentAST.advanceChildToEnd();
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == LBRACK)) {
				lb = LT(1);
				if ( inputState->guessing == 0 ) {
					lb_AST = astFactory->create(lb);
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(lb_AST));
				}
				match(LBRACK);
				if ( inputState->guessing==0 ) {
#line 453 "java.g"
					lb_AST->setType(ARRAY_DECLARATOR);
#line 1929 "JavaRecognizer.cpp"
				}
				match(RBRACK);
			}
			else {
				goto _loop63;
			}
			
		}
		_loop63:;
		} // ( ... )*
		declaratorBrackets_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_25);
		} else {
			throw;
		}
	}
	returnAST = declaratorBrackets_AST;
}

void JavaRecognizer::throwsClause() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST throwsClause_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefJavaAST tmp59_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp59_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp59_AST));
		}
		match(LITERAL_throws);
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
				goto _loop76;
			}
			
		}
		_loop76:;
		} // ( ... )*
		throwsClause_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_26);
		} else {
			throw;
		}
	}
	returnAST = throwsClause_AST;
}

void JavaRecognizer::compoundStatement() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST compoundStatement_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  lc = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lc_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		lc = LT(1);
		if ( inputState->guessing == 0 ) {
			lc_AST = astFactory->create(lc);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(lc_AST));
		}
		match(LCURLY);
		if ( inputState->guessing==0 ) {
#line 535 "java.g"
			lc_AST->setType(SLIST);
#line 2015 "JavaRecognizer.cpp"
		}
		{ // ( ... )*
		for (;;) {
			if ((_tokenSet_23.member(LA(1)))) {
				statement();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop86;
			}
			
		}
		_loop86:;
		} // ( ... )*
		match(RCURLY);
		compoundStatement_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_27);
		} else {
			throw;
		}
	}
	returnAST = compoundStatement_AST;
}

/** Catch obvious constructor calls, but not the expr.super(...) calls */
void JavaRecognizer::explicitConstructorInvocation() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST explicitConstructorInvocation_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  lp1 = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lp1_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  lp2 = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lp2_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case LITERAL_this:
		{
			match(LITERAL_this);
			lp1 = LT(1);
			if ( inputState->guessing == 0 ) {
				lp1_AST = astFactory->create(lp1);
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(lp1_AST));
			}
			match(LPAREN);
			argList();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(RPAREN);
			match(SEMI);
			if ( inputState->guessing==0 ) {
#line 428 "java.g"
				lp1_AST->setType(CTOR_CALL);
#line 2076 "JavaRecognizer.cpp"
			}
			explicitConstructorInvocation_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_super:
		{
			match(LITERAL_super);
			lp2 = LT(1);
			if ( inputState->guessing == 0 ) {
				lp2_AST = astFactory->create(lp2);
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(lp2_AST));
			}
			match(LPAREN);
			argList();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(RPAREN);
			match(SEMI);
			if ( inputState->guessing==0 ) {
#line 430 "java.g"
				lp2_AST->setType(SUPER_CTOR_CALL);
#line 2099 "JavaRecognizer.cpp"
			}
			explicitConstructorInvocation_AST = RefJavaAST(currentAST.root);
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
			recover(ex,_tokenSet_21);
		} else {
			throw;
		}
	}
	returnAST = explicitConstructorInvocation_AST;
}

void JavaRecognizer::statement() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST statement_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST m_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  c = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST c_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  s = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST s_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case LCURLY:
		{
			compoundStatement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			statement_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_if:
		{
			RefJavaAST tmp68_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp68_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp68_AST));
			}
			match(LITERAL_if);
			match(LPAREN);
			expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(RPAREN);
			statement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{
			if ((LA(1) == LITERAL_else) && (_tokenSet_23.member(LA(2)))) {
				match(LITERAL_else);
				statement();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else if ((_tokenSet_28.member(LA(1))) && (_tokenSet_29.member(LA(2)))) {
			}
			else {
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			
			}
			statement_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_for:
		{
			RefJavaAST tmp72_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp72_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp72_AST));
			}
			match(LITERAL_for);
			match(LPAREN);
			forInit();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(SEMI);
			forCond();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(SEMI);
			forIter();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(RPAREN);
			statement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			statement_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_while:
		{
			RefJavaAST tmp77_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp77_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp77_AST));
			}
			match(LITERAL_while);
			match(LPAREN);
			expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(RPAREN);
			statement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			statement_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_do:
		{
			RefJavaAST tmp80_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp80_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp80_AST));
			}
			match(LITERAL_do);
			statement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(LITERAL_while);
			match(LPAREN);
			expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(RPAREN);
			match(SEMI);
			statement_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_break:
		{
			RefJavaAST tmp85_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp85_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp85_AST));
			}
			match(LITERAL_break);
			{
			switch ( LA(1)) {
			case IDENT:
			{
				RefJavaAST tmp86_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp86_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp86_AST));
				}
				match(IDENT);
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
			statement_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_continue:
		{
			RefJavaAST tmp88_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp88_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp88_AST));
			}
			match(LITERAL_continue);
			{
			switch ( LA(1)) {
			case IDENT:
			{
				RefJavaAST tmp89_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp89_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp89_AST));
				}
				match(IDENT);
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
			statement_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_return:
		{
			RefJavaAST tmp91_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp91_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp91_AST));
			}
			match(LITERAL_return);
			{
			switch ( LA(1)) {
			case LITERAL_void:
			case LITERAL_boolean:
			case LITERAL_byte:
			case LITERAL_char:
			case LITERAL_short:
			case LITERAL_int:
			case LITERAL_float:
			case LITERAL_long:
			case LITERAL_double:
			case IDENT:
			case LPAREN:
			case LITERAL_this:
			case LITERAL_super:
			case PLUS:
			case MINUS:
			case INC:
			case DEC:
			case BNOT:
			case LNOT:
			case LITERAL_true:
			case LITERAL_false:
			case LITERAL_null:
			case LITERAL_new:
			case NUM_INT:
			case CHAR_LITERAL:
			case STRING_LITERAL:
			case NUM_FLOAT:
			case NUM_LONG:
			case NUM_DOUBLE:
			{
				expression();
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
			statement_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_switch:
		{
			RefJavaAST tmp93_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp93_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp93_AST));
			}
			match(LITERAL_switch);
			match(LPAREN);
			expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(RPAREN);
			match(LCURLY);
			{ // ( ... )*
			for (;;) {
				if ((LA(1) == LITERAL_case || LA(1) == LITERAL_default)) {
					casesGroup();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
				}
				else {
					goto _loop95;
				}
				
			}
			_loop95:;
			} // ( ... )*
			match(RCURLY);
			statement_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_try:
		{
			tryBlock();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			statement_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_throw:
		{
			RefJavaAST tmp98_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp98_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp98_AST));
			}
			match(LITERAL_throw);
			expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(SEMI);
			statement_AST = RefJavaAST(currentAST.root);
			break;
		}
		case SEMI:
		{
			s = LT(1);
			if ( inputState->guessing == 0 ) {
				s_AST = astFactory->create(s);
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(s_AST));
			}
			match(SEMI);
			if ( inputState->guessing==0 ) {
#line 617 "java.g"
				s_AST->setType(EMPTY_STAT);
#line 2451 "JavaRecognizer.cpp"
			}
			statement_AST = RefJavaAST(currentAST.root);
			break;
		}
		default:
			bool synPredMatched89 = false;
			if (((_tokenSet_30.member(LA(1))) && (_tokenSet_31.member(LA(2))))) {
				int _m89 = mark();
				synPredMatched89 = true;
				inputState->guessing++;
				try {
					{
					declaration();
					}
				}
				catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
					synPredMatched89 = false;
				}
				rewind(_m89);
				inputState->guessing--;
			}
			if ( synPredMatched89 ) {
				declaration();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				match(SEMI);
				statement_AST = RefJavaAST(currentAST.root);
			}
			else if ((_tokenSet_32.member(LA(1))) && (_tokenSet_33.member(LA(2)))) {
				expression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				match(SEMI);
				statement_AST = RefJavaAST(currentAST.root);
			}
			else if ((_tokenSet_34.member(LA(1))) && (_tokenSet_35.member(LA(2)))) {
				modifiers();
				if (inputState->guessing==0) {
					m_AST = returnAST;
				}
				classDefinition(m_AST);
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				statement_AST = RefJavaAST(currentAST.root);
			}
			else if ((LA(1) == IDENT) && (LA(2) == COLON)) {
				RefJavaAST tmp102_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp102_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp102_AST));
				}
				match(IDENT);
				c = LT(1);
				if ( inputState->guessing == 0 ) {
					c_AST = astFactory->create(c);
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(c_AST));
				}
				match(COLON);
				if ( inputState->guessing==0 ) {
#line 560 "java.g"
					c_AST->setType(LABELED_STAT);
#line 2516 "JavaRecognizer.cpp"
				}
				statement();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				statement_AST = RefJavaAST(currentAST.root);
			}
			else if ((LA(1) == LITERAL_synchronized) && (LA(2) == LPAREN)) {
				RefJavaAST tmp103_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp103_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp103_AST));
				}
				match(LITERAL_synchronized);
				match(LPAREN);
				expression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				match(RPAREN);
				compoundStatement();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				statement_AST = RefJavaAST(currentAST.root);
			}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_28);
		} else {
			throw;
		}
	}
	returnAST = statement_AST;
}

void JavaRecognizer::argList() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST argList_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LITERAL_void:
		case LITERAL_boolean:
		case LITERAL_byte:
		case LITERAL_char:
		case LITERAL_short:
		case LITERAL_int:
		case LITERAL_float:
		case LITERAL_long:
		case LITERAL_double:
		case IDENT:
		case LPAREN:
		case LITERAL_this:
		case LITERAL_super:
		case PLUS:
		case MINUS:
		case INC:
		case DEC:
		case BNOT:
		case LNOT:
		case LITERAL_true:
		case LITERAL_false:
		case LITERAL_null:
		case LITERAL_new:
		case NUM_INT:
		case CHAR_LITERAL:
		case STRING_LITERAL:
		case NUM_FLOAT:
		case NUM_LONG:
		case NUM_DOUBLE:
		{
			expressionList();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case RPAREN:
		{
			if ( inputState->guessing==0 ) {
				argList_AST = RefJavaAST(currentAST.root);
#line 1008 "java.g"
				argList_AST = astFactory->create(ELIST,"ELIST");
#line 2609 "JavaRecognizer.cpp"
				currentAST.root = argList_AST;
				if ( argList_AST!=RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
					argList_AST->getFirstChild() != RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
					  currentAST.child = argList_AST->getFirstChild();
				else
					currentAST.child = argList_AST;
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
		argList_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_24);
		} else {
			throw;
		}
	}
	returnAST = argList_AST;
}

/** Declaration of a variable.  This can be a class/instance variable,
 *   or a local variable in a method
 * It can also include possible initialization.
 */
void JavaRecognizer::variableDeclarator(
	RefJavaAST mods, RefJavaAST t
) {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST variableDeclarator_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  id = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST id_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST d_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST v_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		id = LT(1);
		if ( inputState->guessing == 0 ) {
			id_AST = astFactory->create(id);
		}
		match(IDENT);
		declaratorBrackets(t);
		if (inputState->guessing==0) {
			d_AST = returnAST;
		}
		varInitializer();
		if (inputState->guessing==0) {
			v_AST = returnAST;
		}
		if ( inputState->guessing==0 ) {
			variableDeclarator_AST = RefJavaAST(currentAST.root);
#line 448 "java.g"
			variableDeclarator_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(5))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(VARIABLE_DEF,"VARIABLE_DEF")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(mods))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(TYPE,"TYPE")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(d_AST))))))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(id_AST))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(v_AST))));
#line 2672 "JavaRecognizer.cpp"
			currentAST.root = variableDeclarator_AST;
			if ( variableDeclarator_AST!=RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				variableDeclarator_AST->getFirstChild() != RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = variableDeclarator_AST->getFirstChild();
			else
				currentAST.child = variableDeclarator_AST;
			currentAST.advanceChildToEnd();
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
	returnAST = variableDeclarator_AST;
}

void JavaRecognizer::varInitializer() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST varInitializer_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case ASSIGN:
		{
			RefJavaAST tmp106_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp106_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp106_AST));
			}
			match(ASSIGN);
			initializer();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case SEMI:
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
		varInitializer_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_36);
		} else {
			throw;
		}
	}
	returnAST = varInitializer_AST;
}

void JavaRecognizer::initializer() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST initializer_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case LITERAL_void:
		case LITERAL_boolean:
		case LITERAL_byte:
		case LITERAL_char:
		case LITERAL_short:
		case LITERAL_int:
		case LITERAL_float:
		case LITERAL_long:
		case LITERAL_double:
		case IDENT:
		case LPAREN:
		case LITERAL_this:
		case LITERAL_super:
		case PLUS:
		case MINUS:
		case INC:
		case DEC:
		case BNOT:
		case LNOT:
		case LITERAL_true:
		case LITERAL_false:
		case LITERAL_null:
		case LITERAL_new:
		case NUM_INT:
		case CHAR_LITERAL:
		case STRING_LITERAL:
		case NUM_FLOAT:
		case NUM_LONG:
		case NUM_DOUBLE:
		{
			expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			initializer_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LCURLY:
		{
			arrayInitializer();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			initializer_AST = RefJavaAST(currentAST.root);
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
			recover(ex,_tokenSet_37);
		} else {
			throw;
		}
	}
	returnAST = initializer_AST;
}

void JavaRecognizer::arrayInitializer() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST arrayInitializer_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  lc = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lc_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		lc = LT(1);
		if ( inputState->guessing == 0 ) {
			lc_AST = astFactory->create(lc);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(lc_AST));
		}
		match(LCURLY);
		if ( inputState->guessing==0 ) {
#line 462 "java.g"
			lc_AST->setType(ARRAY_INIT);
#line 2826 "JavaRecognizer.cpp"
		}
		{
		switch ( LA(1)) {
		case LITERAL_void:
		case LITERAL_boolean:
		case LITERAL_byte:
		case LITERAL_char:
		case LITERAL_short:
		case LITERAL_int:
		case LITERAL_float:
		case LITERAL_long:
		case LITERAL_double:
		case IDENT:
		case LCURLY:
		case LPAREN:
		case LITERAL_this:
		case LITERAL_super:
		case PLUS:
		case MINUS:
		case INC:
		case DEC:
		case BNOT:
		case LNOT:
		case LITERAL_true:
		case LITERAL_false:
		case LITERAL_null:
		case LITERAL_new:
		case NUM_INT:
		case CHAR_LITERAL:
		case STRING_LITERAL:
		case NUM_FLOAT:
		case NUM_LONG:
		case NUM_DOUBLE:
		{
			initializer();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{ // ( ... )*
			for (;;) {
				if ((LA(1) == COMMA) && (_tokenSet_38.member(LA(2)))) {
					match(COMMA);
					initializer();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
				}
				else {
					goto _loop69;
				}
				
			}
			_loop69:;
			} // ( ... )*
			{
			switch ( LA(1)) {
			case COMMA:
			{
				match(COMMA);
				break;
			}
			case RCURLY:
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
		case RCURLY:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(RCURLY);
		arrayInitializer_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_39);
		} else {
			throw;
		}
	}
	returnAST = arrayInitializer_AST;
}

void JavaRecognizer::expression() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST expression_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		assignmentExpression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if ( inputState->guessing==0 ) {
			expression_AST = RefJavaAST(currentAST.root);
#line 715 "java.g"
			expression_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(EXPR,"EXPR")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(expression_AST))));
#line 2938 "JavaRecognizer.cpp"
			currentAST.root = expression_AST;
			if ( expression_AST!=RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				expression_AST->getFirstChild() != RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = expression_AST->getFirstChild();
			else
				currentAST.child = expression_AST;
			currentAST.advanceChildToEnd();
		}
		expression_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_40);
		} else {
			throw;
		}
	}
	returnAST = expression_AST;
}

void JavaRecognizer::parameterDeclaration() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST parameterDeclaration_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST pm_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST t_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  id = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST id_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST pd_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		parameterModifier();
		if (inputState->guessing==0) {
			pm_AST = returnAST;
		}
		typeSpec(false);
		if (inputState->guessing==0) {
			t_AST = returnAST;
		}
		id = LT(1);
		if ( inputState->guessing == 0 ) {
			id_AST = astFactory->create(id);
		}
		match(IDENT);
		declaratorBrackets(t_AST);
		if (inputState->guessing==0) {
			pd_AST = returnAST;
		}
		if ( inputState->guessing==0 ) {
			parameterDeclaration_AST = RefJavaAST(currentAST.root);
#line 516 "java.g"
			parameterDeclaration_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(4))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(PARAMETER_DEF,"PARAMETER_DEF")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(pm_AST))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(TYPE,"TYPE")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(pd_AST))))))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(id_AST))));
#line 2992 "JavaRecognizer.cpp"
			currentAST.root = parameterDeclaration_AST;
			if ( parameterDeclaration_AST!=RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				parameterDeclaration_AST->getFirstChild() != RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = parameterDeclaration_AST->getFirstChild();
			else
				currentAST.child = parameterDeclaration_AST;
			currentAST.advanceChildToEnd();
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_41);
		} else {
			throw;
		}
	}
	returnAST = parameterDeclaration_AST;
}

void JavaRecognizer::parameterModifier() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST parameterModifier_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  f = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST f_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case FINAL:
		{
			f = LT(1);
			if ( inputState->guessing == 0 ) {
				f_AST = astFactory->create(f);
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(f_AST));
			}
			match(FINAL);
			break;
		}
		case LITERAL_void:
		case LITERAL_boolean:
		case LITERAL_byte:
		case LITERAL_char:
		case LITERAL_short:
		case LITERAL_int:
		case LITERAL_float:
		case LITERAL_long:
		case LITERAL_double:
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
		if ( inputState->guessing==0 ) {
			parameterModifier_AST = RefJavaAST(currentAST.root);
#line 522 "java.g"
			parameterModifier_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(MODIFIERS,"MODIFIERS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(f_AST))));
#line 3056 "JavaRecognizer.cpp"
			currentAST.root = parameterModifier_AST;
			if ( parameterModifier_AST!=RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				parameterModifier_AST->getFirstChild() != RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = parameterModifier_AST->getFirstChild();
			else
				currentAST.child = parameterModifier_AST;
			currentAST.advanceChildToEnd();
		}
		parameterModifier_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_42);
		} else {
			throw;
		}
	}
	returnAST = parameterModifier_AST;
}

void JavaRecognizer::forInit() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST forInit_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		bool synPredMatched107 = false;
		if (((_tokenSet_30.member(LA(1))) && (_tokenSet_31.member(LA(2))))) {
			int _m107 = mark();
			synPredMatched107 = true;
			inputState->guessing++;
			try {
				{
				declaration();
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched107 = false;
			}
			rewind(_m107);
			inputState->guessing--;
		}
		if ( synPredMatched107 ) {
			declaration();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
		}
		else if ((_tokenSet_32.member(LA(1))) && (_tokenSet_43.member(LA(2)))) {
			expressionList();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
		}
		else if ((LA(1) == SEMI)) {
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
		if ( inputState->guessing==0 ) {
			forInit_AST = RefJavaAST(currentAST.root);
#line 651 "java.g"
			forInit_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(FOR_INIT,"FOR_INIT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(forInit_AST))));
#line 3124 "JavaRecognizer.cpp"
			currentAST.root = forInit_AST;
			if ( forInit_AST!=RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				forInit_AST->getFirstChild() != RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = forInit_AST->getFirstChild();
			else
				currentAST.child = forInit_AST;
			currentAST.advanceChildToEnd();
		}
		forInit_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_5);
		} else {
			throw;
		}
	}
	returnAST = forInit_AST;
}

void JavaRecognizer::forCond() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST forCond_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LITERAL_void:
		case LITERAL_boolean:
		case LITERAL_byte:
		case LITERAL_char:
		case LITERAL_short:
		case LITERAL_int:
		case LITERAL_float:
		case LITERAL_long:
		case LITERAL_double:
		case IDENT:
		case LPAREN:
		case LITERAL_this:
		case LITERAL_super:
		case PLUS:
		case MINUS:
		case INC:
		case DEC:
		case BNOT:
		case LNOT:
		case LITERAL_true:
		case LITERAL_false:
		case LITERAL_null:
		case LITERAL_new:
		case NUM_INT:
		case CHAR_LITERAL:
		case STRING_LITERAL:
		case NUM_FLOAT:
		case NUM_LONG:
		case NUM_DOUBLE:
		{
			expression();
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
		if ( inputState->guessing==0 ) {
			forCond_AST = RefJavaAST(currentAST.root);
#line 656 "java.g"
			forCond_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(FOR_CONDITION,"FOR_CONDITION")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(forCond_AST))));
#line 3204 "JavaRecognizer.cpp"
			currentAST.root = forCond_AST;
			if ( forCond_AST!=RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				forCond_AST->getFirstChild() != RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = forCond_AST->getFirstChild();
			else
				currentAST.child = forCond_AST;
			currentAST.advanceChildToEnd();
		}
		forCond_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_5);
		} else {
			throw;
		}
	}
	returnAST = forCond_AST;
}

void JavaRecognizer::forIter() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST forIter_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LITERAL_void:
		case LITERAL_boolean:
		case LITERAL_byte:
		case LITERAL_char:
		case LITERAL_short:
		case LITERAL_int:
		case LITERAL_float:
		case LITERAL_long:
		case LITERAL_double:
		case IDENT:
		case LPAREN:
		case LITERAL_this:
		case LITERAL_super:
		case PLUS:
		case MINUS:
		case INC:
		case DEC:
		case BNOT:
		case LNOT:
		case LITERAL_true:
		case LITERAL_false:
		case LITERAL_null:
		case LITERAL_new:
		case NUM_INT:
		case CHAR_LITERAL:
		case STRING_LITERAL:
		case NUM_FLOAT:
		case NUM_LONG:
		case NUM_DOUBLE:
		{
			expressionList();
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
		if ( inputState->guessing==0 ) {
			forIter_AST = RefJavaAST(currentAST.root);
#line 661 "java.g"
			forIter_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(FOR_ITERATOR,"FOR_ITERATOR")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(forIter_AST))));
#line 3284 "JavaRecognizer.cpp"
			currentAST.root = forIter_AST;
			if ( forIter_AST!=RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				forIter_AST->getFirstChild() != RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = forIter_AST->getFirstChild();
			else
				currentAST.child = forIter_AST;
			currentAST.advanceChildToEnd();
		}
		forIter_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_24);
		} else {
			throw;
		}
	}
	returnAST = forIter_AST;
}

void JavaRecognizer::casesGroup() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST casesGroup_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{ // ( ... )+
		int _cnt98=0;
		for (;;) {
			if ((LA(1) == LITERAL_case || LA(1) == LITERAL_default) && (_tokenSet_44.member(LA(2)))) {
				aCase();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				if ( _cnt98>=1 ) { goto _loop98; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
			}
			
			_cnt98++;
		}
		_loop98:;
		}  // ( ... )+
		caseSList();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if ( inputState->guessing==0 ) {
			casesGroup_AST = RefJavaAST(currentAST.root);
#line 632 "java.g"
			casesGroup_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(CASE_GROUP,"CASE_GROUP")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(casesGroup_AST))));
#line 3337 "JavaRecognizer.cpp"
			currentAST.root = casesGroup_AST;
			if ( casesGroup_AST!=RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				casesGroup_AST->getFirstChild() != RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = casesGroup_AST->getFirstChild();
			else
				currentAST.child = casesGroup_AST;
			currentAST.advanceChildToEnd();
		}
		casesGroup_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_45);
		} else {
			throw;
		}
	}
	returnAST = casesGroup_AST;
}

void JavaRecognizer::tryBlock() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST tryBlock_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefJavaAST tmp110_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp110_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp110_AST));
		}
		match(LITERAL_try);
		compoundStatement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == LITERAL_catch)) {
				handler();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop114;
			}
			
		}
		_loop114:;
		} // ( ... )*
		{
		switch ( LA(1)) {
		case LITERAL_finally:
		{
			finallyClause();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case FINAL:
		case ABSTRACT:
		case STRICTFP:
		case SEMI:
		case LITERAL_void:
		case LITERAL_boolean:
		case LITERAL_byte:
		case LITERAL_char:
		case LITERAL_short:
		case LITERAL_int:
		case LITERAL_float:
		case LITERAL_long:
		case LITERAL_double:
		case IDENT:
		case LITERAL_private:
		case LITERAL_public:
		case LITERAL_protected:
		case LITERAL_static:
		case LITERAL_transient:
		case LITERAL_native:
		case LITERAL_threadsafe:
		case LITERAL_synchronized:
		case LITERAL_volatile:
		case LITERAL_class:
		case LCURLY:
		case RCURLY:
		case LPAREN:
		case LITERAL_this:
		case LITERAL_super:
		case LITERAL_if:
		case LITERAL_else:
		case LITERAL_for:
		case LITERAL_while:
		case LITERAL_do:
		case LITERAL_break:
		case LITERAL_continue:
		case LITERAL_return:
		case LITERAL_switch:
		case LITERAL_throw:
		case LITERAL_case:
		case LITERAL_default:
		case LITERAL_try:
		case PLUS:
		case MINUS:
		case INC:
		case DEC:
		case BNOT:
		case LNOT:
		case LITERAL_true:
		case LITERAL_false:
		case LITERAL_null:
		case LITERAL_new:
		case NUM_INT:
		case CHAR_LITERAL:
		case STRING_LITERAL:
		case NUM_FLOAT:
		case NUM_LONG:
		case NUM_DOUBLE:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		tryBlock_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_28);
		} else {
			throw;
		}
	}
	returnAST = tryBlock_AST;
}

void JavaRecognizer::aCase() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST aCase_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LITERAL_case:
		{
			RefJavaAST tmp111_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp111_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp111_AST));
			}
			match(LITERAL_case);
			expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case LITERAL_default:
		{
			RefJavaAST tmp112_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp112_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp112_AST));
			}
			match(LITERAL_default);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(COLON);
		aCase_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_46);
		} else {
			throw;
		}
	}
	returnAST = aCase_AST;
}

void JavaRecognizer::caseSList() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST caseSList_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{ // ( ... )*
		for (;;) {
			if ((_tokenSet_23.member(LA(1)))) {
				statement();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop103;
			}
			
		}
		_loop103:;
		} // ( ... )*
		if ( inputState->guessing==0 ) {
			caseSList_AST = RefJavaAST(currentAST.root);
#line 641 "java.g"
			caseSList_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(SLIST,"SLIST")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(caseSList_AST))));
#line 3557 "JavaRecognizer.cpp"
			currentAST.root = caseSList_AST;
			if ( caseSList_AST!=RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				caseSList_AST->getFirstChild() != RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = caseSList_AST->getFirstChild();
			else
				currentAST.child = caseSList_AST;
			currentAST.advanceChildToEnd();
		}
		caseSList_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_45);
		} else {
			throw;
		}
	}
	returnAST = caseSList_AST;
}

void JavaRecognizer::expressionList() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST expressionList_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
				goto _loop121;
			}
			
		}
		_loop121:;
		} // ( ... )*
		if ( inputState->guessing==0 ) {
			expressionList_AST = RefJavaAST(currentAST.root);
#line 721 "java.g"
			expressionList_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(ELIST,"ELIST")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(expressionList_AST))));
#line 3609 "JavaRecognizer.cpp"
			currentAST.root = expressionList_AST;
			if ( expressionList_AST!=RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				expressionList_AST->getFirstChild() != RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = expressionList_AST->getFirstChild();
			else
				currentAST.child = expressionList_AST;
			currentAST.advanceChildToEnd();
		}
		expressionList_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_47);
		} else {
			throw;
		}
	}
	returnAST = expressionList_AST;
}

void JavaRecognizer::handler() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST handler_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefJavaAST tmp115_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp115_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp115_AST));
		}
		match(LITERAL_catch);
		match(LPAREN);
		parameterDeclaration();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(RPAREN);
		compoundStatement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		handler_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_48);
		} else {
			throw;
		}
	}
	returnAST = handler_AST;
}

void JavaRecognizer::finallyClause() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST finallyClause_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefJavaAST tmp118_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp118_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp118_AST));
		}
		match(LITERAL_finally);
		compoundStatement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		finallyClause_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_28);
		} else {
			throw;
		}
	}
	returnAST = finallyClause_AST;
}

void JavaRecognizer::assignmentExpression() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST assignmentExpression_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		conditionalExpression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case ASSIGN:
		case PLUS_ASSIGN:
		case MINUS_ASSIGN:
		case STAR_ASSIGN:
		case DIV_ASSIGN:
		case MOD_ASSIGN:
		case SR_ASSIGN:
		case BSR_ASSIGN:
		case SL_ASSIGN:
		case BAND_ASSIGN:
		case BXOR_ASSIGN:
		case BOR_ASSIGN:
		{
			{
			switch ( LA(1)) {
			case ASSIGN:
			{
				RefJavaAST tmp119_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp119_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp119_AST));
				}
				match(ASSIGN);
				break;
			}
			case PLUS_ASSIGN:
			{
				RefJavaAST tmp120_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp120_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp120_AST));
				}
				match(PLUS_ASSIGN);
				break;
			}
			case MINUS_ASSIGN:
			{
				RefJavaAST tmp121_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp121_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp121_AST));
				}
				match(MINUS_ASSIGN);
				break;
			}
			case STAR_ASSIGN:
			{
				RefJavaAST tmp122_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp122_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp122_AST));
				}
				match(STAR_ASSIGN);
				break;
			}
			case DIV_ASSIGN:
			{
				RefJavaAST tmp123_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp123_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp123_AST));
				}
				match(DIV_ASSIGN);
				break;
			}
			case MOD_ASSIGN:
			{
				RefJavaAST tmp124_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp124_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp124_AST));
				}
				match(MOD_ASSIGN);
				break;
			}
			case SR_ASSIGN:
			{
				RefJavaAST tmp125_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp125_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp125_AST));
				}
				match(SR_ASSIGN);
				break;
			}
			case BSR_ASSIGN:
			{
				RefJavaAST tmp126_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp126_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp126_AST));
				}
				match(BSR_ASSIGN);
				break;
			}
			case SL_ASSIGN:
			{
				RefJavaAST tmp127_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp127_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp127_AST));
				}
				match(SL_ASSIGN);
				break;
			}
			case BAND_ASSIGN:
			{
				RefJavaAST tmp128_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp128_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp128_AST));
				}
				match(BAND_ASSIGN);
				break;
			}
			case BXOR_ASSIGN:
			{
				RefJavaAST tmp129_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp129_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp129_AST));
				}
				match(BXOR_ASSIGN);
				break;
			}
			case BOR_ASSIGN:
			{
				RefJavaAST tmp130_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp130_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp130_AST));
				}
				match(BOR_ASSIGN);
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			assignmentExpression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case SEMI:
		case RBRACK:
		case RCURLY:
		case COMMA:
		case RPAREN:
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
		assignmentExpression_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_40);
		} else {
			throw;
		}
	}
	returnAST = assignmentExpression_AST;
}

void JavaRecognizer::conditionalExpression() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST conditionalExpression_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		logicalOrExpression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case QUESTION:
		{
			RefJavaAST tmp131_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp131_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp131_AST));
			}
			match(QUESTION);
			assignmentExpression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(COLON);
			conditionalExpression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case SEMI:
		case RBRACK:
		case RCURLY:
		case COMMA:
		case RPAREN:
		case ASSIGN:
		case COLON:
		case PLUS_ASSIGN:
		case MINUS_ASSIGN:
		case STAR_ASSIGN:
		case DIV_ASSIGN:
		case MOD_ASSIGN:
		case SR_ASSIGN:
		case BSR_ASSIGN:
		case SL_ASSIGN:
		case BAND_ASSIGN:
		case BXOR_ASSIGN:
		case BOR_ASSIGN:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		conditionalExpression_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_49);
		} else {
			throw;
		}
	}
	returnAST = conditionalExpression_AST;
}

void JavaRecognizer::logicalOrExpression() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST logicalOrExpression_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		logicalAndExpression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == LOR)) {
				RefJavaAST tmp133_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp133_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp133_AST));
				}
				match(LOR);
				logicalAndExpression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop129;
			}
			
		}
		_loop129:;
		} // ( ... )*
		logicalOrExpression_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_50);
		} else {
			throw;
		}
	}
	returnAST = logicalOrExpression_AST;
}

void JavaRecognizer::logicalAndExpression() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST logicalAndExpression_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		inclusiveOrExpression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == LAND)) {
				RefJavaAST tmp134_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp134_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp134_AST));
				}
				match(LAND);
				inclusiveOrExpression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop132;
			}
			
		}
		_loop132:;
		} // ( ... )*
		logicalAndExpression_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_51);
		} else {
			throw;
		}
	}
	returnAST = logicalAndExpression_AST;
}

void JavaRecognizer::inclusiveOrExpression() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST inclusiveOrExpression_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		exclusiveOrExpression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == BOR)) {
				RefJavaAST tmp135_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp135_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp135_AST));
				}
				match(BOR);
				exclusiveOrExpression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop135;
			}
			
		}
		_loop135:;
		} // ( ... )*
		inclusiveOrExpression_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_52);
		} else {
			throw;
		}
	}
	returnAST = inclusiveOrExpression_AST;
}

void JavaRecognizer::exclusiveOrExpression() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST exclusiveOrExpression_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		andExpression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == BXOR)) {
				RefJavaAST tmp136_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp136_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp136_AST));
				}
				match(BXOR);
				andExpression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop138;
			}
			
		}
		_loop138:;
		} // ( ... )*
		exclusiveOrExpression_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_53);
		} else {
			throw;
		}
	}
	returnAST = exclusiveOrExpression_AST;
}

void JavaRecognizer::andExpression() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST andExpression_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		equalityExpression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == BAND)) {
				RefJavaAST tmp137_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp137_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp137_AST));
				}
				match(BAND);
				equalityExpression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop141;
			}
			
		}
		_loop141:;
		} // ( ... )*
		andExpression_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_54);
		} else {
			throw;
		}
	}
	returnAST = andExpression_AST;
}

void JavaRecognizer::equalityExpression() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST equalityExpression_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		relationalExpression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == NOT_EQUAL || LA(1) == EQUAL)) {
				{
				switch ( LA(1)) {
				case NOT_EQUAL:
				{
					RefJavaAST tmp138_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if ( inputState->guessing == 0 ) {
						tmp138_AST = astFactory->create(LT(1));
						astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp138_AST));
					}
					match(NOT_EQUAL);
					break;
				}
				case EQUAL:
				{
					RefJavaAST tmp139_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if ( inputState->guessing == 0 ) {
						tmp139_AST = astFactory->create(LT(1));
						astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp139_AST));
					}
					match(EQUAL);
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				relationalExpression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop145;
			}
			
		}
		_loop145:;
		} // ( ... )*
		equalityExpression_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_55);
		} else {
			throw;
		}
	}
	returnAST = equalityExpression_AST;
}

void JavaRecognizer::relationalExpression() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST relationalExpression_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		shiftExpression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case SEMI:
		case RBRACK:
		case RCURLY:
		case COMMA:
		case RPAREN:
		case ASSIGN:
		case COLON:
		case PLUS_ASSIGN:
		case MINUS_ASSIGN:
		case STAR_ASSIGN:
		case DIV_ASSIGN:
		case MOD_ASSIGN:
		case SR_ASSIGN:
		case BSR_ASSIGN:
		case SL_ASSIGN:
		case BAND_ASSIGN:
		case BXOR_ASSIGN:
		case BOR_ASSIGN:
		case QUESTION:
		case LOR:
		case LAND:
		case BOR:
		case BXOR:
		case BAND:
		case NOT_EQUAL:
		case EQUAL:
		case LT_:
		case GT:
		case LE:
		case GE:
		{
			{ // ( ... )*
			for (;;) {
				if (((LA(1) >= LT_ && LA(1) <= GE))) {
					{
					switch ( LA(1)) {
					case LT_:
					{
						RefJavaAST tmp140_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
						if ( inputState->guessing == 0 ) {
							tmp140_AST = astFactory->create(LT(1));
							astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp140_AST));
						}
						match(LT_);
						break;
					}
					case GT:
					{
						RefJavaAST tmp141_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
						if ( inputState->guessing == 0 ) {
							tmp141_AST = astFactory->create(LT(1));
							astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp141_AST));
						}
						match(GT);
						break;
					}
					case LE:
					{
						RefJavaAST tmp142_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
						if ( inputState->guessing == 0 ) {
							tmp142_AST = astFactory->create(LT(1));
							astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp142_AST));
						}
						match(LE);
						break;
					}
					case GE:
					{
						RefJavaAST tmp143_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
						if ( inputState->guessing == 0 ) {
							tmp143_AST = astFactory->create(LT(1));
							astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp143_AST));
						}
						match(GE);
						break;
					}
					default:
					{
						throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
					}
					}
					}
					shiftExpression();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
				}
				else {
					goto _loop150;
				}
				
			}
			_loop150:;
			} // ( ... )*
			break;
		}
		case LITERAL_instanceof:
		{
			RefJavaAST tmp144_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp144_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp144_AST));
			}
			match(LITERAL_instanceof);
			typeSpec(true);
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
		relationalExpression_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_56);
		} else {
			throw;
		}
	}
	returnAST = relationalExpression_AST;
}

void JavaRecognizer::shiftExpression() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST shiftExpression_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		additiveExpression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if (((LA(1) >= SL && LA(1) <= BSR))) {
				{
				switch ( LA(1)) {
				case SL:
				{
					RefJavaAST tmp145_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if ( inputState->guessing == 0 ) {
						tmp145_AST = astFactory->create(LT(1));
						astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp145_AST));
					}
					match(SL);
					break;
				}
				case SR:
				{
					RefJavaAST tmp146_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if ( inputState->guessing == 0 ) {
						tmp146_AST = astFactory->create(LT(1));
						astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp146_AST));
					}
					match(SR);
					break;
				}
				case BSR:
				{
					RefJavaAST tmp147_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if ( inputState->guessing == 0 ) {
						tmp147_AST = astFactory->create(LT(1));
						astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp147_AST));
					}
					match(BSR);
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				additiveExpression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop154;
			}
			
		}
		_loop154:;
		} // ( ... )*
		shiftExpression_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_57);
		} else {
			throw;
		}
	}
	returnAST = shiftExpression_AST;
}

void JavaRecognizer::additiveExpression() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST additiveExpression_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		multiplicativeExpression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == PLUS || LA(1) == MINUS)) {
				{
				switch ( LA(1)) {
				case PLUS:
				{
					RefJavaAST tmp148_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if ( inputState->guessing == 0 ) {
						tmp148_AST = astFactory->create(LT(1));
						astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp148_AST));
					}
					match(PLUS);
					break;
				}
				case MINUS:
				{
					RefJavaAST tmp149_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if ( inputState->guessing == 0 ) {
						tmp149_AST = astFactory->create(LT(1));
						astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp149_AST));
					}
					match(MINUS);
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				multiplicativeExpression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop158;
			}
			
		}
		_loop158:;
		} // ( ... )*
		additiveExpression_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_58);
		} else {
			throw;
		}
	}
	returnAST = additiveExpression_AST;
}

void JavaRecognizer::multiplicativeExpression() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST multiplicativeExpression_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		unaryExpression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((_tokenSet_59.member(LA(1)))) {
				{
				switch ( LA(1)) {
				case STAR:
				{
					RefJavaAST tmp150_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if ( inputState->guessing == 0 ) {
						tmp150_AST = astFactory->create(LT(1));
						astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp150_AST));
					}
					match(STAR);
					break;
				}
				case DIV:
				{
					RefJavaAST tmp151_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if ( inputState->guessing == 0 ) {
						tmp151_AST = astFactory->create(LT(1));
						astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp151_AST));
					}
					match(DIV);
					break;
				}
				case MOD:
				{
					RefJavaAST tmp152_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if ( inputState->guessing == 0 ) {
						tmp152_AST = astFactory->create(LT(1));
						astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp152_AST));
					}
					match(MOD);
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				unaryExpression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop162;
			}
			
		}
		_loop162:;
		} // ( ... )*
		multiplicativeExpression_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_60);
		} else {
			throw;
		}
	}
	returnAST = multiplicativeExpression_AST;
}

void JavaRecognizer::unaryExpression() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST unaryExpression_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case INC:
		{
			RefJavaAST tmp153_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp153_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp153_AST));
			}
			match(INC);
			unaryExpression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			unaryExpression_AST = RefJavaAST(currentAST.root);
			break;
		}
		case DEC:
		{
			RefJavaAST tmp154_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp154_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp154_AST));
			}
			match(DEC);
			unaryExpression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			unaryExpression_AST = RefJavaAST(currentAST.root);
			break;
		}
		case MINUS:
		{
			RefJavaAST tmp155_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp155_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp155_AST));
			}
			match(MINUS);
			if ( inputState->guessing==0 ) {
#line 812 "java.g"
				tmp155_AST->setType(UNARY_MINUS);
#line 4646 "JavaRecognizer.cpp"
			}
			unaryExpression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			unaryExpression_AST = RefJavaAST(currentAST.root);
			break;
		}
		case PLUS:
		{
			RefJavaAST tmp156_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp156_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp156_AST));
			}
			match(PLUS);
			if ( inputState->guessing==0 ) {
#line 813 "java.g"
				tmp156_AST->setType(UNARY_PLUS);
#line 4666 "JavaRecognizer.cpp"
			}
			unaryExpression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			unaryExpression_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_void:
		case LITERAL_boolean:
		case LITERAL_byte:
		case LITERAL_char:
		case LITERAL_short:
		case LITERAL_int:
		case LITERAL_float:
		case LITERAL_long:
		case LITERAL_double:
		case IDENT:
		case LPAREN:
		case LITERAL_this:
		case LITERAL_super:
		case BNOT:
		case LNOT:
		case LITERAL_true:
		case LITERAL_false:
		case LITERAL_null:
		case LITERAL_new:
		case NUM_INT:
		case CHAR_LITERAL:
		case STRING_LITERAL:
		case NUM_FLOAT:
		case NUM_LONG:
		case NUM_DOUBLE:
		{
			unaryExpressionNotPlusMinus();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			unaryExpression_AST = RefJavaAST(currentAST.root);
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
	returnAST = unaryExpression_AST;
}

void JavaRecognizer::unaryExpressionNotPlusMinus() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST unaryExpressionNotPlusMinus_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  lpb = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lpb_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  lp = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lp_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case BNOT:
		{
			RefJavaAST tmp157_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp157_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp157_AST));
			}
			match(BNOT);
			unaryExpression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			unaryExpressionNotPlusMinus_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LNOT:
		{
			RefJavaAST tmp158_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp158_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp158_AST));
			}
			match(LNOT);
			unaryExpression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			unaryExpressionNotPlusMinus_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_void:
		case LITERAL_boolean:
		case LITERAL_byte:
		case LITERAL_char:
		case LITERAL_short:
		case LITERAL_int:
		case LITERAL_float:
		case LITERAL_long:
		case LITERAL_double:
		case IDENT:
		case LPAREN:
		case LITERAL_this:
		case LITERAL_super:
		case LITERAL_true:
		case LITERAL_false:
		case LITERAL_null:
		case LITERAL_new:
		case NUM_INT:
		case CHAR_LITERAL:
		case STRING_LITERAL:
		case NUM_FLOAT:
		case NUM_LONG:
		case NUM_DOUBLE:
		{
			{
			if ((LA(1) == LPAREN) && ((LA(2) >= LITERAL_void && LA(2) <= LITERAL_double))) {
				lpb = LT(1);
				if ( inputState->guessing == 0 ) {
					lpb_AST = astFactory->create(lpb);
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(lpb_AST));
				}
				match(LPAREN);
				if ( inputState->guessing==0 ) {
#line 829 "java.g"
					lpb_AST->setType(TYPECAST);
#line 4801 "JavaRecognizer.cpp"
				}
				builtInTypeSpec(true);
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				match(RPAREN);
				unaryExpression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				bool synPredMatched167 = false;
				if (((LA(1) == LPAREN) && (LA(2) == IDENT))) {
					int _m167 = mark();
					synPredMatched167 = true;
					inputState->guessing++;
					try {
						{
						match(LPAREN);
						classTypeSpec(true);
						match(RPAREN);
						unaryExpressionNotPlusMinus();
						}
					}
					catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
						synPredMatched167 = false;
					}
					rewind(_m167);
					inputState->guessing--;
				}
				if ( synPredMatched167 ) {
					lp = LT(1);
					if ( inputState->guessing == 0 ) {
						lp_AST = astFactory->create(lp);
						astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(lp_AST));
					}
					match(LPAREN);
					if ( inputState->guessing==0 ) {
#line 836 "java.g"
						lp_AST->setType(TYPECAST);
#line 4843 "JavaRecognizer.cpp"
					}
					classTypeSpec(true);
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					match(RPAREN);
					unaryExpressionNotPlusMinus();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
				}
				else if ((_tokenSet_62.member(LA(1))) && (_tokenSet_63.member(LA(2)))) {
					postfixExpression();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
				}
			else {
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			unaryExpressionNotPlusMinus_AST = RefJavaAST(currentAST.root);
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
	returnAST = unaryExpressionNotPlusMinus_AST;
}

void JavaRecognizer::postfixExpression() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST postfixExpression_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  lp = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lp_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  lp3 = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lp3_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  lps = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lps_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  lb = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lb_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  in = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST in_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  de = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST de_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		primaryExpression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == DOT) && (LA(2) == IDENT)) {
				RefJavaAST tmp161_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp161_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp161_AST));
				}
				match(DOT);
				RefJavaAST tmp162_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp162_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp162_AST));
				}
				match(IDENT);
				{
				switch ( LA(1)) {
				case LPAREN:
				{
					lp = LT(1);
					if ( inputState->guessing == 0 ) {
						lp_AST = astFactory->create(lp);
						astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(lp_AST));
					}
					match(LPAREN);
					if ( inputState->guessing==0 ) {
#line 867 "java.g"
						lp_AST->setType(METHOD_CALL);
#line 4936 "JavaRecognizer.cpp"
					}
					argList();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					match(RPAREN);
					break;
				}
				case SEMI:
				case LBRACK:
				case RBRACK:
				case DOT:
				case STAR:
				case RCURLY:
				case COMMA:
				case RPAREN:
				case ASSIGN:
				case COLON:
				case PLUS_ASSIGN:
				case MINUS_ASSIGN:
				case STAR_ASSIGN:
				case DIV_ASSIGN:
				case MOD_ASSIGN:
				case SR_ASSIGN:
				case BSR_ASSIGN:
				case SL_ASSIGN:
				case BAND_ASSIGN:
				case BXOR_ASSIGN:
				case BOR_ASSIGN:
				case QUESTION:
				case LOR:
				case LAND:
				case BOR:
				case BXOR:
				case BAND:
				case NOT_EQUAL:
				case EQUAL:
				case LT_:
				case GT:
				case LE:
				case GE:
				case LITERAL_instanceof:
				case SL:
				case SR:
				case BSR:
				case PLUS:
				case MINUS:
				case DIV:
				case MOD:
				case INC:
				case DEC:
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
			else if ((LA(1) == DOT) && (LA(2) == LITERAL_this)) {
				RefJavaAST tmp164_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp164_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp164_AST));
				}
				match(DOT);
				RefJavaAST tmp165_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp165_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp165_AST));
				}
				match(LITERAL_this);
			}
			else if ((LA(1) == DOT) && (LA(2) == LITERAL_super)) {
				RefJavaAST tmp166_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp166_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp166_AST));
				}
				match(DOT);
				RefJavaAST tmp167_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp167_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp167_AST));
				}
				match(LITERAL_super);
				{
				switch ( LA(1)) {
				case LPAREN:
				{
					lp3 = LT(1);
					if ( inputState->guessing == 0 ) {
						lp3_AST = astFactory->create(lp3);
						astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(lp3_AST));
					}
					match(LPAREN);
					argList();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					match(RPAREN);
					if ( inputState->guessing==0 ) {
#line 876 "java.g"
						lp3_AST->setType(SUPER_CTOR_CALL);
#line 5043 "JavaRecognizer.cpp"
					}
					break;
				}
				case DOT:
				{
					RefJavaAST tmp169_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if ( inputState->guessing == 0 ) {
						tmp169_AST = astFactory->create(LT(1));
						astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp169_AST));
					}
					match(DOT);
					RefJavaAST tmp170_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if ( inputState->guessing == 0 ) {
						tmp170_AST = astFactory->create(LT(1));
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp170_AST));
					}
					match(IDENT);
					{
					switch ( LA(1)) {
					case LPAREN:
					{
						lps = LT(1);
						if ( inputState->guessing == 0 ) {
							lps_AST = astFactory->create(lps);
							astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(lps_AST));
						}
						match(LPAREN);
						if ( inputState->guessing==0 ) {
#line 878 "java.g"
							lps_AST->setType(METHOD_CALL);
#line 5074 "JavaRecognizer.cpp"
						}
						argList();
						if (inputState->guessing==0) {
							astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
						}
						match(RPAREN);
						break;
					}
					case SEMI:
					case LBRACK:
					case RBRACK:
					case DOT:
					case STAR:
					case RCURLY:
					case COMMA:
					case RPAREN:
					case ASSIGN:
					case COLON:
					case PLUS_ASSIGN:
					case MINUS_ASSIGN:
					case STAR_ASSIGN:
					case DIV_ASSIGN:
					case MOD_ASSIGN:
					case SR_ASSIGN:
					case BSR_ASSIGN:
					case SL_ASSIGN:
					case BAND_ASSIGN:
					case BXOR_ASSIGN:
					case BOR_ASSIGN:
					case QUESTION:
					case LOR:
					case LAND:
					case BOR:
					case BXOR:
					case BAND:
					case NOT_EQUAL:
					case EQUAL:
					case LT_:
					case GT:
					case LE:
					case GE:
					case LITERAL_instanceof:
					case SL:
					case SR:
					case BSR:
					case PLUS:
					case MINUS:
					case DIV:
					case MOD:
					case INC:
					case DEC:
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
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
			}
			else if ((LA(1) == DOT) && (LA(2) == LITERAL_new)) {
				RefJavaAST tmp172_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp172_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp172_AST));
				}
				match(DOT);
				newExpression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else if ((LA(1) == LBRACK)) {
				lb = LT(1);
				if ( inputState->guessing == 0 ) {
					lb_AST = astFactory->create(lb);
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(lb_AST));
				}
				match(LBRACK);
				if ( inputState->guessing==0 ) {
#line 884 "java.g"
					lb_AST->setType(INDEX_OP);
#line 5166 "JavaRecognizer.cpp"
				}
				expression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				match(RBRACK);
			}
			else {
				goto _loop173;
			}
			
		}
		_loop173:;
		} // ( ... )*
		{
		switch ( LA(1)) {
		case INC:
		{
			in = LT(1);
			if ( inputState->guessing == 0 ) {
				in_AST = astFactory->create(in);
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(in_AST));
			}
			match(INC);
			if ( inputState->guessing==0 ) {
#line 889 "java.g"
				in_AST->setType(POST_INC);
#line 5194 "JavaRecognizer.cpp"
			}
			break;
		}
		case DEC:
		{
			de = LT(1);
			if ( inputState->guessing == 0 ) {
				de_AST = astFactory->create(de);
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(de_AST));
			}
			match(DEC);
			if ( inputState->guessing==0 ) {
#line 890 "java.g"
				de_AST->setType(POST_DEC);
#line 5209 "JavaRecognizer.cpp"
			}
			break;
		}
		case SEMI:
		case RBRACK:
		case STAR:
		case RCURLY:
		case COMMA:
		case RPAREN:
		case ASSIGN:
		case COLON:
		case PLUS_ASSIGN:
		case MINUS_ASSIGN:
		case STAR_ASSIGN:
		case DIV_ASSIGN:
		case MOD_ASSIGN:
		case SR_ASSIGN:
		case BSR_ASSIGN:
		case SL_ASSIGN:
		case BAND_ASSIGN:
		case BXOR_ASSIGN:
		case BOR_ASSIGN:
		case QUESTION:
		case LOR:
		case LAND:
		case BOR:
		case BXOR:
		case BAND:
		case NOT_EQUAL:
		case EQUAL:
		case LT_:
		case GT:
		case LE:
		case GE:
		case LITERAL_instanceof:
		case SL:
		case SR:
		case BSR:
		case PLUS:
		case MINUS:
		case DIV:
		case MOD:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		postfixExpression_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_61);
		} else {
			throw;
		}
	}
	returnAST = postfixExpression_AST;
}

void JavaRecognizer::primaryExpression() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST primaryExpression_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  lbt = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lbt_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case IDENT:
		{
			identPrimary();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{
			if ((LA(1) == DOT) && (LA(2) == LITERAL_class)) {
				RefJavaAST tmp174_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp174_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp174_AST));
				}
				match(DOT);
				RefJavaAST tmp175_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp175_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp175_AST));
				}
				match(LITERAL_class);
			}
			else if ((_tokenSet_39.member(LA(1))) && (_tokenSet_64.member(LA(2)))) {
			}
			else {
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			
			}
			primaryExpression_AST = RefJavaAST(currentAST.root);
			break;
		}
		case NUM_INT:
		case CHAR_LITERAL:
		case STRING_LITERAL:
		case NUM_FLOAT:
		case NUM_LONG:
		case NUM_DOUBLE:
		{
			constant();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			primaryExpression_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_true:
		{
			RefJavaAST tmp176_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp176_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp176_AST));
			}
			match(LITERAL_true);
			primaryExpression_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_false:
		{
			RefJavaAST tmp177_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp177_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp177_AST));
			}
			match(LITERAL_false);
			primaryExpression_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_null:
		{
			RefJavaAST tmp178_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp178_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp178_AST));
			}
			match(LITERAL_null);
			primaryExpression_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_new:
		{
			newExpression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			primaryExpression_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_this:
		{
			RefJavaAST tmp179_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp179_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp179_AST));
			}
			match(LITERAL_this);
			primaryExpression_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_super:
		{
			RefJavaAST tmp180_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp180_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp180_AST));
			}
			match(LITERAL_super);
			primaryExpression_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LPAREN:
		{
			match(LPAREN);
			assignmentExpression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(RPAREN);
			primaryExpression_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_void:
		case LITERAL_boolean:
		case LITERAL_byte:
		case LITERAL_char:
		case LITERAL_short:
		case LITERAL_int:
		case LITERAL_float:
		case LITERAL_long:
		case LITERAL_double:
		{
			builtInType();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{ // ( ... )*
			for (;;) {
				if ((LA(1) == LBRACK)) {
					lbt = LT(1);
					if ( inputState->guessing == 0 ) {
						lbt_AST = astFactory->create(lbt);
						astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(lbt_AST));
					}
					match(LBRACK);
					if ( inputState->guessing==0 ) {
#line 907 "java.g"
						lbt_AST->setType(ARRAY_DECLARATOR);
#line 5429 "JavaRecognizer.cpp"
					}
					match(RBRACK);
				}
				else {
					goto _loop178;
				}
				
			}
			_loop178:;
			} // ( ... )*
			RefJavaAST tmp184_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp184_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp184_AST));
			}
			match(DOT);
			RefJavaAST tmp185_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp185_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp185_AST));
			}
			match(LITERAL_class);
			primaryExpression_AST = RefJavaAST(currentAST.root);
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
			recover(ex,_tokenSet_39);
		} else {
			throw;
		}
	}
	returnAST = primaryExpression_AST;
}

/** object instantiation.
 *  Trees are built as illustrated by the following input/tree pairs:
 *
 *  new T()
 *
 *  new
 *   |
 *   T --  ELIST
 *           |
 *          arg1 -- arg2 -- .. -- argn
 *
 *  new int[]
 *
 *  new
 *   |
 *  int -- ARRAY_DECLARATOR
 *
 *  new int[] {1,2}
 *
 *  new
 *   |
 *  int -- ARRAY_DECLARATOR -- ARRAY_INIT
 *                                  |
 *                                EXPR -- EXPR
 *                                  |      |
 *                                  1      2
 *
 *  new int[3]
 *  new
 *   |
 *  int -- ARRAY_DECLARATOR
 *                |
 *              EXPR
 *                |
 *                3
 *
 *  new int[1][2]
 *
 *  new
 *   |
 *  int -- ARRAY_DECLARATOR
 *               |
 *         ARRAY_DECLARATOR -- EXPR
 *               |              |
 *             EXPR             1
 *               |
 *               2
 *
 */
void JavaRecognizer::newExpression() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST newExpression_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefJavaAST tmp186_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp186_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp186_AST));
		}
		match(LITERAL_new);
		type();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case LPAREN:
		{
			match(LPAREN);
			argList();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(RPAREN);
			{
			switch ( LA(1)) {
			case LCURLY:
			{
				classBlock();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case SEMI:
			case LBRACK:
			case RBRACK:
			case DOT:
			case STAR:
			case RCURLY:
			case COMMA:
			case RPAREN:
			case ASSIGN:
			case COLON:
			case PLUS_ASSIGN:
			case MINUS_ASSIGN:
			case STAR_ASSIGN:
			case DIV_ASSIGN:
			case MOD_ASSIGN:
			case SR_ASSIGN:
			case BSR_ASSIGN:
			case SL_ASSIGN:
			case BAND_ASSIGN:
			case BXOR_ASSIGN:
			case BOR_ASSIGN:
			case QUESTION:
			case LOR:
			case LAND:
			case BOR:
			case BXOR:
			case BAND:
			case NOT_EQUAL:
			case EQUAL:
			case LT_:
			case GT:
			case LE:
			case GE:
			case LITERAL_instanceof:
			case SL:
			case SR:
			case BSR:
			case PLUS:
			case MINUS:
			case DIV:
			case MOD:
			case INC:
			case DEC:
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
		case LBRACK:
		{
			newArrayDeclarator();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{
			switch ( LA(1)) {
			case LCURLY:
			{
				arrayInitializer();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case SEMI:
			case LBRACK:
			case RBRACK:
			case DOT:
			case STAR:
			case RCURLY:
			case COMMA:
			case RPAREN:
			case ASSIGN:
			case COLON:
			case PLUS_ASSIGN:
			case MINUS_ASSIGN:
			case STAR_ASSIGN:
			case DIV_ASSIGN:
			case MOD_ASSIGN:
			case SR_ASSIGN:
			case BSR_ASSIGN:
			case SL_ASSIGN:
			case BAND_ASSIGN:
			case BXOR_ASSIGN:
			case BOR_ASSIGN:
			case QUESTION:
			case LOR:
			case LAND:
			case BOR:
			case BXOR:
			case BAND:
			case NOT_EQUAL:
			case EQUAL:
			case LT_:
			case GT:
			case LE:
			case GE:
			case LITERAL_instanceof:
			case SL:
			case SR:
			case BSR:
			case PLUS:
			case MINUS:
			case DIV:
			case MOD:
			case INC:
			case DEC:
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
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		newExpression_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_39);
		} else {
			throw;
		}
	}
	returnAST = newExpression_AST;
}

/** Match a, a.b.c refs, a.b.c(...) refs, a.b.c[], a.b.c[].class,
 *  and a.b.c.class refs.  Also this(...) and super(...).  Match
 *  this or super.
 */
void JavaRecognizer::identPrimary() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST identPrimary_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  lp = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lp_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  lbc = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lbc_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefJavaAST tmp189_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp189_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp189_AST));
		}
		match(IDENT);
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == DOT) && (LA(2) == IDENT)) {
				RefJavaAST tmp190_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp190_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp190_AST));
				}
				match(DOT);
				RefJavaAST tmp191_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp191_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp191_AST));
				}
				match(IDENT);
			}
			else {
				goto _loop181;
			}
			
		}
		_loop181:;
		} // ( ... )*
		{
		if ((LA(1) == LPAREN)) {
			{
			lp = LT(1);
			if ( inputState->guessing == 0 ) {
				lp_AST = astFactory->create(lp);
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(lp_AST));
			}
			match(LPAREN);
			if ( inputState->guessing==0 ) {
#line 933 "java.g"
				lp_AST->setType(METHOD_CALL);
#line 5755 "JavaRecognizer.cpp"
			}
			argList();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(RPAREN);
			}
		}
		else if ((LA(1) == LBRACK) && (LA(2) == RBRACK)) {
			{ // ( ... )+
			int _cnt185=0;
			for (;;) {
				if ((LA(1) == LBRACK) && (LA(2) == RBRACK)) {
					lbc = LT(1);
					if ( inputState->guessing == 0 ) {
						lbc_AST = astFactory->create(lbc);
						astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(lbc_AST));
					}
					match(LBRACK);
					if ( inputState->guessing==0 ) {
#line 935 "java.g"
						lbc_AST->setType(ARRAY_DECLARATOR);
#line 5778 "JavaRecognizer.cpp"
					}
					match(RBRACK);
				}
				else {
					if ( _cnt185>=1 ) { goto _loop185; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
				}
				
				_cnt185++;
			}
			_loop185:;
			}  // ( ... )+
		}
		else if ((_tokenSet_39.member(LA(1))) && (_tokenSet_64.member(LA(2)))) {
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
		identPrimary_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_39);
		} else {
			throw;
		}
	}
	returnAST = identPrimary_AST;
}

void JavaRecognizer::constant() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST constant_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case NUM_INT:
		{
			RefJavaAST tmp194_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp194_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp194_AST));
			}
			match(NUM_INT);
			constant_AST = RefJavaAST(currentAST.root);
			break;
		}
		case CHAR_LITERAL:
		{
			RefJavaAST tmp195_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp195_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp195_AST));
			}
			match(CHAR_LITERAL);
			constant_AST = RefJavaAST(currentAST.root);
			break;
		}
		case STRING_LITERAL:
		{
			RefJavaAST tmp196_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp196_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp196_AST));
			}
			match(STRING_LITERAL);
			constant_AST = RefJavaAST(currentAST.root);
			break;
		}
		case NUM_FLOAT:
		{
			RefJavaAST tmp197_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp197_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp197_AST));
			}
			match(NUM_FLOAT);
			constant_AST = RefJavaAST(currentAST.root);
			break;
		}
		case NUM_LONG:
		{
			RefJavaAST tmp198_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp198_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp198_AST));
			}
			match(NUM_LONG);
			constant_AST = RefJavaAST(currentAST.root);
			break;
		}
		case NUM_DOUBLE:
		{
			RefJavaAST tmp199_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp199_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp199_AST));
			}
			match(NUM_DOUBLE);
			constant_AST = RefJavaAST(currentAST.root);
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
			recover(ex,_tokenSet_39);
		} else {
			throw;
		}
	}
	returnAST = constant_AST;
}

void JavaRecognizer::newArrayDeclarator() {
	returnAST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST newArrayDeclarator_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  lb = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lb_AST = RefJavaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{ // ( ... )+
		int _cnt195=0;
		for (;;) {
			if ((LA(1) == LBRACK) && (_tokenSet_65.member(LA(2)))) {
				lb = LT(1);
				if ( inputState->guessing == 0 ) {
					lb_AST = astFactory->create(lb);
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(lb_AST));
				}
				match(LBRACK);
				if ( inputState->guessing==0 ) {
#line 1023 "java.g"
					lb_AST->setType(ARRAY_DECLARATOR);
#line 5922 "JavaRecognizer.cpp"
				}
				{
				switch ( LA(1)) {
				case LITERAL_void:
				case LITERAL_boolean:
				case LITERAL_byte:
				case LITERAL_char:
				case LITERAL_short:
				case LITERAL_int:
				case LITERAL_float:
				case LITERAL_long:
				case LITERAL_double:
				case IDENT:
				case LPAREN:
				case LITERAL_this:
				case LITERAL_super:
				case PLUS:
				case MINUS:
				case INC:
				case DEC:
				case BNOT:
				case LNOT:
				case LITERAL_true:
				case LITERAL_false:
				case LITERAL_null:
				case LITERAL_new:
				case NUM_INT:
				case CHAR_LITERAL:
				case STRING_LITERAL:
				case NUM_FLOAT:
				case NUM_LONG:
				case NUM_DOUBLE:
				{
					expression();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
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
			}
			else {
				if ( _cnt195>=1 ) { goto _loop195; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
			}
			
			_cnt195++;
		}
		_loop195:;
		}  // ( ... )+
		newArrayDeclarator_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_66);
		} else {
			throw;
		}
	}
	returnAST = newArrayDeclarator_AST;
}

void JavaRecognizer::initializeASTFactory( ANTLR_USE_NAMESPACE(antlr)ASTFactory& factory )
{
	factory.setMaxNodeType(151);
}
const char* JavaRecognizer::tokenNames[] = {
	"<0>",
	"EOF",
	"<2>",
	"NULL_TREE_LOOKAHEAD",
	"BLOCK",
	"MODIFIERS",
	"OBJBLOCK",
	"SLIST",
	"CTOR_DEF",
	"METHOD_DEF",
	"VARIABLE_DEF",
	"INSTANCE_INIT",
	"STATIC_INIT",
	"TYPE",
	"CLASS_DEF",
	"INTERFACE_DEF",
	"PACKAGE_DEF",
	"ARRAY_DECLARATOR",
	"EXTENDS_CLAUSE",
	"IMPLEMENTS_CLAUSE",
	"PARAMETERS",
	"PARAMETER_DEF",
	"LABELED_STAT",
	"TYPECAST",
	"INDEX_OP",
	"POST_INC",
	"POST_DEC",
	"METHOD_CALL",
	"EXPR",
	"ARRAY_INIT",
	"IMPORT",
	"UNARY_MINUS",
	"UNARY_PLUS",
	"CASE_GROUP",
	"ELIST",
	"FOR_INIT",
	"FOR_CONDITION",
	"FOR_ITERATOR",
	"EMPTY_STAT",
	"\"final\"",
	"\"abstract\"",
	"\"strictfp\"",
	"SUPER_CTOR_CALL",
	"CTOR_CALL",
	"\"package\"",
	"SEMI",
	"\"import\"",
	"LBRACK",
	"RBRACK",
	"\"void\"",
	"\"boolean\"",
	"\"byte\"",
	"\"char\"",
	"\"short\"",
	"\"int\"",
	"\"float\"",
	"\"long\"",
	"\"double\"",
	"IDENT",
	"DOT",
	"STAR",
	"\"private\"",
	"\"public\"",
	"\"protected\"",
	"\"static\"",
	"\"transient\"",
	"\"native\"",
	"\"threadsafe\"",
	"\"synchronized\"",
	"\"volatile\"",
	"\"class\"",
	"\"extends\"",
	"\"interface\"",
	"LCURLY",
	"RCURLY",
	"COMMA",
	"\"implements\"",
	"LPAREN",
	"RPAREN",
	"\"this\"",
	"\"super\"",
	"ASSIGN",
	"\"throws\"",
	"COLON",
	"\"if\"",
	"\"else\"",
	"\"for\"",
	"\"while\"",
	"\"do\"",
	"\"break\"",
	"\"continue\"",
	"\"return\"",
	"\"switch\"",
	"\"throw\"",
	"\"case\"",
	"\"default\"",
	"\"try\"",
	"\"finally\"",
	"\"catch\"",
	"PLUS_ASSIGN",
	"MINUS_ASSIGN",
	"STAR_ASSIGN",
	"DIV_ASSIGN",
	"MOD_ASSIGN",
	"SR_ASSIGN",
	"BSR_ASSIGN",
	"SL_ASSIGN",
	"BAND_ASSIGN",
	"BXOR_ASSIGN",
	"BOR_ASSIGN",
	"QUESTION",
	"LOR",
	"LAND",
	"BOR",
	"BXOR",
	"BAND",
	"NOT_EQUAL",
	"EQUAL",
	"LT_",
	"GT",
	"LE",
	"GE",
	"\"instanceof\"",
	"SL",
	"SR",
	"BSR",
	"PLUS",
	"MINUS",
	"DIV",
	"MOD",
	"INC",
	"DEC",
	"BNOT",
	"LNOT",
	"\"true\"",
	"\"false\"",
	"\"null\"",
	"\"new\"",
	"NUM_INT",
	"CHAR_LITERAL",
	"STRING_LITERAL",
	"NUM_FLOAT",
	"NUM_LONG",
	"NUM_DOUBLE",
	"WS",
	"SL_COMMENT",
	"ML_COMMENT",
	"ESC",
	"HEX_DIGIT",
	"VOCAB",
	"EXPONENT",
	"FLOAT_SUFFIX",
	0
};

const unsigned long JavaRecognizer::_tokenSet_0_data_[] = { 0UL, 3758105472UL, 383UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "final" "abstract" "strictfp" SEMI "private" "public" "protected" "static" 
// "transient" "native" "threadsafe" "synchronized" "volatile" "class" 
// "interface" 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_0(_tokenSet_0_data_,8);
const unsigned long JavaRecognizer::_tokenSet_1_data_[] = { 2UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_1(_tokenSet_1_data_,6);
const unsigned long JavaRecognizer::_tokenSet_2_data_[] = { 2UL, 3758121856UL, 383UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF "final" "abstract" "strictfp" SEMI "import" "private" "public" "protected" 
// "static" "transient" "native" "threadsafe" "synchronized" "volatile" 
// "class" "interface" 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_2(_tokenSet_2_data_,8);
const unsigned long JavaRecognizer::_tokenSet_3_data_[] = { 2UL, 3758105472UL, 383UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF "final" "abstract" "strictfp" SEMI "private" "public" "protected" 
// "static" "transient" "native" "threadsafe" "synchronized" "volatile" 
// "class" "interface" 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_3(_tokenSet_3_data_,8);
const unsigned long JavaRecognizer::_tokenSet_4_data_[] = { 0UL, 67215360UL, 687616UL, 4194296UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LBRACK RBRACK IDENT LCURLY RCURLY COMMA "implements" LPAREN RPAREN 
// ASSIGN COLON PLUS_ASSIGN MINUS_ASSIGN STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN 
// SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN BXOR_ASSIGN BOR_ASSIGN QUESTION 
// LOR LAND BOR BXOR BAND NOT_EQUAL EQUAL 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_4(_tokenSet_4_data_,8);
const unsigned long JavaRecognizer::_tokenSet_5_data_[] = { 0UL, 8192UL, 0UL, 0UL, 0UL, 0UL };
// SEMI 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_5(_tokenSet_5_data_,6);
const unsigned long JavaRecognizer::_tokenSet_6_data_[] = { 0UL, 3758097280UL, 63UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "final" "abstract" "strictfp" "private" "public" "protected" "static" 
// "transient" "native" "threadsafe" "synchronized" "volatile" 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_6(_tokenSet_6_data_,8);
const unsigned long JavaRecognizer::_tokenSet_7_data_[] = { 0UL, 134086656UL, 320UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "void" "boolean" "byte" "char" "short" "int" "float" "long" "double" 
// IDENT "class" "interface" 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_7(_tokenSet_7_data_,8);
const unsigned long JavaRecognizer::_tokenSet_8_data_[] = { 2UL, 3892192128UL, 4294027135UL, 3221225473UL, 65532UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF "final" "abstract" "strictfp" SEMI "void" "boolean" "byte" "char" 
// "short" "int" "float" "long" "double" IDENT "private" "public" "protected" 
// "static" "transient" "native" "threadsafe" "synchronized" "volatile" 
// "class" "interface" LCURLY RCURLY LPAREN "this" "super" "if" "else" 
// "for" "while" "do" "break" "continue" "return" "switch" "throw" "case" 
// "default" "try" PLUS MINUS INC DEC BNOT LNOT "true" "false" "null" "new" 
// NUM_INT CHAR_LITERAL STRING_LITERAL NUM_FLOAT NUM_LONG NUM_DOUBLE 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_8(_tokenSet_8_data_,12);
const unsigned long JavaRecognizer::_tokenSet_9_data_[] = { 2UL, 3892192128UL, 1919UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF "final" "abstract" "strictfp" SEMI "void" "boolean" "byte" "char" 
// "short" "int" "float" "long" "double" IDENT "private" "public" "protected" 
// "static" "transient" "native" "threadsafe" "synchronized" "volatile" 
// "class" "interface" LCURLY RCURLY 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_9(_tokenSet_9_data_,8);
const unsigned long JavaRecognizer::_tokenSet_10_data_[] = { 0UL, 67182592UL, 674816UL, 4194296UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RBRACK IDENT RCURLY COMMA RPAREN ASSIGN COLON PLUS_ASSIGN MINUS_ASSIGN 
// STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN 
// BXOR_ASSIGN BOR_ASSIGN QUESTION LOR LAND BOR BXOR BAND NOT_EQUAL EQUAL 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_10(_tokenSet_10_data_,8);
const unsigned long JavaRecognizer::_tokenSet_11_data_[] = { 0UL, 201433088UL, 683008UL, 4194296UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LBRACK RBRACK IDENT DOT RCURLY COMMA LPAREN RPAREN ASSIGN COLON 
// PLUS_ASSIGN MINUS_ASSIGN STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN 
// BSR_ASSIGN SL_ASSIGN BAND_ASSIGN BXOR_ASSIGN BOR_ASSIGN QUESTION LOR 
// LAND BOR BXOR BAND NOT_EQUAL EQUAL 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_11(_tokenSet_11_data_,8);
const unsigned long JavaRecognizer::_tokenSet_12_data_[] = { 0UL, 32768UL, 8192UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LBRACK LPAREN 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_12(_tokenSet_12_data_,8);
const unsigned long JavaRecognizer::_tokenSet_13_data_[] = { 0UL, 3892183936UL, 383UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "final" "abstract" "strictfp" "void" "boolean" "byte" "char" "short" 
// "int" "float" "long" "double" IDENT "private" "public" "protected" "static" 
// "transient" "native" "threadsafe" "synchronized" "volatile" "class" 
// "interface" 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_13(_tokenSet_13_data_,8);
const unsigned long JavaRecognizer::_tokenSet_14_data_[] = { 0UL, 0UL, 4608UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LCURLY "implements" 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_14(_tokenSet_14_data_,8);
const unsigned long JavaRecognizer::_tokenSet_15_data_[] = { 0UL, 0UL, 512UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LCURLY 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_15(_tokenSet_15_data_,8);
const unsigned long JavaRecognizer::_tokenSet_16_data_[] = { 2UL, 4294943616UL, 4294700927UL, 4294967289UL, 65535UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF "final" "abstract" "strictfp" SEMI LBRACK RBRACK "void" "boolean" 
// "byte" "char" "short" "int" "float" "long" "double" IDENT DOT STAR "private" 
// "public" "protected" "static" "transient" "native" "threadsafe" "synchronized" 
// "volatile" "class" "interface" LCURLY RCURLY COMMA LPAREN RPAREN "this" 
// "super" ASSIGN COLON "if" "else" "for" "while" "do" "break" "continue" 
// "return" "switch" "throw" "case" "default" "try" PLUS_ASSIGN MINUS_ASSIGN 
// STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN 
// BXOR_ASSIGN BOR_ASSIGN QUESTION LOR LAND BOR BXOR BAND NOT_EQUAL EQUAL 
// LT_ GT LE GE "instanceof" SL SR BSR PLUS MINUS DIV MOD INC DEC BNOT 
// LNOT "true" "false" "null" "new" NUM_INT CHAR_LITERAL STRING_LITERAL 
// NUM_FLOAT NUM_LONG NUM_DOUBLE 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_16(_tokenSet_16_data_,12);
const unsigned long JavaRecognizer::_tokenSet_17_data_[] = { 0UL, 4026434432UL, 8575UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "final" "abstract" "strictfp" LBRACK "void" "boolean" "byte" "char" 
// "short" "int" "float" "long" "double" IDENT DOT "private" "public" "protected" 
// "static" "transient" "native" "threadsafe" "synchronized" "volatile" 
// "class" "interface" LPAREN 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_17(_tokenSet_17_data_,8);
const unsigned long JavaRecognizer::_tokenSet_18_data_[] = { 0UL, 201359360UL, 0UL, 0UL, 0UL, 0UL };
// LBRACK IDENT DOT 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_18(_tokenSet_18_data_,6);
const unsigned long JavaRecognizer::_tokenSet_19_data_[] = { 0UL, 40960UL, 133120UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LBRACK COMMA ASSIGN 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_19(_tokenSet_19_data_,8);
const unsigned long JavaRecognizer::_tokenSet_20_data_[] = { 0UL, 3892192128UL, 1919UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "final" "abstract" "strictfp" SEMI "void" "boolean" "byte" "char" "short" 
// "int" "float" "long" "double" IDENT "private" "public" "protected" "static" 
// "transient" "native" "threadsafe" "synchronized" "volatile" "class" 
// "interface" LCURLY RCURLY 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_20(_tokenSet_20_data_,8);
const unsigned long JavaRecognizer::_tokenSet_21_data_[] = { 0UL, 3892192128UL, 1070704255UL, 3221225473UL, 65532UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "final" "abstract" "strictfp" SEMI "void" "boolean" "byte" "char" "short" 
// "int" "float" "long" "double" IDENT "private" "public" "protected" "static" 
// "transient" "native" "threadsafe" "synchronized" "volatile" "class" 
// LCURLY RCURLY LPAREN "this" "super" "if" "for" "while" "do" "break" 
// "continue" "return" "switch" "throw" "try" PLUS MINUS INC DEC BNOT LNOT 
// "true" "false" "null" "new" NUM_INT CHAR_LITERAL STRING_LITERAL NUM_FLOAT 
// NUM_LONG NUM_DOUBLE 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_21(_tokenSet_21_data_,12);
const unsigned long JavaRecognizer::_tokenSet_22_data_[] = { 0UL, 4294878080UL, 1071359871UL, 4294967289UL, 65535UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "final" "abstract" "strictfp" SEMI LBRACK "void" "boolean" "byte" "char" 
// "short" "int" "float" "long" "double" IDENT DOT STAR "private" "public" 
// "protected" "static" "transient" "native" "threadsafe" "synchronized" 
// "volatile" "class" "interface" LCURLY RCURLY LPAREN "this" "super" ASSIGN 
// COLON "if" "for" "while" "do" "break" "continue" "return" "switch" "throw" 
// "try" PLUS_ASSIGN MINUS_ASSIGN STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN 
// BSR_ASSIGN SL_ASSIGN BAND_ASSIGN BXOR_ASSIGN BOR_ASSIGN QUESTION LOR 
// LAND BOR BXOR BAND NOT_EQUAL EQUAL LT_ GT LE GE "instanceof" SL SR BSR 
// PLUS MINUS DIV MOD INC DEC BNOT LNOT "true" "false" "null" "new" NUM_INT 
// CHAR_LITERAL STRING_LITERAL NUM_FLOAT NUM_LONG NUM_DOUBLE 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_22(_tokenSet_22_data_,12);
const unsigned long JavaRecognizer::_tokenSet_23_data_[] = { 0UL, 3892192128UL, 1070703231UL, 3221225473UL, 65532UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "final" "abstract" "strictfp" SEMI "void" "boolean" "byte" "char" "short" 
// "int" "float" "long" "double" IDENT "private" "public" "protected" "static" 
// "transient" "native" "threadsafe" "synchronized" "volatile" "class" 
// LCURLY LPAREN "this" "super" "if" "for" "while" "do" "break" "continue" 
// "return" "switch" "throw" "try" PLUS MINUS INC DEC BNOT LNOT "true" 
// "false" "null" "new" NUM_INT CHAR_LITERAL STRING_LITERAL NUM_FLOAT NUM_LONG 
// NUM_DOUBLE 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_23(_tokenSet_23_data_,12);
const unsigned long JavaRecognizer::_tokenSet_24_data_[] = { 0UL, 0UL, 16384UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// RPAREN 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_24(_tokenSet_24_data_,8);
const unsigned long JavaRecognizer::_tokenSet_25_data_[] = { 0UL, 8192UL, 412160UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LCURLY COMMA RPAREN ASSIGN "throws" 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_25(_tokenSet_25_data_,8);
const unsigned long JavaRecognizer::_tokenSet_26_data_[] = { 0UL, 8192UL, 512UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LCURLY 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_26(_tokenSet_26_data_,8);
const unsigned long JavaRecognizer::_tokenSet_27_data_[] = { 0UL, 3892192128UL, 4294027135UL, 3221225479UL, 65532UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "final" "abstract" "strictfp" SEMI "void" "boolean" "byte" "char" "short" 
// "int" "float" "long" "double" IDENT "private" "public" "protected" "static" 
// "transient" "native" "threadsafe" "synchronized" "volatile" "class" 
// "interface" LCURLY RCURLY LPAREN "this" "super" "if" "else" "for" "while" 
// "do" "break" "continue" "return" "switch" "throw" "case" "default" "try" 
// "finally" "catch" PLUS MINUS INC DEC BNOT LNOT "true" "false" "null" 
// "new" NUM_INT CHAR_LITERAL STRING_LITERAL NUM_FLOAT NUM_LONG NUM_DOUBLE 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_27(_tokenSet_27_data_,12);
const unsigned long JavaRecognizer::_tokenSet_28_data_[] = { 0UL, 3892192128UL, 4294026879UL, 3221225473UL, 65532UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "final" "abstract" "strictfp" SEMI "void" "boolean" "byte" "char" "short" 
// "int" "float" "long" "double" IDENT "private" "public" "protected" "static" 
// "transient" "native" "threadsafe" "synchronized" "volatile" "class" 
// LCURLY RCURLY LPAREN "this" "super" "if" "else" "for" "while" "do" "break" 
// "continue" "return" "switch" "throw" "case" "default" "try" PLUS MINUS 
// INC DEC BNOT LNOT "true" "false" "null" "new" NUM_INT CHAR_LITERAL STRING_LITERAL 
// NUM_FLOAT NUM_LONG NUM_DOUBLE 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_28(_tokenSet_28_data_,12);
const unsigned long JavaRecognizer::_tokenSet_29_data_[] = { 0UL, 4294878080UL, 4294682495UL, 4294967295UL, 65535UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "final" "abstract" "strictfp" SEMI LBRACK "void" "boolean" "byte" "char" 
// "short" "int" "float" "long" "double" IDENT DOT STAR "private" "public" 
// "protected" "static" "transient" "native" "threadsafe" "synchronized" 
// "volatile" "class" "interface" LCURLY RCURLY LPAREN "this" "super" ASSIGN 
// COLON "if" "else" "for" "while" "do" "break" "continue" "return" "switch" 
// "throw" "case" "default" "try" "finally" "catch" PLUS_ASSIGN MINUS_ASSIGN 
// STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN 
// BXOR_ASSIGN BOR_ASSIGN QUESTION LOR LAND BOR BXOR BAND NOT_EQUAL EQUAL 
// LT_ GT LE GE "instanceof" SL SR BSR PLUS MINUS DIV MOD INC DEC BNOT 
// LNOT "true" "false" "null" "new" NUM_INT CHAR_LITERAL STRING_LITERAL 
// NUM_FLOAT NUM_LONG NUM_DOUBLE 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_29(_tokenSet_29_data_,12);
const unsigned long JavaRecognizer::_tokenSet_30_data_[] = { 0UL, 3892183936UL, 63UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "final" "abstract" "strictfp" "void" "boolean" "byte" "char" "short" 
// "int" "float" "long" "double" IDENT "private" "public" "protected" "static" 
// "transient" "native" "threadsafe" "synchronized" "volatile" 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_30(_tokenSet_30_data_,8);
const unsigned long JavaRecognizer::_tokenSet_31_data_[] = { 0UL, 4026434432UL, 63UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "final" "abstract" "strictfp" LBRACK "void" "boolean" "byte" "char" 
// "short" "int" "float" "long" "double" IDENT DOT "private" "public" "protected" 
// "static" "transient" "native" "threadsafe" "synchronized" "volatile" 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_31(_tokenSet_31_data_,8);
const unsigned long JavaRecognizer::_tokenSet_32_data_[] = { 0UL, 134086656UL, 106496UL, 3221225472UL, 65532UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "void" "boolean" "byte" "char" "short" "int" "float" "long" "double" 
// IDENT LPAREN "this" "super" PLUS MINUS INC DEC BNOT LNOT "true" "false" 
// "null" "new" NUM_INT CHAR_LITERAL STRING_LITERAL NUM_FLOAT NUM_LONG 
// NUM_DOUBLE 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_32(_tokenSet_32_data_,12);
const unsigned long JavaRecognizer::_tokenSet_33_data_[] = { 0UL, 536780800UL, 237568UL, 4294967288UL, 65535UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LBRACK "void" "boolean" "byte" "char" "short" "int" "float" "long" 
// "double" IDENT DOT STAR LPAREN "this" "super" ASSIGN PLUS_ASSIGN MINUS_ASSIGN 
// STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN 
// BXOR_ASSIGN BOR_ASSIGN QUESTION LOR LAND BOR BXOR BAND NOT_EQUAL EQUAL 
// LT_ GT LE GE "instanceof" SL SR BSR PLUS MINUS DIV MOD INC DEC BNOT 
// LNOT "true" "false" "null" "new" NUM_INT CHAR_LITERAL STRING_LITERAL 
// NUM_FLOAT NUM_LONG NUM_DOUBLE 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_33(_tokenSet_33_data_,12);
const unsigned long JavaRecognizer::_tokenSet_34_data_[] = { 0UL, 3758097280UL, 127UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "final" "abstract" "strictfp" "private" "public" "protected" "static" 
// "transient" "native" "threadsafe" "synchronized" "volatile" "class" 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_34(_tokenSet_34_data_,8);
const unsigned long JavaRecognizer::_tokenSet_35_data_[] = { 0UL, 3825206144UL, 127UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "final" "abstract" "strictfp" IDENT "private" "public" "protected" "static" 
// "transient" "native" "threadsafe" "synchronized" "volatile" "class" 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_35(_tokenSet_35_data_,8);
const unsigned long JavaRecognizer::_tokenSet_36_data_[] = { 0UL, 8192UL, 2048UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI COMMA 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_36(_tokenSet_36_data_,8);
const unsigned long JavaRecognizer::_tokenSet_37_data_[] = { 0UL, 8192UL, 3072UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RCURLY COMMA 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_37(_tokenSet_37_data_,8);
const unsigned long JavaRecognizer::_tokenSet_38_data_[] = { 0UL, 134086656UL, 107008UL, 3221225472UL, 65532UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "void" "boolean" "byte" "char" "short" "int" "float" "long" "double" 
// IDENT LCURLY LPAREN "this" "super" PLUS MINUS INC DEC BNOT LNOT "true" 
// "false" "null" "new" NUM_INT CHAR_LITERAL STRING_LITERAL NUM_FLOAT NUM_LONG 
// NUM_DOUBLE 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_38(_tokenSet_38_data_,12);
const unsigned long JavaRecognizer::_tokenSet_39_data_[] = { 0UL, 402759680UL, 674816UL, 4294967288UL, 15UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LBRACK RBRACK DOT STAR RCURLY COMMA RPAREN ASSIGN COLON PLUS_ASSIGN 
// MINUS_ASSIGN STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN 
// SL_ASSIGN BAND_ASSIGN BXOR_ASSIGN BOR_ASSIGN QUESTION LOR LAND BOR BXOR 
// BAND NOT_EQUAL EQUAL LT_ GT LE GE "instanceof" SL SR BSR PLUS MINUS 
// DIV MOD INC DEC 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_39(_tokenSet_39_data_,12);
const unsigned long JavaRecognizer::_tokenSet_40_data_[] = { 0UL, 73728UL, 543744UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RBRACK RCURLY COMMA RPAREN COLON 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_40(_tokenSet_40_data_,8);
const unsigned long JavaRecognizer::_tokenSet_41_data_[] = { 0UL, 0UL, 18432UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// COMMA RPAREN 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_41(_tokenSet_41_data_,8);
const unsigned long JavaRecognizer::_tokenSet_42_data_[] = { 0UL, 134086656UL, 0UL, 0UL, 0UL, 0UL };
// "void" "boolean" "byte" "char" "short" "int" "float" "long" "double" 
// IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_42(_tokenSet_42_data_,6);
const unsigned long JavaRecognizer::_tokenSet_43_data_[] = { 0UL, 536780800UL, 239616UL, 4294967288UL, 65535UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LBRACK "void" "boolean" "byte" "char" "short" "int" "float" "long" 
// "double" IDENT DOT STAR COMMA LPAREN "this" "super" ASSIGN PLUS_ASSIGN 
// MINUS_ASSIGN STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN 
// SL_ASSIGN BAND_ASSIGN BXOR_ASSIGN BOR_ASSIGN QUESTION LOR LAND BOR BXOR 
// BAND NOT_EQUAL EQUAL LT_ GT LE GE "instanceof" SL SR BSR PLUS MINUS 
// DIV MOD INC DEC BNOT LNOT "true" "false" "null" "new" NUM_INT CHAR_LITERAL 
// STRING_LITERAL NUM_FLOAT NUM_LONG NUM_DOUBLE 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_43(_tokenSet_43_data_,12);
const unsigned long JavaRecognizer::_tokenSet_44_data_[] = { 0UL, 134086656UL, 630784UL, 3221225472UL, 65532UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "void" "boolean" "byte" "char" "short" "int" "float" "long" "double" 
// IDENT LPAREN "this" "super" COLON PLUS MINUS INC DEC BNOT LNOT "true" 
// "false" "null" "new" NUM_INT CHAR_LITERAL STRING_LITERAL NUM_FLOAT NUM_LONG 
// NUM_DOUBLE 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_44(_tokenSet_44_data_,12);
const unsigned long JavaRecognizer::_tokenSet_45_data_[] = { 0UL, 0UL, 3221226496UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// RCURLY "case" "default" 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_45(_tokenSet_45_data_,8);
const unsigned long JavaRecognizer::_tokenSet_46_data_[] = { 0UL, 3892192128UL, 4291929727UL, 3221225473UL, 65532UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "final" "abstract" "strictfp" SEMI "void" "boolean" "byte" "char" "short" 
// "int" "float" "long" "double" IDENT "private" "public" "protected" "static" 
// "transient" "native" "threadsafe" "synchronized" "volatile" "class" 
// LCURLY RCURLY LPAREN "this" "super" "if" "for" "while" "do" "break" 
// "continue" "return" "switch" "throw" "case" "default" "try" PLUS MINUS 
// INC DEC BNOT LNOT "true" "false" "null" "new" NUM_INT CHAR_LITERAL STRING_LITERAL 
// NUM_FLOAT NUM_LONG NUM_DOUBLE 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_46(_tokenSet_46_data_,12);
const unsigned long JavaRecognizer::_tokenSet_47_data_[] = { 0UL, 8192UL, 16384UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RPAREN 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_47(_tokenSet_47_data_,8);
const unsigned long JavaRecognizer::_tokenSet_48_data_[] = { 0UL, 3892192128UL, 4294026879UL, 3221225479UL, 65532UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "final" "abstract" "strictfp" SEMI "void" "boolean" "byte" "char" "short" 
// "int" "float" "long" "double" IDENT "private" "public" "protected" "static" 
// "transient" "native" "threadsafe" "synchronized" "volatile" "class" 
// LCURLY RCURLY LPAREN "this" "super" "if" "else" "for" "while" "do" "break" 
// "continue" "return" "switch" "throw" "case" "default" "try" "finally" 
// "catch" PLUS MINUS INC DEC BNOT LNOT "true" "false" "null" "new" NUM_INT 
// CHAR_LITERAL STRING_LITERAL NUM_FLOAT NUM_LONG NUM_DOUBLE 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_48(_tokenSet_48_data_,12);
const unsigned long JavaRecognizer::_tokenSet_49_data_[] = { 0UL, 73728UL, 674816UL, 16376UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RBRACK RCURLY COMMA RPAREN ASSIGN COLON PLUS_ASSIGN MINUS_ASSIGN 
// STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN 
// BXOR_ASSIGN BOR_ASSIGN 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_49(_tokenSet_49_data_,8);
const unsigned long JavaRecognizer::_tokenSet_50_data_[] = { 0UL, 73728UL, 674816UL, 32760UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RBRACK RCURLY COMMA RPAREN ASSIGN COLON PLUS_ASSIGN MINUS_ASSIGN 
// STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN 
// BXOR_ASSIGN BOR_ASSIGN QUESTION 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_50(_tokenSet_50_data_,8);
const unsigned long JavaRecognizer::_tokenSet_51_data_[] = { 0UL, 73728UL, 674816UL, 65528UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RBRACK RCURLY COMMA RPAREN ASSIGN COLON PLUS_ASSIGN MINUS_ASSIGN 
// STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN 
// BXOR_ASSIGN BOR_ASSIGN QUESTION LOR 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_51(_tokenSet_51_data_,8);
const unsigned long JavaRecognizer::_tokenSet_52_data_[] = { 0UL, 73728UL, 674816UL, 131064UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RBRACK RCURLY COMMA RPAREN ASSIGN COLON PLUS_ASSIGN MINUS_ASSIGN 
// STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN 
// BXOR_ASSIGN BOR_ASSIGN QUESTION LOR LAND 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_52(_tokenSet_52_data_,8);
const unsigned long JavaRecognizer::_tokenSet_53_data_[] = { 0UL, 73728UL, 674816UL, 262136UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RBRACK RCURLY COMMA RPAREN ASSIGN COLON PLUS_ASSIGN MINUS_ASSIGN 
// STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN 
// BXOR_ASSIGN BOR_ASSIGN QUESTION LOR LAND BOR 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_53(_tokenSet_53_data_,8);
const unsigned long JavaRecognizer::_tokenSet_54_data_[] = { 0UL, 73728UL, 674816UL, 524280UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RBRACK RCURLY COMMA RPAREN ASSIGN COLON PLUS_ASSIGN MINUS_ASSIGN 
// STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN 
// BXOR_ASSIGN BOR_ASSIGN QUESTION LOR LAND BOR BXOR 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_54(_tokenSet_54_data_,8);
const unsigned long JavaRecognizer::_tokenSet_55_data_[] = { 0UL, 73728UL, 674816UL, 1048568UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RBRACK RCURLY COMMA RPAREN ASSIGN COLON PLUS_ASSIGN MINUS_ASSIGN 
// STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN 
// BXOR_ASSIGN BOR_ASSIGN QUESTION LOR LAND BOR BXOR BAND 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_55(_tokenSet_55_data_,8);
const unsigned long JavaRecognizer::_tokenSet_56_data_[] = { 0UL, 73728UL, 674816UL, 4194296UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RBRACK RCURLY COMMA RPAREN ASSIGN COLON PLUS_ASSIGN MINUS_ASSIGN 
// STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN 
// BXOR_ASSIGN BOR_ASSIGN QUESTION LOR LAND BOR BXOR BAND NOT_EQUAL EQUAL 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_56(_tokenSet_56_data_,8);
const unsigned long JavaRecognizer::_tokenSet_57_data_[] = { 0UL, 73728UL, 674816UL, 134217720UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RBRACK RCURLY COMMA RPAREN ASSIGN COLON PLUS_ASSIGN MINUS_ASSIGN 
// STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN 
// BXOR_ASSIGN BOR_ASSIGN QUESTION LOR LAND BOR BXOR BAND NOT_EQUAL EQUAL 
// LT_ GT LE GE "instanceof" 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_57(_tokenSet_57_data_,8);
const unsigned long JavaRecognizer::_tokenSet_58_data_[] = { 0UL, 73728UL, 674816UL, 1073741816UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RBRACK RCURLY COMMA RPAREN ASSIGN COLON PLUS_ASSIGN MINUS_ASSIGN 
// STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN 
// BXOR_ASSIGN BOR_ASSIGN QUESTION LOR LAND BOR BXOR BAND NOT_EQUAL EQUAL 
// LT_ GT LE GE "instanceof" SL SR BSR 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_58(_tokenSet_58_data_,8);
const unsigned long JavaRecognizer::_tokenSet_59_data_[] = { 0UL, 268435456UL, 0UL, 0UL, 3UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// STAR DIV MOD 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_59(_tokenSet_59_data_,12);
const unsigned long JavaRecognizer::_tokenSet_60_data_[] = { 0UL, 73728UL, 674816UL, 4294967288UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RBRACK RCURLY COMMA RPAREN ASSIGN COLON PLUS_ASSIGN MINUS_ASSIGN 
// STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN 
// BXOR_ASSIGN BOR_ASSIGN QUESTION LOR LAND BOR BXOR BAND NOT_EQUAL EQUAL 
// LT_ GT LE GE "instanceof" SL SR BSR PLUS MINUS 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_60(_tokenSet_60_data_,8);
const unsigned long JavaRecognizer::_tokenSet_61_data_[] = { 0UL, 268509184UL, 674816UL, 4294967288UL, 3UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RBRACK STAR RCURLY COMMA RPAREN ASSIGN COLON PLUS_ASSIGN MINUS_ASSIGN 
// STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN 
// BXOR_ASSIGN BOR_ASSIGN QUESTION LOR LAND BOR BXOR BAND NOT_EQUAL EQUAL 
// LT_ GT LE GE "instanceof" SL SR BSR PLUS MINUS DIV MOD 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_61(_tokenSet_61_data_,12);
const unsigned long JavaRecognizer::_tokenSet_62_data_[] = { 0UL, 134086656UL, 106496UL, 0UL, 65472UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "void" "boolean" "byte" "char" "short" "int" "float" "long" "double" 
// IDENT LPAREN "this" "super" "true" "false" "null" "new" NUM_INT CHAR_LITERAL 
// STRING_LITERAL NUM_FLOAT NUM_LONG NUM_DOUBLE 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_62(_tokenSet_62_data_,12);
const unsigned long JavaRecognizer::_tokenSet_63_data_[] = { 0UL, 536846336UL, 781312UL, 4294967288UL, 65535UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LBRACK RBRACK "void" "boolean" "byte" "char" "short" "int" "float" 
// "long" "double" IDENT DOT STAR RCURLY COMMA LPAREN RPAREN "this" "super" 
// ASSIGN COLON PLUS_ASSIGN MINUS_ASSIGN STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN 
// SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN BXOR_ASSIGN BOR_ASSIGN QUESTION 
// LOR LAND BOR BXOR BAND NOT_EQUAL EQUAL LT_ GT LE GE "instanceof" SL 
// SR BSR PLUS MINUS DIV MOD INC DEC BNOT LNOT "true" "false" "null" "new" 
// NUM_INT CHAR_LITERAL STRING_LITERAL NUM_FLOAT NUM_LONG NUM_DOUBLE 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_63(_tokenSet_63_data_,12);
const unsigned long JavaRecognizer::_tokenSet_64_data_[] = { 0UL, 4294943616UL, 4294700927UL, 4294967289UL, 65535UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "final" "abstract" "strictfp" SEMI LBRACK RBRACK "void" "boolean" "byte" 
// "char" "short" "int" "float" "long" "double" IDENT DOT STAR "private" 
// "public" "protected" "static" "transient" "native" "threadsafe" "synchronized" 
// "volatile" "class" "interface" LCURLY RCURLY COMMA LPAREN RPAREN "this" 
// "super" ASSIGN COLON "if" "else" "for" "while" "do" "break" "continue" 
// "return" "switch" "throw" "case" "default" "try" PLUS_ASSIGN MINUS_ASSIGN 
// STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN 
// BXOR_ASSIGN BOR_ASSIGN QUESTION LOR LAND BOR BXOR BAND NOT_EQUAL EQUAL 
// LT_ GT LE GE "instanceof" SL SR BSR PLUS MINUS DIV MOD INC DEC BNOT 
// LNOT "true" "false" "null" "new" NUM_INT CHAR_LITERAL STRING_LITERAL 
// NUM_FLOAT NUM_LONG NUM_DOUBLE 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_64(_tokenSet_64_data_,12);
const unsigned long JavaRecognizer::_tokenSet_65_data_[] = { 0UL, 134152192UL, 106496UL, 3221225472UL, 65532UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// RBRACK "void" "boolean" "byte" "char" "short" "int" "float" "long" "double" 
// IDENT LPAREN "this" "super" PLUS MINUS INC DEC BNOT LNOT "true" "false" 
// "null" "new" NUM_INT CHAR_LITERAL STRING_LITERAL NUM_FLOAT NUM_LONG 
// NUM_DOUBLE 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_65(_tokenSet_65_data_,12);
const unsigned long JavaRecognizer::_tokenSet_66_data_[] = { 0UL, 402759680UL, 675328UL, 4294967288UL, 15UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LBRACK RBRACK DOT STAR LCURLY RCURLY COMMA RPAREN ASSIGN COLON 
// PLUS_ASSIGN MINUS_ASSIGN STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN 
// BSR_ASSIGN SL_ASSIGN BAND_ASSIGN BXOR_ASSIGN BOR_ASSIGN QUESTION LOR 
// LAND BOR BXOR BAND NOT_EQUAL EQUAL LT_ GT LE GE "instanceof" SL SR BSR 
// PLUS MINUS DIV MOD INC DEC 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_66(_tokenSet_66_data_,12);


