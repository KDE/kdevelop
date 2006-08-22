/***************************************************************************
*   Copyright (C) 2003 by KDevelop Authors                                *
*   kdevelop-devel@kde.org                                                *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "kdevkonsoleview_part.h"

#include <kgenericfactory.h>

#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevkonsoleview.h"

typedef KGenericFactory<KDevKonsoleViewPart> KonsoleViewFactory;
K_EXPORT_COMPONENT_FACTORY( kdevkonsoleview,
                            KonsoleViewFactory( "kdevkonsoleview" )  )

KDevKonsoleViewPart::KDevKonsoleViewPart( QObject *parent, const QStringList & )
    : KDevPlugin( KonsoleViewFactory::instance(), parent )
{
    m_konsoleView = new KDevKonsoleView;
}

KDevKonsoleViewPart::~KDevKonsoleViewPart()
{
    if ( m_konsoleView )
    {
        delete m_konsoleView;
    }
}

QWidget *KDevKonsoleViewPart::pluginView() const
{
    return m_konsoleView;
}

Qt::DockWidgetArea KDevKonsoleViewPart::dockWidgetAreaHint() const
{
    return Qt::BottomDockWidgetArea;
}

#include "kdevkonsoleview_part.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
