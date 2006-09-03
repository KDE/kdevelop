/*
 * This file is part of KDevelop
 *
 * Copyright (c) 2006 Adam Treat <treat@kde.org>
 * Copyright (c) 2006 Hamish Rodda <rodda@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "duchainview_part.h"
#include "duchainmodel.h"

#include <QTreeView>

#include <klocale.h>
#include <kgenericfactory.h>

#include <kdevcore.h>
#include <kdevmainwindow.h>

typedef KGenericFactory<DUChainViewPart> KDevDUChainViewFactory;
K_EXPORT_COMPONENT_FACTORY( kdevduchainview, KDevDUChainViewFactory( "kdevduchainview" ) )

DUChainViewPart::DUChainViewPart( QObject *parent,
                                    const QStringList& )
    : KDevPlugin( KDevDUChainViewFactory::instance(), parent )
    , m_model(new DUChainModel(this))
    , m_view(new QTreeView())
{
    m_view->setObjectName("DUChain Viewer Tree");
    m_view->setWindowTitle(i18n("Definition-Use Chain"));
    m_view->setModel(m_model);
    setXMLFile( "kdevduchainview.rc" );
}

DUChainViewPart::~DUChainViewPart()
{
    delete m_view;
}

QWidget *DUChainViewPart::pluginView() const
{
    return m_view;
}

Qt::DockWidgetArea DUChainViewPart::dockWidgetAreaHint() const
{
    return Qt::RightDockWidgetArea;
}

#include "duchainview_part.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
