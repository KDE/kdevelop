#ifndef INC_JavaStoreWalker_hpp_
#define INC_JavaStoreWalker_hpp_

#line 3 "java.store.g"

	#include "classstore.h"

	#include <qstring.h>
	#include <qstringlist.h>

#line 12 "JavaStoreWalker.hpp"
#include "antlr/config.hpp"
#include "JavaStoreWalkerTokenTypes.hpp"
/* $ANTLR 2.7.1: "java.store.g" -> "JavaStoreWalker.hpp"$ */
#include "antlr/TreeParser.hpp"

#line 10 "java.store.g"

	#include "parsedmethod.h"
	#include "parsedclass.h"
	#include "parsedattribute.h"

	#include <kdebug.h>

#line 26 "JavaStoreWalker.hpp"
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
#line 41 "java.store.g"

private:
	ClassStore* m_store;
	QString m_package;
	ANTLR_USE_NAMESPACE(std)string m_filename;
	ParsedClassContainer* global_ns;

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
#line 43 "JavaStoreWalker.hpp"
public:
	JavaStoreWalker();
	public: void compilationUnit(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void packageDefinition(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public:  QString  importDefinition(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: ParsedClass*  typeDefinition(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public:  QString  identifier(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public:  QString  identifierStar(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public:  QStringList  modifiers(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public:  QStringList  extendsClause(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public:  QStringList  implementsClause(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void objBlock(ANTLR_USE_NAMESPACE(antlr)RefAST _t,
		 ParsedClass* klass 
	);
	public: void interfaceBlock(ANTLR_USE_NAMESPACE(antlr)RefAST _t,
		 ParsedClass* klass 
	);
	public: void typeSpec(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void typeSpecArray(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public:  QString  type(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void builtInType(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void modifier(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public:  ParsedMethod*  methodDecl(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public:  ParsedAttribute*  variableDef(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public:  ParsedMethod*  ctorDef(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public:  ParsedMethod*  methodDef(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void slist(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void methodHead(ANTLR_USE_NAMESPACE(antlr)RefAST _t,
		 ParsedMethod* meth 
	);
	public: void variableDeclarator(ANTLR_USE_NAMESPACE(antlr)RefAST _t,
		 ParsedAttribute* attr 
	);
	public: void varInitializer(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void parameterDef(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void objectinitializer(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void initializer(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void expression(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void arrayInitializer(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void throwsClause(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void stat(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void elist(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void caseGroup(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void tryBlock(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void handler(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void expr(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void primaryExpression(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void arrayIndex(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void newExpression(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void constant(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
	public: void newArrayDeclarator(ANTLR_USE_NAMESPACE(antlr)RefAST _t);
private:
	static const char* _tokenNames[];
	
	static const unsigned long _tokenSet_0_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_0;
	static const unsigned long _tokenSet_1_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_1;
};

#endif /*INC_JavaStoreWalker_hpp_*/
