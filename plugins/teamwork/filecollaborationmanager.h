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

#include <QStringList>
#include <QList>

#ifndef FILECOLLABORATIONMANAGER_H
#define FILECOLLABORATIONMANAGER_H

#include "network/messagetypeset.h"
#include "network/safesharedptr.h"
#include "network/weaksafesharedptr.h"
#include "network/crossmap.h"

#include "filecollaborationmessages.h"
#include "safelogger.h"

#include "filecollaboration.h"
#include "filecollaborationsession.h"


///There's not many messages that need to be dispatched right now
BIND_LIST_2( FileCollaborationMessages, FileCollaborationMessage, FileCollaborationRequest );
CROSSMAP_DEFINE_CONTAINER( QList );

class FileCollaborationManager : public QObject, public SafeLogger {
    Q_OBJECT;
  public:
    ///FileCollaborationSession itself is used as key too, so sessions can always be found even if all Files und Users change.
    BIND_LIST_5( SessionKeys, FileCollaborationSessionPointer, CollabSessionId, QList<QString>, QList<KDevTeamworkUserPointer>, SessionName );

    typedef Utils::CrossMap< FileCollaborationSessionPointer, SessionKeys > SessionSet;

    FileCollaborationManager( CollaborationManager* manager );

    const SessionSet& sessions();

    ///May return zero on failure
    FileCollaborationSession* startSession( QString name, CollabFileList files , uint primaryIndex = 0, CollabSessionId id = 0 );

    KDevTeamwork* teamwork();

    bool acceptCollaboration( const FileCollaborationRequestPointer& msg );

    void denyCollaboration( const FileCollaborationRequestPointer& msg );

    void closeSession( const FileCollaborationSessionPointer& session );

  public slots:
    void updateCollaborationModel( QStandardItemModel* model );
    void slotFillCollaboratingUserMenu( QMenu* menu, const UserPointer& user );
    void slotStartCollaborationSession();
    void slotSessionStateChanged( const FileCollaborationSessionPointer& );

  signals:
    void fillCollaboratingUserMenu( QMenu* menu, const UserPointer& user );

  private:

    void processMessage( FileCollaborationMessagePointer msg );

    int receiveMessage( MessageInterface* msg );

    int receiveMessage( FileCollaborationRequest* msg );

    int receiveMessage( FileCollaborationMessage* msg );

    CollaborationManager* m_manager;
    SessionSet m_sessions;
    QAction* m_startCollaborationSessionAction;

    MessageDispatcher< FileCollaborationManager, FileCollaborationMessages > m_dispatcher;
    friend class MessageDispatcher< FileCollaborationManager, FileCollaborationMessages >;
    friend class CollaborationManager;

    typedef std::multimap< CollabSessionId, FileCollaborationRequestPointer> RequestMap;
    RequestMap m_requests; ///Here, requests are stored so all requests for the same session can be grouped together once that session is accepted
};

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
