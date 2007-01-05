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

#include <kdevexport.h>

#include "viewcreator.h"

class QWidget;

namespace Sublime {

class View;
class Controller;

/**
@short Abstract base class for all Sublime documents

Subclass from Document and implement createViewWidget() method
to return a new widget for a view.
*/
class SUBLIME_EXPORT Document: public QObject, public ViewCreator {
    Q_OBJECT
public:
    /**Creates a document and adds it to a @p controller.*/
    Document(Controller *controller);
    ~Document();

    /**@return the new view for this document.
    @note it will not create a widget, just return a view object.*/
    View *createView();

    /**@return the controller for this document.*/
    Controller *controller() const;

    /**@return the document title*/
    virtual QString title() const;

protected:
    /**Reimplement this to create and return the new widget to display
    this document in the view. This method is used by View class when it
    is asked for its widget.*/
    virtual QWidget *createViewWidget(QWidget *parent = 0) = 0;

    /**@return the list of all views in all areas for this document.*/
    const QList<View*> &views() const;

private:
    Q_PRIVATE_SLOT(d, void removeView(QObject*))

    struct DocumentPrivate *d;

    friend class ViewWidgetCreator;
};

}

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
