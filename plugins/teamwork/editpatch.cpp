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

#include "editpatch.h"

#include <kmimetype.h>
#include <klineedit.h>
#include <kmimetypechooser.h>
#include <kmimetypetrader.h>
#include <k3process.h>
#include <krandom.h>
#include <QTabWidget>
#include <QMenu>
#include <QFile>
#include <QTimer>
#include <QMutexLocker>
#include <QPersistentModelIndex>
//#include <lib/kdevappintterface.h>
#include "patchesmanager.h"
#include <kfiledialog.h>
#include "kdevteamwork.h"
#include <idocument.h>
//#include "kdevdiffinterface.h"
#include "kdevteamwork_part.h"
#include "collaborationmanager.h"
#include <QStandardItemModel>
#include <icore.h>
#include "serializationutils.h"
#include "kdevteamwork_user.h"
#include "messagemanager.h"
#include "network/messagetypeset.h"
#include "kdevteamwork_helpers.h"
#include "teamworkfoldermanager.h"
#include <kde_terminal_interface.h>
#include <kparts/part.h>
#include <kparts/factory.h>
#include <kdialog.h>

#include "libdiff2/komparemodellist.h"
#include "libdiff2/kompare.h"
#include <kmessagebox.h>
#include <QMetaType>
#include <QVariant>
#include "libdiff2/diffsettings.h"
#include <ktexteditor/cursor.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <ktexteditor/markinterface.h>
#include <ktexteditor/smartinterface.h>
#include "idocumentcontroller.h"

#include "kdevteamwork_client.h"

/* Exclude this file from doublequote_chars check as krazy doesn't understand
std::string*/
//krazy:excludeall=doublequote_chars

///Whether arbitrary exceptions that occurred while diff-parsing within the library should be caught
#define CATCHLIBDIFF

using namespace KDevelop;

QString getDefaultExtension( const QStringList& patterns );

Q_DECLARE_METATYPE( const Diff2::DiffModel* )

EditPatch::EditPatch( PatchesManager* parent, LocalPatchSourcePointer patch, bool local ) : QObject( parent ), SafeLogger( parent->teamwork() ->logger(), "EditPatch: " ), m_actionState( LocalPatchSource::Unknown ), m_parent( parent ), m_editingPatch( patch ), m_editPatchLocal( local ), m_editDlg( 0 ), m_reversed( false ), m_started( false ), m_isSource( false ) {
  m_updateKompareTimer = new QTimer( this );
  m_updateKompareTimer->setSingleShot( true );
  connect( m_updateKompareTimer, SIGNAL( timeout() ), this, SLOT( updateKompareModel() ) );
  showEditDialog();
  if ( m_editingPatch )
    fillEditFromPatch();
  m_started = true;
}

EditPatch::~EditPatch() {
  removeHighlighting();
}

void EditPatch::slotEditOk() {

  LocalPatchSourcePointer p = patchFromEdit();

  if ( p && !m_parent->hasPatch( p ) )
    m_parent->addPatch( p );
  //emit dialogClosed( this );
  emit stateChanged( this );
}

void EditPatch::slotEditCancel() {
  //emit dialogClosed( this );
}

LocalPatchSourcePointer EditPatch::patchFromEdit() {
  if ( !m_editDlg )
    return 0;
  LocalPatchSourcePointer::Locked ps;

  if ( m_editingPatch )
    ps = m_editingPatch;
  else {
    ps = new LocalPatchSource();
    ps->userIdentity = m_parent->teamwork() ->currentUserIdentity();
  }

  if ( !ps ) {
    err() << "could not lock edited patch";
    return 0;
  }

  LocalPatchSource& ls( *ps );
  ls.name = ~m_editPatch.name->text();
  ls.unApplyCommand = ~m_editPatch.unapplyCommand->text();
  ls.type = ~m_editPatch.type->text();
  if( !m_editPatch.filename->url().toLocalFile().isEmpty() )
    ls.filename = ~TeamworkFolderManager::teamworkRelative( ( m_editPatch.filename->url() ) );
  else
    ls.filename = "";
  ls.description = ~m_editPatch.description->toPlainText();
  ls.dependencies = ~m_editPatch.dependencies->text();
  ls.command = ~m_editPatch.command->text();
  ls.applyCommand = ~m_editPatch.applyCommand->text();
  ls.state = editState();
  ls.author = ~m_editPatch.author->text();
  QString txt = m_editPatch.accessRights->currentText();

  ls.access = LocalPatchSource::accessFromString( ~txt );

  m_editingPatch = ps;
  return ps;
}

void EditPatch::fillEditFromPatch() {
  out( Logger::Debug ) << "filling edit from patch";

  LocalPatchSourcePointer _patch = m_editingPatch;
  bool local = m_editPatchLocal;
  LocalPatchSourcePointer::Locked l = _patch;
  if ( !m_editDlg || !_patch || !l )
    return ;
  LocalPatchSource& patch( *l );
  m_editPatchLocal = local;

  m_editPatch.name->setText( ~patch.name );
  m_editPatch.unapplyCommand->setText( ~patch.unApplyCommand );
  m_editPatch.type->setText( ~patch.type );
  if( !patch.filename.empty() )
    m_editPatch.filename->setUrl( TeamworkFolderManager::teamworkAbsolute( ~patch.filename ) );
  else
    m_editPatch.filename->setUrl( KUrl() );
  m_editPatch.description->setText( ~patch.description );
  m_editPatch.dependencies->setText( ~patch.dependencies );
  m_editPatch.command->setText( ~patch.command );
  m_editPatch.applyCommand->setText( ~patch.applyCommand );
  m_editPatch.author->setText( ~patch.author );

  int stateIndex = 0;

  switch ( patch.state ) {
    case LocalPatchSource::Applied:
    stateIndex = 1;
    break;
    case LocalPatchSource::NotApplied:
    stateIndex = 2;
    break;
    default:
    stateIndex = 0;
  }

  m_editPatch.state->setCurrentIndex( stateIndex );
  slotStateChanged();

  if ( !patch.filename.empty() )
    m_editPatch.tabWidget->setCurrentIndex( m_editPatch.tabWidget->indexOf( m_editPatch.fileTab ) );
  else
    m_editPatch.tabWidget->setCurrentIndex( m_editPatch.tabWidget->indexOf( m_editPatch.commandTab ) );

  m_editPatch.accessRights->setEditText( ~patch.accessAsString() );

  UserIdentity id = l->userIdentity;
  if ( id ) {
    m_editPatch.userButton->setEnabled( true );
    m_editPatch.userButton->setText( ~id.name() );
  } else {
    m_editPatch.userButton->setEnabled( false );
  }

  updateByType();
  m_updateKompareTimer->start( 400 );
}

void EditPatch::slotFileNameEdited() {
  m_updateKompareTimer->start( 100 );
}

void EditPatch::slotCommandEdited() {
  m_updateKompareTimer->start( 100 );
}

void EditPatch::slotEditMimeType( const QString& /*str*/ ) {
  if ( !m_started )
    return ;
  m_editingPatch = patchFromEdit();
  updateByType();
  m_updateKompareTimer->start( 100 );
}

void EditPatch::slotEditCommandChanged( const QString& /*str*/ ) {
  if ( !m_started )
    return ;
  m_editPatch.filename->setUrl( QString( "" ) );
  emit stateChanged( this );

  //m_updateKompareTimer->start( 100 );
}

void EditPatch::slotUserButton() {
  LocalPatchSourcePointer::Locked patch = patchFromEdit();
  try {
    if ( !patch )
      throw "could not lock patch";
    UserPointer u;
    {
      TeamworkClientPointer::Locked client = m_parent->teamwork() ->client();
      if ( !client )
        throw "no client";
      u = client->findUser( patch->userIdentity );
    }
    if ( !u )
      throw "could not get user";
    if ( !u.cast<KDevTeamworkUser>() )
      throw "user has wrong type";

    m_parent->teamwork() ->showUserInfo( u.cast<KDevTeamworkUser>() );
  } catch ( const QString & str ) {
    err() << "slotUserButton():" << str;
  } catch ( const char * str ) {
    err() << "slotUserButton():" << str;
  }
}

void EditPatch::slotApplyEditPatch() {
  LocalPatchSourcePointer patch = patchFromEdit();
  if ( m_editPatchLocal ) {
    LocalPatchSourcePointer::Locked l = m_editingPatch;
    if ( !m_parent->hasPatch( patch ) )
      m_parent->addPatch( patch );

    if ( l && l->filename.empty() ) {
      ///The patch has to be serialized first before it can be applied

      SafeSharedPtr<PatchMessage>::Locked l = getPatchMessage( PatchRequestData::Apply );
      if ( !l )
        throw "could not get the patch-message";
      m_parent->processMessage( l.data() );
    } else {
      apply();
      fillEditFromPatch();
    }
  } else {
    ///Request the patch from the peer and apply it
    try {
      LocalPatchSourcePointer::Locked lpatch = patch;
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

      MessagePointer::Locked mp = new PatchRequestMessage( globalMessageTypeSet(), lpatch, m_parent->teamwork(), PatchRequestData::Apply );
      session.unsafe() ->send( mp );
      m_parent->teamwork() ->addMessageToList( mp );
    } catch ( const char * str ) {
      err() << "error in slotApplyEditPatch:" << str;
    }
  }
  emit stateChanged( this );
}

void EditPatch::slotUnapplyEditPatch() {
  if ( !m_editPatchLocal )
    return ;

  LocalPatchSourcePointer patch = patchFromEdit();
  if ( !m_parent->hasPatch( patch ) )
    m_parent->addPatch( patch );
  LocalPatchSourcePointer::Locked l = patch;
  if ( l ) {
    if ( !l->command.empty() ) {
      int result = KMessageBox::warningContinueCancel( m_editDlg, QString( "You are about to unapply a dynamic patch created by the command \"%1\". That can be very dangerous. If the same command will not produce the same text after the unapply, the patch cannot be reapplied and all data may be lost. \nRecommendation: Make the patch a file-patch before unapplying it." ), "Warning" );
      if ( result != KMessageBox::Continue )
        return ;
    }
  }
  apply( true );
  fillEditFromPatch();

  emit stateChanged( this );
}

SafeSharedPtr<PatchMessage> EditPatch::getPatchMessage( PatchRequestData::RequestType /*type*/ ) {
  try {
    LocalPatchSourcePointer::Locked lpatch = patchFromEdit();
    if ( !lpatch )
      throw "could not lock patch";

    UserPointer::Locked fakeUser( new KDevTeamworkUser( UserPointer::Locked( new User( "local" ) ) ) );
    SafeSharedPtr<Teamwork::FakeSession>::Locked fakeSession( new Teamwork::FakeSession( fakeUser, m_parent->teamwork() ->logger(), globalMessageTypeSet(), 0 ) );
    fakeUser->setSession( ( SessionInterface* ) fakeSession );

    SafeSharedPtr<PatchRequestMessage>::Locked request = new PatchRequestMessage( globalMessageTypeSet(), lpatch, m_parent->teamwork() );

    request->info().setSession( ( SessionInterface* ) fakeSession );

    m_parent->processMessage( request.data() );

    MessagePointer::Locked msg = fakeSession->getFirstMessage();
    if ( !msg.cast<PatchMessage>() ) {
      if ( KDevSystemMessagePointer::Locked l = msg.cast<KDevSystemMessage>() ) {
        throw QString( "Requesting the message from self failed: %1: %2" ).arg( l->messageAsString() ).arg( l->text() );
      } else {
        throw QString( "answer-message is not of type PatchMessage: " ) + ( msg ? msg->name() : "" );
      }
    }

    return msg.cast<PatchMessage>().data();
  } catch ( const char * str ) {
    err() << "getPatchMessage: error:" << str;
  } catch ( const QString & str ) {
    err() << "getPatchMessage: error:" << str;
  } catch ( const TeamworkError & error ) {
    err() << "getPatchMessage: failed to serialize and deserialize the patch locally:" << error.what();
  }
  return 0;
}

KUrl EditPatch::getPatchFile( bool temp ) {
  try {
    LocalPatchSourcePointer patch = patchFromEdit();
    if ( !m_parent->hasPatch( patch ) )
      m_parent->addPatch( patch );
    SafeSharedPtr<PatchMessage> smsg = getPatchMessage( PatchRequestData::View );
    SafeSharedPtr<PatchMessage>::Locked msg = smsg;
    if ( !msg )
      throw QString( "could not serialize patch-message" );
    LocalPatchSourcePointer::Locked patchInfo = m_editingPatch;
    if ( !patchInfo )
      throw QString( "could not lock patchi-info" );

    QString mimeName = ~patchInfo->type;
    if ( mimeName.trimmed().isEmpty() )
      mimeName = "text/x-diff";

    KMimeType::Ptr mime = KMimeType::mimeType( mimeName );
    if ( !mime )
      throw QString( "Error in kdelibs: could not create mime-type" ); ///according to the documentation this should never happen, but it does

    QString fileName = ~patchInfo->name;
    if ( !patchInfo->filename.empty() )
      fileName += "_" + QFileInfo( ~patchInfo->filename ).fileName();
    else
      fileName += getDefaultExtension( mime->patterns() );

    out( Logger::Debug ) << "getPatchFile() creating file for patch:" << fileName;

    QString subFolder = "local";
    if ( temp )
      subFolder = "temp";

    KUrl filePath = TeamworkFolderManager::createUniqueFile( "patches/" + subFolder, fileName );
    if ( temp )
      TeamworkFolderManager::registerTempItem( filePath );

    {
      ///@todo use NetAccess
      QFile file( filePath.toLocalFile() );

      file.open( QIODevice::WriteOnly );
      if ( !file.isOpen() )
        throw QString( "could not open %1" ).arg( filePath.toLocalFile() );

      if ( file.write( msg->data() ) != msg->data().size() )
        throw "writing the file " + filePath.toLocalFile() + " failed";
    }
    return filePath;
  } catch ( const QString & str ) {
    err() << "getPatchFile(" << temp << ") failed:" << str;
    return QString();
  } catch ( const char * str ) {
    err() << "getPatchFile(" << temp << ") failed:" << str;
    return QString();
  }
}
void EditPatch::slotShowEditPatch() {
  LocalPatchSourcePointer patch = patchFromEdit();
  if ( !m_parent->hasPatch( patch ) )
    m_parent->addPatch( patch );
  try {

    if ( m_editPatchLocal ) {
      SafeSharedPtr<PatchMessage>::Locked l = getPatchMessage( PatchRequestData::View );
      if ( !l )
        throw "could not get the patch-message";
      m_parent->processMessage( l.data() );
    } else {
      ///Request the patch from the peer and show it
      LocalPatchSourcePointer::Locked lpatch = patch;
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

      MessagePointer::Locked mp = new PatchRequestMessage( globalMessageTypeSet(), lpatch, m_parent->teamwork() );
      session.unsafe() ->send( mp );
      m_parent->teamwork() ->addMessageToList( mp );
    }
  } catch ( const QString & str ) {
    err() << "error in slotShowEditPatch:" << str;
  } catch ( const char * str ) {
    err() << "error in slotShowEditPatch:" << str;
  }
}

void EditPatch::slotStateChanged() {
  if ( !m_started )
    return ;
  if ( !m_editDlg )
    return ;
  patchFromEdit();

  switch ( editState() ) {
    case LocalPatchSource::Unknown:
    m_editPatch.applyButton->setEnabled( true );
    m_editPatch.unApplyButton->setEnabled( true );
    break;
    case LocalPatchSource::Applied:
    m_editPatch.applyButton->setEnabled( false );
    m_editPatch.unApplyButton->setEnabled( true );
    break;
    case LocalPatchSource::NotApplied:
    m_editPatch.applyButton->setEnabled( true );
    m_editPatch.unApplyButton->setEnabled( false );
    break;
  }

  if ( !m_editPatchLocal ) {
    m_editPatch.unApplyButton->setEnabled( false );
  }

  m_updateKompareTimer->start( 100 );

  updateByType();
  emit stateChanged( this );
}

void EditPatch::slotEditFileNameChanged( const QString& str ) {
  if ( !m_editDlg || !m_started )
    return ;
  QString txt;

  KSharedPtr<KMimeType> mt = KMimeType::findByUrl( KUrl( str ) );

  if ( !str.isEmpty() ) {
    txt = "mime-type: " + mt->name();
    m_editPatch.type->setText( mt->name() );
  }

  m_editPatch.mimetype->setPixmap( m_parent->teamwork() ->icons().getIcon( mt->iconName(), K3Icon::Desktop ).pixmap( 16, 16 ) );

  m_editPatch.mimetype->setText( txt );
  m_editPatch.command->setText( "" );
  emit stateChanged( this );

  //m_updateKompareTimer->start( 100 );
}

void EditPatch::slotEditDialogFinished( int /*result*/ ) {
  if ( !m_started )
    return ;
  LocalPatchSourcePointer patch = patchFromEdit();

  m_editDlg = 0;
  emit dialogClosed( this );
}

void EditPatch::slotChooseType() {
  if ( !m_started )
    return ;
  if ( !m_editDlg )
    return ;

  QString text = "Select the MimeType for this item.";
  QStringList list;
  list << m_editPatch.type->text();
  KMimeTypeChooserDialog *d = new KMimeTypeChooserDialog( text, "", list , "text", QStringList(), KMimeTypeChooser::Comments | KMimeTypeChooser::Patterns | KMimeTypeChooser::EditButton, m_editDlg );
  if ( d->exec() == QDialog::Accepted ) {
    if ( !d->chooser() ->mimeTypes().isEmpty() ) {
      ///It would be perfect to have a mime-type-chooser that allows only one choice.
      if ( d->chooser() ->mimeTypes().back() == m_editPatch.type->text() ) {
        m_editPatch.type->setText( d->chooser() ->mimeTypes().front() );
      } else {
        m_editPatch.type->setText( d->chooser() ->mimeTypes().back() );
      }
    }
  }

  delete d;
  updateByType();
}

std::string EditPatch::logPrefix() {
  LocalPatchSourcePointer::Locked l = m_editingPatch;
  if ( !l )
    return "EditPatch: ";
  else
    return "EditPatch(" + l->name + "): ";
}

void EditPatch::slotToFile() {
  LocalPatchSourcePointer::Locked l = patchFromEdit();
  if ( !l ) {
    err() << "slotToFile() could not lock edited patch";
    return ;
  }
  KUrl f = getPatchFile();
  if ( f.isEmpty() ) {
    err() << "slotToFile() could not get create patch-file";
    return ;
  }
  out( Logger::Debug ) << "converting patch with command \"" << l->command << "\" to file \"" << f.prettyUrl() << "\"";
  if( !f.isEmpty() )
    l->setFileName( ~TeamworkFolderManager::teamworkRelative( f ) );
  else
    l->setFileName( "" );

  m_editPatch.tabWidget->setCurrentIndex( 0 );
  m_editPatch.filename->setUrl( f );
  fillEditFromPatch();
}


void EditPatch::slotDetermineState() {
  m_parent->determineState( patchFromEdit() );
  fillEditFromPatch();
  emit stateChanged( this );
}

void EditPatch::dialogDestroyed() {
  m_editDlg = 0;
  EditPatchPointer p = this;
  emit dialogClosed( this );
}

void EditPatch::updateByType() {
  LocalPatchSourcePointer::Locked l = m_editingPatch;
  if ( !l )
    return ;
  l->applyCommand = ~m_editPatch.applyCommand->text();
  l->unApplyCommand = ~m_editPatch.unapplyCommand->text();
  l->type = ~m_editPatch.type->text();

  if ( l->type == "text/x-diff" ) {
    if ( l->state == LocalPatchSource::Applied ) {
      m_editPatch.filesGroup->show();
    } else {
      m_editPatch.filesGroup->hide();
    }
    if ( l->patchTool( false ) == "patch" && l->patchTool( true ) == "patch" ) {
      m_editPatch.determineState->setEnabled( true );
    } else {
      m_editPatch.determineState->setEnabled( false );
    }
  } else {
    m_editPatch.determineState->setEnabled( false );
    m_editPatch.filesGroup->hide();
  }
}

void EditPatch::showEditDialog() {
  if ( m_editDlg ) {
    out( Logger::Warning ) << "there is still another edit-dialog open";
    return ;
  }


  m_editDlg = new QDialog( m_parent->teamwork() ->widget() );

  connect( m_editDlg, SIGNAL( destroyed( QObject* ) ), this, SLOT( dialogDestroyed() ) );
  m_editPatch.setupUi( m_editDlg );

  m_filesModel = new QStandardItemModel( m_editPatch.filesList );
  m_editPatch.filesList->setModel( m_filesModel );

  connect( m_editPatch.previousHunk, SIGNAL( clicked( bool ) ), this, SLOT( prevHunk() ) );
  connect( m_editPatch.nextHunk, SIGNAL( clicked( bool ) ), this, SLOT( nextHunk() ) );
  connect( m_editPatch.filesList, SIGNAL( doubleClicked( const QModelIndex& ) ), this, SLOT( fileDoubleClicked( const QModelIndex& ) ) );
  connect( m_editPatch.filesList->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ), this, SLOT( fileSelectionChanged() ) );
  connect( m_editPatch.okButton, SIGNAL( pressed() ), this, SLOT( slotEditOk() ) );
  //connect( m_editPatch.cancelButton, SIGNAL( pressed() ), this, SLOT( slotEditCancel() ) );

  connect( m_editPatch.highlightFiles, SIGNAL( clicked( bool ) ), this, SLOT( highlightFile() ) );
  connect( m_editPatch.applyButton, SIGNAL( pressed() ), this, SLOT( slotApplyEditPatch() ) );
  connect( m_editPatch.unApplyButton, SIGNAL( pressed() ), this, SLOT( slotUnapplyEditPatch() ) );
  connect( m_editPatch.showButton, SIGNAL( pressed() ), this, SLOT( slotShowEditPatch() ) );

  connect( m_editPatch.type, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotEditMimeType( const QString& ) ) );
  connect( m_editPatch.chooseType, SIGNAL( pressed() ), this, SLOT( slotChooseType() ) );
  connect( m_editDlg, SIGNAL( finished( int ) ), this, SLOT( slotEditDialogFinished( int ) ) );
  connect( m_editPatch.filename, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotEditFileNameChanged( const QString& ) ) );
  connect( m_editPatch.command, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotEditCommandChanged( const QString& ) ) );
  connect( m_editPatch.state, SIGNAL( currentIndexChanged( int ) ), this, SLOT( slotStateChanged() ) );
  connect( m_editPatch.determineState, SIGNAL( clicked( bool ) ), this, SLOT( slotDetermineState() ) );
  connect( m_editPatch.commandToFile, SIGNAL( clicked( bool ) ), this, SLOT( slotToFile() ) );

  connect( m_editPatch.filename->lineEdit(), SIGNAL( returnPressed() ), this, SLOT( slotFileNameEdited() ) );
  connect( m_editPatch.filename->lineEdit(), SIGNAL( editingFinished() ), this, SLOT( slotFileNameEdited() ) );
  connect( m_editPatch.filename, SIGNAL( urlSelected( const QString& ) ), this, SLOT( slotCommandEdited() ) );
  connect( m_editPatch.command, SIGNAL( editingFinished() ), this, SLOT( slotCommandEdited() ) );
  connect( m_editPatch.command, SIGNAL( returnPressed() ), this, SLOT( slotCommandEdited() ) );
  connect( m_editPatch.command, SIGNAL( () ), this, SLOT( slotCommandEdited() ) );

  connect( m_editPatch.userButton, SIGNAL( clicked( bool ) ), this, SLOT( slotUserButton() ) );
  connect( m_editPatch.applyCommand, SIGNAL( textEdited( const QString& ) ), this, SLOT( updateByType() ) );
  connect( m_editPatch.unapplyCommand, SIGNAL( textEdited( const QString& ) ), this, SLOT( updateByType() ) );

  blockSignals( true );
  m_editPatch.accessRights->insertItem( 0, ~LocalPatchSource::accessToString( Public ) );
  m_editPatch.accessRights->insertItem( 1, ~LocalPatchSource::accessToString( ConnectedOnly ) );
  m_editPatch.accessRights->insertItem( 2, ~LocalPatchSource::accessToString( Ask ) );
  m_editPatch.accessRights->insertItem( 3, ~LocalPatchSource::accessToString( Private ) );

  m_editPatch.state->insertItem( 0, "Unknown" );
  m_editPatch.state->insertItem( 1, "Applied" );
  m_editPatch.state->insertItem( 2, "Not Applied" );


  m_editPatch.filename->fileDialog() ->setUrl( TeamworkFolderManager::workspaceDirectory() );

  m_editPatch.type->setText( "text/x-diff" );

  m_editDlg->show();
  blockSignals( false );
}

void EditPatch::hideEditDialog() {
  if ( !m_editDlg )
    return ;
  m_editDlg->close();
  delete m_editDlg;
  m_editDlg = 0;
}

void EditPatch::editPatchReadOnly() {
  if ( !m_editDlg )
    return ;

  m_editPatch.applyCommand->setReadOnly( true );
  m_editPatch.unapplyCommand->setReadOnly( true );
  m_editPatch.name->setReadOnly( true );
  m_editPatch.dependencies->setReadOnly( true );
  m_editPatch.command->setReadOnly( true );
  m_editPatch.accessRights->setEditable( false );
  if ( m_editPatch.accessRights->lineEdit() )
    m_editPatch.accessRights->lineEdit() ->setReadOnly( true );
  m_editPatch.type->setReadOnly( true );
  /*if( m_editPatch.type->lineEdit()  )
  m_editPatch.type->lineEdit()->setReadOnly( true );*/
  m_editPatch.description->setReadOnly( true );
  if ( m_editPatch.filename->lineEdit() )
    m_editPatch.filename->lineEdit() ->setReadOnly( true );
  if ( m_editPatch.filename->comboBox() )
    m_editPatch.filename->comboBox() ->setEditable( false );
  if ( m_editPatch.filename->lineEdit() )
    m_editPatch.filename->lineEdit() ->setReadOnly( true );
  /*m_editPatch.applyButton->setEnabled( false );
  m_editPatch.unApplyButton->setEnabled( false );
  m_editPatch.showButton->setEnabled( false );*/
  if ( !m_editPatchLocal ) {
    m_editPatch.unApplyButton->setEnabled( true );
    m_editPatch.applyButton->setEnabled( true );
    m_editPatch.showButton->setEnabled( true );
  }
  //  m_editPatch.cancelButton->setEnabled( false );
}

LocalPatchSource::State EditPatch::editState() {
  if ( !m_editDlg )
    return LocalPatchSource::Unknown;
  switch ( m_editPatch.state->currentIndex() ) {
    case 0:
    return LocalPatchSource::Unknown;
    case 1:
    return LocalPatchSource::Applied;
    default:
    return LocalPatchSource::NotApplied;
  }
}

LocalPatchSourcePointer EditPatch::patch() const {
  return m_editingPatch;
}

const QString terminalSuccessMarker = "TERMINAL_ACTION_SUCCESSFUL";
const QString reversedMarker = "Reversed (or previously applied) patch detected";

void EditPatch::apply( bool reverse, const QString& _fileName ) {
  try {
    if ( m_lastDataTime.isValid() ) {
      if ( m_lastDataTime.msecsTo( QTime::currentTime() ) < 100 ) {
        m_editPatch.konsoleDock->showNormal();
        throw "terminal is still in use";
      }
    }
    LocalPatchSourcePointer::Locked patch = patchFromEdit();
    if ( !patch )
      throw "could not lock patch";

    QString fileName = _fileName;
    if ( fileName.trimmed().isEmpty() )
      fileName = ( ~patch->filename ).trimmed();

    if ( fileName.isEmpty() )
      throw "cannot apply patch without a filename";

    out() << "Applying patch" << patch->name << "from file" << fileName;

    if ( ( !reverse && patch->state == LocalPatchSource::Applied ) )
      throw QString( "tried to apply an already applied patch: %1" ).arg( ~patch->name );
    if ( ( reverse && patch->state == LocalPatchSource::NotApplied ) )
      throw QString( "tried to unapply a not applied patch: %1" ).arg( ~patch->name );

    KLibFactory* factory = KLibLoader::self() ->factory( "libkonsolepart" );
    if ( factory == 0L )
      throw "no factory for libkonsolepart available";

    KParts::Part* p;
    if ( !m_konsolePart ) {
      p = static_cast<KParts::Part*>( factory->create( ( QObject* ) m_editDlg ) );
    } else {
      p = m_konsolePart;
    }

    if ( !p )
      throw "could not create konsole-part";

    p->widget() ->setFocusPolicy( Qt::WheelFocus );
    p->widget() ->setFocus();

    if ( QFrame * frame = qobject_cast<QFrame*>( p->widget() ) )
      frame->setFrameStyle( QFrame::Panel | QFrame::Sunken );

    m_konsolePart = p;

    TerminalInterface* terminal = static_cast<TerminalInterface*>( p->qt_metacast( "TerminalInterface" ) );
    if ( !terminal )
      throw "could not get terminal-interface";

    m_editPatch.konsoleDock->show();
    m_editPatch.konsoleDock->setWidget( p->widget() );
    p->widget() ->show();
    m_editPatch.konsoleDock->showNormal();

    m_reversed = false;

    QString params = ~patch->patchParams( reverse );

    bool hadFile = params.indexOf( "$FILE" ) != -1;
    //params.replace( "$FILE",  fileName );

    if ( !hadFile ) {
      if ( patch->patchTool( reverse ) == "patch" )
        params += " -i";
      params += " " + fileName;
    }
    QString command = "FILE=" + fileName + " " + ~patch->patchTool( reverse ) + " " + params + " && echo " + QString( terminalSuccessMarker ) + "\n";

    ///@todo not working with remove directories
    terminal->showShellInDir( TeamworkFolderManager::workspaceDirectory().toLocalFile() );

    if ( reverse )
      m_actionState = LocalPatchSource::NotApplied;
    else
      m_actionState = LocalPatchSource::Applied;

    connect( p, SIGNAL( receivedData( const QString& ) ), this, SLOT( receivedTerminalData( const QString& ) ) );

    patch->state = LocalPatchSource::Unknown;

    m_lastTerminalData.clear();
    terminal->sendInput( command );
    //terminal->startProgram( ~patch->patchTool( reverse ), l );

    fillEditFromPatch();
  } catch ( const QString & str ) {
    err() << QString( "error in applyPatch(reverse = %1): %2" ).arg( reverse ).arg( str );
  } catch ( const char * str ) {
    err() << QString( "error in applyPatch(reverse = %1): %2" ).arg( reverse ).arg( str );
  }
}

///This ugly hacking is necerssary because the status-output cannot be retrieved anymore
void EditPatch::receivedTerminalData( const QString& s ) {
  m_lastDataTime = QTime::currentTime();
  m_lastTerminalData += s;
  if ( m_lastTerminalData.contains( reversedMarker ) )
    m_reversed = true;

  if ( m_lastTerminalData.contains( terminalSuccessMarker ) ) {
    m_lastTerminalData.replace( "echo " + terminalSuccessMarker, "" );
    if ( m_lastTerminalData.contains( terminalSuccessMarker ) ) {
      LocalPatchSourcePointer::Locked l = patchFromEdit();
      if ( !l )
        return ;
      m_editPatch.konsoleDock->hide();
      if ( m_reversed ) {
        if ( m_actionState == LocalPatchSource::Applied )
          m_actionState = LocalPatchSource::NotApplied;
        else if ( m_actionState == LocalPatchSource::NotApplied )
          m_actionState = LocalPatchSource::Applied;
      }
      l->state = m_actionState;
      fillEditFromPatch();
      emit stateChanged( this );
    }
  }

  m_lastTerminalData = m_lastTerminalData.right( terminalSuccessMarker.length() + reversedMarker.length() );
}

void EditPatch::nextHunk() {
  updateKompareModel();
  seekHunk( true, m_isSource );
}

void EditPatch::prevHunk() {
  updateKompareModel();
  seekHunk( false, m_isSource );
}

void EditPatch::seekHunk( bool forwards, bool isSource, const QString& fileName ) {
  try {
    QModelIndexList il = m_editPatch.filesList->selectionModel() ->selectedIndexes();
    if ( il.isEmpty() )
      throw "selection is empty";
    if ( !m_modelList.get() )
      throw "no model";

    for ( QModelIndexList::iterator it = il.begin(); it != il.end(); ++it ) {
      QVariant v = m_filesModel->data( *it, Qt::UserRole );
      if ( !v.canConvert<const Diff2::DiffModel*>() )
        continue;
      const Diff2::DiffModel* model = v.value<const Diff2::DiffModel*>();
      if ( !model || !model->differences() )
        continue;

      KUrl file = TeamworkFolderManager::workspaceDirectory();
      if ( isSource ) {
        file.addPath( model->sourcePath() );
        file.addPath( model->sourceFile() );
      } else {
        file.addPath( model->destinationPath() );
        file.addPath( model->destinationFile() );
      }
      if ( !fileName.isEmpty() && fileName != file.toLocalFile() )
        continue;

      //out( Logger::Debug ) << "highlighting" << file.toLocalFile();

      IDocument* doc = KDevTeamworkPart::staticDocumentController() ->documentForUrl( file );

      if ( doc ) {
        KDevTeamworkPart::staticDocumentController()->activateDocument( doc );
        if ( doc->textDocument() ) {
          KTextEditor::View * v = doc->textDocument() ->activeView();
          int bestLine = -1;
          if ( v ) {
            KTextEditor::Cursor c = v->cursorPosition();
            for ( Diff2::DifferenceList::const_iterator it = model->differences() ->begin(); it != model->differences() ->end(); ++it ) {
              int line;
              Diff2::Difference* diff = *it;
              if ( isSource )
                line = diff->sourceLineNumber();
              else
                line = diff->destinationLineNumber();
              if ( line > 0 )
                line -= 1;

              if ( forwards ) {
                if ( line > c.line() && ( bestLine == -1 || line < bestLine ) )
                  bestLine = line;
              } else {
                if ( line < c.line() && ( bestLine == -1 || line > bestLine ) )
                  bestLine = line;
              }
            }
            if ( bestLine != -1 ) {
              v->setCursorPosition( KTextEditor::Cursor( bestLine, 0 ) );
              return ;
            }
          }
        }
      }
    }

  } catch ( const QString & str ) {
    err() << "seekHunk():" << str;
  } catch ( const char * str ) {
    err() << "seekHunk():" << str;
  }
  out() << "no matching hunk found";
}

void EditPatch::highlightFile() {
  updateKompareModel();
  try {
    QModelIndexList il = m_editPatch.filesList->selectionModel() ->selectedIndexes();
    if ( il.isEmpty() )
      throw "selection is empty";
    if ( !m_modelList.get() )
      throw "no model";

    for ( QModelIndexList::iterator it = il.begin(); it != il.end(); ++it ) {
      QVariant v = m_filesModel->data( *it, Qt::UserRole );
      if ( !v.canConvert<const Diff2::DiffModel*>() )
        continue;
      const Diff2::DiffModel* model = v.value<const Diff2::DiffModel*>();
      if ( !model )
        continue;

      KUrl file = TeamworkFolderManager::workspaceDirectory();
      if ( m_isSource ) {
        file.addPath( model->sourcePath() );
        file.addPath( model->sourceFile() );
      } else {
        file.addPath( model->destinationPath() );
        file.addPath( model->destinationFile() );
      }

      out( Logger::Debug ) << "highlighting" << file.toLocalFile();

      IDocument* doc = KDevTeamworkPart::staticDocumentController() ->documentForUrl( file );

      if ( !doc ) {
        doc = KDevTeamworkPart::staticDocumentController() ->openDocument( file, KTextEditor::Cursor(), KDevelop::IDocumentController::ActivateOnOpen );
        seekHunk( true, m_isSource, file.toLocalFile() );
      }
      removeHighlighting( file.toLocalFile() );

      m_highlighters[ file.toLocalFile() ] = new DocumentHighlighter( model, doc, m_isSource );
    }

  } catch ( const QString & str ) {
    err() << "highlightFile():" << str;
  } catch ( const char * str ) {
    err() << "highlightFile():" << str;
  }
}

void EditPatch::fileDoubleClicked( const QModelIndex& i ) {
  try {
    if ( !m_modelList.get() )
      throw "no model";
    QVariant v = m_filesModel->data( i, Qt::UserRole );
    if ( !v.canConvert<const Diff2::DiffModel*>() )
      throw "cannot convert";
    const Diff2::DiffModel* model = v.value<const Diff2::DiffModel*>();
    if ( !model )
      throw "bad model-value";

    KUrl file = TeamworkFolderManager::workspaceDirectory();
    if ( m_isSource ) {
      file.addPath( model->sourcePath() );
      file.addPath( model->sourceFile() );
    } else {
      file.addPath( model->destinationPath() );
      file.addPath( model->destinationFile() );
    }

    out( Logger::Debug ) << "opening" << file.toLocalFile();

    KDevTeamworkPart::staticDocumentController() ->openDocument( file, KTextEditor::Cursor(), KDevelop::IDocumentController::ActivateOnOpen );

    seekHunk( true, m_isSource, file.toLocalFile() );
  } catch ( const QString & str ) {
    err() << "fileDoubleClicked():" << str;
  } catch ( const char * str ) {
    err() << "fileDoubleClicked():" << str;
  }
}

void EditPatch::fileSelectionChanged() {
  QModelIndexList i = m_editPatch.filesList->selectionModel() ->selectedIndexes();
  m_editPatch.nextHunk->setEnabled( false );
  m_editPatch.previousHunk->setEnabled( false );
  m_editPatch.highlightFiles->setEnabled( false );
  if ( !m_modelList.get() )
    return ;
  for ( QModelIndexList::iterator it = i.begin(); it != i.end(); ++it ) {
    QVariant v = m_filesModel->data( *it, Qt::UserRole );
    if ( v.canConvert<const Diff2::DiffModel*>() ) {
      const Diff2::DiffModel * model = v.value<const Diff2::DiffModel*>();
      if ( model ) {
        if ( model->differenceCount() != 0 ) {
          m_editPatch.nextHunk->setEnabled( true );
          m_editPatch.previousHunk->setEnabled( true );
          m_editPatch.highlightFiles->setEnabled( true );
        }
      }
    }
  }
}

DocumentHighlighter::DocumentHighlighter( const Diff2::DiffModel* model, IDocument* kdoc, bool isSource ) throw( QString ) : m_doc( kdoc ) {
//  connect( kdoc, SIGNAL( destroyed( QObject* ) ), this, SLOT( documentDestroyed() ) );
  connect( kdoc->textDocument(), SIGNAL( destroyed( QObject* ) ), this, SLOT( documentDestroyed() ) );
  connect( model, SIGNAL( destroyed( QObject* ) ), this, SLOT( documentDestroyed() ) );

  KTextEditor::Document* doc = kdoc->textDocument();
  if ( doc->lines() == 0 )
    return ;
  if( doc->activeView() == 0 ) return;

  if ( !model->differences() )
    return ;
  KTextEditor::SmartInterface* smart = dynamic_cast<KTextEditor::SmartInterface*>( doc );
  if ( !smart )
    throw QString( "no smart-interface" );

  QMutexLocker lock(smart->smartMutex());
  
  KTextEditor::SmartRange* topRange = smart->newSmartRange(doc->documentRange(), 0, KTextEditor::SmartRange::ExpandLeft | KTextEditor::SmartRange::ExpandRight);
  
  for ( Diff2::DifferenceList::const_iterator it = model->differences() ->begin(); it != model->differences() ->end(); ++it ) {
    Diff2::Difference* diff = *it;
    int line, lineCount;
    if ( isSource ) {
      line = diff->sourceLineNumber();
      lineCount = diff->sourceLineCount();
    } else {
      line = diff->destinationLineNumber();
      lineCount = diff->destinationLineCount();
    }
    if ( line > 0 )
      line -= 1;

    KTextEditor::Cursor c( line, 0 );
    KTextEditor::Cursor endC( line + lineCount, 0 );
    if ( doc->lines() <= c.line() )
      c.setLine( doc->lines() - 1 );
    if ( doc->lines() <= endC.line() )
      endC.setLine( doc->lines() - 1 );
    endC.setColumn( doc->lineLength( endC.line() ) ) ;

    if ( endC.isValid() && c.isValid() ) {
      KTextEditor::SmartRange * r = smart->newSmartRange( c, endC );
      r->setParentRange(topRange);
      KSharedPtr<KTextEditor::Attribute> t( new KTextEditor::Attribute() );

      t->setProperty( QTextFormat::BackgroundBrush, QBrush( QColor( 200, 200, 255 ) ) );
      r->setAttribute( t );
    }
  }

  m_ranges << topRange;

  smart->addHighlightToDocument(topRange);
}

DocumentHighlighter::~DocumentHighlighter() {
  for ( QList<KTextEditor::SmartRange*>::iterator it = m_ranges.begin(); it != m_ranges.end(); ++it )
    delete *it;

  m_ranges.clear();
}

IDocument* DocumentHighlighter::doc() {
  return m_doc;
}

void DocumentHighlighter::documentDestroyed() {
  m_ranges.clear();
}

void EditPatch::removeHighlighting( const QString& file ) {
  if ( file.isEmpty() ) {
    ///Remove all highlighting
    for ( HighlightMap::iterator it = m_highlighters.begin(); it != m_highlighters.end(); ++it ) {
      delete *it;
    }
    m_highlighters.clear();
  } else {
    HighlightMap::iterator it = m_highlighters.find( file );
    if ( it != m_highlighters.end() ) {
      delete * it;
      m_highlighters.erase( it );
    }
  }
}


void EditPatch::updateKompareModel() {
  try {
    LocalPatchSourcePointer::Locked l = m_editingPatch;
    if ( l ) {
      if ( l->state != LocalPatchSource::Applied )
        return ;
      if ( l->type != "text/x-diff" )
        return ;
      if ( l->command == ~m_lastModelCommand && l->filename == ~m_lastModelFile && m_modelList.get() )
        return ; ///We already have the correct model
      m_lastModelCommand = ~l->command;
      m_lastModelFile = ~l->filename;
    }


    m_modelList.reset( 0 );
    qRegisterMetaType<const Diff2::DiffModel*>( "const Diff2::DiffModel*" );
    if ( m_diffSettings )
      delete m_diffSettings;
    m_diffSettings = new DiffSettings( m_editDlg );
    m_kompareInfo.reset( new Kompare::Info() );
    removeHighlighting();
    m_modelList.reset( new Diff2::KompareModelList( m_diffSettings, *m_kompareInfo, ( QObject* ) this ) );
    KUrl diffFile = getPatchFile( true );
    if ( diffFile.isEmpty() )
      return ;
    try {
      ///@todo does not work with remote URLs
      if ( !m_modelList->openDirAndDiff( TeamworkFolderManager::workspaceDirectory().toLocalFile(), diffFile.toLocalFile() ) )
        throw "could not open diff " + diffFile.toLocalFile();
    } catch ( const QString & str ) {
      throw;
    } catch ( ... ) {
      throw QString( "libdiff2 crashed, memory may be corrupted. Please restart kdevelop." );
    }
    m_filesModel->clear();
    m_filesModel->insertColumns( 0, 1 );

    const Diff2::DiffModelList* models = m_modelList->models();
    if ( !models )
      throw "no diff-models";
    Diff2::DiffModelList::const_iterator it = models->begin();
    while ( it != models->end() ) {
      Diff2::DifferenceList * diffs = ( *it ) ->differences();
      int cnt = 0;
      if ( diffs )
        cnt = diffs->count();

      KUrl file;
      if ( m_isSource ) {
        file.addPath( ( *it ) ->sourcePath() );
        file.addPath( ( *it ) ->sourceFile() );
      } else {
        file.addPath( ( *it ) ->destinationPath() );
        file.addPath( ( *it ) ->destinationFile() );
      }

      m_filesModel->insertRow( 0 );
      QModelIndex i = m_filesModel->index( 0, 0 );
      if ( i.isValid() ) {
        //m_filesModel->setData( i, file, Qt::DisplayRole );
        m_filesModel->setData( i, QString( "%1 (%2 hunks)" ).arg( file.toLocalFile() ).arg( cnt ), Qt::DisplayRole );
        QVariant v;
        v.setValue<const Diff2::DiffModel*>( *it );
        m_filesModel->setData( i, v, Qt::UserRole );
      }
      ++it;
    }
    fileSelectionChanged();
    return ;
  } catch ( const QString & str ) {
    err() << "updateKompareModel:" << str;
  } catch ( const char * str ) {
    err() << "updateKompareModel:" << str;
  }
  m_modelList.reset( 0 );
  delete m_diffSettings;
  m_kompareInfo.reset( 0 );
}

#include "editpatch.moc"

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
