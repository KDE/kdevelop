/* $ANTLR 2.7.1: "java.store.g" -> "JavaStoreWalker.cpp"$ */
#include "JavaStoreWalker.hpp"
#include "antlr/Token.hpp"
#include "antlr/AST.hpp"
#include "antlr/NoViableAltException.hpp"
#include "antlr/MismatchedTokenException.hpp"
#include "antlr/SemanticException.hpp"
#include "antlr/BitSet.hpp"
#line 1 "java.store.g"

#line 12 "JavaStoreWalker.cpp"
JavaStoreWalker::JavaStoreWalker()
	: ANTLR_USE_NAMESPACE(antlr)TreeParser() {
	setTokenNames(_tokenNames);
}

void JavaStoreWalker::compilationUnit(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST compilationUnit_AST_in = _t;
#line 65 "java.store.g"
	ParsedClass* kl;
#line 22 "JavaStoreWalker.cpp"
	
	try {      // for error handling
#line 66 "java.store.g"
		init();
#line 27 "JavaStoreWalker.cpp"
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case PACKAGE_DEF:
		{
			packageDefinition(_t);
			_t = _retTree;
			break;
		}
		case 3:
		case CLASS_DEF:
		case INTERFACE_DEF:
		case IMPORT:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
		}
		}
		}
		{
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			if ((_t->getType()==IMPORT)) {
				importDefinition(_t);
				_t = _retTree;
			}
			else {
				goto _loop4;
			}
			
		}
		_loop4:;
		}
		{
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			if ((_t->getType()==CLASS_DEF||_t->getType()==INTERFACE_DEF)) {
				kl=typeDefinition(_t);
				_t = _retTree;
#line 69 "java.store.g"
				m_store->addClass( kl );
#line 75 "JavaStoreWalker.cpp"
			}
			else {
				goto _loop6;
			}
			
		}
		_loop6:;
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void JavaStoreWalker::packageDefinition(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST packageDefinition_AST_in = _t;
#line 72 "java.store.g"
	QString id;
#line 97 "JavaStoreWalker.cpp"
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t8 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp1_AST_in = _t;
		match(_t,PACKAGE_DEF);
		_t = _t->getFirstChild();
		id=identifier(_t);
		_t = _retTree;
		_t = __t8;
		_t = _t->getNextSibling();
#line 73 "java.store.g"
		m_package = id;
#line 110 "JavaStoreWalker.cpp"
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

 QString  JavaStoreWalker::importDefinition(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
#line 76 "java.store.g"
	 QString id ;
#line 123 "JavaStoreWalker.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefAST importDefinition_AST_in = _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t10 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp2_AST_in = _t;
		match(_t,IMPORT);
		_t = _t->getFirstChild();
		id=identifierStar(_t);
		_t = _retTree;
		_t = __t10;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
	return id ;
}

ParsedClass*  JavaStoreWalker::typeDefinition(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
#line 80 "java.store.g"
	ParsedClass* klass ;
#line 148 "JavaStoreWalker.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefAST typeDefinition_AST_in = _t;
#line 80 "java.store.g"
	QStringList bases; klass=0;
#line 152 "JavaStoreWalker.cpp"
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case CLASS_DEF:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t12 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp3_AST_in = _t;
			match(_t,CLASS_DEF);
			_t = _t->getFirstChild();
			modifiers(_t);
			_t = _retTree;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp4_AST_in = _t;
			match(_t,IDENT);
			_t = _t->getNextSibling();
#line 82 "java.store.g"
			
									klass = new ParsedClass; 
									klass->setName( tmp4_AST_in->getText().c_str() );
									klass->setDeclaredInFile( getFilename().c_str() );
									klass->setDefinedInFile( getFilename().c_str() );
									
#line 176 "JavaStoreWalker.cpp"
			bases=extendsClause(_t);
			_t = _retTree;
#line 89 "java.store.g"
			
									QStringList::Iterator it = bases.begin();
									while( it != bases.end() ){
										ParsedParent* parent = new ParsedParent;
										parent->setName( *it );
										klass->addParent( parent );
										++it;
									}
									
#line 189 "JavaStoreWalker.cpp"
			implementsClause(_t);
			_t = _retTree;
			objBlock(_t,klass);
			_t = _retTree;
			_t = __t12;
			_t = _t->getNextSibling();
			break;
		}
		case INTERFACE_DEF:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t13 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp5_AST_in = _t;
			match(_t,INTERFACE_DEF);
			_t = _t->getFirstChild();
			modifiers(_t);
			_t = _retTree;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp6_AST_in = _t;
			match(_t,IDENT);
			_t = _t->getNextSibling();
#line 101 "java.store.g"
			
									klass = new ParsedClass; 
									klass->setName( tmp6_AST_in->getText().c_str() );
									klass->setDeclaredInFile( getFilename().c_str() );
									klass->setDefinedInFile( getFilename().c_str() );
									
#line 216 "JavaStoreWalker.cpp"
			bases=extendsClause(_t);
			_t = _retTree;
#line 108 "java.store.g"
			
									QStringList::Iterator it = bases.begin();
									while( it != bases.end() ){
										ParsedParent* parent = new ParsedParent;
										parent->setName( *it );
										klass->addParent( parent );
										++it;
									}
									
#line 229 "JavaStoreWalker.cpp"
			interfaceBlock(_t,klass);
			_t = _retTree;
			_t = __t13;
			_t = _t->getNextSibling();
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
	return klass ;
}

 QString  JavaStoreWalker::identifier(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
#line 271 "java.store.g"
	 QString id ;
#line 254 "JavaStoreWalker.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefAST identifier_AST_in = _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp7_AST_in = _t;
			match(_t,IDENT);
			_t = _t->getNextSibling();
#line 272 "java.store.g"
			id = tmp7_AST_in->getText().c_str();
#line 268 "JavaStoreWalker.cpp"
			break;
		}
		case DOT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t74 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp8_AST_in = _t;
			match(_t,DOT);
			_t = _t->getFirstChild();
			id=identifier(_t);
			_t = _retTree;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp9_AST_in = _t;
			match(_t,IDENT);
			_t = _t->getNextSibling();
			_t = __t74;
			_t = _t->getNextSibling();
#line 273 "java.store.g"
			id += QString(".") + tmp9_AST_in->getText().c_str();
#line 286 "JavaStoreWalker.cpp"
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
	return id ;
}

 QString  JavaStoreWalker::identifierStar(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
#line 276 "java.store.g"
	 QString id ;
#line 307 "JavaStoreWalker.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefAST identifierStar_AST_in = _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp10_AST_in = _t;
			match(_t,IDENT);
			_t = _t->getNextSibling();
#line 277 "java.store.g"
			id = tmp10_AST_in->getText().c_str();
#line 321 "JavaStoreWalker.cpp"
			break;
		}
		case DOT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t76 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp11_AST_in = _t;
			match(_t,DOT);
			_t = _t->getFirstChild();
			id=identifier(_t);
			_t = _retTree;
			{
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case STAR:
			{
				ANTLR_USE_NAMESPACE(antlr)RefAST tmp12_AST_in = _t;
				match(_t,STAR);
				_t = _t->getNextSibling();
#line 278 "java.store.g"
				id += QString(".") + tmp12_AST_in->getText().c_str();
#line 343 "JavaStoreWalker.cpp"
				break;
			}
			case IDENT:
			{
				ANTLR_USE_NAMESPACE(antlr)RefAST tmp13_AST_in = _t;
				match(_t,IDENT);
				_t = _t->getNextSibling();
#line 279 "java.store.g"
				id += QString(".") + tmp13_AST_in->getText().c_str();
#line 353 "JavaStoreWalker.cpp"
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
			}
			}
			}
			_t = __t76;
			_t = _t->getNextSibling();
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
	return id ;
}

 QStringList  JavaStoreWalker::modifiers(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
#line 146 "java.store.g"
	 QStringList l ;
#line 384 "JavaStoreWalker.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefAST modifiers_AST_in = _t;
	ANTLR_USE_NAMESPACE(antlr)RefAST m = ANTLR_USE_NAMESPACE(antlr)nullAST;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t21 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp14_AST_in = _t;
		match(_t,MODIFIERS);
		_t = _t->getFirstChild();
		{
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			if ((_tokenSet_0.member(_t->getType()))) {
				m = (_t == ASTNULL) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
				modifier(_t);
				_t = _retTree;
#line 147 "java.store.g"
				l << m->getText().c_str();
#line 403 "JavaStoreWalker.cpp"
			}
			else {
				goto _loop23;
			}
			
		}
		_loop23:;
		}
		_t = __t21;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
	return l ;
}

 QStringList  JavaStoreWalker::extendsClause(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
#line 165 "java.store.g"
	 QStringList l ;
#line 427 "JavaStoreWalker.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefAST extendsClause_AST_in = _t;
#line 165 "java.store.g"
	QString id;
#line 431 "JavaStoreWalker.cpp"
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t26 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp15_AST_in = _t;
		match(_t,EXTENDS_CLAUSE);
		_t = _t->getFirstChild();
		{
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			if ((_t->getType()==IDENT||_t->getType()==DOT)) {
				id=identifier(_t);
				_t = _retTree;
#line 166 "java.store.g"
				l << id;
#line 447 "JavaStoreWalker.cpp"
			}
			else {
				goto _loop28;
			}
			
		}
		_loop28:;
		}
		_t = __t26;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
	return l ;
}

 QStringList  JavaStoreWalker::implementsClause(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
#line 169 "java.store.g"
	 QStringList l ;
#line 471 "JavaStoreWalker.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefAST implementsClause_AST_in = _t;
#line 169 "java.store.g"
	QString id;
#line 475 "JavaStoreWalker.cpp"
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t30 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp16_AST_in = _t;
		match(_t,IMPLEMENTS_CLAUSE);
		_t = _t->getFirstChild();
		{
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			if ((_t->getType()==IDENT||_t->getType()==DOT)) {
				id=identifier(_t);
				_t = _retTree;
#line 170 "java.store.g"
				l << id;
#line 491 "JavaStoreWalker.cpp"
			}
			else {
				goto _loop32;
			}
			
		}
		_loop32:;
		}
		_t = __t30;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
	return l ;
}

void JavaStoreWalker::objBlock(ANTLR_USE_NAMESPACE(antlr)RefAST _t,
	 ParsedClass* klass 
) {
	ANTLR_USE_NAMESPACE(antlr)RefAST objBlock_AST_in = _t;
#line 182 "java.store.g"
	ParsedClass* kl; ParsedMethod* meth; ParsedAttribute* attr;
#line 518 "JavaStoreWalker.cpp"
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t38 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp17_AST_in = _t;
		match(_t,OBJBLOCK);
		_t = _t->getFirstChild();
		{
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case CTOR_DEF:
			{
				meth=ctorDef(_t);
				_t = _retTree;
#line 184 "java.store.g"
				klass->addMethod( meth );
#line 536 "JavaStoreWalker.cpp"
				break;
			}
			case METHOD_DEF:
			{
				meth=methodDef(_t);
				_t = _retTree;
#line 185 "java.store.g"
				klass->addMethod( meth );
#line 545 "JavaStoreWalker.cpp"
				break;
			}
			case VARIABLE_DEF:
			{
				attr=variableDef(_t);
				_t = _retTree;
#line 186 "java.store.g"
				klass->addAttribute( attr );
#line 554 "JavaStoreWalker.cpp"
				break;
			}
			case CLASS_DEF:
			case INTERFACE_DEF:
			{
				kl=typeDefinition(_t);
				_t = _retTree;
#line 187 "java.store.g"
				klass->addClass( kl );
#line 564 "JavaStoreWalker.cpp"
				break;
			}
			case STATIC_INIT:
			{
				ANTLR_USE_NAMESPACE(antlr)RefAST __t40 = _t;
				ANTLR_USE_NAMESPACE(antlr)RefAST tmp18_AST_in = _t;
				match(_t,STATIC_INIT);
				_t = _t->getFirstChild();
				slist(_t);
				_t = _retTree;
				_t = __t40;
				_t = _t->getNextSibling();
				break;
			}
			case INSTANCE_INIT:
			{
				ANTLR_USE_NAMESPACE(antlr)RefAST __t41 = _t;
				ANTLR_USE_NAMESPACE(antlr)RefAST tmp19_AST_in = _t;
				match(_t,INSTANCE_INIT);
				_t = _t->getFirstChild();
				slist(_t);
				_t = _retTree;
				_t = __t41;
				_t = _t->getNextSibling();
				break;
			}
			default:
			{
				goto _loop42;
			}
			}
		}
		_loop42:;
		}
		_t = __t38;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void JavaStoreWalker::interfaceBlock(ANTLR_USE_NAMESPACE(antlr)RefAST _t,
	 ParsedClass* klass 
) {
	ANTLR_USE_NAMESPACE(antlr)RefAST interfaceBlock_AST_in = _t;
#line 174 "java.store.g"
	ParsedMethod* meth; ParsedAttribute* attr;
#line 616 "JavaStoreWalker.cpp"
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t34 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp20_AST_in = _t;
		match(_t,OBJBLOCK);
		_t = _t->getFirstChild();
		{
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case METHOD_DEF:
			{
				meth=methodDecl(_t);
				_t = _retTree;
#line 176 "java.store.g"
				klass->addMethod( meth );
#line 634 "JavaStoreWalker.cpp"
				break;
			}
			case VARIABLE_DEF:
			{
				attr=variableDef(_t);
				_t = _retTree;
#line 177 "java.store.g"
				klass->addAttribute( attr );
#line 643 "JavaStoreWalker.cpp"
				break;
			}
			default:
			{
				goto _loop36;
			}
			}
		}
		_loop36:;
		}
		_t = __t34;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void JavaStoreWalker::typeSpec(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST typeSpec_AST_in = _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t15 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp21_AST_in = _t;
		match(_t,TYPE);
		_t = _t->getFirstChild();
		typeSpecArray(_t);
		_t = _retTree;
		_t = __t15;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void JavaStoreWalker::typeSpecArray(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST typeSpecArray_AST_in = _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case ARRAY_DECLARATOR:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t17 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp22_AST_in = _t;
			match(_t,ARRAY_DECLARATOR);
			_t = _t->getFirstChild();
			typeSpecArray(_t);
			_t = _retTree;
			_t = __t17;
			_t = _t->getNextSibling();
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
		case DOT:
		{
			type(_t);
			_t = _retTree;
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

 QString  JavaStoreWalker::type(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
#line 129 "java.store.g"
	 QString tp ;
#line 738 "JavaStoreWalker.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefAST type_AST_in = _t;
	ANTLR_USE_NAMESPACE(antlr)RefAST b = ANTLR_USE_NAMESPACE(antlr)nullAST;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENT:
		case DOT:
		{
			tp=identifier(_t);
			_t = _retTree;
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
			b = (_t == ASTNULL) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
			builtInType(_t);
			_t = _retTree;
#line 131 "java.store.g"
			tp = b->getText().c_str();
#line 768 "JavaStoreWalker.cpp"
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
	return tp ;
}

void JavaStoreWalker::builtInType(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST builtInType_AST_in = _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case LITERAL_void:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp23_AST_in = _t;
			match(_t,LITERAL_void);
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_boolean:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp24_AST_in = _t;
			match(_t,LITERAL_boolean);
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_byte:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp25_AST_in = _t;
			match(_t,LITERAL_byte);
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_char:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp26_AST_in = _t;
			match(_t,LITERAL_char);
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_short:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp27_AST_in = _t;
			match(_t,LITERAL_short);
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_int:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp28_AST_in = _t;
			match(_t,LITERAL_int);
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_float:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp29_AST_in = _t;
			match(_t,LITERAL_float);
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_long:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp30_AST_in = _t;
			match(_t,LITERAL_long);
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_double:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp31_AST_in = _t;
			match(_t,LITERAL_double);
			_t = _t->getNextSibling();
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void JavaStoreWalker::modifier(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST modifier_AST_in = _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case LITERAL_private:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp32_AST_in = _t;
			match(_t,LITERAL_private);
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_public:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp33_AST_in = _t;
			match(_t,LITERAL_public);
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_protected:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp34_AST_in = _t;
			match(_t,LITERAL_protected);
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_static:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp35_AST_in = _t;
			match(_t,LITERAL_static);
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_transient:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp36_AST_in = _t;
			match(_t,LITERAL_transient);
			_t = _t->getNextSibling();
			break;
		}
		case FINAL:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp37_AST_in = _t;
			match(_t,FINAL);
			_t = _t->getNextSibling();
			break;
		}
		case ABSTRACT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp38_AST_in = _t;
			match(_t,ABSTRACT);
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_native:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp39_AST_in = _t;
			match(_t,LITERAL_native);
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_threadsafe:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp40_AST_in = _t;
			match(_t,LITERAL_threadsafe);
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_synchronized:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp41_AST_in = _t;
			match(_t,LITERAL_synchronized);
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_const:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp42_AST_in = _t;
			match(_t,LITERAL_const);
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_volatile:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp43_AST_in = _t;
			match(_t,LITERAL_volatile);
			_t = _t->getNextSibling();
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

 ParsedMethod*  JavaStoreWalker::methodDecl(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
#line 206 "java.store.g"
	 ParsedMethod* meth ;
#line 978 "JavaStoreWalker.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefAST methodDecl_AST_in = _t;
#line 206 "java.store.g"
	
								meth = new ParsedMethod;
								meth->setDeclaredInFile( getFilename().c_str() );
								meth->setDefinedInFile( getFilename().c_str() );
								
#line 986 "JavaStoreWalker.cpp"
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t46 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp44_AST_in = _t;
		match(_t,METHOD_DEF);
		_t = _t->getFirstChild();
		modifiers(_t);
		_t = _retTree;
		typeSpec(_t);
		_t = _retTree;
		methodHead(_t,meth);
		_t = _retTree;
		_t = __t46;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
	return meth ;
}

 ParsedAttribute*  JavaStoreWalker::variableDef(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
#line 222 "java.store.g"
	 ParsedAttribute* attr ;
#line 1014 "JavaStoreWalker.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefAST variableDef_AST_in = _t;
#line 222 "java.store.g"
	
								attr = new ParsedAttribute;
								attr->setDeclaredInFile( getFilename().c_str() );
								attr->setDefinedInFile( getFilename().c_str() );
								
#line 1022 "JavaStoreWalker.cpp"
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t51 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp45_AST_in = _t;
		match(_t,VARIABLE_DEF);
		_t = _t->getFirstChild();
		modifiers(_t);
		_t = _retTree;
		typeSpec(_t);
		_t = _retTree;
		variableDeclarator(_t,attr);
		_t = _retTree;
		varInitializer(_t);
		_t = _retTree;
		_t = __t51;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
	return attr ;
}

 ParsedMethod*  JavaStoreWalker::ctorDef(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
#line 194 "java.store.g"
	 ParsedMethod* meth ;
#line 1052 "JavaStoreWalker.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefAST ctorDef_AST_in = _t;
#line 194 "java.store.g"
	
								meth = new ParsedMethod;
								meth->setIsConstructor( TRUE );
								meth->setDeclaredInFile( getFilename().c_str() );
								meth->setDefinedInFile( getFilename().c_str() );
								
#line 1061 "JavaStoreWalker.cpp"
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t44 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp46_AST_in = _t;
		match(_t,CTOR_DEF);
		_t = _t->getFirstChild();
		modifiers(_t);
		_t = _retTree;
		methodHead(_t,meth);
		_t = _retTree;
		slist(_t);
		_t = _retTree;
		_t = __t44;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
	return meth ;
}

 ParsedMethod*  JavaStoreWalker::methodDef(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
#line 214 "java.store.g"
	 ParsedMethod* meth ;
#line 1089 "JavaStoreWalker.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefAST methodDef_AST_in = _t;
#line 214 "java.store.g"
	
								meth = new ParsedMethod; 
								meth->setDeclaredInFile( getFilename().c_str() );	
								meth->setDefinedInFile( getFilename().c_str() );
								
#line 1097 "JavaStoreWalker.cpp"
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t48 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp47_AST_in = _t;
		match(_t,METHOD_DEF);
		_t = _t->getFirstChild();
		modifiers(_t);
		_t = _retTree;
		typeSpec(_t);
		_t = _retTree;
		methodHead(_t,meth);
		_t = _retTree;
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case SLIST:
		{
			slist(_t);
			_t = _retTree;
			break;
		}
		case 3:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
		}
		}
		}
		_t = __t48;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
	return meth ;
}

void JavaStoreWalker::slist(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST slist_AST_in = _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t79 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp48_AST_in = _t;
		match(_t,SLIST);
		_t = _t->getFirstChild();
		{
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			if ((_tokenSet_1.member(_t->getType()))) {
				stat(_t);
				_t = _retTree;
			}
			else {
				goto _loop81;
			}
			
		}
		_loop81:;
		}
		_t = __t79;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void JavaStoreWalker::methodHead(ANTLR_USE_NAMESPACE(antlr)RefAST _t,
	 ParsedMethod* meth 
) {
	ANTLR_USE_NAMESPACE(antlr)RefAST methodHead_AST_in = _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp49_AST_in = _t;
		match(_t,IDENT);
		_t = _t->getNextSibling();
		ANTLR_USE_NAMESPACE(antlr)RefAST __t65 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp50_AST_in = _t;
		match(_t,PARAMETERS);
		_t = _t->getFirstChild();
		{
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			if ((_t->getType()==PARAMETER_DEF)) {
				parameterDef(_t);
				_t = _retTree;
			}
			else {
				goto _loop67;
			}
			
		}
		_loop67:;
		}
		_t = __t65;
		_t = _t->getNextSibling();
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case LITERAL_throws:
		{
			throwsClause(_t);
			_t = _retTree;
			break;
		}
		case 3:
		case SLIST:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
		}
		}
		}
#line 262 "java.store.g"
		
				meth->setName( tmp49_AST_in->getText().c_str() );
			
#line 1231 "JavaStoreWalker.cpp"
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void JavaStoreWalker::variableDeclarator(ANTLR_USE_NAMESPACE(antlr)RefAST _t,
	 ParsedAttribute* attr 
) {
	ANTLR_USE_NAMESPACE(antlr)RefAST variableDeclarator_AST_in = _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp51_AST_in = _t;
			match(_t,IDENT);
			_t = _t->getNextSibling();
#line 240 "java.store.g"
			
										attr->setName( tmp51_AST_in->getText().c_str() );
										
#line 1259 "JavaStoreWalker.cpp"
			break;
		}
		case LBRACK:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp52_AST_in = _t;
			match(_t,LBRACK);
			_t = _t->getNextSibling();
			variableDeclarator(_t,attr);
			_t = _retTree;
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void JavaStoreWalker::varInitializer(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST varInitializer_AST_in = _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case ASSIGN:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t58 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp53_AST_in = _t;
			match(_t,ASSIGN);
			_t = _t->getFirstChild();
			initializer(_t);
			_t = _retTree;
			_t = __t58;
			_t = _t->getNextSibling();
			break;
		}
		case 3:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void JavaStoreWalker::parameterDef(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST parameterDef_AST_in = _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t53 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp54_AST_in = _t;
		match(_t,PARAMETER_DEF);
		_t = _t->getFirstChild();
		modifiers(_t);
		_t = _retTree;
		typeSpec(_t);
		_t = _retTree;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp55_AST_in = _t;
		match(_t,IDENT);
		_t = _t->getNextSibling();
		_t = __t53;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void JavaStoreWalker::objectinitializer(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST objectinitializer_AST_in = _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t55 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp56_AST_in = _t;
		match(_t,INSTANCE_INIT);
		_t = _t->getFirstChild();
		slist(_t);
		_t = _retTree;
		_t = __t55;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void JavaStoreWalker::initializer(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST initializer_AST_in = _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case EXPR:
		{
			expression(_t);
			_t = _retTree;
			break;
		}
		case ARRAY_INIT:
		{
			arrayInitializer(_t);
			_t = _retTree;
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void JavaStoreWalker::expression(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST expression_AST_in = _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t124 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp57_AST_in = _t;
		match(_t,EXPR);
		_t = _t->getFirstChild();
		expr(_t);
		_t = _retTree;
		_t = __t124;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void JavaStoreWalker::arrayInitializer(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST arrayInitializer_AST_in = _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t61 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp58_AST_in = _t;
		match(_t,ARRAY_INIT);
		_t = _t->getFirstChild();
		{
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			if ((_t->getType()==EXPR||_t->getType()==ARRAY_INIT)) {
				initializer(_t);
				_t = _retTree;
			}
			else {
				goto _loop63;
			}
			
		}
		_loop63:;
		}
		_t = __t61;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void JavaStoreWalker::throwsClause(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST throwsClause_AST_in = _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t70 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp59_AST_in = _t;
		match(_t,LITERAL_throws);
		_t = _t->getFirstChild();
		{
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			if ((_t->getType()==IDENT||_t->getType()==DOT)) {
				identifier(_t);
				_t = _retTree;
			}
			else {
				goto _loop72;
			}
			
		}
		_loop72:;
		}
		_t = __t70;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void JavaStoreWalker::stat(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST stat_AST_in = _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case CLASS_DEF:
		case INTERFACE_DEF:
		{
			typeDefinition(_t);
			_t = _retTree;
			break;
		}
		case VARIABLE_DEF:
		{
			variableDef(_t);
			_t = _retTree;
			break;
		}
		case EXPR:
		{
			expression(_t);
			_t = _retTree;
			break;
		}
		case LABELED_STAT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t83 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp60_AST_in = _t;
			match(_t,LABELED_STAT);
			_t = _t->getFirstChild();
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp61_AST_in = _t;
			match(_t,IDENT);
			_t = _t->getNextSibling();
			stat(_t);
			_t = _retTree;
			_t = __t83;
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_if:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t84 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp62_AST_in = _t;
			match(_t,LITERAL_if);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			stat(_t);
			_t = _retTree;
			{
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case SLIST:
			case VARIABLE_DEF:
			case CLASS_DEF:
			case INTERFACE_DEF:
			case LABELED_STAT:
			case EXPR:
			case EMPTY_STAT:
			case LITERAL_synchronized:
			case LITERAL_if:
			case LITERAL_for:
			case LITERAL_while:
			case LITERAL_do:
			case LITERAL_break:
			case LITERAL_continue:
			case LITERAL_return:
			case LITERAL_switch:
			case LITERAL_throw:
			case LITERAL_try:
			{
				stat(_t);
				_t = _retTree;
				break;
			}
			case 3:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
			}
			}
			}
			_t = __t84;
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_for:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t86 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp63_AST_in = _t;
			match(_t,LITERAL_for);
			_t = _t->getFirstChild();
			ANTLR_USE_NAMESPACE(antlr)RefAST __t87 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp64_AST_in = _t;
			match(_t,FOR_INIT);
			_t = _t->getFirstChild();
			{
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case VARIABLE_DEF:
			{
				variableDef(_t);
				_t = _retTree;
				break;
			}
			case ELIST:
			{
				elist(_t);
				_t = _retTree;
				break;
			}
			case 3:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
			}
			}
			}
			_t = __t87;
			_t = _t->getNextSibling();
			ANTLR_USE_NAMESPACE(antlr)RefAST __t89 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp65_AST_in = _t;
			match(_t,FOR_CONDITION);
			_t = _t->getFirstChild();
			{
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case EXPR:
			{
				expression(_t);
				_t = _retTree;
				break;
			}
			case 3:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
			}
			}
			}
			_t = __t89;
			_t = _t->getNextSibling();
			ANTLR_USE_NAMESPACE(antlr)RefAST __t91 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp66_AST_in = _t;
			match(_t,FOR_ITERATOR);
			_t = _t->getFirstChild();
			{
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case ELIST:
			{
				elist(_t);
				_t = _retTree;
				break;
			}
			case 3:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
			}
			}
			}
			_t = __t91;
			_t = _t->getNextSibling();
			stat(_t);
			_t = _retTree;
			_t = __t86;
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_while:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t93 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp67_AST_in = _t;
			match(_t,LITERAL_while);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			stat(_t);
			_t = _retTree;
			_t = __t93;
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_do:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t94 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp68_AST_in = _t;
			match(_t,LITERAL_do);
			_t = _t->getFirstChild();
			stat(_t);
			_t = _retTree;
			expression(_t);
			_t = _retTree;
			_t = __t94;
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_break:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t95 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp69_AST_in = _t;
			match(_t,LITERAL_break);
			_t = _t->getFirstChild();
			{
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case IDENT:
			{
				ANTLR_USE_NAMESPACE(antlr)RefAST tmp70_AST_in = _t;
				match(_t,IDENT);
				_t = _t->getNextSibling();
				break;
			}
			case 3:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
			}
			}
			}
			_t = __t95;
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_continue:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t97 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp71_AST_in = _t;
			match(_t,LITERAL_continue);
			_t = _t->getFirstChild();
			{
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case IDENT:
			{
				ANTLR_USE_NAMESPACE(antlr)RefAST tmp72_AST_in = _t;
				match(_t,IDENT);
				_t = _t->getNextSibling();
				break;
			}
			case 3:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
			}
			}
			}
			_t = __t97;
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_return:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t99 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp73_AST_in = _t;
			match(_t,LITERAL_return);
			_t = _t->getFirstChild();
			{
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case EXPR:
			{
				expression(_t);
				_t = _retTree;
				break;
			}
			case 3:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
			}
			}
			}
			_t = __t99;
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_switch:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t101 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp74_AST_in = _t;
			match(_t,LITERAL_switch);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			{
			for (;;) {
				if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
					_t = ASTNULL;
				if ((_t->getType()==CASE_GROUP)) {
					caseGroup(_t);
					_t = _retTree;
				}
				else {
					goto _loop103;
				}
				
			}
			_loop103:;
			}
			_t = __t101;
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_throw:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t104 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp75_AST_in = _t;
			match(_t,LITERAL_throw);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			_t = __t104;
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_synchronized:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t105 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp76_AST_in = _t;
			match(_t,LITERAL_synchronized);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			stat(_t);
			_t = _retTree;
			_t = __t105;
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_try:
		{
			tryBlock(_t);
			_t = _retTree;
			break;
		}
		case SLIST:
		{
			slist(_t);
			_t = _retTree;
			break;
		}
		case EMPTY_STAT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp77_AST_in = _t;
			match(_t,EMPTY_STAT);
			_t = _t->getNextSibling();
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void JavaStoreWalker::elist(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST elist_AST_in = _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t120 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp78_AST_in = _t;
		match(_t,ELIST);
		_t = _t->getFirstChild();
		{
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			if ((_t->getType()==EXPR)) {
				expression(_t);
				_t = _retTree;
			}
			else {
				goto _loop122;
			}
			
		}
		_loop122:;
		}
		_t = __t120;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void JavaStoreWalker::caseGroup(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST caseGroup_AST_in = _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t107 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp79_AST_in = _t;
		match(_t,CASE_GROUP);
		_t = _t->getFirstChild();
		{
		int _cnt110=0;
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case LITERAL_case:
			{
				ANTLR_USE_NAMESPACE(antlr)RefAST __t109 = _t;
				ANTLR_USE_NAMESPACE(antlr)RefAST tmp80_AST_in = _t;
				match(_t,LITERAL_case);
				_t = _t->getFirstChild();
				expression(_t);
				_t = _retTree;
				_t = __t109;
				_t = _t->getNextSibling();
				break;
			}
			case LITERAL_default:
			{
				ANTLR_USE_NAMESPACE(antlr)RefAST tmp81_AST_in = _t;
				match(_t,LITERAL_default);
				_t = _t->getNextSibling();
				break;
			}
			default:
			{
				if ( _cnt110>=1 ) { goto _loop110; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);}
			}
			}
			_cnt110++;
		}
		_loop110:;
		}
		slist(_t);
		_t = _retTree;
		_t = __t107;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void JavaStoreWalker::tryBlock(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST tryBlock_AST_in = _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t112 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp82_AST_in = _t;
		match(_t,LITERAL_try);
		_t = _t->getFirstChild();
		slist(_t);
		_t = _retTree;
		{
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			if ((_t->getType()==LITERAL_catch)) {
				handler(_t);
				_t = _retTree;
			}
			else {
				goto _loop114;
			}
			
		}
		_loop114:;
		}
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case LITERAL_finally:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t116 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp83_AST_in = _t;
			match(_t,LITERAL_finally);
			_t = _t->getFirstChild();
			slist(_t);
			_t = _retTree;
			_t = __t116;
			_t = _t->getNextSibling();
			break;
		}
		case 3:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
		}
		}
		}
		_t = __t112;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void JavaStoreWalker::handler(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST handler_AST_in = _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t118 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp84_AST_in = _t;
		match(_t,LITERAL_catch);
		_t = _t->getFirstChild();
		parameterDef(_t);
		_t = _retTree;
		slist(_t);
		_t = _retTree;
		_t = __t118;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void JavaStoreWalker::expr(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST expr_AST_in = _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case QUESTION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t126 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp85_AST_in = _t;
			match(_t,QUESTION);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t126;
			_t = _t->getNextSibling();
			break;
		}
		case ASSIGN:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t127 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp86_AST_in = _t;
			match(_t,ASSIGN);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t127;
			_t = _t->getNextSibling();
			break;
		}
		case PLUS_ASSIGN:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t128 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp87_AST_in = _t;
			match(_t,PLUS_ASSIGN);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t128;
			_t = _t->getNextSibling();
			break;
		}
		case MINUS_ASSIGN:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t129 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp88_AST_in = _t;
			match(_t,MINUS_ASSIGN);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t129;
			_t = _t->getNextSibling();
			break;
		}
		case STAR_ASSIGN:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t130 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp89_AST_in = _t;
			match(_t,STAR_ASSIGN);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t130;
			_t = _t->getNextSibling();
			break;
		}
		case DIV_ASSIGN:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t131 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp90_AST_in = _t;
			match(_t,DIV_ASSIGN);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t131;
			_t = _t->getNextSibling();
			break;
		}
		case MOD_ASSIGN:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t132 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp91_AST_in = _t;
			match(_t,MOD_ASSIGN);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t132;
			_t = _t->getNextSibling();
			break;
		}
		case SR_ASSIGN:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t133 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp92_AST_in = _t;
			match(_t,SR_ASSIGN);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t133;
			_t = _t->getNextSibling();
			break;
		}
		case BSR_ASSIGN:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t134 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp93_AST_in = _t;
			match(_t,BSR_ASSIGN);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t134;
			_t = _t->getNextSibling();
			break;
		}
		case SL_ASSIGN:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t135 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp94_AST_in = _t;
			match(_t,SL_ASSIGN);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t135;
			_t = _t->getNextSibling();
			break;
		}
		case BAND_ASSIGN:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t136 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp95_AST_in = _t;
			match(_t,BAND_ASSIGN);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t136;
			_t = _t->getNextSibling();
			break;
		}
		case BXOR_ASSIGN:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t137 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp96_AST_in = _t;
			match(_t,BXOR_ASSIGN);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t137;
			_t = _t->getNextSibling();
			break;
		}
		case BOR_ASSIGN:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t138 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp97_AST_in = _t;
			match(_t,BOR_ASSIGN);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t138;
			_t = _t->getNextSibling();
			break;
		}
		case LOR:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t139 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp98_AST_in = _t;
			match(_t,LOR);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t139;
			_t = _t->getNextSibling();
			break;
		}
		case LAND:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t140 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp99_AST_in = _t;
			match(_t,LAND);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t140;
			_t = _t->getNextSibling();
			break;
		}
		case BOR:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t141 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp100_AST_in = _t;
			match(_t,BOR);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t141;
			_t = _t->getNextSibling();
			break;
		}
		case BXOR:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t142 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp101_AST_in = _t;
			match(_t,BXOR);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t142;
			_t = _t->getNextSibling();
			break;
		}
		case BAND:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t143 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp102_AST_in = _t;
			match(_t,BAND);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t143;
			_t = _t->getNextSibling();
			break;
		}
		case NOT_EQUAL:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t144 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp103_AST_in = _t;
			match(_t,NOT_EQUAL);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t144;
			_t = _t->getNextSibling();
			break;
		}
		case EQUAL:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t145 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp104_AST_in = _t;
			match(_t,EQUAL);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t145;
			_t = _t->getNextSibling();
			break;
		}
		case LT_:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t146 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp105_AST_in = _t;
			match(_t,LT_);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t146;
			_t = _t->getNextSibling();
			break;
		}
		case GT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t147 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp106_AST_in = _t;
			match(_t,GT);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t147;
			_t = _t->getNextSibling();
			break;
		}
		case LE:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t148 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp107_AST_in = _t;
			match(_t,LE);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t148;
			_t = _t->getNextSibling();
			break;
		}
		case GE:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t149 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp108_AST_in = _t;
			match(_t,GE);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t149;
			_t = _t->getNextSibling();
			break;
		}
		case SL:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t150 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp109_AST_in = _t;
			match(_t,SL);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t150;
			_t = _t->getNextSibling();
			break;
		}
		case SR:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t151 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp110_AST_in = _t;
			match(_t,SR);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t151;
			_t = _t->getNextSibling();
			break;
		}
		case BSR:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t152 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp111_AST_in = _t;
			match(_t,BSR);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t152;
			_t = _t->getNextSibling();
			break;
		}
		case PLUS:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t153 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp112_AST_in = _t;
			match(_t,PLUS);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t153;
			_t = _t->getNextSibling();
			break;
		}
		case MINUS:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t154 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp113_AST_in = _t;
			match(_t,MINUS);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t154;
			_t = _t->getNextSibling();
			break;
		}
		case DIV:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t155 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp114_AST_in = _t;
			match(_t,DIV);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t155;
			_t = _t->getNextSibling();
			break;
		}
		case MOD:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t156 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp115_AST_in = _t;
			match(_t,MOD);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t156;
			_t = _t->getNextSibling();
			break;
		}
		case STAR:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t157 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp116_AST_in = _t;
			match(_t,STAR);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t157;
			_t = _t->getNextSibling();
			break;
		}
		case INC:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t158 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp117_AST_in = _t;
			match(_t,INC);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			_t = __t158;
			_t = _t->getNextSibling();
			break;
		}
		case DEC:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t159 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp118_AST_in = _t;
			match(_t,DEC);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			_t = __t159;
			_t = _t->getNextSibling();
			break;
		}
		case POST_INC:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t160 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp119_AST_in = _t;
			match(_t,POST_INC);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			_t = __t160;
			_t = _t->getNextSibling();
			break;
		}
		case POST_DEC:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t161 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp120_AST_in = _t;
			match(_t,POST_DEC);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			_t = __t161;
			_t = _t->getNextSibling();
			break;
		}
		case BNOT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t162 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp121_AST_in = _t;
			match(_t,BNOT);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			_t = __t162;
			_t = _t->getNextSibling();
			break;
		}
		case LNOT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t163 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp122_AST_in = _t;
			match(_t,LNOT);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			_t = __t163;
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_instanceof:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t164 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp123_AST_in = _t;
			match(_t,LITERAL_instanceof);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t164;
			_t = _t->getNextSibling();
			break;
		}
		case UNARY_MINUS:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t165 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp124_AST_in = _t;
			match(_t,UNARY_MINUS);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			_t = __t165;
			_t = _t->getNextSibling();
			break;
		}
		case UNARY_PLUS:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t166 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp125_AST_in = _t;
			match(_t,UNARY_PLUS);
			_t = _t->getFirstChild();
			expr(_t);
			_t = _retTree;
			_t = __t166;
			_t = _t->getNextSibling();
			break;
		}
		case TYPE:
		case TYPECAST:
		case INDEX_OP:
		case METHOD_CALL:
		case IDENT:
		case DOT:
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
			primaryExpression(_t);
			_t = _retTree;
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void JavaStoreWalker::primaryExpression(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST primaryExpression_AST_in = _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp126_AST_in = _t;
			match(_t,IDENT);
			_t = _t->getNextSibling();
			break;
		}
		case DOT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t168 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp127_AST_in = _t;
			match(_t,DOT);
			_t = _t->getFirstChild();
			{
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case TYPE:
			case TYPECAST:
			case INDEX_OP:
			case POST_INC:
			case POST_DEC:
			case METHOD_CALL:
			case UNARY_MINUS:
			case UNARY_PLUS:
			case IDENT:
			case DOT:
			case STAR:
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
				expr(_t);
				_t = _retTree;
				{
				if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
					_t = ASTNULL;
				switch ( _t->getType()) {
				case IDENT:
				{
					ANTLR_USE_NAMESPACE(antlr)RefAST tmp128_AST_in = _t;
					match(_t,IDENT);
					_t = _t->getNextSibling();
					break;
				}
				case INDEX_OP:
				{
					arrayIndex(_t);
					_t = _retTree;
					break;
				}
				case LITERAL_this:
				{
					ANTLR_USE_NAMESPACE(antlr)RefAST tmp129_AST_in = _t;
					match(_t,LITERAL_this);
					_t = _t->getNextSibling();
					break;
				}
				case LITERAL_class:
				{
					ANTLR_USE_NAMESPACE(antlr)RefAST tmp130_AST_in = _t;
					match(_t,LITERAL_class);
					_t = _t->getNextSibling();
					break;
				}
				case LITERAL_new:
				{
					ANTLR_USE_NAMESPACE(antlr)RefAST __t171 = _t;
					ANTLR_USE_NAMESPACE(antlr)RefAST tmp131_AST_in = _t;
					match(_t,LITERAL_new);
					_t = _t->getFirstChild();
					ANTLR_USE_NAMESPACE(antlr)RefAST tmp132_AST_in = _t;
					match(_t,IDENT);
					_t = _t->getNextSibling();
					elist(_t);
					_t = _retTree;
					_t = __t171;
					_t = _t->getNextSibling();
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
				}
				}
				}
				break;
			}
			case ARRAY_DECLARATOR:
			{
				ANTLR_USE_NAMESPACE(antlr)RefAST __t172 = _t;
				ANTLR_USE_NAMESPACE(antlr)RefAST tmp133_AST_in = _t;
				match(_t,ARRAY_DECLARATOR);
				_t = _t->getFirstChild();
				type(_t);
				_t = _retTree;
				_t = __t172;
				_t = _t->getNextSibling();
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
				builtInType(_t);
				_t = _retTree;
				{
				if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
					_t = ASTNULL;
				switch ( _t->getType()) {
				case LITERAL_class:
				{
					ANTLR_USE_NAMESPACE(antlr)RefAST tmp134_AST_in = _t;
					match(_t,LITERAL_class);
					_t = _t->getNextSibling();
					break;
				}
				case 3:
				{
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
				}
				}
				}
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
			}
			}
			}
			_t = __t168;
			_t = _t->getNextSibling();
			break;
		}
		case INDEX_OP:
		{
			arrayIndex(_t);
			_t = _retTree;
			break;
		}
		case METHOD_CALL:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t174 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp135_AST_in = _t;
			match(_t,METHOD_CALL);
			_t = _t->getFirstChild();
			primaryExpression(_t);
			_t = _retTree;
			elist(_t);
			_t = _retTree;
			_t = __t174;
			_t = _t->getNextSibling();
			break;
		}
		case TYPECAST:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t175 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp136_AST_in = _t;
			match(_t,TYPECAST);
			_t = _t->getFirstChild();
			typeSpec(_t);
			_t = _retTree;
			expr(_t);
			_t = _retTree;
			_t = __t175;
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_new:
		{
			newExpression(_t);
			_t = _retTree;
			break;
		}
		case NUM_INT:
		case CHAR_LITERAL:
		case STRING_LITERAL:
		case NUM_FLOAT:
		{
			constant(_t);
			_t = _retTree;
			break;
		}
		case LITERAL_super:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp137_AST_in = _t;
			match(_t,LITERAL_super);
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_true:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp138_AST_in = _t;
			match(_t,LITERAL_true);
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_false:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp139_AST_in = _t;
			match(_t,LITERAL_false);
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_this:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp140_AST_in = _t;
			match(_t,LITERAL_this);
			_t = _t->getNextSibling();
			break;
		}
		case LITERAL_null:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp141_AST_in = _t;
			match(_t,LITERAL_null);
			_t = _t->getNextSibling();
			break;
		}
		case TYPE:
		{
			typeSpec(_t);
			_t = _retTree;
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void JavaStoreWalker::arrayIndex(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST arrayIndex_AST_in = _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t177 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp142_AST_in = _t;
		match(_t,INDEX_OP);
		_t = _t->getFirstChild();
		primaryExpression(_t);
		_t = _retTree;
		expression(_t);
		_t = _retTree;
		_t = __t177;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void JavaStoreWalker::newExpression(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST newExpression_AST_in = _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t180 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp143_AST_in = _t;
		match(_t,LITERAL_new);
		_t = _t->getFirstChild();
		type(_t);
		_t = _retTree;
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case ARRAY_DECLARATOR:
		{
			newArrayDeclarator(_t);
			_t = _retTree;
			{
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case ARRAY_INIT:
			{
				arrayInitializer(_t);
				_t = _retTree;
				break;
			}
			case 3:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
			}
			}
			}
			break;
		}
		case ELIST:
		{
			elist(_t);
			_t = _retTree;
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
		}
		}
		}
		_t = __t180;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void JavaStoreWalker::constant(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST constant_AST_in = _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case NUM_INT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp144_AST_in = _t;
			match(_t,NUM_INT);
			_t = _t->getNextSibling();
			break;
		}
		case CHAR_LITERAL:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp145_AST_in = _t;
			match(_t,CHAR_LITERAL);
			_t = _t->getNextSibling();
			break;
		}
		case STRING_LITERAL:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp146_AST_in = _t;
			match(_t,STRING_LITERAL);
			_t = _t->getNextSibling();
			break;
		}
		case NUM_FLOAT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp147_AST_in = _t;
			match(_t,NUM_FLOAT);
			_t = _t->getNextSibling();
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void JavaStoreWalker::newArrayDeclarator(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST newArrayDeclarator_AST_in = _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t184 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp148_AST_in = _t;
		match(_t,ARRAY_DECLARATOR);
		_t = _t->getFirstChild();
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case ARRAY_DECLARATOR:
		{
			newArrayDeclarator(_t);
			_t = _retTree;
			break;
		}
		case 3:
		case EXPR:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
		}
		}
		}
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case EXPR:
		{
			expression(_t);
			_t = _retTree;
			break;
		}
		case 3:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
		}
		}
		}
		_t = __t184;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

const char* JavaStoreWalker::_tokenNames[] = {
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
	"\"const\"",
	0
};

const unsigned long JavaStoreWalker::_tokenSet_0_data_[] = { 0UL, 4227858816UL, 7UL, 0UL, 524288UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "final" "abstract" "private" "public" "protected" "static" "transient" 
// "native" "threadsafe" "synchronized" "volatile" "const" 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaStoreWalker::_tokenSet_0(_tokenSet_0_data_,12);
const unsigned long JavaStoreWalker::_tokenSet_1_data_[] = { 272680064UL, 64UL, 167673858UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SLIST VARIABLE_DEF CLASS_DEF INTERFACE_DEF LABELED_STAT EXPR EMPTY_STAT 
// "synchronized" "if" "for" "while" "do" "break" "continue" "return" "switch" 
// "throw" "try" 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaStoreWalker::_tokenSet_1(_tokenSet_1_data_,8);


