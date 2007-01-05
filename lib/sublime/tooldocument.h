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
#ifndef SUBLIMETOOLDOCUMENT_H
#define SUBLIMETOOLDOCUMENT_H

#include "document.h"

#include <kdevexport.h>

namespace Sublime {

class ToolDocument;

/**
@short Factory to create widgets for toolviews
*/
class SUBLIME_EXPORT ToolFactory {
public:
    virtual ~ToolFactory() {}
    virtual QWidget* create(ToolDocument *doc, QWidget *parent = 0) = 0;
};

/**
@short Simple factory that just creates a new widget of given type
*/
template <class Widget>
class SimpleToolWidgetFactory: public ToolFactory {
public:
    virtual QWidget* create(ToolDocument */*doc*/, QWidget *parent = 0)
    {
        return new Widget(parent);
    }
};

/**
@short Document to represent and manage widgets as toolviews
*/
class SUBLIME_EXPORT ToolDocument: public Document {
public:
    /**Initializes tool document with given @p factory. Document takes
    ownership over the factory and deletes it together with itself*/
    ToolDocument(Controller *controller, ToolFactory *factory);
    ~ToolDocument();

protected:
    virtual QWidget *createViewWidget(QWidget *parent = 0);
    ToolFactory *factory() const;

private:
    struct ToolDocumentPrivate *d;

};

}

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
