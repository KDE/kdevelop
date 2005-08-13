#ifndef INC_JavaStoreWalker_hpp_
#define INC_JavaStoreWalker_hpp_

#line 3 "java.store.g"

	#include <codemodel.h>
	#include "JavaAST.hpp"

	#include <qstring.h>
	#include <qstringlist.h>
	#include <qvaluestack.h>
        #include <qfileinfo.h>

#line 15 "JavaStoreWalker.hpp"
#include <antlr/config.hpp>
#include "JavaStoreWalkerTokenTypes.hpp"
/* $ANTLR 2.7.2: "java.store.g" -> "JavaStoreWalker.hpp"$ */
#include <antlr/TreeParser.hpp>

#line 13 "java.store.g"

	#include <codemodel.h>

	#include <kdebug.h>

#line 27 "JavaStoreWalker.hpp"
/** Java 1.2 AST Recognizer Grammar
 *
 * Author:
 *	Terence Parr	parrt@magelang.com
 *
 * Version tracking now done with following ID:
 *
 *
 * This grammar is in the PUBLIC DOMAIN
 *
 * BUGS
 */
class JavaStoreWalker : public ANTLR_USE_NAMESPACE(antlr)TreeParser, public JavaStoreWalkerTokenTypes
{
#line 43 "java.store.g"

private:
	QStringList m_currentScope;
	CodeModel* m_model;
	FileDom m_file;
	QValueStack<ClassDom> m_currentClass;
	int m_currentAccess;
	int m_anon;
        ANTLR_USE_NAMESPACE(antlr)JavaASTFactory ast_factory;

public:
	void setCodeModel( CodeModel* model )
	{
		m_model = model;
	}

	void setFile( FileDom file )
	{
		m_file = file;
	}

	void init()
	{
		m_currentScope.clear();
		m_currentAccess = CodeModelItem::Public;
		m_anon = 0;

        	initializeASTFactory (ast_factory);
        	setASTFactory (&ast_factory);
	}
#line 44 "JavaStoreWalker.hpp"
public:
	JavaStoreWalker();
	void initializeASTFactory( ANTLR_USE_NAMESPACE(antlr)ASTFactory& factory );
	int getNumTokens() const
	{
		return JavaStoreWalker::NUM_TOKENS;
	}
	const char* getTokenName( int type ) const
	{
		if( type > getNumTokens() ) return 0;
		return JavaStoreWalker::tokenNames[type];
	}
	public: void compilationUnit(RefJavaAST _t);
	public:  QString  packageDefinition(RefJavaAST _t);
	public:  QString  importDefinition(RefJavaAST _t);
	public: void typeDefinition(RefJavaAST _t);
	public:  QString  identifier(RefJavaAST _t);
	public:  QString  identifierStar(RefJavaAST _t);
	public:  QStringList  modifiers(RefJavaAST _t);
	public:  QStringList  extendsClause(RefJavaAST _t);
	public:  QStringList  implementsClause(RefJavaAST _t);
	public: void objBlock(RefJavaAST _t,
		 ClassDom klass 
	);
	public: void interfaceBlock(RefJavaAST _t,
		 ClassDom klass 
	);
	public:  QString  typeSpec(RefJavaAST _t);
	public:  QString  typeSpecArray(RefJavaAST _t);
	public:  QString  type(RefJavaAST _t);
	public: void builtInType(RefJavaAST _t);
	public: void modifier(RefJavaAST _t);
	public:  FunctionDom  methodDecl(RefJavaAST _t);
	public:  VariableDom  variableDef(RefJavaAST _t);
	public:  FunctionDom  ctorDef(RefJavaAST _t);
	public:  FunctionDom  methodDef(RefJavaAST _t);
	public: void slist(RefJavaAST _t);
	public: void methodHead(RefJavaAST _t,
		 FunctionDom meth 
	);
	public: void variableDeclarator(RefJavaAST _t,
		 VariableDom attr 
	);
	public: void varInitializer(RefJavaAST _t);
	public:  ArgumentDom  parameterDef(RefJavaAST _t);
	public: void objectinitializer(RefJavaAST _t);
	public: void initializer(RefJavaAST _t);
	public: void expression(RefJavaAST _t);
	public: void arrayInitializer(RefJavaAST _t);
	public: void throwsClause(RefJavaAST _t);
	public: void stat(RefJavaAST _t);
	public: void elist(RefJavaAST _t);
	public: void caseGroup(RefJavaAST _t);
	public: void tryBlock(RefJavaAST _t);
	public: void handler(RefJavaAST _t);
	public: void expr(RefJavaAST _t);
	public: void primaryExpression(RefJavaAST _t);
	public: void arrayIndex(RefJavaAST _t);
	public: void newExpression(RefJavaAST _t);
	public: void constant(RefJavaAST _t);
	public: void newArrayDeclarator(RefJavaAST _t);
public:
	RefJavaAST getAST();
	
protected:
	RefJavaAST returnAST;
	RefJavaAST _retTree;
private:
	static const char* tokenNames[];
#ifndef NO_STATIC_CONSTS
	static const int NUM_TOKENS = 153;
#else
	enum {
		NUM_TOKENS = 153
	};
#endif
	
	static const unsigned long _tokenSet_0_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_0;
	static const unsigned long _tokenSet_1_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_1;
};

#endif /*INC_JavaStoreWalker_hpp_*/
