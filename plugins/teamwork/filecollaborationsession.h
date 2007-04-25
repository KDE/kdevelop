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

#ifndef FILECOLLABORATIONSESSION_H
#define FILECOLLABORATIONSESSION_H

#include <QStringList>
#include <QList>

#include "network/message.h"
#include "network/pointer.h"
#include "network/weakpointer.h"

#include "filecollaborationmessages.h"
#include "network/crossmap.h"
#include "dynamictext/vectortimestamp.h"
#include "safelogger.h"

class FileCollaboration;
typedef SharedPtr<FileCollaboration> FileCollaborationPointer;

NAMED_TYPE( SessionName, QString );

class FileCollaborationSession : public QObject, public WeakShared, public SafeLogger {
    Q_OBJECT;
  public:

    enum State {
      Running,
      Finished
  };

  ///Collaboration the associated user
    BIND_LIST_2( Keys, FileCollaborationPointer, UserPointer );
    ///Wrapper, its index, filename
    BIND_LIST_3( FileKeys, DocumentWrapperPointer, uint, QString );
    BIND_LIST_3( Messages, FileCollaborationMessage, DocumentWrapperMessage, FileListMessage );

    typedef Utils::CrossMap<DocumentWrapperPointer, FileKeys> FileSet;

    FileCollaborationSession( QString name, CollabFileList files, FileCollaborationManager* manager, uint primaryIndex, CollabSessionId id = 0 );

    void inviteUser( const KDevTeamworkUserPointer& user );

    void kickUser( const KDevTeamworkUserPointer& user );

    FileCollaborationManager* manager();

    bool acceptMessage( const FileCollaborationRequestPointer& msg );

    const FileSet& files() const;

    QString name() const;

    QList<KDevTeamworkUserPointer> users() const;

    //void processMessage( const FileCollaborationMessagePointer& msg );
    int processMessage( FileCollaborationMessage* msg );

    CollabFileList fileNames() const;

    QStringList plainFileNames() const;

    QList<uint> plainIds() const;

    CollabSessionId id() const;

    State state() const;

    QIcon icon() const;

    bool isMasterSession() const;

    uint primaryIndex() const;

    void updateTree( QModelIndex& i, QStandardItemModel* model );

    void fillContextMenu( int i, QMenu* menu );

    ///Allocates a new timestamp-index(for a new collaborator)
    uint allocateIndex();

    uint allocateWrapperIndex();

    ///Returns the name of the first online peer-user, or empty string
    QString firstPeerName();

    ///This allows/disallows connected users to add own files to the collaboration
    void setAllowSentDocuments( bool allow );
  signals:
    void userJoined( const KDevTeamworkUserPointer& );
    void userLeft( const KDevTeamworkUserPointer& );
    ///Emitted whenever an important information(file-set, user-set, etc.) changed
    void stateChanged( const FileCollaborationSessionPointer& );
    void documentSetChanged();

  public slots:
    void saveAsFiles();
    void saveAsPatch();
    void disableEditing();
    void enableEditing();
    void removeAllFiles();
    void slotPublishFileList();
    void publishStateChange();
    void addFile();
    void stopSession();
    void closeSession();
    void publishEdit( const VectorTimestamp& state, const SimpleReplacement& replacement, DocumentWrapper* sender );
    void slotFillCollaboratingUserMenu( QMenu* menu, const UserPointer& user );
    void slotConnectionResult( FileCollaboration*, bool ); ///If true, connection was successful. If false, connection failed.
  private slots:
    void removeUserAction();
  private:
    int dispatchMessage( MessageInterface* msg );
    int dispatchMessage( FileListMessage* msg );
    int dispatchMessage( FileCollaborationMessage* msg );
    int dispatchMessage( DocumentWrapperMessage* msg );

    void removeCollaboration( const FileCollaborationPointer& collab );

    QAction* getRemoveUserAction( const UserPointer& user );

    void publishFileRemove( uint id );
    void publishFileList( const CollabFileList& files );

    bool synchronizeFile( const FileCollaborationPointer& collab, const DocumentWrapperPointer& wrapper );

    ///Does not necessarily remove the file, maybe it only disables collaboration on it.
    bool killFile( uint id );
    bool removeFile( uint id );
    uint addFileInternal( const CollabFile& file, bool fromBuffer, bool readFile );

    void aboutToClose();

    virtual std::string logPrefix();

    DocumentWrapperPointer getDocument( QString fileName );

    friend class FileCollaboration;
    friend class FileCollaborationManager;

    ///May throw QString on error
    FileCollaborationPointer newCollaboration( const KDevTeamworkUserPointer& u, const FileCollaborationRequestPointer& request = 0 );


    FileSet m_files;
    typedef Utils::CrossMap
    <FileCollaborationPointer, Keys> CollaborationSet;
    /*UserSet m_ivitedUsers; ///All users that are invited to join
    UserSet m_users; ///All users that are actively collaborating*/
    CollaborationSet m_collaborations;
    FileCollaborationManager* m_manager;
    CollabSessionId m_id;

    bool m_allowSentDocuments;
    bool m_isMasterSession;
    uint m_primaryIndex;
    uint m_indexCount;
    uint m_wrapperIndexCount;
    QString m_name;
    QTimer* m_publishFileListTimer;
    QTimer* m_stateChangeTimer;

    QAction* m_removeUserAction;

    friend class MessageDispatcher< FileCollaborationSession, Messages >;
    MessageDispatcher< FileCollaborationSession, Messages > m_dispatcher;
};

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on

