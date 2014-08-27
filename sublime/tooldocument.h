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
#ifndef KDEVPLATFORM_SUBLIMETOOLDOCUMENT_H
#define KDEVPLATFORM_SUBLIMETOOLDOCUMENT_H

#include "document.h"

#include "sublimeexport.h"

#include <QtCore/QDebug>

class QAction;

namespace Sublime {

class ToolDocument;

/**
@short Factory to create widgets for toolviews
*/
class KDEVPLATFORMSUBLIME_EXPORT ToolFactory {
public:
    virtual ~ToolFactory() {}
    virtual QWidget* create(ToolDocument *doc, QWidget *parent = 0) = 0;
    virtual QList<QAction*> toolBarActions( QWidget* viewWidget ) const = 0;
    virtual QList<QAction*> contextMenuActions( QWidget* viewWidget ) const = 0;
    virtual QString id() const = 0;
};

/**
@short Simple factory that just creates a new widget of given type
*/
template <class Widget>
class SimpleToolWidgetFactory: public ToolFactory {
public:
    SimpleToolWidgetFactory(const QString &id): ToolFactory(), m_id(id) {}
    virtual QWidget* create(ToolDocument * /*doc*/, QWidget *parent = 0)
    {
        return new Widget(parent);
    }
    virtual QList<QAction*> toolBarActions( QWidget* ) const { return QList<QAction*>(); }
    virtual QList< QAction* > contextMenuActions(QWidget* /*viewWidget*/) const { return QList<QAction*>(); }
    virtual QString id() const { return m_id; }
    virtual bool viewsWantProgressIndicator() const { return false; }
private:
    QString m_id;
};

/**
@short Document to represent and manage widgets as toolviews
*/
class KDEVPLATFORMSUBLIME_EXPORT ToolDocument: public Document {
public:
    /**Initializes tool document with given @p factory. Document takes
    ownership over the factory and deletes it together with itself*/
    ToolDocument(const QString &title, Controller *controller, ToolFactory *factory);
    ~ToolDocument();

    virtual QString documentType() const;

    virtual QString documentSpecifier() const;

protected:
    virtual QWidget *createViewWidget(QWidget *parent = 0);
    ToolFactory *factory() const;

private:
    struct ToolDocumentPrivate * const d;
    
    friend class View;
};

}

#endif

