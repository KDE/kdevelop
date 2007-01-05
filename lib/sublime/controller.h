/***************************************************************************
 *   Copyright (C) 2006-2007 by Alexander Dymo  <adymo@kdevelop.org>       *
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

#include <QObject>

#include <kdevexport.h>

#include "mainwindowoperator.h"

class QChildEvent;

namespace Sublime {

class Area;
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

    /**Shows an @p area in @p mainWindow.*/
    void showArea(Area *area, MainWindow *mainWindow);

    /**@return the list of areas available for use in this controller.
    This list does not include area clones.*/
    QList<Area*> &areas() const;
    /**@return the list of documents created in this controller.*/
    QList<Document*> &documents() const;

public slots:
    //@todo adymo: this should not be a part of public API
    /**Area can connect to this slot to release itself from its mainwindow.*/
    void areaReleased();
    /**Releases @p area from its mainwindow.*/
    void areaReleased(Sublime::Area *area);

protected:
    /**Reimplemented to catch document and area removals.
    @todo adymo: refactor to use QObject::destroyed*/
    virtual void childEvent(QChildEvent *ev);

private:
    void init();

    /**Adds an area to the controller, used by Area class.
    @todo adymo: refactor*/
    void addArea(Area *area);
    /**Adds a document to the controller, used by Document class.
    @todo adymo: refactor*/
    void addDocument(Document *document);

    struct ControllerPrivate *d;

    friend class Area;
    friend class Document;

};

}

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
