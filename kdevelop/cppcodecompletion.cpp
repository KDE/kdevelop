// basic idea from qtdesigner by TrollTech
#include "kwrite/kwview.h"
#include "kwrite/kwdoc.h"
#include "cppcodecompletion.h"
#include "ceditwidget.h"
#include "classparser/ClassStore.h"

#include <kdebug.h>
#include <qsizegrip.h>
#include <qapplication.h>
#include <qregexp.h>

#include "codecompletion_arghint.h"

#include <iostream.h>


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

static QString purify( const QString& decl )
{
    QRegExp rx( "(\\*|&|\\bconst\\b)" );
    QString s = decl;
    s = s.replace( rx, "" ).simplifyWhiteSpace();
    kdDebug() << "purify " << decl << " -- " << s << endl;
    return s;
}

CppCodeCompletion::CppCodeCompletion( CEditWidget *edit, CClassStore* pStore )
    : m_edit(edit), m_pStore( pStore )
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
            QKeyEvent* ke = (QKeyEvent*) e;
            if( ke->key() == Key_Tab && !m_edit->currentWord().isEmpty() ){
                kdDebug() << "--------------------------> expand (disabled by Falk!)" << endl;
//DISABLED_BY_FALK                m_edit->expandText();
//DISABLED_BY_FALK                return TRUE;
            } else if ( ke->key() == Key_Period ||
                        (ke->key() == Key_Greater &&
                         col > 0 && m_edit->textLine( line )[ col-1 ] == '-') ) {
                kdDebug() << "---------------------------> complete (enabled by robe :-)" << endl;
                m_edit->insertText( ke->text() );
                completeText();
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
                        completeText();
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
    kdDebug() << "---------------------------------------------------------" << endl;
    m_pArgHint->move(m_edit->view()->mapToGlobal(m_edit->view()->getCursorCoordinates()));
    m_pArgHint->show();
}

void CppCodeCompletion::slotCursorPosChanged()
{
    QPoint pos = m_edit->view()->cursorPosition();
    m_pArgHint->cursorPositionChanged ( pos.y(), pos.x() );
}


enum { T_ACCESS, T_PAREN, T_BRACKET, T_IDE, T_UNKNOWN };

int CppCodeCompletion::expressionAt( const QString& text, int index )
{
    kdDebug() << "CppCodeCompletion::expressionAt()" << endl;

    int last = T_UNKNOWN;
    int start = index;
    while( index > 0 ){
        while( index > 0 && text[index].isSpace() ){
            --index;
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
    int index = 0;
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
                                               const QValueList<SimpleVariable>& roo,
                                               CClassStore* sigma )
{
    QStringList exprs = splitExpression( expr );
    for( QStringList::Iterator it=exprs.begin(); it!=exprs.end(); ++it ){
        kdDebug() << "expr " << (*it) << endl;
    }


    SimpleVariable v_this = SimpleParser::findVariable( roo, "this" );
    QString type;

    CParsedClass* pThis = sigma->getClassByName( v_this.type );

    if( exprs.count() == 0 ){
        return v_this.type;
    }

    QString e1 = exprs.first().stripWhiteSpace();
    popFrontStringList(exprs);

    if( e1.isEmpty() ){
        type = v_this.type;
    } else if( check_end(e1, "::") ){
        type = e1.left( e1.length() - 2 ).stripWhiteSpace();
    } else {
        int first_paren_index = 0;
        if( (first_paren_index = e1.find('(')) != -1 ){
            e1 = e1.left( first_paren_index ).stripWhiteSpace();
            type = getTypeOfMethod( pThis, e1 );
        } else {
            SimpleVariable v = SimpleParser::findVariable( roo, e1 );
            if( v.type ){
                type = v.type;
            } else {
                type = getTypeOfAttribute( pThis, e1 );
            }
        }
    }

    type = purify( type );
    CParsedClass* pClass = sigma->getClassByName( type );
    while( pClass && exprs.count() ){

        QString e = exprs.first().stripWhiteSpace();
        popFrontStringList(exprs);
        type = "";  // no type

        // kdDebug() << "----------> evaluate " << e << endl;

        int first_paren_index;
        if( e.isEmpty() ){
            break;
        } else if( (first_paren_index = e.find('(')) != -1 ){
            e = e.left( first_paren_index );
            type = getTypeOfMethod( pClass, e );
            pClass = sigma->getClassByName( type );
        } else {
            type = getTypeOfAttribute( pClass, e );
            pClass = sigma->getClassByName( type );
        }
    }

    kdDebug() << "-------------> last type = " << type << endl;

    return type;
}

QString CppCodeCompletion::getTypeOfMethod( CParsedClass* pClass, const QString& name )
{
    if( !pClass || !m_pStore ){
        return QString::null;
    }

    QList<CParsedMethod>* pMethodList = pClass->getMethodByName( name );
    if( pMethodList->count() != 0 ){
        // TODO: check for method's arguments
        QString type = pMethodList->at( 0 )->type;
        return purify( type );
    }

    QList<CParsedParent> parentList = pClass->parents;
    for( CParsedParent* pParent=parentList.first(); pParent!=0; pParent=parentList.next() ){
        CParsedClass* pClass = m_pStore->getClassByName( pParent->name );
        QString type = getTypeOfMethod( pClass, name );
        type = purify( type );
        if( !type.isEmpty() ){
            return type;
        }
    }
    return QString::null;
}

QString CppCodeCompletion::getTypeOfAttribute( CParsedClass* pClass, const QString& name )
{
    if( !pClass || !m_pStore ){
        return QString::null;
    }

    CParsedAttribute* pAttr = pClass->getAttributeByName( name );
    if( pAttr ){
        QString type = pAttr->type;
        return purify( type );
    }

    QList<CParsedParent> parentList = pClass->parents;
    for( CParsedParent* pParent=parentList.first(); pParent!=0; pParent=parentList.next() ){
        CParsedClass* pClass = m_pStore->getClassByName( pParent->name );
        QString type = getTypeOfAttribute( pClass, name );
        type = purify( type );
        if( !type.isEmpty() ){
            return type;
        }
    }
    return QString::null;
}

QValueList<CompletionEntry>
CppCodeCompletion::getEntryListForExpr( const QString& expr,
                                        const QValueList<SimpleVariable>& vars )
{
    QString type = evaluateExpression( expr, vars, m_pStore );
    kdDebug() << "--------> type = " << type << endl;
    QValueList<CompletionEntry> entries = getEntryListForClass( type );
    return entries;
}

QValueList<CompletionEntry> CppCodeCompletion::getEntryListForClass ( QString strClass )
{
    kdDebug() << "CppCodeCompletion::getEntryListForClass()" << endl;
    QValueList<CompletionEntry> entryList;

    CParsedClass* pClass = m_pStore->getClassByName ( strClass );
    if ( pClass )
    {
        QList<CParsedMethod>* pMethodList;
        QList<CParsedAttribute>* pAttributeList;

        // Load the methods, slots, signals of the current class and its parents into the list
        pMethodList = pClass->getSortedMethodList();

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

        for ( CParsedMethod* pMethod = pMethodList->first(); pMethod != 0; pMethod = pMethodList->next() )
        {
            CompletionEntry entry;
            entry.text = pMethod->name;
            entry.postfix = "()";
            entryList << entry;
        }

        // Load the attributes of the current class and its parents into the list
        pAttributeList = pClass->getSortedAttributeList();

        pAttributeList = getParentAttributeListForClass ( pClass, pAttributeList );

        for ( CParsedAttribute* pAttribute = pAttributeList->first(); pAttribute != 0; pAttribute = pAttributeList->next() )
        {
            CompletionEntry entry;
            entry.text = pAttribute->name;
            entry.postfix = "";
            entryList << entry;
        }
    }

    kdDebug() << "getParentAttributeListForClass() -- END" << endl;
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
    QRegExp regMethod ("\\s*([_\\w]+)\\s*::\\s*[~\\w_][\\w_]*\\s*\\(([^)]*)\\)\\s*[:{]");

    int iMethodBegin = 0;
    QString text;
    QString strLine;
    for( int i=iLine; i>0; --i ){
        QString s = m_edit->textLine( i );
        s = s.replace( QRegExp("\\bconst\\b"), "" );
        text.prepend( s ).simplifyWhiteSpace();

        if( text.isEmpty()){
            continue;
        }

#warning fixme codecompletion
#if QT_VERSION >=300
        if( regMethod.match(text) != -1 ){
            iMethodBegin = i;
            if( classname ){
                *classname = regMethod.cap( 1 );
            }
            break;
        }
#endif
    }

    if( iMethodBegin == 0 ){
        kdDebug( 9007 ) << "no method declaration found" << endl;
        return QString::null;
    }

    QString strCopy;
#warning fixme codecompletion
#if QT_VERSION >=300
    strCopy += regMethod.cap( 2 ).replace( QRegExp(","), ";" ) + ";\n";
    for( int i = iMethodBegin; i < iLine; i++ )
    {
        strCopy += m_edit->textLine( i ) + "\n";
    }
    strCopy += m_edit->textLine( iLine ).left( iCol );
#endif

    return strCopy;
}

void CppCodeCompletion::completeText()
{
    int nLine = m_edit->view()->cursorPosition().y();
    int nCol = m_edit->view()->cursorPosition().x();

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

    QValueList<SimpleVariable> variableList = SimpleParser::localVariables( contents );
    SimpleVariable v;
    v.name = "this";
    v.scope = 1;
    v.type = className;
    variableList.append( v );

    QString word;
    int start_expr = expressionAt( contents, contents.length() - 1 );
    QString expr;
    if( start_expr != contents.length() - 1 ){
        expr = contents.mid( start_expr, contents.length() - start_expr );
        expr = expr.simplifyWhiteSpace();
    }

#warning fixme codecompletion
#if QT_VERSION >=300
   QRegExp rx( "^.*([_\\w]+)\\s*$" );
    if( rx.exactMatch(expr) )
    {
        word = rx.cap( 1 );
        expr = expr.left( rx.pos(1) );
    }

    kdDebug() << "prefix = |" << word << "|" << endl;
    kdDebug() << "expr = |" << expr << "|" << endl;
#endif
    
    if( showArguments ){
        QString type = evaluateExpression( expr, variableList, m_pStore );
        QStringList functionList = getMethodListForClass( type, word );
        showArgHint( functionList, "()", "," );
    } else {
        QValueList<CompletionEntry> entries;
        entries = getEntryListForExpr( expr, variableList );
        if( entries.count() ){
            showCompletionBox( entries, word.length() );
        }
    }

}

bool CppCodeCompletion::check_end(const QString &str, const QString &suffix)
{
#if QT_VERSION >=300
   return str.ends_with(suffix);
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
