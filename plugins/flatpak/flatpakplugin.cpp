/*
    SPDX-FileCopyrightText: 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "flatpakplugin.h"
#include "flatpakruntime.h"
#include <interfaces/icore.h>
#include <interfaces/iruntimecontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <project/projectmodel.h>
#include <util/executecompositejob.h>

#include <QTextStream>
#include <QStandardPaths>
#include <QAction>
#include <QDir>
#include <QDirIterator>
#include <QProcess>
#include <QRegularExpression>
#include <QInputDialog>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QFileDialog>
#include <KPluginFactory>
#include <KActionCollection>
#include <KLocalizedString>
#include <KParts/MainWindow>
#include <KJob>
#include <KSharedConfig>
#include <KConfigGroup>
#include <debug_flatpak.h>

K_PLUGIN_FACTORY_WITH_JSON(KDevFlatpakFactory, "kdevflatpak.json", registerPlugin<FlatpakPlugin>();)

using namespace KDevelop;

FlatpakPlugin::FlatpakPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& /*args*/)
    : KDevelop::IPlugin(QStringLiteral("kdevflatpak"), parent, metaData)
{
    auto ac = actionCollection();

    auto action = new QAction(QIcon::fromTheme(QStringLiteral("run-build-clean")), i18nc("@action", "Rebuild Environment"), this);
    action->setWhatsThis(i18nc("@info:whatsthis", "Recompiles all dependencies for a fresh environment."));
    ac->setDefaultShortcut(action, Qt::CTRL | Qt::META | Qt::Key_X);
    connect(action, &QAction::triggered, this, &FlatpakPlugin::rebuildCurrent);
    ac->addAction(QStringLiteral("runtime_flatpak_rebuild"), action);

    auto exportAction = new QAction(QIcon::fromTheme(QStringLiteral("document-export")), i18nc("@action", "Export Flatpak Bundle..."), this);
    exportAction->setWhatsThis(i18nc("@info:whatsthis", "Exports the current build into a 'bundle.flatpak' file."));
    ac->setDefaultShortcut(exportAction, Qt::CTRL | Qt::META | Qt::Key_E);
    connect(exportAction, &QAction::triggered, this, &FlatpakPlugin::exportCurrent);
    ac->addAction(QStringLiteral("runtime_flatpak_export"), exportAction);

    auto remoteAction = new QAction(QIcon::fromTheme(QStringLiteral("folder-remote-symbolic")), i18nc("@action", "Send to Device..."), this);
    ac->setDefaultShortcut(remoteAction, Qt::CTRL | Qt::META | Qt::Key_D);
    connect(remoteAction, &QAction::triggered, this, &FlatpakPlugin::executeOnRemoteDevice);
    ac->addAction(QStringLiteral("runtime_flatpak_remote"), remoteAction);

    runtimeChanged(ICore::self()->runtimeController()->currentRuntime());

    setXMLFile( QStringLiteral("kdevflatpakplugin.rc") );
    connect(ICore::self()->runtimeController(), &IRuntimeController::currentRuntimeChanged, this, &FlatpakPlugin::runtimeChanged);
    connect(ICore::self()->projectController(), &IProjectController::projectOpened, this, [this](IProject* project) {
        const auto group = project->projectConfiguration()->group(QStringLiteral("Flatpak"));
        if (!group.hasKey("File")) {
            return;
        }
        const Path file(QUrl(group.readEntry("File")));
        if (file.isEmpty() || !file.isLocalFile()) {
            return;
        }

        auto buildDirParentPath = project->path();
        buildDirParentPath.addPath(QStringLiteral(".kdev4/flatpak"));
        QDirIterator d(buildDirParentPath.toLocalFile(), QDir::AllDirs | QDir::NoDotAndDotDot);
        while (d.hasNext()) {
            createRuntime(file, d.nextFileInfo().fileName());
        }
    });
}

FlatpakPlugin::~FlatpakPlugin() = default;

void FlatpakPlugin::runtimeChanged(KDevelop::IRuntime* newRuntime)
{
    const bool isFlatpak = qobject_cast<FlatpakRuntime*>(newRuntime);

    const auto& actions = actionCollection()->actions();
    for (auto action: actions) {
        action->setEnabled(isFlatpak);
    }
}

void FlatpakPlugin::rebuildCurrent()
{
    const auto runtime = qobject_cast<FlatpakRuntime*>(ICore::self()->runtimeController()->currentRuntime());
    Q_ASSERT(runtime);
    ICore::self()->runController()->registerJob(runtime->rebuild());
}

void FlatpakPlugin::exportCurrent()
{
    const auto runtime = qobject_cast<FlatpakRuntime*>(ICore::self()->runtimeController()->currentRuntime());
    Q_ASSERT(runtime);

    const QString path = QFileDialog::getSaveFileName(ICore::self()->uiController()->activeMainWindow(), i18nc("@title:window", "Export %1", runtime->name()), {}, i18n("Flatpak Bundle (*.flatpak)"));
    if (!path.isEmpty()) {
        const auto exportBundle = runtime->exportBundle(path);
        auto* const job = new ExecuteCompositeJob(runtime, exportBundle.jobs);
        job->setObjectName(i18nc("%1 - application ID", "Flatpak Export Bundle %1", exportBundle.applicationId));
        ICore::self()->runController()->registerJob(job);
    }
}

static Path flatpakBuildDirPath(const IProject* project, const QString& arch)
{
    // TODO: Should we offer building a runtime if it's outside a project?
    if (!project || !project->path().isLocalFile()) {
        qCWarning(FLATPAK) << "Cannot create the flatpak-builder directory for" << project << arch;
        return {};
    }

    auto ret = project->path();
    ret.addPath(QStringLiteral(".kdev4"));
    ret.addPath(QStringLiteral("flatpak"));
    ret.addPath(arch);
    if (!QDir().mkpath(ret.toLocalFile())) {
        qCWarning(FLATPAK) << "Cannot create path" << ret;
        return {};
    }
    return ret;
}

void FlatpakPlugin::createRuntime(const KDevelop::Path &file, const QString &arch)
{
    IProject* project = ICore::self()->projectController()->findProjectForUrl(file.toUrl());
    const KDevelop::Path path(flatpakBuildDirPath(project, arch));
    if (path.isEmpty()) {
        return;
    }

    auto process = FlatpakRuntime::createBuildDirectory(path, file, arch);
    connect(process, &KJob::finished, this, [this, path, file, arch](KJob* job) {
        if (job->error() != 0) {
            QDir(path.toLocalFile()).removeRecursively();
            return;
        }

        auto rt = new FlatpakRuntime(path, file, arch);
        m_runtimes += rt;
        connect(rt, &QObject::destroyed, this, [this, rt]() {
            m_runtimes.removeAll(rt);
        });
        ICore::self()->runtimeController()->addRuntimes(rt);
        auto p = ICore::self()->projectController()->findProjectForUrl(file.toUrl());
        if (p) {
            auto config = p->projectConfiguration()->group(QStringLiteral("Flatpak"));
            config.writeEntry("File", file.toUrl());
        }
    });
    process->start();
}

static QStringList availableArches(const QJsonObject& doc)
{
    QProcess supportedArchesProcess;
    QStringList ret;

    const QString sdkName = doc[QLatin1String("sdk")].toString();
    const QString runtimeVersion = doc[QLatin1String("runtime-version")].toString();
    const QString match = sdkName + QLatin1String("/(.+)/") + runtimeVersion;
    QObject::connect(&supportedArchesProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     &supportedArchesProcess, [&supportedArchesProcess, &match, &ret]() {
        QTextStream stream(&supportedArchesProcess);
        QRegularExpression rx(match);
        while (!stream.atEnd()) {
            const QString line = stream.readLine();
            auto m = rx.match(line);
            if (m.hasMatch()) {
                ret << m.captured(1);
            }
        }
    });

    supportedArchesProcess.start(QStringLiteral("flatpak"), {QStringLiteral("list"), QStringLiteral("--runtime"), QStringLiteral("-d") });
    supportedArchesProcess.waitForFinished();
    return ret;
}

KDevelop::ContextMenuExtension FlatpakPlugin::contextMenuExtension(KDevelop::Context* context, QWidget* parent)
{
    QList<QUrl> urls;

    if ( context->type() == KDevelop::Context::FileContext ) {
        auto* filectx = static_cast<KDevelop::FileContext*>(context);
        urls = filectx->urls();
    } else if ( context->type() == KDevelop::Context::ProjectItemContext ) {
        auto* projctx = static_cast<KDevelop::ProjectItemContext*>(context);
        const auto items = projctx->items();
        for (KDevelop::ProjectBaseItem* item : items) {
            if ( item->file() ) {
                urls << item->file()->path().toUrl();
            }
        }
    }

    const QRegularExpression nameRx(QStringLiteral(".*\\..*\\..*\\.json$"));
    for(auto it = urls.begin(); it != urls.end(); ) {
        if (it->isLocalFile() && (it->path().contains(nameRx) || it->fileName() == kdeFlatpakManifestFileName)) {
            ++it;
        } else {
            it = urls.erase(it);
        }
    }

    if ( !urls.isEmpty() ) {
        KDevelop::ContextMenuExtension ext;
        for (const QUrl& url : std::as_const(urls)) {
            const KDevelop::Path file(url);
            const auto doc = FlatpakRuntime::config(file);
            const auto arches = availableArches(doc);
            for (const QString& arch : arches) {
                if (const auto it = std::find_if(m_runtimes.cbegin(), m_runtimes.cend(),
                                                 [&arch, &file](const FlatpakRuntime* runtime) {
                                                     return runtime->arch() == arch && runtime->file() == file;
                                                 });
                    it != m_runtimes.cend()) {
                    auto action =
                        new QAction(i18nc("@action:inmenu", "Rebuild Flatpak Environment: %1", (*it)->name()), parent);
                    connect(action, &QAction::triggered, this, [it]() {
                        ICore::self()->runController()->registerJob((*it)->rebuild());
                    });
                    ext.addAction(KDevelop::ContextMenuExtension::RunGroup, action);
                } else {
                    auto action = new QAction(
                        i18nc("@action:inmenu", "Build Flatpak Environment %1 for %2", doc[u"id"].toString(), arch),
                        parent);
                    connect(action, &QAction::triggered, this, [this, file, arch]() {
                        createRuntime(file, arch);
                    });
                    ext.addAction(KDevelop::ContextMenuExtension::RunGroup, action);
                }
            }
        }

        return ext;
    }

    return KDevelop::IPlugin::contextMenuExtension(context, parent);
}

void FlatpakPlugin::executeOnRemoteDevice()
{
    const auto runtime = qobject_cast<FlatpakRuntime*>(ICore::self()->runtimeController()->currentRuntime());
    Q_ASSERT(runtime);

    KConfigGroup group(KSharedConfig::openConfig(), QStringLiteral("Flatpak"));
    const QString lastDeviceAddress = group.readEntry("DeviceAddress");
    const QString host = QInputDialog::getText(
        ICore::self()->uiController()->activeMainWindow(), i18nc("@title:window", "Choose Tag Name"),
        i18nc("@label:textbox", "Device hostname:"),
        QLineEdit::Normal, lastDeviceAddress
    );
    if (host.isEmpty())
        return;
    group.writeEntry("DeviceAddress", host);

    auto* file = new QTemporaryFile(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1Char('/') + runtime->name() + QLatin1String("XXXXXX.flatpak"));
    file->open();
    file->close();
    auto job = runtime->executeOnDevice(host, file->fileName());
    file->setParent(file);

    ICore::self()->runController()->registerJob(job);
}

#include "flatpakplugin.moc"
#include "moc_flatpakplugin.cpp"
