/*
    SPDX-FileCopyrightText: 2006-2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qmakebuilder.h"

#include "qmakebuilderpreferences.h"
#include "qmakeconfig.h"
#include "qmakeutils.h"

#include <project/projectmodel.h>

#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <makebuilder/imakebuilder.h>

#include <KPluginFactory>

#include "qmakejob.h"
#include <debug.h>

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(QMakeBuilderFactory, "kdevqmakebuilder.json", registerPlugin<QMakeBuilder>();)

QMakeBuilder::QMakeBuilder(QObject* parent, const KPluginMetaData& metaData, const QVariantList&)
    : KDevelop::IPlugin(QStringLiteral("kdevqmakebuilder"), parent, metaData)
{
    m_makeBuilder = core()->pluginController()->pluginForExtension(QStringLiteral("org.kdevelop.IMakeBuilder"));
    if (m_makeBuilder) {
        auto* mbuilder = m_makeBuilder->extension<IMakeBuilder>();
        if (mbuilder) {
            connect(m_makeBuilder, SIGNAL(built(KDevelop::ProjectBaseItem*)), this,
                    SIGNAL(built(KDevelop::ProjectBaseItem*)));
            connect(m_makeBuilder, SIGNAL(cleaned(KDevelop::ProjectBaseItem*)), this,
                    SIGNAL(cleaned(KDevelop::ProjectBaseItem*)));
            connect(m_makeBuilder, SIGNAL(installed(KDevelop::ProjectBaseItem*)), this,
                    SIGNAL(installed(KDevelop::ProjectBaseItem*)));
            connect(m_makeBuilder, SIGNAL(failed(KDevelop::ProjectBaseItem*)), this,
                    SIGNAL(failed(KDevelop::ProjectBaseItem*)));
            connect(m_makeBuilder, SIGNAL(makeTargetBuilt(KDevelop::ProjectBaseItem*,QString)), this,
                    SIGNAL(pruned(KDevelop::ProjectBaseItem*)));
        }
    }
}

QMakeBuilder::~QMakeBuilder()
{
}

KJob* QMakeBuilder::prune(KDevelop::IProject* project)
{
    qCDebug(KDEV_QMAKEBUILDER) << "Distcleaning";
    if (m_makeBuilder) {
        auto* builder = m_makeBuilder->extension<IMakeBuilder>();
        if (builder) {
            qCDebug(KDEV_QMAKEBUILDER) << "Distcleaning with make";
            return builder->executeMakeTarget(project->projectItem(), QStringLiteral("distclean"));
        }
    }
    return nullptr;
}

KJob* QMakeBuilder::build(KDevelop::ProjectBaseItem* dom)
{
    qCDebug(KDEV_QMAKEBUILDER) << "Building";
    if (m_makeBuilder) {
        auto* builder = m_makeBuilder->extension<IMakeBuilder>();
        if (builder) {
            qCDebug(KDEV_QMAKEBUILDER) << "Building with make";
            return maybePrependConfigureJob(dom, builder->build(dom), BuilderJob::Build);
        }
    }
    return nullptr;
}

KJob* QMakeBuilder::configure(KDevelop::IProject* project)
{
    auto job = new QMakeJob(this);
    job->setProject(project);
    return job;
}

KJob* QMakeBuilder::clean(KDevelop::ProjectBaseItem* dom)
{
    qCDebug(KDEV_QMAKEBUILDER) << "Cleaning";
    if (m_makeBuilder) {
        auto* builder = m_makeBuilder->extension<IMakeBuilder>();
        if (builder) {
            qCDebug(KDEV_QMAKEBUILDER) << "Cleaning with make";
            return maybePrependConfigureJob(dom, builder->clean(dom), BuilderJob::Clean);
        }
    }
    return nullptr;
}

KJob* QMakeBuilder::install(KDevelop::ProjectBaseItem* dom, const QUrl& /* prefix */)
{
    qCDebug(KDEV_QMAKEBUILDER) << "Installing";
    if (m_makeBuilder) {
        auto* builder = m_makeBuilder->extension<IMakeBuilder>();
        if (builder) {
            qCDebug(KDEV_QMAKEBUILDER) << "Installing with make";
            return maybePrependConfigureJob(dom, builder->install(dom), BuilderJob::Install);
        }
    }
    return nullptr;
}

int QMakeBuilder::perProjectConfigPages() const
{
    return 1;
}

KDevelop::ConfigPage* QMakeBuilder::perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions& options,
                                                         QWidget* parent)
{
    switch (number) {
    case 0:
        return new QMakeBuilderPreferences(this, options, parent);
    default:
        return nullptr;
    }
}

QList<IProjectBuilder*> QMakeBuilder::additionalBuilderPlugins(KDevelop::IProject* project) const
{
    Q_UNUSED(project);

    if (auto* makeBuilder = m_makeBuilder->extension<IMakeBuilder>()) {
        return QList<KDevelop::IProjectBuilder*>() << makeBuilder;
    }

    return QList<IProjectBuilder*>();
}

KJob* QMakeBuilder::maybePrependConfigureJob(ProjectBaseItem* dom, KJob* job, BuilderJob::BuildType type)
{
    Q_ASSERT(dom);

    if (!job) {
        qCDebug(KDEV_QMAKEBUILDER) << "Null job passed";
        return nullptr;
    }

    const bool needsConfigure = QMakeUtils::checkForNeedingConfigure(dom->project());
    if (needsConfigure) {
        qCDebug(KDEV_QMAKEBUILDER) << "Project" << dom->project()->name() << "needs configure";

        auto builderJob = new BuilderJob;
        builderJob->addCustomJob(BuilderJob::Configure, configure(dom->project()), dom);
        builderJob->addCustomJob(type, job, dom);
        builderJob->updateJobName();
        return builderJob;
    }

    return job;
}

#include "qmakebuilder.moc"
#include "moc_qmakebuilder.cpp"
