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

#include <QString>
#include "kdevexport.h"
class QWidget;
class KStatusBar;
class KMainWindow;

/**
@file kdevmainwindow.h
KDevelop main window interface.
*/

/**
KDevelop main window interface.
Provides methods to control the main window of an application.
*/
class KDEVINTERFACES_EXPORT KDevMainWindow
{
public:

	virtual ~KDevMainWindow(){};
    /**Embeds a view of a part into the main window.
    @param view The view to embed. Must be a KPart.
    @param title The title of a view.
    @param toolTip The tooltip of a view.*/
    virtual void embedPartView(QWidget *view, const QString &title, const QString& toolTip = QString()) = 0;

    /**Embeds a toolview at the left of the main window.
    @param view The view to embed. Must be a KPart.
    @param title The title of a view.
    @param toolTip The tooltip of a view.*/
    virtual void embedSelectView(QWidget *view, const QString &title, const QString &toolTip) = 0;

    /**Embeds a toolview at the bottom of the main window.
    @param view The view to embed. Must be a KPart.
    @param title The title of a view.
    @param toolTip The tooltip of a view.*/
    virtual void embedOutputView(QWidget *view, const QString &title, const QString &toolTip) = 0;

    /**Embeds a toolview at the right of the main window.
    @param view The view to embed. Must be a KPart.
    @param title The title of a view.
    @param toolTip The tooltip of a view.*/
    virtual void embedSelectViewRight(QWidget* view, const QString& title, const QString &toolTip) = 0;

    /**Removes a view from the main window.
    @param view The view to remove.*/
    virtual void removeView(QWidget *view) = 0;

    /**Shows or hides a view.
    @param pView The view to show or hide.
    @param bEnabled true if view should be shown, false it it is not.*/
    virtual void setViewAvailable(QWidget *pView, bool bEnabled) = 0;

    virtual bool containsWidget( QWidget *widget ) const = 0;

    /**Brings the widget to the front of the stack.
    @param widget The widget to give focus.*/
    virtual void setCurrentWidget(QWidget *widget) = 0;

    /**Raises a view (shows it if it was minimized).
    @param view The view to be raised.*/
    virtual void raiseView(QWidget *view, Qt::DockWidgetArea = Qt::AllDockWidgetAreas) = 0;

    /**Minimize a view.
    @param view The view to be lowered.*/
    virtual void lowerView(QWidget *view) = 0;

    /**Loads main window settings.*/
    virtual void loadSettings() = 0;

    /**@return KMainWindow object which actually represents the main window.*/
    virtual KMainWindow *main() = 0;

    /**@return KStatusBar object which actually represents the status bar in the main window.*/
    KStatusBar *statusBar();
};

#endif
