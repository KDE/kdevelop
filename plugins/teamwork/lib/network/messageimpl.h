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

#ifndef MESSAGEIMPL
#define MESSAGEIMPL

#include "message.h"
#include "helpers.h"
#include "interfaces.h"

///Since objects must have external linkage to be usable as template-arguments, these must be defined here.

namespace Teamwork {

/**	Example for a custom Message:
class CustomMessage : public RawMessage {
	DECLARE_MESSAGE( CustomMessage, RawMessage, 83 );
	typedef vector<MyType> MyDataType;
	MyDataType myData;
	
	template<class Arch>
	void ser( Arch& arch ) {
		arch & myData;
}
	public:

	CustomMessage( const MessageInfo& info, MyDataType& data ) : 	RawMessage( info, id ), myData( data ) {
}

	CustomMessage( InArchive& arch, const MessageInfo& info ) : RawMessage( arch, info ) {
		ser( arch );
}

	virtual bool serialize( OutArchive& arch ) {
	  if( !Precursor::serialize( arch ) ) return false;
		ser( arch );
		return true;
}
};
 */

	class RawMessage : public MessageInterface
	{
		DECLARE_MESSAGE( RawMessage, MessageInterface, 1 );

		MessageInfo info_;
		DataVector body_;

		public:
		
			RawMessage( const MessageInfo& info, const DataVector& data );

			///This should be used to indicate that this message is a reply to the other message(replyTarget)
			void setReply( MessageInterface* replyTarget );
			
			RawMessage( InArchive& from, const MessageInfo& info );
			
			virtual void serialize( OutArchive& target );

			virtual const MessageInfo& info() const;

			virtual MessageInfo& info();

			/**This is called once a message has been tried to be sent, with the appropriate result, and can be used by the message to give some feedback to the sender. The Session used to send the message is still locked.
			 */
			virtual void result( bool success );
			
			/**Returns a reference to the data-store of this raw message. It does not include data serialized to inherited messages. */
			DataVector& body();
			
			/**Returns a reference to the data-store of this raw message. It does not include data serialized to inherited messages. */
			const DataVector& body() const;
	};


	class TextMessage : public RawMessage {
		DECLARE_MESSAGE( TextMessage, RawMessage, 3 );
		public:
			TextMessage( const MessageInfo& info, const std::string& text = "" );
			
			TextMessage( InArchive& from, const MessageInfo& info );
	
			std::string text() const;
	};

	struct SystemMessage : public TextMessage
	{
		DECLARE_MESSAGE( SystemMessage, TextMessage, 2 );
		public:
			enum Message {
				NoMessage,
				LoginSuccess,
				LoginFailedUnknown,
				BadAuthentication,
				ServerBusy,
				Kicked,
				BadTarget,
				StoredOnServer,
				AlreadyLoggedIn,
				GetUserList,
				SerializationFailed
			};
		private:
			Message msg_;
		
			template<class Arch>
			void serial( Arch& arch ) {
				arch & msg_;
			}
		public:
			SystemMessage( const MessageInfo& info, Message msg, const string& ptext="" );

			SystemMessage( InArchive& arch, const MessageInfo& info );

			virtual void serialize( OutArchive& arch );

			Message message();
	
			string messageAsString();
	};
	
	typedef Binder< RawMessage > ::
	Append< TextMessage >
	::Result::Append<SystemMessage>::Result StandardMessageTypes;
}

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on

#endif
