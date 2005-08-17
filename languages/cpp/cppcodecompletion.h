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

#include <qobject.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qpointer.h>
#include <qregexp.h>
//Added by qt3to4:
#include <Q3ValueList>

class CodeInformationRepository;
class SimpleContext;
class CppCodeCompletionData;

class CppCodeCompletion : public QObject
{
	Q_OBJECT
public:
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

	int expressionAt( const QString& text, int index );
	QStringList splitExpression( const QString& text );
	QStringList typeOf( const QString& name, const QStringList& scope, MemberAccessOp accessOp );
	QStringList evaluateExpression( QString expr, SimpleContext* ctx );

	static QStringList typeName( const QString& name );

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
	void computeFileEntryList();

private:
	void integratePart( KParts::Part* part );
	void setupCodeInformationRepository();
	FunctionDefinitionAST* functionDefinition( AST* node );
	void computeRecoveryPoints();

	QStringList evaluateExpressionInternal( QStringList& exprList, const QStringList& scope, SimpleContext* ctx = 0 );
	bool correctAccessOp( QStringList ptrList, MemberAccessOp accessOp );

	QStringList typeOf( const Q3ValueList<Tag>& tags, MemberAccessOp accessOp );
	QStringList typeOf( const QString& name, ClassDom klass, MemberAccessOp accessOp );
	QStringList typeOf( const QString& name, NamespaceDom scope, MemberAccessOp accessOp );
	QStringList typeOf( const QString& name, const FunctionList& methods, MemberAccessOp accessOp );

	/// @todo remove isInstance
	void computeCompletionEntryList( Q3ValueList<KTextEditor::CompletionEntry>& entryList, SimpleContext* ctx, bool isInstance );
	void computeCompletionEntryList( Q3ValueList<KTextEditor::CompletionEntry>& entryList, const QStringList& type, bool isInstance );
	void computeCompletionEntryList( Q3ValueList<KTextEditor::CompletionEntry>& entryList, Q3ValueList<Tag>& tags, bool isInstance );
	void computeCompletionEntryList( Q3ValueList<KTextEditor::CompletionEntry>& entryList, ClassDom klass, bool isInstance );
	void computeCompletionEntryList( Q3ValueList<KTextEditor::CompletionEntry>& entryList, NamespaceDom scope, bool isInstance );
	void computeCompletionEntryList( Q3ValueList<KTextEditor::CompletionEntry>& entryList, const FunctionList& methods, bool isInstance );
	void computeCompletionEntryList( Q3ValueList<KTextEditor::CompletionEntry>& entryList, const VariableList& attributes, bool isInstance );
	void computeCompletionEntryList( Q3ValueList<KTextEditor::CompletionEntry>& entryList, const ClassList& lst, bool isInstance );
	void computeCompletionEntryList( Q3ValueList<KTextEditor::CompletionEntry>& entryList, const NamespaceList& lst, bool isInstance );

	void computeSignatureList( QStringList& signatureList, const QString& name, const QStringList& type );
	void computeSignatureList( QStringList& signatureList, const QString& name, ClassDom klass );
	void computeSignatureList( QStringList& signatureList, const QString& name, const FunctionList& methods );
	void computeSignatureList( QStringList& signatureList, const QString& name, Q3ValueList<Tag>& tags );

	SimpleContext* computeContext( FunctionDefinitionAST* ast, int line, int col );
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

	ClassDom findContainer( const QString& name, NamespaceDom container = 0, bool includeImports = false );
	QString findClass( const QString& className );

private:
	QPointer<CppSupportPart> m_pSupport;
	QTimer* m_ccTimer;
	QString m_activeFileName;
	KTextEditor::ViewCursorInterface* m_activeCursor;
	KTextEditor::EditInterface* m_activeEditor;
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
	Q3ValueList<KTextEditor::CompletionEntry> m_fileEntryList;
};

#endif 
// kate: indent-mode csands; tab-width 4;
