/***************************************************************************
*   Copyright 2003, 2006 Adam Treat <treat@kde.org>                       *
*   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "kdevkonsoleview_part.h"

#include <kgenericfactory.h>

#include <iuicontroller.h>
#include "icore.h"
#include "kdevkonsoleview.h"

QObject* createKonsoleView( QWidget*, QObject* op, const QVariantList& args)
{
    KPluginFactory *factory = KPluginLoader("libkonsolepart").factory();
    if( factory )
        return new KDevKonsoleViewPart( op, args );
    return 0;
}

K_PLUGIN_FACTORY(KonsoleViewFactory, registerPlugin<KDevKonsoleViewPart>( "kdevkonsoleview", &createKonsoleView ); )
K_EXPORT_PLUGIN(KonsoleViewFactory("kdevkonsoleview"))

class KDevKonsoleViewFactory: public KDevelop::IToolViewFactory{
public:
    KDevKonsoleViewFactory(KDevKonsoleViewPart *part): m_part(part) {}
    virtual QWidget* create(QWidget *parent = 0)
    {
        return new KDevKonsoleView(m_part, parent);
    }
    virtual Qt::DockWidgetArea defaultPosition(const QString &/*areaName*/)
    {
        return Qt::BottomDockWidgetArea;
    }
private:
    KDevKonsoleViewPart *m_part;
};

KDevKonsoleViewPart::KDevKonsoleViewPart( QObject *parent, const QVariantList & )
    : KDevelop::IPlugin( KonsoleViewFactory::componentData(), parent )
{
    m_factory = new KDevKonsoleViewFactory(this);
    core()->uiController()->addToolView("Konsole", m_factory);
}

KDevKonsoleViewPart::~KDevKonsoleViewPart()
{
}

#include "kdevkonsoleview_part.moc"

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
