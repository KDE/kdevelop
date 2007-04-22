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


#ifndef KDEVTEAMWORK_MESSAGES_H
#define KDEVTEAMWORK_MESSAGES_H

//#include <boost/serialization/extended_type_info_typeid.hpp>

#include "network/common.h"
#include "network/message.h"
#include "network/teamworkmessages.h"
#include <QString>
#include <QDateTime>
#include "utils.h"
#include <QIcon>
#include <QVariant>
#include "easymessage.h"
#include <string>
#include <boost/serialization/level.hpp>
#include "loglevel.h"

/*#define BOOST_SERIALIZATION_DEFAULT_TYPE_INFO(T) \
    extended_type_info_typeid<const T>*/
/**/

using namespace Teamwork;

class KDevTeamwork;
class QMenu;
class QModelIndex;
class QWidget;

class SafeTeamworkEmitter;
typedef SharedPtr<SafeTeamworkEmitter> SafeTeamworkEmitterPointer;

class IconCache;

class KDevSystemMessage : public SystemMessage {
    DECLARE_MESSAGE( KDevSystemMessage, SystemMessage, 5 );
  public:
    enum Message {
      CollaborationAccepted,
      CollaborationRefused,
      CollaborationClosed,
      ActionFailed,     ///An Action failed
      ActionSuccessful,
      ActionDenied
  };
    KDevSystemMessage( InArchive& arch, const Teamwork::MessageInfo& info );

    KDevSystemMessage( const Teamwork::MessageInfo& info, Message msg , const string& str = "" );

    Message message();

    QString messageAsString();

    QString text();
};

typedef SafeSharedPtr<KDevSystemMessage> KDevSystemMessagePointer;

struct AbstractGUIMessage {
  QVariant data; ///@todo remove
  virtual QString messageText() const = 0;
  virtual QIcon messageIcon() const;
  virtual void fillContextMenu( QMenu* /*menu*/, KDevTeamwork* /*teamwork*/ ) {}
  ;
  virtual ~AbstractGUIMessage() {}
  ;
  virtual bool show() {
    return true;
  }
  virtual QString shortName() const {
    return "Message";
  }
  ///This should return an icon that represents the message-type and has nothing to do with the message-instance
  virtual QIcon messageTypeIcon() const;

  virtual bool canShowInWidget() {
    return false;
  }

  virtual void showInWidget( QWidget* /*widget*/, KDevTeamwork* /*tw*/ ) {}
};

class KDevTeamworkTextMessage : public TextMessage, public AbstractGUIMessage {
    DECLARE_MESSAGE( KDevTeamworkTextMessage, TextMessage, 6 );
  public:

    KDevTeamworkTextMessage( const Teamwork::MessageInfo& info, const QString& text );

    KDevTeamworkTextMessage( InArchive& from, const Teamwork::MessageInfo& info );

    virtual QString messageText() const;

    virtual QIcon messageIcon() const;

    void setAnswered( bool );

    bool answered() const;
    
    virtual void serialize( OutArchive& arch );

    virtual bool needReply() const;
    
    QDateTime creationTime();

    virtual bool canShowInWidget();

    ///If this returns false, the widget is not shown.
    virtual void showInWidget( QWidget* widget, KDevTeamwork* tw );

  private:
    QDateTime creationTime_;
    bool m_answered;
};

///Just a dummy for compatible insertion of error-messages to the message-list
class FailureMessage : public KDevTeamworkTextMessage {
  public:
  FailureMessage( const QString& text, const MessagePointer& msg );
  
  virtual QIcon messageIcon() const;
};

class ConnectionRequest : public KDevTeamworkTextMessage {
  public:
    enum State {
      Waiting,
      Denied,
      Accepted,
      Unknown
  };

    ConnectionRequest( const Teamwork::MessageInfo& info, const Teamwork::UserPointer& self, const Teamwork::UserPointer& target, const QString& text, KDevTeamwork* teamwork );

    ConnectionRequest( InArchive& arch, const Teamwork::MessageInfo& info );

    virtual void serialize( OutArchive& arch );

    const Teamwork::User& user();

    virtual bool needReply() const;

    virtual QIcon messageIcon() const;

    virtual ReplyResult gotReply( const DispatchableMessage& /*p*/ );

    void setState( State s );

    State state();

    virtual void fillContextMenu( QMenu* menu, KDevTeamwork* teamwork );
  private:
    DECLARE_MESSAGE( ConnectionRequest, KDevTeamworkTextMessage, 1 );
    Teamwork::User user_;
    State m_state;
    SafeTeamworkEmitterPointer m_emitter;
};

BOOST_CLASS_IMPLEMENTATION( NoData, boost::serialization::object_serializable );

EASY_DECLARE_MESSAGE( CollaborationMessage, TextMessage, 9, NoData, 0 );

typedef SafeSharedPtr<CollaborationMessage> CollaborationMessagePointer;

UserPointer userFromSession( const SessionPointer& session );

typedef KDevTeamworkTextMessage KDevTeamworkMessages;

//typedef Teamwork::Binder<Teamwork::AllTeamworkClientMessages, KDevTeamworkMessages >::Append<ConnectionRequest>::Result::Append<KDevSystemMessage>::Result::Append<KDevSystemMessage>::Result AllKDevTeamworkMessages;

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
