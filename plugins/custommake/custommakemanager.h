/*
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>
    SPDX-FileCopyrightText: 2011 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CUSTOMMAKEMANAGER_H
#define CUSTOMMAKEMANAGER_H

#include <project/abstractfilemanagerplugin.h>
#include <project/interfaces/ibuildsystemmanager.h>

#include <QScopedPointer>
#include <QSet>

class IMakeBuilder;
class CustomMakeProvider;

class CustomMakeManager : public KDevelop::AbstractFileManagerPlugin,
                          public KDevelop::IBuildSystemManager
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IBuildSystemManager )
public:
    explicit CustomMakeManager(QObject* parent, const KPluginMetaData& metaData,
                               const QVariantList& args = QVariantList());

    ~CustomMakeManager() override;

    Features features() const override { return Features(Folders | Targets | Files); }
    KDevelop::ProjectFolderItem* import(KDevelop::IProject* project) override;

    /**
     * Provide access to the builder
     */
    KDevelop::IProjectBuilder* builder() const override;

    /**
     * Provide a list of include directories.
     */
    KDevelop::Path::List includeDirectories(KDevelop::ProjectBaseItem*) const override;

    /**
     * Provide a list of framework directories.
     */
    KDevelop::Path::List frameworkDirectories(KDevelop::ProjectBaseItem*) const override;

    /**
     * Provide a list of files that contain the preprocessor defines for the
     * project
     */
    QHash<QString,QString> defines(KDevelop::ProjectBaseItem*) const override;

    QString extraArguments(KDevelop::ProjectBaseItem *item) const override;

    /**
     * Create a new target
     *
     * Creates the target specified by @p target to the folder @p parent and
     * modifies the underlying build system if needed
     */
    KDevelop::ProjectTargetItem* createTarget(const QString& target,
                                            KDevelop::ProjectFolderItem *parent) override;

    /**
     * Add a file to a target
     *
     * Adds the file specified by @p file to the target @p parent and modifies
     * the underlying build system if needed.
     */
    bool addFilesToTarget(const QList<KDevelop::ProjectFileItem*> &files, KDevelop::ProjectTargetItem *parent) override;

    /**
     * Remove a target
     *
     * Removes the target specified by @p target and
     * modifies the underlying build system if needed.
     */
    bool removeTarget(KDevelop::ProjectTargetItem *target) override;

    /**
     * Remove a file from a target
     *
     * Removes the file specified by @p file from the folder @p parent and
     * modifies the underlying build system if needed. The file is not removed
     * from the folder it is in
     */
    bool removeFilesFromTargets(const QList<KDevelop::ProjectFileItem*>&) override;

    /**
     * Test if @p item has any includes or defines from this BSM
     */
    bool hasBuildInfo(KDevelop::ProjectBaseItem* item) const override;

    /**
     * Get the toplevel build directory for the project
     */
    KDevelop::Path buildDirectory(KDevelop::ProjectBaseItem*) const override;

    /**
     * Get a list of all the targets in this project
     *
     * The list returned by this function should be checked to verify it is not
     * empty before using it
     *
     * @return The list of targets for this project
     * @todo implement
     */
    QList<KDevelop::ProjectTargetItem*> targets(KDevelop::ProjectFolderItem*) const override;

    KDevelop::Path compiler(KDevelop::ProjectTargetItem * p) const override;

protected:
    KDevelop::ProjectFileItem* createFileItem(KDevelop::IProject* project,
                                                      const KDevelop::Path& path,
                                                      KDevelop::ProjectBaseItem* parent) override;
    KDevelop::ProjectFolderItem* createFolderItem(KDevelop::IProject* project,
                                                          const KDevelop::Path& path,
                                                          KDevelop::ProjectBaseItem* parent = nullptr) override;

    void unload() override;

private Q_SLOTS:
    void reloadMakefile(KDevelop::ProjectFileItem *item);

    void projectClosing(KDevelop::IProject*);

private:
    /**
     * Initialize targets by reading Makefile in @arg dir
     * @return Target lists in Makefile at @arg dir.
     */
    QStringList parseCustomMakeFile( const KDevelop::Path &makefile );

    void createTargetItems(KDevelop::IProject* project, const KDevelop::Path& path, KDevelop::ProjectBaseItem* parent);

private:
    IMakeBuilder *m_builder = nullptr;
    QScopedPointer<CustomMakeProvider> m_provider;
    QSet<QString> m_projectPaths;
    friend class CustomMakeProvider;
};
#endif
