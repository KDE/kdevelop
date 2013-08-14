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
#include <QThread>

using namespace KDevelop;

static ProjectFileItem* containsFile(const KUrl& file, const QList<ProjectFileItem*>& tfiles)
{
    foreach(ProjectFileItem* f, tfiles) {
        if(f->url()==file)
            return f;
    }
    return 0;
}

static QStringList resolvePaths(const KUrl& baseUrl, const QStringList& pathsToResolve)
{
    QStringList resolvedPaths;
    foreach(const QString& pathToResolve, pathsToResolve)
    {
        QString dir(pathToResolve);
        if(!pathToResolve.startsWith("#[") && !pathToResolve.startsWith("$<"))
        {
            if(KUrl( pathToResolve ).isRelative())
            {
                KUrl path(baseUrl);
                path.addPath(pathToResolve);
                dir=path.toLocalFile();
            }

            KUrl simp(dir); //We use this to simplify dir
            simp.cleanPath();
            dir=simp.toLocalFile();
        }
        resolvedPaths.append(dir);
    }
    return resolvedPaths;
}

static QSet<QString> filterFiles(const QStringList& orig)
{
    QSet<QString> ret;
    foreach(const QString& str, orig)
    {
        ///@todo This filter should be configurable, and filtering should be done on a manager-independent level
        if (str.endsWith(QLatin1Char('~')) || str.endsWith(QLatin1String(".bak")))
            continue;

        ret.insert(str);
    }
    return ret;
}

static bool isCorrectFolder(const KUrl& url, IProject* p)
{
    KUrl cache(url,"CMakeCache.txt"), missing(url, ".kdev_ignore");
    
    bool ret = !QFile::exists(cache.toLocalFile()) && !QFile::exists(missing.toLocalFile());
    ret &= !CMake::allBuildDirs(p).contains(url.toLocalFile(KUrl::RemoveTrailingSlash));
    
    return ret;
}

template <class T>
static bool textInList(const QList<T>& list, KDevelop::ProjectBaseItem* item)
{
    foreach(const T& s, list) {
        if(item->text()==s.name)
            return true;
    }
    return false;
}

static QList<KDevelop::ProjectBaseItem*> cleanupBuildFolders(CMakeFolderItem* item, const QList< Subdirectory >& subs)
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

CMakeCommitChangesJob::CMakeCommitChangesJob(const KUrl& url, CMakeManager* manager, KDevelop::IProject* parent)
    : KJob()
    , m_url(url)
    , m_project(parent)
    , m_manager(manager)
    , m_projectDataAdded(false)
    , m_parentItem(0)
    , m_waiting(false)
{
    setObjectName(url.prettyUrl());
    moveToThread(parent->thread());
    if(m_url == m_project->folder()) {
        m_parentItem = m_project->projectItem()->folder();
    } else {
        QList<ProjectFolderItem*> folders = parent->foldersForUrl(m_url);
        if(!folders.isEmpty())
            m_parentItem = folders.first();
    }
}

KUrl::List CMakeCommitChangesJob::addProjectData(const CMakeProjectData* data)
{
    m_projectDataAdded = true;
    KUrl::List ret;
    m_tests = data->testSuites;
    
    QSet<QString> alreadyAdded;
    foreach(const Subdirectory& subf, data->subdirectories) {
        if(subf.name.isEmpty() || alreadyAdded.contains(subf.name)) //empty case would not be necessary if we didn't process the wrong lines
            continue;
        alreadyAdded.insert(subf.name);
        m_subdirectories += subf;

        KUrl path(subf.name);
        if(path.isRelative())
        {
            path=m_url;
            path.addPath(subf.name);
        }
        path.adjustPath(KUrl::AddTrailingSlash);
        ret += path;
    }

    QString dir = m_url.toLocalFile(KUrl::RemoveTrailingSlash);
    if(data->vm.value("CMAKE_INCLUDE_CURRENT_DIR")==QStringList("ON")) {
        m_directories += dir;
        m_directories += data->vm.value("CMAKE_CURRENT_BINARY_DIR");
    }
    m_directories += resolvePaths(m_url, data->properties[DirectoryProperty][dir]["INCLUDE_DIRECTORIES"]);
    m_directories.removeAll(QString());

    m_definitions = data->properties[DirectoryProperty][dir]["COMPILE_DEFINITIONS"];

    foreach(const Target& t, data->targets) {
        const QMap<QString, QStringList>& targetProps = data->properties[TargetProperty][t.name];
        if(targetProps["FOLDER"]==QStringList("CTestDashboardTargets"))
            continue; //filter some annoying targets

        ProcessedTarget target;
        target.target = t;
        target.defines = targetProps["COMPILE_DEFINITIONS"];
        target.includes = targetProps["INCLUDE_DIRECTORIES"];
        target.outputName = targetProps.value("OUTPUT_NAME", QStringList(t.name)).join(QString());
        target.location = targetProps["LOCATION"].join(QString());
        
        foreach(const QString& dep, t.libraries) {
            const QMap<QString, QStringList>& depData = data->properties.value(TargetProperty).value(dep);
            if(!depData.isEmpty()) {
                target.includes += depData["INTERFACE_INCLUDE_DIRECTORIES"];
                target.defines += depData["INTERFACE_COMPILE_DEFINITIONS"];
            } else {
                kDebug() << "error: couldn't find dependency " << dep << data->properties.value(TargetProperty).keys();
            }
        }
        m_targets += target;
    }
    return ret;
}

void CMakeCommitChangesJob::start()
{
    if(!m_projectDataAdded || dynamic_cast<CMakeFolderItem*>(m_parentItem)) {
        QMetaObject::invokeMethod(this, "makeChanges", Qt::QueuedConnection);
    } else
        m_waiting = true;
}

void CMakeCommitChangesJob::makeChanges()
{
    Q_ASSERT(m_project->thread() == QThread::currentThread());
    ProjectFolderItem* f = m_parentItem;

    if(!m_projectDataAdded) {
        reloadFiles();
        return;
    }

    CMakeFolderItem* folder = dynamic_cast<CMakeFolderItem*>(f);
    Q_ASSERT(folder);
    qDeleteAll(cleanupBuildFolders(folder, m_subdirectories));
    foreach(const Subdirectory& subf, m_subdirectories)
    {
        KUrl path(subf.name);
        if(path.isRelative())
        {
            path=m_url;
            path.addPath(subf.name);
        }
        path.adjustPath(KUrl::AddTrailingSlash);
        
        if(QDir(path.toLocalFile()).exists())
        {
            CMakeFolderItem* parent=folder;
            if(path.upUrl()!=m_url)
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
                a->setUrl(path);
            emit folderCreated(a);

            if(!parent) {
                a->setFormerParent(folder);
                m_manager->addPending(path, a);
            }

            a->setDescriptor(subf.desc);
        }
    }

    folder->setIncludeDirectories(m_directories);
    folder->defineVariables(m_definitions);

    QSet<ProjectTargetItem*> deletableTargets = folder->targetList().toSet();
    foreach ( const ProcessedTarget& pt, m_targets)
    {
        const Target& t = pt.target;
        
        KUrl resolvedPath;
        if(!pt.location.isEmpty())
            resolvedPath=CMake::resolveSystemDirs(folder->project(), QStringList(pt.location)).first();
        
        KDevelop::ProjectTargetItem* targetItem = folder->targetNamed(t.type, t.name);
        if (targetItem)
            deletableTargets.remove(targetItem);
        else {
            switch(t.type)
            {
                case Target::Library:
                    targetItem = new CMakeLibraryTargetItem( m_project, t.name,
                                                            folder, t.declaration, pt.outputName, resolvedPath);
                    break;
                case Target::Executable:
                    targetItem = new CMakeExecutableTargetItem( m_project, t.name,
                                                                folder, t.declaration, pt.outputName, resolvedPath);
                    break;
                case Target::Custom:
                    targetItem = new CMakeCustomTargetItem( m_project, t.name,
                                                            folder, t.declaration, pt.outputName );
                    break;
            }
        }
        
        DescriptorAttatched* descAtt=dynamic_cast<DescriptorAttatched*>(targetItem);
        if(descAtt)
            descAtt->setDescriptor(t.desc);

        CompilationDataAttached* incAtt = dynamic_cast<CompilationDataAttached*>(targetItem);
        if(incAtt) {
            incAtt->setIncludeDirectories(resolvePaths(m_url, pt.includes));
            incAtt->defineVariables(pt.defines);
        }
        
        KUrl::List tfiles;
        foreach( const QString & sFile, t.files)
        {
            if(sFile.startsWith("#[") || sFile.isEmpty() || sFile.endsWith('/'))
                continue;

            KUrl sourceFile(sFile);

            // important: we want the behavior of KUrl::isRelative(), *not* KUrl::isRelativeUrl()
            if(sourceFile.isRelative()) {
                sourceFile = m_url;
                sourceFile.addPath( sFile );
                if(!QFile::exists(sourceFile.toLocalFile())) {
                    sourceFile.clear();
                }
            }
            sourceFile.cleanPath();

            if(!sourceFile.isEmpty())
                tfiles += sourceFile;
            kDebug(9042) << "..........Adding:" << sourceFile << sFile << m_url;
        }
        
        setTargetFiles(targetItem, tfiles);
    }
    qDeleteAll(deletableTargets);

    reloadFiles();
//         CTestUtils::createTestSuites(m_tests, m_url);
}

void CMakeCommitChangesJob::setTargetFiles(ProjectTargetItem* target, const KUrl::List& files)
{
    QList<ProjectFileItem*> tfiles = target->fileList();
    foreach(ProjectFileItem* file, tfiles) {
        if(!files.contains(file->url()))
            delete file;
    }
    
    tfiles = target->fileList(); //We need to recreate the list without the removed items
    foreach(const KUrl& file, files) {
        ProjectFileItem* f = containsFile(file, tfiles);
        if(!f)
            new KDevelop::ProjectFileItem( target->project(), file, target );
    }
}

void CMakeCommitChangesJob::reloadFiles(ProjectFolderItem* item)
{
    QDir d(item->url().toLocalFile());
    if(!d.exists()) {
        kDebug() << "Trying to return a directory that doesn't exist:" << item->url();
        return;
    }
    
    QStringList entriesL = d.entryList( QDir::AllEntries | QDir::NoDotAndDotDot);
    QSet<QString> entries = filterFiles(entriesL);
    
    KUrl folderurl = item->url();
    folderurl.cleanPath();

    kDebug() << "Reloading Directory!" << folderurl;
    
    //We look for removed elements
    foreach(ProjectBaseItem* it, item->children())
    {
        if(it->type()==ProjectBaseItem::Target || it->type()==ProjectBaseItem::ExecutableTarget || it->type()==ProjectBaseItem::LibraryTarget)
            continue;
        
        QString current=it->text();
        KUrl fileurl = folderurl;
        fileurl.addPath(current);
        
        if(!entries.contains(current))
            delete it;
        else if(!it->url().equals(fileurl, KUrl::CompareWithoutTrailingSlash))
            it->setUrl(fileurl);
    }
    
    //We look for new elements
    QList<ProjectBaseItem*> newItems;
    foreach( const QString& entry, entries )
    {
        KUrl fileurl = folderurl;
        fileurl.addPath( entry );

        if(item->hasFileOrFolder( entry ))
            continue;

        if( QFileInfo( fileurl.toLocalFile() ).isDir() )
        {
            fileurl.adjustPath(KUrl::AddTrailingSlash);
            ProjectFolderItem* pendingfolder = m_manager->takePending(fileurl);
            
            if(pendingfolder) {
                newItems += pendingfolder;
            } else if(isCorrectFolder(fileurl, item->project())) {
                fileurl.adjustPath(KUrl::AddTrailingSlash);
                ProjectFolderItem* it = new ProjectFolderItem( item->project(), fileurl, 0 );
                reloadFiles(it);
                m_manager->addWatcher(item->project(), fileurl.toLocalFile());
                newItems += it;
            }
        }
        else
        {
            newItems += new KDevelop::ProjectFileItem( item->project(), fileurl, 0 );
        }
    }
    foreach(ProjectBaseItem* it, newItems)
        item->appendRow(it);
}

void CMakeCommitChangesJob::folderAvailable(ProjectFolderItem* item)
{
    if(item->url() == m_url) {
        m_parentItem = item;
        if(m_waiting)
            start();
    }
}
void CMakeCommitChangesJob::reloadFiles()
{
    Q_ASSERT(m_project->thread() == QThread::currentThread());
    Q_ASSERT(m_parentItem);
    reloadFiles(m_parentItem);
    emitResult();
}
