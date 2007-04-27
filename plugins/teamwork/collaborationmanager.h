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

#ifndef COLLABORATIONMANAGER_H
#define COLLABORATIONMANAGER_H

#include "network/safesharedptr.h"
#include "filecollaborationmessages.h"
#include "kdevteamwork.h"
#include "utils.h"
#include <QObject>
#include <set>
#include "safelogger.h"
#include <QPointer>
#include <memory>


class QPoint;
class QModelIndex;
class QMenu;
class QStandardItemModel;

///These helpers can be used to plug own actions into the tree-structure of the collaboration-manager. To get active, a CollaborationTreeActionPointer must be set as the UserData-entry in the model.
struct CollaborationTreeAction : public Shared {
  virtual void fillContextMenu( QMenu* menu ) = 0;
  virtual void doubleClicked() {
  }
  virtual void clicked() {
  }
};

///This class will call a fillContextMenu-function in the targetet-class
template <class targetet, class ValueType = int>
struct StandardCollaborationTreeAction : public CollaborationTreeAction {
  QPointer<targetet> target;
  ValueType var;

  StandardCollaborationTreeAction( targetet* t, const ValueType& v ) : var( v ), target( t ) {}
  StandardCollaborationTreeAction( targetet* t ) : target( t ) {}

  virtual void fillContextMenu( QMenu* menu ) {
    if ( target ) {
      target->fillContextMenu( var, menu );
    }
  };
};

typedef SharedPtr< CollaborationTreeAction > CollaborationTreeActionPointer;

Q_DECLARE_METATYPE( CollaborationTreeActionPointer );

using namespace Teamwork;
class KDevTeamwork;
class FileCollaborationManager;
class FileCollaborationSession;
typedef SharedPtr<FileCollaborationSession> FileCollaborationSessionPointer;

using namespace std;

class CollaborationManager : public QObject, public SafeLogger {
    Q_OBJECT
  public:
    CollaborationManager( KDevTeamwork* tw );
    ~CollaborationManager();


    void addCollaboratingUser( const UserPointer& u );

    void removeCollaboratingUser( const UserPointer& user );

    bool isCollaborating( const UserPointer& user );

    void updateList();

    void restorePartialProjectSession( const QDomElement* el );

    void savePartialProjectSession( QDomElement* el );

    void fillUserMenu( QMenu* menu, const UserPointer& user );

    FileCollaborationManager& fileCollaborationManager() ;

    QStandardItemModel* developersModel();

    KDevTeamwork* teamwork();
  signals:
    void fillCollaboratingUserMenu( QMenu* menu, const UserPointer& user );
    void updateModel( QStandardItemModel* model );

  public slots:
    void maybeContextMenu();
    void init();
    void collaboratingDeveloperClicked( const QModelIndex& i );
    void collaboratingDeveloperDoubleClicked( const QModelIndex& i );
    void collaboratingDeveloperContextMenu( const QPoint& );
    void uiCloseCollaboration();
    void updateConnection( const TeamworkClientPointer& );
    void processMessage( const CollaborationMessagePointer& );
    void sessionStateChanged( const FileCollaborationSessionPointer& session );

    void uiAcceptCollaboration();
    void uiDenyCollaboration();
    void uiRequestCollaboration();

  private:
    void contextMenu( const QPoint& pos, const QModelIndex& index );
    friend class ConnectionRequest;
    KDevTeamwork* m_teamwork;
    typedef set
      <UserPointer> UserSet;
    UserSet m_collaboratingUsers;
    AutoConstructPointer<FileCollaborationManager> m_fileCollaboration;
    QStandardItemModel* m_developersModel; ///Model for the collaborating-developers tree-view
    auto_ptr<QPersistentModelIndex> m_contextMenuIndex; ///Only used for the timer

    QAction* m_requestCollaborationAction;
    QAction* m_closeCollaborationAction;
    QAction* m_acceptConnectionAction;
    QAction* m_denyConnectionAction;

    QTimer* m_contextMenuTimer;
    bool m_ignoreClick;
};

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
