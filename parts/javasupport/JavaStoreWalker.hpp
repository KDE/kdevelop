#ifndef INC_JavaStoreWalker_hpp_
#define INC_JavaStoreWalker_hpp_

#line 3 "java.store.g"

	#include "classstore.h"
	#include "JavaAST.hpp"

	#include <qstring.h>
	#include <qstringlist.h>
        #include <qfileinfo.h>

#line 14 "JavaStoreWalker.hpp"
#include "antlr/config.hpp"
#include "JavaStoreWalkerTokenTypes.hpp"
/* $ANTLR 2.7.1: "java.store.g" -> "JavaStoreWalker.hpp"$ */
#include "antlr/TreeParser.hpp"

#line 12 "java.store.g"

	#include "parsedmethod.h"
	#include "parsedclass.h"
	#include "parsedattribute.h"
	#include "parsedargument.h"

	#include <kdebug.h>    

#line 29 "JavaStoreWalker.hpp"
/** Java 1.2 AST Recognizer Grammar
 *
 * Author:
 *	Terence Parr	parrt@magelang.com
 *
 * Version tracking now done with following ID:
 *
 * $Id$
 *
 * This grammar is in the PUBLIC DOMAIN
 *
 * BUGS
 */
class JavaStoreWalker : public ANTLR_USE_NAMESPACE(antlr)TreeParser, public JavaStoreWalkerTokenTypes
 {
#line 45 "java.store.g"

private:
	ClassStore* m_store;
	QString m_package;
	ANTLR_USE_NAMESPACE(std)string m_filename;

public:
	void setClassStore( ClassStore* store )			{ m_store = store; }
	ClassStore* classStore()				{ return m_store; }
	const ClassStore* classStore() const			{ return m_store; }

	ANTLR_USE_NAMESPACE(std)string getFilename() const	{ return m_filename; }
	void setFilename( const ANTLR_USE_NAMESPACE(std)string& filename ) { m_filename = filename; }

	void init(){
		m_package = QString::null;
	}

	void wipeout()						{ m_store->wipeout(); }
	void out()						{ m_store->out(); }
	void removeWithReferences( const QString& fileName )	{ m_store->removeWithReferences( fileName ); }
#line 46 "JavaStoreWalker.hpp"
public:
	JavaStoreWalker();
	public: void compilationUnit(RefJavaAST _t);
	public: void packageDefinition(RefJavaAST _t);
	public:  QString  importDefinition(RefJavaAST _t);
	public: ParsedClass*  typeDefinition(RefJavaAST _t);
	public:  QString  identifier(RefJavaAST _t);
	public:  QString  identifierStar(RefJavaAST _t);
	public:  QStringList  modifiers(RefJavaAST _t);
	public:  QStringList  extendsClause(RefJavaAST _t);
	public:  QStringList  implementsClause(RefJavaAST _t);
	public: void objBlock(RefJavaAST _t,
		 ParsedClass* klass 
	);
	public: void interfaceBlock(RefJavaAST _t,
		 ParsedClass* klass 
	);
	public:  QString  typeSpec(RefJavaAST _t);
	public:  QString  typeSpecArray(RefJavaAST _t);
	public:  QString  type(RefJavaAST _t);
	public: void builtInType(RefJavaAST _t);
	public: void modifier(RefJavaAST _t);
	public:  ParsedMethod*  methodDecl(RefJavaAST _t);
	public:  ParsedAttribute*  variableDef(RefJavaAST _t);
	public:  ParsedMethod*  ctorDef(RefJavaAST _t);
	public:  ParsedMethod*  methodDef(RefJavaAST _t);
	public: void slist(RefJavaAST _t);
	public: void methodHead(RefJavaAST _t,
		 ParsedMethod* meth 
	);
	public: void variableDeclarator(RefJavaAST _t,
		 ParsedAttribute* attr 
	);
	public: void varInitializer(RefJavaAST _t);
	public:  ParsedArgument*  parameterDef(RefJavaAST _t);
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
	static const char* _tokenNames[];
	
	static const unsigned long _tokenSet_0_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_0;
	static const unsigned long _tokenSet_1_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_1;
};

#endif /*INC_JavaStoreWalker_hpp_*/
