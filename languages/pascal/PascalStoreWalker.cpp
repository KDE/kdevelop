/* $ANTLR 2.7.2: "pascal.tree.g" -> "PascalStoreWalker.cpp"$ */
#include "PascalStoreWalker.hpp"
#include <antlr/Token.hpp>
#include <antlr/AST.hpp>
#include <antlr/NoViableAltException.hpp>
#include <antlr/MismatchedTokenException.hpp>
#include <antlr/SemanticException.hpp>
#include <antlr/BitSet.hpp>
#line 1 "pascal.tree.g"
#line 11 "PascalStoreWalker.cpp"
PascalStoreWalker::PascalStoreWalker()
	: ANTLR_USE_NAMESPACE(antlr)TreeParser() {
}

void PascalStoreWalker::program(RefPascalAST _t) {
	RefPascalAST program_AST_in = _t;

	try {      // for error handling
		programHeading(_t);
		_t = _retTree;
		block(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::programHeading(RefPascalAST _t) {
	RefPascalAST programHeading_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case PROGRAM:
		{
			RefPascalAST __t3 = _t;
			RefPascalAST tmp1_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PROGRAM);
			_t = _t->getFirstChild();
			RefPascalAST tmp2_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENT);
			_t = _t->getNextSibling();
			identifierList(_t);
			_t = _retTree;
			_t = __t3;
			_t = _t->getNextSibling();
			break;
		}
		case UNIT:
		{
			RefPascalAST __t4 = _t;
			RefPascalAST tmp3_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),UNIT);
			_t = _t->getFirstChild();
			RefPascalAST tmp4_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENT);
			_t = _t->getNextSibling();
			_t = __t4;
			_t = _t->getNextSibling();
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::block(RefPascalAST _t) {
	RefPascalAST block_AST_in = _t;

	try {      // for error handling
		{ // ( ... )*
		for (;;) {
			if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case LABEL:
			{
				labelDeclarationPart(_t);
				_t = _retTree;
				break;
			}
			case CONST:
			{
				constantDefinitionPart(_t);
				_t = _retTree;
				break;
			}
			case TYPE:
			{
				typeDefinitionPart(_t);
				_t = _retTree;
				break;
			}
			case VAR:
			{
				variableDeclarationPart(_t);
				_t = _retTree;
				break;
			}
			case PROCEDURE:
			case FUNCTION:
			{
				procedureAndFunctionDeclarationPart(_t);
				_t = _retTree;
				break;
			}
			case USES:
			{
				usesUnitsPart(_t);
				_t = _retTree;
				break;
			}
			case IMPLEMENTATION:
			{
				RefPascalAST tmp5_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IMPLEMENTATION);
				_t = _t->getNextSibling();
				break;
			}
			default:
			{
				goto _loop8;
			}
			}
		}
		_loop8:;
		} // ( ... )*
		compoundStatement(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::identifierList(RefPascalAST _t) {
	RefPascalAST identifierList_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t104 = _t;
		RefPascalAST tmp6_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDLIST);
		_t = _t->getFirstChild();
		{ // ( ... )+
		int _cnt106=0;
		for (;;) {
			if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_t->getType() == IDENT)) {
				RefPascalAST tmp7_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENT);
				_t = _t->getNextSibling();
			}
			else {
				if ( _cnt106>=1 ) { goto _loop106; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));}
			}

			_cnt106++;
		}
		_loop106:;
		}  // ( ... )+
		_t = __t104;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::identifier(RefPascalAST _t) {
	RefPascalAST identifier_AST_in = _t;

	try {      // for error handling
		RefPascalAST tmp8_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENT);
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::labelDeclarationPart(RefPascalAST _t) {
	RefPascalAST labelDeclarationPart_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t12 = _t;
		RefPascalAST tmp9_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),LABEL);
		_t = _t->getFirstChild();
		{ // ( ... )+
		int _cnt14=0;
		for (;;) {
			if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_t->getType() == NUM_INT)) {
				label(_t);
				_t = _retTree;
			}
			else {
				if ( _cnt14>=1 ) { goto _loop14; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));}
			}

			_cnt14++;
		}
		_loop14:;
		}  // ( ... )+
		_t = __t12;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::constantDefinitionPart(RefPascalAST _t) {
	RefPascalAST constantDefinitionPart_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t17 = _t;
		RefPascalAST tmp10_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),CONST);
		_t = _t->getFirstChild();
		{ // ( ... )+
		int _cnt19=0;
		for (;;) {
			if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_t->getType() == EQUAL)) {
				constantDefinition(_t);
				_t = _retTree;
			}
			else {
				if ( _cnt19>=1 ) { goto _loop19; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));}
			}

			_cnt19++;
		}
		_loop19:;
		}  // ( ... )+
		_t = __t17;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::typeDefinitionPart(RefPascalAST _t) {
	RefPascalAST typeDefinitionPart_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t31 = _t;
		RefPascalAST tmp11_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),TYPE);
		_t = _t->getFirstChild();
		{ // ( ... )+
		int _cnt33=0;
		for (;;) {
			if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_t->getType() == TYPEDECL)) {
				typeDefinition(_t);
				_t = _retTree;
			}
			else {
				if ( _cnt33>=1 ) { goto _loop33; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));}
			}

			_cnt33++;
		}
		_loop33:;
		}  // ( ... )+
		_t = __t31;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

/** Yields a list of VARDECL-rooted subtrees with VAR at the overall root */
void PascalStoreWalker::variableDeclarationPart(RefPascalAST _t) {
	RefPascalAST variableDeclarationPart_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t83 = _t;
		RefPascalAST tmp12_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),VAR);
		_t = _t->getFirstChild();
		{ // ( ... )+
		int _cnt85=0;
		for (;;) {
			if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_t->getType() == VARDECL)) {
				variableDeclaration(_t);
				_t = _retTree;
			}
			else {
				if ( _cnt85>=1 ) { goto _loop85; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));}
			}

			_cnt85++;
		}
		_loop85:;
		}  // ( ... )+
		_t = __t83;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::procedureAndFunctionDeclarationPart(RefPascalAST _t) {
	RefPascalAST procedureAndFunctionDeclarationPart_AST_in = _t;

	try {      // for error handling
		procedureOrFunctionDeclaration(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::usesUnitsPart(RefPascalAST _t) {
	RefPascalAST usesUnitsPart_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t10 = _t;
		RefPascalAST tmp13_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),USES);
		_t = _t->getFirstChild();
		identifierList(_t);
		_t = _retTree;
		_t = __t10;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::compoundStatement(RefPascalAST _t) {
	RefPascalAST compoundStatement_AST_in = _t;

	try {      // for error handling
		statements(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::label(RefPascalAST _t) {
	RefPascalAST label_AST_in = _t;

	try {      // for error handling
		RefPascalAST tmp14_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),NUM_INT);
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::constantDefinition(RefPascalAST _t) {
	RefPascalAST constantDefinition_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t21 = _t;
		RefPascalAST tmp15_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),EQUAL);
		_t = _t->getFirstChild();
		RefPascalAST tmp16_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENT);
		_t = _t->getNextSibling();
		constant(_t);
		_t = _retTree;
		_t = __t21;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::constant(RefPascalAST _t) {
	RefPascalAST constant_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case NUM_INT:
		{
			RefPascalAST tmp17_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),NUM_INT);
			_t = _t->getNextSibling();
			break;
		}
		case NUM_REAL:
		{
			RefPascalAST tmp18_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),NUM_REAL);
			_t = _t->getNextSibling();
			break;
		}
		case PLUS:
		{
			RefPascalAST __t23 = _t;
			RefPascalAST tmp19_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PLUS);
			_t = _t->getFirstChild();
			{
			if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case NUM_INT:
			{
				RefPascalAST tmp20_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),NUM_INT);
				_t = _t->getNextSibling();
				break;
			}
			case NUM_REAL:
			{
				RefPascalAST tmp21_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),NUM_REAL);
				_t = _t->getNextSibling();
				break;
			}
			case IDENT:
			{
				RefPascalAST tmp22_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENT);
				_t = _t->getNextSibling();
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
			}
			}
			}
			_t = __t23;
			_t = _t->getNextSibling();
			break;
		}
		case MINUS:
		{
			RefPascalAST __t25 = _t;
			RefPascalAST tmp23_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),MINUS);
			_t = _t->getFirstChild();
			{
			if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case NUM_INT:
			{
				RefPascalAST tmp24_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),NUM_INT);
				_t = _t->getNextSibling();
				break;
			}
			case NUM_REAL:
			{
				RefPascalAST tmp25_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),NUM_REAL);
				_t = _t->getNextSibling();
				break;
			}
			case IDENT:
			{
				RefPascalAST tmp26_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENT);
				_t = _t->getNextSibling();
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
			}
			}
			}
			_t = __t25;
			_t = _t->getNextSibling();
			break;
		}
		case IDENT:
		{
			RefPascalAST tmp27_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENT);
			_t = _t->getNextSibling();
			break;
		}
		case STRING_LITERAL:
		{
			RefPascalAST tmp28_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),STRING_LITERAL);
			_t = _t->getNextSibling();
			break;
		}
		case CHR:
		{
			RefPascalAST __t27 = _t;
			RefPascalAST tmp29_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),CHR);
			_t = _t->getFirstChild();
			{
			if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case NUM_INT:
			{
				RefPascalAST tmp30_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),NUM_INT);
				_t = _t->getNextSibling();
				break;
			}
			case NUM_REAL:
			{
				RefPascalAST tmp31_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),NUM_REAL);
				_t = _t->getNextSibling();
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
			}
			}
			}
			_t = __t27;
			_t = _t->getNextSibling();
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::string(RefPascalAST _t) {
	RefPascalAST string_AST_in = _t;

	try {      // for error handling
		RefPascalAST tmp32_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),STRING_LITERAL);
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::typeDefinition(RefPascalAST _t) {
	RefPascalAST typeDefinition_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t35 = _t;
		RefPascalAST tmp33_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),TYPEDECL);
		_t = _t->getFirstChild();
		RefPascalAST tmp34_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENT);
		_t = _t->getNextSibling();
		{
		if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case SCALARTYPE:
		case ARRAY:
		case INTEGER:
		case BOOLEAN:
		case CHAR:
		case DOTDOT:
		case REAL:
		case STRING:
		case PACKED:
		case RECORD:
		case SET:
		case FILE:
		case POINTER:
		case IDENT:
		{
			type(_t);
			_t = _retTree;
			break;
		}
		case FUNCTION:
		{
			RefPascalAST __t37 = _t;
			RefPascalAST tmp35_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),FUNCTION);
			_t = _t->getFirstChild();
			{
			if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case ARGDECLS:
			{
				formalParameterList(_t);
				_t = _retTree;
				break;
			}
			case INTEGER:
			case BOOLEAN:
			case CHAR:
			case REAL:
			case STRING:
			case IDENT:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
			}
			}
			}
			resultType(_t);
			_t = _retTree;
			_t = __t37;
			_t = _t->getNextSibling();
			break;
		}
		case PROCEDURE:
		{
			RefPascalAST __t39 = _t;
			RefPascalAST tmp36_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PROCEDURE);
			_t = _t->getFirstChild();
			{
			if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case ARGDECLS:
			{
				formalParameterList(_t);
				_t = _retTree;
				break;
			}
			case 3:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
			}
			}
			}
			_t = __t39;
			_t = _t->getNextSibling();
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
		_t = __t35;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::type(RefPascalAST _t) {
	RefPascalAST type_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case SCALARTYPE:
		{
			RefPascalAST __t42 = _t;
			RefPascalAST tmp37_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),SCALARTYPE);
			_t = _t->getFirstChild();
			identifierList(_t);
			_t = _retTree;
			_t = __t42;
			_t = _t->getNextSibling();
			break;
		}
		case DOTDOT:
		{
			RefPascalAST __t43 = _t;
			RefPascalAST tmp38_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),DOTDOT);
			_t = _t->getFirstChild();
			constant(_t);
			_t = _retTree;
			constant(_t);
			_t = _retTree;
			_t = __t43;
			_t = _t->getNextSibling();
			break;
		}
		case INTEGER:
		case BOOLEAN:
		case CHAR:
		case REAL:
		case STRING:
		case IDENT:
		{
			typeIdentifier(_t);
			_t = _retTree;
			break;
		}
		case ARRAY:
		case PACKED:
		case RECORD:
		case SET:
		case FILE:
		{
			structuredType(_t);
			_t = _retTree;
			break;
		}
		case POINTER:
		{
			RefPascalAST __t44 = _t;
			RefPascalAST tmp39_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),POINTER);
			_t = _t->getFirstChild();
			typeIdentifier(_t);
			_t = _retTree;
			_t = __t44;
			_t = _t->getNextSibling();
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::formalParameterList(RefPascalAST _t) {
	RefPascalAST formalParameterList_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t94 = _t;
		RefPascalAST tmp40_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ARGDECLS);
		_t = _t->getFirstChild();
		{ // ( ... )+
		int _cnt96=0;
		for (;;) {
			if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_tokenSet_0.member(_t->getType()))) {
				formalParameterSection(_t);
				_t = _retTree;
			}
			else {
				if ( _cnt96>=1 ) { goto _loop96; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));}
			}

			_cnt96++;
		}
		_loop96:;
		}  // ( ... )+
		_t = __t94;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::resultType(RefPascalAST _t) {
	RefPascalAST resultType_AST_in = _t;

	try {      // for error handling
		typeIdentifier(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::typeIdentifier(RefPascalAST _t) {
	RefPascalAST typeIdentifier_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENT:
		{
			RefPascalAST tmp41_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENT);
			_t = _t->getNextSibling();
			break;
		}
		case CHAR:
		{
			RefPascalAST tmp42_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),CHAR);
			_t = _t->getNextSibling();
			break;
		}
		case BOOLEAN:
		{
			RefPascalAST tmp43_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),BOOLEAN);
			_t = _t->getNextSibling();
			break;
		}
		case INTEGER:
		{
			RefPascalAST tmp44_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),INTEGER);
			_t = _t->getNextSibling();
			break;
		}
		case REAL:
		{
			RefPascalAST tmp45_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),REAL);
			_t = _t->getNextSibling();
			break;
		}
		case STRING:
		{
			RefPascalAST __t46 = _t;
			RefPascalAST tmp46_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),STRING);
			_t = _t->getFirstChild();
			{
			if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case IDENT:
			{
				RefPascalAST tmp47_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENT);
				_t = _t->getNextSibling();
				break;
			}
			case NUM_INT:
			{
				RefPascalAST tmp48_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),NUM_INT);
				_t = _t->getNextSibling();
				break;
			}
			case NUM_REAL:
			{
				RefPascalAST tmp49_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),NUM_REAL);
				_t = _t->getNextSibling();
				break;
			}
			case 3:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
			}
			}
			}
			_t = __t46;
			_t = _t->getNextSibling();
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::structuredType(RefPascalAST _t) {
	RefPascalAST structuredType_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case PACKED:
		{
			RefPascalAST __t49 = _t;
			RefPascalAST tmp50_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PACKED);
			_t = _t->getFirstChild();
			unpackedStructuredType(_t);
			_t = _retTree;
			_t = __t49;
			_t = _t->getNextSibling();
			break;
		}
		case ARRAY:
		case RECORD:
		case SET:
		case FILE:
		{
			unpackedStructuredType(_t);
			_t = _retTree;
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::unpackedStructuredType(RefPascalAST _t) {
	RefPascalAST unpackedStructuredType_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case ARRAY:
		{
			arrayType(_t);
			_t = _retTree;
			break;
		}
		case RECORD:
		{
			recordType(_t);
			_t = _retTree;
			break;
		}
		case SET:
		{
			setType(_t);
			_t = _retTree;
			break;
		}
		case FILE:
		{
			fileType(_t);
			_t = _retTree;
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

/** Note here that the syntactic diff between brackets disappears.
 *  If the brackets mean different things semantically, we need
 *  two different alternatives here.
 */
void PascalStoreWalker::arrayType(RefPascalAST _t) {
	RefPascalAST arrayType_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t52 = _t;
		RefPascalAST tmp51_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ARRAY);
		_t = _t->getFirstChild();
		typeList(_t);
		_t = _retTree;
		type(_t);
		_t = _retTree;
		_t = __t52;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::recordType(RefPascalAST _t) {
	RefPascalAST recordType_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t58 = _t;
		RefPascalAST tmp52_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),RECORD);
		_t = _t->getFirstChild();
		fieldList(_t);
		_t = _retTree;
		_t = __t58;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::setType(RefPascalAST _t) {
	RefPascalAST setType_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t78 = _t;
		RefPascalAST tmp53_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),SET);
		_t = _t->getFirstChild();
		type(_t);
		_t = _retTree;
		_t = __t78;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::fileType(RefPascalAST _t) {
	RefPascalAST fileType_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t80 = _t;
		RefPascalAST tmp54_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),FILE);
		_t = _t->getFirstChild();
		{
		if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case SCALARTYPE:
		case ARRAY:
		case INTEGER:
		case BOOLEAN:
		case CHAR:
		case DOTDOT:
		case REAL:
		case STRING:
		case PACKED:
		case RECORD:
		case SET:
		case FILE:
		case POINTER:
		case IDENT:
		{
			type(_t);
			_t = _retTree;
			break;
		}
		case 3:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
		_t = __t80;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::typeList(RefPascalAST _t) {
	RefPascalAST typeList_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t54 = _t;
		RefPascalAST tmp55_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),TYPELIST);
		_t = _t->getFirstChild();
		{ // ( ... )+
		int _cnt56=0;
		for (;;) {
			if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_tokenSet_1.member(_t->getType()))) {
				type(_t);
				_t = _retTree;
			}
			else {
				if ( _cnt56>=1 ) { goto _loop56; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));}
			}

			_cnt56++;
		}
		_loop56:;
		}  // ( ... )+
		_t = __t54;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::fieldList(RefPascalAST _t) {
	RefPascalAST fieldList_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t60 = _t;
		RefPascalAST tmp56_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),FIELDLIST);
		_t = _t->getFirstChild();
		{
		if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case FIELD:
		{
			fixedPart(_t);
			_t = _retTree;
			{
			if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case CASE:
			{
				variantPart(_t);
				_t = _retTree;
				break;
			}
			case 3:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
			}
			}
			}
			break;
		}
		case CASE:
		{
			variantPart(_t);
			_t = _retTree;
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
		_t = __t60;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::fixedPart(RefPascalAST _t) {
	RefPascalAST fixedPart_AST_in = _t;

	try {      // for error handling
		{ // ( ... )+
		int _cnt65=0;
		for (;;) {
			if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_t->getType() == FIELD)) {
				recordSection(_t);
				_t = _retTree;
			}
			else {
				if ( _cnt65>=1 ) { goto _loop65; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));}
			}

			_cnt65++;
		}
		_loop65:;
		}  // ( ... )+
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::variantPart(RefPascalAST _t) {
	RefPascalAST variantPart_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t69 = _t;
		RefPascalAST tmp57_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),CASE);
		_t = _t->getFirstChild();
		tag(_t);
		_t = _retTree;
		{ // ( ... )+
		int _cnt71=0;
		for (;;) {
			if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_t->getType() == VARIANT_CASE)) {
				variant(_t);
				_t = _retTree;
			}
			else {
				if ( _cnt71>=1 ) { goto _loop71; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));}
			}

			_cnt71++;
		}
		_loop71:;
		}  // ( ... )+
		_t = __t69;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::recordSection(RefPascalAST _t) {
	RefPascalAST recordSection_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t67 = _t;
		RefPascalAST tmp58_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),FIELD);
		_t = _t->getFirstChild();
		identifierList(_t);
		_t = _retTree;
		type(_t);
		_t = _retTree;
		_t = __t67;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::tag(RefPascalAST _t) {
	RefPascalAST tag_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case VARIANT_TAG:
		{
			RefPascalAST __t73 = _t;
			RefPascalAST tmp59_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),VARIANT_TAG);
			_t = _t->getFirstChild();
			identifier(_t);
			_t = _retTree;
			typeIdentifier(_t);
			_t = _retTree;
			_t = __t73;
			_t = _t->getNextSibling();
			break;
		}
		case VARIANT_TAG_NO_ID:
		{
			RefPascalAST __t74 = _t;
			RefPascalAST tmp60_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),VARIANT_TAG_NO_ID);
			_t = _t->getFirstChild();
			typeIdentifier(_t);
			_t = _retTree;
			_t = __t74;
			_t = _t->getNextSibling();
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::variant(RefPascalAST _t) {
	RefPascalAST variant_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t76 = _t;
		RefPascalAST tmp61_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),VARIANT_CASE);
		_t = _t->getFirstChild();
		constList(_t);
		_t = _retTree;
		fieldList(_t);
		_t = _retTree;
		_t = __t76;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::constList(RefPascalAST _t) {
	RefPascalAST constList_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t108 = _t;
		RefPascalAST tmp62_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),CONSTLIST);
		_t = _t->getFirstChild();
		{ // ( ... )+
		int _cnt110=0;
		for (;;) {
			if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_tokenSet_2.member(_t->getType()))) {
				constant(_t);
				_t = _retTree;
			}
			else {
				if ( _cnt110>=1 ) { goto _loop110; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));}
			}

			_cnt110++;
		}
		_loop110:;
		}  // ( ... )+
		_t = __t108;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::variableDeclaration(RefPascalAST _t) {
	RefPascalAST variableDeclaration_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t87 = _t;
		RefPascalAST tmp63_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),VARDECL);
		_t = _t->getFirstChild();
		identifierList(_t);
		_t = _retTree;
		type(_t);
		_t = _retTree;
		_t = __t87;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::procedureOrFunctionDeclaration(RefPascalAST _t) {
	RefPascalAST procedureOrFunctionDeclaration_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case PROCEDURE:
		{
			procedureDeclaration(_t);
			_t = _retTree;
			break;
		}
		case FUNCTION:
		{
			functionDeclaration(_t);
			_t = _retTree;
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::procedureDeclaration(RefPascalAST _t) {
	RefPascalAST procedureDeclaration_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t91 = _t;
		RefPascalAST tmp64_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PROCEDURE);
		_t = _t->getFirstChild();
		RefPascalAST tmp65_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENT);
		_t = _t->getNextSibling();
		{
		if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case ARGDECLS:
		{
			formalParameterList(_t);
			_t = _retTree;
			break;
		}
		case BLOCK:
		case USES:
		case IMPLEMENTATION:
		case LABEL:
		case CONST:
		case TYPE:
		case VAR:
		case PROCEDURE:
		case FUNCTION:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
		block(_t);
		_t = _retTree;
		_t = __t91;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::functionDeclaration(RefPascalAST _t) {
	RefPascalAST functionDeclaration_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t112 = _t;
		RefPascalAST tmp66_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),FUNCTION);
		_t = _t->getFirstChild();
		RefPascalAST tmp67_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENT);
		_t = _t->getNextSibling();
		{
		if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case ARGDECLS:
		{
			formalParameterList(_t);
			_t = _retTree;
			break;
		}
		case INTEGER:
		case BOOLEAN:
		case CHAR:
		case REAL:
		case STRING:
		case IDENT:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
		resultType(_t);
		_t = _retTree;
		block(_t);
		_t = _retTree;
		_t = __t112;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::formalParameterSection(RefPascalAST _t) {
	RefPascalAST formalParameterSection_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case ARGDECL:
		{
			parameterGroup(_t);
			_t = _retTree;
			break;
		}
		case VAR:
		{
			RefPascalAST __t98 = _t;
			RefPascalAST tmp68_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),VAR);
			_t = _t->getFirstChild();
			parameterGroup(_t);
			_t = _retTree;
			_t = __t98;
			_t = _t->getNextSibling();
			break;
		}
		case FUNCTION:
		{
			RefPascalAST __t99 = _t;
			RefPascalAST tmp69_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),FUNCTION);
			_t = _t->getFirstChild();
			parameterGroup(_t);
			_t = _retTree;
			_t = __t99;
			_t = _t->getNextSibling();
			break;
		}
		case PROCEDURE:
		{
			RefPascalAST __t100 = _t;
			RefPascalAST tmp70_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PROCEDURE);
			_t = _t->getFirstChild();
			parameterGroup(_t);
			_t = _retTree;
			_t = __t100;
			_t = _t->getNextSibling();
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::parameterGroup(RefPascalAST _t) {
	RefPascalAST parameterGroup_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t102 = _t;
		RefPascalAST tmp71_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ARGDECL);
		_t = _t->getFirstChild();
		identifierList(_t);
		_t = _retTree;
		typeIdentifier(_t);
		_t = _retTree;
		_t = __t102;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::statement(RefPascalAST _t) {
	RefPascalAST statement_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case COLON:
		{
			RefPascalAST __t116 = _t;
			RefPascalAST tmp72_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),COLON);
			_t = _t->getFirstChild();
			label(_t);
			_t = _retTree;
			unlabelledStatement(_t);
			_t = _retTree;
			_t = __t116;
			_t = _t->getNextSibling();
			break;
		}
		case BLOCK:
		case PROC_CALL:
		case ASSIGN:
		case CASE:
		case GOTO:
		case IF:
		case WHILE:
		case REPEAT:
		case FOR:
		case WITH:
		{
			unlabelledStatement(_t);
			_t = _retTree;
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::unlabelledStatement(RefPascalAST _t) {
	RefPascalAST unlabelledStatement_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case PROC_CALL:
		case ASSIGN:
		case GOTO:
		{
			simpleStatement(_t);
			_t = _retTree;
			break;
		}
		case BLOCK:
		case CASE:
		case IF:
		case WHILE:
		case REPEAT:
		case FOR:
		case WITH:
		{
			structuredStatement(_t);
			_t = _retTree;
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::simpleStatement(RefPascalAST _t) {
	RefPascalAST simpleStatement_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case ASSIGN:
		{
			assignmentStatement(_t);
			_t = _retTree;
			break;
		}
		case PROC_CALL:
		{
			procedureStatement(_t);
			_t = _retTree;
			break;
		}
		case GOTO:
		{
			gotoStatement(_t);
			_t = _retTree;
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::structuredStatement(RefPascalAST _t) {
	RefPascalAST structuredStatement_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case BLOCK:
		{
			compoundStatement(_t);
			_t = _retTree;
			break;
		}
		case CASE:
		case IF:
		{
			conditionalStatement(_t);
			_t = _retTree;
			break;
		}
		case WHILE:
		case REPEAT:
		case FOR:
		{
			repetetiveStatement(_t);
			_t = _retTree;
			break;
		}
		case WITH:
		{
			withStatement(_t);
			_t = _retTree;
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::assignmentStatement(RefPascalAST _t) {
	RefPascalAST assignmentStatement_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t120 = _t;
		RefPascalAST tmp73_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ASSIGN);
		_t = _t->getFirstChild();
		variable(_t);
		_t = _retTree;
		expression(_t);
		_t = _retTree;
		_t = __t120;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::procedureStatement(RefPascalAST _t) {
	RefPascalAST procedureStatement_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t166 = _t;
		RefPascalAST tmp74_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PROC_CALL);
		_t = _t->getFirstChild();
		RefPascalAST tmp75_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENT);
		_t = _t->getNextSibling();
		{
		if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case ARGLIST:
		{
			parameterList(_t);
			_t = _retTree;
			break;
		}
		case 3:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
		_t = __t166;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::gotoStatement(RefPascalAST _t) {
	RefPascalAST gotoStatement_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t170 = _t;
		RefPascalAST tmp76_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),GOTO);
		_t = _t->getFirstChild();
		label(_t);
		_t = _retTree;
		_t = __t170;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
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
void PascalStoreWalker::variable(RefPascalAST _t) {
	RefPascalAST variable_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case LBRACK:
		{
			RefPascalAST __t122 = _t;
			RefPascalAST tmp77_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),LBRACK);
			_t = _t->getFirstChild();
			variable(_t);
			_t = _retTree;
			{ // ( ... )+
			int _cnt124=0;
			for (;;) {
				if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
					_t = ASTNULL;
				if ((_tokenSet_3.member(_t->getType()))) {
					expression(_t);
					_t = _retTree;
				}
				else {
					if ( _cnt124>=1 ) { goto _loop124; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));}
				}

				_cnt124++;
			}
			_loop124:;
			}  // ( ... )+
			_t = __t122;
			_t = _t->getNextSibling();
			break;
		}
		case LBRACK2:
		{
			RefPascalAST __t125 = _t;
			RefPascalAST tmp78_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),LBRACK2);
			_t = _t->getFirstChild();
			variable(_t);
			_t = _retTree;
			{ // ( ... )+
			int _cnt127=0;
			for (;;) {
				if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
					_t = ASTNULL;
				if ((_tokenSet_3.member(_t->getType()))) {
					expression(_t);
					_t = _retTree;
				}
				else {
					if ( _cnt127>=1 ) { goto _loop127; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));}
				}

				_cnt127++;
			}
			_loop127:;
			}  // ( ... )+
			_t = __t125;
			_t = _t->getNextSibling();
			break;
		}
		case DOT:
		{
			RefPascalAST __t128 = _t;
			RefPascalAST tmp79_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),DOT);
			_t = _t->getFirstChild();
			variable(_t);
			_t = _retTree;
			RefPascalAST tmp80_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENT);
			_t = _t->getNextSibling();
			_t = __t128;
			_t = _t->getNextSibling();
			break;
		}
		case POINTER:
		{
			RefPascalAST __t129 = _t;
			RefPascalAST tmp81_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),POINTER);
			_t = _t->getFirstChild();
			variable(_t);
			_t = _retTree;
			_t = __t129;
			_t = _t->getNextSibling();
			break;
		}
		case AT:
		{
			RefPascalAST __t130 = _t;
			RefPascalAST tmp82_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),AT);
			_t = _t->getFirstChild();
			RefPascalAST tmp83_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENT);
			_t = _t->getNextSibling();
			_t = __t130;
			_t = _t->getNextSibling();
			break;
		}
		case IDENT:
		{
			RefPascalAST tmp84_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENT);
			_t = _t->getNextSibling();
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::expression(RefPascalAST _t) {
	RefPascalAST expression_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case EQUAL:
		{
			RefPascalAST __t132 = _t;
			RefPascalAST tmp85_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),EQUAL);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			expression(_t);
			_t = _retTree;
			_t = __t132;
			_t = _t->getNextSibling();
			break;
		}
		case NOT_EQUAL:
		{
			RefPascalAST __t133 = _t;
			RefPascalAST tmp86_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),NOT_EQUAL);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			expression(_t);
			_t = _retTree;
			_t = __t133;
			_t = _t->getNextSibling();
			break;
		}
		case LTH:
		{
			RefPascalAST __t134 = _t;
			RefPascalAST tmp87_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),LTH);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			expression(_t);
			_t = _retTree;
			_t = __t134;
			_t = _t->getNextSibling();
			break;
		}
		case LE:
		{
			RefPascalAST __t135 = _t;
			RefPascalAST tmp88_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),LE);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			expression(_t);
			_t = _retTree;
			_t = __t135;
			_t = _t->getNextSibling();
			break;
		}
		case GE:
		{
			RefPascalAST __t136 = _t;
			RefPascalAST tmp89_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),GE);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			expression(_t);
			_t = _retTree;
			_t = __t136;
			_t = _t->getNextSibling();
			break;
		}
		case GT:
		{
			RefPascalAST __t137 = _t;
			RefPascalAST tmp90_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),GT);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			expression(_t);
			_t = _retTree;
			_t = __t137;
			_t = _t->getNextSibling();
			break;
		}
		case IN:
		{
			RefPascalAST __t138 = _t;
			RefPascalAST tmp91_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IN);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			expression(_t);
			_t = _retTree;
			_t = __t138;
			_t = _t->getNextSibling();
			break;
		}
		case PLUS:
		{
			RefPascalAST __t139 = _t;
			RefPascalAST tmp92_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PLUS);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			{
			if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case FUNC_CALL:
			case DOT:
			case EQUAL:
			case NUM_INT:
			case LBRACK:
			case SET:
			case POINTER:
			case LE:
			case GE:
			case LTH:
			case GT:
			case NOT_EQUAL:
			case IN:
			case PLUS:
			case MINUS:
			case OR:
			case STAR:
			case SLASH:
			case DIV:
			case MOD:
			case AND:
			case NOT:
			case AT:
			case LBRACK2:
			case STRING_LITERAL:
			case CHR:
			case NIL:
			case NUM_REAL:
			case IDENT:
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
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
			}
			}
			}
			_t = __t139;
			_t = _t->getNextSibling();
			break;
		}
		case MINUS:
		{
			RefPascalAST __t141 = _t;
			RefPascalAST tmp93_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),MINUS);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			{
			if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case FUNC_CALL:
			case DOT:
			case EQUAL:
			case NUM_INT:
			case LBRACK:
			case SET:
			case POINTER:
			case LE:
			case GE:
			case LTH:
			case GT:
			case NOT_EQUAL:
			case IN:
			case PLUS:
			case MINUS:
			case OR:
			case STAR:
			case SLASH:
			case DIV:
			case MOD:
			case AND:
			case NOT:
			case AT:
			case LBRACK2:
			case STRING_LITERAL:
			case CHR:
			case NIL:
			case NUM_REAL:
			case IDENT:
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
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
			}
			}
			}
			_t = __t141;
			_t = _t->getNextSibling();
			break;
		}
		case OR:
		{
			RefPascalAST __t143 = _t;
			RefPascalAST tmp94_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),OR);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			expression(_t);
			_t = _retTree;
			_t = __t143;
			_t = _t->getNextSibling();
			break;
		}
		case STAR:
		{
			RefPascalAST __t144 = _t;
			RefPascalAST tmp95_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),STAR);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			expression(_t);
			_t = _retTree;
			_t = __t144;
			_t = _t->getNextSibling();
			break;
		}
		case SLASH:
		{
			RefPascalAST __t145 = _t;
			RefPascalAST tmp96_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),SLASH);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			expression(_t);
			_t = _retTree;
			_t = __t145;
			_t = _t->getNextSibling();
			break;
		}
		case DIV:
		{
			RefPascalAST __t146 = _t;
			RefPascalAST tmp97_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),DIV);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			expression(_t);
			_t = _retTree;
			_t = __t146;
			_t = _t->getNextSibling();
			break;
		}
		case MOD:
		{
			RefPascalAST __t147 = _t;
			RefPascalAST tmp98_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),MOD);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			expression(_t);
			_t = _retTree;
			_t = __t147;
			_t = _t->getNextSibling();
			break;
		}
		case AND:
		{
			RefPascalAST __t148 = _t;
			RefPascalAST tmp99_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),AND);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			expression(_t);
			_t = _retTree;
			_t = __t148;
			_t = _t->getNextSibling();
			break;
		}
		case NOT:
		{
			RefPascalAST __t149 = _t;
			RefPascalAST tmp100_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),NOT);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			_t = __t149;
			_t = _t->getNextSibling();
			break;
		}
		case DOT:
		case LBRACK:
		case POINTER:
		case AT:
		case LBRACK2:
		case IDENT:
		{
			variable(_t);
			_t = _retTree;
			break;
		}
		case FUNC_CALL:
		{
			functionDesignator(_t);
			_t = _retTree;
			break;
		}
		case SET:
		{
			set(_t);
			_t = _retTree;
			break;
		}
		case NUM_INT:
		{
			RefPascalAST tmp101_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),NUM_INT);
			_t = _t->getNextSibling();
			break;
		}
		case NUM_REAL:
		{
			RefPascalAST tmp102_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),NUM_REAL);
			_t = _t->getNextSibling();
			break;
		}
		case CHR:
		{
			RefPascalAST __t150 = _t;
			RefPascalAST tmp103_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),CHR);
			_t = _t->getFirstChild();
			{
			if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case NUM_INT:
			{
				RefPascalAST tmp104_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),NUM_INT);
				_t = _t->getNextSibling();
				break;
			}
			case NUM_REAL:
			{
				RefPascalAST tmp105_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),NUM_REAL);
				_t = _t->getNextSibling();
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
			}
			}
			}
			_t = __t150;
			_t = _t->getNextSibling();
			break;
		}
		case STRING_LITERAL:
		{
			string(_t);
			_t = _retTree;
			break;
		}
		case NIL:
		{
			RefPascalAST tmp106_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),NIL);
			_t = _t->getNextSibling();
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::functionDesignator(RefPascalAST _t) {
	RefPascalAST functionDesignator_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t153 = _t;
		RefPascalAST tmp107_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),FUNC_CALL);
		_t = _t->getFirstChild();
		RefPascalAST tmp108_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENT);
		_t = _t->getNextSibling();
		{
		if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case ARGLIST:
		{
			parameterList(_t);
			_t = _retTree;
			break;
		}
		case 3:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
		_t = __t153;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::set(RefPascalAST _t) {
	RefPascalAST set_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t160 = _t;
		RefPascalAST tmp109_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),SET);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_tokenSet_4.member(_t->getType()))) {
				element(_t);
				_t = _retTree;
			}
			else {
				goto _loop162;
			}

		}
		_loop162:;
		} // ( ... )*
		_t = __t160;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::parameterList(RefPascalAST _t) {
	RefPascalAST parameterList_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t156 = _t;
		RefPascalAST tmp110_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ARGLIST);
		_t = _t->getFirstChild();
		{ // ( ... )+
		int _cnt158=0;
		for (;;) {
			if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_tokenSet_3.member(_t->getType()))) {
				actualParameter(_t);
				_t = _retTree;
			}
			else {
				if ( _cnt158>=1 ) { goto _loop158; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));}
			}

			_cnt158++;
		}
		_loop158:;
		}  // ( ... )+
		_t = __t156;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::actualParameter(RefPascalAST _t) {
	RefPascalAST actualParameter_AST_in = _t;

	try {      // for error handling
		expression(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::element(RefPascalAST _t) {
	RefPascalAST element_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case DOTDOT:
		{
			RefPascalAST __t164 = _t;
			RefPascalAST tmp111_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),DOTDOT);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			expression(_t);
			_t = _retTree;
			_t = __t164;
			_t = _t->getNextSibling();
			break;
		}
		case FUNC_CALL:
		case DOT:
		case EQUAL:
		case NUM_INT:
		case LBRACK:
		case SET:
		case POINTER:
		case LE:
		case GE:
		case LTH:
		case GT:
		case NOT_EQUAL:
		case IN:
		case PLUS:
		case MINUS:
		case OR:
		case STAR:
		case SLASH:
		case DIV:
		case MOD:
		case AND:
		case NOT:
		case AT:
		case LBRACK2:
		case STRING_LITERAL:
		case CHR:
		case NIL:
		case NUM_REAL:
		case IDENT:
		{
			expression(_t);
			_t = _retTree;
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::conditionalStatement(RefPascalAST _t) {
	RefPascalAST conditionalStatement_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IF:
		{
			ifStatement(_t);
			_t = _retTree;
			break;
		}
		case CASE:
		{
			caseStatement(_t);
			_t = _retTree;
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::repetetiveStatement(RefPascalAST _t) {
	RefPascalAST repetetiveStatement_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case WHILE:
		{
			whileStatement(_t);
			_t = _retTree;
			break;
		}
		case REPEAT:
		{
			repeatStatement(_t);
			_t = _retTree;
			break;
		}
		case FOR:
		{
			forStatement(_t);
			_t = _retTree;
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::withStatement(RefPascalAST _t) {
	RefPascalAST withStatement_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t201 = _t;
		RefPascalAST tmp112_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),WITH);
		_t = _t->getFirstChild();
		recordVariableList(_t);
		_t = _retTree;
		statement(_t);
		_t = _retTree;
		_t = __t201;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::statements(RefPascalAST _t) {
	RefPascalAST statements_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t174 = _t;
		RefPascalAST tmp113_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),BLOCK);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_tokenSet_5.member(_t->getType()))) {
				statement(_t);
				_t = _retTree;
			}
			else {
				goto _loop176;
			}

		}
		_loop176:;
		} // ( ... )*
		_t = __t174;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::ifStatement(RefPascalAST _t) {
	RefPascalAST ifStatement_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t179 = _t;
		RefPascalAST tmp114_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IF);
		_t = _t->getFirstChild();
		expression(_t);
		_t = _retTree;
		statement(_t);
		_t = _retTree;
		{
		if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case BLOCK:
		case PROC_CALL:
		case COLON:
		case ASSIGN:
		case CASE:
		case GOTO:
		case IF:
		case WHILE:
		case REPEAT:
		case FOR:
		case WITH:
		{
			statement(_t);
			_t = _retTree;
			break;
		}
		case 3:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
		_t = __t179;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::caseStatement(RefPascalAST _t) {
	RefPascalAST caseStatement_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t182 = _t;
		RefPascalAST tmp115_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),CASE);
		_t = _t->getFirstChild();
		expression(_t);
		_t = _retTree;
		{ // ( ... )+
		int _cnt184=0;
		for (;;) {
			if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_t->getType() == COLON)) {
				caseListElement(_t);
				_t = _retTree;
			}
			else {
				if ( _cnt184>=1 ) { goto _loop184; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));}
			}

			_cnt184++;
		}
		_loop184:;
		}  // ( ... )+
		{
		if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case BLOCK:
		{
			statements(_t);
			_t = _retTree;
			break;
		}
		case 3:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
		_t = __t182;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::caseListElement(RefPascalAST _t) {
	RefPascalAST caseListElement_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t187 = _t;
		RefPascalAST tmp116_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),COLON);
		_t = _t->getFirstChild();
		constList(_t);
		_t = _retTree;
		statement(_t);
		_t = _retTree;
		_t = __t187;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::whileStatement(RefPascalAST _t) {
	RefPascalAST whileStatement_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t190 = _t;
		RefPascalAST tmp117_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),WHILE);
		_t = _t->getFirstChild();
		expression(_t);
		_t = _retTree;
		statement(_t);
		_t = _retTree;
		_t = __t190;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::repeatStatement(RefPascalAST _t) {
	RefPascalAST repeatStatement_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t192 = _t;
		RefPascalAST tmp118_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),REPEAT);
		_t = _t->getFirstChild();
		statements(_t);
		_t = _retTree;
		expression(_t);
		_t = _retTree;
		_t = __t192;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::forStatement(RefPascalAST _t) {
	RefPascalAST forStatement_AST_in = _t;

	try {      // for error handling
		RefPascalAST __t194 = _t;
		RefPascalAST tmp119_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),FOR);
		_t = _t->getFirstChild();
		RefPascalAST tmp120_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENT);
		_t = _t->getNextSibling();
		forList(_t);
		_t = _retTree;
		statement(_t);
		_t = _retTree;
		_t = __t194;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::forList(RefPascalAST _t) {
	RefPascalAST forList_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case TO:
		{
			RefPascalAST __t196 = _t;
			RefPascalAST tmp121_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),TO);
			_t = _t->getFirstChild();
			initialValue(_t);
			_t = _retTree;
			finalValue(_t);
			_t = _retTree;
			_t = __t196;
			_t = _t->getNextSibling();
			break;
		}
		case DOWNTO:
		{
			RefPascalAST __t197 = _t;
			RefPascalAST tmp122_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),DOWNTO);
			_t = _t->getFirstChild();
			initialValue(_t);
			_t = _retTree;
			finalValue(_t);
			_t = _retTree;
			_t = __t197;
			_t = _t->getNextSibling();
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::initialValue(RefPascalAST _t) {
	RefPascalAST initialValue_AST_in = _t;

	try {      // for error handling
		expression(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::finalValue(RefPascalAST _t) {
	RefPascalAST finalValue_AST_in = _t;

	try {      // for error handling
		expression(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void PascalStoreWalker::recordVariableList(RefPascalAST _t) {
	RefPascalAST recordVariableList_AST_in = _t;

	try {      // for error handling
		{ // ( ... )+
		int _cnt204=0;
		for (;;) {
			if (_t == static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_tokenSet_6.member(_t->getType()))) {
				variable(_t);
				_t = _retTree;
			}
			else {
				if ( _cnt204>=1 ) { goto _loop204; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));}
			}

			_cnt204++;
		}
		_loop204:;
		}  // ( ... )+
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefPascalAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

RefPascalAST PascalStoreWalker::getAST()
{
	return returnAST;
}

void PascalStoreWalker::initializeASTFactory( ANTLR_USE_NAMESPACE(antlr)ASTFactory& // factory
                                              )
{
}
const char* PascalStoreWalker::tokenNames[] = {
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

const unsigned long PascalStoreWalker::_tokenSet_0_data_[] = { 262144UL, 172032UL, 0UL, 0UL, 0UL, 0UL };
// ARGDECL "var" "procedure" "function"
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalStoreWalker::_tokenSet_0(_tokenSet_0_data_,6);
const unsigned long PascalStoreWalker::_tokenSet_1_data_[] = { 512UL, 262144UL, 3641384976UL, 1UL, 0UL, 8UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SCALARTYPE "array" "integer" "boolean" "char" DOTDOT "real" "string"
// "packed" "record" "set" "file" POINTER IDENT
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalStoreWalker::_tokenSet_1(_tokenSet_1_data_,12);
const unsigned long PascalStoreWalker::_tokenSet_2_data_[] = { 0UL, 0UL, 8UL, 50331648UL, 2147483648UL, 13UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// NUM_INT PLUS MINUS STRING_LITERAL "chr" NUM_REAL IDENT
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalStoreWalker::_tokenSet_2(_tokenSet_2_data_,12);
const unsigned long PascalStoreWalker::_tokenSet_3_data_[] = { 4194432UL, 2048UL, 1107296264UL, 4152229889UL, 2155872329UL, 15UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// FUNC_CALL DOT EQUAL NUM_INT LBRACK "set" POINTER LE GE LTH GT NOT_EQUAL
// "in" PLUS MINUS "or" STAR SLASH "div" "mod" "and" "not" AT LBRACK2 STRING_LITERAL
// "chr" "nil" NUM_REAL IDENT
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalStoreWalker::_tokenSet_3(_tokenSet_3_data_,12);
const unsigned long PascalStoreWalker::_tokenSet_4_data_[] = { 4194432UL, 2048UL, 1107427336UL, 4152229889UL, 2155872329UL, 15UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// FUNC_CALL DOT EQUAL NUM_INT DOTDOT LBRACK "set" POINTER LE GE LTH GT
// NOT_EQUAL "in" PLUS MINUS "or" STAR SLASH "div" "mod" "and" "not" AT
// LBRACK2 STRING_LITERAL "chr" "nil" NUM_REAL IDENT
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalStoreWalker::_tokenSet_4(_tokenSet_4_data_,12);
const unsigned long PascalStoreWalker::_tokenSet_5_data_[] = { 272UL, 16384UL, 537133056UL, 0UL, 4888576UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// BLOCK PROC_CALL COLON ASSIGN "case" "goto" "if" "while" "repeat" "for"
// "with"
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalStoreWalker::_tokenSet_5(_tokenSet_5_data_,12);
const unsigned long PascalStoreWalker::_tokenSet_6_data_[] = { 4194304UL, 0UL, 33554432UL, 1UL, 8388672UL, 8UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// DOT LBRACK POINTER AT LBRACK2 IDENT
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalStoreWalker::_tokenSet_6(_tokenSet_6_data_,12);


