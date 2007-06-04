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

#ifndef MESSAGEHISTORYMANAGER_H
#define MESSAGEHISTORYMANAGER_H


#include <QList>
#include <QMap>

#include <klockfile.h>
#include <kurl.h>

#include "network/user.h"
#include "kdevteamwork_messages.h"
#include "teamworkfwd.h"
#include <nvp.h>


class QTimer;
class KDevTeamworkTextMessage;

typedef SafeSharedPtr<KDevTeamworkTextMessage, MessageSerialization> HistoryMessagePointer;

struct HistoryMessageDesc
{
	UserIdentity user;
	bool isIncoming;
	HistoryMessagePointer message;

	HistoryMessageDesc();

	HistoryMessageDesc( const HistoryMessagePointer& msg );

	template<class Archive>
	void load( Archive& arch, const uint /*version*/ ) {
		arch & NVP( user );
		arch & NVP( isIncoming );
		arch & NVP( message );
		HistoryMessagePointer::Locked l = message;
		if( l ) {
			l->info().setIsIncoming( isIncoming );
		}
	}

	template<class Archive>
	void save( Archive& arch, const uint /*version*/ ) const {
		arch & NVP( user );
		arch & NVP( isIncoming );
		arch & NVP( message );
	}

	operator bool() {
		return (bool) message;
	}

	BOOST_SERIALIZATION_SPLIT_MEMBER()
};

struct HistoryGroupLocation {
	string fileName;
	uint offset;

	template<class Archive>
	void serialize( Archive& arch, const uint /*version*/ ) {
		arch & fileName;
		arch & offset;
	}
};


class MessageHistoryManager : public QObject {
	Q_OBJECT
	public:
		typedef QMap<Teamwork::UserIdentity, bool > UserSet;


		MessageHistoryManager( Teamwork::LoggerPointer logger );
		~MessageHistoryManager();

		void addMessage( KDevTeamworkTextMessage* msg );

		///Returns all matching messages. If user is zero, returns messages from/to all users. This is slow(always reads the messages from disk)
		QList<HistoryMessagePointer>  getMessages( const KDevTeamworkClientPointer& client, QDate from = QDate(), QDate until = QDate(), UserSet users = UserSet() );

		HistoryMessagePointer  getMessageFromId( Teamwork::UniqueMessageId id, const KDevTeamworkClientPointer& client );

	signals:
		void newMessage( HistoryMessagePointer msg ); ///This is emitted whenever a new message should be added to the history

	private  slots:
		void writePending();

	private:
		HistoryMessagePointer fillMessageUser( const HistoryMessageDesc& message, const KDevTeamworkClientPointer& client );

    QString lockFileName();

		QList<HistoryMessagePointer> fillMessageUsers( const QList<HistoryMessageDesc>& messages, const KDevTeamworkClientPointer& client );
		///Reads out the messages of just one chunk(the messages that follow a single header)
		void readMessages( const QString& file, uint offset, QList<HistoryMessageDesc>& messages ) throw(QString);

		void readIndex( std::map<Teamwork::UniqueMessageId, HistoryGroupLocation>& index ) throw( QString);

		void writeIndex( const std::map<Teamwork::UniqueMessageId, HistoryGroupLocation>& index ) throw( QString);

		///May throw QString
		KUrl directory() throw(QString);

		QTimer* m_pendingTimer;
		QList< HistoryMessageDesc > m_pending;
		Teamwork::LoggerPointer m_logger;
    KLockFile::Ptr m_lockFile;

		Teamwork::LoggerPrinter out( Teamwork::Logger::Level = Teamwork::Logger::Info );

		Teamwork::LoggerPrinter err();
};

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
