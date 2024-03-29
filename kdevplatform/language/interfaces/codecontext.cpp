/*
    SPDX-FileCopyrightText: 2001-2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2001-2002 Bernd Gehrmann <bernd@kdevelop.org>
    SPDX-FileCopyrightText: 2001 Sandy Meier <smeier@kdevelop.org>
    SPDX-FileCopyrightText: 2002 Daniel Engelschalt <daniel.engelschalt@gmx.net>
    SPDX-FileCopyrightText: 2002 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2002-2003 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2003 Mario Scalas <mario.scalas@libero.it>
    SPDX-FileCopyrightText: 2003 Harald Fernengel <harry@kdevelop.org>
    SPDX-FileCopyrightText: 2003, 2006, 2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2004 Alexander Dymo <adymo@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "codecontext.h"

#include <duchain/declaration.h>
#include <duchain/ducontext.h>
#include <duchain/duchainlock.h>
#include <duchain/duchain.h>
#include <duchain/duchainutils.h>
#include <duchain/use.h>

#include <KTextEditor/Document>
#include <KTextEditor/View>

namespace KDevelop {
class DUContextContextPrivate
{
public:
    explicit DUContextContextPrivate(const IndexedDUContext& item)
        : m_item(item)
    {}

    IndexedDUContext m_item;
};

DUContextContext::DUContextContext(const IndexedDUContext& item)
    : Context()
    , d_ptr(new DUContextContextPrivate(item))
{}

DUContextContext::~DUContextContext() = default;

int DUContextContext::type() const
{
    return Context::CodeContext;
}

QList<QUrl> DUContextContext::urls() const
{
    Q_D(const DUContextContext);

    DUChainReadLocker lock;
    if (auto context = d->m_item.context()) {
        return {
                   context->url().toUrl()
        };
    }
    return {};
}

IndexedDUContext DUContextContext::context() const
{
    Q_D(const DUContextContext);

    return d->m_item;
}
void DUContextContext::setContext(IndexedDUContext context)
{
    Q_D(DUContextContext);

    d->m_item = context;
}

class DeclarationContextPrivate
{
public:
    DeclarationContextPrivate(const IndexedDeclaration& declaration, const DocumentRange& use)
        : m_declaration(declaration)
        , m_use(use)
    {}

    IndexedDeclaration m_declaration;
    DocumentRange m_use;
};

DeclarationContext::DeclarationContext(const IndexedDeclaration& declaration, const DocumentRange& use,
                                       const IndexedDUContext& context)
    : DUContextContext(context)
    , d_ptr(new DeclarationContextPrivate(declaration, use))
{}

DeclarationContext::DeclarationContext(KTextEditor::View* view, const KTextEditor::Cursor& position) : DUContextContext(
        IndexedDUContext())
{
    const QUrl& url = view->document()->url();
    DUChainReadLocker lock;
    DUChainUtils::ItemUnderCursor item = DUChainUtils::itemUnderCursor(url, position);
    DocumentRange useRange = DocumentRange(IndexedString(url), item.range);
    Declaration* declaration = item.declaration;
    IndexedDeclaration indexed;
    if (declaration) {
        indexed = IndexedDeclaration(declaration);
    }
    d_ptr.reset(new DeclarationContextPrivate(declaration, useRange));
    setContext(IndexedDUContext(item.context));
}

DeclarationContext::~DeclarationContext() = default;

int DeclarationContext::type() const
{
    return Context::CodeContext;
}

IndexedDeclaration DeclarationContext::declaration() const
{
    Q_D(const DeclarationContext);

    return d->m_declaration;
}

DocumentRange DeclarationContext::use() const
{
    Q_D(const DeclarationContext);

    return d->m_use;
}
}
