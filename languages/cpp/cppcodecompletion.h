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
#include <qguardedptr.h>
#include <qregexp.h>

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

public:
    CppCodeCompletion( CppSupportPart* part );
    virtual ~CppCodeCompletion();

    CodeInformationRepository* repository() { return m_repository; }
    CompletionMode completionMode() const { return m_completionMode; }

    int expressionAt( const QString& text, int index );
    QStringList splitExpression( const QString& text );
    QStringList typeOf( const QString& name, const QStringList& scope );
    QStringList evaluateExpression( QString expr, SimpleContext* ctx );

    static QStringList typeName( const QString& name );

public slots:
    void completeText();

private slots:
    void slotPartAdded(KParts::Part *part);
    void slotActivePartChanged(KParts::Part *part);
    void slotArgHintHided();
    void slotCompletionBoxHided( KTextEditor::CompletionEntry entry );
    void slotTextChanged();
    void slotFileParsed( const QString& fileName );
    void slotTimeout();
    void computeFileEntryList();

private:
    void integratePart( KParts::Part* part );
    void setupCodeInformationRepository();
    FunctionDefinitionAST* functionDefinition( AST* node );
    void computeRecoveryPoints();

    QStringList evaluateExpressionInternal( QStringList& exprList, const QStringList& scope, SimpleContext* ctx=0 );

    QStringList typeOf( const QValueList<Tag>& tags );
    QStringList typeOf( const QString& name, ClassDom klass );
    QStringList typeOf( const QString& name, NamespaceDom scope );
    QStringList typeOf( const QString& name, const FunctionList& methods );

    /// @todo remove isInstance
    void computeCompletionEntryList( QValueList<KTextEditor::CompletionEntry>& entryList, SimpleContext* ctx, bool isInstance );
    void computeCompletionEntryList( QValueList<KTextEditor::CompletionEntry>& entryList, const QStringList& type, bool isInstance );
    void computeCompletionEntryList( QValueList<KTextEditor::CompletionEntry>& entryList, QValueList<Tag>& tags, bool isInstance );
    void computeCompletionEntryList( QValueList<KTextEditor::CompletionEntry>& entryList, ClassDom klass, bool isInstance );
    void computeCompletionEntryList( QValueList<KTextEditor::CompletionEntry>& entryList, NamespaceDom scope, bool isInstance );
    void computeCompletionEntryList( QValueList<KTextEditor::CompletionEntry>& entryList, const FunctionList& methods, bool isInstance );
    void computeCompletionEntryList( QValueList<KTextEditor::CompletionEntry>& entryList, const VariableList& attributes, bool isInstance );
    void computeCompletionEntryList( QValueList<KTextEditor::CompletionEntry>& entryList, const ClassList& lst, bool isInstance );
    void computeCompletionEntryList( QValueList<KTextEditor::CompletionEntry>& entryList, const NamespaceList& lst, bool isInstance );

    void computeSignatureList( QStringList& signatureList, const QString& name, const QStringList& type );
    void computeSignatureList( QStringList& signatureList, const QString& name, ClassDom klass );
    void computeSignatureList( QStringList& signatureList, const QString& name, const FunctionList& methods );
    void computeSignatureList( QStringList& signatureList, const QString& name, QValueList<Tag>& tags );

    SimpleContext* computeContext( FunctionDefinitionAST* ast, int line, int col );
    void computeContext( SimpleContext*& ctx, StatementAST* ast, int line, int col );
    void computeContext( SimpleContext*& ctx, StatementListAST* ast, int line, int col );
    void computeContext( SimpleContext*& ctx, IfStatementAST* ast, int line, int col );
    void computeContext( SimpleContext*& ctx, ForStatementAST* ast, int line, int col );
    void computeContext( SimpleContext*& ctx, DoStatementAST* ast, int line, int col );
    void computeContext( SimpleContext*& ctx, WhileStatementAST* ast, int line, int col );
    void computeContext( SimpleContext*& ctx, SwitchStatementAST* ast, int line, int col );
    void computeContext( SimpleContext*& ctx, DeclarationStatementAST* ast, int line, int col );
    void computeContext( SimpleContext*& ctx, ConditionAST* ast, int line, int col );

    QString getText( int startLine, int startColumn, int endLine, int endColumn, int omitLine = -1 );

    ClassDom findContainer( const QString& name, NamespaceDom container=0, bool includeImports=false );
    QString findClass( const QString& className );

private:
    QGuardedPtr<CppSupportPart> m_pSupport;
    QTimer* m_ccTimer;
    QString m_activeFileName;
    KTextEditor::ViewCursorInterface* m_activeCursor;
    KTextEditor::EditInterface* m_activeEditor;
    KTextEditor::CodeCompletionInterface* m_activeCompletion;

    bool m_bArgHintShow;
    bool m_bCompletionBoxShow;
	bool m_blockForKeyword;

    unsigned int m_ccLine;
    unsigned int m_ccColumn;

    CodeInformationRepository* m_repository;
    CppCodeCompletionData* d;
    CompletionMode m_completionMode;

    QRegExp m_includeRx;
    QValueList<KTextEditor::CompletionEntry> m_fileEntryList;
};

#endif
