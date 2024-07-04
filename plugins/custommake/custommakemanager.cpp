/*
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>
    SPDX-FileCopyrightText: 2011 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "custommakemanager.h"
#include "custommakemodelitems.h"
#include <debug.h>
#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iplugincontroller.h>
#include <makebuilder/imakebuilder.h>
#include <project/projectmodel.h>
#include <project/helper.h>
#include <custom-definesandincludes/idefinesandincludesmanager.h>
#include <makefileresolver/makefileresolver.h>

#include <KPluginFactory>
#include <KLocalizedString>

#include <QFile>
#include <QReadLocker>
#include <QReadWriteLock>
#include <QRegExp>
#include <QWriteLocker>

#include <algorithm>

using namespace KDevelop;

class CustomMakeProvider : public IDefinesAndIncludesManager::BackgroundProvider
{
public:
    explicit CustomMakeProvider(CustomMakeManager* manager)
        : m_customMakeManager(manager)
        , m_resolver(new MakeFileResolver())
    {}

    // NOTE: Fixes build failures for GCC versions <4.8.
    // cf. https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53613
    ~CustomMakeProvider() Q_DECL_NOEXCEPT override;

    QHash< QString, QString > definesInBackground(const QString&) const override
    {
        return {};
    }

    Path::List resolvePathInBackground(const QString& path, const bool isFrameworks) const
    {
        {
            QReadLocker lock(&m_lock);

            bool inProject = std::any_of(m_customMakeManager->m_projectPaths.constBegin(), m_customMakeManager->m_projectPaths.constEnd(), [&path](const QString& projectPath)
            {
                return path.startsWith(projectPath);
            } );

            if (!inProject) {
                return {};
            }
        }

        if (isFrameworks) {
            return m_resolver->resolveIncludePath(path).frameworkDirectories;
        } else {
            return m_resolver->resolveIncludePath(path).paths;
        }
    }

    Path::List includesInBackground(const QString& path) const override
    {
        return resolvePathInBackground(path, false);
    }

    Path::List frameworkDirectoriesInBackground(const QString& path) const override
    {
        return resolvePathInBackground(path, true);
    }

    QString parserArgumentsInBackground(const QString& /*path*/) const override
    {
        return {};
    }

    IDefinesAndIncludesManager::Type type() const override
    {
        return IDefinesAndIncludesManager::ProjectSpecific;
    }

    CustomMakeManager* m_customMakeManager;
    QScopedPointer<MakeFileResolver> m_resolver;
    mutable QReadWriteLock m_lock;
};

// NOTE: Fixes build failures for GCC versions <4.8.
// See above.
CustomMakeProvider::~CustomMakeProvider() Q_DECL_NOEXCEPT
{}

K_PLUGIN_FACTORY_WITH_JSON(CustomMakeSupportFactory, "kdevcustommakemanager.json", registerPlugin<CustomMakeManager>(); )

CustomMakeManager::CustomMakeManager(QObject* parent, const KPluginMetaData& metaData, const QVariantList& args)
    : KDevelop::AbstractFileManagerPlugin(QStringLiteral("kdevcustommakemanager"), parent, metaData)
    , m_provider(new CustomMakeProvider(this))
{
    Q_UNUSED(args)

    setXMLFile( QStringLiteral("kdevcustommakemanager.rc") );

    // TODO use CustomMakeBuilder
    IPlugin* i = core()->pluginController()->pluginForExtension( QStringLiteral("org.kdevelop.IMakeBuilder") );
    Q_ASSERT(i);
    m_builder = i->extension<IMakeBuilder>();
    Q_ASSERT(m_builder);

    connect(this, &CustomMakeManager::reloadedFileItem,
            this, &CustomMakeManager::reloadMakefile);

    connect(ICore::self()->projectController(), &IProjectController::projectClosing,
            this, &CustomMakeManager::projectClosing);


    IDefinesAndIncludesManager::manager()->registerBackgroundProvider(m_provider.data());
}

CustomMakeManager::~CustomMakeManager()
{
}

IProjectBuilder* CustomMakeManager::builder() const
{
    Q_ASSERT(m_builder);
    return m_builder;
}

Path::List CustomMakeManager::includeDirectories(KDevelop::ProjectBaseItem*) const
{
    return Path::List();
}

Path::List CustomMakeManager::frameworkDirectories(KDevelop::ProjectBaseItem*) const
{
    return Path::List();
}

QHash<QString,QString> CustomMakeManager::defines(KDevelop::ProjectBaseItem*) const
{
    return QHash<QString,QString>();
}

QString CustomMakeManager::extraArguments(KDevelop::ProjectBaseItem*) const
{
    return {};
}

ProjectTargetItem* CustomMakeManager::createTarget(const QString& target, KDevelop::ProjectFolderItem *parent)
{
    Q_UNUSED(target)
    Q_UNUSED(parent)
    return nullptr;
}

bool CustomMakeManager::addFilesToTarget(const QList< ProjectFileItem* > &files, ProjectTargetItem* parent)
{
    Q_UNUSED( files )
    Q_UNUSED( parent )
    return false;
}

bool CustomMakeManager::removeTarget(KDevelop::ProjectTargetItem *target)
{
    Q_UNUSED( target )
    return false;
}

bool CustomMakeManager::removeFilesFromTargets(const QList< ProjectFileItem* > &targetFiles)
{
    Q_UNUSED( targetFiles )
    return false;
}

bool CustomMakeManager::hasBuildInfo(KDevelop::ProjectBaseItem* item) const
{
    Q_UNUSED(item);
    return false;
}

Path CustomMakeManager::buildDirectory(KDevelop::ProjectBaseItem* item) const
{
    auto *fi=dynamic_cast<ProjectFolderItem*>(item);
    for(; !fi && item; )
    {
        item=item->parent();
        fi=dynamic_cast<ProjectFolderItem*>(item);
    }
    if(!fi) {
        return item->project()->path();
    }
    return fi->path();
}

QList<ProjectTargetItem*> CustomMakeManager::targets(KDevelop::ProjectFolderItem*) const
{
    QList<ProjectTargetItem*> ret;
    return ret;
}

static bool isMakefile(const QString& fileName)
{
    return  ( fileName == QLatin1String("Makefile")
        || fileName == QLatin1String("makefile")
        || fileName == QLatin1String("GNUmakefile")
        || fileName == QLatin1String("BSDmakefile") );
}

void CustomMakeManager::createTargetItems(IProject* project, const Path& path, ProjectBaseItem* parent)
{
    Q_ASSERT(isMakefile(path.lastPathSegment()));
    const auto targets = parseCustomMakeFile(path);
    for (const QString& target : targets) {
        if (!isValid(Path(parent->path(), target), false, project)) {
            continue;
        }
        new CustomMakeTargetItem( project, target, parent );
    }
}

ProjectFileItem* CustomMakeManager::createFileItem(IProject* project, const Path& path, ProjectBaseItem* parent)
{
    auto* item = new ProjectFileItem(project, path, parent);
    if (isMakefile(path.lastPathSegment())){
        createTargetItems(project, path, parent);
    }
    return item;
}

void CustomMakeManager::reloadMakefile(ProjectFileItem* file)
{
    if( !isMakefile(file->path().lastPathSegment())){
        return;
    }
    ProjectBaseItem* parent = file->parent();
    // remove the items that are Makefile targets
    const auto items = parent->children();
    for (ProjectBaseItem* item : items) {
        if (item->target()){
            delete item;
        }
    }
    // Recreate the targets.
    createTargetItems(parent->project(), file->path(), parent);
}

ProjectFolderItem* CustomMakeManager::createFolderItem(IProject* project, const Path& path, ProjectBaseItem* parent)
{
    // TODO more faster algorithm. should determine whether this directory
    // contains makefile or not.
    return new KDevelop::ProjectBuildFolderItem( project, path, parent );
}

KDevelop::ProjectFolderItem* CustomMakeManager::import(KDevelop::IProject *project)
{
    if( project->path().isRemote() )
    {
        //FIXME turn this into a real warning
        qCWarning(CUSTOMMAKE) << project->path() << "not a local file. Custom make support doesn't handle remote projects";
        return nullptr;
    }

    {
        QWriteLocker lock(&m_provider->m_lock);
        m_projectPaths.insert(project->path().path());
    }

    return AbstractFileManagerPlugin::import( project );
}

/////////////////////////////////////////////////////////////////////////////
// private slots

///TODO: move to background thread, probably best would be to use a proper ParseJob
QStringList CustomMakeManager::parseCustomMakeFile( const Path &makefile )
{
    if( !makefile.isValid() )
        return QStringList();

    QStringList ret; // the list of targets
    QFile f( makefile.toLocalFile() );
    if ( !f.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        qCDebug(CUSTOMMAKE) << "could not open" << makefile;
        return ret;
    }

    QRegExp targetRe(QStringLiteral("^ *([^\\t$.#]\\S+) *:?:(?!=).*$"));
    targetRe.setMinimal( true );

    QString str;
    QTextStream stream( &f );
    while ( !stream.atEnd() )
    {
        str = stream.readLine();

        if ( targetRe.indexIn( str ) != -1 )
        {
            QString tmpTarget = targetRe.cap( 1 ).simplified();
            if ( ! ret.contains( tmpTarget ) )
                ret.append( tmpTarget );
        }
    }
    f.close();
    return ret;
}

void CustomMakeManager::projectClosing(IProject* project)
{
    QWriteLocker lock(&m_provider->m_lock);
    m_projectPaths.remove(project->path().path());
}

void CustomMakeManager::unload()
{
  IDefinesAndIncludesManager::manager()->unregisterBackgroundProvider(m_provider.data());
}

KDevelop::Path CustomMakeManager::compiler(KDevelop::ProjectTargetItem* item) const
{
    Q_UNUSED(item);
    return {};
}

#include "custommakemanager.moc"
#include "moc_custommakemanager.cpp"
