
#include "codecompletion_iface_impl.h"
#include "codecompletion_arghint.h"

#include <private/qrichtext_p.h>
#include <qvbox.h>
#include <qlistbox.h>
#include <qeditor.h>
#include <qstringlist.h>
#include <qsizegrip.h>
#include <qapplication.h>

using namespace KEditor;

#include <iostream>
using namespace std;

class CompletionItem : public QListBoxText
{
public:
    CompletionItem( QListBox *lb,KEditor::CompletionEntry entry )
        : QListBoxText( lb ) {
        if(entry.postfix=="()"){ // should be configurable
            setText( entry.prefix + " " + entry.text + entry.postfix);
        }
        else{
            setText( entry.prefix + " " + entry.text + " " + entry.postfix);
        }
        m_entry = entry;
    }
    KEditor::CompletionEntry m_entry;
};


CodeCompletionDocumentIfaceImpl::CodeCompletionDocumentIfaceImpl(QEditor* edit,
                                                                 Document *parent, Editor *editor)
    : CodeCompletionDocumentIface( parent, editor ), m_edit( edit )
{
    m_completionPopup = new QVBox( 0, 0, WType_Popup );
    m_completionPopup->setFrameStyle( QFrame::Box | QFrame::Plain );
    m_completionPopup->setLineWidth( 1 );

    m_completionListBox = new QListBox( m_completionPopup );
    m_completionListBox->setFrameStyle( QFrame::NoFrame );
    m_completionListBox->installEventFilter( this );

    m_completionPopup->installEventFilter( this );
    m_completionPopup->setFocusProxy( m_completionListBox );

#if 0
    KWriteDoc* doc=dynamic_cast<KWriteDoc*>(m_edit->document());
    QFont font = doc->getTextFont(0,0);
    m_completionListBox->setFont(QFont(font.family(),font.pointSize()));
#endif

    m_pArgHint = new KDevArgHint ( m_edit );
    connect(m_pArgHint,SIGNAL(argHintHided()),SIGNAL(argHintHided()));

    connect(m_edit, SIGNAL(cursorPositionChanged(int,int)), this, SLOT(slotCursorPosChanged(int,int)));
}

CodeCompletionDocumentIfaceImpl::~CodeCompletionDocumentIfaceImpl()
{

}

void CodeCompletionDocumentIfaceImpl::showArgHint( QStringList functionList,
                                                   const QString& strWrapping,
                                                   const QString& strDelimiter )
{
    m_pArgHint->reset();

    m_pArgHint->setArgMarkInfos ( strWrapping, strDelimiter );

    QStringList::Iterator it;

    int nNum = 0;

    for( it = functionList.begin(); it != functionList.end(); it++ )
    {
        cerr << "Insert function text: " << *it << endl;

        m_pArgHint->setFunctionText ( nNum, ( *it ) );

        nNum++;
    }
    int parag, index;
    m_edit->getCursorPosition( &parag, &index );
    QRect rc = m_edit->paragraphRect( parag );
    QPoint pt = rc.bottomLeft();
    QTextStringChar *ch = m_edit->document()->paragAt( parag )->at( index );
    pt.setX( pt.x() + ch->x );
    pt = m_edit->contentsToViewport( pt );
    m_pArgHint->move( m_edit->mapToGlobal( pt ) );
    m_pArgHint->show();
}

bool CodeCompletionDocumentIfaceImpl::eventFilter( QObject *o, QEvent *e )
{
    if ( o == m_completionPopup || o == m_completionListBox || o == m_completionListBox->viewport() ) {
        if ( e->type() == QEvent::KeyPress ) {
            QKeyEvent *ke = (QKeyEvent*)e;
            if ( ke->key() == Key_Left || ke->key() == Key_Right ||
                 ke->key() == Key_Up || ke->key() == Key_Down ||
                 ke->key() == Key_Home || ke->key() == Key_End ||
                 ke->key() == Key_Prior || ke->key() == Key_Next ) {
                return FALSE;
            }

            if (ke->key() == Key_Enter || ke->key() == Key_Return) { // return
                CompletionItem* item = static_cast<CompletionItem*> (m_completionListBox->item(m_completionListBox->currentItem()));
                if(item !=0){
                    QString text = item->m_entry.text;
                    int parag, index;
                    m_edit->getCursorPosition( &parag, &index );
                    QString currentLine = m_edit->text( parag );
                    int len = index - m_colCursor;
                    QString currentComplText = currentLine.mid(m_colCursor,len);
                    QString add = text.mid(currentComplText.length());
                    if(item->m_entry.postfix == "()"){ // add (
                        m_edit->insert(add + "(");
                        //	    VConfig c;
                        //	    m_edit->getVConfig(c);
                        //	    m_edit->cursorLeft(c);
                    }
                    else{
                        m_edit->insert(add);
                    }
                    m_completionPopup->hide();
                    m_edit->setFocus();
                    emit completionDone();
                }
                return FALSE;
            }

            if(ke->key() == Key_Escape){ // abort
                m_completionPopup->hide();
                m_edit->setFocus();
                emit completionAborted();
                return FALSE;
            }

            QApplication::sendEvent(m_edit, e ); // redirect the event to the editor
            int parag, index;
            m_edit->getCursorPosition( &parag, &index );
            if(m_colCursor+m_offset > index){ // the cursor is to far left
                m_completionPopup->hide();
                m_edit->setFocus();
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

void CodeCompletionDocumentIfaceImpl::updateBox( bool newCoordinate )
{
    qDebug( "CodeCompletionDocumentIfaceImpl::updateBox()" );
    m_completionListBox->clear();
    int parag, index;
    m_edit->getCursorPosition( &parag, &index );
    QString currentLine = m_edit->text( parag );
    cerr << endl << "Column:" << m_colCursor;
    cerr << endl << "Line:" << currentLine;
    cerr << endl << "CurrentColumn:" << index;
    int len = index - m_colCursor;
    cerr << endl << "Len:" << len;
    QString currentComplText = currentLine.mid(m_colCursor,len);
    cerr << endl << "TEXT:" << currentComplText;
    QValueList<KEditor::CompletionEntry>::Iterator it;
    cerr << "Count:" << m_complList.count();
    for( it = m_complList.begin(); it != m_complList.end(); ++it ){
        cerr << endl << "insert ";
        if((*it).text.startsWith(currentComplText)){
            new CompletionItem(m_completionListBox,*it);
        }
    }
    if(m_completionListBox->count()==0){
        m_completionPopup->hide();
        m_edit->setFocus();
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
        QRect rc = m_edit->paragraphRect( parag );
        QPoint pt = rc.bottomLeft();
        pt = m_edit->contentsToViewport( pt );
        QTextStringChar *ch = m_edit->document()->paragAt( parag )->at( index );
        pt.setX( pt.x() + ch->x );

        m_completionPopup->move( m_edit->mapToGlobal( pt ) );
//        m_completionPopup->move(m_edit->mapToGlobal(m_edit->getCursorCoordinates()));
    }
    m_completionPopup->show();
}


void CodeCompletionDocumentIfaceImpl::showCompletionBox(QValueList<CompletionEntry> complList,int offset )
{
    qDebug( "roberto -----------------------> CompletionEntry::showCompletionBox()" );
    m_complList = complList;
    // align the prefix (begin)
    QValueList<KEditor::CompletionEntry>::Iterator it;
    int maxLen =0;
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
    m_edit->getCursorPosition(&m_lineCursor, &m_colCursor);
    m_colCursor = m_colCursor - offset; // calculate the real start of the code completion text
    updateBox(true);
}

void CodeCompletionDocumentIfaceImpl::slotCursorPosChanged( int parag,
                                                            int index )
{
    m_pArgHint->cursorPositionChanged ( document(), parag, index );
}
