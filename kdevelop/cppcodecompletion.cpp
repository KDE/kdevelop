/***************************************************************************
                          cppcodecompletion.cpp  -  description
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

// basic idea from qtdesigner by TrollTech
#include "kwrite/kwview.h"
#include "kwrite/kwdoc.h"
#include "cppcodecompletion.h"
#include "ceditwidget.h"
#include "ckdevelop.h"
#include "cproject.h"
#include "kdevregexp.h"
#include "codecompletion_arghint.h"
#include "classparser/ClassStore.h"

#include <qsizegrip.h>
#include <qapplication.h>
#include <qregexp.h>
#include <qmap.h>

#include <kdebug.h>
#include <kregexp.h>


class CompletionItem : public QListBoxText
{
public:
    CompletionItem( QListBox *lb, CompletionEntry entry )
        : QListBoxText( lb ) {
        if(entry.postfix=="()"){ // should be configurable
            setText( entry.prefix + " " + entry.text + entry.postfix);
        }
        else{
            setText( entry.prefix + " " + entry.text + " " + entry.postfix);
        }
        m_entry = entry;
    }
    CompletionEntry m_entry;
};

static QValueList<CompletionEntry> unique( const QValueList<CompletionEntry>& entryList )
{
    QValueList<CompletionEntry> l;
    QMap<QString, bool> map;
    QValueList<CompletionEntry>::ConstIterator it=entryList.begin();
    while( it != entryList.end() ){
        CompletionEntry e = *it++;
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

static QString purify( const QString& decl )
{
    QString s = decl;

    QRegExp rx1( "\\*" );
    QRegExp rx2( "&" );
    s = s.replace( rx1, "" ).replace( rx2, "" ).simplifyWhiteSpace();
    return s;
}

static QString remove( QString text, const QChar& l, const QChar& r )
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

static QString remove_keywords( QString text ){
    QRegExp ide_rx( "[_a-zA-Z0-9][_a-zA-Z0-9]*" );
    QStringList keywords = QStringList::split( "|",
            "unsigned|signed|case|delete|return|if|then|else|for|while|do|"
            "const|static|volatile|extern|struct" );

    QString s;
    int index = 0;
    while( index < text.length() ){
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

static QString remove_comment( QString text ){
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

CppCodeCompletion::CppCodeCompletion( CEditWidget *edit, CClassStore* pStore, CKDevelop* dev )
    : m_edit(edit), m_pStore( pStore ), m_pDevelop( dev )
{
    m_completionPopup = new QVBox( 0, 0, WType_Popup );
    m_completionPopup->setFrameStyle( QFrame::Box | QFrame::Plain );
    m_completionPopup->setLineWidth( 1 );

    m_completionListBox = new QListBox( m_completionPopup );
    m_completionListBox->setFrameStyle( QFrame::NoFrame );
    m_completionListBox->installEventFilter( this );

    m_edit->view()->installEventFilter( this );

    m_completionPopup->installEventFilter( this );
    m_completionPopup->setFocusProxy( m_completionListBox );

#if 0
    KWriteDoc* doc=dynamic_cast<KWriteDoc*>(m_edit->doc());
    QFont font = doc->getTextFont(0,0);
    m_completionListBox->setFont(QFont(font.family(),font.pointSize()));
#endif

    m_pArgHint = new KDevArgHint ( m_edit, m_edit );
    connect(m_pArgHint,SIGNAL(argHintHided()),SIGNAL(argHintHided()));

    connect(m_edit, SIGNAL ( newCurPos() ), this, SLOT ( slotCursorPosChanged () ) );
}

void CppCodeCompletion::showCompletionBox(QValueList<CompletionEntry> complList,int offset){
    m_complList = complList;
    // align the prefix (begin)
    QValueList<CompletionEntry>::Iterator it;
    unsigned int maxLen =0;
    for( it = m_complList.begin(); it != m_complList.end(); ++it ){
        if(maxLen < (*it).prefix.length()){
            maxLen = (*it).prefix.length();
        }
    }
    for( it = m_complList.begin(); it != m_complList.end(); ++it ){
        QString fillStr;
        fillStr.fill(QChar(' '),maxLen - (*it).prefix.length()); // add some spaces
        (*it).prefix.append(fillStr);
    }
    // alignt the prefix (end)

    m_offset = offset;
    QPoint pos = m_edit->view()->cursorPosition();
    m_lineCursor = pos.y();
    m_colCursor = pos.x();
    m_colCursor = m_colCursor - offset; // calculate the real start of the code completion text
    updateBox(true);

}

bool CppCodeCompletion::eventFilter( QObject *o, QEvent *e ){

    if( o == m_edit->view() || o == m_edit ){
        KWriteView* ed = m_edit->view();
        int line = ed->cursorPosition().y();
        int col = ed->cursorPosition().x();
        if( e->type() == QEvent::KeyPress ){
            // kdDebug() << "Project = " << prj << endl;
            QKeyEvent* ke = (QKeyEvent*) e;
            if( ke->key() == Key_Tab && !m_edit->currentWord().isEmpty() ){
                kdDebug() << "--------------------------> expand (disabled by Falk!)" << endl;
//DISABLED_BY_FALK                m_edit->expandText();
//DISABLED_BY_FALK                return TRUE;
            } else if ( (m_pDevelop->getAutomaticCompletion()) &&
                         (ke->key() == Key_Period ||
                          (ke->key() == Key_Greater &&
                           col > 0 && m_edit->textLine( line )[ col-1 ] == '-')) ) {
                m_edit->insertText( ke->text() );
                TextLine* l = m_edit->doc()->textLine( line );
                int attr = l->getAttr( col );
                if( attr == 13 ){
                    kdDebug() << "---------------------------> complete (enabled by robe :-)" << endl;
                    completeText();
                }
                return TRUE;
            }
        }
    }
    else if ( o == m_completionPopup || o == m_completionListBox || o == m_completionListBox->viewport() ) {
        if ( e->type() == QEvent::KeyPress ) {
            QKeyEvent *ke = (QKeyEvent*)e;
            if ( ke->key() == Key_Left || ke->key() == Key_Right ||
                 ke->key() == Key_Up || ke->key() == Key_Down ||
                 ke->key() == Key_Home || ke->key() == Key_End ||
                 ke->key() == Key_Prior || ke->key() == Key_Next ) {
                return FALSE;
            }

            if (ke->key() == Key_Enter || ke->key() == Key_Return || ke->key() == Key_Tab ) { // return
                CompletionItem* item = static_cast<CompletionItem*> (m_completionListBox->item(m_completionListBox->currentItem()));
                if(item !=0){
                    QString text = item->m_entry.text;
                    QPoint pos = m_edit->view()->cursorPosition();
                    QString currentLine = m_edit->textLine( pos.y() );
                    int len = pos.x() - m_colCursor;
                    QString currentComplText = currentLine.mid(m_colCursor,len);
                    QString add = text.mid(currentComplText.length());
                    if(item->m_entry.postfix == "()"){ // add (
                        m_edit->insertText(add + "(");
                        if( m_pDevelop->getAutomaticArgsHint() ){
                            completeText();
                        }
                        //	    VConfig c;
                        //	    m_edit->view()->getVConfig(c);
                        //	    m_edit->view()->cursorLeft(c);
                    }
                    else{
                        m_edit->insertText(add);
                    }
                    m_completionPopup->hide();
                    m_edit->view()->setFocus();
                    emit completionDone();
                }
                return FALSE;
            }

            if(ke->key() == Key_Escape){ // abort
                m_completionPopup->hide();
                m_edit->view()->setFocus();
                emit completionAborted();
                return FALSE;
            }

            QApplication::sendEvent(m_edit->view(), e ); // redirect the event to the editor
            QPoint pos = m_edit->view()->cursorPosition();
            if(m_colCursor+m_offset > pos.x()){ // the cursor is to far left
                m_completionPopup->hide();
                m_edit->view()->setFocus();
                emit completionAborted();
                return FALSE;
            }
            updateBox();
            return TRUE;
        }

        if(e->type() == QEvent::FocusOut){
            m_completionPopup->hide();
            emit completionAborted();
        }
    }
    return FALSE;
}

void CppCodeCompletion::updateBox(bool newCoordinate){
    m_completionListBox->clear();
    QPoint pos = m_edit->view()->cursorPosition();
    QString currentLine = m_edit->textLine( pos.y() );

    int len = pos.x() - m_colCursor;
    QString currentComplText = currentLine.mid(m_colCursor,len);
    QValueList<CompletionEntry>::Iterator it;
    for( it = m_complList.begin(); it != m_complList.end(); ++it ){
        if((*it).text.startsWith(currentComplText)){
            new CompletionItem(m_completionListBox,*it);
        }
    }
    if(m_completionListBox->count()==0){
        m_completionPopup->hide();
        m_edit->view()->setFocus();
        emit completionAborted();
        return;
    }
    m_completionListBox->setCurrentItem( 0 );
    m_completionListBox->setSelected( 0,true );
    m_completionListBox->setFocus();
    if(newCoordinate){
        m_completionPopup->resize( m_completionListBox->sizeHint() +
                                   QSize( m_completionListBox->verticalScrollBar()->width() + 4,
                                          m_completionListBox->horizontalScrollBar()->height() + 4 ) );
        // m_edit->paintCursor();
        QPoint pt = m_edit->getCursorCoordinates();
        // QPoint pt = QPoint( 0, 0 );
        m_completionPopup->move(m_edit->view()->mapToGlobal(pt));
    }
    m_completionListBox->sort();
    m_completionPopup->show();
}

void CppCodeCompletion::showArgHint ( QStringList functionList, const QString& strWrapping, const QString& strDelimiter )
{
    m_pArgHint->reset();

    m_pArgHint->setArgMarkInfos ( strWrapping, strDelimiter );

    QStringList::Iterator it;

    int nNum = 0;

    for( it = functionList.begin(); it != functionList.end(); it++ )
    {
        m_pArgHint->setFunctionText ( nNum, ( *it ) );

        nNum++;
    }
    // m_edit->view()->paintCursor();
    m_pArgHint->move(m_edit->view()->mapToGlobal(m_edit->view()->getCursorCoordinates()));
    m_pArgHint->show();
}

void CppCodeCompletion::slotCursorPosChanged()
{
    QPoint pos = m_edit->view()->cursorPosition();
    if( m_pArgHint->isVisible() ){
        m_pArgHint->cursorPositionChanged ( pos.y(), pos.x() );
    }
}


enum { T_ACCESS, T_PAREN, T_BRACKET, T_IDE, T_UNKNOWN };

int CppCodeCompletion::expressionAt( const QString& text, int index )
{
    kdDebug() << "CppCodeCompletion::expressionAt()" << endl;

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

QStringList CppCodeCompletion::splitExpression( const QString& text )
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

QString CppCodeCompletion::evaluateExpression( const QString& expr,
                                               SimpleContext* ctx,
                                               CClassStore* sigma )
{
    QStringList exprs = splitExpression( expr );
//    for( QStringList::Iterator it=exprs.begin(); it!=exprs.end(); ++it ){
//        kdDebug() << "expr " << (*it) << endl;
//    }


    SimpleVariable v_this = ctx->findVariable( "this" );
    CParsedClass* pThis = sigma->getClassByName( v_this.type );
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
                        if( start_expr != subexpr.length()-1 ){
                            subexpr = subexpr.mid( start_expr );
                            kdDebug() << "subexpr = " << subexpr << endl;
                        }
                    }
                    type = evaluateExpression( subexpr, ctx, sigma );
                } else {
                    // e1 is cast
                    kdDebug() << "maybe a cast = " << e1 << endl;
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
                type = getTypeOfMethod( pThis, e1 );
            }
        } else {
            SimpleVariable v = ctx->findVariable( e1 );
            if( v.type ){
                // e1 is a local variable
                type = v.type;
            } else {
                // e1 is an attribute
                type = getTypeOfAttribute( pThis, e1 );
            }
        }
    }

    type = purify( type );
    CParsedContainer* pContainer = sigma->getClassByName( type );
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

QString CppCodeCompletion::getTypeOfMethod( CParsedContainer* pContainer, const QString& name )
{
    if( !pContainer || !m_pStore ){
        return QString::null;
    }

    QList<CParsedMethod>* pMethodList = pContainer->getMethodByName( name );
    if( pMethodList->count() != 0 ){
        // TODO: check for method's arguments
        QString type = pMethodList->at( 0 )->type;
        return purify( type );
    }

    CParsedClass* pClass = dynamic_cast<CParsedClass*>( pContainer );
    if( pClass ){
        QList<CParsedParent> parentList = pClass->parents;
        for( CParsedParent* pParent=parentList.first(); pParent!=0; pParent=parentList.next() ){
            CParsedClass* pClass = m_pStore->getClassByName( pParent->name );
            QString type = getTypeOfMethod( pClass, name );
            type = purify( type );
            if( !type.isEmpty() ){
                return type;
            }
        }
    }
    return QString::null;
}

QString CppCodeCompletion::getTypeOfAttribute( CParsedContainer* pContainer, const QString& name )
{
    if( !pContainer || !m_pStore ){
        return QString::null;
    }

    CParsedAttribute* pAttr = pContainer->getAttributeByName( name );
    if( pAttr ){
        QString type = pAttr->type;
        return purify( type );
    }

    CParsedClass* pClass = dynamic_cast<CParsedClass*>( pContainer );
    if( pClass ){
        QList<CParsedParent> parentList = pClass->parents;
        for( CParsedParent* pParent=parentList.first(); pParent!=0; pParent=parentList.next() ){
            CParsedClass* pClass = m_pStore->getClassByName( pParent->name );
            QString type = getTypeOfAttribute( pClass, name );
            type = purify( type );
            if( !type.isEmpty() ){
                return type;
            }
        }
    }
    return QString::null;
}

QValueList<CompletionEntry>
CppCodeCompletion::getEntryListForExpr( const QString& expr,
                                        SimpleContext* ctx )
{
    QString type = evaluateExpression( expr, ctx, m_pStore );
    kdDebug() << "--------> type = " << type << endl;
    QValueList<CompletionEntry> entries = getEntryListForClass( type );
    return entries;
}

QValueList<CompletionEntry> CppCodeCompletion::getEntryListForClass ( QString strClass )
{
    kdDebug() << "CppCodeCompletion::getEntryListForClass()" << endl;
    QValueList<CompletionEntry> entryList;

    CParsedContainer* pContainer = m_pStore->getClassByName( strClass );
    if( !pContainer ){
        pContainer = m_pStore->globalContainer.getStructByName( strClass );
    }

    if ( pContainer )
    {
        QList<CParsedMethod>* pMethodList;
        QList<CParsedAttribute>* pAttributeList;


        // Load the methods, slots, signals of the current class and its parents into the list
        pMethodList = pContainer->getSortedMethodList();

        // Load the attributes of the current class and its parents into the list
        pAttributeList = pContainer->getSortedAttributeList();

        CParsedClass* pClass = dynamic_cast<CParsedClass*>( pContainer );
        if( pClass ){

            QList<CParsedMethod>* pTmpList = pClass->getSortedSlotList();
            for ( CParsedMethod* pMethod = pTmpList->first(); pMethod != 0; pMethod = pTmpList->next() )
            {
                pMethodList->append ( pMethod );
            }

            pTmpList = pClass->getSortedSignalList();
            for ( CParsedMethod* pMethod = pTmpList->first(); pMethod != 0; pMethod = pTmpList->next() )
            {
                pMethodList->append ( pMethod );
            }

            pMethodList = getParentMethodListForClass ( pClass, pMethodList );
            pAttributeList = getParentAttributeListForClass ( pClass, pAttributeList );
        }

        for ( CParsedMethod* pMethod = pMethodList->first(); pMethod != 0; pMethod = pMethodList->next() )
        {
            CompletionEntry entry;
            entry.text = pMethod->name;
            entry.postfix = "()";
            entryList << entry;
        }

        for ( CParsedAttribute* pAttribute = pAttributeList->first();
              pAttribute != 0; pAttribute = pAttributeList->next() )
        {
            CompletionEntry entry;
            entry.text = pAttribute->name;
            entry.postfix = "";
            entryList << entry;
        }
    }

    kdDebug() << "getEntryListForClass() -- END" << endl;
    return entryList;
}

QList<CParsedMethod>* CppCodeCompletion::getParentMethodListForClass ( CParsedClass* pClass, QList<CParsedMethod>* pList )
{
    QList<CParsedParent> parentList = pClass->parents;

    for ( CParsedParent* pParentClass = parentList.first(); pParentClass != 0; pParentClass = parentList.next() )
    {
        pClass = m_pStore->getClassByName ( pParentClass->name );

        if ( pClass )
        {
            QList<CParsedMethod>* pTmpList = pClass->getSortedMethodList();
            for ( CParsedMethod* pMethod = pTmpList->first(); pMethod != 0; pMethod = pTmpList->next() )
            {
                pList->append ( pMethod );
            }

            pTmpList = pClass->getSortedSlotList();
            for ( CParsedMethod* pMethod = pTmpList->first(); pMethod != 0; pMethod = pTmpList->next() )
            {
                pList->append ( pMethod );
            }

            pTmpList = pClass->getSortedSignalList();
            for ( CParsedMethod* pMethod = pTmpList->first(); pMethod != 0; pMethod = pTmpList->next() )
            {
                pList->append ( pMethod );
            }

            pList = getParentMethodListForClass ( pClass, pList );
        }
        /*else
          {
          // TODO: look in ClassStore for Namespace classes
          } */
    }

    return pList;
}

QList<CParsedAttribute>* CppCodeCompletion::getParentAttributeListForClass ( CParsedClass* pClass, QList<CParsedAttribute>* pList )
{
    QList<CParsedParent> parentList = pClass->parents;

    for ( CParsedParent* pParentClass = parentList.first(); pParentClass != 0; pParentClass = parentList.next() )
    {
        pClass = m_pStore->getClassByName ( pParentClass->name );

        if ( pClass )
        {
            QList<CParsedAttribute>* pTmpList = pClass->getSortedAttributeList();
            for ( CParsedAttribute* pAttribute = pTmpList->first(); pAttribute != 0; pAttribute = pTmpList->next() )
            {
                pList->append ( pAttribute );
            }

            pList = getParentAttributeListForClass ( pClass, pList );
        }
        /*else
          {
          // TODO: look in ClassStore for Namespace classes
          } */
    }

    return pList;
}

QStringList CppCodeCompletion::getMethodListForClass( QString strClass, QString strMethod )
{
     QStringList functionList;

     CParsedClass* pClass = m_pStore->getClassByName ( strClass );
     if ( pClass )
     {
         QList<CParsedMethod>* pMethodList;

         // Load the methods, slots, signals of the current class and its parents into the list
         pMethodList = pClass->getSortedMethodList();

         QList<CParsedMethod>* pTmpList = pClass->getSortedMethodList();
         for ( CParsedMethod* pMethod = pTmpList->first(); pMethod != 0; pMethod = pTmpList->next() )
         {
             if( pMethod->name == strMethod ){
                 QString s;
                 pMethod->asString( s );
                 functionList << s;
             }
         }

         pTmpList = pClass->getSortedSlotList();
         for ( CParsedMethod* pMethod = pTmpList->first(); pMethod != 0; pMethod = pTmpList->next() )
         {
             if( pMethod->name == strMethod ){
                 QString s;
                 pMethod->asString( s );
                 functionList << s;
             }
         }

         pTmpList = pClass->getSortedSignalList();
         for ( CParsedMethod* pMethod = pTmpList->first(); pMethod != 0; pMethod = pTmpList->next() )
         {
             if( pMethod->name == strMethod ){
                 QString s;
                 pMethod->asString( s );
                 functionList << s;
             }
         }

         getParentMethodListForClass( pClass, strMethod, functionList );

     }
     return functionList;
}

void CppCodeCompletion::getParentMethodListForClass( CParsedClass* pClass,
                                                     QString strMethod,
                                                     QStringList& methodList )
{
    QList<CParsedParent> parentList = pClass->parents;

    for ( CParsedParent* pParentClass = parentList.first(); pParentClass != 0; pParentClass = parentList.next() )
    {
        pClass = m_pStore->getClassByName ( pParentClass->name );

        if ( pClass )
        {
            QList<CParsedMethod>* pTmpList = pClass->getSortedMethodList();
            for ( CParsedMethod* pMethod = pTmpList->first(); pMethod != 0; pMethod = pTmpList->next() )
            {
                if( pMethod->name == strMethod ){
                    QString s;
                    pMethod->asString( s );
                    methodList << s;
                }
            }

            pTmpList = pClass->getSortedSlotList();
            for ( CParsedMethod* pMethod = pTmpList->first(); pMethod != 0; pMethod = pTmpList->next() )
            {
                if( pMethod->name == strMethod ){
                    QString s;
                    pMethod->asString( s );
                    methodList << s;
                }
            }

            pTmpList = pClass->getSortedSignalList();
            for ( CParsedMethod* pMethod = pTmpList->first(); pMethod != 0; pMethod = pTmpList->next() )
            {
                if( pMethod->name == strMethod ){
                    QString s;
                    pMethod->asString( s );
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

QStringList CppCodeCompletion::getFunctionList( QString strMethod )
{
    CParsedScopeContainer* pScope = &m_pStore->globalContainer;
    QStringList functionList;
    QList<CParsedMethod>* pMethodList = 0;

    pMethodList = pScope->getSortedMethodList();
    if( pMethodList ){
        for ( CParsedMethod* pMethod = pMethodList->first(); pMethod != 0;
              pMethod = pMethodList->next() ) {
            if( pMethod->name == strMethod ){
                QString s;
                pMethod->asString( s );
                functionList << s;
            }
        }
    }
    return functionList;
}



QString CppCodeCompletion::getMethodBody( int iLine, int iCol, QString* classname )
{
    kdDebug() << "CppCodeCompletion::getMethodBody()" << endl;

    KDevRegExp regMethod( "[ \t]*([a-zA-Z0-9_]+?)[ \t]*(::)?[ \t]*[~a-zA-Z0-9_][a-zA-Z0-9_]*[ \t]*\\(([^)]*)\\)[ \t]*(:[^{]*)?\\{" );

    QRegExp qt_rx( "Q_[A-Z]+" );
    QRegExp strconst_rx( "\"[^\"]*\"" );
    QRegExp chrconst_rx( "'[^']*'" );
    QRegExp newline_rx( "\n" );
    QRegExp comment_rx( "//[^\n]*" );
    QRegExp preproc_rx( "^[ \t]*#[^\n]*$" );

    QString text = m_edit->textLine( iLine ).left( iCol );
    --iLine;
    while( iLine >= 0 ){

        text.prepend( m_edit->textLine( iLine ).simplifyWhiteSpace() + "\n" );
        if( (iLine % 50) == 0 ){
            // kdDebug() << "---> iLine = " << iLine << endl;

            QString contents = text;

            kdDebug() << ".... 2 " << endl;

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

            kdDebug() << ".... 3 " << endl;

            QValueList<KDevRegExpCap> methods = regMethod.findAll( contents );
            if( methods.count() != 0 ){

                kdDebug() << ".... 4 " << endl;

                KDevRegExpCap m = methods.last();

                contents = contents.mid( m.start() );
                regMethod.search( m.text() );
                contents.prepend( regMethod.cap( 3 ).replace( QRegExp(","), ";" ) + ";\n" );
                kdDebug() << "-----> text = " << m.text() << endl;
                if( classname ){
                    *classname = regMethod.cap( 1 );
                }

                return contents;
            }

            kdDebug() << ".... 5 " << endl;

        }

        --iLine;
    }

    return QString::null;
}

void CppCodeCompletion::completeText()
{
    int nLine = m_edit->view()->cursorPosition().y();
    int nCol = m_edit->view()->cursorPosition().x();
    // kdDebug() << "nLine = " << nLine << endl;
    // kdDebug() << "nCol = " << nCol << endl;

    QString strCurLine = m_edit->textLine( nLine );
    QString className;
    QString contents;
    bool showArguments = FALSE;

    if( strCurLine[ nCol-1 ] == '(' ){
        --nCol;
        showArguments = TRUE;
    }

    contents = getMethodBody( nLine, nCol, &className );
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
            QString type = evaluateExpression( expr, ctx, m_pStore );
            QStringList functionList = getMethodListForClass( type, word );
            showArgHint( functionList, "()", "," );
        } else {
            QValueList<CompletionEntry> entries;
            QString type;

            SimpleVariable v = ctx->findVariable( word );
            if( expr.isEmpty() && !v.type.isEmpty() ){
                type = v.type;
            } else {
                type = evaluateExpression( expr, ctx, m_pStore );
            }

            entries = unique( getEntryListForClass( type ) );
            if( entries.count() ){
                showCompletionBox( entries, word.length() );
            }
        }
    }

    delete( ctx );
    ctx = 0;
}

bool CppCodeCompletion::checkEnd(const QString &str, const QString &suffix)
{
#if QT_VERSION >=300
   return str.endsWith(suffix);
#else
   return (str.right(suffix.length()) == suffix);
#endif
}

void CppCodeCompletion::popFrontStringList(QStringList &slist)
{
#if QT_VERSION >=300
  slist.pop_front();
#else
  QStringList::Iterator it;
  if ((it=slist.begin())!=slist.end())
    slist.remove(it);
#endif

}

#include "cppcodecompletion.moc"
