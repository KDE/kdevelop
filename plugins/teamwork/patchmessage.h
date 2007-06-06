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

#ifndef PATCHMESSAGE
#define PATCHMESSAGE

#include <list>
#include <string>
#include <vector>
#include <QObject>
#include <QDataStream>
#include <QByteArray>
#include <QMetaType>
#include <QStandardItemModel>
#include <kurl.h>
#include <ksharedptr.h>

#include "network/serialization.h"
#include "network/safesharedptr.h"
#include "network/messagetypeset.h"
#include "network/easymessage.h"

#include "kdevteamwork_messages.h"
#include "utils.h"
#include "nvp.h"

#include <boost/serialization/split_member.hpp>

namespace KIO {
	class Job;
}
class KMimeType;
class KJob;

QStringList splitArgs( const QString& str );

enum PatchAccessRights {
	None,
	Public,
	ConnectedOnly,
	Private,
	Ask ///This means that the patch is publically visible, but on request, the owner is asked whether it really should be sent.
};

class LocalPatchSource : public SafeShared {

	///This class should be used for identification instead of just the name, because the type of comparison might change in future
	public:
		class Identity {
		private:
			string name_;
		public:
			Identity( const LocalPatchSource& src ) : name_(src.name) {
			}

			template<class Archive>
			void serialize( Archive& arch, const uint /*version*/ ) {
				arch & name_;
			}

			Identity() {
			}

			bool operator == ( const Identity& rhs ) const {
				return name_ == rhs.name_;
			}

			string desc() const {
				return name_;
			}
	};

	enum State {
		Applied,
		NotApplied,
		Unknown
	};

	string stateAsString() {
		switch( state ) {
			case Applied:
				return "Applied";
			case  NotApplied:
				return "NotApplied";
			default:
				return "Unknown";
		};
	}

	Identity identity()  {
		return Identity( *this );
	}
	string name;
	string filename;
	string command;
	string type; ///This is the mime-type that was chosen for this item
	string applyCommand;
	string unApplyCommand;
	string dependencies;
	string description;
	string author;
	State state;

  UserIdentity userIdentity;
	PatchAccessRights access;

  LocalPatchSource()  : state( Unknown ), access( Private ) {
	}

	void setFileName( const string& str ) {
		filename = str;
		command = "";
	}


  ///Tries to determine the patch-depth(by searching for -pX parameters in the apply-command. Defaults to zero.
  uint patchDepth() const;

  ///Tries to determine the tool to be used for applying the patch. Defaults to "patch"
  string patchTool( bool reverse = false ) const;

  ///Tries to determine the params for applying the patch. If none are given, uses reasonable default-parameters. If reverse is true, the unapply-parameters are given.
  string patchParams( bool reverse = false ) const;

	template<class Archive>
	void serialize( Archive& arch, const uint /*version*/ ) {
		KUrl u( ~filename );
		filename = ~u.prettyUrl();  ///If there is a password, leave it out

		arch & NVP( filename );
		arch & NVP( command );
		arch & NVP( name );
		arch & NVP( type );
		arch & NVP( applyCommand );
		arch & NVP( unApplyCommand );
		arch & NVP( dependencies );
		arch & NVP( description );
//		arch & NVP( mimetype );
		arch & NVP( access );
		arch & NVP( state );
    arch & NVP( author );
    arch & NVP( userIdentity );
  }

		static string accessToString( PatchAccessRights access ) {
			switch( access ) {
				case None:
					return "none";
				case Public:
					return "public";
				case ConnectedOnly:
					return "connected only";
				case Private:
					return "private";
				case Ask:
					return "request";
			};
			return "unknown";
		}

		static PatchAccessRights accessFromString( const string& txt ) {
			if( txt == "public")
				return Public;

			if( txt == "connected only" )
				return  ConnectedOnly;

			if( txt == "request" )
				return Ask;

			if( txt == "private" )
				return Private;

			return None;
		}

		string accessAsString() {
			return accessToString( access );
		}

		bool operator == ( const LocalPatchSource& rhs ) const {
			return name == rhs.name;
		}

		QIcon getIcon( IconCache& icons );

		void setUser( const UserPointer& u ) {
			user_ = u;
		}

		const UserPointer& user() {
			return user_;
		}

		/*void setMimeType( KSharedPtr<KMimeType> mimeType );

    KSharedPtr<KMimeType> getMimeType();*/

	private:
		UserPointer user_;
};

//BOOST_CLASS_EXPORT( LocalPatchSource )

typedef SafeSharedPtr<LocalPatchSource> LocalPatchSourcePointer;


///This message is just a base-class for all messages that should be forwarded to the PatchesListManager
class PatchesManagerMessage : public SystemMessage
{
	DECLARE_MESSAGE( PatchesManagerMessage, SystemMessage, 6 );

	enum Message {
		None = 50,
		GetPatchesList
	};

	PatchesManagerMessage( InArchive& arch, const Teamwork::MessageInfo& info ) : Precursor( arch, info ) {
	}

	explicit PatchesManagerMessage( const Teamwork::MessageConstructionInfo& info, Message msg = None ) : Precursor( info(this), (SystemMessage::Message)msg, "" ) {
	}

	Message message() {
		return (Message) SystemMessage::message();
	}
};

typedef SafeSharedPtr<PatchesManagerMessage> PatchesManagerMessagePointer;


class PatchesListMessage : public PatchesManagerMessage
{
	DECLARE_MESSAGE( PatchesListMessage, PatchesManagerMessage, 1 );
	private:

		template<class Arch>
		void serial( Arch& arch ) {
			arch & patches;
		}

	public:
		list<LocalPatchSource> patches;

		PatchesListMessage( InArchive& arch, const Teamwork::MessageInfo& info );

		PatchesListMessage( const Teamwork::MessageConstructionInfo& info, list<LocalPatchSourcePointer>& _patches ) : Precursor( info(this), None ) {
			for( list<LocalPatchSourcePointer>::iterator it = _patches.begin(); it != _patches.end(); ++it ) {
				LocalPatchSourcePointer::Locked l = *it;
				if( l ) {
					patches.push_back( *l );
				} else {
				///could not lock the patch-source
				}
			}
		}

		virtual void serialize( OutArchive& arch );
};

class PatchRequestMessage;

class PatchRequestData : public AbstractGUIMessage {
	public:
	enum Status {
		Waiting,
		Denied,
		Accepted,
		Failed,
		Unknown
	};

	enum RequestType {
		View,
		Download,
		Apply
	};

  explicit PatchRequestData( const LocalPatchSourcePointer& id = LocalPatchSourcePointer(), KDevTeamwork* tw = 0, RequestType req = View );
  virtual ~PatchRequestData();

	template<class Arch>
	void serialize( Arch& arch, const uint /*version*/  ) {
		arch & ident_;
		arch & requestType_;
	}

	LocalPatchSource::Identity patchIdentity() {
		return ident_;
	}

	void setStatus( Status st ) {
		stat = st;
	}

	string patchDesc() const {
		return ident_.desc();
	}

	RequestType requestType() const {
		return requestType_;
	}

  ///This returns the LocalPatchSourcePointer which was used to request the patch(only valid on sender-side, to cache the apply-commands etc. for security)
  LocalPatchSourcePointer request() const {
    return request_;
  }

	virtual void fillContextMenu( QMenu* menu, KDevTeamwork* teamwork );

	virtual QIcon messageIcon() const;

	virtual QString messageText() const;

	PatchRequestMessage* selfMessage() ;
	const PatchRequestMessage* selfMessage()  const;

	private:
		LocalPatchSource::Identity ident_;
    LocalPatchSourcePointer request_;
		RequestType requestType_;
	protected:
		Status stat;
    SafeTeamworkEmitterPointer emitter;
};

EASY_DECLARE_MESSAGE_BEGIN( PatchRequestMessage, PatchesManagerMessage, 11, PatchRequestData, 3 )
		virtual bool needReply () const { return true; }
		virtual ReplyResult gotReply( const MessagePointer& /*p*/ );
END();

class PatchData;
class K3Process;

///This needs a helper, because the object must be created in the same thread.
class PatchDataReceiver : public QObject {
	Q_OBJECT
	PatchData* data;
	public:
		PatchDataReceiver( PatchData* d );
	public slots:
		void receivedStdout(K3Process *proc, char *buffer, int buflen);
		void transferData( KIO::Job*, const QByteArray& );
		void transferFinished( KJob *job );
//		void transferSpeed( KIO::Job *job, unsigned long speed );
};


class PatchData {
	public:
		explicit PatchData( const LocalPatchSourcePointer& p = LocalPatchSourcePointer(), LoggerPointer logg = LoggerPointer() );

	//template<class Arch>
	void load( InArchive& arch, const uint /*version*/  );

	//template<class Arch>
	void save( OutArchive& arch, const uint version  ) const {
		const_cast<PatchData*>(this)->saveInternal( arch, version );
	}

	void receivedStdout(K3Process *proc, char *buffer, int buflen);

	///This is only valid if the message was not created locally.
	const QByteArray& data();

	bool isBinary();

	///Here the signals from TransferJob arrive.
	void transferData( KIO::Job*, const QByteArray& );

	void transferFinished();

  void transferCanceled();

	LocalPatchSourcePointer patch();

	BOOST_SERIALIZATION_SPLIT_MEMBER()
	private:

	//template<class Arch>
	void saveInternal( OutArchive& arch, const uint /*version*/  );

	LocalPatchSourcePointer m_patch;
	LoggerPointer logger;
	OutArchive* currentArchive;
	bool errored;
	bool deserialized, finished, isBinary_;
	QByteArray m_data; ///When this has no content, the patch should be computed from the information in LocalPatchSource while serializing.
	KUrl projectDir;


	enum EntryType {
		End,
		BinaryHeader,
		TextHeader,
		Text,
		Vector,
		None
	};

	LoggerPrinter log( Logger::Level level = Logger::Debug ) {
		return LoggerPrinter( logger, level ) << "PatchMessage: ";
	}
};

EASY_DECLARE_MESSAGE( PatchMessage, PatchesManagerMessage, 6, PatchData, 2 );

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
