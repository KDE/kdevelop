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

#ifndef KDEVPLATFORM_PLUGIN_PROJECTTREEVIEW_H
#define KDEVPLATFORM_PLUGIN_PROJECTTREEVIEW_H

#include <QTreeView>
#include <QtCore/QPointer>

class QAbstractProxyModel;
class QAction;
class QItemSelectionModel;
class QMouseEvent;

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
class Path;
}

class ProjectTreeView: public QTreeView
{
        Q_OBJECT
    public:
        explicit ProjectTreeView( QWidget *parent = nullptr );
        ~ProjectTreeView() override;

        static QModelIndex mapFromSource(const QAbstractProxyModel* proxy, const QModelIndex& sourceIdx);

        bool event(QEvent* event) override;

    Q_SIGNALS:
        void activate( const KDevelop::Path &url );

    protected Q_SLOTS:
        void slotActivated( const QModelIndex &index );
        void popupContextMenu( const QPoint &pos );
        void openProjectConfig();
        void saveState(KDevelop::IProject* project = nullptr);
        void restoreState(KDevelop::IProject* project = nullptr);
        void aboutToShutdown();
        void projectClosed(KDevelop::IProject* project);
        void rowsInserted(const QModelIndex& parent, int start, int end) override;
        void rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end) override;

    protected:
        void keyPressEvent(QKeyEvent *event) override;
        void dropEvent(QDropEvent* event) override;
        void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const override;

    private:
        QModelIndex mapFromItem(const KDevelop::ProjectBaseItem* item);
        KDevelop::ProjectBaseItem* itemAtPos(QPoint pos);
        QList<KDevelop::ProjectBaseItem*> selectedProjects();
        KDevelop::IProject* getCurrentProject();

        QPointer<KDevelop::IProject> m_previousSelection;
        QPointer<KDevelop::NavigationToolTip> m_tooltip;
        QPersistentModelIndex m_idx;
};

#endif // KDEVPLATFORM_PLUGIN_PROJECTTREEVIEW_H

