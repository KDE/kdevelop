
#include "kwrite/kwview.h"
#include <qsizegrip.h>

#include "codecompletion_iface_impl.h"
#include <iostream.h>


CodeCompletionIfaceImpl::CodeCompletionIfaceImpl(KWrite *edit, KEditor::Document *parent, KEditor::Editor *editor)
  : CodeCompletionDocumentIface(parent, editor), m_edit(edit){

  completionPopup = new QVBox( 0, 0, WType_Popup );
  completionPopup->setFrameStyle( QFrame::Box | QFrame::Plain );
  completionPopup->setLineWidth( 1 );
  
  completionListBox = new QListBox( completionPopup );
  completionListBox->setFrameStyle( QFrame::NoFrame );
  //  completionListBox->installEventFilter( this );
  completionListBox->setHScrollBarMode( QScrollView::AlwaysOn );
  completionListBox->setVScrollBarMode( QScrollView::AlwaysOn );
  completionListBox->setCornerWidget( new QSizeGrip( completionListBox ) );
  completionPopup->installEventFilter( this );
  completionPopup->setFocusProxy( completionListBox );
  //  _widget->installEventFilter( this);
}

void CodeCompletionIfaceImpl::showCompletionBox(QValueList<KEditor::CompletionEntry>* complList){
  cerr << "showCompletionBox " << endl;

  QValueList<KEditor::CompletionEntry>::Iterator it;
  for( it = complList->begin(); it != complList->end(); ++it ){
      cerr << "insert " << endl;
    completionListBox->insertItem((*it).prefix + " " + (*it).text + " " + (*it).postfix);
  }
  completionPopup->resize( completionListBox->sizeHint() +
			   QSize( completionListBox->verticalScrollBar()->width() + 4,
				  completionListBox->horizontalScrollBar()->height() + 4 ) );
  completionListBox->setCurrentItem( 0 );
  completionListBox->setFocus();
  //  completionPopup->move( _widget->mapToGlobal( _widget->getCursorPoint() + QPoint(0,7)) );
  completionPopup->show();
}

