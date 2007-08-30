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

#include "kdevteamwork_messageshower.h"
#include  <QWidget>
#include <QObject>
#include "kdevteamwork_messages.h"
#include "ui_kdevteamwork_textmessage.h"
#include "ui_kdevteamwork_indocumentcontextlines.h"
#include "ui_kdevteamwork_indocumentmessagebuttons.h"
#include "ui_kdevteamwork_indocumentmessagestate.h"
#include <klocalizedstring.h>
#include <QPointer>
#include "kdevteamwork.h"
#include "utils.h"
#include "kdevteamwork_user.h"
#include "messagemanager.h"
#include <QPixmap>
#include <QMenu>
#include "messagehistorymanager.h"
#include <kdialog.h>
#include <QMetaType>
#include "conversationmanager.h"
#include <qtextdocument.h>
#include <krandom.h>


KDevTeamworkMessageShower::KDevTeamworkMessageShower( KDevTeamworkTextMessage* msg, QWidget* widget, KDevTeamwork* teamwork ) : QObject( widget ), m_message( msg ), m_widget( widget ), m_teamwork( teamwork ) {
  m_widgetData.setupUi( m_widget );

  fill();

  connect( m_widgetData.menuButton, SIGNAL( clicked() ), this, SLOT( menuButton() ) );
  connect( m_widgetData.historyButton, SIGNAL( clicked() ), this, SLOT( historyButton() ) );
  connect( m_widgetData.isReplyToButton, SIGNAL( clicked() ), this, SLOT( isReplyToButton() ) );
  connect( teamwork->messageManager(), SIGNAL( messageUpdated( const MessagePointer& ) ), this, SLOT( messageUpdated( const MessagePointer& ) ) );
}

void KDevTeamworkMessageShower::fillData() {
  fill();
}

void KDevTeamworkMessageShower::fill() {
  LockedSharedPtr<KDevTeamworkTextMessage> msg = m_message;
  if ( msg ) {
    m_widgetData.text->setText( ~msg->text() );
    m_widgetData.state->setText( QString( "ok" ) );
    QString user = QString( "Unknown" );

    if ( msg->info().user() ) {
      user = ~msg->info().user().unsafe() ->safeName();
      if ( msg->info().user().cast<KDevTeamworkUser>() ) {
        connect( msg->info().user().cast<KDevTeamworkUser>().unsafe(), SIGNAL( userStateChanged( KDevTeamworkUserPointer ) ), this, SLOT( userStateChanged() ) );
      }
    }

    LockedSharedPtr<KDevTeamworkTextMessage> l = m_message;
    if ( l ) {
      m_widget->setWindowIcon( l->messageIcon() );
    }

    m_widgetData.user->setText( QString( user ) );
    m_widgetData.time->setText( QString( msg->creationTime().toString() ) );

    QString direction;
    if ( msg->info().isIncoming() )
      direction = QString( "incoming" );
    else
      direction = QString( "outgoing" );


    m_widgetData.direction->setText( QString( direction ) );

    //m_widgetData.answerTo->setText( QString( "Answer to: %1" ).arg( msg->info().isReplyTo() ) );

    if ( !msg->info().isReplyTo() ) {
      m_widgetData.isReplyToButton->setEnabled( false );
    }
  }
}

void KDevTeamworkMessageShower::messageUpdated( const MessagePointer& msg ) {
  if ( msg.unsafe() == m_message.unsafe() ) {
    fillData();
  }
}


void KDevTeamworkMessageShower::isReplyToButton() {
  LockedSharedPtr<KDevTeamworkTextMessage> l = m_message;
  if ( l ) {
    if ( l->info().isReplyTo() ) {
      MessagePointer repliedMessage = l->info().replyToMessage();
      if ( !repliedMessage && m_teamwork )
        repliedMessage = m_teamwork->messageManager() ->historyManager().getMessageFromId( l->info().isReplyTo() , m_teamwork->client() );

      if ( LockedSharedPtr<KDevTeamworkTextMessage> lrep = repliedMessage.cast<KDevTeamworkTextMessage>() ) {
        QDialog * dialog = new QDialog( m_widget );
        dialog->setAttribute( Qt::WA_DeleteOnClose, true );
        new KDevTeamworkMessageShower( lrep, dialog, m_teamwork );
        dialog->show();
      }
    }
  } else {}
}

void KDevTeamworkMessageShower::menuButton() {
  if ( m_teamwork && m_message ) {
    QMenu * menu = new QMenu( m_widget );

    m_teamwork->messageManager() ->fillMessageMenu( menu, m_message );

    menu->exec( QCursor::pos() );
  }
}

void KDevTeamworkMessageShower::historyButton() {
  LockedSharedPtr<KDevTeamworkTextMessage> l = m_message;
  if ( l && m_teamwork ) {
    KDevTeamworkUserPointer u = l->info().user().cast<KDevTeamworkUser>();
    if ( u ) {
      QList<KDevTeamworkUserPointer> users;
      users << u;
      m_teamwork->messageManager() ->showMessageHistory( users );
    } else {}
  }
}

void KDevTeamworkMessageShower::userStateChanged() {
  LockedSharedPtr<KDevTeamworkTextMessage> lmsg = m_message;
  if ( lmsg ) {
    LockedSharedPtr<KDevTeamworkUser> l = lmsg->info().user().cast<KDevTeamworkUser>();
    if ( l ) {
      m_widgetData.userIcon->setPixmap( l->icon().pixmap( 32, 32 ) );
      return ;
    }
  }
  m_widgetData.userIcon->setPixmap( QPixmap() );
}


KDevTeamworkUserInfoShower::KDevTeamworkUserInfoShower( KDevTeamworkUser* user, QWidget* widget, KDevTeamwork* teamwork ) : QObject( widget ), m_user( user ), m_widget( widget ), m_teamwork( teamwork ) {
  m_widgetData.setupUi( m_widget );
  m_widgetData.description->setText( ~user->description() );
  m_widgetData.name->setText( ~( ( Teamwork::User* ) user ) ->name() );
  m_widgetData.email->setText( QString( ~user->email() ) );
  m_widgetData.rights->setText( QString( ~user->rightsAsString() ) );

  setIp();
  setStatus();
  setIcon();

  connect( m_widgetData.menuButton, SIGNAL( clicked(bool) ), this, SLOT( menuButton() ) );
  connect( m_widgetData.historyButton, SIGNAL( clicked(bool) ), this, SLOT( historyButton() ) );
  connect( m_user.unsafe(), SIGNAL( userStateChanged( KDevTeamworkUserPointer ) ), this, SLOT( userStateChanged() ) );
}

void KDevTeamworkUserInfoShower::menuButton() {
  if ( m_teamwork && m_user ) {
    QMenu * menu = new QMenu( m_widget );

    m_teamwork->fillUserMenu( menu, m_user );

    menu->exec( QCursor::pos() );
  }
}


void KDevTeamworkUserInfoShower::userStateChanged() {
  setIp();
  setStatus();
  setIcon();
}


void KDevTeamworkUserInfoShower::historyButton() {
  if ( m_teamwork && m_user ) {
    QList<KDevTeamworkUserPointer> users;
    users << m_user;
    m_teamwork->messageManager() ->showMessageHistory( users );
  }
}

void KDevTeamworkUserInfoShower::setIp() {
  LockedSharedPtr<KDevTeamworkUser> l = m_user;
  QString address = QString( "Unknown" );
  if ( l ) {
    SessionPointer::Locked ls = l->online().session();
    if ( ls )
      address = ~ls->peerDesc();
  }
  m_widgetData.ipadress->setText( QString( address ) );
}


void KDevTeamworkUserInfoShower::setStatus() {
  LockedSharedPtr<KDevTeamworkUser> l = m_user;
  if ( l ) {
    QStringList available;
    if ( !l->online() ) {
      available << QString( "not available" );
    } else {
      if ( l->online().session().unsafe() ->sessionType() == SessionInterface::Direct ) {
        available << QString( "direct" );
      } else {
        available << QString( "indirect" );
      }

      if ( l->online().session().unsafe() ->sessionDirection() == SessionInterface::Incoming ) {
        available << QString( "incoming" );
      } else {
        available << QString( "Outgoing" );
      }
    }

    m_widgetData.state->setText( QString( available.join( ", " ) ) );
  } else {
    m_widgetData.state->setText( QString( QString( "Lock Failed" ) ) );
  }
}


void KDevTeamworkUserInfoShower::setIcon() {
  LockedSharedPtr<KDevTeamworkUser> l = m_user;
  if ( l ) {
    m_widgetData.icon->setPixmap( l->icon().pixmap( 32, 32 ) );
    m_widget->setWindowIcon( l->icon() );
  } else {
    m_widgetData.icon->setPixmap( QPixmap() );
    m_widget->setWindowIcon( QIcon() );
  }
}

struct InDocumentMessageShowerData {
  Ui_InDocumentButtons buttons;
  Ui_InDocumentContextLines contextLines;
  Ui_InDocumentState state;
  InDocumentMessageShowerData() : m_initialized( false ) {}
  void initialize( Ui_TextMessageShower* widgetData ) {
    if ( !m_initialized ) {

      buttons.setupUi( widgetData->buttonSpace );
      widgetData->buttonSpace->show();

      contextLines.setupUi( widgetData->middleSpace );
      widgetData->middleSpace->setMinimumHeight( widgetData->middleSpace->sizeHint().height() );
      widgetData->middleSpace->show();

      state.setupUi( widgetData->stateSpace );
      widgetData->stateSpace->setMinimumHeight( widgetData->stateSpace->sizeHint().height() );
      widgetData->stateSpace->show();

      m_initialized = true;
    }
  }

private:
  bool m_initialized;
};

InDocumentMessageShower::InDocumentMessageShower( InDocumentMessage* msg, QWidget* widget, KDevTeamwork* teamwork ) : KDevTeamworkMessageShower( msg, widget, teamwork ), SafeLogger( teamwork->logger() , "InDocumentMessage: " ) {
  m_data->initialize( &m_widgetData );
  connect( m_data->state.file, SIGNAL( leftClickedUrl( const QString& ) ), this, SLOT( fileClicked() ) );
  connect( m_data->state.context, SIGNAL( leftClickedUrl( const QString& ) ), this, SLOT( contextClicked() ) );
  connect( m_data->buttons.jumpToPosition, SIGNAL( clicked() ), this, SLOT( fileClicked() ) );
  fill();
}

void InDocumentMessageShower::fillData() {
  KDevTeamworkMessageShower::fillData();
  fill();
}

void InDocumentMessageShower::fileClicked() {
  try {
    if ( !m_teamwork )
      throw "no teamwork";

    LockedSharedPtr<InDocumentMessage> l = m_message.cast<InDocumentMessage>();
    if ( !l )
      throw "could not lock message";

    InDocumentConversationPointer conv = m_teamwork->messageManager() ->conversationManager().getConversation( l );

    if ( !conv )
      throw "could not get conversation";

    conv->selectMessage( l );
  } catch ( QString str ) {
    err() << str;
  }
}


void InDocumentMessageShower::contextClicked() {
  try {
    if ( !m_teamwork )
      throw "no teamwork";

    LockedSharedPtr<InDocumentMessage> l = m_message.cast<InDocumentMessage>();
    if ( !l )
      throw "could not lock message";

    m_teamwork->messageManager() ->showMessageHistory( QList<KDevTeamworkUserPointer>(), l->context() );
  } catch ( QString str ) {
    err() << str;
  }
}

QString escapeWithoutLineBreaks( const QString& str ) {
  QStringList lines = str.split( '\n' );
  QStringList ret;
  foreach( QString line, lines ) {
    ret << Qt::escape( line );
  }
  return ret.join( "<br>\n" );
}

void InDocumentMessageShower::fill() {
  LockedSharedPtr<InDocumentMessage> l = m_message.cast<InDocumentMessage>();
  if ( l ) {
    int offset = l->contextLines().lineOffset();

    QString marker1 = KRandom::randomString( 30 );
    QString marker2 = KRandom::randomString( 30 );

    QString lines = l->contextLines().text();
    if ( l->start() && l->end() ) {
      int startIndex = lineColToIndex( lines, l->start().line() - offset, l->start().col() );
      int endIndex = lineColToIndex( lines, l->end().line() - offset, l->end().col() );
      if ( startIndex != -1 && endIndex != -1 ) {
        lines = lines.mid( 0, startIndex ) + marker1 + lines.mid( startIndex, endIndex - startIndex ) + marker2 + lines.mid( endIndex );
      }
    }

    QString txt = escapeWithoutLineBreaks( lines );

    txt = txt.replace( marker1, "<font color=\"red\">" );
    txt = txt.replace( marker2, "</font>" );

    m_data->contextLines.contextLines->setHtml( txt );
    m_data->state.file->setText( l->document() );
    m_data->state.context->setText( l->context() );
    m_data->state.file->setUrl( "hallo" );
    m_data->state.context->setUrl( "hallo" );

    QString refType = "static";
    if ( !l->start().isValid() ) {
      refType = "invalid";
    } else if ( l->start().isDynamic() ) {
      refType = "dynamic";
      if ( l->end().isValid() ) {
        refType += " range";
      }
    } else {
      refType = "static";
      if ( l->end().isValid() ) {
        refType += " range";
      }
    }

    m_data->state.referenceType->setText( refType );

    QString position = l->start().asText();
    if ( l->end().isValid() )
      position += " to " + l->end().asText();

    m_data->state.position->setText( position );

  } else {}
}

#include "kdevteamwork_messageshower.moc"

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
