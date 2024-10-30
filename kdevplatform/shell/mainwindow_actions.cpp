/*
    SPDX-FileCopyrightText: 2002 Falk Brettschneider <falkbr@kdevelop.org>
    SPDX-FileCopyrightText: 2003 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2006, 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QApplication>

#include <KConfigGroup>
#include <KNotifyConfigWidget>
#include <KToggleFullScreenAction>

#include <sublime/area.h>
#include <sublime/view.h>

#include "core.h"
#include "documentcontroller.h"
#include "mainwindow_p.h"
#include "uicontroller.h"

#include "mainwindow.h"
#include "loadedpluginsdialog.h"

#include <interfaces/itoolviewactionlistener.h>
#include <util/scopeddialog.h>

namespace KDevelop {

// merge the gotoNext and gotoPrev code, to prevent copy/paste errors
static void gotoPrevNextWindow(bool next)
{
    UiController* ui = Core::self()->uiControllerInternal();

    if( !ui->activeSublimeWindow() )
        return;

    Sublime::Area* activeArea = ui->activeArea();
    if (!activeArea)
        return;

    Sublime::View* activeView = ui->activeSublimeWindow()->activeView();

    Sublime::AreaIndex* index = activeArea->indexOf(activeView);
    if (!index)
        return;

    int viewIndex = index->views().indexOf(activeView);
    viewIndex = next ? viewIndex + 1 : viewIndex -1;

    if (viewIndex < 0)
        viewIndex = index->views().count() - 1;
    else if (viewIndex >= index->views().count())
        viewIndex = 0;

    if (viewIndex >= 0 && viewIndex < index->views().count())
        ui->activeSublimeWindow()->activateView(index->views().at(viewIndex));
}

void MainWindowPrivate::gotoNextWindow()
{
    gotoPrevNextWindow(true);
}

void MainWindowPrivate::gotoPreviousWindow()
{
    gotoPrevNextWindow(false);
}

void MainWindowPrivate::selectPrevItem()
{
    auto actionListener = qobject_cast<IToolViewActionListener*>(
        Core::self()->uiControllerInternal()->activeToolViewActionListener());
    if (actionListener) {
        actionListener->selectPreviousItem();
    }
}

void MainWindowPrivate::selectNextItem()
{
    auto actionListener = qobject_cast<IToolViewActionListener*>(
        Core::self()->uiControllerInternal()->activeToolViewActionListener());
    if (actionListener) {
        actionListener->selectNextItem();
    }
}

void MainWindowPrivate::settingsDialog()
{
    Core::self()->uiControllerInternal()->showSettingsDialog();
}

void MainWindowPrivate::newWindow()
{
    Core::self()->uiController()->switchToArea(m_mainWindow->area()->objectName(), UiController::NewWindow);
}

void MainWindowPrivate::splitHorizontal()
{
    split(Qt::Vertical);
}

void MainWindowPrivate::splitVertical()
{
    split(Qt::Horizontal);
}

void MainWindowPrivate::split(Qt::Orientation orientation)
{
    if (!m_mainWindow->area())
        return;
    Sublime::View *view = m_mainWindow->activeView();
    if (!view)
        return;

    Sublime::View *newView = view->document()->createView();
    m_mainWindow->area()->addView(newView, view, orientation);

    m_mainWindow->activateView(newView);
}

static void gotoPrevNextSplit(bool next)
{
    UiController* ui = Core::self()->uiControllerInternal();

    if( !ui->activeSublimeWindow() )
        return;

    Sublime::Area* area = ui->activeSublimeWindow()->area();
    if (!area)
        return;

    QList<Sublime::View*> topViews = ui->activeSublimeWindow()->topViews();

    Sublime::View *activeView = ui->activeSublimeWindow()->activeView();
    if (!activeView)
        return;

    int viewIndex = topViews.indexOf(activeView);
    viewIndex = next ? viewIndex + 1 : viewIndex -1;

    if (viewIndex < 0)
        viewIndex = topViews.count() - 1;
    else if (viewIndex >= topViews.count())
        viewIndex = 0;

    if (viewIndex >= 0 && viewIndex < topViews.count())
        ui->activeSublimeWindow()->activateView(topViews.at(viewIndex));
}

void MainWindowPrivate::gotoNextSplit()
{
    gotoPrevNextSplit(true);
}

void MainWindowPrivate::gotoPreviousSplit()
{
    gotoPrevNextSplit(false);
}

void MainWindowPrivate::toggleFullScreen(bool fullScreen)
{
    KToggleFullScreenAction::setFullScreen( m_mainWindow, fullScreen );
}

void MainWindowPrivate::fileNew()
{
    Core::self()->documentControllerInternal()->openDocument(DocumentController::nextEmptyDocumentUrl());
}

void MainWindowPrivate::viewAddNewToolView()
{
    Core::self()->uiControllerInternal()->selectNewToolViewToAdd(m_mainWindow);
}

void MainWindowPrivate::quitAll()
{
    QApplication::closeAllWindows();
}

void MainWindowPrivate::configureNotifications()
{
    KNotifyConfigWidget::configure(m_mainWindow);
}

void MainWindowPrivate::showLoadedPlugins()
{
    ScopedDialog<LoadedPluginsDialog> dlg(m_mainWindow);
    dlg->exec();
}

void MainWindowPrivate::contextMenuFileNew()
{
    m_mainWindow->activateView(m_tabView);
    fileNew();
}

void MainWindowPrivate::contextMenuSplitHorizontal()
{
    m_mainWindow->activateView(m_tabView);
    splitHorizontal();
}

void MainWindowPrivate::contextMenuSplitVertical()
{
    m_mainWindow->activateView(m_tabView);
    splitVertical();
}

void MainWindowPrivate::reloadAll()
{
    const auto documents = Core::self()->documentController()->openDocuments() ;
    for (IDocument* doc : documents) {
        doc->reload();
    }
}

}

