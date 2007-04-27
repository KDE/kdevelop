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

#include "guimessagehistory.h"
#include "messagemanager.h"
#include <QMetaObject>
#include  <QStandardItemModel>
#include "messagehistorymanager.h"
#include <QTimer>
#include "kdevteamwork_client.h"
#include "kdevteamwork.h"
#include "utils.h"
#include <QMenu>
#include <QHeaderView>
#include "kdevteamwork_messageshower.h"

using namespace Teamwork;

Q_DECLARE_METATYPE( KDevTeamworkUserPointer );
Q_DECLARE_METATYPE( HistoryMessagePointer );
Q_DECLARE_METATYPE( Teamwork::MessageType );

GuiMessageHistory::GuiMessageHistory( MessageManager* manager, UserList users, QString context ) : SafeLogger( manager->teamwork() ->logger() ), m_manager( manager ), m_defaultContext( context ) {
  foreach( KDevTeamworkUserPointer user, users )
  addUser( user );

  if ( users.isEmpty() )
    m_allUsers = true;

  m_dialog = new QDialog( manager->teamwork() ->widget() );
  m_widgetData.setupUi( m_dialog );

  m_developersModel = new QStandardItemModel( 0, 1, m_widgetData.developers );
  m_widgetData.developers->setModel( m_developersModel );
  m_widgetData.developers->setSelectionMode( QAbstractItemView::MultiSelection );

  m_messagesModel = new QStandardItemModel( 0, 1, m_widgetData.messages );
  m_widgetData.messages->setModel( m_messagesModel );

  m_updateTimer = new QTimer( this );
  m_userIconUpdateTimer = new QTimer( this );
  m_updateTimer->setSingleShot( true );
  m_userIconUpdateTimer->setSingleShot( true );
  //m_widgetData.messages->header()->hide();


  qRegisterMetaType<KDevTeamworkUserPointer>( "KDevTeamworkUserPointer" );
  qRegisterMetaType<HistoryMessagePointer>( "HistoryMessagePointer" );
  qRegisterMetaType<Teamwork::MessageType>( "Teamwork::MessageType" );

  connect( m_widgetData.developers->selectionModel(), SIGNAL( selectionChanged ( const QItemSelection &, const QItemSelection & ) ), this, SLOT( slotSelectionChanged() ) );
  connect( m_widgetData.untilDate, SIGNAL( dateChanged ( const QDate & ) ), this, SLOT( slotSelectionChanged() ) );
  connect( m_widgetData.fromDate, SIGNAL( dateChanged ( const QDate & ) ), this, SLOT( slotSelectionChanged() ) );
  connect( m_widgetData.allMessagesButton, SIGNAL( clicked( bool ) ), this, SLOT( slotAllMessages() ) );
  connect( m_widgetData.allDevelopersButton, SIGNAL( clicked( bool ) ), this, SLOT( slotAllDevelopers() ) );
  connect( m_widgetData.typeFilter, SIGNAL( currentIndexChanged ( int ) ), this, SLOT( typeFilterChanged( int ) ) );
  connect( m_widgetData.contextFilter, SIGNAL( currentIndexChanged ( int ) ), this, SLOT( contextFilterChanged( int ) ) );

  connect( &m_manager->historyManager(), SIGNAL( newMessage( HistoryMessagePointer ) ), this, SLOT( slotNewMessage( const HistoryMessagePointer& ) ), Qt::QueuedConnection );
  connect( m_updateTimer, SIGNAL( timeout() ), this, SLOT( slotUpdateMessages() ) );
  connect( m_userIconUpdateTimer, SIGNAL( timeout() ), this, SLOT( updateMessageUserIcons() ) );
  connect( m_widgetData.exitButton, SIGNAL( clicked( bool ) ), this, SLOT( slotExit() ) );
  connect( m_dialog, SIGNAL( finished( int ) ), this, SLOT( slotExit() ) );
  connect( m_widgetData.messages, SIGNAL( clicked ( const QModelIndex & ) ), this, SLOT( messageItemClicked( const QModelIndex& ) ) );


  m_widgetData.messages->setAlternatingRowColors( true );
  m_widgetData.messages->setSelectionBehavior( QAbstractItemView::SelectItems );
  m_widgetData.messages->setRootIsDecorated( false );
  m_widgetData.messages->setIndentation( 0 );
  m_widgetData.messages->header() ->setResizeMode( QHeaderView::Stretch );

  slotAllMessages();
  fillDeveloperList();
  m_dialog->show();
}

GuiMessageHistory::~GuiMessageHistory() {
  delete m_dialog;
}

void GuiMessageHistory::showMessage( const HistoryMessagePointer& msg ) {
  HistoryMessagePointer::Locked l = msg;
  if ( l ) {
    if ( m_messagesModel->columnCount() == 0 ) {
      /*if( m_widget ) {
        m_widgetData.messages->header()->show();
      }*/
      m_messagesModel->insertColumns( 0, ColumnCount );

      m_messagesModel->setHeaderData( TimeColumn, Qt::Horizontal, QString( "Time" ) );
      m_messagesModel->setHeaderData( UserColumn, Qt::Horizontal, QString( "User" ) );
      m_messagesModel->setHeaderData( TypeColumn, Qt::Horizontal, QString( "Type" ) );
      m_messagesModel->setHeaderData( TextColumn, Qt::Horizontal, QString( "Text" ) );
    }
    m_messagesModel->insertRow( 0 );

    ///Time
    {
      UserPointer::Locked lu = l->info().user();
      QString userName;

      QModelIndex index = m_messagesModel->index( 0, TimeColumn );

      m_messagesModel->setData( index, l->creationTime().toString( Qt::LocalDate ), Qt::DisplayRole );
      //m_messagesModel->setData( index, l->messageIcon(), Qt::DecorationRole );

      QVariant v;
      v.setValue( msg );
      m_messagesModel->setData( index, v, Qt::UserRole );
    }

    ///Message-Type
    {
      UserPointer::Locked lu = l->info().user();
      QString userName;

      QModelIndex index = m_messagesModel->index( 0, TypeColumn );
      if ( lu ) {
        userName = ~lu->name();
      } else {
        userName = "unknown user";
      }

      m_messagesModel->setData( index, l->shortName(), Qt::DisplayRole );
      m_messagesModel->setData( index, l->messageIcon(), Qt::DecorationRole );

      QVariant v;
      v.setValue( msg );
      m_messagesModel->setData( index, v, Qt::UserRole );
    }

    ///User
    {
      UserPointer::Locked lu = l->info().user();
      QString userName;

      QModelIndex index = m_messagesModel->index( 0, UserColumn );
      if ( lu ) {
        userName = ~lu->name();
      } else {
        userName = "unknown";
      }

      m_messagesModel->setData( index, userName, Qt::DisplayRole );

      if ( lu.cast<KDevTeamworkUser>() )
        m_messagesModel->setData( index, lu.cast<KDevTeamworkUser>() ->icon(), Qt::DecorationRole );

      QVariant v;
      v.setValue( l->info().user() );
      m_messagesModel->setData( index, v, Qt::UserRole );
    }


    ///Text
    {
      QModelIndex index = m_messagesModel->index( 0, TextColumn );

      m_messagesModel->setData( index, ~l->text(), Qt::DisplayRole );

      QVariant v;
      v.setValue( msg );
      m_messagesModel->setData( index, v, Qt::UserRole );
    }
  } else {
    ///error
    err() << "could not lock Message in showMessage";
  }
}

void GuiMessageHistory::updateMessageUserIcons() {
  QMap<KDevTeamworkUserPointer, KDevTeamworkUserPointer::Locked> lockedUsers;
  foreach( KDevTeamworkUserPointer u, m_changedUsers ) {
    KDevTeamworkUserPointer::Locked l = u;
    lockedUsers[ u ] = l;
  }

  for ( int a = 0; a < m_messagesModel->rowCount(); a++ ) {
    QModelIndex i = m_messagesModel->index( a, UserColumn );
    if ( i.isValid() ) {
      QVariant v = m_messagesModel->data( i , Qt::UserRole );
      if ( v.canConvert<UserPointer>() ) {
        QMap<KDevTeamworkUserPointer, KDevTeamworkUserPointer::Locked>::iterator it = lockedUsers.find( v.value<UserPointer>().cast<KDevTeamworkUser>() );
        if ( it != lockedUsers.end() ) {
          if ( *it ) {
            m_messagesModel->setData( i, ( *it ) ->icon(), Qt::DecorationRole );
          } else {
            m_messagesModel->setData( i, QIcon(), Qt::DecorationRole );
          }
        }

      }
    }
  }
  m_changedUsers.clear();
}


void GuiMessageHistory::messageItemClicked( const QModelIndex& index ) {
  QVariant v = m_messagesModel->data( index, Qt::UserRole );

  if ( v.canConvert<UserPointer>() ) {
    KDevTeamworkUserPointer::Locked lu = v.value<UserPointer>().cast<KDevTeamworkUser>();
    if ( lu ) {
      QWidget * w = m_widgetData.messageFrame->widget();
      m_widgetData.messageFrame->setWidget( 0 );
      if ( w )
        delete w;

      w = new QWidget( m_widgetData.messageFrame );
      new KDevTeamworkUserInfoShower( lu, w, m_manager->teamwork() );
      m_widgetData.messageFrame->setWidget( w );

      /*QMenu* menu = new QMenu( m_dialog );
      m_manager->teamwork()->fillUserMenu( menu, lu.data() );
      menu->exec( QCursor::pos() );*/
    } else {
      err() << "could not lock user in messageItemClicked";
    }
  }

  if ( v.canConvert<HistoryMessagePointer>() ) {
    HistoryMessagePointer::Locked lmsg = v.value<HistoryMessagePointer>();
    if ( lmsg ) {
      QWidget * w = m_widgetData.messageFrame->widget();
      m_widgetData.messageFrame->setWidget( 0 );
      if ( w )
        delete w;

      AbstractGUIMessage* gmsg = lmsg.freeCast<AbstractGUIMessage>();
      if ( gmsg ) {
        w = new QWidget( m_widgetData.messageFrame );
        gmsg->showInWidget( w, m_manager->teamwork() );
        m_widgetData.messageFrame->setWidget( w );
      }

      /*QMenu* menu = new QMenu( m_dialog );
      m_manager->fillMessageMenu( menu, lmsg.data() );
      menu->exec( QCursor::pos() );*/
    } else {
      err() << "could not lock message in messageItemClicked";
    }
  }
}


bool GuiMessageHistory::fitMessageDate( const HistoryMessagePointer& msg ) {
  HistoryMessagePointer::Locked l = msg;

  if ( l ) {
    return m_widgetData.fromDate->date() <= l->creationTime().date() && m_widgetData.untilDate->date() >= l->creationTime().date();
  } else {
    err() << "could not lock Message in fitMessageDate";
    return false;
  }
}

void GuiMessageHistory::slotExit() {
  deleteLater();
}

void GuiMessageHistory::slotAllMessages() {
  m_widgetData.fromDate->setDate( QDate( 2000, 1, 1 ) );
  m_widgetData.untilDate->setDate( QDate::currentDate() );
  slotSelectionChanged();
}


void GuiMessageHistory::slotAllDevelopers() {
  if ( selectedUsers().isEmpty() ) {
    m_users.clear();
    m_allUsers = true;
  }

  m_widgetData.developers->selectionModel() ->clear();
  slotSelectionChanged();
}

void GuiMessageHistory::slotNewMessage( const HistoryMessagePointer& msg ) {
  HistoryMessagePointer::Locked l = msg;
  if ( l ) {
    UserPointer u = userFromSession( l->info().session() );
    if ( m_users.contains( u.cast<KDevTeamworkUser>() ) || m_allUsers ) {
      if ( fitMessageDate( msg ) ) {
        showMessage( msg );
        applyFilters( 1 );
      }
    }
  } else {
    m_manager->log( "GuiMessageHistory: could not lock a new message", Error );
  }
}

void GuiMessageHistory::typeFilterChanged( int /*index*/ ) {
  applyFilters();
}


void GuiMessageHistory::contextFilterChanged( int /*index*/ ) {
  applyFilters();
}


void GuiMessageHistory::clearFilters() {
  m_widgetData.typeFilter->clear();
  m_widgetData.contextFilter->clear();
}


void GuiMessageHistory::applyFilters( int firstN ) {
  QString type = m_widgetData.typeFilter->currentText();
  Teamwork::MessageType id;

  if ( m_widgetData.typeFilter->currentIndex() != -1 ) {
    QVariant v = m_widgetData.typeFilter->itemData( m_widgetData.typeFilter->currentIndex() );
    if ( v.canConvert<Teamwork::MessageType>() ) {
      id = v.value<Teamwork::MessageType>();
    }
  }
  QString context = m_widgetData.contextFilter->currentText();

  int count = m_messagesModel->rowCount();
  if ( firstN != 0 && firstN < count )
    count = firstN;

  for ( int a = 0; a < count; a++ ) {
    QModelIndex index = m_messagesModel->index( a, TypeColumn );
    bool typeFits = false;
    bool contextFits = false;

    QVariant v = m_messagesModel->data( index, Qt::UserRole );
    if ( HistoryMessagePointer::Locked lmsg = v.value<HistoryMessagePointer>() ) {
      if ( !type.isEmpty() ) {
        if ( lmsg->info().type().startsWith( id ) )
          typeFits = true;
      } else {
        typeFits = true;
      }

      if ( context.isEmpty() ) {
        contextFits = true;
      } else {
        InDocumentMessage* dmsg = lmsg.freeCast< InDocumentMessage >();
        if ( dmsg && dmsg->context() == context )
          contextFits = true;
      }
    } else {
      typeFits = true;
      contextFits = true;
    }

    m_widgetData.messages->setRowHidden( a, QModelIndex(), !typeFits || !contextFits );
  }
}


void GuiMessageHistory::slotSelectionChanged() {
  ///Update the list of messages according to the selected users
  m_updateTimer->start( 100 );
}

void GuiMessageHistory::slotUpdateMessages() {
  fillMessages();
}

void GuiMessageHistory::slotUserStateChanged( const KDevTeamworkUserPointer& user ) {
  m_changedUsers << user;
  updateUserIcon( user );
  m_userIconUpdateTimer->start( 100 );
}

void GuiMessageHistory::fillMessages() {
  QString oldTypeFilter = m_widgetData.typeFilter->currentText();
  QString oldContextFilter = m_widgetData.contextFilter->currentText();

  clearFilters();

  UserSet selected = selectedUsers();
  m_messagesModel->clear();
  if ( m_allUsers && selected.isEmpty() ) {
    m_developersModel->clear();
    clearUsers();
  }

  QMap<UserIdentity, bool> users;
  for ( UserSet::iterator it = selected.begin(); it != selected.end(); ++it ) {
    KDevTeamworkUserPointer::Locked l = it.key();
    if ( l ) {
      users[ l->identity() ] = true;
    } else {
      //error
      err() << "failed to lock a user on slotUserStateChanged";
    }
  }

  QMap<QString, bool> contexts;
  QMap<QString, Teamwork::MessageType> types;
  QMap<UserPointer, bool> grabbedUsers;

  QList<HistoryMessagePointer> messages = m_manager->historyManager().getMessages( m_manager->teamwork() ->client(), m_widgetData.fromDate->date(), m_widgetData.untilDate->date(), users );

  foreach( HistoryMessagePointer msg, messages ) {
    showMessage( msg );

    HistoryMessagePointer::Locked l = msg;
    if ( l ) {
      types[ QString( l->name() ) ] = l->info().type();
      if ( InDocumentMessage * dmsg = l.freeCast<InDocumentMessage>() ) {
        contexts[ dmsg->context() ] = true;
      }
    }

    if ( m_allUsers ) {
      HistoryMessagePointer::Locked l = msg;
      if ( msg ) {
        if ( l->info().user() ) {
          grabbedUsers[ l->info().user() ] = true;
        } else {
          err() << "a message has no associated user";
        }
      } else {
        err() << "a message cannot be locked";
      }
    }
  }

  if ( !grabbedUsers.isEmpty() ) {
    TeamworkClientPointer::Locked l = m_manager->teamwork() ->client();
    if ( l ) {
      for ( QMap<UserPointer, bool>::iterator it = grabbedUsers.begin(); it != grabbedUsers.end(); ++it ) {

        SafeSharedPtr<KDevTeamworkUser> user = it.key().cast<KDevTeamworkUser>();
        if ( user ) {
          addUser( user );
        } else {
          err() << "could not cast user to KDevTeamworkUSer";
        }
      }
    } else {
      err() << "could not lock client-session";
    }
  }

  if ( ( m_allUsers || !grabbedUsers.isEmpty() ) && selected.isEmpty() )
    fillDeveloperList();

  m_widgetData.typeFilter->insertItem( 0, "" );
  for ( QMap<QString, Teamwork::MessageType>::iterator it = types.begin(); it != types.end(); ++it ) {
    m_widgetData.typeFilter->insertItem( m_widgetData.typeFilter->count(), it.key() );
    QVariant v;
    v.setValue( *it );
    m_widgetData.typeFilter->setItemData( m_widgetData.typeFilter->count() - 1, v );
  }

  m_widgetData.contextFilter->insertItem( 0, m_defaultContext );
  if ( !m_defaultContext.isEmpty() )
    m_widgetData.contextFilter->insertItem( 1, "" );
  for ( QMap<QString, bool>::iterator it = contexts.begin(); it != contexts.end(); ++it ) {
    m_widgetData.contextFilter->insertItem( m_widgetData.contextFilter->count(), it.key() );
  }

  int i = m_widgetData.contextFilter->findText( oldContextFilter );
  if ( i != -1 )
    m_widgetData.contextFilter->setCurrentIndex( i );

  i = m_widgetData.typeFilter->findText( oldTypeFilter );
  if ( i != -1 )
    m_widgetData.contextFilter->setCurrentIndex( i );

  applyFilters();
}

GuiMessageHistory::UserSet GuiMessageHistory::selectedUsers() {
  QModelIndexList selected = m_widgetData.developers->selectionModel() ->selectedIndexes();
  if ( selected.isEmpty() ) {
    return m_users;
  } else {
    UserSet ret;
    foreach( QModelIndex index, selected ) {
      QVariant v = m_developersModel->data( index, Qt::UserRole );
      if ( v.canConvert<KDevTeamworkUserPointer>() )
        ret[ v.value<KDevTeamworkUserPointer>() ] = true;
    }
    return ret;
  }
}

void GuiMessageHistory::fillDeveloperList() {
  m_developersModel->clear();
  m_developersModel->insertColumn( 0 );
  for ( UserSet::iterator it = m_users.begin(); it != m_users.end(); ++it ) {
    m_developersModel->insertRow( 0 );
    QModelIndex index = m_developersModel->index( 0, 0 );

    KDevTeamworkUserPointer::Locked l = it.key();
    if ( l ) {
      m_developersModel->setData( index, ~l->User::name(), Qt::DisplayRole );

      QVariant v;
      v.setValue( it.key() );
      m_developersModel->setData( index, v, Qt::UserRole );

      m_developersModel->setData( index, l->icon(), Qt::DecorationRole );
    } else {
      m_developersModel->setData( index, QString( "lock_failed" ) );
    }
  }
}

void GuiMessageHistory::clearUsers() {
  for ( UserSet::iterator it = m_users.begin(); it != m_users.end(); ++it ) {
    disconnect( it.key().unsafe(), SIGNAL( userStateChanged( KDevTeamworkUserPointer ) ), this, SLOT( slotUserStateChanged( const KDevTeamworkUserPointer& ) ) );
  }
  m_users.clear();
}

void GuiMessageHistory::addUser( const KDevTeamworkUserPointer& user ) {
  m_users[ user ] = true;
  connect( user.unsafe(), SIGNAL( userStateChanged( KDevTeamworkUserPointer ) ), this, SLOT( slotUserStateChanged( const KDevTeamworkUserPointer& ) ), Qt::QueuedConnection );
}

void GuiMessageHistory::updateUserIcon( const KDevTeamworkUserPointer& user ) {
  KDevTeamworkUserPointer::Locked l = user;
  for ( int a = 0; a < m_developersModel->rowCount(); a++ ) {
    QModelIndex index = m_developersModel->index( a, 0 );
    if ( !index.isValid() )
      break;
    QVariant v = m_developersModel->data( index, Qt::UserRole );
    if ( v.canConvert<KDevTeamworkUserPointer>() ) {
      if ( user == v.value<KDevTeamworkUserPointer>() ) {
        m_developersModel->setData( index, l->icon() , Qt::DecorationRole );
      }
    }
  }
}

#include "guimessagehistory.moc"

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
