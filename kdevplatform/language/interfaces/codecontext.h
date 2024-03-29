/*
    SPDX-FileCopyrightText: 2001-2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2001-2002 Bernd Gehrmann <bernd@kdevelop.org>
    SPDX-FileCopyrightText: 2001 Sandy Meier <smeier@kdevelop.org>
    SPDX-FileCopyrightText: 2002 Daniel Engelschalt <daniel.engelschalt@gmx.net>
    SPDX-FileCopyrightText: 2002 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2002-2003 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2003 Mario Scalas <mario.scalas@libero.it>
    SPDX-FileCopyrightText: 2003 Harald Fernengel <harry@kdevelop.org>
    SPDX-FileCopyrightText: 2003, 2006-2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2004 Alexander Dymo <adymo@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_CODECONTEXT_H
#define KDEVPLATFORM_CODECONTEXT_H

#include <interfaces/context.h>

#include <language/editor/documentrange.h>
#include <language/duchain/declaration.h>
#include <language/duchain/indexedducontext.h>

namespace KTextEditor {
class View;
}

namespace KDevelop {
class IndexedDeclaration;
class IndexedDUContext;

/**
 * A context that represents DUContexts. Before using this, first try casting to DeclarationContext, and use that if possible.
 */
class KDEVPLATFORMLANGUAGE_EXPORT DUContextContext
    : public Context
{
public:
    explicit DUContextContext(const IndexedDUContext& context);
    ~DUContextContext() override;

    ///Returns the represented DUContext
    IndexedDUContext context() const;

    int type() const override;

    QList<QUrl> urls() const override;

protected:
    void setContext(IndexedDUContext context);

private:
    const QScopedPointer<class DUContextContextPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DUContextContext)

    Q_DISABLE_COPY(DUContextContext)
};

/**
   A context for definition-use chain objects.
 */
class KDEVPLATFORMLANGUAGE_EXPORT DeclarationContext
    : public DUContextContext
{
public:
    /**Builds the context.
     * @param declaration The represented declaration.
     * @param use If this context represents the use of a declaration, this should contain the exact use range.
     * @param context If this represents a use, then this should be the context
     *              surrounding the use. Else it should be the context surrounding the declaration.
     */
    explicit DeclarationContext(const IndexedDeclaration& declaration,
                                const DocumentRange& use = DocumentRange::invalid(),
                                const IndexedDUContext& context = IndexedDUContext());
    ///Computes the items under the cursor
    DeclarationContext(KTextEditor::View* view, const KTextEditor::Cursor& position);

    /**Destructor.*/
    ~DeclarationContext() override;

    /// Returns the type of this context.
    int type() const override;

    ///The referenced declaration
    IndexedDeclaration declaration() const;
    ///If this code-context represents the use of a declaration, then this contains the exact position+range
    ///of that use. declaration() returns the used declaration, and context() the context
    ///that surrounds the use.
    DocumentRange use() const;

private:
    // TODO: fix constructor and make const
    QScopedPointer<class DeclarationContextPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DeclarationContext)

    Q_DISABLE_COPY(DeclarationContext)
};
}

#endif
