#ifndef INC_CommonASTWithHiddenTokens_hpp__
#define INC_CommonASTWithHiddenTokens_hpp__

/** A CommonAST whose initialization copies hidden token
 *  information from the Token used to create a node.
 */

#include "antlr/config.hpp"
#include "antlr/CommonAST.hpp"

ANTLR_BEGIN_NAMESPACE(antlr)

class CommonASTWithHiddenTokens : public CommonAST {
public:
	CommonASTWithHiddenTokens() : CommonAST() 
	{
	}
	virtual ~CommonASTWithHiddenTokens()
	{
	}
protected:
	RefToken hiddenBefore,hiddenAfter; // references to hidden tokens
public:
	virtual RefToken getHiddenAfter() const
		{ return hiddenAfter; }
	virtual RefToken getHiddenBefore() const
		{ return hiddenBefore; }

	// Borland C++ builder seems to need the decl's of the first two...
	virtual void initialize(int t,const ANTLR_USE_NAMESPACE(std)string& txt);
	virtual void initialize(RefAST t);
	virtual void initialize(RefToken t);

	static RefAST factory();
};

typedef ASTRefCount<CommonASTWithHiddenTokens> RefCommonASTWithHiddenTokens;

ANTLR_END_NAMESPACE

#endif //INC_CommonASTWithHiddenTokens_hpp__
