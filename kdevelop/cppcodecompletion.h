/***************************************************************************
                          cppcodecompletion.h  -  description
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

#ifndef __CPPCODECOMPLETION_H
#define __CPPCODECOMPLETION_H

#include <qobject.h>
#include <qvaluelist.h>
#include <qstringlist.h>
#include <qvbox.h>
#include <qlistbox.h>
#include "simpleparser.h"

class CEditWidget;
class KDevArgHint;
class CClassStore;
class CKDevelop;
class CParsedClass;
class CParsedContainer;
class CParsedMethod;
class CParsedAttribute;

class CompletionEntry {
public:
    QString type;
    QString text;
    QString prefix;
    QString postfix;
    QString comment;

    bool operator==( const CompletionEntry &c ) const {
        return ( c.type == type &&
                 c.text == text &&
                 c.postfix == postfix &&
                 c.prefix == prefix &&
                 c.comment == comment);
    }
};


class CppCodeCompletion : public QObject{
    Q_OBJECT
public:

    CppCodeCompletion( CEditWidget *edit, CClassStore*, CKDevelop* );

    virtual void showArgHint ( QStringList functionList, const QString& strWrapping, const QString& strDelimiter );
    virtual void showCompletionBox(QValueList<CompletionEntry> complList,int offset=0);
    bool eventFilter( QObject *o, QEvent *e );

    // QT 2.x wrapper methods
    //    please check if KDevelop will be compiled only for KDE 3
    bool checkEnd(const QString &str, const QString &suffix);
    void popFrontStringList(QStringList &slist);

public slots:
    void completeText();

signals:
    void completionAborted();
    void completionDone();
    void argHintHided();

protected:
    QString getMethodBody( int iLine, int iCol, QString* classname );
    QStringList getMethodListForClass( QString strClass, QString strMethod );
    void getParentMethodListForClass( CParsedClass* pClass, QString strMethod, QStringList& methodList );
    QStringList getFunctionList( QString strMethod );
    QString evaluateExpression( const QString& expr,
                                SimpleContext* ctx,
                                CClassStore* sigma );
    int expressionAt( const QString& text, int index );
    QValueList<CompletionEntry> getEntryListForExpr( const QString& expr,
                                                     SimpleContext* ctx );
    QValueList<CompletionEntry> getEntryListForClass ( QString strClass );
    QStringList splitExpression( const QString& text );

    /* methods which are called recursively by getEntryListForClass(...) */
    QList<CParsedMethod>* getParentMethodListForClass ( CParsedClass* pClass, QList<CParsedMethod>* pList );
    QList<CParsedAttribute>* getParentAttributeListForClass ( CParsedClass* pClass, QList<CParsedAttribute>* pList );

    QString getTypeOfMethod( CParsedContainer*, const QString& );
    QString getTypeOfAttribute( CParsedContainer*, const QString& );


private:
    void updateBox( bool newCoordinate=false );
    KDevArgHint* m_pArgHint;
    CEditWidget *m_edit;
    QVBox *m_completionPopup;
    QListBox *m_completionListBox;
    CClassStore* m_pStore;
    CKDevelop* m_pDevelop;
    QValueList<CompletionEntry> m_complList;
    int m_lineCursor;
    int m_colCursor;
    int m_offset;

public slots:
    void slotCursorPosChanged();
};




#endif
