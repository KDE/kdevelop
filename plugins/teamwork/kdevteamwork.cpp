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

///@todo the whole inclusion-structure needs a big cleanup


#include "kdevteamwork.h"
#include <list>

#include <QtGui/QHeaderView>
#include <QAction>
#include <QTimer>
#include <QCursor>
#include <kdialog.h>
#include <QPersistentModelIndex>
#include <QWidget>

#include <kdebug.h>
#include <klocale.h>
#include <ktexteditor/document.h>
#include <kurl.h>

#include <icore.h>
#include <idocumentcontroller.h>
#include <idocument.h>

#include "network/messagetypeset.h"
#include "network/multisession.h"
#include "kdevteamwork_part.h"
#include "kdevteamwork_messageshower.h"
#include "kdevteamwork_user.h"
#include "kdevteamwork_client.h"
#include "teamworkfoldermanager.h"
#include "helpers.h"
#include "kdevteamwork_helpers.h"
#include "messageusertab.h"
#include "messagesendmanager.h"

///@todo Make most of these managers independent
#include "collaborationmanager.h"
#include "messagemanager.h"
#include "patchesmanager.h"

#include "ui_kdevteamwork_list.h"
#include "ui_kdevteamwork_interface.h"

using namespace KDevelop;

///@todo make sure fromUtf8(..) and toUtf8(..) is used everywhere, especially in all the messages

const int messageSendTimeout = 3000;

KDevTeamwork* KDevTeamwork::m_self = 0;

//Q_DECLARE_METATYPE( UserPointer )
Q_DECLARE_METATYPE( QPersistentModelIndex )
Q_DECLARE_METATYPE( MessagePointer )
Q_DECLARE_METATYPE( LocalPatchSourcePointer )
Q_DECLARE_METATYPE( KDevTeamworkUserPointer )


LaterDeleter::LaterDeleter( const KDevTeamworkClientPointer& c ) : m_c( c ) {
  deleteLater();
}

LaterDeleter::~LaterDeleter() {
}

std::string stringToAddr( const QString& txt );
int stringToPort( const QString& txt );

QIcon iconFromLevel( LogLevel level ) {
  switch ( level ) {
    case Error:
      return IconCache::getIconStatic( "remove", K3Icon::Small );
      break;
    case Info:
      return IconCache::getIconStatic( "ok", K3Icon::Small );
      break;
    case Warning:
      return IconCache::getIconStatic( "fileclose", K3Icon::Small );
      break;
    case Debug:
      return IconCache::getIconStatic( "log", K3Icon::Small );
      break;
    default:
      return IconCache::getIconStatic( "unknown", K3Icon::Small );
  }
}

///Get rid of this
void setValueMessage( QVariant& v, const MessagePointer& msg ) {
  v.setValue( msg );
}

PatchesManager* KDevTeamwork::patchesManager() {
  return m_patchesManager;
}

Ui::Teamwork& KDevTeamwork::widgets() {
  return *m_widgets;
}


QWidget* KDevTeamwork::widget() {
  return m_widget;
}

IconCache& KDevTeamwork::icons() {
  return * m_icons;
}


SafeTeamworkEmitter::SafeTeamworkEmitter( KDevTeamwork* tw ) {
  if ( !tw )
    return ;
  connect( this, SIGNAL( signalUpdateMessageInfo( MessagePointer ) ), tw, SLOT( updateMessageInfo( MessagePointer ) ), Qt::QueuedConnection );
}

template <class Type, class Helper>
void forEachInModel( QStandardItemModel* model, Helper& hlp, const QModelIndex& parent = QModelIndex() ) {
  for ( int a = 0; a < model->rowCount(); a++ ) {
    QModelIndex i = model->index( a, 0, parent );
    if ( i.isValid() ) {
      QVariant v = model->data( i, Qt::UserRole );
      if ( v.canConvert<Type>() ) {
        Type t = v.value<Type>();
        if ( !hlp( t, i, model ) )
          return ;
      } else {
        ///Some warning-message
      }
    }
  }
}

template <class Type>
struct NormalIdentCompare {
  bool operator() ( const Type& lhs, const Type& rhs ) {
    return lhs == rhs;
  }
};

template <class Type, class Compare>
struct FindInHelper {
  QModelIndex found;
  Compare compare;
  const Type& searching;
  FindInHelper( const Type& s ) : searching( s ) {}

  bool operator () ( const Type& t, QModelIndex i, QStandardItemModel* /*model*/ ) {
    if ( compare( t, searching ) ) {
      found = i;
      return false;
    }
    return true;
  }
};

template <class Type, class Compare >
QModelIndex findInModel( QStandardItemModel* model, const Type& t, const QModelIndex& /*parent*/ = QModelIndex() ) {

  FindInHelper<Type, Compare> h( t );

  forEachInModel<Type>( model, h );

  return h.found;
}

template <class Type >
QModelIndex findInModelNormal( QStandardItemModel* model, const Type& t, const QModelIndex& parent = QModelIndex() ) {
  return findInModel<Type, NormalIdentCompare<Type> > ( model, t, parent );
}

UserPointer KDevTeamwork::localUser() {
  ///@todo care about real local user
  return new User( "local" );
}

#define ADDFILTERMENUITEM(x)  { QAction *action = filterMenu->addAction( #x ); action->setData( x ); action->setCheckable( true ); connect( action, SIGNAL( triggered() ), this, SLOT( uiFilterLog() ) ); action->setChecked( m_currentLogFilter & x ); };

#define ADD_FILTER_MENU_ITEM(filterMenu, x, data, checked, slot)  { QAction *action = filterMenu->addAction( #x ); action->setData( data ); action->setCheckable( true ); connect( action, SIGNAL( triggered() ), this, SLOT( slot() ) ); action->setChecked( checked ); };

MessageManager* KDevTeamwork::messageManager() {
  return m_messageManager.data();
}


KDevTeamwork::KDevTeamwork( const KUrl& workspaceDirectory, KDevTeamworkPart *part, QWidget *parent ) :
m_folderManager( workspaceDirectory ),
m_logger( new KDevTeamworkLogger( this ) ),
m_destroyed( false ),
m_part( part ),
m_active( false ),
m_serverActive( false ),
m_patchesManager( this ),
m_collaborationManager( this ),
m_messageManager( this ),
m_messageSendManager( *m_widgets ),
m_currentLogFilter( ( LogLevel ) ( Error | Warning | Info ) )
{
  m_self = this;
  m_widget = parent;

  m_widgets->setupUi( parent );

  m_widgets->logFilter->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
  m_widgets->logFilter->setArrowType( Qt::DownArrow );
  m_widgets->logFilter->setPopupMode( QToolButton::InstantPopup );

  QMenu *filterMenu = new QMenu( i18n( "Filter" ), m_widgets->logFilter );

  ADDFILTERMENUITEM( Debug );
  ADDFILTERMENUITEM( Info );
  ADDFILTERMENUITEM( Warning );
  ADDFILTERMENUITEM( Error );

  m_widgets->logFilter->setMenu( filterMenu );

  m_widgets->answeringToButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
  m_widgets->answeringToButton->setArrowType( Qt::DownArrow );
  m_widgets->answeringToButton->setPopupMode( QToolButton::InstantPopup );

  /*m_messageFilterMenu = new QMenu( i18n( "Filter" ), m_widgets->messageFilter );

  m_widgets->messageFilter->setMenu( m_messageFilterMenu );*/

  qRegisterMetaType<QPersistentModelIndex> ( "QPersistentModelIndex" );
  qRegisterMetaType<LocalPatchSourcePointer> ( "LocalPatchSourcePointer" );
  qRegisterMetaType<MessagePointer> ( "MessagePointer" );
  qRegisterMetaType<UserPointer> ( "UserPointer" );


  m_logModel = new QStandardItemModel( 0, 1, m_widgets->logList );
  m_messagesModel = new QStandardItemModel( 0, 1, m_widgets->messageList );
  m_clientsModel = new QStandardItemModel( 0, 1, m_widgets->connectedClients );
  m_serversModel = new QStandardItemModel( 0, 1, m_widgets->connectedServers );
  m_developersModel = new QStandardItemModel( 0, 1, m_widgets->availableDevelopers );

  m_widgets->logList->setModel( m_logModel );
  m_widgets->connectedClients->setModel( m_clientsModel );
  m_widgets->connectedServers->setModel( m_serversModel );
  m_widgets->messageList->setModel( m_messagesModel );
  m_widgets->availableDevelopers->setModel( m_developersModel );
  m_widgets->connectedDevelopers->setModel( m_collaborationManager->developersModel() );

  m_widgets->availableDevelopers->setItemsExpandable( true );

  m_widgets->availableDevelopers->header() ->hide();
  m_widgets->connectedDevelopers->header() ->hide();

  m_updateTimer = new QTimer( this );
  m_updateTimer->setSingleShot( true );
  m_updateAnswerTimer = new QTimer( this );
  m_updateAnswerTimer->setSingleShot( true );

  m_replyWaitingTimeout = new QTimer( this );
  m_replyWaitingTimeout->setSingleShot( true );
  connect( m_replyWaitingTimeout, SIGNAL( timeout() ), this, SLOT( messageReplyTimeout() ) );
  connect( m_updateAnswerTimer, SIGNAL( timeout() ), this, SLOT( updateAnswerMenu() ) );

  connect( m_widgets->allowIncoming, SIGNAL( stateChanged( int ) ), this, SLOT( allowIncomingChanged( int ) ) );
  connect( m_widgets->enableCollaboration, SIGNAL( stateChanged( int ) ), this, SLOT( enableCollaborationChanged( int ) ) );
  connect( m_widgets->connectServer, SIGNAL( clicked() ), this, SLOT( connectServer() ) );
  connect( m_widgets->connectedServers, SIGNAL( clicked( const QModelIndex& ) ), this, SLOT( serverClicked( const QModelIndex& ) ) );
  connect( m_widgets->connectedServers, SIGNAL( doubleClicked( const QModelIndex& ) ), this, SLOT( serverDoubleClicked( const QModelIndex& ) ) );
  connect( m_widgets->availableDevelopers, SIGNAL( clicked( const QModelIndex& ) ), this, SLOT( developerClicked( const QModelIndex& ) ) );
  connect( m_widgets->availableDevelopers, SIGNAL( doubleClicked( const QModelIndex& ) ), this, SLOT( developerDoubleClicked( const QModelIndex& ) ) );
  connect( m_widgets->availableDevelopers, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( developerContextMenu( const QPoint& ) ) );
  connect( m_widgets->connectedClients, SIGNAL( clicked( const QModelIndex& ) ), this, SLOT( clientClicked( const QModelIndex& ) ) );
  connect( m_widgets->connectedClients, SIGNAL( doubleClicked( const QModelIndex& ) ), this, SLOT( clientDoubleClicked( const QModelIndex& ) ) );
  connect( m_widgets->messageList, SIGNAL( clicked( const QModelIndex& ) ), this, SLOT( messageClicked( const QModelIndex& ) ) );
  connect( m_widgets->messageList, SIGNAL( doubleClicked( const QModelIndex& ) ), this, SLOT( messageDoubleClicked( const QModelIndex& ) ) );
  connect( m_widgets->sendMessage, SIGNAL( clicked( ) ), this, SLOT( sendMessageButton( ) ) );
  connect( m_widgets->clearLog, SIGNAL( clicked( ) ), this, SLOT( clearLogButton( ) ) );
  connect( m_widgets->saveLog, SIGNAL( clicked( ) ), this, SLOT( saveLogButton( ) ) );
  connect( m_updateTimer, SIGNAL( timeout() ), this, SLOT( updateTimeout( ) ) );
  connect( m_widgets->messageType, SIGNAL( currentIndexChanged ( const int ) ), this, SLOT( messageTypeIndexChanged( const int ) ) );
  connect( m_widgets->clearMessages, SIGNAL( clicked() ), this, SLOT( slotClearMessages() ) );
  connect( m_widgets->messageHistory, SIGNAL( clicked() ), this, SLOT( slotMessageHistory() ) );
  connect( m_widgets->messageTargetUser, SIGNAL( editTextChanged( const QString& ) ), this, SLOT( messageTargetUserChanged() ) );
  connect( m_widgets->messageTargetUser, SIGNAL( currentIndexChanged( const QString& ) ), this, SLOT( messageTargetUserChanged() ) );
  connect( m_widgets->clearMessageButton, SIGNAL( clicked() ), this, SLOT( slotClearMessageText() ) );
  m_widgets->sendMessage->setDefault( true );

  m_widgets->messageType->clear();
  m_widgets->messageType->insertItem( 0, "Message" );
  m_widgets->messageType->insertItem( 1, "Source-Message" );

  sendMessageToUserAction = new QAction( "Send Message", this );
  connect( sendMessageToUserAction, SIGNAL( triggered() ), this, SLOT( sendMessageToUser() ) );
  sendMessageToUserAction->setToolTip( "Send a text-message to the selected user." );

  sendAnswerAction = new QAction( "Answer", this );
  connect( sendAnswerAction, SIGNAL( triggered() ), this, SLOT( sendAnswerMessage() ) );
  sendAnswerAction->setToolTip( "Send an answer to this message." );

  disconnectFromServerAction = new QAction( "Disconnect", this );
  connect( disconnectFromServerAction, SIGNAL( triggered() ), this, SLOT( disconnectFromServer() ) );
  disconnectFromServerAction ->setToolTip( "Disconnect from the selected server." );

  disconnectAllServersAction = new QAction( "Disconnect All", this );
  connect( disconnectAllServersAction, SIGNAL( triggered() ), this, SLOT( disconnectAllServers() ) );
  disconnectAllServersAction->setToolTip( "Disconnect from all connected servers." );

  kickClientAction = new QAction( "Kick", this );
  connect( kickClientAction, SIGNAL( triggered() ), this, SLOT( kickClient() ) );
  kickClientAction->setToolTip( "Kick the selected client." );

  banClientAction = new QAction( "Ban", this );
  connect( banClientAction, SIGNAL( triggered() ), this, SLOT( banClient() ) );
  banClientAction->setToolTip( "Ban the selected client(stop him from reconnecting)." );

  kickAllClientsAction = new QAction( "Disconnect All", this );
  connect( kickAllClientsAction, SIGNAL( triggered() ), this, SLOT( kickAllClients() ) );
  kickAllClientsAction->setToolTip( "Disconnect all connected clients from the local server." );

  m_userInfoAction = new QAction( "User Info", this );
  connect( m_userInfoAction, SIGNAL( triggered() ), this, SLOT( uiUserInfo() ) );
  m_userInfoAction->setToolTip( "Show information about the user." );

  showPatchesAction = new QAction( "Show Patches", this );
  connect( showPatchesAction, SIGNAL( triggered() ), this, SLOT( uiShowPatches() ) );
  showPatchesAction->setToolTip( "Show the available patches published by the selected user." );

  connect( m_messageManager.data(), SIGNAL( messageUpdated( const MessagePointer& ) ), this, SLOT( messageUpdated( const MessagePointer& ) ) );

  emit init();

  setActive( false );
  m_updateAnswerTimer->start( 1000 );
}

KDevTeamwork::~KDevTeamwork() {
  emit deInit();
  delete m_updateTimer;
  delete m_updateAnswerTimer;
  delete m_replyWaitingTimeout;

  if ( TeamworkClientPointer::Locked l = m_client ) {

    l->invalidateTeamwork();
  } else {
    ///error
    kDebug(9500) << "error while destruction of KDevTeamwork: could not lock client-pointer. Client cannot be destroyed.";
  }

  m_logger.lock() ->invalidate();
  new LaterDeleter( m_client );
  m_destroyed = true;
}

KDevTeamworkPart *KDevTeamwork::part() const {
  return m_part;
}

KDevTeamworkUserPointer KDevTeamwork::currentTabUser() {
  if ( !m_widget )
    return 0;

  QWidget* c = m_widgets->messageUsers->currentWidget();

  for ( QMap<KDevTeamworkUserPointer, SharedPtr<MessageUserTab> >::iterator it = m_userTabs.begin(); it != m_userTabs.end(); ++it ) {
    if ( ( *it ) ->widget() == c ) {
      return ( *it ) ->user();
    }
  }

  return 0;
}

void KDevTeamwork::slotClearMessageText() {
  m_widgets->messageText->clear();
}

void KDevTeamwork::slotClearMessages() {
  KDevTeamworkUserPointer current = currentTabUser();
  if ( !current ) {
    m_messagesModel->clear();
    m_messagesModel->insertColumn( 0 );
  } else {
    m_userTabs.remove( current );
  }
}

void KDevTeamwork::slotMessageHistory() {
  KDevTeamworkUserPointer current = currentTabUser();
  if ( !current ) {
    m_messageManager->showMessageHistory();
  } else {
    QList<KDevTeamworkUserPointer> lst;
    lst << current;
    m_messageManager->showMessageHistory( lst );
  }
}

bool KDevTeamwork::setActive( bool active ) {
  cout << globalMessageTypeSet().stats();
  if ( !m_active && active ) {
    log( "GUI: starting collaboration" );
    ///start the client
    m_client = new KDevTeamworkClient( this );
    {
      TeamworkClientPointer::Locked l = m_client;
      if ( l ) {
        l->insertUserSet( m_persistentUsers );
      } else {
        log( "could not lock fresh created KDevTeamworkClient", Error );
      }
    }
    m_client.unsafe() ->start();

    allowIncomingChanged( m_widgets->allowIncoming->checkState() );
  } else if ( m_client && !active ) {
    ///close the client
    log( "GUI: stopping collaboration" );
    {
      TeamworkClientPointer::Locked l = m_client;
      if ( l ) {
        l->getUserSet( m_persistentUsers );
        l->invalidateTeamwork();
      }
    }
    m_client = 0;
    m_serversModel->clear();
    m_serversModel->insertColumn( 0 );
    m_developersModel->clear();
    m_developersModel->insertColumn( 0 );
    m_clientsModel->clear();
    m_clientsModel->insertColumn( 0 );
    m_widgets->messageTargetUser->clear();
    m_actionMessage = 0;
  }
  m_active = active;

  emit updateConnection( m_client );

  enableActiveGUI( m_active );
  m_updateAnswerTimer->start();

  return m_active;
}

void KDevTeamwork::updateMessageInfo( MessagePointer msg ) {
  if( m_destroyed ) return;

  log( "updateMessageInfo", Debug );
  messageManager() ->updateMessage( msg );
}

bool KDevTeamwork::setServerActive( bool active ) {
  m_serverActive = active;
  return m_serverActive;
}

void KDevTeamwork::documentActivated( KDevelop::IDocument* file ) {
  Q_UNUSED( file );
}

void KDevTeamwork::popupContextMenu( const QPoint &pos ) {
  Q_UNUSED( pos );
}

void KDevTeamwork::allowIncomingChanged( int state ) {
  if ( TeamworkClientPointer::Locked l = m_client ) {
    switch ( state ) {
      case Qt::Checked:
      l->allowIncoming( true );
      break;
      case Qt::Unchecked:
      l->allowIncoming( false );
      break;
    }
  } else {
    err( "cannot lock client" );
  }
}

void KDevTeamwork::enableCollaborationChanged( int state ) {
  switch ( state ) {
    case Qt::Checked:
    setActive( true );
    break;
    case Qt::Unchecked:
    setActive( false );
    break;
  }
}

void KDevTeamwork::uiFilterLog() {
  QAction * action = qobject_cast<QAction*>( sender() );
  LogLevel level = ( LogLevel ) action->data().toInt();

  if ( action->isChecked() )
    m_currentLogFilter = ( LogLevel ) ( m_currentLogFilter | level );
  else
    m_currentLogFilter = ( LogLevel ) ( m_currentLogFilter ^ level );

  filterLog();
}

void KDevTeamwork::filterLog( int max ) {
  int count = m_logModel->rowCount();
  if ( max != 0 && count > max )
    count = max;
  for ( int a = 0; a < m_logModel->rowCount(); a++ ) {
    QModelIndex i = m_logModel->index( a, 0 );
    if ( i.isValid() ) {
      QVariant v = m_logModel->data( i , Qt::UserRole );
      LogLevel lv = ( LogLevel ) v.toInt();
      m_widgets->logList->setRowHidden( a, !( bool ) ( lv & m_currentLogFilter ) );
    }
  }
}

void KDevTeamwork::uiShowPatches() {
  QAction * act = qobject_cast<QAction*>( sender() );
  if ( !act )
    return ;

  QVariant v = act->data();
  if ( !v.canConvert<UserPointer>() ) {
    err( "uiShowPatches: Cannot convert to UserPointer" );
    return ;
  }
  UserPointer::Locked user = v.value<UserPointer>();
  if ( !user || ! user->online().session() ) {
    err( "failed to lock user in uiShowPatches or user is not online" );
    return ;
  }

  globalMessageSendHelper().send<PatchesManagerMessage>( user->online().session().unsafe(), PatchesManagerMessage::GetPatchesList );
}

void KDevTeamwork::showUserInfo( const UserPointer& user ) {
  try {
    if ( !user )
      throw QString( "no user given" );

    UserPointer::Locked l = user;
    if ( !l )
      throw QString( "could not lock user" );

    if ( !l.freeCast<KDevTeamworkUser>() )
      throw QString( "bad user-type" );

    KDialog* d = new KDialog( m_widget );
    d->setAttribute( Qt::WA_DeleteOnClose, true );
    KDevTeamworkUserInfoShower( l.freeCast<KDevTeamworkUser>(), d->mainWidget(), this );
    d->show();
  } catch ( QString str ) {
    log( "error in showUserInfo:" + str, Error );
  }

}

void KDevTeamwork::uiUserInfo() {
  try {
    UserPointer u = sendingUser();
    if ( !u )
      throw QString( "could not get sending-user" );

    showUserInfo( u );
  } catch ( QString str ) {
    log( "error in uiUserInfo:" + str, Error );
  }
}

void KDevTeamwork::connectionRequest( ConnectionRequestPointer request ) {
  log( "got collaboration-request" );
  addReceivedMessageToList( request );
}

void KDevTeamwork::connectServer() {
  if ( !m_active )
    return ;
  QString txt = m_widgets->serverBox->currentText();
  if ( !txt.isEmpty() ) {
    log( "UI: connecting to server " + txt );
    if ( m_client ) {
      TeamworkClientPointer::Locked l = m_client;
      if ( l ) {
        kDebug(9500) << "connecting to \"" << toQ( stringToAddr( txt ) ) << "\":" << stringToPort( txt );

        QString userName = m_widgets->loginName->text();

        QString password = m_widgets->loginPassword->text();

        UserPointer user;
        if ( !userName.isEmpty() )
          user = new User( userName.toUtf8().data(), password.toUtf8().data(), "a user" );
        else
          user = l->identity();
        if ( user ) {
          l->setIdentity( user );
          l->connectToServer( ServerInformation( stringToAddr( txt ), stringToPort( txt ) ), user );
        } else
          err( "cannot connect to a server without having an identity" );
      } else {
        err( "could not lock client-thread" );
      }
    } else {
      err( "error: client is not running" );
    }
  } else {
    err( "UI: cannot connect to unidentified server" );
  }
}

UserIdentity KDevTeamwork::currentUserIdentity() {
  TeamworkClientPointer::Locked l = m_client;;
  if( l ) {
    UserPointer u = l->identity();
    if( UserPointer::Locked lu = u ) {
      return lu->identity();
    }
  }
  return UserIdentity();
}


void KDevTeamwork::log( const QString & str, LogLevel level ) {
  ///@todo assign an icon for each log-level, maybe allow filtering by level, and add a "clear-log" button
  if( m_destroyed ) return;
  //if( level == Debug ) return;
  m_logModel->insertRow( 0 );
  QModelIndex i = m_logModel->index( 0, 0 );
  //m_logModel->insertColumn( 0, i );
  QIcon icon;
  ///choose an icon using the level
  icon = iconFromLevel( level );

  m_logModel->setData( i, str, Qt::DisplayRole );

  QVariant lv( level );
  m_logModel->setData( i, lv, Qt::UserRole );
  m_logModel->setData( i, icon, Qt::DecorationRole );

  /*if( ! (level & m_currentLogFilter) )
      m_widgets->logList->setRowHidden( 0 , true );*/

  filterLog( 3 );
  //kDebug(9500) << "log: \"" << str << "\" level:" << level;
}

void KDevTeamwork::err( const QString& str ) {
  if( m_destroyed ) return;
  log( str, Error );
}

///Starts the update-timer for the developer-lists
void KDevTeamwork::startUpdateTimer() {
  if( m_destroyed ) return;
  m_updateTimer->start( 300 );
}

TeamworkClientPointer& KDevTeamwork::client() {
  return m_client;
}
PatchesManager* KDevTeamwork::patchesManager();

CollaborationManager* KDevTeamwork::collaborationManager() {
  return m_collaborationManager;
}

LoggerPointer KDevTeamwork::logger() {
  return m_logger;
}

void KDevTeamwork::enableActiveGUI( bool active ) {
  m_widgets->connectionTab->setEnabled( active );
  m_widgets->availableLabel->setEnabled( active );
  m_widgets->connectedLabel->setEnabled( active );
  m_widgets->availableDevelopers->setEnabled( active );
  m_widgets->connectedDevelopers->setEnabled( active );

  enableMessageGUI( active );
  if ( !active ) {
    ///empty list of connected and available developers
  }
}

void KDevTeamwork::enableMessageGUI( bool active ) {
  m_widgets->messaging->setEnabled( active );
}

KDevTeamworkLogger::KDevTeamworkLogger( KDevTeamwork* tw ) : m_teamwork( tw ) {
  //    connect( this, SIGNAL( guiLog( QString, int ) ), m_teamwork, SLOT( guiLog( QString, int) ), Qt::QueuedConnection );
}

void KDevTeamworkLogger::log( const std::string& str , Level lv ) {
  if ( m_teamwork ) {
    LogLevel level;
    switch ( lv ) {
      case Info:
      level = ::Info;
      break;
      case Warning:
      level = ::Warning;
      break;
      case Error:
      level = ::Error;
      break;
      case Debug:
      level = ::Debug;
      break;
      default:
      return ;
    }

    kDebug(9500) << "log-level: \"" << ~levelToString( lv ) << "\" message:  \"" << toQ( str ) << "\"";
    QMetaObject::invokeMethod( m_teamwork, "guiLog", Qt::QueuedConnection, Q_ARG( QString, toQ( str ) ), Q_ARG( int, level ) );
  } else {
    kDebug(9500) << "log-level: \"" << ~levelToString( lv ) << "\" message:  \"" << toQ( str ) << "\"";
  }
}

void KDevTeamworkLogger::safeErrorLog( const std::string& str, Level lv ) {

  cout << levelToString( lv ) << "\" message:  \"" << toQ( str ) << "\"" << endl;
}

void KDevTeamwork::guiUserConnected( Teamwork::UserPointer user ) {
  startUpdateTimer();

  UserPointer::Locked l = user;
  if ( !l ) {
    err( "error" );
    return ;
  }
  QString name = toQ( l->name() );

  QStandardItemModel* model = m_clientsModel;
  model->insertRow( 0 );
  QModelIndex i = model->index( 0, 0 );

  model->setData( i, name, Qt::DisplayRole );
  QVariant v;
  v.setValue( user );
  model->setData( i, v, Qt::UserRole );

  addDeveloper( user );
}

void KDevTeamwork::guiUserDisconnected( Teamwork::UserPointer user ) {
  if( m_destroyed ) return;
  startUpdateTimer();

  UserPointer::Locked l = user;
  if ( !l ) {
    err( "failed to lock user-pointer" );
    return ;
  }

  QString name = toQ( l->name() );

  QStandardItemModel* model = m_clientsModel;
  for ( int a = 0; a < model->rowCount(); a++ ) {
    QModelIndex i = model->index( a, 0 );
    QVariant v = model->data( i, Qt::DisplayRole );
    if ( v.type() == QVariant::String ) {
      if ( v.toString() == name ) {
        model->removeRow( a );
      }
    }
  }

  ///remove the user from the target-user-list in the messaging-tab
  QVariant v;
  v.setValue( user );
  int i = m_widgets->messageTargetUser->findData( v );
  if ( i != -1 )
    m_widgets->messageTargetUser->removeItem( i );


  //kDebug(9500) << "disconnecting user is not in list:" << name;
}

void KDevTeamwork::guiServerConnected( Teamwork::ClientSessionDesc /*session*/, Teamwork::ServerInformation server ) {
  if( m_destroyed ) return;
  startUpdateTimer();

  QString desc = toQ( server.desc() );

  QStandardItemModel* model = m_serversModel;
  model->insertRow( 0 );
  QModelIndex i = model->index( 0, 0 );

  model->setData( i, desc, Qt::DisplayRole );
  QVariant v;
  v.setValue( server );
  model->setData( i, v, Qt::UserRole );
}

void KDevTeamwork::guiServerDisconnected( Teamwork::ClientSessionDesc /*session*/, Teamwork::ServerInformation server ) {
  if( m_destroyed ) return;
  startUpdateTimer();

  QStandardItemModel* model = m_serversModel;

  QString desc = toQ( server.desc() );

  for ( int a = 0; a < model->rowCount(); a++ ) {
    QModelIndex i = model->index( a, 0 );
    QVariant v = model->data( i, Qt::DisplayRole );
    if ( v.type() == QVariant::String ) {
      if ( v.toString() == desc ) {
        model->removeRow( a );
        return ;
      }
    }
  }

  kDebug(9500) << "disconnecting server is not in list:" << desc;
}

QIcon KDevTeamwork::iconFromUser( User* user, K3Icon::Group size ) {
  KDevTeamworkUserPointer::Locked u( dynamic_cast<KDevTeamworkUser*>( user ) );
  if ( !u )
    return m_icons->getIcon( "remove", size );
  return u->icon( size );
}


void KDevTeamwork::updateTimeout() {
  if( m_destroyed ) return;

  //log( "update-timer", Debug );
  QStandardItemModel * model = m_developersModel;

  for ( int a = model->rowCount() - 1; a >= 0; a-- ) {
    QModelIndex i = model->index( a, 0 );
    QVariant v = model->data( i, Qt::UserRole );
    //QVariant v2 = model->data( i, Qt::DecorationRole );
    QIcon icon;
    /*Ü if( v2.canConvert<QIcon>() )
     icon = v2.value<QIcon>();*/

    if ( v.canConvert<UserPointer>( ) ) {
      UserPointer p = v.value<UserPointer>();
      UserPointer::Locked l = p;
      if ( l ) {
        SessionPointer s = l->online().session();

        if ( s && s.unsafe() ->isOk() ) {
          icon = iconFromUser( l, K3Icon::Toolbar );
        } else {
          model->removeRow( a );
          continue;
        }

      } else {
        err( "user-pointer contained in developer-list could not be locked" );
        icon = m_icons->getIcon( "filecose", K3Icon::Toolbar );
      }
    } else {
      err( "wrong data in the developers-list" );
      icon = m_icons->getIcon( "filecose", K3Icon::Toolbar );
    }
    model->setData( i, icon, Qt::DecorationRole );
  }

  QComboBox* cb = m_widgets->messageTargetUser;

  for ( int a = cb->count(); a >= 0; a-- ) {
    QVariant v = m_widgets->messageTargetUser->itemData( a );
    if ( v.canConvert<UserPointer>() ) {
      UserPointer::Locked lu = v.value<UserPointer>();
      m_widgets->messageTargetUser->setItemData( a, iconFromUser( lu ), Qt::DecorationRole );
    }
  }

  m_collaborationManager->updateList();

  m_updateTimer->start( 3000 );
}

void KDevTeamwork::addDeveloper( const Teamwork::UserPointer& u ) {
  QStandardItemModel * model = m_developersModel;

  UserPointer::Locked l = u;
  if ( l ) {
    model->insertRow( 0 );
    QModelIndex i = model->index( 0, 0 );

    //log("adding user " + toQ( l->name() ) + " to list" );
    model->setData( i, toQ( l->name() ), Qt::DisplayRole );

    QVariant v;
    v.setValue<UserPointer>( u );
    model->setData( i, v, Qt::UserRole );
    model->setData( i, iconFromUser( l ), Qt::DecorationRole );

  } else {
    err( "could not lock a user-pointer from a received list" );
  }
}

void KDevTeamwork::registerPatches( PatchesListMessage* msg, QStandardItemModel* model ) {
  SessionPointer::Locked lsession = msg->info().session();
  log( "handing patches-list", Debug );

  try {
    if ( !lsession )
      throw "no session";
    UserPointer::Locked luser = lsession->safeUser();
    if ( !luser )
      throw "could not lock user, or wrong data in session";

    QModelIndex i = findInModelNormal( model, ( UserPointer ) luser );
    if ( i.isValid() ) {
      if ( model->columnCount( i ) == 0 )
        model->insertColumns( 0, 1, i );
      else
        model->removeRows( 0, model->rowCount( i ), i );

      for ( list<LocalPatchSource>::iterator it = msg->patches.begin(); it != msg->patches.end(); ++it ) {
        LocalPatchSourcePointer source = new LocalPatchSource( *it );
        source.unsafe() ->setUser( luser );
        QModelIndex ind = findInModel<LocalPatchSourcePointer, LocalPatchSourcePointer::ValueIdentCompare>( model, source, i );
        if ( ind.isValid() )
          model->removeRows( ind.row(), 1, i );

        model->insertRows( 0, 1, i );
        QModelIndex index = model->index( 0, 0, i );
        model->setData( index, ~( *it ).name );
        model->setData( index, ( *it ).getIcon( *m_icons ), Qt::DecorationRole );
        QVariant v;
        v.setValue<LocalPatchSourcePointer>( source );
        model->setData( index, v, Qt::UserRole );
      }
    } else {
      //throw "could not find the user in the developers-list";
    }
  } catch ( const char * str ) {
    err( QString( "in handlePatchesList: " ) + str );
  }
}

void KDevTeamwork::handlePatchesList( PatchesListMessage* msg ) {
  registerPatches( msg, m_developersModel );
  registerPatches( msg, m_collaborationManager->developersModel() );
}

void KDevTeamwork::guiUserList( std::list<UserPointer> users ) {

  //log( QString("guiUserList : handling user-list of size %1").arg( users.size() ) );

  for ( std::list<UserPointer>::iterator it = users.begin(); it != users.end(); ++it ) {
    addDeveloper( *it );
  }
}

void KDevTeamwork::fillUserMenu( QMenu* menu, const UserPointer& user ) {
  UserPointer::Locked luser = user;
  if ( luser && luser->online() ) {
    QVariant v;
    v.setValue( user );

    sendMessageToUserAction->setData( v );
    menu->addAction( sendMessageToUserAction );

    showPatchesAction->setData( v );
    //menu->addAction( showPatchesAction );
    m_collaborationManager->fillUserMenu( menu, user );
    m_userInfoAction->setData( v );
    menu->addAction( m_userInfoAction );
  }
}

void KDevTeamwork::fillMessageMenu( QMenu* menu, const MessagePointer& msg ) {
  MessagePointer::Locked l = msg;
  if ( l->info().session() && l->info().session().unsafe() ->isOk() && l->info().isIncoming() ) {
    QVariant v;
    v.setValue( msg );
    sendAnswerAction->setData( v );
    menu->addAction( sendAnswerAction );
  }
}

void KDevTeamwork::sendAnswerMessage() {
  MessagePointer::Locked msg = sendingMessage();
  if ( msg ) {
    guiSendMessageTo( msg->info().user(), msg );
  } else {
    log( "sendAnswerMessage: could not lock message to answer to ", Error );
  }
}

void KDevTeamwork::sendMessageToUser() {
  if ( sendingUser() )
    QMetaObject::invokeMethod( this, "guiSendMessageTo", Qt::QueuedConnection, Q_ARG( UserPointer, sendingUser() ) );
}


void KDevTeamwork::contextMenu( const QPoint& p, const QModelIndex& index ) {

  if(!index.isValid() )
    return;
  QMenu menu;

  QVariant v = index.model() ->data( index, Qt::UserRole );
  if ( v.canConvert<UserPointer>() ) {
    fillUserMenu( &menu, v.value<UserPointer>() );
    getPatchesList( v.value<UserPointer>() );

  }

  if ( v.canConvert<LocalPatchSourcePointer>() ) {
    m_patchesManager->fillDeveloperActions( index, &menu );
  }

  menu.exec( p );
}

void KDevTeamwork::maybeDeveloperContextMenu() {
  if ( m_contextMenuIndex.get() ) {
    contextMenu( QCursor::pos(), *m_contextMenuIndex );
  }
}

void KDevTeamwork::developerClicked( const QModelIndex& index ) {
  if ( !index.isValid() )
    return ;

  m_contextMenuIndex.reset( new QPersistentModelIndex( index ) );

  QTimer::singleShot( 400, this, SLOT( maybeDeveloperContextMenu() ) );
}

void KDevTeamwork::developerContextMenu( const QPoint& p ) {

  QModelIndex index = widgets().availableDevelopers->indexAt( p );
  if ( !index.isValid() )
    return ;

  contextMenu( p, index );
}

void KDevTeamwork::getPatchesList( const UserPointer& user ) {
  QVariant v;
  v.setValue( user );
  showPatchesAction->setData( v );
  showPatchesAction->activate( QAction::Trigger );
}

void KDevTeamwork::developerDoubleClicked( const QModelIndex& index ) {
  m_contextMenuIndex.reset( 0 );
  QVariant v = index.model() ->data( index, Qt::UserRole );
  if ( v.canConvert<UserPointer>() )
    QMetaObject::invokeMethod( this, "guiSendMessageTo", Qt::QueuedConnection, Q_ARG( UserPointer, v.value<UserPointer>() ) );
}

void KDevTeamwork::serverClicked( const QModelIndex& index ) {
  QMenu * menu = new QMenu( m_widget );

  if ( !index.isValid() )
    return ;
  TeamworkClientPointer::Locked l = m_client;
  if ( l ) {
    QVariant v = index.model() ->data( index, Qt::UserRole );
    if ( v.canConvert<ServerInformation>() ) {
      UserPointer user;
      ServerInformation p = v.value<ServerInformation>();
      MultiSessionPointer sess = l->sessionToServer( p );

      if ( sess )
        user = l->findSessionUser( sess );
      else
        log( "no session to the server is open", Warning );

      if ( !user )
        log( "the server has no associated user, no message can be sent", Warning );
      else
        fillUserMenu( menu, user );

    } else {
      err( "wrong information stored in the list-view" );
    }
  } else {
    err( "could not lock the client-handler" );
  }

  menu->addAction( disconnectFromServerAction );
  menu->addSeparator();
  menu->addAction( disconnectAllServersAction );
  menu->exec( QCursor::pos() );
}

void KDevTeamwork::serverDoubleClicked( const QModelIndex& index ) {
  TeamworkClientPointer::Locked l = m_client;
  if ( l ) {
    QVariant v = index.model() ->data( index, Qt::UserRole );
    if ( v.canConvert<ServerInformation>() ) {
      UserPointer user;
      ServerInformation p = v.value<ServerInformation>();
      MultiSessionPointer::Locked sess = l->sessionToServer( p );
      if ( sess )
        user = l->findSessionUser( sess.cast<SessionInterface>().data() );
      else
        log( "no session to the server is open", Warning );

      if ( !user )
        log( "the server has no associated user, no message can be sent", Warning );
      else
        QMetaObject::invokeMethod( this, "guiSendMessageTo", Qt::QueuedConnection, Q_ARG( UserPointer, user ) );
    }
  }
}

void KDevTeamwork:: messageClicked( const QModelIndex& index ) {
  if ( !index.isValid() )
    return ;
  QMenu menu( m_widget );

  UserPointer user;

  QVariant v = index.model() ->data( index, Qt::UserRole );
  if ( v.canConvert< MessagePointer >() ) {
    MessagePointer::Locked l = v.value< MessagePointer >();
    m_messageManager->fillMessageMenu( &menu, l );
  } else {
    err( "wrong information stored in the message-list-view" );
  }

  menu.exec( QCursor::pos() );
}

void KDevTeamwork::messageTargetUserChanged() {
  m_updateAnswerTimer->start( 100 );
}

void KDevTeamwork::updateAnswerMenu() {
  QMenu* menu = m_widgets->answeringToButton->menu();
  if( menu == 0 ) menu = new QMenu( i18n( "Answer To" ), m_widget );
  menu->clear();
  KDevTeamworkUserPointer::Locked user = currentMessageTargetUser().cast<KDevTeamworkUser>();
  if( !user ) return;

  UserTabMap::iterator it = m_userTabs.find( user );
  if( it != m_userTabs.end() ) {
    QStandardItemModel* model = (*it)->model();
    int maxSearch = model->rowCount();
    if( maxSearch > 100 ) maxSearch = 100;
    int count = 0;
    for( int a = 0 ; a < maxSearch; a++ ) {
      if( count > 10 ) break;
      QModelIndex i = model->index( a, 0 );
      if( i.isValid() ) {
        MessagePointer::Locked m = model->data( i, Qt::UserRole ).value<MessagePointer>();
        if( m ) {
          KDevTeamworkTextMessage* tm = m.freeCast<KDevTeamworkTextMessage>();
          AbstractGUIMessage* gm = m.freeCast<AbstractGUIMessage>();
          if( gm && tm && m->info().isIncoming() && (!tm->answered() || m.data() == m_answerTo.unsafe() ) ) {
            //gm->messageIcon()
            QAction* a = new QAction( gm->messageText().left( 30 )+"...", menu );
            QVariant v;
            v.setValue<MessagePointer>( m );
            a->setData( v );
            a->setCheckable( true );
            if( m.data() == m_answerTo.unsafe() ) a->setChecked( true );
            menu->addAction( a );
            connect( a, SIGNAL( toggled( bool ) ), this, SLOT( answerMenuToggled( bool ) ) );
            connect( a, SIGNAL( triggered( bool ) ), this, SLOT( answerMenuTriggered( bool ) ) );
            count++;
          }
        }
      }
    }
    m_widgets->answeringToButton->setEnabled( model->rowCount() != 0 );
  } else {
    m_widgets->answeringToButton->setEnabled( false );
  }
  m_widgets->answeringToButton->setMenu( menu );
}

void KDevTeamwork::answerMenuToggled( bool state ) {
  if( !m_widgets->answeringToButton->menu() ) return;
  QAction* sender = qobject_cast<QAction*>( QObject::sender() );
  QList<QAction*> actions = m_widgets->answeringToButton->menu()->findChildren<QAction*>();
  ///Uncheck all except his one
  for( QList<QAction*>::iterator it = actions.begin(); it != actions.end(); ++it ) {
    if( *it != sender ) {
      (*it)->setChecked( false );
    }
  }
  if( !state ) {
    m_answerTo = 0;
  } else {
    m_answerTo = sender->data().value<MessagePointer>();
  }
}

void KDevTeamwork::answerMenuTriggered( bool ) {
  MessagePointer m = sendingMessage();
  if( !m ) return;
  m_messageManager->showMessage( m );
}

void KDevTeamwork::messageDoubleClicked( const QModelIndex& index ) {
  if ( !index.isValid() )
    return ;

  UserPointer user;

  QVariant v = index.model() ->data( index, Qt::UserRole );
  if ( v.canConvert<MessagePointer>() ) {
    MessagePointer::Locked l = v.value< MessagePointer >();
    if ( l && l->info().session() ) {
      UserPointer u = userFromSession( l->info().session() );
      if ( u ) {
        QMetaObject::invokeMethod( this, "guiSendMessageTo", Qt::QueuedConnection, Q_ARG( UserPointer, u ) );
      } else
        log( "could not get user from message, or message has no session" );
    } else {
      err( "could not lock text-message from view" );
    }
  } else {
    err( "wrong information stored in the message-list-view" );
  }
}

void KDevTeamwork:: clientClicked( const QModelIndex& index ) {
  if ( !index.isValid() )
    return ;

  QMenu menu( m_widget );

  QVariant v = index.model() ->data( index, Qt::UserRole );
  if ( v.canConvert<UserPointer>() ) {
    fillUserMenu( &menu, v.value<UserPointer>() );
  } else {
    err( "wrong information stored in the list-view" );
  }

  menu.addSeparator();
  menu.addAction( kickClientAction );
  menu.addAction( banClientAction );
  menu.addSeparator();
  menu.addAction( kickAllClientsAction );
  menu.exec( QCursor::pos() );
}

void KDevTeamwork:: clientDoubleClicked( const QModelIndex& index ) {
  if ( !index.isValid() )
    return ;

  QVariant v = index.model() ->data( index, Qt::UserRole );
  if ( v.canConvert<UserPointer>() ) {
    QMetaObject::invokeMethod( this, "guiSendMessageTo", Qt::QueuedConnection, Q_ARG( UserPointer, v.value<UserPointer>() ) );
  } else {
    err( "wrong information stored in the list-view" );
  }
}

void KDevTeamwork::disconnectFromServer() {
  QModelIndex index = m_widgets->connectedServers->currentIndex();
  if ( !index.isValid() )
    return ;
  TeamworkClientPointer::Locked l = m_client;
  if ( l ) {
    QVariant v = index.model() ->data( index, Qt::UserRole );
    if ( v.canConvert<ServerInformation>() ) {
      ServerInformation p = v.value<ServerInformation>();
      l->disconnectFromServer( p );
    } else {
      err( "wrong information stored in the list-view" );
    }
  } else {
    err( "could not lock the client-handler" );
  }
}

void KDevTeamwork::disconnectAllServers() {
  TeamworkClientPointer::Locked l = m_client;
  if ( l ) {
    l->disconnectAllServers();
  } else {
    err( "could not lock the client-handler" );
  }
}

UserPointer KDevTeamwork::sendingUser() {
  QAction * act = qobject_cast<QAction*>( sender() );
  if ( !act )
    return 0;

  QVariant v = act->data();
  if ( !v.canConvert<UserPointer>() ) {
    return 0;
  }
  return v.value<UserPointer>();
}

MessagePointer KDevTeamwork::sendingMessage() {
  QAction * act = qobject_cast<QAction*>( sender() );
  if ( !act )
    return 0;

  QVariant v = act->data();
  if ( !v.canConvert<MessagePointer>() ) {
    return 0;
  }
  return v.value<MessagePointer>();
}

void KDevTeamwork::messageUpdated( const MessagePointer& msg ) {
  try {
    MessagePointer::Locked l = msg;
    if ( !l )
      throw QString( "could not lock a message to update" );

    KDevTeamworkUserPointer user = l->info().user().cast<KDevTeamworkUser>();

    if ( user ) {
      UserTabMap::iterator it = m_userTabs.find( user );
      if ( it != m_userTabs.end() ) {
        ( *it ) ->messageUpdated( msg );
      }
    }

    AbstractGUIMessage* guiMsg = l.freeCast<AbstractGUIMessage>();

    if ( !guiMsg )
      throw QString( "cannot update information for a non-gui-message" );

    QPersistentModelIndex pi = guiMsg->data.value<QPersistentModelIndex>();
    QModelIndex i( pi );

    if ( !i.isValid() )
      throw QString( "cannot update a message, the model-index is invalid" );

    QAbstractItemModel* model = const_cast<QAbstractItemModel*>( i.model() );
    model->setData( i, guiMsg->messageIcon(), Qt::DecorationRole );  ///The text is not updated right now
  } catch ( QString str ) {
    err( "in messageUpdated: " + str );
  }
}

void KDevTeamwork::kickClient() {
  QModelIndex index = m_widgets->connectedClients->currentIndex();
  if ( !index.isValid() )
    return ;
  TeamworkClientPointer::Locked l = m_client;
  if ( l ) {
    QVariant v = index.model() ->data( index, Qt::UserRole );
    if ( v.canConvert<UserPointer>() ) {
      UserPointer p = v.value<UserPointer>();
      UserPointer::Locked lu = p;
      if ( lu ) {
        if ( lu->online() ) {
          l->closeSession( lu->online().session() );
        } else {
          err( "user to kick is not online" );
        }
      } else {
        err( "could not lock user-pointer stored in the list-view" );
      }
    } else {
      err( "wrong information stored in the list-view" );
    }
  } else {
    err( "could not lock the client-handler" );
  }
}

void KDevTeamwork::banClient() {
  QModelIndex index = m_widgets->connectedClients->currentIndex();
  if ( !index.isValid() )
    return ;
  TeamworkClientPointer::Locked l = m_client;
  if ( l ) {
    QVariant v = index.model() ->data( index, Qt::UserRole );
    if ( v.canConvert<UserPointer>() ) {
      UserPointer p = v.value<UserPointer>();
      UserPointer::Locked lu = p;
      if ( lu ) {
        lu->ban( true );
        if ( lu->online() ) {
          l->closeSession( lu->online().session() );
        } else {
          err( "user to kick is not online" );
        }
      } else {
        err( "could not lock user-pointer stored in the list-view" );
      }
    } else {
      err( "wrong information stored in the list-view" );
    }
  } else {
    err( "could not lock the client-handler" );
  }
}

void KDevTeamwork::kickAllClients() {
  TeamworkClientPointer::Locked l = m_client;
  if ( l ) {
    l->closeAllIncomingSessions();
  } else {
    err( "could not lock the client-handler" );
  }
}

void KDevTeamwork::clearLogButton() {
  m_logModel->clear();
  m_logModel->insertColumn( 0 );
}

void KDevTeamwork::messageReplyTimeout() {
  log( "messageReplyTimeout", Debug );
  if( m_waitingForReply ) {
    addMessageToList( new FailureMessage( "got no reply", m_waitingForReply ) );
    unlockMessageGui();
  }
}

void KDevTeamwork::lockMessageGui( const MessagePointer& msg ) {
  //log( QString("lockMessageGui called, waiting for %1").arg( msg.unsafe() ), Debug );
  m_waitingForReply = msg;
  m_widgets->messageText->setEnabled( false );
  m_widgets->clearMessageButton->setEnabled( false );
  m_widgets->sendMessage->setEnabled( false );
  m_replyWaitingTimeout->start( messageSendTimeout );
}

void KDevTeamwork::unlockMessageGui() {
  log( "unlockMessageGui called", Debug );
  m_waitingForReply = 0;
  m_widgets->messageText->setEnabled( true );
  m_widgets->clearMessageButton->setEnabled( true );
  m_widgets->sendMessage->setEnabled( true );
  m_replyWaitingTimeout->stop();
}

void KDevTeamwork::receiveMessage( SafeSharedPtr<KDevSystemMessage> msg ) {
  SafeSharedPtr<KDevSystemMessage>::Locked l = msg;
  MessagePointer::Locked lv = m_waitingForReply;
  if ( l ) {
    if( m_waitingForReply == l->info().replyToMessage() ) {
      if( l->message() == KDevSystemMessage::ActionSuccessful ) {
        m_widgets->messageText->clear();
      } else {
        addMessageToList( new FailureMessage( QString( "could not send message: %1").arg( l->messageAsString() ), m_waitingForReply ) );
      }
      unlockMessageGui();
      LockedSharedPtr<KDevTeamworkTextMessage> la = m_answerTo.cast<KDevTeamworkTextMessage>();
      if( la ) {
        la->setAnswered( true );
      }
      m_answerTo = 0;
      m_updateAnswerTimer->start( 300 );
    }

    MessagePointer isReplyTo = l->info().replyToMessage();
    switch ( l->message() ) {
      case KDevSystemMessage::CollaborationAccepted: {
        if ( !isReplyTo ) {
          log( "got collaboration-accepted-message, but it was not requested", Warning );
          break;
        }
        startUpdateTimer();
        SafeSharedPtr<KDevSystemMessage>::Locked l = msg;
        log( "collaboration was accepted, reason: " + l->text() );
        if ( l ) {
          m_collaborationManager->addCollaboratingUser( userFromSession( l->info().session() ) );
        } else {
          log( "could not lock a KDevSystemMessage" );
        }
        break;
      }
      case KDevSystemMessage::CollaborationRefused:
      log( "collaboration was refused, reason: " + l->text() );
      break;
      case KDevSystemMessage::CollaborationClosed: {
        log( "peer stopped collaboration" );
        SafeSharedPtr<KDevSystemMessage>::Locked l = msg;
        if ( l ) {
          m_collaborationManager->removeCollaboratingUser( userFromSession( l->info().session() ) );
        }
      }
      break;
      default:
      break;
    }
  }
}

void KDevTeamwork::addSentMessageToList( const MessagePointer& smsg ) {
  ///Put the message into the "All Users"-tab
  addSentMessageToList( smsg, m_messagesModel );

  ///Also put the message into the user-tab
  MessagePointer::Locked msg = smsg;
  if ( msg ) {
    UserPointer::Locked lu = userFromSession( msg->info().session() );
    if ( lu ) {
      QMap<KDevTeamworkUserPointer, SharedPtr<MessageUserTab> >::iterator it = m_userTabs.find( lu.freeCast<KDevTeamworkUser>() );
      if ( it != m_userTabs.end() ) {
        addSentMessageToList( smsg, ( *it ) ->model() );
      } else {
        m_userTabs[ lu.freeCast<KDevTeamworkUser>() ] = new MessageUserTab( this, lu.freeCast<KDevTeamworkUser>() );
        addSentMessageToList( smsg, m_userTabs[ lu.freeCast<KDevTeamworkUser>() ] ->model() );
      }
    } else {
      log( "failed to lock user", Error );
    }
  } else {
    log( "failed to lock message", Error );
  }
}

void  KDevTeamwork::addMessageToList( const MessagePointer& smsg ) {
  MessagePointer::Locked l = smsg;
  if( l ) {
    if( l->info().isIncoming() ) {
      addReceivedMessageToList( smsg );
    } else {
      addSentMessageToList( smsg );
    }
  } else {
    err( "addMessageToList: could not lock message" );
  }
}

void KDevTeamwork::addSentMessageToList( const MessagePointer& smsg, QStandardItemModel* model ) {
  MessagePointer::Locked msg = smsg;
  if ( msg ) {

    AbstractGUIMessage * guiMsg = msg.freeCast<AbstractGUIMessage>();
    if ( !guiMsg ) {
      err( "cannot add non-gui-message to list" );
      return ;
    }
    model->insertRow( 0 );
    QModelIndex i = model->index( 0, 0 );

    if ( model == m_messagesModel )
      guiMsg->data.setValue( QPersistentModelIndex( i ) );

    QString toUser, text = guiMsg->messageText();

    UserPointer p = msg->info().user();
    if ( p ) {
      UserPointer::Locked lp = p;
      if ( lp ) {
        toUser = ~lp->name();
      } else {
        toUser = "not-lockable-user";
      }
    } else {
      toUser = "not-connected-user";
    }

    if ( model != m_messagesModel ) {
      text = text;
    } else {
      text = toUser + ": \"" + text + "\"";
    }

    model->setData( i, text, Qt::DisplayRole );
    model->setData( i, guiMsg->messageIcon() , Qt::DecorationRole );

    ///The  whole message is stored, including all its information. That may become a problem, since it keeps references to all involved sessions.

    QVariant v;
    v.setValue( smsg );
    model->setData( i, v, Qt::UserRole );

  } else {
    err( "could not lock incoming kdev-text-message" );
  }
  m_updateAnswerTimer->start( 300 );
}

void KDevTeamwork::addReceivedMessageToList( const MessagePointer& smsg ) {
  ///Put the message into the "All Users"-tab
  addReceivedMessageToList( smsg, m_messagesModel );

  ///Also put the message into the user-tab
  MessagePointer::Locked msg = smsg;
  if ( msg ) {
    UserPointer::Locked lu = userFromSession( msg->info().session() );
    if ( lu ) {
      QMap<KDevTeamworkUserPointer, SharedPtr<MessageUserTab> >::iterator it = m_userTabs.find( lu.freeCast<KDevTeamworkUser>() );
      if ( it != m_userTabs.end() ) {
        addReceivedMessageToList( smsg, ( *it ) ->model() );
      } else {
        m_userTabs[ lu.freeCast<KDevTeamworkUser>() ] = new MessageUserTab( this, lu.freeCast<KDevTeamworkUser>() );
        addReceivedMessageToList( smsg, m_userTabs[ lu.freeCast<KDevTeamworkUser>() ] ->model() );
      }
    } else {
      log( "failed to lock user", Error );
    }
  } else {
    log( "failed to lock message", Error );
  }
}

void KDevTeamwork::addReceivedMessageToList( const MessagePointer& smsg, QStandardItemModel* model ) {
  MessagePointer::Locked msg = smsg;
  if ( msg ) {
    AbstractGUIMessage * guiMsg = msg.freeCast<AbstractGUIMessage>();

    if ( !guiMsg ) {
      err( "cannot add non-gui-message to list" );
      return ;
    }

    model->insertRow( 0 );
    QModelIndex i = model->index( 0, 0 );

    if ( model == m_messagesModel )
      guiMsg->data.setValue( QPersistentModelIndex( i ) );

    QString fromUser, text = guiMsg->messageText();

    UserPointer p = msg->info().user();
    if ( p ) {
      UserPointer::Locked lp = p;
      if ( lp ) {
        fromUser = toQ( lp->name() );
      } else {
        fromUser = "not-lockable-user";
      }
    } else {
      fromUser = "missing-user-info";
    }

    if ( model != m_messagesModel ) {
      text = text;
    } else {
      text = fromUser + ": \"" + text + "\"";
    }

    model->setData( i, text, Qt::DisplayRole );
    model->setData( i, guiMsg->messageIcon() , Qt::DecorationRole );

    ///The  whole message is stored, including all its information. That may become a problem, since it keeps references to all sessions.

    QVariant v;
    v.setValue( smsg );
    model->setData( i, v, Qt::UserRole );

  } else {
    err( "could not lock incoming kdev-text-message" );
  }
  m_updateAnswerTimer->start( 300 );
}

void KDevTeamwork::saveLogButton() {
  ///open a save-dialog and store the content of logModel to a file
  log( "not implemented yet", Warning );
}

void KDevTeamwork::guiSendMessageTo( const UserPointer& user, const MessagePointer& answerTo ) {
  UserPointer::Locked lu = user;
  if ( lu ) {
    m_answerTo = answerTo;

    m_widgets->toolBox->setCurrentWidget( m_widgets->messaging );
    int item = -1;
    QVariant v;
    for ( int a = 0; a < m_widgets->messageTargetUser->count(); a++ ) {
      v = m_widgets->messageTargetUser->itemData( a );
      if ( v.canConvert<UserPointer>() ) {
        if ( v.value<UserPointer>() == user ) {
          item = a;
          break;
        }
      }
    }
    v.setValue( user );
    if ( item == -1 ) {
      m_widgets->messageTargetUser->insertItem( 0, toQ( lu->name() ), v );
      m_widgets->messageTargetUser->setItemData( 0, iconFromUser( lu ), Qt::DecorationRole );
      item = 0;
    } else {}
    m_widgets->messageTargetUser->setCurrentIndex( item );

    m_widgets->messageType->setCurrentIndex( NormalMessage );

    if ( LockedSharedPtr<InDocumentMessage> l = answerTo.cast<InDocumentMessage>() ) {
      m_widgets->messageType->setCurrentIndex( DocumentMessage );
      m_widgets->context->setText( l->context() );
      m_widgets->reference->clear();
    }
    m_widgets->messageText->clear();
  } else {
    err( "could not lock user-pointer stored in the list-view" );
  }

  startUpdateTimer();
}

void KDevTeamwork::messageTypeIndexChanged( const int index ) {
  if( m_destroyed ) return;

  switch ( index ) {
    case Message:
    m_widgets->reference->hide();
    m_widgets->referenceLabel->hide();
    m_widgets->contextLabel->hide();
    m_widgets->context->hide();
    break;
    case SourceMessage:
    m_widgets->reference->show();
    m_widgets->referenceLabel->show();
    m_widgets->contextLabel->show();
    m_widgets->context->show();
    break;
  };
}

UserPointer KDevTeamwork::currentMessageTargetUser() {
  UserPointer user;
  if ( m_widgets->messageTargetUser->currentIndex() != -1 ) {
    QVariant v = m_widgets->messageTargetUser->itemData( m_widgets->messageTargetUser->currentIndex() );
    if ( v.canConvert<UserPointer>() )
      user = v.value<UserPointer>();
  }
  QString uname = m_widgets->messageTargetUser->currentText();
  if ( ( !user && uname.isEmpty() ) )
    return 0;
  if ( !user ) {
    TeamworkClientPointer::Locked l = m_client;
    if( !l )
      return 0;
    else
      return l->findUser( UserIdentity( uname.toUtf8().data() ) );
  }

  return user;
}

void KDevTeamwork::sendMessageButton() {
  //QString txt = m_widgets->messageText->toHtml();
  QString txt = m_widgets->messageText->toPlainText();

  MessagePointer::Locked lmsg = m_answerTo;

  if( txt.isEmpty() ) return;

  UserPointer user = currentMessageTargetUser();
  if( !user ) {
    log( "sendMessageButton(): target-user not available", Error );
  }

  if ( lmsg ) {
    if ( !( lmsg->info().user() == user ) )
      m_answerTo = 0;
  }

  TeamworkClientPointer::Locked l = m_client;

  if ( l ) {
    UserPointer::Locked ul = user;
    MessagePointer::Locked msg;
    if ( user && ul && ul->online() ) {
      switch ( m_widgets->messageType->currentIndex() ) {
        case Message: {
          msg = new KDevTeamworkTextMessage( l->messageTypes(), txt );
          break;
        }
        case SourceMessage: {
          QString docText;
          InDocumentReference ref( true, m_widgets->reference->text() );
          InDocumentReference endRef( false, m_widgets->reference->text() );
          KUrl docUrl = TeamworkFolderManager::workspaceAbsolute( ref.document() );

          IDocument* doc = KDevTeamworkPart::staticCore()->documentController()->documentForUrl( docUrl );

          if ( doc && doc->textDocument() ) {
            docText = doc->textDocument() ->text();
            ref.useText( docText );
            if( endRef )
              endRef.useText( docText );
          } else {
            log( "the document " + docUrl.url() + " is not open, weak-reference can not be computed. A simple line- and column-reference is created", Warning );
          }
          log( QString("created reference for: (%1, %2) : (%3, %4)").arg( ref.line() ).arg( ref.col() ).arg( endRef.line() ).arg( endRef.col() ), Debug );

          msg = new InDocumentMessage( l->messageTypes(), txt, ref, endRef, m_widgets->context->text() );

          InDocumentMessage* dmsg = msg.freeCast<InDocumentMessage>();
          if ( dmsg ) {
            msg.freeCast<InDocumentMessage>() ->contextLines() = DocumentContextLines( dmsg->start(), dmsg->end(), docText );
          } else {
            log( "error: could not build InDocumentMessage", Error );
          }
          break;
        }
      }
      if ( msg ) {
        if ( m_answerTo ) {
          MessagePointer::Locked la = m_answerTo;
          if ( la ) {
            msg->info().setReply( la->info().uniqueId() );
            msg->info().setReplyMessage( m_answerTo );
          }
        }

        ul->online().session().unsafe() ->send( msg );
        addSentMessageToList( msg );
        switch ( m_widgets->messageType->currentIndex() ) {
          case SourceMessage: {}
        }

        lockMessageGui( msg );
        m_messageManager->processMessage( msg.cast<KDevTeamworkTextMessage>() );
      } else {
        err( "could not create the message for sending" );
      }

    } else {
      if ( user && !ul )
        err( "could not lock the user" );
      if ( ul && !ul->online() )
        err( "no open session to user " + ~ul->name() );
      if ( !user )
        err( "target-user not existing" );
      err( "sending the message failed" );
    }
  } else {
    err( "could not lock the client-handler while sending message" );
  }
}


void KDevTeamwork::handleTextMessage( SafeSharedPtr<KDevTeamworkTextMessage> smsg ) {
  LockedSharedPtr<KDevTeamworkTextMessage> msg = smsg;
  if ( msg ) {
    if ( !m_messageManager->processMessage( msg ) )
      addReceivedMessageToList( smsg );
    globalMessageSendHelper().sendReply<KDevSystemMessage>( msg, KDevSystemMessage::ActionSuccessful );
  } else {
    err( "could not lock incoming kdev-text-message" );
  }
}

void KDevTeamwork::restorePartialProjectSession( const QDomElement* el ) {
  m_patchesManager->restorePartialProjectSession( el );
  m_collaborationManager->restorePartialProjectSession( el );
  m_messageManager->restorePartialProjectSession( el );
}

void KDevTeamwork::savePartialProjectSession( QDomElement* el ) {
  m_patchesManager->savePartialProjectSession( el );
  m_collaborationManager->savePartialProjectSession( el );
  m_messageManager->savePartialProjectSession( el );
}

KDevTeamwork* KDevTeamwork::self() {
  return m_self;
}

#include "kdevteamwork.moc"

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
