/*
   KDevelop Autotools Support
   Copyright (c) 2002 by Victor Roeder <victor_roeder@gmx.de>
   Copyright (c) 2005 by Matt Rogers <mattr@kde.org>

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
*/

#ifndef AUTOSUBPROJECTVIEW_H
#define AUTOSUBPROJECTVIEW_H

#include <qwidget.h>
#include "autoprojectviewbase.h"


class KAction;
class AutoToolsAction;
class AutoProjectWidget;
class AutoProjectPart;
class TargetItem;
class SubprojectItem;
class KListViewItem;
class KListView;

namespace AutoProjectPrivate
{
    bool isHeader( const QString& fileName );
}


class AutoSubprojectView : protected AutoProjectViewBase
{
	Q_OBJECT

	public:
		AutoSubprojectView(AutoProjectWidget* widget, AutoProjectPart* part, QWidget *parent, const char *name);
		virtual ~AutoSubprojectView();

	public:
		void loadMakefileams ( const QString& dir );

		void parse(SubprojectItem *item);
		KListView* listView() const { return m_listView; }

		TargetItem *findNoinstHeaders(SubprojectItem *item);
	
	signals:
		void selectionChanged( QListViewItem* );

	protected:
		void initActions ();

		void parseKDEDOCS(SubprojectItem *item,
		                  const QString &lhs, const QString &rhs);
		void parseKDEICON(SubprojectItem *item,
		                  const QString &lhs, const QString &rhs);
		void parsePrimary(SubprojectItem *item,
		                  const QString &lhs, const QString &rhs);
		void parsePrefix(SubprojectItem *item,
		                  const QString &lhs, const QString &rhs);
		void parseSUBDIRS(SubprojectItem *item,
		                  const QString &lhs, const QString &rhs);
		virtual void focusOutEvent(QFocusEvent *e);
		void expandCollapse( QListViewItem * item, bool expand );
		void expandCollapseFirst( QListViewItem * item, bool expand );

	private:
		AutoProjectWidget* m_widget;
		AutoProjectPart* m_part;
		QStringList headers;

		bool m_kdeMode;

		AutoToolsAction* addApplicationAction;
		AutoToolsAction* subProjectOptionsAction;
		AutoToolsAction* addSubprojectAction;
		KAction* addExistingSubprojectAction;
		AutoToolsAction* addTargetAction;
		AutoToolsAction* addServiceAction;
		AutoToolsAction* buildSubprojectAction;
		KAction* removeSubprojectAction;
		KAction* cleanSubprojectAction;
		KAction* forceReeditSubprojectAction;
		KAction* installSubprojectAction;
		KAction* installSuSubprojectAction;
		KAction* otherAction;
		KAction* expandAction;
		KAction* collapseAction;

		QStringList m_commandList;
		QValueList<int> m_commandTypeList;

	private slots:
		void slotContextMenu(KListView *, QListViewItem *item, const QPoint &p);
		//void slotSubprojectExecuted(QListViewItem* item);
		void slotSelectionChanged( QListViewItem* item );
		void slotAddApplication();
		void slotSubprojectOptions();
		void slotAddSubproject();
		void slotAddExistingSubproject();
		void slotAddTarget();
		void slotAddService();
		void slotBuildSubproject();
		void slotRemoveSubproject();
		void slotForceReeditSubproject();
		void slotInstallSubproject();
		void slotInstallSuSubproject();
		void slotCleanSubproject();
		void slotManageBuildCommands();
		void slotCustomBuildCommand(int);
		void slotExpandTree();
		void slotCollapseTree();
};

#endif

//kate: tab-width 4; space-indent off;
