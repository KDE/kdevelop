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

#include "kdevteamwork_messages.h"
#include "kdevteamwork_part.h"
#include <QStandardItemModel>
#include <QModelIndex>
#include <QMenu>
#include <QVariant>
#include <QFileInfo>
#include <QFile>
#include <kmessagebox.h>
#include <klocalizedstring.h>
#include <k3process.h>
#include <ktexteditor/document.h>
#include <ktexteditor/document.h>
#include <ktexteditor/range.h>
#include <idocumentcontroller.h>
#include <idocument.h>
#include "dynamictext/dynamictext.h"
#include "kdevteamwork_helpers.h"
#include "collaborationmanager.h"
#include "filecollaborationmessages.h"
#include "documentwrapper.h"
#include "network/basicsession.h"
#include "utils.h"
#include "dynamictext/verify.h"
#include "filesynchronizemessage.h"
#include "qdynamictext.h"
#include "vectortimestampeditor.h"
#include "kdevteamwork.h"
#include "patchesmanager.h"
#include "teamworkfoldermanager.h"
#include "filecollaborationsession.h"
#include "teamworkfoldermanager.h"
#include "dynamictext/verify.h"
#include "patchmessage.h"
#include "filecollaborationmanager.h"
#include "network/messagetypeset.h"
#include "network/messagesendhelper.h"
#include "iuicontroller.h" /* defines [function] activeMainWindow */
#include "icore.h" /* defines [function] uiController */

using namespace KDevelop;

#define IFDEBUG( x ) /**/
//#define IFDEBUG( x ) x

std::ostream& operator << ( std::ostream& o, const SimpleReplacement& rhs ) {
  o << "( at " << rhs.m_position << ": '" << rhs.m_oldText << "' -> '" << rhs.m_newText << "' )";
  return o;
}

struct DocumentWrapperTreeAction : public StandardCollaborationTreeAction<DocumentWrapper> {
  DocumentWrapperTreeAction( DocumentWrapper* w ) : StandardCollaborationTreeAction<DocumentWrapper>( w ) {}
  virtual void doubleClicked() {
    if ( target ) {
      target->openDocument( true );
    }
  }
  virtual void clicked() {}
  virtual void fillContextMenu( QMenu* menu ) {
    if ( target )
      target->fillContextMenu( 0, menu );
  }
}
;

bool DocumentWrapper::hadError() const {
  return m_hadError;
}

uint DocumentWrapper::id() const {
  return m_id;
}

DocumentWrapper::DocumentWrapper( uint id, const LoggerPointer& logger, QString fileName, uint ownIndex, bool readFile_, FileCollaborationSession* session, bool fromBuffer ) :
    SafeLogger( logger, ~( "document-wrapper \"" + fileName + "\": " ) ),
                m_text( new QDynamicText() ),
                m_block( false ),
                m_realFile( readFile_ ),
                m_fileName( fileName ),
                m_document( 0 ),
                m_session( session ),
                m_ownIndex( ownIndex ),
                m_id( id ),
                m_hadError( false ),
                m_disabled( false ),
                m_dead( false ),
                m_dispatcher( *this ){
  //openDocument();
  out( Logger::Debug ) << "DocumentWrapper for " << fileName << "created: " << ownIndex << " " << readFile_;
  if ( m_realFile )
    readFile( fromBuffer );

  connect( m_text.data(), SIGNAL( stateChanged( QDynamicText& ) ), this, SLOT( stateChanged() ) );
}

QIcon DocumentWrapper::icon() const {
  if ( m_dead )
    return IconCache::getIconStatic( "collaboration_file_dead" );
  if ( !m_disabled )
    return IconCache::getIconStatic( "collaboration_file" );
  else
    return IconCache::getIconStatic( "collaboration_file_disabled" );
}

void DocumentWrapper::updateTree( QModelIndex& i, QStandardItemModel* model ) {
  model->setData( i, fileName(), Qt::DisplayRole );
  model->setData( i, icon(), Qt::DecorationRole );
  QVariant v;
  v.setValue<CollaborationTreeActionPointer>( new DocumentWrapperTreeAction( this ) );
  model->setData( i, v, Qt::UserRole );
}

void DocumentWrapper::fillContextMenu( int /*var*/, QMenu* menu ) {
  QMenu * m = new QMenu( menu );
  m->setTitle( "Save Visible State" );
  m->addAction( "As Patch", this, SLOT( saveAsPatch() ) );
  //m->addAction( "To New Buffer", this, SLOT( saveNewBuffer() ) );
  //m->addSeparator();
  m->addAction( QString( "As %1" ).arg( m_fileName ), this, SLOT( saveAsBufferFile() ) );
  menu->addMenu( m );


  if ( m_disabled )
    menu->addAction( "Enable Editing", this, SLOT( toggleEnabled() ) );
  else if ( m_session->isMasterSession() )
    menu->addAction( "Disable Editing", this, SLOT( toggleEnabled() ) );

  if ( m_session->isMasterSession() || m_dead ) {
    menu->addSeparator();
    menu->addAction( "Remove", this, SLOT( remove
                                             () ) );
  }

  menu->addSeparator();
  menu->addAction( "Manage Dynamic Text", this, SLOT( manageDynamicText() ) );
}

QString DocumentWrapper::text() const {
  return ~m_text->text().text();
}

VectorTimestamp DocumentWrapper::state() const {
  return m_text->state();
}

const QDynamicText& DocumentWrapper::dynamicText() const {
  return * m_text;
}

bool DocumentWrapper::checkDocumentSync() const {
  if ( m_document && m_document->textDocument() ) {
    return m_document->textDocument() ->text() == ~m_text->text().text();
  } else {
    return true;
  }
}

void DocumentWrapper::remove
() {}


void DocumentWrapper::toggleEnabled() {
  m_disabled = !m_disabled;
  out( Logger::Debug ) << "Editing is now " << ( m_disabled ? "disabled" : "enabled");
}

void DocumentWrapper::saveAsBufferFile() {
  KUrl u = TeamworkFolderManager::workspaceAbsolute( m_fileName );

  IDocument* doc = KDevTeamworkPart::staticDocumentController() ->openDocument( u, KTextEditor::Cursor(), KDevelop::IDocumentController::ActivateOnOpen );

  if ( doc && doc->textDocument() ) {
    if ( doc->state() == IDocument::Modified || doc->state() == IDocument::DirtyAndModified ) {
      int answer = KMessageBox::warningYesNo( KDevTeamworkPart::staticCore()->uiController()->activeMainWindow()->window(), i18n( "The buffer of %1 is modified, should the content be replaced?" , m_fileName ) );
      if ( answer != KMessageBox::Yes )
        return ;
    }
    doc->textDocument() ->setText( ~m_text->text().text() );

  } else {
    err() << "saveAsBufferFile(): could not open document for " << u.path();
  }
}

OutputFileWriter::OutputFileWriter( const QString& file ) : m_file( file ), m_failed( false ) {
  if ( !m_file.open( QIODevice::WriteOnly ) )
    throw QString( "could not open file %1" ).arg( file );
}

bool OutputFileWriter::failed() const {
  return m_failed;
}

void OutputFileWriter::receivedStdout( K3Process */*proc*/, char *buffer, int buflen ) {
  if ( m_file.write( buffer, buflen ) != buflen ) {
    m_failed = true;
  }
}

LocalPatchSourcePointer DocumentWrapper::saveAsPatch( bool addToList, bool edit ) {
  try {
    KDevTeamwork * tw = KDevTeamwork::self();

    PatchesManager* pm = tw->patchesManager();

    QString peer = m_session->firstPeerName();
    if ( m_session->isMasterSession() )
      peer = "local";

    LocalPatchSourcePointer::Locked l = new LocalPatchSource();
    l->name = ~( m_session->name() + "@" + peer + "_" + QDateTime::currentDateTime().toString( Qt::ISODate ) );
    l->type = "text/x-diff";
    l->applyCommand = "-p0";
    l->unApplyCommand = "-p0 -R";
    l->userIdentity = m_session->manager() ->teamwork() ->currentUserIdentity();

    KUrl u = TeamworkFolderManager::createUniqueFile( "patches", "diff", ~l->name, "collab_", "" );

    l->filename = ~TeamworkFolderManager::teamworkRelative( u, "patches" );

    KUrl workingDir = TeamworkFolderManager::createUniqueDirectory( "temp", m_session->name(), "diff_" );
    KUrl subFolder( TeamworkFolderManager::teamworkRelative( workingDir ) );
    subFolder.addPath( QFileInfo( m_fileName ).path() );

    KUrl tempCurrent = TeamworkFolderManager::createUniqueFile( subFolder.path(), m_fileName );
    KUrl tempBase = TeamworkFolderManager::createUniqueFile( subFolder.path(), m_fileName, "base_" );

    TeamworkFolderManager::self() ->registerTempItem( tempCurrent );
    TeamworkFolderManager::self() ->registerTempItem( tempBase );

    {
      std::ofstream fCurrent( tempCurrent.path().toUtf8().data(), ios_base::out );
      if ( !fCurrent.is_open() )
        throw "could not open file for writing: " + tempCurrent.path();
      fCurrent << m_text->text().text();
    }
    {
      std::ofstream fBase( tempBase.path().toUtf8().data(), ios_base::out );
      if ( !fBase.is_open() )
        throw "could not open file for writing: " + tempBase.path();
      fBase << m_text->initialText();
    }

    K3Process proc;

    ///@todo make this work with remote files
    OutputFileWriter ow( u.path() );
    proc.setWorkingDirectory( workingDir.path() );
    QString cmdLine = + + " " + tempCurrent.path() + " > " + u.path();
    proc.setPriority( K3Process::PrioLowest );
    proc << "diff";
    proc << "--unified=4";
    proc << tempBase.path();
    proc << tempCurrent.path();

    connect( &proc, SIGNAL( receivedStdout ( K3Process *, char *, int ) ), &ow, SLOT( receivedStdout( K3Process*, char*, int ) ) );

    if ( ow.failed() )
      throw QString( "writing to %1 failed" ).arg( u.prettyUrl() );

    out( Logger::Debug ) << "saveAsPatch(..) executing \"diff --unified=4 " + tempBase.path() + " " + tempCurrent.path() + "\"";

    proc.start( K3Process::Block );
    if ( addToList )
      pm->addPatch( l );
    if ( edit )
      pm->showPatchInfo( l, true );

    return l;
  } catch ( const QString & str ) {
    err() << "saveAsPatch(): " << str;
  } catch ( const char * str ) {
    err() << "saveAsPatch(): " << str;
  }
  return 0;
}

void DocumentWrapper::saveNewBuffer() {
  ///It would be nice to create a nameless buffer here and insert the text
}

void DocumentWrapper::manageDynamicText() {
  new VectorTimestampEditor( logger(), this, false, m_text, true );
}

void DocumentWrapper::stateChanged() {
  if ( m_block )
    return ;
  fillDocumentText();
}

bool DocumentWrapper::synchronize( const UserPointer& user ) {
  if ( m_dead )
    return false;

  out( Logger::Debug ) << "synchronizing with " << user.unsafe()->safeName();

  UserPointer::Locked l = user;
  if ( l && l->online().session() ) {
    return globalMessageSendHelper().send<FileSynchronize>( l->online().session().unsafe(), fileName(), *m_text, true, id(), m_session->id() );
  } else {
    err() << "cannot send synchronization-message because the user cannot be locked, or is not online";
    return false;
  }
}

int DocumentWrapper::receiveMessage( FileEditRejectMessage* msg ) {
  if ( m_session->isMasterSession() ) {
    out( Logger::Warning ) << "got a reject-message from a client, sending complete synchronization";
    synchronize( msg->info().user() );
    return 0;
  }

  ///A reject-message contains a new state that should be applied
  VectorTimestamp v = msg->timeStamp();
  try {
    m_text->changeState( v );
    m_text->cut();
  } catch ( const DynamicTextError & err ) {
    ///There is no other way than resynchronizing the whole file
    out() << "error while handling a reject, resynchronizing. Error: " << err.what();
    globalMessageSendHelper().sendReply<FileEditRejectMessage>( msg, VectorTimestamp(), id(), m_session->id() );
  }
  return 1;
}

int DocumentWrapper::receiveMessage( MessageInterface* msg ) {
  out( Logger::Warning ) << "got unknown message of type " << msg->name();
  return 0;
}

int DocumentWrapper::receiveMessage( FileEditMessage* msg ) {
  try {
    if ( ( m_disabled || m_dead ) && m_session->isMasterSession() ) {
      globalMessageSendHelper().sendReply<FileEditRejectMessage>( msg, m_text->tailState(), id(), m_session->id() );
      return 0;
    }
    FileEditMessage * emsg = dynamic_cast<FileEditMessage*>( msg );
    {
      Block b( m_block );
      bool wasTail = m_text->state() == m_text->tailState();
      m_text->insert( emsg->timeStamp(), emsg->replacement() );

      if ( wasTail ) {
        m_text->text().registerNotifier( this );
        m_text->changeState();
        m_text->text().unregisterNotifier();
      }
    }

  } catch ( const DynamicTextError & error ) {
    ///@todo error-handling
    err() << "receiveMessage( FileEditMessage " << msg->timeStamp() << " " << msg->replacement() << " ): " << error.what();
    globalMessageSendHelper().sendReply<FileEditRejectMessage>( msg, m_text->tailState(), id(), m_session->id() );
    m_text->text().unregisterNotifier(); //@todo use a little wrapper-class for this
  }
  return 1;
}

void DocumentWrapper::notifyFlexibleTextErase( int position, int length ) {
    if ( m_document && m_document->textDocument() ) {
      Block b( m_block );
      int line, column;

      m_text->text().linearToLineColumn( position, line, column );
      if ( line == -1 || column == -1 )
        throw DynamicTextError( "receiveMessage( FileEditMessage ): could not convert index to cursor" );
      
      KTextEditor::Cursor start( line, column );

      
      m_text->text().linearToLineColumn( position + length, line, column );
      
      if ( line == -1 || column == -1 )
        throw DynamicTextError( "receiveMessage( FileEditMessage ): could not convert index to cursor" );
      
      KTextEditor::Cursor end( line, column );

      
      m_document->textDocument() ->replaceText( KTextEditor::Range( start, end ), "" );
    }
}

void DocumentWrapper::notifyFlexibleTextInsert( int position, const std::string& text ) {
    if ( m_document && m_document->textDocument() ) {
      Block b( m_block );
      int line, column;

      m_text->text().linearToLineColumn( position, line, column );
      if ( line == -1 || column == -1 )
        throw DynamicTextError( "receiveMessage( FileEditMessage ): could not convert index to cursor" );
      
      KTextEditor::Cursor start( line, column );

      m_document->textDocument() ->replaceText( KTextEditor::Range( start, start ), toQ( text.c_str() ) );
    }
}

void DocumentWrapper::notifyFlexibleTextReplace( int position, int length, const std::string& replacement ) {
    if ( m_document && m_document->textDocument() ) {
      Block b( m_block );
      int line, column;

      m_text->text().linearToLineColumn( position, line, column );
      if ( line == -1 || column == -1 )
        throw DynamicTextError( "receiveMessage( FileEditMessage ): could not convert index to cursor" );
      
      KTextEditor::Cursor start( line, column );

      
      m_text->text().linearToLineColumn( position + length, line, column );
      
      if ( line == -1 || column == -1 )
        throw DynamicTextError( "receiveMessage( FileEditMessage ): could not convert index to cursor" );
      
      KTextEditor::Cursor end( line, column );

      
      m_document->textDocument() ->replaceText( KTextEditor::Range( start, end ), toQ( replacement.c_str() ) );
    }
}

void DocumentWrapper::processMessage( DocumentWrapperMessage* msg ) {

  /*if ( !checkDocumentSync() ) {
    if ( m_document && m_document->textDocument() ) {
      out( Logger::Warning ) << "processMessage(begin): document and dynamictext got out of sync! Resynchronizing";
      fillDocumentText();
    }
  }*/

  IFDEBUG( out( Logger::Debug ) << "processMessage(..) processing " << msg->name() );

  m_dispatcher( msg );
  /*
  if ( !checkDocumentSync() ) {
    if ( m_document && m_document->textDocument() ) {
      out( Logger::Warning ) << "processMessage(end): document and dynamictext got out of sync! Resynchronizing";
      fillDocumentText();
    }
  }*/
}

void DocumentWrapper::documentDestroyed() {
  m_document = 0;
}

QString DocumentWrapper::fileName() const {
  return m_fileName;
}

IDocument* DocumentWrapper::document() {
  return m_document;
}

void DocumentWrapper::fillDocumentText() {
  Block b( m_block );
  if ( m_document && m_document->textDocument() ) {
    m_document->textDocument() ->setText( ~m_text->text().text() );
  } else {
    err() << "cannot update document-text: no document available";
  }
}

void DocumentWrapper::openDocument( bool toForeground ) {
  try {
    if ( m_document ) {
      if( toForeground )
        KDevTeamworkPart::staticDocumentController() ->activateDocument( m_document );
      return ;
    }

    out( Logger::Debug ) << "openDocument() " << fileName() << " " << toForeground;

    if ( m_tempFile.isEmpty() ) {
      KUrl subfolder( QString( "filecollaboration" ) );
      subfolder.addPath( m_session->name() + "_" + QString( "%1").arg(m_session->id() ) );

      QString location;
      if ( m_session->isMasterSession() ) {
        location = "local";
      } else {
        location = m_session->firstPeerName();
      }

      ///@todo make this work work remote files
      m_tempFile = TeamworkFolderManager::createUniqueFile( subfolder.path(), m_fileName, "", "@" + m_session->name() ).path();
    }

    out( Logger::Debug ) << "temporary file for " << fileName() << " is " << m_tempFile;

    m_document = KDevTeamworkPart::staticDocumentController() ->openDocument( m_tempFile, KTextEditor::Cursor(), toForeground == true ? KDevelop::IDocumentController::ActivateOnOpen : KDevelop::IDocumentController::DontActivate );

    if ( m_document ) {
      KTextEditor::Document * doc = m_document->textDocument();
      if ( !doc ) {
        m_document = 0;
        throw "could not open as text-document";
      }
      fillDocumentText();

      /*connect( m_document, SIGNAL( destroyed( QObject* ) ), this, SLOT( documentDestroyed() ) );*/
      connect( m_document->textDocument(), SIGNAL( destroyed( QObject* ) ), this, SLOT( documentDestroyed() ) );
      connect( doc, SIGNAL( textInserted ( KTextEditor::Document *,
                                           const KTextEditor::Range & ) ), this, SLOT( textInserted ( KTextEditor::Document *,
                                               const KTextEditor::Range & ) ) );
      connect( doc, SIGNAL( textRemoved ( KTextEditor::Document *,
                                          const KTextEditor::Range & ) ), this, SLOT( textRemoved ( KTextEditor::Document *,
                                              const KTextEditor::Range & ) ) );
      connect( doc, SIGNAL( textChanged ( KTextEditor::Document *,
                                          const KTextEditor::Range &,
                                          const KTextEditor::Range & ) ), this, SLOT( textChanged ( KTextEditor::Document *,
                                              const KTextEditor::Range &,
                                              const KTextEditor::Range & ) ) );
    } else {
      throw "could not open document";
    }
  } catch ( const QString & str ) {
    err() << "openDocument(): " << str;
  } catch ( const char * str ) {
    err() << "openDocument(): " << str;
  }
}

uint getPositionInDocument( KTextEditor::Document * doc, const KTextEditor::Cursor& cursor ) {
  QString str = doc->text( KTextEditor::Range( KTextEditor::Cursor( 0, 0 ), cursor ) );
  return str.length();
}

void DocumentWrapper::textChanged ( KTextEditor::Document * document, const KTextEditor::Range & oldRange, const KTextEditor::Range & newRange ) {
  if ( m_block )
    return ;
  out( Logger::Debug ) << "textChanged";

  VectorTimestamp v( m_text->state() );
  v.setPrimaryIndex( m_ownIndex );
  v.increase();

  int start = m_text->text().lineColumnToLinear( oldRange.start().line(), oldRange.start().column() );
  int end = m_text->text().lineColumnToLinear( oldRange.end().line(), oldRange.end().column() );

  if ( start == -1 || end == -1 || start == end ) {
    err() << "textRemoved(..): DynamicText and Document seem to be out of sync";

    fillDocumentText();
    return ;
  }

  std::string oldText = m_text->text().substr( start, end - start );
  QString newText = document->text( newRange );

  if ( oldRange.end().line() == oldRange.start().line() && newRange.end().line() == newRange.start().line() ) {
    out( Logger::Debug ) << "textChanged at " << start << ": " << oldText << " -> " << newText;
  } else {
    out( Logger::Debug ) << "textChanged at " << start;
  }

  ///It might be better to model the edit by one removal and one insertion, because that allows better resolution of conflicts.(Think about if it should be like that)
  SimpleReplacement r( start, ~newText, oldText );

  try {
    DYN_VERIFY( !m_disabled );
    DYN_VERIFY_SAME( m_text->state(), m_text->tailState() );
    Block b( m_block );
    m_text->insert( v, r );
  } catch ( const DynamicTextError & error ) {
    err() << "error in textRemoved(..): " << error.what();
    fillDocumentText();
    return ;
  }

  if ( !m_dead )
    emit publishEdit( v, r, this );
}

///All the line/column to/from linear conversion is very slow and should be done different
void DocumentWrapper::textRemoved ( KTextEditor::Document * /*document*/, const KTextEditor::Range & range ) {
  if ( m_block )
    return ;
  int start = m_text->text().lineColumnToLinear( range.start().line(), range.start().column() );
  int end = m_text->text().lineColumnToLinear( range.end().line(), range.end().column() );

  if ( start == -1 || end == -1 ) {
    err() << "textRemoved(..): DynamicText and Document seem to be out of sync";
    fillDocumentText();
    return ;
  }

  std::string oldText = m_text->text().substr( start, end - start );

  if ( range.end().line() == range.start().line() ) {
    IFDEBUG( out( Logger::Debug ) << "textRemoved at " << start << ": " << document->text( range ) );
  } else {
    IFDEBUG( out( Logger::Debug ) << "textRemoved at " << start );
  }

  SimpleReplacement r( start, "", oldText );

  VectorTimestamp v;
  try {
    DYN_VERIFY( !m_disabled );
    DYN_VERIFY_SAME( m_text->state(), m_text->tailState() );
    Block b( m_block );
    v = m_text->insert( m_ownIndex, r );
  } catch ( const DynamicTextError & error ) {
    err() << "error in textRemoved(..): " << error.what();
    fillDocumentText();
    return ;
  }

  if ( !m_dead )
    emit publishEdit( v, r, this );
}

void DocumentWrapper::textInserted ( KTextEditor::Document * document, const KTextEditor::Range & range ) {
  if ( m_block )
    return ;
  VectorTimestamp v;

  QString newText = document->text( range );
  uint pos = m_text->text().lineColumnToLinear( range.start().line(), range.start().column() );

  if ( range.end().line() == range.start().line() ) {
    IFDEBUG( out( Logger::Debug ) << "textInserted at " << pos << ": " << document->text( range ) );
  } else {
    IFDEBUG( out( Logger::Debug ) << "textInserted at " << pos );
  }

  SimpleReplacement r = SimpleReplacement( pos, ~newText, "" );

  try {
    DYN_VERIFY_SAME( m_text->state(), m_text->tailState() );
    DYN_VERIFY( !m_disabled );
    Block b( m_block );
    v = m_text->insert( m_ownIndex, r );
  } catch ( const DynamicTextError & error ) {
    err() << "error in textInserted(..): " << error.what();
    fillDocumentText();
    return ;
  }

  if ( !m_dead )
    emit publishEdit( v, r, this );
}

std::string DocumentWrapper::logPrefix() {
  return "DocumentWrapper for " + ~fileName() + ": ";
}

int DocumentWrapper::receiveMessage( FileSynchronize* msg ) {
  ///@todo merge synchronizations from different clients when self was crashed
  /*if ( !m_text->state().isZero() ) {
    err() << "got synchronization although timestamp already is " << m_text->state().print();
    return ;
  }*/
  out( Logger::Debug ) << "got synchronization - current Timestamp: " << m_text->state().print() << " new: " << msg->state().print();

  try {
    m_text = msg->createDynamicText();
    connect( m_text.data(), SIGNAL( stateChanged( QDynamicText& ) ), this, SLOT( stateChanged() ) );
    fillDocumentText();
    globalMessageSendHelper().sendReply<KDevSystemMessage>( msg, KDevSystemMessage::ActionSuccessful );
  } catch ( const DynamicTextError & error ) {
    err() << "error while synchronization: " << error.what();
    globalMessageSendHelper().sendReply<KDevSystemMessage>( msg, KDevSystemMessage::ActionFailed );
  }

  return 1;
}

bool DocumentWrapper::dead() const {
  return m_dead;
}

bool DocumentWrapper::disabled() const {
  return m_disabled;
}

void DocumentWrapper::setDisabled( bool disabled ) {
  m_disabled = disabled;
}

void DocumentWrapper::setDead( bool dead ) {
  m_dead = dead;
}

void DocumentWrapper::readFile( bool fromBuffer ) throw ( QString ) {
  KUrl u = TeamworkFolderManager::workspaceAbsolute( m_fileName );

  if ( !m_text->state().isZero() ) {
    out( Logger::Warning ) << "readFile called although state already is " << m_text->state().print();
  }

  QString txt;

  if ( fromBuffer ) {
    IDocument * doc = KDevTeamworkPart::staticDocumentController() ->documentForUrl( u );
    if ( doc && doc->textDocument() ) {
      txt = doc->textDocument() ->text();
    }
  }

  if ( txt.isEmpty() ) {
    if ( !u.isLocalFile() )
      throw QString( "file is not local" );
    if ( !QFileInfo( u.path() ).exists() )
      throw QString( "file does not exist" );

    QFile f( u.path() );
    if ( !f.open( QIODevice::ReadOnly ) )
      throw QString( "could not open file" );

    ///@todo What about encoding etc. ?
    QByteArray b = f.readAll();
    Block bl( m_block );
    m_text = new QDynamicText( VectorTimestamp( m_ownIndex, VectorTimestamp() ), b.data() );
  } else {
    m_text = new QDynamicText( VectorTimestamp( m_ownIndex, VectorTimestamp() ), txt.toUtf8().data() );
  }
  /*m_text->changeState();
  m_text->insert( m_ownIndex, SimpleReplacement( 0, b.data(), m_text->text() ) );*/
}

#include "documentwrapper.moc"

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
