#ifndef INC_LexerSharedInputState_hpp__
#define INC_LexerSharedInputState_hpp__

#include "antlr/config.hpp"
#include "antlr/InputBuffer.hpp"
#include "antlr/RefCount.hpp"
#include <string>

ANTLR_BEGIN_NAMESPACE(antlr)

/** This object contains the data associated with an
 *  input stream of characters.  Multiple lexers
 *  share a single LexerSharedInputState to lex
 *  the same input stream.
 */
class LexerInputState {
public:
	LexerInputState(InputBuffer* inbuf);
	LexerInputState(InputBuffer& inbuf);
	LexerInputState(ANTLR_USE_NAMESPACE(std)istream& in);
	~LexerInputState();

	int column;
	int line;
	int tokenStartColumn;
	int tokenStartLine;
	int guessing;
	/** What file (if known) caused the problem? */
	ANTLR_USE_NAMESPACE(std)string filename;
	InputBuffer& getInput();
private:
	InputBuffer* input;
	bool inputResponsible;

	// we don't want these:
	LexerInputState(const LexerInputState&);
	LexerInputState& operator=(const LexerInputState&);
};

typedef RefCount<LexerInputState> LexerSharedInputState;

inline InputBuffer& LexerInputState::getInput()
{
	return *input;
}

ANTLR_END_NAMESPACE

#endif //INC_LexerSharedInputState_hpp__
