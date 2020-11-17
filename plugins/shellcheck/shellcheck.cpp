/***************************************************************************
 *   Copyright 2020  Morten Danielsen Volden                               *
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "shellcheck.h"

#include <debug.h>


#include <interfaces/contextmenuextension.h>
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/context.h>
//#include <language/interfaces/editorcontext.h>

#include <KActionCollection>
#include <KPluginFactory>
#include <KLocalizedString>

#include <QAction>
#include <QMimeDatabase>


K_PLUGIN_FACTORY_WITH_JSON(KdevshellcheckFactory, "kdevshellcheck.json", registerPlugin<shellcheck::ShellCheck>(); )

namespace shellcheck {

ShellCheck::ShellCheck(QObject *parent, const QVariantList& args)
    : KDevelop::IPlugin(QStringLiteral("kdevshellcheck"), parent)
    , m_currentProject(nullptr)
{
    Q_UNUSED(args);
    qCDebug(PLUGIN_SHELLCHECK) << "Loading shellcheck plugin";
    setXMLFile(QStringLiteral("kdevshellcheck.rc"));

    QIcon shellcheckIcon = QIcon::fromTheme(QStringLiteral("shellcheck"));

    /// FIXME: some check here to disable the plugin, if shellcheck is not installed.

    m_menuActionFile = new QAction(shellcheckIcon, i18nc("@action", "Analyze Current File with Cppcheck"), this);
    connect(m_menuActionFile, &QAction::triggered, this, [this](){
        runShellcheck(false);
    });

    m_contextActionFile = new QAction(shellcheckIcon, i18nc("@item:inmenu", "ShellCheck"), this);
    connect(m_contextActionFile, &QAction::triggered, this, [this]() {
        runShellcheck(false);
    });
}

ShellCheck::~ShellCheck() noexcept
{
}


KDevelop::ContextMenuExtension ShellCheck::contextMenuExtension(KDevelop::Context* context, QWidget* parent)
{
    KDevelop::ContextMenuExtension extension = KDevelop::IPlugin::contextMenuExtension(context, parent);

    if (context->hasType(KDevelop::Context::EditorContext) && m_currentProject && !isRunning()) {
        //auto eContext = static_cast<KDevelop::EditorContext*>(context);
        QMimeDatabase db;
        const auto mime = db.mimeTypeForUrl(context->urls().first());

        if (isSupportedMimeType(mime)) {
            extension.addAction(KDevelop::ContextMenuExtension::AnalyzeFileGroup, m_contextActionFile);
        }
    }

    return extension;
}

bool ShellCheck::isRunning() const
{
    return false;
}

bool ShellCheck::isSupportedMimeType(const QMimeType& mimeType) const
{
    return mimeType.name() == QLatin1String("text/x-shellscript");
}

void ShellCheck::runShellcheck(KDevelop::IProject* project, const QString& path) 
{    
    Q_UNUSED(project)
    Q_UNUSED(path)
    //m_model->modelReset();
    
}

void ShellCheck::runShellcheck(bool /*checkProject*/)
{
    KDevelop::IDocument* doc = core()->documentController()->activeDocument();
    Q_ASSERT(doc);

    runShellcheck(m_currentProject, doc->url().toLocalFile());
}

} // end namespace

// needed for QObject class created from K_PLUGIN_FACTORY_WITH_JSON
#include "shellcheck.moc"
