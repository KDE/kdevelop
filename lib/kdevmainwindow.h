/* This file is part of the KDE project
Copyright (C) 2003 F@lk Brettschneider <falkbr@kdevelop.org>
Copyright (C) 2003 John Firebaugh <jfirebaugh@kde.org>
Copyright (C) 2003 Amilcar do Carmo Lucas <amilcar@ida.ing.tu-bs.de>
Copyright (C) 2004 Alexander Dymo <adymo@kdevelop.org>

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

class QStackedWidget;

class KMenu;
class KStatusBar;

class KDevDocument;
class MainWindowShare;

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
    KDevMainWindow( QWidget *parent = 0, Qt::WFlags flags = 0 );
    virtual ~KDevMainWindow();

    /**Embeds a view of a part into the main window.
    @param view The view to embed. Must be a KPart.
    @param title The title of a view.
    @param toolTip The tooltip of a view.*/
    virtual void embedPartView( QWidget *view, const QString &title, const QString& toolTip = QString() );

    /**Embeds a toolview at the left of the main window.
    @param view The view to embed. Must be a KPart.
    @param title The title of a view.
    @param toolTip The tooltip of a view.*/
    virtual void embedSelectView( QWidget *view, const QString &title, const QString &toolTip );

    /**Embeds a toolview at the bottom of the main window.
    @param view The view to embed. Must be a KPart.
    @param title The title of a view.
    @param toolTip The tooltip of a view.*/
    virtual void embedOutputView( QWidget *view, const QString &title, const QString &toolTip );

    /**Embeds a toolview at the right of the main window.
    @param view The view to embed. Must be a KPart.
    @param title The title of a view.
    @param toolTip The tooltip of a view.*/
    virtual void embedSelectViewRight( QWidget* view, const QString& title, const QString &toolTip );

    /**Removes a view from the main window.
    @param view The view to remove.*/
    virtual void removeView( QWidget *view );

    /**Shows or hides a view.
    @param pView The view to show or hide.
    @param bEnabled true if view should be shown, false it it is not.*/
    virtual void setViewAvailable( QWidget *pView, bool bEnabled );

    virtual bool containsWidget( QWidget *widget ) const;

    /**Brings the widget to the front of the stack.
    @param widget The widget to give focus.*/
    virtual void setCurrentWidget( QWidget *widget );

    /**Raises a view (shows it if it was minimized).
    @param view The view to be raised.*/
    virtual void raiseView( QWidget *view, Qt::DockWidgetArea = Qt::AllDockWidgetAreas );

    /**Minimize a view.
    @param view The view to be lowered.*/
    virtual void lowerView( QWidget *view );

    /**@return KStatusBar object which actually represents the status bar in the main window.*/
    KStatusBar *statusBar();

public Q_SLOTS:
    /**Loads main window settings.*/
    virtual void loadSettings();
    virtual void saveSettings(); //FIXME Document

Q_SIGNALS:
    void finishedLoading();

protected:
    //FIXME DOCUMENT!!!  queryClose() must call all of the KDevCore cleanup() methods!
    virtual bool queryClose();
    virtual bool queryExit();

private Q_SLOTS:
    void gotoNextWindow();
    void gotoPreviousWindow();
    void gotoFirstWindow();
    void gotoLastWindow();

    void projectOpened();
    void configureToolbars();
    void newToolbarConfig();
    void openURL( int w );
    void fillWindowMenu();

private:
    void setupWindowMenu();
    void init();

    MainWindowShare *m_mainWindowShare;

    KMenu *m_windowMenu;
    typedef QPair<int, KDevDocument*> WinInfo;
    QList<WinInfo> m_windowList;
    QList<QDockWidget*> m_dockList;
    QStackedWidget *m_center;
};

#endif
