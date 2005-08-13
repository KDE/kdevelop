#ifndef INC_Parser_hpp__
#define INC_Parser_hpp__

/* ANTLR Translator Generator
 * Project led by Terence Parr at http://www.jGuru.com
 * Software rights: http://www.antlr.org/RIGHTS.html
 *
 */

#include <antlr/config.hpp>
#include <antlr/BitSet.hpp>
#include <antlr/TokenBuffer.hpp>
#include <antlr/RecognitionException.hpp>
#include <antlr/ASTFactory.hpp>
#include <antlr/ParserSharedInputState.hpp>

#include <exception>

#ifdef ANTLR_CXX_SUPPORTS_NAMESPACE
namespace antlr {
#endif

extern bool DEBUG_PARSER;

/** A generic ANTLR parser (LL(k) for k>=1) containing a bunch of
 * utility routines useful at any lookahead depth.  We distinguish between
 * the LL(1) and LL(k) parsers because of efficiency.  This may not be
 * necessary in the near future.
 *
 * Each parser object contains the state of the parse including a lookahead
 * cache (the form of which is determined by the subclass), whether or
 * not the parser is in guess mode, where tokens come from, etc...
 *
 * <p>
 * During <b>guess</b> mode, the current lookahead token(s) and token type(s)
 * cache must be saved because the token stream may not have been informed
 * to save the token (via <tt>mark</tt>) before the <tt>try</tt> block.
 * Guessing is started by:
 * <ol>
 * <li>saving the lookahead cache.
 * <li>marking the current position in the TokenBuffer.
 * <li>increasing the guessing level.
 * </ol>
 *
 * After guessing, the parser state is restored by:
 * <ol>
 * <li>restoring the lookahead cache.
 * <li>rewinding the TokenBuffer.
 * <li>decreasing the guessing level.
 * </ol>
 *
 * @see antlr.Token
 * @see antlr.TokenBuffer
 * @see antlr.TokenStream
 * @see antlr.LL1Parser
 * @see antlr.LLkParser
 *
 * @todo add constructors with ASTFactory.
 */
class ANTLR_API Parser {
protected:
	Parser(TokenBuffer& input_);
	Parser(TokenBuffer* input_);

	Parser(const ParserSharedInputState& state);
public:
	virtual ~Parser();

	/** Return the token type of the ith token of lookahead where i=1
	 * is the current token being examined by the parser (i.e., it
	 * has not been matched yet).
	 */
	virtual int LA(int i)=0;

	/// Return the i-th token of lookahead
	virtual RefToken LT(int i)=0;

	/** DEPRECATED! Specify the factory to be used during tree building. (Compulsory)
	 * Setting the factory is nowadays compulsory.
	 * @see setASTFactory
	 */
	virtual void setASTNodeFactory( ASTFactory *factory )
	{
		astFactory = factory;
	}
	/** Specify the factory to be used during tree building. (Compulsory)
	 * Setting the factory is nowadays compulsory.
	 */
	virtual void setASTFactory( ASTFactory *factory )
	{
		astFactory = factory;
	}
	/** Return a pointer to the ASTFactory used.
	 * So you might use it in subsequent treewalkers or to reload AST's
	 * from disk.
	 */
	virtual ASTFactory* getASTFactory()
	{
		return astFactory;
	}
	/// Get the root AST node of the generated AST.
	inline RefAST getAST()
	{
		return returnAST;
	}

	/// Return the filename of the input file.
	virtual inline ANTLR_USE_NAMESPACE(std)string getFilename() const
	{
		return inputState->filename;
	}
	/// Set the filename of the input file (used for error reporting).
	virtual void setFilename(const ANTLR_USE_NAMESPACE(std)string& f)
	{
		inputState->filename = f;
	}

	virtual void setInputState(ParserSharedInputState state)
	{
		inputState = state;
	}
	virtual inline ParserSharedInputState getInputState() const
	{
		return inputState;
	}

	/// Get another token object from the token stream
	virtual void consume()=0;
	/// Consume tokens until one matches the given token
	virtual void consumeUntil(int tokenType);
	/// Consume tokens until one matches the given token set
	virtual void consumeUntil(const BitSet& set);

	/** Make sure current lookahead symbol matches token type <tt>t</tt>.
	 * Throw an exception upon mismatch, which is catch by either the
	 * error handler or by the syntactic predicate.
	 */
	virtual void match(int t);
	virtual void matchNot(int t);
	/** Make sure current lookahead symbol matches the given set
	 * Throw an exception upon mismatch, which is catch by either the
	 * error handler or by the syntactic predicate.
	 */
	virtual void match(const BitSet& b);

	/** Mark a spot in the input and return the position.
	 * Forwarded to TokenBuffer.
	 */
	virtual inline int mark()
	{
		return inputState->getInput().mark();
	}
	/// rewind to a previously marked position
	virtual inline void rewind(int pos)
	{
		inputState->getInput().rewind(pos);
	}

	/// Parser error-reporting function can be overridden in subclass
	virtual void reportError(const RecognitionException& ex);
	/// Parser error-reporting function can be overridden in subclass
	virtual void reportError(const ANTLR_USE_NAMESPACE(std)string& s);
	/// Parser warning-reporting function can be overridden in subclass
	virtual void reportWarning(const ANTLR_USE_NAMESPACE(std)string& s);

	static void panic();

	/// get the token name for the token number 'num'
	virtual const char* getTokenName(int num) const = 0;
	/// get a vector with all token names
	virtual const char* const* getTokenNames() const = 0;
	/// get the number of tokens defined
	/** get the max token number
	 * This one should be overridden in subclasses.
	 */
	virtual int getNumTokens(void) const = 0;

	/** Set or change the input token buffer */
//	void setTokenBuffer(TokenBuffer<Token>* t);

	virtual void traceIndent();
	virtual void traceIn(const char* rname);
	virtual void traceOut(const char* rname);
protected:
//	void setTokenNames(const char** tokenNames_);

	ParserSharedInputState inputState;

	/// AST return value for a rule is squirreled away here
	RefAST returnAST;

	/// AST support code; parser and treeparser delegate to this object
	ASTFactory *astFactory;

	// used to keep track of the indentation for the trace
	int traceDepth;

	/** Utility class which allows tracing to work even when exceptions are
	 * thrown.
	 */
	class Tracer { /*{{{*/
	private:
		Parser* parser;
		const char* text;
	public:
		Tracer(Parser* p,const char * t)
		: parser(p), text(t)
		{
			parser->traceIn(text);
		}
		~Tracer()
		{
#ifdef ANTLR_CXX_SUPPORTS_UNCAUGHT_EXCEPTION
			// Only give trace if there's no uncaught exception..
			if(!ANTLR_USE_NAMESPACE(std)uncaught_exception())
#endif				
				parser->traceOut(text);
		}
	private:
		Tracer(const Tracer&);							// undefined
		const Tracer& operator=(const Tracer&);	// undefined
		/*}}}*/
	};
private:
	Parser(const Parser&);								// undefined
	const Parser& operator=(const Parser&);		// undefined
};

#ifdef ANTLR_CXX_SUPPORTS_NAMESPACE
}
#endif

#endif //INC_Parser_hpp__
