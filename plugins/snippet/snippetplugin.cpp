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

#include <KLocalizedString>
#include <kpluginfactory.h>
#include <kaboutdata.h>
#include <kpluginloader.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <ktexteditor/codecompletioninterface.h>

#include <QAction>
#include <QMenu>

#include <KActionCollection>

#include <KTextEditor/Editor>

#include <interfaces/ipartcontroller.h>
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <language/codecompletion/codecompletion.h>
#include <language/interfaces/editorcontext.h>
#include <KConfigGroup>

#include "legacy/snippetview.h"
#include "legacy/snippetcompletionmodel.h"
#include "legacy/snippetstore.h"

#include "legacy/snippet.h"
#include "legacy/snippetrepository.h"
#include "legacy/snippetcompletionitem.h"
#include "legacy/editsnippet.h"

K_PLUGIN_FACTORY_WITH_JSON(SnippetFactory, "kdevsnippet.json", registerPlugin<SnippetPlugin>(); )

SnippetPlugin* SnippetPlugin::m_self = 0;

class SnippetViewFactory: public KDevelop::IToolViewFactory{
public:
    SnippetViewFactory(SnippetPlugin *plugin): m_plugin(plugin) {}

    virtual QWidget* create(QWidget *parent = 0)
    {
        KTextEditor::Editor *editor = KDevelop::ICore::self()->partController()->editorPart();
        // use snippets widget provided by editor component, if any
        if (QWidget *snippets = editor->property("snippetWidget").value<QWidget*>()) {
            // add snippets widget
            snippets->setParent (parent);

            return snippets;
        } else {
            // else use own implementation
            // TODO: remove own implementation
            return new SnippetView( m_plugin, parent);
        }
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
  : KDevelop::IPlugin("kdevsnippet", parent)
{
    Q_ASSERT(!m_self);
    m_self = this;
    
    KTextEditor::Editor *editor = KDevelop::ICore::self()->partController()->editorPart();
    if (editor->metaObject()->indexOfProperty("snippetWidget") == -1) {

        SnippetStore::init(this);

        m_model = new SnippetCompletionModel;
        new KDevelop::CodeCompletion(this, m_model, QString());

        setXMLFile( "kdevsnippet.rc" );

        connect( core()->partController(), SIGNAL(partAdded(KParts::Part*)), this, SLOT(documentLoaded(KParts::Part*)) );
    }

    m_factory = new SnippetViewFactory(this);
    core()->uiController()->addToolView(i18n("Snippets"), m_factory);
}

SnippetPlugin::~SnippetPlugin()
{
    m_self = 0;
}

SnippetPlugin* SnippetPlugin::self()
{
    return m_self;
}

void SnippetPlugin::unload()
{
    core()->uiController()->removeToolView(m_factory);
    delete SnippetStore::self();
}

void SnippetPlugin::insertSnippet(Snippet* snippet)
{
    KTextEditor::View* view = core()->documentController()->activeTextDocumentView();
    if (!view)
        return;
    SnippetCompletionItem item(snippet, static_cast<SnippetRepository*>(snippet->parent()));
    KTextEditor::Range range = view->selectionRange();
    if ( !range.isValid() ) {
        range = KTextEditor::Range(view->cursorPosition(), view->cursorPosition());
    }
    item.execute(view, range);
    if ( view ) {
        view->setFocus();
    }
}

void SnippetPlugin::insertSnippetFromActionData()
{
    QAction* action = dynamic_cast<QAction*>(sender());
    Q_ASSERT(action);
    Snippet* snippet = action->data().value<Snippet*>();
    Q_ASSERT(snippet);
    insertSnippet(snippet);
}

void SnippetPlugin::viewCreated( KTextEditor::Document*, KTextEditor::View* view )
{
    QAction* selectionAction = view->actionCollection()->addAction("edit_selection_snippet", this, SLOT(createSnippetFromSelection()));
    selectionAction->setData(QVariant::fromValue<void *>(view));
}

void SnippetPlugin::documentLoaded( KParts::Part* part )
{
    KTextEditor::Document *textDocument = dynamic_cast<KTextEditor::Document*>( part );
    if ( textDocument ) {
        foreach( KTextEditor::View* view, textDocument->views() )
          viewCreated( textDocument, view );

        connect( textDocument, SIGNAL(viewCreated(KTextEditor::Document*,KTextEditor::View*)), SLOT(viewCreated(KTextEditor::Document*,KTextEditor::View*)) );

    }
}

KDevelop::ContextMenuExtension SnippetPlugin::contextMenuExtension(KDevelop::Context* context)
{
    KDevelop::ContextMenuExtension extension = KDevelop::IPlugin::contextMenuExtension(context);

    KTextEditor::Editor *editor = KDevelop::ICore::self()->partController()->editorPart();
    if (editor->metaObject()->indexOfProperty("snippetWidget") != -1) {
        //context menu gets added by KatePart
        return extension;
    }

    if ( context->type() == KDevelop::Context::EditorContext ) {
        KDevelop::EditorContext *econtext = dynamic_cast<KDevelop::EditorContext*>(context);
        if ( econtext->view()->selection() ) {
            QAction* action = new QAction(QIcon::fromTheme("document-new"), i18n("Create Snippet"), this);
            connect(action, SIGNAL(triggered(bool)), this, SLOT(createSnippetFromSelection()));
            action->setData(QVariant::fromValue<void *>(econtext->view()));
            extension.addAction(KDevelop::ContextMenuExtension::ExtensionGroup, action);
        }
    }

    return extension;
}

void SnippetPlugin::createSnippetFromSelection()
{
    QAction * action = qobject_cast<QAction*>(sender());
    Q_ASSERT(action);
    KTextEditor::View* view = static_cast<KTextEditor::View*>(action->data().value<void *>());
    Q_ASSERT(view);

    QString mode = view->document()->highlightingModeAt(view->selectionRange().start());;

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
        match = SnippetRepository::createRepoFromName(
                i18nc("Autogenerated repository name for a programming language",
                      "%1 snippets", mode)
        );
        match->setFileTypes(QStringList() << mode);
    }

    EditSnippet dlg(match, 0, view);
    dlg.setSnippetText(view->selectionText());
    int status = dlg.exec();
    if ( created && status != QDialog::Accepted ) {
        // cleanup
        match->remove();
    }
}

#include "snippetplugin.moc"

