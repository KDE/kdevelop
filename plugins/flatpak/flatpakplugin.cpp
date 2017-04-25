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
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <project/projectmodel.h>
#include <QDebug>
#include <QStandardPaths>
#include <QAction>
#include <QProcess>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QFileDialog>
#include <KPluginFactory>
#include <KActionCollection>
#include <KLocalizedString>
#include <KParts/MainWindow>
#include <KJob>

K_PLUGIN_FACTORY_WITH_JSON(KDevFlatpakFactory, "kdevflatpak.json", registerPlugin<FlatpakPlugin>();)

using namespace KDevelop;

FlatpakPlugin::FlatpakPlugin(QObject *parent, const QVariantList & /*args*/)
    : KDevelop::IPlugin( QStringLiteral("kdevflatpak"), parent )
{
    auto ac = actionCollection();

    auto action = new QAction(QIcon::fromTheme(QStringLiteral("run-build-clean")), i18n("Rebuild environment"), this);
    action->setWhatsThis(i18n("Recompiles all dependencies for a fresh environment."));
    action->setShortcut(Qt::CTRL | Qt::META | Qt::Key_X);
    connect(action, &QAction::triggered, this, &FlatpakPlugin::rebuildCurrent);
    ac->addAction(QStringLiteral("runtime_flatpak_rebuild"), action);

    auto exportAction = new QAction(QIcon::fromTheme(QStringLiteral("document-export")), i18n("Export flatpak environment..."), this);
    exportAction->setWhatsThis(i18n("Exports the current build into a 'bundle.flatpak' file."));
    exportAction->setShortcut(Qt::CTRL | Qt::META | Qt::Key_E);
    connect(exportAction, &QAction::triggered, this, &FlatpakPlugin::exportCurrent);
    ac->addAction(QStringLiteral("runtime_flatpak_export"), exportAction);

    runtimeChanged(ICore::self()->runtimeController()->currentRuntime());

    setXMLFile( QStringLiteral("kdevflatpakplugin.rc") );
    connect(ICore::self()->runtimeController(), &IRuntimeController::currentRuntimeChanged, this, &FlatpakPlugin::runtimeChanged);
}

FlatpakPlugin::~FlatpakPlugin() = default;

void FlatpakPlugin::runtimeChanged(KDevelop::IRuntime* newRuntime)
{
    const bool isFlatpak = qobject_cast<FlatpakRuntime*>(newRuntime);

    for(auto action: actionCollection()->actions())
        action->setEnabled(isFlatpak);
}

void FlatpakPlugin::rebuildCurrent()
{
    const auto runtime = qobject_cast<FlatpakRuntime*>(ICore::self()->runtimeController()->currentRuntime());
    Q_ASSERT(runtime);
    runtime->rebuild();
}

void FlatpakPlugin::exportCurrent()
{
    const auto runtime = qobject_cast<FlatpakRuntime*>(ICore::self()->runtimeController()->currentRuntime());
    Q_ASSERT(runtime);

    const QString name = runtime->name();
    const QString path = QFileDialog::getSaveFileName(ICore::self()->uiController()->activeMainWindow(), i18n("Export %1 to..."), {}, i18n("Flatpak Bundle (*.flatpak)"));
    if (!path.isEmpty()) {
        runtime->exportBundle(path);
    }
}

KDevelop::ContextMenuExtension FlatpakPlugin::contextMenuExtension(KDevelop::Context* context)
{
    QList<QUrl> urls;

    if ( context->type() == KDevelop::Context::FileContext ) {
        KDevelop::FileContext* filectx = dynamic_cast<KDevelop::FileContext*>( context );
        urls = filectx->urls();
    } else if ( context->type() == KDevelop::Context::ProjectItemContext ) {
        KDevelop::ProjectItemContext* projctx = dynamic_cast<KDevelop::ProjectItemContext*>( context );
        foreach( KDevelop::ProjectBaseItem* item, projctx->items() ) {
            if ( item->file() ) {
                urls << item->file()->path().toUrl();
            }
        }
    }

    const QRegularExpression nameRx(".*\\..*\\.*.json$");
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

            auto action = new QAction(i18n("Create flatpak environment for %1", file.lastPathSegment()), this);
            connect(action, &QAction::triggered, this, [this, file]() {
                QTemporaryDir dir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QStringLiteral("/kdevelop-flatpak-"));
                dir.setAutoRemove(false);

                const KDevelop::Path path(dir.path());

                auto process = FlatpakRuntime::createBuildDirectory(path, file);
                connect(process, &KJob::finished, this, [this, path, file] (KJob* job) {
                    if (job->error() != 0)
                        return;

                    auto runtime = new FlatpakRuntime(path, file);
                    ICore::self()->runtimeController()->addRuntimes({runtime});
                });
                process->start();
            });
            ext.addAction(KDevelop::ContextMenuExtension::RunGroup, action);
        }

        return ext;
    }

    return KDevelop::IPlugin::contextMenuExtension( context );
}


#include "flatpakplugin.moc"
