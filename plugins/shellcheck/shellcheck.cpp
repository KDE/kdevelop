/*
 * This file is part of KDevelop
 * Copyright 2020  Morten Danielsen Volden <mvolden2@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "shellcheck.h"
#include "parameters.h"

#include "shellcheckdebug.h"

#include "config/globalconfigpage.h"

#include <interfaces/contextmenuextension.h>
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/context.h>
#include <language/interfaces/editorcontext.h>
#include <util/jobstatus.h>
#include <shell/problemmodelset.h>


#include <KActionCollection>
#include <KPluginFactory>
#include <KLocalizedString>

#include <QAction>
#include <QMimeDatabase>
#include <iproject.h>


K_PLUGIN_FACTORY_WITH_JSON(KdevshellcheckFactory, "kdevshellcheck.json", registerPlugin<shellcheck::ShellCheck>(); )

namespace shellcheck {

ShellCheck::ShellCheck(QObject *parent, const QVariantList&)
    : KDevelop::IPlugin(QStringLiteral("kdevshellcheck"), parent)
    , m_job(nullptr)
    , m_model(new KDevelop::ProblemModel(this))
{
    setXMLFile(QStringLiteral("kdevshellcheck.rc"));

    qCDebug(PLUGIN_SHELLCHECK) << "Loading shellcheck plugin .....";

    QIcon shellcheckIcon = QIcon::fromTheme(QStringLiteral("shellcheck"));

    /// FIXME: some check here to disable the plugin, if shellcheck is not installed.
    m_menuActionFile = new QAction(
        shellcheckIcon,
        i18nc("@action", "Analyze Current File with Shellcheck"),
        this);

    connect(m_menuActionFile, &QAction::triggered, 
            this, static_cast<void (ShellCheck::*)(void)>(&ShellCheck::runShellcheck));
    actionCollection()->addAction(QStringLiteral("shellcheck_file"), m_menuActionFile);

    m_contextActionFile = new QAction(
        shellcheckIcon,
        i18nc("@item:inmenu", "ShellCheck"),
                                      this);
    connect(m_contextActionFile, &QAction::triggered, 
            this, static_cast<void (ShellCheck::*)(void)>(&ShellCheck::runShellcheck));

    connect(core()->documentController(), &KDevelop::IDocumentController::documentClosed,
            this, &ShellCheck::updateActions);
    connect(core()->documentController(), &KDevelop::IDocumentController::documentActivated,
            this, &ShellCheck::updateActions);

    ///--------ProblemModel related ----------------------
    m_model->setFeatures(KDevelop::ProblemModel::CanDoFullUpdate | KDevelop::ProblemModel::ScopeFilter |
    KDevelop::ProblemModel::SeverityFilter | KDevelop::ProblemModel::Grouping | KDevelop::ProblemModel::CanByPassScopeFilter);
    m_model->clearProblems();
    KDevelop::ICore::self()->languageController()->problemModelSet()->addModel(QStringLiteral("ShellCheck"), i18n("ShellCheck"), m_model.data());

    QString tooltip;
    tooltip = i18nc("@info:tooltip", "Re-run last ShellCheck analysis");

    m_model->setFullUpdateTooltip(tooltip);
    
    updateActions();
}

ShellCheck::~ShellCheck() noexcept
{
    killShellcheckJob();
    KDevelop::ICore::self()->languageController()->problemModelSet()->removeModel(QStringLiteral("ShellCheck"));

}

bool ShellCheck::isRunning() const
{
    return m_job;
}

void ShellCheck::killShellcheckJob()
{
    if (m_job) {
        m_job->kill(KJob::EmitResult);
    }
}

void ShellCheck::raiseProblemsView()
{
    qCWarning(PLUGIN_SHELLCHECK) << "Raising the problem view..."; 
    KDevelop::ICore::self()->languageController()->problemModelSet()->showModel(QStringLiteral("ShellCheck"));
}

void ShellCheck::raiseOutputView()
{
    QWidget* test = nullptr;
    test = core()->uiController()->findToolView(
        i18nc("@title:window", "Shellcheck run"),
        nullptr,
        KDevelop::IUiController::FindFlags::Raise);
    if(test == nullptr)
        qCWarning(PLUGIN_SHELLCHECK) << "In raiseOutputView. I didn't find an outputview with that name"; 
}

KDevelop::ContextMenuExtension ShellCheck::contextMenuExtension(KDevelop::Context* context, QWidget* parent)
{
    KDevelop::ContextMenuExtension extension = KDevelop::IPlugin::contextMenuExtension(context, parent);

    qCWarning(PLUGIN_SHELLCHECK) << "In the context menu...........";

    if (context->hasType(KDevelop::Context::EditorContext) && !isRunning()) {
        QMimeDatabase db;
        const auto mime = db.mimeTypeForUrl(context->urls().first());
        qCWarning(PLUGIN_SHELLCHECK) << "The mimetype of this document is: " << mime.name();

        if (isSupportedMimeType(mime)) {
            extension.addAction(KDevelop::ContextMenuExtension::AnalyzeFileGroup, m_contextActionFile);
        }
    }

    return extension;
}


void ShellCheck::updateActions()
{
    m_menuActionFile->setEnabled(false);

    if (isRunning()) {
        return;
    }

    KDevelop::IDocument* activeDocument = core()->documentController()->activeDocument();
    if (!activeDocument) {
        return;
    }

    QUrl url = activeDocument->url();

    m_menuActionFile->setEnabled(true);
}

bool ShellCheck::isSupportedMimeType(const QMimeType& mimeType) const
{
    return mimeType.name() == QLatin1String("text/x-shellscript") || 
        mimeType.name() == QLatin1String("application/x-shellscript");
}

void ShellCheck::runShellcheck()
{
    KDevelop::IDocument* doc = core()->documentController()->activeDocument();
    Q_ASSERT(doc);
    runShellcheck(doc->url().toLocalFile());
}

void ShellCheck::runShellcheck(const QString& path) 
{    
    m_model->clearProblems();
    
    Parameters params;
    params.shellFileToCheck = path;

    m_job = new Job(params);

    connect(m_job, &Job::problemsDetected, m_model.data(), &KDevelop::ProblemModel::setProblems);
    connect(m_job, &Job::finished, this, &ShellCheck::result);

    core()->uiController()->registerStatus(new KDevelop::JobStatus(m_job, QStringLiteral("Shellcheck")));
    core()->runController()->registerJob(m_job);

    if (params.hideOutputView) {
        raiseProblemsView();
    } else {
        raiseOutputView();
    }

    updateActions();
}

void ShellCheck::result(KJob*)
{
    if (m_job->status() == KDevelop::OutputExecuteJob::JobStatus::JobSucceeded ||
       m_job->status() == KDevelop::OutputExecuteJob::JobStatus::JobCanceled) {
        qCWarning(PLUGIN_SHELLCHECK) << "The job finished and we are raising the Problemsview";
        raiseProblemsView();
    } else {
        qCWarning(PLUGIN_SHELLCHECK) << "The job finished and we are raising the Outputview";
        raiseOutputView();
    }

    m_job = nullptr; // job is automatically deleted later

    updateActions();
}

int ShellCheck::configPages() const
{
    return 1;
}

KDevelop::ConfigPage* ShellCheck::configPage(int number, QWidget* parent)
{
    if (number) {
        return nullptr;
    }

    return new GlobalConfigPage(this, parent);
}


} // end namespace

// needed for QObject class created from K_PLUGIN_FACTORY_WITH_JSON
#include "shellcheck.moc"
