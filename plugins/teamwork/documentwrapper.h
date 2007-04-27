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

#ifndef DOCUMENT_WRAPPER_H
#define DOCUMENT_WRAPPER_H

#include "dynamictext/vectortimestamp.h"
#include <QObject>
#include "network/safesharedptr.h"
#include "network/weaksafesharedptr.h"
#include "safelogger.h"
#include <map>
#include <KParts/MainWindow>

#include <autoconstructpointer.h>
#include "filesynchronizemessage.h"
#include <QFile>

namespace Teamwork {
class Logger;
typedef SafeSharedPtr<Logger> LoggerPointer;
};

namespace KTextEditor {
class Document;
class Range;
};

class FileSynchronize;
class DynamicText;
class QStandardItemModel;
class QModelIndex;
class QMenu;
class QVariant;
class DocumentWrapperTreeAction;
namespace KDevelop {
class IDocument;
};
class DocumentWrapperMessage;
class QDynamicText;
class FileCollaborationSession;
class LocalPatchSource;
class  K3Process;

using namespace Teamwork;

typedef SafeSharedPtr<LocalPatchSource> LocalPatchSourcePointer;
typedef SharedPtr<QDynamicText, BoostSerializationNormal> QDynamicTextPointer;

class OutputFileWriter : public QObject {
  Q_OBJECT;
  public:
    OutputFileWriter( const QString& file );
    bool failed() const;
  public slots:
    void receivedStdout(K3Process *proc, char *buffer, int buflen);
  private:
    QFile m_file;
    bool m_failed;
};

///A class that cares about the state of a document and it's history
class DocumentWrapper : public QObject, public SafeLogger, public Shared {
    Q_OBJECT;
  public:
    ///If readFile is true, the file is read from disk. Else we wait for the file from the other side. May throw QString on error.
    BIND_LIST_3( Messages, FileEditMessage, FileEditRejectMessage, FileSynchronize );

    DocumentWrapper( uint id, const LoggerPointer& logger, QString fileName, uint ownIndex, bool readFile, FileCollaborationSession* session, bool fromBuffer );

    QString fileName() const;

    KDevelop::IDocument* document();

    void updateTree( QModelIndex& i, QStandardItemModel* model );

    void fillContextMenu( int var, QMenu* menu );

    bool hadError() const;

    QString text() const;

    VectorTimestamp state() const;

    const QDynamicText& dynamicText() const;

    ///Returns an id that is unique within the wrapper's parent-session, and is equal on all collaborating peers.
    uint id() const;

    void processMessage( DocumentWrapperMessage* msg );

    ///If the document-wrapper is dead, that means the it is not part of the session anymore
    void setDead( bool dead );

    bool dead() const;
    ///This can be usedto completely disable the file(no editing is allowed anymore)
    void setDisabled( bool disabled );

    bool disabled() const;

    QIcon icon() const;

    bool synchronize( const UserPointer& user );

  signals:
    void publishEdit( const VectorTimestamp& state, const SimpleReplacement& replacement, DocumentWrapper* sender );

  public slots:
    void saveAsBufferFile();
    LocalPatchSourcePointer saveAsPatch( bool addToList = true, bool edit = true );

  private slots:
    void remove();
    void toggleEnabled();
    void saveNewBuffer();
    void manageDynamicText();

    void stateChanged();
    void documentDestroyed();
    void textChanged ( KTextEditor::Document * document,
                       const KTextEditor::Range & oldRange,
                       const KTextEditor::Range & newRange );
    void textRemoved ( KTextEditor::Document * document,
                       const KTextEditor::Range & range );
    void textInserted ( KTextEditor::Document * document,
                        const KTextEditor::Range & range );
  private:
    int receiveMessage( FileEditMessage* msg );
    int receiveMessage( FileSynchronize* msg );
    int receiveMessage( FileEditRejectMessage* msg );
    int receiveMessage( MessageInterface* msg );

    bool checkDocumentSync() const; ///Expensive, only for debugging

    void fillDocumentText();
    void openDocument( bool toForeground = false );

    ///If fromBuffer is true, it is tried to get the content from an already open buffer (@todo)
    void readFile( bool fromBuffer ) throw ( QString );

    virtual std::string logPrefix();

    QDynamicTextPointer m_text;

    bool m_block;

    bool m_realFile;
    QString m_fileName; ///Relative path of the file(from project-dir)
    KDevelop::IDocument* m_document;
    WeakSharedPtr<FileCollaborationSession> m_session;
    uint m_ownIndex;
    uint m_id;
    bool m_hadError, m_disabled, m_dead;

    QString m_tempFile;

    friend class DocumentWrapperTreeAction;

    friend class MessageDispatcher< DocumentWrapper, Messages > ;
    MessageDispatcher< DocumentWrapper, Messages > m_dispatcher;
};

typedef SharedPtr<DocumentWrapper> DocumentWrapperPointer;

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
