/***************************************************************************
                          kdevcodecompletion.h  -  description
                             -------------------
    begin                : Sat Jul 21 2001
    copyright            : (C) 2001 by Victor R<F6>der, 2002 by Roberto Raggi
    email                : victor_roeder@gmx.de, raggi@cli.di.unipi.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef kdevcodecompletion_h
#define kdevcodecompletion_h

#include <qobject.h>
#include <ktexteditor/codecompletioninterface.h>

namespace Kate{
    class Document;
    class View;
}

class DocViewMan;
class ClassStore;
class CKDevelop;
class ParsedClass;
class ParsedContainer;
class ParsedMethod;
class ParsedAttribute;
class SimpleContext;

class KDevCodeCompletion: public QObject{
    Q_OBJECT
public:
    KDevCodeCompletion( DocViewMan* =0, const char* =0 );
    virtual ~KDevCodeCompletion();

    void expandText();
    void completeText();

public:
    static QValueList<KTextEditor::CompletionEntry> getAllWords( const QString&, const QString& );
    static QValueList<KTextEditor::CompletionEntry> unique( const QValueList<KTextEditor::CompletionEntry>& );
    static QString remove( QString text, const QChar& l, const QChar& r );
    static QString remove_keywords( QString text );
    static QString remove_comment( QString text );
    static QString purify( const QString& decl );

protected:
    QString getMethodBody( Kate::Document* doc, int iLine, int iCol, QString* classname );
    QStringList getMethodListForClass( QString strClass, QString strMethod );
    void getParentMethodListForClass( ParsedClass* pClass, QString strMethod, QStringList& methodList );
    QStringList getFunctionList( QString strMethod );
    QString evaluateExpression( const QString& expr,
                                SimpleContext* ctx,
                                ClassStore* sigma );
    int expressionAt( const QString& text, int index );
    QValueList<KTextEditor::CompletionEntry> getEntryListForExpr( const QString& expr,
                                                     SimpleContext* ctx );
    QValueList<KTextEditor::CompletionEntry> getEntryListForClass ( QString strClass );
    QStringList splitExpression( const QString& text );

    /* methods which are called recursively by getEntryListForClass(...) */
    QValueList<ParsedMethod*> getParentMethodListForClass ( ParsedClass* pClass, QValueList<ParsedMethod*>* pList );
    QValueList<ParsedAttribute*> getParentAttributeListForClass ( ParsedClass* pClass, QValueList<ParsedAttribute*>* pList );

    QString getTypeOfMethod( ParsedContainer*, const QString& );
    QString getTypeOfAttribute( ParsedContainer*, const QString& );
    bool checkEnd(const QString &str, const QString &suffix);
    void popFrontStringList(QStringList &slist);

protected:
    DocViewMan* m_pDockViewMan;
};

#endif
