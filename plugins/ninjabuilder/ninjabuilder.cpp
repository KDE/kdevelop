/*
    SPDX-FileCopyrightText: 2012 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2017 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "ninjabuilder.h"

#include "ninjajob.h"
#include "ninjabuilderpreferences.h"
#include <debug.h>

#include <KPluginFactory>
#include <KConfigGroup>
#include <KShell>
#include <project/projectmodel.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/builderjob.h>
#include <interfaces/iproject.h>

#include <QFile>

K_PLUGIN_FACTORY_WITH_JSON(NinjaBuilderFactory, "kdevninja.json", registerPlugin<NinjaBuilder>(); )

NinjaBuilder::NinjaBuilder(QObject* parent, const KPluginMetaData& metaData, const QVariantList&)
    : KDevelop::IPlugin(QStringLiteral("kdevninja"), parent, metaData)
{
    if (NinjaJob::ninjaExecutable().isEmpty()) {
        setErrorDescription(i18n("Unable to find ninja executable. Is it installed on the system?"));
    }
}

static QStringList targetsInFolder(KDevelop::ProjectFolderItem* item)
{
    QStringList ret;
    const auto targets = item->targetList();
    ret.reserve(targets.size());
    for (auto* target : targets) {
        ret += target->text();
    }

    return ret;
}

/**
 * Returns the first non-empty list of targets in folder @p item
 * or any of its ancestors if possible
 */
static QStringList closestTargetsForFolder(KDevelop::ProjectFolderItem* item)
{
    KDevelop::ProjectFolderItem* current = item;
    while (current) {
        const QStringList targets = targetsInFolder(current);
        if (!targets.isEmpty()) {
            return targets;
        }
        current = (current->parent() ? current->parent()->folder() : nullptr);
    }
    return QStringList();
}

static QStringList argumentsForItem(KDevelop::ProjectBaseItem* item)
{
    if (!item->parent() &&
        QFile::exists(item->project()->buildSystemManager()->buildDirectory(item->project()->projectItem()).toLocalFile())) {
        return QStringList();
    }

    switch (item->type()) {
    case KDevelop::ProjectBaseItem::File:
        return QStringList(item->path().toLocalFile() + QLatin1Char('^'));
    case KDevelop::ProjectBaseItem::Target:
    case KDevelop::ProjectBaseItem::ExecutableTarget:
    case KDevelop::ProjectBaseItem::LibraryTarget:
        return QStringList(item->target()->text());
    case KDevelop::ProjectBaseItem::Folder:
    case KDevelop::ProjectBaseItem::BuildFolder:
        return closestTargetsForFolder(item->folder());
    }
    return QStringList();
}

NinjaJob* NinjaBuilder::runNinja(KDevelop::ProjectBaseItem* item, NinjaJob::CommandType commandType,
                                           const QStringList& args, const QByteArray& signal)
{
    ///Running the same builder twice may result in serious problems,
    ///so kill jobs already running on the same project
    const auto ninjaJobs = m_activeNinjaJobs.data();
    for (NinjaJob* ninjaJob : ninjaJobs) {
        if (item && ninjaJob->item() && ninjaJob->item()->project() == item->project() && ninjaJob->commandType() == commandType) {
            qCDebug(NINJABUILDER) << "killing running ninja job, due to new started build on same project:" << ninjaJob;
            ninjaJob->kill();
        }
    }

    // Build arguments using data from KCM
    QStringList jobArguments;
    KSharedConfigPtr config = item->project()->projectConfiguration();
    KConfigGroup group = config->group(QStringLiteral("NinjaBuilder"));

    if (!group.readEntry("Abort on First Error", true)) {
        jobArguments << QStringLiteral("-k");
    }
    if (group.readEntry("Override Number Of Jobs", false)) {
        int jobCount = group.readEntry("Number Of Jobs", 1);
        if (jobCount > 0) {
            jobArguments << QStringLiteral("-j%1").arg(jobCount);
        }
    }
    int errorCount = group.readEntry("Number Of Errors", 1);
    if (errorCount > 1) {
        jobArguments << QStringLiteral("-k%1").arg(errorCount);
    }
    if (group.readEntry("Display Only", false)) {
        jobArguments << QStringLiteral("-n");
    }
    QString extraOptions = group.readEntry("Additional Options", QString());
    if (!extraOptions.isEmpty()) {
        const auto options = KShell::splitArgs(extraOptions);
        for (const QString& option : options) {
            jobArguments << option;
        }
    }
    jobArguments << args;

    auto* job = new NinjaJob(item, commandType, jobArguments, signal, this);
    job->setEnvironmentProfile(group.readEntry("Default Ninja Environment Profile", QString()));

    m_activeNinjaJobs.append(job);
    return job;
}

KJob* NinjaBuilder::build(KDevelop::ProjectBaseItem* item)
{
    return runNinja(item, NinjaJob::BuildCommand, argumentsForItem(item), "built");
}

KJob* NinjaBuilder::clean(KDevelop::ProjectBaseItem* item)
{
    return runNinja(item, NinjaJob::CleanCommand, QStringList(QStringLiteral("-t")) << QStringLiteral("clean"), "cleaned");
}

KJob* NinjaBuilder::install(KDevelop::ProjectBaseItem* item)
{
    NinjaJob* installJob = runNinja(item, NinjaJob::InstallCommand, QStringList(QStringLiteral("install")), "installed");
    installJob->setIsInstalling(true);

    KSharedConfigPtr configPtr = item->project()->projectConfiguration();
    KConfigGroup builderGroup(configPtr, QStringLiteral("NinjaBuilder"));
    bool installAsRoot = builderGroup.readEntry("Install As Root", false);
    if (installAsRoot) {
        auto* job = new KDevelop::BuilderJob;
        job->addCustomJob(KDevelop::BuilderJob::Build, build(item), item);
        job->addCustomJob(KDevelop::BuilderJob::Install, installJob, item);
        job->updateJobName();
        return job;
    } else {
        return installJob;
    }
}

int NinjaBuilder::perProjectConfigPages() const
{
    return 1;
}

KDevelop::ConfigPage* NinjaBuilder::perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions& options, QWidget* parent)
{
    if (number == 0) {
        return new NinjaBuilderPreferences(this, options, parent);
    }
    return nullptr;
}

class ErrorJob
    : public KJob
{
    Q_OBJECT
public:
    ErrorJob(QObject* parent, const QString& error)
        : KJob(parent)
        , m_error(error)
    {}

    void start() override
    {
        setError(!m_error.isEmpty());
        setErrorText(m_error);
        emitResult();
    }

private:
    QString m_error;
};

KJob* NinjaBuilder::install(KDevelop::ProjectBaseItem* dom, const QUrl& installPath)
{
    return installPath.isEmpty() ? install(dom) : new ErrorJob(nullptr, i18n("Cannot specify prefix in %1, on ninja", installPath.toDisplayString()));
}

#include "ninjabuilder.moc"
#include "moc_ninjabuilder.cpp"
