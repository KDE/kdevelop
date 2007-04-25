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

#ifndef MESSAGEUSERTAB_H
#define MESSAGEUSERTAB_H

#include <QObject>
#include <QPointer>
#include "autoconstructpointer.h"

#include "ui_kdevteamwork_list.h"
#include "network/pointer.h"

class QWidget;
class QListView;
class QStandardItemModel;
class KDevTeamworkUser;
class MessageSerialization;
typedef SafeSharedPtr<KDevTeamworkUser, BoostSerialization> KDevTeamworkUserPointer;

namespace Teamwork {
  class MessageInterface;
  typedef SafeSharedPtr< MessageInterface, MessageSerialization > MessagePointer;
};

class KDevTeamwork;

class MessageUserTab : public QObject, public Shared {
    Q_OBJECT;
		QPointer<QWidget> m_widget;
		QStandardItemModel* m_model;
		QListView* m_view;
		KDevTeamworkUserPointer m_user;
		KDevTeamwork* m_teamwork;

		/*MessageUserTab( const MessageUserTab& rhs );*/

		MessageUserTab& operator = ( const MessageUserTab& rhs );

		AutoConstructPointer<Ui_List> m_list;

	public:
		MessageUserTab( KDevTeamwork* tw, KDevTeamworkUserPointer user );

		~MessageUserTab();

		bool operator < ( const MessageUserTab& rhs );

		QStandardItemModel* model();

		QListView* view() ;

		KDevTeamworkUserPointer user();

		QWidget* widget();

    void messageUpdated( const Teamwork::MessagePointer& msg );

	public slots:
		void userStateChanged( KDevTeamworkUserPointer );
};

#endif
// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
