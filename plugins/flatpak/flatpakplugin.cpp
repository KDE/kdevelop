/*
   Copyright 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
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

    auto action = new QAction(QIcon::fromTheme(QStringLiteral("run-build-clean")), i18n("Rebuild environment"), this);
    action->setWhatsThis(i18n("Recompiles all dependencies for a fresh environment."));
    ac->setDefaultShortcut(action, Qt::CTRL | Qt::META | Qt::Key_X);
    connect(action, &QAction::triggered, this, &FlatpakPlugin::rebuildCurrent);
    ac->addAction(QStringLiteral("runtime_flatpak_rebuild"), action);

    auto exportAction = new QAction(QIcon::fromTheme(QStringLiteral("document-export")), i18n("Export flatpak bundle..."), this);
    exportAction->setWhatsThis(i18n("Exports the current build into a 'bundle.flatpak' file."));
    ac->setDefaultShortcut(exportAction, Qt::CTRL | Qt::META | Qt::Key_E);
    connect(exportAction, &QAction::triggered, this, &FlatpakPlugin::exportCurrent);
    ac->addAction(QStringLiteral("runtime_flatpak_export"), exportAction);

    auto remoteAction = new QAction(QIcon::fromTheme(QStringLiteral("folder-remote-symbolic")), i18n("Send to device..."), this);
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

    for(auto action: actionCollection()->actions()) {
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

    const QString path = QFileDialog::getSaveFileName(ICore::self()->uiController()->activeMainWindow(), i18n("Export %1 to...", runtime->name()), {}, i18n("Flatpak Bundle (*.flatpak)"));
    if (!path.isEmpty()) {
        ICore::self()->runController()->registerJob(new ExecuteCompositeJob(runtime, runtime->exportBundle(path)));
    }
}

void FlatpakPlugin::createRuntime(const KDevelop::Path &file, const QString &arch)
{
    QTemporaryDir* dir = new QTemporaryDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QStringLiteral("/kdevelop-flatpak-"));
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

    QObject::connect(&supportedArchesProcess, static_cast<void (QProcess::*)(int)>(&QProcess::finished), &supportedArchesProcess, [&supportedArchesProcess, &ret]() {
        supportedArchesProcess.deleteLater();

        QTextStream stream(&supportedArchesProcess);
        while (!stream.atEnd()) {
            const QString line = stream.readLine();
            ret << line.section(QLatin1Char('/'), 2, 2);
        }
    });

    const auto doc = FlatpakRuntime::config(url);
    const QString sdkName = doc[QLatin1String("sdk")].toString();
    const QString runtimeVersion = doc[QLatin1String("runtime-version")].toString();
    supportedArchesProcess.start("flatpak", {"info", "-r", sdkName + "//" + runtimeVersion });
    supportedArchesProcess.waitForFinished();
    return ret;
}

KDevelop::ContextMenuExtension FlatpakPlugin::contextMenuExtension(KDevelop::Context* context, QWidget* parent)
{
    QList<QUrl> urls;

    if ( context->type() == KDevelop::Context::FileContext ) {
        KDevelop::FileContext* filectx = static_cast<KDevelop::FileContext*>(context);
        urls = filectx->urls();
    } else if ( context->type() == KDevelop::Context::ProjectItemContext ) {
        KDevelop::ProjectItemContext* projctx = static_cast<KDevelop::ProjectItemContext*>(context);
        foreach( KDevelop::ProjectBaseItem* item, projctx->items() ) {
            if ( item->file() ) {
                urls << item->file()->path().toUrl();
            }
        }
    }

    const QRegularExpression nameRx(".*\\..*\\..*\\.json$");
    for(auto it = urls.begin(); it != urls.end(); ) {
        if (it->isLocalFile() && it->path().contains(nameRx)) {
            ++it;
        } else {
            it = urls.erase(it);
        }
    }

    if ( !urls.isEmpty() ) {
        KDevelop::ContextMenuExtension ext;
        foreach(const QUrl &url, urls) {
            const KDevelop::Path file(url);

            foreach(const QString &arch, availableArches(file)) {
                auto action = new QAction(i18n("Build flatpak %1 for %2", file.lastPathSegment(), arch), parent);
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

    KConfigGroup group(KSharedConfig::openConfig(), "Flatpak");
    const QString lastDeviceAddress = group.readEntry("DeviceAddress");
    const QString host = QInputDialog::getText(
        ICore::self()->uiController()->activeMainWindow(), i18n("Choose tag name..."),
        i18n("Device hostname"),
        QLineEdit::Normal, lastDeviceAddress
    );
    if (host.isEmpty())
        return;
    group.writeEntry("DeviceAddress", host);

    QTemporaryFile* file = new QTemporaryFile(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1Char('/') + runtime->name() + "XXXXXX.flatpak");
    file->open();
    file->close();
    auto job = runtime->executeOnDevice(host, file->fileName());
    file->setParent(file);

    ICore::self()->runController()->registerJob(job);
}

#include "flatpakplugin.moc"
