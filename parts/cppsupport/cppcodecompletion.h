/***************************************************************************
                          cppcodecompletion.h  -  description
                             -------------------
    begin                : Sat Jul 21 2001
    copyright            : (C) 2001 by Victor Röder
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

#include <qobject.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qguardedptr.h>

#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/codecompletioninterface.h>

#include "cppsupportpart.h"
#include "simpleparser.h"
#include "ast.h"

class ParsedClass;
class ParsedClassContainer;
class ParsedScopeContainer;
class CodeInformationRepository;

class CppCodeCompletion : public QObject
{
    Q_OBJECT
public:
    CppCodeCompletion( CppSupportPart* part );
    virtual ~CppCodeCompletion();

    bool isEnabled() const { return m_bCodeCompletion; }
    void setEnabled( bool setEnable );
    
    virtual QString evaluateExpression( const QString&, SimpleContext* ctx );
    
    virtual int expressionAt( const QString& text, int index );
    virtual QStringList splitExpression( const QString& text );

    virtual QString getMethodBody( int line, int column, QString* classname );
    
    QValueList<KTextEditor::CompletionEntry> findAllEntries( const QString& type, bool includePrivate, bool isInstance );
    
    QStringList getGlobalSignatureList( const QString& functionName );
    QStringList getSignatureListForClass( const QString& className, const QString& functionName, bool isInstance );
    
    QString typeOf( const QString& name, ParsedClassContainer* container = 0 );
    ParsedClassContainer* findContainer( const QString& name, ParsedScopeContainer* container = 0, 
					 const QStringList& imports = QStringList() );
        
public slots:
    void completeText();

private slots:
    void slotActivePartChanged(KParts::Part *part);
    void slotArgHintHided();
    void slotCompletionBoxHided( KTextEditor::CompletionEntry entry );
    void slotTextChanged();
    void slotFileParsed( const QString& fileName );
    void slotTimeout();

private:
    void setupCodeInformationRepository();
    QString typeName( const QString& s );
    FunctionDefinitionAST* functionDefinition( AST* node );
    
    SimpleContext* computeContext( FunctionDefinitionAST* ast, int line, int col );
    void computeContext( SimpleContext*& ctx, StatementAST* ast, int line, int col );
    void computeContext( SimpleContext*& ctx, StatementListAST* ast, int line, int col );
    void computeContext( SimpleContext*& ctx, IfStatementAST* ast, int line, int col );
    void computeContext( SimpleContext*& ctx, ForStatementAST* ast, int line, int col );
    void computeContext( SimpleContext*& ctx, DoStatementAST* ast, int line, int col );
    void computeContext( SimpleContext*& ctx, WhileStatementAST* ast, int line, int col );
    void computeContext( SimpleContext*& ctx, SwitchStatementAST* ast, int line, int col );
    void computeContext( SimpleContext*& ctx, DeclarationStatementAST* ast, int line, int col );
    
    QString getText( int startLine, int startColumn, int endLine, int endColumn );

private:
    QGuardedPtr<CppSupportPart> m_pSupport;
    ClassStore* m_pStore;
    QTimer* m_ccTimer;
    QString m_activeFileName;
    KTextEditor::ViewCursorInterface* m_activeCursor;
    KTextEditor::EditInterface* m_activeEditor;
    KTextEditor::CodeCompletionInterface* m_activeCompletion;

    bool m_bArgHintShow;
    bool m_bCompletionBoxShow;
    bool m_bCodeCompletion;

    private: unsigned int m_ccLine;
    private: unsigned int m_ccColumn;

    CodeInformationRepository* m_repository;
};

#endif
