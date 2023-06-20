/*
    SPDX-FileCopyrightText: 2010 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kastentoolviewwidget.h"

// plugin
#include "oktetaview.h"
// Okteta Kasten
#include <Kasten/Okteta/ByteArrayView>
// Kasten
#include <Kasten/AbstractToolView>
#include <Kasten/AbstractTool>
// KDev
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <sublime/mainwindow.h>
#include <sublime/controller.h>
// Qt
#include <QVBoxLayout>

namespace KDevelop
{

// TODO: find if hidden, than unset target model
KastenToolViewWidget::KastenToolViewWidget( Kasten::AbstractToolView* toolView, QWidget* parent )
  : QWidget( parent ),
    mToolView( toolView )
{
    Sublime::Controller* controller = ICore::self()->uiController()->controller();
    connect( controller, &Sublime::Controller::mainWindowAdded,
             this, &KastenToolViewWidget::onMainWindowAdded );
    const QList<Sublime::MainWindow*>& mainWindows = controller->mainWindows();
    for (Sublime::MainWindow* mainWindow : mainWindows) {
        onMainWindowAdded( mainWindow );
    }

    auto* layout = new QVBoxLayout( this );
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget( mToolView->widget() );
}

void KastenToolViewWidget::onMainWindowAdded( Sublime::MainWindow* mainWindow )
{
    connect( mainWindow, &Sublime::MainWindow::activeViewChanged,
             this, &KastenToolViewWidget::onActiveViewChanged );
    onActiveViewChanged( mainWindow->activeView() );
}

void KastenToolViewWidget::onActiveViewChanged( Sublime::View* view )
{
    // TODO: check if own mainWindow
    auto* oktetaView = qobject_cast<OktetaView*>( view );
    Kasten::ByteArrayView* byteArrayView = oktetaView ? oktetaView->byteArrayView() : nullptr;
    mToolView->tool()->setTargetModel( byteArrayView );
}

KastenToolViewWidget::~KastenToolViewWidget()
{
    Kasten::AbstractTool* tool = mToolView->tool();
    delete mToolView;
    delete tool;
}

}

#include "moc_kastentoolviewwidget.cpp"
