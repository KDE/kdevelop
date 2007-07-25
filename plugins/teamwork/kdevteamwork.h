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

#ifndef KDEVTEAMWORK_H
#define KDEVTEAMWORK_H

#include <QObject>
#include <QMap>
#include <QMenu>
#include <QDomDocument>
#include <memory>
#include <kiconloader.h>
#include <set>
#include <list>

#include "teamworkfwd.h"
#include "loglevel.h"
#include "network/safesharedptr.h"
#include "network/messageinterface.h"
#include "autoconstructpointer.h"

using namespace Teamwork;
class KUrl;

namespace Ui {
class Teamwork;
}

class QModelIndex;

class KDevTeamwork : public QObject {
    static KDevTeamwork* m_self;
    enum MessageTypes {
      Message = 0,
      SourceMessage = 1
  };

    Q_OBJECT
  public:
    typedef SafeSharedPtr<KDevTeamworkClient> TeamworkClientPointer;
    KDevTeamwork( const KUrl& workspaceDirectory, KDevTeamworkPart *part, QWidget *parent );
    virtual ~KDevTeamwork();

    KDevTeamworkPart *part() const;

    static KDevTeamwork* self() ;

    /**Sets the whole  teamwork-part(client plus server) active/inactive. When set inactive, all connetions are closed.
    on success returns the given activity-state. */
    bool setActive( bool active );
    //
    /**enables/disables the server. */
    bool setServerActive( bool active );

    void showUserInfo( const UserPointer& user );

    ///@todo: make this a signal
    void fillUserMenu( QMenu* menu, const UserPointer& user );

    ///The main fillMessageMenu is the one in the MessageManager. That also invokes this one.
    void fillMessageMenu( QMenu* menu, const MessagePointer& msg );

    ///@todo remove
    Ui::Teamwork& widgets();

    QWidget* widget();

    IconCache& icons();

    void log( const QString& str, LogLevel level = Info );

    void err( const QString& str );

    QIcon iconFromUser( User* user, K3Icon::Group size = K3Icon::Small );

    ///Starts the update-timer for the developer-lists
    void startUpdateTimer();

    TeamworkClientPointer& client();

    PatchesManager* patchesManager();

    CollaborationManager* collaborationManager();

    MessageManager* messageManager();

    void handlePatchesList( PatchesListMessage* msg );
    void getPatchesList( const UserPointer& user );

    LoggerPointer logger();

    ///Inserts the given message into the various GUI-structures. It should be based on AbstractGUIMessage.
    void  addMessageToList( const MessagePointer& smsg );

    virtual void restorePartialProjectSession( const QDomElement* el );

    virtual void savePartialProjectSession( QDomElement* el );

    UserPointer localUser();

    UserIdentity currentUserIdentity();

  public slots:

    /**Prepares the GUI for sending a message to the given user. Second parameter may be zero
     * @param target The user the message should be sent to
     * @param answerTo which message this one is a reply to.
     * */
    void guiSendMessageTo( const UserPointer& target, const MessagePointer& answerTo = MessagePointer() );

    ///This can be used by messages in other threads to send a request that their state(currently only icon) has changed
    void updateMessageInfo( MessagePointer );

  signals:
    ///This is called whenever the local client was destroyed or created, the current client is given as parameter(may be null). Whenever this is emitted, all UserPointers become invalid.
    void updateConnection( TeamworkClientPointer client );
    ///Called once KDevTeamwork is completely constructed
    void init();
    void deInit();

  private slots:
    void documentActivated( KDevelop::IDocument* file );
    void popupContextMenu( const QPoint &pos );
    void allowIncomingChanged( int state );
    void enableCollaborationChanged( int state );
    void connectServer();

    void uiShowPatches();
    void uiUserInfo();

    void uiFilterLog();

    void connectionRequest( ConnectionRequestPointer request );

    void developerClicked( const QModelIndex& index );
    void developerDoubleClicked( const QModelIndex& index );
    void developerContextMenu( const QPoint& );

    void serverClicked( const QModelIndex& index );
    void clientClicked( const QModelIndex& index );
    void messageClicked( const QModelIndex& index );

    void serverDoubleClicked( const QModelIndex& index );
    void clientDoubleClicked( const QModelIndex& index );
    void messageDoubleClicked( const QModelIndex& index );

    void receiveMessage( SafeSharedPtr<KDevSystemMessage> msg );

    void clearLogButton();
    void saveLogButton();

    void messageReplyTimeout();
    void sendMessageButton();
    void messageTypeIndexChanged( const int index );

    void slotClearMessageText();

    void slotClearMessages();
    void slotMessageHistory();

    void disconnectFromServer();
    void disconnectAllServers();

    void sendAnswerMessage();
    void sendMessageToUser();
    void kickClient();
    void banClient();
    void kickAllClients();

    void messageTargetUserChanged();
    void updateAnswerMenu();

    void handleTextMessage( SafeSharedPtr<KDevTeamworkTextMessage> msg );

    void guiUserConnected( Teamwork::UserPointer );
    void guiUserDisconnected( Teamwork::UserPointer );
    void guiServerConnected( Teamwork::ClientSessionDesc, Teamwork::ServerInformation );
    void guiServerDisconnected( Teamwork::ClientSessionDesc, Teamwork::ServerInformation );
    void guiUserList( std::list<UserPointer> );
    void guiLog( QString str, int level ) {
      log( str, ( LogLevel ) level );
    }

    ///Should be called time by time after the state of users changes. This updates all lists that contain users.
    void updateTimeout();

    void messageUpdated( const MessagePointer& msg );

    void maybeDeveloperContextMenu();

    void answerMenuToggled( bool state );
    void answerMenuTriggered( bool );

  private:
    void contextMenu( const QPoint& p, const QModelIndex& index );

    void lockMessageGui( const MessagePointer& msg );
    void unlockMessageGui();

    friend class KDevTeamworkLogger;
    friend class KDevTeamworkClient;

    void addSentMessageToList( const MessagePointer&, QStandardItemModel* model );

    void addReceivedMessageToList( const MessagePointer&, QStandardItemModel* model );

    QAction* sendAnswerAction;
    QAction* sendMessageToUserAction;
    QAction* disconnectFromServerAction;
    QAction* disconnectAllServersAction;

    QAction* kickClientAction;
    QAction* banClientAction;
    QAction* kickAllClientsAction;
    QAction* m_userInfoAction;
    QAction* showPatchesAction;

    UserPointer sendingUser();

    MessagePointer sendingMessage();

    void addSentMessageToList( const MessagePointer& );

    void addReceivedMessageToList( const MessagePointer& );

    void addDeveloper( const Teamwork::UserPointer& u );

    void enableActiveGUI( bool active );

    void enableMessageGUI( bool active );

    void filterLog( int max = 0 );

    void registerPatches( PatchesListMessage* msg, QStandardItemModel* model );

    UserPointer currentMessageTargetUser();

    QMenu* m_messageFilterMenu;

    AutoConstructPointer<TeamworkFolderManager> m_folderManager;

    SafeSharedPtr<KDevTeamworkLogger> m_logger;

    bool m_destroyed;
    KDevTeamworkPart *m_part;
    QWidget* m_widget;
    AutoConstructPointer<Ui::Teamwork> m_widgets;
    bool m_active, m_serverActive;
    TeamworkClientPointer m_client;
    QStandardItemModel* m_logModel;
    QStandardItemModel* m_messagesModel;
    QStandardItemModel* m_clientsModel;
    QStandardItemModel* m_serversModel;
    QStandardItemModel* m_developersModel;

    ///The next message to send is answer to:
    MessagePointer m_answerTo;
    MessagePointer m_waitingForReply; ///This message is blocking the message-gui waiting for a reply
    QTimer* m_replyWaitingTimeout;


    friend class MessageUserTab;

    KDevTeamworkUserPointer currentTabUser();

    typedef QMap<KDevTeamworkUserPointer, SharedPtr<MessageUserTab> > UserTabMap;

    UserTabMap m_userTabs;

    MessagePointer m_actionMessage;

    QTimer* m_updateTimer;
    QTimer* m_updateAnswerTimer; ///Timer for updating the list of possible messages the user can reply to
    AutoConstructPointer<PatchesManager> m_patchesManager;
    AutoConstructPointer<CollaborationManager> m_collaborationManager;
    AutoConstructPointer<MessageManager> m_messageManager;
    AutoConstructPointer<MessageSendManager> m_messageSendManager;

    std::set<UserPointer, UserPointer::ValueSmallerCompare> m_persistentUsers;

    LogLevel m_currentLogFilter;

    AutoConstructPointer<IconCache> m_icons;
    std::auto_ptr<QPersistentModelIndex> m_contextMenuIndex;

    enum MessageTypeIndex {
      NormalMessage,
      DocumentMessage
  };
};

#endif // KDEVCODEVIEW_H

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
