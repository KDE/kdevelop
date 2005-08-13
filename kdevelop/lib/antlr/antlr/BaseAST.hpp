#ifndef INC_BaseAST_hpp__
#define INC_BaseAST_hpp__

/* ANTLR Translator Generator
 * Project led by Terence Parr at http://www.jGuru.com
 * Software rights: http://www.antlr.org/RIGHTS.html
 *
 */

#include <antlr/config.hpp>
#include <antlr/AST.hpp>

#ifdef ANTLR_CXX_SUPPORTS_NAMESPACE
namespace antlr {
#endif

class ANTLR_API BaseAST;
typedef ASTRefCount<BaseAST> RefBaseAST;

class ANTLR_API BaseAST : public AST {
public:
	BaseAST();
	BaseAST(const BaseAST& other);

	virtual ~BaseAST();

	/// Return the class name
	virtual const char* typeName( void ) const;

	/// Clone this AST node.
	virtual RefAST clone( void ) const;

   /// Is node t equal to this in terms of token type and text?
	virtual bool equals(RefAST t) const;

   /** Is t an exact structural and equals() match of this tree. The
	 * 'this' reference is considered the start of a sibling list.
	 */
	virtual bool equalsList(RefAST t) const;

   /** Is 't' a subtree of this list? The siblings of the root are NOT ignored.
    */
	virtual bool equalsListPartial(RefAST t) const;

	/** Is tree rooted at 'this' equal to 't'?  The siblings of 'this' are
	 * ignored.
	 */
	virtual bool equalsTree(RefAST t) const;

	/** Is 't' a subtree of the tree rooted at 'this'? The siblings of
	 * 'this' are ignored.
	 */
	virtual bool equalsTreePartial(RefAST t) const;

	/** Walk the tree looking for all exact subtree matches.  Return
	 *  an ASTEnumerator that lets the caller walk the list
	 *  of subtree roots found herein.
	 */
	virtual ANTLR_USE_NAMESPACE(std)vector<RefAST> findAll(RefAST t);

   /** Walk the tree looking for all subtrees.  Return
    *  an ASTEnumerator that lets the caller walk the list
    *  of subtree roots found herein.
    */
	virtual ANTLR_USE_NAMESPACE(std)vector<RefAST> findAllPartial(RefAST t);

   /// Add a node to the end of the child list for this node
	virtual void addChild(RefAST c);
	/// Get the first child of this node; null if no children
	virtual RefAST getFirstChild() const
	{
		return RefAST(down);
	}
	/// Get  the next sibling in line after this one
	virtual RefAST getNextSibling() const
	{
		return RefAST(right);
	}

	/// Get the token text for this node
	virtual ANTLR_USE_NAMESPACE(std)string getText() const
	{
		return "";
	}
	/// Get the token type for this node
	virtual int getType() const
	{
		return 0;
	}

	/// Remove all children
	virtual void removeChildren()
	{
		down = static_cast<BaseAST*>(static_cast<AST*>(nullAST));
	}

	/// Set the first child of a node.
	virtual void setFirstChild(RefAST c)
	{
		down = static_cast<BaseAST*>(static_cast<AST*>(c));
	}

	/// Set the next sibling after this one.
	void setNextSibling(RefAST n)
	{
		right = static_cast<BaseAST*>(static_cast<AST*>(n));
	}

	/// Set the token text for this node
	virtual void setText(const ANTLR_USE_NAMESPACE(std)string& txt);

	/// Set the token type for this node
	virtual void setType(int type);

#ifdef ANTLR_SUPPORT_XML
	/** print attributes of this node to 'out'. Override to customize XML
	 * output.
	 * @param out the stream to write the AST attributes to.
	 */
	virtual bool attributesToStream( ANTLR_USE_NAMESPACE(std)ostream& out ) const;
	/** Write this subtree to a stream. Overload this one to customize the XML
	 * output for AST derived AST-types
	 * @param output stream
	 */
	virtual void toStream( ANTLR_USE_NAMESPACE(std)ostream &out ) const;
#endif

	/// Return string representation for the AST
	virtual ANTLR_USE_NAMESPACE(std)string toString() const;

	/// Print out a child sibling tree in LISP notation
	virtual ANTLR_USE_NAMESPACE(std)string toStringList() const;
	virtual ANTLR_USE_NAMESPACE(std)string toStringTree() const;

protected:
	RefBaseAST down;
	RefBaseAST right;
private:
	void doWorkForFindAll(ANTLR_USE_NAMESPACE(std)vector<RefAST>& v,
								 RefAST target,
								 bool partialMatch);
};

/** Is node t equal to this in terms of token type and text?
 */
inline bool BaseAST::equals(RefAST t) const
{
	if (!t)
		return false;
	return ((getType() == t->getType()) && (getText() == t->getText()));
}

#ifdef ANTLR_CXX_SUPPORTS_NAMESPACE
}
#endif

#endif //INC_BaseAST_hpp__
