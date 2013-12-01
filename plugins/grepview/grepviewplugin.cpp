/***************************************************************************
*   Copyright 1999-2001 by Bernd Gehrmann                                 *
*   bernd@kdevelop.org                                                    *
*   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
*   Copyright 2010 Benjamin Port <port.benjamin@gmail.com>                *
*   Copyright 2010 Julien Desgats <julien.desgats@gmail.com>              *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "grepviewplugin.h"
#include "grepdialog.h"
#include "grepoutputmodel.h"
#include "grepoutputdelegate.h"
#include "grepjob.h"
#include "grepoutputview.h"

#include <QWhatsThis>

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kaboutdata.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <ktexteditor/document.h>
#include <ktexteditor/cursor.h>
#include <ktexteditor/view.h>
#include <kparts/mainwindow.h>

#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <project/projectmodel.h>
#include <project/path.h>
#include <language/interfaces/editorcontext.h>
#include <outputview/ioutputview.h>
#include <QDBusConnection>

K_PLUGIN_FACTORY(GrepViewFactory, registerPlugin<GrepViewPlugin>(); )
K_EXPORT_PLUGIN(GrepViewFactory(KAboutData("kdevgrepview","kdevgrepview", ki18n("Find/Replace in Files"), "0.1", ki18n("Allows fast searching of multiple files using patterns or regular expressions. And allow to replace it too."), KAboutData::License_GPL)))

GrepViewPlugin::GrepViewPlugin( QObject *parent, const QVariantList & )
    : KDevelop::IPlugin( GrepViewFactory::componentData(), parent ), m_currentJob(0)
{
    setXMLFile("kdevgrepview.rc");

    QDBusConnection::sessionBus().registerObject( "/org/kdevelop/GrepViewPlugin",
        this, QDBusConnection::ExportScriptableSlots );

    KAction *action = actionCollection()->addAction("edit_grep");
    action->setText(i18n("Find/Replace in Fi&les..."));
    action->setShortcut( i18n("Ctrl+Alt+f") );
    connect(action, SIGNAL(triggered(bool)), this, SLOT(showDialogFromMenu()));
    action->setToolTip( i18n("Search for expressions over several files") );
    action->setWhatsThis( i18n("Opens the 'Find/Replace in files' dialog. There you "
                               "can enter a regular expression which is then "
                               "searched for within all files in the directories "
                               "you specify. Matches will be displayed, you "
                               "can switch to a match directly. You can also do replacement.") );
    action->setIcon(KIcon("edit-find"));

    // instantiate delegate, it's supposed to be deleted via QObject inheritance
    new GrepOutputDelegate(this);
    m_factory = new GrepOutputViewFactory(this);
    core()->uiController()->addToolView(i18n("Find/Replace in Files"), m_factory);
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
    core()->uiController()->removeToolView(m_factory);
}
void GrepViewPlugin::startSearch(QString pattern, QString directory, bool showOptions)
{
    m_directory = directory;
    showDialog(false, pattern, showOptions);
}

KDevelop::ContextMenuExtension GrepViewPlugin::contextMenuExtension(KDevelop::Context* context)
{
    KDevelop::ContextMenuExtension extension = KDevelop::IPlugin::contextMenuExtension(context);
    if( context->type() == KDevelop::Context::ProjectItemContext ) {
        KDevelop::ProjectItemContext* ctx = dynamic_cast<KDevelop::ProjectItemContext*>( context );
        QList<KDevelop::ProjectBaseItem*> items = ctx->items();
        // verify if there is only one folder selected
        if ((items.count() == 1) && (items.first()->folder())) {
            KAction* action = new KAction( i18n( "Find/Replace in This Folder" ), this );
            action->setIcon(KIcon("edit-find"));
            m_contextMenuDirectory = items.at(0)->folder()->path().toLocalFile();
            connect( action, SIGNAL(triggered()), this, SLOT(showDialogFromProject()));
            extension.addAction( KDevelop::ContextMenuExtension::ExtensionGroup, action );
        }
    }

    if ( context->type() == KDevelop::Context::EditorContext ) {
        KDevelop::EditorContext *econtext = dynamic_cast<KDevelop::EditorContext*>(context);
        if ( econtext->view()->selection() ) {
            QAction* action = new QAction(KIcon("edit-find"), i18n("&Find/Replace in Files"), this);
            connect(action, SIGNAL(triggered(bool)), this, SLOT(showDialogFromMenu()));
            extension.addAction(KDevelop::ContextMenuExtension::ExtensionGroup, action);
        }
    }

    if(context->type() == KDevelop::Context::FileContext) {
        KDevelop::FileContext *fcontext = dynamic_cast<KDevelop::FileContext*>(context);
        KMimeType::Ptr mimetype = KMimeType::findByUrl( fcontext->urls().first() );
        if(mimetype->is("inode/directory")) {
            KAction* action = new KAction( i18n( "Find/Replace in This Folder" ), this );
            action->setIcon(KIcon("edit-find"));
            m_contextMenuDirectory = fcontext->urls().first().toLocalFile();
            connect( action, SIGNAL(triggered()), this, SLOT(showDialogFromProject()));
            extension.addAction( KDevelop::ContextMenuExtension::ExtensionGroup, action );
        }
    }
    return extension;
}

void GrepViewPlugin::showDialog(bool setLastUsed, QString pattern, bool showOptions)
{
    GrepDialog* dlg = new GrepDialog( this, core()->uiController()->activeMainWindow(), setLastUsed );
    KDevelop::IDocument* doc = core()->documentController()->activeDocument();
    
    if(!pattern.isEmpty())
    {
        dlg->setPattern(pattern);
    }
    else if(!setLastUsed)
    {
        QString pattern;
        if( doc )
        {
            KTextEditor::Range range = doc->textSelection();
            if( range.isValid() )
            {
                pattern = doc->textDocument()->text( range );
            }
            if( pattern.isEmpty() )
            {
                pattern = doc->textWord();
            }
        }

        // Before anything, this removes line feeds from the
        // beginning and the end.
        int len = pattern.length();
        if (len > 0 && pattern[0] == '\n')
        {
            pattern.remove(0, 1);
            len--;
        }
        if (len > 0 && pattern[len-1] == '\n')
            pattern.truncate(len-1);
        if (!pattern.isEmpty()) {
            dlg->setPattern( pattern );
        }

        dlg->enableButtonOk( !pattern.isEmpty() );
    }

    //if directory is empty then use a default value from the config file.
    if (!m_directory.isEmpty()) {
        dlg->setSearchLocations(m_directory);
    } 
    
    if(showOptions)
        dlg->show();
    else{
        dlg->start();
        dlg->deleteLater();
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
    if(m_currentJob != 0)
    {
        m_currentJob->kill();
    }
    m_currentJob = new GrepJob();
    connect(m_currentJob, SIGNAL(finished(KJob*)), this, SLOT(jobFinished(KJob*)));
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
        m_currentJob = 0;
    }
}

#include "grepviewplugin.moc"
