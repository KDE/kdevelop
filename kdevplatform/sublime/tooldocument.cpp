/*
    SPDX-FileCopyrightText: 2006-2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "tooldocument.h"

#include <QWidget>

namespace Sublime {

// class ToolDocumentPrivate

class ToolDocumentPrivate
{
public:
    ~ToolDocumentPrivate()
    {
        delete factory;
    }
    ToolFactory *factory;
};



// class ToolDocument

ToolDocument::ToolDocument(const QString &title, Controller *controller, ToolFactory *factory)
    :Document(title, controller)
    , d_ptr(new ToolDocumentPrivate())
{
    Q_D(ToolDocument);

    d->factory = factory;
}

ToolDocument::~ToolDocument() = default;

ToolFactory *ToolDocument::factory() const
{
    Q_D(const ToolDocument);

    return d->factory;
}

QWidget *ToolDocument::createViewWidget(QWidget *parent)
{
    return factory()->create(this, parent);
}

QString ToolDocument::documentType() const
{
    return QStringLiteral("Tool");
}

QString ToolDocument::documentSpecifier() const
{
    return factory()->id();
}

}

