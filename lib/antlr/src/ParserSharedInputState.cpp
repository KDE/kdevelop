#include "antlr/ParserSharedInputState.hpp"

ANTLR_BEGIN_NAMESPACE(antlr)

/** This object contains the data associated with an
 *  input stream of tokens.  Multiple parsers
 *  share a single ParserSharedInputState to parse
 *  the same stream of tokens.
 */

ParserInputState::ParserInputState(TokenBuffer* input_)
: guessing(0)
, input(input_)
, inputResponsible(true)
{
}

ParserInputState::ParserInputState(TokenBuffer& input_)
: guessing(0)
, input(&input_)
, inputResponsible(false)
{
}

ParserInputState::~ParserInputState()
{
	if (inputResponsible)
		delete input;
}

TokenBuffer& ParserInputState::getInput()
{
	return *input;
}

ANTLR_END_NAMESPACE

