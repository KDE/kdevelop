/* KDevelop CMake Support
 *
 * Copyright 2013 Aleix Pol <aleixpol@kde.org>
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

#include "cmakeimportjob.h"
#include "cmakeutils.h"
#include <cmakeparserutils.h>
#include "cmakecommitchangesjob.h"
#include "cmakemanager.h"
#include "cmakeprojectdata.h"
#include "cmakemodelitems.h"
#include <project/projectmodel.h>
#include <project/projectfiltermanager.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/parsingenvironment.h>
#include <interfaces/iproject.h>
#include <util/environmentgrouplist.h>
#include <KCompositeJob>
#include <QtConcurrentRun>
#include <QFutureWatcher>

using namespace KDevelop;

class WaitAllJobs : public KCompositeJob
{
Q_OBJECT
public:
    friend class CMakeManager;
    WaitAllJobs(QObject* parent)
        : KCompositeJob(parent)
        , m_started(false) {}
    
    virtual void slotResult(KJob* job) {
        KCompositeJob::slotResult(job);
        reconsider();
    }
    void addJob(KJob* job) {
        Q_ASSERT(thread() == QThread::currentThread());
//         Q_ASSERT(thread() == job->thread());
        addSubjob(job);
        job->start();
    }

public slots:
    virtual void start() {
        m_started = true;
        QMetaObject::invokeMethod(this, "reconsider", Qt::QueuedConnection);
    }

private slots:
    void reconsider()
    {
        if(subjobs().isEmpty() && m_started) {
            m_started = false; //don't emit the result twice!
            emitResult();
        }
    }

private:
    bool m_started;
};

CMakeImportJob::CMakeImportJob(ProjectFolderItem* dom, CMakeManager* parent)
    : KJob(parent)
    , m_project(dom->project())
    , m_dom(dom)
    , m_data(parent->projectData(dom->project()))
    , m_manager(parent)
    , m_futureWatcher(new QFutureWatcher<void>)
{
    connect(m_futureWatcher, SIGNAL(finished()), SLOT(importFinished()));
}

void CMakeImportJob::start()
{
    QFuture<void> future = QtConcurrent::run(this, &CMakeImportJob::initialize);
    m_futureWatcher->setFuture(future);
}

void CMakeImportJob::importFinished()
{
    Q_ASSERT(m_project->thread() == QThread::currentThread());

    WaitAllJobs* wjob = new WaitAllJobs(this);
    connect(wjob, SIGNAL(finished(KJob*)), SLOT(waitFinished(KJob*)));
    foreach(KJob* job, m_jobs) {
        wjob->addJob(job);
    }
    wjob->start();
}

void CMakeImportJob::initialize()
{
    ReferencedTopDUContext ctx;
    ProjectBaseItem* parent = m_dom->parent();
    while (parent && !ctx) {
        DUChainReadLocker lock;
        ctx = DUChain::self()->chainForDocument(IndexedString(Path(parent->path(), "CMakeLists.txt").pathOrUrl()));
        parent = parent->parent();
    }
    if (!ctx) {
        ctx = initializeProject(dynamic_cast<CMakeFolderItem*>(m_dom));
    }
    importDirectory(m_project, m_dom->path(), ctx);
}

KDevelop::ReferencedTopDUContext CMakeImportJob::initializeProject(CMakeFolderItem* rootFolder)
{
    Path base(CMake::projectRoot(m_project));
    
    QPair<VariableMap,QStringList> initials = CMakeParserUtils::initialVariables();
    
    m_data.clear();
    m_data.modulePath=initials.first["CMAKE_MODULE_PATH"];
    m_data.vm=initials.first;
    m_data.vm.insertGlobal("CMAKE_SOURCE_DIR", QStringList(base.toLocalFile()));
    m_data.vm.insertGlobal("CMAKE_BINARY_DIR", QStringList(CMake::currentBuildDir(m_project).toLocalFile(KUrl::RemoveTrailingSlash)));
    
    const Path cachefile(m_manager->buildDirectory(m_project->projectItem()), "CMakeCache.txt");
    m_data.cache = CMakeParserUtils::readCache(cachefile);

    KDevelop::ReferencedTopDUContext buildstrapContext;
    {
        const IndexedString idxpath(Path(base, "buildstrap").pathOrUrl());
        DUChainWriteLocker lock;
        
        buildstrapContext = DUChain::self()->chainForDocument(idxpath);
        
        if(buildstrapContext) {
            buildstrapContext->clearLocalDeclarations();
            buildstrapContext->clearImportedParentContexts();
            buildstrapContext->deleteChildContextsRecursively();
        }else{
            buildstrapContext=new TopDUContext(idxpath, RangeInRevision(0,0, 0,0),
                                               new ParsingEnvironmentFile(idxpath));
            DUChain::self()->addDocumentChain(buildstrapContext);
        }
        
        Q_ASSERT(buildstrapContext);
    }
    ReferencedTopDUContext ref=buildstrapContext;
    foreach(const QString& script, initials.second)
    {
        ref = includeScript(CMakeProjectVisitor::findFile(script, m_data.modulePath, QStringList()), base.toLocalFile(), ref);
    }
    
    //Initialize parent parts of the project that don't belong to the tree (because it's a partial import)
    if(base.isParentOf(m_project->path()))
    {
        QList<Path> toimport;
        toimport += base;
        QStringList includes;
        while(!toimport.isEmpty()) {
            const Path currentDir = toimport.takeFirst();
            const Path script(currentDir, "CMakeLists.txt");
            
            QString dir = currentDir.toLocalFile();
            ref = includeScript(script.toLocalFile(), dir, ref);
            Q_ASSERT(ref);
            includes << m_data.properties[DirectoryProperty][dir]["INCLUDE_DIRECTORIES"];
            CMakeParserUtils::addDefinitions(m_data.properties[DirectoryProperty][dir]["COMPILE_DEFINITIONS"], &m_data.definitions);
            CMakeParserUtils::addDefinitions(m_data.vm["CMAKE_CXX_FLAGS"], &m_data.definitions, true);
            rootFolder->setDefinitions(m_data.definitions);
            
            foreach(const Subdirectory& s, m_data.subdirectories) {
                const Path candidate(currentDir, s.name);

                if(candidate.isParentOf(m_project->path()))
                    toimport += candidate;
            }
        }
        rootFolder->setIncludeDirectories(includes);
        rootFolder->setBuildDir(base.relativePath(m_project->path()));
    }
    return ref;
}

void CMakeImportJob::waitFinished(KJob*)
{
    emitResult();
}

KDevelop::ReferencedTopDUContext CMakeImportJob::includeScript(const QString& file, const QString& dir, ReferencedTopDUContext parent)
{
    m_manager->addWatcher(m_project, file);
    QString profile = CMake::currentEnvironment(m_project);
    const KDevelop::EnvironmentGroupList env( KSharedConfig::openConfig() );
    return CMakeParserUtils::includeScript( file, parent, &m_data, dir, env.variables(profile));
}

CMakeCommitChangesJob* CMakeImportJob::importDirectory(IProject* project, const Path& path, const KDevelop::ReferencedTopDUContext& parentTop)
{
    Q_ASSERT(thread() == m_project->thread());
    Path cmakeListsPath(path, "CMakeLists.txt");
    CMakeCommitChangesJob* commitJob = new CMakeCommitChangesJob(path, m_manager, project);
    commitJob->moveToThread(thread());
    m_jobs += commitJob;
    if(QFile::exists(cmakeListsPath.toLocalFile()))
    {
        kDebug(9042) << "Adding cmake: " << cmakeListsPath << " to the model";

        m_data.vm.pushScope();
        ReferencedTopDUContext ctx = includeScript(cmakeListsPath.toLocalFile(),
                                                   path.toLocalFile(), parentTop);
        Path::List folderList = commitJob->addProjectData(m_data);
        foreach(const Path& folder, folderList) {
            if (!m_manager->filterManager()->isValid(folder, true, project)) {
                continue;
            }
            Path newcmakeListsPath(folder, "CMakeLists.txt");
            if(!QFile::exists(newcmakeListsPath.toLocalFile())) {
               kWarning() << "Unable to open " << newcmakeListsPath.toLocalFile();
               continue;
            }
            CMakeCommitChangesJob* job = importDirectory(project, folder, ctx);
            job->setFindParentItem(false);
            connect(commitJob, SIGNAL(folderCreated(KDevelop::ProjectFolderItem*)),
                    job, SLOT(folderAvailable(KDevelop::ProjectFolderItem*)));
        }
        m_data.vm.popScope();
    }
    
    return commitJob;
}

IProject* CMakeImportJob::project() const
{
    Q_ASSERT(!m_futureWatcher->isRunning());
    return m_project;
}

CMakeProjectData CMakeImportJob::projectData() const
{
    Q_ASSERT(!m_futureWatcher->isRunning());
    return m_data;
}

#include "moc_cmakeimportjob.cpp"
#include "cmakeimportjob.moc"
