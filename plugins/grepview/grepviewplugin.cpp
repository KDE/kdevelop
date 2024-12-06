/*
    SPDX-FileCopyrightText: 1999-2001 Bernd Gehrmann <bernd@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>
    SPDX-FileCopyrightText: 2010 Julien Desgats <julien.desgats@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "grepviewplugin.h"
#include "grepdialog.h"
#include "grepoutputmodel.h"
#include "grepoutputdelegate.h"
#include "grepjob.h"
#include "grepoutputview.h"
#include "debug.h"

#include <QAction>
#include <QDBusConnection>
#include <QKeySequence>
#include <QMimeDatabase>

#include <KActionCollection>
#include <KLocalizedString>
#include <KParts/MainWindow>
#include <KTextEditor/Document>
#include <KTextEditor/View>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/contextmenuextension.h>
#include <project/projectmodel.h>
#include <util/path.h>
#include <language/interfaces/editorcontext.h>

#include <utility>

static QString patternFromSelection(const KDevelop::IDocument* doc)
{
    if (!doc)
        return QString();

    QString pattern;
    KTextEditor::Range range = doc->textSelection();
    if( range.isValid() )
    {
        pattern = doc->textDocument()->text( range );
    }
    if( pattern.isEmpty() )
    {
        pattern = doc->textWord();
    }

    // Before anything, this removes line feeds from the
    // beginning and the end.
    int len = pattern.length();
    if (len > 0 && pattern[0] == QLatin1Char('\n')) {
        pattern.remove(0, 1);
        len--;
    }
    if (len > 0 && pattern[len-1] == QLatin1Char('\n'))
        pattern.truncate(len-1);
    return pattern;
}

GrepViewPlugin::GrepViewPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList&)
    : KDevelop::IPlugin(QStringLiteral("kdevgrepview"), parent, metaData)
    , m_currentJob(nullptr)
{
    setXMLFile(QStringLiteral("kdevgrepview.rc"));

    QDBusConnection::sessionBus().registerObject( QStringLiteral("/org/kdevelop/GrepViewPlugin"),
        this, QDBusConnection::ExportScriptableSlots );

    QAction*action = actionCollection()->addAction(QStringLiteral("edit_grep"));
    action->setText(i18nc("@action", "Find/Replace in Fi&les..."));
    actionCollection()->setDefaultShortcut( action, QKeySequence(QStringLiteral("Ctrl+Alt+F")) );
    connect(action, &QAction::triggered, this, &GrepViewPlugin::showDialogFromMenu);
    action->setToolTip( i18nc("@info:tooltip", "Search for expressions over several files") );
    action->setWhatsThis( i18nc("@info:whatsthis",
                               "Opens the 'Find/Replace in Files' dialog. There you "
                               "can enter a regular expression which is then "
                               "searched for within all files in the directories "
                               "you specify. Matches will be displayed, you "
                               "can switch to a match directly. You can also do replacement.") );
    action->setIcon(QIcon::fromTheme(QStringLiteral("edit-find")));

    // instantiate delegate, it's supposed to be deleted via QObject inheritance
    new GrepOutputDelegate(this);
    m_factory = new GrepOutputViewFactory(this);
    core()->uiController()->addToolView(i18nc("@title:window", "Find/Replace in Files"), m_factory);
}

GrepOutputViewFactory* GrepViewPlugin::toolViewFactory() const
{
    return m_factory;
}

GrepViewPlugin::~GrepViewPlugin()
{
}

void GrepViewPlugin::unload()
{
    for (const QPointer<GrepDialog>& p : std::as_const(m_currentDialogs)) {
        if (p) {
            p->reject();
            p->deleteLater();
        }
    }

    core()->uiController()->removeToolView(m_factory);
}

void GrepViewPlugin::startSearch(const QString& pattern, const QString& directory, bool show)
{
    m_directory = directory;
    showDialog(false, pattern, show);
}

KDevelop::ContextMenuExtension GrepViewPlugin::contextMenuExtension(KDevelop::Context* context, QWidget* parent)
{
    KDevelop::ContextMenuExtension extension = KDevelop::IPlugin::contextMenuExtension(context, parent);
    if( context->type() == KDevelop::Context::ProjectItemContext ) {
        auto* ctx = static_cast<KDevelop::ProjectItemContext*>(context);
        QList<KDevelop::ProjectBaseItem*> items = ctx->items();
        // verify if there is only one folder selected
        if ((items.count() == 1) && (items.first()->folder())) {
            auto* action = new QAction(i18nc("@action:inmenu", "Find/Replace in This Folder..."), parent);
            action->setIcon(QIcon::fromTheme(QStringLiteral("edit-find")));
            m_contextMenuDirectory = items.at(0)->folder()->path().toLocalFile();
            connect( action, &QAction::triggered, this, &GrepViewPlugin::showDialogFromProject);
            extension.addAction( KDevelop::ContextMenuExtension::ExtensionGroup, action );
        }
    }

    if ( context->type() == KDevelop::Context::EditorContext ) {
        auto* econtext = static_cast<KDevelop::EditorContext*>(context);
        if ( econtext->view()->selection() ) {
            auto* action = new QAction(QIcon::fromTheme(QStringLiteral("edit-find")), i18nc("@action:inmenu", "&Find/Replace in Files..."), parent);
            connect(action, &QAction::triggered, this, &GrepViewPlugin::showDialogFromMenu);
            extension.addAction(KDevelop::ContextMenuExtension::ExtensionGroup, action);
        }
    }

    if(context->type() == KDevelop::Context::FileContext) {
        auto* fcontext = static_cast<KDevelop::FileContext*>(context);
        // TODO: just stat() or QFileInfo().isDir() for local files? should be faster than mime type checking
        QMimeType mimetype = QMimeDatabase().mimeTypeForUrl(fcontext->urls().at(0));
        static const QMimeType directoryMime = QMimeDatabase().mimeTypeForName(QStringLiteral("inode/directory"));
        if (mimetype == directoryMime) {
            auto* action = new QAction(i18nc("@action:inmenu", "Find/Replace in This Folder..."), parent);
            action->setIcon(QIcon::fromTheme(QStringLiteral("edit-find")));
            m_contextMenuDirectory = fcontext->urls().at(0).toLocalFile();
            connect( action, &QAction::triggered, this, &GrepViewPlugin::showDialogFromProject);
            extension.addAction( KDevelop::ContextMenuExtension::ExtensionGroup, action );
        }
    }
    return extension;
}

void GrepViewPlugin::showDialog(bool setLastUsed, const QString& pattern, bool show)
{
    // check if dialog pointers are still valid, remove them otherwise
    m_currentDialogs.removeAll(QPointer<GrepDialog>());

    auto* const dlg = new GrepDialog(this, nullptr, core()->uiController()->activeMainWindow(), show);
    m_currentDialogs << dlg;

    if (!show) {
        // The UI is uninitialized, so the settings must be read from config.
        dlg->setLastUsedSettings();
    }

    if(!pattern.isEmpty())
    {
        dlg->setPattern(pattern);
    }
    else if(!setLastUsed)
    {
        QString pattern = patternFromSelection(core()->documentController()->activeDocument());
        if (!pattern.isEmpty()) {
            dlg->setPattern(std::move(pattern));
        }
    }

    //if directory is empty then use a default value from the config file.
    if (!m_directory.isEmpty()) {
        dlg->setSearchLocations(m_directory);
    }

    if(show)
        dlg->show();
    else{
        dlg->startSearch();
    }
}

void GrepViewPlugin::showDialogFromMenu()
{
    showDialog();
}

void GrepViewPlugin::showDialogFromProject()
{
    rememberSearchDirectory(m_contextMenuDirectory);
    showDialog();
}

void GrepViewPlugin::rememberSearchDirectory(QString const & directory)
{
    m_directory = directory;
}

GrepJob* GrepViewPlugin::newGrepJob()
{
    if(m_currentJob != nullptr)
    {
        m_currentJob->kill();
    }
    m_currentJob = new GrepJob();
    connect(m_currentJob, &GrepJob::finished, this, &GrepViewPlugin::jobFinished);
    return m_currentJob;
}

GrepJob* GrepViewPlugin::grepJob()
{
    return m_currentJob;
}

void GrepViewPlugin::jobFinished(KJob* job)
{
    if(job == m_currentJob)
    {
        m_currentJob = nullptr;
        emit grepJobFinished(job->error() == KJob::NoError);
    }
}

#include "moc_grepviewplugin.cpp"
