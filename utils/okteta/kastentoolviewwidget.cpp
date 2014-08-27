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

#include "kastentoolviewwidget.h"

// plugin
#include "oktetaview.h"
// Okteta Kasten
#include <bytearrayview.h>
// Kasten
#if KASTEN_VERSION == 2
#include <Kasten2/AbstractToolView>
#include <Kasten2/AbstractTool>
#elif KASTEN_VERSION == 1
#include <Kasten1/AbstractToolView>
#include <Kasten1/AbstractTool>
#else
#include <Kasten/AbstractToolView>
#include <Kasten/AbstractTool>
#endif
// KDev
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <sublime/mainwindow.h>
#include <sublime/controller.h>
// Qt
#include <QVBoxLayout>

namespace KDevelop
{

// TODO: find if hiddden, than unset target model
KastenToolViewWidget::KastenToolViewWidget( Kasten::AbstractToolView* toolView, QWidget* parent )
  : QWidget( parent ),
    mToolView( toolView )
{
    Sublime::Controller* controller = ICore::self()->uiController()->controller();
    connect( controller, SIGNAL(mainWindowAdded(Sublime::MainWindow*)),
             SLOT(onMainWindowAdded(Sublime::MainWindow*)) );
    const QList<Sublime::MainWindow*>& mainWindows = controller->mainWindows();
    foreach( Sublime::MainWindow* mainWindow, mainWindows )
        onMainWindowAdded( mainWindow );

    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->setMargin( 0 );
    layout->addWidget( mToolView->widget() );
}

void KastenToolViewWidget::onMainWindowAdded( Sublime::MainWindow* mainWindow )
{
    connect( mainWindow, SIGNAL(activeViewChanged(Sublime::View*)),
             SLOT(onActiveViewChanged(Sublime::View*)) );
    onActiveViewChanged( mainWindow->activeView() );
}

void KastenToolViewWidget::onActiveViewChanged( Sublime::View* view )
{
    // TODO: check if own mainWindow
    OktetaView* oktetaView = qobject_cast<OktetaView*>( view );
    Kasten::ByteArrayView* byteArrayView = oktetaView ? oktetaView->byteArrayView() : 0;
    mToolView->tool()->setTargetModel( byteArrayView );
}

KastenToolViewWidget::~KastenToolViewWidget()
{
    Kasten::AbstractTool* tool = mToolView->tool();
    delete mToolView;
    delete tool;
}

}
