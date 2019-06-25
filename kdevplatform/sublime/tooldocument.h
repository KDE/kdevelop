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

class QAction;

namespace Sublime {

class ToolDocument;
class ToolDocumentPrivate;

/**
@short Factory to create widgets for tool views
*/
class KDEVPLATFORMSUBLIME_EXPORT ToolFactory {
public:
    virtual ~ToolFactory() {}
    virtual QWidget* create(ToolDocument *doc, QWidget *parent = nullptr) = 0;
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
    explicit SimpleToolWidgetFactory(const QString &id): ToolFactory(), m_id(id) {}
    QWidget* create(ToolDocument * /*doc*/, QWidget *parent = nullptr) override
    {
        return new Widget(parent);
    }
    QList<QAction*> toolBarActions( QWidget* ) const override { return QList<QAction*>(); }
    QList< QAction* > contextMenuActions(QWidget* /*viewWidget*/) const override { return QList<QAction*>(); }
    QString id() const override { return m_id; }
    virtual bool viewsWantProgressIndicator() const { return false; }
private:
    QString m_id;
};

/**
@short Document to represent and manage widgets as tool views
*/
class KDEVPLATFORMSUBLIME_EXPORT ToolDocument: public Document {
Q_OBJECT
public:
    /**Initializes tool document with given @p factory. Document takes
    ownership over the factory and deletes it together with itself*/
    ToolDocument(const QString &title, Controller *controller, ToolFactory *factory);
    ~ToolDocument() override;

    QString documentType() const override;

    QString documentSpecifier() const override;

protected:
    QWidget *createViewWidget(QWidget *parent = nullptr) override;
    ToolFactory *factory() const;

private:
    const QScopedPointer<class ToolDocumentPrivate> d_ptr;
    Q_DECLARE_PRIVATE(ToolDocument)

    friend class View;
};

}

#endif

