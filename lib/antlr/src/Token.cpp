/* ANTLR Translator Generator
 * Project led by Terence Parr at http://www.jGuru.com
 * Software rights: http://www.antlr.org/RIGHTS.html
 *
 */

#include "antlr/Token.hpp"
#include "antlr/String.hpp"

#ifdef ANTLR_CXX_SUPPORTS_NAMESPACE
namespace antlr {
#endif

// The below initialization ICED AIX Visualage CC
//ANTLR_API RefToken Token::badToken(new Token(Token::INVALID_TYPE, "<no text>"));
// this seemed to work
ANTLR_API RefToken Token::badToken = RefToken(new Token(Token::INVALID_TYPE, "<no text>"));

Token::Token() : type(INVALID_TYPE)
{
}

Token::Token(int t) : type(t)
{
}

Token::Token(int t, const ANTLR_USE_NAMESPACE(std)string& txt)
	: type(t)
{
	type=t;
	setText(txt);
}

int Token::getColumn() const
{
	return 0;
}

int Token::getLine() const
{
	return 0;
}

ANTLR_USE_NAMESPACE(std)string Token::getText() const
{
	return "<no text>";
}

int Token::getType() const
{
	return type;
}

void Token::setColumn(int /*c*/)
{}

void Token::setLine(int /*l*/)
{}

void Token::setText(const ANTLR_USE_NAMESPACE(std)string& /*t*/)
{}

void Token::setType(int t)
{
	type=t;
}

ANTLR_USE_NAMESPACE(std)string Token::toString() const
{
	return "[\""+getText()+"\",<"+type+">]";
}

Token::~Token()
{}

ANTLR_API RefToken nullToken;

#ifndef NO_STATIC_CONSTS
const int Token::MIN_USER_TYPE;
const int Token::NULL_TREE_LOOKAHEAD;
const int Token::INVALID_TYPE;
const int Token::EOF_TYPE;
const int Token::SKIP;
#endif

#ifdef ANTLR_CXX_SUPPORTS_NAMESPACE
}
#endif
