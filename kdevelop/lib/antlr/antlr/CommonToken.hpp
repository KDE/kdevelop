#ifndef INC_CommonToken_hpp__
#define INC_CommonToken_hpp__

/* ANTLR Translator Generator
 * Project led by Terence Parr at http://www.jGuru.com
 * Software rights: http://www.antlr.org/RIGHTS.html
 *
 */

#include <antlr/config.hpp>
#include <antlr/Token.hpp>
#include <string>

#ifdef ANTLR_CXX_SUPPORTS_NAMESPACE
namespace antlr {
#endif

class ANTLR_API CommonToken : public Token {
public:
	CommonToken();
	CommonToken(int t, const ANTLR_USE_NAMESPACE(std)string& txt);
	CommonToken(const ANTLR_USE_NAMESPACE(std)string& s);

	/// return contents of token
	ANTLR_USE_NAMESPACE(std)string getText() const
	{
		return text;
	}

	/// set contents of token
	void setText(const ANTLR_USE_NAMESPACE(std)string& s)
	{
		text = s;
	}

	/** get the line the token is at (starting at 1)
	 * @see CharScanner::newline()
	 * @see CharScanner::tab()
	 */
	int getLine() const
	{
		return line;
	}
	/** gt the column the token is at (starting at 1)
	 * @see CharScanner::newline()
	 * @see CharScanner::tab()
	 */
	int getColumn() const
	{
		return col;
	}

	/// set line for token
	void setLine(int l)
	{
		line = l;
	}
	/// set column for token
	void setColumn(int c)
	{
		col = c;
	}

	bool isInvalid() const
	{
		return type==INVALID_TYPE;
	}

	ANTLR_USE_NAMESPACE(std)string toString() const;
	static RefToken factory();

protected:
	// most tokens will want line and text information
	int line;
	int col;
	ANTLR_USE_NAMESPACE(std)string text;

private:
	CommonToken(const CommonToken&);
	const CommonToken& operator=(const CommonToken&);
};

#ifdef ANTLR_CXX_SUPPORTS_NAMESPACE
}
#endif

#endif //INC_CommonToken_hpp__
