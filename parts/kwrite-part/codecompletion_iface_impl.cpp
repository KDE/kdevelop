// basic idea from qtdesigner by TrollTech
#include "kwrite/kwview.h"
#include "kwrite/kwdoc.h"
#include <qsizegrip.h>
#include <qapp.h>

#include "codecompletion_arghint.h"

#include "codecompletion_iface_impl.h"
#include <iostream.h>


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

CodeCompletionIfaceImpl::CodeCompletionIfaceImpl(KWrite *edit, KEditor::Document *parent, KEditor::Editor *editor)
  : CodeCompletionDocumentIface(parent, editor), m_edit(edit){

  m_completionPopup = new QVBox( 0, 0, WType_Popup );
  m_completionPopup->setFrameStyle( QFrame::Box | QFrame::Plain );
  m_completionPopup->setLineWidth( 1 );
  
  m_completionListBox = new QListBox( m_completionPopup );
  m_completionListBox->setFrameStyle( QFrame::NoFrame );
  m_completionListBox->installEventFilter( this );

  m_completionPopup->installEventFilter( this );
  m_completionPopup->setFocusProxy( m_completionListBox );

  QFont font = m_edit->doc()->getTextFont(0,0);
  m_completionListBox->setFont(QFont(font.family(),font.pointSize()));

  m_pArgHint = new KDevArgHint ( m_edit );

  connect(edit, SIGNAL ( newCurPos() ), this, SLOT ( slotCursorPosChanged () ) );
}

void CodeCompletionIfaceImpl::showCompletionBox(QValueList<KEditor::CompletionEntry> complList,int offset){
  cerr << "showCompletionBox " << endl;

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

bool CodeCompletionIfaceImpl::eventFilter( QObject *o, QEvent *e ){
  
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
	  QString currentLine = m_edit->currentTextLine();
	  int len = m_edit->currentColumn() - m_colCursor;
	  QString currentComplText = currentLine.mid(m_colCursor,len);
	  QString add = text.mid(currentComplText.length());
	  if(item->m_entry.postfix == "()"){ // add ()
	    m_edit->insertText(add + "()");
	    VConfig c;
	    m_edit->view()->getVConfig(c);
	    m_edit->view()->cursorLeft(c);
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
      if(m_colCursor+m_offset > m_edit->currentColumn()){ // the cursor is to far left
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

void CodeCompletionIfaceImpl::updateBox(bool newCoordinate){
  m_completionListBox->clear();
  QString currentLine = m_edit->currentTextLine();
  cerr << endl << "Column:" << m_colCursor;
  cerr << endl << "Line:" << currentLine;
  cerr << endl << "CurrentColumn:" << m_edit->currentColumn();
  int len = m_edit->currentColumn() - m_colCursor;
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
    m_edit->view()->paintCursor();
    m_completionPopup->move(m_edit->view()->mapToGlobal(m_edit->view()->getCursorCoordinates()));
  }
  m_completionPopup->show();
}

void CodeCompletionIfaceImpl::showArgHint ( QStringList functionList, const QString& strWrapping, const QString& strDelimiter )
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

	m_pArgHint->move(m_edit->view()->mapToGlobal(m_edit->view()->getCursorCoordinates()));
	m_pArgHint->show();

}

void CodeCompletionIfaceImpl::slotCursorPosChanged()
{
	m_pArgHint->cursorPositionChanged ( document(), m_edit->currentLine(), m_edit->currentColumn() );
}


#include "codecompletion_iface_impl.moc"
