/* This file is part of KDevelop
   Copyright 2001-2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright 2001-2002 Bernd Gehrmann <bernd@kdevelop.org>
   Copyright 2001 Sandy Meier <smeier@kdevelop.org>
   Copyright 2002 Daniel Engelschalt <daniel.engelschalt@gmx.net>
   Copyright 2002 Simon Hausmann <hausmann@kde.org>
   Copyright 2002-2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright 2003 Mario Scalas <mario.scalas@libero.it>
   Copyright 2003 Harald Fernengel <harry@kdevelop.org>
   Copyright 2003,2006,2008 Hamish Rodda <rodda@kde.org>
   Copyright 2004 Alexander Dymo <adymo@kdevelop.org>
   Copyright 2006 Adam Treat <treat@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
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
