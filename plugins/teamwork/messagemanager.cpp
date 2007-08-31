/***************************************************************************
  Copyright 2006 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "messagemanager.h"
#include "kdevteamwork.h"
#include "serializationutils.h"
#include "conversationmanager.h"
#include "guimessagehistory.h"
#include "messagehistorymanager.h"
#include "network/messagetypeset.h"
#include <kdialog.h>
#include <QMenu>
#include "kdevteamwork_messageshower.h"

#include <QList>
#include <QAction>

Q_DECLARE_METATYPE( MessagePointer )



MessageManager::MessageManager( KDevTeamwork* tw ) : dispatcher_( *this ), m_teamwork( tw ), m_conversationManager( this ), m_historyManager( m_teamwork->logger() ) {
  m_showMessageAction = new QAction( "Show", this );
  connect( m_showMessageAction, SIGNAL( triggered() ), this, SLOT( slotShowMessage() ) );
  connect( m_teamwork, SIGNAL( init() ), this, SLOT( init() ) );
  m_showMessageAction->setToolTip( "Show the message." );
}

void MessageManager::init() {
}

MessageManager::~MessageManager() {}

void MessageManager::restorePartialProjectSession( const QDomElement* /*el*/ ) {
  /*try {
    xmlDeserializeFromElementItem( el, "ConversationManager", m_conversationManager );
  } catch ( const QString & str ) {
    log( "failed to restore the conversation-manager: " + str, Error );
  }*/
}

void MessageManager::savePartialProjectSession( QDomElement* /*el*/ ) {
  /*try {
    xmlSerializeToElementItem( el, "ConversationManager", m_conversationManager );
  } catch ( const QString & str ) {
    log( "failed to store the conversation-manager: " + str, Error );
  }*/
}

void MessageManager::showMessage( const MessagePointer& msg ) {
  MessagePointer::Locked l = msg;
  if ( !l )
    throw "cannot lock message";
  if ( !l.freeCast<AbstractGUIMessage>() )
    throw "message is no GUI-message";

  KDialog* d = new KDialog( m_teamwork->widget() );
  d->setAttribute( Qt::WA_DeleteOnClose, true );

  l.freeCast<AbstractGUIMessage>() ->showInWidget( d->mainWidget(), m_teamwork );

  d->show();
}

void MessageManager::slotShowMessage() {
  try {
    QAction * act = qobject_cast<QAction*>( sender() );
    if ( !act )
      throw "no action";

    QVariant v = act->data();
    if ( !v.canConvert<MessagePointer>() )
      throw "cannot convert";

    showMessage( v.value<MessagePointer>() );
  } catch ( const char * str ) {
    log( QString( "error in showMessage: " ) + str, Error );
  }
}


int MessageManager::receiveMessage( MessageInterface* /*msg*/ ) {
  return 0;
}

int MessageManager::receiveMessage( KDevTeamworkTextMessage* /*msg*/ ) {
  return 0;
}

int MessageManager::receiveMessage( InDocumentMessage* msg ) {
  return m_conversationManager->processMessage( msg );
}

int MessageManager::processMessage( KDevTeamworkTextMessage* msg ) {
  if ( !msg )
    return 0;
  m_historyManager->addMessage( msg );
  return dispatcher_( msg );
}

void MessageManager::log( const QString& str, LogLevel level ) {
  m_teamwork->log( "MessageManager: " + str, level );
}

KDevTeamwork* MessageManager::teamwork() {
  return m_teamwork;
}

MessageHistoryManager& MessageManager::historyManager() {
  return * m_historyManager;
}

ConversationManager& MessageManager::conversationManager() {
  return * m_conversationManager;
}

void MessageManager::showMessageHistory( const QList<KDevTeamworkUserPointer>& users, const QString& context ) {
  new GuiMessageHistory( this, users, context );
}

void MessageManager::fillMessageMenu( QMenu* menu, MessagePointer msg ) {
  QVariant v;
  v.setValue( msg );
  m_showMessageAction->setData( v );
  menu->addAction( m_showMessageAction );

  teamwork() ->fillMessageMenu( menu, msg );



  MessagePointer::Locked l = msg;
  if ( l ) {
    UserPointer u = l->info().user();
    if ( u ) {
      if ( l->info().isIncoming() && l.freeCast<AbstractGUIMessage>() ) {
        AbstractGUIMessage * request = l.freeCast<AbstractGUIMessage>();
        request->fillContextMenu( menu, teamwork() );
      }

      if ( !menu->children().isEmpty() )
        menu->addSeparator();

      if ( l->info().session() && l->info().session().unsafe() ->isOk() ) {
        QMenu * m = new QMenu( ~u.unsafe() ->safeName(), menu );
        menu->addMenu( m );
        m_teamwork->fillUserMenu( m, u );
      }
    } else
      log( "could not get user from message", Error );

  } else {
    if ( !l )
      log( "could not lock message", Error );
  }
}

void MessageManager::updateMessage( const MessagePointer& msg ) {
  emit messageUpdated( msg );
}

#include "messagemanager.moc"

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
