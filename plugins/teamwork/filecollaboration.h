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

#ifndef FILECOLLABORATION_H
#define FILECOLLABORATION_H

#include <QStringList>
#include <QList>
#include <QIcon>

#include "network/messagetypeset.h"
#include "network/safesharedptr.h"
#include "network/weaksafesharedptr.h"

#include "filecollaborationmessages.h"
#include "collaborationmanager.h"
#include "network/crossmap.h"
#include "safelogger.h"
#include "verify.h"

namespace KTextEditor {
class Document;
}

class FileCollaborationManager;
class KDevTeamworkUser;
class CollaborationManager;

class FileCollaborationManager;
class FileCollaborationSession;
class QAction;
class DocumentWrapper;
class QModelIndex;
class QStandardItemModel;

typedef SafeSharedPtr<KDevTeamworkUser, BoostSerialization> KDevTeamworkUserPointer;
typedef SharedPtr<FileCollaborationSession> FileCollaborationSessionPointer;
typedef SharedPtr<DocumentWrapper> DocumentWrapperPointer;

// BIND_LIST_2( FileCollaborationDirectMessages, FileCollaborationRequest, FileCollaborationMessage )

class FileCollaborationUserTreeAction;

class FileCollaboration : public QObject, public Shared, public SafeLogger {
  Q_OBJECT
  public:

    ///May throw QString
    FileCollaboration( KDevTeamworkUserPointer user, FileCollaborationSession* session, const FileCollaborationRequestPointer& request = 0 );

    KDevTeamworkUserPointer user() const;

    FileCollaborationSessionPointer session() const;

    ///This collaboration's index in the vector-timestamps
    uint primaryIndex() const;

    ///Returns whether the other side was invited to join the session by this side
    bool invited() const;

    ///@return whether the connection is active
    bool connectionActive() const;

    void close( const QString& reason );

    QIcon icon() const;

  signals:
    void connectionResult( FileCollaboration*, bool ); ///If true, connection was successful. If false, connection failed.

  public slots:
    void slotFillCollaboratingUserMenu( QMenu* menu, const UserPointer& user );
    void processMessage( const MessagePointer& msg );
    void slotConnectionResult( bool );
    void userStateChanged();

  private:
    void updateTree( QModelIndex& i, QStandardItemModel* model );

    void invite( const FileCollaborationRequestPointer& request, bool automatic ) throw(QString);
    void doubleClicked();
    void fillContextMenu( int i, QMenu* menu );

    friend class FileCollaborationUserTreeAction;
    friend class FileCollaborationSession;

//     int receiveMessage( MessageInterface* msg );
//     int receiveMessage( FileSynchronize* msg );
//     int receiveMessage( FileCollaborationRequest* msg );
//     int receiveMessage( FileCollaborationMessage* msg );

    KDevTeamworkUserPointer m_user;
    FileCollaborationSessionPointer m_session;
    FileCollaborationRequestPointer m_request;

    QString m_userName;
    uint m_index; ///The index of the other side this collaboration-object is a bridge to

    bool m_connectionActive;
    bool m_userConnected;
    bool m_invited;

    virtual std::string logPrefix();

/*    friend class MessageDispatcher< FileCollaboration, FileCollaborationDirectMessages >;
    MessageDispatcher< FileCollaboration, FileCollaborationDirectMessages > m_dispatcher;*/
};

typedef SharedPtr<FileCollaboration> FileCollaborationPointer;

struct FileCollaborationUserTreeAction : public StandardCollaborationTreeAction<FileCollaboration> {
  FileCollaborationUserTreeAction( FileCollaboration* w ) : StandardCollaborationTreeAction<FileCollaboration>( w ) {}
  virtual void doubleClicked();

  virtual void fillContextMenu( QMenu* menu );
}
;

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
