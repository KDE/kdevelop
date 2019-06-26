/*
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_ABSTRACTDECLARATIONNAVIGATIONCONTEXT_H
#define KDEVPLATFORM_ABSTRACTDECLARATIONNAVIGATIONCONTEXT_H

#include "abstractnavigationcontext.h"
#include "../declaration.h"
#include "../duchainpointer.h"
#include "../types/abstracttype.h"

namespace KDevelop {
class IdentifiedType;
class Identifier;
class QualifiedIdentifier;
class AbstractDeclarationNavigationContextPrivate;

class KDEVPLATFORMLANGUAGE_EXPORT AbstractDeclarationNavigationContext
    : public AbstractNavigationContext
{
    Q_OBJECT

public:
    AbstractDeclarationNavigationContext(const DeclarationPointer& decl, const TopDUContextPointer& topContext,
                                         AbstractNavigationContext* previousContext = nullptr);
    ~AbstractDeclarationNavigationContext() override;

    QString name() const override;

    DeclarationPointer declaration() const;

    ///Execute an action. For example "show_uses" shows the uses of the declaration.
    ///Returns the context pointer for the new state.
    NavigationContextPointer executeKeyAction(const QString& key) override;

    QString html(bool shorten = false) override;

protected:
    ///Should returns a stripped version of the declarations qualified identifier, with all implicit/redundant parts removed
    virtual QualifiedIdentifier prettyQualifiedIdentifier(const DeclarationPointer& decl) const;
    ///Returns a stripped version of the declarations identifier, using prettyQualifiedIdentifier
    Identifier prettyIdentifier(const DeclarationPointer& decl) const;
    /// @return String version of the qualified identifier of @p decl, "<anonymous>" on an invalid QID
    QString prettyQualifiedName(const DeclarationPointer& decl) const;

    /**
     * Return a rich-text version of the identifier @p identifier representing the declaration @p decl
     *
     * @note In case @p declaration is deprecated, the resulting string will get a special formatting
     */
    QString identifierHighlight(const QString& identifier, const DeclarationPointer& decl) const;

    static QString stringFromAccess(Declaration::AccessPolicy access);
    static QString stringFromAccess(const DeclarationPointer& decl);
    QString declarationName(const DeclarationPointer& decl) const;
    static QStringList declarationDetails(const DeclarationPointer& decl);
    static QString declarationSizeInformation(const DeclarationPointer& decl, const TopDUContext* topContext);

    ///This can be used for example to resolve typedefs within the type.
    ///All types that are visualized in the navigation-context are/should be mangled through this.
    ///The default-implementation returns the original type.
    virtual AbstractType::Ptr typeToShow(AbstractType::Ptr type);

    ///Print the function-signature in a way that return-type and argument can be jumped to
    virtual void htmlFunction();
    ///Navigation for additional less important links, like what function was overloaded etc.
    virtual void htmlAdditionalNavigation();

    virtual void htmlClass();
    virtual void htmlIdentifiedType(AbstractType::Ptr type, const IdentifiedType* idType);

    ///Creates and registers a link for the given type that jumps to its declaration and to the template-argument declarations
    virtual void eventuallyMakeTypeLinks(KDevelop::AbstractType::Ptr type);

    ///Creates a link that triggers a recomputation of this context with m_fullBackwardSearch set to true
    void createFullBackwardSearchLink(const QString& string);

private:
    const QScopedPointer<class AbstractDeclarationNavigationContextPrivate> d_ptr;
    Q_DECLARE_PRIVATE(AbstractDeclarationNavigationContext)
};
}

#endif
