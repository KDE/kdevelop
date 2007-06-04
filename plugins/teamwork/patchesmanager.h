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

#ifndef PATCHESMANAGER_H
#define PATCHESMANAGER_H
//#include <boost/serialization/list.hpp>
#include <list>
#include <string>
#include <QObject>
#include <QMetaType>
#include <QStandardItemModel>
#include <QPointer>

#include "network/serialization.h"
#include "network/safesharedptr.h"
#include "network/messagetypeset.h"
#include "network/easymessage.h"

#include "ui_kdevteamwork_managepatches.h"
#include "kdevteamwork_messages.h"
#include "patchmessage.h"

#include "utils.h"


using namespace std;
using namespace Teamwork;

class EditPatch;
class QDomElement;
class KDevTeamwork;
class KDevTeamworkClient;
class PatchesManager;
typedef SafeSharedPtr<KDevTeamworkClient> TeamworkClientPointer;
typedef SharedPtr<EditPatch> EditPatchPointer;


struct PatchesManagerConfig {
  std::list<LocalPatchSourcePointer> patchSources;

  template <class Archive>
  void serializePatches( Archive& arch, const uint /*version*/ ) {
    arch & NVP( patchSources );
  }

  template <class Archive>
  void serialize( Archive& arch, const uint version ) {
    serializePatches( arch, version );
  }

  bool hasPatch( const LocalPatchSourcePointer& patch ) {
    for ( std::list<LocalPatchSourcePointer>::iterator it = patchSources.begin(); it != patchSources.end(); ++it ) {
      if ( *it == patch )
        return true;
    }
    return false;
  }

  void addPatch( const LocalPatchSourcePointer& patch ) {
    patchSources.push_back( patch );
  }
};

BIND_LIST_4( PatchesManagerMessages, PatchesManagerMessage, PatchesListMessage, PatchRequestMessage, PatchMessage )

class PatchesManager : public QObject {
    Q_OBJECT
  public:
    PatchesManager( KDevTeamwork* tw );
    ~PatchesManager();

    void showPatchInfo( const LocalPatchSourcePointer& patch , bool local = true );

    void restorePartialProjectSession( const QDomElement* el );

    void savePartialProjectSession( QDomElement* el );

    void fillDeveloperActions( const QModelIndex& index, QMenu* menu );

    void addPatch( const LocalPatchSourcePointer& patch );
    bool hasPatch( const LocalPatchSourcePointer& patch );

    ///Tries to find out whether the patch is applied by using the patch-command in dry-run mode. Only  works with patch-files(not commands). Fills the result into the given patch
    LocalPatchSource::State determineState( const LocalPatchSourcePointer& patch );

    ///Tries to merge the patches to one. If it fails, returns zero.
    LocalPatchSourcePointer merge( const QString& name, const QList<LocalPatchSourcePointer>& patches );

    KDevTeamwork* teamwork();

  public slots:
    void processMessage( PatchesManagerMessagePointer );
  private slots:
    void save();
    void load();
    void editDialogClosed( EditPatch* );
    void editStateChanged( EditPatch* );

    void guiUpdatePatchesList();

    void init();
    void slotManagePatches();
    void slotUpdateConnection( TeamworkClientPointer newClient );

    ///The patches-management-menu:
    void slotEditPatch();
    void slotAddPatch();
    void slotRemovePatch();
    void slotCloseManagement();
    void slotManagementFinished( int result );

    ///The developer/patch context-menu:
    void slotShowPatch();
    void slotShowPatchInfo();
    void slotApplyPatch();
    void slotDownloadPatch();

    ///The patch-message context-menu
    void slotShowRequestedPatch();
    void slotDenyPatch();
    void slotAllowPatch();

  private:
    EditPatchPointer showEditDialog( const LocalPatchSourcePointer& patch, bool local );
    ///Actions for the developer/patch-menu:
    QAction* m_showPatchAction;
    QAction* m_showPatchInfoAction;
    QAction* m_downloadPatchAction;
    QAction* m_applyPatchAction;

    LocalPatchSourcePointer::Locked patchFromIdentity( const LocalPatchSource::Identity& identity );

    void log( QString str, LogLevel level = Info );

    LocalPatchSourcePointer selectedPatch();

    ///The edit-dialog:
    LocalPatchSourcePointer patchFromEdit(); ///may return invalid pointer
    void fillEditFromPatch( LocalPatchSourcePointer patch , bool local = true );

    KDevTeamwork* m_teamwork;
    PatchesManagerConfig m_config;

    friend class MessageDispatcher< PatchesManager, PatchesManagerMessages >;
    friend class EditPatch;

    MessageDispatcher< PatchesManager, PatchesManagerMessages > dispatcher_;

    Ui_ManagePatches m_managePatches;
    QPointer<QDialog> m_manageDlg;

    QStandardItemModel* m_patchesModel;

    LocalPatchSourcePointer m_editingPatch;

    void applyPatch( LocalPatchSourcePointer::Locked patch, bool reverse = false );

    int receiveMessage( MessageInterface* msg );
    int receiveMessage( PatchesManagerMessage* msg );
    int receiveMessage( PatchesListMessage* msg );
    int receiveMessage( PatchRequestMessage* msg );
    int receiveMessage( PatchMessage* msg );

    QList<EditPatchPointer> m_editing;

    ///Actions for the PatchRequestMessage-menu:
    QAction* m_denyPatchAction;
    QAction* m_allowPatchAction;
    QAction* m_showRequestedPatchAction;

    QTimer* m_updateTimer;
    friend class PatchRequestData;
};

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
