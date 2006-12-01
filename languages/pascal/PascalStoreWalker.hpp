#ifndef INC_PascalStoreWalker_hpp_
#define INC_PascalStoreWalker_hpp_

#line 20 "pascal.tree.g"

	#include <codemodel.h>
        #include "PascalAST.hpp"

        #include <qstring.h>
        #include <qstringlist.h>
        #include <qfileinfo.h>

#line 14 "PascalStoreWalker.hpp"
#include <antlr/config.hpp>
#include "PascalStoreWalkerTokenTypes.hpp"
/* $ANTLR 2.7.2: "pascal.tree.g" -> "PascalStoreWalker.hpp"$ */
#include <antlr/TreeParser.hpp>

#line 29 "pascal.tree.g"

	#include <codemodel.h>
        #include <kdebug.h>

#line 25 "PascalStoreWalker.hpp"
class PascalStoreWalker : public ANTLR_USE_NAMESPACE(antlr)TreeParser, public PascalStoreWalkerTokenTypes
{
#line 47 "pascal.tree.g"

private:
        QString m_fileName;
        QStringList m_currentScope;
        int m_currentAccess;
        int m_anon;
	CodeModel* m_model;

public:
        void setCodeModel( CodeModel* model )			{ m_model = model; }
        CodeModel* codeModel()					{ return m_model; }
        const CodeModel* codeModel() const			{ return m_model; }

        QString fileName() const				{ return m_fileName; }
        void setFileName( const QString& fileName )		{ m_fileName = fileName; }

        void init(){
                m_currentScope.clear();
                m_currentAccess = CodeModelItem::Public;
                m_anon = 0;
        }

        void wipeout()						{ m_model->wipeout(); }
#line 29 "PascalStoreWalker.hpp"
public:
	PascalStoreWalker();
	void initializeASTFactory( ANTLR_USE_NAMESPACE(antlr)ASTFactory& factory );
	int getNumTokens() const
	{
		return PascalStoreWalker::NUM_TOKENS;
	}
	const char* getTokenName( int type ) const
	{
		if( type > getNumTokens() ) return 0;
		return PascalStoreWalker::tokenNames[type];
	}
	public: void program(RefPascalAST _t);
	public: void programHeading(RefPascalAST _t);
	public: void block(RefPascalAST _t);
	public: void identifierList(RefPascalAST _t);
	public: void identifier(RefPascalAST _t);
	public: void labelDeclarationPart(RefPascalAST _t);
	public: void constantDefinitionPart(RefPascalAST _t);
	public: void typeDefinitionPart(RefPascalAST _t);
	public: void variableDeclarationPart(RefPascalAST _t);
	public: void procedureAndFunctionDeclarationPart(RefPascalAST _t);
	public: void usesUnitsPart(RefPascalAST _t);
	public: void compoundStatement(RefPascalAST _t);
	public: void label(RefPascalAST _t);
	public: void constantDefinition(RefPascalAST _t);
	public: void constant(RefPascalAST _t);
	public: void string(RefPascalAST _t);
	public: void typeDefinition(RefPascalAST _t);
	public: void type(RefPascalAST _t);
	public: void formalParameterList(RefPascalAST _t);
	public: void resultType(RefPascalAST _t);
	public: void typeIdentifier(RefPascalAST _t);
	public: void structuredType(RefPascalAST _t);
	public: void unpackedStructuredType(RefPascalAST _t);
	public: void arrayType(RefPascalAST _t);
	public: void recordType(RefPascalAST _t);
	public: void setType(RefPascalAST _t);
	public: void fileType(RefPascalAST _t);
	public: void typeList(RefPascalAST _t);
	public: void fieldList(RefPascalAST _t);
	public: void fixedPart(RefPascalAST _t);
	public: void variantPart(RefPascalAST _t);
	public: void recordSection(RefPascalAST _t);
	public: void tag(RefPascalAST _t);
	public: void variant(RefPascalAST _t);
	public: void constList(RefPascalAST _t);
	public: void variableDeclaration(RefPascalAST _t);
	public: void procedureOrFunctionDeclaration(RefPascalAST _t);
	public: void procedureDeclaration(RefPascalAST _t);
	public: void functionDeclaration(RefPascalAST _t);
	public: void formalParameterSection(RefPascalAST _t);
	public: void parameterGroup(RefPascalAST _t);
	public: void statement(RefPascalAST _t);
	public: void unlabelledStatement(RefPascalAST _t);
	public: void simpleStatement(RefPascalAST _t);
	public: void structuredStatement(RefPascalAST _t);
	public: void assignmentStatement(RefPascalAST _t);
	public: void procedureStatement(RefPascalAST _t);
	public: void gotoStatement(RefPascalAST _t);
	public: void variable(RefPascalAST _t);
	public: void expression(RefPascalAST _t);
	public: void functionDesignator(RefPascalAST _t);
	public: void set(RefPascalAST _t);
	public: void parameterList(RefPascalAST _t);
	public: void actualParameter(RefPascalAST _t);
	public: void element(RefPascalAST _t);
	public: void conditionalStatement(RefPascalAST _t);
	public: void repetetiveStatement(RefPascalAST _t);
	public: void withStatement(RefPascalAST _t);
	public: void statements(RefPascalAST _t);
	public: void ifStatement(RefPascalAST _t);
	public: void caseStatement(RefPascalAST _t);
	public: void caseListElement(RefPascalAST _t);
	public: void whileStatement(RefPascalAST _t);
	public: void repeatStatement(RefPascalAST _t);
	public: void forStatement(RefPascalAST _t);
	public: void forList(RefPascalAST _t);
	public: void initialValue(RefPascalAST _t);
	public: void finalValue(RefPascalAST _t);
	public: void recordVariableList(RefPascalAST _t);
public:
	RefPascalAST getAST();
	
protected:
	RefPascalAST returnAST;
	RefPascalAST _retTree;
private:
	static const char* tokenNames[];
#ifndef NO_STATIC_CONSTS
	static const int NUM_TOKENS = 190;
#else
	enum {
		NUM_TOKENS = 190
	};
#endif
	
	static const unsigned long _tokenSet_0_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_0;
	static const unsigned long _tokenSet_1_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_1;
	static const unsigned long _tokenSet_2_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_2;
	static const unsigned long _tokenSet_3_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_3;
	static const unsigned long _tokenSet_4_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_4;
	static const unsigned long _tokenSet_5_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_5;
	static const unsigned long _tokenSet_6_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_6;
};

#endif /*INC_PascalStoreWalker_hpp_*/
