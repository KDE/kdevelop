#ifndef INC_TreeParserSharedInputState_hpp__
#define INC_TreeParserSharedInputState_hpp__

#include "antlr/config.hpp"
#include "antlr/RefCount.hpp"

ANTLR_BEGIN_NAMESPACE(antlr)

/** This object contains the data associated with an
 *  input AST.  Multiple parsers
 *  share a single TreeParserSharedInputState to parse
 *  the same tree or to have the parser walk multiple
 *  trees.
 */
class TreeParserInputState {
public:
	TreeParserInputState();
	~TreeParserInputState();

public:
	/** Are we guessing (guessing>0)? */
	int guessing; //= 0;

private:
	// we don't want these:
	TreeParserInputState(const TreeParserInputState&);
	TreeParserInputState& operator=(const TreeParserInputState&);
};

typedef RefCount<TreeParserInputState> TreeParserSharedInputState;

ANTLR_END_NAMESPACE

#endif //INC_TreeParserSharedInputState_hpp__
