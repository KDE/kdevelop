/***************************************************************************
                        cppcodecompletion.h  -  description
                           -------------------
  begin                : Sat Jul 21 2001
  copyright            : (C) 2001 by Victor R�er
  email                : victor_roeder@gmx.de
  copyright            : (C) 2002,2003 by Roberto Raggi
  email                : roberto@kdevelop.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __CPPCODECOMPLETION_H__
#define __CPPCODECOMPLETION_H__

#include "cppsupportpart.h"

#include <ast.h>
#include <codemodel.h>

#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/codecompletioninterface.h>
#include <ktexteditor/texthintinterface.h>
#include <ktexteditor/cursorinterface.h>

#include <qobject.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qguardedptr.h>
#include <qregexp.h>


class CodeCompletionEntry;
class CodeInformationRepository;
class SimpleContext;
class SimpleType;
class CppCodeCompletionData;
class SimpleTypeConfiguration;
class TypeDesc;
struct PopupFillerHelpStruct;
struct PopupClassViewFillerHelpStruct;


struct DeclarationInfo {
    class File {
        QString m_file;
    public:
    File( const QString& file = "" ) : m_file( file ) {
    }
        
        operator QString() const {
            return m_file;
        }
    };
    
DeclarationInfo() : startLine(0), startCol(0), endLine(0), endCol(0) {
}
    
    operator bool() {
        return !name.isEmpty();
    }
    
    QString locationToText() const {
    return QString("line %1 col %2 - line %3 col %4\nfile: %5").arg(startLine).arg(startCol).arg(endLine).arg(endCol).arg(file);
    }
    
    QString toText() const {
        if( name.isEmpty() ) return "";
        
        QString ret;
        ret = QString("name: " + name + "\n" ) + locationToText();
        if( !comment.isEmpty() ) {
            ret += "\n\"" + comment + "\"";
        }
        return ret;
    }
    
    int startLine, startCol;
    int endLine, endCol;
    
    File file;
    QString name;
    QString comment;
};


class CppCodeCompletion : public QObject
{
	Q_OBJECT
public:
    struct ExpressionInfo;
    friend class SimpleType;
	enum CompletionMode
	{
	    NormalCompletion,
	    SignalCompletion,
	    SlotCompletion,
	    VirtualDeclCompletion
	};
	enum MemberAccessOp
	{
		NoOp,
		DotOp,
		ArrowOp
	};
    struct EvaluationResult;
    
public:
	CppCodeCompletion( CppSupportPart* part );
	virtual ~CppCodeCompletion();

	CodeInformationRepository* repository()
	{
		return m_repository;
	}
	CompletionMode completionMode() const
	{
		return m_completionMode;
	}

    QString replaceCppComments( const QString& contents );
    int expressionAt( const QString& text, int index );
	QStringList splitExpression( const QString& text );
	
	EvaluationResult evaluateExpression( ExpressionInfo expr, SimpleContext* ctx );

    static SimpleType typeName( QString name );

    EvaluationResult evaluateExpressionAt( int line, int column, SimpleTypeConfiguration& conf, bool ifUnknownSetType = false );
    
    void contextEvaluationMenus ( QPopupMenu *popup, const Context *context, int line, int col );
    
public slots:
	/**
	 * @param invokedOnDemand if true and there is exactly one matching entry
	 *        complete the match immediately without showing the completion box.
	 *        This is only true, when the users invokes the completion himself
	 *        (eg presses the completion shortcut CTRL+space)
	 */
	void completeText( bool invokedOnDemand = false );

private slots:
	void slotPartAdded( KParts::Part *part );
	void slotActivePartChanged( KParts::Part *part );
	void slotArgHintHidden();
	void slotCompletionBoxHidden();
	void slotTextChanged();
	void slotFileParsed( const QString& fileName );
    void slotTimeout();
    void slotStatusTextTimeout();
    void computeFileEntryList();
    bool isTypeExpression( const QString& expr );
	void slotTextHint( int line, int col, QString &text );
    void popupAction( int number );
    void popupClassViewAction( int number );
private:
    bool functionContains( FunctionDom f , int line, int col );
    void selectItem( ItemDom item );
    void addTypePopups( QPopupMenu* parent, TypeDesc d, QString depthAdd, QString prefix = "" );
    void addTypeClassPopups( QPopupMenu* parent, TypeDesc d, QString depthAdd, QString prefix = "" );
    QValueList<QStringList> computeSignatureList( EvaluationResult function );
	void integratePart( KParts::Part* part );
	void setupCodeInformationRepository();
	FunctionDefinitionAST* functionDefinition( AST* node );
	void computeRecoveryPoints();

    enum EvaluateExpressionOptions {
        IncludeStandardExpressions = 1,
        IncludeTypeExpression = 2,
        CompletionOption = 4, ///Cut off the last word because it is incomplete
	    SearchInFunctions = 8,
        SearchInClasses = 16,
        DefaultAsTypeExpression = 32, ///This makes the evaluation interpret any unidentified expression as a type-expression
        DefaultEvaluationOptions = 1 | 2 | 8 | 16,
        DefaultCompletionOptions = 1 | 4 | 8 | 16
    };
    
    bool mayBeTypeTail( int line, int column, QString& append, bool inFunction = false );
    bool canBeTypePrefix( const QString& prefix, bool inFunction = false );
    

    ExpressionInfo findExpressionAt( int line, int col, int startLine, int startCol, bool inFunction = false );
    SimpleContext* computeFunctionContext( FunctionDom f, int line, int col );
    
    EvaluationResult evaluateExpressionType( int line, int column, SimpleTypeConfiguration& conf, EvaluateExpressionOptions opt = DefaultCompletionOptions  );
    SimpleType unTypeDef( SimpleType scope , QMap<QString, QString>& typedefs );
    
	bool correctAccessOp( QStringList ptrList, MemberAccessOp accessOp );
    bool correctAccessOpAccurate( QStringList ptrList, MemberAccessOp accessOp );    

    QString buildSignature( SimpleType currType );
    SimpleType typeOf( QValueList<Tag>& tags, MemberAccessOp accessOp );
    
	/// @todo remove isInstance
	void computeCompletionEntryList( QValueList<CodeCompletionEntry>& entryList, SimpleContext* ctx, bool isInstance );
    void computeCompletionEntryList( SimpleType type, QValueList<CodeCompletionEntry>& entryList, const QStringList& type, bool isInstance );
    void computeCompletionEntryList( SimpleType type, QValueList<CodeCompletionEntry>& entryList, QValueList<Tag>& tags, bool isInstance );
    void computeCompletionEntryList( SimpleType type, QValueList<CodeCompletionEntry>& entryList, ClassDom klass, bool isInstance );
    void computeCompletionEntryList( SimpleType type, QValueList<CodeCompletionEntry>& entryList, NamespaceDom scope, bool isInstance );
    void computeCompletionEntryList( SimpleType type, QValueList<CodeCompletionEntry>& entryList, const FunctionList& methods, bool isInstance );
    void computeCompletionEntryList( SimpleType type, QValueList<CodeCompletionEntry>& entryList, const VariableList& attributes, bool isInstance );
    void computeCompletionEntryList( SimpleType type, QValueList<CodeCompletionEntry>& entryList, const ClassList& lst, bool isInstance );
    void computeCompletionEntryList( SimpleType type, QValueList<CodeCompletionEntry>& entryList, const NamespaceList& lst, bool isInstance );

	SimpleContext* computeContext( FunctionDefinitionAST* ast, int line, int col, int lineOffset, int colOffset );
	void computeContext( SimpleContext*& ctx, StatementAST* ast, int line, int col );
	void computeContext( SimpleContext*& ctx, StatementListAST* ast, int line, int col );
	void computeContext( SimpleContext*& ctx, IfStatementAST* ast, int line, int col );
	void computeContext( SimpleContext*& ctx, ForStatementAST* ast, int line, int col );
	void computeContext( SimpleContext*& ctx, DoStatementAST* ast, int line, int col );
	void computeContext( SimpleContext*& ctx, WhileStatementAST* ast, int line, int col );
	void computeContext( SimpleContext*& ctx, SwitchStatementAST* ast, int line, int col );
	void computeContext( SimpleContext*& ctx, TryBlockStatementAST* ast, int line, int col );
	void computeContext( SimpleContext*& ctx, CatchStatementListAST* ast, int line, int col );
	void computeContext( SimpleContext*& ctx, CatchStatementAST* ast, int line, int col );
	void computeContext( SimpleContext*& ctx, DeclarationStatementAST* ast, int line, int col );
	void computeContext( SimpleContext*& ctx, ConditionAST* ast, int line, int col );
	bool inContextScope( AST* ast, int line, int col, bool checkStart = true, bool checkEnd = true );

	QString getText( int startLine, int startColumn, int endLine, int endColumn, int omitLine = -1 );
	
    
private:
    friend class SimpleTypeCatalog;
    friend class SimpleTypeCodeModel;
    friend class SimpleTypeImpl;
    friend class ExpressionEvaluation;
    friend class PopupFillerHelpStruct;
    friend class PopupClassViewFillerHelpStruct;
    QGuardedPtr<CppSupportPart> m_pSupport;
	QTimer* m_ccTimer;
    QTimer* m_showStatusTextTimer;
    QValueList<QPair<int, QString> > m_statusTextList;
    
    void addStatusText( QString text, int timeout );
    void clearStatusText();
    
	QString m_activeFileName;
	KTextEditor::ViewCursorInterface* m_activeCursor;
	KTextEditor::EditInterface* m_activeEditor;
    KTextEditor::TextHintInterface* m_activeHintInterface;
	KTextEditor::CodeCompletionInterface* m_activeCompletion;

	bool m_bArgHintShow;
	bool m_bCompletionBoxShow;
	bool m_blockForKeyword;
	bool m_demandCompletion;

	unsigned int m_ccLine;
	unsigned int m_ccColumn;

	CodeInformationRepository* m_repository;
	CppCodeCompletionData* d;
	CompletionMode m_completionMode;

	QRegExp m_includeRx;
	QRegExp m_cppCodeCommentsRx;
	QRegExp m_codeCompleteChRx;
	QRegExp m_codeCompleteCh2Rx;
    QValueList<KTextEditor::CompletionEntry> m_fileEntryList;
    
    typedef QMap<int, DeclarationInfo> PopupActions;
    typedef QMap<int, ItemDom> PopupClassViewActions;
    PopupActions m_popupActions;
    PopupClassViewActions m_popupClassViewActions;
};

#endif 
// kate: indent-mode csands; tab-width 4;
