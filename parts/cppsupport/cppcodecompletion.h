/***************************************************************************
                          cppcodecompletion.h  -  description
                             -------------------
    begin                : Sat Jul 21 2001
    copyright            : (C) 2001 by Victor Röder
    email                : victor_roeder@gmx.de
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

#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/codecompletioninterface.h>

#include "parsedclass.h"
/*#include "parsedmethod.h"*/

#include "kdevcore.h"

#include "cppccparser.h"
#include "cppsupportpart.h"
#include "simpleparser.h"

class KTempFile;

class CppCodeCompletion : public QObject
{
    Q_OBJECT

public:
    CppCodeCompletion ( CppSupportPart* part, ClassStore* pStore, ClassStore* pCCStore );
    void setEnableCodeCompletion( bool setEnable );
    virtual ~CppCodeCompletion();

public slots:
    void completeText();
    void typeOf();
    QString typingTypeOf( int, int );

protected slots:
    void slotActivePartChanged(KParts::Part *part);
    void slotArgHintHided();
    void slotCompletionBoxHided( KTextEditor::CompletionEntry entry );
    //void slotTextChanged( KEditor::Document *pDoc, int nLine, int nCol );
    void slotTextChanged( int nLine, int nCol, const QString& text );
    void slotTextChangedRoberto(int nLine, int nCol, const QString &text );
    void slotCursorPositionChanged();

protected:
    QString evaluateExpression( const QString& expr,
                                const QValueList<SimpleVariable>& roo,
                                ClassStore* sigma );
    int expressionAt( const QString& text, int index );
    QStringList splitExpression( const QString& text );

    QString getMethodBody( int iLine, int iCol, QString* classname );
    QString getCompletionText ( int nLine, int nCol );
    QString getNodeText ( int nNode, int nLine );
    QString getNodeDelimiter ( int nNode, int nLine );
    int getNodePos ( int nLine, int nCol );

    QValueList<KTextEditor::CompletionEntry> getEntryListForExpr( const QString& expr, const QValueList<SimpleVariable>& vars );


    QString createTmpFileForParser (int iLine);

    QValueList<KTextEditor::CompletionEntry> getEntryListForClass( QString strClass );
    QValueList<KTextEditor::CompletionEntry> getEntryListForNamespace( const QString& strNamespace );
    QValueList<KTextEditor::CompletionEntry> getEntryListForStruct( const QString& strStruct );
    QValueList<KTextEditor::CompletionEntry> getEntryListForClassOfNamespace( QString strClass, const QString& strNamespace );

    /* methods which are called recursively by getEntryListForClass(...) */
    /* These are utility functions which add a layer above the ClassStore API */
    QString getTypeOfMethod( ParsedClass*, const QString& );
    QString getTypeOfAttribute( ParsedClass*, const QString& );

    QValueList<ParsedMethod*> getMethodListForClassAndAncestors( ParsedClass* pClass );
    QValueList<ParsedAttribute*> getAttributeListForClassAndAncestors( ParsedClass* pClass );

    QStringList getGlobalSignatureList(const QString &functionName);
    QStringList getSignatureListForClass( QString strClass, QString strMethod );
    QStringList getParentSignatureListForClass( ParsedClass* pClass, QString strMethod );

private:
    CppSupportPart* m_pSupport;
    KDevCore* m_pCore;
    ClassStore* m_pStore;
    ClassStore* m_pCCStore;
    CppCCParser* m_pParser;
    KTextEditor::ViewCursorInterface* m_pCursorIface;
    KTextEditor::EditInterface* m_pEditIface;
    KTextEditor::CodeCompletionInterface* m_pCompletionIface;

    KTempFile* m_pTmpFile;
    QString m_currentClassName;
    bool m_bArgHintShow;
    bool m_bCompletionBoxShow;
    bool m_bCodeCompletion;

    QStringList m_CHCommentList;

};

#endif
