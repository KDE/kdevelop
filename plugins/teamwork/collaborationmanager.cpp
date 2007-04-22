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

#include "collaborationmanager.h"
#include  <QPersistentModelIndex>
#include  <QMenu>
#include <QMetaType>
#include "filecollaborationmanager.h"
#include "kdevteamwork_user.h"
#include "messagemanager.h"
#include "kdevteamwork_user.h"
#include <QModelIndex>
#include <QPoint>

Q_DECLARE_METATYPE( MessagePointer );

using namespace std;
typedef QPointer<FileCollaborationSession> QFileCollaborationSessionPointer;
Q_DECLARE_METATYPE( QPersistentModelIndex );
Q_DECLARE_METATYPE( QFileCollaborationSessionPointer );

CollaborationManager::CollaborationManager( KDevTeamwork* tw ) : SafeLogger( tw->logger() ), m_teamwork( tw ), m_fileCollaboration( this ), m_ignoreClick( false ) {

  m_contextMenuTimer = new QTimer( this );
  m_contextMenuTimer->setSingleShot( true );

  connect( m_contextMenuTimer, SIGNAL( timeout() ), this, SLOT( maybeContextMenu() ) );
  
  m_closeCollaborationAction = new QAction( "Stop Collaboration", this );
  connect( m_closeCollaborationAction, SIGNAL( triggered() ), this, SLOT( uiCloseCollaboration() ) );
  
  m_developersModel = new QStandardItemModel( 0, 1, this );

  qRegisterMetaType<MessagePointer>( "MessagePointer" );
  qRegisterMetaType<QFileCollaborationSessionPointer>( "QFileCollaborationSessionPointer" );

  m_acceptConnectionAction = new QAction( "Accept Collaboration", this );
  connect( m_acceptConnectionAction, SIGNAL( triggered() ), this, SLOT( uiAcceptCollaboration() ) );
  m_acceptConnectionAction->setToolTip( "Start a collaboration-session with the user." );

  m_denyConnectionAction = new QAction( "Refuse Collaboration", this );
  connect( m_denyConnectionAction, SIGNAL( triggered() ), this, SLOT( uiDenyCollaboration() ) );
  m_denyConnectionAction->setToolTip( "Refuse collaborating with the user." );

  m_requestCollaborationAction = new QAction( "Request Collaboration", this );
  connect( m_requestCollaborationAction, SIGNAL( triggered() ), this, SLOT( uiRequestCollaboration() ) );
  m_requestCollaborationAction->setToolTip( "Request a collaboration-session to the selected user." );

  connect( m_teamwork, SIGNAL(init()), this, SLOT(init() ) );
};

void CollaborationManager::init() {
  connect( m_teamwork->widgets().connectedDevelopers, SIGNAL( clicked( const QModelIndex& ) ), this, SLOT( collaboratingDeveloperClicked( const QModelIndex& ) ) );
  connect( m_teamwork->widgets().connectedDevelopers, SIGNAL( doubleClicked( const QModelIndex& ) ), this, SLOT( collaboratingDeveloperDoubleClicked( const QModelIndex& ) ) );

  connect( m_teamwork->widgets().connectedDevelopers, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT(collaboratingDeveloperContextMenu( const QPoint& ) ) );

  connect( m_teamwork, SIGNAL( updateConnection(TeamworkClientPointer) ), this, SLOT( updateConnection(const TeamworkClientPointer&) ) );
  m_closeCollaborationAction->setToolTip( "Stop collaborating with the selected developer." );
}

CollaborationManager::~CollaborationManager() {
}

void CollaborationManager::addCollaboratingUser( const UserPointer& u ) {
  if ( u )
    m_collaboratingUsers.insert( u );
  m_teamwork->startUpdateTimer();
}

void CollaborationManager::removeCollaboratingUser( const UserPointer& user ) {
  m_collaboratingUsers.erase( user );
  m_teamwork->startUpdateTimer();
}

bool CollaborationManager::isCollaborating( const UserPointer& user ) {
  return m_collaboratingUsers.find( user ) != m_collaboratingUsers.end();
}

void CollaborationManager::sessionStateChanged( const FileCollaborationSessionPointer& /*session*/ ) {
  updateList();
}

void CollaborationManager::updateList() {
  QStandardItemModel* model = m_developersModel;
  /*model->clear();
  model->insertColumn( 0 );*/
  
  ///find all users and sessions, and where they are stored in the tree-view
  QMap< UserPointer, QPersistentModelIndex > users;
  
  for( int r = 0; r < model->rowCount(); ++r ) {
    QModelIndex i = model->index( r, 0 );
    if( !i.isValid() ) continue;
    QVariant v = i.data( Qt::UserRole );

    if( v.canConvert<UserPointer>() ) {
      users[ v.value<UserPointer>() ] = QPersistentModelIndex( i );
    } else {
      for( int r2 = 0; r2 < model->rowCount( i ); ++r2 ) {
        QModelIndex i2 = model->index( r2, 0, i );
        if( !i2.isValid() ) continue;
        
        QVariant v = i.data( Qt::UserRole );

        if( v.canConvert<UserPointer>() )
          users[ v.value<UserPointer>() ] = QPersistentModelIndex( i2 );
      }
    }
  }

  ///Now remove all users that are not collaborating anymore
  for( QMap< UserPointer, QPersistentModelIndex >::iterator it = users.begin(); it != users.end(); ++it ) {
    if( m_collaboratingUsers.find( it.key() ) == m_collaboratingUsers.end() )
      model->removeRows( it->row(), 1, it->parent() );
  }

  ///Now add/update all users
  for ( UserSet::reverse_iterator it = m_collaboratingUsers.rbegin(); it != m_collaboratingUsers.rend(); ++it ) {
    //FileCollaborationSessionPointer session = m_fileCollaboration->sessions().value( (*it).cast<KDevTeamworkUser>() );
    
    KDevTeamworkUserPointer::Locked l = (*it).cast<KDevTeamworkUser>();

    QModelIndex iparent;//( sessions[session] );
    ///Insert the user
    QModelIndex i;
    if( users.contains( l.data() ) ) {
      i = users[l.data()];
    } else {
      model->insertRow( 0, iparent );
      i = model->index( 0, 0, iparent );
    }

    ///Update user-data
    if ( l  ) {
      QIcon icon = l->icon();
      model->setData( i, ~l->User::name(), Qt::DisplayRole );
      model->setData( i, icon, Qt::DecorationRole );
    } else {
      model->setData( i, QString( "not lockable" ), Qt::DisplayRole );
    }
    QVariant lv;
    lv.setValue( *it );

    model->setData( i, lv, Qt::UserRole );
  }

  emit updateModel( model );
}

void CollaborationManager::maybeContextMenu() {
  if( !m_ignoreClick && m_contextMenuIndex.get() && m_contextMenuIndex->isValid() ) {
    QModelIndex i = *m_contextMenuIndex;
    QPoint p = QCursor::pos();
    contextMenu( p, i );
  }

  m_ignoreClick = false;
  m_contextMenuIndex.reset( 0 );
}

void CollaborationManager::collaboratingDeveloperDoubleClicked( const QModelIndex& index ) {
  m_ignoreClick = true;
  QVariant v = index.model() ->data( index, Qt::UserRole );
  if ( v.canConvert<UserPointer>() ) {
    m_teamwork->guiSendMessageTo( v.value<UserPointer>() );
  } else if( v.canConvert<CollaborationTreeActionPointer>() ) {
    CollaborationTreeActionPointer p = v.value<CollaborationTreeActionPointer>();
    if( p ) {
      p->doubleClicked();
    }
  }
}



void CollaborationManager::collaboratingDeveloperClicked( const QModelIndex& index ) {
  if ( !index.isValid() )
    return ;
  QVariant v = index.model() ->data( index, Qt::UserRole );
  //if ( v.canConvert<UserPointer>() ) {
    m_contextMenuIndex.reset( new QPersistentModelIndex( index ) );
    m_contextMenuTimer->start( 250 );
  /*} else if( v.canConvert<CollaborationTreeActionPointer>() ) {
    CollaborationTreeActionPointer p = v.value<CollaborationTreeActionPointer>();
    if( p ) {
      p->clicked();
    }
  }*/

}

void CollaborationManager::contextMenu( const QPoint& p, const QModelIndex& index ) {
  
  if(!index.isValid() )
    return;
  QMenu menu;
  
  QVariant v = index.model() ->data( index, Qt::UserRole );
  if ( v.canConvert<UserPointer>() ) {
    m_teamwork->fillUserMenu( &menu, v.value<UserPointer>() );
    m_teamwork->getPatchesList( v.value<UserPointer>() );

    if( isCollaborating( v.value<UserPointer>() ) )
      emit fillCollaboratingUserMenu( &menu, v.value<UserPointer>() );
  
    m_closeCollaborationAction->setData( v );
    menu.addAction( m_closeCollaborationAction );
  } else if( v.canConvert<CollaborationTreeActionPointer>() ) {
    CollaborationTreeActionPointer p = v.value<CollaborationTreeActionPointer>();
    if( p ) {
      p->fillContextMenu( &menu );
    }
  }

  if( !menu.isEmpty() )
    menu.exec( p );
}

void CollaborationManager::collaboratingDeveloperContextMenu( const QPoint& p ) {

  QModelIndex index = m_teamwork->widgets().connectedDevelopers->indexAt( p );

  contextMenu( p, index );
}

void CollaborationManager::uiCloseCollaboration() {
  try {
    QAction * act = qobject_cast<QAction*>( sender() );
    if ( !act )
      throw "no action";

    QVariant v = act->data();
    if ( !v.canConvert<UserPointer>() )
      throw "cannot convert model-index";

    UserPointer::Locked lUser = v.value<UserPointer>();
    if( !lUser ) throw "failed to lock user";
    
    removeCollaboratingUser( lUser );

    m_teamwork->startUpdateTimer();
    if ( !m_teamwork->client() || !lUser->online().session() )
      throw "no physical collaboration";

    globalSendHelper.send<KDevSystemMessage>( lUser->online().session().getUnsafeData(), KDevSystemMessage::CollaborationClosed, "stopping collaboration" );
  }
  catch ( QString & str ) {
    err() << "in uiCloseCollaboration: " <<  str;
  } catch ( const char * str ) {
    err() << "in uiCloseCollaboration: " <<  str;
  }
}

void CollaborationManager::fillUserMenu( QMenu* menu, const UserPointer& user ) {
  if ( !isCollaborating( user ) ) {
    UserPointer::Locked l = user;
    if ( l && l->online() ) {
      QVariant v;
      v.setValue( user );
      m_requestCollaborationAction->setData( v );
      menu->addAction( m_requestCollaborationAction );
    }
  }
  //emit fillCollaboratingUserMenu( menu, user );
}

void CollaborationManager::updateConnection( const TeamworkClientPointer& cl ) {
  if( cl ) {
    connect( cl.getUnsafeData(), SIGNAL(signalDispatchMessage(CollaborationMessagePointer)), this, SLOT(processMessage(const CollaborationMessagePointer&)) );
  } else {
    m_developersModel->clear();
    m_developersModel->insertColumn( 0 );
  }
}

void CollaborationManager::processMessage( const CollaborationMessagePointer& msg ) {
  if( msg.cast< FileCollaborationMessage >() ) ///Here a dispatcher is not necessary yet
    m_fileCollaboration->processMessage( msg.cast< FileCollaborationMessage >() );
}

void CollaborationManager::restorePartialProjectSession( const QDomElement* el ) {}

void CollaborationManager::savePartialProjectSession( QDomElement* el ) {}

void setValueMessage( QVariant& v, const MessagePointer& msg );

void ConnectionRequest::fillContextMenu( QMenu* menu, KDevTeamwork* teamwork ) {
  if ( m_state == Waiting ) {
    QVariant v;
    setValueMessage( v, MessagePointer( this ) );
    teamwork->collaborationManager()->m_acceptConnectionAction->setData( v );
    menu->addAction( teamwork->collaborationManager()->m_acceptConnectionAction );
    teamwork->collaborationManager()->m_denyConnectionAction->setData( v );
    menu->addAction( teamwork->collaborationManager()->m_denyConnectionAction );
    menu->addSeparator();
  }
}

KDevTeamwork * CollaborationManager::teamwork() {
  return m_teamwork;
}

FileCollaborationManager& CollaborationManager::fileCollaborationManager() {
  return *m_fileCollaboration;
}

QStandardItemModel * CollaborationManager::developersModel( )
{
  return m_developersModel;
}

void CollaborationManager::uiAcceptCollaboration() {
  try {
    QAction * act = qobject_cast<QAction*>( sender() );
    if ( !act )
      throw "error";

    QVariant v = act->data();

    if ( !v.canConvert<MessagePointer>() )
      throw "cannot convert data";

    MessagePointer::Locked lMessage = v.value<MessagePointer>();

    if ( lMessage && lMessage.cast<ConnectionRequest>() ) {
      if ( lMessage->info().session() ) {
        lMessage.cast<ConnectionRequest>() ->setState( ConnectionRequest::Accepted );
        globalSendHelper.sendReply<KDevSystemMessage>( lMessage, KDevSystemMessage::CollaborationAccepted, "collaboration accepted" );
        if ( lMessage->info().user() )
          addCollaboratingUser( lMessage->info().user() );
        m_teamwork->messageManager()->updateMessage( lMessage );

        m_teamwork->startUpdateTimer();
      }
    } else {
      throw "could not lock message-info or client-class";
    }
  } catch ( const char * str ) {
    err() << "in uiAcceptCollaboration: " << str;
  }
}

void CollaborationManager::uiDenyCollaboration() {
  try {
    if ( !m_teamwork->client() )
      throw "client not running";

    QAction* act = qobject_cast<QAction*>( sender() );
    if ( !act )
      throw "error";

    QVariant v = act->data();

    if ( !v.canConvert<MessagePointer>() )
      throw "cannot convert data";

    MessagePointer::Locked lMessage = v.value<MessagePointer>();

    if ( lMessage && lMessage.cast<ConnectionRequest>() ) {
      if ( lMessage->info().session() ) {
        lMessage.cast<ConnectionRequest>() ->setState( ConnectionRequest::Accepted );
        globalSendHelper.sendReply<KDevSystemMessage>( lMessage, KDevSystemMessage::CollaborationRefused, "collaboration denied" );
        m_teamwork->messageManager()->updateMessage( lMessage );
      }
    } else {
      throw "could not lock message-info or client-class";
    }
  } catch ( const char * str ) {
    err() << "in uiDenyCollaboration: "  << str;
  }
}

void CollaborationManager::uiRequestCollaboration() {
  TeamworkClientPointer::Locked l = m_teamwork->client();

  try {
    if ( !l )
      throw "could not client while requesting connection" ;
    QAction* act = qobject_cast<QAction*>( sender() );
    if ( !act )
      throw "error in uiDenyCollaboration";

    QVariant v = act->data();

    if ( !v.canConvert<UserPointer>() )
      throw "error in uiDenyCollaboration, cannot convert data";

    UserPointer::Locked user = v.value<UserPointer>();
    UserPointer::Locked ident = l->identity();
    if ( user && ident ) {
      if ( user->online().session() ) {
        ConnectionRequest * msg = l->messageTypes().create<ConnectionRequest>( ident, user, "please let me connect", m_teamwork );
        user->online().session().getUnsafeData() ->sendMessage( msg );
        m_teamwork->addMessageToList( msg );
      } else {
        throw "no open session to the target-user";
      }
    } else {
      throw "could not lock target-user or local user";
    }
  } catch ( const char * str ) {
    err() << "uiRequestCollaboration: " << str;
  }
}


#include "collaborationmanager.moc"

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on


