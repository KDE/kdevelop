#ifndef MESSAGEHISTORY_H
#define MESSAGEHISTORY_H

#include "ui_kdevteamwork_messagehistory.h"
#include <QList>
#include <QObject>
#include <QWidget>
#include "kdevteamwork_user.h"
#include "safelogger.h"

namespace Teamwork {
	class Logger;
	typedef SafeSharedPtr<Logger> LoggerPointer;
};


class MessageManager;
class KDevTeamworkTextMessage;

typedef SafeSharedPtr<KDevTeamworkTextMessage, MessageSerialization> HistoryMessagePointer;

class QStandardItemModel;

class GuiMessageHistory : public QObject, public SafeLogger {
	Q_OBJECT;
	public:
		typedef QList<KDevTeamworkUserPointer> UserList;
		typedef QMap<KDevTeamworkUserPointer, bool> UserSet;
		///Empty list means All Users
		GuiMessageHistory( MessageManager* manager, UserList users = UserList(), QString context = "" );
		~GuiMessageHistory();

	public slots:
		void slotAllMessages();
		void slotAllDevelopers();
		void slotNewMessage( const HistoryMessagePointer& );
		void slotUserStateChanged( const KDevTeamworkUserPointer& user );
		void slotSelectionChanged();
		void slotUpdateMessages();
		void slotExit();
		void messageItemClicked( const QModelIndex& index );
		void updateMessageUserIcons();

		///Filters:
		void typeFilterChanged( int index );
		void contextFilterChanged( int index );

	private:
		bool fitMessageDate( const HistoryMessagePointer& msg );
		void showMessage( const HistoryMessagePointer& msg );
		void fillDeveloperList();
		void updateUserIcon( const KDevTeamworkUserPointer& user );
		UserSet selectedUsers();
		void fillMessages();
		
		void clearFilters();
		void  applyFilters( int firstN = 0 );

		virtual std::string logPrefix() {
			return "GuiMessageHistory: ";
		}

		void clearUsers();
		void addUser( const KDevTeamworkUserPointer& user );

		bool m_allUsers;
		UserSet m_users;
		MessageManager* m_manager;
		Ui_MessageHistory m_widgetData;
		QStandardItemModel* m_developersModel;
		QStandardItemModel* m_messagesModel;
		QTimer* m_updateTimer;
		QTimer* m_userIconUpdateTimer;
		QDialog* m_dialog;
		QString m_defaultContext;

		enum TableColumns {
			TimeColumn = 0,
			TypeColumn,
			UserColumn,
			TextColumn,
			ColumnCount
		};

		QList<KDevTeamworkUserPointer> m_changedUsers;
};

#endif
