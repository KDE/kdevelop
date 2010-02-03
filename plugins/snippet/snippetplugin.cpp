/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "snippetplugin.h"

#include <klocale.h>
#include <kpluginfactory.h>
#include <kaboutdata.h>
#include <kpluginloader.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <ktexteditor/codecompletioninterface.h>
#include <QMenu>

#include <kdeversion.h>
#if KDE_VERSION > KDE_MAKE_VERSION(4, 3, 80)
    #define HAVE_HIGHLIGHT_IFACE
    #include <KTextEditor/HighlightInterface>
#endif

#include <interfaces/ipartcontroller.h>
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocumentcontroller.h>

#include "snippetview.h"
#include "snippetcompletionmodel.h"
#include "snippetstore.h"

#include "snippet.h"
#include "snippetrepository.h"
#include "snippetcompletionitem.h"
#include "editsnippet.h"

K_PLUGIN_FACTORY(SnippetFactory, registerPlugin<SnippetPlugin>(); )
K_EXPORT_PLUGIN(SnippetFactory(KAboutData("kdevsnippet","kdevsnippet", ki18n("Snippets"), "0.1", ki18n("Support for managing and using code snippets"), KAboutData::License_GPL)))

class SnippetViewFactory: public KDevelop::IToolViewFactory{
public:
    SnippetViewFactory(SnippetPlugin *plugin): m_plugin(plugin) {}

    virtual QWidget* create(QWidget *parent = 0)
    {
        Q_UNUSED(parent)
        return new SnippetView( m_plugin, parent);
    }

    virtual Qt::DockWidgetArea defaultPosition()
    {
        return Qt::RightDockWidgetArea;
    }

    virtual QString id() const
    {
        return "org.kdevelop.SnippetView";
    }

private:
    SnippetPlugin *m_plugin;
};


SnippetPlugin::SnippetPlugin(QObject *parent, const QVariantList &)
  : KDevelop::IPlugin(SnippetFactory::componentData(), parent)
{
    SnippetStore::init(this);

    m_model = new SnippetCompletionModel;

    setXMLFile( "kdevsnippet.rc" );

    m_factory = new SnippetViewFactory(this);
    core()->uiController()->addToolView(i18n("Snippets"), m_factory);
    connect( core()->partController(), SIGNAL(partAdded(KParts::Part*)), this, SLOT(documentLoaded(KParts::Part*)) );
}

SnippetPlugin::~SnippetPlugin()
{
}

void SnippetPlugin::unload()
{
    core()->uiController()->removeToolView(m_factory);
    delete SnippetStore::self();
}

void SnippetPlugin::insertSnippet(Snippet* snippet)
{
    KDevelop::IDocument* doc = core()->documentController()->activeDocument();
    if (!doc) return;
    if (doc->isTextDocument()) {
        SnippetCompletionItem item(snippet);
        KTextEditor::Range range = doc->textSelection();
        if ( !range.isValid() ) {
            range = KTextEditor::Range(doc->cursorPosition(), doc->cursorPosition());
        }
        item.execute(doc->textDocument(), range);
        if ( doc->textDocument()->activeView() ) {
            doc->textDocument()->activeView()->setFocus();
        }
    }
}

void SnippetPlugin::viewCreated( KTextEditor::Document*, KTextEditor::View* view )
{
    if( KTextEditor::CodeCompletionInterface* cc = dynamic_cast<KTextEditor::CodeCompletionInterface*>( view ) )
    {
        cc->registerCompletionModel( m_model );
    }
    connect(view, SIGNAL(contextMenuAboutToShow(KTextEditor::View*,QMenu*)),
            this, SLOT(contextMenuAboutToShow(KTextEditor::View*,QMenu*)));
}

void SnippetPlugin::documentLoaded( KParts::Part* part )
{
    KTextEditor::Document *textDocument = dynamic_cast<KTextEditor::Document*>( part );
    if ( textDocument ) {
        foreach( KTextEditor::View* view, textDocument->views() )
          viewCreated( textDocument, view );

        connect( textDocument, SIGNAL( viewCreated( KTextEditor::Document*, KTextEditor::View* ) ), SLOT( viewCreated(KTextEditor::Document*, KTextEditor::View* ) ) );

    } else {
        kDebug() << "Non-text editor document added";
    }

}

void SnippetPlugin::contextMenuAboutToShow(KTextEditor::View* view, QMenu* menu)
{
    QAction* action = menu->findChild<QAction*>("create-snippet-from-selection");
    if ( !action ) {
        action = menu->addAction(KIcon("document-new"), i18n("Create Snippet from Selection"));
        action->setObjectName("create-snippet-from-selection");
        action->setData(QVariant(QMetaType::VoidStar, view));
        connect(action, SIGNAL(triggered(bool)), this, SLOT(createSnippetFromSelection()));
    }
    action->setEnabled(view->selection());
    m_view = view;
}

void SnippetPlugin::createSnippetFromSelection()
{
    QString mode;
    #ifdef HAVE_HIGHLIGHT_IFACE
        if ( KTextEditor::HighlightInterface* iface = qobject_cast<KTextEditor::HighlightInterface*>(m_view->document()) ) {
            mode = iface->modeAt(m_view->selectionRange().start());
        }
    #endif
    if ( mode.isEmpty() ) {
        mode = m_view->document()->mode();
    }

    // try to look for a fitting repo
    SnippetRepository* match = 0;
    for ( int i = 0; i < SnippetStore::self()->rowCount(); ++i ) {
        SnippetRepository* repo = dynamic_cast<SnippetRepository*>( SnippetStore::self()->item(i) );
        if ( repo && repo->fileTypes().count() == 1 && repo->fileTypes().first() == mode ) {
            match = repo;
            break;
        }
    }
    bool created = !match;
    if ( created ) {
        const QString& name = i18n("%1 snippets", mode);
        match = new SnippetRepository(SnippetRepository::getFileForName(name));
        match->setText(name);
        match->setActive(true);
        match->setFileTypes(QStringList() << mode);
        SnippetStore::self()->appendRow(match);
    }

    Snippet* snippet = new Snippet();
    match->appendRow(snippet);
    snippet->setText("");
    snippet->setSnippet(m_view->selectionText());
    EditSnippet dlg(match, snippet, m_view);
    if ( dlg.exec() != KDialog::Accepted ) {
        // cleanup
        delete snippet;
        if ( created ) {
            match->remove();
        }
    }
}

#include "snippetplugin.moc"

