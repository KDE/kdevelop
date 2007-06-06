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

#ifndef MESSAGEMANAGER_H
#define MESSAGEMANAGER_H
#include <list>
#include "kdevteamwork_messages.h"
#include "utils.h"
#include "indocumentmessage.h"
#include "autoconstructpointer.h"

class QDomElement;
class KDevTeamwork;
class LocalPatchSource;
class KDevTeamworkTextMessage;
class MessageHistory;
class MessageHistoryManager;
class QAction;
//class LocalPatchSource::Identity;

class KDevTeamworkUser;

typedef SafeSharedPtr<KDevTeamworkUser, BoostSerialization> KDevTeamworkUserPointer;

class ConversationManager;


BIND_LIST_2( MessageManagerMessages, KDevTeamworkTextMessage, InDocumentMessage )

/** This class manages the KDevTeamworkTextMessage-based messages
 */

class MessageManager : public QObject {
    Q_OBJECT
  public:
    MessageManager( KDevTeamwork* tw );
    ~MessageManager();

    void restorePartialProjectSession( const QDomElement* el );

    void savePartialProjectSession( QDomElement* el );

    int receiveMessage( MessageInterface* msg );
    int receiveMessage( KDevTeamworkTextMessage* msg );
    int receiveMessage( InDocumentMessage* msg );

    int processMessage( KDevTeamworkTextMessage* msg );

    KDevTeamwork* teamwork();

    void log( const QString& str, LogLevel level = Info );

    MessageHistoryManager& historyManager();
    ConversationManager& conversationManager();

    void showMessageHistory( const QList<KDevTeamworkUserPointer>& users = QList<KDevTeamworkUserPointer>(), const QString& context = "" );

    void fillMessageMenu( QMenu* menu, MessagePointer msg );

    void showMessage( const MessagePointer& msg );

  public slots:
    ///This can be used to indicate that information about the message(like icon, text, etc.) has been changed.
    void updateMessage( const MessagePointer& msg );
    void slotShowMessage();
    void init();

  signals:
    ///Is emitted whenver a message is reported to have changed.(through updateMessage(..))
    void messageUpdated( const MessagePointer& msg );

  private:
    MessageDispatcher<MessageManager, MessageManagerMessages> dispatcher_;
    KDevTeamwork* m_teamwork;

    AutoConstructPointer<ConversationManager> m_conversationManager;
    AutoConstructPointer<MessageHistoryManager> m_historyManager;

    QAction* m_showMessageAction;
};


#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
