#ifndef ADAAST_HPP
#define ADAAST_HPP

#include <antlr/CommonAST.hpp>

class AdaAST;
typedef antlr::ASTRefCount<AdaAST> RefAdaAST;

class AdaAST : public antlr::CommonAST {
public:
    AdaAST() : m_line (0), m_column (0) {}

    ~AdaAST() {}

    int getLine () const { return m_line; }
    void setLine (int line) { m_line = line; }

    int getColumn () const { return m_column; }
    void setColumn (int column) { m_column = column; }

    void initialize (antlr::RefToken t) {
        antlr::CommonAST::initialize (t);
        m_line = t->getLine () - 1;
        m_column = t->getColumn () - 1;
    }

    void initialize (int t, const std::string& txt) {
        setType (t);
        setText (txt);
        m_line = 0;
        m_column = 0;
    }

    RefAdaAST duplicate (void) const {
        AdaAST *ast = new AdaAST (*this);
        return RefAdaAST (ast);
    }

    void set (int t, const std::string& txt) {
        setType (t);
        setText (txt);
    }

    void addChild (RefAdaAST c) {
        antlr::RefAST n( c.get() );
        antlr::BaseAST::addChild (n);
    }

    RefAdaAST down () const {
        return RefAdaAST (antlr::BaseAST::getFirstChild ());
    }

    RefAdaAST right () const {
        return RefAdaAST (antlr::BaseAST::getNextSibling ());
    }

    /* bool equals(RefAdaAST t) const {
        return ((antlr::BaseAST*)this)->equals (t);
    } */

    static antlr::RefAST factory (void) {
      RefAdaAST n( new AdaAST );
      return n.get();
    }

    static const RefAdaAST nullAdaAST;

private:
    int m_line;
    int m_column;
};

#define Set(n, t) (n)->set(t, #t)

#endif
