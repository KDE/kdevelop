/*
    SPDX-FileCopyrightText: 2006-2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "cmakebuilder.h"
#include "debug.h"

#include <cmakebuilderconfig.h>

#include <project/projectmodel.h>

#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <project/builderjob.h>
#include <makebuilder/imakebuilder.h>
#include <util/stringviewhelpers.h>

#include "cmakejob.h"
#include "prunejob.h"
#include "cmakebuilderpreferences.h"
#include "cmakeutils.h"
#include <cmakemodelitems.h>

#include <KPluginFactory>
#include <KLocalizedString>
#include <KJob>

#include <QUrl>

K_PLUGIN_FACTORY_WITH_JSON(CMakeBuilderFactory, "kdevcmakebuilder.json", registerPlugin<CMakeBuilder>(); )

class ErrorJob : public KJob
{
    Q_OBJECT
public:
    ErrorJob(QObject* parent, const QString& error)
        : KJob(parent)
        , m_error(error)
    {}

    void start() override {
        setError(!m_error.isEmpty());
        setErrorText(m_error);
        emitResult();
    }

private:
    QString m_error;
};

CMakeBuilder::CMakeBuilder(QObject* parent, const KPluginMetaData& metaData, const QVariantList&)
    : KDevelop::IPlugin(QStringLiteral("kdevcmakebuilder"), parent, metaData)
{
    addBuilder(QStringLiteral("Makefile"), QStringList{QStringLiteral("Unix Makefiles"),
                                                       QStringLiteral("NMake Makefiles"),
                                                       QStringLiteral("MinGW Makefiles")},
               core()->pluginController()->pluginForExtension(QStringLiteral("org.kdevelop.IMakeBuilder")));
    addBuilder(QStringLiteral("build.ninja"), QStringList(QStringLiteral("Ninja")), core()->pluginController()->pluginForExtension(QStringLiteral("org.kdevelop.IProjectBuilder"), QStringLiteral("KDevNinjaBuilder")));
}

CMakeBuilder::~CMakeBuilder()
{
}

void CMakeBuilder::addBuilder(const QString& neededfile, const QStringList& generators, KDevelop::IPlugin* i)
{
    if( i )
    {
        auto* b = i->extension<KDevelop::IProjectBuilder>();
        if( b )
        {
            m_builders[neededfile] = b;
            for (const QString& gen : generators) {
                m_buildersForGenerator[gen] = b;
            }
            // can't use new signal/slot syntax here, IProjectBuilder is not a QObject
            connect(i, SIGNAL(built(KDevelop::ProjectBaseItem*)), this, SIGNAL(built(KDevelop::ProjectBaseItem*)));
            connect(i, SIGNAL(failed(KDevelop::ProjectBaseItem*)), this, SIGNAL(failed(KDevelop::ProjectBaseItem*)));
            connect(i, SIGNAL(cleaned(KDevelop::ProjectBaseItem*)), this, SIGNAL(cleaned(KDevelop::ProjectBaseItem*)));
            connect(i, SIGNAL(installed(KDevelop::ProjectBaseItem*)), this, SIGNAL(installed(KDevelop::ProjectBaseItem*)));

            qCDebug(KDEV_CMAKEBUILDER) << "Added builder " << i->metaObject()->className() << "for" << neededfile;
        }
        else
            qCWarning(KDEV_CMAKEBUILDER) << "Couldn't add" << i->metaObject()->className();
    }
}

KJob* CMakeBuilder::build(KDevelop::ProjectBaseItem *dom)
{
    KDevelop::IProject* p = dom->project();
    IProjectBuilder* builder = builderForProject(p);
    if( builder )
    {
        bool valid;
        KJob* configure = checkConfigureJob(dom->project(), valid);

        KJob* build = nullptr;
        if(dom->file())
        {
            auto* makeBuilder = dynamic_cast<IMakeBuilder*>(builder);
            if (!makeBuilder) {
                return new ErrorJob(this, i18n("Could not find the make builder. Check your installation"));
            }

            KDevelop::ProjectFileItem* file = dom->file();
            const auto projectFileName = file->text();
            const auto projectFileBaseName = KDevelop::leftOfLastNeedleOrEntireView(projectFileName, QLatin1Char{'.'});

            const QString target = projectFileBaseName + QLatin1String(".o");
            build = makeBuilder->executeMakeTarget(dom->parent(), target);
            qCDebug(KDEV_CMAKEBUILDER) << "create build job for target" << build << dom << target;
        }
        qCDebug(KDEV_CMAKEBUILDER) << "Building with" << builder;
        if (!build)
        {
            build = builder->build(dom);
        }
        if( configure )
        {
            qCDebug(KDEV_CMAKEBUILDER) << "creating composite job";
            auto* builderJob = new KDevelop::BuilderJob;
            builderJob->addCustomJob( KDevelop::BuilderJob::Configure, configure, dom );
            builderJob->addCustomJob( KDevelop::BuilderJob::Build, build, dom );
            builderJob->updateJobName();
            build = builderJob;
        }
        return build;
    }
    return new ErrorJob(this, i18n("Could not find a builder for %1", p->name()));
}

KJob* CMakeBuilder::clean(KDevelop::ProjectBaseItem *dom)
{
    IProjectBuilder* builder = builderForProject(dom->project());
    if( builder )
    {
        bool valid;
        KJob* configure = checkConfigureJob(dom->project(), valid);

        KDevelop::ProjectBaseItem* item = dom;
        if(dom->file()) //It doesn't work to compile a file
            item=(KDevelop::ProjectBaseItem*) dom->parent();

        qCDebug(KDEV_CMAKEBUILDER) << "Cleaning with" << builder;
        KJob* clean = builder->clean(item);
        if( configure ) {
            auto* builderJob = new KDevelop::BuilderJob;
            builderJob->addCustomJob( KDevelop::BuilderJob::Configure, configure, item );
            builderJob->addCustomJob( KDevelop::BuilderJob::Clean, clean, item );
            builderJob->updateJobName();
            clean = builderJob;
        }
        return clean;
    }
    return new ErrorJob(this, i18n("Could not find a builder for %1", dom->project()->name()));
}

KJob* CMakeBuilder::install(KDevelop::ProjectBaseItem *dom, const QUrl &installPrefix)
{
    IProjectBuilder* builder = builderForProject(dom->project());
    if( builder )
    {
        bool valid;
        KJob* configure = checkConfigureJob(dom->project(), valid);

        KDevelop::ProjectBaseItem* item = dom;
        if(dom->file())
            item=(KDevelop::ProjectBaseItem*) dom->parent();

        qCDebug(KDEV_CMAKEBUILDER) << "Installing with" << builder;
        KJob* install = builder->install(item, installPrefix);
        if( configure ) {
            auto* builderJob = new KDevelop::BuilderJob;
            builderJob->addCustomJob( KDevelop::BuilderJob::Configure, configure, item );
            builderJob->addCustomJob( KDevelop::BuilderJob::Install, install, item );
            builderJob->updateJobName();
            install = builderJob;
        }
        return install;

    }
    return new ErrorJob(this, i18n("Could not find a builder for %1", dom->project()->name()));
}

KJob* CMakeBuilder::checkConfigureJob(KDevelop::IProject* project, bool& valid)
{
    valid = false;
    KJob* configure = nullptr;
    if( CMake::checkForNeedingConfigure(project) )
    {
        configure = this->configure(project);
    }
    else if( CMake::currentBuildDir(project).isEmpty() )
    {
        return new ErrorJob(this, i18n("No build directory configured, cannot install"));
    }
    valid = true;
    return configure;
}

KJob* CMakeBuilder::configure( KDevelop::IProject* project )
{
    if( CMake::currentBuildDir( project ).isEmpty() )
    {
        return new ErrorJob(this, i18n("No build directory configured, cannot configure"));
    }
    auto* job = new CMakeJob(this);
    job->setProject(project);
    connect(job, &KJob::result, this, [this, project] {
        emit configured(project);
    });
    return job;
}

KJob* CMakeBuilder::prune( KDevelop::IProject* project )
{
    return new PruneJob(project);
}

KDevelop::IProjectBuilder* CMakeBuilder::builderForProject(KDevelop::IProject* p) const
{
    QString builddir = CMake::currentBuildDir( p ).toLocalFile();
    QMap<QString, IProjectBuilder*>::const_iterator it = m_builders.constBegin(), itEnd = m_builders.constEnd();
    for(; it!=itEnd; ++it) {
        if (QFile::exists(builddir+QLatin1Char('/')+it.key()))
            return it.value();
    }
    //It means that it still has to be generated, so use the builder for
    //the generator we use
    return m_buildersForGenerator[CMake::defaultGenerator()];
}

QList< KDevelop::IProjectBuilder* > CMakeBuilder::additionalBuilderPlugins( KDevelop::IProject* project  ) const
{
    IProjectBuilder* b = builderForProject( project );
    QList< KDevelop::IProjectBuilder* > ret;
    if(b)
        ret << b;
    return ret;
}

int CMakeBuilder::configPages() const
{
    return 1;
}

KDevelop::ConfigPage* CMakeBuilder::configPage(int number, QWidget* parent)
{
    if (number == 0) {
        return new CMakeBuilderPreferences(this, parent);
    }
    return nullptr;
}


#include "cmakebuilder.moc"
#include "moc_cmakebuilder.cpp"
