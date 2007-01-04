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

class SUBLIME_EXPORT Controller: public QObject, public MainWindowOperator {
Q_OBJECT
public:
    Controller(QObject *parent = 0);
    ~Controller();

    void showArea(Area *area, MainWindow *mainWindow);

    QList<Area*> &areas() const;
    QList<Document*> &documents() const;

public slots:
    void areaReleased();
    void areaReleased(Sublime::Area *area);

protected:
    /**Reimplemented to catch document and area additions and removals.*/
    virtual void childEvent(QChildEvent *ev);

private:
    void init();
    void addArea(Area *area);
    void addDocument(Document *document);

    struct ControllerPrivate *d;

    friend class Area;
    friend class Document;

};

}

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
