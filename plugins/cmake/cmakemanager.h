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
#include <QVariant>

#include <project/interfaces/iprojectfilemanager.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/abstractfilemanagerplugin.h>
#include <language/interfaces/ilanguagesupport.h>
#include <interfaces/iplugin.h>

#include "cmakeprojectdata.h"
#include "icmakemanager.h"

class WaitAllJobs;
class CMakeCommitChangesJob;
struct CMakeProjectData;
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
    class IRuntime;
}

class CMakeFolderItem;
class CTestSuite;
class CTestFindJob;

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
    explicit CMakeManager( QObject* parent = nullptr, const QVariantList& args = QVariantList() );

    ~CMakeManager() override;

    Features features() const override { return Features(Folders | Targets | Files ); }
    KDevelop::IProjectBuilder* builder() const override;
    bool hasBuildInfo(KDevelop::ProjectBaseItem*) const override;
    KDevelop::Path buildDirectory(KDevelop::ProjectBaseItem*) const override;
    KDevelop::Path::List includeDirectories(KDevelop::ProjectBaseItem *) const override;
    KDevelop::Path::List frameworkDirectories(KDevelop::ProjectBaseItem *item) const override;
    QHash<QString, QString> defines(KDevelop::ProjectBaseItem *) const override;
    QString extraArguments(KDevelop::ProjectBaseItem *item) const override;

    KDevelop::ProjectTargetItem* createTarget( const QString&, KDevelop::ProjectFolderItem* ) override { return nullptr; }

    virtual QList<KDevelop::ProjectTargetItem*> targets() const;
    QList<KDevelop::ProjectTargetItem*> targets(KDevelop::ProjectFolderItem* folder) const override;
    bool addFilesToTarget( const QList<KDevelop::ProjectFileItem*> &files, KDevelop::ProjectTargetItem* target) override;

    bool removeTarget( KDevelop::ProjectTargetItem* ) override { return false; }
    bool removeFilesFromTargets( const QList<KDevelop::ProjectFileItem*> &files ) override;

    KDevelop::ProjectFolderItem* import( KDevelop::IProject *project ) override;
    KJob* createImportJob(KDevelop::ProjectFolderItem* item) override;
    bool reload(KDevelop::ProjectFolderItem*) override;


    KDevelop::ProjectFolderItem* createFolderItem(KDevelop::IProject* project, const KDevelop::Path& path, KDevelop::ProjectBaseItem* parent = nullptr) override;
    QPair<QString, QString> cacheValue(KDevelop::IProject* project, const QString& id) const override;
    
    //LanguageSupport
    QString name() const override;
    KDevelop::ParseJob *createParseJob(const KDevelop::IndexedString &url) override;
    KDevelop::ICodeHighlighting* codeHighlighting() const override;
    QPair<QWidget*, KTextEditor::Range> specialLanguageObjectNavigationWidget(const QUrl& url, const KTextEditor::Cursor& position) override;

    KDevelop::ProjectFilterManager* filterManager() const;

    static KDevelop::IndexedString languageName();

    int perProjectConfigPages() const override;
    KDevelop::ConfigPage* perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions& options, QWidget* parent) override;

    void integrateData(const CMakeProjectData &data, KDevelop::IProject* project,
                       const QSharedPointer<CMakeServer>& server = {});

    KDevelop::Path compiler(KDevelop::ProjectTargetItem * p) const override;

Q_SIGNALS:
    void folderRenamed(const KDevelop::Path& oldFolder, KDevelop::ProjectFolderItem* newFolder);
    void fileRenamed(const KDevelop::Path& oldFile, KDevelop::ProjectFileItem* newFile);

private Q_SLOTS:
    void projectClosing(KDevelop::IProject*);

private:
    void reloadProjects();
    CMakeFile fileInformation(KDevelop::ProjectBaseItem* item) const;
    CMakeTarget targetInformation(KDevelop::ProjectTargetItem* item) const;

    void folderAdded(KDevelop::ProjectFolderItem* folder);
    KTextEditor::Range termRangeAtPosition(const KTextEditor::Document* textDocument,
                                           const KTextEditor::Cursor& position) const;

    void showConfigureErrorMessage(const QString& projectName, const QString& errorMessage) const;

private:
    struct PerProjectData
    {
        CMakeProjectData data;
        QSharedPointer<CMakeServer> server;
        QVector<CTestSuite*> testSuites;
        QVector<CTestFindJob*> testSuiteJobs;
    };
    QHash<KDevelop::IProject*, PerProjectData> m_projects;
    KDevelop::ProjectFilterManager* m_filter;
    KDevelop::ICodeHighlighting* m_highlight;
};

#endif


