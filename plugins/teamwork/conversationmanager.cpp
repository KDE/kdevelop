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

#include <boost/archive/polymorphic_xml_oarchive.hpp>
#include <boost/archive/polymorphic_xml_iarchive.hpp>
#include <QPalette>
#include <QColor>
#include <QPoint>
#include <QMenu>
#include <QHeaderView>
#include  "messagemanager.h"
#include "conversationmanager.h"
#include "kdevteamwork_messages.h"
#include "idocumentcontroller.h"
#include "idocument.h"
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <ktexteditor/cursor.h>
#include "kdevteamwork.h"
#include <ktexteditor/view.h>
#include <ktexteditor/cursor.h>
#include <QStandardItemModel>
#include "kdevteamwork_user.h"
#include "kdevteamwork_client.h"
#include <boost/archive/text_iarchive.hpp>
#include <ktexteditor/view.h>
#include <ktexteditor/markinterface.h>
#include <ktexteditor/highlightinginterface.h>
#include <ktexteditor/attribute.h>
#include <QBrush>
#include "kdevteamwork_helpers.h"
#include "teamworkfoldermanager.h"
#include "crossmap.h"

using namespace KDevelop;
    
Q_DECLARE_METATYPE( MessagePointer );
Q_DECLARE_METATYPE( InDocumentMessagePointer );

CROSSMAP_KEY_EXTRACTOR( OrderedDocumentMessage, int, 0, value.position );
CROSSMAP_KEY_EXTRACTOR( OrderedDocumentMessage, InDocumentMessagePointer, 0, value.message );
CROSSMAP_KEY_EXTRACTOR( OrderedDocumentMessage, QString, 0, value.message.lock()->start().document() );

template <class ArchType>
void InDocumentConversation::load( ArchType& arch, unsigned int version ) {
  m_messageCount = 0;
  serial( arch, version );
  std::list<Teamwork::UniqueMessageId> ids;
  arch & ids;
  log( QString( "loading %1 ids" ).arg( ids.size() ), Debug );
  m_messages.clear();
  for ( std::list<Teamwork::UniqueMessageId>::iterator it = ids.begin(); it != ids.end(); ++it ) {
    InDocumentMessagePointer msg = ConversationManager::instance() ->manager() ->historyManager().getMessageFromId( *it, ConversationManager::instance() ->manager() ->teamwork() ->client() );
    if ( msg )
      pushMessage( msg );
    else
      log( QString( "Could not get the message with id %1 from the history" ).arg( *it ), Warning );
  }
}

KTextEditor::Cursor InDocumentConversation::currentDocCursor() const {
  if( !m_currentConnectedDocument || !m_currentConnectedDocument->activeView() ) return KTextEditor::Cursor();
  return m_currentConnectedDocument->activeView()->cursorPosition();
}

void InDocumentConversation::pushMessage( const InDocumentMessagePointer& msg ) {
  m_messages.insert( OrderedDocumentMessage( m_messageCount++, msg ) );
}

template <class ArchType>
void InDocumentConversation::save( ArchType& arch, unsigned int version ) const {
  const_cast<InDocumentConversation*>( this ) ->serial( arch, version );

  std::list<Teamwork::UniqueMessageId> ids;

  ;
  for ( MessageSet::Iterator it = m_messages.orderedValues<int>(); it; ++it ) {
    InDocumentMessagePointer::Locked l = it->message;
    if ( l ) {
      ids.push_back( l->info().id().uniqueId() );
    } else {
      log( "could not lock InDocumentMessage for getting id", Error );
    }
  }

  log( QString( "saving %1 ids" ).arg( ids.size() ), Debug );
  arch & ids;
}

template void InDocumentConversation::load( InArchive& arch, unsigned int version )
;
template void InDocumentConversation::save( OutArchive& arch, unsigned int version )
const;

///This helper-class helps tracking connected user-signals and cleanly disconnecting them once they are not needed anymore
struct TemporaryConversationConnector : public Shared {
  KDevTeamworkUserPointer user;
  InDocumentConversation* target;
  TemporaryConversationConnector() : target( 0 ) {}
  TemporaryConversationConnector( const KDevTeamworkUserPointer& u, InDocumentConversation* t ) : user( u ), target( t ) {
    if ( user && target ) {
      KDevTeamworkUserPointer::Locked l = user;
      if ( l ) {
        QObject::connect( l, SIGNAL( userStateChanged( KDevTeamworkUserPointer ) ), target, SLOT( userStateChanged( KDevTeamworkUserPointer ) ), Qt::QueuedConnection );
      }
    }
  }
  ~TemporaryConversationConnector() {
    cleanup();
  }
  private:

  void cleanup() {
    if ( user && target ) {
      KDevTeamworkUserPointer::Locked l = user;
      if ( l ) {
        QObject::disconnect( l, SIGNAL( userStateChanged( KDevTeamworkUserPointer ) ), target, SLOT( userStateChanged( KDevTeamworkUserPointer ) ) );
      }
    }
    user = 0;
    target = 0;
  }

  void invalidate() {
    user = 0;
    target = 0;
  }

  TemporaryConversationConnector ( const TemporaryConversationConnector& /*rhs*/ ) {}
}
;

using namespace std;
using namespace Teamwork;

ConversationManager* ConversationManager::m_instance;

ConversationManager::ConversationManager( MessageManager* mng ) {
  m_manager = mng;
  m_instance = this;
  IDocumentController* docControl = KDevTeamwork::documentController();
  if ( docControl ) {
    connect( docControl, SIGNAL( documentActivated( KDevelop::IDocument* ) ), this, SLOT( documentActivated( KDevelop::IDocument* ) ) );
    connect( docControl, SIGNAL( documentLoaded( KDevelop::IDocument* ) ), this, SLOT( documentActivated( KDevelop::IDocument* ) ) );
  }
  connect( mng->teamwork(), SIGNAL( init() ), this, SLOT( load() ) );
  connect( mng->teamwork(), SIGNAL( deInit() ), this, SLOT( save() ) );
}

ConversationManager::~ConversationManager() {
}

InDocumentConversationPointer ConversationManager::findConversation( QString context ) {
  ConversationSet::iterator it = m_conversations.find( ~context );

  if ( it != m_conversations.end() ) {
    return ( *it ).second;
  } else {
    return 0;
  }
}

InDocumentConversationPointer ConversationManager::getConversation( InDocumentMessage* msg ) {
  SharedPtr<InDocumentConversation> ret = findConversation( msg->context() );
  if ( ret ) {
    return ret;
  } else {
    m_conversations[ ~msg->context() ] = new InDocumentConversation( msg );
    return m_conversations[ ~msg->context() ];
  }
}

int ConversationManager::processMessage( InDocumentMessage* msg ) {
  getConversation( msg ) ->addMessage( msg );

  return 0;
}

void ConversationManager::save() {
  try {
    QString fileName = TeamworkFolderManager::absolute( "conversation.database" );
    std::ofstream file(fileName.toLatin1(), ios_base::out | ios_base::binary );
    if( !file.good() ) throw "could not open " + fileName + " for writing";
    boost::archive::polymorphic_xml_oarchive arch( file );
    arch << NVP(m_conversations);
  } catch ( std::exception & exc ) {
    log( QString("save(): exception occured while serialization: %1").arg( exc.what() ), Error );
  } catch( const char* str ) {
    log( QString("save(): %1").arg( str ), Error );
  } catch( const QString& str ) {
    log( QString( "save(): %1").arg( str ), Error );
  }
}

void ConversationManager::load() {
  try {
    QString fileName = TeamworkFolderManager::absolute( "conversation.database" );
    std::ifstream file(fileName.toLatin1(), ios_base::binary );
    if( !file.good() ) throw "could not open " + fileName + " for reading";
    boost::archive::polymorphic_xml_iarchive arch( file );
    arch >> NVP(m_conversations);
  } catch ( std::exception & exc ) {
    log( QString("load(): exception occured while serialization: %1").arg( exc.what() ), Error );
  } catch( const char* str ) {
    log( QString("load(): %1").arg( str ), Error );
  } catch( const QString& str ) {
    log( QString( "load(): %1").arg( str ), Error );
  }
}

void ConversationManager::documentActivated( IDocument* document ) {
  //document->
  /*for( ConversationSet::iterator it = m_conversations.begin(); it != m_conversations.end(); ++it ) {
    (*it)->documentActivated( document );
  }*/
}

void ConversationManager::log( QString str, LogLevel level ) {
  m_manager->log( str, level );
};

ConversationManager* InDocumentConversation::manager() const {
  return ConversationManager::instance();
}

std::string InDocumentConversation::logPrefix() {
  return "InDocumentConversation(" + ~context() + "): ";
}

void InDocumentConversation::messageSelected( const MessagePointer& msg ) {
  LogSuffix( "messageSelected: ", this );
  try {
  if( !m_currentConnectedDocument ) 
    throw "no connected document";

  KTextEditor::View* view = m_currentConnectedDocument->activeView();
  if( !view )
    throw "current document has no view";
  
  MessagePointer::Locked l = msg;
  if( !l )
    throw "messageSelected: could not lock message";

  KTextEditor::Cursor c, endC;
  c = findPositionInDocument( l.cast<InDocumentMessage>(), &endC );
  
  KTextEditor::SmartInterface* smart = dynamic_cast<KTextEditor::SmartInterface*>( (KTextEditor::Document*)m_currentConnectedDocument );
  if ( smart ) {
    if ( endC.isValid() && c.isValid() ) {
      KTextEditor::Document* d = m_currentConnectedDocument;
      disconnect( d, SIGNAL(destroyed( QObject* )), this, SLOT(rangeDeleted()) );
      connect( d, SIGNAL(destroyed( QObject* )), this, SLOT(rangeDeleted()) );

      if( m_currentRange )
        delete m_currentRange;
      
      m_currentRange = smart->newSmartRange( c, endC );
      KSharedPtr<KTextEditor::Attribute> t( new KTextEditor::Attribute() );

      t->setProperty( QTextFormat::BackgroundBrush, QBrush( Qt::yellow ) );
      m_currentRange->setAttribute( t );

      smart->addHighlightToView( view, m_currentRange, true );
    }
  } else {
    throw "no smart-interface";
  }

  ///Select the message in the widget
  if( m_widget && m_messagesModel ) {
    for( int a = 0; a < m_messagesModel->rowCount(); a++ ) {
      QModelIndex i = m_messagesModel->index( a, 0 );
      if( i.isValid() ) {
        QVariant v = m_messagesModel->data( i, Qt::UserRole );
        if ( v.canConvert<MessagePointer>() ) {
          InDocumentMessagePointer smsg = v.value<MessagePointer>().cast<InDocumentMessage>();
          if( smsg.getUnsafeData() == msg.getUnsafeData() ) {
            m_widgets.messages->selectionModel()->clear();
            m_widgets.messages->selectionModel()->select( i, QItemSelectionModel::Select );
            break;
          }
        }
      }
    }
  }

  c = currentDocCursor();
  placeWidget( view, &c, true );
  } catch( const char* str ) {
    err() << str;
  } catch( const QString& str ) {
    err() << str;
  }
}

void InDocumentConversation::selectMessage( InDocumentMessagePointer msg ) {
  LogSuffix( "selectMessage: ", this );
  try {
    setActive( true );

    InDocumentMessagePointer::Locked l = msg;
    if ( !l )
      throw "could not lock message";

    if( l->document().isEmpty() ) {
      out( Logger::Warning ) << "the selected message has no text-reference";
      return;
    }

    KUrl url = TeamworkFolderManager::workspaceAbsolute( l->document() );

    out( Logger::Debug ) << "opening: " << url.path();


    IDocumentController* docControl = KDevTeamwork::documentController();

    IDocument* doc = docControl->documentForUrl( url );
    if(!doc) doc = docControl->openDocument( url );
    if ( doc ) {
      docControl->activateDocument( doc );
      documentActivated( doc, msg );

      KTextEditor::View* view = doc->textDocument() ->activeView();
      if( !view ) {
        out( Logger::Debug ) << "no view for the activated document available";
        QList<KTextEditor::View*> views = doc->textDocument() ->views();
        if( views.isEmpty() ) throw "no views available for document";
        view = dynamic_cast<KTextEditor::View*>( views.front() );
      }
      
      if ( view ) {
        KTextEditor::Cursor endC = KTextEditor::Cursor::invalid();
        KTextEditor::Cursor c = findPositionInDocument( l, l->end().isValid() ? &endC : 0 );
        view->setCursorPosition( c );
        //view->setCursorPosition( KTextEditor::Cursor( c.line() > 10 ? c.line() - 10 : c.line(), c.column() ) );
        if ( !c.isValid() )
          throw "could not find corrent position, cursor is invalid";

        {
          Block b( m_block );
          view->setCursorPosition( c );
        }

        messageSelected( msg );
      } else {
        throw "could not get view for " + url.path();
      }
    } else {
      throw "could not open document " + url.path();
    }
  } catch ( const char * str ) {
    err() << str;
  } catch ( QString str ) {
    err() << str;
  }
}
void InDocumentConversation::userInfo() {
  manager() ->manager() ->teamwork() ->showUserInfo( primaryUser() );
}

void InDocumentConversation::messageContextMenu ( const QPoint &p ) {
  log( "messageContextMenu", Debug );
  QMenu menu( m_widget );
  QModelIndex i = m_widgets.messages->indexAt( p );
  if( !i.isValid() ) {
    log( "messageContextMenu: index is invalid", Error );
    return;
  }
  QVariant v = m_messagesModel->data( i, Qt::UserRole );
  if ( v.canConvert<MessagePointer>() ) {
    InDocumentMessagePointer msg = v.value<MessagePointer>().cast<InDocumentMessage>();
    manager() ->manager() ->fillMessageMenu( &menu, msg );
    menu.exec( p );
  }
}

InDocumentMessagePointer InDocumentConversation::selectedMessage() {
  if( !m_widget ) return 0;
  QModelIndexList il = m_widgets.messages->selectionModel()->selectedIndexes ();
  if( il.isEmpty() ) return 0;
  return m_messagesModel->data( il.front(), Qt::UserRole ).value<MessagePointer>().cast<InDocumentMessage>();
}


void InDocumentConversation::messageClicked( const QModelIndex& index ) {
  log( "messageClicked", Debug );
  QVariant v = index.model() ->data( index, Qt::UserRole );
  if ( v.canConvert<MessagePointer>() ) {
    InDocumentMessagePointer msg = v.value<MessagePointer>().cast<InDocumentMessage>();
    
    QMetaObject::invokeMethod( this, "selectMessage", Qt::QueuedConnection, Q_ARG( InDocumentMessagePointer, msg ) );
  } else {
    manager() ->log( "cannot convert a list-item to a MessagePointer", Error );
  }
}

void InDocumentConversation::gotReply( MessagePointer smsg ) {
  MessagePointer::Locked msg = smsg;
  if ( !msg ) {
    addListItem( "failed to send, got no reply" );
  } else {
    if ( msg->info().replyToMessage().getUnsafeData() == ( MessageInterface* ) m_sendingMessage.getUnsafeData() ) {
      SafeSharedPtr<KDevSystemMessage>::Locked sysMsg = msg.cast<KDevSystemMessage>();
      if ( sysMsg ) {
        if ( sysMsg->message() == KDevSystemMessage::ActionSuccessful ) {
          messageSendReady( true );
          SafeSharedPtr<InDocumentMessage>::Locked l = msg->info().replyToMessage().cast<InDocumentMessage>();
          addMessage( l );
          return ;
        } else {
          addListItem( "failed to send, reply: " + sysMsg->messageAsString() );
        }
      } else {
        manager() ->log( "could  not lock the received message as kdev-system-message", Error );
      }
    } else {
      manager() ->log( "got unexpected reply-message", Error );
      return ;
    }
  }

  messageSendReady( false );
}

void InDocumentConversation::messageSendReady( bool success ) {
  if ( m_widget ) {
    if ( success ) {
      m_widgets.message->clear();
    }
    m_widgets.message->setEnabled( true );
  }

  m_sendingMessage = 0;
  if( !m_lastSendTime.isValid() || m_lastSendTime.msecsTo( QTime::currentTime() ) < 400 ) {
    m_widgets.message->setFocus();
  }
}

void InDocumentConversation::hide() {
  setActive( false );
  if ( m_widget )
    m_widget->hide();
}

void InDocumentConversation::sendMessage() {
  try {
    LogSuffix( "sendMessage(): ", this );
    if ( !m_widget )
      throw QString( "no widget" );
    QString text = m_widgets.message->text();
    if ( text.isEmpty() )
      throw "no text";

    IDocumentController* docControl = KDevTeamwork::documentController();

    IDocument* d = docControl->activeDocument();
    if ( !d )
      throw "no active document";

    KTextEditor::Document* doc = d->textDocument();
    if ( !doc )
      throw "no active text-document";

    KTextEditor::View* view = doc->activeView();
    if ( !view )
      throw "no active view";

    InDocumentReference start( "" );
    InDocumentReference end;
    /*
    KTextEditor::MarkInterface* mark = dynamic_cast<KTextEditor::MarkInterface*>( view );
    if( !mark ) {
      log( "no MarkInterface", Warning );
    } else {
      mark->
    }*/
    KTextEditor::Range r = view->selectionRange();
    if ( r.isValid() ) {
      KTextEditor::Cursor b = r.start();
      KTextEditor::Cursor e = r.end();
      QString txt = doc->text();
      QString path = TeamworkFolderManager::workspaceRelative( doc->url().pathOrUrl() );
      start = InDocumentReference( path, b.line(), b.column(), txt );
      end = InDocumentReference( path, e.line(), e.column(), txt );
      out( Logger::Debug ) << "sending references: " << start.asText() << " " << end.asText();
    }

    MessagePointer::Locked stdMsg = globalMessageTypeSet().create<InDocumentMessage>( text, start, end, context() );

    InDocumentMessagePointer::Locked msg = stdMsg.cast<InDocumentMessage>();
    
    if ( !msg )
      throw QString( "could not create InDocumentMessage" );

    if ( r.isValid() )
      msg->contextLines() = DocumentContextLines( start, end, doc->text(), 5 );


    msg->setConversation( this );

    SessionPointer s = session();

    if ( !s )
      throw QString( "no session to send the message" );

    s.getUnsafeData() ->sendMessage( msg );
    m_widgets.message->setEnabled( false );
    m_sendingMessage = msg;
    
    manager()->manager()->teamwork()->addMessageToList( msg.data() );
  } catch ( const char * str ) {
    err() <<"error while sending message: " << str;
  } catch ( QString str ) {
    err() <<"error while sending message: " << str;
  }
}

void InDocumentConversation::jumpTo() {
  try {
    if( m_messages.empty() )
      throw "no messages";
    //setActive( true );
    QAction* action = qobject_cast<QAction*>( sender() );
    if ( !action )
      throw "no sender-action";

    QVariant v = action->data();
    if ( !v.canConvert<MessagePointer>() )
      throw "cannot convert variant to message";

    MessagePointer mp = v.value<MessagePointer>();
    if ( !mp.cast<InDocumentMessage>( ) )
      throw "wrong message-type";

    selectMessage( mp.cast<InDocumentMessage>() );
  } catch ( const char * str ) {
    manager() ->log( QString( "error in jumpTo: " ) + str, Error );
  }
}

void InDocumentConversation::fillContextMenu( QMenu* menu, KDevTeamwork* teamwork, MessagePointer msg ) {
  if ( !m_jumpToAction ) {
    m_jumpToAction = new QAction( "Show in Document", this );
    connect( m_jumpToAction, SIGNAL( triggered() ), this, SLOT( jumpTo() ), Qt::QueuedConnection );
    m_jumpToAction->setToolTip( "Open the document and jump to the references position." );

    QVariant v;
    v.setValue( msg );
    m_jumpToAction->setData( v );
  }

  menu->addAction( m_jumpToAction );
}

void InDocumentConversation::cursorPositionChanged ( KTextEditor::View *view, const KTextEditor::Cursor& newPos ) {
  if( !m_active ) return;
  if ( m_smartCursor ) {
    int diff = m_smartCursor->line() - newPos.line();
    if ( diff > -10 && diff < 20 ) {
      placeWidget( view, &newPos );  ///Move the widget away from the cursor
    }
  }

  if( m_currentConnectedDocument ) ///Eventually select another message that is nearer
    m_selectNearestMessageTimer->start( 400 );
    
}

void InDocumentConversation::verticalScrollPositionChanged ( KTextEditor::View *view, const KTextEditor::Cursor& newPos ) {
  if( !m_active || m_block ) return;
  KTextEditor::Cursor c = currentDocCursor();
  placeWidget( view, &c );
  //manager()->log( "verticalScrollPositionChanged", Debug );
}

void InDocumentConversation::log( QString str, LogLevel level ) const {
  const_cast<ConversationManager*>( manager() ) ->log( "InDocumentConversation: " + str, level );
};


void InDocumentConversation::horizontalScrollPositionChanged ( KTextEditor::View *view ) {
  //manager()->log( "horizontalScrollPositionChanged", Debug );
}

void InDocumentConversation::fillMessageModel() {
  for ( MessageSet::Iterator it = m_messages.orderedValues<int>(); it; ++it ) {
    fillMessageToModel( it->message );
  }
}

void  InDocumentConversation::selectNearestMessage() {
  if( !m_currentConnectedDocument ) {
    err() << "selectNearestMessage() called without active document";
    return;
  }
  InDocumentMessagePointer msg;
  
    KTextEditor::Cursor c = m_currentConnectedDocument->activeView()->cursorPosition();
    int nearestDiff = 1000000;
    KTextEditor::Cursor nearestCursor;
    
    QString file = TeamworkFolderManager::workspaceRelative( m_currentConnectedDocument->url().path() );

    MessageSet::Iterator it = m_messages.values( file );

    while( it ) {
      InDocumentMessagePointer::Locked l = it->message;
      if( l ) {
        KTextEditor::Cursor end;
        KTextEditor::Cursor start = findPositionInDocument( l, &end );
        int diff = c.line() - start.line();
        if( diff < 0 ) diff = -diff;
        if( diff < nearestDiff ) {
          nearestDiff = diff;
          nearestCursor = start;
          msg = it->message;
        }
      }
      ++it;
    }
    if( msg )
      messageSelected( msg );
}

void InDocumentConversation::userStateChanged( KDevTeamworkUserPointer user ) {
  log( "userStateChanged" );
  fillUserBox();
}

void InDocumentConversation::fillUserBox() {
  try {
    if ( !m_widget )
      throw "no widget";
    m_widgets.talkingUsers->clear();

    UserPointer user = primaryUser();
    if ( !user )
      throw "could not figure out primary user";

    UserPointer::Locked l = user;
    if ( !l )
      throw "could not lock user";
    QVariant v;
    v.setValue<UserPointer>( user );


    KDevTeamworkUserPointer::Locked us = user.cast<KDevTeamworkUser>();
    if ( !us )
      throw "could not cast to KDevTeamworkUser, and lock.";

    m_widgets.talkingUsers->addItem( us->icon(), ~l->name(), v );
    m_userConnector = 0;
    m_userConnector = new TemporaryConversationConnector( us, this );
  } catch ( const char * str ) {
    log( QString( "error in fillUserBox: " ) + str , Error );
  }
}

void InDocumentConversation::rangeDeleted() {
  m_currentRange = 0;
}


void InDocumentConversation::fillMessageToModel( const InDocumentMessagePointer::Locked& msg ) {
  if ( !m_messagesModel )
    return ;
  if ( msg ) {
    int row = 0; //m_messagesModel->rowCount();
    m_messagesModel->insertRow( row );
    QModelIndex i = m_messagesModel->index( row, 0 );
    m_messagesModel->setData( i, msg->messageText(), Qt::DisplayRole );
    m_messagesModel->setData( i, msg->messageIcon() , Qt::DecorationRole );
    QVariant v;
    MessagePointer m( ( MessageInterface* ) msg.data() );
    v.setValue( m );
    m_messagesModel->setData( i, v , Qt::UserRole );
  } else {}
}

void InDocumentConversation::addListItem( const QString& txt , const QString& icon ) {
  manager() ->log( txt, Debug );
  if ( !m_messagesModel )
    return ;
  int row = 0; //m_messagesModel->rowCount();
  m_messagesModel->insertRow( row );
  QModelIndex i = m_messagesModel->index( row, 0 );
  m_messagesModel->setData( i, txt, Qt::DisplayRole );
  m_messagesModel->setData( i, IconCache::instance() ->getIcon( icon ) , Qt::DecorationRole );
}

void InDocumentConversation::setupWidget( QWidget* parent ) {
  m_widget = new QWidget( parent );
  m_widgets.setupUi( m_widget );

  QPalette p = m_widgets.frame->palette();
  p.setColor( QPalette::Background, QColor::fromRgb( 190, 190, 0, 150 ) );
  m_widgets.frame->setPalette( p );
  m_widgets.messages->header() ->hide();
  //m_widgets.messages->setWrapping( true );

  connect( m_widgets.message, SIGNAL( returnPressed() ), this, SLOT( sendMessage() ), Qt::QueuedConnection );
  connect( m_widgets.messages, SIGNAL( clicked( const QModelIndex& ) ), this, SLOT( messageClicked( const QModelIndex& ) ) );
  connect( m_widgets.messages, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( messageContextMenu ( const QPoint & ) ) );

  if ( !m_messagesModel ) {
    m_messagesModel = new QStandardItemModel( 0, 1, 0 );
    fillMessageModel();
  }
  m_widgets.messages->setModel( m_messagesModel );

  if ( !m_userInfoAction ) {
    m_userInfoAction = new QAction( "User Info", this );
    connect( m_userInfoAction, SIGNAL( triggered() ), this, SLOT( userInfo() ) );
    m_userInfoAction->setToolTip( "Show information about the selected user." );
  }

  if ( !m_hideAction ) {
    m_hideAction = new QAction( "Hide", this );
    connect( m_hideAction, SIGNAL( triggered() ), this, SLOT( hide() ) , Qt::QueuedConnection );
    m_hideAction->setToolTip( "Hide this conversation." );
  }

  QMenu *actionMenu = new QMenu( "Actions", m_widgets.actionsButton );

  actionMenu->addAction( m_hideAction );
  actionMenu->addSeparator();
  actionMenu->addAction( m_userInfoAction );

  m_widgets.actionsButton->setMenu( actionMenu );
  m_widgets.actionsButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
  m_widgets.actionsButton->setArrowType( Qt::DownArrow );
  m_widgets.actionsButton->setPopupMode( QToolButton::InstantPopup );

  fillUserBox();
}

void InDocumentConversation::embedInView( KTextEditor::View* view, IDocument* document, KTextEditor::Cursor position ) {
  LogSuffix( "embedInView: ", this );
  try {
    if ( !view )
      throw "no view";
    if ( m_messages.empty() )
      throw "no messages";
    if ( m_widget )
      delete m_widget;

    disconnect( view, 0, this, 0 );
    //    KTextEditor::Cursor cursor( line, 0 );
    setupWidget( view );

    if ( !m_widget )
      throw "could not setup widget";

    connect( view, SIGNAL( verticalScrollPositionChanged ( KTextEditor::View*, const KTextEditor::Cursor& ) ), this, SLOT( verticalScrollPositionChanged ( KTextEditor::View*, const KTextEditor::Cursor& ) ) );

    connect( view, SIGNAL( horizontalScrollPositionChanged ( KTextEditor::View*, const KTextEditor::Cursor& ) ), this, SLOT( horizontalScrollPositionChanged ( KTextEditor::View*, const KTextEditor::Cursor& ) ) );

    connect( view, SIGNAL( cursorPositionChanged ( KTextEditor::View*, const KTextEditor::Cursor& ) ), this, SLOT( cursorPositionChanged ( KTextEditor::View*, const KTextEditor::Cursor& ) ) );

    m_smartCursor = SmartCursorContainer( document->textDocument() );

    if ( !m_smartCursor )
      throw "smart-cursor could not be created, maybe the editor is missing the interface";

    if( !document->textDocument() )
      throw "no text-document";
    
    if ( !position.isValid() )
      throw "the cursor-position is not valid";

    if( position > document->textDocument()->documentEnd() ) {
      out( Logger::Debug ) << "found position behind end of document";
      position = document->textDocument()->documentEnd();
    }

    m_smartCursor->setPosition( position );

    KTextEditor::Cursor c = currentDocCursor();

    placeWidget( view, &c, true );
  } catch ( const char * str ) {
    err() << str;
  }
}

void InDocumentConversation::placeWidget( KTextEditor::View* view, const KTextEditor::Cursor* awayFrom, bool forceShow ) {
  try {
    if ( !m_smartCursor )
      throw "smartcursor is invalid";
    if ( !m_widget )
      throw "no widget";

    QPoint p = view->cursorToCoordinate( *m_smartCursor );
    if ( p == QPoint( -1, -1 ) ) {
      if( !forceShow ) m_widget->hide();
    } else {
      if ( awayFrom ) {
        int sline = m_smartCursor->line();
        int widgetLines = 8; ///Determine this somehow better
        int sbottom = sline + widgetLines;
        int cline = awayFrom->line();
        int diff = sline - cline;
        if ( diff < 0 )
          diff = -diff;
        int diff2 = sbottom - cline;
        if ( diff2 < 0 )
          diff2 = -diff2;
        if ( diff < 10 || diff2 < 10 ) {
          p = QPoint( -1, -1 );

          if ( diff > diff2 ) {
            int nline = cline - 5 - widgetLines;
            if ( nline > 0 )
              p = view->cursorToCoordinate( KTextEditor::Cursor( nline, 0 ) );

            if ( p == QPoint( -1, -1 ) ) {
              nline = cline + 5;  ///If there's no space at the top, try it downwards.
              if ( view->document() && nline < view->document() ->lines() )
                p = view->cursorToCoordinate( KTextEditor::Cursor( nline, 0 ) );
            }
          } else {
            int nline = cline + 5;
            if ( view->document() && nline < view->document() ->lines() )
              p = view->cursorToCoordinate( KTextEditor::Cursor( nline, 0 ) );

            if ( p == QPoint( -1, -1 ) ) {
              nline = cline - 5 - widgetLines;  ///If there's no space at the top, try it downwards.
              if ( nline > 0 )
                p = view->cursorToCoordinate( KTextEditor::Cursor( nline, 0 ) );
            }
          }
        }
      }

      if( p == QPoint( -1, -1 ) ) {
        if( !forceShow ) {
            err() << "could not determine a good position to show the widget";
          p.setX( 50 );
        } else {
          p.setX( 50 );
          p.setY( 50 );
        }
      }

      //manager()->log( QString("smartcursor coordinates: %1 %2").arg( p.x() ).arg( p.y() ), Debug );
      m_widget->move( p );
      m_widget->show();
    }
  } catch ( const char * str ) {
    err() << "placeWidget: " << str;
  }
}

KTextEditor::Cursor InDocumentConversation::findPositionInDocument( InDocumentMessagePointer::Locked l, KTextEditor::Cursor* endTarget ) {
  LogSuffix s( "findPositionInDocument: ", this );
  if( !m_currentConnectedDocument ) {
    err() << "no connected document";
    return KTextEditor::Cursor();
  }
  if( !m_currentSearchInstance )
    m_currentSearchInstance = InDocumentReference::TextSearchInstance( m_currentConnectedDocument->text() );

  if ( l ) {
    int line, col;
    l->start().findInText( m_currentSearchInstance, line, col );
    int endLine = -1, endCol = -1;
    if ( l->end().isValid() && endTarget ) {
      l->end().findInText( m_currentSearchInstance, endLine, endCol );
      if ( endLine < line || ( endLine == line && endCol < col ) ) {
        int l = line, c = col;
        line = endLine;
        col = endCol;
        endLine = l;
        endCol = c;
        out( Logger::Warning ) << "Found end-marker before start-marker: " << line << ":" << col << "  -  " << endLine << ":" << endCol;
      }
    }

    if ( line == -1 ) {
      out( Logger::Warning ) << "Could not find reference-position for " << l->start().asText() << " in text of length "<< m_currentSearchInstance.text().length();
      line = l->start().line();
      col = l->start().col();
      if ( l->end().isValid() && endTarget ) {
        endLine = l->end().line();
        endCol = l->end().col();
      }
    }

    if ( endTarget && endLine != -1 ) {
      *endTarget = KTextEditor::Cursor( endLine, endCol == -1 ? 0 : endCol );
    }

    if ( line == -1 ) {
      err() << "cursor is invalid";
      line = 0;
      col = 0;
      return KTextEditor::Cursor::invalid();
    }
    if ( line != -1 )
      return KTextEditor::Cursor( line, col );
  } else {
    if ( !l )
      err() << "could not lock first message";
    else
      err() << "kdev-document has no text-document";
    return KTextEditor::Cursor::invalid();
  }
}

InDocumentConversation::InDocumentConversation( InDocumentMessage* msg ) : SafeLogger( KDevTeamwork::self()->logger() ), m_jumpToAction( 0 ), m_hideAction( 0 ), m_userInfoAction( 0 ), m_messagesModel( 0 ), m_messageCount( 0 ), m_block( false ), m_currentRange(0) {
  setActive( true );
  LogSuffix s( "constructor: ", this );

  m_selectNearestMessageTimer = new QTimer( this );
  m_selectNearestMessageTimer->setSingleShot( true );
  connect( m_selectNearestMessageTimer, SIGNAL( timeout() ), this, SLOT( selectNearestMessage() ) );
  
  qRegisterMetaType<InDocumentMessagePointer>( "InDocumentMessagePointer" );
  
  if ( msg ) {
    m_line = msg->start().line();
    m_documentName = ~msg->document();
    manager() ->log( "document-name: " + ~m_documentName, Debug );
    addMessage( msg );

    try {
      QString document = msg->document();
      if ( document.isEmpty() )
        throw "in-document-message has no document-information";

      int line = msg->start().line();
      if ( line == -1 )
        throw "in-document-message has no line-information";

      IDocumentController* docControl = KDevTeamwork::documentController();
      if ( !docControl )
        throw "no document-controller";

      IDocument* kdevDoc = docControl->activeDocument();
      if ( kdevDoc )
        documentActivated( kdevDoc, msg );
    }
    catch ( const char * str ) {
      manager() ->log( QString( "error while dispatching InDocumentMessage: " ) + str, Error );
    }
  }
}

InDocumentConversation::~InDocumentConversation() {
  if ( m_messagesModel )
    delete m_messagesModel;
  if( m_currentRange )
    delete m_currentRange;
}

SessionPointer InDocumentConversation::session() {
  try {
    if ( m_messages.empty() )
      throw "no messages";
    InDocumentMessagePointer::Locked lmsg = m_messages.value<int>( m_messageCount-1 ).message;
    if ( !lmsg )
      throw "could not lock message";
    return lmsg->info().session();
  } catch ( const char * str ) {
    err() << "could not get session: " << str;
    return 0;
  }
}

UserPointer InDocumentConversation::primaryUser() {
  if ( m_messages.empty() )
    return 0;
  InDocumentMessagePointer::Locked l = m_messages.value<int>(m_messageCount-1).message;
  if ( !l )
    return 0;
  return l->info().user();
}

void InDocumentConversation::textChanged ( KTextEditor::Document * document, const KTextEditor::Range & oldRange, const KTextEditor::Range & newRange ) {
  m_currentSearchInstance = InDocumentReference::TextSearchInstance();
}

void InDocumentConversation::textRemoved ( KTextEditor::Document * document, const KTextEditor::Range & range ) {
  m_currentSearchInstance = InDocumentReference::TextSearchInstance();
}

void InDocumentConversation::textInserted ( KTextEditor::Document * document, const KTextEditor::Range & range ) {
  m_currentSearchInstance = InDocumentReference::TextSearchInstance();
}

void InDocumentConversation::documentActivated( IDocument* document ) {
  documentActivated( document, 0 );
}

void InDocumentConversation::documentActivated( IDocument* document, const InDocumentMessagePointer& msg_ ) {
  LogSuffix s( "documentActivated: " , this );
  if ( !m_active ) {
    err() << "This conversation is not active";
    return;
  }
  if( !document->textDocument() || ! document->textDocument() ->activeView() ) {
    err() << "Document is no text-document";
    return ;
  }
  QString file = TeamworkFolderManager::workspaceRelative( document->url().path() );

  MessageSet::Iterator it = m_messages.values( file );
  
  if( !it /*&& !msg_*/ ) {
  /*  out( Logger::Debug ) << "comparing " << file << " failed: is not part of conversation";
    MessageSet::Iterator it = m_messages.orderedValues<QString>();
    QString current = "__/__";
    while( it ) {
      InDocumentMessagePointer::Locked l = it->message;
      if( l ) {
        if( current != l->document() ) {
          out( Logger::Debug ) << "files are: \"" << l->document() << "\"";
          current = l->document();
        }
      }
      
      ++it;
  }*/
    return; ///Document contains none of our messages
  }
  out( Logger::Debug ) << "comparing " << file << " successful";

  KTextEditor::Document* doc = document->textDocument();

  InDocumentMessagePointer msg = msg_;
  
  if( doc != m_currentConnectedDocument ) {

    if( m_currentConnectedDocument )
      disconnect( m_currentConnectedDocument, 0, this, 0 );

    connect( doc, SIGNAL( textInserted ( KTextEditor::Document *,
             const KTextEditor::Range & ) ), this, SLOT( textInserted ( KTextEditor::Document *,
             const KTextEditor::Range & ) ) );
    connect( doc, SIGNAL( textRemoved ( KTextEditor::Document *,
             const KTextEditor::Range & ) ), this, SLOT( textRemoved ( KTextEditor::Document *,
             const KTextEditor::Range & ) ) );
    connect( doc, SIGNAL( textChanged ( KTextEditor::Document *,
             const KTextEditor::Range &,
             const KTextEditor::Range & ) ), this, SLOT( textChanged ( KTextEditor::Document *,
             const KTextEditor::Range &,
             const KTextEditor::Range & ) ) );
  }

  bool wasNewDocument = false;
  
  if( m_currentConnectedDocument != doc ) {
    wasNewDocument = true;
    m_currentSearchInstance = InDocumentReference::TextSearchInstance();
  }

  m_currentConnectedDocument = doc;
  
  ///Determine the nearest message to the cursor

  if( !msg ) {
    err() << "no nearest message found";
    return;
  }

  if( wasNewDocument )
    embedInView( doc ->activeView(), document, findPositionInDocument(  msg ) );

  if( msg )
    messageSelected( msg );
  else
    m_selectNearestMessageTimer->start( 300 );
}

void InDocumentConversation::setActive( bool active ) {
  if ( m_active == active )
    return ;

  m_active = active;
  IDocumentController* docControl = KDevTeamwork::documentController();

  if ( active ) {
    if( m_widget ) m_widget->show();
    connect( docControl, SIGNAL( documentActivated( KDevelop::IDocument* ) ), this, SLOT( documentActivated( KDevelop::IDocument* ) ) );
    connect( docControl, SIGNAL( documentLoaded( KDevelop::IDocument* ) ), this, SLOT( documentActivated( KDevelop::IDocument* ) ) );
    connect( manager() ->manager() ->teamwork(), SIGNAL( updateConnection( TeamworkClientPointer ) ), this, SLOT( updateAllUsers() ) );
    documentActivated( docControl->activeDocument() );
  } else {
    if( m_currentConnectedDocument ) {
      if( m_currentConnectedDocument->activeView() )
        disconnect( m_currentConnectedDocument->activeView(), 0, this, 0 );
      disconnect( m_currentConnectedDocument, 0, this, 0 );
      
    }
    m_currentConnectedDocument = 0;

    m_currentSearchInstance = InDocumentReference::TextSearchInstance();

    disconnect( docControl, 0, this, 0 );
    disconnect( manager() ->manager() ->teamwork(), 0, this, 0 );
  }
}

QString InDocumentConversation::context() const {
  if ( m_messages.empty() )
    return "";
  InDocumentMessagePointer::Locked l = const_cast<InDocumentMessagePointer&>( m_messages.value<int>(m_messageCount-1).message );
  if ( l )
    return l->context();
  else
    return "";
}

bool InDocumentConversation::match( InDocumentMessage* msg ) const {
  return context() == msg->context();
}

void InDocumentConversation::addMessage( InDocumentMessage* msg ) {
  msg->setConversation( this );
  if( m_messages[(InDocumentMessagePointer)msg] ) return;
  pushMessage( msg );
  if ( m_messagesModel )
    fillMessageToModel( msg );

  if ( msg->info().isIncoming() && msg->info().session() ) {
    ///Send a notification that the message arrived to the other side.
    //globalMessageSendHelper().sendReply<KDevSystemMessage>( msg, KDevSystemMessage::ActionSuccessful ); //this is also done for KDevTeamworkTextMessage now
  }
}


struct DocumentMessageInternal {
  QPointer<InDocumentConversation> m_conversation;
};

QIcon InDocumentMessage::messageIcon() const {
  IconCache & cache( *IconCache::instance() );
  if ( info().isIncoming() ) {
      return cache( "document_message_in" ).pixmap( QSize( 16, 16 ), m_start ? QIcon::Normal : QIcon::Disabled  );
  } else {
    return cache( "document_message_out" ).pixmap( QSize( 16, 16 ), m_start ? QIcon::Normal : QIcon::Disabled  );
  }
}

void InDocumentMessage::fillContextMenu( QMenu* menu, KDevTeamwork* teamwork ) {
  if ( m_internal.get() ) {
    if ( m_internal->m_conversation ) {
      m_internal->m_conversation->fillContextMenu( menu, teamwork, this );
    }
  }
  KDevTeamworkTextMessage::fillContextMenu( menu, teamwork );
}

void InDocumentMessage::setConversation( InDocumentConversation* conv ) {
  if ( !m_internal.get() )
    m_internal = auto_ptr<DocumentMessageInternal>( new DocumentMessageInternal );
  m_internal->m_conversation = conv;
}

QString InDocumentMessage::context() {
  return ~m_context;
}

QString InDocumentMessage::document() {
  return m_start.document();
}

InDocumentMessage::InDocumentMessage( const Teamwork::MessageInfo& info, const QString& text, const InDocumentReference& startRef, const InDocumentReference& endRef, const QString& context ) : KDevTeamworkTextMessage( info, text ), m_start( startRef ), m_end( endRef ), m_context( ~context ) {}

InDocumentMessage::InDocumentMessage( InArchive& from, const Teamwork::MessageInfo& info ) : KDevTeamworkTextMessage( from, info ) {
  serial( from );
}

void InDocumentMessage::serialize( OutArchive& arch ) {
  Precursor::serialize( arch );
  serial( arch );
}

bool InDocumentMessage::needReply() const {
  return m_internal.get() && m_internal->m_conversation;
}

void InDocumentMessage::result( bool success ) {
  if ( !success ) {
    QMetaObject::invokeMethod( ( InDocumentConversation* ) m_internal->m_conversation, "gotReply", Qt::QueuedConnection, Q_ARG( MessagePointer, ( MessagePointer ) 0 ) );
  }
}

MessageInterface::ReplyResult InDocumentMessage::gotReply( const DispatchableMessage& p ) {
  if ( m_internal.get() && m_internal->m_conversation ) {
    QMetaObject::invokeMethod( ( InDocumentConversation* ) m_internal->m_conversation, "gotReply", Qt::QueuedConnection, Q_ARG( MessagePointer, ( MessagePointer ) p ) );
  }
  
  return KDevTeamworkTextMessage::gotReply( p );
}

#include "conversationmanager.moc"

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
