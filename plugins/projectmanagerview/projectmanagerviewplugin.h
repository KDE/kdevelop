/*
    SPDX-FileCopyrightText: 2004 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_PROJECTMANAGERVIEWPLUGIN_H
#define KDEVPLATFORM_PLUGIN_PROJECTMANAGERVIEWPLUGIN_H

#include <interfaces/iplugin.h>
#include <QVariantList>
#include <project/builderjob.h>

namespace KDevelop
{
class ProjectBaseItem;
}

class ProjectManagerView;

class ProjectManagerViewPlugin: public KDevelop::IPlugin
{
    Q_OBJECT
public:

public:
    explicit ProjectManagerViewPlugin(QObject* parent, const KPluginMetaData& metaData,
                                      const QVariantList& = QVariantList());
    ~ProjectManagerViewPlugin() override;

    // Plugin methods
    void unload() override;

    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context, QWidget* parent) override;

    void removeItems(const QList<KDevelop::ProjectBaseItem*>& items);
    void renameItems(const QList< KDevelop::ProjectBaseItem* >& items);

public Q_SLOTS:
    void buildProjectItems();
    void installProjectItems();
    void cleanProjectItems();
    void copyFromContextMenu();
    void pasteFromContextMenu();

protected Q_SLOTS:
    void closeProjects();
    void buildItemsFromContextMenu();
    void installItemsFromContextMenu();
    void cleanItemsFromContextMenu();
    void configureProjectItems();
    void pruneProjectItems();
    void buildAllProjects();
    void addItemsFromContextMenuToBuildset();
    void projectConfiguration();
    void runTargetsFromContextMenu();
    void reloadFromContextMenu();
    void createFolderFromContextMenu();
    void createFileFromContextMenu();
    void removeFromContextMenu();
    void cutFromContextMenu();
    void removeTargetFilesFromContextMenu();
    void renameItemFromContextMenu();
    void updateActionState( KDevelop::Context* ctx );
    void updateFromBuildSetChange();

private:
    QList<KDevelop::ProjectBaseItem*> recurseAndFetchCheckedItems( KDevelop::ProjectBaseItem* item );
    QList<KDevelop::ProjectBaseItem*> collectItems();
    QList<KDevelop::ProjectBaseItem*> collectAllProjects();
    void runBuilderJob( KDevelop::BuilderJob::BuildType type, const QList<KDevelop::ProjectBaseItem*>& items );
    class ProjectManagerViewPluginPrivate* const d;
};

#endif

