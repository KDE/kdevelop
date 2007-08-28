/*
 * This file is part of KDevelop
 *
 * Copyright 2006 Adam Treat <treat@kde.org>
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

#include "kdevcodeview_part.h"
#include "kdevcodeview.h"

#include <klocale.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>

#include <kdevcore.h>
#include <kdevmainwindow.h>

K_PLUGIN_FACTORY(KDevCodeViewFactory, registerPlugin<KDevCodeViewPart>(); )
K_EXPORT_PLUGIN(KDevCodeViewFactory("kdevcodeview"))

KDevCodeViewPart::KDevCodeViewPart( QObject *parent,
                                    const QVariantList& )
    : KDevelop::Plugin( KDevCodeViewFactory::componentData(), parent )
{
    m_codeView = new KDevCodeView;

    setXMLFile( "kdevcodeview.rc" );
}

KDevCodeViewPart::~KDevCodeViewPart()
{
    if ( m_codeView )
    {
        delete m_codeView;
    }
}

QWidget *KDevCodeViewPart::pluginView() const
{
    return m_codeView;
}

Qt::DockWidgetArea KDevCodeViewPart::dockWidgetAreaHint() const
{
    return Qt::RightDockWidgetArea;
}

#include "kdevcodeview_part.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
