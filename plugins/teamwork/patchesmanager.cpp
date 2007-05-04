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

#include <QPersistentModelIndex>
#include <QMenu>
#include <QFile>
#include <QTimer>

#include <kmimetype.h>
#include <kmimetypetrader.h>
#include <kopenwithdialog.h>
#include <k3process.h>

#include <idocumentcontroller.h>
#include "iplugincontroller.h"

#include "network/sessioninterface.h"
#include "network/messagetypeset.h"
#include "network/messagesendhelper.h"

#include "kdevteamwork_part.h"
#include "kdevteamwork_user.h"
#include "ui_kdevteamwork_interface.h"
#include "patchesmanager.h"
#include "collaborationmanager.h"
#include "messagemanager.h"
#include "kdevteamwork_client.h"
#include "kdevteamwork_helpers.h"
#include "teamworkfoldermanager.h"
#include "editpatch.h"

#include "serializationutils.h"

QString userNameFromSession( const SessionPointer& session ) {
  UserPointer::Locked lu = userFromSession( session );
  if ( lu ) {
    return ~lu->name();
  } else {
    return "unlockable-user";
  }
}

///Tries to extract the default-extension from a pattern-list. The returned extension includes the dot.
QString getDefaultExtension( const QStringList& patterns ) {
  if ( patterns.isEmpty() )
    return "";
  for ( QStringList::const_iterator it = patterns.begin(); it != patterns.end(); ++it ) {
    QString s = *it;
    int i = s.lastIndexOf( "*." );
    if ( i == -1 )
      continue;
    return s.mid( i + 1 );
  }
  return "";
}


///currently, KOpenWithDlg crashes.
//#define USE_KOPENWITHDLG

REGISTER_MESSAGE( PatchesManagerMessage )
REGISTER_MESSAGE( PatchesListMessage )
EASY_IMPLEMENT_MESSAGE( PatchRequestMessage )
EASY_IMPLEMENT_MESSAGE( PatchMessage )

Q_DECLARE_METATYPE( QPersistentModelIndex )
Q_DECLARE_METATYPE( LocalPatchSourcePointer )
Q_DECLARE_METATYPE( MessagePointer )

PatchesManager::PatchesManager( KDevTeamwork* tw ) : QObject( tw ), m_teamwork( tw ), dispatcher_( *this ), m_manageDlg( 0 ) {

  connect( m_teamwork, SIGNAL( init() ), this, SLOT( init() ) );

  qRegisterMetaType<PatchesManagerMessagePointer>( "PatchesManagerMessagePointer" );
  qRegisterMetaType<LocalPatchSourcePointer>( "LocalPatchSourcePointer" );

  m_showPatchAction = new QAction( "Show", this );
  connect( m_showPatchAction, SIGNAL( triggered() ), this, SLOT( slotShowPatch() ) );
  m_showPatchAction->setToolTip( "Request and show the selected file." );

  m_showPatchInfoAction = new QAction( "Show Info", this );
  connect( m_showPatchInfoAction, SIGNAL( triggered() ), this, SLOT( slotShowPatchInfo() ) );
  m_showPatchInfoAction->setToolTip( "Show Information about the selected file." );

  m_downloadPatchAction = new QAction( "Download Item", this );
  connect( m_downloadPatchAction, SIGNAL( triggered() ), this, SLOT( slotDownloadPatch() ) );
  m_downloadPatchAction->setToolTip( "Download the item and store it into the list of local items." );

  m_applyPatchAction = new QAction( "Apply Locally", this );
  connect( m_applyPatchAction, SIGNAL( triggered() ), this, SLOT( slotApplyPatch() ) );
  m_applyPatchAction->setToolTip( "Apply the item/patch to the local workspace/project and store it into the list of local patches." );

  m_allowPatchAction = new QAction( "Allow", this );
  connect( m_allowPatchAction, SIGNAL( triggered() ), this, SLOT( slotAllowPatch() ) );
  m_allowPatchAction->setToolTip( "Send the requested patch to the user." );

  m_denyPatchAction = new QAction( "Deny", this );
  connect( m_denyPatchAction, SIGNAL( triggered() ), this, SLOT( slotDenyPatch() ) );
  m_denyPatchAction->setToolTip( "Do not send the requested patch to the user." );

  m_showRequestedPatchAction = new QAction( "Show Patch-Info", this );
  connect( m_showRequestedPatchAction, SIGNAL( triggered() ), this, SLOT( slotShowRequestedPatch() ) );
  m_showRequestedPatchAction->setToolTip( "Show the requested patch." );

  m_updateTimer = new QTimer( this );
  m_updateTimer->setSingleShot( true );
  connect( m_updateTimer, SIGNAL(timeout()), this, SLOT(guiUpdatePatchesList() ) );
  connect( m_teamwork, SIGNAL( deInit() ), this, SLOT( save() ) );
}

void PatchesManager::init() {
  connect( m_teamwork->widgets().managePatches, SIGNAL( pressed() ), this, SLOT( slotManagePatches() ) );
  connect( m_teamwork, SIGNAL( updateConnection( TeamworkClientPointer ) ), this, SLOT( slotUpdateConnection( TeamworkClientPointer ) ) );
  load();
}

void PatchesManager::slotManagePatches() {
  m_manageDlg = new QDialog( m_teamwork->widget() );
  m_managePatches.setupUi( m_manageDlg );
  m_managePatches.patchesList->setEditTriggers( QAbstractItemView::NoEditTriggers );
  connect( m_managePatches.edit, SIGNAL( pressed() ), this, SLOT( slotEditPatch() ) );
  connect( m_managePatches.add, SIGNAL( pressed() ), this, SLOT( slotAddPatch() ) );
  connect( m_managePatches.remove, SIGNAL( pressed() ), this, SLOT( slotRemovePatch() ) );
  connect( m_managePatches.closeButton, SIGNAL( pressed() ), this, SLOT( slotCloseManagement() ) );
  m_manageDlg->show();
  m_patchesModel = new QStandardItemModel( 0, 1, m_managePatches.patchesList );
  m_managePatches.patchesList->setModel( m_patchesModel );
  guiUpdatePatchesList();
}

void PatchesManager::slotUpdateConnection( TeamworkClientPointer newClient ) {
  if ( newClient ) {
    log( "slotUpdateConnection", Debug );
    connect( newClient.unsafe(), SIGNAL( signalDispatchMessage( PatchesManagerMessagePointer ) ), this, SLOT( processMessage( PatchesManagerMessagePointer ) ), Qt::QueuedConnection );
  }
}

void PatchesManager::log( QString str, LogLevel level ) {
  m_teamwork->log( "patchesmanager: " + str, level );
}

void PatchesManager::addPatch( const LocalPatchSourcePointer& patch ) {
  if( !hasPatch( patch ) ) {
    m_config.addPatch( patch );
    guiUpdatePatchesList();
  }
}

void PatchesManager::editStateChanged( EditPatch* ) {
  m_updateTimer->start( 100 );
}
    
void PatchesManager::editDialogClosed( EditPatch* dialog ) {
  m_editing.removeAll( dialog );
  m_updateTimer->start( 100 );
}

bool PatchesManager::hasPatch( const LocalPatchSourcePointer& patch ) {
  return m_config.hasPatch( patch );
}

void PatchesManager::slotAddPatch() {
  showEditDialog( new LocalPatchSource(), true );
}

void PatchesManager::showPatchInfo( const LocalPatchSourcePointer & patch, bool local ) {
  showEditDialog( patch, local );
}

EditPatchPointer PatchesManager::showEditDialog(  const LocalPatchSourcePointer & patch, bool local ) {
  if( !patch ) return 0;
  for( QList<EditPatchPointer>::iterator it = m_editing.begin(); it != m_editing.end(); ++it ) {
    if( (*it)->patch() == patch ) return *it;
  }
  EditPatchPointer p = new EditPatch( this, patch, local );
  
  connect( p.data(), SIGNAL(dialogClosed( EditPatch* )), this, SLOT( editDialogClosed( EditPatch* ) ) );
  connect( p.data(), SIGNAL(stateChanged( EditPatch* )), this, SLOT( editStateChanged( EditPatch* ) ) );
  m_editing << p;
  return p;
}

void PatchesManager::slotEditPatch() {
  showEditDialog( selectedPatch(), true );
}

void PatchesManager::slotRemovePatch() {
  LocalPatchSourcePointer p = selectedPatch();

  if ( p )
    m_config.patchSources.remove( p );

  guiUpdatePatchesList();
}

void PatchesManager::slotCloseManagement() {}

void PatchesManager::slotManagementFinished( int /*result*/ ) {
  m_manageDlg = 0;
}

LocalPatchSourcePointer PatchesManager::selectedPatch() {
  if ( !m_manageDlg )
    return LocalPatchSourcePointer();
  QModelIndex i = m_managePatches.patchesList->currentIndex();
  if ( i.isValid() )
    i = m_patchesModel->index( i.row(), 0 );

  if ( i.isValid() ) {
    QVariant v = m_patchesModel->data( i, Qt::UserRole );
    if ( v.canConvert<LocalPatchSourcePointer>() ) {
      return v.value<LocalPatchSourcePointer>();
    } else {
      log( "bad data in patches-list" );
    }
  }

  return LocalPatchSourcePointer();
}

///The following three functions are nearly the same, maybe they should be put together
void PatchesManager::slotApplyPatch() {
  try {
    QAction * act = qobject_cast<QAction*>( sender() );
    if ( !act )
      throw "no action";

    QVariant v = act->data();
    if ( !v.canConvert<LocalPatchSourcePointer>() )
      throw "cannot convert to patch-source";

    LocalPatchSourcePointer::Locked lpatch = v.value<LocalPatchSourcePointer>();
    if ( !lpatch )
      throw "could not lock patch-source";

    UserPointer::Locked user = lpatch->user();
    if ( !user )
      throw "the patch has no associated user";
    if ( !user->online() )
      throw "the user is not online";

    SessionPointer::Locked lsession = user->online().session();
    if ( !lsession )
      throw "the session could not be locked";

    SafeSharedPtr<PatchRequestMessage>::Locked mp = new PatchRequestMessage( globalMessageTypeSet(), lpatch, m_teamwork, PatchRequestData::Apply );
    lsession->send( mp );
    m_teamwork->addMessageToList( ( PatchRequestMessage* ) mp );
  } catch ( const char * str ) {
    log( QString( "error in slotApplyPatch: " ) + str, Error );
  }
}

void PatchesManager::slotDownloadPatch() {
  try {
    QAction * act = qobject_cast<QAction*>( sender() );
    if ( !act )
      throw "no action";

    QVariant v = act->data();
    if ( !v.canConvert<LocalPatchSourcePointer>() )
      throw "cannot convert to patch-source";

    LocalPatchSourcePointer::Locked lpatch = v.value<LocalPatchSourcePointer>();
    if ( !lpatch )
      throw "could not lock patch-source";

    UserPointer::Locked user = lpatch->user();
    if ( !user )
      throw "the patch has no associated user";
    if ( !user->online() )
      throw "the user is not online";

    SessionPointer session = user->online().session();
    if ( !session )
      throw "the session could not be acquired";

    MessagePointer::Locked mp = new PatchRequestMessage( globalMessageTypeSet(), lpatch, m_teamwork, PatchRequestData::Download );
    session.unsafe() ->send( mp );
    m_teamwork->addMessageToList( mp );
  } catch ( const char * str ) {
    log( QString( "error in slotDownloadPatch: " ) + str, Error );
  }
}

void PatchesManager::slotShowPatch() {
  try {
    QAction * act = qobject_cast<QAction*>( sender() );
    if ( !act )
      throw "no action";

    QVariant v = act->data();
    if ( !v.canConvert<LocalPatchSourcePointer>() )
      throw "cannot convert to patch-source";

    LocalPatchSourcePointer::Locked lpatch = v.value<LocalPatchSourcePointer>();
    if ( !lpatch )
      throw "could not lock patch-source";

    UserPointer::Locked user = lpatch->user();
    if ( !user )
      throw "the patch has no associated user";
    if ( !user->online() )
      throw "the user is not online";

    SessionPointer session = user->online().session();
    if ( !session )
      throw "the session could not be acquired";

    MessagePointer::Locked mp = new PatchRequestMessage( globalMessageTypeSet(), lpatch, m_teamwork );
    session.unsafe() ->send( mp );
    m_teamwork->addMessageToList( mp );
  } catch ( const char * str ) {
    log( QString( "error in slotShowPatch: " ) + str, Error );
  }
}

void PatchesManager::slotShowPatchInfo() {
  try {
    QAction * act = qobject_cast<QAction*>( sender() );
    if ( !act )
      throw "no action";

    QVariant v = act->data();
    if ( !v.canConvert<LocalPatchSourcePointer>() )
      throw "cannot convert to patch-source";

    showPatchInfo( v.value<LocalPatchSourcePointer>(), false );
  } catch ( const char * str ) {
    log( QString( "error in slotShowPatchInfo: " ) + str, Error );
  }
}

void PatchesManager::processMessage( PatchesManagerMessagePointer msg ) {
  log( "dispatching message", Debug );
  PatchesManagerMessagePointer::Locked l = msg;
  if ( l ) {
    dispatcher_( l );
  } else {
    log( "PatchesManager could not lock a message-pointer", Error );
  }
}

int PatchesManager::receiveMessage( MessageInterface* msg ) {
  m_teamwork->log( QString( "PatchesManager got unknown message of type " ) + msg->name() );
  return 0;
}

int PatchesManager::receiveMessage( PatchMessage* msg ) {
  try {
    SafeSharedPtr<PatchRequestMessage>::Locked request = msg->info().replyToMessage().cast<PatchRequestMessage>();
    if ( !request )
      throw QString( "got unrequested patch-message, or could not lock patch-request" );

    LocalPatchSourcePointer::Locked patchInfo = request->request(); ///For security, informations like apply-command etc. are cached
    if ( !patchInfo )
      throw QString( "could not get or lock patch-information" );

    LocalPatchSourcePointer::Locked patchInfoNew = msg->patch(); ///For security, informations like apply-command etc. are cached
    if ( !patchInfo )
      throw QString( "could not get or lock patch-information" );

    if ( !( patchInfoNew->identity() == request->patchIdentity() ) )
      throw QString( "identity of received patch does not match the identity of the corresponding request" );

    KMimeType::Ptr mime = KMimeType::mimeType( ~patchInfo->type );
    if ( !mime )
      throw QString( "Error in kdelibs: could not create mime-type" ); ///according to the documentation this should never happen, but it does

    QString userName;
    if( msg->info().session().cast<FakeSession>() )
      userName = "local";
    else
      userName = userNameFromSession( msg->info().session() );
    QString fileName = ~patchInfo->name;
    if( !patchInfo->filename.empty() )
      fileName += "_" + QFileInfo(~patchInfo->filename).fileName();
    else
      fileName += getDefaultExtension( mime->patterns() );

    log( QString( "creating file for patch: %1" ).arg( fileName ), Debug );

    KUrl filePath = TeamworkFolderManager::createUniqueFile( "patches/"+userName, fileName );

    {
      ///@todo make this work with remove files
      QFile file( filePath.path() );
  
      file.open( QIODevice::WriteOnly );
      if ( !file.isOpen() )
        throw QString( "could not open %1" ).arg( filePath.prettyUrl() );
  
      file.write( msg->data() );
      log( QString( "writing patch of size %1 to %2" ).arg( msg->data().size() ).arg( filePath.prettyUrl() ), Debug );
    }
    if ( request->requestType() == PatchRequestData::View )
      TeamworkFolderManager::registerTempItem( filePath );

    switch ( request->requestType() ) {
      case PatchRequestData::Apply: {
        ///Apply the patch to the local tree
        if ( patchFromIdentity( patchInfo->identity() ) )
          throw QString( "there already is another local patch with identity \"%1\"" ).arg( ~patchInfo->identity().desc() );

        //LocalPatchSourcePointer::Locked newPatchInfo( patchInfo );//new LocalPatchSource( *patchInfo ) );
        if( !hasPatch( patchInfo ) ) {
          ///@todo make work with remote files
          patchInfo->setFileName( ~filePath.path() );
          addPatch( patchInfo);
        }

        EditPatch* p = showEditDialog( patchInfo, true );
        if( !p ) throw QString( "cannot edit received patch" );
        ///@todo ...
        p->apply( false, filePath.path() );

        guiUpdatePatchesList();
      };
      break;
      case PatchRequestData::Download: {
        if ( patchFromIdentity( patchInfo->identity() ) )
          throw QString( "there already is another local patch with identity \"%1\"" ).arg( ~patchInfo->identity().desc() );
        ///Store the patch locally
        LocalPatchSourcePointer::Locked newPatchInfo( new LocalPatchSource( *patchInfo ) );
          ///@todo make work with remote files
        newPatchInfo->setFileName( ~filePath.path() );
        m_config.patchSources.push_back( newPatchInfo );
        guiUpdatePatchesList();
      }
      break;
      case PatchRequestData::View: {
        ///Show the patch

        ///@todo where has KDevDiffFrontend gone? When will it be back?
//         if ( patchInfo->type == "text/x-diff" ) {
//           QString str = msg->data();
// 
//           KDevDiffFrontend *df = KDevTeamworkPart::staticCore()->pluginController()->pluginForExtension<KDevDiffFrontend>( "KDevelop/DiffFrontend" );
//           if ( df ) {
//             df->showDiff( str );
//             return 1;
//           } else {
//             log( "no diff-interface available!", Error );
//           }
//         }

        if( !KDevTeamworkPart::staticDocumentController()->openDocument( filePath, KTextEditor::Cursor(), KDevelop::IDocumentController::ActivateOnOpen ) ) {
          log( QString( "could not open %1 with the document-controller" ).arg( filePath.prettyUrl() ), Warning );
          
          auto_ptr<KOpenWithDialog> d( new KOpenWithDialog( ~patchInfo->type, "" ) );

          if ( d->exec() == KDialog::Accepted ) {
            QString app = d->text();
            if ( !app.isEmpty() ) {
              ///@todo How to run this in a better way, so that .desktop-files are accepted etc.?
              K3Process proc;
              proc << app;

              proc << fileName;
              proc.start( K3Process::DontCare );
            } else {
              log( "no application was chosen for opening " + fileName, Warning );
            }
          } else {
            throw QString( "open-with dialog was closed" );
          }
          d.reset(0);
          
/*          ///Open with KRun instead
          if( KRun::runUrl( KUrl(filePath), ~patchInfo->type, KDevTeamworkPart::staticCore()->uiController()->activeMainWindow() ) == 0 ) {
            log( QString( "Failed to open %1 with an application" ).arg( filePath ), Warning );
          }*/
        }
        ///Find an application that is able to open the selected mime-type
        /*
        QString tempFileName;
        KTempFile tempFile( QString(), getDefaultExtension( mime->patterns() ) );

        if ( tempFile.status() != 0 ) {
          throw QString( "could not create temporary file" );
        } else {
          tempFileName = tempFile.name();
          log( "name of the temp-file: " + tempFileName, Debug );
          QDataStream* stream = tempFile.dataStream();
          if ( stream ) {
            *stream << msg->data();
            if ( tempFile.status() != 0 )
              throw QString( "writing to the temporary file failed" );
          } else {
            throw QString( "could not get stream" );
          }
        }

        if ( !mime->isValid() )
          throw "service-type is not valid, file: " + tempFileName;

        QString app;

#ifdef USE_KOPENWITHDLG

        auto_ptr<KOpenWithDlg> d = new KOpenWithDlg( ~patchInfo->type, "" );

        if ( d->exec() == KDialog::Accepted ) {
          app = d->text();
        } else {
          throw QString( "open-with dialog was closed" );
        }
        d = 0;
#else

        KService::Ptr service = KMimeTypeTrader::self() ->preferredService( ~patchInfo->type , "KParts/ReadOnlyPart" );
        if ( !service )
          throw QString( "could not get a service that can handle " + ~patchInfo->type );

        app = service->exec();

#endif

        if ( !app.isEmpty() ) {
          K3Process proc;
          proc << app;

          proc << tempFileName;
          proc.start( K3Process::DontCare );
        } else {
          log( "no application was chosen for opening " + tempFileName, Warning );
      }*/
      }
      break;
    }
  } catch ( const QString & str ) {
    log( QString( "dispatch of patch-message from %1 failed: " ).arg( userFromSession( msg->info().session() ) ) + str , Error );
  }

  return 0;
}

LocalPatchSourcePointer::Locked PatchesManager::patchFromIdentity( const LocalPatchSource::Identity& ident ) {
  for ( list<LocalPatchSourcePointer>::iterator it = m_config.patchSources.begin(); it != m_config.patchSources.end(); ++it ) {
    LocalPatchSourcePointer::Locked l = ( *it );
    if ( l ) {
      if ( ident == l->identity() ) {
        return l;
      }
    } else {
      log( "could not lock patch-source", Warning );
    }
  }
  return 0;
}

LocalPatchSourcePointer PatchesManager::merge( const QString& name, const QList<LocalPatchSourcePointer>& patches ) {
  try {
    LocalPatchSourcePointer::Locked lp = new LocalPatchSource();

    foreach( LocalPatchSourcePointer patch, patches ) {
      LocalPatchSourcePointer::Locked l = patch;
      if ( !l )
        throw "could not lock patch-source";
      if ( l->type != "text/x-diff" )
        throw "a mime-type is not text/x-diff, only that types can be merged";
      if(l->userIdentity) lp->userIdentity = l->userIdentity;
    }


    QString user = "local";
    if( lp->userIdentity )
      user = ~lp->userIdentity.name();

    KUrl file = TeamworkFolderManager::createUniqueFile( "patches/"+user, name+".diff" );

    ///@todo make this work with remove Urls
    QFile target( file.path() );
    if ( !target.open( QIODevice::WriteOnly ) )
      throw QString( "could not open file %1" ).arg( file.prettyUrl() );

    foreach( LocalPatchSourcePointer patch, patches ) {
      LocalPatchSourcePointer::Locked l = patch;
      if ( !l )
        throw "could not lock patch-source";

      
      
      KUrl u = TeamworkFolderManager::teamworkAbsolute( ~l->filename, "patches" );
      ///@todo make this work with remove Urls
      QFile f( u.path() );
      if ( !f.open( QIODevice::ReadOnly ) )
        throw QString( "could not open file %1" ).arg( u.prettyUrl() );

      target.write( f.readAll() );
      target.write( "\n", 1 );
    }

    lp->name = ~name;
    lp->filename = ~TeamworkFolderManager::teamworkRelative( file, "patches" );
    lp->type = "text/x-diff";

    return lp;
  } catch ( const char * str ) {
    log( QString( "error while trying to merge patches: %1" ).arg( str ) , Error );
  } catch ( const QString & str ) {
    log( QString( "error while trying to merge patches: %1" ).arg( str ) , Error );
  }
  return 0;
}


int PatchesManager::receiveMessage( PatchRequestMessage* msg ) {
  ///Send the patch to the target.
  LocalPatchSourcePointer::Locked patch = patchFromIdentity( msg->patchIdentity() );

  bool overrideAccess = false;
  if( msg->info().session().cast<FakeSession>() ) overrideAccess = true;
  
  LocalPatchSourcePointer::Locked lpatch = patch;
  if ( patch && lpatch ) {
    if( !overrideAccess ) {
      switch ( lpatch->access ) {
        case Public:
        break;
        case ConnectedOnly: {
          if ( !m_teamwork->collaborationManager() ->isCollaborating( userFromSession( msg->info().session() ) ) ) {
            log( "not sending patch " + ~patch->name + " to " + userNameFromSession( msg->info().session() ) + " because the user is not collaborating" );
            globalMessageSendHelper().sendReply<KDevSystemMessage>( msg, KDevSystemMessage::ActionFailed, "access only for collaborating users" );
            return 1;
          }
        }
        break;
        case Ask:
        m_teamwork->addMessageToList( msg );
        return 1;
        break;
        default: {
          log( "not sending patch " + ~patch->name + " to " + userNameFromSession( msg->info().session() ) + " because the patch is private" );
          globalMessageSendHelper().sendReply<KDevSystemMessage>( msg, KDevSystemMessage::ActionFailed, "the patch is private" );
          return 1;
        }
        break;
      }
    }
    log( "sending patch " + ~patch->name + " to " + userNameFromSession( msg->info().session() ) );
    globalMessageSendHelper().sendReply<PatchMessage>( msg, ( LocalPatchSourcePointer ) patch, m_teamwork->logger() );
  } else {
    globalMessageSendHelper().sendReply<KDevSystemMessage>( msg, KDevSystemMessage::ActionFailed, "no fitting patch available, or patch could not be locked" );
    log( "got a patch-request, but the requested patch could not be found or could not be locked", Warning );
  }

  return 1;
}

int PatchesManager::receiveMessage( PatchesListMessage* msg ) {
  ///Give the list to the GUI or whoever was waiting for it
  m_teamwork->handlePatchesList( msg );
  return 0;
}

int PatchesManager::receiveMessage( PatchesManagerMessage* msg ) {
  if ( !msg->isDerived() ) {
    switch ( msg->message() ) {
      case PatchesManagerMessage::None:
      break;
      case PatchesManagerMessage::GetPatchesList: {
        SessionPointer::Locked l = msg->info().session();
        if ( l && l->isRunning() ) {
          l->send( new PatchesListMessage( globalMessageTypeSet(), m_config.patchSources ) );

          UserPointer::Locked pl = l->safeUser();
          if ( pl ) {
            log( QString( "sending patches-list to " ) + pl->name().c_str() );
          } else {
            log( "sent patches-list through anonymous session" );
          }

        } else {
          log( QString( "could not answer a patches-list-request" ) );
        }
      }
      break;
    }
  } else {
    log( QString( "could not handle a PatchesManagerMessage of real type " ) + msg->name() );
  }
  return 0;
}

void PatchesManager::guiUpdatePatchesList() {
  if ( !m_manageDlg )
    return ;

  m_patchesModel->clear();
  m_patchesModel->insertColumns( 0, 5 );

  m_patchesModel->setHeaderData( 0, Qt::Horizontal, QString( "Name" ) );
  m_patchesModel->setHeaderData( 1, Qt::Horizontal, QString( "File/Command" ) );
  m_patchesModel->setHeaderData( 2, Qt::Horizontal, QString( "Access" ) );
  m_patchesModel->setHeaderData( 3, Qt::Horizontal, QString( "Type" ) );
  m_patchesModel->setHeaderData( 4, Qt::Horizontal, QString( "State" ) );

  for ( std::list<LocalPatchSourcePointer>::iterator it = m_config.patchSources.begin(); it != m_config.patchSources.end(); ++it ) {
    LocalPatchSourcePointer::Locked l = *it;
    m_patchesModel->insertRow( 0 );
    if ( l ) {
      m_patchesModel->setData( m_patchesModel->index( 0, 0 ), ~l->name, Qt::DisplayRole );
      m_patchesModel->setData( m_patchesModel->index( 0, 1 ), ~( l->filename + l->command ), Qt::DisplayRole );
      m_patchesModel->setData( m_patchesModel->index( 0, 2 ), ~l->accessAsString(), Qt::DisplayRole );
      m_patchesModel->setData( m_patchesModel->index( 0, 3 ), ~l->type, Qt::DisplayRole );
      m_patchesModel->setData( m_patchesModel->index( 0, 4 ), ~l->stateAsString(), Qt::DisplayRole );
    } else {
      m_patchesModel->setData( m_patchesModel->index( 0, 0 ), "lock failed", Qt::DisplayRole );
    }
    QVariant v;
    v.setValue( *it );

    m_patchesModel->setData( m_patchesModel->index( 0, 0 ), v, Qt::UserRole );
  }

  m_patchesModel->sort( 4 );
}

PatchesManager::~PatchesManager() {
}

QIcon LocalPatchSource::getIcon( IconCache& icons ) {
  return icons( "patch" );
}

void PatchesManager::slotShowRequestedPatch() {
  try {
    QAction * act = qobject_cast<QAction*>( sender() );
    if ( !act )
      throw "no action";

    QVariant userVar = act->data();
    if ( !userVar.canConvert<MessagePointer>() )
      throw "cannot convert to message-pointer";

    SafeSharedPtr<PatchRequestMessage>::Locked msg = userVar.value<MessagePointer>().cast<PatchRequestMessage>();
    if ( !msg )
      throw "cannot lock/cast message";

    LocalPatchSourcePointer::Locked lpatch = patchFromIdentity( msg->patchIdentity() );
    if ( !lpatch )
      throw "could not find the requested patch";

    showEditDialog( lpatch, false );

  } catch ( const char * str ) {
    log( QString( "slotShowRequestedPatch failed: " ) + str );
  }
}

void PatchesManager::slotAllowPatch() {
  try {
    QAction * act = qobject_cast<QAction*>( sender() );
    if ( !act )
      throw "no action";

    QVariant userVar = act->data();

    if ( !userVar.canConvert<MessagePointer>() )
      throw "cannot convert to message-pointer";

    SafeSharedPtr<PatchRequestMessage>::Locked msg = userVar.value<MessagePointer>().cast<PatchRequestMessage>();
    if ( !msg )
      throw "cannot lock/cast message";

    LocalPatchSourcePointer::Locked lpatch = patchFromIdentity( msg->patchIdentity() );
    if ( !lpatch )
      throw "could not find the requested patch";

    log( "sending patch " + ~lpatch->name + " to " + userNameFromSession( msg->info().session() ) );
    globalMessageSendHelper().sendReply<PatchMessage>( msg, ( LocalPatchSourcePointer ) lpatch, m_teamwork->logger() );

    msg->setStatus( PatchRequestData::Accepted );

    m_teamwork->messageManager() ->updateMessage( msg.data() );
  } catch ( const char * str ) {
    log( QString( "slotAllowPatch failed: " ) + str );
  }
}


void PatchesManager::slotDenyPatch() {
  try {
    QAction * act = qobject_cast<QAction*>( sender() );
    if ( !act )
      throw "no action";

    QVariant v = act->data();

    if ( v.canConvert<MessagePointer>() )
      throw "cannot convert to message-pointer";

    SafeSharedPtr<PatchRequestMessage>::Locked msg = v.value<MessagePointer>().cast<PatchRequestMessage>();
    if ( !msg )
      throw "cannot lock/cast message";

    log( "denying patch to " + userNameFromSession( msg->info().session() ) );
    globalMessageSendHelper().sendReply<KDevSystemMessage>( msg, KDevSystemMessage::ActionFailed, "access denied" );

    msg->setStatus( PatchRequestData::Denied );

    m_teamwork->messageManager() ->updateMessage( msg.data() );
  } catch ( const char * str ) {
    log( QString( "slotAllowPatch failed: " ) + str );
  }
}

void PatchesManager::restorePartialProjectSession( const QDomElement* /*el*/ ) {
  /*try {
    xmlDeserializeFromElementItem( el, "PatchesManager", NVP( m_config ) );
  } catch ( const QString & str ) {
    log( "could not restore the patch-informations: " + str, Error );
  }*/
}

void PatchesManager::savePartialProjectSession( QDomElement* /*el*/ ) {
  /*try {
    xmlSerializeToElementItem( el, "PatchesManager", NVP( m_config ) );
  } catch ( const QString & str ) {
    log( "could not save the patch-informations: " + str, Error );
  }*/
}

KDevTeamwork* PatchesManager::teamwork() {
  return m_teamwork;
}

void PatchesManager::fillDeveloperActions( const QModelIndex& index, QMenu* menu ) {
  QVariant v = index.model() ->data( index, Qt::UserRole );

  m_showPatchInfoAction->setData( v );
  m_showPatchAction->setData( v );
  m_downloadPatchAction->setData( v );
  m_applyPatchAction->setData( v );

  menu->addAction( m_showPatchInfoAction );
  menu->addAction( m_showPatchAction );
  menu->addAction( m_downloadPatchAction );
  menu->addAction( m_applyPatchAction );
}

void PatchRequestData::fillContextMenu( QMenu* menu, KDevTeamwork* teamwork ) {
  QVariant v;

  v.setValue( MessagePointer( selfMessage() ) );
  teamwork->patchesManager() ->m_denyPatchAction->setData( v );
  menu->addAction( teamwork->patchesManager() ->m_denyPatchAction );

  teamwork->patchesManager() ->m_allowPatchAction->setData( v );
  menu->addAction( teamwork->patchesManager() ->m_allowPatchAction );

  teamwork->patchesManager() ->m_showRequestedPatchAction->setData( v );
  menu->addAction( teamwork->patchesManager() ->m_showRequestedPatchAction );
}

QStringList splitArgs( const QString& str );

LocalPatchSource::State PatchesManager::determineState( const LocalPatchSourcePointer& patch ) {
  LocalPatchSourcePointer::Locked lpatch = patch;
  if( !lpatch ) {
    log( "determineState(..) could not lock patch", Error );
    return LocalPatchSource::Unknown;
  }
  try {
    if( lpatch->type != "text/x-diff" )
      throw "state can only be determined for files of type \"text/x-diff\"";
    if( lpatch->filename.empty() )
        throw "state can only be determined for file-patches";
    
/*    KUrl fileUrl = projectDir();
    if( (~lpatch->filename).startsWith( fileUrl.path() ) )
      fileUrl = KUrl( ~lpatch->filename );
    else
    fileUrl.addPath( ~lpatch->filename );*/
    KUrl fileUrl = ~lpatch->filename;

    if( lpatch->patchTool() != "patch" || lpatch->patchTool(true) != "patch" ) throw QString( "cannot determine state with other tool than patch: \"%1\" \"%2\"").arg(~lpatch->patchTool()).arg(~lpatch->patchTool(true)) ;
    
    {
      K3Process proc;
      ///@todo does not work with remove directories
      proc.setWorkingDirectory( TeamworkFolderManager::workspaceDirectory().path() );
     // proc << ~lpatch->patchTool();
      bool hadFile = false;
      QString applyParams = ~lpatch->patchParams(false);
      if( applyParams.contains( "$FILE" ) )
        hadFile = true;
        //applyParams.replace( "$FILE", fileUrl.path() );
      proc.setEnvironment( "FILE", fileUrl.path() );
      //proc << "--dry-run" << "-s" << "-f";
      QString cmd = ~lpatch->patchTool() + " --dry-run " + applyParams + " -s -f";
      if( !hadFile ) {
        //proc << "-i" << fileUrl.path();
        cmd += " -i " + fileUrl.path();
      }
      proc << splitArgs( cmd );
      
      log( "determineState(...) calling " + cmd, Debug );

      if( !proc.start( K3Process::Block ) ) throw "could not start process";
      if( !proc.normalExit() ) throw "process did not exit normally";
      log( QString( "exit-status: %1").arg( proc.exitStatus() ), Debug );
      if( proc.exitStatus() == 0 ) {
        lpatch->state = LocalPatchSource::NotApplied;
        return LocalPatchSource::NotApplied;
      }
    }

    {
      K3Process proc;
      ///@todo does not work with remove directories
      proc.setWorkingDirectory( TeamworkFolderManager::workspaceDirectory().path() );
      //proc << ~lpatch->patchTool(true);
      
      bool hadFile = false;
      QString applyParams = ~lpatch->patchParams(true);
      if( applyParams.contains( "$FILE" ) ) {
        hadFile = true;
      }
      proc.setEnvironment( "FILE", fileUrl.path() );

      //proc << "--dry-run" << "-s" << "-f" << applyParams;
      QString cmd = ~lpatch->patchTool(true) + " --dry-run -s -f " + applyParams;
      if( !hadFile ) {
        //proc << "-i" << fileUrl.path();
        cmd += " -i " + fileUrl.path();
      }
      proc << splitArgs( cmd );
      log( "determineState(...) calling " + cmd, Debug );
    
      if( !proc.start( K3Process::Block ) ) throw "could not start patch-process";
      if( !proc.normalExit() ) throw "process did not exit normally";
      log( QString( "exit-status: %1").arg( proc.exitStatus() ), Debug );
      if( proc.exitStatus() == 0 ) {
        lpatch->state = LocalPatchSource::Applied;
        return LocalPatchSource::Applied;
      }
    }
  } catch( const QString& str ) {
    log( "Error in determineState: " + str, Error );
  } catch( const char* str ) {
    log( QString("Error in determineState: ") + str, Error );
  }
  lpatch->state = LocalPatchSource::Unknown;
  return LocalPatchSource::Unknown;
}

void PatchesManager::save() {
  try {
    KUrl fileName = TeamworkFolderManager::teamworkAbsolute( "patches.database" );
    ///@todo does not work with remote files
    if( !fileName.isLocalFile() ) throw QString( "file is not a local Url: %1" ).arg( fileName.prettyUrl() );
    
    std::ofstream file(fileName.path().toLocal8Bit(), ios_base::out | ios_base::binary );
    if( !file.good() ) throw "could not open " + fileName.prettyUrl() + " for writing";
    boost::archive::polymorphic_xml_oarchive arch( file );
    arch << NVP(m_config);
  } catch ( std::exception & exc ) {
    log( QString("save(): exception occured while serialization: %1").arg( exc.what() ), Error );
  } catch( const char* str ) {
    log( QString("save(): %1").arg( str ), Error );
  } catch( const QString& str ) {
    log( QString( "save(): %1").arg( str ), Error );
  }
}

void PatchesManager::load() {
  try {
    KUrl fileName = TeamworkFolderManager::teamworkAbsolute( "patches.database" );
    ///@todo does not work with remote files
    if( !fileName.isLocalFile() ) throw QString( "file is not a local Url: %1" ).arg( fileName.prettyUrl() );
    
    std::ifstream file(fileName.path().toLocal8Bit(), ios_base::binary );
    if( !file.good() ) throw "could not open " + fileName.prettyUrl() + " for reading";
    boost::archive::polymorphic_xml_iarchive arch( file );
    arch >> NVP(m_config);
  } catch ( std::exception & exc ) {
    log( QString("load(): exception occured while serialization: %1").arg( exc.what() ), Error );
  } catch( const char* str ) {
    log( QString("load(): %1").arg( str ), Error );
  } catch( const QString& str ) {
    log( QString( "load(): %1").arg( str ), Error );
  }
  guiUpdatePatchesList();
}

#include "patchesmanager.moc"

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
