/* This file is part of the KDevelop project
Copyright (C) 2003 F@lk Brettschneider <falkbr@kdevelop.org>
Copyright (C) 2003 John Firebaugh <jfirebaugh@kde.org>
Copyright (C) 2003 Amilcar do Carmo Lucas <amilcar@ida.ing.tu-bs.de>
Copyright (C) 2004 Alexander Dymo <adymo@kdevelop.org>
Copyright (C) 2006 Adam Treat <treat@kde.org>

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
#ifndef KDEV_MAINWINDOW_H
#define KDEV_MAINWINDOW_H

#include <kmainwindow.h>
#include "kdevcore.h"

#include "kdevexport.h"

namespace Koncrete
{

class Context;

class Plugin;
class Document;

class MainWindowPrivate;

/**
KDevelop main window interface.
Provides methods to control the main window of an application.
*/
class KDEVPLATFORM_EXPORT MainWindow : public KMainWindow, protected CoreInterface
{
    friend class Core;
    Q_OBJECT

public:
    enum UIMode
    {
        NeutralMode,
        TopLevelMode,
        DockedMode
    };

public:
    MainWindow( QWidget *parent = 0, Qt::WFlags flags = 0 );
    virtual ~MainWindow();

    UIMode mode() const;

    //FIXME document this
    virtual void fillContextMenu( KMenu *menu, const Context *context );

    virtual void addDocument( Document *document );

    virtual void removeDocument( Document *document );

    virtual bool containsDocument( Document *document ) const;

    virtual void setCurrentDocument( Document *document );

public Q_SLOTS:
    virtual void addPlugin( Plugin *plugin );

    virtual void removePlugin( Plugin *plugin );

    virtual void setVisible( bool visible );

    void setUIMode( UIMode mode );

Q_SIGNALS:
    void modeChanged( UIMode mode );
    void finishedLoading();
    void contextMenu( KMenu *menu, const Context *context );

protected:
    //FIXME DOCUMENT!!!  queryClose() must call all of the Core cleanup() methods!
    virtual bool queryClose();

private Q_SLOTS:
    void gotoNextWindow();
    void gotoPreviousWindow();
    void gotoFirstWindow();
    void gotoLastWindow();

    void projectOpened();
    void projectClosed();
    void configureToolbars();
    void fillWindowMenu();
    void newToolbarConfig();

    void reportBug();
    void keyBindings();
    void configureNotifications();
    void configureEditors();
    void settingsDialog();
    void activeProcessChanged( Plugin *plugin, bool b );
    void documentActivated( Document *document );
    void stopPopupActivated( int );
    void stopMenuAboutToShow();
    void stopButtonPressed();
    void showMenuBar();

    void toggleStatusbar();

    void switchToNeutralMode();
    void switchToDockedMode();
    void switchToTopLevelMode();

protected:
    virtual void loadSettings( bool projectIsLoaded );
    virtual void saveSettings( bool projectIsLoaded );
    virtual void initialize();
    virtual void cleanup();

private:
    QWidget *magicalParent() const;
    QWidget *magicalWidget( QDockWidget *dockWidget ) const;
    QDockWidget *magicalDockWidget( QWidget *widget ) const;
    Qt::WindowFlags magicalWindowFlags( const QWidget *widgetForFlags ) const;

    void setupActions();
    void setupWindowMenu();

    QString beautifyToolTip( const QString& text ) const;
    MainWindowPrivate *d;
};

}
#endif
