/* KDevelop CMake Support
 *
 * Copyright 2013 Aleix Pol Gonzalez <aleixpol@kde.org>
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

#include "cmakecommitchangesjob.h"
#include "cmakeprojectdata.h"
#include "testing/ctestutils.h"
#include "cmakemodelitems.h"
#include "cmakeutils.h"
#include "cmakemanager.h"
#include <cmakeparserutils.h>
#include <project/projectfiltermanager.h>
#include <project/interfaces/iprojectfilter.h>

#include <KUrl>

#include <QThread>

using namespace KDevelop;

static ProjectFileItem* containsFile(const Path& file, const QList<ProjectFileItem*>& tfiles)
{
    foreach(ProjectFileItem* f, tfiles) {
        if(f->path()==file)
            return f;
    }
    return 0;
}

static QStringList resolvePaths(const Path& base, const QStringList& pathsToResolve)
{
    QStringList resolvedPaths;
    resolvedPaths.reserve(pathsToResolve.size());
    foreach(const QString& pathToResolve, pathsToResolve)
    {
        QString dir(pathToResolve);
        if(!pathToResolve.startsWith("#[") && !pathToResolve.startsWith("$<"))
        {
            dir = Path(base, pathToResolve).toLocalFile();
        }
        resolvedPaths.append(dir);
    }
    return resolvedPaths;
}

static QSet<QString> filterFiles(const QFileInfoList& orig, const Path& base, IProject* project, ProjectFilterManager* filter)
{
    QSet<QString> ret;
    ret.reserve(orig.size());
    foreach(const QFileInfo& info, orig)
    {
        const QString str = info.fileName();
        const Path path(base, str);

        if (!filter->isValid(path, info.isDir(), project)) {
            continue;
        }

        ret.insert(str);
    }
    return ret;
}

static bool isCorrectFolder(const Path& path, IProject* p)
{
    const QString cache = Path(path, "CMakeCache.txt").toLocalFile();

    bool ret = !QFile::exists(cache);
    ret &= !CMake::allBuildDirs(p).contains(path.toLocalFile());

    return ret;
}

template <class T>
static bool textInList(const QVector<T>& list, KDevelop::ProjectBaseItem* item)
{
    foreach(const T& s, list) {
        if(item->text()==s.name)
            return true;
    }
    return false;
}

static QList<KDevelop::ProjectBaseItem*> cleanupBuildFolders(CMakeFolderItem* item, const QVector<Subdirectory>& subs)
{
    QList<ProjectBaseItem*> ret;
    QList<KDevelop::ProjectFolderItem*> folders = item->folderList();
    foreach(KDevelop::ProjectFolderItem* folder, folders) {
        CMakeFolderItem* cmfolder = dynamic_cast<CMakeFolderItem*>(folder);
        if(cmfolder && cmfolder->formerParent()==item && !textInList<Subdirectory>(subs, folder))
            ret += folder;
    }
    return ret;
}

/////////////////////////////////////////

CMakeCommitChangesJob::CMakeCommitChangesJob(const Path& path, CMakeManager* manager, KDevelop::IProject* project)
    : KJob()
    , m_path(path)
    , m_project(project)
    , m_manager(manager)
    , m_projectDataAdded(false)
    , m_parentItem(0)
    , m_waiting(false)
    , m_findParent(true)
{
    setObjectName(path.pathOrUrl());
}

void processDependencies(ProcessedTarget &target, const QString& dep, const CMakeProjectData& data)
{
//     kDebug() << "processing..." << target.target.name << dep;
    QMap<QString, QStringList> depData = data.properties.value(TargetProperty).value(dep);
    if(depData.isEmpty()) {
        kDebug() << "error: couldn't find dependency " << dep << data.properties.value(TargetProperty).keys();
        return;
    }

    target.includes += depData["INTERFACE_INCLUDE_DIRECTORIES"];
    target.defines += depData["INTERFACE_COMPILE_DEFINITIONS"];
    foreach(const QString& d, depData["INTERFACE_LINK_LIBRARIES"])
        processDependencies(target, d, data);
}

Path::List CMakeCommitChangesJob::addProjectData(const CMakeProjectData& data)
{
    m_projectDataAdded = true;
    Path::List ret;
    m_tests = data.testSuites;
    
    QSet<QString> alreadyAdded;
    foreach(const Subdirectory& subf, data.subdirectories) {
        if(subf.name.isEmpty() || alreadyAdded.contains(subf.name)) //empty case would not be necessary if we didn't process the wrong lines
            continue;
        alreadyAdded.insert(subf.name);
        m_subdirectories += subf;

        ret += Path(m_path, subf.name);
    }

    QString dir = m_path.toLocalFile();
    if(data.vm.value("CMAKE_INCLUDE_CURRENT_DIR")==QStringList("ON")) {
        m_directories += dir;
        m_directories += CMakeParserUtils::binaryPath(dir, m_project->path().toLocalFile(), CMake::currentBuildDir(m_project).toLocalFile(KUrl::RemoveTrailingSlash));
    }
    m_directories += resolvePaths(m_path, data.properties[DirectoryProperty][dir]["INCLUDE_DIRECTORIES"]);
    m_directories.removeAll(QString());

    m_definitions.unite(data.definitions);
    CMakeParserUtils::addDefinitions(data.properties[DirectoryProperty][dir]["COMPILE_DEFINITIONS"], &m_definitions);
    CMakeParserUtils::addDefinitions(data.vm["CMAKE_CXX_FLAGS"], &m_definitions, true);

    foreach(const Target& t, data.targets) {
        const QMap<QString, QStringList>& targetProps = data.properties[TargetProperty][t.name];
        if(targetProps["FOLDER"]==QStringList("CTestDashboardTargets"))
            continue; //filter some annoying targets

        if (!m_manager->filterManager()->isValid(Path(m_path, t.name), false, m_project)) {
            continue;
        }

        ProcessedTarget target;
        target.target = t;
        target.defines = targetProps["COMPILE_DEFINITIONS"];
        target.includes = targetProps["INCLUDE_DIRECTORIES"];
        target.outputName = targetProps.value("OUTPUT_NAME", QStringList(t.name)).join(QString());
        target.location = CMake::resolveSystemDirs(m_project, targetProps["LOCATION"]).first();
        
        foreach(const QString& dep, targetProps["PRIVATE_LINK_LIBRARIES"]) {
            processDependencies(target, dep, data);
        }
        processDependencies(target, t.name, data);
        m_targets += target;
    }
    return ret;
}

void CMakeCommitChangesJob::start()
{
    Q_ASSERT(m_project->thread() == QThread::currentThread());

    if(!m_parentItem && m_findParent) {
        if(m_path == m_project->path()) {
            m_parentItem = m_project->projectItem()->folder();
        } else {
            QList<ProjectFolderItem*> folders = m_project->foldersForPath(IndexedString(m_path.pathOrUrl()));
            if(!folders.isEmpty())
                m_parentItem = folders.first();
        }
    }

    if((!m_projectDataAdded && m_parentItem) || dynamic_cast<CMakeFolderItem*>(m_parentItem)) {
        QMetaObject::invokeMethod(this, "makeChanges", Qt::QueuedConnection);
        m_waiting = false;
    } else
        m_waiting = true;
}

void CMakeCommitChangesJob::makeChanges()
{
    Q_ASSERT(m_project->thread() == QThread::currentThread());
    ProjectFolderItem* f = m_parentItem;
    m_manager->addWatcher(m_project, m_path.toLocalFile());

    if(!m_projectDataAdded) {
        reloadFiles();
        return;
    }

    CMakeFolderItem* folder = dynamic_cast<CMakeFolderItem*>(f);
    Q_ASSERT(folder);
    qDeleteAll(cleanupBuildFolders(folder, m_subdirectories));
    foreach(const Subdirectory& subf, m_subdirectories)
    {
        const Path path(m_path, subf.name);
        
        if (!m_manager->filterManager()->isValid(path, true, m_project)) {
            continue;
        }
        if(QDir(path.toLocalFile()).exists())
        {
            CMakeFolderItem* parent=folder;
            if(!m_path.isDirectParentOf(path))
                parent=0;

            CMakeFolderItem* a = 0;
            ProjectFolderItem* ff = folder->folderNamed(subf.name);
            if(ff)
            {
                if(ff->type()!=ProjectBaseItem::BuildFolder)
                    delete ff;
                else
                    a = static_cast<CMakeFolderItem*>(ff);
            }
            if(!a)
                a = new CMakeFolderItem( folder->project(), path, subf.build_dir, parent );
            else
                a->setPath(path);
            emit folderCreated(a);

            if(!parent) {
                a->setFormerParent(folder);
                m_manager->addPending(path, a);
            }

            a->setDescriptor(subf.desc);
        }
    }

    folder->setIncludeDirectories(m_directories);
    folder->setDefinitions(m_definitions);

    QSet<ProjectTargetItem*> deletableTargets = folder->targetList().toSet();
    foreach ( const ProcessedTarget& pt, m_targets)
    {
        const Target& t = pt.target;
        
        KDevelop::ProjectTargetItem* targetItem = folder->targetNamed(t.type, t.name);
        if (targetItem)
            deletableTargets.remove(targetItem);
        else {
            switch(t.type)
            {
                case Target::Library:
                    targetItem = new CMakeLibraryTargetItem( m_project, t.name, folder, pt.outputName, pt.location);
                    break;
                case Target::Executable:
                    targetItem = new CMakeExecutableTargetItem( m_project, t.name, folder, pt.outputName, pt.location);
                    break;
                case Target::Custom:
                    targetItem = new CMakeCustomTargetItem( m_project, t.name, folder, pt.outputName );
                    break;
            }
        }
        DUChainAttatched* duchainAtt=dynamic_cast<DUChainAttatched*>(targetItem);
        if(duchainAtt) {
            duchainAtt->setDeclaration(t.declaration);
        }
        
        DescriptorAttatched* descAtt=dynamic_cast<DescriptorAttatched*>(targetItem);
        if(descAtt)
            descAtt->setDescriptor(t.desc);

        CompilationDataAttached* incAtt = dynamic_cast<CompilationDataAttached*>(targetItem);
        if(incAtt) {
            incAtt->setIncludeDirectories(resolvePaths(m_path, pt.includes));
            incAtt->addDefinitions(pt.defines);
        }
        
        Path::List tfiles;
        foreach( const QString & sFile, t.files)
        {
            if(sFile.startsWith("#[") || sFile.isEmpty() || sFile.endsWith('/'))
                continue;

            const Path sourceFile(m_path, sFile);

            if(!sourceFile.isValid() || !QFile::exists(sourceFile.toLocalFile())) {
                kDebug(9042) << "..........Skipping non-existing source file:" << sourceFile << sFile << m_path;
                continue;
            }

            tfiles += sourceFile;
            kDebug(9042) << "..........Adding:" << sourceFile << sFile << m_path;
        }
        
        setTargetFiles(targetItem, tfiles);
    }
    qDeleteAll(deletableTargets);

    CTestUtils::createTestSuites(m_tests, folder);
    reloadFiles();
}

void CMakeCommitChangesJob::setTargetFiles(ProjectTargetItem* target, const Path::List& files)
{
    QList<ProjectFileItem*> tfiles = target->fileList();
    foreach(ProjectFileItem* file, tfiles) {
        if(!files.contains(file->path()))
            delete file;
    }
    
    tfiles = target->fileList(); //We need to recreate the list without the removed items
    foreach(const Path& file, files) {
        ProjectFileItem* f = containsFile(file, tfiles);
        if(!f)
            new KDevelop::ProjectFileItem( target->project(), file, target );
    }
}

void CMakeCommitChangesJob::reloadFiles(ProjectFolderItem* item)
{
    QDir d(item->path().toLocalFile());
    if(!d.exists()) {
        kDebug() << "Trying to return a directory that doesn't exist:" << item->path();
        return;
    }

    const Path folderPath = item->path();

    const QFileInfoList entriesL = d.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
    QSet<QString> entries = filterFiles(entriesL, folderPath, item->project(), m_manager->filterManager());

    kDebug() << "Reloading Directory!" << folderPath;
    
    //We look for removed elements
    foreach(ProjectBaseItem* it, item->children())
    {
        if(it->type()==ProjectBaseItem::Target || it->type()==ProjectBaseItem::ExecutableTarget || it->type()==ProjectBaseItem::LibraryTarget)
            continue;
        
        QString current=it->text();
        const Path filePath(folderPath, current);
        
        if(!entries.contains(current))
            delete it;
        else if(it->path() != filePath)
            it->setPath(filePath);
    }
    
    //We look for new elements
    QList<ProjectBaseItem*> newItems;
    foreach( const QString& entry, entries )
    {
        if(item->hasFileOrFolder( entry ))
            continue;

        const Path filePath(folderPath, entry);

        if( QFileInfo( filePath.toLocalFile() ).isDir() )
        {
            ProjectFolderItem* pendingfolder = m_manager->takePending(filePath);
            
            if(pendingfolder) {
                newItems += pendingfolder;
            } else if(isCorrectFolder(filePath, item->project())) {
                ProjectFolderItem* it = new ProjectFolderItem( item->project(), filePath );
                reloadFiles(it);
                m_manager->addWatcher(item->project(), filePath.toLocalFile());
                newItems += it;
            }
        }
        else
        {
            newItems += new KDevelop::ProjectFileItem( item->project(), filePath );
        }
    }
    foreach(ProjectBaseItem* it, newItems)
        item->appendRow(it);
}

void CMakeCommitChangesJob::folderAvailable(ProjectFolderItem* item)
{
    if(item->path() == m_path) {
        m_parentItem = item;
        if(m_waiting) {
            start();
            Q_ASSERT(!m_waiting);
        }
    }
}
void CMakeCommitChangesJob::reloadFiles()
{
    Q_ASSERT(m_project->thread() == QThread::currentThread());
    Q_ASSERT(m_parentItem);
    reloadFiles(m_parentItem);
    emitResult();
}

void CMakeCommitChangesJob::setFindParentItem(bool find)
{
    m_findParent = find;
}
