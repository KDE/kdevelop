/*
    SPDX-FileCopyrightText: 2006 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qmakemanager.h"

#include <QAction>
#include <QDir>
#include <QIcon>
#include <QFileInfo>
#include <QHash>
#include <QList>
#include <QUrl>

#include <KIO/Global>
#include <KConfigGroup>
#include <KDirWatch>
#include <KLocalizedString>
#include <KPluginFactory>

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/context.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iplugincontroller.h>
#include <project/projectmodel.h>
#include <serialization/indexedstring.h>

#include <qmakebuilder/iqmakebuilder.h>

#include "qmakemodelitems.h"
#include "qmakeprojectfile.h"
#include "qmakecache.h"
#include "qmakemkspecs.h"
#include "qmakejob.h"
#include "qmakebuilddirchooserdialog.h"
#include "qmakeconfig.h"
#include "qmakeutils.h"
#include <debug.h>

using namespace KDevelop;

// BEGIN Helpers

QMakeFolderItem* findQMakeFolderParent(ProjectBaseItem* item)
{
    QMakeFolderItem* p = nullptr;
    while (!p && item) {
        p = dynamic_cast<QMakeFolderItem*>(item);
        item = item->parent();
    }
    return p;
}

// END Helpers

K_PLUGIN_FACTORY_WITH_JSON(QMakeSupportFactory, "kdevqmakemanager.json", registerPlugin<QMakeProjectManager>();)

QMakeProjectManager::QMakeProjectManager(QObject* parent, const KPluginMetaData& metaData, const QVariantList&)
    : AbstractFileManagerPlugin(QStringLiteral("kdevqmakemanager"), parent, metaData)
    , IBuildSystemManager()
{
    IPlugin* i = core()->pluginController()->pluginForExtension(QStringLiteral("org.kdevelop.IQMakeBuilder"));
    Q_ASSERT(i);
    m_builder = i->extension<IQMakeBuilder>();
    Q_ASSERT(m_builder);

    connect(this, SIGNAL(folderAdded(KDevelop::ProjectFolderItem*)), this,
            SLOT(slotFolderAdded(KDevelop::ProjectFolderItem*)));

    m_runQMake = new QAction(QIcon::fromTheme(QStringLiteral("qtlogo")), i18nc("@action", "Run QMake"), this);
    connect(m_runQMake, &QAction::triggered, this, &QMakeProjectManager::slotRunQMake);
}

QMakeProjectManager::~QMakeProjectManager()
{
}

IProjectFileManager::Features QMakeProjectManager::features() const
{
    return Features(Folders | Targets | Files);
}

bool QMakeProjectManager::isValid(const Path& path, const bool isFolder, IProject* project) const
{
    if (!isFolder && path.lastPathSegment().startsWith(QLatin1String("Makefile"))) {
        return false;
    }
    return AbstractFileManagerPlugin::isValid(path, isFolder, project);
}

Path QMakeProjectManager::buildDirectory(ProjectBaseItem* item) const
{
    /// TODO: support includes by some other parent or sibling in a different file-tree-branch
    QMakeFolderItem* qmakeItem = findQMakeFolderParent(item);
    Path dir;
    if (qmakeItem) {
        if (!qmakeItem->parent()) {
            // build root item
            dir = QMakeConfig::buildDirFromSrc(qmakeItem->project(), qmakeItem->path());
        } else {
            // build sub-item
            const auto proFiles = qmakeItem->projectFiles();
            for (QMakeProjectFile* pro : proFiles) {
                if (QDir(pro->absoluteDir()) == QFileInfo(qmakeItem->path().toUrl().toLocalFile() + QLatin1Char('/')).absoluteDir()
                    || pro->hasSubProject(qmakeItem->path().toUrl().toLocalFile())) {
                    // get path from project root and it to buildDir
                    dir = QMakeConfig::buildDirFromSrc(qmakeItem->project(), Path(pro->absoluteDir()));
                    break;
                }
            }
        }
    }

    qCDebug(KDEV_QMAKE) << "build dir for" << item->text() << item->path() << "is:" << dir;
    return dir;
}

ProjectFolderItem* QMakeProjectManager::createFolderItem(IProject* project, const Path& path, ProjectBaseItem* parent)
{
    if (!parent) {
        return projectRootItem(project, path);
    } else if (ProjectFolderItem* buildFolder = buildFolderItem(project, path, parent)) {
        // child folder in a qmake folder
        return buildFolder;
    } else {
        return AbstractFileManagerPlugin::createFolderItem(project, path, parent);
    }
}

ProjectFolderItem* QMakeProjectManager::projectRootItem(IProject* project, const Path& path)
{
    QDir dir(path.toLocalFile());

    auto item = new QMakeFolderItem(project, path);

    const auto projectfiles = dir.entryList(QStringList() << QStringLiteral("*.pro"));
    if (projectfiles.isEmpty()) {
        return item;
    }

    QHash<QString, QString> qmvars = QMakeUtils::queryQMake(project);
    const QString mkSpecFile = QMakeConfig::findBasicMkSpec(qmvars);
    Q_ASSERT(!mkSpecFile.isEmpty());
    auto* mkspecs = new QMakeMkSpecs(mkSpecFile, qmvars);
    mkspecs->setProject(project);
    mkspecs->read();
    QMakeCache* cache = findQMakeCache(project);
    if (cache) {
        cache->setMkSpecs(mkspecs);
        cache->read();
    }

    for (const auto& projectfile : projectfiles) {
        Path proPath(path, projectfile);
        /// TODO: use Path in QMakeProjectFile
        auto* scope = new QMakeProjectFile(proPath.toLocalFile());
        scope->setProject(project);
        scope->setMkSpecs(mkspecs);
        scope->setOwnMkSpecs(true);
        if (cache) {
            scope->setQMakeCache(cache);
        }
        scope->read();
        qCDebug(KDEV_QMAKE) << "top-level scope with variables:" << scope->variables();
        item->addProjectFile(scope);
    }
    return item;
}

ProjectFolderItem* QMakeProjectManager::buildFolderItem(IProject* project, const Path& path, ProjectBaseItem* parent)
{
    // find .pro or .pri files in dir
    QDir dir(path.toLocalFile());
    const QStringList projectFiles = dir.entryList(QStringList{QStringLiteral("*.pro"), QStringLiteral("*.pri")},
                                             QDir::Files);
    if (projectFiles.isEmpty()) {
        return nullptr;
    }

    auto folderItem = new QMakeFolderItem(project, path, parent);

    // TODO: included by not-parent file (in a nother file-tree-branch).
    QMakeFolderItem* qmakeParent = findQMakeFolderParent(parent);
    if (!qmakeParent) {
        // happens for bad qmake configurations
        return nullptr;
    }

    for (const QString& file : projectFiles) {
        const QString absFile = dir.absoluteFilePath(file);

        // TODO: multiple includes by different .pro's
        QMakeProjectFile* parentPro = nullptr;
        const auto proFiles = qmakeParent->projectFiles();
        for (QMakeProjectFile* p : proFiles) {
            if (p->hasSubProject(absFile)) {
                parentPro = p;
                break;
            }
        }
        if (!parentPro && file.endsWith(QLatin1String(".pri"))) {
            continue;
        }
        qCDebug(KDEV_QMAKE) << "add project file:" << absFile;
        if (parentPro) {
            qCDebug(KDEV_QMAKE) << "parent:" << parentPro->absoluteFile();
        } else {
            qCDebug(KDEV_QMAKE) << "no parent, assume project root";
        }

        auto qmscope = new QMakeProjectFile(absFile);
        qmscope->setProject(project);

        const QFileInfo info(absFile);
        const QDir d = info.dir();
        /// TODO: cleanup
        if (parentPro) {
            // subdir
            if (QMakeCache* cache = findQMakeCache(project, Path(d.canonicalPath()))) {
                cache->setMkSpecs(parentPro->mkSpecs());
                cache->read();
                qmscope->setQMakeCache(cache);
            } else {
                qmscope->setQMakeCache(parentPro->qmakeCache());
            }

            qmscope->setMkSpecs(parentPro->mkSpecs());
        } else {
            // new project
            auto* root = dynamic_cast<QMakeFolderItem*>(project->projectItem());
            Q_ASSERT(root);
            qmscope->setMkSpecs(root->projectFiles().first()->mkSpecs());
            if (root->projectFiles().first()->qmakeCache()) {
                qmscope->setQMakeCache(root->projectFiles().first()->qmakeCache());
            }
        }

        if (qmscope->read()) {
            // TODO: only on read?
            folderItem->addProjectFile(qmscope);
        } else {
            delete qmscope;
            return nullptr;
        }
    }

    return folderItem;
}

void QMakeProjectManager::slotFolderAdded(ProjectFolderItem* folder)
{
    auto* qmakeParent = dynamic_cast<QMakeFolderItem*>(folder);
    if (!qmakeParent) {
        return;
    }

    qCDebug(KDEV_QMAKE) << "adding targets for" << folder->path();
    const auto proFiles = qmakeParent->projectFiles();
    for (QMakeProjectFile* pro : proFiles) {
        const auto targets = pro->targets();
        for (const auto& s : targets) {
            if (!isValid(Path(folder->path(), s), false, folder->project())) {
                continue;
            }
            qCDebug(KDEV_QMAKE) << "adding target:" << s;
            Q_ASSERT(!s.isEmpty());
            auto target = new QMakeTargetItem(pro, folder->project(), s, folder);
            const auto files = pro->filesForTarget(s);
            for (const auto& path : files) {
                new ProjectFileItem(folder->project(), Path(path), target);
                /// TODO: signal?
            }
        }
    }
}

ProjectFolderItem* QMakeProjectManager::import(IProject* project)
{
    const Path dirName = project->path();
    if (dirName.isRemote()) {
        // FIXME turn this into a real warning
        qCWarning(KDEV_QMAKE) << "not a local file. QMake support doesn't handle remote projects";
        return nullptr;
    }

    QMakeUtils::checkForNeedingConfigure(project);

    ProjectFolderItem* ret = AbstractFileManagerPlugin::import(project);
    connect(projectWatcher(project), &KDirWatch::dirty, this, &QMakeProjectManager::slotDirty);
    return ret;
}

void QMakeProjectManager::slotDirty(const QString& path)
{
    if (!path.endsWith(QLatin1String(".pro")) && !path.endsWith(QLatin1String(".pri"))) {
        return;
    }

    QFileInfo info(path);
    if (!info.isFile()) {
        return;
    }

    const QUrl url = QUrl::fromLocalFile(path);
    if (!isValid(Path(url), false, nullptr)) {
        return;
    }

    IProject* project = ICore::self()->projectController()->findProjectForUrl(url);
    if (!project) {
        // this can happen when we create/remove lots of files in a
        // sub dir of a project - ignore such cases for now
        return;
    }

    bool finished = false;
    const auto folders = project->foldersForPath(IndexedString(KIO::upUrl(url)));
    for (ProjectFolderItem* folder : folders) {
        if (auto* qmakeFolder = dynamic_cast<QMakeFolderItem*>(folder)) {
            const auto proFiles = qmakeFolder->projectFiles();
            for (QMakeProjectFile* pro : proFiles) {
                if (pro->absoluteFile() == path) {
                    // TODO: children
                    // TODO: cache added
                    qCDebug(KDEV_QMAKE) << "reloading" << pro << path;
                    pro->read();
                }
            }
            finished = true;
        } else if (ProjectFolderItem* newFolder = buildFolderItem(project, folder->path(), folder->parent())) {
            qCDebug(KDEV_QMAKE) << "changing from normal folder to qmake project folder:" << folder->path().toUrl();
            // .pro / .pri file did not exist before
            while (folder->rowCount()) {
                newFolder->appendRow(folder->takeRow(0));
            }
            folder->parent()->removeRow(folder->row());
            folder = newFolder;
            finished = true;
        }
        if (finished) {
            // remove existing targets and readd them
            for (int i = 0; i < folder->rowCount(); ++i) {
                if (folder->child(i)->target()) {
                    folder->removeRow(i);
                }
            }
            /// TODO: put into it's own function once we add more stuff to that slot
            slotFolderAdded(folder);
            break;
        }
    }
}

QList<ProjectTargetItem*> QMakeProjectManager::targets(ProjectFolderItem* item) const
{
    Q_UNUSED(item)
    return QList<ProjectTargetItem*>();
}

IProjectBuilder* QMakeProjectManager::builder() const
{
    Q_ASSERT(m_builder);
    return m_builder;
}

Path::List QMakeProjectManager::collectDirectories(ProjectBaseItem* item, const bool collectIncludes) const
{
    Path::List list;
    QMakeFolderItem* folder = findQMakeFolderParent(item);
    if (folder) {
        const auto proFiles = folder->projectFiles();
        for (QMakeProjectFile* pro : proFiles) {
            if (pro->files().contains(item->path().toLocalFile())) {
                const QStringList directories = collectIncludes ? pro->includeDirectories() : pro->frameworkDirectories();
                for (const QString& dir : directories) {
                    Path path(dir);
                    if (!list.contains(path)) {
                        list << path;
                    }
                }
            }
        }
        if (list.isEmpty()) {
            // fallback for new files, use all possible include dirs
            const auto proFiles = folder->projectFiles();
            for (QMakeProjectFile* pro : proFiles) {
                const QStringList directories = collectIncludes ? pro->includeDirectories() : pro->frameworkDirectories();
                for (const QString& dir : directories) {
                    Path path(dir);
                    if (!list.contains(path)) {
                        list << path;
                    }
                }
            }
        }
        // make sure the base dir is included
        if (!list.contains(folder->path())) {
            list << folder->path();
        }
        // qCDebug(KDEV_QMAKE) << "include dirs for" << item->path() << ":" << list;
    }
    return list;
}

Path::List QMakeProjectManager::includeDirectories(ProjectBaseItem* item) const
{
    return collectDirectories(item);
}

Path::List QMakeProjectManager::frameworkDirectories(ProjectBaseItem* item) const
{
    return collectDirectories(item, false);
}

QHash<QString, QString> QMakeProjectManager::defines(ProjectBaseItem* item) const
{
    QHash<QString, QString> d;
    QMakeFolderItem* folder = findQMakeFolderParent(item);
    if (!folder) {
        // happens for bad qmake configurations
        return d;
    }
    const auto proFiles = folder->projectFiles();
    for (QMakeProjectFile* pro : proFiles) {
        const auto defines = pro->defines();
        for (const auto& def : defines) {
            d.insert(def.first, def.second);
        }
    }
    return d;
}

QString QMakeProjectManager::extraArguments(KDevelop::ProjectBaseItem *item) const
{
    QMakeFolderItem* folder = findQMakeFolderParent(item);
    if (!folder) {
        // happens for bad qmake configurations
        return {};
    }

    QStringList d;
    const auto proFiles = folder->projectFiles();
    for (QMakeProjectFile* pro : proFiles) {
        d << pro->extraArguments();
    }
    return d.join(QLatin1Char(' '));
}

bool QMakeProjectManager::hasBuildInfo(KDevelop::ProjectBaseItem* item) const
{
    return findQMakeFolderParent(item);
}

QMakeCache* QMakeProjectManager::findQMakeCache(IProject* project, const Path& path) const
{
    QDir curdir(QMakeConfig::buildDirFromSrc(project, !path.isValid() ? project->path() : path).toLocalFile());
    curdir.makeAbsolute();
    while (!curdir.exists(QStringLiteral(".qmake.cache")) && !curdir.isRoot() && curdir.cdUp()) {
        qCDebug(KDEV_QMAKE) << curdir;
    }

    if (curdir.exists(QStringLiteral(".qmake.cache"))) {
        qCDebug(KDEV_QMAKE) << "Found QMake cache in " << curdir.absolutePath();
        return new QMakeCache(curdir.canonicalPath() + QLatin1String("/.qmake.cache"));
    }
    return nullptr;
}

ContextMenuExtension QMakeProjectManager::contextMenuExtension(Context* context, QWidget* parent)
{
    Q_UNUSED(parent);

    ContextMenuExtension ext;

    if (context->hasType(Context::ProjectItemContext)) {
        auto* pic = dynamic_cast<ProjectItemContext*>(context);
        Q_ASSERT(pic);
        if (pic->items().isEmpty()) {
            return ext;
        }

        m_actionItem = dynamic_cast<QMakeFolderItem*>(pic->items().first());
        if (m_actionItem) {
            ext.addAction(ContextMenuExtension::ProjectGroup, m_runQMake);
        }
    }

    return ext;
}

void QMakeProjectManager::slotRunQMake()
{
    Q_ASSERT(m_actionItem);

    Path srcDir = m_actionItem->path();
    Path buildDir = QMakeConfig::buildDirFromSrc(m_actionItem->project(), srcDir);
    auto* job = new QMakeJob(srcDir.toLocalFile(), buildDir.toLocalFile(), this);

    job->setQMakePath(QMakeConfig::qmakeExecutable(m_actionItem->project()));

    KConfigGroup cg(m_actionItem->project()->projectConfiguration(), QMakeConfig::CONFIG_GROUP());
    QString installPrefix = cg.readEntry(QMakeConfig::INSTALL_PREFIX, QString());
    if (!installPrefix.isEmpty())
        job->setInstallPrefix(installPrefix);
    job->setBuildType(cg.readEntry<int>(QMakeConfig::BUILD_TYPE, 0));
    job->setExtraArguments(cg.readEntry(QMakeConfig::EXTRA_ARGUMENTS, QString()));

    KDevelop::ICore::self()->runController()->registerJob(job);
}

KDevelop::Path QMakeProjectManager::compiler(KDevelop::ProjectTargetItem* p) const
{
    Q_UNUSED(p);
    return {};
}

#include "qmakemanager.moc"
#include "moc_qmakemanager.cpp"
