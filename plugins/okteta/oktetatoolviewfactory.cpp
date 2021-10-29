/*
    SPDX-FileCopyrightText: 2010 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "oktetatoolviewfactory.h"

// plugin
#include "kastentoolviewwidget.h"
// Kasten
#include <Kasten/AbstractToolViewFactory>
#include <Kasten/AbstractToolFactory>
#include <Kasten/AbstractToolView>

#include <QIcon>

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
Qt::DockWidgetArea OktetaToolViewFactory::defaultPosition() const
{
    return (Qt::DockWidgetArea)mToolViewFactory->defaultPosition();
}

QWidget* OktetaToolViewFactory::create( QWidget* parent )
{
    Kasten::AbstractTool* tool = mToolFactory->create();
    Kasten::AbstractToolView* toolView = mToolViewFactory->create( tool );

    QWidget* widget = new KastenToolViewWidget( toolView, parent );
    widget->setWindowIcon( QIcon::fromTheme(mToolViewFactory->iconName(), widget->windowIcon()) );
    return widget;
}


OktetaToolViewFactory::~OktetaToolViewFactory()
{
    delete mToolViewFactory;
    delete mToolFactory;
}

}
