/***************************************************************************
                             -------------------
    begin                : 19.01.2003
    copyright            : (C) 2002 by Victor R�er
    email                : victor_roeder@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef AUTOSUBPROJECTVIEW_H
#define AUTOSUBPROJECTVIEW_H

#include <qwidget.h>

#include <klistview.h>


class KAction;

class AutoProjectWidget;
class AutoProjectPart;
class TargetItem;
class SubprojectItem;

namespace AutoProjectPrivate
{
    bool isHeader( const QString& fileName );
}


class AutoSubprojectView : public KListView
{
	Q_OBJECT

	public:
		AutoSubprojectView(AutoProjectWidget* widget, AutoProjectPart* part, QWidget *parent, const char *name);
		virtual ~AutoSubprojectView();

	public:
		void loadMakefileams ( const QString& dir );

		void parse(SubprojectItem *item);

        TargetItem *findNoinstHeaders(SubprojectItem *item);

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

	private:
		AutoProjectWidget* m_widget;
		AutoProjectPart* m_part;
		QStringList headers;

		bool m_kdeMode;

		KAction* addApplicationAction;
		KAction* subProjectOptionsAction;
		KAction* addSubprojectAction;
		KAction* addExistingSubprojectAction;
		KAction* addTargetAction;
		KAction* addServiceAction;
		KAction* buildSubprojectAction;
		KAction* removeSubprojectAction;
		KAction* cleanSubprojectAction;
        KAction* forceReeditSubprojectAction;
        KAction* installSubprojectAction;
        KAction* installSuSubprojectAction;

	private slots:
		void slotContextMenu(KListView *, QListViewItem *item, const QPoint &p);
// 		void slotSubprojectExecuted(QListViewItem* item);

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
};

#endif
