/* This file is part of KDevelop
Copyright 2001-2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
Copyright 2001-2002 Bernd Gehrmann <bernd@kdevelop.org>
Copyright 2001 Sandy Meier <smeier@kdevelop.org>
Copyright 2002 Daniel Engelschalt <daniel.engelschalt@gmx.net>
Copyright 2002 Simon Hausmann <hausmann@kde.org>
Copyright 2002-2003 Roberto Raggi <roberto@kdevelop.org>
Copyright 2003 Mario Scalas <mario.scalas@libero.it>
Copyright 2003 Harald Fernengel <harry@kdevelop.org>
Copyright 2003,2006-2007 Hamish Rodda <rodda@kde.org>
Copyright 2004 Alexander Dymo <adymo@kdevelop.org>
Copyright 2006 Adam Treat <treat@kde.org>
Copyright 2007 Andreas Pakulat <apaku@gmx.org>

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

#ifndef KDEVPLATFORM_CODECONTEXT_H
#define KDEVPLATFORM_CODECONTEXT_H

#include <interfaces/context.h>

#include <language/editor/documentrange.h>
#include <language/duchain/declaration.h>
#include <language/duchain/indexedducontext.h>

namespace KTextEditor
{
class View;
}

namespace KDevelop
{

class IndexedDeclaration;
class IndexedDUContext;

/**
 * A context that represents DUContexts. Before using this, first try casting to DeclarationContext, and use that if possible.
 */
class KDEVPLATFORMLANGUAGE_EXPORT  DUContextContext : public Context {
public:
    DUContextContext(const IndexedDUContext& context);
    virtual ~DUContextContext();
        
    ///Returns the represented DUContext
    IndexedDUContext context() const;
    
    int type() const;

    virtual QList<QUrl> urls() const override;
    
protected:
    void setContext(IndexedDUContext context);
private:
    class Private;
    Private *d;

    Q_DISABLE_COPY(DUContextContext)
};

/**
A context for definition-use chain objects.
 */
class KDEVPLATFORMLANGUAGE_EXPORT DeclarationContext: public DUContextContext
{
public:
    /**Builds the context.
     * @param declaration The represented declaration.
     * @param use If this context represents the use of a declaration, this should contain the exact use range.
     * @param context If this represents a use, then this should be the context 
     *              surrounding the use. Else it should be the context surrounding the declaration.
     */
    DeclarationContext(const IndexedDeclaration& declaration, const DocumentRange& use = DocumentRange::invalid(), const IndexedDUContext& context = IndexedDUContext());
    ///Computes the items under the cursor
    DeclarationContext(KTextEditor::View* view, KTextEditor::Cursor position);

    /**Destructor.*/
    virtual ~DeclarationContext();

    /// Returns the type of this context.
    virtual int type() const;

    ///The referenced declaration
    IndexedDeclaration declaration() const;
    ///If this code-context represents the use of a declaration, then this contains the exact position+range
    ///of that use. declaration() returnes the used declaration, and context() the context
    ///that surrounds the use.
    DocumentRange use() const;

private:
    class Private;
    Private *d;

    Q_DISABLE_COPY(DeclarationContext)
};

}

#endif

