/* This file is part of KDevelop
    Copyright 2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2007 Andreas Pakulat <apaku@gmx.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPROJECTTREEVIEW_H
#define KDEVPROJECTTREEVIEW_H

#include <QtGui/QTreeView>
#include <QtCore/QPointer>

class KUrl;
class QAction;
class QItemSelectionModel;
class QMouseEvent;

class ProjectManagerViewPlugin;
namespace KDevelop
{
class IProject;
class ProjectModel;
class ProjectBaseItem;
class ProjectFolderItem;
class ProjectFileItem;
class ProjectTargetItem;
class ProjectBaseItem;
class NavigationToolTip;
}

class ProjectTreeView: public QTreeView
{
        Q_OBJECT
    public:
        ProjectTreeView( QWidget *parent = 0 );
        virtual ~ProjectTreeView();

        ProjectManagerViewPlugin *plugin() const;
        KDevelop::ProjectModel *projectModel() const;

        KDevelop::ProjectFolderItem *currentFolderItem() const;
        KDevelop::ProjectFileItem *currentFileItem() const;
        KDevelop::ProjectTargetItem *currentTargetItem() const;

        virtual bool event(QEvent* event);


    Q_SIGNALS:
        void activateUrl( const KUrl &url );

    protected Q_SLOTS:
        void slotActivated( const QModelIndex &index );
        void popupContextMenu( const QPoint &pos );
        void openProjectConfig();
        void saveState();
        void restoreState(KDevelop::IProject* project = 0);
        void aboutToShutdown();

    protected:
        virtual void keyPressEvent(QKeyEvent *event);
        virtual void dropEvent(QDropEvent* event);

    private:
        KDevelop::ProjectBaseItem* itemAtPos(QPoint pos);

        KDevelop::IProject* m_ctxProject;
        QPointer<KDevelop::NavigationToolTip> m_tooltip;
        QPersistentModelIndex m_idx;
};

#endif // KDEVPROJECTMANAGER_H

