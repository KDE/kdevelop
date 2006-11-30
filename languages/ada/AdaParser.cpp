/* $ANTLR 2.7.7 (20061129): "ada.g" -> "AdaParser.cpp"$ */
#include "AdaParser.hpp"
#include <antlr/NoViableAltException.hpp>
#include <antlr/SemanticException.hpp>
#include <antlr/ASTFactory.hpp>
#line 1 "ada.g"
#line 8 "AdaParser.cpp"
AdaParser::AdaParser(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf, int k)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(tokenBuf,k)
{
}

AdaParser::AdaParser(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(tokenBuf,2)
{
}

AdaParser::AdaParser(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer, int k)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(lexer,k)
{
}

AdaParser::AdaParser(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(lexer,2)
{
}

AdaParser::AdaParser(const ANTLR_USE_NAMESPACE(antlr)ParserSharedInputState& state)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(state,2)
{
}

void AdaParser::compilation_unit() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST compilation_unit_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		context_items_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case PRIVATE:
		case PACKAGE:
		case PROCEDURE:
		case FUNCTION:
		case GENERIC:
		{
			library_item();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case SEPARATE:
		{
			subunit();
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
			if ((LA(1) == PRAGMA)) {
				pragma();
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
		compilation_unit_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_0);
		} else {
			throw;
		}
	}
	returnAST = compilation_unit_AST;
}

void AdaParser::context_items_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST context_items_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == PRAGMA)) {
				pragma();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop14;
			}
			
		}
		_loop14:;
		} // ( ... )*
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == WITH)) {
				with_clause();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				{ // ( ... )*
				for (;;) {
					switch ( LA(1)) {
					case USE:
					{
						use_clause();
						if (inputState->guessing==0) {
							astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
						}
						break;
					}
					case PRAGMA:
					{
						pragma();
						if (inputState->guessing==0) {
							astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
						}
						break;
					}
					default:
					{
						goto _loop17;
					}
					}
				}
				_loop17:;
				} // ( ... )*
			}
			else {
				goto _loop18;
			}
			
		}
		_loop18:;
		} // ( ... )*
		if ( inputState->guessing==0 ) {
			context_items_opt_AST = RefAdaAST(currentAST.root);
#line 86 "ada.g"
			context_items_opt_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(CONTEXT_CLAUSE,"CONTEXT_CLAUSE")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(context_items_opt_AST))));
#line 168 "AdaParser.cpp"
			currentAST.root = context_items_opt_AST;
			if ( context_items_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				context_items_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = context_items_opt_AST->getFirstChild();
			else
				currentAST.child = context_items_opt_AST;
			currentAST.advanceChildToEnd();
		}
		context_items_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_1);
		} else {
			throw;
		}
	}
	returnAST = context_items_opt_AST;
}

void AdaParser::library_item() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST library_item_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		private_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case PACKAGE:
		{
			lib_pkg_spec_or_body();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case PROCEDURE:
		case FUNCTION:
		{
			subprog_decl_or_rename_or_inst_or_body(true);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case GENERIC:
		{
			generic_decl(true);
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
		if ( inputState->guessing==0 ) {
			library_item_AST = RefAdaAST(currentAST.root);
#line 133 "ada.g"
			library_item_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(LIBRARY_ITEM,"LIBRARY_ITEM")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(library_item_AST))));
#line 237 "AdaParser.cpp"
			currentAST.root = library_item_AST;
			if ( library_item_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				library_item_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = library_item_AST->getFirstChild();
			else
				currentAST.child = library_item_AST;
			currentAST.advanceChildToEnd();
		}
		library_item_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_2);
		} else {
			throw;
		}
	}
	returnAST = library_item_AST;
}

void AdaParser::subunit() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST subunit_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  sep = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST sep_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		sep = LT(1);
		if ( inputState->guessing == 0 ) {
			sep_AST = astFactory->create(sep);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(sep_AST));
		}
		match(SEPARATE);
		match(LPAREN);
		compound_name();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(RPAREN);
		if ( inputState->guessing==0 ) {
#line 1472 "ada.g"
			Set(sep_AST, SUBUNIT);
#line 282 "AdaParser.cpp"
		}
		{
		switch ( LA(1)) {
		case PROCEDURE:
		case FUNCTION:
		{
			subprogram_body();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case PACKAGE:
		{
			package_body();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case TASK:
		{
			task_body();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case PROTECTED:
		{
			protected_body();
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
		subunit_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_2);
		} else {
			throw;
		}
	}
	returnAST = subunit_AST;
}

void AdaParser::pragma() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST pragma_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefAdaAST tmp3_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp3_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp3_AST));
		}
		match(PRAGMA);
		RefAdaAST tmp4_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp4_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp4_AST));
		}
		match(IDENTIFIER);
		pragma_args_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(SEMI);
		pragma_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_3);
		} else {
			throw;
		}
	}
	returnAST = pragma_AST;
}

void AdaParser::pragma_args_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST pragma_args_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LPAREN:
		{
			match(LPAREN);
			pragma_arg();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{ // ( ... )*
			for (;;) {
				if ((LA(1) == COMMA)) {
					match(COMMA);
					pragma_arg();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
				}
				else {
					goto _loop9;
				}
				
			}
			_loop9:;
			} // ( ... )*
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
		pragma_args_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_4);
		} else {
			throw;
		}
	}
	returnAST = pragma_args_opt_AST;
}

void AdaParser::pragma_arg() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST pragma_arg_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		if ((LA(1) == IDENTIFIER) && (LA(2) == RIGHT_SHAFT)) {
			RefAdaAST tmp9_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp9_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp9_AST));
			}
			match(IDENTIFIER);
			RefAdaAST tmp10_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp10_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp10_AST));
			}
			match(RIGHT_SHAFT);
		}
		else if ((_tokenSet_5.member(LA(1))) && (_tokenSet_6.member(LA(2)))) {
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		pragma_arg_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_7);
		} else {
			throw;
		}
	}
	returnAST = pragma_arg_AST;
}

void AdaParser::expression() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST expression_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  a = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST a_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  o = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST o_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		relation();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			switch ( LA(1)) {
			case AND:
			{
				a = LT(1);
				if ( inputState->guessing == 0 ) {
					a_AST = astFactory->create(a);
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(a_AST));
				}
				match(AND);
				{
				switch ( LA(1)) {
				case THEN:
				{
					match(THEN);
					if ( inputState->guessing==0 ) {
#line 1392 "ada.g"
						Set (a_AST, AND_THEN);
#line 509 "AdaParser.cpp"
					}
					break;
				}
				case IDENTIFIER:
				case LPAREN:
				case NEW:
				case CHARACTER_LITERAL:
				case CHAR_STRING:
				case NuLL:
				case NOT:
				case PLUS:
				case MINUS:
				case ABS:
				case NUMERIC_LIT:
				{
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				relation();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case OR:
			{
				o = LT(1);
				if ( inputState->guessing == 0 ) {
					o_AST = astFactory->create(o);
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(o_AST));
				}
				match(OR);
				{
				switch ( LA(1)) {
				case ELSE:
				{
					match(ELSE);
					if ( inputState->guessing==0 ) {
#line 1393 "ada.g"
						Set (o_AST, OR_ELSE);
#line 555 "AdaParser.cpp"
					}
					break;
				}
				case IDENTIFIER:
				case LPAREN:
				case NEW:
				case CHARACTER_LITERAL:
				case CHAR_STRING:
				case NuLL:
				case NOT:
				case PLUS:
				case MINUS:
				case ABS:
				case NUMERIC_LIT:
				{
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				relation();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case XOR:
			{
				RefAdaAST tmp13_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp13_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp13_AST));
				}
				match(XOR);
				relation();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			default:
			{
				goto _loop445;
			}
			}
		}
		_loop445:;
		} // ( ... )*
		expression_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_8);
		} else {
			throw;
		}
	}
	returnAST = expression_AST;
}

void AdaParser::with_clause() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST with_clause_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  w = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST w_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		w = LT(1);
		if ( inputState->guessing == 0 ) {
			w_AST = astFactory->create(w);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(w_AST));
		}
		match(WITH);
		c_name_list();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(SEMI);
		if ( inputState->guessing==0 ) {
#line 94 "ada.g"
			Set(w_AST, WITH_CLAUSE);
#line 642 "AdaParser.cpp"
		}
		with_clause_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_9);
		} else {
			throw;
		}
	}
	returnAST = with_clause_AST;
}

void AdaParser::use_clause() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST use_clause_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  u = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST u_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		u = LT(1);
		if ( inputState->guessing == 0 ) {
			u_AST = astFactory->create(u);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(u_AST));
		}
		match(USE);
		{
		switch ( LA(1)) {
		case TYPE:
		{
			match(TYPE);
			subtype_mark();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{ // ( ... )*
			for (;;) {
				if ((LA(1) == COMMA)) {
					match(COMMA);
					subtype_mark();
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
			if ( inputState->guessing==0 ) {
#line 108 "ada.g"
				Set(u_AST, USE_TYPE_CLAUSE);
#line 699 "AdaParser.cpp"
			}
			break;
		}
		case IDENTIFIER:
		{
			c_name_list();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			if ( inputState->guessing==0 ) {
#line 109 "ada.g"
				Set(u_AST, USE_CLAUSE);
#line 712 "AdaParser.cpp"
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
		use_clause_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_10);
		} else {
			throw;
		}
	}
	returnAST = use_clause_AST;
}

void AdaParser::c_name_list() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST c_name_list_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		compound_name();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				compound_name();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop22;
			}
			
		}
		_loop22:;
		} // ( ... )*
		c_name_list_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_4);
		} else {
			throw;
		}
	}
	returnAST = c_name_list_AST;
}

void AdaParser::compound_name() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST compound_name_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefAdaAST tmp19_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp19_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp19_AST));
		}
		match(IDENTIFIER);
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == DOT)) {
				RefAdaAST tmp20_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp20_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp20_AST));
				}
				match(DOT);
				RefAdaAST tmp21_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp21_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp21_AST));
				}
				match(IDENTIFIER);
			}
			else {
				goto _loop25;
			}
			
		}
		_loop25:;
		} // ( ... )*
		compound_name_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_11);
		} else {
			throw;
		}
	}
	returnAST = compound_name_AST;
}

void AdaParser::subtype_mark() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST subtype_mark_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		compound_name();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case TIC:
		{
			RefAdaAST tmp22_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp22_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp22_AST));
			}
			match(TIC);
			attribute_id();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case SEMI:
		case LPAREN:
		case COMMA:
		case RPAREN:
		case RIGHT_SHAFT:
		case WITH:
		case USE:
		case RANGE:
		case DIGITS:
		case DELTA:
		case IS:
		case PIPE:
		case DOT_DOT:
		case RENAMES:
		case ASSIGN:
		case AT:
		case THEN:
		case LOOP:
		case OR:
		case AND:
		case XOR:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		subtype_mark_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_12);
		} else {
			throw;
		}
	}
	returnAST = subtype_mark_AST;
}

void AdaParser::attribute_id() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST attribute_id_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case RANGE:
		{
			RefAdaAST tmp23_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp23_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp23_AST));
			}
			match(RANGE);
			attribute_id_AST = RefAdaAST(currentAST.root);
			break;
		}
		case DIGITS:
		{
			RefAdaAST tmp24_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp24_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp24_AST));
			}
			match(DIGITS);
			attribute_id_AST = RefAdaAST(currentAST.root);
			break;
		}
		case DELTA:
		{
			RefAdaAST tmp25_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp25_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp25_AST));
			}
			match(DELTA);
			attribute_id_AST = RefAdaAST(currentAST.root);
			break;
		}
		case ACCESS:
		{
			RefAdaAST tmp26_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp26_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp26_AST));
			}
			match(ACCESS);
			attribute_id_AST = RefAdaAST(currentAST.root);
			break;
		}
		case IDENTIFIER:
		{
			RefAdaAST tmp27_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp27_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp27_AST));
			}
			match(IDENTIFIER);
			attribute_id_AST = RefAdaAST(currentAST.root);
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
	returnAST = attribute_id_AST;
}

void AdaParser::private_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST private_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case PRIVATE:
		{
			RefAdaAST tmp28_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp28_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp28_AST));
			}
			match(PRIVATE);
			break;
		}
		case PACKAGE:
		case PROCEDURE:
		case FUNCTION:
		case GENERIC:
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
			private_opt_AST = RefAdaAST(currentAST.root);
#line 137 "ada.g"
			private_opt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(MODIFIERS,"MODIFIERS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(private_opt_AST))));
#line 1006 "AdaParser.cpp"
			currentAST.root = private_opt_AST;
			if ( private_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				private_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = private_opt_AST->getFirstChild();
			else
				currentAST.child = private_opt_AST;
			currentAST.advanceChildToEnd();
		}
		private_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_14);
		} else {
			throw;
		}
	}
	returnAST = private_opt_AST;
}

void AdaParser::lib_pkg_spec_or_body() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST lib_pkg_spec_or_body_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  pkg = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST pkg_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		pkg = LT(1);
		if ( inputState->guessing == 0 ) {
			pkg_AST = astFactory->create(pkg);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(pkg_AST));
		}
		match(PACKAGE);
		{
		switch ( LA(1)) {
		case BODY:
		{
			match(BODY);
			def_id(true);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(IS);
			pkg_body_part();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			end_id_opt();
			match(SEMI);
			if ( inputState->guessing==0 ) {
#line 143 "ada.g"
				Set(pkg_AST, PACKAGE_BODY);
#line 1061 "AdaParser.cpp"
			}
			break;
		}
		case IDENTIFIER:
		{
			def_id(true);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			spec_decl_part(pkg_AST);
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
		lib_pkg_spec_or_body_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_2);
		} else {
			throw;
		}
	}
	returnAST = lib_pkg_spec_or_body_AST;
}

void AdaParser::subprog_decl_or_rename_or_inst_or_body(
	bool lib_level
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST subprog_decl_or_rename_or_inst_or_body_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  p = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST p_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  f = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST f_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
#line 930 "ada.g"
	RefAdaAST t;
#line 1108 "AdaParser.cpp"
	
	try {      // for error handling
		switch ( LA(1)) {
		case PROCEDURE:
		{
			p = LT(1);
			if ( inputState->guessing == 0 ) {
				p_AST = astFactory->create(p);
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(p_AST));
			}
			match(PROCEDURE);
			def_id(lib_level);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{
			if ((LA(1) == IS) && (LA(2) == NEW)) {
				generic_subp_inst();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				if ( inputState->guessing==0 ) {
#line 934 "ada.g"
					Set(p_AST, GENERIC_PROCEDURE_INSTANTIATION);
#line 1133 "AdaParser.cpp"
				}
			}
			else if ((_tokenSet_15.member(LA(1))) && (_tokenSet_16.member(LA(2)))) {
				formal_part_opt();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				{
				switch ( LA(1)) {
				case RENAMES:
				{
					renames();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					if ( inputState->guessing==0 ) {
#line 936 "ada.g"
						Set(p_AST, PROCEDURE_RENAMING_DECLARATION);
#line 1152 "AdaParser.cpp"
					}
					break;
				}
				case IS:
				{
					match(IS);
					{
					switch ( LA(1)) {
					case SEPARATE:
					case ABSTRACT:
					{
						separate_or_abstract(p_AST);
						if (inputState->guessing==0) {
							astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
						}
						break;
					}
					case PRAGMA:
					case IDENTIFIER:
					case USE:
					case TYPE:
					case PACKAGE:
					case PROCEDURE:
					case FUNCTION:
					case TASK:
					case PROTECTED:
					case FOR:
					case SUBTYPE:
					case GENERIC:
					case BEGIN:
					{
						body_part();
						if (inputState->guessing==0) {
							astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
						}
						if ( inputState->guessing==0 ) {
#line 938 "ada.g"
							Set(p_AST, PROCEDURE_BODY);
#line 1191 "AdaParser.cpp"
						}
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
				{
					if ( inputState->guessing==0 ) {
#line 940 "ada.g"
						pop_def_id();
									    Set(p_AST, PROCEDURE_DECLARATION);
#line 1209 "AdaParser.cpp"
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
			}
			else {
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			
			}
			subprog_decl_or_rename_or_inst_or_body_AST = RefAdaAST(currentAST.root);
			break;
		}
		case FUNCTION:
		{
			f = LT(1);
			if ( inputState->guessing == 0 ) {
				f_AST = astFactory->create(f);
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(f_AST));
			}
			match(FUNCTION);
			def_designator(lib_level);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{
			switch ( LA(1)) {
			case IS:
			{
				generic_subp_inst();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				if ( inputState->guessing==0 ) {
#line 947 "ada.g"
					Set(f_AST, GENERIC_FUNCTION_INSTANTIATION);
#line 1252 "AdaParser.cpp"
				}
				break;
			}
			case LPAREN:
			case RETURN:
			{
				function_tail();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				{
				switch ( LA(1)) {
				case RENAMES:
				{
					renames();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					if ( inputState->guessing==0 ) {
#line 949 "ada.g"
						Set(f_AST, FUNCTION_RENAMING_DECLARATION);
#line 1274 "AdaParser.cpp"
					}
					break;
				}
				case IS:
				{
					match(IS);
					{
					switch ( LA(1)) {
					case SEPARATE:
					case ABSTRACT:
					{
						separate_or_abstract(f_AST);
						if (inputState->guessing==0) {
							astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
						}
						break;
					}
					case PRAGMA:
					case IDENTIFIER:
					case USE:
					case TYPE:
					case PACKAGE:
					case PROCEDURE:
					case FUNCTION:
					case TASK:
					case PROTECTED:
					case FOR:
					case SUBTYPE:
					case GENERIC:
					case BEGIN:
					{
						body_part();
						if (inputState->guessing==0) {
							astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
						}
						if ( inputState->guessing==0 ) {
#line 951 "ada.g"
							Set(f_AST, FUNCTION_BODY);
#line 1313 "AdaParser.cpp"
						}
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
				{
					if ( inputState->guessing==0 ) {
#line 953 "ada.g"
						pop_def_id();
									    Set(f_AST, FUNCTION_DECLARATION);
#line 1331 "AdaParser.cpp"
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
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			subprog_decl_or_rename_or_inst_or_body_AST = RefAdaAST(currentAST.root);
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
	returnAST = subprog_decl_or_rename_or_inst_or_body_AST;
}

void AdaParser::generic_decl(
	bool lib_level
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST generic_decl_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  g = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST g_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		g = LT(1);
		if ( inputState->guessing == 0 ) {
			g_AST = astFactory->create(g);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(g_AST));
		}
		match(GENERIC);
		generic_formal_part_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case PACKAGE:
		{
			match(PACKAGE);
			def_id(lib_level);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{
			switch ( LA(1)) {
			case RENAMES:
			{
				renames();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				if ( inputState->guessing==0 ) {
#line 851 "ada.g"
					Set(g_AST, GENERIC_PACKAGE_RENAMING);
#line 1410 "AdaParser.cpp"
				}
				break;
			}
			case IS:
			{
				match(IS);
				pkg_spec_part();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				if ( inputState->guessing==0 ) {
#line 852 "ada.g"
					Set(g_AST, GENERIC_PACKAGE_DECLARATION);
#line 1424 "AdaParser.cpp"
				}
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
		case PROCEDURE:
		{
			match(PROCEDURE);
			def_id(lib_level);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			formal_part_opt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{
			switch ( LA(1)) {
			case RENAMES:
			{
				renames();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				if ( inputState->guessing==0 ) {
#line 855 "ada.g"
					Set(g_AST, GENERIC_PROCEDURE_RENAMING);
#line 1458 "AdaParser.cpp"
				}
				break;
			}
			case SEMI:
			{
				if ( inputState->guessing==0 ) {
#line 858 "ada.g"
					Set(g_AST, GENERIC_PROCEDURE_DECLARATION); pop_def_id();
#line 1467 "AdaParser.cpp"
				}
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
		case FUNCTION:
		{
			match(FUNCTION);
			def_designator(lib_level);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			function_tail();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{
			switch ( LA(1)) {
			case RENAMES:
			{
				renames();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				if ( inputState->guessing==0 ) {
#line 861 "ada.g"
					Set(g_AST, GENERIC_FUNCTION_RENAMING);
#line 1501 "AdaParser.cpp"
				}
				break;
			}
			case SEMI:
			{
				if ( inputState->guessing==0 ) {
#line 864 "ada.g"
					Set(g_AST, GENERIC_FUNCTION_DECLARATION); pop_def_id();
#line 1510 "AdaParser.cpp"
				}
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
		match(SEMI);
		generic_decl_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_18);
		} else {
			throw;
		}
	}
	returnAST = generic_decl_AST;
}

void AdaParser::def_id(
	bool lib_level
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST def_id_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefAdaAST cn_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  n = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST n_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		if (((LA(1) == IDENTIFIER) && (_tokenSet_19.member(LA(2))))&&( lib_level )) {
			compound_name();
			if (inputState->guessing==0) {
				cn_AST = returnAST;
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			if ( inputState->guessing==0 ) {
#line 171 "ada.g"
				push_def_id(cn_AST);
#line 1562 "AdaParser.cpp"
			}
			def_id_AST = RefAdaAST(currentAST.root);
		}
		else if (((LA(1) == IDENTIFIER) && (_tokenSet_20.member(LA(2))))&&( !lib_level )) {
			n = LT(1);
			if ( inputState->guessing == 0 ) {
				n_AST = astFactory->create(n);
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(n_AST));
			}
			match(IDENTIFIER);
			if ( inputState->guessing==0 ) {
#line 172 "ada.g"
				push_def_id(n_AST);
#line 1576 "AdaParser.cpp"
			}
			def_id_AST = RefAdaAST(currentAST.root);
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
	returnAST = def_id_AST;
}

void AdaParser::pkg_body_part() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST pkg_body_part_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		declarative_part();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		block_body_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		pkg_body_part_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_21);
		} else {
			throw;
		}
	}
	returnAST = pkg_body_part_AST;
}

void AdaParser::end_id_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST end_id_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(END);
		id_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		end_id_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_4);
		} else {
			throw;
		}
	}
	returnAST = end_id_opt_AST;
}

void AdaParser::spec_decl_part(
	RefAdaAST pkg
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST spec_decl_part_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case IS:
		{
			match(IS);
			{
			switch ( LA(1)) {
			case NEW:
			{
				generic_inst();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				if ( inputState->guessing==0 ) {
#line 363 "ada.g"
					Set(pkg, GENERIC_PACKAGE_INSTANTIATION);
#line 1671 "AdaParser.cpp"
				}
				break;
			}
			case PRAGMA:
			case IDENTIFIER:
			case USE:
			case TYPE:
			case PRIVATE:
			case PACKAGE:
			case PROCEDURE:
			case FUNCTION:
			case TASK:
			case PROTECTED:
			case FOR:
			case END:
			case SUBTYPE:
			case GENERIC:
			{
				pkg_spec_part();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				if ( inputState->guessing==0 ) {
#line 364 "ada.g"
					Set(pkg, PACKAGE_SPECIFICATION);
#line 1697 "AdaParser.cpp"
				}
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
		case RENAMES:
		{
			renames();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			if ( inputState->guessing==0 ) {
#line 366 "ada.g"
				Set(pkg, PACKAGE_RENAMING_DECLARATION);
#line 1718 "AdaParser.cpp"
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
		spec_decl_part_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_18);
		} else {
			throw;
		}
	}
	returnAST = spec_decl_part_AST;
}

void AdaParser::subprog_decl(
	bool lib_level
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST subprog_decl_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  p = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST p_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  f = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST f_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
#line 148 "ada.g"
	RefAdaAST t;
#line 1754 "AdaParser.cpp"
	
	try {      // for error handling
		switch ( LA(1)) {
		case PROCEDURE:
		{
			p = LT(1);
			if ( inputState->guessing == 0 ) {
				p_AST = astFactory->create(p);
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(p_AST));
			}
			match(PROCEDURE);
			def_id(lib_level);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{
			if ((LA(1) == IS) && (LA(2) == NEW)) {
				generic_subp_inst();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				if ( inputState->guessing==0 ) {
#line 152 "ada.g"
					Set(p_AST, GENERIC_PROCEDURE_INSTANTIATION);
#line 1779 "AdaParser.cpp"
				}
			}
			else if ((_tokenSet_15.member(LA(1))) && (_tokenSet_22.member(LA(2)))) {
				formal_part_opt();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				{
				switch ( LA(1)) {
				case RENAMES:
				{
					renames();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					if ( inputState->guessing==0 ) {
#line 154 "ada.g"
						Set(p_AST, PROCEDURE_RENAMING_DECLARATION);
#line 1798 "AdaParser.cpp"
					}
					break;
				}
				case SEMI:
				case IS:
				{
					is_separate_or_abstract_or_decl(p_AST);
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
			}
			else {
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			
			}
			subprog_decl_AST = RefAdaAST(currentAST.root);
			break;
		}
		case FUNCTION:
		{
			f = LT(1);
			if ( inputState->guessing == 0 ) {
				f_AST = astFactory->create(f);
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(f_AST));
			}
			match(FUNCTION);
			def_designator(lib_level);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{
			switch ( LA(1)) {
			case IS:
			{
				generic_subp_inst();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				if ( inputState->guessing==0 ) {
#line 161 "ada.g"
					Set(f_AST, GENERIC_FUNCTION_INSTANTIATION);
#line 1850 "AdaParser.cpp"
				}
				break;
			}
			case LPAREN:
			case RETURN:
			{
				function_tail();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				{
				switch ( LA(1)) {
				case RENAMES:
				{
					renames();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					if ( inputState->guessing==0 ) {
#line 163 "ada.g"
						Set(f_AST, FUNCTION_RENAMING_DECLARATION);
#line 1872 "AdaParser.cpp"
					}
					break;
				}
				case SEMI:
				case IS:
				{
					is_separate_or_abstract_or_decl(f_AST);
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
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			subprog_decl_AST = RefAdaAST(currentAST.root);
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
	returnAST = subprog_decl_AST;
}

void AdaParser::generic_subp_inst() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST generic_subp_inst_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(IS);
		generic_inst();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(SEMI);
		generic_subp_inst_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_18);
		} else {
			throw;
		}
	}
	returnAST = generic_subp_inst_AST;
}

void AdaParser::formal_part_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST formal_part_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LPAREN:
		{
			match(LPAREN);
			parameter_specification();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{ // ( ... )*
			for (;;) {
				if ((LA(1) == SEMI)) {
					match(SEMI);
					parameter_specification();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
				}
				else {
					goto _loop74;
				}
				
			}
			_loop74:;
			} // ( ... )*
			match(RPAREN);
			break;
		}
		case SEMI:
		case IS:
		case RENAMES:
		case WHEN:
		case DO:
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
			formal_part_opt_AST = RefAdaAST(currentAST.root);
#line 234 "ada.g"
			formal_part_opt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(FORMAL_PART_OPT,"FORMAL_PART_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(formal_part_opt_AST))));
#line 1997 "AdaParser.cpp"
			currentAST.root = formal_part_opt_AST;
			if ( formal_part_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				formal_part_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = formal_part_opt_AST->getFirstChild();
			else
				currentAST.child = formal_part_opt_AST;
			currentAST.advanceChildToEnd();
		}
		formal_part_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_24);
		} else {
			throw;
		}
	}
	returnAST = formal_part_opt_AST;
}

void AdaParser::renames() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST renames_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
#line 257 "ada.g"
	RefAdaAST dummy;
#line 2025 "AdaParser.cpp"
	
	try {      // for error handling
		match(RENAMES);
		{
		switch ( LA(1)) {
		case IDENTIFIER:
		{
			name();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case CHAR_STRING:
		{
			dummy=definable_operator_symbol();
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
		if ( inputState->guessing==0 ) {
#line 261 "ada.g"
			pop_def_id();
#line 2056 "AdaParser.cpp"
		}
		renames_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_4);
		} else {
			throw;
		}
	}
	returnAST = renames_AST;
}

void AdaParser::is_separate_or_abstract_or_decl(
	RefAdaAST t
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST is_separate_or_abstract_or_decl_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case IS:
		{
			match(IS);
			separate_or_abstract(t);
			break;
		}
		case SEMI:
		{
			if ( inputState->guessing==0 ) {
#line 304 "ada.g"
				pop_def_id();
					    if (t->getType() == AdaTokenTypes::PROCEDURE)
					      Set(t, PROCEDURE_DECLARATION);
					    else
					      Set(t, FUNCTION_DECLARATION);
					
#line 2096 "AdaParser.cpp"
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
			recover(ex,_tokenSet_4);
		} else {
			throw;
		}
	}
	returnAST = is_separate_or_abstract_or_decl_AST;
}

void AdaParser::def_designator(
	bool lib_level
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST def_designator_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefAdaAST n_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
#line 329 "ada.g"
	RefAdaAST d;
#line 2126 "AdaParser.cpp"
	
	try {      // for error handling
		if (((LA(1) == IDENTIFIER) && (_tokenSet_25.member(LA(2))))&&( lib_level )) {
			compound_name();
			if (inputState->guessing==0) {
				n_AST = returnAST;
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			if ( inputState->guessing==0 ) {
#line 331 "ada.g"
				push_def_id(n_AST);
#line 2138 "AdaParser.cpp"
			}
			def_designator_AST = RefAdaAST(currentAST.root);
		}
		else if (((LA(1) == IDENTIFIER || LA(1) == CHAR_STRING) && (LA(2) == LPAREN || LA(2) == IS || LA(2) == RETURN))&&( !lib_level )) {
			d=designator();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			if ( inputState->guessing==0 ) {
#line 332 "ada.g"
				push_def_id(d);
#line 2150 "AdaParser.cpp"
			}
			def_designator_AST = RefAdaAST(currentAST.root);
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_26);
		} else {
			throw;
		}
	}
	returnAST = def_designator_AST;
}

void AdaParser::function_tail() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST function_tail_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		func_formal_part_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(RETURN);
		subtype_mark();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		function_tail_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_27);
		} else {
			throw;
		}
	}
	returnAST = function_tail_AST;
}

void AdaParser::generic_inst() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST generic_inst_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(NEW);
		compound_name();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case LPAREN:
		{
			match(LPAREN);
			value_s();
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
		if ( inputState->guessing==0 ) {
#line 179 "ada.g"
			pop_def_id();
#line 2234 "AdaParser.cpp"
		}
		generic_inst_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_4);
		} else {
			throw;
		}
	}
	returnAST = generic_inst_AST;
}

void AdaParser::value_s() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST value_s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		value();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				value();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop440;
			}
			
		}
		_loop440:;
		} // ( ... )*
		if ( inputState->guessing==0 ) {
			value_s_AST = RefAdaAST(currentAST.root);
#line 1380 "ada.g"
			value_s_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(VALUES,"VALUES")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(value_s_AST))));
#line 2279 "AdaParser.cpp"
			currentAST.root = value_s_AST;
			if ( value_s_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				value_s_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = value_s_AST->getFirstChild();
			else
				currentAST.child = value_s_AST;
			currentAST.advanceChildToEnd();
		}
		value_s_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_28);
		} else {
			throw;
		}
	}
	returnAST = value_s_AST;
}

void AdaParser::parenth_values() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST parenth_values_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(LPAREN);
		value();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				value();
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
		match(RPAREN);
		parenth_values_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_4);
		} else {
			throw;
		}
	}
	returnAST = parenth_values_AST;
}

void AdaParser::value() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST value_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case OTHERS:
		{
			RefAdaAST tmp61_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp61_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp61_AST));
			}
			match(OTHERS);
			match(RIGHT_SHAFT);
			expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case IDENTIFIER:
		case LPAREN:
		case NEW:
		case CHARACTER_LITERAL:
		case CHAR_STRING:
		case NuLL:
		case NOT:
		case PLUS:
		case MINUS:
		case ABS:
		case NUMERIC_LIT:
		{
			ranged_expr_s();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{
			switch ( LA(1)) {
			case RIGHT_SHAFT:
			{
				RefAdaAST tmp63_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp63_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp63_AST));
				}
				match(RIGHT_SHAFT);
				expression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case COMMA:
			case RPAREN:
			case WITH:
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
		value_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_29);
		} else {
			throw;
		}
	}
	returnAST = value_AST;
}

void AdaParser::ranged_expr_s() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST ranged_expr_s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		ranged_expr();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == PIPE)) {
				RefAdaAST tmp64_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp64_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp64_AST));
				}
				match(PIPE);
				ranged_expr();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop56;
			}
			
		}
		_loop56:;
		} // ( ... )*
		ranged_expr_s_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_30);
		} else {
			throw;
		}
	}
	returnAST = ranged_expr_s_AST;
}

void AdaParser::ranged_expr() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST ranged_expr_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case DOT_DOT:
		{
			RefAdaAST tmp65_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp65_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp65_AST));
			}
			match(DOT_DOT);
			simple_expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case RANGE:
		{
			RefAdaAST tmp66_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp66_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp66_AST));
			}
			match(RANGE);
			range();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case COMMA:
		case RPAREN:
		case RIGHT_SHAFT:
		case WITH:
		case PIPE:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		ranged_expr_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_31);
		} else {
			throw;
		}
	}
	returnAST = ranged_expr_AST;
}

void AdaParser::simple_expression() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST simple_expression_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		signed_term();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			switch ( LA(1)) {
			case PLUS:
			{
				RefAdaAST tmp67_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp67_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp67_AST));
				}
				match(PLUS);
				signed_term();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case MINUS:
			{
				RefAdaAST tmp68_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp68_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp68_AST));
				}
				match(MINUS);
				signed_term();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case CONCAT:
			{
				RefAdaAST tmp69_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp69_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp69_AST));
				}
				match(CONCAT);
				signed_term();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			default:
			{
				goto _loop453;
			}
			}
		}
		_loop453:;
		} // ( ... )*
		simple_expression_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_32);
		} else {
			throw;
		}
	}
	returnAST = simple_expression_AST;
}

void AdaParser::range() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST range_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		bool synPredMatched63 = false;
		if (((_tokenSet_5.member(LA(1))) && (_tokenSet_33.member(LA(2))))) {
			int _m63 = mark();
			synPredMatched63 = true;
			inputState->guessing++;
			try {
				{
				range_dots();
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched63 = false;
			}
			rewind(_m63);
			inputState->guessing--;
		}
		if ( synPredMatched63 ) {
			range_dots();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
		}
		else if ((LA(1) == IDENTIFIER) && (LA(2) == LPAREN || LA(2) == DOT || LA(2) == TIC)) {
			range_attrib_ref();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
		range_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_34);
		} else {
			throw;
		}
	}
	returnAST = range_AST;
}

void AdaParser::range_constraint() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST range_constraint_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(RANGE);
		range();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		range_constraint_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_35);
		} else {
			throw;
		}
	}
	returnAST = range_constraint_AST;
}

void AdaParser::range_dots() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST range_dots_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		simple_expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		RefAdaAST tmp71_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp71_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp71_AST));
		}
		match(DOT_DOT);
		simple_expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		range_dots_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_34);
		} else {
			throw;
		}
	}
	returnAST = range_dots_AST;
}

void AdaParser::range_attrib_ref() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST range_attrib_ref_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  r = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST r_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		prefix();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(TIC);
		r = LT(1);
		if ( inputState->guessing == 0 ) {
			r_AST = astFactory->create(r);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(r_AST));
		}
		match(RANGE);
		{
		switch ( LA(1)) {
		case LPAREN:
		{
			match(LPAREN);
			expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(RPAREN);
			break;
		}
		case SEMI:
		case COMMA:
		case RPAREN:
		case RIGHT_SHAFT:
		case WITH:
		case RANGE:
		case DIGITS:
		case IS:
		case PIPE:
		case DOT_DOT:
		case ASSIGN:
		case THEN:
		case LOOP:
		case OR:
		case AND:
		case XOR:
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
#line 218 "ada.g"
			Set(r_AST, RANGE_ATTRIBUTE_REFERENCE);
#line 2786 "AdaParser.cpp"
		}
		range_attrib_ref_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_34);
		} else {
			throw;
		}
	}
	returnAST = range_attrib_ref_AST;
}

void AdaParser::prefix() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST prefix_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  p = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST p_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefAdaAST tmp75_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp75_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp75_AST));
		}
		match(IDENTIFIER);
		{ // ( ... )*
		for (;;) {
			switch ( LA(1)) {
			case DOT:
			{
				RefAdaAST tmp76_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp76_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp76_AST));
				}
				match(DOT);
				{
				switch ( LA(1)) {
				case ALL:
				{
					RefAdaAST tmp77_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if ( inputState->guessing == 0 ) {
						tmp77_AST = astFactory->create(LT(1));
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp77_AST));
					}
					match(ALL);
					break;
				}
				case IDENTIFIER:
				{
					RefAdaAST tmp78_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if ( inputState->guessing == 0 ) {
						tmp78_AST = astFactory->create(LT(1));
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp78_AST));
					}
					match(IDENTIFIER);
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
				p = LT(1);
				if ( inputState->guessing == 0 ) {
					p_AST = astFactory->create(p);
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(p_AST));
				}
				match(LPAREN);
				value_s();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				match(RPAREN);
				if ( inputState->guessing==0 ) {
#line 227 "ada.g"
					Set(p_AST, INDEXED_COMPONENT);
#line 2872 "AdaParser.cpp"
				}
				break;
			}
			default:
			{
				goto _loop70;
			}
			}
		}
		_loop70:;
		} // ( ... )*
		prefix_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_36);
		} else {
			throw;
		}
	}
	returnAST = prefix_AST;
}

void AdaParser::parameter_specification() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST parameter_specification_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		def_ids_colon();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		mode_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		subtype_mark();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		init_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if ( inputState->guessing==0 ) {
			parameter_specification_AST = RefAdaAST(currentAST.root);
#line 239 "ada.g"
			parameter_specification_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(PARAMETER_SPECIFICATION,"PARAMETER_SPECIFICATION")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(parameter_specification_AST))));
#line 2924 "AdaParser.cpp"
			currentAST.root = parameter_specification_AST;
			if ( parameter_specification_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				parameter_specification_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = parameter_specification_AST->getFirstChild();
			else
				currentAST.child = parameter_specification_AST;
			currentAST.advanceChildToEnd();
		}
		parameter_specification_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_37);
		} else {
			throw;
		}
	}
	returnAST = parameter_specification_AST;
}

void AdaParser::def_ids_colon() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST def_ids_colon_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		defining_identifier_list();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(COLON);
		def_ids_colon_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_38);
		} else {
			throw;
		}
	}
	returnAST = def_ids_colon_AST;
}

void AdaParser::mode_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST mode_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case IN:
		{
			RefAdaAST tmp81_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp81_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp81_AST));
			}
			match(IN);
			{
			switch ( LA(1)) {
			case OUT:
			{
				RefAdaAST tmp82_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp82_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp82_AST));
				}
				match(OUT);
				break;
			}
			case IDENTIFIER:
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
		case OUT:
		{
			RefAdaAST tmp83_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp83_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp83_AST));
			}
			match(OUT);
			break;
		}
		case ACCESS:
		{
			RefAdaAST tmp84_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp84_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp84_AST));
			}
			match(ACCESS);
			break;
		}
		case IDENTIFIER:
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
			mode_opt_AST = RefAdaAST(currentAST.root);
#line 254 "ada.g"
			mode_opt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(MODIFIERS,"MODIFIERS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(mode_opt_AST))));
#line 3044 "AdaParser.cpp"
			currentAST.root = mode_opt_AST;
			if ( mode_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				mode_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = mode_opt_AST->getFirstChild();
			else
				currentAST.child = mode_opt_AST;
			currentAST.advanceChildToEnd();
		}
		mode_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_39);
		} else {
			throw;
		}
	}
	returnAST = mode_opt_AST;
}

void AdaParser::init_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST init_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case ASSIGN:
		{
			match(ASSIGN);
			expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case SEMI:
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
			init_opt_AST = RefAdaAST(currentAST.root);
#line 460 "ada.g"
			init_opt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(INIT_OPT,"INIT_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(init_opt_AST))));
#line 3098 "AdaParser.cpp"
			currentAST.root = init_opt_AST;
			if ( init_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				init_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = init_opt_AST->getFirstChild();
			else
				currentAST.child = init_opt_AST;
			currentAST.advanceChildToEnd();
		}
		init_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_37);
		} else {
			throw;
		}
	}
	returnAST = init_opt_AST;
}

void AdaParser::defining_identifier_list() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST defining_identifier_list_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefAdaAST tmp86_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp86_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp86_AST));
		}
		match(IDENTIFIER);
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				RefAdaAST tmp88_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp88_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp88_AST));
				}
				match(IDENTIFIER);
			}
			else {
				goto _loop79;
			}
			
		}
		_loop79:;
		} // ( ... )*
		if ( inputState->guessing==0 ) {
			defining_identifier_list_AST = RefAdaAST(currentAST.root);
#line 248 "ada.g"
			defining_identifier_list_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(DEFINING_IDENTIFIER_LIST,"DEFINING_IDENTIFIER_LIST")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(defining_identifier_list_AST))));
#line 3155 "AdaParser.cpp"
			currentAST.root = defining_identifier_list_AST;
			if ( defining_identifier_list_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				defining_identifier_list_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = defining_identifier_list_AST->getFirstChild();
			else
				currentAST.child = defining_identifier_list_AST;
			currentAST.advanceChildToEnd();
		}
		defining_identifier_list_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_40);
		} else {
			throw;
		}
	}
	returnAST = defining_identifier_list_AST;
}

void AdaParser::name() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST name_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  p = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST p_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
#line 264 "ada.g"
	RefAdaAST dummy;
#line 3185 "AdaParser.cpp"
	
	try {      // for error handling
		RefAdaAST tmp89_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp89_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp89_AST));
		}
		match(IDENTIFIER);
		{ // ( ... )*
		for (;;) {
			switch ( LA(1)) {
			case DOT:
			{
				RefAdaAST tmp90_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp90_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp90_AST));
				}
				match(DOT);
				{
				switch ( LA(1)) {
				case ALL:
				{
					RefAdaAST tmp91_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if ( inputState->guessing == 0 ) {
						tmp91_AST = astFactory->create(LT(1));
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp91_AST));
					}
					match(ALL);
					break;
				}
				case IDENTIFIER:
				{
					RefAdaAST tmp92_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if ( inputState->guessing == 0 ) {
						tmp92_AST = astFactory->create(LT(1));
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp92_AST));
					}
					match(IDENTIFIER);
					break;
				}
				case CHARACTER_LITERAL:
				{
					RefAdaAST tmp93_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if ( inputState->guessing == 0 ) {
						tmp93_AST = astFactory->create(LT(1));
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp93_AST));
					}
					match(CHARACTER_LITERAL);
					break;
				}
				case CHAR_STRING:
				{
					dummy=is_operator();
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
				break;
			}
			case LPAREN:
			{
				p = LT(1);
				if ( inputState->guessing == 0 ) {
					p_AST = astFactory->create(p);
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(p_AST));
				}
				match(LPAREN);
				value_s();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				match(RPAREN);
				if ( inputState->guessing==0 ) {
#line 272 "ada.g"
					Set(p_AST, INDEXED_COMPONENT);
#line 3269 "AdaParser.cpp"
				}
				break;
			}
			case TIC:
			{
				RefAdaAST tmp95_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp95_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp95_AST));
				}
				match(TIC);
				attribute_id();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			default:
			{
				goto _loop88;
			}
			}
		}
		_loop88:;
		} // ( ... )*
		name_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_41);
		} else {
			throw;
		}
	}
	returnAST = name_AST;
}

RefAdaAST  AdaParser::definable_operator_symbol() {
#line 284 "ada.g"
	RefAdaAST d;
#line 3311 "AdaParser.cpp"
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST definable_operator_symbol_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  op = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST op_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		if (!( definable_operator(LT(1)->getText().c_str()) ))
			throw ANTLR_USE_NAMESPACE(antlr)SemanticException(" definable_operator(LT(1)->getText().c_str()) ");
		op = LT(1);
		if ( inputState->guessing == 0 ) {
			op_AST = astFactory->create(op);
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(op_AST));
		}
		match(CHAR_STRING);
		if ( inputState->guessing==0 ) {
#line 286 "ada.g"
			op_AST->setType(OPERATOR_SYMBOL); d=op_AST;
#line 3330 "AdaParser.cpp"
		}
		definable_operator_symbol_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_42);
		} else {
			throw;
		}
	}
	returnAST = definable_operator_symbol_AST;
	return d;
}

RefAdaAST  AdaParser::is_operator() {
#line 279 "ada.g"
	RefAdaAST d;
#line 3349 "AdaParser.cpp"
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST is_operator_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  op = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST op_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		if (!( is_operator_symbol(LT(1)->getText().c_str()) ))
			throw ANTLR_USE_NAMESPACE(antlr)SemanticException(" is_operator_symbol(LT(1)->getText().c_str()) ");
		op = LT(1);
		if ( inputState->guessing == 0 ) {
			op_AST = astFactory->create(op);
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(op_AST));
		}
		match(CHAR_STRING);
		if ( inputState->guessing==0 ) {
#line 281 "ada.g"
			op_AST->setType(OPERATOR_SYMBOL); d=op_AST;
#line 3368 "AdaParser.cpp"
		}
		is_operator_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_43);
		} else {
			throw;
		}
	}
	returnAST = is_operator_AST;
	return d;
}

void AdaParser::parenthesized_primary() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST parenthesized_primary_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  pp = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST pp_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		pp = LT(1);
		if ( inputState->guessing == 0 ) {
			pp_AST = astFactory->create(pp);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(pp_AST));
		}
		match(LPAREN);
		{
		if ((LA(1) == NuLL) && (LA(2) == RECORD)) {
			RefAdaAST tmp96_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp96_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp96_AST));
			}
			match(NuLL);
			match(RECORD);
		}
		else if ((_tokenSet_44.member(LA(1))) && (_tokenSet_45.member(LA(2)))) {
			value_s();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			extension_opt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
		match(RPAREN);
		if ( inputState->guessing==0 ) {
#line 294 "ada.g"
			Set(pp_AST, PARENTHESIZED_PRIMARY);
#line 3427 "AdaParser.cpp"
		}
		parenthesized_primary_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_43);
		} else {
			throw;
		}
	}
	returnAST = parenthesized_primary_AST;
}

void AdaParser::extension_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST extension_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case WITH:
		{
			match(WITH);
			{
			if ((LA(1) == NuLL) && (LA(2) == RECORD)) {
				RefAdaAST tmp100_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp100_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp100_AST));
				}
				match(NuLL);
				match(RECORD);
			}
			else if ((_tokenSet_44.member(LA(1))) && (_tokenSet_46.member(LA(2)))) {
				value_s();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			
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
			extension_opt_AST = RefAdaAST(currentAST.root);
#line 298 "ada.g"
			extension_opt_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(EXTENSION_OPT,"EXTENSION_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(extension_opt_AST))));
#line 3491 "AdaParser.cpp"
			currentAST.root = extension_opt_AST;
			if ( extension_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				extension_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = extension_opt_AST->getFirstChild();
			else
				currentAST.child = extension_opt_AST;
			currentAST.advanceChildToEnd();
		}
		extension_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_47);
		} else {
			throw;
		}
	}
	returnAST = extension_opt_AST;
}

void AdaParser::separate_or_abstract(
	RefAdaAST t
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST separate_or_abstract_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case SEPARATE:
		{
			match(SEPARATE);
			if ( inputState->guessing==0 ) {
#line 314 "ada.g"
				pop_def_id();
						  if (t->getType() == AdaTokenTypes::PROCEDURE)
						    Set(t, PROCEDURE_BODY_STUB);
						  else
						    Set(t, FUNCTION_BODY_STUB);
						
#line 3533 "AdaParser.cpp"
			}
			break;
		}
		case ABSTRACT:
		{
			match(ABSTRACT);
			if ( inputState->guessing==0 ) {
#line 321 "ada.g"
				pop_def_id();
						  if (t->getType() == AdaTokenTypes::PROCEDURE)
						    Set(t, ABSTRACT_PROCEDURE_DECLARATION);
						  else
						    Set(t, ABSTRACT_FUNCTION_DECLARATION);
						
#line 3548 "AdaParser.cpp"
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
			recover(ex,_tokenSet_4);
		} else {
			throw;
		}
	}
	returnAST = separate_or_abstract_AST;
}

RefAdaAST  AdaParser::designator() {
#line 335 "ada.g"
	RefAdaAST d;
#line 3572 "AdaParser.cpp"
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST designator_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  n = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST n_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
#line 335 "ada.g"
	RefAdaAST op;
#line 3580 "AdaParser.cpp"
	
	try {      // for error handling
		switch ( LA(1)) {
		case CHAR_STRING:
		{
			op=definable_operator_symbol();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			if ( inputState->guessing==0 ) {
#line 337 "ada.g"
				d = op;
#line 3593 "AdaParser.cpp"
			}
			designator_AST = RefAdaAST(currentAST.root);
			break;
		}
		case IDENTIFIER:
		{
			n = LT(1);
			if ( inputState->guessing == 0 ) {
				n_AST = astFactory->create(n);
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(n_AST));
			}
			match(IDENTIFIER);
			if ( inputState->guessing==0 ) {
#line 338 "ada.g"
				d = n_AST;
#line 3609 "AdaParser.cpp"
			}
			designator_AST = RefAdaAST(currentAST.root);
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
			recover(ex,_tokenSet_26);
		} else {
			throw;
		}
	}
	returnAST = designator_AST;
	return d;
}

void AdaParser::func_formal_part_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST func_formal_part_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LPAREN:
		{
			match(LPAREN);
			func_param();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{ // ( ... )*
			for (;;) {
				if ((LA(1) == SEMI)) {
					match(SEMI);
					func_param();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
				}
				else {
					goto _loop104;
				}
				
			}
			_loop104:;
			} // ( ... )*
			match(RPAREN);
			break;
		}
		case RETURN:
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
			func_formal_part_opt_AST = RefAdaAST(currentAST.root);
#line 347 "ada.g"
			func_formal_part_opt_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(FORMAL_PART_OPT,"FORMAL_PART_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(func_formal_part_opt_AST))));
#line 3681 "AdaParser.cpp"
			currentAST.root = func_formal_part_opt_AST;
			if ( func_formal_part_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				func_formal_part_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = func_formal_part_opt_AST->getFirstChild();
			else
				currentAST.child = func_formal_part_opt_AST;
			currentAST.advanceChildToEnd();
		}
		func_formal_part_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_48);
		} else {
			throw;
		}
	}
	returnAST = func_formal_part_opt_AST;
}

void AdaParser::func_param() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST func_param_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		def_ids_colon();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		in_access_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		subtype_mark();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		init_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if ( inputState->guessing==0 ) {
			func_param_AST = RefAdaAST(currentAST.root);
#line 353 "ada.g"
			func_param_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(PARAMETER_SPECIFICATION,"PARAMETER_SPECIFICATION")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(func_param_AST))));
#line 3730 "AdaParser.cpp"
			currentAST.root = func_param_AST;
			if ( func_param_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				func_param_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = func_param_AST->getFirstChild();
			else
				currentAST.child = func_param_AST;
			currentAST.advanceChildToEnd();
		}
		func_param_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_37);
		} else {
			throw;
		}
	}
	returnAST = func_param_AST;
}

void AdaParser::in_access_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST in_access_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case IN:
		{
			match(IN);
			break;
		}
		case ACCESS:
		{
			RefAdaAST tmp108_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp108_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp108_AST));
			}
			match(ACCESS);
			break;
		}
		case IDENTIFIER:
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
			in_access_opt_AST = RefAdaAST(currentAST.root);
#line 359 "ada.g"
			in_access_opt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(MODIFIERS,"MODIFIERS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(in_access_opt_AST))));
#line 3789 "AdaParser.cpp"
			currentAST.root = in_access_opt_AST;
			if ( in_access_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				in_access_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = in_access_opt_AST->getFirstChild();
			else
				currentAST.child = in_access_opt_AST;
			currentAST.advanceChildToEnd();
		}
		in_access_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_39);
		} else {
			throw;
		}
	}
	returnAST = in_access_opt_AST;
}

void AdaParser::pkg_spec_part() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST pkg_spec_part_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		basic_declarative_items_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case PRIVATE:
		{
			match(PRIVATE);
			basic_declarative_items_opt();
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
		end_id_opt();
		pkg_spec_part_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_4);
		} else {
			throw;
		}
	}
	returnAST = pkg_spec_part_AST;
}

void AdaParser::basic_declarative_items_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST basic_declarative_items_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{ // ( ... )*
		for (;;) {
			switch ( LA(1)) {
			case IDENTIFIER:
			case USE:
			case TYPE:
			case PACKAGE:
			case PROCEDURE:
			case FUNCTION:
			case TASK:
			case PROTECTED:
			case FOR:
			case SUBTYPE:
			case GENERIC:
			{
				basic_decl_item();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case PRAGMA:
			{
				pragma();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			default:
			{
				goto _loop115;
			}
			}
		}
		_loop115:;
		} // ( ... )*
		if ( inputState->guessing==0 ) {
			basic_declarative_items_opt_AST = RefAdaAST(currentAST.root);
#line 377 "ada.g"
			basic_declarative_items_opt_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(BASIC_DECLARATIVE_ITEMS_OPT,"BASIC_DECLARATIVE_ITEMS_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(basic_declarative_items_opt_AST))));
#line 3904 "AdaParser.cpp"
			currentAST.root = basic_declarative_items_opt_AST;
			if ( basic_declarative_items_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				basic_declarative_items_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = basic_declarative_items_opt_AST->getFirstChild();
			else
				currentAST.child = basic_declarative_items_opt_AST;
			currentAST.advanceChildToEnd();
		}
		basic_declarative_items_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_49);
		} else {
			throw;
		}
	}
	returnAST = basic_declarative_items_opt_AST;
}

void AdaParser::basic_decl_item() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST basic_decl_item_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  pkg = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST pkg_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  tsk = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST tsk_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  pro = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST pro_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case PACKAGE:
		{
			pkg = LT(1);
			if ( inputState->guessing == 0 ) {
				pkg_AST = astFactory->create(pkg);
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(pkg_AST));
			}
			match(PACKAGE);
			def_id(false);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			spec_decl_part(pkg_AST);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			basic_decl_item_AST = RefAdaAST(currentAST.root);
			break;
		}
		case TASK:
		{
			tsk = LT(1);
			if ( inputState->guessing == 0 ) {
				tsk_AST = astFactory->create(tsk);
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tsk_AST));
			}
			match(TASK);
			task_type_or_single_decl(tsk_AST);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			basic_decl_item_AST = RefAdaAST(currentAST.root);
			break;
		}
		case PROTECTED:
		{
			pro = LT(1);
			if ( inputState->guessing == 0 ) {
				pro_AST = astFactory->create(pro);
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(pro_AST));
			}
			match(PROTECTED);
			prot_type_or_single_decl(pro_AST);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(SEMI);
			basic_decl_item_AST = RefAdaAST(currentAST.root);
			break;
		}
		case PROCEDURE:
		case FUNCTION:
		{
			subprog_decl(false);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			basic_decl_item_AST = RefAdaAST(currentAST.root);
			break;
		}
		case IDENTIFIER:
		case USE:
		case TYPE:
		case FOR:
		case SUBTYPE:
		case GENERIC:
		{
			decl_common();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			basic_decl_item_AST = RefAdaAST(currentAST.root);
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
	returnAST = basic_decl_item_AST;
}

void AdaParser::basic_declarative_items() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST basic_declarative_items_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{ // ( ... )+
		int _cnt118=0;
		for (;;) {
			switch ( LA(1)) {
			case IDENTIFIER:
			case USE:
			case TYPE:
			case PACKAGE:
			case PROCEDURE:
			case FUNCTION:
			case TASK:
			case PROTECTED:
			case FOR:
			case SUBTYPE:
			case GENERIC:
			{
				basic_decl_item();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case PRAGMA:
			{
				pragma();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			default:
			{
				if ( _cnt118>=1 ) { goto _loop118; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
			}
			}
			_cnt118++;
		}
		_loop118:;
		}  // ( ... )+
		if ( inputState->guessing==0 ) {
			basic_declarative_items_AST = RefAdaAST(currentAST.root);
#line 384 "ada.g"
			basic_declarative_items_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(BASIC_DECLARATIVE_ITEMS_OPT,"BASIC_DECLARATIVE_ITEMS_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(basic_declarative_items_AST))));
#line 4080 "AdaParser.cpp"
			currentAST.root = basic_declarative_items_AST;
			if ( basic_declarative_items_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				basic_declarative_items_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = basic_declarative_items_AST->getFirstChild();
			else
				currentAST.child = basic_declarative_items_AST;
			currentAST.advanceChildToEnd();
		}
		basic_declarative_items_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_0);
		} else {
			throw;
		}
	}
	returnAST = basic_declarative_items_AST;
}

void AdaParser::task_type_or_single_decl(
	RefAdaAST tsk
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST task_type_or_single_decl_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case TYPE:
		{
			match(TYPE);
			def_id(false);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			discrim_part_opt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			task_definition_opt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			if ( inputState->guessing==0 ) {
#line 400 "ada.g"
				Set(tsk, TASK_TYPE_DECLARATION);
#line 4129 "AdaParser.cpp"
			}
			task_type_or_single_decl_AST = RefAdaAST(currentAST.root);
			break;
		}
		case IDENTIFIER:
		{
			def_id(false);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			task_definition_opt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			if ( inputState->guessing==0 ) {
#line 402 "ada.g"
				Set(tsk, SINGLE_TASK_DECLARATION);
#line 4147 "AdaParser.cpp"
			}
			task_type_or_single_decl_AST = RefAdaAST(currentAST.root);
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
			recover(ex,_tokenSet_18);
		} else {
			throw;
		}
	}
	returnAST = task_type_or_single_decl_AST;
}

void AdaParser::prot_type_or_single_decl(
	RefAdaAST pro
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST prot_type_or_single_decl_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case TYPE:
		{
			match(TYPE);
			def_id(false);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			discrim_part_opt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			protected_definition();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			if ( inputState->guessing==0 ) {
#line 531 "ada.g"
				Set(pro, PROTECTED_TYPE_DECLARATION);
#line 4196 "AdaParser.cpp"
			}
			prot_type_or_single_decl_AST = RefAdaAST(currentAST.root);
			break;
		}
		case IDENTIFIER:
		{
			def_id(false);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			protected_definition();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			if ( inputState->guessing==0 ) {
#line 533 "ada.g"
				Set(pro, SINGLE_PROTECTED_DECLARATION);
#line 4214 "AdaParser.cpp"
			}
			prot_type_or_single_decl_AST = RefAdaAST(currentAST.root);
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
			recover(ex,_tokenSet_4);
		} else {
			throw;
		}
	}
	returnAST = prot_type_or_single_decl_AST;
}

void AdaParser::decl_common() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST decl_common_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  t = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST t_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  s = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  r = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST r_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  erd = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST erd_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  ord = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST ord_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  od = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST od_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case TYPE:
		{
			t = LT(1);
			if ( inputState->guessing == 0 ) {
				t_AST = astFactory->create(t);
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(t_AST));
			}
			match(TYPE);
			RefAdaAST tmp113_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp113_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp113_AST));
			}
			match(IDENTIFIER);
			{
			switch ( LA(1)) {
			case IS:
			{
				match(IS);
				type_def(t_AST);
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case SEMI:
			case LPAREN:
			{
				{
				switch ( LA(1)) {
				case LPAREN:
				{
					discrim_part();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					{
					switch ( LA(1)) {
					case IS:
					{
						match(IS);
						derived_or_private_or_record(t_AST, true);
						if (inputState->guessing==0) {
							astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
						}
						break;
					}
					case SEMI:
					{
						if ( inputState->guessing==0 ) {
#line 573 "ada.g"
							Set(t_AST, INCOMPLETE_TYPE_DECLARATION);
#line 4307 "AdaParser.cpp"
						}
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
				{
					empty_discrim_opt();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					if ( inputState->guessing==0 ) {
#line 576 "ada.g"
						Set(t_AST, INCOMPLETE_TYPE_DECLARATION);
#line 4328 "AdaParser.cpp"
					}
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
			match(SEMI);
			decl_common_AST = RefAdaAST(currentAST.root);
			break;
		}
		case SUBTYPE:
		{
			s = LT(1);
			if ( inputState->guessing == 0 ) {
				s_AST = astFactory->create(s);
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(s_AST));
			}
			match(SUBTYPE);
			RefAdaAST tmp117_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp117_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp117_AST));
			}
			match(IDENTIFIER);
			match(IS);
			subtype_ind();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(SEMI);
			if ( inputState->guessing==0 ) {
#line 590 "ada.g"
				Set(s_AST, SUBTYPE_DECLARATION);
#line 4373 "AdaParser.cpp"
			}
			decl_common_AST = RefAdaAST(currentAST.root);
			break;
		}
		case GENERIC:
		{
			generic_decl(false);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			decl_common_AST = RefAdaAST(currentAST.root);
			break;
		}
		case USE:
		{
			use_clause();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			decl_common_AST = RefAdaAST(currentAST.root);
			break;
		}
		case FOR:
		{
			r = LT(1);
			if ( inputState->guessing == 0 ) {
				r_AST = astFactory->create(r);
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(r_AST));
			}
			match(FOR);
			{
			bool synPredMatched182 = false;
			if (((LA(1) == IDENTIFIER) && (LA(2) == USE))) {
				int _m182 = mark();
				synPredMatched182 = true;
				inputState->guessing++;
				try {
					{
					local_enum_name();
					match(USE);
					match(LPAREN);
					}
				}
				catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
					synPredMatched182 = false;
				}
				rewind(_m182);
				inputState->guessing--;
			}
			if ( synPredMatched182 ) {
				local_enum_name();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				match(USE);
				enumeration_aggregate();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				if ( inputState->guessing==0 ) {
#line 595 "ada.g"
					Set(r_AST, ENUMERATION_REPESENTATION_CLAUSE);
#line 4436 "AdaParser.cpp"
				}
			}
			else if ((LA(1) == IDENTIFIER) && (LA(2) == DOT || LA(2) == USE || LA(2) == TIC)) {
				subtype_mark();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				match(USE);
				rep_spec_part(r_AST);
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			
			}
			match(SEMI);
			decl_common_AST = RefAdaAST(currentAST.root);
			break;
		}
		default:
			bool synPredMatched184 = false;
			if (((LA(1) == IDENTIFIER) && (LA(2) == COLON))) {
				int _m184 = mark();
				synPredMatched184 = true;
				inputState->guessing++;
				try {
					{
					match(IDENTIFIER);
					match(COLON);
					match(EXCEPTION);
					match(RENAMES);
					}
				}
				catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
					synPredMatched184 = false;
				}
				rewind(_m184);
				inputState->guessing--;
			}
			if ( synPredMatched184 ) {
				RefAdaAST tmp123_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp123_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp123_AST));
				}
				match(IDENTIFIER);
				erd = LT(1);
				if ( inputState->guessing == 0 ) {
					erd_AST = astFactory->create(erd);
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(erd_AST));
				}
				match(COLON);
				match(EXCEPTION);
				match(RENAMES);
				compound_name();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				match(SEMI);
				if ( inputState->guessing==0 ) {
#line 601 "ada.g"
					Set(erd_AST, EXCEPTION_RENAMING_DECLARATION);
#line 4502 "AdaParser.cpp"
				}
				decl_common_AST = RefAdaAST(currentAST.root);
			}
			else {
				bool synPredMatched186 = false;
				if (((LA(1) == IDENTIFIER) && (LA(2) == COLON))) {
					int _m186 = mark();
					synPredMatched186 = true;
					inputState->guessing++;
					try {
						{
						match(IDENTIFIER);
						match(COLON);
						subtype_mark();
						match(RENAMES);
						}
					}
					catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
						synPredMatched186 = false;
					}
					rewind(_m186);
					inputState->guessing--;
				}
				if ( synPredMatched186 ) {
					RefAdaAST tmp127_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if ( inputState->guessing == 0 ) {
						tmp127_AST = astFactory->create(LT(1));
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp127_AST));
					}
					match(IDENTIFIER);
					ord = LT(1);
					if ( inputState->guessing == 0 ) {
						ord_AST = astFactory->create(ord);
						astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(ord_AST));
					}
					match(COLON);
					subtype_mark();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					match(RENAMES);
					name();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					match(SEMI);
					if ( inputState->guessing==0 ) {
#line 604 "ada.g"
						Set(ord_AST, OBJECT_RENAMING_DECLARATION);
#line 4552 "AdaParser.cpp"
					}
					decl_common_AST = RefAdaAST(currentAST.root);
				}
				else if ((LA(1) == IDENTIFIER) && (LA(2) == COMMA || LA(2) == COLON)) {
					defining_identifier_list();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					od = LT(1);
					if ( inputState->guessing == 0 ) {
						od_AST = astFactory->create(od);
						astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(od_AST));
					}
					match(COLON);
					{
					if ((LA(1) == EXCEPTION)) {
						match(EXCEPTION);
						if ( inputState->guessing==0 ) {
#line 607 "ada.g"
							Set(od_AST, EXCEPTION_DECLARATION);
#line 4573 "AdaParser.cpp"
						}
					}
					else {
						bool synPredMatched189 = false;
						if (((LA(1) == CONSTANT) && (LA(2) == ASSIGN))) {
							int _m189 = mark();
							synPredMatched189 = true;
							inputState->guessing++;
							try {
								{
								match(CONSTANT);
								match(ASSIGN);
								}
							}
							catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
								synPredMatched189 = false;
							}
							rewind(_m189);
							inputState->guessing--;
						}
						if ( synPredMatched189 ) {
							match(CONSTANT);
							match(ASSIGN);
							expression();
							if (inputState->guessing==0) {
								astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
							}
							if ( inputState->guessing==0 ) {
#line 609 "ada.g"
								Set(od_AST, NUMBER_DECLARATION);
#line 4604 "AdaParser.cpp"
							}
						}
						else if ((_tokenSet_50.member(LA(1))) && (_tokenSet_51.member(LA(2)))) {
							aliased_constant_opt();
							if (inputState->guessing==0) {
								astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
							}
							{
							switch ( LA(1)) {
							case ARRAY:
							{
								array_type_definition(od_AST);
								if (inputState->guessing==0) {
									astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
								}
								init_opt();
								if (inputState->guessing==0) {
									astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
								}
								if ( inputState->guessing==0 ) {
#line 612 "ada.g"
									Set(od_AST, ARRAY_OBJECT_DECLARATION);
#line 4627 "AdaParser.cpp"
								}
								break;
							}
							case IDENTIFIER:
							{
								subtype_ind();
								if (inputState->guessing==0) {
									astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
								}
								init_opt();
								if (inputState->guessing==0) {
									astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
								}
								if ( inputState->guessing==0 ) {
#line 616 "ada.g"
									Set(od_AST, OBJECT_DECLARATION);
#line 4644 "AdaParser.cpp"
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
						throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
					}
					}
					}
					match(SEMI);
					decl_common_AST = RefAdaAST(currentAST.root);
				}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_18);
		} else {
			throw;
		}
	}
	returnAST = decl_common_AST;
}

void AdaParser::discrim_part_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST discrim_part_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LPAREN:
		{
			discrim_part_text();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case SEMI:
		case IS:
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
			discrim_part_opt_AST = RefAdaAST(currentAST.root);
#line 412 "ada.g"
			discrim_part_opt_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(DISCRIM_PART_OPT,"DISCRIM_PART_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(discrim_part_opt_AST))));
#line 4711 "AdaParser.cpp"
			currentAST.root = discrim_part_opt_AST;
			if ( discrim_part_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				discrim_part_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = discrim_part_opt_AST->getFirstChild();
			else
				currentAST.child = discrim_part_opt_AST;
			currentAST.advanceChildToEnd();
		}
		discrim_part_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_52);
		} else {
			throw;
		}
	}
	returnAST = discrim_part_opt_AST;
}

void AdaParser::task_definition_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST task_definition_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case IS:
		{
			match(IS);
			task_items_opt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			private_task_items_opt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			end_id_opt();
			match(SEMI);
			task_definition_opt_AST = RefAdaAST(currentAST.root);
			break;
		}
		case SEMI:
		{
			match(SEMI);
			if ( inputState->guessing==0 ) {
#line 407 "ada.g"
				pop_def_id();
#line 4762 "AdaParser.cpp"
			}
			task_definition_opt_AST = RefAdaAST(currentAST.root);
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
			recover(ex,_tokenSet_18);
		} else {
			throw;
		}
	}
	returnAST = task_definition_opt_AST;
}

void AdaParser::task_items_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST task_items_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == PRAGMA)) {
				pragma();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop139;
			}
			
		}
		_loop139:;
		} // ( ... )*
		entrydecls_repspecs_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if ( inputState->guessing==0 ) {
			task_items_opt_AST = RefAdaAST(currentAST.root);
#line 465 "ada.g"
			task_items_opt_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(TASK_ITEMS_OPT,"TASK_ITEMS_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(task_items_opt_AST))));
#line 4814 "AdaParser.cpp"
			currentAST.root = task_items_opt_AST;
			if ( task_items_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				task_items_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = task_items_opt_AST->getFirstChild();
			else
				currentAST.child = task_items_opt_AST;
			currentAST.advanceChildToEnd();
		}
		task_items_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_49);
		} else {
			throw;
		}
	}
	returnAST = task_items_opt_AST;
}

void AdaParser::private_task_items_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST private_task_items_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case PRIVATE:
		{
			match(PRIVATE);
			{ // ( ... )*
			for (;;) {
				if ((LA(1) == PRAGMA)) {
					pragma();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
				}
				else {
					goto _loop164;
				}
				
			}
			_loop164:;
			} // ( ... )*
			entrydecls_repspecs_opt();
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
		if ( inputState->guessing==0 ) {
			private_task_items_opt_AST = RefAdaAST(currentAST.root);
#line 522 "ada.g"
			private_task_items_opt_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(PRIVATE_TASK_ITEMS_OPT,"PRIVATE_TASK_ITEMS_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(private_task_items_opt_AST))));
#line 4883 "AdaParser.cpp"
			currentAST.root = private_task_items_opt_AST;
			if ( private_task_items_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				private_task_items_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = private_task_items_opt_AST->getFirstChild();
			else
				currentAST.child = private_task_items_opt_AST;
			currentAST.advanceChildToEnd();
		}
		private_task_items_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_21);
		} else {
			throw;
		}
	}
	returnAST = private_task_items_opt_AST;
}

void AdaParser::discrim_part_text() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST discrim_part_text_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(LPAREN);
		{
		switch ( LA(1)) {
		case BOX:
		{
			RefAdaAST tmp139_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp139_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp139_AST));
			}
			match(BOX);
			break;
		}
		case IDENTIFIER:
		{
			discriminant_specifications();
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
		match(RPAREN);
		discrim_part_text_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_52);
		} else {
			throw;
		}
	}
	returnAST = discrim_part_text_AST;
}

void AdaParser::discriminant_specifications() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST discriminant_specifications_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		discriminant_specification();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == SEMI)) {
				match(SEMI);
				discriminant_specification();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop131;
			}
			
		}
		_loop131:;
		} // ( ... )*
		if ( inputState->guessing==0 ) {
			discriminant_specifications_AST = RefAdaAST(currentAST.root);
#line 442 "ada.g"
			discriminant_specifications_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(DISCRIMINANT_SPECIFICATIONS,"DISCRIMINANT_SPECIFICATIONS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(discriminant_specifications_AST))));
#line 4983 "AdaParser.cpp"
			currentAST.root = discriminant_specifications_AST;
			if ( discriminant_specifications_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				discriminant_specifications_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = discriminant_specifications_AST->getFirstChild();
			else
				currentAST.child = discriminant_specifications_AST;
			currentAST.advanceChildToEnd();
		}
		discriminant_specifications_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_47);
		} else {
			throw;
		}
	}
	returnAST = discriminant_specifications_AST;
}

void AdaParser::known_discrim_part() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST known_discrim_part_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(LPAREN);
		discriminant_specifications();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(RPAREN);
		if ( inputState->guessing==0 ) {
			known_discrim_part_AST = RefAdaAST(currentAST.root);
#line 422 "ada.g"
			known_discrim_part_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(DISCRIM_PART_OPT,"DISCRIM_PART_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(known_discrim_part_AST))));
#line 5022 "AdaParser.cpp"
			currentAST.root = known_discrim_part_AST;
			if ( known_discrim_part_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				known_discrim_part_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = known_discrim_part_AST->getFirstChild();
			else
				currentAST.child = known_discrim_part_AST;
			currentAST.advanceChildToEnd();
		}
		known_discrim_part_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_0);
		} else {
			throw;
		}
	}
	returnAST = known_discrim_part_AST;
}

void AdaParser::empty_discrim_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST empty_discrim_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		if ( inputState->guessing==0 ) {
			empty_discrim_opt_AST = RefAdaAST(currentAST.root);
#line 428 "ada.g"
			empty_discrim_opt_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(DISCRIM_PART_OPT,"DISCRIM_PART_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(empty_discrim_opt_AST))));
#line 5055 "AdaParser.cpp"
			currentAST.root = empty_discrim_opt_AST;
			if ( empty_discrim_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				empty_discrim_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = empty_discrim_opt_AST->getFirstChild();
			else
				currentAST.child = empty_discrim_opt_AST;
			currentAST.advanceChildToEnd();
		}
		empty_discrim_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_53);
		} else {
			throw;
		}
	}
	returnAST = empty_discrim_opt_AST;
}

void AdaParser::discrim_part() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST discrim_part_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		discrim_part_text();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if ( inputState->guessing==0 ) {
			discrim_part_AST = RefAdaAST(currentAST.root);
#line 435 "ada.g"
			discrim_part_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(DISCRIM_PART_OPT,"DISCRIM_PART_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(discrim_part_AST))));
#line 5092 "AdaParser.cpp"
			currentAST.root = discrim_part_AST;
			if ( discrim_part_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				discrim_part_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = discrim_part_AST->getFirstChild();
			else
				currentAST.child = discrim_part_AST;
			currentAST.advanceChildToEnd();
		}
		discrim_part_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_52);
		} else {
			throw;
		}
	}
	returnAST = discrim_part_AST;
}

void AdaParser::discriminant_specification() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST discriminant_specification_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		def_ids_colon();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		access_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		subtype_mark();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		init_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if ( inputState->guessing==0 ) {
			discriminant_specification_AST = RefAdaAST(currentAST.root);
#line 449 "ada.g"
			discriminant_specification_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(DISCRIMINANT_SPECIFICATION,"DISCRIMINANT_SPECIFICATION")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(discriminant_specification_AST))));
#line 5141 "AdaParser.cpp"
			currentAST.root = discriminant_specification_AST;
			if ( discriminant_specification_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				discriminant_specification_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = discriminant_specification_AST->getFirstChild();
			else
				currentAST.child = discriminant_specification_AST;
			currentAST.advanceChildToEnd();
		}
		discriminant_specification_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_37);
		} else {
			throw;
		}
	}
	returnAST = discriminant_specification_AST;
}

void AdaParser::access_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST access_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case ACCESS:
		{
			RefAdaAST tmp144_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp144_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp144_AST));
			}
			match(ACCESS);
			break;
		}
		case IDENTIFIER:
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
			access_opt_AST = RefAdaAST(currentAST.root);
#line 456 "ada.g"
			access_opt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(MODIFIERS,"MODIFIERS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(access_opt_AST))));
#line 5195 "AdaParser.cpp"
			currentAST.root = access_opt_AST;
			if ( access_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				access_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = access_opt_AST->getFirstChild();
			else
				currentAST.child = access_opt_AST;
			currentAST.advanceChildToEnd();
		}
		access_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_39);
		} else {
			throw;
		}
	}
	returnAST = access_opt_AST;
}

void AdaParser::entrydecls_repspecs_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST entrydecls_repspecs_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == ENTRY)) {
				entry_declaration();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				{ // ( ... )*
				for (;;) {
					switch ( LA(1)) {
					case PRAGMA:
					{
						pragma();
						if (inputState->guessing==0) {
							astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
						}
						break;
					}
					case FOR:
					{
						rep_spec();
						if (inputState->guessing==0) {
							astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
						}
						break;
					}
					default:
					{
						goto _loop143;
					}
					}
				}
				_loop143:;
				} // ( ... )*
			}
			else {
				goto _loop144;
			}
			
		}
		_loop144:;
		} // ( ... )*
		entrydecls_repspecs_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_49);
		} else {
			throw;
		}
	}
	returnAST = entrydecls_repspecs_opt_AST;
}

void AdaParser::entry_declaration() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST entry_declaration_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  e = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST e_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		e = LT(1);
		if ( inputState->guessing == 0 ) {
			e_AST = astFactory->create(e);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(e_AST));
		}
		match(ENTRY);
		RefAdaAST tmp145_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp145_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp145_AST));
		}
		match(IDENTIFIER);
		discrete_subtype_def_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		formal_part_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(SEMI);
		if ( inputState->guessing==0 ) {
#line 474 "ada.g"
			Set (e_AST, ENTRY_DECLARATION);
#line 5310 "AdaParser.cpp"
		}
		entry_declaration_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_54);
		} else {
			throw;
		}
	}
	returnAST = entry_declaration_AST;
}

void AdaParser::rep_spec() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST rep_spec_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  r = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST r_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		r = LT(1);
		if ( inputState->guessing == 0 ) {
			r_AST = astFactory->create(r);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(r_AST));
		}
		match(FOR);
		subtype_mark();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(USE);
		rep_spec_part(r_AST);
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(SEMI);
		rep_spec_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_54);
		} else {
			throw;
		}
	}
	returnAST = rep_spec_AST;
}

void AdaParser::discrete_subtype_def_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST discrete_subtype_def_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		bool synPredMatched149 = false;
		if (((LA(1) == LPAREN) && (_tokenSet_5.member(LA(2))))) {
			int _m149 = mark();
			synPredMatched149 = true;
			inputState->guessing++;
			try {
				{
				match(LPAREN);
				discrete_subtype_definition();
				match(RPAREN);
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched149 = false;
			}
			rewind(_m149);
			inputState->guessing--;
		}
		if ( synPredMatched149 ) {
			match(LPAREN);
			discrete_subtype_definition();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(RPAREN);
		}
		else if ((LA(1) == SEMI || LA(1) == LPAREN) && (_tokenSet_54.member(LA(2)))) {
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
		if ( inputState->guessing==0 ) {
			discrete_subtype_def_opt_AST = RefAdaAST(currentAST.root);
#line 481 "ada.g"
			discrete_subtype_def_opt_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(DISCRETE_SUBTYPE_DEF_OPT,"DISCRETE_SUBTYPE_DEF_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(discrete_subtype_def_opt_AST))));
#line 5407 "AdaParser.cpp"
			currentAST.root = discrete_subtype_def_opt_AST;
			if ( discrete_subtype_def_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				discrete_subtype_def_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = discrete_subtype_def_opt_AST->getFirstChild();
			else
				currentAST.child = discrete_subtype_def_opt_AST;
			currentAST.advanceChildToEnd();
		}
		discrete_subtype_def_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_55);
		} else {
			throw;
		}
	}
	returnAST = discrete_subtype_def_opt_AST;
}

void AdaParser::discrete_subtype_definition() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST discrete_subtype_definition_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		bool synPredMatched153 = false;
		if (((_tokenSet_5.member(LA(1))) && (_tokenSet_33.member(LA(2))))) {
			int _m153 = mark();
			synPredMatched153 = true;
			inputState->guessing++;
			try {
				{
				range();
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched153 = false;
			}
			rewind(_m153);
			inputState->guessing--;
		}
		if ( synPredMatched153 ) {
			range();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
		}
		else if ((LA(1) == IDENTIFIER) && (_tokenSet_56.member(LA(2)))) {
			subtype_ind();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
		discrete_subtype_definition_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_57);
		} else {
			throw;
		}
	}
	returnAST = discrete_subtype_definition_AST;
}

void AdaParser::subtype_ind() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST subtype_ind_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		subtype_mark();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		constraint_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if ( inputState->guessing==0 ) {
			subtype_ind_AST = RefAdaAST(currentAST.root);
#line 679 "ada.g"
			subtype_ind_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(SUBTYPE_INDICATION,"SUBTYPE_INDICATION")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(subtype_ind_AST))));
#line 5500 "AdaParser.cpp"
			currentAST.root = subtype_ind_AST;
			if ( subtype_ind_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				subtype_ind_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = subtype_ind_AST->getFirstChild();
			else
				currentAST.child = subtype_ind_AST;
			currentAST.advanceChildToEnd();
		}
		subtype_ind_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_58);
		} else {
			throw;
		}
	}
	returnAST = subtype_ind_AST;
}

void AdaParser::rep_spec_part(
	RefAdaAST t
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST rep_spec_part_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case RECORD:
		{
			match(RECORD);
			align_opt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			comp_loc_s();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(END);
			match(RECORD);
			if ( inputState->guessing==0 ) {
#line 504 "ada.g"
				Set(t, RECORD_REPRESENTATION_CLAUSE);
#line 5547 "AdaParser.cpp"
			}
			rep_spec_part_AST = RefAdaAST(currentAST.root);
			break;
		}
		case AT:
		{
			match(AT);
			expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			if ( inputState->guessing==0 ) {
#line 506 "ada.g"
				Set(t, AT_CLAUSE);
#line 5562 "AdaParser.cpp"
			}
			rep_spec_part_AST = RefAdaAST(currentAST.root);
			break;
		}
		case IDENTIFIER:
		case LPAREN:
		case NEW:
		case CHARACTER_LITERAL:
		case CHAR_STRING:
		case NuLL:
		case NOT:
		case PLUS:
		case MINUS:
		case ABS:
		case NUMERIC_LIT:
		{
			expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			if ( inputState->guessing==0 ) {
#line 509 "ada.g"
				Set(t, ATTRIBUTE_DEFINITION_CLAUSE);
#line 5586 "AdaParser.cpp"
			}
			rep_spec_part_AST = RefAdaAST(currentAST.root);
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
			recover(ex,_tokenSet_4);
		} else {
			throw;
		}
	}
	returnAST = rep_spec_part_AST;
}

void AdaParser::align_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST align_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case AT:
		{
			match(AT);
			match(MOD);
			expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(SEMI);
			break;
		}
		case PRAGMA:
		case IDENTIFIER:
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
		if ( inputState->guessing==0 ) {
			align_opt_AST = RefAdaAST(currentAST.root);
#line 513 "ada.g"
			align_opt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(MOD_CLAUSE_OPT,"MOD_CLAUSE_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(align_opt_AST))));
#line 5643 "AdaParser.cpp"
			currentAST.root = align_opt_AST;
			if ( align_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				align_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = align_opt_AST->getFirstChild();
			else
				currentAST.child = align_opt_AST;
			currentAST.advanceChildToEnd();
		}
		align_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_59);
		} else {
			throw;
		}
	}
	returnAST = align_opt_AST;
}

void AdaParser::comp_loc_s() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST comp_loc_s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{ // ( ... )*
		for (;;) {
			switch ( LA(1)) {
			case PRAGMA:
			{
				pragma();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case IDENTIFIER:
			{
				subtype_mark();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				match(AT);
				expression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				match(RANGE);
				range();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				match(SEMI);
				break;
			}
			default:
			{
				goto _loop160;
			}
			}
		}
		_loop160:;
		} // ( ... )*
		if ( inputState->guessing==0 ) {
			comp_loc_s_AST = RefAdaAST(currentAST.root);
#line 517 "ada.g"
			comp_loc_s_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(COMPONENT_CLAUSES_OPT,"COMPONENT_CLAUSES_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(comp_loc_s_AST))));
#line 5713 "AdaParser.cpp"
			currentAST.root = comp_loc_s_AST;
			if ( comp_loc_s_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				comp_loc_s_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = comp_loc_s_AST->getFirstChild();
			else
				currentAST.child = comp_loc_s_AST;
			currentAST.advanceChildToEnd();
		}
		comp_loc_s_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_21);
		} else {
			throw;
		}
	}
	returnAST = comp_loc_s_AST;
}

void AdaParser::protected_definition() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST protected_definition_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(IS);
		prot_op_decl_s();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case PRIVATE:
		{
			match(PRIVATE);
			prot_member_decl_s();
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
		end_id_opt();
		protected_definition_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_4);
		} else {
			throw;
		}
	}
	returnAST = protected_definition_AST;
}

void AdaParser::prot_op_decl_s() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST prot_op_decl_s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{ // ( ... )*
		for (;;) {
			if ((_tokenSet_60.member(LA(1)))) {
				prot_op_decl();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop170;
			}
			
		}
		_loop170:;
		} // ( ... )*
		if ( inputState->guessing==0 ) {
			prot_op_decl_s_AST = RefAdaAST(currentAST.root);
#line 541 "ada.g"
			prot_op_decl_s_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(PROT_OP_DECLARATIONS,"PROT_OP_DECLARATIONS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(prot_op_decl_s_AST))));
#line 5806 "AdaParser.cpp"
			currentAST.root = prot_op_decl_s_AST;
			if ( prot_op_decl_s_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				prot_op_decl_s_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = prot_op_decl_s_AST->getFirstChild();
			else
				currentAST.child = prot_op_decl_s_AST;
			currentAST.advanceChildToEnd();
		}
		prot_op_decl_s_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_49);
		} else {
			throw;
		}
	}
	returnAST = prot_op_decl_s_AST;
}

void AdaParser::prot_member_decl_s() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST prot_member_decl_s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{ // ( ... )*
		for (;;) {
			switch ( LA(1)) {
			case PRAGMA:
			case PROCEDURE:
			case FUNCTION:
			case ENTRY:
			case FOR:
			{
				prot_op_decl();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case IDENTIFIER:
			{
				comp_decl();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			default:
			{
				goto _loop174;
			}
			}
		}
		_loop174:;
		} // ( ... )*
		if ( inputState->guessing==0 ) {
			prot_member_decl_s_AST = RefAdaAST(currentAST.root);
#line 555 "ada.g"
			prot_member_decl_s_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(PROT_MEMBER_DECLARATIONS,"PROT_MEMBER_DECLARATIONS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(prot_member_decl_s_AST))));
#line 5870 "AdaParser.cpp"
			currentAST.root = prot_member_decl_s_AST;
			if ( prot_member_decl_s_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				prot_member_decl_s_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = prot_member_decl_s_AST->getFirstChild();
			else
				currentAST.child = prot_member_decl_s_AST;
			currentAST.advanceChildToEnd();
		}
		prot_member_decl_s_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_21);
		} else {
			throw;
		}
	}
	returnAST = prot_member_decl_s_AST;
}

void AdaParser::prot_op_decl() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST prot_op_decl_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  p = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST p_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  f = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST f_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case ENTRY:
		{
			entry_declaration();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			prot_op_decl_AST = RefAdaAST(currentAST.root);
			break;
		}
		case PROCEDURE:
		{
			p = LT(1);
			if ( inputState->guessing == 0 ) {
				p_AST = astFactory->create(p);
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(p_AST));
			}
			match(PROCEDURE);
			def_id(false);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			formal_part_opt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(SEMI);
			if ( inputState->guessing==0 ) {
#line 547 "ada.g"
				pop_def_id(); Set(p_AST, PROCEDURE_DECLARATION);
#line 5932 "AdaParser.cpp"
			}
			prot_op_decl_AST = RefAdaAST(currentAST.root);
			break;
		}
		case FUNCTION:
		{
			f = LT(1);
			if ( inputState->guessing == 0 ) {
				f_AST = astFactory->create(f);
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(f_AST));
			}
			match(FUNCTION);
			def_designator(false);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			function_tail();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(SEMI);
			if ( inputState->guessing==0 ) {
#line 549 "ada.g"
				pop_def_id(); Set(f_AST, FUNCTION_DECLARATION);
#line 5957 "AdaParser.cpp"
			}
			prot_op_decl_AST = RefAdaAST(currentAST.root);
			break;
		}
		case FOR:
		{
			rep_spec();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			prot_op_decl_AST = RefAdaAST(currentAST.root);
			break;
		}
		case PRAGMA:
		{
			pragma();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			prot_op_decl_AST = RefAdaAST(currentAST.root);
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
			recover(ex,_tokenSet_54);
		} else {
			throw;
		}
	}
	returnAST = prot_op_decl_AST;
}

void AdaParser::comp_decl() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST comp_decl_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		def_ids_colon();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		component_subtype_def();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		init_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(SEMI);
		if ( inputState->guessing==0 ) {
			comp_decl_AST = RefAdaAST(currentAST.root);
#line 561 "ada.g"
			comp_decl_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(COMPONENT_DECLARATION,"COMPONENT_DECLARATION")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(comp_decl_AST))));
#line 6021 "AdaParser.cpp"
			currentAST.root = comp_decl_AST;
			if ( comp_decl_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				comp_decl_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = comp_decl_AST->getFirstChild();
			else
				currentAST.child = comp_decl_AST;
			currentAST.advanceChildToEnd();
		}
		comp_decl_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_61);
		} else {
			throw;
		}
	}
	returnAST = comp_decl_AST;
}

void AdaParser::component_subtype_def() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST component_subtype_def_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		aliased_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		subtype_ind();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		component_subtype_def_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_62);
		} else {
			throw;
		}
	}
	returnAST = component_subtype_def_AST;
}

void AdaParser::type_def(
	RefAdaAST t
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST type_def_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case LPAREN:
		{
			match(LPAREN);
			enum_id_s();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(RPAREN);
			if ( inputState->guessing==0 ) {
#line 624 "ada.g"
				Set(t, ENUMERATION_TYPE_DECLARATION);
#line 6090 "AdaParser.cpp"
			}
			type_def_AST = RefAdaAST(currentAST.root);
			break;
		}
		case RANGE:
		{
			match(RANGE);
			range();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			if ( inputState->guessing==0 ) {
#line 626 "ada.g"
				Set(t, SIGNED_INTEGER_TYPE_DECLARATION);
#line 6105 "AdaParser.cpp"
			}
			type_def_AST = RefAdaAST(currentAST.root);
			break;
		}
		case MOD:
		{
			match(MOD);
			expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			if ( inputState->guessing==0 ) {
#line 628 "ada.g"
				Set(t, MODULAR_TYPE_DECLARATION);
#line 6120 "AdaParser.cpp"
			}
			type_def_AST = RefAdaAST(currentAST.root);
			break;
		}
		case DIGITS:
		{
			match(DIGITS);
			expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			range_constraint_opt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			if ( inputState->guessing==0 ) {
#line 630 "ada.g"
				Set(t, FLOATING_POINT_DECLARATION);
#line 6139 "AdaParser.cpp"
			}
			type_def_AST = RefAdaAST(currentAST.root);
			break;
		}
		case DELTA:
		{
			match(DELTA);
			expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{
			switch ( LA(1)) {
			case RANGE:
			{
				match(RANGE);
				range();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				if ( inputState->guessing==0 ) {
#line 633 "ada.g"
					Set(t, ORDINARY_FIXED_POINT_DECLARATION);
#line 6163 "AdaParser.cpp"
				}
				break;
			}
			case DIGITS:
			{
				match(DIGITS);
				expression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				range_constraint_opt();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				if ( inputState->guessing==0 ) {
#line 635 "ada.g"
					Set(t, DECIMAL_FIXED_POINT_DECLARATION);
#line 6181 "AdaParser.cpp"
				}
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			type_def_AST = RefAdaAST(currentAST.root);
			break;
		}
		case ARRAY:
		{
			array_type_definition(t);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			type_def_AST = RefAdaAST(currentAST.root);
			break;
		}
		case ACCESS:
		{
			access_type_definition(t);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			type_def_AST = RefAdaAST(currentAST.root);
			break;
		}
		case PRIVATE:
		case NEW:
		case NuLL:
		case RECORD:
		case ABSTRACT:
		case TAGGED:
		case LIMITED:
		{
			empty_discrim_opt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			derived_or_private_or_record(t, false);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			type_def_AST = RefAdaAST(currentAST.root);
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
			recover(ex,_tokenSet_4);
		} else {
			throw;
		}
	}
	returnAST = type_def_AST;
}

void AdaParser::derived_or_private_or_record(
	RefAdaAST t, bool has_discrim
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST derived_or_private_or_record_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		bool synPredMatched244 = false;
		if (((LA(1) == NEW || LA(1) == ABSTRACT) && (LA(2) == IDENTIFIER || LA(2) == NEW))) {
			int _m244 = mark();
			synPredMatched244 = true;
			inputState->guessing++;
			try {
				{
				{
				switch ( LA(1)) {
				case ABSTRACT:
				{
					match(ABSTRACT);
					break;
				}
				case NEW:
				{
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				match(NEW);
				subtype_ind();
				match(WITH);
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched244 = false;
			}
			rewind(_m244);
			inputState->guessing--;
		}
		if ( synPredMatched244 ) {
			abstract_opt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(NEW);
			subtype_ind();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(WITH);
			{
			switch ( LA(1)) {
			case PRIVATE:
			{
				match(PRIVATE);
				if ( inputState->guessing==0 ) {
#line 758 "ada.g"
					Set(t, PRIVATE_EXTENSION_DECLARATION);
#line 6310 "AdaParser.cpp"
				}
				break;
			}
			case NuLL:
			case RECORD:
			{
				record_definition(has_discrim);
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				if ( inputState->guessing==0 ) {
#line 760 "ada.g"
					Set(t, DERIVED_RECORD_EXTENSION);
#line 6324 "AdaParser.cpp"
				}
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			derived_or_private_or_record_AST = RefAdaAST(currentAST.root);
		}
		else if ((LA(1) == NEW) && (LA(2) == IDENTIFIER)) {
			match(NEW);
			subtype_ind();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			if ( inputState->guessing==0 ) {
#line 762 "ada.g"
				Set(t, ORDINARY_DERIVED_TYPE_DECLARATION);
#line 6345 "AdaParser.cpp"
			}
			derived_or_private_or_record_AST = RefAdaAST(currentAST.root);
		}
		else if ((_tokenSet_63.member(LA(1))) && (_tokenSet_64.member(LA(2)))) {
			abstract_tagged_limited_opt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{
			switch ( LA(1)) {
			case PRIVATE:
			{
				match(PRIVATE);
				if ( inputState->guessing==0 ) {
#line 764 "ada.g"
					Set(t, PRIVATE_TYPE_DECLARATION);
#line 6362 "AdaParser.cpp"
				}
				break;
			}
			case NuLL:
			case RECORD:
			{
				record_definition(has_discrim);
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				if ( inputState->guessing==0 ) {
#line 766 "ada.g"
					Set(t, RECORD_TYPE_DECLARATION);
#line 6376 "AdaParser.cpp"
				}
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			derived_or_private_or_record_AST = RefAdaAST(currentAST.root);
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_4);
		} else {
			throw;
		}
	}
	returnAST = derived_or_private_or_record_AST;
}

void AdaParser::local_enum_name() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST local_enum_name_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefAdaAST tmp179_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp179_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp179_AST));
		}
		match(IDENTIFIER);
		local_enum_name_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_65);
		} else {
			throw;
		}
	}
	returnAST = local_enum_name_AST;
}

void AdaParser::enumeration_aggregate() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST enumeration_aggregate_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		parenth_values();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		enumeration_aggregate_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_4);
		} else {
			throw;
		}
	}
	returnAST = enumeration_aggregate_AST;
}

void AdaParser::aliased_constant_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST aliased_constant_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case ALIASED:
		{
			RefAdaAST tmp180_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp180_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp180_AST));
			}
			match(ALIASED);
			break;
		}
		case IDENTIFIER:
		case CONSTANT:
		case ARRAY:
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
		case CONSTANT:
		{
			RefAdaAST tmp181_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp181_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp181_AST));
			}
			match(CONSTANT);
			break;
		}
		case IDENTIFIER:
		case ARRAY:
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
			aliased_constant_opt_AST = RefAdaAST(currentAST.root);
#line 844 "ada.g"
			aliased_constant_opt_AST =
				  RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(MODIFIERS,"MODIFIERS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(aliased_constant_opt_AST))));
#line 6510 "AdaParser.cpp"
			currentAST.root = aliased_constant_opt_AST;
			if ( aliased_constant_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				aliased_constant_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = aliased_constant_opt_AST->getFirstChild();
			else
				currentAST.child = aliased_constant_opt_AST;
			currentAST.advanceChildToEnd();
		}
		aliased_constant_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_66);
		} else {
			throw;
		}
	}
	returnAST = aliased_constant_opt_AST;
}

void AdaParser::array_type_definition(
	RefAdaAST t
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST array_type_definition_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(ARRAY);
		match(LPAREN);
		index_or_discrete_range_s();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(RPAREN);
		match(OF);
		component_subtype_def();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if ( inputState->guessing==0 ) {
#line 655 "ada.g"
			Set(t, ARRAY_TYPE_DECLARATION);
#line 6555 "AdaParser.cpp"
		}
		array_type_definition_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_62);
		} else {
			throw;
		}
	}
	returnAST = array_type_definition_AST;
}

void AdaParser::enum_id_s() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST enum_id_s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		enumeration_literal_specification();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				enumeration_literal_specification();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop195;
			}
			
		}
		_loop195:;
		} // ( ... )*
		enum_id_s_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_47);
		} else {
			throw;
		}
	}
	returnAST = enum_id_s_AST;
}

void AdaParser::range_constraint_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST range_constraint_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case RANGE:
		{
			range_constraint();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case SEMI:
		case COMMA:
		case RPAREN:
		case WITH:
		case ASSIGN:
		case LOOP:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		range_constraint_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_58);
		} else {
			throw;
		}
	}
	returnAST = range_constraint_opt_AST;
}

void AdaParser::access_type_definition(
	RefAdaAST t
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST access_type_definition_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(ACCESS);
		{
		switch ( LA(1)) {
		case PROCEDURE:
		case FUNCTION:
		case PROTECTED:
		{
			protected_opt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{
			switch ( LA(1)) {
			case PROCEDURE:
			{
				match(PROCEDURE);
				formal_part_opt();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				if ( inputState->guessing==0 ) {
#line 737 "ada.g"
					Set(t, ACCESS_TO_PROCEDURE_DECLARATION);
#line 6684 "AdaParser.cpp"
				}
				break;
			}
			case FUNCTION:
			{
				match(FUNCTION);
				func_formal_part_opt();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				match(RETURN);
				subtype_mark();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				if ( inputState->guessing==0 ) {
#line 739 "ada.g"
					Set(t, ACCESS_TO_FUNCTION_DECLARATION);
#line 6703 "AdaParser.cpp"
				}
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
		case IDENTIFIER:
		case ALL:
		case CONSTANT:
		{
			constant_all_opt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			subtype_ind();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			if ( inputState->guessing==0 ) {
#line 742 "ada.g"
				Set(t, ACCESS_TO_OBJECT_DECLARATION);
#line 6730 "AdaParser.cpp"
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		access_type_definition_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_4);
		} else {
			throw;
		}
	}
	returnAST = access_type_definition_AST;
}

void AdaParser::enumeration_literal_specification() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST enumeration_literal_specification_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case IDENTIFIER:
		{
			RefAdaAST tmp191_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp191_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp191_AST));
			}
			match(IDENTIFIER);
			enumeration_literal_specification_AST = RefAdaAST(currentAST.root);
			break;
		}
		case CHARACTER_LITERAL:
		{
			RefAdaAST tmp192_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp192_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp192_AST));
			}
			match(CHARACTER_LITERAL);
			enumeration_literal_specification_AST = RefAdaAST(currentAST.root);
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
			recover(ex,_tokenSet_7);
		} else {
			throw;
		}
	}
	returnAST = enumeration_literal_specification_AST;
}

void AdaParser::index_or_discrete_range_s() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST index_or_discrete_range_s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		index_or_discrete_range();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				RefAdaAST tmp193_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp193_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp193_AST));
				}
				match(COMMA);
				index_or_discrete_range();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop202;
			}
			
		}
		_loop202:;
		} // ( ... )*
		index_or_discrete_range_s_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_47);
		} else {
			throw;
		}
	}
	returnAST = index_or_discrete_range_s_AST;
}

void AdaParser::index_or_discrete_range() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST index_or_discrete_range_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		simple_expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case DOT_DOT:
		{
			RefAdaAST tmp194_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp194_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp194_AST));
			}
			match(DOT_DOT);
			simple_expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case RANGE:
		{
			RefAdaAST tmp195_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp195_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp195_AST));
			}
			match(RANGE);
			{
			switch ( LA(1)) {
			case BOX:
			{
				RefAdaAST tmp196_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp196_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp196_AST));
				}
				match(BOX);
				break;
			}
			case IDENTIFIER:
			case LPAREN:
			case NEW:
			case CHARACTER_LITERAL:
			case CHAR_STRING:
			case NuLL:
			case NOT:
			case PLUS:
			case MINUS:
			case ABS:
			case NUMERIC_LIT:
			{
				range();
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
			break;
		}
		case COMMA:
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
		index_or_discrete_range_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_7);
		} else {
			throw;
		}
	}
	returnAST = index_or_discrete_range_AST;
}

void AdaParser::aliased_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST aliased_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case ALIASED:
		{
			RefAdaAST tmp197_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp197_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp197_AST));
			}
			match(ALIASED);
			break;
		}
		case IDENTIFIER:
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
			aliased_opt_AST = RefAdaAST(currentAST.root);
#line 675 "ada.g"
			aliased_opt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(MODIFIERS,"MODIFIERS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(aliased_opt_AST))));
#line 6971 "AdaParser.cpp"
			currentAST.root = aliased_opt_AST;
			if ( aliased_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				aliased_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = aliased_opt_AST->getFirstChild();
			else
				currentAST.child = aliased_opt_AST;
			currentAST.advanceChildToEnd();
		}
		aliased_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_39);
		} else {
			throw;
		}
	}
	returnAST = aliased_opt_AST;
}

void AdaParser::constraint_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST constraint_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case RANGE:
		{
			range_constraint();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case DIGITS:
		{
			digits_constraint();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case DELTA:
		{
			delta_constraint();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case SEMI:
		case COMMA:
		case RPAREN:
		case WITH:
		case ASSIGN:
		case LOOP:
		{
			break;
		}
		default:
			bool synPredMatched213 = false;
			if (((LA(1) == LPAREN) && (_tokenSet_5.member(LA(2))))) {
				int _m213 = mark();
				synPredMatched213 = true;
				inputState->guessing++;
				try {
					{
					index_constraint();
					}
				}
				catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
					synPredMatched213 = false;
				}
				rewind(_m213);
				inputState->guessing--;
			}
			if ( synPredMatched213 ) {
				index_constraint();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else if ((LA(1) == LPAREN) && (_tokenSet_5.member(LA(2)))) {
				discriminant_constraint();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		constraint_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_58);
		} else {
			throw;
		}
	}
	returnAST = constraint_opt_AST;
}

void AdaParser::digits_constraint() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST digits_constraint_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  d = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST d_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		d = LT(1);
		if ( inputState->guessing == 0 ) {
			d_AST = astFactory->create(d);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(d_AST));
		}
		match(DIGITS);
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		range_constraint_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if ( inputState->guessing==0 ) {
#line 692 "ada.g"
			Set(d_AST, DIGITS_CONSTRAINT);
#line 7106 "AdaParser.cpp"
		}
		digits_constraint_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_58);
		} else {
			throw;
		}
	}
	returnAST = digits_constraint_AST;
}

void AdaParser::delta_constraint() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST delta_constraint_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  d = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST d_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		d = LT(1);
		if ( inputState->guessing == 0 ) {
			d_AST = astFactory->create(d);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(d_AST));
		}
		match(DELTA);
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		range_constraint_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if ( inputState->guessing==0 ) {
#line 696 "ada.g"
			Set(d_AST, DELTA_CONSTRAINT);
#line 7146 "AdaParser.cpp"
		}
		delta_constraint_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_58);
		} else {
			throw;
		}
	}
	returnAST = delta_constraint_AST;
}

void AdaParser::index_constraint() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST index_constraint_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  p = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST p_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		p = LT(1);
		if ( inputState->guessing == 0 ) {
			p_AST = astFactory->create(p);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(p_AST));
		}
		match(LPAREN);
		discrete_range();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				discrete_range();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop218;
			}
			
		}
		_loop218:;
		} // ( ... )*
		match(RPAREN);
		if ( inputState->guessing==0 ) {
#line 700 "ada.g"
			Set(p_AST, INDEX_CONSTRAINT);
#line 7199 "AdaParser.cpp"
		}
		index_constraint_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_58);
		} else {
			throw;
		}
	}
	returnAST = index_constraint_AST;
}

void AdaParser::discriminant_constraint() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST discriminant_constraint_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  p = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST p_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		p = LT(1);
		if ( inputState->guessing == 0 ) {
			p_AST = astFactory->create(p);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(p_AST));
		}
		match(LPAREN);
		discriminant_association();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				discriminant_association();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop224;
			}
			
		}
		_loop224:;
		} // ( ... )*
		match(RPAREN);
		if ( inputState->guessing==0 ) {
#line 710 "ada.g"
			Set(p_AST, DISCRIMINANT_CONSTRAINT);
#line 7252 "AdaParser.cpp"
		}
		discriminant_constraint_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_58);
		} else {
			throw;
		}
	}
	returnAST = discriminant_constraint_AST;
}

void AdaParser::discrete_range() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST discrete_range_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		bool synPredMatched221 = false;
		if (((_tokenSet_5.member(LA(1))) && (_tokenSet_33.member(LA(2))))) {
			int _m221 = mark();
			synPredMatched221 = true;
			inputState->guessing++;
			try {
				{
				range();
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched221 = false;
			}
			rewind(_m221);
			inputState->guessing--;
		}
		if ( synPredMatched221 ) {
			range();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			discrete_range_AST = RefAdaAST(currentAST.root);
		}
		else if ((LA(1) == IDENTIFIER) && (_tokenSet_67.member(LA(2)))) {
			subtype_ind();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			discrete_range_AST = RefAdaAST(currentAST.root);
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_7);
		} else {
			throw;
		}
	}
	returnAST = discrete_range_AST;
}

void AdaParser::discriminant_association() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST discriminant_association_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		selector_names_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if ( inputState->guessing==0 ) {
			discriminant_association_AST = RefAdaAST(currentAST.root);
#line 714 "ada.g"
			discriminant_association_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(DISCRIMINANT_ASSOCIATION,"DISCRIMINANT_ASSOCIATION")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(discriminant_association_AST))));
#line 7338 "AdaParser.cpp"
			currentAST.root = discriminant_association_AST;
			if ( discriminant_association_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				discriminant_association_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = discriminant_association_AST->getFirstChild();
			else
				currentAST.child = discriminant_association_AST;
			currentAST.advanceChildToEnd();
		}
		discriminant_association_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_7);
		} else {
			throw;
		}
	}
	returnAST = discriminant_association_AST;
}

void AdaParser::selector_names_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST selector_names_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		bool synPredMatched229 = false;
		if (((LA(1) == IDENTIFIER) && (LA(2) == RIGHT_SHAFT || LA(2) == PIPE))) {
			int _m229 = mark();
			synPredMatched229 = true;
			inputState->guessing++;
			try {
				{
				association_head();
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched229 = false;
			}
			rewind(_m229);
			inputState->guessing--;
		}
		if ( synPredMatched229 ) {
			association_head();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
		}
		else if ((_tokenSet_5.member(LA(1))) && (_tokenSet_6.member(LA(2)))) {
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
		if ( inputState->guessing==0 ) {
			selector_names_opt_AST = RefAdaAST(currentAST.root);
#line 722 "ada.g"
			selector_names_opt_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(SELECTOR_NAMES_OPT,"SELECTOR_NAMES_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(selector_names_opt_AST))));
#line 7401 "AdaParser.cpp"
			currentAST.root = selector_names_opt_AST;
			if ( selector_names_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				selector_names_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = selector_names_opt_AST->getFirstChild();
			else
				currentAST.child = selector_names_opt_AST;
			currentAST.advanceChildToEnd();
		}
		selector_names_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_5);
		} else {
			throw;
		}
	}
	returnAST = selector_names_opt_AST;
}

void AdaParser::association_head() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST association_head_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		selector_name();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == PIPE)) {
				match(PIPE);
				selector_name();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop232;
			}
			
		}
		_loop232:;
		} // ( ... )*
		match(RIGHT_SHAFT);
		association_head_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_5);
		} else {
			throw;
		}
	}
	returnAST = association_head_AST;
}

void AdaParser::selector_name() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST selector_name_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefAdaAST tmp204_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp204_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp204_AST));
		}
		match(IDENTIFIER);
		selector_name_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_68);
		} else {
			throw;
		}
	}
	returnAST = selector_name_AST;
}

void AdaParser::protected_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST protected_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case PROTECTED:
		{
			RefAdaAST tmp205_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp205_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp205_AST));
			}
			match(PROTECTED);
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
		if ( inputState->guessing==0 ) {
			protected_opt_AST = RefAdaAST(currentAST.root);
#line 747 "ada.g"
			protected_opt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(MODIFIERS,"MODIFIERS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(protected_opt_AST))));
#line 7521 "AdaParser.cpp"
			currentAST.root = protected_opt_AST;
			if ( protected_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				protected_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = protected_opt_AST->getFirstChild();
			else
				currentAST.child = protected_opt_AST;
			currentAST.advanceChildToEnd();
		}
		protected_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_69);
		} else {
			throw;
		}
	}
	returnAST = protected_opt_AST;
}

void AdaParser::constant_all_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST constant_all_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case CONSTANT:
		{
			RefAdaAST tmp206_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp206_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp206_AST));
			}
			match(CONSTANT);
			break;
		}
		case ALL:
		{
			RefAdaAST tmp207_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp207_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp207_AST));
			}
			match(ALL);
			break;
		}
		case IDENTIFIER:
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
			constant_all_opt_AST = RefAdaAST(currentAST.root);
#line 751 "ada.g"
			constant_all_opt_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(MODIFIERS,"MODIFIERS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(constant_all_opt_AST))));
#line 7586 "AdaParser.cpp"
			currentAST.root = constant_all_opt_AST;
			if ( constant_all_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				constant_all_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = constant_all_opt_AST->getFirstChild();
			else
				currentAST.child = constant_all_opt_AST;
			currentAST.advanceChildToEnd();
		}
		constant_all_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_39);
		} else {
			throw;
		}
	}
	returnAST = constant_all_opt_AST;
}

void AdaParser::abstract_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST abstract_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case ABSTRACT:
		{
			RefAdaAST tmp208_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp208_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp208_AST));
			}
			match(ABSTRACT);
			break;
		}
		case NEW:
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
			abstract_opt_AST = RefAdaAST(currentAST.root);
#line 771 "ada.g"
			abstract_opt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(MODIFIERS,"MODIFIERS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(abstract_opt_AST))));
#line 7640 "AdaParser.cpp"
			currentAST.root = abstract_opt_AST;
			if ( abstract_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				abstract_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = abstract_opt_AST->getFirstChild();
			else
				currentAST.child = abstract_opt_AST;
			currentAST.advanceChildToEnd();
		}
		abstract_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_70);
		} else {
			throw;
		}
	}
	returnAST = abstract_opt_AST;
}

void AdaParser::record_definition(
	bool has_discrim
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST record_definition_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case RECORD:
		{
			match(RECORD);
			component_list(has_discrim);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(END);
			match(RECORD);
			record_definition_AST = RefAdaAST(currentAST.root);
			break;
		}
		case NuLL:
		{
			match(NuLL);
			match(RECORD);
			record_definition_AST = RefAdaAST(currentAST.root);
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
			recover(ex,_tokenSet_4);
		} else {
			throw;
		}
	}
	returnAST = record_definition_AST;
}

void AdaParser::abstract_tagged_limited_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST abstract_tagged_limited_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case ABSTRACT:
		{
			RefAdaAST tmp214_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp214_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp214_AST));
			}
			match(ABSTRACT);
			match(TAGGED);
			break;
		}
		case TAGGED:
		{
			RefAdaAST tmp216_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp216_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp216_AST));
			}
			match(TAGGED);
			break;
		}
		case PRIVATE:
		case NuLL:
		case RECORD:
		case LIMITED:
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
		case LIMITED:
		{
			RefAdaAST tmp217_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp217_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp217_AST));
			}
			match(LIMITED);
			break;
		}
		case PRIVATE:
		case NuLL:
		case RECORD:
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
			abstract_tagged_limited_opt_AST = RefAdaAST(currentAST.root);
#line 833 "ada.g"
			abstract_tagged_limited_opt_AST =
				  RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(MODIFIERS,"MODIFIERS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(abstract_tagged_limited_opt_AST))));
#line 7778 "AdaParser.cpp"
			currentAST.root = abstract_tagged_limited_opt_AST;
			if ( abstract_tagged_limited_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				abstract_tagged_limited_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = abstract_tagged_limited_opt_AST->getFirstChild();
			else
				currentAST.child = abstract_tagged_limited_opt_AST;
			currentAST.advanceChildToEnd();
		}
		abstract_tagged_limited_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_71);
		} else {
			throw;
		}
	}
	returnAST = abstract_tagged_limited_opt_AST;
}

void AdaParser::component_list(
	bool has_discrim
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST component_list_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case NuLL:
		{
			match(NuLL);
			match(SEMI);
			component_list_AST = RefAdaAST(currentAST.root);
			break;
		}
		case PRAGMA:
		case IDENTIFIER:
		{
			component_items();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{
			switch ( LA(1)) {
			case CASE:
			{
				variant_part();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				if (!( has_discrim ))
					throw ANTLR_USE_NAMESPACE(antlr)SemanticException(" has_discrim ");
				break;
			}
			case END:
			case WHEN:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			component_list_AST = RefAdaAST(currentAST.root);
			break;
		}
		case CASE:
		{
			empty_component_items();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			variant_part();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			if (!( has_discrim ))
				throw ANTLR_USE_NAMESPACE(antlr)SemanticException(" has_discrim ");
			component_list_AST = RefAdaAST(currentAST.root);
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
			recover(ex,_tokenSet_72);
		} else {
			throw;
		}
	}
	returnAST = component_list_AST;
}

void AdaParser::component_items() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST component_items_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{ // ( ... )+
		int _cnt254=0;
		for (;;) {
			switch ( LA(1)) {
			case PRAGMA:
			{
				pragma();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case IDENTIFIER:
			{
				comp_decl();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			default:
			{
				if ( _cnt254>=1 ) { goto _loop254; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
			}
			}
			_cnt254++;
		}
		_loop254:;
		}  // ( ... )+
		if ( inputState->guessing==0 ) {
			component_items_AST = RefAdaAST(currentAST.root);
#line 786 "ada.g"
			component_items_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(COMPONENT_ITEMS,"COMPONENT_ITEMS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(component_items_AST))));
#line 7921 "AdaParser.cpp"
			currentAST.root = component_items_AST;
			if ( component_items_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				component_items_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = component_items_AST->getFirstChild();
			else
				currentAST.child = component_items_AST;
			currentAST.advanceChildToEnd();
		}
		component_items_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_73);
		} else {
			throw;
		}
	}
	returnAST = component_items_AST;
}

void AdaParser::variant_part() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST variant_part_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  c = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST c_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		c = LT(1);
		if ( inputState->guessing == 0 ) {
			c_AST = astFactory->create(c);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(c_AST));
		}
		match(CASE);
		discriminant_direct_name();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(IS);
		variant_s();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(END);
		match(CASE);
		match(SEMI);
		if ( inputState->guessing==0 ) {
#line 798 "ada.g"
			Set (c_AST, VARIANT_PART);
#line 7972 "AdaParser.cpp"
		}
		variant_part_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_72);
		} else {
			throw;
		}
	}
	returnAST = variant_part_AST;
}

void AdaParser::empty_component_items() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST empty_component_items_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		if ( inputState->guessing==0 ) {
			empty_component_items_AST = RefAdaAST(currentAST.root);
#line 792 "ada.g"
			empty_component_items_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(COMPONENT_ITEMS,"COMPONENT_ITEMS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(empty_component_items_AST))));
#line 7998 "AdaParser.cpp"
			currentAST.root = empty_component_items_AST;
			if ( empty_component_items_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				empty_component_items_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = empty_component_items_AST->getFirstChild();
			else
				currentAST.child = empty_component_items_AST;
			currentAST.advanceChildToEnd();
		}
		empty_component_items_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_74);
		} else {
			throw;
		}
	}
	returnAST = empty_component_items_AST;
}

void AdaParser::discriminant_direct_name() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST discriminant_direct_name_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefAdaAST tmp224_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp224_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp224_AST));
		}
		match(IDENTIFIER);
		discriminant_direct_name_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_75);
		} else {
			throw;
		}
	}
	returnAST = discriminant_direct_name_AST;
}

void AdaParser::variant_s() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST variant_s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{ // ( ... )+
		int _cnt260=0;
		for (;;) {
			if ((LA(1) == WHEN)) {
				variant();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				if ( _cnt260>=1 ) { goto _loop260; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
			}
			
			_cnt260++;
		}
		_loop260:;
		}  // ( ... )+
		if ( inputState->guessing==0 ) {
			variant_s_AST = RefAdaAST(currentAST.root);
#line 805 "ada.g"
			variant_s_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(VARIANTS,"VARIANTS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(variant_s_AST))));
#line 8072 "AdaParser.cpp"
			currentAST.root = variant_s_AST;
			if ( variant_s_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				variant_s_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = variant_s_AST->getFirstChild();
			else
				currentAST.child = variant_s_AST;
			currentAST.advanceChildToEnd();
		}
		variant_s_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_21);
		} else {
			throw;
		}
	}
	returnAST = variant_s_AST;
}

void AdaParser::variant() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST variant_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  w = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST w_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		w = LT(1);
		if ( inputState->guessing == 0 ) {
			w_AST = astFactory->create(w);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(w_AST));
		}
		match(WHEN);
		choice_s();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(RIGHT_SHAFT);
		component_list(true);
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if ( inputState->guessing==0 ) {
#line 809 "ada.g"
			Set (w_AST, VARIANT);
#line 8120 "AdaParser.cpp"
		}
		variant_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_72);
		} else {
			throw;
		}
	}
	returnAST = variant_AST;
}

void AdaParser::choice_s() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST choice_s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		choice();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == PIPE)) {
				RefAdaAST tmp226_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp226_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp226_AST));
				}
				match(PIPE);
				choice();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop264;
			}
			
		}
		_loop264:;
		} // ( ... )*
		choice_s_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_76);
		} else {
			throw;
		}
	}
	returnAST = choice_s_AST;
}

void AdaParser::choice() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST choice_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		if ((LA(1) == OTHERS)) {
			RefAdaAST tmp227_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp227_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp227_AST));
			}
			match(OTHERS);
			choice_AST = RefAdaAST(currentAST.root);
		}
		else {
			bool synPredMatched267 = false;
			if (((_tokenSet_5.member(LA(1))) && (_tokenSet_77.member(LA(2))))) {
				int _m267 = mark();
				synPredMatched267 = true;
				inputState->guessing++;
				try {
					{
					discrete_with_range();
					}
				}
				catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
					synPredMatched267 = false;
				}
				rewind(_m267);
				inputState->guessing--;
			}
			if ( synPredMatched267 ) {
				discrete_with_range();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				choice_AST = RefAdaAST(currentAST.root);
			}
			else if ((_tokenSet_5.member(LA(1))) && (_tokenSet_78.member(LA(2)))) {
				expression();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				choice_AST = RefAdaAST(currentAST.root);
			}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_68);
		} else {
			throw;
		}
	}
	returnAST = choice_AST;
}

void AdaParser::discrete_with_range() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST discrete_with_range_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		bool synPredMatched270 = false;
		if (((LA(1) == IDENTIFIER) && (LA(2) == DOT || LA(2) == TIC || LA(2) == RANGE))) {
			int _m270 = mark();
			synPredMatched270 = true;
			inputState->guessing++;
			try {
				{
				mark_with_constraint();
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched270 = false;
			}
			rewind(_m270);
			inputState->guessing--;
		}
		if ( synPredMatched270 ) {
			mark_with_constraint();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			discrete_with_range_AST = RefAdaAST(currentAST.root);
		}
		else if ((_tokenSet_5.member(LA(1))) && (_tokenSet_33.member(LA(2)))) {
			range();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			discrete_with_range_AST = RefAdaAST(currentAST.root);
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_68);
		} else {
			throw;
		}
	}
	returnAST = discrete_with_range_AST;
}

void AdaParser::mark_with_constraint() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST mark_with_constraint_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		subtype_mark();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		range_constraint();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if ( inputState->guessing==0 ) {
			mark_with_constraint_AST = RefAdaAST(currentAST.root);
#line 825 "ada.g"
			mark_with_constraint_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(MARK_WITH_CONSTRAINT,"MARK_WITH_CONSTRAINT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(mark_with_constraint_AST))));
#line 8312 "AdaParser.cpp"
			currentAST.root = mark_with_constraint_AST;
			if ( mark_with_constraint_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				mark_with_constraint_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = mark_with_constraint_AST->getFirstChild();
			else
				currentAST.child = mark_with_constraint_AST;
			currentAST.advanceChildToEnd();
		}
		mark_with_constraint_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_68);
		} else {
			throw;
		}
	}
	returnAST = mark_with_constraint_AST;
}

void AdaParser::generic_formal_part_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST generic_formal_part_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{ // ( ... )*
		for (;;) {
			switch ( LA(1)) {
			case USE:
			{
				use_clause();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case PRAGMA:
			{
				pragma();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case IDENTIFIER:
			case WITH:
			case TYPE:
			{
				generic_formal_parameter();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			default:
			{
				goto _loop287;
			}
			}
		}
		_loop287:;
		} // ( ... )*
		if ( inputState->guessing==0 ) {
			generic_formal_part_opt_AST = RefAdaAST(currentAST.root);
#line 871 "ada.g"
			generic_formal_part_opt_AST =
						RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(GENERIC_FORMAL_PART,"GENERIC_FORMAL_PART")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(generic_formal_part_opt_AST))));
#line 8382 "AdaParser.cpp"
			currentAST.root = generic_formal_part_opt_AST;
			if ( generic_formal_part_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				generic_formal_part_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = generic_formal_part_opt_AST->getFirstChild();
			else
				currentAST.child = generic_formal_part_opt_AST;
			currentAST.advanceChildToEnd();
		}
		generic_formal_part_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_79);
		} else {
			throw;
		}
	}
	returnAST = generic_formal_part_opt_AST;
}

void AdaParser::generic_formal_parameter() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST generic_formal_parameter_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  t = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST t_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  w = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST w_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case TYPE:
		{
			t = LT(1);
			if ( inputState->guessing == 0 ) {
				t_AST = astFactory->create(t);
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(t_AST));
			}
			match(TYPE);
			def_id(false);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{
			switch ( LA(1)) {
			case IS:
			{
				match(IS);
				{
				switch ( LA(1)) {
				case LPAREN:
				{
					match(LPAREN);
					match(BOX);
					match(RPAREN);
					if ( inputState->guessing==0 ) {
#line 881 "ada.g"
						Set (t_AST, FORMAL_DISCRETE_TYPE_DECLARATION);
#line 8443 "AdaParser.cpp"
					}
					break;
				}
				case RANGE:
				{
					match(RANGE);
					match(BOX);
					if ( inputState->guessing==0 ) {
#line 883 "ada.g"
						Set (t_AST, FORMAL_SIGNED_INTEGER_TYPE_DECLARATION);
#line 8454 "AdaParser.cpp"
					}
					break;
				}
				case MOD:
				{
					match(MOD);
					match(BOX);
					if ( inputState->guessing==0 ) {
#line 885 "ada.g"
						Set (t_AST, FORMAL_MODULAR_TYPE_DECLARATION);
#line 8465 "AdaParser.cpp"
					}
					break;
				}
				case DELTA:
				{
					match(DELTA);
					match(BOX);
					{
					switch ( LA(1)) {
					case DIGITS:
					{
						match(DIGITS);
						match(BOX);
						if ( inputState->guessing==0 ) {
#line 888 "ada.g"
							Set (t_AST, FORMAL_DECIMAL_FIXED_POINT_DECLARATION);
#line 8482 "AdaParser.cpp"
						}
						break;
					}
					case SEMI:
					{
						if ( inputState->guessing==0 ) {
#line 889 "ada.g"
							Set (t_AST, FORMAL_ORDINARY_FIXED_POINT_DECLARATION);
#line 8491 "AdaParser.cpp"
						}
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
				case DIGITS:
				{
					match(DIGITS);
					match(BOX);
					if ( inputState->guessing==0 ) {
#line 892 "ada.g"
						Set (t_AST, FORMAL_FLOATING_POINT_DECLARATION);
#line 8510 "AdaParser.cpp"
					}
					break;
				}
				case ARRAY:
				{
					array_type_definition(t_AST);
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					break;
				}
				case ACCESS:
				{
					access_type_definition(t_AST);
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					break;
				}
				case PRIVATE:
				case NEW:
				case ABSTRACT:
				case TAGGED:
				case LIMITED:
				{
					empty_discrim_opt();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					discriminable_type_definition(t_AST);
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
				break;
			}
			case LPAREN:
			{
				discrim_part();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				match(IS);
				discriminable_type_definition(t_AST);
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
			if ( inputState->guessing==0 ) {
#line 899 "ada.g"
				pop_def_id();
#line 8576 "AdaParser.cpp"
			}
			break;
		}
		case WITH:
		{
			w = LT(1);
			if ( inputState->guessing == 0 ) {
				w_AST = astFactory->create(w);
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(w_AST));
			}
			match(WITH);
			{
			switch ( LA(1)) {
			case PROCEDURE:
			{
				match(PROCEDURE);
				def_id(false);
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				formal_part_opt();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				subprogram_default_opt();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				if ( inputState->guessing==0 ) {
#line 901 "ada.g"
					Set(w_AST, FORMAL_PROCEDURE_DECLARATION);
#line 8608 "AdaParser.cpp"
				}
				break;
			}
			case FUNCTION:
			{
				match(FUNCTION);
				def_designator(false);
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				function_tail();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				subprogram_default_opt();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				if ( inputState->guessing==0 ) {
#line 903 "ada.g"
					Set(w_AST, FORMAL_FUNCTION_DECLARATION);
#line 8630 "AdaParser.cpp"
				}
				break;
			}
			case PACKAGE:
			{
				match(PACKAGE);
				def_id(false);
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				match(IS);
				match(NEW);
				compound_name();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				formal_package_actual_part_opt();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				if ( inputState->guessing==0 ) {
#line 905 "ada.g"
					Set(w_AST, FORMAL_PACKAGE_DECLARATION);
#line 8654 "AdaParser.cpp"
				}
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			if ( inputState->guessing==0 ) {
#line 907 "ada.g"
				pop_def_id();
#line 8667 "AdaParser.cpp"
			}
			break;
		}
		case IDENTIFIER:
		{
			parameter_specification();
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
		generic_formal_parameter_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_80);
		} else {
			throw;
		}
	}
	returnAST = generic_formal_parameter_AST;
}

void AdaParser::discriminable_type_definition(
	RefAdaAST t
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST discriminable_type_definition_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		bool synPredMatched297 = false;
		if (((LA(1) == NEW || LA(1) == ABSTRACT) && (LA(2) == IDENTIFIER || LA(2) == NEW))) {
			int _m297 = mark();
			synPredMatched297 = true;
			inputState->guessing++;
			try {
				{
				{
				switch ( LA(1)) {
				case ABSTRACT:
				{
					match(ABSTRACT);
					break;
				}
				case NEW:
				{
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				match(NEW);
				subtype_ind();
				match(WITH);
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched297 = false;
			}
			rewind(_m297);
			inputState->guessing--;
		}
		if ( synPredMatched297 ) {
			abstract_opt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(NEW);
			subtype_ind();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(WITH);
			match(PRIVATE);
			if ( inputState->guessing==0 ) {
#line 916 "ada.g"
				Set (t, FORMAL_PRIVATE_EXTENSION_DECLARATION);
#line 8757 "AdaParser.cpp"
			}
			discriminable_type_definition_AST = RefAdaAST(currentAST.root);
		}
		else if ((LA(1) == NEW) && (LA(2) == IDENTIFIER)) {
			match(NEW);
			subtype_ind();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			if ( inputState->guessing==0 ) {
#line 918 "ada.g"
				Set (t, FORMAL_ORDINARY_DERIVED_TYPE_DECLARATION);
#line 8770 "AdaParser.cpp"
			}
			discriminable_type_definition_AST = RefAdaAST(currentAST.root);
		}
		else if ((_tokenSet_81.member(LA(1))) && (_tokenSet_82.member(LA(2)))) {
			abstract_tagged_limited_opt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(PRIVATE);
			if ( inputState->guessing==0 ) {
#line 920 "ada.g"
				Set (t, FORMAL_PRIVATE_TYPE_DECLARATION);
#line 8783 "AdaParser.cpp"
			}
			discriminable_type_definition_AST = RefAdaAST(currentAST.root);
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_4);
		} else {
			throw;
		}
	}
	returnAST = discriminable_type_definition_AST;
}

void AdaParser::subprogram_default_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST subprogram_default_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case IS:
		{
			match(IS);
			{
			switch ( LA(1)) {
			case BOX:
			{
				RefAdaAST tmp255_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp255_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp255_AST));
				}
				match(BOX);
				break;
			}
			case IDENTIFIER:
			{
				name();
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
		subprogram_default_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_4);
		} else {
			throw;
		}
	}
	returnAST = subprogram_default_opt_AST;
}

void AdaParser::formal_package_actual_part_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST formal_package_actual_part_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LPAREN:
		{
			match(LPAREN);
			{
			switch ( LA(1)) {
			case BOX:
			{
				RefAdaAST tmp257_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp257_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp257_AST));
				}
				match(BOX);
				break;
			}
			case IDENTIFIER:
			{
				defining_identifier_list();
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
		formal_package_actual_part_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_4);
		} else {
			throw;
		}
	}
	returnAST = formal_package_actual_part_opt_AST;
}

void AdaParser::body_part() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST body_part_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		declarative_part();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		block_body();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		end_id_opt();
		body_part_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_4);
		} else {
			throw;
		}
	}
	returnAST = body_part_AST;
}

void AdaParser::declarative_part() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST declarative_part_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{ // ( ... )*
		for (;;) {
			switch ( LA(1)) {
			case PRAGMA:
			{
				pragma();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case IDENTIFIER:
			case USE:
			case TYPE:
			case PACKAGE:
			case PROCEDURE:
			case FUNCTION:
			case TASK:
			case PROTECTED:
			case FOR:
			case SUBTYPE:
			case GENERIC:
			{
				declarative_item();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			default:
			{
				goto _loop314;
			}
			}
		}
		_loop314:;
		} // ( ... )*
		if ( inputState->guessing==0 ) {
			declarative_part_AST = RefAdaAST(currentAST.root);
#line 964 "ada.g"
			declarative_part_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(DECLARATIVE_PART,"DECLARATIVE_PART")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(declarative_part_AST))));
#line 9004 "AdaParser.cpp"
			currentAST.root = declarative_part_AST;
			if ( declarative_part_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				declarative_part_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = declarative_part_AST->getFirstChild();
			else
				currentAST.child = declarative_part_AST;
			currentAST.advanceChildToEnd();
		}
		declarative_part_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_83);
		} else {
			throw;
		}
	}
	returnAST = declarative_part_AST;
}

void AdaParser::block_body() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST block_body_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  b = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST b_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		b = LT(1);
		if ( inputState->guessing == 0 ) {
			b_AST = astFactory->create(b);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(b_AST));
		}
		match(BEGIN);
		handled_stmt_s();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if ( inputState->guessing==0 ) {
#line 1044 "ada.g"
			Set(b_AST, BLOCK_BODY);
#line 9047 "AdaParser.cpp"
		}
		block_body_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_21);
		} else {
			throw;
		}
	}
	returnAST = block_body_AST;
}

void AdaParser::declarative_item() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST declarative_item_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  pkg = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST pkg_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  tsk = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST tsk_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  pro = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST pro_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case PACKAGE:
		{
			pkg = LT(1);
			if ( inputState->guessing == 0 ) {
				pkg_AST = astFactory->create(pkg);
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(pkg_AST));
			}
			match(PACKAGE);
			{
			switch ( LA(1)) {
			case BODY:
			{
				body_is();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				{
				switch ( LA(1)) {
				case SEPARATE:
				{
					separate();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					if ( inputState->guessing==0 ) {
#line 972 "ada.g"
						Set(pkg_AST, PACKAGE_BODY_STUB);
#line 9103 "AdaParser.cpp"
					}
					break;
				}
				case PRAGMA:
				case IDENTIFIER:
				case USE:
				case TYPE:
				case PACKAGE:
				case PROCEDURE:
				case FUNCTION:
				case TASK:
				case PROTECTED:
				case FOR:
				case END:
				case SUBTYPE:
				case GENERIC:
				case BEGIN:
				{
					pkg_body_part();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					end_id_opt();
					if ( inputState->guessing==0 ) {
#line 974 "ada.g"
						Set(pkg_AST, PACKAGE_BODY);
#line 9130 "AdaParser.cpp"
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
				break;
			}
			case IDENTIFIER:
			{
				def_id(false);
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				spec_decl_part(pkg_AST);
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
			break;
		}
		case TASK:
		{
			tsk = LT(1);
			if ( inputState->guessing == 0 ) {
				tsk_AST = astFactory->create(tsk);
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tsk_AST));
			}
			match(TASK);
			{
			switch ( LA(1)) {
			case BODY:
			{
				body_is();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				{
				switch ( LA(1)) {
				case SEPARATE:
				{
					separate();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					if ( inputState->guessing==0 ) {
#line 980 "ada.g"
						Set(tsk_AST, TASK_BODY_STUB);
#line 9190 "AdaParser.cpp"
					}
					break;
				}
				case PRAGMA:
				case IDENTIFIER:
				case USE:
				case TYPE:
				case PACKAGE:
				case PROCEDURE:
				case FUNCTION:
				case TASK:
				case PROTECTED:
				case FOR:
				case SUBTYPE:
				case GENERIC:
				case BEGIN:
				{
					body_part();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					if ( inputState->guessing==0 ) {
#line 981 "ada.g"
						Set(tsk_AST, TASK_BODY);
#line 9215 "AdaParser.cpp"
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
				break;
			}
			case IDENTIFIER:
			case TYPE:
			{
				task_type_or_single_decl(tsk_AST);
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
			break;
		}
		case PROTECTED:
		{
			pro = LT(1);
			if ( inputState->guessing == 0 ) {
				pro_AST = astFactory->create(pro);
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(pro_AST));
			}
			match(PROTECTED);
			{
			switch ( LA(1)) {
			case BODY:
			{
				body_is();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				{
				switch ( LA(1)) {
				case SEPARATE:
				{
					separate();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					if ( inputState->guessing==0 ) {
#line 988 "ada.g"
						Set(pro_AST, PROTECTED_BODY_STUB);
#line 9272 "AdaParser.cpp"
					}
					break;
				}
				case PRAGMA:
				case PROCEDURE:
				case FUNCTION:
				case ENTRY:
				case END:
				{
					prot_op_bodies_opt();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					end_id_opt();
					if ( inputState->guessing==0 ) {
#line 990 "ada.g"
						Set(pro_AST, PROTECTED_BODY);
#line 9290 "AdaParser.cpp"
					}
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
			case IDENTIFIER:
			case TYPE:
			{
				prot_type_or_single_decl(pro_AST);
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
			break;
		}
		case PROCEDURE:
		case FUNCTION:
		{
			subprog_decl_or_rename_or_inst_or_body(false);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case IDENTIFIER:
		case USE:
		case TYPE:
		case FOR:
		case SUBTYPE:
		case GENERIC:
		{
			decl_common();
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
		declarative_item_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_84);
		} else {
			throw;
		}
	}
	returnAST = declarative_item_AST;
}

void AdaParser::body_is() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST body_is_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(BODY);
		def_id(false);
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(IS);
		body_is_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_85);
		} else {
			throw;
		}
	}
	returnAST = body_is_AST;
}

void AdaParser::separate() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST separate_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(SEPARATE);
		if ( inputState->guessing==0 ) {
#line 1009 "ada.g"
			pop_def_id();
#line 9396 "AdaParser.cpp"
		}
		separate_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_4);
		} else {
			throw;
		}
	}
	returnAST = separate_AST;
}

void AdaParser::prot_op_bodies_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST prot_op_bodies_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{ // ( ... )*
		for (;;) {
			switch ( LA(1)) {
			case ENTRY:
			{
				entry_body();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case PROCEDURE:
			case FUNCTION:
			{
				subprog_decl_or_body();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case PRAGMA:
			{
				pragma();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
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
		if ( inputState->guessing==0 ) {
			prot_op_bodies_opt_AST = RefAdaAST(currentAST.root);
#line 1025 "ada.g"
			prot_op_bodies_opt_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(PROT_OP_BODIES_OPT,"PROT_OP_BODIES_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(prot_op_bodies_opt_AST))));
#line 9458 "AdaParser.cpp"
			currentAST.root = prot_op_bodies_opt_AST;
			if ( prot_op_bodies_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				prot_op_bodies_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = prot_op_bodies_opt_AST->getFirstChild();
			else
				currentAST.child = prot_op_bodies_opt_AST;
			currentAST.advanceChildToEnd();
		}
		prot_op_bodies_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_21);
		} else {
			throw;
		}
	}
	returnAST = prot_op_bodies_opt_AST;
}

void AdaParser::block_body_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST block_body_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case BEGIN:
		{
			match(BEGIN);
			handled_stmt_s();
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
		if ( inputState->guessing==0 ) {
			block_body_opt_AST = RefAdaAST(currentAST.root);
#line 1016 "ada.g"
			block_body_opt_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(BLOCK_BODY_OPT,"BLOCK_BODY_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(block_body_opt_AST))));
#line 9512 "AdaParser.cpp"
			currentAST.root = block_body_opt_AST;
			if ( block_body_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				block_body_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = block_body_opt_AST->getFirstChild();
			else
				currentAST.child = block_body_opt_AST;
			currentAST.advanceChildToEnd();
		}
		block_body_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_21);
		} else {
			throw;
		}
	}
	returnAST = block_body_opt_AST;
}

void AdaParser::handled_stmt_s() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST handled_stmt_s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		statements();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		except_handler_part_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if ( inputState->guessing==0 ) {
			handled_stmt_s_AST = RefAdaAST(currentAST.root);
#line 1048 "ada.g"
			handled_stmt_s_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(HANDLED_SEQUENCE_OF_STATEMENTS,"HANDLED_SEQUENCE_OF_STATEMENTS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(handled_stmt_s_AST))));
#line 9553 "AdaParser.cpp"
			currentAST.root = handled_stmt_s_AST;
			if ( handled_stmt_s_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				handled_stmt_s_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = handled_stmt_s_AST->getFirstChild();
			else
				currentAST.child = handled_stmt_s_AST;
			currentAST.advanceChildToEnd();
		}
		handled_stmt_s_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_21);
		} else {
			throw;
		}
	}
	returnAST = handled_stmt_s_AST;
}

void AdaParser::entry_body() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST entry_body_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  e = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST e_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		e = LT(1);
		if ( inputState->guessing == 0 ) {
			e_AST = astFactory->create(e);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(e_AST));
		}
		match(ENTRY);
		def_id(false);
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		entry_body_formal_part();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		entry_barrier();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(IS);
		body_part();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(SEMI);
		if ( inputState->guessing==0 ) {
#line 1211 "ada.g"
			Set (e_AST, ENTRY_BODY);
#line 9610 "AdaParser.cpp"
		}
		entry_body_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_86);
		} else {
			throw;
		}
	}
	returnAST = entry_body_AST;
}

void AdaParser::subprog_decl_or_body() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST subprog_decl_or_body_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  p = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST p_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  f = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST f_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case PROCEDURE:
		{
			p = LT(1);
			if ( inputState->guessing == 0 ) {
				p_AST = astFactory->create(p);
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(p_AST));
			}
			match(PROCEDURE);
			def_id(false);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			formal_part_opt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{
			switch ( LA(1)) {
			case IS:
			{
				match(IS);
				body_part();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				if ( inputState->guessing==0 ) {
#line 1032 "ada.g"
					Set(p_AST, PROCEDURE_BODY);
#line 9664 "AdaParser.cpp"
				}
				break;
			}
			case SEMI:
			{
				if ( inputState->guessing==0 ) {
#line 1033 "ada.g"
					pop_def_id(); Set(p_AST, PROCEDURE_DECLARATION);
#line 9673 "AdaParser.cpp"
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
			subprog_decl_or_body_AST = RefAdaAST(currentAST.root);
			break;
		}
		case FUNCTION:
		{
			f = LT(1);
			if ( inputState->guessing == 0 ) {
				f_AST = astFactory->create(f);
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(f_AST));
			}
			match(FUNCTION);
			def_designator(false);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			function_tail();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{
			switch ( LA(1)) {
			case IS:
			{
				match(IS);
				body_part();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				if ( inputState->guessing==0 ) {
#line 1037 "ada.g"
					Set(f_AST, FUNCTION_BODY);
#line 9715 "AdaParser.cpp"
				}
				break;
			}
			case SEMI:
			{
				if ( inputState->guessing==0 ) {
#line 1038 "ada.g"
					pop_def_id(); Set(f_AST, FUNCTION_DECLARATION);
#line 9724 "AdaParser.cpp"
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
			subprog_decl_or_body_AST = RefAdaAST(currentAST.root);
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
			recover(ex,_tokenSet_86);
		} else {
			throw;
		}
	}
	returnAST = subprog_decl_or_body_AST;
}

void AdaParser::statements() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST statements_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{ // ( ... )+
		int _cnt338=0;
		for (;;) {
			switch ( LA(1)) {
			case PRAGMA:
			{
				pragma();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case IDENTIFIER:
			case NuLL:
			case RETURN:
			case FOR:
			case CASE:
			case BEGIN:
			case LT_LT:
			case IF:
			case LOOP:
			case WHILE:
			case DECLARE:
			case EXIT:
			case GOTO:
			case ACCEPT:
			case DELAY:
			case SELECT:
			case ABORT:
			case RAISE:
			case REQUEUE:
			{
				statement();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			default:
			{
				if ( _cnt338>=1 ) { goto _loop338; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
			}
			}
			_cnt338++;
		}
		_loop338:;
		}  // ( ... )+
		if ( inputState->guessing==0 ) {
			statements_AST = RefAdaAST(currentAST.root);
#line 1054 "ada.g"
			statements_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(SEQUENCE_OF_STATEMENTS,"SEQUENCE_OF_STATEMENTS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(statements_AST))));
#line 9812 "AdaParser.cpp"
			currentAST.root = statements_AST;
			if ( statements_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				statements_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = statements_AST->getFirstChild();
			else
				currentAST.child = statements_AST;
			currentAST.advanceChildToEnd();
		}
		statements_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_87);
		} else {
			throw;
		}
	}
	returnAST = statements_AST;
}

void AdaParser::except_handler_part_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST except_handler_part_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case EXCEPTION:
		{
			match(EXCEPTION);
			{ // ( ... )+
			int _cnt424=0;
			for (;;) {
				if ((LA(1) == WHEN)) {
					exception_handler();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
				}
				else {
					if ( _cnt424>=1 ) { goto _loop424; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
				}
				
				_cnt424++;
			}
			_loop424:;
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
		if ( inputState->guessing==0 ) {
			except_handler_part_opt_AST = RefAdaAST(currentAST.root);
#line 1340 "ada.g"
			except_handler_part_opt_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(EXCEPT_HANDLER_PART_OPT,"EXCEPT_HANDLER_PART_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(except_handler_part_opt_AST))));
#line 9879 "AdaParser.cpp"
			currentAST.root = except_handler_part_opt_AST;
			if ( except_handler_part_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				except_handler_part_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = except_handler_part_opt_AST->getFirstChild();
			else
				currentAST.child = except_handler_part_opt_AST;
			currentAST.advanceChildToEnd();
		}
		except_handler_part_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_21);
		} else {
			throw;
		}
	}
	returnAST = except_handler_part_opt_AST;
}

void AdaParser::statement() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST statement_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		def_label_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case NuLL:
		{
			null_stmt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case EXIT:
		{
			exit_stmt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case RETURN:
		{
			return_stmt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case GOTO:
		{
			goto_stmt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case DELAY:
		{
			delay_stmt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case ABORT:
		{
			abort_stmt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case RAISE:
		{
			raise_stmt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case REQUEUE:
		{
			requeue_stmt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case ACCEPT:
		{
			accept_stmt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case SELECT:
		{
			select_stmt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case IF:
		{
			if_stmt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case CASE:
		{
			case_stmt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case FOR:
		case LOOP:
		case WHILE:
		{
			loop_stmt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(SEMI);
			break;
		}
		case BEGIN:
		case DECLARE:
		{
			block();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(END);
			match(SEMI);
			break;
		}
		default:
			if ((LA(1) == IDENTIFIER) && (LA(2) == COLON)) {
				statement_identifier();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				{
				switch ( LA(1)) {
				case FOR:
				case LOOP:
				case WHILE:
				{
					loop_stmt();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					id_opt();
					match(SEMI);
					break;
				}
				case BEGIN:
				case DECLARE:
				{
					block();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					end_id_opt();
					match(SEMI);
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
			}
			else if ((LA(1) == IDENTIFIER) && (_tokenSet_88.member(LA(2)))) {
				call_or_assignment();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		if ( inputState->guessing==0 ) {
			statement_AST = RefAdaAST(currentAST.root);
#line 1080 "ada.g"
			statement_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(STATEMENT,"STATEMENT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(statement_AST))));
#line 10084 "AdaParser.cpp"
			currentAST.root = statement_AST;
			if ( statement_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				statement_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = statement_AST->getFirstChild();
			else
				currentAST.child = statement_AST;
			currentAST.advanceChildToEnd();
		}
		statement_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_89);
		} else {
			throw;
		}
	}
	returnAST = statement_AST;
}

void AdaParser::def_label_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST def_label_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LT_LT:
		{
			match(LT_LT);
			RefAdaAST tmp279_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp279_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp279_AST));
			}
			match(IDENTIFIER);
			match(GT_GT);
			break;
		}
		case IDENTIFIER:
		case NuLL:
		case RETURN:
		case FOR:
		case CASE:
		case BEGIN:
		case IF:
		case LOOP:
		case WHILE:
		case DECLARE:
		case EXIT:
		case GOTO:
		case ACCEPT:
		case DELAY:
		case SELECT:
		case ABORT:
		case RAISE:
		case REQUEUE:
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
			def_label_opt_AST = RefAdaAST(currentAST.root);
#line 1084 "ada.g"
			def_label_opt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(LABEL_OPT,"LABEL_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(def_label_opt_AST))));
#line 10157 "AdaParser.cpp"
			currentAST.root = def_label_opt_AST;
			if ( def_label_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				def_label_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = def_label_opt_AST->getFirstChild();
			else
				currentAST.child = def_label_opt_AST;
			currentAST.advanceChildToEnd();
		}
		def_label_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_90);
		} else {
			throw;
		}
	}
	returnAST = def_label_opt_AST;
}

void AdaParser::null_stmt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST null_stmt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  s = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		s = LT(1);
		if ( inputState->guessing == 0 ) {
			s_AST = astFactory->create(s);
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(s_AST));
		}
		match(NuLL);
		match(SEMI);
		if ( inputState->guessing==0 ) {
#line 1088 "ada.g"
			Set(s_AST, NULL_STATEMENT);
#line 10197 "AdaParser.cpp"
		}
		null_stmt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_89);
		} else {
			throw;
		}
	}
	returnAST = null_stmt_AST;
}

void AdaParser::exit_stmt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST exit_stmt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  s = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		s = LT(1);
		if ( inputState->guessing == 0 ) {
			s_AST = astFactory->create(s);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(s_AST));
		}
		match(EXIT);
		{
		switch ( LA(1)) {
		case IDENTIFIER:
		{
			label_name();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case SEMI:
		case WHEN:
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
		case WHEN:
		{
			RefAdaAST tmp282_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp282_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp282_AST));
			}
			match(WHEN);
			condition();
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
		if ( inputState->guessing==0 ) {
#line 1181 "ada.g"
			Set(s_AST, EXIT_STATEMENT);
#line 10277 "AdaParser.cpp"
		}
		exit_stmt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_89);
		} else {
			throw;
		}
	}
	returnAST = exit_stmt_AST;
}

void AdaParser::return_stmt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST return_stmt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  s = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		s = LT(1);
		if ( inputState->guessing == 0 ) {
			s_AST = astFactory->create(s);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(s_AST));
		}
		match(RETURN);
		{
		switch ( LA(1)) {
		case IDENTIFIER:
		case LPAREN:
		case NEW:
		case CHARACTER_LITERAL:
		case CHAR_STRING:
		case NuLL:
		case NOT:
		case PLUS:
		case MINUS:
		case ABS:
		case NUMERIC_LIT:
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
		if ( inputState->guessing==0 ) {
#line 1188 "ada.g"
			Set(s_AST, RETURN_STATEMENT);
#line 10340 "AdaParser.cpp"
		}
		return_stmt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_89);
		} else {
			throw;
		}
	}
	returnAST = return_stmt_AST;
}

void AdaParser::goto_stmt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST goto_stmt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  s = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		s = LT(1);
		if ( inputState->guessing == 0 ) {
			s_AST = astFactory->create(s);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(s_AST));
		}
		match(GOTO);
		label_name();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(SEMI);
		if ( inputState->guessing==0 ) {
#line 1192 "ada.g"
			Set(s_AST, GOTO_STATEMENT);
#line 10377 "AdaParser.cpp"
		}
		goto_stmt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_89);
		} else {
			throw;
		}
	}
	returnAST = goto_stmt_AST;
}

void AdaParser::delay_stmt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST delay_stmt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  d = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST d_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		d = LT(1);
		if ( inputState->guessing == 0 ) {
			d_AST = astFactory->create(d);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(d_AST));
		}
		match(DELAY);
		until_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(SEMI);
		if ( inputState->guessing==0 ) {
#line 1257 "ada.g"
			Set (d_AST, DELAY_STATEMENT);
#line 10418 "AdaParser.cpp"
		}
		delay_stmt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_89);
		} else {
			throw;
		}
	}
	returnAST = delay_stmt_AST;
}

void AdaParser::abort_stmt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST abort_stmt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  a = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST a_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		a = LT(1);
		if ( inputState->guessing == 0 ) {
			a_AST = astFactory->create(a);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(a_AST));
		}
		match(ABORT);
		name();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				name();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop420;
			}
			
		}
		_loop420:;
		} // ( ... )*
		match(SEMI);
		if ( inputState->guessing==0 ) {
#line 1336 "ada.g"
			Set (a_AST, ABORT_STATEMENT);
#line 10471 "AdaParser.cpp"
		}
		abort_stmt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_89);
		} else {
			throw;
		}
	}
	returnAST = abort_stmt_AST;
}

void AdaParser::raise_stmt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST raise_stmt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  r = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST r_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		r = LT(1);
		if ( inputState->guessing == 0 ) {
			r_AST = astFactory->create(r);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(r_AST));
		}
		match(RAISE);
		{
		switch ( LA(1)) {
		case IDENTIFIER:
		{
			compound_name();
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
		if ( inputState->guessing==0 ) {
#line 1364 "ada.g"
			Set (r_AST, RAISE_STATEMENT);
#line 10524 "AdaParser.cpp"
		}
		raise_stmt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_89);
		} else {
			throw;
		}
	}
	returnAST = raise_stmt_AST;
}

void AdaParser::requeue_stmt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST requeue_stmt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  r = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST r_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		r = LT(1);
		if ( inputState->guessing == 0 ) {
			r_AST = astFactory->create(r);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(r_AST));
		}
		match(REQUEUE);
		name();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case WITH:
		{
			match(WITH);
			RefAdaAST tmp291_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp291_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp291_AST));
			}
			match(ABORT);
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
		if ( inputState->guessing==0 ) {
#line 1368 "ada.g"
			Set (r_AST, REQUEUE_STATEMENT);
#line 10584 "AdaParser.cpp"
		}
		requeue_stmt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_89);
		} else {
			throw;
		}
	}
	returnAST = requeue_stmt_AST;
}

void AdaParser::accept_stmt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST accept_stmt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  a = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST a_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		a = LT(1);
		if ( inputState->guessing == 0 ) {
			a_AST = astFactory->create(a);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(a_AST));
		}
		match(ACCEPT);
		def_id(false);
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		entry_index_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		formal_part_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case DO:
		{
			match(DO);
			handled_stmt_s();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			end_id_opt();
			match(SEMI);
			break;
		}
		case SEMI:
		{
			match(SEMI);
			if ( inputState->guessing==0 ) {
#line 1239 "ada.g"
				pop_def_id();
#line 10644 "AdaParser.cpp"
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		if ( inputState->guessing==0 ) {
#line 1241 "ada.g"
			Set (a_AST, ACCEPT_STATEMENT);
#line 10657 "AdaParser.cpp"
		}
		accept_stmt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_89);
		} else {
			throw;
		}
	}
	returnAST = accept_stmt_AST;
}

void AdaParser::select_stmt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST select_stmt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  s = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		s = LT(1);
		if ( inputState->guessing == 0 ) {
			s_AST = astFactory->create(s);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(s_AST));
		}
		match(SELECT);
		{
		bool synPredMatched398 = false;
		if (((LA(1) == IDENTIFIER || LA(1) == DELAY) && (_tokenSet_91.member(LA(2))))) {
			int _m398 = mark();
			synPredMatched398 = true;
			inputState->guessing++;
			try {
				{
				triggering_alternative();
				match(THEN);
				match(ABORT);
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched398 = false;
			}
			rewind(_m398);
			inputState->guessing--;
		}
		if ( synPredMatched398 ) {
			triggering_alternative();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(THEN);
			match(ABORT);
			abortable_part();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			if ( inputState->guessing==0 ) {
#line 1272 "ada.g"
				Set (s_AST, ASYNCHRONOUS_SELECT);
#line 10719 "AdaParser.cpp"
			}
		}
		else if ((_tokenSet_92.member(LA(1))) && (_tokenSet_93.member(LA(2)))) {
			selective_accept();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			if ( inputState->guessing==0 ) {
#line 1274 "ada.g"
				Set (s_AST, SELECTIVE_ACCEPT);
#line 10730 "AdaParser.cpp"
			}
		}
		else if ((LA(1) == IDENTIFIER) && (_tokenSet_94.member(LA(2)))) {
			entry_call_alternative();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{
			switch ( LA(1)) {
			case OR:
			{
				match(OR);
				delay_alternative();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				if ( inputState->guessing==0 ) {
#line 1276 "ada.g"
					Set (s_AST, TIMED_ENTRY_CALL);
#line 10750 "AdaParser.cpp"
				}
				break;
			}
			case ELSE:
			{
				match(ELSE);
				statements();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				if ( inputState->guessing==0 ) {
#line 1277 "ada.g"
					Set (s_AST, CONDITIONAL_ENTRY_CALL);
#line 10764 "AdaParser.cpp"
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
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
		match(END);
		match(SELECT);
		match(SEMI);
		select_stmt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_89);
		} else {
			throw;
		}
	}
	returnAST = select_stmt_AST;
}

void AdaParser::if_stmt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST if_stmt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  s = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		s = LT(1);
		if ( inputState->guessing == 0 ) {
			s_AST = astFactory->create(s);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(s_AST));
		}
		match(IF);
		cond_clause();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		elsifs_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		else_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(END);
		match(IF);
		match(SEMI);
		if ( inputState->guessing==0 ) {
#line 1094 "ada.g"
			Set(s_AST, IF_STATEMENT);
#line 10828 "AdaParser.cpp"
		}
		if_stmt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_89);
		} else {
			throw;
		}
	}
	returnAST = if_stmt_AST;
}

void AdaParser::case_stmt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST case_stmt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  s = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		s = LT(1);
		if ( inputState->guessing == 0 ) {
			s_AST = astFactory->create(s);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(s_AST));
		}
		match(CASE);
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(IS);
		alternative_s();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(END);
		match(CASE);
		match(SEMI);
		if ( inputState->guessing==0 ) {
#line 1114 "ada.g"
			Set(s_AST, CASE_STATEMENT);
#line 10872 "AdaParser.cpp"
		}
		case_stmt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_89);
		} else {
			throw;
		}
	}
	returnAST = case_stmt_AST;
}

void AdaParser::loop_stmt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST loop_stmt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		iteration_scheme_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(LOOP);
		statements();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(END);
		match(LOOP);
		if ( inputState->guessing==0 ) {
			loop_stmt_AST = RefAdaAST(currentAST.root);
#line 1126 "ada.g"
			loop_stmt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(LOOP_STATEMENT,"LOOP_STATEMENT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(loop_stmt_AST))));
#line 10908 "AdaParser.cpp"
			currentAST.root = loop_stmt_AST;
			if ( loop_stmt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				loop_stmt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = loop_stmt_AST->getFirstChild();
			else
				currentAST.child = loop_stmt_AST;
			currentAST.advanceChildToEnd();
		}
		loop_stmt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_95);
		} else {
			throw;
		}
	}
	returnAST = loop_stmt_AST;
}

void AdaParser::block() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST block_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		declare_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		block_body();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if ( inputState->guessing==0 ) {
			block_AST = RefAdaAST(currentAST.root);
#line 1173 "ada.g"
			block_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(BLOCK_STATEMENT,"BLOCK_STATEMENT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(block_AST))));
#line 10948 "AdaParser.cpp"
			currentAST.root = block_AST;
			if ( block_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				block_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = block_AST->getFirstChild();
			else
				currentAST.child = block_AST;
			currentAST.advanceChildToEnd();
		}
		block_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_21);
		} else {
			throw;
		}
	}
	returnAST = block_AST;
}

void AdaParser::statement_identifier() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST statement_identifier_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  n = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST n_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		n = LT(1);
		if ( inputState->guessing == 0 ) {
			n_AST = astFactory->create(n);
		}
		match(IDENTIFIER);
		match(COLON);
		if ( inputState->guessing==0 ) {
#line 1161 "ada.g"
			push_def_id(n_AST);
#line 10987 "AdaParser.cpp"
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_96);
		} else {
			throw;
		}
	}
	returnAST = statement_identifier_AST;
}

void AdaParser::id_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST id_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefAdaAST n_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
#line 1141 "ada.g"
	RefAdaAST endid;
#line 11008 "AdaParser.cpp"
	
	try {      // for error handling
		switch ( LA(1)) {
		case CHAR_STRING:
		{
			endid=definable_operator_symbol();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			if (!( end_id_matches_def_id (endid) ))
				throw ANTLR_USE_NAMESPACE(antlr)SemanticException(" end_id_matches_def_id (endid) ");
			id_opt_AST = RefAdaAST(currentAST.root);
			break;
		}
		case IDENTIFIER:
		{
			compound_name();
			if (inputState->guessing==0) {
				n_AST = returnAST;
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			if (!( end_id_matches_def_id (n_AST) ))
				throw ANTLR_USE_NAMESPACE(antlr)SemanticException(" end_id_matches_def_id (n_AST) ");
			id_opt_AST = RefAdaAST(currentAST.root);
			break;
		}
		case SEMI:
		{
			if ( inputState->guessing==0 ) {
#line 1148 "ada.g"
				pop_def_id();
#line 11040 "AdaParser.cpp"
			}
			id_opt_AST = RefAdaAST(currentAST.root);
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
			recover(ex,_tokenSet_4);
		} else {
			throw;
		}
	}
	returnAST = id_opt_AST;
}

void AdaParser::call_or_assignment() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST call_or_assignment_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		name();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case ASSIGN:
		{
			match(ASSIGN);
			expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			if ( inputState->guessing==0 ) {
				call_or_assignment_AST = RefAdaAST(currentAST.root);
#line 1197 "ada.g"
				call_or_assignment_AST =
							RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(ASSIGNMENT_STATEMENT,"ASSIGNMENT_STATEMENT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(call_or_assignment_AST))));
#line 11086 "AdaParser.cpp"
				currentAST.root = call_or_assignment_AST;
				if ( call_or_assignment_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
					call_or_assignment_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
					  currentAST.child = call_or_assignment_AST->getFirstChild();
				else
					currentAST.child = call_or_assignment_AST;
				currentAST.advanceChildToEnd();
			}
			break;
		}
		case SEMI:
		{
			if ( inputState->guessing==0 ) {
				call_or_assignment_AST = RefAdaAST(currentAST.root);
#line 1200 "ada.g"
				call_or_assignment_AST =
							RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(CALL_STATEMENT,"CALL_STATEMENT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(call_or_assignment_AST))));
#line 11104 "AdaParser.cpp"
				currentAST.root = call_or_assignment_AST;
				if ( call_or_assignment_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
					call_or_assignment_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
					  currentAST.child = call_or_assignment_AST->getFirstChild();
				else
					currentAST.child = call_or_assignment_AST;
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
		match(SEMI);
		call_or_assignment_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_89);
		} else {
			throw;
		}
	}
	returnAST = call_or_assignment_AST;
}

void AdaParser::cond_clause() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST cond_clause_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  c = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST c_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		condition();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		c = LT(1);
		if ( inputState->guessing == 0 ) {
			c_AST = astFactory->create(c);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(c_AST));
		}
		match(THEN);
		statements();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if ( inputState->guessing==0 ) {
#line 1098 "ada.g"
			Set(c_AST, COND_CLAUSE);
#line 11160 "AdaParser.cpp"
		}
		cond_clause_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_97);
		} else {
			throw;
		}
	}
	returnAST = cond_clause_AST;
}

void AdaParser::elsifs_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST elsifs_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == ELSIF)) {
				match(ELSIF);
				cond_clause();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop350;
			}
			
		}
		_loop350:;
		} // ( ... )*
		if ( inputState->guessing==0 ) {
			elsifs_opt_AST = RefAdaAST(currentAST.root);
#line 1106 "ada.g"
			elsifs_opt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(ELSIFS_OPT,"ELSIFS_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(elsifs_opt_AST))));
#line 11201 "AdaParser.cpp"
			currentAST.root = elsifs_opt_AST;
			if ( elsifs_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				elsifs_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = elsifs_opt_AST->getFirstChild();
			else
				currentAST.child = elsifs_opt_AST;
			currentAST.advanceChildToEnd();
		}
		elsifs_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_98);
		} else {
			throw;
		}
	}
	returnAST = elsifs_opt_AST;
}

void AdaParser::else_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST else_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
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
		if ( inputState->guessing==0 ) {
			else_opt_AST = RefAdaAST(currentAST.root);
#line 1110 "ada.g"
			else_opt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(ELSE_OPT,"ELSE_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(else_opt_AST))));
#line 11254 "AdaParser.cpp"
			currentAST.root = else_opt_AST;
			if ( else_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				else_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = else_opt_AST->getFirstChild();
			else
				currentAST.child = else_opt_AST;
			currentAST.advanceChildToEnd();
		}
		else_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_21);
		} else {
			throw;
		}
	}
	returnAST = else_opt_AST;
}

void AdaParser::condition() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST condition_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		condition_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_99);
		} else {
			throw;
		}
	}
	returnAST = condition_AST;
}

void AdaParser::alternative_s() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST alternative_s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{ // ( ... )+
		int _cnt356=0;
		for (;;) {
			if ((LA(1) == WHEN)) {
				case_statement_alternative();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				if ( _cnt356>=1 ) { goto _loop356; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
			}
			
			_cnt356++;
		}
		_loop356:;
		}  // ( ... )+
		alternative_s_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_21);
		} else {
			throw;
		}
	}
	returnAST = alternative_s_AST;
}

void AdaParser::case_statement_alternative() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST case_statement_alternative_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  s = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		s = LT(1);
		if ( inputState->guessing == 0 ) {
			s_AST = astFactory->create(s);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(s_AST));
		}
		match(WHEN);
		choice_s();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(RIGHT_SHAFT);
		statements();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if ( inputState->guessing==0 ) {
#line 1121 "ada.g"
			Set(s_AST, CASE_STATEMENT_ALTERNATIVE);
#line 11361 "AdaParser.cpp"
		}
		case_statement_alternative_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_72);
		} else {
			throw;
		}
	}
	returnAST = case_statement_alternative_AST;
}

void AdaParser::iteration_scheme_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST iteration_scheme_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case WHILE:
		{
			RefAdaAST tmp319_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp319_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp319_AST));
			}
			match(WHILE);
			condition();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case FOR:
		{
			RefAdaAST tmp320_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp320_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp320_AST));
			}
			match(FOR);
			RefAdaAST tmp321_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp321_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp321_AST));
			}
			match(IDENTIFIER);
			match(IN);
			reverse_opt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			discrete_subtype_definition();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case LOOP:
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
			iteration_scheme_opt_AST = RefAdaAST(currentAST.root);
#line 1132 "ada.g"
			iteration_scheme_opt_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(ITERATION_SCHEME_OPT,"ITERATION_SCHEME_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(iteration_scheme_opt_AST))));
#line 11438 "AdaParser.cpp"
			currentAST.root = iteration_scheme_opt_AST;
			if ( iteration_scheme_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				iteration_scheme_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = iteration_scheme_opt_AST->getFirstChild();
			else
				currentAST.child = iteration_scheme_opt_AST;
			currentAST.advanceChildToEnd();
		}
		iteration_scheme_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_100);
		} else {
			throw;
		}
	}
	returnAST = iteration_scheme_opt_AST;
}

void AdaParser::reverse_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST reverse_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case REVERSE:
		{
			RefAdaAST tmp323_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp323_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp323_AST));
			}
			match(REVERSE);
			break;
		}
		case IDENTIFIER:
		case LPAREN:
		case NEW:
		case CHARACTER_LITERAL:
		case CHAR_STRING:
		case NuLL:
		case NOT:
		case PLUS:
		case MINUS:
		case ABS:
		case NUMERIC_LIT:
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
			reverse_opt_AST = RefAdaAST(currentAST.root);
#line 1138 "ada.g"
			reverse_opt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(MODIFIERS,"MODIFIERS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(reverse_opt_AST))));
#line 11502 "AdaParser.cpp"
			currentAST.root = reverse_opt_AST;
			if ( reverse_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				reverse_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = reverse_opt_AST->getFirstChild();
			else
				currentAST.child = reverse_opt_AST;
			currentAST.advanceChildToEnd();
		}
		reverse_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_5);
		} else {
			throw;
		}
	}
	returnAST = reverse_opt_AST;
}

void AdaParser::declare_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST declare_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case DECLARE:
		{
			match(DECLARE);
			declarative_part();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
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
		if ( inputState->guessing==0 ) {
			declare_opt_AST = RefAdaAST(currentAST.root);
#line 1177 "ada.g"
			declare_opt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(DECLARE_OPT,"DECLARE_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(declare_opt_AST))));
#line 11555 "AdaParser.cpp"
			currentAST.root = declare_opt_AST;
			if ( declare_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				declare_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = declare_opt_AST->getFirstChild();
			else
				currentAST.child = declare_opt_AST;
			currentAST.advanceChildToEnd();
		}
		declare_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_101);
		} else {
			throw;
		}
	}
	returnAST = declare_opt_AST;
}

void AdaParser::label_name() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST label_name_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		RefAdaAST tmp325_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp325_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp325_AST));
		}
		match(IDENTIFIER);
		label_name_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_102);
		} else {
			throw;
		}
	}
	returnAST = label_name_AST;
}

void AdaParser::entry_body_formal_part() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST entry_body_formal_part_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		entry_index_spec_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		formal_part_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		entry_body_formal_part_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_103);
		} else {
			throw;
		}
	}
	returnAST = entry_body_formal_part_AST;
}

void AdaParser::entry_barrier() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST entry_barrier_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(WHEN);
		condition();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		entry_barrier_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_75);
		} else {
			throw;
		}
	}
	returnAST = entry_barrier_AST;
}

void AdaParser::entry_index_spec_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST entry_index_spec_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		bool synPredMatched383 = false;
		if (((LA(1) == LPAREN) && (LA(2) == FOR))) {
			int _m383 = mark();
			synPredMatched383 = true;
			inputState->guessing++;
			try {
				{
				match(LPAREN);
				match(FOR);
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched383 = false;
			}
			rewind(_m383);
			inputState->guessing--;
		}
		if ( synPredMatched383 ) {
			match(LPAREN);
			match(FOR);
			def_id(false);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(IN);
			discrete_subtype_definition();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(RPAREN);
		}
		else if ((LA(1) == LPAREN || LA(1) == WHEN) && (_tokenSet_5.member(LA(2)))) {
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
		if ( inputState->guessing==0 ) {
			entry_index_spec_opt_AST = RefAdaAST(currentAST.root);
#line 1222 "ada.g"
			entry_index_spec_opt_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(ENTRY_INDEX_SPECIFICATION,"ENTRY_INDEX_SPECIFICATION")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(entry_index_spec_opt_AST))));
#line 11703 "AdaParser.cpp"
			currentAST.root = entry_index_spec_opt_AST;
			if ( entry_index_spec_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				entry_index_spec_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = entry_index_spec_opt_AST->getFirstChild();
			else
				currentAST.child = entry_index_spec_opt_AST;
			currentAST.advanceChildToEnd();
		}
		entry_index_spec_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_104);
		} else {
			throw;
		}
	}
	returnAST = entry_index_spec_opt_AST;
}

void AdaParser::entry_call_stmt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST entry_call_stmt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		name();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(SEMI);
		if ( inputState->guessing==0 ) {
			entry_call_stmt_AST = RefAdaAST(currentAST.root);
#line 1232 "ada.g"
			entry_call_stmt_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(ENTRY_CALL_STATEMENT,"ENTRY_CALL_STATEMENT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(entry_call_stmt_AST))));
#line 11741 "AdaParser.cpp"
			currentAST.root = entry_call_stmt_AST;
			if ( entry_call_stmt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				entry_call_stmt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = entry_call_stmt_AST->getFirstChild();
			else
				currentAST.child = entry_call_stmt_AST;
			currentAST.advanceChildToEnd();
		}
		entry_call_stmt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_105);
		} else {
			throw;
		}
	}
	returnAST = entry_call_stmt_AST;
}

void AdaParser::entry_index_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST entry_index_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		bool synPredMatched391 = false;
		if (((LA(1) == LPAREN) && (_tokenSet_5.member(LA(2))))) {
			int _m391 = mark();
			synPredMatched391 = true;
			inputState->guessing++;
			try {
				{
				match(LPAREN);
				expression();
				match(RPAREN);
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched391 = false;
			}
			rewind(_m391);
			inputState->guessing--;
		}
		if ( synPredMatched391 ) {
			match(LPAREN);
			expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(RPAREN);
		}
		else if ((LA(1) == SEMI || LA(1) == LPAREN || LA(1) == DO) && (_tokenSet_89.member(LA(2)))) {
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
		if ( inputState->guessing==0 ) {
			entry_index_opt_AST = RefAdaAST(currentAST.root);
#line 1251 "ada.g"
			entry_index_opt_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(ENTRY_INDEX_OPT,"ENTRY_INDEX_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(entry_index_opt_AST))));
#line 11808 "AdaParser.cpp"
			currentAST.root = entry_index_opt_AST;
			if ( entry_index_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				entry_index_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = entry_index_opt_AST->getFirstChild();
			else
				currentAST.child = entry_index_opt_AST;
			currentAST.advanceChildToEnd();
		}
		entry_index_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_106);
		} else {
			throw;
		}
	}
	returnAST = entry_index_opt_AST;
}

void AdaParser::until_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST until_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case UNTIL:
		{
			RefAdaAST tmp334_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp334_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp334_AST));
			}
			match(UNTIL);
			break;
		}
		case IDENTIFIER:
		case LPAREN:
		case NEW:
		case CHARACTER_LITERAL:
		case CHAR_STRING:
		case NuLL:
		case NOT:
		case PLUS:
		case MINUS:
		case ABS:
		case NUMERIC_LIT:
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
			until_opt_AST = RefAdaAST(currentAST.root);
#line 1261 "ada.g"
			until_opt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(MODIFIERS,"MODIFIERS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(until_opt_AST))));
#line 11872 "AdaParser.cpp"
			currentAST.root = until_opt_AST;
			if ( until_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				until_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = until_opt_AST->getFirstChild();
			else
				currentAST.child = until_opt_AST;
			currentAST.advanceChildToEnd();
		}
		until_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_5);
		} else {
			throw;
		}
	}
	returnAST = until_opt_AST;
}

void AdaParser::triggering_alternative() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST triggering_alternative_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case DELAY:
		{
			delay_stmt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case IDENTIFIER:
		{
			entry_call_stmt();
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
		stmts_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if ( inputState->guessing==0 ) {
			triggering_alternative_AST = RefAdaAST(currentAST.root);
#line 1285 "ada.g"
			triggering_alternative_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(TRIGGERING_ALTERNATIVE,"TRIGGERING_ALTERNATIVE")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(triggering_alternative_AST))));
#line 11933 "AdaParser.cpp"
			currentAST.root = triggering_alternative_AST;
			if ( triggering_alternative_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				triggering_alternative_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = triggering_alternative_AST->getFirstChild();
			else
				currentAST.child = triggering_alternative_AST;
			currentAST.advanceChildToEnd();
		}
		triggering_alternative_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_107);
		} else {
			throw;
		}
	}
	returnAST = triggering_alternative_AST;
}

void AdaParser::abortable_part() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST abortable_part_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		stmts_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if ( inputState->guessing==0 ) {
			abortable_part_AST = RefAdaAST(currentAST.root);
#line 1291 "ada.g"
			abortable_part_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(ABORTABLE_PART,"ABORTABLE_PART")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(abortable_part_AST))));
#line 11970 "AdaParser.cpp"
			currentAST.root = abortable_part_AST;
			if ( abortable_part_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				abortable_part_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = abortable_part_AST->getFirstChild();
			else
				currentAST.child = abortable_part_AST;
			currentAST.advanceChildToEnd();
		}
		abortable_part_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_21);
		} else {
			throw;
		}
	}
	returnAST = abortable_part_AST;
}

void AdaParser::selective_accept() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST selective_accept_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		guard_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		select_alternative();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		or_select_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		else_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		selective_accept_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_21);
		} else {
			throw;
		}
	}
	returnAST = selective_accept_AST;
}

void AdaParser::entry_call_alternative() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST entry_call_alternative_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		entry_call_stmt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		stmts_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if ( inputState->guessing==0 ) {
			entry_call_alternative_AST = RefAdaAST(currentAST.root);
#line 1297 "ada.g"
			entry_call_alternative_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(ENTRY_CALL_ALTERNATIVE,"ENTRY_CALL_ALTERNATIVE")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(entry_call_alternative_AST))));
#line 12046 "AdaParser.cpp"
			currentAST.root = entry_call_alternative_AST;
			if ( entry_call_alternative_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				entry_call_alternative_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = entry_call_alternative_AST->getFirstChild();
			else
				currentAST.child = entry_call_alternative_AST;
			currentAST.advanceChildToEnd();
		}
		entry_call_alternative_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_108);
		} else {
			throw;
		}
	}
	returnAST = entry_call_alternative_AST;
}

void AdaParser::delay_alternative() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST delay_alternative_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		delay_stmt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		stmts_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if ( inputState->guessing==0 ) {
			delay_alternative_AST = RefAdaAST(currentAST.root);
#line 1322 "ada.g"
			delay_alternative_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(DELAY_ALTERNATIVE,"DELAY_ALTERNATIVE")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(delay_alternative_AST))));
#line 12087 "AdaParser.cpp"
			currentAST.root = delay_alternative_AST;
			if ( delay_alternative_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				delay_alternative_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = delay_alternative_AST->getFirstChild();
			else
				currentAST.child = delay_alternative_AST;
			currentAST.advanceChildToEnd();
		}
		delay_alternative_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_109);
		} else {
			throw;
		}
	}
	returnAST = delay_alternative_AST;
}

void AdaParser::stmts_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST stmts_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{ // ( ... )*
		for (;;) {
			switch ( LA(1)) {
			case PRAGMA:
			{
				pragma();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case IDENTIFIER:
			case NuLL:
			case RETURN:
			case FOR:
			case CASE:
			case BEGIN:
			case LT_LT:
			case IF:
			case LOOP:
			case WHILE:
			case DECLARE:
			case EXIT:
			case GOTO:
			case ACCEPT:
			case DELAY:
			case SELECT:
			case ABORT:
			case RAISE:
			case REQUEUE:
			{
				statement();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			default:
			{
				goto _loop414;
			}
			}
		}
		_loop414:;
		} // ( ... )*
		stmts_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_110);
		} else {
			throw;
		}
	}
	returnAST = stmts_opt_AST;
}

void AdaParser::guard_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST guard_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case WHEN:
		{
			match(WHEN);
			condition();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(RIGHT_SHAFT);
			{ // ( ... )*
			for (;;) {
				if ((LA(1) == PRAGMA)) {
					pragma();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
				}
				else {
					goto _loop408;
				}
				
			}
			_loop408:;
			} // ( ... )*
			break;
		}
		case ACCEPT:
		case DELAY:
		case TERMINATE:
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
			guard_opt_AST = RefAdaAST(currentAST.root);
#line 1306 "ada.g"
			guard_opt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(GUARD_OPT,"GUARD_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(guard_opt_AST))));
#line 12222 "AdaParser.cpp"
			currentAST.root = guard_opt_AST;
			if ( guard_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				guard_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = guard_opt_AST->getFirstChild();
			else
				currentAST.child = guard_opt_AST;
			currentAST.advanceChildToEnd();
		}
		guard_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_111);
		} else {
			throw;
		}
	}
	returnAST = guard_opt_AST;
}

void AdaParser::select_alternative() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST select_alternative_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  t = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST t_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case ACCEPT:
		{
			accept_alternative();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			select_alternative_AST = RefAdaAST(currentAST.root);
			break;
		}
		case DELAY:
		{
			delay_alternative();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			select_alternative_AST = RefAdaAST(currentAST.root);
			break;
		}
		case TERMINATE:
		{
			t = LT(1);
			if ( inputState->guessing == 0 ) {
				t_AST = astFactory->create(t);
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(t_AST));
			}
			match(TERMINATE);
			match(SEMI);
			if ( inputState->guessing==0 ) {
#line 1312 "ada.g"
				Set(t_AST, TERMINATE_ALTERNATIVE);
#line 12283 "AdaParser.cpp"
			}
			select_alternative_AST = RefAdaAST(currentAST.root);
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
			recover(ex,_tokenSet_109);
		} else {
			throw;
		}
	}
	returnAST = select_alternative_AST;
}

void AdaParser::or_select_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST or_select_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == OR)) {
				match(OR);
				guard_opt();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				select_alternative();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop417;
			}
			
		}
		_loop417:;
		} // ( ... )*
		if ( inputState->guessing==0 ) {
			or_select_opt_AST = RefAdaAST(currentAST.root);
#line 1331 "ada.g"
			or_select_opt_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(OR_SELECT_OPT,"OR_SELECT_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(or_select_opt_AST))));
#line 12336 "AdaParser.cpp"
			currentAST.root = or_select_opt_AST;
			if ( or_select_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				or_select_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = or_select_opt_AST->getFirstChild();
			else
				currentAST.child = or_select_opt_AST;
			currentAST.advanceChildToEnd();
		}
		or_select_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_98);
		} else {
			throw;
		}
	}
	returnAST = or_select_opt_AST;
}

void AdaParser::accept_alternative() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST accept_alternative_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		accept_stmt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		stmts_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if ( inputState->guessing==0 ) {
			accept_alternative_AST = RefAdaAST(currentAST.root);
#line 1316 "ada.g"
			accept_alternative_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(ACCEPT_ALTERNATIVE,"ACCEPT_ALTERNATIVE")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(accept_alternative_AST))));
#line 12377 "AdaParser.cpp"
			currentAST.root = accept_alternative_AST;
			if ( accept_alternative_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				accept_alternative_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = accept_alternative_AST->getFirstChild();
			else
				currentAST.child = accept_alternative_AST;
			currentAST.advanceChildToEnd();
		}
		accept_alternative_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_109);
		} else {
			throw;
		}
	}
	returnAST = accept_alternative_AST;
}

void AdaParser::exception_handler() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST exception_handler_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  w = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST w_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		w = LT(1);
		if ( inputState->guessing == 0 ) {
			w_AST = astFactory->create(w);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(w_AST));
		}
		match(WHEN);
		identifier_colon_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		except_choice_s();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(RIGHT_SHAFT);
		statements();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if ( inputState->guessing==0 ) {
#line 1347 "ada.g"
			Set (w_AST, EXCEPTION_HANDLER);
#line 12429 "AdaParser.cpp"
		}
		exception_handler_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_72);
		} else {
			throw;
		}
	}
	returnAST = exception_handler_AST;
}

void AdaParser::identifier_colon_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST identifier_colon_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		if ((LA(1) == IDENTIFIER) && (LA(2) == COLON)) {
			RefAdaAST tmp340_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp340_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp340_AST));
			}
			match(IDENTIFIER);
			match(COLON);
		}
		else if ((LA(1) == IDENTIFIER || LA(1) == OTHERS) && (LA(2) == RIGHT_SHAFT || LA(2) == DOT || LA(2) == PIPE)) {
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
		if ( inputState->guessing==0 ) {
			identifier_colon_opt_AST = RefAdaAST(currentAST.root);
#line 1351 "ada.g"
			identifier_colon_opt_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(IDENTIFIER_COLON_OPT,"IDENTIFIER_COLON_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(identifier_colon_opt_AST))));
#line 12472 "AdaParser.cpp"
			currentAST.root = identifier_colon_opt_AST;
			if ( identifier_colon_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
				identifier_colon_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
				  currentAST.child = identifier_colon_opt_AST->getFirstChild();
			else
				currentAST.child = identifier_colon_opt_AST;
			currentAST.advanceChildToEnd();
		}
		identifier_colon_opt_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_112);
		} else {
			throw;
		}
	}
	returnAST = identifier_colon_opt_AST;
}

void AdaParser::except_choice_s() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST except_choice_s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		exception_choice();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == PIPE)) {
				RefAdaAST tmp342_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp342_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp342_AST));
				}
				match(PIPE);
				exception_choice();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				goto _loop430;
			}
			
		}
		_loop430:;
		} // ( ... )*
		except_choice_s_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_76);
		} else {
			throw;
		}
	}
	returnAST = except_choice_s_AST;
}

void AdaParser::exception_choice() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST exception_choice_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case IDENTIFIER:
		{
			compound_name();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			exception_choice_AST = RefAdaAST(currentAST.root);
			break;
		}
		case OTHERS:
		{
			RefAdaAST tmp343_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp343_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp343_AST));
			}
			match(OTHERS);
			exception_choice_AST = RefAdaAST(currentAST.root);
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
			recover(ex,_tokenSet_68);
		} else {
			throw;
		}
	}
	returnAST = exception_choice_AST;
}

void AdaParser::operator_call() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST operator_call_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  cs = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST cs_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		cs = LT(1);
		if ( inputState->guessing == 0 ) {
			cs_AST = astFactory->create(cs);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(cs_AST));
		}
		match(CHAR_STRING);
		operator_call_tail(cs_AST);
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		operator_call_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_0);
		} else {
			throw;
		}
	}
	returnAST = operator_call_AST;
}

void AdaParser::operator_call_tail(
	RefAdaAST opstr
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST operator_call_tail_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		match(LPAREN);
		if (!( is_operator_symbol(opstr->getText().c_str()) ))
			throw ANTLR_USE_NAMESPACE(antlr)SemanticException(" is_operator_symbol(opstr->getText().c_str()) ");
		value_s();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(RPAREN);
		if ( inputState->guessing==0 ) {
#line 1376 "ada.g"
			opstr->setType(OPERATOR_SYMBOL);
#line 12632 "AdaParser.cpp"
		}
		operator_call_tail_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_113);
		} else {
			throw;
		}
	}
	returnAST = operator_call_tail_AST;
}

void AdaParser::relation() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST relation_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  n = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST n_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		simple_expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{
		switch ( LA(1)) {
		case IN:
		{
			RefAdaAST tmp346_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp346_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp346_AST));
			}
			match(IN);
			range_or_mark();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case NOT:
		{
			n = LT(1);
			if ( inputState->guessing == 0 ) {
				n_AST = astFactory->create(n);
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(n_AST));
			}
			match(NOT);
			match(IN);
			range_or_mark();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			if ( inputState->guessing==0 ) {
#line 1400 "ada.g"
				Set (n_AST, NOT_IN);
#line 12691 "AdaParser.cpp"
			}
			break;
		}
		case EQ:
		{
			RefAdaAST tmp348_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp348_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp348_AST));
			}
			match(EQ);
			simple_expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case NE:
		{
			RefAdaAST tmp349_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp349_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp349_AST));
			}
			match(NE);
			simple_expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case LT_:
		{
			RefAdaAST tmp350_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp350_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp350_AST));
			}
			match(LT_);
			simple_expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case LE:
		{
			RefAdaAST tmp351_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp351_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp351_AST));
			}
			match(LE);
			simple_expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case GT:
		{
			RefAdaAST tmp352_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp352_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp352_AST));
			}
			match(GT);
			simple_expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case GE:
		{
			RefAdaAST tmp353_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp353_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp353_AST));
			}
			match(GE);
			simple_expression();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case SEMI:
		case COMMA:
		case RPAREN:
		case RIGHT_SHAFT:
		case WITH:
		case RANGE:
		case DIGITS:
		case IS:
		case PIPE:
		case DOT_DOT:
		case ASSIGN:
		case THEN:
		case LOOP:
		case OR:
		case AND:
		case XOR:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		relation_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_34);
		} else {
			throw;
		}
	}
	returnAST = relation_AST;
}

void AdaParser::range_or_mark() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST range_or_mark_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		bool synPredMatched450 = false;
		if (((_tokenSet_5.member(LA(1))) && (_tokenSet_33.member(LA(2))))) {
			int _m450 = mark();
			synPredMatched450 = true;
			inputState->guessing++;
			try {
				{
				range();
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched450 = false;
			}
			rewind(_m450);
			inputState->guessing--;
		}
		if ( synPredMatched450 ) {
			range();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			range_or_mark_AST = RefAdaAST(currentAST.root);
		}
		else if ((LA(1) == IDENTIFIER) && (_tokenSet_114.member(LA(2)))) {
			subtype_mark();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			range_or_mark_AST = RefAdaAST(currentAST.root);
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_34);
		} else {
			throw;
		}
	}
	returnAST = range_or_mark_AST;
}

void AdaParser::signed_term() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST signed_term_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  p = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST p_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  m = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST m_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case PLUS:
		{
			p = LT(1);
			if ( inputState->guessing == 0 ) {
				p_AST = astFactory->create(p);
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(p_AST));
			}
			match(PLUS);
			term();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			if ( inputState->guessing==0 ) {
#line 1422 "ada.g"
				Set(p_AST, UNARY_PLUS);
#line 12895 "AdaParser.cpp"
			}
			signed_term_AST = RefAdaAST(currentAST.root);
			break;
		}
		case MINUS:
		{
			m = LT(1);
			if ( inputState->guessing == 0 ) {
				m_AST = astFactory->create(m);
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(m_AST));
			}
			match(MINUS);
			term();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			if ( inputState->guessing==0 ) {
#line 1423 "ada.g"
				Set(m_AST, UNARY_MINUS);
#line 12915 "AdaParser.cpp"
			}
			signed_term_AST = RefAdaAST(currentAST.root);
			break;
		}
		case IDENTIFIER:
		case LPAREN:
		case NEW:
		case CHARACTER_LITERAL:
		case CHAR_STRING:
		case NuLL:
		case NOT:
		case ABS:
		case NUMERIC_LIT:
		{
			term();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			signed_term_AST = RefAdaAST(currentAST.root);
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
			recover(ex,_tokenSet_115);
		} else {
			throw;
		}
	}
	returnAST = signed_term_AST;
}

void AdaParser::term() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST term_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		factor();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		{ // ( ... )*
		for (;;) {
			switch ( LA(1)) {
			case STAR:
			{
				RefAdaAST tmp354_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp354_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp354_AST));
				}
				match(STAR);
				factor();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case DIV:
			{
				RefAdaAST tmp355_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp355_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp355_AST));
				}
				match(DIV);
				factor();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case MOD:
			{
				RefAdaAST tmp356_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp356_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp356_AST));
				}
				match(MOD);
				factor();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case REM:
			{
				RefAdaAST tmp357_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp357_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp357_AST));
				}
				match(REM);
				factor();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			default:
			{
				goto _loop457;
			}
			}
		}
		_loop457:;
		} // ( ... )*
		term_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_115);
		} else {
			throw;
		}
	}
	returnAST = term_AST;
}

void AdaParser::factor() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST factor_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case NOT:
		{
			RefAdaAST tmp358_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp358_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp358_AST));
			}
			match(NOT);
			primary();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case ABS:
		{
			RefAdaAST tmp359_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp359_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp359_AST));
			}
			match(ABS);
			primary();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case IDENTIFIER:
		case LPAREN:
		case NEW:
		case CHARACTER_LITERAL:
		case CHAR_STRING:
		case NuLL:
		case NUMERIC_LIT:
		{
			primary();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			{
			switch ( LA(1)) {
			case EXPON:
			{
				RefAdaAST tmp360_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp360_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp360_AST));
				}
				match(EXPON);
				primary();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case SEMI:
			case COMMA:
			case RPAREN:
			case RIGHT_SHAFT:
			case WITH:
			case RANGE:
			case DIGITS:
			case IS:
			case PIPE:
			case DOT_DOT:
			case IN:
			case ASSIGN:
			case MOD:
			case THEN:
			case LOOP:
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
		factor_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_116);
		} else {
			throw;
		}
	}
	returnAST = factor_AST;
}

void AdaParser::primary() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST primary_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  cs = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST cs_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case IDENTIFIER:
		{
			name_or_qualified();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case LPAREN:
		{
			parenthesized_primary();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case NEW:
		{
			allocator();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case NuLL:
		{
			RefAdaAST tmp361_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp361_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp361_AST));
			}
			match(NuLL);
			break;
		}
		case NUMERIC_LIT:
		{
			RefAdaAST tmp362_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp362_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp362_AST));
			}
			match(NUMERIC_LIT);
			break;
		}
		case CHARACTER_LITERAL:
		{
			RefAdaAST tmp363_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp363_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp363_AST));
			}
			match(CHARACTER_LITERAL);
			break;
		}
		case CHAR_STRING:
		{
			cs = LT(1);
			if ( inputState->guessing == 0 ) {
				cs_AST = astFactory->create(cs);
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(cs_AST));
			}
			match(CHAR_STRING);
			{
			switch ( LA(1)) {
			case LPAREN:
			{
				operator_call_tail(cs_AST);
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				break;
			}
			case SEMI:
			case COMMA:
			case RPAREN:
			case RIGHT_SHAFT:
			case WITH:
			case RANGE:
			case DIGITS:
			case IS:
			case PIPE:
			case DOT_DOT:
			case IN:
			case ASSIGN:
			case MOD:
			case THEN:
			case LOOP:
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
			case EXPON:
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
		primary_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_117);
		} else {
			throw;
		}
	}
	returnAST = primary_AST;
}

void AdaParser::name_or_qualified() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST name_or_qualified_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  p = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST p_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
#line 1454 "ada.g"
	RefAdaAST dummy;
#line 13321 "AdaParser.cpp"
	
	try {      // for error handling
		RefAdaAST tmp364_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp364_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp364_AST));
		}
		match(IDENTIFIER);
		{ // ( ... )*
		for (;;) {
			switch ( LA(1)) {
			case DOT:
			{
				RefAdaAST tmp365_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp365_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp365_AST));
				}
				match(DOT);
				{
				switch ( LA(1)) {
				case ALL:
				{
					RefAdaAST tmp366_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if ( inputState->guessing == 0 ) {
						tmp366_AST = astFactory->create(LT(1));
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp366_AST));
					}
					match(ALL);
					break;
				}
				case IDENTIFIER:
				{
					RefAdaAST tmp367_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if ( inputState->guessing == 0 ) {
						tmp367_AST = astFactory->create(LT(1));
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp367_AST));
					}
					match(IDENTIFIER);
					break;
				}
				case CHARACTER_LITERAL:
				{
					RefAdaAST tmp368_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
					if ( inputState->guessing == 0 ) {
						tmp368_AST = astFactory->create(LT(1));
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp368_AST));
					}
					match(CHARACTER_LITERAL);
					break;
				}
				case CHAR_STRING:
				{
					dummy=is_operator();
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
				break;
			}
			case LPAREN:
			{
				p = LT(1);
				if ( inputState->guessing == 0 ) {
					p_AST = astFactory->create(p);
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(p_AST));
				}
				match(LPAREN);
				value_s();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				match(RPAREN);
				if ( inputState->guessing==0 ) {
#line 1462 "ada.g"
					Set(p_AST, INDEXED_COMPONENT);
#line 13405 "AdaParser.cpp"
				}
				break;
			}
			case TIC:
			{
				RefAdaAST tmp370_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp370_AST = astFactory->create(LT(1));
					astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp370_AST));
				}
				match(TIC);
				{
				switch ( LA(1)) {
				case LPAREN:
				{
					parenthesized_primary();
					if (inputState->guessing==0) {
						astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
					}
					break;
				}
				case IDENTIFIER:
				case RANGE:
				case DIGITS:
				case DELTA:
				case ACCESS:
				{
					attribute_id();
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
				break;
			}
			default:
			{
				goto _loop468;
			}
			}
		}
		_loop468:;
		} // ( ... )*
		name_or_qualified_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_117);
		} else {
			throw;
		}
	}
	returnAST = name_or_qualified_AST;
}

void AdaParser::allocator() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST allocator_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  n = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST n_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		n = LT(1);
		if ( inputState->guessing == 0 ) {
			n_AST = astFactory->create(n);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(n_AST));
		}
		match(NEW);
		name_or_qualified();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if ( inputState->guessing==0 ) {
#line 1468 "ada.g"
			Set(n_AST, ALLOCATOR);
#line 13489 "AdaParser.cpp"
		}
		allocator_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_117);
		} else {
			throw;
		}
	}
	returnAST = allocator_AST;
}

void AdaParser::subprogram_body() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST subprogram_body_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  p = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST p_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  f = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST f_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		switch ( LA(1)) {
		case PROCEDURE:
		{
			p = LT(1);
			if ( inputState->guessing == 0 ) {
				p_AST = astFactory->create(p);
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(p_AST));
			}
			match(PROCEDURE);
			def_id(false);
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			formal_part_opt();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(IS);
			body_part();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(SEMI);
			if ( inputState->guessing==0 ) {
#line 1482 "ada.g"
				Set(p_AST, PROCEDURE_BODY);
#line 13540 "AdaParser.cpp"
			}
			subprogram_body_AST = RefAdaAST(currentAST.root);
			break;
		}
		case FUNCTION:
		{
			f = LT(1);
			if ( inputState->guessing == 0 ) {
				f_AST = astFactory->create(f);
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(f_AST));
			}
			match(FUNCTION);
			function_tail();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(IS);
			body_part();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			match(SEMI);
			if ( inputState->guessing==0 ) {
#line 1484 "ada.g"
				Set(f_AST, FUNCTION_BODY);
#line 13566 "AdaParser.cpp"
			}
			subprogram_body_AST = RefAdaAST(currentAST.root);
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
			recover(ex,_tokenSet_2);
		} else {
			throw;
		}
	}
	returnAST = subprogram_body_AST;
}

void AdaParser::package_body() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST package_body_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  p = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST p_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		p = LT(1);
		if ( inputState->guessing == 0 ) {
			p_AST = astFactory->create(p);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(p_AST));
		}
		match(PACKAGE);
		body_is();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		pkg_body_part();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		end_id_opt();
		match(SEMI);
		if ( inputState->guessing==0 ) {
#line 1488 "ada.g"
			Set(p_AST, PACKAGE_BODY);
#line 13615 "AdaParser.cpp"
		}
		package_body_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_2);
		} else {
			throw;
		}
	}
	returnAST = package_body_AST;
}

void AdaParser::task_body() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST task_body_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  t = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST t_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		t = LT(1);
		if ( inputState->guessing == 0 ) {
			t_AST = astFactory->create(t);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(t_AST));
		}
		match(TASK);
		body_is();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		body_part();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(SEMI);
		if ( inputState->guessing==0 ) {
#line 1492 "ada.g"
			Set(t_AST, TASK_BODY);
#line 13656 "AdaParser.cpp"
		}
		task_body_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_2);
		} else {
			throw;
		}
	}
	returnAST = task_body_AST;
}

void AdaParser::protected_body() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST protected_body_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  p = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST p_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	try {      // for error handling
		p = LT(1);
		if ( inputState->guessing == 0 ) {
			p_AST = astFactory->create(p);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(p_AST));
		}
		match(PROTECTED);
		body_is();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		prot_op_bodies_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		end_id_opt();
		match(SEMI);
		if ( inputState->guessing==0 ) {
#line 1496 "ada.g"
			Set(p_AST, PROTECTED_BODY);
#line 13698 "AdaParser.cpp"
		}
		protected_body_AST = RefAdaAST(currentAST.root);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_2);
		} else {
			throw;
		}
	}
	returnAST = protected_body_AST;
}

void AdaParser::initializeASTFactory( ANTLR_USE_NAMESPACE(antlr)ASTFactory& factory )
{
	factory.setMaxNodeType(303);
}
const char* AdaParser::tokenNames[] = {
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
	0
};

const unsigned long AdaParser::_tokenSet_0_data_[] = { 2UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_0(_tokenSet_0_data_,10);
const unsigned long AdaParser::_tokenSet_1_data_[] = { 53477376UL, 536871040UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "private" "package" "procedure" "function" "separate" "generic" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_1(_tokenSet_1_data_,10);
const unsigned long AdaParser::_tokenSet_2_data_[] = { 18UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF "pragma" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_2(_tokenSet_2_data_,10);
const unsigned long AdaParser::_tokenSet_3_data_[] = { 53504050UL, 3860450976UL, 2076542UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF "pragma" IDENTIFIER "with" "use" "type" "private" "package" "procedure" 
// "function" "null" "separate" "return" "task" "protected" "entry" "for" 
// "end" "subtype" "exception" "case" "when" "generic" "begin" LT_LT "if" 
// "then" "elsif" "else" "loop" "while" "declare" "exit" "goto" "accept" 
// "delay" "select" "abort" "or" "terminate" "raise" "requeue" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_3(_tokenSet_3_data_,10);
const unsigned long AdaParser::_tokenSet_4_data_[] = { 64UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_4(_tokenSet_4_data_,10);
const unsigned long AdaParser::_tokenSet_5_data_[] = { 67109024UL, 56UL, 3229614080UL, 80UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER LPAREN "new" CHARACTER_LITERAL CHAR_STRING "null" "not" PLUS 
// MINUS "abs" NUMERIC_LIT 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_5(_tokenSet_5_data_,10);
const unsigned long AdaParser::_tokenSet_6_data_[] = { 201364384UL, 262201UL, 4293001216UL, 127UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER LPAREN COMMA RPAREN DOT TIC "new" "others" "in" CHARACTER_LITERAL 
// CHAR_STRING "null" "mod" "or" "and" "xor" "not" EQ NE LT_ LE GT GE PLUS 
// MINUS CONCAT STAR DIV "rem" "abs" EXPON NUMERIC_LIT 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_6(_tokenSet_6_data_,10);
const unsigned long AdaParser::_tokenSet_7_data_[] = { 768UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// COMMA RPAREN 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_7(_tokenSet_7_data_,10);
const unsigned long AdaParser::_tokenSet_8_data_[] = { 813895488UL, 8192UL, 36UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI COMMA RPAREN RIGHT_SHAFT "with" "range" "digits" "is" PIPE DOT_DOT 
// ASSIGN "then" "loop" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_8(_tokenSet_8_data_,10);
const unsigned long AdaParser::_tokenSet_9_data_[] = { 53487632UL, 536871040UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "pragma" "with" "use" "private" "package" "procedure" "function" "separate" 
// "generic" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_9(_tokenSet_9_data_,10);
const unsigned long AdaParser::_tokenSet_10_data_[] = { 53504050UL, 1611238528UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF "pragma" IDENTIFIER "with" "use" "type" "private" "package" "procedure" 
// "function" "separate" "task" "protected" "for" "end" "subtype" "generic" 
// "begin" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_10(_tokenSet_10_data_,10);
const unsigned long AdaParser::_tokenSet_11_data_[] = { 814198720UL, 67248645UL, 6426660UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LPAREN COMMA RPAREN RIGHT_SHAFT "with" "use" TIC "range" "digits" 
// "delta" "is" PIPE DOT_DOT "in" "renames" "return" ASSIGN "at" "when" 
// "then" "loop" "do" "or" "and" "xor" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_11(_tokenSet_11_data_,10);
const unsigned long AdaParser::_tokenSet_12_data_[] = { 814165952UL, 139268UL, 6422564UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LPAREN COMMA RPAREN RIGHT_SHAFT "with" "use" "range" "digits" "delta" 
// "is" PIPE DOT_DOT "renames" ASSIGN "at" "then" "loop" "or" "and" "xor" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_12(_tokenSet_12_data_,10);
const unsigned long AdaParser::_tokenSet_13_data_[] = { 814202816UL, 401413UL, 4293001252UL, 47UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LPAREN COMMA RPAREN RIGHT_SHAFT "with" DOT "use" TIC "range" "digits" 
// "delta" "is" PIPE DOT_DOT "in" "renames" ASSIGN "at" "mod" "then" "loop" 
// "or" "and" "xor" "not" EQ NE LT_ LE GT GE PLUS MINUS CONCAT STAR DIV 
// "rem" EXPON 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_13(_tokenSet_13_data_,10);
const unsigned long AdaParser::_tokenSet_14_data_[] = { 52428800UL, 536870912UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "package" "procedure" "function" "generic" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_14(_tokenSet_14_data_,10);
const unsigned long AdaParser::_tokenSet_15_data_[] = { 8388800UL, 4UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LPAREN "is" "renames" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_15(_tokenSet_15_data_,10);
const unsigned long AdaParser::_tokenSet_16_data_[] = { 52453426UL, 1611238800UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF "pragma" IDENTIFIER "use" "type" "package" "procedure" "function" 
// CHAR_STRING "separate" "abstract" "task" "protected" "for" "end" "subtype" 
// "generic" "begin" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_16(_tokenSet_16_data_,10);
const unsigned long AdaParser::_tokenSet_17_data_[] = { 52453426UL, 1611238400UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF "pragma" IDENTIFIER "use" "type" "package" "procedure" "function" 
// "task" "protected" "for" "end" "subtype" "generic" "begin" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_17(_tokenSet_17_data_,10);
const unsigned long AdaParser::_tokenSet_18_data_[] = { 53502002UL, 1611238400UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF "pragma" IDENTIFIER "use" "type" "private" "package" "procedure" 
// "function" "task" "protected" "for" "end" "subtype" "generic" "begin" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_18(_tokenSet_18_data_,10);
const unsigned long AdaParser::_tokenSet_19_data_[] = { 8392896UL, 67108869UL, 4096UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LPAREN DOT "is" "in" "renames" "when" "do" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_19(_tokenSet_19_data_,10);
const unsigned long AdaParser::_tokenSet_20_data_[] = { 8388800UL, 67108869UL, 4096UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LPAREN "is" "in" "renames" "when" "do" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_20(_tokenSet_20_data_,10);
const unsigned long AdaParser::_tokenSet_21_data_[] = { 0UL, 65536UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "end" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_21(_tokenSet_21_data_,10);
const unsigned long AdaParser::_tokenSet_22_data_[] = { 53502002UL, 537496976UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF "pragma" IDENTIFIER "use" "type" "private" "package" "procedure" 
// "function" CHAR_STRING "separate" "abstract" "task" "protected" "for" 
// "end" "subtype" "generic" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_22(_tokenSet_22_data_,10);
const unsigned long AdaParser::_tokenSet_23_data_[] = { 53502002UL, 537496576UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF "pragma" IDENTIFIER "use" "type" "private" "package" "procedure" 
// "function" "task" "protected" "for" "end" "subtype" "generic" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_23(_tokenSet_23_data_,10);
const unsigned long AdaParser::_tokenSet_24_data_[] = { 8388672UL, 67108868UL, 4096UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI "is" "renames" "when" "do" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_24(_tokenSet_24_data_,10);
const unsigned long AdaParser::_tokenSet_25_data_[] = { 8392832UL, 512UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LPAREN DOT "is" "return" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_25(_tokenSet_25_data_,10);
const unsigned long AdaParser::_tokenSet_26_data_[] = { 8388736UL, 512UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LPAREN "is" "return" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_26(_tokenSet_26_data_,10);
const unsigned long AdaParser::_tokenSet_27_data_[] = { 8388672UL, 4UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI "is" "renames" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_27(_tokenSet_27_data_,10);
const unsigned long AdaParser::_tokenSet_28_data_[] = { 2560UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// RPAREN "with" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_28(_tokenSet_28_data_,10);
const unsigned long AdaParser::_tokenSet_29_data_[] = { 2816UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// COMMA RPAREN "with" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_29(_tokenSet_29_data_,10);
const unsigned long AdaParser::_tokenSet_30_data_[] = { 3840UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// COMMA RPAREN RIGHT_SHAFT "with" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_30(_tokenSet_30_data_,10);
const unsigned long AdaParser::_tokenSet_31_data_[] = { 268439296UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// COMMA RPAREN RIGHT_SHAFT "with" PIPE 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_31(_tokenSet_31_data_,10);
const unsigned long AdaParser::_tokenSet_32_data_[] = { 813895488UL, 8193UL, 1071775780UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI COMMA RPAREN RIGHT_SHAFT "with" "range" "digits" "is" PIPE DOT_DOT 
// "in" ASSIGN "then" "loop" "or" "and" "xor" "not" EQ NE LT_ LE GT GE 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_32(_tokenSet_32_data_,10);
const unsigned long AdaParser::_tokenSet_33_data_[] = { 738234528UL, 262200UL, 3229614080UL, 127UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER LPAREN DOT TIC "new" "others" DOT_DOT CHARACTER_LITERAL CHAR_STRING 
// "null" "mod" "not" PLUS MINUS CONCAT STAR DIV "rem" "abs" EXPON NUMERIC_LIT 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_33(_tokenSet_33_data_,10);
const unsigned long AdaParser::_tokenSet_34_data_[] = { 813895488UL, 8192UL, 6422564UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI COMMA RPAREN RIGHT_SHAFT "with" "range" "digits" "is" PIPE DOT_DOT 
// ASSIGN "then" "loop" "or" "and" "xor" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_34(_tokenSet_34_data_,10);
const unsigned long AdaParser::_tokenSet_35_data_[] = { 268439360UL, 8192UL, 32UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI COMMA RPAREN RIGHT_SHAFT "with" PIPE ASSIGN "loop" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_35(_tokenSet_35_data_,10);
const unsigned long AdaParser::_tokenSet_36_data_[] = { 32768UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// TIC 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_36(_tokenSet_36_data_,10);
const unsigned long AdaParser::_tokenSet_37_data_[] = { 576UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RPAREN 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_37(_tokenSet_37_data_,10);
const unsigned long AdaParser::_tokenSet_38_data_[] = { 524320UL, 16777219UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER "access" "in" "out" "aliased" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_38(_tokenSet_38_data_,10);
const unsigned long AdaParser::_tokenSet_39_data_[] = { 32UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_39(_tokenSet_39_data_,10);
const unsigned long AdaParser::_tokenSet_40_data_[] = { 2147484160UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// RPAREN COLON 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_40(_tokenSet_40_data_,10);
const unsigned long AdaParser::_tokenSet_41_data_[] = { 2368UL, 8192UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI COMMA "with" ASSIGN 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_41(_tokenSet_41_data_,10);
const unsigned long AdaParser::_tokenSet_42_data_[] = { 8388800UL, 512UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LPAREN "is" "return" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_42(_tokenSet_42_data_,10);
const unsigned long AdaParser::_tokenSet_43_data_[] = { 813932480UL, 270337UL, 4293001252UL, 47UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LPAREN COMMA RPAREN RIGHT_SHAFT "with" DOT TIC "range" "digits" 
// "is" PIPE DOT_DOT "in" ASSIGN "mod" "then" "loop" "or" "and" "xor" "not" 
// EQ NE LT_ LE GT GE PLUS MINUS CONCAT STAR DIV "rem" EXPON 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_43(_tokenSet_43_data_,10);
const unsigned long AdaParser::_tokenSet_44_data_[] = { 201326752UL, 56UL, 3229614080UL, 80UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER LPAREN "new" "others" CHARACTER_LITERAL CHAR_STRING "null" 
// "not" PLUS MINUS "abs" NUMERIC_LIT 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_44(_tokenSet_44_data_,10);
const unsigned long AdaParser::_tokenSet_45_data_[] = { 1006739360UL, 262201UL, 4293001216UL, 127UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER LPAREN COMMA RPAREN RIGHT_SHAFT "with" DOT TIC "range" "new" 
// "others" PIPE DOT_DOT "in" CHARACTER_LITERAL CHAR_STRING "null" "mod" 
// "or" "and" "xor" "not" EQ NE LT_ LE GT GE PLUS MINUS CONCAT STAR DIV 
// "rem" "abs" EXPON NUMERIC_LIT 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_45(_tokenSet_45_data_,10);
const unsigned long AdaParser::_tokenSet_46_data_[] = { 1006737312UL, 262201UL, 4293001216UL, 127UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER LPAREN COMMA RPAREN RIGHT_SHAFT DOT TIC "range" "new" "others" 
// PIPE DOT_DOT "in" CHARACTER_LITERAL CHAR_STRING "null" "mod" "or" "and" 
// "xor" "not" EQ NE LT_ LE GT GE PLUS MINUS CONCAT STAR DIV "rem" "abs" 
// EXPON NUMERIC_LIT 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_46(_tokenSet_46_data_,10);
const unsigned long AdaParser::_tokenSet_47_data_[] = { 512UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// RPAREN 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_47(_tokenSet_47_data_,10);
const unsigned long AdaParser::_tokenSet_48_data_[] = { 0UL, 512UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "return" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_48(_tokenSet_48_data_,10);
const unsigned long AdaParser::_tokenSet_49_data_[] = { 1048576UL, 65536UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "private" "end" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_49(_tokenSet_49_data_,10);
const unsigned long AdaParser::_tokenSet_50_data_[] = { 32UL, 23068672UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER "constant" "array" "aliased" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_50(_tokenSet_50_data_,10);
const unsigned long AdaParser::_tokenSet_51_data_[] = { 495840UL, 6299648UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER SEMI LPAREN DOT TIC "range" "digits" "delta" ASSIGN "constant" 
// "array" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_51(_tokenSet_51_data_,10);
const unsigned long AdaParser::_tokenSet_52_data_[] = { 8388672UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI "is" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_52(_tokenSet_52_data_,10);
const unsigned long AdaParser::_tokenSet_53_data_[] = { 68157504UL, 402653536UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI "private" "new" "null" "record" "abstract" "tagged" "limited" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_53(_tokenSet_53_data_,10);
const unsigned long AdaParser::_tokenSet_54_data_[] = { 51380272UL, 114688UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "pragma" IDENTIFIER "private" "procedure" "function" "entry" "for" "end" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_54(_tokenSet_54_data_,10);
const unsigned long AdaParser::_tokenSet_55_data_[] = { 192UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LPAREN 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_55(_tokenSet_55_data_,10);
const unsigned long AdaParser::_tokenSet_56_data_[] = { 496256UL, 0UL, 32UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LPAREN RPAREN DOT TIC "range" "digits" "delta" "loop" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_56(_tokenSet_56_data_,10);
const unsigned long AdaParser::_tokenSet_57_data_[] = { 512UL, 0UL, 32UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// RPAREN "loop" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_57(_tokenSet_57_data_,10);
const unsigned long AdaParser::_tokenSet_58_data_[] = { 2880UL, 8192UL, 32UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI COMMA RPAREN "with" ASSIGN "loop" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_58(_tokenSet_58_data_,10);
const unsigned long AdaParser::_tokenSet_59_data_[] = { 48UL, 65536UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "pragma" IDENTIFIER "end" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_59(_tokenSet_59_data_,10);
const unsigned long AdaParser::_tokenSet_60_data_[] = { 50331664UL, 49152UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "pragma" "procedure" "function" "entry" "for" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_60(_tokenSet_60_data_,10);
const unsigned long AdaParser::_tokenSet_61_data_[] = { 50331696UL, 100777984UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "pragma" IDENTIFIER "procedure" "function" "entry" "for" "end" "case" 
// "when" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_61(_tokenSet_61_data_,10);
const unsigned long AdaParser::_tokenSet_62_data_[] = { 64UL, 8192UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI ASSIGN 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_62(_tokenSet_62_data_,10);
const unsigned long AdaParser::_tokenSet_63_data_[] = { 1048576UL, 402653536UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "private" "null" "record" "abstract" "tagged" "limited" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_63(_tokenSet_63_data_,10);
const unsigned long AdaParser::_tokenSet_64_data_[] = { 1048688UL, 436207712UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "pragma" IDENTIFIER SEMI "private" "null" "record" "case" "tagged" "limited" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_64(_tokenSet_64_data_,10);
const unsigned long AdaParser::_tokenSet_65_data_[] = { 8192UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "use" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_65(_tokenSet_65_data_,10);
const unsigned long AdaParser::_tokenSet_66_data_[] = { 32UL, 4194304UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER "array" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_66(_tokenSet_66_data_,10);
const unsigned long AdaParser::_tokenSet_67_data_[] = { 496512UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LPAREN COMMA RPAREN DOT TIC "range" "digits" "delta" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_67(_tokenSet_67_data_,10);
const unsigned long AdaParser::_tokenSet_68_data_[] = { 268436480UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// RIGHT_SHAFT PIPE 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_68(_tokenSet_68_data_,10);
const unsigned long AdaParser::_tokenSet_69_data_[] = { 50331648UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "procedure" "function" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_69(_tokenSet_69_data_,10);
const unsigned long AdaParser::_tokenSet_70_data_[] = { 67108864UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "new" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_70(_tokenSet_70_data_,10);
const unsigned long AdaParser::_tokenSet_71_data_[] = { 1048576UL, 96UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "private" "null" "record" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_71(_tokenSet_71_data_,10);
const unsigned long AdaParser::_tokenSet_72_data_[] = { 0UL, 67174400UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "end" "when" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_72(_tokenSet_72_data_,10);
const unsigned long AdaParser::_tokenSet_73_data_[] = { 0UL, 100728832UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "end" "case" "when" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_73(_tokenSet_73_data_,10);
const unsigned long AdaParser::_tokenSet_74_data_[] = { 0UL, 33554432UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "case" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_74(_tokenSet_74_data_,10);
const unsigned long AdaParser::_tokenSet_75_data_[] = { 8388608UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "is" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_75(_tokenSet_75_data_,10);
const unsigned long AdaParser::_tokenSet_76_data_[] = { 1024UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// RIGHT_SHAFT 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_76(_tokenSet_76_data_,10);
const unsigned long AdaParser::_tokenSet_77_data_[] = { 738300064UL, 262200UL, 3229614080UL, 127UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER LPAREN DOT TIC "range" "new" "others" DOT_DOT CHARACTER_LITERAL 
// CHAR_STRING "null" "mod" "not" PLUS MINUS CONCAT STAR DIV "rem" "abs" 
// EXPON NUMERIC_LIT 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_77(_tokenSet_77_data_,10);
const unsigned long AdaParser::_tokenSet_78_data_[] = { 469800096UL, 262201UL, 4293001216UL, 127UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER LPAREN RIGHT_SHAFT DOT TIC "new" "others" PIPE "in" CHARACTER_LITERAL 
// CHAR_STRING "null" "mod" "or" "and" "xor" "not" EQ NE LT_ LE GT GE PLUS 
// MINUS CONCAT STAR DIV "rem" "abs" EXPON NUMERIC_LIT 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_78(_tokenSet_78_data_,10);
const unsigned long AdaParser::_tokenSet_79_data_[] = { 52428800UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "package" "procedure" "function" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_79(_tokenSet_79_data_,10);
const unsigned long AdaParser::_tokenSet_80_data_[] = { 52455472UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "pragma" IDENTIFIER "with" "use" "type" "package" "procedure" "function" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_80(_tokenSet_80_data_,10);
const unsigned long AdaParser::_tokenSet_81_data_[] = { 1048576UL, 402653440UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "private" "abstract" "tagged" "limited" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_81(_tokenSet_81_data_,10);
const unsigned long AdaParser::_tokenSet_82_data_[] = { 1048640UL, 402653184UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI "private" "tagged" "limited" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_82(_tokenSet_82_data_,10);
const unsigned long AdaParser::_tokenSet_83_data_[] = { 0UL, 1073807360UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "end" "begin" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_83(_tokenSet_83_data_,10);
const unsigned long AdaParser::_tokenSet_84_data_[] = { 52453424UL, 1611238400UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "pragma" IDENTIFIER "use" "type" "package" "procedure" "function" "task" 
// "protected" "for" "end" "subtype" "generic" "begin" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_84(_tokenSet_84_data_,10);
const unsigned long AdaParser::_tokenSet_85_data_[] = { 52453424UL, 1611254912UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "pragma" IDENTIFIER "use" "type" "package" "procedure" "function" "separate" 
// "task" "protected" "entry" "for" "end" "subtype" "generic" "begin" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_85(_tokenSet_85_data_,10);
const unsigned long AdaParser::_tokenSet_86_data_[] = { 50331664UL, 81920UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "pragma" "procedure" "function" "entry" "end" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_86(_tokenSet_86_data_,10);
const unsigned long AdaParser::_tokenSet_87_data_[] = { 0UL, 68222976UL, 24UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "end" "exception" "when" "elsif" "else" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_87(_tokenSet_87_data_,10);
const unsigned long AdaParser::_tokenSet_88_data_[] = { 37056UL, 8192UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LPAREN DOT TIC ASSIGN 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_88(_tokenSet_88_data_,10);
const unsigned long AdaParser::_tokenSet_89_data_[] = { 48UL, 3323036192UL, 1814398UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "pragma" IDENTIFIER "null" "return" "for" "end" "exception" "case" "when" 
// "begin" LT_LT "if" "then" "elsif" "else" "loop" "while" "declare" "exit" 
// "goto" "accept" "delay" "select" "abort" "or" "raise" "requeue" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_89(_tokenSet_89_data_,10);
const unsigned long AdaParser::_tokenSet_90_data_[] = { 32UL, 1107329568UL, 1683298UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER "null" "return" "for" "case" "begin" "if" "loop" "while" 
// "declare" "exit" "goto" "accept" "delay" "select" "abort" "raise" "requeue" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_90(_tokenSet_90_data_,10);
const unsigned long AdaParser::_tokenSet_91_data_[] = { 67145952UL, 56UL, 3229630464UL, 80UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER SEMI LPAREN DOT TIC "new" CHARACTER_LITERAL CHAR_STRING "null" 
// "until" "not" PLUS MINUS "abs" NUMERIC_LIT 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_91(_tokenSet_91_data_,10);
const unsigned long AdaParser::_tokenSet_92_data_[] = { 0UL, 67108864UL, 272384UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "when" "accept" "delay" "terminate" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_92(_tokenSet_92_data_,10);
const unsigned long AdaParser::_tokenSet_93_data_[] = { 67109088UL, 56UL, 3229630464UL, 80UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER SEMI LPAREN "new" CHARACTER_LITERAL CHAR_STRING "null" "until" 
// "not" PLUS MINUS "abs" NUMERIC_LIT 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_93(_tokenSet_93_data_,10);
const unsigned long AdaParser::_tokenSet_94_data_[] = { 37056UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LPAREN DOT TIC 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_94(_tokenSet_94_data_,10);
const unsigned long AdaParser::_tokenSet_95_data_[] = { 96UL, 16UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER SEMI CHAR_STRING 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_95(_tokenSet_95_data_,10);
const unsigned long AdaParser::_tokenSet_96_data_[] = { 0UL, 1073774592UL, 352UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "for" "begin" "loop" "while" "declare" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_96(_tokenSet_96_data_,10);
const unsigned long AdaParser::_tokenSet_97_data_[] = { 0UL, 65536UL, 24UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "end" "elsif" "else" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_97(_tokenSet_97_data_,10);
const unsigned long AdaParser::_tokenSet_98_data_[] = { 0UL, 65536UL, 16UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "end" "else" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_98(_tokenSet_98_data_,10);
const unsigned long AdaParser::_tokenSet_99_data_[] = { 8389696UL, 0UL, 36UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI RIGHT_SHAFT "is" "then" "loop" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_99(_tokenSet_99_data_,10);
const unsigned long AdaParser::_tokenSet_100_data_[] = { 0UL, 0UL, 32UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "loop" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_100(_tokenSet_100_data_,10);
const unsigned long AdaParser::_tokenSet_101_data_[] = { 0UL, 1073741824UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "begin" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_101(_tokenSet_101_data_,10);
const unsigned long AdaParser::_tokenSet_102_data_[] = { 64UL, 67108864UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI "when" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_102(_tokenSet_102_data_,10);
const unsigned long AdaParser::_tokenSet_103_data_[] = { 0UL, 67108864UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "when" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_103(_tokenSet_103_data_,10);
const unsigned long AdaParser::_tokenSet_104_data_[] = { 128UL, 67108864UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LPAREN "when" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_104(_tokenSet_104_data_,10);
const unsigned long AdaParser::_tokenSet_105_data_[] = { 48UL, 3254813216UL, 1814390UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "pragma" IDENTIFIER "null" "return" "for" "case" "begin" LT_LT "if" 
// "then" "else" "loop" "while" "declare" "exit" "goto" "accept" "delay" 
// "select" "abort" "or" "raise" "requeue" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_105(_tokenSet_105_data_,10);
const unsigned long AdaParser::_tokenSet_106_data_[] = { 192UL, 0UL, 4096UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LPAREN "do" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_106(_tokenSet_106_data_,10);
const unsigned long AdaParser::_tokenSet_107_data_[] = { 0UL, 0UL, 4UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "then" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_107(_tokenSet_107_data_,10);
const unsigned long AdaParser::_tokenSet_108_data_[] = { 0UL, 0UL, 131088UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "else" "or" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_108(_tokenSet_108_data_,10);
const unsigned long AdaParser::_tokenSet_109_data_[] = { 0UL, 65536UL, 131088UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "end" "else" "or" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_109(_tokenSet_109_data_,10);
const unsigned long AdaParser::_tokenSet_110_data_[] = { 0UL, 65536UL, 131092UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "end" "then" "else" "or" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_110(_tokenSet_110_data_,10);
const unsigned long AdaParser::_tokenSet_111_data_[] = { 0UL, 0UL, 272384UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "accept" "delay" "terminate" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_111(_tokenSet_111_data_,10);
const unsigned long AdaParser::_tokenSet_112_data_[] = { 134217760UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER "others" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_112(_tokenSet_112_data_,10);
const unsigned long AdaParser::_tokenSet_113_data_[] = { 813895490UL, 270337UL, 4293001252UL, 47UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF SEMI COMMA RPAREN RIGHT_SHAFT "with" "range" "digits" "is" PIPE 
// DOT_DOT "in" ASSIGN "mod" "then" "loop" "or" "and" "xor" "not" EQ NE 
// LT_ LE GT GE PLUS MINUS CONCAT STAR DIV "rem" EXPON 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_113(_tokenSet_113_data_,10);
const unsigned long AdaParser::_tokenSet_114_data_[] = { 813932352UL, 8192UL, 6422564UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI COMMA RPAREN RIGHT_SHAFT "with" DOT TIC "range" "digits" "is" PIPE 
// DOT_DOT ASSIGN "then" "loop" "or" "and" "xor" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_114(_tokenSet_114_data_,10);
const unsigned long AdaParser::_tokenSet_115_data_[] = { 813895488UL, 8193UL, 4293001252UL, 1UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI COMMA RPAREN RIGHT_SHAFT "with" "range" "digits" "is" PIPE DOT_DOT 
// "in" ASSIGN "then" "loop" "or" "and" "xor" "not" EQ NE LT_ LE GT GE 
// PLUS MINUS CONCAT 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_115(_tokenSet_115_data_,10);
const unsigned long AdaParser::_tokenSet_116_data_[] = { 813895488UL, 270337UL, 4293001252UL, 15UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI COMMA RPAREN RIGHT_SHAFT "with" "range" "digits" "is" PIPE DOT_DOT 
// "in" ASSIGN "mod" "then" "loop" "or" "and" "xor" "not" EQ NE LT_ LE 
// GT GE PLUS MINUS CONCAT STAR DIV "rem" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_116(_tokenSet_116_data_,10);
const unsigned long AdaParser::_tokenSet_117_data_[] = { 813895488UL, 270337UL, 4293001252UL, 47UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI COMMA RPAREN RIGHT_SHAFT "with" "range" "digits" "is" PIPE DOT_DOT 
// "in" ASSIGN "mod" "then" "loop" "or" "and" "xor" "not" EQ NE LT_ LE 
// GT GE PLUS MINUS CONCAT STAR DIV "rem" EXPON 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_117(_tokenSet_117_data_,10);


