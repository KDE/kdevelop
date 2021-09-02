/*
    SPDX-FileCopyrightText: 2005 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_PROJECTMANAGERVIEW_H
#define KDEVPLATFORM_PLUGIN_PROJECTMANAGERVIEW_H

#include <QWidget>

#include <interfaces/context.h>
#include <project/projectutils.h>

class QModelIndex;
class QAction;

namespace Ui
{
class ProjectManagerView;
}

class ProjectProxyModel;
class VcsOverlayProxyModel;

namespace KDevelop
{
class ProjectBaseItem;
class Path;
}

class ProjectManagerView;
class ProjectManagerViewPlugin;

//own subclass to the current view can be passed from ProjectManagetView to ProjectManagerViewPlugin
class ProjectManagerViewItemContext : public KDevelop::ProjectItemContextImpl
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
    ~ProjectManagerView() override;

    ProjectManagerViewPlugin* plugin() const { return m_plugin; }
    QList<KDevelop::ProjectBaseItem*> selectedItems() const;
    void selectItems(const QList<KDevelop::ProjectBaseItem*> &items);
    void expandItem(KDevelop::ProjectBaseItem *item);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private Q_SLOTS:
    void selectionChanged();
    void raiseAndLocateCurrentDocument();
    void open( const KDevelop::Path& );
    void toggleHideTargets(bool hidden);
    void toggleSyncCurrentDocument(bool sync);

private:
    void locateCurrentDocument();
    QModelIndex indexFromView(const QModelIndex& index) const;
    QModelIndex indexToView(const QModelIndex& index) const;

    QAction* m_toggleTargetsAction;
    Ui::ProjectManagerView* m_ui;
    QStringList m_cachedFileList;
    ProjectProxyModel* m_modelFilter;
    VcsOverlayProxyModel* m_overlayProxy;
    ProjectManagerViewPlugin* m_plugin;
};

#endif // KDEVPLATFORM_PLUGIN_PROJECTMANAGERVIEW_H

