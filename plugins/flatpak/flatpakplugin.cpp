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
#include <interfaces/iruncontroller.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <project/projectmodel.h>
#include <util/executecompositejob.h>

#include <QTextStream>
#include <QStandardPaths>
#include <QAction>
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

K_PLUGIN_FACTORY_WITH_JSON(KDevFlatpakFactory, "kdevflatpak.json", registerPlugin<FlatpakPlugin>();)

using namespace KDevelop;

FlatpakPlugin::FlatpakPlugin(QObject *parent, const QVariantList & /*args*/)
    : KDevelop::IPlugin( QStringLiteral("kdevflatpak"), parent )
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
        ICore::self()->runController()->registerJob(new ExecuteCompositeJob(runtime, runtime->exportBundle(path)));
    }
}

void FlatpakPlugin::createRuntime(const KDevelop::Path &file, const QString &arch)
{
    auto* dir = new QTemporaryDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/kdevelop-flatpak-"));
    const KDevelop::Path path(dir->path());

    auto process = FlatpakRuntime::createBuildDirectory(path, file, arch);
    connect(process, &KJob::finished, this, [path, file, arch, dir] (KJob* job) {
        if (job->error() != 0) {
            delete dir;
            return;
        }

        auto rt = new FlatpakRuntime(path, file, arch);
        connect(rt, &QObject::destroyed, rt, [dir]() { delete dir; });
        ICore::self()->runtimeController()->addRuntimes(rt);
    });
    process->start();
}

static QStringList availableArches(const KDevelop::Path& url)
{
    QProcess supportedArchesProcess;
    QStringList ret;

    const auto doc = FlatpakRuntime::config(url);
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
        if (it->isLocalFile() && it->path().contains(nameRx)) {
            ++it;
        } else {
            it = urls.erase(it);
        }
    }

    if ( !urls.isEmpty() ) {
        KDevelop::ContextMenuExtension ext;
        for (const QUrl& url : std::as_const(urls)) {
            const KDevelop::Path file(url);
            const auto arches = availableArches(file);
            for (const QString& arch : arches) {
                auto action = new QAction(i18nc("@action:inmenu", "Build Flatpak %1 for %2", file.lastPathSegment(), arch), parent);
                connect(action, &QAction::triggered, this, [this, file, arch]() {
                    createRuntime(file, arch);
                });
                ext.addAction(KDevelop::ContextMenuExtension::RunGroup, action);
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
