/***************************************************************************
 *   Copyright 2007 Rober Gruber <rgruber@users.sourceforge.net>                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "snippetpart.h"

#include <klocale.h>
#include <kgenericfactory.h>
#include <ktexteditor/view.h>
#include <kparts/partmanager.h>

#include <icore.h>
#include <iuicontroller.h>

#include "snippetview.h"


typedef KGenericFactory<SnippetPart> SnippetFactory;
K_EXPORT_COMPONENT_FACTORY( kdevsnippet, SnippetFactory("kdevsnippet") )

class SnippetViewFactory: public KDevelop::IToolViewFactory{
public:
    SnippetViewFactory(SnippetPart *part): m_part(part) {}

    virtual QWidget* create(QWidget *parent = 0)
    {
        Q_UNUSED(parent)
        return new SnippetView( m_part, parent);
    }

    virtual Qt::DockWidgetArea defaultPosition(const QString &/*areaName*/)
    {
        return Qt::RightDockWidgetArea;
    }

private:
    SnippetPart *m_part;
};


SnippetPart::SnippetPart(QObject *parent, const QStringList &)
  : KDevelop::IPlugin(SnippetFactory::componentData(), parent)
{
    m_factory = new SnippetViewFactory(this);
    core()->uiController()->addToolView(i18n("Snippets"), m_factory);
}

SnippetPart::~SnippetPart()
{
    kDebug(9500) ;
}

void SnippetPart::unload()
{
    core()->uiController()->removeToolView(m_factory);
}

void SnippetPart::insertText(const QString& snippet)
{
    kDebug(9500) << "Insert Snippet:" << snippet ;

	KTextEditor::View* view = dynamic_cast<KTextEditor::View*>( core()->partManager()->activeWidget() );
	if (view) {
		view->insertText( snippet );
	}
}

#include "snippetpart.moc"

// kate: space-indent on; indent-width 4; tab-width 4; indent-mode cstyle; replace-tabs on; auto-insert-doxygen on;
