/* $ANTLR 2.7.7 (20061129): "ada.tree.g" -> "AdaTreeParserSuper.cpp"$ */
#include "AdaTreeParserSuper.hpp"
#include <antlr/Token.hpp>
#include <antlr/AST.hpp>
#include <antlr/NoViableAltException.hpp>
#include <antlr/MismatchedTokenException.hpp>
#include <antlr/SemanticException.hpp>
#include <antlr/BitSet.hpp>
#line 1 "ada.tree.g"
#line 11 "AdaTreeParserSuper.cpp"
AdaTreeParserSuper::AdaTreeParserSuper()
	: ANTLR_USE_NAMESPACE(antlr)TreeParser() {
}

void AdaTreeParserSuper::compilation_unit(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST compilation_unit_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		context_items_opt(_t);
		_t = _retTree;
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case LIBRARY_ITEM:
		{
			library_item(_t);
			_t = _retTree;
			break;
		}
		case SUBUNIT:
		{
			subunit(_t);
			_t = _retTree;
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
		}
		}
		}
		{ // ( ... )*
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			if ((_t->getType() == PRAGMA)) {
				pragma(_t);
				_t = _retTree;
			}
			else {
				goto _loop4;
			}
			
		}
		_loop4:;
		} // ( ... )*
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::context_items_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST context_items_opt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t12 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp1_AST_in = _t;
		match(_t,CONTEXT_CLAUSE);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case PRAGMA:
			{
				pragma(_t);
				_t = _retTree;
				break;
			}
			case WITH_CLAUSE:
			{
				with_clause(_t);
				_t = _retTree;
				break;
			}
			case USE_CLAUSE:
			case USE_TYPE_CLAUSE:
			{
				use_clause(_t);
				_t = _retTree;
				break;
			}
			default:
			{
				goto _loop14;
			}
			}
		}
		_loop14:;
		} // ( ... )*
		_t = __t12;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::library_item(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST library_item_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t32 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp2_AST_in = _t;
		match(_t,LIBRARY_ITEM);
		_t = _t->getFirstChild();
		modifiers(_t);
		_t = _retTree;
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case ABSTRACT_FUNCTION_DECLARATION:
		case ABSTRACT_PROCEDURE_DECLARATION:
		case FUNCTION_BODY:
		case FUNCTION_BODY_STUB:
		case FUNCTION_DECLARATION:
		case FUNCTION_RENAMING_DECLARATION:
		case GENERIC_FUNCTION_INSTANTIATION:
		case GENERIC_PROCEDURE_INSTANTIATION:
		case PROCEDURE_BODY:
		case PROCEDURE_BODY_STUB:
		case PROCEDURE_DECLARATION:
		case PROCEDURE_RENAMING_DECLARATION:
		{
			subprog_decl_or_rename_or_inst_or_body(_t);
			_t = _retTree;
			break;
		}
		case PACKAGE_BODY:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t34 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp3_AST_in = _t;
			match(_t,PACKAGE_BODY);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			pkg_body_part(_t);
			_t = _retTree;
			_t = __t34;
			_t = _t->getNextSibling();
			break;
		}
		case GENERIC_PACKAGE_INSTANTIATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t35 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp4_AST_in = _t;
			match(_t,GENERIC_PACKAGE_INSTANTIATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			generic_inst(_t);
			_t = _retTree;
			_t = __t35;
			_t = _t->getNextSibling();
			break;
		}
		case PACKAGE_SPECIFICATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t36 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp5_AST_in = _t;
			match(_t,PACKAGE_SPECIFICATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			pkg_spec_part(_t);
			_t = _retTree;
			_t = __t36;
			_t = _t->getNextSibling();
			break;
		}
		case PACKAGE_RENAMING_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t37 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp6_AST_in = _t;
			match(_t,PACKAGE_RENAMING_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			renames(_t);
			_t = _retTree;
			_t = __t37;
			_t = _t->getNextSibling();
			break;
		}
		case GENERIC_PACKAGE_DECLARATION:
		case GENERIC_FUNCTION_DECLARATION:
		case GENERIC_FUNCTION_RENAMING:
		case GENERIC_PACKAGE_RENAMING:
		case GENERIC_PROCEDURE_DECLARATION:
		case GENERIC_PROCEDURE_RENAMING:
		{
			generic_decl(_t);
			_t = _retTree;
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
		}
		}
		}
		_t = __t32;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::subunit(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST subunit_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t501 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp7_AST_in = _t;
		match(_t,SUBUNIT);
		_t = _t->getFirstChild();
		compound_name(_t);
		_t = _retTree;
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case FUNCTION_BODY:
		case PROCEDURE_BODY:
		{
			subprogram_body(_t);
			_t = _retTree;
			break;
		}
		case PACKAGE_BODY:
		{
			package_body(_t);
			_t = _retTree;
			break;
		}
		case TASK_BODY:
		{
			task_body(_t);
			_t = _retTree;
			break;
		}
		case PROTECTED_BODY:
		{
			protected_body(_t);
			_t = _retTree;
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
		}
		}
		}
		_t = __t501;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::pragma(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST pragma_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t6 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp8_AST_in = _t;
		match(_t,PRAGMA);
		_t = _t->getFirstChild();
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp9_AST_in = _t;
		match(_t,IDENTIFIER);
		_t = _t->getNextSibling();
		{ // ( ... )*
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			if ((_tokenSet_0.member(_t->getType()))) {
				pragma_arg(_t);
				_t = _retTree;
			}
			else {
				goto _loop8;
			}
			
		}
		_loop8:;
		} // ( ... )*
		_t = __t6;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::pragma_arg(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST pragma_arg_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case RIGHT_SHAFT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t10 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp10_AST_in = _t;
			match(_t,RIGHT_SHAFT);
			_t = _t->getFirstChild();
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp11_AST_in = _t;
			match(_t,IDENTIFIER);
			_t = _t->getNextSibling();
			expression(_t);
			_t = _retTree;
			_t = __t10;
			_t = _t->getNextSibling();
			break;
		}
		case IDENTIFIER:
		case DOT:
		case TIC:
		case IN:
		case CHARACTER_LITERAL:
		case CHAR_STRING:
		case NuLL:
		case MOD:
		case OR:
		case AND:
		case XOR:
		case NOT:
		case EQ:
		case NE:
		case LT_:
		case LE:
		case GT:
		case GE:
		case PLUS:
		case MINUS:
		case CONCAT:
		case STAR:
		case DIV:
		case REM:
		case ABS:
		case EXPON:
		case NUMERIC_LIT:
		case ALLOCATOR:
		case INDEXED_COMPONENT:
		case OPERATOR_SYMBOL:
		case AND_THEN:
		case NOT_IN:
		case OR_ELSE:
		case PARENTHESIZED_PRIMARY:
		case UNARY_MINUS:
		case UNARY_PLUS:
		{
			expression(_t);
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

void AdaTreeParserSuper::expression(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST expression_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case AND:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t459 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp12_AST_in = _t;
			match(_t,AND);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			relation(_t);
			_t = _retTree;
			_t = __t459;
			_t = _t->getNextSibling();
			break;
		}
		case AND_THEN:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t460 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp13_AST_in = _t;
			match(_t,AND_THEN);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			relation(_t);
			_t = _retTree;
			_t = __t460;
			_t = _t->getNextSibling();
			break;
		}
		case OR:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t461 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp14_AST_in = _t;
			match(_t,OR);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			relation(_t);
			_t = _retTree;
			_t = __t461;
			_t = _t->getNextSibling();
			break;
		}
		case OR_ELSE:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t462 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp15_AST_in = _t;
			match(_t,OR_ELSE);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			relation(_t);
			_t = _retTree;
			_t = __t462;
			_t = _t->getNextSibling();
			break;
		}
		case XOR:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t463 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp16_AST_in = _t;
			match(_t,XOR);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			relation(_t);
			_t = _retTree;
			_t = __t463;
			_t = _t->getNextSibling();
			break;
		}
		case IDENTIFIER:
		case DOT:
		case TIC:
		case IN:
		case CHARACTER_LITERAL:
		case CHAR_STRING:
		case NuLL:
		case MOD:
		case NOT:
		case EQ:
		case NE:
		case LT_:
		case LE:
		case GT:
		case GE:
		case PLUS:
		case MINUS:
		case CONCAT:
		case STAR:
		case DIV:
		case REM:
		case ABS:
		case EXPON:
		case NUMERIC_LIT:
		case ALLOCATOR:
		case INDEXED_COMPONENT:
		case OPERATOR_SYMBOL:
		case NOT_IN:
		case PARENTHESIZED_PRIMARY:
		case UNARY_MINUS:
		case UNARY_PLUS:
		{
			relation(_t);
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

void AdaTreeParserSuper::with_clause(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST with_clause_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t16 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp17_AST_in = _t;
		match(_t,WITH_CLAUSE);
		_t = _t->getFirstChild();
		{ // ( ... )+
		int _cnt18=0;
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			if ((_t->getType() == IDENTIFIER || _t->getType() == DOT)) {
				compound_name(_t);
				_t = _retTree;
			}
			else {
				if ( _cnt18>=1 ) { goto _loop18; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);}
			}
			
			_cnt18++;
		}
		_loop18:;
		}  // ( ... )+
		_t = __t16;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::use_clause(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST use_clause_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case USE_TYPE_CLAUSE:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t22 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp18_AST_in = _t;
			match(_t,USE_TYPE_CLAUSE);
			_t = _t->getFirstChild();
			{ // ( ... )+
			int _cnt24=0;
			for (;;) {
				if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
					_t = ASTNULL;
				if ((_t->getType() == IDENTIFIER || _t->getType() == DOT || _t->getType() == TIC)) {
					subtype_mark(_t);
					_t = _retTree;
				}
				else {
					if ( _cnt24>=1 ) { goto _loop24; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);}
				}
				
				_cnt24++;
			}
			_loop24:;
			}  // ( ... )+
			_t = __t22;
			_t = _t->getNextSibling();
			break;
		}
		case USE_CLAUSE:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t25 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp19_AST_in = _t;
			match(_t,USE_CLAUSE);
			_t = _t->getFirstChild();
			{ // ( ... )+
			int _cnt27=0;
			for (;;) {
				if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
					_t = ASTNULL;
				if ((_t->getType() == IDENTIFIER || _t->getType() == DOT)) {
					compound_name(_t);
					_t = _retTree;
				}
				else {
					if ( _cnt27>=1 ) { goto _loop27; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);}
				}
				
				_cnt27++;
			}
			_loop27:;
			}  // ( ... )+
			_t = __t25;
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

void AdaTreeParserSuper::compound_name(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST compound_name_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENTIFIER:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp20_AST_in = _t;
			match(_t,IDENTIFIER);
			_t = _t->getNextSibling();
			break;
		}
		case DOT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t20 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp21_AST_in = _t;
			match(_t,DOT);
			_t = _t->getFirstChild();
			compound_name(_t);
			_t = _retTree;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp22_AST_in = _t;
			match(_t,IDENTIFIER);
			_t = _t->getNextSibling();
			_t = __t20;
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

void AdaTreeParserSuper::subtype_mark(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST subtype_mark_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENTIFIER:
		case DOT:
		{
			compound_name(_t);
			_t = _retTree;
			break;
		}
		case TIC:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t29 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp23_AST_in = _t;
			match(_t,TIC);
			_t = _t->getFirstChild();
			compound_name(_t);
			_t = _retTree;
			attribute_id(_t);
			_t = _retTree;
			_t = __t29;
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

void AdaTreeParserSuper::attribute_id(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST attribute_id_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case RANGE:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp24_AST_in = _t;
			match(_t,RANGE);
			_t = _t->getNextSibling();
			break;
		}
		case DIGITS:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp25_AST_in = _t;
			match(_t,DIGITS);
			_t = _t->getNextSibling();
			break;
		}
		case DELTA:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp26_AST_in = _t;
			match(_t,DELTA);
			_t = _t->getNextSibling();
			break;
		}
		case ACCESS:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp27_AST_in = _t;
			match(_t,ACCESS);
			_t = _t->getNextSibling();
			break;
		}
		case IDENTIFIER:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp28_AST_in = _t;
			match(_t,IDENTIFIER);
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

void AdaTreeParserSuper::modifiers(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST modifiers_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t39 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp29_AST_in = _t;
		match(_t,MODIFIERS);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case ABSTRACT:
			{
				ANTLR_USE_NAMESPACE(antlr)RefAST tmp30_AST_in = _t;
				match(_t,ABSTRACT);
				_t = _t->getNextSibling();
				break;
			}
			case ACCESS:
			{
				ANTLR_USE_NAMESPACE(antlr)RefAST tmp31_AST_in = _t;
				match(_t,ACCESS);
				_t = _t->getNextSibling();
				break;
			}
			case ALIASED:
			{
				ANTLR_USE_NAMESPACE(antlr)RefAST tmp32_AST_in = _t;
				match(_t,ALIASED);
				_t = _t->getNextSibling();
				break;
			}
			case ALL:
			{
				ANTLR_USE_NAMESPACE(antlr)RefAST tmp33_AST_in = _t;
				match(_t,ALL);
				_t = _t->getNextSibling();
				break;
			}
			case CONSTANT:
			{
				ANTLR_USE_NAMESPACE(antlr)RefAST tmp34_AST_in = _t;
				match(_t,CONSTANT);
				_t = _t->getNextSibling();
				break;
			}
			case IN:
			{
				ANTLR_USE_NAMESPACE(antlr)RefAST tmp35_AST_in = _t;
				match(_t,IN);
				_t = _t->getNextSibling();
				break;
			}
			case LIMITED:
			{
				ANTLR_USE_NAMESPACE(antlr)RefAST tmp36_AST_in = _t;
				match(_t,LIMITED);
				_t = _t->getNextSibling();
				break;
			}
			case OUT:
			{
				ANTLR_USE_NAMESPACE(antlr)RefAST tmp37_AST_in = _t;
				match(_t,OUT);
				_t = _t->getNextSibling();
				break;
			}
			case PRIVATE:
			{
				ANTLR_USE_NAMESPACE(antlr)RefAST tmp38_AST_in = _t;
				match(_t,PRIVATE);
				_t = _t->getNextSibling();
				break;
			}
			case PROTECTED:
			{
				ANTLR_USE_NAMESPACE(antlr)RefAST tmp39_AST_in = _t;
				match(_t,PROTECTED);
				_t = _t->getNextSibling();
				break;
			}
			case REVERSE:
			{
				ANTLR_USE_NAMESPACE(antlr)RefAST tmp40_AST_in = _t;
				match(_t,REVERSE);
				_t = _t->getNextSibling();
				break;
			}
			case TAGGED:
			{
				ANTLR_USE_NAMESPACE(antlr)RefAST tmp41_AST_in = _t;
				match(_t,TAGGED);
				_t = _t->getNextSibling();
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
		_t = __t39;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::subprog_decl_or_rename_or_inst_or_body(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST subprog_decl_or_rename_or_inst_or_body_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case ABSTRACT_FUNCTION_DECLARATION:
		case ABSTRACT_PROCEDURE_DECLARATION:
		case FUNCTION_BODY_STUB:
		case FUNCTION_DECLARATION:
		case FUNCTION_RENAMING_DECLARATION:
		case GENERIC_FUNCTION_INSTANTIATION:
		case GENERIC_PROCEDURE_INSTANTIATION:
		case PROCEDURE_BODY_STUB:
		case PROCEDURE_DECLARATION:
		case PROCEDURE_RENAMING_DECLARATION:
		{
			subprog_decl(_t);
			_t = _retTree;
			break;
		}
		case PROCEDURE_BODY:
		{
			procedure_body(_t);
			_t = _retTree;
			break;
		}
		case FUNCTION_BODY:
		{
			function_body(_t);
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

void AdaTreeParserSuper::def_id(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST def_id_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		compound_name(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::pkg_body_part(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST pkg_body_part_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		declarative_part(_t);
		_t = _retTree;
		block_body_opt(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::generic_inst(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST generic_inst_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		compound_name(_t);
		_t = _retTree;
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case VALUES:
		{
			value_s(_t);
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
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::pkg_spec_part(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST pkg_spec_part_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		basic_declarative_items_opt(_t);
		_t = _retTree;
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case BASIC_DECLARATIVE_ITEMS_OPT:
		{
			basic_declarative_items_opt(_t);
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
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::renames(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST renames_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case CHARACTER_STRING:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp42_AST_in = _t;
			match(_t,CHARACTER_STRING);
			_t = _t->getNextSibling();
			break;
		}
		case OPERATOR_SYMBOL:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp43_AST_in = _t;
			match(_t,OPERATOR_SYMBOL);
			_t = _t->getNextSibling();
			break;
		}
		case IDENTIFIER:
		case DOT:
		case TIC:
		case INDEXED_COMPONENT:
		{
			name(_t);
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

void AdaTreeParserSuper::generic_decl(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST generic_decl_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case GENERIC_PACKAGE_RENAMING:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t266 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp44_AST_in = _t;
			match(_t,GENERIC_PACKAGE_RENAMING);
			_t = _t->getFirstChild();
			generic_formal_part_opt(_t);
			_t = _retTree;
			def_id(_t);
			_t = _retTree;
			renames(_t);
			_t = _retTree;
			_t = __t266;
			_t = _t->getNextSibling();
			break;
		}
		case GENERIC_PACKAGE_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t267 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp45_AST_in = _t;
			match(_t,GENERIC_PACKAGE_DECLARATION);
			_t = _t->getFirstChild();
			generic_formal_part_opt(_t);
			_t = _retTree;
			def_id(_t);
			_t = _retTree;
			pkg_spec_part(_t);
			_t = _retTree;
			_t = __t267;
			_t = _t->getNextSibling();
			break;
		}
		case GENERIC_PROCEDURE_RENAMING:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t268 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp46_AST_in = _t;
			match(_t,GENERIC_PROCEDURE_RENAMING);
			_t = _t->getFirstChild();
			generic_formal_part_opt(_t);
			_t = _retTree;
			def_id(_t);
			_t = _retTree;
			formal_part_opt(_t);
			_t = _retTree;
			renames(_t);
			_t = _retTree;
			_t = __t268;
			_t = _t->getNextSibling();
			break;
		}
		case GENERIC_PROCEDURE_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t269 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp47_AST_in = _t;
			match(_t,GENERIC_PROCEDURE_DECLARATION);
			_t = _t->getFirstChild();
			generic_formal_part_opt(_t);
			_t = _retTree;
			def_id(_t);
			_t = _retTree;
			formal_part_opt(_t);
			_t = _retTree;
			_t = __t269;
			_t = _t->getNextSibling();
			break;
		}
		case GENERIC_FUNCTION_RENAMING:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t270 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp48_AST_in = _t;
			match(_t,GENERIC_FUNCTION_RENAMING);
			_t = _t->getFirstChild();
			generic_formal_part_opt(_t);
			_t = _retTree;
			def_designator(_t);
			_t = _retTree;
			function_tail(_t);
			_t = _retTree;
			renames(_t);
			_t = _retTree;
			_t = __t270;
			_t = _t->getNextSibling();
			break;
		}
		case GENERIC_FUNCTION_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t271 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp49_AST_in = _t;
			match(_t,GENERIC_FUNCTION_DECLARATION);
			_t = _t->getFirstChild();
			generic_formal_part_opt(_t);
			_t = _retTree;
			def_id(_t);
			_t = _retTree;
			function_tail(_t);
			_t = _retTree;
			_t = __t271;
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

void AdaTreeParserSuper::subprog_decl(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST subprog_decl_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case GENERIC_PROCEDURE_INSTANTIATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t43 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp50_AST_in = _t;
			match(_t,GENERIC_PROCEDURE_INSTANTIATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			generic_inst(_t);
			_t = _retTree;
			_t = __t43;
			_t = _t->getNextSibling();
			break;
		}
		case PROCEDURE_RENAMING_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t44 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp51_AST_in = _t;
			match(_t,PROCEDURE_RENAMING_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			formal_part_opt(_t);
			_t = _retTree;
			renames(_t);
			_t = _retTree;
			_t = __t44;
			_t = _t->getNextSibling();
			break;
		}
		case PROCEDURE_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t45 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp52_AST_in = _t;
			match(_t,PROCEDURE_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			formal_part_opt(_t);
			_t = _retTree;
			_t = __t45;
			_t = _t->getNextSibling();
			break;
		}
		case PROCEDURE_BODY_STUB:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t46 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp53_AST_in = _t;
			match(_t,PROCEDURE_BODY_STUB);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			formal_part_opt(_t);
			_t = _retTree;
			_t = __t46;
			_t = _t->getNextSibling();
			break;
		}
		case ABSTRACT_PROCEDURE_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t47 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp54_AST_in = _t;
			match(_t,ABSTRACT_PROCEDURE_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			formal_part_opt(_t);
			_t = _retTree;
			_t = __t47;
			_t = _t->getNextSibling();
			break;
		}
		case GENERIC_FUNCTION_INSTANTIATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t48 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp55_AST_in = _t;
			match(_t,GENERIC_FUNCTION_INSTANTIATION);
			_t = _t->getFirstChild();
			def_designator(_t);
			_t = _retTree;
			generic_inst(_t);
			_t = _retTree;
			_t = __t48;
			_t = _t->getNextSibling();
			break;
		}
		case FUNCTION_RENAMING_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t49 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp56_AST_in = _t;
			match(_t,FUNCTION_RENAMING_DECLARATION);
			_t = _t->getFirstChild();
			def_designator(_t);
			_t = _retTree;
			function_tail(_t);
			_t = _retTree;
			renames(_t);
			_t = _retTree;
			_t = __t49;
			_t = _t->getNextSibling();
			break;
		}
		case FUNCTION_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t50 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp57_AST_in = _t;
			match(_t,FUNCTION_DECLARATION);
			_t = _t->getFirstChild();
			def_designator(_t);
			_t = _retTree;
			function_tail(_t);
			_t = _retTree;
			_t = __t50;
			_t = _t->getNextSibling();
			break;
		}
		case FUNCTION_BODY_STUB:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t51 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp58_AST_in = _t;
			match(_t,FUNCTION_BODY_STUB);
			_t = _t->getFirstChild();
			def_designator(_t);
			_t = _retTree;
			function_tail(_t);
			_t = _retTree;
			_t = __t51;
			_t = _t->getNextSibling();
			break;
		}
		case ABSTRACT_FUNCTION_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t52 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp59_AST_in = _t;
			match(_t,ABSTRACT_FUNCTION_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			function_tail(_t);
			_t = _retTree;
			_t = __t52;
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

void AdaTreeParserSuper::formal_part_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST formal_part_opt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t77 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp60_AST_in = _t;
		match(_t,FORMAL_PART_OPT);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			if ((_t->getType() == PARAMETER_SPECIFICATION)) {
				parameter_specification(_t);
				_t = _retTree;
			}
			else {
				goto _loop79;
			}
			
		}
		_loop79:;
		} // ( ... )*
		_t = __t77;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::def_designator(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST def_designator_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENTIFIER:
		case DOT:
		{
			compound_name(_t);
			_t = _retTree;
			break;
		}
		case OPERATOR_SYMBOL:
		{
			definable_operator_symbol(_t);
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

void AdaTreeParserSuper::function_tail(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST function_tail_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		formal_part_opt(_t);
		_t = _retTree;
		subtype_mark(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::value_s(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST value_s_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t455 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp61_AST_in = _t;
		match(_t,VALUES);
		_t = _t->getFirstChild();
		{ // ( ... )+
		int _cnt457=0;
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			if ((_tokenSet_1.member(_t->getType()))) {
				value(_t);
				_t = _retTree;
			}
			else {
				if ( _cnt457>=1 ) { goto _loop457; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);}
			}
			
			_cnt457++;
		}
		_loop457:;
		}  // ( ... )+
		_t = __t455;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::value(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST value_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case OTHERS:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t57 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp62_AST_in = _t;
			match(_t,OTHERS);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			_t = __t57;
			_t = _t->getNextSibling();
			break;
		}
		case RIGHT_SHAFT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t58 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp63_AST_in = _t;
			match(_t,RIGHT_SHAFT);
			_t = _t->getFirstChild();
			ranged_expr_s(_t);
			_t = _retTree;
			expression(_t);
			_t = _retTree;
			_t = __t58;
			_t = _t->getNextSibling();
			break;
		}
		case IDENTIFIER:
		case DOT:
		case TIC:
		case RANGE:
		case PIPE:
		case DOT_DOT:
		case IN:
		case CHARACTER_LITERAL:
		case CHAR_STRING:
		case NuLL:
		case MOD:
		case OR:
		case AND:
		case XOR:
		case NOT:
		case EQ:
		case NE:
		case LT_:
		case LE:
		case GT:
		case GE:
		case PLUS:
		case MINUS:
		case CONCAT:
		case STAR:
		case DIV:
		case REM:
		case ABS:
		case EXPON:
		case NUMERIC_LIT:
		case ALLOCATOR:
		case INDEXED_COMPONENT:
		case OPERATOR_SYMBOL:
		case AND_THEN:
		case NOT_IN:
		case OR_ELSE:
		case PARENTHESIZED_PRIMARY:
		case UNARY_MINUS:
		case UNARY_PLUS:
		{
			ranged_expr_s(_t);
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

void AdaTreeParserSuper::ranged_expr_s(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST ranged_expr_s_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case PIPE:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t60 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp64_AST_in = _t;
			match(_t,PIPE);
			_t = _t->getFirstChild();
			ranged_expr_s(_t);
			_t = _retTree;
			ranged_expr(_t);
			_t = _retTree;
			_t = __t60;
			_t = _t->getNextSibling();
			break;
		}
		case IDENTIFIER:
		case DOT:
		case TIC:
		case RANGE:
		case DOT_DOT:
		case IN:
		case CHARACTER_LITERAL:
		case CHAR_STRING:
		case NuLL:
		case MOD:
		case OR:
		case AND:
		case XOR:
		case NOT:
		case EQ:
		case NE:
		case LT_:
		case LE:
		case GT:
		case GE:
		case PLUS:
		case MINUS:
		case CONCAT:
		case STAR:
		case DIV:
		case REM:
		case ABS:
		case EXPON:
		case NUMERIC_LIT:
		case ALLOCATOR:
		case INDEXED_COMPONENT:
		case OPERATOR_SYMBOL:
		case AND_THEN:
		case NOT_IN:
		case OR_ELSE:
		case PARENTHESIZED_PRIMARY:
		case UNARY_MINUS:
		case UNARY_PLUS:
		{
			ranged_expr(_t);
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

void AdaTreeParserSuper::ranged_expr(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST ranged_expr_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case DOT_DOT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t62 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp65_AST_in = _t;
			match(_t,DOT_DOT);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			simple_expression(_t);
			_t = _retTree;
			_t = __t62;
			_t = _t->getNextSibling();
			break;
		}
		case RANGE:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t63 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp66_AST_in = _t;
			match(_t,RANGE);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			range(_t);
			_t = _retTree;
			_t = __t63;
			_t = _t->getNextSibling();
			break;
		}
		case IDENTIFIER:
		case DOT:
		case TIC:
		case IN:
		case CHARACTER_LITERAL:
		case CHAR_STRING:
		case NuLL:
		case MOD:
		case OR:
		case AND:
		case XOR:
		case NOT:
		case EQ:
		case NE:
		case LT_:
		case LE:
		case GT:
		case GE:
		case PLUS:
		case MINUS:
		case CONCAT:
		case STAR:
		case DIV:
		case REM:
		case ABS:
		case EXPON:
		case NUMERIC_LIT:
		case ALLOCATOR:
		case INDEXED_COMPONENT:
		case OPERATOR_SYMBOL:
		case AND_THEN:
		case NOT_IN:
		case OR_ELSE:
		case PARENTHESIZED_PRIMARY:
		case UNARY_MINUS:
		case UNARY_PLUS:
		{
			expression(_t);
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

void AdaTreeParserSuper::simple_expression(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST simple_expression_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case PLUS:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t475 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp67_AST_in = _t;
			match(_t,PLUS);
			_t = _t->getFirstChild();
			simple_expression(_t);
			_t = _retTree;
			signed_term(_t);
			_t = _retTree;
			_t = __t475;
			_t = _t->getNextSibling();
			break;
		}
		case MINUS:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t476 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp68_AST_in = _t;
			match(_t,MINUS);
			_t = _t->getFirstChild();
			simple_expression(_t);
			_t = _retTree;
			signed_term(_t);
			_t = _retTree;
			_t = __t476;
			_t = _t->getNextSibling();
			break;
		}
		case CONCAT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t477 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp69_AST_in = _t;
			match(_t,CONCAT);
			_t = _t->getFirstChild();
			simple_expression(_t);
			_t = _retTree;
			signed_term(_t);
			_t = _retTree;
			_t = __t477;
			_t = _t->getNextSibling();
			break;
		}
		case IDENTIFIER:
		case DOT:
		case TIC:
		case CHARACTER_LITERAL:
		case CHAR_STRING:
		case NuLL:
		case MOD:
		case NOT:
		case STAR:
		case DIV:
		case REM:
		case ABS:
		case EXPON:
		case NUMERIC_LIT:
		case ALLOCATOR:
		case INDEXED_COMPONENT:
		case OPERATOR_SYMBOL:
		case PARENTHESIZED_PRIMARY:
		case UNARY_MINUS:
		case UNARY_PLUS:
		{
			signed_term(_t);
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

void AdaTreeParserSuper::range(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST range_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case DOT_DOT:
		{
			range_dots(_t);
			_t = _retTree;
			break;
		}
		case RANGE_ATTRIBUTE_REFERENCE:
		{
			range_attrib_ref(_t);
			_t = _retTree;
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
		}
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

void AdaTreeParserSuper::range_constraint(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST range_constraint_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		range(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::range_dots(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST range_dots_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t68 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp70_AST_in = _t;
		match(_t,DOT_DOT);
		_t = _t->getFirstChild();
		simple_expression(_t);
		_t = _retTree;
		simple_expression(_t);
		_t = _retTree;
		_t = __t68;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::range_attrib_ref(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST range_attrib_ref_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t70 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp71_AST_in = _t;
		match(_t,RANGE_ATTRIBUTE_REFERENCE);
		_t = _t->getFirstChild();
		prefix(_t);
		_t = _retTree;
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENTIFIER:
		case DOT:
		case TIC:
		case IN:
		case CHARACTER_LITERAL:
		case CHAR_STRING:
		case NuLL:
		case MOD:
		case OR:
		case AND:
		case XOR:
		case NOT:
		case EQ:
		case NE:
		case LT_:
		case LE:
		case GT:
		case GE:
		case PLUS:
		case MINUS:
		case CONCAT:
		case STAR:
		case DIV:
		case REM:
		case ABS:
		case EXPON:
		case NUMERIC_LIT:
		case ALLOCATOR:
		case INDEXED_COMPONENT:
		case OPERATOR_SYMBOL:
		case AND_THEN:
		case NOT_IN:
		case OR_ELSE:
		case PARENTHESIZED_PRIMARY:
		case UNARY_MINUS:
		case UNARY_PLUS:
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

void AdaTreeParserSuper::prefix(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST prefix_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENTIFIER:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp72_AST_in = _t;
			match(_t,IDENTIFIER);
			_t = _t->getNextSibling();
			break;
		}
		case DOT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t73 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp73_AST_in = _t;
			match(_t,DOT);
			_t = _t->getFirstChild();
			prefix(_t);
			_t = _retTree;
			{
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case ALL:
			{
				ANTLR_USE_NAMESPACE(antlr)RefAST tmp74_AST_in = _t;
				match(_t,ALL);
				_t = _t->getNextSibling();
				break;
			}
			case IDENTIFIER:
			{
				ANTLR_USE_NAMESPACE(antlr)RefAST tmp75_AST_in = _t;
				match(_t,IDENTIFIER);
				_t = _t->getNextSibling();
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
			}
			}
			}
			_t = __t73;
			_t = _t->getNextSibling();
			break;
		}
		case INDEXED_COMPONENT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t75 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp76_AST_in = _t;
			match(_t,INDEXED_COMPONENT);
			_t = _t->getFirstChild();
			prefix(_t);
			_t = _retTree;
			value_s(_t);
			_t = _retTree;
			_t = __t75;
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

void AdaTreeParserSuper::parameter_specification(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST parameter_specification_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t81 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp77_AST_in = _t;
		match(_t,PARAMETER_SPECIFICATION);
		_t = _t->getFirstChild();
		defining_identifier_list(_t);
		_t = _retTree;
		modifiers(_t);
		_t = _retTree;
		subtype_mark(_t);
		_t = _retTree;
		init_opt(_t);
		_t = _retTree;
		_t = __t81;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::defining_identifier_list(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST defining_identifier_list_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t83 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp78_AST_in = _t;
		match(_t,DEFINING_IDENTIFIER_LIST);
		_t = _t->getFirstChild();
		{ // ( ... )+
		int _cnt85=0;
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			if ((_t->getType() == IDENTIFIER)) {
				ANTLR_USE_NAMESPACE(antlr)RefAST tmp79_AST_in = _t;
				match(_t,IDENTIFIER);
				_t = _t->getNextSibling();
			}
			else {
				if ( _cnt85>=1 ) { goto _loop85; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);}
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
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::init_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST init_opt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t127 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp80_AST_in = _t;
		match(_t,INIT_OPT);
		_t = _t->getFirstChild();
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENTIFIER:
		case DOT:
		case TIC:
		case IN:
		case CHARACTER_LITERAL:
		case CHAR_STRING:
		case NuLL:
		case MOD:
		case OR:
		case AND:
		case XOR:
		case NOT:
		case EQ:
		case NE:
		case LT_:
		case LE:
		case GT:
		case GE:
		case PLUS:
		case MINUS:
		case CONCAT:
		case STAR:
		case DIV:
		case REM:
		case ABS:
		case EXPON:
		case NUMERIC_LIT:
		case ALLOCATOR:
		case INDEXED_COMPONENT:
		case OPERATOR_SYMBOL:
		case AND_THEN:
		case NOT_IN:
		case OR_ELSE:
		case PARENTHESIZED_PRIMARY:
		case UNARY_MINUS:
		case UNARY_PLUS:
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
		_t = __t127;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::name(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST name_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENTIFIER:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp81_AST_in = _t;
			match(_t,IDENTIFIER);
			_t = _t->getNextSibling();
			break;
		}
		case DOT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t88 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp82_AST_in = _t;
			match(_t,DOT);
			_t = _t->getFirstChild();
			name(_t);
			_t = _retTree;
			{
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case ALL:
			{
				ANTLR_USE_NAMESPACE(antlr)RefAST tmp83_AST_in = _t;
				match(_t,ALL);
				_t = _t->getNextSibling();
				break;
			}
			case IDENTIFIER:
			{
				ANTLR_USE_NAMESPACE(antlr)RefAST tmp84_AST_in = _t;
				match(_t,IDENTIFIER);
				_t = _t->getNextSibling();
				break;
			}
			case CHARACTER_LITERAL:
			{
				ANTLR_USE_NAMESPACE(antlr)RefAST tmp85_AST_in = _t;
				match(_t,CHARACTER_LITERAL);
				_t = _t->getNextSibling();
				break;
			}
			case OPERATOR_SYMBOL:
			{
				ANTLR_USE_NAMESPACE(antlr)RefAST tmp86_AST_in = _t;
				match(_t,OPERATOR_SYMBOL);
				_t = _t->getNextSibling();
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
			}
			}
			}
			_t = __t88;
			_t = _t->getNextSibling();
			break;
		}
		case INDEXED_COMPONENT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t90 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp87_AST_in = _t;
			match(_t,INDEXED_COMPONENT);
			_t = _t->getFirstChild();
			name(_t);
			_t = _retTree;
			value_s(_t);
			_t = _retTree;
			_t = __t90;
			_t = _t->getNextSibling();
			break;
		}
		case TIC:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t91 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp88_AST_in = _t;
			match(_t,TIC);
			_t = _t->getFirstChild();
			name(_t);
			_t = _retTree;
			attribute_id(_t);
			_t = _retTree;
			_t = __t91;
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

void AdaTreeParserSuper::definable_operator_symbol(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST definable_operator_symbol_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp89_AST_in = _t;
		match(_t,OPERATOR_SYMBOL);
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::parenthesized_primary(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST parenthesized_primary_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t94 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp90_AST_in = _t;
		match(_t,PARENTHESIZED_PRIMARY);
		_t = _t->getFirstChild();
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case NuLL:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp91_AST_in = _t;
			match(_t,NuLL);
			_t = _t->getNextSibling();
			break;
		}
		case VALUES:
		{
			value_s(_t);
			_t = _retTree;
			extension_opt(_t);
			_t = _retTree;
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
		}
		}
		}
		_t = __t94;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::extension_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST extension_opt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t97 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp92_AST_in = _t;
		match(_t,EXTENSION_OPT);
		_t = _t->getFirstChild();
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case NuLL:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp93_AST_in = _t;
			match(_t,NuLL);
			_t = _t->getNextSibling();
			break;
		}
		case VALUES:
		{
			value_s(_t);
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
		_t = __t97;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::spec_decl_part(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST spec_decl_part_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case GENERIC_PACKAGE_INSTANTIATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t102 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp94_AST_in = _t;
			match(_t,GENERIC_PACKAGE_INSTANTIATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			generic_inst(_t);
			_t = _retTree;
			_t = __t102;
			_t = _t->getNextSibling();
			break;
		}
		case PACKAGE_SPECIFICATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t103 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp95_AST_in = _t;
			match(_t,PACKAGE_SPECIFICATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			pkg_spec_part(_t);
			_t = _retTree;
			_t = __t103;
			_t = _t->getNextSibling();
			break;
		}
		case PACKAGE_RENAMING_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t104 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp96_AST_in = _t;
			match(_t,PACKAGE_RENAMING_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			renames(_t);
			_t = _retTree;
			_t = __t104;
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

void AdaTreeParserSuper::basic_declarative_items_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST basic_declarative_items_opt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t108 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp97_AST_in = _t;
		match(_t,BASIC_DECLARATIVE_ITEMS_OPT);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			if ((_tokenSet_2.member(_t->getType()))) {
				basic_decl_item(_t);
				_t = _retTree;
			}
			else {
				goto _loop110;
			}
			
		}
		_loop110:;
		} // ( ... )*
		_t = __t108;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::basic_decl_item(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST basic_decl_item_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case PACKAGE_RENAMING_DECLARATION:
		case PACKAGE_SPECIFICATION:
		case GENERIC_PACKAGE_INSTANTIATION:
		{
			spec_decl_part(_t);
			_t = _retTree;
			break;
		}
		case SINGLE_TASK_DECLARATION:
		case TASK_TYPE_DECLARATION:
		{
			task_type_or_single_decl(_t);
			_t = _retTree;
			break;
		}
		case PROTECTED_TYPE_DECLARATION:
		case SINGLE_PROTECTED_DECLARATION:
		{
			prot_type_or_single_decl(_t);
			_t = _retTree;
			break;
		}
		case ABSTRACT_FUNCTION_DECLARATION:
		case ABSTRACT_PROCEDURE_DECLARATION:
		case FUNCTION_BODY_STUB:
		case FUNCTION_DECLARATION:
		case FUNCTION_RENAMING_DECLARATION:
		case GENERIC_FUNCTION_INSTANTIATION:
		case GENERIC_PROCEDURE_INSTANTIATION:
		case PROCEDURE_BODY_STUB:
		case PROCEDURE_DECLARATION:
		case PROCEDURE_RENAMING_DECLARATION:
		{
			subprog_decl(_t);
			_t = _retTree;
			break;
		}
		case ATTRIBUTE_DEFINITION_CLAUSE:
		case AT_CLAUSE:
		case ENUMERATION_REPESENTATION_CLAUSE:
		case EXCEPTION_DECLARATION:
		case EXCEPTION_RENAMING_DECLARATION:
		case GENERIC_PACKAGE_DECLARATION:
		case INCOMPLETE_TYPE_DECLARATION:
		case NUMBER_DECLARATION:
		case OBJECT_DECLARATION:
		case OBJECT_RENAMING_DECLARATION:
		case PRIVATE_EXTENSION_DECLARATION:
		case PRIVATE_TYPE_DECLARATION:
		case RECORD_REPRESENTATION_CLAUSE:
		case SUBTYPE_DECLARATION:
		case USE_CLAUSE:
		case USE_TYPE_CLAUSE:
		case ACCESS_TO_FUNCTION_DECLARATION:
		case ACCESS_TO_OBJECT_DECLARATION:
		case ACCESS_TO_PROCEDURE_DECLARATION:
		case ARRAY_OBJECT_DECLARATION:
		case ARRAY_TYPE_DECLARATION:
		case DECIMAL_FIXED_POINT_DECLARATION:
		case DERIVED_RECORD_EXTENSION:
		case ENUMERATION_TYPE_DECLARATION:
		case FLOATING_POINT_DECLARATION:
		case GENERIC_FUNCTION_DECLARATION:
		case GENERIC_FUNCTION_RENAMING:
		case GENERIC_PACKAGE_RENAMING:
		case GENERIC_PROCEDURE_DECLARATION:
		case GENERIC_PROCEDURE_RENAMING:
		case MODULAR_TYPE_DECLARATION:
		case ORDINARY_DERIVED_TYPE_DECLARATION:
		case ORDINARY_FIXED_POINT_DECLARATION:
		case RECORD_TYPE_DECLARATION:
		case SIGNED_INTEGER_TYPE_DECLARATION:
		{
			decl_common(_t);
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

void AdaTreeParserSuper::task_type_or_single_decl(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST task_type_or_single_decl_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case TASK_TYPE_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t113 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp98_AST_in = _t;
			match(_t,TASK_TYPE_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			discrim_part_opt(_t);
			_t = _retTree;
			task_definition_opt(_t);
			_t = _retTree;
			_t = __t113;
			_t = _t->getNextSibling();
			break;
		}
		case SINGLE_TASK_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t114 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp99_AST_in = _t;
			match(_t,SINGLE_TASK_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			task_definition_opt(_t);
			_t = _retTree;
			_t = __t114;
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

void AdaTreeParserSuper::prot_type_or_single_decl(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST prot_type_or_single_decl_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case PROTECTED_TYPE_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t155 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp100_AST_in = _t;
			match(_t,PROTECTED_TYPE_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			discrim_part_opt(_t);
			_t = _retTree;
			protected_definition(_t);
			_t = _retTree;
			_t = __t155;
			_t = _t->getNextSibling();
			break;
		}
		case SINGLE_PROTECTED_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t156 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp101_AST_in = _t;
			match(_t,SINGLE_PROTECTED_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			protected_definition(_t);
			_t = _retTree;
			_t = __t156;
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

void AdaTreeParserSuper::decl_common(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST decl_common_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case ENUMERATION_TYPE_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t173 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp102_AST_in = _t;
			match(_t,ENUMERATION_TYPE_DECLARATION);
			_t = _t->getFirstChild();
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp103_AST_in = _t;
			match(_t,IDENTIFIER);
			_t = _t->getNextSibling();
			enum_id_s(_t);
			_t = _retTree;
			_t = __t173;
			_t = _t->getNextSibling();
			break;
		}
		case SIGNED_INTEGER_TYPE_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t174 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp104_AST_in = _t;
			match(_t,SIGNED_INTEGER_TYPE_DECLARATION);
			_t = _t->getFirstChild();
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp105_AST_in = _t;
			match(_t,IDENTIFIER);
			_t = _t->getNextSibling();
			range(_t);
			_t = _retTree;
			_t = __t174;
			_t = _t->getNextSibling();
			break;
		}
		case MODULAR_TYPE_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t175 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp106_AST_in = _t;
			match(_t,MODULAR_TYPE_DECLARATION);
			_t = _t->getFirstChild();
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp107_AST_in = _t;
			match(_t,IDENTIFIER);
			_t = _t->getNextSibling();
			expression(_t);
			_t = _retTree;
			_t = __t175;
			_t = _t->getNextSibling();
			break;
		}
		case FLOATING_POINT_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t176 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp108_AST_in = _t;
			match(_t,FLOATING_POINT_DECLARATION);
			_t = _t->getFirstChild();
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp109_AST_in = _t;
			match(_t,IDENTIFIER);
			_t = _t->getNextSibling();
			expression(_t);
			_t = _retTree;
			range_constraint_opt(_t);
			_t = _retTree;
			_t = __t176;
			_t = _t->getNextSibling();
			break;
		}
		case ORDINARY_FIXED_POINT_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t177 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp110_AST_in = _t;
			match(_t,ORDINARY_FIXED_POINT_DECLARATION);
			_t = _t->getFirstChild();
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp111_AST_in = _t;
			match(_t,IDENTIFIER);
			_t = _t->getNextSibling();
			expression(_t);
			_t = _retTree;
			range(_t);
			_t = _retTree;
			_t = __t177;
			_t = _t->getNextSibling();
			break;
		}
		case DECIMAL_FIXED_POINT_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t178 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp112_AST_in = _t;
			match(_t,DECIMAL_FIXED_POINT_DECLARATION);
			_t = _t->getFirstChild();
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp113_AST_in = _t;
			match(_t,IDENTIFIER);
			_t = _t->getNextSibling();
			expression(_t);
			_t = _retTree;
			expression(_t);
			_t = _retTree;
			range_constraint_opt(_t);
			_t = _retTree;
			_t = __t178;
			_t = _t->getNextSibling();
			break;
		}
		case ARRAY_TYPE_DECLARATION:
		{
			array_type_declaration(_t);
			_t = _retTree;
			break;
		}
		case ACCESS_TO_FUNCTION_DECLARATION:
		case ACCESS_TO_OBJECT_DECLARATION:
		case ACCESS_TO_PROCEDURE_DECLARATION:
		{
			access_type_declaration(_t);
			_t = _retTree;
			break;
		}
		case INCOMPLETE_TYPE_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t179 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp114_AST_in = _t;
			match(_t,INCOMPLETE_TYPE_DECLARATION);
			_t = _t->getFirstChild();
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp115_AST_in = _t;
			match(_t,IDENTIFIER);
			_t = _t->getNextSibling();
			discrim_part_opt(_t);
			_t = _retTree;
			_t = __t179;
			_t = _t->getNextSibling();
			break;
		}
		case PRIVATE_EXTENSION_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t180 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp116_AST_in = _t;
			match(_t,PRIVATE_EXTENSION_DECLARATION);
			_t = _t->getFirstChild();
			id_and_discrim(_t);
			_t = _retTree;
			modifiers(_t);
			_t = _retTree;
			subtype_ind(_t);
			_t = _retTree;
			_t = __t180;
			_t = _t->getNextSibling();
			break;
		}
		case DERIVED_RECORD_EXTENSION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t181 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp117_AST_in = _t;
			match(_t,DERIVED_RECORD_EXTENSION);
			_t = _t->getFirstChild();
			id_and_discrim(_t);
			_t = _retTree;
			modifiers(_t);
			_t = _retTree;
			subtype_ind(_t);
			_t = _retTree;
			record_definition(_t);
			_t = _retTree;
			_t = __t181;
			_t = _t->getNextSibling();
			break;
		}
		case ORDINARY_DERIVED_TYPE_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t182 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp118_AST_in = _t;
			match(_t,ORDINARY_DERIVED_TYPE_DECLARATION);
			_t = _t->getFirstChild();
			id_and_discrim(_t);
			_t = _retTree;
			subtype_ind(_t);
			_t = _retTree;
			_t = __t182;
			_t = _t->getNextSibling();
			break;
		}
		case PRIVATE_TYPE_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t183 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp119_AST_in = _t;
			match(_t,PRIVATE_TYPE_DECLARATION);
			_t = _t->getFirstChild();
			id_and_discrim(_t);
			_t = _retTree;
			modifiers(_t);
			_t = _retTree;
			_t = __t183;
			_t = _t->getNextSibling();
			break;
		}
		case RECORD_TYPE_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t184 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp120_AST_in = _t;
			match(_t,RECORD_TYPE_DECLARATION);
			_t = _t->getFirstChild();
			id_and_discrim(_t);
			_t = _retTree;
			modifiers(_t);
			_t = _retTree;
			record_definition(_t);
			_t = _retTree;
			_t = __t184;
			_t = _t->getNextSibling();
			break;
		}
		case SUBTYPE_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t185 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp121_AST_in = _t;
			match(_t,SUBTYPE_DECLARATION);
			_t = _t->getFirstChild();
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp122_AST_in = _t;
			match(_t,IDENTIFIER);
			_t = _t->getNextSibling();
			subtype_ind(_t);
			_t = _retTree;
			_t = __t185;
			_t = _t->getNextSibling();
			break;
		}
		case GENERIC_PACKAGE_DECLARATION:
		case GENERIC_FUNCTION_DECLARATION:
		case GENERIC_FUNCTION_RENAMING:
		case GENERIC_PACKAGE_RENAMING:
		case GENERIC_PROCEDURE_DECLARATION:
		case GENERIC_PROCEDURE_RENAMING:
		{
			generic_decl(_t);
			_t = _retTree;
			break;
		}
		case USE_CLAUSE:
		case USE_TYPE_CLAUSE:
		{
			use_clause(_t);
			_t = _retTree;
			break;
		}
		case ATTRIBUTE_DEFINITION_CLAUSE:
		case AT_CLAUSE:
		case ENUMERATION_REPESENTATION_CLAUSE:
		case RECORD_REPRESENTATION_CLAUSE:
		{
			rep_spec(_t);
			_t = _retTree;
			break;
		}
		case EXCEPTION_RENAMING_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t186 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp123_AST_in = _t;
			match(_t,EXCEPTION_RENAMING_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			compound_name(_t);
			_t = _retTree;
			_t = __t186;
			_t = _t->getNextSibling();
			break;
		}
		case OBJECT_RENAMING_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t187 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp124_AST_in = _t;
			match(_t,OBJECT_RENAMING_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			subtype_mark(_t);
			_t = _retTree;
			name(_t);
			_t = _retTree;
			_t = __t187;
			_t = _t->getNextSibling();
			break;
		}
		case EXCEPTION_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t188 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp125_AST_in = _t;
			match(_t,EXCEPTION_DECLARATION);
			_t = _t->getFirstChild();
			defining_identifier_list(_t);
			_t = _retTree;
			_t = __t188;
			_t = _t->getNextSibling();
			break;
		}
		case NUMBER_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t189 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp126_AST_in = _t;
			match(_t,NUMBER_DECLARATION);
			_t = _t->getFirstChild();
			defining_identifier_list(_t);
			_t = _retTree;
			expression(_t);
			_t = _retTree;
			_t = __t189;
			_t = _t->getNextSibling();
			break;
		}
		case ARRAY_OBJECT_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t190 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp127_AST_in = _t;
			match(_t,ARRAY_OBJECT_DECLARATION);
			_t = _t->getFirstChild();
			defining_identifier_list(_t);
			_t = _retTree;
			modifiers(_t);
			_t = _retTree;
			array_type_definition(_t);
			_t = _retTree;
			init_opt(_t);
			_t = _retTree;
			_t = __t190;
			_t = _t->getNextSibling();
			break;
		}
		case OBJECT_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t191 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp128_AST_in = _t;
			match(_t,OBJECT_DECLARATION);
			_t = _t->getFirstChild();
			defining_identifier_list(_t);
			_t = _retTree;
			modifiers(_t);
			_t = _retTree;
			subtype_ind(_t);
			_t = _retTree;
			init_opt(_t);
			_t = _retTree;
			_t = __t191;
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

void AdaTreeParserSuper::discrim_part_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST discrim_part_opt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t118 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp129_AST_in = _t;
		match(_t,DISCRIM_PART_OPT);
		_t = _t->getFirstChild();
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case BOX:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp130_AST_in = _t;
			match(_t,BOX);
			_t = _t->getNextSibling();
			break;
		}
		case DISCRIMINANT_SPECIFICATIONS:
		{
			discriminant_specifications(_t);
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

void AdaTreeParserSuper::task_definition_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST task_definition_opt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case TASK_ITEMS_OPT:
		{
			task_items_opt(_t);
			_t = _retTree;
			private_task_items_opt(_t);
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
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::task_items_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST task_items_opt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t130 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp131_AST_in = _t;
		match(_t,TASK_ITEMS_OPT);
		_t = _t->getFirstChild();
		entrydecls_repspecs_opt(_t);
		_t = _retTree;
		_t = __t130;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::private_task_items_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST private_task_items_opt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t153 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp132_AST_in = _t;
		match(_t,PRIVATE_TASK_ITEMS_OPT);
		_t = _t->getFirstChild();
		entrydecls_repspecs_opt(_t);
		_t = _retTree;
		_t = __t153;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::discriminant_specifications(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST discriminant_specifications_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t121 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp133_AST_in = _t;
		match(_t,DISCRIMINANT_SPECIFICATIONS);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			if ((_t->getType() == DISCRIMINANT_SPECIFICATION)) {
				discriminant_specification(_t);
				_t = _retTree;
			}
			else {
				goto _loop123;
			}
			
		}
		_loop123:;
		} // ( ... )*
		_t = __t121;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::discriminant_specification(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST discriminant_specification_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t125 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp134_AST_in = _t;
		match(_t,DISCRIMINANT_SPECIFICATION);
		_t = _t->getFirstChild();
		defining_identifier_list(_t);
		_t = _retTree;
		modifiers(_t);
		_t = _retTree;
		subtype_mark(_t);
		_t = _retTree;
		init_opt(_t);
		_t = _retTree;
		_t = __t125;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::entrydecls_repspecs_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST entrydecls_repspecs_opt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		{ // ( ... )*
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case ENTRY_DECLARATION:
			{
				entry_declaration(_t);
				_t = _retTree;
				break;
			}
			case PRAGMA:
			{
				pragma(_t);
				_t = _retTree;
				break;
			}
			case ATTRIBUTE_DEFINITION_CLAUSE:
			case AT_CLAUSE:
			case ENUMERATION_REPESENTATION_CLAUSE:
			case RECORD_REPRESENTATION_CLAUSE:
			{
				rep_spec(_t);
				_t = _retTree;
				break;
			}
			default:
			{
				goto _loop133;
			}
			}
		}
		_loop133:;
		} // ( ... )*
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::entry_declaration(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST entry_declaration_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t135 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp135_AST_in = _t;
		match(_t,ENTRY_DECLARATION);
		_t = _t->getFirstChild();
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp136_AST_in = _t;
		match(_t,IDENTIFIER);
		_t = _t->getNextSibling();
		discrete_subtype_def_opt(_t);
		_t = _retTree;
		formal_part_opt(_t);
		_t = _retTree;
		_t = __t135;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::rep_spec(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST rep_spec_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case RECORD_REPRESENTATION_CLAUSE:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t141 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp137_AST_in = _t;
			match(_t,RECORD_REPRESENTATION_CLAUSE);
			_t = _t->getFirstChild();
			subtype_mark(_t);
			_t = _retTree;
			align_opt(_t);
			_t = _retTree;
			comp_loc_s(_t);
			_t = _retTree;
			_t = __t141;
			_t = _t->getNextSibling();
			break;
		}
		case AT_CLAUSE:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t142 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp138_AST_in = _t;
			match(_t,AT_CLAUSE);
			_t = _t->getFirstChild();
			subtype_mark(_t);
			_t = _retTree;
			expression(_t);
			_t = _retTree;
			_t = __t142;
			_t = _t->getNextSibling();
			break;
		}
		case ATTRIBUTE_DEFINITION_CLAUSE:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t143 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp139_AST_in = _t;
			match(_t,ATTRIBUTE_DEFINITION_CLAUSE);
			_t = _t->getFirstChild();
			subtype_mark(_t);
			_t = _retTree;
			expression(_t);
			_t = _retTree;
			_t = __t143;
			_t = _t->getNextSibling();
			break;
		}
		case ENUMERATION_REPESENTATION_CLAUSE:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t144 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp140_AST_in = _t;
			match(_t,ENUMERATION_REPESENTATION_CLAUSE);
			_t = _t->getFirstChild();
			local_enum_name(_t);
			_t = _retTree;
			enumeration_aggregate(_t);
			_t = _retTree;
			_t = __t144;
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

void AdaTreeParserSuper::discrete_subtype_def_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST discrete_subtype_def_opt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t137 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp141_AST_in = _t;
		match(_t,DISCRETE_SUBTYPE_DEF_OPT);
		_t = _t->getFirstChild();
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case DOT_DOT:
		case RANGE_ATTRIBUTE_REFERENCE:
		case SUBTYPE_INDICATION:
		{
			discrete_subtype_definition(_t);
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
		_t = __t137;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::discrete_subtype_definition(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST discrete_subtype_definition_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case DOT_DOT:
		case RANGE_ATTRIBUTE_REFERENCE:
		{
			range(_t);
			_t = _retTree;
			break;
		}
		case SUBTYPE_INDICATION:
		{
			subtype_ind(_t);
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

void AdaTreeParserSuper::subtype_ind(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST subtype_ind_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t210 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp142_AST_in = _t;
		match(_t,SUBTYPE_INDICATION);
		_t = _t->getFirstChild();
		subtype_mark(_t);
		_t = _retTree;
		constraint_opt(_t);
		_t = _retTree;
		_t = __t210;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::align_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST align_opt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t146 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp143_AST_in = _t;
		match(_t,MOD_CLAUSE_OPT);
		_t = _t->getFirstChild();
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENTIFIER:
		case DOT:
		case TIC:
		case IN:
		case CHARACTER_LITERAL:
		case CHAR_STRING:
		case NuLL:
		case MOD:
		case OR:
		case AND:
		case XOR:
		case NOT:
		case EQ:
		case NE:
		case LT_:
		case LE:
		case GT:
		case GE:
		case PLUS:
		case MINUS:
		case CONCAT:
		case STAR:
		case DIV:
		case REM:
		case ABS:
		case EXPON:
		case NUMERIC_LIT:
		case ALLOCATOR:
		case INDEXED_COMPONENT:
		case OPERATOR_SYMBOL:
		case AND_THEN:
		case NOT_IN:
		case OR_ELSE:
		case PARENTHESIZED_PRIMARY:
		case UNARY_MINUS:
		case UNARY_PLUS:
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
		_t = __t146;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::comp_loc_s(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST comp_loc_s_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t149 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp144_AST_in = _t;
		match(_t,COMPONENT_CLAUSES_OPT);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case PRAGMA:
			{
				pragma(_t);
				_t = _retTree;
				break;
			}
			case IDENTIFIER:
			case DOT:
			case TIC:
			{
				subtype_mark(_t);
				_t = _retTree;
				expression(_t);
				_t = _retTree;
				range(_t);
				_t = _retTree;
				break;
			}
			default:
			{
				goto _loop151;
			}
			}
		}
		_loop151:;
		} // ( ... )*
		_t = __t149;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::local_enum_name(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST local_enum_name_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp145_AST_in = _t;
		match(_t,IDENTIFIER);
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::enumeration_aggregate(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST enumeration_aggregate_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		{ // ( ... )*
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			if ((_tokenSet_1.member(_t->getType()))) {
				value(_t);
				_t = _retTree;
			}
			else {
				goto _loop264;
			}
			
		}
		_loop264:;
		} // ( ... )*
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::protected_definition(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST protected_definition_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		prot_op_decl_s(_t);
		_t = _retTree;
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case PROT_MEMBER_DECLARATIONS:
		{
			prot_member_decl_s(_t);
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
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::prot_op_decl_s(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST prot_op_decl_s_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t160 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp146_AST_in = _t;
		match(_t,PROT_OP_DECLARATIONS);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			if ((_tokenSet_3.member(_t->getType()))) {
				prot_op_decl(_t);
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
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::prot_member_decl_s(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST prot_member_decl_s_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t167 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp147_AST_in = _t;
		match(_t,PROT_MEMBER_DECLARATIONS);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case PRAGMA:
			case ATTRIBUTE_DEFINITION_CLAUSE:
			case AT_CLAUSE:
			case ENTRY_DECLARATION:
			case ENUMERATION_REPESENTATION_CLAUSE:
			case RECORD_REPRESENTATION_CLAUSE:
			case FUNCTION_DECLARATION:
			case PROCEDURE_DECLARATION:
			{
				prot_op_decl(_t);
				_t = _retTree;
				break;
			}
			case COMPONENT_DECLARATION:
			{
				comp_decl(_t);
				_t = _retTree;
				break;
			}
			default:
			{
				goto _loop169;
			}
			}
		}
		_loop169:;
		} // ( ... )*
		_t = __t167;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::prot_op_decl(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST prot_op_decl_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case ENTRY_DECLARATION:
		{
			entry_declaration(_t);
			_t = _retTree;
			break;
		}
		case PROCEDURE_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t164 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp148_AST_in = _t;
			match(_t,PROCEDURE_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			formal_part_opt(_t);
			_t = _retTree;
			_t = __t164;
			_t = _t->getNextSibling();
			break;
		}
		case FUNCTION_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t165 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp149_AST_in = _t;
			match(_t,FUNCTION_DECLARATION);
			_t = _t->getFirstChild();
			def_designator(_t);
			_t = _retTree;
			function_tail(_t);
			_t = _retTree;
			_t = __t165;
			_t = _t->getNextSibling();
			break;
		}
		case ATTRIBUTE_DEFINITION_CLAUSE:
		case AT_CLAUSE:
		case ENUMERATION_REPESENTATION_CLAUSE:
		case RECORD_REPRESENTATION_CLAUSE:
		{
			rep_spec(_t);
			_t = _retTree;
			break;
		}
		case PRAGMA:
		{
			pragma(_t);
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

void AdaTreeParserSuper::comp_decl(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST comp_decl_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t171 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp150_AST_in = _t;
		match(_t,COMPONENT_DECLARATION);
		_t = _t->getFirstChild();
		defining_identifier_list(_t);
		_t = _retTree;
		component_subtype_def(_t);
		_t = _retTree;
		init_opt(_t);
		_t = _retTree;
		_t = __t171;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::component_subtype_def(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST component_subtype_def_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		modifiers(_t);
		_t = _retTree;
		subtype_ind(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::enum_id_s(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST enum_id_s_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		{ // ( ... )+
		int _cnt195=0;
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			if ((_t->getType() == IDENTIFIER || _t->getType() == CHARACTER_LITERAL)) {
				enumeration_literal_specification(_t);
				_t = _retTree;
			}
			else {
				if ( _cnt195>=1 ) { goto _loop195; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);}
			}
			
			_cnt195++;
		}
		_loop195:;
		}  // ( ... )+
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::range_constraint_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST range_constraint_opt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case DOT_DOT:
		case RANGE_ATTRIBUTE_REFERENCE:
		{
			range_constraint(_t);
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
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::array_type_declaration(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST array_type_declaration_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t201 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp151_AST_in = _t;
		match(_t,ARRAY_TYPE_DECLARATION);
		_t = _t->getFirstChild();
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp152_AST_in = _t;
		match(_t,IDENTIFIER);
		_t = _t->getNextSibling();
		array_type_definition(_t);
		_t = _retTree;
		_t = __t201;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::access_type_declaration(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST access_type_declaration_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case ACCESS_TO_PROCEDURE_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t234 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp153_AST_in = _t;
			match(_t,ACCESS_TO_PROCEDURE_DECLARATION);
			_t = _t->getFirstChild();
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp154_AST_in = _t;
			match(_t,IDENTIFIER);
			_t = _t->getNextSibling();
			modifiers(_t);
			_t = _retTree;
			formal_part_opt(_t);
			_t = _retTree;
			_t = __t234;
			_t = _t->getNextSibling();
			break;
		}
		case ACCESS_TO_FUNCTION_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t235 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp155_AST_in = _t;
			match(_t,ACCESS_TO_FUNCTION_DECLARATION);
			_t = _t->getFirstChild();
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp156_AST_in = _t;
			match(_t,IDENTIFIER);
			_t = _t->getNextSibling();
			modifiers(_t);
			_t = _retTree;
			function_tail(_t);
			_t = _retTree;
			_t = __t235;
			_t = _t->getNextSibling();
			break;
		}
		case ACCESS_TO_OBJECT_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t236 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp157_AST_in = _t;
			match(_t,ACCESS_TO_OBJECT_DECLARATION);
			_t = _t->getFirstChild();
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp158_AST_in = _t;
			match(_t,IDENTIFIER);
			_t = _t->getNextSibling();
			modifiers(_t);
			_t = _retTree;
			subtype_ind(_t);
			_t = _retTree;
			_t = __t236;
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

void AdaTreeParserSuper::id_and_discrim(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST id_and_discrim_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp159_AST_in = _t;
		match(_t,IDENTIFIER);
		_t = _t->getNextSibling();
		discrim_part_opt(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::record_definition(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST record_definition_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case COMPONENT_ITEMS:
		{
			component_list(_t);
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
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::array_type_definition(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST array_type_definition_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		index_or_discrete_range_s(_t);
		_t = _retTree;
		component_subtype_def(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::enumeration_literal_specification(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST enumeration_literal_specification_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENTIFIER:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp160_AST_in = _t;
			match(_t,IDENTIFIER);
			_t = _t->getNextSibling();
			break;
		}
		case CHARACTER_LITERAL:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp161_AST_in = _t;
			match(_t,CHARACTER_LITERAL);
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

void AdaTreeParserSuper::index_or_discrete_range_s(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST index_or_discrete_range_s_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENTIFIER:
		case DOT:
		case TIC:
		case RANGE:
		case DOT_DOT:
		case CHARACTER_LITERAL:
		case CHAR_STRING:
		case NuLL:
		case MOD:
		case NOT:
		case PLUS:
		case MINUS:
		case CONCAT:
		case STAR:
		case DIV:
		case REM:
		case ABS:
		case EXPON:
		case NUMERIC_LIT:
		case ALLOCATOR:
		case INDEXED_COMPONENT:
		case OPERATOR_SYMBOL:
		case PARENTHESIZED_PRIMARY:
		case UNARY_MINUS:
		case UNARY_PLUS:
		{
			index_or_discrete_range(_t);
			_t = _retTree;
			break;
		}
		case COMMA:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t203 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp162_AST_in = _t;
			match(_t,COMMA);
			_t = _t->getFirstChild();
			index_or_discrete_range_s(_t);
			_t = _retTree;
			index_or_discrete_range(_t);
			_t = _retTree;
			_t = __t203;
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

void AdaTreeParserSuper::index_or_discrete_range(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST index_or_discrete_range_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case DOT_DOT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t205 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp163_AST_in = _t;
			match(_t,DOT_DOT);
			_t = _t->getFirstChild();
			simple_expression(_t);
			_t = _retTree;
			simple_expression(_t);
			_t = _retTree;
			_t = __t205;
			_t = _t->getNextSibling();
			break;
		}
		case RANGE:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t206 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp164_AST_in = _t;
			match(_t,RANGE);
			_t = _t->getFirstChild();
			simple_expression(_t);
			_t = _retTree;
			{
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case BOX:
			{
				ANTLR_USE_NAMESPACE(antlr)RefAST tmp165_AST_in = _t;
				match(_t,BOX);
				_t = _t->getNextSibling();
				break;
			}
			case DOT_DOT:
			case RANGE_ATTRIBUTE_REFERENCE:
			{
				range(_t);
				_t = _retTree;
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
			}
			}
			}
			_t = __t206;
			_t = _t->getNextSibling();
			break;
		}
		case IDENTIFIER:
		case DOT:
		case TIC:
		case CHARACTER_LITERAL:
		case CHAR_STRING:
		case NuLL:
		case MOD:
		case NOT:
		case PLUS:
		case MINUS:
		case CONCAT:
		case STAR:
		case DIV:
		case REM:
		case ABS:
		case EXPON:
		case NUMERIC_LIT:
		case ALLOCATOR:
		case INDEXED_COMPONENT:
		case OPERATOR_SYMBOL:
		case PARENTHESIZED_PRIMARY:
		case UNARY_MINUS:
		case UNARY_PLUS:
		{
			simple_expression(_t);
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

void AdaTreeParserSuper::constraint_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST constraint_opt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case DOT_DOT:
		case RANGE_ATTRIBUTE_REFERENCE:
		{
			range_constraint(_t);
			_t = _retTree;
			break;
		}
		case DIGITS_CONSTRAINT:
		{
			digits_constraint(_t);
			_t = _retTree;
			break;
		}
		case DELTA_CONSTRAINT:
		{
			delta_constraint(_t);
			_t = _retTree;
			break;
		}
		case INDEX_CONSTRAINT:
		{
			index_constraint(_t);
			_t = _retTree;
			break;
		}
		case DISCRIMINANT_CONSTRAINT:
		{
			discriminant_constraint(_t);
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
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::digits_constraint(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST digits_constraint_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t214 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp166_AST_in = _t;
		match(_t,DIGITS_CONSTRAINT);
		_t = _t->getFirstChild();
		expression(_t);
		_t = _retTree;
		range_constraint_opt(_t);
		_t = _retTree;
		_t = __t214;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::delta_constraint(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST delta_constraint_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t216 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp167_AST_in = _t;
		match(_t,DELTA_CONSTRAINT);
		_t = _t->getFirstChild();
		expression(_t);
		_t = _retTree;
		range_constraint_opt(_t);
		_t = _retTree;
		_t = __t216;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::index_constraint(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST index_constraint_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t218 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp168_AST_in = _t;
		match(_t,INDEX_CONSTRAINT);
		_t = _t->getFirstChild();
		{ // ( ... )+
		int _cnt220=0;
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			if ((_t->getType() == DOT_DOT || _t->getType() == RANGE_ATTRIBUTE_REFERENCE || _t->getType() == SUBTYPE_INDICATION)) {
				discrete_range(_t);
				_t = _retTree;
			}
			else {
				if ( _cnt220>=1 ) { goto _loop220; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);}
			}
			
			_cnt220++;
		}
		_loop220:;
		}  // ( ... )+
		_t = __t218;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::discriminant_constraint(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST discriminant_constraint_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t223 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp169_AST_in = _t;
		match(_t,DISCRIMINANT_CONSTRAINT);
		_t = _t->getFirstChild();
		{ // ( ... )+
		int _cnt225=0;
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			if ((_t->getType() == DISCRIMINANT_ASSOCIATION)) {
				discriminant_association(_t);
				_t = _retTree;
			}
			else {
				if ( _cnt225>=1 ) { goto _loop225; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);}
			}
			
			_cnt225++;
		}
		_loop225:;
		}  // ( ... )+
		_t = __t223;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::discrete_range(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST discrete_range_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case DOT_DOT:
		case RANGE_ATTRIBUTE_REFERENCE:
		{
			range(_t);
			_t = _retTree;
			break;
		}
		case SUBTYPE_INDICATION:
		{
			subtype_ind(_t);
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

void AdaTreeParserSuper::discriminant_association(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST discriminant_association_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t227 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp170_AST_in = _t;
		match(_t,DISCRIMINANT_ASSOCIATION);
		_t = _t->getFirstChild();
		selector_names_opt(_t);
		_t = _retTree;
		expression(_t);
		_t = _retTree;
		_t = __t227;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::selector_names_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST selector_names_opt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t229 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp171_AST_in = _t;
		match(_t,SELECTOR_NAMES_OPT);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			if ((_t->getType() == IDENTIFIER)) {
				selector_name(_t);
				_t = _retTree;
			}
			else {
				goto _loop231;
			}
			
		}
		_loop231:;
		} // ( ... )*
		_t = __t229;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::selector_name(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST selector_name_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp172_AST_in = _t;
		match(_t,IDENTIFIER);
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::component_list(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST component_list_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		component_items(_t);
		_t = _retTree;
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case VARIANT_PART:
		{
			variant_part(_t);
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
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::component_items(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST component_items_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t242 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp173_AST_in = _t;
		match(_t,COMPONENT_ITEMS);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case PRAGMA:
			{
				pragma(_t);
				_t = _retTree;
				break;
			}
			case COMPONENT_DECLARATION:
			{
				comp_decl(_t);
				_t = _retTree;
				break;
			}
			default:
			{
				goto _loop244;
			}
			}
		}
		_loop244:;
		} // ( ... )*
		_t = __t242;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::variant_part(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST variant_part_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t246 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp174_AST_in = _t;
		match(_t,VARIANT_PART);
		_t = _t->getFirstChild();
		discriminant_direct_name(_t);
		_t = _retTree;
		variant_s(_t);
		_t = _retTree;
		_t = __t246;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::discriminant_direct_name(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST discriminant_direct_name_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp175_AST_in = _t;
		match(_t,IDENTIFIER);
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::variant_s(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST variant_s_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t249 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp176_AST_in = _t;
		match(_t,VARIANTS);
		_t = _t->getFirstChild();
		{ // ( ... )+
		int _cnt251=0;
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			if ((_t->getType() == VARIANT)) {
				variant(_t);
				_t = _retTree;
			}
			else {
				if ( _cnt251>=1 ) { goto _loop251; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);}
			}
			
			_cnt251++;
		}
		_loop251:;
		}  // ( ... )+
		_t = __t249;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::variant(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST variant_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t253 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp177_AST_in = _t;
		match(_t,VARIANT);
		_t = _t->getFirstChild();
		choice_s(_t);
		_t = _retTree;
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case COMPONENT_ITEMS:
		{
			component_list(_t);
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
		_t = __t253;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::choice_s(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST choice_s_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case PIPE:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t256 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp178_AST_in = _t;
			match(_t,PIPE);
			_t = _t->getFirstChild();
			choice_s(_t);
			_t = _retTree;
			choice(_t);
			_t = _retTree;
			_t = __t256;
			_t = _t->getNextSibling();
			break;
		}
		case IDENTIFIER:
		case DOT:
		case TIC:
		case OTHERS:
		case DOT_DOT:
		case IN:
		case CHARACTER_LITERAL:
		case CHAR_STRING:
		case NuLL:
		case MOD:
		case OR:
		case AND:
		case XOR:
		case NOT:
		case EQ:
		case NE:
		case LT_:
		case LE:
		case GT:
		case GE:
		case PLUS:
		case MINUS:
		case CONCAT:
		case STAR:
		case DIV:
		case REM:
		case ABS:
		case EXPON:
		case NUMERIC_LIT:
		case ALLOCATOR:
		case INDEXED_COMPONENT:
		case OPERATOR_SYMBOL:
		case RANGE_ATTRIBUTE_REFERENCE:
		case AND_THEN:
		case MARK_WITH_CONSTRAINT:
		case NOT_IN:
		case OR_ELSE:
		case PARENTHESIZED_PRIMARY:
		case UNARY_MINUS:
		case UNARY_PLUS:
		{
			choice(_t);
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

void AdaTreeParserSuper::choice(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST choice_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case OTHERS:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp179_AST_in = _t;
			match(_t,OTHERS);
			_t = _t->getNextSibling();
			break;
		}
		case DOT_DOT:
		case RANGE_ATTRIBUTE_REFERENCE:
		case MARK_WITH_CONSTRAINT:
		{
			discrete_with_range(_t);
			_t = _retTree;
			break;
		}
		case IDENTIFIER:
		case DOT:
		case TIC:
		case IN:
		case CHARACTER_LITERAL:
		case CHAR_STRING:
		case NuLL:
		case MOD:
		case OR:
		case AND:
		case XOR:
		case NOT:
		case EQ:
		case NE:
		case LT_:
		case LE:
		case GT:
		case GE:
		case PLUS:
		case MINUS:
		case CONCAT:
		case STAR:
		case DIV:
		case REM:
		case ABS:
		case EXPON:
		case NUMERIC_LIT:
		case ALLOCATOR:
		case INDEXED_COMPONENT:
		case OPERATOR_SYMBOL:
		case AND_THEN:
		case NOT_IN:
		case OR_ELSE:
		case PARENTHESIZED_PRIMARY:
		case UNARY_MINUS:
		case UNARY_PLUS:
		{
			expression(_t);
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

void AdaTreeParserSuper::discrete_with_range(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST discrete_with_range_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case MARK_WITH_CONSTRAINT:
		{
			mark_with_constraint(_t);
			_t = _retTree;
			break;
		}
		case DOT_DOT:
		case RANGE_ATTRIBUTE_REFERENCE:
		{
			range(_t);
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

void AdaTreeParserSuper::mark_with_constraint(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST mark_with_constraint_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t260 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp180_AST_in = _t;
		match(_t,MARK_WITH_CONSTRAINT);
		_t = _t->getFirstChild();
		subtype_mark(_t);
		_t = _retTree;
		range_constraint(_t);
		_t = _retTree;
		_t = __t260;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::generic_formal_part_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST generic_formal_part_opt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t273 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp181_AST_in = _t;
		match(_t,GENERIC_FORMAL_PART);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case PRAGMA:
			{
				pragma(_t);
				_t = _retTree;
				break;
			}
			case USE_CLAUSE:
			case USE_TYPE_CLAUSE:
			{
				use_clause(_t);
				_t = _retTree;
				break;
			}
			case FORMAL_PACKAGE_DECLARATION:
			case PARAMETER_SPECIFICATION:
			case ACCESS_TO_FUNCTION_DECLARATION:
			case ACCESS_TO_OBJECT_DECLARATION:
			case ACCESS_TO_PROCEDURE_DECLARATION:
			case ARRAY_TYPE_DECLARATION:
			case FORMAL_DECIMAL_FIXED_POINT_DECLARATION:
			case FORMAL_DISCRETE_TYPE_DECLARATION:
			case FORMAL_FLOATING_POINT_DECLARATION:
			case FORMAL_FUNCTION_DECLARATION:
			case FORMAL_MODULAR_TYPE_DECLARATION:
			case FORMAL_ORDINARY_DERIVED_TYPE_DECLARATION:
			case FORMAL_ORDINARY_FIXED_POINT_DECLARATION:
			case FORMAL_PRIVATE_EXTENSION_DECLARATION:
			case FORMAL_PRIVATE_TYPE_DECLARATION:
			case FORMAL_PROCEDURE_DECLARATION:
			case FORMAL_SIGNED_INTEGER_TYPE_DECLARATION:
			{
				generic_formal_parameter(_t);
				_t = _retTree;
				break;
			}
			default:
			{
				goto _loop275;
			}
			}
		}
		_loop275:;
		} // ( ... )*
		_t = __t273;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::generic_formal_parameter(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST generic_formal_parameter_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case FORMAL_DISCRETE_TYPE_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t277 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp182_AST_in = _t;
			match(_t,FORMAL_DISCRETE_TYPE_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			_t = __t277;
			_t = _t->getNextSibling();
			break;
		}
		case FORMAL_SIGNED_INTEGER_TYPE_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t278 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp183_AST_in = _t;
			match(_t,FORMAL_SIGNED_INTEGER_TYPE_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			_t = __t278;
			_t = _t->getNextSibling();
			break;
		}
		case FORMAL_MODULAR_TYPE_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t279 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp184_AST_in = _t;
			match(_t,FORMAL_MODULAR_TYPE_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			_t = __t279;
			_t = _t->getNextSibling();
			break;
		}
		case FORMAL_DECIMAL_FIXED_POINT_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t280 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp185_AST_in = _t;
			match(_t,FORMAL_DECIMAL_FIXED_POINT_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			_t = __t280;
			_t = _t->getNextSibling();
			break;
		}
		case FORMAL_ORDINARY_FIXED_POINT_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t281 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp186_AST_in = _t;
			match(_t,FORMAL_ORDINARY_FIXED_POINT_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			_t = __t281;
			_t = _t->getNextSibling();
			break;
		}
		case FORMAL_FLOATING_POINT_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t282 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp187_AST_in = _t;
			match(_t,FORMAL_FLOATING_POINT_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			_t = __t282;
			_t = _t->getNextSibling();
			break;
		}
		case ARRAY_TYPE_DECLARATION:
		{
			formal_array_type_declaration(_t);
			_t = _retTree;
			break;
		}
		case ACCESS_TO_FUNCTION_DECLARATION:
		case ACCESS_TO_OBJECT_DECLARATION:
		case ACCESS_TO_PROCEDURE_DECLARATION:
		{
			formal_access_type_declaration(_t);
			_t = _retTree;
			break;
		}
		case FORMAL_PRIVATE_TYPE_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t283 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp188_AST_in = _t;
			match(_t,FORMAL_PRIVATE_TYPE_DECLARATION);
			_t = _t->getFirstChild();
			id_part(_t);
			_t = _retTree;
			modifiers(_t);
			_t = _retTree;
			_t = __t283;
			_t = _t->getNextSibling();
			break;
		}
		case FORMAL_ORDINARY_DERIVED_TYPE_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t284 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp189_AST_in = _t;
			match(_t,FORMAL_ORDINARY_DERIVED_TYPE_DECLARATION);
			_t = _t->getFirstChild();
			id_part(_t);
			_t = _retTree;
			subtype_ind(_t);
			_t = _retTree;
			_t = __t284;
			_t = _t->getNextSibling();
			break;
		}
		case FORMAL_PRIVATE_EXTENSION_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t285 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp190_AST_in = _t;
			match(_t,FORMAL_PRIVATE_EXTENSION_DECLARATION);
			_t = _t->getFirstChild();
			id_part(_t);
			_t = _retTree;
			modifiers(_t);
			_t = _retTree;
			subtype_ind(_t);
			_t = _retTree;
			_t = __t285;
			_t = _t->getNextSibling();
			break;
		}
		case FORMAL_PROCEDURE_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t286 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp191_AST_in = _t;
			match(_t,FORMAL_PROCEDURE_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			formal_part_opt(_t);
			_t = _retTree;
			subprogram_default_opt(_t);
			_t = _retTree;
			_t = __t286;
			_t = _t->getNextSibling();
			break;
		}
		case FORMAL_FUNCTION_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t287 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp192_AST_in = _t;
			match(_t,FORMAL_FUNCTION_DECLARATION);
			_t = _t->getFirstChild();
			def_designator(_t);
			_t = _retTree;
			function_tail(_t);
			_t = _retTree;
			subprogram_default_opt(_t);
			_t = _retTree;
			_t = __t287;
			_t = _t->getNextSibling();
			break;
		}
		case FORMAL_PACKAGE_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t288 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp193_AST_in = _t;
			match(_t,FORMAL_PACKAGE_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			compound_name(_t);
			_t = _retTree;
			formal_package_actual_part_opt(_t);
			_t = _retTree;
			_t = __t288;
			_t = _t->getNextSibling();
			break;
		}
		case PARAMETER_SPECIFICATION:
		{
			parameter_specification(_t);
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

void AdaTreeParserSuper::formal_array_type_declaration(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST formal_array_type_declaration_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		array_type_declaration(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::formal_access_type_declaration(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST formal_access_type_declaration_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		access_type_declaration(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::id_part(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST id_part_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		def_id(_t);
		_t = _retTree;
		discrim_part_opt(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::subprogram_default_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST subprogram_default_opt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case BOX:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp194_AST_in = _t;
			match(_t,BOX);
			_t = _t->getNextSibling();
			break;
		}
		case IDENTIFIER:
		case DOT:
		case TIC:
		case INDEXED_COMPONENT:
		{
			name(_t);
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
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::formal_package_actual_part_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST formal_package_actual_part_opt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case BOX:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp195_AST_in = _t;
			match(_t,BOX);
			_t = _t->getNextSibling();
			break;
		}
		case DEFINING_IDENTIFIER_LIST:
		{
			defining_identifier_list(_t);
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
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::procedure_body(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST procedure_body_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t298 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp196_AST_in = _t;
		match(_t,PROCEDURE_BODY);
		_t = _t->getFirstChild();
		def_id(_t);
		_t = _retTree;
		formal_part_opt(_t);
		_t = _retTree;
		body_part(_t);
		_t = _retTree;
		_t = __t298;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::function_body(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST function_body_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t300 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp197_AST_in = _t;
		match(_t,FUNCTION_BODY);
		_t = _t->getFirstChild();
		def_designator(_t);
		_t = _retTree;
		function_tail(_t);
		_t = _retTree;
		body_part(_t);
		_t = _retTree;
		_t = __t300;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::body_part(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST body_part_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		declarative_part(_t);
		_t = _retTree;
		block_body(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::declarative_part(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST declarative_part_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t303 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp198_AST_in = _t;
		match(_t,DECLARATIVE_PART);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case PRAGMA:
			{
				pragma(_t);
				_t = _retTree;
				break;
			}
			case ATTRIBUTE_DEFINITION_CLAUSE:
			case AT_CLAUSE:
			case ENUMERATION_REPESENTATION_CLAUSE:
			case EXCEPTION_DECLARATION:
			case EXCEPTION_RENAMING_DECLARATION:
			case GENERIC_PACKAGE_DECLARATION:
			case INCOMPLETE_TYPE_DECLARATION:
			case NUMBER_DECLARATION:
			case OBJECT_DECLARATION:
			case OBJECT_RENAMING_DECLARATION:
			case PACKAGE_BODY:
			case PACKAGE_BODY_STUB:
			case PACKAGE_RENAMING_DECLARATION:
			case PACKAGE_SPECIFICATION:
			case PRIVATE_EXTENSION_DECLARATION:
			case PRIVATE_TYPE_DECLARATION:
			case PROTECTED_BODY:
			case PROTECTED_BODY_STUB:
			case PROTECTED_TYPE_DECLARATION:
			case RECORD_REPRESENTATION_CLAUSE:
			case SINGLE_PROTECTED_DECLARATION:
			case SINGLE_TASK_DECLARATION:
			case SUBTYPE_DECLARATION:
			case TASK_BODY:
			case TASK_BODY_STUB:
			case TASK_TYPE_DECLARATION:
			case USE_CLAUSE:
			case USE_TYPE_CLAUSE:
			case ABSTRACT_FUNCTION_DECLARATION:
			case ABSTRACT_PROCEDURE_DECLARATION:
			case ACCESS_TO_FUNCTION_DECLARATION:
			case ACCESS_TO_OBJECT_DECLARATION:
			case ACCESS_TO_PROCEDURE_DECLARATION:
			case ARRAY_OBJECT_DECLARATION:
			case ARRAY_TYPE_DECLARATION:
			case DECIMAL_FIXED_POINT_DECLARATION:
			case DERIVED_RECORD_EXTENSION:
			case ENUMERATION_TYPE_DECLARATION:
			case FLOATING_POINT_DECLARATION:
			case FUNCTION_BODY:
			case FUNCTION_BODY_STUB:
			case FUNCTION_DECLARATION:
			case FUNCTION_RENAMING_DECLARATION:
			case GENERIC_FUNCTION_DECLARATION:
			case GENERIC_FUNCTION_INSTANTIATION:
			case GENERIC_FUNCTION_RENAMING:
			case GENERIC_PACKAGE_INSTANTIATION:
			case GENERIC_PACKAGE_RENAMING:
			case GENERIC_PROCEDURE_DECLARATION:
			case GENERIC_PROCEDURE_INSTANTIATION:
			case GENERIC_PROCEDURE_RENAMING:
			case MODULAR_TYPE_DECLARATION:
			case ORDINARY_DERIVED_TYPE_DECLARATION:
			case ORDINARY_FIXED_POINT_DECLARATION:
			case PROCEDURE_BODY:
			case PROCEDURE_BODY_STUB:
			case PROCEDURE_DECLARATION:
			case PROCEDURE_RENAMING_DECLARATION:
			case RECORD_TYPE_DECLARATION:
			case SIGNED_INTEGER_TYPE_DECLARATION:
			{
				declarative_item(_t);
				_t = _retTree;
				break;
			}
			default:
			{
				goto _loop305;
			}
			}
		}
		_loop305:;
		} // ( ... )*
		_t = __t303;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::block_body(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST block_body_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t325 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp199_AST_in = _t;
		match(_t,BLOCK_BODY);
		_t = _t->getFirstChild();
		handled_stmt_s(_t);
		_t = _retTree;
		_t = __t325;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::declarative_item(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST declarative_item_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case PACKAGE_BODY_STUB:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t307 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp200_AST_in = _t;
			match(_t,PACKAGE_BODY_STUB);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			_t = __t307;
			_t = _t->getNextSibling();
			break;
		}
		case PACKAGE_BODY:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t308 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp201_AST_in = _t;
			match(_t,PACKAGE_BODY);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			pkg_body_part(_t);
			_t = _retTree;
			_t = __t308;
			_t = _t->getNextSibling();
			break;
		}
		case PACKAGE_RENAMING_DECLARATION:
		case PACKAGE_SPECIFICATION:
		case GENERIC_PACKAGE_INSTANTIATION:
		{
			spec_decl_part(_t);
			_t = _retTree;
			break;
		}
		case TASK_BODY_STUB:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t309 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp202_AST_in = _t;
			match(_t,TASK_BODY_STUB);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			_t = __t309;
			_t = _t->getNextSibling();
			break;
		}
		case TASK_BODY:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t310 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp203_AST_in = _t;
			match(_t,TASK_BODY);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			body_part(_t);
			_t = _retTree;
			_t = __t310;
			_t = _t->getNextSibling();
			break;
		}
		case SINGLE_TASK_DECLARATION:
		case TASK_TYPE_DECLARATION:
		{
			task_type_or_single_decl(_t);
			_t = _retTree;
			break;
		}
		case PROTECTED_BODY_STUB:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t311 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp204_AST_in = _t;
			match(_t,PROTECTED_BODY_STUB);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			_t = __t311;
			_t = _t->getNextSibling();
			break;
		}
		case PROTECTED_BODY:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t312 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp205_AST_in = _t;
			match(_t,PROTECTED_BODY);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			prot_op_bodies_opt(_t);
			_t = _retTree;
			_t = __t312;
			_t = _t->getNextSibling();
			break;
		}
		case PROTECTED_TYPE_DECLARATION:
		case SINGLE_PROTECTED_DECLARATION:
		{
			prot_type_or_single_decl(_t);
			_t = _retTree;
			break;
		}
		case ABSTRACT_FUNCTION_DECLARATION:
		case ABSTRACT_PROCEDURE_DECLARATION:
		case FUNCTION_BODY:
		case FUNCTION_BODY_STUB:
		case FUNCTION_DECLARATION:
		case FUNCTION_RENAMING_DECLARATION:
		case GENERIC_FUNCTION_INSTANTIATION:
		case GENERIC_PROCEDURE_INSTANTIATION:
		case PROCEDURE_BODY:
		case PROCEDURE_BODY_STUB:
		case PROCEDURE_DECLARATION:
		case PROCEDURE_RENAMING_DECLARATION:
		{
			subprog_decl_or_rename_or_inst_or_body(_t);
			_t = _retTree;
			break;
		}
		case ATTRIBUTE_DEFINITION_CLAUSE:
		case AT_CLAUSE:
		case ENUMERATION_REPESENTATION_CLAUSE:
		case EXCEPTION_DECLARATION:
		case EXCEPTION_RENAMING_DECLARATION:
		case GENERIC_PACKAGE_DECLARATION:
		case INCOMPLETE_TYPE_DECLARATION:
		case NUMBER_DECLARATION:
		case OBJECT_DECLARATION:
		case OBJECT_RENAMING_DECLARATION:
		case PRIVATE_EXTENSION_DECLARATION:
		case PRIVATE_TYPE_DECLARATION:
		case RECORD_REPRESENTATION_CLAUSE:
		case SUBTYPE_DECLARATION:
		case USE_CLAUSE:
		case USE_TYPE_CLAUSE:
		case ACCESS_TO_FUNCTION_DECLARATION:
		case ACCESS_TO_OBJECT_DECLARATION:
		case ACCESS_TO_PROCEDURE_DECLARATION:
		case ARRAY_OBJECT_DECLARATION:
		case ARRAY_TYPE_DECLARATION:
		case DECIMAL_FIXED_POINT_DECLARATION:
		case DERIVED_RECORD_EXTENSION:
		case ENUMERATION_TYPE_DECLARATION:
		case FLOATING_POINT_DECLARATION:
		case GENERIC_FUNCTION_DECLARATION:
		case GENERIC_FUNCTION_RENAMING:
		case GENERIC_PACKAGE_RENAMING:
		case GENERIC_PROCEDURE_DECLARATION:
		case GENERIC_PROCEDURE_RENAMING:
		case MODULAR_TYPE_DECLARATION:
		case ORDINARY_DERIVED_TYPE_DECLARATION:
		case ORDINARY_FIXED_POINT_DECLARATION:
		case RECORD_TYPE_DECLARATION:
		case SIGNED_INTEGER_TYPE_DECLARATION:
		{
			decl_common(_t);
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

void AdaTreeParserSuper::prot_op_bodies_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST prot_op_bodies_opt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t318 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp206_AST_in = _t;
		match(_t,PROT_OP_BODIES_OPT);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case PRAGMA:
			{
				pragma(_t);
				_t = _retTree;
				break;
			}
			case ENTRY_BODY:
			{
				entry_body(_t);
				_t = _retTree;
				break;
			}
			case FUNCTION_BODY:
			case FUNCTION_DECLARATION:
			case PROCEDURE_BODY:
			case PROCEDURE_DECLARATION:
			{
				subprog_decl_or_body(_t);
				_t = _retTree;
				break;
			}
			default:
			{
				goto _loop320;
			}
			}
		}
		_loop320:;
		} // ( ... )*
		_t = __t318;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::block_body_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST block_body_opt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t315 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp207_AST_in = _t;
		match(_t,BLOCK_BODY_OPT);
		_t = _t->getFirstChild();
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case HANDLED_SEQUENCE_OF_STATEMENTS:
		{
			handled_stmt_s(_t);
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
		_t = __t315;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::handled_stmt_s(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST handled_stmt_s_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t327 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp208_AST_in = _t;
		match(_t,HANDLED_SEQUENCE_OF_STATEMENTS);
		_t = _t->getFirstChild();
		statements(_t);
		_t = _retTree;
		except_handler_part_opt(_t);
		_t = _retTree;
		_t = __t327;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::entry_body(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST entry_body_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t384 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp209_AST_in = _t;
		match(_t,ENTRY_BODY);
		_t = _t->getFirstChild();
		def_id(_t);
		_t = _retTree;
		entry_body_formal_part(_t);
		_t = _retTree;
		entry_barrier(_t);
		_t = _retTree;
		body_part(_t);
		_t = _retTree;
		_t = __t384;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::subprog_decl_or_body(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST subprog_decl_or_body_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case PROCEDURE_BODY:
		{
			procedure_body(_t);
			_t = _retTree;
			break;
		}
		case PROCEDURE_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t322 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp210_AST_in = _t;
			match(_t,PROCEDURE_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			formal_part_opt(_t);
			_t = _retTree;
			_t = __t322;
			_t = _t->getNextSibling();
			break;
		}
		case FUNCTION_BODY:
		{
			function_body(_t);
			_t = _retTree;
			break;
		}
		case FUNCTION_DECLARATION:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t323 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp211_AST_in = _t;
			match(_t,FUNCTION_DECLARATION);
			_t = _t->getFirstChild();
			def_designator(_t);
			_t = _retTree;
			function_tail(_t);
			_t = _retTree;
			_t = __t323;
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

void AdaTreeParserSuper::statements(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST statements_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t329 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp212_AST_in = _t;
		match(_t,SEQUENCE_OF_STATEMENTS);
		_t = _t->getFirstChild();
		{ // ( ... )+
		int _cnt331=0;
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case PRAGMA:
			{
				pragma(_t);
				_t = _retTree;
				break;
			}
			case STATEMENT:
			{
				statement(_t);
				_t = _retTree;
				break;
			}
			default:
			{
				if ( _cnt331>=1 ) { goto _loop331; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);}
			}
			}
			_cnt331++;
		}
		_loop331:;
		}  // ( ... )+
		_t = __t329;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::except_handler_part_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST except_handler_part_opt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t435 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp213_AST_in = _t;
		match(_t,EXCEPT_HANDLER_PART_OPT);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			if ((_t->getType() == EXCEPTION_HANDLER)) {
				exception_handler(_t);
				_t = _retTree;
			}
			else {
				goto _loop437;
			}
			
		}
		_loop437:;
		} // ( ... )*
		_t = __t435;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::statement(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST statement_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t333 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp214_AST_in = _t;
		match(_t,STATEMENT);
		_t = _t->getFirstChild();
		def_label_opt(_t);
		_t = _retTree;
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case NULL_STATEMENT:
		{
			null_stmt(_t);
			_t = _retTree;
			break;
		}
		case EXIT_STATEMENT:
		{
			exit_stmt(_t);
			_t = _retTree;
			break;
		}
		case RETURN_STATEMENT:
		{
			return_stmt(_t);
			_t = _retTree;
			break;
		}
		case GOTO_STATEMENT:
		{
			goto_stmt(_t);
			_t = _retTree;
			break;
		}
		case DELAY_STATEMENT:
		{
			delay_stmt(_t);
			_t = _retTree;
			break;
		}
		case ABORT_STATEMENT:
		{
			abort_stmt(_t);
			_t = _retTree;
			break;
		}
		case RAISE_STATEMENT:
		{
			raise_stmt(_t);
			_t = _retTree;
			break;
		}
		case REQUEUE_STATEMENT:
		{
			requeue_stmt(_t);
			_t = _retTree;
			break;
		}
		case ACCEPT_STATEMENT:
		{
			accept_stmt(_t);
			_t = _retTree;
			break;
		}
		case ASYNCHRONOUS_SELECT:
		case CONDITIONAL_ENTRY_CALL:
		case SELECTIVE_ACCEPT:
		case TIMED_ENTRY_CALL:
		{
			select_stmt(_t);
			_t = _retTree;
			break;
		}
		case IF_STATEMENT:
		{
			if_stmt(_t);
			_t = _retTree;
			break;
		}
		case CASE_STATEMENT:
		{
			case_stmt(_t);
			_t = _retTree;
			break;
		}
		case LOOP_STATEMENT:
		{
			loop_stmt(_t);
			_t = _retTree;
			break;
		}
		case BLOCK_STATEMENT:
		{
			block(_t);
			_t = _retTree;
			break;
		}
		case ASSIGNMENT_STATEMENT:
		case CALL_STATEMENT:
		{
			call_or_assignment(_t);
			_t = _retTree;
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
		}
		}
		}
		_t = __t333;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::def_label_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST def_label_opt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t336 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp215_AST_in = _t;
		match(_t,LABEL_OPT);
		_t = _t->getFirstChild();
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENTIFIER:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp216_AST_in = _t;
			match(_t,IDENTIFIER);
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
		_t = __t336;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::null_stmt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST null_stmt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp217_AST_in = _t;
		match(_t,NULL_STATEMENT);
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::exit_stmt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST exit_stmt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t371 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp218_AST_in = _t;
		match(_t,EXIT_STATEMENT);
		_t = _t->getFirstChild();
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENTIFIER:
		{
			label_name(_t);
			_t = _retTree;
			break;
		}
		case 3:
		case WHEN:
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
		case WHEN:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp219_AST_in = _t;
			match(_t,WHEN);
			_t = _t->getNextSibling();
			condition(_t);
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
		_t = __t371;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::return_stmt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST return_stmt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t376 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp220_AST_in = _t;
		match(_t,RETURN_STATEMENT);
		_t = _t->getFirstChild();
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENTIFIER:
		case DOT:
		case TIC:
		case IN:
		case CHARACTER_LITERAL:
		case CHAR_STRING:
		case NuLL:
		case MOD:
		case OR:
		case AND:
		case XOR:
		case NOT:
		case EQ:
		case NE:
		case LT_:
		case LE:
		case GT:
		case GE:
		case PLUS:
		case MINUS:
		case CONCAT:
		case STAR:
		case DIV:
		case REM:
		case ABS:
		case EXPON:
		case NUMERIC_LIT:
		case ALLOCATOR:
		case INDEXED_COMPONENT:
		case OPERATOR_SYMBOL:
		case AND_THEN:
		case NOT_IN:
		case OR_ELSE:
		case PARENTHESIZED_PRIMARY:
		case UNARY_MINUS:
		case UNARY_PLUS:
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
		_t = __t376;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::goto_stmt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST goto_stmt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t379 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp221_AST_in = _t;
		match(_t,GOTO_STATEMENT);
		_t = _t->getFirstChild();
		label_name(_t);
		_t = _retTree;
		_t = __t379;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::delay_stmt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST delay_stmt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t399 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp222_AST_in = _t;
		match(_t,DELAY_STATEMENT);
		_t = _t->getFirstChild();
		modifiers(_t);
		_t = _retTree;
		expression(_t);
		_t = _retTree;
		_t = __t399;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::abort_stmt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST abort_stmt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t431 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp223_AST_in = _t;
		match(_t,ABORT_STATEMENT);
		_t = _t->getFirstChild();
		{ // ( ... )+
		int _cnt433=0;
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			if ((_tokenSet_4.member(_t->getType()))) {
				name(_t);
				_t = _retTree;
			}
			else {
				if ( _cnt433>=1 ) { goto _loop433; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);}
			}
			
			_cnt433++;
		}
		_loop433:;
		}  // ( ... )+
		_t = __t431;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::raise_stmt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST raise_stmt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t447 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp224_AST_in = _t;
		match(_t,RAISE_STATEMENT);
		_t = _t->getFirstChild();
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENTIFIER:
		case DOT:
		{
			compound_name(_t);
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
		_t = __t447;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::requeue_stmt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST requeue_stmt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t450 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp225_AST_in = _t;
		match(_t,REQUEUE_STATEMENT);
		_t = _t->getFirstChild();
		name(_t);
		_t = _retTree;
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case ABORT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp226_AST_in = _t;
			match(_t,ABORT);
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
		_t = __t450;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::accept_stmt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST accept_stmt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t393 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp227_AST_in = _t;
		match(_t,ACCEPT_STATEMENT);
		_t = _t->getFirstChild();
		def_id(_t);
		_t = _retTree;
		entry_index_opt(_t);
		_t = _retTree;
		formal_part_opt(_t);
		_t = _retTree;
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case HANDLED_SEQUENCE_OF_STATEMENTS:
		{
			handled_stmt_s(_t);
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
		_t = __t393;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::select_stmt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST select_stmt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case ASYNCHRONOUS_SELECT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t401 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp228_AST_in = _t;
			match(_t,ASYNCHRONOUS_SELECT);
			_t = _t->getFirstChild();
			triggering_alternative(_t);
			_t = _retTree;
			abortable_part(_t);
			_t = _retTree;
			_t = __t401;
			_t = _t->getNextSibling();
			break;
		}
		case SELECTIVE_ACCEPT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t402 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp229_AST_in = _t;
			match(_t,SELECTIVE_ACCEPT);
			_t = _t->getFirstChild();
			selective_accept(_t);
			_t = _retTree;
			_t = __t402;
			_t = _t->getNextSibling();
			break;
		}
		case TIMED_ENTRY_CALL:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t403 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp230_AST_in = _t;
			match(_t,TIMED_ENTRY_CALL);
			_t = _t->getFirstChild();
			entry_call_alternative(_t);
			_t = _retTree;
			delay_alternative(_t);
			_t = _retTree;
			_t = __t403;
			_t = _t->getNextSibling();
			break;
		}
		case CONDITIONAL_ENTRY_CALL:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t404 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp231_AST_in = _t;
			match(_t,CONDITIONAL_ENTRY_CALL);
			_t = _t->getFirstChild();
			entry_call_alternative(_t);
			_t = _retTree;
			statements(_t);
			_t = _retTree;
			_t = __t404;
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

void AdaTreeParserSuper::if_stmt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST if_stmt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t340 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp232_AST_in = _t;
		match(_t,IF_STATEMENT);
		_t = _t->getFirstChild();
		cond_clause(_t);
		_t = _retTree;
		elsifs_opt(_t);
		_t = _retTree;
		else_opt(_t);
		_t = _retTree;
		_t = __t340;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::case_stmt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST case_stmt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t352 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp233_AST_in = _t;
		match(_t,CASE_STATEMENT);
		_t = _t->getFirstChild();
		expression(_t);
		_t = _retTree;
		alternative_s(_t);
		_t = _retTree;
		_t = __t352;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::loop_stmt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST loop_stmt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t359 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp234_AST_in = _t;
		match(_t,LOOP_STATEMENT);
		_t = _t->getFirstChild();
		iteration_scheme_opt(_t);
		_t = _retTree;
		statements(_t);
		_t = _retTree;
		_t = __t359;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::block(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST block_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t366 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp235_AST_in = _t;
		match(_t,BLOCK_STATEMENT);
		_t = _t->getFirstChild();
		declare_opt(_t);
		_t = _retTree;
		block_body(_t);
		_t = _retTree;
		_t = __t366;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::call_or_assignment(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST call_or_assignment_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case ASSIGNMENT_STATEMENT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t381 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp236_AST_in = _t;
			match(_t,ASSIGNMENT_STATEMENT);
			_t = _t->getFirstChild();
			name(_t);
			_t = _retTree;
			expression(_t);
			_t = _retTree;
			_t = __t381;
			_t = _t->getNextSibling();
			break;
		}
		case CALL_STATEMENT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t382 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp237_AST_in = _t;
			match(_t,CALL_STATEMENT);
			_t = _t->getFirstChild();
			name(_t);
			_t = _retTree;
			_t = __t382;
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

void AdaTreeParserSuper::cond_clause(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST cond_clause_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t342 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp238_AST_in = _t;
		match(_t,COND_CLAUSE);
		_t = _t->getFirstChild();
		condition(_t);
		_t = _retTree;
		statements(_t);
		_t = _retTree;
		_t = __t342;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::elsifs_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST elsifs_opt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t345 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp239_AST_in = _t;
		match(_t,ELSIFS_OPT);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			if ((_t->getType() == COND_CLAUSE)) {
				cond_clause(_t);
				_t = _retTree;
			}
			else {
				goto _loop347;
			}
			
		}
		_loop347:;
		} // ( ... )*
		_t = __t345;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::else_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST else_opt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t349 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp240_AST_in = _t;
		match(_t,ELSE_OPT);
		_t = _t->getFirstChild();
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case SEQUENCE_OF_STATEMENTS:
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
		}
		}
		}
		_t = __t349;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::condition(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST condition_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		expression(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::alternative_s(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST alternative_s_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		{ // ( ... )+
		int _cnt355=0;
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			if ((_t->getType() == CASE_STATEMENT_ALTERNATIVE)) {
				case_statement_alternative(_t);
				_t = _retTree;
			}
			else {
				if ( _cnt355>=1 ) { goto _loop355; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);}
			}
			
			_cnt355++;
		}
		_loop355:;
		}  // ( ... )+
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::case_statement_alternative(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST case_statement_alternative_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t357 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp241_AST_in = _t;
		match(_t,CASE_STATEMENT_ALTERNATIVE);
		_t = _t->getFirstChild();
		choice_s(_t);
		_t = _retTree;
		statements(_t);
		_t = _retTree;
		_t = __t357;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::iteration_scheme_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST iteration_scheme_opt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t361 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp242_AST_in = _t;
		match(_t,ITERATION_SCHEME_OPT);
		_t = _t->getFirstChild();
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case WHILE:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t363 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp243_AST_in = _t;
			match(_t,WHILE);
			_t = _t->getFirstChild();
			condition(_t);
			_t = _retTree;
			_t = __t363;
			_t = _t->getNextSibling();
			break;
		}
		case FOR:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t364 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp244_AST_in = _t;
			match(_t,FOR);
			_t = _t->getFirstChild();
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp245_AST_in = _t;
			match(_t,IDENTIFIER);
			_t = _t->getNextSibling();
			modifiers(_t);
			_t = _retTree;
			discrete_subtype_definition(_t);
			_t = _retTree;
			_t = __t364;
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
		_t = __t361;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::declare_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST declare_opt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t368 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp246_AST_in = _t;
		match(_t,DECLARE_OPT);
		_t = _t->getFirstChild();
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case DECLARATIVE_PART:
		{
			declarative_part(_t);
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
		_t = __t368;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::label_name(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST label_name_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp247_AST_in = _t;
		match(_t,IDENTIFIER);
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::entry_body_formal_part(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST entry_body_formal_part_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		entry_index_spec_opt(_t);
		_t = _retTree;
		formal_part_opt(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::entry_barrier(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST entry_barrier_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		condition(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::entry_index_spec_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST entry_index_spec_opt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t387 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp248_AST_in = _t;
		match(_t,ENTRY_INDEX_SPECIFICATION);
		_t = _t->getFirstChild();
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENTIFIER:
		case DOT:
		{
			def_id(_t);
			_t = _retTree;
			discrete_subtype_definition(_t);
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
		_t = __t387;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::entry_call_stmt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST entry_call_stmt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t391 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp249_AST_in = _t;
		match(_t,ENTRY_CALL_STATEMENT);
		_t = _t->getFirstChild();
		name(_t);
		_t = _retTree;
		_t = __t391;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::entry_index_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST entry_index_opt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t396 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp250_AST_in = _t;
		match(_t,ENTRY_INDEX_OPT);
		_t = _t->getFirstChild();
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENTIFIER:
		case DOT:
		case TIC:
		case IN:
		case CHARACTER_LITERAL:
		case CHAR_STRING:
		case NuLL:
		case MOD:
		case OR:
		case AND:
		case XOR:
		case NOT:
		case EQ:
		case NE:
		case LT_:
		case LE:
		case GT:
		case GE:
		case PLUS:
		case MINUS:
		case CONCAT:
		case STAR:
		case DIV:
		case REM:
		case ABS:
		case EXPON:
		case NUMERIC_LIT:
		case ALLOCATOR:
		case INDEXED_COMPONENT:
		case OPERATOR_SYMBOL:
		case AND_THEN:
		case NOT_IN:
		case OR_ELSE:
		case PARENTHESIZED_PRIMARY:
		case UNARY_MINUS:
		case UNARY_PLUS:
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
		_t = __t396;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::triggering_alternative(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST triggering_alternative_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t406 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp251_AST_in = _t;
		match(_t,TRIGGERING_ALTERNATIVE);
		_t = _t->getFirstChild();
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case DELAY_STATEMENT:
		{
			delay_stmt(_t);
			_t = _retTree;
			break;
		}
		case ENTRY_CALL_STATEMENT:
		{
			entry_call_stmt(_t);
			_t = _retTree;
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
		}
		}
		}
		stmts_opt(_t);
		_t = _retTree;
		_t = __t406;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::abortable_part(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST abortable_part_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t409 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp252_AST_in = _t;
		match(_t,ABORTABLE_PART);
		_t = _t->getFirstChild();
		stmts_opt(_t);
		_t = _retTree;
		_t = __t409;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::selective_accept(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST selective_accept_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		guard_opt(_t);
		_t = _retTree;
		select_alternative(_t);
		_t = _retTree;
		or_select_opt(_t);
		_t = _retTree;
		else_opt(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::entry_call_alternative(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST entry_call_alternative_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t411 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp253_AST_in = _t;
		match(_t,ENTRY_CALL_ALTERNATIVE);
		_t = _t->getFirstChild();
		entry_call_stmt(_t);
		_t = _retTree;
		stmts_opt(_t);
		_t = _retTree;
		_t = __t411;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::delay_alternative(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST delay_alternative_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t422 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp254_AST_in = _t;
		match(_t,DELAY_ALTERNATIVE);
		_t = _t->getFirstChild();
		delay_stmt(_t);
		_t = _retTree;
		stmts_opt(_t);
		_t = _retTree;
		_t = __t422;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::stmts_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST stmts_opt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		{ // ( ... )*
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case PRAGMA:
			{
				pragma(_t);
				_t = _retTree;
				break;
			}
			case STATEMENT:
			{
				statement(_t);
				_t = _retTree;
				break;
			}
			default:
			{
				goto _loop425;
			}
			}
		}
		_loop425:;
		} // ( ... )*
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::guard_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST guard_opt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t414 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp255_AST_in = _t;
		match(_t,GUARD_OPT);
		_t = _t->getFirstChild();
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENTIFIER:
		case DOT:
		case TIC:
		case IN:
		case CHARACTER_LITERAL:
		case CHAR_STRING:
		case NuLL:
		case MOD:
		case OR:
		case AND:
		case XOR:
		case NOT:
		case EQ:
		case NE:
		case LT_:
		case LE:
		case GT:
		case GE:
		case PLUS:
		case MINUS:
		case CONCAT:
		case STAR:
		case DIV:
		case REM:
		case ABS:
		case EXPON:
		case NUMERIC_LIT:
		case ALLOCATOR:
		case INDEXED_COMPONENT:
		case OPERATOR_SYMBOL:
		case AND_THEN:
		case NOT_IN:
		case OR_ELSE:
		case PARENTHESIZED_PRIMARY:
		case UNARY_MINUS:
		case UNARY_PLUS:
		{
			condition(_t);
			_t = _retTree;
			{ // ( ... )*
			for (;;) {
				if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
					_t = ASTNULL;
				if ((_t->getType() == PRAGMA)) {
					pragma(_t);
					_t = _retTree;
				}
				else {
					goto _loop417;
				}
				
			}
			_loop417:;
			} // ( ... )*
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
		_t = __t414;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::select_alternative(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST select_alternative_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case ACCEPT_ALTERNATIVE:
		{
			accept_alternative(_t);
			_t = _retTree;
			break;
		}
		case DELAY_ALTERNATIVE:
		{
			delay_alternative(_t);
			_t = _retTree;
			break;
		}
		case TERMINATE_ALTERNATIVE:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp256_AST_in = _t;
			match(_t,TERMINATE_ALTERNATIVE);
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

void AdaTreeParserSuper::or_select_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST or_select_opt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t427 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp257_AST_in = _t;
		match(_t,OR_SELECT_OPT);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			if ((_t->getType() == GUARD_OPT)) {
				guard_opt(_t);
				_t = _retTree;
				select_alternative(_t);
				_t = _retTree;
			}
			else {
				goto _loop429;
			}
			
		}
		_loop429:;
		} // ( ... )*
		_t = __t427;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::accept_alternative(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST accept_alternative_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t420 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp258_AST_in = _t;
		match(_t,ACCEPT_ALTERNATIVE);
		_t = _t->getFirstChild();
		accept_stmt(_t);
		_t = _retTree;
		stmts_opt(_t);
		_t = _retTree;
		_t = __t420;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::exception_handler(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST exception_handler_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t439 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp259_AST_in = _t;
		match(_t,EXCEPTION_HANDLER);
		_t = _t->getFirstChild();
		identifier_colon_opt(_t);
		_t = _retTree;
		except_choice_s(_t);
		_t = _retTree;
		statements(_t);
		_t = _retTree;
		_t = __t439;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::identifier_colon_opt(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST identifier_colon_opt_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t441 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp260_AST_in = _t;
		match(_t,IDENTIFIER_COLON_OPT);
		_t = _t->getFirstChild();
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENTIFIER:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp261_AST_in = _t;
			match(_t,IDENTIFIER);
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
		_t = __t441;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::except_choice_s(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST except_choice_s_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case PIPE:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t444 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp262_AST_in = _t;
			match(_t,PIPE);
			_t = _t->getFirstChild();
			except_choice_s(_t);
			_t = _retTree;
			exception_choice(_t);
			_t = _retTree;
			_t = __t444;
			_t = _t->getNextSibling();
			break;
		}
		case IDENTIFIER:
		case DOT:
		case OTHERS:
		{
			exception_choice(_t);
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

void AdaTreeParserSuper::exception_choice(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST exception_choice_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENTIFIER:
		case DOT:
		{
			compound_name(_t);
			_t = _retTree;
			break;
		}
		case OTHERS:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp263_AST_in = _t;
			match(_t,OTHERS);
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

void AdaTreeParserSuper::operator_call(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST operator_call_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t453 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp264_AST_in = _t;
		match(_t,OPERATOR_SYMBOL);
		_t = _t->getFirstChild();
		value_s(_t);
		_t = _retTree;
		_t = __t453;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::relation(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST relation_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IN:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t465 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp265_AST_in = _t;
			match(_t,IN);
			_t = _t->getFirstChild();
			simple_expression(_t);
			_t = _retTree;
			range_or_mark(_t);
			_t = _retTree;
			_t = __t465;
			_t = _t->getNextSibling();
			break;
		}
		case NOT_IN:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t466 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp266_AST_in = _t;
			match(_t,NOT_IN);
			_t = _t->getFirstChild();
			simple_expression(_t);
			_t = _retTree;
			range_or_mark(_t);
			_t = _retTree;
			_t = __t466;
			_t = _t->getNextSibling();
			break;
		}
		case EQ:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t467 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp267_AST_in = _t;
			match(_t,EQ);
			_t = _t->getFirstChild();
			simple_expression(_t);
			_t = _retTree;
			simple_expression(_t);
			_t = _retTree;
			_t = __t467;
			_t = _t->getNextSibling();
			break;
		}
		case NE:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t468 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp268_AST_in = _t;
			match(_t,NE);
			_t = _t->getFirstChild();
			simple_expression(_t);
			_t = _retTree;
			simple_expression(_t);
			_t = _retTree;
			_t = __t468;
			_t = _t->getNextSibling();
			break;
		}
		case LT_:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t469 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp269_AST_in = _t;
			match(_t,LT_);
			_t = _t->getFirstChild();
			simple_expression(_t);
			_t = _retTree;
			simple_expression(_t);
			_t = _retTree;
			_t = __t469;
			_t = _t->getNextSibling();
			break;
		}
		case LE:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t470 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp270_AST_in = _t;
			match(_t,LE);
			_t = _t->getFirstChild();
			simple_expression(_t);
			_t = _retTree;
			simple_expression(_t);
			_t = _retTree;
			_t = __t470;
			_t = _t->getNextSibling();
			break;
		}
		case GT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t471 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp271_AST_in = _t;
			match(_t,GT);
			_t = _t->getFirstChild();
			simple_expression(_t);
			_t = _retTree;
			simple_expression(_t);
			_t = _retTree;
			_t = __t471;
			_t = _t->getNextSibling();
			break;
		}
		case GE:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t472 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp272_AST_in = _t;
			match(_t,GE);
			_t = _t->getFirstChild();
			simple_expression(_t);
			_t = _retTree;
			simple_expression(_t);
			_t = _retTree;
			_t = __t472;
			_t = _t->getNextSibling();
			break;
		}
		case IDENTIFIER:
		case DOT:
		case TIC:
		case CHARACTER_LITERAL:
		case CHAR_STRING:
		case NuLL:
		case MOD:
		case NOT:
		case PLUS:
		case MINUS:
		case CONCAT:
		case STAR:
		case DIV:
		case REM:
		case ABS:
		case EXPON:
		case NUMERIC_LIT:
		case ALLOCATOR:
		case INDEXED_COMPONENT:
		case OPERATOR_SYMBOL:
		case PARENTHESIZED_PRIMARY:
		case UNARY_MINUS:
		case UNARY_PLUS:
		{
			simple_expression(_t);
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

void AdaTreeParserSuper::range_or_mark(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST range_or_mark_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case DOT_DOT:
		case RANGE_ATTRIBUTE_REFERENCE:
		{
			range(_t);
			_t = _retTree;
			break;
		}
		case IDENTIFIER:
		case DOT:
		case TIC:
		{
			subtype_mark(_t);
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

void AdaTreeParserSuper::signed_term(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST signed_term_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case UNARY_PLUS:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t479 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp273_AST_in = _t;
			match(_t,UNARY_PLUS);
			_t = _t->getFirstChild();
			term(_t);
			_t = _retTree;
			_t = __t479;
			_t = _t->getNextSibling();
			break;
		}
		case UNARY_MINUS:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t480 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp274_AST_in = _t;
			match(_t,UNARY_MINUS);
			_t = _t->getFirstChild();
			term(_t);
			_t = _retTree;
			_t = __t480;
			_t = _t->getNextSibling();
			break;
		}
		case IDENTIFIER:
		case DOT:
		case TIC:
		case CHARACTER_LITERAL:
		case CHAR_STRING:
		case NuLL:
		case MOD:
		case NOT:
		case STAR:
		case DIV:
		case REM:
		case ABS:
		case EXPON:
		case NUMERIC_LIT:
		case ALLOCATOR:
		case INDEXED_COMPONENT:
		case OPERATOR_SYMBOL:
		case PARENTHESIZED_PRIMARY:
		{
			term(_t);
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

void AdaTreeParserSuper::term(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST term_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case STAR:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t482 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp275_AST_in = _t;
			match(_t,STAR);
			_t = _t->getFirstChild();
			term(_t);
			_t = _retTree;
			factor(_t);
			_t = _retTree;
			_t = __t482;
			_t = _t->getNextSibling();
			break;
		}
		case DIV:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t483 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp276_AST_in = _t;
			match(_t,DIV);
			_t = _t->getFirstChild();
			term(_t);
			_t = _retTree;
			factor(_t);
			_t = _retTree;
			_t = __t483;
			_t = _t->getNextSibling();
			break;
		}
		case MOD:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t484 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp277_AST_in = _t;
			match(_t,MOD);
			_t = _t->getFirstChild();
			term(_t);
			_t = _retTree;
			factor(_t);
			_t = _retTree;
			_t = __t484;
			_t = _t->getNextSibling();
			break;
		}
		case REM:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t485 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp278_AST_in = _t;
			match(_t,REM);
			_t = _t->getFirstChild();
			term(_t);
			_t = _retTree;
			factor(_t);
			_t = _retTree;
			_t = __t485;
			_t = _t->getNextSibling();
			break;
		}
		case IDENTIFIER:
		case DOT:
		case TIC:
		case CHARACTER_LITERAL:
		case CHAR_STRING:
		case NuLL:
		case NOT:
		case ABS:
		case EXPON:
		case NUMERIC_LIT:
		case ALLOCATOR:
		case INDEXED_COMPONENT:
		case OPERATOR_SYMBOL:
		case PARENTHESIZED_PRIMARY:
		{
			factor(_t);
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

void AdaTreeParserSuper::factor(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST factor_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case NOT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t487 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp279_AST_in = _t;
			match(_t,NOT);
			_t = _t->getFirstChild();
			primary(_t);
			_t = _retTree;
			_t = __t487;
			_t = _t->getNextSibling();
			break;
		}
		case ABS:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t488 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp280_AST_in = _t;
			match(_t,ABS);
			_t = _t->getFirstChild();
			primary(_t);
			_t = _retTree;
			_t = __t488;
			_t = _t->getNextSibling();
			break;
		}
		case EXPON:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t489 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp281_AST_in = _t;
			match(_t,EXPON);
			_t = _t->getFirstChild();
			primary(_t);
			_t = _retTree;
			primary(_t);
			_t = _retTree;
			_t = __t489;
			_t = _t->getNextSibling();
			break;
		}
		case IDENTIFIER:
		case DOT:
		case TIC:
		case CHARACTER_LITERAL:
		case CHAR_STRING:
		case NuLL:
		case NUMERIC_LIT:
		case ALLOCATOR:
		case INDEXED_COMPONENT:
		case OPERATOR_SYMBOL:
		case PARENTHESIZED_PRIMARY:
		{
			primary(_t);
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

void AdaTreeParserSuper::primary(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST primary_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		{
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENTIFIER:
		case DOT:
		case TIC:
		case INDEXED_COMPONENT:
		{
			name_or_qualified(_t);
			_t = _retTree;
			break;
		}
		case PARENTHESIZED_PRIMARY:
		{
			parenthesized_primary(_t);
			_t = _retTree;
			break;
		}
		case ALLOCATOR:
		{
			allocator(_t);
			_t = _retTree;
			break;
		}
		case NuLL:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp282_AST_in = _t;
			match(_t,NuLL);
			_t = _t->getNextSibling();
			break;
		}
		case NUMERIC_LIT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp283_AST_in = _t;
			match(_t,NUMERIC_LIT);
			_t = _t->getNextSibling();
			break;
		}
		case CHARACTER_LITERAL:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp284_AST_in = _t;
			match(_t,CHARACTER_LITERAL);
			_t = _t->getNextSibling();
			break;
		}
		case CHAR_STRING:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp285_AST_in = _t;
			match(_t,CHAR_STRING);
			_t = _t->getNextSibling();
			break;
		}
		case OPERATOR_SYMBOL:
		{
			operator_call(_t);
			_t = _retTree;
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
		}
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

void AdaTreeParserSuper::name_or_qualified(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST name_or_qualified_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENTIFIER:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp286_AST_in = _t;
			match(_t,IDENTIFIER);
			_t = _t->getNextSibling();
			break;
		}
		case DOT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t493 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp287_AST_in = _t;
			match(_t,DOT);
			_t = _t->getFirstChild();
			name_or_qualified(_t);
			_t = _retTree;
			{
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case ALL:
			{
				ANTLR_USE_NAMESPACE(antlr)RefAST tmp288_AST_in = _t;
				match(_t,ALL);
				_t = _t->getNextSibling();
				break;
			}
			case IDENTIFIER:
			{
				ANTLR_USE_NAMESPACE(antlr)RefAST tmp289_AST_in = _t;
				match(_t,IDENTIFIER);
				_t = _t->getNextSibling();
				break;
			}
			case CHARACTER_LITERAL:
			{
				ANTLR_USE_NAMESPACE(antlr)RefAST tmp290_AST_in = _t;
				match(_t,CHARACTER_LITERAL);
				_t = _t->getNextSibling();
				break;
			}
			case OPERATOR_SYMBOL:
			{
				ANTLR_USE_NAMESPACE(antlr)RefAST tmp291_AST_in = _t;
				match(_t,OPERATOR_SYMBOL);
				_t = _t->getNextSibling();
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
			}
			}
			}
			_t = __t493;
			_t = _t->getNextSibling();
			break;
		}
		case INDEXED_COMPONENT:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t495 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp292_AST_in = _t;
			match(_t,INDEXED_COMPONENT);
			_t = _t->getFirstChild();
			name_or_qualified(_t);
			_t = _retTree;
			value_s(_t);
			_t = _retTree;
			_t = __t495;
			_t = _t->getNextSibling();
			break;
		}
		case TIC:
		{
			ANTLR_USE_NAMESPACE(antlr)RefAST __t496 = _t;
			ANTLR_USE_NAMESPACE(antlr)RefAST tmp293_AST_in = _t;
			match(_t,TIC);
			_t = _t->getFirstChild();
			name_or_qualified(_t);
			_t = _retTree;
			{
			if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case PARENTHESIZED_PRIMARY:
			{
				parenthesized_primary(_t);
				_t = _retTree;
				break;
			}
			case IDENTIFIER:
			case RANGE:
			case DIGITS:
			case DELTA:
			case ACCESS:
			{
				attribute_id(_t);
				_t = _retTree;
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(_t);
			}
			}
			}
			_t = __t496;
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

void AdaTreeParserSuper::allocator(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST allocator_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t499 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp294_AST_in = _t;
		match(_t,ALLOCATOR);
		_t = _t->getFirstChild();
		name_or_qualified(_t);
		_t = _retTree;
		_t = __t499;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::subprogram_body(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST subprogram_body_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		if (_t == ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case PROCEDURE_BODY:
		{
			procedure_body(_t);
			_t = _retTree;
			break;
		}
		case FUNCTION_BODY:
		{
			function_body(_t);
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

void AdaTreeParserSuper::package_body(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST package_body_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t505 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp295_AST_in = _t;
		match(_t,PACKAGE_BODY);
		_t = _t->getFirstChild();
		def_id(_t);
		_t = _retTree;
		pkg_body_part(_t);
		_t = _retTree;
		_t = __t505;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::task_body(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST task_body_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t507 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp296_AST_in = _t;
		match(_t,TASK_BODY);
		_t = _t->getFirstChild();
		def_id(_t);
		_t = _retTree;
		body_part(_t);
		_t = _retTree;
		_t = __t507;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::protected_body(ANTLR_USE_NAMESPACE(antlr)RefAST _t) {
	ANTLR_USE_NAMESPACE(antlr)RefAST protected_body_AST_in = (_t == ANTLR_USE_NAMESPACE(antlr)RefAST(ASTNULL)) ? ANTLR_USE_NAMESPACE(antlr)nullAST : _t;
	
	try {      // for error handling
		ANTLR_USE_NAMESPACE(antlr)RefAST __t509 = _t;
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp297_AST_in = _t;
		match(_t,PROTECTED_BODY);
		_t = _t->getFirstChild();
		def_id(_t);
		_t = _retTree;
		prot_op_bodies_opt(_t);
		_t = _retTree;
		_t = __t509;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != ANTLR_USE_NAMESPACE(antlr)nullAST )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaTreeParserSuper::initializeASTFactory( ANTLR_USE_NAMESPACE(antlr)ASTFactory& )
{
}
const char* AdaTreeParserSuper::tokenNames[] = {
	"<0>",
	"EOF",
	"<2>",
	"NULL_TREE_LOOKAHEAD",
	"\"pragma\"",
	"IDENTIFIER",
	"SEMI",
	"LPAREN",
	"COMMA",
	"RPAREN",
	"RIGHT_SHAFT",
	"\"with\"",
	"DOT",
	"\"use\"",
	"\"type\"",
	"TIC",
	"\"range\"",
	"\"digits\"",
	"\"delta\"",
	"\"access\"",
	"\"private\"",
	"\"package\"",
	"\"body\"",
	"\"is\"",
	"\"procedure\"",
	"\"function\"",
	"\"new\"",
	"\"others\"",
	"PIPE",
	"DOT_DOT",
	"\"all\"",
	"COLON",
	"\"in\"",
	"\"out\"",
	"\"renames\"",
	"CHARACTER_LITERAL",
	"CHAR_STRING",
	"\"null\"",
	"\"record\"",
	"\"separate\"",
	"\"abstract\"",
	"\"return\"",
	"\"task\"",
	"\"protected\"",
	"BOX",
	"ASSIGN",
	"\"entry\"",
	"\"for\"",
	"\"end\"",
	"\"at\"",
	"\"mod\"",
	"\"subtype\"",
	"\"exception\"",
	"\"constant\"",
	"\"array\"",
	"\"of\"",
	"\"aliased\"",
	"\"case\"",
	"\"when\"",
	"\"tagged\"",
	"\"limited\"",
	"\"generic\"",
	"\"begin\"",
	"LT_LT",
	"GT_GT",
	"\"if\"",
	"\"then\"",
	"\"elsif\"",
	"\"else\"",
	"\"loop\"",
	"\"while\"",
	"\"reverse\"",
	"\"declare\"",
	"\"exit\"",
	"\"goto\"",
	"\"accept\"",
	"\"do\"",
	"\"delay\"",
	"\"until\"",
	"\"select\"",
	"\"abort\"",
	"\"or\"",
	"\"terminate\"",
	"\"raise\"",
	"\"requeue\"",
	"\"and\"",
	"\"xor\"",
	"\"not\"",
	"EQ",
	"NE",
	"LT_",
	"LE",
	"GT",
	"GE",
	"PLUS",
	"MINUS",
	"CONCAT",
	"STAR",
	"DIV",
	"\"rem\"",
	"\"abs\"",
	"EXPON",
	"NUMERIC_LIT",
	"ABORTABLE_PART",
	"ABORT_STATEMENT",
	"ABSTRACT_SUBPROGRAM_DECLARATION",
	"ACCEPT_ALTERNATIVE",
	"ACCEPT_STATEMENT",
	"ALLOCATOR",
	"ASSIGNMENT_STATEMENT",
	"ASYNCHRONOUS_SELECT",
	"ATTRIBUTE_DEFINITION_CLAUSE",
	"AT_CLAUSE",
	"BLOCK_STATEMENT",
	"CASE_STATEMENT",
	"CASE_STATEMENT_ALTERNATIVE",
	"CODE_STATEMENT",
	"COMPONENT_DECLARATION",
	"COMPONENT_LIST",
	"CONDITION",
	"CONDITIONAL_ENTRY_CALL",
	"CONTEXT_CLAUSE",
	"DECLARATIVE_ITEM",
	"DECLARATIVE_PART",
	"DEFINING_IDENTIFIER_LIST",
	"DELAY_ALTERNATIVE",
	"DELAY_STATEMENT",
	"DELTA_CONSTRAINT",
	"DIGITS_CONSTRAINT",
	"DISCRETE_RANGE",
	"DISCRIMINANT_ASSOCIATION",
	"DISCRIMINANT_CONSTRAINT",
	"DISCRIMINANT_SPECIFICATION",
	"ENTRY_BODY",
	"ENTRY_CALL_ALTERNATIVE",
	"ENTRY_CALL_STATEMENT",
	"ENTRY_DECLARATION",
	"ENTRY_INDEX_SPECIFICATION",
	"ENUMERATION_REPESENTATION_CLAUSE",
	"EXCEPTION_DECLARATION",
	"EXCEPTION_HANDLER",
	"EXCEPTION_RENAMING_DECLARATION",
	"EXIT_STATEMENT",
	"FORMAL_PACKAGE_DECLARATION",
	"FORMAL_TYPE_DECLARATION",
	"FULL_TYPE_DECLARATION",
	"GENERIC_FORMAL_PART",
	"GENERIC_INSTANTIATION",
	"GENERIC_PACKAGE_DECLARATION",
	"GENERIC_RENAMING_DECLARATION",
	"GENERIC_SUBPROGRAM_DECLARATION",
	"GOTO_STATEMENT",
	"HANDLED_SEQUENCE_OF_STATEMENTS",
	"IF_STATEMENT",
	"INCOMPLETE_TYPE_DECLARATION",
	"INDEXED_COMPONENT",
	"INDEX_CONSTRAINT",
	"LIBRARY_ITEM",
	"LOOP_STATEMENT",
	"NAME",
	"NULL_STATEMENT",
	"NUMBER_DECLARATION",
	"OBJECT_DECLARATION",
	"OBJECT_RENAMING_DECLARATION",
	"OPERATOR_SYMBOL",
	"PACKAGE_BODY",
	"PACKAGE_BODY_STUB",
	"PACKAGE_RENAMING_DECLARATION",
	"PACKAGE_SPECIFICATION",
	"PARAMETER_SPECIFICATION",
	"PREFIX",
	"PRIMARY",
	"PRIVATE_EXTENSION_DECLARATION",
	"PRIVATE_TYPE_DECLARATION",
	"PROCEDURE_CALL_STATEMENT",
	"PROTECTED_BODY",
	"PROTECTED_BODY_STUB",
	"PROTECTED_TYPE_DECLARATION",
	"RAISE_STATEMENT",
	"RANGE_ATTRIBUTE_REFERENCE",
	"RECORD_REPRESENTATION_CLAUSE",
	"REQUEUE_STATEMENT",
	"RETURN_STATEMENT",
	"SELECTIVE_ACCEPT",
	"SELECT_ALTERNATIVE",
	"SELECT_STATEMENT",
	"SEQUENCE_OF_STATEMENTS",
	"SINGLE_PROTECTED_DECLARATION",
	"SINGLE_TASK_DECLARATION",
	"STATEMENT",
	"SUBPROGRAM_BODY",
	"SUBPROGRAM_BODY_STUB",
	"SUBPROGRAM_DECLARATION",
	"SUBPROGRAM_RENAMING_DECLARATION",
	"SUBTYPE_DECLARATION",
	"SUBTYPE_INDICATION",
	"SUBTYPE_MARK",
	"SUBUNIT",
	"TASK_BODY",
	"TASK_BODY_STUB",
	"TASK_TYPE_DECLARATION",
	"TERMINATE_ALTERNATIVE",
	"TIMED_ENTRY_CALL",
	"TRIGGERING_ALTERNATIVE",
	"TYPE_DECLARATION",
	"USE_CLAUSE",
	"USE_TYPE_CLAUSE",
	"VARIANT",
	"VARIANT_PART",
	"WITH_CLAUSE",
	"ABSTRACT_FUNCTION_DECLARATION",
	"ABSTRACT_PROCEDURE_DECLARATION",
	"ACCESS_TO_FUNCTION_DECLARATION",
	"ACCESS_TO_OBJECT_DECLARATION",
	"ACCESS_TO_PROCEDURE_DECLARATION",
	"ACCESS_TYPE_DECLARATION",
	"ARRAY_OBJECT_DECLARATION",
	"ARRAY_TYPE_DECLARATION",
	"AND_THEN",
	"BASIC_DECLARATIVE_ITEMS_OPT",
	"BLOCK_BODY",
	"BLOCK_BODY_OPT",
	"CALL_STATEMENT",
	"COMPONENT_CLAUSES_OPT",
	"COMPONENT_ITEMS",
	"COND_CLAUSE",
	"DECIMAL_FIXED_POINT_DECLARATION",
	"DECLARE_OPT",
	"DERIVED_RECORD_EXTENSION",
	"DERIVED_TYPE_DECLARATION",
	"DISCRETE_SUBTYPE_DEF_OPT",
	"DISCRIMINANT_SPECIFICATIONS",
	"DISCRIM_PART_OPT",
	"ELSE_OPT",
	"ELSIFS_OPT",
	"ENTRY_INDEX_OPT",
	"ENUMERATION_TYPE_DECLARATION",
	"EXCEPT_HANDLER_PART_OPT",
	"EXTENSION_OPT",
	"FLOATING_POINT_DECLARATION",
	"FORMAL_DECIMAL_FIXED_POINT_DECLARATION",
	"FORMAL_DISCRETE_TYPE_DECLARATION",
	"FORMAL_FLOATING_POINT_DECLARATION",
	"FORMAL_FUNCTION_DECLARATION",
	"FORMAL_MODULAR_TYPE_DECLARATION",
	"FORMAL_ORDINARY_DERIVED_TYPE_DECLARATION",
	"FORMAL_ORDINARY_FIXED_POINT_DECLARATION",
	"FORMAL_PART_OPT",
	"FORMAL_PRIVATE_EXTENSION_DECLARATION",
	"FORMAL_PRIVATE_TYPE_DECLARATION",
	"FORMAL_PROCEDURE_DECLARATION",
	"FORMAL_SIGNED_INTEGER_TYPE_DECLARATION",
	"FUNCTION_BODY",
	"FUNCTION_BODY_STUB",
	"FUNCTION_DECLARATION",
	"FUNCTION_RENAMING_DECLARATION",
	"GENERIC_FUNCTION_DECLARATION",
	"GENERIC_FUNCTION_INSTANTIATION",
	"GENERIC_FUNCTION_RENAMING",
	"GENERIC_PACKAGE_INSTANTIATION",
	"GENERIC_PACKAGE_RENAMING",
	"GENERIC_PROCEDURE_DECLARATION",
	"GENERIC_PROCEDURE_INSTANTIATION",
	"GENERIC_PROCEDURE_RENAMING",
	"GUARD_OPT",
	"IDENTIFIER_COLON_OPT",
	"INIT_OPT",
	"ITERATION_SCHEME_OPT",
	"LABEL_OPT",
	"MARK_WITH_CONSTRAINT",
	"MODIFIERS",
	"MODULAR_TYPE_DECLARATION",
	"MOD_CLAUSE_OPT",
	"NOT_IN",
	"ORDINARY_DERIVED_TYPE_DECLARATION",
	"ORDINARY_FIXED_POINT_DECLARATION",
	"OR_ELSE",
	"OR_SELECT_OPT",
	"PARENTHESIZED_PRIMARY",
	"PRIVATE_TASK_ITEMS_OPT",
	"PROCEDURE_BODY",
	"PROCEDURE_BODY_STUB",
	"PROCEDURE_DECLARATION",
	"PROCEDURE_RENAMING_DECLARATION",
	"PROT_MEMBER_DECLARATIONS",
	"PROT_OP_BODIES_OPT",
	"PROT_OP_DECLARATIONS",
	"RANGED_EXPRS",
	"RECORD_TYPE_DECLARATION",
	"SELECTOR_NAMES_OPT",
	"SIGNED_INTEGER_TYPE_DECLARATION",
	"TASK_ITEMS_OPT",
	"UNARY_MINUS",
	"UNARY_PLUS",
	"VALUE",
	"VALUES",
	"VARIANTS",
	"COMMENT_INTRO",
	"DIGIT",
	"EXPONENT",
	"EXTENDED_DIGIT",
	"BASED_INTEGER",
	"WS_",
	"COMMENT",
	"CHARACTER_STRING",
	0
};

const unsigned long AdaTreeParserSuper::_tokenSet_0_data_[] = { 37920UL, 262201UL, 4293001216UL, 4223UL, 134217728UL, 16UL, 67108864UL, 0UL, 5373952UL, 48UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER RIGHT_SHAFT DOT TIC "in" CHARACTER_LITERAL CHAR_STRING "null" 
// "mod" "or" "and" "xor" "not" EQ NE LT_ LE GT GE PLUS MINUS CONCAT STAR 
// DIV "rem" "abs" EXPON NUMERIC_LIT ALLOCATOR INDEXED_COMPONENT OPERATOR_SYMBOL 
// AND_THEN NOT_IN OR_ELSE PARENTHESIZED_PRIMARY UNARY_MINUS UNARY_PLUS 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaTreeParserSuper::_tokenSet_0(_tokenSet_0_data_,20);
const unsigned long AdaTreeParserSuper::_tokenSet_1_data_[] = { 939627552UL, 262201UL, 4293001216UL, 4223UL, 134217728UL, 16UL, 67108864UL, 0UL, 5373952UL, 48UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER RIGHT_SHAFT DOT TIC "range" "others" PIPE DOT_DOT "in" CHARACTER_LITERAL 
// CHAR_STRING "null" "mod" "or" "and" "xor" "not" EQ NE LT_ LE GT GE PLUS 
// MINUS CONCAT STAR DIV "rem" "abs" EXPON NUMERIC_LIT ALLOCATOR INDEXED_COMPONENT 
// OPERATOR_SYMBOL AND_THEN NOT_IN OR_ELSE PARENTHESIZED_PRIMARY UNARY_MINUS 
// UNARY_PLUS 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaTreeParserSuper::_tokenSet_1(_tokenSet_1_data_,20);
const unsigned long AdaTreeParserSuper::_tokenSet_2_data_[] = { 0UL, 0UL, 0UL, 98304UL, 68168704UL, 403845518UL, 58482948UL, 3758133268UL, 235700479UL, 5UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// ATTRIBUTE_DEFINITION_CLAUSE AT_CLAUSE ENUMERATION_REPESENTATION_CLAUSE 
// EXCEPTION_DECLARATION EXCEPTION_RENAMING_DECLARATION GENERIC_PACKAGE_DECLARATION 
// INCOMPLETE_TYPE_DECLARATION NUMBER_DECLARATION OBJECT_DECLARATION OBJECT_RENAMING_DECLARATION 
// PACKAGE_RENAMING_DECLARATION PACKAGE_SPECIFICATION PRIVATE_EXTENSION_DECLARATION 
// PRIVATE_TYPE_DECLARATION PROTECTED_TYPE_DECLARATION RECORD_REPRESENTATION_CLAUSE 
// SINGLE_PROTECTED_DECLARATION SINGLE_TASK_DECLARATION SUBTYPE_DECLARATION 
// TASK_TYPE_DECLARATION USE_CLAUSE USE_TYPE_CLAUSE ABSTRACT_FUNCTION_DECLARATION 
// ABSTRACT_PROCEDURE_DECLARATION ACCESS_TO_FUNCTION_DECLARATION ACCESS_TO_OBJECT_DECLARATION 
// ACCESS_TO_PROCEDURE_DECLARATION ARRAY_OBJECT_DECLARATION ARRAY_TYPE_DECLARATION 
// DECIMAL_FIXED_POINT_DECLARATION DERIVED_RECORD_EXTENSION ENUMERATION_TYPE_DECLARATION 
// FLOATING_POINT_DECLARATION FUNCTION_BODY_STUB FUNCTION_DECLARATION FUNCTION_RENAMING_DECLARATION 
// GENERIC_FUNCTION_DECLARATION GENERIC_FUNCTION_INSTANTIATION GENERIC_FUNCTION_RENAMING 
// GENERIC_PACKAGE_INSTANTIATION GENERIC_PACKAGE_RENAMING GENERIC_PROCEDURE_DECLARATION 
// GENERIC_PROCEDURE_INSTANTIATION GENERIC_PROCEDURE_RENAMING MODULAR_TYPE_DECLARATION 
// ORDINARY_DERIVED_TYPE_DECLARATION ORDINARY_FIXED_POINT_DECLARATION PROCEDURE_BODY_STUB 
// PROCEDURE_DECLARATION PROCEDURE_RENAMING_DECLARATION RECORD_TYPE_DECLARATION 
// SIGNED_INTEGER_TYPE_DECLARATION 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaTreeParserSuper::_tokenSet_2(_tokenSet_2_data_,20);
const unsigned long AdaTreeParserSuper::_tokenSet_3_data_[] = { 16UL, 0UL, 0UL, 98304UL, 1280UL, 1048576UL, 0UL, 1073741824UL, 67108864UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "pragma" ATTRIBUTE_DEFINITION_CLAUSE AT_CLAUSE ENTRY_DECLARATION ENUMERATION_REPESENTATION_CLAUSE 
// RECORD_REPRESENTATION_CLAUSE FUNCTION_DECLARATION PROCEDURE_DECLARATION 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaTreeParserSuper::_tokenSet_3(_tokenSet_3_data_,20);
const unsigned long AdaTreeParserSuper::_tokenSet_4_data_[] = { 36896UL, 0UL, 0UL, 0UL, 134217728UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER DOT TIC INDEXED_COMPONENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaTreeParserSuper::_tokenSet_4(_tokenSet_4_data_,12);


