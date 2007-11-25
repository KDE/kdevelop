/***************************************************************************
 *   Copyright 2007 Rober Gruber <rgruber@users.sourceforge.net>                      *
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
#include <kparts/partmanager.h>

#include <icore.h>
#include <iuicontroller.h>

#include "snippetview.h"

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

    virtual Qt::DockWidgetArea defaultPosition(const QString &/*areaName*/)
    {
        return Qt::RightDockWidgetArea;
    }

private:
    SnippetPlugin *m_plugin;
};


SnippetPlugin::SnippetPlugin(QObject *parent, const QVariantList &)
  : KDevelop::IPlugin(SnippetFactory::componentData(), parent)
{
    m_factory = new SnippetViewFactory(this);
    core()->uiController()->addToolView(i18n("Snippets"), m_factory);
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

#include "snippetplugin.moc"

