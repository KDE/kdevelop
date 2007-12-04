/* $ANTLR 2.7.7 (20070609): "ada.g" -> "AdaParser.cpp"$ */
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
	returnAST = compilation_unit_AST;
}

void AdaParser::context_items_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST context_items_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
		case WITH:
		{
			with_clause();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			break;
		}
		case USE:
		{
			use_clause();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
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
	if ( inputState->guessing==0 ) {
		context_items_opt_AST = RefAdaAST(currentAST.root);
#line 86 "ada.g"
		context_items_opt_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(CONTEXT_CLAUSE,"CONTEXT_CLAUSE")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(context_items_opt_AST))));
#line 135 "AdaParser.cpp"
		currentAST.root = context_items_opt_AST;
		if ( context_items_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			context_items_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = context_items_opt_AST->getFirstChild();
		else
			currentAST.child = context_items_opt_AST;
		currentAST.advanceChildToEnd();
	}
	context_items_opt_AST = RefAdaAST(currentAST.root);
	returnAST = context_items_opt_AST;
}

void AdaParser::library_item() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST library_item_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 194 "AdaParser.cpp"
		currentAST.root = library_item_AST;
		if ( library_item_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			library_item_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = library_item_AST->getFirstChild();
		else
			currentAST.child = library_item_AST;
		currentAST.advanceChildToEnd();
	}
	library_item_AST = RefAdaAST(currentAST.root);
	returnAST = library_item_AST;
}

void AdaParser::subunit() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST subunit_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  sep = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST sep_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 1497 "ada.g"
		Set(sep_AST, SUBUNIT);
#line 229 "AdaParser.cpp"
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
	returnAST = subunit_AST;
}

void AdaParser::pragma() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST pragma_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
	returnAST = pragma_AST;
}

void AdaParser::pragma_args_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST pragma_args_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
	returnAST = pragma_args_opt_AST;
}

void AdaParser::pragma_arg() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST pragma_arg_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
	else if ((_tokenSet_0.member(LA(1))) && (_tokenSet_1.member(LA(2)))) {
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
#line 1417 "ada.g"
					Set (a_AST, AND_THEN);
#line 416 "AdaParser.cpp"
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
#line 1418 "ada.g"
					Set (o_AST, OR_ELSE);
#line 462 "AdaParser.cpp"
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
			goto _loop450;
		}
		}
	}
	_loop450:;
	} // ( ... )*
	expression_AST = RefAdaAST(currentAST.root);
	returnAST = expression_AST;
}

void AdaParser::with_clause() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST with_clause_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  w = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST w_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 539 "AdaParser.cpp"
	}
	with_clause_AST = RefAdaAST(currentAST.root);
	returnAST = with_clause_AST;
}

void AdaParser::use_clause() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST use_clause_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  u = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST u_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
				goto _loop25;
			}
			
		}
		_loop25:;
		} // ( ... )*
		if ( inputState->guessing==0 ) {
#line 108 "ada.g"
			Set(u_AST, USE_TYPE_CLAUSE);
#line 586 "AdaParser.cpp"
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
#line 599 "AdaParser.cpp"
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
	returnAST = use_clause_AST;
}

void AdaParser::c_name_list() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST c_name_list_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
			goto _loop18;
		}
		
	}
	_loop18:;
	} // ( ... )*
	c_name_list_AST = RefAdaAST(currentAST.root);
	returnAST = c_name_list_AST;
}

void AdaParser::compound_name() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST compound_name_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
			goto _loop21;
		}
		
	}
	_loop21:;
	} // ( ... )*
	compound_name_AST = RefAdaAST(currentAST.root);
	returnAST = compound_name_AST;
}

void AdaParser::subtype_mark() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST subtype_mark_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
	returnAST = subtype_mark_AST;
}

void AdaParser::attribute_id() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST attribute_id_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
	returnAST = attribute_id_AST;
}

void AdaParser::private_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST private_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 843 "AdaParser.cpp"
		currentAST.root = private_opt_AST;
		if ( private_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			private_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = private_opt_AST->getFirstChild();
		else
			currentAST.child = private_opt_AST;
		currentAST.advanceChildToEnd();
	}
	private_opt_AST = RefAdaAST(currentAST.root);
	returnAST = private_opt_AST;
}

void AdaParser::lib_pkg_spec_or_body() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST lib_pkg_spec_or_body_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  pkg = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST pkg_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
		match(SEMI);
		if ( inputState->guessing==0 ) {
#line 143 "ada.g"
			Set(pkg_AST, PACKAGE_BODY);
#line 887 "AdaParser.cpp"
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
	returnAST = lib_pkg_spec_or_body_AST;
}

void AdaParser::subprog_decl_or_rename_or_inst_or_body(
	boolean lib_level
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST subprog_decl_or_rename_or_inst_or_body_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  p = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST p_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  f = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST f_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
#line 944 "ada.g"
	RefAdaAST t;
#line 925 "AdaParser.cpp"
	
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
#line 948 "ada.g"
				Set(p_AST, GENERIC_PROCEDURE_INSTANTIATION);
#line 949 "AdaParser.cpp"
			}
		}
		else if ((_tokenSet_2.member(LA(1))) && (_tokenSet_3.member(LA(2)))) {
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
#line 950 "ada.g"
					Set(p_AST, PROCEDURE_RENAMING_DECLARATION);
#line 968 "AdaParser.cpp"
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
#line 952 "ada.g"
						Set(p_AST, PROCEDURE_BODY);
#line 1007 "AdaParser.cpp"
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
#line 954 "ada.g"
					pop_def_id();
								    Set(p_AST, PROCEDURE_DECLARATION);
#line 1025 "AdaParser.cpp"
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
#line 961 "ada.g"
				Set(f_AST, GENERIC_FUNCTION_INSTANTIATION);
#line 1068 "AdaParser.cpp"
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
#line 963 "ada.g"
					Set(f_AST, FUNCTION_RENAMING_DECLARATION);
#line 1090 "AdaParser.cpp"
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
#line 965 "ada.g"
						Set(f_AST, FUNCTION_BODY);
#line 1129 "AdaParser.cpp"
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
#line 967 "ada.g"
					pop_def_id();
								    Set(f_AST, FUNCTION_DECLARATION);
#line 1147 "AdaParser.cpp"
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
	returnAST = subprog_decl_or_rename_or_inst_or_body_AST;
}

void AdaParser::generic_decl(
	boolean lib_level
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST generic_decl_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  g = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST g_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 865 "ada.g"
				Set(g_AST, GENERIC_PACKAGE_RENAMING);
#line 1216 "AdaParser.cpp"
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
#line 866 "ada.g"
				Set(g_AST, GENERIC_PACKAGE_DECLARATION);
#line 1230 "AdaParser.cpp"
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
#line 869 "ada.g"
				Set(g_AST, GENERIC_PROCEDURE_RENAMING);
#line 1264 "AdaParser.cpp"
			}
			break;
		}
		case SEMI:
		{
			if ( inputState->guessing==0 ) {
#line 872 "ada.g"
				Set(g_AST, GENERIC_PROCEDURE_DECLARATION); pop_def_id();
#line 1273 "AdaParser.cpp"
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
#line 875 "ada.g"
				Set(g_AST, GENERIC_FUNCTION_RENAMING);
#line 1307 "AdaParser.cpp"
			}
			break;
		}
		case SEMI:
		{
			if ( inputState->guessing==0 ) {
#line 878 "ada.g"
				Set(g_AST, GENERIC_FUNCTION_DECLARATION); pop_def_id();
#line 1316 "AdaParser.cpp"
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
	returnAST = generic_decl_AST;
}

void AdaParser::def_id(
	boolean lib_level
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST def_id_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefAdaAST cn_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  n = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST n_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	if (((LA(1) == IDENTIFIER) && (_tokenSet_4.member(LA(2))))&&( lib_level )) {
		compound_name();
		if (inputState->guessing==0) {
			cn_AST = returnAST;
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if ( inputState->guessing==0 ) {
#line 171 "ada.g"
			push_def_id(cn_AST);
#line 1358 "AdaParser.cpp"
		}
		def_id_AST = RefAdaAST(currentAST.root);
	}
	else if (((LA(1) == IDENTIFIER) && (_tokenSet_5.member(LA(2))))&&( !lib_level )) {
		n = LT(1);
		if ( inputState->guessing == 0 ) {
			n_AST = astFactory->create(n);
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(n_AST));
		}
		match(IDENTIFIER);
		if ( inputState->guessing==0 ) {
#line 172 "ada.g"
			push_def_id(n_AST);
#line 1372 "AdaParser.cpp"
		}
		def_id_AST = RefAdaAST(currentAST.root);
	}
	else {
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	
	returnAST = def_id_AST;
}

void AdaParser::pkg_body_part() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST pkg_body_part_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	declarative_part();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	block_body_opt();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	end_id_opt();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	pkg_body_part_AST = RefAdaAST(currentAST.root);
	returnAST = pkg_body_part_AST;
}

void AdaParser::spec_decl_part(
	RefAdaAST pkg
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST spec_decl_part_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 364 "ada.g"
				Set(pkg, GENERIC_PACKAGE_INSTANTIATION);
#line 1427 "AdaParser.cpp"
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
#line 365 "ada.g"
				Set(pkg, PACKAGE_SPECIFICATION);
#line 1453 "AdaParser.cpp"
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
#line 367 "ada.g"
			Set(pkg, PACKAGE_RENAMING_DECLARATION);
#line 1474 "AdaParser.cpp"
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
	returnAST = spec_decl_part_AST;
}

void AdaParser::subprog_decl(
	boolean lib_level
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
#line 1501 "AdaParser.cpp"
	
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
#line 1525 "AdaParser.cpp"
			}
		}
		else if ((_tokenSet_2.member(LA(1))) && (_tokenSet_6.member(LA(2)))) {
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
#line 1544 "AdaParser.cpp"
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
#line 1596 "AdaParser.cpp"
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
#line 1618 "AdaParser.cpp"
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
	returnAST = subprog_decl_AST;
}

void AdaParser::generic_subp_inst() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST generic_subp_inst_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	match(IS);
	generic_inst();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	match(SEMI);
	generic_subp_inst_AST = RefAdaAST(currentAST.root);
	returnAST = generic_subp_inst_AST;
}

void AdaParser::formal_part_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST formal_part_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
				goto _loop70;
			}
			
		}
		_loop70:;
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
#line 235 "ada.g"
		formal_part_opt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(FORMAL_PART_OPT,"FORMAL_PART_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(formal_part_opt_AST))));
#line 1723 "AdaParser.cpp"
		currentAST.root = formal_part_opt_AST;
		if ( formal_part_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			formal_part_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = formal_part_opt_AST->getFirstChild();
		else
			currentAST.child = formal_part_opt_AST;
		currentAST.advanceChildToEnd();
	}
	formal_part_opt_AST = RefAdaAST(currentAST.root);
	returnAST = formal_part_opt_AST;
}

void AdaParser::renames() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST renames_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
#line 258 "ada.g"
	RefAdaAST dummy;
#line 1742 "AdaParser.cpp"
	
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
#line 262 "ada.g"
		pop_def_id();
#line 1772 "AdaParser.cpp"
	}
	renames_AST = RefAdaAST(currentAST.root);
	returnAST = renames_AST;
}

void AdaParser::is_separate_or_abstract_or_decl(
	RefAdaAST t
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST is_separate_or_abstract_or_decl_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 305 "ada.g"
			pop_def_id();
				    if (t->getType() == AdaTokenTypes::PROCEDURE)
				      Set(t, PROCEDURE_DECLARATION);
				    else
				      Set(t, FUNCTION_DECLARATION);
				
#line 1802 "AdaParser.cpp"
		}
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	returnAST = is_separate_or_abstract_or_decl_AST;
}

void AdaParser::def_designator(
	boolean lib_level
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST def_designator_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefAdaAST n_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
#line 330 "ada.g"
	RefAdaAST d;
#line 1823 "AdaParser.cpp"
	
	if (((LA(1) == IDENTIFIER) && (_tokenSet_7.member(LA(2))))&&( lib_level )) {
		compound_name();
		if (inputState->guessing==0) {
			n_AST = returnAST;
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if ( inputState->guessing==0 ) {
#line 332 "ada.g"
			push_def_id(n_AST);
#line 1834 "AdaParser.cpp"
		}
		def_designator_AST = RefAdaAST(currentAST.root);
	}
	else if (((LA(1) == IDENTIFIER || LA(1) == CHAR_STRING) && (LA(2) == LPAREN || LA(2) == IS || LA(2) == RETURN))&&( !lib_level )) {
		d=designator();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if ( inputState->guessing==0 ) {
#line 333 "ada.g"
			push_def_id(d);
#line 1846 "AdaParser.cpp"
		}
		def_designator_AST = RefAdaAST(currentAST.root);
	}
	else {
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	
	returnAST = def_designator_AST;
}

void AdaParser::function_tail() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST function_tail_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
	returnAST = function_tail_AST;
}

void AdaParser::generic_inst() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST generic_inst_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 1910 "AdaParser.cpp"
	}
	generic_inst_AST = RefAdaAST(currentAST.root);
	returnAST = generic_inst_AST;
}

void AdaParser::value_s() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST value_s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
			goto _loop445;
		}
		
	}
	_loop445:;
	} // ( ... )*
	if ( inputState->guessing==0 ) {
		value_s_AST = RefAdaAST(currentAST.root);
#line 1405 "ada.g"
		value_s_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(VALUES,"VALUES")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(value_s_AST))));
#line 1945 "AdaParser.cpp"
		currentAST.root = value_s_AST;
		if ( value_s_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			value_s_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = value_s_AST->getFirstChild();
		else
			currentAST.child = value_s_AST;
		currentAST.advanceChildToEnd();
	}
	value_s_AST = RefAdaAST(currentAST.root);
	returnAST = value_s_AST;
}

void AdaParser::parenth_values() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST parenth_values_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
			goto _loop46;
		}
		
	}
	_loop46:;
	} // ( ... )*
	match(RPAREN);
	parenth_values_AST = RefAdaAST(currentAST.root);
	returnAST = parenth_values_AST;
}

void AdaParser::value() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST value_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{
	switch ( LA(1)) {
	case OTHERS:
	{
		RefAdaAST tmp60_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp60_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp60_AST));
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
			RefAdaAST tmp62_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp62_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp62_AST));
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
	returnAST = value_AST;
}

void AdaParser::ranged_expr_s() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST ranged_expr_s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	ranged_expr();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	{ // ( ... )*
	for (;;) {
		if ((LA(1) == PIPE)) {
			RefAdaAST tmp63_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp63_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp63_AST));
			}
			match(PIPE);
			ranged_expr();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
		}
		else {
			goto _loop52;
		}
		
	}
	_loop52:;
	} // ( ... )*
	ranged_expr_s_AST = RefAdaAST(currentAST.root);
	returnAST = ranged_expr_s_AST;
}

void AdaParser::ranged_expr() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST ranged_expr_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	expression();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	{
	switch ( LA(1)) {
	case DOT_DOT:
	{
		RefAdaAST tmp64_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp64_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp64_AST));
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
		RefAdaAST tmp65_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp65_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp65_AST));
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
	returnAST = ranged_expr_AST;
}

void AdaParser::simple_expression() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST simple_expression_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	signed_term();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	{ // ( ... )*
	for (;;) {
		switch ( LA(1)) {
		case PLUS:
		{
			RefAdaAST tmp66_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp66_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp66_AST));
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
			RefAdaAST tmp67_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp67_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp67_AST));
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
			RefAdaAST tmp68_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp68_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp68_AST));
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
			goto _loop458;
		}
		}
	}
	_loop458:;
	} // ( ... )*
	simple_expression_AST = RefAdaAST(currentAST.root);
	returnAST = simple_expression_AST;
}

void AdaParser::range() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST range_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{
	bool synPredMatched59 = false;
	if (((_tokenSet_0.member(LA(1))) && (_tokenSet_8.member(LA(2))))) {
		int _m59 = mark();
		synPredMatched59 = true;
		inputState->guessing++;
		try {
			{
			range_dots();
			}
		}
		catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
			synPredMatched59 = false;
		}
		rewind(_m59);
		inputState->guessing--;
	}
	if ( synPredMatched59 ) {
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
	returnAST = range_AST;
}

void AdaParser::range_constraint() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST range_constraint_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  r = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST r_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	r = LT(1);
	if ( inputState->guessing == 0 ) {
		r_AST = astFactory->create(r);
		astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(r_AST));
	}
	match(RANGE);
	range();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	if ( inputState->guessing==0 ) {
#line 203 "ada.g"
		Set(r_AST, RANGE_CONSTRAINT);
#line 2287 "AdaParser.cpp"
	}
	range_constraint_AST = RefAdaAST(currentAST.root);
	returnAST = range_constraint_AST;
}

void AdaParser::range_dots() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST range_dots_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	simple_expression();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	RefAdaAST tmp69_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	if ( inputState->guessing == 0 ) {
		tmp69_AST = astFactory->create(LT(1));
		astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp69_AST));
	}
	match(DOT_DOT);
	simple_expression();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	range_dots_AST = RefAdaAST(currentAST.root);
	returnAST = range_dots_AST;
}

void AdaParser::range_attrib_ref() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST range_attrib_ref_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  r = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST r_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 219 "ada.g"
		Set(r_AST, RANGE_ATTRIBUTE_REFERENCE);
#line 2374 "AdaParser.cpp"
	}
	range_attrib_ref_AST = RefAdaAST(currentAST.root);
	returnAST = range_attrib_ref_AST;
}

void AdaParser::prefix() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST prefix_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  p = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST p_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	RefAdaAST tmp73_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	if ( inputState->guessing == 0 ) {
		tmp73_AST = astFactory->create(LT(1));
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp73_AST));
	}
	match(IDENTIFIER);
	{ // ( ... )*
	for (;;) {
		switch ( LA(1)) {
		case DOT:
		{
			RefAdaAST tmp74_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp74_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp74_AST));
			}
			match(DOT);
			{
			switch ( LA(1)) {
			case ALL:
			{
				RefAdaAST tmp75_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp75_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp75_AST));
				}
				match(ALL);
				break;
			}
			case IDENTIFIER:
			{
				RefAdaAST tmp76_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp76_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp76_AST));
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
#line 228 "ada.g"
				Set(p_AST, INDEXED_COMPONENT);
#line 2450 "AdaParser.cpp"
			}
			break;
		}
		default:
		{
			goto _loop66;
		}
		}
	}
	_loop66:;
	} // ( ... )*
	prefix_AST = RefAdaAST(currentAST.root);
	returnAST = prefix_AST;
}

void AdaParser::parameter_specification() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST parameter_specification_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 240 "ada.g"
		parameter_specification_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(PARAMETER_SPECIFICATION,"PARAMETER_SPECIFICATION")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(parameter_specification_AST))));
#line 2492 "AdaParser.cpp"
		currentAST.root = parameter_specification_AST;
		if ( parameter_specification_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			parameter_specification_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = parameter_specification_AST->getFirstChild();
		else
			currentAST.child = parameter_specification_AST;
		currentAST.advanceChildToEnd();
	}
	parameter_specification_AST = RefAdaAST(currentAST.root);
	returnAST = parameter_specification_AST;
}

void AdaParser::def_ids_colon() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST def_ids_colon_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	defining_identifier_list();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	match(COLON);
	def_ids_colon_AST = RefAdaAST(currentAST.root);
	returnAST = def_ids_colon_AST;
}

void AdaParser::mode_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST mode_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{
	switch ( LA(1)) {
	case IN:
	{
		RefAdaAST tmp79_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp79_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp79_AST));
		}
		match(IN);
		{
		switch ( LA(1)) {
		case OUT:
		{
			RefAdaAST tmp80_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp80_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp80_AST));
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
		RefAdaAST tmp81_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp81_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp81_AST));
		}
		match(OUT);
		break;
	}
	case ACCESS:
	{
		RefAdaAST tmp82_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp82_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp82_AST));
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
#line 255 "ada.g"
		mode_opt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(MODIFIERS,"MODIFIERS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(mode_opt_AST))));
#line 2592 "AdaParser.cpp"
		currentAST.root = mode_opt_AST;
		if ( mode_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			mode_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = mode_opt_AST->getFirstChild();
		else
			currentAST.child = mode_opt_AST;
		currentAST.advanceChildToEnd();
	}
	mode_opt_AST = RefAdaAST(currentAST.root);
	returnAST = mode_opt_AST;
}

void AdaParser::init_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST init_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 468 "ada.g"
		init_opt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(INIT_OPT,"INIT_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(init_opt_AST))));
#line 2636 "AdaParser.cpp"
		currentAST.root = init_opt_AST;
		if ( init_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			init_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = init_opt_AST->getFirstChild();
		else
			currentAST.child = init_opt_AST;
		currentAST.advanceChildToEnd();
	}
	init_opt_AST = RefAdaAST(currentAST.root);
	returnAST = init_opt_AST;
}

void AdaParser::defining_identifier_list() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST defining_identifier_list_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	RefAdaAST tmp84_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	if ( inputState->guessing == 0 ) {
		tmp84_AST = astFactory->create(LT(1));
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp84_AST));
	}
	match(IDENTIFIER);
	{ // ( ... )*
	for (;;) {
		if ((LA(1) == COMMA)) {
			match(COMMA);
			RefAdaAST tmp86_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp86_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp86_AST));
			}
			match(IDENTIFIER);
		}
		else {
			goto _loop75;
		}
		
	}
	_loop75:;
	} // ( ... )*
	if ( inputState->guessing==0 ) {
		defining_identifier_list_AST = RefAdaAST(currentAST.root);
#line 249 "ada.g"
		defining_identifier_list_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(DEFINING_IDENTIFIER_LIST,"DEFINING_IDENTIFIER_LIST")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(defining_identifier_list_AST))));
#line 2683 "AdaParser.cpp"
		currentAST.root = defining_identifier_list_AST;
		if ( defining_identifier_list_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			defining_identifier_list_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = defining_identifier_list_AST->getFirstChild();
		else
			currentAST.child = defining_identifier_list_AST;
		currentAST.advanceChildToEnd();
	}
	defining_identifier_list_AST = RefAdaAST(currentAST.root);
	returnAST = defining_identifier_list_AST;
}

void AdaParser::name() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST name_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  p = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST p_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
#line 265 "ada.g"
	RefAdaAST dummy;
#line 2704 "AdaParser.cpp"
	
	RefAdaAST tmp87_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	if ( inputState->guessing == 0 ) {
		tmp87_AST = astFactory->create(LT(1));
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp87_AST));
	}
	match(IDENTIFIER);
	{ // ( ... )*
	for (;;) {
		switch ( LA(1)) {
		case DOT:
		{
			RefAdaAST tmp88_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp88_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp88_AST));
			}
			match(DOT);
			{
			switch ( LA(1)) {
			case ALL:
			{
				RefAdaAST tmp89_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp89_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp89_AST));
				}
				match(ALL);
				break;
			}
			case IDENTIFIER:
			{
				RefAdaAST tmp90_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp90_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp90_AST));
				}
				match(IDENTIFIER);
				break;
			}
			case CHARACTER_LITERAL:
			{
				RefAdaAST tmp91_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp91_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp91_AST));
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
#line 273 "ada.g"
				Set(p_AST, INDEXED_COMPONENT);
#line 2787 "AdaParser.cpp"
			}
			break;
		}
		case TIC:
		{
			RefAdaAST tmp93_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp93_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp93_AST));
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
			goto _loop84;
		}
		}
	}
	_loop84:;
	} // ( ... )*
	name_AST = RefAdaAST(currentAST.root);
	returnAST = name_AST;
}

RefAdaAST  AdaParser::definable_operator_symbol() {
#line 285 "ada.g"
	RefAdaAST d;
#line 2820 "AdaParser.cpp"
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST definable_operator_symbol_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  op = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST op_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	if (!( definable_operator(LT(1)->getText().c_str()) ))
		throw ANTLR_USE_NAMESPACE(antlr)SemanticException(" definable_operator(LT(1)->getText().c_str()) ");
	op = LT(1);
	if ( inputState->guessing == 0 ) {
		op_AST = astFactory->create(op);
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(op_AST));
	}
	match(CHAR_STRING);
	if ( inputState->guessing==0 ) {
#line 287 "ada.g"
		op_AST->setType(OPERATOR_SYMBOL); d=op_AST;
#line 2838 "AdaParser.cpp"
	}
	definable_operator_symbol_AST = RefAdaAST(currentAST.root);
	returnAST = definable_operator_symbol_AST;
	return d;
}

RefAdaAST  AdaParser::is_operator() {
#line 280 "ada.g"
	RefAdaAST d;
#line 2848 "AdaParser.cpp"
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST is_operator_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  op = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST op_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	if (!( is_operator_symbol(LT(1)->getText().c_str()) ))
		throw ANTLR_USE_NAMESPACE(antlr)SemanticException(" is_operator_symbol(LT(1)->getText().c_str()) ");
	op = LT(1);
	if ( inputState->guessing == 0 ) {
		op_AST = astFactory->create(op);
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(op_AST));
	}
	match(CHAR_STRING);
	if ( inputState->guessing==0 ) {
#line 282 "ada.g"
		op_AST->setType(OPERATOR_SYMBOL); d=op_AST;
#line 2866 "AdaParser.cpp"
	}
	is_operator_AST = RefAdaAST(currentAST.root);
	returnAST = is_operator_AST;
	return d;
}

void AdaParser::parenthesized_primary() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST parenthesized_primary_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  pp = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST pp_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	pp = LT(1);
	if ( inputState->guessing == 0 ) {
		pp_AST = astFactory->create(pp);
		astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(pp_AST));
	}
	match(LPAREN);
	{
	if ((LA(1) == NuLL) && (LA(2) == RECORD)) {
		RefAdaAST tmp94_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp94_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp94_AST));
		}
		match(NuLL);
		match(RECORD);
	}
	else if ((_tokenSet_9.member(LA(1))) && (_tokenSet_10.member(LA(2)))) {
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
#line 295 "ada.g"
		Set(pp_AST, PARENTHESIZED_PRIMARY);
#line 2915 "AdaParser.cpp"
	}
	parenthesized_primary_AST = RefAdaAST(currentAST.root);
	returnAST = parenthesized_primary_AST;
}

void AdaParser::extension_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST extension_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{
	switch ( LA(1)) {
	case WITH:
	{
		match(WITH);
		{
		if ((LA(1) == NuLL) && (LA(2) == RECORD)) {
			RefAdaAST tmp98_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp98_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp98_AST));
			}
			match(NuLL);
			match(RECORD);
		}
		else if ((_tokenSet_9.member(LA(1))) && (_tokenSet_11.member(LA(2)))) {
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
#line 299 "ada.g"
		extension_opt_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(EXTENSION_OPT,"EXTENSION_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(extension_opt_AST))));
#line 2969 "AdaParser.cpp"
		currentAST.root = extension_opt_AST;
		if ( extension_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			extension_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = extension_opt_AST->getFirstChild();
		else
			currentAST.child = extension_opt_AST;
		currentAST.advanceChildToEnd();
	}
	extension_opt_AST = RefAdaAST(currentAST.root);
	returnAST = extension_opt_AST;
}

void AdaParser::separate_or_abstract(
	RefAdaAST t
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST separate_or_abstract_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	switch ( LA(1)) {
	case SEPARATE:
	{
		match(SEPARATE);
		if ( inputState->guessing==0 ) {
#line 315 "ada.g"
			pop_def_id();
					  if (t->getType() == AdaTokenTypes::PROCEDURE)
					    Set(t, PROCEDURE_BODY_STUB);
					  else
					    Set(t, FUNCTION_BODY_STUB);
					
#line 3001 "AdaParser.cpp"
		}
		break;
	}
	case ABSTRACT:
	{
		match(ABSTRACT);
		if ( inputState->guessing==0 ) {
#line 322 "ada.g"
			pop_def_id();
					  if (t->getType() == AdaTokenTypes::PROCEDURE)
					    Set(t, ABSTRACT_PROCEDURE_DECLARATION);
					  else
					    Set(t, ABSTRACT_FUNCTION_DECLARATION);
					
#line 3016 "AdaParser.cpp"
		}
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	returnAST = separate_or_abstract_AST;
}

RefAdaAST  AdaParser::designator() {
#line 336 "ada.g"
	RefAdaAST d;
#line 3031 "AdaParser.cpp"
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST designator_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  n = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST n_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
#line 336 "ada.g"
	RefAdaAST op;
#line 3039 "AdaParser.cpp"
	
	switch ( LA(1)) {
	case CHAR_STRING:
	{
		op=definable_operator_symbol();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if ( inputState->guessing==0 ) {
#line 338 "ada.g"
			d = op;
#line 3051 "AdaParser.cpp"
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
#line 339 "ada.g"
			d = n_AST;
#line 3067 "AdaParser.cpp"
		}
		designator_AST = RefAdaAST(currentAST.root);
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	returnAST = designator_AST;
	return d;
}

void AdaParser::func_formal_part_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST func_formal_part_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
				goto _loop100;
			}
			
		}
		_loop100:;
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
#line 348 "ada.g"
		func_formal_part_opt_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(FORMAL_PART_OPT,"FORMAL_PART_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(func_formal_part_opt_AST))));
#line 3129 "AdaParser.cpp"
		currentAST.root = func_formal_part_opt_AST;
		if ( func_formal_part_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			func_formal_part_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = func_formal_part_opt_AST->getFirstChild();
		else
			currentAST.child = func_formal_part_opt_AST;
		currentAST.advanceChildToEnd();
	}
	func_formal_part_opt_AST = RefAdaAST(currentAST.root);
	returnAST = func_formal_part_opt_AST;
}

void AdaParser::func_param() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST func_param_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 354 "ada.g"
		func_param_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(PARAMETER_SPECIFICATION,"PARAMETER_SPECIFICATION")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(func_param_AST))));
#line 3168 "AdaParser.cpp"
		currentAST.root = func_param_AST;
		if ( func_param_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			func_param_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = func_param_AST->getFirstChild();
		else
			currentAST.child = func_param_AST;
		currentAST.advanceChildToEnd();
	}
	func_param_AST = RefAdaAST(currentAST.root);
	returnAST = func_param_AST;
}

void AdaParser::in_access_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST in_access_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{
	switch ( LA(1)) {
	case IN:
	{
		RefAdaAST tmp105_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp105_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp105_AST));
		}
		match(IN);
		break;
	}
	case ACCESS:
	{
		RefAdaAST tmp106_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp106_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp106_AST));
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
#line 360 "ada.g"
		in_access_opt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(MODIFIERS,"MODIFIERS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(in_access_opt_AST))));
#line 3222 "AdaParser.cpp"
		currentAST.root = in_access_opt_AST;
		if ( in_access_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			in_access_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = in_access_opt_AST->getFirstChild();
		else
			currentAST.child = in_access_opt_AST;
		currentAST.advanceChildToEnd();
	}
	in_access_opt_AST = RefAdaAST(currentAST.root);
	returnAST = in_access_opt_AST;
}

void AdaParser::pkg_spec_part() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST pkg_spec_part_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	basic_declarative_items_opt();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	private_declarative_items_opt();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	end_id_opt();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	pkg_spec_part_AST = RefAdaAST(currentAST.root);
	returnAST = pkg_spec_part_AST;
}

void AdaParser::basic_declarative_items_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST basic_declarative_items_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
			goto _loop114;
		}
		}
	}
	_loop114:;
	} // ( ... )*
	if ( inputState->guessing==0 ) {
		basic_declarative_items_opt_AST = RefAdaAST(currentAST.root);
#line 385 "ada.g"
		basic_declarative_items_opt_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(BASIC_DECLARATIVE_ITEMS_OPT,"BASIC_DECLARATIVE_ITEMS_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(basic_declarative_items_opt_AST))));
#line 3303 "AdaParser.cpp"
		currentAST.root = basic_declarative_items_opt_AST;
		if ( basic_declarative_items_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			basic_declarative_items_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = basic_declarative_items_opt_AST->getFirstChild();
		else
			currentAST.child = basic_declarative_items_opt_AST;
		currentAST.advanceChildToEnd();
	}
	basic_declarative_items_opt_AST = RefAdaAST(currentAST.root);
	returnAST = basic_declarative_items_opt_AST;
}

void AdaParser::private_declarative_items_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST private_declarative_items_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{
	switch ( LA(1)) {
	case PRIVATE:
	{
		match(PRIVATE);
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
				goto _loop111;
			}
			}
		}
		_loop111:;
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
	if ( inputState->guessing==0 ) {
		private_declarative_items_opt_AST = RefAdaAST(currentAST.root);
#line 378 "ada.g"
		private_declarative_items_opt_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(PRIVATE_DECLARATIVE_ITEMS_OPT,"PRIVATE_DECLARATIVE_ITEMS_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(private_declarative_items_opt_AST))));
#line 3380 "AdaParser.cpp"
		currentAST.root = private_declarative_items_opt_AST;
		if ( private_declarative_items_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			private_declarative_items_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = private_declarative_items_opt_AST->getFirstChild();
		else
			currentAST.child = private_declarative_items_opt_AST;
		currentAST.advanceChildToEnd();
	}
	private_declarative_items_opt_AST = RefAdaAST(currentAST.root);
	returnAST = private_declarative_items_opt_AST;
}

void AdaParser::end_id_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST end_id_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  e = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST e_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	e = LT(1);
	if ( inputState->guessing == 0 ) {
		e_AST = astFactory->create(e);
		astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(e_AST));
	}
	match(END);
	id_opt_aux();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	if ( inputState->guessing==0 ) {
#line 1176 "ada.g"
		Set(e_AST, END_ID_OPT);
#line 3413 "AdaParser.cpp"
	}
	end_id_opt_AST = RefAdaAST(currentAST.root);
	returnAST = end_id_opt_AST;
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
	returnAST = basic_decl_item_AST;
}

void AdaParser::basic_declarative_items() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST basic_declarative_items_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{ // ( ... )+
	int _cnt117=0;
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
			if ( _cnt117>=1 ) { goto _loop117; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
		}
		}
		_cnt117++;
	}
	_loop117:;
	}  // ( ... )+
	if ( inputState->guessing==0 ) {
		basic_declarative_items_AST = RefAdaAST(currentAST.root);
#line 392 "ada.g"
		basic_declarative_items_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(BASIC_DECLARATIVE_ITEMS_OPT,"BASIC_DECLARATIVE_ITEMS_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(basic_declarative_items_AST))));
#line 3562 "AdaParser.cpp"
		currentAST.root = basic_declarative_items_AST;
		if ( basic_declarative_items_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			basic_declarative_items_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = basic_declarative_items_AST->getFirstChild();
		else
			currentAST.child = basic_declarative_items_AST;
		currentAST.advanceChildToEnd();
	}
	basic_declarative_items_AST = RefAdaAST(currentAST.root);
	returnAST = basic_declarative_items_AST;
}

void AdaParser::task_type_or_single_decl(
	RefAdaAST tsk
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST task_type_or_single_decl_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 408 "ada.g"
			Set(tsk, TASK_TYPE_DECLARATION);
#line 3601 "AdaParser.cpp"
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
#line 410 "ada.g"
			Set(tsk, SINGLE_TASK_DECLARATION);
#line 3619 "AdaParser.cpp"
		}
		task_type_or_single_decl_AST = RefAdaAST(currentAST.root);
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
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
#line 539 "ada.g"
			Set(pro, PROTECTED_TYPE_DECLARATION);
#line 3658 "AdaParser.cpp"
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
#line 541 "ada.g"
			Set(pro, SINGLE_PROTECTED_DECLARATION);
#line 3676 "AdaParser.cpp"
		}
		prot_type_or_single_decl_AST = RefAdaAST(currentAST.root);
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
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
	
	switch ( LA(1)) {
	case TYPE:
	{
		t = LT(1);
		if ( inputState->guessing == 0 ) {
			t_AST = astFactory->create(t);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(t_AST));
		}
		match(TYPE);
		RefAdaAST tmp111_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp111_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp111_AST));
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
#line 587 "ada.g"
						Set(t_AST, INCOMPLETE_TYPE_DECLARATION);
#line 3759 "AdaParser.cpp"
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
#line 590 "ada.g"
					Set(t_AST, INCOMPLETE_TYPE_DECLARATION);
#line 3780 "AdaParser.cpp"
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
		RefAdaAST tmp115_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp115_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp115_AST));
		}
		match(IDENTIFIER);
		match(IS);
		subtype_ind();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(SEMI);
		if ( inputState->guessing==0 ) {
#line 604 "ada.g"
			Set(s_AST, SUBTYPE_DECLARATION);
#line 3825 "AdaParser.cpp"
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
		bool synPredMatched184 = false;
		if (((LA(1) == IDENTIFIER) && (LA(2) == USE))) {
			int _m184 = mark();
			synPredMatched184 = true;
			inputState->guessing++;
			try {
				{
				local_enum_name();
				match(USE);
				match(LPAREN);
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched184 = false;
			}
			rewind(_m184);
			inputState->guessing--;
		}
		if ( synPredMatched184 ) {
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
#line 609 "ada.g"
				Set(r_AST, ENUMERATION_REPESENTATION_CLAUSE);
#line 3888 "AdaParser.cpp"
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
		bool synPredMatched186 = false;
		if (((LA(1) == IDENTIFIER) && (LA(2) == COLON))) {
			int _m186 = mark();
			synPredMatched186 = true;
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
				synPredMatched186 = false;
			}
			rewind(_m186);
			inputState->guessing--;
		}
		if ( synPredMatched186 ) {
			RefAdaAST tmp121_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp121_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp121_AST));
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
#line 615 "ada.g"
				Set(erd_AST, EXCEPTION_RENAMING_DECLARATION);
#line 3954 "AdaParser.cpp"
			}
			decl_common_AST = RefAdaAST(currentAST.root);
		}
		else {
			bool synPredMatched188 = false;
			if (((LA(1) == IDENTIFIER) && (LA(2) == COLON))) {
				int _m188 = mark();
				synPredMatched188 = true;
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
					synPredMatched188 = false;
				}
				rewind(_m188);
				inputState->guessing--;
			}
			if ( synPredMatched188 ) {
				RefAdaAST tmp125_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp125_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp125_AST));
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
#line 618 "ada.g"
					Set(ord_AST, OBJECT_RENAMING_DECLARATION);
#line 4004 "AdaParser.cpp"
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
#line 621 "ada.g"
						Set(od_AST, EXCEPTION_DECLARATION);
#line 4025 "AdaParser.cpp"
					}
				}
				else {
					bool synPredMatched191 = false;
					if (((LA(1) == CONSTANT) && (LA(2) == ASSIGN))) {
						int _m191 = mark();
						synPredMatched191 = true;
						inputState->guessing++;
						try {
							{
							match(CONSTANT);
							match(ASSIGN);
							}
						}
						catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
							synPredMatched191 = false;
						}
						rewind(_m191);
						inputState->guessing--;
					}
					if ( synPredMatched191 ) {
						match(CONSTANT);
						match(ASSIGN);
						expression();
						if (inputState->guessing==0) {
							astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
						}
						if ( inputState->guessing==0 ) {
#line 623 "ada.g"
							Set(od_AST, NUMBER_DECLARATION);
#line 4056 "AdaParser.cpp"
						}
					}
					else if ((_tokenSet_12.member(LA(1))) && (_tokenSet_13.member(LA(2)))) {
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
#line 626 "ada.g"
								Set(od_AST, ARRAY_OBJECT_DECLARATION);
#line 4079 "AdaParser.cpp"
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
#line 630 "ada.g"
								Set(od_AST, OBJECT_DECLARATION);
#line 4096 "AdaParser.cpp"
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
	returnAST = decl_common_AST;
}

void AdaParser::discrim_part_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST discrim_part_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 420 "ada.g"
		discrim_part_opt_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(DISCRIM_PART_OPT,"DISCRIM_PART_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(discrim_part_opt_AST))));
#line 4153 "AdaParser.cpp"
		currentAST.root = discrim_part_opt_AST;
		if ( discrim_part_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			discrim_part_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = discrim_part_opt_AST->getFirstChild();
		else
			currentAST.child = discrim_part_opt_AST;
		currentAST.advanceChildToEnd();
	}
	discrim_part_opt_AST = RefAdaAST(currentAST.root);
	returnAST = discrim_part_opt_AST;
}

void AdaParser::task_definition_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST task_definition_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(SEMI);
		task_definition_opt_AST = RefAdaAST(currentAST.root);
		break;
	}
	case SEMI:
	{
		match(SEMI);
		if ( inputState->guessing==0 ) {
#line 415 "ada.g"
			pop_def_id();
#line 4197 "AdaParser.cpp"
		}
		task_definition_opt_AST = RefAdaAST(currentAST.root);
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	returnAST = task_definition_opt_AST;
}

void AdaParser::task_items_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST task_items_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{ // ( ... )*
	for (;;) {
		if ((LA(1) == PRAGMA)) {
			pragma();
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
	entrydecls_repspecs_opt();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	if ( inputState->guessing==0 ) {
		task_items_opt_AST = RefAdaAST(currentAST.root);
#line 473 "ada.g"
		task_items_opt_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(TASK_ITEMS_OPT,"TASK_ITEMS_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(task_items_opt_AST))));
#line 4239 "AdaParser.cpp"
		currentAST.root = task_items_opt_AST;
		if ( task_items_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			task_items_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = task_items_opt_AST->getFirstChild();
		else
			currentAST.child = task_items_opt_AST;
		currentAST.advanceChildToEnd();
	}
	task_items_opt_AST = RefAdaAST(currentAST.root);
	returnAST = task_items_opt_AST;
}

void AdaParser::private_task_items_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST private_task_items_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
				goto _loop163;
			}
			
		}
		_loop163:;
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
#line 530 "ada.g"
		private_task_items_opt_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(PRIVATE_TASK_ITEMS_OPT,"PRIVATE_TASK_ITEMS_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(private_task_items_opt_AST))));
#line 4298 "AdaParser.cpp"
		currentAST.root = private_task_items_opt_AST;
		if ( private_task_items_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			private_task_items_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = private_task_items_opt_AST->getFirstChild();
		else
			currentAST.child = private_task_items_opt_AST;
		currentAST.advanceChildToEnd();
	}
	private_task_items_opt_AST = RefAdaAST(currentAST.root);
	returnAST = private_task_items_opt_AST;
}

void AdaParser::discrim_part_text() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST discrim_part_text_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	match(LPAREN);
	{
	switch ( LA(1)) {
	case BOX:
	{
		RefAdaAST tmp137_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp137_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp137_AST));
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
	returnAST = discrim_part_text_AST;
}

void AdaParser::discriminant_specifications() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST discriminant_specifications_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
			goto _loop130;
		}
		
	}
	_loop130:;
	} // ( ... )*
	if ( inputState->guessing==0 ) {
		discriminant_specifications_AST = RefAdaAST(currentAST.root);
#line 450 "ada.g"
		discriminant_specifications_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(DISCRIMINANT_SPECIFICATIONS,"DISCRIMINANT_SPECIFICATIONS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(discriminant_specifications_AST))));
#line 4378 "AdaParser.cpp"
		currentAST.root = discriminant_specifications_AST;
		if ( discriminant_specifications_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			discriminant_specifications_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = discriminant_specifications_AST->getFirstChild();
		else
			currentAST.child = discriminant_specifications_AST;
		currentAST.advanceChildToEnd();
	}
	discriminant_specifications_AST = RefAdaAST(currentAST.root);
	returnAST = discriminant_specifications_AST;
}

void AdaParser::known_discrim_part() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST known_discrim_part_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	match(LPAREN);
	discriminant_specifications();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	match(RPAREN);
	if ( inputState->guessing==0 ) {
		known_discrim_part_AST = RefAdaAST(currentAST.root);
#line 430 "ada.g"
		known_discrim_part_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(DISCRIM_PART_OPT,"DISCRIM_PART_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(known_discrim_part_AST))));
#line 4407 "AdaParser.cpp"
		currentAST.root = known_discrim_part_AST;
		if ( known_discrim_part_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			known_discrim_part_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = known_discrim_part_AST->getFirstChild();
		else
			currentAST.child = known_discrim_part_AST;
		currentAST.advanceChildToEnd();
	}
	known_discrim_part_AST = RefAdaAST(currentAST.root);
	returnAST = known_discrim_part_AST;
}

void AdaParser::empty_discrim_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST empty_discrim_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	if ( inputState->guessing==0 ) {
		empty_discrim_opt_AST = RefAdaAST(currentAST.root);
#line 436 "ada.g"
		empty_discrim_opt_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(DISCRIM_PART_OPT,"DISCRIM_PART_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(empty_discrim_opt_AST))));
#line 4430 "AdaParser.cpp"
		currentAST.root = empty_discrim_opt_AST;
		if ( empty_discrim_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			empty_discrim_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = empty_discrim_opt_AST->getFirstChild();
		else
			currentAST.child = empty_discrim_opt_AST;
		currentAST.advanceChildToEnd();
	}
	empty_discrim_opt_AST = RefAdaAST(currentAST.root);
	returnAST = empty_discrim_opt_AST;
}

void AdaParser::discrim_part() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST discrim_part_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	discrim_part_text();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	if ( inputState->guessing==0 ) {
		discrim_part_AST = RefAdaAST(currentAST.root);
#line 443 "ada.g"
		discrim_part_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(DISCRIM_PART_OPT,"DISCRIM_PART_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(discrim_part_AST))));
#line 4457 "AdaParser.cpp"
		currentAST.root = discrim_part_AST;
		if ( discrim_part_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			discrim_part_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = discrim_part_AST->getFirstChild();
		else
			currentAST.child = discrim_part_AST;
		currentAST.advanceChildToEnd();
	}
	discrim_part_AST = RefAdaAST(currentAST.root);
	returnAST = discrim_part_AST;
}

void AdaParser::discriminant_specification() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST discriminant_specification_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 457 "ada.g"
		discriminant_specification_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(DISCRIMINANT_SPECIFICATION,"DISCRIMINANT_SPECIFICATION")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(discriminant_specification_AST))));
#line 4496 "AdaParser.cpp"
		currentAST.root = discriminant_specification_AST;
		if ( discriminant_specification_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			discriminant_specification_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = discriminant_specification_AST->getFirstChild();
		else
			currentAST.child = discriminant_specification_AST;
		currentAST.advanceChildToEnd();
	}
	discriminant_specification_AST = RefAdaAST(currentAST.root);
	returnAST = discriminant_specification_AST;
}

void AdaParser::access_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST access_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{
	switch ( LA(1)) {
	case ACCESS:
	{
		RefAdaAST tmp142_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp142_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp142_AST));
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
#line 464 "ada.g"
		access_opt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(MODIFIERS,"MODIFIERS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(access_opt_AST))));
#line 4540 "AdaParser.cpp"
		currentAST.root = access_opt_AST;
		if ( access_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			access_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = access_opt_AST->getFirstChild();
		else
			currentAST.child = access_opt_AST;
		currentAST.advanceChildToEnd();
	}
	access_opt_AST = RefAdaAST(currentAST.root);
	returnAST = access_opt_AST;
}

void AdaParser::entrydecls_repspecs_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST entrydecls_repspecs_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
					goto _loop142;
				}
				}
			}
			_loop142:;
			} // ( ... )*
		}
		else {
			goto _loop143;
		}
		
	}
	_loop143:;
	} // ( ... )*
	entrydecls_repspecs_opt_AST = RefAdaAST(currentAST.root);
	returnAST = entrydecls_repspecs_opt_AST;
}

void AdaParser::entry_declaration() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST entry_declaration_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  e = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST e_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	e = LT(1);
	if ( inputState->guessing == 0 ) {
		e_AST = astFactory->create(e);
		astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(e_AST));
	}
	match(ENTRY);
	RefAdaAST tmp143_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	if ( inputState->guessing == 0 ) {
		tmp143_AST = astFactory->create(LT(1));
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp143_AST));
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
#line 482 "ada.g"
		Set (e_AST, ENTRY_DECLARATION);
#line 4635 "AdaParser.cpp"
	}
	entry_declaration_AST = RefAdaAST(currentAST.root);
	returnAST = entry_declaration_AST;
}

void AdaParser::rep_spec() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST rep_spec_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  r = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST r_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
	returnAST = rep_spec_AST;
}

void AdaParser::discrete_subtype_def_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST discrete_subtype_def_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{
	bool synPredMatched148 = false;
	if (((LA(1) == LPAREN) && (_tokenSet_0.member(LA(2))))) {
		int _m148 = mark();
		synPredMatched148 = true;
		inputState->guessing++;
		try {
			{
			match(LPAREN);
			discrete_subtype_definition();
			match(RPAREN);
			}
		}
		catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
			synPredMatched148 = false;
		}
		rewind(_m148);
		inputState->guessing--;
	}
	if ( synPredMatched148 ) {
		match(LPAREN);
		discrete_subtype_definition();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(RPAREN);
	}
	else if ((LA(1) == SEMI || LA(1) == LPAREN) && (_tokenSet_14.member(LA(2)))) {
	}
	else {
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	
	}
	if ( inputState->guessing==0 ) {
		discrete_subtype_def_opt_AST = RefAdaAST(currentAST.root);
#line 489 "ada.g"
		discrete_subtype_def_opt_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(DISCRETE_SUBTYPE_DEF_OPT,"DISCRETE_SUBTYPE_DEF_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(discrete_subtype_def_opt_AST))));
#line 4712 "AdaParser.cpp"
		currentAST.root = discrete_subtype_def_opt_AST;
		if ( discrete_subtype_def_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			discrete_subtype_def_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = discrete_subtype_def_opt_AST->getFirstChild();
		else
			currentAST.child = discrete_subtype_def_opt_AST;
		currentAST.advanceChildToEnd();
	}
	discrete_subtype_def_opt_AST = RefAdaAST(currentAST.root);
	returnAST = discrete_subtype_def_opt_AST;
}

void AdaParser::discrete_subtype_definition() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST discrete_subtype_definition_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{
	bool synPredMatched152 = false;
	if (((_tokenSet_0.member(LA(1))) && (_tokenSet_8.member(LA(2))))) {
		int _m152 = mark();
		synPredMatched152 = true;
		inputState->guessing++;
		try {
			{
			range();
			}
		}
		catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
			synPredMatched152 = false;
		}
		rewind(_m152);
		inputState->guessing--;
	}
	if ( synPredMatched152 ) {
		range();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
	}
	else if ((LA(1) == IDENTIFIER) && (_tokenSet_15.member(LA(2)))) {
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
	returnAST = discrete_subtype_definition_AST;
}

void AdaParser::subtype_ind() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST subtype_ind_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 693 "ada.g"
		subtype_ind_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(SUBTYPE_INDICATION,"SUBTYPE_INDICATION")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(subtype_ind_AST))));
#line 4785 "AdaParser.cpp"
		currentAST.root = subtype_ind_AST;
		if ( subtype_ind_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			subtype_ind_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = subtype_ind_AST->getFirstChild();
		else
			currentAST.child = subtype_ind_AST;
		currentAST.advanceChildToEnd();
	}
	subtype_ind_AST = RefAdaAST(currentAST.root);
	returnAST = subtype_ind_AST;
}

void AdaParser::rep_spec_part(
	RefAdaAST t
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST rep_spec_part_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 512 "ada.g"
			Set(t, RECORD_REPRESENTATION_CLAUSE);
#line 4822 "AdaParser.cpp"
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
#line 514 "ada.g"
			Set(t, AT_CLAUSE);
#line 4837 "AdaParser.cpp"
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
#line 517 "ada.g"
			Set(t, ATTRIBUTE_DEFINITION_CLAUSE);
#line 4861 "AdaParser.cpp"
		}
		rep_spec_part_AST = RefAdaAST(currentAST.root);
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	returnAST = rep_spec_part_AST;
}

void AdaParser::align_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST align_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 521 "ada.g"
		align_opt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(MOD_CLAUSE_OPT,"MOD_CLAUSE_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(align_opt_AST))));
#line 4908 "AdaParser.cpp"
		currentAST.root = align_opt_AST;
		if ( align_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			align_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = align_opt_AST->getFirstChild();
		else
			currentAST.child = align_opt_AST;
		currentAST.advanceChildToEnd();
	}
	align_opt_AST = RefAdaAST(currentAST.root);
	returnAST = align_opt_AST;
}

void AdaParser::comp_loc_s() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST comp_loc_s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
			goto _loop159;
		}
		}
	}
	_loop159:;
	} // ( ... )*
	if ( inputState->guessing==0 ) {
		comp_loc_s_AST = RefAdaAST(currentAST.root);
#line 525 "ada.g"
		comp_loc_s_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(COMPONENT_CLAUSES_OPT,"COMPONENT_CLAUSES_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(comp_loc_s_AST))));
#line 4968 "AdaParser.cpp"
		currentAST.root = comp_loc_s_AST;
		if ( comp_loc_s_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			comp_loc_s_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = comp_loc_s_AST->getFirstChild();
		else
			currentAST.child = comp_loc_s_AST;
		currentAST.advanceChildToEnd();
	}
	comp_loc_s_AST = RefAdaAST(currentAST.root);
	returnAST = comp_loc_s_AST;
}

void AdaParser::protected_definition() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST protected_definition_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	match(IS);
	prot_op_decl_s();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	prot_private_opt();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	end_id_opt();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	protected_definition_AST = RefAdaAST(currentAST.root);
	returnAST = protected_definition_AST;
}

void AdaParser::prot_private_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST prot_private_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{
	switch ( LA(1)) {
	case PRIVATE:
	{
		match(PRIVATE);
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
				goto _loop168;
			}
			}
		}
		_loop168:;
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
	if ( inputState->guessing==0 ) {
		prot_private_opt_AST = RefAdaAST(currentAST.root);
#line 545 "ada.g"
		prot_private_opt_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(PROT_PRIVATE_OPT,"PROT_PRIVATE_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(prot_private_opt_AST))));
#line 5061 "AdaParser.cpp"
		currentAST.root = prot_private_opt_AST;
		if ( prot_private_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			prot_private_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = prot_private_opt_AST->getFirstChild();
		else
			currentAST.child = prot_private_opt_AST;
		currentAST.advanceChildToEnd();
	}
	prot_private_opt_AST = RefAdaAST(currentAST.root);
	returnAST = prot_private_opt_AST;
}

void AdaParser::prot_op_decl() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST prot_op_decl_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  p = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST p_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  f = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST f_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 561 "ada.g"
			pop_def_id(); Set(p_AST, PROCEDURE_DECLARATION);
#line 5113 "AdaParser.cpp"
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
#line 563 "ada.g"
			pop_def_id(); Set(f_AST, FUNCTION_DECLARATION);
#line 5138 "AdaParser.cpp"
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
	returnAST = prot_op_decl_AST;
}

void AdaParser::comp_decl() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST comp_decl_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 575 "ada.g"
		comp_decl_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(COMPONENT_DECLARATION,"COMPONENT_DECLARATION")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(comp_decl_AST))));
#line 5192 "AdaParser.cpp"
		currentAST.root = comp_decl_AST;
		if ( comp_decl_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			comp_decl_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = comp_decl_AST->getFirstChild();
		else
			currentAST.child = comp_decl_AST;
		currentAST.advanceChildToEnd();
	}
	comp_decl_AST = RefAdaAST(currentAST.root);
	returnAST = comp_decl_AST;
}

void AdaParser::prot_op_decl_s() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST prot_op_decl_s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{ // ( ... )*
	for (;;) {
		if ((_tokenSet_16.member(LA(1)))) {
			prot_op_decl();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
		}
		else {
			goto _loop172;
		}
		
	}
	_loop172:;
	} // ( ... )*
	if ( inputState->guessing==0 ) {
		prot_op_decl_s_AST = RefAdaAST(currentAST.root);
#line 555 "ada.g"
		prot_op_decl_s_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(PROT_OP_DECLARATIONS,"PROT_OP_DECLARATIONS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(prot_op_decl_s_AST))));
#line 5229 "AdaParser.cpp"
		currentAST.root = prot_op_decl_s_AST;
		if ( prot_op_decl_s_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			prot_op_decl_s_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = prot_op_decl_s_AST->getFirstChild();
		else
			currentAST.child = prot_op_decl_s_AST;
		currentAST.advanceChildToEnd();
	}
	prot_op_decl_s_AST = RefAdaAST(currentAST.root);
	returnAST = prot_op_decl_s_AST;
}

void AdaParser::prot_member_decl_s() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST prot_member_decl_s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
			goto _loop176;
		}
		}
	}
	_loop176:;
	} // ( ... )*
	if ( inputState->guessing==0 ) {
		prot_member_decl_s_AST = RefAdaAST(currentAST.root);
#line 569 "ada.g"
		prot_member_decl_s_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(PROT_MEMBER_DECLARATIONS,"PROT_MEMBER_DECLARATIONS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(prot_member_decl_s_AST))));
#line 5283 "AdaParser.cpp"
		currentAST.root = prot_member_decl_s_AST;
		if ( prot_member_decl_s_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			prot_member_decl_s_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = prot_member_decl_s_AST->getFirstChild();
		else
			currentAST.child = prot_member_decl_s_AST;
		currentAST.advanceChildToEnd();
	}
	prot_member_decl_s_AST = RefAdaAST(currentAST.root);
	returnAST = prot_member_decl_s_AST;
}

void AdaParser::component_subtype_def() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST component_subtype_def_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	aliased_opt();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	subtype_ind();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	component_subtype_def_AST = RefAdaAST(currentAST.root);
	returnAST = component_subtype_def_AST;
}

void AdaParser::type_def(
	RefAdaAST t
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST type_def_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 638 "ada.g"
			Set(t, ENUMERATION_TYPE_DECLARATION);
#line 5332 "AdaParser.cpp"
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
#line 640 "ada.g"
			Set(t, SIGNED_INTEGER_TYPE_DECLARATION);
#line 5347 "AdaParser.cpp"
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
#line 642 "ada.g"
			Set(t, MODULAR_TYPE_DECLARATION);
#line 5362 "AdaParser.cpp"
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
#line 644 "ada.g"
			Set(t, FLOATING_POINT_DECLARATION);
#line 5381 "AdaParser.cpp"
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
#line 647 "ada.g"
				Set(t, ORDINARY_FIXED_POINT_DECLARATION);
#line 5405 "AdaParser.cpp"
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
#line 649 "ada.g"
				Set(t, DECIMAL_FIXED_POINT_DECLARATION);
#line 5423 "AdaParser.cpp"
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
	returnAST = type_def_AST;
}

void AdaParser::derived_or_private_or_record(
	RefAdaAST t, boolean has_discrim
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST derived_or_private_or_record_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	bool synPredMatched246 = false;
	if (((LA(1) == NEW || LA(1) == ABSTRACT) && (LA(2) == IDENTIFIER || LA(2) == NEW))) {
		int _m246 = mark();
		synPredMatched246 = true;
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
			synPredMatched246 = false;
		}
		rewind(_m246);
		inputState->guessing--;
	}
	if ( synPredMatched246 ) {
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
#line 772 "ada.g"
				Set(t, PRIVATE_EXTENSION_DECLARATION);
#line 5542 "AdaParser.cpp"
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
#line 774 "ada.g"
				Set(t, DERIVED_RECORD_EXTENSION);
#line 5556 "AdaParser.cpp"
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
#line 776 "ada.g"
			Set(t, ORDINARY_DERIVED_TYPE_DECLARATION);
#line 5577 "AdaParser.cpp"
		}
		derived_or_private_or_record_AST = RefAdaAST(currentAST.root);
	}
	else if ((_tokenSet_17.member(LA(1))) && (_tokenSet_18.member(LA(2)))) {
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
#line 778 "ada.g"
				Set(t, PRIVATE_TYPE_DECLARATION);
#line 5594 "AdaParser.cpp"
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
#line 780 "ada.g"
				Set(t, RECORD_TYPE_DECLARATION);
#line 5608 "AdaParser.cpp"
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
	
	returnAST = derived_or_private_or_record_AST;
}

void AdaParser::local_enum_name() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST local_enum_name_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	RefAdaAST tmp177_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	if ( inputState->guessing == 0 ) {
		tmp177_AST = astFactory->create(LT(1));
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp177_AST));
	}
	match(IDENTIFIER);
	local_enum_name_AST = RefAdaAST(currentAST.root);
	returnAST = local_enum_name_AST;
}

void AdaParser::enumeration_aggregate() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST enumeration_aggregate_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	parenth_values();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	enumeration_aggregate_AST = RefAdaAST(currentAST.root);
	returnAST = enumeration_aggregate_AST;
}

void AdaParser::aliased_constant_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST aliased_constant_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{
	switch ( LA(1)) {
	case ALIASED:
	{
		RefAdaAST tmp178_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp178_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp178_AST));
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
		RefAdaAST tmp179_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp179_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp179_AST));
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
#line 858 "ada.g"
		aliased_constant_opt_AST =
			  RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(MODIFIERS,"MODIFIERS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(aliased_constant_opt_AST))));
#line 5712 "AdaParser.cpp"
		currentAST.root = aliased_constant_opt_AST;
		if ( aliased_constant_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			aliased_constant_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = aliased_constant_opt_AST->getFirstChild();
		else
			currentAST.child = aliased_constant_opt_AST;
		currentAST.advanceChildToEnd();
	}
	aliased_constant_opt_AST = RefAdaAST(currentAST.root);
	returnAST = aliased_constant_opt_AST;
}

void AdaParser::array_type_definition(
	RefAdaAST t
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST array_type_definition_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 669 "ada.g"
		Set(t, ARRAY_TYPE_DECLARATION);
#line 5747 "AdaParser.cpp"
	}
	array_type_definition_AST = RefAdaAST(currentAST.root);
	returnAST = array_type_definition_AST;
}

void AdaParser::enum_id_s() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST enum_id_s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
			goto _loop197;
		}
		
	}
	_loop197:;
	} // ( ... )*
	enum_id_s_AST = RefAdaAST(currentAST.root);
	returnAST = enum_id_s_AST;
}

void AdaParser::range_constraint_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST range_constraint_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
	returnAST = range_constraint_opt_AST;
}

void AdaParser::access_type_definition(
	RefAdaAST t
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST access_type_definition_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 751 "ada.g"
				Set(t, ACCESS_TO_PROCEDURE_DECLARATION);
#line 5846 "AdaParser.cpp"
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
#line 753 "ada.g"
				Set(t, ACCESS_TO_FUNCTION_DECLARATION);
#line 5865 "AdaParser.cpp"
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
#line 756 "ada.g"
			Set(t, ACCESS_TO_OBJECT_DECLARATION);
#line 5892 "AdaParser.cpp"
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
	returnAST = access_type_definition_AST;
}

void AdaParser::enumeration_literal_specification() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST enumeration_literal_specification_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	switch ( LA(1)) {
	case IDENTIFIER:
	{
		RefAdaAST tmp189_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp189_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp189_AST));
		}
		match(IDENTIFIER);
		enumeration_literal_specification_AST = RefAdaAST(currentAST.root);
		break;
	}
	case CHARACTER_LITERAL:
	{
		RefAdaAST tmp190_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp190_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp190_AST));
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
	returnAST = enumeration_literal_specification_AST;
}

void AdaParser::index_or_discrete_range_s() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST index_or_discrete_range_s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	index_or_discrete_range();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	{ // ( ... )*
	for (;;) {
		if ((LA(1) == COMMA)) {
			RefAdaAST tmp191_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp191_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp191_AST));
			}
			match(COMMA);
			index_or_discrete_range();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
		}
		else {
			goto _loop204;
		}
		
	}
	_loop204:;
	} // ( ... )*
	index_or_discrete_range_s_AST = RefAdaAST(currentAST.root);
	returnAST = index_or_discrete_range_s_AST;
}

void AdaParser::index_or_discrete_range() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST index_or_discrete_range_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	simple_expression();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	{
	switch ( LA(1)) {
	case DOT_DOT:
	{
		RefAdaAST tmp192_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp192_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp192_AST));
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
		RefAdaAST tmp193_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp193_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp193_AST));
		}
		match(RANGE);
		{
		switch ( LA(1)) {
		case BOX:
		{
			RefAdaAST tmp194_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp194_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp194_AST));
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
	returnAST = index_or_discrete_range_AST;
}

void AdaParser::aliased_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST aliased_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{
	switch ( LA(1)) {
	case ALIASED:
	{
		RefAdaAST tmp195_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp195_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp195_AST));
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
#line 689 "ada.g"
		aliased_opt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(MODIFIERS,"MODIFIERS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(aliased_opt_AST))));
#line 6093 "AdaParser.cpp"
		currentAST.root = aliased_opt_AST;
		if ( aliased_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			aliased_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = aliased_opt_AST->getFirstChild();
		else
			currentAST.child = aliased_opt_AST;
		currentAST.advanceChildToEnd();
	}
	aliased_opt_AST = RefAdaAST(currentAST.root);
	returnAST = aliased_opt_AST;
}

void AdaParser::constraint_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST constraint_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
		bool synPredMatched215 = false;
		if (((LA(1) == LPAREN) && (_tokenSet_0.member(LA(2))))) {
			int _m215 = mark();
			synPredMatched215 = true;
			inputState->guessing++;
			try {
				{
				index_constraint();
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched215 = false;
			}
			rewind(_m215);
			inputState->guessing--;
		}
		if ( synPredMatched215 ) {
			index_constraint();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
		}
		else if ((LA(1) == LPAREN) && (_tokenSet_0.member(LA(2)))) {
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
	returnAST = constraint_opt_AST;
}

void AdaParser::digits_constraint() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST digits_constraint_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  d = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST d_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 706 "ada.g"
		Set(d_AST, DIGITS_CONSTRAINT);
#line 6208 "AdaParser.cpp"
	}
	digits_constraint_AST = RefAdaAST(currentAST.root);
	returnAST = digits_constraint_AST;
}

void AdaParser::delta_constraint() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST delta_constraint_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  d = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST d_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 710 "ada.g"
		Set(d_AST, DELTA_CONSTRAINT);
#line 6238 "AdaParser.cpp"
	}
	delta_constraint_AST = RefAdaAST(currentAST.root);
	returnAST = delta_constraint_AST;
}

void AdaParser::index_constraint() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST index_constraint_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  p = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST p_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
			goto _loop220;
		}
		
	}
	_loop220:;
	} // ( ... )*
	match(RPAREN);
	if ( inputState->guessing==0 ) {
#line 714 "ada.g"
		Set(p_AST, INDEX_CONSTRAINT);
#line 6281 "AdaParser.cpp"
	}
	index_constraint_AST = RefAdaAST(currentAST.root);
	returnAST = index_constraint_AST;
}

void AdaParser::discriminant_constraint() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST discriminant_constraint_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  p = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST p_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
			goto _loop226;
		}
		
	}
	_loop226:;
	} // ( ... )*
	match(RPAREN);
	if ( inputState->guessing==0 ) {
#line 724 "ada.g"
		Set(p_AST, DISCRIMINANT_CONSTRAINT);
#line 6324 "AdaParser.cpp"
	}
	discriminant_constraint_AST = RefAdaAST(currentAST.root);
	returnAST = discriminant_constraint_AST;
}

void AdaParser::discrete_range() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST discrete_range_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	bool synPredMatched223 = false;
	if (((_tokenSet_0.member(LA(1))) && (_tokenSet_8.member(LA(2))))) {
		int _m223 = mark();
		synPredMatched223 = true;
		inputState->guessing++;
		try {
			{
			range();
			}
		}
		catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
			synPredMatched223 = false;
		}
		rewind(_m223);
		inputState->guessing--;
	}
	if ( synPredMatched223 ) {
		range();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		discrete_range_AST = RefAdaAST(currentAST.root);
	}
	else if ((LA(1) == IDENTIFIER) && (_tokenSet_19.member(LA(2)))) {
		subtype_ind();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		discrete_range_AST = RefAdaAST(currentAST.root);
	}
	else {
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	
	returnAST = discrete_range_AST;
}

void AdaParser::discriminant_association() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST discriminant_association_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 728 "ada.g"
		discriminant_association_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(DISCRIMINANT_ASSOCIATION,"DISCRIMINANT_ASSOCIATION")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(discriminant_association_AST))));
#line 6390 "AdaParser.cpp"
		currentAST.root = discriminant_association_AST;
		if ( discriminant_association_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			discriminant_association_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = discriminant_association_AST->getFirstChild();
		else
			currentAST.child = discriminant_association_AST;
		currentAST.advanceChildToEnd();
	}
	discriminant_association_AST = RefAdaAST(currentAST.root);
	returnAST = discriminant_association_AST;
}

void AdaParser::selector_names_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST selector_names_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{
	bool synPredMatched231 = false;
	if (((LA(1) == IDENTIFIER) && (LA(2) == RIGHT_SHAFT || LA(2) == PIPE))) {
		int _m231 = mark();
		synPredMatched231 = true;
		inputState->guessing++;
		try {
			{
			association_head();
			}
		}
		catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
			synPredMatched231 = false;
		}
		rewind(_m231);
		inputState->guessing--;
	}
	if ( synPredMatched231 ) {
		association_head();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
	}
	else if ((_tokenSet_0.member(LA(1))) && (_tokenSet_1.member(LA(2)))) {
	}
	else {
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	
	}
	if ( inputState->guessing==0 ) {
		selector_names_opt_AST = RefAdaAST(currentAST.root);
#line 736 "ada.g"
		selector_names_opt_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(SELECTOR_NAMES_OPT,"SELECTOR_NAMES_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(selector_names_opt_AST))));
#line 6443 "AdaParser.cpp"
		currentAST.root = selector_names_opt_AST;
		if ( selector_names_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			selector_names_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = selector_names_opt_AST->getFirstChild();
		else
			currentAST.child = selector_names_opt_AST;
		currentAST.advanceChildToEnd();
	}
	selector_names_opt_AST = RefAdaAST(currentAST.root);
	returnAST = selector_names_opt_AST;
}

void AdaParser::association_head() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST association_head_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
			goto _loop234;
		}
		
	}
	_loop234:;
	} // ( ... )*
	match(RIGHT_SHAFT);
	association_head_AST = RefAdaAST(currentAST.root);
	returnAST = association_head_AST;
}

void AdaParser::selector_name() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST selector_name_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	RefAdaAST tmp202_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	if ( inputState->guessing == 0 ) {
		tmp202_AST = astFactory->create(LT(1));
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp202_AST));
	}
	match(IDENTIFIER);
	selector_name_AST = RefAdaAST(currentAST.root);
	returnAST = selector_name_AST;
}

void AdaParser::protected_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST protected_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{
	switch ( LA(1)) {
	case PROTECTED:
	{
		RefAdaAST tmp203_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp203_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp203_AST));
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
#line 761 "ada.g"
		protected_opt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(MODIFIERS,"MODIFIERS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(protected_opt_AST))));
#line 6533 "AdaParser.cpp"
		currentAST.root = protected_opt_AST;
		if ( protected_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			protected_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = protected_opt_AST->getFirstChild();
		else
			currentAST.child = protected_opt_AST;
		currentAST.advanceChildToEnd();
	}
	protected_opt_AST = RefAdaAST(currentAST.root);
	returnAST = protected_opt_AST;
}

void AdaParser::constant_all_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST constant_all_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{
	switch ( LA(1)) {
	case CONSTANT:
	{
		RefAdaAST tmp204_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp204_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp204_AST));
		}
		match(CONSTANT);
		break;
	}
	case ALL:
	{
		RefAdaAST tmp205_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp205_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp205_AST));
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
#line 765 "ada.g"
		constant_all_opt_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(MODIFIERS,"MODIFIERS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(constant_all_opt_AST))));
#line 6588 "AdaParser.cpp"
		currentAST.root = constant_all_opt_AST;
		if ( constant_all_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			constant_all_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = constant_all_opt_AST->getFirstChild();
		else
			currentAST.child = constant_all_opt_AST;
		currentAST.advanceChildToEnd();
	}
	constant_all_opt_AST = RefAdaAST(currentAST.root);
	returnAST = constant_all_opt_AST;
}

void AdaParser::abstract_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST abstract_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{
	switch ( LA(1)) {
	case ABSTRACT:
	{
		RefAdaAST tmp206_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp206_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp206_AST));
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
#line 785 "ada.g"
		abstract_opt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(MODIFIERS,"MODIFIERS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(abstract_opt_AST))));
#line 6632 "AdaParser.cpp"
		currentAST.root = abstract_opt_AST;
		if ( abstract_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			abstract_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = abstract_opt_AST->getFirstChild();
		else
			currentAST.child = abstract_opt_AST;
		currentAST.advanceChildToEnd();
	}
	abstract_opt_AST = RefAdaAST(currentAST.root);
	returnAST = abstract_opt_AST;
}

void AdaParser::record_definition(
	boolean has_discrim
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST record_definition_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
	returnAST = record_definition_AST;
}

void AdaParser::abstract_tagged_limited_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST abstract_tagged_limited_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{
	switch ( LA(1)) {
	case ABSTRACT:
	{
		RefAdaAST tmp212_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp212_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp212_AST));
		}
		match(ABSTRACT);
		match(TAGGED);
		break;
	}
	case TAGGED:
	{
		RefAdaAST tmp214_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp214_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp214_AST));
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
		RefAdaAST tmp215_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp215_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp215_AST));
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
#line 847 "ada.g"
		abstract_tagged_limited_opt_AST =
			  RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(MODIFIERS,"MODIFIERS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(abstract_tagged_limited_opt_AST))));
#line 6750 "AdaParser.cpp"
		currentAST.root = abstract_tagged_limited_opt_AST;
		if ( abstract_tagged_limited_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			abstract_tagged_limited_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = abstract_tagged_limited_opt_AST->getFirstChild();
		else
			currentAST.child = abstract_tagged_limited_opt_AST;
		currentAST.advanceChildToEnd();
	}
	abstract_tagged_limited_opt_AST = RefAdaAST(currentAST.root);
	returnAST = abstract_tagged_limited_opt_AST;
}

void AdaParser::component_list(
	boolean has_discrim
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST component_list_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
	returnAST = component_list_AST;
}

void AdaParser::component_items() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST component_items_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{ // ( ... )+
	int _cnt256=0;
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
			if ( _cnt256>=1 ) { goto _loop256; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
		}
		}
		_cnt256++;
	}
	_loop256:;
	}  // ( ... )+
	if ( inputState->guessing==0 ) {
		component_items_AST = RefAdaAST(currentAST.root);
#line 800 "ada.g"
		component_items_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(COMPONENT_ITEMS,"COMPONENT_ITEMS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(component_items_AST))));
#line 6873 "AdaParser.cpp"
		currentAST.root = component_items_AST;
		if ( component_items_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			component_items_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = component_items_AST->getFirstChild();
		else
			currentAST.child = component_items_AST;
		currentAST.advanceChildToEnd();
	}
	component_items_AST = RefAdaAST(currentAST.root);
	returnAST = component_items_AST;
}

void AdaParser::variant_part() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST variant_part_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  c = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST c_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 812 "ada.g"
		Set (c_AST, VARIANT_PART);
#line 6914 "AdaParser.cpp"
	}
	variant_part_AST = RefAdaAST(currentAST.root);
	returnAST = variant_part_AST;
}

void AdaParser::empty_component_items() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST empty_component_items_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	if ( inputState->guessing==0 ) {
		empty_component_items_AST = RefAdaAST(currentAST.root);
#line 806 "ada.g"
		empty_component_items_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(COMPONENT_ITEMS,"COMPONENT_ITEMS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(empty_component_items_AST))));
#line 6930 "AdaParser.cpp"
		currentAST.root = empty_component_items_AST;
		if ( empty_component_items_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			empty_component_items_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = empty_component_items_AST->getFirstChild();
		else
			currentAST.child = empty_component_items_AST;
		currentAST.advanceChildToEnd();
	}
	empty_component_items_AST = RefAdaAST(currentAST.root);
	returnAST = empty_component_items_AST;
}

void AdaParser::discriminant_direct_name() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST discriminant_direct_name_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	RefAdaAST tmp222_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	if ( inputState->guessing == 0 ) {
		tmp222_AST = astFactory->create(LT(1));
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp222_AST));
	}
	match(IDENTIFIER);
	discriminant_direct_name_AST = RefAdaAST(currentAST.root);
	returnAST = discriminant_direct_name_AST;
}

void AdaParser::variant_s() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST variant_s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{ // ( ... )+
	int _cnt262=0;
	for (;;) {
		if ((LA(1) == WHEN)) {
			variant();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
		}
		else {
			if ( _cnt262>=1 ) { goto _loop262; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
		}
		
		_cnt262++;
	}
	_loop262:;
	}  // ( ... )+
	if ( inputState->guessing==0 ) {
		variant_s_AST = RefAdaAST(currentAST.root);
#line 819 "ada.g"
		variant_s_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(VARIANTS,"VARIANTS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(variant_s_AST))));
#line 6984 "AdaParser.cpp"
		currentAST.root = variant_s_AST;
		if ( variant_s_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			variant_s_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = variant_s_AST->getFirstChild();
		else
			currentAST.child = variant_s_AST;
		currentAST.advanceChildToEnd();
	}
	variant_s_AST = RefAdaAST(currentAST.root);
	returnAST = variant_s_AST;
}

void AdaParser::variant() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST variant_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  w = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST w_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 823 "ada.g"
		Set (w_AST, VARIANT);
#line 7022 "AdaParser.cpp"
	}
	variant_AST = RefAdaAST(currentAST.root);
	returnAST = variant_AST;
}

void AdaParser::choice_s() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST choice_s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	choice();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	{ // ( ... )*
	for (;;) {
		if ((LA(1) == PIPE)) {
			RefAdaAST tmp224_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp224_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp224_AST));
			}
			match(PIPE);
			choice();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
		}
		else {
			goto _loop266;
		}
		
	}
	_loop266:;
	} // ( ... )*
	choice_s_AST = RefAdaAST(currentAST.root);
	returnAST = choice_s_AST;
}

void AdaParser::choice() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST choice_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	if ((LA(1) == OTHERS)) {
		RefAdaAST tmp225_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp225_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp225_AST));
		}
		match(OTHERS);
		choice_AST = RefAdaAST(currentAST.root);
	}
	else {
		bool synPredMatched269 = false;
		if (((_tokenSet_0.member(LA(1))) && (_tokenSet_20.member(LA(2))))) {
			int _m269 = mark();
			synPredMatched269 = true;
			inputState->guessing++;
			try {
				{
				discrete_with_range();
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched269 = false;
			}
			rewind(_m269);
			inputState->guessing--;
		}
		if ( synPredMatched269 ) {
			discrete_with_range();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
			choice_AST = RefAdaAST(currentAST.root);
		}
		else if ((_tokenSet_0.member(LA(1))) && (_tokenSet_21.member(LA(2)))) {
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
	returnAST = choice_AST;
}

void AdaParser::discrete_with_range() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST discrete_with_range_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	bool synPredMatched272 = false;
	if (((LA(1) == IDENTIFIER) && (LA(2) == DOT || LA(2) == TIC || LA(2) == RANGE))) {
		int _m272 = mark();
		synPredMatched272 = true;
		inputState->guessing++;
		try {
			{
			mark_with_constraint();
			}
		}
		catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
			synPredMatched272 = false;
		}
		rewind(_m272);
		inputState->guessing--;
	}
	if ( synPredMatched272 ) {
		mark_with_constraint();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		discrete_with_range_AST = RefAdaAST(currentAST.root);
	}
	else if ((_tokenSet_0.member(LA(1))) && (_tokenSet_8.member(LA(2)))) {
		range();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		discrete_with_range_AST = RefAdaAST(currentAST.root);
	}
	else {
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	
	returnAST = discrete_with_range_AST;
}

void AdaParser::mark_with_constraint() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST mark_with_constraint_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 839 "ada.g"
		mark_with_constraint_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(MARK_WITH_CONSTRAINT,"MARK_WITH_CONSTRAINT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(mark_with_constraint_AST))));
#line 7174 "AdaParser.cpp"
		currentAST.root = mark_with_constraint_AST;
		if ( mark_with_constraint_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			mark_with_constraint_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = mark_with_constraint_AST->getFirstChild();
		else
			currentAST.child = mark_with_constraint_AST;
		currentAST.advanceChildToEnd();
	}
	mark_with_constraint_AST = RefAdaAST(currentAST.root);
	returnAST = mark_with_constraint_AST;
}

void AdaParser::generic_formal_part_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST generic_formal_part_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
			goto _loop289;
		}
		}
	}
	_loop289:;
	} // ( ... )*
	if ( inputState->guessing==0 ) {
		generic_formal_part_opt_AST = RefAdaAST(currentAST.root);
#line 885 "ada.g"
		generic_formal_part_opt_AST =
					RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(GENERIC_FORMAL_PART,"GENERIC_FORMAL_PART")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(generic_formal_part_opt_AST))));
#line 7234 "AdaParser.cpp"
		currentAST.root = generic_formal_part_opt_AST;
		if ( generic_formal_part_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			generic_formal_part_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = generic_formal_part_opt_AST->getFirstChild();
		else
			currentAST.child = generic_formal_part_opt_AST;
		currentAST.advanceChildToEnd();
	}
	generic_formal_part_opt_AST = RefAdaAST(currentAST.root);
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
#line 895 "ada.g"
					Set (t_AST, FORMAL_DISCRETE_TYPE_DECLARATION);
#line 7285 "AdaParser.cpp"
				}
				break;
			}
			case RANGE:
			{
				match(RANGE);
				match(BOX);
				if ( inputState->guessing==0 ) {
#line 897 "ada.g"
					Set (t_AST, FORMAL_SIGNED_INTEGER_TYPE_DECLARATION);
#line 7296 "AdaParser.cpp"
				}
				break;
			}
			case MOD:
			{
				match(MOD);
				match(BOX);
				if ( inputState->guessing==0 ) {
#line 899 "ada.g"
					Set (t_AST, FORMAL_MODULAR_TYPE_DECLARATION);
#line 7307 "AdaParser.cpp"
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
#line 902 "ada.g"
						Set (t_AST, FORMAL_DECIMAL_FIXED_POINT_DECLARATION);
#line 7324 "AdaParser.cpp"
					}
					break;
				}
				case SEMI:
				{
					if ( inputState->guessing==0 ) {
#line 903 "ada.g"
						Set (t_AST, FORMAL_ORDINARY_FIXED_POINT_DECLARATION);
#line 7333 "AdaParser.cpp"
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
#line 906 "ada.g"
					Set (t_AST, FORMAL_FLOATING_POINT_DECLARATION);
#line 7352 "AdaParser.cpp"
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
#line 913 "ada.g"
			pop_def_id();
#line 7418 "AdaParser.cpp"
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
#line 915 "ada.g"
				Set(w_AST, FORMAL_PROCEDURE_DECLARATION);
#line 7450 "AdaParser.cpp"
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
#line 917 "ada.g"
				Set(w_AST, FORMAL_FUNCTION_DECLARATION);
#line 7472 "AdaParser.cpp"
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
#line 919 "ada.g"
				Set(w_AST, FORMAL_PACKAGE_DECLARATION);
#line 7496 "AdaParser.cpp"
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
#line 921 "ada.g"
			pop_def_id();
#line 7509 "AdaParser.cpp"
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
	returnAST = generic_formal_parameter_AST;
}

void AdaParser::discriminable_type_definition(
	RefAdaAST t
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST discriminable_type_definition_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	bool synPredMatched299 = false;
	if (((LA(1) == NEW || LA(1) == ABSTRACT) && (LA(2) == IDENTIFIER || LA(2) == NEW))) {
		int _m299 = mark();
		synPredMatched299 = true;
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
			synPredMatched299 = false;
		}
		rewind(_m299);
		inputState->guessing--;
	}
	if ( synPredMatched299 ) {
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
#line 930 "ada.g"
			Set (t, FORMAL_PRIVATE_EXTENSION_DECLARATION);
#line 7589 "AdaParser.cpp"
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
#line 932 "ada.g"
			Set (t, FORMAL_ORDINARY_DERIVED_TYPE_DECLARATION);
#line 7602 "AdaParser.cpp"
		}
		discriminable_type_definition_AST = RefAdaAST(currentAST.root);
	}
	else if ((_tokenSet_22.member(LA(1))) && (_tokenSet_23.member(LA(2)))) {
		abstract_tagged_limited_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(PRIVATE);
		if ( inputState->guessing==0 ) {
#line 934 "ada.g"
			Set (t, FORMAL_PRIVATE_TYPE_DECLARATION);
#line 7615 "AdaParser.cpp"
		}
		discriminable_type_definition_AST = RefAdaAST(currentAST.root);
	}
	else {
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	
	returnAST = discriminable_type_definition_AST;
}

void AdaParser::subprogram_default_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST subprogram_default_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{
	switch ( LA(1)) {
	case IS:
	{
		match(IS);
		{
		switch ( LA(1)) {
		case BOX:
		{
			RefAdaAST tmp253_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp253_AST = astFactory->create(LT(1));
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp253_AST));
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
	returnAST = subprogram_default_opt_AST;
}

void AdaParser::formal_package_actual_part_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST formal_package_actual_part_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{
	switch ( LA(1)) {
	case LPAREN:
	{
		match(LPAREN);
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
	returnAST = formal_package_actual_part_opt_AST;
}

void AdaParser::body_part() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST body_part_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	declarative_part();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	block_body();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	end_id_opt();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	body_part_AST = RefAdaAST(currentAST.root);
	returnAST = body_part_AST;
}

void AdaParser::declarative_part() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST declarative_part_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
			goto _loop316;
		}
		}
	}
	_loop316:;
	} // ( ... )*
	if ( inputState->guessing==0 ) {
		declarative_part_AST = RefAdaAST(currentAST.root);
#line 978 "ada.g"
		declarative_part_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(DECLARATIVE_PART,"DECLARATIVE_PART")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(declarative_part_AST))));
#line 7799 "AdaParser.cpp"
		currentAST.root = declarative_part_AST;
		if ( declarative_part_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			declarative_part_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = declarative_part_AST->getFirstChild();
		else
			currentAST.child = declarative_part_AST;
		currentAST.advanceChildToEnd();
	}
	declarative_part_AST = RefAdaAST(currentAST.root);
	returnAST = declarative_part_AST;
}

void AdaParser::block_body() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST block_body_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  b = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST b_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 1058 "ada.g"
		Set(b_AST, BLOCK_BODY);
#line 7832 "AdaParser.cpp"
	}
	block_body_AST = RefAdaAST(currentAST.root);
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
#line 986 "ada.g"
					Set(pkg_AST, PACKAGE_BODY_STUB);
#line 7878 "AdaParser.cpp"
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
				if ( inputState->guessing==0 ) {
#line 988 "ada.g"
					Set(pkg_AST, PACKAGE_BODY);
#line 7904 "AdaParser.cpp"
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
#line 994 "ada.g"
					Set(tsk_AST, TASK_BODY_STUB);
#line 7964 "AdaParser.cpp"
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
#line 995 "ada.g"
					Set(tsk_AST, TASK_BODY);
#line 7989 "AdaParser.cpp"
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
#line 1002 "ada.g"
					Set(pro_AST, PROTECTED_BODY_STUB);
#line 8046 "AdaParser.cpp"
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
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
				if ( inputState->guessing==0 ) {
#line 1004 "ada.g"
					Set(pro_AST, PROTECTED_BODY);
#line 8067 "AdaParser.cpp"
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
	returnAST = declarative_item_AST;
}

void AdaParser::body_is() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST body_is_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	match(BODY);
	def_id(false);
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	match(IS);
	body_is_AST = RefAdaAST(currentAST.root);
	returnAST = body_is_AST;
}

void AdaParser::separate() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST separate_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	match(SEPARATE);
	if ( inputState->guessing==0 ) {
#line 1023 "ada.g"
		pop_def_id();
#line 8153 "AdaParser.cpp"
	}
	separate_AST = RefAdaAST(currentAST.root);
	returnAST = separate_AST;
}

void AdaParser::prot_op_bodies_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST prot_op_bodies_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
			goto _loop332;
		}
		}
	}
	_loop332:;
	} // ( ... )*
	if ( inputState->guessing==0 ) {
		prot_op_bodies_opt_AST = RefAdaAST(currentAST.root);
#line 1039 "ada.g"
		prot_op_bodies_opt_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(PROT_OP_BODIES_OPT,"PROT_OP_BODIES_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(prot_op_bodies_opt_AST))));
#line 8205 "AdaParser.cpp"
		currentAST.root = prot_op_bodies_opt_AST;
		if ( prot_op_bodies_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			prot_op_bodies_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = prot_op_bodies_opt_AST->getFirstChild();
		else
			currentAST.child = prot_op_bodies_opt_AST;
		currentAST.advanceChildToEnd();
	}
	prot_op_bodies_opt_AST = RefAdaAST(currentAST.root);
	returnAST = prot_op_bodies_opt_AST;
}

void AdaParser::block_body_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST block_body_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 1030 "ada.g"
		block_body_opt_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(BLOCK_BODY_OPT,"BLOCK_BODY_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(block_body_opt_AST))));
#line 8249 "AdaParser.cpp"
		currentAST.root = block_body_opt_AST;
		if ( block_body_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			block_body_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = block_body_opt_AST->getFirstChild();
		else
			currentAST.child = block_body_opt_AST;
		currentAST.advanceChildToEnd();
	}
	block_body_opt_AST = RefAdaAST(currentAST.root);
	returnAST = block_body_opt_AST;
}

void AdaParser::handled_stmt_s() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST handled_stmt_s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 1062 "ada.g"
		handled_stmt_s_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(HANDLED_SEQUENCE_OF_STATEMENTS,"HANDLED_SEQUENCE_OF_STATEMENTS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(handled_stmt_s_AST))));
#line 8280 "AdaParser.cpp"
		currentAST.root = handled_stmt_s_AST;
		if ( handled_stmt_s_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			handled_stmt_s_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = handled_stmt_s_AST->getFirstChild();
		else
			currentAST.child = handled_stmt_s_AST;
		currentAST.advanceChildToEnd();
	}
	handled_stmt_s_AST = RefAdaAST(currentAST.root);
	returnAST = handled_stmt_s_AST;
}

void AdaParser::entry_body() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST entry_body_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  e = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST e_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 1236 "ada.g"
		Set (e_AST, ENTRY_BODY);
#line 8327 "AdaParser.cpp"
	}
	entry_body_AST = RefAdaAST(currentAST.root);
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
#line 1046 "ada.g"
				Set(p_AST, PROCEDURE_BODY);
#line 8371 "AdaParser.cpp"
			}
			break;
		}
		case SEMI:
		{
			if ( inputState->guessing==0 ) {
#line 1047 "ada.g"
				pop_def_id(); Set(p_AST, PROCEDURE_DECLARATION);
#line 8380 "AdaParser.cpp"
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
#line 1051 "ada.g"
				Set(f_AST, FUNCTION_BODY);
#line 8422 "AdaParser.cpp"
			}
			break;
		}
		case SEMI:
		{
			if ( inputState->guessing==0 ) {
#line 1052 "ada.g"
				pop_def_id(); Set(f_AST, FUNCTION_DECLARATION);
#line 8431 "AdaParser.cpp"
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
	returnAST = subprog_decl_or_body_AST;
}

void AdaParser::statements() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST statements_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{ // ( ... )+
	int _cnt342=0;
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
			if ( _cnt342>=1 ) { goto _loop342; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
		}
		}
		_cnt342++;
	}
	_loop342:;
	}  // ( ... )+
	if ( inputState->guessing==0 ) {
		statements_AST = RefAdaAST(currentAST.root);
#line 1074 "ada.g"
		statements_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(SEQUENCE_OF_STATEMENTS,"SEQUENCE_OF_STATEMENTS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(statements_AST))));
#line 8509 "AdaParser.cpp"
		currentAST.root = statements_AST;
		if ( statements_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			statements_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = statements_AST->getFirstChild();
		else
			currentAST.child = statements_AST;
		currentAST.advanceChildToEnd();
	}
	statements_AST = RefAdaAST(currentAST.root);
	returnAST = statements_AST;
}

void AdaParser::except_handler_part_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST except_handler_part_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{
	switch ( LA(1)) {
	case EXCEPTION:
	{
		match(EXCEPTION);
		{ // ( ... )+
		int _cnt429=0;
		for (;;) {
			if ((LA(1) == WHEN)) {
				exception_handler();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
				}
			}
			else {
				if ( _cnt429>=1 ) { goto _loop429; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
			}
			
			_cnt429++;
		}
		_loop429:;
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
#line 1365 "ada.g"
		except_handler_part_opt_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(EXCEPT_HANDLER_PART_OPT,"EXCEPT_HANDLER_PART_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(except_handler_part_opt_AST))));
#line 8566 "AdaParser.cpp"
		currentAST.root = except_handler_part_opt_AST;
		if ( except_handler_part_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			except_handler_part_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = except_handler_part_opt_AST->getFirstChild();
		else
			currentAST.child = except_handler_part_opt_AST;
		currentAST.advanceChildToEnd();
	}
	except_handler_part_opt_AST = RefAdaAST(currentAST.root);
	returnAST = except_handler_part_opt_AST;
}

void AdaParser::handled_stmts_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST handled_stmts_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{
	switch ( LA(1)) {
	case PRAGMA:
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
		statements();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		except_handler_part_opt();
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
		handled_stmts_opt_AST = RefAdaAST(currentAST.root);
#line 1068 "ada.g"
		handled_stmts_opt_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(HANDLED_STMTS_OPT,"HANDLED_STMTS_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(handled_stmts_opt_AST))));
#line 8632 "AdaParser.cpp"
		currentAST.root = handled_stmts_opt_AST;
		if ( handled_stmts_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			handled_stmts_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = handled_stmts_opt_AST->getFirstChild();
		else
			currentAST.child = handled_stmts_opt_AST;
		currentAST.advanceChildToEnd();
	}
	handled_stmts_opt_AST = RefAdaAST(currentAST.root);
	returnAST = handled_stmts_opt_AST;
}

void AdaParser::statement() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST statement_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
				end_id_opt();
				if (inputState->guessing==0) {
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
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
		}
		else if ((LA(1) == IDENTIFIER) && (_tokenSet_24.member(LA(2)))) {
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
#line 1100 "ada.g"
		statement_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(STATEMENT,"STATEMENT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(statement_AST))));
#line 8833 "AdaParser.cpp"
		currentAST.root = statement_AST;
		if ( statement_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			statement_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = statement_AST->getFirstChild();
		else
			currentAST.child = statement_AST;
		currentAST.advanceChildToEnd();
	}
	statement_AST = RefAdaAST(currentAST.root);
	returnAST = statement_AST;
}

void AdaParser::def_label_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST def_label_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{
	switch ( LA(1)) {
	case LT_LT:
	{
		match(LT_LT);
		RefAdaAST tmp277_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp277_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp277_AST));
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
#line 1104 "ada.g"
		def_label_opt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(LABEL_OPT,"LABEL_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(def_label_opt_AST))));
#line 8896 "AdaParser.cpp"
		currentAST.root = def_label_opt_AST;
		if ( def_label_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			def_label_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = def_label_opt_AST->getFirstChild();
		else
			currentAST.child = def_label_opt_AST;
		currentAST.advanceChildToEnd();
	}
	def_label_opt_AST = RefAdaAST(currentAST.root);
	returnAST = def_label_opt_AST;
}

void AdaParser::null_stmt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST null_stmt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  s = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	s = LT(1);
	if ( inputState->guessing == 0 ) {
		s_AST = astFactory->create(s);
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(s_AST));
	}
	match(NuLL);
	match(SEMI);
	if ( inputState->guessing==0 ) {
#line 1108 "ada.g"
		Set(s_AST, NULL_STATEMENT);
#line 8926 "AdaParser.cpp"
	}
	null_stmt_AST = RefAdaAST(currentAST.root);
	returnAST = null_stmt_AST;
}

void AdaParser::exit_stmt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST exit_stmt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  s = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
		RefAdaAST tmp280_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp280_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp280_AST));
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
#line 1206 "ada.g"
		Set(s_AST, EXIT_STATEMENT);
#line 8996 "AdaParser.cpp"
	}
	exit_stmt_AST = RefAdaAST(currentAST.root);
	returnAST = exit_stmt_AST;
}

void AdaParser::return_stmt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST return_stmt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  s = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 1213 "ada.g"
		Set(s_AST, RETURN_STATEMENT);
#line 9049 "AdaParser.cpp"
	}
	return_stmt_AST = RefAdaAST(currentAST.root);
	returnAST = return_stmt_AST;
}

void AdaParser::goto_stmt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST goto_stmt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  s = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 1217 "ada.g"
		Set(s_AST, GOTO_STATEMENT);
#line 9076 "AdaParser.cpp"
	}
	goto_stmt_AST = RefAdaAST(currentAST.root);
	returnAST = goto_stmt_AST;
}

void AdaParser::delay_stmt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST delay_stmt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  d = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST d_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 1282 "ada.g"
		Set (d_AST, DELAY_STATEMENT);
#line 9107 "AdaParser.cpp"
	}
	delay_stmt_AST = RefAdaAST(currentAST.root);
	returnAST = delay_stmt_AST;
}

void AdaParser::abort_stmt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST abort_stmt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  a = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST a_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
			goto _loop425;
		}
		
	}
	_loop425:;
	} // ( ... )*
	match(SEMI);
	if ( inputState->guessing==0 ) {
#line 1361 "ada.g"
		Set (a_AST, ABORT_STATEMENT);
#line 9150 "AdaParser.cpp"
	}
	abort_stmt_AST = RefAdaAST(currentAST.root);
	returnAST = abort_stmt_AST;
}

void AdaParser::raise_stmt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST raise_stmt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  r = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST r_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 1389 "ada.g"
		Set (r_AST, RAISE_STATEMENT);
#line 9193 "AdaParser.cpp"
	}
	raise_stmt_AST = RefAdaAST(currentAST.root);
	returnAST = raise_stmt_AST;
}

void AdaParser::requeue_stmt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST requeue_stmt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  r = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST r_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
		RefAdaAST tmp289_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp289_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp289_AST));
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
#line 1393 "ada.g"
		Set (r_AST, REQUEUE_STATEMENT);
#line 9243 "AdaParser.cpp"
	}
	requeue_stmt_AST = RefAdaAST(currentAST.root);
	returnAST = requeue_stmt_AST;
}

void AdaParser::accept_stmt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST accept_stmt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  a = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST a_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
		handled_stmts_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		end_id_opt();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(SEMI);
		break;
	}
	case SEMI:
	{
		match(SEMI);
		if ( inputState->guessing==0 ) {
#line 1264 "ada.g"
			pop_def_id();
#line 9296 "AdaParser.cpp"
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
#line 1266 "ada.g"
		Set (a_AST, ACCEPT_STATEMENT);
#line 9309 "AdaParser.cpp"
	}
	accept_stmt_AST = RefAdaAST(currentAST.root);
	returnAST = accept_stmt_AST;
}

void AdaParser::select_stmt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST select_stmt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  s = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	s = LT(1);
	if ( inputState->guessing == 0 ) {
		s_AST = astFactory->create(s);
		astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(s_AST));
	}
	match(SELECT);
	{
	bool synPredMatched403 = false;
	if (((LA(1) == IDENTIFIER || LA(1) == DELAY) && (_tokenSet_25.member(LA(2))))) {
		int _m403 = mark();
		synPredMatched403 = true;
		inputState->guessing++;
		try {
			{
			triggering_alternative();
			match(THEN);
			match(ABORT);
			}
		}
		catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
			synPredMatched403 = false;
		}
		rewind(_m403);
		inputState->guessing--;
	}
	if ( synPredMatched403 ) {
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
#line 1297 "ada.g"
			Set (s_AST, ASYNCHRONOUS_SELECT);
#line 9361 "AdaParser.cpp"
		}
	}
	else if ((_tokenSet_26.member(LA(1))) && (_tokenSet_27.member(LA(2)))) {
		selective_accept();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if ( inputState->guessing==0 ) {
#line 1299 "ada.g"
			Set (s_AST, SELECTIVE_ACCEPT);
#line 9372 "AdaParser.cpp"
		}
	}
	else if ((LA(1) == IDENTIFIER) && (_tokenSet_28.member(LA(2)))) {
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
#line 1301 "ada.g"
				Set (s_AST, TIMED_ENTRY_CALL);
#line 9392 "AdaParser.cpp"
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
#line 1302 "ada.g"
				Set (s_AST, CONDITIONAL_ENTRY_CALL);
#line 9406 "AdaParser.cpp"
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
	returnAST = select_stmt_AST;
}

void AdaParser::if_stmt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST if_stmt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  s = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 1114 "ada.g"
		Set(s_AST, IF_STATEMENT);
#line 9460 "AdaParser.cpp"
	}
	if_stmt_AST = RefAdaAST(currentAST.root);
	returnAST = if_stmt_AST;
}

void AdaParser::case_stmt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST case_stmt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  s = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 1134 "ada.g"
		Set(s_AST, CASE_STATEMENT);
#line 9494 "AdaParser.cpp"
	}
	case_stmt_AST = RefAdaAST(currentAST.root);
	returnAST = case_stmt_AST;
}

void AdaParser::loop_stmt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST loop_stmt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  l = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST l_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	iteration_scheme_opt();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	l = LT(1);
	if ( inputState->guessing == 0 ) {
		l_AST = astFactory->create(l);
		astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(l_AST));
	}
	match(LOOP);
	statements();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	match(END);
	match(LOOP);
	if ( inputState->guessing==0 ) {
#line 1146 "ada.g"
		Set(l_AST, LOOP_STATEMENT);
#line 9526 "AdaParser.cpp"
	}
	loop_stmt_AST = RefAdaAST(currentAST.root);
	returnAST = loop_stmt_AST;
}

void AdaParser::block() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST block_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 1198 "ada.g"
		block_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(BLOCK_STATEMENT,"BLOCK_STATEMENT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(block_AST))));
#line 9549 "AdaParser.cpp"
		currentAST.root = block_AST;
		if ( block_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			block_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = block_AST->getFirstChild();
		else
			currentAST.child = block_AST;
		currentAST.advanceChildToEnd();
	}
	block_AST = RefAdaAST(currentAST.root);
	returnAST = block_AST;
}

void AdaParser::statement_identifier() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST statement_identifier_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  n = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST n_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	n = LT(1);
	if ( inputState->guessing == 0 ) {
		n_AST = astFactory->create(n);
	}
	match(IDENTIFIER);
	match(COLON);
	if ( inputState->guessing==0 ) {
#line 1186 "ada.g"
		push_def_id(n_AST);
#line 9578 "AdaParser.cpp"
	}
	returnAST = statement_identifier_AST;
}

void AdaParser::id_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST id_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	id_opt_aux();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	if ( inputState->guessing==0 ) {
		id_opt_AST = RefAdaAST(currentAST.root);
#line 1172 "ada.g"
		id_opt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(ID_OPT,"ID_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(id_opt_AST))));
#line 9596 "AdaParser.cpp"
		currentAST.root = id_opt_AST;
		if ( id_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			id_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = id_opt_AST->getFirstChild();
		else
			currentAST.child = id_opt_AST;
		currentAST.advanceChildToEnd();
	}
	id_opt_AST = RefAdaAST(currentAST.root);
	returnAST = id_opt_AST;
}

void AdaParser::call_or_assignment() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST call_or_assignment_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 1222 "ada.g"
			call_or_assignment_AST =
						RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(ASSIGNMENT_STATEMENT,"ASSIGNMENT_STATEMENT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(call_or_assignment_AST))));
#line 9632 "AdaParser.cpp"
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
#line 1225 "ada.g"
			call_or_assignment_AST =
						RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(CALL_STATEMENT,"CALL_STATEMENT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(call_or_assignment_AST))));
#line 9650 "AdaParser.cpp"
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
	returnAST = call_or_assignment_AST;
}

void AdaParser::cond_clause() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST cond_clause_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  c = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST c_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 1118 "ada.g"
		Set(c_AST, COND_CLAUSE);
#line 9696 "AdaParser.cpp"
	}
	cond_clause_AST = RefAdaAST(currentAST.root);
	returnAST = cond_clause_AST;
}

void AdaParser::elsifs_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST elsifs_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
			goto _loop354;
		}
		
	}
	_loop354:;
	} // ( ... )*
	if ( inputState->guessing==0 ) {
		elsifs_opt_AST = RefAdaAST(currentAST.root);
#line 1126 "ada.g"
		elsifs_opt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(ELSIFS_OPT,"ELSIFS_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(elsifs_opt_AST))));
#line 9727 "AdaParser.cpp"
		currentAST.root = elsifs_opt_AST;
		if ( elsifs_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			elsifs_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = elsifs_opt_AST->getFirstChild();
		else
			currentAST.child = elsifs_opt_AST;
		currentAST.advanceChildToEnd();
	}
	elsifs_opt_AST = RefAdaAST(currentAST.root);
	returnAST = elsifs_opt_AST;
}

void AdaParser::else_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST else_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 1130 "ada.g"
		else_opt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(ELSE_OPT,"ELSE_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(else_opt_AST))));
#line 9770 "AdaParser.cpp"
		currentAST.root = else_opt_AST;
		if ( else_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			else_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = else_opt_AST->getFirstChild();
		else
			currentAST.child = else_opt_AST;
		currentAST.advanceChildToEnd();
	}
	else_opt_AST = RefAdaAST(currentAST.root);
	returnAST = else_opt_AST;
}

void AdaParser::condition() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST condition_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	expression();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	condition_AST = RefAdaAST(currentAST.root);
	returnAST = condition_AST;
}

void AdaParser::alternative_s() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST alternative_s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{ // ( ... )+
	int _cnt360=0;
	for (;;) {
		if ((LA(1) == WHEN)) {
			case_statement_alternative();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
		}
		else {
			if ( _cnt360>=1 ) { goto _loop360; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
		}
		
		_cnt360++;
	}
	_loop360:;
	}  // ( ... )+
	alternative_s_AST = RefAdaAST(currentAST.root);
	returnAST = alternative_s_AST;
}

void AdaParser::case_statement_alternative() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST case_statement_alternative_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  s = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 1141 "ada.g"
		Set(s_AST, CASE_STATEMENT_ALTERNATIVE);
#line 9847 "AdaParser.cpp"
	}
	case_statement_alternative_AST = RefAdaAST(currentAST.root);
	returnAST = case_statement_alternative_AST;
}

void AdaParser::iteration_scheme_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST iteration_scheme_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{
	switch ( LA(1)) {
	case WHILE:
	{
		RefAdaAST tmp316_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp316_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp316_AST));
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
		RefAdaAST tmp317_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp317_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp317_AST));
		}
		match(FOR);
		RefAdaAST tmp318_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp318_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp318_AST));
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
#line 1152 "ada.g"
		iteration_scheme_opt_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(ITERATION_SCHEME_OPT,"ITERATION_SCHEME_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(iteration_scheme_opt_AST))));
#line 9914 "AdaParser.cpp"
		currentAST.root = iteration_scheme_opt_AST;
		if ( iteration_scheme_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			iteration_scheme_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = iteration_scheme_opt_AST->getFirstChild();
		else
			currentAST.child = iteration_scheme_opt_AST;
		currentAST.advanceChildToEnd();
	}
	iteration_scheme_opt_AST = RefAdaAST(currentAST.root);
	returnAST = iteration_scheme_opt_AST;
}

void AdaParser::reverse_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST reverse_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{
	switch ( LA(1)) {
	case REVERSE:
	{
		RefAdaAST tmp320_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp320_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp320_AST));
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
#line 1158 "ada.g"
		reverse_opt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(MODIFIERS,"MODIFIERS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(reverse_opt_AST))));
#line 9968 "AdaParser.cpp"
		currentAST.root = reverse_opt_AST;
		if ( reverse_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			reverse_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = reverse_opt_AST->getFirstChild();
		else
			currentAST.child = reverse_opt_AST;
		currentAST.advanceChildToEnd();
	}
	reverse_opt_AST = RefAdaAST(currentAST.root);
	returnAST = reverse_opt_AST;
}

void AdaParser::id_opt_aux() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST id_opt_aux_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	RefAdaAST n_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
#line 1161 "ada.g"
	RefAdaAST endid;
#line 9988 "AdaParser.cpp"
	
	switch ( LA(1)) {
	case CHAR_STRING:
	{
		endid=definable_operator_symbol();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		if (!( end_id_matches_def_id (endid) ))
			throw ANTLR_USE_NAMESPACE(antlr)SemanticException(" end_id_matches_def_id (endid) ");
		id_opt_aux_AST = RefAdaAST(currentAST.root);
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
		id_opt_aux_AST = RefAdaAST(currentAST.root);
		break;
	}
	case SEMI:
	{
		if ( inputState->guessing==0 ) {
#line 1168 "ada.g"
			pop_def_id();
#line 10019 "AdaParser.cpp"
		}
		id_opt_aux_AST = RefAdaAST(currentAST.root);
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	returnAST = id_opt_aux_AST;
}

void AdaParser::declare_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST declare_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 1202 "ada.g"
		declare_opt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(DECLARE_OPT,"DECLARE_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(declare_opt_AST))));
#line 10062 "AdaParser.cpp"
		currentAST.root = declare_opt_AST;
		if ( declare_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			declare_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = declare_opt_AST->getFirstChild();
		else
			currentAST.child = declare_opt_AST;
		currentAST.advanceChildToEnd();
	}
	declare_opt_AST = RefAdaAST(currentAST.root);
	returnAST = declare_opt_AST;
}

void AdaParser::label_name() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST label_name_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	RefAdaAST tmp322_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	if ( inputState->guessing == 0 ) {
		tmp322_AST = astFactory->create(LT(1));
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp322_AST));
	}
	match(IDENTIFIER);
	label_name_AST = RefAdaAST(currentAST.root);
	returnAST = label_name_AST;
}

void AdaParser::entry_body_formal_part() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST entry_body_formal_part_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	entry_index_spec_opt();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	formal_part_opt();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	entry_body_formal_part_AST = RefAdaAST(currentAST.root);
	returnAST = entry_body_formal_part_AST;
}

void AdaParser::entry_barrier() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST entry_barrier_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	match(WHEN);
	condition();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	entry_barrier_AST = RefAdaAST(currentAST.root);
	returnAST = entry_barrier_AST;
}

void AdaParser::entry_index_spec_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST entry_index_spec_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{
	bool synPredMatched388 = false;
	if (((LA(1) == LPAREN) && (LA(2) == FOR))) {
		int _m388 = mark();
		synPredMatched388 = true;
		inputState->guessing++;
		try {
			{
			match(LPAREN);
			match(FOR);
			}
		}
		catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
			synPredMatched388 = false;
		}
		rewind(_m388);
		inputState->guessing--;
	}
	if ( synPredMatched388 ) {
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
	else if ((LA(1) == LPAREN || LA(1) == WHEN) && (_tokenSet_0.member(LA(2)))) {
	}
	else {
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	
	}
	if ( inputState->guessing==0 ) {
		entry_index_spec_opt_AST = RefAdaAST(currentAST.root);
#line 1247 "ada.g"
		entry_index_spec_opt_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(ENTRY_INDEX_SPECIFICATION,"ENTRY_INDEX_SPECIFICATION")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(entry_index_spec_opt_AST))));
#line 10170 "AdaParser.cpp"
		currentAST.root = entry_index_spec_opt_AST;
		if ( entry_index_spec_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			entry_index_spec_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = entry_index_spec_opt_AST->getFirstChild();
		else
			currentAST.child = entry_index_spec_opt_AST;
		currentAST.advanceChildToEnd();
	}
	entry_index_spec_opt_AST = RefAdaAST(currentAST.root);
	returnAST = entry_index_spec_opt_AST;
}

void AdaParser::entry_call_stmt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST entry_call_stmt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	name();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	match(SEMI);
	if ( inputState->guessing==0 ) {
		entry_call_stmt_AST = RefAdaAST(currentAST.root);
#line 1257 "ada.g"
		entry_call_stmt_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(ENTRY_CALL_STATEMENT,"ENTRY_CALL_STATEMENT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(entry_call_stmt_AST))));
#line 10198 "AdaParser.cpp"
		currentAST.root = entry_call_stmt_AST;
		if ( entry_call_stmt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			entry_call_stmt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = entry_call_stmt_AST->getFirstChild();
		else
			currentAST.child = entry_call_stmt_AST;
		currentAST.advanceChildToEnd();
	}
	entry_call_stmt_AST = RefAdaAST(currentAST.root);
	returnAST = entry_call_stmt_AST;
}

void AdaParser::entry_index_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST entry_index_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{
	bool synPredMatched396 = false;
	if (((LA(1) == LPAREN) && (_tokenSet_0.member(LA(2))))) {
		int _m396 = mark();
		synPredMatched396 = true;
		inputState->guessing++;
		try {
			{
			match(LPAREN);
			expression();
			match(RPAREN);
			}
		}
		catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
			synPredMatched396 = false;
		}
		rewind(_m396);
		inputState->guessing--;
	}
	if ( synPredMatched396 ) {
		match(LPAREN);
		expression();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		match(RPAREN);
	}
	else if ((LA(1) == SEMI || LA(1) == LPAREN || LA(1) == DO) && (_tokenSet_29.member(LA(2)))) {
	}
	else {
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	
	}
	if ( inputState->guessing==0 ) {
		entry_index_opt_AST = RefAdaAST(currentAST.root);
#line 1276 "ada.g"
		entry_index_opt_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(ENTRY_INDEX_OPT,"ENTRY_INDEX_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(entry_index_opt_AST))));
#line 10255 "AdaParser.cpp"
		currentAST.root = entry_index_opt_AST;
		if ( entry_index_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			entry_index_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = entry_index_opt_AST->getFirstChild();
		else
			currentAST.child = entry_index_opt_AST;
		currentAST.advanceChildToEnd();
	}
	entry_index_opt_AST = RefAdaAST(currentAST.root);
	returnAST = entry_index_opt_AST;
}

void AdaParser::until_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST until_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{
	switch ( LA(1)) {
	case UNTIL:
	{
		RefAdaAST tmp331_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp331_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp331_AST));
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
#line 1286 "ada.g"
		until_opt_AST = RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(MODIFIERS,"MODIFIERS")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(until_opt_AST))));
#line 10309 "AdaParser.cpp"
		currentAST.root = until_opt_AST;
		if ( until_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			until_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = until_opt_AST->getFirstChild();
		else
			currentAST.child = until_opt_AST;
		currentAST.advanceChildToEnd();
	}
	until_opt_AST = RefAdaAST(currentAST.root);
	returnAST = until_opt_AST;
}

void AdaParser::triggering_alternative() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST triggering_alternative_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 1310 "ada.g"
		triggering_alternative_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(TRIGGERING_ALTERNATIVE,"TRIGGERING_ALTERNATIVE")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(triggering_alternative_AST))));
#line 10360 "AdaParser.cpp"
		currentAST.root = triggering_alternative_AST;
		if ( triggering_alternative_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			triggering_alternative_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = triggering_alternative_AST->getFirstChild();
		else
			currentAST.child = triggering_alternative_AST;
		currentAST.advanceChildToEnd();
	}
	triggering_alternative_AST = RefAdaAST(currentAST.root);
	returnAST = triggering_alternative_AST;
}

void AdaParser::abortable_part() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST abortable_part_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	stmts_opt();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	if ( inputState->guessing==0 ) {
		abortable_part_AST = RefAdaAST(currentAST.root);
#line 1316 "ada.g"
		abortable_part_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(ABORTABLE_PART,"ABORTABLE_PART")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(abortable_part_AST))));
#line 10387 "AdaParser.cpp"
		currentAST.root = abortable_part_AST;
		if ( abortable_part_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			abortable_part_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = abortable_part_AST->getFirstChild();
		else
			currentAST.child = abortable_part_AST;
		currentAST.advanceChildToEnd();
	}
	abortable_part_AST = RefAdaAST(currentAST.root);
	returnAST = abortable_part_AST;
}

void AdaParser::selective_accept() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST selective_accept_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
	returnAST = selective_accept_AST;
}

void AdaParser::entry_call_alternative() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST entry_call_alternative_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 1322 "ada.g"
		entry_call_alternative_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(ENTRY_CALL_ALTERNATIVE,"ENTRY_CALL_ALTERNATIVE")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(entry_call_alternative_AST))));
#line 10443 "AdaParser.cpp"
		currentAST.root = entry_call_alternative_AST;
		if ( entry_call_alternative_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			entry_call_alternative_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = entry_call_alternative_AST->getFirstChild();
		else
			currentAST.child = entry_call_alternative_AST;
		currentAST.advanceChildToEnd();
	}
	entry_call_alternative_AST = RefAdaAST(currentAST.root);
	returnAST = entry_call_alternative_AST;
}

void AdaParser::delay_alternative() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST delay_alternative_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 1347 "ada.g"
		delay_alternative_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(DELAY_ALTERNATIVE,"DELAY_ALTERNATIVE")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(delay_alternative_AST))));
#line 10474 "AdaParser.cpp"
		currentAST.root = delay_alternative_AST;
		if ( delay_alternative_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			delay_alternative_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = delay_alternative_AST->getFirstChild();
		else
			currentAST.child = delay_alternative_AST;
		currentAST.advanceChildToEnd();
	}
	delay_alternative_AST = RefAdaAST(currentAST.root);
	returnAST = delay_alternative_AST;
}

void AdaParser::stmts_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST stmts_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
			goto _loop419;
		}
		}
	}
	_loop419:;
	} // ( ... )*
	stmts_opt_AST = RefAdaAST(currentAST.root);
	returnAST = stmts_opt_AST;
}

void AdaParser::guard_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST guard_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  w = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST w_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{
	switch ( LA(1)) {
	case WHEN:
	{
		w = LT(1);
		if ( inputState->guessing == 0 ) {
			w_AST = astFactory->create(w);
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(w_AST));
		}
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
				goto _loop413;
			}
			
		}
		_loop413:;
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
#line 1331 "ada.g"
		Set(w_AST, GUARD_OPT);
#line 10595 "AdaParser.cpp"
	}
	guard_opt_AST = RefAdaAST(currentAST.root);
	returnAST = guard_opt_AST;
}

void AdaParser::select_alternative() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST select_alternative_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  t = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST t_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 1337 "ada.g"
			Set(t_AST, TERMINATE_ALTERNATIVE);
#line 10639 "AdaParser.cpp"
		}
		select_alternative_AST = RefAdaAST(currentAST.root);
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	returnAST = select_alternative_AST;
}

void AdaParser::or_select_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST or_select_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
			goto _loop422;
		}
		
	}
	_loop422:;
	} // ( ... )*
	if ( inputState->guessing==0 ) {
		or_select_opt_AST = RefAdaAST(currentAST.root);
#line 1356 "ada.g"
		or_select_opt_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(OR_SELECT_OPT,"OR_SELECT_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(or_select_opt_AST))));
#line 10682 "AdaParser.cpp"
		currentAST.root = or_select_opt_AST;
		if ( or_select_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			or_select_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = or_select_opt_AST->getFirstChild();
		else
			currentAST.child = or_select_opt_AST;
		currentAST.advanceChildToEnd();
	}
	or_select_opt_AST = RefAdaAST(currentAST.root);
	returnAST = or_select_opt_AST;
}

void AdaParser::accept_alternative() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST accept_alternative_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 1341 "ada.g"
		accept_alternative_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(ACCEPT_ALTERNATIVE,"ACCEPT_ALTERNATIVE")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(accept_alternative_AST))));
#line 10713 "AdaParser.cpp"
		currentAST.root = accept_alternative_AST;
		if ( accept_alternative_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			accept_alternative_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = accept_alternative_AST->getFirstChild();
		else
			currentAST.child = accept_alternative_AST;
		currentAST.advanceChildToEnd();
	}
	accept_alternative_AST = RefAdaAST(currentAST.root);
	returnAST = accept_alternative_AST;
}

void AdaParser::exception_handler() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST exception_handler_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  w = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST w_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 1372 "ada.g"
		Set (w_AST, EXCEPTION_HANDLER);
#line 10755 "AdaParser.cpp"
	}
	exception_handler_AST = RefAdaAST(currentAST.root);
	returnAST = exception_handler_AST;
}

void AdaParser::identifier_colon_opt() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST identifier_colon_opt_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{
	if ((LA(1) == IDENTIFIER) && (LA(2) == COLON)) {
		RefAdaAST tmp336_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp336_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp336_AST));
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
#line 1376 "ada.g"
		identifier_colon_opt_AST =
				RefAdaAST(astFactory->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(astFactory->create(IDENTIFIER_COLON_OPT,"IDENTIFIER_COLON_OPT")))->add(ANTLR_USE_NAMESPACE(antlr)RefAST(identifier_colon_opt_AST))));
#line 10788 "AdaParser.cpp"
		currentAST.root = identifier_colon_opt_AST;
		if ( identifier_colon_opt_AST!=RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) &&
			identifier_colon_opt_AST->getFirstChild() != RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST) )
			  currentAST.child = identifier_colon_opt_AST->getFirstChild();
		else
			currentAST.child = identifier_colon_opt_AST;
		currentAST.advanceChildToEnd();
	}
	identifier_colon_opt_AST = RefAdaAST(currentAST.root);
	returnAST = identifier_colon_opt_AST;
}

void AdaParser::except_choice_s() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST except_choice_s_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	exception_choice();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	{ // ( ... )*
	for (;;) {
		if ((LA(1) == PIPE)) {
			RefAdaAST tmp338_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp338_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp338_AST));
			}
			match(PIPE);
			exception_choice();
			if (inputState->guessing==0) {
				astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
			}
		}
		else {
			goto _loop435;
		}
		
	}
	_loop435:;
	} // ( ... )*
	except_choice_s_AST = RefAdaAST(currentAST.root);
	returnAST = except_choice_s_AST;
}

void AdaParser::exception_choice() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST exception_choice_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
		RefAdaAST tmp339_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp339_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp339_AST));
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
	returnAST = exception_choice_AST;
}

void AdaParser::operator_call() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST operator_call_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  cs = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST cs_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
	returnAST = operator_call_AST;
}

void AdaParser::operator_call_tail(
	RefAdaAST opstr
) {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST operator_call_tail_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	match(LPAREN);
	if (!( is_operator_symbol(opstr->getText().c_str()) ))
		throw ANTLR_USE_NAMESPACE(antlr)SemanticException(" is_operator_symbol(opstr->getText().c_str()) ");
	value_s();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	match(RPAREN);
	if ( inputState->guessing==0 ) {
#line 1401 "ada.g"
		opstr->setType(OPERATOR_SYMBOL);
#line 10908 "AdaParser.cpp"
	}
	operator_call_tail_AST = RefAdaAST(currentAST.root);
	returnAST = operator_call_tail_AST;
}

void AdaParser::relation() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST relation_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  n = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST n_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	simple_expression();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	{
	switch ( LA(1)) {
	case IN:
	{
		RefAdaAST tmp342_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp342_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp342_AST));
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
#line 1425 "ada.g"
			Set (n_AST, NOT_IN);
#line 10957 "AdaParser.cpp"
		}
		break;
	}
	case EQ:
	{
		RefAdaAST tmp344_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp344_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp344_AST));
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
		RefAdaAST tmp345_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp345_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp345_AST));
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
		RefAdaAST tmp346_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp346_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp346_AST));
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
		RefAdaAST tmp347_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp347_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp347_AST));
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
		RefAdaAST tmp348_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp348_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp348_AST));
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
		RefAdaAST tmp349_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp349_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp349_AST));
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
	returnAST = relation_AST;
}

void AdaParser::range_or_mark() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST range_or_mark_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	bool synPredMatched455 = false;
	if (((_tokenSet_0.member(LA(1))) && (_tokenSet_8.member(LA(2))))) {
		int _m455 = mark();
		synPredMatched455 = true;
		inputState->guessing++;
		try {
			{
			range();
			}
		}
		catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
			synPredMatched455 = false;
		}
		rewind(_m455);
		inputState->guessing--;
	}
	if ( synPredMatched455 ) {
		range();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		range_or_mark_AST = RefAdaAST(currentAST.root);
	}
	else if ((LA(1) == IDENTIFIER) && (_tokenSet_30.member(LA(2)))) {
		subtype_mark();
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
		range_or_mark_AST = RefAdaAST(currentAST.root);
	}
	else {
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
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
#line 1447 "ada.g"
			Set(p_AST, UNARY_PLUS);
#line 11141 "AdaParser.cpp"
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
#line 1448 "ada.g"
			Set(m_AST, UNARY_MINUS);
#line 11161 "AdaParser.cpp"
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
	returnAST = signed_term_AST;
}

void AdaParser::term() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST term_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	factor();
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	{ // ( ... )*
	for (;;) {
		switch ( LA(1)) {
		case STAR:
		{
			RefAdaAST tmp350_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp350_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp350_AST));
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
			RefAdaAST tmp351_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp351_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp351_AST));
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
			RefAdaAST tmp352_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp352_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp352_AST));
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
			RefAdaAST tmp353_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp353_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp353_AST));
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
			goto _loop462;
		}
		}
	}
	_loop462:;
	} // ( ... )*
	term_AST = RefAdaAST(currentAST.root);
	returnAST = term_AST;
}

void AdaParser::factor() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST factor_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
	{
	switch ( LA(1)) {
	case NOT:
	{
		RefAdaAST tmp354_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp354_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp354_AST));
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
		RefAdaAST tmp355_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp355_AST = astFactory->create(LT(1));
			astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp355_AST));
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
			RefAdaAST tmp356_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp356_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp356_AST));
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
	returnAST = factor_AST;
}

void AdaParser::primary() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST primary_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  cs = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST cs_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
		RefAdaAST tmp357_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp357_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp357_AST));
		}
		match(NuLL);
		break;
	}
	case NUMERIC_LIT:
	{
		RefAdaAST tmp358_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp358_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp358_AST));
		}
		match(NUMERIC_LIT);
		break;
	}
	case CHARACTER_LITERAL:
	{
		RefAdaAST tmp359_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
		if ( inputState->guessing == 0 ) {
			tmp359_AST = astFactory->create(LT(1));
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp359_AST));
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
	returnAST = primary_AST;
}

void AdaParser::name_or_qualified() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST name_or_qualified_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  p = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST p_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
#line 1479 "ada.g"
	RefAdaAST dummy;
#line 11528 "AdaParser.cpp"
	
	RefAdaAST tmp360_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	if ( inputState->guessing == 0 ) {
		tmp360_AST = astFactory->create(LT(1));
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp360_AST));
	}
	match(IDENTIFIER);
	{ // ( ... )*
	for (;;) {
		switch ( LA(1)) {
		case DOT:
		{
			RefAdaAST tmp361_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp361_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp361_AST));
			}
			match(DOT);
			{
			switch ( LA(1)) {
			case ALL:
			{
				RefAdaAST tmp362_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp362_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp362_AST));
				}
				match(ALL);
				break;
			}
			case IDENTIFIER:
			{
				RefAdaAST tmp363_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp363_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp363_AST));
				}
				match(IDENTIFIER);
				break;
			}
			case CHARACTER_LITERAL:
			{
				RefAdaAST tmp364_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
				if ( inputState->guessing == 0 ) {
					tmp364_AST = astFactory->create(LT(1));
					astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp364_AST));
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
#line 1487 "ada.g"
				Set(p_AST, INDEXED_COMPONENT);
#line 11611 "AdaParser.cpp"
			}
			break;
		}
		case TIC:
		{
			RefAdaAST tmp366_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
			if ( inputState->guessing == 0 ) {
				tmp366_AST = astFactory->create(LT(1));
				astFactory->makeASTRoot(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(tmp366_AST));
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
			goto _loop473;
		}
		}
	}
	_loop473:;
	} // ( ... )*
	name_or_qualified_AST = RefAdaAST(currentAST.root);
	returnAST = name_or_qualified_AST;
}

void AdaParser::allocator() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST allocator_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  n = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST n_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 1493 "ada.g"
		Set(n_AST, ALLOCATOR);
#line 11685 "AdaParser.cpp"
	}
	allocator_AST = RefAdaAST(currentAST.root);
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
#line 1507 "ada.g"
			pop_def_id(); Set(p_AST, PROCEDURE_BODY);
#line 11726 "AdaParser.cpp"
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
		def_designator(false);
		if (inputState->guessing==0) {
			astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
		}
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
#line 1509 "ada.g"
			pop_def_id(); Set(f_AST, FUNCTION_BODY);
#line 11756 "AdaParser.cpp"
		}
		subprogram_body_AST = RefAdaAST(currentAST.root);
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
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
	match(SEMI);
	if ( inputState->guessing==0 ) {
#line 1513 "ada.g"
		Set(p_AST, PACKAGE_BODY);
#line 11794 "AdaParser.cpp"
	}
	package_body_AST = RefAdaAST(currentAST.root);
	returnAST = package_body_AST;
}

void AdaParser::task_body() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST task_body_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  t = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST t_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
#line 1517 "ada.g"
		Set(t_AST, TASK_BODY);
#line 11825 "AdaParser.cpp"
	}
	task_body_AST = RefAdaAST(currentAST.root);
	returnAST = task_body_AST;
}

void AdaParser::protected_body() {
	returnAST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	RefAdaAST protected_body_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	ANTLR_USE_NAMESPACE(antlr)RefToken  p = ANTLR_USE_NAMESPACE(antlr)nullToken;
	RefAdaAST p_AST = RefAdaAST(ANTLR_USE_NAMESPACE(antlr)nullAST);
	
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
	if (inputState->guessing==0) {
		astFactory->addASTChild(currentAST, ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST));
	}
	match(SEMI);
	if ( inputState->guessing==0 ) {
#line 1521 "ada.g"
		Set(p_AST, PROTECTED_BODY);
#line 11860 "AdaParser.cpp"
	}
	protected_body_AST = RefAdaAST(currentAST.root);
	returnAST = protected_body_AST;
}

void AdaParser::initializeASTFactory( ANTLR_USE_NAMESPACE(antlr)ASTFactory& factory )
{
	factory.setMaxNodeType(289);
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
	"CONDITIONAL_ENTRY_CALL",
	"CONTEXT_CLAUSE",
	"DECLARATIVE_PART",
	"DEFINING_IDENTIFIER_LIST",
	"DELAY_ALTERNATIVE",
	"DELAY_STATEMENT",
	"DELTA_CONSTRAINT",
	"DIGITS_CONSTRAINT",
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
	"GENERIC_FORMAL_PART",
	"GENERIC_PACKAGE_DECLARATION",
	"GOTO_STATEMENT",
	"HANDLED_SEQUENCE_OF_STATEMENTS",
	"HANDLED_STMTS_OPT",
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
	"PRIVATE_EXTENSION_DECLARATION",
	"PRIVATE_TYPE_DECLARATION",
	"PROTECTED_BODY",
	"PROTECTED_BODY_STUB",
	"PROTECTED_TYPE_DECLARATION",
	"RAISE_STATEMENT",
	"RANGE_ATTRIBUTE_REFERENCE",
	"RECORD_REPRESENTATION_CLAUSE",
	"REQUEUE_STATEMENT",
	"RETURN_STATEMENT",
	"SELECTIVE_ACCEPT",
	"SEQUENCE_OF_STATEMENTS",
	"SINGLE_PROTECTED_DECLARATION",
	"SINGLE_TASK_DECLARATION",
	"STATEMENT",
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
	"DISCRETE_SUBTYPE_DEF_OPT",
	"DISCRIMINANT_SPECIFICATIONS",
	"DISCRIM_PART_OPT",
	"ELSE_OPT",
	"ELSIFS_OPT",
	"END_ID_OPT",
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
	"ID_OPT",
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
	"PRIVATE_DECLARATIVE_ITEMS_OPT",
	"PRIVATE_TASK_ITEMS_OPT",
	"PROCEDURE_BODY",
	"PROCEDURE_BODY_STUB",
	"PROCEDURE_DECLARATION",
	"PROCEDURE_RENAMING_DECLARATION",
	"PROT_MEMBER_DECLARATIONS",
	"PROT_OP_BODIES_OPT",
	"PROT_OP_DECLARATIONS",
	"PROT_PRIVATE_OPT",
	"RANGED_EXPRS",
	"RANGE_CONSTRAINT",
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
	"OX",
	"TIC_OR_CHARACTER_LITERAL",
	"DIGIT",
	"EXPONENT",
	"EXTENDED_DIGIT",
	"BASED_INTEGER",
	"WS_",
	"COMMENT",
	0
};

const unsigned long AdaParser::_tokenSet_0_data_[] = { 67109024UL, 56UL, 3229614080UL, 80UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER LPAREN "new" CHARACTER_LITERAL CHAR_STRING "null" "not" PLUS 
// MINUS "abs" NUMERIC_LIT 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_0(_tokenSet_0_data_,10);
const unsigned long AdaParser::_tokenSet_1_data_[] = { 201364384UL, 262201UL, 4293001216UL, 127UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER LPAREN COMMA RPAREN DOT TIC "new" "others" "in" CHARACTER_LITERAL 
// CHAR_STRING "null" "mod" "or" "and" "xor" "not" EQ NE LT_ LE GT GE PLUS 
// MINUS CONCAT STAR DIV "rem" "abs" EXPON NUMERIC_LIT 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_1(_tokenSet_1_data_,10);
const unsigned long AdaParser::_tokenSet_2_data_[] = { 8388800UL, 4UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LPAREN "is" "renames" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_2(_tokenSet_2_data_,10);
const unsigned long AdaParser::_tokenSet_3_data_[] = { 52453426UL, 1611238800UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF "pragma" IDENTIFIER "use" "type" "package" "procedure" "function" 
// CHAR_STRING "separate" "abstract" "task" "protected" "for" "end" "subtype" 
// "generic" "begin" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_3(_tokenSet_3_data_,10);
const unsigned long AdaParser::_tokenSet_4_data_[] = { 8392896UL, 67108869UL, 4096UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LPAREN DOT "is" "in" "renames" "when" "do" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_4(_tokenSet_4_data_,10);
const unsigned long AdaParser::_tokenSet_5_data_[] = { 8388800UL, 67108869UL, 4096UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LPAREN "is" "in" "renames" "when" "do" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_5(_tokenSet_5_data_,10);
const unsigned long AdaParser::_tokenSet_6_data_[] = { 53502002UL, 537496976UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF "pragma" IDENTIFIER "use" "type" "private" "package" "procedure" 
// "function" CHAR_STRING "separate" "abstract" "task" "protected" "for" 
// "end" "subtype" "generic" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_6(_tokenSet_6_data_,10);
const unsigned long AdaParser::_tokenSet_7_data_[] = { 8392832UL, 512UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LPAREN DOT "is" "return" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_7(_tokenSet_7_data_,10);
const unsigned long AdaParser::_tokenSet_8_data_[] = { 738234528UL, 262200UL, 3229614080UL, 127UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER LPAREN DOT TIC "new" "others" DOT_DOT CHARACTER_LITERAL CHAR_STRING 
// "null" "mod" "not" PLUS MINUS CONCAT STAR DIV "rem" "abs" EXPON NUMERIC_LIT 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_8(_tokenSet_8_data_,10);
const unsigned long AdaParser::_tokenSet_9_data_[] = { 201326752UL, 56UL, 3229614080UL, 80UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER LPAREN "new" "others" CHARACTER_LITERAL CHAR_STRING "null" 
// "not" PLUS MINUS "abs" NUMERIC_LIT 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_9(_tokenSet_9_data_,10);
const unsigned long AdaParser::_tokenSet_10_data_[] = { 1006739360UL, 262201UL, 4293001216UL, 127UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER LPAREN COMMA RPAREN RIGHT_SHAFT "with" DOT TIC "range" "new" 
// "others" PIPE DOT_DOT "in" CHARACTER_LITERAL CHAR_STRING "null" "mod" 
// "or" "and" "xor" "not" EQ NE LT_ LE GT GE PLUS MINUS CONCAT STAR DIV 
// "rem" "abs" EXPON NUMERIC_LIT 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_10(_tokenSet_10_data_,10);
const unsigned long AdaParser::_tokenSet_11_data_[] = { 1006737312UL, 262201UL, 4293001216UL, 127UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER LPAREN COMMA RPAREN RIGHT_SHAFT DOT TIC "range" "new" "others" 
// PIPE DOT_DOT "in" CHARACTER_LITERAL CHAR_STRING "null" "mod" "or" "and" 
// "xor" "not" EQ NE LT_ LE GT GE PLUS MINUS CONCAT STAR DIV "rem" "abs" 
// EXPON NUMERIC_LIT 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_11(_tokenSet_11_data_,10);
const unsigned long AdaParser::_tokenSet_12_data_[] = { 32UL, 23068672UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER "constant" "array" "aliased" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_12(_tokenSet_12_data_,10);
const unsigned long AdaParser::_tokenSet_13_data_[] = { 495840UL, 6299648UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER SEMI LPAREN DOT TIC "range" "digits" "delta" ASSIGN "constant" 
// "array" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_13(_tokenSet_13_data_,10);
const unsigned long AdaParser::_tokenSet_14_data_[] = { 51380274UL, 114688UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF "pragma" IDENTIFIER "private" "procedure" "function" "entry" "for" 
// "end" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_14(_tokenSet_14_data_,10);
const unsigned long AdaParser::_tokenSet_15_data_[] = { 496256UL, 0UL, 32UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LPAREN RPAREN DOT TIC "range" "digits" "delta" "loop" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_15(_tokenSet_15_data_,10);
const unsigned long AdaParser::_tokenSet_16_data_[] = { 50331664UL, 49152UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "pragma" "procedure" "function" "entry" "for" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_16(_tokenSet_16_data_,10);
const unsigned long AdaParser::_tokenSet_17_data_[] = { 1048576UL, 402653536UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "private" "null" "record" "abstract" "tagged" "limited" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_17(_tokenSet_17_data_,10);
const unsigned long AdaParser::_tokenSet_18_data_[] = { 1048688UL, 436207712UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "pragma" IDENTIFIER SEMI "private" "null" "record" "case" "tagged" "limited" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_18(_tokenSet_18_data_,10);
const unsigned long AdaParser::_tokenSet_19_data_[] = { 496512UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LPAREN COMMA RPAREN DOT TIC "range" "digits" "delta" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_19(_tokenSet_19_data_,10);
const unsigned long AdaParser::_tokenSet_20_data_[] = { 738300064UL, 262200UL, 3229614080UL, 127UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER LPAREN DOT TIC "range" "new" "others" DOT_DOT CHARACTER_LITERAL 
// CHAR_STRING "null" "mod" "not" PLUS MINUS CONCAT STAR DIV "rem" "abs" 
// EXPON NUMERIC_LIT 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_20(_tokenSet_20_data_,10);
const unsigned long AdaParser::_tokenSet_21_data_[] = { 469800096UL, 262201UL, 4293001216UL, 127UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER LPAREN RIGHT_SHAFT DOT TIC "new" "others" PIPE "in" CHARACTER_LITERAL 
// CHAR_STRING "null" "mod" "or" "and" "xor" "not" EQ NE LT_ LE GT GE PLUS 
// MINUS CONCAT STAR DIV "rem" "abs" EXPON NUMERIC_LIT 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_21(_tokenSet_21_data_,10);
const unsigned long AdaParser::_tokenSet_22_data_[] = { 1048576UL, 402653440UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "private" "abstract" "tagged" "limited" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_22(_tokenSet_22_data_,10);
const unsigned long AdaParser::_tokenSet_23_data_[] = { 1048640UL, 402653184UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI "private" "tagged" "limited" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_23(_tokenSet_23_data_,10);
const unsigned long AdaParser::_tokenSet_24_data_[] = { 37056UL, 8192UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LPAREN DOT TIC ASSIGN 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_24(_tokenSet_24_data_,10);
const unsigned long AdaParser::_tokenSet_25_data_[] = { 67145952UL, 56UL, 3229630464UL, 80UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER SEMI LPAREN DOT TIC "new" CHARACTER_LITERAL CHAR_STRING "null" 
// "until" "not" PLUS MINUS "abs" NUMERIC_LIT 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_25(_tokenSet_25_data_,10);
const unsigned long AdaParser::_tokenSet_26_data_[] = { 0UL, 67108864UL, 272384UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "when" "accept" "delay" "terminate" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_26(_tokenSet_26_data_,10);
const unsigned long AdaParser::_tokenSet_27_data_[] = { 67109088UL, 56UL, 3229630464UL, 80UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// IDENTIFIER SEMI LPAREN "new" CHARACTER_LITERAL CHAR_STRING "null" "until" 
// "not" PLUS MINUS "abs" NUMERIC_LIT 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_27(_tokenSet_27_data_,10);
const unsigned long AdaParser::_tokenSet_28_data_[] = { 37056UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI LPAREN DOT TIC 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_28(_tokenSet_28_data_,10);
const unsigned long AdaParser::_tokenSet_29_data_[] = { 48UL, 3323036192UL, 1814398UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "pragma" IDENTIFIER "null" "return" "for" "end" "exception" "case" "when" 
// "begin" LT_LT "if" "then" "elsif" "else" "loop" "while" "declare" "exit" 
// "goto" "accept" "delay" "select" "abort" "or" "raise" "requeue" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_29(_tokenSet_29_data_,10);
const unsigned long AdaParser::_tokenSet_30_data_[] = { 813932352UL, 8192UL, 6422564UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMI COMMA RPAREN RIGHT_SHAFT "with" DOT TIC "range" "digits" "is" PIPE 
// DOT_DOT ASSIGN "then" "loop" "or" "and" "xor" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaParser::_tokenSet_30(_tokenSet_30_data_,10);


