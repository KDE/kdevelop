/*
    SPDX-FileCopyrightText: 2005 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_PROJECTTREEVIEW_H
#define KDEVPLATFORM_PLUGIN_PROJECTTREEVIEW_H

#include <QTreeView>
#include <QPointer>

class QAbstractProxyModel;

namespace KDevelop
{
class IProject;
class ProjectBaseItem;
class Path;
}

class ProjectTreeView: public QTreeView
{
        Q_OBJECT
    public:
        explicit ProjectTreeView( QWidget *parent = nullptr );
        ~ProjectTreeView() override;

        static QModelIndex mapFromSource(const QAbstractProxyModel* proxy, const QModelIndex& sourceIdx);

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
        KDevelop::ProjectBaseItem* itemAtPos(const QPoint& pos) const;
        QList<KDevelop::ProjectBaseItem*> selectedProjects();
        KDevelop::IProject* getCurrentProject();

        QPointer<KDevelop::IProject> m_previousSelection;
};

#endif // KDEVPLATFORM_PLUGIN_PROJECTTREEVIEW_H

