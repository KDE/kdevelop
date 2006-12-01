/* $ANTLR 2.7.2: "expandedada.store.g" -> "AdaStoreWalker.cpp"$ */
#include "AdaStoreWalker.hpp"
#include <antlr/Token.hpp>
#include <antlr/AST.hpp>
#include <antlr/NoViableAltException.hpp>
#include <antlr/MismatchedTokenException.hpp>
#include <antlr/SemanticException.hpp>
#include <antlr/BitSet.hpp>
#line 1 "expandedada.store.g"
#line 11 "AdaStoreWalker.cpp"
AdaStoreWalker::AdaStoreWalker()
	: ANTLR_USE_NAMESPACE(antlr)TreeParser() {
}

void AdaStoreWalker::compilation_unit(RefAdaAST _t) {
	RefAdaAST compilation_unit_AST_in = _t;

	try {      // for error handling
#line 116 "expandedada.store.g"
		init();
#line 22 "AdaStoreWalker.cpp"
		context_items_opt(_t);
		_t = _retTree;
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
		{ // ( ... )*
		for (;;) {
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::context_items_opt(RefAdaAST _t) {
	RefAdaAST context_items_opt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t67 = _t;
		RefAdaAST tmp1_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),CONTEXT_CLAUSE);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
				goto _loop69;
			}
			}
		}
		_loop69:;
		} // ( ... )*
		_t = __t67;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::library_item(RefAdaAST _t) {
	RefAdaAST library_item_AST_in = _t;
	RefAdaAST pb = static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	RefAdaAST gpi = static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	RefAdaAST ps = static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());
	RefAdaAST prd = static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());

	try {      // for error handling
		RefAdaAST __t17 = _t;
		RefAdaAST tmp2_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),LIBRARY_ITEM);
		_t = _t->getFirstChild();
		RefAdaAST __t18 = _t;
		RefAdaAST tmp3_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),MODIFIERS);
		_t = _t->getFirstChild();
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case PRIVATE:
		{
			RefAdaAST tmp4_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PRIVATE);
			_t = _t->getNextSibling();
#line 132 "expandedada.store.g"
			m_currentAccess = CodeModelItem::Protected;
#line 149 "AdaStoreWalker.cpp"
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
		_t = __t18;
		_t = _t->getNextSibling();
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			lib_subprog_decl_or_rename_or_inst_or_body(_t);
			_t = _retTree;
			break;
		}
		case PACKAGE_BODY:
		{
			RefAdaAST __t21 = _t;
			RefAdaAST tmp5_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PACKAGE_BODY);
			_t = _t->getFirstChild();
			pb = (_t == ASTNULL) ? static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) : _t;
			def_id(_t);
			_t = _retTree;
			pkg_body_part(_t);
			_t = _retTree;
			_t = __t21;
			_t = _t->getNextSibling();
			break;
		}
		case GENERIC_PACKAGE_INSTANTIATION:
		{
			RefAdaAST __t22 = _t;
			RefAdaAST tmp6_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),GENERIC_PACKAGE_INSTANTIATION);
			_t = _t->getFirstChild();
			gpi = (_t == ASTNULL) ? static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) : _t;
			def_id(_t);
			_t = _retTree;
#line 136 "expandedada.store.g"

					       defineScope( gpi );

#line 213 "AdaStoreWalker.cpp"
			generic_inst(_t);
			_t = _retTree;
			_t = __t22;
			_t = _t->getNextSibling();
			break;
		}
		case PACKAGE_SPECIFICATION:
		{
			RefAdaAST __t23 = _t;
			RefAdaAST tmp7_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PACKAGE_SPECIFICATION);
			_t = _t->getFirstChild();
			ps = (_t == ASTNULL) ? static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) : _t;
			def_id(_t);
			_t = _retTree;
#line 142 "expandedada.store.g"

					       NamespaceDom psc = defineScope( ps );
					       m_currentContainer = psc;
					       m_scopeStack.append( psc );
					       m_addToStore = true;

#line 236 "AdaStoreWalker.cpp"
			pkg_spec_part(_t);
			_t = _retTree;
#line 149 "expandedada.store.g"

					       m_scopeStack.remove(m_scopeStack.last());
					       if (m_scopeStack.count() == 0) {
						 kdDebug() << "adastore: m_scopeStack is empty!" << endl;
					         m_scopeStack.append( m_model->globalNamespace() );
					       }
					       m_currentContainer = m_scopeStack.last();
					       // m_currentContainer->setEndPosition (endLine, 0);
					       m_addToStore = false;

#line 250 "AdaStoreWalker.cpp"
			_t = __t23;
			_t = _t->getNextSibling();
			break;
		}
		case PACKAGE_RENAMING_DECLARATION:
		{
			RefAdaAST __t24 = _t;
			RefAdaAST tmp8_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PACKAGE_RENAMING_DECLARATION);
			_t = _t->getFirstChild();
			prd = (_t == ASTNULL) ? static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) : _t;
			def_id(_t);
			_t = _retTree;
#line 161 "expandedada.store.g"

					       defineScope( prd );

#line 268 "AdaStoreWalker.cpp"
			renames(_t);
			_t = _retTree;
			_t = __t24;
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
		_t = __t17;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::subunit(RefAdaAST _t) {
	RefAdaAST subunit_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t508 = _t;
		RefAdaAST tmp9_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),SUBUNIT);
		_t = _t->getFirstChild();
		compound_name(_t);
		_t = _retTree;
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
		_t = __t508;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::pragma(RefAdaAST _t) {
	RefAdaAST pragma_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t61 = _t;
		RefAdaAST tmp10_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PRAGMA);
		_t = _t->getFirstChild();
		RefAdaAST tmp11_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
		_t = _t->getNextSibling();
		{ // ( ... )*
		for (;;) {
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_tokenSet_0.member(_t->getType()))) {
				pragma_arg(_t);
				_t = _retTree;
			}
			else {
				goto _loop63;
			}

		}
		_loop63:;
		} // ( ... )*
		_t = __t61;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::with_clause(RefAdaAST _t) {
	RefAdaAST with_clause_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t6 = _t;
		RefAdaAST tmp12_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),WITH_CLAUSE);
		_t = _t->getFirstChild();
		{ // ( ... )+
		int _cnt8=0;
		for (;;) {
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_t->getType() == IDENTIFIER || _t->getType() == DOT)) {
				compound_name(_t);
				_t = _retTree;
			}
			else {
				if ( _cnt8>=1 ) { goto _loop8; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));}
			}

			_cnt8++;
		}
		_loop8:;
		}  // ( ... )+
		_t = __t6;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::compound_name(RefAdaAST _t) {
	RefAdaAST compound_name_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENTIFIER:
		{
			RefAdaAST tmp13_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
			_t = _t->getNextSibling();
			break;
		}
		case DOT:
		{
			RefAdaAST __t71 = _t;
			RefAdaAST tmp14_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),DOT);
			_t = _t->getFirstChild();
			compound_name(_t);
			_t = _retTree;
			RefAdaAST tmp15_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
			_t = _t->getNextSibling();
			_t = __t71;
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
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::use_clause(RefAdaAST _t) {
	RefAdaAST use_clause_AST_in = _t;
	RefAdaAST c = static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case USE_TYPE_CLAUSE:
		{
			RefAdaAST __t10 = _t;
			RefAdaAST tmp16_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),USE_TYPE_CLAUSE);
			_t = _t->getFirstChild();
			{ // ( ... )+
			int _cnt12=0;
			for (;;) {
				if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
					_t = ASTNULL;
				if ((_t->getType() == IDENTIFIER || _t->getType() == DOT || _t->getType() == TIC)) {
					subtype_mark(_t);
					_t = _retTree;
				}
				else {
					if ( _cnt12>=1 ) { goto _loop12; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));}
				}

				_cnt12++;
			}
			_loop12:;
			}  // ( ... )+
			_t = __t10;
			_t = _t->getNextSibling();
			break;
		}
		case USE_CLAUSE:
		{
			RefAdaAST __t13 = _t;
			RefAdaAST tmp17_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),USE_CLAUSE);
			_t = _t->getFirstChild();
			{ // ( ... )+
			int _cnt15=0;
			for (;;) {
				if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
					_t = ASTNULL;
				if ((_t->getType() == IDENTIFIER || _t->getType() == DOT)) {
					c = (_t == ASTNULL) ? static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) : _t;
					compound_name(_t);
					_t = _retTree;
#line 127 "expandedada.store.g"
					m_imports.back ().push_back (qtext (c));
#line 527 "AdaStoreWalker.cpp"
				}
				else {
					if ( _cnt15>=1 ) { goto _loop15; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));}
				}

				_cnt15++;
			}
			_loop15:;
			}  // ( ... )+
			_t = __t13;
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
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::subtype_mark(RefAdaAST _t) {
	RefAdaAST subtype_mark_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			RefAdaAST __t73 = _t;
			RefAdaAST tmp18_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),TIC);
			_t = _t->getFirstChild();
			compound_name(_t);
			_t = _retTree;
			attribute_id(_t);
			_t = _retTree;
			_t = __t73;
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
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::lib_subprog_decl_or_rename_or_inst_or_body(RefAdaAST _t) {
	RefAdaAST lib_subprog_decl_or_rename_or_inst_or_body_AST_in = _t;

	try {      // for error handling
#line 306 "expandedada.store.g"
		m_addToStore = true;
#line 603 "AdaStoreWalker.cpp"
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
#line 311 "expandedada.store.g"
		m_addToStore = false;
#line 643 "AdaStoreWalker.cpp"
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::def_id(RefAdaAST _t) {
	RefAdaAST def_id_AST_in = _t;
	RefAdaAST cn = static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());

	try {      // for error handling
		cn = (_t == ASTNULL) ? static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) : _t;
		compound_name(_t);
		_t = _retTree;
#line 189 "expandedada.store.g"

		//        kdDebug() << "cn:compound_name started " << endl;
			    if (m_addToStore) {
		//          kdDebug() << "cn:compound_name m_addToStore " << endl;
			      if (m_isSubprogram) {
		//            kdDebug() << "cn:compound_name m_isSubprogram " << endl;
		FunctionDom method = m_model->create<FunctionModel>();
			        method->setName (qtext (cn));
		method->setFileName(m_fileName);
		//            kdDebug() << "cn:compound_name method->setStartPosition(" << endl;
		method->setStartPosition(cn->getLine(), cn->getColumn());

		if (m_currentContainer == m_model->globalNamespace())
		m_file->addFunction(method);
		else
		m_currentContainer->addFunction(method);
		/// \FIXME adymo: is this valid for CodeModel
		/*	        ParsedMethod *old = m_currentContainer->getMethod (method);
			        if (old) {
			          delete (method);
			          method = old;
			        } else {
			          m_currentContainer->addMethod (method);
			        }*/
			      } else {
			        // TBC: what about other declarations?
			      }
			    }

#line 691 "AdaStoreWalker.cpp"
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::pkg_body_part(RefAdaAST _t) {
	RefAdaAST pkg_body_part_AST_in = _t;

	try {      // for error handling
		declarative_part(_t);
		_t = _retTree;
		block_body_opt(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::generic_inst(RefAdaAST _t) {
	RefAdaAST generic_inst_AST_in = _t;

	try {      // for error handling
		compound_name(_t);
		_t = _retTree;
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::pkg_spec_part(RefAdaAST _t) {
	RefAdaAST pkg_spec_part_AST_in = _t;

	try {      // for error handling
		basic_declarative_items_opt(_t);
		_t = _retTree;
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case BASIC_DECLARATIVE_ITEMS_OPT:
		{
#line 272 "expandedada.store.g"
			m_currentAccess = CodeModelItem::Protected;
#line 767 "AdaStoreWalker.cpp"
			basic_declarative_items_opt(_t);
			_t = _retTree;
#line 274 "expandedada.store.g"
			m_currentAccess = CodeModelItem::Public;
#line 772 "AdaStoreWalker.cpp"
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
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::renames(RefAdaAST _t) {
	RefAdaAST renames_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case CHARACTER_STRING:
		{
			RefAdaAST tmp19_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),CHARACTER_STRING);
			_t = _t->getNextSibling();
			break;
		}
		case OPERATOR_SYMBOL:
		{
			RefAdaAST tmp20_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),OPERATOR_SYMBOL);
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::generic_decl(RefAdaAST _t) {
	RefAdaAST generic_decl_AST_in = _t;
	RefAdaAST gpd = static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case GENERIC_PACKAGE_RENAMING:
		{
			RefAdaAST __t46 = _t;
			RefAdaAST tmp21_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),GENERIC_PACKAGE_RENAMING);
			_t = _t->getFirstChild();
			generic_formal_part_opt(_t);
			_t = _retTree;
			def_id(_t);
			_t = _retTree;
			renames(_t);
			_t = _retTree;
			_t = __t46;
			_t = _t->getNextSibling();
			break;
		}
		case GENERIC_PACKAGE_DECLARATION:
		{
			RefAdaAST __t47 = _t;
			RefAdaAST tmp22_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),GENERIC_PACKAGE_DECLARATION);
			_t = _t->getFirstChild();
			generic_formal_part_opt(_t);
			_t = _retTree;
			gpd = (_t == ASTNULL) ? static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) : _t;
			def_id(_t);
			_t = _retTree;
#line 280 "expandedada.store.g"

					       NamespaceDom psc = defineScope( gpd );
					       m_currentContainer = psc;
					       m_scopeStack.append( psc );
					       m_addToStore = true;

#line 880 "AdaStoreWalker.cpp"
			pkg_spec_part(_t);
			_t = _retTree;
#line 287 "expandedada.store.g"

					       m_scopeStack.remove(m_scopeStack.last());
					       if (m_scopeStack.count() == 0)
					         m_scopeStack.append( m_model->globalNamespace() );
					       m_currentContainer = m_scopeStack.last();
					       // m_currentContainer->setDeclarationEndsOnLine (endLine);
					       m_addToStore = false;

#line 892 "AdaStoreWalker.cpp"
			_t = __t47;
			_t = _t->getNextSibling();
			break;
		}
		case GENERIC_PROCEDURE_RENAMING:
		{
			RefAdaAST __t48 = _t;
			RefAdaAST tmp23_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),GENERIC_PROCEDURE_RENAMING);
			_t = _t->getFirstChild();
			generic_formal_part_opt(_t);
			_t = _retTree;
			def_id(_t);
			_t = _retTree;
			formal_part_opt(_t);
			_t = _retTree;
			renames(_t);
			_t = _retTree;
			_t = __t48;
			_t = _t->getNextSibling();
			break;
		}
		case GENERIC_PROCEDURE_DECLARATION:
		{
			RefAdaAST __t49 = _t;
			RefAdaAST tmp24_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),GENERIC_PROCEDURE_DECLARATION);
			_t = _t->getFirstChild();
			generic_formal_part_opt(_t);
			_t = _retTree;
			subprog_def_id(_t);
			_t = _retTree;
			formal_part_opt(_t);
			_t = _retTree;
			_t = __t49;
			_t = _t->getNextSibling();
			break;
		}
		case GENERIC_FUNCTION_RENAMING:
		{
			RefAdaAST __t50 = _t;
			RefAdaAST tmp25_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),GENERIC_FUNCTION_RENAMING);
			_t = _t->getFirstChild();
			generic_formal_part_opt(_t);
			_t = _retTree;
			def_designator(_t);
			_t = _retTree;
			function_tail(_t);
			_t = _retTree;
			renames(_t);
			_t = _retTree;
			_t = __t50;
			_t = _t->getNextSibling();
			break;
		}
		case GENERIC_FUNCTION_DECLARATION:
		{
			RefAdaAST __t51 = _t;
			RefAdaAST tmp26_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),GENERIC_FUNCTION_DECLARATION);
			_t = _t->getFirstChild();
			generic_formal_part_opt(_t);
			_t = _retTree;
			subprog_def_id(_t);
			_t = _retTree;
			function_tail(_t);
			_t = _retTree;
			_t = __t51;
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
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::subprog_def_id(RefAdaAST _t) {
	RefAdaAST subprog_def_id_AST_in = _t;

	try {      // for error handling
#line 171 "expandedada.store.g"
		m_isSubprogram = true;
#line 985 "AdaStoreWalker.cpp"
		def_id(_t);
		_t = _retTree;
#line 173 "expandedada.store.g"
		m_isSubprogram = false;
#line 990 "AdaStoreWalker.cpp"
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::subprog_decl(RefAdaAST _t) {
	RefAdaAST subprog_decl_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case GENERIC_PROCEDURE_INSTANTIATION:
		{
			RefAdaAST __t27 = _t;
			RefAdaAST tmp27_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),GENERIC_PROCEDURE_INSTANTIATION);
			_t = _t->getFirstChild();
			subprog_def_id(_t);
			_t = _retTree;
			generic_inst(_t);
			_t = _retTree;
			_t = __t27;
			_t = _t->getNextSibling();
			break;
		}
		case PROCEDURE_RENAMING_DECLARATION:
		{
			RefAdaAST __t28 = _t;
			RefAdaAST tmp28_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PROCEDURE_RENAMING_DECLARATION);
			_t = _t->getFirstChild();
			subprog_def_id(_t);
			_t = _retTree;
			formal_part_opt(_t);
			_t = _retTree;
			renames(_t);
			_t = _retTree;
			_t = __t28;
			_t = _t->getNextSibling();
			break;
		}
		case PROCEDURE_DECLARATION:
		{
			RefAdaAST __t29 = _t;
			RefAdaAST tmp29_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PROCEDURE_DECLARATION);
			_t = _t->getFirstChild();
			subprog_def_id(_t);
			_t = _retTree;
			formal_part_opt(_t);
			_t = _retTree;
			_t = __t29;
			_t = _t->getNextSibling();
			break;
		}
		case PROCEDURE_BODY_STUB:
		{
			RefAdaAST __t30 = _t;
			RefAdaAST tmp30_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PROCEDURE_BODY_STUB);
			_t = _t->getFirstChild();
			subprog_def_id(_t);
			_t = _retTree;
			formal_part_opt(_t);
			_t = _retTree;
			_t = __t30;
			_t = _t->getNextSibling();
			break;
		}
		case ABSTRACT_PROCEDURE_DECLARATION:
		{
			RefAdaAST __t31 = _t;
			RefAdaAST tmp31_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ABSTRACT_PROCEDURE_DECLARATION);
			_t = _t->getFirstChild();
			subprog_def_id(_t);
			_t = _retTree;
			formal_part_opt(_t);
			_t = _retTree;
			_t = __t31;
			_t = _t->getNextSibling();
			break;
		}
		case GENERIC_FUNCTION_INSTANTIATION:
		{
			RefAdaAST __t32 = _t;
			RefAdaAST tmp32_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),GENERIC_FUNCTION_INSTANTIATION);
			_t = _t->getFirstChild();
			def_designator(_t);
			_t = _retTree;
			generic_inst(_t);
			_t = _retTree;
			_t = __t32;
			_t = _t->getNextSibling();
			break;
		}
		case FUNCTION_RENAMING_DECLARATION:
		{
			RefAdaAST __t33 = _t;
			RefAdaAST tmp33_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),FUNCTION_RENAMING_DECLARATION);
			_t = _t->getFirstChild();
			def_designator(_t);
			_t = _retTree;
			function_tail(_t);
			_t = _retTree;
			renames(_t);
			_t = _retTree;
			_t = __t33;
			_t = _t->getNextSibling();
			break;
		}
		case FUNCTION_DECLARATION:
		{
			RefAdaAST __t34 = _t;
			RefAdaAST tmp34_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),FUNCTION_DECLARATION);
			_t = _t->getFirstChild();
			def_designator(_t);
			_t = _retTree;
			function_tail(_t);
			_t = _retTree;
			_t = __t34;
			_t = _t->getNextSibling();
			break;
		}
		case FUNCTION_BODY_STUB:
		{
			RefAdaAST __t35 = _t;
			RefAdaAST tmp35_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),FUNCTION_BODY_STUB);
			_t = _t->getFirstChild();
			def_designator(_t);
			_t = _retTree;
			function_tail(_t);
			_t = _retTree;
			_t = __t35;
			_t = _t->getNextSibling();
			break;
		}
		case ABSTRACT_FUNCTION_DECLARATION:
		{
			RefAdaAST __t36 = _t;
			RefAdaAST tmp36_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ABSTRACT_FUNCTION_DECLARATION);
			_t = _t->getFirstChild();
			subprog_def_id(_t);
			_t = _retTree;
			function_tail(_t);
			_t = _retTree;
			_t = __t36;
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
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::formal_part_opt(RefAdaAST _t) {
	RefAdaAST formal_part_opt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t102 = _t;
		RefAdaAST tmp37_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),FORMAL_PART_OPT);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_t->getType() == PARAMETER_SPECIFICATION)) {
				parameter_specification(_t);
				_t = _retTree;
			}
			else {
				goto _loop104;
			}

		}
		_loop104:;
		} // ( ... )*
		_t = __t102;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::def_designator(RefAdaAST _t) {
	RefAdaAST def_designator_AST_in = _t;
	RefAdaAST cn = static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENTIFIER:
		case DOT:
		{
			cn = (_t == ASTNULL) ? static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) : _t;
			compound_name(_t);
			_t = _retTree;
#line 221 "expandedada.store.g"

			//        kdDebug() << "def_designator cn:compound_name started" << endl;
				    if (m_addToStore) {
			//            kdDebug() << "def_designator cn:compound_name m_addToStore" << endl;
			FunctionDom method = m_model->create<FunctionModel>();
			method->setName (qtext (cn));
			method->setFileName(m_fileName);
			//            kdDebug() << "def_designator cn:compound_name method->setStartPosition(" << endl;
			method->setStartPosition(cn->getLine(), cn->getColumn());

			if (m_currentContainer == m_model->globalNamespace())
			m_file->addFunction(method);
			else
			m_currentContainer->addFunction(method);
			/// \FIXME adymo: is this valid for CodeModel
			/*	      ParsedMethod *old = m_currentContainer->getMethod (method);
				      if (old) {
					delete method;
					method = old;
				      } else {
					m_currentContainer->addMethod (method);
				      }*/
				    }

#line 1238 "AdaStoreWalker.cpp"
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::function_tail(RefAdaAST _t) {
	RefAdaAST function_tail_AST_in = _t;

	try {      // for error handling
		formal_part_opt(_t);
		_t = _retTree;
		subtype_mark(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::definable_operator_symbol(RefAdaAST _t) {
	RefAdaAST definable_operator_symbol_AST_in = _t;

	try {      // for error handling
		RefAdaAST tmp38_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),OPERATOR_SYMBOL);
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::spec_decl_part(RefAdaAST _t) {
	RefAdaAST spec_decl_part_AST_in = _t;
	RefAdaAST ps = static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case GENERIC_PACKAGE_INSTANTIATION:
		{
			RefAdaAST __t40 = _t;
			RefAdaAST tmp39_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),GENERIC_PACKAGE_INSTANTIATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			generic_inst(_t);
			_t = _retTree;
			_t = __t40;
			_t = _t->getNextSibling();
			break;
		}
		case PACKAGE_SPECIFICATION:
		{
			RefAdaAST __t41 = _t;
			RefAdaAST tmp40_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PACKAGE_SPECIFICATION);
			_t = _t->getFirstChild();
			ps = (_t == ASTNULL) ? static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) : _t;
			def_id(_t);
			_t = _retTree;
#line 250 "expandedada.store.g"

				       NamespaceDom psc = defineScope( ps );
				       m_currentContainer = psc;
				       m_scopeStack.append( psc );
				       m_addToStore = true;

#line 1332 "AdaStoreWalker.cpp"
			pkg_spec_part(_t);
			_t = _retTree;
#line 257 "expandedada.store.g"

				       m_scopeStack.remove(m_scopeStack.last());
				       if (m_scopeStack.count() == 0) {
					 kdDebug() << "adastore: m_scopeStack is empty!" << endl;
				         m_scopeStack.append( m_model->globalNamespace() );
				       }
				       m_currentContainer = m_scopeStack.last();
				       // m_currentContainer->setDeclarationEndsOnLine (endLine);
				       m_addToStore = false;

#line 1346 "AdaStoreWalker.cpp"
			_t = __t41;
			_t = _t->getNextSibling();
			break;
		}
		case PACKAGE_RENAMING_DECLARATION:
		{
			RefAdaAST __t42 = _t;
			RefAdaAST tmp41_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PACKAGE_RENAMING_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			renames(_t);
			_t = _retTree;
			_t = __t42;
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
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::basic_declarative_items_opt(RefAdaAST _t) {
	RefAdaAST basic_declarative_items_opt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t126 = _t;
		RefAdaAST tmp42_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),BASIC_DECLARATIVE_ITEMS_OPT);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_tokenSet_1.member(_t->getType()))) {
				basic_decl_item(_t);
				_t = _retTree;
			}
			else {
				goto _loop128;
			}

		}
		_loop128:;
		} // ( ... )*
		_t = __t126;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::generic_formal_part_opt(RefAdaAST _t) {
	RefAdaAST generic_formal_part_opt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t284 = _t;
		RefAdaAST tmp43_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),GENERIC_FORMAL_PART);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
				goto _loop286;
			}
			}
		}
		_loop286:;
		} // ( ... )*
		_t = __t284;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::procedure_body(RefAdaAST _t) {
	RefAdaAST procedure_body_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t308 = _t;
		RefAdaAST tmp44_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PROCEDURE_BODY);
		_t = _t->getFirstChild();
		def_id(_t);
		_t = _retTree;
		formal_part_opt(_t);
		_t = _retTree;
		body_part(_t);
		_t = _retTree;
		_t = __t308;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::function_body(RefAdaAST _t) {
	RefAdaAST function_body_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t310 = _t;
		RefAdaAST tmp45_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),FUNCTION_BODY);
		_t = _t->getFirstChild();
		def_designator(_t);
		_t = _retTree;
		function_tail(_t);
		_t = _retTree;
		body_part(_t);
		_t = _retTree;
		_t = __t310;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::subprog_decl_or_rename_or_inst_or_body(RefAdaAST _t) {
	RefAdaAST subprog_decl_or_rename_or_inst_or_body_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::subprog_decl_or_body(RefAdaAST _t) {
	RefAdaAST subprog_decl_or_body_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			RefAdaAST __t56 = _t;
			RefAdaAST tmp46_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PROCEDURE_DECLARATION);
			_t = _t->getFirstChild();
			subprog_def_id(_t);
			_t = _retTree;
			formal_part_opt(_t);
			_t = _retTree;
			_t = __t56;
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
			RefAdaAST __t57 = _t;
			RefAdaAST tmp47_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),FUNCTION_DECLARATION);
			_t = _t->getFirstChild();
			def_designator(_t);
			_t = _retTree;
			function_tail(_t);
			_t = _retTree;
			_t = __t57;
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
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::package_body(RefAdaAST _t) {
	RefAdaAST package_body_AST_in = _t;
	RefAdaAST id = static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get());

	try {      // for error handling
		RefAdaAST __t59 = _t;
		RefAdaAST tmp48_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PACKAGE_BODY);
		_t = _t->getFirstChild();
		id = (_t == ASTNULL) ? static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) : _t;
		def_id(_t);
		_t = _retTree;
		pkg_body_part(_t);
		_t = _retTree;
		_t = __t59;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::pragma_arg(RefAdaAST _t) {
	RefAdaAST pragma_arg_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case RIGHT_SHAFT:
		{
			RefAdaAST __t65 = _t;
			RefAdaAST tmp49_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),RIGHT_SHAFT);
			_t = _t->getFirstChild();
			RefAdaAST tmp50_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
			_t = _t->getNextSibling();
			expression(_t);
			_t = _retTree;
			_t = __t65;
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::expression(RefAdaAST _t) {
	RefAdaAST expression_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case AND:
		{
			RefAdaAST __t466 = _t;
			RefAdaAST tmp51_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),AND);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			relation(_t);
			_t = _retTree;
			_t = __t466;
			_t = _t->getNextSibling();
			break;
		}
		case AND_THEN:
		{
			RefAdaAST __t467 = _t;
			RefAdaAST tmp52_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),AND_THEN);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			relation(_t);
			_t = _retTree;
			_t = __t467;
			_t = _t->getNextSibling();
			break;
		}
		case OR:
		{
			RefAdaAST __t468 = _t;
			RefAdaAST tmp53_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),OR);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			relation(_t);
			_t = _retTree;
			_t = __t468;
			_t = _t->getNextSibling();
			break;
		}
		case OR_ELSE:
		{
			RefAdaAST __t469 = _t;
			RefAdaAST tmp54_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),OR_ELSE);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			relation(_t);
			_t = _retTree;
			_t = __t469;
			_t = _t->getNextSibling();
			break;
		}
		case XOR:
		{
			RefAdaAST __t470 = _t;
			RefAdaAST tmp55_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),XOR);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			relation(_t);
			_t = _retTree;
			_t = __t470;
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::attribute_id(RefAdaAST _t) {
	RefAdaAST attribute_id_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case RANGE:
		{
			RefAdaAST tmp56_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),RANGE);
			_t = _t->getNextSibling();
			break;
		}
		case DIGITS:
		{
			RefAdaAST tmp57_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),DIGITS);
			_t = _t->getNextSibling();
			break;
		}
		case DELTA:
		{
			RefAdaAST tmp58_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),DELTA);
			_t = _t->getNextSibling();
			break;
		}
		case ACCESS:
		{
			RefAdaAST tmp59_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ACCESS);
			_t = _t->getNextSibling();
			break;
		}
		case IDENTIFIER:
		{
			RefAdaAST tmp60_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
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
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::modifiers(RefAdaAST _t) {
	RefAdaAST modifiers_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t76 = _t;
		RefAdaAST tmp61_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),MODIFIERS);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case ABSTRACT:
			{
				RefAdaAST tmp62_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ABSTRACT);
				_t = _t->getNextSibling();
				break;
			}
			case ACCESS:
			{
				RefAdaAST tmp63_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ACCESS);
				_t = _t->getNextSibling();
				break;
			}
			case ALIASED:
			{
				RefAdaAST tmp64_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ALIASED);
				_t = _t->getNextSibling();
				break;
			}
			case ALL:
			{
				RefAdaAST tmp65_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ALL);
				_t = _t->getNextSibling();
				break;
			}
			case CONSTANT:
			{
				RefAdaAST tmp66_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),CONSTANT);
				_t = _t->getNextSibling();
				break;
			}
			case IN:
			{
				RefAdaAST tmp67_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IN);
				_t = _t->getNextSibling();
				break;
			}
			case LIMITED:
			{
				RefAdaAST tmp68_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),LIMITED);
				_t = _t->getNextSibling();
				break;
			}
			case OUT:
			{
				RefAdaAST tmp69_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),OUT);
				_t = _t->getNextSibling();
				break;
			}
			case PRIVATE:
			{
				RefAdaAST tmp70_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PRIVATE);
				_t = _t->getNextSibling();
				break;
			}
			case PROTECTED:
			{
				RefAdaAST tmp71_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PROTECTED);
				_t = _t->getNextSibling();
				break;
			}
			case REVERSE:
			{
				RefAdaAST tmp72_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),REVERSE);
				_t = _t->getNextSibling();
				break;
			}
			case TAGGED:
			{
				RefAdaAST tmp73_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),TAGGED);
				_t = _t->getNextSibling();
				break;
			}
			default:
			{
				goto _loop78;
			}
			}
		}
		_loop78:;
		} // ( ... )*
		_t = __t76;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::value_s(RefAdaAST _t) {
	RefAdaAST value_s_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t462 = _t;
		RefAdaAST tmp74_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),VALUES);
		_t = _t->getFirstChild();
		{ // ( ... )+
		int _cnt464=0;
		for (;;) {
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_tokenSet_2.member(_t->getType()))) {
				value(_t);
				_t = _retTree;
			}
			else {
				if ( _cnt464>=1 ) { goto _loop464; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));}
			}

			_cnt464++;
		}
		_loop464:;
		}  // ( ... )+
		_t = __t462;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::value(RefAdaAST _t) {
	RefAdaAST value_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case OTHERS:
		{
			RefAdaAST __t82 = _t;
			RefAdaAST tmp75_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),OTHERS);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			_t = __t82;
			_t = _t->getNextSibling();
			break;
		}
		case RIGHT_SHAFT:
		{
			RefAdaAST __t83 = _t;
			RefAdaAST tmp76_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),RIGHT_SHAFT);
			_t = _t->getFirstChild();
			ranged_expr_s(_t);
			_t = _retTree;
			expression(_t);
			_t = _retTree;
			_t = __t83;
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::ranged_expr_s(RefAdaAST _t) {
	RefAdaAST ranged_expr_s_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case PIPE:
		{
			RefAdaAST __t85 = _t;
			RefAdaAST tmp77_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PIPE);
			_t = _t->getFirstChild();
			ranged_expr_s(_t);
			_t = _retTree;
			ranged_expr(_t);
			_t = _retTree;
			_t = __t85;
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::ranged_expr(RefAdaAST _t) {
	RefAdaAST ranged_expr_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case DOT_DOT:
		{
			RefAdaAST __t87 = _t;
			RefAdaAST tmp78_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),DOT_DOT);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			simple_expression(_t);
			_t = _retTree;
			_t = __t87;
			_t = _t->getNextSibling();
			break;
		}
		case RANGE:
		{
			RefAdaAST __t88 = _t;
			RefAdaAST tmp79_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),RANGE);
			_t = _t->getFirstChild();
			expression(_t);
			_t = _retTree;
			range(_t);
			_t = _retTree;
			_t = __t88;
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::simple_expression(RefAdaAST _t) {
	RefAdaAST simple_expression_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case PLUS:
		{
			RefAdaAST __t482 = _t;
			RefAdaAST tmp80_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PLUS);
			_t = _t->getFirstChild();
			simple_expression(_t);
			_t = _retTree;
			signed_term(_t);
			_t = _retTree;
			_t = __t482;
			_t = _t->getNextSibling();
			break;
		}
		case MINUS:
		{
			RefAdaAST __t483 = _t;
			RefAdaAST tmp81_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),MINUS);
			_t = _t->getFirstChild();
			simple_expression(_t);
			_t = _retTree;
			signed_term(_t);
			_t = _retTree;
			_t = __t483;
			_t = _t->getNextSibling();
			break;
		}
		case CONCAT:
		{
			RefAdaAST __t484 = _t;
			RefAdaAST tmp82_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),CONCAT);
			_t = _t->getFirstChild();
			simple_expression(_t);
			_t = _retTree;
			signed_term(_t);
			_t = _retTree;
			_t = __t484;
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::range(RefAdaAST _t) {
	RefAdaAST range_AST_in = _t;

	try {      // for error handling
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::range_constraint(RefAdaAST _t) {
	RefAdaAST range_constraint_AST_in = _t;

	try {      // for error handling
		range(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::range_dots(RefAdaAST _t) {
	RefAdaAST range_dots_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t93 = _t;
		RefAdaAST tmp83_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),DOT_DOT);
		_t = _t->getFirstChild();
		simple_expression(_t);
		_t = _retTree;
		simple_expression(_t);
		_t = _retTree;
		_t = __t93;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::range_attrib_ref(RefAdaAST _t) {
	RefAdaAST range_attrib_ref_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t95 = _t;
		RefAdaAST tmp84_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),RANGE_ATTRIBUTE_REFERENCE);
		_t = _t->getFirstChild();
		prefix(_t);
		_t = _retTree;
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
		_t = __t95;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::prefix(RefAdaAST _t) {
	RefAdaAST prefix_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENTIFIER:
		{
			RefAdaAST tmp85_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
			_t = _t->getNextSibling();
			break;
		}
		case DOT:
		{
			RefAdaAST __t98 = _t;
			RefAdaAST tmp86_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),DOT);
			_t = _t->getFirstChild();
			prefix(_t);
			_t = _retTree;
			{
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case ALL:
			{
				RefAdaAST tmp87_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ALL);
				_t = _t->getNextSibling();
				break;
			}
			case IDENTIFIER:
			{
				RefAdaAST tmp88_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
				_t = _t->getNextSibling();
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
			}
			}
			}
			_t = __t98;
			_t = _t->getNextSibling();
			break;
		}
		case INDEXED_COMPONENT:
		{
			RefAdaAST __t100 = _t;
			RefAdaAST tmp89_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),INDEXED_COMPONENT);
			_t = _t->getFirstChild();
			prefix(_t);
			_t = _retTree;
			value_s(_t);
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
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::parameter_specification(RefAdaAST _t) {
	RefAdaAST parameter_specification_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t106 = _t;
		RefAdaAST tmp90_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PARAMETER_SPECIFICATION);
		_t = _t->getFirstChild();
		defining_identifier_list(_t);
		_t = _retTree;
		modifiers(_t);
		_t = _retTree;
		subtype_mark(_t);
		_t = _retTree;
		init_opt(_t);
		_t = _retTree;
		_t = __t106;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::defining_identifier_list(RefAdaAST _t) {
	RefAdaAST defining_identifier_list_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t108 = _t;
		RefAdaAST tmp91_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),DEFINING_IDENTIFIER_LIST);
		_t = _t->getFirstChild();
		{ // ( ... )+
		int _cnt110=0;
		for (;;) {
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_t->getType() == IDENTIFIER)) {
				RefAdaAST tmp92_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
				_t = _t->getNextSibling();
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
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::init_opt(RefAdaAST _t) {
	RefAdaAST init_opt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t145 = _t;
		RefAdaAST tmp93_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),INIT_OPT);
		_t = _t->getFirstChild();
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
		_t = __t145;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::name(RefAdaAST _t) {
	RefAdaAST name_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENTIFIER:
		{
			RefAdaAST tmp94_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
			_t = _t->getNextSibling();
			break;
		}
		case DOT:
		{
			RefAdaAST __t113 = _t;
			RefAdaAST tmp95_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),DOT);
			_t = _t->getFirstChild();
			name(_t);
			_t = _retTree;
			{
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case ALL:
			{
				RefAdaAST tmp96_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ALL);
				_t = _t->getNextSibling();
				break;
			}
			case IDENTIFIER:
			{
				RefAdaAST tmp97_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
				_t = _t->getNextSibling();
				break;
			}
			case CHARACTER_LITERAL:
			{
				RefAdaAST tmp98_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),CHARACTER_LITERAL);
				_t = _t->getNextSibling();
				break;
			}
			case OPERATOR_SYMBOL:
			{
				RefAdaAST tmp99_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),OPERATOR_SYMBOL);
				_t = _t->getNextSibling();
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
			}
			}
			}
			_t = __t113;
			_t = _t->getNextSibling();
			break;
		}
		case INDEXED_COMPONENT:
		{
			RefAdaAST __t115 = _t;
			RefAdaAST tmp100_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),INDEXED_COMPONENT);
			_t = _t->getFirstChild();
			name(_t);
			_t = _retTree;
			value_s(_t);
			_t = _retTree;
			_t = __t115;
			_t = _t->getNextSibling();
			break;
		}
		case TIC:
		{
			RefAdaAST __t116 = _t;
			RefAdaAST tmp101_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),TIC);
			_t = _t->getFirstChild();
			name(_t);
			_t = _retTree;
			attribute_id(_t);
			_t = _retTree;
			_t = __t116;
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
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::parenthesized_primary(RefAdaAST _t) {
	RefAdaAST parenthesized_primary_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t119 = _t;
		RefAdaAST tmp102_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PARENTHESIZED_PRIMARY);
		_t = _t->getFirstChild();
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case NuLL:
		{
			RefAdaAST tmp103_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),NuLL);
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
		_t = __t119;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::extension_opt(RefAdaAST _t) {
	RefAdaAST extension_opt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t122 = _t;
		RefAdaAST tmp104_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),EXTENSION_OPT);
		_t = _t->getFirstChild();
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case NuLL:
		{
			RefAdaAST tmp105_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),NuLL);
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
		_t = __t122;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::basic_decl_item(RefAdaAST _t) {
	RefAdaAST basic_decl_item_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::task_type_or_single_decl(RefAdaAST _t) {
	RefAdaAST task_type_or_single_decl_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case TASK_TYPE_DECLARATION:
		{
			RefAdaAST __t131 = _t;
			RefAdaAST tmp106_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),TASK_TYPE_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			discrim_part_opt(_t);
			_t = _retTree;
			task_definition_opt(_t);
			_t = _retTree;
			_t = __t131;
			_t = _t->getNextSibling();
			break;
		}
		case SINGLE_TASK_DECLARATION:
		{
			RefAdaAST __t132 = _t;
			RefAdaAST tmp107_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),SINGLE_TASK_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			task_definition_opt(_t);
			_t = _retTree;
			_t = __t132;
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
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::prot_type_or_single_decl(RefAdaAST _t) {
	RefAdaAST prot_type_or_single_decl_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case PROTECTED_TYPE_DECLARATION:
		{
			RefAdaAST __t173 = _t;
			RefAdaAST tmp108_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PROTECTED_TYPE_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			discrim_part_opt(_t);
			_t = _retTree;
			protected_definition(_t);
			_t = _retTree;
			_t = __t173;
			_t = _t->getNextSibling();
			break;
		}
		case SINGLE_PROTECTED_DECLARATION:
		{
			RefAdaAST __t174 = _t;
			RefAdaAST tmp109_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),SINGLE_PROTECTED_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			protected_definition(_t);
			_t = _retTree;
			_t = __t174;
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
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::decl_common(RefAdaAST _t) {
	RefAdaAST decl_common_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case ENUMERATION_TYPE_DECLARATION:
		{
			RefAdaAST __t191 = _t;
			RefAdaAST tmp110_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ENUMERATION_TYPE_DECLARATION);
			_t = _t->getFirstChild();
			RefAdaAST tmp111_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
			_t = _t->getNextSibling();
			enum_id_s(_t);
			_t = _retTree;
			_t = __t191;
			_t = _t->getNextSibling();
			break;
		}
		case SIGNED_INTEGER_TYPE_DECLARATION:
		{
			RefAdaAST __t192 = _t;
			RefAdaAST tmp112_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),SIGNED_INTEGER_TYPE_DECLARATION);
			_t = _t->getFirstChild();
			RefAdaAST tmp113_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
			_t = _t->getNextSibling();
			range(_t);
			_t = _retTree;
			_t = __t192;
			_t = _t->getNextSibling();
			break;
		}
		case MODULAR_TYPE_DECLARATION:
		{
			RefAdaAST __t193 = _t;
			RefAdaAST tmp114_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),MODULAR_TYPE_DECLARATION);
			_t = _t->getFirstChild();
			RefAdaAST tmp115_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
			_t = _t->getNextSibling();
			expression(_t);
			_t = _retTree;
			_t = __t193;
			_t = _t->getNextSibling();
			break;
		}
		case FLOATING_POINT_DECLARATION:
		{
			RefAdaAST __t194 = _t;
			RefAdaAST tmp116_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),FLOATING_POINT_DECLARATION);
			_t = _t->getFirstChild();
			RefAdaAST tmp117_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
			_t = _t->getNextSibling();
			expression(_t);
			_t = _retTree;
			range_constraint_opt(_t);
			_t = _retTree;
			_t = __t194;
			_t = _t->getNextSibling();
			break;
		}
		case ORDINARY_FIXED_POINT_DECLARATION:
		{
			RefAdaAST __t195 = _t;
			RefAdaAST tmp118_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ORDINARY_FIXED_POINT_DECLARATION);
			_t = _t->getFirstChild();
			RefAdaAST tmp119_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
			_t = _t->getNextSibling();
			expression(_t);
			_t = _retTree;
			range(_t);
			_t = _retTree;
			_t = __t195;
			_t = _t->getNextSibling();
			break;
		}
		case DECIMAL_FIXED_POINT_DECLARATION:
		{
			RefAdaAST __t196 = _t;
			RefAdaAST tmp120_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),DECIMAL_FIXED_POINT_DECLARATION);
			_t = _t->getFirstChild();
			RefAdaAST tmp121_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
			_t = _t->getNextSibling();
			expression(_t);
			_t = _retTree;
			expression(_t);
			_t = _retTree;
			range_constraint_opt(_t);
			_t = _retTree;
			_t = __t196;
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
			RefAdaAST __t197 = _t;
			RefAdaAST tmp122_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),INCOMPLETE_TYPE_DECLARATION);
			_t = _t->getFirstChild();
			RefAdaAST tmp123_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
			_t = _t->getNextSibling();
			discrim_part_opt(_t);
			_t = _retTree;
			_t = __t197;
			_t = _t->getNextSibling();
			break;
		}
		case PRIVATE_EXTENSION_DECLARATION:
		{
			RefAdaAST __t198 = _t;
			RefAdaAST tmp124_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PRIVATE_EXTENSION_DECLARATION);
			_t = _t->getFirstChild();
			id_and_discrim(_t);
			_t = _retTree;
			modifiers(_t);
			_t = _retTree;
			subtype_ind(_t);
			_t = _retTree;
			_t = __t198;
			_t = _t->getNextSibling();
			break;
		}
		case DERIVED_RECORD_EXTENSION:
		{
			RefAdaAST __t199 = _t;
			RefAdaAST tmp125_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),DERIVED_RECORD_EXTENSION);
			_t = _t->getFirstChild();
			id_and_discrim(_t);
			_t = _retTree;
			modifiers(_t);
			_t = _retTree;
			subtype_ind(_t);
			_t = _retTree;
			record_definition(_t);
			_t = _retTree;
			_t = __t199;
			_t = _t->getNextSibling();
			break;
		}
		case ORDINARY_DERIVED_TYPE_DECLARATION:
		{
			RefAdaAST __t200 = _t;
			RefAdaAST tmp126_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ORDINARY_DERIVED_TYPE_DECLARATION);
			_t = _t->getFirstChild();
			id_and_discrim(_t);
			_t = _retTree;
			subtype_ind(_t);
			_t = _retTree;
			_t = __t200;
			_t = _t->getNextSibling();
			break;
		}
		case PRIVATE_TYPE_DECLARATION:
		{
			RefAdaAST __t201 = _t;
			RefAdaAST tmp127_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PRIVATE_TYPE_DECLARATION);
			_t = _t->getFirstChild();
			id_and_discrim(_t);
			_t = _retTree;
			modifiers(_t);
			_t = _retTree;
			_t = __t201;
			_t = _t->getNextSibling();
			break;
		}
		case RECORD_TYPE_DECLARATION:
		{
			RefAdaAST __t202 = _t;
			RefAdaAST tmp128_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),RECORD_TYPE_DECLARATION);
			_t = _t->getFirstChild();
			id_and_discrim(_t);
			_t = _retTree;
			modifiers(_t);
			_t = _retTree;
			record_definition(_t);
			_t = _retTree;
			_t = __t202;
			_t = _t->getNextSibling();
			break;
		}
		case SUBTYPE_DECLARATION:
		{
			RefAdaAST __t203 = _t;
			RefAdaAST tmp129_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),SUBTYPE_DECLARATION);
			_t = _t->getFirstChild();
			RefAdaAST tmp130_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
			_t = _t->getNextSibling();
			subtype_ind(_t);
			_t = _retTree;
			_t = __t203;
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
			RefAdaAST __t204 = _t;
			RefAdaAST tmp131_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),EXCEPTION_RENAMING_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			compound_name(_t);
			_t = _retTree;
			_t = __t204;
			_t = _t->getNextSibling();
			break;
		}
		case OBJECT_RENAMING_DECLARATION:
		{
			RefAdaAST __t205 = _t;
			RefAdaAST tmp132_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),OBJECT_RENAMING_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			subtype_mark(_t);
			_t = _retTree;
			name(_t);
			_t = _retTree;
			_t = __t205;
			_t = _t->getNextSibling();
			break;
		}
		case EXCEPTION_DECLARATION:
		{
			RefAdaAST __t206 = _t;
			RefAdaAST tmp133_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),EXCEPTION_DECLARATION);
			_t = _t->getFirstChild();
			defining_identifier_list(_t);
			_t = _retTree;
			_t = __t206;
			_t = _t->getNextSibling();
			break;
		}
		case NUMBER_DECLARATION:
		{
			RefAdaAST __t207 = _t;
			RefAdaAST tmp134_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),NUMBER_DECLARATION);
			_t = _t->getFirstChild();
			defining_identifier_list(_t);
			_t = _retTree;
			expression(_t);
			_t = _retTree;
			_t = __t207;
			_t = _t->getNextSibling();
			break;
		}
		case ARRAY_OBJECT_DECLARATION:
		{
			RefAdaAST __t208 = _t;
			RefAdaAST tmp135_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ARRAY_OBJECT_DECLARATION);
			_t = _t->getFirstChild();
			defining_identifier_list(_t);
			_t = _retTree;
			modifiers(_t);
			_t = _retTree;
			array_type_definition(_t);
			_t = _retTree;
			init_opt(_t);
			_t = _retTree;
			_t = __t208;
			_t = _t->getNextSibling();
			break;
		}
		case OBJECT_DECLARATION:
		{
			RefAdaAST __t209 = _t;
			RefAdaAST tmp136_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),OBJECT_DECLARATION);
			_t = _t->getFirstChild();
			defining_identifier_list(_t);
			_t = _retTree;
			modifiers(_t);
			_t = _retTree;
			subtype_ind(_t);
			_t = _retTree;
			init_opt(_t);
			_t = _retTree;
			_t = __t209;
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
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::discrim_part_opt(RefAdaAST _t) {
	RefAdaAST discrim_part_opt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t136 = _t;
		RefAdaAST tmp137_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),DISCRIM_PART_OPT);
		_t = _t->getFirstChild();
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case BOX:
		{
			RefAdaAST tmp138_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),BOX);
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
		_t = __t136;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::task_definition_opt(RefAdaAST _t) {
	RefAdaAST task_definition_opt_AST_in = _t;

	try {      // for error handling
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::task_items_opt(RefAdaAST _t) {
	RefAdaAST task_items_opt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t148 = _t;
		RefAdaAST tmp139_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),TASK_ITEMS_OPT);
		_t = _t->getFirstChild();
		entrydecls_repspecs_opt(_t);
		_t = _retTree;
		_t = __t148;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::private_task_items_opt(RefAdaAST _t) {
	RefAdaAST private_task_items_opt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t171 = _t;
		RefAdaAST tmp140_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PRIVATE_TASK_ITEMS_OPT);
		_t = _t->getFirstChild();
		entrydecls_repspecs_opt(_t);
		_t = _retTree;
		_t = __t171;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::discriminant_specifications(RefAdaAST _t) {
	RefAdaAST discriminant_specifications_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t139 = _t;
		RefAdaAST tmp141_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),DISCRIMINANT_SPECIFICATIONS);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_t->getType() == DISCRIMINANT_SPECIFICATION)) {
				discriminant_specification(_t);
				_t = _retTree;
			}
			else {
				goto _loop141;
			}

		}
		_loop141:;
		} // ( ... )*
		_t = __t139;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::discriminant_specification(RefAdaAST _t) {
	RefAdaAST discriminant_specification_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t143 = _t;
		RefAdaAST tmp142_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),DISCRIMINANT_SPECIFICATION);
		_t = _t->getFirstChild();
		defining_identifier_list(_t);
		_t = _retTree;
		modifiers(_t);
		_t = _retTree;
		subtype_mark(_t);
		_t = _retTree;
		init_opt(_t);
		_t = _retTree;
		_t = __t143;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::entrydecls_repspecs_opt(RefAdaAST _t) {
	RefAdaAST entrydecls_repspecs_opt_AST_in = _t;

	try {      // for error handling
		{ // ( ... )*
		for (;;) {
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
				goto _loop151;
			}
			}
		}
		_loop151:;
		} // ( ... )*
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::entry_declaration(RefAdaAST _t) {
	RefAdaAST entry_declaration_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t153 = _t;
		RefAdaAST tmp143_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ENTRY_DECLARATION);
		_t = _t->getFirstChild();
		RefAdaAST tmp144_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
		_t = _t->getNextSibling();
		discrete_subtype_def_opt(_t);
		_t = _retTree;
		formal_part_opt(_t);
		_t = _retTree;
		_t = __t153;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::rep_spec(RefAdaAST _t) {
	RefAdaAST rep_spec_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case RECORD_REPRESENTATION_CLAUSE:
		{
			RefAdaAST __t159 = _t;
			RefAdaAST tmp145_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),RECORD_REPRESENTATION_CLAUSE);
			_t = _t->getFirstChild();
			subtype_mark(_t);
			_t = _retTree;
			align_opt(_t);
			_t = _retTree;
			comp_loc_s(_t);
			_t = _retTree;
			_t = __t159;
			_t = _t->getNextSibling();
			break;
		}
		case AT_CLAUSE:
		{
			RefAdaAST __t160 = _t;
			RefAdaAST tmp146_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),AT_CLAUSE);
			_t = _t->getFirstChild();
			subtype_mark(_t);
			_t = _retTree;
			expression(_t);
			_t = _retTree;
			_t = __t160;
			_t = _t->getNextSibling();
			break;
		}
		case ATTRIBUTE_DEFINITION_CLAUSE:
		{
			RefAdaAST __t161 = _t;
			RefAdaAST tmp147_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ATTRIBUTE_DEFINITION_CLAUSE);
			_t = _t->getFirstChild();
			subtype_mark(_t);
			_t = _retTree;
			expression(_t);
			_t = _retTree;
			_t = __t161;
			_t = _t->getNextSibling();
			break;
		}
		case ENUMERATION_REPESENTATION_CLAUSE:
		{
			RefAdaAST __t162 = _t;
			RefAdaAST tmp148_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ENUMERATION_REPESENTATION_CLAUSE);
			_t = _t->getFirstChild();
			local_enum_name(_t);
			_t = _retTree;
			enumeration_aggregate(_t);
			_t = _retTree;
			_t = __t162;
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
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::discrete_subtype_def_opt(RefAdaAST _t) {
	RefAdaAST discrete_subtype_def_opt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t155 = _t;
		RefAdaAST tmp149_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),DISCRETE_SUBTYPE_DEF_OPT);
		_t = _t->getFirstChild();
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
		_t = __t155;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::discrete_subtype_definition(RefAdaAST _t) {
	RefAdaAST discrete_subtype_definition_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::subtype_ind(RefAdaAST _t) {
	RefAdaAST subtype_ind_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t228 = _t;
		RefAdaAST tmp150_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),SUBTYPE_INDICATION);
		_t = _t->getFirstChild();
		subtype_mark(_t);
		_t = _retTree;
		constraint_opt(_t);
		_t = _retTree;
		_t = __t228;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::align_opt(RefAdaAST _t) {
	RefAdaAST align_opt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t164 = _t;
		RefAdaAST tmp151_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),MOD_CLAUSE_OPT);
		_t = _t->getFirstChild();
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
		_t = __t164;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::comp_loc_s(RefAdaAST _t) {
	RefAdaAST comp_loc_s_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t167 = _t;
		RefAdaAST tmp152_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),COMPONENT_CLAUSES_OPT);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::local_enum_name(RefAdaAST _t) {
	RefAdaAST local_enum_name_AST_in = _t;

	try {      // for error handling
		RefAdaAST tmp153_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::enumeration_aggregate(RefAdaAST _t) {
	RefAdaAST enumeration_aggregate_AST_in = _t;

	try {      // for error handling
		{ // ( ... )*
		for (;;) {
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_tokenSet_2.member(_t->getType()))) {
				value(_t);
				_t = _retTree;
			}
			else {
				goto _loop282;
			}

		}
		_loop282:;
		} // ( ... )*
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::protected_definition(RefAdaAST _t) {
	RefAdaAST protected_definition_AST_in = _t;

	try {      // for error handling
		prot_op_decl_s(_t);
		_t = _retTree;
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::prot_op_decl_s(RefAdaAST _t) {
	RefAdaAST prot_op_decl_s_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t178 = _t;
		RefAdaAST tmp154_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PROT_OP_DECLARATIONS);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_tokenSet_3.member(_t->getType()))) {
				prot_op_decl(_t);
				_t = _retTree;
			}
			else {
				goto _loop180;
			}

		}
		_loop180:;
		} // ( ... )*
		_t = __t178;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::prot_member_decl_s(RefAdaAST _t) {
	RefAdaAST prot_member_decl_s_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t185 = _t;
		RefAdaAST tmp155_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PROT_MEMBER_DECLARATIONS);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
				goto _loop187;
			}
			}
		}
		_loop187:;
		} // ( ... )*
		_t = __t185;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::prot_op_decl(RefAdaAST _t) {
	RefAdaAST prot_op_decl_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			RefAdaAST __t182 = _t;
			RefAdaAST tmp156_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PROCEDURE_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			formal_part_opt(_t);
			_t = _retTree;
			_t = __t182;
			_t = _t->getNextSibling();
			break;
		}
		case FUNCTION_DECLARATION:
		{
			RefAdaAST __t183 = _t;
			RefAdaAST tmp157_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),FUNCTION_DECLARATION);
			_t = _t->getFirstChild();
			def_designator(_t);
			_t = _retTree;
			function_tail(_t);
			_t = _retTree;
			_t = __t183;
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::comp_decl(RefAdaAST _t) {
	RefAdaAST comp_decl_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t189 = _t;
		RefAdaAST tmp158_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),COMPONENT_DECLARATION);
		_t = _t->getFirstChild();
		defining_identifier_list(_t);
		_t = _retTree;
		component_subtype_def(_t);
		_t = _retTree;
		init_opt(_t);
		_t = _retTree;
		_t = __t189;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::component_subtype_def(RefAdaAST _t) {
	RefAdaAST component_subtype_def_AST_in = _t;

	try {      // for error handling
		modifiers(_t);
		_t = _retTree;
		subtype_ind(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::enum_id_s(RefAdaAST _t) {
	RefAdaAST enum_id_s_AST_in = _t;

	try {      // for error handling
		{ // ( ... )+
		int _cnt213=0;
		for (;;) {
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_t->getType() == IDENTIFIER || _t->getType() == CHARACTER_LITERAL)) {
				enumeration_literal_specification(_t);
				_t = _retTree;
			}
			else {
				if ( _cnt213>=1 ) { goto _loop213; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));}
			}

			_cnt213++;
		}
		_loop213:;
		}  // ( ... )+
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::range_constraint_opt(RefAdaAST _t) {
	RefAdaAST range_constraint_opt_AST_in = _t;

	try {      // for error handling
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::array_type_declaration(RefAdaAST _t) {
	RefAdaAST array_type_declaration_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t219 = _t;
		RefAdaAST tmp159_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ARRAY_TYPE_DECLARATION);
		_t = _t->getFirstChild();
		RefAdaAST tmp160_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
		_t = _t->getNextSibling();
		array_type_definition(_t);
		_t = _retTree;
		_t = __t219;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::access_type_declaration(RefAdaAST _t) {
	RefAdaAST access_type_declaration_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case ACCESS_TO_PROCEDURE_DECLARATION:
		{
			RefAdaAST __t252 = _t;
			RefAdaAST tmp161_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ACCESS_TO_PROCEDURE_DECLARATION);
			_t = _t->getFirstChild();
			RefAdaAST tmp162_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
			_t = _t->getNextSibling();
			modifiers(_t);
			_t = _retTree;
			formal_part_opt(_t);
			_t = _retTree;
			_t = __t252;
			_t = _t->getNextSibling();
			break;
		}
		case ACCESS_TO_FUNCTION_DECLARATION:
		{
			RefAdaAST __t253 = _t;
			RefAdaAST tmp163_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ACCESS_TO_FUNCTION_DECLARATION);
			_t = _t->getFirstChild();
			RefAdaAST tmp164_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
			_t = _t->getNextSibling();
			modifiers(_t);
			_t = _retTree;
			function_tail(_t);
			_t = _retTree;
			_t = __t253;
			_t = _t->getNextSibling();
			break;
		}
		case ACCESS_TO_OBJECT_DECLARATION:
		{
			RefAdaAST __t254 = _t;
			RefAdaAST tmp165_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ACCESS_TO_OBJECT_DECLARATION);
			_t = _t->getFirstChild();
			RefAdaAST tmp166_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
			_t = _t->getNextSibling();
			modifiers(_t);
			_t = _retTree;
			subtype_ind(_t);
			_t = _retTree;
			_t = __t254;
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
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::id_and_discrim(RefAdaAST _t) {
	RefAdaAST id_and_discrim_AST_in = _t;

	try {      // for error handling
		RefAdaAST tmp167_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
		_t = _t->getNextSibling();
		discrim_part_opt(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::record_definition(RefAdaAST _t) {
	RefAdaAST record_definition_AST_in = _t;

	try {      // for error handling
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::array_type_definition(RefAdaAST _t) {
	RefAdaAST array_type_definition_AST_in = _t;

	try {      // for error handling
		index_or_discrete_range_s(_t);
		_t = _retTree;
		component_subtype_def(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::enumeration_literal_specification(RefAdaAST _t) {
	RefAdaAST enumeration_literal_specification_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENTIFIER:
		{
			RefAdaAST tmp168_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
			_t = _t->getNextSibling();
			break;
		}
		case CHARACTER_LITERAL:
		{
			RefAdaAST tmp169_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),CHARACTER_LITERAL);
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
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::index_or_discrete_range_s(RefAdaAST _t) {
	RefAdaAST index_or_discrete_range_s_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			RefAdaAST __t221 = _t;
			RefAdaAST tmp170_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),COMMA);
			_t = _t->getFirstChild();
			index_or_discrete_range_s(_t);
			_t = _retTree;
			index_or_discrete_range(_t);
			_t = _retTree;
			_t = __t221;
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
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::index_or_discrete_range(RefAdaAST _t) {
	RefAdaAST index_or_discrete_range_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case DOT_DOT:
		{
			RefAdaAST __t223 = _t;
			RefAdaAST tmp171_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),DOT_DOT);
			_t = _t->getFirstChild();
			simple_expression(_t);
			_t = _retTree;
			simple_expression(_t);
			_t = _retTree;
			_t = __t223;
			_t = _t->getNextSibling();
			break;
		}
		case RANGE:
		{
			RefAdaAST __t224 = _t;
			RefAdaAST tmp172_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),RANGE);
			_t = _t->getFirstChild();
			simple_expression(_t);
			_t = _retTree;
			{
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case BOX:
			{
				RefAdaAST tmp173_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),BOX);
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
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
			}
			}
			}
			_t = __t224;
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::constraint_opt(RefAdaAST _t) {
	RefAdaAST constraint_opt_AST_in = _t;

	try {      // for error handling
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::digits_constraint(RefAdaAST _t) {
	RefAdaAST digits_constraint_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t232 = _t;
		RefAdaAST tmp174_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),DIGITS_CONSTRAINT);
		_t = _t->getFirstChild();
		expression(_t);
		_t = _retTree;
		range_constraint_opt(_t);
		_t = _retTree;
		_t = __t232;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::delta_constraint(RefAdaAST _t) {
	RefAdaAST delta_constraint_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t234 = _t;
		RefAdaAST tmp175_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),DELTA_CONSTRAINT);
		_t = _t->getFirstChild();
		expression(_t);
		_t = _retTree;
		range_constraint_opt(_t);
		_t = _retTree;
		_t = __t234;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::index_constraint(RefAdaAST _t) {
	RefAdaAST index_constraint_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t236 = _t;
		RefAdaAST tmp176_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),INDEX_CONSTRAINT);
		_t = _t->getFirstChild();
		{ // ( ... )+
		int _cnt238=0;
		for (;;) {
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_t->getType() == DOT_DOT || _t->getType() == RANGE_ATTRIBUTE_REFERENCE || _t->getType() == SUBTYPE_INDICATION)) {
				discrete_range(_t);
				_t = _retTree;
			}
			else {
				if ( _cnt238>=1 ) { goto _loop238; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));}
			}

			_cnt238++;
		}
		_loop238:;
		}  // ( ... )+
		_t = __t236;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::discriminant_constraint(RefAdaAST _t) {
	RefAdaAST discriminant_constraint_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t241 = _t;
		RefAdaAST tmp177_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),DISCRIMINANT_CONSTRAINT);
		_t = _t->getFirstChild();
		{ // ( ... )+
		int _cnt243=0;
		for (;;) {
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_t->getType() == DISCRIMINANT_ASSOCIATION)) {
				discriminant_association(_t);
				_t = _retTree;
			}
			else {
				if ( _cnt243>=1 ) { goto _loop243; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));}
			}

			_cnt243++;
		}
		_loop243:;
		}  // ( ... )+
		_t = __t241;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::discrete_range(RefAdaAST _t) {
	RefAdaAST discrete_range_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::discriminant_association(RefAdaAST _t) {
	RefAdaAST discriminant_association_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t245 = _t;
		RefAdaAST tmp178_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),DISCRIMINANT_ASSOCIATION);
		_t = _t->getFirstChild();
		selector_names_opt(_t);
		_t = _retTree;
		expression(_t);
		_t = _retTree;
		_t = __t245;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::selector_names_opt(RefAdaAST _t) {
	RefAdaAST selector_names_opt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t247 = _t;
		RefAdaAST tmp179_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),SELECTOR_NAMES_OPT);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_t->getType() == IDENTIFIER)) {
				selector_name(_t);
				_t = _retTree;
			}
			else {
				goto _loop249;
			}

		}
		_loop249:;
		} // ( ... )*
		_t = __t247;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::selector_name(RefAdaAST _t) {
	RefAdaAST selector_name_AST_in = _t;

	try {      // for error handling
		RefAdaAST tmp180_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::component_list(RefAdaAST _t) {
	RefAdaAST component_list_AST_in = _t;

	try {      // for error handling
		component_items(_t);
		_t = _retTree;
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::component_items(RefAdaAST _t) {
	RefAdaAST component_items_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t260 = _t;
		RefAdaAST tmp181_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),COMPONENT_ITEMS);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
				goto _loop262;
			}
			}
		}
		_loop262:;
		} // ( ... )*
		_t = __t260;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::variant_part(RefAdaAST _t) {
	RefAdaAST variant_part_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t264 = _t;
		RefAdaAST tmp182_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),VARIANT_PART);
		_t = _t->getFirstChild();
		discriminant_direct_name(_t);
		_t = _retTree;
		variant_s(_t);
		_t = _retTree;
		_t = __t264;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::discriminant_direct_name(RefAdaAST _t) {
	RefAdaAST discriminant_direct_name_AST_in = _t;

	try {      // for error handling
		RefAdaAST tmp183_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::variant_s(RefAdaAST _t) {
	RefAdaAST variant_s_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t267 = _t;
		RefAdaAST tmp184_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),VARIANTS);
		_t = _t->getFirstChild();
		{ // ( ... )+
		int _cnt269=0;
		for (;;) {
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_t->getType() == VARIANT)) {
				variant(_t);
				_t = _retTree;
			}
			else {
				if ( _cnt269>=1 ) { goto _loop269; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));}
			}

			_cnt269++;
		}
		_loop269:;
		}  // ( ... )+
		_t = __t267;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::variant(RefAdaAST _t) {
	RefAdaAST variant_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t271 = _t;
		RefAdaAST tmp185_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),VARIANT);
		_t = _t->getFirstChild();
		choice_s(_t);
		_t = _retTree;
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
		_t = __t271;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::choice_s(RefAdaAST _t) {
	RefAdaAST choice_s_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case PIPE:
		{
			RefAdaAST __t274 = _t;
			RefAdaAST tmp186_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PIPE);
			_t = _t->getFirstChild();
			choice_s(_t);
			_t = _retTree;
			choice(_t);
			_t = _retTree;
			_t = __t274;
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::choice(RefAdaAST _t) {
	RefAdaAST choice_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case OTHERS:
		{
			RefAdaAST tmp187_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),OTHERS);
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::discrete_with_range(RefAdaAST _t) {
	RefAdaAST discrete_with_range_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::mark_with_constraint(RefAdaAST _t) {
	RefAdaAST mark_with_constraint_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t278 = _t;
		RefAdaAST tmp188_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),MARK_WITH_CONSTRAINT);
		_t = _t->getFirstChild();
		subtype_mark(_t);
		_t = _retTree;
		range_constraint(_t);
		_t = _retTree;
		_t = __t278;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::generic_formal_parameter(RefAdaAST _t) {
	RefAdaAST generic_formal_parameter_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case FORMAL_DISCRETE_TYPE_DECLARATION:
		{
			RefAdaAST __t288 = _t;
			RefAdaAST tmp189_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),FORMAL_DISCRETE_TYPE_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			_t = __t288;
			_t = _t->getNextSibling();
			break;
		}
		case FORMAL_SIGNED_INTEGER_TYPE_DECLARATION:
		{
			RefAdaAST __t289 = _t;
			RefAdaAST tmp190_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),FORMAL_SIGNED_INTEGER_TYPE_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			_t = __t289;
			_t = _t->getNextSibling();
			break;
		}
		case FORMAL_MODULAR_TYPE_DECLARATION:
		{
			RefAdaAST __t290 = _t;
			RefAdaAST tmp191_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),FORMAL_MODULAR_TYPE_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			_t = __t290;
			_t = _t->getNextSibling();
			break;
		}
		case FORMAL_DECIMAL_FIXED_POINT_DECLARATION:
		{
			RefAdaAST __t291 = _t;
			RefAdaAST tmp192_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),FORMAL_DECIMAL_FIXED_POINT_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			_t = __t291;
			_t = _t->getNextSibling();
			break;
		}
		case FORMAL_ORDINARY_FIXED_POINT_DECLARATION:
		{
			RefAdaAST __t292 = _t;
			RefAdaAST tmp193_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),FORMAL_ORDINARY_FIXED_POINT_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			_t = __t292;
			_t = _t->getNextSibling();
			break;
		}
		case FORMAL_FLOATING_POINT_DECLARATION:
		{
			RefAdaAST __t293 = _t;
			RefAdaAST tmp194_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),FORMAL_FLOATING_POINT_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			_t = __t293;
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
			RefAdaAST __t294 = _t;
			RefAdaAST tmp195_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),FORMAL_PRIVATE_TYPE_DECLARATION);
			_t = _t->getFirstChild();
			id_part(_t);
			_t = _retTree;
			modifiers(_t);
			_t = _retTree;
			_t = __t294;
			_t = _t->getNextSibling();
			break;
		}
		case FORMAL_ORDINARY_DERIVED_TYPE_DECLARATION:
		{
			RefAdaAST __t295 = _t;
			RefAdaAST tmp196_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),FORMAL_ORDINARY_DERIVED_TYPE_DECLARATION);
			_t = _t->getFirstChild();
			id_part(_t);
			_t = _retTree;
			subtype_ind(_t);
			_t = _retTree;
			_t = __t295;
			_t = _t->getNextSibling();
			break;
		}
		case FORMAL_PRIVATE_EXTENSION_DECLARATION:
		{
			RefAdaAST __t296 = _t;
			RefAdaAST tmp197_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),FORMAL_PRIVATE_EXTENSION_DECLARATION);
			_t = _t->getFirstChild();
			id_part(_t);
			_t = _retTree;
			modifiers(_t);
			_t = _retTree;
			subtype_ind(_t);
			_t = _retTree;
			_t = __t296;
			_t = _t->getNextSibling();
			break;
		}
		case FORMAL_PROCEDURE_DECLARATION:
		{
			RefAdaAST __t297 = _t;
			RefAdaAST tmp198_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),FORMAL_PROCEDURE_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			formal_part_opt(_t);
			_t = _retTree;
			subprogram_default_opt(_t);
			_t = _retTree;
			_t = __t297;
			_t = _t->getNextSibling();
			break;
		}
		case FORMAL_FUNCTION_DECLARATION:
		{
			RefAdaAST __t298 = _t;
			RefAdaAST tmp199_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),FORMAL_FUNCTION_DECLARATION);
			_t = _t->getFirstChild();
			def_designator(_t);
			_t = _retTree;
			function_tail(_t);
			_t = _retTree;
			subprogram_default_opt(_t);
			_t = _retTree;
			_t = __t298;
			_t = _t->getNextSibling();
			break;
		}
		case FORMAL_PACKAGE_DECLARATION:
		{
			RefAdaAST __t299 = _t;
			RefAdaAST tmp200_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),FORMAL_PACKAGE_DECLARATION);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			compound_name(_t);
			_t = _retTree;
			formal_package_actual_part_opt(_t);
			_t = _retTree;
			_t = __t299;
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::formal_array_type_declaration(RefAdaAST _t) {
	RefAdaAST formal_array_type_declaration_AST_in = _t;

	try {      // for error handling
		array_type_declaration(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::formal_access_type_declaration(RefAdaAST _t) {
	RefAdaAST formal_access_type_declaration_AST_in = _t;

	try {      // for error handling
		access_type_declaration(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::id_part(RefAdaAST _t) {
	RefAdaAST id_part_AST_in = _t;

	try {      // for error handling
		def_id(_t);
		_t = _retTree;
		discrim_part_opt(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::subprogram_default_opt(RefAdaAST _t) {
	RefAdaAST subprogram_default_opt_AST_in = _t;

	try {      // for error handling
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case BOX:
		{
			RefAdaAST tmp201_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),BOX);
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::formal_package_actual_part_opt(RefAdaAST _t) {
	RefAdaAST formal_package_actual_part_opt_AST_in = _t;

	try {      // for error handling
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case BOX:
		{
			RefAdaAST tmp202_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),BOX);
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::body_part(RefAdaAST _t) {
	RefAdaAST body_part_AST_in = _t;

	try {      // for error handling
		declarative_part(_t);
		_t = _retTree;
		block_body(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::declarative_part(RefAdaAST _t) {
	RefAdaAST declarative_part_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t313 = _t;
		RefAdaAST tmp203_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),DECLARATIVE_PART);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
				goto _loop315;
			}
			}
		}
		_loop315:;
		} // ( ... )*
		_t = __t313;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::block_body(RefAdaAST _t) {
	RefAdaAST block_body_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t332 = _t;
		RefAdaAST tmp204_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),BLOCK_BODY);
		_t = _t->getFirstChild();
		handled_stmt_s(_t);
		_t = _retTree;
		_t = __t332;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::declarative_item(RefAdaAST _t) {
	RefAdaAST declarative_item_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case PACKAGE_BODY_STUB:
		{
			RefAdaAST __t317 = _t;
			RefAdaAST tmp205_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PACKAGE_BODY_STUB);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			_t = __t317;
			_t = _t->getNextSibling();
			break;
		}
		case PACKAGE_BODY:
		{
			RefAdaAST __t318 = _t;
			RefAdaAST tmp206_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PACKAGE_BODY);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			pkg_body_part(_t);
			_t = _retTree;
			_t = __t318;
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
			RefAdaAST __t319 = _t;
			RefAdaAST tmp207_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),TASK_BODY_STUB);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			_t = __t319;
			_t = _t->getNextSibling();
			break;
		}
		case TASK_BODY:
		{
			RefAdaAST __t320 = _t;
			RefAdaAST tmp208_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),TASK_BODY);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			body_part(_t);
			_t = _retTree;
			_t = __t320;
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
			RefAdaAST __t321 = _t;
			RefAdaAST tmp209_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PROTECTED_BODY_STUB);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			_t = __t321;
			_t = _t->getNextSibling();
			break;
		}
		case PROTECTED_BODY:
		{
			RefAdaAST __t322 = _t;
			RefAdaAST tmp210_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PROTECTED_BODY);
			_t = _t->getFirstChild();
			def_id(_t);
			_t = _retTree;
			prot_op_bodies_opt(_t);
			_t = _retTree;
			_t = __t322;
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::prot_op_bodies_opt(RefAdaAST _t) {
	RefAdaAST prot_op_bodies_opt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t328 = _t;
		RefAdaAST tmp211_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PROT_OP_BODIES_OPT);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
				goto _loop330;
			}
			}
		}
		_loop330:;
		} // ( ... )*
		_t = __t328;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::block_body_opt(RefAdaAST _t) {
	RefAdaAST block_body_opt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t325 = _t;
		RefAdaAST tmp212_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),BLOCK_BODY_OPT);
		_t = _t->getFirstChild();
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
		_t = __t325;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::handled_stmt_s(RefAdaAST _t) {
	RefAdaAST handled_stmt_s_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t334 = _t;
		RefAdaAST tmp213_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),HANDLED_SEQUENCE_OF_STATEMENTS);
		_t = _t->getFirstChild();
		statements(_t);
		_t = _retTree;
		except_handler_part_opt(_t);
		_t = _retTree;
		_t = __t334;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::entry_body(RefAdaAST _t) {
	RefAdaAST entry_body_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t391 = _t;
		RefAdaAST tmp214_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ENTRY_BODY);
		_t = _t->getFirstChild();
		def_id(_t);
		_t = _retTree;
		entry_body_formal_part(_t);
		_t = _retTree;
		entry_barrier(_t);
		_t = _retTree;
		body_part(_t);
		_t = _retTree;
		_t = __t391;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::statements(RefAdaAST _t) {
	RefAdaAST statements_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t336 = _t;
		RefAdaAST tmp215_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),SEQUENCE_OF_STATEMENTS);
		_t = _t->getFirstChild();
		{ // ( ... )+
		int _cnt338=0;
		for (;;) {
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
				if ( _cnt338>=1 ) { goto _loop338; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));}
			}
			}
			_cnt338++;
		}
		_loop338:;
		}  // ( ... )+
		_t = __t336;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::except_handler_part_opt(RefAdaAST _t) {
	RefAdaAST except_handler_part_opt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t442 = _t;
		RefAdaAST tmp216_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),EXCEPT_HANDLER_PART_OPT);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_t->getType() == EXCEPTION_HANDLER)) {
				exception_handler(_t);
				_t = _retTree;
			}
			else {
				goto _loop444;
			}

		}
		_loop444:;
		} // ( ... )*
		_t = __t442;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::statement(RefAdaAST _t) {
	RefAdaAST statement_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t340 = _t;
		RefAdaAST tmp217_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),STATEMENT);
		_t = _t->getFirstChild();
		def_label_opt(_t);
		_t = _retTree;
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
		_t = __t340;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::def_label_opt(RefAdaAST _t) {
	RefAdaAST def_label_opt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t343 = _t;
		RefAdaAST tmp218_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),LABEL_OPT);
		_t = _t->getFirstChild();
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENTIFIER:
		{
			RefAdaAST tmp219_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
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
		_t = __t343;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::null_stmt(RefAdaAST _t) {
	RefAdaAST null_stmt_AST_in = _t;

	try {      // for error handling
		RefAdaAST tmp220_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),NULL_STATEMENT);
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::exit_stmt(RefAdaAST _t) {
	RefAdaAST exit_stmt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t378 = _t;
		RefAdaAST tmp221_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),EXIT_STATEMENT);
		_t = _t->getFirstChild();
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case WHEN:
		{
			RefAdaAST tmp222_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),WHEN);
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
		_t = __t378;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::return_stmt(RefAdaAST _t) {
	RefAdaAST return_stmt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t383 = _t;
		RefAdaAST tmp223_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),RETURN_STATEMENT);
		_t = _t->getFirstChild();
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
		_t = __t383;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::goto_stmt(RefAdaAST _t) {
	RefAdaAST goto_stmt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t386 = _t;
		RefAdaAST tmp224_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),GOTO_STATEMENT);
		_t = _t->getFirstChild();
		label_name(_t);
		_t = _retTree;
		_t = __t386;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::delay_stmt(RefAdaAST _t) {
	RefAdaAST delay_stmt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t406 = _t;
		RefAdaAST tmp225_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),DELAY_STATEMENT);
		_t = _t->getFirstChild();
		modifiers(_t);
		_t = _retTree;
		expression(_t);
		_t = _retTree;
		_t = __t406;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::abort_stmt(RefAdaAST _t) {
	RefAdaAST abort_stmt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t438 = _t;
		RefAdaAST tmp226_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ABORT_STATEMENT);
		_t = _t->getFirstChild();
		{ // ( ... )+
		int _cnt440=0;
		for (;;) {
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_tokenSet_4.member(_t->getType()))) {
				name(_t);
				_t = _retTree;
			}
			else {
				if ( _cnt440>=1 ) { goto _loop440; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));}
			}

			_cnt440++;
		}
		_loop440:;
		}  // ( ... )+
		_t = __t438;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::raise_stmt(RefAdaAST _t) {
	RefAdaAST raise_stmt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t454 = _t;
		RefAdaAST tmp227_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),RAISE_STATEMENT);
		_t = _t->getFirstChild();
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
		_t = __t454;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::requeue_stmt(RefAdaAST _t) {
	RefAdaAST requeue_stmt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t457 = _t;
		RefAdaAST tmp228_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),REQUEUE_STATEMENT);
		_t = _t->getFirstChild();
		name(_t);
		_t = _retTree;
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case ABORT:
		{
			RefAdaAST tmp229_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ABORT);
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
		_t = __t457;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::accept_stmt(RefAdaAST _t) {
	RefAdaAST accept_stmt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t400 = _t;
		RefAdaAST tmp230_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ACCEPT_STATEMENT);
		_t = _t->getFirstChild();
		def_id(_t);
		_t = _retTree;
		entry_index_opt(_t);
		_t = _retTree;
		formal_part_opt(_t);
		_t = _retTree;
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
		_t = __t400;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::select_stmt(RefAdaAST _t) {
	RefAdaAST select_stmt_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case ASYNCHRONOUS_SELECT:
		{
			RefAdaAST __t408 = _t;
			RefAdaAST tmp231_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ASYNCHRONOUS_SELECT);
			_t = _t->getFirstChild();
			triggering_alternative(_t);
			_t = _retTree;
			abortable_part(_t);
			_t = _retTree;
			_t = __t408;
			_t = _t->getNextSibling();
			break;
		}
		case SELECTIVE_ACCEPT:
		{
			RefAdaAST __t409 = _t;
			RefAdaAST tmp232_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),SELECTIVE_ACCEPT);
			_t = _t->getFirstChild();
			selective_accept(_t);
			_t = _retTree;
			_t = __t409;
			_t = _t->getNextSibling();
			break;
		}
		case TIMED_ENTRY_CALL:
		{
			RefAdaAST __t410 = _t;
			RefAdaAST tmp233_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),TIMED_ENTRY_CALL);
			_t = _t->getFirstChild();
			entry_call_alternative(_t);
			_t = _retTree;
			delay_alternative(_t);
			_t = _retTree;
			_t = __t410;
			_t = _t->getNextSibling();
			break;
		}
		case CONDITIONAL_ENTRY_CALL:
		{
			RefAdaAST __t411 = _t;
			RefAdaAST tmp234_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),CONDITIONAL_ENTRY_CALL);
			_t = _t->getFirstChild();
			entry_call_alternative(_t);
			_t = _retTree;
			statements(_t);
			_t = _retTree;
			_t = __t411;
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
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::if_stmt(RefAdaAST _t) {
	RefAdaAST if_stmt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t347 = _t;
		RefAdaAST tmp235_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IF_STATEMENT);
		_t = _t->getFirstChild();
		cond_clause(_t);
		_t = _retTree;
		elsifs_opt(_t);
		_t = _retTree;
		else_opt(_t);
		_t = _retTree;
		_t = __t347;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::case_stmt(RefAdaAST _t) {
	RefAdaAST case_stmt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t359 = _t;
		RefAdaAST tmp236_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),CASE_STATEMENT);
		_t = _t->getFirstChild();
		expression(_t);
		_t = _retTree;
		alternative_s(_t);
		_t = _retTree;
		_t = __t359;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::loop_stmt(RefAdaAST _t) {
	RefAdaAST loop_stmt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t366 = _t;
		RefAdaAST tmp237_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),LOOP_STATEMENT);
		_t = _t->getFirstChild();
		iteration_scheme_opt(_t);
		_t = _retTree;
		statements(_t);
		_t = _retTree;
		_t = __t366;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::block(RefAdaAST _t) {
	RefAdaAST block_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t373 = _t;
		RefAdaAST tmp238_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),BLOCK_STATEMENT);
		_t = _t->getFirstChild();
		declare_opt(_t);
		_t = _retTree;
		block_body(_t);
		_t = _retTree;
		_t = __t373;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::call_or_assignment(RefAdaAST _t) {
	RefAdaAST call_or_assignment_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case ASSIGNMENT_STATEMENT:
		{
			RefAdaAST __t388 = _t;
			RefAdaAST tmp239_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ASSIGNMENT_STATEMENT);
			_t = _t->getFirstChild();
			name(_t);
			_t = _retTree;
			expression(_t);
			_t = _retTree;
			_t = __t388;
			_t = _t->getNextSibling();
			break;
		}
		case CALL_STATEMENT:
		{
			RefAdaAST __t389 = _t;
			RefAdaAST tmp240_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),CALL_STATEMENT);
			_t = _t->getFirstChild();
			name(_t);
			_t = _retTree;
			_t = __t389;
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
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::cond_clause(RefAdaAST _t) {
	RefAdaAST cond_clause_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t349 = _t;
		RefAdaAST tmp241_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),COND_CLAUSE);
		_t = _t->getFirstChild();
		condition(_t);
		_t = _retTree;
		statements(_t);
		_t = _retTree;
		_t = __t349;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::elsifs_opt(RefAdaAST _t) {
	RefAdaAST elsifs_opt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t352 = _t;
		RefAdaAST tmp242_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ELSIFS_OPT);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_t->getType() == COND_CLAUSE)) {
				cond_clause(_t);
				_t = _retTree;
			}
			else {
				goto _loop354;
			}

		}
		_loop354:;
		} // ( ... )*
		_t = __t352;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::else_opt(RefAdaAST _t) {
	RefAdaAST else_opt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t356 = _t;
		RefAdaAST tmp243_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ELSE_OPT);
		_t = _t->getFirstChild();
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
		_t = __t356;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::condition(RefAdaAST _t) {
	RefAdaAST condition_AST_in = _t;

	try {      // for error handling
		expression(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::alternative_s(RefAdaAST _t) {
	RefAdaAST alternative_s_AST_in = _t;

	try {      // for error handling
		{ // ( ... )+
		int _cnt362=0;
		for (;;) {
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_t->getType() == CASE_STATEMENT_ALTERNATIVE)) {
				case_statement_alternative(_t);
				_t = _retTree;
			}
			else {
				if ( _cnt362>=1 ) { goto _loop362; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));}
			}

			_cnt362++;
		}
		_loop362:;
		}  // ( ... )+
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::case_statement_alternative(RefAdaAST _t) {
	RefAdaAST case_statement_alternative_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t364 = _t;
		RefAdaAST tmp244_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),CASE_STATEMENT_ALTERNATIVE);
		_t = _t->getFirstChild();
		choice_s(_t);
		_t = _retTree;
		statements(_t);
		_t = _retTree;
		_t = __t364;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::iteration_scheme_opt(RefAdaAST _t) {
	RefAdaAST iteration_scheme_opt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t368 = _t;
		RefAdaAST tmp245_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ITERATION_SCHEME_OPT);
		_t = _t->getFirstChild();
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case WHILE:
		{
			RefAdaAST __t370 = _t;
			RefAdaAST tmp246_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),WHILE);
			_t = _t->getFirstChild();
			condition(_t);
			_t = _retTree;
			_t = __t370;
			_t = _t->getNextSibling();
			break;
		}
		case FOR:
		{
			RefAdaAST __t371 = _t;
			RefAdaAST tmp247_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),FOR);
			_t = _t->getFirstChild();
			RefAdaAST tmp248_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
			_t = _t->getNextSibling();
			modifiers(_t);
			_t = _retTree;
			discrete_subtype_definition(_t);
			_t = _retTree;
			_t = __t371;
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
		_t = __t368;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::declare_opt(RefAdaAST _t) {
	RefAdaAST declare_opt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t375 = _t;
		RefAdaAST tmp249_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),DECLARE_OPT);
		_t = _t->getFirstChild();
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
		_t = __t375;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::label_name(RefAdaAST _t) {
	RefAdaAST label_name_AST_in = _t;

	try {      // for error handling
		RefAdaAST tmp250_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::entry_body_formal_part(RefAdaAST _t) {
	RefAdaAST entry_body_formal_part_AST_in = _t;

	try {      // for error handling
		entry_index_spec_opt(_t);
		_t = _retTree;
		formal_part_opt(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::entry_barrier(RefAdaAST _t) {
	RefAdaAST entry_barrier_AST_in = _t;

	try {      // for error handling
		condition(_t);
		_t = _retTree;
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::entry_index_spec_opt(RefAdaAST _t) {
	RefAdaAST entry_index_spec_opt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t394 = _t;
		RefAdaAST tmp251_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ENTRY_INDEX_SPECIFICATION);
		_t = _t->getFirstChild();
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
		_t = __t394;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::entry_call_stmt(RefAdaAST _t) {
	RefAdaAST entry_call_stmt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t398 = _t;
		RefAdaAST tmp252_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ENTRY_CALL_STATEMENT);
		_t = _t->getFirstChild();
		name(_t);
		_t = _retTree;
		_t = __t398;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::entry_index_opt(RefAdaAST _t) {
	RefAdaAST entry_index_opt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t403 = _t;
		RefAdaAST tmp253_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ENTRY_INDEX_OPT);
		_t = _t->getFirstChild();
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
		_t = __t403;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::triggering_alternative(RefAdaAST _t) {
	RefAdaAST triggering_alternative_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t413 = _t;
		RefAdaAST tmp254_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),TRIGGERING_ALTERNATIVE);
		_t = _t->getFirstChild();
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
		stmts_opt(_t);
		_t = _retTree;
		_t = __t413;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::abortable_part(RefAdaAST _t) {
	RefAdaAST abortable_part_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t416 = _t;
		RefAdaAST tmp255_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ABORTABLE_PART);
		_t = _t->getFirstChild();
		stmts_opt(_t);
		_t = _retTree;
		_t = __t416;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::selective_accept(RefAdaAST _t) {
	RefAdaAST selective_accept_AST_in = _t;

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
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::entry_call_alternative(RefAdaAST _t) {
	RefAdaAST entry_call_alternative_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t418 = _t;
		RefAdaAST tmp256_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ENTRY_CALL_ALTERNATIVE);
		_t = _t->getFirstChild();
		entry_call_stmt(_t);
		_t = _retTree;
		stmts_opt(_t);
		_t = _retTree;
		_t = __t418;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::delay_alternative(RefAdaAST _t) {
	RefAdaAST delay_alternative_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t429 = _t;
		RefAdaAST tmp257_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),DELAY_ALTERNATIVE);
		_t = _t->getFirstChild();
		delay_stmt(_t);
		_t = _retTree;
		stmts_opt(_t);
		_t = _retTree;
		_t = __t429;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::stmts_opt(RefAdaAST _t) {
	RefAdaAST stmts_opt_AST_in = _t;

	try {      // for error handling
		{ // ( ... )*
		for (;;) {
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
				goto _loop432;
			}
			}
		}
		_loop432:;
		} // ( ... )*
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::guard_opt(RefAdaAST _t) {
	RefAdaAST guard_opt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t421 = _t;
		RefAdaAST tmp258_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),GUARD_OPT);
		_t = _t->getFirstChild();
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
				if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
					_t = ASTNULL;
				if ((_t->getType() == PRAGMA)) {
					pragma(_t);
					_t = _retTree;
				}
				else {
					goto _loop424;
				}

			}
			_loop424:;
			} // ( ... )*
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
		_t = __t421;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::select_alternative(RefAdaAST _t) {
	RefAdaAST select_alternative_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			RefAdaAST tmp259_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),TERMINATE_ALTERNATIVE);
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
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::or_select_opt(RefAdaAST _t) {
	RefAdaAST or_select_opt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t434 = _t;
		RefAdaAST tmp260_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),OR_SELECT_OPT);
		_t = _t->getFirstChild();
		{ // ( ... )*
		for (;;) {
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			if ((_t->getType() == GUARD_OPT)) {
				guard_opt(_t);
				_t = _retTree;
				select_alternative(_t);
				_t = _retTree;
			}
			else {
				goto _loop436;
			}

		}
		_loop436:;
		} // ( ... )*
		_t = __t434;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::accept_alternative(RefAdaAST _t) {
	RefAdaAST accept_alternative_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t427 = _t;
		RefAdaAST tmp261_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ACCEPT_ALTERNATIVE);
		_t = _t->getFirstChild();
		accept_stmt(_t);
		_t = _retTree;
		stmts_opt(_t);
		_t = _retTree;
		_t = __t427;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::exception_handler(RefAdaAST _t) {
	RefAdaAST exception_handler_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t446 = _t;
		RefAdaAST tmp262_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),EXCEPTION_HANDLER);
		_t = _t->getFirstChild();
		identifier_colon_opt(_t);
		_t = _retTree;
		except_choice_s(_t);
		_t = _retTree;
		statements(_t);
		_t = _retTree;
		_t = __t446;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::identifier_colon_opt(RefAdaAST _t) {
	RefAdaAST identifier_colon_opt_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t448 = _t;
		RefAdaAST tmp263_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER_COLON_OPT);
		_t = _t->getFirstChild();
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENTIFIER:
		{
			RefAdaAST tmp264_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
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
		_t = __t448;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::except_choice_s(RefAdaAST _t) {
	RefAdaAST except_choice_s_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case PIPE:
		{
			RefAdaAST __t451 = _t;
			RefAdaAST tmp265_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PIPE);
			_t = _t->getFirstChild();
			except_choice_s(_t);
			_t = _retTree;
			exception_choice(_t);
			_t = _retTree;
			_t = __t451;
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::exception_choice(RefAdaAST _t) {
	RefAdaAST exception_choice_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			RefAdaAST tmp266_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),OTHERS);
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
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::operator_call(RefAdaAST _t) {
	RefAdaAST operator_call_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t460 = _t;
		RefAdaAST tmp267_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),OPERATOR_SYMBOL);
		_t = _t->getFirstChild();
		value_s(_t);
		_t = _retTree;
		_t = __t460;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::relation(RefAdaAST _t) {
	RefAdaAST relation_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IN:
		{
			RefAdaAST __t472 = _t;
			RefAdaAST tmp268_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IN);
			_t = _t->getFirstChild();
			simple_expression(_t);
			_t = _retTree;
			range_or_mark(_t);
			_t = _retTree;
			_t = __t472;
			_t = _t->getNextSibling();
			break;
		}
		case NOT_IN:
		{
			RefAdaAST __t473 = _t;
			RefAdaAST tmp269_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),NOT_IN);
			_t = _t->getFirstChild();
			simple_expression(_t);
			_t = _retTree;
			range_or_mark(_t);
			_t = _retTree;
			_t = __t473;
			_t = _t->getNextSibling();
			break;
		}
		case EQ:
		{
			RefAdaAST __t474 = _t;
			RefAdaAST tmp270_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),EQ);
			_t = _t->getFirstChild();
			simple_expression(_t);
			_t = _retTree;
			simple_expression(_t);
			_t = _retTree;
			_t = __t474;
			_t = _t->getNextSibling();
			break;
		}
		case NE:
		{
			RefAdaAST __t475 = _t;
			RefAdaAST tmp271_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),NE);
			_t = _t->getFirstChild();
			simple_expression(_t);
			_t = _retTree;
			simple_expression(_t);
			_t = _retTree;
			_t = __t475;
			_t = _t->getNextSibling();
			break;
		}
		case LT_:
		{
			RefAdaAST __t476 = _t;
			RefAdaAST tmp272_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),LT_);
			_t = _t->getFirstChild();
			simple_expression(_t);
			_t = _retTree;
			simple_expression(_t);
			_t = _retTree;
			_t = __t476;
			_t = _t->getNextSibling();
			break;
		}
		case LE:
		{
			RefAdaAST __t477 = _t;
			RefAdaAST tmp273_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),LE);
			_t = _t->getFirstChild();
			simple_expression(_t);
			_t = _retTree;
			simple_expression(_t);
			_t = _retTree;
			_t = __t477;
			_t = _t->getNextSibling();
			break;
		}
		case GT:
		{
			RefAdaAST __t478 = _t;
			RefAdaAST tmp274_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),GT);
			_t = _t->getFirstChild();
			simple_expression(_t);
			_t = _retTree;
			simple_expression(_t);
			_t = _retTree;
			_t = __t478;
			_t = _t->getNextSibling();
			break;
		}
		case GE:
		{
			RefAdaAST __t479 = _t;
			RefAdaAST tmp275_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),GE);
			_t = _t->getFirstChild();
			simple_expression(_t);
			_t = _retTree;
			simple_expression(_t);
			_t = _retTree;
			_t = __t479;
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::range_or_mark(RefAdaAST _t) {
	RefAdaAST range_or_mark_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::signed_term(RefAdaAST _t) {
	RefAdaAST signed_term_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case UNARY_PLUS:
		{
			RefAdaAST __t486 = _t;
			RefAdaAST tmp276_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),UNARY_PLUS);
			_t = _t->getFirstChild();
			term(_t);
			_t = _retTree;
			_t = __t486;
			_t = _t->getNextSibling();
			break;
		}
		case UNARY_MINUS:
		{
			RefAdaAST __t487 = _t;
			RefAdaAST tmp277_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),UNARY_MINUS);
			_t = _t->getFirstChild();
			term(_t);
			_t = _retTree;
			_t = __t487;
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::term(RefAdaAST _t) {
	RefAdaAST term_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case STAR:
		{
			RefAdaAST __t489 = _t;
			RefAdaAST tmp278_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),STAR);
			_t = _t->getFirstChild();
			term(_t);
			_t = _retTree;
			factor(_t);
			_t = _retTree;
			_t = __t489;
			_t = _t->getNextSibling();
			break;
		}
		case DIV:
		{
			RefAdaAST __t490 = _t;
			RefAdaAST tmp279_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),DIV);
			_t = _t->getFirstChild();
			term(_t);
			_t = _retTree;
			factor(_t);
			_t = _retTree;
			_t = __t490;
			_t = _t->getNextSibling();
			break;
		}
		case MOD:
		{
			RefAdaAST __t491 = _t;
			RefAdaAST tmp280_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),MOD);
			_t = _t->getFirstChild();
			term(_t);
			_t = _retTree;
			factor(_t);
			_t = _retTree;
			_t = __t491;
			_t = _t->getNextSibling();
			break;
		}
		case REM:
		{
			RefAdaAST __t492 = _t;
			RefAdaAST tmp281_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),REM);
			_t = _t->getFirstChild();
			term(_t);
			_t = _retTree;
			factor(_t);
			_t = _retTree;
			_t = __t492;
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::factor(RefAdaAST _t) {
	RefAdaAST factor_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case NOT:
		{
			RefAdaAST __t494 = _t;
			RefAdaAST tmp282_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),NOT);
			_t = _t->getFirstChild();
			primary(_t);
			_t = _retTree;
			_t = __t494;
			_t = _t->getNextSibling();
			break;
		}
		case ABS:
		{
			RefAdaAST __t495 = _t;
			RefAdaAST tmp283_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ABS);
			_t = _t->getFirstChild();
			primary(_t);
			_t = _retTree;
			_t = __t495;
			_t = _t->getNextSibling();
			break;
		}
		case EXPON:
		{
			RefAdaAST __t496 = _t;
			RefAdaAST tmp284_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),EXPON);
			_t = _t->getFirstChild();
			primary(_t);
			_t = _retTree;
			primary(_t);
			_t = _retTree;
			_t = __t496;
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::primary(RefAdaAST _t) {
	RefAdaAST primary_AST_in = _t;

	try {      // for error handling
		{
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			RefAdaAST tmp285_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),NuLL);
			_t = _t->getNextSibling();
			break;
		}
		case NUMERIC_LIT:
		{
			RefAdaAST tmp286_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),NUMERIC_LIT);
			_t = _t->getNextSibling();
			break;
		}
		case CHARACTER_LITERAL:
		{
			RefAdaAST tmp287_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),CHARACTER_LITERAL);
			_t = _t->getNextSibling();
			break;
		}
		case CHAR_STRING:
		{
			RefAdaAST tmp288_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),CHAR_STRING);
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::name_or_qualified(RefAdaAST _t) {
	RefAdaAST name_or_qualified_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = ASTNULL;
		switch ( _t->getType()) {
		case IDENTIFIER:
		{
			RefAdaAST tmp289_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
			_t = _t->getNextSibling();
			break;
		}
		case DOT:
		{
			RefAdaAST __t500 = _t;
			RefAdaAST tmp290_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),DOT);
			_t = _t->getFirstChild();
			name_or_qualified(_t);
			_t = _retTree;
			{
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
				_t = ASTNULL;
			switch ( _t->getType()) {
			case ALL:
			{
				RefAdaAST tmp291_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ALL);
				_t = _t->getNextSibling();
				break;
			}
			case IDENTIFIER:
			{
				RefAdaAST tmp292_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),IDENTIFIER);
				_t = _t->getNextSibling();
				break;
			}
			case CHARACTER_LITERAL:
			{
				RefAdaAST tmp293_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),CHARACTER_LITERAL);
				_t = _t->getNextSibling();
				break;
			}
			case OPERATOR_SYMBOL:
			{
				RefAdaAST tmp294_AST_in = _t;
				match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),OPERATOR_SYMBOL);
				_t = _t->getNextSibling();
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
			}
			}
			}
			_t = __t500;
			_t = _t->getNextSibling();
			break;
		}
		case INDEXED_COMPONENT:
		{
			RefAdaAST __t502 = _t;
			RefAdaAST tmp295_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),INDEXED_COMPONENT);
			_t = _t->getFirstChild();
			name_or_qualified(_t);
			_t = _retTree;
			value_s(_t);
			_t = _retTree;
			_t = __t502;
			_t = _t->getNextSibling();
			break;
		}
		case TIC:
		{
			RefAdaAST __t503 = _t;
			RefAdaAST tmp296_AST_in = _t;
			match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),TIC);
			_t = _t->getFirstChild();
			name_or_qualified(_t);
			_t = _retTree;
			{
			if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
			}
			}
			}
			_t = __t503;
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
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::allocator(RefAdaAST _t) {
	RefAdaAST allocator_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t506 = _t;
		RefAdaAST tmp297_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),ALLOCATOR);
		_t = _t->getFirstChild();
		name_or_qualified(_t);
		_t = _retTree;
		_t = __t506;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::subprogram_body(RefAdaAST _t) {
	RefAdaAST subprogram_body_AST_in = _t;

	try {      // for error handling
		if (_t == static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()));
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::task_body(RefAdaAST _t) {
	RefAdaAST task_body_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t512 = _t;
		RefAdaAST tmp298_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),TASK_BODY);
		_t = _t->getFirstChild();
		def_id(_t);
		_t = _retTree;
		body_part(_t);
		_t = _retTree;
		_t = __t512;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

void AdaStoreWalker::protected_body(RefAdaAST _t) {
	RefAdaAST protected_body_AST_in = _t;

	try {      // for error handling
		RefAdaAST __t514 = _t;
		RefAdaAST tmp299_AST_in = _t;
		match(static_cast<ANTLR_USE_NAMESPACE(antlr)RefAST>(_t.get()),PROTECTED_BODY);
		_t = _t->getFirstChild();
		def_id(_t);
		_t = _retTree;
		prot_op_bodies_opt(_t);
		_t = _retTree;
		_t = __t514;
		_t = _t->getNextSibling();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		if ( _t != static_cast<RefAdaAST>(ANTLR_USE_NAMESPACE(antlr)nullAST.get()) )
			_t = _t->getNextSibling();
	}
	_retTree = _t;
}

RefAdaAST AdaStoreWalker::getAST()
{
	return returnAST;
}

void AdaStoreWalker::initializeASTFactory( ANTLR_USE_NAMESPACE(antlr)ASTFactory& // factory
                                           )
{
}
const char* AdaStoreWalker::tokenNames[] = {
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

const unsigned long AdaStoreWalker::_tokenSet_0_data_[] = { 37920UL, 262201UL, 4293001216UL, 4223UL, 134217728UL, 16UL, 67108864UL, 0UL, 5373952UL, 48UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER RIGHT_SHAFT DOT TIC "in" CHARACTER_LITERAL CHAR_STRING "null"
// "mod" "or" "and" "xor" "not" EQ NE LT_ LE GT GE PLUS MINUS CONCAT STAR
// DIV "rem" "abs" EXPON NUMERIC_LIT ALLOCATOR INDEXED_COMPONENT OPERATOR_SYMBOL
// AND_THEN NOT_IN OR_ELSE PARENTHESIZED_PRIMARY UNARY_MINUS UNARY_PLUS
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaStoreWalker::_tokenSet_0(_tokenSet_0_data_,20);
const unsigned long AdaStoreWalker::_tokenSet_1_data_[] = { 0UL, 0UL, 0UL, 98304UL, 68168704UL, 403845518UL, 58482948UL, 3758133268UL, 235700479UL, 5UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
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
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaStoreWalker::_tokenSet_1(_tokenSet_1_data_,20);
const unsigned long AdaStoreWalker::_tokenSet_2_data_[] = { 939627552UL, 262201UL, 4293001216UL, 4223UL, 134217728UL, 16UL, 67108864UL, 0UL, 5373952UL, 48UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER RIGHT_SHAFT DOT TIC "range" "others" PIPE DOT_DOT "in" CHARACTER_LITERAL
// CHAR_STRING "null" "mod" "or" "and" "xor" "not" EQ NE LT_ LE GT GE PLUS
// MINUS CONCAT STAR DIV "rem" "abs" EXPON NUMERIC_LIT ALLOCATOR INDEXED_COMPONENT
// OPERATOR_SYMBOL AND_THEN NOT_IN OR_ELSE PARENTHESIZED_PRIMARY UNARY_MINUS
// UNARY_PLUS
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaStoreWalker::_tokenSet_2(_tokenSet_2_data_,20);
const unsigned long AdaStoreWalker::_tokenSet_3_data_[] = { 16UL, 0UL, 0UL, 98304UL, 1280UL, 1048576UL, 0UL, 1073741824UL, 67108864UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "pragma" ATTRIBUTE_DEFINITION_CLAUSE AT_CLAUSE ENTRY_DECLARATION ENUMERATION_REPESENTATION_CLAUSE
// RECORD_REPRESENTATION_CLAUSE FUNCTION_DECLARATION PROCEDURE_DECLARATION
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaStoreWalker::_tokenSet_3(_tokenSet_3_data_,20);
const unsigned long AdaStoreWalker::_tokenSet_4_data_[] = { 36896UL, 0UL, 0UL, 0UL, 134217728UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER DOT TIC INDEXED_COMPONENT
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaStoreWalker::_tokenSet_4(_tokenSet_4_data_,12);


