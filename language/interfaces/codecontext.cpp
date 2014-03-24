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

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

namespace KDevelop
{

class DUContextContext::Private
{
public:
    Private( const IndexedDUContext& item ) : m_item( item )
    {}

    IndexedDUContext m_item;
};

DUContextContext::DUContextContext( const IndexedDUContext& item )
        : Context(), d( new Private( item ) )
{}

DUContextContext::~DUContextContext()
{
    delete d;
}

int DUContextContext::type() const
{
    return Context::CodeContext;
}

IndexedDUContext DUContextContext::context() const
{
    return d->m_item;
}
void DUContextContext::setContext(IndexedDUContext context)
{
    d->m_item = context;
}

class DeclarationContext::Private
{
public:
    Private( const IndexedDeclaration& declaration, const DocumentRange& use ) : m_declaration( declaration ), m_use(use)
    {}

    IndexedDeclaration m_declaration;
    DocumentRange m_use;
};

DeclarationContext::DeclarationContext( const IndexedDeclaration& declaration, const DocumentRange& use, const IndexedDUContext& context )
        : DUContextContext(context), d( new Private( declaration, use ) )
{}

DeclarationContext::DeclarationContext(KTextEditor::View* view, KTextEditor::Cursor position) : DUContextContext(IndexedDUContext())
{
    const KUrl& url = view->document()->url();
    const SimpleCursor pos = SimpleCursor(position);
    DUChainReadLocker lock;
    DocumentRange useRange = DocumentRange(IndexedString(url), DUChainUtils::itemRangeUnderCursor(url, pos));
    Declaration* declaration = DUChainUtils::itemUnderCursor(url, pos);
    IndexedDeclaration indexed;
    if ( declaration ) {
        indexed = IndexedDeclaration(declaration);
    }
    IndexedDUContext context;
    TopDUContext* topContext = DUChainUtils::standardContextForUrl(view->document()->url());
    if(topContext) {
        DUContext* specific = topContext->findContextAt(CursorInRevision(pos.line, pos.column));
        if(specific)
            context = IndexedDUContext(specific);
    }
    d = new Private(declaration, useRange);
    setContext(context);
}

DeclarationContext::~DeclarationContext()
{
    delete d;
}

int DeclarationContext::type() const
{
    return Context::CodeContext;
}

IndexedDeclaration DeclarationContext::declaration() const
{
    return d->m_declaration;
}

DocumentRange DeclarationContext::use() const
{
    return d->m_use;
}

}

