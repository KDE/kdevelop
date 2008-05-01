/***************************************************************************
 *   Copyright 2006-2007 Alexander Dymo  <adymo@kdevelop.org>       *
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
#ifndef SUBLIMECONTROLLER_H
#define SUBLIMECONTROLLER_H

#include <QtCore/QObject>

#include "sublimedefs.h"
#include "sublimeexport.h"

#include "mainwindowoperator.h"


namespace Sublime {

class Area;
class AreaIndex;
class Document;
class MainWindow;

/**
@short Sublime controller to manage areas, views, documents and mainwindows

Controller has to exist before any area, document or mainwindow can be created.
There's no point in having two controllers for one application unless they
need to show completely different sets of areas.
*/
class SUBLIME_EXPORT Controller: public QObject, public MainWindowOperator {
    Q_OBJECT
public:
    Controller(QObject *parent = 0);
    ~Controller();

    /**Adds the area to the controller, used by Area class.
    @todo adymo: refactor*/
    void addArea(Area *area);

    /**Shows an @p area in @p mainWindow.*/
    void showArea(Area *area, MainWindow *mainWindow);

    void showArea(const QString& areaTypeId, MainWindow *mainWindow);

    void resetCurrentArea(MainWindow *mainWindow);

    /**Returns the area with given @p areaName.*/
    Area *area(const QString &areaName);

    Area *area(int mainWindow, const QString& areaName);

    /**@return the list of areas available for use in this controller.
    This list does not include area clones.*/
    const QList<Area*> &areas() const;

    const QList<Area*> &areas(int mainWindow) const;

    const QList<Area*> &allAreas() const;

    void addMainWindow(MainWindow* mainWindow);

    /**@return the list of documents created in this controller.*/
    const QList<Document*> &documents() const;

    const QList<MainWindow*> &mainWindows() const;

    void setStatusIcon(Document* document, const QIcon& icon);

public Q_SLOTS:
    //@todo adymo: this should not be a part of public API
    /**Area can connect to this slot to release itself from its mainwindow.*/
    void areaReleased();
    /**Releases @p area from its mainwindow.*/
    void areaReleased(Sublime::Area *area);

protected:
    bool eventFilter(QObject *obj, QEvent *ev);
    void showAreaInternal(Area* area, MainWindow *mainWindow);

private Q_SLOTS:
    void notifyToolViewRemoved(Sublime::View *view, Sublime::Position);
    void notifyToolViewAdded(Sublime::View *view, Sublime::Position);
    void notifyViewRemoved(Sublime::AreaIndex*, Sublime::View *view);
    void notifyViewAdded(Sublime::AreaIndex*, Sublime::View *view);

Q_SIGNALS:
    void aboutToRemoveToolView(Sublime::View*);
    void toolViewAdded(Sublime::View*);
    void aboutToRemoveView(Sublime::View*);
    void viewAdded(Sublime::View*);

private:
    void init();
    Q_PRIVATE_SLOT(d, void removeArea(QObject*))
    Q_PRIVATE_SLOT(d, void removeDocument(QObject*))

    /**Adds the document to the controller, used by Document class.
    @todo adymo: refactor*/
    void addDocument(Document *document);

    struct ControllerPrivate *const d;

    friend class Area;
    friend class Document;

};

}

#endif

