/* $ANTLR 2.7.1: "java.g" -> "JavaRecognizer.cpp"$ */
#include "JavaRecognizer.hpp"
#include "antlr/NoViableAltException.hpp"
#include "antlr/SemanticException.hpp"
#line 1 "java.g"

#line 8 "JavaRecognizer.cpp"
JavaRecognizer::JavaRecognizer(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf, int k)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(tokenBuf,k)
{
	setTokenNames(_tokenNames);
}

JavaRecognizer::JavaRecognizer(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(tokenBuf,2)
{
	setTokenNames(_tokenNames);
}

JavaRecognizer::JavaRecognizer(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer, int k)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(lexer,k)
{
	setTokenNames(_tokenNames);
}

JavaRecognizer::JavaRecognizer(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(lexer,2)
{
	setTokenNames(_tokenNames);
}

JavaRecognizer::JavaRecognizer(const ANTLR_USE_NAMESPACE(antlr)ParserSharedInputState& state)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(state,2)
{
	setTokenNames(_tokenNames);
}

void JavaRecognizer::compilationUnit() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST compilationUnit_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LITERAL_package:
		{
			packageDefinition();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case ANTLR_USE_NAMESPACE(antlr)Token::EOF_TYPE:
		case FINAL:
		case ABSTRACT:
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
		{
		for (;;) {
			if ((LA(1)==LITERAL_import)) {
				importDefinition();
				if (inputState->guessing==0) {
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop4;
			}
			
		}
		_loop4:;
		}
		{
		for (;;) {
			if ((_tokenSet_0.member(LA(1)))) {
				typeDefinition();
				if (inputState->guessing==0) {
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop6;
			}
			
		}
		_loop6:;
		}
		RefJavaAST tmp1_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp1_AST = astFactory.create(LT(1));
		match(ANTLR_USE_NAMESPACE(antlr)Token::EOF_TYPE);
		compilationUnit_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_1);
		} else {
			throw ex;
		}
	}
	returnAST = compilationUnit_AST;
}

void JavaRecognizer::packageDefinition() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST packageDefinition_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  p = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST p_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		p = LT(1);
		if (inputState->guessing==0) {
			p_AST = astFactory.create(p);
			astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(p_AST));
		}
		match(LITERAL_package);
		if ( inputState->guessing==0 ) {
#line 160 "java.g"
			p_AST->setType(PACKAGE_DEF);
#line 144 "JavaRecognizer.cpp"
		}
		identifier();
		if (inputState->guessing==0) {
			astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		RefJavaAST tmp2_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp2_AST = astFactory.create(LT(1));
		match(SEMI);
		packageDefinition_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_2);
		} else {
			throw ex;
		}
	}
	returnAST = packageDefinition_AST;
}

void JavaRecognizer::importDefinition() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST importDefinition_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  i = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST i_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		i = LT(1);
		if (inputState->guessing==0) {
			i_AST = astFactory.create(i);
			astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(i_AST));
		}
		match(LITERAL_import);
		if ( inputState->guessing==0 ) {
#line 167 "java.g"
			i_AST->setType(IMPORT);
#line 184 "JavaRecognizer.cpp"
		}
		identifierStar();
		if (inputState->guessing==0) {
			astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		RefJavaAST tmp3_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp3_AST = astFactory.create(LT(1));
		match(SEMI);
		importDefinition_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_2);
		} else {
			throw ex;
		}
	}
	returnAST = importDefinition_AST;
}

void JavaRecognizer::typeDefinition() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST typeDefinition_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST m_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case FINAL:
		case ABSTRACT:
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
				m_AST = RefJavaAST(returnAST);
			}
			{
			switch ( LA(1)) {
			case LITERAL_class:
			{
				classDefinition(m_AST);
				if (inputState->guessing==0) {
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case LITERAL_interface:
			{
				interfaceDefinition(m_AST);
				if (inputState->guessing==0) {
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
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
			RefJavaAST tmp4_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp4_AST = astFactory.create(LT(1));
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
			consume();
			consumeUntil(_tokenSet_3);
		} else {
			throw ex;
		}
	}
	returnAST = typeDefinition_AST;
}

void JavaRecognizer::identifier() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST identifier_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefJavaAST tmp5_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if (inputState->guessing==0) {
			tmp5_AST = astFactory.create(LT(1));
			astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp5_AST));
		}
		match(IDENT);
		{
		for (;;) {
			if ((LA(1)==DOT)) {
				RefJavaAST tmp6_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if (inputState->guessing==0) {
					tmp6_AST = astFactory.create(LT(1));
					astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp6_AST));
				}
				match(DOT);
				RefJavaAST tmp7_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if (inputState->guessing==0) {
					tmp7_AST = astFactory.create(LT(1));
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp7_AST));
				}
				match(IDENT);
			}
			else {
				goto _loop26;
			}
			
		}
		_loop26:;
		}
		identifier_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_4);
		} else {
			throw ex;
		}
	}
	returnAST = identifier_AST;
}

void JavaRecognizer::identifierStar() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST identifierStar_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefJavaAST tmp8_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if (inputState->guessing==0) {
			tmp8_AST = astFactory.create(LT(1));
			astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp8_AST));
		}
		match(IDENT);
		{
		for (;;) {
			if ((LA(1)==DOT) && (LA(2)==IDENT)) {
				RefJavaAST tmp9_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if (inputState->guessing==0) {
					tmp9_AST = astFactory.create(LT(1));
					astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp9_AST));
				}
				match(DOT);
				RefJavaAST tmp10_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if (inputState->guessing==0) {
					tmp10_AST = astFactory.create(LT(1));
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp10_AST));
				}
				match(IDENT);
			}
			else {
				goto _loop29;
			}
			
		}
		_loop29:;
		}
		{
		switch ( LA(1)) {
		case DOT:
		{
			RefJavaAST tmp11_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp11_AST = astFactory.create(LT(1));
				astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp11_AST));
			}
			match(DOT);
			RefJavaAST tmp12_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp12_AST = astFactory.create(LT(1));
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp12_AST));
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
			consume();
			consumeUntil(_tokenSet_5);
		} else {
			throw ex;
		}
	}
	returnAST = identifierStar_AST;
}

void JavaRecognizer::modifiers() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST modifiers_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		for (;;) {
			if ((_tokenSet_6.member(LA(1)))) {
				modifier();
				if (inputState->guessing==0) {
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop14;
			}
			
		}
		_loop14:;
		}
		if ( inputState->guessing==0 ) {
			modifiers_AST = RefJavaAST(currentAST.root);
#line 194 "java.g"
			modifiers_AST = RefJavaAST(astFactory.make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory.create(MODIFIERS,"MODIFIERS")))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(modifiers_AST))));
#line 437 "JavaRecognizer.cpp"
			currentAST.root = modifiers_AST;
			if ( modifiers_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				modifiers_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
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
			consume();
			consumeUntil(_tokenSet_7);
		} else {
			throw ex;
		}
	}
	returnAST = modifiers_AST;
}

void JavaRecognizer::classDefinition(
	RefJavaAST modifiers
) {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST classDefinition_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST sc_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST ic_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST cb_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefJavaAST tmp13_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if (inputState->guessing==0) {
			tmp13_AST = astFactory.create(LT(1));
		}
		match(LITERAL_class);
		RefJavaAST tmp14_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if (inputState->guessing==0) {
			tmp14_AST = astFactory.create(LT(1));
		}
		match(IDENT);
		superClassClause();
		if (inputState->guessing==0) {
			sc_AST = RefJavaAST(returnAST);
		}
		implementsClause();
		if (inputState->guessing==0) {
			ic_AST = RefJavaAST(returnAST);
		}
		classBlock();
		if (inputState->guessing==0) {
			cb_AST = RefJavaAST(returnAST);
		}
		if ( inputState->guessing==0 ) {
			classDefinition_AST = RefJavaAST(currentAST.root);
#line 286 "java.g"
			classDefinition_AST = RefJavaAST(astFactory.make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(6))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory.create(CLASS_DEF,"CLASS_DEF")))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(modifiers))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp14_AST))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(sc_AST))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(ic_AST))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(cb_AST))));
#line 497 "JavaRecognizer.cpp"
			currentAST.root = classDefinition_AST;
			if ( classDefinition_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				classDefinition_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
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
			throw ex;
		}
	}
	returnAST = classDefinition_AST;
}

void JavaRecognizer::interfaceDefinition(
	RefJavaAST modifiers
) {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST interfaceDefinition_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST ie_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST cb_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefJavaAST tmp15_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if (inputState->guessing==0) {
			tmp15_AST = astFactory.create(LT(1));
		}
		match(LITERAL_interface);
		RefJavaAST tmp16_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if (inputState->guessing==0) {
			tmp16_AST = astFactory.create(LT(1));
		}
		match(IDENT);
		interfaceExtends();
		if (inputState->guessing==0) {
			ie_AST = RefJavaAST(returnAST);
		}
		classBlock();
		if (inputState->guessing==0) {
			cb_AST = RefJavaAST(returnAST);
		}
		if ( inputState->guessing==0 ) {
			interfaceDefinition_AST = RefJavaAST(currentAST.root);
#line 302 "java.g"
			interfaceDefinition_AST = RefJavaAST(astFactory.make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(5))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory.create(INTERFACE_DEF,"INTERFACE_DEF")))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(modifiers))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp16_AST))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(ie_AST))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(cb_AST))));
#line 551 "JavaRecognizer.cpp"
			currentAST.root = interfaceDefinition_AST;
			if ( interfaceDefinition_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				interfaceDefinition_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
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
			throw ex;
		}
	}
	returnAST = interfaceDefinition_AST;
}

/** A declaration is the creation of a reference or primitive-type variable
 *  Create a separate Type/Var tree for each var in the var list.
 */
void JavaRecognizer::declaration() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST declaration_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST m_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST t_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST v_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		modifiers();
		if (inputState->guessing==0) {
			m_AST = RefJavaAST(returnAST);
		}
		typeSpec(false);
		if (inputState->guessing==0) {
			t_AST = RefJavaAST(returnAST);
		}
		variableDefinitions(m_AST,t_AST);
		if (inputState->guessing==0) {
			v_AST = RefJavaAST(returnAST);
		}
		if ( inputState->guessing==0 ) {
			declaration_AST = RefJavaAST(currentAST.root);
#line 185 "java.g"
			declaration_AST = v_AST;
#line 601 "JavaRecognizer.cpp"
			currentAST.root = declaration_AST;
			if ( declaration_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				declaration_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
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
			throw ex;
		}
	}
	returnAST = declaration_AST;
}

void JavaRecognizer::typeSpec(
	bool addImagNode
) {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST typeSpec_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case IDENT:
		{
			classTypeSpec(addImagNode);
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
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
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
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
			consume();
			consumeUntil(_tokenSet_10);
		} else {
			throw ex;
		}
	}
	returnAST = typeSpec_AST;
}

void JavaRecognizer::variableDefinitions(
	RefJavaAST mods, RefJavaAST t
) {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST variableDefinitions_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		variableDeclarator((RefJavaAST)getASTFactory().dupTree((antlr::RefAST)mods),
                                   (RefJavaAST)getASTFactory().dupTree((antlr::RefAST)t));
		if (inputState->guessing==0) {
			astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		for (;;) {
			if ((LA(1)==COMMA)) {
				RefJavaAST tmp17_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				tmp17_AST = astFactory.create(LT(1));
				match(COMMA);
				variableDeclarator((RefJavaAST)getASTFactory().dupTree((antlr::RefAST)mods),
                                           (RefJavaAST)getASTFactory().dupTree((antlr::RefAST)t));
				if (inputState->guessing==0) {
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop54;
			}
			
		}
		_loop54:;
		}
		variableDefinitions_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_5);
		} else {
			throw ex;
		}
	}
	returnAST = variableDefinitions_AST;
}

void JavaRecognizer::modifier() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST modifier_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case LITERAL_private:
		{
			RefJavaAST tmp18_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp18_AST = astFactory.create(LT(1));
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp18_AST));
			}
			match(LITERAL_private);
			modifier_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_public:
		{
			RefJavaAST tmp19_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp19_AST = astFactory.create(LT(1));
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp19_AST));
			}
			match(LITERAL_public);
			modifier_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_protected:
		{
			RefJavaAST tmp20_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp20_AST = astFactory.create(LT(1));
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp20_AST));
			}
			match(LITERAL_protected);
			modifier_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_static:
		{
			RefJavaAST tmp21_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp21_AST = astFactory.create(LT(1));
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp21_AST));
			}
			match(LITERAL_static);
			modifier_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_transient:
		{
			RefJavaAST tmp22_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp22_AST = astFactory.create(LT(1));
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp22_AST));
			}
			match(LITERAL_transient);
			modifier_AST = RefJavaAST(currentAST.root);
			break;
		}
		case FINAL:
		{
			RefJavaAST tmp23_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp23_AST = astFactory.create(LT(1));
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp23_AST));
			}
			match(FINAL);
			modifier_AST = RefJavaAST(currentAST.root);
			break;
		}
		case ABSTRACT:
		{
			RefJavaAST tmp24_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp24_AST = astFactory.create(LT(1));
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp24_AST));
			}
			match(ABSTRACT);
			modifier_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_native:
		{
			RefJavaAST tmp25_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp25_AST = astFactory.create(LT(1));
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp25_AST));
			}
			match(LITERAL_native);
			modifier_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_threadsafe:
		{
			RefJavaAST tmp26_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp26_AST = astFactory.create(LT(1));
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp26_AST));
			}
			match(LITERAL_threadsafe);
			modifier_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_synchronized:
		{
			RefJavaAST tmp27_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp27_AST = astFactory.create(LT(1));
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp27_AST));
			}
			match(LITERAL_synchronized);
			modifier_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_volatile:
		{
			RefJavaAST tmp28_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp28_AST = astFactory.create(LT(1));
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp28_AST));
			}
			match(LITERAL_volatile);
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
			consume();
			consumeUntil(_tokenSet_11);
		} else {
			throw ex;
		}
	}
	returnAST = modifier_AST;
}

void JavaRecognizer::classTypeSpec(
	bool addImagNode
) {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST classTypeSpec_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  lb = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lb_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		identifier();
		if (inputState->guessing==0) {
			astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		for (;;) {
			if ((LA(1)==LBRACK)) {
				lb = LT(1);
				if (inputState->guessing==0) {
					lb_AST = astFactory.create(lb);
					astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(lb_AST));
				}
				match(LBRACK);
				if ( inputState->guessing==0 ) {
#line 208 "java.g"
					lb_AST->setType(ARRAY_DECLARATOR);
#line 894 "JavaRecognizer.cpp"
				}
				RefJavaAST tmp29_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				tmp29_AST = astFactory.create(LT(1));
				match(RBRACK);
			}
			else {
				goto _loop18;
			}
			
		}
		_loop18:;
		}
		if ( inputState->guessing==0 ) {
			classTypeSpec_AST = RefJavaAST(currentAST.root);
#line 209 "java.g"
			
						if ( addImagNode ) {
							classTypeSpec_AST = RefJavaAST(astFactory.make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory.create(TYPE,"TYPE")))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(classTypeSpec_AST))));
						}
					
#line 915 "JavaRecognizer.cpp"
			currentAST.root = classTypeSpec_AST;
			if ( classTypeSpec_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				classTypeSpec_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
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
			consume();
			consumeUntil(_tokenSet_10);
		} else {
			throw ex;
		}
	}
	returnAST = classTypeSpec_AST;
}

void JavaRecognizer::builtInTypeSpec(
	bool addImagNode
) {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST builtInTypeSpec_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  lb = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lb_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		builtInType();
		if (inputState->guessing==0) {
			astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		for (;;) {
			if ((LA(1)==LBRACK)) {
				lb = LT(1);
				if (inputState->guessing==0) {
					lb_AST = astFactory.create(lb);
					astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(lb_AST));
				}
				match(LBRACK);
				if ( inputState->guessing==0 ) {
#line 219 "java.g"
					lb_AST->setType(ARRAY_DECLARATOR);
#line 964 "JavaRecognizer.cpp"
				}
				RefJavaAST tmp30_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				tmp30_AST = astFactory.create(LT(1));
				match(RBRACK);
			}
			else {
				goto _loop21;
			}
			
		}
		_loop21:;
		}
		if ( inputState->guessing==0 ) {
			builtInTypeSpec_AST = RefJavaAST(currentAST.root);
#line 220 "java.g"
			
						if ( addImagNode ) {
							builtInTypeSpec_AST = RefJavaAST(astFactory.make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory.create(TYPE,"TYPE")))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(builtInTypeSpec_AST))));
						}
					
#line 985 "JavaRecognizer.cpp"
			currentAST.root = builtInTypeSpec_AST;
			if ( builtInTypeSpec_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				builtInTypeSpec_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
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
			consume();
			consumeUntil(_tokenSet_10);
		} else {
			throw ex;
		}
	}
	returnAST = builtInTypeSpec_AST;
}

void JavaRecognizer::builtInType() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST builtInType_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case LITERAL_void:
		{
			RefJavaAST tmp31_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp31_AST = astFactory.create(LT(1));
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp31_AST));
			}
			match(LITERAL_void);
			builtInType_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_boolean:
		{
			RefJavaAST tmp32_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp32_AST = astFactory.create(LT(1));
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp32_AST));
			}
			match(LITERAL_boolean);
			builtInType_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_byte:
		{
			RefJavaAST tmp33_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp33_AST = astFactory.create(LT(1));
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp33_AST));
			}
			match(LITERAL_byte);
			builtInType_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_char:
		{
			RefJavaAST tmp34_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp34_AST = astFactory.create(LT(1));
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp34_AST));
			}
			match(LITERAL_char);
			builtInType_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_short:
		{
			RefJavaAST tmp35_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp35_AST = astFactory.create(LT(1));
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp35_AST));
			}
			match(LITERAL_short);
			builtInType_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_int:
		{
			RefJavaAST tmp36_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp36_AST = astFactory.create(LT(1));
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp36_AST));
			}
			match(LITERAL_int);
			builtInType_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_float:
		{
			RefJavaAST tmp37_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp37_AST = astFactory.create(LT(1));
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp37_AST));
			}
			match(LITERAL_float);
			builtInType_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_long:
		{
			RefJavaAST tmp38_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp38_AST = astFactory.create(LT(1));
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp38_AST));
			}
			match(LITERAL_long);
			builtInType_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_double:
		{
			RefJavaAST tmp39_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp39_AST = astFactory.create(LT(1));
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp39_AST));
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
			consume();
			consumeUntil(_tokenSet_12);
		} else {
			throw ex;
		}
	}
	returnAST = builtInType_AST;
}

void JavaRecognizer::type() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST type_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case IDENT:
		{
			identifier();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
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
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
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
			consume();
			consumeUntil(_tokenSet_13);
		} else {
			throw ex;
		}
	}
	returnAST = type_AST;
}

void JavaRecognizer::superClassClause() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST superClassClause_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST id_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LITERAL_extends:
		{
			RefJavaAST tmp40_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp40_AST = astFactory.create(LT(1));
			}
			match(LITERAL_extends);
			identifier();
			if (inputState->guessing==0) {
				id_AST = RefJavaAST(returnAST);
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
#line 292 "java.g"
			superClassClause_AST = RefJavaAST(astFactory.make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory.create(EXTENDS_CLAUSE,"EXTENDS_CLAUSE")))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(id_AST))));
#line 1220 "JavaRecognizer.cpp"
			currentAST.root = superClassClause_AST;
			if ( superClassClause_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				superClassClause_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
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
			throw ex;
		}
	}
	returnAST = superClassClause_AST;
}

void JavaRecognizer::implementsClause() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST implementsClause_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  i = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST i_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LITERAL_implements:
		{
			i = LT(1);
			if (inputState->guessing==0) {
				i_AST = astFactory.create(i);
			}
			match(LITERAL_implements);
			identifier();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{
			for (;;) {
				if ((LA(1)==COMMA)) {
					RefJavaAST tmp41_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					tmp41_AST = astFactory.create(LT(1));
					match(COMMA);
					identifier();
					if (inputState->guessing==0) {
						astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
				}
				else {
					goto _loop46;
				}
				
			}
			_loop46:;
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
		if ( inputState->guessing==0 ) {
			implementsClause_AST = RefJavaAST(currentAST.root);
#line 331 "java.g"
			implementsClause_AST = RefJavaAST(astFactory.make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory.create(IMPLEMENTS_CLAUSE,"IMPLEMENTS_CLAUSE")))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(implementsClause_AST))));
#line 1297 "JavaRecognizer.cpp"
			currentAST.root = implementsClause_AST;
			if ( implementsClause_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				implementsClause_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
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
			consume();
			consumeUntil(_tokenSet_15);
		} else {
			throw ex;
		}
	}
	returnAST = implementsClause_AST;
}

void JavaRecognizer::classBlock() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST classBlock_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefJavaAST tmp42_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp42_AST = astFactory.create(LT(1));
		match(LCURLY);
		{
		for (;;) {
			switch ( LA(1)) {
			case FINAL:
			case ABSTRACT:
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
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case SEMI:
			{
				RefJavaAST tmp43_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				tmp43_AST = astFactory.create(LT(1));
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
		}
		RefJavaAST tmp44_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp44_AST = astFactory.create(LT(1));
		match(RCURLY);
		if ( inputState->guessing==0 ) {
			classBlock_AST = RefJavaAST(currentAST.root);
#line 313 "java.g"
			classBlock_AST = RefJavaAST(astFactory.make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory.create(OBJBLOCK,"OBJBLOCK")))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(classBlock_AST))));
#line 1385 "JavaRecognizer.cpp"
			currentAST.root = classBlock_AST;
			if ( classBlock_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				classBlock_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
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
			consume();
			consumeUntil(_tokenSet_16);
		} else {
			throw ex;
		}
	}
	returnAST = classBlock_AST;
}

void JavaRecognizer::interfaceExtends() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST interfaceExtends_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  e = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST e_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LITERAL_extends:
		{
			e = LT(1);
			if (inputState->guessing==0) {
				e_AST = astFactory.create(e);
			}
			match(LITERAL_extends);
			identifier();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{
			for (;;) {
				if ((LA(1)==COMMA)) {
					RefJavaAST tmp45_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					tmp45_AST = astFactory.create(LT(1));
					match(COMMA);
					identifier();
					if (inputState->guessing==0) {
						astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
				}
				else {
					goto _loop42;
				}
				
			}
			_loop42:;
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
		if ( inputState->guessing==0 ) {
			interfaceExtends_AST = RefJavaAST(currentAST.root);
#line 322 "java.g"
			interfaceExtends_AST = RefJavaAST(astFactory.make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory.create(EXTENDS_CLAUSE,"EXTENDS_CLAUSE")))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(interfaceExtends_AST))));
#line 1463 "JavaRecognizer.cpp"
			currentAST.root = interfaceExtends_AST;
			if ( interfaceExtends_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				interfaceExtends_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
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
			consume();
			consumeUntil(_tokenSet_15);
		} else {
			throw ex;
		}
	}
	returnAST = interfaceExtends_AST;
}

void JavaRecognizer::field() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST field_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST mods_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST h_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST s_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST cd_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST id_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST t_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST param_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST rt_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST tc_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST s2_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST v_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST s3_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST s4_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		if ((_tokenSet_11.member(LA(1))) && (_tokenSet_17.member(LA(2)))) {
			modifiers();
			if (inputState->guessing==0) {
				mods_AST = RefJavaAST(returnAST);
			}
			{
			switch ( LA(1)) {
			case LITERAL_class:
			{
				classDefinition(mods_AST);
				if (inputState->guessing==0) {
					cd_AST = RefJavaAST(returnAST);
				}
				if ( inputState->guessing==0 ) {
					field_AST = RefJavaAST(currentAST.root);
#line 346 "java.g"
					field_AST = cd_AST;
#line 1522 "JavaRecognizer.cpp"
					currentAST.root = field_AST;
					if ( field_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
						field_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
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
					id_AST = RefJavaAST(returnAST);
				}
				if ( inputState->guessing==0 ) {
					field_AST = RefJavaAST(currentAST.root);
#line 349 "java.g"
					field_AST = id_AST;
#line 1543 "JavaRecognizer.cpp"
					currentAST.root = field_AST;
					if ( field_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
						field_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
						  currentAST.child = field_AST->getFirstChild();
					else
						currentAST.child = field_AST;
					currentAST.advanceChildToEnd();
				}
				break;
			}
			default:
				if ((LA(1)==IDENT) && (LA(2)==LPAREN)) {
					ctorHead();
					if (inputState->guessing==0) {
						h_AST = RefJavaAST(returnAST);
					}
					compoundStatement();
					if (inputState->guessing==0) {
						s_AST = RefJavaAST(returnAST);
					}
					if ( inputState->guessing==0 ) {
						field_AST = RefJavaAST(currentAST.root);
#line 343 "java.g"
						field_AST = RefJavaAST(astFactory.make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(4))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory.create(CTOR_DEF,"CTOR_DEF")))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(mods_AST))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(h_AST))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(s_AST))));
#line 1568 "JavaRecognizer.cpp"
						currentAST.root = field_AST;
						if ( field_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
							field_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
							  currentAST.child = field_AST->getFirstChild();
						else
							currentAST.child = field_AST;
						currentAST.advanceChildToEnd();
					}
				}
				else if (((LA(1) >= LITERAL_void && LA(1) <= IDENT)) && (_tokenSet_18.member(LA(2)))) {
					typeSpec(false);
					if (inputState->guessing==0) {
						t_AST = RefJavaAST(returnAST);
					}
					{
					if ((LA(1)==IDENT) && (LA(2)==LPAREN)) {
						RefJavaAST tmp46_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
						if (inputState->guessing==0) {
							tmp46_AST = astFactory.create(LT(1));
						}
						match(IDENT);
						RefJavaAST tmp47_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
						tmp47_AST = astFactory.create(LT(1));
						match(LPAREN);
						parameterDeclarationList();
						if (inputState->guessing==0) {
							param_AST = RefJavaAST(returnAST);
						}
						RefJavaAST tmp48_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
						tmp48_AST = astFactory.create(LT(1));
						match(RPAREN);
						returnTypeBrackersOnEndOfMethodHead(t_AST);
						if (inputState->guessing==0) {
							rt_AST = RefJavaAST(returnAST);
						}
						{
						switch ( LA(1)) {
						case LITERAL_throws:
						{
							throwsClause();
							if (inputState->guessing==0) {
								tc_AST = RefJavaAST(returnAST);
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
								s2_AST = RefJavaAST(returnAST);
							}
							break;
						}
						case SEMI:
						{
							RefJavaAST tmp49_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
							if (inputState->guessing==0) {
								tmp49_AST = astFactory.create(LT(1));
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
#line 363 "java.g"
							field_AST = RefJavaAST(astFactory.make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(7))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory.create(METHOD_DEF,"METHOD_DEF")))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(mods_AST))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(RefJavaAST(astFactory.make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory.create(TYPE,"TYPE")))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(rt_AST))))))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tmp46_AST))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(param_AST))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(tc_AST))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(s2_AST))));
#line 1654 "JavaRecognizer.cpp"
							currentAST.root = field_AST;
							if ( field_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
								field_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
								  currentAST.child = field_AST->getFirstChild();
							else
								currentAST.child = field_AST;
							currentAST.advanceChildToEnd();
						}
					}
					else if ((LA(1)==IDENT) && (_tokenSet_19.member(LA(2)))) {
						variableDefinitions(mods_AST,t_AST);
						if (inputState->guessing==0) {
							v_AST = RefJavaAST(returnAST);
						}
						RefJavaAST tmp50_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
						if (inputState->guessing==0) {
							tmp50_AST = astFactory.create(LT(1));
						}
						match(SEMI);
						if ( inputState->guessing==0 ) {
							field_AST = RefJavaAST(currentAST.root);
#line 372 "java.g"
							field_AST = v_AST;
#line 1678 "JavaRecognizer.cpp"
							currentAST.root = field_AST;
							if ( field_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
								field_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
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
		else if ((LA(1)==LITERAL_static) && (LA(2)==LCURLY)) {
			RefJavaAST tmp51_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp51_AST = astFactory.create(LT(1));
			}
			match(LITERAL_static);
			compoundStatement();
			if (inputState->guessing==0) {
				s3_AST = RefJavaAST(returnAST);
			}
			if ( inputState->guessing==0 ) {
				field_AST = RefJavaAST(currentAST.root);
#line 378 "java.g"
				field_AST = RefJavaAST(astFactory.make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory.create(STATIC_INIT,"STATIC_INIT")))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(s3_AST))));
#line 1714 "JavaRecognizer.cpp"
				currentAST.root = field_AST;
				if ( field_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
					field_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
					  currentAST.child = field_AST->getFirstChild();
				else
					currentAST.child = field_AST;
				currentAST.advanceChildToEnd();
			}
		}
		else if ((LA(1)==LCURLY)) {
			compoundStatement();
			if (inputState->guessing==0) {
				s4_AST = RefJavaAST(returnAST);
			}
			if ( inputState->guessing==0 ) {
				field_AST = RefJavaAST(currentAST.root);
#line 382 "java.g"
				field_AST = RefJavaAST(astFactory.make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory.create(INSTANCE_INIT,"INSTANCE_INIT")))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(s4_AST))));
#line 1733 "JavaRecognizer.cpp"
				currentAST.root = field_AST;
				if ( field_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
					field_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
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
			throw ex;
		}
	}
	returnAST = field_AST;
}

void JavaRecognizer::ctorHead() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST ctorHead_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefJavaAST tmp52_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if (inputState->guessing==0) {
			tmp52_AST = astFactory.create(LT(1));
			astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp52_AST));
		}
		match(IDENT);
		RefJavaAST tmp53_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp53_AST = astFactory.create(LT(1));
		match(LPAREN);
		parameterDeclarationList();
		if (inputState->guessing==0) {
			astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		RefJavaAST tmp54_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp54_AST = astFactory.create(LT(1));
		match(RPAREN);
		{
		switch ( LA(1)) {
		case LITERAL_throws:
		{
			throwsClause();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
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
			consume();
			consumeUntil(_tokenSet_15);
		} else {
			throw ex;
		}
	}
	returnAST = ctorHead_AST;
}

void JavaRecognizer::compoundStatement() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST compoundStatement_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  lc = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lc_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		lc = LT(1);
		if (inputState->guessing==0) {
			lc_AST = astFactory.create(lc);
			astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(lc_AST));
		}
		match(LCURLY);
		if ( inputState->guessing==0 ) {
#line 500 "java.g"
			lc_AST->setType(SLIST);
#line 1833 "JavaRecognizer.cpp"
		}
		{
		for (;;) {
			if ((_tokenSet_21.member(LA(1)))) {
				statement();
				if (inputState->guessing==0) {
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop87;
			}
			
		}
		_loop87:;
		}
		RefJavaAST tmp55_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp55_AST = astFactory.create(LT(1));
		match(RCURLY);
		compoundStatement_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_22);
		} else {
			throw ex;
		}
	}
	returnAST = compoundStatement_AST;
}

void JavaRecognizer::parameterDeclarationList() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST parameterDeclarationList_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{
			for (;;) {
				if ((LA(1)==COMMA)) {
					RefJavaAST tmp56_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					tmp56_AST = astFactory.create(LT(1));
					match(COMMA);
					parameterDeclaration();
					if (inputState->guessing==0) {
						astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
				}
				else {
					goto _loop78;
				}
				
			}
			_loop78:;
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
			parameterDeclarationList_AST = RefJavaAST(currentAST.root);
#line 468 "java.g"
			parameterDeclarationList_AST = RefJavaAST(astFactory.make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory.create(PARAMETERS,"PARAMETERS")))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(parameterDeclarationList_AST))));
#line 1925 "JavaRecognizer.cpp"
			currentAST.root = parameterDeclarationList_AST;
			if ( parameterDeclarationList_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				parameterDeclarationList_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
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
			consume();
			consumeUntil(_tokenSet_23);
		} else {
			throw ex;
		}
	}
	returnAST = parameterDeclarationList_AST;
}

void JavaRecognizer::returnTypeBrackersOnEndOfMethodHead(
	RefJavaAST typ
) {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST returnTypeBrackersOnEndOfMethodHead_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  lb = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lb_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		if ( inputState->guessing==0 ) {
			returnTypeBrackersOnEndOfMethodHead_AST = RefJavaAST(currentAST.root);
#line 461 "java.g"
			returnTypeBrackersOnEndOfMethodHead_AST = typ;
#line 1962 "JavaRecognizer.cpp"
			currentAST.root = returnTypeBrackersOnEndOfMethodHead_AST;
			if ( returnTypeBrackersOnEndOfMethodHead_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				returnTypeBrackersOnEndOfMethodHead_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = returnTypeBrackersOnEndOfMethodHead_AST->getFirstChild();
			else
				currentAST.child = returnTypeBrackersOnEndOfMethodHead_AST;
			currentAST.advanceChildToEnd();
		}
		{
		for (;;) {
			if ((LA(1)==LBRACK)) {
				lb = LT(1);
				if (inputState->guessing==0) {
					lb_AST = astFactory.create(lb);
					astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(lb_AST));
				}
				match(LBRACK);
				if ( inputState->guessing==0 ) {
#line 462 "java.g"
					lb_AST->setType(ARRAY_DECLARATOR);
#line 1983 "JavaRecognizer.cpp"
				}
				RefJavaAST tmp57_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				tmp57_AST = astFactory.create(LT(1));
				match(RBRACK);
			}
			else {
				goto _loop74;
			}
			
		}
		_loop74:;
		}
		returnTypeBrackersOnEndOfMethodHead_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_24);
		} else {
			throw ex;
		}
	}
	returnAST = returnTypeBrackersOnEndOfMethodHead_AST;
}

void JavaRecognizer::throwsClause() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST throwsClause_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefJavaAST tmp58_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if (inputState->guessing==0) {
			tmp58_AST = astFactory.create(LT(1));
			astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp58_AST));
		}
		match(LITERAL_throws);
		identifier();
		if (inputState->guessing==0) {
			astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		for (;;) {
			if ((LA(1)==COMMA)) {
				RefJavaAST tmp59_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				tmp59_AST = astFactory.create(LT(1));
				match(COMMA);
				identifier();
				if (inputState->guessing==0) {
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop71;
			}
			
		}
		_loop71:;
		}
		throwsClause_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_25);
		} else {
			throw ex;
		}
	}
	returnAST = throwsClause_AST;
}

/** Declaration of a variable.  This can be a class/instance variable,
 *   or a local variable in a method
 * It can also include possible initialization.
 */
void JavaRecognizer::variableDeclarator(
	RefJavaAST mods, RefJavaAST t
) {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST variableDeclarator_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  id = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST id_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST d_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST v_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		id = LT(1);
		if (inputState->guessing==0) {
			id_AST = astFactory.create(id);
		}
		match(IDENT);
		declaratorBrackets(t);
		if (inputState->guessing==0) {
			d_AST = RefJavaAST(returnAST);
		}
		varInitializer();
		if (inputState->guessing==0) {
			v_AST = RefJavaAST(returnAST);
		}
		if ( inputState->guessing==0 ) {
			variableDeclarator_AST = RefJavaAST(currentAST.root);
#line 401 "java.g"
			variableDeclarator_AST = RefJavaAST(astFactory.make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(5))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory.create(VARIABLE_DEF,"VARIABLE_DEF")))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(mods))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(RefJavaAST(astFactory.make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory.create(TYPE,"TYPE")))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(d_AST))))))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(id_AST))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(v_AST))));
#line 2091 "JavaRecognizer.cpp"
			currentAST.root = variableDeclarator_AST;
			if ( variableDeclarator_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				variableDeclarator_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
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
			consumeUntil(_tokenSet_26);
		} else {
			throw ex;
		}
	}
	returnAST = variableDeclarator_AST;
}

void JavaRecognizer::declaratorBrackets(
	RefJavaAST typ
) {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST declaratorBrackets_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  lb = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lb_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		if ( inputState->guessing==0 ) {
			declaratorBrackets_AST = RefJavaAST(currentAST.root);
#line 405 "java.g"
			declaratorBrackets_AST=typ;
#line 2127 "JavaRecognizer.cpp"
			currentAST.root = declaratorBrackets_AST;
			if ( declaratorBrackets_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				declaratorBrackets_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = declaratorBrackets_AST->getFirstChild();
			else
				currentAST.child = declaratorBrackets_AST;
			currentAST.advanceChildToEnd();
		}
		{
		for (;;) {
			if ((LA(1)==LBRACK)) {
				lb = LT(1);
				if (inputState->guessing==0) {
					lb_AST = astFactory.create(lb);
					astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(lb_AST));
				}
				match(LBRACK);
				if ( inputState->guessing==0 ) {
#line 406 "java.g"
					lb_AST->setType(ARRAY_DECLARATOR);
#line 2148 "JavaRecognizer.cpp"
				}
				RefJavaAST tmp60_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				tmp60_AST = astFactory.create(LT(1));
				match(RBRACK);
			}
			else {
				goto _loop58;
			}
			
		}
		_loop58:;
		}
		declaratorBrackets_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_27);
		} else {
			throw ex;
		}
	}
	returnAST = declaratorBrackets_AST;
}

void JavaRecognizer::varInitializer() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST varInitializer_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case ASSIGN:
		{
			RefJavaAST tmp61_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp61_AST = astFactory.create(LT(1));
				astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp61_AST));
			}
			match(ASSIGN);
			initializer();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
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
			consume();
			consumeUntil(_tokenSet_26);
		} else {
			throw ex;
		}
	}
	returnAST = varInitializer_AST;
}

void JavaRecognizer::initializer() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST initializer_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
		case PLUS:
		case MINUS:
		case INC:
		case DEC:
		case BNOT:
		case LNOT:
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
		{
			expression();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			initializer_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LCURLY:
		{
			arrayInitializer();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
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
			consume();
			consumeUntil(_tokenSet_28);
		} else {
			throw ex;
		}
	}
	returnAST = initializer_AST;
}

void JavaRecognizer::arrayInitializer() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST arrayInitializer_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  lc = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lc_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		lc = LT(1);
		if (inputState->guessing==0) {
			lc_AST = astFactory.create(lc);
			astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(lc_AST));
		}
		match(LCURLY);
		if ( inputState->guessing==0 ) {
#line 415 "java.g"
			lc_AST->setType(ARRAY_INIT);
#line 2308 "JavaRecognizer.cpp"
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
		case PLUS:
		case MINUS:
		case INC:
		case DEC:
		case BNOT:
		case LNOT:
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
		{
			initializer();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{
			for (;;) {
				if ((LA(1)==COMMA) && (_tokenSet_29.member(LA(2)))) {
					RefJavaAST tmp62_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					tmp62_AST = astFactory.create(LT(1));
					match(COMMA);
					initializer();
					if (inputState->guessing==0) {
						astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
				}
				else {
					goto _loop64;
				}
				
			}
			_loop64:;
			}
			{
			switch ( LA(1)) {
			case COMMA:
			{
				RefJavaAST tmp63_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				tmp63_AST = astFactory.create(LT(1));
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
		RefJavaAST tmp64_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp64_AST = astFactory.create(LT(1));
		match(RCURLY);
		arrayInitializer_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_30);
		} else {
			throw ex;
		}
	}
	returnAST = arrayInitializer_AST;
}

void JavaRecognizer::expression() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST expression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		assignmentExpression();
		if (inputState->guessing==0) {
			astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if ( inputState->guessing==0 ) {
			expression_AST = RefJavaAST(currentAST.root);
#line 685 "java.g"
			expression_AST = RefJavaAST(astFactory.make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory.create(EXPR,"EXPR")))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(expression_AST))));
#line 2425 "JavaRecognizer.cpp"
			currentAST.root = expression_AST;
			if ( expression_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				expression_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
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
			consume();
			consumeUntil(_tokenSet_31);
		} else {
			throw ex;
		}
	}
	returnAST = expression_AST;
}

void JavaRecognizer::parameterDeclaration() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST parameterDeclaration_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST pm_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST t_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  id = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST id_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefJavaAST pd_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		parameterModifier();
		if (inputState->guessing==0) {
			pm_AST = RefJavaAST(returnAST);
		}
		typeSpec(false);
		if (inputState->guessing==0) {
			t_AST = RefJavaAST(returnAST);
		}
		id = LT(1);
		if (inputState->guessing==0) {
			id_AST = astFactory.create(id);
		}
		match(IDENT);
		parameterDeclaratorBrackets(t_AST);
		if (inputState->guessing==0) {
			pd_AST = RefJavaAST(returnAST);
		}
		if ( inputState->guessing==0 ) {
			parameterDeclaration_AST = RefJavaAST(currentAST.root);
#line 476 "java.g"
			parameterDeclaration_AST = RefJavaAST(astFactory.make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(4))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory.create(PARAMETER_DEF,"PARAMETER_DEF")))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(pm_AST))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(RefJavaAST(astFactory.make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory.create(TYPE,"TYPE")))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(pd_AST))))))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(id_AST))));
#line 2480 "JavaRecognizer.cpp"
			currentAST.root = parameterDeclaration_AST;
			if ( parameterDeclaration_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				parameterDeclaration_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
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
			consumeUntil(_tokenSet_32);
		} else {
			throw ex;
		}
	}
	returnAST = parameterDeclaration_AST;
}

void JavaRecognizer::parameterModifier() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST parameterModifier_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  f = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST f_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case FINAL:
		{
			f = LT(1);
			if (inputState->guessing==0) {
				f_AST = astFactory.create(f);
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(f_AST));
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
#line 487 "java.g"
			parameterModifier_AST = RefJavaAST(astFactory.make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory.create(MODIFIERS,"MODIFIERS")))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(f_AST))));
#line 2545 "JavaRecognizer.cpp"
			currentAST.root = parameterModifier_AST;
			if ( parameterModifier_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				parameterModifier_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
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
			consume();
			consumeUntil(_tokenSet_33);
		} else {
			throw ex;
		}
	}
	returnAST = parameterModifier_AST;
}

void JavaRecognizer::parameterDeclaratorBrackets(
	RefJavaAST t
) {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST parameterDeclaratorBrackets_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  lb = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lb_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		if ( inputState->guessing==0 ) {
			parameterDeclaratorBrackets_AST = RefJavaAST(currentAST.root);
#line 481 "java.g"
			parameterDeclaratorBrackets_AST = t;
#line 2582 "JavaRecognizer.cpp"
			currentAST.root = parameterDeclaratorBrackets_AST;
			if ( parameterDeclaratorBrackets_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				parameterDeclaratorBrackets_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = parameterDeclaratorBrackets_AST->getFirstChild();
			else
				currentAST.child = parameterDeclaratorBrackets_AST;
			currentAST.advanceChildToEnd();
		}
		{
		for (;;) {
			if ((LA(1)==LBRACK)) {
				lb = LT(1);
				if (inputState->guessing==0) {
					lb_AST = astFactory.create(lb);
					astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(lb_AST));
				}
				match(LBRACK);
				if ( inputState->guessing==0 ) {
#line 482 "java.g"
					lb_AST->setType(ARRAY_DECLARATOR);
#line 2603 "JavaRecognizer.cpp"
				}
				RefJavaAST tmp65_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				tmp65_AST = astFactory.create(LT(1));
				match(RBRACK);
			}
			else {
				goto _loop82;
			}
			
		}
		_loop82:;
		}
		parameterDeclaratorBrackets_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_32);
		} else {
			throw ex;
		}
	}
	returnAST = parameterDeclaratorBrackets_AST;
}

void JavaRecognizer::statement() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST statement_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  c = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST c_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  s = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST s_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case LCURLY:
		{
			compoundStatement();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			statement_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_class:
		{
			classDefinition( static_cast<RefJavaAST>(astFactory.create(MODIFIERS,"MODIFIERS")) );
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			statement_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_if:
		{
			RefJavaAST tmp66_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp66_AST = astFactory.create(LT(1));
				astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp66_AST));
			}
			match(LITERAL_if);
			RefJavaAST tmp67_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp67_AST = astFactory.create(LT(1));
			match(LPAREN);
			expression();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			RefJavaAST tmp68_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp68_AST = astFactory.create(LT(1));
			match(RPAREN);
			statement();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{
			if ((LA(1)==LITERAL_else) && (_tokenSet_21.member(LA(2)))) {
				RefJavaAST tmp69_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				tmp69_AST = astFactory.create(LT(1));
				match(LITERAL_else);
				statement();
				if (inputState->guessing==0) {
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else if ((_tokenSet_34.member(LA(1))) && (_tokenSet_35.member(LA(2)))) {
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
			RefJavaAST tmp70_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp70_AST = astFactory.create(LT(1));
				astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp70_AST));
			}
			match(LITERAL_for);
			RefJavaAST tmp71_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp71_AST = astFactory.create(LT(1));
			match(LPAREN);
			forInit();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			RefJavaAST tmp72_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp72_AST = astFactory.create(LT(1));
			match(SEMI);
			forCond();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			RefJavaAST tmp73_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp73_AST = astFactory.create(LT(1));
			match(SEMI);
			forIter();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			RefJavaAST tmp74_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp74_AST = astFactory.create(LT(1));
			match(RPAREN);
			statement();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			statement_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_while:
		{
			RefJavaAST tmp75_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp75_AST = astFactory.create(LT(1));
				astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp75_AST));
			}
			match(LITERAL_while);
			RefJavaAST tmp76_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp76_AST = astFactory.create(LT(1));
			match(LPAREN);
			expression();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			RefJavaAST tmp77_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp77_AST = astFactory.create(LT(1));
			match(RPAREN);
			statement();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			statement_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_do:
		{
			RefJavaAST tmp78_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp78_AST = astFactory.create(LT(1));
				astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp78_AST));
			}
			match(LITERAL_do);
			statement();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			RefJavaAST tmp79_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp79_AST = astFactory.create(LT(1));
			match(LITERAL_while);
			RefJavaAST tmp80_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp80_AST = astFactory.create(LT(1));
			match(LPAREN);
			expression();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			RefJavaAST tmp81_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp81_AST = astFactory.create(LT(1));
			match(RPAREN);
			RefJavaAST tmp82_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp82_AST = astFactory.create(LT(1));
			match(SEMI);
			statement_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_break:
		{
			RefJavaAST tmp83_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp83_AST = astFactory.create(LT(1));
				astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp83_AST));
			}
			match(LITERAL_break);
			{
			switch ( LA(1)) {
			case IDENT:
			{
				RefJavaAST tmp84_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if (inputState->guessing==0) {
					tmp84_AST = astFactory.create(LT(1));
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp84_AST));
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
			RefJavaAST tmp85_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp85_AST = astFactory.create(LT(1));
			match(SEMI);
			statement_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_continue:
		{
			RefJavaAST tmp86_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp86_AST = astFactory.create(LT(1));
				astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp86_AST));
			}
			match(LITERAL_continue);
			{
			switch ( LA(1)) {
			case IDENT:
			{
				RefJavaAST tmp87_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if (inputState->guessing==0) {
					tmp87_AST = astFactory.create(LT(1));
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp87_AST));
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
			RefJavaAST tmp88_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp88_AST = astFactory.create(LT(1));
			match(SEMI);
			statement_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_return:
		{
			RefJavaAST tmp89_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp89_AST = astFactory.create(LT(1));
				astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp89_AST));
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
			case PLUS:
			case MINUS:
			case INC:
			case DEC:
			case BNOT:
			case LNOT:
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
			{
				expression();
				if (inputState->guessing==0) {
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
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
			RefJavaAST tmp90_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp90_AST = astFactory.create(LT(1));
			match(SEMI);
			statement_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_switch:
		{
			RefJavaAST tmp91_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp91_AST = astFactory.create(LT(1));
				astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp91_AST));
			}
			match(LITERAL_switch);
			RefJavaAST tmp92_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp92_AST = astFactory.create(LT(1));
			match(LPAREN);
			expression();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			RefJavaAST tmp93_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp93_AST = astFactory.create(LT(1));
			match(RPAREN);
			RefJavaAST tmp94_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp94_AST = astFactory.create(LT(1));
			match(LCURLY);
			{
			for (;;) {
				if ((LA(1)==LITERAL_case||LA(1)==LITERAL_default)) {
					casesGroup();
					if (inputState->guessing==0) {
						astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
				}
				else {
					goto _loop96;
				}
				
			}
			_loop96:;
			}
			RefJavaAST tmp95_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp95_AST = astFactory.create(LT(1));
			match(RCURLY);
			statement_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_try:
		{
			tryBlock();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			statement_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_throw:
		{
			RefJavaAST tmp96_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp96_AST = astFactory.create(LT(1));
				astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp96_AST));
			}
			match(LITERAL_throw);
			expression();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			RefJavaAST tmp97_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp97_AST = astFactory.create(LT(1));
			match(SEMI);
			statement_AST = RefJavaAST(currentAST.root);
			break;
		}
		case SEMI:
		{
			s = LT(1);
			if (inputState->guessing==0) {
				s_AST = astFactory.create(s);
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(s_AST));
			}
			match(SEMI);
			if ( inputState->guessing==0 ) {
#line 586 "java.g"
				s_AST->setType(EMPTY_STAT);
#line 3008 "JavaRecognizer.cpp"
			}
			statement_AST = RefJavaAST(currentAST.root);
			break;
		}
		default:
			if ((LA(1)==FINAL) && (LA(2)==LITERAL_class)) {
				RefJavaAST tmp98_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				tmp98_AST = astFactory.create(LT(1));
				match(FINAL);
				classDefinition(RefJavaAST(astFactory.make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory.create(MODIFIERS,"MODIFIERS")))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory.create(FINAL,"final"))))));
				if (inputState->guessing==0) {
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				statement_AST = RefJavaAST(currentAST.root);
			}
			else if ((LA(1)==ABSTRACT) && (LA(2)==LITERAL_class)) {
				RefJavaAST tmp99_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				tmp99_AST = astFactory.create(LT(1));
				match(ABSTRACT);
				classDefinition(RefJavaAST(astFactory.make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory.create(MODIFIERS,"MODIFIERS")))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory.create(ABSTRACT,"abstract"))))));
				if (inputState->guessing==0) {
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				statement_AST = RefJavaAST(currentAST.root);
			}
			else {
				bool synPredMatched90 = false;
				if (((_tokenSet_36.member(LA(1))) && (_tokenSet_37.member(LA(2))))) {
					int _m90 = mark();
					synPredMatched90 = true;
					inputState->guessing++;
					try {
						{
						declaration();
						}
					}
					catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
						synPredMatched90 = false;
					}
					rewind(_m90);
					inputState->guessing--;
				}
				if ( synPredMatched90 ) {
					declaration();
					if (inputState->guessing==0) {
						astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					RefJavaAST tmp100_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					tmp100_AST = astFactory.create(LT(1));
					match(SEMI);
					statement_AST = RefJavaAST(currentAST.root);
				}
				else if ((_tokenSet_38.member(LA(1))) && (_tokenSet_39.member(LA(2)))) {
					expression();
					if (inputState->guessing==0) {
						astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					RefJavaAST tmp101_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					tmp101_AST = astFactory.create(LT(1));
					match(SEMI);
					statement_AST = RefJavaAST(currentAST.root);
				}
				else if ((LA(1)==IDENT) && (LA(2)==COLON)) {
					RefJavaAST tmp102_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if (inputState->guessing==0) {
						tmp102_AST = astFactory.create(LT(1));
						astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp102_AST));
					}
					match(IDENT);
					c = LT(1);
					if (inputState->guessing==0) {
						c_AST = astFactory.create(c);
						astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(c_AST));
					}
					match(COLON);
					if ( inputState->guessing==0 ) {
#line 532 "java.g"
						c_AST->setType(LABELED_STAT);
#line 3087 "JavaRecognizer.cpp"
					}
					statement();
					if (inputState->guessing==0) {
						astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					statement_AST = RefJavaAST(currentAST.root);
				}
				else if ((LA(1)==LITERAL_synchronized) && (LA(2)==LPAREN)) {
					RefJavaAST tmp103_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if (inputState->guessing==0) {
						tmp103_AST = astFactory.create(LT(1));
						astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp103_AST));
					}
					match(LITERAL_synchronized);
					RefJavaAST tmp104_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					tmp104_AST = astFactory.create(LT(1));
					match(LPAREN);
					expression();
					if (inputState->guessing==0) {
						astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					RefJavaAST tmp105_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					tmp105_AST = astFactory.create(LT(1));
					match(RPAREN);
					compoundStatement();
					if (inputState->guessing==0) {
						astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					statement_AST = RefJavaAST(currentAST.root);
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
			consumeUntil(_tokenSet_34);
		} else {
			throw ex;
		}
	}
	returnAST = statement_AST;
}

void JavaRecognizer::forInit() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST forInit_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		bool synPredMatched108 = false;
		if (((_tokenSet_36.member(LA(1))) && (_tokenSet_37.member(LA(2))))) {
			int _m108 = mark();
			synPredMatched108 = true;
			inputState->guessing++;
			try {
				{
				declaration();
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched108 = false;
			}
			rewind(_m108);
			inputState->guessing--;
		}
		if ( synPredMatched108 ) {
			declaration();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
		}
		else if ((_tokenSet_38.member(LA(1))) && (_tokenSet_40.member(LA(2)))) {
			expressionList();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
		}
		else if ((LA(1)==SEMI)) {
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
		if ( inputState->guessing==0 ) {
			forInit_AST = RefJavaAST(currentAST.root);
#line 621 "java.g"
			forInit_AST = RefJavaAST(astFactory.make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory.create(FOR_INIT,"FOR_INIT")))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(forInit_AST))));
#line 3181 "JavaRecognizer.cpp"
			currentAST.root = forInit_AST;
			if ( forInit_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				forInit_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
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
			consume();
			consumeUntil(_tokenSet_5);
		} else {
			throw ex;
		}
	}
	returnAST = forInit_AST;
}

void JavaRecognizer::forCond() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST forCond_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
		case PLUS:
		case MINUS:
		case INC:
		case DEC:
		case BNOT:
		case LNOT:
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
		{
			expression();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
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
#line 626 "java.g"
			forCond_AST = RefJavaAST(astFactory.make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory.create(FOR_CONDITION,"FOR_CONDITION")))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(forCond_AST))));
#line 3260 "JavaRecognizer.cpp"
			currentAST.root = forCond_AST;
			if ( forCond_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				forCond_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
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
			consume();
			consumeUntil(_tokenSet_5);
		} else {
			throw ex;
		}
	}
	returnAST = forCond_AST;
}

void JavaRecognizer::forIter() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST forIter_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
		case PLUS:
		case MINUS:
		case INC:
		case DEC:
		case BNOT:
		case LNOT:
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
		{
			expressionList();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
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
#line 631 "java.g"
			forIter_AST = RefJavaAST(astFactory.make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory.create(FOR_ITERATOR,"FOR_ITERATOR")))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(forIter_AST))));
#line 3339 "JavaRecognizer.cpp"
			currentAST.root = forIter_AST;
			if ( forIter_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				forIter_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
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
			consume();
			consumeUntil(_tokenSet_23);
		} else {
			throw ex;
		}
	}
	returnAST = forIter_AST;
}

void JavaRecognizer::casesGroup() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST casesGroup_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		int _cnt99=0;
		for (;;) {
			if ((LA(1)==LITERAL_case||LA(1)==LITERAL_default) && (_tokenSet_41.member(LA(2)))) {
				aCase();
				if (inputState->guessing==0) {
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				if ( _cnt99>=1 ) { goto _loop99; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
			}
			
			_cnt99++;
		}
		_loop99:;
		}
		caseSList();
		if (inputState->guessing==0) {
			astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if ( inputState->guessing==0 ) {
			casesGroup_AST = RefJavaAST(currentAST.root);
#line 602 "java.g"
			casesGroup_AST = RefJavaAST(astFactory.make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory.create(CASE_GROUP,"CASE_GROUP")))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(casesGroup_AST))));
#line 3393 "JavaRecognizer.cpp"
			currentAST.root = casesGroup_AST;
			if ( casesGroup_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				casesGroup_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
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
			consume();
			consumeUntil(_tokenSet_42);
		} else {
			throw ex;
		}
	}
	returnAST = casesGroup_AST;
}

void JavaRecognizer::tryBlock() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST tryBlock_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefJavaAST tmp106_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if (inputState->guessing==0) {
			tmp106_AST = astFactory.create(LT(1));
			astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp106_AST));
		}
		match(LITERAL_try);
		compoundStatement();
		if (inputState->guessing==0) {
			astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		for (;;) {
			if ((LA(1)==LITERAL_catch)) {
				handler();
				if (inputState->guessing==0) {
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop115;
			}
			
		}
		_loop115:;
		}
		{
		switch ( LA(1)) {
		case LITERAL_finally:
		{
			RefJavaAST tmp107_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp107_AST = astFactory.create(LT(1));
				astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp107_AST));
			}
			match(LITERAL_finally);
			compoundStatement();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case FINAL:
		case ABSTRACT:
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
			consume();
			consumeUntil(_tokenSet_34);
		} else {
			throw ex;
		}
	}
	returnAST = tryBlock_AST;
}

void JavaRecognizer::aCase() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST aCase_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LITERAL_case:
		{
			RefJavaAST tmp108_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp108_AST = astFactory.create(LT(1));
				astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp108_AST));
			}
			match(LITERAL_case);
			expression();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case LITERAL_default:
		{
			RefJavaAST tmp109_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp109_AST = astFactory.create(LT(1));
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp109_AST));
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
		RefJavaAST tmp110_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp110_AST = astFactory.create(LT(1));
		match(COLON);
		aCase_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_43);
		} else {
			throw ex;
		}
	}
	returnAST = aCase_AST;
}

void JavaRecognizer::caseSList() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST caseSList_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		for (;;) {
			if ((_tokenSet_21.member(LA(1)))) {
				statement();
				if (inputState->guessing==0) {
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop104;
			}
			
		}
		_loop104:;
		}
		if ( inputState->guessing==0 ) {
			caseSList_AST = RefJavaAST(currentAST.root);
#line 611 "java.g"
			caseSList_AST = RefJavaAST(astFactory.make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory.create(SLIST,"SLIST")))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(caseSList_AST))));
#line 3621 "JavaRecognizer.cpp"
			currentAST.root = caseSList_AST;
			if ( caseSList_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				caseSList_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
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
			consume();
			consumeUntil(_tokenSet_42);
		} else {
			throw ex;
		}
	}
	returnAST = caseSList_AST;
}

void JavaRecognizer::expressionList() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST expressionList_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		expression();
		if (inputState->guessing==0) {
			astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		for (;;) {
			if ((LA(1)==COMMA)) {
				RefJavaAST tmp111_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				tmp111_AST = astFactory.create(LT(1));
				match(COMMA);
				expression();
				if (inputState->guessing==0) {
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop121;
			}
			
		}
		_loop121:;
		}
		if ( inputState->guessing==0 ) {
			expressionList_AST = RefJavaAST(currentAST.root);
#line 692 "java.g"
			expressionList_AST = RefJavaAST(astFactory.make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(astFactory.create(ELIST,"ELIST")))->add(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(expressionList_AST))));
#line 3676 "JavaRecognizer.cpp"
			currentAST.root = expressionList_AST;
			if ( expressionList_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				expressionList_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
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
			consume();
			consumeUntil(_tokenSet_44);
		} else {
			throw ex;
		}
	}
	returnAST = expressionList_AST;
}

void JavaRecognizer::handler() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST handler_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefJavaAST tmp112_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if (inputState->guessing==0) {
			tmp112_AST = astFactory.create(LT(1));
			astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp112_AST));
		}
		match(LITERAL_catch);
		RefJavaAST tmp113_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp113_AST = astFactory.create(LT(1));
		match(LPAREN);
		parameterDeclaration();
		if (inputState->guessing==0) {
			astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		RefJavaAST tmp114_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		tmp114_AST = astFactory.create(LT(1));
		match(RPAREN);
		compoundStatement();
		if (inputState->guessing==0) {
			astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		handler_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_45);
		} else {
			throw ex;
		}
	}
	returnAST = handler_AST;
}

void JavaRecognizer::assignmentExpression() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST assignmentExpression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		conditionalExpression();
		if (inputState->guessing==0) {
			astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
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
				RefJavaAST tmp115_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if (inputState->guessing==0) {
					tmp115_AST = astFactory.create(LT(1));
					astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp115_AST));
				}
				match(ASSIGN);
				break;
			}
			case PLUS_ASSIGN:
			{
				RefJavaAST tmp116_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if (inputState->guessing==0) {
					tmp116_AST = astFactory.create(LT(1));
					astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp116_AST));
				}
				match(PLUS_ASSIGN);
				break;
			}
			case MINUS_ASSIGN:
			{
				RefJavaAST tmp117_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if (inputState->guessing==0) {
					tmp117_AST = astFactory.create(LT(1));
					astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp117_AST));
				}
				match(MINUS_ASSIGN);
				break;
			}
			case STAR_ASSIGN:
			{
				RefJavaAST tmp118_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if (inputState->guessing==0) {
					tmp118_AST = astFactory.create(LT(1));
					astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp118_AST));
				}
				match(STAR_ASSIGN);
				break;
			}
			case DIV_ASSIGN:
			{
				RefJavaAST tmp119_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if (inputState->guessing==0) {
					tmp119_AST = astFactory.create(LT(1));
					astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp119_AST));
				}
				match(DIV_ASSIGN);
				break;
			}
			case MOD_ASSIGN:
			{
				RefJavaAST tmp120_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if (inputState->guessing==0) {
					tmp120_AST = astFactory.create(LT(1));
					astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp120_AST));
				}
				match(MOD_ASSIGN);
				break;
			}
			case SR_ASSIGN:
			{
				RefJavaAST tmp121_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if (inputState->guessing==0) {
					tmp121_AST = astFactory.create(LT(1));
					astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp121_AST));
				}
				match(SR_ASSIGN);
				break;
			}
			case BSR_ASSIGN:
			{
				RefJavaAST tmp122_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if (inputState->guessing==0) {
					tmp122_AST = astFactory.create(LT(1));
					astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp122_AST));
				}
				match(BSR_ASSIGN);
				break;
			}
			case SL_ASSIGN:
			{
				RefJavaAST tmp123_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if (inputState->guessing==0) {
					tmp123_AST = astFactory.create(LT(1));
					astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp123_AST));
				}
				match(SL_ASSIGN);
				break;
			}
			case BAND_ASSIGN:
			{
				RefJavaAST tmp124_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if (inputState->guessing==0) {
					tmp124_AST = astFactory.create(LT(1));
					astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp124_AST));
				}
				match(BAND_ASSIGN);
				break;
			}
			case BXOR_ASSIGN:
			{
				RefJavaAST tmp125_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if (inputState->guessing==0) {
					tmp125_AST = astFactory.create(LT(1));
					astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp125_AST));
				}
				match(BXOR_ASSIGN);
				break;
			}
			case BOR_ASSIGN:
			{
				RefJavaAST tmp126_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if (inputState->guessing==0) {
					tmp126_AST = astFactory.create(LT(1));
					astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp126_AST));
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
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
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
			consume();
			consumeUntil(_tokenSet_31);
		} else {
			throw ex;
		}
	}
	returnAST = assignmentExpression_AST;
}

void JavaRecognizer::conditionalExpression() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST conditionalExpression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		logicalOrExpression();
		if (inputState->guessing==0) {
			astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case QUESTION:
		{
			RefJavaAST tmp127_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp127_AST = astFactory.create(LT(1));
				astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp127_AST));
			}
			match(QUESTION);
			assignmentExpression();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			RefJavaAST tmp128_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp128_AST = astFactory.create(LT(1));
			match(COLON);
			conditionalExpression();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
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
			consume();
			consumeUntil(_tokenSet_46);
		} else {
			throw ex;
		}
	}
	returnAST = conditionalExpression_AST;
}

void JavaRecognizer::logicalOrExpression() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST logicalOrExpression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		logicalAndExpression();
		if (inputState->guessing==0) {
			astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		for (;;) {
			if ((LA(1)==LOR)) {
				RefJavaAST tmp129_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if (inputState->guessing==0) {
					tmp129_AST = astFactory.create(LT(1));
					astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp129_AST));
				}
				match(LOR);
				logicalAndExpression();
				if (inputState->guessing==0) {
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop129;
			}
			
		}
		_loop129:;
		}
		logicalOrExpression_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_47);
		} else {
			throw ex;
		}
	}
	returnAST = logicalOrExpression_AST;
}

void JavaRecognizer::logicalAndExpression() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST logicalAndExpression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		inclusiveOrExpression();
		if (inputState->guessing==0) {
			astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		for (;;) {
			if ((LA(1)==LAND)) {
				RefJavaAST tmp130_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if (inputState->guessing==0) {
					tmp130_AST = astFactory.create(LT(1));
					astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp130_AST));
				}
				match(LAND);
				inclusiveOrExpression();
				if (inputState->guessing==0) {
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop132;
			}
			
		}
		_loop132:;
		}
		logicalAndExpression_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_48);
		} else {
			throw ex;
		}
	}
	returnAST = logicalAndExpression_AST;
}

void JavaRecognizer::inclusiveOrExpression() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST inclusiveOrExpression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		exclusiveOrExpression();
		if (inputState->guessing==0) {
			astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		for (;;) {
			if ((LA(1)==BOR)) {
				RefJavaAST tmp131_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if (inputState->guessing==0) {
					tmp131_AST = astFactory.create(LT(1));
					astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp131_AST));
				}
				match(BOR);
				exclusiveOrExpression();
				if (inputState->guessing==0) {
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop135;
			}
			
		}
		_loop135:;
		}
		inclusiveOrExpression_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_49);
		} else {
			throw ex;
		}
	}
	returnAST = inclusiveOrExpression_AST;
}

void JavaRecognizer::exclusiveOrExpression() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST exclusiveOrExpression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		andExpression();
		if (inputState->guessing==0) {
			astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		for (;;) {
			if ((LA(1)==BXOR)) {
				RefJavaAST tmp132_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if (inputState->guessing==0) {
					tmp132_AST = astFactory.create(LT(1));
					astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp132_AST));
				}
				match(BXOR);
				andExpression();
				if (inputState->guessing==0) {
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop138;
			}
			
		}
		_loop138:;
		}
		exclusiveOrExpression_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_50);
		} else {
			throw ex;
		}
	}
	returnAST = exclusiveOrExpression_AST;
}

void JavaRecognizer::andExpression() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST andExpression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		equalityExpression();
		if (inputState->guessing==0) {
			astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		for (;;) {
			if ((LA(1)==BAND)) {
				RefJavaAST tmp133_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if (inputState->guessing==0) {
					tmp133_AST = astFactory.create(LT(1));
					astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp133_AST));
				}
				match(BAND);
				equalityExpression();
				if (inputState->guessing==0) {
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop141;
			}
			
		}
		_loop141:;
		}
		andExpression_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_51);
		} else {
			throw ex;
		}
	}
	returnAST = andExpression_AST;
}

void JavaRecognizer::equalityExpression() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST equalityExpression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		relationalExpression();
		if (inputState->guessing==0) {
			astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		for (;;) {
			if ((LA(1)==NOT_EQUAL||LA(1)==EQUAL)) {
				{
				switch ( LA(1)) {
				case NOT_EQUAL:
				{
					RefJavaAST tmp134_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if (inputState->guessing==0) {
						tmp134_AST = astFactory.create(LT(1));
						astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp134_AST));
					}
					match(NOT_EQUAL);
					break;
				}
				case EQUAL:
				{
					RefJavaAST tmp135_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if (inputState->guessing==0) {
						tmp135_AST = astFactory.create(LT(1));
						astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp135_AST));
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
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop145;
			}
			
		}
		_loop145:;
		}
		equalityExpression_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_52);
		} else {
			throw ex;
		}
	}
	returnAST = equalityExpression_AST;
}

void JavaRecognizer::relationalExpression() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST relationalExpression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		shiftExpression();
		if (inputState->guessing==0) {
			astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
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
			{
			for (;;) {
				if (((LA(1) >= LT_ && LA(1) <= GE))) {
					{
					switch ( LA(1)) {
					case LT_:
					{
						RefJavaAST tmp136_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
						if (inputState->guessing==0) {
							tmp136_AST = astFactory.create(LT(1));
							astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp136_AST));
						}
						match(LT_);
						break;
					}
					case GT:
					{
						RefJavaAST tmp137_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
						if (inputState->guessing==0) {
							tmp137_AST = astFactory.create(LT(1));
							astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp137_AST));
						}
						match(GT);
						break;
					}
					case LE:
					{
						RefJavaAST tmp138_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
						if (inputState->guessing==0) {
							tmp138_AST = astFactory.create(LT(1));
							astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp138_AST));
						}
						match(LE);
						break;
					}
					case GE:
					{
						RefJavaAST tmp139_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
						if (inputState->guessing==0) {
							tmp139_AST = astFactory.create(LT(1));
							astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp139_AST));
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
						astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
				}
				else {
					goto _loop150;
				}
				
			}
			_loop150:;
			}
			break;
		}
		case LITERAL_instanceof:
		{
			RefJavaAST tmp140_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp140_AST = astFactory.create(LT(1));
				astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp140_AST));
			}
			match(LITERAL_instanceof);
			typeSpec(true);
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
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
			consume();
			consumeUntil(_tokenSet_53);
		} else {
			throw ex;
		}
	}
	returnAST = relationalExpression_AST;
}

void JavaRecognizer::shiftExpression() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST shiftExpression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		additiveExpression();
		if (inputState->guessing==0) {
			astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		for (;;) {
			if (((LA(1) >= SL && LA(1) <= BSR))) {
				{
				switch ( LA(1)) {
				case SL:
				{
					RefJavaAST tmp141_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if (inputState->guessing==0) {
						tmp141_AST = astFactory.create(LT(1));
						astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp141_AST));
					}
					match(SL);
					break;
				}
				case SR:
				{
					RefJavaAST tmp142_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if (inputState->guessing==0) {
						tmp142_AST = astFactory.create(LT(1));
						astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp142_AST));
					}
					match(SR);
					break;
				}
				case BSR:
				{
					RefJavaAST tmp143_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if (inputState->guessing==0) {
						tmp143_AST = astFactory.create(LT(1));
						astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp143_AST));
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
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop154;
			}
			
		}
		_loop154:;
		}
		shiftExpression_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_54);
		} else {
			throw ex;
		}
	}
	returnAST = shiftExpression_AST;
}

void JavaRecognizer::additiveExpression() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST additiveExpression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		multiplicativeExpression();
		if (inputState->guessing==0) {
			astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		for (;;) {
			if ((LA(1)==PLUS||LA(1)==MINUS)) {
				{
				switch ( LA(1)) {
				case PLUS:
				{
					RefJavaAST tmp144_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if (inputState->guessing==0) {
						tmp144_AST = astFactory.create(LT(1));
						astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp144_AST));
					}
					match(PLUS);
					break;
				}
				case MINUS:
				{
					RefJavaAST tmp145_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if (inputState->guessing==0) {
						tmp145_AST = astFactory.create(LT(1));
						astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp145_AST));
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
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop158;
			}
			
		}
		_loop158:;
		}
		additiveExpression_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_55);
		} else {
			throw ex;
		}
	}
	returnAST = additiveExpression_AST;
}

void JavaRecognizer::multiplicativeExpression() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST multiplicativeExpression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		unaryExpression();
		if (inputState->guessing==0) {
			astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		for (;;) {
			if ((_tokenSet_56.member(LA(1)))) {
				{
				switch ( LA(1)) {
				case STAR:
				{
					RefJavaAST tmp146_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if (inputState->guessing==0) {
						tmp146_AST = astFactory.create(LT(1));
						astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp146_AST));
					}
					match(STAR);
					break;
				}
				case DIV:
				{
					RefJavaAST tmp147_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if (inputState->guessing==0) {
						tmp147_AST = astFactory.create(LT(1));
						astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp147_AST));
					}
					match(DIV);
					break;
				}
				case MOD:
				{
					RefJavaAST tmp148_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if (inputState->guessing==0) {
						tmp148_AST = astFactory.create(LT(1));
						astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp148_AST));
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
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop162;
			}
			
		}
		_loop162:;
		}
		multiplicativeExpression_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_57);
		} else {
			throw ex;
		}
	}
	returnAST = multiplicativeExpression_AST;
}

void JavaRecognizer::unaryExpression() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST unaryExpression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case INC:
		{
			RefJavaAST tmp149_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp149_AST = astFactory.create(LT(1));
				astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp149_AST));
			}
			match(INC);
			unaryExpression();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			unaryExpression_AST = RefJavaAST(currentAST.root);
			break;
		}
		case DEC:
		{
			RefJavaAST tmp150_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp150_AST = astFactory.create(LT(1));
				astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp150_AST));
			}
			match(DEC);
			unaryExpression();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			unaryExpression_AST = RefJavaAST(currentAST.root);
			break;
		}
		case MINUS:
		{
			RefJavaAST tmp151_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp151_AST = astFactory.create(LT(1));
				astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp151_AST));
			}
			match(MINUS);
			if ( inputState->guessing==0 ) {
#line 795 "java.g"
				tmp151_AST->setType(UNARY_MINUS);
#line 4704 "JavaRecognizer.cpp"
			}
			unaryExpression();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			unaryExpression_AST = RefJavaAST(currentAST.root);
			break;
		}
		case PLUS:
		{
			RefJavaAST tmp152_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp152_AST = astFactory.create(LT(1));
				astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp152_AST));
			}
			match(PLUS);
			if ( inputState->guessing==0 ) {
#line 796 "java.g"
				tmp152_AST->setType(UNARY_PLUS);
#line 4724 "JavaRecognizer.cpp"
			}
			unaryExpression();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
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
		case BNOT:
		case LNOT:
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
		{
			unaryExpressionNotPlusMinus();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
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
			consume();
			consumeUntil(_tokenSet_58);
		} else {
			throw ex;
		}
	}
	returnAST = unaryExpression_AST;
}

void JavaRecognizer::unaryExpressionNotPlusMinus() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST unaryExpressionNotPlusMinus_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  lpb = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lpb_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  lp = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lp_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case BNOT:
		{
			RefJavaAST tmp153_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp153_AST = astFactory.create(LT(1));
				astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp153_AST));
			}
			match(BNOT);
			unaryExpression();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			unaryExpressionNotPlusMinus_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LNOT:
		{
			RefJavaAST tmp154_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp154_AST = astFactory.create(LT(1));
				astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp154_AST));
			}
			match(LNOT);
			unaryExpression();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
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
		{
			{
			if ((LA(1)==LPAREN) && ((LA(2) >= LITERAL_void && LA(2) <= LITERAL_double))) {
				lpb = LT(1);
				if (inputState->guessing==0) {
					lpb_AST = astFactory.create(lpb);
					astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(lpb_AST));
				}
				match(LPAREN);
				if ( inputState->guessing==0 ) {
#line 812 "java.g"
					lpb_AST->setType(TYPECAST);
#line 4856 "JavaRecognizer.cpp"
				}
				builtInTypeSpec(true);
				if (inputState->guessing==0) {
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				RefJavaAST tmp155_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				tmp155_AST = astFactory.create(LT(1));
				match(RPAREN);
				unaryExpression();
				if (inputState->guessing==0) {
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				bool synPredMatched167 = false;
				if (((LA(1)==LPAREN) && (LA(2)==IDENT))) {
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
					if (inputState->guessing==0) {
						lp_AST = astFactory.create(lp);
						astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(lp_AST));
					}
					match(LPAREN);
					if ( inputState->guessing==0 ) {
#line 819 "java.g"
						lp_AST->setType(TYPECAST);
#line 4900 "JavaRecognizer.cpp"
					}
					classTypeSpec(true);
					if (inputState->guessing==0) {
						astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					RefJavaAST tmp156_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					tmp156_AST = astFactory.create(LT(1));
					match(RPAREN);
					unaryExpressionNotPlusMinus();
					if (inputState->guessing==0) {
						astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
				}
				else if ((_tokenSet_59.member(LA(1))) && (_tokenSet_60.member(LA(2)))) {
					postfixExpression();
					if (inputState->guessing==0) {
						astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
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
			consume();
			consumeUntil(_tokenSet_58);
		} else {
			throw ex;
		}
	}
	returnAST = unaryExpressionNotPlusMinus_AST;
}

void JavaRecognizer::postfixExpression() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST postfixExpression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  lbc = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lbc_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  lb = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lb_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  lp = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lp_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  in = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST in_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  de = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST de_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  lbt = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lbt_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
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
		{
			primaryExpression();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{
			for (;;) {
				switch ( LA(1)) {
				case DOT:
				{
					RefJavaAST tmp157_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if (inputState->guessing==0) {
						tmp157_AST = astFactory.create(LT(1));
						astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp157_AST));
					}
					match(DOT);
					{
					switch ( LA(1)) {
					case IDENT:
					{
						RefJavaAST tmp158_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
						if (inputState->guessing==0) {
							tmp158_AST = astFactory.create(LT(1));
							astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp158_AST));
						}
						match(IDENT);
						break;
					}
					case LITERAL_this:
					{
						RefJavaAST tmp159_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
						if (inputState->guessing==0) {
							tmp159_AST = astFactory.create(LT(1));
							astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp159_AST));
						}
						match(LITERAL_this);
						break;
					}
					case LITERAL_class:
					{
						RefJavaAST tmp160_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
						if (inputState->guessing==0) {
							tmp160_AST = astFactory.create(LT(1));
							astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp160_AST));
						}
						match(LITERAL_class);
						break;
					}
					case LITERAL_new:
					{
						newExpression();
						if (inputState->guessing==0) {
							astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
						}
						break;
					}
					case LITERAL_super:
					{
						RefJavaAST tmp161_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
						if (inputState->guessing==0) {
							tmp161_AST = astFactory.create(LT(1));
							astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp161_AST));
						}
						match(LITERAL_super);
						RefJavaAST tmp162_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
						if (inputState->guessing==0) {
							tmp162_AST = astFactory.create(LT(1));
							astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp162_AST));
						}
						match(LPAREN);
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
						case PLUS:
						case MINUS:
						case INC:
						case DEC:
						case BNOT:
						case LNOT:
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
						{
							expressionList();
							if (inputState->guessing==0) {
								astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
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
						RefJavaAST tmp163_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
						if (inputState->guessing==0) {
							tmp163_AST = astFactory.create(LT(1));
							astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp163_AST));
						}
						match(RPAREN);
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
				case LPAREN:
				{
					lp = LT(1);
					if (inputState->guessing==0) {
						lp_AST = astFactory.create(lp);
						astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(lp_AST));
					}
					match(LPAREN);
					if ( inputState->guessing==0 ) {
#line 852 "java.g"
						lp_AST->setType(METHOD_CALL);
#line 5120 "JavaRecognizer.cpp"
					}
					argList();
					if (inputState->guessing==0) {
						astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					RefJavaAST tmp164_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					tmp164_AST = astFactory.create(LT(1));
					match(RPAREN);
					break;
				}
				default:
					if ((LA(1)==LBRACK) && (LA(2)==RBRACK)) {
						{
						int _cnt173=0;
						for (;;) {
							if ((LA(1)==LBRACK)) {
								lbc = LT(1);
								if (inputState->guessing==0) {
									lbc_AST = astFactory.create(lbc);
									astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(lbc_AST));
								}
								match(LBRACK);
								if ( inputState->guessing==0 ) {
#line 841 "java.g"
									lbc_AST->setType(ARRAY_DECLARATOR);
#line 5146 "JavaRecognizer.cpp"
								}
								RefJavaAST tmp165_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
								tmp165_AST = astFactory.create(LT(1));
								match(RBRACK);
							}
							else {
								if ( _cnt173>=1 ) { goto _loop173; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
							}
							
							_cnt173++;
						}
						_loop173:;
						}
						RefJavaAST tmp166_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
						if (inputState->guessing==0) {
							tmp166_AST = astFactory.create(LT(1));
							astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp166_AST));
						}
						match(DOT);
						RefJavaAST tmp167_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
						if (inputState->guessing==0) {
							tmp167_AST = astFactory.create(LT(1));
							astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp167_AST));
						}
						match(LITERAL_class);
					}
					else if ((LA(1)==LBRACK) && (_tokenSet_38.member(LA(2)))) {
						lb = LT(1);
						if (inputState->guessing==0) {
							lb_AST = astFactory.create(lb);
							astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(lb_AST));
						}
						match(LBRACK);
						if ( inputState->guessing==0 ) {
#line 845 "java.g"
							lb_AST->setType(INDEX_OP);
#line 5183 "JavaRecognizer.cpp"
						}
						expression();
						if (inputState->guessing==0) {
							astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
						}
						RefJavaAST tmp168_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
						tmp168_AST = astFactory.create(LT(1));
						match(RBRACK);
					}
				else {
					goto _loop174;
				}
				}
			}
			_loop174:;
			}
			{
			switch ( LA(1)) {
			case INC:
			{
				in = LT(1);
				if (inputState->guessing==0) {
					in_AST = astFactory.create(in);
					astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(in_AST));
				}
				match(INC);
				if ( inputState->guessing==0 ) {
#line 859 "java.g"
					in_AST->setType(POST_INC);
#line 5213 "JavaRecognizer.cpp"
				}
				break;
			}
			case DEC:
			{
				de = LT(1);
				if (inputState->guessing==0) {
					de_AST = astFactory.create(de);
					astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(de_AST));
				}
				match(DEC);
				if ( inputState->guessing==0 ) {
#line 860 "java.g"
					de_AST->setType(POST_DEC);
#line 5228 "JavaRecognizer.cpp"
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
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{
			for (;;) {
				if ((LA(1)==LBRACK)) {
					lbt = LT(1);
					if (inputState->guessing==0) {
						lbt_AST = astFactory.create(lbt);
						astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(lbt_AST));
					}
					match(LBRACK);
					if ( inputState->guessing==0 ) {
#line 866 "java.g"
						lbt_AST->setType(ARRAY_DECLARATOR);
#line 5309 "JavaRecognizer.cpp"
					}
					RefJavaAST tmp169_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
					tmp169_AST = astFactory.create(LT(1));
					match(RBRACK);
				}
				else {
					goto _loop177;
				}
				
			}
			_loop177:;
			}
			RefJavaAST tmp170_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp170_AST = astFactory.create(LT(1));
				astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp170_AST));
			}
			match(DOT);
			RefJavaAST tmp171_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp171_AST = astFactory.create(LT(1));
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp171_AST));
			}
			match(LITERAL_class);
			postfixExpression_AST = RefJavaAST(currentAST.root);
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
			consumeUntil(_tokenSet_58);
		} else {
			throw ex;
		}
	}
	returnAST = postfixExpression_AST;
}

void JavaRecognizer::primaryExpression() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST primaryExpression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case IDENT:
		{
			RefJavaAST tmp172_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp172_AST = astFactory.create(LT(1));
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp172_AST));
			}
			match(IDENT);
			primaryExpression_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_new:
		{
			newExpression();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			primaryExpression_AST = RefJavaAST(currentAST.root);
			break;
		}
		case NUM_INT:
		case CHAR_LITERAL:
		case STRING_LITERAL:
		case NUM_FLOAT:
		{
			constant();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			primaryExpression_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_super:
		{
			RefJavaAST tmp173_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp173_AST = astFactory.create(LT(1));
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp173_AST));
			}
			match(LITERAL_super);
			primaryExpression_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_true:
		{
			RefJavaAST tmp174_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp174_AST = astFactory.create(LT(1));
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp174_AST));
			}
			match(LITERAL_true);
			primaryExpression_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_false:
		{
			RefJavaAST tmp175_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp175_AST = astFactory.create(LT(1));
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp175_AST));
			}
			match(LITERAL_false);
			primaryExpression_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_this:
		{
			RefJavaAST tmp176_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp176_AST = astFactory.create(LT(1));
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp176_AST));
			}
			match(LITERAL_this);
			primaryExpression_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LITERAL_null:
		{
			RefJavaAST tmp177_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp177_AST = astFactory.create(LT(1));
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp177_AST));
			}
			match(LITERAL_null);
			primaryExpression_AST = RefJavaAST(currentAST.root);
			break;
		}
		case LPAREN:
		{
			RefJavaAST tmp178_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp178_AST = astFactory.create(LT(1));
			match(LPAREN);
			assignmentExpression();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			RefJavaAST tmp179_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp179_AST = astFactory.create(LT(1));
			match(RPAREN);
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
			consume();
			consumeUntil(_tokenSet_30);
		} else {
			throw ex;
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
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST newExpression_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefJavaAST tmp180_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if (inputState->guessing==0) {
			tmp180_AST = astFactory.create(LT(1));
			astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp180_AST));
		}
		match(LITERAL_new);
		type();
		if (inputState->guessing==0) {
			astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case LPAREN:
		{
			RefJavaAST tmp181_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp181_AST = astFactory.create(LT(1));
			match(LPAREN);
			argList();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			RefJavaAST tmp182_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			tmp182_AST = astFactory.create(LT(1));
			match(RPAREN);
			{
			switch ( LA(1)) {
			case LCURLY:
			{
				classBlock();
				if (inputState->guessing==0) {
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
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
			case LPAREN:
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
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{
			switch ( LA(1)) {
			case LCURLY:
			{
				arrayInitializer();
				if (inputState->guessing==0) {
					astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
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
			case LPAREN:
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
			consume();
			consumeUntil(_tokenSet_30);
		} else {
			throw ex;
		}
	}
	returnAST = newExpression_AST;
}

void JavaRecognizer::argList() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST argList_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
		case PLUS:
		case MINUS:
		case INC:
		case DEC:
		case BNOT:
		case LNOT:
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
		{
			expressionList();
			if (inputState->guessing==0) {
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case RPAREN:
		{
			if ( inputState->guessing==0 ) {
				argList_AST = RefJavaAST(currentAST.root);
#line 951 "java.g"
				argList_AST = astFactory.create(ELIST,"ELIST");
#line 5765 "JavaRecognizer.cpp"
				currentAST.root = argList_AST;
				if ( argList_AST!=static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
					argList_AST->getFirstChild() != static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST) )
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
			consume();
			consumeUntil(_tokenSet_23);
		} else {
			throw ex;
		}
	}
	returnAST = argList_AST;
}

void JavaRecognizer::constant() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST constant_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case NUM_INT:
		{
			RefJavaAST tmp183_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp183_AST = astFactory.create(LT(1));
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp183_AST));
			}
			match(NUM_INT);
			constant_AST = RefJavaAST(currentAST.root);
			break;
		}
		case CHAR_LITERAL:
		{
			RefJavaAST tmp184_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp184_AST = astFactory.create(LT(1));
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp184_AST));
			}
			match(CHAR_LITERAL);
			constant_AST = RefJavaAST(currentAST.root);
			break;
		}
		case STRING_LITERAL:
		{
			RefJavaAST tmp185_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp185_AST = astFactory.create(LT(1));
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp185_AST));
			}
			match(STRING_LITERAL);
			constant_AST = RefJavaAST(currentAST.root);
			break;
		}
		case NUM_FLOAT:
		{
			RefJavaAST tmp186_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if (inputState->guessing==0) {
				tmp186_AST = astFactory.create(LT(1));
				astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp186_AST));
			}
			match(NUM_FLOAT);
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
			consume();
			consumeUntil(_tokenSet_30);
		} else {
			throw ex;
		}
	}
	returnAST = constant_AST;
}

void JavaRecognizer::newArrayDeclarator() {
	returnAST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefJavaAST newArrayDeclarator_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  lb = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefJavaAST lb_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		int _cnt188=0;
		for (;;) {
			if ((LA(1)==LBRACK) && (_tokenSet_61.member(LA(2)))) {
				lb = LT(1);
				if (inputState->guessing==0) {
					lb_AST = astFactory.create(lb);
					astFactory.makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(lb_AST));
				}
				match(LBRACK);
				if ( inputState->guessing==0 ) {
#line 966 "java.g"
					lb_AST->setType(ARRAY_DECLARATOR);
#line 5886 "JavaRecognizer.cpp"
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
				case PLUS:
				case MINUS:
				case INC:
				case DEC:
				case BNOT:
				case LNOT:
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
				{
					expression();
					if (inputState->guessing==0) {
						astFactory.addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
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
				RefJavaAST tmp187_AST = static_cast<RefJavaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST);
				tmp187_AST = astFactory.create(LT(1));
				match(RBRACK);
			}
			else {
				if ( _cnt188>=1 ) { goto _loop188; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
			}
			
			_cnt188++;
		}
		_loop188:;
		}
		newArrayDeclarator_AST = RefJavaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			consume();
			consumeUntil(_tokenSet_62);
		} else {
			throw ex;
		}
	}
	returnAST = newArrayDeclarator_AST;
}

RefJavaAST JavaRecognizer::getAST()
{
	return returnAST;
}

const char* JavaRecognizer::_tokenNames[] = {
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
	"\"this\"",
	"\"super\"",
	"\"true\"",
	"\"false\"",
	"\"null\"",
	"\"new\"",
	"NUM_INT",
	"CHAR_LITERAL",
	"STRING_LITERAL",
	"NUM_FLOAT",
	"WS_",
	"SL_COMMENT",
	"ML_COMMENT",
	"ESC",
	"HEX_DIGIT",
	"VOCAB",
	"EXPONENT",
	"FLOAT_SUFFIX",
	0
};

const unsigned long JavaRecognizer::_tokenSet_0_data_[] = { 0UL, 4227859840UL, 47UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "final" "abstract" SEMI "private" "public" "protected" "static" "transient" 
// "native" "threadsafe" "synchronized" "volatile" "class" "interface" 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_0(_tokenSet_0_data_,8);
const unsigned long JavaRecognizer::_tokenSet_1_data_[] = { 2UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_1(_tokenSet_1_data_,6);
const unsigned long JavaRecognizer::_tokenSet_2_data_[] = { 2UL, 4227861888UL, 47UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF "final" "abstract" SEMI "import" "private" "public" "protected" 
// "static" "transient" "native" "threadsafe" "synchronized" "volatile" 
// "class" "interface" 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_2(_tokenSet_2_data_,8);
const unsigned long JavaRecognizer::_tokenSet_3_data_[] = { 2UL, 4227859840UL, 47UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF "final" "abstract" SEMI "private" "public" "protected" "static" 
// "transient" "native" "threadsafe" "synchronized" "volatile" "class" 
// "interface" 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_3(_tokenSet_3_data_,8);
const unsigned long JavaRecognizer::_tokenSet_4_data_[] = { 0UL, 8401920UL, 3221249984UL, 131071UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LBRACK RBRACK IDENT LCURLY RCURLY COMMA "implements" LPAREN RPAREN 
// ASSIGN COLON PLUS_ASSIGN MINUS_ASSIGN STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN 
// SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN BXOR_ASSIGN BOR_ASSIGN QUESTION 
// LOR LAND BOR BXOR BAND NOT_EQUAL EQUAL 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_4(_tokenSet_4_data_,8);
const unsigned long JavaRecognizer::_tokenSet_5_data_[] = { 0UL, 1024UL, 0UL, 0UL, 0UL, 0UL };
// SEMI 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_5(_tokenSet_5_data_,6);
const unsigned long JavaRecognizer::_tokenSet_6_data_[] = { 0UL, 4227858816UL, 7UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "final" "abstract" "private" "public" "protected" "static" "transient" 
// "native" "threadsafe" "synchronized" "volatile" 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_6(_tokenSet_6_data_,8);
const unsigned long JavaRecognizer::_tokenSet_7_data_[] = { 0UL, 16760832UL, 40UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "void" "boolean" "byte" "char" "short" "int" "float" "long" "double" 
// IDENT "class" "interface" 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_7(_tokenSet_7_data_,8);
const unsigned long JavaRecognizer::_tokenSet_8_data_[] = { 2UL, 4244620672UL, 268403951UL, 3858759680UL, 2047UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF "final" "abstract" SEMI "void" "boolean" "byte" "char" "short" "int" 
// "float" "long" "double" IDENT "private" "public" "protected" "static" 
// "transient" "native" "threadsafe" "synchronized" "volatile" "class" 
// "interface" LCURLY RCURLY LPAREN "if" "else" "for" "while" "do" "break" 
// "continue" "return" "switch" "throw" "case" "default" "try" PLUS MINUS 
// INC DEC BNOT LNOT "this" "super" "true" "false" "null" "new" NUM_INT 
// CHAR_LITERAL STRING_LITERAL NUM_FLOAT 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_8(_tokenSet_8_data_,12);
const unsigned long JavaRecognizer::_tokenSet_9_data_[] = { 2UL, 4244620672UL, 239UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF "final" "abstract" SEMI "void" "boolean" "byte" "char" "short" "int" 
// "float" "long" "double" IDENT "private" "public" "protected" "static" 
// "transient" "native" "threadsafe" "synchronized" "volatile" "class" 
// "interface" LCURLY RCURLY 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_9(_tokenSet_9_data_,8);
const unsigned long JavaRecognizer::_tokenSet_10_data_[] = { 0UL, 8397824UL, 3221248384UL, 131071UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RBRACK IDENT RCURLY COMMA RPAREN ASSIGN COLON PLUS_ASSIGN MINUS_ASSIGN 
// STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN 
// BXOR_ASSIGN BOR_ASSIGN QUESTION LOR LAND BOR BXOR BAND NOT_EQUAL EQUAL 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_10(_tokenSet_10_data_,8);
const unsigned long JavaRecognizer::_tokenSet_11_data_[] = { 0UL, 4244619648UL, 47UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "final" "abstract" "void" "boolean" "byte" "char" "short" "int" "float" 
// "long" "double" IDENT "private" "public" "protected" "static" "transient" 
// "native" "threadsafe" "synchronized" "volatile" "class" "interface" 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_11(_tokenSet_11_data_,8);
const unsigned long JavaRecognizer::_tokenSet_12_data_[] = { 0UL, 25179136UL, 3221249408UL, 131071UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LBRACK RBRACK IDENT DOT RCURLY COMMA LPAREN RPAREN ASSIGN COLON 
// PLUS_ASSIGN MINUS_ASSIGN STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN 
// BSR_ASSIGN SL_ASSIGN BAND_ASSIGN BXOR_ASSIGN BOR_ASSIGN QUESTION LOR 
// LAND BOR BXOR BAND NOT_EQUAL EQUAL 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_12(_tokenSet_12_data_,8);
const unsigned long JavaRecognizer::_tokenSet_13_data_[] = { 0UL, 4096UL, 1024UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LBRACK LPAREN 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_13(_tokenSet_13_data_,8);
const unsigned long JavaRecognizer::_tokenSet_14_data_[] = { 0UL, 0UL, 576UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LCURLY "implements" 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_14(_tokenSet_14_data_,8);
const unsigned long JavaRecognizer::_tokenSet_15_data_[] = { 0UL, 0UL, 64UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LCURLY 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_15(_tokenSet_15_data_,8);
const unsigned long JavaRecognizer::_tokenSet_16_data_[] = { 2UL, 4294964608UL, 3489652207UL, 4294967295UL, 2047UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF "final" "abstract" SEMI LBRACK RBRACK "void" "boolean" "byte" "char" 
// "short" "int" "float" "long" "double" IDENT DOT STAR "private" "public" 
// "protected" "static" "transient" "native" "threadsafe" "synchronized" 
// "volatile" "class" "interface" LCURLY RCURLY COMMA LPAREN RPAREN ASSIGN 
// COLON "if" "else" "for" "while" "do" "break" "continue" "return" "switch" 
// "throw" "case" "default" "try" PLUS_ASSIGN MINUS_ASSIGN STAR_ASSIGN 
// DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN BXOR_ASSIGN 
// BOR_ASSIGN QUESTION LOR LAND BOR BXOR BAND NOT_EQUAL EQUAL LT_ GT LE 
// GE "instanceof" SL SR BSR PLUS MINUS DIV MOD INC DEC BNOT LNOT "this" 
// "super" "true" "false" "null" "new" NUM_INT CHAR_LITERAL STRING_LITERAL 
// NUM_FLOAT 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_16(_tokenSet_16_data_,12);
const unsigned long JavaRecognizer::_tokenSet_17_data_[] = { 0UL, 4261400960UL, 1071UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "final" "abstract" LBRACK "void" "boolean" "byte" "char" "short" "int" 
// "float" "long" "double" IDENT DOT "private" "public" "protected" "static" 
// "transient" "native" "threadsafe" "synchronized" "volatile" "class" 
// "interface" LPAREN 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_17(_tokenSet_17_data_,8);
const unsigned long JavaRecognizer::_tokenSet_18_data_[] = { 0UL, 25169920UL, 0UL, 0UL, 0UL, 0UL };
// LBRACK IDENT DOT 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_18(_tokenSet_18_data_,6);
const unsigned long JavaRecognizer::_tokenSet_19_data_[] = { 0UL, 5120UL, 4352UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LBRACK COMMA ASSIGN 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_19(_tokenSet_19_data_,8);
const unsigned long JavaRecognizer::_tokenSet_20_data_[] = { 0UL, 4244620672UL, 239UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "final" "abstract" SEMI "void" "boolean" "byte" "char" "short" "int" 
// "float" "long" "double" IDENT "private" "public" "protected" "static" 
// "transient" "native" "threadsafe" "synchronized" "volatile" "class" 
// "interface" LCURLY RCURLY 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_20(_tokenSet_20_data_,8);
const unsigned long JavaRecognizer::_tokenSet_21_data_[] = { 0UL, 4244620672UL, 167674959UL, 3858759680UL, 2047UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "final" "abstract" SEMI "void" "boolean" "byte" "char" "short" "int" 
// "float" "long" "double" IDENT "private" "public" "protected" "static" 
// "transient" "native" "threadsafe" "synchronized" "volatile" "class" 
// LCURLY LPAREN "if" "for" "while" "do" "break" "continue" "return" "switch" 
// "throw" "try" PLUS MINUS INC DEC BNOT LNOT "this" "super" "true" "false" 
// "null" "new" NUM_INT CHAR_LITERAL STRING_LITERAL NUM_FLOAT 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_21(_tokenSet_21_data_,12);
const unsigned long JavaRecognizer::_tokenSet_22_data_[] = { 0UL, 4244620672UL, 1073710319UL, 3858759680UL, 2047UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "final" "abstract" SEMI "void" "boolean" "byte" "char" "short" "int" 
// "float" "long" "double" IDENT "private" "public" "protected" "static" 
// "transient" "native" "threadsafe" "synchronized" "volatile" "class" 
// "interface" LCURLY RCURLY LPAREN "if" "else" "for" "while" "do" "break" 
// "continue" "return" "switch" "throw" "case" "default" "try" "finally" 
// "catch" PLUS MINUS INC DEC BNOT LNOT "this" "super" "true" "false" "null" 
// "new" NUM_INT CHAR_LITERAL STRING_LITERAL NUM_FLOAT 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_22(_tokenSet_22_data_,12);
const unsigned long JavaRecognizer::_tokenSet_23_data_[] = { 0UL, 0UL, 2048UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// RPAREN 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_23(_tokenSet_23_data_,8);
const unsigned long JavaRecognizer::_tokenSet_24_data_[] = { 0UL, 1024UL, 8256UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LCURLY "throws" 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_24(_tokenSet_24_data_,8);
const unsigned long JavaRecognizer::_tokenSet_25_data_[] = { 0UL, 1024UL, 64UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LCURLY 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_25(_tokenSet_25_data_,8);
const unsigned long JavaRecognizer::_tokenSet_26_data_[] = { 0UL, 1024UL, 256UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI COMMA 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_26(_tokenSet_26_data_,8);
const unsigned long JavaRecognizer::_tokenSet_27_data_[] = { 0UL, 1024UL, 4352UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI COMMA ASSIGN 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_27(_tokenSet_27_data_,8);
const unsigned long JavaRecognizer::_tokenSet_28_data_[] = { 0UL, 1024UL, 384UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RCURLY COMMA 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_28(_tokenSet_28_data_,8);
const unsigned long JavaRecognizer::_tokenSet_29_data_[] = { 0UL, 16760832UL, 1088UL, 3858759680UL, 2047UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "void" "boolean" "byte" "char" "short" "int" "float" "long" "double" 
// IDENT LCURLY LPAREN PLUS MINUS INC DEC BNOT LNOT "this" "super" "true" 
// "false" "null" "new" NUM_INT CHAR_LITERAL STRING_LITERAL NUM_FLOAT 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_29(_tokenSet_29_data_,12);
const unsigned long JavaRecognizer::_tokenSet_30_data_[] = { 0UL, 50344960UL, 3221249408UL, 2147483647UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LBRACK RBRACK DOT STAR RCURLY COMMA LPAREN RPAREN ASSIGN COLON 
// PLUS_ASSIGN MINUS_ASSIGN STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN 
// BSR_ASSIGN SL_ASSIGN BAND_ASSIGN BXOR_ASSIGN BOR_ASSIGN QUESTION LOR 
// LAND BOR BXOR BAND NOT_EQUAL EQUAL LT_ GT LE GE "instanceof" SL SR BSR 
// PLUS MINUS DIV MOD INC DEC 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_30(_tokenSet_30_data_,8);
const unsigned long JavaRecognizer::_tokenSet_31_data_[] = { 0UL, 9216UL, 18816UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RBRACK RCURLY COMMA RPAREN COLON 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_31(_tokenSet_31_data_,8);
const unsigned long JavaRecognizer::_tokenSet_32_data_[] = { 0UL, 0UL, 2304UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// COMMA RPAREN 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_32(_tokenSet_32_data_,8);
const unsigned long JavaRecognizer::_tokenSet_33_data_[] = { 0UL, 16760832UL, 0UL, 0UL, 0UL, 0UL };
// "void" "boolean" "byte" "char" "short" "int" "float" "long" "double" 
// IDENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_33(_tokenSet_33_data_,6);
const unsigned long JavaRecognizer::_tokenSet_34_data_[] = { 0UL, 4244620672UL, 268403919UL, 3858759680UL, 2047UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "final" "abstract" SEMI "void" "boolean" "byte" "char" "short" "int" 
// "float" "long" "double" IDENT "private" "public" "protected" "static" 
// "transient" "native" "threadsafe" "synchronized" "volatile" "class" 
// LCURLY RCURLY LPAREN "if" "else" "for" "while" "do" "break" "continue" 
// "return" "switch" "throw" "case" "default" "try" PLUS MINUS INC DEC 
// BNOT LNOT "this" "super" "true" "false" "null" "new" NUM_INT CHAR_LITERAL 
// STRING_LITERAL NUM_FLOAT 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_34(_tokenSet_34_data_,12);
const unsigned long JavaRecognizer::_tokenSet_35_data_[] = { 0UL, 4294956416UL, 4294956271UL, 4294967295UL, 2047UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "final" "abstract" SEMI LBRACK "void" "boolean" "byte" "char" "short" 
// "int" "float" "long" "double" IDENT DOT STAR "private" "public" "protected" 
// "static" "transient" "native" "threadsafe" "synchronized" "volatile" 
// "class" "interface" LCURLY RCURLY LPAREN ASSIGN COLON "if" "else" "for" 
// "while" "do" "break" "continue" "return" "switch" "throw" "case" "default" 
// "try" "finally" "catch" PLUS_ASSIGN MINUS_ASSIGN STAR_ASSIGN DIV_ASSIGN 
// MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN BXOR_ASSIGN BOR_ASSIGN 
// QUESTION LOR LAND BOR BXOR BAND NOT_EQUAL EQUAL LT_ GT LE GE "instanceof" 
// SL SR BSR PLUS MINUS DIV MOD INC DEC BNOT LNOT "this" "super" "true" 
// "false" "null" "new" NUM_INT CHAR_LITERAL STRING_LITERAL NUM_FLOAT 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_35(_tokenSet_35_data_,12);
const unsigned long JavaRecognizer::_tokenSet_36_data_[] = { 0UL, 4244619648UL, 7UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "final" "abstract" "void" "boolean" "byte" "char" "short" "int" "float" 
// "long" "double" IDENT "private" "public" "protected" "static" "transient" 
// "native" "threadsafe" "synchronized" "volatile" 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_36(_tokenSet_36_data_,8);
const unsigned long JavaRecognizer::_tokenSet_37_data_[] = { 0UL, 4261400960UL, 7UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "final" "abstract" LBRACK "void" "boolean" "byte" "char" "short" "int" 
// "float" "long" "double" IDENT DOT "private" "public" "protected" "static" 
// "transient" "native" "threadsafe" "synchronized" "volatile" 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_37(_tokenSet_37_data_,8);
const unsigned long JavaRecognizer::_tokenSet_38_data_[] = { 0UL, 16760832UL, 1024UL, 3858759680UL, 2047UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "void" "boolean" "byte" "char" "short" "int" "float" "long" "double" 
// IDENT LPAREN PLUS MINUS INC DEC BNOT LNOT "this" "super" "true" "false" 
// "null" "new" NUM_INT CHAR_LITERAL STRING_LITERAL NUM_FLOAT 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_38(_tokenSet_38_data_,12);
const unsigned long JavaRecognizer::_tokenSet_39_data_[] = { 0UL, 67097600UL, 3221230592UL, 4294967295UL, 2047UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LBRACK "void" "boolean" "byte" "char" "short" "int" "float" "long" 
// "double" IDENT DOT STAR LPAREN ASSIGN PLUS_ASSIGN MINUS_ASSIGN STAR_ASSIGN 
// DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN BXOR_ASSIGN 
// BOR_ASSIGN QUESTION LOR LAND BOR BXOR BAND NOT_EQUAL EQUAL LT_ GT LE 
// GE "instanceof" SL SR BSR PLUS MINUS DIV MOD INC DEC BNOT LNOT "this" 
// "super" "true" "false" "null" "new" NUM_INT CHAR_LITERAL STRING_LITERAL 
// NUM_FLOAT 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_39(_tokenSet_39_data_,12);
const unsigned long JavaRecognizer::_tokenSet_40_data_[] = { 0UL, 67097600UL, 3221230848UL, 4294967295UL, 2047UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LBRACK "void" "boolean" "byte" "char" "short" "int" "float" "long" 
// "double" IDENT DOT STAR COMMA LPAREN ASSIGN PLUS_ASSIGN MINUS_ASSIGN 
// STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN 
// BXOR_ASSIGN BOR_ASSIGN QUESTION LOR LAND BOR BXOR BAND NOT_EQUAL EQUAL 
// LT_ GT LE GE "instanceof" SL SR BSR PLUS MINUS DIV MOD INC DEC BNOT 
// LNOT "this" "super" "true" "false" "null" "new" NUM_INT CHAR_LITERAL 
// STRING_LITERAL NUM_FLOAT 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_40(_tokenSet_40_data_,12);
const unsigned long JavaRecognizer::_tokenSet_41_data_[] = { 0UL, 16760832UL, 17408UL, 3858759680UL, 2047UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "void" "boolean" "byte" "char" "short" "int" "float" "long" "double" 
// IDENT LPAREN COLON PLUS MINUS INC DEC BNOT LNOT "this" "super" "true" 
// "false" "null" "new" NUM_INT CHAR_LITERAL STRING_LITERAL NUM_FLOAT 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_41(_tokenSet_41_data_,12);
const unsigned long JavaRecognizer::_tokenSet_42_data_[] = { 0UL, 0UL, 100663424UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// RCURLY "case" "default" 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_42(_tokenSet_42_data_,8);
const unsigned long JavaRecognizer::_tokenSet_43_data_[] = { 0UL, 4244620672UL, 268338383UL, 3858759680UL, 2047UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "final" "abstract" SEMI "void" "boolean" "byte" "char" "short" "int" 
// "float" "long" "double" IDENT "private" "public" "protected" "static" 
// "transient" "native" "threadsafe" "synchronized" "volatile" "class" 
// LCURLY RCURLY LPAREN "if" "for" "while" "do" "break" "continue" "return" 
// "switch" "throw" "case" "default" "try" PLUS MINUS INC DEC BNOT LNOT 
// "this" "super" "true" "false" "null" "new" NUM_INT CHAR_LITERAL STRING_LITERAL 
// NUM_FLOAT 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_43(_tokenSet_43_data_,12);
const unsigned long JavaRecognizer::_tokenSet_44_data_[] = { 0UL, 1024UL, 2048UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RPAREN 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_44(_tokenSet_44_data_,8);
const unsigned long JavaRecognizer::_tokenSet_45_data_[] = { 0UL, 4244620672UL, 1073710287UL, 3858759680UL, 2047UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "final" "abstract" SEMI "void" "boolean" "byte" "char" "short" "int" 
// "float" "long" "double" IDENT "private" "public" "protected" "static" 
// "transient" "native" "threadsafe" "synchronized" "volatile" "class" 
// LCURLY RCURLY LPAREN "if" "else" "for" "while" "do" "break" "continue" 
// "return" "switch" "throw" "case" "default" "try" "finally" "catch" PLUS 
// MINUS INC DEC BNOT LNOT "this" "super" "true" "false" "null" "new" NUM_INT 
// CHAR_LITERAL STRING_LITERAL NUM_FLOAT 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_45(_tokenSet_45_data_,12);
const unsigned long JavaRecognizer::_tokenSet_46_data_[] = { 0UL, 9216UL, 3221248384UL, 511UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RBRACK RCURLY COMMA RPAREN ASSIGN COLON PLUS_ASSIGN MINUS_ASSIGN 
// STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN 
// BXOR_ASSIGN BOR_ASSIGN 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_46(_tokenSet_46_data_,8);
const unsigned long JavaRecognizer::_tokenSet_47_data_[] = { 0UL, 9216UL, 3221248384UL, 1023UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RBRACK RCURLY COMMA RPAREN ASSIGN COLON PLUS_ASSIGN MINUS_ASSIGN 
// STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN 
// BXOR_ASSIGN BOR_ASSIGN QUESTION 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_47(_tokenSet_47_data_,8);
const unsigned long JavaRecognizer::_tokenSet_48_data_[] = { 0UL, 9216UL, 3221248384UL, 2047UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RBRACK RCURLY COMMA RPAREN ASSIGN COLON PLUS_ASSIGN MINUS_ASSIGN 
// STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN 
// BXOR_ASSIGN BOR_ASSIGN QUESTION LOR 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_48(_tokenSet_48_data_,8);
const unsigned long JavaRecognizer::_tokenSet_49_data_[] = { 0UL, 9216UL, 3221248384UL, 4095UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RBRACK RCURLY COMMA RPAREN ASSIGN COLON PLUS_ASSIGN MINUS_ASSIGN 
// STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN 
// BXOR_ASSIGN BOR_ASSIGN QUESTION LOR LAND 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_49(_tokenSet_49_data_,8);
const unsigned long JavaRecognizer::_tokenSet_50_data_[] = { 0UL, 9216UL, 3221248384UL, 8191UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RBRACK RCURLY COMMA RPAREN ASSIGN COLON PLUS_ASSIGN MINUS_ASSIGN 
// STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN 
// BXOR_ASSIGN BOR_ASSIGN QUESTION LOR LAND BOR 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_50(_tokenSet_50_data_,8);
const unsigned long JavaRecognizer::_tokenSet_51_data_[] = { 0UL, 9216UL, 3221248384UL, 16383UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RBRACK RCURLY COMMA RPAREN ASSIGN COLON PLUS_ASSIGN MINUS_ASSIGN 
// STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN 
// BXOR_ASSIGN BOR_ASSIGN QUESTION LOR LAND BOR BXOR 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_51(_tokenSet_51_data_,8);
const unsigned long JavaRecognizer::_tokenSet_52_data_[] = { 0UL, 9216UL, 3221248384UL, 32767UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RBRACK RCURLY COMMA RPAREN ASSIGN COLON PLUS_ASSIGN MINUS_ASSIGN 
// STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN 
// BXOR_ASSIGN BOR_ASSIGN QUESTION LOR LAND BOR BXOR BAND 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_52(_tokenSet_52_data_,8);
const unsigned long JavaRecognizer::_tokenSet_53_data_[] = { 0UL, 9216UL, 3221248384UL, 131071UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RBRACK RCURLY COMMA RPAREN ASSIGN COLON PLUS_ASSIGN MINUS_ASSIGN 
// STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN 
// BXOR_ASSIGN BOR_ASSIGN QUESTION LOR LAND BOR BXOR BAND NOT_EQUAL EQUAL 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_53(_tokenSet_53_data_,8);
const unsigned long JavaRecognizer::_tokenSet_54_data_[] = { 0UL, 9216UL, 3221248384UL, 4194303UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RBRACK RCURLY COMMA RPAREN ASSIGN COLON PLUS_ASSIGN MINUS_ASSIGN 
// STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN 
// BXOR_ASSIGN BOR_ASSIGN QUESTION LOR LAND BOR BXOR BAND NOT_EQUAL EQUAL 
// LT_ GT LE GE "instanceof" 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_54(_tokenSet_54_data_,8);
const unsigned long JavaRecognizer::_tokenSet_55_data_[] = { 0UL, 9216UL, 3221248384UL, 33554431UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RBRACK RCURLY COMMA RPAREN ASSIGN COLON PLUS_ASSIGN MINUS_ASSIGN 
// STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN 
// BXOR_ASSIGN BOR_ASSIGN QUESTION LOR LAND BOR BXOR BAND NOT_EQUAL EQUAL 
// LT_ GT LE GE "instanceof" SL SR BSR 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_55(_tokenSet_55_data_,8);
const unsigned long JavaRecognizer::_tokenSet_56_data_[] = { 0UL, 33554432UL, 0UL, 402653184UL, 0UL, 0UL, 0UL, 0UL };
// STAR DIV MOD 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_56(_tokenSet_56_data_,8);
const unsigned long JavaRecognizer::_tokenSet_57_data_[] = { 0UL, 9216UL, 3221248384UL, 134217727UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RBRACK RCURLY COMMA RPAREN ASSIGN COLON PLUS_ASSIGN MINUS_ASSIGN 
// STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN 
// BXOR_ASSIGN BOR_ASSIGN QUESTION LOR LAND BOR BXOR BAND NOT_EQUAL EQUAL 
// LT_ GT LE GE "instanceof" SL SR BSR PLUS MINUS 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_57(_tokenSet_57_data_,8);
const unsigned long JavaRecognizer::_tokenSet_58_data_[] = { 0UL, 33563648UL, 3221248384UL, 536870911UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RBRACK STAR RCURLY COMMA RPAREN ASSIGN COLON PLUS_ASSIGN MINUS_ASSIGN 
// STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN 
// BXOR_ASSIGN BOR_ASSIGN QUESTION LOR LAND BOR BXOR BAND NOT_EQUAL EQUAL 
// LT_ GT LE GE "instanceof" SL SR BSR PLUS MINUS DIV MOD 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_58(_tokenSet_58_data_,8);
const unsigned long JavaRecognizer::_tokenSet_59_data_[] = { 0UL, 16760832UL, 1024UL, 0UL, 2046UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "void" "boolean" "byte" "char" "short" "int" "float" "long" "double" 
// IDENT LPAREN "this" "super" "true" "false" "null" "new" NUM_INT CHAR_LITERAL 
// STRING_LITERAL NUM_FLOAT 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_59(_tokenSet_59_data_,12);
const unsigned long JavaRecognizer::_tokenSet_60_data_[] = { 0UL, 67105792UL, 3221249408UL, 4294967295UL, 2047UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LBRACK RBRACK "void" "boolean" "byte" "char" "short" "int" "float" 
// "long" "double" IDENT DOT STAR RCURLY COMMA LPAREN RPAREN ASSIGN COLON 
// PLUS_ASSIGN MINUS_ASSIGN STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN 
// BSR_ASSIGN SL_ASSIGN BAND_ASSIGN BXOR_ASSIGN BOR_ASSIGN QUESTION LOR 
// LAND BOR BXOR BAND NOT_EQUAL EQUAL LT_ GT LE GE "instanceof" SL SR BSR 
// PLUS MINUS DIV MOD INC DEC BNOT LNOT "this" "super" "true" "false" "null" 
// "new" NUM_INT CHAR_LITERAL STRING_LITERAL NUM_FLOAT 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_60(_tokenSet_60_data_,12);
const unsigned long JavaRecognizer::_tokenSet_61_data_[] = { 0UL, 16769024UL, 1024UL, 3858759680UL, 2047UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// RBRACK "void" "boolean" "byte" "char" "short" "int" "float" "long" "double" 
// IDENT LPAREN PLUS MINUS INC DEC BNOT LNOT "this" "super" "true" "false" 
// "null" "new" NUM_INT CHAR_LITERAL STRING_LITERAL NUM_FLOAT 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_61(_tokenSet_61_data_,12);
const unsigned long JavaRecognizer::_tokenSet_62_data_[] = { 0UL, 50344960UL, 3221249472UL, 2147483647UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LBRACK RBRACK DOT STAR LCURLY RCURLY COMMA LPAREN RPAREN ASSIGN 
// COLON PLUS_ASSIGN MINUS_ASSIGN STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN 
// BSR_ASSIGN SL_ASSIGN BAND_ASSIGN BXOR_ASSIGN BOR_ASSIGN QUESTION LOR 
// LAND BOR BXOR BAND NOT_EQUAL EQUAL LT_ GT LE GE "instanceof" SL SR BSR 
// PLUS MINUS DIV MOD INC DEC 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaRecognizer::_tokenSet_62(_tokenSet_62_data_,8);


