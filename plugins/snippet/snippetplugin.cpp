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
#include <kpluginloader.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <ktexteditor/codecompletioninterface.h>
#include <kparts/partmanager.h>

#include <icore.h>
#include <iuicontroller.h>

#include "snippetview.h"
#include "snippetcompletionmodel.h"

K_PLUGIN_FACTORY(SnippetFactory, registerPlugin<SnippetPlugin>(); )
K_EXPORT_PLUGIN(SnippetFactory("kdevsnippet"))

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
  : KDevelop::IPlugin(SnippetFactory::componentData(), parent), m_model( new SnippetCompletionModel )
{
    m_factory = new SnippetViewFactory(this);
    core()->uiController()->addToolView(i18n("Snippets"), m_factory);
    connect( core()->partManager(), SIGNAL(partAdded(KParts::Part*)), this, SLOT(documentLoaded(KParts::Part*)) );
}

SnippetPlugin::~SnippetPlugin()
{
    kDebug(9500) ;
}

void SnippetPlugin::unload()
{
    core()->uiController()->removeToolView(m_factory);
}

void SnippetPlugin::insertText(const QString& snippet)
{
    kDebug(9500) << "Insert Snippet:" << snippet ;

	KTextEditor::View* view = dynamic_cast<KTextEditor::View*>( core()->partManager()->activeWidget() );
	if (view) {
		view->insertText( snippet );
	}
}

void SnippetPlugin::viewCreated( KTextEditor::Document*, KTextEditor::View* view )
{
    if( KTextEditor::CodeCompletionInterface* cc = dynamic_cast<KTextEditor::CodeCompletionInterface*>( view ) )
    {
        cc->registerCompletionModel( m_model );
    }
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

#include "snippetplugin.moc"

