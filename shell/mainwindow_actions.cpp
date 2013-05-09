/* This file is part of the KDevelop project
Copyright 2002 Falk Brettschneider <falkbr@kdevelop.org>
Copyright 2003 John Firebaugh <jfirebaugh@kde.org>
Copyright 2006 Adam Treat <treat@kde.org>
Copyright 2006, 2007 Alexander Dymo <adymo@kdevelop.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#include <kconfiggroup.h>
#include <kapplication.h>
#include <kaboutapplicationdialog.h>
#include <knotifyconfigwidget.h>
#include <ktogglefullscreenaction.h>

#include <ktexteditor/editor.h>

#include <sublime/area.h>
#include <sublime/view.h>

#include "core.h"
#include "documentcontroller.h"
#include "mainwindow_p.h"
#include "sessiondialog.h"
#include "uicontroller.h"

#include "mainwindow.h"
#include "loadedpluginsdialog.h"
#include <interfaces/ipartcontroller.h>

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

void MainWindowPrivate::configureToolbars()
{}

void MainWindowPrivate::newToolbarConfig()
{
    m_mainWindow->applyMainWindowSettings( KConfigGroup(KGlobal::config(), "MainWindow") );
}

void MainWindowPrivate::stopButtonPressed()
{}

void MainWindowPrivate::stopPopupActivated( int id )
{
    Q_UNUSED( id );
}

void MainWindowPrivate::stopMenuAboutToShow()
{}

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
    s_quitRequested = true;

    QApplication::closeAllWindows();

    //if (Core::self()->documentController()->saveAllDocuments(IDocument::Default))
    //    return qApp->exit();

    s_quitRequested = false;
}

void MainWindowPrivate::configureNotifications()
{
    KNotifyConfigWidget::configure(m_mainWindow);
}

void MainWindowPrivate::showAboutPlatform()
{
    KAboutApplicationDialog dlg(Core::self()->componentData().aboutData(), m_mainWindow );
    dlg.exec();
}

void MainWindowPrivate::showLoadedPlugins()
{
    LoadedPluginsDialog dlg(m_mainWindow);
    dlg.exec();
}

void MainWindowPrivate::showEditorConfig()
{
    KTextEditor::Editor* editor = Core::self()->partController()->editorPart();
    if (editor->configDialogSupported()) {
        editor->configDialog(m_mainWindow);
        editor->writeConfig();
    }
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
    foreach ( IDocument* doc, Core::self()->documentController()->openDocuments() ) {
        doc->reload();
    }
}

}

