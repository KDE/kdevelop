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
#ifndef SUBLIMEDOCUMENT_H
#define SUBLIMEDOCUMENT_H

#include <QObject>
#include <QList>
#include <QString>

class QWidget;

namespace Sublime {

class View;
class Controller;

class Document: public QObject {
    Q_OBJECT
public:
    Document(Controller *controller);
    ~Document();

    View *createView();
    Controller *controller() const;

    virtual QString title() const;

    //@todo adymo: make protected
    virtual QWidget *createViewWidget(QWidget *parent = 0) = 0;

protected:
    const QList<View*> &views() const;

private slots:
    void removeView();

private:
    struct DocumentPrivate *d;
};

}

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
