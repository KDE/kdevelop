#ifndef JAVAAST_HPP
#define JAVAAST_HPP

#include <antlr/CommonAST.hpp>
#include <antlr/ASTFactory.hpp>

class JavaAST;
typedef ANTLR_USE_NAMESPACE(antlr)ASTRefCount<JavaAST> RefJavaAST;

class JavaAST : public ANTLR_USE_NAMESPACE(antlr)CommonAST
{
public:
    JavaAST()
        : m_line(0), m_column(0) {}

    virtual ~JavaAST() {}

    int getLine() const { return m_line; }
    void setLine( int line ) { m_line = line; }

    int getColumn() const { return m_column; }
    void setColumn( int column ) { m_column = column; }

    void initialize( ANTLR_USE_NAMESPACE(antlr)RefToken t )
    {
	CommonAST::initialize(t);
	m_line = t->getLine() - 1;
	m_column = t->getColumn() - 1;
    }

    void initialize( ANTLR_USE_NAMESPACE(antlr)RefAST t )
    {
	CommonAST::initialize( t );

	m_line = 0;
	m_column = 0;

	RefJavaAST a( dynamic_cast<JavaAST*>(t.get()) );
	m_line = a->getLine();
	m_column = a->getColumn();
    }

    void initialize(int t, const ANTLR_USE_NAMESPACE(std)string& txt)
    {
        CommonAST::initialize( t, txt );
	m_line = 0;
	m_column = 0;
    }

    static ANTLR_USE_NAMESPACE(antlr)RefAST factory()
    {
        RefJavaAST n(new JavaAST);
        return n.get();
    }


private:
    int m_line;
    int m_column;

private:
    JavaAST( const JavaAST& source );
    void operator = ( const JavaAST& source );
};

namespace antlr
{

class JavaASTFactory: public ASTFactory
{
public:
    JavaASTFactory(): ASTFactory( "JavaAST", JavaAST::factory ) {}
};

} // namespace antlr

#endif
