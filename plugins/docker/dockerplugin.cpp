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

#include "dockerplugin.h"
#include "dockerruntime.h"
#include "dockerpreferences.h"
#include "dockerpreferencessettings.h"
#include <interfaces/icore.h>
#include <interfaces/iruntimecontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <outputview/outputexecutejob.h>
#include <project/projectmodel.h>
#include <QStandardPaths>
#include <QAction>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QFileDialog>
#include <QInputDialog>
#include <QProcess>
#include <KPluginFactory>
#include <KActionCollection>
#include <KLocalizedString>
#include <KParts/MainWindow>
#include <KJob>

K_PLUGIN_FACTORY_WITH_JSON(KDevDockerFactory, "kdevdocker.json", registerPlugin<DockerPlugin>();)

using namespace KDevelop;

DockerPlugin::DockerPlugin(QObject *parent, const QVariantList & /*args*/)
    : KDevelop::IPlugin( QStringLiteral("kdevdocker"), parent )
    , m_settings(new DockerPreferencesSettings)
{
    runtimeChanged(ICore::self()->runtimeController()->currentRuntime());

    setXMLFile( QStringLiteral("kdevdockerplugin.rc") );
    connect(ICore::self()->runtimeController(), &IRuntimeController::currentRuntimeChanged, this, &DockerPlugin::runtimeChanged);

    QProcess* process = new QProcess(this);
    connect(process, static_cast<void(QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished), this, &DockerPlugin::imagesListFinished);
    process->start(QStringLiteral("docker"), {QStringLiteral("images"), QStringLiteral("--filter"), QStringLiteral("dangling=false"), QStringLiteral("--format"), QStringLiteral("{{.Repository}}:{{.Tag}}\t{{.ID}}")}, QIODevice::ReadOnly);

    DockerRuntime::s_settings = m_settings.data();
}

DockerPlugin::~DockerPlugin()
{
    DockerRuntime::s_settings = nullptr;
}

void DockerPlugin::imagesListFinished(int code)
{
    if (code != 0)
        return;

    QProcess* process = qobject_cast<QProcess*>(sender());
    Q_ASSERT(process);
    QTextStream stream(process);
    while(!stream.atEnd()) {
        const QString line = stream.readLine();
        const QStringList parts = line.split(QLatin1Char('\t'));

        const QString tag = parts[0] == QLatin1String("<none>") ? parts[1] : parts[0];
        ICore::self()->runtimeController()->addRuntimes(new DockerRuntime(tag));
    }

    process->deleteLater();
    Q_EMIT imagesListed();
}

void DockerPlugin::runtimeChanged(KDevelop::IRuntime* newRuntime)
{
    const bool isDocker = qobject_cast<DockerRuntime*>(newRuntime);

    for(auto action: actionCollection()->actions()) {
        action->setEnabled(isDocker);
    }
}

KDevelop::ContextMenuExtension DockerPlugin::contextMenuExtension(KDevelop::Context* context, QWidget* parent)
{
    QList<QUrl> urls;

    if ( context->type() == KDevelop::Context::FileContext ) {
        KDevelop::FileContext* filectx = static_cast<KDevelop::FileContext*>(context);
        urls = filectx->urls();
    } else if ( context->type() == KDevelop::Context::ProjectItemContext ) {
        KDevelop::ProjectItemContext* projctx = static_cast<KDevelop::ProjectItemContext*>(context);
        foreach( KDevelop::ProjectBaseItem* item, projctx->items() ) {
            if ( item->file() ) {
                urls << item->path().toUrl();
            }
        }
    }

    for(auto it = urls.begin(); it != urls.end(); ) {
        if (it->isLocalFile() && it->fileName() == QLatin1String("Dockerfile")) {
            ++it;
        } else {
            it = urls.erase(it);
        }
    }

    if ( !urls.isEmpty() ) {
        KDevelop::ContextMenuExtension ext;
        foreach(const QUrl &url, urls) {
            const KDevelop::Path file(url);

            auto action = new QAction(QIcon::fromTheme("text-dockerfile"), i18n("docker build '%1'", file.path()), parent);
            connect(action, &QAction::triggered, this, [this, file]() {
                const auto dir = file.parent();
                const QString name = QInputDialog::getText(
                    ICore::self()->uiController()->activeMainWindow(), i18n("Choose tag name..."),
                    i18n("Tag name for '%1'", file.path()),
                    QLineEdit::Normal, dir.lastPathSegment()
                );

                auto process = new OutputExecuteJob;
                process->setExecuteOnHost(true);
                process->setProperties(OutputExecuteJob::DisplayStdout | OutputExecuteJob::DisplayStderr);
                *process << QStringList{"docker", "build", "--tag", name, dir.toLocalFile()};
                connect(process, &KJob::finished, this, [name] (KJob* job) {
                    if (job->error() != 0)
                        return;

                    ICore::self()->runtimeController()->addRuntimes(new DockerRuntime(name));
                });
                process->start();
            });
            ext.addAction(KDevelop::ContextMenuExtension::RunGroup, action);
        }

        return ext;
    }

    return KDevelop::IPlugin::contextMenuExtension(context, parent);
}

int DockerPlugin::configPages() const
{
    return 1;
}

KDevelop::ConfigPage* DockerPlugin::configPage(int number, QWidget* parent)
{
    if (number == 0) {
        return new DockerPreferences(this, m_settings.data(), parent);
    }
    return nullptr;
}

#include "dockerplugin.moc"
