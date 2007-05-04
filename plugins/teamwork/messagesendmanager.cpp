/***************************************************************************
copyright            : (C) 2006 by David Nolden
email                : david.nolden.kdevelop@art-master.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include <icore.h>
#include <idocument.h>
#include <idocumentcontroller.h>
#include <indocumentreference.h>

#include "kdevteamwork_part.h"
#include "messagesendmanager.h"
#include "teamworkfoldermanager.h"

MessageSendManager::MessageSendManager( Ui::Teamwork& widgets ) : m_widgets( widgets ), m_lastDocument(0) {
  connect( KDevTeamworkPart::staticDocumentController(), SIGNAL( documentActivated( KDevelop::IDocument* ) ), this, SLOT( documentActivated( KDevelop::IDocument* ) ) );
  connect( KDevTeamworkPart::staticDocumentController(), SIGNAL( documentLoaded( KDevelop::IDocument* ) ), this, SLOT( documentActivated( KDevelop::IDocument* ) ) );
  connect( KDevTeamworkPart::staticDocumentController(), SIGNAL( documentClosed( KDevelop::IDocument* ) ), this, SLOT( documentClosed( KDevelop::IDocument* ) ) );
  if( KDevTeamworkPart::staticDocumentController()->activeDocument() )
    documentActivated( KDevTeamworkPart::staticDocumentController()->activeDocument() );
}

void MessageSendManager::documentActivated( KDevelop::IDocument* document ) {
  if( document == m_lastDocument ) return;
  disconnectDocument();
  connectDocument( document );
}

void MessageSendManager::documentClosed( KDevelop::IDocument* document ) {
  if( document == m_lastDocument )
    disconnectDocument();
}

void MessageSendManager::connectDocument( KDevelop::IDocument* document ) {
  m_lastDocument = document;
  if( !document->textDocument() ) return;
  connect( document->textDocument(), SIGNAL(destroyed( QObject* )), this, SLOT( documentDestroyed( QObject* ) ) );
  connect( document->textDocument(), SIGNAL( viewCreated( KTextEditor::Document* , KTextEditor::View* ) ), this, SLOT( viewCreated( KTextEditor::Document*, KTextEditor::View* ) ) );
  
  KTextEditor::View* v  = view();
  if( !v ) return;
  connectView( v );
}

void MessageSendManager::connectView( KTextEditor::View* v ) {
  disconnect( v, SIGNAL( selectionChanged( KTextEditor::View* ) ), this, SLOT( selectionChanged( KTextEditor::View* ) ) ); ///Just to make sure that multiple connections don't happen
  connect( v, SIGNAL( selectionChanged( KTextEditor::View* ) ), this, SLOT( selectionChanged( KTextEditor::View* ) ) );
}

KTextEditor::View* MessageSendManager::view() {
  if( !m_lastDocument ) return 0;
  if( !m_lastDocument->textDocument() ) return 0;
  return m_lastDocument->textDocument()->activeView();
}

void MessageSendManager::disconnectDocument() {
  KDevelop::IDocument* document = m_lastDocument;
  if( !document ) return;
  if( !document->textDocument() ) return;
  disconnect( document->textDocument(), SIGNAL(destroyed( QObject* )), this, SLOT( documentDestroyed( QObject* ) ) );
  disconnect( document->textDocument(), SIGNAL( viewCreated( KTextEditor::Document* , KTextEditor::View* ) ), this, SLOT( viewCreated( KTextEditor::Document*, KTextEditor::View* ) ) );
  
  m_lastDocument = 0;
}

void MessageSendManager::documentDestroyed( QObject* /*obj*/ ) {
  m_lastDocument = 0;
}

void MessageSendManager::viewCreated( KTextEditor::Document* , KTextEditor::View* ) {
}

void MessageSendManager::selectionChanged( KTextEditor::View* view ) {
  
  m_currentSelection = view->selectionRange();
  KTextEditor::Document* doc = view->document();
  m_currentFile = "";
  if( doc )
      m_currentFile = TeamworkFolderManager::workspaceRelative( doc->url() );

  ///Now check if the message-send-widget is visible
  if( m_widgets.toolBox->currentWidget() == m_widgets.messaging && !m_currentFile.isEmpty() ) {
      if( m_currentSelection.isEmpty() ) {
        ///Remove the selection
    } else {
      m_widgets.reference->setText( InDocumentReference::createReference( m_currentFile.pathOrUrl(), m_currentSelection.start().line(), m_currentSelection.start().column(), m_currentSelection.end().line(), m_currentSelection.end().column() ) );
    }
  }
}

#include "messagesendmanager.moc"

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
