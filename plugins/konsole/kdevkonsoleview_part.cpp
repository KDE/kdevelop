/***************************************************************************
*   Copyright (C) 2003 by KDevelop Authors                                *
*   kdevelop-devel@kde.org                                                *
*   Copyright (C) 2007 by Andreas Pakulat <apaku@gmx.de>                  *
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

typedef KGenericFactory<KDevKonsoleViewPart> KonsoleViewFactory;
K_EXPORT_COMPONENT_FACTORY( kdevkonsoleview,
                            KonsoleViewFactory( "kdevkonsoleview" )  )


class KDevKonsoleViewFactory: public KDevelop::IToolViewFactory{
public:
    KDevKonsoleViewFactory(KDevKonsoleViewPart *part): m_part(part) {}
    virtual QWidget* create(QWidget *parent = 0)
    {
        return new KDevKonsoleView(m_part, parent);
    }
private:
    KDevKonsoleViewPart *m_part;
};

KDevKonsoleViewPart::KDevKonsoleViewPart( QObject *parent, const QStringList & )
    : KDevelop::IPlugin( KonsoleViewFactory::componentData(), parent )
{
    m_factory = new KDevKonsoleViewFactory(this);
    core()->uiController()->addToolView("Konsole", m_factory);
}

KDevKonsoleViewPart::~KDevKonsoleViewPart()
{
}

Qt::DockWidgetArea KDevKonsoleViewPart::dockWidgetAreaHint() const
{
    return Qt::BottomDockWidgetArea;
}

#include "kdevkonsoleview_part.moc"

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
