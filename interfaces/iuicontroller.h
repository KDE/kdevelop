/***************************************************************************
 *   Copyright 2007 Alexander Dymo  <adymo@kdevelop.org>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#ifndef IUICONTROLLER_H
#define IUICONTROLLER_H

#include "interfacesexport.h"

#include <QtCore/QStringList>
#include <QtGui/QWidget>
#include <kurl.h>

class QAction;

namespace KParts {
    class MainWindow;
}
namespace Sublime{
    class Controller;
    class View;
}

namespace KDevelop {

class IDocument;

class KDEVPLATFORMINTERFACES_EXPORT IToolViewFactory {
public:
    virtual ~IToolViewFactory() {}
    virtual QWidget* create(QWidget *parent = 0) = 0;
    /* Return the identifier of this toolview.  The identifier
       is used to remember which areas the tool view should appear
       in, and must never change. */
    virtual QString id() const = 0;
    virtual Qt::DockWidgetArea defaultPosition() = 0;
    /**
      * Fetch a list of actions to add to the toolbar of the toolview @p view
      * @param view the view to which the actions should be added
      * @returns a list of actions to be added to the toolbar
      */
    
    virtual QList<QAction*> toolBarActions( QWidget* viewWidget ) const { return viewWidget->actions(); }
    virtual void viewCreated(Sublime::View* view);
};

class KDEVPLATFORMINTERFACES_EXPORT IUiController {
public:
    virtual ~IUiController();

    enum SwitchMode {
        ThisWindow /**< indicates that the area switch should be in the this window */,
        NewWindow  /**< indicates that the area switch should be using a new window */
    };

    virtual void switchToArea(const QString &areaName, SwitchMode switchMode) = 0;

    virtual void addToolView(const QString &name, IToolViewFactory *factory) = 0;
    virtual void removeToolView(IToolViewFactory *factory) = 0;

    /** @return active mainwindow or 0 if no such mainwindow is active.*/
    virtual KParts::MainWindow *activeMainWindow() = 0;

    /**
     * This is meant to be used by IDocument subclasses to initialize the
     * Sublime::Document.
     *
     * This cannot be used for anything else, without linking to the sublime
     * library, which is forbidden and may break your plugin
     */
    virtual Sublime::Controller* controller() = 0;
protected:
    IUiController();
};

}

#endif

