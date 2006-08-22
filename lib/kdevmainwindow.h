/* This file is part of the KDE project
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

#include <QPair>
#include <kdevplugin.h>
#include <kmainwindow.h>

#include "kdevexport.h"

class KAction;
class KToggleAction;
class KToolBarPopupAction;

namespace KParts
{
class MainWindow;
}

namespace KSettings
{
class Dialog;
}

class Context;

class KDevPlugin;
class KDevDocument;

namespace KParts
{
class Part;
}

class KDevMainWindowPrivate;

/**
@file kdevmainwindow.h
KDevelop main window interface.
*/

/**
KDevelop main window interface.
Provides methods to control the main window of an application.
*/
class KDEVINTERFACES_EXPORT KDevMainWindow : public KMainWindow
{
    friend class KDevCore;
    Q_OBJECT

public:
    enum UIMode
    {
        NeutralMode,
        TopLevelMode,
        DockedMode
    };

public:
    KDevMainWindow( QWidget *parent = 0, Qt::WFlags flags = 0 );
    virtual ~KDevMainWindow();

    UIMode mode() const;

    //FIXME document this
    virtual void fillContextMenu( KMenu *menu, const Context *context );

    virtual void addDocument( KDevDocument *document );

    virtual void removeDocument( KDevDocument *document );

    virtual bool containsDocument( KDevDocument *document ) const;

    virtual void setCurrentDocument( KDevDocument *document );

    virtual void addPlugin( KDevPlugin *plugin );

    virtual void removePlugin( KDevPlugin *plugin );

public Q_SLOTS:
    /**Loads main window settings.*/
    virtual void loadSettings();
    virtual void saveSettings(); //FIXME Document

    virtual void setVisible( bool visible );
    void setUIMode( UIMode mode );

Q_SIGNALS:
    void modeChanged( UIMode mode );
    void finishedLoading();
    void contextMenu( KMenu *menu, const Context *context );

protected:
    //FIXME DOCUMENT!!!  queryClose() must call all of the KDevCore cleanup() methods!
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
    void settings();
    void activeProcessChanged( KDevPlugin *plugin, bool b );
    void documentActivated( KDevDocument *document );
    void stopPopupActivated( int );
    void stopMenuAboutToShow();
    void stopButtonPressed();
    void showMenuBar();

    void toggleStatusbar();

    void switchToNeutralMode();
    void switchToDockedMode();
    void switchToTopLevelMode();

private:
    QWidget *magicalParent() const;
    QWidget *magicalWidget( QDockWidget *dockWidget ) const;
    QDockWidget *magicalDockWidget( QWidget *widget ) const;
    Qt::WindowFlags magicalWindowFlags( const QWidget *widgetForFlags ) const;

    void setupActions();
    void setupWindowMenu();
    void init();
    QString beautifyToolTip( const QString& text ) const;
    KDevMainWindowPrivate *d;
};

#endif
