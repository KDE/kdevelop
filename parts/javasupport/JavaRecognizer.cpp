/* $ANTLR 2.7.2: "java.g" -> "JavaRecognizer.cpp"$ */
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
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST compilationUnit_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LITERAL_package:
		{
			packageDefinition();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
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
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
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
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
				}
			}
			else {
				goto _loop6;
			}
			
		}
		_loop6:;
		} // ( ... )*
		match(ANTLR_USE_NAMESPACE(antlr)Token::EOF_TYPE);
		compilationUnit_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = compilationUnit_AST;
}

void JavaRecognizer::packageDefinition() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST packageDefinition_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)RefToken  p = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST p_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		p = LT(1);
		if ( inputState->guessing == 0 ) {
			p_AST = astFactory->create(p);
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(p_AST.get()));
		}
		match(LITERAL_package);
		if ( inputState->guessing==0 ) {
#line 196 "java.g"
			p_AST->setType(PACKAGE_DEF);
#line 138 "JavaRecognizer.cpp"
		}
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
		}
		match(SEMI);
		packageDefinition_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = packageDefinition_AST;
}

void JavaRecognizer::importDefinition() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST importDefinition_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)RefToken  i = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST i_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		i = LT(1);
		if ( inputState->guessing == 0 ) {
			i_AST = astFactory->create(i);
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(i_AST.get()));
		}
		match(LITERAL_import);
		if ( inputState->guessing==0 ) {
#line 202 "java.g"
			i_AST->setType(IMPORT);
#line 176 "JavaRecognizer.cpp"
		}
		identifierStar();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
		}
		match(SEMI);
		importDefinition_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = importDefinition_AST;
}

void JavaRecognizer::typeDefinition() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST typeDefinition_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	RefJavaAST m_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
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
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
				}
				break;
			}
			case LITERAL_interface:
			{
				interfaceDefinition(m_AST);
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
				}
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			typeDefinition_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case SEMI:
		{
			match(SEMI);
			typeDefinition_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
			consumeUntil(_tokenSet_3);
		} else {
			throw;
		}
	}
	returnAST = typeDefinition_AST;
}

void JavaRecognizer::identifier() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST identifier_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		RefJavaAST tmp5_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
		if ( inputState->guessing == 0 ) {
			tmp5_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp5_AST.get()));
		}
		match(IDENT);
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == DOT)) {
				RefJavaAST tmp6_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp6_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp6_AST.get()));
				}
				match(DOT);
				RefJavaAST tmp7_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp7_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp7_AST.get()));
				}
				match(IDENT);
			}
			else {
				goto _loop23;
			}
			
		}
		_loop23:;
		} // ( ... )*
		identifier_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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

void JavaRecognizer::identifierStar() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST identifierStar_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		RefJavaAST tmp8_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
		if ( inputState->guessing == 0 ) {
			tmp8_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp8_AST.get()));
		}
		match(IDENT);
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == DOT) && (LA(2) == IDENT)) {
				RefJavaAST tmp9_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp9_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp9_AST.get()));
				}
				match(DOT);
				RefJavaAST tmp10_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp10_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp10_AST.get()));
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
			RefJavaAST tmp11_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp11_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp11_AST.get()));
			}
			match(DOT);
			RefJavaAST tmp12_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp12_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp12_AST.get()));
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
		identifierStar_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = identifierStar_AST;
}

void JavaRecognizer::modifiers() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST modifiers_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		{ // ( ... )*
		for (;;) {
			if ((_tokenSet_6.member(LA(1)))) {
				modifier();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
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
			modifiers_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(MODIFIERS,"MODIFIERS").get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(modifiers_AST.get()))));
#line 426 "JavaRecognizer.cpp"
			currentAST.root = modifiers_AST;
			if ( modifiers_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) &&
				modifiers_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				  currentAST.child = modifiers_AST->getFirstChild();
			else
				currentAST.child = modifiers_AST;
			currentAST.advanceChildToEnd();
		}
		modifiers_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = modifiers_AST;
}

void JavaRecognizer::classDefinition(
	RefJavaAST modifiers
) {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST classDefinition_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	RefJavaAST sc_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	RefJavaAST ic_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	RefJavaAST cb_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		match(LITERAL_class);
		RefJavaAST tmp14_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
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
			classDefinition_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(6))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(CLASS_DEF,"CLASS_DEF").get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(modifiers.get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp14_AST.get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(sc_AST.get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(ic_AST.get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(cb_AST.get()))));
#line 482 "JavaRecognizer.cpp"
			currentAST.root = classDefinition_AST;
			if ( classDefinition_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) &&
				classDefinition_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				  currentAST.child = classDefinition_AST->getFirstChild();
			else
				currentAST.child = classDefinition_AST;
			currentAST.advanceChildToEnd();
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
	returnAST = classDefinition_AST;
}

void JavaRecognizer::interfaceDefinition(
	RefJavaAST modifiers
) {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST interfaceDefinition_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	RefJavaAST ie_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	RefJavaAST cb_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		match(LITERAL_interface);
		RefJavaAST tmp16_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
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
			interfaceDefinition_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(5))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(INTERFACE_DEF,"INTERFACE_DEF").get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(modifiers.get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp16_AST.get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(ie_AST.get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(cb_AST.get()))));
#line 532 "JavaRecognizer.cpp"
			currentAST.root = interfaceDefinition_AST;
			if ( interfaceDefinition_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) &&
				interfaceDefinition_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				  currentAST.child = interfaceDefinition_AST->getFirstChild();
			else
				currentAST.child = interfaceDefinition_AST;
			currentAST.advanceChildToEnd();
		}
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
	returnAST = interfaceDefinition_AST;
}

/** A declaration is the creation of a reference or primitive-type variable
 *  Create a separate Type/Var tree for each var in the var list.
 */
void JavaRecognizer::declaration() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST declaration_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	RefJavaAST m_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	RefJavaAST t_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	RefJavaAST v_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
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
#line 582 "JavaRecognizer.cpp"
			currentAST.root = declaration_AST;
			if ( declaration_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) &&
				declaration_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				  currentAST.child = declaration_AST->getFirstChild();
			else
				currentAST.child = declaration_AST;
			currentAST.advanceChildToEnd();
		}
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
	returnAST = declaration_AST;
}

void JavaRecognizer::typeSpec(
	bool addImagNode
) {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST typeSpec_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		switch ( LA(1)) {
		case IDENT:
		{
			classTypeSpec(addImagNode);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			typeSpec_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			typeSpec_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
			consumeUntil(_tokenSet_10);
		} else {
			throw;
		}
	}
	returnAST = typeSpec_AST;
}

void JavaRecognizer::variableDefinitions(
	RefJavaAST mods, RefJavaAST t
) {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST variableDefinitions_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		variableDeclarator((RefJavaAST)getASTFactory()->dupTree((antlr::RefAST)mods),
				   (RefJavaAST)getASTFactory()->dupTree((antlr::RefAST)t));
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				variableDeclarator((RefJavaAST)getASTFactory()->dupTree((antlr::RefAST)mods),
					   (RefJavaAST)getASTFactory()->dupTree((antlr::RefAST)t));
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
				}
			}
			else {
				goto _loop59;
			}
			
		}
		_loop59:;
		} // ( ... )*
		variableDefinitions_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = variableDefinitions_AST;
}

void JavaRecognizer::classTypeSpec(
	bool addImagNode
) {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST classTypeSpec_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)RefToken  lb = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lb_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == LBRACK)) {
				lb = LT(1);
				if ( inputState->guessing == 0 ) {
					lb_AST = astFactory->create(lb);
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(lb_AST.get()));
				}
				match(LBRACK);
				if ( inputState->guessing==0 ) {
#line 233 "java.g"
					lb_AST->setType(ARRAY_DECLARATOR);
#line 727 "JavaRecognizer.cpp"
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
							classTypeSpec_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(TYPE,"TYPE").get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(classTypeSpec_AST.get()))));
						}
					
#line 746 "JavaRecognizer.cpp"
			currentAST.root = classTypeSpec_AST;
			if ( classTypeSpec_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) &&
				classTypeSpec_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				  currentAST.child = classTypeSpec_AST->getFirstChild();
			else
				currentAST.child = classTypeSpec_AST;
			currentAST.advanceChildToEnd();
		}
		classTypeSpec_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = classTypeSpec_AST;
}

void JavaRecognizer::builtInTypeSpec(
	bool addImagNode
) {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST builtInTypeSpec_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)RefToken  lb = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lb_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		builtInType();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == LBRACK)) {
				lb = LT(1);
				if ( inputState->guessing == 0 ) {
					lb_AST = astFactory->create(lb);
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(lb_AST.get()));
				}
				match(LBRACK);
				if ( inputState->guessing==0 ) {
#line 244 "java.g"
					lb_AST->setType(ARRAY_DECLARATOR);
#line 795 "JavaRecognizer.cpp"
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
							builtInTypeSpec_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(TYPE,"TYPE").get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(builtInTypeSpec_AST.get()))));
						}
					
#line 814 "JavaRecognizer.cpp"
			currentAST.root = builtInTypeSpec_AST;
			if ( builtInTypeSpec_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) &&
				builtInTypeSpec_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				  currentAST.child = builtInTypeSpec_AST->getFirstChild();
			else
				currentAST.child = builtInTypeSpec_AST;
			currentAST.advanceChildToEnd();
		}
		builtInTypeSpec_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = builtInTypeSpec_AST;
}

void JavaRecognizer::builtInType() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST builtInType_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		switch ( LA(1)) {
		case LITERAL_void:
		{
			RefJavaAST tmp20_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp20_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp20_AST.get()));
			}
			match(LITERAL_void);
			builtInType_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LITERAL_boolean:
		{
			RefJavaAST tmp21_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp21_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp21_AST.get()));
			}
			match(LITERAL_boolean);
			builtInType_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LITERAL_byte:
		{
			RefJavaAST tmp22_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp22_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp22_AST.get()));
			}
			match(LITERAL_byte);
			builtInType_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LITERAL_char:
		{
			RefJavaAST tmp23_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp23_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp23_AST.get()));
			}
			match(LITERAL_char);
			builtInType_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LITERAL_short:
		{
			RefJavaAST tmp24_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp24_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp24_AST.get()));
			}
			match(LITERAL_short);
			builtInType_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LITERAL_int:
		{
			RefJavaAST tmp25_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp25_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp25_AST.get()));
			}
			match(LITERAL_int);
			builtInType_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LITERAL_float:
		{
			RefJavaAST tmp26_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp26_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp26_AST.get()));
			}
			match(LITERAL_float);
			builtInType_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LITERAL_long:
		{
			RefJavaAST tmp27_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp27_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp27_AST.get()));
			}
			match(LITERAL_long);
			builtInType_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LITERAL_double:
		{
			RefJavaAST tmp28_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp28_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp28_AST.get()));
			}
			match(LITERAL_double);
			builtInType_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
			consumeUntil(_tokenSet_11);
		} else {
			throw;
		}
	}
	returnAST = builtInType_AST;
}

void JavaRecognizer::type() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST type_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		switch ( LA(1)) {
		case IDENT:
		{
			identifier();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			type_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			type_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
			consumeUntil(_tokenSet_12);
		} else {
			throw;
		}
	}
	returnAST = type_AST;
}

void JavaRecognizer::modifier() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST modifier_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		switch ( LA(1)) {
		case LITERAL_private:
		{
			RefJavaAST tmp29_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp29_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp29_AST.get()));
			}
			match(LITERAL_private);
			modifier_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LITERAL_public:
		{
			RefJavaAST tmp30_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp30_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp30_AST.get()));
			}
			match(LITERAL_public);
			modifier_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LITERAL_protected:
		{
			RefJavaAST tmp31_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp31_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp31_AST.get()));
			}
			match(LITERAL_protected);
			modifier_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LITERAL_static:
		{
			RefJavaAST tmp32_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp32_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp32_AST.get()));
			}
			match(LITERAL_static);
			modifier_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LITERAL_transient:
		{
			RefJavaAST tmp33_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp33_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp33_AST.get()));
			}
			match(LITERAL_transient);
			modifier_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case FINAL:
		{
			RefJavaAST tmp34_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp34_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp34_AST.get()));
			}
			match(FINAL);
			modifier_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case ABSTRACT:
		{
			RefJavaAST tmp35_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp35_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp35_AST.get()));
			}
			match(ABSTRACT);
			modifier_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LITERAL_native:
		{
			RefJavaAST tmp36_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp36_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp36_AST.get()));
			}
			match(LITERAL_native);
			modifier_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LITERAL_threadsafe:
		{
			RefJavaAST tmp37_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp37_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp37_AST.get()));
			}
			match(LITERAL_threadsafe);
			modifier_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LITERAL_synchronized:
		{
			RefJavaAST tmp38_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp38_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp38_AST.get()));
			}
			match(LITERAL_synchronized);
			modifier_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LITERAL_volatile:
		{
			RefJavaAST tmp39_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp39_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp39_AST.get()));
			}
			match(LITERAL_volatile);
			modifier_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case STRICTFP:
		{
			RefJavaAST tmp40_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp40_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp40_AST.get()));
			}
			match(STRICTFP);
			modifier_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = modifier_AST;
}

void JavaRecognizer::superClassClause() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST superClassClause_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	RefJavaAST id_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
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
			superClassClause_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(EXTENDS_CLAUSE,"EXTENDS_CLAUSE").get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(id_AST.get()))));
#line 1202 "JavaRecognizer.cpp"
			currentAST.root = superClassClause_AST;
			if ( superClassClause_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) &&
				superClassClause_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				  currentAST.child = superClassClause_AST->getFirstChild();
			else
				currentAST.child = superClassClause_AST;
			currentAST.advanceChildToEnd();
		}
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
	returnAST = superClassClause_AST;
}

void JavaRecognizer::implementsClause() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST implementsClause_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)RefToken  i = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST i_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			{ // ( ... )*
			for (;;) {
				if ((LA(1) == COMMA)) {
					match(COMMA);
					identifier();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
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
			implementsClause_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(IMPLEMENTS_CLAUSE,"IMPLEMENTS_CLAUSE").get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(implementsClause_AST.get()))));
#line 1277 "JavaRecognizer.cpp"
			currentAST.root = implementsClause_AST;
			if ( implementsClause_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) &&
				implementsClause_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				  currentAST.child = implementsClause_AST->getFirstChild();
			else
				currentAST.child = implementsClause_AST;
			currentAST.advanceChildToEnd();
		}
		implementsClause_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = implementsClause_AST;
}

void JavaRecognizer::classBlock() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST classBlock_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
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
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
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
			classBlock_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(OBJBLOCK,"OBJBLOCK").get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(classBlock_AST.get()))));
#line 1360 "JavaRecognizer.cpp"
			currentAST.root = classBlock_AST;
			if ( classBlock_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) &&
				classBlock_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				  currentAST.child = classBlock_AST->getFirstChild();
			else
				currentAST.child = classBlock_AST;
			currentAST.advanceChildToEnd();
		}
		classBlock_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = classBlock_AST;
}

void JavaRecognizer::interfaceExtends() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST interfaceExtends_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)RefToken  e = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST e_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			{ // ( ... )*
			for (;;) {
				if ((LA(1) == COMMA)) {
					match(COMMA);
					identifier();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
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
			interfaceExtends_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(EXTENDS_CLAUSE,"EXTENDS_CLAUSE").get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(interfaceExtends_AST.get()))));
#line 1436 "JavaRecognizer.cpp"
			currentAST.root = interfaceExtends_AST;
			if ( interfaceExtends_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) &&
				interfaceExtends_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				  currentAST.child = interfaceExtends_AST->getFirstChild();
			else
				currentAST.child = interfaceExtends_AST;
			currentAST.advanceChildToEnd();
		}
		interfaceExtends_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = interfaceExtends_AST;
}

void JavaRecognizer::field() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST field_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	RefJavaAST mods_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	RefJavaAST h_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	RefJavaAST s_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	RefJavaAST cd_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	RefJavaAST id_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	RefJavaAST t_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	RefJavaAST param_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	RefJavaAST rt_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	RefJavaAST tc_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	RefJavaAST s2_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	RefJavaAST v_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	RefJavaAST s3_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	RefJavaAST s4_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
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
#line 1495 "JavaRecognizer.cpp"
					currentAST.root = field_AST;
					if ( field_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) &&
						field_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
#line 1516 "JavaRecognizer.cpp"
					currentAST.root = field_AST;
					if ( field_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) &&
						field_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
						field_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(4))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(CTOR_DEF,"CTOR_DEF").get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(mods_AST.get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(h_AST.get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(s_AST.get()))));
#line 1541 "JavaRecognizer.cpp"
						currentAST.root = field_AST;
						if ( field_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) &&
							field_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
						RefJavaAST tmp47_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
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
							RefJavaAST tmp50_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
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
							field_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(7))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(METHOD_DEF,"METHOD_DEF").get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(mods_AST.get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(TYPE,"TYPE").get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(rt_AST.get())))).get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp47_AST.get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(param_AST.get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tc_AST.get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(s2_AST.get()))));
#line 1623 "JavaRecognizer.cpp"
							currentAST.root = field_AST;
							if ( field_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) &&
								field_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
						RefJavaAST tmp51_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
						if ( inputState->guessing == 0 ) {
							tmp51_AST = astFactory->create(LT(1));
						}
						match(SEMI);
						if ( inputState->guessing==0 ) {
							field_AST = RefJavaAST(currentAST.root);
#line 405 "java.g"
							field_AST = v_AST;
#line 1647 "JavaRecognizer.cpp"
							currentAST.root = field_AST;
							if ( field_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) &&
								field_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
				field_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(STATIC_INIT,"STATIC_INIT").get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(s3_AST.get()))));
#line 1679 "JavaRecognizer.cpp"
				currentAST.root = field_AST;
				if ( field_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) &&
					field_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
				field_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(INSTANCE_INIT,"INSTANCE_INIT").get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(s4_AST.get()))));
#line 1698 "JavaRecognizer.cpp"
				currentAST.root = field_AST;
				if ( field_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) &&
					field_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			consume();
			consumeUntil(_tokenSet_20);
		} else {
			throw;
		}
	}
	returnAST = field_AST;
}

void JavaRecognizer::ctorHead() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST ctorHead_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		RefJavaAST tmp53_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
		if ( inputState->guessing == 0 ) {
			tmp53_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp53_AST.get()));
		}
		match(IDENT);
		match(LPAREN);
		parameterDeclarationList();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
		}
		match(RPAREN);
		{
		switch ( LA(1)) {
		case LITERAL_throws:
		{
			throwsClause();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
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
		ctorHead_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = ctorHead_AST;
}

void JavaRecognizer::constructorBody() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST constructorBody_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)RefToken  lc = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lc_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		lc = LT(1);
		if ( inputState->guessing == 0 ) {
			lc_AST = astFactory->create(lc);
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(lc_AST.get()));
		}
		match(LCURLY);
		if ( inputState->guessing==0 ) {
#line 419 "java.g"
			lc_AST->setType(SLIST);
#line 1794 "JavaRecognizer.cpp"
		}
		{
		if ((LA(1) == LITERAL_this || LA(1) == LITERAL_super) && (LA(2) == LPAREN)) {
			explicitConstructorInvocation();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
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
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
				}
			}
			else {
				goto _loop55;
			}
			
		}
		_loop55:;
		} // ( ... )*
		match(RCURLY);
		constructorBody_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = constructorBody_AST;
}

void JavaRecognizer::parameterDeclarationList() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST parameterDeclarationList_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			{ // ( ... )*
			for (;;) {
				if ((LA(1) == COMMA)) {
					match(COMMA);
					parameterDeclaration();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
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
			parameterDeclarationList_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(PARAMETERS,"PARAMETERS").get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(parameterDeclarationList_AST.get()))));
#line 1896 "JavaRecognizer.cpp"
			currentAST.root = parameterDeclarationList_AST;
			if ( parameterDeclarationList_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) &&
				parameterDeclarationList_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				  currentAST.child = parameterDeclarationList_AST->getFirstChild();
			else
				currentAST.child = parameterDeclarationList_AST;
			currentAST.advanceChildToEnd();
		}
		parameterDeclarationList_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = parameterDeclarationList_AST;
}

void JavaRecognizer::declaratorBrackets(
	RefJavaAST typ
) {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST declaratorBrackets_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)RefToken  lb = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lb_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		if ( inputState->guessing==0 ) {
			declaratorBrackets_AST = RefJavaAST(currentAST.root);
#line 452 "java.g"
			declaratorBrackets_AST=typ;
#line 1933 "JavaRecognizer.cpp"
			currentAST.root = declaratorBrackets_AST;
			if ( declaratorBrackets_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) &&
				declaratorBrackets_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(lb_AST.get()));
				}
				match(LBRACK);
				if ( inputState->guessing==0 ) {
#line 453 "java.g"
					lb_AST->setType(ARRAY_DECLARATOR);
#line 1954 "JavaRecognizer.cpp"
				}
				match(RBRACK);
			}
			else {
				goto _loop63;
			}
			
		}
		_loop63:;
		} // ( ... )*
		declaratorBrackets_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = declaratorBrackets_AST;
}

void JavaRecognizer::throwsClause() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST throwsClause_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		RefJavaAST tmp59_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
		if ( inputState->guessing == 0 ) {
			tmp59_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp59_AST.get()));
		}
		match(LITERAL_throws);
		identifier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				identifier();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
				}
			}
			else {
				goto _loop76;
			}
			
		}
		_loop76:;
		} // ( ... )*
		throwsClause_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = throwsClause_AST;
}

void JavaRecognizer::compoundStatement() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST compoundStatement_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)RefToken  lc = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lc_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		lc = LT(1);
		if ( inputState->guessing == 0 ) {
			lc_AST = astFactory->create(lc);
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(lc_AST.get()));
		}
		match(LCURLY);
		if ( inputState->guessing==0 ) {
#line 535 "java.g"
			lc_AST->setType(SLIST);
#line 2042 "JavaRecognizer.cpp"
		}
		{ // ( ... )*
		for (;;) {
			if ((_tokenSet_23.member(LA(1)))) {
				statement();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
				}
			}
			else {
				goto _loop86;
			}
			
		}
		_loop86:;
		} // ( ... )*
		match(RCURLY);
		compoundStatement_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = compoundStatement_AST;
}

/** Catch obvious constructor calls, but not the expr.super(...) calls */
void JavaRecognizer::explicitConstructorInvocation() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST explicitConstructorInvocation_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)RefToken  lp1 = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lp1_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)RefToken  lp2 = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lp2_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		switch ( LA(1)) {
		case LITERAL_this:
		{
			match(LITERAL_this);
			lp1 = LT(1);
			if ( inputState->guessing == 0 ) {
				lp1_AST = astFactory->create(lp1);
				astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(lp1_AST.get()));
			}
			match(LPAREN);
			argList();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			match(RPAREN);
			match(SEMI);
			if ( inputState->guessing==0 ) {
#line 428 "java.g"
				lp1_AST->setType(CTOR_CALL);
#line 2104 "JavaRecognizer.cpp"
			}
			explicitConstructorInvocation_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LITERAL_super:
		{
			match(LITERAL_super);
			lp2 = LT(1);
			if ( inputState->guessing == 0 ) {
				lp2_AST = astFactory->create(lp2);
				astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(lp2_AST.get()));
			}
			match(LPAREN);
			argList();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			match(RPAREN);
			match(SEMI);
			if ( inputState->guessing==0 ) {
#line 430 "java.g"
				lp2_AST->setType(SUPER_CTOR_CALL);
#line 2127 "JavaRecognizer.cpp"
			}
			explicitConstructorInvocation_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = explicitConstructorInvocation_AST;
}

void JavaRecognizer::statement() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST statement_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	RefJavaAST m_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)RefToken  c = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST c_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)RefToken  s = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST s_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		switch ( LA(1)) {
		case LCURLY:
		{
			compoundStatement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			statement_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LITERAL_if:
		{
			RefJavaAST tmp68_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp68_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp68_AST.get()));
			}
			match(LITERAL_if);
			match(LPAREN);
			expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			match(RPAREN);
			statement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			{
			if ((LA(1) == LITERAL_else) && (_tokenSet_23.member(LA(2)))) {
				match(LITERAL_else);
				statement();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
				}
			}
			else if ((_tokenSet_28.member(LA(1))) && (_tokenSet_29.member(LA(2)))) {
			}
			else {
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			
			}
			statement_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LITERAL_for:
		{
			RefJavaAST tmp72_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp72_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp72_AST.get()));
			}
			match(LITERAL_for);
			match(LPAREN);
			forInit();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			match(SEMI);
			forCond();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			match(SEMI);
			forIter();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			match(RPAREN);
			statement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			statement_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LITERAL_while:
		{
			RefJavaAST tmp77_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp77_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp77_AST.get()));
			}
			match(LITERAL_while);
			match(LPAREN);
			expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			match(RPAREN);
			statement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			statement_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LITERAL_do:
		{
			RefJavaAST tmp80_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp80_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp80_AST.get()));
			}
			match(LITERAL_do);
			statement();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			match(LITERAL_while);
			match(LPAREN);
			expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			match(RPAREN);
			match(SEMI);
			statement_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LITERAL_break:
		{
			RefJavaAST tmp85_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp85_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp85_AST.get()));
			}
			match(LITERAL_break);
			{
			switch ( LA(1)) {
			case IDENT:
			{
				RefJavaAST tmp86_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp86_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp86_AST.get()));
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
			statement_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LITERAL_continue:
		{
			RefJavaAST tmp88_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp88_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp88_AST.get()));
			}
			match(LITERAL_continue);
			{
			switch ( LA(1)) {
			case IDENT:
			{
				RefJavaAST tmp89_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp89_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp89_AST.get()));
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
			statement_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LITERAL_return:
		{
			RefJavaAST tmp91_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp91_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp91_AST.get()));
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
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
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
			statement_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LITERAL_switch:
		{
			RefJavaAST tmp93_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp93_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp93_AST.get()));
			}
			match(LITERAL_switch);
			match(LPAREN);
			expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			match(RPAREN);
			match(LCURLY);
			{ // ( ... )*
			for (;;) {
				if ((LA(1) == LITERAL_case || LA(1) == LITERAL_default)) {
					casesGroup();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
					}
				}
				else {
					goto _loop95;
				}
				
			}
			_loop95:;
			} // ( ... )*
			match(RCURLY);
			statement_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LITERAL_try:
		{
			tryBlock();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			statement_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LITERAL_throw:
		{
			RefJavaAST tmp98_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp98_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp98_AST.get()));
			}
			match(LITERAL_throw);
			expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			match(SEMI);
			statement_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case SEMI:
		{
			s = LT(1);
			if ( inputState->guessing == 0 ) {
				s_AST = astFactory->create(s);
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(s_AST.get()));
			}
			match(SEMI);
			if ( inputState->guessing==0 ) {
#line 617 "java.g"
				s_AST->setType(EMPTY_STAT);
#line 2480 "JavaRecognizer.cpp"
			}
			statement_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
				}
				match(SEMI);
				statement_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			}
			else if ((_tokenSet_32.member(LA(1))) && (_tokenSet_33.member(LA(2)))) {
				expression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
				}
				match(SEMI);
				statement_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			}
			else if ((_tokenSet_34.member(LA(1))) && (_tokenSet_35.member(LA(2)))) {
				modifiers();
				if (inputState->guessing==0) {
					m_AST = returnAST;
				}
				classDefinition(m_AST);
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
				}
				statement_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			}
			else if ((LA(1) == IDENT) && (LA(2) == COLON)) {
				RefJavaAST tmp102_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp102_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp102_AST.get()));
				}
				match(IDENT);
				c = LT(1);
				if ( inputState->guessing == 0 ) {
					c_AST = astFactory->create(c);
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(c_AST.get()));
				}
				match(COLON);
				if ( inputState->guessing==0 ) {
#line 560 "java.g"
					c_AST->setType(LABELED_STAT);
#line 2545 "JavaRecognizer.cpp"
				}
				statement();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
				}
				statement_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			}
			else if ((LA(1) == LITERAL_synchronized) && (LA(2) == LPAREN)) {
				RefJavaAST tmp103_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp103_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp103_AST.get()));
				}
				match(LITERAL_synchronized);
				match(LPAREN);
				expression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
				}
				match(RPAREN);
				compoundStatement();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
				}
				statement_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
			consumeUntil(_tokenSet_28);
		} else {
			throw;
		}
	}
	returnAST = statement_AST;
}

void JavaRecognizer::argList() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST argList_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			break;
		}
		case RPAREN:
		{
			if ( inputState->guessing==0 ) {
				argList_AST = RefJavaAST(currentAST.root);
#line 1008 "java.g"
				argList_AST = astFactory->create(ELIST,"ELIST");
#line 2639 "JavaRecognizer.cpp"
				currentAST.root = argList_AST;
				if ( argList_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) &&
					argList_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
		argList_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = argList_AST;
}

/** Declaration of a variable.  This can be a class/instance variable,
 *   or a local variable in a method
 * It can also include possible initialization.
 */
void JavaRecognizer::variableDeclarator(
	RefJavaAST mods, RefJavaAST t
) {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST variableDeclarator_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)RefToken  id = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST id_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	RefJavaAST d_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	RefJavaAST v_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
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
			variableDeclarator_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(5))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(VARIABLE_DEF,"VARIABLE_DEF").get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(mods.get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(TYPE,"TYPE").get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(d_AST.get())))).get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(id_AST.get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(v_AST.get()))));
#line 2703 "JavaRecognizer.cpp"
			currentAST.root = variableDeclarator_AST;
			if ( variableDeclarator_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) &&
				variableDeclarator_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				  currentAST.child = variableDeclarator_AST->getFirstChild();
			else
				currentAST.child = variableDeclarator_AST;
			currentAST.advanceChildToEnd();
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
	returnAST = variableDeclarator_AST;
}

void JavaRecognizer::varInitializer() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST varInitializer_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case ASSIGN:
		{
			RefJavaAST tmp106_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp106_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp106_AST.get()));
			}
			match(ASSIGN);
			initializer();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
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
		varInitializer_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = varInitializer_AST;
}

void JavaRecognizer::initializer() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST initializer_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			initializer_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LCURLY:
		{
			arrayInitializer();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			initializer_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
			consumeUntil(_tokenSet_37);
		} else {
			throw;
		}
	}
	returnAST = initializer_AST;
}

void JavaRecognizer::arrayInitializer() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST arrayInitializer_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)RefToken  lc = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lc_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		lc = LT(1);
		if ( inputState->guessing == 0 ) {
			lc_AST = astFactory->create(lc);
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(lc_AST.get()));
		}
		match(LCURLY);
		if ( inputState->guessing==0 ) {
#line 462 "java.g"
			lc_AST->setType(ARRAY_INIT);
#line 2860 "JavaRecognizer.cpp"
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			{ // ( ... )*
			for (;;) {
				if ((LA(1) == COMMA) && (_tokenSet_38.member(LA(2)))) {
					match(COMMA);
					initializer();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
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
		arrayInitializer_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = arrayInitializer_AST;
}

void JavaRecognizer::expression() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST expression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		assignmentExpression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
		}
		if ( inputState->guessing==0 ) {
			expression_AST = RefJavaAST(currentAST.root);
#line 715 "java.g"
			expression_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(EXPR,"EXPR").get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(expression_AST.get()))));
#line 2973 "JavaRecognizer.cpp"
			currentAST.root = expression_AST;
			if ( expression_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) &&
				expression_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				  currentAST.child = expression_AST->getFirstChild();
			else
				currentAST.child = expression_AST;
			currentAST.advanceChildToEnd();
		}
		expression_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = expression_AST;
}

void JavaRecognizer::parameterDeclaration() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST parameterDeclaration_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	RefJavaAST pm_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	RefJavaAST t_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)RefToken  id = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST id_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	RefJavaAST pd_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
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
			parameterDeclaration_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(4))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(PARAMETER_DEF,"PARAMETER_DEF").get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(pm_AST.get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(TYPE,"TYPE").get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(pd_AST.get())))).get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(id_AST.get()))));
#line 3028 "JavaRecognizer.cpp"
			currentAST.root = parameterDeclaration_AST;
			if ( parameterDeclaration_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) &&
				parameterDeclaration_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				  currentAST.child = parameterDeclaration_AST->getFirstChild();
			else
				currentAST.child = parameterDeclaration_AST;
			currentAST.advanceChildToEnd();
		}
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
	returnAST = parameterDeclaration_AST;
}

void JavaRecognizer::parameterModifier() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST parameterModifier_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)RefToken  f = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST f_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case FINAL:
		{
			f = LT(1);
			if ( inputState->guessing == 0 ) {
				f_AST = astFactory->create(f);
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(f_AST.get()));
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
			parameterModifier_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(MODIFIERS,"MODIFIERS").get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(f_AST.get()))));
#line 3093 "JavaRecognizer.cpp"
			currentAST.root = parameterModifier_AST;
			if ( parameterModifier_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) &&
				parameterModifier_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				  currentAST.child = parameterModifier_AST->getFirstChild();
			else
				currentAST.child = parameterModifier_AST;
			currentAST.advanceChildToEnd();
		}
		parameterModifier_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = parameterModifier_AST;
}

void JavaRecognizer::forInit() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST forInit_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
		}
		else if ((_tokenSet_32.member(LA(1))) && (_tokenSet_43.member(LA(2)))) {
			expressionList();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
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
			forInit_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(FOR_INIT,"FOR_INIT").get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(forInit_AST.get()))));
#line 3162 "JavaRecognizer.cpp"
			currentAST.root = forInit_AST;
			if ( forInit_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) &&
				forInit_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				  currentAST.child = forInit_AST->getFirstChild();
			else
				currentAST.child = forInit_AST;
			currentAST.advanceChildToEnd();
		}
		forInit_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = forInit_AST;
}

void JavaRecognizer::forCond() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST forCond_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
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
			forCond_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(FOR_CONDITION,"FOR_CONDITION").get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(forCond_AST.get()))));
#line 3243 "JavaRecognizer.cpp"
			currentAST.root = forCond_AST;
			if ( forCond_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) &&
				forCond_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				  currentAST.child = forCond_AST->getFirstChild();
			else
				currentAST.child = forCond_AST;
			currentAST.advanceChildToEnd();
		}
		forCond_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = forCond_AST;
}

void JavaRecognizer::forIter() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST forIter_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
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
			forIter_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(FOR_ITERATOR,"FOR_ITERATOR").get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(forIter_AST.get()))));
#line 3324 "JavaRecognizer.cpp"
			currentAST.root = forIter_AST;
			if ( forIter_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) &&
				forIter_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				  currentAST.child = forIter_AST->getFirstChild();
			else
				currentAST.child = forIter_AST;
			currentAST.advanceChildToEnd();
		}
		forIter_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = forIter_AST;
}

void JavaRecognizer::casesGroup() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST casesGroup_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		{ // ( ... )+
		int _cnt98=0;
		for (;;) {
			if ((LA(1) == LITERAL_case || LA(1) == LITERAL_default) && (_tokenSet_44.member(LA(2)))) {
				aCase();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
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
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
		}
		if ( inputState->guessing==0 ) {
			casesGroup_AST = RefJavaAST(currentAST.root);
#line 632 "java.g"
			casesGroup_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(CASE_GROUP,"CASE_GROUP").get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(casesGroup_AST.get()))));
#line 3378 "JavaRecognizer.cpp"
			currentAST.root = casesGroup_AST;
			if ( casesGroup_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) &&
				casesGroup_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				  currentAST.child = casesGroup_AST->getFirstChild();
			else
				currentAST.child = casesGroup_AST;
			currentAST.advanceChildToEnd();
		}
		casesGroup_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_45);
		} else {
			throw;
		}
	}
	returnAST = casesGroup_AST;
}

void JavaRecognizer::tryBlock() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST tryBlock_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		RefJavaAST tmp110_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
		if ( inputState->guessing == 0 ) {
			tmp110_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp110_AST.get()));
		}
		match(LITERAL_try);
		compoundStatement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == LITERAL_catch)) {
				handler();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
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
		tryBlock_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = tryBlock_AST;
}

void JavaRecognizer::aCase() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST aCase_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LITERAL_case:
		{
			RefJavaAST tmp111_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp111_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp111_AST.get()));
			}
			match(LITERAL_case);
			expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			break;
		}
		case LITERAL_default:
		{
			RefJavaAST tmp112_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp112_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp112_AST.get()));
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
		aCase_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = aCase_AST;
}

void JavaRecognizer::caseSList() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST caseSList_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		{ // ( ... )*
		for (;;) {
			if ((_tokenSet_23.member(LA(1)))) {
				statement();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
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
			caseSList_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(SLIST,"SLIST").get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(caseSList_AST.get()))));
#line 3601 "JavaRecognizer.cpp"
			currentAST.root = caseSList_AST;
			if ( caseSList_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) &&
				caseSList_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				  currentAST.child = caseSList_AST->getFirstChild();
			else
				currentAST.child = caseSList_AST;
			currentAST.advanceChildToEnd();
		}
		caseSList_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_45);
		} else {
			throw;
		}
	}
	returnAST = caseSList_AST;
}

void JavaRecognizer::expressionList() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST expressionList_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				expression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
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
			expressionList_AST = RefJavaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory->create(ELIST,"ELIST").get()))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(expressionList_AST.get()))));
#line 3654 "JavaRecognizer.cpp"
			currentAST.root = expressionList_AST;
			if ( expressionList_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) &&
				expressionList_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				  currentAST.child = expressionList_AST->getFirstChild();
			else
				currentAST.child = expressionList_AST;
			currentAST.advanceChildToEnd();
		}
		expressionList_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = expressionList_AST;
}

void JavaRecognizer::handler() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST handler_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		RefJavaAST tmp115_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
		if ( inputState->guessing == 0 ) {
			tmp115_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp115_AST.get()));
		}
		match(LITERAL_catch);
		match(LPAREN);
		parameterDeclaration();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
		}
		match(RPAREN);
		compoundStatement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
		}
		handler_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_48);
		} else {
			throw;
		}
	}
	returnAST = handler_AST;
}

void JavaRecognizer::finallyClause() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST finallyClause_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		RefJavaAST tmp118_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
		if ( inputState->guessing == 0 ) {
			tmp118_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp118_AST.get()));
		}
		match(LITERAL_finally);
		compoundStatement();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
		}
		finallyClause_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = finallyClause_AST;
}

void JavaRecognizer::assignmentExpression() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST assignmentExpression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		conditionalExpression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
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
				RefJavaAST tmp119_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp119_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp119_AST.get()));
				}
				match(ASSIGN);
				break;
			}
			case PLUS_ASSIGN:
			{
				RefJavaAST tmp120_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp120_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp120_AST.get()));
				}
				match(PLUS_ASSIGN);
				break;
			}
			case MINUS_ASSIGN:
			{
				RefJavaAST tmp121_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp121_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp121_AST.get()));
				}
				match(MINUS_ASSIGN);
				break;
			}
			case STAR_ASSIGN:
			{
				RefJavaAST tmp122_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp122_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp122_AST.get()));
				}
				match(STAR_ASSIGN);
				break;
			}
			case DIV_ASSIGN:
			{
				RefJavaAST tmp123_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp123_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp123_AST.get()));
				}
				match(DIV_ASSIGN);
				break;
			}
			case MOD_ASSIGN:
			{
				RefJavaAST tmp124_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp124_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp124_AST.get()));
				}
				match(MOD_ASSIGN);
				break;
			}
			case SR_ASSIGN:
			{
				RefJavaAST tmp125_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp125_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp125_AST.get()));
				}
				match(SR_ASSIGN);
				break;
			}
			case BSR_ASSIGN:
			{
				RefJavaAST tmp126_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp126_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp126_AST.get()));
				}
				match(BSR_ASSIGN);
				break;
			}
			case SL_ASSIGN:
			{
				RefJavaAST tmp127_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp127_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp127_AST.get()));
				}
				match(SL_ASSIGN);
				break;
			}
			case BAND_ASSIGN:
			{
				RefJavaAST tmp128_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp128_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp128_AST.get()));
				}
				match(BAND_ASSIGN);
				break;
			}
			case BXOR_ASSIGN:
			{
				RefJavaAST tmp129_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp129_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp129_AST.get()));
				}
				match(BXOR_ASSIGN);
				break;
			}
			case BOR_ASSIGN:
			{
				RefJavaAST tmp130_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp130_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp130_AST.get()));
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
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
		assignmentExpression_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = assignmentExpression_AST;
}

void JavaRecognizer::conditionalExpression() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST conditionalExpression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		logicalOrExpression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
		}
		{
		switch ( LA(1)) {
		case QUESTION:
		{
			RefJavaAST tmp131_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp131_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp131_AST.get()));
			}
			match(QUESTION);
			assignmentExpression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			match(COLON);
			conditionalExpression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
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
		conditionalExpression_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = conditionalExpression_AST;
}

void JavaRecognizer::logicalOrExpression() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST logicalOrExpression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		logicalAndExpression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == LOR)) {
				RefJavaAST tmp133_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp133_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp133_AST.get()));
				}
				match(LOR);
				logicalAndExpression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
				}
			}
			else {
				goto _loop129;
			}
			
		}
		_loop129:;
		} // ( ... )*
		logicalOrExpression_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = logicalOrExpression_AST;
}

void JavaRecognizer::logicalAndExpression() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST logicalAndExpression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		inclusiveOrExpression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == LAND)) {
				RefJavaAST tmp134_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp134_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp134_AST.get()));
				}
				match(LAND);
				inclusiveOrExpression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
				}
			}
			else {
				goto _loop132;
			}
			
		}
		_loop132:;
		} // ( ... )*
		logicalAndExpression_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = logicalAndExpression_AST;
}

void JavaRecognizer::inclusiveOrExpression() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST inclusiveOrExpression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		exclusiveOrExpression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == BOR)) {
				RefJavaAST tmp135_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp135_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp135_AST.get()));
				}
				match(BOR);
				exclusiveOrExpression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
				}
			}
			else {
				goto _loop135;
			}
			
		}
		_loop135:;
		} // ( ... )*
		inclusiveOrExpression_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = inclusiveOrExpression_AST;
}

void JavaRecognizer::exclusiveOrExpression() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST exclusiveOrExpression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		andExpression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == BXOR)) {
				RefJavaAST tmp136_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp136_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp136_AST.get()));
				}
				match(BXOR);
				andExpression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
				}
			}
			else {
				goto _loop138;
			}
			
		}
		_loop138:;
		} // ( ... )*
		exclusiveOrExpression_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = exclusiveOrExpression_AST;
}

void JavaRecognizer::andExpression() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST andExpression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		equalityExpression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == BAND)) {
				RefJavaAST tmp137_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp137_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp137_AST.get()));
				}
				match(BAND);
				equalityExpression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
				}
			}
			else {
				goto _loop141;
			}
			
		}
		_loop141:;
		} // ( ... )*
		andExpression_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = andExpression_AST;
}

void JavaRecognizer::equalityExpression() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST equalityExpression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		relationalExpression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == NOT_EQUAL || LA(1) == EQUAL)) {
				{
				switch ( LA(1)) {
				case NOT_EQUAL:
				{
					RefJavaAST tmp138_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
					if ( inputState->guessing == 0 ) {
						tmp138_AST = astFactory->create(LT(1));
						astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp138_AST.get()));
					}
					match(NOT_EQUAL);
					break;
				}
				case EQUAL:
				{
					RefJavaAST tmp139_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
					if ( inputState->guessing == 0 ) {
						tmp139_AST = astFactory->create(LT(1));
						astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp139_AST.get()));
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
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
				}
			}
			else {
				goto _loop145;
			}
			
		}
		_loop145:;
		} // ( ... )*
		equalityExpression_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = equalityExpression_AST;
}

void JavaRecognizer::relationalExpression() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST relationalExpression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		shiftExpression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
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
						RefJavaAST tmp140_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
						if ( inputState->guessing == 0 ) {
							tmp140_AST = astFactory->create(LT(1));
							astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp140_AST.get()));
						}
						match(LT_);
						break;
					}
					case GT:
					{
						RefJavaAST tmp141_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
						if ( inputState->guessing == 0 ) {
							tmp141_AST = astFactory->create(LT(1));
							astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp141_AST.get()));
						}
						match(GT);
						break;
					}
					case LE:
					{
						RefJavaAST tmp142_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
						if ( inputState->guessing == 0 ) {
							tmp142_AST = astFactory->create(LT(1));
							astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp142_AST.get()));
						}
						match(LE);
						break;
					}
					case GE:
					{
						RefJavaAST tmp143_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
						if ( inputState->guessing == 0 ) {
							tmp143_AST = astFactory->create(LT(1));
							astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp143_AST.get()));
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
						astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
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
			RefJavaAST tmp144_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp144_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp144_AST.get()));
			}
			match(LITERAL_instanceof);
			typeSpec(true);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		relationalExpression_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = relationalExpression_AST;
}

void JavaRecognizer::shiftExpression() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST shiftExpression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		additiveExpression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
		}
		{ // ( ... )*
		for (;;) {
			if (((LA(1) >= SL && LA(1) <= BSR))) {
				{
				switch ( LA(1)) {
				case SL:
				{
					RefJavaAST tmp145_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
					if ( inputState->guessing == 0 ) {
						tmp145_AST = astFactory->create(LT(1));
						astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp145_AST.get()));
					}
					match(SL);
					break;
				}
				case SR:
				{
					RefJavaAST tmp146_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
					if ( inputState->guessing == 0 ) {
						tmp146_AST = astFactory->create(LT(1));
						astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp146_AST.get()));
					}
					match(SR);
					break;
				}
				case BSR:
				{
					RefJavaAST tmp147_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
					if ( inputState->guessing == 0 ) {
						tmp147_AST = astFactory->create(LT(1));
						astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp147_AST.get()));
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
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
				}
			}
			else {
				goto _loop154;
			}
			
		}
		_loop154:;
		} // ( ... )*
		shiftExpression_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = shiftExpression_AST;
}

void JavaRecognizer::additiveExpression() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST additiveExpression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		multiplicativeExpression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == PLUS || LA(1) == MINUS)) {
				{
				switch ( LA(1)) {
				case PLUS:
				{
					RefJavaAST tmp148_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
					if ( inputState->guessing == 0 ) {
						tmp148_AST = astFactory->create(LT(1));
						astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp148_AST.get()));
					}
					match(PLUS);
					break;
				}
				case MINUS:
				{
					RefJavaAST tmp149_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
					if ( inputState->guessing == 0 ) {
						tmp149_AST = astFactory->create(LT(1));
						astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp149_AST.get()));
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
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
				}
			}
			else {
				goto _loop158;
			}
			
		}
		_loop158:;
		} // ( ... )*
		additiveExpression_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = additiveExpression_AST;
}

void JavaRecognizer::multiplicativeExpression() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST multiplicativeExpression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		unaryExpression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
		}
		{ // ( ... )*
		for (;;) {
			if ((_tokenSet_59.member(LA(1)))) {
				{
				switch ( LA(1)) {
				case STAR:
				{
					RefJavaAST tmp150_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
					if ( inputState->guessing == 0 ) {
						tmp150_AST = astFactory->create(LT(1));
						astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp150_AST.get()));
					}
					match(STAR);
					break;
				}
				case DIV:
				{
					RefJavaAST tmp151_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
					if ( inputState->guessing == 0 ) {
						tmp151_AST = astFactory->create(LT(1));
						astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp151_AST.get()));
					}
					match(DIV);
					break;
				}
				case MOD:
				{
					RefJavaAST tmp152_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
					if ( inputState->guessing == 0 ) {
						tmp152_AST = astFactory->create(LT(1));
						astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp152_AST.get()));
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
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
				}
			}
			else {
				goto _loop162;
			}
			
		}
		_loop162:;
		} // ( ... )*
		multiplicativeExpression_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_60);
		} else {
			throw;
		}
	}
	returnAST = multiplicativeExpression_AST;
}

void JavaRecognizer::unaryExpression() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST unaryExpression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		switch ( LA(1)) {
		case INC:
		{
			RefJavaAST tmp153_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp153_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp153_AST.get()));
			}
			match(INC);
			unaryExpression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			unaryExpression_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case DEC:
		{
			RefJavaAST tmp154_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp154_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp154_AST.get()));
			}
			match(DEC);
			unaryExpression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			unaryExpression_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case MINUS:
		{
			RefJavaAST tmp155_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp155_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp155_AST.get()));
			}
			match(MINUS);
			if ( inputState->guessing==0 ) {
#line 812 "java.g"
				tmp155_AST->setType(UNARY_MINUS);
#line 4706 "JavaRecognizer.cpp"
			}
			unaryExpression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			unaryExpression_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case PLUS:
		{
			RefJavaAST tmp156_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp156_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp156_AST.get()));
			}
			match(PLUS);
			if ( inputState->guessing==0 ) {
#line 813 "java.g"
				tmp156_AST->setType(UNARY_PLUS);
#line 4726 "JavaRecognizer.cpp"
			}
			unaryExpression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			unaryExpression_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			unaryExpression_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
			consumeUntil(_tokenSet_61);
		} else {
			throw;
		}
	}
	returnAST = unaryExpression_AST;
}

void JavaRecognizer::unaryExpressionNotPlusMinus() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST unaryExpressionNotPlusMinus_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)RefToken  lpb = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lpb_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)RefToken  lp = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lp_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		switch ( LA(1)) {
		case BNOT:
		{
			RefJavaAST tmp157_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp157_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp157_AST.get()));
			}
			match(BNOT);
			unaryExpression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			unaryExpressionNotPlusMinus_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LNOT:
		{
			RefJavaAST tmp158_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp158_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp158_AST.get()));
			}
			match(LNOT);
			unaryExpression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			unaryExpressionNotPlusMinus_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(lpb_AST.get()));
				}
				match(LPAREN);
				if ( inputState->guessing==0 ) {
#line 829 "java.g"
					lpb_AST->setType(TYPECAST);
#line 4862 "JavaRecognizer.cpp"
				}
				builtInTypeSpec(true);
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
				}
				match(RPAREN);
				unaryExpression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
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
						astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(lp_AST.get()));
					}
					match(LPAREN);
					if ( inputState->guessing==0 ) {
#line 836 "java.g"
						lp_AST->setType(TYPECAST);
#line 4904 "JavaRecognizer.cpp"
					}
					classTypeSpec(true);
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
					}
					match(RPAREN);
					unaryExpressionNotPlusMinus();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
					}
				}
				else if ((_tokenSet_62.member(LA(1))) && (_tokenSet_63.member(LA(2)))) {
					postfixExpression();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
					}
				}
			else {
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			unaryExpressionNotPlusMinus_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
			consumeUntil(_tokenSet_61);
		} else {
			throw;
		}
	}
	returnAST = unaryExpressionNotPlusMinus_AST;
}

void JavaRecognizer::postfixExpression() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST postfixExpression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)RefToken  lp = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lp_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)RefToken  lp3 = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lp3_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)RefToken  lps = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lps_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)RefToken  lb = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lb_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)RefToken  in = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST in_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)RefToken  de = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST de_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		primaryExpression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == DOT) && (LA(2) == IDENT)) {
				RefJavaAST tmp161_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp161_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp161_AST.get()));
				}
				match(DOT);
				RefJavaAST tmp162_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp162_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp162_AST.get()));
				}
				match(IDENT);
				{
				switch ( LA(1)) {
				case LPAREN:
				{
					lp = LT(1);
					if ( inputState->guessing == 0 ) {
						lp_AST = astFactory->create(lp);
						astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(lp_AST.get()));
					}
					match(LPAREN);
					if ( inputState->guessing==0 ) {
#line 867 "java.g"
						lp_AST->setType(METHOD_CALL);
#line 4998 "JavaRecognizer.cpp"
					}
					argList();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
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
				RefJavaAST tmp164_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp164_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp164_AST.get()));
				}
				match(DOT);
				RefJavaAST tmp165_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp165_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp165_AST.get()));
				}
				match(LITERAL_this);
			}
			else if ((LA(1) == DOT) && (LA(2) == LITERAL_super)) {
				RefJavaAST tmp166_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp166_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp166_AST.get()));
				}
				match(DOT);
				RefJavaAST tmp167_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp167_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp167_AST.get()));
				}
				match(LITERAL_super);
				{
				switch ( LA(1)) {
				case LPAREN:
				{
					lp3 = LT(1);
					if ( inputState->guessing == 0 ) {
						lp3_AST = astFactory->create(lp3);
						astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(lp3_AST.get()));
					}
					match(LPAREN);
					argList();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
					}
					match(RPAREN);
					if ( inputState->guessing==0 ) {
#line 876 "java.g"
						lp3_AST->setType(SUPER_CTOR_CALL);
#line 5105 "JavaRecognizer.cpp"
					}
					break;
				}
				case DOT:
				{
					RefJavaAST tmp169_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
					if ( inputState->guessing == 0 ) {
						tmp169_AST = astFactory->create(LT(1));
						astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp169_AST.get()));
					}
					match(DOT);
					RefJavaAST tmp170_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
					if ( inputState->guessing == 0 ) {
						tmp170_AST = astFactory->create(LT(1));
						astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp170_AST.get()));
					}
					match(IDENT);
					{
					switch ( LA(1)) {
					case LPAREN:
					{
						lps = LT(1);
						if ( inputState->guessing == 0 ) {
							lps_AST = astFactory->create(lps);
							astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(lps_AST.get()));
						}
						match(LPAREN);
						if ( inputState->guessing==0 ) {
#line 878 "java.g"
							lps_AST->setType(METHOD_CALL);
#line 5136 "JavaRecognizer.cpp"
						}
						argList();
						if (inputState->guessing==0) {
							astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
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
				RefJavaAST tmp172_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp172_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp172_AST.get()));
				}
				match(DOT);
				newExpression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
				}
			}
			else if ((LA(1) == LBRACK)) {
				lb = LT(1);
				if ( inputState->guessing == 0 ) {
					lb_AST = astFactory->create(lb);
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(lb_AST.get()));
				}
				match(LBRACK);
				if ( inputState->guessing==0 ) {
#line 884 "java.g"
					lb_AST->setType(INDEX_OP);
#line 5228 "JavaRecognizer.cpp"
				}
				expression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
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
				astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(in_AST.get()));
			}
			match(INC);
			if ( inputState->guessing==0 ) {
#line 889 "java.g"
				in_AST->setType(POST_INC);
#line 5256 "JavaRecognizer.cpp"
			}
			break;
		}
		case DEC:
		{
			de = LT(1);
			if ( inputState->guessing == 0 ) {
				de_AST = astFactory->create(de);
				astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(de_AST.get()));
			}
			match(DEC);
			if ( inputState->guessing==0 ) {
#line 890 "java.g"
				de_AST->setType(POST_DEC);
#line 5271 "JavaRecognizer.cpp"
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
		postfixExpression_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_61);
		} else {
			throw;
		}
	}
	returnAST = postfixExpression_AST;
}

void JavaRecognizer::primaryExpression() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST primaryExpression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)RefToken  lbt = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lbt_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		switch ( LA(1)) {
		case IDENT:
		{
			identPrimary();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			{
			if ((LA(1) == DOT) && (LA(2) == LITERAL_class)) {
				RefJavaAST tmp174_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp174_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp174_AST.get()));
				}
				match(DOT);
				RefJavaAST tmp175_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp175_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp175_AST.get()));
				}
				match(LITERAL_class);
			}
			else if ((_tokenSet_39.member(LA(1))) && (_tokenSet_64.member(LA(2)))) {
			}
			else {
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			
			}
			primaryExpression_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			primaryExpression_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LITERAL_true:
		{
			RefJavaAST tmp176_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp176_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp176_AST.get()));
			}
			match(LITERAL_true);
			primaryExpression_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LITERAL_false:
		{
			RefJavaAST tmp177_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp177_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp177_AST.get()));
			}
			match(LITERAL_false);
			primaryExpression_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LITERAL_null:
		{
			RefJavaAST tmp178_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp178_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp178_AST.get()));
			}
			match(LITERAL_null);
			primaryExpression_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LITERAL_new:
		{
			newExpression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			primaryExpression_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LITERAL_this:
		{
			RefJavaAST tmp179_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp179_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp179_AST.get()));
			}
			match(LITERAL_this);
			primaryExpression_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LITERAL_super:
		{
			RefJavaAST tmp180_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp180_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp180_AST.get()));
			}
			match(LITERAL_super);
			primaryExpression_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case LPAREN:
		{
			match(LPAREN);
			assignmentExpression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			match(RPAREN);
			primaryExpression_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			{ // ( ... )*
			for (;;) {
				if ((LA(1) == LBRACK)) {
					lbt = LT(1);
					if ( inputState->guessing == 0 ) {
						lbt_AST = astFactory->create(lbt);
						astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(lbt_AST.get()));
					}
					match(LBRACK);
					if ( inputState->guessing==0 ) {
#line 907 "java.g"
						lbt_AST->setType(ARRAY_DECLARATOR);
#line 5492 "JavaRecognizer.cpp"
					}
					match(RBRACK);
				}
				else {
					goto _loop178;
				}
				
			}
			_loop178:;
			} // ( ... )*
			RefJavaAST tmp184_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp184_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp184_AST.get()));
			}
			match(DOT);
			RefJavaAST tmp185_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp185_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp185_AST.get()));
			}
			match(LITERAL_class);
			primaryExpression_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
			consumeUntil(_tokenSet_39);
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
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST newExpression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		RefJavaAST tmp186_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
		if ( inputState->guessing == 0 ) {
			tmp186_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp186_AST.get()));
		}
		match(LITERAL_new);
		type();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
		}
		{
		switch ( LA(1)) {
		case LPAREN:
		{
			match(LPAREN);
			argList();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			match(RPAREN);
			{
			switch ( LA(1)) {
			case LCURLY:
			{
				classBlock();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
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
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
			}
			{
			switch ( LA(1)) {
			case LCURLY:
			{
				arrayInitializer();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
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
		newExpression_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = newExpression_AST;
}

/** Match a, a.b.c refs, a.b.c(...) refs, a.b.c[], a.b.c[].class,
 *  and a.b.c.class refs.  Also this(...) and super(...).  Match
 *  this or super.
 */
void JavaRecognizer::identPrimary() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST identPrimary_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)RefToken  lp = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lp_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)RefToken  lbc = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lbc_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		RefJavaAST tmp189_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
		if ( inputState->guessing == 0 ) {
			tmp189_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp189_AST.get()));
		}
		match(IDENT);
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == DOT) && (LA(2) == IDENT)) {
				RefJavaAST tmp190_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp190_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp190_AST.get()));
				}
				match(DOT);
				RefJavaAST tmp191_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
				if ( inputState->guessing == 0 ) {
					tmp191_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp191_AST.get()));
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
				astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(lp_AST.get()));
			}
			match(LPAREN);
			if ( inputState->guessing==0 ) {
#line 933 "java.g"
				lp_AST->setType(METHOD_CALL);
#line 5820 "JavaRecognizer.cpp"
			}
			argList();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
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
						astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(lbc_AST.get()));
					}
					match(LBRACK);
					if ( inputState->guessing==0 ) {
#line 935 "java.g"
						lbc_AST->setType(ARRAY_DECLARATOR);
#line 5843 "JavaRecognizer.cpp"
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
		identPrimary_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = identPrimary_AST;
}

void JavaRecognizer::constant() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST constant_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		switch ( LA(1)) {
		case NUM_INT:
		{
			RefJavaAST tmp194_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp194_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp194_AST.get()));
			}
			match(NUM_INT);
			constant_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case CHAR_LITERAL:
		{
			RefJavaAST tmp195_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp195_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp195_AST.get()));
			}
			match(CHAR_LITERAL);
			constant_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case STRING_LITERAL:
		{
			RefJavaAST tmp196_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp196_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp196_AST.get()));
			}
			match(STRING_LITERAL);
			constant_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case NUM_FLOAT:
		{
			RefJavaAST tmp197_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp197_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp197_AST.get()));
			}
			match(NUM_FLOAT);
			constant_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case NUM_LONG:
		{
			RefJavaAST tmp198_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp198_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp198_AST.get()));
			}
			match(NUM_LONG);
			constant_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
			break;
		}
		case NUM_DOUBLE:
		{
			RefJavaAST tmp199_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
			if ( inputState->guessing == 0 ) {
				tmp199_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp199_AST.get()));
			}
			match(NUM_DOUBLE);
			constant_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
			consumeUntil(_tokenSet_39);
		} else {
			throw;
		}
	}
	returnAST = constant_AST;
}

void JavaRecognizer::newArrayDeclarator() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST newArrayDeclarator_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	ANTLR_USE_NAMESPACE(antlr)RefToken  lb = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lb_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	
	try {      // for error handling
		{ // ( ... )+
		int _cnt195=0;
		for (;;) {
			if ((LA(1) == LBRACK) && (_tokenSet_65.member(LA(2)))) {
				lb = LT(1);
				if ( inputState->guessing == 0 ) {
					lb_AST = astFactory->create(lb);
					astFactory->makeASTRoot(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(lb_AST.get()));
				}
				match(LBRACK);
				if ( inputState->guessing==0 ) {
#line 1023 "java.g"
					lb_AST->setType(ARRAY_DECLARATOR);
#line 5989 "JavaRecognizer.cpp"
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
						astFactory->addASTChild(currentAST, static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(returnAST.get()));
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
		newArrayDeclarator_AST = /*static_cast<RefJavaAST>*/(currentAST.root);
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
	returnAST = newArrayDeclarator_AST;
}

RefJavaAST JavaRecognizer::getAST()
{
	return returnAST;
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


