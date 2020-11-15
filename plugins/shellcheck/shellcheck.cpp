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
//#include <language/interfaces/editorcontext.h>

//#include <project/projectconfigpage.h>
//#include <project/projectmodel.h>
//#include <util/jobstatus.h>

#include <KPluginFactory>
#include <KLocalizedString>

#include <QAction>
#include <QMimeDatabase>


K_PLUGIN_FACTORY_WITH_JSON(KdevshellcheckFactory, "kdevshellcheck.json", registerPlugin<ShellCheck>(); )

ShellCheck::ShellCheck(QObject *parent, const QVariantList& args)
    : KDevelop::IPlugin(QStringLiteral("kdevshellcheck"), parent)
    , m_currentProject(nullptr)
{
    QIcon shellcheckIcon = QIcon::fromTheme(QStringLiteral("cppcheck"));

    Q_UNUSED(args);
    /// FIXME: some check here to disable the plugin, if shellcheck is not installed.
    qCDebug(PLUGIN_SHELLCHECK) << "Loading shellcheck plugin";
    m_contextActionFile = new QAction(shellcheckIcon, i18nc("@item:inmenu", "ShellCheck"), this);
    connect(m_contextActionFile, &QAction::triggered, this, [this]() {
        runShellcheck(false);
    });
}


KDevelop::ContextMenuExtension ShellCheck::contextMenuExtension(KDevelop::Context* context, QWidget* parent)
{
    KDevelop::ContextMenuExtension extension = KDevelop::IPlugin::contextMenuExtension(context, parent);

//     if (context->hasType(KDevelop::Context::EditorContext) && m_currentProject && !isRunning()) {
//         auto eContext = static_cast<KDevelop::EditorContext*>(context);
//         QMimeDatabase db;
//         const auto mime = db.mimeTypeForUrl(eContext->url());
// 
//         if (isSupportedMimeType(mime)) {
//             extension.addAction(KDevelop::ContextMenuExtension::AnalyzeFileGroup, m_contextActionFile);
//         }
//     }

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
    //m_model->modelReset();
    
}

void ShellCheck::runShellcheck(bool /*checkProject*/)
{
    KDevelop::IDocument* doc = core()->documentController()->activeDocument();
    Q_ASSERT(doc);

    runShellcheck(m_currentProject, doc->url().toLocalFile());
}


// needed for QObject class created from K_PLUGIN_FACTORY_WITH_JSON
#include "shellcheck.moc"
