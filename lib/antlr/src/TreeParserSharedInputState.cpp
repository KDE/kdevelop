#include "antlr/TreeParserSharedInputState.hpp"

ANTLR_BEGIN_NAMESPACE(antlr)

/** This object contains the data associated with an
 *  input AST.  Multiple parsers
 *  share a single TreeParserSharedInputState to parse
 *  the same tree or to have the parser walk multiple
 *  trees.
 */

TreeParserInputState::TreeParserInputState()
: guessing(0)
{
}

TreeParserInputState::~TreeParserInputState()
{
}

ANTLR_END_NAMESPACE

