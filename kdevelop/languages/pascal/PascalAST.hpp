#ifndef PASCALAST_HPP
#define PASCALAST_HPP

#include <antlr/CommonAST.hpp>

class PascalAST;
typedef antlr::ASTRefCount<PascalAST> RefPascalAST;

class PascalAST : public antlr::CommonAST {
public:
    PascalAST()
        : m_line(0), m_column(0) {}

    ~PascalAST() {}

    int getLine() const { return m_line; }
    void setLine( int line ) { m_line = line; }

    int getColumn() const { return m_column; }
    void setColumn( int column ) { m_column = column; }

    void initialize( antlr::RefToken t ) {
            antlr::CommonAST::initialize(t);
            m_line = t->getLine() - 1;
            m_column = t->getColumn() - 1;
    }

    void initialize(int t,const ANTLR_USE_NAMESPACE(std)string& txt) {
            setType(t);
            setText(txt);
            m_line = 0;
            m_column = 0;
        }

    void addChild( RefPascalAST c ) {
        antlr::RefAST n( c.get() );
        antlr::BaseAST::addChild( n );
    }

    static antlr::RefAST factory( void ) {
        RefPascalAST n(new PascalAST);
        return n.get();
    }

private:
    int m_line;
    int m_column;
};


#endif
