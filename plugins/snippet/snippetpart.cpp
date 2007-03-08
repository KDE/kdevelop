/***************************************************************************
 *   Copyright (C) 2007 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
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
    kDebug() << k_funcinfo <<endl;
}

void SnippetPart::unload()
{
    core()->uiController()->removeToolView(m_factory);
}

void SnippetPart::insertText(QString snippet)
{
    kDebug() << "Insert Snippet: " << snippet <<endl;
}

#include "snippetpart.moc"

// kate: space-indent on; indent-width 4; tab-width 4; indent-mode cstyle; replace-tabs on; auto-insert-doxygen on;
