/*
    SPDX-FileCopyrightText: 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
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

namespace {
class DockerBuildJob : public OutputExecuteJob
{
public:
    explicit DockerBuildJob(QObject* parent = nullptr)
        : OutputExecuteJob(parent)
    {
        setStandardToolView(IOutputView::BuildView);
        setBehaviours(IOutputView::AllowUserClose | IOutputView::AutoScroll);
        setProperties(DisplayStdout | DisplayStderr);
    }
};
} // unnamed namespace

DockerPlugin::DockerPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& /*args*/)
    : KDevelop::IPlugin(QStringLiteral("kdevdocker"), parent, metaData)
    , m_settings(new DockerPreferencesSettings)
{
    runtimeChanged(ICore::self()->runtimeController()->currentRuntime());

    setXMLFile( QStringLiteral("kdevdockerplugin.rc") );
    connect(ICore::self()->runtimeController(), &IRuntimeController::currentRuntimeChanged, this, &DockerPlugin::runtimeChanged);

    auto* process = new QProcess(this);
    connect(process, QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),
            this, &DockerPlugin::imagesListFinished);
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

    auto* process = qobject_cast<QProcess*>(sender());
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

    const auto& actions = actionCollection()->actions();
    for (auto action: actions) {
        action->setEnabled(isDocker);
    }
}

KDevelop::ContextMenuExtension DockerPlugin::contextMenuExtension(KDevelop::Context* context, QWidget* parent)
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
        for (const QUrl& url : std::as_const(urls)) {
            const KDevelop::Path file(url);

            auto action = new QAction(QIcon::fromTheme(QStringLiteral("text-dockerfile")), i18n("docker build '%1'", file.path()), parent);
            connect(action, &QAction::triggered, this, [this, file]() {
                const auto dir = file.parent();
                const QString name = QInputDialog::getText(
                    ICore::self()->uiController()->activeMainWindow(), i18nc("@title:window", "Choose Tag Name"),
                    i18nc("@label:textbox", "Tag name for '%1':", file.path()),
                    QLineEdit::Normal, dir.lastPathSegment()
                );

                auto* const process = new DockerBuildJob(this);
                process->setExecuteOnHost(true);
                *process << QStringList{QStringLiteral("docker"), QStringLiteral("build"), QStringLiteral("--tag"), name, dir.toLocalFile()};
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
#include "moc_dockerplugin.cpp"
