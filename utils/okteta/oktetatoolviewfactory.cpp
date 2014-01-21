/*
    This file is part of the KDevelop Okteta module, part of the KDE project.

    Copyright 2010 Friedrich W. H. Kossebau <kossebau@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#include "oktetatoolviewfactory.h"

// plugin
#include "kastentoolviewwidget.h"
// Kasten
#if KASTEN_VERSION == 2
#include <Kasten2/AbstractToolViewFactory>
#include <Kasten2/AbstractToolFactory>
#include <Kasten2/AbstractToolView>
#elif KASTEN_VERSION == 1
#include <Kasten1/AbstractToolViewFactory>
#include <Kasten1/AbstractToolFactory>
#include <Kasten1/AbstractToolView>
#else
#include <Kasten/AbstractToolViewFactory>
#include <Kasten/AbstractToolFactory>
#include <Kasten/AbstractToolView>
#endif
// KDE

namespace KDevelop
{

OktetaToolViewFactory::OktetaToolViewFactory( Kasten::AbstractToolViewFactory* toolViewFactory,
                                              Kasten::AbstractToolFactory* toolFactory )
  : IToolViewFactory(),
    mToolViewFactory( toolViewFactory ),
    mToolFactory( toolFactory )
{
}

QString OktetaToolViewFactory::id() const
{
    return mToolViewFactory->id();
}
Qt::DockWidgetArea OktetaToolViewFactory::defaultPosition()
{
    return (Qt::DockWidgetArea)mToolViewFactory->defaultPosition();
}

QWidget* OktetaToolViewFactory::create( QWidget* parent )
{
    Kasten::AbstractTool* tool = mToolFactory->create();
    Kasten::AbstractToolView* toolView = mToolViewFactory->create( tool );

    QWidget* widget = new KastenToolViewWidget( toolView, parent );
    widget->setWindowIcon( QIcon::fromTheme(mToolViewFactory->iconName()) );
    return widget;
}


OktetaToolViewFactory::~OktetaToolViewFactory()
{
    delete mToolViewFactory;
    delete mToolFactory;
}

}
