/***************************************************************************
                          kdevcodecompletion.cpp  -  description
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

#include "kdevcodecompletion.h"
#include "kdevcodecompletion.moc"
#include "simpleparser.h"
#include "ckdevelop.h"
#include "kdevregexp.h"
#include "docviewman.h"
#include "classstore.h"
#include "api.h"

#include <kate/document.h>
#include <kate/view.h>
#include <kdebug.h>
#include <kregexp.h>

#include <qsizegrip.h>
#include <qapplication.h>
#include <qregexp.h>
#include <qmap.h>

QValueList<KTextEditor::CompletionEntry>
KDevCodeCompletion::unique( const QValueList<KTextEditor::CompletionEntry>& entryList )
{
    QValueList<KTextEditor::CompletionEntry> l;
    QMap<QString, bool> map;
    QValueList<KTextEditor::CompletionEntry>::ConstIterator it=entryList.begin();
    while( it != entryList.end() ){
        KTextEditor::CompletionEntry e = *it++;
        QString key = e.type + " " +
                      e.text + " " +
                      e.prefix + " " +
                      e.postfix + " ";
        if( map.find(key) == map.end() ){
            map[ key ] = TRUE;
            l << e;
        }
    }
    return l;
}

QString KDevCodeCompletion::remove( QString text, const QChar& l, const QChar& r )
{
    QString s;

    unsigned int index = 0;
    int count = 0;
    while( index < text.length() ){
        if( text[index] == l ){
            ++count;
        } else if( text[index] == r ){
            --count;
        } else if( count == 0 ){
            s += text[ index ];
        }
        ++index;
    }
    return s;
}

QString KDevCodeCompletion::remove_keywords( QString text ){
    QRegExp ide_rx( "[_a-zA-Z0-9][_a-zA-Z0-9]*" );
    QStringList keywords = QStringList::split( "|",
            "unsigned|signed|case|delete|return|if|then|else|for|while|do|"
            "const|static|volatile|extern|struct" );

    QString s;
    int index = 0;
    while( index < (int)text.length() ){
        int len = 0;
        int pos = ide_rx.match( text, index, &len );
        if( pos == -1 ){
            break;
        }
        s += text.mid( index, pos - index );
        QString ide = text.mid( pos, len );
        if( keywords.findIndex(ide) == -1 ){
            s += ide;
        }
        index = pos + ide.length();
    }
    s += text.mid( index );
    return s;
}

QString KDevCodeCompletion::remove_comment( QString text ){
    QString s;
    unsigned int index = 0;
    bool skip = FALSE;
    while( index < text.length() ){
        if( text.mid(index, 2) == "/*" ){
            skip = TRUE;
            index += 2;
            continue;
        } else if( text.mid(index, 2) == "*/" ){
            skip = FALSE;
            index += 2;
            continue;
        } else if( !skip ){
            s += text[ index ];
        }
        ++index;
    }
    return s;
}

QString KDevCodeCompletion::purify( const QString& decl )
{
    QString s = decl;

    QRegExp rx1( "\\*" );
    QRegExp rx2( "&" );
    s = s.replace( rx1, "" ).replace( rx2, "" );
    s = remove_keywords( s );
    s = remove( s, '[', ']' );
    s = s.simplifyWhiteSpace();

    return s;
}


KDevCodeCompletion::KDevCodeCompletion( DocViewMan* parent, const char* name )
    : QObject( parent, name ),
      m_pDockViewMan( parent )
{

}

KDevCodeCompletion::~KDevCodeCompletion()
{

}

QValueList<KTextEditor::CompletionEntry>
KDevCodeCompletion::getAllWords( const QString& text, const QString& prefix )
{
    QMap<QString, bool> map;
    QValueList<KTextEditor::CompletionEntry> entries;
    QRegExp rx( QString("\\b") + prefix + "[a-zA-Z0-9_]+\\b" );

    int idx = 0;
    int pos = 0;
    int len = 0;
    while( (pos = rx.match(text, idx, &len)) != -1 ){
	QString word = text.mid( pos, len );
        if( map.find(word) == map.end() ){
            KTextEditor::CompletionEntry e;
            e.text = word;
            entries << e;
            map[ word ] = TRUE;
        }
        idx = pos + len + 1;
    }
    return entries;
}

void KDevCodeCompletion::expandText()
{
    Kate::View* pView = m_pDockViewMan->currentEditView();
    if( !pView ){
        return;
    }

    QString prefix = pView->currentWord();
    if( prefix.length() != 0 ){
        QValueList<KTextEditor::CompletionEntry> entries;
        entries = getAllWords( pView->getDoc()->text(), prefix );

        if( entries.count() == 1 ){
            pView->insertText( entries[ 0 ].text.mid(prefix.length()) );
        } else if( entries.size() ){
            pView->showCompletionBox( entries, prefix.length() );
        }
    }
}

void KDevCodeCompletion::completeText()
{
    Kate::View* pView = m_pDockViewMan->currentEditView();
    if( !pView ){
        return;
    }

    unsigned int nLine, nCol;
    pView->cursorPositionReal( &nLine, &nCol );
    Kate::Document* doc = dynamic_cast<Kate::Document*>( pView->document() );
    // kdDebug() << "nLine = " << nLine << endl;
    // kdDebug() << "nCol = " << nCol << endl;

    QString strCurLine = doc->textLine( nLine );
    QString className;
    QString contents;
    bool showArguments = FALSE;

    if( strCurLine[ nCol-1 ] == '(' ){
        --nCol;
        showArguments = TRUE;
    }

    contents = getMethodBody( doc, nLine, nCol, &className );
    kdDebug() << "contents = " << contents << endl;
    kdDebug() << "classname = " << className << endl;

    SimpleContext* ctx = SimpleParser::localVariables( contents );
    if( !ctx )
        return;

    QValueList<SimpleVariable> variableList;
    SimpleVariable v;
    v.name = "this";
    v.type = className;
    variableList.append( v );
    ctx->add( v );

    QString word;
    int start_expr = expressionAt( contents, contents.length() - 1 );
    kdDebug() << "start_expr = " << start_expr << endl;
    QString expr;
    if( start_expr != int(contents.length()) - 1 ){
        expr = contents.mid( start_expr, contents.length() - start_expr );
        expr = expr.stripWhiteSpace();
    }

    int idx = expr.length() - 1;
    while( expr[idx].isLetterOrNumber() || expr[idx] == '_' ){
        --idx;
    }
    if( idx != int(expr.length()) - 1 ){
        ++idx;
        word = expr.mid( idx ).stripWhiteSpace();
        expr = expr.left( idx ).stripWhiteSpace();
    }

    kdDebug() << "prefix = |" << word << "|" << endl;
    kdDebug() << "expr = |" << expr << "|" << endl;

    if( expr == "." || expr == "->" ){
        kdDebug() << "---------> no expression ;-)" << endl;
    } else {
        if( showArguments ){
            QString type = evaluateExpression( expr, ctx, API::getInstance()->classStore() );
            QStringList functionList;

            functionList = getMethodListForClass( type, word );

            if( functionList.count() == 0 ){
                functionList = getFunctionList( word );
            }

            if( functionList.count() ){
                pView->showArgHint( functionList, "()", "," );
            }
        } else {
            QValueList<KTextEditor::CompletionEntry> entries;
            QString type;

            SimpleVariable v = ctx->findVariable( word );
            if( expr.isEmpty() && !v.type.isEmpty() ){
                type = v.type;
            } else {
                type = evaluateExpression( expr, ctx, API::getInstance()->classStore() );
            }

            entries = unique( getEntryListForClass( type ) );
            if( entries.count() ){
                pView->showCompletionBox( entries, word.length() );
            }
        }
    }

    delete( ctx );
    ctx = 0;
}

enum { T_ACCESS, T_PAREN, T_BRACKET, T_IDE, T_UNKNOWN };

int KDevCodeCompletion::expressionAt( const QString& text, int index )
{
    kdDebug() << "KDevCodeCompletion::expressionAt()" << endl;

    int last = T_UNKNOWN;
    int start = index;
    for( ;; ){
        while( index >= 0 && text[index].isSpace() ){
            --index;
        }

        if( index < 0 ){
            break;
        }

        QChar ch = text[ index ];
        QString ch2 = text.mid( index-1, 2 );
        if( (last != T_IDE) && (ch.isLetterOrNumber() || ch == '_') ){
            while( index > 0 ){
                if( text[index].isLetterOrNumber() || text[index] == '_' ){
                    --index;
                } else if( text.mid( index-1, 2 ) == "::" ){
                    index -= 2;
                } else {
                    break;
                }
            }
            last = T_IDE;
        } else if( last != T_IDE && ch == ')' ){
            int count = 0;
            while( index > 0 ){
                QChar ch = text[ index ];
                if( ch == '(' ){
                    ++count;
                } else if( ch == ')' ){
                    --count;
                } else if( count == 0 ){
                    --index;
                    last = T_PAREN;
                    break;
                }
                --index;
            }
        } else if( ch == ']' ){
            int count = 0;
            while( index > 0 ){
                QChar ch = text[ index ];
                if( ch == '[' ){
                    ++count;
                } else if( ch == ']' ){
                    --count;
                } else if( count == 0 ){
                    --index;
                    last = T_BRACKET;
                    break;
                }
                --index;
            }
        } else if( ch == '.' ){
            --index;
            last = T_ACCESS;
        } else if( ch2 == "->" ){
            index -= 2;
            last = T_ACCESS;
        } else {
            if( start > index ){
                kdDebug() << "-----------> incr index" << endl;
                ++index;
            }
            last = T_UNKNOWN;
            break;
        }
    }
    return index;
}

QStringList KDevCodeCompletion::splitExpression( const QString& text )
{
#define ADD_CURRENT()\
 if( current.length() ) { l << current; /*kdDebug() << "add word " << current << endl;*/ current = ""; }

    QStringList l;
    unsigned int index = 0;
    QString current;
    while( index < text.length() ){
        QChar ch = text[ index ];
        QString ch2 = text.mid( index, 2 );

        if( ch == '.' ){
            ADD_CURRENT();
            ++index;
        } else if( ch == '(' ){
            int count = 0;
            while( index < text.length() ){
                QChar ch = text[ index ];
                if( ch == '(' ){
                    ++count;
                } else if( ch == ')' ){
                    --count;
                } else if( count == 0 ){
                    break;
                }
                current += ch;
                ++index;
            }
        } else if( ch == '[' ){
            int count = 0;
            while( index < text.length() ){
                QChar ch = text[ index ];
                if( ch == '[' ){
                    ++count;
                } else if( ch == ']' ){
                    --count;
                } else if( count == 0 ){
                    break;
                }
                current += ch;
                ++index;
            }
        } else if( ch2 == "->" ){
            ADD_CURRENT();
            index += 2;
        } else {
            current += text[ index ];
            ++index;
        }
    }
    ADD_CURRENT();
    return l;
}

QString KDevCodeCompletion::evaluateExpression( const QString& expr,
                                               SimpleContext* ctx,
                                               ClassStore* sigma )
{
    kdDebug() << "KDevCodeCompletion::evaluateExpression()" << endl;
    kdDebug() << "ctx = " << ctx << endl;
    kdDebug() << "sigma = " << sigma << endl;
    QStringList exprs = splitExpression( expr );
    for( QStringList::Iterator it=exprs.begin(); it!=exprs.end(); ++it ){
        kdDebug() << "expr " << (*it) << endl;
    }

    SimpleVariable v_this = ctx->findVariable( "this" );
    ParsedClass* pThis = sigma->getClassByName( v_this.type );
    QString type;

    if( exprs.count() == 0 ){
        return v_this.type;
    }

    QString e1 = exprs.first().stripWhiteSpace();
    popFrontStringList(exprs);

    kdDebug() << "---> e1 = " << e1 << endl;

    if( e1.isEmpty() ){
        type = v_this.type;
    } else if( checkEnd(e1, "::") ){
        type = e1.left( e1.length() - 2 ).stripWhiteSpace();
    } else {
        int first_paren_index = 0;
        if( (first_paren_index = e1.find('(')) != -1 ){
            if( first_paren_index == 0 ){
                if( e1[e1.length()-1] == ')' ){
                    // e1 is a subexpression
                    QString subexpr = e1.mid( 1, e1.length() - 2 );
                    subexpr = subexpr.stripWhiteSpace();
                    if( subexpr[0] != '(' ){
                        int start_expr = expressionAt( subexpr, subexpr.length()-1 );
                        if( start_expr != (int)subexpr.length()-1 ){
                            subexpr = subexpr.mid( start_expr );
                            kdDebug() << "subexpr = " << subexpr << endl;
                        }
                    }
                    type = evaluateExpression( subexpr, ctx, sigma );
                } else {
                    // e1 is cast
                    //kdDebug() << "maybe a cast = " << e1 << endl;
                    KDevRegExp cast_rx( "^\\([ \t]*([a-zA-Z_][a-zA-Z0-9_]*)[^)]*)" );
                    if( cast_rx.search(e1) == 0 ){
                        type = cast_rx.cap( 1 );
                        kdDebug() << "cast type = " << type << endl;
                    } else {
                        type = QString::null;
                    }
                }
            } else {
                e1 = e1.left( first_paren_index ).stripWhiteSpace();
                if( pThis ){
                    type = getTypeOfMethod( pThis, e1 );
                }
                if( type.isEmpty() ){
                    type = getTypeOfMethod( &API::getInstance()->classStore()->globalContainer,
                                            e1 );
                }
            }
        } else {
            SimpleVariable v = ctx->findVariable( e1 );
            if( v.type ){
                // e1 is a local variable
                type = v.type;
            } else {
                // e1 is an attribute
                if( pThis ){
                    type = getTypeOfAttribute( pThis, e1 );
                }
                if( type.isEmpty() ){
                    type = getTypeOfAttribute( &API::getInstance()->classStore()->globalContainer,
                                               e1 );
                }
            }
        }
    }

    type = purify( type );
    ParsedContainer* pContainer = sigma->getClassByName( type );
    if( !pContainer ){
        pContainer = sigma->globalContainer.getStructByName( type );
        kdDebug() << "is a struct??" << endl;
    }
    kdDebug() << "pContainer = " << pContainer << endl;
    while( pContainer && exprs.count() ){

        QString e = exprs.first().stripWhiteSpace();
        popFrontStringList(exprs);
        type = "";  // no type

        kdDebug() << "----------> evaluate " << e << endl;

        int first_paren_index;
        if( e.isEmpty() ){
            break;
        } else if( (first_paren_index = e.find('(')) != -1 ){
            e = e.left( first_paren_index );
            type = getTypeOfMethod( pContainer, e );
            pContainer = sigma->getClassByName( type );
            if( !pContainer ){
                pContainer = sigma->globalContainer.getStructByName( type );
            }
        } else {
            type = getTypeOfAttribute( pContainer, e );
            pContainer = sigma->getClassByName( type );
            if( !pContainer ){
                pContainer = sigma->globalContainer.getStructByName( type );
            }
        }
    }

    kdDebug() << "-------------> last type = " << type << endl;

    return type;
}

QString KDevCodeCompletion::getTypeOfMethod( ParsedContainer* pContainer, const QString& name )
{
    kdDebug() << "KDevCodeCompletion::getTypeOfMethod()"
              << " - " << pContainer
              << " - " << name << endl;
    if( !pContainer || !API::getInstance()->classStore() ){
        return QString::null;
    }

    QValueList<ParsedMethod*> pMethodList = pContainer->getMethodByName( name );
    if( pMethodList.count() != 0 ){
        // TODO: check for method's arguments
        QString type = (*(pMethodList.at( 0 )))->type();
        return purify( type );
    }

    ParsedClass* pClass = dynamic_cast<ParsedClass*>( pContainer );
    if( pClass ){
        QList<ParsedParent> parentList = pClass->parents;
        for( ParsedParent* pParent=parentList.first(); pParent!=0; pParent=parentList.next() ){
            ParsedClass* pClass = API::getInstance()->classStore()->getClassByName( pParent->name() );
            QString type = getTypeOfMethod( pClass, name );
            type = purify( type );
            if( !type.isEmpty() ){
                return type;
            }
        }
    }
    return QString::null;
}

QString KDevCodeCompletion::getTypeOfAttribute( ParsedContainer* pContainer, const QString& name )
{
    if( !pContainer || !API::getInstance()->classStore() ){
        return QString::null;
    }

    ParsedAttribute* pAttr = pContainer->getAttributeByName( name );
    if( pAttr ){
        QString type = pAttr->type();
        return purify( type );
    }

    ParsedClass* pClass = dynamic_cast<ParsedClass*>( pContainer );
    if( pClass ){
        QList<ParsedParent> parentList = pClass->parents;
        for( ParsedParent* pParent=parentList.first(); pParent!=0; pParent=parentList.next() ){
            ParsedClass* pClass = API::getInstance()->classStore()->getClassByName( pParent->name() );
            QString type = getTypeOfAttribute( pClass, name );
            type = purify( type );
            if( !type.isEmpty() ){
                return type;
            }
        }
    }
    return QString::null;
}

QValueList<KTextEditor::CompletionEntry>
KDevCodeCompletion::getEntryListForExpr( const QString& expr,
                                        SimpleContext* ctx )
{
    QString type = evaluateExpression( expr, ctx, API::getInstance()->classStore() );
    kdDebug() << "--------> type = " << type << endl;
    QValueList<KTextEditor::CompletionEntry> entries = getEntryListForClass( type );
    return entries;
}

QValueList<KTextEditor::CompletionEntry> KDevCodeCompletion::getEntryListForClass ( QString strClass )
{
    kdDebug() << "KDevCodeCompletion::getEntryListForClass()" << endl;
    QValueList<KTextEditor::CompletionEntry> entryList;

    ParsedContainer* pContainer = API::getInstance()->classStore()->getClassByName( strClass );
    if( !pContainer ){
        pContainer = API::getInstance()->classStore()->globalContainer.getStructByName( strClass );
    }

    if ( pContainer )
    {
        QValueList<ParsedMethod*> pMethodList;
        QValueList<ParsedAttribute*> pAttributeList;


        // Load the methods, slots, signals of the current class and its parents into the list
        pMethodList = pContainer->getSortedMethodList();

        // Load the attributes of the current class and its parents into the list
        pAttributeList = pContainer->getSortedAttributeList();

        ParsedClass* pClass = dynamic_cast<ParsedClass*>( pContainer );
        if( pClass ){

            ParsedMethod *pMethod;
            
            QValueList<ParsedMethod*> pTmpList = pClass->getSortedSlotList();
            QValueList<ParsedMethod*>::ConstIterator pTmpListIt;
            for (pTmpListIt = pTmpList.begin(); pTmpListIt != pTmpList.end(); ++pTmpListIt )
            {
                pMethod = *pTmpListIt;
                pMethodList.append ( pMethod );
            }

            pTmpList = pClass->getSortedSignalList();
            for (pTmpListIt = pTmpList.begin(); pTmpListIt != pTmpList.end(); ++pTmpListIt )
            {
                pMethod = *pTmpListIt;
                pMethodList.append ( pMethod );
            }

            pMethodList = getParentMethodListForClass ( pClass, &pMethodList );
            pAttributeList = getParentAttributeListForClass ( pClass, &pAttributeList );
        }

        ParsedMethod *pMethod;
        
        QValueList<ParsedMethod*>::ConstIterator pMethodListIt;
        for (pMethodListIt = pMethodList.begin(); pMethodListIt != pMethodList.end(); ++pMethodListIt )
        {
            pMethod = *pMethodListIt;
            KTextEditor::CompletionEntry entry;
            entry.text = pMethod->name();
            entry.postfix = "()";
            entryList << entry;
        }

        ParsedAttribute *pAttribute;
        
        QValueList<ParsedAttribute*>::ConstIterator pAttributeListIt;
        for (pAttributeListIt = pAttributeList.begin(); pAttributeListIt != pAttributeList.end(); ++pAttributeListIt )
        {
            pAttribute = *pAttributeListIt;
            KTextEditor::CompletionEntry entry;
            entry.text = pAttribute->name();
            entry.postfix = "";
            entryList << entry;
        }
    }

    kdDebug() << "getEntryListForClass() -- END" << endl;
    return entryList;
}

QValueList<ParsedMethod*> KDevCodeCompletion::getParentMethodListForClass ( ParsedClass* pClass, QValueList<ParsedMethod*>* pList )
{
    QList<ParsedParent> parentList = pClass->parents;

    for ( ParsedParent* pParentClass = parentList.first(); pParentClass != 0; pParentClass = parentList.next() )
    {
        pClass = API::getInstance()->classStore()->getClassByName ( pParentClass->name() );

        if ( pClass )
        {
            QValueList<ParsedMethod*> pTmpList = pClass->getSortedMethodList();
            QValueList<ParsedMethod*>::ConstIterator pTmpListIt;
            ParsedMethod *pMethod;

            for (pTmpListIt = pTmpList.begin(); pTmpListIt != pTmpList.end(); ++pTmpListIt )
            {
                pMethod = *pTmpListIt;
                pList->append ( pMethod );
            }

            pTmpList = pClass->getSortedSlotList();
            for (pTmpListIt = pTmpList.begin(); pTmpListIt != pTmpList.end(); ++pTmpListIt )
            {
                pList->append ( pMethod );
            }

            pTmpList = pClass->getSortedSignalList();
            for (pTmpListIt = pTmpList.begin(); pTmpListIt != pTmpList.end(); ++pTmpListIt )
            {
                pList->append ( pMethod );
            }

            *pList = getParentMethodListForClass ( pClass, pList );
        }
        /*else
          {
          // TODO: look in ClassStore for Namespace classes
          } */
    }

    return *pList;
}

QValueList<ParsedAttribute*> KDevCodeCompletion::getParentAttributeListForClass ( ParsedClass* pClass, QValueList<ParsedAttribute*>* pList )
{
    QList<ParsedParent> parentList = pClass->parents;

    for ( ParsedParent* pParentClass = parentList.first(); pParentClass != 0; pParentClass = parentList.next() )
    {
        pClass = API::getInstance()->classStore()->getClassByName ( pParentClass->name() );

        if ( pClass )
        {
            QValueList<ParsedAttribute*> pTmpList = pClass->getSortedAttributeList();
            QValueList<ParsedAttribute*>::ConstIterator pTmpListIt;
            ParsedAttribute *pAttribute;
            for (pTmpListIt = pTmpList.begin(); pTmpListIt != pTmpList.end(); ++pTmpListIt)
            {
                pAttribute = *pTmpListIt;
                pList->append ( pAttribute );
            }

            *pList = getParentAttributeListForClass ( pClass, pList );
        }
        /*else
          {
          // TODO: look in ClassStore for Namespace classes
          } */
    }

    return *pList;
}

QStringList KDevCodeCompletion::getMethodListForClass( QString strClass, QString strMethod )
{
     QStringList functionList;

     ParsedClass* pClass = API::getInstance()->classStore()->getClassByName ( strClass );
     if ( pClass )
     {
         QValueList<ParsedMethod*> pMethodList;

         // Load the methods, slots, signals of the current class and its parents into the list
         pMethodList = pClass->getSortedMethodList();

         QValueList<ParsedMethod*> pTmpList = pClass->getSortedMethodList();
         QValueList<ParsedMethod*>::ConstIterator pTmpListIt;
         ParsedMethod *pMethod;
         
         for (pTmpListIt = pTmpList.begin(); pTmpListIt != pTmpList.end(); ++pTmpListIt )
         {
             pMethod = *pTmpListIt;
             if( pMethod->name() == strMethod ){
                 QString s;
                 s = pMethod->asString();
                 functionList << s;
             }
         }

         pTmpList = pClass->getSortedSlotList();
         for (pTmpListIt = pTmpList.begin(); pTmpListIt != pTmpList.end(); ++pTmpListIt )
         {
             pMethod = *pTmpListIt;
             if( pMethod->name() == strMethod ){
                 QString s;
                 s = pMethod->asString();
                 functionList << s;
             }
         }

         pTmpList = pClass->getSortedSignalList();
         for (pTmpListIt = pTmpList.begin(); pTmpListIt != pTmpList.end(); ++pTmpListIt )
         {
             pMethod = *pTmpListIt;
             if( pMethod->name() == strMethod ){
                 QString s;
                 s = pMethod->asString();
                 functionList << s;
             }
         }

         getParentMethodListForClass( pClass, strMethod, functionList );

     }
     return functionList;
}

void KDevCodeCompletion::getParentMethodListForClass( ParsedClass* pClass,
                                                     QString strMethod,
                                                     QStringList& methodList )
{
    QList<ParsedParent> parentList = pClass->parents;

    for ( ParsedParent* pParentClass = parentList.first(); pParentClass != 0; pParentClass = parentList.next() )
    {
        pClass = API::getInstance()->classStore()->getClassByName ( pParentClass->name() );

        if ( pClass )
        {
            QValueList<ParsedMethod*> pTmpList = pClass->getSortedMethodList();
            QValueList<ParsedMethod*>::ConstIterator pTmpListIt;
            ParsedMethod *pMethod;
            
            for (pTmpListIt = pTmpList.begin(); pTmpListIt != pTmpList.end(); ++pTmpListIt )
            {
                pMethod = *pTmpListIt;
                if( pMethod->name() == strMethod ){
                    QString s;
                    s = pMethod->asString();
                    methodList << s;
                }
            }

            pTmpList = pClass->getSortedSlotList();
            for (pTmpListIt = pTmpList.begin(); pTmpListIt != pTmpList.end(); ++pTmpListIt )
            {
                pMethod = *pTmpListIt;
                if( pMethod->name() == strMethod ){
                    QString s;
                    s = pMethod->asString();
                    methodList << s;
                }
            }

            pTmpList = pClass->getSortedSignalList();
            for (pTmpListIt = pTmpList.begin(); pTmpListIt != pTmpList.end(); ++pTmpListIt )
            {
                pMethod = *pTmpListIt;
                if( pMethod->name() == strMethod ){
                    QString s;
                    s = pMethod->asString();
                    methodList << s;
                }
            }

            getParentMethodListForClass ( pClass, strMethod, methodList );
        }
        /*else
          {
          // TODO: look in ClassStore for Namespace classes
          } */
    }

}

QStringList KDevCodeCompletion::getFunctionList( QString strMethod )
{
    ParsedScopeContainer* pScope = &API::getInstance()->classStore()->globalContainer;
    QStringList functionList;
    QValueList<ParsedMethod*> pMethodList;

    pMethodList = pScope->getSortedMethodList();
    if( !pMethodList.isEmpty() ){
        QValueList<ParsedMethod*>::ConstIterator pMethodListIt;
        ParsedMethod *pMethod;
        for (pMethodListIt = pMethodList.begin(); pMethodListIt != pMethodList.end(); ++pMethodListIt ) {
            pMethod = *pMethodListIt;
            if( pMethod->name() == strMethod ){
                QString s;
                s = pMethod->asString();
                functionList << s;
            }
        }
    }
    return functionList;
}



QString KDevCodeCompletion::getMethodBody( Kate::Document* doc, int iLine, int iCol, QString* classname )
{
    kdDebug() << "KDevCodeCompletion::getMethodBody()" << endl;

    // TODO: add support for function pointer arguments
    KDevRegExp regMethod( "[ \t]*([a-zA-Z0-9_]+[ \t]*::)?[ \t]*[~a-zA-Z0-9_][a-zA-Z0-9_]*[ \t]*\\(([^)]*)\\)[ \t]*(:[^{]*)?\\{" );

    //QRegExp qt_rx( "Q_[A-Z]+" );
    QRegExp strconst_rx( "\"[^\"]*\"" );
    QRegExp chrconst_rx( "'[^']*'" );
    QRegExp newline_rx( "\n" );
    QRegExp comment_rx( "//[^\n]*" );
    QRegExp preproc_rx( "^[ \t]*#[^\n]*$" );

    QString text = doc->textLine( iLine ).left( iCol );
    --iLine;
    while( iLine >= 0 ){

        text.prepend( doc->textLine( iLine ).simplifyWhiteSpace() + "\n" );
        if( (iLine % 50) == 0 ){
            // kdDebug() << "---> iLine = " << iLine << endl;

            QString contents = text;

            //kdDebug() << ".... 2 " << endl;

            contents = contents
                       // .replace( qt_rx, "" )
                       .replace( comment_rx, "" )
                       .replace( preproc_rx, "" )
                       .replace( strconst_rx, "\"\"" )
                       .replace( chrconst_rx, "''" )
                       .replace( newline_rx, " " );

            contents = remove_comment( contents );
            contents = remove_keywords( contents );
            contents = remove( contents, '[', ']' );

            //kdDebug() << ".... 3 " << endl;

            QValueList<KDevRegExpCap> methods = regMethod.findAll( contents );
            if( methods.count() != 0 ){

                //kdDebug() << ".... 4 " << endl;

                KDevRegExpCap m = methods.last();

                contents = contents.mid( m.start() );
                regMethod.search( m.text() );
                contents.prepend( regMethod.cap( 2 ).replace( QRegExp(","), ";" ) + ";\n" );
                //kdDebug() << "-----> text = " << m.text() << endl;
                if( classname ){
                    QString s = regMethod.cap( 1 ).stripWhiteSpace();
                    if( s.length() ){
                        // remove "::"
                        s = s.left( s.length() - 2 ).stripWhiteSpace();
                    }
                    *classname = s;
                }

                return contents;
            }

            //kdDebug() << ".... 5 " << endl;

        }

        --iLine;
    }

    return QString::null;
}

bool KDevCodeCompletion::checkEnd(const QString &str, const QString &suffix)
{
#if QT_VERSION >=300
   return str.endsWith(suffix);
#else
   return (str.right(suffix.length()) == suffix);
#endif
}

void KDevCodeCompletion::popFrontStringList(QStringList &slist)
{
#if QT_VERSION >=300
  slist.pop_front();
#else
  QStringList::Iterator it;
  if ((it=slist.begin())!=slist.end())
    slist.remove(it);
#endif

}
