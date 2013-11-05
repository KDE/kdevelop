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

#ifndef KDEVPLATFORM_PLUGIN_PROJECTMANAGERVIEW_H
#define KDEVPLATFORM_PLUGIN_PROJECTMANAGERVIEW_H

#include <QtGui/QWidget>
#include <KAction>
#include <interfaces/context.h>

class QModelIndex;

class KUrl;

namespace Ui
{
class ProjectManagerView;
}

class ProjectProxyModel;
class VcsOverlayProxyModel;

namespace KDevelop
{
class ProjectBaseItem;
}

class ProjectManagerView;
class ProjectManagerViewPlugin;

//own subclass to the current view can be passed from ProjectManagetView to ProjectManagerViewPlugin
class ProjectManagerViewItemContext : public KDevelop::ProjectItemContext
{
public:
    ProjectManagerViewItemContext(const QList< KDevelop::ProjectBaseItem* >& items, ProjectManagerView *view);
    ProjectManagerView *view() const;
private:
    ProjectManagerView *m_view;
};

class ProjectManagerView: public QWidget
{
    Q_OBJECT
public:
    ProjectManagerView( ProjectManagerViewPlugin*, QWidget *parent );
    virtual ~ProjectManagerView();

    ProjectManagerViewPlugin* plugin() const { return m_plugin; }
    QList<KDevelop::ProjectBaseItem*> selectedItems() const;
    void selectItems(const QList<KDevelop::ProjectBaseItem*> &items);
    void expandItem(KDevelop::ProjectBaseItem *item);

protected:
    virtual bool eventFilter(QObject* obj, QEvent* event);

private slots:
    void selectionChanged();
    void locateCurrentDocument();
    void updateSyncAction();
    void openUrl( const KUrl& );

private:
    QModelIndex indexFromView(const QModelIndex& index) const;
    QModelIndex indexToView(const QModelIndex& index) const;

    QAction* m_syncAction;
    Ui::ProjectManagerView* m_ui;
    QStringList m_cachedFileList;
    ProjectProxyModel* m_modelFilter;
    VcsOverlayProxyModel* m_overlayProxy;
    ProjectManagerViewPlugin* m_plugin;
};

#endif // KDEVPLATFORM_PLUGIN_PROJECTMANAGERVIEW_H

