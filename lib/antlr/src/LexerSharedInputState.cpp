#include "antlr/LexerSharedInputState.hpp"
#include "antlr/CharBuffer.hpp"

ANTLR_BEGIN_NAMESPACE(antlr)

/** This object contains the data associated with an
 *  input stream of characters.  Multiple lexers
 *  share a single LexerSharedInputState to lex
 *  the same input stream.
 */

LexerInputState::LexerInputState(InputBuffer* inbuf)
: column(1)
, line(1)
, tokenStartColumn(1)
, tokenStartLine(1)
, guessing(0)
, filename("")									
, input(inbuf)
, inputResponsible(true)
{
}

LexerInputState::LexerInputState(InputBuffer& inbuf)
: column(1)
, line(1)
, tokenStartColumn(1)
, tokenStartLine(1)
, guessing(0)
, filename("")									
, input(&inbuf)
, inputResponsible(false)
{
}

LexerInputState::LexerInputState(ANTLR_USE_NAMESPACE(std)istream& in)
: column(1)
, line(1)
, tokenStartColumn(1)
, tokenStartLine(1)
, guessing(0)
, filename("")									
, input(new CharBuffer(in))
, inputResponsible(true)
{
}

LexerInputState::~LexerInputState()
{
	if (inputResponsible)
		delete input;
}

ANTLR_END_NAMESPACE

