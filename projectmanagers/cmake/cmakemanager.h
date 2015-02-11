/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
 * Copyright 2007-2009 Aleix Pol <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef CMAKEMANAGER_H
#define CMAKEMANAGER_H

#include <QList>
#include <QString>
#include <QtCore/QVariant>

#include <project/interfaces/iprojectfilemanager.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/abstractfilemanagerplugin.h>
#include <language/interfaces/ilanguagesupport.h>
#include <language/codegen/applychangeswidget.h>
#include <interfaces/iplugin.h>
#include <interfaces/idocumentationprovider.h>

#include "cmakelistsparser.h"
#include "icmakemanager.h"
#include "cmakeprojectvisitor.h"

class WaitAllJobs;
class CMakeCommitChangesJob;
struct CMakeProjectData;
class QStandardItem;
class QDir;
class QObject;
class CMakeHighlighting;
class CMakeDocumentation;

namespace KDevelop
{
    class IProject;
    class IProjectBuilder;
    class ICodeHighlighting;
    class ProjectFolderItem;
    class ProjectBaseItem;
    class ProjectFileItem;
    class ProjectTargetItem;
    class ProjectFilterManager;
    class IProjectFilter;
    class ParseJob;
    class ContextMenuExtension;
    class Context;
}

class CMakeFolderItem;

class CMakeManager
    : public KDevelop::AbstractFileManagerPlugin
    , public KDevelop::IBuildSystemManager
    , public KDevelop::ILanguageSupport
    , public ICMakeManager
{
Q_OBJECT
Q_INTERFACES( KDevelop::IBuildSystemManager )
Q_INTERFACES( KDevelop::IProjectFileManager )
Q_INTERFACES( KDevelop::ILanguageSupport )
Q_INTERFACES( ICMakeManager )
public:
    explicit CMakeManager( QObject* parent = 0, const QVariantList& args = QVariantList() );

    virtual ~CMakeManager();

    virtual bool hasError() const override;
    virtual QString errorDescription() const override;

    virtual Features features() const override { return Features(Folders | Targets | Files ); }
    virtual KDevelop::IProjectBuilder* builder() const override;
    virtual bool hasIncludesOrDefines(KDevelop::ProjectBaseItem*) const override;
    virtual KDevelop::Path buildDirectory(KDevelop::ProjectBaseItem*) const override;
    virtual KDevelop::Path::List includeDirectories(KDevelop::ProjectBaseItem *) const override;
    virtual QHash<QString, QString> defines(KDevelop::ProjectBaseItem *) const override;

    virtual KDevelop::ProjectTargetItem* createTarget( const QString&, KDevelop::ProjectFolderItem* ) override { return 0; }

    virtual QList<KDevelop::ProjectTargetItem*> targets() const;
    virtual QList<KDevelop::ProjectTargetItem*> targets(KDevelop::ProjectFolderItem* folder) const override;
//     virtual KDevelop::ProjectFolderItem* addFolder( const KDevelop::Path& folder, KDevelop::ProjectFolderItem* parent );
//     virtual KDevelop::ProjectFileItem* addFile( const KDevelop::Path&, KDevelop::ProjectFolderItem* );
    virtual bool addFilesToTarget( const QList<KDevelop::ProjectFileItem*> &files, KDevelop::ProjectTargetItem* target) override;

    virtual bool removeTarget( KDevelop::ProjectTargetItem* ) override { return false; }
    virtual bool removeFilesFromTargets( const QList<KDevelop::ProjectFileItem*> &files ) override;
//     virtual bool removeFilesAndFolders( const QList<KDevelop::ProjectBaseItem*> &items);
//
//     virtual bool renameFile(KDevelop::ProjectFileItem*, const KDevelop::Path&);
//     virtual bool renameFolder(KDevelop::ProjectFolderItem*, const KDevelop::Path&);
//     virtual bool moveFilesAndFolders(const QList< KDevelop::ProjectBaseItem* > &items, KDevelop::ProjectFolderItem *newParent);
//     virtual bool copyFilesAndFolders(const KDevelop::Path::List &items, KDevelop::ProjectFolderItem* newParent);
//
//     virtual QList<KDevelop::ProjectFolderItem*> parse( KDevelop::ProjectFolderItem* dom );
    virtual KDevelop::ProjectFolderItem* import( KDevelop::IProject *project ) override;

    virtual KJob* createImportJob(KDevelop::ProjectFolderItem* item) override;
//
    virtual bool reload(KDevelop::ProjectFolderItem*) override;
//
//     virtual KDevelop::ContextMenuExtension contextMenuExtension( KDevelop::Context* context );


    virtual KDevelop::ProjectFolderItem* createFolderItem(KDevelop::IProject* project, const KDevelop::Path& path, KDevelop::ProjectBaseItem* parent = 0) override;
    virtual QPair<QString, QString> cacheValue(KDevelop::IProject* project, const QString& id) const override;
    
    //LanguageSupport
    virtual QString name() const override;
    virtual KDevelop::ParseJob *createParseJob(const KDevelop::IndexedString &url) override;
    virtual KDevelop::ICodeHighlighting* codeHighlighting() const override;
    virtual QWidget* specialLanguageObjectNavigationWidget(const QUrl &url, const KTextEditor::Cursor& position) override;
    
//     void addPending(const KDevelop::Path& path, CMakeFolderItem* folder);
//     CMakeFolderItem* takePending(const KDevelop::Path& path);
//     void addWatcher(KDevelop::IProject* p, const QString& path);
    
//     CMakeProjectData projectData(KDevelop::IProject* project);

    KDevelop::ProjectFilterManager* filterManager() const;

    static KDevelop::IndexedString languageName();

    virtual int perProjectConfigPages() const override;
    virtual KDevelop::ConfigPage* perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions& options, QWidget* parent) override;

signals:
    void folderRenamed(const KDevelop::Path& oldFolder, KDevelop::ProjectFolderItem* newFolder);
    void fileRenamed(const KDevelop::Path& oldFile, KDevelop::ProjectFileItem* newFile);

private slots:
//     void dirtyFile(const QString& file);
//
//     void jumpToDeclaration();
    void projectClosing(KDevelop::IProject*);
    void dirtyFile(const QString& file);
//
//     void directoryChanged(const QString& dir);
//     void filesystemBuffererTimeout();
    void importFinished(KJob* job);

private:
    QHash<KDevelop::IProject*, CMakeProjectData> m_projects;
    KDevelop::ProjectFilterManager* m_filter;
    KDevelop::ICodeHighlighting* m_highlight;
};

#endif


