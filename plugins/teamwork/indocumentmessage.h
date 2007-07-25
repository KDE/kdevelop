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

#ifndef INDOCUMENTMESSAGE_H
#define INDOCUMENTMESSAGE_H

#include "kdevteamwork_messages.h"
#include "indocumentreference.h"

class DocumentMessageInternal;
class InDocumentConversation;

class InDocumentMessage : public KDevTeamworkTextMessage {
    DECLARE_MESSAGE( InDocumentMessage, KDevTeamworkTextMessage, 3 );
		InDocumentReference m_start, m_end;
		string m_context;
		auto_ptr<DocumentMessageInternal> m_internal;

		DocumentContextLines m_contextLines;

		enum {
			MessageVersion = 1
		};

		template <class Arch>
				void serial( Arch& arch ) {
			int v = MessageVersion;
			arch & v;

			arch & m_start & m_end & m_context;

			if ( v >= 1 )
				arch & m_contextLines;
				}

	public:

		InDocumentMessage( const Teamwork::MessageConstructionInfo& info, const QString& text, const InDocumentReference& startRef, const InDocumentReference& endRef, const QString& context );

		InDocumentMessage( InArchive& from, const Teamwork::MessageInfo& info );

		virtual void serialize( OutArchive& arch );

    ///Context means the name of a conversation-thread
		QString context();

    ///Can be used to get and set the context-lines
		DocumentContextLines& contextLines() ;

		QString document();

		InDocumentReference& start();

		InDocumentReference& end() ;

		virtual void fillContextMenu( QMenu* menu, KDevTeamwork* teamwork );

		virtual QIcon messageIcon() const;

		void setConversation( InDocumentConversation* conv );

		virtual bool needReply() const;

		virtual void result( bool success );

		virtual ReplyResult gotReply( const MessagePointer& /*p*/ );

		virtual QString shortName() const;

		virtual void showInWidget( QWidget* widget, KDevTeamwork* tw );
};

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
