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

#ifndef CONVERSATIONMANAGER_H
#define CONVERSATIONMANAGER_H
#include <list>
#include <map>

#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <QObject>
#include <QTimer>
#include "kdevteamwork_internalmessage.ui.h"
#include <ktexteditor/document.h>
#include <ktexteditor/smartcursor.h>
#include <ktexteditor/smartinterface.h>
#include <QtCore/QPointer>
#include "kdevteamwork_user.h"
#include "messageserialization.h"
#include "messagehistorymanager.h"
#include "crossmap.h"
#include <QTime>
#include "safelogger.h"

class QStandardItemModel;
class TemporaryConversationConnector;
class KDevTeamworkClient;

namespace KDevelop {
  class IDocument;
};

typedef SafeSharedPtr<InDocumentMessage, MessageSerialization> InDocumentMessagePointer;

///This is a class that helps detecting when the document is deleted and then deleting the smartcursor, also it automatically creates the cursor. @todo replace with QOwnedPointer
class SmartCursorContainer {
    QPointer<KTextEditor::Document> m_document;
    KTextEditor::SmartCursor* m_smartCursor;
  public:
    SmartCursorContainer( KTextEditor::Document* doc = 0 ) : m_document( doc ), m_smartCursor( 0 ) {
      if ( m_document ) {
        KTextEditor::SmartInterface * smart =
          qobject_cast<KTextEditor::SmartInterface*>( doc );

        if ( smart )
          m_smartCursor = smart->newSmartCursor();
      }
    }
    KTextEditor::SmartCursor* operator -> () {
      return m_smartCursor;
    }
    ~SmartCursorContainer() {
      if ( m_smartCursor )
        delete m_smartCursor;
      m_smartCursor = 0;
    }
    KTextEditor::SmartCursor& operator *() {
      return * m_smartCursor;
    }
    SmartCursorContainer( const SmartCursorContainer& rhs ) {
      if ( &rhs == this )
        return ;
      if ( m_smartCursor && m_document )
        delete m_smartCursor;
      m_document = rhs.m_document;
      m_smartCursor = rhs.m_smartCursor;
      const_cast<SmartCursorContainer&>( rhs ).m_smartCursor = 0;
    }
    SmartCursorContainer& operator = ( const SmartCursorContainer& rhs ) {
      if ( &rhs == this )
        return * this;
      if ( m_smartCursor && m_document )
        delete m_smartCursor;
      m_document = rhs.m_document;
      m_smartCursor = rhs.m_smartCursor;
      const_cast<SmartCursorContainer&>( rhs ).m_smartCursor = 0;
      return *this;
    }

    operator bool() {
      return m_document && m_smartCursor && m_smartCursor->isValid();
    }
};

namespace KTextEditor {
class View;
class Cursor;
class SmartCursor;
};
class InDocumentMessage;
enum LogLevel;
class ConversationManager;
class QWidget;
class SmartCursor;

CROSSMAP_DEFINE_CONTAINER( QList );

struct OrderedDocumentMessage {
  int position;
  InDocumentMessagePointer message;
  OrderedDocumentMessage( uint pos, const InDocumentMessagePointer& msg ) : position( pos ), message( msg ) {}
  OrderedDocumentMessage() : position( 0 ), message( 0 ) {}

  operator bool() const {
    return ( bool ) message;
  }
  /*template<class Archive>
  void serialize( Archive& arch, const uint version ) {
    arch & position & message;
  }*/
};

class InDocumentConversation : public QObject, public Shared, public SafeLogger {
    Q_OBJECT
    ///Order, message, file
    BIND_LIST_3( Keys, InDocumentMessagePointer, int, QString );
    typedef Utils::CrossMap< OrderedDocumentMessage, Keys > MessageSet;
  public:
    InDocumentConversation( InDocumentMessage* msg = 0 );
    ~InDocumentConversation();

    void addMessage( InDocumentMessage* msg );

    template <class ArchType>
    void load( ArchType& arch, unsigned int version );

    template <class ArchType>
    void save( ArchType& arch, unsigned int version ) const;

    ConversationManager* manager() const;

    void fillContextMenu( QMenu* menu, KDevTeamwork* teamwork, MessagePointer msg );

    ///Returns true if the message belongs to this conversation
    bool match( InDocumentMessage* msg ) const;

    void setActive( bool );

    UserPointer primaryUser();

  public slots:
    void userStateChanged( KDevTeamworkUserPointer );
    void documentActivated( KDevelop::IDocument* document );
    void verticalScrollPositionChanged ( KTextEditor::View *view, const KTextEditor::Cursor& newPos );
    void cursorPositionChanged ( KTextEditor::View *view, const KTextEditor::Cursor& newPos );
    void horizontalScrollPositionChanged ( KTextEditor::View *view );
    void jumpTo();
    void hide();
    void sendMessage();
    void gotReply( MessagePointer msg );

    void messageClicked( const QModelIndex& );
    void messageContextMenu ( const QPoint & );
    void selectMessage( InDocumentMessagePointer msg );

    void userInfo();
    void log( QString str, LogLevel level = Info ) const;
    void selectNearestMessage();
  private slots:
    void textChanged ( KTextEditor::Document * document, const KTextEditor::Range & oldRange, const KTextEditor::Range & newRange );
    void textRemoved ( KTextEditor::Document * document, const KTextEditor::Range & range );
    void textInserted ( KTextEditor::Document * document, const KTextEditor::Range & range );

    void rangeDeleted();
  private:
    virtual std::string logPrefix();

    InDocumentMessagePointer selectedMessage();
    
    void documentActivated( KDevelop::IDocument* document, const InDocumentMessagePointer& msg );

    void messageSelected( const MessagePointer& msg );
    template <class ArchType>
    void serial( ArchType& arch, unsigned int /*version*/ ) {
      arch & m_documentName;
      arch & m_line;
      arch & m_active;
    }
    void pushMessage( const InDocumentMessagePointer& msg );

    KTextEditor::Cursor currentDocCursor() const;

    QString context() const;
    void messageSendReady( bool success );
    void addListItem( const QString& txt, const QString& icon = "error" );
    SessionPointer session();
    void fillUserBox();
    void fillMessageModel();
    void fillMessageToModel( const InDocumentMessagePointer::Locked& );
    void setupWidget( QWidget* parent );
    void embedInView( KTextEditor::View* view, KDevelop::IDocument* document, KTextEditor::Cursor position );
    KTextEditor::Cursor findPositionInDocument( InDocumentMessagePointer::Locked l, KTextEditor::Cursor* endTarget = 0 );
    void placeWidget( KTextEditor::View* view, const KTextEditor::Cursor* awayFrom = 0, bool forceShow = false );

    string m_documentName;
    int m_line;
    int m_conversationId;
    bool m_active;
    uint m_messageCount;

    //list<InDocumentMessagePointer> m_messages;
    MessageSet m_messages;

    QPointer<QWidget> m_widget;
    Ui_InternalMessage m_widgets;
    SmartCursorContainer m_smartCursor;
    QAction* m_jumpToAction;
    QAction* m_hideAction;
    QAction* m_userInfoAction;
    QStandardItemModel* m_messagesModel;
    InDocumentMessagePointer m_sendingMessage;
    SharedPtr<TemporaryConversationConnector> m_userConnector;
    bool m_block;

    QTimer* m_selectNearestMessageTimer;

    QPointer<KTextEditor::Document> m_currentConnectedDocument;

    KTextEditor::SmartRange* m_currentRange;

    InDocumentReference::TextSearchInstance m_currentSearchInstance;

    QTime m_lastSendTime;
  public:
    BOOST_SERIALIZATION_SPLIT_MEMBER();
};

typedef SharedPtr<InDocumentConversation> InDocumentConversationPointer;

class ConversationManager : public QObject {
    Q_OBJECT
  public:

    ConversationManager( MessageManager* mng );
    ~ConversationManager();

    int processMessage( InDocumentMessage* msg );

    static ConversationManager* instance() {
      return m_instance;
    }

    void log( QString str, LogLevel level );

    /*    template <class ArchType>
      void serialize( ArchType& arch, unsigned int ) {
        arch & m_conversations;
      }*/

    MessageManager* manager() const {
      return const_cast<MessageManager*>( m_manager );
    }

    ///Returns zero if the conversation does not exist.
    InDocumentConversationPointer findConversation( QString context );

    ///If the conversation does not exist, it is created.
    InDocumentConversationPointer getConversation( InDocumentMessage* );

  public slots:
    void load();
    void save();
    void documentActivated( KDevelop::IDocument* document );

  private:
    static ConversationManager* m_instance;
    typedef map<string, InDocumentConversationPointer> ConversationSet;
    ConversationSet m_conversations; ///maps conversation-ids to conversations
    MessageManager* m_manager;
    static ConversationManager* globalManager;
};

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
