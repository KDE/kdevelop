/***************************************************************************
 *   Copyright (C) 2006 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
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
#ifndef IDEALTOOLVIEW_H
#define IDEALTOOLVIEW_H

#include <QObject>
#include <QString>
#include <QIcon>

#include "idealdefs.h"

namespace Ideal {

class Button;
class MainWindow;
class ToolViewWidget;

/**
@short Toolview.

Toolview represents a widget and its placement and area options.
*/
class IDEAL_EXPORT ToolView: public QObject {
    Q_OBJECT
public:
    enum Mode { None = 1, Visible=2, Enabled=4 };

    /**Creates a toolview with contents @p view in the @p place.
    @p area defines the or-ed list of allowed areas.*/
    ToolView(MainWindow *parent, QWidget *contents, Ideal::Place place, int area);
    virtual ~ToolView();

    /** @return the place of the toolview.*/
    Ideal::Place place() const;
    /**The or-ed list of allowed toolview areas.*/
    int area() const;
    /**The contents widget of the toolview.*/
    QWidget *contents() const;

    /** @return the Qt dock place for the toolview.*/
    Qt::DockWidgetArea dockPlace() const;
    /** @return the Qt dock place corresponding to the given @p place.*/
    static Qt::DockWidgetArea dockPlace(Ideal::Place place);

    /** @return the button for this toolview.*/
    Button *button() const;
    /** @return the dock widget for this toolview. Creates it if no dock is available.*/
    virtual ToolViewWidget *dockWidget();

    /** @return true if the toolview is visible.*/
    bool isVisible() const;
    /** @return true if the toolview is enabled.*/
    bool isEnabled() const;
    /** @return the mode of the toolview (visible and/or enabled: or-ed list of ToolView::Mode).*/
    int mode() const;

public slots:
    /**Toggles the visibility of the toolview. The toolview button is
    checked or unchecked respectively.*/
    virtual void setViewVisible(bool visible);
    /**Toggles the availability of the toolview. The toolview button
    is hidden or shown respectively. The toolview itself is not shown.*/
    virtual void setViewEnabled(bool enabled);

    /**Shows the toolview. Synonym for @ref setViewVisible(true).*/
    void showView();
    /**Hides the toolview. Synonym for @ref setViewVisible(false).*/
    void hideView();
    /**Enables the toolview. Synonym for @ref setViewEnabled(true).*/
    void enableView();
    /**Disables the toolview.  Synonym for @ref setViewEnabled(false).*/
    void disableView();

protected:
    /**Factory method to create the dock widget for this toolview.
    Reimplement to return ToolViewWidget subclasses here.*/
    virtual ToolViewWidget *createDockWidget();
    /**Adds the dockwidget to the main window and initializes it.*/
    virtual void setupDockWidget(ToolViewWidget *dockWidget);
    /**Factory method to create the toolview button for this toolview.
    Reimplement to return Button subclasses here.*/
    virtual Button *createToolViewButton(Ideal::Place place, const QString &title, const QIcon &icon);

private:
    struct ToolViewPrivate *d;
    friend class ToolViewPrivate;

};

}

#endif
