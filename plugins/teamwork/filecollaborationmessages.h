
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


#ifndef FILECOLLABORATIONMESSAGES_H
#define FILECOLLABORATIONMESSAGES_H

#include <QPointer>
#include <QObject>
#include <QAction>

//#include <boost/serialization/extended_type_info_typeid.hpp>
#include "kdevteamwork_messages.h"
#include "network/easymessage.h"
#include "utils.h"
#include "dynamictext/vectortimestamp.h"
#include "dynamictext/dynamictexthelpers.h"
#include <list>
//#include <boost/serialization/extended_type_info.hpp>

namespace KTextEditor {
class Document;
};

struct CollabFile {
  uint id;
  QString file;
  CollabFile() : id(0) {
  }
  CollabFile( uint i, const QString& f ) : id(i), file(f) {
  }
  template <class Archive>
  void serialize( Archive& arch, const uint ) {
    arch & id;
    arch & file;
  }
};

typedef std::list<CollabFile> CollabFileList;

typedef quint64 CollabSessionId; //unsigned long long CollabSessionId;

class FileCollaborationMessageData {
  public:
    enum Message {
      NoMessage,
      Synchronize, ///messageData contains a QString containing the name of the file to synchronize
      CloseSession
  };

    FileCollaborationMessageData( CollabSessionId sessionId = 0, Message message = NoMessage, const QVariant& messageData = QVariant() ) : m_sessionId( sessionId ), m_message( message ), m_messageData( messageData ) {}


    template <class Archive>
    void serialize( Archive& arch, const uint /*version*/ ) {
      arch & m_sessionId;
      arch & m_message;
      arch & m_messageData;
    }

    ///Returns zero if the message is not associated to a session
    CollabSessionId sessionId() {
      return m_sessionId;
    };

    const QVariant& messageData() const {
      return m_messageData;
    }

    void setMessageData( const QVariant& v ) {
      m_messageData = v;
    }

    Message message() const {
      return m_message;
    }

    QString messageAsString() const {
      switch ( m_message ) {
        case NoMessage:
        return "NoMessage";
        case Synchronize:
        return "Synchronize";
        case CloseSession:
        return "CloseSession";
        default:
        return "unknown";
      };
    }

  private:
    CollabSessionId m_sessionId;
    Message m_message;
    QVariant m_messageData;
};

class FileCollaborationSession;
class FileCollaboration;

EASY_DECLARE_MESSAGE( FileCollaborationMessage, CollaborationMessage, 6, FileCollaborationMessageData, 3 );

/**A file-collaboration-request is sent to request collaboration. The request stays alive
 * in the session and keeps forwarding all replies to the FileCollaboration-object, until that
 * object is deleted. When the request is accepted, a FileCollaborationRequest is sent back
 * and stays alive in the other session, forwarding messages directly to the peer
 * FileCollaboration-object. That way the connection is established without the messages having
 * to go a very long way.
*/



class FileCollaborationRequestData : public QObject, public AbstractGUIMessage {
    Q_OBJECT;
  public:
    struct AcceptSignal {
    };
    struct DenySignal {
    };
    enum State {
      Unknown,
      Accepted,
      Denied,
      Failed
  };

    explicit FileCollaborationRequestData( FileCollaboration* collab = 0, uint index = 0, uint senderIndex = 0 );

    template <class Archive>
    void serialize( Archive& arch, unsigned int /*version*/ ) {
      arch & m_name;
      arch & m_files;
      arch & m_index;
      arch & m_senderIndex;
      arch & m_isAutomatic;
    }

    void setName( const QString& theValue );

    QString name() const;

    void setFiles( const CollabFileList& theValue );

    CollabFileList files() const;

    ///If the request is automatic that means that it was sent automatically by a session that discovered that a user came back online
    bool isAutomatic() const;

    void setAutomatic( bool automatic );

    State state() const;

    ///the index assigned to the side that is requested to collaborate
    uint index() const;

    void dispatchSignal( const AcceptSignal& sig );
    void dispatchSignal( const DenySignal& sig );

    ///The index of the sender of this message
    uint senderIndex() const;
  signals:
    void processReply( const MessagePointer& msg );
    void stateChanged( const MessagePointer& msg );
    void connectionResult( bool );

  public slots:
    void acceptCollaboration();
    void denyCollaboration();
  private:
    void updateState();
    ///Data:
    CollabFileList m_files;
    QString m_name;
    State m_state;
    uint m_index;
    uint m_senderIndex;
    bool m_isAutomatic;

    ///Internal:
    QPointer<FileCollaborationSession> m_session;
    QPointer<FileCollaboration> m_collab;
    QPointer<KDevTeamwork> m_teamwork;
    friend class FileCollaborationRequest;

    QAction* m_acceptAction;
    QAction* m_denyAction;
};

///A FileCollaborationRequest is an invitation to a local file-collaboration-session.
EASY_DECLARE_MESSAGE_BEGIN( FileCollaborationRequest, FileCollaborationMessage, 2, FileCollaborationRequestData, 3 );

virtual bool needReply() const;

virtual void result( bool success ) {
  if ( !success ) {
    m_state = Failed;
    updateState();
  }
}

virtual QString messageText() const;

virtual QIcon messageIcon() const;

virtual ReplyResult gotReply( const MessagePointer& p );

virtual void fillContextMenu( QMenu* /*menu*/, KDevTeamwork* /*teamwork*/ );

END();

typedef SafeSharedPtr<FileCollaborationMessage> FileCollaborationMessagePointer;
typedef SafeSharedPtr<FileCollaborationRequest> FileCollaborationRequestPointer;

;

class DocumentWrapperMessageData {
  public:
    DocumentWrapperMessageData( uint wrapperId = 0 ) : m_wrapperId( wrapperId ) {}

    template <class Archive>
        void serialize( Archive& arch, unsigned int /*version*/ ) {
      arch & m_wrapperId;
        };
        uint wrapperId() {
          return m_wrapperId;
        }
  private:
    uint m_wrapperId;
};

EASY_DECLARE_MESSAGE( DocumentWrapperMessage, FileCollaborationMessage, 4, DocumentWrapperMessageData, 1 );

class FileEditMessageData {
  public:
    template <class Archive>
    void serialize( Archive& arch, unsigned int /*version*/ ) {
      arch & m_replacement & m_state;
    }

    FileEditMessageData( const VectorTimestamp& state = VectorTimestamp(), const SimpleReplacement& rep = SimpleReplacement() ) : m_replacement( rep ), m_state(state) {
    }

    const SimpleReplacement& replacement () const {
      return m_replacement;
    }

    const VectorTimestamp& timeStamp() const {
      return m_state;
    }

  private:
    SimpleReplacement m_replacement;
    VectorTimestamp m_state;
};

EASY_DECLARE_MESSAGE( FileEditMessage, DocumentWrapperMessage, 1, FileEditMessageData, 2 );

struct FileEditRejectMessageData {
  public:
    FileEditRejectMessageData( const VectorTimestamp& vec = VectorTimestamp() ) : m_state( vec ) {
    }

    template <class Archive>
    void serialize( Archive& arch, unsigned int /*version*/ ) {
      arch & m_state;
    }

    const VectorTimestamp& timeStamp() const {
      return m_state;
    }
  private:
    VectorTimestamp m_state;
};

EASY_DECLARE_MESSAGE( FileEditRejectMessage, DocumentWrapperMessage, 2, FileEditRejectMessageData, 1 );

struct FileListMessageData {
  FileListMessageData( const CollabFileList& files = CollabFileList() ) : m_files( files ) {
  }

  template <class Archive>
  void serialize( Archive& arch, unsigned int /*version*/ ) {
    arch & m_files;
  }

  CollabFileList m_files;
};


EASY_DECLARE_MESSAGE( FileListMessage, FileCollaborationMessage, 5, FileListMessageData, 1 );

BOOST_CLASS_IMPLEMENTATION( FileEditRejectMessageData, boost::serialization::object_serializable );
BOOST_CLASS_IMPLEMENTATION( SimpleReplacement, boost::serialization::object_serializable );
BOOST_CLASS_IMPLEMENTATION( FileCollaborationMessageData, boost::serialization::object_serializable );
BOOST_CLASS_IMPLEMENTATION( FileCollaborationRequestData, boost::serialization::object_serializable );
BOOST_CLASS_IMPLEMENTATION( FileEditMessageData, boost::serialization::object_serializable );


#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
